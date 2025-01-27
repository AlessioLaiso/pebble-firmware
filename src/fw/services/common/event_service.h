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

#include "kernel/events.h"
#include "process_management/app_manager.h"

typedef void (*EventServiceAddSubscriberCallback)(PebbleTask task);
typedef void (*EventServiceRemoveSubscriberCallback)(PebbleTask task);

//! Call once during system startup
void event_service_system_init(void);

//! Called from app task for each event type the app wants notifications for
void event_service_init(PebbleEventType type, EventServiceAddSubscriberCallback start_cb,
                        EventServiceRemoveSubscriberCallback stop_cb);
bool event_service_is_running(PebbleEventType event_type);
void event_service_handle_event(PebbleEvent *e);

//! Subscribe to an event. This can only be called from the Kernel Main task
void event_service_subscribe_from_kernel_main(PebbleSubscriptionEvent *subscription);

void event_service_handle_subscription(PebbleSubscriptionEvent *subscription);
void event_service_clear_process_subscriptions(PebbleTask task);

//! Claim a buffer. This means it won't automatically get cleaned up
//! If you claim a buffer you must free it with event_service_free_claimed_buffer()
void* event_service_claim_buffer(PebbleEvent *e);
//! This function expects the pointer returned by event_service_claim_buffer
void event_service_free_claimed_buffer(void *ref);
