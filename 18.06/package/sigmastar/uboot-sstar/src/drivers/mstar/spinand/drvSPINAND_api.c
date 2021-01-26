/*
* drvSPINAND_api.c- Sigmastar
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
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <asm/io.h>
#include <asm/errno.h>
#include "MsCommon.h"
#include "MsTypes.h"
#include "spinand.h"

/* SPI NAND messages */

//  static U32 CheckSum(U8 *pu8_Data, U16 u16_ByteCnt)
//  {
//      U32 u32_Sum = 0;
//
//      while (u16_ByteCnt--)
//      u32_Sum += *pu8_Data++;
//
//      return u32_Sum;
//  }

void dumpPartition(PARTITION_INFO_t *ptPartInfo, SPI_NAND_DRIVER_t *pSpiNandDrv)
{
    printf("PartInfo(.pni) isn't equal to CISInfo(.sni)!\n");
    printf("PartInfo(.pni)\n");
    printf("        Spare   %d\n",ptPartInfo->u16_SpareByteCnt);
    printf("        Page    %d\n",ptPartInfo->u16_PageByteCnt);
    printf("        BlkPage %d\n",ptPartInfo->u16_BlkPageCnt);
    printf("        BlkCnt  %d\n",ptPartInfo->u16_BlkCnt);

    printf("CISInfo(.sni) \n");
    printf("        Spare   %d\n",pSpiNandDrv->tSpinandInfo.u16_SpareByteCnt);
    printf("        Page    %d\n",pSpiNandDrv->tSpinandInfo.u16_PageByteCnt);
    printf("        BlkPage %d\n",pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt);
    printf("        BlkCnt  %d\n",pSpiNandDrv->tSpinandInfo.u16_BlkCnt);
}

/* return 1: Good block, 0: Bad block */
U32 MDrv_SPINAND_IsGoodBlk(U16 u16_PBA)
{
    U32 u32_PageIdx;
    U32 u32_Err;
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    u32_PageIdx = u16_PBA * pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
    u32_Err = MDrv_SPINAND_Read(u32_PageIdx, pSpiNandDrv->pu8_pagebuf, pSpiNandDrv->pu8_sparebuf);

    if(u32_Err != ERR_SPINAND_SUCCESS)
    spi_nand_err("MDrv_SPINAND_Read(0x%x)=0x%x\n", u32_PageIdx, u32_Err);

    if(pSpiNandDrv->pu8_sparebuf[0] != 0xFF)
    return 0;

    return 1;
}

int MDrv_SPINAND_WriteCIS_for_ROM(SPINAND_FLASH_INFO_TAG_t *pSpiNandInfoTagOut, int nCopies)
{
    U32 u32_Err = ERR_SPINAND_SUCCESS;
    U16 u16_PBA;
    U32 u32_PageIdx;

    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();

    /* Search for two good blocks within the first 10 physical blocks */
    for (u16_PBA = 0; u16_PBA < nCopies * 2; u16_PBA += 2)
    {
        /* Check first page of block */
        u32_PageIdx = u16_PBA * pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
        u32_Err = MDrv_SPINAND_Read(u32_PageIdx, pSpiNandDrv->pu8_pagebuf, pSpiNandDrv->pu8_sparebuf);

        if(u32_Err != ERR_SPINAND_SUCCESS)
            spi_nand_err("MDrv_SPINAND_Read(0x%x)=0x%x\n", u32_PageIdx, u32_Err);
        u32_Err = MDrv_SPINAND_BLOCK_ERASE(u32_PageIdx);

        if(u32_Err != ERR_SPINAND_SUCCESS)
        {
            spi_nand_err("MDrv_SPINAND_BLOCK_ERASE(0x%X)=0x%x\n", u16_PBA, u32_Err);
            continue;
        }

        memset(pSpiNandDrv->pu8_pagebuf, 0xFF, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt);
        memcpy(pSpiNandDrv->pu8_pagebuf, pSpiNandInfoTagOut, sizeof(SPINAND_FLASH_INFO_TAG_t));
        memset(pSpiNandDrv->pu8_sparebuf, 0xFF, pSpiNandDrv->tSpinandInfo.u16_SpareByteCnt);
        u32_Err = MDrv_SPINAND_Write(u32_PageIdx, pSpiNandDrv->pu8_pagebuf, pSpiNandDrv->pu8_sparebuf);
        if(u32_Err != ERR_SPINAND_SUCCESS)
        {
            spi_nand_err("MDrv_SPINAND_Write(0x%x)=0x%x\n", u32_PageIdx, u32_Err);
            continue;
        }

        /**
         ** Write Partition Info the 2nd page
         **/

        if(pSpiNandDrv->u8_HasPNI == 1)
        {
            memset(pSpiNandDrv->pu8_pagebuf, 0xFF, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt);
            memcpy(pSpiNandDrv->pu8_pagebuf, &pSpiNandDrv->tPartInfo, 0x200);
            u32_Err = MDrv_SPINAND_Write(u32_PageIdx + 1, pSpiNandDrv->pu8_pagebuf, pSpiNandDrv->pu8_sparebuf);
            if(u32_Err != ERR_SPINAND_SUCCESS)
            {
                spi_nand_err("MDrv_SPINAND_Write(0x%x)=0x%x\n", u32_PageIdx + 1, u32_Err);
                continue;
            }
        }

        spi_nand_msg("CIS is written to blk 0x%04x", u16_PBA);

    }

    return ERR_SPINAND_SUCCESS;
}

static U32 _MDrv_SPINAND_checkSum(U8 *pu8_Data, U16 u16_ByteCnt)
{
    U32 u32_Sum = 0;

    while (u16_ByteCnt--)
        u32_Sum += *pu8_Data++;
    return u32_Sum;
}


static int _MDrv_SPINAND_GetMtdPartsFromSNI(char *buf)
{
        char *mtd_buf=buf, tmp[32];
        int len, u8_i, Maxlen = 512;
        SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
        u32 blockSize = pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt * pSpiNandDrv->tSpinandInfo.u16_PageByteCnt;

        sprintf(mtd_buf, "mtdparts=nand0:");
        mtd_buf += strlen(mtd_buf);
        Maxlen -= strlen(mtd_buf);

        for(u8_i = 0; u8_i < IPL_BACKUP; u8_i++)
        {
            if(u8_i == 0)
                sprintf(tmp, "0x%X@0x%X(IPL%d)", SBOOT_MAXBLOCK * blockSize, CIS_DEFAULT_BACKUP * blockSize, u8_i);
            else
                sprintf(tmp, ",0x%X(IPL%d)", SBOOT_MAXBLOCK * blockSize, u8_i);
            len = strlen(tmp);
            memcpy(mtd_buf, tmp, len);
            mtd_buf += len;
            if (Maxlen < len)
            goto cleanup;
            Maxlen -= len;
        }
        for(u8_i = 0; u8_i < IPL_CUST_BACKUP; u8_i++)
        {
            sprintf(tmp, ",0x%X(IPL_CUST%d)", SBOOT_MAXBLOCK * blockSize, u8_i);
            len = strlen(tmp);
            memcpy(mtd_buf, tmp, len);
            mtd_buf += len;
            if (Maxlen < len)
            goto cleanup;
            Maxlen -= len;
        }
        for(u8_i = 0; u8_i < UBOOT_BACKUP; u8_i++)
        {
            sprintf(tmp, ",0x%X(UBOOT%d)", UBOOT_MAXBLOCK * blockSize, u8_i);
            len = strlen(tmp);
            memcpy(mtd_buf, tmp, len);
            mtd_buf += len;
            if (Maxlen < len)
            goto cleanup;
            Maxlen -= len;
        }
        /* env */
        sprintf(tmp, ",0x%X(ENV)", blockSize);
        len = strlen(tmp);
        memcpy(mtd_buf, tmp, len);
        mtd_buf += len;
        if (Maxlen < len)
        goto cleanup;
        Maxlen -= len;
        *mtd_buf = '\0';
        return 0;
    cleanup:
        buf[0] = '\0';
        return -1;
}


