/*
* mdrv_spinand.h- Sigmastar
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
#ifndef _DRV_SPINAND_H_
#define _DRV_SPINAND_H_
#include <linux/platform_device.h>
#include "inc/mdrv_spinand_common.h"
//#include "../hal/reg_spinand.h"
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

/* SPI NAND messages */
#if 1
#define spi_nand_msg(fmt, ...) printk(KERN_NOTICE "%s: " fmt "\n", __func__, ##__VA_ARGS__)
#define spi_nand_debug(fmt, ...)
#else
#define spi_nand_msg(fmt, ...)
#define spi_nand_debug(fmt, ...) printk(KERN_NOTICE "%s: " fmt "\n", __func__, ##__VA_ARGS__)
#endif
#define spi_nand_warn(fmt, ...) printk(KERN_WARNING "%s:warning, " fmt "\n", __func__, ##__VA_ARGS__)
#define spi_nand_err(fmt, ...) printk(KERN_ERR "%s:error, " fmt "\n", __func__, ##__VA_ARGS__)

#define SPINAND_FLASH_INFO_TAG "MSTARSEMIUSFDCIS"

#define DEBUG_SPINAND(debug_level, x)     do { if (_u8SPINANDDbgLevel >= (debug_level)) (x); } while(0)

#define SPINAND_MEASURE_PERFORMANCE 0
#define USE_SPINAND_INFO_TABLE 0

#define UNFD_LOGI_PART    0x8000 // bit-or if the partition needs Wear-Leveling
#define UNFD_HIDDEN_PART  0x4000 // bit-or if this partition is hidden, normally it is set for the LOGI PARTs.

//#define UNFD_PART_MIU			1
#define UNFD_PART_IPL_CUST		1
#define UNFD_PART_BOOTLOGO		2
#define UNFD_PART_IPL			3
#define UNFD_PART_OS			4
#define UNFD_PART_CUS			5
#define UNFD_PART_UBOOT			6
#define UNFD_PART_SECINFO		7
#define UNFD_PART_OTP			8
#define UNFD_PART_RECOVERY		9
#define UNFD_PART_E2PBAK		10
#define UNFD_PART_NVRAMBAK		11
#define UNFD_PART_NPT			12
#define UNFD_PART_ENV			13
#define UNFD_PART_MISC			14
#define UNFD_PART_RTOS          0x10
#define UNFD_PART_RTOS_BACKUP   0x11
#define UNFD_PART_KERNEL        0x12
#define UNFD_PART_KERNEL_BACKUP 0x13 //not used now

#define UNFD_PART_CIS           0x20
#define UNFD_PART_UBI           0x21

#define UNFD_PART_CUST0         0x30
#define UNFD_PART_CUST1         0x31
#define UNFD_PART_CUST2         0x32
#define UNFD_PART_CUST3         0x33
#define UNFD_PART_CUSTf         0x3F



struct mstar_spinand_info
{
    struct mtd_info mtd;
    struct nand_chip nand;
    struct platform_device *pdev;
    struct mtd_partition *parts;
};

    /// Suspend type
typedef enum
{
    E_MSOS_PRIORITY,            ///< Priority-order suspension
    E_MSOS_FIFO,                ///< FIFO-order suspension
} MsOSAttribute;

typedef struct
{
    U16 u16_StartBlk;		// the start block index
    U16 u16_BlkCnt;			// project team defined
    U16 u16_PartType;		// project team defined
    U16 u16_BackupBlkCnt;	// reserved good blocks count for backup, UNFD internal use.
        // e.g. u16BackupBlkCnt  = u16BlkCnt * 0.03 + 2
} SPI_NAND_PARTITION_RECORD_t;

typedef struct
{
    U32 u32_ChkSum;
    U16	u16_SpareByteCnt;
    U16	u16_PageByteCnt;
    U16	u16_BlkPageCnt;
    U16	u16_BlkCnt;
    U16 u16_PartCnt;
    U16 u16_UnitByteCnt;
    SPI_NAND_PARTITION_RECORD_t records[62];
} SPI_NAND_PARTITION_INFO_t;

typedef struct _SPI_NAND_DRIVER
{
    SPINAND_FLASH_INFO_t tSpinandInfo;

    SPI_NAND_PARTITION_INFO_t tPartInfo;
    U8 u8_HasPNI;

    U8 *pu8_pagebuf;
    U8 *pu8_sparebuf;
    U8 *pu8_statusbuf;
    U32 u32_column;
    U8 u8_status;
    U8 u8_statusRequest;
    U8 *pu8_pagechkbuf;
} SPI_NAND_DRIVER_t;

typedef struct
{
    U8  au8_Tag[16];
    SPINAND_FLASH_INFO_t tSpiNandInfo;
    U8 au8_Reserved[];
} SPINAND_FLASH_INFO_TAG_t;


//	BOOL MDrv_SPINAND_Init(SPINAND_FLASH_INFO_t *tSpinandInfo);
//	U32 MDrv_SPINAND_Read(U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf);
//	U32 MDrv_SPINAND_Write(U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf);
//	U32 MDrv_SPINAND_BLOCK_ERASE(U32 u32_BlkIdx);
//	U8 MDrv_SPINAND_ReadID(U16 u16Size, U8 *u8Data);
//	void _spiNandMain(unsigned int dwSramAddress, unsigned int dwSramSize);
//	U32 MDrv_SPINAND_SetMode(SPINAND_MODE eMode);
//	BOOL MDrv_SPINAND_ForceInit(SPINAND_FLASH_INFO_t *tSpinandInfo);
//	void MDrv_SPINAND_Device(struct device *dev);
//	U32 MDrv_SPINAND_WriteProtect(BOOL bEnable);
//	BOOL MDrv_SPINAND_IsActive(void);
//	U32 MDrv_SPINAND_Read_RandomIn(U32 u32_PageIdx, U32 u32_Column, U32 u32_Byte, U8 *u8Data);
//	U32 MDrv_SPINAND_ReadStatusRegister(U8 *u8Status, U8 u8Addr);
//
//	//	unsigned char MsOS_In_Interrupt (void);
//	inline BOOL MS_SPINAND_IN_INTERRUPT (void);
//	inline U32  MS_SPINAND_CREATE_MUTEX ( MsOSAttribute eAttribute, char *pMutexName, U32 u32Flag);
//	inline BOOL MS_SPINAND_DELETE_MUTEX(S32 s32MutexId);
//	inline BOOL MS_SPINAND_OBTAIN_MUTEX (S32 s32MutexId, U32 u32WaitMs);
//	inline BOOL MS_SPINAND_RELEASE_MUTEX (S32 s32MutexId);
void *drvSPINAND_get_DrvContext_address(void);
extern SPINAND_FLASH_INFO_t _gtSpinandInfo;
extern  U8 _u8SPINANDDbgLevel;
#ifdef __cplusplus
}
#endif

#endif
