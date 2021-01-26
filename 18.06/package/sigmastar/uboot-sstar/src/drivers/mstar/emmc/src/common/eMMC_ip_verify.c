/*
* eMMC_ip_verify.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: joe.su <joe.su@sigmastar.com.tw>
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

#include "../../inc/common/eMMC.h"
#if defined (UNIFIED_eMMC_DRIVER) && UNIFIED_eMMC_DRIVER

#if defined(FCIE_ZDEC_TEST) && FCIE_ZDEC_TEST
#include "zdec_top_fpga.h"
#endif

//=============================================================

//=============================================================
#if defined(IF_IP_VERIFY) && IF_IP_VERIFY

#define TEST_BUFFER_SIZE    0x8000 // 1MB
//eMMC_ALIGN0 static U8 gau8_WBuf[TEST_BUFFER_SIZE] eMMC_ALIGN1;
//eMMC_ALIGN0 static U8 gau8_RBuf[TEST_BUFFER_SIZE] eMMC_ALIGN1;
static U8* gau8_WBuf;
static U8* gau8_RBuf;

#define eMMC_PATTERN_00000000    0x00000000 // smooth
#define eMMC_PATTERN_FFFFFFFF    0xFFFFFFFF // smooth
#define eMMC_PATTERN_000000FF    0x000000FF // board
#define eMMC_PATTERN_0000FFFF    0x0000FFFF // board
#define eMMC_PATTERN_00FF00FF    0x00FF00FF // board
#define eMMC_PATTERN_AA55AA55    0xAA55AA55 // board

U32  eMMCTest_SingleBlkWRC_RIU(U32 u32_eMMC_Addr, U32 u32_DataPattern);
U32  eMMCTest_SingleBlkWRC_MIU(U32 u32_eMMC_Addr, U32 u32_DataPattern);
U32  eMMCTest_SingleBlkWRC_MIU_Ex(U32 u32_eMMC_Addr, U8 *pu8_W, U8 *pu8_R);
U32  eMMCTest_SingleBlkWRC_SRAM_Ex(U32 u32_eMMC_Addr, U8 *pu8_W, U8 *pu8_R);
U32  eMMCTest_SgWRC_MIU(U32 u32_eMMC_Addr, U16 u16_BlkCnt, U32 u32_DataPattern);
U32  eMMCTest_SgW_MIU(U32 u32_eMMC_Addr);
U32  eMMCTest_SgR_MIU(U32 u32_eMMC_Addr);
U32  eMMCTest_MultiBlkWRC_MIU(U32 u32_eMMC_Addr, U16 u16_BlkCnt, U32 u32_DataPattern);
U32  eMMC_IPVerify_Main_Ex(U32 u32_DataPattern);
U32  eMMC_IPVerify_Main_Sg_Ex(U32 u32_DataPattern);

void eMMCTest_DownCount(U32 u32_Sec);

#define MAX_SCATTERLIST_COUNT 0x10

#define eMMC_GENERIC_WAIT_TIME  (HW_TIMER_DELAY_1s*10) // 10 sec

static struct _scatterlist {
    U32 u32_length;
    U32 u32_dma_address;

}pSG_st[MAX_SCATTERLIST_COUNT];

#if 1
//128 bit
eMMC_PACK0 struct  _AdmaDescriptor{
/*
    U32 u32_End : 1;
    U32 u32_MiuSel : 2;
    U32         : 13;
    U32 u32_JobCnt : 16;
*/
    U16 u16_Config;
    U16 u16_JobCnt;
    U32 u32_Address;
    U32 u32_DmaLen;
    U32 u32_Dummy;
}eMMC_PACK1 p_AdmaDesc_st[MAX_SCATTERLIST_COUNT];

struct _ZDecDescriptor
{
    U32 u32_End     : 1;
    U32 u32_MiuSel  : 2;
    U32             : 13;
    U32 u32_JobCnt  : 16;
    U32 u32_Address;
} eMMC_ALIGN0 p_ZDecDesc_st[MAX_SCATTERLIST_COUNT] eMMC_ALIGN1, \
  eMMC_ALIGN0 p_ZDecOutput_st[MAX_SCATTERLIST_COUNT] eMMC_ALIGN1;
#endif

#define DDR_TEST_BLK_CNT        8
#define DDR_TEST_BUFFER_SIZE    (eMMC_SECTOR_512BYTE*DDR_TEST_BLK_CNT)

eMMC_ALIGN0 static U8 gau8_WBuf_DDR[DDR_TEST_BUFFER_SIZE] eMMC_ALIGN1;
eMMC_ALIGN0 static U8 gau8_RBuf_DDR[DDR_TEST_BUFFER_SIZE] eMMC_ALIGN1;
#if 1
eMMC_ALIGN0 U8 gau8_BootImageOri[] eMMC_ALIGN1 ={
    #include "eMMC_loader.txt"
};
#endif

void eMMC_software_reset(U32 u32_us)
{
    eMMC_CMD0_RBoot(0xF0F0F0F0, 0, 0);
}

#if 1

U32 eMMC_LoadRbootImages(U32 u32_Addr, U32 u32_ByteCnt, U32 u32_HashStage)
{
    U32 u32_err = 0;

    eMMC_FCIE_ClearEvents();
    if( u32_HashStage == 0 )
    {
        eMMC_pads_switch(FCIE_eMMC_BYPASS);
        //REG_FCIE_SETBIT(FCIE_BOOT, BIT_NAND_BOOT_EN);
    }

    if( u32_HashStage == 0 )
    {
        eMMC_FCIE_Init();       // reset & init FCIE
        eMMC_software_reset(HW_TIMER_DELAY_1ms); // reset eMMC
    }


    // Start transfer
    if( u32_HashStage == 0 )
    {

        eMMC_hw_timer_delay(HW_TIMER_DELAY_1ms);

        u32_err = eMMC_CMD0_RBoot(0xFFFFFFFA, u32_Addr, u32_ByteCnt);

        if( u32_err )
        {
            goto LABEL_LOAD_IMAGE_END;
        }
    }
    else
    {
        REG_FCIE_W(FCIE_JOB_BL_CNT, u32_ByteCnt>>eMMC_SECTOR_512BYTE_BITS);
        REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, u32_Addr & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, u32_Addr >> 16);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, u32_ByteCnt & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, u32_ByteCnt >> 16);

        //if( u32_HashStage < 2 )  
        {
            REG_FCIE_SETBIT(FCIE_BOOT, BIT_BOOTSRAM_ACCESS_SEL);
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_BOOT_MODE_EN); // DMA to SRAM
        }
        #if 0
        else
        {
            REG_CLR_BITS_UINT16(FCIE_BOOT, BIT_BOOTSRAM_ACCESS_SEL);
            REG_CLR_BITS_UINT16(FCIE_BOOT_CONFIG, BIT_BOOT_MODE_EN); // DMA to DRAM
            REG_WRITE_UINT16(DEBUG_REG_2, 0xE240);
        }
        #endif
        REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN);
        REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_JOB_START);

        while(1)
        {
            if( (REG_FCIE(FCIE_MIE_EVENT) & BIT_DMA_END) == BIT_DMA_END )
                break;
        }


        REG_FCIE_W(FCIE_MIE_EVENT, REG_FCIE(FCIE_MIE_EVENT));

        //if( u32_HashStage < 2 )    
        {
            //REG_FCIE_CLRBIT(FCIE_BOOT, BIT_BOOTSRAM_ACCESS_SEL); //release SRAM to CPU
        }

    }


    // check status
    if( REG_FCIE(FCIE_SD_STATUS) & BIT_SD_FCIE_ERR_FLAGS )
    {
        eMMC_debug(0,1, "SD_STATUS Error\n");

        goto LABEL_LOAD_IMAGE_END;
    }

    #if 0
    if( u32_HashStage == 2 )
    {
        REG_WRITE_UINT16(DEBUG_REG_2, 0xE410);
        eMMC_CMD0(0, 0, 0); // boot end
    }
    #endif
    return eMMC_ST_SUCCESS;

    LABEL_LOAD_IMAGE_END:
    return u32_err;
}

U32 eMMCTest_BootMode(void)
{
    U8*  pu8_SramBufferAddr;
    U32 u32_Err;

    pu8_SramBufferAddr = (U8*)DMA_W_IMI_ADDR;

    memset((U8*)DMA_W_IMI_ADDR, 0, 0x200);
    ///*
    eMMC_debug(0, 1, "1s\r\n");
    eMMC_hw_timer_sleep(HW_TIMER_DELAY_1s);
        eMMC_debug(0, 1, "1s\r\n");
    eMMC_hw_timer_sleep(HW_TIMER_DELAY_1s);
        eMMC_debug(0, 1, "1s\r\n");
    eMMC_hw_timer_sleep(HW_TIMER_DELAY_1s);
    //*/

    eMMC_debug(0,1,"begin stage 0\r\n");

    u32_Err = eMMC_LoadRbootImages((U32)pu8_SramBufferAddr, 32* 1024, 0);
    if(u32_Err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,"stage 0 fail\r\n");
        return u32_Err;
    }
    eMMC_debug(0,1,"end stage 0\r\n");

    eMMC_dump_mem((U8*)DMA_W_IMI_ADDR, 0x200);

    return eMMC_ST_SUCCESS;
}
#endif
U32 eMMCTest_BlkWRC_ProbeTiming(U32 u32_eMMC_Addr)
{
    U32 u32_err=eMMC_ST_SUCCESS;
    U32 u32_i, u32_j, *pu32_W=(U32*)gau8_WBuf_DDR, *pu32_R=(U32*)gau8_RBuf_DDR;
    U32 u32_BlkCnt=eMMC_TEST_BLK_CNT, u32_BufByteCnt;

    u32_BlkCnt = u32_BlkCnt > DDR_TEST_BLK_CNT ? DDR_TEST_BLK_CNT : u32_BlkCnt;
    u32_BufByteCnt = u32_BlkCnt << eMMC_SECTOR_512BYTE_BITS;

    for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
        pu32_R[u32_i] = 0;

    for(u32_j=0; u32_j<9; u32_j++)
    {
        // init data pattern
        switch(u32_j)
        {
            case 0: // increase
                #if 0
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i] = u32_i+1;
                break;
                #else
                continue;
                #endif
            case 1: // decrease
                #if 0
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i] = 0-(u32_i+1); // more FF, more chance to lose start bit
                break;
                #else
                continue;
                #endif
            case 2: // 0xF00F
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0xF00FF00F;
                break;
            case 3: // 0xFF00
                #if 1
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0xFF00FF00;
                break;
                #else
                continue;
                #endif
            case 4: // 0x5AA5
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0x5AA55AA5;
                break;
            case 5: // 0x55AA
                #if 0
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0x55AA55AA;
                break;
                #else
                continue;
                #endif
            case 6: // 0x5A5A
                #if 0
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0x5A5A5A5A;
                break;
                #else
                continue;
                #endif
            case 7: // 0x0000
                #if 1
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0x00000000;
                break;
                #else
                continue;
                #endif
            case 8: // 0xFFFF
                #if 0
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0xFFFFFFFF;
                break;
                #else
                continue;
                #endif
        }

        u32_err = eMMC_CMD24_MIU(u32_eMMC_Addr, gau8_WBuf_DDR);
        if(eMMC_ST_SUCCESS != u32_err)
            break;

        u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr, gau8_RBuf_DDR);
        if(eMMC_ST_SUCCESS != u32_err)
            break;

        u32_err = eMMC_ComapreData(gau8_WBuf_DDR, gau8_RBuf_DDR, eMMC_SECTOR_512BYTE);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(0,1,"Err, compare fail.single, %Xh \r\n", u32_err);
            break;
        }

        u32_err = eMMC_CMD25_MIU(u32_eMMC_Addr, gau8_WBuf_DDR, u32_BlkCnt);
        if(eMMC_ST_SUCCESS != u32_err)
            break;

        u32_err = eMMC_CMD18_MIU(u32_eMMC_Addr, gau8_RBuf_DDR, u32_BlkCnt);
        if(eMMC_ST_SUCCESS != u32_err)
            break;

        u32_err = eMMC_ComapreData(gau8_WBuf_DDR, gau8_RBuf_DDR, u32_BufByteCnt);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(0,1,"Err, compare fail.multi %Xh \n", u32_err);
            break;
        }
    }

    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(1,1,"data pattern %u: %02X%02X%02X%02Xh \n\n",
            u32_j, gau8_WBuf_DDR[3], gau8_WBuf_DDR[2], gau8_WBuf_DDR[1], gau8_WBuf_DDR[0]);
    }

    return u32_err;
}

