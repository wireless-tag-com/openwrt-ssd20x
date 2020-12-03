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

void ms_uart_select_pad( u8 select, u8 padmux, u8 pad_mode)
{
 /*   u8 select=0;
    
    if(p->line == 0)
        select=UART_PIU_UART0; //PIU UART0
    else if(p->line == 1)
        select=UART_PIU_UART1; //PIU UART1
    else if(p->line == 2)
        select=UART_PIU_UART2; //PIU UART2
    else if(p->line == 3)
        select=UART_PIU_FUART; //PIU FUART
    else
    {
        UART_ERR("[%s] port line %d is not supported\n", __func__, p->line);
        return;
    }
*/
//printk("ms_select_pad padmux:%d: pad_mode:%d: select:%d \n", padmux, pad_mode, select);

    switch(padmux)
    {
        case DIG_MUX_SEL0:
            OUTREGMSK16(REG_UART_SEL, select << 0, 0xF << 0);           //reg_uart_sel0
            break;
        case DIG_MUX_SEL1:
            OUTREGMSK16(REG_UART_MODE, pad_mode << 3, 0x7 << 3);
            OUTREGMSK16(REG_UART_SEL, select << 4, 0xF << 4);           //reg_uart_sel1
            break;
        case DIG_MUX_SEL2:
            OUTREGMSK16(REG_UART_MODE, pad_mode << 6, 0x7 << 6);
            OUTREGMSK16(REG_UART_SEL, select << 8, 0xF << 8);           //reg_uart_sel2
            break;
        case DIG_MUX_SEL3:
            OUTREGMSK16(REG_UART_MODE, pad_mode << 0, 0x3 << 0);
            OUTREGMSK16(REG_UART_SEL, select << 12, 0xF << 12);         //reg_uart_sel3
            break;
        case DIG_MUX_SEL4:
            OUTREGMSK16(REG_FUART_MODE, pad_mode << 0, 0x3 << 0);
            OUTREGMSK16(REG_FUART_SEL, select << 0, 0xF << 0);          //reg_uart_sel4
            break;
        default:
            //UART_ERR("[%s] Padmux %d not defined\n", __func__, padmux);
            break;
    }
}
int ms_uart_get_padmux(int tx_pad, u8 *padmux, u8 *pad_mode)
{
    int ret = 0;

        switch(tx_pad)
        {
            case PAD_UART0_TX:
                *padmux=DIG_MUX_SEL1;
                *pad_mode=0x1;
                break;

            case PAD_I2C2_SDA:
                *padmux=DIG_MUX_SEL1;
                *pad_mode=0x2;
                break;
                
            case PAD_SNR1_GPIO2:
                *padmux=DIG_MUX_SEL1;
                *pad_mode=0x3;
                break;

            case PAD_UART1_TX:
                *padmux=DIG_MUX_SEL2;
                *pad_mode=0x1;
                break;

            case PAD_SPI0_CK:
                *padmux=DIG_MUX_SEL2;
                *pad_mode=0x2;
                break;

            case PAD_SNR3_D4:
                *padmux=DIG_MUX_SEL2;
                *pad_mode=0x3;
                break;

            case PAD_HSYNC_OUT:
                *padmux=DIG_MUX_SEL2;
                *pad_mode=0x4;
                break;

            case PAD_UART2_TX:
                *padmux=DIG_MUX_SEL3;
                *pad_mode=0x1;
                break;

            case PAD_TTL_HSYNC:
                *padmux=DIG_MUX_SEL3;
                *pad_mode=0x2;
                break;

            case PAD_FUART_RTS:
                *padmux=DIG_MUX_SEL3;
                *pad_mode=0x3;
                break;

            case PAD_SPI0_DO:
                *padmux=DIG_MUX_SEL3;
                *pad_mode=0x4;
                break;

            case PAD_FUART_TX:
                *padmux=DIG_MUX_SEL4;
                *pad_mode=0x1;
                break;

            case PAD_NAND_DA4:
                *padmux=DIG_MUX_SEL4;
                *pad_mode=0x2;
                break;

            case PAD_SNR3_D0:
                *padmux=DIG_MUX_SEL4;
                *pad_mode=0x3;
                break;

            default:
                //UART_ERR("[%s] Use undefined pad number %d\n", __func__, tx_pad);
                ret = -EINVAL;
                break;
        }

    return ret;
}
