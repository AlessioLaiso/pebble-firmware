#!/usr/bin/env python
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


import zipfile
import argparse
import json
import base64
import os

CHUNK_SIZE = (1024 * 3) / 4

def chunk_generator(stream):
  while True:
    chunk = stream.read(CHUNK_SIZE)
    yield chunk
    if (len(chunk) != CHUNK_SIZE):
      break

def dump_base64(data, outpath):
  with open(outpath, 'w') as outfile:
    for chunk in chunk_generator(data):
      outfile.write(base64.b64encode(chunk))
      outfile.write('\n')

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description='Unpack a Pebble bundle\'s binary payload into Base64 format for serial loading')
  parser.add_argument('bundle', help='pbz bundle to unpack')
  args = parser.parse_args()

  if args.bundle:
    bundlename = os.path.basename(args.bundle).rsplit('.pbz', 1)[0]
    with zipfile.ZipFile(args.bundle, 'r') as pbz:
      manifest = json.load(pbz.open('manifest.json'))
      firmwarefile = manifest['firmware']['name']
      if 'firmware' in manifest:
        dump_base64(pbz.open(manifest['firmware']['name']), bundlename + '.firmware.base64')
      if 'resources' in manifest:
        dump_base64(pbz.open(manifest['resources']['name']), bundlename + '.resources.base64')
  else:
    parser.print_help()
    sys.exit(1)
