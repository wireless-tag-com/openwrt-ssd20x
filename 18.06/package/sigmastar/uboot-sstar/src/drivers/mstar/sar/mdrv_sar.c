/*
* mdrv_sar.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
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
#include "MsTypes.h"
#include <common.h>
#include "mdrv_sar.h"

#define BOOL    MS_BOOL

//#define OPEN_SAR_DEBUG
static U32 _gMIO_MapBase = 0x1F002800;

#ifdef OPEN_SAR_DEBUG
#define sarDbg  printf
#else
#define sarDbg(...)
#endif


BOOL HAL_SAR_Write2Byte(U32 u32RegAddr, U16 u16Val)
{
    (*(volatile U32*)(_gMIO_MapBase+((u32RegAddr & 0xFFFFFF00ul) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2))) = u16Val;
    return TRUE;
}

U16 HAL_SAR_Read2Byte(U32 u32RegAddr)
{
    return (*(volatile U32*)(_gMIO_MapBase+((u32RegAddr & 0xFFFFFF00ul) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2)));
}

BOOL HAL_SAR_Write2ByteMask(U32 u32RegAddr, U16 u16Val, U16 u16Mask)
{
    u16Val = (HAL_SAR_Read2Byte(u32RegAddr) & ~u16Mask) | (u16Val & u16Mask);
    //sarDbg("sar IOMap base:%16llx u16Val:%4x\n", _gMIO_MapBase, u16Val);
    HAL_SAR_Write2Byte(u32RegAddr, u16Val);
    return TRUE;
}


void sar_hw_init(void)
{
    HAL_SAR_Write2Byte(REG_SAR_CTRL0,0x0a20);
    //HAL_SAR_Write2Byte(REG_SAR_CKSAMP_PRD,0x0005);
    //HAL_SAR_Write2ByteMask(REG_SAR_CTRL0,0x4000,0x4000);
}

int sar_get_value(int ch)
{
    U16 value=0;
    U32 count=0;
    HAL_SAR_Write2ByteMask(REG_SAR_CTRL0,BIT14, 0x4000);
    while(HAL_SAR_Read2Byte(REG_SAR_CTRL0)&BIT14 && count<100000)
    {
        udelay(1);
        count++;
    }

    switch(ch)
    {
    case 0:
        HAL_SAR_Write2ByteMask(REG_SAR_AISEL_CTRL, BIT0, BIT0);
        value=HAL_SAR_Read2Byte(REG_SAR_CH1_DATA);
        break;
    case 1:
        HAL_SAR_Write2ByteMask(REG_SAR_AISEL_CTRL, BIT1, BIT1);
        value=HAL_SAR_Read2Byte(REG_SAR_CH2_DATA);
        break;
    case 2:
        HAL_SAR_Write2ByteMask(REG_SAR_AISEL_CTRL, BIT2, BIT2);
        value=HAL_SAR_Read2Byte(REG_SAR_CH3_DATA);
        break;
    case 3:
        HAL_SAR_Write2ByteMask(REG_SAR_AISEL_CTRL, BIT3, BIT3);
        value=HAL_SAR_Read2Byte(REG_SAR_CH4_DATA);
        break;
    default:
        sarDbg("error channel,support SAR0,SAR1,SAR2,SAR3\n");
        break;
    }
    return  value;
}


