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

import argparse
import sys
import json
import struct

def main(pack_path, manifest_path):
    with open(manifest_path, 'r') as f:
        manifest = json.load(f)

    resource_names = []
    for r in manifest['media']:
        if r['type'] == 'png-trans':
            resource_names.append(r['defName'] + '_WHITE')
            resource_names.append(r['defName'] + '_BLACK')
        else:
            resource_names.append(r['defName'])

    with open(pack_path, 'rb') as f:
        header = f.read(4116)

    def resource_generator(tbl, num):
        for i in xrange(0, num * 16, 16):
            yield struct.unpack('<IIII', tbl[i:i + 16])

    (num_resources, res_version) = struct.unpack('<I16s', header[:20])


    print 'number of resources: {}'.format(num_resources)
    print 'resource pack version: {}'.format(res_version)
    print 'resource entries:'
    print ''
    print '{:<32s}\t{:>8s}\t{:>8s}\t{:>8s}'.format('name', 'offset', 'size', 'crc')
    print '{:<32s}\t{:>8s}\t{:>8s}\t{:>8s}'.format('----', '------', '----', '---')
    for x in resource_generator(header[20:], num_resources):
        (index, offset, size, crc) = x
        print '{:<32s}\t{:>8d}\t{:>8d}\t{:>08x}'.format(resource_names[index], offset, size, crc)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('resource_pack')
    parser.add_argument('resource_map')
    args = parser.parse_args()

    main(args.resource_pack, args.resource_map)
