/*
* hal_card_platform.h- Sigmastar
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
 * FileName hal_card_platform.h
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This file is the header file of hal_card_platform_XX.c.
 *     Every project has the same header file.
 *
 ***************************************************************************************************************/

#ifndef __HAL_CARD_PLATFORM_H
#define __HAL_CARD_PLATFORM_H

#include "hal_card_regs.h"

typedef enum
{
    EV_PULLDOWN,
    EV_PULLUP,

} PinPullEmType;

typedef enum
{
    EV_GPIO_OPT1 = 0,
    EV_GPIO_OPT2 = 1,
    EV_GPIO_OPT3 = 2,
    EV_GPIO_OPT4 = 3,
    EV_GPIO_OPT5 = 4,

} GPIOOptEmType;



void Hal_CARD_IPOnceSetting(IpOrder eIP);

// PAD Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void Hal_CARD_InitPADPin(IpOrder eIP, PadOrder ePAD);
BOOL_T Hal_CARD_GetPadInfoCdzPad(IpOrder eIP, U32_T *nPadNum);
BOOL_T Hal_CARD_GetPadInfoPowerPad(IpOrder eIP, U32_T *nPadNum);
void Hal_CARD_ConfigSdPad(IpOrder eIP, PadOrder ePAD);
void Hal_CARD_ConfigPowerPad(IpOrder eIP, U16_T nPadNum);
void Hal_CARD_PullPADPin(IpOrder eIP, PadOrder ePAD, PinPullEmType ePinPull);

// Clock Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void Hal_CARD_SetClock(IpOrder eIP, U32_T u32ClkFromIPSet);
U32_T Hal_CARD_FindClockSetting(IpOrder eIP, U32_T u32ReffClk);

// Power and Voltage Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void Hal_CARD_PowerOn(IpOrder eIP, U16_T u16DelayMs);
void Hal_CARD_PowerOff(IpOrder eIP, U16_T u16DelayMs);

// Card Detect and GPIO Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void Hal_CARD_ConfigCdzPad(IpOrder eIP, U16_T nPadNum); //Hal_CARD_InitGPIO
BOOL_T Hal_CARD_GetCdzState(IpOrder eIP); //Hal_CARD_GetGPIOState

// MIU Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
U32_T Hal_CARD_TransMIUAddr(U32_T u32Addr, U8_T* pu8MIUSel);

#endif //End of __HAL_CARD_PLATFORM_H
