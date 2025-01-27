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

#include "apps/system_apps/timeline/timeline.h"
#include "resource/resource.h"
#include "resource/resource_ids.auto.h"
#include "services/normal/timeline/timeline_resources.h"

#include "test_timeline_app_includes.h"

// Setup and Teardown
////////////////////////////////////

typedef struct TimelineTestData {
  TimelineModel model;
} TimelineTestData;

TimelineTestData s_data;

void test_timeline_list_view__initialize(void) {
  fake_app_state_init();
  load_system_resources_fixture();

  s_data = (TimelineTestData) {};
  rtc_set_time(3 * SECONDS_PER_DAY);
}

void test_timeline_list_view__cleanup(void) {
}

// Helpers
//////////////////////

typedef struct TimelineItemConfig {
  time_t relative_timestamp;
  uint16_t duration;
  const char *title;
  const char *subtitle;
  TimelineResourceId icon;
} TimelineItemConfig;

typedef struct ListViewConfig {
  TimelineItemConfig *pins[2];
  bool past;
  bool day_separator;
} ListViewConfig;

static void prv_add_timeline_item(const TimelineItemConfig *config, bool past) {
  PBL_ASSERTN(config);
  TimelineItem *item = NULL;
  const time_t now = rtc_get_time();
  const time_t timestamp = now + ((past ? -1 : 1) * config->relative_timestamp);
  if (config) {
    AttributeList list;
    attribute_list_init_list(0 /* num_attributes */, &list);
    attribute_list_add_cstring(&list, AttributeIdTitle, config->title);
    if (config->subtitle) {
      attribute_list_add_cstring(&list, AttributeIdSubtitle, config->subtitle);
    }
    attribute_list_add_uint32(&list, AttributeIdIconPin, config->icon);
    item = timeline_item_create_with_attributes(timestamp, config->duration,
                                                TimelineItemTypePin, LayoutIdGeneric,
                                                &list, NULL);
    attribute_list_destroy_list(&list);
    PBL_ASSERTN(item);
  }
  if (item) {
    pin_db_insert_item(item);
    timeline_item_destroy(item);
  }
}

static void prv_create_list_view_and_render(ListViewConfig *config) {
  pin_db_init();

  for (int i = 0; i < (int)ARRAY_LENGTH(config->pins); i++) {
    if (config->pins[i]) {
      prv_add_timeline_item(config->pins[i], config->past);
    }
  }

  s_data.model = (TimelineModel) {};
  s_data.model.direction = config->past ? TimelineIterDirectionPast : TimelineIterDirectionFuture;
  timeline_model_init(rtc_get_time(), &s_data.model);

  Window window;
  window_init(&window, "Timeline");

  TimelineLayer timeline_layer = {};
  const TimelineScrollDirection scroll_direction =
      config->past ? TimelineScrollDirectionUp : TimelineScrollDirectionDown;
  timeline_layer_init(&timeline_layer, &window.layer.frame, scroll_direction);
  const GColor color = config->past ? TIMELINE_PAST_COLOR : TIMELINE_FUTURE_COLOR;
  timeline_layer_set_sidebar_color(&timeline_layer, color);
  timeline_layer_set_sidebar_width(&timeline_layer, timeline_layer_get_ideal_sidebar_width());
  layer_add_child(&window.layer, &timeline_layer.layer);
  timeline_layer_reset(&timeline_layer);

  if (config->day_separator) {
    // Simulate showing the day separator
    int new_idx;
    bool has_new;
    cl_assert(timeline_model_iter_next(&new_idx, &has_new));
    if (has_new) {
      timeline_layer_set_next_item(&timeline_layer, new_idx);
    }
    timeline_layer_move_data(&timeline_layer, 1);
    cl_assert(timeline_layer_should_animate_day_separator(&timeline_layer));
    fake_animation_complete(timeline_layer_create_day_sep_show(&timeline_layer));
    fake_animation_complete(timeline_layer.day_separator.kino_layer.player.animation);
    timeline_layer_set_layouts_hidden(&timeline_layer, true);
  }

  window_set_on_screen(&window, true, true);
  window_render(&window, fake_graphics_context_get_context());

  timeline_layer_deinit(&timeline_layer);
  timeline_model_deinit();
  pin_db_flush();
}

// Tests
//////////////////////

static void prv_create_and_render_title_and_subtitle(bool past, uint16_t first_duration_m) {
  prv_create_list_view_and_render(&(ListViewConfig) {
    .pins = {
      &(TimelineItemConfig) {
        .relative_timestamp = (11 * SECONDS_PER_HOUR) + (30 * SECONDS_PER_MINUTE),
        .duration = first_duration_m,
        .title = "Jon Byrd birthday party",
        .subtitle = "Kaboom, Redwood City",
        .icon = TIMELINE_RESOURCE_TIMELINE_CALENDAR,
      }, &(TimelineItemConfig) {
        .relative_timestamp = 12 * SECONDS_PER_HOUR,
        .duration = MINUTES_PER_HOUR,
        .title = "Design Review Meeting",
        .subtitle = "Batavia, Palo Alto",
        .icon = TIMELINE_RESOURCE_TIMELINE_CALENDAR,
      }
    },
    .past = past,
  });
}

