/*
* mdrv_spinand.c- Sigmastar
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
#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>

// Common Definition
#include <ms_msys.h>
#include "mdrv_spinand_command.h"
#include "mdrv_spinand.h"
#include "mhal_spinand.h"
#include "../include/ms_platform.h"
#include "mdrv_spinand_hal.h"
#include "mtdcore.h"
#define HERE //printk(KERN_ERR"[%s]%d\n",__FILE__,__LINE__)

//==========================================================================
// Global Variable
//==========================================================================

SPINAND_FLASH_INFO_t _gtSpinandInfo;
SPI_NAND_DRIVER_t gtSpiNandDrv;

void *drvSPINAND_get_DrvContext_address(void) // exposed API
{
  return &gtSpiNandDrv;
}

#if defined(SPINAND_MEASURE_PERFORMANCE) && SPINAND_MEASURE_PERFORMANCE
#include <linux/proc_fs.h>
uint64_t	u64_TotalWriteBytes;
uint64_t	u64_TotalReadBytes;
struct proc_dir_entry * writefile;
const char procfs_name[] = "StorageBytes";
#endif


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static struct mstar_spinand_info *info;
static U32 u32_curRow = 0;
//static U32 u32_curCol = 0;
/* These really don't belong here, as they are specific to the NAND Model */
static uint8_t scan_ff_pattern[] = {0xff};

#define DRIVER_NAME "ms-spinand"

#define CACHE_LINE	0x10

#define MS_ASSERT(a)
//serial flash mutex wait time
#define SPINAND_MUTEX_WAIT_TIME    3000
// For linux, thread sync is handled by mtd. So, these functions are empty.
#define MSOS_PROCESS_PRIVATE    0x00000000
#define MSOS_PROCESS_SHARED     0x00000001

#define NAND_USE_FLASH_BBT	0x00010000
#define NAND_BBT_BLOCK_NUM  4

//	static spinlock_t _gtSpiNANDLock;

/* struct nand_bbt_descr - bad block table descriptor */
static struct nand_bbt_descr spi_nand_bbt_descr = {
    .options = NAND_BBT_2BIT | NAND_BBT_LASTBLOCK | NAND_BBT_VERSION | NAND_BBT_CREATE | NAND_BBT_WRITE,
    .offs = 0,
    .len = 1,
    .pattern = scan_ff_pattern
};

#if 0
static struct nand_ecclayout spi_nand_oobinfo = {
    .eccbytes = 32,
    .eccpos = {8, 9, 10, 11, 12, 13, 14, 15,
        24, 25, 26, 27, 28, 29, 30, 31,
        40, 41, 42, 43, 44, 45, 46, 47,
        56, 57, 58, 59, 60, 61, 62, 63},
    .oobavail = 30,
    .oobfree = {
        {2, 6},
        {16, 8},
        {32, 8},
        {48, 8},
        {0, 0}
    },
};
#endif

static uint8_t bbt_pattern[] = {'B', 'b', 't', '0' };
static uint8_t mirror_pattern[] = {'1', 't', 'b', 'B' };

static struct nand_bbt_descr spi_nand_bbt_main_descr = {
    .options		= NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE |
    NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
    .offs			= 1,
    .len			= 3,
    .veroffs		= 4,
    .maxblocks		= NAND_BBT_BLOCK_NUM,
    .pattern		= bbt_pattern
};

static struct nand_bbt_descr spi_nand_bbt_mirror_descr = {
    .options		= NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE |
    NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
    .offs			= 1,
    .len			= 3,
    .veroffs		= 4,
    .maxblocks		= NAND_BBT_BLOCK_NUM,
    .pattern		= mirror_pattern
};



#if defined(CONFIG_MTD_CMDLINE_PARTS) || defined(CONFIG_MTD_CMDLINE_PARTS_MODULE)
#define MTD_PARTITION_MAX		64
static struct mtd_partition partition_info[MTD_PARTITION_MAX];
#endif

void _spiNandMain(unsigned int dwSramAddress, unsigned int dwSramSize)
{
    U8 u8Data[2];
    MDrv_SPINAND_ReadID(2,u8Data);
}

//==========================================================================
// Global Function  for structure  nand_chip and nand_ecc_ctrl
//==========================================================================

#if 0
static __inline void dump_mem_line(unsigned char *buf, int cnt)
{
#if 1
    printk(KERN_NOTICE" 0x%08lx: " \
           "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X \n" \
           , (U32)buf,
           buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],buf[10],buf[11],buf[12],buf[13],buf[14],buf[15]
          );
#else
    int i;

    printk(KERN_NOTICE" 0x%08lx: ", (U32)buf);
    for (i= 0; i < cnt; i++)
        printk(KERN_NOTICE"%02X ", buf[i]);

    printk(KERN_NOTICE" | ");

    for (i = 0; i < cnt; i++)
        printk(KERN_NOTICE"%c", (buf[i] >= 32 && buf[i] < 128) ? buf[i] : '.');

    printk(KERN_NOTICE"\n");
#endif
}

void dump_mem(unsigned char *buf, int cnt)
{
    int i;

    for (i= 0; i < cnt; i+= 16)
        dump_mem_line(buf + i, 16);
}
#endif

