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

#include "tick_timer.h"

#include "kernel/events.h"
#include "drivers/rtc.h"
#include "services/common/regular_timer.h"
#include "process_management/app_manager.h"
#include "system/logging.h"
#include "system/passert.h"

static uint16_t s_num_subscribers;

static void timer_tick_event_publisher(void* data) {
  PebbleEvent e = {
    .type = PEBBLE_TICK_EVENT,
    .clock_tick.tick_time = rtc_get_time(),
  };

  event_put(&e);
}

static RegularTimerInfo s_tick_timer_info = {
  .cb = &timer_tick_event_publisher
};

void tick_timer_add_subscriber(PebbleTask task) {
  ++s_num_subscribers;
  if (s_num_subscribers == 1) {
    PBL_LOG(LOG_LEVEL_DEBUG, "starting tick timer");
    regular_timer_add_seconds_callback(&s_tick_timer_info);
  }
}

void tick_timer_remove_subscriber(PebbleTask task) {
  PBL_ASSERTN(s_num_subscribers > 0);
  --s_num_subscribers;
  if (s_num_subscribers == 0) {
    PBL_LOG(LOG_LEVEL_DEBUG, "stopping tick timer");
    regular_timer_remove_callback(&s_tick_timer_info);
  }
}
