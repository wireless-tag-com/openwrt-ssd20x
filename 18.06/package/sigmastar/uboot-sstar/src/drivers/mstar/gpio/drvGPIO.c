/*
* drvGPIO.c- Sigmastar
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

#ifndef __DRVGPIO_C__
#define __DRVGPIO_C__

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include <common.h>
#include <command.h>
#include <MsDebug.h>
#include "drvGPIO.h"
#include "mhal_gpio.h"


//-------------------------------------------------------------------------------------------------
//  Local Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

 
//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static const struct gpio_operations __gpio =
{
     .set_high  = MHal_GPIO_Set_High,
     .set_low   = MHal_GPIO_Set_Low,
     .set_input = MHal_GPIO_Set_Input,
     .set_output = MHal_GPIO_Set_Output,
     .get_inout = MHal_GPIO_Get_InOut,
     .get_level = MHal_GPIO_Pad_Level,
     
};

static const struct gpio_operations *gpio_op = &__gpio;

static GPIO_DbgLv _geDbfLevel = E_GPIO_DBGLV_NONE;


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------
#define MS_DEBUG_MSG(x)             x

#define GPIO_DBG_FUNC()             if (_geDbfLevel >= E_GPIO_DBGLV_ALL) \
                                        {MS_DEBUG_MSG(printf("\t====   %s   ====\n", __FUNCTION__);)}

#define GPIO_DBG_INFO(x, args...)   if (_geDbfLevel >= E_GPIO_DBGLV_INFO ) \
                                        {MS_DEBUG_MSG(printf(x, ##args);)}

#define GPIO_DBG_ERR(x, args...)    if (_geDbfLevel >= E_GPIO_DBGLV_ERR_ONLY) \
                                        {MS_DEBUG_MSG(printf(x, ##args);)}

#ifndef MS_ASSERT
#define MS_ASSERT(expr)             do {                                                        \
                                        if(!(expr))                                             \
                                            printf("MVD assert fail %s %d!\n", __FILE__, __LINE__); \
                                    } while(0)
#endif


//------------------------------------------------------------------------------
//  Local Functions
//------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/// select one pad to set
/// @param  gpio              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pad_Set(MS_GPIO_NUM gpio)
{
    MHal_GPIO_Pad_Set(gpio);
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
int MDrv_GPIO_PadVal_Set(MS_GPIO_NUM gpio, U32 u32PadMode)
{
    return MHal_GPIO_PadVal_Set(gpio, u32PadMode);
}



void mdrv_gpio_set_high(MS_GPIO_NUM gpio)
{
    GPIO_DBG_FUNC();

    MS_ASSERT(gpio_op);
    if (gpio_op)
    {
        gpio_op->set_high(gpio);
    }
}

void mdrv_gpio_set_low(MS_GPIO_NUM gpio)
{
    GPIO_DBG_FUNC();

    MS_ASSERT(gpio_op);
    if (gpio_op)
    {
        gpio_op->set_low(gpio);
    }
}

void mdrv_gpio_set_input(MS_GPIO_NUM gpio)
{
    GPIO_DBG_FUNC();

    MS_ASSERT(gpio_op);
    if (gpio_op)
    {
        gpio_op->set_input(gpio);
    }
}

#if 0
void mdrv_gpio_set_output(MS_GPIO_NUM gpio)
{
    MS_ASSERT(gpio_op);
    if (gpio_op)
    {
        gpio_op->set_output(gpio);
    }
}
#endif

int mdrv_gpio_get_inout(MS_GPIO_NUM gpio)
{
    GPIO_DBG_FUNC();

    MS_ASSERT(gpio_op);
    if (gpio_op)
    {
        return gpio_op->get_inout(gpio);
    }
    else
    {
        return 0;
    }
}

int mdrv_gpio_get_level(MS_GPIO_NUM gpio)
{
    GPIO_DBG_FUNC();

    MS_ASSERT(gpio_op);
    if (gpio_op)
    {
        return gpio_op->get_level(gpio);
    }
    else
    {
        return 0;
    }
}

//-------------------------------------------------------------------------------------------------
/// enable input for selected one pad
/// @param  u32IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pad_Odn(MS_GPIO_NUM u32IndexGPIO)
{
    MHal_GPIO_Pad_Odn(u32IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// enable output for selected one pad
/// @param  u32IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pad_Oen(MS_GPIO_NUM u32IndexGPIO)
{
    MHal_GPIO_Pad_Oen(u32IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// output pull high for selected one pad
/// @param  u32IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pull_High(MS_GPIO_NUM u32IndexGPIO)
{
    MHal_GPIO_Pull_High(u32IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// output pull low for selected one pad
/// @param  u32IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
void MDrv_GPIO_Pull_Low(MS_GPIO_NUM u32IndexGPIO)
{
    MHal_GPIO_Pull_Low(u32IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// read data from selected one pad
/// @param  u32IndexGPIO              \b IN:  pad index
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
U8 MDrv_GPIO_Pad_Read(MS_GPIO_NUM u32IndexGPIO)
{
    return MHal_GPIO_Pad_Level(u32IndexGPIO);
}

//-------------------------------------------------------------------------------------------------
/// set gpio driving
/// @param  u8IndexGPIO              \b IN:  pad index
///         setHigh                  \b IN:  0->4mA, 1->8mA
/// @return None
/// @note
//-------------------------------------------------------------------------------------------------
__attribute__((weak)) void MHal_GPIO_Set_Driving(MS_GPIO_NUM u32IndexGPIO, U32 setHigh) {
   printf("not support\n");
}

void MDrv_GPIO_Set_Driving(MS_GPIO_NUM u32IndexGPIO, U32 setHigh)
{
    MHal_GPIO_Set_Driving(u32IndexGPIO, setHigh);
}

#endif  //__DRVGPIO_C__