U32 eMMCTest_SingleBlkWRC_RIU(U32 u32_eMMC_Addr, U32 u32_DataPattern)
{
    U32 u32_err;
    U32 u32_i, *pu32_W=(U32*)gau8_WBuf, *pu32_R=(U32*)gau8_RBuf;

    // init data pattern
    #if defined(SEL_FCIE5) && (SEL_FCIE5)
    if(u32_DataPattern == 0x00112233 || (u32_DataPattern == 0x33221100))
    {
        for(u32_i = 0; u32_i < TEST_BUFFER_SIZE; u32_i ++)
        {
            if(u32_DataPattern == 0x00112233)
            {
                gau8_WBuf[u32_i] = u32_i & 0xFF;
            }
            else
            {
                gau8_WBuf[u32_i] = 0xFF - (u32_i & 0xFF);
            }
            gau8_RBuf[u32_i] = ~gau8_WBuf[u32_i];
        }
    }
    else
    {
        for(u32_i=0; u32_i<TEST_BUFFER_SIZE>>2; u32_i++)
        {
            pu32_W[u32_i] = u32_DataPattern;
            pu32_R[u32_i] = ~pu32_W[u32_i];
        }
    }
    //for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE>>2; u32_i++)
    //{
    //  pu32_W[u32_i] = u32_DataPattern;
    //  pu32_R[u32_i] = ~pu32_W[u32_i];
    //}

    #else
    for(u32_i=0; u32_i<FCIE_CIFD_BYTE_CNT>>2; u32_i++)
    {
        pu32_W[u32_i] = u32_DataPattern;
        pu32_R[u32_i] = ~pu32_W[u32_i];
    }
    #endif

    u32_err = eMMC_CMD24_CIFD(u32_eMMC_Addr, gau8_WBuf);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC W, %Xh \r\n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_CMD17_CIFD(u32_eMMC_Addr, gau8_RBuf);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC R, %Xh \r\n", u32_err);
        return u32_err;
    }
    #if defined(SEL_FCIE5) && (SEL_FCIE5)
    u32_err = eMMC_ComapreData(gau8_WBuf, gau8_RBuf, eMMC_SECTOR_512BYTE);
    #else
    u32_err = eMMC_ComapreData(gau8_WBuf, gau8_RBuf, FCIE_CIFD_BYTE_CNT);
    #endif
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC C, %Xh \r\n", u32_err);
        return u32_err;
    }

    return u32_err;
}

extern U32 gu32_DMAAddr;
U32 eMMCTest_SingleBlkWRC_MIU(U32 u32_eMMC_Addr, U32 u32_DataPattern)
{
    volatile U32 u32_i, *pu32_W=(U32*)gau8_WBuf, *pu32_R=(U32*)gau8_RBuf;
//    eMMC_debug(0,1, "1\n");
    // init data pattern
    for(u32_i=0; u32_i<eMMC_SECTOR_BYTECNT>>2; u32_i++)
    {
        #if 0
        u8_shift = 0;
        switch(u32_i & 0x3)
        {
           case 3: u8_shift +=8;
           case 2: u8_shift +=8;
           case 1: u8_shift +=8;
           case 0:
                gau8_WBuf[u32_i] = u32_DataPattern >> u8_shift;
                gau8_RBuf[u32_i] = ~gau8_WBuf[u32_i];
                break;
        }
        #endif
        pu32_W[u32_i] = u32_DataPattern;
        pu32_R[u32_i] = ~pu32_W[u32_i];
    }
//    eMMC_debug(0,1, "2\n");


    return eMMCTest_SingleBlkWRC_MIU_Ex(u32_eMMC_Addr, (U8*)pu32_W, (U8*)pu32_R);
}


U32 eMMCTest_SingleBlkWRC_MIU_Ex(U32 u32_eMMC_Addr, U8 *pu8_W, U8 *pu8_R)
{
    U32 u32_err;

    u32_err = eMMC_CMD24_MIU(u32_eMMC_Addr, pu8_W);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC W, %Xh \r\n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr, pu8_R);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC R, %Xh \r\n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_ComapreData(pu8_W, pu8_R, eMMC_SECTOR_BYTECNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC C, %Xh \r\n", u32_err);
        return u32_err;
    }

    return u32_err;
}

U32 eMMCTest_SingleBlkWRC_IMI_Ex(U32 u32_eMMC_Addr, U8 *pu8_W, U8 *pu8_R)
{
    U32 u32_err;

    REG_FCIE_SETBIT(FCIE_BOOT, BIT2);
    u32_err = eMMC_CMD24_MIU(u32_eMMC_Addr, pu8_W);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlk IMIWRC W, %Xh \r\n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr, pu8_R);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlk IMIWRC R, %Xh \r\n", u32_err);
        return u32_err;
    }
    REG_FCIE_CLRBIT(FCIE_BOOT, BIT2);

    u32_err = eMMC_ComapreData(pu8_W, pu8_R, eMMC_SECTOR_BYTECNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlk IMIWRC C, %Xh \r\n", u32_err);
        return u32_err;
    }

    return u32_err;
}

U32 eMMCTest_SingleBlkWRC_IMI(U32 u32_eMMC_Addr, U32 u32_DataPattern)
{
    volatile U32 u32_i, *pu32_W=(U32*)DMA_W_IMI_ADDR, *pu32_R=(U32*)DMA_R_IMI_ADDR;
//    eMMC_debug(0,1, "1\n");
    // init data pattern
    for(u32_i=0; u32_i<eMMC_SECTOR_BYTECNT>>2; u32_i++)
    {
        #if 0
        u8_shift = 0;
        switch(u32_i & 0x3)
        {
           case 3: u8_shift +=8;
           case 2: u8_shift +=8;
           case 1: u8_shift +=8;
           case 0:
                gau8_WBuf[u32_i] = u32_DataPattern >> u8_shift;
                gau8_RBuf[u32_i] = ~gau8_WBuf[u32_i];
                break;
        }
        #endif
        pu32_W[u32_i] = u32_DataPattern;
        pu32_R[u32_i] = ~pu32_W[u32_i];
    }
//    eMMC_debug(0,1, "2\n");


    return eMMCTest_SingleBlkWRC_IMI_Ex(u32_eMMC_Addr, (U8*)pu32_W, (U8*)pu32_R);
}


extern U32 gu32_DMAAddr;

#if 0
U32 eMMCTest_SingleBlkWRC_SRAM(U32 u32_eMMC_Addr, U32 u32_DataPattern)
{
    volatile U32 u32_i, *pu32_W=(U32*)gau8_WBuf, *pu32_R=(U32*)gau8_RBuf;

    // init data pattern
    for(u32_i=0; u32_i<eMMC_SECTOR_BYTECNT>>2; u32_i++)
    {
        pu32_W[u32_i] = u32_DataPattern;
        pu32_R[u32_i] = ~pu32_W[u32_i];
    }


    return eMMCTest_SingleBlkWRC_SRAM_Ex(u32_eMMC_Addr, (U8*)pu32_W, (U8*)pu32_R);
}


U32 eMMCTest_SingleBlkWRC_SRAM_Ex(U32 u32_eMMC_Addr, U8 *pu8_W, U8 *pu8_R)
{
    U32 u32_err;
    U8  *pu8_SramAddres = (U8*)CONFIG_SRAM_BASE_ADDRESS;

    u32_err = eMMC_CMD24_MIU(u32_eMMC_Addr, pu8_W);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC W 1, %Xh \r\n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr, pu8_R);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC R 1, %Xh \r\n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_ComapreData(pu8_W, pu8_R, eMMC_SECTOR_BYTECNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC C 1, %Xh \r\n", u32_err);
        return u32_err;
    }

    //dma to sram
    
    REG_FCIE_SETBIT(FCIE_BOOT, BIT_BOOTSRAM_ACCESS_SEL);
    REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_BOOT_MODE_EN); // DMA to SRAM
    u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr, pu8_SramAddres);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC R 2, %Xh \r\n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_CMD24_MIU(u32_eMMC_Addr + 1, pu8_SramAddres);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC W 2, %Xh \r\n", u32_err);
        return u32_err;
    }
    REG_FCIE_CLRBIT(FCIE_BOOT, BIT_BOOTSRAM_ACCESS_SEL);
    REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT_BOOT_MODE_EN); 

    u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr + 1, pu8_R);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC R 3, %Xh \r\n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_ComapreData(pu8_W, pu8_R, eMMC_SECTOR_BYTECNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC C 2, %Xh \r\n", u32_err);
        return u32_err;
    }


    return u32_err;
}
#endif

#if  1
U32  eMMCTest_SgW_MIU(U32 u32_eMMC_Addr)
{
    U16 u16_mode, u16_ctrl=0, u16_reg;
    U32 u32_dmaaddr,u32_dma_addr,u32_dmalen,u32_arg;
    U32 u32_err,u32_err_12=eMMC_ST_SUCCESS,u32_i;
    //volatile U16 u16_tmp_reg,u16_tmp_reg2, u16_Rx39;

    eMMC_FCIE_ClearEvents();
    // -------------------------------
    // send cmd
    u16_ctrl |= (BIT_SD_CMD_EN | BIT_SD_RSP_EN|BIT_ADMA_EN);
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    //eMMC_debug(0, 1, "p_AdmaDesc_st 0x%X\r\n", p_AdmaDesc_st);
    for(u32_i=0; u32_i< MAX_SCATTERLIST_COUNT; u32_i++)
    {
        u32_dmaaddr = pSG_st[u32_i].u32_dma_address;
        u32_dmalen  = pSG_st[u32_i].u32_length;
        u32_dma_addr = eMMC_translate_DMA_address_Ex(u32_dmaaddr, u32_dmalen, WRITE_TO_eMMC);

        //eMMC_debug(0, 1, "address 0x%X, len 0x%X\r\n", u32_dma_addr, u32_dmalen);

        //parsing dma address, length to descriptor
        p_AdmaDesc_st[u32_i].u32_Address =cpu2le32(u32_dma_addr);
        p_AdmaDesc_st[u32_i].u32_DmaLen = cpu2le32(u32_dmalen);
        p_AdmaDesc_st[u32_i].u16_JobCnt = cpu2le32(u32_dmalen >> 9);
        p_AdmaDesc_st[u32_i].u16_Config = 0;        //FPGA Mode always miu0

        if(u32_i == (MAX_SCATTERLIST_COUNT - 1))
            p_AdmaDesc_st[u32_i].u16_Config |= 1; //end of ADMA

    }
    eMMC_translate_DMA_address_Ex((U32)p_AdmaDesc_st, sizeof(p_AdmaDesc_st)*MAX_SCATTERLIST_COUNT, WRITE_TO_eMMC);

    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);

    REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, (U32)((U8 *)(p_AdmaDesc_st)) & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16,(U32)((U8 *)(p_AdmaDesc_st)) >> 16);

    REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, 0x10 & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16,0x10 >> 16);

    // -------------------------------
    u32_arg =  u32_eMMC_Addr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 25, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD25 %Xh \r\n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }
    // -------------------------------
    // check device
    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR) )
    {
        u32_err = eMMC_ST_ERR_CMD25;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD25 Reg.12: %04Xh, Err: %Xh,  Arg: %Xh\r\n", u16_reg, u32_err, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        u32_err = eMMC_ST_ERR_CMD25_CHK_R1;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD25 check R1 error: %Xh\r\n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W|BIT_ADMA_EN);
    REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W|BIT_ADMA_EN|BIT_JOB_START);
    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    REG_FCIE_W(FCIE_MIE_INT_EN, BIT_DMA_END);
    #endif
    
    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_DMA_END, eMMC_GENERIC_WAIT_TIME);

    if(u32_err!= eMMC_ST_SUCCESS)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC Err: w timeout \r\n");
        //eMMC_die("\r\n");
        //while(1);
        g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_ERROR_RETRY;
    }

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if((u16_reg & (BIT_SD_W_FAIL|BIT_SD_W_CRC_ERR))||(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_ERROR_RETRY))
    {

        u32_err = eMMC_ST_ERR_CMD25;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0,
            "eMMC Err: CMD25 Reg.12: %04Xh, Err: %Xh\r\n", u16_reg, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:
    u32_err_12 = eMMC_CMD12(g_eMMCDrv.u16_RCA);

    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_CLK_EN);
    return u32_err ? u32_err : u32_err_12;
}