static int _MDrv_SPINAND_GetMtdPartsFromPNI(char *buf)
{
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    PARTITION_INFO_t *pPartInfo = &pSpiNandDrv->tPartInfo;
    U8 u8_i, u8_PartNo = 0;
    int len, Maxlen = 512;
    U32 PartSize = 0;
    char *mtd_buf=buf, tmp[32], PartName[32];
    U16 u16_LastPartType;
    BOOL bFound;
    BOOL bLogic = FALSE;
    BOOL bFirst = TRUE;
    U16 nMaxPart;
    U32 nPartTypeNoFlag;

    sprintf(mtd_buf, "mtdparts=nand0:");
    mtd_buf += 15;
    Maxlen -= 15;

    u16_LastPartType = 0;
    bFound = FALSE;
    nMaxPart = sizeof(pPartInfo->records) / sizeof(pPartInfo->records[0]);
    for(u8_i = 0; u8_i < pPartInfo->u16_PartCnt && u8_i < nMaxPart; u8_i ++)
    {
        if( (pPartInfo->records[u8_i].u16_PartType & UNFD_LOGI_PART) == UNFD_LOGI_PART)
            bLogic = TRUE;
        else
            bLogic = FALSE;
        if( (pPartInfo->records[u8_i].u16_PartType & UNFD_HIDDEN_PART) == UNFD_HIDDEN_PART)
            continue;

        nPartTypeNoFlag = pPartInfo->records[u8_i].u16_PartType & 0x0FFF;

        PartSize = 0;
        if( bFound )
        {
            memcpy(mtd_buf, ",", 1);
            mtd_buf ++;
            Maxlen --;
        }
        bFound = FALSE;

        PartSize = pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt * pSpiNandDrv->tSpinandInfo.u16_PageByteCnt *
            (pPartInfo->records[u8_i].u16_BackupBlkCnt + pPartInfo->records[u8_i].u16_BlkCnt);

        if(u16_LastPartType != pPartInfo->records[u8_i].u16_PartType)
        {
            u8_PartNo = 0;
        }
        else
        {
            u8_PartNo++;
        }
        u16_LastPartType = pPartInfo->records[u8_i].u16_PartType;

        switch(nPartTypeNoFlag)
        {
            case UNFD_PART_CIS:
                sprintf(PartName,"CIS");
                break;
            case UNFD_PART_IPL_CUST:
                sprintf(PartName,"IPL_CUST%d",u8_PartNo );
                break;
            case UNFD_PART_BOOTLOGO:
                sprintf(PartName,"LOGO" );
                break;
            case UNFD_PART_IPL:
                sprintf(PartName,"IPL%d",u8_PartNo );
                break;
            case UNFD_PART_CUS:
                sprintf(PartName,"CUS" );
                break;
            case UNFD_PART_UBOOT:
                sprintf(PartName,"UBOOT%d",u8_PartNo );
                break;
            case UNFD_PART_OS:
                sprintf(PartName,"KERNEL" );
                break;
            case UNFD_PART_SECINFO:
                sprintf(PartName,"SECINFO" );
                break;
            case UNFD_PART_OTP:
                sprintf(PartName,"OTP" );
                break;
            case UNFD_PART_RECOVERY:
                sprintf(PartName,"RECOVERY" );
                break;
            case UNFD_PART_E2PBAK:
                sprintf(PartName,"E2PBAK" );
                break;
            case UNFD_PART_NVRAMBAK:
                sprintf(PartName,"NVRAMBAK" );
                break;
            case UNFD_PART_NPT:
                sprintf(PartName,"NPT" );
                break;
            case UNFD_PART_ENV:
                if(!u8_PartNo)
                {
                    sprintf(PartName,"ENV");
                }
                else
                {
                    sprintf(PartName,"ENV%d",u8_PartNo );
                }
                break;
            case UNFD_PART_MISC:
                sprintf(PartName,"MISC" );
                break;
            case UNFD_PART_RTOS:
                sprintf(PartName,"RTOS" );
                break;
            case UNFD_PART_RTOS_BAK:
                sprintf(PartName,"RTOS_BACKUP" );
                break;
            case UNFD_PART_KERNEL:
                sprintf(PartName,"KERNEL" );
                break;
            case UNFD_PART_KERNEL_BAK:
                sprintf(PartName,"KERNEL_BACKUP" );
                break;
            case UNFD_PART_UBI:
                sprintf(PartName,"UBI" );
                break;
            case UNFD_PART_ROOTFS:
                sprintf(PartName,"rootfs" );
                break;
            case UNFD_PART_ROOTFS_BAK:
                sprintf(PartName,"rootfs_bak" );
                break;
			default:
			    if(nPartTypeNoFlag >= UNFD_PART_CUST0 &&
			       nPartTypeNoFlag <= UNFD_PART_CUSTf)
			    {
			        sprintf(PartName,"CUST%d", nPartTypeNoFlag - UNFD_PART_CUST0);
			    }
			    else
			    {
			        sprintf(PartName,"UNKNOWN%d",u8_i );
			    }
				break;
		}
        bFound = TRUE;
        if (bFirst)
        {
			sprintf(tmp, "0x%X@0x%X(%s)", (unsigned int)PartSize,
                pPartInfo->records[u8_i].u16_StartBlk * pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt *
                    pSpiNandDrv->tSpinandInfo.u16_PageByteCnt, PartName); //,pPartInfo->records[u8_i].u16_PartType);
            bFirst = FALSE;
        }
        else
            sprintf(tmp, "0x%X(%s)", (unsigned int)PartSize, PartName); //,pPartInfo->records[u8_i].u16_PartType);

        len = strlen(tmp);
        memcpy(mtd_buf, tmp, len);
        mtd_buf += len;
        if (Maxlen < len)
            goto cleanup;
        Maxlen -= len;

    }

    while(bLogic && u8_i > 0)
    {
        if(pPartInfo->records[u8_i - 1].u16_PartType == 0xC000)
            sprintf(tmp,",-(UBI)");
        else if((pPartInfo->records[u8_i - 1].u16_PartType & UNFD_HIDDEN_PART) == 0)
            sprintf(tmp,",-(%s)", PartName);
        else
        {
            //spi_nand_err("Unable to parse type 0x%X\n", pPartInfo->records[u8_i - 1].u16_PartType);
            break;
        }
        len= strlen(tmp);
        memcpy(mtd_buf, tmp, len);
        mtd_buf += len;
        if (Maxlen < len)
            goto cleanup;
        Maxlen -= len;
        break;
    }

    *mtd_buf = '\0';
    return 0;

cleanup:
    buf[0] = '\0';
    return -1;
}

int MDrv_SPINAND_SearchCIS_in_DRAM(U8 *pu8_CISAddr, U8 *pu8_PNIAddr, SPINAND_FLASH_INFO_TAG_t *pSpiNandInfoTagOut)
{
    SPINAND_FLASH_INFO_TAG_t *pSpiNandInfoTag;
    SPINAND_FLASH_INFO_t *ptSpinandInfo;
    PARTITION_INFO_t *ptPartInfo;
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    U32 u32_i, u32_j, u32_chkSum;
    U8 u8_IsEnd = 0;

    spi_nand_err("Search CIS in DRAM");

    //Search 20 pieces of possiable cis memory
    u32_i = 0;
//  printf("pu8_CISAddr %x\n",pu8_CISAddr);
//  printf("pSpiNandDrv->tSpinandInfo.au8_ID[0] %x\n",pSpiNandDrv->tSpinandInfo.au8_ID[0]);

    while(1)
    {
        pSpiNandInfoTag = (SPINAND_FLASH_INFO_TAG_t *)(pu8_CISAddr + u32_i);
        ptSpinandInfo = &pSpiNandInfoTag->tSpiNandInfo;

        if(u32_i == 0)
        {
            if(ptSpinandInfo->u8_IDByteCnt == 5 &&
                    ptSpinandInfo->au8_ID[0] == 0xEE && ptSpinandInfo->au8_ID[1] == 0xEE)
            {
                spi_nand_err("SNI v:%d.%d.%d", ptSpinandInfo->au8_ID[2], ptSpinandInfo->au8_ID[3], ptSpinandInfo->au8_ID[4]);
            }
        }

        for(u32_j = 0; u32_j < pSpiNandDrv->tSpinandInfo.u8_IDByteCnt; u32_j++)
        {
            if(pSpiNandDrv->tSpinandInfo.au8_ID[u32_j] != ptSpinandInfo->au8_ID[u32_j])
            break;
        }

        if(u32_j == pSpiNandDrv->tSpinandInfo.u8_IDByteCnt)
        break;

        if(memcmp((const void *) pSpiNandInfoTag->au8_Tag, (const void *) SPINAND_FLASH_INFO_TAG, 16))
        {
            u8_IsEnd = 1;
            break;
        }
        u32_i += 0x200;
    }

    if(u8_IsEnd == 1)
    {
        spi_nand_err("No available SNI match with current SPINAND flash");
        return ERR_SPINAND_INVALID;
    }
    spi_nand_err("Found SNI in given memory");

    memcpy(&pSpiNandDrv->tSpinandInfo, ptSpinandInfo, sizeof(SPINAND_FLASH_INFO_t));
    //MDrv_SPINAND_ForceInit(ptSpinandInfo);

    /*Search pni*/
    pSpiNandDrv->u8_HasPNI = 0;

    if(pu8_PNIAddr != NULL)
    {
        ptPartInfo = (PARTITION_INFO_t*) (pu8_PNIAddr);
        memcpy(&pSpiNandDrv->tPartInfo, ptPartInfo, 0x200);
        /*Add checksum*/
        u32_chkSum = _MDrv_SPINAND_checkSum((U8*)ptPartInfo + 0x04, 0x200 - 0x04);
        if(u32_chkSum== ptPartInfo->u32_ChkSum)
        {
            spi_nand_msg("PNI match success");
            pSpiNandDrv->u8_HasPNI = 1;
        }
        else
        {
            spi_nand_err("PNI mismatch \x1b[33m%X\x1b[0m", u32_chkSum);
            pSpiNandDrv->u8_HasPNI = 0;
            return ERR_SPINAND_INVALID;
        }
    }
    else
    {
        spi_nand_err("CIS doesn't contain part info");
        pSpiNandDrv->u8_HasPNI = 0;
    }
    memcpy(pSpiNandInfoTagOut, pSpiNandInfoTag , sizeof(SPINAND_FLASH_INFO_TAG_t));
    return ERR_SPINAND_SUCCESS;
}

