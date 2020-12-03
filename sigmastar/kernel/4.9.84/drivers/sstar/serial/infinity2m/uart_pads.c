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
#include "ms_platform.h"

#define REG_UART_MODE          0x1F203C0C
#define REG_UART_SEL           0x1F203D4C
#define REG_UART_SEL4          0x1F203D50
#define REG_FORCE_RX_DISABLE   0x1F203D5C


void ms_uart_select_pad( u8 select, u8 padmux, u8 pad_mode)
{
    switch(padmux)
    {
        case MUX_PM_UART:
            OUTREGMSK16(REG_UART_SEL, select << 0, 0xF << 0);     //reg_uart_sel0[3:0]
            break;
        case MUX_FUART:
            OUTREGMSK16(REG_UART_MODE, pad_mode << 0, 0x7 << 0);  //reg_uart_mode[2:0]
            OUTREGMSK16(REG_UART_SEL, select << 4, 0xF << 4);     //reg_uart_sel0[7:4]
            break;
        case MUX_UART0:
            OUTREGMSK16(REG_UART_MODE, pad_mode << 4, 0x7 << 4);  //reg_uart_mode[6:4]
            OUTREGMSK16(REG_UART_SEL, select << 8, 0xF << 8);     //reg_uart_sel0[11:8]
            break;
        case MUX_UART1:
            OUTREGMSK16(REG_UART_MODE, pad_mode << 8, 0x7 << 8);  //reg_uart_mode[10:8]
            OUTREGMSK16(REG_UART_SEL, select << 12, 0xF << 12);   //reg_uart_sel0[15:12]
            break;
#ifdef CONFIG_MS_SUPPORT_UART2
        case MUX_UART2:
            OUTREGMSK16(REG_UART_MODE, pad_mode << 12, 0x7 << 12);//reg_uart_mode[14:12]
            OUTREGMSK16(REG_UART_SEL4, select << 0, 0xF << 0);    //reg_uart_sel4[3:0]
            break;
#endif
        default:
            // UART_ERR("[%s] Padmux %d not defined\n", __func__, padmux);
            break;
    }
}
int ms_uart_get_padmux(int tx_pad, u8 *padmux, u8 *pad_mode)
{
    int ret = 0;

    switch(tx_pad)
    {
        case PAD_FUART_CTS:
            *padmux=MUX_FUART;
            *pad_mode=0x1;
            break;

        case PAD_FUART_RX:
            *padmux=MUX_FUART;
            *pad_mode=0x2;
            break;

        case PAD_TTL1:
            *padmux=MUX_FUART;
            *pad_mode=0x3;
            break;

        case PAD_TTL21:
            *padmux=MUX_FUART;
            *pad_mode=0x4;
            break;

        case PAD_GPIO1:
            *padmux=MUX_FUART;
            *pad_mode=0x5;
            break;

        case PAD_GPIO5:
            *padmux=MUX_FUART;
            *pad_mode=0x6;
            break;

        case PAD_SD_D0:
            *padmux=MUX_FUART;
            *pad_mode=0x7;
            break;

        case PAD_UART0_TX:
            *padmux=MUX_UART0;
            *pad_mode=0x1;
            break;

        case PAD_FUART_RTS:
            *padmux=MUX_UART0;
            *pad_mode=0x2;
            break;

        case PAD_TTL13:
            *padmux=MUX_UART0;
            *pad_mode=0x3;
            break;

        case PAD_GPIO9:
            *padmux=MUX_UART0;
            *pad_mode=0x4;
            break;

        case PAD_UART1_TX:
            *padmux=MUX_UART1;
            *pad_mode=0x1;
            break;

        case PAD_TTL15:
            *padmux=MUX_UART1;
            *pad_mode=0x2;
            break;

        case PAD_GPIO14:
            *padmux=MUX_UART1;
            *pad_mode=0x3;
            break;

        case PAD_GPIO11:
            *padmux=MUX_UART1;
            *pad_mode=0x4;
            break;

        case PAD_FUART_TX:
            *padmux=MUX_UART2;
            *pad_mode=0x1;
            break;

        case PAD_GPIO8:
            *padmux=MUX_UART2;
            *pad_mode=0x2;
            break;

        case PAD_VSYNC_OUT:
            *padmux=MUX_UART2;
            *pad_mode=0x3;
            break;

        case PAD_SD_D2:
            *padmux=MUX_UART2;
            *pad_mode=0x4;
            break;

        default:
            ret = -1;
            break;
    }

    return ret;
}