U32  eMMCTest_SgR_MIU(U32 u32_eMMC_Addr)
{
    U16 u16_mode, u16_ctrl, u16_reg;
    U32 u32_dmaaddr,u32_dma_addr,u32_dmalen,u32_arg;
    U32 u32_err,u32_err_12=eMMC_ST_SUCCESS,u32_i;

    eMMC_FCIE_ClearEvents();

    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;

    REG_FCIE_W(FCIE_SD_MODE, u16_mode);

    for(u32_i=0; u32_i< MAX_SCATTERLIST_COUNT; u32_i++)
    {
        u32_dmaaddr = pSG_st[u32_i].u32_dma_address;
        u32_dmalen  = pSG_st[u32_i].u32_length;
        u32_dma_addr = eMMC_translate_DMA_address_Ex(u32_dmaaddr, u32_dmalen, READ_FROM_eMMC);

        //eMMC_debug(0, 1, "address 0x%X, len 0x%X\r\n", u32_dma_addr, u32_dmalen);

        //parsing dma address, length to descriptor
        p_AdmaDesc_st[u32_i].u32_Address =cpu2le32(u32_dma_addr);
        p_AdmaDesc_st[u32_i].u32_DmaLen = cpu2le32(u32_dmalen);
        p_AdmaDesc_st[u32_i].u16_JobCnt = cpu2le32(u32_dmalen >> 9);
        p_AdmaDesc_st[u32_i].u16_Config  = 0;

        #if defined(FCIE_ZDEC_TEST) && FCIE_ZDEC_TEST
        p_ZDecDesc_st[u32_i].u32_Address =  cpu2le32(u32_dma_addr);
        p_ZDecDesc_st[u32_i].u32_JobCnt = cpu2le32(u32_dmalen >> 9);
        p_ZDecDesc_st[u32_i].u32_MiuSel  = 0;
        #endif

        if(u32_i == (MAX_SCATTERLIST_COUNT - 1))
            p_AdmaDesc_st[u32_i].u16_Config |= 1;
    }
    p_AdmaDesc_st[u32_i-1].u16_Config |= 1;
    #if defined(FCIE_ZDEC_TEST) && FCIE_ZDEC_TEST
    p_ZDecDesc_st[u32_i-1].u32_End = 1;
    #endif

    #if defined(FCIE_ZDEC_TALK_TEST) && FCIE_ZDEC_TALK_TEST
    REG_FCIE_SETBIT(FCIE_ZDEC_CTL0, BIT_ZDEC_EN);
    #endif

    #if defined(FCIE_ZDEC_TALK_TEST) && FCIE_ZDEC_TALK_TEST
    zdec_top_fpga_feed(1, // last
                       p_ZDecDesc_st,
                       sizeof(struct _ZDecDescriptor) * u32_i);
    eMMC_debug(0, 1, "zdec_top_fpga_feed sadr=%p\r\n", p_ZDecDesc_st);
    eMMC_debug(0, 1, "zdec_top_fpga_feed size=%X\r\n", sizeof(struct _ZDecDescriptor) * u32_i);
    //eMMC_debug(0, 1, "u32_i=%d\r\n", u32_i);

    //eMMC_dump_mem(p_AdmaDesc_st, u32_i *  sizeof(struct _AdmaDescriptor));
    eMMC_dump_mem(p_ZDecDesc_st, u32_i *  sizeof(struct _ZDecDescriptor));

    //printf("ZDEC test stop here!\r\n");
    //while(1);
    #endif
    eMMC_translate_DMA_address_Ex((U32)p_AdmaDesc_st, sizeof(p_AdmaDesc_st)*MAX_SCATTERLIST_COUNT, WRITE_TO_eMMC);

    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);

    REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, (U32)((U8 *)(p_AdmaDesc_st)) & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16,(U32)((U8 *)(p_AdmaDesc_st)) >> 16);

    REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, 0x10 & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16,0x10 >> 16);

    // -------------------------------
    // send cmd
    u16_ctrl = (BIT_SD_CMD_EN | BIT_SD_RSP_EN|BIT_SD_DAT_EN|BIT_ADMA_EN);

    // -------------------------------
    u32_arg =  u32_eMMC_Addr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 18, eMMC_R1_BYTE_CNT);


    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_DMA_END
        , eMMC_GENERIC_WAIT_TIME);

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

    if((eMMC_ST_SUCCESS != u32_err) || (u16_reg & BIT_SD_FCIE_ERR_FLAGS))
    {
        u32_err = eMMC_ST_ERR_CMD18;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0,
            "eMMC Err: CMD18 Reg.12: %04Xh, Err: %Xh\r\n", u16_reg, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }


    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if((u16_reg & BIT_SD_FCIE_ERR_FLAGS)||(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_ERROR_RETRY))
    {
        u32_err = eMMC_ST_ERR_CMD18;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD18 Reg.12: %04Xh, Err: %Xh\n", u16_reg, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        u32_err = eMMC_ST_ERR_CMD18;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD18 check R1 error: %Xh\r\n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:
    u32_err_12 = eMMC_CMD12(g_eMMCDrv.u16_RCA);

    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_CLK_EN);

    return u32_err ? u32_err : u32_err_12;
}
#endif
U32  eMMCTest_SgWRC_MIU(U32 u32_eMMC_Addr, U16 u16_BlkCnt, U32 u32_DataPattern)
{
    U32 u32_err;
    U32 u32_i, u32_j, *pu32_W=(U32*)gau8_WBuf, *pu32_R=(U32*)gau8_RBuf;
    U32 u32_temp, u32_sg_blocks;

    // init data pattern
    for(u32_i=0; u32_i<TEST_BUFFER_SIZE>>2; u32_i++)
    {
        pu32_W[u32_i] = u32_DataPattern;
        pu32_R[u32_i] = ~pu32_W[u32_i];
    }

    //check scatterlist count
    if(u16_BlkCnt % MAX_SCATTERLIST_COUNT)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC Err: Scatterlist Count Error\n");
        return eMMC_ST_ERR_NOT_ALIGN;
    }

    u32_sg_blocks = u16_BlkCnt/MAX_SCATTERLIST_COUNT;
    u32_temp =0;
    //setting scatterlist dma address and dma length for write multiple block
    for(u32_i=0; u32_i< MAX_SCATTERLIST_COUNT/4; u32_i++)
    {
        for(u32_j=0; u32_j< 4; u32_j++) // 4 groups
        {
            pSG_st[u32_i*4+u32_j].u32_dma_address = (U32)(gau8_WBuf+ u32_temp);
            if(u32_j < 3) // first 3 have 1 block
            {
                pSG_st[u32_i*4+u32_j].u32_length = eMMC_SECTOR_512BYTE;
                u32_temp += eMMC_SECTOR_512BYTE;
            }
            else
            {
                pSG_st[u32_i*4+u32_j].u32_length = (4*u32_sg_blocks-3)<<eMMC_SECTOR_512BYTE_BITS;
                u32_temp += (4*u32_sg_blocks-3)<<eMMC_SECTOR_512BYTE_BITS;
            }
        }
    }
    //write scatterlist
    u32_err = eMMCTest_SgW_MIU(u32_eMMC_Addr);
    if(u32_err != eMMC_ST_SUCCESS)
    {
        eMMC_debug(0,1,"Err, SgWRC_MIU W, %Xh \n", u32_err);
        return u32_err;
    }

    u32_temp =0;
    //setting scatterlist dma address and dma length for read multiple block
    for(u32_i=0; u32_i< MAX_SCATTERLIST_COUNT/4; u32_i++)
    {
        for(u32_j=0; u32_j< 4; u32_j++) // 4 groups
        {
            pSG_st[u32_i*4+u32_j].u32_dma_address = (U32)(gau8_RBuf+ u32_temp);
            if(u32_j < 3) // first 3 have 1 block
            {
                pSG_st[u32_i*4+u32_j].u32_length = eMMC_SECTOR_512BYTE;
                u32_temp += eMMC_SECTOR_512BYTE;
            }
            else
            {
                pSG_st[u32_i*4+u32_j].u32_length = (4*u32_sg_blocks-3)<<eMMC_SECTOR_512BYTE_BITS;
                u32_temp += (4*u32_sg_blocks-3)<<eMMC_SECTOR_512BYTE_BITS;
            }
        }
    }

    //read scatterlist
    u32_err = eMMCTest_SgR_MIU(u32_eMMC_Addr);
    if(u32_err != eMMC_ST_SUCCESS)
    {
        eMMC_debug(0,1,"Err, SgWRC_MIU R, %Xh \n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_ComapreData(
        gau8_WBuf, gau8_RBuf, u16_BlkCnt<<eMMC_SECTOR_512BYTE_BITS);

    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, MultiBlkWRC C, %u, %Xh \n", u32_i, u32_err);
        return u32_err;
    }
    return eMMC_ST_SUCCESS;
}

U32 eMMCTest_MultiBlkWRC_MIU(U32 u32_eMMC_Addr, U16 u16_BlkCnt, U32 u32_DataPattern)
{
    U32 u32_err;
    U32 u32_i, *pu32_W=(U32*)gau8_WBuf, *pu32_R=(U32*)gau8_RBuf;
    U16 u16_BlkCnt_tmp;

    // init data pattern

    for(u32_i=0; u32_i<FCIE_CIFD_BYTE_CNT>>2; u32_i++)
    {
        pu32_W[u32_i] = u32_DataPattern;
        pu32_R[u32_i] = ~pu32_W[u32_i];
    }

    u16_BlkCnt_tmp = TEST_BUFFER_SIZE >> eMMC_SECTOR_512BYTE_BITS;
    u16_BlkCnt_tmp = u16_BlkCnt_tmp > u16_BlkCnt ? u16_BlkCnt : u16_BlkCnt_tmp;

    for(u32_i=0; u32_i<u16_BlkCnt_tmp; u32_i++)
    {
        u32_err = eMMC_CMD25_MIU(
            u32_eMMC_Addr+u32_i, gau8_WBuf+(u32_i<<eMMC_SECTOR_512BYTE_BITS), u16_BlkCnt_tmp-u32_i);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(0,1,"Err, MultiBlkWRC W, %u, %Xh \n", u32_i, u32_err);
            goto LABEL_OF_ERROR;
        }

        u32_err = eMMC_CMD18_MIU(
            u32_eMMC_Addr+u32_i, gau8_RBuf+(u32_i<<eMMC_SECTOR_512BYTE_BITS), u16_BlkCnt_tmp-u32_i);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(0,1,"Err, MultiBlkWRC R, %u, %Xh \n", u32_i, u32_err);
            goto LABEL_OF_ERROR;
        }

        u32_err = eMMC_ComapreData(
            gau8_WBuf+(u32_i<<eMMC_SECTOR_512BYTE_BITS), gau8_RBuf+(u32_i<<eMMC_SECTOR_512BYTE_BITS), (u16_BlkCnt_tmp-u32_i)<<eMMC_SECTOR_512BYTE_BITS);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(0,1,"Err, MultiBlkWRC C, %u, %Xh \n", u32_i, u32_err);
            goto LABEL_OF_ERROR;
        }

        u32_i += u16_BlkCnt_tmp-u32_i;
    }

    return u32_err;

    LABEL_OF_ERROR:
    return u32_err;
}

