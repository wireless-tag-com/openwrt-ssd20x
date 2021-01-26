/*
* drvDeviceInfo.h- Sigmastar
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

#ifndef _DRV_DEVICE_INFO_H_
#define _DRV_DEVICE_INFO_H_
//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

// Flash IC


#define FLASH_IC_UNKNOWN         0x0000UL
// SST
#define FLASH_IC_SST25VF080B     0x0100UL      // 1M    SST
#define FLASH_IC_SST25VF016B     0x0101UL      // 2M
#define FLASH_IC_SST25VF032B     0x0102UL      // 2M

#define FLASH_IC_PM25LV080       0x0200UL      // 1M    PMC
#define FLASH_IC_PM25LV016       0x0201UL      // 2M
#define FLASH_IC_PM25LQ032C      0x0202UL      // 4M

#define FLASH_IC_ST25P80_A       0x0300UL      // 1M    ST
#define FLASH_IC_ST25P16_A       0x0301UL      // 2M
#define FLASH_IC_STM25P32        0x0302UL      // 4M
//ATMEL
#define FLASH_IC_AT26DF081A      0x0400UL      // 1M    ATMEL
#define FLASH_IC_AT26DF161       0x0401UL      // 2M
#define FLASH_IC_AT26DF321       0x0402UL      // 4M
#define FLASH_IC_AT25DF321A      0x0403UL      // 4M

// MXIC
#define FLASH_IC_MX25L8005       0x0500UL      // 1M    MXIC
#define FLASH_IC_MX25L1655D      0x0501UL      // 2M for secure option
#define FLASH_IC_MX25L1606E      0x0502UL      // 2M
#define FLASH_IC_MX25L1605A      0x0503UL      // 2M
#define FLASH_IC_MX25L3206E      0x0504UL      // 4M
#define FLASH_IC_MX25L3205D      0x0505UL      // 4M
#define FLASH_IC_MX25L6405D      0x0506UL      // 8M
#define FLASH_IC_MX25L6406E      0x0507UL      // 8M
#define FLASH_IC_MX25L12805D     0x0508UL      // 16M
#define FLASH_IC_MX25L12845E     0x0509UL      // 16M
#define FLASH_IC_MX25L6445E      0x050AUL      // 8M
#define FLASH_IC_MX25L6455E      0x050BUL      // 8M
#define FLASH_IC_MX25L12855E     0x050CUL      // 16M
#define FLASH_IC_MX25L4006E      0x050DUL      // 512K    MXIC
#define FLASH_IC_MX25L8006E      0x050EUL      // 1M    MXIC
#define FLASH_IC_MX25L3255D      0x0512UL      // 4M for secure option
#define FLASH_IC_MX25L25645G     0x0513UL      // 32M
#define FLASH_IC_MX25L5123G      0x5014UL      //64M /MX25L51245G


// NX
#define FLASH_IC_NX25P80         0x0600UL      // 1M    NX
#define FLASH_IC_NX25P16         0x0601UL      // 2M
#define FLASH_IC_FS25Q128F2TFI   0x0602UL      //16M  aaron add
#define FLASH_IC_XT25F64B        0x0603UL      //8M   arron add
#define FLASH_IC_XT25F128B       0x0604UL      //16M
// WINB
#define FLASH_IC_W25X80          0x0700UL      // 1M    WINB
#define FLASH_IC_W25Q80          0x0701UL      // 1M
#define FLASH_IC_W25Q16          0x0702UL      // 2M
#define FLASH_IC_W25X16          0x0703UL      // 2M
#define FLASH_IC_W25X32          0x0704UL      // 4M
#define FLASH_IC_W25Q32          0x0705UL      // 4M
#define FLASH_IC_W25X64          0x0706UL      // 8M
#define FLASH_IC_W25Q64          0x0707UL      // 8M
#define FLASH_IC_W25Q64CV        0x0708UL      // 8M
#define FLASH_IC_W25Q128         0x0709UL      // 16M
#define FLASH_IC_W25Q32BV        0x070AUL      // 4M
#define FLASH_IC_W25X40          0x070BUL      // 512K    WINB
#define FLASH_IC_W25Q256JV       0x070CUL      // 32M

//SPANSION
#define FLASH_IC_S25FL008A       0x0800UL      // 1M    SPANSION
#define FLASH_IC_S25FL016A       0x0801UL      // 2M
#define FLASH_IC_S25FL128P       0x0802UL      // 16M
#define FLASH_IC_S25FL032P       0x0803UL      // 4M
#define FLASH_IC_S25FL064P       0x0804UL      // 8M
#define FLASH_IC_S25FL032K       0x0805UL      // 4M
#define FLASH_IC_S25FL032        0x0804UL      // 4M

// EON
#define FLASH_IC_EN25F10         0x0900UL      // 128K for secure boot
#define FLASH_IC_EN25F16         0x0901UL      // 2M
#define FLASH_IC_EN25F32         0x0902UL      // 4M
#define FLASH_IC_EN25F80         0x0903UL      // 1M
#define FLASH_IC_EN25B20T        0x0904UL      // 2M    EON
#define FLASH_IC_EN25B20B        0x0905UL      // 2M
#define FLASH_IC_EN25B10T        0x0906UL      // 1M
#define FLASH_IC_EN25B10B        0x0907UL      // 1M
#define FLASH_IC_EN25B32B        0x0908UL      // 4M    (Bottom Boot)
#define FLASH_IC_EN25Q32         0x0909UL      // 4M
#define FLASH_IC_EN25Q32A        0x090AUL      // 4M
#define FLASH_IC_EN25Q64         0x090BUL      // 4M
#define FLASH_IC_EN25B64B        0x090CUL      // 4M
#define FLASH_IC_EN25Q128        0x090DUL      // 16M
#define FLASH_IC_EN25QH128A      0x090FUL      // 16M
#define FLASH_IC_EN25P16         0x090EUL      // 2M
#define FLASH_IC_EN25F40         0x090AUL      // 512k
#define FLASH_IC_EN25QH16        0x090BUL      // 2M
#define FLASH_IC_EN25QH64A       0x090CUL      // 8M  aaron add
// ESMT
#define FLASH_IC_ESMT_F25L016A   0x0A00UL      // 2M
// GD
#define FLASH_IC_GD25Q32         0x0B00UL      // 4M
#define FLASH_IC_GD25Q16         0x0B01UL      // 2MByte
#define FLASH_IC_GD25S80         0x0B02UL
#define FLASH_IC_GD25Q64         0x0B03UL      // 8M
#define FLASH_IC_GD25Q128        0x0B05UL      // 16M
#define FLASH_IC_KH25L6406E      0x0B08UL      // 8M aaron add
#define FLASH_IC_PN25F128B       0x0C00UL      // 16M //aaron add
#define FLASH_IC_PN25F64B        0x0C01UL      // 8M //aaron add
#define FLASH_IC_GD25Q256C       0x0B06UL      // 32M
#define FLASH_IC_XT25F128A		 0x0C05UL  	   // 16M  aaron add
//XM
#define FLASH_IC_XM25QH128A      0x0C06UL	   // 16M aaron add
#define FLASH_IC_XM25QH64A       0x0C07UL      // 8M aaron add
#define FLASH_IC_XM25QH128B      0x0C08UL      // 16M
#define FLASH_IC_XM25QH64B       0x0C09UL      // 8M
#define FLASH_IC_XM25QH256B      0x0C10UL      //32M
// AM
#define FLASH_IC_A25LM032        0x0D00UL

// MICRON
#define FLASH_IC_M25P16   0x0E00UL      // 2M
#define FLASH_IC_N25Q32   0x0E01UL      // 4M
#define FLASH_IC_N25Q64   0x0E02UL      // 8M
#define FLASH_IC_N25Q128  0x0E03UL      // 16M
#define FLASH_IC_M25PX16  0x0E04UL      // 2M


#define FLASH_IC_IC25LP128      0x0F00UL      // 16M

//ZB
#define FLASH_IC_ZB25Q64        0x1002UL      // 8M
#define FLASH_IC_ZB25Q128       0x1003UL      // 16M
#define FLASH_IC_ZB25VQ128      0x1004UL

#define FLASH_IC_25Q128AS        0x1100UL     //16M

// Flash Manufacture ID
#define MID_MXIC                0xC2UL
#define MID_WB                  0xEFUL
#define MID_EON                 0x1CUL
#define MID_ST                  0x20UL
#define MID_SST                 0xBFUL
#define MID_PMC                 0x9DUL
#define MID_ATMEL               0x1FUL
#define MID_SPAN                0x01UL
#define MID_GD                  0xC8UL
#define MID_MICRON              0x20UL
#define MID_NUMONYX             0x20UL
#define MID_AM                  0x37UL
#define MID_IC                  0x9DUL
#define MID_FS                  0xA1UL
#define MID_EN                  0x1CUL
#define MID_XT1                 0x0BUL
#define MID_PN			        0x1CUL  //aaron add
#define MID_XT			        0x20UL	//aaron add
#define MID_XMC			        0x20UL	
#define MID_KH			        0xC2UL
#define MID_25Q                 0x68UL//25q128as
#define MID_ZB                  0x5EUL
// Flash Storage Size
#define SIZE_1KB                0x400UL
#define SIZE_2KB                0x800UL
#define SIZE_4KB                0x1000UL
#define SIZE_8KB                0x2000UL
#define SIZE_16KB               0x4000UL
#define SIZE_32KB               0x8000UL
#define SIZE_64KB               0x10000UL
#define SIZE_128KB              0x20000UL
#define SIZE_256KB              0x40000UL
#define SIZE_512KB              0x80000UL
#define SIZE_1MB                0x100000UL
#define SIZE_2MB                0x200000UL
#define SIZE_4MB                0x400000UL
#define SIZE_8MB                0x800000UL
#define SIZE_16MB               0x1000000UL

// ISP_DEV_SEL
#define ISP_DEV_PMC             BITS(2:0, 0)
#define ISP_DEV_NEXTFLASH       BITS(2:0, 1)
#define ISP_DEV_ST              BITS(2:0, 2)
#define ISP_DEV_SST             BITS(2:0, 3)
#define ISP_DEV_ATMEL           BITS(2:0, 4)

// ISP_SPI_ENDIAN_SEL
#define ISP_SPI_ENDIAN_BIG      BITS(0:0, 1)
#define ISP_SPI_ENDIAN_LITTLE   BITS(0:0, 0)

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
    MS_U16  u16SPIMaxClk[2];
    MS_BOOL b2XREAD;
    MS_BOOL b4XREAD;
} hal_SERFLASH_t;
#endif //_DRV_DEVICE_INFO_H_

