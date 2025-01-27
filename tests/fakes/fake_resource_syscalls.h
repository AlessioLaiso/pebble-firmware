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

#include "resource/resource.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


ResAppNum sys_get_current_resource_num(void);

uint32_t sys_resource_load_file_as_resource(const char *filepath, const char *filename);

size_t sys_resource_size(ResAppNum app_num, uint32_t handle);

size_t sys_resource_load_range(ResAppNum app_num, uint32_t id, uint32_t start_bytes, uint8_t *buffer, size_t num_bytes);

const uint8_t * sys_resource_read_only_bytes(ResAppNum app_num, uint32_t resource_id,
                                             size_t *num_bytes_out);

uint32_t sys_resource_get_and_cache(ResAppNum app_num, uint32_t resource_id);

bool sys_resource_is_valid(ResAppNum app_num, uint32_t resource_id);

void fake_resource_syscalls_cleanup(void);
