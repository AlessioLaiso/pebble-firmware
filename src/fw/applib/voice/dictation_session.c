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

#include "dictation_session.h"
#include "dictation_session_private.h"
#include "voice_window_private.h"

#include "applib/voice/voice_window.h"
#include "applib/applib_malloc.auto.h"
#include "process_management/app_install_manager.h"
#include "syscall/syscall.h"
#include "system/logging.h"
#include "system/passert.h"

#include <string.h>

#if CAPABILITY_HAS_MICROPHONE

static void prv_handle_transcription_result(PebbleEvent *e, void *context) {
  PBL_ASSERTN(context);

  PBL_LOG(LOG_LEVEL_DEBUG, "Exiting with status code: %"PRId8, e->dictation.result);
  DictationSession *session = context;

  session->callback(session, e->dictation.result, e->dictation.text, session->context);
  voice_window_reset(session->voice_window);
  session->in_progress = false;

  if (session->destroy_pending) {
    dictation_session_destroy(session);
  }
}

static void prv_app_focus_handler(PebbleEvent *e, void *context) {
  DictationSession *session = context;
  if (e->app_focus.in_focus) {
    event_service_client_subscribe(&session->dictation_result_sub);
    voice_window_regain_focus(session->voice_window);
  } else {
    event_service_client_unsubscribe(&session->dictation_result_sub);
    voice_window_lose_focus(session->voice_window);
  }
}

static void prv_stop_session(DictationSession *session) {
  session->in_progress = false;
  event_service_client_unsubscribe(&session->dictation_result_sub);
  if (pebble_task_get_current() == PebbleTask_App) {
    event_service_client_unsubscribe(&session->app_focus_sub);
  }
}

#endif

DictationSession *dictation_session_create(uint32_t buffer_size,
                                           DictationSessionStatusCallback callback, void *context) {
#if CAPABILITY_HAS_MICROPHONE
  if (!callback) {
    return NULL;
  }

  // Old versions of the Android app (<3.5) will allow voice replies (which also use this code-path)
  // but don't set the capability flag, so we don't want to block all requests here, just those from
  // apps. This will result in apps not being able to use the voice APIs unless the phone has the
  // capability flag set, which is what we want.
  bool from_app = (pebble_task_get_current() == PebbleTask_App) &&
                   !app_install_id_from_system(sys_process_manager_get_current_process_id());
  if (from_app && !sys_system_pp_has_capability(CommSessionVoiceApiSupport)) {
    PBL_LOG(LOG_LEVEL_INFO, "No phone connected or phone app does not support app-initiated "
        "dictation sessions");
    return NULL;
  }

  DictationSession *session = applib_type_malloc(DictationSession);
  if (!session) {
    return NULL;
  }

  char *buffer = NULL;
  if (buffer_size > 0) {
    buffer = applib_malloc(buffer_size);
    if (!buffer) {
      applib_free(session);
      return NULL;
    }
  }

  VoiceWindow *voice_window = voice_window_create(buffer, buffer_size,
                                                  VoiceEndpointSessionTypeDictation);
  if (!voice_window) {
    applib_free(buffer);
    applib_free(session);
    return NULL;
  }

  *session = (DictationSession) {
    .callback = callback,
    .context = context,
    .voice_window = voice_window,
    .dictation_result_sub = (EventServiceInfo) {
      .type = PEBBLE_DICTATION_EVENT,
      .handler = prv_handle_transcription_result,
      .context = session
    }
  };

  if (pebble_task_get_current() == PebbleTask_App) {
    session->app_focus_sub = (EventServiceInfo) {
        .type = PEBBLE_APP_DID_CHANGE_FOCUS_EVENT,
        .handler = prv_app_focus_handler,
        .context = session
    };
  }
#else
  DictationSession *session = NULL;
#endif

  return session;
}

void dictation_session_destroy(DictationSession *session) {
#if CAPABILITY_HAS_MICROPHONE
  if (!session) {
    return;
  }

  if (session->in_progress) {
    // we can't destroy a session while it is in progress,
    // so we mark it as destroy pending and we'll destroy it later
    session->destroy_pending = true;
    return;
  }

  prv_stop_session(session);
  voice_window_destroy(session->voice_window);
  applib_free(session);
#endif
}

void dictation_session_enable_confirmation(DictationSession *session, bool is_enabled) {
#if CAPABILITY_HAS_MICROPHONE
  if (!session || session->in_progress) {
    return;
  }
  voice_window_set_confirmation_enabled(session->voice_window, is_enabled);
#endif
}

void dictation_session_enable_error_dialogs(DictationSession *session, bool is_enabled) {
#if CAPABILITY_HAS_MICROPHONE
  if (!session || session->in_progress) {
    return;
  }
  voice_window_set_error_enabled(session->voice_window, is_enabled);
#endif
}

DictationSessionStatus dictation_session_start(DictationSession *session) {
#if CAPABILITY_HAS_MICROPHONE
  if (!session || session->in_progress) {
    return DictationSessionStatusFailureInternalError;
  }

  DictationSessionStatus result = voice_window_push(session->voice_window);
  if (result != DictationSessionStatusSuccess) {
    return result;
  }

  session->in_progress = true;
  event_service_client_subscribe(&session->dictation_result_sub);
  if (pebble_task_get_current() == PebbleTask_App) {
    event_service_client_subscribe(&session->app_focus_sub);
  }
  return result;
#else
  return DictationSessionStatusFailureInternalError;
#endif
}

DictationSessionStatus dictation_session_stop(DictationSession *session) {
#if CAPABILITY_HAS_MICROPHONE
  if (!session || !session->in_progress) {
    return DictationSessionStatusFailureInternalError;
  }
  prv_stop_session(session);
  voice_window_pop(session->voice_window);
  return DictationSessionStatusSuccess;
#else
  return DictationSessionStatusFailureInternalError;
#endif
}
