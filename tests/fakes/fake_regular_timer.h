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

#include <stdbool.h>

#include "services/common/regular_timer.h"

static ListNode s_seconds_callbacks;
static ListNode s_minutes_callbacks;

static bool prv_callback_registered_filter(ListNode *found_node, void *data) {
  return (found_node == (ListNode *)data);
}

void regular_timer_add_multisecond_callback(RegularTimerInfo* cb, uint16_t seconds) {
  if (!list_find(&s_seconds_callbacks, prv_callback_registered_filter, &cb->list_node)) {
    list_append(&s_seconds_callbacks, &cb->list_node);
  }
}

void regular_timer_add_seconds_callback(RegularTimerInfo* cb) {
  regular_timer_add_multisecond_callback(cb, 1);
}

void regular_timer_add_multiminute_callback(RegularTimerInfo* cb, uint16_t minutes) {
  if (!list_find(&s_minutes_callbacks, prv_callback_registered_filter, &cb->list_node)) {
    list_append(&s_minutes_callbacks, &cb->list_node);
  }
}

void regular_timer_add_minutes_callback(RegularTimerInfo* cb) {
  regular_timer_add_multiminute_callback(cb, 1);
}

bool regular_timer_is_scheduled(RegularTimerInfo *cb) {
  return (list_find(&s_seconds_callbacks, prv_callback_registered_filter, &cb->list_node) ||
          list_find(&s_minutes_callbacks, prv_callback_registered_filter, &cb->list_node));
}

bool regular_timer_pending_deletion(RegularTimerInfo *cb) {
  return cb->pending_delete;
}

bool regular_timer_remove_callback(RegularTimerInfo* cb) {
  bool timer_removed = false;
  if (regular_timer_is_scheduled(cb)) {
    list_remove(&cb->list_node, NULL, NULL);
    timer_removed = true;
  }

  return timer_removed;
}

void fake_regular_timer_trigger(RegularTimerInfo *timer) {
  if (regular_timer_is_scheduled(timer)) {
    timer->cb(timer->cb_data);
  }
}
