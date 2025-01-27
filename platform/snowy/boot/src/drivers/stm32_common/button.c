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

#include "drivers/button.h"

#include "board/board.h"
#include "drivers/periph_config.h"
#include "drivers/gpio.h"

static void initialize_button_common(void) {
  if (!BOARD_CONFIG_BUTTON.button_com.gpio) {
    // This board doesn't use a button common pin.
    return;
  }

  // Configure BUTTON_COM to drive low. When the button
  // is pressed this pin will be connected to the pin for the
  // button.
  gpio_use(BOARD_CONFIG_BUTTON.button_com.gpio);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_StructInit(&GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = BOARD_CONFIG_BUTTON.button_com.gpio_pin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(BOARD_CONFIG_BUTTON.button_com.gpio, &GPIO_InitStructure);
  GPIO_WriteBit(BOARD_CONFIG_BUTTON.button_com.gpio, BOARD_CONFIG_BUTTON.button_com.gpio_pin, 0);

  gpio_release(BOARD_CONFIG_BUTTON.button_com.gpio);
}

static void initialize_button(const ButtonConfig* config) {
  // Configure the pin itself
  gpio_use(config->gpio);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_StructInit(&GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = config->pull;
  GPIO_InitStructure.GPIO_Pin = config->gpio_pin;
  GPIO_Init(config->gpio, &GPIO_InitStructure);

  gpio_release(config->gpio);
}

bool button_is_pressed(ButtonId id) {
  const ButtonConfig* button_config = &BOARD_CONFIG_BUTTON.buttons[id];
  gpio_use(button_config->gpio);
  uint8_t bit = GPIO_ReadInputDataBit(button_config->gpio, button_config->gpio_pin);
  gpio_release(button_config->gpio);
  return !bit;
}

uint8_t button_get_state_bits(void) {
  uint8_t button_state = 0x00;
  for (int i = 0; i < NUM_BUTTONS; ++i) {
    button_state |= (button_is_pressed(i) ? 0x01 : 0x00) << i;
  }
  return button_state;
}

void button_init(void) {
  periph_config_acquire_lock();

  periph_config_enable(RCC_APB2PeriphClockCmd, RCC_APB2Periph_SYSCFG);

  initialize_button_common();
  for (int i = 0; i < NUM_BUTTONS; ++i) {
    initialize_button(&BOARD_CONFIG_BUTTON.buttons[i]);
  }

  periph_config_disable(RCC_APB2PeriphClockCmd, RCC_APB2Periph_SYSCFG);

  periph_config_release_lock();
}

bool button_selftest(void) {
  return button_get_state_bits() == 0;
}
