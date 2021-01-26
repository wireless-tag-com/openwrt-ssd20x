/*
* mstar_i2c.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: alterman.lin <alterman.lin@sigmastar.com.tw>
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


#ifndef __HAL_BUS_I2C_H__
#define __HAL_BUS_I2C_H__

/*=============================================================*/
// Include files
/*=============================================================*/
#include <asm/types.h>


////////////////////////////////////////////////////////////////////////////////
// Define & data type
////////////////////////////////////////////////////////////////////////////////

#ifdef _HAL_IIC_C_
#define _extern_HAL_IIC_
#else
#define _extern_HAL_IIC_ extern
#endif

#define BOOL bool
#define FALSE false
#define TRUE true
#define U8 u8
#define U16 u16
#define U32 u32
#define UartSendTrace printf

//v: value n: shift n bits
//v: value n: shift n bits
#define _LShift(v, n)       ((v) << (n))
#define _RShift(v, n)       ((v) >> (n))
//#define udelay(x)	SYS_UDELAY(x)
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


//pad mux configuration
#define CHIP_REG_ALLPADIN               (CHIP_REG_BASE+0xA0)
    #define CHIP_ALLPAD_IN              (__BIT0)

#define CHIP_REG_DDCRMOD               (CHIP_REG_BASE+0xAE)

//############################
//
//IP bank address : for independent port
//
//############################
//Standard mode
#define HWI2C_REG_BASE                  0 //(0x111800) //0x1(11800) + offset ==> default set to port 0
#define REG_HWI2C_MIIC_CFG              (HWI2C_REG_BASE+0x00*2)
    #define _MIIC_CFG_RESET             (__BIT0)
    #define _MIIC_CFG_EN_DMA            (__BIT1)
    #define _MIIC_CFG_EN_INT            (__BIT2)
    #define _MIIC_CFG_EN_CLKSTR         (__BIT3)
    #define _MIIC_CFG_EN_TMTINT         (__BIT4)
    #define _MIIC_CFG_EN_FILTER         (__BIT5)
    #define _MIIC_CFG_EN_PUSH1T         (__BIT6)
    #define _MIIC_CFG_RESERVED          (__BIT7)
#define REG_HWI2C_CMD_START             (HWI2C_REG_BASE+0x01*2)
    #define _CMD_START                  (__BIT0)
#define REG_HWI2C_CMD_STOP              (HWI2C_REG_BASE+0x01*2+1)
    #define _CMD_STOP                   (__BIT0)
#define REG_HWI2C_WDATA                 (HWI2C_REG_BASE+0x02*2)
#define REG_HWI2C_WDATA_GET             (HWI2C_REG_BASE+0x02*2+1)
    #define _WDATA_GET_ACKBIT           (__BIT0)
#define REG_HWI2C_RDATA                 (HWI2C_REG_BASE+0x03*2)
#define REG_HWI2C_RDATA_CFG             (HWI2C_REG_BASE+0x03*2+1)
    #define _RDATA_CFG_TRIG             (__BIT0)
    #define _RDATA_CFG_ACKBIT           (__BIT1)
#define REG_HWI2C_INT_CTL               (HWI2C_REG_BASE+0x04*2)
    #define _INT_CTL                    (__BIT0) //write this register to clear int
#define REG_HWI2C_CUR_STATE             (HWI2C_REG_BASE+0x05*2) //For Debug
    #define _CUR_STATE_MSK              (__BIT4|__BIT3|__BIT2|__BIT1|__BIT0)
#define REG_HWI2C_INT_STATUS            (HWI2C_REG_BASE+0x05*2+1) //For Debug
    #define _INT_STARTDET               (__BIT0)
    #define _INT_STOPDET                (__BIT1)
    #define _INT_RXDONE                 (__BIT2)
    #define _INT_TXDONE                 (__BIT3)
    #define _INT_CLKSTR                 (__BIT4)
    #define _INT_SCLERR                 (__BIT5)
    #define _INT_TIMEOUT                (__BIT6)
