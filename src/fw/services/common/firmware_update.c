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

#include "services/common/firmware_update.h"

#include "apps/core_apps/progress_ui_app.h"
#include "flash_region/flash_region.h"
#include "kernel/event_loop.h"
#include "kernel/system_message.h"
#include "kernel/ui/modals/modal_manager.h"
#include "process_management/app_manager.h"
#include "process_management/app_manager.h"
#include "services/common/battery/battery_monitor.h"
#include "services/common/system_task.h"
#include "services/runlevel.h"
#include "system/bootbits.h"
#include "system/logging.h"
#include "system/passert.h"
#include "system/reset.h"
#include "util/math.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

// The legacy firmware UI breaks firmware and resources into 50% chunks. In reality since these
// parts are not of equal sizes, one of these '50%' blocks will take longer than the
// other. Additionally, iOS/Android and the watch are not in sync over what this should look like.
//
// The new UI allows the phone to give us more info up front about how much data will be
// transmitted and also cleanly drive a re-start of the UI to a non-0 percentage if the FW update
// is being resumed. Newer implementations should this! (See PBL-42130)

static SemaphoreHandle_t s_firmware_update_semaphore;
static bool s_is_recovery_fw = false;
static FirmwareUpdateStatus s_update_status = FirmwareUpdateStopped;

typedef struct LegacyFwUpdateCompletionStatus {
  uint32_t recovery_percent_completion;
  uint32_t resource_percent_completion;
  uint32_t firmware_percent_completion;
} LegacyFwUpdateCompletionStatus;

typedef struct FwUpdateCompletionStatus {
  uint32_t bytes_transferred;
  uint32_t total_size;
} FwUpdateCompletionStatus;

typedef struct {
  bool use_legacy_mode;
  union {
    FwUpdateCompletionStatus status;
    LegacyFwUpdateCompletionStatus legacy_status;
  };
} FwUpdateCurrentCompletionStatus;

static FwUpdateCurrentCompletionStatus s_current_completion_status =  { 0 };

//
// Start handlers for legacy percentage status handling. Someday, we can hopefully
// remove them outright and it should just involve deleting these routines
//

static bool prv_legacy_firmware_update_get_percent_progress(uint32_t *progress) {
  if (!s_current_completion_status.use_legacy_mode) {
    return false;
  }

  LegacyFwUpdateCompletionStatus *status = &s_current_completion_status.legacy_status;
  if (s_is_recovery_fw) {
    *progress = MIN(100, status->recovery_percent_completion);
  } else {
    *progress =
        MIN(100, (status->resource_percent_completion + status->firmware_percent_completion) / 2);
  }

  return true;
}

static bool prv_legacy_handle_progress(PebblePutBytesEvent *event) {
  if (!s_current_completion_status.use_legacy_mode) {
    return false;
  }

  LegacyFwUpdateCompletionStatus *status = &s_current_completion_status.legacy_status;
  switch (event->object_type) {
    case ObjectFirmware:
      status->firmware_percent_completion = event->progress_percent;
      break;

    case ObjectSysResources:
      status->resource_percent_completion = event->progress_percent;
      break;

    case ObjectRecovery:
      status->recovery_percent_completion = event->progress_percent;
      break;

    default:
      PBL_LOG(LOG_LEVEL_ERROR, "Unexpected Object type %u", event->object_type);
      break;
  }

  return true;
}

static bool prv_legacy_completion_status_init(PebbleSystemMessageEvent *event) {
  if (event->type != PebbleSystemMessageFirmwareUpdateStartLegacy) {
    return false;
  }

  s_current_completion_status.use_legacy_mode = true;
  LegacyFwUpdateCompletionStatus *status =
      &s_current_completion_status.legacy_status;

  *status = (LegacyFwUpdateCompletionStatus) {
    .recovery_percent_completion = 0,
    .resource_percent_completion = 0,
    .firmware_percent_completion = 0
  };

  return true;
}

// End Legacy completion handlers

bool firmware_update_is_in_progress(void) {
  return s_update_status == FirmwareUpdateRunning;
}

FirmwareUpdateStatus firmware_update_current_status(void) {
  return s_update_status;
}

void firmware_update_init(void) {
  vSemaphoreCreateBinary(s_firmware_update_semaphore);
  PBL_ASSERTN(s_firmware_update_semaphore != NULL);
}

static void prv_initialize_completion_status(PebbleSystemMessageEvent *event) {
  if (prv_legacy_completion_status_init(event)) {
    return;
  }

  s_current_completion_status.use_legacy_mode = false;
  FwUpdateCompletionStatus *status = &s_current_completion_status.status;
  *status = (FwUpdateCompletionStatus) {
    .bytes_transferred = event->bytes_transferred,
    .total_size = event->total_transfer_size
  };
}

