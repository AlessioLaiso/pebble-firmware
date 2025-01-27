# Copyright 2024 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

try:
    import Queue
except ImportError:
    # Py3 support
    import queue as Queue

HDLC_FRAME_START = 0x7e
HDLC_ESCAPE = 0x7d
HDLC_ESCAPE_MASK = 0x20


class HDLCDecoder(object):
    _STATE_SYNC, _STATE_DATA, _STATE_ESCAPE = range(3)

    def __init__(self):
        self._frames = Queue.Queue()
        self._state = self._STATE_SYNC
        self._buffer = bytearray()

    def write(self, data):
        for b in bytearray(data):
            if self._state == self._STATE_SYNC:
                # waiting for the first FRAME_START byte
                if b == HDLC_FRAME_START:
                    self._state = self._STATE_DATA
            elif self._state == self._STATE_DATA:
                if b == HDLC_FRAME_START:
                    # this is the end of the frame (and the start of the next one)
                    if self._buffer:
                        self._frames.put_nowait(bytes(self._buffer))
                    self._buffer = bytearray()
                elif b == HDLC_ESCAPE:
                    # escape the next byte
                    self._state = self._STATE_ESCAPE
                else:
                    # this a valid byte of data
                    self._buffer.append(b)
            elif self._state == self._STATE_ESCAPE:
                if b == HDLC_FRAME_START:
                    # invalid byte combination - drop this frame and start the next one
                    self._buffer = bytearray()
                else:
                    # escape this character
                    self._buffer.append(b ^ HDLC_ESCAPE_MASK)
                self._state = self._STATE_DATA
            else:
                assert False, 'Invalid state!'

    def get_frame(self):
        try:
            return self._frames.get_nowait()
        except Queue.Empty:
            return None


def hdlc_encode_data(data):
    frame = bytearray()
    frame.append(HDLC_FRAME_START)
    for b in bytearray(data):
        if b == HDLC_FRAME_START or b == HDLC_ESCAPE:
            b ^= HDLC_ESCAPE_MASK
            frame.append(HDLC_ESCAPE)
        frame.append(b)
    frame.append(HDLC_FRAME_START)
    return bytes(frame)
