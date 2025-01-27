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

import freetype
import argparse
import os, sys, re
from math import ceil

MIN_CODEPOINT = 0x20
MAX_CODEPOINT = 0xffff
# Set a codepoint that the font doesn't know how to render
# The watch will use this glyph as the wildcard character
WILDCARD_CODEPOINT = 0x3456

class Font:
    def __init__(self, ttf_path):
        self.version = 1
        self.ttf_path = ttf_path
        # Get the font's size from the filename:
        self.basename = os.path.basename(self.ttf_path)
        m = re.search('([0-9]+)', self.basename)
        if m == None:
            sys.stderr.write('Font {0}: no size found in file name...\n'.format(filename))
            return
        self.max_height = int(m.group(0))
        self.face = freetype.Face(self.ttf_path)
        self.face.set_pixel_sizes(0, self.max_height)
        self.name = self.face.family_name + "_" + self.face.style_name
        self.wildcard_codepoint = WILDCARD_CODEPOINT
        self.number_of_glyphs = 0
        return

    def is_supported_glyph(self, codepoint):
        return (self.face.get_char_index(codepoint) > 0 or (codepoint == unichr(self.wildcard_codepoint)))

    def emit_codepoints(self):
        to_file = os.path.splitext(self.ttf_path)[0] + '.codepoints'
        f = open(to_file, 'wb')
        for codepoint in xrange(MIN_CODEPOINT, MAX_CODEPOINT + 1):
            self.face.load_char(unichr(codepoint))
            if self.is_supported_glyph(codepoint):
                print>>f,"U+%08d" % (codepoint,)
        f.close()
        return

    def emit_codepoints_as_utf8(self):
        to_file = os.path.splitext(self.ttf_path)[0] + '.utf8'
        f = open(to_file, 'wb')
        for codepoint in xrange(MIN_CODEPOINT, MAX_CODEPOINT + 1):
            self.face.load_char(unichr(codepoint))
            if self.is_supported_glyph(codepoint):
                f.write(unichr(codepoint).encode('utf-8'))
        f.close()
        return

def main():
    font_directory = "ttf"
    font_paths = []
    for _, _, filenames in os.walk(font_directory):
        for filename in filenames:
            if os.path.splitext(filename)[1] == '.ttf':
                font_paths.append(os.path.join(font_directory, filename))

    for font_path in font_paths:
        f = Font(font_path)
        f.emit_codepoints()
        f.emit_codepoints_as_utf8()
    return

if __name__ == "__main__":
    main()