#if defined(SPINAND_MEASURE_PERFORMANCE) && SPINAND_MEASURE_PERFORMANCE
int procfile_read(char* buffer, char ** buffer_location, off_t offset,
                  int buffer_length, int *eof, void *data)
{
    int ret;
    if (offset > 0)
        ret = 0;
    else
        ret = sprintf(buffer,"TotalWriteBytes %lld GB %lld MB\nTotalReadBytes %lld GB %lld MB\n",
                      u64_TotalWriteBytes/1024/1024/1024, (u64_TotalWriteBytes/1024/1024) % 1024,
                      u64_TotalReadBytes/1024/1024/1024, (u64_TotalReadBytes/1024/1024) % 1024);
    return ret;
}
#endif

uint8_t	spi_nand_read_byte(struct mtd_info *mtd)
{
    u8 u8_word;
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    spi_nand_debug("");
    u8_word = pSpiNandDrv->pu8_sparebuf[pSpiNandDrv->u32_column];

    if (pSpiNandDrv->u8_statusRequest)
    {
        /*If write protect, the status will be 0x80. Normal will return 0x0. It revert wiht P_NAND. */
        /*See function nand_check_wp in nand_base.c */
        u8_word = ~(*(pSpiNandDrv->pu8_statusbuf));
    }
    pSpiNandDrv->u32_column++;
    return u8_word;
}

u16 spi_nand_read_word(struct mtd_info *mtd)
{
    u16 u16_word;

    spi_nand_debug("");
    u16_word = ((u16)gtSpiNandDrv.pu8_sparebuf[gtSpiNandDrv.u32_column] | ((u16)gtSpiNandDrv.pu8_sparebuf[gtSpiNandDrv.u32_column+1]<<8));
    gtSpiNandDrv.u32_column += 2;

    return u16_word;
}

void spi_nand_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
    spi_nand_debug("not support");
}

void spi_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
    spi_nand_debug("not support");
}

int spi_nand_verify_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
    spi_nand_debug("not support");

    return 0;
}

void spi_nand_select_chip(struct mtd_info *mtd, int chip)
{
    spi_nand_debug("not support");
}

void spi_nand_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
    spi_nand_debug("not support");
}

int spi_nand_dev_ready(struct mtd_info *mtd)
{
    spi_nand_debug("not support");

    return 1;
}

void spi_nand_cmdfunc(struct mtd_info *mtd, unsigned command, int column, int page_addr)
{
    U32 ret = 0;
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    pSpiNandDrv->u8_statusRequest = FALSE;
    switch (command) {

    case NAND_CMD_STATUS:
        spi_nand_debug("NAND_CMD_STATUS");
        pSpiNandDrv->u8_statusRequest = TRUE;
        ret = MDrv_SPINAND_ReadStatusRegister(pSpiNandDrv->pu8_statusbuf, SPI_NAND_REG_PROT);
        if (ret != ERR_SPINAND_SUCCESS)
            spi_nand_err(" ReadStatusRegister != ERR_SPINAND_SUCCESS~! %ld \n", ret);
//	        gtSpiNandDrv.pu8_sparebuf[0] = NAND_STATUS_READY|NAND_STATUS_TRUE_READY;
//	        gtSpiNandDrv.u32_column = 0;
        break;

    case NAND_CMD_READOOB:
        spi_nand_debug("NAND_CMD_READOOB");
        ret = MDrv_SPINAND_Read(page_addr, (U8 *)gtSpiNandDrv.pu8_pagebuf, (U8 *)gtSpiNandDrv.pu8_sparebuf);

        if(ret == ECC_NOT_CORRECTED)
        {
            mtd->ecc_stats.failed++;
            spi_nand_err("MDrv_SPINAND_Read=%lx, P: %d", ret, page_addr);
        }
        else if((ret != ERR_SPINAND_SUCCESS) && (ret != ECC_NOT_CORRECTED))
        {
            mtd->ecc_stats.corrected += 1;
        }

        gtSpiNandDrv.u32_column = column;
        break;

    case NAND_CMD_ERASE2:
        spi_nand_debug("NAND_CMD_ERASE2");
        break;

    case NAND_CMD_ERASE1:
        spi_nand_debug("NAND_CMD_ERASE1");
//	        spi_nand_msg("NAND_CMD_ERASE1, page_addr: 0x%X\n", page_addr);
        gtSpiNandDrv.u8_status = NAND_STATUS_READY|NAND_STATUS_TRUE_READY;
        ret = MDrv_SPINAND_BLOCK_ERASE(page_addr);
        if (ret != ERR_SPINAND_SUCCESS)
        {
            spi_nand_err("MDrv_SPINAND_Erase= %ld \n", ret);
            gtSpiNandDrv.u8_status |= NAND_STATUS_FAIL;
        }
        break;

    case NAND_CMD_READ0:
        spi_nand_debug("NAND_CMD_READ0");
        u32_curRow = page_addr;
//        u32_curCol = column; //set not used
        break;

    default:
        spi_nand_err("unsupported command %02Xh", command);
        break;
    }

}

int spi_nand_waitfunc(struct mtd_info *mtd, struct nand_chip *this)
{
    spi_nand_debug("");

    return (int)gtSpiNandDrv.u8_status;
}