int MDrv_SPINAND_GetMtdParts(char *buf)
{
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();

    if(1 == pSpiNandDrv->u8_HasPNI && 0 == (pSpiNandDrv->tSpinandInfo.u8_UBOOTPBA))
    {
        spi_nand_err("Force Search mtdparts from PNI...\n");
        return _MDrv_SPINAND_GetMtdPartsFromPNI(buf);
    }
    else if(1 == pSpiNandDrv->u8_HasPNI)
    {
        spi_nand_err("Search mtdparts from PNI...\n");
        return _MDrv_SPINAND_GetMtdPartsFromPNI(buf);
    }
    else if (0 != (pSpiNandDrv->tSpinandInfo.u8_BL0PBA))
    {
        spi_nand_err("Search mtdparts from SNI...\n");
        return _MDrv_SPINAND_GetMtdPartsFromSNI(buf);
    }
    else
    {
        spi_nand_err("Search mtdparts fail...\n");
        return -1;
    }
}
/* Search for partition of type @u16_PartType, begin from @pRecord */
PARTITION_RECORD_t *MDrv_SPINAND_SearchPartition(PARTITION_RECORD_t *pRecord,
                                            U16 u16_PartType)
{
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    PARTITION_INFO_t *pPartInfo = &pSpiNandDrv->tPartInfo;

    while (pRecord - pPartInfo->records < pPartInfo->u16_PartCnt) {

        if (pRecord->u16_PartType == u16_PartType)
            return pRecord;

        pRecord++;
    }

    return (void*)0;
}

int MDrv_SPINAND_GetPartOffset(U16 u16_PartType, U32* u32_Offset)
{
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    u16 ENVPBA = 0;
    u32 blockSize = pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt * pSpiNandDrv->tSpinandInfo.u16_PageByteCnt;

    if (0 != (pSpiNandDrv->tSpinandInfo.u8_UBOOTPBA))
    {
        ENVPBA = ((CIS_DEFAULT_BACKUP + IPL_BACKUP + IPL_CUST_BACKUP) * SBOOT_MAXBLOCK)+(UBOOT_BACKUP * UBOOT_MAXBLOCK);
        *u32_Offset = ENVPBA * blockSize;
        return 0;
    }
    else if(1 == pSpiNandDrv->u8_HasPNI)
    {
        PARTITION_INFO_t *pPartInfo = &pSpiNandDrv->tPartInfo;
        PARTITION_RECORD_t *pRecord = pPartInfo->records;
        pRecord = MDrv_SPINAND_SearchPartition(pRecord, u16_PartType);

        if (!pRecord)
        {
            *u32_Offset = 0;
            return -1;
        }
        *u32_Offset = pRecord->u16_StartBlk * pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt * pSpiNandDrv->tSpinandInfo.u16_PageByteCnt;
        return 0;
    }
    else
    {
        ENVPBA = ((CIS_DEFAULT_BACKUP + IPL_BACKUP + IPL_CUST_BACKUP) * SBOOT_MAXBLOCK)+(UBOOT_BACKUP * UBOOT_MAXBLOCK);
        *u32_Offset = ENVPBA * blockSize;
        spi_nand_err("UBOOT_PBA==0 and no PNI: %d %d %d", pSpiNandDrv->tSpinandInfo.u8_BL0PBA, pSpiNandDrv->tSpinandInfo.u8_BL1PBA, pSpiNandDrv->tSpinandInfo.u8_UBOOTPBA);
        spi_nand_err("use offset %X", *u32_Offset);
        return 0;
        //spi_nand_err("Search part fail...\n");
        //return -1;
    }
}

int MDrv_SPINAND_ReadCISBlk(U8* pu8_DataBuf)
{
    U32 u32_Err = ERR_SPINAND_SUCCESS;
    U16 u16Idx = 0;
    U16 u16PageIndex = 0;
    U16 u16BlkIndex = 10;
    U16 page_off = 0;

    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    page_off = pSpiNandDrv->tSpinandInfo.u16_PageByteCnt + pSpiNandDrv->tSpinandInfo.u16_SpareByteCnt;
    u16PageIndex = (u16BlkIndex * pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt) - 1;

    for(u16Idx = 0; u16Idx <= u16PageIndex; u16Idx++)
    {
        u32_Err = MDrv_SPINAND_Read(u16Idx, pu8_DataBuf, pu8_DataBuf + pSpiNandDrv->tSpinandInfo.u16_PageByteCnt);
        pu8_DataBuf += page_off;
    }
    printf("Read total size 0x%X\n", page_off * (u16Idx));
    return u32_Err;
}


static __inline void dump_mem_line(unsigned char *buf, int cnt)
{
    int i;

    printk(KERN_NOTICE" 0x%x: ", (U32)buf);
    for (i= 0; i < cnt; i++)
    printk(KERN_NOTICE"%02X ", buf[i]);

    printk(KERN_NOTICE" | ");

    for (i = 0; i < cnt; i++)
    printk(KERN_NOTICE"%c", (buf[i] >= 32 && buf[i] < 128) ? buf[i] : '.');

    printk(KERN_NOTICE"\n");
}

void dump_mem(unsigned char *buf, int cnt)
{
    int i;

    for (i= 0; i < cnt; i+= 16)
    dump_mem_line(buf + i, 16);
}

#if 0
void HWTimer_Start(void)
{
    // reset HW timer
    (*(volatile U16*)(TIMER1_MAX_LOW)) = 0xFFFF;
    (*(volatile U16*)(TIMER1_MAX_HIGH)) = 0xFFFF;
    (*(volatile U16*)(TIMER1_ENABLE)) = 0;

    // start HW timer
    (*(volatile U16*)(TIMER1_ENABLE)) |= 0x0001;
}

U32 HWTimer_End(void)
{
    U32 u32HWTimer = 0;
    U16 u16TimerLow = 0;
    U16 u16TimerHigh = 0;

    // Get HW timer
    u16TimerLow = (*(volatile U16*)(TIMER1_CAP_LOW));
    u16TimerHigh = (*(volatile U16*)(TIMER1_CAP_HIGH));

    u32HWTimer = (u16TimerHigh<<16) | u16TimerLow;

    return u32HWTimer;
}
#endif

