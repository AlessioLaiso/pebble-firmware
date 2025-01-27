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

#include "kernel/pebble_tasks.h"
#include "flash_region/flash_region.h"
#include "process_management/pebble_process_info.h"
#include "process_management/app_install_types.h"

#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>

#define APP_FILE_NAME_SUFFIX "app"
#define WORKER_FILE_NAME_SUFFIX "worker"

//! @file app_storage.h
//!
//! Dumping ground for functions for discovering and managing apps stored in SPI flash in the 8 app banks. This will
//! eventually be replaced by app_file.h when we're ready to get rid of the 8-app limit, so this file shouldn't exist
//! in a few months.

#define MAX_APP_BANKS 8
#define APP_FILENAME_MAX_LENGTH 32

//! See app_storage_get_app_info
typedef enum AppStorageGetAppInfoResult {
  GET_APP_INFO_SUCCESS,
  GET_APP_INFO_COULD_NOT_READ_FORMAT,
  GET_APP_INFO_INCOMPATIBLE_SDK,
  GET_APP_INFO_APP_TOO_LARGE
} AppStorageGetAppInfoResult;

//! Retrieve the process metadata for a given app_bank and performs sanity checks
//! to make sure that the process in the specified app_bank can be run by the current system.
//! @param app_info[in,out] Structure to be populated with information from flash.
//! @param build_id_out[out] Buffer into which the GNU build ID of the process its executable
//! should be copied. The buffer must be at least BUILD_ID_EXPECTED_LEN bytes. OK to pass NULL.
//! If no build ID was present, the buffer will be filled with zeroes.
//! @param app_id The app id for which the app metadata needs to be fetched.
//! @param task PebbleTask_App or PebbleTask_Worker
//! @return See AppStorageGetAppInfoResult
AppStorageGetAppInfoResult app_storage_get_process_info(PebbleProcessInfo* app_info,
  uint8_t *build_id_out, AppInstallId app_id, PebbleTask task);

//! Remove related app files for app bank
void app_storage_delete_app(AppInstallId id);

bool app_storage_app_exists(AppInstallId id);

//! Gives a name to a file given the app bank and type
//! @param name Buffer in which to place the filename in
//! @param buf_length Maximum length of buffer
//! @param app_id The app id for which the app metadata needs to be fetched.
//! @param task PebbleTask_App or PebbleTask_Worker
void app_storage_get_file_name(char *name, size_t buf_length, AppInstallId app_id, PebbleTask task);

//! Returns the size of the executable inside the given PebbleProcessInfo
//! @param info pointer to a valid PebbleProcessInfo struct
//! @return the size of the executable inside the given PebbleProcessInfo
uint32_t app_storage_get_process_load_size(PebbleProcessInfo *info);

