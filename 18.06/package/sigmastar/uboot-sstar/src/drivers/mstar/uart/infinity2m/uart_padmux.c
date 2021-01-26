/*
* uart_padmux.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: XXXX <XXXX@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/
/*-----------------------------------------------------------------------------
    Include Files
------------------------------------------------------------------------------*/
#include <common.h>
#include <command.h>
#include "asm/arch/mach/ms_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
#include "ms_serial.h"
#include <linux/compiler.h>
#include <serial.h>
#define MS_BASE_REG_UART1_PA           GET_BASE_ADDR_BY_BANK(MS_BASE_REG_RIU_PA, 0x110900)
#define MS_BASE_REG_FUART_PA           GET_BASE_ADDR_BY_BANK(MS_BASE_REG_RIU_PA, 0x110200)

#if 0
#define REG_UART2_CLK                0x1F201650 /*0x100B, h14*/
#define REG_FUART_CLK                0x1F20165c /*0x100B, h17*/
#define REG_FUART_MODE         0x1F204C14 /*0x1026, h05*/
#define REG_UART_MODE          0x1F204C04 /*0x1026, h01*/
#define REG_FUART_SEL          0x1F203D50 /*0x101E, h54*/
#define REG_UART_SEL           0x1F203D4C /*0x101E, h53*/

#define UART_PIU_UART1   1
#define UART_VD_MHEG5    2
#define UART_PIU_UART2   3
#define UART_PIU_UART0   4
#define UART_PIU_FUART   7

//infinity2
/* reg_uart_sel2 <--> reg_ThirdUARTMode*/
#define PAD_HSYNC_OUT (0x4)
#define PAD_SNR3_D4 (0x3)
#define PAD_SPI0_CK (0x2)
#define PAD_UART1_TX (0x1)

/* reg_uart_sel3 <--> reg_ForthUARTMode */
#define PAD_UART2_TX (0x1)
#define PAD_TTL_HSYNC (0x2)
#define PAD_FUART_RTS (0x3)
#define PAD_SPI0_DO (0x4)

/* reg_uart_sel4 <--> reg_FastUART_RTX_Mode  */
#define PAD_FUART_TX (0x1)
#define PAD_NAND_DA4 (0x2)
#define PAD_SNR3_DO (0x3)

#define CONFIG_UART1_PAD PAD_HSYNC_OUT
/* #define CONFIG_UART1_PAD PAD_SNR3_D4 */
/* #define CONFIG_UART1_PAD PAD_SPI0_CK */
/* #define CONFIG_UART1_PAD PAD_UART1_TX */
#define CONFIG_UART2_PAD PAD_FUART_RTS
/* #define CONFIG_UART2_PAD PAD_UART2_TX */
/* #define CONFIG_UART2_PAD PAD_TTL_HSYNC  */
/* #define CONFIG_UART2_PAD PAD_SPI0_DO */
#define CONFIG_UART3_PAD PAD_FUART_TX
/* #define CONFIG_UART3_PAD PAD_NAND_DA4 */
/* #define CONFIG_UART3_PAD PAD_SNR3_DO  */
#endif
extern U32 uart_multi_base;

U32 ms_uart_padmux(U8 u8_Port)
{
    /*todo*/
	printf("TODO: uart padmux\r\n");
    return 0;
}
