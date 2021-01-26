/*
* mdrv_PWM.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: ken.chang <ken-ms.chang@sigmastar.com.tw>
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
#ifndef __MDRV_PWM_H
#define __MDRV_PWM_H

void DrvPWMInit(U8 u8Id);
void DrvPWMSetPeriod(U8 u8Id, U32 u32Val);
void DrvPWMGetPeriod(U8 u8Id, U32* pu32Val);
void DrvPWMSetDuty(U8 u8Id, U32 u32Val);
void DrvPWMGetDuty(U8 u8Id, U32* pu32Val);
void DrvPWMEnable(U8 u8Id, U8 u8Val);
void DrvPWMEnableGet(U8 u8Id, U8* pu8Val);
void DrvPWMSetPolarity(U8 u8Id, U8 u8Val);
void DrvPWMGetPolarity(U8 u8Id, U8* pu8Val);
void DrvPWMPadSet(U8 u8Id, U8 u8Val);
void DrvPWMSetDben(U8 u8Id, U8 u8Val);

#endif
