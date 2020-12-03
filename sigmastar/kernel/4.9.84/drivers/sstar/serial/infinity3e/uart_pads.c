/*
* uart_pads.c- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/

#include <linux/serial.h>

#include "ms_uart.h"
#include "gpio.h"

int ms_uart_get_padmux(int tx_pad, u8 *padmux, u8 *pad_mode)
{
    int ret = 0;

    switch(tx_pad)
    {
        case PAD_FUART_RX:
            *padmux=MUX_FUART;
            *pad_mode=0x1;
            break;

        case PAD_GPIO0:
            *padmux=MUX_FUART;
            *pad_mode=0x2;
            break;

        case PAD_UART0_TX:
            *padmux=MUX_UART0;
            *pad_mode=0x1;
            break;

        case PAD_FUART_TX:
            *padmux=MUX_UART0;
            *pad_mode=0x2;
            break;

        case PAD_GPIO4:
            *padmux=MUX_UART0;
            *pad_mode=0x3;
            break;

        case PAD_UART1_TX:
            *padmux=MUX_UART1;
            *pad_mode=0x1;
            break;

        case PAD_FUART_RTS:
            *padmux=MUX_UART1;
            *pad_mode=0x2;
            break;

        case PAD_GPIO6:
            *padmux=MUX_UART1;
            *pad_mode=0x3;
            break;

        default:
            ret = -1;
            break;
    }

    return ret;
}
