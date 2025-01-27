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

#include "weather_app.h"
#include "weather_app_layout.h"
#include "weather_app_splash_screen.h"
#include "weather_app_warning_dialog.h"

#include "applib/app.h"
#include "applib/event_service_client.h"
#include "applib/ui/click.h"
#include "applib/ui/content_indicator.h"
#include "applib/ui/ui.h"
#include "kernel/events.h"
#include "kernel/pbl_malloc.h"
#include "process_management/pebble_process_md.h"
#include "process_state/app_state/app_state.h"
#include "resource/resource_ids.auto.h"
#include "services/common/i18n/i18n.h"
#include "services/normal/timeline/timeline.h"
#include "services/normal/weather/weather_service.h"
#include "services/normal/weather/weather_types.h"
#include "util/array.h"
#include "util/attributes.h"
#include "util/list.h"
#include "util/math.h"

typedef struct WeatherAppData {
  Window window;
  WeatherAppLayout layout;
  WeatherDataListNode *forecasts_list_head;
  size_t forecasts_count;
  unsigned int current_forecast_index;
  EventServiceInfo weather_event_info;
  WeatherAppWarningDialog *warning_dialog;
} WeatherAppData;

static bool prv_is_weather_forecast_recent(WeatherLocationForecast *forecast) {
  if (!forecast) {
    return false;
  }
  const time_t current_time_utc = rtc_get_time();
  const int recent_threshold_seconds = 5 * SECONDS_PER_HOUR / 2; // 2.5 hours
  const int seconds_since_forecast_was_updated = current_time_utc - forecast->time_updated_utc;
  return (seconds_since_forecast_was_updated < recent_threshold_seconds);
}

static void prv_warning_dialog_dismiss_cb(void) {
  WeatherAppData *data = app_state_get_user_data();
  data->warning_dialog = NULL;
}

static void prv_show_warning_dialog(WeatherAppData *data, bool exit_on_pop,
                                    const char *localized_text) {
  if (data->warning_dialog) {
    return; // only show one dialog at a time
  }
  if (exit_on_pop) {
    bool animated = false;
    app_window_stack_pop_all(animated);
  }
  data->warning_dialog = weather_app_warning_dialog_push(localized_text,
                                                         prv_warning_dialog_dismiss_cb);
}

static void prv_handle_weather(PebbleEvent *unused_event, void *unused_context) {
  // Unschedule any ongoing animations that would try to touch the weather data we're about to
  // update
  animation_unschedule_all();

  size_t forecasts_count_out = 0;
  WeatherDataListNode *forecasts_list_head =
      weather_service_locations_list_create(&forecasts_count_out);

  WeatherAppData *data = app_state_get_user_data();
  weather_service_locations_list_destroy(data->forecasts_list_head);
  WeatherAppLayout *layout = &data->layout;
  if (forecasts_count_out > 0) {
    weather_app_layout_set_data(layout, &forecasts_list_head->forecast);
    const bool multiple_forecasts_exist = (forecasts_count_out > 1);
    weather_app_layout_set_down_arrow_visible(layout, multiple_forecasts_exist);

    data->forecasts_list_head = forecasts_list_head;
    // Only show the first forecast if the number of forecasts has differed between fetches.
    // i.e. assume that the same number of forecasts means the locations have remained the same.
    if (data->forecasts_count != forecasts_count_out) {
      data->forecasts_count = forecasts_count_out;
      data->current_forecast_index = 0;
    }
  } else {
    /// Shown when there are no forecasts available to show the user
    const char *warning_text = i18n_get("No location information available. To see weather, add "\
                                        "locations in your Pebble mobile app.", data);
    const bool exit_on_pop = true;
    prv_show_warning_dialog(data, exit_on_pop, warning_text);
    weather_app_layout_set_down_arrow_visible(layout, false);
    weather_app_layout_set_data(layout, NULL);
  }
}

static void prv_main_window_appear(Window *window) {
  WeatherAppData *data = app_state_get_user_data();
  data->weather_event_info = (EventServiceInfo) {
    .type = PEBBLE_WEATHER_EVENT,
    .handler = prv_handle_weather,
  };
  event_service_client_subscribe(&data->weather_event_info);
}

