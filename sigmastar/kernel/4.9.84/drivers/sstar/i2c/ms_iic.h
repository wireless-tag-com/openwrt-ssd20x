/*
* ms_iic.h- Sigmastar
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
#ifndef _DRV_IIC_H_
#define _DRV_IIC_H_

#include "ms_types.h"

#define	IIC_NUM_OF_MAX				(20)
#define I2C_BYTE_MASK	            0xFF

#define	IIC_NUM_OF_HW				(1)
#define MDRV_NAME_IIC                   "iic"
#define MDRV_MAJOR_IIC                  0x8a
#define MDRV_MINOR_IIC                  0x00
#define HWI2C_PORTM                   4 //maximum support ports

#define I2C_CUST_M_NOSTOP     BIT1        /* customized use of i2c_msg.flags
                                             no stop after sending data: S Addr Wr [A] Data [A] Data [A] ... [A] Data [A]
                                           */


////////////////////////////////////////////////////////////////////////////////
// Define & data type
////////////////////////////////////////////////////////////////////////////////
#define MSIF_HWI2C_LIB_CODE                     {'H','I','2','C'}    //Lib code
#define MSIF_HWI2C_LIBVER                       {'0','6'}            //LIB version
#define MSIF_HWI2C_BUILDNUM                     {'0','6'}            //Build Number
#define MSIF_HWI2C_CHANGELIST                   {'0','0','5','4','9','6','1','5'} //P4 ChangeList Number

#define HWI2C_DRV_VERSION               /* Character String for DRV/API version             */  \
    MSIF_TAG,                           /* 'MSIF'                                           */  \
    MSIF_CLASS,                         /* '00'                                             */  \
    MSIF_CUS,                           /* 0x0000                                           */  \
    MSIF_MOD,                           /* 0x0000                                           */  \
    MSIF_CHIP,                                                                                  \
    MSIF_CPU,                                                                                   \
    MSIF_HWI2C_LIB_CODE,                /* IP__                                             */  \
    MSIF_HWI2C_LIBVER,                  /* 0.0 ~ Z.Z                                        */  \
    MSIF_HWI2C_BUILDNUM,                /* 00 ~ 99                                          */  \
    MSIF_HWI2C_CHANGELIST,              /* CL#                                              */  \
    MSIF_OS

//
typedef void* (*ms_i2c_feature_fp)(u16, u16, void*, u32);

/// debug level
typedef enum _HWI2C_DbgLv
{
    E_HWI2C_DBGLV_NONE,          /// no debug message
    E_HWI2C_DBGLV_ERR_ONLY,      /// show error only
    E_HWI2C_DBGLV_INFO,          /// show error & informaiton
    E_HWI2C_DBGLV_ALL            /// show error, information & funciton name
}HWI2C_DbgLv;

/// I2C select master port
typedef enum _HWI2C_PORT
{
    E_HWI2C_PORT_0 = 0, /// port 0_0 //disable port 0
    E_HWI2C_PORT0_1,    /// port 0_1
    E_HWI2C_PORT0_2,    /// port 0_2
    E_HWI2C_PORT0_3,    /// port 0_3
    E_HWI2C_PORT0_4,    /// port 0_4
    E_HWI2C_PORT0_5,    /// port 0_5
    E_HWI2C_PORT0_6,    /// port 0_6
    E_HWI2C_PORT0_7,    /// port 0_7

    E_HWI2C_PORT_1 = 8, /// port 1_0 //disable port 1
    E_HWI2C_PORT1_1,    /// port 1_1
    E_HWI2C_PORT1_2,    /// port 1_2
    E_HWI2C_PORT1_3,    /// port 1_3
    E_HWI2C_PORT1_4,    /// port 1_4
    E_HWI2C_PORT1_5,    /// port 1_5
    E_HWI2C_PORT1_6,    /// port 1_6
    E_HWI2C_PORT1_7,    /// port 1_7

    E_HWI2C_PORT_2 = 16,/// port 2_0 //disable port 2
    E_HWI2C_PORT2_1,    /// port 2_1
    E_HWI2C_PORT2_2,    /// port 2_2
    E_HWI2C_PORT2_3,    /// port 2_3
    E_HWI2C_PORT2_4,    /// port 2_4
    E_HWI2C_PORT2_5,    /// port 2_5
    E_HWI2C_PORT2_6,    /// port 2_6
    E_HWI2C_PORT2_7,    /// port 2_7

    E_HWI2C_PORT_3 = 24,/// port 3_0 //disable port 3
    E_HWI2C_PORT3_1,    /// port 3_1
    E_HWI2C_PORT3_2,    /// port 3_2
    E_HWI2C_PORT3_3,    /// port 3_3
    E_HWI2C_PORT3_4,    /// port 3_4
    E_HWI2C_PORT3_5,    /// port 3_5
    E_HWI2C_PORT3_6,    /// port 3_6
    E_HWI2C_PORT3_7,    /// port 3_7

    E_HWI2C_PORT_NOSUP  /// non-support port
}HWI2C_PORT;

