/****************************************************************************
 * arch/arm/src/bk7258/bk7258_allocateheap.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <stdint.h>

#include <nuttx/arch.h>
#include <nuttx/kmalloc.h>

#include "arm_internal.h"
#include "hardware/bk7258_memorymap.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_allocate_heap
 *
 * Description:
 *   This heap function is called before the RTOS is started to provide the
 *   initial heap.  The heap starts at the end of the idle task stack
 *   (g_idle_topstack) and extends to the end of the BK7258 AP SRAM
 *   (336K @ 0x28000000).
 *
 ****************************************************************************/

void up_allocate_heap(FAR void **heap_start, size_t *heap_size)
{
  *heap_start = (FAR void *)g_idle_topstack;
  *heap_size  = BK7258_SRAM_BASE + BK7258_SRAM_SIZE - g_idle_topstack;
}

/****************************************************************************
 * Name: arm_addregion
 *
 * Description:
 *   Add the BK7258 PSRAM (8M @ 0x60000000) as an additional heap region
 *   when CONFIG_MM_REGIONS > 1.  This gives the system access to the full
 *   8M PSRAM for dynamic allocations (audio buffers, display buffers, etc).
 *
 ****************************************************************************/

#if CONFIG_MM_REGIONS > 1
void arm_addregion(void)
{
  kumm_addregion((FAR void *)BK7258_PSRAM_BASE, BK7258_PSRAM_SIZE);
}
#endif
