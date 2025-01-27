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

#include "profile_mutexes_app.h"

#include "applib/app.h"
#include "applib/ui/app_window_stack.h"
#include "applib/ui/window.h"
#include "applib/ui/window_stack.h"

#include "system/logging.h"
#include "os/mutex.h"
#include "system/profiler.h"

static Window *window;
static PebbleMutex *s_mutex;
static PebbleRecursiveMutex *s_rmutex;

static void profile_mutexes(void) {
  PBL_LOG(LOG_LEVEL_DEBUG, "INITIALIZING PROFILER FOR MUTEXES!");
  PROFILER_INIT;
  PROFILER_START;

  s_mutex = mutex_create();
  for (int i=0; i < 10000; i++) {
    mutex_lock(s_mutex);
    mutex_unlock(s_mutex);
  }
  mutex_destroy(s_mutex);
  s_mutex = NULL;

  s_rmutex = mutex_create_recursive();
  for (int i=0; i < 10000; i++) {
    mutex_lock_recursive(s_rmutex);
  }
  for (int i=0; i < 10000; i++) {
    mutex_unlock_recursive(s_rmutex);
  }
  mutex_destroy((PebbleMutex *)s_rmutex);
  s_rmutex = NULL;

  PROFILER_STOP;
  PROFILER_PRINT_STATS;
}

static void s_main(void) {
  window = window_create();
  app_window_stack_push(window, true /* Animated */);

  profile_mutexes();

  app_event_loop();
}

const PebbleProcessMd* profile_mutexes_get_app_info(void) {
  static const PebbleProcessMdSystem s_app_info = {
    .common.main_func = &s_main,
    .name = "Profile Mutexes"
  };
  return (const PebbleProcessMd*) &s_app_info;
}
