/*
* hal_card_timer.h- Sigmastar
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
#ifndef __HAL_CARD_TIMER_H
#define __HAL_CARD_TIMER_H

#include "hal_card_regs.h"

//###########################################################################################################
#if (D_OS == D_OS__LINUX)
//###########################################################################################################
#include <linux/delay.h>
#include <linux/time.h>
//###########################################################################################################
#endif

U32_T Hal_Timer_mDelay(U32_T u32_msec);
U32_T Hal_Timer_uDelay(U32_T u32_usec);
U32_T Hal_Timer_mSleep(U32_T u32_msec);

#endif //End of __HAL_CARD_TIMER_H
