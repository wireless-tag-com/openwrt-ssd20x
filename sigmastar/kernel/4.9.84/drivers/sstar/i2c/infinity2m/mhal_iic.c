/*
* mhal_iic.c- Sigmastar
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
//#include "MsCommon.h"
//#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <asm/io.h>

#ifndef _HAL_IIC_C_
#define _HAL_IIC_C_

#include "ms_platform.h"
#include "mhal_iic.h"
#include "mhal_iic_reg.h"
#include "../ms_iic.h"

#if 0 // For GPIO (emac orange led) trigger debug
#include "gpio.h"
extern void MDrv_GPIO_Set_Low(U8 u8IndexGPIO);
extern void MDrv_GPIO_Set_High(U8 u8IndexGPIO);
#endif

#define LOWBYTE(w)                ((w) & 0x00ff)
#define HIBYTE(w)                (((w) >> 8) & 0x00ff)



////////////////////////////////////////////////////////////////////////////////
// Define & data type
///////////////////////////////////////////////////////////////////////////////
#define HWI2C_HAL_RETRY_TIMES     (5)
#define HWI2C_HAL_WAIT_TIMEOUT    65535////30000//(1500)
#define HWI2C_HAL_FUNC()              //{printk("=============%s\n",  __func__);}
#define HWI2C_HAL_INFO(x, args...)    //{printk(x, ##args);}
#define HWI2C_HAL_ERR(x, args...)     {printk(x, ##args);}
#ifndef UNUSED
#define UNUSED(x) ((x)=(x))
#endif

#define HWI2C_DMA_CMD_DATA_LEN      7
#define HWI2C_DMA_WAIT_TIMEOUT      (30000)
#define HWI2C_DMA_WRITE             0
#define HWI2C_DMA_READ              1
#define _PA2VA(x) (U32)MsOS_PA2KSEG1((x))
#define _VA2PA(x) (U32)MsOS_VA2PA((x))

////////////////////////////////////////////////////////////////////////////////
// Local variable
////////////////////////////////////////////////////////////////////////////////
static U32 _gMIO_MapBase[HAL_HWI2C_PORTS] = {0};
static U32 _gMChipIO_MapBase = 0;
static U32 _gMClkIO_MapBase = 0;
static BOOL g_bLastByte[HAL_HWI2C_PORTS];
static U32 g_u32DmaPhyAddr[HAL_HWI2C_PORTS];
static HAL_HWI2C_PortCfg g_stPortCfg[HAL_HWI2C_PORTS];
static U16 g_u16DmaDelayFactor[HAL_HWI2C_PORTS];

////////////////////////////////////////////////////////////////////////////////
// Extern Function
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function Declaration
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Local Function
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Global Function
////////////////////////////////////////////////////////////////////////////////

U32 MsOS_PA2KSEG1(U32 addr)
{
    return ((U32)(((U32)addr) | 0xa0000000));
}
U32 MsOS_VA2PA(U32 addr)
{
    return ((U32)(((U32)addr) & 0x1fffffff));
}

void HAL_HWI2C_ExtraDelay(U32 u32Us)
{
    // volatile is necessary to avoid optimization
    U32 volatile u32Dummy = 0;
    //U32 u32Loop;
    U32 volatile u32Loop;

    u32Loop = (U32)(50 * u32Us);
    while (u32Loop--)
    {
        u32Dummy++;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_SetIOMapBase
/// @brief \b Function  \b Description: Dump bdma all register
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b None :
////////////////////////////////////////////////////////////////////////////////
void HAL_HWI2C_SetIOMapBase(U8 u8Port, U32 u32Base,U32 u32ChipBase,U32 u32ClkBase)
{
    HWI2C_HAL_FUNC();

    _gMIO_MapBase[u8Port] = u32Base;
    _gMChipIO_MapBase = u32ChipBase;
    _gMClkIO_MapBase = u32ClkBase;

    HWI2C_HAL_INFO(" _gMIO_MapBase[%d]: %x \n", u8Port, u32Base);
    HWI2C_HAL_INFO(" _gMChipIO_MapBase: %x\n", u32ChipBase);
    HWI2C_HAL_INFO(" _gMClkIO_MapBase: %x \n", u32ClkBase);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_ReadByte
/// @brief \b Function  \b Description: read 1 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <OUT>        \b None :
/// @param <RET>        \b U8
////////////////////////////////////////////////////////////////////////////////
U8 HAL_HWI2C_ReadByte(U32 u32RegAddr)
{
    U16 u16value;
    U8 u8Port;

    HWI2C_HAL_FUNC();
    if(FALSE == HAL_HWI2C_GetPortIdxByOffset(u32RegAddr&(~0xFF), &u8Port))
        return FALSE;
    u32RegAddr &= 0xFF;

    HWI2C_HAL_INFO("HWI2C IOMap base:%8x u32RegAddr:%8x u32RegAddr:%8x\n", _gMIO_MapBase[u8Port], u32RegAddr,(_gMIO_MapBase[u8Port]+((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2)));

    u16value = (*(volatile U32*)(_gMIO_MapBase[u8Port]+((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2)));
    HWI2C_HAL_INFO("u16value===:%4x\n", u16value);
    return ((u32RegAddr & 0xFF) % 2)? HIBYTE(u16value) : LOWBYTE(u16value);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Read4Byte
/// @brief \b Function  \b Description: read 2 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <OUT>        \b None :
/// @param <RET>        \b U16
////////////////////////////////////////////////////////////////////////////////
U16 HAL_HWI2C_Read2Byte(U32 u32RegAddr)
{
    U8 u8Port;

    HWI2C_HAL_FUNC();
    if(FALSE == HAL_HWI2C_GetPortIdxByOffset(u32RegAddr&(~0xFF), &u8Port))
        return FALSE;
    u32RegAddr &= 0xFF;

    HWI2C_HAL_INFO("HWI2C IOMap base:%8x u32RegAddr:%8x\n", _gMIO_MapBase[u8Port], u32RegAddr);
    return (*(volatile U32*)(_gMIO_MapBase[u8Port]+((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2)));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Read4Byte
/// @brief \b Function  \b Description: read 4 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <OUT>        \b None :
/// @param <RET>        \b U32
////////////////////////////////////////////////////////////////////////////////
U32 HAL_HWI2C_Read4Byte(U32 u32RegAddr)
{
    HWI2C_HAL_FUNC();

    return (HAL_HWI2C_Read2Byte(u32RegAddr) | HAL_HWI2C_Read2Byte(u32RegAddr+2) << 16);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_WriteByte
/// @brief \b Function  \b Description: write 1 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u8Val : 1 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_WriteByte(U32 u32RegAddr, U8 u8Val)
{
    U8 u8Port;

    HWI2C_HAL_FUNC();
    if(FALSE == HAL_HWI2C_GetPortIdxByOffset(u32RegAddr&(~0xFF), &u8Port))
        return FALSE;
    u32RegAddr &= 0xFF;

    HWI2C_HAL_INFO("HWI2C IOMap base:%8x u32RegAddr:%8x, u8Val:%2x\n", _gMIO_MapBase[u8Port], u32RegAddr, u8Val);

    if((u32RegAddr & 0xFF) % 2)
    {
        (*(volatile U8*)(_gMIO_MapBase[u8Port]+(u32RegAddr << 1) - 1)) = u8Val;
    }
    else
    {
        (*(volatile U8*)(_gMIO_MapBase[u8Port]+(u32RegAddr << 1) )) = u8Val;
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Write2Byte
/// @brief \b Function  \b Description: write 2 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u16Val : 2 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Write2Byte(U32 u32RegAddr, U16 u16Val)
{
    U8 u8Port;

    HWI2C_HAL_FUNC();
    if(FALSE == HAL_HWI2C_GetPortIdxByOffset(u32RegAddr&(~0xFF), &u8Port))
        return FALSE;
    u32RegAddr &= 0xFF;

//  HWI2C_HAL_INFO("HWI2C IOMap base:%16lx u32RegAddr:%4x\n", _gMIO_MapBase[u8Port], u16Val);
    HWI2C_HAL_INFO("HWI2C IOMap base:%8x u32RegAddr:%8x u16Val:%4x\n", _gMIO_MapBase[u8Port], u32RegAddr, u16Val);

    (*(volatile U32*)(_gMIO_MapBase[u8Port]+((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2))) = u16Val;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Write4Byte
/// @brief \b Function  \b Description: write 4 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u32Val : 4 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Write4Byte(U32 u32RegAddr, U32 u32Val)
{
    HWI2C_HAL_FUNC();

    HAL_HWI2C_Write2Byte(u32RegAddr, u32Val & 0x0000FFFF);
    HAL_HWI2C_Write2Byte(u32RegAddr+2, u32Val >> 16);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_WriteRegBit
/// @brief \b Function  \b Description: write 1 Byte data
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u8Val : 1 byte data
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_WriteRegBit(U32 u32RegAddr, U8 u8Mask, BOOL bEnable)
{
    U8 u8Val = 0;

    HWI2C_HAL_FUNC();

    u8Val = HAL_HWI2C_ReadByte(u32RegAddr);
    u8Val = (bEnable) ? (u8Val | u8Mask) : (u8Val & ~u8Mask);
    HAL_HWI2C_WriteByte(u32RegAddr, u8Val);
    HWI2C_HAL_INFO("read back u32RegAddr:%x\n", HAL_HWI2C_ReadByte(u32RegAddr));
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_WriteByteMask
/// @brief \b Function  \b Description: write data with mask bits
/// @param <IN>         \b u32RegAddr: register address
/// @param <IN>         \b u8Val : 1 byte data
/// @param <IN>         \b u8Mask : mask bits
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_WriteByteMask(U32 u32RegAddr, U8 u8Val, U8 u8Mask)
{
    HWI2C_HAL_FUNC();

    u8Val = (HAL_HWI2C_ReadByte(u32RegAddr) & ~u8Mask) | (u8Val & u8Mask);
    HAL_HWI2C_WriteByte(u32RegAddr, u8Val);
    return TRUE;
}


U8 HAL_HWI2C_ReadChipByte(U32 u32RegAddr)
{
    U16 u16value;

    HWI2C_HAL_FUNC();
    HWI2C_HAL_INFO("HWI2C IOMap chipbase:%8x u32RegAddr:%8x u32RegAddr:%8x\n", _gMChipIO_MapBase, u32RegAddr,(_gMChipIO_MapBase+((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2)));

    u16value = (*(volatile U32*)(_gMChipIO_MapBase+((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2)));
    HWI2C_HAL_INFO("u16value===:%4x\n", u16value);
    return ((u32RegAddr & 0xFF) % 2)? HIBYTE(u16value) : LOWBYTE(u16value);
}

BOOL HAL_HWI2C_WriteChipByte(U32 u32RegAddr, U8 u8Val)
{
    U16 u16value;


    HWI2C_HAL_FUNC();
    HWI2C_HAL_INFO("HWI2C IOMap chipbase:%8x u32RegAddr:%8x\n", _gMChipIO_MapBase, u32RegAddr);

    if((u32RegAddr & 0xFF) % 2)
    {
        u16value = (((U16)u8Val) << 8)|((*(volatile U32*)(_gMChipIO_MapBase+((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2))) & 0xFF);
    }
    else
    {
        u16value = ((U16)u8Val)|((*(volatile U32*)(_gMChipIO_MapBase+((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2))) & 0xFF00);
    }

    (*(volatile U32*)(_gMChipIO_MapBase+((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2))) = u16value;
    return TRUE;
}

BOOL HAL_HWI2C_WriteChipByteMask(U32 u32RegAddr, U8 u8Val, U8 u8Mask)
{

    HWI2C_HAL_FUNC();
    HWI2C_HAL_INFO("HWI2C IOMap chipbase:%8x u32RegAddr:%8x\n", _gMChipIO_MapBase, u32RegAddr);

    u8Val = (HAL_HWI2C_ReadChipByte(u32RegAddr) & ~u8Mask) | (u8Val & u8Mask);
    HAL_HWI2C_WriteChipByte(u32RegAddr, u8Val);
    return TRUE;
}

U16 HAL_HWI2C_ReadClk2Byte(U32 u32RegAddr)
{
    HWI2C_HAL_FUNC();
    HWI2C_HAL_INFO("HWI2C IOMap clkbase:%8x u32RegAddr:%8x\n", _gMClkIO_MapBase, u32RegAddr);
    return (*(volatile U32*)(_gMClkIO_MapBase+((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2)));
}

BOOL HAL_HWI2C_WriteClk2Byte(U32 u32RegAddr, U16 u16Val)
{
    HWI2C_HAL_FUNC();
    HWI2C_HAL_INFO("HWI2C IOMap clkbase:%8x u32RegAddr:%8x u32RegAddr:%4x\n", _gMClkIO_MapBase, u32RegAddr, u16Val);

    (*(volatile U32*)(_gMClkIO_MapBase+((u32RegAddr & 0xFFFFFF00) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2))) = u16Val;
    return TRUE;
}
BOOL HAL_HWI2C_WriteClkByteMask(U32 u32RegAddr, U16 u16Val, U16 u16Mask)
{
    HWI2C_HAL_FUNC();
    HWI2C_HAL_INFO("HWI2C IOMap clkbase:%8x u32RegAddr:%8x\n", _gMClkIO_MapBase, u32RegAddr);

    u16Val = (HAL_HWI2C_ReadClk2Byte(u32RegAddr) & ~u16Mask) | (u16Val & u16Mask);
    HAL_HWI2C_WriteClk2Byte(u32RegAddr, u16Val);
    return TRUE;
}
//#####################
//
//  MIIC STD Related Functions
//  Static or Internal use
//
//#####################
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_EnINT
/// @brief \b Function  \b Description: Enable Interrupt
/// @param <IN>         \b bEnable : TRUE: Enable, FALSE: Disable
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok, FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_EnINT(U16 u16PortOffset, BOOL bEnable)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_MIIC_CFG+u16PortOffset, _MIIC_CFG_EN_INT, TRUE);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_EnDMA
/// @brief \b Function  \b Description: Enable DMA
/// @param <IN>         \b bEnable : TRUE: Enable, FALSE: Disable
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok, FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_EnDMA(U16 u16PortOffset, BOOL bEnable)
{
    HWI2C_HAL_FUNC();
    //pr_err("[%s] u16PortOffset: %#x, bEnable: %#x \n", __func__, u16PortOffset, bEnable);
    //if(u16PortOffset == 0x100)
    //    bEnable = FALSE;
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_MIIC_CFG+u16PortOffset, _MIIC_CFG_EN_DMA, bEnable);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_EnClkStretch
/// @brief \b Function  \b Description: Enable Clock Stretch
/// @param <IN>         \b bEnable : TRUE: Enable, FALSE: Disable
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok, FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_EnClkStretch(U16 u16PortOffset, BOOL bEnable)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_MIIC_CFG+u16PortOffset, _MIIC_CFG_EN_CLKSTR, bEnable);
}

#if 0//RFU
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_EnTimeoutINT
/// @brief \b Function  \b Description: Enable Timeout Interrupt
/// @param <IN>         \b bEnable : TRUE: Enable, FALSE: Disable
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok, FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_EnTimeoutINT(U16 u16PortOffset, BOOL bEnable)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_MIIC_CFG+u16PortOffset, _MIIC_CFG_EN_TMTINT, bEnable);
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_EnFilter
/// @brief \b Function  \b Description: Enable Filter
/// @param <IN>         \b bEnable : TRUE: Enable, FALSE: Disable
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok, FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_EnFilter(U16 u16PortOffset, BOOL bEnable)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_MIIC_CFG+u16PortOffset, _MIIC_CFG_EN_FILTER, bEnable);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_EnPushSda
/// @brief \b Function  \b Description: Enable push current for SDA
/// @param <IN>         \b bEnable : TRUE: Enable, FALSE: Disable
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok, FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_EnPushSda(U16 u16PortOffset, BOOL bEnable)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_MIIC_CFG+u16PortOffset, _MIIC_CFG_EN_PUSH1T, bEnable);
}

//#####################
//
//  MIIC DMA Related Functions
//  Static or Internal use
//
//#####################
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetINT
/// @brief \b Function  \b Description: Initialize HWI2C DMA
/// @param <IN>         \b bEnable : TRUE: enable INT, FALSE: disable INT
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetINT(U16 u16PortOffset, BOOL bEnable)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CFG+u16PortOffset, _DMA_CFG_INTEN, bEnable);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_Reset
/// @brief \b Function  \b Description: Reset HWI2C DMA
/// @param <IN>         \b bReset : TRUE: Not Reset FALSE: Reset
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_Reset(U16 u16PortOffset, BOOL bReset)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CFG+u16PortOffset, _DMA_CFG_RESET, bReset);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_MiuReset
/// @brief \b Function  \b Description: Reset HWI2C DMA MIU
/// @param <IN>         \b bReset : TRUE: Not Reset FALSE: Reset
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_MiuReset(U16 u16PortOffset, BOOL bReset)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CFG+u16PortOffset, _DMA_CFG_MIURST, bReset);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetMiuPri
/// @brief \b Function  \b Description: Set HWI2C DMA MIU Priority
/// @param <IN>         \b eMiuPri : E_HAL_HWI2C_DMA_PRI_LOW, E_HAL_HWI2C_DMA_PRI_HIGH
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetMiuPri(U16 u16PortOffset, HAL_HWI2C_DMA_MIUPRI eMiuPri)
{
    BOOL bHighPri;

    HWI2C_HAL_FUNC();
    if(eMiuPri>=E_HAL_HWI2C_DMA_PRI_MAX)
        return FALSE;
    bHighPri = (eMiuPri==E_HAL_HWI2C_DMA_PRI_HIGH)? TRUE : FALSE;
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CFG+u16PortOffset, _DMA_CFG_MIUPRI, bHighPri);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetMiuAddr
/// @brief \b Function  \b Description: Set HWI2C DMA MIU Address
/// @param <IN>         \b u32MiuAddr : MIU Address
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetMiuAddr(U16 u16PortOffset, U32 u32MiuAddr)
{
    U8 u8Port;

    HWI2C_HAL_FUNC();

    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset,&u8Port)==FALSE)
        return FALSE;
    g_u32DmaPhyAddr[u8Port] = u32MiuAddr;

    /*Enable I2C DMA wait MIU done*/
    HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_RESERVED0+u16PortOffset, __BIT7|__BIT4, TRUE);

    return HAL_HWI2C_Write4Byte(REG_HWI2C_DMA_MIU_ADR+u16PortOffset, Chip_Phys_to_MIU(u32MiuAddr));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_Trigger