U32 eMMCTest_BIST(U8 u8_PatSel)
{
    U32 u32_err = eMMC_ST_SUCCESS;

    //eMMC_debug(0, 1, " mode %d\r\n", u8_PatSel);

    // Select pattern
    REG_FCIE_SETBIT(FCIE_BIST, (u8_PatSel & 0xF)<<12);

    // Set BIST Mode
    REG_FCIE_SETBIT(FCIE_TEST_MODE, BIT_BIST_MODE);

    // Do Block Write (CMD24)
    eMMC_CMD24(0, gau8_WBuf);

    // Do Block Read (CMD17)
    eMMC_CMD17(0, gau8_RBuf);

    // Wait BIST Error Flag
    if( (REG_FCIE(FCIE_BIST) & BIT0) == BIT0 )
    {
        printf("\033[7;31m%s: BIST pattern type %d fail!!\033[m\r\n", __FUNCTION__, u8_PatSel);
        while(1);
        //return eMMC_ST_ERR_BIST_FAIL;
    }

    printf("%s: BIST pattern type %d ok!!\r\n", __FUNCTION__, u8_PatSel);

    REG_FCIE_CLRBIT(FCIE_TEST_MODE, BIT_BIST_MODE);

    return u32_err;
}

U32 eMMCTest_RSP_Shift_Tuning_Ex(U32 u32_eMMC_Addr, U8 *pu8_W, U8 *pu8_R)
{
    U32 u32_err;
    U8  u8_i;
    U8  u8_RSP_test_result[16] = {0};

    u32_err = eMMC_CMD24_MIU(u32_eMMC_Addr, pu8_W);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC W, %Xh \r\n", u32_err);
        return eMMC_ST_ERR_CMD24_MIU;
    }

    g_eMMCDrv.u8_disable_retry = 1;
    g_eMMCDrv.u32_DrvFlag|=DRV_FLAG_TUNING_TTABLE;

    for(u8_i=0; u8_i<16; u8_i++)
    {
        REG_FCIE_CLRBIT(FCIE_RSP_SHIFT_CNT, BIT_RSP_SHIFT_TUNE_MASK);
        REG_FCIE_SETBIT(FCIE_RSP_SHIFT_CNT, BIT_RSP_SHIFT_SEL|(u8_i&0xF));

        memset(pu8_R, 0, 512);

        u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr, pu8_R);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(0,1,"RSP shift tuning %d is not ok\r\n", u8_i);
            u8_RSP_test_result[u8_i] = 0;
            continue;
        }

        u32_err = eMMC_ComapreData(pu8_W, pu8_R, eMMC_SECTOR_BYTECNT);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(0,1,"Err, SingleBlkWRC C, %Xh \r\n", u32_err);
            return eMMC_ST_ERR_DATA_MISMATCH;
        }

        eMMC_debug(0,1,"RSP shift tuning %d is ok!!!\r\n", u8_i);
        u8_RSP_test_result[u8_i] = 1;
    }

    REG_FCIE_CLRBIT(FCIE_RSP_SHIFT_CNT, BIT_RSP_SHIFT_SEL);

    g_eMMCDrv.u8_disable_retry = 0;
    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_TUNING_TTABLE;

    for(u8_i=0; u8_i<16; u8_i++)
    {
        if( u8_RSP_test_result[u8_i] )
            return eMMC_ST_SUCCESS;
    }

    return eMMC_ST_ERR_RSP_TUNING_FAIL;
}

U32 eMMCTest_RSP_Shift_Tuning(U32 u32_eMMC_Addr, U32 u32_DataPattern)
{
    volatile U32 u32_i, *pu32_W=(U32*)gau8_WBuf, *pu32_R=(U32*)gau8_RBuf;

    eMMC_debug(0, 1, "\033[7;35mData pattern %08X\033[m\r\n", u32_DataPattern);

    // init data pattern
    #if defined(SEL_FCIE5) && (SEL_FCIE5)

    if(u32_DataPattern == 0x00112233 || (u32_DataPattern == 0x33221100))
    {
        for(u32_i = 0; u32_i < TEST_BUFFER_SIZE; u32_i ++)
        {
            if(u32_DataPattern == 0x00112233)
            {
                gau8_WBuf[u32_i] = u32_i & 0xFF;
            }
            else
            {
                gau8_WBuf[u32_i] = 0xFF - (u32_i & 0xFF);
            }
            gau8_RBuf[u32_i] = ~gau8_WBuf[u32_i];
        }
    }
    else
    {
        for(u32_i=0; u32_i<TEST_BUFFER_SIZE>>2; u32_i++)
        {
            pu32_W[u32_i] = u32_DataPattern;
            pu32_R[u32_i] = ~pu32_W[u32_i];
        }
    }
    #else
    for(u32_i=0; u32_i<TEST_BUFFER_SIZE>>2; u32_i++)
    {
        pu32_W[u32_i] = u32_DataPattern;
        pu32_R[u32_i] = ~pu32_W[u32_i];
    }
    #endif

    return eMMCTest_RSP_Shift_Tuning_Ex(u32_eMMC_Addr, (U8*)pu32_W, (U8*)pu32_R);
}

U32 eMMCTest_WCRC_Shift_Tuning_Ex(U32 u32_eMMC_Addr, U8 *pu8_W, U8 *pu8_R)
{
    U32 u32_err;
    U8  u8_i;
    U8  u8_WCRC_test_result[16] = {0};
    g_eMMCDrv.u8_disable_retry = 1;
    g_eMMCDrv.u32_DrvFlag|=DRV_FLAG_TUNING_TTABLE;

    for(u8_i=0; u8_i<16; u8_i++)
    {
        //eMMC_debug(0, 0, "\033[7;35m1 FCIE_RX_SHIFT_CNT=%04X\033[m\r\n", REG_FCIE(FCIE_RX_SHIFT_CNT));
        REG_FCIE_CLRBIT(FCIE_RX_SHIFT_CNT, BIT_WRSTS_SHIFT_TUNE_MASK);
        REG_FCIE_SETBIT(FCIE_RX_SHIFT_CNT, BIT_WRSTS_SHIFT_SEL|((u8_i&0xF)<<8));
        //eMMC_debug(0, 0, "\033[7;36m2 FCIE_RX_SHIFT_CNT=%04X\033[m\r\n", REG_FCIE(FCIE_RX_SHIFT_CNT));

        u32_err = eMMC_CMD24_MIU(u32_eMMC_Addr, pu8_W);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(0,1,"WCRC shift tuning %d is not ok\r\n", u8_i);
            continue;
        }

        g_eMMCDrv.u8_disable_retry = 0;

        memset(pu8_R, 0, 512);

        u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr, pu8_R);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(0,1,"Err, SingleBlkWRC R, %Xh \r\n", u32_err);
            while(1);
        }

        u32_err = eMMC_ComapreData(pu8_W, pu8_R, eMMC_SECTOR_BYTECNT);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(0,1,"Err, SingleBlkWRC C, %Xh \r\n", u32_err);
            while(1);
        }

        eMMC_debug(0,1,"WCRC shift tuning %d is ok!!!\r\n", u8_i);

        u8_WCRC_test_result[u8_i] = 1;
    }

    REG_FCIE_CLRBIT(FCIE_RX_SHIFT_CNT, BIT_WRSTS_SHIFT_SEL);

    g_eMMCDrv.u8_disable_retry = 0;
    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_TUNING_TTABLE;

    for(u8_i=0; u8_i<16; u8_i++)
    {
        if( u8_WCRC_test_result[u8_i] )
            return eMMC_ST_SUCCESS;
    }

    return eMMC_ST_ERR_RSP_TUNING_FAIL;
}

U32 eMMCTest_WCRC_Shift_Tuning(U32 u32_eMMC_Addr, U32 u32_DataPattern)
{
    volatile U32 u32_i, *pu32_W=(U32*)gau8_WBuf, *pu32_R=(U32*)gau8_RBuf;

    eMMC_debug(0, 1, "\033[7;35mData pattern %08X\033[m\r\n", u32_DataPattern);

    // init data pattern
    #if defined(SEL_FCIE5) && (SEL_FCIE5)

    if(u32_DataPattern == 0x00112233 || (u32_DataPattern == 0x33221100))
    {
        for(u32_i = 0; u32_i < TEST_BUFFER_SIZE; u32_i ++)
        {
            if(u32_DataPattern == 0x00112233)
            {
                gau8_WBuf[u32_i] = u32_i & 0xFF;
            }
            else
            {
                gau8_WBuf[u32_i] = 0xFF - (u32_i & 0xFF);
            }
            gau8_RBuf[u32_i] = ~gau8_WBuf[u32_i];
        }
    }
    else
    {
        for(u32_i=0; u32_i<TEST_BUFFER_SIZE>>2; u32_i++)
        {
            pu32_W[u32_i] = u32_DataPattern;
            pu32_R[u32_i] = ~pu32_W[u32_i];
        }
    }
    #else
    for(u32_i=0; u32_i<TEST_BUFFER_SIZE>>2; u32_i++)
    {
        pu32_W[u32_i] = u32_DataPattern;
        pu32_R[u32_i] = ~pu32_W[u32_i];
    }
    #endif

    return eMMCTest_WCRC_Shift_Tuning_Ex(u32_eMMC_Addr, (U8*)pu32_W, (U8*)pu32_R);
}

U32 eMMCTest_RSTOP_Shift_Tuning_Ex(U32 u32_eMMC_Addr, U8 *pu8_W, U8 *pu8_R)
{
    U32 u32_err;
    U8  u8_i;
    U8  u8_RSTOP_test_result[16] = {0};

    u32_err = eMMC_CMD24_MIU(u32_eMMC_Addr, pu8_W);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC W, %Xh \r\n", u32_err);
        while(1);
    }

    g_eMMCDrv.u8_disable_retry = 1;
    g_eMMCDrv.u32_DrvFlag|=DRV_FLAG_TUNING_TTABLE;

    for(u8_i=0; u8_i<16; u8_i++)
    {
        REG_FCIE_CLRBIT(FCIE_RX_SHIFT_CNT, BIT_RSTOP_SHIFT_TUNE_MASK);
        REG_FCIE_SETBIT(FCIE_RX_SHIFT_CNT, BIT_RSTOP_SHIFT_SEL|u8_i);

        memset(pu8_R, 0, 512);

        u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr, pu8_R);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(0,1,"RStop shift tuning %d is not ok\r\n", u8_i);
            continue;
            //return 0;
        }

        u32_err = eMMC_ComapreData(pu8_W, pu8_R, eMMC_SECTOR_BYTECNT);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(0,1,"Err, SingleBlkWRC C, %Xh \r\n", u32_err);
            while(1);
        }

        eMMC_debug(0,1,"RStop shift tuning %d is ok!!!\r\n", u8_i);

        u8_RSTOP_test_result[u8_i] = 1;
    }
    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_TUNING_TTABLE;

    REG_FCIE_CLRBIT(FCIE_RX_SHIFT_CNT, BIT_RSTOP_SHIFT_SEL);

    g_eMMCDrv.u8_disable_retry = 0;

    for(u8_i=0; u8_i<16; u8_i++)
    {
        if( u8_RSTOP_test_result[u8_i] )
            return eMMC_ST_SUCCESS;
    }

    return eMMC_ST_ERR_RSP_TUNING_FAIL;
}

