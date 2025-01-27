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

#include "window_stack_animation_round.h"

#include "animation_timing.h"
#include "window_private.h"
#include "window_stack.h"

#include "applib/applib_malloc.auto.h"
#include "applib/graphics/graphics.h"
#include "applib/graphics/graphics_private.h"
#include "applib/graphics/graphics_private_raw.h"
#include "applib/graphics/gtypes.h"
#include "board/display.h"
#include "kernel/ui/kernel_ui.h"
#include "services/common/compositor/compositor_transitions.h"
#include "system/passert.h"
#include "util/attributes.h"
#include "util/math.h"
#include "util/trig.h"

// Window transition implementations
//////////////////////////////////////

static uint16_t prv_window_distance_from_screen_bounds(const Window *window) {
  const GPoint origin = window->layer.frame.origin;
  return MAX(distance_to_mod_boundary(origin.x, DISP_COLS),
             distance_to_mod_boundary(origin.y, DISP_ROWS));
}

static void prv_window_transition_render(WindowTransitioningContext *context, GContext *ctx) {
  const WindowTransitionRoundImplementation *implementation =
    (WindowTransitionRoundImplementation *)context->implementation;
  Window *window_to = context->window_to;
  if (window_to) {
    // move framebuffer by amount of pixels the window_to moves
    // => gives impression of a moving window_from
    const int16_t new_x = window_to->layer.frame.origin.x;
    const int16_t delta_x = new_x - context->window_to_last_x;
    graphics_private_move_pixels_horizontally(&ctx->dest_bitmap, delta_x,
                                              false /* patch_garbage */);
    context->window_to_last_x = new_x;

    // render window_from
    window_render(window_to, ctx);

    // cover whole movement with a ring that distracts from the simple movement
    uint16_t gap_to_cover = prv_window_distance_from_screen_bounds(window_to);
    compositor_port_hole_transition_draw_outer_ring(ctx, gap_to_cover, GColorBlack);
  }
}

static void prv_window_transition_animation_setup(Animation *animation) {
  WindowTransitioningContext *context = animation_get_context(animation);
  window_transition_context_appearance_call_all(context);

  const AnimationImplementation *impl = animation_get_implementation(animation);
  if (impl && impl->update) {
    // make sure window_to is at its starting position for the transition
    impl->update(animation, 0);
  }
  if (context->window_to) {
    // store starting position of window_to to know which pixels to update
    context->window_to_last_x = context->window_to->layer.frame.origin.x;
  }
}

static GPoint prv_displacement_from(CompositorTransitionDirection direction) {
  switch (direction) {
    case CompositorTransitionDirectionUp:
      return GPoint(0, 1);
    case CompositorTransitionDirectionDown:
      return GPoint(0, -1);
    case CompositorTransitionDirectionLeft:
      return GPoint(1, 0);
    case CompositorTransitionDirectionRight:
      return GPoint(-1, 0);
    case CompositorTransitionDirectionNone:
    default:
      return GPointZero;
  }
}

static CompositorTransitionDirection prv_direction_from_context(
  const WindowTransitioningContext *context) {
  return ((WindowTransitionRoundImplementation *)context->implementation)->transition_direction;
}

static void prv_window_transition_animation_update(Animation *animation,
                                                   const AnimationProgress progress) {
  const WindowTransitioningContext *context = animation_get_context(animation);
  const CompositorTransitionDirection direction = prv_direction_from_context(context);
  Window *window_to = context->window_to;
  if (window_to) {
    const GPoint factor = prv_displacement_from(direction);

    // in the video for S4 with 180px I measured 80px
    // this expression tries express this in a future-proof manner in case we will have round
    // displays with a different resolution
    const int16_t offset_value = DISP_COLS * 80 / 180;
    const GPoint offset = GPoint(factor.x * offset_value, factor.y * offset_value);

    const AnimationProgress half_time = ANIMATION_NORMALIZED_MAX / 2;
    const bool first_half = progress < half_time;
    GPoint from;
    GPoint to;

    // does a movement of the first pixels, a cut, and then a movement of the last pixels
    if (first_half) {
      from = GPoint(factor.x * DISP_COLS, factor.y * DISP_ROWS);
      to = gpoint_sub(from, offset);
    } else {
      to = GPointZero;
      from = gpoint_add(to, offset);
    }
    window_to->layer.frame.origin = interpolate_gpoint(progress, from, to);
    window_schedule_render(window_to);
  }
}

static Animation *prv_window_transition_create_animation(WindowTransitioningContext *context) {
  static AnimationImplementation const impl = {
    .setup = prv_window_transition_animation_setup,
    .update = prv_window_transition_animation_update,
  };

  Animation *animation = animation_create();
  animation_set_implementation(animation, &impl);
  animation_set_handlers(animation, (AnimationHandlers){}, context);
  animation_set_curve(animation, AnimationCurveEaseInOut);
  animation_set_duration(animation, PORT_HOLE_TRANSITION_DURATION_MS);

  return animation;
}

const WindowTransitionRoundImplementation g_window_transition_default_push_implementation_round = {
  .implementation =  {
    .create_animation = prv_window_transition_create_animation,
    .render = prv_window_transition_render,
  },
  .transition_direction = CompositorTransitionDirectionLeft,
};

const WindowTransitionRoundImplementation g_window_transition_default_pop_implementation_round = {
  .implementation =  {
    .create_animation = prv_window_transition_create_animation,
    .render = prv_window_transition_render,
  },
  .transition_direction = CompositorTransitionDirectionRight,
};
