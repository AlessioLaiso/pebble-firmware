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

// All graphics/UI includes needed for tests. Add here if more are needed.
#include "applib/app.h"
#include "applib/graphics/graphics.h"
#include "applib/graphics/text.h"
#include "applib/graphics/gtypes.h"
#include "applib/ui/layer.h"
#include "applib/ui/window.h"
#include "applib/ui/window_stack.h"
#include "applib/ui/text_layer.h"
#include "applib/ui/menu_layer.h"
#include "applib/fonts/fonts.h"
#include "util/trig.h"
#include "applib/graphics/gpath.h"

#include <stdint.h>

//! GFX test struct
typedef struct GfxTest {
  char *name;                         //!< Name string
  uint32_t duration;                  //!< Number of seconds to run the test for
  uint32_t unit_multiple;             //!< Number of actions per test iteration
  LayerUpdateProc test_proc;          //!< Test procedure
  void (*setup)(Window *window);      //!< Test setup function
  void (*teardown)( Window *window);  //!< Test teardown function
} GfxTest;

#include "process_management/pebble_process_md.h"

const PebbleProcessMd* gfx_tests_get_app_info(void);
