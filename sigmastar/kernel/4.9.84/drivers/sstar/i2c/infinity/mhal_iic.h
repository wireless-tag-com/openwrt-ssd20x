/*
* mhal_iic.h- Sigmastar
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
#ifndef _HAL_HWI2C_H_
#define _HAL_HWI2C_H_

#ifdef _HAL_IIC_C_
#define _extern_HAL_IIC_
#else
#define _extern_HAL_IIC_ extern
#endif

#include <asm/types.h>
#include "mdrv_types.h"

////////////////////////////////////////////////////////////////////////////////
/// @file halHWI2C.h
/// @brief MIIC control functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Header Files
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Define & data type
////////////////////////////////////////////////////////////////////////////////
//v: value n: shift n bits
//v: value n: shift n bits
#define _LShift(v, n)       ((v) << (n))
#define _RShift(v, n)       ((v) >> (n))

#define HIGH_BYTE(val)      (U8)_RShift((val), 8)
#define LOW_BYTE(val)       ((U8)((val) & 0xFF))

#define __BIT(x)    ((U8)_LShift(1, x))
#define __BIT0       __BIT(0)
#define __BIT1       __BIT(1)
#define __BIT2       __BIT(2)
#define __BIT3       __BIT(3)
#define __BIT4       __BIT(4)
#define __BIT5       __BIT(5)
#define __BIT6       __BIT(6)
#define __BIT7       __BIT(7)
#if 0
//////////////////////////////////////////////////////////////////////////////////////
typedef unsigned int               BOOL;                            // 1 byte
/// data type unsigned char, data length 1 byte
typedef unsigned char               U8;                              // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short              U16;                             // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned int               U32;                             // 4 bytes
/// data type unsigned int64, data length 8 byte
typedef unsigned long         MS_U64;                             // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char                 MS_S8;                              // 1 byte
/// data type signed short, data length 2 byte
typedef signed short                MS_S16;                             // 2 bytes
/// data type signed int, data length 4 byte
typedef signed int                 MS_S32;                             // 4 bytes
/// data type signed int64, data length 8 byte
typedef signed long            MS_S64;                             // 8 bytes
/////////////////////////////////////////////////////////////////////////////////
#endif

#define HWI2C_SET_RW_BIT(bRead, val) ((bRead) ? ((val) | __BIT0) : ((val) & ~__BIT0))

#define HAL_HWI2C_PORTS         4
#define HAL_HWI2C_PORT0         0
#define HAL_HWI2C_PORT1         1
#define HAL_HWI2C_PORT2         2
#define HAL_HWI2C_PORT3         3

typedef enum _HAL_HWI2C_STATE
{
	E_HAL_HWI2C_STATE_IDEL = 0,
	E_HAL_HWI2C_STATE_START,
	E_HAL_HWI2C_STATE_WRITE,
	E_HAL_HWI2C_STATE_READ,
	E_HAL_HWI2C_STATE_INT,
	E_HAL_HWI2C_STATE_WAIT,
	E_HAL_HWI2C_STATE_STOP
} HAL_HWI2C_STATE;


typedef enum _HAL_HWI2C_PORT
{
    E_HAL_HWI2C_PORT0_0 = 0, //disable port 0
    E_HAL_HWI2C_PORT0_1,
    E_HAL_HWI2C_PORT0_2,
    E_HAL_HWI2C_PORT0_3,
    E_HAL_HWI2C_PORT0_4,
    E_HAL_HWI2C_PORT0_5,
    E_HAL_HWI2C_PORT0_6,
    E_HAL_HWI2C_PORT0_7,

    E_HAL_HWI2C_PORT1_0,  //disable port 1
    E_HAL_HWI2C_PORT1_1,
    E_HAL_HWI2C_PORT1_2,
    E_HAL_HWI2C_PORT1_3,
    E_HAL_HWI2C_PORT1_4,
    E_HAL_HWI2C_PORT1_5,
    E_HAL_HWI2C_PORT1_6,
    E_HAL_HWI2C_PORT1_7,

    E_HAL_HWI2C_PORT2_0,  //disable port 2
    E_HAL_HWI2C_PORT2_1,
    E_HAL_HWI2C_PORT2_2,
    E_HAL_HWI2C_PORT2_3,
    E_HAL_HWI2C_PORT2_4,
    E_HAL_HWI2C_PORT2_5,
    E_HAL_HWI2C_PORT2_6,
    E_HAL_HWI2C_PORT2_7,

    E_HAL_HWI2C_PORT3_0, //disable port 3
    E_HAL_HWI2C_PORT3_1,
    E_HAL_HWI2C_PORT3_2,
    E_HAL_HWI2C_PORT3_3,
    E_HAL_HWI2C_PORT3_4,
    E_HAL_HWI2C_PORT3_5,
    E_HAL_HWI2C_PORT3_6,
    E_HAL_HWI2C_PORT3_7,

    E_HAL_HWI2C_PORT_NOSUP
}HAL_HWI2C_PORT;

typedef enum _HAL_HWI2C_CLKSEL
{
    E_HAL_HWI2C_CLKSEL_HIGH = 0,
    E_HAL_HWI2C_CLKSEL_NORMAL,
    E_HAL_HWI2C_CLKSEL_SLOW,
    E_HAL_HWI2C_CLKSEL_VSLOW,
    E_HAL_HWI2C_CLKSEL_USLOW,
    E_HAL_HWI2C_CLKSEL_UVSLOW,
    E_HAL_HWI2C_CLKSEL_NOSUP
}HAL_HWI2C_CLKSEL;

typedef enum _HAL_HWI2C_CLK
{
    E_HAL_HWI2C_CLK_DIV4 = 1, //750K@12MHz
    E_HAL_HWI2C_CLK_DIV8,     //375K@12MHz
    E_HAL_HWI2C_CLK_DIV16,    //187.5K@12MHz
    E_HAL_HWI2C_CLK_DIV32,    //93.75K@12MHz
    E_HAL_HWI2C_CLK_DIV64,    //46.875K@12MHz
    E_HAL_HWI2C_CLK_DIV128,   //23.4375K@12MHz
    E_HAL_HWI2C_CLK_DIV256,   //11.71875K@12MHz
    E_HAL_HWI2C_CLK_DIV512,   //5.859375K@12MHz
    E_HAL_HWI2C_CLK_DIV1024,  //2.9296875K@12MHz
    E_HAL_HWI2C_CLK_NOSUP
}HAL_HWI2C_CLK;

typedef enum {
    E_HAL_HWI2C_READ_MODE_DIRECT,                       ///< first transmit slave address + reg address and then start receive the data */
    E_HAL_HWI2C_READ_MODE_DIRECTION_CHANGE,             ///< slave address + reg address in write mode, direction change to read mode, repeat start slave address in read mode, data from device
    E_HAL_HWI2C_READ_MODE_DIRECTION_CHANGE_STOP_START,  ///< slave address + reg address in write mode + stop, direction change to read mode, repeat start slave address in read mode, data from device
    E_HAL_HWI2C_READ_MODE_MAX
} HAL_HWI2C_ReadMode;

