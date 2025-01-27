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

import functools
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), '../'))
import parse_c_decl
from parse_c_decl import clang

def extract_exported_functions(node, functions=[], types=[], defines=[]):
    def update_matching_export(exports, node):
        spelling = parse_c_decl.get_node_spelling(node)
        for e in exports:
            impl_name = e.impl_name if hasattr(e, 'impl_name') else ""

            definition_name = impl_name if impl_name else e.name
            if spelling == definition_name:
                # Found a matching node! Before we update our export make sure this attribute is larger
                # than the one we may already have. This is to handle the case where we have typedef and
                # a struct as part of the same definition, we want to make sure we get the outer typedef.
                definition = parse_c_decl.get_string_from_file(node.extent)
                if e.full_definition is None or len(definition) > len(e.full_definition):
                    if node.kind == clang.cindex.CursorKind.MACRO_DEFINITION:
                        e.full_definition = "#define " + definition
                    else:
                        e.full_definition = definition

            # Update the exports with comments / definition info from both the
            # 'implName' and 'name'. Keep whatever is longer and does not start
            # with @internal (meaning the whole docstring is internal).
            if spelling == e.name or (impl_name and spelling == impl_name):
                comment = parse_c_decl.get_comment_string_for_decl(node)
                if comment is not None and not comment.startswith("//! @internal"):
                    if e.comment is None or len(comment) > len(e.comment):
                        e.comment = comment

        return None

    if node.kind == clang.cindex.CursorKind.FUNCTION_DECL:
        update_matching_export(functions, node)

    elif node.kind == clang.cindex.CursorKind.STRUCT_DECL or \
         node.kind == clang.cindex.CursorKind.ENUM_DECL or \
         node.kind == clang.cindex.CursorKind.TYPEDEF_DECL:

        update_matching_export(types, node)

    elif node.kind == clang.cindex.CursorKind.MACRO_DEFINITION:

        update_matching_export(defines, node)


def extract_symbol_info(filenames, functions, types, defines, output_dir, internal_sdk_build=False,
                        compiler_flags=None):

    # Parse all the headers at the same time since that is much faster than
    # parsing each one individually
    all_headers_file = os.path.join(output_dir, "all_sdk_headers.h")
    with open(all_headers_file, 'w') as outfile:
        for f in filenames:
            outfile.write('#include "%s"\n' % f)

    parse_c_decl.parse_file(all_headers_file, filenames,
                            functools.partial(extract_exported_functions,
                                              functions=functions,
                                              types=types,
                                              defines=defines),
                            internal_sdk_build=internal_sdk_build,
                            compiler_flags=compiler_flags)

if __name__ == '__main__':
    parse_c_decl.dump_tree = True

    class Export(object):
        def __init__(self, name):
            self.name = name

            self.full_definition = None
            self.comment = None

    #clang.cindex.Config.library_file = "/home/brad/src/llvmbuild/Debug+Asserts/lib/libclang.so"

    extract_symbol_info((sys.argv[1],), [], [], [])

