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

#include "ble_hrm.h"

#include "applib/event_service_client.h"
#include "comm/ble/gap_le_connection.h"
#include "comm/ble/gap_le_slave_reconnect.h"
#include "comm/bt_lock.h"
#include "kernel/pbl_malloc.h"
#include "kernel/event_loop.h"
#include "kernel/events.h"
#include "popups/ble_hrm/ble_hrm_reminder_popup.h"
#include "popups/ble_hrm/ble_hrm_sharing_popup.h"
#include "process_management/app_manager.h"
#include "services/common/analytics/analytics.h"
#include "services/common/analytics/analytics_event.h"
#include "services/common/hrm/hrm_manager_private.h"
#include "services/common/regular_timer.h"
#include "services/normal/activity/activity.h"
#include "shell/system_app_ids.auto.h"
#include "system/logging.h"
#include "system/passert.h"

#include <bluetooth/gap_le_connect.h>
#include <bluetooth/hrm_service.h>
#include <btutil/bt_device.h>
#include <util/list.h>
#include <util/size.h>

#if CAPABILITY_HAS_BUILTIN_HRM

#define BLE_HRM_UPDATE_INTERVAL_SEC (1)

typedef struct BLEHRMSharingRequest {
  GAPLEConnection *connection;
} BLEHRMSharingRequest;

static bool s_ble_hrm_is_inited;
static int s_ble_hrm_subscription_count;
static RegularTimerInfo s_ble_hrm_timer;
static struct {
  EventServiceInfo service_info;
  HRMSessionRef manager_session;
} s_ble_hrm_session;

typedef enum {
  HrmSharingPermission_Unknown,
  HrmSharingPermission_Granted,
  HrmSharingPermission_Declined,
} HrmSharingPermission;

typedef struct BLEHRMSharingPermission {
  ListNode node;
  BTDeviceInternal device;

  //! Whether the user has confirmed that sharing HRM data to this device is permitted.
  HrmSharingPermission permission;
} BLEHRMSharingPermission;

static BLEHRMSharingPermission *s_permissions_head;

static bool prv_hw_and_sw_supports_hrm(void) {
  return (bt_driver_is_hrm_service_supported() &&
          sys_hrm_manager_is_hrm_present());
}

bool ble_hrm_is_supported_and_enabled(void) {
  return (prv_hw_and_sw_supports_hrm() &&
          activity_prefs_heart_rate_is_enabled());
}

static void prv_reset_subscriptions(void);

static bool prv_free_permission_for_each_cb(ListNode *node, void *unused) {
  kernel_free(node);
  return true;  // continue iteration
}

static void prv_free_all_permissions(void) {
  list_foreach((ListNode *)s_permissions_head, prv_free_permission_for_each_cb, NULL);
  s_permissions_head = NULL;
}

static bool prv_find_permission_by_device_filter_cb(ListNode *found_node, void *data) {
  const BTDeviceInternal *device = data;
  BLEHRMSharingPermission *permission = (BLEHRMSharingPermission *)found_node;
  return bt_device_internal_equal(device, &permission->device);
}

static BLEHRMSharingPermission *prv_find_permission_by_device(const BTDeviceInternal *device) {
  return (BLEHRMSharingPermission *)list_find((ListNode *)s_permissions_head,
                                              prv_find_permission_by_device_filter_cb,
                                              (void *)device);
}

static void prv_upsert_permission(const BTDeviceInternal *device, HrmSharingPermission permission) {
  BLEHRMSharingPermission *p = prv_find_permission_by_device(device);
  if (!p) {
    p = kernel_zalloc_check(sizeof(*p));
    p->device = *device;
    s_permissions_head = (BLEHRMSharingPermission *)list_prepend((ListNode *)s_permissions_head,
                                                                 (ListNode *)p);
  }
  p->permission = permission;
}

static HrmSharingPermission prv_get_permission_by_device(const BTDeviceInternal *device) {
  BLEHRMSharingPermission *p = prv_find_permission_by_device(device);
  if (!p) {
    return HrmSharingPermission_Unknown;
  }
  return p->permission;
}

void ble_hrm_handle_activity_prefs_heart_rate_is_enabled(bool is_enabled) {
  if (!prv_hw_and_sw_supports_hrm()) {
    return;
  }
  PBL_LOG(LOG_LEVEL_INFO, "BLE HRM sharing prefs updated: is_enabled=%u", is_enabled);

  if (!is_enabled) {
    prv_reset_subscriptions();
  }
  bt_driver_hrm_service_enable(is_enabled);
}

