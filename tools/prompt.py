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

from serial_port_wrapper import SerialPortWrapper
import sys
import stm32_crc
import time

def go_to_prompt(s):
    s.clear()
    s.write_fast('\x03') # CTRL-C
    read_line = s.read(0.05)
    if not read_line.endswith('>'):
        time.sleep(0.6) # give the microcontroller time to wake from stop mode
        s.write_fast('\x03') # CTRL-C
        read_line = s.read()
        if not read_line.endswith('>'):
            print "ctrl-c response 1: " + read_line
            sys.stderr.write("Couldn't get to a prompt for some reason\n");
            sys.exit(2)

def finish_session(s, reset=False, verbose=False):
    try:
        go_to_prompt(s)
        if reset:
            if verbose:
                print "Resetting..."
            s.write('reset\x0d')
            while s.readline().find('Launcher') < 0:
                if verbose:
                    sys.stdout.write(".")
                    sys.stdout.flush()
    finally:
        s.close()
    if verbose:
        print 'done'
    return True

def show_log(s):
    s.write("\x04")

def boot_bit_set(s, bit, value):
    if value != 0 and value != 1:
        raise Exception("Value can only be 1 or 0.")
    if bit > 31 or bit < 0:
        raise Exception("Bit needs to be between 0 and 31.")
    go_to_prompt(s)
    s.write("boot bit set %u %u\x0d" % (bit, value))
    response = s.readline(1) # echo line
    response = s.readline(1)
    words = response.split(' ')
    if words[0] != 'OK':
        raise Exception("non-OK response for set bits: {}".format(response))

def resource_bank_info(s, bank_num, verbose=False):
    if verbose:
        print "Getting bank locations..."
    go_to_prompt(s)
    s.write('resource bank info %d\x0d' % bank_num)
    response = s.readline(1) # echo line
    response = s.readline(1)
    words = response.split(' ')
    if words[0] != 'OK':
        raise Exception("non-OK response for bank info: {}".format(response))
    start = int(words[1])
    length = int(words[2])
    return (start, length)

def issue_command(s, cmd):
    """ Write a command to the prompt of the SerialPortWrapper

        cmd: Command to execute with parameters, should not include a trailing newline
    """

    s.clear()
    s.write(cmd + '\r\n')
    s.readline() # Consume the echo

def issue_command_and_tail(s, cmd):
    issue_command(s, '!' + cmd)

    try:
        while True:
            line = s.readline().strip()
            if len(line):
                print line
    except KeyboardInterrupt:
        pass

