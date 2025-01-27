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

#include "applib/ui/window_private.h"
#include "util/size.h"

#include "clar.h"

// Fakes
/////////////////////

#include "fake_animation.h"
#include "fake_app_state.h"
#include "fake_content_indicator.h"
#include "fake_evented_timer.h"
#include "fake_graphics_context.h"
#include "fake_rtc.h"
#include "fake_spi_flash.h"
#include "fixtures/load_test_resources.h"

// Stubs
/////////////////////

#include "stubs_action_menu.h"
#include "stubs_activity.h"
#include "stubs_analytics.h"
#include "stubs_ancs.h"
#include "stubs_animation_timing.h"
#include "stubs_animation_timing.h"
#include "stubs_app.h"
#include "stubs_app_cache.h"
#include "stubs_app_install_manager.h"
#include "stubs_app_launch_reason.h"
#include "stubs_app_manager.h"
#include "stubs_app_timer.h"
#include "stubs_app_window_stack.h"
#include "stubs_blob_db.h"
#include "stubs_blob_db_sync.h"
#include "stubs_blob_db_sync_util.h"
#include "stubs_bootbits.h"
#include "stubs_click.h"
#include "stubs_compositor_transitions.h"
#include "stubs_event_loop.h"
#include "stubs_event_service_client.h"
#include "stubs_events.h"
#include "stubs_i18n.h"
#include "stubs_layer.h"
#include "stubs_logging.h"
#include "stubs_memory_layout.h"
#include "stubs_modal_manager.h"
#include "stubs_mutex.h"
#include "stubs_notification_storage.h"
#include "stubs_notifications.h"
#include "stubs_passert.h"
#include "stubs_pbl_malloc.h"
#include "stubs_pebble_process_info.h"
#include "stubs_pebble_tasks.h"
#include "stubs_phone_call_util.h"
#include "stubs_process_manager.h"
#include "stubs_prompt.h"
#include "stubs_property_animation.h"
#include "stubs_regular_timer.h"
#include "stubs_reminder_db.h"
#include "stubs_serial.h"
#include "stubs_session.h"
#include "stubs_shell_prefs.h"
#include "stubs_sleep.h"
#include "stubs_syscalls.h"
#include "stubs_system_theme.h"
#include "stubs_task_watchdog.h"
#include "stubs_timeline.h"
#include "stubs_timeline_actions.h"
#include "stubs_timeline_layout.h"
#include "stubs_timeline_layout_animations.h"
#include "stubs_timeline_peek.h"
#include "stubs_timezone_database.h"
#include "stubs_wakeup.h"
#include "stubs_watchface.h"
#include "stubs_window_manager.h"
#include "stubs_window_stack.h"

// Helper Functions
/////////////////////

#include "fw/graphics/util.h"
