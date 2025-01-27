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

#include "applib/graphics/gtypes.h"
#include "applib/graphics/text_render.h"

#include "clar.h"

#include "stubs_applib_resource.h"
#include "stubs_app_state.h"
#include "stubs_compiled_with_legacy2_sdk.h"
#include "stubs_heap.h"
#include "stubs_logging.h"
#include "stubs_passert.h"
#include "stubs_resources.h"
#include "stubs_syscalls.h"

GBitmap* graphics_context_get_bitmap(GContext* ctx) { return NULL; }

void graphics_context_mark_dirty_rect(GContext* ctx, GRect rect) {}

const GlyphData* text_resources_get_glyph(FontCache* font_cache, const Codepoint codepoint,
                                          FontInfo* fontinfo) { return NULL; }


extern int32_t prv_convert_1bit_addr_to_8bit_x(GBitmap *dest_bitmap, uint32_t *block_addr,
                                               int32_t y_offset);

static int32_t prv_get_8bit_x_from_1bit_x(int32_t dest_1bit_x) {
  return (((dest_1bit_x / 32) * 4)) * 8;
}

void test_text_render__convert_1bit_to_8bit_144x168(void) {
  GSize size = GSize(144, 168);
  const int row_1bit_size_words = 1 + (size.w - 1) / 32;

  GBitmap *bitmap = gbitmap_create_blank(size, GBitmapFormat8Bit);
  uintptr_t base = (uintptr_t)bitmap->addr;

  int dest_x = 0;
  int dest_y = 0;
  uint32_t *block_addr = NULL;

  block_addr = (uint32_t *)(uintptr_t)(((dest_y * row_1bit_size_words) + (dest_x / 32)) * 4);
  cl_assert_equal_i(prv_convert_1bit_addr_to_8bit_x(bitmap, block_addr, dest_y),
                    prv_get_8bit_x_from_1bit_x(dest_x));

  dest_x = 50;
  dest_y = 0;
  block_addr = (uint32_t *)(uintptr_t)(((dest_y * row_1bit_size_words) + (dest_x / 32)) * 4);
  cl_assert_equal_i(prv_convert_1bit_addr_to_8bit_x(bitmap, block_addr, dest_y),
                    prv_get_8bit_x_from_1bit_x(dest_x));

  dest_x = 0;
  dest_y = 50;
  block_addr = (uint32_t *)(uintptr_t)(((dest_y * row_1bit_size_words) + (dest_x / 32)) * 4);
  cl_assert_equal_i(prv_convert_1bit_addr_to_8bit_x(bitmap, block_addr, dest_y),
                    prv_get_8bit_x_from_1bit_x(dest_x));

  dest_x = 20;
  dest_y = 100;
  block_addr = (uint32_t *)(uintptr_t)(((dest_y * row_1bit_size_words) + (dest_x / 32)) * 4);
  cl_assert_equal_i(prv_convert_1bit_addr_to_8bit_x(bitmap, block_addr, dest_y),
                    prv_get_8bit_x_from_1bit_x(dest_x));

  gbitmap_destroy(bitmap);
}

void test_text_render__convert_1bit_to_8bit_180x180(void) {
  GSize size = GSize(180, 180);
  const int row_1bit_size_words = 1 + (size.w - 1) / 32;

  GBitmap *bitmap = gbitmap_create_blank(size, GBitmapFormat8Bit);
  uintptr_t base = (uintptr_t)bitmap->addr;

  int dest_x = 0;
  int dest_y = 0;
  uint32_t *block_addr = NULL;

  block_addr = (uint32_t *)(uintptr_t)(((dest_y * row_1bit_size_words) + (dest_x / 32)) * 4);
  cl_assert_equal_i(prv_convert_1bit_addr_to_8bit_x(bitmap, block_addr, dest_y),
                    prv_get_8bit_x_from_1bit_x(dest_x));

  dest_x = 50;
  dest_y = 0;
  block_addr = (uint32_t *)(uintptr_t)(((dest_y * row_1bit_size_words) + (dest_x / 32)) * 4);
  cl_assert_equal_i(prv_convert_1bit_addr_to_8bit_x(bitmap, block_addr, dest_y),
                    prv_get_8bit_x_from_1bit_x(dest_x));

  dest_x = 0;
  dest_y = 50;
  block_addr = (uint32_t *)(uintptr_t)(((dest_y * row_1bit_size_words) + (dest_x / 32)) * 4);
  cl_assert_equal_i(prv_convert_1bit_addr_to_8bit_x(bitmap, block_addr, dest_y),
                    prv_get_8bit_x_from_1bit_x(dest_x));

  dest_x = 20;
  dest_y = 100;
  block_addr = (uint32_t *)(uintptr_t)(((dest_y * row_1bit_size_words) + (dest_x / 32)) * 4);
  cl_assert_equal_i(prv_convert_1bit_addr_to_8bit_x(bitmap, block_addr, dest_y),
                    prv_get_8bit_x_from_1bit_x(dest_x));

  gbitmap_destroy(bitmap);
}
