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

#include "util/circular_cache.h"

#include "util/assert.h"
#include "util/math.h"

#include <string.h>

void circular_cache_init(CircularCache* c, uint8_t* buffer, size_t item_size,
    int total_items, Comparator compare_cb) {
  UTIL_ASSERT(c);
  UTIL_ASSERT(buffer);
  UTIL_ASSERT(item_size);
  UTIL_ASSERT(compare_cb);

  *c = (CircularCache) {
    .cache = buffer,
    .next_erased_item_idx = 0,
    .item_size = item_size,
    .total_items = total_items,
    .compare_cb = compare_cb
  };
}

void circular_cache_set_item_destructor(CircularCache *c, CircularCacheItemDestructor destructor) {
  c->item_destructor = destructor;
}

static uint8_t* prv_get_item_at_index(CircularCache* c, int index) {
  return c->cache + index * c->item_size;
}

bool circular_cache_contains(CircularCache* c, void* theirs) {
  return (circular_cache_get(c, theirs) != NULL);
}

void *circular_cache_get(CircularCache* c, void* theirs) {
  for (int i = 0; i < c->total_items; ++i) {
    uint8_t* ours = prv_get_item_at_index(c, i);
    if (c->compare_cb(ours, theirs) == 0) {
      return ours;
    }
  }
  return NULL;
}

void circular_cache_push(CircularCache* c, void* new_item) {
  uint8_t* old_item = prv_get_item_at_index(c, c->next_erased_item_idx);
  if (c->item_destructor) {
    c->item_destructor(old_item);
  }
  memcpy(old_item, new_item, c->item_size);
  ++c->next_erased_item_idx;
  UTIL_ASSERT(c->next_erased_item_idx <= c->total_items);
  if (c->next_erased_item_idx == c->total_items) {
    c->next_erased_item_idx = 0;
  }
}

void circular_cache_fill(CircularCache *c, uint8_t *item) {
  // If you need item_destructor and fill, add an index pointing to the oldest item for destruction
  UTIL_ASSERT(c->item_destructor == NULL);
  for (int i = 0; i < c->total_items; ++i) {
    memcpy(c->cache + i * c->item_size, item, c->item_size);
  }
}

void circular_cache_flush(CircularCache *c) {
  // This assumes that the user of this library can differentiate empty entries in the cache from
  // valid ones.
  for (int i = 0; i < c->total_items; i++) {
    uint8_t* item = prv_get_item_at_index(c, i);
    if (c->item_destructor) {
      c->item_destructor(item);
    }
  }
  c->next_erased_item_idx = 0;
}
