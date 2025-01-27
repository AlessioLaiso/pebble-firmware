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

#include "airplane_mode.h"

#include "applib/app.h"
#include "applib/ui/action_toggle.h"
#include "process_management/app_manager.h"
#include "services/common/bluetooth/bluetooth_ctl.h"
#include "services/common/i18n/i18n.h"

static bool prv_get_state(void *context) {
  return bt_ctl_is_airplane_mode_on();
}

static void prv_set_state(bool enabled, void *context) {
  bt_ctl_set_airplane_mode_async(!bt_ctl_is_airplane_mode_on());
}

static const ActionToggleImpl s_airplane_mode_action_toggle_impl = {
  .window_name = "Airplane Mode Toggle",
  .prompt_icon = RESOURCE_ID_AIRPLANE,
  .result_icon = RESOURCE_ID_AIRPLANE,
  // Toggling airplane mode involves locks which can block animation, don't animate
  .result_icon_static = true,
  .prompt_enable_message = i18n_noop("Turn On Airplane Mode?"),
  .prompt_disable_message = i18n_noop("Turn Off Airplane Mode?"),
  .result_enable_message = i18n_noop("Airplane\nMode On"),
  .result_disable_message = i18n_noop("Airplane\nMode Off"),
  .callbacks = {
    .get_state = prv_get_state,
    .set_state = prv_set_state,
  },
};

static void prv_main(void) {
  action_toggle_push(&(ActionToggleConfig) {
    .impl = &s_airplane_mode_action_toggle_impl,
    .set_exit_reason = true,
  });
  app_event_loop();
}

const PebbleProcessMd *airplane_mode_toggle_get_app_info(void) {
  static const PebbleProcessMdSystem s_app_info = {
    .common = {
      .main_func = &prv_main,
      .uuid = AIRPLANE_MODE_TOGGLE_UUID,
      .visibility = ProcessVisibilityQuickLaunch,
    },
    .name = i18n_noop("Airplane Mode"),
  };
  return &s_app_info.common;
}
