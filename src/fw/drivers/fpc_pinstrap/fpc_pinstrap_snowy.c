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

#include "drivers/fpc_pinstrap.h"

#include "board/board.h"
#include "drivers/gpio.h"

static uint8_t prv_read_pinstrap_pin(InputConfig pin) {
  // Read the pin value with it pulled up
  gpio_input_init_pull_up_down(&pin, GPIO_PuPd_UP);
  const bool pull_up_value = gpio_input_read(&pin);

  // If the pull up was high, that either means it's actually high or floating. Read the
  // pin again with a pull down to differentiate.

  gpio_input_init_pull_up_down(&pin, GPIO_PuPd_DOWN);
  const bool pull_down_value = gpio_input_read(&pin);

  // Reset the pin to an analog input when we're not using it to reduce power draw.
  gpio_analog_init(&pin);

  if (pull_down_value != pull_up_value) {
    // The value changed based on the pullup so it's floating.
    return 2;
  }
  // It's not floating, return what the initial read told us.
  return pull_up_value ? 1 : 0;
}

uint8_t fpc_pinstrap_get_value(void) {
  // This is an uncommon operation so just configure the GPIOs as needed.

  if (BOARD_CONFIG.fpc_pinstrap_1.gpio == GPIO_Port_NULL) {
    return FPC_PINSTRAP_NOT_AVAILABLE;
  }

  return (prv_read_pinstrap_pin(BOARD_CONFIG.fpc_pinstrap_1) * 3)
         + prv_read_pinstrap_pin(BOARD_CONFIG.fpc_pinstrap_2);
}
