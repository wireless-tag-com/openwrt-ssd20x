/*
* eMMC_utl.c- Sigmastar
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
#include "eMMC.h"
#if defined(UNIFIED_eMMC_DRIVER) && UNIFIED_eMMC_DRIVER

//--------------------------------------------------------------------------
static eMMC_ALIGN0 eMMC_TEST_ALIGN_PACK_t g_eMMCTestAlignPack_t eMMC_ALIGN1;
U32 eMMC_CheckAlignPack(U8 u8_AlignByteCnt)
{
    // check alignment
    if((uintptr_t)&(g_eMMCTestAlignPack_t.u8_0) & (u8_AlignByteCnt-1))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: not aliged. expect %Xh but %lXh \n",
            u8_AlignByteCnt, (uintptr_t)&(g_eMMCTestAlignPack_t.u8_0));
        eMMC_die("\n");
        return eMMC_ST_ERR_NOT_ALIGN;
    }
    // check packed - 0
    if((uintptr_t)&(g_eMMCTestAlignPack_t.u16_0)-(uintptr_t)&(g_eMMCTestAlignPack_t.u8_0) != 1 ||
       (uintptr_t)&(g_eMMCTestAlignPack_t.u32_0)-(uintptr_t)&(g_eMMCTestAlignPack_t.u8_0) != 3 ||
       (uintptr_t)&(g_eMMCTestAlignPack_t.u32_1)-(uintptr_t)&(g_eMMCTestAlignPack_t.u8_0) != 7)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: not packed. check err.0 \n");
        eMMC_die("\n");
        return eMMC_ST_ERR_NOT_PACKED;
    }
    // check packed - 1
    if((uintptr_t)&(g_eMMCTestAlignPack_t.u16_0)-(uintptr_t)&(g_eMMCTestAlignPack_t.u8_0) != 1 ||
       (uintptr_t)&(g_eMMCTestAlignPack_t.u32_0)-(uintptr_t)&(g_eMMCTestAlignPack_t.u16_0)!= 2 ||
       (uintptr_t)&(g_eMMCTestAlignPack_t.u32_1)-(uintptr_t)&(g_eMMCTestAlignPack_t.u32_0)!= 4)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: not packed. check err.1 \n");
        eMMC_die("\n");
        return eMMC_ST_ERR_NOT_PACKED;
    }

    //eMMC_debug(eMMC_DEBUG_LEVEL_HIGH, 1, "ok\n");
    return eMMC_ST_SUCCESS;
}


static __inline void dump_mem_line(unsigned char *buf, U32 cnt)
{
    U32 i;

    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0, " 0x%08lX: ", (uintptr_t)buf);
    for (i= 0; i < cnt; i++)
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0, "%02Xh ", buf[i]);

    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0, " | ");

    for (i = 0; i < cnt; i++)
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0, "%c", (buf[i] >= 32 && buf[i] < 128) ? buf[i] : '.');

    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0, "\n");
}

void eMMC_dump_mem(unsigned char *buf, U32 cnt)
{
    U32 i;

    for (i= 0; i < cnt; i+= 16)
        dump_mem_line(buf + i, 16);
}

static __inline void dump_mem_line_32(U32 *buf, int cnt)
{
    U32 i;

    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0, " 0x%08lX: ", (uintptr_t)buf);
    for (i= 0; i < cnt; i++)
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0, "%08Xh ", buf[i]);
}

void eMMC_dump_mem_32(U32 *buf, U32 cnt)
{
    U32 i;

    for (i= 0; i < cnt; i+= 8)
    {
        dump_mem_line_32(buf + i, 8);

        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0, " | 0x%08X \n", i+7);
    }
}



U32 eMMC_ComapreData(U8 *pu8_Buf0, U8 *pu8_Buf1, U32 u32_ByteCnt)
{
    U32 u32_i, u32_offset;

    for(u32_i=0; u32_i<u32_ByteCnt; u32_i++)
    {
        if(pu8_Buf0[u32_i] != pu8_Buf1[u32_i])
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: data mismatch: i:[%Xh], Buf0: %02Xh, Buf1: %02Xh\n",
                u32_i, pu8_Buf0[u32_i], pu8_Buf1[u32_i]);
            break;
        }
    }

    if(u32_i != u32_ByteCnt && 0==(g_eMMCDrv.u32_DrvFlag&DRV_FLAG_TUNING_TTABLE))
    {
        u32_offset = (u32_i>>eMMC_SECTOR_512BYTE_BITS)<<eMMC_SECTOR_512BYTE_BITS;

        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\ndump Buf0, from %Xh-th bytes\n", u32_offset);
        eMMC_dump_mem(pu8_Buf0+u32_offset, eMMC_SECTOR_512BYTE);
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"dump Buf1, from %Xh-th bytes\n", u32_offset);
        eMMC_dump_mem(pu8_Buf1+u32_offset, eMMC_SECTOR_512BYTE);
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\n");
        return eMMC_ST_ERR_DATA_MISMATCH;
    }

    if(u32_i != u32_ByteCnt)
        return eMMC_ST_ERR_DATA_MISMATCH;

    return eMMC_ST_SUCCESS;
}

U32 eMMC_ChkSum(U8 *pu8_Data, U32 u32_ByteCnt)
{
    volatile U16 u16_Tmp;
    volatile U32 u32_Sum=0;
    for (u16_Tmp=0; u16_Tmp < u32_ByteCnt; u16_Tmp++)
       u32_Sum += pu8_Data[u16_Tmp];
    return (u32_Sum);
}


/* return 0: same, 1: different */
U32 eMMC_CompareCISTag(U8 *tag)
{
    const char *str = "MSTARSEMIUNFDCIS";
    int i = 0;

    for (i = 0; i < 16; i++) {
        if (tag[i] != str[i])
            return 1;
    }

    return 0;
}