/// I2C clock speed select
typedef enum _HWI2C_CLKSEL
{
    E_HWI2C_HIGH = 0,  /// high speed
    E_HWI2C_NORMAL,    /// normal speed
    E_HWI2C_SLOW,      /// slow speed
    E_HWI2C_VSLOW,     /// very slow
    E_HWI2C_USLOW,     /// ultra slow
    E_HWI2C_UVSLOW,    /// ultra-very slow
    E_HWI2C_NOSUP      /// non-support speed
}HWI2C_CLKSEL;

/// I2C state
typedef enum _HWI2C_State
{
    E_HWI2C_IDLE,      /// idle state
    E_HWI2C_READ_DATA, /// read data state
    E_HWI2C_WRITE_DATA, /// write data state
    E_HWI2C_DMA_READ_DATA, /// DMA read data state
    E_HWI2C_DMA_WRITE_DATA /// DMA write data state
}HWI2C_State;

typedef enum {
    E_HWI2C_READ_MODE_DIRECT,                 ///< first transmit slave address + reg address and then start receive the data */
    E_HWI2C_READ_MODE_DIRECTION_CHANGE,       ///< slave address + reg address in write mode, direction change to read mode, repeat start slave address in read mode, data from device
    E_HWI2C_READ_MODE_DIRECTION_CHANGE_STOP_START,  ///< slave address + reg address in write mode + stop, direction change to read mode, repeat start slave address in read mode, data from device
    E_HWI2C_READ_MODE_MAX
} HWI2C_ReadMode;

typedef enum _HWI2C_DMA_ADDRMODE
{
    E_HWI2C_DMA_ADDR_NORMAL = 0,
    E_HWI2C_DMA_ADDR_10BIT,
    E_HWI2C_DMA_ADDR_MAX,
}HWI2C_DMA_ADDRMODE;

typedef enum _HWI2C_DMA_READMODE
{
    E_HWI2C_DMA_READ_NOSTOP = 0,
    E_HWI2C_DMA_READ_STOP,
    E_HWI2C_DMA_READ_MAX,
}HWI2C_DMA_READMODE;

typedef enum _HWI2C_DMA_MIUPRI
{
    E_HWI2C_DMA_PRI_LOW = 0,
    E_HWI2C_DMA_PRI_HIGH,
    E_HWI2C_DMA_PRI_MAX,
}HWI2C_DMA_MIUPRI;

typedef enum _HWI2C_DMA_MIUCH
{
    E_HWI2C_DMA_MIU_CH0 = 0,
    E_HWI2C_DMA_MIU_CH1,
    E_HWI2C_DMA_MIU_MAX,
}HWI2C_DMA_MIUCH;

typedef enum _HWI2C_DMA_HW_FEATURE
{
	E_HWI2C_FEATURE_NWRITE = 0,
	E_HWI2C_FEATURE_MAX,
}HWI2C_DMA_HW_FEATURE;

