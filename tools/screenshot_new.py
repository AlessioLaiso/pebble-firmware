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

import png
import pulse
from bitarray import bitarray


def chunks(l, n):
    for i in xrange(0, len(l), n):
        yield l[i:i+n]


def convert_8bpp(data):
    ret = []
    for pixel in data:
        pixel = ord(pixel)
        red, green, blue = (((pixel >> n) & 0b11) * 255 / 3 for n in (4, 2, 0))
        ret.extend((red, green, blue))
    return ret


def convert_1bpp(data, width):
    # Chop off the unused bytes at the end of each row
    bytes_per_row = (width / 32 + 1) * 4
    data = ''.join(c[:width/8] for c in chunks(data, bytes_per_row))
    ba = bitarray(endian='little')
    ba.frombytes(data)
    return ba.tolist()


def framebuffer_to_png(data, png_path, width, bpp):
    if bpp == 1:
        data = convert_1bpp(data, width)
        channels = 'L'
    elif bpp == 8:
        data = convert_8bpp(data)
        channels = 'RGB'

    data = list(chunks(data, width*len(channels)))

    png.from_array(data, mode='%s;%d' % (channels, bpp)).save(png_path)


def cmd_screenshot(args):
    with pulse.Connection.open_dbgserial(args.tty) as connection:
        connection.change_baud_rate(921600)
        screenshot(connection, args.filename)


def screenshot(connection, filename):
    """ Take a screenshot over the serial console and save it to file. """

    data = str(connection.read.read_framebuffer())
    stats = connection.read.stat_framebuffer()

    framebuffer_to_png(data, filename, stats.width, stats.bpp)


def main():
    import argparse

    parser = argparse.ArgumentParser(description='Tool to take a Pebble screenshot over serial')
    parser.add_argument('tty', metavar='TTY', help='the target serial port')
    parser.add_argument('filename', help='the filename to save screenshot to')

    args = parser.parse_args()
    cmd_screenshot(args)

if __name__ == '__main__':
    main()
