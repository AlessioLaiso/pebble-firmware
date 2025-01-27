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

import os.path
import subprocess
import tempfile

SHIM_PREAMBLE = """
.syntax unified
.thumb

"""

# We generate one of these trampolines for each function we export. This is a stub function
# that pretty much just sets us up to call 'jump_to_pbl_function', defined in the SHIM_FOOTER
# string.
SHIM_TEMPLATE = """
.section ".text.%(function_name)s"
.align 2
.thumb_func
.global %(function_name)s
.type %(function_name)s STT_FUNC
%(function_name)s:
        push {r0, r1, r2, r3}  @ Stack the original parameters. Remember that the caller
                               @ thinks they're calling a C function, not this shim function.
                               @ We don't want to touch these.
        ldr r1, =%(offset)d    @ Load up the index into the jump table array
        b jump_to_pbl_function @ See below...
 """

# This section defines the second part of the trampoline function. We only jump to it from
# the functions we define in SHIM_TEMPLATE. The SHIM_TEMPLATE function jumps to us after
# setting up the offset in the jump table in r1.
SHIM_FOOTER = """
.section ".text"
.type jump_to_pbl_function function
jump_to_pbl_function:
        adr r3, pbl_table_addr
        ldr r0, [r3]           @ r0 now holds the base address of the jump table
        add r0, r0, r1         @ add the offset to address the function pointer to the exported function
        ldr r2, [r0]           @ r2 now holds the address of the exported function
        mov r12, r2            @ r12 aka intra-procedure scratch register. We're allowed to
                               @ muck with this and not restore it
        pop {r0, r1, r2, r3}   @ Restore the original parameters to the exported C function
        bx r12                 @ And finally jump! Don't use branch and link, we want to
                               @ return to the original call site, not this shim

@ This pbl_table_addr variable will get compiled into our app binary. As part of our build process
@ the address of this variable will get poked into the PebbleProcessInfo meta data struct. Then, when
@ we load the app from flash we change the value of the variable to point the jump table exported
@ by the OS.
.align
pbl_table_addr:
.long 0xA8A8A8A8

"""

def gen_shim_asm(functions):
    output = []

    output.append(SHIM_PREAMBLE)
    for idx, fun in enumerate(functions):
        if not fun.removed:
            output.append(SHIM_TEMPLATE % {'function_name': fun.name, 'offset': idx * 4})
    output.append(SHIM_FOOTER)

    return '\n'.join(output)

class CompilationFailedError(Exception):
    pass

def build_shim(shim_s, dest_dir):
    shim_a = os.path.join(dest_dir, 'libpebble.a')
    # Delete any existing archive, otherwise `ar` will append/insert to it:
    if os.path.exists(shim_a):
        os.remove(shim_a)
    shim_o = tempfile.NamedTemporaryFile(suffix='pebble.o').name
    gcc_process = subprocess.Popen(['arm-none-eabi-gcc',
                                    '-mcpu=cortex-m3',
                                    '-mthumb',
                                    '-fPIC',
                                    '-c',
                                    '-o',
                                    shim_o,
                                    shim_s],
                                   stdout = subprocess.PIPE,
                                   stderr = subprocess.PIPE)
    output = gcc_process.communicate()
    if (gcc_process.returncode != 0):
        print(output[1])
        raise CompilationFailedError()

    ar_process = subprocess.Popen(['arm-none-eabi-ar',
                                   'rcs',
                                   shim_a,
                                   shim_o],
                                  stdout = subprocess.PIPE,
                                  stderr = subprocess.PIPE)
    output = ar_process.communicate()
    if (ar_process.returncode != 0):
        print(output[1])
        raise CompilationFailedError()

def make_app_shim_lib(functions, sdk_lib_dir):
    temp_asm_file = tempfile.NamedTemporaryFile(suffix='pbl_shim.s').name
    with open(temp_asm_file, 'w') as shim_s:
        shim_s.write(gen_shim_asm(functions))

    build_shim(temp_asm_file, sdk_lib_dir)

