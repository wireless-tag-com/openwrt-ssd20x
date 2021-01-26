/*
* eMMC_platform.c- Sigmastar
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

#if defined(UNIFIED_eMMC_DRIVER) && UNIFIED_eMMC_DRIVER

//=============================================================
#if (defined(eMMC_DRV_CEDRIC_UBOOT)&&eMMC_DRV_CEDRIC_UBOOT)
#if 0
{
#endif
eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_hw_timer_delay(U32 u32us)
{
    #if 0
    volatile int i = 0;

    for (i = 0; i < (u32us>>0); i++)
    {
        #if 1
        volatile int j = 0, tmp;
        for (j = 0; j < 0x2; j++)
        {
            tmp = j;
        }
        #endif
    }
    #else

    extern void udelay(unsigned long usec);
    udelay(u32us);

    #endif
    return u32us;
}

//--------------------------------
// use to performance test
U32 eMMC_hw_timer_start(void)
{
    return 0;
}

U32 eMMC_hw_timer_tick(void)
{
    // TIMER_FREERUN_32K  32 KHz
    // TIMER_FREERUN_XTAL 38.4 MHz,
    // counting down
    return 0;//HalTimerRead(TIMER_FREERUN_32K);
}
//--------------------------------


void eMMC_DumpPadClk(void)
{
#if 0
    //----------------------------------------------
    eMMC_debug(0, 1, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 1, "FCIE 1X (0x%X):0x%X\n", reg_ckg_fcie_1X, REG_FCIE_U16(reg_ckg_fcie_1X));
    eMMC_debug(0, 1, "FCIE 4X (0x%X):0x%X\n", reg_ckg_fcie_4X, REG_FCIE_U16(reg_ckg_fcie_4X));
    eMMC_debug(0, 1, "CMU (0x%X):0x%X\n", reg_ckg_CMU, REG_FCIE_U16(reg_ckg_CMU));

    //----------------------------------------------
    eMMC_debug(0, 1, "\n[pad setting]: ");
    switch(g_eMMCDrv.u32_Flag & FCIE_FLAG_PADTYPE_MASK)
    {
    case FCIE_FLAG_PADTYPE_DDR:
        eMMC_debug(0,0,"DDR\n");  break;
    case FCIE_FLAG_PADTYPE_SDR:
        eMMC_debug(0,0,"SDR\n");  break;
    case FCIE_FLAG_PADTYPE_BYPASS:
        eMMC_debug(0,0,"BYPASS\n");  break;
    default:
        eMMC_debug(0,0,"eMMC Err: Pad unknown\n");  eMMC_die("\n");
    }
    eMMC_debug(0, 1, "chiptop_0x5A (0x%X):0x%X\n", reg_chiptop_0x5A, REG_FCIE_U16(reg_chiptop_0x5A));
    eMMC_debug(0, 1, "chiptop_0x1F (0x%X):0x%X\n", reg_chiptop_0x1F, REG_FCIE_U16(reg_chiptop_0x1F));
    eMMC_debug(0, 1, "chiptop_0x70 (0x%X):0x%X\n", reg_chiptop_0x70, REG_FCIE_U16(reg_chiptop_0x70));
    eMMC_debug(0, 1, "chiptop_0x64 (0x%X):0x%X\n", reg_chiptop_0x64, REG_FCIE_U16(reg_chiptop_0x64));
    eMMC_debug(0, 1, "chiptop_0x4F (0x%X):0x%X\n", reg_chiptop_0x4F, REG_FCIE_U16(reg_chiptop_0x4F));
    eMMC_debug(0, 1, "chiptop_0x03 (0x%X):0x%X\n", reg_chiptop_0x03, REG_FCIE_U16(reg_chiptop_0x03));
    eMMC_debug(0, 1, "chiptop_0x51 (0x%X):0x%X\n", reg_chiptop_0x51, REG_FCIE_U16(reg_chiptop_0x51));
    eMMC_debug(0, 1, "chiptop_0x6F (0x%X):0x%X\n\n", reg_chiptop_0x6F, REG_FCIE_U16(reg_chiptop_0x6F));

    eMMC_debug(0, 1, "chiptop_0x43 (0x%X):0x%X\n", reg_chiptop_0x43, REG_FCIE_U16(reg_chiptop_0x43));
    eMMC_debug(0, 1, "chiptop_0x50 (0x%X):0x%X\n", reg_chiptop_0x50, REG_FCIE_U16(reg_chiptop_0x50));
    eMMC_debug(0, 1, "chiptop_0x0B (0x%X):0x%X\n", reg_chiptop_0x0B, REG_FCIE_U16(reg_chiptop_0x0B));
    eMMC_debug(0, 1, "chiptop_0x5D (0x%X):0x%X\n", reg_chiptop_0x5D, REG_FCIE_U16(reg_chiptop_0x5D));

    eMMC_debug(0, 1, "\n");
#endif
}


#define eMMC_MODE_MASK BIT_MACRO_EN | BIT_SD_DDR_EN | BIT_SD_BYPASS_MODE_EN | BIT_SD_SDR_IN_BYPASS | BIT_SD_FROM_TMUX
U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
//  REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG,
//              BIT_MACRO_EN|BIT_SD_DDR_EN|BIT_SD_BYPASS_MODE_EN|BIT_SD_SDR_IN_BYPASS|BIT_SD_FROM_TMUX);

    switch(u32_FCIE_IF_Type)
    {
        case FCIE_eMMC_DDR:
            printf("eMMC_pads_switch FCIE_eMMC_DDR\r\n");
            #define R_DDR_MODE  BIT_MACRO_EN | BIT_SD_DDR_EN
            REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, eMMC_MODE_MASK);
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, R_DDR_MODE);
            REG_FCIE_SETBIT(RIU_BASE_CHIPTOP + (0x4 << 2), BIT1);
            REG_FCIE_CLRBIT(REG_BPS_MODE, 0x3FF);
            REG_FCIE_CLRBIT(REG_ALL_PAD_IN, BIT0);
            REG_FCIE_SETBIT(REG_EMMC_MODE, BIT13);
  
            REG_FCIE_SETBIT(eMMC_REG_CLK_4X_DIV_EN, BIT0);
            
            // set chiptop
            g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
            break;

        case FCIE_eMMC_SDR:
            printf("eMMC_pads_switch FCIE_eMMC_SDR\r\n");
            #define R_SDR_MODE      BIT_MACRO_EN
            REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, eMMC_MODE_MASK);
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, R_SDR_MODE);
            REG_FCIE_SETBIT(RIU_BASE_CHIPTOP + (0x4 << 2), BIT1);
            REG_FCIE_CLRBIT(REG_BPS_MODE, 0x3FF);
            REG_FCIE_CLRBIT(REG_ALL_PAD_IN, BIT0);
            REG_FCIE_SETBIT(REG_EMMC_MODE, BIT13);
            REG_FCIE_SETBIT(eMMC_REG_CLK_4X_DIV_EN, BIT0);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
            break;
        case FCIE_eMMC_BYPASS:
            printf("eMMC_pads_switch FCIE_eMMC_BYPASS\r\n");
            #define R_BYPASS_MODE       BIT_MACRO_EN | BIT_SD_BYPASS_MODE_EN | BIT_SD_SDR_IN_BYPASS
            REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, eMMC_MODE_MASK);
            REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, R_BYPASS_MODE);
            REG_FCIE_SETBIT(RIU_BASE_CHIPTOP + (0x4 << 2), BIT1);
            REG_FCIE_SETBIT(REG_BPS_MODE, 0x3FF);
            REG_FCIE_CLRBIT(REG_ALL_PAD_IN, BIT0);
            REG_FCIE_SETBIT(REG_EMMC_MODE, BIT13);
            REG_FCIE_CLRBIT(eMMC_REG_CLK_4X_DIV_EN, BIT0);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
            break;

        default:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
            return eMMC_ST_ERR_INVALID_PARAM;
    }

    return eMMC_ST_SUCCESS;
}

 
U32 eMMC_clock_setting(U16 u16_ClkParam)
{
    eMMC_PlatformResetPre();

    switch(u16_ClkParam)    {
    case BIT_FCIE_CLK_24M:      g_eMMCDrv.u32_ClkKHz = 24000;   break;
    case BIT_FCIE_CLK_12M:      g_eMMCDrv.u32_ClkKHz = 12000;   break;
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
    case BIT_FCIE_CLK_86M:      g_eMMCDrv.u32_ClkKHz = 86000;   break;
    case BIT_FCIE_CLK_108M:     g_eMMCDrv.u32_ClkKHz = 108000;  break;
    case BIT_FCIE_CLK_123M:     g_eMMCDrv.u32_ClkKHz = 123000;  break;
    case BIT_FCIE_CLK_144M:     g_eMMCDrv.u32_ClkKHz = 144000;  break;
    case BIT_FCIE_CLK_172M:     g_eMMCDrv.u32_ClkKHz = 172000;  break;
    case BIT_FCIE_CLK_192M:     g_eMMCDrv.u32_ClkKHz = 192000;  break;
    case BIT_FCIE_CLK_216M:     g_eMMCDrv.u32_ClkKHz = 216000;  break;
    case BIT_FCIE_CLK_240M:     g_eMMCDrv.u32_ClkKHz = 240000;  break;
    case BIT_FCIE_CLK_345M:     g_eMMCDrv.u32_ClkKHz = 345000;  break;
    case BIT_FCIE_CLK_432M:     g_eMMCDrv.u32_ClkKHz = 432000;  break;
    case BIT_FCIE_CLK_1200K:    g_eMMCDrv.u32_ClkKHz = 1200;    break;
    case BIT_FCIE_CLK_300K:     g_eMMCDrv.u32_ClkKHz = 300;     break;
    case BIT_FCIE_CLK_32M:      g_eMMCDrv.u32_ClkKHz = 32000;   break;
    case BIT_FCIE_CLK_36M:      g_eMMCDrv.u32_ClkKHz = 36000;   break;
    case BIT_FCIE_CLK_40M:      g_eMMCDrv.u32_ClkKHz = 40000;   break;
    case BIT_FCIE_CLK_43_2M:    g_eMMCDrv.u32_ClkKHz = 43200;   break;
    case BIT_FCIE_CLK_54M:      g_eMMCDrv.u32_ClkKHz = 54000;   break;
    case BIT_FCIE_CLK_62M:      g_eMMCDrv.u32_ClkKHz = 62000;   break;
    case BIT_FCIE_CLK_72M:      g_eMMCDrv.u32_ClkKHz = 72000;   break;
    case BIT_FCIE_CLK_80M:      g_eMMCDrv.u32_ClkKHz = 80000;   break;

    default:
         eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: invalid clk: %Xh\n", u16_ClkParam);
         return eMMC_ST_ERR_INVALID_PARAM;
    }
    
    printf("REG_CLK_EMMC : %X, %X\r\n", u16_ClkParam, g_eMMCDrv.u32_ClkKHz);
    REG_FCIE_W(REG_CLK_EMMC, (u16_ClkParam));   
    g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
    
    eMMC_PlatformResetPost();
    return eMMC_ST_SUCCESS;
}

//U32 eMMC_clock_gating(void)
//{
//    eMMC_PlatformResetPre();
//    g_eMMCDrv.u32_ClkKHz = 0;
//    REG_FCIE_W(reg_ckg_fcie_1X, BIT_FCIE_CLK_Gate);
//    REG_FCIE_W(reg_ckg_fcie_4X, BIT_FCIE_CLK4X_Gate);
//    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
//    eMMC_PlatformResetPost();
//    return eMMC_ST_SUCCESS;
//}

U16 gau16_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={

    BIT_FCIE_CLK_192M, BIT_FCIE_CLK_172M,   /*BIT_FCIE_CLK_144M,*/ BIT_FCIE_CLK_123M, /*BIT_FCIE_CLK_108M,*/
    BIT_FCIE_CLK_86M, /*BIT_FCIE_CLK_80M, BIT_FCIE_CLK_72M,*/   BIT_FCIE_CLK_62M, BIT_FCIE_CLK_54M, 
    /*BIT_FCIE_CLK_48M, BIT_FCIE_CLK_43_2M,*/ BIT_FCIE_CLK_40M, /*BIT_FCIE_CLK_36M,*/   BIT_FCIE_CLK_32M,
    BIT_FCIE_CLK_24M, BIT_FCIE_CLK_12M/*, BIT_FCIE_CLK_1200K, BIT_FCIE_CLK_300K*/
};
#define FCIE_SLOW_CLK_4X                                BIT_FCIE_CLK_12M

