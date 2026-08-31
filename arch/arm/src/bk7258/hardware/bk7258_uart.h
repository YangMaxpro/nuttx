/****************************************************************************
 * arch/arm/src/bk7258/hardware/bk7258_uart.h
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

#ifndef __ARCH_ARM_SRC_BK7258_HARDWARE_BK7258_UART_H
#define __ARCH_ARM_SRC_BK7258_HARDWARE_BK7258_UART_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include "bk7258_memorymap.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* The BK7258 UART is a Beken-proprietary design (not PL011/NS16550).
 * All registers are 32-bit and laid out as a packed structure, see the
 * Armino SDK uart_struct.h.  The register offsets below are expressed in
 * bytes and equal (struct index * 4).
 */

#define BK7258_UART_DEV_ID_OFFSET         0x00 /* Device ID */
#define BK7258_UART_DEV_VERSION_OFFSET    0x04 /* Device version */
#define BK7258_UART_GLOBAL_CTRL_OFFSET    0x08 /* bit0 soft reset, bit1 clk gate bypass */
#define BK7258_UART_DEV_STATUS_OFFSET     0x0c /* Device status */
#define BK7258_UART_CONFIG_OFFSET         0x10 /* Main configuration (see below) */
#define BK7258_UART_FIFO_CONFIG_OFFSET    0x14 /* bit0:7 tx threshold, bit8:15 rx threshold */
#define BK7258_UART_FIFO_STATUS_OFFSET    0x18 /* FIFO status flags */
#define BK7258_UART_FIFO_PORT_OFFSET      0x1c /* bit0:7 TX data, bit8:15 RX data */
#define BK7258_UART_INT_ENABLE_OFFSET     0x20 /* Interrupt enable */
#define BK7258_UART_INT_STATUS_OFFSET     0x24 /* Interrupt status (read clears) */
#define BK7258_UART_FLOW_CTRL_OFFSET      0x28 /* Flow control */
#define BK7258_UART_WAKE_CONFIG_OFFSET    0x2c /* Wake up config */

/* CONFIG register (offset 0x10) bit definitions */

#define UART_CONFIG_TX_ENABLE             (1 << 0)  /* bit0: UART TX enable */
#define UART_CONFIG_RX_ENABLE             (1 << 1)  /* bit1: UART RX enable */
#define UART_CONFIG_DATA_BITS_SHIFT       (3)       /* bits3:4: 0=5b,1=6b,2=7b,3=8b */
#define UART_CONFIG_DATA_BITS_MASK        (3 << UART_CONFIG_DATA_BITS_SHIFT)
#define UART_CONFIG_DATA_BITS_8           (3 << UART_CONFIG_DATA_BITS_SHIFT)
#define UART_CONFIG_PARITY_EN             (1 << 5)  /* bit5: parity enable */
#define UART_CONFIG_PARITY_ODD            (1 << 6)  /* bit6: 0=Even, 1=Odd */
#define UART_CONFIG_STOP_BITS             (1 << 7)  /* bit7: 0=1 stop, 1=2 stop */
#define UART_CONFIG_CLK_DIV_SHIFT         (8)       /* bits8:23: clk_div = uart_clk / baud */
#define UART_CONFIG_CLK_DIV_MASK          (0xffff << UART_CONFIG_CLK_DIV_SHIFT)

/* FIFO STATUS register (offset 0x18) bit definitions */

#define UART_FIFO_STATUS_TX_FIFO_COUNT    (0xff << 0)  /* bits0:7:  tx fifo count */
#define UART_FIFO_STATUS_RX_FIFO_COUNT    (0xff << 8)  /* bits8:15: rx fifo count */
#define UART_FIFO_STATUS_TX_FIFO_FULL     (1 << 16)    /* bit16 */
#define UART_FIFO_STATUS_TX_FIFO_EMPTY    (1 << 17)    /* bit17 */
#define UART_FIFO_STATUS_RX_FIFO_FULL     (1 << 18)    /* bit18 */
#define UART_FIFO_STATUS_RX_FIFO_EMPTY    (1 << 19)    /* bit19 */
#define UART_FIFO_STATUS_FIFO_WR_READY    (1 << 20)    /* bit20: TX ready */
#define UART_FIFO_STATUS_FIFO_RD_READY    (1 << 21)    /* bit21: RX data available */

/* FIFO PORT register (offset 0x1c) bit definitions */

#define UART_FIFO_PORT_TX_DATA            (0xff << 0)  /* bits0:7:  write TX data */
#define UART_FIFO_PORT_RX_DATA            (0xff << 8)  /* bits8:15: read RX data */

/* INT ENABLE / INT STATUS registers (offsets 0x20 / 0x24) bit definitions */

#define UART_INT_TX_FIFO_NEED_WRITE       (1 << 0)  /* bit0 */
#define UART_INT_RX_FIFO_NEED_READ        (1 << 1)  /* bit1 */
#define UART_INT_RX_FIFO_OVERFLOW         (1 << 2)  /* bit2 */
#define UART_INT_RX_PARITY_ERR            (1 << 3)  /* bit3 */
#define UART_INT_RX_STOP_BITS_ERR         (1 << 4)  /* bit4 */
#define UART_INT_TX_FINISH                (1 << 5)  /* bit5 */
#define UART_INT_RX_FINISH                (1 << 6)  /* bit6 */
#define UART_INT_RXD_WAKEUP               (1 << 7)  /* bit7 */

/* UART clock: 26 MHz crystal (CONFIG_XTAL_FREQ in the Armino SDK).
 * baud rate = UART_CLOCK / (clk_div + 1), hence clk_div = 26M / baud - 1.
 */

#define BK7258_UART_CLOCK                 (26000000)

#define BK7258_UART_CLK_DIV(baud) \
  ((BK7258_UART_CLOCK / (baud)) - 1)

#endif /* __ARCH_ARM_SRC_BK7258_HARDWARE_BK7258_UART_H */
