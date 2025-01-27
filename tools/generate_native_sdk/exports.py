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

import json
import logging
import os

INTERNAL_REVISION = 999


class Export(object):
    def __init__(self, v, app_only, worker_only, deprecated):
        self.name = v['name']
        self.type = v['type']

        self.comment = None
        self.app_only = v.get('appOnly', app_only)
        self.worker_only = v.get('workerOnly', worker_only)
        self.include_after = v.get('includeAfter', [])
        self.deprecated = deprecated or v.get('deprecated', False)

    def complete(self):
        return True


class FullExport(Export):
    def __init__(self, v, app_only, worker_only, deprecated):
        super(FullExport, self).__init__(v, app_only, worker_only, deprecated)

        self.full_definition = None

    def complete(self):
        return self.full_definition is not None


class FunctionExport(FullExport):
    def __init__(self, v, app_only, worker_only, deprecated):
        super(FunctionExport, self).__init__(v, app_only, worker_only, deprecated)

        self.removed = False
        self.skip_definition = v.get('skipDefinition', False)
        self.added_revision = int(v['addedRevision'])
        self.sort_name = v.get('sortName', self.name)

        if v.get('removed', False):
            self.removed = True
        else:
            self.impl_name = v.get('implName', self.name)

    def complete(self):
        if self.removed or self.skip_definition:
            return True

        return super(FunctionExport, self).complete()


class Group(object):
    def __init__(self, export, parent_group, app_only, worker_only, current_revision, deprecated):
        self.name = export['name']
        self.parent_group = parent_group
        self.app_only = export.get('appOnly', app_only)
        self.worker_only = export.get('workerOnly', worker_only)
        self.deprecated = export.get('deprecated', False) or deprecated
        self.exports = parse_exports_list(export['exports'], current_revision, self, self.app_only,
                                          self.worker_only, self.deprecated)
        self.comment = None
        self.display_name = None

    def group_stack(self):
        stack = [self.name,]
        parent_iter = self.parent_group
        while parent_iter is not None:
            stack.insert(0, parent_iter.name)
            parent_iter = parent_iter.parent_group
        return stack

    def qualified_name(self):
        return self.group_stack.join('_')


def parse_exports_list(export_definition_list, current_revision, parent_group=None, app_only=False,
                       worker_only=False, deprecated=False):
    exports = []

    for e in export_definition_list:
        # Functions marked internal must be assigned a revision number to be sorted later
        if e.get('internal', False):
            if 'addedRevision' in e:
                raise Exception('Internal symbol %s should not have an addedRevision property'
                                % e['name'])
            else:
                e['addedRevision'] = INTERNAL_REVISION
        if 'addedRevision' in e:
            added_revision = int(e['addedRevision'])
            if added_revision > current_revision:
                logging.warn("Omitting '%s' from SDK export because its revision "
                             "(%u) is higher than the current revision (%u)" %
                             (e['name'], added_revision, current_revision))
                continue

        if e['type'] == 'group':
            exports.append(Group(e, parent_group, app_only, worker_only, current_revision,
                           deprecated))
        elif e['type'] == 'forward_struct':
            exports.append(Export(e, app_only, worker_only, deprecated))
        elif e['type'] == 'function':
            exports.append(FunctionExport(e, app_only, worker_only, deprecated))
        elif e['type'] == 'type' or e['type'] == 'define':
            exports.append(FullExport(e, app_only, worker_only, deprecated))
        else:
            raise Exception('Unknown type "%s" in export "%s"' % (e['type'], e['name']))

    return exports


def parse_export_file(filename, internal_sdk_build):
    with open(filename, 'r') as f:
        shim_defs = json.load(f)
        file_revision = int(shim_defs['revision'])

        if file_revision >= INTERNAL_REVISION - 12:
            raise Exception('File revision at %d is approaching INTERNAL_REVISION at %d' %
                            (file_revision, INTERNAL_REVISION))

        current_revision = INTERNAL_REVISION if internal_sdk_build else file_revision
        exports = parse_exports_list(shim_defs['exports'], current_revision)

    return shim_defs['files'], exports


def walk_tree(exports_tree, func, include_groups = False):
    """ Call func on every Export in our tree """
    for e in exports_tree:
        if isinstance(e, Group):
            if include_groups:
                func(e)
            walk_tree(e.exports, func, include_groups)
        else:
            func(e)