#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
static char *sgachar_string[]={"  2T", "1.5T", "2.5T","  1T"}; 

#define DDR_TEST_BLK_CNT        8
#define DDR_TEST_BUFFER_SIZE    (eMMC_SECTOR_512BYTE*DDR_TEST_BLK_CNT)

eMMC_ALIGN0 static U8 gau8_WBuf_DDR[DDR_TEST_BUFFER_SIZE] eMMC_ALIGN1;
eMMC_ALIGN0 static U8 gau8_RBuf_DDR[DDR_TEST_BUFFER_SIZE] eMMC_ALIGN1;

U32 eMMCTest_BlkWRC_ProbeDDR(U32 u32_eMMC_Addr)
{
    U32 u32_err;
    U32 u32_i, u32_j, *pu32_W=(U32*)gau8_WBuf_DDR, *pu32_R=(U32*)gau8_RBuf_DDR;
    U32 u32_BlkCnt=eMMC_TEST_BLK_CNT, u32_BufByteCnt;
    
    //printf("eMMCTest_BlkWRC_ProbeDDR\r\n");
    
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
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i] = u32_i+1;
                break;
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
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0xFF00FF00;
                break;
            case 4: // 0x5AA5
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0x5AA55AA5;
                break;
            case 5: // 0x55AA
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0x55AA55AA;
                break;
            case 6: // 0x5A5A
                #if 0
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0x5A5A5A5A;
                break;
                #else
                continue;
                #endif
            case 7: // 0x0000
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0x00000000;
                break;
            case 8: // 0xFFFF
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0xFFFFFFFF;
                break;
            
        }

        #if 0
        u32_err = eMMC_CMD24_MIU(u32_eMMC_Addr, gau8_WBuf_DDR);
        if(eMMC_ST_SUCCESS != u32_err)
            break;

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
        //eMMC_FCIE_ErrHandler_ReInit();
        eMMC_debug(0,0,"data pattern %u: %02X%02X%02X%02Xh \n\n", 
            u32_j, gau8_WBuf_DDR[3], gau8_WBuf_DDR[2], gau8_WBuf_DDR[1], gau8_WBuf_DDR[0]);
    }

    return u32_err;
}

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel) 
{
    U32 u32_SectorAddr;

    u32_SectorAddr = eMMC_TEST_BLK_0;

    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\ndqs:%s[%Xh]  cell:%02Xh \n",
        sgachar_string[u8_DQS], u8_DQS, u8_DelaySel);

    eMMC_FCIE_SetDDRTimingReg(u8_DQS, u8_DelaySel); 
    return eMMCTest_BlkWRC_ProbeDDR(u32_SectorAddr); 
}

#define FCIE_DELAY_CELL_ts         300 // 0.3ns
static eMMC_FCIE_DDRT_WINDOW_t sg_DDRTWindow[2]; 

U32 eMMC_FCIE_DetectDDRTiming(void)
{
    U8  u8_dqs, u8_delay_sel, u8_i;
    U8  u8_dqs_prev=0xFF, u8_delay_sel_prev=0;
    U8  au8_DQSRegVal[4]={3,1,0,2}; // 0T, 1.5T, 2T, 2.5T
    U8  au8_DQS_10T[4]={0,15,20,25}; // 0T, 1.5T, 2T, 2.5T

    U8  u8_delay_Sel_max;
    U32 u32_ts;
    U32 u32_err;
    eMMC_FCIE_DDRT_WINDOW_t *pWindow = &sg_DDRTWindow[0];
    g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_DDR_TUNING; // just to turn off some log

    //--------------------------------------------------
    sg_DDRTWindow[0].u8_Cnt = 0;
    sg_DDRTWindow[0].aParam[0].u8_DQS = 0;
    sg_DDRTWindow[0].aParam[1].u8_DQS = 0;
    sg_DDRTWindow[1].u8_Cnt = 0;
    sg_DDRTWindow[1].aParam[0].u8_DQS = 0;
    sg_DDRTWindow[1].aParam[1].u8_DQS = 0;
    for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
    {
        sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]=0;
        sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]=0;
    }

    //--------------------------------------------------
    // calculate delay_Sel_max
    u32_ts = 1000*1000*1000 / g_eMMCDrv.u32_ClkKHz;
