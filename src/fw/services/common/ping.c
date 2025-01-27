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

#include "applib/ui/dialogs/dialog.h"
#include "applib/ui/dialogs/simple_dialog.h"
#include "console/prompt.h"
#include "drivers/battery.h"
#include "kernel/event_loop.h"
#include "kernel/ui/modals/modal_manager.h"
#include "services/common/accel_manager.h"
#include "services/common/analytics/analytics.h"
#include "services/common/comm_session/session.h"
#include "services/common/system_task.h"
#include "system/logging.h"
#include "util/attributes.h"
#include "util/net.h"

#include <inttypes.h>

#define PING_ENDPOINT 2001
#define PING_MIN_PERIOD_SECS  (60 * 60)    // 1 hour

static time_t s_last_send_time;
static bool s_is_ping_kernel_bg_callback_scheduled;


// ---------------------------------------------------------------------------------------------------------
// Ping Pong structures
typedef struct PACKED {
  uint8_t  cmd;
  uint32_t cookie;
} PingMsgHeader;

typedef struct PACKED {
  PingMsgHeader hdr;
} PingMsgV1;

typedef struct PACKED {
  PingMsgHeader hdr;
  uint8_t idle;   // Optional
} PingMsgV2;

typedef struct PACKED {
  PingMsgHeader hdr;
} PongMsg;


static void prv_send_ping_kernel_bg_cb(void *unused) {
  CommSession *system_session = comm_session_get_system_session();
  if (system_session) {
    // Are we idle?
    bool idle = (battery_is_usb_connected() || accel_is_idle());

    PingMsgV2 ping_msg = (PingMsgV2) {
      .hdr = {
        .cmd = 0,
        .cookie = htonl(42)
      },
      .idle = idle
    };
    bool success = comm_session_send_data(system_session, PING_ENDPOINT,
                                          (const uint8_t *) &ping_msg, sizeof(ping_msg),
                                          COMM_SESSION_DEFAULT_TIMEOUT);
    if (success) {
      s_last_send_time = rtc_get_time();
      analytics_inc(ANALYTICS_DEVICE_METRIC_PING_SENT_COUNT, AnalyticsClient_System);
    }
    PBL_LOG(LOG_LEVEL_DEBUG, "Sent ping idle=%d, success=%d", (int)idle, (int)success);
  }

  s_is_ping_kernel_bg_callback_scheduled = false;
}

// ------------------------------------------------------------------------------------------------------------
// If a ping is due to be sent, send it.
// bt_lock() is held by the caller
void ping_send_if_due(void) {
  if (s_is_ping_kernel_bg_callback_scheduled) {
    return;
  }

  // Only send if we haven't sent within the last PING_MIN_PERIOD_SECS
  time_t current_time = rtc_get_time();
  if (current_time < s_last_send_time + PING_MIN_PERIOD_SECS) {
    return;
  }

  // Offload to KernelBG, because we cannot use comm_session_send_data() with bt_lock held.
  system_task_add_callback(prv_send_ping_kernel_bg_cb, NULL);
  s_is_ping_kernel_bg_callback_scheduled = true;
}

static void prv_push_window(void *data) {
  SimpleDialog *s_dialog = simple_dialog_create("Ping");
  Dialog *dialog = simple_dialog_get_dialog(s_dialog);

  dialog_set_background_color(dialog, GColorCobaltBlue);
  dialog_set_text_color(dialog, GColorWhite);
  dialog_set_text(dialog, "Ping");

  WindowStack *stack = modal_manager_get_window_stack(ModalPriorityGeneric);
  simple_dialog_push(s_dialog, stack);
}

void ping_protocol_msg_callback(CommSession *session, const uint8_t* data, size_t length) {
  PingMsgV1 *ping = (PingMsgV1 *)data;
  switch (ping->hdr.cmd) {
  case 0:
  {
    if (length != sizeof(PingMsgV1) && length != sizeof(PingMsgV2) /* idle boolean is optional */) {
      PBL_LOG(LOG_LEVEL_ERROR, "Invalid Ping, l=%u", length);
      return;
    }

    // Ping message
    uint32_t cookie = ntohl(ping->hdr.cookie);
    PBL_LOG(LOG_LEVEL_DEBUG, "Ping c=%"PRIu32"", cookie);
    launcher_task_add_callback(prv_push_window, NULL);

    // Send the pong response
    PongMsg pong = {
      .hdr = {
        .cmd = 1,
        .cookie = htonl(cookie)
      }
    };
    comm_session_send_data(session, PING_ENDPOINT, (uint8_t *)&pong, sizeof(pong), COMM_SESSION_DEFAULT_TIMEOUT);
    break;
  }

  case 1:
    if (length != sizeof(PongMsg)) {
      PBL_LOG(LOG_LEVEL_ERROR, "Invalid Pong, l=%u", length);
      return;
    }

    PongMsg *pong = (PongMsg *)data;
    PBL_LOG(LOG_LEVEL_DEBUG, "Pong c=%"PRIu32, ntohl(pong->hdr.cookie));
    analytics_inc(ANALYTICS_DEVICE_METRIC_PONG_RECEIVED_COUNT, AnalyticsClient_System);
    break;

  default:
    PBL_LOG(LOG_LEVEL_ERROR, "Invalid message received. First byte is %u", ping->hdr.cmd);
    break;
  }
}


// Serial Commands
//////////////////////////////////////////////////////////////////////
void command_ping_send(void) {
  // Override last send time
  s_last_send_time = 0;
  ping_send_if_due();
}