U32 eMMCTest_RSTOP_Shift_Tuning(U32 u32_eMMC_Addr, U32 u32_DataPattern)
{
    volatile U32 u32_i, *pu32_W=(U32*)gau8_WBuf, *pu32_R=(U32*)gau8_RBuf;

    eMMC_debug(0, 1, "\033[7;35mData pattern %08X\033[m\r\n", u32_DataPattern);

    // init data pattern
    #if defined(SEL_FCIE5) && (SEL_FCIE5)

    if(u32_DataPattern == 0x00112233 || (u32_DataPattern == 0x33221100))
    {
        for(u32_i = 0; u32_i < TEST_BUFFER_SIZE; u32_i ++)
        {
            if(u32_DataPattern == 0x00112233)
            {
                gau8_WBuf[u32_i] = u32_i & 0xFF;
            }
            else
            {
                gau8_WBuf[u32_i] = 0xFF - (u32_i & 0xFF);
            }
            gau8_RBuf[u32_i] = ~gau8_WBuf[u32_i];
        }
    }
    else
    {
        for(u32_i=0; u32_i<TEST_BUFFER_SIZE>>2; u32_i++)
        {
            pu32_W[u32_i] = u32_DataPattern;
            pu32_R[u32_i] = ~pu32_W[u32_i];
        }
    }
    #else
    for(u32_i=0; u32_i<TEST_BUFFER_SIZE>>2; u32_i++)
    {
        pu32_W[u32_i] = u32_DataPattern;
        pu32_R[u32_i] = ~pu32_W[u32_i];
    }
    #endif

    return eMMCTest_RSTOP_Shift_Tuning_Ex(u32_eMMC_Addr, (U8*)pu32_W, (U8*)pu32_R);
}

U32 eMMCTest_Command_No_Response(void)
{

    g_eMMCDrv.u8_make_sts_err = FCIE_MAKE_CMD_NO_RSP;

    eMMC_CMD0(0);

    g_eMMCDrv.u8_make_sts_err = 0;

    return 0;
}

U32 eMMCTest_Command_Response_CRC_Error(void)
{
    g_eMMCDrv.u8_make_sts_err = FCIE_MAKE_CMD_RSP_ERR;

    eMMC_CMD17_MIU(eMMC_TEST_BLK_0, gau8_RBuf);

    g_eMMCDrv.u8_make_sts_err = 0;

    return 0;
}

U32 eMMCTest_Read_CRC_Error(void)
{
    g_eMMCDrv.u8_make_sts_err = FCIE_MAKE_RD_CRC_ERR;

    eMMC_CMD17_MIU(eMMC_TEST_BLK_0, gau8_RBuf);

    g_eMMCDrv.u8_make_sts_err = 0;

    return 0;
}

U32 eMMCTest_Write_CRC_Error(void)
{
    g_eMMCDrv.u8_make_sts_err = FCIE_MAKE_WR_CRC_ERR;

    eMMC_CMD24_MIU(eMMC_TEST_BLK_0, gau8_WBuf);

    g_eMMCDrv.u8_make_sts_err = 0;

    return 0;
}

U32 eMMCTest_Write_Timeout_Error(void)
{
    g_eMMCDrv.u8_make_sts_err = FCIE_MAKE_WR_TOUT_ERR;

    eMMC_CMD25_MIU(eMMC_TEST_BLK_0, gau8_WBuf, 2);

    g_eMMCDrv.u8_make_sts_err = 0;

    return 0;
}

U32 eMMCTest_Read_Timeout_Error(void)
{
    g_eMMCDrv.u8_make_sts_err = FCIE_MAKE_RD_TOUT_ERR;

    eMMC_CMD0(0);

    g_eMMCDrv.u8_make_sts_err = 0;

    return 0;
}


// =========================================================
#define IF_eMMC_BOOT_MODE_STG2   1

#if 0 == IF_eMMC_BOOT_MODE_STG2
eMMC_ALIGN0 static U8 gau8_BootImage0[0x18200] eMMC_ALIGN1;
#else
//eMMC_ALIGN0 static U8 gau8_BootImage0[0x400] eMMC_ALIGN1;
//eMMC_ALIGN0 static U8 gau8_BootImage1[0x800] eMMC_ALIGN1;
//eMMC_ALIGN0 static U8 gau8_BootImage2[0x18200-0xC00] eMMC_ALIGN1;
#endif


//========================================================
// exposed function
//========================================================
#define eMMC_SPEED_TEST_COUNTDOWN          3

#define eMMC_SPEED_TEST_SINGLE_BLKCNT_W    500
#define eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W  30*(1024*1024/512) // total 30MB

#define eMMC_SPEED_TEST_R_MULTI            10
#define eMMC_SPEED_TEST_SINGLE_BLKCNT_R    eMMC_SPEED_TEST_SINGLE_BLKCNT_W*eMMC_SPEED_TEST_R_MULTI
#define eMMC_SPEED_TEST_MULTIPLE_BLKCNT_R  eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W*eMMC_SPEED_TEST_R_MULTI

void eMMCTest_DownCount(U32 u32_Sec) // verify the HW Timer
{
    U32 u32_i, u32_t0, u32_t1;//, au32_tmp[6];

    //eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"2013 0125 x\r\n");
    for(u32_i=0; u32_i<u32_Sec; u32_i++)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"%u \r\n", u32_Sec-u32_i);
        #if 0
        eMMC_hw_timer_start();
        au32_tmp[u32_i*2] = eMMC_hw_timer_tick();
        eMMC_hw_timer_delay(HW_TIMER_DELAY_1s);
        au32_tmp[u32_i*2+1] = eMMC_hw_timer_tick();
        #else
        eMMC_hw_timer_start();
        u32_t0 = eMMC_hw_timer_tick();
        u32_t1 = eMMC_hw_timer_tick();
        if(u32_t0 > u32_t1)
            while(0==(u32_t0-eMMC_hw_timer_tick())/eMMC_HW_TIMER_MHZ)
                ;
        else
            while(0==(eMMC_hw_timer_tick()-u32_t0)/eMMC_HW_TIMER_MHZ)
                ;
        #endif
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,".\r\n");
    //eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"%u %u   %u %u   %u %u \r\n",
    //  au32_tmp[0], au32_tmp[1], au32_tmp[2], au32_tmp[3], au32_tmp[4], au32_tmp[5]);
}

void eMMC_Prepare_Power_Saving_Mode_Queue(void)
{
    U32 u32_Count;
    U16 u16_Reg;
    
    REG_FCIE_CLRBIT(FCIE_PWR_SAVE_CTL, BIT_BAT_SD_POWER_SAVE_MASK);

    /* (1) Clear HW Enable */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x00), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x01),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x07);

    /* (2) Clear All Interrupt */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x02), 0xffff);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x03),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x00);

    /* (3) Clear SD MODE Enable */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x04), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x05),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0B);

    /* (4) Clear SD CTL Enable */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x06), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x07),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0C);

    /* (5) Reset Start */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x08), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x09),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x3F);

    /* (6) Reset End */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0A), 0x0001);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0B),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x3F);

    /* (7) Set "SD_MOD" */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0C), 0x0021);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0D),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0B);

    /* (8) Enable "reg_sd_en" */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0E), 0x0001);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x0F),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x07);

    /* (9) Command Content, IDLE */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x10), 0x0040);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x11),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x20);

    /* (10) Command Content, STOP */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x12), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x13),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x21);

    /* (11) Command Content, STOP */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x14), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x15),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x22);

    /* (12) Command & Response Size */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x16), 0x0500);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x17),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0A);

    /* (13) Enable Interrupt, SD_CMD_END */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x18), 0x0002);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x19),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x01);

    /* (14) Command Enable + job Start */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1A), 0x0044);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1B),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0C);

    /* (15) Wait Interrupt */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1C), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1D),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WINT);

    /* (16) STOP */
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1E), 0x0000);
    REG_FCIE_W(GET_REG_ADDR(FCIE_POWEER_SAVE_MODE_BASE, 0x1F),
               PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_STOP);

    REG_FCIE_CLRBIT(FCIE_PWR_SAVE_CTL, BIT_SD_POWER_SAVE_RST);
    REG_FCIE_SETBIT(FCIE_PWR_SAVE_CTL, BIT_SD_POWER_SAVE_RST);

    REG_FCIE_SETBIT(FCIE_PWR_SAVE_CTL, BIT_POWER_SAVE_MODE);

    
    /* Step 4: Before Nand IP DMA end, use RIU interface to test power save function */
    REG_FCIE_W(FCIE_PWR_SAVE_CTL, BIT_POWER_SAVE_MODE
                     | BIT_SD_POWER_SAVE_RIU
                     | BIT_POWER_SAVE_MODE_INT_EN
                     | BIT_SD_POWER_SAVE_RST);
    
    for (u32_Count=0; u32_Count < HW_TIMER_DELAY_1s; u32_Count++)
    {
        REG_FCIE_R(FCIE_PWR_SAVE_CTL, u16_Reg);
        if ((u16_Reg & BIT_POWER_SAVE_MODE_INT) == BIT_POWER_SAVE_MODE_INT)
        {
            eMMC_debug(0, 1, "eMMC_PWRSAVE_CTL: 0x%x\r\n", REG_FCIE(FCIE_PWR_SAVE_CTL));
            break;
        }
        eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
    }
    REG_FCIE_R(FCIE_PWR_SAVE_CTL, u16_Reg);
    eMMC_debug(0, 1, "eMMC_PWRSAVE_CTL: 0x%x\r\n", u16_Reg); 

}
U8 u8_BusWidth[3] = { 1, 4, 8};
//U8 u8_BusWidth[3] = { 1, 8, 4};
//U8 u8_BusWidth[3] = { 8, 4, 1};

U32 u32_test_pattern[] = {
    0x00112233,
    0x33221100,
    0x00000000,
    0xFFFFFFFF,
    0xFF00FF00,
    0x00FF00FF,
    0x55555555,
    0xAAAAAAAA,
    0xAA55AA55,
    0x55AA55AA,
    0xA55AA55A,
    0x5AA55AA5
};

#if (defined(WRITE_TEST_SQUASHFS_IMAGE) && WRITE_TEST_SQUASHFS_IMAGE) || \
    (defined(FCIE_ZDEC_TEST) && FCIE_ZDEC_TEST)
#include "squashfs_test_img.h"
#endif

#if defined(FCIE_ZDEC_TEST) && FCIE_ZDEC_TEST
#include "squashfs_test_1_96_35731_131072.h"
#include "squashfs_test_2_35827_19864_70326.h"
#include "squashfs_test_3_72094_29571_110762.h"
#include "squashfs_test_4_107474_13776_72656.h"
#include "squashfs_test_5_121250_28458_106205.h"

U8* squashfs_test_data[5] =
{
    squashfs_test_1_96_35731_131072_bin,
    squashfs_test_2_35827_19864_70326_bin,
    squashfs_test_3_72094_29571_110762_bin,
    squashfs_test_4_107474_13776_72656_bin,
    squashfs_test_5_121250_28458_106205_bin,
};
#endif

