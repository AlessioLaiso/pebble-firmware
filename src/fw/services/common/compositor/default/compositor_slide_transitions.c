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

#include "compositor_slide_transitions.h"

#include "services/common/compositor/compositor_private.h"
#include "services/common/compositor/compositor_transitions.h"

#include "apps/system_apps/timeline/timeline_common.h"
#include "applib/graphics/bitblt.h"
#include "applib/graphics/framebuffer.h"
#include "applib/graphics/gtypes.h"
#include "applib/ui/animation_interpolate.h"
#include "popups/timeline/peek.h"

// TODO: PBL-31388 Factor out vertical compositor slide animations
// This does a similar transition to the legacy modal slide transition
// With a few tweaks, this compositor animation can drive both

typedef struct {
  int16_t offset_y;
  bool slide_up;
  bool timeline_is_destination;
  bool timeline_is_empty;
  GColor fill_color;
} CompositorSlideTransitionData;

static CompositorSlideTransitionData s_data;

static void prv_copy_framebuffer_rows(GBitmap *dest_bitmap, GBitmap *src_bitmap, int16_t start_row,
                                      int16_t end_row, int16_t dupe_row, int16_t shift_amount) {
  const int16_t delta = start_row > end_row ? -1 : 1;
  for (int16_t dest_row = start_row; dest_row != end_row; dest_row += delta) {
    src_bitmap->bounds = (GRect) {
      .origin.y = (dupe_row >= 0 ? dupe_row : dest_row) - shift_amount,
      .size = { DISP_COLS, 1 },
    };
    bitblt_bitmap_into_bitmap(dest_bitmap, src_bitmap, GPoint(0, dest_row), GCompOpAssign,
                              GColorWhite);
  }
}

static void prv_shift_framebuffer_rows(GBitmap *dest_bitmap, int16_t start_row, int16_t end_row,
                                       int16_t shift_amount) {
  GBitmap src_bitmap = *dest_bitmap;
  prv_copy_framebuffer_rows(dest_bitmap, &src_bitmap, start_row, end_row, -1, shift_amount);
}

static void prv_duplicate_framebuffer_row(GBitmap *dest_bitmap, int16_t start_row, int16_t end_row,
                                          GBitmap *src_bitmap, int16_t dupe_row) {
  prv_copy_framebuffer_rows(dest_bitmap, src_bitmap, start_row, end_row, dupe_row, 0);
}

static void prv_slide_transition_animation_update(GContext *ctx, Animation *animation,
                                                  uint32_t distance_normalized) {
  const AnimationProgress progress = distance_normalized;
  const int last_offset_y = s_data.offset_y;
  const int delta_rows = s_data.slide_up ? -DISP_ROWS : DISP_ROWS;
  s_data.offset_y = interpolate_int16(progress, 0, delta_rows);
  // Advance one line to give movement from the very start only for the full curve
  const int advance = s_data.timeline_is_destination ? 0 : 1;

  // Whether shifting should occur. offset_y starts at 0, and |delta_rows| is DISP_ROWS
  const bool should_shift =
      s_data.slide_up ? (s_data.offset_y > delta_rows) : (s_data.offset_y < delta_rows);
  // Horizontal framebuffer lines are shifted vertically, these indicate the start and end of the
  // entire region to be shifted
  int shift_start_row;
  int shift_end_row;
  // Lines that shift with no replacement framebuffer line are filled if we are entering timeline
  int fill_offset_y;
  int fill_height;
  // Otherwise they are overdrawn with lines from the app framebuffer
  int app_offset_y;
  // When the app framebuffer overshoots, the line nearest the overshoot needs to be duplicated
  bool app_should_dupe;
  int app_dupe_row;

  if (s_data.slide_up) {
    s_data.offset_y -= advance;
    shift_start_row = 0;
    shift_end_row = DISP_ROWS + s_data.offset_y;
    fill_offset_y = shift_end_row;
    fill_height = DISP_ROWS - shift_end_row;
    app_offset_y = shift_end_row;
    app_should_dupe = (app_offset_y < 0);
    app_dupe_row = DISP_ROWS - 1;
  } else {
    s_data.offset_y += advance;
    shift_start_row = DISP_ROWS;
    shift_end_row = s_data.offset_y;
    fill_offset_y = 0;
    fill_height = shift_end_row;
    app_offset_y = shift_end_row - DISP_ROWS + 1;
    app_should_dupe = (app_offset_y > 0);
    app_dupe_row = 0;
  }

  GBitmap dest_bitmap = compositor_get_framebuffer_as_bitmap();
  if (should_shift) {
    const int shift_amount = s_data.offset_y - last_offset_y;
    prv_shift_framebuffer_rows(&dest_bitmap, shift_start_row, shift_end_row, shift_amount);
  }
  if (s_data.timeline_is_destination) {
#if CAPABILITY_HAS_TIMELINE_PEEK
    if (!s_data.timeline_is_empty) {
      graphics_context_set_fill_color(ctx, GColorWhite);
      const int content_width = DISP_COLS - TIMELINE_PEEK_ICON_BOX_WIDTH;
      graphics_fill_rect(ctx, &GRect(0, fill_offset_y, content_width, fill_height));
      graphics_context_set_fill_color(ctx, s_data.fill_color);
      graphics_fill_rect(ctx, &GRect(content_width, fill_offset_y, TIMELINE_PEEK_ICON_BOX_WIDTH,
                                    fill_height));
    } else
#endif
    {
      graphics_context_set_fill_color(ctx, s_data.fill_color);
      graphics_fill_rect(ctx, &GRect(0, fill_offset_y, DISP_COLS, fill_height));
    }
  } else {
    GBitmap app_bitmap = compositor_get_app_framebuffer_as_bitmap();
    bitblt_bitmap_into_bitmap(&dest_bitmap, &app_bitmap, GPoint(0, app_offset_y),
                              GCompOpAssign, GColorWhite);
    if (app_should_dupe) {
      prv_duplicate_framebuffer_row(&dest_bitmap, app_dupe_row, app_offset_y + app_dupe_row,
                                    &app_bitmap, app_dupe_row);
    }
  }
  framebuffer_dirty_all(compositor_get_framebuffer());

  // Update modal position for transparent modals
  compositor_set_modal_transition_offset(GPoint(0, app_offset_y));
}

static void prv_slide_transition_animation_init(Animation *animation) {
  // Give a regular moook more time to stretch the anticipation
  const uint32_t duration = s_data.timeline_is_destination ? interpolate_moook_in_duration() :
                                                             interpolate_moook_duration();
  const InterpolateInt64Function interpolation =
      s_data.timeline_is_destination ? interpolate_moook_in_only : interpolate_moook;
  animation_set_duration(animation, duration);
  animation_set_custom_interpolation(animation, interpolation);
}

const CompositorTransition *prv_slide_transition_get(void) {
  static const CompositorTransition s_impl = {
    .init = prv_slide_transition_animation_init,
    .update = prv_slide_transition_animation_update,
  };
  return &s_impl;
}

const CompositorTransition *compositor_slide_transition_timeline_get(
    bool timeline_is_future, bool timeline_is_destination, bool timeline_is_empty) {
  s_data = (CompositorSlideTransitionData) {
    .slide_up = timeline_is_future ^ !timeline_is_destination,
    .fill_color = timeline_is_future ? TIMELINE_FUTURE_COLOR : TIMELINE_PAST_COLOR,
    .timeline_is_destination = timeline_is_destination,
    .timeline_is_empty = timeline_is_empty,
  };
  return prv_slide_transition_get();
}
