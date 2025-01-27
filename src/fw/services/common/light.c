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

#include "services/common/light.h"

#include "board/board.h"
#include "drivers/ambient_light.h"
#include "drivers/backlight.h"
#include "kernel/low_power.h"
#include "services/common/analytics/analytics.h"
#include "services/common/battery/battery_monitor.h"
#include "services/common/new_timer/new_timer.h"
#include "syscall/syscall_internal.h"
#include "system/logging.h"
#include "os/mutex.h"
#include "system/passert.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include <stdlib.h>

typedef enum {
  LIGHT_STATE_ON = 1,           // backlight on, no timeouts
  LIGHT_STATE_ON_TIMED = 2,     // backlight on, will start fading after a period
  LIGHT_STATE_ON_FADING = 3,    // backlight in the process of fading out
  LIGHT_STATE_OFF = 4,          // backlight off; idle state
} BacklightState;

// the time duration of the fade out
const uint32_t LIGHT_FADE_TIME_MS = 500;
// number of fade-out steps
const uint32_t LIGHT_FADE_STEPS = 20;

/*
 *              ^
 *              |
 *     LIGHT_ON |            +---------------------------------+
 *              |           /                                   \
 *              |          /                                     \
 *              |         /                                       \
 *              |        /                                         \
 *              |       /                                           \
 *  LIGHT_ON/2  |      /+                                           +\
 *              |     / |                                           | \
 *              |    /  |                                           |  \
 *              |   /   |                                           |   \
 *              |  /    |                                           |    \
 *              | /     |                                           |     \
 *              |/      |                                           |      \
 *    LIGHT_OFF +-------|-------------------------------------------|--------->
 *                      |                                           |
 *                      |<----------------------------------------->|
 *                          Integrate over this range for the mean
 */

//! The current state of the backlight (example: ON/ON_TIMED/ON_FADING).
static BacklightState s_light_state;

//! The brightness of the display in a range between BACKLIGHT_BRIGHTNESS_OFF and BACKLIGHT_BRIGHTNESS_ON
static int32_t s_current_brightness;

//! Timer to count down from the LIGHT_STATE_ON_TIMED state.
static TimerID s_timer_id;

//! Refcount of the number of buttons that are currently pushed
static int s_num_buttons_down;

//! The current app is forcing the light on and off, don't muck with it.
static bool s_user_controlled_state;

//! For temporary disabling backlight (ie: low power mode)
static bool s_backlight_allowed = false;

//! Mutex to guard all the above state. We have a pattern of taking the lock in the public functions and assuming
//! it's already taken in the prv_ functions.
static PebbleMutex *s_mutex;

static void prv_change_state(BacklightState new_state);

static void light_timer_callback(void *data) {
  mutex_lock(s_mutex);
  prv_change_state(LIGHT_STATE_ON_FADING);
  mutex_unlock(s_mutex);
}

static uint16_t prv_backlight_get_intensity(void) {
  // low_power_mode backlight intensity (25% of max brightness)
  const uint16_t backlight_low_power_intensity = (BACKLIGHT_BRIGHTNESS_MAX * (uint32_t)25) / 100;
  return low_power_is_active() ? backlight_low_power_intensity : backlight_get_intensity();
}

static void prv_change_brightness(int32_t new_brightness) {
  const uint16_t HALF_BRIGHTNESS = (prv_backlight_get_intensity() - BACKLIGHT_BRIGHTNESS_OFF) / 2;

  // update the debug stats
  if (new_brightness > HALF_BRIGHTNESS && s_current_brightness <= HALF_BRIGHTNESS) {
    // getting brighter and have now transitioned past half brightness
    analytics_stopwatch_start(ANALYTICS_APP_METRIC_BACKLIGHT_ON_TIME, AnalyticsClient_App);
    analytics_stopwatch_start(ANALYTICS_DEVICE_METRIC_BACKLIGHT_ON_TIME, AnalyticsClient_System);
    analytics_inc(ANALYTICS_APP_METRIC_BACKLIGHT_ON_COUNT, AnalyticsClient_App);
    analytics_inc(ANALYTICS_DEVICE_METRIC_BACKLIGHT_ON_COUNT, AnalyticsClient_System);
  }

  if (new_brightness <= HALF_BRIGHTNESS && s_current_brightness > HALF_BRIGHTNESS) {
    // getting dimmer and have now transitioned past half brightness
    analytics_stopwatch_stop(ANALYTICS_APP_METRIC_BACKLIGHT_ON_TIME);
    analytics_stopwatch_stop(ANALYTICS_DEVICE_METRIC_BACKLIGHT_ON_TIME);
  }

  backlight_set_brightness(new_brightness);
  s_current_brightness = new_brightness;
}

