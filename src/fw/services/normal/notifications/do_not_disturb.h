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

#include "util/attributes.h"
#include "kernel/events.h"
#include "services/normal/notifications/alerts_preferences.h"

#include <inttypes.h>
#include <stdbool.h>

typedef enum DoNotDisturbScheduleType {
  WeekdaySchedule,
  WeekendSchedule,
  NumDNDSchedules,
} DoNotDisturbScheduleType;

typedef struct PACKED DoNotDisturbSchedule {
  uint8_t from_hour;
  uint8_t from_minute;
  uint8_t to_hour;
  uint8_t to_minute;
} DoNotDisturbSchedule;

typedef enum ManualDNDFirstUseSource {
  ManualDNDFirstUseSourceActionMenu = 0,
  ManualDNDFirstUseSourceSettingsMenu
} ManualDNDFirstUseSource;

//! The Do Not Disturb service is meant for internal use only. Clients should use the Alerts
//! Service to determine how/when the user can be notified.

//! DND (Quiet Time) Activation Modes
//! Manual - Allows the user to quickly put the watch into an active DND mode. It
//! overrides other DND activation modes if toggled off. Once the watch comes out of scheduled DND,
//! manual DND automatically turns off.
//! Smart DND (Calendar Aware) - Leverages the calendar service to determine if an event is ongoing
//! and automatically puts the watch into an Active DND Mode
//! Scheduled DND - Allows the user to specify a daily schedule for when the DND should be in active
//! mode. Once coming out of a schedule, if the Manual DND is enabled, it disables that setting.

//! @return True if DND is in effect, false if not.
bool do_not_disturb_is_active(void);

//! Manual DND is a simple on / off switch for DND,
//! which works along side automatic modes (scheduled and calendar aware ('smart')

//! @return True if DND has been manually enabled, false if not.
bool do_not_disturb_is_manually_enabled(void);

//! Set the current manual DND state
void do_not_disturb_set_manually_enabled(bool enable);

//! Toggle the current manual DND state. Provide the source from which it was toggled.
//! Toggling from the settings menu simply toggles the Manual DND setting
//! Toggling from a notification action menu sets the Manual DND setting to opposite of the current
//! DND active state.
void do_not_disturb_toggle_manually_enabled(ManualDNDFirstUseSource source);

bool do_not_disturb_is_smart_dnd_enabled(void);

void do_not_disturb_toggle_smart_dnd(void);

void do_not_disturb_get_schedule(DoNotDisturbScheduleType type, DoNotDisturbSchedule *schedule_out);

void do_not_disturb_set_schedule(DoNotDisturbScheduleType type, DoNotDisturbSchedule *schedule);

bool do_not_disturb_is_schedule_enabled(DoNotDisturbScheduleType type);

void do_not_disturb_set_schedule_enabled(DoNotDisturbScheduleType type, bool scheduled);

void do_not_disturb_toggle_scheduled(DoNotDisturbScheduleType type);

void do_not_disturb_init(void);

void do_not_disturb_handle_clock_change(void);

void do_not_disturb_handle_calendar_event(PebbleCalendarEvent *e);

void do_not_disturb_manual_toggle_with_dialog(void);

#if UNITTEST
#include "services/common/new_timer/new_timer.h"
TimerID get_dnd_timer_id(void);
void set_dnd_timer_id(TimerID id);
#endif
