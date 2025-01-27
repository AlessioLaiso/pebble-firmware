/*
 * Copyright 2024 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "watch_app_prefs_db.h"

#include "kernel/pbl_malloc.h"
#include "os/mutex.h"
#include "services/normal/filesystem/pfs.h"
#include "services/normal/settings/settings_file.h"
#include "services/normal/weather/weather_service_private.h"
#include "system/logging.h"
#include "system/status_codes.h"
#include "util/units.h"
#include "util/uuid.h"

static struct {
  SettingsFile settings_file;
  PebbleRecursiveMutex *mutex;
  // We cache the reminder app prefs because they're read in reminder_app_get_info() which needs to
  // be fast because it's called by analytics from the system task while counting timeline pins
  bool is_cached_reminder_app_prefs_valid;
  SerializedReminderAppPrefs cached_reminder_app_prefs;
} s_watch_app_prefs_db;

#define SETTINGS_FILE_NAME "watch_app_prefs"
#define SETTINGS_FILE_SIZE KiBYTES(20)

T_STATIC const char *PREF_KEY_SEND_TEXT_APP = "sendTextApp";

// Settings helpers
////////////////////////////////////////////////////////////////////////////////

static status_t prv_lock_mutex_and_open_file(void) {
  mutex_lock_recursive(s_watch_app_prefs_db.mutex);
  status_t rv = settings_file_open(&s_watch_app_prefs_db.settings_file,
                                   SETTINGS_FILE_NAME,
                                   SETTINGS_FILE_SIZE);
  if (rv != S_SUCCESS) {
    mutex_unlock_recursive(s_watch_app_prefs_db.mutex);
  }
  return rv;
}

static void prv_close_file_and_unlock_mutex(void) {
  settings_file_close(&s_watch_app_prefs_db.settings_file);
  mutex_unlock_recursive(s_watch_app_prefs_db.mutex);
}

// WatchAppPrefDB API
////////////////////////////////////////////////////////////////////////////////

static void *prv_get_prefs(const char *pref_key) {
  status_t rv = prv_lock_mutex_and_open_file();
  if (rv != S_SUCCESS) {
    return NULL;
  }

  const int len = settings_file_get_len(&s_watch_app_prefs_db.settings_file, pref_key,
                                        strlen(pref_key));
  void *prefs = task_zalloc(len);

  if (prefs) {
    rv = settings_file_get(&s_watch_app_prefs_db.settings_file, pref_key,
                           strlen(pref_key), prefs, len);
    if (rv != S_SUCCESS) {
      task_free(prefs);
      prefs = NULL;
    }
  }

  prv_close_file_and_unlock_mutex();
  return prefs;
}

SerializedSendTextPrefs *watch_app_prefs_get_send_text(void) {
  return (SerializedSendTextPrefs *)prv_get_prefs(PREF_KEY_SEND_TEXT_APP);
}

SerializedWeatherAppPrefs *watch_app_prefs_get_weather(void) {
  return (SerializedWeatherAppPrefs *)prv_get_prefs(PREF_KEY_WEATHER_APP);
}

SerializedReminderAppPrefs *watch_app_prefs_get_reminder(void) {
  SerializedReminderAppPrefs *result = NULL;
  mutex_lock_recursive(s_watch_app_prefs_db.mutex);
  if (s_watch_app_prefs_db.is_cached_reminder_app_prefs_valid) {
    result = task_zalloc(sizeof(*result));
    if (result) {
      *result = s_watch_app_prefs_db.cached_reminder_app_prefs;
    }
  } else {
    result = prv_get_prefs(PREF_KEY_REMINDER_APP);
    s_watch_app_prefs_db.is_cached_reminder_app_prefs_valid = true;
    s_watch_app_prefs_db.cached_reminder_app_prefs = result ? *result :
                                                              (SerializedReminderAppPrefs) {};
  }
  mutex_unlock_recursive(s_watch_app_prefs_db.mutex);
  return result;
}

void watch_app_prefs_destroy_weather(SerializedWeatherAppPrefs *prefs) {
  if (prefs) {
    task_free(prefs);
  }
}
// BlobDB APIs
////////////////////////////////////////////////////////////////////////////////

void watch_app_prefs_db_init(void) {
  s_watch_app_prefs_db.mutex = mutex_create_recursive();
}

// All entries in this db currently follow a structure of base data + arbitrary list of records.
// All records in the list are the same size
static bool prv_validate_received_pref(const size_t received_val_size, const size_t min_val_size,
                                       const size_t num_records, const size_t record_size) {
  if ((received_val_size % record_size) != min_val_size) {
    return false;
  }
  const size_t calculated_size = min_val_size + (num_records * record_size);

  return received_val_size >= calculated_size;
}

static bool prv_is_key_valid(const uint8_t *received_key, size_t received_key_len,
                             const char *system_key) {
  return (received_key_len == strlen(system_key)) &&
         (memcmp(received_key, system_key, received_key_len) == 0);
}

status_t watch_app_prefs_db_insert(const uint8_t *key, int key_len, const uint8_t *val,
                                   int val_len) {
  const bool is_valid_send_text_key = prv_is_key_valid(key, key_len, PREF_KEY_SEND_TEXT_APP);
  const bool is_valid_weather_key = prv_is_key_valid(key, key_len, PREF_KEY_WEATHER_APP);
  const bool is_valid_reminder_key = prv_is_key_valid(key, key_len, PREF_KEY_REMINDER_APP);

  if (!is_valid_send_text_key && !is_valid_weather_key && !is_valid_reminder_key) {
    PBL_LOG(LOG_LEVEL_ERROR, "Error inserting app_prefs: invalid key");
    return E_INVALID_ARGUMENT;
  }

  if (is_valid_send_text_key &&
      !prv_validate_received_pref(val_len, sizeof(SerializedSendTextPrefs),
                                  ((SerializedSendTextPrefs *)val)->num_contacts,
                                  sizeof(SerializedSendTextContact))) {
    PBL_LOG(LOG_LEVEL_ERROR, "Error inserting app_prefs: invalid send text contact list");
    return E_INVALID_ARGUMENT;
  }

  if (is_valid_weather_key &&
      !prv_validate_received_pref(val_len, sizeof(SerializedWeatherAppPrefs),
                                  ((SerializedWeatherAppPrefs *)val)->num_locations,
                                  sizeof(Uuid))) {
    PBL_LOG(LOG_LEVEL_ERROR, "Error inserting app_prefs: invalid weather list");
    return E_INVALID_ARGUMENT;
  }

  status_t rv = prv_lock_mutex_and_open_file();
  if (rv == S_SUCCESS) {
    rv = settings_file_set(&s_watch_app_prefs_db.settings_file, key, key_len, val, val_len);

    // Cache the data we just set if it was for the Reminders app
    const int expected_reminder_app_prefs_size = sizeof(SerializedReminderAppPrefs);
    if ((rv == S_SUCCESS) &&
        is_valid_reminder_key &&
        (val_len == expected_reminder_app_prefs_size)) {
      s_watch_app_prefs_db.is_cached_reminder_app_prefs_valid = true;
      memcpy(&s_watch_app_prefs_db.cached_reminder_app_prefs, val,
             (size_t)expected_reminder_app_prefs_size);
    }
    prv_close_file_and_unlock_mutex();
  }

  return rv;
}

int watch_app_prefs_db_get_len(const uint8_t *key, int key_len) {
  int len = 0;
  if (prv_lock_mutex_and_open_file() == S_SUCCESS) {
    len = settings_file_get_len(&s_watch_app_prefs_db.settings_file, key, key_len);
    prv_close_file_and_unlock_mutex();
  }

  return len;
}

status_t watch_app_prefs_db_read(const uint8_t *key, int key_len, uint8_t *val_out,
                                 int val_out_len) {
  if (!val_out) {
    return E_INVALID_ARGUMENT;
  }

  status_t rv = prv_lock_mutex_and_open_file();
  if (rv == S_SUCCESS) {
    rv = settings_file_get(&s_watch_app_prefs_db.settings_file, key, key_len, val_out, val_out_len);
    prv_close_file_and_unlock_mutex();
  }

  return rv;
}

status_t watch_app_prefs_db_delete(const uint8_t *key, int key_len) {
  status_t rv = prv_lock_mutex_and_open_file();
  if (rv == S_SUCCESS) {
    rv = settings_file_delete(&s_watch_app_prefs_db.settings_file, key, key_len);
    prv_close_file_and_unlock_mutex();
  }

  return rv;
}

status_t watch_app_prefs_db_flush(void) {
  mutex_lock_recursive(s_watch_app_prefs_db.mutex);
  status_t rv = pfs_remove(SETTINGS_FILE_NAME);
  mutex_unlock_recursive(s_watch_app_prefs_db.mutex);
  return rv;
}
