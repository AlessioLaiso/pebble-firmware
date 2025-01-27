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

#include "ble_hrm_reminder_popup.h"

#include "drivers/rtc.h"
#include "services/common/i18n/i18n.h"
#include "services/normal/notifications/notifications.h"
#include "services/normal/timeline/timeline.h"
#include "services/normal/timeline/timeline_resources.h"

#include <util/size.h>

void ble_hrm_push_reminder_popup(void) {
  AttributeList attr_list = {};

  const char *body = i18n_get("Your heart rate has been shared with an app on your phone for "
                              "several hours. This could affect your battery. Stop sharing now?",
                              &attr_list);
  attribute_list_add_cstring(&attr_list, AttributeIdBody, body);
  attribute_list_add_uint32(&attr_list, AttributeIdIconTiny,
                            TIMELINE_RESOURCE_BLE_HRM_SHARING);
  attribute_list_add_uint8(&attr_list, AttributeIdBgColor, GColorOrangeARGB8);

  AttributeList dismiss_action_attr_list = {};
  attribute_list_add_cstring(&dismiss_action_attr_list, AttributeIdTitle,
                             i18n_get("Dismiss", &attr_list));

  AttributeList stop_action_attr_list = {};
  attribute_list_add_cstring(&stop_action_attr_list, AttributeIdTitle,
                             i18n_get("Stop Sharing Heart Rate", &attr_list));

  TimelineItemActionGroup action_group = {
    .num_actions = 2,
    .actions = (TimelineItemAction[]) {
      {
        .id = 0,
        .type = TimelineItemActionTypeDismiss,
        .attr_list = dismiss_action_attr_list,
      },
      {
        .id = 1,
        .type = TimelineItemActionTypeBLEHRMStopSharing,
        .attr_list = stop_action_attr_list,
      },
    },
  };

  TimelineItem *item = timeline_item_create_with_attributes(rtc_get_time(), 0,
                                                            TimelineItemTypeNotification,
                                                            LayoutIdNotification, &attr_list,
                                                            &action_group);
  i18n_free_all(&attr_list);
  attribute_list_destroy_list(&attr_list);
  attribute_list_destroy_list(&dismiss_action_attr_list);
  attribute_list_destroy_list(&stop_action_attr_list);

  notifications_add_notification(item);

  timeline_item_destroy(item);
}
