/****************************************************************************
 * arch/arm/src/bk7258/bk7258_serial.c
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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <nuttx/arch.h>
#include <nuttx/irq.h>
#include <nuttx/serial/serial.h>

#include "arm_internal.h"
#include "chip.h"
#include "hardware/bk7258_uart.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* If we are not using the serial driver for the console, then we still must
 * provide some minimal implementation of up_putc.
 */

#ifdef USE_SERIALDRIVER

/* Which UART shall be the console? */

#if defined(CONFIG_BK7258_UART0_SERIAL_CONSOLE)
#  define CONSOLE_UART 0
#elif defined(CONFIG_BK7258_UART1_SERIAL_CONSOLE)
#  define CONSOLE_UART 1
#elif defined(CONFIG_BK7258_UART2_SERIAL_CONSOLE)
#  define CONSOLE_UART 2
#endif

/* UART register helpers */

#define BK7258_UART_REG(uart, offset) \
  (*(volatile uint32_t *)((uart)->base + (offset)))

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct bk7258_uart_s
{
  uintptr_t base;       /* UART register base address */
  uint32_t  irq;        /* NVIC IRQ number (vector) */
  uint32_t  baud;       /* Configured baud rate */
  uint32_t  parity;     /* 0=None, 1=Odd, 2=Even */
  uint32_t  bits;       /* Number of data bits (5..8) */
  bool      two_stop;   /* Two stop bits */
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int  bk7258_setup(struct uart_dev_s *dev);
static void bk7258_shutdown(struct uart_dev_s *dev);
static int  bk7258_attach(struct uart_dev_s *dev);
static void bk7258_detach(struct uart_dev_s *dev);
static int  uart_interrupt(int irq, void *context, FAR void *arg);
static int  bk7258_ioctl(struct file *filep, int cmd, unsigned long arg);
static int  bk7258_receive(struct uart_dev_s *dev, unsigned int *status);
static void bk7258_rxint(struct uart_dev_s *dev, bool enable);
static bool bk7258_rxavailable(struct uart_dev_s *dev);
static void bk7258_send(struct uart_dev_s *dev, int ch);
static void bk7258_txint(struct uart_dev_s *dev, bool enable);
static bool bk7258_txready(struct uart_dev_s *dev);
static bool bk7258_txempty(struct uart_dev_s *dev);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct uart_ops_s g_bk7258_uart_ops =
{
  .setup          = bk7258_setup,
  .shutdown       = bk7258_shutdown,
  .attach         = bk7258_attach,
  .detach         = bk7258_detach,
  .ioctl          = bk7258_ioctl,
  .receive        = bk7258_receive,
  .rxint          = bk7258_rxint,
  .rxavailable    = bk7258_rxavailable,
  .send           = bk7258_send,
  .txint          = bk7258_txint,
  .txready        = bk7258_txready,
  .txempty        = bk7258_txempty,
};

/* UART 0 */

#ifdef CONFIG_BK7258_UART0
static char g_uart0rxbuffer[CONFIG_BK7258_UART0_RXBUFSIZE];
static char g_uart0txbuffer[CONFIG_BK7258_UART0_TXBUFSIZE];

static struct bk7258_uart_s g_bk7258_uart0priv =
{
  .base       = BK7258_UART0_BASE,
  .irq        = NVIC_IRQ_UART0,
  .baud       = CONFIG_BK7258_UART0_BAUD,
  .parity     = CONFIG_BK7258_UART0_PARITY,
  .bits       = CONFIG_BK7258_UART0_BITS,
  .two_stop   = CONFIG_BK7258_UART0_2STOP,
};

static struct uart_dev_s g_bk7258_uart0port =
{
  .recv       =
  {
    .size     = CONFIG_BK7258_UART0_RXBUFSIZE,
    .buffer   = g_uart0rxbuffer,
  },
  .xmit       =
  {
    .size     = CONFIG_BK7258_UART0_TXBUFSIZE,
    .buffer   = g_uart0txbuffer,
  },
  .ops        = &g_bk7258_uart_ops,
  .priv       = &g_bk7258_uart0priv,
};
#endif

/* UART 1 */

#ifdef CONFIG_BK7258_UART1
static char g_uart1rxbuffer[CONFIG_BK7258_UART1_RXBUFSIZE];
static char g_uart1txbuffer[CONFIG_BK7258_UART1_TXBUFSIZE];

static struct bk7258_uart_s g_bk7258_uart1priv =
{
  .base       = BK7258_UART1_BASE,
  .irq        = NVIC_IRQ_UART1,
  .baud       = CONFIG_BK7258_UART1_BAUD,
  .parity     = CONFIG_BK7258_UART1_PARITY,
  .bits       = CONFIG_BK7258_UART1_BITS,
  .two_stop   = CONFIG_BK7258_UART1_2STOP,
};

static struct uart_dev_s g_bk7258_uart1port =
{
  .recv       =
  {
    .size     = CONFIG_BK7258_UART1_RXBUFSIZE,
    .buffer   = g_uart1rxbuffer,
  },
  .xmit       =
  {
    .size     = CONFIG_BK7258_UART1_TXBUFSIZE,
    .buffer   = g_uart1txbuffer,
  },
  .ops        = &g_bk7258_uart_ops,
  .priv       = &g_bk7258_uart1priv,
};
#endif

/* UART 2 */

#ifdef CONFIG_BK7258_UART2
static char g_uart2rxbuffer[CONFIG_BK7258_UART2_RXBUFSIZE];
static char g_uart2txbuffer[CONFIG_BK7258_UART2_TXBUFSIZE];

static struct bk7258_uart_s g_bk7258_uart2priv =
{
  .base       = BK7258_UART2_BASE,
  .irq        = NVIC_IRQ_UART2,
  .baud       = CONFIG_BK7258_UART2_BAUD,
  .parity     = CONFIG_BK7258_UART2_PARITY,
  .bits       = CONFIG_BK7258_UART2_BITS,
  .two_stop   = CONFIG_BK7258_UART2_2STOP,
};

static struct uart_dev_s g_bk7258_uart2port =
{
  .recv       =
  {
    .size     = CONFIG_BK7258_UART2_RXBUFSIZE,
    .buffer   = g_uart2rxbuffer,
  },
  .xmit       =
  {
    .size     = CONFIG_BK7258_UART2_TXBUFSIZE,
    .buffer   = g_uart2txbuffer,
  },
  .ops        = &g_bk7258_uart_ops,
  .priv       = &g_bk7258_uart2priv,
};
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: bk7258_uart_configure
 *
 * Description:
 *   Apply the UART configuration (baud, bits, parity, stop bits) to the
 *   hardware.  baud = UART_CLOCK / (clk_div + 1), i.e. the clk_div field
 *   must hold UART_CLOCK / baud - 1.
 *
 ****************************************************************************/

static void bk7258_uart_configure(struct bk7258_uart_s *priv)
{
  uint32_t regval;

  /* Soft reset the UART block */

  BK7258_UART_REG(priv, BK7258_UART_GLOBAL_CTRL_OFFSET) = 1;

  /* Build the CONFIG register: enable TX/RX, data bits, parity, stop
   * bits and the baud rate divisor.
   */

  regval  = UART_CONFIG_TX_ENABLE | UART_CONFIG_RX_ENABLE;
  regval |= ((priv->bits - 5) << UART_CONFIG_DATA_BITS_SHIFT) &
            UART_CONFIG_DATA_BITS_MASK;

  if (priv->parity != 0)
    {
      regval |= UART_CONFIG_PARITY_EN;
      if (priv->parity == 1)
        {
          regval |= UART_CONFIG_PARITY_ODD;
        }
    }

  if (priv->two_stop)
    {
      regval |= UART_CONFIG_STOP_BITS;
    }

  regval |= ((uint32_t)BK7258_UART_CLK_DIV(priv->baud) <<
             UART_CONFIG_CLK_DIV_SHIFT) & UART_CONFIG_CLK_DIV_MASK;

  BK7258_UART_REG(priv, BK7258_UART_CONFIG_OFFSET) = regval;

  /* Clear any pending interrupts */

  BK7258_UART_REG(priv, BK7258_UART_INT_STATUS_OFFSET) = 0xff;
}

/****************************************************************************
 * Name: bk7258_setup
 *
 * Description:
 *   Configure the UART baud, bits, parity, etc.
 *
 ****************************************************************************/

static int bk7258_setup(struct uart_dev_s *dev)
{
  struct bk7258_uart_s *priv = (struct bk7258_uart_s *)dev->priv;

  bk7258_uart_configure(priv);

  return 0;
}

/****************************************************************************
 * Name: bk7258_shutdown
 *
 * Description:
 *   Disable the UART.
 *
 ****************************************************************************/

static void bk7258_shutdown(struct uart_dev_s *dev)
{
  struct bk7258_uart_s *priv = (struct bk7258_uart_s *)dev->priv;

  /* Disable the UART and all interrupts */

  BK7258_UART_REG(priv, BK7258_UART_INT_ENABLE_OFFSET) = 0;
  BK7258_UART_REG(priv, BK7258_UART_CONFIG_OFFSET) = 0;
}

/****************************************************************************
 * Name: uart_interrupt
 *
 * Description:
 *   Common UART interrupt handler for all BK7258 UARTs.
 *
 ****************************************************************************/

static int uart_interrupt(int irq, void *context, FAR void *arg)
{
  struct uart_dev_s *dev = (struct uart_dev_s *)arg;
  struct bk7258_uart_s *priv = (struct bk7258_uart_s *)dev->priv;
  uint32_t intstatus;

  /* Read the interrupt status register (reading clears the flags) */

  intstatus = BK7258_UART_REG(priv, BK7258_UART_INT_STATUS_OFFSET);

  /* RX data available: drain the FIFO into the RX buffer */

  if ((intstatus & UART_INT_RX_FIFO_NEED_READ) != 0)
    {
      uart_recvchars(dev);
    }

  /* TX FIFO has space: feed more characters */

  if ((intstatus & UART_INT_TX_FIFO_NEED_WRITE) != 0)
    {
      uart_xmitchars(dev);
    }

  return 0;
}

/****************************************************************************
 * Name: bk7258_attach
 *
 * Description:
 *   Attach the UART interrupt handlers.
 *
 ****************************************************************************/

static int bk7258_attach(struct uart_dev_s *dev)
{
  struct bk7258_uart_s *priv = (struct bk7258_uart_s *)dev->priv;

  return irq_attach(priv->irq, uart_interrupt, dev);
}

/****************************************************************************
 * Name: bk7258_detach
 *
 * Description:
 *   Detach the UART interrupt handlers.
 *
 ****************************************************************************/

static void bk7258_detach(struct uart_dev_s *dev)
{
  struct bk7258_uart_s *priv = (struct bk7258_uart_s *)dev->priv;

  irq_detach(priv->irq);
}

/****************************************************************************
 * Name: bk7258_ioctl
 *
 * Description:
 *   Standard NuttX driver IOCTL handler.
 *
 ****************************************************************************/

static int bk7258_ioctl(struct file *filep, int cmd, unsigned long arg)
{
  return -ENOTTY;
}

/****************************************************************************
 * Name: bk7258_receive
 *
 * Description:
 *   Called (usually) from the interrupt level to receive one character
 *   from the UART.
 *
 ****************************************************************************/

static int bk7258_receive(struct uart_dev_s *dev, unsigned int *status)
{
  struct bk7258_uart_s *priv = (struct bk7258_uart_s *)dev->priv;
  uint32_t regval;

  /* RX data lives in bits8:15 of the FIFO PORT register */

  regval = BK7258_UART_REG(priv, BK7258_UART_FIFO_PORT_OFFSET);
  *status = 0;

  return (regval & UART_FIFO_PORT_RX_DATA) >> 8;
}

/****************************************************************************
 * Name: bk7258_rxint
 *
 * Description:
 *   Called to enable or disable RX interrupts.
 *
 ****************************************************************************/

static void bk7258_rxint(struct uart_dev_s *dev, bool enable)
{
  struct bk7258_uart_s *priv = (struct bk7258_uart_s *)dev->priv;
  uint32_t regval;

  regval = BK7258_UART_REG(priv, BK7258_UART_INT_ENABLE_OFFSET);
  if (enable)
    {
      regval |= UART_INT_RX_FIFO_NEED_READ;
    }
  else
    {
      regval &= ~UART_INT_RX_FIFO_NEED_READ;
    }

  BK7258_UART_REG(priv, BK7258_UART_INT_ENABLE_OFFSET) = regval;
}

/****************************************************************************
 * Name: bk7258_rxavailable
 *
 * Description:
 *   Return true if the receive FIFO holds data.
 *
 ****************************************************************************/

static bool bk7258_rxavailable(struct uart_dev_s *dev)
{
  struct bk7258_uart_s *priv = (struct bk7258_uart_s *)dev->priv;
  uint32_t regval;

  regval = BK7258_UART_REG(priv, BK7258_UART_FIFO_STATUS_OFFSET);

  return (regval & UART_FIFO_STATUS_FIFO_RD_READY) != 0;
}

/****************************************************************************
 * Name: bk7258_send
 *
 * Description:
 *   This method will send one byte on the UART.
 *
 ****************************************************************************/

static void bk7258_send(struct uart_dev_s *dev, int ch)
{
  struct bk7258_uart_s *priv = (struct bk7258_uart_s *)dev->priv;

  /* TX data lives in bits0:7 of the FIFO PORT register */

  BK7258_UART_REG(priv, BK7258_UART_FIFO_PORT_OFFSET) =
    (ch & UART_FIFO_PORT_TX_DATA);
}

/****************************************************************************
 * Name: bk7258_txint
 *
 * Description:
 *   Called to enable or disable TX interrupts.
 *
 ****************************************************************************/

static void bk7258_txint(struct uart_dev_s *dev, bool enable)
{
  struct bk7258_uart_s *priv = (struct bk7258_uart_s *)dev->priv;
  uint32_t regval;

  regval = BK7258_UART_REG(priv, BK7258_UART_INT_ENABLE_OFFSET);
  if (enable)
    {
      regval |= UART_INT_TX_FIFO_NEED_WRITE;
    }
  else
    {
      regval &= ~UART_INT_TX_FIFO_NEED_WRITE;
    }

  BK7258_UART_REG(priv, BK7258_UART_INT_ENABLE_OFFSET) = regval;
}

/****************************************************************************
 * Name: bk7258_txready
 *
 * Description:
 *   Return true if the transmit FIFO is ready to accept a byte.
 *
 ****************************************************************************/

static bool bk7258_txready(struct uart_dev_s *dev)
{
  struct bk7258_uart_s *priv = (struct bk7258_uart_s *)dev->priv;
  uint32_t regval;

  regval = BK7258_UART_REG(priv, BK7258_UART_FIFO_STATUS_OFFSET);

  return (regval & UART_FIFO_STATUS_FIFO_WR_READY) != 0;
}

/****************************************************************************
 * Name: bk7258_txempty
 *
 * Description:
 *   Return true if the transmit FIFO is empty.
 *
 ****************************************************************************/

static bool bk7258_txempty(struct uart_dev_s *dev)
{
  struct bk7258_uart_s *priv = (struct bk7258_uart_s *)dev->priv;
  uint32_t regval;

  regval = BK7258_UART_REG(priv, BK7258_UART_FIFO_STATUS_OFFSET);

  return (regval & UART_FIFO_STATUS_TX_FIFO_EMPTY) != 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef USE_EARLYSERIALINIT

/****************************************************************************
 * Name: arm_earlyserialinit
 *
 * Description:
 *   Performs the low level UART initialization early in debug so that the
 *   serial console will be available during bootup.  This must be called
 *   before arm_serialinit.  NOTE:  This function depends on GPIO pin
 *   configuration performed in the board initialization.
 *
 ****************************************************************************/

void arm_earlyserialinit(void)
{
  /* Configure the console UART and register it as /dev/console */

#if defined(CONFIG_BK7258_UART0_SERIAL_CONSOLE) && defined(CONFIG_BK7258_UART0)
  bk7258_setup(&g_bk7258_uart0port);
  uart_register("/dev/console", &g_bk7258_uart0port);
#elif defined(CONFIG_BK7258_UART1_SERIAL_CONSOLE) && defined(CONFIG_BK7258_UART1)
  bk7258_setup(&g_bk7258_uart1port);
  uart_register("/dev/console", &g_bk7258_uart1port);
#elif defined(CONFIG_BK7258_UART2_SERIAL_CONSOLE) && defined(CONFIG_BK7258_UART2)
  bk7258_setup(&g_bk7258_uart2port);
  uart_register("/dev/console", &g_bk7258_uart2port);
#endif
}

#endif /* USE_EARLYSERIALINIT */

/****************************************************************************
 * Name: arm_serialinit
 *
 * Description:
 *   Register the serial console and all serial ports.  This assumes that
 *   arm_earlyserialinit was called previously.
 *
 ****************************************************************************/

void arm_serialinit(void)
{
#ifdef CONFIG_BK7258_UART0
  uart_register("/dev/ttyS0", &g_bk7258_uart0port);
#endif

#ifdef CONFIG_BK7258_UART1
  uart_register("/dev/ttyS1", &g_bk7258_uart1port);
#endif

#ifdef CONFIG_BK7258_UART2
  uart_register("/dev/ttyS2", &g_bk7258_uart2port);
#endif
}

/****************************************************************************
 * Name: up_putc
 *
 * Description:
 *   Provide priority, low-level access to support OS debug writes.
 *
 ****************************************************************************/

void up_putc(int ch)
{
#if defined(CONFIG_BK7258_UART0_SERIAL_CONSOLE) && defined(CONFIG_BK7258_UART0)
  struct bk7258_uart_s *priv = &g_bk7258_uart0priv;
#elif defined(CONFIG_BK7258_UART1_SERIAL_CONSOLE) && defined(CONFIG_BK7258_UART1)
  struct bk7258_uart_s *priv = &g_bk7258_uart1priv;
#elif defined(CONFIG_BK7258_UART2_SERIAL_CONSOLE) && defined(CONFIG_BK7258_UART2)
  struct bk7258_uart_s *priv = &g_bk7258_uart2priv;
#else
  return;
#endif
  uint32_t regval;

  /* Wait for the transmit FIFO to be ready */

  do
    {
      regval = BK7258_UART_REG(priv, BK7258_UART_FIFO_STATUS_OFFSET);
    }
  while ((regval & UART_FIFO_STATUS_FIFO_WR_READY) == 0);

  /* Send the character */

  BK7258_UART_REG(priv, BK7258_UART_FIFO_PORT_OFFSET) =
    (ch & UART_FIFO_PORT_TX_DATA);

  /* CR-LF handling: NuttX console expects \n to move to the next line */

  if (ch == '\n')
    {
      do
        {
          regval = BK7258_UART_REG(priv, BK7258_UART_FIFO_STATUS_OFFSET);
        }
      while ((regval & UART_FIFO_STATUS_FIFO_WR_READY) == 0);

      BK7258_UART_REG(priv, BK7258_UART_FIFO_PORT_OFFSET) = '\r';
    }
}

#else /* USE_SERIALDRIVER */

/****************************************************************************
 * Name: up_putc
 *
 * Description:
 *   Provide priority, low-level access to support OS debug writes in the
 *   minimal (no serial driver) configuration.
 *
 ****************************************************************************/

void up_putc(int ch)
{
#if defined(CONFIG_BK7258_UART0_SERIAL_CONSOLE) && defined(CONFIG_BK7258_UART0)
  struct bk7258_uart_s *priv = &g_bk7258_uart0priv;
#elif defined(CONFIG_BK7258_UART1_SERIAL_CONSOLE) && defined(CONFIG_BK7258_UART1)
  struct bk7258_uart_s *priv = &g_bk7258_uart1priv;
#elif defined(CONFIG_BK7258_UART2_SERIAL_CONSOLE) && defined(CONFIG_BK7258_UART2)
  struct bk7258_uart_s *priv = &g_bk7258_uart2priv;
#else
  return;
#endif
  uint32_t regval;

  /* Wait for the transmit FIFO to be ready */

  do
    {
      regval = BK7258_UART_REG(priv, BK7258_UART_FIFO_STATUS_OFFSET);
    }
  while ((regval & UART_FIFO_STATUS_FIFO_WR_READY) == 0);

  /* Send the character */

  BK7258_UART_REG(priv, BK7258_UART_FIFO_PORT_OFFSET) =
    (ch & UART_FIFO_PORT_TX_DATA);
}

#endif /* USE_SERIALDRIVER */
