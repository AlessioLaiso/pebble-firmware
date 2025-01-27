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

#include "new_timer.h"

#include "kernel/pbl_malloc.h"
#include "kernel/task_timer_manager.h"
#include "kernel/util/task_init.h"
#include "kernel/pebble_tasks.h"
#include "mcu/interrupts.h"
#include "os/mutex.h"
#include "os/tick.h"
#include "system/logging.h"
#include "system/passert.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"


typedef struct {
  NewTimerWorkCallback cb;
  void *data;
} NewTimerWorkItem;

// The timer service loop blocks on this binary semaphore with a timeout waiting for the next timer
// to be ready to fire.
static SemaphoreHandle_t s_wake_srv_loop;

//! Queue of pointers to that should be called on the new_timer thread. This allows very high
//! priority pieces of work to be done on the new_timer thread in between timers.
static QueueHandle_t s_work_queue;

// Used by debugging facility
static void *s_current_work_cb = 0;

static TaskTimerManager s_task_timer_manager;

// =======================================================================================
// Client-side Implementation

// ---------------------------------------------------------------------------------------
// Create a new timer
TimerID new_timer_create(void) {
  return task_timer_create(&s_task_timer_manager);
}


// --------------------------------------------------------------------------------
// Schedule a timer to run. 
bool new_timer_start(TimerID timer_id, uint32_t timeout_ms, NewTimerCallback cb, void *cb_data,
                     uint32_t flags) {
  return task_timer_start(&s_task_timer_manager, timer_id, timeout_ms, cb, cb_data, flags);
}


// --------------------------------------------------------------------------------
// Return scheduled status
bool new_timer_scheduled(TimerID timer_id, uint32_t *expire_ms_p) {
  return task_timer_scheduled(&s_task_timer_manager, timer_id, expire_ms_p);
}


// --------------------------------------------------------------------------------
// Stop a timer. If the timer callback is currently executing, return false, else return true.
bool new_timer_stop(TimerID timer_id) {
  return task_timer_stop(&s_task_timer_manager, timer_id);
}


// --------------------------------------------------------------------------------
// Delete a timer
void new_timer_delete(TimerID timer_id) {
  task_timer_delete(&s_task_timer_manager, timer_id);
}


// ========================================================================================
// Service Implementation
static void new_timer_service_loop(void *data) {
  task_init();

  while (1) {
    TickType_t ticks_to_wait = task_timer_manager_execute_expired_timers(&s_task_timer_manager);

    xSemaphoreTake(s_wake_srv_loop, ticks_to_wait);

    // See if we have any work to do
    NewTimerWorkItem work;
    if (xQueueReceive(s_work_queue, &work, 0) == pdTRUE) {
      s_current_work_cb = work.cb;
      work.cb(work.data);
      s_current_work_cb = NULL;
    }
  }
}


// -----------------------------------------------------------------------------------------------
// Used by the watchdog timer logic
void* new_timer_debug_get_current_callback(void) {
  void *timer_cb = task_timer_manager_get_current_cb(&s_task_timer_manager);
  if (timer_cb) {
    return timer_cb;
  }
  return s_current_work_cb;
}

//=========================================================================================
// Initialize the timer service
void new_timer_service_init(void) {
  PBL_LOG(LOG_LEVEL_DEBUG, "NT: Initializing");

  vSemaphoreCreateBinary(s_wake_srv_loop);

  task_timer_manager_init(&s_task_timer_manager, s_wake_srv_loop);

  const int WORK_QUEUE_SIZE = 5;
  s_work_queue = xQueueCreate(WORK_QUEUE_SIZE, sizeof(NewTimerWorkItem));

  const int TASK_STACK_SIZE_BYTES = 1380;

  TaskParameters_t task_params = {
    .pvTaskCode = new_timer_service_loop,
    .pcName = "NewTimer",
    .usStackDepth = TASK_STACK_SIZE_BYTES / sizeof( portSTACK_TYPE ),
    .uxPriority =  (configMAX_PRIORITIES - 1) | portPRIVILEGE_BIT, // Max priority
    .puxStackBuffer = NULL,
  };

  pebble_task_create(PebbleTask_NewTimers, &task_params, NULL);
}


// -----------------------------------------------------------------------------------------------------
// Used by the console command to list timers
bool new_timer_add_work_callback_from_isr(NewTimerWorkCallback cb, void *data) {
  BaseType_t should_context_switch;
  NewTimerWorkItem work = { cb, data };
  xQueueSendFromISR(s_work_queue, &work, &should_context_switch);

  // Wake up the thread to process the work item we just added.
  // Reuse the previous bool since we don't actually care about the above result. No one blocks
  // on the above queue, only this semaphore.
  xSemaphoreGiveFromISR(s_wake_srv_loop, &should_context_switch);

  return (should_context_switch == pdTRUE);
}

bool new_timer_add_work_callback(NewTimerWorkCallback cb, void *data) {
  TickType_t TICKS_TO_WAIT = 50;

  NewTimerWorkItem work = { cb, data };
  if (xQueueSend(s_work_queue, &work, TICKS_TO_WAIT)) {
    // Wake up the thread to process the work item we just added.
    xSemaphoreGive(s_wake_srv_loop);
    return true;
  }

  return false;
}
