/*
* mdrv_gpio.c- Sigmastar
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
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <asm/io.h>

#include "mdrv_gpio.h"
#include "mhal_gpio.h"

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/// GPIO chiptop initialization
/// @return None
/// @note   Called only once at system initialization
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Init(void)
{
    MHal_GPIO_Init();
}

//-------------------------------------------------------------------------------------------------
/// select one pad to set to GPIO mode
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pad_Set(U8 u8IndexGPIO)
{
    MHal_GPIO_Pad_Set(u8IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// set the specified pad mode( a set of GPIO pad will be effected)
/// @param  u8PadMode              \b IN:  pad mode
/// @return 0: success; -1: fail or not supported
/// @note
//-------------------------------------------------------------------------------------------------
int MDrv_GPIO_PadGroupMode_Set(U32 u32PadMode)
{
    return MHal_GPIO_PadGroupMode_Set(u32PadMode);
}

//-------------------------------------------------------------------------------------------------
/// set a pad to the specified mode
/// @param  u8PadMode              \b IN:  pad mode
/// @return 0: success; -1: fail or not supported
/// @note
//-------------------------------------------------------------------------------------------------
int MDrv_GPIO_PadVal_Set(U8 u8IndexGPIO, U32 u32PadMode)
{
    return MHal_GPIO_PadVal_Set(u8IndexGPIO, u32PadMode);
}

//-------------------------------------------------------------------------------------------------
/// enable output for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pad_Oen(U8 u8IndexGPIO)
{
    MHal_GPIO_Pad_Oen(u8IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// enable input for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pad_Odn(U8 u8IndexGPIO)
{
    MHal_GPIO_Pad_Odn(u8IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// read data from selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
U8 MDrv_GPIO_Pad_Read(U8 u8IndexGPIO)
{
    return MHal_GPIO_Pad_Level(u8IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// read pad direction for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
U8 MDrv_GPIO_Pad_InOut(U8 u8IndexGPIO)
{
    return MHal_GPIO_Pad_InOut(u8IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// output pull high for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pull_High(U8 u8IndexGPIO)
{
    MHal_GPIO_Pull_High(u8IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// output pull low for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pull_Low(U8 u8IndexGPIO)
{
    MHal_GPIO_Pull_Low(u8IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// output set high for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Set_High(U8 u8IndexGPIO)
{
    MHal_GPIO_Set_High(u8IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// output set low for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Set_Low(U8 u8IndexGPIO)
{
    MHal_GPIO_Set_Low(u8IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// enable GPIO int for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_Enable_GPIO_INT(U8 u8IndexGPIO)
{
    MHal_Enable_GPIO_INT(u8IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// enable GPIO int for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
int MDrv_GPIO_To_Irq(U8 u8IndexGPIO)
{
    return MHal_GPIO_To_Irq(u8IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// set GPIO int polarity for selected one pad
/// @param  u8IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Set_POLARITY(U8 u8IndexGPIO, U8 reverse)
{
    MHal_GPIO_Set_POLARITY(u8IndexGPIO, reverse);
}

//-------------------------------------------------------------------------------------------------
/// set gpio driving
/// @param  u8IndexGPIO              \b IN:  pad index
///         setHigh                  \b IN:  0->4mA, 1->8mA
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
__attribute__((weak)) void MHal_GPIO_Set_Driving(U8 u8IndexGPIO, U8 setHigh) {
   // "not support
}

void MDrv_GPIO_Set_Driving(U8 u8IndexGPIO, U8 setHigh)
{
    MHal_GPIO_Set_Driving(u8IndexGPIO, setHigh);
}


EXPORT_SYMBOL(MDrv_GPIO_Init);
EXPORT_SYMBOL(MDrv_GPIO_Pad_Set);
EXPORT_SYMBOL(MDrv_GPIO_PadGroupMode_Set);
EXPORT_SYMBOL(MDrv_GPIO_PadVal_Set);
EXPORT_SYMBOL(MDrv_GPIO_Pad_Oen);
EXPORT_SYMBOL(MDrv_GPIO_Pad_Odn);
EXPORT_SYMBOL(MDrv_GPIO_Pad_Read);
EXPORT_SYMBOL(MDrv_GPIO_Pad_InOut);
EXPORT_SYMBOL(MDrv_GPIO_Pull_High);
EXPORT_SYMBOL(MDrv_GPIO_Pull_Low);
EXPORT_SYMBOL(MDrv_GPIO_Set_High);
EXPORT_SYMBOL(MDrv_GPIO_Set_Low);
EXPORT_SYMBOL(MDrv_Enable_GPIO_INT);
EXPORT_SYMBOL(MDrv_GPIO_To_Irq);
EXPORT_SYMBOL(MDrv_GPIO_Set_POLARITY);
EXPORT_SYMBOL(MDrv_GPIO_Set_Driving);