int spi_nand_write_page(struct mtd_info *mtd, struct nand_chip *chip,
                        uint32_t offset, int data_len, const uint8_t *buf,
                        int oob_required, int page, int cached, int raw)
{
    U32 ret;

    spi_nand_debug("0x%X", page);

    ret = MDrv_SPINAND_Write(page, (U8 *)buf, (U8 *)chip->oob_poi);
    if (ret != ERR_SPINAND_SUCCESS)
    {
        spi_nand_err("MDrv_SPINAND_Write=%ld", ret);
        return -EIO;
    }

    return 0;
}

void spi_nand_ecc_hwctl(struct mtd_info *mtd, int mode)
{
    spi_nand_debug("not support");
}

int spi_nand_ecc_calculate(struct mtd_info *mtd, const uint8_t *dat, uint8_t *ecc_code)
{
    spi_nand_debug("not support");

    return 0;
}

int spi_nand_ecc_correct(struct mtd_info *mtd, uint8_t *dat, uint8_t *read_ecc, uint8_t *calc_ecc)
{
    spi_nand_debug("not support");

    return 0;
}

int spi_nand_ecc_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
                               uint8_t *buf, int oob_required, int page)
{
    U32 ret;
    U8 *u8_DmaBuf = buf;

    spi_nand_debug("0x%X", page);

#if defined(CONFIG_MIPS)
    if ( ((U32)buf) >= 0xC0000000 || ((U32)buf) % CACHE_LINE )
#elif defined(CONFIG_ARM)
    if (!virt_addr_valid((U32)buf) || !virt_addr_valid((U32)buf + (U32)mtd->writesize - 1) || ((U32)buf) % CACHE_LINE )
#endif
    {
        spi_nand_debug("Receive Virtual Mem:%08lXh", (U32)buf);
        u8_DmaBuf = gtSpiNandDrv.pu8_pagebuf;
    }

    ret = MDrv_SPINAND_Read(page, (U8 *)u8_DmaBuf, (U8 *)chip->oob_poi);

    if(ret == ECC_NOT_CORRECTED)
    {
//        spi_nand_err("MDrv_SPINAND_Read=%x, P: 0x%x", ret, page);
        mtd->ecc_stats.failed++;
    }
    else if((ret != ERR_SPINAND_SUCCESS) && (ret != ECC_NOT_CORRECTED))
    {
        mtd->ecc_stats.corrected += 1;
    }
    if (u8_DmaBuf != buf)
    {
        memcpy((void *) buf, (const void *) u8_DmaBuf, mtd->writesize);
    }

    return 0;
}
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
int spi_nand_ecc_write_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
                                const uint8_t *buf, int oob_required)
#else
int spi_nand_ecc_write_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
                                const uint8_t *buf, int oob_required, int page)
#endif
{
    spi_nand_debug("not support");
    return 0;
}

int spi_nand_ecc_read_page(struct mtd_info *mtd, struct nand_chip *chip,
                           uint8_t *buf, int oob_required, int page)
{
    U32 ret;
    U8 *u8_DmaBuf = buf;

//	    spi_nand_msg("0x%X", page);

#if defined(CONFIG_MIPS)
    if ( ((U32)buf) >= 0xC0000000 || ((U32)buf) % CACHE_LINE )
#elif defined(CONFIG_ARM)
    if (!virt_addr_valid((U32)buf) || !virt_addr_valid((U32)buf + (U32)mtd->writesize - 1) || ((U32)buf) % CACHE_LINE )
#endif
    {
//	        spi_nand_msg("Receive Virtual Mem:%08lXh", (U32)buf);
        u8_DmaBuf = gtSpiNandDrv.pu8_pagebuf;
    }

    ret = MDrv_SPINAND_Read(page, (U8 *)u8_DmaBuf, (U8 *)chip->oob_poi);

    if(ret == ECC_NOT_CORRECTED)
    {
        spi_nand_err("MDrv_SPINAND_Read=0x%lx, P: %d", ret, page);
        mtd->ecc_stats.failed++;
    }
    else if((ret != ERR_SPINAND_SUCCESS) && (ret != ECC_NOT_CORRECTED))
    {
        mtd->ecc_stats.corrected += 1;
    }

    if (u8_DmaBuf != buf)
    {
        memcpy((void *) buf, (const void *) u8_DmaBuf, mtd->writesize);
    }

    return 0;
}

int spi_nand_ecc_read_subpage(struct mtd_info *mtd, struct nand_chip *chip,
			uint32_t offs, uint32_t len, uint8_t *buf, int page)
{
    U32 ret = 0;
    U8 *u8_DmaBuf;

    buf += offs;
    u8_DmaBuf = buf;

#if defined(CONFIG_MIPS)
    if ( ((U32)buf) >= 0xC0000000 || ((U32)buf) % CACHE_LINE )
#elif defined(CONFIG_ARM)
    if (!virt_addr_valid((U32)buf) || !virt_addr_valid((U32)buf + (U32)mtd->writesize - 1) || ((U32)buf) % CACHE_LINE )
#endif
    {
//	        spi_nand_msg("Receive Virtual Mem:%08lXh", (U32)buf);
        u8_DmaBuf = gtSpiNandDrv.pu8_pagebuf;
    }

    ret = MDrv_SPINAND_Read_RandomIn(u32_curRow, offs, len, u8_DmaBuf);

    if (ret == ERR_SPINAND_SUCCESS)
    {
        //printk("read subpage %ld success\n", u32_curRow);
    }
    else if (ret == ECC_NOT_CORRECTED)
        mtd->ecc_stats.failed++;
    else if ((ret != ERR_SPINAND_SUCCESS) && (ret != ECC_NOT_CORRECTED))
        mtd->ecc_stats.corrected += 1;

    if (u8_DmaBuf != buf)
    {
        memcpy((void *) buf, (const void *) u8_DmaBuf, len);
    }

    return 0;
}
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
int spi_nand_ecc_write_page(struct mtd_info *mtd, struct nand_chip *chip,
				const uint8_t *buf, int oob_required)
