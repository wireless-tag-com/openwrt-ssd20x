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
#define FLASH_IC_UNKNOWN        0x0000
#define FLASH_IC_SST25VF080B    0x0100      // 1M    SST
#define FLASH_IC_SST25VF016B    0x0101      // 2M
#define FLASH_IC_SST25VF032B    0x0102      // 2M
#define FLASH_IC_PM25LV080      0x0200      // 1M    PMC
#define FLASH_IC_PM25LV016      0x0201      // 2M
#define FLASH_IC_ST25P80_A      0x0300      // 1M    ST
#define FLASH_IC_ST25P16_A      0x0301      // 2M
#define FLASH_IC_STM25P32       0x0302      // 4M
#define FLASH_IC_AT26DF081A     0x0400      // 1M    ATMEL
#define FLASH_IC_AT26DF161      0x0401      // 2M
#define FLASH_IC_AT26DF321      0x0402      // 4M
#define FLASH_IC_AT25DF321A     0x0403      // 4M
#define FLASH_IC_AT25DF641      0x0404      // 8M
#define FLASH_IC_MX25L8005      0x0500      // 1M    MXIC
#define FLASH_IC_MX25L1605A     0x0501      // 2M
#define FLASH_IC_MX25L3205D     0x0502      // 4M
#define FLASH_IC_MX25L6405D     0x0503      // 8M
#define FLASH_IC_MX25L12805D    0x0504      // 16M
#define FLASH_IC_MX25L6445E     0x0505      // 8M
#define FLASH_IC_MX25L12845E    0x0506      // 16M
#define FLASH_IC_MX25L1606E     0x0507      // 2M
#define FLASH_IC_MX25L3206E     0x0508      // 4M
#define FLASH_IC_MX25L6406E     0x0509      // 8M

#define FLASH_IC_NX25P80        0x0600      // 1M    NX
#define FLASH_IC_NX25P16        0x0601      // 2M
#define FLASH_IC_W25X80         0x0700      // 1M    WINB
#define FLASH_IC_W25X16         0x0701      // 2M
#define FLASH_IC_W25Q16         0x0711      // 2M
#define FLASH_IC_W25X32         0x0702      // 4M
#define FLASH_IC_W25Q32         0x0703      // 4M
#define FLASH_IC_W25X64         0x0704      // 8M
#define FLASH_IC_W25Q64         0x0705      // 8M
#define FLASH_IC_W25Q128        0x0706      // 16M
#define FLASH_IC_W25Q80         0x0707      // 1M
#define FLASH_IC_W25Q64CV       0x0708      // 8M
#define FLASH_IC_W25Q32BV       0x0709      // 4M

#define FLASH_IC_S25FL008A      0x0800      // 1M    SPANSION
#define FLASH_IC_S25FL016A      0x0801      // 2M
#define FLASH_IC_S25FL128P      0x0802      // 16M
#define FLASH_IC_S25FL032P      0x0803      // 4M
#define FLASH_IC_S25FL064P      0x0804      // 8M
#define FLASH_IC_S25FL032K      0x0805      // 4M

#define FLASH_IC_EN25B20T       0x0900      // 2M    EON
#define FLASH_IC_EN25B20B       0x0901      // 2M
#define FLASH_IC_EN25B10T       0x0902      // 1M
#define FLASH_IC_EN25B10B       0x0903      // 1M
#define FLASH_IC_EN25B32B       0x0904      // 4M    (Bottom Boot)
#define FLASH_IC_EN25B64B       0x0905      // 4M
#define FLASH_IC_EN25Q32A       0x0906      // 4M
#define FLASH_IC_EN25Q128       0x0907      // 16M
#define FLASH_IC_EN25Q32B       0x0908      // 4M
#define FLASH_IC_EN25Q64        0x0909      // 4M
#define FLASH_IC_EN25F16        0x0910	     // 2M
#define FLASH_IC_EN25F32        0x0911      // 4M
#define FLASH_IC_EN25F80        0x0912      // 1M
#define FLASH_IC_EN25P16        0x0913      // 2M
#define FLASH_IC_EN25F10        0x0999      // 128K for secure boot
#define FLASH_IC_ESMT_F25L016A  0x0A00      // 2M
#define FLASH_IC_EN25Q32        0x0A01      // 2M
#define FLASH_IC_EN25Q16        0x0A02      // 2M
#define FLASH_IC_GD25Q32        0x0B00      // 4M
#define FLASH_IC_PM25LQ032C      0x0D00      // 4M
#define FLASH_IC_MICRON_M25P16   0x0E00      // 2M
#define FLASH_IC_MICRON_N25Q32   0x0E01      // 4M
#define FLASH_IC_MICRON_N25Q64   0x0E02      // 8M
#define FLASH_IC_MICRON_N25Q128  0x0E03      // 16M


