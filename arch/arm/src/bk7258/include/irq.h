/****************************************************************************
 * arch/arm/src/bk7258/include/irq.h
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

/* This file should never be included directly but, rather, only indirectly
 * through nuttx/irq.h
 */

#ifndef __ARCH_ARM_SRC_BK7258_INCLUDE_IRQ_H
#define __ARCH_ARM_SRC_BK7258_INCLUDE_IRQ_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Exception/interrupt vector numbers */

/* Vector  0: Reset stack pointer value */

/* Vector  1: Reset */

#define NVIC_IRQ_NMI               (2)    /* Vector  2: Non-Maskable Interrupt (NMI) */

#define NVIC_IRQ_HARDFAULT         (3)    /* Vector  3: Hard fault */

#define NVIC_IRQ_MEMFAULT          (4)    /* Vector  4: Memory management (MPU) */

#define NVIC_IRQ_BUSFAULT          (5)    /* Vector  5: Bus fault */

#define NVIC_IRQ_USAGEFAULT        (6)    /* Vector  6: Usage fault */

/* Vectors 7-10: Reserved */

#define NVIC_IRQ_SVCALL            (11)   /* Vector 11: SVC call */

#define NVIC_IRQ_DBGMONITOR        (12)   /* Vector 12: Debug Monitor */

/* Vector 13: Reserved */

#define NVIC_IRQ_PENDSV            (14)   /* Vector 14: Pendable system service request */

#define NVIC_IRQ_SYSTICK           (15)   /* Vector 15: System tick */

/* External interrupts (vectors >= 16). These definitions are chip-specific.
 * The BK7258 interrupt numbers below follow the CMSIS IRQn_Type encoding
 * used by the Beken Armino SDK (see bk7236.h): the NuttX IRQ number equals
 * the vector number, i.e. 16 + CMSIS IRQn.
 */

#define NVIC_IRQ_FIRST             (16)   /* Vector number of the first interrupt */

/* BK7258 peripheral interrupts (16 + IRQn) */

#define NVIC_IRQ_UART0             (20)   /* IRQn 4:  UART0 */
#define NVIC_IRQ_UART1             (31)   /* IRQn 15: UART1 */
#define NVIC_IRQ_UART2             (32)   /* IRQn 16: UART2 */

/* IRQ numbers.  The IRQ number corresponds to the vector number and hence
 * maps directly to bits in the NVIC.  NR_IRQS covers all 64 peripheral
 * interrupt lines of the BK7258 (IRQn 0..63, see InterruptMAX_IRQn in the
 * CMSIS device header).
 */

#define NR_IRQS                    CONFIG_BK7258_NR_IRQS

/* NVIC priority levels */

#define NVIC_SYSH_PRIORITY_MIN     0xff /* All bits set in minimum priority */

#define NVIC_SYSH_PRIORITY_DEFAULT 0x40 /* Midpoint is the default */

#define NVIC_SYSH_PRIORITY_MAX     0x00 /* Zero is maximum priority */

#define NVIC_SYSH_PRIORITY_STEP    0x40 /* Three bits priority used, bits[7-6] as group */

#define NVIC_SYSH_PRIORITY_SUBSTEP 0x20 /* Three bits priority used, bit[5] as sub */

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

#undef EXTERN

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLY__ */

#endif /* __ARCH_ARM_SRC_BK7258_INCLUDE_IRQ_H */
