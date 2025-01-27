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

#pragma once

#include "applib/graphics/gtypes.h"
#include "services/common/battery/battery_monitor.h"

typedef enum BatteryUIWarningLevel {
  BatteryUIWarningLevel_None = -1,
  BatteryUIWarningLevel_Low,
  BatteryUIWarningLevel_VeryLow
} BatteryUIWarningLevel;

//! Process the incoming battery state change notification
void battery_ui_handle_state_change_event(PreciseBatteryChargeState new_state);

//! Handle shutting down the watch.
//!
//! If the watch is plugged in at the time, a "shut down while charging" UI is
//! displayed to give the user feedback on the charge state. Standby will be
//! entered once the watch is unplugged.
void battery_ui_handle_shut_down(void);

//! Show the 'battery charging' modal dialog
void battery_ui_display_plugged(void);

//! Show the 'battery charged' modal dialog
void battery_ui_display_fully_charged(void);

//! Show the 'battery critical' modal dialog
void battery_ui_display_warning(uint32_t percent, BatteryUIWarningLevel warning_level);

//! Dismiss the battery UI modal window.
void battery_ui_dismiss_modal(void);
