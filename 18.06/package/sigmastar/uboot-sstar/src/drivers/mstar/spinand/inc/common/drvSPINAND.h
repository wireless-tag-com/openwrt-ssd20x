/*
* drvSPINAND.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: XXXX <XXXX@sigmastar.com.tw>
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


#ifndef _DRV_SPINAND_H_
#define _DRV_SPINAND_H_
#include "drvSPINAND_common.h"
#include "../MsTypes.h"
#include "../config/drvSPINAND_config.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define MID_GD          0xC8
#define MID_MICRON      0x2C
#define MID_ATO         0x9B
#define MID_WINBOND     0xEF
#define MID_MXIC        0xC2
#define MID_TOSHIBA     0x98
#define MID_XTX         0x0B

#define SPINAND_READ_SUBPAGE 1

typedef enum _SPINAND_ERROR_NUM
{
    ERR_SPINAND_SUCCESS,
    ERR_SPINAND_RESET_FAIL,
    ERR_SPINAND_TIMEOUT,
    ERR_SPINAND_BDMA_TIMEOUT,
    ERR_SPINAND_BAD_BLK,
    ERR_SPINAND_E_FAIL,
    ERR_SPINAND_W_FAIL,
    ERR_SPINAND_INVALID,
    ERR_SPINAND_UNKNOWN_ID,
    ERR_SPINAND_ECC_1_3_CORRECTED,
    ERR_SPINAND_ECC_4_6_CORRECTED,
    ERR_SPINAND_ECC_7_8_CORRECTED,
    ERR_SPINAND_ECC_ERROR,
} SPINAND_FLASH_ERRNO_e;

typedef enum
{
    E_SPINAND_SINGLE_MODE,
    E_SPINAND_FAST_MODE,
    E_SPINAND_DUAL_MODE,
    E_SPINAND_DUAL_MODE_IO,
    E_SPINAND_QUAD_MODE,
    E_SPINAND_QUAD_MODE_IO,
}SPINAND_MODE;

typedef struct
{
    U8   u8_IDByteCnt;
    U8   au8_ID[15];
    U16  u16_SpareByteCnt;
    U16  u16_PageByteCnt;
    U16  u16_BlkPageCnt;
    U16  u16_BlkCnt;
    U16  u16_SectorByteCnt;
    U8   u8PlaneCnt;
    U8   u8WrapConfig;
    BOOL bRIURead;
    U8   u8CLKConfig;
    U8   u8_UBOOTPBA;
    U8   u8_BL0PBA;
    U8   u8_BL1PBA;
    U8   u8_HashPBA[3][2];
    U8   u8ReadMode;
    U8   u8WriteMode;
} SPINAND_FLASH_INFO_t;

typedef U32 (*Mdev_SPINAND_SetDriving)(U16 u16Driving);

#define DEBUG_SPINAND(debug_level, x)     do { if (_u8SPINANDDbgLevel >= (debug_level)) (x); } while(0)

int MDrv_SPINAND_WriteOtp(U8* pDataBuf, U32 nOffset, U32 nLen);

extern BOOL MDrv_SPINAND_Init(SPINAND_FLASH_INFO_t *tSpinandInfo);
extern U32 MDrv_SPINAND_Read(U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf);
extern U32 MDrv_SPINAND_Write(U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf);
extern U32 MDrv_SPINAND_BLOCK_ERASE(U32 u32_BlkIdx);
extern U8 MDrv_SPINAND_ReadID(U16 u16Size, U8 *u8Data);
extern void SpiNandMain(unsigned int dwSramAddress, unsigned int dwSramSize);
extern U32 MDrv_SPINAND_SetMode(SPINAND_MODE eMode);
extern void MDrv_SPINAND_ForceInit(SPINAND_FLASH_INFO_t *tSpinandInfo);
extern U32 MDrv_SPINAND_Read_RandomIn(U32 u32_PageIdx, U32 u32_Column, U32 u32_Byte, U8 *u8Data);
extern U32 MDrv_SPINAND_ReadStatusRegister(MS_U8 *u8Status, MS_U8 u8Addr);
extern U32 MDrv_SPINAND_WriteStatusRegister(MS_U8 *u8Status, MS_U8 u8Addr);
U32 MDrv_SPINAND_EnableOtp(BOOL bEnable);
U32 MDrv_SPINAND_LockOtp(void);
U8 MDrv_SPINAND_WB_BBM(U32 u32LBA, U32 u32PBA);
extern BOOL MDrv_SPINAND_SetDevDriving(U16 u16Driving);
extern U32 HAL_SPINAND_Read (U32 u32Addr, U32 u32DataSize, U8 *pu8Data);
extern U32 HAL_SPINAND_Write( U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf);
extern U32  HAL_SPINAND_ReadID(U32 u32DataSize, U8 *pu8Data);
extern U32 HAL_SPINAND_RFC(U32 u32Addr, U8 *pu8Data);
extern void HAL_SPINAND_Config(U32 u32PMRegBaseAddr, U32 u32NonPMRegBaseAddr);
extern U32 HAL_SPINAND_BLOCKERASE(U32 u32_PageIdx);
extern U32 HAL_SPINAND_Init(void);
extern U32 HAL_SPINAND_WriteProtect(BOOL bEnable);
extern BOOL HAL_SPINAND_PLANE_HANDLER(U32 u32Addr);
extern void HAL_SPINAND_PreHandle(SPINAND_MODE eMode);
extern U32 HAL_SPINAND_SetMode(SPINAND_MODE eMode);
extern BOOL HAL_FSP_GET_SPINAND_STATUS(U8 *pu8Data);
extern void HAL_SPINAND_CSCONFIG(void);
U32 HAL_SPINAND_EnableOtp(BOOL bEnable);
U32 HAL_SPINAND_LockOtp(void);
U8 HAL_SPINAND_WB_DumpBBM(U8 *pu8Data);
U8 HAL_SPINAND_WB_BBM(U32 u32LBA, U32 u32PBA);
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: HAL_SPINAND_SetCKG()
/// @brief \b Function \b Description: This function is used to set ckg_spi dynamically
/// @param <IN>        \b eCkgSpi    : enumerate the ckg_spi
/// @param <OUT>       \b NONE    :
/// @param <RET>       \b TRUE: Success FALSE: Fail
/// @param <GLOBAL>    \b NONE    :
/// @param <NOTE>    \b : Please use this function carefully , and is restricted to Flash ability
////////////////////////////////////////////////////////////////////////////////
extern BOOL HAL_SPINAND_SetCKG(U8 u8CkgSpi);
extern BOOL HAL_SPINAND_DumpCkg(void);
extern U8 HAL_SPINAND_ReadStatusRegister(U8 *u8Status, U8 u8Addr);
extern U8 HAL_SPINAND_WriteStatusRegister(U8 u8Status, U8 u8Addr);
extern U32 HAL_SPINAND_READ_STATUS(U8 *pu8Status, U8 u8Addr);
extern U8 _u8SPINANDDbgLevel;
extern U32 HAL_SPINAND_Read_RandomIn(U32 u32_PageIdx, U32 u32_Column, U32 u32_Byte, U8 *u8Data);
extern void HAL_SPINAND_DieSelect(U8 u8Die);

#define SPINAND_ID_SIZE 3

#define RIU_FLAG    FALSE
// SPI NAND COMMAND
#define SPI_NAND_CMD_PGRD                        0x13
#define SPI_NAND_CMD_RDID                        0x9F
#define SPI_NAND_CMD_WREN                        0x06
#define SPI_NAND_CMD_WRDIS                       0x04
#define SPI_NAND_CMD_RFC                         0x03
#define SPI_NAND_CMD_PP                          0x02
#define SPI_NAND_CMD_QPP                         0x32
#define SPI_NAND_CMD_RPL                         0x84
#define SPI_NAND_CMD_PE                          0x10
#define SPI_NAND_CMD_GF                          0x0F
#define SPI_NAND_CMD_RESET                       0xFF
#define SPI_NAND_CMD_BBM                       0xA1
#define SPI_NAND_CMD_READBBM               0xA5
#define SPI_NAND_CMD_DIESELECT                   0xC2
        #define SPI_NAND_REG_PROT                0xA0
                #define SPINAND_CMP              (0x02)
                #define SPINAND_INV              (0x04)
                #define SPINAND_BP0              (0x08)
                #define SPINAND_BP1              (0x10)
                #define SPINAND_BP2              (0x20)
                #define SPINAND_BRWD             (0x80)
        #define SPI_NAND_REG_FEAT                0xB0
                #define QUAD_ENABLE              1
                #define ECC_ENABLE               (1 << 4)
                #define NAND_OTP_EN              0x40
                #define NAND_OTP_PRT             0x80
        #define SPI_NAND_REG_STAT                0xC0
                #define E_FAIL                   (0x01 << 2)
                #define P_FAIL                   (0x01 << 3)
                #define ECC_STATUS_PASS          (0x00 << 4)
                #define ECC_1_3_CORRECTED                (0x01 << 4)
                #define ECC_NOT_CORRECTED               (0x02 << 4)
                #define ECC_4_6_CORRECTED                (0x03 << 4)
                #define ECC_7_8_CORRECTED                (0x05 << 4)
                #define SPI_NAND_STAT_OIP              (0x1)
                #define LUT_FULL                       (0x01 << 6)
        #define SPI_NAND_REG_FUT                 0xD0
#define SPI_NAND_CMD_SF                          0x1F
#define SPI_NAND_CMD_BE                          0xD8
#define SPI_NAND_CMD_CE                          0xC7
#define SPI_NAND_ADDR_LEN                        3
#define SPI_NAND_PAGE_ADDR_LEN                   2
#define SPI_NAND_PLANE_OFFSET                    6
#define SPI_PLANE_ADDR_MASK                      0x40

typedef enum
{
    E_SPINAND_DBGLV_NONE,    //disable all the debug message
    E_SPINAND_DBGLV_INFO,    //information
    E_SPINAND_DBGLV_NOTICE,  //normal but significant condition
    E_SPINAND_DBGLV_WARNING, //warning conditions
    E_SPINAND_DBGLV_ERR,     //error conditions
    E_SPINAND_DBGLV_CRIT,    //critical conditions
    E_SPINAND_ALERT,   //action must be taken immediately
    E_SPINAND_DBGLV_EMERG,   //system is unusable
    E_SPINAND_DBGLV_DEBUG,   //debug-level messages
} SPINAND_DbgLv;

typedef enum
{
    E_SPI_PIN_CZ = 0x1,
    E_SPI_PIN_CK = 0x2,
    E_SPI_PIN_DI = 0x4,
    E_SPI_PIN_DO = 0x8,
    E_SPI_PIN_WPZ = 0x10,
    E_SPI_PIN_HOLDZ = 0x20,
    E_SPI_PIN_ALL = 0X3F,
    E_SPI_PIN_D0 = E_SPI_PIN_DI,
    E_SPI_PIN_D1 = E_SPI_PIN_DO,
    E_SPI_PIN_D2 = E_SPI_PIN_WPZ,
    E_SPI_PIN_D3 = E_SPI_PIN_HOLDZ,
} SPI_Pins;
extern BOOL PalSpinand_SetDriving(U32 u32Pins, u16 u16Driving);

U32 GD_SPINAND_SetDriving(U16);
#ifdef __cplusplus
}
#endif

#endif