static bool prv_is_sharing(const GAPLEConnection *const connection) {
  return (connection->hrm_service_is_subscribed &&
          (prv_get_permission_by_device(&connection->device) == HrmSharingPermission_Granted));
}

bool ble_hrm_is_sharing_to_connection(const GAPLEConnection *const connection) {
  bt_lock_assert_held(true);
  if (!connection) {
    return false;
  }
  return prv_is_sharing(connection);
}

bool ble_hrm_is_sharing(void) {
  return (s_ble_hrm_subscription_count > 0);
}

typedef struct {
  BTDeviceInternal *next_permitted_device;
  size_t slots_left;
} CopySharingDevicesCtx;

static void prv_copy_sharing_devices_for_each_connection_cb(GAPLEConnection *connection,
                                                              void *data) {
  CopySharingDevicesCtx *ctx = data;
  if (ctx->slots_left && prv_is_sharing(connection)) {
    *ctx->next_permitted_device = connection->device;
    ++ctx->next_permitted_device;
    --ctx->slots_left;
  }
}

static size_t prv_copy_sharing_devices(BTDeviceInternal *devices_out,
                                       size_t max_devices) {
  bt_lock();
  CopySharingDevicesCtx ctx = {
    .next_permitted_device = devices_out,
    .slots_left = max_devices,
  };
  gap_le_connection_for_each(prv_copy_sharing_devices_for_each_connection_cb, &ctx);
  bt_unlock();
  return (max_devices - ctx.slots_left);
}

static void prv_ble_hrm_handle_hrm_data(PebbleEvent *e, void *context) {
  if (!s_ble_hrm_is_inited) {
    return;
  }
  if (s_ble_hrm_subscription_count == 0) {
    return;
  }
  PBL_ASSERTN(e->type == PEBBLE_HRM_EVENT);
  const PebbleHRMEvent *const hrm_event = &e->hrm;
  if (hrm_event->event_type != HRMEvent_BPM) {
    return;
  }
  const BleHrmServiceMeasurement measurement = {
    .bpm = hrm_event->bpm.bpm,
    .is_on_wrist = (hrm_event->bpm.quality > HRMQuality_NoSignal),
  };

  BTDeviceInternal sharing_to_devices[4];
  const size_t num_devices = prv_copy_sharing_devices(sharing_to_devices,
                                                      ARRAY_LENGTH(sharing_to_devices));
  bt_driver_hrm_service_handle_measurement(&measurement, sharing_to_devices, num_devices);
}

static void prv_start_hrm_kernel_main(void *unused) {
  PBL_LOG(LOG_LEVEL_INFO, "BLE HRM sharing started");
  s_ble_hrm_session.service_info = (EventServiceInfo) {
    .type = PEBBLE_HRM_EVENT,
    .handler = prv_ble_hrm_handle_hrm_data,
  };
  event_service_client_subscribe(&s_ble_hrm_session.service_info);
  s_ble_hrm_session.manager_session =
      hrm_manager_subscribe_with_callback(INSTALL_ID_INVALID, 1 /*update_interval_s*/,
                                          0 /*expire_s*/, HRMFeature_BPM, NULL, NULL);

  analytics_stopwatch_start(ANALYTICS_DEVICE_METRIC_BLE_HRM_SHARING_TIME, AnalyticsClient_System);
}

static void prv_stop_hrm_kernel_main(void *unused) {
  PBL_LOG(LOG_LEVEL_INFO, "BLE HRM sharing stopped");
  sys_hrm_manager_unsubscribe(s_ble_hrm_session.manager_session);
  event_service_client_unsubscribe(&s_ble_hrm_session.service_info);

  analytics_stopwatch_stop(ANALYTICS_DEVICE_METRIC_BLE_HRM_SHARING_TIME);
}

static void prv_execute_on_kernel_main(CallbackEventCallback cb) {
  if (pebble_task_get_current() != PebbleTask_KernelMain) {
    launcher_task_add_callback(cb, NULL);
  } else {
    cb(NULL);
  }
}

static void prv_push_sharing_request_window_kernel_main_cb(void *ctx) {
  ble_hrm_push_sharing_request_window((BLEHRMSharingRequest *)ctx);
}

static void prv_request_sharing_permission(GAPLEConnection *const connection) {
  PBL_LOG(LOG_LEVEL_INFO, "Requesting BLE HRM sharing permission");
  BLEHRMSharingRequest *const sharing_request = kernel_zalloc_check(sizeof(*sharing_request));
  sharing_request->connection = connection;
  launcher_task_add_callback(prv_push_sharing_request_window_kernel_main_cb, sharing_request);
}

