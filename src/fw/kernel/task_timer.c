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

#include "task_timer.h"
#include "task_timer_manager.h"

#include "kernel/pebble_tasks.h"
#include "kernel/pbl_malloc.h"
#include "os/mutex.h"
#include "os/tick.h"
#include "system/logging.h"
#include "system/passert.h"
#include "util/list.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"


// Structure of a timer
typedef struct TaskTimer {
  //! Entry into either the running timers (manager->running_timers) list or the idle timers
  //! (manager->idle_timers)
  ListNode list_node;

  //! The tick value when this timer will expire (in ticks). If the timer isn't currently
  //! running (scheduled) this value will be zero.
  RtcTicks expire_time;

  RtcTicks period_ticks;

  TaskTimerID id;            //<! ID assigned to this timer

  //! client provided callback function and argument
  TaskTimerCallback cb;
  void* cb_data;

  //! True if this timer should automatically be rescheduled for period_time ticks from now
  bool repeating:1;

  //! True if this timer is currently having its callback executed.
  bool executing:1;

  //! Set by the delete function of client tries to delete a timer currently executing its
  //! callback
  bool defer_delete:1;
} TaskTimer;

// ------------------------------------------------------------------------------------
// Comparator function for list_sorted_add
// Returns the order in which (a, b) occurs
// returns negative int for a descending value (a > b), positive for an ascending value (b > a),
// 0 for equal
static int prv_timer_expire_compare_func(void* a, void* b) {
  if (((TaskTimer*)b)->expire_time < ((TaskTimer*)a)->expire_time) {
    return -1;
  } else if (((TaskTimer*)b)->expire_time > ((TaskTimer*)a)->expire_time) {
    return 1;
  } else {
    return 0;
  }
}


// ------------------------------------------------------------------------------------
// Find timer by id
static bool prv_id_list_filter(ListNode* node, void* data) {
  TaskTimer* timer = (TaskTimer*)node;
  return timer->id == (uint32_t) data;
}

static TaskTimer* prv_find_timer(TaskTimerManager *manager, TaskTimerID timer_id) {
  PBL_ASSERTN(timer_id != TASK_TIMER_INVALID_ID);
  // Look for this timer in either the running or idle list
  ListNode* node = list_find(manager->running_timers, prv_id_list_filter, (void*)timer_id);
  if (!node) {
    node = list_find(manager->idle_timers, prv_id_list_filter, (void*)timer_id);
  }
  PBL_ASSERTN(node);
  return (TaskTimer *)node;
}


// =======================================================================================
// Client-side Implementation

// ---------------------------------------------------------------------------------------
// Create a new timer
TaskTimerID task_timer_create(TaskTimerManager *manager) {
  TaskTimer *timer = kernel_malloc(sizeof(TaskTimer));
  if (!timer) {
    return TASK_TIMER_INVALID_ID;
  }

  // Grab lock on timer structures, create a unique ID for this timer and put it into our idle
  // timers list
  mutex_lock(manager->mutex);
  *timer = (TaskTimer) {
    .id = manager->next_id++,
  };

  // We don't expect to wrap around, this would take over 100 years if we allocated a timer every
  // second
  PBL_ASSERTN(timer->id != TASK_TIMER_INVALID_ID);

  manager->idle_timers = list_insert_before(manager->idle_timers, &timer->list_node);
  mutex_unlock(manager->mutex);

  return timer->id;
}


