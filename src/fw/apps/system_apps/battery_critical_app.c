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

#include "applib/app.h"
#include "applib/graphics/graphics.h"
#include "applib/ui/window_private.h"
#include "applib/ui/app_window_stack.h"
#include "kernel/pbl_malloc.h"
#include "process_management/pebble_process_md.h"
#include "process_state/app_state/app_state.h"
#include "resource/resource_ids.auto.h"
#include "system/passert.h"

typedef struct BatteryCriticalAppData {
  Window window;
  Layer layer;
  GBitmap bitmap;
} BatteryCriticalAppData;

static void update_proc(Layer* layer, GContext* ctx) {
  BatteryCriticalAppData *app_data = app_state_get_user_data();

  GRect low_battery_bounds = {
    .origin = {
      .x = (DISP_COLS - app_data->bitmap.bounds.size.w) / 2,
      .y = (DISP_ROWS - app_data->bitmap.bounds.size.h),
    },
    .size = app_data->bitmap.bounds.size,
  };

  graphics_draw_bitmap_in_rect(ctx, &app_data->bitmap, &low_battery_bounds);
}

static void handle_init(void) {
  BatteryCriticalAppData* data = app_malloc_check(sizeof(BatteryCriticalAppData));

  gbitmap_init_with_resource(&data->bitmap, RESOURCE_ID_BATTERY_ICON_CHARGE);

  app_state_set_user_data(data);

  Window *window = &data->window;

  window_init(window, WINDOW_NAME("Battery Critical"));
  window_set_overrides_back_button(window, true);

  layer_init(&data->layer, &window_get_root_layer(&data->window)->frame);
  layer_set_update_proc(&data->layer, update_proc);
  layer_add_child(window_get_root_layer(&data->window), &data->layer);

  const bool animated = false;
  app_window_stack_push(window, animated);
}

static void handle_deinit(void) {
  BatteryCriticalAppData* app_data = app_state_get_user_data();
  gbitmap_deinit(&app_data->bitmap);
  app_free(app_data);
}

static void s_main(void) {
  handle_init();

  app_event_loop();

  handle_deinit();
}

const PebbleProcessMd* battery_critical_get_app_info() {
  static const PebbleProcessMdSystem s_app_md = {
    .common = {
      .main_func = s_main,
      .visibility = ProcessVisibilityHidden,
      // UUID: 4a71eb65-238d-4faa-b2a0-112aa910d7b4
      .uuid = {0x4a, 0x71, 0xeb, 0x65, 0x23, 0x8d, 0x4f, 0xaa, 0xb2, 0xa0, 0x11, 0x2a, 0xa9, 0x10, 0xd7, 0xb4},
    },
    .name = "Battery Critical",
    .run_level = ProcessAppRunLevelCritical,
  };
  return (const PebbleProcessMd*) &s_app_md;
}

