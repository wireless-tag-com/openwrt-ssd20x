/*
* eMMC_ip_verify.c- Sigmastar
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
#if defined (UNIFIED_eMMC_DRIVER) && UNIFIED_eMMC_DRIVER

#define DDR_TEST_BLK_CNT        8
#define DDR_TEST_BUFFER_SIZE    (eMMC_SECTOR_512BYTE*DDR_TEST_BLK_CNT)

eMMC_ALIGN0 static U8 gau8_WBuf_DDR[DDR_TEST_BUFFER_SIZE] eMMC_ALIGN1;
eMMC_ALIGN0 static U8 gau8_RBuf_DDR[DDR_TEST_BUFFER_SIZE] eMMC_ALIGN1;

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
                #if 1
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
                #if 1
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
                #if 1
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0xFFFFFFFF;
                break;
                #else
                continue;
                #endif
        }

        #if 1
        u32_err = eMMC_CMD24_MIU(u32_eMMC_Addr, gau8_WBuf_DDR);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            break;
        }
        u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr, gau8_RBuf_DDR);
        if(eMMC_ST_SUCCESS != u32_err)
            break;

        u32_err = eMMC_ComapreData(gau8_WBuf_DDR, gau8_RBuf_DDR, eMMC_SECTOR_512BYTE);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(0,1,"Err, compare fail.single, %Xh \n", u32_err);
            break;
        }
        #endif
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
        //eMMC_debug(0,0,"%s() fail %Xh\n", __FUNCTION__, u32_err);
        //eMMC_FCIE_ErrHandler_ReInit();
        eMMC_debug(1,1,"data pattern %u: %02X%02X%02X%02Xh \n\n",
            u32_j, gau8_WBuf_DDR[3], gau8_WBuf_DDR[2], gau8_WBuf_DDR[1], gau8_WBuf_DDR[0]);
    }
    //eMMC_debug(0,0,"%s() return %Xh\n", __FUNCTION__, u32_err);
    return u32_err;
}


//=============================================================
#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING


// can use for RF team test
#ifndef IP_FCIE_VERSION_5
U32 eMMCTest_KeepR_TestDDR(U32 u32_LoopCnt)
{
    U32 u32_err;
    U32 u32_i, u32_j, u32_k;
    U32 u32_BlkCnt, u32_eMMC_Addr;

    u32_eMMC_Addr = eMMC_TEST_BLK_0;

    u32_BlkCnt = eMMC_TEST_BLK_CNT;
    u32_BlkCnt = u32_BlkCnt > DDR_TEST_BLK_CNT ? DDR_TEST_BLK_CNT : u32_BlkCnt;

    if(8 != u32_BlkCnt)
    {
        eMMC_debug(0,1,"Blk count needs to be 8 \n");
        while(1);
    }

    for(u32_j=0; u32_j<u32_BlkCnt; u32_j++){
        // init data pattern
        switch(u32_j)
        {
            case 0: // increase
                for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE; u32_i++)
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i] = u32_i+1;
                break;
            case 1: // 0xF00F
                for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE/2; u32_i++)
                {
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i]=0xF0;
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i+1]=0x0F;
                }
                break;
            case 2: // 0xFF00
                for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE/2; u32_i++)
                {
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i]=0xFF;
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i+1]=0;
                }
                break;
            case 3: // 0x5AA5
                for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE/2; u32_i++)
                {
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i]=0x5A;
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i+1]=0xA5;
                }
                break;
            case 4: // 0x55AA
                for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE/2; u32_i++)
                {
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i]=0x55;
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i+1]=0xAA;
                }
                break;
            case 5: // 0x5A5A
                for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE/2; u32_i++)
                {
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i]=0x5A;
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i+1]=0x5A;
                }
                break;
            case 6: // 0x0000
                for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE/2; u32_i++)
                {
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i]=0;
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i+1]=0;
                }
                break;
            case 7: // 0xFFFF
                for(u32_i=0; u32_i<eMMC_SECTOR_512BYTE/2; u32_i++)
                {
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i]=0xFF;
                    gau8_WBuf_DDR[(u32_j<<eMMC_SECTOR_512BYTE_BITS)+u32_i+1]=0xFF;
                }
                break;

        }

        u32_err = eMMC_CMD25_MIU(u32_eMMC_Addr, gau8_WBuf_DDR, u32_BlkCnt);
        if(eMMC_ST_SUCCESS != u32_err)
            break;
    }


    for(u32_k=0; u32_k<u32_LoopCnt; u32_k++){

        u32_err = eMMC_CMD18_MIU(u32_eMMC_Addr, gau8_RBuf_DDR, u32_BlkCnt/2);
        if(eMMC_ST_SUCCESS != u32_err)
            break;

        u32_err = eMMC_ComapreData(gau8_WBuf_DDR, gau8_RBuf_DDR, (u32_BlkCnt/2)<<eMMC_SECTOR_512BYTE_BITS);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(0,1,"Err 1, compare fail, %Xh \n", u32_err);
            break;
        }

        for(u32_j=0; u32_j<u32_BlkCnt/2; u32_j++)
        {
            u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr+u32_BlkCnt/2+u32_j,
                gau8_RBuf_DDR+ ((u32_BlkCnt/2+u32_j)<<eMMC_SECTOR_512BYTE_BITS));
            if(eMMC_ST_SUCCESS != u32_err)
                break;

            u32_err = eMMC_ComapreData(
                gau8_WBuf_DDR+ ((u32_BlkCnt/2+u32_j)<<eMMC_SECTOR_512BYTE_BITS),
                gau8_RBuf_DDR+ ((u32_BlkCnt/2+u32_j)<<eMMC_SECTOR_512BYTE_BITS),
                eMMC_SECTOR_512BYTE);
            if(eMMC_ST_SUCCESS != u32_err)
            {
                eMMC_debug(0,1,"Err 2, compare fail, %Xh \n", u32_err);
                break;
            }
        }
    }

    return u32_err;
}
#endif

#endif


//=============================================================
#if defined(IF_IP_VERIFY) && IF_IP_VERIFY

#define TEST_BUFFER_SIZE    0x100000 // 1MB
eMMC_ALIGN0 static U8 gau8_WBuf[TEST_BUFFER_SIZE] eMMC_ALIGN1;
eMMC_ALIGN0 static U8 gau8_RBuf[TEST_BUFFER_SIZE] eMMC_ALIGN1;

U32 eMMC_BootMode(void);
#define eMMC_PATTERN_00000000    0x00000000 // smooth
#define eMMC_PATTERN_FFFFFFFF    0xFFFFFFFF // smooth
#define eMMC_PATTERN_000000FF    0x000000FF // board
#define eMMC_PATTERN_0000FFFF    0x0000FFFF // board
#define eMMC_PATTERN_00FF00FF    0x00FF00FF // board
#define eMMC_PATTERN_AA55AA55    0xAA55AA55 // board

U32  eMMCTest_SingleBlkWRC_RIU(U32 u32_eMMC_Addr, U32 u32_DataPattern);
U32  eMMCTest_SingleBlkWRC_MIU(U32 u32_eMMC_Addr, U32 u32_DataPattern);
U32  eMMCTest_SingleBlkWRC_MIU_Ex(U32 u32_eMMC_Addr, U8 *pu8_W, U8 *pu8_R);

U32  eMMCTest_SgWRC_MIU(U32 u32_eMMC_Addr, U16 u16_BlkCnt, U32 u32_DataPattern);
U32  eMMCTest_SgW_MIU(U32 u32_eMMC_Addr);
U32  eMMCTest_SgR_MIU(U32 u32_eMMC_Addr);
U32  eMMCTest_MultiBlkWRC_MIU(U32 u32_eMMC_Addr, U16 u16_BlkCnt, U32 u32_DataPattern);
U32  eMMC_IPVerify_Main_Ex(U32 u32_DataPattern);
U32  eMMC_IPVerify_Main_Sg_Ex(U32 u32_DataPattern);
U32  eMMC_IPVerify_Main_API_Ex(U32 u32_DataPattern);

void eMMCTest_DownCount(U32 u32_Sec);

#define MAX_SCATTERLIST_COUNT 0x10
#define eMMC_GENERIC_WAIT_TIME  (HW_TIMER_DELAY_1s*2) // 2 sec

static struct _scatterlist {
    U32 u32_length;
    U32 u32_dma_address;

}pSG_st[MAX_SCATTERLIST_COUNT];


U32 eMMCTest_SingleBlkWRC_RIU(U32 u32_eMMC_Addr, U32 u32_DataPattern)
{
    U32 u32_err;
    U32 u32_i, *pu32_W=(U32*)gau8_WBuf, *pu32_R=(U32*)gau8_RBuf;

    // init data pattern
    for(u32_i=0; u32_i<FCIE_CIFD_BYTE_CNT>>2; u32_i++)
    {
        pu32_W[u32_i] = u32_DataPattern;
        pu32_R[u32_i] = ~pu32_W[u32_i];
    }

    u32_err = eMMC_CMD24_CIFD(u32_eMMC_Addr, gau8_WBuf);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC W, %Xh \n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_CMD17_CIFD(u32_eMMC_Addr, gau8_RBuf);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC R, %Xh \n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_ComapreData(gau8_WBuf, gau8_RBuf, FCIE_CIFD_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC C, %Xh \n", u32_err);
        return u32_err;
    }

    return u32_err;
}

extern U32 gu32_DMAAddr;
U32 eMMCTest_SingleBlkWRC_MIU(U32 u32_eMMC_Addr, U32 u32_DataPattern)
{
    volatile U32 u32_i, *pu32_W=(U32*)gau8_WBuf, *pu32_R=(U32*)gau8_RBuf;

    // init data pattern
    for(u32_i=0; u32_i<FCIE_CIFD_BYTE_CNT>>2; u32_i++)
    {
        pu32_W[u32_i] = u32_DataPattern;
        pu32_R[u32_i] = ~pu32_W[u32_i];
    }

    return eMMCTest_SingleBlkWRC_MIU_Ex(u32_eMMC_Addr, (U8*)pu32_W, (U8*)pu32_R);
}


U32 eMMCTest_SingleBlkWRC_MIU_Ex(U32 u32_eMMC_Addr, U8 *pu8_W, U8 *pu8_R)
{
    U32 u32_err;

    u32_err = eMMC_CMD24_MIU(u32_eMMC_Addr, pu8_W);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC W, %Xh \n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_CMD17_MIU(u32_eMMC_Addr, pu8_R);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC R, %Xh \n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_ComapreData(pu8_W, pu8_R, eMMC_SECTOR_BYTECNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, SingleBlkWRC C, %Xh \n", u32_err);
        return u32_err;
    }

    return u32_err;
}


#define eMMC_RETRY_CNT  10

U32  eMMCTest_SgW_MIU(U32 u32_eMMC_Addr)
{
    U16 u16_mode, u16_ctrl=0, u16_reg;
    U32 u32_dmaaddr,u32_dma_addr,u32_dmalen,u32_arg;
    U32 u32_err,u32_err_12=eMMC_ST_SUCCESS,u32_i;
    U8  u8_retry_cnt=0;

    LABEL_RETRY:
    eMMC_FCIE_ClearEvents();
    // -------------------------------
    // send cmd
    u16_ctrl = (BIT_SD_CMD_EN | BIT_SD_RSP_EN);
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;

    // -------------------------------

    u32_arg =  u32_eMMC_Addr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 25, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cnt++ < eMMC_RETRY_CNT)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: CMD fail: %Xh \n", u32_err);
            eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_RETRY;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD fail: %Xh \n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cnt++ < eMMC_RETRY_CNT)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: R1 fail: %Xh \n", u32_err);
            eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_RETRY;
        }
        u32_err = eMMC_ST_ERR_CMD25_CHK_R1;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: R1 fail: %Xh\n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    if(g_eMMCDrv.u32_DrvFlag & (DRV_FLAG_DDR_MODE|DRV_FLAG_SPEED_HS200) )
    {
        REG_FCIE_W(FCIE_TOGGLE_CNT, (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_SPEED_HS200) ? TOGGLE_CNT_512_CLK_W : TOGGLE_CNT_256_CLK_W);
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
    }

    for(u32_i=0; u32_i< MAX_SCATTERLIST_COUNT; u32_i++)
    {
        u32_dmaaddr = pSG_st[u32_i].u32_dma_address;
        u32_dmalen  = pSG_st[u32_i].u32_length;
        u32_dma_addr = eMMC_translate_DMA_address_Ex(u32_dmaaddr, u32_dmalen);

        eMMC_FCIE_ClearEvents_Reg0();
        u32_err = eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            if(u8_retry_cnt++ < eMMC_RETRY_CNT)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: Wait D0 H TO: %Xh \n", u32_err);
                eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_RETRY;
            }
            u32_err = eMMC_ST_ERR_CMD25_WAIT_D0H;
            goto LABEL_END;
        }

        REG_FCIE_W(FCIE_JOB_BL_CNT, u32_dmalen>>eMMC_SECTOR_512BYTE_BITS);
        REG_FCIE_W(FCIE_SDIO_ADDR0, u32_dma_addr & 0xFFFF);
        REG_FCIE_W(FCIE_SDIO_ADDR1, u32_dma_addr >> 16);

        REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);
        if(u32_i ==0)
        {
            u32_err = eMMC_FCIE_FifoClkRdy(BIT_DMA_DIR_W);

            if(eMMC_ST_SUCCESS != u32_err)
            {
                if(u8_retry_cnt++ < eMMC_RETRY_CNT)
                {
                    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: FifoClkRdy fail: %Xh \n", u32_err);
                    eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
                    goto LABEL_RETRY;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC Err: FifoClkRdy fail\n");
                eMMC_FCIE_ErrHandler_Stop();
                goto LABEL_END;
            }
        }
        REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);
        REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W);
        #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
        REG_FCIE_W(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);
        #endif
        //u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_CARD_DMA_END|BIT_MIU_LAST_DONE, eMMC_GENERIC_WAIT_TIME);
        u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_CARD_DMA_END, eMMC_GENERIC_WAIT_TIME);
        if(u32_err!= eMMC_ST_SUCCESS)
        {
            if(u8_retry_cnt++ < eMMC_RETRY_CNT)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: event TO: %Xh \n", u32_err);
                eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_RETRY;
            }
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC Err: event TO \n");
            //eMMC_die("\n");
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_ERROR_RETRY;
            break;
        }
        //eMMC_hw_timer_delay(1000000);
    }

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if((u16_reg & BIT_SD_FCIE_ERR_FLAGS)||(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_ERROR_RETRY))
    {
        if(u8_retry_cnt++ < eMMC_RETRY_CNT)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: Reg.12h: %Xh fail: %Xh \n", u16_reg, u32_err);
            eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_RETRY;
        }
        u32_err = eMMC_ST_ERR_CMD25;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0,
            "eMMC Err: Reg.12h: %04Xh, Err: %Xh\n", u16_reg, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:
    u32_err_12 = eMMC_CMD12(g_eMMCDrv.u16_RCA);
    if(eMMC_ST_SUCCESS != u32_err_12 && u8_retry_cnt++ < eMMC_RETRY_CNT)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: CMD12 fail: %Xh \n", u32_err);
        eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
        goto LABEL_RETRY;
    }

    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
    return u32_err ? u32_err : u32_err_12;
}


U32  eMMCTest_SgR_MIU(U32 u32_eMMC_Addr)
{
    U16 u16_mode, u16_ctrl=0, u16_reg;
    U32 u32_dmaaddr,u32_dma_addr,u32_dmalen,u32_arg;
    U32 u32_err,u32_err_12=eMMC_ST_SUCCESS,u32_i;
    U8  u8_retry_cnt=0;

    LABEL_RETRY:
    eMMC_FCIE_ClearEvents();

    u16_ctrl = BIT_SD_DAT_EN;
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    u16_mode |= BIT_SD_DMA_R_CLK_STOP;

    REG_FCIE_W(FCIE_SD_MODE, u16_mode);

    if(g_eMMCDrv.u32_DrvFlag & (DRV_FLAG_DDR_MODE|DRV_FLAG_SPEED_HS200) )
    {
        REG_FCIE_W(FCIE_TOGGLE_CNT, (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_SPEED_HS200) ? TOGGLE_CNT_512_CLK_R : TOGGLE_CNT_256_CLK_R);
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
        REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
        eMMC_hw_timer_delay(TIME_WAIT_FCIE_RST_TOGGLE_CNT); // Brian needs 2T
        REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
    }

    u32_dmaaddr = pSG_st[0].u32_dma_address;
    u32_dmalen  = pSG_st[0].u32_length;
    u32_dma_addr = eMMC_translate_DMA_address_Ex(u32_dmaaddr, u32_dmalen);

    REG_FCIE_W(FCIE_JOB_BL_CNT, u32_dmalen>>eMMC_SECTOR_512BYTE_BITS);
    REG_FCIE_W(FCIE_SDIO_ADDR0, u32_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_SDIO_ADDR1, u32_dma_addr >> 16);

    REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);

    u32_err = eMMC_FCIE_FifoClkRdy(0);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cnt++ < eMMC_RETRY_CNT)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: FifoClkRdy fail: %Xh \n", u32_err);
            eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_RETRY;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC Err: FifoClkRdy fail\n");
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }
    REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);

    // -------------------------------
    // send cmd
    u16_ctrl |= (BIT_SD_CMD_EN | BIT_SD_RSP_EN);

    // -------------------------------
    u32_arg =  u32_eMMC_Addr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 18, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cnt++ < eMMC_RETRY_CNT)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: CMD fail: %Xh \n", u32_err);
            eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_RETRY;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0, "eMMC Err: CMD fail %Xh \n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }
    else
    {
        #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
        REG_FCIE_W(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);
        #endif

        u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
            BIT_MIU_LAST_DONE, eMMC_GENERIC_WAIT_TIME);
        if(u32_err!= eMMC_ST_SUCCESS)
        {
            if(u8_retry_cnt++ < eMMC_RETRY_CNT)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: event TO.0: %Xh \n", u32_err);
                eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_RETRY;
            }
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC Err: event TO.0 \n");
            //eMMC_die("\n");
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_ERROR_RETRY;
            return u32_err;
        }
        REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
        if((eMMC_ST_SUCCESS != u32_err) || (u16_reg & BIT_SD_FCIE_ERR_FLAGS))
        {
            if(u8_retry_cnt++ < eMMC_RETRY_CNT)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: Reg.12h: %Xh fail: %Xh \n", u16_reg, u32_err);
                eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_RETRY;
            }
            u32_err = eMMC_ST_ERR_CMD18;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0,
                "eMMC Err: Reg.12h: %Xh fail: %Xh\n", u16_reg, u32_err);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }

        for(u32_i=1; u32_i< MAX_SCATTERLIST_COUNT; u32_i++)
        {
            eMMC_FCIE_ClearEvents_Reg0();

            u32_dmaaddr = pSG_st[u32_i].u32_dma_address;
            u32_dmalen  = pSG_st[u32_i].u32_length;
            u32_dma_addr = eMMC_translate_DMA_address_Ex(u32_dmaaddr, u32_dmalen);

            REG_FCIE_W(FCIE_JOB_BL_CNT, u32_dmalen>>9);
            REG_FCIE_W(FCIE_SDIO_ADDR0, u32_dma_addr & 0xFFFF);
            REG_FCIE_W(FCIE_SDIO_ADDR1, u32_dma_addr >> 16);
            REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);
            REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN);
            #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
            REG_FCIE_W(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);
            #endif
            u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_MIU_LAST_DONE, TIME_WAIT_n_BLK_END*(1+(u32_dmalen>>24)));
            if(u32_err!= eMMC_ST_SUCCESS)
            {
                if(u8_retry_cnt++ < eMMC_RETRY_CNT)
                {
                    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: event TO.1: %Xh \n", u32_err);
                    eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
                    goto LABEL_RETRY;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC Err: event TO.1 \n");
                //eMMC_die("\n");
                g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_ERROR_RETRY;
                break;
            }
        }
    }

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if((u16_reg & BIT_SD_FCIE_ERR_FLAGS)||(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_ERROR_RETRY))
    {
        if(u8_retry_cnt++ < eMMC_RETRY_CNT)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: Reg.12h: %Xh fail: %Xh \n", u16_reg, u32_err);
            eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_RETRY;
        }
        u32_err = eMMC_ST_ERR_CMD18;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: Reg.12h: %Xh fail: %Xh\n", u16_reg, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cnt++ < eMMC_RETRY_CNT)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: R1 fail: %Xh \n", u32_err);
            eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_RETRY;
        }
        u32_err = eMMC_ST_ERR_CMD18;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: R1 fail: %Xh\n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:
    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);

    u32_err_12 = eMMC_CMD12(g_eMMCDrv.u16_RCA);
    if(eMMC_ST_SUCCESS != u32_err_12 && u8_retry_cnt++ < eMMC_RETRY_CNT)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "eMMC Warn: CMD12 fail: %Xh \n", u32_err);
        eMMC_FCIE_ErrHandler_ReInit();  eMMC_FCIE_ErrHandler_Retry();
        goto LABEL_RETRY;
    }

    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
    return u32_err ? u32_err : u32_err_12;
}


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
    for(u32_i=0; u32_i<TEST_BUFFER_SIZE>>2; u32_i++)
    {
        pu32_W[u32_i] = u32_DataPattern;
        pu32_R[u32_i] = ~pu32_W[u32_i];
    }

    u16_BlkCnt_tmp = TEST_BUFFER_SIZE >> eMMC_SECTOR_512BYTE_BITS;
    u16_BlkCnt_tmp = u16_BlkCnt_tmp > u16_BlkCnt ? u16_BlkCnt : u16_BlkCnt_tmp;

    for(u32_i=0; u32_i<u16_BlkCnt_tmp; )
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


// =========================================================
#define IF_eMMC_BOOT_MODE_STG2   0

eMMC_ALIGN0 U8 gau8_BootImageOri[] eMMC_ALIGN1 ={
    #include "cmd_mstar_emmc_bootloader.txt"
};

#if 0 == IF_eMMC_BOOT_MODE_STG2
eMMC_ALIGN0 static U8 gau8_BootImage0[0x18200] eMMC_ALIGN1;
#else
eMMC_ALIGN0 static U8 gau8_BootImage0[0x400] eMMC_ALIGN1;
eMMC_ALIGN0 static U8 gau8_BootImage1[0x800] eMMC_ALIGN1;
eMMC_ALIGN0 static U8 gau8_BootImage2[0x18200-0xC00] eMMC_ALIGN1;
#endif

U32 eMMC_BootMode(void)
{
    U32 u32_err;
    #if IF_eMMC_BOOT_MODE_STG2
    U8 *pu8_buf;
    U32 u32_SecIdx, u32_tmp, u32_i, u32_j;
    #endif
    U32 au32_Addr[3], au32_SectorCnt[3];

    eMMC_debug(0,1,"Boot Mode test \n");

    #if 0
    eMMC_debug(0,1,"ChkSum +: %Xh, ByteCnt: %Xh \n",
        eMMC_ChkSum(gau8_BootImageOri, sizeof(gau8_BootImageOri)),
        sizeof(gau8_BootImageOri));
    #else
    eMMC_debug(0,1,"ChkSum +: %Xh, ByteCnt: %Xh \n",
        eMMC_ChkSum(gau8_BootImageOri, 0xC1<<eMMC_SECTOR_512BYTE_BITS),
        0xC1<<eMMC_SECTOR_512BYTE_BITS); // for Gladius
    #endif

    au32_Addr[0] = (U32)gau8_BootImage0;
    au32_SectorCnt[0] = sizeof(gau8_BootImageOri)>>eMMC_SECTOR_512BYTE_BITS;
    #if IF_eMMC_BOOT_MODE_STG2
    au32_Addr[1] = (U32)gau8_BootImage1;
    au32_SectorCnt[1] = sizeof(gau8_BootImage1)>>eMMC_SECTOR_512BYTE_BITS;
    au32_Addr[2] = (U32)gau8_BootImage2;
    au32_SectorCnt[2] = sizeof(gau8_BootImage2)>>eMMC_SECTOR_512BYTE_BITS;
    #endif

    #if 0 == IF_eMMC_BOOT_MODE_STG2
    u32_err = eMMC_LoadImages(au32_Addr, au32_SectorCnt, 1);
    #else
    u32_err = eMMC_LoadImages(au32_Addr, au32_SectorCnt, 3);
    #endif
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, eMMC_LoadImages: %Xh\n", u32_err);
        return u32_err;
    }

    #if 0 == IF_eMMC_BOOT_MODE_STG2
    //eMMC_debug(0,1,"ChkSum -: %Xh \n", eMMC_ChkSum((U8*)au32_Addr, sizeof(gau8_BootImageOri)));
    eMMC_debug(0,1,"ChkSum -: %Xh, Addr: %Xh \n", eMMC_ChkSum((U8*)au32_Addr[0], 0xC1<<eMMC_SECTOR_512BYTE_BITS), au32_Addr[0]);
    #else
    u32_SecIdx = 0;
    for(u32_j=0; u32_j<3; u32_j++)
    for(u32_i=0; u32_i<au32_SectorCnt[u32_j]; u32_i++)
    {
        eMMC_debug(0,1,"Sector: %Xh\n", u32_SecIdx);
        eMMC_debug(0,1,"\n");
        eMMC_dump_mem(au32_Addr[u32_j] + (u32_i<<eMMC_SECTOR_512BYTE_BITS), eMMC_SECTOR_512BYTE);
        u32_SecIdx++;

        pu8_buf = (U8*)(au32_Addr[u32_j] + (u32_i<<eMMC_SECTOR_512BYTE_BITS));
        for(u32_tmp=0; u32_tmp<eMMC_SECTOR_512BYTE; u32_tmp++)
            u32_ChkSum += pu8_buf[u32_tmp];
    }
    #endif

    eMMC_debug(0,1,"Boot Mode test end\n");
    return eMMC_ST_SUCCESS;

}

//========================================================
// exposed function
//========================================================
#define eMMC_SPEED_TEST_COUNTDOWN          3

#define eMMC_SPEED_TEST_SINGLE_BLKCNT_W    500
#define eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W  ((U32)30*(1024*1024/512)) // total 30MB

#define eMMC_SPEED_TEST_R_MULTI            10
#define eMMC_SPEED_TEST_SINGLE_BLKCNT_R    ((U32)eMMC_SPEED_TEST_SINGLE_BLKCNT_W*eMMC_SPEED_TEST_R_MULTI)
#define eMMC_SPEED_TEST_MULTIPLE_BLKCNT_R  ((U32)eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W*eMMC_SPEED_TEST_R_MULTI)

void eMMCTest_DownCount(U32 u32_Sec) // verify the HW Timer
{
    U32 u32_i, u32_t0, u32_t1;//, au32_tmp[6];

    //eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"2013 0125 x\n");
    for(u32_i=0; u32_i<u32_Sec; u32_i++)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"%u \n", u32_Sec-u32_i);
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
            while(0==(u32_t0-eMMC_hw_timer_tick())/eMMC_HW_TIMER_HZ)
                ;
        else
            while(0==(eMMC_hw_timer_tick()-u32_t0)/eMMC_HW_TIMER_HZ)
                ;
        #endif
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,".\n");
    //eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"%u %u   %u %u   %u %u \n",
    //  au32_tmp[0], au32_tmp[1], au32_tmp[2], au32_tmp[3], au32_tmp[4], au32_tmp[5]);
}

U32 eMMC_IPVerify_Performance(void)
{
    volatile U32 u32_err, u32_i, u32_SectorAddr=0, u32_SectorCnt=0, u32_t0, u32_t1, u32_t;
    U8 u8_LoopCnt=0;

    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"\n");
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"SectorBuf:%Xh, RBuf:%Xh, WBuf:%Xh\n",
        (U32)gau8_eMMC_SectorBuf, (U32)gau8_RBuf, (U32)gau8_WBuf);

    // ===============================================
    u32_err = eMMC_Init();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_Init fail: %Xh \n", u32_err);
        return u32_err;
    }
    eMMCTest_DownCount(eMMC_SPEED_TEST_COUNTDOWN);
    for(u32_i=0; u32_i<TEST_BUFFER_SIZE; u32_i++)
        gau8_WBuf[u32_i] = (U8)u32_i;

    // ===============================================
    u32_err = eMMC_FCIE_EnableSDRMode();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableSDRMode fail: %Xh\n", u32_err);
        eMMC_DumpDriverStatus();
        return u32_err;
    }
    eMMC_clock_setting(gau8_FCIEClkSel[0]);
    eMMC_debug(0, 0, "\n\nSDR, clk: %u.%u MHz\n",
    g_eMMCDrv.u32_ClkKHz/1000, (g_eMMCDrv.u32_ClkKHz/100)%10);

    // ===============================================
    LABEL_TEST_START:
    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"Single Write, total: %u KB, %u blocks, burst size: %Xh\n",
        (eMMC_SPEED_TEST_SINGLE_BLKCNT_W<<eMMC_SECTOR_512BYTE_BITS)>>10,
        eMMC_SPEED_TEST_SINGLE_BLKCNT_W, 1<<eMMC_SECTOR_512BYTE_BITS);
    eMMC_hw_timer_start();
    u32_t0 = eMMC_hw_timer_tick();
    for(u32_i=0; u32_i<eMMC_SPEED_TEST_SINGLE_BLKCNT_W; u32_i++)
    {
        u32_SectorAddr += (g_eMMCDrv.u32_SEC_COUNT>>2) + 333*u32_i;
        u32_SectorAddr %= g_eMMCDrv.u32_SEC_COUNT;
        if(u32_SectorAddr < g_eMMCDrv.u32_SEC_COUNT>>1)
            u32_SectorAddr += g_eMMCDrv.u32_SEC_COUNT>>1;
        u32_SectorAddr -= eMMC_SPEED_TEST_SINGLE_BLKCNT_W;
        u32_SectorAddr = eMMC_TEST_BLK_0+(u32_SectorAddr%eMMC_TEST_BLK_CNT);

        //eMMC_debug(0,0,"%08Xh \n", u32_SectorAddr+u32_i);
        #if eMMC_FEATURE_RELIABLE_WRITE
        u32_err = eMMC_CMD24(u32_SectorAddr, gau8_WBuf);
        #else
        u32_err = eMMC_CMD24_MIU(u32_SectorAddr, gau8_WBuf);
        #endif
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh \n", u32_err);
            return u32_err;
        }
    }
    eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH);
    u32_t1 = eMMC_hw_timer_tick();
    u32_t = u32_t1 > u32_t0 ? u32_t1-u32_t0 : 0xFFFFFFFF-u32_t0+u32_t1;
    u32_t = u32_t/(eMMC_HW_TIMER_HZ/1000); // ms
    if(0!=u32_t)
        eMMC_debug(0,0," %u KB/s \n",
            ((eMMC_SPEED_TEST_SINGLE_BLKCNT_W<<eMMC_SECTOR_512BYTE_BITS)*1000/u32_t)>>10);
    else
        eMMC_debug(0,0,"t=0 \n");

    // -----------------------------------------
    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"Single Read, total: %u KB, %u blocks, burst size: %Xh\n",
        ((eMMC_SPEED_TEST_SINGLE_BLKCNT_R)<<eMMC_SECTOR_512BYTE_BITS)>>10,
        eMMC_SPEED_TEST_SINGLE_BLKCNT_R, 1<<eMMC_SECTOR_512BYTE_BITS);
    eMMC_hw_timer_start();
    u32_t0 = eMMC_hw_timer_tick();
    for(u32_i=0; u32_i<eMMC_SPEED_TEST_SINGLE_BLKCNT_R; u32_i++)
    {
        u32_SectorAddr += (g_eMMCDrv.u32_SEC_COUNT>>2) + 333*u32_i;
        u32_SectorAddr %= g_eMMCDrv.u32_SEC_COUNT;
        if(u32_SectorAddr < g_eMMCDrv.u32_SEC_COUNT>>1)
            u32_SectorAddr += g_eMMCDrv.u32_SEC_COUNT>>1;
        u32_SectorAddr -= eMMC_SPEED_TEST_SINGLE_BLKCNT_R;
        u32_SectorAddr = eMMC_TEST_BLK_0+(u32_SectorAddr%eMMC_TEST_BLK_CNT);

        u32_err = eMMC_CMD17_MIU(u32_SectorAddr, gau8_RBuf);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh \n", u32_err);
            return u32_err;
        }
    }
    eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH);
    u32_t1 = eMMC_hw_timer_tick();
    u32_t = u32_t1 > u32_t0 ? u32_t1-u32_t0 : 0xFFFFFFFF-u32_t0+u32_t1;
    u32_t = u32_t/(eMMC_HW_TIMER_HZ/1000); // ms
    if(0!=u32_t)
        eMMC_debug(0,0," %u KB/s \n",
            ((eMMC_SPEED_TEST_SINGLE_BLKCNT_R<<eMMC_SECTOR_512BYTE_BITS)*1000/u32_t)>>10);
    else
        eMMC_debug(0,0,"t=0 \n");

    // ==================================
    u32_SectorCnt = 0;
    LABEL_MULTI:
    eMMC_debug(0,0,"------------------------------\n");
    switch(u32_SectorCnt)
    {
        case 0:
            u32_SectorCnt = 0x400*4 >> eMMC_SECTOR_512BYTE_BITS; //4KB
            break;
        case 0x400*4 >> eMMC_SECTOR_512BYTE_BITS:
            u32_SectorCnt = 0x400*16 >> eMMC_SECTOR_512BYTE_BITS; //16KB
            break;
        case 0x400*16 >> eMMC_SECTOR_512BYTE_BITS:
            u32_SectorCnt = 0x400*64 >> eMMC_SECTOR_512BYTE_BITS; //64KB
            break;
        case 0x400*64 >> eMMC_SECTOR_512BYTE_BITS:
            u32_SectorCnt = 0x400*1024 >> eMMC_SECTOR_512BYTE_BITS; //1MB
            break;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"Multi Write, total: %u MB, burst size: %u KB\n",
        (eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W<<eMMC_SECTOR_512BYTE_BITS)>>20, u32_SectorCnt>>1);

    u32_SectorAddr = eMMC_TEST_BLK_0;
    eMMC_hw_timer_start();
    u32_t0 = eMMC_hw_timer_tick();
    for(u32_i=0; u32_i<eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W; )
    {
        u32_err = eMMC_CMD25_MIU(u32_SectorAddr, gau8_WBuf, u32_SectorCnt);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh \n", u32_err);
            return u32_err;
        }
        u32_i += u32_SectorCnt;
    }
    eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH);
    u32_t1 = eMMC_hw_timer_tick();
    u32_t = u32_t1 > u32_t0 ? u32_t1-u32_t0 : 0xFFFFFFFF-u32_t0+u32_t1;
    u32_t = u32_t/(eMMC_HW_TIMER_HZ/100); // 10ms
    if(0!=u32_t)
        eMMC_debug(0,0," %u.%u MB/s \n",
            ((eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W<<eMMC_SECTOR_512BYTE_BITS)*100/u32_t)>>20,
            (((eMMC_SPEED_TEST_MULTIPLE_BLKCNT_W<<eMMC_SECTOR_512BYTE_BITS)*100/u32_t)>>10)*10/1024%10);
    else
        eMMC_debug(0,0,"t=0 \n");

    // -----------------------------------------
    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"Multi Read, total: %uMB, burst size: %u KB \n",
        (eMMC_SPEED_TEST_MULTIPLE_BLKCNT_R<<eMMC_SECTOR_512BYTE_BITS)>>20, u32_SectorCnt>>1);

    u32_SectorAddr = eMMC_TEST_BLK_0;
    eMMC_hw_timer_start();
    u32_t0 = eMMC_hw_timer_tick();
    for(u32_i=0; u32_i<eMMC_SPEED_TEST_MULTIPLE_BLKCNT_R;)
    {
        u32_err = eMMC_CMD18_MIU(u32_SectorAddr, gau8_RBuf, u32_SectorCnt);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh \n", u32_err);
            return u32_err;
        }
        u32_i += u32_SectorCnt;
    }
    eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH);
    u32_t1 = eMMC_hw_timer_tick();
    u32_t = u32_t1 > u32_t0 ? u32_t1-u32_t0 : 0xFFFFFFFF-u32_t0+u32_t1;
    u32_t = u32_t/(eMMC_HW_TIMER_HZ/10); // 100ms
    if(0!=u32_t)
        eMMC_debug(0,0," %u.%u MB/s \n",
            ((eMMC_SPEED_TEST_MULTIPLE_BLKCNT_R<<eMMC_SECTOR_512BYTE_BITS)*10/u32_t)>>20,
            (((eMMC_SPEED_TEST_MULTIPLE_BLKCNT_R<<eMMC_SECTOR_512BYTE_BITS)*10/u32_t)>>10)*10/1024%10);
    else
        eMMC_debug(0,0,"t=0 \n");

    if(u32_SectorCnt != 0x400*1024 >> eMMC_SECTOR_512BYTE_BITS)
        goto LABEL_MULTI;

    // ===============================================
    if(0==u8_LoopCnt)
    {
        u8_LoopCnt++;

        eMMC_debug(0,0,"\n\n");
        // test for next mode
        u32_err = eMMC_FCIE_ChooseSpeedMode();
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: ChooseSpeedMode fail: %Xh\n", u32_err);
            return u32_err;
        }

        goto LABEL_TEST_START;
    }

    return u32_err;
}


U32 eMMC_IPVerify_Main(void)
{
    U32 u32_err;

    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"eMMC_IPVerify_Main\n");
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"%Xh %Xh %Xh\n",
        (U32)gau8_eMMC_SectorBuf, (U32)gau8_RBuf, (U32)gau8_WBuf);

    // ===============================================
    #if 0
    eMMC_BootMode();
    //while(1);
    return 0;
    #endif

    // ===============================================
    u32_err = eMMC_Init();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_Init fail: %Xh \n", u32_err);
        return u32_err;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMC_Init ok] \n");

    // ===============================================
    #if 0
    eMMC_debug(0,1,"BootImage ByteCnt: %Xh, ChkSum:%Xh\n",
        sizeof(gau8_BootImageOri), eMMC_ChkSum(gau8_BootImageOri, sizeof(gau8_BootImageOri)));

    u32_err = eMMC_WriteBootPart(gau8_BootImageOri, sizeof(gau8_BootImageOri), 0, 1);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_WriteBootPart fail: %Xh \n", u32_err);
        //goto LABEL_IP_VERIFY_ERROR;
    }
    else
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "[eMMC_WriteBootPart ok] \n");

    while(1);
    #endif

    // ===============================================
    eMMC_IPVerify_Main_Ex(eMMC_PATTERN_FFFFFFFF);
    eMMC_IPVerify_Main_Ex(eMMC_PATTERN_00000000);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "normal pattern test ok\n\n");

    eMMC_IPVerify_Main_Ex(eMMC_PATTERN_000000FF);
    eMMC_IPVerify_Main_Ex(eMMC_PATTERN_0000FFFF);
    eMMC_IPVerify_Main_Ex(eMMC_PATTERN_00FF00FF);
    eMMC_IPVerify_Main_Ex(eMMC_PATTERN_AA55AA55);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "specific pattern test ok\n\n");

    #if 1
    eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_FFFFFFFF);
    eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_00000000);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "normal pattern test ok\n\n");

    eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_000000FF);
    eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_0000FFFF);
    eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_00FF00FF);
    eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_AA55AA55);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "specific pattern test ok\n\n");
    #endif

    #if 1
    eMMC_IPVerify_Main_API_Ex(eMMC_PATTERN_00FF00FF);
    eMMC_IPVerify_Main_API_Ex(eMMC_PATTERN_AA55AA55);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "APIs test ok\n\n");
    #endif

    return eMMC_ST_SUCCESS;
}


U32  eMMC_IPVerify_Main_API_Ex(U32 u32_DataPattern)
{
    U32 u32_i, *pu32_W=(U32*)gau8_WBuf, *pu32_R=(U32*)gau8_RBuf, u32_err;

    // init data pattern
    for(u32_i=0; u32_i<TEST_BUFFER_SIZE>>2; u32_i++)
    {
        pu32_W[u32_i] = u32_DataPattern;
        pu32_R[u32_i] = ~pu32_W[u32_i];
    }

    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "Data Pattern: %08Xh\n", u32_DataPattern);

    u32_err = eMMC_WriteData((U8*)pu32_W, TEST_BUFFER_SIZE, eMMC_TEST_BLK_0);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_WriteData fail: %Xh \n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMC_WriteData ok] \n");

    u32_err = eMMC_ReadData((U8*)pu32_R, TEST_BUFFER_SIZE, eMMC_TEST_BLK_0);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMC_ReadData fail: %Xh \n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMC_ReadData ok] \n");

    u32_err = eMMC_ComapreData((U8*)pu32_W, (U8*)pu32_R, TEST_BUFFER_SIZE);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, eMMC_ComapreData fail: %u, %Xh \n", u32_i, u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }

    // ===============================================
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n");
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "[OK] \n\n");
    return eMMC_ST_SUCCESS;

    LABEL_IP_VERIFY_ERROR:
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n\n");
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "[eMMC IPVerify Fail: %Xh] \n\n", u32_err);
    eMMC_DumpDriverStatus();
    while(1);
    return u32_err;
}


U32  eMMC_IPVerify_Main_Sg_Ex(U32 u32_DataPattern)
{
    static U32 u32_StartSector=0, u32_SectorCnt=0, u32_err;

    // make StartSector SectorCnt random
    u32_StartSector = eMMC_TEST_BLK_0;
    u32_SectorCnt = eMMC_TEST_BLK_CNT;


    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "Data Pattern: %08Xh\n", u32_DataPattern);

    #if 1
    u32_err = eMMCTest_SgWRC_MIU(u32_StartSector, u32_SectorCnt, u32_DataPattern);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMCTest_SgWRC_MIU fail: %Xh \n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMCTest_SgWRC_MIU ok] \n");
    #endif

    // ===============================================
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n");
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "[OK] \n\n");
    return eMMC_ST_SUCCESS;

    LABEL_IP_VERIFY_ERROR:
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n\n");
    eMMC_debug(eMMC_DEBUG_LEVEL,1,"Total Sec: %Xh, Test: StartSec: %Xh, SecCnt: %Xh \n",
            g_eMMCDrv.u32_SEC_COUNT, u32_StartSector, u32_SectorCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "[eMMC IPVerify Fail: %Xh] \n\n", u32_err);
    eMMC_DumpDriverStatus();
    while(1);
    return u32_err;
}


U32 eMMC_IPVerify_Main_Ex(U32 u32_DataPattern)
{
    static U32 u32_StartSector=0, u32_SectorCnt=0, u32_err;

    // make StartSector SectorCnt random
    u32_StartSector = eMMC_TEST_BLK_0;
    u32_SectorCnt++;
    while(u32_SectorCnt > eMMC_TEST_BLK_CNT)
        u32_SectorCnt = 1;

    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "Data Pattern: %08Xh\n", u32_DataPattern);
    //eMMC_debug(0,1,"Total Sec: %Xh, StartSec: %Xh, SecCnt: %Xh \n",
    //      g_eMMCDrv.u32_SEC_COUNT, u32_StartSector, u32_SectorCnt);

    #if 1
    if(eMMC_IF_NORMAL_SDR()){
    u32_err = eMMCTest_SingleBlkWRC_RIU(u32_StartSector, u32_DataPattern);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMCTest_SingleBlkWRC_RIU fail: %Xh \n", u32_err);
        //goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMCTest_SingleBlkWRC_RIU ok] \n");}
    #endif

    #if 1
    u32_err = eMMCTest_SingleBlkWRC_MIU(u32_StartSector, u32_DataPattern);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMCTest_SingleBlkWRC_MIU fail: %Xh \n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMCTest_SingleBlkWRC_MIU ok] \n");
    #endif

    #if 1
    u32_err = eMMCTest_MultiBlkWRC_MIU(u32_StartSector, u32_SectorCnt, u32_DataPattern);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Err, eMMCTest_MultiBlkWRC_MIU fail: %Xh \n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[eMMCTest_MultiBlkWRC_MIU ok] \n");
    #endif


    // ===============================================
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n");
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "[OK] \n\n");
    return eMMC_ST_SUCCESS;

    LABEL_IP_VERIFY_ERROR:
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n\n");
    eMMC_debug(eMMC_DEBUG_LEVEL,1,"Total Sec: %Xh, Test: StartSec: %Xh, SecCnt: %Xh \n",
            g_eMMCDrv.u32_SEC_COUNT, u32_StartSector, u32_SectorCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "[eMMC IPVerify Fail: %Xh] \n\n", u32_err);
    eMMC_DumpDriverStatus();
    while(1);
    return u32_err;
}


U32 eMMC_IPVerify_ReadOnly(void)
{
    static U32 u32_StartSector=0, u32_SectorCnt=0, u32_err;

    // make StartSector SectorCnt random
    u32_StartSector = eMMC_TEST_BLK_0;
    u32_SectorCnt++;
    while(u32_SectorCnt > eMMC_TEST_BLK_CNT)
        u32_SectorCnt = 1;

    #if 1
    if(eMMC_IF_NORMAL_SDR()){
    u32_err = eMMC_CMD17_CIFD(u32_StartSector, gau8_RBuf);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, CMD17_CIFD fail: %Xh \n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[CMD17_CIFD ok] \n");}
    #endif

    #if 1
    u32_err = eMMC_CMD17_MIU(u32_StartSector, gau8_RBuf);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, CMD17_MIU fail: %Xh \n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[CMD17_MIU ok] \n");
    #endif

    #if 1
    u32_err = eMMC_CMD18_MIU(u32_StartSector, gau8_RBuf, u32_SectorCnt);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, CMD18_MIU fail: %Xh \n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[CMD18_MIU ok] \n");
    #endif

    // ===============================================
    //eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n");
    //eMMC_debug(eMMC_DEBUG_LEVEL, 1, "[OK] \n\n");
    return eMMC_ST_SUCCESS;

    LABEL_IP_VERIFY_ERROR:
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n\n");
    eMMC_debug(eMMC_DEBUG_LEVEL,1,"Total Sec: %Xh, Test: StartSec: %Xh, SecCnt: %Xh \n",
            g_eMMCDrv.u32_SEC_COUNT, u32_StartSector, u32_SectorCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "fail: %Xh \n\n", u32_err);
    eMMC_DumpDriverStatus();
    while(1);
    return u32_err;
}


U32 eMMC_IPVerify_WriteOnly(U16 u16_TestPattern)
{
    static U32 u32_StartSector=0, u32_SectorCnt=0, u32_err;
    U32 u32_i, *pu32_W=(U32*)gau8_WBuf, u32_ByteCnt;

    // make StartSector SectorCnt random
    u32_StartSector = eMMC_TEST_BLK_0;
    u32_SectorCnt++;
    while(u32_SectorCnt > eMMC_TEST_BLK_CNT)
        u32_SectorCnt = 1;

    u32_ByteCnt = (u32_SectorCnt<<9) > TEST_BUFFER_SIZE ?
        TEST_BUFFER_SIZE : (u32_SectorCnt<<9);

    // init data pattern
    for(u32_i=0; u32_i<u32_ByteCnt>>2; u32_i++)
    {
        pu32_W[u32_i] = (u16_TestPattern<<16)+u16_TestPattern;
    }

    #if 1
    if(eMMC_IF_NORMAL_SDR()){
    u32_err = eMMC_CMD24_CIFD(u32_StartSector, gau8_WBuf);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, CMD24_CIFD fail: %Xh \n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[CMD24_CIFD ok] \n");}
    #endif

    #if 1
    u32_err = eMMC_CMD24_MIU(u32_StartSector, gau8_WBuf);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, CMD24_MIU fail: %Xh \n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[CMD24_MIU ok] \n");
    #endif

    #if 1
    u32_err = eMMC_CMD25_MIU(u32_StartSector, gau8_WBuf, u32_SectorCnt);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(0,1,"Err, CMD25_MIU fail: %Xh \n", u32_err);
        goto LABEL_IP_VERIFY_ERROR;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,1,"[CMD25_MIU ok] \n");
    #endif

    // ===============================================
    //eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n");
    //eMMC_debug(eMMC_DEBUG_LEVEL, 1, "[OK] \n\n");
    return eMMC_ST_SUCCESS;

    LABEL_IP_VERIFY_ERROR:
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n\n");
    eMMC_debug(eMMC_DEBUG_LEVEL,1,"Total Sec: %Xh, Test: StartSec: %Xh, SecCnt: %Xh \n",
            g_eMMCDrv.u32_SEC_COUNT, u32_StartSector, u32_SectorCnt);
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "fail: %Xh \n\n", u32_err);
    eMMC_DumpDriverStatus();
    while(1);
    return u32_err;
}



U32 eMMCTest_Lifetime(U8 u8_TestMode)
{
    U32 u32_i, u32_err=eMMC_ST_SUCCESS, u32_BlkAddr;
    U32 u32_BlkCnt0=0, u32_BlkCnt1=0, *pu32_BlkCnt, u32_LoopCnt=0;

    // set to slow & safe clock
    eMMC_clock_setting(gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT-1]);

    eMMC_debug(eMMC_DEBUG_LEVEL,0,"===========================================\n");
    eMMC_debug(eMMC_DEBUG_LEVEL,0,"CAUTION: will damage images in eMMC \n");
    eMMC_debug(eMMC_DEBUG_LEVEL,0,"===========================================\n");

    //-------------------------------------------------
    if(eMMC_LIFETIME_TEST_FIXED == u8_TestMode)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL,1,"fixed addr: 6th block\n");

        while(1)
        {
            eMMC_reset_WatchDog();

            if(u32_BlkCnt0 < (U32)(0-1))
                pu32_BlkCnt = &u32_BlkCnt0;
            else if((U32)(0-1) == u32_BlkCnt1)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n eMMC: %Xh %Xh blocks tested, stop \n",
                    u32_BlkCnt1, u32_BlkCnt0);
                while(1);
            }
            else
                pu32_BlkCnt = &u32_BlkCnt1;

            for(u32_i=0; u32_i<eMMC_SECTOR_BYTECNT; u32_i++)
                gau8_WBuf[u32_i] = *pu32_BlkCnt+u32_i;

            u32_err = eMMC_CMD24_MIU(6, gau8_WBuf);
            if(eMMC_ST_SUCCESS != u32_err)
            {
                eMMC_debug(0,1,"Err, CMD24_MIU, %Xh \n", u32_err);
                break;
            }

            u32_err = eMMC_CMD18_MIU( // to drop data from possible cache inside eMMC
                0, gau8_RBuf, TEST_BUFFER_SIZE>>eMMC_SECTOR_BYTECNT_BITS);
            if(eMMC_ST_SUCCESS != u32_err)
            {
                eMMC_debug(0,1,"Err, CMD18_MIU, %Xh \n", u32_err);
                break;
            }u32_err = eMMC_CMD18_MIU( // to drop data from possible cache inside eMMC
                100, gau8_RBuf, TEST_BUFFER_SIZE>>eMMC_SECTOR_BYTECNT_BITS);
            if(eMMC_ST_SUCCESS != u32_err)
            {
                eMMC_debug(0,1,"Err, CMD18_MIU, %Xh \n", u32_err);
                break;
            }

            u32_err = eMMC_CMD17_MIU(6, gau8_RBuf);
            if(eMMC_ST_SUCCESS != u32_err)
            {
                eMMC_debug(0,1,"Err, CMD17_MIU, %Xh \n", u32_err);
                break;
            }

            u32_err = eMMC_ComapreData(gau8_WBuf, gau8_RBuf, eMMC_SECTOR_BYTECNT);
            if(eMMC_ST_SUCCESS != u32_err)
            {
                eMMC_debug(0,1,"Err, ComapreData, %Xh \n", u32_err);
                break;
            }

            *pu32_BlkCnt+=1;
            eMMC_debug(eMMC_DEBUG_LEVEL,0,"\r eMMC: %08X %08X blocks tested ... ",
                u32_BlkCnt1, u32_BlkCnt0);
        }

        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\neMMC Err: fail: %Xh \n", u32_err);
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: %Xh %Xh blocks tested, stop \n",
            u32_BlkCnt1, u32_BlkCnt0);
        eMMC_FCIE_ErrHandler_Stop();

    }
    //-------------------------------------------------
    else if(eMMC_LIFETIME_TEST_FILLED == u8_TestMode)
    {
        while(1)
        {
            eMMC_reset_WatchDog();

            if((U32)(0-1) == u32_LoopCnt)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n eMMC: %u loops tested, stop \n",
                    u32_LoopCnt);
                while(1);
            }

            for(u32_i=0; u32_i<TEST_BUFFER_SIZE; u32_i++)
                gau8_WBuf[u32_i] = u32_LoopCnt+u32_i;

            // w
            for(u32_i=0; u32_i<g_eMMCDrv.u32_SEC_COUNT;)
            {
                u32_err = eMMC_CMD25_MIU(u32_i, gau8_WBuf, TEST_BUFFER_SIZE>>eMMC_SECTOR_BYTECNT_BITS);
                if(eMMC_ST_SUCCESS != u32_err)
                {
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\neMMC Err: w fail: %Xh \n", u32_err);
                    eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: (%Xh x %Xh) blocks tested, stop \n",
                        u32_LoopCnt, g_eMMCDrv.u32_SEC_COUNT);
                    eMMC_FCIE_ErrHandler_Stop();
                }

                u32_i += TEST_BUFFER_SIZE>>eMMC_SECTOR_BYTECNT_BITS;
                if(g_eMMCDrv.u32_SEC_COUNT-u32_i <
                    (TEST_BUFFER_SIZE>>eMMC_SECTOR_BYTECNT_BITS))
                    break;

                eMMC_debug(eMMC_DEBUG_LEVEL,0,"\r eMMC: w (%08Xh x %08Xh + %08Xh) blocks ...  ",
                    u32_LoopCnt, g_eMMCDrv.u32_SEC_COUNT, u32_i);
            }
            // r & c
            for(u32_i=0; u32_i<g_eMMCDrv.u32_SEC_COUNT;)
            {
                u32_err = eMMC_CMD18_MIU(u32_i, gau8_RBuf, TEST_BUFFER_SIZE>>eMMC_SECTOR_BYTECNT_BITS);
                if(eMMC_ST_SUCCESS != u32_err)
                {
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\neMMC Err: r fail: %Xh \n", u32_err);
                    eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: (%Xh x %Xh) blocks tested, stop \n",
                        u32_LoopCnt, g_eMMCDrv.u32_SEC_COUNT);
                    eMMC_FCIE_ErrHandler_Stop();
                }

                u32_err = eMMC_ComapreData(gau8_WBuf, gau8_RBuf, TEST_BUFFER_SIZE);
                if(eMMC_ST_SUCCESS != u32_err)
                {
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\neMMC Err: c fail: %Xh \n", u32_err);
                    eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: (%Xh x %Xh) blocks tested, stop \n",
                        u32_LoopCnt, g_eMMCDrv.u32_SEC_COUNT);
                    eMMC_FCIE_ErrHandler_Stop();
                }

                u32_i += TEST_BUFFER_SIZE>>eMMC_SECTOR_BYTECNT_BITS;
                if(g_eMMCDrv.u32_SEC_COUNT-u32_i <
                    (TEST_BUFFER_SIZE>>eMMC_SECTOR_BYTECNT_BITS))
                    break;

                eMMC_debug(eMMC_DEBUG_LEVEL,0,"\r eMMC: r c (%08Xh x %08Xh + %08Xh) blocks ...  ",
                    u32_LoopCnt, g_eMMCDrv.u32_SEC_COUNT, u32_i);
            }

            u32_LoopCnt += 1;
        }
    }
    //-------------------------------------------------
    else if(eMMC_LIFETIME_TEST_RANDOM == u8_TestMode)
    {
        eMMC_hw_timer_start(); // use to get tick as a random seed

        while(1)
        {
            eMMC_reset_WatchDog();

            if((U32)(0-1) == u32_LoopCnt)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n eMMC: %u loops tested, stop \n",
                    u32_LoopCnt);
                while(1);
            }

            for(u32_i=0; u32_i<TEST_BUFFER_SIZE; u32_i++)
                gau8_WBuf[u32_i] = u32_LoopCnt+u32_i;

            u32_BlkAddr = eMMC_hw_timer_tick()*11;
            u32_BlkAddr %= g_eMMCDrv.u32_SEC_COUNT;
            if(g_eMMCDrv.u32_SEC_COUNT - u32_BlkAddr
                < (TEST_BUFFER_SIZE>>eMMC_SECTOR_BYTECNT_BITS))
                u32_BlkAddr = g_eMMCDrv.u32_SEC_COUNT
                    - (TEST_BUFFER_SIZE>>eMMC_SECTOR_BYTECNT_BITS) - 1;
            // w
            u32_err = eMMC_CMD25_MIU(u32_BlkAddr, gau8_WBuf, TEST_BUFFER_SIZE>>eMMC_SECTOR_BYTECNT_BITS);
            if(eMMC_ST_SUCCESS != u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\neMMC Err: w fail: %Xh, BlkAddr: %Xh \n", u32_err, u32_BlkAddr);
                eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: (%Xh x %Xh) blocks tested, stop \n",
                    u32_LoopCnt, g_eMMCDrv.u32_SEC_COUNT);
                eMMC_FCIE_ErrHandler_Stop();
            }

            // r & c
            u32_err = eMMC_CMD18_MIU(u32_BlkAddr, gau8_RBuf, TEST_BUFFER_SIZE>>eMMC_SECTOR_BYTECNT_BITS);
            if(eMMC_ST_SUCCESS != u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\neMMC Err: r fail: %Xh, BlkAddr: %Xh \n", u32_err, u32_BlkAddr);
                eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: (%Xh x %Xh) blocks tested, stop \n",
                    u32_LoopCnt, g_eMMCDrv.u32_SEC_COUNT);
                eMMC_FCIE_ErrHandler_Stop();
            }
            u32_err = eMMC_ComapreData(gau8_WBuf, gau8_RBuf, TEST_BUFFER_SIZE);
            if(eMMC_ST_SUCCESS != u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"\neMMC Err: c fail: %Xh, BlkAddr: %Xh \n", u32_err, u32_BlkAddr);
                eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: (%Xh x %Xh) blocks tested, stop \n",
                    u32_LoopCnt, g_eMMCDrv.u32_SEC_COUNT);
                eMMC_FCIE_ErrHandler_Stop();
            }

            u32_LoopCnt += 1;
            eMMC_debug(eMMC_DEBUG_LEVEL,0,"\r eMMC: now @ %08Xh block, total %08Xh x %Xh blocks tested ...  ",
                u32_BlkAddr, u32_LoopCnt, TEST_BUFFER_SIZE>>eMMC_SECTOR_BYTECNT_BITS);
        }
    }

    return u32_err;
}


U32 eMMC_IPVerify_SDRDDR_AllClkTemp(void)
{
    U32 u32_err;
    U8  u8_SDRClkIdx, u8_DDRClkIdx;

    u32_err = eMMC_FCIE_ChooseSpeedMode();
    if(eMMC_ST_SUCCESS != u32_err){
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: ChooseSpeedMode fail: %Xh\n", u32_err);
        return u32_err;
    }
    eMMC_DumpTimingTable();

    u8_SDRClkIdx = 0;
    u8_DDRClkIdx = 0;

    while(1)
    {
        #if 1
        //eMMC_hw_timer_delay(HW_TIMER_DELAY_1s);
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"\n");
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"======================================\n");
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"SDR\n");
        u32_err = eMMC_FCIE_EnableSDRMode();
        if(eMMC_ST_SUCCESS != u32_err){
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableSDRMode fail: %Xh\n", u32_err);
            eMMC_DumpDriverStatus();
            return u32_err;
        }
        //if(0==u8_SDRClkIdx)  u8_SDRClkIdx++; // skip 48MHz
        eMMC_clock_setting(gau8_FCIEClkSel[u8_SDRClkIdx]);
        eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Clk: %u %sHz\n",
            g_eMMCDrv.u32_ClkKHz>1000 ? g_eMMCDrv.u32_ClkKHz/1000 : g_eMMCDrv.u32_ClkKHz,
            g_eMMCDrv.u32_ClkKHz>1000 ? "M" : "K");
        u8_SDRClkIdx++;
        //u8_SDRClkIdx = u8_SDRClkIdx%(eMMC_FCIE_VALID_CLK_CNT-1);// skip 300KHz
        u8_SDRClkIdx = u8_SDRClkIdx%eMMC_FCIE_VALID_CLK_CNT;

        eMMC_IPVerify_Main_Ex(eMMC_PATTERN_FFFFFFFF);
        eMMC_IPVerify_Main_Ex(eMMC_PATTERN_00000000);
        eMMC_IPVerify_Main_Ex(eMMC_PATTERN_000000FF);
        eMMC_IPVerify_Main_Ex(eMMC_PATTERN_0000FFFF);
        eMMC_IPVerify_Main_Ex(eMMC_PATTERN_00FF00FF);
        eMMC_IPVerify_Main_Ex(eMMC_PATTERN_AA55AA55);

        eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_FFFFFFFF);
        eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_00000000);
        eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_000000FF);
        eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_0000FFFF);
        eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_00FF00FF);
        eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_AA55AA55);
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"\n");
        #endif

        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"======================================\n");
        // [CAUTION]: switch to fast mode,  Brian said a fixed clock for emmcpll, do not switch emmcpll clock. [2013 Nov.. Napoli back]
        u32_err = eMMC_FCIE_ChooseSpeedMode();
        if(eMMC_ST_SUCCESS != u32_err){
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: ChooseSpeedMode fail: %Xh\n", u32_err);
            eMMC_DumpDriverStatus();
            return u32_err;
        }
        eMMC_IPVerify_Main_Ex(eMMC_PATTERN_FFFFFFFF);
        eMMC_IPVerify_Main_Ex(eMMC_PATTERN_00000000);
        eMMC_IPVerify_Main_Ex(eMMC_PATTERN_000000FF);
        eMMC_IPVerify_Main_Ex(eMMC_PATTERN_0000FFFF);
        eMMC_IPVerify_Main_Ex(eMMC_PATTERN_00FF00FF);
        eMMC_IPVerify_Main_Ex(eMMC_PATTERN_AA55AA55);

        eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_FFFFFFFF);
        eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_00000000);
        eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_000000FF);
        eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_0000FFFF);
        eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_00FF00FF);
        eMMC_IPVerify_Main_Sg_Ex(eMMC_PATTERN_AA55AA55);
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"\n");
    }

    return eMMC_ST_SUCCESS;
}


#define eMMC_PWRCUT_DATA0      0x55
#define eMMC_PWRCUT_DATA1      0xAA
#define eMMC_PWRCUT_DATA_CNT   2

#define eMMC_PWRCUT_TEST_UNIT_BYTECNT    0x1000000
#define eMMC_PWRCUT_TEST_SPACE_BYTECNT   (16*eMMC_PWRCUT_TEST_UNIT_BYTECNT) // 256MB
#define eMMC_PWRCUT_TEST_UNIT_CNT        (eMMC_PWRCUT_TEST_SPACE_BYTECNT/eMMC_PWRCUT_TEST_UNIT_BYTECNT)

typedef eMMC_PACK0 struct _eMMC_PWRCUT_CTRLBLK {

    U32 u32_CheckSum;
    U8  u8_TargetUnit;
    U8  au8_TaegetData[eMMC_PWRCUT_TEST_UNIT_CNT];

} eMMC_PACK1 eMMC_PWRCUT_CTRLBLK_t;

void eMMC_DumpsPwrCutCtrlBlk(eMMC_PWRCUT_CTRLBLK_t *pCtrlBlk_t)
{
    U32 u32_i;

    eMMC_debug(eMMC_DEBUG_LEVEL, 0,
        "eMMC: CheckSum: %Xh\n", pCtrlBlk_t->u32_CheckSum);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0,
        "eMMC: TargetUnit: %Xh\n", pCtrlBlk_t->u8_TargetUnit);

    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "  Target Data: ");
    for(u32_i=0; u32_i<eMMC_PWRCUT_TEST_UNIT_CNT; u32_i++)
    {
        if(0 == (u32_i & 0xF))
            eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\n");

        eMMC_debug(eMMC_DEBUG_LEVEL, 0, " %Xh",
            pCtrlBlk_t->au8_TaegetData[u32_i]);
    }
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\n");
}

U32 eMMC_CheckPwrCutCtrlBlk(U8* u8_DataBuf, U32 u32_BlkAddr)
{
    U32 u32_err;
    eMMC_PWRCUT_CTRLBLK_t *pCtrlBlk_t;

    u32_err = eMMC_ReadData(u8_DataBuf, eMMC_SECTOR_512BYTE, u32_BlkAddr);
    if(u32_err != eMMC_ST_SUCCESS)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
            "eMMC Err: read CtrlBlk, Err:%x\n", u32_err);
        return u32_err;
    }

    pCtrlBlk_t = (eMMC_PWRCUT_CTRLBLK_t*)u8_DataBuf;

    if(pCtrlBlk_t->u32_CheckSum !=
        eMMC_ChkSum(u8_DataBuf+4, sizeof(eMMC_PWRCUT_CTRLBLK_t)-4))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
            "eMMC Err: read CtrlBlk, CheckSum fail\n");
        return eMMC_ST_ERR_CHKSUM;
    }

    return eMMC_ST_SUCCESS;
}


U32 eMMCTest_PwrCut_InitData(U8* u8_DataBuf, U32 u32_BlkStartAddr)
{
    U32 u32_i, u32_j;
    U32 u32_err = eMMC_ST_SUCCESS;
    eMMC_PWRCUT_CTRLBLK_t CtrlBlk_t;

    struct mmc *mmc = find_mmc_device(0);
    mmc_slc_mode(mmc, 0, 1); // enable reliable

    // ---------------------------------------
    CtrlBlk_t.u8_TargetUnit = 0xFF;

    for(u32_i=0; u32_i<eMMC_PWRCUT_TEST_UNIT_CNT; u32_i++)
        CtrlBlk_t.au8_TaegetData[u32_i] = eMMC_PWRCUT_DATA0;

    CtrlBlk_t.u32_CheckSum = eMMC_ChkSum((U8*)&CtrlBlk_t.u8_TargetUnit,
        sizeof(eMMC_PWRCUT_CTRLBLK_t)-4);

    u32_err = eMMC_WriteData((U8*)&CtrlBlk_t, eMMC_SECTOR_512BYTE, u32_BlkStartAddr);
    if(u32_err != eMMC_ST_SUCCESS)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
            "eMMC Err: write CtrlBlk, Err:%x\n", u32_err);
        return u32_err;
    }

    // ---------------------------------------
    u32_BlkStartAddr+=1; // skip Ctrl Blk

    memset(u8_DataBuf, eMMC_PWRCUT_DATA0, eMMC_PWRCUT_TEST_UNIT_BYTECNT);
    eMMC_debug(eMMC_DEBUG_LEVEL,1,"Reliable write ...\n");

    for(u32_i=0; u32_i<eMMC_PWRCUT_TEST_UNIT_CNT; u32_i++)
    {
        u32_err = eMMC_WriteData(u8_DataBuf, eMMC_PWRCUT_TEST_UNIT_BYTECNT,
            u32_BlkStartAddr + u32_i*(eMMC_PWRCUT_TEST_UNIT_BYTECNT>>9));
        if(u32_err != eMMC_ST_SUCCESS)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
                "eMMC Err: Reliable write Fail @ %u unit, Err:%x\n", u32_i, u32_err);
            return u32_err;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL, 0,"%03u%% \r", (u32_i+1)*100/eMMC_PWRCUT_TEST_UNIT_CNT);
    }

    // ---------------------------------------
    eMMC_debug(eMMC_DEBUG_LEVEL, 0,"\ncheck ...\n");

    if(eMMC_ST_SUCCESS != eMMC_CheckPwrCutCtrlBlk(u8_DataBuf, u32_BlkStartAddr-1))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: read CtrlBlk fail\n");
        return eMMC_ST_ERR_CHKSUM;
    }

    for(u32_i=0; u32_i<eMMC_PWRCUT_TEST_UNIT_CNT; u32_i++)
    {
        u32_err = eMMC_ReadData(u8_DataBuf, eMMC_PWRCUT_TEST_UNIT_BYTECNT,
            u32_BlkStartAddr + u32_i*(eMMC_PWRCUT_TEST_UNIT_BYTECNT>>9));
        if(u32_err != eMMC_ST_SUCCESS)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
                "eMMC Err: Reliable write Fail @ %u unit, Err:%x\n", u32_i, u32_err);
            return u32_err;
        }

        for (u32_j=0; u32_j < eMMC_PWRCUT_TEST_UNIT_BYTECNT; u32_j++)
        {
            if (u8_DataBuf[u32_j] != eMMC_PWRCUT_DATA0)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
                    "eMMC Err: Data Mismatch:  Blk:%Xh  ByteIdx:%Xh  ByteVal:%Xh \n",
                    u32_BlkStartAddr+u32_i*(eMMC_PWRCUT_TEST_UNIT_BYTECNT>>9)+(u32_j>>9),
                    u32_j & 0x1FF, u8_DataBuf[u32_j]);
                return eMMC_ST_ERR_DATA_MISMATCH;
            }
        }

        eMMC_debug(eMMC_DEBUG_LEVEL, 0,"%03u%% \r", (u32_i+1)*100/eMMC_PWRCUT_TEST_UNIT_CNT);
    }

    eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n init success\n");
    return eMMC_ST_SUCCESS;
}


U32 eMMCTest_PwrCut_Test(U8* u8_DataBuf, U32 u32_BlkStartAddr)
{
    U32 u32_i, u32_j;
    U32 u32_err = eMMC_ST_SUCCESS;
    U32 u32_T0, u32_BlkAddr;
    eMMC_PWRCUT_CTRLBLK_t CtrlBlk_t;

    eMMC_hw_timer_start();

    // ---------------------------------------
    eMMC_debug(eMMC_DEBUG_LEVEL, 0,"\n eMMC PwrCut Test, checking ...\n");

    // get Ctrl Blk
    if(eMMC_ST_SUCCESS != eMMC_CheckPwrCutCtrlBlk(u8_DataBuf, u32_BlkStartAddr))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: read CtrlBlk fail\n");
        //return eMMC_ST_ERR_CHKSUM;
        while(1);
    }
    memcpy(&CtrlBlk_t, u8_DataBuf, sizeof(eMMC_PWRCUT_CTRLBLK_t));

    // check data
    u32_BlkStartAddr++;

    for(u32_i=0; u32_i<eMMC_PWRCUT_TEST_UNIT_CNT; u32_i++)
    {
        u32_err = eMMC_ReadData(u8_DataBuf, eMMC_PWRCUT_TEST_UNIT_BYTECNT,
            u32_BlkStartAddr + u32_i*(eMMC_PWRCUT_TEST_UNIT_BYTECNT>>9));
        if(u32_err != eMMC_ST_SUCCESS)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,
                "\neMMC Err: Reliable write Fail @ %u unit, Err:%x\n", u32_i, u32_err);
            return u32_err;
        }

        if(u32_i == CtrlBlk_t.u8_TargetUnit){
        for (u32_j=0; u32_j < eMMC_PWRCUT_TEST_UNIT_BYTECNT; u32_j++)
        {
            if (u8_DataBuf[u32_j]!=eMMC_PWRCUT_DATA0 &&
                u8_DataBuf[u32_j]!=eMMC_PWRCUT_DATA1)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,
                    "\neMMC Err: Target Data Mismatch:  Unit:%Xh  "
                    "Blk: %Xh + %Xh x %Xh + %Xh = %Xh, ByteIdx:%Xh  ByteVal:%Xh \n",
                    u32_i,
                    u32_BlkStartAddr, u32_i, (eMMC_PWRCUT_TEST_UNIT_BYTECNT>>9), (u32_j>>9),
                    u32_BlkStartAddr+u32_i*(eMMC_PWRCUT_TEST_UNIT_BYTECNT>>9)+(u32_j>>9),
                    u32_j & 0x1FF, u8_DataBuf[u32_j]);
                eMMC_DumpsPwrCutCtrlBlk(&CtrlBlk_t);
                eMMC_dump_mem(&u8_DataBuf[u32_j]-0x10, 0x30);
                return eMMC_ST_ERR_DATA_MISMATCH;
            }
        }
        // recover TargetUnit
        u32_BlkAddr = u32_BlkStartAddr +
            CtrlBlk_t.u8_TargetUnit *
            (eMMC_PWRCUT_TEST_UNIT_BYTECNT>>eMMC_SECTOR_512BYTE_BITS);
        memset(u8_DataBuf, CtrlBlk_t.au8_TaegetData[CtrlBlk_t.u8_TargetUnit], eMMC_PWRCUT_TEST_UNIT_BYTECNT);
        u32_err = eMMC_WriteData(u8_DataBuf, eMMC_PWRCUT_TEST_UNIT_BYTECNT, u32_BlkAddr);
        if(u32_err != eMMC_ST_SUCCESS)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: write TargetUnit fail, Err:%x\n",u32_err);
            return u32_err;
        }}
        else{
        for (u32_j=0; u32_j < eMMC_PWRCUT_TEST_UNIT_BYTECNT; u32_j++)
        {
            if (u8_DataBuf[u32_j]!=CtrlBlk_t.au8_TaegetData[u32_i])
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,
                    "\neMMC Err: Data Mismatch:  Unit:%Xh  "
                    "Blk: %Xh + %Xh x %Xh + %Xh = %Xh, ByteIdx:%Xh  ByteVal:%Xh \n",
                    u32_i,
                    u32_BlkStartAddr, u32_i, (eMMC_PWRCUT_TEST_UNIT_BYTECNT>>9), (u32_j>>9),
                    u32_BlkStartAddr+u32_i*(eMMC_PWRCUT_TEST_UNIT_BYTECNT>>9)+(u32_j>>9),
                    u32_j & 0x1FF, u8_DataBuf[u32_j]);
                eMMC_DumpsPwrCutCtrlBlk(&CtrlBlk_t);
                eMMC_dump_mem(&u8_DataBuf[u32_j]-0x10, 0x30);
                //return eMMC_ST_ERR_DATA_MISMATCH;
                while(1);
            }
        }}

        eMMC_debug(eMMC_DEBUG_LEVEL, 0,"%03u%% \r", (u32_i+1)*100/eMMC_PWRCUT_TEST_UNIT_CNT);
    }

    eMMC_debug(eMMC_DEBUG_LEVEL, 0,"   ok\n");

    // ---------------------------------------
    while(1)
    {
        u32_T0 = eMMC_hw_timer_tick();
        CtrlBlk_t.u8_TargetUnit = u32_T0 % eMMC_PWRCUT_TEST_UNIT_CNT;
        CtrlBlk_t.au8_TaegetData[CtrlBlk_t.u8_TargetUnit] ^= 0xFF;
        CtrlBlk_t.u32_CheckSum = eMMC_ChkSum((U8*)&CtrlBlk_t.u8_TargetUnit,
            sizeof(eMMC_PWRCUT_CTRLBLK_t)-4);
        u32_err = eMMC_WriteData((U8*)&CtrlBlk_t, eMMC_SECTOR_512BYTE, u32_BlkStartAddr-1);
        if(u32_err != eMMC_ST_SUCCESS)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
                "eMMC Err: write CtrlBlk, Err:%x\n", u32_err);
            return u32_err;
        }

        u32_BlkAddr = u32_BlkStartAddr +
            CtrlBlk_t.u8_TargetUnit *
            (eMMC_PWRCUT_TEST_UNIT_BYTECNT>>eMMC_SECTOR_512BYTE_BITS);
        memset(u8_DataBuf, CtrlBlk_t.au8_TaegetData[CtrlBlk_t.u8_TargetUnit], eMMC_PWRCUT_TEST_UNIT_BYTECNT);

        eMMC_debug(eMMC_DEBUG_LEVEL, 0 ,"writing... %02Xh %08Xh %02Xh\n",
            CtrlBlk_t.u8_TargetUnit, u32_BlkAddr, CtrlBlk_t.au8_TaegetData[CtrlBlk_t.u8_TargetUnit]);

        u32_err = eMMC_WriteData(u8_DataBuf, eMMC_PWRCUT_TEST_UNIT_BYTECNT, u32_BlkAddr);
        //u32_err = eMMC_CMD25(u32_BlkAddr, u8_DataBuf, eMMC_PWRCUT_TEST_UNIT_BYTECNT>>9);
        if(u32_err != eMMC_ST_SUCCESS)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: write Fail, Err:%x\n",u32_err);
            return u32_err;
        }
    }

    return eMMC_ST_SUCCESS;
}


U32 eMMCTest_PwrCut_Test2(U8* u8_DataBuf, U32 u32_BlkStartAddr)
{
    U32 u32_blk_cnt ,u32_i,u32_test_loop;
    U32 u32_err = eMMC_ST_SUCCESS;
    U8 data;

    struct mmc *mmc = find_mmc_device(0);

    if (!mmc)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no mmc device at slot\n");
        return 1;
    }

    if(!mmc->has_init)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: Need mmc init first!\n");
        return 1;
    }


    eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: Reliable write ext_csd[167] 0x%02x, ext_csd[166] 0x%02x\n", mmc->ext_csd[167], mmc->ext_csd[166]);

    if (mmc->reliable_write == 1)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: Reliable write is supported but unconfigured\n");

        u32_err = eMMC_ModifyExtCSD(eMMC_ExtCSD_WByte, EXT_CSD_WR_REL_SET, 0x1f);
        if(eMMC_ST_SUCCESS != u32_err)
            return u32_err;

        // complete the partition configuration
        u32_err = eMMC_ModifyExtCSD(eMMC_ExtCSD_WByte, EXT_CSD_PARTITION_SETTING_COMPLETED, 0x01);
        if(eMMC_ST_SUCCESS != u32_err)
            return u32_err;

        mmc->reliable_write = 2;

        while (1)
            eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: Please reset the board!!!!!! Reliable write would be active after reset!!!!!!\n");
    }
    else if (mmc->reliable_write == 2)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL, 1,"eMMC Info: Reliable write is supported and has been configured\n");
    }
    else if (mmc->reliable_write == 0)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Info: Reliable write is unsupported\n");
        while(1);
    }

    u32_blk_cnt = (48 *1024 * 1024)>>eMMC_SECTOR_512BYTE_BITS;

    memset(u8_DataBuf, 0, (48 *1024 * 1024));
    //step1
    u32_err = eMMC_ReadData(u8_DataBuf, u32_blk_cnt<<eMMC_SECTOR_512BYTE_BITS, u32_BlkStartAddr);
    if(u32_err != eMMC_ST_SUCCESS)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: Reliable read Fail,Err:%x\n",u32_err);
        return 1;
    }

    data = u8_DataBuf[0];
    for (u32_i=0; u32_i < (48 *1024 * 1024); u32_i++)
    {
        if (u8_DataBuf[u32_i] != data)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: Data Mismatch: Blk:%X  Byte:%X  buf:%X data:%x\n", u32_i>>9,u32_i & 0x1FF, u8_DataBuf[u32_i], data);
            return eMMC_ST_ERR_DATA_MISMATCH;
        }
    }
    eMMC_debug(eMMC_DEBUG_LEVEL,1,"Reliable read compare success\n");

    data = (U8)eMMC_hw_timer_tick();//rRand((unsigned int)data, (unsigned int)(u8_DataBuf - data));
    memset(u8_DataBuf, data, (48 *1024 * 1024));
    u32_test_loop=0;

    u32_err = eMMC_WriteData(u8_DataBuf, (48 *1024 * 1024), u32_BlkStartAddr);
    if(u32_err != eMMC_ST_SUCCESS)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: Reliable write Fail,Err:%x\n",u32_err);
        return u32_err;
    }
    eMMC_debug(eMMC_DEBUG_LEVEL,1,"Reliable write Success\n");

    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"power cut test loop star\n");

    u32_BlkStartAddr += ((48 *1024 * 1024) >> eMMC_SECTOR_512BYTE_BITS);
    u32_blk_cnt = (4 *1024 * 1024)>>eMMC_SECTOR_512BYTE_BITS;
    while(1)
    {
        //step3
        for (u32_i=0; u32_i < u32_blk_cnt; u32_i++)
        {
            u32_err = eMMC_WriteData(u8_DataBuf+(u32_i<<eMMC_SECTOR_512BYTE_BITS), 512, u32_BlkStartAddr+u32_i);
            if(u32_err != eMMC_ST_SUCCESS)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: Reliable write Fail,Err:%x\n",u32_err);
                return u32_err;
            }
        }

        u32_test_loop++;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"Test_loop:%X\n",u32_test_loop);
    }

    return eMMC_ST_SUCCESS;
}

#endif
#endif
