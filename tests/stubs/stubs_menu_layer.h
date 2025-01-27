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

#include "applib/ui/menu_layer.h"
#include "util/attributes.h"

void WEAK menu_cell_basic_draw(GContext* ctx, const Layer *cell_layer, const char *title,
                               const char *subtitle, GBitmap *icon) {}

void WEAK menu_cell_title_draw(GContext* ctx, const Layer *cell_layer, const char *title) {}

void WEAK menu_cell_basic_header_draw(GContext* ctx, const Layer *cell_layer, const char *title) {}

void WEAK menu_layer_init(MenuLayer *menu_layer, const GRect *frame) {}

MenuLayer* WEAK menu_layer_create(GRect frame) {
  return NULL;
}

void WEAK menu_layer_deinit(MenuLayer* menu_layer) {}

void WEAK menu_layer_destroy(MenuLayer* menu_layer) {}

Layer* WEAK menu_layer_get_layer(const MenuLayer *menu_layer) {
  return NULL;
}

ScrollLayer* WEAK menu_layer_get_scroll_layer(const MenuLayer *menu_layer) {
  return NULL;
}

void WEAK menu_layer_set_callbacks(MenuLayer *menu_layer, void *callback_context,
                                   const MenuLayerCallbacks *callbacks) {}

void WEAK menu_layer_set_callbacks__deprecated(MenuLayer *menu_layer, void *callback_context,
                                               const MenuLayerCallbacks *callbacks) {}

void WEAK menu_layer_set_click_config_onto_window(MenuLayer *menu_layer, struct Window *window) {}

void WEAK menu_layer_set_selected_next(MenuLayer *menu_layer, bool up, MenuRowAlign scroll_align,
                                       bool animated) {}

void WEAK menu_layer_set_selected_index(MenuLayer *menu_layer, MenuIndex index,
                                        MenuRowAlign scroll_align, bool animated) {}

void WEAK menu_layer_reload_data(MenuLayer *menu_layer) {}
