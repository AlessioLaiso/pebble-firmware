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

#include "util/misc.h"

#define USE_PARALLEL_FLASH
#define HAS_ACCESSORY_CONNECTOR
#define BOARD_HAS_PMIC

#define BOARD_I2C_BUS_COUNT (ARRAY_LENGTH(SNOWY_EVT2_I2C_BUS_CONFIGS))

extern void snowy_i2c_rail_1_ctl_fn(bool enable);

static const I2cBusConfig SNOWY_EVT2_I2C_BUS_CONFIGS[] = {
  // Listed as I2C_PMIC_MAG on the schematic, runs at 1.8V
  [0] = {
    .i2c = I2C1,
    .i2c_scl = { GPIOB, GPIO_Pin_6, GPIO_PinSource6, GPIO_AF_I2C1 },
    .i2c_sda = { GPIOB, GPIO_Pin_9, GPIO_PinSource9, GPIO_AF_I2C1 },
    .clock_speed = 400000,
    .duty_cycle = I2C_DutyCycle_16_9,
    .clock_ctrl = RCC_APB1Periph_I2C1,
    .ev_irq_channel = I2C1_EV_IRQn,
    .er_irq_channel = I2C1_ER_IRQn,
  },
  // Listed as I2C_MFI on the schematic, runs at 1.8V
  [1] = {
    .i2c = I2C2,
    .i2c_scl = { GPIOF, GPIO_Pin_1, GPIO_PinSource1, GPIO_AF_I2C2 },
    .i2c_sda = { GPIOF, GPIO_Pin_0, GPIO_PinSource0, GPIO_AF_I2C2 },
    .clock_speed = 400000,
    .duty_cycle = I2C_DutyCycle_2,
    .clock_ctrl = RCC_APB1Periph_I2C2,
    .ev_irq_channel = I2C2_EV_IRQn,
    .er_irq_channel = I2C2_ER_IRQn,
    .rail_ctl_fn = snowy_i2c_rail_1_ctl_fn
  }
};

static const uint8_t SNOWY_EVT2_I2C_DEVICE_MAP[] = {
  [I2C_DEVICE_MAG3110] = 0,
  [I2C_DEVICE_MFI] = 1,
  [I2C_DEVICE_MAX14690] = 0
};

static const BoardConfig BOARD_CONFIG = {
  .i2c_bus_configs = SNOWY_EVT2_I2C_BUS_CONFIGS,
  .i2c_bus_count = BOARD_I2C_BUS_COUNT,
  .i2c_device_map = SNOWY_EVT2_I2C_DEVICE_MAP,
  .i2c_device_count = ARRAY_LENGTH(SNOWY_EVT2_I2C_DEVICE_MAP),

  .has_ambient_light_sensor = true,
  .ambient_light_dark_threshold = 3000,
  .photo_en = { GPIOA, GPIO_Pin_3, true },
  .light_level = { GPIOA, GPIO_Pin_2, ADC_Channel_2 },

  .dbgserial_int = { EXTI_PortSourceGPIOC, 12 },

  .accessory_power_en = { GPIOF, GPIO_Pin_13, true },
  .accessory_rxtx_afcfg = { GPIOE, GPIO_Pin_1, GPIO_PinSource1, GPIO_AF_UART8 },
  .accessory_uart = UART8,
  .accessory_exti = { EXTI_PortSourceGPIOE, 0 },

  .bt_controller = CC2564B,
  .bt_shutdown = { GPIOB, GPIO_Pin_12, false},
  .bt_cts_int = { GPIOA, GPIO_Pin_11, false},
  .bt_cts_exti = { EXTI_PortSourceGPIOA, 11 },

  // Only used with Sharp displays
  .lcd_com = { 0 },

  .cdone_int = { EXTI_PortSourceGPIOG, 9 },
  .intn_int = { EXTI_PortSourceGPIOG, 10 },

  .power_5v0_options = OptionNotPresent,
  .power_ctl_5v0 = { 0 },

  .backlight_options = BacklightPinPwm,
  .backlight_ctl = { GPIOB, GPIO_Pin_14, true },
  .backlight_timer = {
    .peripheral = TIM12,
    .config_clock = RCC_APB1Periph_TIM12,
    .init = TIM_OC1Init,
    .preload = TIM_OC1PreloadConfig
  },
  .backlight_afcfg = { GPIOB, GPIO_Pin_14, GPIO_PinSource14, GPIO_AF_TIM12 },

  .has_mic = true,
  .mic_config = {
    .i2s_ck = { GPIOB, GPIO_Pin_10, GPIO_PinSource10, GPIO_AF_SPI2 },
    .i2s_sd = { GPIOB, GPIO_Pin_15, GPIO_PinSource15, GPIO_AF_SPI2 },
    .dma_stream = DMA1_Stream3,
    .dma_channel = DMA_Channel_0,
    .dma_channel_irq = DMA1_Stream3_IRQn,
    .dma_clock_ctrl = RCC_AHB1Periph_DMA1,
    .spi = SPI2,
    .spi_clock_ctrl = RCC_APB1Periph_SPI2,

    .mic_gpio_power = { GPIOF, GPIO_Pin_5, true }
  },
};

static const BoardConfigButton BOARD_CONFIG_BUTTON = {
  .buttons = {
    [BUTTON_ID_BACK]    = { "Back",   GPIOG, GPIO_Pin_4, { EXTI_PortSourceGPIOG, 4 }, GPIO_PuPd_NOPULL },
    [BUTTON_ID_UP]      = { "Up",     GPIOG, GPIO_Pin_3, { EXTI_PortSourceGPIOG, 3 }, GPIO_PuPd_UP },
    [BUTTON_ID_SELECT]  = { "Select", GPIOG, GPIO_Pin_1, { EXTI_PortSourceGPIOG, 1 }, GPIO_PuPd_UP },
    [BUTTON_ID_DOWN]    = { "Down",   GPIOG, GPIO_Pin_2, { EXTI_PortSourceGPIOG, 2 }, GPIO_PuPd_UP },
  },

  .button_com = { 0 },
};

static const BoardConfigPower BOARD_CONFIG_POWER = {
  .pmic_int = { EXTI_PortSourceGPIOG, 7 },

  .battery_vmon = { GPIOA, GPIO_Pin_1, ADC_Channel_1 },

  .vusb_stat = { .gpio = GPIO_Port_NULL, },
  .chg_stat = { GPIO_Port_NULL },
  .chg_fast = { GPIO_Port_NULL },
  .chg_en = { GPIO_Port_NULL },
  .has_vusb_interrupt = false,

  .wake_on_usb_power = false,

  .charging_status_led_voltage_compensation = 0,

  .low_power_threshold = 5,
};

static const BoardConfigVibe BOARD_CONFIG_VIBE = {
  .vibe_options = VibePinPwm,
  .vibe_ctl = { GPIOF, GPIO_Pin_4, true },
  .vibe_pwm = { GPIOB, GPIO_Pin_8, true },
  .vibe_timer = {
    .peripheral = TIM10,
    .config_clock = RCC_APB2Periph_TIM10,
    .init = TIM_OC1Init,
    .preload = TIM_OC1PreloadConfig
  },
  .vibe_afcfg = { GPIOB, GPIO_Pin_8, GPIO_PinSource8, GPIO_AF_TIM10 },
};
