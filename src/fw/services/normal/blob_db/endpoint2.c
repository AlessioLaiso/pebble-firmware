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

#include "api.h"
#include "sync.h"
#include "endpoint_private.h"

#include "services/common/comm_session/session.h"
#include "services/common/comm_session/session_send_buffer.h"
#include "services/common/analytics/analytics.h"
#include "system/logging.h"
#include "system/passert.h"
#include "system/status_codes.h"
#include "system/hexdump.h"
#include "util/attributes.h"
#include "util/net.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

//! BlobDB Endpoint ID
static const uint16_t BLOB_DB2_ENDPOINT_ID = 0xb2db;

//! Message Length Constants
static const uint8_t DIRTY_DATABASES_LENGTH = 2;
static const uint8_t START_SYNC_LENGTH = 3;
static const uint8_t WRITE_RESPONSE_LENGTH = 3;
static const uint8_t WRITEBACK_RESPONSE_LENGTH = 3;
static const uint8_t SYNC_DONE_RESPONSE_LENGTH = 3;

static bool s_b2db_accepting_messages;

T_STATIC BlobDBToken prv_new_token(void) {
  static BlobDBToken next_token = 1; // 0 token should be avoided
  return next_token++;
}

static const uint8_t *prv_read_token_and_response(const uint8_t *iter, BlobDBToken *out_token,
                                                  BlobDBResponse *out_response) {
  *out_token = *(BlobDBToken *)iter;
  iter += sizeof(BlobDBToken);
  *out_response = *(BlobDBResponse *)iter;
  iter += sizeof(BlobDBResponse);

  return iter;
}

T_STATIC void prv_send_response(CommSession *session, uint8_t *response,
                                uint8_t response_length) {
  comm_session_send_data(session, BLOB_DB2_ENDPOINT_ID, response, response_length,
                         COMM_SESSION_DEFAULT_TIMEOUT);
}

static void prv_handle_get_dirty_databases(CommSession *session,
                                           const uint8_t *data,
                                           uint32_t length) {
  if (length < DIRTY_DATABASES_LENGTH) {
    PBL_LOG(LOG_LEVEL_ERROR, "Got a dirty databases with an invalid length: %"PRIu32"", length);
    return;
  }

  struct PACKED DirtyDatabasesResponseMsg {
    BlobDBCommand cmd;
    BlobDBToken token;
    BlobDBResponse result;
    uint8_t num_ids;
    BlobDBId db_ids[NumBlobDBs];
  } response = {
    .cmd = BLOB_DB_COMMAND_DIRTY_DBS_RESPONSE,
    .token = *(BlobDBToken *)data,
    .result = BLOB_DB_SUCCESS,
  };


  blob_db_get_dirty_dbs(response.db_ids, &response.num_ids);
  // we don't want to send the extra bytes in response.db_ids
  int num_empty_ids = NumBlobDBs - response.num_ids;

  prv_send_response(session, (uint8_t *) &response, sizeof(response) - num_empty_ids);
}

static void prv_handle_start_sync(CommSession *session,
                                  const uint8_t *data,
                                  uint32_t length) {
  if (length < START_SYNC_LENGTH) {
    PBL_LOG(LOG_LEVEL_ERROR, "Got a start sync with an invalid length: %"PRIu32"", length);
    return;
  }

  struct PACKED StartSyncResponseMsg {
    BlobDBCommand cmd;
    BlobDBToken token;
    BlobDBResponse result;
  } response = {
    .cmd = BLOB_DB_COMMAND_START_SYNC_RESPONSE,
  };

  BlobDBId db_id;
  endpoint_private_read_token_db_id(data, &response.token, &db_id);

  status_t rv = blob_db_sync_db(db_id);
  switch (rv) {
    case S_SUCCESS:
    case S_NO_ACTION_REQUIRED:
      response.result = BLOB_DB_SUCCESS;
      break;
    case E_INVALID_ARGUMENT:
      response.result = BLOB_DB_INVALID_DATABASE_ID;
      break;
    case E_BUSY:
      response.result = BLOB_DB_TRY_LATER;
      break;
    default:
      response.result = BLOB_DB_GENERAL_FAILURE;
      break;
  }

  prv_send_response(session, (uint8_t *)&response, sizeof(response));
}

