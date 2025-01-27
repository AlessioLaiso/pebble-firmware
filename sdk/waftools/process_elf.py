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

import objcopy
import pebble_sdk_gcc


# TODO: PBL-33841 Make this a feature
def generate_bin_file(task_gen, bin_type, elf_file, has_pkjs, has_worker):
    """
    Generate bin file by injecting metadata from elf file and resources file
    :param task_gen: the task generator instance
    :param bin_type: the type of binary being built (app, worker, lib)
    :param elf_file: the path to the compiled elf file
    :param has_pkjs: boolean for whether the build contains PebbleKit JS files
    :param has_worker: boolean for whether the build contains a worker binary
    :return: the modified binary file with injected metadata
    """
    platform_build_node = task_gen.bld.path.get_bld().find_node(task_gen.bld.env.BUILD_DIR)

    packaged_files = [elf_file]
    resources_file = None
    if bin_type != 'worker':
        resources_file = platform_build_node.find_or_declare('app_resources.pbpack')
        packaged_files.append(resources_file)

    raw_bin_file = platform_build_node.make_node('pebble-{}.raw.bin'.format(bin_type))
    bin_file = platform_build_node.make_node('pebble-{}.bin'.format(bin_type))

    task_gen.bld(rule=objcopy.objcopy_bin, source=elf_file, target=raw_bin_file)
    pebble_sdk_gcc.gen_inject_metadata_rule(task_gen.bld,
                                            src_bin_file=raw_bin_file,
                                            dst_bin_file=bin_file,
                                            elf_file=elf_file,
                                            resource_file=resources_file,
                                            timestamp=task_gen.bld.env.TIMESTAMP,
                                            has_pkjs=has_pkjs,
                                            has_worker=has_worker)
    return bin_file
