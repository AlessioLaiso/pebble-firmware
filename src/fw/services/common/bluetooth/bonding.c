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

#include "comm/ble/gap_le_connection.h"
#include "comm/ble/gap_le_device_name.h"
#include "comm/bt_lock.h"

#include "services/common/bluetooth/bluetooth_persistent_storage.h"
#include "services/common/bluetooth/local_addr.h"
#include "system/logging.h"

#include <bluetooth/bonding_sync.h>
#include <bluetooth/bluetooth_types.h>
#include <bluetooth/sm_types.h>

void bt_driver_cb_handle_create_bonding(const BleBonding *bonding,
                                        const BTDeviceAddress *addr) {
#if !defined(PLATFORM_TINTIN)
  PBL_LOG(LOG_LEVEL_INFO, "Creating new bonding for "BT_DEVICE_ADDRESS_FMT,
          BT_DEVICE_ADDRESS_XPLODE(bonding->pairing_info.identity.address));
#endif
  const bool should_pin_address = bonding->should_pin_address;
  if (should_pin_address) {
    bt_local_addr_pin(&bonding->pinned_address);
  }
  const uint8_t flags = bonding->flags;
  if (flags) {
    PBL_LOG(LOG_LEVEL_INFO, "flags: 0x02%x", flags);
  }
  BTBondingID bonding_id = bt_persistent_storage_store_ble_pairing(&bonding->pairing_info,
                                                                   bonding->is_gateway, NULL,
                                                                   should_pin_address,
                                                                   flags);
  bt_lock();
  {
    GAPLEConnection *connection = gap_le_connection_by_addr(addr);
    if (connection) {
      // Associate the connection with the bonding:
      connection->bonding_id = bonding_id;
      connection->is_gateway = bonding->is_gateway;

      if (!connection->is_gateway) {
        PBL_LOG(LOG_LEVEL_DEBUG, "New bonding is not gateway?");
      }

      // Request device name. iOS returns an "anonymized" device name before encryption, like
      // "iPhone" and only returns the real name i.e. "Martijn's iPhone" after encryption is set up.
      gap_le_device_name_request(&connection->device);
    } else {
      PBL_LOG(LOG_LEVEL_ERROR, "Couldn't find connection for bonding!");
    }
  }
  bt_unlock();
}