U32 eMMC_PrintDeviceInfo(void)
{
    U32 u32_err;
    U8 au8_buf[eMMC_SECTOR_512BYTE];

    u32_err = eMMC_CheckIfReady();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: eMMC_Init_Ex fail: %Xh\n", u32_err);
        return u32_err;
    }

    // ------------------------------
    u32_err = eMMC_CMD8(au8_buf);
    if(eMMC_ST_SUCCESS != u32_err)
        return u32_err;
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\nExtCSD:\n");
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "-------------------------------\n");
    //eMMC_dump_mem(au8_buf, eMMC_SECTOR_512BYTE);

    switch(g_eMMCDrv.u8_ECSD192_Ver)
    {
        case 7: eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC V.5.0\n"); break;
        case 6: eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC V.4.5\n"); break;
        case 5: eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC V.4.41\n"); break;
        default: eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC V.x, ECSD[192]:%u\n", g_eMMCDrv.u8_ECSD192_Ver);
    }
    eMMC_debug(eMMC_DEBUG_LEVEL,0,"Support: %s %s %s %s\n",
        (g_eMMCDrv.u8_ECSD196_DevType&BIT5) ? "HS200 1.2V" : "",
        (g_eMMCDrv.u8_ECSD196_DevType&BIT4) ? "HS200 1.8V" : "",
        (g_eMMCDrv.u8_ECSD196_DevType&BIT3) ? "DDR52 1.2V" : "",
        (g_eMMCDrv.u8_ECSD196_DevType&BIT2) ? "DDR52 1.8/3V" : "");

    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "SEC_COUNT: %Xh\n", g_eMMCDrv.u32_SEC_COUNT);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "Capacity: %u MB\n", g_eMMCDrv.u32_SEC_COUNT>>11);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "eMMC BUS_WIDTH: %Xh\n", g_eMMCDrv.u8_BUS_WIDTH);

    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "TRIM: %Xh\n",
        (g_eMMCDrv.u32_eMMCFlag&eMMC_FLAG_TRIM)?1:0);

    if(g_eMMCDrv.u32_eMMCFlag & (eMMC_FLAG_HPI_CMD12|eMMC_FLAG_HPI_CMD13))
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "HPI: CMD%u \n",
            (g_eMMCDrv.u32_eMMCFlag&eMMC_FLAG_HPI_CMD12)?12:13);
    else
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "HPI: 0 \n");

    eMMC_debug(eMMC_DEBUG_LEVEL,0,"Reliable Write BlkCnt: %Xh\n", g_eMMCDrv.u16_ReliableWBlkCnt);

    eMMC_debug(eMMC_DEBUG_LEVEL,0,"Power Off Notification: ECSD[34]:%Xh, Short: %u ms, Long: %u ms\n",
        g_eMMCDrv.u8_ECSD34_PwrOffCtrl,
        g_eMMCDrv.u8_ECSD248_CMD6TO*10,
        g_eMMCDrv.u8_ECSD247_PwrOffLongTO*10);

    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "-------------------------------\n\n");

    // ------------------------------
    #if 0
    u32_err = eMMC_CMD3_CMD7(0, 7);
        if(eMMC_ST_SUCCESS != u32_err)
            return u32_err;

    u32_err = eMMC_CMD9(g_eMMCDrv.u16_RCA);
    if(eMMC_ST_SUCCESS != u32_err)
        return u32_err;
    #endif

    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "CSD:\n");
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "-------------------------------\n");
    eMMC_dump_mem(&g_eMMCDrv.au8_CSD[0], eMMC_MAX_RSP_BYTE_CNT);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "SPEC_VERS: %Xh, R_BL_LEN: %Xh, W_BL_LEN: %Xh \n",
        g_eMMCDrv.u8_SPEC_VERS, g_eMMCDrv.u8_R_BL_LEN, g_eMMCDrv.u8_W_BL_LEN);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "Access Mode: %s\n", g_eMMCDrv.u8_IfSectorMode?"Sector":"Byte");
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "C_SIZE: %Xh\n", g_eMMCDrv.u16_C_SIZE);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "ERASE_GRP_SIZE: %Xh, ERASE_GRP_MULT: %Xh\n",
        g_eMMCDrv.u8_ERASE_GRP_SIZE, g_eMMCDrv.u8_ERASE_GRP_MULT);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "EraseUnitSize: %u\n", g_eMMCDrv.u32_EraseUnitSize);
    switch(g_eMMCDrv.u8_Tran_Speed)
    {
        case 0x2A:
            eMMC_debug(eMMC_DEBUG_LEVEL, 0, "default speed 20MHz\n"); break;
        case 0x32:
            eMMC_debug(eMMC_DEBUG_LEVEL, 0, "default speed 26MHz\n"); break;
        default:
            eMMC_debug(eMMC_DEBUG_LEVEL, 0, "normal TRAN_SPEED: %Xh\n", g_eMMCDrv.u8_Tran_Speed);
    }
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "TAAC: %Xh, NSAC: %Xh, R2W_FACTOR: %Xh\n",
        g_eMMCDrv.u8_TAAC, g_eMMCDrv.u8_NSAC, g_eMMCDrv.u8_R2W_FACTOR);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "-------------------------------\n\n");


    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "RCA: %Xh\n", g_eMMCDrv.u16_RCA);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "CID:\n");
    eMMC_dump_mem(&g_eMMCDrv.au8_CID[1], eMMC_MAX_RSP_BYTE_CNT-1);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "-------------------------------\n\n");

    #if 0
    u32_err = eMMC_CMD3_CMD7(g_eMMCDrv.u16_RCA, 7);
        if(eMMC_ST_SUCCESS != u32_err)
            return u32_err;
    #endif
    return u32_err;
}


