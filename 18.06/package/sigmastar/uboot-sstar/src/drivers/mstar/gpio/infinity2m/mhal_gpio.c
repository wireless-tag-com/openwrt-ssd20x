/*
* mhal_gpio.c- Sigmastar
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

#ifndef _MHAL_GPIO_C_
#define _MHAL_GPIO_C_


#include <common.h>
#include <command.h>
#include <MsDebug.h>

#include "mhal_gpio.h"
#include "padmux.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
///#define GPIO_HAL_ERR(x, args...)        //{printf(x, ##args);}
//#define GPIO_HAL_NOTSUPPORT()           {printf("%s not support!\n", __FUNCTION__);}

#ifndef MS_ASSERT
#define MS_ASSERT(expr)                 do {                                                        \
                                            if(!(expr))                                             \
                                                printf("MVD assert fail %s %d!\n", __FILE__, __LINE__); \
                                        } while(0)
#endif

//PADTOP
#include "GPIO_TABLE.ci"

//  reg Defines
U32 gChipBaseAddr    = 0x1F203C00;
U32 gPmSleepBaseAddr = 0x1F001C00;
U32 gSarBaseAddr     = 0x1F002800;
U32 gRIUBaseAddr     = 0x1F000000;


#define MHal_CHIPTOP_REG(addr)  (*(volatile U8*)(gChipBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_PM_SLEEP_REG(addr) (*(volatile U8*)(gPmSleepBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_SAR_GPIO_REG(addr) (*(volatile U8*)(gSarBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_RIU_REG(addr)      (*(volatile U8*)(gRIUBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))

#define REG_ALL_PAD_IN     0xA1

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

void MHal_GPIO_Init(void)
{
    MHal_CHIPTOP_REG(REG_ALL_PAD_IN) &= ~BIT7;
}

void MHal_GPIO_Pad_Set(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        UBOOT_INFO("gpio debug %s: pin=%d\n", __FUNCTION__, u32IndexGPIO);
        HalPadSetVal(u32IndexGPIO, PINMUX_FOR_GPIO_MODE);
    }
    else
    {
        MS_ASSERT(0);
    }
}

int MHal_GPIO_PadGroupMode_Set(U32 u32PadMode)
{
    return HalPadSetMode(u32PadMode);
}

int MHal_GPIO_PadVal_Set(MS_GPIO_NUM u32IndexGPIO, U32 u32PadMode)
{
    return HalPadSetVal(u32IndexGPIO, u32PadMode);
}

void MHal_GPIO_Pad_Oen(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen) &= (~gpio_table[u32IndexGPIO].m_oen);
    }
    else
    {
        MS_ASSERT(0);
    }
}

void MHal_GPIO_Pad_Odn(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen) |= gpio_table[u32IndexGPIO].m_oen;
    }
    else
    {
        MS_ASSERT(0);
    }
}

int MHal_GPIO_Pad_Level(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        return ((MHal_RIU_REG(gpio_table[u32IndexGPIO].r_in)&gpio_table[u32IndexGPIO].m_in)? 1 : 0);
    }
    else
    {
        MS_ASSERT(0);
        return 0;
    }
}

U8 MHal_GPIO_Pad_InOut(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        return ((MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen)&gpio_table[u32IndexGPIO].m_oen)? 1 : 0);
    }
    else
    {
        MS_ASSERT(0);
        return 0;
    }
}

void MHal_GPIO_Pull_High(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_out) |= gpio_table[u32IndexGPIO].m_out;
    }
    else
    {
        MS_ASSERT(0);
    }
}

void MHal_GPIO_Pull_Low(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_out) &= (~gpio_table[u32IndexGPIO].m_out);
    }
    else
    {
        MS_ASSERT(0);
    }
}

void MHal_GPIO_Set_High(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen) &= (~gpio_table[u32IndexGPIO].m_oen);
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_out) |= gpio_table[u32IndexGPIO].m_out;
    }
    else
    {
        MS_ASSERT(0);
    }
}

void MHal_GPIO_Set_Low(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen) &= (~gpio_table[u32IndexGPIO].m_oen);
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_out) &= (~gpio_table[u32IndexGPIO].m_out);
    }
    else
    {
        MS_ASSERT(0);
    }
}

void MHal_GPIO_Set_Input(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen) |= (gpio_table[u32IndexGPIO].m_oen);
    }
    else
    {
        MS_ASSERT(0);
    }
}

void MHal_GPIO_Set_Output(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen) &= (gpio_table[u32IndexGPIO].m_oen);
    }
    else
    {
        MS_ASSERT(0);
    }
}

int MHal_GPIO_Get_InOut(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        return ((MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen)&gpio_table[u32IndexGPIO].m_oen)? 1 : 0);
    }
    else
    {
        MS_ASSERT(0);
        return 0;
    }
}

void MHal_GPIO_Set_Driving(MS_GPIO_NUM u32IndexGPIO, U32 setHigh)
{
    U32 r_driving[] = {0x101E7E, 0x101E7E, 0x101E7E, 0x101E7E};
    U8  m_driving[] = {BIT0,     BIT1,     BIT2,     BIT3};
    int index = u32IndexGPIO - PAD_TTL16;

    if (u32IndexGPIO < PAD_TTL16 || u32IndexGPIO > PAD_TTL19 )
    {
        printf("not support\n");
        return;
    }

    if (setHigh)
    {
        printf("pad %d: driving high\n", u32IndexGPIO);
        MHal_RIU_REG(r_driving[index]) |= m_driving[index];
    }
    else
    {
        MHal_RIU_REG(r_driving[index]) &= (~m_driving[index]);
    }
}

#endif //_MHAL_GPIO_C_