typedef struct _SQUASHFS_INFO
{
    U32 u32_offset;
    U32 u32_compressed_size;
    U32 u32_uncompressed_size;
} SQUASHFS_INFO;

U32 eMMC_IPVerify_Main(void)
{
    U32 u32_err;

    //U8 u8_ext_csd[512];
    #if (defined(WRITE_TEST_SQUASHFS_IMAGE) && WRITE_TEST_SQUASHFS_IMAGE) || \
        (defined(FCIE_GPIO_PAD_VERIFY) && FCIE_GPIO_PAD_VERIFY) || \
        (defined(FCIE_SDR_FLASH_MACRO8_VERIFY) && FCIE_SDR_FLASH_MACRO8_VERIFY) || \
        (defined(FCIE_DDR_FLASH_MACRO8_VERIFY) && FCIE_DDR_FLASH_MACRO8_VERIFY) || \
        (defined(FCIE_HS200_FLASH_MACRO32_VERIFY) && FCIE_HS200_FLASH_MACRO32_VERIFY) || \
        (defined(FCIE_HS400_FLASH_MACRO32_VERIFY) && FCIE_HS400_FLASH_MACRO32_VERIFY) || \
        (defined(FCIE_eMMC5_1_FLASH_MACRO32_VERIFY) && FCIE_eMMC5_1_FLASH_MACRO32_VERIFY)
    U8 u8_i;
    int i;
    #endif

    #if defined(FCIE_REGRESSION_TEST) && FCIE_REGRESSION_TEST
    U32 u32_regression_test_count = 0;
    #endif

    gau8_WBuf = (U8*)DMA_W_ADDR;
    gau8_RBuf = (U8*)DMA_R_ADDR;

    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"eMMC_IPVerify_Main\r\n");
    #if 1
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"%Xh %Xh %Xh\r\n",
        (U32)gau8_eMMC_SectorBuf, (U32)gau8_RBuf, (U32)gau8_WBuf);
    #endif
    // ===============================================
    //timer test
    ///*
    eMMC_debug(0, 1, "1s\r\n");
    eMMC_hw_timer_delay(HW_TIMER_DELAY_1s);
    eMMC_debug(0, 1, "1s\r\n");
    eMMC_hw_timer_delay(HW_TIMER_DELAY_1s);
    eMMC_debug(0, 1, "1s\r\n");
    eMMC_hw_timer_delay(HW_TIMER_DELAY_1s);
    //*/
    // ===============================================
    #if defined(BOOT_MODE_TEST) && BOOT_MODE_TEST
    u32_err = eMMCTest_BootMode();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC boot test fail: %Xh \r\n", u32_err);
        return u32_err;
    }
    eMMC_debug(0,1,"[eMMC boot test ok] \r\n");    
    while(1);
    #endif

    
    #if 1
    u32_err = eMMC_Init();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_Init fail: %Xh \r\n", u32_err);
        return u32_err;
    }
 
    eMMC_debug(0,1,"[eMMC_Init ok] \r\n");
    #endif

    #if defined(BOOT_SRAM_DMA_TEST) && BOOT_SRAM_DMA_TEST
    
    u32_err = eMMCTest_SingleBlkWRC_SRAM(0, u32_test_pattern[0]);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMCTest_SingleBlkWRC_MIU fail: %Xh \r\n", u32_err);
        return u32_err;
    }
    #endif

    #if 0
    eMMC_debug(0,0,"[eMMC_Timer start] \r\n");
    REG_FCIE(FCIE_BIST_PATTERN0)=0;
    while(1)
    {        
        REG_FCIE(FCIE_BIST_PATTERN0)=REG_FCIE(FCIE_BIST_PATTERN0)+1;
        if(REG_FCIE(FCIE_BIST_PATTERN0)==0xf000)
            break;        
    }
    eMMC_debug(0,0,"[eMMC_Timer done] \r\n");
    #endif
    #if 0
    g_eMMCDrv.u8_DefaultBusMode = u8_BusWidth[2];
    u32_err = eMMC_SetBusWidth(g_eMMCDrv.u8_DefaultBusMode, 1);
    if(eMMC_ST_SUCCESS != u32_err)
        while(1);
    u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);
    if(eMMC_ST_SUCCESS != u32_err)
        while(1);
    
    eMMC_pads_switch(FCIE_eMMC_DDR_8BIT_MACRO);
    
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC IP DDR 8-bit macro, Bus Width%d\033[m\r\n\r\n",
               g_eMMCDrv.u8_DefaultBusMode);

    
    eMMC_FCIE_DetectDDRTiming();
    #endif

    // ===============================================
    #if defined(WRITE_TEST_BOOT_CODE) && WRITE_TEST_BOOT_CODE
    eMMC_debug(0, 0, "\033[7;36mWrite Boot Partition\033[m\r\n");
    u32_err = eMMC_WriteBootPart(gau8_BootImageOri, sizeof(gau8_BootImageOri), 0, 1);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_WriteBootPart fail: %Xh \r\n", u32_err);
        //goto LABEL_IP_VERIFY_ERROR;
    }
    else
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "[eMMC_WriteBootPart ok] \r\n");

    while(1);
    #endif

    #if defined(WRITE_TEST_SQUASHFS_IMAGE) && WRITE_TEST_SQUASHFS_IMAGE
    eMMC_debug(0, 0, "Write test SQUASHFS image\r\n");
    u32_err = eMMC_CMD25_MIU(0x400000, squashfs_test_img, squashfs_test_img_len/512);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, write test SQUASHFS image fail: %Xh \r\n", u32_err);
        //goto LABEL_IP_VERIFY_ERROR;
    }

    u32_err = eMMC_CMD18_MIU(0x400000, gau8_RBuf, squashfs_test_img_len/512);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, read back test SQUASHFS image fail: %Xh \r\n", u32_err);
        //goto LABEL_IP_VERIFY_ERROR;
    }

    for(i=0; i<squashfs_test_img_len; i++)
    {
        if( squashfs_test_img[i] != gau8_RBuf[i] )
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, check test SQUASHFS image fail @ %d\r\n", i);
            while(1);
        }
    }

    //eMMC_dump_mem(gau8_RBuf, squashfs_test_img_len);

    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "[Write test SQUASHFS image ok] \r\n");

    //while(1);
    #endif

    #if defined(FCIE_REGRESSION_TEST) && FCIE_REGRESSION_TEST
    while(1)
    {
    #endif
#if 1
    eMMC_ResetReadyFlag();        
    u32_err = eMMC_Init();
    
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_Init fail: %Xh \r\n", u32_err);
        return u32_err;
    }
    
    //eMMC_debug(0,1,"[eMMC_Init ok] \r\n");
#endif

    // ===============================================
    #if defined(FCIE_GPIO_PAD_VERIFY) && FCIE_GPIO_PAD_VERIFY   // GPIO: BYPASS
    for(u8_i = 0; u8_i < 3; u8_i ++)
    {
        u32_err = eMMC_SetBusWidth(u8_BusWidth[u8_i], 0);
        if(eMMC_ST_SUCCESS != u32_err)
            while(1);
        u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);
        if(eMMC_ST_SUCCESS != u32_err)
            while(1);

        eMMC_pads_switch(FCIE_eMMC_BYPASS);

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC IP GPIO pad: %s Mode Bus Width: %d\033[m\r\n\r\n",
                    (g_eMMCDrv.u8_PadType == FCIE_eMMC_BYPASS) ? "BYPASS" : "SDR(GPIO)",
                    u8_BusWidth[u8_i]);

        for(i=0; i<12; i++)
        {
            eMMC_IPVerify_Main_Ex(u32_test_pattern[i]);
        }

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "normal pattern test ok\r\n\r\n");

        #if !(defined(ENABLE_eMMC_RIU_MODE) && ENABLE_eMMC_RIU_MODE)
        for(i=0; i<12; i++)
        {
            eMMC_IPVerify_Main_Sg_Ex(u32_test_pattern[i]);
        }

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "sglist pattern test ok\r\n\r\n");
        #endif

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC IP GPIO pad: %s Mode Bus Width:%d Verify ok\033[m\r\n\r\n",
                       (g_eMMCDrv.u8_PadType == FCIE_eMMC_BYPASS) ? "BYPASS" : "SDR(GPIO)",
                       u8_BusWidth[u8_i]);
    }

    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;34meMMC IP GPIO pad BYPASS Mode all Verify ok\033[m\r\n\r\n");
    #endif

    #if 1
    eMMC_ResetReadyFlag();
    u32_err = eMMC_Init();
    
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_Init fail: %Xh \r\n", u32_err);
        return u32_err;
    }
    
    //eMMC_debug(0,1,"[eMMC_Init ok] \r\n");
    #endif

    #if defined(FCIE_SDR_FLASH_MACRO8_VERIFY) && FCIE_SDR_FLASH_MACRO8_VERIFY // Flash macro
    for(u8_i = 1; u8_i < 3; u8_i ++)
    {
        u32_err = eMMC_SetBusWidth(u8_BusWidth[u8_i], 0);
        if(eMMC_ST_SUCCESS != u32_err)
            while(1);
        u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);
        if(eMMC_ST_SUCCESS != u32_err)
            while(1);

        eMMC_pads_switch(FCIE_eMMC_SDR);

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC IP SDR 8-bit macro, Bus Width%d\033[m\r\n\r\n",
                   u8_BusWidth[u8_i]);

        for(i=0; i<12; i++)
        {
            eMMC_IPVerify_Main_Ex(u32_test_pattern[i]);
        }

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "normal pattern test ok\r\n\r\n");
        #if !(defined(ENABLE_eMMC_RIU_MODE) && ENABLE_eMMC_RIU_MODE)
        for(i=0; i<12; i++)
        {
            eMMC_IPVerify_Main_Sg_Ex(u32_test_pattern[i]);
        }
        #endif
        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "sglist pattern test ok\r\n\r\n");

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC IP SDR 8-bit macro, Bus Width%d verify ok\033[m\r\n\r\n",
                   u8_BusWidth[u8_i]);
    }
    
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;34meMMC IP SDR Mode All Verify ok\033[m\r\n\r\n");
    #endif

    #if 1
    eMMC_ResetReadyFlag();
    u32_err = eMMC_Init();
    
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_Init fail: %Xh \r\n", u32_err);
        return u32_err;
    }
    
    //eMMC_debug(0,1,"[eMMC_Init ok] \r\n");
    #endif

    #if defined(FCIE_DDR_FLASH_MACRO8_VERIFY) && FCIE_DDR_FLASH_MACRO8_VERIFY // Flash macro
    for(u8_i = 1; u8_i < 3; u8_i ++)
    {
        u32_err = eMMC_SetBusWidth(u8_BusWidth[u8_i], 1);
        if(eMMC_ST_SUCCESS != u32_err)
            while(1);
        u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);
        if(eMMC_ST_SUCCESS != u32_err)
            while(1);

        eMMC_pads_switch(FCIE_eMMC_DDR_8BIT_MACRO);

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC IP DDR 8-bit macro, Bus Width%d\033[m\r\n\r\n",
                   u8_BusWidth[u8_i]);

        for(i=0; i<12; i++)
        {
            eMMC_debug(0, 0, "pattern:0x%x\r\n\r\n",u32_test_pattern[i]);
            eMMC_IPVerify_Main_Ex(u32_test_pattern[i]);
        }

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "normal pattern test ok\r\n\r\n");
        #if !(defined(ENABLE_eMMC_RIU_MODE) && ENABLE_eMMC_RIU_MODE)
        for(i=0; i<12; i++)
        {
            eMMC_IPVerify_Main_Sg_Ex(u32_test_pattern[i]);
        }
        #endif
        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "sglist pattern test ok\r\n\r\n");

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC IP DDR 8-bit macro, Bus Width%d verify ok\033[m\r\n\r\n",
                   u8_BusWidth[u8_i]);
    }
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;34meMMC IP DDR Mode All Verify ok\033[m\r\n\r\n");
    #endif
#if 1
    eMMC_ResetReadyFlag();
    u32_err = eMMC_Init();
    
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_Init fail: %Xh \r\n", u32_err);
        return u32_err;
    }

    //eMMC_debug(0,1,"[eMMC_Init ok] \r\n");