//  u32_ts >>= 2; // for 4X's 1T

    //--------------------------------------------------
    // no need to try DQS of no delay
    //for(u8_dqs=0; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
    for(u8_dqs=1; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n---------------------------\n");

        if(u8_dqs < (BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT))
            u8_delay_Sel_max =
                (au8_DQS_10T[u8_dqs+1]-au8_DQS_10T[u8_dqs])
                *u32_ts/(FCIE_DELAY_CELL_ts*10);
        else
            u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);

        if(u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT))
        {
            //eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n", u8_delay_Sel_max);
            //eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"invalid, not try\n\n");
            //continue;
            printf("u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT)\r\n");
            u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);
        }
        else
            eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"max delay cell: %u\n\n", u8_delay_Sel_max);

        sg_DDRTWindow[0].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;
        sg_DDRTWindow[1].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;

        //for(u8_delay_sel=0; u8_delay_sel<=(BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT); u8_delay_sel++)
        for(u8_delay_sel=0; u8_delay_sel<=u8_delay_Sel_max; u8_delay_sel++)
        {
            u32_err = eMMC_FCIE_DetectDDRTiming_Ex(au8_DQSRegVal[u8_dqs], u8_delay_sel);
            if(eMMC_ST_SUCCESS == u32_err)
            {
                //printf("DetectDDRTiming pass\r\n");
                eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"  PASS\n\n");
                pWindow->u8_Cnt++;
                if(0 == pWindow->aParam[0].u8_DQS) // save the window head
                {
                    pWindow->aParam[0].u8_DQS = u8_dqs; // dqs uses index
                    pWindow->aParam[0].u8_Cell = u8_delay_sel;
                }
                pWindow->au8_DQSValidCellCnt[u8_dqs]++;

                u8_dqs_prev = u8_dqs;
                u8_delay_sel_prev = u8_delay_sel;
            }
            else
            {   // save the window tail
                if(0xFF != u8_dqs_prev)
                {
                    pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
                    pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
                }
                u8_dqs_prev = 0xFF;

                // discard & re-use the window having less PASS cnt
                pWindow =
                    (sg_DDRTWindow[0].u8_Cnt < sg_DDRTWindow[1].u8_Cnt) ?
                    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];
                pWindow->u8_Cnt = 0;
                pWindow->aParam[0].u8_DQS = 0;
                pWindow->aParam[1].u8_DQS = 0;
                for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
                    pWindow->au8_DQSValidCellCnt[u8_i]=0;
            }
        }
    }

    // for the case of last try is ok
    if(0xFF != u8_dqs_prev)
    {
        pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
        pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
    }

    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_DDR_TUNING; 

    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W0, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
        sg_DDRTWindow[0].u8_Cnt,
        sg_DDRTWindow[0].aParam[0].u8_DQS, sg_DDRTWindow[0].aParam[0].u8_Cell,
        sg_DDRTWindow[0].aParam[1].u8_DQS, sg_DDRTWindow[0].aParam[1].u8_Cell);
    for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
        eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
            u8_i, sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]);

    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n W1, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\n",
        sg_DDRTWindow[1].u8_Cnt,
        sg_DDRTWindow[1].aParam[0].u8_DQS, sg_DDRTWindow[1].aParam[0].u8_Cell,
        sg_DDRTWindow[1].aParam[1].u8_DQS, sg_DDRTWindow[1].aParam[1].u8_Cell);
    for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
        eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"DQSValidCellCnt[%u]:%u \n",
            u8_i, sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]);

    if(sg_DDRTWindow[0].u8_Cnt || sg_DDRTWindow[1].u8_Cnt)
    {
        printf("eMMC_FCIE_DetectDDRTiming OK\r\n");
        return eMMC_ST_SUCCESS;
    }
    else
    {
        printf("eMMC_FCIE_DetectDDRTiming NG\r\n");
        return eMMC_ST_ERR_NO_OK_DDR_PARAM;
    }
}

U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
    U8 au8_DQSRegVal[4]={3,1,0,2}; // 0T, 1.5T, 2T, 2.5T
    U8  u8_i, u8_ClkIdx, u8_SetIdx, u8_tmp, u8_DqsIdx=0, u8_CellBase;
    U32 u32_err, u32_ret=eMMC_ST_ERR_NO_OK_DDR_PARAM;
    eMMC_FCIE_DDRT_WINDOW_t *pWindow;

    eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Info: building DDR table, please wait... \n");
    //printf("eMMC Info: building DDR table, please wait... \n");

    memset((void*)&g_eMMCDrv.DDRTable, '\0', sizeof(g_eMMCDrv.DDRTable));
    u8_SetIdx = 0;

    //iantest ttt
    if(0 == (g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE))
    {
        u32_err = eMMC_FCIE_EnableDDRMode_Ex();
        if(eMMC_ST_SUCCESS != u32_err)
        {
            //RETAILMSG(1, (TEXT("eMMC Error: set DDR IF fail: %Xh\r\n"),u32_err));
            printf("eMMC Error: set DDR IF fail: %Xh\r\n",u32_err);
            return u32_err;
        }
    }
    
    eMMC_pads_switch(FCIE_eMMC_DDR);    //iantest ttt
   //iantest ttt  
    for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
    {

        eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");
        eMMC_clock_setting(gau16_FCIEClkSel[u8_ClkIdx]); 
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"=================================\n");

        // ---------------------------
        // search and set the Windows
        u32_err = eMMC_FCIE_DetectDDRTiming();

        // ---------------------------
        // set the Table
        if(eMMC_ST_SUCCESS == u32_err)
        {
            #if 0
            // before 12MHz, should at least 2 clk is ok for DDR
            if(eMMC_FCIE_DDRT_SET_CNT-1 == u8_SetIdx &&
                BIT_FCIE_CLK_12M == gau16_FCIEClkSel[u8_ClkIdx])
                u32_ret = eMMC_ST_SUCCESS;
            #endif

            //printf("OK\r\n");

            g_eMMCDrv.DDRTable.Set[u8_SetIdx].u16_Clk = gau16_FCIEClkSel[u8_ClkIdx];

            // ---------------------------
            // select Window
            pWindow = NULL;

            // pick up the Window of Cell=0 case
            if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
               sg_DDRTWindow[1].aParam[0].u8_DQS == sg_DDRTWindow[1].aParam[1].u8_DQS)
               pWindow = &sg_DDRTWindow[0];
            else if(sg_DDRTWindow[0].aParam[0].u8_DQS == sg_DDRTWindow[0].aParam[1].u8_DQS &&
               sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
               pWindow = &sg_DDRTWindow[1];
            else if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
               sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
               pWindow =
                    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
                    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

            // ---------------------------
            if(NULL != pWindow)
            {

                // pick up the DQS having max valid cell
                u8_tmp = 0;
                for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
                {
                    if(u8_tmp <= pWindow->au8_DQSValidCellCnt[u8_i]){
                        u8_tmp = pWindow->au8_DQSValidCellCnt[u8_i];
                        u8_DqsIdx = u8_i;
                    }
                }
                if(0 != u8_DqsIdx) // do not use 0T, this should be always TRUE
                {
                    if(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] >= 3 &&
                        pWindow->au8_DQSValidCellCnt[u8_DqsIdx] >= 3)
                    {
                        g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx];
                        g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell = 0; // nice
                    }
                    else
                    {
                        u8_tmp = (pWindow->au8_DQSValidCellCnt[u8_DqsIdx] +
                            pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1]) / 2;


                        if(u8_tmp < pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1])
                        {
                            g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS =
                                au8_DQSRegVal[u8_DqsIdx-1];

                            u8_CellBase = pWindow->au8_DQSTryCellCnt[u8_DqsIdx-1] - pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1];
                            g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
                                u8_CellBase + pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
                                pWindow->au8_DQSValidCellCnt[u8_DqsIdx] - u8_tmp;
                        }
                        else
                        {   g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS =
                                au8_DQSRegVal[u8_DqsIdx];
                            g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
                                (pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
                                pWindow->au8_DQSValidCellCnt[u8_DqsIdx]) / 2;
                        }
                    }
                }
            }

            // ---------------------------
            // or, pick up the Window of large PASS Cnt
            else //if(NULL == pWindow)
            {
                pWindow =
                    (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ?
                    &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

                g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[pWindow->aParam[0].u8_DQS];
                g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
                    (pWindow->aParam[0].u8_Cell + pWindow->aParam[1].u8_Cell)/2;
            }

            #if 0
            // ---------------------------
            // use 12M for Set.Min
            if(eMMC_FCIE_DDRT_SET_CNT-2 == u8_SetIdx)
                u8_ClkIdx = eMMC_FCIE_VALID_CLK_CNT-2;
            #else
            if(FCIE_SLOW_CLK_4X == g_eMMCDrv.DDRTable.Set[u8_SetIdx].u16_Clk)
            {
                printf("FCIE_SLOW_CLK_4X == g_eMMCDrv.DDRTable.Set[u8_SetIdx].u16_Clk\r\n");
                g_eMMCDrv.DDRTable.u8_SetCnt = u8_SetIdx + 1;
            }
            #endif

            u8_SetIdx++;
        }
    }

    // dump DDRT Set
    u8_tmp = eMMC_FCIE_VALID_CLK_CNT > eMMC_FCIE_DDRT_SET_CNT ?
        eMMC_FCIE_DDRT_SET_CNT : eMMC_FCIE_VALID_CLK_CNT;
    u8_tmp = u8_tmp > g_eMMCDrv.DDRTable.u8_SetCnt ?
        g_eMMCDrv.DDRTable.u8_SetCnt : u8_tmp;

    eMMC_DumpDDRTTable();

    // ======================================================
    // CAUTION: expect 48MHz can have valid DDRT parameter
    
    printf("gau16_FCIEClkSel[0] : %X\r\n", gau16_FCIEClkSel[0]);
    printf("g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u16_Clk : %X\r\n", g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u16_Clk);
    printf("g_eMMCDrv.DDRTable.u8_SetCnt : %X\r\n", g_eMMCDrv.DDRTable.u8_SetCnt);
    if(g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u16_Clk == gau16_FCIEClkSel[0]
        && 0 != g_eMMCDrv.DDRTable.u8_SetCnt)
        u32_ret = eMMC_ST_SUCCESS;
    else if(g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u16_Clk != gau16_FCIEClkSel[0])
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: max clk can NOT run DDR\n");
    else if(0 == g_eMMCDrv.DDRTable.u8_SetCnt)
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: min clk can NOT run DDR\n");
    // ======================================================

    // ---------------------------
    // save DDRT Table
    if(eMMC_ST_SUCCESS == u32_ret)
    {

        g_eMMCDrv.DDRTable.u32_ChkSum =
            eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable)-4);
        memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable));

        eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);

        u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
        u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
        if(eMMC_ST_SUCCESS!=u32_err && eMMC_ST_SUCCESS!=u32_ret)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC ERROR: %Xh %Xh\n",
                u32_err, u32_ret);
            return eMMC_ST_ERR_SAVE_DDRT_FAIL;
        }
    }
    else
    {
        gau8_eMMC_SectorBuf[0] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>24);
        gau8_eMMC_SectorBuf[1] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>16);
        gau8_eMMC_SectorBuf[2] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 8);
        gau8_eMMC_SectorBuf[3] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 0);

        eMMC_FCIE_EnableSDRMode();
    }

    return u32_ret;
}