#else
int spi_nand_ecc_write_page(struct mtd_info *mtd, struct nand_chip *chip,
				const uint8_t *buf, int oob_required, int page)
#endif
{
    spi_nand_debug("not support");
    return 0;
}

int spi_nand_ecc_read_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
    U32 ret;
//	    spi_nand_msg("0x%X", page);

    ret = MDrv_SPINAND_Read(page, (U8 *)gtSpiNandDrv.pu8_pagebuf, (U8 *)chip->oob_poi);

    if ((ret != ERR_SPINAND_SUCCESS) && (ret == ECC_NOT_CORRECTED))
    {
        spi_nand_err("MDrv_SPINAND_Read=%ld", ret);
        ret = MDrv_SPINAND_Read(page, (U8 *)gtSpiNandDrv.pu8_pagebuf, (U8 *)chip->oob_poi);
        spi_nand_err("MDrv_SPINAND_Read retry=%ld", ret);
    }

    return 0;
}

int spi_nand_ecc_write_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
    U32 ret;

    spi_nand_debug("0x%X", page);

    memset((void *)gtSpiNandDrv.pu8_pagebuf, 0xFF, mtd->writesize);
    ret = MDrv_SPINAND_Write(page, (U8 *)gtSpiNandDrv.pu8_pagebuf, (U8 *)chip->oob_poi);
    if (ret != ERR_SPINAND_SUCCESS)
    {
        spi_nand_err("MDrv_SPINAND_Write=%ld", ret);
        return -EIO;
    }
    memset((void *)gtSpiNandDrv.pu8_pagechkbuf, 0xFF, mtd->writesize);
    MDrv_SPINAND_Read(page, gtSpiNandDrv.pu8_pagechkbuf, (U8 *)chip->oob_poi);

    if(memcmp((void*)(gtSpiNandDrv.pu8_pagechkbuf), (void*)(gtSpiNandDrv.pu8_pagebuf), mtd->writesize))
    {
        spi_nand_err("cmp fail!! retry");

        ret = MDrv_SPINAND_Write(page, (U8 *)gtSpiNandDrv.pu8_pagebuf, (U8 *)chip->oob_poi);
        if (ret != ERR_SPINAND_SUCCESS)
        {
            spi_nand_err("MDrv_SPINAND_Write=%ld", ret);
            return -EIO;
        }
    }
    return 0;
}

static U32 _checkSum(U8 *pu8_Data, U16 u16_ByteCnt)
{
    U32 u32_Sum = 0;

    while (u16_ByteCnt--)
        u32_Sum += *pu8_Data++;

    return u32_Sum;
}