static void prv_put_sharing_state_updated_event(int subscription_count) {
  // 2 purposes of this event:
  // - refresh the Settings/Bluetooth UI whenever a device (un)subscribes.
  // - present a "Sharing HRM" icon in the Settings app glance.
  PebbleEvent e = {
    .type = PEBBLE_BLE_HRM_SHARING_STATE_UPDATED_EVENT,
    .bluetooth = {
      .le = {
        .hrm_sharing_state = {
          .subscription_count = subscription_count,
        },
      },
    },
  };
  event_put(&e);
}

static void prv_reschedule_popup_timer(void);

static void prv_push_reminder_popup_kernel_main_cb(void *unused) {
  bt_lock();
  if (s_ble_hrm_subscription_count > 0) {
    // Reschedule to show again after BLE_HRM_REMINDER_POPUP_DELAY_MINS
    prv_reschedule_popup_timer();
  }
  bt_unlock();

  ble_hrm_push_reminder_popup();

  analytics_event_ble_hrm(BleHrmEventSubtype_SharingTimeoutPopupPresented);
  PBL_LOG(LOG_LEVEL_INFO, "BLE HRM sharing timeout fired!");
}

//! @note executes on timer task
static void prv_reminder_popup_timer_cb(void *unused) {
  prv_execute_on_kernel_main(prv_push_reminder_popup_kernel_main_cb);
}

static void prv_stop_popup_timer(void) {
  if (regular_timer_is_scheduled(&s_ble_hrm_timer)) {
    regular_timer_remove_callback(&s_ble_hrm_timer);
  }
}

static void prv_reschedule_popup_timer(void) {
  prv_stop_popup_timer();
  s_ble_hrm_timer = (RegularTimerInfo) {
    .cb = prv_reminder_popup_timer_cb,
  };
  regular_timer_add_multiminute_callback(&s_ble_hrm_timer, BLE_HRM_REMINDER_POPUP_DELAY_MINS);
}

static void prv_update_is_sharing(GAPLEConnection *connection, bool prev_is_sharing) {
  const bool is_sharing = prv_is_sharing(connection);
  if (is_sharing == prev_is_sharing) {
    return;
  }

  if (is_sharing) {
    if (s_ble_hrm_subscription_count == 0) {
      prv_reschedule_popup_timer();
      prv_execute_on_kernel_main(prv_start_hrm_kernel_main);
    }
    ++s_ble_hrm_subscription_count;
  } else {
    --s_ble_hrm_subscription_count;
    if (s_ble_hrm_subscription_count == 0) {
      prv_stop_popup_timer();
      prv_execute_on_kernel_main(prv_stop_hrm_kernel_main);
    }
  }

  // Emit for every subscription, so Settings/Bluetooth menu can update accordingly.
  prv_put_sharing_state_updated_event(s_ble_hrm_subscription_count);
}

static void prv_update_permission(GAPLEConnection *connection, HrmSharingPermission permission) {
  bt_lock_assert_held(true);
  if (prv_get_permission_by_device(&connection->device) == permission) {
    return;
  }
  const bool prev_is_sharing = prv_is_sharing(connection);
  prv_upsert_permission(&connection->device, permission);
  prv_update_is_sharing(connection, prev_is_sharing);
}

static void prv_disconnect_to_kill_subscription(GAPLEConnection *connection) {
  // Unfortunately, GATT does not offer a way to remove a subscription from the server side.
  // Only clients (subscribers) themselves can change the subscription state (write the CCCD).
  // When stopping sharing, we're disconnecting the LE link just to reset the remote subscription
  // state. Yes, a pretty big hammer... :( If we don't do this, the other end will stay subscribed.
  // Then when an app on the phone uses the HRM service "again", there won't be a new CCCD write
  // because the phone was already subscribed...
  // For declining to share up-front, we'll just leave the client subscribed and don't disconnect
  // to prevent reconnection-loops.
  bt_driver_gap_le_disconnect(&connection->device);
}

void ble_hrm_revoke_sharing_permission_for_connection(GAPLEConnection *connection) {
  PBL_LOG(LOG_LEVEL_INFO, "BLE HRM sharing: revoked for conn %p", connection);
  bt_lock();
  if (gap_le_connection_is_valid(connection)) {
    prv_update_permission(connection, HrmSharingPermission_Declined);
    prv_disconnect_to_kill_subscription(connection);
  }
  bt_unlock();

  analytics_event_ble_hrm(BleHrmEventSubtype_SharingRevoked);
}