void test_timeline_list_view__title_and_subtitle_overlap_future(void) {
#if !PLATFORM_SPALDING
  prv_create_and_render_title_and_subtitle(false /* past */, MINUTES_PER_HOUR);
  FAKE_GRAPHICS_CONTEXT_CHECK_DEST_BITMAP_FILE();
#endif
}

void test_timeline_list_view__title_and_subtitle_back_to_back_future(void) {
#if !PLATFORM_SPALDING
  prv_create_and_render_title_and_subtitle(false /* past */, MINUTES_PER_HOUR / 2);
  FAKE_GRAPHICS_CONTEXT_CHECK_DEST_BITMAP_FILE();
#endif
}

void test_timeline_list_view__title_and_subtitle_free_time_future(void) {
  prv_create_and_render_title_and_subtitle(false /* past */, 5 /* first_duration_m */);
  FAKE_GRAPHICS_CONTEXT_CHECK_DEST_BITMAP_FILE();
}

void test_timeline_list_view__title_and_subtitle_free_time_past(void) {
  prv_create_and_render_title_and_subtitle(true /* past */, 5 /* first_duration_m */);
  FAKE_GRAPHICS_CONTEXT_CHECK_DEST_BITMAP(TEST_PBI_FILE);
}

void prv_create_and_render_pin_and_dot(bool past) {
  prv_create_list_view_and_render(&(ListViewConfig) {
    .pins = {
      &(TimelineItemConfig) {
        .relative_timestamp = (11 * SECONDS_PER_HOUR) + (30 * SECONDS_PER_MINUTE),
        .duration = MINUTES_PER_HOUR,
        .title = "Jon Byrd birthday party",
        .subtitle = "Kaboom, Redwood City",
        .icon = TIMELINE_RESOURCE_TIMELINE_CALENDAR,
      }, &(TimelineItemConfig) {
        .relative_timestamp = SECONDS_PER_DAY + SECONDS_PER_HOUR,
        .duration = MINUTES_PER_HOUR,
        .title = "Design Review Meeting",
        .subtitle = "Batavia, Palo Alto",
        .icon = TIMELINE_RESOURCE_TIMELINE_CALENDAR,
      }
    },
    .past = past,
  });
}

void test_timeline_list_view__pin_and_dot_future(void) {
  prv_create_and_render_pin_and_dot(false /* past */);
  FAKE_GRAPHICS_CONTEXT_CHECK_DEST_BITMAP_FILE();
}

void test_timeline_list_view__pin_and_dot_past(void) {
  prv_create_and_render_pin_and_dot(true /* past */);
  FAKE_GRAPHICS_CONTEXT_CHECK_DEST_BITMAP_FILE();
}

void prv_create_and_render_day_sep_tomorrow(bool past) {
  prv_create_list_view_and_render(&(ListViewConfig) {
    .pins = {
      &(TimelineItemConfig) {
        .relative_timestamp = (11 * SECONDS_PER_HOUR) + (30 * SECONDS_PER_MINUTE),
        .duration = MINUTES_PER_HOUR,
        .title = "Jon Byrd birthday party",
        .subtitle = "Kaboom, Redwood City",
        .icon = TIMELINE_RESOURCE_TIMELINE_CALENDAR,
      }, &(TimelineItemConfig) {
        .relative_timestamp = ((11 * SECONDS_PER_HOUR) + (30 * SECONDS_PER_MINUTE) +
                               SECONDS_PER_DAY),
        .duration = MINUTES_PER_HOUR,
        .title = "Design Review Meeting",
        .subtitle = "Batavia, Palo Alto",
        .icon = TIMELINE_RESOURCE_TIMELINE_CALENDAR,
      }
    },
    .past = past,
    .day_separator = true,
  });
}

void test_timeline_list_view__day_sep_tomorrow_future(void) {
  prv_create_and_render_day_sep_tomorrow(false /* past */);
  FAKE_GRAPHICS_CONTEXT_CHECK_DEST_BITMAP_FILE();
}

void test_timeline_list_view__day_sep_tomorrow_past(void) {
  prv_create_and_render_day_sep_tomorrow(true /* past */);
  FAKE_GRAPHICS_CONTEXT_CHECK_DEST_BITMAP_FILE();
}

void prv_create_and_render_pin_and_fin(bool past) {
  prv_create_list_view_and_render(&(ListViewConfig) {
    .pins = {
      &(TimelineItemConfig) {
        .relative_timestamp = (11 * SECONDS_PER_HOUR) + (30 * SECONDS_PER_MINUTE),
        .title = "Jon Byrd birthday party",
        .duration = MINUTES_PER_HOUR,
        .subtitle = "Kaboom, Redwood City",
        .icon = TIMELINE_RESOURCE_TIMELINE_CALENDAR,
      }
    },
    .past = past,
  });
}

void test_timeline_list_view__pin_and_fin_future(void) {
  prv_create_and_render_pin_and_fin(false /* past */);
  FAKE_GRAPHICS_CONTEXT_CHECK_DEST_BITMAP_FILE();
}

void test_timeline_list_view__pin_and_fin_past(void) {
  prv_create_and_render_pin_and_fin(true /* past */);
  FAKE_GRAPHICS_CONTEXT_CHECK_DEST_BITMAP_FILE();
}