// Initialization for a firmware update could involve an erase of 8 flash
// sectors. Worst case timing for an erase is ~5s, so let's set our timeout to
// 40s to give us some headroom.
#define FIRMWARE_TIMEOUT_MS (1000 * 40)

static FirmwareUpdateStatus prv_firmware_update_start(PebbleSystemMessageEvent *event) {
  if (battery_monitor_critical_lockout()) {
    return FirmwareUpdateCancelled;  // Disable firmware updates on low power
  }

  if (xSemaphoreTake(s_firmware_update_semaphore, 0) == pdFALSE) {
    return FirmwareUpdateStopped;
  }

  FirmwareUpdateStatus result = s_update_status;
  if (result != FirmwareUpdateRunning) {
    prv_initialize_completion_status(event);

    services_set_runlevel(RunLevel_FirmwareUpdate);
    modal_manager_pop_all();

    static const ProgressUIAppArgs s_update_args = {
      .progress_source = PROGRESS_UI_SOURCE_FW_UPDATE,
    };
    app_manager_launch_new_app(&(AppLaunchConfig) {
      .md = progress_ui_app_get_info(),
      .common.args = &s_update_args,
      .restart = true,
    });
    put_bytes_expect_init(FIRMWARE_TIMEOUT_MS);
    result = FirmwareUpdateRunning;
  }

  xSemaphoreGive(s_firmware_update_semaphore);
  return result;
}

static void prv_handle_firmware_update_start_msg(PebbleSystemMessageEvent *event) {
  FirmwareUpdateStatus result = prv_firmware_update_start(event);
  s_update_status = result;
  PBL_ASSERTN((result == FirmwareUpdateRunning) ||
              (result == FirmwareUpdateStopped) ||
              (result == FirmwareUpdateCancelled));
  system_message_send_firmware_start_response(result);
}

static void prv_firmware_update_finish(bool failed) {
  if (xSemaphoreTake(s_firmware_update_semaphore, 0) == pdFALSE) {
    return;
  }

  if (failed) {
    // If we failed, we can set it back to normal. If we succeeded, we'll reboot shortly.
    // We don't know the runlevel that was set before, so we assume it was Normal.
    services_set_runlevel(RunLevel_Normal);
  }

  s_update_status = failed ? FirmwareUpdateFailed : FirmwareUpdateStopped;

  xSemaphoreGive(s_firmware_update_semaphore);
}

unsigned int firmware_update_get_percent_progress(void) {
  if (!firmware_update_is_in_progress()) {
    return 0;
  }

  uint32_t progress = 0;
  if (prv_legacy_firmware_update_get_percent_progress(&progress)) {
    return progress;
  }

  FwUpdateCompletionStatus *status = &s_current_completion_status.status;
  return (status->bytes_transferred * 100) / status->total_size;
}

void firmware_update_event_handler(PebbleSystemMessageEvent* event) {
  switch (event->type) {
    case PebbleSystemMessageFirmwareUpdateStartLegacy:
    case PebbleSystemMessageFirmwareUpdateStart:
      prv_handle_firmware_update_start_msg(event);
      break;

    case PebbleSystemMessageFirmwareUpdateFailed:
      prv_firmware_update_finish(true /* failed */);
      break;

    case PebbleSystemMessageFirmwareUpdateComplete:
      prv_firmware_update_finish(false /* failed */);
      break;

    default:
      break;
  }
}

static void prv_handle_progress(PebblePutBytesEvent *event) {
  if (prv_legacy_handle_progress(event)) {
    return;
  }

  if (event->type != PebblePutBytesEventTypeProgress) {
    return; // Only progress events report bytes_transferred updates
  }

  FwUpdateCompletionStatus *status = &s_current_completion_status.status;
  status->bytes_transferred += event->bytes_transferred;
}

void firmware_update_pb_event_handler(PebblePutBytesEvent *event) {
  if (!firmware_update_is_in_progress()) {
    return; // not my pb transfer
  }

  switch (event->type) {
    case PebblePutBytesEventTypeStart:
      s_is_recovery_fw = (event->object_type == ObjectRecovery);
      prv_handle_progress(event);
      break;

    case PebblePutBytesEventTypeProgress:
      prv_handle_progress(event);
      break;

    case PebblePutBytesEventTypeCleanup:
      if (event->failed) {
        // exit now in case the phone is gone
        prv_firmware_update_finish(true /* failed */);
      }
      break;

    case PebblePutBytesEventTypeInitTimeout:
      PBL_LOG(LOG_LEVEL_WARNING, "Timed out waiting for putbytes request from phone");
      prv_firmware_update_finish(true /* failed */);
      break;

    default:
      break;
  }
}