static void prv_handle_wb_write_response(const uint8_t *data,
                                         uint32_t length) {
  // read token and response code
  BlobDBToken token;
  BlobDBResponse response_code;
  prv_read_token_and_response(data, &token, &response_code);

  BlobDBSyncSession *sync_session = blob_db_sync_get_session_for_token(token);
  if (sync_session) {
    if (response_code == BLOB_DB_SUCCESS) {
      blob_db_sync_next(sync_session);
    } else {
      blob_db_sync_cancel(sync_session);
    }
  } else {
    // No session
    PBL_LOG(LOG_LEVEL_WARNING, "received blob db wb response with an invalid token: %d", token);
  }
}

static void prv_handle_write_response(CommSession *session,
                                      const uint8_t *data,
                                      uint32_t length) {
  if (length < WRITE_RESPONSE_LENGTH) {
    PBL_LOG(LOG_LEVEL_ERROR, "Got a write response with an invalid length: %"PRIu32"", length);
    return;
  }

  prv_handle_wb_write_response(data, length);
}

static void prv_handle_wb_response(CommSession *session,
                                   const uint8_t *data,
                                   uint32_t length) {
  if (length < WRITEBACK_RESPONSE_LENGTH) {
    PBL_LOG(LOG_LEVEL_ERROR, "Got a writeback response with an invalid length: %"PRIu32"", length);
    return;
  }

  prv_handle_wb_write_response(data, length);
}

static void prv_handle_sync_done_response(CommSession *session,
                                          const uint8_t *data,
                                          uint32_t length) {
  if (length < SYNC_DONE_RESPONSE_LENGTH) {
    PBL_LOG(LOG_LEVEL_ERROR, "Got a sync done response with an invalid length: %"PRIu32"", length);
    return;
  }

  // read token and response code
  BlobDBToken token;
  BlobDBResponse response_code;
  prv_read_token_and_response(data, &token, &response_code);

  if (response_code != BLOB_DB_SUCCESS) {
    PBL_LOG(LOG_LEVEL_ERROR, "Sync Done response error: %d", response_code);
  }
}

static void prv_send_error_response(CommSession *session,
                                    BlobDBCommand cmd,
                                    const uint8_t *data,
                                    BlobDBResponse response_code) {
  struct PACKED ErrorResponseMsg {
    BlobDBCommand cmd;
    BlobDBToken token;
    BlobDBResponse result;
  } response = {
    .cmd = cmd | RESPONSE_MASK,
    .token = *(BlobDBToken *)data,
    .result = response_code,
  };

  prv_send_response(session, (uint8_t *)&response, sizeof(response));
}

static void prv_blob_db_msg_decode_and_handle(
    CommSession *session, BlobDBCommand cmd, const uint8_t *data, size_t data_length) {
  switch (cmd) {
    case BLOB_DB_COMMAND_DIRTY_DBS:
      PBL_LOG(LOG_LEVEL_DEBUG, "Got DIRTY DBs");
      prv_handle_get_dirty_databases(session, data, data_length);
      break;
    case BLOB_DB_COMMAND_START_SYNC:
      PBL_LOG(LOG_LEVEL_DEBUG, "Got SYNC");
      prv_handle_start_sync(session, data, data_length);
      break;
    case BLOB_DB_COMMAND_WRITE_RESPONSE:
      PBL_LOG(LOG_LEVEL_DEBUG, "WRITE Response");
      prv_handle_write_response(session, data, data_length);
      break;
    case BLOB_DB_COMMAND_WRITEBACK_RESPONSE:
      PBL_LOG(LOG_LEVEL_DEBUG, "WRITEBACK Response");
      prv_handle_wb_response(session, data, data_length);
      break;
    case BLOB_DB_COMMAND_SYNC_DONE_RESPONSE:
      PBL_LOG(LOG_LEVEL_DEBUG, "SYNC DONE Response");
      prv_handle_sync_done_response(session, data, data_length);
      break;
    default:
      PBL_LOG(LOG_LEVEL_ERROR, "Invalid BlobDB2 message received, cmd is %u", cmd);
      prv_send_error_response(session, cmd, data, BLOB_DB_INVALID_OPERATION);
      break;
  }
}

