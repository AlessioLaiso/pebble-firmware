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

#include "applib/ui/status_bar_layer.h"
#include "util/list.h"
#include "resource/resource_ids.auto.h"
#include "resource/resource.h"

#include "clar.h"

// Fakes
////////////////////////////////////
#include "fake_fonts.h"

// Stubs
////////////////////////////////////
#include "stubs_app_state.h"
#include "stubs_app_timer.h"
#include "stubs_applib_resource.h"
#include "stubs_compiled_with_legacy2_sdk.h"
#include "stubs_event_service_client.h"
#include "stubs_heap.h"
#include "stubs_logging.h"
#include "stubs_passert.h"
#include "stubs_pbl_malloc.h"
#include "stubs_pebble_tasks.h"
#include "stubs_print.h"
#include "stubs_process_manager.h"
#include "stubs_resources.h"
#include "stubs_syscalls.h"
#include "stubs_ui_window.h"
#include "stubs_unobstructed_area.h"
#include "stubs_window_stack.h"

// Stubs
////////////////////////////////////
GContext *graphics_context_get_current_context(void) {
  return NULL;
}

// Setup
////////////////////////////////////

ResourceCallbackHandle resource_watch(ResAppNum app_num, uint32_t resource_id,
                                      ResourceChangedCallback callback, void* data) {
  return (ResourceCallbackHandle) { 0 };
}

// Helpers
////////////////////////////////////

#define cl_assert_status_bar_height(status_bar) \
  do { \
    cl_assert(status_bar.layer.frame.size.h == STATUS_BAR_LAYER_HEIGHT); \
    cl_assert(status_bar.layer.bounds.size.h == STATUS_BAR_LAYER_HEIGHT); \
  } while (0);

// Tests
////////////////////////////////////

//! The height of the status bar should always be locked to STATUS_BAR_LAYER_HEIGHT.
//! Make sure that after marking dirty, it is always reset to STATUS_BAR_LAYER_HEIGHT.
void test_status_bar_layer__modify_height(void) {
  StatusBarLayer status_bar;
  status_bar_layer_init(&status_bar);

  cl_assert_status_bar_height(status_bar);

  GRect frame = status_bar.layer.frame;
  GRect bounds = status_bar.layer.bounds;

  frame.size.h = STATUS_BAR_LAYER_HEIGHT - 5;
  layer_set_frame(&status_bar.layer, &frame);
  cl_assert_status_bar_height(status_bar);

  bounds.size.h = STATUS_BAR_LAYER_HEIGHT + 5;
  layer_set_bounds(&status_bar.layer, &bounds);
  cl_assert_status_bar_height(status_bar);
}

