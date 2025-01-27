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

#include "ams.h"

//! AMS Service UUID - 89D3502B-0F36-433A-8EF4-C502AD55F8DC
static const Uuid s_ams_service_uuid = {
  0x89, 0xD3, 0x50, 0x2B, 0x0F, 0x36, 0x43, 0x3A,
  0x8E, 0xF4, 0xC5, 0x02, 0xAD, 0x55, 0xF8, 0xDC,
};

//! AMS Characteristic UUIDs
static const Uuid s_ams_characteristic_uuids[NumAMSCharacteristic] = {
  //! Remote Command - 9B3C81D8-57B1-4A8A-B8DF-0E56F7CA51C2
  [AMSCharacteristicRemoteCommand] = {
    0x9B, 0x3C, 0x81, 0xD8, 0x57, 0xB1, 0x4A, 0x8A,
    0xB8, 0xDF, 0x0E, 0x56, 0xF7, 0xCA, 0x51, 0xC2,
  },

  //! Entity Update - 2F7CABCE-808D-411F-9A0C-BB92BA96C102
  [AMSCharacteristicEntityUpdate] = {
    0x2F, 0x7C, 0xAB, 0xCE, 0x80, 0x8D, 0x41, 0x1F,
    0x9A, 0x0C, 0xBB, 0x92, 0xBA, 0x96, 0xC1, 0x02,
  },

  //! Entity Attribute - C6B2F38C-23AB-46D8-A6AB-A3A870BBD5D7
  [AMSCharacteristicEntityAttribute] = {
    0xC6, 0xB2, 0xF3, 0x8C, 0x23, 0xAB, 0x46, 0xD8,
    0xA6, 0xAB, 0xA3, 0xA8, 0x70, 0xBB, 0xD5, 0xD7,
  },
};
