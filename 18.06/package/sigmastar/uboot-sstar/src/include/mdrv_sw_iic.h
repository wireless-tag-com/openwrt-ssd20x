/*
* mdrv_sw_iic.h- Sigmastar
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

#ifndef _DRV_IIC_H_
#define _DRV_IIC_H_

#include <MsTypes.h>

//-------------------------------------------------------------------------------------------------
//  Driver Capability
//-------------------------------------------------------------------------------------------------
#define SW_IIC_NUM_OF_MAX    (5)


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
#define SW_IIC_SPEED_400KHZ  4
#define SW_IIC_SPEED_300KHZ  3
#define SW_IIC_SPEED_200KHZ  2
#define SW_IIC_SPEED_100KHZ  1

struct I2C_BusCfg
{
    U8 u8ChIdx;         ///Channel index
    U8 u8PadSCL;        ///Pad(Gpio) number for SCL
    U8 u8PadSDA;        ///Pad(Gpio) number for SDA
    U16 u16SpeedKHz;    ///Speed in KHz
    U8 u8Enable;        ///Enable
} ;
//} __attribute__ ((packed));

typedef struct I2C_BusCfg I2C_BusCfg_t;
extern int gpioi2c_delay_us;
extern int access_dummy_time;

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
// for software IIC
void MDrv_SW_IIC_SetSpeed(U8 u8ChIIC, U8 u8Speed);
void MDrv_SW_IIC_Delay(U8 u8ChIIC);
void MDrv_SW_IIC_SCL(U8 u8ChIIC, U8 u8Data);
void MDrv_SW_IIC_SDA(U8 u8ChIIC, U8 u8Data);
void MDrv_SW_IIC_SCL_Chk(U8 u8ChIIC, U16 bSet);
void MDrv_SW_IIC_SDA_Chk(U8 u8ChIIC, U16 bSet);
U16 MDrv_SW_IIC_Start(U8 u8ChIIC);
void MDrv_SW_IIC_Stop(U8 u8ChIIC);
U16 MDrv_SW_IIC_SendByte(U8 u8ChIIC, U8 u8data, U8 u8Delay4Ack);
U16 MDrv_SW_IIC_AccessStart(U8 u8ChIIC, U8 u8SlaveAdr, U8 u8Trans);
U8 MDrv_SW_IIC_GetByte (U8 u8ChIIC, U16 u16Ack);
S32 MDrv_SW_IIC_Write(U8 u8ChIIC, U8 u8SlaveID, U8 u8AddrCnt, U8* pu8Addr, U32 u32BufLen, U8* pu8Buf);
S32 MDrv_SW_IIC_Read(U8 u8ChIIC, U8 u8SlaveID, U8 u8AddrCnt, U8* pu8Addr, U32 u32BufLen, U8* pu8Buf);
void MDrv_SW_IIC_Enable( U8 u8ChIIC, U8 bEnable );

U16 MDrv_SW_IIC_ConfigBus(I2C_BusCfg_t* pBusCfg);
void MDrv_IIC_Init(void);

#endif // _DRV_IIC_H_