typedef enum _HAL_HWI2C_DMA_ADDRMODE
{
    E_HAL_HWI2C_DMA_ADDR_NORMAL = 0,
    E_HAL_HWI2C_DMA_ADDR_10BIT,
    E_HAL_HWI2C_DMA_ADDR_MAX,
}HAL_HWI2C_DMA_ADDRMODE;

typedef enum _HAL_HWI2C_DMA_MIUPRI
{
    E_HAL_HWI2C_DMA_PRI_LOW = 0,
    E_HAL_HWI2C_DMA_PRI_HIGH,
    E_HAL_HWI2C_DMA_PRI_MAX,
}HAL_HWI2C_DMA_MIUPRI;

typedef enum _HAL_HWI2C_DMA_MIUCH
{
    E_HAL_HWI2C_DMA_MIU_CH0 = 0,
    E_HAL_HWI2C_DMA_MIU_CH1,
    E_HAL_HWI2C_DMA_MIU_MAX,
}HAL_HWI2C_DMA_MIUCH;

typedef struct _HAL_HWI2C_PinCfg
{
    U32  u32Reg;    /// register
    U8   u8BitPos;  /// bit position
    BOOL bEnable;   /// enable or disable
}HAL_HWI2C_PinCfg;

typedef struct _HAL_HWI2C_PortCfg //Synchronize with drvHWI2C.h
{
    U32                  u32DmaPhyAddr;  /// DMA physical address
    HAL_HWI2C_DMA_ADDRMODE  eDmaAddrMode;   /// DMA address mode
    HAL_HWI2C_DMA_MIUPRI    eDmaMiuPri;     /// DMA miu priroity
    HAL_HWI2C_DMA_MIUCH     eDmaMiuCh;      /// DMA miu channel
    BOOL                 bDmaEnable;     /// DMA enable

    HAL_HWI2C_PORT          ePort;          /// number
    HAL_HWI2C_CLKSEL        eSpeed;         /// clock speed
    HAL_HWI2C_ReadMode      eReadMode;      /// read mode
    BOOL                 bEnable;        /// enable

}HAL_HWI2C_PortCfg;

