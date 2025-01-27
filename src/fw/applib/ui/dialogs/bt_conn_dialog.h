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

#include "applib/event_service_client.h"
#include "applib/ui/dialogs/simple_dialog.h"

#include <stdint.h>
#include <stdbool.h>

typedef void (*BtConnDialogResultHandler)(bool connected, void *context);

typedef struct {
  SimpleDialog dialog;
  EventServiceInfo pebble_app_event_sub;
  BtConnDialogResultHandler connected_handler;
  void *context;
  char *text_buffer;
  bool owns_buffer;
} BtConnDialog;

//! @internal
//! Wrapper around a \ref SimpleDialog for showing a bluetooth connection event.
//! @param bt_dialog Pointer to the \ref BtConnDialog to push
//! @param handler The \ref BtConnDialogResultHandler to be called when
//!     bluetooth is reconnected.
//! @param context The context to pass to the handler
void bt_conn_dialog_push(BtConnDialog *bt_dialog, BtConnDialogResultHandler handler, void *context);

//! @internal
//! Allocates a \ref BtConnDialog on the heap and returns it
//! @return Pointer to a \ref BtConnDialog
BtConnDialog *bt_conn_dialog_create(void);

//! @internal
//! Initializes a \ref BtConnDialog
//! @param bt_dialog Pointer to the \ref BtConnDialog to initialize
void bt_conn_dialog_init(BtConnDialog *bt_dialog, char *text_buffer, size_t buffer_size);
