/*
* mhal_gpio.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: giggshuang <giggshuang@sigmastar.com.tw>
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

#ifndef _MHAL_GPIO_H_
#define _MHAL_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
//  Header
//-------------------------------------------------------------------------------------------------
#include <MsTypes.h>
#include "gpio.h"


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
//#define MASK(x)     (((1<<(x##_BITS))-1) << x##_SHIFT)
//#define BIT(_bit_)                  (1 << (_bit_))
#define BIT_(x)                     BIT(x) //[OBSOLETED] //TODO: remove it later
#define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
#define BMASK(_bits_)               (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))

#define END_GPIO_NUM                GPIO_NR


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
void MHal_GPIO_Pad_Set(MS_GPIO_NUM u32IndexGPIO);
int  MHal_GPIO_PadGroupMode_Set(U32 u32PadMode);    // return -1: not support
int  MHal_GPIO_PadVal_Set(MS_GPIO_NUM u32IndexGPIO, U32 u32PadMode); // return -1: not support
void MHal_GPIO_Set_High(MS_GPIO_NUM u32IndexGPIO);
void MHal_GPIO_Set_Low(MS_GPIO_NUM u32IndexGPIO);
void MHal_GPIO_Set_Input(MS_GPIO_NUM u32IndexGPIO);
void MHal_GPIO_Set_Output(MS_GPIO_NUM u32IndexGPIO);
int MHal_GPIO_Get_InOut(MS_GPIO_NUM u32IndexGPIO);
int MHal_GPIO_Pad_Level(MS_GPIO_NUM u32IndexGPIO);

void MHal_GPIO_Pad_Oen(MS_GPIO_NUM u32IndexGPIO);
void MHal_GPIO_Pad_Odn(MS_GPIO_NUM u32IndexGPIO);
U8 MHal_GPIO_Pad_InOut(MS_GPIO_NUM u32IndexGPIO);
void MHal_GPIO_Pull_Low(MS_GPIO_NUM u32IndexGPIO);
void MHal_GPIO_Pull_High(MS_GPIO_NUM u32IndexGPIO);

#ifdef __cplusplus
}
#endif

#endif // _MHAL_GPIO_H_