/// @brief \b Function  \b Description: Trigger HWI2C DMA
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_Trigger(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CTL_TRIG+u16PortOffset, _DMA_CTL_TRIG, TRUE);
}

#if 0 //will be used later
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_ReTrigger
/// @brief \b Function  \b Description: Re-Trigger HWI2C DMA
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_ReTrigger(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CTL+u16PortOffset, _DMA_CTL_RETRIG, TRUE);
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetTxfrStop
/// @brief \b Function  \b Description: Control HWI2C DMA Transfer Format with or w/o STOP
/// @param <IN>         \b bEnable : TRUE:  with STOP, FALSE: without STOP
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetTxfrStop(U16 u16PortOffset, BOOL bEnable)
{
    BOOL bTxNoStop;

    HWI2C_HAL_FUNC();
    bTxNoStop = (bEnable)? FALSE : TRUE;
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CTL+u16PortOffset, _DMA_CTL_TXNOSTOP, bTxNoStop);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetReadMode
/// @brief \b Function  \b Description: Control HWI2C DMA Transfer Format with or w/o STOP
/// @param <IN>         \b eReadMode : E_HAL_HWI2C_DMA_READ_NOSTOP, E_HAL_HWI2C_DMA_READ_STOP
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetReadMode(U16 u16PortOffset, HAL_HWI2C_ReadMode eReadMode)
{
    HWI2C_HAL_FUNC();
    if(eReadMode>=E_HAL_HWI2C_READ_MODE_MAX)
        return FALSE;
    if(eReadMode==E_HAL_HWI2C_READ_MODE_DIRECTION_CHANGE)
        return HAL_HWI2C_DMA_SetTxfrStop(u16PortOffset, FALSE);
    else
    if(eReadMode==E_HAL_HWI2C_READ_MODE_DIRECTION_CHANGE_STOP_START)
        return HAL_HWI2C_DMA_SetTxfrStop(u16PortOffset, TRUE);
    else
        return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetRdWrt
/// @brief \b Function  \b Description: Control HWI2C DMA Read or Write
/// @param <IN>         \b bRdWrt : TRUE: read ,FALSE: write
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetRdWrt(U16 u16PortOffset, BOOL bRdWrt)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CTL+u16PortOffset, _DMA_CTL_RDWTCMD, bRdWrt);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetMiuChannel
/// @brief \b Function  \b Description: Control HWI2C DMA MIU channel
/// @param <IN>         \b u8MiuCh : E_HAL_HWI2C_DMA_MIU_CH0 , E_HAL_HWI2C_DMA_MIU_CH1
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetMiuChannel(U16 u16PortOffset, HAL_HWI2C_DMA_MIUCH eMiuCh)
{
    BOOL bMiuCh1;

    HWI2C_HAL_FUNC();
    if(eMiuCh>=E_HAL_HWI2C_DMA_MIU_MAX)
        return FALSE;
    bMiuCh1 = (eMiuCh==E_HAL_HWI2C_DMA_MIU_CH1)? TRUE : FALSE;
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_CTL+u16PortOffset, _DMA_CTL_MIUCHSEL, bMiuCh1);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_TxfrDone
/// @brief \b Function  \b Description: Enable interrupt for HWI2C
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_TxfrDone(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_TXR+u16PortOffset, _DMA_TXR_DONE, TRUE);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_IsTxfrDone
/// @brief \b Function  \b Description: Check HWI2C DMA Tx done or not
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: DMA TX Done, FALSE: DMA TX Not Done
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_IsTxfrDone(U16 u16PortOffset, U8 u8Port)
{
    HWI2C_HAL_FUNC();
    //##########################
    //
    // [Note] : IMPORTANT !!!
    // Need to put some delay here,
    // Otherwise, reading data will fail
    //
    //##########################
    if(u8Port>=HAL_HWI2C_PORTS)
        return FALSE;
    HAL_HWI2C_ExtraDelay(g_u16DmaDelayFactor[u8Port]);
    return (HAL_HWI2C_ReadByte(REG_HWI2C_DMA_TXR+u16PortOffset) & _DMA_TXR_DONE) ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetTxfrCmd
/// @brief \b Function  \b Description: Set Transfer HWI2C DMA Command & Length
/// @param <IN>         \b pu8CmdBuf : data pointer
/// @param <IN>         \b u8CmdLen : command length
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: cmd len in range, FALSE: cmd len out of range
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetTxfrCmd(U16 u16PortOffset, U8 u8CmdLen, U8* pu8CmdBuf)
{
    U8 k,u8CmdData;
    U32 u32RegAdr;

    HWI2C_HAL_FUNC();
    if(u8CmdLen>HWI2C_DMA_CMD_DATA_LEN)
        return FALSE;
    for( k=0 ; (k<u8CmdLen)&&(k<HWI2C_DMA_CMD_DATA_LEN); k++ )
    {
        u32RegAdr = REG_HWI2C_DMA_CMDDAT0 + k;
        u8CmdData = *(pu8CmdBuf + k);
        HAL_HWI2C_WriteByte(u32RegAdr+u16PortOffset, u8CmdData);
    }
    HAL_HWI2C_WriteByte(REG_HWI2C_DMA_CMDLEN+u16PortOffset, u8CmdLen&_DMA_CMDLEN_MSK);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetCmdLen
/// @brief \b Function  \b Description: Set HWI2C DMA MIU command length
/// @param <IN>         \b u8CmdLen : command length
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetCmdLen(U16 u16PortOffset, U8 u8CmdLen)
{
    HWI2C_HAL_FUNC();
    if(u8CmdLen>HWI2C_DMA_CMD_DATA_LEN)
        return FALSE;
    HAL_HWI2C_WriteByte(REG_HWI2C_DMA_CMDLEN+u16PortOffset, u8CmdLen&_DMA_CMDLEN_MSK);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetDataLen
/// @brief \b Function  \b Description: Set HWI2C DMA data length
/// @param <IN>         \b u32DataLen : data length
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetDataLen(U16 u16PortOffset, U32 u32DataLen)
{
    U32 u32DataLenSet;

    HWI2C_HAL_FUNC();
    u32DataLenSet = u32DataLen;
    return HAL_HWI2C_Write4Byte(REG_HWI2C_DMA_DATLEN+u16PortOffset, u32DataLenSet);
}

#if 0 //will be used later
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_GetTxfrCnt
/// @brief \b Function  \b Description: Get MIIC DMA Transfer Count
/// @param <IN>         \b u32TxfrCnt : transfer count
/// @param <OUT>        \b None :
/// @param <RET>        \b None :
////////////////////////////////////////////////////////////////////////////////
static U32 HAL_HWI2C_DMA_GetTxfrCnt(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_Read4Byte(REG_HWI2C_DMA_TXFRCNT+u16PortOffset);
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_GetAddrMode
/// @brief \b Function  \b Description: Set MIIC DMA Slave Device Address length mode
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b E_HAL_HWI2C_DMA_ADDR_10BIT(10 bits mode),
///                                \b E_HAL_HWI2C_DMA_ADDR_NORMAL(7 bits mode)
////////////////////////////////////////////////////////////////////////////////
static HAL_HWI2C_DMA_ADDRMODE HAL_HWI2C_DMA_GetAddrMode(U16 u16PortOffset)
{
    HAL_HWI2C_DMA_ADDRMODE eAddrMode;

    HWI2C_HAL_FUNC();
    if(HAL_HWI2C_ReadByte(REG_HWI2C_DMA_SLVCFG+u16PortOffset) & _DMA_10BIT_MODE)
        eAddrMode = E_HAL_HWI2C_DMA_ADDR_10BIT;
    else
        eAddrMode = E_HAL_HWI2C_DMA_ADDR_NORMAL;
    return eAddrMode;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetSlaveAddr
/// @brief \b Function  \b Description: Set MIIC DMA Slave Device Address
/// @param <IN>         \b u32TxfrCnt : slave device address
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetSlaveAddr(U16 u16PortOffset, U16 u16SlaveAddr)
{
    HWI2C_HAL_FUNC();
    if(HAL_HWI2C_DMA_GetAddrMode(u16PortOffset)==E_HAL_HWI2C_DMA_ADDR_10BIT)
        return HAL_HWI2C_Write2Byte(REG_HWI2C_DMA_SLVADR+u16PortOffset, u16SlaveAddr&_DMA_SLVADR_10BIT_MSK);
    else
        return HAL_HWI2C_Write2Byte(REG_HWI2C_DMA_SLVADR+u16PortOffset, u16SlaveAddr&_DMA_SLVADR_NORML_MSK);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_SetAddrMode
/// @brief \b Function  \b Description: Set MIIC DMA Slave Device Address length mode
/// @param <IN>         \b eAddrMode : E_HAL_HWI2C_DMA_ADDR_NORMAL, E_HAL_HWI2C_DMA_ADDR_10BIT
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
static BOOL HAL_HWI2C_DMA_SetAddrMode(U16 u16PortOffset, HAL_HWI2C_DMA_ADDRMODE eAddrMode)
{
    BOOL b10BitMode;

    HWI2C_HAL_FUNC();
    if(eAddrMode>=E_HAL_HWI2C_DMA_ADDR_MAX)
        return FALSE;
    b10BitMode = (eAddrMode==E_HAL_HWI2C_DMA_ADDR_10BIT)? TRUE : FALSE;
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_DMA_SLVCFG+u16PortOffset, _DMA_10BIT_MODE, b10BitMode);
}

static BOOL HAL_HWI2C_DMA_SetMiuData(U16 u16PortOffset, U32 u32Length, U8* pu8SrcData)
{
    U32 u32PhyAddr = 0;
    U8 *pMiuData = 0;
    U8 u8Port;

    HWI2C_HAL_FUNC();
    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset,&u8Port)==FALSE)
        return FALSE;

	//pMiuData = (U8*)_PA2VA((U32)u32PhyAddr);
	//memcpy((void*)pMiuData,(void*)pu8SrcData,u32Length);

    u32PhyAddr = g_u32DmaPhyAddr[u8Port];
    pMiuData = HWI2C_DMA[u8Port].i2c_virt_addr;
    memcpy(pMiuData,pu8SrcData,u32Length);
    HAL_HWI2C_DMA_SetDataLen(u16PortOffset,u32Length);
    return TRUE;
}

static BOOL HAL_HWI2C_DMA_GetMiuData(U16 u16PortOffset, U32 u32Length, U8* pu8DstData)
{
    U32 u32PhyAddr = 0;
    U8 *pMiuData = 0;
    U8 u8Port;

    HWI2C_HAL_FUNC();

    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset,&u8Port)==FALSE)
        return FALSE;
    //u32PhyAddr = g_u32DmaPhyAddr[u8Port];
    //pMiuData = (U8*)_PA2VA((U32)u32PhyAddr);
    u32PhyAddr = g_u32DmaPhyAddr[u8Port];
    pMiuData = HWI2C_DMA[u8Port].i2c_virt_addr;
    memcpy((void*)pu8DstData,(void*)pMiuData,u32Length);
    return TRUE;
}

static BOOL HAL_HWI2C_DMA_WaitDone(U16 u16PortOffset, U8 u8ReadWrite)
{
    U16 volatile u16Timeout = HWI2C_DMA_WAIT_TIMEOUT;
    U8 u8Port;

    HWI2C_HAL_FUNC();

    //################
    //
    // IMPORTANT HERE !!!
    //
    //################
    //MsOS_FlushMemory();
    //(2-1) reset DMA engine
    HAL_HWI2C_DMA_Reset(u16PortOffset,TRUE);
    HAL_HWI2C_DMA_Reset(u16PortOffset,FALSE);
    //(2-2)  reset MIU module in DMA engine
    HAL_HWI2C_DMA_MiuReset(u16PortOffset,TRUE);
    HAL_HWI2C_DMA_MiuReset(u16PortOffset,FALSE);


    //get port index for delay factor
    if(FALSE == HAL_HWI2C_GetPortIdxByOffset(u16PortOffset,&u8Port))
    {
        // HWI2C_HAL_ERR("[DMA]: Get Port Idx By Offset Error!\n");
        return FALSE;
    }
    //clear transfer dine first for savfty
    HAL_HWI2C_DMA_TxfrDone(u16PortOffset);
    //set command : 0 for Write, 1 for Read
    HAL_HWI2C_DMA_SetRdWrt(u16PortOffset,u8ReadWrite);
    //issue write trigger
    HAL_HWI2C_DMA_Trigger(u16PortOffset);
    //check transfer done
    while(u16Timeout--)
    {
        if(HAL_HWI2C_DMA_IsTxfrDone(u16PortOffset,u8Port))
        {
            HAL_HWI2C_DMA_TxfrDone(u16PortOffset);
            HWI2C_HAL_INFO("[DMA]: Transfer DONE!\n");
            if(HAL_HWI2C_Get_SendAck(u16PortOffset)){
                return TRUE;
			} else {
				return FALSE;
			}
        }
    }
    HWI2C_HAL_ERR("[DMA]: Transfer NOT Completely!\n");
    return FALSE;
}

static BOOL HAL_HWI2C_DMA_SetDelayFactor(U16 u16PortOffset, HAL_HWI2C_CLKSEL eClkSel)
{
    U8 u8Port;

    HWI2C_HAL_FUNC();

    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset,&u8Port)==FALSE)
    {
        g_u16DmaDelayFactor[u8Port]=5;
        return FALSE;
    }
    switch(eClkSel)//use Xtal = 24M Hz
    {
        case E_HAL_HWI2C_CLKSEL_HIGH: // 400 KHz
            g_u16DmaDelayFactor[u8Port]=1; break;
        case E_HAL_HWI2C_CLKSEL_NORMAL: //300 KHz
            g_u16DmaDelayFactor[u8Port]=1; break;
        case E_HAL_HWI2C_CLKSEL_SLOW: //200 KHz
            g_u16DmaDelayFactor[u8Port]=1; break;
        case E_HAL_HWI2C_CLKSEL_VSLOW: //100 KHz
            g_u16DmaDelayFactor[u8Port]=2; break;
        case E_HAL_HWI2C_CLKSEL_USLOW: //50 KHz
            g_u16DmaDelayFactor[u8Port]=3; break;
        case E_HAL_HWI2C_CLKSEL_UVSLOW: //25 KHz
            g_u16DmaDelayFactor[u8Port]=3; break;
        default:
            g_u16DmaDelayFactor[u8Port]=5;
            return FALSE;
    }
    return TRUE;
}

