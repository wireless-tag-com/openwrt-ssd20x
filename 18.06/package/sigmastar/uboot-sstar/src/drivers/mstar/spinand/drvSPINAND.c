/*
* drvSPINAND.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: edie.chen <edie.chen@sigmastar.com.tw>
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

#include <common.h>
#include <malloc.h>
#include <linux/err.h>
//#include <linux/compat.h>
#include <asm/io.h>
#include <asm/errno.h>


#include <linux/string.h>
// Common Definition
#include "MsCommon.h"
#include "MsVersion.h"
#include "MsTypes.h"
//#include "MsOS.h"
#include "spinand.h"
// Internal Definition
#include "../../inc/common/drvSPINAND.h"

//	#ifndef _DRV_SPICMD_H_
//	#include "../../inc/common/drvSPICMD.h"
//	#endif


//extern SPINAND_FLASH_INFO_t _gtSpinandInfo;


#if defined(MSOS_TYPE_LINUX)
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#endif

//==========================================================================
// Define
//==========================================================================

//==========================================================================
// Global Variable
//==========================================================================
U8 u8MagicData[] = {0x4D, 0x53, 0x54, 0x41, 0x52, 0x53, 0x45, 0x4D, 0x49, 0x55, 0x53, 0x46, 0x44, 0x43, 0x49, 0x53};
#define HW_TIMER_DELAY_1us          1
#define HW_TIMER_DELAY_10us         10
#define HW_TIMER_DELAY_100us        100
#define HW_TIMER_DELAY_1ms          (1000 * HW_TIMER_DELAY_1us)
#define HW_TIMER_DELAY_5ms          (5    * HW_TIMER_DELAY_1ms)
#if 0
SPINAND_FLASH_INFO_t gtSpiNandInfoTable[]=
{    //u8_IDByteCnt           au8_ID                                                                u16_SpareByteCnt   u16_PageByteCnt   u16_BlkPageCnt   u16_BlkCnt  u16_SectorByteCnt
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
#if defined(SUPPORT_SPINAND_QUAD) && SUPPORT_SPINAND_QUAD
SPINAND_MODE gSpiMode = E_SPINAND_QUAD_MODE;
#else
SPINAND_MODE gSpiMode = E_SPINAND_SINGLE_MODE;
#endif
Mdev_SPINAND_SetDriving SetDriving = NULL;
//==========================================================================
// Global Function
//==========================================================================
static BOOL _MDrv_SPINAND_GET_INFO(void)
{
    U32 u32Ret;
    U8 u8_IDByteCnt;
    U8 u8Spare[16];
    U8 u8Data[512];
    U8 u8Idx, u8Status;
    U8 *u8SrcAddr, *u8DstAddr;
    U16 u16PageIndex;
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
	u8SrcAddr = u8Data;
	u8DstAddr = (U8*)(&pSpiNandDrv->tSpinandInfo);

    // if ecc error read back up block(block2 ,4, 6, 8)
    for(u8Idx = 0; u8Idx < CIS_MAX_BACKUP * 2; u8Idx+=2)
    {
        u16PageIndex = (pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt ? pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt : 64) * u8Idx;
        //read data to cache first
        u32Ret = HAL_SPINAND_RFC(u16PageIndex, &u8Status);

        if(u32Ret != ERR_SPINAND_SUCCESS)//|| u8Status & ECC_STATUS_ERR
        {
            spi_nand_debug("read data to cache first != ERR_SPINAND_SUCCESS\n");
            continue;
        }

        // Read SPINand Data
//        printf("source %p", u8SrcAddr);
        u32Ret = HAL_SPINAND_Read (0, 512, u8SrcAddr);
        if(u32Ret != ERR_SPINAND_SUCCESS)
        {
            spi_nand_debug("Read SPINand Data != ERR_SPINAND_SUCCESS\n");
            continue;
        }

        // Read SPINand Spare Data
        u32Ret = HAL_SPINAND_Read(2048, 16, u8Spare);
        if(u32Ret != ERR_SPINAND_SUCCESS)
        {
            spi_nand_debug("Read SPINand Spare Data != ERR_SPINAND_SUCCESS\n");
            continue;
        }

        if(memcmp((const void *) u8SrcAddr, (const void *) u8MagicData, sizeof(u8MagicData)) != 0)
        {
            spi_nand_err("Magic number Error %p\n", u8SrcAddr);
            continue;
        }

        u8SrcAddr += sizeof(u8MagicData);
        memcpy(u8DstAddr, u8SrcAddr, sizeof(SPINAND_FLASH_INFO_t));
        {
            //HAL_SPINAND_ReadID(pSpiNandDrv->tSpinandInfo.u8_IDByteCnt, pSpiNandDrv->tSpinandInfo.au8_ID);
            //Use max 3 bytes to read ID because GCIS might fixed to 2 bytes only.
            HAL_SPINAND_ReadID(SPINAND_ID_SIZE, pSpiNandDrv->tSpinandInfo.au8_ID);
            if(pSpiNandDrv->tSpinandInfo.u8_IDByteCnt != SPINAND_ID_SIZE)
            {
                if((pSpiNandDrv->tSpinandInfo.au8_ID[2] == pSpiNandDrv->tSpinandInfo.au8_ID[0]) ||
                    pSpiNandDrv->tSpinandInfo.au8_ID[2] == 0xFF || pSpiNandDrv->tSpinandInfo.au8_ID[2] == 0x7F/*ESMT*/)
                {//2 bytes
                    pSpiNandDrv->tSpinandInfo.au8_ID[2] = 0;
                }
                else
                {//3 bytes
                    pSpiNandDrv->tSpinandInfo.u8_IDByteCnt = 3;
                }
            }
            spi_nand_err("Found SPINAND INFO ");
            for(u8_IDByteCnt = 0; u8_IDByteCnt < pSpiNandDrv->tSpinandInfo.u8_IDByteCnt;u8_IDByteCnt++)
            {
                printf("(0x%X) ",pSpiNandDrv->tSpinandInfo.au8_ID[u8_IDByteCnt]);
            }
            printf("\r\n");
        }
        return TRUE;
    }

    return FALSE;
}

