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

#include "weather_app_warning_dialog.h"

#include "applib/ui/app_window_stack.h"
#include "applib/ui/dialogs/dialog.h"
#include "applib/ui/dialogs/expandable_dialog.h"
#include "kernel/pbl_malloc.h"

typedef struct WeatherAppWarningDialogData {
  WeatherAppWarningDialogDismissedCallback dismissed_cb;
} WeatherAppWarningDialogData;

static void prv_warning_dialog_unload(void *context) {
  WeatherAppWarningDialogData *data = context;
  if (data->dismissed_cb) {
    data->dismissed_cb();
  }
  task_free(data);
}

static void prv_warning_dialog_select_handler(ClickRecognizerRef recognizer, void *context) {
  ExpandableDialog *expandable_dialog = context;
  expandable_dialog_pop(expandable_dialog);
}

WeatherAppWarningDialog *weather_app_warning_dialog_push(const char *localized_string,
    WeatherAppWarningDialogDismissedCallback dismissed_cb) {
  WeatherAppWarningDialogData *data = task_zalloc_check(sizeof(WeatherAppWarningDialogData));
  ExpandableDialog *expandable_dialog = expandable_dialog_create("Weather - warning dialog");

  Dialog *dialog = expandable_dialog_get_dialog(expandable_dialog);
  dialog_set_destroy_on_pop(dialog, false);
  dialog_set_icon(dialog, RESOURCE_ID_GENERIC_WARNING_TINY);
  dialog_set_text(dialog, localized_string);
  const DialogCallbacks callbacks = {
    .unload = prv_warning_dialog_unload,
  };
  dialog_set_callbacks(dialog, &callbacks, data);

  expandable_dialog_show_action_bar(expandable_dialog, true);
  expandable_dialog_set_select_action(expandable_dialog, RESOURCE_ID_ACTION_BAR_ICON_CHECK,
                                      prv_warning_dialog_select_handler);

  data->dismissed_cb = dismissed_cb;
  app_expandable_dialog_push(expandable_dialog);

  return expandable_dialog;
}
