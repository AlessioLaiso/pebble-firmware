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

#include "services/normal/timeline/timeline_layout.h"
#include "util/attributes.h"

void WEAK timeline_layout_init(TimelineLayout *layout, const LayoutLayerConfig *config,
                               const TimelineLayoutImpl *timeline_layout_impl) {}

void WEAK timeline_layout_time_text_update(const LayoutLayer *layout,
                                           const LayoutNodeTextDynamicConfig *config,
                                           char *buffer, bool render) {}


LayoutLayer *WEAK alarm_layout_create(const LayoutLayerConfig *config) {
  return NULL;
}

bool WEAK alarm_layout_verify(bool existing_attributes[]) {
  return false;
}

LayoutLayer *WEAK calendar_layout_create(const LayoutLayerConfig *config) {
  return NULL;
}

bool WEAK calendar_layout_verify(bool existing_attributes[]) {
  return false;
}

LayoutLayer *WEAK generic_layout_create(const LayoutLayerConfig *config) {
  return NULL;
}

bool WEAK generic_layout_verify(bool existing_attributes[]) {
  return false;
}

LayoutLayer *WEAK health_layout_create(const LayoutLayerConfig *config) {
  return NULL;
}

bool WEAK health_layout_verify(bool existing_attributes[]) {
  return false;
}

LayoutLayer *WEAK notification_layout_create(const LayoutLayerConfig *config) {
  return NULL;
}

bool WEAK notification_layout_verify(bool existing_attributes[]) {
  return false;
}

LayoutLayer *WEAK sports_layout_create(const LayoutLayerConfig *config) {
  return NULL;
}

bool WEAK sports_layout_verify(bool existing_attributes[]) {
  return false;
}

LayoutLayer *WEAK weather_layout_create(const LayoutLayerConfig *config) {
  return NULL;
}

bool WEAK weather_layout_verify(bool existing_attributes[]) {
  return false;
}