static void _addSstarPartition(struct mtd_info* mtd, SPI_NAND_PARTITION_INFO_t *pPartInfo)
{
    U32 u32_BlkSize;
    SPI_NAND_PARTITION_RECORD_t *pRecord;
    U8 u8_PartNo;
    U16 u16_LastPartType;
    int NumOfPart;
    int nMaxRec = sizeof(pPartInfo->records) / sizeof(pPartInfo->records[0]);
    BOOL bLogic = FALSE;
	U16  nPartTypeNoFlag;

    u32_BlkSize = (U32)gtSpiNandDrv.tSpinandInfo.u16_PageByteCnt * (U32)gtSpiNandDrv.tSpinandInfo.u16_BlkPageCnt;
	pRecord = pPartInfo->records;
    u16_LastPartType = 0xFFFF;
    u8_PartNo = 0;
    NumOfPart = 0;
    while(pRecord - pPartInfo->records < pPartInfo->u16_PartCnt)
    {
        if((pRecord->u16_PartType & UNFD_LOGI_PART) == UNFD_LOGI_PART)
		{
            bLogic = TRUE;
        }
        else
        {
            bLogic = FALSE;
        }
        if((pRecord->u16_PartType & UNFD_HIDDEN_PART) == UNFD_HIDDEN_PART)
		{
            pRecord++;
            continue;
		}
        if (pRecord - pPartInfo->records >= nMaxRec) break;
		nPartTypeNoFlag = pRecord->u16_PartType & 0x0FFF;

        if( u16_LastPartType == pRecord->u16_PartType)
            u8_PartNo++;
        else
            u8_PartNo = 0;

        u16_LastPartType = pRecord->u16_PartType;

        switch(nPartTypeNoFlag)
        {
            case UNFD_PART_IPL_CUST:
                if(u8_PartNo == 0)
                    partition_info[NumOfPart].name = "IPL_CUST0";
                else
                    partition_info[NumOfPart].name = "IPL_CUST1";
                break;
            case UNFD_PART_BOOTLOGO:
                partition_info[NumOfPart].name = "BOOTLOGO";
                break;
            case UNFD_PART_IPL:
                if(u8_PartNo == 0)
                    partition_info[NumOfPart].name = "IPL0";
                else
                    partition_info[NumOfPart].name = "IPL1";
                break;
            case UNFD_PART_OS:
                    partition_info[NumOfPart].name = "OS";
                break;
            case UNFD_PART_CUS:
                    partition_info[NumOfPart].name = "CUS";
                break;
            case UNFD_PART_UBOOT:
                if(u8_PartNo == 0)
                    partition_info[NumOfPart].name = "UBOOT0";
                else
                    partition_info[NumOfPart].name = "UBOOT1";
                break;
            case UNFD_PART_SECINFO:
					partition_info[NumOfPart].name = "SECINFO";
                break;
            case UNFD_PART_OTP:
					partition_info[NumOfPart].name = "OTP";
                break;
            case UNFD_PART_RTOS:
                    partition_info[NumOfPart].name = "RTOS";
                break;
            case UNFD_PART_RTOS_BACKUP:
                    partition_info[NumOfPart].name = "RTOS_BACKUP";
                break;
			case UNFD_PART_KERNEL:
					partition_info[NumOfPart].name = "KERNEL";
				break;
            case UNFD_PART_UBI:
                    partition_info[NumOfPart].name = "UBI";
                break;
			case UNFD_PART_KERNEL_BACKUP:
					partition_info[NumOfPart].name = "KERNEL_BACKUP";
				break;
            case UNFD_PART_RECOVERY:
					partition_info[NumOfPart].name = "RECOVERY";
                break;
            case UNFD_PART_E2PBAK:
					partition_info[NumOfPart].name = "E2PBAK";
                break;
            case UNFD_PART_NVRAMBAK:
					partition_info[NumOfPart].name = "NVRAMBAK";
                break;
            case UNFD_PART_NPT:
					partition_info[NumOfPart].name = "NPT";
                break;
            case UNFD_PART_ENV:
					partition_info[NumOfPart].name = "ENV";
                break;
            case UNFD_PART_MISC:
					partition_info[NumOfPart].name = "MISC";
                break;
            default:
			    if(nPartTypeNoFlag >= UNFD_PART_CUST0 &&
			       nPartTypeNoFlag <= UNFD_PART_CUSTf)
			    {
			        partition_info[NumOfPart].name = "CUST";
			    }
			    else
			    {
					partition_info[NumOfPart].name = "UNKNOWN";
                }
                break;
		}
        partition_info[NumOfPart].offset = (U32)(pRecord->u16_StartBlk * u32_BlkSize);
        partition_info[NumOfPart].size = (U32)((pRecord->u16_BlkCnt+pRecord->u16_BackupBlkCnt)*u32_BlkSize);
        partition_info[NumOfPart].mask_flags = 0;
        printk("%s:%llX, %llX\n", partition_info[NumOfPart].name,
                                    partition_info[NumOfPart].offset,
									partition_info[NumOfPart].size);
        NumOfPart++;
        pRecord++;
    }// while
	pRecord--;
    if(pRecord->u16_PartType == 0xC000)
	{
	    partition_info[NumOfPart].name = "UBI";
	    partition_info[NumOfPart].offset = (U32)(pRecord->u16_StartBlk * u32_BlkSize);
           partition_info[NumOfPart].size = (U32)((gtSpiNandDrv.tSpinandInfo.u16_BlkCnt - pRecord->u16_StartBlk) * u32_BlkSize);
	    partition_info[NumOfPart].mask_flags = 0;
	    printk("%s:%llX, %llX\n", partition_info[NumOfPart].name,
		                            partition_info[NumOfPart].offset,
			                        partition_info[NumOfPart].size);
	    NumOfPart++;
	}
    printk("parse_mtd_partitions from CIS ok\n");
    add_mtd_partitions(mtd, partition_info, NumOfPart);

}

void _enableClock(struct platform_device *pdev)
{
    int num_parents, i;
    struct clk **spinand_clks;
    num_parents = of_clk_get_parent_count(pdev->dev.of_node);
    if(num_parents > 0)
    {
        spinand_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);

		if(spinand_clks == NULL)
        {
            printk( "[SPINAND]kzalloc failed!\n" );
            return;
        }


		//enable all clk
        for(i = 0; i < num_parents; i++)
        {
            spinand_clks[i] = of_clk_get(pdev->dev.of_node, i);
            if (IS_ERR(spinand_clks[i]))
            {
                spi_nand_err( "[SPINAND] Fail to get clk!\n" );
                kfree(spinand_clks);
                return ;
            }
            else
            {
                clk_prepare_enable(spinand_clks[i]);
            }
            clk_put(spinand_clks[i]);
        }
        kfree(spinand_clks);
    }
}

void _disableClock(struct platform_device *pdev)
{
    int num_parents, i;
    struct clk **spinand_clks;

    num_parents = of_clk_get_parent_count(pdev->dev.of_node);
    if(num_parents > 0)
    {
        spinand_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);

		if(spinand_clks == NULL)
        {
            printk( "[SPINAND]kzalloc failed!\n" );
            return;
        }

        //disable all clk
        for(i = 0; i < num_parents; i++)
        {
            spinand_clks[i] = of_clk_get(pdev->dev.of_node, i);
            if (IS_ERR(spinand_clks[i]))
            {
                printk( "[SPINAND] Fail to get clk!\n" );
                kfree(spinand_clks);
                return;
            }
            else
            {
                clk_disable_unprepare(spinand_clks[i]);
            }
            clk_put(spinand_clks[i]);
        }
        kfree(spinand_clks);
    }
}