/// I2C Configuration for initialization
typedef struct _HAL_HWI2C_CfgInit //Synchronize with drvHWI2C.h
{
    HAL_HWI2C_PortCfg   sCfgPort[4];    /// port cfg info
    HAL_HWI2C_PinCfg    sI2CPin;        /// pin info
    HAL_HWI2C_CLKSEL    eSpeed;         /// speed
    HAL_HWI2C_PORT      ePort;          /// port
    HAL_HWI2C_ReadMode  eReadMode;      /// read mode
    
}HAL_HWI2C_CfgInit;

////////////////////////////////////////////////////////////////////////////////
// Extern function
////////////////////////////////////////////////////////////////////////////////
_extern_HAL_IIC_ U32 MsOS_PA2KSEG1(U32 addr);
_extern_HAL_IIC_ U32 MsOS_VA2PA(U32 addr);

_extern_HAL_IIC_ void HAL_HWI2C_ExtraDelay(U32 u32Us);
_extern_HAL_IIC_ void HAL_HWI2C_SetIOMapBase(U32 u32Base,U32 u32ChipBase,U32 u32ClkBase);
_extern_HAL_IIC_ U8 HAL_HWI2C_ReadByte(U32 u32RegAddr);
_extern_HAL_IIC_ U16 HAL_HWI2C_Read2Byte(U32 u32RegAddr);
_extern_HAL_IIC_ U32 HAL_HWI2C_Read4Byte(U32 u32RegAddr);
_extern_HAL_IIC_ BOOL HAL_HWI2C_WriteByte(U32 u32RegAddr, U8 u8Val);
_extern_HAL_IIC_ BOOL HAL_HWI2C_Write2Byte(U32 u32RegAddr, U16 u16Val);
_extern_HAL_IIC_ BOOL HAL_HWI2C_Write4Byte(U32 u32RegAddr, U32 u32Val);
_extern_HAL_IIC_ BOOL HAL_HWI2C_WriteRegBit(U32 u32RegAddr, U8 u8Mask, BOOL bEnable);
_extern_HAL_IIC_ BOOL HAL_HWI2C_WriteByteMask(U32 u32RegAddr, U8 u8Val, U8 u8Mask);