#if 0
int MDrv_SPINAND_write_bootloader(U32 u32_Row,U8 * pu8_addr, U32 u32_size, U8 u8_BootStageId)
{
    U8 *pu8_DataBuf = pu8_addr;
    U16 u16_BlkPageCnt;
    U32 u32_pagecnt, u32_Err, u32_size_tmp = u32_size, u32_Row_Read = u32_Row;
    U16 u16_i;
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    u16_BlkPageCnt = pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
    u32_pagecnt = (u32_size+pSpiNandDrv->tSpinandInfo.u16_PageByteCnt-1)/pSpiNandDrv->tSpinandInfo.u16_PageByteCnt;

    //add register status for polling by TV Tool
    REG(RIU_BASE + (0x1980 << 2) + (0x41 << 2) )  = 0x0000;

    while(u32_pagecnt >= u16_BlkPageCnt)
    {
        while (MDrv_SPINAND_IsGoodBlk(u32_Row / pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt) == 0)
    {
    u32_Row += pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
    //bad block jump to next block
    if(u32_Row == (pSpiNandDrv->tSpinandInfo.u16_BlkCnt*pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt))
    {
    printk(KERN_NOTICE"Error : There is no available GOOD block in current nand device\n");
    REG(RIU_BASE + (0x1980 << 2) + (0x41 << 2) )  = 0x0004;
    return -1;
    }
    }
    u32_Err = MDrv_SPINAND_BLOCK_ERASE(u32_Row);
    if(u32_Err != ERR_SPINAND_SUCCESS)
    {
    //jump to next block
    u32_Row += pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
    continue;
    }

    memset(pSpiNandDrv->pu8_sparebuf, 0xFF, pSpiNandDrv->tSpinandInfo.u16_SpareByteCnt);
    pSpiNandDrv->pu8_sparebuf[4] = u8_BootStageId;

    for(u16_i = 0 ; u16_i<pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt ; u16_i++)
    {
    u32_Err = MDrv_SPINAND_Write(u32_Row+u16_i, pu8_DataBuf+(u16_i*pSpiNandDrv->tSpinandInfo.u16_PageByteCnt), gtSpiNandDrv.pu8_sparebuf);
    if(u32_Err != ERR_SPINAND_SUCCESS)
    {
    //jump to next block
    u32_Row += pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
    continue;
    }
    }
    pu8_DataBuf += u16_BlkPageCnt*pSpiNandDrv->tSpinandInfo.u16_PageByteCnt;

    u32_pagecnt -= u16_BlkPageCnt;
    u32_size -= u16_BlkPageCnt*pSpiNandDrv->tSpinandInfo.u16_PageByteCnt;
    u32_Row += pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
    }

    while(u32_size)
    {
    while (MDrv_SPINAND_IsGoodBlk(u32_Row/pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt) == 0)
    {
    u32_Row += pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
    //bad block jump to next block
    if(u32_Row == (pSpiNandDrv->tSpinandInfo.u16_BlkCnt*pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt))
    {
    printk(KERN_NOTICE"Error : There is no available GOOD block in current nand device\n");
    REG(RIU_BASE + (0x1980 << 2) + (0x41 << 2) )  = 0x0004;
    return -1;
    }
    }
    u32_Err = MDrv_SPINAND_BLOCK_ERASE(u32_Row);
    if(u32_Err != ERR_SPINAND_SUCCESS)
    {
    //jump to next block
    u32_Row += pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
    continue;
    }

    memset(pSpiNandDrv->pu8_sparebuf, 0xFF, pSpiNandDrv->tSpinandInfo.u16_SpareByteCnt);
    pSpiNandDrv->pu8_sparebuf[4] = u8_BootStageId;

    for(u16_i = 0 ; u16_i<u32_pagecnt ; u16_i++)
    {
    u32_Err = MDrv_SPINAND_Write(u32_Row+u16_i, pu8_DataBuf+(u16_i*pSpiNandDrv->tSpinandInfo.u16_PageByteCnt), gtSpiNandDrv.pu8_sparebuf);
    if(u32_Err != ERR_SPINAND_SUCCESS)
    {
    //jump to next block
    u32_Row += pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
    continue;
    }
    }

    u32_size-= u32_size;
    }
    //finish programming
    //Add register checking
    //read back check
    // TODO
    u32_size = u32_size_tmp;
    u32_pagecnt = (u32_size+pSpiNandDrv->tSpinandInfo.u16_PageByteCnt-1)/pSpiNandDrv->tSpinandInfo.u16_PageByteCnt;
    pu8_DataBuf = pu8_addr;
    u32_Row = u32_Row_Read;

    while(u32_pagecnt >= u16_BlkPageCnt)
    {
    while (MDrv_SPINAND_IsGoodBlk(u32_Row / pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt) == 0)
    {
    u32_Row += pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
    //bad block jump to next block
    if(u32_Row == (pSpiNandDrv->tSpinandInfo.u16_BlkCnt*pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt))
    {
    printk(KERN_NOTICE"Error : There is no available GOOD block in current nand device\n");
    REG(RIU_BASE + (0x1980 << 2) + (0x41 << 2) )  = 0x0004;
    return -1;
    }
    }

    for(u16_i =0; u16_i < u16_BlkPageCnt; u16_i ++)
    {
    u32_Err = MDrv_SPINAND_Read(u32_Row+u16_i, pSpiNandDrv->pu8_pagebuf, pSpiNandDrv->pu8_sparebuf);
    if(u32_Err != ERR_SPINAND_SUCCESS)
    {
    printk(KERN_NOTICE"Error: ECC fail when read back checking\n");
    REG(RIU_BASE + (0x1980 << 2) + (0x41 << 2) )  = 0x0002;
    return -1;
    }
    if(memcmp(pSpiNandDrv->pu8_pagebuf, pu8_DataBuf, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt))
    {
    printk(KERN_NOTICE"Error: compare fail when read back checking\n");
    printf("RAM--->\n");
    dump_mem((unsigned char *) pu8_DataBuf, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt);
    printf("NAND--->\n");
    dump_mem((unsigned char *) pSpiNandDrv->pu8_pagebuf, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt);
    REG(RIU_BASE + (0x1980 << 2) + (0x41 << 2) )  = 0x0003;
    return -1;
    }
    pu8_DataBuf += pSpiNandDrv->tSpinandInfo.u16_PageByteCnt;
    }


    u32_pagecnt -= u16_BlkPageCnt;
    u32_size -= (u16_BlkPageCnt*pSpiNandDrv->tSpinandInfo.u16_PageByteCnt);
    u32_Row += pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
    }

    while(u32_size)
    {
    while (MDrv_SPINAND_IsGoodBlk(u32_Row / pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt) == 0)
    {
    u32_Row += pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt;
    //bad block jump to next block
    if(u32_Row == (pSpiNandDrv->tSpinandInfo.u16_BlkCnt*pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt))
    {
    printk(KERN_NOTICE"Error : There is no available GOOD block in current nand device\n");
    REG(RIU_BASE + (0x1980 << 2) + (0x41 << 2) )  = 0x0004;
    return -1;
    }
    }

    for(u16_i=0; u16_i<u32_pagecnt; u16_i++)
    {
    u32_Err = MDrv_SPINAND_Read(u32_Row+u16_i, pSpiNandDrv->pu8_pagebuf, pSpiNandDrv->pu8_sparebuf);
    if(u32_Err != ERR_SPINAND_SUCCESS)
    {
    printk(KERN_NOTICE"Error: ECC fail when read back checking\n");
    REG(RIU_BASE + (0x1980 << 2) + (0x41 << 2) )  = 0x0002;
    return -1;
    }

    if(memcmp(pSpiNandDrv->pu8_pagebuf, pu8_DataBuf, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt))
    {
    printk(KERN_NOTICE"Error: compare fail when read back checking\n");
    printf("RAM--->\n");
    dump_mem((unsigned char *) pu8_DataBuf, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt);
    printf("NAND--->\n");
    dump_mem((unsigned char *) pSpiNandDrv->pu8_pagebuf, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt);
    REG(RIU_BASE + (0x1980 << 2) + (0x41 << 2) )  = 0x0003;
    return -1;
    }
    pu8_DataBuf += pSpiNandDrv->tSpinandInfo.u16_PageByteCnt;
    }

    u32_size -= u32_size;
    }
    //Register status Ok
    REG(RIU_BASE + (0x1980 << 2) + (0x41 << 2) )  = 0x0001;
    return 0;
}
#endif
#if 0
static u32 empty_check(const void *buf, u32 len)
{
    int i;
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    for (i = (len >> 2) - 1; i >= 0; i--)
        if (((const uint32_t *)buf)[i] != 0xFFFFFFFF)
            break;

    /* The resulting length must be aligned to the minimum flash I/O size */
    len = ALIGN((i + 1) << 2, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt + pSpiNandDrv->tSpinandInfo.u16_SpareByteCnt);
    return len;
}
#endif
#if 0
int MDrv_SPINAND_ReadCISBlk(U32 u32_off, U8* pu8_DataBuf)
{
    U32 u32_Err, i;// pglen;
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    #if 0
    #ifdef FULL_BLOCK_PROGRAM_SCRUB
    U32  chk_len;
    #endif
    #endif
    U32 u32_Row = u32_off/pSpiNandDrv->tSpinandInfo.u16_PageByteCnt;
    U32 page_off = pSpiNandDrv->tSpinandInfo.u16_PageByteCnt + pSpiNandDrv->tSpinandInfo.u16_SpareByteCnt;

//      pglen = pSpiNandDrv->tSpinandInfo.u16_PageByteCnt + pSpiNandDrv->tSpinandInfo.u16_SpareByteCnt;
    for(i=0 ; i<pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt; i++)
    {
    u32_Err = MDrv_SPINAND_Read(u32_Row+i, pu8_DataBuf+page_off*i, pu8_DataBuf+page_off*i+pSpiNandDrv->tSpinandInfo.u16_PageByteCnt);
    if(u32_Err != ERR_SPINAND_SUCCESS)
    return u32_Err;

    /*
     *For SLC, full block program is not necessary
     *Turn on this switch if Spinand is MLC
     */
    #if 0
    #ifdef FULL_BLOCK_PROGRAM_SCRUB
    chk_len = empty_check(pu8_DataBuf+page_off*i, pglen);
    if (!chk_len)
    {
        (pu8_DataBuf + page_off * i)[0] = 0xfe; // indicate empty page
    }
    #endif
    #endif
    }

    return ERR_SPINAND_SUCCESS;
}
#endif