static void prv_revoke_gap_le_connection_for_each_cb(GAPLEConnection *connection, void *unused) {
  prv_update_permission(connection, HrmSharingPermission_Declined);
  prv_disconnect_to_kill_subscription(connection);
}

void ble_hrm_revoke_all(void) {
  bt_lock();
  gap_le_connection_for_each(prv_revoke_gap_le_connection_for_each_cb, NULL);
  bt_unlock();

  // Counting as one -- it's one user action.
  analytics_event_ble_hrm(BleHrmEventSubtype_SharingRevoked);
  PBL_LOG(LOG_LEVEL_INFO, "BLE HRM sharing: all revoked");
}

static void prv_update_subscription(GAPLEConnection *connection, bool is_subscribed) {
  bt_lock_assert_held(true);
  if (connection->hrm_service_is_subscribed == is_subscribed) {
    return;
  }
  PBL_LOG(LOG_LEVEL_INFO, "BLE HRM sharing: conn <%p> is_subscribed=%u", connection, is_subscribed);

  const bool prev_is_sharing = prv_is_sharing(connection);
  connection->hrm_service_is_subscribed = is_subscribed;
  prv_update_is_sharing(connection, prev_is_sharing);

  if (is_subscribed) {
    const HrmSharingPermission current_permission =
        prv_get_permission_by_device(&connection->device);
    switch (current_permission) {
      case HrmSharingPermission_Unknown:
        prv_request_sharing_permission(connection);
        break;
      case HrmSharingPermission_Granted:
        // Stop advertising the with the HR service in the adv payload.
        // Note: we're assuming this is the only device we were advertising for.
        gap_le_slave_reconnect_hrm_stop();
        break;
      default:
        break;
    }
  }
}

static void prv_reset_subscriptions(void) {
  bt_lock();
  if (s_ble_hrm_subscription_count) {
    s_ble_hrm_subscription_count = 0;
    prv_stop_popup_timer();
    bt_unlock();

    prv_execute_on_kernel_main(prv_stop_hrm_kernel_main);
  } else {
    bt_unlock();
  }
}

void ble_hrm_handle_sharing_request_response(bool is_granted,
                                             BLEHRMSharingRequest *sharing_request) {
  PBL_LOG(LOG_LEVEL_INFO, "BLE HRM sharing permission is_granted=%u", is_granted);

  bt_lock();
  GAPLEConnection *connection = sharing_request->connection;
  if (gap_le_connection_is_valid(connection)) {
    const HrmSharingPermission permission =
        (is_granted ? HrmSharingPermission_Granted : HrmSharingPermission_Declined);
    prv_update_permission(connection, permission);
  }
  bt_unlock();

  kernel_free(sharing_request);

  analytics_event_ble_hrm(is_granted ?
                          BleHrmEventSubtype_SharingAccepted : BleHrmEventSubtype_SharingDeclined);
}

void bt_driver_cb_hrm_service_update_subscription(const BTDeviceInternal *device,
                                                  bool is_subscribed) {
  bt_lock();
  if (!s_ble_hrm_is_inited) {
    goto unlock;
  }
  GAPLEConnection *connection = gap_le_connection_by_device(device);
  if (!connection) {
    PBL_LOG(LOG_LEVEL_ERROR, "Subscription update but no connection?");
    goto unlock;
  }
  prv_update_subscription(connection, is_subscribed);
unlock:
  bt_unlock();
}

void ble_hrm_handle_disconnection(GAPLEConnection *connection) {
  if (!s_ble_hrm_is_inited) {
    return;
  }
  if (prv_is_sharing(connection)) {
    // Certain phone apps require the HR device to advertise with the HR service in the adv payload
    // in order to make reconnection work, regardless of whether the Pebble mobile app already takes
    // care of reconnecting... Therefore, advertise with the HR service for up to 60 seconds:
    gap_le_slave_reconnect_hrm_restart();
  }
  prv_update_subscription(connection, false /* is_subscribed */);

  // Just leave the permission until we reboot, toggle airplane mode or the user manually revokes.
}

void ble_hrm_init(void) {
  s_ble_hrm_is_inited = true;
  s_ble_hrm_timer = (RegularTimerInfo) {};
}

void ble_hrm_deinit(void) {
  s_ble_hrm_is_inited = false;
  gap_le_slave_reconnect_hrm_stop();
  prv_reset_subscriptions();
  prv_free_all_permissions();
}

// For unit testing
RegularTimerInfo *ble_hrm_timer(void) {
  return &s_ble_hrm_timer;
}

#endif
