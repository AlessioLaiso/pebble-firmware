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

#include "drivers/gpio.h"

#include <stdbool.h>

typedef enum {
  CMD_FRAME_BEGIN = 0x5,
} DisplayCmd;



void display_spi_begin_transaction(void);
void display_spi_end_transaction(void);
void display_spi_configure_default(void);
bool display_busy(void);
void display_start(void);
void display_program(const uint8_t *fpga_bitstream, uint32_t bitstream_size);
void display_send_clocks(uint32_t count);
void display_start_frame(void);
void display_write_byte(uint8_t d);
void display_send_cmd(DisplayCmd cmd);
void display_power_enable(void);
void display_power_disable(void);

//! Reset the FPGA into bootloader mode.
//!
//! @return true if successful, false if the NVCM is not programmed.
bool display_switch_to_bootloader_mode(void);
