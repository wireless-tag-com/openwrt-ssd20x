/*
* ms_uart.h- Sigmastar
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

#ifndef _MS_UART_H_
#define _MS_UART_H_

#define CONFIG_MS_SUPPORT_UART2
#define CONFIG_MS_SUPPORT_EXT_PADMUX // use external padmux driver

#define MUX_PM_UART 0
#define MUX_FUART   1
#define MUX_UART0   2
#define MUX_UART1   3
#define MUX_UART2   4

void ms_uart_select_pad( u8 select, u8 padmux, u8 pad_mode);
int ms_uart_get_padmux(int tx_pad, u8 *padmux, u8 *pad_mode);
#endif
