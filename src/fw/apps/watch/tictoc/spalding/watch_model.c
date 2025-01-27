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

#include "watch_model.h"

#include "util/trig.h"
#include "applib/app_watch_info.h"
#include "applib/pbl_std/pbl_std.h"
#include "applib/tick_timer_service.h"
#include "resource/resource_ids.auto.h"
#include "syscall/syscall.h"
#include "util/time/time.h"

#include <ctype.h>

static void prv_calculate_hand_angles(struct tm *tick_time, int32_t *hour_angle,
                                      int32_t *minute_angle) {
  *hour_angle = (tick_time->tm_hour % 12) * TRIG_MAX_ANGLE / 12
                            + tick_time->tm_min * TRIG_MAX_ANGLE / 60 / 12;
  *minute_angle = tick_time->tm_min * TRIG_MAX_ANGLE / 60;
}

static ClockFace prv_local_clock_face_default(struct tm *tick_time) {
  int32_t hour_angle, minute_angle;
  prv_calculate_hand_angles(tick_time, &hour_angle, &minute_angle);

  // TODO: Don't return by value. This thing is massive.
  return (ClockFace) {
      .hour_hand = {
        .angle = hour_angle,
        .backwards_extension = LOCAL_HOUR_HAND_BACK_EXT_DEFAULT,
        .color = LOCAL_HOUR_HAND_COLOR_DEFAULT,
        .length = LOCAL_HOUR_HAND_LENGTH_DEFAULT,
        .style = CLOCK_HAND_STYLE_ROUNDED,
        .thickness = LOCAL_HOUR_HAND_THICKNESS_DEFAULT,
      },
      .minute_hand = {
        .angle = minute_angle,
        .backwards_extension = LOCAL_MINUTE_HAND_BACK_EXT_DEFAULT,
        .color = LOCAL_MINUTE_HAND_COLOR_DEFAULT,
        .length = LOCAL_MINUTE_HAND_LENGTH_DEFAULT,
        .style = CLOCK_HAND_STYLE_ROUNDED,
        .thickness = LOCAL_MINUTE_HAND_THICKNESS_DEFAULT,
      },
      .bob_radius = LOCAL_BOB_RADIUS_DEFAULT,
      .bob_color = LOCAL_BOB_COLOR_DEFAULT,
      .location = CLOCK_LOCATION_CENTER,
  };
}

static NonLocalClockFace prv_configure_non_local_clock_face(int32_t utc_offset, const char *text,
                                                            GColor text_color, GColor hand_color,
                                                            ClockTextLocation location) {
  time_t t = rtc_get_time();
  struct tm* tick_time = pbl_override_gmtime(&t);
  tick_time->tm_hour += utc_offset; // TODO check if this works properly
  int32_t hour_angle, minute_angle;
  prv_calculate_hand_angles(tick_time, &hour_angle, &minute_angle);

  // TODO: Don't return by value. This thing is massive.
  NonLocalClockFace non_local_clock = (NonLocalClockFace) {
    .face = {
      .hour_hand = {
        .length = NON_LOCAL_HOUR_HAND_LENGTH_DEFAULT,
        .thickness = NON_LOCAL_HOUR_HAND_WIDTH_DEFAULT,
        .backwards_extension = 0,
        .angle = hour_angle,
        .color = hand_color,
        .style = CLOCK_HAND_STYLE_ROUNDED,
      },
      .minute_hand = {
        .length = NON_LOCAL_MINUTE_HAND_LENGTH_DEFAULT,
        .thickness = NON_LOCAL_MINUTE_HAND_WIDTH_DEFAULT,
        .backwards_extension = 0,
        .angle = minute_angle,
        .color = hand_color,
        .style = CLOCK_HAND_STYLE_ROUNDED,
      },
      .location = location,
    },
    .text_color = text_color,
  };
  strncpy(non_local_clock.buffer, text, sizeof(non_local_clock.buffer));
  return non_local_clock;
}

// Configure the text displayed on the clock.
static ClockText prv_configure_clock_text(ClockTextType type, ClockTextLocation location,
                                          GColor color, struct tm *tick_time) {
  ClockText text = (ClockText) {
    .location = location,
    .color = color,
  };

  if (type == CLOCK_TEXT_TYPE_DATE) {
    strftime(text.buffer, sizeof(text.buffer), "%a %d", tick_time);
  } else if (type == CLOCK_TEXT_TYPE_TIME) {
    strftime(text.buffer, sizeof(text.buffer), "$l:%M%P", tick_time);
  }

  for (uint32_t i = 0; i < sizeof(text.buffer); i++) {
    text.buffer[i] = toupper((unsigned char)text.buffer[i]);
  }

  // TODO: Don't return a struct
  return text;
}