static void prv_main_window_load(Window *window) {
  WeatherAppData *data = app_state_get_user_data();
  layer_add_child(&window->layer, &data->layout.root_layer);
}

static void prv_main_window_disappear(Window *window) {
  WeatherAppData *data = app_state_get_user_data();
  event_service_client_unsubscribe(&data->weather_event_info);
}

static void prv_up_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  WeatherAppData *data = app_state_get_user_data();
  const bool not_enough_items_to_scroll = (data->forecasts_count <= 1);
  if (not_enough_items_to_scroll) {
    return;
  }

  const bool is_down_pressed = (click_recognizer_get_button_id(recognizer) == BUTTON_ID_DOWN);
  const int delta = is_down_pressed ? 1 : -1;
  data->current_forecast_index = positive_modulo(data->current_forecast_index + delta,
                                                 data->forecasts_count);

  WeatherDataListNode *node =
      weather_service_locations_list_get_location_at_index(data->forecasts_list_head,
                                                           data->current_forecast_index);
  weather_app_layout_animate(&data->layout, &node->forecast, is_down_pressed);
}

static void prv_main_window_click_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_up_down_click_handler);
}

static void prv_main_window_unload(Window *window) {
  WeatherAppData *data = app_state_get_user_data();
  weather_app_layout_deinit(&data->layout);
}

static NOINLINE void prv_init(void) {
  WeatherAppData *data = app_zalloc_check(sizeof(WeatherAppData));
  app_state_set_user_data(data);

  Window *window = &data->window;
  window_init(window, WINDOW_NAME("Weather"));
  const WindowHandlers window_handlers = {
    .appear = prv_main_window_appear,
    .load = prv_main_window_load,
    .disappear = prv_main_window_disappear,
    .unload = prv_main_window_unload,
  };
  window_set_window_handlers(window, &window_handlers);

  window_set_click_config_provider(window, prv_main_window_click_provider);
  window_set_user_data(window, data);

  const GRect *layout_frame = &window->layer.bounds;
  WeatherAppLayout *layout = &data->layout;
  weather_app_layout_init(layout, layout_frame);
  window_set_user_data(window, layout);

  // Fetch initial data
  prv_handle_weather(NULL, NULL);

  if (data->forecasts_count == 0) {
    return;
  }

  const bool animated = true;
  app_window_stack_push(&data->window, animated);

  // Request the default forecast separately instead of using the forecast list in `data` to avoid
  // any potential race conditions
  WeatherLocationForecast *default_forecast = weather_service_create_default_forecast();
  const bool is_default_forecast_data_recent = prv_is_weather_forecast_recent(default_forecast);
  weather_service_destroy_default_forecast(default_forecast);

  // TODO PBL-38484: Consider using a different dialog for when data is stale but phone is connected
  if (is_default_forecast_data_recent || connection_service_peek_pebble_app_connection()) {
    const uint32_t splash_screen_timeout_ms = 500;
    weather_app_splash_screen_push(splash_screen_timeout_ms);
  } else {
    /// Shown when there is no connection to the phone and the data that we have is not recent
    const char *warning_text = i18n_get("Unable to connect. Your weather data may be out of date; "\
                                        "try checking the connection on your phone.", data);
    const bool exit_on_pop = false;
    prv_show_warning_dialog(data, exit_on_pop, warning_text);
  }
}

static void prv_deinit(void) {
  WeatherAppData *data = app_state_get_user_data();
  i18n_free_all(data);
}

static void prv_main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}

const PebbleProcessMd* weather_app_get_info() {
  const bool is_visible_in_launcher = weather_service_supported_by_phone();

  static const PebbleProcessMdSystem s_weather_app_info = {
    .common = {
      .main_func = prv_main,
      .uuid = UUID_WEATHER_DATA_SOURCE,
    },
    .name = i18n_noop("Weather"),
#if CAPABILITY_HAS_APP_GLANCES
    .icon_resource_id = RESOURCE_ID_GENERIC_WEATHER_TINY,
#endif
  };

  return is_visible_in_launcher ? (const PebbleProcessMd *)&s_weather_app_info : NULL;
}