#endif // IF_DETECT_eMMC_DDR_TIMING

void eMMC_set_WatchDog(U8 u8_IfEnable)
{
    // do nothing
}

void eMMC_reset_WatchDog(void)
{
    // do nothing
}

#define MSTAR_MIU0_BUS_BASE                      0x40000000
#define MSTAR_MIU1_BUS_BASE                      0xA0000000


U32 eMMC_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt, int mode)
{
#if 0
        extern void Chip_Clean_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
        extern void Chip_Flush_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
    //mode 0 for write, 1 for read
    if( mode == WRITE_TO_eMMC ) //Write
    {
        //Write (DRAM->NAND)-> flush
        Chip_Clean_Cache_Range_VA_PA(u32_DMAAddr,__pa(u32_DMAAddr), u32_ByteCnt);
    }
    else //Read
    {
        //Read (NAND->DRAM) -> inv
        Chip_Flush_Cache_Range_VA_PA(u32_DMAAddr,__pa(u32_DMAAddr), u32_ByteCnt);
    }

//    if(virt_to_phys((void *)u32_DMAAddr) >= MSTAR_MIU1_BUS_BASE) 
//    {
//        REG_SET_BITS_UINT16( NC_MIU_DMA_SEL, BIT_MIU1_SELECT);
//    }
//    else {
//        REG_CLR_BITS_UINT16( NC_MIU_DMA_SEL, BIT_MIU1_SELECT);
//    }

    return virt_to_phys((void *)u32_DMAAddr);
#else
    flush_cache(u32_DMAAddr, u32_ByteCnt); 
    return (u32_DMAAddr);
#endif
}

//void eMMC_Invalidate_data_cache_buffer(U32 u32_addr, S32 s32_size)
//{
//    flush_cache(u32_addr, s32_size);
//}
//
//void eMMC_flush_miu_pipe(void)
//{
//
//}

U32 eMMC_PlatformResetPre(void)
{

    return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{

    return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
    eMMC_pads_switch(FCIE_eMMC_SDR);
    eMMC_clock_setting(FCIE_SLOWEST_CLK);

    return eMMC_ST_SUCCESS;
}


U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
    switch(u16_PartType)
    {
    case eMMC_PART_BL:
        u32_StartSector += BL_BLK_OFFSET;
        break;

    case eMMC_PART_OTP:
        u32_StartSector += OTP_BLK_OFFSET;
        break;

    case eMMC_PART_SECINFO:
        u32_StartSector += SecInfo_BLK_OFFSET;
        break;

    default:
        return eMMC_ST_SUCCESS;
    }

    eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

    if(eMMC_BOOT_PART_W == u8_OP)
        return eMMC_WriteBootPart(pDataBuf,
            u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
            u32_StartSector, 1);
    else
        return eMMC_ReadBootPart(pDataBuf,
            u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
            u32_StartSector, 1);

}


U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
    U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

    switch(u16_PartType)
    {
    case eMMC_PART_BL:
        u32_eMMCBlkAddr_start = 0;
        u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
        break;

    case eMMC_PART_OTP:
        u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
        u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
        break;

    case eMMC_PART_SECINFO:
        u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
        u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
        break;

    default:
        return eMMC_ST_SUCCESS;
    }

    eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
        u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

}

char *gpas8_eMMCPartName[]={
    "e2pbak", "nvrambak", "hwcfgs", "recovery", "os",
    "fdd", "tdd", "blogo", "apanic","misc", "cus",
    "e2p0","e2p1","nvram0","nvram1", "system","cache", "data", "internal sd"};

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

U32 eMMC_CheckIfMemCorrupt(void)
{
    if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
        return eMMC_ST_ERR_MEM_CORRUPT;

    return eMMC_ST_SUCCESS;
}


#if 0
}
#endif

//=============================================================
#elif (defined(eMMC_DRV_CHICAGO_UBOOT)&&eMMC_DRV_CHICAGO_UBOOT)
#if 0
{
#endif
eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

U32 eMMC_hw_timer_delay(U32 u32us) 
{
    #if 0
    volatile int i = 0;

    for (i = 0; i < (u32us>>0); i++)
    {
    #if 1
        volatile int j = 0, tmp;
        for (j = 0; j < 0x2; j++)
        {
            tmp = j;
        }
    #endif
    }
    #else
    volatile U32 u32_i=u32us;
    //udelay((u32us>>1)+(u32us>>4)+1);
    while(u32_i>1000)
    {
        udelay(1000);
        u32_i-=1000;
    }

    udelay(u32_i);
    #endif
    return u32us;

}

//--------------------------------
// use to performance test
U32 eMMC_hw_timer_start(void)
{
    return 0;
}

U32 eMMC_hw_timer_tick(void)
{
    // TIMER_FREERUN_32K  32 KHz
    // TIMER_FREERUN_XTAL 38.4 MHz,
    // counting down
    return 0;//HalTimerRead(TIMER_FREERUN_32K);
}
//--------------------------------


void eMMC_DumpPadClk(void)
{

    //----------------------------------------------
    eMMC_debug(0, 1, "\n[clk setting]: %uKHz \n", g_eMMCDrv.u32_ClkKHz);
    eMMC_debug(0, 1, "FCIE 1X (0x%X):0x%X\n", REG_CLK_EMMC, REG_FCIE_U16(REG_CLK_EMMC));
    eMMC_debug(0, 1, "FCIE 4X (0x%X):0x%X\n", REG_CLK_4X_DIV_EN, REG_FCIE_U16(REG_CLK_4X_DIV_EN));

    //----------------------------------------------
    eMMC_debug(0, 1, "\n[pad setting]: ");
    switch(g_eMMCDrv.u8_PadType)
    {
        case FCIE_eMMC_DDR:
            eMMC_debug(0,0,"DDR\n");
            break;
        case FCIE_eMMC_SDR:
            eMMC_debug(0,0,"SDR\n");
            break;
        case FCIE_eMMC_BYPASS:
            eMMC_debug(0,0,"BYPASS\n");
            break;
        default:
            eMMC_debug(0,0,"eMMC Err: Pad unknown\n");
            eMMC_die("\n");
    }
    eMMC_debug(0, 1, "padtop_0x00 (0x%X):0x%X\n", REG_PADTOP_00, REG_FCIE_U16(REG_PADTOP_00));
    eMMC_debug(0, 1, "padtop_0x01 (0x%X):0x%X\n", REG_PADTOP_01, REG_FCIE_U16(REG_PADTOP_01));
    eMMC_debug(0, 1, "padtop_0x40 (0x%X):0x%X\n", REG_PADTOP_40, REG_FCIE_U16(REG_PADTOP_40));
    eMMC_debug(0, 1, "padtop_0x43 (0x%X):0x%X\n", REG_PADTOP_43, REG_FCIE_U16(REG_PADTOP_43));
    eMMC_debug(0, 1, "chiptop_0x31 (0x%X):0x%X\n", REG_RESET_PIN, REG_FCIE_U16(REG_RESET_PIN));

    eMMC_debug(0, 1, "fcie_0x2F (0x%X):0x%X\n", FCIE_BOOT_CONFIG, REG_FCIE_U16(FCIE_BOOT_CONFIG));
    eMMC_debug(0, 1, "fcie_0x2D (0x%X):0x%X\n", FCIE_REORDER, REG_FCIE_U16(FCIE_REORDER));

    eMMC_debug(0, 1, "\n");
}  

U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type)
{
    U16 u16_reg;
    switch(u32_FCIE_IF_Type)
    {
        case FCIE_eMMC_DDR:
            printf("eMMC_pads_switch FCIE_eMMC_DDR\r\n");

            REG_FCIE_R(FCIE_BOOT_CONFIG, u16_reg);
            u16_reg &= ~(BIT_MACRO_EN | BIT_SD_DDR_EN | BIT_SD_BYPASS_MODE_EN | BIT_SD_SDR_IN_BYPASS | BIT_SD_FROM_TMUX);
            u16_reg |= (BIT_MACRO_EN | BIT_SD_DDR_EN);
            REG_FCIE_W(FCIE_BOOT_CONFIG, u16_reg);

            REG_FCIE_CLRBIT(FCIE_REORDER, BIT14);

            //CLRREG16(FCIE_BOOT_CONFIG, BIT3); //to MIU
            REG_FCIE_R(REG_PADTOP_00, u16_reg);
            u16_reg |= BIT15;
            u16_reg &= ~(BIT0 + BIT14);
            REG_FCIE_W(REG_PADTOP_00, u16_reg);

            REG_FCIE_R(REG_PADTOP_01, u16_reg);
            u16_reg &= ~(BIT3);
            REG_FCIE_W(REG_PADTOP_01, u16_reg);

            REG_FCIE_R(REG_PADTOP_40, u16_reg);
            u16_reg &= ~(BIT3 + BIT8 + BIT9);
            REG_FCIE_W(REG_PADTOP_40, u16_reg);

            REG_FCIE_SETBIT(REG_PADTOP_43, BIT3);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_DDR;
            break;

        case FCIE_eMMC_SDR:
            printf("eMMC_pads_switch FCIE_eMMC_SDR\r\n");
        
            REG_FCIE_R(FCIE_BOOT_CONFIG, u16_reg);
            u16_reg &= ~(BIT_MACRO_EN | BIT_SD_DDR_EN | BIT_SD_BYPASS_MODE_EN | BIT_SD_SDR_IN_BYPASS | BIT_SD_FROM_TMUX);
            u16_reg |= (BIT_MACRO_EN);
            REG_FCIE_W(FCIE_BOOT_CONFIG, u16_reg);

            REG_FCIE_CLRBIT(FCIE_REORDER, BIT14);

            //CLRREG16(FCIE_BOOT_CONFIG, BIT3); //to MIU
            REG_FCIE_R(REG_PADTOP_00, u16_reg);
            u16_reg |= BIT15;
            u16_reg &= ~(BIT0 + BIT14);
            REG_FCIE_W(REG_PADTOP_00, u16_reg);

            REG_FCIE_R(REG_PADTOP_01, u16_reg);
            u16_reg &= ~(BIT3);
            REG_FCIE_W(REG_PADTOP_01, u16_reg);

            REG_FCIE_R(REG_PADTOP_40, u16_reg);
            u16_reg &= ~(BIT3 + BIT8 + BIT9);
            REG_FCIE_W(REG_PADTOP_40, u16_reg);

            REG_FCIE_SETBIT(REG_PADTOP_43, BIT3);
            g_eMMCDrv.u8_PadType = FCIE_eMMC_SDR;
            break;

        case FCIE_eMMC_BYPASS:
            printf("eMMC_pads_switch FCIE_eMMC_BYPASS\r\n");
            REG_FCIE_R(FCIE_BOOT_CONFIG, u16_reg);
            u16_reg &= ~(BIT_MACRO_EN | BIT_SD_DDR_EN | BIT_SD_BYPASS_MODE_EN | BIT_SD_SDR_IN_BYPASS | BIT_SD_FROM_TMUX);
            u16_reg |= (BIT_MACRO_EN | BIT_SD_BYPASS_MODE_EN | BIT_SD_SDR_IN_BYPASS);
            REG_FCIE_W(FCIE_BOOT_CONFIG, u16_reg);

            REG_FCIE_CLRBIT(FCIE_REORDER, BIT14);

            //CLRREG16(FCIE_BOOT_CONFIG, BIT3); //to MIU

            REG_FCIE_R(REG_PADTOP_00, u16_reg);
            u16_reg |= BIT15;
            u16_reg &= ~(BIT0 + BIT14);
            REG_FCIE_W(REG_PADTOP_00, u16_reg);

            REG_FCIE_R(REG_PADTOP_01, u16_reg);
            u16_reg &= ~(BIT3);
            REG_FCIE_W(REG_PADTOP_01, u16_reg);

            REG_FCIE_R(REG_PADTOP_40, u16_reg);
            u16_reg |= (BIT3 + BIT8 + BIT9);
            REG_FCIE_W(REG_PADTOP_40, u16_reg);

            REG_FCIE_SETBIT(REG_PADTOP_43, BIT3);

            g_eMMCDrv.u8_PadType = FCIE_eMMC_BYPASS;
            break;

        default:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown interface: %X\n",u32_FCIE_IF_Type);
            return eMMC_ST_ERR_INVALID_PARAM;
    }

    return eMMC_ST_SUCCESS;
}

