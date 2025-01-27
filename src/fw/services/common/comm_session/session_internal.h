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

#include "drivers/rtc.h"

#include "session_receive_router.h"
#include "session_transport.h"

#include "services/common/regular_timer.h"
#include "util/list.h"

#include <stdbool.h>

typedef struct SessionSendQueueJob SessionSendQueueJob;

//! Data structure representing a Pebble Protocol communication session.
//! There can be multiple. For example, with the iAP transport, the Pebble app has a session and
//! 3rd party apps share another separate session as well. With PPoGATT, the Pebble app has its own
//! session, but each 3rd party app has its own session as well.
typedef struct CommSession {
  ListNode node;

  //! The underlying transport responsible for actually sending and receiving the Pebble Protocol
  //! data. This can be SPP, iAP (see ispp.c), PPoGATT (see ppogatt.c) or QEMU (qemu_transport.c).
  Transport *transport;

  //! Set of function pointers that the session uses to call back to the transport.
  const TransportImplementation *transport_imp;

  //! True if a Kernel BG callback has been scheduled to call transport_imp->send_next()
  bool is_send_next_call_pending;

  //! True if the session is a system session (connected to the Pebble mobile app).
  TransportDestination destination;

  // Extensions supported by the mobile endpoint, see
  // https://pebbletechnology.atlassian.net/wiki/pages/viewpage.action?pageId=491698
  CommSessionCapability protocol_capabilities;

  //! The send queue of this session. See session_send_queue.c
  SessionSendQueueJob *send_queue_head;

  ReceiveRouter recv_router;

  //! Absolute number of ticks since session opened.
  RtcTicks open_ticks;
} CommSession;
