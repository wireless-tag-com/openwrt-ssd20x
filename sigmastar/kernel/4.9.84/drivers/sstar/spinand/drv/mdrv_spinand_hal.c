/*
* mdrv_spinand_hal.c- Sigmastar
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
//==========================================================================
// Common Definition
//==========================================================================

#include <ms_msys.h>
#include "mdrv_spinand_command.h"
#include "mdrv_spinand.h"
#include "mhal_spinand.h"
#include "../include/ms_platform.h"

//==========================================================================
// Local Variables
//==========================================================================
U8 u8MagicData[] = {0x4D, 0x53, 0x54, 0x41, 0x52, 0x53, 0x45, 0x4D, 0x49, 0x55, 0x53, 0x46, 0x44, 0x43, 0x49, 0x53};
struct device *spi_nand_dev = NULL;
static int _s32SPINAND_Mutex;
static spinlock_t _gtSpiNANDLock;
#if defined(SUPPORT_SPINAND_QUAD) && SUPPORT_SPINAND_QUAD
SPINAND_MODE _SpinandRdMode = E_SPINAND_QUAD_MODE;
#else
SPINAND_MODE _SpinandRdMode = E_SPINAND_SINGLE_MODE;
#endif

/********Read nand info by sni & pni, so no need the table.********/
#if USE_SPINAND_INFO_TABLE
    SPINAND_FLASH_INFO_t gtSpiNandInfoTable[]=
    {    //u8_IDByteCnt           au8_ID                u16_SpareByteCnt   u16_PageByteCnt   u16_BlkPageCnt   u16_BlkCnt  u16_SectorByteCnt u8PlaneCnt  u8WrapConfig bRIURead u8CLKConfig u8_UBOOTPBA u8_BL0PBA u8_BL1PBA u8_HashPBA  eReadMode                            eWriteMode
        {2, {MID_GD     , 0xF4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 4096, 512, 0},
        {2, {MID_GD     , 0xF1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 0},
        {2, {MID_GD     , 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 0},
        {2, {MID_GD     , 0xD1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 0},
        {2, {MID_GD     , 0xD5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 0},
        {2, {MID_GD     , 0xD2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 2048, 512, 0},
        {2, {MID_MICRON , 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 2},
        {2, {MID_MICRON , 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 2048, 512, 2},
        {2, {MID_MICRON , 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 4096, 512, 2},
        {2, {MID_ATO    , 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 0},
        {2, {MID_WINBOND, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 0},
        {2, {MID_MXIC   , 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 1024, 512, 0},
        {2, {MID_MXIC   , 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 2048, 512, 2},
        {2, {MID_TOSHIBA, 0xCB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 64, 2048, 64, 2048, 512, 2},
        {2, {0x00       , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  0,    0,  0,    0,   0, 0},
    };
#endif

// For linux, thread sync is handled by mtd. So, these functions are empty.
#define MSOS_PROCESS_PRIVATE    0x00000000
#define MSOS_PROCESS_SHARED     0x00000001
#define MS_ASSERT(a)

//serial flash mutex wait time
#define SPINAND_MUTEX_WAIT_TIME    3000

//==========================================================================
// Local Functions:  SPI NAND Driver Function
//==========================================================================
static BOOL _MDrv_SPINAND_GET_INFO(void)
{
    U32 u32Ret;
    U8 u8Spare[16];
    U8 u8Data[512];
    U8 u8Idx, u8Status;
    U8 *u8SrcAddr, *u8DstAddr;
    U16 u16PageIndex;
    u8SrcAddr = u8Data;
    u8DstAddr = (U8*)(&_gtSpinandInfo);

    // if ecc error read back up block(block2 ,4, 6, 8)
    for (u8Idx = 0; u8Idx < 10; u8Idx+=2)
    {
        u16PageIndex = 64 * u8Idx;

        //read data to cache first
        u32Ret = HAL_SPINAND_RFC(u16PageIndex, &u8Status);
       if (u32Ret != ERR_SPINAND_SUCCESS || (u8Status == ERR_SPINAND_ECC_ERROR))
            continue;

        // Read SPINand Data
        u32Ret = HAL_SPINAND_RIU_READ(0, 512, u8SrcAddr);
        if (u32Ret != ERR_SPINAND_SUCCESS)
            continue;

        // Read SPINand Spare Data
        u32Ret = HAL_SPINAND_RIU_READ(2048, 16, u8Spare);
        if (u32Ret != ERR_SPINAND_SUCCESS)
            continue;

        if (memcmp((const void *) u8SrcAddr, (const void *) u8MagicData, sizeof(u8MagicData)) != 0)
            continue;

        u8SrcAddr += sizeof(u8MagicData);

        memcpy(u8DstAddr, u8SrcAddr, sizeof(SPINAND_FLASH_INFO_t));
        return TRUE;
    }

    return FALSE;
}

inline BOOL MS_SPINAND_IN_INTERRUPT (void)
{
    return FALSE;
}

inline  U32 MS_SPINAND_CREATE_MUTEX ( MsOSAttribute eAttribute, char *pMutexName, U32 u32Flag)
{
    spin_lock_init(&_gtSpiNANDLock);
    return 1;
}

inline BOOL MS_SPINAND_DELETE_MUTEX(S32 s32MutexId)
{
    return TRUE;
}

inline  BOOL MS_SPINAND_OBTAIN_MUTEX (S32 s32MutexId, U32 u32WaitMs)
{
    spin_lock_irq(&_gtSpiNANDLock);
    return TRUE;
}

inline BOOL MS_SPINAND_RELEASE_MUTEX (S32 s32MutexId)
{
    spin_unlock_irq(&_gtSpiNANDLock);
    return TRUE;
}

BOOL MDrv_SPINAND_Init(SPINAND_FLASH_INFO_t *tSpinandInfo)
{
    #define SPINAND_ID_SIZE 2
    U32 u32Ret;

    // 1. HAL init
    _u8SPINANDDbgLevel = E_SPINAND_DBGLV_DEBUG;

    u32Ret = HAL_SPINAND_Init();
    if (u32Ret != ERR_SPINAND_SUCCESS)
    {
        spi_nand_err("Init SPI NAND fail!!!!");
        tSpinandInfo->au8_ID[0] = 0xFF;
        tSpinandInfo->au8_ID[1] = 0xFF;
        tSpinandInfo->u8_IDByteCnt = 2;
        return FALSE;
    }

    _s32SPINAND_Mutex= MS_SPINAND_CREATE_MUTEX(E_MSOS_FIFO, "Mutex SPINAND", MSOS_PROCESS_SHARED);
    MS_ASSERT(_s32SPINAND_Mutex >= 0);

    if (!_MDrv_SPINAND_GET_INFO())
    {
        spi_nand_err("Can't find the CIS in BLOCK0!!!!");
        return FALSE;

        /***************************USE_SPINAND_INFO_TABLE***************************/
#if USE_SPINAND_INFO_TABLE
        u32Ret = HAL_SPINAND_ReadID(SPINAND_ID_SIZE, u8ID);
        if (u32Ret != ERR_SPINAND_SUCCESS)
        {
            spi_nand_err("Can't not Detect SPINAND Device!!!!");
            tSpinandInfo->au8_ID[0] = 0xFF;
            tSpinandInfo->au8_ID[1] = 0xFF;
            tSpinandInfo->u8_IDByteCnt = 2;
            return FALSE;
        }
        spi_nand_msg("MID =%x, DID =%x \r\n",u8ID[0], u8ID[1]);

        for (u32Index = 0; gtSpiNandInfoTable[u32Index].au8_ID[0] != 0; u32Index++)
        {
            if (gtSpiNandInfoTable[u32Index].au8_ID[0] == u8ID[0] &&
                    gtSpiNandInfoTable[u32Index].au8_ID[1] == u8ID[1])
            {
                spi_nand_msg("SPINAND Device DETECT");
                memcpy(tSpinandInfo, &gtSpiNandInfoTable[u32Index], sizeof(SPINAND_FLASH_INFO_t));
                memcpy(&_gtSpinandInfo, &gtSpiNandInfoTable[u32Index], sizeof(SPINAND_FLASH_INFO_t));
                break;
            }
        }
        if ((!gtSpiNandInfoTable[u32Index].au8_ID[0]) && (!gtSpiNandInfoTable[u32Index].au8_ID[1]))
        {
            spi_nand_err("Can't not Detect SPINAND Device!!!!");
            tSpinandInfo->au8_ID[0] = u8ID[0];
            tSpinandInfo->au8_ID[1] = u8ID[1];
            tSpinandInfo->u8_IDByteCnt = 2;
            if (!bReFind)
            {
                bReFind = TRUE;
                HAL_SPINAND_CSCONFIG();
                goto MDrv_SPINAND_Init_Detect_ID;
            }
            return FALSE;
        }
#endif
        /***************************USE_SPINAND_INFO_TABLE***************************/
    }
    else
    {
        spi_nand_msg("Detected ID: MID =%x, DID =%x",_gtSpinandInfo.au8_ID[0] ,_gtSpinandInfo.au8_ID[1]);
        memcpy(tSpinandInfo, &_gtSpinandInfo, sizeof(SPINAND_FLASH_INFO_t));
    }
#if defined(SUPPORT_SPINAND_QUAD) && SUPPORT_SPINAND_QUAD
    printk("\r\nQuad mode enabled\r\n");
    {
        //HAL_SPINAND_PreHandle(E_SPINAND_QUAD_MODE_IO);
        HAL_SPINAND_PreHandle(_SpinandRdMode);
    }
#endif
    return TRUE;
}

BOOL MDrv_SPINAND_ForceInit(SPINAND_FLASH_INFO_t *tSpinandInfo)
{
    memcpy(&_gtSpinandInfo, tSpinandInfo, sizeof(SPINAND_FLASH_INFO_t));
    return TRUE;
}

static U8 MDrv_SPINAND_CountBits(U32 u32_x)
{
    U8 u8_i = 0;

    while (u32_x)
    {
        u8_i++;
        u32_x >>= 1;
    }

    return u8_i-1;
}

//-------------------------------------------------------------------------------------------------
// Read SPINAND Data
// @param u32_PageIdx : page index of read data in specific block
// @return TRUE : succeed
// @return FALSE : fail
// @note : If Enable ISP engine, the XIU mode does not work
//-------------------------------------------------------------------------------------------------
U32 MDrv_SPINAND_Read(U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf)
{
    U8 u8Status;
    U32 u32Ret = ERR_SPINAND_SUCCESS;
    U16 u16ColumnAddr = 0;

    MS_ASSERT( MS_SPINAND_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SPINAND_OBTAIN_MUTEX(_s32SPINAND_Mutex, SPINAND_MUTEX_WAIT_TIME))
    {
        spi_nand_err("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    if ((ID1 == 0xEF) && (ID2 == 0xAB) && (ID3 == 0x21))
    {
        U16 u16DiePageCnt = MDrv_SPINAND_CountBits(BLOCKCNT * BLOCK_PAGE_SIZE);
        HAL_SPINAND_DieSelect((U8)(u32_PageIdx >> (u16DiePageCnt - 1)));
    }

    //read data to cache first
    u32Ret = HAL_SPINAND_RFC(u32_PageIdx, &u8Status);

    if (u32Ret != ERR_SPINAND_SUCCESS)
    {
        MS_SPINAND_RELEASE_MUTEX(_s32SPINAND_Mutex);
        return u32Ret;
    }

    if (PLANE && (((u32_PageIdx / BLOCK_PAGE_SIZE)&0x1) == 1)) //odd numbered blocks
    {
        u16ColumnAddr |= (1<<12); // plane select for MICRON & 2GB
    }
//    HAL_SPINAND_PLANE_HANDLER(u32_PageIdx);

#if (defined(SUPPORT_SPINAND_QUAD) && SUPPORT_SPINAND_QUAD) || defined(CONFIG_NAND_QUAL_READ)
    HAL_SPINAND_SetMode(E_SPINAND_QUAD_MODE);
    // Read SPINand Data
    u32Ret = HAL_SPINAND_Read (u16ColumnAddr, PAGE_SIZE, u8Data);
    HAL_SPINAND_SetMode(E_SPINAND_SINGLE_MODE);
#else
    // Read SPINand Data
    u32Ret = HAL_SPINAND_Read (u16ColumnAddr, PAGE_SIZE, u8Data);
#endif
    if (u32Ret != ERR_SPINAND_SUCCESS)
    {
        MS_SPINAND_RELEASE_MUTEX(_s32SPINAND_Mutex);
        return u32Ret;
    }

    // Read SPINand Spare Data
    u32Ret= HAL_SPINAND_Read(u16ColumnAddr|PAGE_SIZE, SPARE_SIZE, pu8_SpareBuf);

    if (u32Ret == ERR_SPINAND_SUCCESS) {
       if(u8Status == ECC_1_3_CORRECTED)
            u32Ret = ERR_SPINAND_ECC_1_3_CORRECTED;
        if(u8Status == ECC_4_6_CORRECTED)
           u32Ret = ERR_SPINAND_ECC_4_6_CORRECTED;
        if(u8Status == ECC_7_8_CORRECTED)
            u32Ret = ERR_SPINAND_ECC_7_8_CORRECTED;
        if(u8Status == ECC_NOT_CORRECTED){
            u32Ret = ERR_SPINAND_ECC_ERROR;
            printk("ecc error P: %lx\r\n", u32_PageIdx);
        }
    }

#if defined(SPINAND_MEASURE_PERFORMANCE) && SPINAND_MEASURE_PERFORMANCE
    u64_TotalReadBytes+= _gtSpinandInfo.u16_PageByteCnt;
#endif

    MS_SPINAND_RELEASE_MUTEX(_s32SPINAND_Mutex);

    return u32Ret;
}

//-------------------------------------------------------------------------------------------------
// Read SPINAND Data From Random column address
// @param u32_PageIdx : page index of read data in specific block
// @return TRUE : succeed
// @return FALSE : fail
// @note : If Enable ISP engine, the XIU mode does not work
//-------------------------------------------------------------------------------------------------
U32 MDrv_SPINAND_Read_RandomIn(U32 u32_PageIdx, U32 u32_Column, U32 u32_Byte, U8 *u8Data)
{
    U8 u8Status;
    U32 u32Ret = ERR_SPINAND_SUCCESS;

    MS_ASSERT( MS_SPINAND_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SPINAND_OBTAIN_MUTEX(_s32SPINAND_Mutex, SPINAND_MUTEX_WAIT_TIME))
    {
        spi_nand_err("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    //read data to cache first
    u32Ret = HAL_SPINAND_RFC(u32_PageIdx, &u8Status);
    if (u32Ret != ERR_SPINAND_SUCCESS)
    {
        MS_SPINAND_RELEASE_MUTEX(_s32SPINAND_Mutex);
        return u32Ret;
    }

    HAL_SPINAND_PLANE_HANDLER(u32_PageIdx);

    // Read SPINand Data
    u32Ret = HAL_SPINAND_Read (u32_Column, u32_Byte, u8Data);
    if (u32Ret != ERR_SPINAND_SUCCESS)
    {
        MS_SPINAND_RELEASE_MUTEX(_s32SPINAND_Mutex);
        return u32Ret;
    }

    if (u32Ret == ERR_SPINAND_SUCCESS) {
       if(u8Status == ECC_1_3_CORRECTED)
            u32Ret = ERR_SPINAND_ECC_1_3_CORRECTED;
        if(u8Status == ECC_4_6_CORRECTED)
           u32Ret = ERR_SPINAND_ECC_4_6_CORRECTED;
        if(u8Status == ECC_7_8_CORRECTED)
            u32Ret = ERR_SPINAND_ECC_7_8_CORRECTED;
        if(u8Status == ECC_NOT_CORRECTED){
            u32Ret = ERR_SPINAND_ECC_ERROR;
            printk("ecc error P: %lx\r\n", u32_PageIdx);
        }
    }

#if defined(SPINAND_MEASURE_PERFORMANCE) && SPINAND_MEASURE_PERFORMANCE
    u64_TotalReadBytes += u32_Byte;
#endif

    MS_SPINAND_RELEASE_MUTEX(_s32SPINAND_Mutex);
    return u32Ret;
}

U32 MDrv_SPINAND_SetMode(SPINAND_MODE eMode)
{
    U32 u32Ret;
    MS_ASSERT( MS_SPINAND_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SPINAND_OBTAIN_MUTEX(_s32SPINAND_Mutex, SPINAND_MUTEX_WAIT_TIME))
    {
        spi_nand_err("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }
    u32Ret = HAL_SPINAND_SetMode(eMode);

    MS_SPINAND_RELEASE_MUTEX(_s32SPINAND_Mutex);
    return u32Ret;
}

U32 MDrv_SPINAND_Write(U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf)
{
    U32 u32Ret;

    MS_ASSERT( MS_SPINAND_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SPINAND_OBTAIN_MUTEX(_s32SPINAND_Mutex, SPINAND_MUTEX_WAIT_TIME))
    {
        spi_nand_err("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    if ((ID1 == 0xEF) && (ID2 == 0xAB) && (ID3 == 0x21))
    {
        U16 u16DiePageCnt = MDrv_SPINAND_CountBits(BLOCKCNT * BLOCK_PAGE_SIZE);
        HAL_SPINAND_DieSelect((U8)(u32_PageIdx >> (u16DiePageCnt - 1)));
    }
#if defined(SPINAND_MEASURE_PERFORMANCE) && SPINAND_MEASURE_PERFORMANCE
    u64_TotalWriteBytes += _gtSpinandInfo.u16_PageByteCnt;
#endif
    u32Ret=HAL_SPINAND_Write(u32_PageIdx, u8Data, pu8_SpareBuf);

    MS_SPINAND_RELEASE_MUTEX(_s32SPINAND_Mutex);

    return u32Ret;
}

U8 MDrv_SPINAND_ReadID(U16 u16Size, U8 *u8Data)
{
    U8 u8Ret;

    MS_ASSERT( MS_SPINAND_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SPINAND_OBTAIN_MUTEX(_s32SPINAND_Mutex, SPINAND_MUTEX_WAIT_TIME))
    {
        spi_nand_err("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    u8Ret=HAL_SPINAND_ReadID(u16Size, u8Data);

    MS_SPINAND_RELEASE_MUTEX(_s32SPINAND_Mutex);
    return u8Ret;
}

U32 MDrv_SPINAND_BLOCK_ERASE(U32 u32_PageIdx)
{
    U32 u32Ret;
    MS_ASSERT( MS_SPINAND_IN_INTERRUPT() == FALSE );
    if (FALSE == MS_SPINAND_OBTAIN_MUTEX(_s32SPINAND_Mutex, SPINAND_MUTEX_WAIT_TIME))
    {
        spi_nand_err("%s ENTRY fails!\n", __FUNCTION__);
        return FALSE;
    }

    if ((ID1 == 0xEF) && (ID2 == 0xAB) && (ID3 == 0x21))
    {
        U16 u16DiePageCnt = MDrv_SPINAND_CountBits(BLOCKCNT * BLOCK_PAGE_SIZE);
        HAL_SPINAND_DieSelect((U8)(u32_PageIdx >> (u16DiePageCnt - 1)));
    }

    u32Ret=HAL_SPINAND_BLOCKERASE(u32_PageIdx);

    MS_SPINAND_RELEASE_MUTEX(_s32SPINAND_Mutex);
    return u32Ret;
}

void MDrv_SPINAND_Device(struct device *dev)
{
    spi_nand_dev = dev;
}

U32 MDrv_SPINAND_WriteProtect(BOOL bEnable)
{
    return HAL_SPINAND_WriteProtect(bEnable);
}

BOOL MDrv_SPINAND_IsActive(void)
{
    return HAL_SPINAND_IsActive();
}

U32 MDrv_SPINAND_ReadStatusRegister(MS_U8 *u8Status, MS_U8 u8Addr)
{
    return HAL_SPINAND_ReadStatusRegister(u8Status, u8Addr);
}