static ClockModel prv_clock_model_default(struct tm *tick_time) {
  // Create a generic model and configure a default clock.
  ClockModel model;
  model.local_clock = prv_local_clock_face_default(tick_time);

  // Add watch-specific details.
  const WatchInfoColor watch_color = sys_watch_info_get_color();
  switch (watch_color) {
    case WATCH_INFO_COLOR_TIME_ROUND_BLACK_14:
      model.local_clock.minute_hand.color = GColorBlue;
      model.text = prv_configure_clock_text(CLOCK_TEXT_TYPE_DATE, CLOCK_TEXT_LOCATION_LEFT,
                                            GColorWhite, tick_time);
      model.bg_bitmap_id = RESOURCE_ID_MULTIWATCH_BACKGROUND_14MM_BLACK_RED;
      break;
    case WATCH_INFO_COLOR_TIME_ROUND_BLACK_20:
      model.num_non_local_clocks = 2;
      model.non_local_clock[0] = prv_configure_non_local_clock_face(-7, "LA", GColorDarkGray,
                                                                    GColorWhite,
                                                                    CLOCK_LOCATION_LEFT);
      model.non_local_clock[1] = prv_configure_non_local_clock_face(2, "PAR", GColorDarkGray,
                                                                    GColorWhite,
                                                                    CLOCK_LOCATION_RIGHT);
      model.text = prv_configure_clock_text(CLOCK_TEXT_TYPE_DATE, CLOCK_TEXT_LOCATION_BOTTOM,
                                            GColorWhite, tick_time);
      model.bg_bitmap_id = RESOURCE_ID_MULTIWATCH_BACKGROUND_20MM_BLACK;
      break;
    case WATCH_INFO_COLOR_TIME_ROUND_SILVER_14:
      model.local_clock.hour_hand.style = CLOCK_HAND_STYLE_POINTED;
      model.local_clock.hour_hand.color = GColorBlack;
      model.local_clock.minute_hand.style = CLOCK_HAND_STYLE_POINTED;
      model.local_clock.minute_hand.color = GColorCadetBlue;
      model.text = prv_configure_clock_text(CLOCK_TEXT_TYPE_DATE, CLOCK_TEXT_LOCATION_BOTTOM,
                                            GColorDarkGray, tick_time);
      model.bg_bitmap_id = RESOURCE_ID_MULTIWATCH_BACKGROUND_14MM_SILVER;
      break;
    case WATCH_INFO_COLOR_TIME_ROUND_SILVER_20:
      model.local_clock.hour_hand.style = CLOCK_HAND_STYLE_POINTED;
      model.local_clock.minute_hand.style = CLOCK_HAND_STYLE_POINTED;
      model.local_clock.minute_hand.color = GColorRed;
      model.local_clock.bob_color = GColorBlack;
      model.bg_bitmap_id = RESOURCE_ID_MULTIWATCH_BACKGROUND_20MM_SILVER_BROWN;
      break;
    case WATCH_INFO_COLOR_TIME_ROUND_ROSE_GOLD_14:
    default:
      model.local_clock.bob_center_color = GColorOrange;
      model.local_clock.minute_hand.color = GColorWhite;
      model.local_clock.minute_hand.thickness = 2;
      model.local_clock.minute_hand.length = 54;
      model.local_clock.hour_hand.color = GColorBlack;
      model.local_clock.hour_hand.thickness = 8;
      model.local_clock.hour_hand.length = 39;
      model.local_clock.bob_radius = 7;
      model.local_clock.bob_center_radius = 3;
      model.local_clock.bob_color = GColorWhite;
      model.bg_bitmap_id = RESOURCE_ID_MULTIWATCH_BACKGROUND_14MM_ROSE_GOLD;
      break;
  }

  // disable timezones until they can be configured by the user
  model.num_non_local_clocks = 0;

  // TODO: Don't return a struct here
  return model;
}

static void prv_handle_time_update(struct tm *tick_time, TimeUnits units_changed) {
  ClockModel model = prv_clock_model_default(tick_time);
  watch_model_handle_change(&model);
}

void watch_model_cleanup() {
  tick_timer_service_unsubscribe();
}

static void prv_intro_animation_finished(Animation *animation) {
  const time_t t = rtc_get_time();
  prv_handle_time_update(pbl_override_localtime(&t), (TimeUnits)0xff);

  tick_timer_service_subscribe(MINUTE_UNIT, prv_handle_time_update);
}

void watch_model_start_intro() {
  prv_intro_animation_finished(NULL);
}

void watch_model_init(void) {
  const time_t t = rtc_get_time();
  struct tm *tick_time = pbl_override_localtime(&t);
  ClockModel model = prv_clock_model_default(tick_time);
  watch_model_handle_change(&model);
}
