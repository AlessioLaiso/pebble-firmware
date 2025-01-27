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

#include "compositor_port_hole_transitions.h"

#include "applib/graphics/bitblt.h"
#include "applib/graphics/framebuffer.h"
#include "applib/graphics/graphics_private.h"
#include "util/trig.h"
#include "services/common/compositor/compositor_transitions.h"

#include "resource/resource_ids.auto.h"
#include "system/logging.h"

typedef struct {
  CompositorTransitionDirection direction;
  int16_t animation_offset_px;
} CompositorPortHoleTransitionData;

static CompositorPortHoleTransitionData s_data;

void compositor_port_hole_transition_draw_outer_ring(GContext *ctx, int16_t thickness,
                                                     GColor ring_color) {
  const uint16_t overdraw = 2;
  graphics_context_set_fill_color(ctx, ring_color);
  graphics_fill_radial(ctx, grect_inset(DISP_FRAME, GEdgeInsets(-overdraw)),
                       GOvalScaleModeFitCircle, thickness + overdraw, 0, TRIG_MAX_ANGLE);
}

// piecewise interpolator between 0 and to for the first half of ANIMATION_NORMALIZED_MAX
// and between -to and 0 for the second half
static int16_t prv_interpolate_two_ways_int16(AnimationProgress normalized_progress,
                                              int32_t discontinuity_progress, int16_t to) {
  if (normalized_progress < discontinuity_progress) {
    return interpolate_int16(animation_timing_scaled(normalized_progress, 0,
                                                     discontinuity_progress), 0, to);
  } else {
    return interpolate_int16(animation_timing_scaled(normalized_progress, discontinuity_progress,
                                                     ANIMATION_NORMALIZED_MAX), -to, 0);
  }
}

static void prv_port_hole_transition_animation_init(Animation *animation) {
  animation_set_duration(animation, PORT_HOLE_TRANSITION_DURATION_MS);
  s_data.animation_offset_px = 0;
}

static void prv_port_hole_transition_animation_update(GContext *ctx,
                                                      Animation *animation,
                                                      uint32_t distance_normalized) {
  const uint32_t transition_progress_threshold = ANIMATION_NORMALIZED_MAX / 2;
  const int32_t ring_max_thickness = 40;
  const bool direction_negative = ((s_data.direction == CompositorTransitionDirectionRight) ||
                                   (s_data.direction == CompositorTransitionDirectionDown));
  const bool direction_vertical = ((s_data.direction == CompositorTransitionDirectionDown) ||
                                   (s_data.direction == CompositorTransitionDirectionUp));

  const int16_t current_offset_px =
      prv_interpolate_two_ways_int16(distance_normalized, transition_progress_threshold,
                                     direction_negative ? ring_max_thickness :
                                                          -ring_max_thickness);

  if (distance_normalized > transition_progress_threshold) {
    // Second half of the transition
    const GBitmap app_bitmap = compositor_get_app_framebuffer_as_bitmap();
    GBitmap sys_bitmap = compositor_get_framebuffer_as_bitmap();
    const GPoint point = direction_vertical ? GPoint(0, -current_offset_px) :
                                              GPoint(-current_offset_px, 0);
    // the framebuffer is already wiped at the beginning, so we can use GColorWhite as a fill color
    // without filling it ourselves
    bitblt_bitmap_into_bitmap(&sys_bitmap, &app_bitmap, point, GCompOpAssign, GColorWhite);
  } else {
    // First half of the transition
    const int16_t diff = s_data.animation_offset_px - current_offset_px;
    if (direction_vertical) {
      graphics_private_move_pixels_vertically(&ctx->dest_bitmap, diff);
    } else {
      graphics_private_move_pixels_horizontally(&ctx->dest_bitmap, diff,
                                                false /* patch_garbage */);
    }
  }

  compositor_port_hole_transition_draw_outer_ring(ctx, ABS(current_offset_px), GColorBlack);
  s_data.animation_offset_px = current_offset_px;
}

const CompositorTransition *compositor_port_hole_transition_app_get(
    CompositorTransitionDirection direction) {
  if (compositor_transition_app_to_app_should_be_skipped()) {
    return NULL;
  }

  s_data.direction = direction;

  static const CompositorTransition s_impl = {
    .init = prv_port_hole_transition_animation_init,
    .update = prv_port_hole_transition_animation_update,
  };

  return &s_impl;
}
