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
#define UART_NA          0
#define UART_PIU_UART1   1
#define UART_VD_MHEG5    2
#define UART_PIU_UART2   3
#define UART_PIU_UART0   4
#define UART_PIU_FUART   7
#define DIG_MUX_SEL0    0
#define DIG_MUX_SEL1    1
#define DIG_MUX_SEL2    2
#define DIG_MUX_SEL3    3
#define DIG_MUX_SEL4    4

#define REG_FUART_MODE         0x1F204C14 /*0x1026, h05*/
#define REG_UART_MODE          0x1F204C04 /*0x1026, h01*/
#define REG_FUART_SEL          0x1F203D50 /*0x101E, h54*/
#define REG_UART_SEL           0x1F203D4C /*0x101E, h53*/
#define REG_FORCE_RX_DISABLE   0x1F203D5C /*0x101E, h57*/
#define REG_UART2_CLK                0x1F201650 /*0x100B, h14*/
#define REG_FUART_CLK                0x1F20165c /*0x100B, h17*/

void ms_uart_select_pad( u8 select, u8 padmux, u8 pad_mode);
int ms_uart_get_padmux(int tx_pad, u8 *padmux, u8 *pad_mode);
#endif
