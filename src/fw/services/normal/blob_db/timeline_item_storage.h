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

#include "util/uuid.h"
#include "services/common/regular_timer.h"
#include "services/normal/settings/settings_file.h"
#include "os/mutex.h"
#include "services/normal/timeline/item.h"

typedef struct {
  SettingsFile file;
  PebbleMutex *mutex;
  char *name;
  size_t max_size;
  uint32_t max_item_age; // seconds
} TimelineItemStorage;

typedef bool (*TimelineItemStorageFilterCallback)(SerializedTimelineItemHeader *hdr,
                                                  void *context);

typedef SettingsFileEachCallback TimelineItemStorageEachCallback;

//! Callback used for timeline_item_storage_delete_with_parent.
//! Called with the UUID of each child that is deleted.
typedef void (*TimelineItemStorageChildDeleteCallback)(const Uuid *id);

void timeline_item_storage_init(TimelineItemStorage *storage,
    char *filename, uint32_t max_size, uint32_t max_age);

void timeline_item_storage_deinit(TimelineItemStorage *storage);

bool timeline_item_storage_exists_with_parent(TimelineItemStorage *storage, const Uuid *parent_id);

status_t timeline_item_storage_flush(TimelineItemStorage *storage);

status_t timeline_item_storage_delete(TimelineItemStorage *storage,
    const uint8_t *key, int key_len);

status_t timeline_item_storage_read(TimelineItemStorage *storage,
    const uint8_t *key, int key_len, uint8_t *val_out, int val_len);

// This temporarily allocates heap memory, so use sparingly to prevent heap fragmentation
status_t timeline_item_storage_get_from_settings_record(SettingsFile *file,
                                                        SettingsRecordInfo *info,
                                                        TimelineItem *item);

status_t timeline_item_storage_set_status_bits(TimelineItemStorage *storage,
    const uint8_t *key, int key_len, uint8_t status);

int timeline_item_storage_get_len(TimelineItemStorage *storage,
    const uint8_t *key, int key_len);

status_t timeline_item_storage_insert(TimelineItemStorage *storage,
    const uint8_t *key, int key_len, const uint8_t *val, int val_len, bool mark_as_synced);


//! Caution: CommonTimelineItemHeader .flags & .status are stored inverted and not auto-restored
status_t timeline_item_storage_each(TimelineItemStorage *storage,
    TimelineItemStorageEachCallback each, void *data);

status_t timeline_item_storage_mark_synced(TimelineItemStorage *storage,
                                           const uint8_t *key, int key_len);

status_t timeline_item_storage_delete_with_parent(
    TimelineItemStorage *storage,
    const Uuid *parent_id,
    TimelineItemStorageChildDeleteCallback child_delete_cb);

//! filter_cb is a type that returns TRUE if the item should be used, or FALSE if the item should
//! be ignored.
status_t timeline_item_storage_next_item(TimelineItemStorage *storage, Uuid *id_out,
    TimelineItemStorageFilterCallback filter_cb);

bool timeline_item_storage_is_empty(TimelineItemStorage *storage);
