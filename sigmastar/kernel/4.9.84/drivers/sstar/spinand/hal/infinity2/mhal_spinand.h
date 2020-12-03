/*
* mhal_spinand.h- Sigmastar
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
#ifndef _HAL_SPINAND_H_
#define _HAL_SPINAND_H_
#include "mdrv_spinand_common.h"

//-------------------------------------------------------------------------------------------------
//  Structures definition
//-------------------------------------------------------------------------------------------------

typedef struct
{
    U32  u32FspBaseAddr;     // REG_ISP_BASE
    U32  u32QspiBaseAddr;    // REG_QSPI_BASE
    U32  u32PMBaseAddr;      // REG_PM_BASE
    U32  u32CLK0BaseAddr;    // REG_PM_BASE
    U32  u32CHIPBaseAddr;    // REG_CHIP_BASE
    U32  u32RiuBaseAddr;     // REG_PM_BASE
    U32  u32BDMABaseAddr;    // REG_BDMA_BASE
} hal_fsp_t;

//-------------------------------------------------------------------------------------------------
//  Macro definition
//-------------------------------------------------------------------------------------------------
#define READ_WORD(_reg)                     (*(volatile U16*)(_reg))
#define WRITE_WORD(_reg, _val)              { (*((volatile U16*)(_reg))) = (U16)(_val); }
#define WRITE_WORD_MASK(_reg, _val, _mask)  { (*((volatile U16*)(_reg))) = ((*((volatile U16*)(_reg))) & ~(_mask)) | ((U16)(_val) & (_mask)); }
#define BDMA_READ(addr)                     READ_WORD(_hal_fsp.u32BDMABaseAddr + (addr<<2))
#define BDMA_WRITE(addr, val)               WRITE_WORD(_hal_fsp.u32BDMABaseAddr + (addr<<2),(val))
#define QSPI_READ(addr)                     READ_WORD(_hal_fsp.u32QspiBaseAddr + (addr<<2))
#define QSPI_WRITE(addr, val)               WRITE_WORD(_hal_fsp.u32QspiBaseAddr + (addr<<2),(val))
#define CLK_READ(addr)                      READ_WORD(_hal_fsp.u32CLK0BaseAddr + (addr<<2))
#define CLK_WRITE(addr, val)                WRITE_WORD(_hal_fsp.u32CLK0BaseAddr + (addr<<2),(val))
#define CLK_WRITE_MASK(addr, val, mask)     WRITE_WORD_MASK(_hal_fsp.u32CLK0BaseAddr + ((addr)<<2), (val), (mask))
#define CHIP_READ(addr)                     READ_WORD(_hal_fsp.u32CHIPBaseAddr + (addr<<2))
#define CHIP_WRITE(addr, val)               WRITE_WORD(_hal_fsp.u32CHIPBaseAddr + (addr<<2),(val))
#define CHIP_WRITE_MASK(addr, val, mask)    WRITE_WORD_MASK(_hal_fsp.u32CHIPBaseAddr + ((addr)<<2), (val), (mask))
#define PM_WRITE_MASK(addr, val, mask)      WRITE_WORD_MASK(_hal_fsp.u32PMBaseAddr+ ((addr)<<2), (val), (mask))
#define PM_READ(addr)                       READ_WORD(_hal_fsp.u32PMBaseAddr + (addr<<2))
#define PM_WRITE(addr, val)                 WRITE_WORD(_hal_fsp.u32PMBaseAddr + (addr<<2),(val))
#define ISP_READ(addr)                      READ_WORD(_hal_fsp.u32ISPBaseAddr + (addr<<2))

// Read method
#define RIU_FLAG FALSE
#define BDMA_FLAG FALSE
#define XIP_FLAG TRUE
// Write method
#define BDMA_W_FLAG FALSE
#define DENSITY_2G   2048
#define MS_SPI_ADDR             0x14000000
//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
extern SPINAND_FLASH_INFO_t _gtSpinandInfo;
extern U8 _u8SPINANDDbgLevel;
void HAL_SPINAND_DieSelect(U8 u8Die);
U8 HAL_SPINAND_ReadStatusRegister(U8 *u8Status, U8 u8Addr);
U8 HAL_SPINAND_WriteStatusRegister(U8 *u8Status, U8 u8Addr);
U32  HAL_SPINAND_Read (U32 u32Addr, U32 u32DataSize, U8 *pu8Data);
U32  HAL_SPINAND_Write( U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf);
U32  HAL_SPINAND_ReadID(U32 u32DataSize, U8 *pu8Data);
U32  HAL_SPINAND_RFC(U32 u32Addr, U8 *pu8Data);
void HAL_SPINAND_Config(U32 u32PMRegBaseAddr, U32 u32NonPMRegBaseAddr);
U32  HAL_SPINAND_BLOCKERASE(U32 u32_PageIdx);
U32  HAL_SPINAND_Init(void);
U32  HAL_SPINAND_WriteProtect(BOOL bEnable);
BOOL HAL_SPINAND_PLANE_HANDLER(U32 u32Addr);
U32  HAL_SPINAND_SetMode(SPINAND_MODE eMode);
BOOL HAL_SPINAND_SetCKG(U8 u8CkgSpi);
void HAL_SPINAND_CSCONFIG(void);
BOOL HAL_SPINAND_IsActive(void);
U32 HAL_SPINAND_RIU_READ(U16 u16Addr, U32 u32DataSize, U8 *u8pData);
U8 HAL_QSPI_FOR_DEBUG(void);
void HAL_SPINAND_Chip_Config(void);


//-------------------------------------------------------------------------------------------------
//  System Data Type
//-------------------------------------------------------------------------------------------------
/// data type unsigned char, data length 1 byte
#define MS_U8       unsigned char                                            // 1 byte
/// data type unsigned short, data length 2 byte
#define MS_U16      unsigned short                                           // 2 bytes
/// data type unsigned int, data length 4 byte
#define MS_U32      unsigned long                                            // 4 bytes
/// data type unsigned int, data length 8 byte
#define MS_U64      unsigned long long                                       // 8 bytes
/// data type signed char, data length 1 byte
#define MS_S8       signed char                                              // 1 byte
/// data type signed short, data length 2 byte
#define MS_S16      signed short                                             // 2 bytes
/// data type signed int, data length 4 byte
#define MS_S32      signed long                                              // 4 bytes
/// data type signed int, data length 8 byte
#define MS_S64      signed long long                                         // 8 bytes
/// data type float, data length 4 byte
#define MS_FLOAT    float                                                    // 4 bytes
/// data type null pointer
#ifdef NULL
#undef NULL
#endif
#define NULL                        0

#define MS_BOOL     unsigned char

/// data type hardware physical address
#define MS_PHYADDR unsigned long                                    // 32bit physical address



#endif // _HAL_SPINAND_H_
