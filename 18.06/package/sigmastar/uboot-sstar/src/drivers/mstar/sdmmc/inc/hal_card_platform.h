/*
* hal_card_platform.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: truman.yang <truman.yang@sigmastar.com.tw>
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

/***************************************************************************************************************
 *
 * FileName hal_card_platform.h
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 * 	   This file is the header file of hal_card_platform_XX.c.
 *	   Every project has the same header file.
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
	EV_GPIO1        = 0,
	EV_GPIO2        = 1,
	EV_GPIO3        = 2,

} GPIOEmType;

typedef enum
{
	EV_GPIO_OPT1 = 0,
	EV_GPIO_OPT2 = 1,
	EV_GPIO_OPT3 = 2,
	EV_GPIO_OPT4 = 3,
	EV_GPIO_OPT5 = 4,

} GPIOOptEmType;

typedef enum
{
	EV_NORVOL        = 0,
	EV_MINVOL        = 1,
	EV_LOWVOL        = 2,
	EV_HIGHVOL       = 3,
    EV_MAXVOL        = 4,

} PADVddEmType;

typedef enum
{
	EV_VDD_DUMMY     = 0,
	EV_VDD_165_195   = BIT07_T,
	EV_VDD_20_21     = BIT08_T,
	EV_VDD_21_22     = BIT09_T,
	EV_VDD_22_23     = BIT10_T,
	EV_VDD_23_24     = BIT11_T,
	EV_VDD_24_25     = BIT12_T,
	EV_VDD_25_26     = BIT13_T,
	EV_VDD_26_27     = BIT14_T,
	EV_VDD_27_28     = BIT15_T,
	EV_VDD_28_29     = BIT16_T,
	EV_VDD_29_30     = BIT17_T,
	EV_VDD_30_31     = BIT18_T,
	EV_VDD_31_32     = BIT19_T,
	EV_VDD_32_33     = BIT20_T,
	EV_VDD_33_34     = BIT21_T,
	EV_VDD_34_35     = BIT22_T,
	EV_VDD_35_36     = BIT23_T,
	EV_VDD_50        = BIT24_T,

} VddEmType;


void Hal_CARD_IPOnceSetting(IPEmType eIP);
void Hal_CARD_IPBeginSetting(IPEmType eIP);
void Hal_CARD_IPEndSetting(IPEmType eIP);

// PAD Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void Hal_CARD_InitPADPin(PADEmType ePAD, BOOL_T bTwoCard);
void Hal_CARD_SetPADToPortPath(IPEmType eIP, PortEmType ePort, PADEmType ePAD, BOOL_T bTwoCard);
void Hal_CARD_PullPADPin(PADEmType ePAD, PinPullEmType ePinPull, BOOL_T bTwoCard);

// Clock Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void Hal_CARD_SetClock(IPEmType eIP, U32_T u32ClkFromIPSet);
U32_T Hal_CARD_FindClockSetting(IPEmType eIP, U32_T u32ReffClk, U8_T u8PassLevel, U8_T u8DownLevel);
void Hal_CARD_SetBusTiming(IPEmType eIP, BusTimingEmType eBusTiming);

// Power and Voltage Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
void Hal_CARD_SetPADPower(PADEmType ePAD, PADVddEmType ePADVdd);
void Hal_CARD_PowerOn(PADEmType ePAD, U16_T u16DelayMs);
void Hal_CARD_PowerOff(PADEmType ePAD, U16_T u16DelayMs);

// Card Detect and GPIO Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
U32_T Hal_CARD_GetGPIONum(GPIOEmType eGPIO);
BOOL_T Hal_CARD_GetGPIOState(GPIOEmType eGPIO);
void Hal_CARD_InitGPIO(GPIOEmType eGPIO, BOOL_T bEnable);

void Hal_CARD_SetGPIOIntAttr(GPIOEmType eGPIO, GPIOOptEmType eGPIOOPT);
BOOL_T Hal_CARD_GPIOIntFilter(GPIOEmType eGPIO);

// MIU Setting for Card Platform
//----------------------------------------------------------------------------------------------------------
U32_T Hal_CARD_TransMIUAddr(U32_T u32Addr);

#endif //End of __HAL_CARD_PLATFORM_H






