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

#include <bluetooth/bluetooth_types.h>

void bt_driver_id_set_local_device_name(const char device_name[BT_DEVICE_NAME_BUFFER_SIZE]);

void bt_driver_id_copy_local_identity_address(BTDeviceAddress *addr_out);

//! Configures the local address that the BT driver should use "on-air".
//! @note This address and the identity address are different things!
//! @note bt_lock() is held when this call is made.
//! @param allow_cycling True if the controller is allowed to cycle the address (implies address
//! pinning is *not* used!)
//! @param pinned_address The address to use, or NULL for "don't care".
void bt_driver_set_local_address(bool allow_cycling,
                                 const BTDeviceAddress *pinned_address);

//! Copies a human-readable string of freeform info that uniquely identifies the Bluetooth chip.
//! Used by MFG for part tracking purposes.
//! @param[out] dest Buffer into which to copy the info.
//! @param[in] dest_size Size of dest in bytes.
void bt_driver_id_copy_chip_info_string(char *dest, size_t dest_size);

//! Generates a new private resolvable address using the current IRK (as passed with the
//! bt_driver_start() call when setting up the stack).
bool bt_driver_id_generate_private_resolvable_address(BTDeviceAddress *address_out);
