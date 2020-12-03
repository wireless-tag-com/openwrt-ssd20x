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
#define IO_PHYS                 0x1F000000
#define MS_BASE_REG_RIU_PA				IO_PHYS


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
extern void MHal_GPIO_WriteRegBit(U32 u32Reg, U8 u8Enable, U8 u8BitMsk);
extern U8 MHal_GPIO_ReadRegBit(U32 u32Reg, U8 u8BitMsk);
extern void MHal_GPIO_Pad_Set(U8 u8IndexGPIO);
extern void MHal_GPIO_Pad_Oen(U8 u8IndexGPIO);
extern void MHal_GPIO_Pad_Odn(U8 u8IndexGPIO);
extern U8 MHal_GPIO_Pad_Level(U8 u8IndexGPIO);
extern U8 MHal_GPIO_Pad_InOut(U8 u8IndexGPIO);
extern void MHal_GPIO_Pull_High(U8 u8IndexGPIO);
extern void MHal_GPIO_Pull_Low(U8 u8IndexGPIO);
extern void MHal_GPIO_Set_High(U8 u8IndexGPIO);
extern void MHal_GPIO_Set_Low(U8 u8IndexGPIO);
extern void MHal_Enable_GPIO_INT(U8 u8IndexGPIO);
extern void MHal_GPIO_Set_POLARITY(U8 u8IndexGPIO,U8 reverse);
extern void MHal_GPIO_PAD_32K_OUT(U8 u8Enable);

#endif // _HAL_GPIO_H_