BOOL MDrv_SPINAND_Init(SPINAND_FLASH_INFO_t *tSpinandInfo)
{

    U8 u8ID[SPINAND_ID_SIZE];
//	U32 u32Index;
    U32 u32Ret;
    BOOL bReFind = FALSE;
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    // 1. HAL init
    //
    _u8SPINANDDbgLevel = E_SPINAND_DBGLV_DEBUG;

MDrv_SPINAND_Init_Detect_ID:
#if 1
    u32Ret = HAL_SPINAND_Init();
    if(u32Ret != ERR_SPINAND_SUCCESS)
    {
        tSpinandInfo->au8_ID[0] = 0xFF;
        tSpinandInfo->au8_ID[1] = 0xFF;
        tSpinandInfo->u8_IDByteCnt = 2;
        if(!bReFind)
        {
            bReFind = TRUE;
            HAL_SPINAND_CSCONFIG();
            goto MDrv_SPINAND_Init_Detect_ID;
        }
        return FALSE;
    }
#endif
    /*_MDrv_SPINAND_GET_INFO : Search CIS in block 0 2 4 8.*/
    if(!_MDrv_SPINAND_GET_INFO())
    {
        u32Ret = HAL_SPINAND_ReadID(SPINAND_ID_SIZE, u8ID);
        if(u32Ret != ERR_SPINAND_SUCCESS)
        {
            spi_nand_debug("Can't not Detect SPINAND Device!!!!\r");
            tSpinandInfo->au8_ID[0] = 0xFF;
            tSpinandInfo->au8_ID[1] = 0xFF;
            tSpinandInfo->u8_IDByteCnt = 2;
            return FALSE;
        }
        spi_nand_debug("Spi-nand device detect! MID =%x, DID =%x\r",u8ID[0], u8ID[1]);
        tSpinandInfo->au8_ID[0] = u8ID[0];
        tSpinandInfo->au8_ID[1] = u8ID[1];
        tSpinandInfo->u8_IDByteCnt = 2;
        return FALSE;
#if 0
        spi_nand_debug("Search nand ID on internal table");
        for (u32Index = 0; gtSpiNandInfoTable[u32Index].au8_ID[0] != 0; u32Index++)
        {
            if(gtSpiNandInfoTable[u32Index].au8_ID[0] == u8ID[0] &&
               gtSpiNandInfoTable[u32Index].au8_ID[1] == u8ID[1])
            {
                spi_nand_debug("SPINAND Device DETECT");
                memcpy(tSpinandInfo, &gtSpiNandInfoTable[u32Index], sizeof(SPINAND_FLASH_INFO_t));
//                memcpy(&_gtSpinandInfo, &gtSpiNandInfoTable[u32Index], sizeof(SPINAND_FLASH_INFO_t));
                break;
            }
        }
        if((!gtSpiNandInfoTable[u32Index].au8_ID[0]) && (!gtSpiNandInfoTable[u32Index].au8_ID[1]))
        {
            tSpinandInfo->au8_ID[0] = u8ID[0];
            tSpinandInfo->au8_ID[1] = u8ID[1];
            tSpinandInfo->u8_IDByteCnt = 2;
            if(!bReFind)
            {
                bReFind = TRUE;
                HAL_SPINAND_CSCONFIG();
                goto MDrv_SPINAND_Init_Detect_ID;
            }
            return FALSE;
        }
#endif
    }
    else
    {
        memcpy(tSpinandInfo, &pSpiNandDrv->tSpinandInfo, sizeof(SPINAND_FLASH_INFO_t));
    }

    if(pSpiNandDrv->tSpinandInfo.au8_ID[0] == MID_GD)
    {
#if defined(SUPPORT_SPINAND_QUAD) && SUPPORT_SPINAND_QUAD
        if(pSpiNandDrv->tSpinandInfo.au8_ID[1] == 0xD1 ||
           pSpiNandDrv->tSpinandInfo.au8_ID[1] == 0xD9)
        {
            gSpiMode = E_SPINAND_QUAD_MODE_IO;
            printk("\r\n GD Quad mode enabled\r\n");
            //HAL_SPINAND_SetCKG(108);
            HAL_SPINAND_PreHandle(gSpiMode);
        }
#else
        //printk("\r\n XXXX regular mode XXXX\r\n");
#endif
        SetDriving = GD_SPINAND_SetDriving;
    }

    if (pSpiNandDrv->tSpinandInfo.au8_ID[0] == MID_XTX) /*XTX NAND flash need to pull down QE bit when use single mode*/
    {
#if !defined(SUPPORT_SPINAND_QUAD) || (0 == SUPPORT_SPINAND_QUAD)

        U8 u8Status;

        HAL_SPINAND_ReadStatusRegister(&u8Status, SPI_NAND_REG_FEAT);
        if(u8Status & QUAD_ENABLE)
        {
            u8Status &= ~(QUAD_ENABLE);
            HAL_SPINAND_WriteStatusRegister(u8Status, SPI_NAND_REG_FEAT);
        }
#endif
    }

    return TRUE;
}
/*void MDrv_SPINAND_ForceInit(SPINAND_FLASH_INFO_t *tSpinandInfo)
{
    memcpy(&_gtSpinandInfo, tSpinandInfo, sizeof(SPINAND_FLASH_INFO_t));
}*/

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
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();

    if((pSpiNandDrv->tSpinandInfo.au8_ID[0] == 0xEF) &&
       (pSpiNandDrv->tSpinandInfo.au8_ID[1] == 0xAB) &&
       (pSpiNandDrv->tSpinandInfo.au8_ID[2] == 0x21))
    {
        U16 u16DiePageCnt = MDrv_SPINAND_CountBits(pSpiNandDrv->tSpinandInfo.u16_BlkCnt*pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt);
        HAL_SPINAND_DieSelect((U8)(u32_PageIdx >> (u16DiePageCnt-1)));
    }

    //read data to cache first
    u32Ret = HAL_SPINAND_RFC(u32_PageIdx, &u8Status);

    if(u32Ret != ERR_SPINAND_SUCCESS)
    {
        return u32Ret;
    }