U32 eMMC_clock_setting(U16 u16_ClkParam)
{
    eMMC_PlatformResetPre();
    REG_FCIE_SETBIT(REG_CLK_4X_DIV_EN, BIT0);

    switch(u16_ClkParam) {
    case NFIE_REG_4XCLK_1_3M:       g_eMMCDrv.u32_ClkKHz = 1300;    break;
    case NFIE_REG_4XCLK_26M:        g_eMMCDrv.u32_ClkKHz = 26000;   break;
    case NFIE_REG_4XCLK_48M:        g_eMMCDrv.u32_ClkKHz = 48000;   break;
    case NFIE_REG_4XCLK_80M:        g_eMMCDrv.u32_ClkKHz = 80000;   break;
    case NFIE_REG_4XCLK_96M:        g_eMMCDrv.u32_ClkKHz = 96000;   break;
    case NFIE_REG_4XCLK_120M:       g_eMMCDrv.u32_ClkKHz = 120000;  break;
    case NFIE_REG_4XCLK_147M:       g_eMMCDrv.u32_ClkKHz =  147000; break;
    case NFIE_REG_4XCLK_160M:       g_eMMCDrv.u32_ClkKHz =  160000; break;
    case NFIE_REG_4XCLK_176_8M:     g_eMMCDrv.u32_ClkKHz = 176800;  break;
    case NFIE_REG_4XCLK_192M:       g_eMMCDrv.u32_ClkKHz = 192000;  break;
    case NFIE_REG_4XCLK_221M:       g_eMMCDrv.u32_ClkKHz = 221000;  break;
    case NFIE_REG_4XCLK_240M:       g_eMMCDrv.u32_ClkKHz = 240000;  break;
    case NFIE_REG_4XCLK_294_6M:     g_eMMCDrv.u32_ClkKHz = 294600;  break;
    case NFIE_REG_4XCLK_353_6M:     g_eMMCDrv.u32_ClkKHz = 353600;  break;
    case NFIE_REG_4XCLK_60M:        g_eMMCDrv.u32_ClkKHz =  60000;  break;
        default:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: invalid clk: %Xh\n", u16_ClkParam);
            return eMMC_ST_ERR_INVALID_PARAM;
    }

    //printf("REG_CLK_EMMC : %X, %uKHZ\r\n", u16_ClkParam, g_eMMCDrv.u32_ClkKHz/4);

    REG_FCIE_W(REG_CLK_EMMC, (u16_ClkParam));
    g_eMMCDrv.u16_ClkRegVal = (U16)u16_ClkParam;
    eMMC_PlatformResetPost();
    return eMMC_ST_SUCCESS;
}

U16 gau16_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT]={
    NFIE_REG_4XCLK_192M, NFIE_REG_4XCLK_160M, NFIE_REG_4XCLK_120M,
    NFIE_REG_4XCLK_96M ,NFIE_REG_4XCLK_80M
};


#if defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING 
static char *sgachar_string[]={"  0T", "0.5T", "  1T", "1.5T", "  2T", "2.5T", "  3T", "3.5T"};

#define DDR_TEST_BLK_CNT        8
#define DDR_TEST_BUFFER_SIZE    (eMMC_SECTOR_512BYTE*DDR_TEST_BLK_CNT)

eMMC_ALIGN0 static U8 gau8_WBuf_DDR[DDR_TEST_BUFFER_SIZE] eMMC_ALIGN1;
eMMC_ALIGN0 static U8 gau8_RBuf_DDR[DDR_TEST_BUFFER_SIZE] eMMC_ALIGN1;

U32 eMMCTest_BlkWRC_ProbeDDR(U32 u32_eMMC_Addr)
{
    U32 u32_err;
    U32 u32_i, u32_j, *pu32_W=(U32*)gau8_WBuf_DDR, *pu32_R=(U32*)gau8_RBuf_DDR;
    U32 u32_BlkCnt=eMMC_TEST_BLK_CNT, u32_BufByteCnt;
    
    //printf("eMMCTest_BlkWRC_ProbeDDR\r\n");
    
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
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i] = u32_i+1;
                break;
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
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0xFF00FF00;
                break;
            case 4: // 0x5AA5
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0x5AA55AA5;
                break;
            case 5: // 0x55AA
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0x55AA55AA;
                break;  
            case 6: // 0x5A5A
                #if 0
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0x5A5A5A5A;
                break;
                #else
                continue;
                #endif
            case 7: // 0x0000
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0x00000000;
                break;
            case 8: // 0xFFFF
                for(u32_i=0; u32_i<u32_BufByteCnt>>2; u32_i++)
                    pu32_W[u32_i]=0xFFFFFFFF;
                break;
            
        }

        #if 0
        u32_err = eMMC_CMD24_MIU(u32_eMMC_Addr, gau8_WBuf_DDR);
        if(eMMC_ST_SUCCESS != u32_err)
            break;

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
        //eMMC_FCIE_ErrHandler_ReInit();
        eMMC_debug(0,0,"data pattern %u: %02X%02X%02X%02Xh \n\n", 
            u32_j, gau8_WBuf_DDR[3], gau8_WBuf_DDR[2], gau8_WBuf_DDR[1], gau8_WBuf_DDR[0]);
    }

    return u32_err;
}