_extern_HAL_IIC_ BOOL HAL_HWI2C_Init_Chip(void);
_extern_HAL_IIC_ BOOL HAL_HWI2C_IsMaster(void);
_extern_HAL_IIC_ BOOL HAL_HWI2C_Master_Enable(U16 u16PortOffset);
_extern_HAL_IIC_ BOOL HAL_HWI2C_SetPortRegOffset(HAL_HWI2C_PORT ePort, U16* pu16Offset);
_extern_HAL_IIC_ BOOL HAL_HWI2C_GetPortIdxByOffset(U16 u16Offset, U8* pu8Port);
_extern_HAL_IIC_ BOOL HAL_HWI2C_GetPortIdxByPort(HAL_HWI2C_PORT ePort, U8* pu8Port);
_extern_HAL_IIC_ BOOL HAL_HWI2C_SelectPort(int ePort);
_extern_HAL_IIC_ BOOL HAL_HWI2C_SetClk(U16 u16PortOffset, HAL_HWI2C_CLKSEL eClkSel);

_extern_HAL_IIC_ BOOL HAL_HWI2C_Start(U16 u16PortOffset);
_extern_HAL_IIC_ BOOL HAL_HWI2C_Stop(U16 u16PortOffset);
_extern_HAL_IIC_ BOOL HAL_HWI2C_ReadRdy(U16 u16PortOffset);
_extern_HAL_IIC_ BOOL HAL_HWI2C_SendData(U16 u16PortOffset, U8 u8Data);
_extern_HAL_IIC_ U8 HAL_HWI2C_RecvData(U16 u16PortOffset);
_extern_HAL_IIC_ BOOL HAL_HWI2C_Get_SendAck(U16 u16PortOffset);
_extern_HAL_IIC_ BOOL HAL_HWI2C_NoAck(U16 u16PortOffset);
_extern_HAL_IIC_ BOOL HAL_HWI2C_Ack(U16 u16PortOffset);
_extern_HAL_IIC_ U8 HAL_HWI2C_GetState(U16 u16PortOffset);
_extern_HAL_IIC_ BOOL HAL_HWI2C_Is_Idle(U16 u16PortOffset);
_extern_HAL_IIC_ BOOL HAL_HWI2C_Is_INT(U16 u16PortOffset);
_extern_HAL_IIC_ BOOL HAL_HWI2C_Clear_INT(U16 u16PortOffset);
_extern_HAL_IIC_ BOOL HAL_HWI2C_Reset(U16 u16PortOffset, BOOL bReset);
_extern_HAL_IIC_ BOOL HAL_HWI2C_Send_Byte(U16 u16PortOffset, U8 u8Data);
_extern_HAL_IIC_ BOOL HAL_HWI2C_Recv_Byte(U16 u16PortOffset, U8 *pData);

_extern_HAL_IIC_ BOOL HAL_HWI2C_DMA_Init(U16 u16PortOffset, HAL_HWI2C_PortCfg* pstPortCfg);
_extern_HAL_IIC_ BOOL HAL_HWI2C_DMA_Enable(U16 u16PortOffset, BOOL bEnable);
_extern_HAL_IIC_ BOOL HAL_HWI2C_DMA_ReadBytes(U16 u16PortOffset, U16 u16SlaveCfg, U32 uAddrCnt, U8 *pRegAddr, U32 uSize, U8 *pData);
_extern_HAL_IIC_ BOOL HAL_HWI2C_DMA_WriteBytes(U16 u16PortOffset, U16 u16SlaveCfg, U32 uAddrCnt, U8 *pRegAddr, U32 uSize, U8 *pData);

_extern_HAL_IIC_ void HAL_HWI2C_Init_ExtraProc(void);
_extern_HAL_IIC_ BOOL HAL_HWI2C_WriteChipByteMask(U32 u32RegAddr, U8 u8Val, U8 u8Mask);
_extern_HAL_IIC_ U8 HAL_HWI2C_ReadChipByte(U32 u32RegAddr);
_extern_HAL_IIC_ BOOL HAL_HWI2C_WriteChipByte(U32 u32RegAddr, U8 u8Val);


#endif  //_MHAL_HWI2C_H_

