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

#################################################################################################
# Generate sample health activity blob
##################################################################################################
import argparse
import os
import sys
import logging
import math
import time
import struct


"""
typedef struct {
     uint8_t steps;             // # of steps in this minute
     uint8_t orientation;       // average orientation of the watch
     uint8_t vmc;               // vector magnitude count
} MinuteData;


typedef struct {
     uint16_t version;          // version, initial version is 1
     uint16_t len;              // length in bytes of blob, including this entire header
     uint32_t time_utc;         // UTC time of pebble
     uint32_t time_local;       // local time of pebble
     uint16_t num_samples;      // number of samples that follow
     MinuteData samples[];
} Header

"""

###################################################################################################
if __name__ == '__main__':

    # Collect our command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('--debug', action='store_true', help="Turn on debug logging")
    args = parser.parse_args()

    level = logging.INFO
    if args.debug:
        level = logging.DEBUG
    logging.basicConfig(level=level)

    sample_format = '<BBB'
    header_format = '<HHIIH'

    num_samples = 10
    blob = struct.pack(
        header_format,
        1,
        struct.calcsize(header_format) + num_samples * struct.calcsize(sample_format),
        int(time.time()),
        int(time.time()),
        num_samples)

    for i in range(num_samples):
        blob += struct.pack(sample_format,
                            30 + (i % 5),
                            4,
                            50 + (i % 4))

    with open('health_blob.bin', "w") as out:
        out.write(blob)