static U32 eMMC_FCIE_DetectDDRTiming_Ex(U8 u8_DQS, U8 u8_DelaySel)
{
    U32 u32_SectorAddr;

    u32_SectorAddr = eMMC_TEST_BLK_0;

    eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\ndqs:%s[%Xh]  cell:%02Xh \n",
        sgachar_string[u8_DQS], u8_DQS, u8_DelaySel);

    eMMC_FCIE_SetDDRTimingReg(u8_DQS, u8_DelaySel); 
    return eMMCTest_BlkWRC_ProbeDDR(u32_SectorAddr); 
}

#define FCIE_DELAY_CELL_ts         300 // 0.3ns
static eMMC_FCIE_DDRT_WINDOW_t sg_DDRTWindow[2];

U32 eMMC_FCIE_DetectDDRTiming(void)
{
    U8  u8_dqs, u8_delay_sel, u8_i;
    U8  u8_dqs_prev=0xFF, u8_delay_sel_prev=0;
    U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3,5T
    U8  au8_DQS_10T[8]={0,5,10,15,20,25,30,35}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3,5T
    U8  u8_delay_Sel_max;
    U32 u32_ts;
    U32 u32_err;
    eMMC_FCIE_DDRT_WINDOW_t *pWindow = &sg_DDRTWindow[0];

//    if(0 == (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_DDR_TUNING))
//    {
//        u32_err = eMMC_FCIE_EnableDDRMode();
//        if(eMMC_ST_SUCCESS != u32_err)
//        {
//            RETAILMSG(1, (TEXT("eMMC Error: set DDR IF fail: %Xh\r\n"),u32_err));
//            return u32_err;
//        }
//    }

    g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_DDR_TUNING; // just to turn off some log 

    //--------------------------------------------------
    sg_DDRTWindow[0].u8_Cnt = 0;
    sg_DDRTWindow[0].aParam[0].u8_DQS = 0;
    sg_DDRTWindow[0].aParam[1].u8_DQS = 0;
    sg_DDRTWindow[1].u8_Cnt = 0;
    sg_DDRTWindow[1].aParam[0].u8_DQS = 0;
    sg_DDRTWindow[1].aParam[1].u8_DQS = 0;
    for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
    {
        sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]=0;
        sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]=0;
    }

    //--------------------------------------------------
    // calculate delay_Sel_max
    u32_ts = 1000*1000*1000 / g_eMMCDrv.u32_ClkKHz; 
    //u32_ts >>= 2; // for 4X's 1T  //temptemptemp

    //--------------------------------------------------
    //no need to try DQS of no delay
    //for(u8_dqs=0; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
    for(u8_dqs=1; u8_dqs<=(BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT); u8_dqs++)
    {
#if 1
        if(u8_dqs < (BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT))
            u8_delay_Sel_max = (U8)((au8_DQS_10T[u8_dqs+1]-au8_DQS_10T[u8_dqs])*u32_ts/(FCIE_DELAY_CELL_ts*10));
        else
            u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);

        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"max delay cell: %X\r\n", u8_delay_Sel_max);
        if(u8_delay_Sel_max > (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT))
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"let u8_delay_Sel_max be (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT)\r\n");
            u8_delay_Sel_max = (BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT);
        }
#else
        u8_delay_Sel_max = 0;   //let delay_sel always be 0
#endif

        sg_DDRTWindow[0].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;
        sg_DDRTWindow[1].au8_DQSTryCellCnt[u8_dqs] = u8_delay_Sel_max;

        //for(u8_delay_sel=0; u8_delay_sel<=(BIT_DQS_DELAY_CELL_MASK>>BIT_DQS_DELAY_CELL_SHIFT); u8_delay_sel++)
        for(u8_delay_sel=0; u8_delay_sel<=u8_delay_Sel_max; u8_delay_sel++)
        {
            //RETAILMSG(1, (TEXT("start LA\r\n")));
            //eMMC_hw_timer_delay(HW_TIMER_DELAY_3s);
            u32_err = eMMC_FCIE_DetectDDRTiming_Ex(au8_DQSRegVal[u8_dqs], u8_delay_sel);
            if(eMMC_ST_SUCCESS == u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC_FCIE_DetectDDRTiming_Ex PASS\r\n");

                pWindow->u8_Cnt++;
                if(0 == pWindow->aParam[0].u8_DQS) // save the window head
                {
                    pWindow->aParam[0].u8_DQS = u8_dqs; // dqs uses index
                    pWindow->aParam[0].u8_Cell = u8_delay_sel;
                }
                pWindow->au8_DQSValidCellCnt[u8_dqs]++;

                u8_dqs_prev = u8_dqs;
                u8_delay_sel_prev = u8_delay_sel;
            }
            else
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC_FCIE_DetectDDRTiming_Ex Fail\r\n");
               // save the window tail
                if(0xFF != u8_dqs_prev)
                {
                    pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
                    pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
                }
                u8_dqs_prev = 0xFF;

                // discard & re-use the window having less PASS cnt
                pWindow = (sg_DDRTWindow[0].u8_Cnt < sg_DDRTWindow[1].u8_Cnt) ? &sg_DDRTWindow[0] : &sg_DDRTWindow[1];
                pWindow->u8_Cnt = 0;
                pWindow->aParam[0].u8_DQS = 0;
                for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
                    pWindow->au8_DQSValidCellCnt[u8_i]=0;
            }
        }
    }

    // for the case of last try is ok
    if(0xFF != u8_dqs_prev)
    {
        pWindow->aParam[1].u8_DQS = u8_dqs_prev; // dqs uses index
        pWindow->aParam[1].u8_Cell = u8_delay_sel_prev;
    }

    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_DDR_TUNING;

    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"W0, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\r\n",
                sg_DDRTWindow[0].u8_Cnt,
                sg_DDRTWindow[0].aParam[0].u8_DQS, sg_DDRTWindow[0].aParam[0].u8_Cell,
                sg_DDRTWindow[0].aParam[1].u8_DQS, sg_DDRTWindow[0].aParam[1].u8_Cell);

    for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"DQSValidCellCnt[%X]:%X\r\n",u8_i, sg_DDRTWindow[0].au8_DQSValidCellCnt[u8_i]);

    //---
    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"W1, Cnt:%Xh, [%Xh %Xh], [%Xh %Xh]\r\n",
                sg_DDRTWindow[1].u8_Cnt,
                sg_DDRTWindow[1].aParam[0].u8_DQS, sg_DDRTWindow[1].aParam[0].u8_Cell,
                sg_DDRTWindow[1].aParam[1].u8_DQS, sg_DDRTWindow[1].aParam[1].u8_Cell);

    for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"DQSValidCellCnt[%X]:%X\r\n",u8_i, sg_DDRTWindow[1].au8_DQSValidCellCnt[u8_i]);

    if(sg_DDRTWindow[0].u8_Cnt || sg_DDRTWindow[1].u8_Cnt)
        return eMMC_ST_SUCCESS;
    else
        return eMMC_ST_ERR_NO_OK_DDR_PARAM;
}

