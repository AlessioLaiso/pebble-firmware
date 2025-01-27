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

#include "drivers/system_flash.h"

#include "drivers/dbgserial.h"
#include "util/misc.h"

#if defined(MICRO_FAMILY_STM32F2)
#include "stm32f2xx_flash.h"
#elif defined(MICRO_FAMILY_STM32F4)
#include "stm32f4xx_flash.h"
#endif

static uint16_t s_sectors[] = {
  FLASH_Sector_0, FLASH_Sector_1, FLASH_Sector_2, FLASH_Sector_3,
  FLASH_Sector_4, FLASH_Sector_5, FLASH_Sector_6, FLASH_Sector_7,
  FLASH_Sector_8, FLASH_Sector_9, FLASH_Sector_10, FLASH_Sector_11 };
static uint32_t s_sector_addresses[] = {
  ADDR_FLASH_SECTOR_0, ADDR_FLASH_SECTOR_1, ADDR_FLASH_SECTOR_2,
  ADDR_FLASH_SECTOR_3, ADDR_FLASH_SECTOR_4, ADDR_FLASH_SECTOR_5,
  ADDR_FLASH_SECTOR_6, ADDR_FLASH_SECTOR_7, ADDR_FLASH_SECTOR_8,
  ADDR_FLASH_SECTOR_9, ADDR_FLASH_SECTOR_10, ADDR_FLASH_SECTOR_11 };

int prv_get_sector_num_for_address(uint32_t address) {
  if (address < s_sector_addresses[0]) {
    dbgserial_print("address ");
    dbgserial_print_hex(address);
    dbgserial_print(" is outside system flash\r\n");
    return -1;
  }
  for (size_t i=0; i < ARRAY_LENGTH(s_sector_addresses)-1; ++i) {
    if (s_sector_addresses[i] <= address
        && address < s_sector_addresses[i+1]) {
      return i;
    }
  }
  return ARRAY_LENGTH(s_sector_addresses)-1;
}

bool system_flash_erase(
    uint32_t address, size_t length,
    SystemFlashProgressCb progress_callback, void *progress_context) {
  dbgserial_print("system_flash_erase(");
  dbgserial_print_hex(address);
  dbgserial_print(", ");
  dbgserial_print_hex(length);
  dbgserial_print(")\r\n");

  if (length == 0) {
    // Nothing to do
    return true;
  }

  int first_sector = prv_get_sector_num_for_address(address);
  int last_sector = prv_get_sector_num_for_address(address + length - 1);
  if (first_sector < 0 || last_sector < 0) {
    return false;
  }
  int count = last_sector - first_sector + 1;
  if (progress_callback) {
    progress_callback(0, count, progress_context);
  }

  FLASH_Unlock();
  for (int sector = first_sector; sector <= last_sector; ++sector) {
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    if (FLASH_EraseSector(
          s_sectors[sector], VoltageRange_1) != FLASH_COMPLETE) {
      dbgserial_print("failed to erase sector ");
      dbgserial_print_hex(sector);
      dbgserial_putstr("");
      FLASH_Lock();
      return false;
    }
    if (progress_callback) {
      progress_callback(sector - first_sector + 1, count, progress_context);
    }
  }
  FLASH_Lock();
  return true;
}

bool system_flash_write(
    uint32_t address, const void *data, size_t length,
    SystemFlashProgressCb progress_callback, void *progress_context) {
  dbgserial_print("system_flash_write(");
  dbgserial_print_hex(address);
  dbgserial_print(", ");
  dbgserial_print_hex(length);
  dbgserial_print(")\r\n");

  FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);

  const uint8_t *data_array = data;
  for (uint32_t i = 0; i < length; ++i) {
    if (FLASH_ProgramByte(address + i, data_array[i]) != FLASH_COMPLETE) {
      dbgserial_print("failed to write address ");
      dbgserial_print_hex(address);
      dbgserial_putstr("");
      FLASH_Lock();
      return false;
    }
    if (progress_callback && i % 128 == 0) {
      progress_callback(i/128, length/128, progress_context);
    }
  }
  FLASH_Lock();
  return true;
}

uint32_t system_flash_read(uint32_t address) {
  uint32_t data = *(volatile uint32_t*) address;
  return data;
}
