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

import uuid

try:
    import gdb
except ImportError:
    raise Exception("This file is a GDB script.\n"
                    "It is not intended to be run outside of GDB.\n"
                    "Hint: to load a script in GDB, use `source this_file.py`")
import gdb.printing


class grectPrinter:
    """Print a GRect struct as a fragment of C code."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        code = ("(GRect) { "
                ".origin = { .x = %i, .y = %i }, "
                ".size = { .w = %i, .h = %i } "
                "}")
        return code % (int(self.val["origin"]["x"]),
                       int(self.val["origin"]["y"]),
                       int(self.val["size"]["w"]),
                       int(self.val["size"]["h"]))


class gpathInfoPrinter:
    """Print a GPathInfo struct as a fragment of C code."""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        points_code = ""
        num_points = int(self.val["num_points"])
        array_val = self.val["points"]
        for i in xrange(0, num_points):
            point_val = array_val[i]
            if (points_code):
                points_code += ", "
            points_code += "{ %i, %i }" % (point_val["x"], point_val["y"])
        outer_code_fmt = ("(GPathInfo) { "
                          ".num_points = %i, "
                          ".points = (GPoint[]) {%s} "
                          "}")
        return outer_code_fmt % (num_points, points_code)


class UuidPrinter(object):
    """Print a UUID."""

    def __init__(self, val):
        bytes = ''.join(chr(int(val['byte%d' % n])) for n in xrange(16))
        self.uuid = uuid.UUID(bytes=bytes)

    def to_string(self):
        return '{%s}' % self.uuid

pp = gdb.printing.RegexpCollectionPrettyPrinter('tintin')
pp.add_printer('GRect', '^GRect$', grectPrinter)
pp.add_printer('GPathInfo', '^GPathInfo$', gpathInfoPrinter)
pp.add_printer('Uuid', '^Uuid$', UuidPrinter)
# Register the pretty-printer globally
gdb.printing.register_pretty_printer(None, pp, replace=True)