U32 eMMC_FCIE_BuildDDRTimingTable(void)
{
    U8  au8_DQSRegVal[8]={0,1,2,3,4,5,6,7}; // 0T, 0.5T, 1T, 1.5T, 2T, 2.5T, 3T, 3,5T
    U8  u8_i, u8_ClkIdx, u8_SetIdx, u8_tmp, u8_DqsIdx=0, u8_CellBase;
    U32 u32_err, u32_ret=eMMC_ST_ERR_NO_OK_DDR_PARAM;
    eMMC_FCIE_DDRT_WINDOW_t *pWindow;

    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC Info: building DDR table, please wait...\r\n");

    memset((void*)&g_eMMCDrv.DDRTable, '\0', sizeof(g_eMMCDrv.DDRTable));
    u8_SetIdx = 0;

    if(0 == (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_DDR_MODE))
    {
        u32_err = eMMC_FCIE_EnableDDRMode_Ex();
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC Error: set DDR IF fail: %Xh\r\n", u32_err);
            return u32_err;
        }
    }
    for(u8_ClkIdx=0; u8_ClkIdx<eMMC_FCIE_VALID_CLK_CNT; u8_ClkIdx++)
    {
        eMMC_clock_setting(gau16_FCIEClkSel[u8_ClkIdx]);

        // ---------------------------
        // search and set the Windows
        u32_err = eMMC_FCIE_DetectDDRTiming();

        // ---------------------------
        // set the Table
        if(eMMC_ST_SUCCESS == u32_err)
        {
            g_eMMCDrv.DDRTable.Set[u8_SetIdx].u16_Clk = gau16_FCIEClkSel[u8_ClkIdx];

            // ---------------------------
            // select Window
            pWindow = NULL;

            // pick up the Window of Cell=0 case
            if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
               sg_DDRTWindow[1].aParam[0].u8_DQS == sg_DDRTWindow[1].aParam[1].u8_DQS)
                    pWindow = &sg_DDRTWindow[0];
            else if(sg_DDRTWindow[0].aParam[0].u8_DQS == sg_DDRTWindow[0].aParam[1].u8_DQS &&
               sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
                    pWindow = &sg_DDRTWindow[1];
            else if(sg_DDRTWindow[0].aParam[0].u8_DQS != sg_DDRTWindow[0].aParam[1].u8_DQS &&
               sg_DDRTWindow[1].aParam[0].u8_DQS != sg_DDRTWindow[1].aParam[1].u8_DQS)
                    pWindow =   (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ? &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

            // ---------------------------
            if(NULL != pWindow)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"NULL != pWindow\r\n");
                // pick up the DQS having max valid cell
                u8_tmp = 0;
                for(u8_i=1; u8_i <= BIT_DQS_MODE_MASK>>BIT_DQS_MDOE_SHIFT; u8_i++)
                {
                    if(u8_tmp <= pWindow->au8_DQSValidCellCnt[u8_i])
                    {
                        u8_tmp = pWindow->au8_DQSValidCellCnt[u8_i];
                        u8_DqsIdx = u8_i;
                    }
                }
                if(0 != u8_DqsIdx) // do not use 0T, this should be always TRUE
                {
                    if(pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] >= 3 && pWindow->au8_DQSValidCellCnt[u8_DqsIdx] >= 3)
                    {
                        g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx];
                        g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell = 0; // nice
                    }
                    else
                    {
                        u8_tmp = (pWindow->au8_DQSValidCellCnt[u8_DqsIdx] + pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1]) / 2;
                        if(u8_tmp < pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1])
                        {
                            g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx-1];
                            u8_CellBase = pWindow->au8_DQSTryCellCnt[u8_DqsIdx-1] - pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1];
                            g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
                                    u8_CellBase + pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
                                    pWindow->au8_DQSValidCellCnt[u8_DqsIdx] - u8_tmp;
                        }
                        else
                        {
                            g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[u8_DqsIdx];
                            g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell =
                                        (pWindow->au8_DQSValidCellCnt[u8_DqsIdx-1] +
                                        pWindow->au8_DQSValidCellCnt[u8_DqsIdx]) / 2;
                        }
                    }
                }
            }

            // ---------------------------
            // or, pick up the Window of large PASS Cnt
            else //if(NULL == pWindow)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"NULL == pWindow\r\n");
                pWindow = (sg_DDRTWindow[0].u8_Cnt > sg_DDRTWindow[1].u8_Cnt) ? &sg_DDRTWindow[0] : &sg_DDRTWindow[1];

                g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_DQS = au8_DQSRegVal[pWindow->aParam[0].u8_DQS];
                g_eMMCDrv.DDRTable.Set[u8_SetIdx].Param.u8_Cell = (pWindow->aParam[0].u8_Cell + pWindow->aParam[1].u8_Cell)/2;
            }

            if(FCIE_SLOW_CLK == g_eMMCDrv.DDRTable.Set[u8_SetIdx].u16_Clk)
                g_eMMCDrv.DDRTable.u8_SetCnt = u8_SetIdx + 1;

            u8_SetIdx++;
        }
    }

    // dump DDRT Set
    u8_tmp = eMMC_FCIE_VALID_CLK_CNT > eMMC_FCIE_DDRT_SET_CNT ? eMMC_FCIE_DDRT_SET_CNT : eMMC_FCIE_VALID_CLK_CNT;
    u8_tmp = u8_tmp > g_eMMCDrv.DDRTable.u8_SetCnt ? g_eMMCDrv.DDRTable.u8_SetCnt : u8_tmp;

    eMMC_DumpDDRTTable();

    // ======================================================
    // CAUTION: expect 48MHz can have valid DDRT parameter
    if ((g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u16_Clk == gau16_FCIEClkSel[0]) && (0 != g_eMMCDrv.DDRTable.u8_SetCnt))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"((g_eMMCDrv.DDRTable.Set[eMMC_DDRT_SET_MAX].u16_Clk == gau16_FCIEClkSel[0]) && (0 != g_eMMCDrv.DDRTable.u8_SetCnt))");
        u32_ret = eMMC_ST_SUCCESS;
    }
    else
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"to modify");
        while(1);
    }
    // ======================================================

    // ---------------------------
    // save DDRT Table
    if(eMMC_ST_SUCCESS == u32_ret)
    {
        g_eMMCDrv.DDRTable.u32_ChkSum = eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable)-4);
        memcpy(gau8_eMMC_SectorBuf, (U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable));

        eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);

        u32_err = eMMC_CMD24(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf);
        u32_ret = eMMC_CMD24(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf);
        if  ((eMMC_ST_SUCCESS!=u32_err) || (eMMC_ST_SUCCESS!=u32_ret))
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC ERROR: %Xh %Xh\r\n",u32_err, u32_ret);
            return eMMC_ST_ERR_SAVE_DDRT_FAIL;
        }
    }
    else
    {
        gau8_eMMC_SectorBuf[0] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>24);
        gau8_eMMC_SectorBuf[1] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>>16);
        gau8_eMMC_SectorBuf[2] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 8);
        gau8_eMMC_SectorBuf[3] = (U8)~(eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable))>> 0);

        eMMC_FCIE_EnableSDRMode();
    }

    return u32_ret;
}

#endif // IF_DETECT_eMMC_DDR_TIMING

void eMMC_set_WatchDog(U8 u8_IfEnable)
{
    // do nothing
}

void eMMC_reset_WatchDog(void)
{
    // do nothing
}

#define MSTAR_MIU0_BUS_BASE                      0x40000000
#define MSTAR_MIU1_BUS_BASE                      0xA0000000


U32 eMMC_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt, int mode)
{
#if 0
        extern void Chip_Clean_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
        extern void Chip_Flush_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
    //mode 0 for write, 1 for read
    if( mode == WRITE_TO_eMMC ) //Write
    {
        //Write (DRAM->NAND)-> flush
        Chip_Clean_Cache_Range_VA_PA(u32_DMAAddr,__pa(u32_DMAAddr), u32_ByteCnt);
    }
    else //Read
    {
        //Read (NAND->DRAM) -> inv
        Chip_Flush_Cache_Range_VA_PA(u32_DMAAddr,__pa(u32_DMAAddr), u32_ByteCnt);
    }
    /*
    if(virt_to_phys((void *)u32_DMAAddr) >= MSTAR_MIU1_BUS_BASE) 
    {
        REG_SET_BITS_UINT16( NC_MIU_DMA_SEL, BIT_MIU1_SELECT);
    }
    else
        REG_CLR_BITS_UINT16( NC_MIU_DMA_SEL, BIT_MIU1_SELECT);
        */

    return virt_to_phys((void *)u32_DMAAddr);
#else
    flush_cache(u32_DMAAddr, u32_ByteCnt); 
    return (u32_DMAAddr);
#endif
}

//void eMMC_Invalidate_data_cache_buffer(U32 u32_addr, S32 s32_size)
//{
//    flush_cache(u32_addr, s32_size);
//}
//
//void eMMC_flush_miu_pipe(void)
//{
//
//}

U32 eMMC_PlatformResetPre(void)
{

    return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{

    return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
    eMMC_pads_switch(FCIE_eMMC_SDR);
    eMMC_clock_setting(FCIE_SLOWEST_CLK);

    return eMMC_ST_SUCCESS;
}


U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
    switch(u16_PartType)
    {
    case eMMC_PART_BL:
        u32_StartSector += BL_BLK_OFFSET;
        break;

    case eMMC_PART_OTP:
        u32_StartSector += OTP_BLK_OFFSET;
        break;

    case eMMC_PART_SECINFO:
        u32_StartSector += SecInfo_BLK_OFFSET;
        break;

    default:
        return eMMC_ST_SUCCESS;
    }

    eMMC_debug(eMMC_DEBUG_LEVEL,1,"SecAddr: %Xh, SecCnt: %Xh\n", u32_StartSector, u32_SectorCnt);

    if(eMMC_BOOT_PART_W == u8_OP)
        return eMMC_WriteBootPart(pDataBuf,
            u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
            u32_StartSector, 1);
    else
        return eMMC_ReadBootPart(pDataBuf,
            u32_SectorCnt<<eMMC_SECTOR_512BYTE_BITS,
            u32_StartSector, 1);

}

U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
    U32 u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end;

    switch(u16_PartType)
    {
    case eMMC_PART_BL:
        u32_eMMCBlkAddr_start = 0;
        u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+BL_BLK_CNT-1;
        break;

    case eMMC_PART_OTP:
        u32_eMMCBlkAddr_start = OTP_BLK_OFFSET;
        u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+OTP_BLK_CNT-1;
        break;

    case eMMC_PART_SECINFO:
        u32_eMMCBlkAddr_start = SecInfo_BLK_OFFSET;
        u32_eMMCBlkAddr_end = u32_eMMCBlkAddr_start+SecInfo_BLK_CNT-1;
        break;

    default:
        return eMMC_ST_SUCCESS;
    }

    eMMC_debug(eMMC_DEBUG_LEVEL,1,"BlkAddr_start: %Xh, BlkAddr_end: %Xh\n",
        u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);

    return eMMC_EraseBootPart(u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end, 1);

}

char *gpas8_eMMCPartName[]={
    "e2pbak", "nvrambak", "hwcfgs", "recovery", "os",
    "fdd", "tdd", "blogo", "apanic","misc", "cus",
    "e2p0","e2p1","nvram0","nvram1", "system","cache", "data", "internal sd"};

// --------------------------------------------
static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
static U32 sgu32_MemGuard1 = 0x1289;

