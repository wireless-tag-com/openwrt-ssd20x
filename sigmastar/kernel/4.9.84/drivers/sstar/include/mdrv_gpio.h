/*
* mdrv_gpio.h- Sigmastar
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
#ifndef _MDRV_GPIO_H_
#define _MDRV_GPIO_H_

#include <asm/types.h>
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

void MDrv_GPIO_Init(void);
void MDrv_GPIO_Pad_Set(U8 u8IndexGPIO);                    // set a pad to GPIO mode
int  MDrv_GPIO_PadGroupMode_Set(U32 u32PadMode);           // set the specified pad mode(a set of GPIO pads will be effected)
                                                           //     return: 0->success; -1: fail or not supported
int  MDrv_GPIO_PadVal_Set(U8 u8IndexGPIO, U32 u32PadMode); // set a pad to the specified mode
                                                           //     return: 0->success; -1: fail or not supported
void MDrv_GPIO_Pad_Oen(U8 u8IndexGPIO);
void MDrv_GPIO_Pad_Odn(U8 u8IndexGPIO);
U8 MDrv_GPIO_Pad_Read(U8 u8IndexGPIO);
U8 MDrv_GPIO_Pad_InOut(U8 u8IndexGPIO);
void MDrv_GPIO_Pull_High(U8 u8IndexGPIO);
void MDrv_GPIO_Pull_Low(U8 u8IndexGPIO);
void MDrv_GPIO_Set_High(U8 u8IndexGPIO);
void MDrv_GPIO_Set_Low(U8 u8IndexGPIO);
void MDrv_Enable_GPIO_INT(U8 u8IndexGPIO);
int MDrv_GPIO_To_Irq(U8 u8IndexGPIO);
void MDrv_GPIO_PAD_32K_OUT(U8 u8Enable);
void MDrv_GPIO_Set_POLARITY(U8 u8IndexGPIO, U8 reverse);
void MDrv_GPIO_Set_Driving(U8 u8IndexGPIO, U8 setHigh);

#endif // _MDRV_GPIO_H_