// --------------------------------------------------------------------------------
// Schedule a timer to run.
bool task_timer_start(TaskTimerManager *manager, TaskTimerID timer_id,
                      uint32_t timeout_ms, TaskTimerCallback cb, void *cb_data, uint32_t flags) {
  TickType_t timeout_ticks = milliseconds_to_ticks(timeout_ms);
  RtcTicks current_time = rtc_get_ticks();

  // Grab lock on timer structures
  mutex_lock(manager->mutex);

  // Find this timer
  TaskTimer* timer = prv_find_timer(manager, timer_id);
  PBL_ASSERTN(!timer->defer_delete);

  // If this timer is currently executing it's callback, return false if
  // TIMER_START_FLAG_FAIL_IF_EXECUTING is on
  if (timer->executing && (flags & TIMER_START_FLAG_FAIL_IF_EXECUTING)) {
    mutex_unlock(manager->mutex);
    return false;
  }

  // If the TIMER_START_FLAG_FAIL_IF_SCHEDULED flag is on, make sure timer is not already scheduled
  if ((flags & TIMER_START_FLAG_FAIL_IF_SCHEDULED) && timer->expire_time) {
    mutex_unlock(manager->mutex);
    return false;
  }

  // Remove it from its current list
  if (timer->expire_time) {
    PBL_ASSERTN(list_contains(manager->running_timers, &timer->list_node));
    list_remove(&timer->list_node, &manager->running_timers /* &head */, NULL /* &tail */);
  } else {
    PBL_ASSERTN(list_contains(manager->idle_timers, &timer->list_node));
    list_remove(&timer->list_node, &manager->idle_timers /* &head */, NULL /* &tail */);
  }

  // Set timer variables
  timer->cb = cb;
  timer->cb_data = cb_data;
  timer->expire_time = current_time + timeout_ticks;
  timer->repeating = flags & TIMER_START_FLAG_REPEATING;
  timer->period_ticks = timeout_ticks;

  // Insert into sorted order in the running list
  manager->running_timers = list_sorted_add(manager->running_timers, &timer->list_node,
                                            prv_timer_expire_compare_func, true);

  // Wake up our service task if this is the new head so that it can recompute its wait timeout
  if (manager->running_timers == &timer->list_node) {
    xSemaphoreGive(manager->semaphore);
  }
  mutex_unlock(manager->mutex);
  return true;
}


// --------------------------------------------------------------------------------
// Return scheduled status
bool task_timer_scheduled(TaskTimerManager *manager, TaskTimerID timer_id, uint32_t *expire_ms_p) {
  mutex_lock(manager->mutex);

  // Find this timer in our list
  TaskTimer* timer = prv_find_timer(manager, timer_id);
  PBL_ASSERTN(!timer->defer_delete);

  // If expire timer is not 0, it means we are scheduled
  bool retval = (timer->expire_time != 0);

  // Figure out expire timer?
  if (expire_ms_p != NULL && retval) {
    RtcTicks current_ticks = rtc_get_ticks();
    if (timer->expire_time > current_ticks) {
      *expire_ms_p = ((timer->expire_time - current_ticks) * 1000)  / configTICK_RATE_HZ;
    } else {
      *expire_ms_p = 0;
    }
  }

  mutex_unlock(manager->mutex);
  return retval;
}


// --------------------------------------------------------------------------------
// Stop a timer. If the timer callback is currently executing, return false, else return true.
bool task_timer_stop(TaskTimerManager *manager, TaskTimerID timer_id) {
  mutex_lock(manager->mutex);

  // Find this timer in our list
  TaskTimer* timer = prv_find_timer(manager, timer_id);
  PBL_ASSERTN(!timer->defer_delete);

  // Move it to the idle list if it's currently running
  if (timer->expire_time) {
    PBL_ASSERTN(list_contains(manager->running_timers, &timer->list_node));
    list_remove(&timer->list_node, &manager->running_timers /* &head */, NULL /* &tail */);
    manager->idle_timers = list_insert_before(manager->idle_timers, &timer->list_node);
  }

  // Clear the repeating flag so that if they call this method from a callback it won't get
  // rescheduled.
  timer->repeating = false;
  timer->expire_time = 0;

  mutex_unlock(manager->mutex);
  return (!timer->executing);
}


// --------------------------------------------------------------------------------
// Delete a timer
void task_timer_delete(TaskTimerManager *manager, TaskTimerID timer_id) {
  mutex_lock(manager->mutex);

  // Find this timer in our list
  TaskTimer* timer = prv_find_timer(manager, timer_id);

  // If it's already marked for deletion return
  if (timer->defer_delete) {
    mutex_unlock(manager->mutex);
    return;
  }

  // Automatically stop it if it it's not stopped already
  if (timer->expire_time) {
    timer->expire_time = 0;
    PBL_ASSERTN(list_contains(manager->running_timers, &timer->list_node));
    list_remove(&timer->list_node, &manager->running_timers /* &head */, NULL /* &tail */);
    manager->idle_timers = list_insert_before(manager->idle_timers, &timer->list_node);
  }
  timer->repeating = false; // In case it's currently executing, make sure we don't reschedule it

  // If it's currently executing, defer the delete till after the callback returns. The next call
  // to task_timer_manager_execute_expired_timers service loop will take care of this for us.
  if (timer->executing) {
    timer->defer_delete = true;
    mutex_unlock(manager->mutex);
  } else {
    PBL_ASSERTN(list_contains(manager->idle_timers, &timer->list_node));
    list_remove(&timer->list_node, &manager->idle_timers /* &head */, NULL /* &tail */);
    mutex_unlock(manager->mutex);
    kernel_free(timer);
  }
}


