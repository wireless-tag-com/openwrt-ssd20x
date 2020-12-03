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
#ifndef _HAL_GPIO_H_
#define _HAL_GPIO_H_

#include <asm/types.h>
#include "mdrv_types.h"

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
//#define MASK(x)     (((1<<(x##_BITS))-1) << x##_SHIFT)
//#define BIT(_bit_)                  (1 << (_bit_))
#define BIT_(x)                     BIT(x) //[OBSOLETED] //TODO: remove it later
#define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
#define BMASK(_bits_)               (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
//the functions of this section set to initialize
extern void MHal_GPIO_Init(void);
extern void MHal_GPIO_Pad_Set(U16 u16IndexGPIO);
extern int  MHal_GPIO_PadGroupMode_Set(U32 u32PadMode);
extern int  MHal_GPIO_PadVal_Set(U16 u16IndexGPIO, U32 u32PadMode);
extern void MHal_GPIO_Pad_Oen(U16 u16IndexGPIO);
extern void MHal_GPIO_Pad_Odn(U16 u16IndexGPIO);
extern U16 MHal_GPIO_Pad_Level(U16 u16IndexGPIO);
extern U16 MHal_GPIO_Pad_InOut(U16 u16IndexGPIO);
extern void MHal_GPIO_Pull_High(U16 u16IndexGPIO);
extern void MHal_GPIO_Pull_Low(U16 u16IndexGPIO);
extern void MHal_GPIO_Set_High(U16 u16IndexGPIO);
extern void MHal_GPIO_Set_Low(U16 u16IndexGPIO);
extern void MHal_Enable_GPIO_INT(U16 u16IndexGPIO);
extern int MHal_GPIO_To_Irq(U16 u16IndexGPIO);
extern void MHal_GPIO_Set_POLARITY(U16 u16IndexGPIO, U8 reverse);
//extern void MHal_GPIO_PAD_32K_OUT(U8 u8Enable);
#endif // _HAL_GPIO_H_

