/*
* ms_serial.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
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

DECLARE_GLOBAL_DATA_PTR;

#define UART_BAUDRATE_DEFAULT  CONFIG_BAUDRATE
#if CONFIG_ARCH_CEDRIC
#define UART_ENABLE_REG (GET_REG_ADDR(MS_BASE_REG_CHIPTOP_PA,0x64))
#endif
#if CONFIG_ARCH_CHICAGO
#define UART_ENABLE_REG (GET_REG_ADDR(MS_BASE_REG_PADTOP_PA,0x03))
#endif

#define UART_REG8(_x_)  ((U8 volatile *)(UART_BASE))[((_x_) * 4) - ((_x_) & 1)]


#define UART_BASE MS_BASE_REG_UART0_PA

/*------------------------------------------------------------------------------
    Function: serial_init

    Description:
        Init UART
    Input:
        None.
    Output:
        None.
    Return:
        already return 0
    Remark:
        None.
-------------------------------------------------------------------------------*/
int ms_serial_init(void)
{


	//i.   Set "reg_mcr_loopback";
	UART_REG8(UART_MCR) |= 0x10;

	//ii.   Poll "reg_usr_busy" till 0;
	while(UART_REG8(UART_USR) & 0x01)
	{
		UART_REG8(UART_IIR)=(UART_REG8(UART_IIR)|(UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT));
	}

    UART_REG8(UART_IER) = 0x00;

    // Reset receiver and transmiter
    UART_REG8(UART_FCR) = UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_1;

    // Set 8 bit char, 1 stop bit, no parity
    UART_REG8(UART_LCR) = UART_LCR_WLEN8 & ~(UART_LCR_STOP2 | UART_LCR_PARITY);

    //i.   Set "reg_mcr_loopback back;
	UART_REG8(UART_MCR) &= ~0x10;

    serial_setbrg();

    return (0);
}


void ms_putc(const char c)
{
    while (!(UART_REG8(UART_LSR) & UART_LSR_THRE));

    UART_REG8(UART_TX) = c;

}

/*------------------------------------------------------------------------------
    Function: serial_putc

    Description:
        Write a character to serial port
    Input:
        c - the character which will be written
    Output:
        None.
    Return:
        None.
    Remark:
        None.
-------------------------------------------------------------------------------*/
void ms_serial_putc (const char c)
{
	if (c == '\n')
		ms_putc('\r');

	ms_putc(c);
}

/*------------------------------------------------------------------------------
    Function: serial_putc

    Description:
        Read a character from UART
    Input: (The arguments were used by caller to input data.)
        None.
    Output: (The arguments were used by caller to receive data.)
        None.
    Return:
        c - the character got from UART
    Remark:
        None.
-------------------------------------------------------------------------------*/
int ms_serial_getc (void)
{
    char c;
//
    while(!(UART_REG8(UART_LSR) & UART_LSR_DR));

    c=(char) ( UART_REG8(UART_TX) & 0xff);

    return c;
}

/*------------------------------------------------------------------------------
    Function: serial_tstc

    Description:
        Check if UART's LSR_DR bit
    Input:
        None.
    Output:
        None.
    Return:
        value of UART's LSR_DR bit
    Remark:
        None.
-------------------------------------------------------------------------------*/
int ms_serial_tstc (void)
{

    return  ((UART_REG8(UART_LSR) & UART_LSR_DR));
}

/*------------------------------------------------------------------------------
    Function: serial_tstc

    Description:
        set UART's baud rate
    Input:
        None.
    Output:
        None.
    Return:
        None.
    Remark:
        None.
-------------------------------------------------------------------------------*/
void ms_serial_setbrg (void)
{
//    // set baud_rate
//
#if CONFIG_ARCH_CHICAGO
    U16 DLR = ((26000000+8*UART_BAUDRATE_DEFAULT)/ (16 * UART_BAUDRATE_DEFAULT));
#elif CONFIG_ARCH_CEDRIC
    U16 DLR = ((123000000+8*UART_BAUDRATE_DEFAULT)/ (16 * UART_BAUDRATE_DEFAULT));
#else
#error unknown UART_CLK
#endif
    //i.   Set "reg_mcr_loopback";
    UART_REG8(UART_MCR) |= 0x10;

    //  Clear FIFO Buffer
    UART_REG8(UART_FCR) |= 0x07;

    //ii.   Poll "reg_usr_busy" till 0;
    while(UART_REG8(UART_USR) & 0x01)
    {
    	UART_REG8(UART_IIR)=(UART_REG8(UART_IIR)|(UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT));
    }
//
    UART_REG8(UART_LCR) |= UART_LCR_DLAB;
    UART_REG8(UART_DLL) = (DLR & 0xFF);
    UART_REG8(UART_DLM) = ((DLR >> 8) & 0xFF);
    UART_REG8(UART_LCR) &= ~(UART_LCR_DLAB);
//
    UART_REG8(UART_MCR) &= ~0x10;
}

/*------------------------------------------------------------------------------
    Function: serial_puts

    Description:
        write a string to UART
    Input:
        s - The string which will be written.
    Output:
        None.
    Return:
        None.
    Remark:
        None.
-------------------------------------------------------------------------------*/
//void ms_serial_puts (const char *s)
//{
//    while (*s)
//    {
//		serial_putc (*s++);
//    }
//}


static struct serial_device ms_serial_drv = {
	.name	= "ms_uart",
	.start	= ms_serial_init,
	.stop	= NULL,
	.setbrg	= ms_serial_setbrg,
	.putc	= ms_serial_putc,
	.puts	= default_serial_puts,
	.getc	= ms_serial_getc,
	.tstc	= ms_serial_tstc,
};

void ms_serial_initialize(void)
{
	serial_register(&ms_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &ms_serial_drv;
}