void task_timer_manager_init(TaskTimerManager *manager, SemaphoreHandle_t semaphore) {
  *manager = (TaskTimerManager) {
    .mutex = mutex_create(),
    // Initialize next id to be a number that's theoretically unique per-task
    .next_id = (pebble_task_get_current() << 28) + 1,
    .semaphore = semaphore
  };

  // The above shift assumes next_id is a 32-bit int and there are fewer than 16 tasks.
  _Static_assert(sizeof(((TaskTimerManager*)0)->next_id) == 4, "next_id is not the right width");
  _Static_assert(NumPebbleTask < 16, "Too many tasks");
}


TickType_t task_timer_manager_execute_expired_timers(TaskTimerManager *manager) {
  while (1) {
    TickType_t ticks_to_wait = 0;
    RtcTicks next_expiry_time = 0;

    // -------------------------------------------------------------------------------------
    // If a timer is ready to run, set time_to_wait to 0 and put the timer into 'next_timer'.
    // If no timer is ready yet, then ticks_to_wait will be > 0.
    mutex_lock(manager->mutex);

    TaskTimer *next_timer = (TaskTimer*) manager->running_timers;
    if (next_timer != NULL) {
      next_expiry_time = next_timer->expire_time;
      RtcTicks current_time = rtc_get_ticks();

      if (next_expiry_time <= current_time) {
        // Found a timer that has expired! Move it from the running list to the idle talk and
        // mark it as executing.
        manager->running_timers = list_pop_head(manager->running_timers);
        manager->idle_timers = list_insert_before(manager->idle_timers, &next_timer->list_node);

        next_timer->executing = true;
        next_timer->expire_time = 0;

        // If we fell way behind (at least 1 timer period + 5 seconds) on a repeating timer
        // (presumably because we were in the debugger) advance next_expiry_time so that we don't
        // need to call this callback more than twice in a row in order to catch up.
        if (next_timer->repeating
            && (int64_t)next_expiry_time < (int64_t)(current_time - next_timer->period_ticks
                                            - 5 * RTC_TICKS_HZ)) {
          PBL_LOG(LOG_LEVEL_WARNING, "NT: Skipping some callbacks for %p because we fell behind",
                  next_timer->cb);
        }
      } else {
        // The next timer hasn't expired yet. Update
        ticks_to_wait = next_expiry_time - current_time;
      }
    } else {
      // No timers running
      ticks_to_wait = portMAX_DELAY;
    }

    mutex_unlock(manager->mutex);

    if (ticks_to_wait) {
      return ticks_to_wait;
    }

    // Run the timer callback now
    manager->current_cb = next_timer->cb_data;
    next_timer->cb(next_timer->cb_data);
    manager->current_cb = NULL;

    // Update state after the callback
    mutex_lock(manager->mutex);
    next_timer->executing = false;

    // Re-insert into timers list now if it's a repeating timer and wasn't re-scheduled by the
    // callback (next_timer->expire_time != 0)
    if (next_timer->repeating && !next_timer->expire_time) {
      next_timer->expire_time = next_expiry_time + next_timer->period_ticks;
      list_remove(&next_timer->list_node, &manager->idle_timers /* &head */, NULL /* &tail */);
      manager->running_timers = list_sorted_add(manager->running_timers, &next_timer->list_node,
                                                prv_timer_expire_compare_func, true);
    }

    // If it's been marked for deletion, take care of that now
    if (next_timer->defer_delete) {
      PBL_ASSERTN(list_contains(manager->idle_timers, &next_timer->list_node));
      list_remove(&next_timer->list_node, &manager->idle_timers /* &head */, NULL /* &tail */);
      mutex_unlock(manager->mutex);

      kernel_free(next_timer);

    } else {
      mutex_unlock(manager->mutex);
    }
  }
}

void* task_timer_manager_get_current_cb(const TaskTimerManager *manager) {
  return manager->current_cb;
}