#if 0
static U8 MDrv_SPINAND_CheckAll0xFF(U8* pu8_Buf, U32 u32_ByteCnt)
{
    register U32 u32_i;

    for(u32_i=0; u32_i<u32_ByteCnt; u32_i++)
    if(0xFF != pu8_Buf[u32_i])
    return 0;

    return 0xFF;
}

U32 MDrv_SPINAND_RefreshCIS(void)
{
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    SPINAND_FLASH_INFO_t *ptSpinandInfo;
    SPINAND_FLASH_INFO_TAG_t *pSpiNandInfoTag;
    PARTITION_INFO_t *pPartInfo = &pSpiNandDrv->tPartInfo;
    U32 u32_BlkIdx, u32_MBootBegin = 0, u32_MBootEnd = 0;
    U32 u32_Err;
    U8  u8_i;
    int bl_count = 0;

    pSpiNandInfoTag = (SPINAND_FLASH_INFO_TAG_t *)malloc(512);
    if(!pSpiNandInfoTag)
    {
    printk(KERN_NOTICE"Memory Allocate fail for pSpiNandInfoTag\n");
    return -1;
    }

    memset(pSpiNandInfoTag, 0, 512);
    memcpy(pSpiNandInfoTag->au8_Tag, SPINAND_FLASH_INFO_TAG, 16);
    ptSpinandInfo = &pSpiNandInfoTag->tSpiNandInfo;
    memcpy(ptSpinandInfo, &pSpiNandDrv->tSpinandInfo, sizeof(SPINAND_FLASH_INFO_t));

    //search MBOOT partition in partinfo

    if(pSpiNandDrv->u8_HasPNI == 1)
    {
    for(u8_i=0 ; u8_i<pPartInfo->u16_PartCnt ; u8_i++)
    {
    if(pPartInfo->records[u8_i].u16_PartType == MTD_LOGI_PART)
    {
    u32_MBootBegin = pPartInfo->records[u8_i].u16_StartBlk;
    u32_MBootEnd = pPartInfo->records[u8_i].u16_StartBlk
    + pPartInfo->records[u8_i].u16_BlkCnt
    + pPartInfo->records[u8_i].u16_BackupBlkCnt;
    break;
    }
    }
    if(u8_i == pPartInfo->u16_PartCnt)
    {
    printk(KERN_NOTICE"ERROR: Partition info does not contain MBOOT partition\n");
    return -1;
    }

    //search sboot uboot/ HashX location for update nni infomation

    if(pSpiNandDrv->tSpinandInfo.u8_BL0PBA != 0)    //for bl uboot
    {
    bl_count = 0;
    //search bl location in MBOOT PARTITION
    for(u32_BlkIdx=u32_MBootBegin ; u32_BlkIdx<u32_MBootEnd ; u32_BlkIdx++)
    {
    u32_Err = MDrv_SPINAND_Read(u32_BlkIdx*pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt,
    pSpiNandDrv->pu8_pagebuf, pSpiNandDrv->pu8_sparebuf);
    if(u32_Err!=ERR_SPINAND_SUCCESS || pSpiNandDrv->pu8_sparebuf[0]!=0xFF)
    continue;

    if(!MDrv_SPINAND_CheckAll0xFF(pSpiNandDrv->pu8_pagebuf, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt))
    {
    if(bl_count == 0)
    ptSpinandInfo->u8_BL0PBA = pSpiNandDrv->tSpinandInfo.u8_BL0PBA = (U8)u32_BlkIdx;
    else if(bl_count == 1)
    {
    ptSpinandInfo->u8_BL1PBA = pSpiNandDrv->tSpinandInfo.u8_BL1PBA = (U8)u32_BlkIdx;
    bl_count ++;
    break;
    }
    bl_count ++;
    }
    }

    printk(KERN_NOTICE"BL0_PBA %X, BL1_PBA %X\n", pSpiNandDrv->tSpinandInfo.u8_BL0PBA, pSpiNandDrv->tSpinandInfo.u8_BL1PBA);
    if(bl_count != 2)
    {
    printk(KERN_NOTICE"WARNING: there is no two sboots in SPI NAND Flash, Please Reupgrade Sboot\n");
    return -1;
    }

    if(pSpiNandDrv->tSpinandInfo.u8_UBOOTPBA != 0)
    {
    bl_count = 0;
    for(/*u32_BlkIdx = u32_MBootBegin*/; u32_BlkIdx < u32_MBootEnd; u32_BlkIdx ++)
    {
    u32_Err = MDrv_SPINAND_Read(u32_BlkIdx*pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt,
    pSpiNandDrv->pu8_pagebuf, pSpiNandDrv->pu8_sparebuf);
    if(u32_Err!=ERR_SPINAND_SUCCESS || pSpiNandDrv->pu8_sparebuf[0]!=0xFF)
    continue;

    if(((U32 *)pSpiNandDrv->pu8_pagebuf)[0x7] == 0x0000B007)
    {
    if(bl_count == 1)
    {
    ptSpinandInfo->u8_UBOOTPBA = pSpiNandDrv->tSpinandInfo.u8_UBOOTPBA = (U8)u32_BlkIdx;
    bl_count ++;
    break;
    }
    bl_count ++;
    }
    }
    if(bl_count != 2)
    {
    printk(KERN_NOTICE"WARNING: there is no two Mboots in SPI NAND Flash, Please Reupgrade Mboot\n");
    return -1;
    }

    printk(KERN_NOTICE"UBOOTPBA %X\n", pSpiNandDrv->tSpinandInfo.u8_UBOOTPBA);
    }
    }
    else if(pSpiNandDrv->tSpinandInfo.u8_HashPBA[0][0] != 0)    //for hash
    {
    bl_count = 0;
    //search bl location in MBOOT PARTITION
    for(u32_BlkIdx = u32_MBootBegin; u32_BlkIdx < u32_MBootEnd; u32_BlkIdx ++)
    {
    u32_Err = MDrv_SPINAND_Read(u32_BlkIdx*pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt,
    pSpiNandDrv->pu8_pagebuf, pSpiNandDrv->pu8_sparebuf);
    if(u32_Err!=ERR_SPINAND_SUCCESS || pSpiNandDrv->pu8_sparebuf[0]!=0xFF)
    continue;
    if(!MDrv_SPINAND_CheckAll0xFF(pSpiNandDrv->pu8_pagebuf, pSpiNandDrv->tSpinandInfo.u16_PageByteCnt))
    {
    ptSpinandInfo->u8_HashPBA[bl_count>>1][bl_count&1] = pSpiNandDrv->tSpinandInfo.u8_HashPBA[bl_count>>1][bl_count&1] = (U8)u32_BlkIdx;
    if(++bl_count == 4)
    break;
    }
    }

    printk(KERN_NOTICE"HASH00_PBA %X, HASH01_PBA %X\n", pSpiNandDrv->tSpinandInfo.u8_HashPBA[0][0], gtSpiNandDrv.tSpinandInfo.u8_HashPBA[0][1]);
    printk(KERN_NOTICE"HASH10_PBA %X, HASH11_PBA %X\n", pSpiNandDrv->tSpinandInfo.u8_HashPBA[1][0], gtSpiNandDrv.tSpinandInfo.u8_HashPBA[1][1]);
    if(bl_count != 4)
    {
    printk(KERN_NOTICE"WARNING: there is no two sboots in SPI NAND Flash, Please Reupgrade Sboot\n");
    return -1;
    }

    bl_count = 0;
    for(u32_BlkIdx = pSpiNandDrv->tSpinandInfo.u8_HashPBA[1][1]+1; u32_BlkIdx < u32_MBootEnd; u32_BlkIdx ++)
    {
    u32_Err = MDrv_SPINAND_Read(u32_BlkIdx*pSpiNandDrv->tSpinandInfo.u16_BlkPageCnt,
    pSpiNandDrv->pu8_pagebuf, pSpiNandDrv->pu8_sparebuf);
    if(u32_Err!=ERR_SPINAND_SUCCESS || pSpiNandDrv->pu8_sparebuf[0]!=0xFF)
    continue;

    if(((U32 *)pSpiNandDrv->pu8_pagebuf)[0x7] == 0x0000B007)
    {
    ptSpinandInfo->u8_HashPBA[2][bl_count] = pSpiNandDrv->tSpinandInfo.u8_HashPBA[2][bl_count] = (U8)u32_BlkIdx;
    if(++bl_count == 2)
    break;
    }
    }
    if(bl_count != 2)
    {
    printk(KERN_NOTICE"WARNING: there is no two Mboots in SPI NAND Flash, Please Reupgrade Mboot\n");
    return -1;
    }

    printk(KERN_NOTICE"HASH20_PBA %X, HASH21_PBA %X\n", pSpiNandDrv->tSpinandInfo.u8_HashPBA[2][0], gtSpiNandDrv.tSpinandInfo.u8_HashPBA[2][1]);
    }
    }

    u32_Err = MDrv_SPINAND_WriteCIS_for_ROM(pSpiNandInfoTag);

    free(pSpiNandInfoTag);

    return u32_Err;
}
#endif


