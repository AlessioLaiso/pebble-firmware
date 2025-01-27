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

#include "music.h"

#include <stdbool.h>

//! OS interface to initialize the music service
void music_init(void);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface to music metadata server (Pebble Protocol, AMS, ...)

//! Bits in the bitset of optional features of a server
typedef enum {
  MusicServerCapabilityNone = 0,
  MusicServerCapabilityPlaybackStateReporting = (1 << 0),
  MusicServerCapabilityProgressReporting = (1 << 1),
  MusicServerCapabilityVolumeReporting = (1 << 2),
} MusicServerCapability;

//! Pointers to server-specific implementations of the music backend server
typedef struct {
  const char *debug_name;
  bool (*is_command_supported)(MusicCommand command);
  void (*command_send)(MusicCommand command);
  bool (*needs_user_to_start_playback_on_phone)(void);
  MusicServerCapability (*get_capability_bitset)(void);
  void (*request_reduced_latency)(bool reduced_latency);
  void (*request_low_latency_for_period)(uint32_t period_ms);
} MusicServerImplementation;

//! Informs the music service when the server got (dis)connected.
//! We're assuming only one instance of each type of server can exist, so there is no need for a
//! "context" argument here. Just the implementation pointer is enough.
//! @return True if the server was successfully (dis)connected, false if not. The server *MUST NOT*
//! call into any music_update_... if the connection was not successful!
bool music_set_connected_server(const MusicServerImplementation *implementation, bool connected);

//! Update the track that's currently playing. The strings don't need to be null terminated.
void music_update_now_playing(const char *title, size_t title_length,
                              const char *artist, size_t artist_length,
                              const char *album, size_t album_length);

//! Update the name of the player that's currently playing.
//! The string doesn't need to be null terminated.
void music_update_player_name(const char *player_name, size_t player_name_length);

//! Data structure to update playback state info.
//! @see music_update_player_playback_state
typedef struct {
  MusicPlayState playback_state;
  int32_t playback_rate_percent;
  uint32_t elapsed_time_ms;
} MusicPlayerStateUpdate;

//! Updates playstate, playback rate and elapsed time in one go.
void music_update_player_playback_state(const MusicPlayerStateUpdate *state);

//! Update the volume of the current player.
//! @param volume_percent The volume. Valid range: [0, 100]
void music_update_player_volume_percent(uint8_t volume_percent);

//! Update the title of the track that's currently playing.
//! The string doesn't need to be null terminated.
void music_update_track_title(const char *title, size_t title_length);

//! Update the artist of the track that's currently playing.
//! The string doesn't need to be null terminated.
void music_update_track_artist(const char *artist, size_t artist_length);

//! Update the artist of the track that's currently playing.
//! The string doesn't need to be null terminated.
void music_update_track_album(const char *album, size_t album_length);

//! Update the position of the current track.
void music_update_track_position(uint32_t track_pos_ms);

//! Update the duration of the current track.
void music_update_track_duration(uint32_t track_duration_ms);