static uint16_t prv_send_write_writeback(BlobDBCommand cmd,
                                         BlobDBId db_id,
                                         time_t last_updated,
                                         const uint8_t *key,
                                         int key_len,
                                         const uint8_t *val,
                                         int val_len) {
  struct PACKED WritebackMetadata {
    BlobDBCommand cmd;
    BlobDBToken token;
    BlobDBId db_id;
    uint32_t last_updated;
  } writeback_metadata = {
    .cmd = cmd,
    .token = prv_new_token(),
    .db_id = db_id,
    .last_updated = last_updated,
  };

  size_t writeback_length = sizeof(writeback_metadata) +
                            sizeof(uint8_t) /* key length size*/ +
                            key_len +
                            sizeof(uint16_t) /* val length size */ +
                            val_len;

  SendBuffer *sb = comm_session_send_buffer_begin_write(comm_session_get_system_session(),
                                                        BLOB_DB2_ENDPOINT_ID,
                                                        writeback_length,
                                                        COMM_SESSION_DEFAULT_TIMEOUT);
  if (sb) {
    comm_session_send_buffer_write(sb, (uint8_t *)&writeback_metadata, sizeof(writeback_metadata));
    comm_session_send_buffer_write(sb, (uint8_t *)&key_len, sizeof(uint8_t));
    comm_session_send_buffer_write(sb, key, key_len);
    comm_session_send_buffer_write(sb, (uint8_t *)&val_len, sizeof(uint16_t));
    comm_session_send_buffer_write(sb, val, val_len);
    comm_session_send_buffer_end_write(sb);
  }

  return writeback_metadata.token;
}

BlobDBToken blob_db_endpoint_send_write(BlobDBId db_id,
                                        time_t last_updated,
                                        const void *key,
                                        int key_len,
                                        const void *val,
                                        int val_len) {
  BlobDBToken token = prv_send_write_writeback(BLOB_DB_COMMAND_WRITE, db_id, last_updated,
                                               key, key_len, val, val_len);

  return token;
}

BlobDBToken blob_db_endpoint_send_writeback(BlobDBId db_id,
                                            time_t last_updated,
                                            const void *key,
                                            int key_len,
                                            const void *val,
                                            int val_len) {
  BlobDBToken token = prv_send_write_writeback(BLOB_DB_COMMAND_WRITEBACK, db_id, last_updated,
                                               key, key_len, val, val_len);

  return token;
}

void blob_db_endpoint_send_sync_done(BlobDBId db_id) {
  struct PACKED SyncDoneMsg {
    BlobDBCommand cmd;
    BlobDBToken token;
    BlobDBId db_id;
  } msg = {
    .cmd = BLOB_DB_COMMAND_SYNC_DONE,
    .token = prv_new_token(),
    .db_id = db_id,
  };

  PBL_LOG(LOG_LEVEL_DEBUG, "Sending sync done for db: %d", db_id);

  comm_session_send_data(comm_session_get_system_session(),
                         BLOB_DB2_ENDPOINT_ID,
                         (uint8_t *)&msg,
                         sizeof(msg),
                         COMM_SESSION_DEFAULT_TIMEOUT);
}

void blob_db2_protocol_msg_callback(CommSession *session, const uint8_t* data, size_t length) {
  PBL_ASSERT_TASK(PebbleTask_KernelBackground);

  analytics_inc(ANALYTICS_DEVICE_METRIC_BLOB_DB_EVENT_COUNT, AnalyticsClient_System);

  // Each BlobDB message is required to have at least a Command and a Token
  static const uint8_t MIN_RAW_DATA_LEN = sizeof(BlobDBCommand) + sizeof(BlobDBToken);
  if (length < MIN_RAW_DATA_LEN) {
    // We don't send failure responses for too short messages in endpoint2
    PBL_LOG(LOG_LEVEL_ERROR, "Got a blob_db2 message that was too short, len: %zu", length);
    return;
  }

  const BlobDBCommand cmd = *data;
  data += sizeof(BlobDBCommand); // fwd to message contents
  const size_t data_length = length - sizeof(BlobDBCommand);

  if (!s_b2db_accepting_messages) {
    prv_send_error_response(session, cmd, data, BLOB_DB_TRY_LATER);
    return;
  }

  prv_blob_db_msg_decode_and_handle(session, cmd, data, data_length);
}

void blob_db2_set_accepting_messages(bool enabled) {
  s_b2db_accepting_messages = enabled;
}
