/*
* mhal_gpio.h- Sigmastar
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
#ifndef _MHAL_GPIO_H_
#define _MHAL_GPIO_H_

#include <asm/types.h>
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

extern void MHal_GPIO_Init(void);
extern void MHal_GPIO_Pad_Set(U8 u8IndexGPIO);
extern int  MHal_GPIO_PadGroupMode_Set(U32 u32PadMode);
extern int  MHal_GPIO_PadVal_Set(U8 u8IndexGPIO, U32 u32PadMode);
extern void MHal_GPIO_Pad_Oen(U8 u8IndexGPIO);
extern void MHal_GPIO_Pad_Odn(U8 u8IndexGPIO);
extern U8 MHal_GPIO_Pad_Level(U8 u8IndexGPIO);
extern U8 MHal_GPIO_Pad_InOut(U8 u8IndexGPIO);
extern void MHal_GPIO_Pull_High(U8 u8IndexGPIO);
extern void MHal_GPIO_Pull_Low(U8 u8IndexGPIO);
extern void MHal_GPIO_Set_High(U8 u8IndexGPIO);
extern void MHal_GPIO_Set_Low(U8 u8IndexGPIO);
extern void MHal_Enable_GPIO_INT(U8 u8IndexGPIO);
extern int MHal_GPIO_To_Irq(U8 u8IndexGPIO);
extern void MHal_GPIO_Set_POLARITY(U8 u8IndexGPIO, U8 reverse);
extern void MHal_GPIO_Set_Driving(U8 u8IndexGPIO, U8 setHigh);
extern void MHal_GPIO_PAD_32K_OUT(U8 u8Enable);

#endif // _MHAL_GPIO_H_

