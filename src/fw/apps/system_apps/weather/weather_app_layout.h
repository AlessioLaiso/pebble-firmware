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

#include "applib/graphics/gdraw_command_image.h"
#include "applib/ui/content_indicator_private.h"
#include "applib/ui/kino/kino_layer.h"
#include "services/normal/weather/weather_service.h"

typedef struct WeatherAppLayout {
  Layer root_layer;
  Layer content_layer;
  KinoLayer current_weather_icon_layer;
  KinoLayer tomorrow_weather_icon_layer;
  const WeatherLocationForecast *forecast;
  GFont location_font;
  GFont temperature_font;
  GFont high_low_phrase_font;
  GFont tomorrow_font;
  Layer down_arrow_layer;
  ContentIndicator content_indicator;
  struct { // used during animations
    const WeatherLocationForecast *next_forecast;
    bool hide_bottom_half_text;
  } animation_state;
} WeatherAppLayout;

void weather_app_layout_init(WeatherAppLayout *layout, const GRect *frame);

void weather_app_layout_set_data(WeatherAppLayout *layout,
                                 const WeatherLocationForecast *forecast);

void weather_app_layout_set_down_arrow_visible(WeatherAppLayout *layout, bool is_down_visible);

void weather_app_layout_deinit(WeatherAppLayout *layout);

void weather_app_layout_animate(WeatherAppLayout *layout, WeatherLocationForecast *new_forecast,
                                bool animate_down);
