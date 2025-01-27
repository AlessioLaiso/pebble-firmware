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

#include "do_not_disturb.h"
#include "do_not_disturb_toggle.h"

#include "applib/app_exit_reason.h"
#include "applib/ui/action_toggle.h"
#include "services/common/i18n/i18n.h"
#include "system/logging.h"

static bool prv_get_state(void *context) {
  // This toggle does not necessarily toggle Manual DND. It sets Manual DND to the opposite of DND
  // active status which in turn overrides Smart and Scheduled DND.
  return do_not_disturb_is_active();
}

static void prv_set_state(bool enabled, void *context) {
  PBL_LOG(LOG_LEVEL_DEBUG, "Manual DND toggle: %s", enabled ? "enabled" : "disabled");
  do_not_disturb_set_manually_enabled(enabled);
}

static const ActionToggleImpl s_dnd_action_toggle_impl = {
  .window_name = "DNDManualToggle",
  .prompt_icon = PBL_IF_RECT_ELSE(RESOURCE_ID_QUIET_TIME_MOUSE,
                                  RESOURCE_ID_QUIET_TIME_MOUSE_RIGHT_ALIGNED),
  .result_icon = RESOURCE_ID_QUIET_TIME_MOUSE,
  .prompt_enable_message = i18n_noop("Start Quiet Time?"),
  .prompt_disable_message = i18n_noop("End Quiet Time?"),
  .result_enable_message = i18n_noop("Quiet Time\nStarted"),
  .result_disable_message = i18n_noop("Quiet Time\nEnded"),
  .callbacks = {
    .get_state = prv_get_state,
    .set_state = prv_set_state,
  },
};

void do_not_disturb_toggle_push(ActionTogglePrompt prompt, bool set_exit_reason) {
  action_toggle_push(&(ActionToggleConfig) {
    .impl = &s_dnd_action_toggle_impl,
    .prompt = prompt,
    .set_exit_reason = set_exit_reason,
  });
}