#define REG_HWI2C_STP_CNT               (HWI2C_REG_BASE+0x08*2)
#define REG_HWI2C_CKH_CNT               (HWI2C_REG_BASE+0x09*2)
#define REG_HWI2C_CKL_CNT               (HWI2C_REG_BASE+0x0A*2)
#define REG_HWI2C_SDA_CNT               (HWI2C_REG_BASE+0x0B*2)
#define REG_HWI2C_STT_CNT               (HWI2C_REG_BASE+0x0C*2)
#define REG_HWI2C_LTH_CNT               (HWI2C_REG_BASE+0x0D*2)
#define REG_HWI2C_TMT_CNT               (HWI2C_REG_BASE+0x0E*2)
#define REG_HWI2C_SCLI_DELAY            (HWI2C_REG_BASE+0x0F*2)
    #define _SCLI_DELAY                 (__BIT2|__BIT1|__BIT0)


//DMA mode
#define REG_HWI2C_DMA_CFG               (HWI2C_REG_BASE+0x20*2)
    #define _DMA_CFG_RESET              (__BIT1)
    #define _DMA_CFG_INTEN              (__BIT2)
    #define _DMA_CFG_MIURST             (__BIT3)
    #define _DMA_CFG_MIUPRI             (__BIT4)
#define REG_HWI2C_DMA_MIU_ADR           (HWI2C_REG_BASE+0x21*2) // 4 bytes
#define REG_HWI2C_DMA_CTL               (HWI2C_REG_BASE+0x23*2)
//    #define _DMA_CTL_TRIG               (__BIT0)
//    #define _DMA_CTL_RETRIG             (__BIT1)
    #define _DMA_CTL_TXNOSTOP           (__BIT5) //miic transfer format, 1: S+data..., 0: S+data...+P
    #define _DMA_CTL_RDWTCMD            (__BIT6) //miic transfer format, 1:read, 0:write
    #define _DMA_CTL_MIUCHSEL           (__BIT7) //0: miu0, 1:miu1
#define REG_HWI2C_DMA_TXR               (HWI2C_REG_BASE+0x24*2)
    #define _DMA_TXR_DONE               (__BIT0)
#define REG_HWI2C_DMA_CMDDAT0           (HWI2C_REG_BASE+0x25*2) // 8 bytes
#define REG_HWI2C_DMA_CMDDAT1           (HWI2C_REG_BASE+0x25*2+1)
#define REG_HWI2C_DMA_CMDDAT2           (HWI2C_REG_BASE+0x26*2)
#define REG_HWI2C_DMA_CMDDAT3           (HWI2C_REG_BASE+0x26*2+1)
#define REG_HWI2C_DMA_CMDDAT4           (HWI2C_REG_BASE+0x27*2)
#define REG_HWI2C_DMA_CMDDAT5           (HWI2C_REG_BASE+0x27*2+1)
#define REG_HWI2C_DMA_CMDDAT6           (HWI2C_REG_BASE+0x28*2)
#define REG_HWI2C_DMA_CMDDAT7           (HWI2C_REG_BASE+0x28*2+1)
#define REG_HWI2C_DMA_CMDLEN            (HWI2C_REG_BASE+0x29*2)
    #define _DMA_CMDLEN_MSK             (__BIT2|__BIT1|__BIT0)
#define REG_HWI2C_DMA_DATLEN            (HWI2C_REG_BASE+0x2A*2) // 4 bytes
#define REG_HWI2C_DMA_TXFRCNT           (HWI2C_REG_BASE+0x2C*2) // 4 bytes
#define REG_HWI2C_DMA_SLVADR            (HWI2C_REG_BASE+0x2E*2)
    #define _DMA_SLVADR_10BIT_MSK       0x3FF //10 bits
    #define _DMA_SLVADR_NORML_MSK       0x7F //7 bits
#define REG_HWI2C_DMA_SLVCFG            (HWI2C_REG_BASE+0x2E*2+1)
    #define _DMA_10BIT_MODE             (__BIT2)
#define REG_HWI2C_DMA_CTL_TRIG          (HWI2C_REG_BASE+0x2F*2)
    #define _DMA_CTL_TRIG               (__BIT0)
#define REG_HWI2C_DMA_CTL_RETRIG        (HWI2C_REG_BASE+0x2F*2+1)
    #define _DMA_CTL_RETRIG             (__BIT0)

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

#define HAL_HWI2C_PORTS         2
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
} HAL_HWI2C_PORT;

