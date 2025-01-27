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

#include "fake_events.h"
#include "kernel/pbl_malloc.h"

#include "freertos_types.h"
#include "projdefs.h"

static PebbleEvent s_last_pebble_event;
static uint32_t s_fake_event_count = 0;
static FakeEventCallback s_fake_event_cb = NULL;

WEAK void **fake_event_get_buffer(PebbleEvent *event) {
  switch (event->type) {
    case PEBBLE_BLE_GATT_CLIENT_EVENT:
      if (event->bluetooth.le.gatt_client.subtype == PebbleBLEGATTClientEventTypeServiceChange) {
        return (void **)(&event->bluetooth.le.gatt_client_service.info);
      }
      break;

    default:
      break; // Nothing to do!
  }
  return NULL;
}

void event_put(PebbleEvent* event) {
  fake_event_clear_last();
  s_last_pebble_event = *event;
  ++s_fake_event_count;
  if (s_fake_event_cb) {
    s_fake_event_cb(event);
  }
}

bool event_put_isr(PebbleEvent* event) {
  return false;
}

QueueHandle_t event_kernel_to_kernel_event_queue(void) {
  return (NULL);
}

BaseType_t event_queue_cleanup_and_reset(QueueHandle_t queue) {
  return pdPASS;
}

void fake_event_init(void) {
  fake_event_reset_count();
  fake_event_clear_last();
}

PebbleEvent fake_event_get_last(void) {
  return s_last_pebble_event;
}

void fake_event_clear_last(void) {
  void **buf = fake_event_get_buffer(&s_last_pebble_event);
  if (buf && *buf) {
    kernel_free(*buf);
    *buf = NULL;
  }

  s_last_pebble_event = (PebbleEvent) {};
}

void fake_event_reset_count(void) {
  s_fake_event_count = 0;
}

uint32_t fake_event_get_count(void) {
  return s_fake_event_count;
}

void fake_event_set_callback(FakeEventCallback cb) {
  s_fake_event_cb = cb;
}
