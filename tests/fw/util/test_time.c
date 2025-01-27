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

#include "util/time/time.h"

#include <clar.h>

// Fakes
///////////////////////////////////////////////////////////
#include "../../fakes/fake_rtc.h"

// Overrides
///////////////////////////////////////////////////////////
int16_t clock_get_timezone_region_id(void) {
  return rtc_get_timezone_id();
}

void clock_set_timezone_by_region_id(uint16_t region_id) {
  return;
}

// Tests
///////////////////////////////////////////////////////////

void test_time__initialize(void) {
}

void test_time__cleanup(void) {
}

void test_time__serial_distance32(void) {
  uint32_t day, hour, minute, second;

  {
    time_util_split_seconds_into_parts(1, &day, &hour, &minute, &second);
    cl_assert_equal_i(day, 0);
    cl_assert_equal_i(hour, 0);
    cl_assert_equal_i(minute, 0);
    cl_assert_equal_i(second, 1);
  }

  {
    time_util_split_seconds_into_parts(61, &day, &hour, &minute, &second);
    cl_assert_equal_i(day, 0);
    cl_assert_equal_i(hour, 0);
    cl_assert_equal_i(minute, 1);
    cl_assert_equal_i(second, 1);
  }

  {
    second = 1;

    time_util_split_seconds_into_parts((3 * (24 * 60 * 60)), &day, &hour, &minute, &second);
    cl_assert_equal_i(day, 3);
    cl_assert_equal_i(hour, 0);
    cl_assert_equal_i(minute, 0);
    cl_assert_equal_i(second, 0);
  }


  {
    time_util_split_seconds_into_parts((3 * (24 * 60 * 60)) + (2 * (60 * 60)) + (4 * 60) + 5, &day, &hour, &minute, &second);
    cl_assert_equal_i(day, 3);
    cl_assert_equal_i(hour, 2);
    cl_assert_equal_i(minute, 4);
    cl_assert_equal_i(second, 5);
  }

}
