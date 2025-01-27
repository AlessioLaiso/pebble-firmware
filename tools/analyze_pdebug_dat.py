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

# This file can be used to process output from waf's extra tool parallel_debug.py that enables
# the profiling of tasks in a waf build.
#
# To use this!
# * Grab the parallel_debug.py file from the waf repo and throw it in your waftools directory
#   https://raw.githubusercontent.com/waf-project/waf/master/waflib/extras/parallel_debug.py
# * Add "conf.load('parallel_debug', tooldir='waftools')" to the top of def configure in the
#   root wscript
# * Run your build! This should produce a pdebug.data and pdebug.svg in your root
# * Run this script with the current directory being the root of your repo
#
# Output will look something like the following:
# c                                                             5326  44.79%  251.37s   0.05s
# .pbi                                                          2496  40.42%  226.83s   0.09s
# run_test                                                       434  08.29%   46.55s   0.11s
# clar_main.c,clar.h                                             434  03.68%   20.63s   0.05s
# cprogram                                                       436  02.56%   14.39s   0.03s
# .png                                                            40  00.14%    0.79s   0.02s
# .pdc                                                            16  00.03%    0.18s   0.01s
# python -m unittest discover -s /Users/brad/pebble/tintin/t       2  00.03%    0.16s   0.08s
# .apng                                                            8  00.02%    0.13s   0.02s
# .c                                                              68  00.02%    0.09s   0.00s
# pbi2png.py                                                       2  00.01%    0.07s   0.03s
# cstlib                                                           2  00.01%    0.06s   0.03s
#
# Columns are task name, count, total percentage, total time, average time per task

tasks_by_thread = {}


class Task(object):
    pass

# process all lines
with open('pdebug.dat') as f:
    import csv
    reader = csv.reader(f, delimiter=' ')
    for row in reader:
        t = Task()
        t.thread_id = int(row[0])
        t.task_id = int(row[1])
        t.start_time = float(row[2])
        t.task_name = row[3]

        if t.task_name.startswith("'"):
            t.task_name = t.task_name[1:]
        if t.task_name.endswith("'"):
            t.task_name = t.task_name[:-1]

        thread_tasks = tasks_by_thread.setdefault(t.thread_id, [])
        thread_tasks.append(t)

# assign durations
for thread_tasks in tasks_by_thread.values():
    for i in xrange(len(thread_tasks) - 1):
        thread_tasks[i].duration = thread_tasks[i+1].start_time - thread_tasks[i].start_time

    # Can't guess the duration for the final task because the values only have start times :(
    thread_tasks[-1].duration = 0

# Flatten the dict into a big list
all_tasks = [item for sublist in tasks_by_thread.values() for item in sublist]

tasks_by_task_type = {}
for t in all_tasks:
    task_type_name = t.task_name

    if task_type_name.endswith('.pbi'):
        task_type_name = '.pbi'
    elif task_type_name.endswith('.png'):
        task_type_name = '.png'
    elif task_type_name.endswith('.apng'):
        task_type_name = '.apng'
    elif task_type_name.endswith('.pdc'):
        task_type_name = '.pdc'
    elif task_type_name.endswith('.c'):
        task_type_name = '.c'

    task_type_tasks = tasks_by_task_type.setdefault(task_type_name, [])
    task_type_tasks.append(t)


class TaskType(object):
    pass

task_types = []
total_duration = 0.0
for task_type_name, tasks in tasks_by_task_type.items():
    tt = TaskType()

    tt.name = task_type_name
    tt.total_duration = reduce(lambda accumulated, x: accumulated + x.duration, tasks, 0.0)
    tt.average_duration = tt.total_duration / len(tasks)
    tt.count = len(tasks)

    task_types.append(tt)

    total_duration += tt.total_duration

task_types.sort(key=lambda x: -x.total_duration)

for tt in task_types:
    percentage_of_total = (tt.total_duration / total_duration) * 100

    print "%-60s %5u  %05.2f%% %7.2fs %6.2fs" % \
        (tt.name[:58], tt.count, percentage_of_total, tt.total_duration, tt.average_duration)
