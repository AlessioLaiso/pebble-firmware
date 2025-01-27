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

#include "process_management/app_install_manager_private.h"
#include "process_management/app_order_storage.h"
#include "services/common/comm_session/session.h"
#include "system/hexdump.h"
#include "system/logging.h"
#include "system/passert.h"
#include "system/status_codes.h"
#include "util/uuid.h"

#include <stdbool.h>
#include <string.h>

//! @file app_order_endpoint.c
//! App Order Endpoint
//!
//! There is only 1 way to use this endpoint

//! \code{.c}
//! 0x01 <uint8_t num_uuids>
//! <16-byte UUID_1>
//! ...
//! <16-byte UUID_N>
//! \endcode

//! AppOrder Endpoint ID
static const uint16_t APP_ORDER_ENDPOINT_ID = 0xabcd;

typedef enum {
  APP_ORDER_CMD = 0x01,
} AppOrderCommand;

typedef enum {
  APP_ORDER_RES_SUCCESS = 0x01,
  APP_ORDER_RES_FAILURE = 0x02,
  APP_ORDER_RES_INVALID = 0x03,
  APP_ORDER_RES_RETRY_LATER = 0x04,
} AppOrderResponse;

typedef struct {
  CommSession *session;
  uint8_t result;
} ResponseInfo;

static void prv_send_result(CommSession *session, uint8_t result) {
  PBL_LOG(LOG_LEVEL_DEBUG, "Sending result of %d", result);
  comm_session_send_data(session, APP_ORDER_ENDPOINT_ID, (uint8_t*)&result, sizeof(result),
                         COMM_SESSION_DEFAULT_TIMEOUT);
}

static void prv_handle_app_order_msg(CommSession *session, const uint8_t *data, uint32_t length) {
  // call write order function, then fire an event for app install manager to tell the launcher
  // to throw everything away or at least don't overwrite the data please.
  uint8_t num_uuids = data[0];

  if (num_uuids != (length / UUID_SIZE)) {
    PBL_LOG(LOG_LEVEL_DEBUG, "invalid length, num_uuids does not match with the length of message");
    prv_send_result(session, APP_ORDER_RES_INVALID);
  }

  write_uuid_list_to_file((const Uuid *)&data[1], num_uuids);
  prv_send_result(session, APP_ORDER_RES_SUCCESS);
}

void app_order_protocol_msg_callback(CommSession *session, const uint8_t* data, size_t length) {
  // header includes APP_ORDER_CMD and a num_uuids uint8_t
  const uint8_t header_len = sizeof(AppOrderCommand) + sizeof(uint8_t);

  // Ensure it is a valid message. There is a list of UUID's after the header.
  if ((length % UUID_SIZE) != header_len) {
    PBL_LOG(LOG_LEVEL_DEBUG, "invalid length, (length - header_len) not multiple of 16");
    prv_send_result(session, APP_ORDER_RES_INVALID);
    return;
  }

  switch (data[0]) {
    case APP_ORDER_CMD:
      PBL_LOG(LOG_LEVEL_DEBUG, "Got APP_ORDER message");
      prv_handle_app_order_msg(session, &data[1], length - 1);
      break;
    default:
      PBL_LOG(LOG_LEVEL_ERROR, "Invalid message received, first byte is %u", data[0]);
      prv_send_result(session, APP_ORDER_RES_FAILURE);
      break;
  }
}
