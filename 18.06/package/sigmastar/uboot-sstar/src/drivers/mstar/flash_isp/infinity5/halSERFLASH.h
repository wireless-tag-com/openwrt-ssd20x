/*
* halSERFLASH.h- Sigmastar
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

#ifndef _HAL_SERFLASH_H_
#define _HAL_SERFLASH_H_

#include "MsTypes.h"

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

// Flash IC



#ifndef UNUSED
#define UNUSED(x) ((x)=(x))
#endif

#define MSOS_TYPE_LINUX 1
// ISP_DEV_SEL
#define ISP_DEV_PMC             BITS(2:0, 0)
#define ISP_DEV_NEXTFLASH       BITS(2:0, 1)
#define ISP_DEV_ST              BITS(2:0, 2)
#define ISP_DEV_SST             BITS(2:0, 3)
#define ISP_DEV_ATMEL           BITS(2:0, 4)

// ISP_SPI_ENDIAN_SEL
#define ISP_SPI_ENDIAN_BIG      BITS(0:0, 1)
#define ISP_SPI_ENDIAN_LITTLE   BITS(0:0, 0)


#define NUMBER_OF_SERFLASH_SECTORS          (_hal_SERFLASH.u32NumSec)
#define SERFLASH_SECTOR_SIZE                (_hal_SERFLASH.u32SecSize)
#define SERFLASH_PAGE_SIZE                  (_hal_SERFLASH.u16PageSize)
#define SERFLASH_MAX_CHIP_WR_DONE_TIMEOUT   (_hal_SERFLASH.u16MaxChipWrDoneTimeout)
#define SERFLASH_WRSR_BLK_PROTECT           (_hal_SERFLASH.u8WrsrBlkProtect)
#define ISP_DEV_SEL                         (_hal_SERFLASH.u16DevSel)
#define ISP_SPI_ENDIAN_SEL                  (_hal_SERFLASH.u16SpiEndianSel)


#define DEBUG_SER_FLASH(debug_level, x)     do { if (_u8SERFLASHDbgLevel >= (debug_level)) (x); } while(0)
#define WAIT_SFSH_CS_STAT()             {while(ISP_READ(REG_ISP_SPI_CHIP_SELE_BUSY) == SFSH_CHIP_SELE_SWITCH){}}

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

typedef enum
{
    FLASH_ID0       = 0x00,
    FLASH_ID1       = 0x01,
    FLASH_ID2       = 0x02,
    FLASH_ID3       = 0x03
} EN_FLASH_ID;

typedef enum
{
    WP_AREA_EXACTLY_AVAILABLE,
    WP_AREA_PARTIALLY_AVAILABLE,
    WP_AREA_NOT_AVAILABLE,
    WP_TABLE_NOT_SUPPORT,
} EN_WP_AREA_EXISTED_RTN;

typedef enum
{
    FLASH_ERASE_04K       = SPI_CMD_SE,
    FLASH_ERASE_32K       = SPI_CMD_32BE,
    FLASH_ERASE_64K       = SPI_CMD_64BE
} EN_FLASH_ERASE;


typedef enum
{
    E_SINGLE_MODE,
    E_FAST_MODE,
    E_DUAL_D_MODE,
    E_DUAL_AD_MODE,
    E_QUAD_MODE,
    E_RIUISP_MODE=0xFF


}SPI_READ_MODE;


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------
extern MS_BOOL HAL_SERFLASH_SetCKG(SPI_DrvCKG eCkgSpi);
extern void HAL_SERFLASH_ClkDiv(SPI_DrvClkDiv eClkDivSpi);
extern MS_BOOL HAL_SERFLASH_SetMode(MS_BOOL bXiuRiu);
extern MS_BOOL HAL_SERFLASH_Set2XREAD(MS_BOOL b2XMode);
extern MS_BOOL HAL_SERFLASH_ChipSelect(MS_U8 u8FlashIndex);
extern void HAL_SERFLASH_Config(void);
extern void HAL_SERFLASH_Init(void);
extern void HAL_SERFLASH_SetGPIO(MS_BOOL bSwitch);
extern MS_BOOL HAL_SERFLASH_DetectType(void);
extern MS_BOOL HAL_SERFLASH_DetectSize(MS_U32  *u32FlashSize);
extern MS_BOOL HAL_SERFLASH_EraseChip(void);
extern MS_BOOL HAL_SERFLASH_AddressToBlock(MS_U32 u32FlashAddr, MS_U32 *pu32BlockIndex);
extern MS_BOOL HAL_SERFLASH_BlockToAddress(MS_U32 u32BlockIndex, MS_U32 *pu32FlashAddr);
extern MS_BOOL HAL_SERFLASH_BlockErase(MS_U32 u32StartBlock,MS_U32 u32EndBlock,MS_BOOL bWait);
extern MS_BOOL HAL_SERFLASH_SectorErase(MS_U32 u32SectorAddress);
extern MS_BOOL HAL_SERFLASH_CheckWriteDone(void);
extern MS_BOOL HAL_SERFLASH_Write(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data);
extern MS_BOOL HAL_SERFLASH_Read(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data);
extern EN_WP_AREA_EXISTED_RTN HAL_SERFLASH_WP_Area_Existed(MS_U32 u32UpperBound, MS_U32 u32LowerBound, MS_U8 *pu8BlockProtectBits);
extern MS_BOOL HAL_SERFLASH_WriteProtect_Area(MS_BOOL bEnableAllArea, MS_U8 u8BlockProtectBits);
extern MS_BOOL HAL_SERFLASH_WriteProtect(MS_BOOL bEnable);
extern MS_BOOL HAL_SERFLASH_ReadID(MS_U8 *pu8Data, MS_U32 u32Size);
extern MS_BOOL HAL_SERFLASH_ReadREMS(MS_U8 * pu8Data, MS_U32 u32Size);
//extern MS_BOOL HAL_SERFLASH_DMA(MS_U32 u32FlashStart, MS_U32 u32DRASstart, MS_U32 u32Size);
extern MS_BOOL HAL_SERFLASH_ReadStatusReg(MS_U8 *pu8StatusReg);
extern MS_BOOL HAL_SERFLASH_ReadStatusReg2(MS_U8 *pu8StatusReg);
extern MS_BOOL HAL_SERFLASH_WriteStatusReg(MS_U16 u16StatusReg);
extern MS_BOOL HAL_SERFLASH_WriteExtAddrRegister(MS_U8 u8ExtAddrReg);
//#if MXIC_ONLY
extern MS_BOOL HAL_SPI_EnterIBPM(void);
extern MS_BOOL HAL_SPI_SingleBlockLock(MS_PHYADDR u32FlashAddr, MS_BOOL bLock);
//extern MS_BOOL HAL_SPI_GangBlockLock(MS_BOOL bLock);
extern MS_U8 HAL_SPI_ReadBlockStatus(MS_PHYADDR u32FlashAddr);
//#endif//MXIC_ONLY
// DON'T USE THESE DIRECTLY
extern hal_SERFLASH_t _hal_SERFLASH;
extern MS_U8 _u8SERFLASHDbgLevel;
extern MS_BOOL _bIBPM;
extern MS_U8 _bNon4kErase;

//extern MS_U8 HAL_SERFLASH_ReadStatusByFSP(void);
//extern void HAL_SERFLASH_ReadWordFlashByFSP(MS_U32 u32Addr, MS_U8 *pu8Buf);
//extern void HAL_SERFLASH_CheckEmptyByFSP(MS_U32 u32Addr, MS_U32 u32ChkSize);
//extern void HAL_SERFLASH_EraseSectorByFSP(MS_U32 u32Addr);
//extern void HAL_SERFLASH_EraseBlock32KByFSP(MS_U32 u32Addr);
//extern void HAL_SERFLASH_EraseBlock64KByFSP(MS_U32 u32Addr);
//extern void HAL_SERFLASH_ProgramFlashByFSP(MS_U32 u32Addr, MS_U32 u32Data);


MS_BOOL HAL_FSP_BlockErase(MS_U32 u32StartBlock, MS_U32 u32EndBlock, MS_BOOL bWait);
MS_BOOL HAL_FSP_BurstRead(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data);
MS_BOOL HAL_FSP_Read(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data);
MS_BOOL HAL_FSP_SectorErase(MS_U32 u32SectorAddress,MS_U8 u8cmd);
MS_BOOL HAL_FSP_Write(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data);
MS_BOOL HAL_FSP_Write_Burst(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data);
MS_BOOL HAL_FSP_WriteProtect_Area(MS_BOOL bEnableAllArea, MS_U8 u8BlockProtectBits);
MS_BOOL HAL_FSP_WriteProtect(MS_BOOL bEnable);
MS_BOOL HAL_FSP_ReadID(MS_U8 *pu8Data, MS_U32 u32Size);
MS_BOOL HAL_FSP_ReadStatusReg(MS_U8 *pu8StatusReg);
MS_BOOL HAL_FSP_ReadStatusReg2(MS_U8 *pu8StatusReg);
MS_BOOL HAL_FSP_WriteStatusReg(MS_U16 u16StatusReg);
MS_BOOL HAL_FSP_WriteExtAddrReg(MS_U8 u8ExtAddrReg);
MS_BOOL HAL_QPI_Enable(MS_BOOL bEnable);
MS_BOOL HAL_QPI_RESET(MS_BOOL bEnable);

MS_BOOL HAL_FSP_Write_BDMA(MS_U32 u32Addr, MS_U32 u32Size, MS_U8 *pu8Data);


MS_BOOL HAL_MAP_Read(MS_U32 u32FlashAddr, MS_U32 u32FlashSize, MS_U8 *user_buffer);

#endif // _HAL_SERFLASH_H_
