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

#include "resource/resource.h"
#include "util/attributes.h"

// 
// Definitions only for font loading and text rendering
//

// Initial version
#define FONT_VERSION_1 1
// 4 byte codepoints in offset table
#define FONT_VERSION_2 2
// feature bits: 2 or 4 byte offsets, RLE encoding
#define FONT_VERSION_3 3
#define FEATURE_OFFSET_16 (1 << 0)
#define FEATURE_RLE4      (1 << 1)

// HACK ALERT: Store the v3 FontMetaDataV3 feature bits in the top two bits of FontMetaData
// version field. We need this information at the lowest levels and can't extend FontMetaData
// for legacy support reasons.
#define FONT_VERSION(_version)           ((_version) & 0x3F)
#define HAS_FEATURE(_version, _feature)  ((_version) & (_feature))
#define VERSION_FIELD_FEATURE_OFFSET_16  (1 << 7)
#define VERSION_FIELD_FEATURE_RLE4       (1 << 6)


// There are now three versions of the FontMetaData structure: V1 (formerly known as 'legacy'), V2
// (still known as FontMetaData), and V3 (know as V3). We can't change the stack/memory usage
// until we drop support for existing applications so we can't simply use V3 as the base.
//
// The name 'FontMetaData' is retained instead of a more consistent 'FontMetaDataV2' because the
// uses of V1 and V3 are localized but 'FontMetaData' is used in many places, requiring many ugly
// changes.
typedef struct PACKED {
  uint8_t version;
  uint8_t max_height;
  uint16_t number_of_glyphs;
  uint16_t wildcard_codepoint;
  uint8_t hash_table_size;
  uint8_t codepoint_bytes;
  uint8_t size;
  uint8_t features;
} FontMetaDataV3;

typedef struct PACKED {
  uint8_t version;
  uint8_t max_height;
  uint16_t number_of_glyphs;
  uint16_t wildcard_codepoint;
  uint8_t hash_table_size;
  uint8_t codepoint_bytes;
} FontMetaData;

typedef struct PACKED {
  uint8_t version;
  uint8_t max_height;
  uint16_t number_of_glyphs;
  uint16_t wildcard_codepoint;
} FontMetaDataV1;

typedef struct {
  FontMetaData md;
  ResAppNum app_num;
  uint32_t resource_id;
} FontResource;

typedef struct {
  bool loaded;
  bool extended;
  uint8_t max_height;
  FontResource base;
  FontResource extension;
  ResourceCallbackHandle extension_changed_cb;
} FontInfo;

typedef struct PACKED {
  uint8_t hash;
  uint8_t count;
  uint16_t offset;
} FontHashTableEntry;

