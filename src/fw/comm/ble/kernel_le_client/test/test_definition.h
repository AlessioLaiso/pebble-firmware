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

#include "applib/bluetooth/ble_client.h"

typedef enum {
  TestCharacteristic_One,
  TestCharacteristic_Two,

  TestCharacteristicCount,
} TestCharacteristic;

//! Test Service UUID
static const Uuid s_test_service_uuid = {
  0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
  0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
};

//! Test Characteristic UUIDs
static const Uuid s_test_characteristic_uuids[TestCharacteristicCount] = {
  [TestCharacteristic_One] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  },

  [TestCharacteristic_Two] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
  },
};

void test_client_handle_service_discovered(BLECharacteristic *characteristics);

void test_client_invalidate_all_references(void);

void test_client_handle_service_removed(BLECharacteristic *characteristics,
                                        uint8_t num_characteristics);

bool test_client_can_handle_characteristic(BLECharacteristic characteristic);

void test_client_handle_write_response(BLECharacteristic characteristic, BLEGATTError error);

void test_client_handle_subscribe(BLECharacteristic characteristic,
                                  BLESubscription subscription_type, BLEGATTError error);

void test_client_handle_read_or_notification(BLECharacteristic characteristic, const uint8_t *value,
                                             size_t value_length, BLEGATTError error);