static void _dumpNandInformation(void)
{
//	    spi_nand_warn("u8_SectorByteCntBits %d", pSpiNandDrv->u8_SectorByteCntBits);
//	    spi_nand_warn("u8_PageSectorCntBits %d", pSpiNandDrv->u8_PageSectorCntBits);
//	    spi_nand_warn("u8_PageByteCntBits %d", pSpiNandDrv->u8_PageByteCntBits);
//	    spi_nand_warn("u8_BlkPageCntBits %d", pSpiNandDrv->u8_BlkPageCntBits);
    spi_nand_warn("Bytes / Page :  %d", gtSpiNandDrv.tSpinandInfo.u16_PageByteCnt);
    spi_nand_warn("Pages / Block:  %d", gtSpiNandDrv.tSpinandInfo.u16_BlkPageCnt);
    spi_nand_warn("Sector/ Page :  %d", gtSpiNandDrv.tSpinandInfo.u16_SectorByteCnt);
    spi_nand_warn("Spare / Page :  %d", gtSpiNandDrv.tSpinandInfo.u16_SpareByteCnt);
    {
        int read = 1, write = 1, drv = 1;
#if defined(CONFIG_NAND_QUAL_READ)
        read = 4;
#endif
#if defined(CONFIG_NAND_QUAL_WRITE)
        write = 4;
#endif
#if defined(SUPPORT_SPINAND_QUAD) && (SUPPORT_SPINAND_QUAD)
        drv = 4;
#endif
        spi_nand_warn("Current config r:%d w:%d drv:%d", read, write, drv);
    }
}

int _ms_mtd_param_init(struct mtd_info *mtd,
                          struct nand_chip *chip,
                          int *maf_id, int *dev_id,
                          const struct nand_flash_dev *type)
{
    SPI_NAND_DRIVER_t *pNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();

    if (!mtd->name)
        mtd->name = "nand0";
    HERE;
    mtd->writesize = pNandDrv->tSpinandInfo.u16_PageByteCnt;
    mtd->oobsize = pNandDrv->tSpinandInfo.u16_SpareByteCnt;
    mtd->erasesize = pNandDrv->tSpinandInfo.u16_BlkPageCnt * pNandDrv->tSpinandInfo.u16_PageByteCnt;
    chip->chipsize = (uint64_t)pNandDrv->tSpinandInfo.u16_BlkCnt * (uint64_t)pNandDrv->tSpinandInfo.u16_BlkPageCnt * (uint64_t)pNandDrv->tSpinandInfo.u16_PageByteCnt;

    if(!mtd->writesize || !mtd->oobsize || !mtd->erasesize)
    {
        int i;
        printk("Unsupported NAND Flash type is detected with ID");
        for(i = 0; i < pNandDrv->tSpinandInfo.u8_IDByteCnt; i++)
            printk(" 0x%X", pNandDrv->tSpinandInfo.au8_ID[i]);
        printk("\n");
        return (-EINVAL);
    }
    chip->onfi_version = 0;

    /* Get chip options */
    chip->options = NAND_NO_SUBPAGE_WRITE;

    /*
	 ** Set chip as a default. Board drivers can override it, if necessary
    **/

	if(0)
        chip->options |= NAND_BUSWIDTH_16;

    /* Calculate the address shift from the page size */
    chip->page_shift = ffs(mtd->writesize) - 1;

    /* Convert chipsize to number of pages per chip -1. */
    chip->pagemask = (chip->chipsize >> chip->page_shift) - 1;
    chip->bbt_erase_shift = chip->phys_erase_shift = ffs(mtd->erasesize) - 1;

    if (chip->chipsize & 0xffffffff)
        chip->chip_shift = ffs((unsigned)chip->chipsize) - 1;
    else
        chip->chip_shift = ffs((unsigned)(chip->chipsize >> 32)) + 31;

    /* Set the bad block position */
    chip->badblockpos = mtd->writesize > 512 ? NAND_LARGE_BADBLOCK_POS : NAND_SMALL_BADBLOCK_POS;
    chip->options |= NAND_SKIP_BBTSCAN;
    chip->badblockbits = 8;
    chip->bits_per_cell = 1;

    return 0;

}

//	#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,1)
//	static int __init mstar_spinand_probe(struct platform_device *pdev)
//	#else
//	static int __devinit mstar_spinand_probe(struct platform_device *pdev)
//	#endif