//#####################
//
//  MIIC STD Related Functions
//  External
//
//#####################
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Init_Chip
/// @brief \b Function  \b Description: Init HWI2C chip
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Init_Chip(void)
{
    BOOL bRet = TRUE;

    HWI2C_HAL_FUNC();
    //not set all pads (except SPI) as input
    //bRet &= HAL_HWI2C_WriteRegBit(CHIP_REG_ALLPADIN, CHIP_ALLPAD_IN, FALSE);
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_IsMaster
/// @brief \b Function  \b Description: Check if Master I2C
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Master, FALSE: Slave
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_IsMaster(void)
{
    HWI2C_HAL_FUNC();
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Master_Enable
/// @brief \b Function  \b Description: Master I2C enable
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Master_Enable(U16 u16PortOffset)
{
    U8 u8Port=0;
    BOOL bRet = TRUE;
    HWI2C_HAL_FUNC();

    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset,&u8Port)==FALSE)
        return FALSE;
    g_bLastByte[u8Port] = FALSE;

    //(1) clear interrupt
    HAL_HWI2C_Clear_INT(u16PortOffset);
    //(2) reset standard master iic
    HAL_HWI2C_Reset(u16PortOffset,TRUE);
    HAL_HWI2C_Reset(u16PortOffset,FALSE);
    //(3) configuration
    HAL_HWI2C_EnINT(u16PortOffset,FALSE);
    HAL_HWI2C_EnClkStretch(u16PortOffset,TRUE);
    HAL_HWI2C_EnFilter(u16PortOffset,TRUE);
    HAL_HWI2C_EnPushSda(u16PortOffset,TRUE);
    #if 0
    HAL_HWI2C_EnTimeoutINT(u16PortOffset,TRUE);
    HAL_HWI2C_Write2Byte(REG_HWI2C_TMT_CNT+u16PortOffset, 0x100);
    #endif
    //(4) Disable DMA
    HAL_HWI2C_DMA_MiuReset(u16PortOffset,TRUE);
    HAL_HWI2C_DMA_Reset(u16PortOffset,TRUE);
    //bRet = HAL_HWI2C_DMA_Enable(u16PortOffset,FALSE);
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_SetPortRegOffset
/// @brief \b Function  \b Description: Set HWI2C port register offset
/// @param <IN>         \b ePort : HWI2C port number
/// @param <OUT>         \b pu16Offset : port register offset
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_SetPortRegOffset(U8 u8Port, U16* pu16Offset)
{
    HWI2C_HAL_FUNC();

    if(u8Port==HAL_HWI2C_PORT0)
    {//port 0 : bank register address 0x111800
        *pu16Offset = (U16)0x00;
    }
    else if(u8Port==HAL_HWI2C_PORT1)
    {//port 1 : bank register address 0x111900
        *pu16Offset = (U16)0x100;
    }
    else if(u8Port==HAL_HWI2C_PORT2)
    {//port 2 : bank register address 0x111A00
        *pu16Offset = (U16)0x200;
    }
    else if(u8Port==HAL_HWI2C_PORT3)
    {//port 3 : bank register address 0x111B00
        *pu16Offset = (U16)0x300;
    }
    else
    {
        *pu16Offset = (U16)0x00;
        return FALSE;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_GetPortIdxByRegOffset
/// @brief \b Function  \b Description: Get HWI2C port index by register offset
/// @param <IN>         \b u16Offset : port register offset
/// @param <OUT>         \b pu8Port :  port index
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_GetPortIdxByOffset(U16 u16Offset, U8* pu8Port)
{
    HWI2C_HAL_FUNC();

    if(u16Offset==(U16)0x00)
    {//port 0 : bank register address 0x11800
        *pu8Port = HAL_HWI2C_PORT0;
    }
    else if(u16Offset==(U16)0x100)
    {//port 1 : bank register address 0x11900
        *pu8Port = HAL_HWI2C_PORT1;
    }
    else if(u16Offset==(U16)0x200)
    {//port 2 : bank register address 0x11A00
        *pu8Port = HAL_HWI2C_PORT2;
    }
    else if(u16Offset==(U16)0x300)
    {//port 3 : bank register address 0x11B00
        *pu8Port = HAL_HWI2C_PORT3;
    }
    else
    {
        *pu8Port = HAL_HWI2C_PORT0;
        return FALSE;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_GetPortIdxByPort
/// @brief \b Function  \b Description: Get HWI2C port index by port number
/// @param <IN>         \b ePort : port number
/// @param <OUT>         \b pu8Port :  port index
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_GetPortIdxByPort(HAL_HWI2C_PORT ePort, U8* pu8Port)
{
    HWI2C_HAL_FUNC();

    if((ePort>=E_HAL_HWI2C_PORT0_0)&&(ePort<=E_HAL_HWI2C_PORT0_7))
    {
        *pu8Port = HAL_HWI2C_PORT0;
    }
    else if((ePort>=E_HAL_HWI2C_PORT1_0)&&(ePort<=E_HAL_HWI2C_PORT1_7))
    {
        *pu8Port = HAL_HWI2C_PORT1;
    }
    else if((ePort>=E_HAL_HWI2C_PORT2_0)&&(ePort<=E_HAL_HWI2C_PORT2_7))
    {
        *pu8Port = HAL_HWI2C_PORT2;
    }
    else if((ePort>=E_HAL_HWI2C_PORT3_0)&&(ePort<=E_HAL_HWI2C_PORT3_7))
    {
        *pu8Port = HAL_HWI2C_PORT3;
    }
    else
    {
        *pu8Port = HAL_HWI2C_PORT0;
        return FALSE;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_SelectPort
/// @brief \b Function  \b Description: Select HWI2C port
/// @param <IN>         \b None : HWI2C port
/// @param param        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_SelectPort(HAL_HWI2C_PORT ePort)
{
    U32 u32RegAddr = CHIP_REG_HWI2C_MIIC0;
    U8  u8Val = CHIP_MIIC0_PAD_1;
    U8  u8Mask = CHIP_MIIC0_PAD_MSK;

    HWI2C_HAL_FUNC();

    if((ePort >= E_HAL_HWI2C_PORT0_0) && (ePort <= E_HAL_HWI2C_PORT0_7))
    {
        u32RegAddr = CHIP_REG_HWI2C_MIIC0;
        u8Mask = CHIP_MIIC0_PAD_MSK;
    }
    else if((ePort >= E_HAL_HWI2C_PORT1_0) && (ePort <= E_HAL_HWI2C_PORT1_7))
    {
        u32RegAddr = CHIP_REG_HWI2C_MIIC1;
        u8Mask = CHIP_MIIC1_PAD_MSK;
    }
    else if((ePort >= E_HAL_HWI2C_PORT2_0) && (ePort <= E_HAL_HWI2C_PORT2_7))
    {
        u32RegAddr = CHIP_REG_HWI2C_MIIC2;
        u8Mask = CHIP_MIIC2_PAD_MSK;
    }
    else if((ePort >= E_HAL_HWI2C_PORT3_0) && (ePort <= E_HAL_HWI2C_PORT3_7))
    {
        u32RegAddr = CHIP_REG_HWI2C_MIIC3;
        u8Mask = CHIP_MIIC3_PAD_MSK;
    }

    switch(ePort) {
    case E_HAL_HWI2C_PORT0_0: u8Val = CHIP_MIIC0_PAD_0; break;
    case E_HAL_HWI2C_PORT0_1: u8Val = CHIP_MIIC0_PAD_1; break;
    case E_HAL_HWI2C_PORT0_2: u8Val = CHIP_MIIC0_PAD_2; break;
    case E_HAL_HWI2C_PORT0_3: u8Val = CHIP_MIIC0_PAD_3; break;
    case E_HAL_HWI2C_PORT0_4: u8Val = CHIP_MIIC0_PAD_4; break;
    case E_HAL_HWI2C_PORT1_0: u8Val = CHIP_MIIC1_PAD_0; break;
    case E_HAL_HWI2C_PORT1_1: u8Val = CHIP_MIIC1_PAD_1; break;
    case E_HAL_HWI2C_PORT1_2: u8Val = CHIP_MIIC1_PAD_2; break;
    case E_HAL_HWI2C_PORT1_3: u8Val = CHIP_MIIC1_PAD_3; break;
    case E_HAL_HWI2C_PORT1_4: u8Val = CHIP_MIIC1_PAD_4; break;
    case E_HAL_HWI2C_PORT1_5: u8Val = CHIP_MIIC1_PAD_5; break;
    case E_HAL_HWI2C_PORT2_0: u8Val = CHIP_MIIC2_PAD_0; break;
    case E_HAL_HWI2C_PORT2_1: u8Val = CHIP_MIIC2_PAD_1; break;
    case E_HAL_HWI2C_PORT2_2: u8Val = CHIP_MIIC2_PAD_2; break;
    case E_HAL_HWI2C_PORT2_3: u8Val = CHIP_MIIC2_PAD_3; break;
    case E_HAL_HWI2C_PORT3_0: u8Val = CHIP_MIIC3_PAD_0; break;
    case E_HAL_HWI2C_PORT3_1: u8Val = CHIP_MIIC3_PAD_1; break;
    case E_HAL_HWI2C_PORT3_2: u8Val = CHIP_MIIC3_PAD_2; break;
    case E_HAL_HWI2C_PORT3_3: u8Val = CHIP_MIIC3_PAD_3; break;
    case E_HAL_HWI2C_PORT3_4: u8Val = CHIP_MIIC3_PAD_4; break;
    case E_HAL_HWI2C_PORT3_5: u8Val = CHIP_MIIC3_PAD_5; break;
    default:
        HWI2C_HAL_ERR("[%s]: Port(%d) not support\n", __func__, ePort);
        return FALSE;
    }

    HAL_HWI2C_WriteChipByteMask(u32RegAddr, u8Val, u8Mask);

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_SetClk
/// @brief \b Function  \b Description: Set I2C clock
/// @param <IN>         \b u8Clk: clock rate
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_SetClk(U16 u16PortOffset, HAL_HWI2C_CLKSEL eClkSel)
{
    U16 u16ClkHCnt=0,u16ClkLCnt=0;
    U16 u16StpCnt=0,u16SdaCnt=0,u16SttCnt=0,u16LchCnt=0;

    HWI2C_HAL_FUNC();

    if(eClkSel>=E_HAL_HWI2C_CLKSEL_NOSUP)
        return FALSE;

    switch(eClkSel)//use Xtal = 12M Hz
    {
        case E_HAL_HWI2C_CLKSEL_HIGH: // 400 KHz
            u16ClkHCnt =   9; u16ClkLCnt =   13; break;
        case E_HAL_HWI2C_CLKSEL_NORMAL: //300 KHz
            u16ClkHCnt =  15; u16ClkLCnt =   17; break;
        case E_HAL_HWI2C_CLKSEL_SLOW: //200 KHz
            u16ClkHCnt =  25; u16ClkLCnt =   27; break;
        case E_HAL_HWI2C_CLKSEL_VSLOW: //100 KHz
            u16ClkHCnt =  55; u16ClkLCnt =   57; break;
        case E_HAL_HWI2C_CLKSEL_USLOW: //50 KHz
            u16ClkHCnt =  115; u16ClkLCnt = 117; break;
        case E_HAL_HWI2C_CLKSEL_UVSLOW: //25 KHz
            u16ClkHCnt =  235; u16ClkLCnt = 237; break;
        default:
            u16ClkHCnt =  15; u16ClkLCnt =  17; break;
    }
    u16SttCnt=38; u16StpCnt=38; u16SdaCnt=5; u16LchCnt=5;

    HAL_HWI2C_Write2Byte(REG_HWI2C_CKH_CNT+u16PortOffset, u16ClkHCnt);
    HAL_HWI2C_Write2Byte(REG_HWI2C_CKL_CNT+u16PortOffset, u16ClkLCnt);
    HAL_HWI2C_Write2Byte(REG_HWI2C_STP_CNT+u16PortOffset, u16StpCnt);
    HAL_HWI2C_Write2Byte(REG_HWI2C_SDA_CNT+u16PortOffset, u16SdaCnt);
    HAL_HWI2C_Write2Byte(REG_HWI2C_STT_CNT+u16PortOffset, u16SttCnt);
    HAL_HWI2C_Write2Byte(REG_HWI2C_LTH_CNT+u16PortOffset, u16LchCnt);
    //HAL_HWI2C_Write2Byte(REG_HWI2C_TMT_CNT+u16PortOffset, 0x0000);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Start
/// @brief \b Function  \b Description: Send start condition
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Start(U16 u16PortOffset)
{
    U16 u16Count = HWI2C_HAL_WAIT_TIMEOUT;

    HWI2C_HAL_FUNC();
    //reset I2C
    HAL_HWI2C_WriteRegBit(REG_HWI2C_CMD_START+u16PortOffset, _CMD_START, TRUE);
    while((!HAL_HWI2C_Is_INT(u16PortOffset))&&(u16Count > 0))
        u16Count--;
    //udelay(5);
    HAL_HWI2C_Clear_INT(u16PortOffset);
    return (u16Count)? TRUE:FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Stop
/// @brief \b Function  \b Description: Send Stop condition
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Stop(U16 u16PortOffset)
{
    U16 u16Count = HWI2C_HAL_WAIT_TIMEOUT;

    HWI2C_HAL_FUNC();
    //udelay(5);
    HAL_HWI2C_WriteRegBit(REG_HWI2C_CMD_STOP+u16PortOffset, _CMD_STOP, TRUE);
    while((!HAL_HWI2C_Is_Idle(u16PortOffset))&&(!HAL_HWI2C_Is_INT(u16PortOffset))&&(u16Count > 0))
        u16Count--;
    HAL_HWI2C_Clear_INT(u16PortOffset);
    return (u16Count)? TRUE:FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_ReadRdy
/// @brief \b Function  \b Description: Start byte reading
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_ReadRdy(U16 u16PortOffset)
{
    U8 u8Value=0;
    U8 u8Port;

    HWI2C_HAL_FUNC();

    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset,&u8Port)==FALSE)
        return FALSE;
    u8Value = (g_bLastByte[u8Port])? (_RDATA_CFG_TRIG|_RDATA_CFG_ACKBIT) : (_RDATA_CFG_TRIG);
    g_bLastByte[u8Port] = FALSE;
    return HAL_HWI2C_WriteByte(REG_HWI2C_RDATA_CFG+u16PortOffset, u8Value);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_SendData
/// @brief \b Function  \b Description: Send 1 byte data to SDA
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_SendData(U16 u16PortOffset, U8 u8Data)
{
    HWI2C_HAL_FUNC();

    return HAL_HWI2C_WriteByte(REG_HWI2C_WDATA+u16PortOffset, u8Data);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_RecvData
/// @brief \b Function  \b Description: Receive 1 byte data from SDA
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b U8 :
////////////////////////////////////////////////////////////////////////////////
U8 HAL_HWI2C_RecvData(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();

    return HAL_HWI2C_ReadByte(REG_HWI2C_RDATA+u16PortOffset);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Get_SendAck
/// @brief \b Function  \b Description: Get ack after sending data
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Valid ack, FALSE: No ack
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Get_SendAck(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();
    return (HAL_HWI2C_ReadByte(REG_HWI2C_WDATA_GET+u16PortOffset) & _WDATA_GET_ACKBIT) ? FALSE : TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_NoAck
/// @brief \b Function  \b Description: generate no ack pulse
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_NoAck(U16 u16PortOffset)
{
    U8 u8Port;

    HWI2C_HAL_FUNC();

    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset,&u8Port)==FALSE)
        return FALSE;
    g_bLastByte[u8Port] = TRUE;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Ack
/// @brief \b Function  \b Description: generate ack pulse
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Ack(U16 u16PortOffset)
{
    U8 u8Port;

    HWI2C_HAL_FUNC();

    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset,&u8Port)==FALSE)
        return FALSE;
    g_bLastByte[u8Port] = FALSE;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_GetStae
/// @brief \b Function  \b Description: Get i2c Current State
/// @param <IN>         \b u16PortOffset: HWI2C Port Offset
/// @param <OUT>        \b None
/// @param <RET>        \b HWI2C current status
////////////////////////////////////////////////////////////////////////////////
U8 HAL_HWI2C_GetState(U16 u16PortOffset)
{

    U8 cur_state =  HAL_HWI2C_ReadByte(REG_HWI2C_CUR_STATE+u16PortOffset) & _CUR_STATE_MSK;
    HWI2C_HAL_FUNC();

    if (cur_state <= 0) // 0: idle
        return E_HAL_HWI2C_STATE_IDEL;
    else if (cur_state <= 2) // 1~2:start
        return E_HAL_HWI2C_STATE_START;
    else if (cur_state <= 6) // 3~6:write
        return E_HAL_HWI2C_STATE_WRITE;
    else if (cur_state <= 10) // 7~10:read
        return E_HAL_HWI2C_STATE_READ;
    else if (cur_state <= 11) // 11:interrupt
        return E_HAL_HWI2C_STATE_INT;
    else if (cur_state <= 12) // 12:wait
        return E_HAL_HWI2C_STATE_WAIT;
    else  // 13~15:stop
        return E_HAL_HWI2C_STATE_STOP;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Is_Idle
/// @brief \b Function  \b Description: Check if i2c is idle
/// @param <IN>         \b u16PortOffset: HWI2C Port Offset
/// @param <OUT>        \b None
/// @param <RET>        \b TRUE : idle, FALSE : not idle
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Is_Idle(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();

    return ((HAL_HWI2C_GetState(u16PortOffset)==E_HAL_HWI2C_STATE_IDEL) ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Is_INT
/// @brief \b Function  \b Description: Check if i2c is interrupted
/// @param <IN>         \b u8Status : queried status
/// @param <IN>         \b u8Ch: Channel 0/1
/// @param <OUT>        \b None
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Is_INT(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();
    return (HAL_HWI2C_ReadByte(REG_HWI2C_INT_CTL+u16PortOffset) & _INT_CTL) ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Clear_INT
/// @brief \b Function  \b Description: Enable interrupt for HWI2C
/// @param <IN>         \b None :
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Clear_INT(U16 u16PortOffset)
{
    HWI2C_HAL_FUNC();

    return HAL_HWI2C_WriteRegBit(REG_HWI2C_INT_CTL+u16PortOffset, _INT_CTL, TRUE);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Reset
/// @brief \b Function  \b Description: Reset HWI2C state machine
/// @param <IN>         \b bReset : TRUE: Reset FALSE: Not reset
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE: Ok, FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Reset(U16 u16PortOffset, BOOL bReset)
{
    HWI2C_HAL_FUNC();
    return HAL_HWI2C_WriteRegBit(REG_HWI2C_MIIC_CFG+u16PortOffset, _MIIC_CFG_RESET, bReset);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Send_Byte
/// @brief \b Function  \b Description: Send one byte
/// @param u8Data       \b IN: 1 byte data
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Send_Byte(U16 u16PortOffset, U8 u8Data)
{
    U8 u8Retry = HWI2C_HAL_RETRY_TIMES;
    U32 u16Count = HWI2C_HAL_WAIT_TIMEOUT;

    HWI2C_HAL_FUNC();
    //HWI2C_HAL_ERR("Send byte 0x%X !\n", u8Data);

    while(u8Retry--)
    {
        HAL_HWI2C_Clear_INT(u16PortOffset);
        if (HAL_HWI2C_SendData(u16PortOffset,u8Data))
        {
            u16Count = HWI2C_HAL_WAIT_TIMEOUT;
            while(u16Count--)
            {
                if (HAL_HWI2C_Is_INT(u16PortOffset))
                {
                    HAL_HWI2C_Clear_INT(u16PortOffset);
                    if (HAL_HWI2C_Get_SendAck(u16PortOffset))
                    {
                        #if 1
                        HAL_HWI2C_ExtraDelay(1);
                        #else
                        MsOS_DelayTaskUs(1);
                        #endif
                        return TRUE;
                    }
                    //break;
                }
            }
        }
		//pr_err("REG_HWI2C_INT_STATUS %#x\n", HAL_HWI2C_ReadByte(REG_HWI2C_INT_STATUS+u16PortOffset));
		//pr_err("REG_HWI2C_CUR_STATE %#x\n", HAL_HWI2C_ReadByte(REG_HWI2C_CUR_STATE+u16PortOffset));
        //MDrv_GPIO_Set_High(72);
		// check if in Idle state
		if(HAL_HWI2C_ReadByte(REG_HWI2C_CUR_STATE+u16PortOffset)==0)
		{
			//pr_err("## START bit");
			HAL_HWI2C_Start(u16PortOffset);
			udelay(2);
		} else {
			//pr_err("REG_HWI2C_CUR_STATE %#x\n", HAL_HWI2C_ReadByte(REG_HWI2C_CUR_STATE+u16PortOffset));
			HAL_HWI2C_Stop(u16PortOffset);
			udelay(2);
			// reset I2C IP
			HAL_HWI2C_Reset(u16PortOffset,TRUE);
			HAL_HWI2C_Reset(u16PortOffset,FALSE);
			udelay(2);
			HAL_HWI2C_Start(u16PortOffset);
			udelay(2);
		}
        //MDrv_GPIO_Set_Low(72);
    }
    //HWI2C_HAL_ERR("Send byte 0x%X fail!\n", u8Data);
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Recv_Byte
/// @brief \b Function  \b Description: Init HWI2C driver and auto generate ACK
/// @param *pData       \b Out: received data
/// @return             \b TRUE: Success FALSE: Fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_Recv_Byte(U16 u16PortOffset, U8 *pData)
{
    U16 u16Count = HWI2C_HAL_WAIT_TIMEOUT;

    HWI2C_HAL_FUNC();

    if (!pData)
        return FALSE;

    HAL_HWI2C_ReadRdy(u16PortOffset);
    while((!HAL_HWI2C_Is_INT(u16PortOffset))&&(u16Count > 0))
        u16Count--;
    HAL_HWI2C_Clear_INT(u16PortOffset);
    if (u16Count)
    {
        //get data before clear int and stop
        *pData = HAL_HWI2C_RecvData(u16PortOffset);
        HWI2C_HAL_INFO("Recv byte =%x\n",*pData);
        //clear interrupt
        HAL_HWI2C_Clear_INT(u16PortOffset);
        #if 1
        HAL_HWI2C_ExtraDelay(1);
        #else
        MsOS_DelayTaskUs(1);
        #endif
        return TRUE;
    }
    HWI2C_HAL_INFO("Recv byte fail!\n");
    return FALSE;
}

//#####################
//
//  MIIC DMA Related Functions
//  External
//
//#####################
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_Enable
/// @brief \b Function  \b Description: Enable HWI2C DMA
/// @param <IN>         \b bEnable : TRUE: enable DMA, FALSE: disable DMA
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_DMA_Enable(U16 u16PortOffset, BOOL bEnable)
{
    BOOL bRet=TRUE;

    HWI2C_HAL_FUNC();

    bRet &= HAL_HWI2C_DMA_SetINT(u16PortOffset,bEnable);
    bRet &= HAL_HWI2C_EnDMA(u16PortOffset,bEnable);
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_Init
/// @brief \b Function  \b Description: Initialize HWI2C DMA
/// @param <IN>         \b pstCfg : Init structure
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_DMA_Init(U16 u16PortOffset, HAL_HWI2C_PortCfg* pstPortCfg)
{
    U8 u8Port = 0;
    BOOL bRet=TRUE;

    HWI2C_HAL_FUNC();

    //check pointer
    if(!pstPortCfg)
    {
        // HWI2C_HAL_ERR("Port cfg null pointer!\n");
        return FALSE;
    }
    //(1) clear interrupt
    HAL_HWI2C_DMA_TxfrDone(u16PortOffset);
    //(2) reset DMA
    //(2-1) reset DMA engine
    HAL_HWI2C_DMA_Reset(u16PortOffset,TRUE);
    HAL_HWI2C_DMA_Reset(u16PortOffset,FALSE);
    //(2-2)  reset MIU module in DMA engine
    HAL_HWI2C_DMA_MiuReset(u16PortOffset,TRUE);
    HAL_HWI2C_DMA_MiuReset(u16PortOffset,FALSE);
    //(3) default configursation
    bRet &= HAL_HWI2C_DMA_SetAddrMode(u16PortOffset,pstPortCfg->eDmaAddrMode);
    bRet &= HAL_HWI2C_DMA_SetMiuPri(u16PortOffset,pstPortCfg->eDmaMiuPri);
    bRet &= HAL_HWI2C_DMA_SetMiuChannel(u16PortOffset,pstPortCfg->eDmaMiuCh);
    bRet &= HAL_HWI2C_DMA_SetMiuAddr(u16PortOffset,pstPortCfg->u32DmaPhyAddr);
    bRet &= HAL_HWI2C_DMA_Enable(u16PortOffset,pstPortCfg->bDmaEnable);
    bRet &= HAL_HWI2C_DMA_SetDelayFactor(u16PortOffset,pstPortCfg->eSpeed);
    //(4) backup configuration info
    if(HAL_HWI2C_GetPortIdxByOffset(u16PortOffset,&u8Port))
    {
        memcpy(&g_stPortCfg[u8Port], pstPortCfg, sizeof(HAL_HWI2C_PortCfg));
    }

    return bRet;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_WriteBytes
/// @brief \b Function  \b Description: Initialize HWI2C DMA
/// @param <IN>         \b u16SlaveCfg : slave id
/// @param <IN>         \b uAddrCnt : address size in bytes
/// @param <IN>         \b pRegAddr : address pointer
/// @param <IN>         \b uSize : data size in bytes
/// @param <IN>         \b pData : data pointer
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_DMA_WriteBytes(U16 u16PortOffset, U16 u16SlaveCfg, U32 uAddrCnt, U8 *pRegAddr, U32 uSize, U8 *pData)
{
    U8 u8SlaveAddr = LOW_BYTE(u16SlaveCfg)>>1;

    HWI2C_HAL_FUNC();

    if (!pRegAddr)
    {
        // HWI2C_HAL_ERR("[DMA_W]: Null address!\n");
        return FALSE;
    }
    if (!pData)
    {
        // HWI2C_HAL_ERR("[DMA_W]: No data for writing!\n");
        return FALSE;
    }

    //set transfer with stop
    HAL_HWI2C_DMA_SetTxfrStop(u16PortOffset,TRUE);
    //set slave address
    HAL_HWI2C_DMA_SetSlaveAddr(u16PortOffset,u8SlaveAddr);

    //#################
    //  Set WRITE command if length 0 , cmd buffer will not be used
    //#################
    //set command buffer
    if(HAL_HWI2C_DMA_SetTxfrCmd(u16PortOffset,(U8)uAddrCnt,pRegAddr)==FALSE)
    {
        // HWI2C_HAL_ERR("[DMA_W]: Set command buffer error!\n");
        return FALSE;
    }
    //set data to dram
    if(HAL_HWI2C_DMA_SetMiuData(u16PortOffset,0,pData)==FALSE)
    {
        // HWI2C_HAL_ERR("[DMA_W]: Set MIU data error!\n");
        return FALSE;
    }
    //##################
    //  Trigger to WRITE
    if(HAL_HWI2C_DMA_WaitDone(u16PortOffset,HWI2C_DMA_WRITE)==FALSE)
    {
        //HWI2C_HAL_ERR("[DMA_W]: Transfer command error!\n");
        return FALSE;
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_DMA_ReadBytes
/// @brief \b Function  \b Description: Initialize HWI2C DMA
/// @param <IN>         \b u16SlaveCfg : slave id
/// @param <IN>         \b uAddrCnt : address size in bytes
/// @param <IN>         \b pRegAddr : address pointer
/// @param <IN>         \b uSize : data size in bytes
/// @param <IN>         \b pData : data pointer
/// @param <OUT>        \b None :
/// @param <RET>        \b TRUE : ok, FALSE : fail
////////////////////////////////////////////////////////////////////////////////
BOOL HAL_HWI2C_DMA_ReadBytes(U16 u16PortOffset, U16 u16SlaveCfg, U32 uAddrCnt, U8 *pRegAddr, U32 uSize, U8 *pData)
{
    U8 u8SlaveAddr = LOW_BYTE(u16SlaveCfg)>>1;
    U8 u8Port = HIGH_BYTE(u16SlaveCfg);
    HAL_HWI2C_ReadMode eReadMode;

    HWI2C_HAL_FUNC();

    if (!pRegAddr)
    {
        // HWI2C_HAL_ERR("[DMA_R]: Null address!\n");
        return FALSE;
    }
    if (!pData)
    {
        // HWI2C_HAL_ERR("[DMA_R]: No data for reading!\n");
        return FALSE;
    }
    if (u8Port>=HAL_HWI2C_PORTS)
    {
        // HWI2C_HAL_ERR("[DMA_R]: Port failure!\n");
        return FALSE;
    }

    eReadMode = g_stPortCfg[u8Port].eReadMode;
    if(eReadMode>=E_HAL_HWI2C_READ_MODE_MAX)
    {
        // HWI2C_HAL_ERR("[DMA_R]: Read mode failure!\n");
        return FALSE;
    }

    if(eReadMode!=E_HAL_HWI2C_READ_MODE_DIRECT)
    {
        //set transfer read mode
        HAL_HWI2C_DMA_SetReadMode(u16PortOffset,eReadMode);
        //set slave address
        HAL_HWI2C_DMA_SetSlaveAddr(u16PortOffset,u8SlaveAddr);

        //#################
        //  Set WRITE command
        //#################
        //set command buffer
        if(HAL_HWI2C_DMA_SetTxfrCmd(u16PortOffset,(U8)uAddrCnt,pRegAddr)==FALSE)
        {
            // HWI2C_HAL_ERR("[DMA_R:W]: Set command buffer error!\n");
            return FALSE;
        }
        HAL_HWI2C_DMA_SetDataLen(u16PortOffset,0);

        //##################
        //  Trigger to WRITE
        if(HAL_HWI2C_DMA_WaitDone(u16PortOffset,HWI2C_DMA_WRITE)==FALSE)
        {
            HWI2C_HAL_ERR("[DMA_R:W]: Transfer command error!\n");
            return FALSE;
        }
    }


    //#################
    //  Set READ command
    //#################
    //set transfer with stop
    HAL_HWI2C_DMA_SetTxfrStop(u16PortOffset,TRUE);
    //set slave address
    HAL_HWI2C_DMA_SetSlaveAddr(u16PortOffset,u8SlaveAddr);
    //set command length to 0
    HAL_HWI2C_DMA_SetCmdLen(u16PortOffset,0);
    //set command length for reading
    HAL_HWI2C_DMA_SetDataLen(u16PortOffset,uSize);
    //##################
    //  Trigger to READ
    if(HAL_HWI2C_DMA_WaitDone(u16PortOffset,HWI2C_DMA_READ)==FALSE)
    {
        HWI2C_HAL_ERR("[DMA_R:R]: Transfer command error!\n");
        return FALSE;
    }
    //get data to dram
    if(HAL_HWI2C_DMA_GetMiuData(u16PortOffset,uSize,pData)==FALSE)
    {
        // HWI2C_HAL_ERR("[DMA_R:R]: Get MIU data error!\n");
        return FALSE;
    }

    return TRUE;
}

//#####################
//
//  MIIC Miscellaneous Functions
//
//#####################
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: HAL_HWI2C_Init_ExtraProc
/// @brief \b Function  \b Description: Do extral procedure after initialization
/// @param <IN>         \b None :
/// @param param        \b None :
/// @param <RET>        \b None :
////////////////////////////////////////////////////////////////////////////////
void HAL_HWI2C_Init_ExtraProc(void)
{
    HWI2C_HAL_FUNC();
    //Extra procedure TODO
}

BOOL HAL_HWI2C_CheckAbility(HAL_HWI2C_HW_FEATURE etype,mhal_i2c_feature_fp *fp)
{
	return FALSE;
}
void HAL_HWI2C_IrqFree(U32 u32irq)
{
	return;
}
void HAL_HWI2C_IrqRequest(U32 u32irq, U32 u32group, void *pdev)
{
	return;
}
void HAL_HWI2C_DMA_TsemInit(U8 u8Port)
{
	return;
}
void HAL_HWI2C_DMA_TsemDeinit(U8 u8Port)
{
	return;
}

#endif