#endif

    #if defined(FCIE_HS200_FLASH_MACRO32_VERIFY) && FCIE_HS200_FLASH_MACRO32_VERIFY // Flash macro
    for(u8_i = 1; u8_i < 3; u8_i ++)
    {        
        u32_err = eMMC_SetBusWidth(u8_BusWidth[u8_i], 0);
        if(eMMC_ST_SUCCESS != u32_err)
            while(1);
        u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HS200);
        if(eMMC_ST_SUCCESS != u32_err)
            while(1);

        eMMC_pads_switch(FCIE_eMMC_HS200);

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC IP HS200 32-bit macro, Bus Width%d\033[m\r\n\r\n",
                   u8_BusWidth[u8_i]);

        for(i=0; i<12; i++)
        {
            eMMC_IPVerify_Main_Ex(u32_test_pattern[i]);
        }

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "normal pattern test ok\r\n\r\n");
        #if !(defined(ENABLE_eMMC_RIU_MODE) && ENABLE_eMMC_RIU_MODE)
        for(i=0; i<12; i++)
        {
            eMMC_IPVerify_Main_Sg_Ex(u32_test_pattern[i]);
        }
        #endif
        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "sglist pattern test ok\r\n\r\n");

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC IP HS200 32-bit macro, Bus Width%d verify ok\033[m\r\n\r\n",
                   u8_BusWidth[u8_i]);
    }
    
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;34meMMC IP HS200 Mode All Verify ok\033[m\r\n\r\n");
    #endif

#if 1
    eMMC_ResetReadyFlag();
    u32_err = eMMC_Init();
    
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_Init fail: %Xh \r\n", u32_err);
        return u32_err;
    }
    
    //eMMC_debug(0,1,"[eMMC_Init ok] \r\n");
#endif

    #if defined(FCIE_HS400_FLASH_MACRO32_VERIFY) && FCIE_HS400_FLASH_MACRO32_VERIFY // Flash macro
    for(u8_i = 2; u8_i < 3; u8_i ++)
    {        
        u32_err = eMMC_SetBusWidth(u8_BusWidth[u8_i], 1);

        if(eMMC_ST_SUCCESS != u32_err)
            while(1);

        u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HS400);
        if(eMMC_ST_SUCCESS != u32_err)
            while(1);


        eMMC_pads_switch(FCIE_eMMC_HS400_DS);

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC IP HS400 32-bit macro, Bus Width%d\033[m\r\n\r\n",
                   u8_BusWidth[u8_i]);

        for(i=0; i<12; i++)
        {
            eMMC_IPVerify_Main_Ex(u32_test_pattern[i]);
        }

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "normal pattern test ok\r\n\r\n");
        #if !(defined(ENABLE_eMMC_RIU_MODE) && ENABLE_eMMC_RIU_MODE)
        for(i=0; i<12; i++)
        {
            eMMC_IPVerify_Main_Sg_Ex(u32_test_pattern[i]);
        }
        #endif
        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "sglist pattern test ok\r\n\r\n");

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC IP HS400 32-bit macro, Bus Width%d verify ok\033[m\r\n\r\n",
                   u8_BusWidth[u8_i]);
    }
    
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;34meMMC IP HS400 Mode All Verify ok\033[m\r\n\r\n");
    #endif

    #if ((defined(FCIE_eMMC5_1_FLASH_MACRO32_VERIFY) && FCIE_eMMC5_1_FLASH_MACRO32_VERIFY) && (defined(ENABLE_AFIFO) && ENABLE_AFIFO))
    for(u8_i = 2; u8_i < 3; u8_i ++)
    {
        #if 1
        u32_err = eMMC_Init();
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_Init fail: %Xh \r\n", u32_err);
            return u32_err;
        }
        
        //eMMC_debug(0,1,"[eMMC_Init ok] \r\n");
        #endif

        u32_err = eMMC_SetBusWidth(u8_BusWidth[u8_i], 2);

        if(eMMC_ST_SUCCESS != u32_err)
            while(1);

        u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HS400);
        if(eMMC_ST_SUCCESS != u32_err)
            while(1);


        eMMC_pads_switch(FCIE_eMMC_5_1_AFIFO);

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC IP HS400 32-bit macro, Bus Width%d\033[m\r\n\r\n",
                   u8_BusWidth[u8_i]);

        for(i=0; i<12; i++)
        {
            eMMC_IPVerify_Main_Ex(u32_test_pattern[i]);
        }

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "normal pattern test ok\r\n\r\n");
    #if !(defined(ENABLE_eMMC_RIU_MODE) && ENABLE_eMMC_RIU_MODE)
        for(i=0; i<12; i++)
        {
            eMMC_IPVerify_Main_Sg_Ex(u32_test_pattern[i]);
        }
    #endif
        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "sglist pattern test ok\r\n\r\n");

        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC IP eMMC 5_1, Bus Width%d verify ok\033[m\r\n\r\n",
                   u8_BusWidth[u8_i]);
    }
    
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;34meMMC IP eMMC 5_1 All Verify ok\033[m\r\n\r\n");
#endif


    #if defined(FCIE_BIST_TEST) && FCIE_BIST_TEST
    {
        eMMC_Init();
        eMMC_pads_switch(FCIE_eMMC_BYPASS);

        for(u8_i=0; u8_i<12; u8_i++)
        {
            eMMCTest_BIST(u8_i);
        }
        eMMC_Init();
        eMMC_pads_switch(FCIE_eMMC_SDR);
        for(u8_i=0; u8_i<12; u8_i++)
        {
            eMMCTest_BIST(u8_i);
        }


        eMMC_debug(0, 0, "eMMC BIST Verify OK\r\n\r\n");
    }

    #endif

    #if defined(FCIE_RSP_SHIFT_TUNING_TEST) && FCIE_RSP_SHIFT_TUNING_TEST
    {
        eMMC_Init();

        g_eMMCDrv.u8_disable_retry = 1;

        for(u8_i=0; u8_i<12; u8_i++)
        {
            u32_err = eMMCTest_RSP_Shift_Tuning(eMMC_TEST_BLK_0, u32_test_pattern[u8_i]);
            if( u32_err ) break;
        }

        if( u32_err )
            eMMC_debug(0, 0, "\033[7;31meMMC RSP Shift Tuning Fail\033[m\r\n\r\n");
        else
            eMMC_debug(0, 0, "\033[7;32meMMC RSP Shift Tuning OK\033[m\r\n\r\n");
    }
    #endif

    #if defined(FCIE_WCRC_SHIFT_TUNING_TEST) && FCIE_WCRC_SHIFT_TUNING_TEST
    {
        eMMC_Init();

        g_eMMCDrv.u8_disable_retry = 1;

        for(u8_i=0; u8_i<12; u8_i++)
        {
            u32_err = eMMCTest_WCRC_Shift_Tuning(eMMC_TEST_BLK_0, u32_test_pattern[u8_i]);
            if( u32_err ) break;
        }

        if( u32_err )
            eMMC_debug(0, 1, "\033[7;31meMMC WCRC Shift Tuning fail\033[m\r\n\r\n");
        else
            eMMC_debug(0, 1, "\033[7;32meMMC WCRC Shift Tuning OK\033[m\r\n\r\n");
    }
    #endif

    #if defined(FCIE_RSTOP_SHIFT_TUNING_TEST) && FCIE_RSTOP_SHIFT_TUNING_TEST
    {
        eMMC_Init();

        g_eMMCDrv.u8_disable_retry = 1;

        for(u8_i=0; u8_i<12; u8_i++)
        {
            u32_err = eMMCTest_RSTOP_Shift_Tuning(eMMC_TEST_BLK_0, u32_test_pattern[u8_i]);
            if( u32_err ) break;
        }

        if( u32_err)
            eMMC_debug(0, 1, "\033[7;31meMMC RStop Shift Tuning fail\033[m\r\n\r\n");
        else
            eMMC_debug(0, 1, "\033[7;32meMMC RStop Shift Tuning OK\033[m\r\n\r\n");
    }
    #endif

    // Test Read CRC error int
    #if defined(FCIE_READ_CRC_ERROR_INT_TEST) && FCIE_READ_CRC_ERROR_INT_TEST
    eMMC_Init();
    g_eMMCDrv.u8_disable_retry = 1;
    eMMC_debug(0, 0, "\033[7;31mTest read CRC error int\033[m\r\n");
    eMMCTest_Read_CRC_Error();
    #endif

    #if defined(FCIE_WRITE_CRC_ERROR_INT_TEST) && FCIE_WRITE_CRC_ERROR_INT_TEST
    // Test Write CRC error int
    eMMC_Init();
    g_eMMCDrv.u8_disable_retry = 1;
    eMMC_debug(0, 0, "\033[7;31mTest write CRC error int\033[m\r\n");
    eMMCTest_Write_CRC_Error();
    #endif

    #if defined(FCIE_WRITE_TIMEOUT_INT_TEST) && FCIE_WRITE_TIMEOUT_INT_TEST
    // Test Write timeout error int
    eMMC_Init();
    g_eMMCDrv.u8_disable_retry = 1;
    eMMC_debug(0, 0, "\033[7;31mTest write timeout int\033[m\r\n");
    eMMCTest_Write_Timeout_Error();
    #endif

    #if defined(FCIE_READ_TIMEOUT_INT_TEST) && FCIE_READ_TIMEOUT_INT_TEST
    // Test read timeout error int
    eMMC_Init();
    g_eMMCDrv.u8_disable_retry = 1;
    eMMC_debug(0, 0, "\033[7;31mTest read timeout int\033[m\r\n");
    eMMCTest_Read_Timeout_Error(); // We have problem again
    #endif

    #if defined(FCIE_CMD_NO_RESPONSE_INT_TEST) && FCIE_CMD_NO_RESPONSE_INT_TEST
    // Test commnad No response int
    eMMC_Init();
    g_eMMCDrv.u8_disable_retry = 1;
    eMMC_debug(0, 0, "\033[7;31mTest commnad no response int\033[m\r\n");
    eMMCTest_Command_No_Response();
    #endif

    #if defined(FCIE_CMD_RSP_CRC_ERROR_INT_TEST) && FCIE_CMD_RSP_CRC_ERROR_INT_TEST
    // Test command response CRC error int
    eMMC_Init();
    g_eMMCDrv.u8_disable_retry = 1;
    eMMC_debug(0, 0, "\033[7;31mTest commnad response CRC int\033[m\r\n");
    eMMCTest_Command_Response_CRC_Error();
    #endif

    #if  defined(FCIE_POWER_SAVING_TEST) && FCIE_POWER_SAVING_TEST
    eMMC_hw_timer_delay(HW_TIMER_DELAY_1s);
    eMMC_debug(0, 0, "COUNT1\r\n\r\n");
    eMMC_hw_timer_delay(HW_TIMER_DELAY_1s);
    eMMC_debug(0, 0, "COUNT2\r\n\r\n");    
    eMMC_Prepare_Power_Saving_Mode_Queue();
    #endif

    #if 0//defined(FCIE_ZDEC_TEST) && FCIE_ZDEC_TEST

    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;33meMMC IP ZDEC test\033[m\r\n\r\n");

    {
    U32 u32_err;
    U32 u32_i;
    U32 u32_j;
    U32 u32_blk_addr;
    U32 u32_blk_count;
    U32 u32_tmp_blk_count;
    U8  u8_decompress[128*1024];
    int s32_decode_size;
    U32 u32_count = 0;

    SQUASHFS_INFO squashfs_info[5] =
    {
        { 96, 35731, 131072 },
        { 35827, 19864, 70326 },
        { 72094, 29571, 110762 },
        { 107474, 13776, 72656 },
        { 121250, 28458, 106205 },
    };

    for(u32_i=0; u32_i<5; u32_i++)
    {

    u32_blk_addr = 0x400000+(squashfs_info[u32_i].u32_offset>>eMMC_SECTOR_512BYTE_BITS);
    u32_blk_count = ((squashfs_info[u32_i].u32_offset%512)+squashfs_info[u32_i].u32_compressed_size+511)
                    >>eMMC_SECTOR_512BYTE_BITS;
    u32_tmp_blk_count = u32_blk_count;

    memset(gau8_RBuf, 0, u32_blk_count<<eMMC_SECTOR_512BYTE_BITS);
    memset(pSG_st, 0, sizeof(struct _scatterlist)*MAX_SCATTERLIST_COUNT);
    memset(p_AdmaDesc_st, 0, sizeof(struct _AdmaDescriptor)*MAX_SCATTERLIST_COUNT);
    memset(p_ZDecDesc_st, 0, sizeof(struct _ZDecDescriptor)*MAX_SCATTERLIST_COUNT);
    memset(p_ZDecOutput_st, 0, sizeof(struct _ZDecDescriptor)*MAX_SCATTERLIST_COUNT);
    memset(u8_decompress, 0, 128*1024);

    u32_j=0;
    while( u32_tmp_blk_count >= 8 )
    {
        pSG_st[u32_j].u32_dma_address = (U32)(gau8_RBuf+(u32_j*4096));
        pSG_st[u32_j].u32_length = 4096;
        u32_tmp_blk_count -= 8;
        u32_j++;
    }

    pSG_st[u32_j].u32_dma_address = (U32)(gau8_RBuf+(u32_j*4096));
    pSG_st[u32_j].u32_length = u32_tmp_blk_count*512;
    u32_j++;

    p_ZDecOutput_st[0].u32_Address = u8_decompress;
    p_ZDecOutput_st[0].u32_JobCnt = (squashfs_info[u32_i].u32_uncompressed_size+511)/512;
    p_ZDecOutput_st[0].u32_MiuSel = 0;
    p_ZDecOutput_st[0].u32_End = 1;

    #if defined(FCIE_ZDEC_TALK_TEST) && FCIE_ZDEC_TALK_TEST
    // Setup ZDEC
    zdec_top_fpga_init(ZMODE_DEC_SCATTER_EMMC,
                       0,
                       squashfs_info[u32_i].u32_compressed_size,
                       1,
                       (squashfs_info[u32_i].u32_offset%512)+2, // ZLIB offset = 2 bytes
                       0,                                       // no bit offset
                       p_ZDecOutput_st,
                       sizeof(struct _ZDecDescriptor)*1,
                       0,
                       0);
    #else
     // Setup ZDEC
    zdec_top_fpga_init(ZMODE_DEC_SCATTER_EMMC,
                       0,
                       squashfs_info[u32_i].u32_compressed_size,
                       0,
                       (squashfs_info[u32_i].u32_offset%512)+2, // ZLIB offset = 2 bytes
                       0,                                       // no bit offset
                       p_ZDecOutput_st,
                       sizeof(struct _ZDecDescriptor)*1,
                       0,
                       0);
    #endif

    //read scatterlist

    u32_err = eMMCTest_SgR_MIU(u32_blk_addr);
    if(u32_err != eMMC_ST_SUCCESS)
    {
        eMMC_debug(0,1,"Err, SgWRC_MIU R, %Xh \r\n", u32_err);
        return u32_err;
    }

    //eMMC_dump_mem(gau8_RBuf, u32_blk_count<<eMMC_SECTOR_512BYTE_BITS);

    u32_err = eMMC_ComapreData(
        (U8*)(squashfs_test_img+((squashfs_info[u32_i].u32_offset>>eMMC_SECTOR_512BYTE_BITS)<<eMMC_SECTOR_512BYTE_BITS)),
        gau8_RBuf,
        u32_blk_count<<eMMC_SECTOR_512BYTE_BITS);

    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, eMMCTest_SgR_MIU, %u, %Xh \r\n", u32_j, u32_err);
        return u32_err;
    }

    //eMMC_translate_DMA_address_Ex(u8_decompress, squashfs_info[u32_i].u32_uncompressed_size);
    eMMC_translate_DMA_address_Ex(u8_decompress, 128*1024, READ_FROM_eMMC);

    #if !(defined(FCIE_ZDEC_TALK_TEST) && FCIE_ZDEC_TALK_TEST)
    zdec_top_fpga_feed(1, // last
                       p_ZDecDesc_st,
                       sizeof(struct _ZDecDescriptor) * u32_j);
    //eMMC_debug(0, 1, "zdec_top_fpga_feed sadr=%p\r\n", p_ZDecDesc_st);
    //eMMC_debug(0, 1, "zdec_top_fpga_feed size=%X\r\n", sizeof(struct _ZDecDescriptor) * u32_i);
    #endif

    eMMC_debug(0, 1, "Wait decoding\r\n");
    // wait for decode done
    u32_count = 0;
    while (-1 == (s32_decode_size = zdec_top_fpga_wait_decode_done()))
    {
        eMMC_hw_timer_sleep(1);
        if( ++u32_count > 30 )
        {
            eMMC_debug(0, 1, "Decode timeout!\r\n");
            //eMMC_dump_mem(u8_decompress, u32_uncompressed_size);
            while(1);
        }
    }

    REG_FCIE_SETBIT(FCIE_ZDEC_CTL0, BIT_SD2ZDEC_PTR_CLR);
    eMMC_hw_timer_sleep(1);
    REG_FCIE_CLRBIT(FCIE_ZDEC_CTL0, BIT_SD2ZDEC_PTR_CLR|BIT_ZDEC_EN);

    eMMC_debug(0, 1, "Decode finished\r\n");

    u32_err = eMMC_ComapreData(
        squashfs_test_data[u32_i], u8_decompress, squashfs_info[u32_i].u32_uncompressed_size);

    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, Uncompress data mismatch\r\n");
        eMMC_dump_mem(u8_decompress, squashfs_info[u32_i].u32_uncompressed_size);
        return u32_err;
    }

    eMMC_debug(0, 1, "ZDEC test case %d ok\r\n", u32_i+1);

    }

    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;32meMMC IP ZDEC test ok\033[m\r\n\r\n");

    return eMMC_ST_SUCCESS;
    }

    #endif



    #if 0

    eMMC_debug(0, 1, "Test strange problem....\r\n");
    eMMC_Init();
    eMMC_FCIE_ErrHandler_ReInit();

    #endif

    #if defined(FCIE_REGRESSION_TEST) && FCIE_REGRESSION_TEST
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\033[7;34meMMC IP regression test count %d ok\033[m\r\n\r\n",
               ++u32_regression_test_count);

    #if 0
    u32_err = eMMC_Init();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_Init fail: %Xh \r\n", u32_err);
        return u32_err;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMC_Init ok] \r\n");
    #endif

    }
    #endif

    return eMMC_ST_SUCCESS;
}

