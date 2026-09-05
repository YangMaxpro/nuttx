/****************************************************************************
 * arch/arm/src/bk7258/bk7258_start.c
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
#include <nuttx/init.h>

#include "arm_internal.h"
#include "hardware/bk7258_memorymap.h"
#include "hardware/bk7258_uart.h"

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* g_idle_topstack: _sbss is the start of the BSS region as defined by the
 * linker script.  _ebss lies at the end of the BSS region.  The idle task
 * stack starts at the end of BSS and is of size CONFIG_IDLETHREAD_STACKSIZE.
 * The IDLE thread is the thread that the system boots on and, eventually,
 * becomes the IDLE, do nothing task that runs only when there is nothing
 * else to run.  The heap continues from there until the end of memory.
 * g_idle_topstack is a read-only variable that provides this computed
 * address.
 */

#define HEAP_BASE ((uintptr_t)_ebss + CONFIG_IDLETHREAD_STACKSIZE)

const uintptr_t g_idle_topstack = HEAP_BASE;

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: __start
 *
 * Description:
 *   This is the reset entry point, jumped to by the Beken Armino
 *   bootloader (or the vector table).  It initializes BSS, copies .data,
 *   enables the FPU if configured, initializes the early serial console
 *   and finally starts NuttX.
 *
 ****************************************************************************/


/****************************************************************************
 * Name: bk7258_uart0_pinconfig
 *
 * Description:
 *   Configure the BK7258 UART0 clock and GPIO matrix before early serial
 *   initialization.  The R1 debug header uses GPIO10 for UART0 RX and GPIO11
 *   for UART0 TX.  This function uses only direct register accesses and
 *   runs immediately before early serial initialization.
 *
 ****************************************************************************/

static inline void bk7258_w32(uintptr_t addr, uint32_t val)
{
  *(volatile uint32_t *)addr = val;
}

static inline uint32_t bk7258_r32(uintptr_t addr)
{
  return *(volatile uint32_t *)addr;
}

static void bk7258_uart0_pinconfig(void)
{
  uint32_t reg;

  /* Enable the UART0 device clock and select the 26 MHz crystal at /1. */

  reg = bk7258_r32(BK7258_SYS_CPU_DEVICE_CLK_EN);
  bk7258_w32(BK7258_SYS_CPU_DEVICE_CLK_EN, reg | BK7258_UART0_CKEN);

  reg = bk7258_r32(BK7258_SYS_CPU_CLK_DIV_MODE1);
  bk7258_w32(BK7258_SYS_CPU_CLK_DIV_MODE1,
             reg & ~BK7258_UART0_CLKSEL_MASK);

  /* Keep the UART0 functional clock ungated across the UART soft reset. */

  reg = bk7258_r32(BK7258_UART0_BASE + BK7258_UART_GLOBAL_CTRL_OFFSET);
  bk7258_w32(BK7258_UART0_BASE + BK7258_UART_GLOBAL_CTRL_OFFSET,
             reg | BK7258_UART_CLK_GATE_BYPASS);

  /* GPIO10/GPIO11 are gpio_sys_num[1] fields 2 and 3.  UART0 is mode 0. */

  reg = bk7258_r32(BK7258_GPIO_SYS_FUNC_MODE + 4);
  reg &= ~((0xfu << 8) | (0xfu << 12));
  bk7258_w32(BK7258_GPIO_SYS_FUNC_MODE + 4, reg);

  /* gpio_hal_func_map() uses second-function mode, GPIO_IO_DISABLE and pull-up
   * for these UART pins.  Disable the ordinary GPIO input/output drivers. */

  reg = bk7258_r32(BK7258_AON_GPIO_REG_BASE + 10 * 4);
  reg &= ~(BK7258_GPIO_INPUT_EN | BK7258_GPIO_OUTPUT_EN);
  reg |= BK7258_GPIO_PULL_MODE | BK7258_GPIO_PULL_MODE_EN |
         BK7258_GPIO_2_FUNC_EN;
  bk7258_w32(BK7258_AON_GPIO_REG_BASE + 10 * 4, reg);

  reg = bk7258_r32(BK7258_AON_GPIO_REG_BASE + 11 * 4);
  reg &= ~(BK7258_GPIO_INPUT_EN | BK7258_GPIO_OUTPUT_EN);
  reg |= BK7258_GPIO_PULL_MODE | BK7258_GPIO_PULL_MODE_EN |
         BK7258_GPIO_2_FUNC_EN;
  bk7258_w32(BK7258_AON_GPIO_REG_BASE + 11 * 4, reg);
}


void __start(void)
{
  const uint32_t *src;
  uint32_t *dest;

#ifdef CONFIG_ARCH_FPU
  /* Enable the Cortex-M33 FPU (CPACR) */

  arm_fpuconfig();
#endif

  /* Set BSS to zero */

  for (dest = (uint32_t *)_sbss; dest < (uint32_t *)_ebss; )
    {
      *dest++ = 0;
    }

  /* Copy the program/data from its load address (flash) to its runtime
   * address (SRAM).
   */

  for (src = (const uint32_t *)_eronly,
       dest = (uint32_t *)_sdata; dest < (uint32_t *)_edata;
      )
    {
      *dest++ = *src++;
    }

  /* Perform early serial initialization */

  /* Route and clock the debug UART before touching the console. */

  bk7258_uart0_pinconfig();

#ifdef USE_EARLYSERIALINIT
  arm_earlyserialinit();
#endif

  /* Then start NuttX */

  nx_start();

  /* Shouldn't get here */

  for (; ; );
}
