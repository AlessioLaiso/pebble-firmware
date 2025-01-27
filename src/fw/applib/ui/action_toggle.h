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

#include "applib/graphics/gtypes.h"
#include "resource/resource_ids.auto.h"

typedef bool (*ActionToggleGetStateCallback)(void *context);
typedef void (*ActionToggleSetStateCallback)(bool enabled, void *context);

typedef enum ActionToggleState {
  ActionToggleState_Disabled = 0,
  ActionToggleState_Enabled,

  ActionToggleStateCount,
} ActionToggleState;

typedef enum ActionToggleDialogType {
  ActionToggleDialogType_Prompt = 0,
  ActionToggleDialogType_Result,

  ActionToggleDialogTypeCount,
} ActionToggleDialogType;

typedef enum ActionTogglePrompt {
  ActionTogglePrompt_Auto = 0,
  ActionTogglePrompt_NoPrompt,
  ActionTogglePrompt_Prompt,
} ActionTogglePrompt;

typedef struct ActionToggleCallbacks {
  ActionToggleGetStateCallback get_state;
  ActionToggleSetStateCallback set_state;
} ActionToggleCallbacks;

typedef struct ActionToggleImpl {
  ActionToggleCallbacks callbacks;
  const char *window_name;
  union {
    struct {
      const char *prompt_disable_message;
      const char *prompt_enable_message;
    };
    const char *prompt_messages[ActionToggleStateCount];
  };
  union {
    struct {
      const char *result_disable_message;
      const char *result_enable_message;
    };
    const char *result_messages[ActionToggleStateCount];
  };
  union {
    struct {
      ResourceId prompt_icon;
      ResourceId result_icon;
    };
    ResourceId icons[ActionToggleDialogTypeCount];
  };
  bool result_icon_static;
} ActionToggleImpl;

typedef struct ActionToggleConfig {
  const ActionToggleImpl *impl;
  void *context;
  ActionTogglePrompt prompt;
  bool set_exit_reason;
} ActionToggleConfig;

//! Pushes either a prompt or result dialog depending on the prompt config option. If a prompt
//! dialog is requested, the result dialog will be pushed if the user confirms the prompt dialog
//! and the new toggled state would be set. Otherwise, a result dialog is unconditionally pushed
//! and the new toggled state is set.
//! @param config The action toggle configuration.
void action_toggle_push(const ActionToggleConfig *config);