//    HAL_SPINAND_PLANE_HANDLER(u32_PageIdx);

    // Read SPINand Data

    if(pSpiNandDrv->tSpinandInfo.u8PlaneCnt && (((u32_PageIdx/pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt)&0x1) == 1))
    {
        u16ColumnAddr = (1<<12); // plane select for MICRON & 2GB
        spi_nand_msg("PLANE: u16ColumnAddr %x\n", u16ColumnAddr);
    }

#if defined(SUPPORT_SPINAND_QUAD) && SUPPORT_SPINAND_QUAD
    HAL_SPINAND_SetMode(gSpiMode);
#endif
    u32Ret = HAL_SPINAND_Read (u16ColumnAddr, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt, u8Data);
#if defined(SUPPORT_SPINAND_QUAD) && SUPPORT_SPINAND_QUAD
    HAL_SPINAND_SetMode(E_SPINAND_SINGLE_MODE);
#endif

    if(u32Ret != ERR_SPINAND_SUCCESS)
    {
        return u32Ret;
    }

    // Read SPINand Spare Data
    u32Ret= HAL_SPINAND_Read(pSpiNandDrv->tSpinandInfo.u16_PageByteCnt, pSpiNandDrv->tSpinandInfo.u16_SpareByteCnt, pu8_SpareBuf);

    if(u32Ret == ERR_SPINAND_SUCCESS)
    {
        if(u8Status == ECC_1_3_CORRECTED)
            u32Ret = ERR_SPINAND_ECC_1_3_CORRECTED;
        if(u8Status == ECC_4_6_CORRECTED)
           u32Ret = ERR_SPINAND_ECC_4_6_CORRECTED;
        if(u8Status == ECC_7_8_CORRECTED)
            u32Ret = ERR_SPINAND_ECC_7_8_CORRECTED;        
        if(u8Status == ECC_NOT_CORRECTED){
            u32Ret = ERR_SPINAND_ECC_ERROR;
            printf("ecc error P: 0x%x\r\n", u32_PageIdx);
        }
    }
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
    return HAL_SPINAND_Read_RandomIn(u32_PageIdx, u32_Column, u32_Byte, u8Data);
}