/// I2C master pin config
typedef struct _HWI2C_PinCfg
{
    U32  u32Reg;    /// register
    U8   u8BitPos;  /// bit position
    BOOL bEnable;   /// enable or disable
}HWI2C_PinCfg;

/// I2C port config
typedef struct _HWI2C_PortCfg
{
    U32              u32DmaPhyAddr;  /// DMA physical address
    HWI2C_DMA_ADDRMODE  eDmaAddrMode;   /// DMA address mode
    HWI2C_DMA_MIUPRI    eDmaMiuPri;     /// DMA miu priroity
    HWI2C_DMA_MIUCH     eDmaMiuCh;      /// DMA miu channel
    BOOL             bDmaEnable;     /// DMA enable

    HWI2C_PORT          ePort;          /// number
    HWI2C_CLKSEL        eSpeed;         /// clock speed
    HWI2C_ReadMode      eReadMode;      /// read mode
    BOOL             bEnable;        /// enable

}HWI2C_PortCfg;

/// I2C Configuration for initialization
typedef struct _HWI2C_UnitCfg
{
    HWI2C_PortCfg   sCfgPort[4];    /// port cfg info
    HWI2C_PinCfg    sI2CPin;        /// pin info
    HWI2C_CLKSEL    eSpeed;         /// speed
    HWI2C_PORT      ePort;          /// port
    HWI2C_ReadMode  eReadMode;      /// read mode
    int             eGroup;          /// port
    U32             eBaseAddr;
    U32             eChipAddr;
    U32             eClkAddr;
}HWI2C_UnitCfg;

/// I2C information
typedef struct _HWI2C_Info
{
    U32          u32IOMap; /// base address
    HWI2C_UnitCfg   sUnitCfg; /// configuration
}HWI2C_Info;

/// I2C status
typedef struct _HWI2C_Status
{
    U8       u8DbgLevel;   /// debug level
    BOOL     bIsInit;      /// initialized
    BOOL     bIsMaster;    /// master
    HWI2C_State eState;       /// state
}HWI2C_Status;

typedef struct _I2C_DMA
{
	dma_addr_t i2c_dma_addr;
	u8  *i2c_virt_addr;
}I2C_DMA;

extern I2C_DMA HWI2C_DMA[HWI2C_PORTM];

////////////////////////////////////////////////////////////////////////////////
// Extern Function
////////////////////////////////////////////////////////////////////////////////
void MDrv_HW_IIC_Init(void *base,void *chipbase,int i2cgroup,void *clkbase, int i2cpadmux, int i2cspeed, int i2c_enDma);
BOOL MDrv_HWI2C_Init(HWI2C_UnitCfg *psCfg);
BOOL MDrv_HWI2C_WriteBytes(U16 u16SlaveCfg, U32 uAddrCnt, U8 *pRegAddr, U32 uSize, U8 *pData);
BOOL MDrv_HWI2C_ReadBytes(U16 u16SlaveCfg, U32 uAddrCnt, U8 *pRegAddr, U32 uSize, U8 *pData);
//BOOL MDrv_HWI2C_SetClk(HWI2C_CLKSEL eClk);


BOOL MDrv_HWI2C_Start(U16 u16PortOffset);
BOOL MDrv_HWI2C_Stop(U16 u16PortOffset);
BOOL MDrv_HWI2C_Send_Byte(U16 u16PortOffset, U8 u8Data);
BOOL MDrv_HWI2C_Recv_Byte(U16 u16PortOffset, U8 *pData);
BOOL MDrv_HWI2C_NoAck(U16 u16PortOffset);
BOOL _MDrv_HWI2C_GetPortRegOffset(U8 u8Port, U16 *pu16Offset);
BOOL MDrv_HWI2C_Reset(U16 u16PortOffset, BOOL bReset);



#endif // _DRV_IIC_H_

