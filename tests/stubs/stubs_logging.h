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

#include "system/logging.h"
#include "util/string.h"

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

bool g_pbl_log_enabled = false;
int g_pbl_log_level = 0;

#ifdef CUSTOM_LOG_INTERNAL
static void log_internal(uint8_t log_level, const char* src_filename, int src_line_number,
                         const char* fmt, va_list args);
#else
static void log_internal(uint8_t log_level, const char* src_filename, int src_line_number,
                         const char* fmt, va_list args) {
  printf("%s:%d> ", GET_FILE_NAME(src_filename), src_line_number);
  vprintf(fmt, args);
  printf("\n");
}
#endif

void pbl_log_vargs(uint8_t log_level, const char* src_filename, int src_line_number,
                   const char* fmt, va_list args) {
  log_internal(log_level, src_filename, src_line_number, fmt, args);
}

void pbl_log(uint8_t log_level, const char* src_filename, int src_line_number,
             const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  log_internal(log_level, src_filename, src_line_number, fmt, args);
  va_end(args);
}

void pbl_log_sync(uint8_t log_level, const char* src_filename, int src_line_number,
                  const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  log_internal(log_level, src_filename, src_line_number, fmt, args);
  va_end(args);
}

void command_dump_malloc() {
}

void reset_due_to_software_failure() {
  assert(0);
}

void app_log_vargs(uint8_t log_level, const char *src_filename, int src_line_number,
                   const char *fmt, va_list args) {
  log_internal(log_level, src_filename, src_line_number, fmt, args);
}

void app_log(uint8_t log_level, const char* src_filename, int src_line_number,
             const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  log_internal(log_level, src_filename, src_line_number, fmt, args);
  va_end(args);
}

void kernel_pbl_log_from_fault_handler_fmt(
    const char *src_filename, uint16_t src_line_number, char *buffer,
    unsigned int buffer_size, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  log_internal(LOG_LEVEL_ALWAYS, src_filename, src_line_number, fmt, args);
  va_end(args);
}
