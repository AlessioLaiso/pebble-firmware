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

#include "applib/graphics/graphics.h"
#include "apps/system_apps/timeline/text_node.h"

int health_util_format_hours_and_minutes(char *buffer, size_t buffer_size, int duration_s,
                                         void *i18n_owner) {
  return 0;
}

int health_util_format_minutes_and_seconds(char *buffer, size_t buffer_size, int duration_s,
                                           void *i18n_owner) {
  return 0;
}

int health_util_format_hours_minutes_seconds(char *buffer, size_t buffer_size, int duration_s,
                                             bool leading_zero, void *i18n_owner) {
  return 0;
}

void health_util_duration_to_hours_and_minutes_text_node(int duration_s, void *i18n_owner,
                                                         GFont number_font, GFont units_font,
                                                         GTextNodeContainer *container) { }

void health_util_convert_fraction_to_whole_and_decimal_part(int numerator, int denominator,
                                                            int* whole_part, int *decimal_part) { }

int health_util_format_whole_and_decimal(char *buffer, size_t buffer_size, int numerator,
                                         int denominator) {
  return 0;
}

int health_util_get_distance_factor(void) {
  return 1;
}

const char *health_util_get_distance_string(const char *miles_string, const char *km_string) {
  return miles_string;
}

int health_util_format_distance(char *buffer, size_t buffer_size, uint32_t distance_m) {
  return 0;
}

void health_util_convert_distance_to_whole_and_decimal_part(int distance_m, int *whole_part,
                                                            int *decimal_part) { }

int health_util_get_pace(int time_min, int distance_meter) {
  return 0;
}
