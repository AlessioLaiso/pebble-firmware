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

#include "kernel/pebble_tasks.h"

#include <stddef.h>
#include <stdint.h>
#include <time.h>

void smartstrap_connection_init(void);

//! Kicks the monitor which is responsible for detection of connected smartstraps and sending any
//! pending requests.
void smartstrap_connection_kick_monitor(void);

//! Called to indicate that we got valid data from the smartstrap (@see smartstrap_got_valid_data())
void smartstrap_connection_got_valid_data(void);

//! Returns the number of milliseconds since smartstrap_got_valid_data() was last called.
time_t smartstrap_connection_get_time_since_valid_data(void);

//! Returns whether or not we currently have any subscribers.
bool smartstrap_connection_has_subscriber(void);

//! Subscribes to the smartstrap. When there is at least one subscriber, we will attempt to connect
//! to the smartstrap.
void sys_smartstrap_subscribe(void);

//! Unsubscribes from the smartstrap. When nobody is subscribed, we will disconnect from the
//! smartstrap.
void sys_smartstrap_unsubscribe(void);