void eMMC_dump_nni(eMMC_NNI_t *peMMCInfo)
{
#if eMMC_DEBUG_LEVEL <= eMMC_DEBUG_LEVEL_LOW
    int i;
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "###############################################\n");
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "#        eMMC NNI                             #\n");
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "###############################################\n");

    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "peMMCInfo: 0x%08lX\n", (uintptr_t)peMMCInfo);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "au8_Tag          : [");
    for (i = 0; i < 16; i++)
        eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 0, "%c", peMMCInfo->au8_Tag[i]);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 0, "]\n");

    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u8_IDByteCnt     : 0x%04x\n", peMMCInfo->u8_IDByteCnt);

    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "au8_ID           : 0x[ ");
    for (i = 0; i < peMMCInfo->u8_IDByteCnt; i++)
        eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 0, "%02X ", peMMCInfo->au8_ID[i]);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 0, "]\n");

    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u32_ChkSum       : 0x%04X\n", peMMCInfo->u32_ChkSum);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u16_SpareByteCnt : 0x%04x\n", peMMCInfo->u16_SpareByteCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u16_PageByteCnt  : 0x%04x\n", peMMCInfo->u16_PageByteCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u16_BlkPageCnt   : 0x%04x\n", peMMCInfo->u16_BlkPageCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u16_BlkCnt       : 0x%04x\n", peMMCInfo->u16_BlkCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u32_Config       : 0x%08X\n", peMMCInfo->u32_Config);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u16_ECCType      : 0x%04x\n", peMMCInfo->u16_ECCType);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u16_SeqAccessTime: 0x%04x\n", peMMCInfo->u16_SeqAccessTime);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u8_Vendor: %s\n", peMMCInfo->au8_Vendor);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u8_PartNumber: %s\n", peMMCInfo->au8_PartNumber);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u8_PairPageMapLoc: 0x%04x\n", peMMCInfo->u8_PairPageMapLoc);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u8_PairPageMapType: 0x%04x\n", peMMCInfo->u8_PairPageMapType);