typedef enum _HAL_HWI2C_CLKSEL
{
    E_HAL_HWI2C_CLKSEL_HIGH = 0,
    E_HAL_HWI2C_CLKSEL_NORMAL,
    E_HAL_HWI2C_CLKSEL_SLOW,
    E_HAL_HWI2C_CLKSEL_VSLOW,
    E_HAL_HWI2C_CLKSEL_USLOW,
    E_HAL_HWI2C_CLKSEL_UVSLOW,
    E_HAL_HWI2C_CLKSEL_NOSUP
} HAL_HWI2C_CLKSEL;

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
} HAL_HWI2C_CLK;

typedef enum
{
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
} HAL_HWI2C_DMA_ADDRMODE;

typedef enum _HAL_HWI2C_DMA_MIUPRI
{
    E_HAL_HWI2C_DMA_PRI_LOW = 0,
    E_HAL_HWI2C_DMA_PRI_HIGH,
    E_HAL_HWI2C_DMA_PRI_MAX,
} HAL_HWI2C_DMA_MIUPRI;

typedef enum _HAL_HWI2C_DMA_MIUCH
{
    E_HAL_HWI2C_DMA_MIU_CH0 = 0,
    E_HAL_HWI2C_DMA_MIU_CH1,
    E_HAL_HWI2C_DMA_MIU_MAX,
} HAL_HWI2C_DMA_MIUCH;

typedef struct _HAL_HWI2C_PinCfg
{
    U32  u32Reg;    /// register
    U8   u8BitPos;  /// bit position
    BOOL bEnable;   /// enable or disable
} HAL_HWI2C_PinCfg;

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

} HAL_HWI2C_PortCfg;

/// I2C Configuration for initialization
typedef struct _HAL_HWI2C_CfgInit //Synchronize with drvHWI2C.h
{
    HAL_HWI2C_PortCfg   sCfgPort[4];    /// port cfg info
    HAL_HWI2C_PinCfg    sI2CPin;        /// pin info
    HAL_HWI2C_CLKSEL    eSpeed;         /// speed
    HAL_HWI2C_PORT      ePort;          /// port
    HAL_HWI2C_ReadMode  eReadMode;      /// read mode

} HAL_HWI2C_CfgInit;

typedef struct _I2C_DMA
{
    void  *i2c_dma_addr;
    u8  *i2c_virt_addr;
} I2C_DMA;

/// debug level
typedef enum _HWI2C_DbgLv
{
    E_HWI2C_DBGLV_NONE,          /// no debug message
    E_HWI2C_DBGLV_ERR_ONLY,      /// show error only
    E_HWI2C_DBGLV_INFO,          /// show error & informaiton
    E_HWI2C_DBGLV_ALL            /// show error, information & funciton name
} HWI2C_DbgLv;

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
} HWI2C_PORT;

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
} HWI2C_CLKSEL;

/// I2C state
typedef enum _HWI2C_State
{
    E_HWI2C_IDLE,      /// idle state
    E_HWI2C_READ_DATA, /// read data state
    E_HWI2C_WRITE_DATA, /// write data state
    E_HWI2C_DMA_READ_DATA, /// DMA read data state
    E_HWI2C_DMA_WRITE_DATA /// DMA write data state
} HWI2C_State;

typedef enum
{
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
} HWI2C_DMA_ADDRMODE;

typedef enum _HWI2C_DMA_READMODE
{
    E_HWI2C_DMA_READ_NOSTOP = 0,
    E_HWI2C_DMA_READ_STOP,
    E_HWI2C_DMA_READ_MAX,
} HWI2C_DMA_READMODE;

typedef enum _HWI2C_DMA_MIUPRI
{
    E_HWI2C_DMA_PRI_LOW = 0,
    E_HWI2C_DMA_PRI_HIGH,
    E_HWI2C_DMA_PRI_MAX,
} HWI2C_DMA_MIUPRI;

typedef enum _HWI2C_DMA_MIUCH
{
    E_HWI2C_DMA_MIU_CH0 = 0,
    E_HWI2C_DMA_MIU_CH1,
    E_HWI2C_DMA_MIU_MAX,
} HWI2C_DMA_MIUCH;