static void prv_change_state(BacklightState new_state) {
  s_light_state = new_state;

  // Calculate the new brightness and reset any timers based on our state.
  int32_t new_brightness = 0;
  switch (new_state) {
    case LIGHT_STATE_ON:
      new_brightness = prv_backlight_get_intensity();
      new_timer_stop(s_timer_id);
      break;
    case LIGHT_STATE_ON_TIMED:
      new_brightness = prv_backlight_get_intensity();

      // Schedule the timer to move us from the ON_TIMED state to the ON_FADING state
      new_timer_start(s_timer_id, backlight_get_timeout_ms(),
                      light_timer_callback, NULL, 0 /* flags */);
      break;
    case LIGHT_STATE_ON_FADING:
      new_brightness = s_current_brightness - (prv_backlight_get_intensity() / LIGHT_FADE_STEPS);

      if (new_brightness <= BACKLIGHT_BRIGHTNESS_OFF) {
        // Done fading!
        new_brightness = BACKLIGHT_BRIGHTNESS_OFF;
        s_light_state = LIGHT_STATE_OFF;

        // Don't need to cancel the timer, we can only get here from the just-expired timer.
      } else {
        // Reschedule the timer so we step down the brightness again.
        new_timer_start(s_timer_id, LIGHT_FADE_TIME_MS / LIGHT_FADE_STEPS, light_timer_callback, NULL, 0 /* flags */);
      }
      break;
    case LIGHT_STATE_OFF:
      new_brightness = BACKLIGHT_BRIGHTNESS_OFF;
      new_timer_stop(s_timer_id);
      break;
  }

  if (s_current_brightness != new_brightness) {
    prv_change_brightness(new_brightness);
  }
}

static bool prv_light_allowed(void) {
  if (!s_backlight_allowed) {
    return false;
  }
  
  if (backlight_is_enabled()) {
    if (backlight_is_ambient_sensor_enabled()) {
      // If the light is off and it's bright outside, don't allow the light to turn on
      // (we don't need it!). Grab the mutex here so that the timer state machine doesn't change
      // the light brightness while we're checking the ambient light levels.
      bool allowed = !((s_current_brightness == 0) && ambient_light_is_light());
      return allowed;
    } else {
      return true;
    }
  } else {
    return false;
  }
}

void light_init(void) {
  s_light_state = LIGHT_STATE_OFF;
  s_current_brightness = BACKLIGHT_BRIGHTNESS_OFF;
  s_timer_id = new_timer_create();
  s_num_buttons_down = 0;
  s_user_controlled_state = false;
  s_mutex = mutex_create();
}

void light_button_pressed(void) {
  mutex_lock(s_mutex);

  s_num_buttons_down++;
  if (s_num_buttons_down > 4) {
    PBL_LOG(LOG_LEVEL_ERROR, "More buttons were pressed than have been released.");
    s_num_buttons_down = 0;
  }

  // set the state to be on; releasing buttons will start the timer counting down
  if (prv_light_allowed()) {
    prv_change_state(LIGHT_STATE_ON);
  }

  mutex_unlock(s_mutex);
}

void light_button_released(void) {
  mutex_lock(s_mutex);

  s_num_buttons_down--;
  if (s_num_buttons_down < 0) {
    PBL_LOG(LOG_LEVEL_ERROR, "More buttons were released than have been pressed.");
    s_num_buttons_down = 0;
  }

  if (s_num_buttons_down == 0 &&
      s_light_state == LIGHT_STATE_ON &&
      !s_user_controlled_state) {
    // no more buttons pressed: wait for a bit and then start the fade-out timer
    prv_change_state(LIGHT_STATE_ON_TIMED);
  }

  mutex_unlock(s_mutex);
}

void light_enable_interaction(void) {
  mutex_lock(s_mutex);

  //if some buttons are held or light_enable is asserted, do nothing
  if (s_num_buttons_down > 0 || s_light_state == LIGHT_STATE_ON) {
    mutex_unlock(s_mutex);
    return;
  }

  if (prv_light_allowed()) {
    prv_change_state(LIGHT_STATE_ON_TIMED);
  }

  mutex_unlock(s_mutex);
}