static int mstar_spinand_probe(struct platform_device *pdev)
{
    U8 u8_i;
    U32 u32_ret;
    SPI_NAND_PARTITION_INFO_t *ptPartInfo;
    struct nand_chip *nand;
    struct mtd_info* mtd;
    int err = 0;

#if defined(CONFIG_MTD_CMDLINE_PARTS) || defined(CONFIG_MTD_CMDLINE_PARTS_MODULE)
        static const char *part_probes[] = { "cmdlinepart", NULL, };
#endif
#if defined(CONFIG_ARCH_INFINITY2)
    if(Chip_Get_Storage_Type()!= MS_STORAGE_SPINAND_ECC)
        return 0;
#endif
    _enableClock(pdev);
    spi_nand_msg("mstar_spinand enableClock");

    /* Allocate memory for MTD device structure and private data */
    info = kzalloc(sizeof(struct mstar_spinand_info), GFP_KERNEL);

    if (!info)
    {
        spi_nand_err("Allocate Sstar spi nand info fail\n");
        return -ENOMEM;
    }
    platform_set_drvdata(pdev, info);

    /* Get pointer to private data */
    info->pdev = pdev;
    nand = &info->nand;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
    mtd = &info->mtd;
    /* Get pointer to private data */

    /* Initialize structures */
    mtd->priv = nand;
#else
    mtd = &nand->mtd;
    /* Initialize structures */
    mtd->priv = nand;
#endif
    //mtd->name = dev_name(&pdev->dev);
    mtd->name = "nand0";
    mtd->owner = THIS_MODULE;

    MDrv_SPINAND_Device(&pdev->dev);

    if (MDrv_SPINAND_Init(&(gtSpiNandDrv.tSpinandInfo)) != TRUE)
    {
        spi_nand_err("MDrv_SPINAND_Init fail");
        return -ENODEV;
    }
    _dumpNandInformation();
    gtSpiNandDrv.u8_status = NAND_STATUS_READY|NAND_STATUS_TRUE_READY;
    gtSpiNandDrv.u32_column = 0;
    gtSpiNandDrv.pu8_pagebuf = kmalloc(gtSpiNandDrv.tSpinandInfo.u16_PageByteCnt, GFP_KERNEL);
    gtSpiNandDrv.pu8_sparebuf = kmalloc(gtSpiNandDrv.tSpinandInfo.u16_SpareByteCnt, GFP_KERNEL);
    gtSpiNandDrv.pu8_statusbuf = kmalloc(16, GFP_KERNEL);

    if (!gtSpiNandDrv.pu8_pagebuf || !gtSpiNandDrv.pu8_sparebuf)
    {
        spi_nand_err("Can not alloc memory for page/spare buffer");
        return -ENOMEM;
    }

    ptPartInfo = (SPI_NAND_PARTITION_INFO_t *)gtSpiNandDrv.pu8_pagebuf;
    for (u8_i=0 ; u8_i<2 ; u8_i+=2)
    {
        u32_ret = MDrv_SPINAND_Read(u8_i*gtSpiNandDrv.tSpinandInfo.u16_BlkPageCnt, gtSpiNandDrv.pu8_pagebuf, gtSpiNandDrv.pu8_sparebuf);
        if (u32_ret == ERR_SPINAND_SUCCESS)
        {
            if (memcmp((const void *) gtSpiNandDrv.pu8_pagebuf, SPINAND_FLASH_INFO_TAG, 16) == 0)
            {
                spi_nand_msg("Magic memcmp pass");
                spi_nand_msg("Get partition (Block 0 : page 1)");
                u32_ret = MDrv_SPINAND_Read(u8_i*gtSpiNandDrv.tSpinandInfo.u16_BlkPageCnt + 1, gtSpiNandDrv.pu8_pagebuf, gtSpiNandDrv.pu8_sparebuf);
                if (u32_ret == ERR_SPINAND_SUCCESS)
                {
                    U32 checkresult = 0;
                    checkresult = _checkSum((u8*)&(ptPartInfo->u16_SpareByteCnt), 0x200 - 0x04);
                    if (ptPartInfo->u32_ChkSum == checkresult)
                        break;
                }
            }
        }
    }

    if (u8_i == 10)
    {
        spi_nand_err("CIS doesn't contain part info");
        gtSpiNandDrv.u8_HasPNI = 0;
    }
    else
    {
        spi_nand_msg("CIS contains part info");
        gtSpiNandDrv.u8_HasPNI = 1;
        memcpy((void *)&gtSpiNandDrv.tPartInfo, (const void *) ptPartInfo, 0x200);
    }

    /* please refer to include/linux/nand.h for more info. */
    nand->read_byte = spi_nand_read_byte;
    nand->read_word = spi_nand_read_word;
    nand->write_buf = spi_nand_write_buf;
    nand->read_buf = spi_nand_read_buf;
    nand->select_chip = spi_nand_select_chip;
    nand->cmd_ctrl = spi_nand_cmd_ctrl;
    nand->dev_ready = spi_nand_dev_ready;
    nand->cmdfunc = spi_nand_cmdfunc;
    nand->waitfunc = spi_nand_waitfunc;
    nand->write_page = spi_nand_write_page;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,1)
    nand->bbt_options = NAND_BBT_USE_FLASH;
#else
    nand->options = NAND_USE_FLASH_BBT;
#endif
    nand->chip_delay = 0;
    nand->badblock_pattern = &spi_nand_bbt_descr; //using default badblock pattern.
    nand->bbt_td = &spi_nand_bbt_main_descr;
    nand->bbt_md = &spi_nand_bbt_mirror_descr;
    nand->mtd_param_init = _ms_mtd_param_init;
    nand->ecc.mode = NAND_ECC_HW;
    nand->ecc.size = gtSpiNandDrv.tSpinandInfo.u16_PageByteCnt;
    nand->ecc.bytes = (gtSpiNandDrv.tSpinandInfo.u16_SpareByteCnt>>1);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,1)
    nand->ecc.strength                  = 4;
