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

#include "peek_layer.h"
#include "pin_window.h"
#include "timeline_model.h"
#include "timeline_layer.h"

#include "applib/ui/action_menu_layer.h"
#include "applib/ui/ui.h"
#include "popups/timeline/timeline_item_layer.h"
#include "process_management/pebble_process_md.h"
#include "process_state/app_state/app_state.h"
#include "services/common/evented_timer.h"
#include "services/normal/timeline/timeline.h"

typedef enum {
  TimelineAppStateNone = 0,
  TimelineAppStatePeek,
  TimelineAppStateHidePeek,
  TimelineAppStateStationary,
  TimelineAppStateUpDown,
  TimelineAppStateFarDayHidePeek,
  TimelineAppStateShowDaySeparator,
  TimelineAppStateDaySeparator,
  TimelineAppStateHideDaySeparator,
  TimelineAppStatePushCard,
  TimelineAppStateCard,
  TimelineAppStatePopCard,
  TimelineAppStateNoEvents,
  TimelineAppStateInactive,
  TimelineAppStateExit,
} TimelineAppState;

typedef struct {
  TimelineDirection direction;
  bool launch_into_pin; //!< Launch to a pin specified by `pin_id`.
  bool stay_in_list_view; //!< Whether to stay in list view or launch into the detail view.
  Uuid pin_id;
} TimelineArgs;

typedef struct {
  // Windows
  Window timeline_window;
  TimelinePinWindow pin_window;

  // Layers
  TimelineLayer timeline_layer;
  PeekLayer peek_layer;

  EventServiceInfo blobdb_event_info;
  EventServiceInfo focus_event_info;

  EventedTimerID inactive_timer_id; //!< To go back to watchface after inactivity
  EventedTimerID intro_timer_id; //!< To perform the intro animation after a peek
  EventedTimerID day_separator_timer_id; //!< To hide the day separator after a moment

  TimelineModel timeline_model;

  Animation *current_animation;

  TimelineAppState state;

  bool launch_into_deep_pin; //!< Whether we launched directly into a pin that isn't the first
  bool in_pin_view; //!< Whether we're in pin view
} TimelineAppData;

Animation *timeline_animate_back_from_card(void);

const PebbleProcessMd *timeline_get_app_info(void);
const PebbleProcessMd *timeline_past_get_app_info(void);
