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

#include "fake_shared_prf_storage.h"

#include <bluetooth/bluetooth_types.h>
#include <bluetooth/sm_types.h>

static int s_prf_storage_ble_store_count;
static int s_prf_storage_ble_delete_count;
static int s_prf_storage_bt_classic_store_count;
static int s_prf_storage_bt_classic_platform_bits_count;
static int s_prf_storage_bt_classic_delete_count;

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Test functions

void fake_shared_prf_storage_reset_counts(void) {
  s_prf_storage_ble_store_count = 0;
  s_prf_storage_ble_delete_count = 0;
  s_prf_storage_bt_classic_store_count = 0;
  s_prf_storage_bt_classic_platform_bits_count = 0;
  s_prf_storage_bt_classic_delete_count = 0;
}

int fake_shared_prf_storage_get_ble_store_count(void) {
  return s_prf_storage_ble_store_count;
}

int fake_shared_prf_storage_get_ble_delete_count(void) {
  return s_prf_storage_ble_delete_count;
}

int fake_shared_prf_storage_get_bt_classic_store_count(void) {
  return s_prf_storage_bt_classic_store_count;
}

int fake_shared_prf_storage_get_bt_classic_platform_bits_count(void) {
  return s_prf_storage_bt_classic_platform_bits_count;
}

int fake_shared_prf_storage_get_bt_classic_delete_count(void) {
  return s_prf_storage_bt_classic_delete_count;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Custom Local Device Name

bool shared_prf_storage_get_local_device_name(char *local_device_name_out, size_t max_size) {
  return false;
}

void shared_prf_storage_set_local_device_name(char *local_device_name) {
  return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! BLE Root Keys

bool shared_prf_storage_get_root_key(SMRootKeyType key_type, SM128BitKey *key_out) {
  return false;
}

void shared_prf_storage_set_root_keys(SM128BitKey *keys_in) {
  return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! BLE Pairing Data

bool shared_prf_storage_get_ble_pairing_data(SMPairingInfo *pairing_info_out,
                                             char *name_out, bool *requires_address_pinning_out,
                                             uint8_t *flags) {
  return false;
}


void shared_prf_storage_store_ble_pairing_data(const SMPairingInfo *pairing_info,
                                               char *name, bool requires_address_pinning,
                                               uint8_t flags) {
  s_prf_storage_ble_delete_count++;
  s_prf_storage_ble_store_count++;
}

void shared_prf_storage_erase_ble_pairing_data(void) {
  s_prf_storage_ble_delete_count++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! BT Classic Pairing Data

bool shared_prf_storage_get_bt_classic_pairing_data(BTDeviceAddress *addr_out,
                                                    char *device_name_out,
                                                    SM128BitKey *link_key_out,
                                                    uint8_t *platform_bits) {
  return false;
}

void shared_prf_storage_store_bt_classic_pairing_data(BTDeviceAddress *addr,
                                                      char *device_name,
                                                      SM128BitKey *link_key,
                                                      uint8_t platform_bits) {
  s_prf_storage_bt_classic_delete_count++;
  s_prf_storage_bt_classic_store_count++;
}

void shared_prf_storage_store_platform_bits(uint8_t platform_bits) {
  s_prf_storage_bt_classic_platform_bits_count++;
}

void shared_prf_storage_erase_bt_classic_pairing_data(void) {
  s_prf_storage_bt_classic_delete_count++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Getting Started Is Complete

bool shared_prf_storage_get_getting_started_complete(void) {
  return true;
}

void shared_prf_storage_set_getting_started_complete(bool set) {
  return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Factory Reset

void shared_prf_storage_wipe_all(void) {
  return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//! Pinned Address

bool shared_prf_storage_get_ble_pinned_address(BTDeviceAddress *address_out) {
  return false;
}

//! Stores the new BLE Pinned Address in the shared storage.
void shared_prf_storage_set_ble_pinned_address(const BTDeviceAddress *address) {
}
