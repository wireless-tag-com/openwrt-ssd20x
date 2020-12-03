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

#define REG_UART_MODE          0x1F2079B4 
#define REG_FUART_MODE         0x1F2079B8 
#define REG_UART_SEL           0x1F203D4C 
#define REG_UART_SEL4          0x1F203D50 
//#define REG_FORCE_RX_DISABLE   0x1F203D5C


void ms_uart_select_pad( u8 select, u8 padmux, u8 pad_mode)
{
    switch(padmux)
    {
        case MUX_PM_UART:
            OUTREGMSK16(REG_UART_SEL, select << 0, 0xF << 0);     //reg_uart_sel0[3:0]
            break;
        case MUX_FUART:
            OUTREGMSK16(REG_FUART_MODE, pad_mode << 4, 0x7 << 4);  //reg_uart_mode[2:0]
            OUTREGMSK16(REG_UART_SEL, select << 4, 0xF << 4);     //reg_uart_sel0[7:4]
            break;
        case MUX_UART0:
            OUTREGMSK16(REG_UART_MODE, pad_mode << 0, 0x7 << 0);  //reg_uart_mode[6:4]
            OUTREGMSK16(REG_UART_SEL, select << 8, 0xF << 8);     //reg_uart_sel0[11:8]
            break;
        case MUX_UART1:
            OUTREGMSK16(REG_UART_MODE, pad_mode << 4, 0x7 << 4);  //reg_uart_mode[10:8]
            OUTREGMSK16(REG_UART_SEL, select << 12, 0xF << 12);   //reg_uart_sel0[15:12]
            break;
#ifdef CONFIG_MS_SUPPORT_UART2
        case MUX_UART2:
            OUTREGMSK16(REG_UART_MODE, pad_mode << 8, 0x7 << 8);//reg_uart_mode[14:12]
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
	unsigned short TempU16 = 0;

    switch(tx_pad)
    {
    	//reg_uart0_mode 1~4
        case PAD_PM_UART_RX:
            *padmux=MUX_UART0;
            *pad_mode=0x1;
            break;

        case PAD_ETH_LED0:
            *padmux=MUX_UART0;
            *pad_mode=0x2;
            break;

        case PAD_I2S0_BCK:
            *padmux=MUX_UART0;
            *pad_mode=0x3;
            break;

		case PAD_GPIO14:
            *padmux=MUX_UART0;
            *pad_mode=0x4;
            break;		

		//reg_fuart_mode 1~7
        case PAD_FUART_RX:
			//reg_ckg_fuart0_synth_in
			TempU16 = ( *(volatile	unsigned short *)(0xFD000000 +(0x1038 * 0x0200) +(0x34 << 2) ) );
			TempU16 &= 0xFF00; //172.8m   432m		
			( *(volatile  unsigned short *)(0xFD000000 +(0x1038 * 0x0200) +(0x34 << 2) ) ) = TempU16; 

			TempU16 = ( *(volatile	unsigned short *)(0xFD000000 +(0x103C * 0x0200) +(0x22 << 2) ) );
			TempU16 &= 0xFFF0;//PAD_FUART_CTS		
			( *(volatile  unsigned short *)(0xFD000000 +(0x103C * 0x0200) +(0x22 << 2) ) ) = TempU16; 
			
			TempU16 = ( *(volatile	unsigned short *)(0xFD000000 +(0x103C * 0x0200) +(0x23 << 2) ) );
			TempU16 &= 0xFFF0; //PAD_FUART_RTS	
			( *(volatile  unsigned short *)(0xFD000000 +(0x103C * 0x0200) +(0x23 << 2) ) ) = TempU16;

			TempU16 = ( *(volatile  unsigned short *)(0xFD000000 +(0x103C * 0x0200) +(0x24 << 2) ) );
			TempU16 &= 0xFFF0;//PAD_FUART_CTS		
			( *(volatile  unsigned short *)(0xFD000000 +(0x103C * 0x0200) +(0x24 << 2) ) ) = TempU16; 
			
			TempU16 = ( *(volatile  unsigned short *)(0xFD000000 +(0x103C * 0x0200) +(0x25 << 2) ) );
			TempU16 &= 0xFFF0; //PAD_FUART_RTS	
			( *(volatile  unsigned short *)(0xFD000000 +(0x103C * 0x0200) +(0x25 << 2) ) ) = TempU16;
			
            *padmux=MUX_FUART;
            *pad_mode=0x1;
            break;

        /*case PAD_GPIO0:
            *padmux=MUX_FUART;
            *pad_mode=0x2;
            break;*/

		case PAD_PM_GPIO2:			
			//reg_ckg_fuart0_synth_in
			TempU16 = ( *(volatile	unsigned short *)(0xFD000000 +(0x1038 * 0x0200) +(0x34 << 2) ) );
			TempU16 &= 0xFF00; //172.8m   432m		
			( *(volatile  unsigned short *)(0xFD000000 +(0x1038 * 0x0200) +(0x34 << 2) ) ) = TempU16; 
		
			//PADPMTOP setting
			TempU16 = ( *(volatile  unsigned short *)(0xFD000000 +(0x003F * 0x0200) +(0x55 << 2) ) );
			TempU16 |= 0x80; // reg_pm_pad_ext_mode [7]=1
			( *(volatile  unsigned short *)(0xFD000000 +(0x003F * 0x0200) +(0x55 << 2) ) ) = TempU16;
			
			*padmux=MUX_FUART;
			*pad_mode=0x3;
			break;
			
        case PAD_SD1_D1:			
			//reg_ckg_fuart0_synth_in
			TempU16 = ( *(volatile	unsigned short *)(0xFD000000 +(0x1038 * 0x0200) +(0x34 << 2) ) );
			TempU16 &= 0xFF00; //172.8m   432m		
			( *(volatile  unsigned short *)(0xFD000000 +(0x1038 * 0x0200) +(0x34 << 2) ) ) = TempU16; 
			
            *padmux=MUX_FUART;
            *pad_mode=0x4;
            break;
			
        /*case PAD_FUART_RX:
            *padmux=MUX_FUART;
            *pad_mode=0x5;
            break;*/
			
        /*case PAD_PM_GPIO2:
            *padmux=MUX_FUART;
            *pad_mode=0x6;
            break;*/
			
        case PAD_PM_SPI_CZ:			
			//reg_ckg_fuart0_synth_in
			TempU16 = ( *(volatile	unsigned short *)(0xFD000000 +(0x1038 * 0x0200) +(0x34 << 2) ) );
			TempU16 &= 0xFF00; //172.8m   432m		
			( *(volatile  unsigned short *)(0xFD000000 +(0x1038 * 0x0200) +(0x34 << 2) ) ) = TempU16; 
			
            *padmux=MUX_FUART;
            *pad_mode=0x7;
            break;			

		//reg_uart1_mode 1~6
        case PAD_GPIO0:
            *padmux=MUX_UART1;
            *pad_mode=0x1;
            break;

        case PAD_I2S0_DI:
            *padmux=MUX_UART1;
            *pad_mode=0x2;
            break;

        /*case PAD_ETH_LED0:
            *padmux=MUX_UART1;
            *pad_mode=0x3;
            break;*/

        case PAD_GPIO4:
            *padmux=MUX_UART1;
            *pad_mode=0x4;
            break;
			
		case PAD_FUART_CTS:	
			
			TempU16 = ( *(volatile  unsigned short *)(0xFD000000 +(0x103C * 0x0200) +(0x24 << 2) ) );
			TempU16 &= 0xFFF0;//PAD_FUART_CTS		
			( *(volatile  unsigned short *)(0xFD000000 +(0x103C * 0x0200) +(0x24 << 2) ) ) = TempU16; 
			
			TempU16 = ( *(volatile  unsigned short *)(0xFD000000 +(0x103C * 0x0200) +(0x25 << 2) ) );
			TempU16 &= 0xFFF0; //PAD_FUART_RTS	
			( *(volatile  unsigned short *)(0xFD000000 +(0x103C * 0x0200) +(0x25 << 2) ) ) = TempU16;
			
			*padmux=MUX_UART1;
			*pad_mode=0x5;
			break;
			
		case PAD_GPIO8:
			*padmux=MUX_UART1;
			*pad_mode=0x6;
			break;

        default:
            ret = -1;
            break;
    }

    return ret;
}