#endif
}

void eMMC_dump_pni(eMMC_PNI_t *pPartInfo)
{
#if eMMC_DEBUG_LEVEL <= eMMC_DEBUG_LEVEL_LOW
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "###############################################\n");
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "#        eMMC PNI                             #\n");
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "###############################################\n");

    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "pPartInfo: 0x%08lX\n", (uintptr_t)pPartInfo);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u32_ChkSum      : 0x%04X\n", pPartInfo->u32_ChkSum);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u16_SpareByteCnt : 0x%04x\n", pPartInfo->u16_SpareByteCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u16_PageByteCnt : 0x%04x\n", pPartInfo->u16_PageByteCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u16_BlkPageCnt : 0x%04x\n", pPartInfo->u16_BlkPageCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u16_BlkCnt : 0x%04x\n", pPartInfo->u16_BlkCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u16_PartCnt     : 0x%04x\n", pPartInfo->u16_PartCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL_LOW, 1, "u16_UnitByteCnt : 0x%04x\n", pPartInfo->u16_UnitByteCnt);

    //dump_part_records(pPartInfo->records, pPartInfo->u16_PartCnt);
#endif
}

void eMMC_dump_WR_Count(void)
{
    #if defined(eMMC_PROFILE_WR) && eMMC_PROFILE_WR
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n");
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "CNT_CMD17: %u, CNT_CMD24: %u \n", g_eMMCDrv.u32_CNT_CMD17, g_eMMCDrv.u32_CNT_CMD24);
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "CNT_CMD18: %u, CNT_CMD25: %u \n", g_eMMCDrv.u32_CNT_CMD18, g_eMMCDrv.u32_CNT_CMD25);
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "TotalRBlk: %llu, TotalWBlk: %llu \n", g_eMMCDrv.u64_CNT_TotalRBlk, g_eMMCDrv.u64_CNT_TotalWBlk);
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "MinRBlk: %u, MinWBlk: %u \n", g_eMMCDrv.u32_CNT_MinRBlk, g_eMMCDrv.u32_CNT_MinWBlk);
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "MaxRBlk: %u, MaxWBlk: %u \n", g_eMMCDrv.u32_CNT_MaxRBlk, g_eMMCDrv.u32_CNT_MaxWBlk);
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "RHitCnt: %u, WHitCnt: %u \n", g_eMMCDrv.u32_Addr_RHitCnt, g_eMMCDrv.u32_Addr_WHitCnt);

    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "au32_CNT_MinRBlk: \n");
    eMMC_dump_mem_32(g_eMMCDrv.au32_CNT_MinRBlk, 0x200);
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "au32_CNT_MinWBlk: \n");
    eMMC_dump_mem_32(g_eMMCDrv.au32_CNT_MinWBlk, 0x200);

    #endif
}

#endif
