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

#include "app_focus_service.h"

#include "event_service_client.h"
#include "process_state/app_state/app_state.h"
#include "services/common/event_service.h"

static void prv_focus_event_handler(PebbleEvent *e, void *context) {
  AppFocusState *state = app_state_get_app_focus_state();
  bool in_focus = e->app_focus.in_focus;
  if (e->type == PEBBLE_APP_WILL_CHANGE_FOCUS_EVENT &&
      state->handlers.will_focus) {
    state->handlers.will_focus(in_focus);
  } else if (e->type == PEBBLE_APP_DID_CHANGE_FOCUS_EVENT &&
             state->handlers.did_focus) {
    state->handlers.did_focus(in_focus);
  }
}

void app_focus_service_subscribe_handlers(AppFocusHandlers handlers) {
  AppFocusState *state = app_state_get_app_focus_state();

  app_focus_service_unsubscribe();
  if (handlers.did_focus) {
    // NOTE: the individual fields of state->did_focus_info are assigned
    // to instead of writing
    //     state->did_focus_info = (EventServiceInfo) { ... }
    // as the latter would zero out the ListNode embedded in the struct.
    // Doing so would corrupt the events list if the event was already
    // subscribed.
    state->did_focus_info.type = PEBBLE_APP_DID_CHANGE_FOCUS_EVENT;
    state->did_focus_info.handler = prv_focus_event_handler;
    event_service_client_subscribe(&state->did_focus_info);
  }
  if (handlers.will_focus) {
    state->will_focus_info.type = PEBBLE_APP_WILL_CHANGE_FOCUS_EVENT;
    state->will_focus_info.handler = prv_focus_event_handler;
    event_service_client_subscribe(&state->will_focus_info);
  }
  state->handlers = handlers;
}

void app_focus_service_subscribe(AppFocusHandler handler) {
  AppFocusHandlers handlers = (AppFocusHandlers) { .will_focus = handler };
  app_focus_service_subscribe_handlers(handlers);
}

void app_focus_service_unsubscribe(void) {
  AppFocusState *state = app_state_get_app_focus_state();
  if (state->handlers.will_focus) {
    event_service_client_unsubscribe(&state->will_focus_info);
  }
  if (state->handlers.did_focus) {
    event_service_client_unsubscribe(&state->did_focus_info);
  }
  state->handlers = (AppFocusHandlers) {};
}
