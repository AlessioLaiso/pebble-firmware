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

#include "locale.h"
#include "applib/i18n.h"
#include "process_state/app_state/app_state.h"

void locale_init_app_locale(LocaleInfo *info) {
  strncpy(info->app_locale_strings, "en_US", ISO_LOCALE_LENGTH);
  strncpy(info->app_locale_time, "en_US", ISO_LOCALE_LENGTH);
}

static void prv_update_locale(char *locale, const char *new) {
  if (new) {
    strncpy(locale, new, ISO_LOCALE_LENGTH);
  }
}

char *pbl_setlocale(int category, const char *locale) {
  LocaleInfo *info = app_state_get_locale_info();
  if (locale == NULL) {
    switch (category) {
      case LC_ALL:
        return info->app_locale_strings;
      case LC_TIME:
        return info->app_locale_time;
      default:
        return NULL;
    }
  } else if (*locale == '\0') {
    locale = app_get_system_locale();
  }

  switch (category) {
    case LC_ALL:
      prv_update_locale(info->app_locale_strings, locale);
      prv_update_locale(info->app_locale_time, locale);
      return info->app_locale_strings;
    case LC_TIME:
      prv_update_locale(info->app_locale_time, locale);
      return info->app_locale_time;
    default:
      /* not implemented */
      break;
  }

  return NULL;
}

static const struct lconv pbl_lconv = {
  ".", "", "", "", "", "", "", "", "", "",
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255, 255, 255,
  255, 255
};

struct lconv *pbl_localeconv_r(struct _reent *data) {
  return (struct lconv *)&pbl_lconv;
}

