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

#include "reminders.h"

#include "drivers/rtc.h"
#include "kernel/event_loop.h"
#include "kernel/events.h"
#include "kernel/pbl_malloc.h"
#include "kernel/pebble_tasks.h"
#include "services/common/system_task.h"
#include "services/normal/blob_db/pin_db.h"
#include "services/normal/blob_db/reminder_db.h"
#include "services/normal/timeline/item.h"
#include "system/logging.h"

#include <inttypes.h>

#define INVALID_SNOOZE_DELAY 0
#define HALF_SNOOZE_END_MARK 30 // Seconds
#define CONSTANT_SNOOZE_DELAY (10 * SECONDS_PER_MINUTE) // Seconds
#define CONSTANT_SNOOZE_END_MARK (48 * MINUTES_PER_HOUR * SECONDS_PER_MINUTE) // Seconds

static TimerID s_reminder_timer;

// this reminder should stay here so the timer callback has something to refer to
static ReminderId s_next_reminder_id;

bool reminders_mark_has_reminded(ReminderId *reminder_id);

static void prv_put_reminder_event(ReminderId *reminder_id, ReminderEventType type) {
  Uuid *removed_id = kernel_malloc(sizeof(Uuid));
  if (!removed_id) {
    return;
  }

  *removed_id = *reminder_id;
  PebbleEvent event = {
    .type = PEBBLE_REMINDER_EVENT,
    .reminder = {
      .type = type,
      .reminder_id = removed_id,
    }
  };
  event_put(&event);
}

void reminders_handle_reminder_updated(const Uuid *reminder_id) {
  prv_put_reminder_event((ReminderId *)reminder_id, ReminderUpdated);
}

void reminders_handle_reminder_removed(const Uuid *reminder_id) {
  prv_put_reminder_event((ReminderId *)reminder_id, ReminderRemoved);
}

static void prv_trigger_reminder_system_task_callback(void *data) {
  ReminderId *item_id = (ReminderId *)data;

  // Mark that we are about to display the reminder
  if (!reminders_mark_has_reminded(item_id)) {
    return;
  }

  prv_put_reminder_event(item_id, ReminderTriggered);
  reminders_update_timer();
}

static void prv_new_timer_callback(void *data) {
  system_task_add_callback(prv_trigger_reminder_system_task_callback, data);
}

static status_t prv_set_timer(Reminder *item) {
  time_t now = rtc_get_time();
  uint32_t timeout_ms;
  if (item->header.timestamp <= now) {
    timeout_ms = 0;
  } else {
    timeout_ms = 1000 * (item->header.timestamp - now);
  }
  s_next_reminder_id = item->header.id;
  if (new_timer_start(s_reminder_timer, timeout_ms, prv_new_timer_callback,
                      &s_next_reminder_id, 0)) {
    PBL_LOG(LOG_LEVEL_DEBUG, "Set timer for %"PRIu32, timeout_ms);
    return S_SUCCESS;
  } else {
    PBL_LOG(LOG_LEVEL_DEBUG, "Could not set timer.");
    return E_ERROR;
  }
}

status_t reminders_update_timer(void) {
  PBL_LOG(LOG_LEVEL_DEBUG, "Attempting to update timer.");
  if (!new_timer_stop(s_reminder_timer)) {
    PBL_LOG(LOG_LEVEL_DEBUG, "Updated timer while callback running.");
    return S_SUCCESS;
  }

  TimelineItem item = {{{0}}};
  status_t rv = reminder_db_next_item_header(&item);
  if (rv == S_NO_MORE_ITEMS) {
    PBL_LOG(LOG_LEVEL_DEBUG, "No more reminders to add to queue.");
    return S_SUCCESS;
  } else if (rv) {
    return rv;
  }

  rv = prv_set_timer(&item);
  if (rv) {
    return E_ERROR;
  }
  return S_SUCCESS;
}

status_t reminders_insert(Reminder *reminder) {
  status_t rv = reminder_db_insert_item(reminder);
  return rv;
}

status_t reminders_init(void) {
  if (s_reminder_timer) {
    new_timer_delete(s_reminder_timer);
  }
  s_reminder_timer = new_timer_create();
  if (s_reminder_timer == TIMER_INVALID_ID) {
    return E_ERROR;
  } else {
    return reminders_update_timer();
  }
}

status_t reminders_delete(ReminderId *reminder_id) {
  return reminder_db_delete_item(reminder_id, true /* send_event */);
}

T_STATIC uint32_t prv_calculate_snooze_delay(TimelineItem *item) {
  time_t current_time_utc = rtc_get_time();
  time_t reminder_time_utc = item->header.timestamp;
  if (current_time_utc <= reminder_time_utc) {
    return INVALID_SNOOZE_DELAY;
  }

  uint32_t snooze_delay;

  // Get parent pin
  const TimelineItemId *parent_id = &item->header.parent_id;
  TimelineItem parent_item;
  status_t status = pin_db_get(parent_id, &parent_item);
  if (status != S_SUCCESS) {
    return INVALID_SNOOZE_DELAY;
  }

  // Snooze logic:
  // If current_time is more than HALF_SNOOZE_END_MARK before event_time, snooze for half the
  //   remaining time until the event.
  // If current_time is less than HALF_SNOOZE_END_MARK before event_time, and not more than
  //   CONSTANT_SNOOZE_END_MARK after event_time, snooze for CONSTANT_SNOOZE_DELAY.
  // If current_time is more than CONSTANT_SNOOZE_END_MARK after event_time, don't snooze.
  time_t event_time_utc = parent_item.header.timestamp;
  if (event_time_utc > current_time_utc &&
      event_time_utc - current_time_utc > HALF_SNOOZE_END_MARK) {
    // Half-time snooze
    snooze_delay = (event_time_utc - reminder_time_utc) / 2;
  } else if (current_time_utc > event_time_utc &&
             current_time_utc - event_time_utc > CONSTANT_SNOOZE_END_MARK) {
    // Stop snoozing
    snooze_delay = INVALID_SNOOZE_DELAY;
  } else {
    // Constant-time snooze
    snooze_delay = CONSTANT_SNOOZE_DELAY;
  }

  timeline_item_free_allocated_buffer(&parent_item);
  return snooze_delay;
}

bool reminders_can_snooze(Reminder *reminder) {
  return (prv_calculate_snooze_delay((TimelineItem *)reminder) > 0);
}

status_t reminders_snooze(Reminder *reminder) {
  uint32_t snooze_delay = prv_calculate_snooze_delay((TimelineItem *)reminder);
  if (snooze_delay == 0) {
    return E_INVALID_OPERATION;
  }

  // Modify reminder timestamp
  TimelineItem *item = (TimelineItem*) reminder;
  item->header.timestamp = rtc_get_time() + (time_t) snooze_delay;

  // Unset the reminded status
  item->header.reminded = false;

  // Reinsert the reminder
  return reminders_insert(reminder);
}

// only used for tests
TimerID get_reminder_timer_id(void) {
  return s_reminder_timer;
}

bool reminders_mark_has_reminded(ReminderId *reminder_id) {
  status_t rv = reminder_db_set_status_bits(reminder_id, TimelineItemStatusReminded);
  return (rv == S_SUCCESS);
}
