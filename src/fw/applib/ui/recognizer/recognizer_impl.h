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

#include "recognizer.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct RecognizerImpl {
  //! Handle touch event
  //! @note This function must be implemented
  //! @param recognizer recognizer to handle event
  //! @param touch_event touch event that just occurred
  void (*handle_touch_event)(Recognizer *recognizer, const TouchEvent *touch_event);

  //! Cancel the recognizer
  //! @note This function must be implemented
  //! @param recognizer recognizer to be reset
  //! @return true if event should be fired, otherwise false
  bool (*cancel)(Recognizer *recognizer);

  //! Reset the recognizer
  //! @note This function must be implemented
  //! @param recognizer recognizer to be reset
  void (*reset)(Recognizer *recognizer);

  //! Called when the recognizer is failed by a manager. Used to clean up any timers or otherwise
  //! stop further recognition activity until the recognizer is reset.
  //! @param recognizer recognizer that failed
  void (*on_fail)(Recognizer *recognizer);

  //! Called when the recognizer is destroyed
  //! @param recognizer recognizer that will be destroyed
  void (*on_destroy)(Recognizer *recognizer);
} RecognizerImpl;

//! Create a recognizer with implementation specific data. This is used by internal and custom
//! recognizers to instantiate a recognizer from the base class. A recognizer created from this
//! function cannot be used without an implementation.
//! @note A recognizer cannot be created without implementation details
//! @param data recognizer-specific data (copied into created recognizer)
//! @param size data size
//! @param event_cb event callback
//! @param event_context context to provide to event callback
//! @return NULL if an error occurs, otherwise a pointer to the newly created recognizer
Recognizer *recognizer_create_with_data(const RecognizerImpl *impl, const void *data,
                                        size_t data_size, RecognizerEventCb event_cb,
                                        void *user_data);

//! Get the implementation specific data for the recognizer. If the implementation specified does
//! not match the implementation belonging to the recognizer, NULL is returned
//! @param recognizer recognizer
//! @param impl pointer to implementation definition
//! @return pointer to implementation-specific data, or NULL if incorrect implementation specified
void *recognizer_get_impl_data(Recognizer *recognizer, const RecognizerImpl *impl);

//! Transition the recognizer state. This is called by the implementation to change the state of the
//! recognizer when it needs to update its state. It cannot not be called by anything else. The
//! state transition must be valid
//! @param recognizer recognizer to modify
//! @param new_state new recognizer state
void recognizer_transition_state(Recognizer *recognizer, RecognizerState new_state);