// Flash Manufacture ID
#define MID_MXIC                0xC2
#define MID_WB                  0xEF
#define MID_EON                 0x1C
#define MID_ST                  0x20
#define MID_SST                 0xBF
#define MID_PMC                 0x9D
#define MID_ATMEL               0x1F
#define MID_SPAN                0x01
#define MID_GD                  0xC8
#define MID_MICRON              0x20

// Flash Storage Size
#define SIZE_1KB                0x400
#define SIZE_2KB                0x800
#define SIZE_4KB                0x1000
#define SIZE_8KB                0x2000
#define SIZE_16KB               0x4000
#define SIZE_32KB               0x8000
#define SIZE_64KB               0x10000
#define SIZE_128KB              0x20000
#define SIZE_256KB              0x40000
#define SIZE_512KB              0x80000
#define SIZE_1MB                0x100000
#define SIZE_2MB                0x200000
#define SIZE_4MB                0x400000
#define SIZE_8MB                0x800000
#define SIZE_16MB               0x1000000

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

typedef struct
{
    MS_U8   u8BlockProtectBits;  // Block Protection Bits
    MS_U32  u32LowerBound;
    MS_U32  u32UpperBound;
} ST_WRITE_PROTECT;

typedef struct
{
    MS_U16  u16Start;           // Start block # of special block size
    MS_U16  u16End;             // End   block # of special block size
    MS_U32  au32SizeList[8];    // List of special block sizes. Total size must be equal to FLASH_BLOCK_SIZE
} ST_SPECIAL_BLOCKS;

typedef struct
{
    MS_U16  u16FlashType;   // flash type
    MS_U8   u8MID;          // Manufacture ID
    MS_U8   u8DID0;         // Device ID (memory type)
    MS_U8   u8DID1;         // Device ID (memory capacity)

    ST_WRITE_PROTECT   *pWriteProtectTable;
    ST_SPECIAL_BLOCKS  *pSpecialBlocks;

    MS_U32  u32FlashSize;               // Flash Size
    MS_U32  u32NumSec;                  // NUMBER_OF_SERFLASH_SECTORS                  // number of sectors
    MS_U32  u32SecSize;                 // SERFLASH_SECTOR_SIZE                                // sector size
    MS_U16  u16PageSize;                // SERFLASH_PAGE_SIZE                                    // page size
    MS_U16  u16MaxChipWrDoneTimeout;    // SERFLASH_MAX_CHIP_WR_DONE_TIMEOUT    // max timeout for chip write done
    MS_U8   u8WrsrBlkProtect;           // SERFLASH_WRSR_BLK_PROTECT                     // BP bits @ Serial Flash Status Register
    MS_U16  u16DevSel;                  // ISP_DEV_SEL                                                  // reg_device_select
    MS_U16  u16SpiEndianSel;            // ISP_SPI_ENDIAN_SEL                                     // reg_endian_sel_spi
    MS_BOOL b2XREAD;
} hal_SERFLASH_t;

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
extern MS_BOOL HAL_SERFLASH_ReadREMS4(MS_U8 * pu8Data, MS_U32 u32Size);
extern MS_BOOL HAL_SERFLASH_DMA(MS_U32 u32FlashStart, MS_U32 u32DRASstart, MS_U32 u32Size);
extern MS_BOOL HAL_SERFLASH_ReadStatusReg(MS_U8 *pu8StatusReg);
extern MS_BOOL HAL_SERFLASH_ReadStatusReg2(MS_U8 *pu8StatusReg);
extern MS_BOOL HAL_SERFLASH_WriteStatusReg(MS_U16 u16StatusReg);
//#if MXIC_ONLY
extern MS_BOOL HAL_SPI_EnterIBPM(void);
extern MS_BOOL HAL_SPI_SingleBlockLock(MS_PHYADDR u32FlashAddr, MS_BOOL bLock);
extern MS_BOOL HAL_SPI_GangBlockLock(MS_BOOL bLock);
extern MS_U8 HAL_SPI_ReadBlockStatus(MS_PHYADDR u32FlashAddr);
//#endif//MXIC_ONLY
// DON'T USE THESE DIRECTLY
extern hal_SERFLASH_t _hal_SERFLASH;
extern MS_U8 _u8SERFLASHDbgLevel;
extern MS_BOOL _bIBPM;

extern MS_U8 HAL_SERFLASH_ReadStatusByFSP(void);
extern void HAL_SERFLASH_ReadWordFlashByFSP(MS_U32 u32Addr, MS_U8 *pu8Buf);
extern void HAL_SERFLASH_CheckEmptyByFSP(MS_U32 u32Addr, MS_U32 u32ChkSize);
extern void HAL_SERFLASH_EraseSectorByFSP(MS_U32 u32Addr);
extern void HAL_SERFLASH_EraseBlock32KByFSP(MS_U32 u32Addr);
extern void HAL_SERFLASH_EraseBlock64KByFSP(MS_U32 u32Addr);
extern void HAL_SERFLASH_ProgramFlashByFSP(MS_U32 u32Addr, MS_U32 u32Data);

#endif // _HAL_SERFLASH_H_
