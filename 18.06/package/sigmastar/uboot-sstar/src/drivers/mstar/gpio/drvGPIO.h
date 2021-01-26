/*
* drvGPIO.h- Sigmastar
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

#ifndef __DRVGPIO_H__
#define __DRVGPIO_H__


#ifdef __cplusplus
extern "C"
{
#endif

#include "MsTypes.h"


typedef enum _GPIO_DbgLv
{
    E_GPIO_DBGLV_NONE,                  //no debug message
    E_GPIO_DBGLV_ERR_ONLY,              //show error only
    E_GPIO_DBGLV_REG_DUMP,              //show error & reg dump
    E_GPIO_DBGLV_INFO,                  //show error & informaiton
    E_GPIO_DBGLV_ALL                    //show error, information & funciton name
}GPIO_DbgLv;

typedef enum _GPIO_Result
{
    E_GPIO_NOT_SUPPORT = -1,
    E_GPIO_FAIL = 0,
    E_GPIO_OK = 1
}GPIO_Result;


/// GPIO operations for GPIO extension or IIC implmentation
struct gpio_operations
{
    void (*set_high) (MS_GPIO_NUM gpio);    ///< output high
    void (*set_low)  (MS_GPIO_NUM gpio);    ///< output low
    void (*set_input)(MS_GPIO_NUM gpio);    ///< set to input
    void (*set_output)(MS_GPIO_NUM gpio);   ///< set to output
    int (*get_inout)(MS_GPIO_NUM gpio);     ///< get in or out
    int (*get_level)(MS_GPIO_NUM gpio);     ///< get level
};

void MDrv_GPIO_Pad_Set(MS_GPIO_NUM gpio);                  // set a pad to GPIO mode
int  MDrv_GPIO_PadGroupMode_Set(U32 u32PadMode);           // set the specified pad mode(a set of GPIO pads will be effected)
                                                           //     return: 0->success; -1: fail or not supported
int  MDrv_GPIO_PadVal_Set(MS_GPIO_NUM MS_GPIO_NUM, U32 u32PadMode); // set a pad to the specified mode
                                                           //     return: 0->success; -1: fail or not supported
void mdrv_gpio_set_high(MS_GPIO_NUM gpio);
void mdrv_gpio_set_low(MS_GPIO_NUM gpio);
void mdrv_gpio_set_input(MS_GPIO_NUM gpio);
void mdrv_gpio_set_output(MS_GPIO_NUM gpio);
int mdrv_gpio_get_inout(MS_GPIO_NUM gpio);
int mdrv_gpio_get_level(MS_GPIO_NUM gpio);
void MDrv_GPIO_Pad_Odn(MS_GPIO_NUM u32IndexGPIO);
void MDrv_GPIO_Pad_Oen(MS_GPIO_NUM u32IndexGPIO);
void MDrv_GPIO_Pull_High(MS_GPIO_NUM u32IndexGPIO);
void MDrv_GPIO_Pull_Low(MS_GPIO_NUM u32IndexGPIO);
unsigned char MDrv_GPIO_Pad_Read(MS_GPIO_NUM u32IndexGPIO);
void MDrv_GPIO_Set_Driving(MS_GPIO_NUM u32IndexGPIO, U32 setHigh);


#ifdef __cplusplus
}
#endif

#endif /* __DRVGPIO_H__ */