void light_enable(bool enable) {
  mutex_lock(s_mutex);

  // This function is a bit of a black sheep - it dives in and messes with the normal
  // flow of the state machine.
  // We don't actually use it, but it is now documented and used in the SDK, so
  // I am reluctant to chop it out.

  s_user_controlled_state = enable;

  if (enable) {
    prv_change_state(LIGHT_STATE_ON);
  } else if (s_num_buttons_down == 0) {
    // reset the state if someone calls light_enable(false);
    // (unless there are buttons pressed, then leave the backlight on)
    prv_change_state(LIGHT_STATE_OFF);
  }

  mutex_unlock(s_mutex);
}

void light_enable_respect_settings(bool enable) {
  mutex_lock(s_mutex);

  s_user_controlled_state = enable;

  if (enable) {
    if (prv_light_allowed()) {
      prv_change_state(LIGHT_STATE_ON);
    }
  } else if (s_num_buttons_down == 0) {
    prv_change_state(LIGHT_STATE_OFF);
  }

  mutex_unlock(s_mutex);
}

void light_reset_user_controlled(void) {
  mutex_lock(s_mutex);

  // http://www.youtube.com/watch?v=6t_KgE6Yuqg
  if (s_user_controlled_state) {
    s_user_controlled_state = false;

    if (s_num_buttons_down == 0) {
      prv_change_state(LIGHT_STATE_OFF);
    }
  }

  mutex_unlock(s_mutex);
}

static void prv_light_reset_to_timed_mode(void) {
  mutex_lock(s_mutex);

  if (s_user_controlled_state) {
    s_user_controlled_state = false;
    if (prv_light_allowed()) {
      prv_change_state(LIGHT_STATE_ON_TIMED);
    }
  }

  mutex_unlock(s_mutex);
}

void light_toggle_enabled(void) {
  mutex_lock(s_mutex);

  backlight_set_enabled(!backlight_is_enabled());
  if (prv_light_allowed()) {
    prv_change_state(LIGHT_STATE_ON_TIMED);
  } else {
    prv_change_state(LIGHT_STATE_OFF);
  }
  mutex_unlock(s_mutex);
}

void light_toggle_ambient_sensor_enabled(void) {
  mutex_lock(s_mutex);
  backlight_set_ambient_sensor_enabled(!backlight_is_ambient_sensor_enabled());
  if (prv_light_allowed() && !ambient_light_is_light()) {
    prv_change_state(LIGHT_STATE_ON_TIMED);
  } else {
    prv_change_state(LIGHT_STATE_OFF);
    // FIXME: PBL-24793 There is an edge case of when the backlight has timed off
    // or you're toggling it from no ambient (always light on buttons) to ambient,
    // you will see it turn on and immediately off if its bright out
  }
  mutex_unlock(s_mutex);
}

void light_allow(bool allowed) {
  if (s_backlight_allowed && !allowed) {
    prv_change_state(LIGHT_STATE_OFF);
  }
  s_backlight_allowed = allowed;
}

DEFINE_SYSCALL(void, sys_light_enable_interaction, void) {
  light_enable_interaction();
}

DEFINE_SYSCALL(void, sys_light_enable, bool enable) {
  light_enable(enable);
}

DEFINE_SYSCALL(void, sys_light_enable_respect_settings, bool enable) {
  light_enable_respect_settings(enable);
}

DEFINE_SYSCALL(void, sys_light_reset_to_timed_mode, void) {
  prv_light_reset_to_timed_mode();
}

extern BacklightBehaviour backlight_get_behaviour(void);

void analytics_external_collect_backlight_settings(void) {
  BacklightBehaviour behaviour = backlight_get_behaviour();
  bool is_motion_enabled = backlight_is_motion_enabled();
  uint8_t backlight_intensity_pct = backlight_get_intensity_percent();
  uint8_t backlight_timeout_sec = backlight_get_timeout_ms() / 1000;
  analytics_set(ANALYTICS_DEVICE_METRIC_SETTING_BACKLIGHT, behaviour,
                AnalyticsClient_System);
  analytics_set(ANALYTICS_DEVICE_METRIC_SETTING_SHAKE_TO_LIGHT,
                is_motion_enabled, AnalyticsClient_System);
  analytics_set(ANALYTICS_DEVICE_METRIC_SETTING_BACKLIGHT_INTENSITY_PCT,
                backlight_intensity_pct, AnalyticsClient_System);
  analytics_set(ANALYTICS_DEVICE_METRIC_SETTING_BACKLIGHT_TIMEOUT_SEC,
                backlight_timeout_sec, AnalyticsClient_System);
}
