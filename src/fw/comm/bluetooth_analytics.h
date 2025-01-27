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
#include <stdint.h>

#include "bluetooth/gap_le_connect.h"
#include "services/common/analytics/analytics_event.h"

typedef struct SlaveConnEventStats SlaveConnEventStats;

void bluetooth_analytics_get_param_averages(uint16_t *params);

void bluetooth_analytics_handle_param_update_failed(void);

void bluetooth_analytics_handle_connection_params_update(const BleConnectionParams *params);

void bluetooth_analytics_handle_connect(
    const BTDeviceInternal *peer_addr, const BleConnectionParams *conn_params);

void bluetooth_analytics_handle_disconnect(bool local_is_master);

void bluetooth_analytics_handle_encryption_change(void);

void bluetooth_analytics_handle_no_intent_for_connection(void);

void bluetooth_analytics_handle_ble_pairing_request(void);

void bluetooth_analytics_handle_bt_classic_pairing_request(void);

void bluetooth_analytics_handle_ble_pairing_error(uint32_t error);

void bluetooth_analytics_handle_bt_classic_pairing_error(uint32_t error);

void bluetooth_analytics_handle_connection_disconnection_event(
    AnalyticsEvent type, uint8_t reason, const BleRemoteVersionInfo *vers_info);

void bluetooth_analytics_handle_put_bytes_stats(bool successful, uint8_t type, uint32_t total_size,
                                                uint32_t elapsed_time_ms,
                                                const SlaveConnEventStats *orig_stats);

void bluetooth_analytics_handle_get_bytes_stats(uint8_t type, uint32_t total_size,
                                                uint32_t elapsed_time_ms,
                                                const SlaveConnEventStats *orig_stats);

void bluetooth_analytics_ble_mic_error(uint32_t num_sequential_mic_errors);
