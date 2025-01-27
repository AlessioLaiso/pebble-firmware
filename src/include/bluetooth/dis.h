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

#include "util/attributes.h"

// The reason the headers that define these lengths aren't included is because this header
// is included by the various number of bt_driver implementations. They don't know what "mfg"
// is, etc.
// NOTE: These sizes are asserted in a .c file to be in sync with the FW
#define MODEL_NUMBER_LEN  (10) // MFG_HW_VERSION_SIZE + 1
#define MANUFACTURER_LEN  (18) // sizeof("Pebble Technology")
#define SERIAL_NUMBER_LEN (13) // MFG_SERIAL_NUMBER_SIZE + 1
#define FW_REVISION_LEN   (32) // FW_METADATA_VERSION_TAG_BYTES)
#define SW_REVISION_LEN   (6)  // Fmt: xx.xx\0

typedef struct PACKED DisInfo {
  char model_number[MODEL_NUMBER_LEN];
  char manufacturer[MANUFACTURER_LEN];
  char serial_number[SERIAL_NUMBER_LEN];
  char fw_revision[FW_REVISION_LEN];
  char sw_revision[SW_REVISION_LEN];
} DisInfo;