typedef enum
{
    I2C_PROC_DONE                =  0,
    I2C_ERROR                    = -1,
    I2C_BAD_PARAMETER            = -2,
    I2C_ALREADY_USED             = -3,
    I2C_UNKNOWN_HANDLE           = -4,
    I2C_HANDLE_NOT_OPENED        = -5,
    I2C_NO_MORE_HANDLE_FREE      = -6,
    I2C_HAL_NOT_SUPPORT          = -7,
    I2C_DRV_NOT_SUPPORT          = -8,
    I2C_POLLING_TIMEOUT          = -9,
    I2C_MAS_GDMA_ERROR           = -10,
    I2C_MAS_GDMA_POLLING_TIMEOUT = -11
}I2CErrorCode_e;

/// I2C master pin config
typedef struct _HWI2C_PinCfg
{
    U32  u32Reg;    /// register
    U8   u8BitPos;  /// bit position
    BOOL bEnable;   /// enable or disable
} HWI2C_PinCfg;

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

} HWI2C_PortCfg;

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
} HWI2C_UnitCfg;

/// I2C information
typedef struct _HWI2C_Info
{
    U32          u32IOMap; /// base address
    HWI2C_UnitCfg   sUnitCfg; /// configuration
} HWI2C_Info;

/// I2C status
typedef struct _HWI2C_Status
{
    U8       u8DbgLevel;   /// debug level
    BOOL     bIsInit;      /// initialized
    BOOL     bIsMaster;    /// master
    HWI2C_State eState;       /// state
} HWI2C_Status;


////////////////////////////////////////////////////////////////////////////////
// Extern function
////////////////////////////////////////////////////////////////////////////////
_extern_HAL_IIC_ U32 MsOS_PA2KSEG1(U32 addr);
_extern_HAL_IIC_ U32 MsOS_VA2PA(U32 addr);

_extern_HAL_IIC_ void HAL_HWI2C_ExtraDelay(U32 u32Us);
_extern_HAL_IIC_ void HAL_HWI2C_SetIOMapBase(U8 u8Port, U32 u32Base, U32 u32ChipBase, U32 u32ClkBase);
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
_extern_HAL_IIC_ BOOL HAL_HWI2C_SetPortRegOffset(U8 u8Port, U16* pu16Offset);
_extern_HAL_IIC_ BOOL HAL_HWI2C_GetPortIdxByOffset(U16 u16Offset, U8* pu8Port);
_extern_HAL_IIC_ BOOL HAL_HWI2C_GetPortIdxByPort(HAL_HWI2C_PORT ePort, U8* pu8Port);
_extern_HAL_IIC_ BOOL HAL_HWI2C_SelectPort(HAL_HWI2C_PORT ePort);
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
_extern_HAL_IIC_ void Hal_HW_IIC_Init(volatile void *base, volatile void *chipbase, unsigned int i2cgroup, volatile void *clkbase, int i2cpadmux);
_extern_HAL_IIC_ BOOL Hal_HWI2C_Init(HWI2C_UnitCfg *psCfg);
_extern_HAL_IIC_ BOOL _Hal_HWI2C_InitPort(HWI2C_UnitCfg *psCfg);
_extern_HAL_IIC_ BOOL HAL_HWI2C_WriteClk2Byte(U32 u32RegAddr, U16 u16Val);
_extern_HAL_IIC_ U16 HAL_HWI2C_ReadClk2Byte(U32 u32RegAddr);
_extern_HAL_IIC_ U32 HAL_HWI2C_XferWrite(u8 u8Port, u8 u8slaveAddr, u32 addr, int alen, u8 *pbuf, int length);
_extern_HAL_IIC_ U32 HAL_HWI2C_XferRead(u8 u8Port, u8 u8slaveAddr, u32 addr, int alen, u8 *pbuf, int length);
_extern_HAL_IIC_ I2CErrorCode_e HalI2cInitStandard(void);
_extern_HAL_IIC_ I2CErrorCode_e HalI2cSendDataStandard(u8 nPortNum, u8 nAddr, u8 nSpeed, bool bReStart, s32 nNbItems, const u8 *pDataToWrite, bool bStopBit);
_extern_HAL_IIC_ I2CErrorCode_e HalI2cReceiveStandard(u8 nPortNum, u8 nAddr, u8 nSpeed, bool bReStart, u32 nNbItemsToWrite, u8 *pDataToWrite, u32 nNbItemsToRead, u8 *pDataToRead);

#endif //__HAL_BUS_I2C_H__

