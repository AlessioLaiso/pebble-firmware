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

#define FILE_LOG_COLOR LOG_COLOR_BLUE

#include "pp_ble_control.h"
#include "services/common/bluetooth/pairability.h"
#include "system/logging.h"
#include "system/passert.h"
#include "util/attributes.h"

typedef enum {
  // Values 0 - 3 are deprecated, do not use.
  BLEControlCommandTypeSetDiscoverablePairable = 4,
} BLEControlCommandType;

typedef struct PACKED {
  uint8_t opcode;
  bool discoverable_pairable;
  uint16_t duration;
} BLEControlCommandSetDiscoverablePairable;

// -----------------------------------------------------------------------------
//! Handler for the "Set Discoverable & Pairable" command
static void prv_handle_set_discoverable_pairable(
    BLEControlCommandSetDiscoverablePairable *cmd_data) {
  bt_pairability_use_ble_for_period(cmd_data->duration);
  PBL_LOG(LOG_LEVEL_INFO, "Set Discoverable Pairable: %u, %u",
          cmd_data->discoverable_pairable, cmd_data->duration);
}

// -----------------------------------------------------------------------------
//! Pebble protocol handler for the BLE control endpoint
void pp_ble_control_protocol_msg_callback(
    CommSession* session, const uint8_t *data, unsigned int length) {
  PBL_ASSERT_RUNNING_FROM_EXPECTED_TASK(PebbleTask_KernelBackground);

  if (length < sizeof(BLEControlCommandSetDiscoverablePairable)) {
    PBL_LOG(LOG_LEVEL_WARNING, "Invalid pp_ble_control_protocol_msg_callback message: %d", length);
    return;
  }

  const uint8_t opcode = *(const uint8_t *) data;
  switch (opcode) {
    case 0 ... 3:
      PBL_LOG(LOG_LEVEL_INFO, "Deprecated & unsupported opcode: %u", opcode);
      break;

    case BLEControlCommandTypeSetDiscoverablePairable: {
      BLEControlCommandSetDiscoverablePairable *cmd_data =
          (BLEControlCommandSetDiscoverablePairable *)data;
      prv_handle_set_discoverable_pairable(cmd_data);
      break;
    }
    default:
      PBL_LOG(LOG_LEVEL_DEBUG, "Unknown opcode %u", opcode);
      break;
  }
}
