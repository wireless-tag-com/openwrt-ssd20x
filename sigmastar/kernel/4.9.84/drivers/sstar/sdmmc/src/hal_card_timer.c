/*
* hal_card_timer.c- Sigmastar
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
/***************************************************************************************************************
 *
 * FileName hal_card_timer.c
 *     @author jeremy.wang (2012/01/13)
 * Desc:
 *     All of timer behavior will run here.
 *     The goal is that we don't need to change HAL Level code (But its h file code)
 *
 *     The limitation were listed as below:
 *     (1) This c file belongs to HAL level.
 *     (2) Its h file is included by driver API level, not driver flow process.
 *     (2) Delay and GetTimerTick function belong to here.
 *     (3) Because timer may belong to OS design or HW timer, so we could use OS define option to separate them.
 *
 ***************************************************************************************************************/

#include "../inc/hal_card_timer.h"
#if (D_OS == D_OS__LINUX)
#include "linux/sched.h"
#endif

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_Timer_mDelay
 *     @author jeremy.wang (2013/7/19)
 * Desc: Run millisecond delay
 *
 * @param u32_msec : Millisecond for Delay
 *
 * @return U32_T  : Delay time
 ----------------------------------------------------------------------------------------------------------*/
U32_T Hal_Timer_mDelay(U32_T u32_msec)
{

//###########################################################################################################
#if (D_OS == D_OS__LINUX) || (D_OS == D_OS__UBOOT)
//###########################################################################################################
    mdelay(u32_msec);
    return u32_msec;
//###########################################################################################################
#endif
    return 0;

}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_Timer_uDelay
 *     @author jeremy.wang (2013/7/19)
 * Desc: Run micro-second delay
 *
 * @param u32_usec : Micro-second for Delay
 *
 * @return U32_T  : Delay time
 ----------------------------------------------------------------------------------------------------------*/
U32_T Hal_Timer_uDelay(U32_T u32_usec)
{
//###########################################################################################################
#if (D_OS == D_OS__LINUX) || (D_OS == D_OS__UBOOT)
//###########################################################################################################
    udelay(u32_usec);
    return u32_usec;
//###########################################################################################################
#endif

    return 0;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_Timer_mSleep
 *     @author jeremy.wang (2015/1/15)
 * Desc: Run millisecond sleep
 *
 * @param u32_msec : Millisecond for Sleep
 *
 * @return U32_T  : Delay time
 ----------------------------------------------------------------------------------------------------------*/
U32_T Hal_Timer_mSleep(U32_T u32_msec)
{
//###########################################################################################################
#if (D_OS == D_OS__LINUX)
//###########################################################################################################
#if (HZ) <= 100
    if (u32_msec < 1000 / HZ)
    {
        cond_resched();
        mdelay(u32_msec);
    }
    else
    {
        msleep(u32_msec);
    }
#else
    msleep(u32_msec);
#endif
    return u32_msec;
//###########################################################################################################
#endif
    return 0;

}