U32 MDrv_SPINAND_ReadStatusRegister(MS_U8 *u8Status, MS_U8 u8Addr)
{
    return HAL_SPINAND_ReadStatusRegister(u8Status, u8Addr);
}

U32 MDrv_SPINAND_WriteStatusRegister(MS_U8 *u8Status, MS_U8 u8Addr)
{
    return HAL_SPINAND_WriteStatusRegister(*u8Status, u8Addr);
}

U32 MDrv_SPINAND_SetMode(SPINAND_MODE eMode)
{
    return HAL_SPINAND_SetMode(eMode);
}

U32 MDrv_SPINAND_Write(U32 u32_PageIdx, U8 *u8Data, U8 *pu8_SpareBuf)
{
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();

    if((pSpiNandDrv->tSpinandInfo.au8_ID[0] == 0xEF) &&
       (pSpiNandDrv->tSpinandInfo.au8_ID[1] == 0xAB) &&
       (pSpiNandDrv->tSpinandInfo.au8_ID[2] == 0x21))
    {
        U16 u16DiePageCnt = MDrv_SPINAND_CountBits(pSpiNandDrv->tSpinandInfo.u16_BlkCnt*pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt);
        HAL_SPINAND_DieSelect((U8)(u32_PageIdx >> (u16DiePageCnt-1)));
    }
    return HAL_SPINAND_Write(u32_PageIdx, u8Data, pu8_SpareBuf);
}

U8 MDrv_SPINAND_ReadID(U16 u16Size, U8 *u8Data)
{
   return HAL_SPINAND_ReadID(u16Size, u8Data);
}

U32 MDrv_SPINAND_BLOCK_ERASE(U32 u32_PageIdx)
{
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();

    if((pSpiNandDrv->tSpinandInfo.au8_ID[0] == 0xEF) &&
       (pSpiNandDrv->tSpinandInfo.au8_ID[1] == 0xAB) &&
       (pSpiNandDrv->tSpinandInfo.au8_ID[2] == 0x21))
    {
        U16 u16DiePageCnt = MDrv_SPINAND_CountBits(pSpiNandDrv->tSpinandInfo.u16_BlkCnt*pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt);
        HAL_SPINAND_DieSelect((U8)(u32_PageIdx >> (u16DiePageCnt-1)));
    }
    return HAL_SPINAND_BLOCKERASE(u32_PageIdx);
}

U32 MDrv_SPINAND_EnableOtp(BOOL bEnable)
{
    return HAL_SPINAND_EnableOtp(bEnable);
}

U32 MDrv_SPINAND_LockOtp(void)
{
    return HAL_SPINAND_LockOtp();
}

U8 MDrv_SPINAND_WB_BBM(U32 u32LBA, U32 u32PBA)
{
    printf("[WB bbm] replace LBA 0x%x to PBA 0x%x \r\n", u32LBA, u32PBA);
    return HAL_SPINAND_WB_BBM(u32LBA, u32PBA);
}

U8 MDrv_SPINAND_WB_DumpBBM(U8 *u8Data)
{
    return HAL_SPINAND_WB_DumpBBM(u8Data);
}

void SpiNandMain(unsigned int dwSramAddress, unsigned int dwSramSize)
{
    U8 u8Data[2];
    MDrv_SPINAND_ReadID(2,u8Data);
}

BOOL MDrv_SPINAND_SetDevDriving(U16 u16Driving)
{
    if(SetDriving)
    {
        return SetDriving(u16Driving);
    }
    printf("No Dev Driving Setting function\n");
    return 0;
}