U32 eMMC_CheckIfMemCorrupt(void)
{
    if(0xA55A != sgu32_MemGuard0 || 0x1289 != sgu32_MemGuard1)
        return eMMC_ST_ERR_MEM_CORRUPT;

    return eMMC_ST_SUCCESS;
}


#if 0
}
#endif


#elif (defined(eMMC_DRV_INFINITY3_UBOOT)&&eMMC_DRV_INFINITY3_UBOOT) || \
      (defined(eMMC_DRV_INFINITY5_UBOOT)&&eMMC_DRV_INFINITY5_UBOOT) || \
      (defined(eMMC_DRV_INFINITY6E_UBOOT)&&eMMC_DRV_INFINITY6E_UBOOT)
// check some fix value, print only when setting wrong

U32 eMMC_hw_timer_delay(U32 u32us) 
{
    #if 0
   volatile int i = 0;

    for (i = 0; i < (u32us>>0); i++)
    {
        #if 1
        volatile int j = 0, tmp;
        for (j = 0; j < 0x2; j++)
        {
            tmp = j;
        }
        #endif
    }
    #else

  extern void udelay(unsigned long usec);
  udelay(u32us);

    #endif
    return u32us;
}

U32 eMMC_hw_timer_sleep(U32 u32ms)
{
    U32 u32_us = u32ms*1000;

    while(u32_us > 1000)
    {
        eMMC_hw_timer_delay(1000); // uboot has no context switch
        u32_us -= 1000;
    }

    eMMC_hw_timer_delay(u32_us);
    return u32ms;
}
U32 eMMC_hw_timer_start(void)
{
    // Reset PIU Timer1
    return 0;
}

U32 eMMC_hw_timer_tick(void)
{

    return 0;
}

void eMMC_DumpPadClk(void)
{
    //---------------------------------------------------------------------
    eMMC_debug(0, 0, "[pad setting]:\r\n");
    switch(g_eMMCDrv.u8_PadType)
    {
        case FCIE_eMMC_BYPASS:          eMMC_debug(0, 0, "Bypass\r\n");  break;
        case FCIE_eMMC_SDR:             eMMC_debug(0, 0, "SDR\r\n");  break;
        default:
            eMMC_debug(0, 0, "eMMC Err: Pad unknown, %d\r\n", g_eMMCDrv.u8_PadType); eMMC_die("\r\n");
            break;
    }
}

// set pad first, then config clock
U32 eMMC_pads_switch(U32 u32Mode)
{
    g_eMMCDrv.u8_PadType = u32Mode;

    REG_FCIE_CLRBIT(reg_all_pad_in, BIT_ALL_PAD_IN);
    REG_FCIE_CLRBIT(reg_sd_config, BIT_SD_MODE_MASK);
    REG_FCIE_CLRBIT(reg_sdio_config, BIT_SDIO_MODE_MASK);
    REG_FCIE_CLRBIT(reg_emmc_config, (BIT_EMMC_MODE_MASK));
#if (defined(eMMC_DRV_INFINITY5_UBOOT)&&eMMC_DRV_INFINITY5_UBOOT)
    REG_FCIE_CLRBIT(reg_nand_config, BIT_NAND_MODE_MASK);
    REG_FCIE_SETBIT(reg_emmc_config, BIT_EMMC_MODE_1);
    #if (EMMC_SEL_INTERFACE == 1)
    REG_FCIE_SETBIT(reg_sdio_config, BIT_SDIO_MODE_1);
    #elif (EMMC_SEL_INTERFACE == 2)
    REG_FCIE_CLRBIT(reg_emmc_config, BIT1);
    REG_FCIE_SETBIT(reg_sd_config, BIT_SD_MODE_1);
    #endif
#elif (defined(eMMC_DRV_INFINITY6E_UBOOT)&&eMMC_DRV_INFINITY6E_UBOOT)
    if( u32Mode == FCIE_MODE_8BITS_MACRO_HIGH_SPEED )
    {
        REG_FCIE_SETBIT(reg_emmc_config, (BIT_EMMC_MODE_8X)); //8B mode
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "8X-MOD\r\n");
    }
    else {
        REG_FCIE_SETBIT(reg_emmc_config, (BIT_EMMC_MODE_1)); //4B mode
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "1X/4X-MOD\r\n");
    }
#endif
    // fcie
    REG_FCIE_CLRBIT(FCIE_DDR_MODE, BIT_MACRO_MODE_MASK);

    eMMC_debug(eMMC_DEBUG_LEVEL_MEDIUM, 1, "SDR(%1d)[0:1:2]/[X1:X4:X8]\r\n", u32Mode);

    REG_FCIE_SETBIT(FCIE_DDR_MODE, BIT_PAD_CLK_SEL|BIT_PAD_IN_SEL_SD|BIT_FALL_LATCH);
    //REG_FCIE_SETBIT(FCIE_DDR_MODE, BIT_PAD_IN_RDY_SEL|BIT_PRE_FULL_SEL0|BIT_PRE_FULL_SEL1);    

    return eMMC_ST_SUCCESS;

}
// Notice!!! you need to set pad before config clock

U32 eMMC_clock_setting(U16 u16_ClkParam)
{
    eMMC_PlatformResetPre();
    //FPGA mode
    
    switch(u16_ClkParam)    {
    case BIT_FCIE_CLK_300K:     g_eMMCDrv.u32_ClkKHz = 300;     break;
    case BIT_CLK_XTAL_12M:      g_eMMCDrv.u32_ClkKHz = 12000;   break;
    case BIT_FCIE_CLK_20M:      g_eMMCDrv.u32_ClkKHz = 20000;   break;
    case BIT_FCIE_CLK_32M:      g_eMMCDrv.u32_ClkKHz = 32000;   break;
    case BIT_FCIE_CLK_36M:      g_eMMCDrv.u32_ClkKHz = 36000;   break;
    case BIT_FCIE_CLK_40M:      g_eMMCDrv.u32_ClkKHz = 40000;   break;
    case BIT_FCIE_CLK_43_2M:    g_eMMCDrv.u32_ClkKHz = 43200;   break;
    case BIT_FCIE_CLK_48M:      g_eMMCDrv.u32_ClkKHz = 48000;   break;
    default:
        eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_INVALID_PARAM);
        return eMMC_ST_ERR_INVALID_PARAM;
    }

    //eMMC_debug(0, 1, "clock %dk\n", g_eMMCDrv.u32_ClkKHz);
    if(u16_ClkParam != BIT_CLK_XTAL_12M)
    {
        REG_FCIE_CLRBIT(reg_ckg_fcie, BIT0|BIT1);
        REG_FCIE_CLRBIT(reg_ckg_fcie, BIT_CLKGEN_FCIE_MASK|BIT_FCIE_CLK_SRC_SEL);
        REG_FCIE_SETBIT(reg_ckg_fcie, u16_ClkParam<<2);
    }
    //else {
    //    REG_FCIE_SETBIT(reg_ckg_fcie, BIT_FCIE_CLK_SRC_SEL);
    //}
    REG_FCIE_SETBIT(reg_ckg_fcie, BIT_FCIE_CLK_SRC_SEL);
    REG_FCIE_SETBIT(reg_sc_gp_ctrl, BIT7); //reg_ckg_sd [B3]SDIO30 [B7]SD30

    g_eMMCDrv.u16_ClkRegVal = u16_ClkParam;

    eMMC_PlatformResetPost();

    return eMMC_ST_SUCCESS;
}


U32 eMMC_clock_gating(void)
{
    eMMC_PlatformResetPre();
    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_CLK_EN);
    eMMC_PlatformResetPost();
    return eMMC_ST_SUCCESS;
}

U32 eMMC_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt, int mode)
{
    flush_cache(u32_DMAAddr, u32_ByteCnt); 
    return (u32_DMAAddr);
}
U32 eMMC_PlatformResetPre(void)
{
    return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformResetPost(void)
{
    return eMMC_ST_SUCCESS;
}

U32 eMMC_PlatformInit(void)
{
    eMMC_pads_switch(EMMC_DEFO_SPEED_MODE);
    eMMC_clock_setting(FCIE_SLOWEST_CLK);

    return eMMC_ST_SUCCESS;
}

//static U32 sgu32_MemGuard0 = 0xA55A;
eMMC_ALIGN0 eMMC_DRIVER g_eMMCDrv eMMC_ALIGN1;
//static U32 sgu32_MemGuard1 = 0x1289;

eMMC_ALIGN0 U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KB] eMMC_ALIGN1; // 512 bytes
eMMC_ALIGN0 U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTE] eMMC_ALIGN1; // 512 bytes

void *eMMC_memcpy (void *destaddr, void const *srcaddr, unsigned int len)
{
    char *dest = destaddr;
    char const *src = srcaddr;

    while (len-- > 0)
        *dest++ = *src++;
    return destaddr;
}

U32 eMMC_FCIE_DetectDDRTiming(void)
{
    return eMMC_ST_SUCCESS;
}

U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP)
{
    return eMMC_ST_SUCCESS;
}

U32 eMMC_BootPartitionHandler_E(U16 u16_PartType)
{
    return eMMC_ST_SUCCESS;
}
#else
  #error "Error! no platform functions."
#endif
#endif