#endif
#if 0
    nand->ecc.layout =  &spi_nand_oobinfo;
#endif
    nand->ecc.hwctl = spi_nand_ecc_hwctl;
    nand->ecc.calculate = spi_nand_ecc_calculate;
    nand->ecc.correct = spi_nand_ecc_correct;
    nand->ecc.read_page_raw = spi_nand_ecc_read_page_raw;
    nand->ecc.write_page_raw = spi_nand_ecc_write_page_raw;
    nand->ecc.read_page = spi_nand_ecc_read_page;
    nand->ecc.read_subpage = spi_nand_ecc_read_subpage;
    nand->ecc.write_page = spi_nand_ecc_write_page;
    nand->ecc.read_oob = spi_nand_ecc_read_oob;
    nand->ecc.write_oob = spi_nand_ecc_write_oob;
//	    nand->options |= NAND_IS_SPI;
    nand->options |= NAND_CACHEPRG;
    nand->bits_per_cell = 1;
    pr_info("%s: Before nand_scan()...\n",__FUNCTION__);

    if ((err = nand_scan(mtd, 1)) != 0)
    {
        spi_nand_err("can't register SPI NAND\n");
        kfree(mtd);
        return -ENOMEM;
    }

#if defined(CONFIG_MTD_CMDLINE_PARTS) || defined(CONFIG_MTD_CMDLINE_PARTS_MODULE)
//	#ifdef CONFIG_MTD_PARTITIONS
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0)
        struct mtd_partitions pparts;
        err = parse_mtd_partitions(mtd, part_probes, &pparts, 0);

        if((!err) && (pparts.nr_parts > 0))
        {
            spi_nand_msg( "Mtd parts parse");
            add_mtd_partitions(mtd, pparts.parts, pparts.nr_parts);
        }
        else if (IS_ERR_VALUE(err))
        {  
            spi_nand_msg("parse_mtd_partitions error!!! %d\r\n", err);
        }
#else
        int mtd_parts_nb = 0;
        mtd_parts_nb = parse_mtd_partitions(mtd, part_probes, &info->parts, 0);
        if (mtd_parts_nb > 0)
        {
            spi_nand_msg( "Mtd parts default");
            add_mtd_partitions(mtd, info->parts, mtd_parts_nb);
        }
#endif
        else
        {
            spi_nand_msg( "Add Sstar Partition");
            _addSstarPartition(mtd,ptPartInfo);
        }
     }
#else
    add_mtd_partitions(mtd, NULL, 0);
#endif

    platform_set_drvdata(pdev, &info->mtd);

#if defined(SPINAND_MEASURE_PERFORMANCE) && SPINAND_MEASURE_PERFORMANCE
    u64_TotalWriteBytes = u64_TotalReadBytes = 0;
    writefile = proc_create (procfs_name, 0644, NULL);

    if (writefile == NULL)
        printk(KERN_CRIT"Error: Can not initialize /proc/%s\n", procfs_name);
    else
    {
        writefile->read_proc = procfile_read;
        writefile->mode      = S_IFREG | S_IRUGO;
        writefile->uid       = 0;
        writefile->gid      = 0;
        writefile->size      = 0x10;
    }
#endif
    return 0;
}

static int mstar_spinand_remove(struct platform_device *pdev)
{
    platform_set_drvdata(pdev, NULL);

    /* Release NAND device, its internal structures and partitions */
    nand_release(&info->mtd);
    kfree(info);
    kfree(gtSpiNandDrv.pu8_pagebuf);
    kfree(gtSpiNandDrv.pu8_sparebuf);
    kfree(gtSpiNandDrv.pu8_statusbuf);
    _disableClock(pdev);
    return 0;
}

static const struct of_device_id spinand_of_dt_ids[] =
{
	{ .compatible = "ms-spinand" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, spinand_of_dt_ids);

static struct platform_driver mstar_spinand_driver = {
    .probe 		= mstar_spinand_probe,
    .remove		= mstar_spinand_remove,
    .driver = {
        .name 	= DRIVER_NAME,
        .owner	= THIS_MODULE,
        .of_match_table = (spinand_of_dt_ids),
    },
};
module_platform_driver(mstar_spinand_driver);

#if (0)
static int __init mstar_spinand_init(void)
{
    int err = 0;
    printk("mstar_spinand_init\n");
    if (MDrv_SPINAND_IsActive() == 0)
    {
        pr_info("%s device not found\n", DRIVER_NAME);
        return -ENODEV;
    }

    pr_info("%s driver initializing\n", DRIVER_NAME);

//	    err = platform_device_register(&mstar_spinand_deivce_st);
    if (err < 0)
        spi_nand_err("SPI NAND Err: platform device register fail %d\n", err);

    return platform_driver_register(&mstar_spinand_driver);
}

static void __exit mstar_spinand_exit(void)
{
    platform_driver_unregister(&mstar_spinand_driver);
}

module_init(mstar_spinand_init);
module_exit(mstar_spinand_exit);
#endif
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("Sstar MTD SPI NAND driver");
