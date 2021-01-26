/*
* timer.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: edie.chen <edie.chen@sigmastar.com.tw>
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
#include "asm/arch/mach/ms_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"

DECLARE_GLOBAL_DATA_PTR;

#define BASE_REG_TIMER      MS_BASE_REG_TIMER0_PA

#define TIMER_CTRL_ADDR     GET_REG_ADDR(BASE_REG_TIMER, 0x00)
#define TIMER_HIT_ADDR      GET_REG_ADDR(BASE_REG_TIMER, 0x01)
#define TIMER_MAX_L_ADDR    GET_REG_ADDR(BASE_REG_TIMER, 0x02)
#define TIMER_MAX_H_ADDR    GET_REG_ADDR(BASE_REG_TIMER, 0x03)
#define TIMER_CAPCNT_L_ADDR GET_REG_ADDR(BASE_REG_TIMER, 0x04)
#define TIMER_CAPCNT_H_ADDR GET_REG_ADDR(BASE_REG_TIMER, 0x05)

extern unsigned int get_ms_ticks(void);
extern unsigned int get_full_ms_ticks(void);


static U32 get_timer_ticks(void)
{
    // 1 tick = 1/OSC1 sec
    return ((REG(TIMER_CAPCNT_L_ADDR)& 0xFFFF) | ((REG(TIMER_CAPCNT_H_ADDR) & 0xFFFF)<<16));
}


static void loop_delay_timer(U32 u32MicroSeconds)
{
	U32 c0;
	U32 period;
	U32 count=0;

    period=u32MicroSeconds*(gd->xtal_clk/1000000);
    
	c0=get_timer_ticks();

	while(TRUE)
	{
		U32 c1=get_timer_ticks();
		if(c1<c0)
		{
			count+=(c1+(0xFFFFFFFF-c0));
		}
		else
		{
			count+=(c1-c0);
		}
		c0=c1;

		if(count>=period)
		{
			break;
		}
	}
}

/*------------------------------------------------------------------------------
    Function Code
-------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    Function: timer_init

    Description:
        Do nothing here.
        But, u-boot's other functions will call this function. For compiling
        correctly, I make a dummy function here.
    Input:
        None.
    Output:
        None.
    Return:
        None.
    Remark:
        None.
-------------------------------------------------------------------------------*/
int timer_init (void)
{
	REG(TIMER_CTRL_ADDR)=0x0000;

	REG(TIMER_MAX_L_ADDR)=0xFFFF;
	REG(TIMER_MAX_H_ADDR)=0xFFFF;


	REG(TIMER_CTRL_ADDR)=0x0001;

	return 0;
}

/*------------------------------------------------------------------------------
    Function: reset_timer

    Description:
        Do nothing here.
        But, u-boot's other functions will call this function. For compiling
        correctly, I make a dummy function here.
    Input:
        None.
    Output:
        None.
    Return:
        None.
    Remark:
        None.
-------------------------------------------------------------------------------*/
void reset_timer (void)
{
}

/*------------------------------------------------------------------------------
    Function: get_timer

    Description:

    Input:
        base - not used here
    Output:
        None.
    Return:
        None.
    Remark:
        None.
-------------------------------------------------------------------------------*/
extern unsigned int get_ms_ticks(void);
static ulong offset_ms_ticks=0;
static ulong current_ms_ticks=0;
ulong get_timer (ulong base)
{
	ulong t1=get_ms_ticks();
	ulong t=0;

	if(current_ms_ticks>t1)
	{
		offset_ms_ticks+=get_full_ms_ticks();
	}
	current_ms_ticks=t1;
	t=(offset_ms_ticks+current_ms_ticks);

	return (base > t)? 0 : (t-base);
}

/*------------------------------------------------------------------------------
    Function: set_timer

    Description:
        Do nothing here.
        But, u-boot's other functions will call this function. For compiling
        correctly, I make a dummy function here.
    Input:
        t - not used here
    Output:
        None.
    Return:
        None.
    Remark:
        None.
-------------------------------------------------------------------------------*/
void set_timer (ulong t)
{
}

/*------------------------------------------------------------------------------
    Function: udelay

    Description:
        delay according to the input time
    Input:
        usec - delay time
    Output:
        None.
    Return:
        None.
    Remark:
        None.
-------------------------------------------------------------------------------*/
void __udelay (unsigned long usec)
{
	loop_delay_timer(usec);
}

/*------------------------------------------------------------------------------
    Function: reset_timer_masked

    Description:
        Do nothing here.
        But, u-boot's other functions will call this function. For compiling
        correctly, I make a dummy function here.
    Input:
        None.
    Output:
        None.
    Return:
        None.
    Remark:
        None.
-------------------------------------------------------------------------------*/
void reset_timer_masked (void)
{
}

/*------------------------------------------------------------------------------
    Function: get_timer_masked

    Description:
        Do nothing here.
        But, u-boot's other functions will call this function. For compiling
        correctly, I make a dummy function here.
    Input:
        None.
    Output:
        None.
    Return:
        timer masked
    Remark:
        None.
-------------------------------------------------------------------------------*/
ulong get_timer_masked (void)
{
	return 0;
}

/*------------------------------------------------------------------------------
    Function: udelay_masked

    Description:
        Do nothing here.
        But, u-boot's other functions will call this function. For compiling
        correctly, I make a dummy function here.
    Input:
        usec - not used
    Output:
        None.
    Return:
        None.
    Remark:
        None.
-------------------------------------------------------------------------------*/
void udelay_masked (unsigned long usec)
{
}

/*------------------------------------------------------------------------------
    Function: get_ticks

    Description:
        Do nothing here.
        But, u-boot's other functions will call this function. For compiling
        correctly, I make a dummy function here.
    Input:
        None.
    Output:
        None.
    Return:
        tick
    Remark:
        None.
-------------------------------------------------------------------------------*/
unsigned long long get_ticks(void)
{
	return 0;
}

/*------------------------------------------------------------------------------
    Function: get_tbclk

    Description:
        Do nothing here.
        But, u-boot's other functions will call this function. For compiling
        correctly, I make a dummy function here.
    Input:
        None.
    Output:
        None.
    Return:
        tbclk
    Remark:
        None.
-------------------------------------------------------------------------------*/
ulong get_tbclk (void)
{
	return 0;
}

unsigned int get_ms_ticks(void)
{
	unsigned long c0=get_timer_ticks();
    return (c0/(ulong)(gd->xtal_clk/1000));
}

unsigned int get_full_ms_ticks(void)
{
	return 0xFFFFFFFFUL/((ulong)(gd->xtal_clk/1000));

}

