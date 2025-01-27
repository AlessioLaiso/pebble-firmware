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

#include "peek.h"

#include "applib/ui/animation.h"
#include "applib/ui/window.h"
#include "services/normal/timeline/timeline_layout.h"

typedef struct PeekLayout {
  TimelineLayoutInfo info;
  TimelineLayout *timeline_layout;
  TimelineItem *item;
} PeekLayout;

typedef struct TimelinePeek {
  Window window;
  Layer layout_layer;
  PeekLayout *peek_layout;
  Animation *animation; //!< Currently running animation
  bool exists; //!< Whether there exists an item to show in peek.
  bool started; //!< Whether the item has started.
  bool enabled; //!< Whether to persistently show or hide the peek.
  bool visible; //!< Whether the peek is visible or not.
  bool first; //!< Whether the item is the first item in Timeline.
  bool removing_concurrent; //!< Whether the removing concurrent animation is occurring.
  bool future_empty; //!< Whether Timeline future is empty.
} TimelinePeek;

#if UNITTEST
TimelinePeek *timeline_peek_get_peek(void);
#endif
