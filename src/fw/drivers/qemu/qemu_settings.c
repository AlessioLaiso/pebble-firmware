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

#include "drivers/qemu/qemu_settings.h"
#include "system/passert.h"

#define STM32F2_COMPATIBLE
#define STM32F4_COMPATIBLE
#define STM32F7_COMPATIBLE
#include <mcu.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>


// QEMU backup registers and bit indices. These are also defined in the qemu project in
// hw/arm/pebble.c
#define QEMU_REG_0_FIRST_BOOT_LOGIC_ENABLE   0x00000001
#define QEMU_REG_0_DEFAULT_CONNECTED         0x00000002
#define QEMU_REG_0_DEFAULT_PLUGGED_IN        0x00000004

// -------------------------------------------------------------------------------------
// Read a QEMU specific register from the RTC backup register area
static uint32_t prv_rtc_read_qemu_register(uint32_t qemu_register) {
  __IO uint32_t tmp = 0;

  // The first qemu_register (0) starts 1 past the implemented registers in the STM
  uint32_t  backup_reg = RTC_BKP_DR19 + 1 + qemu_register;

  tmp = RTC_BASE + 0x50;
  tmp += (backup_reg * 4);

  // Read the specified register
  return (*(__IO uint32_t *)tmp);
}


// -------------------------------------------------------------------------------------
// Return the value of a QEMU setting. QEMU communicates these by setting values into an
// unused area of the RTC registers, what would be RTC_BKP20R on up.
uint32_t qemu_setting_get(QemuSetting setting) {
  switch (setting) {
    case QemuSetting_FirstBootLogicEnable:
      return prv_rtc_read_qemu_register(0) & QEMU_REG_0_FIRST_BOOT_LOGIC_ENABLE;
      break;

    case QemuSetting_DefaultConnected:
      return prv_rtc_read_qemu_register(0) & QEMU_REG_0_DEFAULT_CONNECTED;
      break;

    case QemuSetting_DefaultPluggedIn:
      return prv_rtc_read_qemu_register(0) & QEMU_REG_0_DEFAULT_PLUGGED_IN;
      break;

    default:
      WTF;
  }
}