#if 0
U32 MDrv_SPINAND_WriteBlkByteOffset(U8 *pu8_DestAddr, U16 u16_PBA, U32 u32_StartByte, U32 u32_ByteCnt)
{
#if 1
    return ERR_SPINAND_W_FAIL;
#else
    NAND_DRIVER *pNandDrv = drvNAND_get_DrvContext_address();
    U8 *au8_PageBuf = (U8*)pNandDrv->PlatCtx_t.pu8_PageDataBuf;
    U8 *au8_SpareBuf = (U8*)pNandDrv->PlatCtx_t.pu8_PageSpareBuf;
    U32 u32_Err;
    //MBBS_INFO_t* SPAREINFO =(MBBS_INFO_t*) pNandDrv->PlatCtx_t.pu8_PageSpareBuf;
    U32 u32_SrcPageIdx, u32_BakPageIdx;
    U16 u16_PageCnt;
    U16 u16_ByteCntRead;
    U16 u16_ByteIdxInPage;
    U16 u16_i;
    U16 u16_LastWrittenPage = 0;
    U16 u16_BakBlkIdx;
    U8 u8_GoodBlkCnt;
    U16 u16_BlkPageCnt;
    U16 *pu16_BlkPBA = (U16 *)(pNandDrv->PlatCtx_t.pu8_PageSpareBuf + pNandDrv->u16_SectorSpareByteCnt);

    #if defined(FCIE_LFSR) && FCIE_LFSR
    if(pNandDrv->u8_RequireRandomizer)
    {
    NC_DisableLFSR();
    }
    #endif

    u16_BlkPageCnt = pNandDrv->u16_BlkPageCnt;
    if(pNandDrv->u8_CellType == 1)  // MLC
    u16_BlkPageCnt = u16_BlkPageCnt>>1;

    /* sector size from AP is 512 Bytes, not NAND sector size */
    nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1, "BlkIdx:%d, BlkNo:%d, StartByte:0x%X, ByteCnt:0x%X\n", (int)u16_PBA, (int)u16_LBA, (int)u32_StartByte, (int)u32_ByteCnt);

    /* Find physical Backup BlkIdx in CIS remain block */
    u16_BakBlkIdx = 9;
    u8_GoodBlkCnt = 0;
    while(1)
    {
    if(!drvNAND_IsGoodBlk(u16_BakBlkIdx))
    {
    nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "Skip bad blk: 0x%04x\n", u16_BakBlkIdx);
    }
    else
    {
    if(++u8_GoodBlkCnt == 1)
    break;
    }

    if((--u16_BakBlkIdx) < 5)
    {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "too many bad block\n");
    return UNFD_ST_ERR_LACK_BLK;
    }
    }
    nand_debug(UNFD_DEBUG_LEVEL_HIGH, 1, "u16_BakBlkIdx:%d\n", u16_BakBlkIdx);

    //nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "erase u16_BakBlkIdx:%d\n", u16_BakBlkIdx);

    // erase bak block
    u32_Err = drvNAND_ErasePhyBlk(u16_BakBlkIdx);
    if (u32_Err != UNFD_ST_SUCCESS) {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Erase Blk 0x%04x failed with EC: 0x%08lx\n",
       u16_BakBlkIdx, u32_Err);
    drvNAND_MarkBadBlk(u16_BakBlkIdx);
    return u32_Err;
    }

    //copy data from src block to backup block before the startbyte
    for(u16_i=0 ; u16_i<(u32_StartByte>>pNandDrv->u8_PageByteCntBits) ; u16_i++)
    {
    u32_SrcPageIdx = (u16_PBA << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u16_i].u16_LSB;

    //nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "copy data before start byte read  page :0x%X\n", u32_SrcPageIdx);
    u32_Err = NC_ReadPages(u32_SrcPageIdx, au8_PageBuf, au8_SpareBuf, 1);
    if (u32_Err != UNFD_ST_SUCCESS)
    {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Reading page 0x%04lx failed with EC: 0x%04lx\n",
       u32_SrcPageIdx, u32_Err);

    return u32_Err;
    }
//      SPAREINFO->u8_BadBlkMarker = 0xFF;
//      SPAREINFO->u16_BackupPBA = u16_PBA;
//      au8_SpareBuf[3] = 0x36;
//      au8_SpareBuf[4] = 0x97;

    au8_SpareBuf[0] = 0xFF;
    *pu16_BlkPBA = u16_PBA;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 2] = 0x36;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 2 + 1] = 0x97;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 3] = 0xA1;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 3 + 1] = 1;
    u32_BakPageIdx = (u16_BakBlkIdx << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u16_i].u16_LSB;
    //nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "write  page :0x%X\n", u32_BakPageIdx);

    u32_Err = NC_WritePages(u32_BakPageIdx, au8_PageBuf, au8_SpareBuf, 1);
    if (u32_Err != UNFD_ST_SUCCESS)
    {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Writing page 0x%04lx failed with EC: 0x%04lx\n",
       u32_BakPageIdx, u32_Err);
    drvNAND_MarkBadBlk(u16_BakBlkIdx);
    return u32_Err;
    }
    memset(au8_PageBuf, 0xFF, pNandDrv->u16_PageByteCnt);

    u32_Err = drvNAND_WriteDummyToMSBPage(u16_i, (u16_BakBlkIdx << pNandDrv->u8_BlkPageCntBits), au8_PageBuf, au8_SpareBuf, NULL);
    if(u32_Err != UNFD_ST_SUCCESS)
    {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Writing page 0x%04lx failed with EC: 0x%04lx\n",
       u32_BakPageIdx, u32_Err);
    return u32_Err;
    }
    u16_LastWrittenPage = u16_i;
    }

    /* Read unaligned Byte first */
    if( u32_StartByte & pNandDrv->u16_PageByteCntMask)
    {
    u32_SrcPageIdx = (u16_PBA << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[(u32_StartByte>>pNandDrv->u8_PageByteCntBits)].u16_LSB;

    //nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "unaligned byte first read  page :0x%X\n", u32_SrcPageIdx);
    u32_Err = NC_ReadPages(u32_SrcPageIdx, au8_PageBuf, au8_SpareBuf, 1);
    if (u32_Err != UNFD_ST_SUCCESS) {

    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Reading page 0x%04lx failed with EC: 0x%04lx\n",
       u32_SrcPageIdx, u32_Err);

    return u32_Err;
    }
    u16_ByteIdxInPage = u32_StartByte & pNandDrv->u16_PageByteCntMask;
    u16_ByteCntRead = (u32_ByteCnt > ((U32)pNandDrv->u16_PageByteCnt-(U32)u16_ByteIdxInPage)) ? ((U32)pNandDrv->u16_PageByteCnt-(U32)u16_ByteIdxInPage) : (u32_ByteCnt);

    memcpy((au8_PageBuf+u16_ByteIdxInPage), pu8_DestAddr, u16_ByteCntRead);
    u32_BakPageIdx = (u16_BakBlkIdx << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[(u32_StartByte>>pNandDrv->u8_PageByteCntBits)].u16_LSB;

//      SPAREINFO->u8_BadBlkMarker = 0xFF;
//      SPAREINFO->u16_BackupPBA = u16_PBA;
//      au8_SpareBuf[3] = 0x36;
//      au8_SpareBuf[4] = 0x97;

    au8_SpareBuf[0] = 0xFF;
    *pu16_BlkPBA = u16_PBA;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 2] = 0x36;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 2 + 1] = 0x97;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 3] = 0xA1;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 3 + 1] = 1;
    //nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "write  page :0x%X\n", u32_BakPageIdx);
    u32_Err = NC_WritePages(u32_BakPageIdx, au8_PageBuf, au8_SpareBuf, 1);
    if (u32_Err != UNFD_ST_SUCCESS) {

    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Writing page 0x%04lx failed with EC: 0x%04lx\n",
       u32_BakPageIdx, u32_Err);
    drvNAND_MarkBadBlk(u16_BakBlkIdx);
    return u32_Err;
    }

    memset(au8_PageBuf, 0xFF, pNandDrv->u16_PageByteCnt);
    u32_Err = drvNAND_WriteDummyToMSBPage((u32_StartByte>>pNandDrv->u8_PageByteCntBits), (u16_BakBlkIdx << pNandDrv->u8_BlkPageCntBits), au8_PageBuf, au8_SpareBuf, NULL);
    if(u32_Err != UNFD_ST_SUCCESS)
    {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Writing page 0x%04lx failed with EC: 0x%04lx\n",
       u32_BakPageIdx, u32_Err);
    return u32_Err;
    }

    pu8_DestAddr += u16_ByteCntRead;
    u32_StartByte += u16_ByteCntRead;
    u32_ByteCnt -= u16_ByteCntRead;
    u16_LastWrittenPage = (u32_StartByte>>pNandDrv->u8_PageByteCntBits);
    }

    /* Read aligned sectors then */
