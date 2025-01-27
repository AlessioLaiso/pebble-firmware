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

#include "services/normal/activity/activity.h"
#include "system/status_codes.h"
#include "util/attributes.h"


//! Get the typical metric value for a given day.
//! If you want "typical steps" you probably want health_db_get_typical_step_averages
bool health_db_get_typical_value(ActivityMetric metric,
                                 DayInWeek day,
                                 int32_t *value_out);

//! Get the average metric value over the last month
bool health_db_get_monthly_average_value(ActivityMetric metric,
                                         int32_t *value_out);

//! Often referred to as "typical steps"
bool health_db_get_typical_step_averages(DayInWeek day,
                                         ActivityMetricAverages *averages);



//! For test / debug purposes only
bool health_db_set_typical_values(ActivityMetric metric,
                                  DayInWeek day,
                                  uint16_t *values,
                                  int num_values);

///////////////////////////////////////////
// BlobDB Boilerplate (see blob_db/api.h)
///////////////////////////////////////////

void health_db_init(void);

status_t health_db_insert(const uint8_t *key, int key_len, const uint8_t *val, int val_len);

int health_db_get_len(const uint8_t *key, int key_len);

status_t health_db_read(const uint8_t *key, int key_len, uint8_t *val_out, int val_out_len);

status_t health_db_delete(const uint8_t *key, int key_len);

status_t health_db_flush(void);
