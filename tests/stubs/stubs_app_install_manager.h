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

#include "process_management/app_install_manager.h"
#include "process_management/app_install_types.h"
#include "util/attributes.h"
#include "util/uuid.h"

AppInstallId WEAK app_install_get_id_for_uuid(const Uuid *uuid) {
  return 1;
}

AppInstallId WEAK app_get_install_id_for_uuid_from_registry(const Uuid *uuid) {
  return INSTALL_ID_INVALID;
}

bool WEAK app_install_get_entry_for_install_id(AppInstallId install_id, AppInstallEntry *entry) {
  return true;
}

bool WEAK app_install_get_uuid_for_install_id(AppInstallId install_id, Uuid *uuid_out) {
  return true;
}

bool WEAK app_install_is_watchface(AppInstallId app_id) {
  return false;
}

bool WEAK app_install_id_from_system(AppInstallId id) {
  return (id < INSTALL_ID_INVALID);
}

bool WEAK app_install_id_from_app_db(AppInstallId id) {
  return (id > INSTALL_ID_INVALID);
}

void WEAK app_install_enumerate_entries(AppInstallEnumerateCb cb, void *data) {
  return;
}

bool WEAK app_install_entry_is_watchface(const AppInstallEntry *entry) {
  return false;
}

bool WEAK app_install_entry_is_hidden(const AppInstallEntry *entry) {
  return false;
}

bool WEAK app_install_entry_is_SDK_compatible(const AppInstallEntry *entry) {
  return true;
}

const WEAK PebbleProcessMd *app_install_get_md(AppInstallId id, bool worker) {
  return NULL;
}

ResAppNum WEAK app_install_get_app_icon_bank(const AppInstallEntry *entry) {
  return SYSTEM_APP;
}

