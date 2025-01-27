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

#include "drivers/dbgserial.h"
#include "drivers/pwr.h"
#include "system/bootbits.h"
#include "system/rtc_registers.h"

#include "git_version.auto.h"

#include "stm32f7xx.h"

#include <inttypes.h>
#include <stdint.h>

static const uint32_t s_bootloader_timestamp = GIT_TIMESTAMP;

void boot_bit_init(void) {
  pwr_access_backup_domain(true);

  if (!boot_bit_test(BOOT_BIT_INITIALIZED)) {
    RTC_WriteBackupRegister(RTC_BKP_BOOTBIT_DR, BOOT_BIT_INITIALIZED);
  }
}

void boot_bit_set(BootBitValue bit) {
  uint32_t current_value = RTC_ReadBackupRegister(RTC_BKP_BOOTBIT_DR);
  current_value |= bit;
  RTC_WriteBackupRegister(RTC_BKP_BOOTBIT_DR, current_value);
}

void boot_bit_clear(BootBitValue bit) {
  uint32_t current_value = RTC_ReadBackupRegister(RTC_BKP_BOOTBIT_DR);
  current_value &= ~bit;
  RTC_WriteBackupRegister(RTC_BKP_BOOTBIT_DR, current_value);
}

bool boot_bit_test(BootBitValue bit) {
  uint32_t current_value = RTC_ReadBackupRegister(RTC_BKP_BOOTBIT_DR);
  return (current_value & bit);
}

void boot_bit_dump(void) {
  dbgserial_print("Boot bits: ");
  dbgserial_print_hex(RTC_ReadBackupRegister(RTC_BKP_BOOTBIT_DR));
  dbgserial_newline();
}

void boot_version_write(void) {
  if (boot_version_read() == s_bootloader_timestamp) {
    return;
  }
  RTC_WriteBackupRegister(BOOTLOADER_VERSION_REGISTER, s_bootloader_timestamp);
}

uint32_t boot_version_read(void) {
  return RTC_ReadBackupRegister(BOOTLOADER_VERSION_REGISTER);
}