//  SPAREINFO->u8_BadBlkMarker = 0xFF;
//  SPAREINFO->u16_BackupPBA = u16_PBA;
//  au8_SpareBuf[3] = 0x36;
//  au8_SpareBuf[4] = 0x97;

    au8_SpareBuf[0] = 0xFF;
    *pu16_BlkPBA = u16_PBA;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 2] = 0x36;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 2 + 1] = 0x97;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 3] = 0xA1;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 3 + 1] = 1;

    u32_BakPageIdx = (u16_BakBlkIdx << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[(u32_StartByte>>pNandDrv->u8_PageByteCntBits)].u16_LSB;
    u16_PageCnt = (u32_ByteCnt>>pNandDrv->u8_PageByteCntBits);
    while (u16_PageCnt)
    {

    //nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "aligned write page :0x%X\n", u32_BakPageIdx);
    u32_Err = NC_WritePages(u32_BakPageIdx, pu8_DestAddr, au8_SpareBuf, 1);
    if (u32_Err != UNFD_ST_SUCCESS) {

    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Writing page 0x%04lx failed with EC: 0x%04lx\n",
       u32_BakPageIdx, u32_Err);
    drvNAND_MarkBadBlk(u16_BakBlkIdx);
    return u32_Err;
    }

    memset(au8_PageBuf, 0xFF, pNandDrv->u16_PageByteCnt);
    u32_Err = drvNAND_WriteDummyToMSBPage((u32_StartByte>>pNandDrv->u8_PageByteCntBits), (u16_BakBlkIdx << pNandDrv->u8_BlkPageCntBits), au8_PageBuf, au8_SpareBuf, NULL);
    if(u32_Err != UNFD_ST_SUCCESS)
    {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Writing page 0x%04lx failed with EC: 0x%04lx\n",
       u32_BakPageIdx, u32_Err);
    return u32_Err;
    }

    u16_LastWrittenPage = u32_BakPageIdx -(u16_BakBlkIdx << pNandDrv->u8_BlkPageCntBits);
    pu8_DestAddr += pNandDrv->u16_PageByteCnt;
    u16_PageCnt--;
    u32_StartByte += pNandDrv->u16_PageByteCnt;
    u32_BakPageIdx = (u16_BakBlkIdx << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[(u32_StartByte>>pNandDrv->u8_PageByteCntBits)].u16_LSB;
    u32_ByteCnt -= pNandDrv->u16_PageByteCnt;
    }

    /* Read remaining unaligned sectors finally */
    if( u32_ByteCnt)
    {
    u32_SrcPageIdx = (u16_PBA << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[(u32_StartByte>>pNandDrv->u8_PageByteCntBits)].u16_LSB;
    //nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "remaining unaligned read  page :0x%X\n", u32_SrcPageIdx);

    u32_Err = NC_ReadPages(u32_SrcPageIdx, au8_PageBuf, au8_SpareBuf, 1);
    if (u32_Err != UNFD_ST_SUCCESS) {

    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Reading page 0x%04lx failed with EC: 0x%04lx\n",
       u32_SrcPageIdx, u32_Err);

    return u32_Err;
    }

//      SPAREINFO->u8_BadBlkMarker = 0xFF;
//      SPAREINFO->u16_BackupPBA = u16_PBA;
//      au8_SpareBuf[3] = 0x36;
//      au8_SpareBuf[4] = 0x97;

    au8_SpareBuf[0] = 0xFF;
    *pu16_BlkPBA = u16_PBA;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 2] = 0x36;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 2 + 1] = 0x97;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 3] = 0xA1;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 3 + 1] = 1;

    memcpy(au8_PageBuf, pu8_DestAddr, u32_ByteCnt);
    u32_BakPageIdx = (u16_BakBlkIdx << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[(u32_StartByte>>pNandDrv->u8_PageByteCntBits)].u16_LSB;
    //nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "write  page :0x%X\n", u32_BakPageIdx);

    u32_Err = NC_WritePages(u32_BakPageIdx, au8_PageBuf, au8_SpareBuf, 1);
    if (u32_Err != UNFD_ST_SUCCESS) {

    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Writing page 0x%04lx failed with EC: 0x%04lx\n",
       u32_BakPageIdx, u32_Err);
    drvNAND_MarkBadBlk(u16_BakBlkIdx);
    return u32_Err;
    }

    memset(au8_PageBuf, 0xFF, pNandDrv->u16_PageByteCnt);
    u32_Err = drvNAND_WriteDummyToMSBPage((u32_StartByte>>pNandDrv->u8_PageByteCntBits), (u16_BakBlkIdx << pNandDrv->u8_BlkPageCntBits), au8_PageBuf, au8_SpareBuf, NULL);
    if(u32_Err != UNFD_ST_SUCCESS)
    {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Writing page 0x%04lx failed with EC: 0x%04lx\n",
       u32_BakPageIdx, u32_Err);
    return u32_Err;
    }

    u16_LastWrittenPage = (u32_StartByte>>pNandDrv->u8_PageByteCntBits);
    }

    //copy remain data from PBA to Backup block
    for(u16_i=(u16_LastWrittenPage+1) ; u16_i<u16_BlkPageCnt; u16_i++)
    {
    u32_SrcPageIdx = (u16_PBA << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u16_i].u16_LSB;
    //nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "remain data read page :0x%X\n", u32_SrcPageIdx);

    u32_Err = NC_ReadPages(u32_SrcPageIdx, au8_PageBuf, au8_SpareBuf, 1);
    if (u32_Err != UNFD_ST_SUCCESS)
    {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Reading page 0x%04lx failed with EC: 0x%04lx\n",
       u32_SrcPageIdx, u32_Err);

    return u32_Err;
    }
