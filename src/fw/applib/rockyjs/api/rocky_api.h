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

#include "stdbool.h"
#include "jerry-api.h"

// generic callback per API, e.g. to (de-) initialize
typedef void (*RockyAPIHandler)(void);

// callback to let APIs know when a caller registers an event rocky.on(event_name, handler)
// return true, if you are interested in the given event so that the pair will be stored
typedef bool (*RockyEventedAPIAddHandler)(const char *event_name, jerry_value_t handler);

// callback to let APIs know when a caller unregisters an event rocky.off(event_name, handler)
typedef void (*RockyEventedAPIRemoveHandler)(const char *event_name, jerry_value_t handler);

typedef struct {
  RockyAPIHandler init;
  RockyAPIHandler deinit;

  // responds to .on('someevent', f)
  RockyEventedAPIAddHandler add_handler;
  // responds to .off('someevent', f);
  RockyEventedAPIRemoveHandler remove_handler;
} RockyGlobalAPI;


void rocky_api_watchface_init(void);
void rocky_api_deinit(void);
