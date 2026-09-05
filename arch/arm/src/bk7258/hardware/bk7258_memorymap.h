/****************************************************************************
 * arch/arm/src/bk7258/hardware/bk7258_memorymap.h
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

#ifndef __ARCH_ARM_SRC_BK7258_HARDWARE_BK7258_MEMORYMAP_H
#define __ARCH_ARM_SRC_BK7258_HARDWARE_BK7258_MEMORYMAP_H

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* BK7258 memory map (SPE mode, CONFIG_SPE=1 in the Beken Armino SDK).
 *
 * SRAM: 6 blocks of 64K/64K/128K/128K/128K/128K aliased at
 *       0x28000000 (data), 0x00000000 (ITCM), 0x20000000 (DTCM) and
 *       0x02000000 (Flash alias).  AP core gets 336K (0x054000) of
 *       SRAM starting at 0x28000000 (see ram_regions.csv).
 *
 * PSRAM: 8M/16M at 0x60000000.
 *
 * Flash: 8MB NOR at 0x02000000 (alias), executed from XIP region.
 */

#define BK7258_SRAM_BASE          0x28010000  /* AP RAM (336K); 0x28000000 is spinlock */
#define BK7258_SRAM_SIZE          0x00054000

#define BK7258_PSRAM_BASE         0x60000000  /* 8M/16M PSRAM */
#define BK7258_PSRAM_SIZE         (8 * 1024 * 1024)

#define BK7258_FLASH_BASE         0x02000000  /* XIP flash alias */
#define BK7258_FLASH_SIZE         (8 * 1024 * 1024)

/* Peripheral base addresses (SPE mode, offset 0) */

#define BK7258_UART0_BASE         0x44820000
#define BK7258_UART1_BASE         0x45830000
#define BK7258_UART2_BASE         0x45840000


/* System control + AON GPIO controller bases (SPE mode, offset 0).
 * Used by the very early console bring-up to bypass the UART0 clock gate
 * and route GPIO10/GPIO11 to the UART0 RX/TX second function. */

#define BK7258_SYS_REG_BASE          0x44010000
#define BK7258_AON_GPIO_REG_BASE     0x44000400  /* per-pin cfg word at base+id*4 */
/* gpio_sys_num[] peripheral function-select array (4 bits per pin). */
#define BK7258_GPIO_SYS_FUNC_MODE    (BK7258_SYS_REG_BASE + 0x30 * 4)

/* AON GPIO per-pin cfg bit fields (see Armino gpio_struct.h) */
#define BK7258_GPIO_INPUT_EN         (1u << 2)
#define BK7258_GPIO_OUTPUT_EN        (1u << 3)
#define BK7258_GPIO_PULL_MODE        (1u << 4) /* 1 = pull-up */
#define BK7258_GPIO_PULL_MODE_EN     (1u << 5)
#define BK7258_GPIO_2_FUNC_EN        (1u << 6)

/* UART0 GLOBAL_CTRL bit1 = clock gate bypass */
#define BK7258_UART_CLK_GATE_BYPASS  (1 << 1)

/* SYS clock control (word index -> byte offset = idx*4 from SYS base) */
#define BK7258_SYS_CPU_CLK_DIV_MODE1   (BK7258_SYS_REG_BASE + 0x08 * 4) /* 0x44010020 */
#define BK7258_SYS_CPU_DEVICE_CLK_EN   (BK7258_SYS_REG_BASE + 0x0c * 4) /* 0x44010030 */
#define BK7258_UART0_CKEN              (1 << 2)  /* bit2: uart0 device clock enable */
/* In CLK_DIV_MODE1: clkdiv_uart0 bits[9:8], clksel_uart0 bit[10] (0=XTAL,1=APLL) */
#define BK7258_UART0_CLKSEL_MASK       (0x7 << 8)


#endif /* __ARCH_ARM_SRC_BK7258_HARDWARE_BK7258_MEMORYMAP_H */