U32  eMMC_IPVerify_Main_Sg_Ex(U32 u32_DataPattern)
{
    static U32 u32_StartSector=0, u32_SectorCnt=0, u32_err = 0;

    // make StartSector SectorCnt random
    u32_StartSector = eMMC_TEST_BLK_0;
    u32_SectorCnt = eMMC_TEST_BLK_CNT;

    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "Data Pattern: %08Xh\r\n", u32_DataPattern);

    #if 1
    u32_err = eMMCTest_SgWRC_MIU(u32_StartSector, u32_SectorCnt, u32_DataPattern);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMCTest_SgWRC_MIU fail: %Xh \r\n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMCTest_SgWRC_MIU ok] \r\n");
    #endif

    // ===============================================
    //eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\r\n");
    //eMMC_debug(eMMC_DEBUG_LEVEL, 1, "[OK] \r\n\r\n");
    return eMMC_ST_SUCCESS;

    LABEL_IP_VERIFY_ERROR:
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\r\n\r\n");
    eMMC_debug(eMMC_DEBUG_LEVEL,1,"Total Sec: %Xh, Test: StartSec: %Xh, SecCnt: %Xh \r\n",
            g_eMMCDrv.u32_SEC_COUNT, u32_StartSector, u32_SectorCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "[eMMC IPVerify Fail: %Xh] \r\n\r\n", u32_err);
    eMMC_DumpDriverStatus();
    while(1);
    return u32_err;
}


U32 eMMC_IPVerify_Main_Ex(U32 u32_DataPattern)
{
    static U32 u32_StartSector=0, u32_SectorCnt=0, u32_err;
    //U8 u8_i;

    // make StartSector SectorCnt random
    u32_StartSector = eMMC_TEST_BLK_0;
    u32_SectorCnt = eMMC_TEST_BLK_CNT;
    //u32_SectorCnt++;
    //while(u32_SectorCnt > eMMC_TEST_BLK_CNT)
    //  u32_SectorCnt = 1;

    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "Data Pattern: %08Xh\r\n", u32_DataPattern);



    #if defined(ENABLE_eMMC_RIU_MODE) && ENABLE_eMMC_RIU_MODE
    u32_err = eMMCTest_SingleBlkWRC_RIU(u32_StartSector, u32_DataPattern);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMCTest_SingleBlkWRC_RIU fail: %Xh \r\n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMCTest_SingleBlkWRC_RIU ok] \r\n");
    #else
    u32_err = eMMCTest_SingleBlkWRC_MIU(u32_StartSector, u32_DataPattern);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMCTest_SingleBlkWRC_MIU fail: %Xh \r\n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMCTest_SingleBlkWRC_MIU ok] \r\n");
    #endif

    #if defined(ENABLE_IMI_TEST) && ENABLE_IMI_TEST
    u32_err = eMMCTest_SingleBlkWRC_IMI(u32_StartSector, u32_DataPattern);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMCTest_SingleBlkWRC_IMI fail: %Xh \r\n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMCTest_SingleBlkWRC_IMI ok] \r\n");
    #endif

    #if !(defined(ENABLE_eMMC_RIU_MODE) && ENABLE_eMMC_RIU_MODE)
    u32_err = eMMCTest_MultiBlkWRC_MIU(u32_StartSector, u32_SectorCnt, u32_DataPattern);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMCTest_MultiBlkWRC_MIU fail: %Xh \r\n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMCTest_MultiBlkWRC_MIU ok] \r\n");
    #endif

    // ===============================================
    //eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\r\n");
    //eMMC_debug(eMMC_DEBUG_LEVEL, 1, "[OK] \r\n\r\n");
    return eMMC_ST_SUCCESS;

    LABEL_IP_VERIFY_ERROR:
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\r\n\r\n");
    eMMC_debug(eMMC_DEBUG_LEVEL,1,"Total Sec: %Xh, Test: StartSec: %Xh, SecCnt: %Xh \r\n",
            g_eMMCDrv.u32_SEC_COUNT, u32_StartSector, u32_SectorCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "[eMMC IPVerify Fail: %Xh] \r\n\r\n", u32_err);
    eMMC_DumpDriverStatus();
    while(1);

    return u32_err;
}

#endif
#endif