//      SPAREINFO->u8_BadBlkMarker = 0xFF;
//      SPAREINFO->u16_BackupPBA = u16_PBA;
//      au8_SpareBuf[3] = 0x36;
//      au8_SpareBuf[4] = 0x97;

    au8_SpareBuf[0] = 0xFF;
    *pu16_BlkPBA = u16_PBA;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 2] = 0x36;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 2 + 1] = 0x97;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 3] = 0xA1;
    au8_SpareBuf[pNandDrv->u16_SectorSpareByteCnt * 3 + 1] = 1;

    u32_BakPageIdx = (u16_BakBlkIdx << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u16_i].u16_LSB;

    //nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "write  page :0x%X\n", u32_BakPageIdx);
    u32_Err = NC_WritePages(u32_BakPageIdx, au8_PageBuf, au8_SpareBuf, 1);
    if (u32_Err != UNFD_ST_SUCCESS)
    {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Writing page 0x%04lx failed with EC: 0x%04lx\n",
       u32_BakPageIdx, u32_Err);
    drvNAND_MarkBadBlk(u16_BakBlkIdx);
    return u32_Err;
    }

    memset(au8_PageBuf, 0xFF, pNandDrv->u16_PageByteCnt);
    u32_Err = drvNAND_WriteDummyToMSBPage(u16_i, (u16_BakBlkIdx << pNandDrv->u8_BlkPageCntBits), au8_PageBuf, au8_SpareBuf, NULL);
    if(u32_Err != UNFD_ST_SUCCESS)
    {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Writing page 0x%04lx failed with EC: 0x%04lx\n",
       u32_BakPageIdx, u32_Err);
    return u32_Err;
    }

    }

    // erase src block

    //nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "erase src block :0x%X\n", u16_PBA);
    u32_Err = drvNAND_ErasePhyBlk(u16_PBA);
    if (u32_Err != UNFD_ST_SUCCESS) {
        nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Erase Blk 0x%04x failed with EC: 0x%08x\n",
            (unsigned int)u16_PBA, (unsigned int)u32_Err);
    drvNAND_MarkBadBlk(u16_PBA);
    return u32_Err;
    }

    // copy bak to src
    for(u16_i=0 ; u16_i<u16_BlkPageCnt; u16_i++)
    {
    u32_BakPageIdx = (u16_BakBlkIdx << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u16_i].u16_LSB;
    //nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "read page :0x%X\n", u32_BakPageIdx);

    u32_Err = NC_ReadPages(u32_BakPageIdx, au8_PageBuf, au8_SpareBuf, 1);
    if (u32_Err != UNFD_ST_SUCCESS)
    {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Reading page 0x%04lx failed with EC: 0x%04lx\n",
       u32_BakPageIdx, u32_Err);

    return u32_Err;
    }

    u32_SrcPageIdx = (u16_PBA << pNandDrv->u8_BlkPageCntBits) + ga_tPairedPageMap[u16_i].u16_LSB;
    //nand_debug(UNFD_DEBUG_LEVEL_WARNING, 1, "write  page :0x%X\n", u32_SrcPageIdx);

    memset(au8_SpareBuf, 0xFF, pNandDrv->u16_SpareByteCnt);
    u32_Err = NC_WritePages(u32_SrcPageIdx, au8_PageBuf, au8_SpareBuf, 1);
    if (u32_Err != UNFD_ST_SUCCESS)
    {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Reading page 0x%04lx failed with EC: 0x%04lx\n",
       u32_SrcPageIdx, u32_Err);
    drvNAND_MarkBadBlk(u16_PBA);
    return u32_Err;
    }

    memset(au8_PageBuf, 0xFF, pNandDrv->u16_PageByteCnt);
    u32_Err = drvNAND_WriteDummyToMSBPage(u16_i, (u16_PBA << pNandDrv->u8_BlkPageCntBits), au8_PageBuf, au8_SpareBuf, NULL);
    if(u32_Err != UNFD_ST_SUCCESS)
    {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Writing page 0x%04lx failed with EC: 0x%04lx\n",
       u32_BakPageIdx, u32_Err);
    return u32_Err;
    }
    }

    // erase bak block
    u32_Err = drvNAND_ErasePhyBlk(u16_BakBlkIdx);
    if (u32_Err != UNFD_ST_SUCCESS) {
    nand_debug(UNFD_DEBUG_LEVEL_ERROR, 1, "Erase Blk 0x%04x failed with EC: 0x%08lx\n",
       u16_BakBlkIdx, u32_Err);
    drvNAND_MarkBadBlk(u16_BakBlkIdx);
    return u32_Err;
    }

    return UNFD_ST_SUCCESS;
#endif
}
#if 0
U32 MDrv_SPINAND_ReadBlkByteOffset(U8 *pu8_DestAddr, U16 u16_PBA, U32 u32_StartByte, U32 u32_ByteCnt)
{
    SPI_NAND_DRIVER_t *pSpiNandDrv = (SPI_NAND_DRIVER_t*)drvSPINAND_get_DrvContext_address();
    U8 *au8_PageBuf = (U8*)pSpiNandDrv->pu8_pagebuf;
    U8 *au8_SpareBuf = (U8*)pSpiNandDrv->pu8_sparebuf;
    U32 u32_Err;
    U32 u32_PageIdx;
    U16 u16_PageCnt;
    U16 u16_ByteCntRead;
    U16 u16_ByteInPage;

    /* Read unaligned sectors first */
    if( u32_StartByte & (pSpiNandDrv->tSpinandInfo.u16_PageByteCnt-1))
    {
    u16_ByteInPage = u32_StartByte & (pSpiNandDrv->tSpinandInfo.u16_PageByteCnt-1);
    u16_ByteCntRead = u32_ByteCnt > ((U32)pSpiNandDrv->tSpinandInfo.u16_PageByteCnt-(U32)u16_ByteInPage) ? ((U32)gtSpiNandDrv.tSpinandInfo.u16_PageByteCnt-(U32)u16_ByteInPage) : (u32_ByteCnt);
    u32_PageIdx = (u16_PBA << pSpiNandDrv->u8_BlkPageCntBits) + (u32_StartByte >> pSpiNandDrv->u8_PageByteCntBits);

        u32_Err = MDrv_SPINAND_Read(u32_PageIdx, au8_PageBuf, au8_SpareBuf);
    if (u32_Err != ERR_SPINAND_SUCCESS) {

            spi_nand_err("Reading page 0x%04x failed with EC: 0x%04x\n",
                    (unsigned int)u32_PageIdx, (unsigned int)u32_Err);
    return u32_Err;
    }

    memcpy(pu8_DestAddr, au8_PageBuf + u16_ByteInPage, u16_ByteCntRead);
    pu8_DestAddr += u16_ByteCntRead;
    u32_StartByte += u16_ByteCntRead;
    u32_ByteCnt -= u16_ByteCntRead;
    }

    /* Read aligned sectors then */
    u32_PageIdx = (u16_PBA << pSpiNandDrv->u8_BlkPageCntBits) + (u32_StartByte >> pSpiNandDrv->u8_PageByteCntBits);
    u16_PageCnt = (u32_ByteCnt >> pSpiNandDrv->u8_PageByteCntBits);

    while (u16_PageCnt)
    {
        u32_Err = MDrv_SPINAND_Read(u32_PageIdx, pu8_DestAddr, au8_SpareBuf);
    if (u32_Err != ERR_SPINAND_SUCCESS) {

            spi_nand_err("Reading page 0x%04x failed with EC: 0x%04x\n",
            (unsigned int)u32_PageIdx, (unsigned int)u32_Err);
    return u32_Err;
    }

    pu8_DestAddr += pSpiNandDrv->tSpinandInfo.u16_PageByteCnt;
    u16_PageCnt--;
    u32_StartByte += pSpiNandDrv->tSpinandInfo.u16_PageByteCnt;
    u32_ByteCnt -= pSpiNandDrv->tSpinandInfo.u16_PageByteCnt;
    u32_PageIdx = (u16_PBA << pSpiNandDrv->u8_BlkPageCntBits) + (u32_StartByte >> pSpiNandDrv->u8_PageByteCntBits);

    }

    /* Read remaining unaligned sectors finally */
    if( u32_ByteCnt )
    {
    u32_PageIdx = (u16_PBA << pSpiNandDrv->u8_BlkPageCntBits) +  (u32_StartByte >> pSpiNandDrv->u8_PageByteCntBits);
        u32_Err = MDrv_SPINAND_Read(u32_PageIdx, au8_PageBuf, au8_SpareBuf);
    if (u32_Err != ERR_SPINAND_SUCCESS) {

            spi_nand_err("Reading page 0x%04x failed with EC: 0x%04x\n",
                        (unsigned int)u32_PageIdx, (unsigned int)u32_Err);
    return u32_Err;
    }
    memcpy(pu8_DestAddr, au8_PageBuf, u32_ByteCnt);
    }
    return ERR_SPINAND_SUCCESS;
}
#endif
#endif
