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

#include "event.h"

//! Default time at which the Timeline Peek will show an event before it starts.
//! This setting is user configurable.
#define TIMELINE_PEEK_DEFAULT_SHOW_BEFORE_TIME_S (10 * SECONDS_PER_MINUTE)

//! Time at which the Timeline Peek will hide an event after it starts.
//! This settings is not user configurable.
#define TIMELINE_PEEK_HIDE_AFTER_TIME_S (10 * SECONDS_PER_MINUTE)

//! TimelinePeek event subtypes which signify the relation between now and the event timestamp
typedef enum TimelinePeekTimeType {
  TimelinePeekTimeType_None = 0,
  //! The event is next, but not immediately, specifically > show_before_time_s
  TimelinePeekTimeType_SomeTimeNext,
  //! The event will start almost immediately, specifically <= show_before_time_s, and should be
  //! presented to the user
  TimelinePeekTimeType_ShowWillStart,
  //! The event has started moments ago, specifically < TIMELINE_PEEK_HIDE_AFTER_TIME_S,
  //! and should be presented to the user
  TimelinePeekTimeType_ShowStarted,
  //! The event is ongoing and will end and has already elapsed >= TIMELINE_PEEK_HIDE_AFTER_TIME_S
  TimelinePeekTimeType_WillEnd,
} TimelinePeekTimeType;

const TimelineEventImpl *timeline_peek_get_event_service(void);

//! Sets the show before timing of timeline peek.
//! @param before_time_s The amount of time before event start the peek should be visible.
void timeline_peek_set_show_before_time(unsigned int before_time_s);
