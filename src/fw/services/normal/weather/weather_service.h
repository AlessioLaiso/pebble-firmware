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

//! Weather Service
//!
//! The weather service manages the store of weather forecast data on the watch.
//! Forecast data and location data is sent from the phone to the watch. No
//! requests for data are made from the watch. Clients that wish to subscribe to weather database
//! changed events should use the PEBBLE_WEATHER_CHANGED_EVENT (see events.h)

#include "services/normal/blob_db/weather_db.h"
#include "services/normal/weather/weather_types.h"
#include "util/list.h"
#include "util/time/time.h"

#include <stdint.h>

#define WEATHER_SERVICE_MAX_SHORT_PHRASE_BUFFER_SIZE (32)
#define WEATHER_SERVICE_MAX_WEATHER_LOCATION_BUFFER_SIZE (64)
#define WEATHER_SERVICE_INVALID_DATA_LAST_UPDATE_TIME (0)
#define WEATHER_SERVICE_LOCATION_FORECAST_UNKNOWN_TEMP (INT16_MAX)

//! Unique handle for each weather location.
typedef int WeatherLocationID;

typedef struct WeatherLocationForecast {
  char *location_name;
  bool is_current_location;
  int current_temp;
  int today_high;
  int today_low;
  WeatherType current_weather_type;
  char *current_weather_phrase;
  int tomorrow_high;
  int tomorrow_low;
  WeatherType tomorrow_weather_type;
  time_t time_updated_utc;
} WeatherLocationForecast;

typedef struct WeatherDataListNode {
  ListNode node;
  WeatherLocationID id;
  WeatherLocationForecast forecast;
} WeatherDataListNode;

//! Initializes the weather service
void weather_service_init(void);

//! Retrieves the forecast for the default location in the database, if possible
//! @return a copy of the default location's forecast, or NULL
WeatherLocationForecast *weather_service_create_default_forecast(void);

//! Destroys the WeatherLocationForecast created with
//! weather_service_create_forecast_for_default_location
//! @param forecast The forecast to delete
void weather_service_destroy_default_forecast(WeatherLocationForecast *forecast);

//! Retrieves all valid weather records from weather_db and stores them in a list
//! List is guaranteed to be sorted by key
//! List must be destroyed by weather_destroy_locations_list
//! NOTE: ListNode and list.h are not exposed, so if this function becomes part of the public API,
//! refactoring will be needed.
//! @param count_out A pointer to the to the size_t used to store the number of records fetched
//! @return The head of the newly created list. May be NULL
WeatherDataListNode *weather_service_locations_list_create(size_t *count_out);

//! Retrieves the WeatherDataListNode at the specified index, given the head of the list
WeatherDataListNode *weather_service_locations_list_get_location_at_index(WeatherDataListNode *head,
                                                                          unsigned int index);

//! Destroys a weather locations list previously created with weather_get_head_of_all_locations_list
//! @param head The head of the list
void weather_service_locations_list_destroy(WeatherDataListNode *head);

//! Returns whether or not the phone has weather support
bool weather_service_supported_by_phone(void);
