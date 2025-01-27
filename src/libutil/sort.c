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

#include <util/sort.h>

#include <stdint.h>
#include <stddef.h>

static void prv_swap(void *a, void *b, size_t elem_size) {
  uint8_t *a_ptr = (uint8_t *)a;
  uint8_t *b_ptr = (uint8_t *)b;
  for (size_t i = 0; i < elem_size; i++) {
    uint8_t tmp = *a_ptr;
    *a_ptr++ = *b_ptr;
    *b_ptr++ = tmp;
  }
}

void sort_bubble(void *array, size_t num_elem, size_t elem_size, SortComparator comp) {
  // as the number of sessions is expected to be small (<=16), and we don't seem to have a generic
  // sort implementation, we do a simple bubble sort here
  for (uint32_t i = 0; i + 1 < num_elem; i++) {
    for (uint32_t j = i + 1; j < num_elem; j++) {
      uint8_t *val1 = ((uint8_t *)array) + (i * elem_size);
      uint8_t *val2 = ((uint8_t *)array) + (j * elem_size);
      if (comp(val1, val2) > 0) {
        prv_swap(val1, val2, elem_size);
      }
    }
  }
}
