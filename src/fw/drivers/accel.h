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

#include "services/imu/units.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

/*! Accelerometer driver interface
 *  ==============================
 *
 * The accelerometer driver is simply an interface to the accelerometer
 * hardware. It is dumb; it does not contain any circular buffers, has no
 * knowledge of clients, threads, subsampling or even other hardware. It is up
 * to higher level code (read: the accelerometer service) to deal with
 * that. The reason for that is to maximize code reuse: anything which could
 * possibly need to be copy-pasted from one accel driver to another should be
 * moved outside of the driver.
 *
 * The accelerometer knows (almost) nothing about the OS, events, analytics or
 * the vibe motor. It does not even keep around a sample buffer for any reason.
 * All of that code is handled externally. The interface for the accelerometer
 * driver is a set of functions implemented by the accelerometer, and a set of
 * external functions that it will call in response to certain events. While OS
 * services may be employed internally by a driver, they are not part of the
 * public interface.
 *
 * One of the goals of the accelerometer interface is to hide the state of the
 * accelerometer hardware as much as possible (e.g. FIFO mode) and use
 * higher-level constructs to allow the driver to make its own decisions on what
 * state the hardware should be in. This way the interface is (hopefully)
 * generic enough that accelerometers with vastly different operating and
 * power-saving modes can have all of those details hidden away in the driver,
 * and the higher-level code can work unmodified with different accelerometers.
 */

//! Information which varies by accelerometer driver.
typedef struct {
  //! Highest sample interval (slowest rate) supported by the driver.
  uint32_t sample_interval_max;
  //! Recommended sample interval for low-power use, around 100,000 us.
  uint32_t sample_interval_low_power;
  //! Recommended sample interval for interactive use, around 250,000 us.
  uint32_t sample_interval_ui;
  //! Recommended sample interval for games and fast interactivity,
  //! around 20,000 us.
  uint32_t sample_interval_game;
  //! Lowest sample interval (fastest rate) supported by the driver.
  uint32_t sample_interval_min;
} AccelDriverInfo;

extern const AccelDriverInfo ACCEL_DRIVER_INFO;

typedef struct {
  //! Timestamp of when the sample was taken in microseconds since the epoch.
  //! The precision of the timestamp is not guaranteed.
  uint64_t timestamp_us;
  //! Acceleration along the x axis.
  int16_t x;
  //! Acceleration along the y axis.
  int16_t y;
  //! Acceleration along the z axis.
  int16_t z;
} AccelDriverSample;

//! Sets the accelerometer sampling interval.
//!
//! Not all sampling intervals are supported by all drivers. The driver must
//! select a sampling interval which is equal to or shorter than the requested
//! interval, saturating at the shortest interval supported by the hardware.
//!
//! The new sampling rate takes effect immediately. The driver may flush any
//! queued samples before changing the sampling rate to ensure that timestamps
//! remain accurate.
//!
//! @param interval_us The requested sampling interval in microseconds.
//!
//! @return The actual sampling interval set by the driver.
uint32_t accel_set_sampling_interval(uint32_t interval_us);

//! Returns the currently set accelerometer sampling interval.
uint32_t accel_get_sampling_interval(void);

//! Set the max number of samples the driver may batch.
//!
//! @param n Maximum number of samples the driver can batch
//!
//! When n=0, the accelerometer driver must not call accel_cb_new_sample().
//!
//! When n=1, the accelerometer driver must call accel_cb_new_sample() for
//! each sample as soon as the hardware has acquired it.
//!
//! When n>1, the accelerometer driver may batch up to n samples before
//! calling accel_cb_new_sample() up to n times in rapid succession with
//! all of the queued samples. The last item in a batch must be the most
//! recently acquired sample from the hardware. This is used by the driver
//! as a hint for power saving or other optimizations; it only sets an
//! upper bound on the number of samples the driver may batch up.
//!
//! When n is set to a value smaller than the number of samples already
//! queued up, the driver must flush all of the queued samples to
//! accel_cb_new_sample() before the new value of n takes effect. The
//! accel_cb_new_sample() function may be called from within
//! accel_set_num_samples().
//!
//! @see accel_cb_new_sample
void accel_set_num_samples(uint32_t num_samples);

//! Peek at the most recent accelerometer sample.
//!
//! @param[out] data Pointer to a buffer to write accelerometer data
//!
//! @return 0 if successful, nonzero on failure
//!
//! During the execution of this function, the driver may call
//! accel_cb_new_sample() one or more times iff accel_set_num_samples(n) was
//! called most recently with a value of n >= 1.
int accel_peek(AccelDriverSample *data);

//! Enable or disable shake detection
//!
//! @param on Enable shake detection when true, disable when false
//!
//! When shake detection is enabled, accel_cb_shake_detected must be called every
//! time the accelerometer hardware detects a shake. When shake detection is
//! disabled, accel_cb_shake_detected must not be called.
//!
//! @see accel_cb_shake_detected
void accel_enable_shake_detection(bool on);

//! Returns whether shake detection is enabled
bool accel_get_shake_detection_enabled(void);

//! Enable or disable double tap detection
//!
//! @param on Enable double tap detection when true, disable when false
//!
//! When double tap detection is enabled, accel_cb_double_tap_detected must be called every
//! time the accelerometer hardware detects a double tap. When double tap detection is
//! disabled, accel_cb_double_tap_detected must not be called.
//!
//! @see accel_cb_double_tap_detected
void accel_enable_double_tap_detection(bool on);

//! Returns whether double tap detection is enabled
bool accel_get_double_tap_detection_enabled(void);

//! Function called by the driver whenever a new accel sample is available.
//!
//! @param[in] data pointer to a populated AccelDriverSample struct. The pointer is
//!   only valid for the duration of the function call.
//!
//! This function will always be called with samples monotonically increasing in
//! time. It will always be called from within a thread context.
//!
//! @note This function may be called from within any of the functions in the
//!   accelerometer driver interface. To prevent reentrancy issues, avoid
//!   calling accelerometer driver functions from within this function.
extern void accel_cb_new_sample(AccelDriverSample const *data);

//! Function called by driver whenever shake is detected.
//!
//! @param axis      Axis which the shake was detected on
//! @param direction The sign indicates whether the shake was on the positive or
//!        negative axis
//!
//! @note It is up to the implementer to filter out shake events triggered by the
//!       vibrate motor.
extern void accel_cb_shake_detected(IMUCoordinateAxis axis, int32_t direction);

//! Function called by driver whenever a double tap is detected.
//!
//! @param axis      Axis which the double tap was detected on
//! @param direction The sign indicates whether the double tap was on the positive or
//!        negative axis
extern void accel_cb_double_tap_detected(IMUCoordinateAxis axis, int32_t direction);

//! Function called by driver when it needs to offload work from an ISR context.
//! It is up to the implementer to decide how this should work
//!
//! @param cb The callback to be invoked from a thread context
//! @param data Data to be passed to the callback, NULL if none
typedef void (*AccelOffloadCallback)(void);
extern void accel_offload_work_from_isr(AccelOffloadCallback cb, bool *should_context_switch);

//! Function runs a diagnostic test on the accelerometer hardware to confirm it
//! works as expected
extern bool accel_run_selftest(void);

//! The accelerometer supports a changeable sensitivity for shake detection. This call will
//! select whether we want the accelerometer to enter a highly sensitive state with a low
//! threshold, where any minor amount of motion would trigger the system shake event.
//! Note: Setting this value does not ensure that shake detection is enabled.
void accel_set_shake_sensitivity_high(bool sensitivity_high);
