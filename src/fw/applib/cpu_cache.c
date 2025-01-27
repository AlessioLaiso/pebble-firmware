/*
 * Copyright 2024 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cpu_cache.h"
#include "syscall/syscall_internal.h"

#include "mcu/cache.h"

DEFINE_SYSCALL(void, memory_cache_flush, void *start, size_t size) {
  // We need to align the address and size properly for the cache functions. It needs to be done
  // before we do syscall_assert_userspace_buffer though, because otherwise the user could
  // potentially abuse this behavior to flush+invalidate kernel memory. Theoretically that
  // shouldn't actually have any really effect, but it's better to be safe than sorry.
  // That should only be possible if the user region is not cache aligned, so in any realistic
  // case this won't even matter.
  uintptr_t start_addr = (uintptr_t)start;
  icache_align(&start_addr, &size);
  dcache_align(&start_addr, &size);
  start = (void *)start_addr;

  if (PRIVILEGE_WAS_ELEVATED) {
    syscall_assert_userspace_buffer(start, size);
  }

  if (dcache_is_enabled()) {
    dcache_flush(start, size);
  }
  if (icache_is_enabled()) {
    icache_invalidate(start, size);
  }
}
