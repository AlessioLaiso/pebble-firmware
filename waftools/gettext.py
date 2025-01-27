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

from waflib import Configure, TaskGen, Task

GETTEXT_KEYWORDS = ['i18n_noop',
                    'i18n_get', 'i18n_get_with_buffer',
                    'sys_i18n_get_with_buffer',
                    'i18n_ctx_noop:1c,2',
                    'i18n_ctx_get:1c,2', 'i18n_ctx_get_with_buffer:1c,2']

def configure(conf):
    conf.find_program('xgettext', exts="", errmsg="""
=======================================================================
`gettext` might not be installed properly.
 - If using a Mac, try running `brew install gettext; brew link gettext --force`
 - If using Linux, and you fix this error, please insert solution here
=======================================================================""")
    conf.find_program('msgcat')

class xgettext(Task.Task):
    run_str = ('${XGETTEXT} -c/ -k --from-code=UTF-8 --language=C ' +
               ' '.join('--keyword=' + word for word in GETTEXT_KEYWORDS) +
               ' -o ${TGT[0].abspath()} ${SRC}')

class msgcat(Task.Task):
    run_str = '${MSGCAT} ${SRC} -o ${TGT}'

@TaskGen.before('process_source')
@TaskGen.feature('gettext')
def do_gettext(self):
    sources = [src for src in self.to_nodes(self.source)
               if src.suffix() not in ('.s', '.S')]

    # There is a convenient to_nodes method for sources (that already exist),
    # but no equivalent for targets (files which don't exist yet).
    if isinstance(self.target, str):
        target = self.path.find_or_declare(self.target)
    else:
        target = self.target
    self.create_task('xgettext', src=sources, tgt=target)
    # Bypass the execution of process_source
    self.source = []

@TaskGen.before('process_source')
@TaskGen.feature('msgcat')
def do_msgcat(self):
    if isinstance(self.target, str):
        target = self.path.find_or_declare(self.target)
    else:
        target = self.target
    self.create_task('msgcat', src=self.to_nodes(self.source), tgt=target)
    # Bypass the execution of process_source
    self.source = []

@Configure.conf
def gettext(self, *args, **kwargs):
    kwargs['features'] = 'gettext'
    return self(*args, **kwargs)

@Configure.conf
def msgcat(self, *args, **kwargs):
    kwargs['features'] = 'msgcat'
    return self(*args, **kwargs)
