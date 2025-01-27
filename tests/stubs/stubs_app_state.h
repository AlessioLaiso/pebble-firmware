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

#include "applib/app_smartstrap_private.h"
#include "applib/graphics/graphics.h"
#include "applib/ui/animation_private.h"
#include "applib/ui/click_internal.h"
#include "applib/ui/layer.h"
#include "applib/ui/window_stack_private.h"
#include "applib/unobstructed_area_service_private.h"
#include "process_state/app_state/app_state.h"
#include "services/normal/app_glances/app_glance_service.h"
#include "util/attributes.h"
#include "util/heap.h"

static Heap s_app_heap;

bool app_state_configure(MemorySegment *app_state_ram,
                         ProcessAppSDKType sdk_type,
                         int16_t obstruction_origin_y) {
  return true;
}

void app_state_init(void) {
}

void app_state_deinit(void) {
}

struct tm *app_state_get_gmtime_tm(void) {
  static struct tm gmtime_tm = {0};
  return &gmtime_tm;
}

struct tm *app_state_get_localtime_tm(void) {
  static struct tm localtime_tm = {0};
  return &localtime_tm;
}

char *app_state_get_localtime_zone(void) {
  static char localtime_zone[TZ_LEN] = {0};
  return localtime_zone;
}

LocaleInfo *app_state_get_locale_info(void) {
  return NULL;
}

GContext *s_app_state_get_graphics_context;
GContext* app_state_get_graphics_context(void) {
  return s_app_state_get_graphics_context;
}

Heap* app_state_get_heap(void) {
  return &s_app_heap;
}

static AnimationState s_stub_app_animation_state;

AnimationState* app_state_get_animation_state(void) {
  return &s_stub_app_animation_state;
}

static AnimationState s_stub_kernel_animation_state;

AnimationState* kernel_applib_get_animation_state(void) {
  return &s_stub_kernel_animation_state;
}

GBitmap* app_state_legacy2_get_2bit_framebuffer(void) {
  // Shouldn't be used, only for backwards compatibility
  return NULL;
}


static Layer *s_layer_tree_stack[LAYER_TREE_STACK_SIZE];

Layer** app_state_get_layer_tree_stack(void) {
  return s_layer_tree_stack;
}

Layer** kernel_applib_get_layer_tree_stack(void) {
  return s_layer_tree_stack;
}

static WindowStack s_window_stack;

WindowStack *app_state_get_window_stack(void) {
  return &s_window_stack;
}

static SmartstrapConnectionState s_smartstrap_state;

SmartstrapConnectionState *app_state_get_smartstrap_state(void) {
  return &s_smartstrap_state;
}

static ClickManager click_manager;

ClickManager *app_state_get_click_manager(void) {
  return &click_manager;
}

static void *s_user_data;
void app_state_set_user_data(void *data) {
  s_user_data = data;
}

void *app_state_get_user_data(void) {
  return s_user_data;
}

static RockyRuntimeContext *s_rocky_runtime_context = NULL;
static uint8_t *s_runtime_context_buffer = NULL;
void app_state_set_rocky_runtime_context(uint8_t *unaligned_buffer,
                                         RockyRuntimeContext *rocky_runtime_context) {
  s_rocky_runtime_context = rocky_runtime_context;
  s_runtime_context_buffer = unaligned_buffer;
}

uint8_t *app_state_get_rocky_runtime_context_buffer(void) {
  return s_runtime_context_buffer;
}

RockyRuntimeContext *app_state_get_rocky_runtime_context(void) {
  return s_rocky_runtime_context;
}

static RockyMemoryAPIContext *s_rocky_memory_api_context = NULL;
void app_state_set_rocky_memory_api_context(RockyMemoryAPIContext *context) {
  s_rocky_memory_api_context = context;
}

RockyMemoryAPIContext *app_state_get_rocky_memory_api_context(void) {
  return s_rocky_memory_api_context;
}

UnobstructedAreaState s_stub_unobstructed_area_state;

UnobstructedAreaState *app_state_get_unobstructed_area_state(void) {
  return &s_stub_unobstructed_area_state;
}

AppGlance s_app_glance;

AppGlance *app_state_get_glance(void) {
  return &s_app_glance;
}

static bool s_text_perimeter_debugging_enabled;
bool app_state_get_text_perimeter_debugging_enabled(void) {
  return s_text_perimeter_debugging_enabled;
}

void app_state_set_text_perimeter_debugging_enabled(bool enabled) {
  s_text_perimeter_debugging_enabled = enabled;
}

TextRenderState *app_state_get_text_render_state(void) {
  static TextRenderState s_state = {0};
  return &s_state;
}

FrameBuffer *s_app_state_framebuffer;
FrameBuffer * WEAK app_state_get_framebuffer(void) {
  return s_app_state_framebuffer;
}
