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

#include "fake_session_send_buffer.h"

#include <stdint.h>

static SendBuffer *s_stub_send_buffer = (SendBuffer *) ~0;

SendBuffer * comm_session_send_buffer_begin_write(CommSession *session, uint16_t endpoint_id,
                                                  size_t required_free_length,
                                                  uint32_t timeout_ms) {
  if (!session) {
    return NULL;
  }
  return s_stub_send_buffer;
}

bool comm_session_send_buffer_write(SendBuffer *send_buffer, const uint8_t *data, size_t length) {
  return true;
}

void comm_session_send_buffer_end_write(SendBuffer *send_buffer) {
}

static int s_send_buffer_create_count;
static bool s_send_buffer_create_simulate_oom;

SendBuffer * comm_session_send_buffer_create(bool is_system) {
  ++s_send_buffer_create_count;
  if (s_send_buffer_create_simulate_oom) {
    return NULL;
  } else {
    return (SendBuffer *) ~0;
  }
}

static int s_send_buffer_destroy_count;

void comm_session_send_buffer_destroy(SendBuffer *sb) {
  ++s_send_buffer_destroy_count;
}

void fake_session_send_buffer_init(void) {
  s_send_buffer_create_count = 0;
  s_send_buffer_destroy_count = 0;
  s_send_buffer_create_simulate_oom = false;
}

void fake_session_send_buffer_set_simulate_oom(bool enabled) {
  s_send_buffer_create_simulate_oom = enabled;
}

SendBuffer *fake_session_send_buffer_get_buffer(void) {
  return s_stub_send_buffer;
}
