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

#include "interval_timer.h"

#include "drivers/rtc.h"
#include "FreeRTOS.h"

static uint64_t prv_get_curr_system_time_ms(void) {
  time_t time_s;
  uint16_t time_ms;
  rtc_get_time_ms(&time_s, &time_ms);
  return (((uint64_t)time_s) * 1000 + time_ms);
}

void interval_timer_init(IntervalTimer *timer, uint32_t min_expected_ms, uint32_t max_expected_ms,
                         uint32_t weighting_factor_inverted) {
  PBL_ASSERTN(weighting_factor_inverted != 0); // Divide by zero is not awesome

  *timer = (IntervalTimer) {
    .min_expected_ms = min_expected_ms,
    .max_expected_ms = max_expected_ms,
    .weighting_factor_inverted = weighting_factor_inverted
  };
}

//! Record a sample that marks the start/end of an interval.
//! Safe to call from an ISR.
void interval_timer_take_sample(IntervalTimer *timer) {
  portENTER_CRITICAL();
  {
    const uint64_t current_time = prv_get_curr_system_time_ms();

    // Handle the first sample specially. We don't have an interval until we have
    // 2 samples.
    if (timer->num_samples == 0) {
      timer->num_samples++;
    } else {
      const int64_t last_interval = current_time - timer->last_sample_timestamp_ms;

      // Make sure this interval is valid
      if (last_interval >= timer->min_expected_ms &&
          last_interval <= timer->max_expected_ms) {

        // It's valid! Let's roll it into our moving average

        // This is an exponential moving average.
        // https://en.wikipedia.org/wiki/Moving_average#Exponential_moving_average
        // average_now = average_previous + (weighting_factor * (new_value - average_previous))
        // Where alpha is between 0 and 1. The closer to 1 the more responsive to recent changes
        // the average is.

        if (timer->num_samples == 1) {
          // Initialize the average to the first sample we have
          timer->average_ms = last_interval;
        } else {
          timer->average_ms = timer->average_ms +
              ((last_interval - timer->average_ms) / timer->weighting_factor_inverted);
        }

        timer->num_samples++;
      }
    }

    timer->last_sample_timestamp_ms = current_time;
  }

  portEXIT_CRITICAL();
}

uint32_t interval_timer_get(IntervalTimer *timer, uint32_t *average_ms_out) {
  uint32_t num_intervals;

  portENTER_CRITICAL();
  {
    num_intervals = timer->num_samples ? timer->num_samples - 1 : 0;
    *average_ms_out = timer->average_ms;
  }
  portEXIT_CRITICAL();

  return num_intervals;
}
