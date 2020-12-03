/*
* eMMC_hal.c- Sigmastar
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

//========================================================
// HAL pre-processors
//========================================================
#if IF_FCIE_SHARE_IP
    // re-config FCIE3 for NFIE mode
    #define eMMC_RECONFIG()       //eMMC_ReConfig();
#else
    // NULL to save CPU a JMP/RET time
    #define eMMC_RECONFIG()
#endif


#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
    #define eMMC_KEEP_RSP(pu8_OneRspBuf, u8_CmdIdx)         \
        if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_RSPFROMRAM_SAVE)\
            eMMC_KeepRsp(pu8_OneRspBuf, u8_CmdIdx)
#else
    #define eMMC_KEEP_RSP(pu8_OneRspBuf, u8_CmdIdx)  // NULL to save CPU a JMP/RET time
#endif


#define eMMC_FCIE_CLK_DIS()    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN)

#define eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT       20
#define eMMC_CMD_API_ERR_RETRY_CNT            20
#define eMMC_CMD_API_WAIT_FIFOCLK_RETRY_CNT   20

#define eMMC_CMD1_RETRY_CNT    0x8000
#define eMMC_CMD3_RETRY_CNT    0x5

static U32 (*sgpFn_eMMC_FCIE_WaitEvents_Ex)(U32, U16, U32) = NULL;
//========================================================
// HAL APIs
//========================================================
U32 eMMC_FCIE_WaitEvents(U32 u32_RegAddr, U16 u16_Events, U32 u32_MicroSec)
{
    return (*sgpFn_eMMC_FCIE_WaitEvents_Ex)(u32_RegAddr, u16_Events, u32_MicroSec);
}

U32 eMMC_FCIE_PollingEvents(U32 u32_RegAddr, U16 u16_Events, U32 u32_MicroSec)
{
    volatile U32 u32_i, u32_DelayX;
    volatile U16 u16_val;

    REG_FCIE_W(FCIE_MIE_INT_EN, 0); // mask interrupts
    if(u32_MicroSec > HW_TIMER_DELAY_100us)
    {
        u32_DelayX = HW_TIMER_DELAY_100us/HW_TIMER_DELAY_1us;
        u32_MicroSec /= u32_DelayX;
    }
    else
        u32_DelayX = 1;

    for(u32_i=0; u32_i<u32_MicroSec; u32_i++)
    {
        eMMC_hw_timer_delay(HW_TIMER_DELAY_1us*u32_DelayX);
        REG_FCIE_R(u32_RegAddr, u16_val);
        if(u16_Events == (u16_val & u16_Events))
            break;
    }

    if(u32_i == u32_MicroSec)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: %u us, Reg.%04Xh: %04Xh, but wanted: %04Xh\n",
            u32_MicroSec*u32_DelayX,
            (u32_RegAddr-FCIE_MIE_EVENT)>>REG_OFFSET_SHIFT_BITS,
            u16_val, u16_Events);

        return eMMC_ST_ERR_TIMEOUT_WAIT_REG0;
    }

    return eMMC_ST_SUCCESS;
}


static U16 sgau16_eMMCDebugBus[100];
void eMMC_FCIE_DumpDebugBus(void)
{
    volatile U16 u16_reg, u16_i;
    U16 u16_idx = 0;

    memset(sgau16_eMMCDebugBus, 0xFF, sizeof(sgau16_eMMCDebugBus));
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\n\n");
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n");

    for(u16_i=0; u16_i<8; u16_i++)
    {
        REG_FCIE_CLRBIT(FCIE_TEST_MODE, BIT_FCIE_DEBUG_MODE_MASK);
        REG_FCIE_SETBIT(FCIE_TEST_MODE, u16_i<<BIT_FCIE_DEBUG_MODE_SHIFT);

        REG_FCIE_R(FCIE_TEST_MODE, u16_reg);
        sgau16_eMMCDebugBus[u16_idx] = u16_reg;
        u16_idx++;
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "FCIE_TEST_MODE: %Xh\n", u16_reg);

        REG_FCIE_R(FCIE_DEBUG_BUS, u16_reg);
        sgau16_eMMCDebugBus[u16_idx] = u16_reg;
        u16_idx++;
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "FCIE_DEBUG_BUS: %Xh\n", u16_reg);
        REG_FCIE_R(FCIE_MACRO_REDNT, u16_reg);
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "FCIE_REG_0x32: %Xh\n", u16_reg);

    }

    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\n");
    for(u16_i=0; u16_i<8; u16_i++)
    {
        REG_FCIE_CLRBIT(NC_WIDTH, BIT_NC_DEB_SEL_MASK);
        REG_FCIE_SETBIT(NC_WIDTH, u16_i<<BIT_NC_DEB_SEL_SHIFT);

        REG_FCIE_R(FCIE_TEST_MODE, u16_reg);
        sgau16_eMMCDebugBus[u16_idx] = u16_reg;
        u16_idx++;
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "FCIE_TEST_MODE: %Xh\n", u16_reg);

        REG_FCIE_R(NC_WIDTH, u16_reg);
        sgau16_eMMCDebugBus[u16_idx] = u16_reg;
        u16_idx++;
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "NC_WIDTH: %Xh\n", u16_reg);

        REG_FCIE_R(FCIE_DEBUG_BUS, u16_reg);
        sgau16_eMMCDebugBus[u16_idx] = u16_reg;
        u16_idx++;
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "FCIE_DEBUG_BUS: %Xh\n", u16_reg);
        REG_FCIE_R(FCIE_MACRO_REDNT, u16_reg);
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "FCIE_REG_0x32: %Xh\n", u16_reg);
    }

}


static U16 sgau16_eMMCDebugReg[100];
void eMMC_FCIE_DumpRegisters(void)
{
    volatile U16 u16_reg;
    U16 u16_i;

    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n\nFCIE Reg:");

    for(u16_i=0 ; u16_i<0x60; u16_i++)
    {
        if(0 == u16_i%8)
            eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n%02Xh:| ", u16_i);

        REG_FCIE_R(GET_REG_ADDR(FCIE_REG_BASE_ADDR, u16_i), u16_reg);
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", u16_reg);
    }

    REG_FCIE_SETBIT(GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x36), BIT0|BIT1);
    REG_FCIE_R(GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x36), u16_reg);

    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\nFCIE Clock status 0x36 = 0x%04X \n", u16_reg);
    eMMC_hw_timer_delay(HW_TIMER_DELAY_1ms);

    REG_FCIE_R(GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x36), u16_reg);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "FCIE Clock status delay 0x36 = 0x%04X \n", u16_reg);

    #if defined(CONFIG_ENABLE_EMMC_ACP) && CONFIG_ENABLE_EMMC_ACP
    REG_FCIE_R(reg_acp_idle, u16_reg);
    if((u16_reg & BIT_ACP_IDLE_STS) == BIT_ACP_IDLE_STS)
        eMMC_debug(eMMC_DEBUG_LEVEL, 0 ,"\n ACP is idle 0x%X\n", u16_reg);
    else
        eMMC_debug(eMMC_DEBUG_LEVEL, 0 ,"\n ACP is busy 0x%X\n", u16_reg);
    #endif

    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\nFCIE JobCnt: \n");
    REG_FCIE_SETBIT(FCIE_JOB_BL_CNT, BIT_SD_JOB_CNT_SRC_SEL|BIT_SD_JOB_CNT_SRC_MIU);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "MIU side JobCnt: %Xh \n", REG_FCIE(FCIE_TR_BK_CNT));
    REG_FCIE_CLRBIT(FCIE_JOB_BL_CNT, BIT_SD_JOB_CNT_SRC_MIU);
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "Card side JobCnt: %Xh \n", REG_FCIE(FCIE_TR_BK_CNT));
    REG_FCIE_CLRBIT(FCIE_JOB_BL_CNT, BIT_SD_JOB_CNT_SRC_SEL);

    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\nFCIE CIFC:");
    eMMC_FCIE_GetCIFC(0, 0x20, (U16*)sgau16_eMMCDebugReg);
    for(u16_i=0 ; u16_i<0x20; u16_i++)
    {
        if(0 == u16_i%8)
            eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n%02Xh:| ", u16_i);

        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", sgau16_eMMCDebugReg[u16_i]);
    }
}

void eMMC_FCIE_ErrHandler_Stop(void)
{
    if(0==eMMC_IF_TUNING_TTABLE())
    {
        eMMC_DumpDriverStatus();
        eMMC_DumpPadClk();
        eMMC_FCIE_DumpRegisters();
        eMMC_FCIE_DumpDebugBus();
        eMMC_die("\n");
    }
    else {
        eMMC_FCIE_Init();
    }
}

static U8 sgu8_IfNeedRestorePadType=0xFF;
U32 eMMC_FCIE_ErrHandler_Retry(void)
{
    #if !(defined(ENABLE_eMMC_ATOP) && ENABLE_eMMC_ATOP)
    static U8 u8_IfToggleDataSync=0;
    #endif
    U32 u32_err;

    eMMC_FCIE_Init();

    #if !(defined(ENABLE_eMMC_ATOP) && ENABLE_eMMC_ATOP)
    if((++u8_IfToggleDataSync) & 1)
    {
        g_eMMCDrv.u16_Reg10_Mode ^= BIT_SD_DATA_SYNC;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 0, "eMMC: retry DATA_SYNC: %Xh\n",
            g_eMMCDrv.u16_Reg10_Mode&BIT_SD_DATA_SYNC);

        return eMMC_ST_SUCCESS;
    }
    #endif

    // -------------------------------------------------
    // if error @ init, do not retry.
    if(0 ==(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_INIT_DONE) )
        return eMMC_ST_SUCCESS;

    // -------------------------------------------------
    if(eMMC_IF_NORMAL_SDR())
    {
        if(gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT-1] == g_eMMCDrv.u16_ClkRegVal)
            return eMMC_ST_ERR_NO_SLOWER_CLK;
    }

    if(eMMC_IF_NORMAL_SDR())
    {
        U8 u8_i;

        for(u8_i=0; u8_i<eMMC_FCIE_VALID_CLK_CNT; u8_i++)
            if(gau8_FCIEClkSel[u8_i] == g_eMMCDrv.u16_ClkRegVal)
                break;

        if (u8_i < (eMMC_FCIE_VALID_CLK_CNT-1))
        {
            eMMC_clock_setting(gau8_FCIEClkSel[u8_i+1]);
        }
        else
        {
            eMMC_clock_setting(gau8_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT-1]);
        }

    }
    else
    {
        sgu8_IfNeedRestorePadType = g_eMMCDrv.u8_PadType;
        u32_err = eMMC_FCIE_EnableSDRMode();
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableSDRModefail: %Xh\n", u32_err);
        }
    }

    eMMC_debug(eMMC_DEBUG_LEVEL_WARNING,0,
        "eMMC Warn: slow clk to %u.%uMHz, %Xh\n",
        g_eMMCDrv.u32_ClkKHz/1000, (g_eMMCDrv.u32_ClkKHz%1000)/100, g_eMMCDrv.u16_ClkRegVal);

    return eMMC_ST_SUCCESS;
}


void eMMC_FCIE_ErrHandler_RestoreClk(void)
{
    if(0xFF != sgu8_IfNeedRestorePadType)
    {
        U32 u32_err;
        u32_err = eMMC_FCIE_EnableFastMode_Ex(sgu8_IfNeedRestorePadType);
        if(u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableFastMode_Ex fail, %Xh\n", u32_err);
            return;
        }
        sgu8_IfNeedRestorePadType = 0xFF;
        return;
    }

    if(eMMC_IF_NORMAL_SDR() && FCIE_DEFAULT_CLK!=g_eMMCDrv.u16_ClkRegVal)
    {
        eMMC_clock_setting(gau8_FCIEClkSel[0]);
        return;
    }

}


void eMMC_FCIE_ErrHandler_ReInit(void)
{
    U32 u32_err;
    U32 u32_DrvFlag = g_eMMCDrv.u32_DrvFlag;
    U16 u16_Reg10 = g_eMMCDrv.u16_Reg10_Mode;
    U8  u8_OriPadType = g_eMMCDrv.u8_PadType;
    U16 u16_OldClkRegVal = g_eMMCDrv.u16_ClkRegVal;

    u32_err = eMMC_FCIE_Init();
    if(u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: FCIE_Init fail, %Xh\n", u32_err);
        goto LABEL_REINIT_END;
    }

    g_eMMCDrv.u32_DrvFlag = 0;
    eMMC_PlatformInit();
    u32_err = eMMC_Identify();
    if(u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: Identify fail, %Xh\n", u32_err);
        goto LABEL_REINIT_END;
    }

    eMMC_clock_setting(FCIE_SLOW_CLK);

    u32_err = eMMC_CMD3_CMD7(g_eMMCDrv.u16_RCA, 7);
    if(u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CMD7 fail, %Xh\n", u32_err);
        goto LABEL_REINIT_END;
    }

    u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);
    if(eMMC_ST_SUCCESS != u32_err)
        goto  LABEL_REINIT_END;

    eMMC_clock_setting(FCIE_DEFAULT_CLK);

    // ---------------------------------
    g_eMMCDrv.u32_DrvFlag = u32_DrvFlag;
    if(eMMC_IF_NORMAL_SDR())
    {
        u32_err = eMMC_FCIE_EnableSDRMode();
        if(u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableSDRMode fail, %Xh\n", u32_err);
            goto LABEL_REINIT_END;
        }
        eMMC_clock_setting(u16_OldClkRegVal);
    }
    else
    {
        u32_err = eMMC_FCIE_EnableFastMode_Ex(u8_OriPadType);
        if(u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableFastMode_Ex fail, %Xh\n", u32_err);
            goto LABEL_REINIT_END;
        }
    }

    g_eMMCDrv.u16_Reg10_Mode = u16_Reg10;
    REG_FCIE_W(FCIE_SD_MODE, g_eMMCDrv.u16_Reg10_Mode);

    LABEL_REINIT_END:
    if(u32_err)
        eMMC_die("\n");
}


#define FCIE_WAIT_RST_DONE_D1  0x5555
#define FCIE_WAIT_RST_DONE_D2  0xAAAA
#define FCIE_WAIT_RST_DONE_CNT 3

void eMMC_FCIE_CheckResetDone(void)
{
    volatile U16 au16_tmp[FCIE_WAIT_RST_DONE_CNT];
    volatile U32 u32_i, u32_j, u32_err;

    for(u32_i=0; u32_i<TIME_WAIT_FCIE_RESET; u32_i++)
    {
        for(u32_j=0; u32_j<FCIE_WAIT_RST_DONE_CNT; u32_j++)
            REG_FCIE_W(FCIE_CIFC_ADDR(u32_j), FCIE_WAIT_RST_DONE_D1+u32_j);

        for(u32_j=0; u32_j<FCIE_WAIT_RST_DONE_CNT; u32_j++)
            REG_FCIE_R(FCIE_CIFC_ADDR(u32_j), au16_tmp[u32_j]);

        for(u32_j=0; u32_j<FCIE_WAIT_RST_DONE_CNT; u32_j++)
            if(FCIE_WAIT_RST_DONE_D1+u32_j != au16_tmp[u32_j])
                break;

        if(FCIE_WAIT_RST_DONE_CNT == u32_j)
            break;
        eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
    }

    if(TIME_WAIT_FCIE_RESET == u32_i)
    {
        u32_err = eMMC_ST_ERR_FCIE_NO_RIU;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: check CIFC fail: %Xh \n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
    }
}

U32 eMMC_FCIE_Reset(void)
{
    U16 au16_tmp[FCIE_WAIT_RST_DONE_CNT];
    U32 u32_i, u32_j, u32_err = eMMC_ST_SUCCESS;
    U16 u16_clk = g_eMMCDrv.u16_ClkRegVal;

    eMMC_clock_setting(gau8_FCIEClkSel[0]); // speed up FCIE reset done

    REG_FCIE_W(FCIE_MIE_INT_EN, 0);
    REG_FCIE_W(FCIE_MMA_PRI_REG, BIT_MIU_CLK_FREE_RUN);
    REG_FCIE_W(FCIE_PATH_CTRL, BIT_SD_EN);
    REG_FCIE_W(FCIE_SD_CTRL, 0);
    REG_FCIE_W(FCIE_SD_MODE, 0);

    // miu request reset - set
    REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_MIU_REQUEST_RST);

    // FCIE reset - prepare
    for(u32_i=0; u32_i<TIME_WAIT_FCIE_RESET; u32_i++)
    {
        for(u32_j=0; u32_j<FCIE_WAIT_RST_DONE_CNT; u32_j++)
            REG_FCIE_W(FCIE_CIFC_ADDR(u32_j), FCIE_WAIT_RST_DONE_D1+u32_j);

        for(u32_j=0; u32_j<FCIE_WAIT_RST_DONE_CNT; u32_j++)
            REG_FCIE_R(FCIE_CIFC_ADDR(u32_j), au16_tmp[u32_j]);

        for(u32_j=0; u32_j<FCIE_WAIT_RST_DONE_CNT; u32_j++)
            if(FCIE_WAIT_RST_DONE_D1+u32_j != au16_tmp[u32_j])
                break;

        if(FCIE_WAIT_RST_DONE_CNT == u32_j)
            break;
        eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
    }
    if(TIME_WAIT_FCIE_RESET == u32_i)
    {
        u32_err = eMMC_ST_ERR_FCIE_NO_RIU;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: preset CIFC fail: %Xh \n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
    }

    // FCIE reset - set
    REG_FCIE_CLRBIT(FCIE_TEST_MODE, BIT_FCIE_SOFT_RST_n); /* active low */
    // FCIE reset - wait
    for(u32_i=0; u32_i<TIME_WAIT_FCIE_RESET; u32_i++)
    {
        REG_FCIE_R(FCIE_CIFC_ADDR(0), au16_tmp[0]);
        REG_FCIE_R(FCIE_CIFC_ADDR(1), au16_tmp[1]);
        REG_FCIE_R(FCIE_CIFC_ADDR(2), au16_tmp[2]);
        if(0==au16_tmp[0] && 0==au16_tmp[1] && 0==au16_tmp[2])
            break;
        eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
    }
    if(TIME_WAIT_FCIE_RESET == u32_i){
        u32_err = eMMC_ST_ERR_FCIE_NO_CLK;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: reset CIFC fail: %Xh \n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
    }

    // FCIE reset - clear
    REG_FCIE_SETBIT(FCIE_TEST_MODE, BIT_FCIE_SOFT_RST_n);
    // FCIE reset - check
    eMMC_FCIE_CheckResetDone();
    // miu request reset - clear
    REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_MIU_REQUEST_RST);

    eMMC_clock_setting(u16_clk);
    return u32_err;
}


U32 eMMC_FCIE_Init(void)
{
    U32 u32_err;

    // ------------------------------------------
    // setup function pointer to wait for events
    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    if(NULL==sgpFn_eMMC_FCIE_WaitEvents_Ex)
        sgpFn_eMMC_FCIE_WaitEvents_Ex = eMMC_WaitCompleteIntr;
    #else
    if(NULL==sgpFn_eMMC_FCIE_WaitEvents_Ex)
        sgpFn_eMMC_FCIE_WaitEvents_Ex = eMMC_FCIE_PollingEvents;
    #endif

    // ------------------------------------------
    eMMC_PlatformResetPre();

    // ------------------------------------------
    #if eMMC_TEST_IN_DESIGN
    {
        volatile U16 u16_i, u16_reg;
        // check timer clock
        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "Timer test, for 6 sec: ");
        for(u16_i = 6; u16_i > 0; u16_i--)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL, 1, "%u ", u16_i);
            eMMC_hw_timer_delay(HW_TIMER_DELAY_1s);
        }
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\n");

        // check FCIE reg.30h
        REG_FCIE_R(FCIE_TEST_MODE, u16_reg);
        if(0)//u16_reg & BIT_FCIE_BIST_FAIL) /* Andersen: "don't care." */
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Reg0x30h BIST fail: %04Xh \r\n", u16_reg);
            return eMMC_ST_ERR_BIST_FAIL;
        }
        if(u16_reg & BIT_FCIE_DEBUG_MODE_MASK)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: Reg0x30h Debug Mode: %04Xh \r\n", u16_reg);
            return eMMC_ST_ERR_DEBUG_MODE;
        }

        u32_err = eMMC_FCIE_Reset();
        if(eMMC_ST_SUCCESS != u32_err){
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: reset fail\n");
            eMMC_FCIE_ErrHandler_Stop();
            return u32_err;
        }
    }
    #endif // eMMC_TEST_IN_DESIGN

    // ------------------------------------------
    u32_err = eMMC_FCIE_Reset();
    if(eMMC_ST_SUCCESS != u32_err){
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: reset fail: %Xh\n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        return u32_err;
    }

    REG_FCIE_W(FCIE_MIE_INT_EN, 0);
    REG_FCIE_W(FCIE_MMA_PRI_REG, BIT_MIU_CLK_FREE_RUN);
    REG_FCIE_W(FCIE_PATH_CTRL, BIT_SD_EN);
    // all cmd are 5 bytes (excluding CRC)
    REG_FCIE_W(FCIE_CMD_SIZE, eMMC_CMD_BYTE_CNT);
    REG_FCIE_W(FCIE_SD_CTRL, 0);
    REG_FCIE_W(FCIE_SD_MODE, g_eMMCDrv.u16_Reg10_Mode);
    #if FICE_BYTE_MODE_ENABLE
    // default sector size: 0x200
    REG_FCIE_W(FCIE_SDIO_CTRL, BIT_SDIO_BLK_MODE | eMMC_SECTOR_512BYTE);
    #else
    REG_FCIE_W(FCIE_SDIO_CTRL, 0);
    #endif

    REG_FCIE_W(FCIE_PWR_SAVE_MODE, 0);
    eMMC_FCIE_ClearEvents();
    eMMC_PlatformResetPost();

    return eMMC_ST_SUCCESS; // ok
}


void eMMC_FCIE_ClearEvents(void)
{
    volatile U16 u16_reg;
    REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_MIU_REQUEST_RST);
    while(1){
        REG_FCIE_W(FCIE_MIE_EVENT, BIT_ALL_CARD_INT_EVENTS);
        REG_FCIE_R(FCIE_MIE_EVENT, u16_reg);
        if(0==(u16_reg&BIT_ALL_CARD_INT_EVENTS))
            break;
        REG_FCIE_W(FCIE_MIE_EVENT, 0);
        REG_FCIE_W(FCIE_MIE_EVENT, 0);
    }
    REG_FCIE_W1C(FCIE_SD_STATUS, BIT_SD_FCIE_ERR_FLAGS); // W1C
    REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_MIU_REQUEST_RST);
}


void eMMC_FCIE_ClearEvents_Reg0(void)
{
    volatile U16 u16_reg;

    while(1){
        REG_FCIE_W(FCIE_MIE_EVENT, BIT_ALL_CARD_INT_EVENTS);
        REG_FCIE_R(FCIE_MIE_EVENT, u16_reg);
        if(0==(u16_reg&BIT_ALL_CARD_INT_EVENTS))
            break;
        REG_FCIE_W(FCIE_MIE_EVENT, 0);
        REG_FCIE_W(FCIE_MIE_EVENT, 0);
    }
}


U32 eMMC_FCIE_WaitD0High_Ex(U32 u32_us)
{
   volatile U32 u32_cnt;
   volatile U16 u16_read0=0, u16_read1=0;

   for(u32_cnt=0; u32_cnt < u32_us; u32_cnt++)
   {
       REG_FCIE_R(FCIE_SD_STATUS, u16_read0);
       eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
       REG_FCIE_R(FCIE_SD_STATUS, u16_read1);

       if((u16_read0&BIT_SD_CARD_D0_ST) && (u16_read1&BIT_SD_CARD_D0_ST))
           break;

       // interrupt mode, then sleep
       #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
       if(eMMC_WaitCompleteIntr == sgpFn_eMMC_FCIE_WaitEvents_Ex)
       if(u32_cnt > 500 && u32_us-u32_cnt > 1000)
       {
           eMMC_hw_timer_sleep(1);
           u32_cnt += 1000-2;
       }
       #endif
   }

   return u32_cnt;
}

U32 eMMC_FCIE_WaitD0High(U32 u32_us)
{
    volatile U32 u32_cnt;

    REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);
    u32_cnt = eMMC_FCIE_WaitD0High_Ex(u32_us);

    if(u32_us == u32_cnt)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: wait D0 H timeout %u us\n", u32_cnt);
        return eMMC_ST_ERR_TIMEOUT_WAITD0HIGH;
    }

    return eMMC_ST_SUCCESS;
}


#define FIFO_CLK_RDY_CHECK_CNT    3
U32 eMMC_FCIE_FifoClkRdy(U8 u8_Dir)
{
    volatile U32 u32_cnt=0;
    volatile U16 au16_read[FIFO_CLK_RDY_CHECK_CNT], u16_i;

    for(u32_cnt=0; u32_cnt < TIME_WAIT_FIFOCLK_RDY; u32_cnt++)
    {
        REG_FCIE_R(FCIE_MMA_PRI_REG, au16_read[0]);
        if(u8_Dir == (au16_read[0]&BIT_DMA_DIR_W))
            break;
    }
    if(TIME_WAIT_FIFOCLK_RDY == u32_cnt)
        return eMMC_ST_ERR_TIMEOUT_FIFOCLKRDY;

    for(u32_cnt=0; u32_cnt < TIME_WAIT_FIFOCLK_RDY; u32_cnt++)
    {
        for(u16_i=0; u16_i<FIFO_CLK_RDY_CHECK_CNT; u16_i++)
        {
            REG_FCIE_R(FCIE_MMA_PRI_REG, au16_read[u16_i]);
        }
        for(u16_i=0; u16_i<FIFO_CLK_RDY_CHECK_CNT; u16_i++)
            if(0 == (au16_read[u16_i] & BIT_FIFO_CLKRDY))
                break;
        if(FIFO_CLK_RDY_CHECK_CNT == u16_i)
            return eMMC_ST_SUCCESS;

        eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
    }

    if(TIME_WAIT_FIFOCLK_RDY == u32_cnt)
        return eMMC_ST_ERR_TIMEOUT_FIFOCLKRDY;

    return eMMC_ST_SUCCESS;
}


U32 eMMC_FCIE_SendCmd
(
    U16 u16_Mode, U16 u16_Ctrl, U32 u32_Arg, U8 u8_CmdIdx, U8 u8_RspByteCnt
)
{
    U32 u32_err, u32_Timeout = TIME_WAIT_DAT0_HIGH;
    U16 au16_tmp[3];

    if(38 == u8_CmdIdx)
        u32_Timeout = TIME_WAIT_ERASE_DAT0_HIGH;

    #if 0
    eMMC_debug(0,1,"\n");
    eMMC_debug(0,1,"cmd:%u, arg:%Xh, rspb:%Xh, mode:%Xh, ctrl:%Xh \n",
        u8_CmdIdx, u32_Arg, u8_RspByteCnt, u16_Mode, u16_Ctrl);
    #endif

    REG_FCIE_W(FCIE_RSP_SIZE, u8_RspByteCnt & BIT_SD_RSP_SIZE_MASK);
    REG_FCIE_W(FCIE_SD_MODE, u16_Mode);
    // set cmd
    //   CIFC(0) = 39:32 | 47:40
    //   CIFC(1) = 23:16 | 31:24
    //   CIFC(2) = (CIFC(2) & 0xFF00) | 15:8, ignore (CRC7 | end_bit).
    while(1)
    {
        REG_FCIE_W(FCIE_CIFC_ADDR(0),((u32_Arg>>24)<<8) | (0x40|u8_CmdIdx));
        REG_FCIE_W(FCIE_CIFC_ADDR(1), (u32_Arg&0xFF00) | ((u32_Arg>>16)&0xFF));
        REG_FCIE_W(FCIE_CIFC_ADDR(2),  u32_Arg&0xFF);

        REG_FCIE_R(FCIE_CIFC_ADDR(0),au16_tmp[0]);
        REG_FCIE_R(FCIE_CIFC_ADDR(1),au16_tmp[1]);
        REG_FCIE_R(FCIE_CIFC_ADDR(2),au16_tmp[2]);

        if(au16_tmp[0] == (((u32_Arg>>24)<<8) | (0x40|u8_CmdIdx))&&
            au16_tmp[1] == ((u32_Arg&0xFF00) | ((u32_Arg>>16)&0xFF))&&
            au16_tmp[2] == (u32_Arg&0xFF))
            break;
    }

    if(12 != u8_CmdIdx)
    {
        u32_err = eMMC_FCIE_WaitD0High(u32_Timeout);
        if(eMMC_ST_SUCCESS != u32_err)
            goto LABEL_SEND_CMD_ERROR;
    }
    REG_FCIE_W(FCIE_SD_CTRL, u16_Ctrl);

    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    REG_FCIE_W(FCIE_MIE_INT_EN, BIT_SD_CMD_END);
    #endif
    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_RSP_WAIT_D0H){
        u32_err = eMMC_FCIE_WaitD0High(u32_Timeout);
        if(eMMC_ST_SUCCESS != u32_err)
            goto LABEL_SEND_CMD_ERROR;
    }

    // wait event
    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_SD_CMD_END, TIME_WAIT_CMDRSP_END);

    LABEL_SEND_CMD_ERROR:
    return u32_err;

}

void eMMC_FCIE_GetCIFC(U16 u16_WordPos, U16 u16_WordCnt, U16 *pu16_Buf)
{
    U16 u16_i;

    for(u16_i=0; u16_i<u16_WordCnt; u16_i++)
        REG_FCIE_R(FCIE_CIFC_ADDR(u16_i), pu16_Buf[u16_i]);
}

void eMMC_FCIE_GetCIFD(U16 u16_WordPos, U16 u16_WordCnt, U16 *pu16_Buf)
{
    U16 u16_i;

    for(u16_i=0; u16_i<u16_WordCnt; u16_i++)
        REG_FCIE_R(FCIE_CIFD_ADDR(u16_i), pu16_Buf[u16_i]);
}

U8 eMMC_FCIE_CmdRspBufGet(U8 u8addr)
{
    U16 u16Tmp;

    u16Tmp = REG_FCIE(FCIE_CIFC_ADDR(u8addr>>1));

    if(u8addr&0x1)
    {
        return ((u16Tmp>>8)&0xFF);
    }
    else
    {
        return (u16Tmp&0xFF);
    }
}

U8 eMMC_FCIE_DataFifoGet(U8 u8addr)
{
    U16 u16Tmp;

    u16Tmp = REG_FCIE(FCIE_CIFD_ADDR(u8addr>>1));

    if(u8addr&0x1)
        return ((u16Tmp>>8)&0xFF);
    else
        return (u16Tmp&0xFF);
}

//===================================================
#if defined(eMMC_RSP_FROM_RAM) && eMMC_RSP_FROM_RAM
void eMMC_KeepRsp(U8 *pu8_OneRspBuf, U8 u8_CmdIdx)
{
    U16 u16_idx;
    U8  u8_ByteCnt;

    u16_idx = u8_CmdIdx * eMMC_CMD_BYTE_CNT;
    u8_ByteCnt = eMMC_CMD_BYTE_CNT;

    if(u8_CmdIdx > 10)
        u16_idx += (eMMC_R2_BYTE_CNT-eMMC_CMD_BYTE_CNT)*3;
    else if(u8_CmdIdx > 9)
        u16_idx += (eMMC_R2_BYTE_CNT-eMMC_CMD_BYTE_CNT)*2;
    else if(u8_CmdIdx > 2)
        u16_idx += (eMMC_R2_BYTE_CNT-eMMC_CMD_BYTE_CNT)*1;

    if(10==u8_CmdIdx || 9==u8_CmdIdx || 2==u8_CmdIdx)
        u8_ByteCnt = eMMC_R2_BYTE_CNT;

    if(u16_idx+u8_ByteCnt > eMMC_SECTOR_512BYTE-4) // last 4 bytes are CRC
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: KeepRsp over 512B, %Xh, %Xh, %Xh\n",
            u8_CmdIdx, u16_idx, u8_ByteCnt);
        eMMC_die("\n"); // should be system fatal error, not eMMC driver
    }

    memcpy(g_eMMCDrv.au8_AllRsp + u16_idx, pu8_OneRspBuf, u8_ByteCnt);

}


U32 eMMC_ReturnRsp(U8 *pu8_OneRspBuf, U8 u8_CmdIdx)
{
    U16 u16_idx;
    U8  u8_ByteCnt;

    u16_idx = u8_CmdIdx * eMMC_CMD_BYTE_CNT;
    u8_ByteCnt = eMMC_CMD_BYTE_CNT;

    if(u8_CmdIdx > 10)
        u16_idx += (eMMC_R2_BYTE_CNT-eMMC_CMD_BYTE_CNT)*3;
    else if(u8_CmdIdx > 9)
        u16_idx += (eMMC_R2_BYTE_CNT-eMMC_CMD_BYTE_CNT)*2;
    else if(u8_CmdIdx > 2)
        u16_idx += (eMMC_R2_BYTE_CNT-eMMC_CMD_BYTE_CNT)*1;

    if(10==u8_CmdIdx || 9==u8_CmdIdx || 2==u8_CmdIdx)
        u8_ByteCnt = eMMC_R2_BYTE_CNT;

    if(u16_idx+u8_ByteCnt > eMMC_SECTOR_512BYTE-4) // last 4 bytes are CRC
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: KeepRsp over 512B, %Xh, %Xh, %Xh\n",
            u8_CmdIdx, u16_idx, u8_ByteCnt);
        return eMMC_ST_ERR_NO_RSP_IN_RAM;
    }

    if(0 == g_eMMCDrv.au8_AllRsp[u16_idx])
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_LOW,1,"eMMC Info: no rsp, %u %u \n", u8_CmdIdx, u16_idx);
        return eMMC_ST_ERR_NO_RSP_IN_RAM;
    }

    memcpy(pu8_OneRspBuf, g_eMMCDrv.au8_AllRsp + u16_idx, u8_ByteCnt);

    return eMMC_ST_SUCCESS;
}


// -------------------------------
U32 eMMC_SaveRsp(void)
{

    return eMMC_ST_SUCCESS;
}

U32 eMMC_SaveDriverContext(void)
{

    return eMMC_ST_SUCCESS;
}


U32 eMMC_LoadRsp(U8 *pu8_AllRspBuf)
{

    return eMMC_ST_SUCCESS;
}

U32 eMMC_LoadDriverContext(U8 *pu8_Buf)
{

    return eMMC_ST_SUCCESS;
}

#endif
//========================================================
// Send CMD HAL APIs
//========================================================
U32 eMMC_Identify(void)
{
    U32 u32_err = eMMC_ST_SUCCESS;
    U16 u16_i, u16_retry=0;

    g_eMMCDrv.u16_RCA=1;
    g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_1;
    g_eMMCDrv.u16_Reg10_Mode &= ~BIT_SD_DATA_WIDTH_MASK;

    LABEL_IDENTIFY_CMD0:
    if(eMMC_ST_SUCCESS != eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: WaitD0High TO\n");
        eMMC_FCIE_ErrHandler_Stop();
    }
    eMMC_RST_L();  eMMC_hw_timer_sleep(1);
    eMMC_RST_H();  eMMC_hw_timer_sleep(1);

    if(u16_retry > 10)
    {
        eMMC_FCIE_ErrHandler_Stop();
        return u32_err;
    }
    if(u16_retry)
        eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC Warn: retry: %u\n", u16_retry);

    // CMD0
    u32_err = eMMC_CMD0(0); // reset to idle state
    if(eMMC_ST_SUCCESS != u32_err)
    {u16_retry++;  goto LABEL_IDENTIFY_CMD0;}

    // CMD1
    for(u16_i=0; u16_i<eMMC_CMD1_RETRY_CNT; u16_i++)
    {
        //eMMC_debug(eMMC_DEBUG_LEVEL, 1, "CMD1 try: %02Xh \n", u16_i);
        u32_err = eMMC_CMD1();
        if(eMMC_ST_SUCCESS == u32_err)
            break;

        eMMC_hw_timer_sleep(2);

        if(eMMC_ST_ERR_CMD1_DEV_NOT_RDY != u32_err)
        {u16_retry++;  goto LABEL_IDENTIFY_CMD0;}
    }
    //eMMC_debug(eMMC_DEBUG_LEVEL, 1, "CMD1 try: %02Xh \n", u16_i);
    if(eMMC_ST_SUCCESS != u32_err)
    {u16_retry++;  goto LABEL_IDENTIFY_CMD0;}

    // CMD2
    u32_err = eMMC_CMD2();
    if(eMMC_ST_SUCCESS != u32_err)
    {u16_retry++;  goto LABEL_IDENTIFY_CMD0;}

    // CMD3
    u32_err = eMMC_CMD3_CMD7(g_eMMCDrv.u16_RCA, 3);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        u16_retry++;
        g_eMMCDrv.u16_RCA++;
        goto LABEL_IDENTIFY_CMD0;
    }

    return eMMC_ST_SUCCESS;
}


// reest, etc.
U32 eMMC_CMD0(U32 u32_Arg)
{
    U32 u32_err;
    U16 u16_ctrl;
    U8  u8_retry_cmd=0;

    u16_ctrl = BIT_SD_CMD_EN;

    LABEL_SEND_CMD:
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_Arg, 0, 0);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT)
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD0 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }

        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD0 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
    }

    //eMMC_FCIE_CLK_DIS();
    return u32_err;
}


// send OCR
U32 eMMC_CMD1(void)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;
    #if 0
    U8  u8_retry_fcie=0, u8_retry_cmd=0;
    #endif

    // (sector mode | byte mode) | (3.0|3.1|3.2|3.3|3.4 V)
    u32_arg = BIT30 | (BIT23|BIT22|BIT21|BIT20|BIT19|BIT18|BIT17|BIT16|BIT15|BIT7);
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    #if 0
    LABEL_SEND_CMD:
    #endif
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 1, eMMC_R3_BYTE_CNT);

    #if 1
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Warn: CMD1 send CMD fail: %08Xh \n", u32_err);
        return u32_err;
    }

    // check status
    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    // R3 has no CRC, so does not check BIT_SD_RSP_CRC_ERR
    if(u16_reg & BIT_SD_RSP_TIMEOUT)
    {
        u32_err = eMMC_ST_ERR_CMD1;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Warn: CMD1 no Rsp, Reg.12: %04Xh \n", u16_reg);

        return u32_err;
    }
    else
    {   // CMD1 ok, do things here
        eMMC_FCIE_GetCIFC(0, 3, (U16*)g_eMMCDrv.au8_Rsp);
        //eMMC_dump_mem(g_eMMCDrv.au8_Rsp, eMMC_R3_BYTE_CNT);

        if(0 == (g_eMMCDrv.au8_Rsp[1] & 0x80))
            u32_err =  eMMC_ST_ERR_CMD1_DEV_NOT_RDY;
        else
        {
            g_eMMCDrv.u8_IfSectorMode = (g_eMMCDrv.au8_Rsp[1]&BIT6)>>6;
            eMMC_KEEP_RSP(g_eMMCDrv.au8_Rsp, 1);
        }
    }
    #else
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT)
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD1 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }

        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD1 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
    }
    else
    {   // check status
        REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
        // R3 has no CRC, so does not check BIT_SD_RSP_CRC_ERR
        if(u16_reg & BIT_SD_RSP_TIMEOUT)
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT)
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD1 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD1;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD1 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {   // CMD1 ok, do things here
            eMMC_FCIE_GetCIFC(0, 3, (U16*)g_eMMCDrv.au8_Rsp);
            //eMMC_dump_mem(g_eMMCDrv.au8_Rsp, eMMC_R3_BYTE_CNT);

            if(0 == (g_eMMCDrv.au8_Rsp[1] & 0x80))
                u32_err =  eMMC_ST_ERR_CMD1_DEV_NOT_RDY;
            else
                g_eMMCDrv.u8_IfSectorMode = (g_eMMCDrv.au8_Rsp[1]&BIT6)>>6;
        }
    }
    #endif

    //eMMC_FCIE_CLK_DIS();
    return u32_err;
}


// send CID
U32 eMMC_CMD2(void)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;

    u32_arg = 0;
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_RSPR2_EN;

    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 2, eMMC_R2_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD2, %Xh \n", u32_err);
        return u32_err;
    }
    else
    {   // check status
        REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
        if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR))
        {
            u32_err = eMMC_ST_ERR_CMD2;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD2 Reg.12: %04Xh\n", u16_reg);
            return u32_err;
        }
        else
        {   // CMD2 ok, do things here (get CID)
            eMMC_FCIE_GetCIFC(0, eMMC_R2_BYTE_CNT>>1, (U16*)g_eMMCDrv.au8_CID);
            //eMMC_dump_mem(g_eMMCDrv.au8_CID, eMMC_R2_BYTE_CNT);
            eMMC_KEEP_RSP(g_eMMCDrv.au8_CID, 2);
        }
    }
    //eMMC_FCIE_CLK_DIS();
    return u32_err;
}


// CMD3: assign RCA. CMD7: select device
U32 eMMC_CMD3_CMD7(U16 u16_RCA, U8 u8_CmdIdx)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;
    U8  u8_retry_r1=0, u8_retry_fcie=0, u8_retry_cmd=0;

    if(7 == u8_CmdIdx)
        g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_RSP_WAIT_D0H;
    else{
        u8_retry_fcie = 0xF0; // CMD3: not retry
    }

    u32_arg = u16_RCA<<16;

    if(7==u8_CmdIdx && u16_RCA!=g_eMMCDrv.u16_RCA)
        u16_ctrl = BIT_SD_CMD_EN;
    else
        u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, u8_CmdIdx, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(3 == u8_CmdIdx)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD%u retry:%u, %Xh \n",
                u8_CmdIdx, u8_retry_cmd, u32_err);
            return u32_err;
        }
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD%u retry:%u, %Xh \n",
                u8_CmdIdx, u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }

        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD%u retry:%u, %Xh \n",
            u8_CmdIdx, u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
    }
    else
    {
        // check status
        REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

        // de-select has no rsp
        if(!(7==u8_CmdIdx && u16_RCA!=g_eMMCDrv.u16_RCA)){
        if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR))
        {
            if(3 == u8_CmdIdx)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD%u Reg.12: %04Xh, Retry: %u\n", u8_CmdIdx, u16_reg, u8_retry_fcie);
                return u32_err;
            }
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD%u Reg.12: %04Xh, Retry: %u\n", u8_CmdIdx, u16_reg, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD3_CMD7;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD%u Reg.12: %04Xh, Retry: %u\n", u8_CmdIdx, u16_reg, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {   // CMD3 ok, do things here
            u32_err = eMMC_CheckR1Error();
            if(eMMC_ST_SUCCESS != u32_err)
            {
                if(3 == u8_CmdIdx)
                {
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                        "eMMC WARN: CMD%u check R1 error: %Xh, retry: %u\n",
                        u8_CmdIdx, u32_err, u8_retry_r1);
                    return u32_err;
                }

                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
                {
                    u8_retry_r1++;
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                        "eMMC WARN: CMD%u check R1 error: %Xh, retry: %u\n",
                        u8_CmdIdx, u32_err, u8_retry_r1);
                    eMMC_FCIE_ErrHandler_Retry();
                    goto LABEL_SEND_CMD;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD%u check R1 error: %Xh, retry: %u\n",
                    u8_CmdIdx, u32_err, u8_retry_r1);
                eMMC_FCIE_ErrHandler_Stop();
            }
            eMMC_KEEP_RSP(g_eMMCDrv.au8_Rsp, u8_CmdIdx);
        }}
    }

    eMMC_FCIE_CLK_DIS();
    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_RSP_WAIT_D0H;
    return u32_err;
}


//------------------------------------------------
U32 eMMC_CSD_Config(void)
{
    U32 u32_err;

    u32_err = eMMC_CMD9(g_eMMCDrv.u16_RCA);
    if(eMMC_ST_SUCCESS != u32_err)
        return u32_err;

    // ------------------------------
    g_eMMCDrv.u8_SPEC_VERS = (g_eMMCDrv.au8_CSD[1]&0x3C)>>2;
    g_eMMCDrv.u8_R_BL_LEN = g_eMMCDrv.au8_CSD[6] & 0xF;
    g_eMMCDrv.u8_W_BL_LEN = ((g_eMMCDrv.au8_CSD[13]&0x3)<<2)+
        ((g_eMMCDrv.au8_CSD[14]&0xC0)>>6);

    // ------------------------------
    g_eMMCDrv.u16_C_SIZE = (g_eMMCDrv.au8_CSD[7] & 3)<<10;
    g_eMMCDrv.u16_C_SIZE += g_eMMCDrv.au8_CSD[8] << 2;
    g_eMMCDrv.u16_C_SIZE +=(g_eMMCDrv.au8_CSD[9] & 0xC0) >> 6;
    if(0xFFF == g_eMMCDrv.u16_C_SIZE)
    {
        g_eMMCDrv.u32_SEC_COUNT = 0;
    }
    else
    {
        g_eMMCDrv.u8_C_SIZE_MULT = ((g_eMMCDrv.au8_CSD[10]&3)<<1)+
            ((g_eMMCDrv.au8_CSD[11]&0x80)>>7);

        g_eMMCDrv.u32_SEC_COUNT =
            (g_eMMCDrv.u16_C_SIZE+1)*
            (1<<(g_eMMCDrv.u8_C_SIZE_MULT+2))*
            ((1<<g_eMMCDrv.u8_R_BL_LEN)>>9) - 8; // -8: //Toshiba CMD18 access the last block report out of range error
    }
    // ------------------------------
    g_eMMCDrv.u8_ERASE_GRP_SIZE = (g_eMMCDrv.au8_CSD[10]&0x7C)>>2;
    g_eMMCDrv.u8_ERASE_GRP_MULT = ((g_eMMCDrv.au8_CSD[10]&0x03)<<3)+
        ((g_eMMCDrv.au8_CSD[11]&0xE0)>>5);
    g_eMMCDrv.u32_EraseUnitSize = (g_eMMCDrv.u8_ERASE_GRP_SIZE+1)*
        (g_eMMCDrv.u8_ERASE_GRP_MULT+1);
    // ------------------------------
    // others
    g_eMMCDrv.u8_TAAC = g_eMMCDrv.au8_CSD[2];
    g_eMMCDrv.u8_NSAC = g_eMMCDrv.au8_CSD[3];
    g_eMMCDrv.u8_Tran_Speed = g_eMMCDrv.au8_CSD[4];
    g_eMMCDrv.u8_R2W_FACTOR = (g_eMMCDrv.au8_CSD[13]&0x1C)>>2;

    return eMMC_ST_SUCCESS;
}


// send CSD (in R2)
U32 eMMC_CMD9(U16 u16_RCA)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;
    U8  u8_retry_fcie=0, u8_retry_cmd=0;

    u32_arg = u16_RCA<<16;
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_RSPR2_EN;

    LABEL_SEND_CMD:
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 9, eMMC_R2_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD9 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }

        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD9 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
    }
    else
    {   // check status
        REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
        if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR))
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD9 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD9;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD9 Reg.12: %04Xh, Retry fail: %u\n", u16_reg, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {   // CMD2 ok, do things here
            eMMC_FCIE_GetCIFC(0, eMMC_R2_BYTE_CNT>>1, (U16*)g_eMMCDrv.au8_CSD);
            //eMMC_dump_mem(g_eMMCDrv.au8_CSD, eMMC_R2_BYTE_CNT);
            eMMC_KEEP_RSP(g_eMMCDrv.au8_CSD, 9);
        }
    }

    eMMC_FCIE_CLK_DIS();
    return u32_err;
}


//------------------------------------------------
U32 eMMC_ExtCSD_Config(void)
{
    U32 u32_err;

    u32_err = eMMC_CMD8(gau8_eMMC_SectorBuf);
    if(eMMC_ST_SUCCESS != u32_err)
        return u32_err;

    //eMMC_dump_mem(gau8_eMMC_SectorBuf, 0x200);

    //--------------------------------
    if(0 == g_eMMCDrv.u32_SEC_COUNT)
         g_eMMCDrv.u32_SEC_COUNT = ((gau8_eMMC_SectorBuf[215]<<24)|
                                   (gau8_eMMC_SectorBuf[214]<<16)|
                                   (gau8_eMMC_SectorBuf[213]<< 8)|
                                   (gau8_eMMC_SectorBuf[212])) - 8; //-8: Toshiba CMD18 access the last block report out of range error

    //-------------------------------
    if(0 == g_eMMCDrv.u32_BOOT_SEC_COUNT)
         g_eMMCDrv.u32_BOOT_SEC_COUNT = gau8_eMMC_SectorBuf[226] * 128 * 2;

    //--------------------------------
    if(!g_eMMCDrv.u8_BUS_WIDTH) {
        g_eMMCDrv.u8_BUS_WIDTH = gau8_eMMC_SectorBuf[183];
        switch(g_eMMCDrv.u8_BUS_WIDTH)
        {
        case 0:  g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_1;  break;
        case 1:  g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_4;  break;
        case 2:  g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_8;  break;
        default: eMMC_debug(0,1,"eMMC Err: eMMC BUS_WIDTH not support \n");
            while(1);
        }
    }
    //--------------------------------
    if(gau8_eMMC_SectorBuf[231]&BIT4) // TRIM
        g_eMMCDrv.u32_eMMCFlag |= eMMC_FLAG_TRIM;
    else
        g_eMMCDrv.u32_eMMCFlag &= ~eMMC_FLAG_TRIM;

    //--------------------------------
    if(gau8_eMMC_SectorBuf[503]&BIT0) // HPI
    {
        if(gau8_eMMC_SectorBuf[503]&BIT1)
            g_eMMCDrv.u32_eMMCFlag |= eMMC_FLAG_HPI_CMD12;
        else
            g_eMMCDrv.u32_eMMCFlag |= eMMC_FLAG_HPI_CMD13;
    }else
        g_eMMCDrv.u32_eMMCFlag &= ~(eMMC_FLAG_HPI_CMD12|eMMC_FLAG_HPI_CMD13);

    //--------------------------------
    if(gau8_eMMC_SectorBuf[166]&BIT2) // Reliable Write
        g_eMMCDrv.u16_ReliableWBlkCnt = BIT_SD_JOB_BLK_CNT_MASK;
    else
    {
        #if 0
        g_eMMCDrv.u16_ReliableWBlkCnt = gau8_eMMC_SectorBuf[222];
        #else
        if((gau8_eMMC_SectorBuf[503]&BIT0) && 1==gau8_eMMC_SectorBuf[222])
            g_eMMCDrv.u16_ReliableWBlkCnt = 1;
        else if(0==(gau8_eMMC_SectorBuf[503]&BIT0))
            g_eMMCDrv.u16_ReliableWBlkCnt = gau8_eMMC_SectorBuf[222];
        else
        {
            //eMMC_debug(0,1,"eMMC Warn: not support dynamic  Reliable-W\n");
            g_eMMCDrv.u16_ReliableWBlkCnt = 0; // can not support Reliable Write
        }
        #endif
    }

    //--------------------------------
    g_eMMCDrv.u8_ErasedMemContent = gau8_eMMC_SectorBuf[181];

    //--------------------------------
    g_eMMCDrv.u8_ECSD185_HsTiming = gau8_eMMC_SectorBuf[185];
    g_eMMCDrv.u8_ECSD192_Ver = gau8_eMMC_SectorBuf[192];
    g_eMMCDrv.u8_ECSD196_DevType = gau8_eMMC_SectorBuf[196];
    g_eMMCDrv.u8_ECSD197_DriverStrength = gau8_eMMC_SectorBuf[197];
    g_eMMCDrv.u8_ECSD248_CMD6TO = gau8_eMMC_SectorBuf[248];
    g_eMMCDrv.u8_ECSD247_PwrOffLongTO = gau8_eMMC_SectorBuf[247];
    g_eMMCDrv.u8_ECSD34_PwrOffCtrl = gau8_eMMC_SectorBuf[34];

    //for GP Partition
    g_eMMCDrv.u8_ECSD160_PartSupField = gau8_eMMC_SectorBuf[160];
    g_eMMCDrv.u8_ECSD224_HCEraseGRPSize= gau8_eMMC_SectorBuf[224];
    g_eMMCDrv.u8_ECSD221_HCWpGRPSize= gau8_eMMC_SectorBuf[221];

    g_eMMCDrv.GP_Part[0].u32_PartSize = ((gau8_eMMC_SectorBuf[145] << 16) |
                                             (gau8_eMMC_SectorBuf[144] << 8) |
                                             (gau8_eMMC_SectorBuf[143])) *
                                             (g_eMMCDrv.u8_ECSD224_HCEraseGRPSize  * g_eMMCDrv.u8_ECSD221_HCWpGRPSize * 0x80000);

    g_eMMCDrv.GP_Part[1].u32_PartSize = ((gau8_eMMC_SectorBuf[148] << 16) |
                                             (gau8_eMMC_SectorBuf[147] << 8) |
                                             (gau8_eMMC_SectorBuf[146])) *
                                             (g_eMMCDrv.u8_ECSD224_HCEraseGRPSize  * g_eMMCDrv.u8_ECSD221_HCWpGRPSize * 0x80000);

    g_eMMCDrv.GP_Part[2].u32_PartSize = ((gau8_eMMC_SectorBuf[151] << 16) |
                                             (gau8_eMMC_SectorBuf[150] << 8) |
                                             (gau8_eMMC_SectorBuf[149])) *
                                             (g_eMMCDrv.u8_ECSD224_HCEraseGRPSize  * g_eMMCDrv.u8_ECSD221_HCWpGRPSize * 0x80000);

    g_eMMCDrv.GP_Part[3].u32_PartSize = ((gau8_eMMC_SectorBuf[154] << 16) |
                                             (gau8_eMMC_SectorBuf[153] << 8) |
                                             (gau8_eMMC_SectorBuf[152])) *
                                             (g_eMMCDrv.u8_ECSD224_HCEraseGRPSize  * g_eMMCDrv.u8_ECSD221_HCWpGRPSize * 0x80000);

    //for Max Enhance Size
    g_eMMCDrv.u8_ECSD157_MaxEnhSize_0= gau8_eMMC_SectorBuf[157];
    g_eMMCDrv.u8_ECSD158_MaxEnhSize_1= gau8_eMMC_SectorBuf[158];
    g_eMMCDrv.u8_ECSD159_MaxEnhSize_2= gau8_eMMC_SectorBuf[159];

    g_eMMCDrv.u8_u8_ECSD155_PartSetComplete = gau8_eMMC_SectorBuf[155];
    g_eMMCDrv.u8_ECSD166_WrRelParam = gau8_eMMC_SectorBuf[166];

    //--------------------------------
    // set HW RST
    if(0 == gau8_eMMC_SectorBuf[162])
    {
        u32_err = eMMC_ModifyExtCSD(eMMC_ExtCSD_WByte, 162, BIT0); // RST_FUNC
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh, eMMC, set Ext_CSD[162]: %Xh fail\n",
                u32_err, BIT0);
            return u32_err;
        }
    }

    return eMMC_ST_SUCCESS;
}


U32 eMMC_CMD8(U8 *pu8_DataBuf)
{
    #if defined(ENABLE_eMMC_RIU_MODE)&&ENABLE_eMMC_RIU_MODE
    return eMMC_CMD8_CIFD(pu8_DataBuf);
    #else
    return eMMC_CMD8_MIU(pu8_DataBuf);
    #endif
}

// CMD8: send EXT_CSD
U32 eMMC_CMD8_MIU(U8 *pu8_DataBuf)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_fifoclk=0, u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;
    U32 u32_DataDMAAddr, u32_DataMapAddr;

    // -------------------------------
    #if 0
    if(0 == eMMC_IF_TUNING_TTABLE())
        eMMC_FCIE_ErrHandler_RestoreClk();
    #endif
    // -------------------------------
    // send cmd
    u32_arg =  0;
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;

    LABEL_SEND_CMD:
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    if(g_eMMCDrv.u32_DrvFlag & (DRV_FLAG_DDR_MODE|DRV_FLAG_SPEED_HS200) )
    {
        REG_FCIE_W(FCIE_TOGGLE_CNT, (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_SPEED_HS200) ? TOGGLE_CNT_512_CLK_R : TOGGLE_CNT_256_CLK_R);
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
        REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
        eMMC_hw_timer_delay(TIME_WAIT_FCIE_RST_TOGGLE_CNT); // Brian needs 2T
        REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
    }
    eMMC_FCIE_ClearEvents();
    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
    u32_DataMapAddr = eMMC_DMA_MAP_address((U32)pu8_DataBuf, eMMC_SECTOR_512BYTE,1);
    u32_DataDMAAddr = eMMC_translate_DMA_address_Ex(u32_DataMapAddr, eMMC_SECTOR_512BYTE);
    #if FICE_BYTE_MODE_ENABLE
    REG_FCIE_W(FCIE_SDIO_ADDR0, u32_DataDMAAddr & 0xFFFF);
    REG_FCIE_W(FCIE_SDIO_ADDR1, u32_DataDMAAddr >> 16);
    #else
    REG_FCIE_W(FCIE_MIU_DMA_15_0, (u32_DataDMAAddr>>MIU_BUS_WIDTH_BITS)&0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_26_16,(u32_DataDMAAddr>>MIU_BUS_WIDTH_BITS)>>16);
    #endif
    REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);
    u32_err = eMMC_FCIE_FifoClkRdy(0);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_fifoclk < eMMC_CMD_API_WAIT_FIFOCLK_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fifoclk++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: CMD8 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
            eMMC_FCIE_Init();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE,1);
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CMD8 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }
    REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);

    u32_err = eMMC_FCIE_SendCmd(u16_mode, u16_ctrl, u32_arg, 8, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD8 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE,1);
            goto LABEL_SEND_CMD;
        }

        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD8 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check FCIE
    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    REG_FCIE_W(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);
    #endif
    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
                                   BIT_MIU_LAST_DONE|BIT_CARD_DMA_END,
                                   TIME_WAIT_1_BLK_END);

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & BIT_SD_FCIE_ERR_FLAGS))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD8 Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE,1);
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD8_MIU;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                   "eMMC Err: CMD8 Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_r1++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD8 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE,1);
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD8_MIU;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD8 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:
    if(g_eMMCDrv.u32_DrvFlag & (DRV_FLAG_DDR_MODE|DRV_FLAG_SPEED_HS200))
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);

    eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE,1);
    eMMC_FCIE_CLK_DIS();
    return u32_err;
}

U32 eMMC_CMD8_CIFD(U8 *pu8_DataBuf)
{
    U32 u32_err, u32_arg;
    volatile U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;;

    REG_FCIE_W(FCIE_CIFD_WORD_CNT, 0);

    // -------------------------------
    // send cmd
    u32_arg =  0;
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;

    LABEL_SEND_CMD:
    u16_mode = BIT_SD_DATA_CIFD | g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    eMMC_FCIE_ClearEvents();

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 8, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD8 CIFD retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD8 CIFD retry:%u, %Xh \n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check FCIE
    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
        BIT_SD_DATA_END, TIME_WAIT_1_BLK_END);

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & BIT_SD_FCIE_ERR_FLAGS))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD8 CIFD Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }

        u32_err = eMMC_ST_ERR_CMD8_CIFD;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD8 CIFD Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_r1++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD8 CIFD check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }

        u32_err = eMMC_ST_ERR_CMD8_CIFD;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD8 CIFD check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }
    //eMMC_KEEP_RSP(g_eMMCDrv.au8_Rsp, 8);

    // -------------------------------
    // CMD8 ok, do things here
    eMMC_FCIE_GetCIFD(0, eMMC_SECTOR_512BYTE>>1, (U16*)pu8_DataBuf);

    LABEL_END:
    eMMC_FCIE_CLK_DIS();
    return u32_err;
}


U32 eMMC_SetPwrOffNotification(U8 u8_SetECSD34)
{
    U32 u32_err;
    static U8 u8_OldECSD34=0;

    if(eMMC_PwrOffNotif_SHORT==u8_OldECSD34 || eMMC_PwrOffNotif_LONG==u8_OldECSD34)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: PwrOffNotif already set: %u, now: %u\n",
            u8_OldECSD34, u8_SetECSD34);
        return eMMC_ST_SUCCESS;
    }

    u32_err = eMMC_ModifyExtCSD(eMMC_ExtCSD_WByte, 34, u8_SetECSD34);
    if(eMMC_ST_SUCCESS != u32_err)
        return u32_err;

    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_PwrOffNotif_MASK;
    switch(u8_SetECSD34)
    {
        case eMMC_PwrOffNotif_OFF:
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_PwrOffNotif_OFF;  break;
        case eMMC_PwrOffNotif_ON:
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_PwrOffNotif_ON;  break;
        case eMMC_PwrOffNotif_SHORT:
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_PwrOffNotif_SHORT;  break;
        case eMMC_PwrOffNotif_LONG:
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_PwrOffNotif_LONG;  break;
    }

    return u32_err;
}


U32 eMMC_Sanitize(U8 u8_ECSD165)
{
    U32 u32_err;

    u32_err = eMMC_ModifyExtCSD(eMMC_ExtCSD_WByte, 165, u8_ECSD165);
    if(eMMC_ST_SUCCESS != u32_err)
        return u32_err;

    u32_err = eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH<<2);
    if(eMMC_ST_SUCCESS != u32_err)
        return u32_err;

    return eMMC_ST_SUCCESS;
}

#define BITS_MSK_DRIVER_STRENGTH    0xF0
#define BITS_MSK_TIMING             0x0F

U32 eMMC_SetBusSpeed(U8 u8_BusSpeed)
{
    U32 u32_err;

    g_eMMCDrv.u8_ECSD185_HsTiming &= ~BITS_MSK_TIMING;
    g_eMMCDrv.u8_ECSD185_HsTiming |= u8_BusSpeed;

    u32_err = eMMC_ModifyExtCSD(eMMC_ExtCSD_WByte, 185, g_eMMCDrv.u8_ECSD185_HsTiming);
    if(eMMC_ST_SUCCESS != u32_err)
        return u32_err;

    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_SPEED_MASK;
    switch(u8_BusSpeed)
    {
        case eMMC_SPEED_HIGH:
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_SPEED_HIGH;
            g_eMMCDrv.u16_Reg10_Mode |= BIT_SD_DATA_SYNC;
            break;
        case eMMC_SPEED_HS200:
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_SPEED_HS200;
            g_eMMCDrv.u16_Reg10_Mode &= ~BIT_SD_DATA_SYNC;
            break;
        case eMMC_SPEED_HS400:
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_SPEED_HS400;
            g_eMMCDrv.u16_Reg10_Mode &= ~BIT_SD_DATA_SYNC;
            break;
        default:
            g_eMMCDrv.u16_Reg10_Mode &= ~BIT_SD_DATA_SYNC;
    }

    return u32_err;
}

U32 eMMC_SetDrivingStrength(U8 u8Driving)
{
    U32 u32_err;

    g_eMMCDrv.u8_ECSD185_HsTiming &= ~BITS_MSK_DRIVER_STRENGTH;
    g_eMMCDrv.u8_ECSD185_HsTiming |= u8Driving<<4;

    u32_err = eMMC_ModifyExtCSD(eMMC_ExtCSD_WByte, 185, g_eMMCDrv.u8_ECSD185_HsTiming); // HS_TIMING, HS200
    if(eMMC_ST_SUCCESS != u32_err) {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: fail: %Xh\n", u32_err);
        return eMMC_ST_ERR_SET_DRV_STRENGTH;
    }
    return eMMC_ST_SUCCESS;
}

// Use CMD6 to set ExtCSD[183] BUS_WIDTH
U32 eMMC_SetBusWidth(U8 u8_BusWidth, U8 u8_IfDDR)
{
    U8  u8_value;
    U32 u32_err;

    // -------------------------------
    switch(u8_BusWidth)
    {
        case 1:  u8_value=0; break;
        case 4:  u8_value=1; break;
        case 8:  u8_value=2; break;
        default: return eMMC_ST_ERR_PARAMETER;
    }

    if(u8_IfDDR)
    {
        u8_value |= BIT2;
        g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_DDR_MODE;
    }
    else
        g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_DDR_MODE;


    // -------------------------------
    u32_err = eMMC_ModifyExtCSD(eMMC_ExtCSD_WByte, 183, u8_value); // BUS_WIDTH
    if(eMMC_ST_SUCCESS != u32_err)
        return u32_err;

    // -------------------------------
    g_eMMCDrv.u16_Reg10_Mode &= ~BIT_SD_DATA_WIDTH_MASK;
    switch(u8_BusWidth)
    {
        case 1:
            g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_1;
            g_eMMCDrv.u16_Reg10_Mode |= BIT_SD_DATA_WIDTH_1;
            break;
        case 4:
            g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_4;
            g_eMMCDrv.u16_Reg10_Mode |= BIT_SD_DATA_WIDTH_4;
            break;
        case 8:
            g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_8;
            g_eMMCDrv.u16_Reg10_Mode |= BIT_SD_DATA_WIDTH_8;
            break;
    }

    //eMMC_debug(eMMC_DEBUG_LEVEL,1,"set %u bus width\n", u8_BusWidth);
    return u32_err;
}

U32 eMMC_ModifyExtCSD(U8 u8_AccessMode, U8 u8_ByteIdx, U8 u8_Value)
{
    U32 u32_arg, u32_err;

    u32_arg = ((u8_AccessMode&3)<<24) | (u8_ByteIdx<<16) |
              (u8_Value<<8);

    u32_err = eMMC_CMD6(u32_arg);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC: %Xh \n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_CMD13(g_eMMCDrv.u16_RCA);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: %Xh \n", u32_err);
        return u32_err;
    }

    return u32_err;
}

// SWITCH cmd
U32 eMMC_CMD6(U32 u32_Arg)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;
    U8  u8_retry_r1=0, u8_retry_fcie=0, u8_retry_cmd=0;

    u32_arg = u32_Arg;
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
    g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_RSP_WAIT_D0H;

    LABEL_SEND_CMD:
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 6, eMMC_R1b_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD6 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD6 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
    }
    else
    {   // check status
        REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

        if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR))
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD6 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_ReInit();
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }
            u32_err = eMMC_ST_ERR_CMD6;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD6 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {   // CMD3 ok, do things here
            u32_err = eMMC_CheckR1Error();
            if(eMMC_ST_SUCCESS != u32_err)
            {
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE()){
                    u8_retry_r1++;
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                        "eMMC WARN: CMD6 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
                    eMMC_FCIE_ErrHandler_ReInit();
                    eMMC_FCIE_ErrHandler_Retry();
                    goto LABEL_SEND_CMD;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD6 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
                eMMC_FCIE_ErrHandler_Stop();
            }
            eMMC_KEEP_RSP(g_eMMCDrv.au8_Rsp, 6);
        }
    }

    eMMC_FCIE_CLK_DIS();
    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_RSP_WAIT_D0H;
    return u32_err;
}


U32 eMMC_EraseCMDSeq(U32 u32_eMMCBlkAddr_start, U32 u32_eMMCBlkAddr_end)
{
    U32 u32_err;
    U8  u8_retry_cmd=0;

    LABEL_SEND_CMD:
    u32_err = eMMC_CMD35_CMD36(u32_eMMCBlkAddr_start, 35);
    if(eMMC_ST_SUCCESS != u32_err)
        goto LABEN_END;

    u32_err = eMMC_CMD35_CMD36(u32_eMMCBlkAddr_end, 36);
    if(eMMC_ST_SUCCESS != u32_err)
        goto LABEN_END;

    u32_err = eMMC_CMD38();
    if(eMMC_ST_SUCCESS != u32_err)
        goto LABEN_END;

    return eMMC_ST_SUCCESS;

    LABEN_END:
    if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
    {
        u8_retry_cmd++;
        eMMC_FCIE_ErrHandler_ReInit();
        eMMC_FCIE_ErrHandler_Retry();
        goto LABEL_SEND_CMD;
    }
    eMMC_FCIE_ErrHandler_Stop();
    return u32_err;
}


U32 eMMC_CMD35_CMD36(U32 u32_eMMCBlkAddr, U8 u8_CmdIdx)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;

    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, u8_CmdIdx, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD%u fail, %Xh \n",
            u8_CmdIdx, u32_err);
        return u32_err;
    }
    else
    {   // check status
        REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

        if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR))
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD%u Reg.12: %04Xh \n", u8_CmdIdx, u16_reg);
            return eMMC_ST_ERR_CMD3536_ERR;
        }
        else
        {   // CMD3 ok, do things here
            u32_err = eMMC_CheckR1Error();
            if(eMMC_ST_SUCCESS != u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD%u check R1 error: %Xh\n", u8_CmdIdx, u32_err);
                return u32_err;
            }
            eMMC_KEEP_RSP(g_eMMCDrv.au8_Rsp, u8_CmdIdx);
        }
    }

    eMMC_FCIE_CLK_DIS();
    return u32_err;
}

U32 eMMC_CMD38(void)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;

    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
    if(g_eMMCDrv.u32_eMMCFlag & eMMC_FLAG_TRIM)
        u32_arg =  0x1;
    else
        u32_arg =  0x0;

    g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_RSP_WAIT_D0H;

    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 38, eMMC_R1b_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD38 fail: %Xh \n", u32_err);
        return u32_err;
    }
    else
    {
        // check status
        REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

        if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR))
        {
            u32_err = eMMC_ST_ERR_CMD38_ERR;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD38 Reg.12: %04Xh\n", u16_reg);
            return u32_err;
        }
        else
        {   // CMD38 ok, do things here
            u32_err = eMMC_CheckR1Error();
            if(eMMC_ST_SUCCESS != u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD38 check R1 error: %Xh\n", u32_err);
                return u32_err;
            }
            eMMC_KEEP_RSP(g_eMMCDrv.au8_Rsp, 38);
        }
    }

    eMMC_FCIE_CLK_DIS();
    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_RSP_WAIT_D0H;
    return u32_err;
}


// CMD13: send Status
U32 eMMC_CMD13(U16 u16_RCA)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;
    U8  u8_retry_fcie=0, u8_retry_cmd=0;// ,u8_retry_r1=0;

    u32_arg = (u16_RCA<<16); // |
    //  ((g_eMMCDrv.u32_eMMCFlag & eMMC_FLAG_HPI_CMD13)?1:0);
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 13, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD13 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD13 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
    }
    else
    {   // check status
        REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

        if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR))
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD13 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_ReInit();
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD13;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD13 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {   // CMD13 ok, do things here
            u32_err = eMMC_CheckR1Error();
            if(eMMC_ST_SUCCESS != u32_err)
            {
                #if 0
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE()){
                    u8_retry_r1++;
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                        "eMMC WARN: CMD13 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
                    eMMC_FCIE_ErrHandler_Retry();
                    goto LABEL_SEND_CMD;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD13 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
                eMMC_FCIE_ErrHandler_Stop();
                #else
                eMMC_debug(0, 1,
                    "eMMC: CMD13 check R1 error: %Xh, should not retry\n", u32_err);
                #endif
            }
            eMMC_KEEP_RSP(g_eMMCDrv.au8_Rsp, 13);
        }
    }

    eMMC_FCIE_CLK_DIS();
    return u32_err;
}

U32 eMMC_CMD16(U32 u32_BlkLength)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;
    U8  u8_retry_r1=0, u8_retry_fcie=0, u8_retry_cmd=0;

    u32_arg = u32_BlkLength;
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 16, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD16 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD16 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
    }
    else
    {   // check status
        REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

        if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR))
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD16 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD16;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD16 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {   // CMD16 ok, do things here
            u32_err = eMMC_CheckR1Error();
            if(eMMC_ST_SUCCESS != u32_err)
            {
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE()){
                    u8_retry_r1++;
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                        "eMMC WARN: CMD16 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
                    eMMC_FCIE_ErrHandler_Retry();
                    goto LABEL_SEND_CMD;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD16 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
                eMMC_FCIE_ErrHandler_Stop();
            }
            eMMC_KEEP_RSP(g_eMMCDrv.au8_Rsp, 16);
        }
    }

    eMMC_FCIE_CLK_DIS();
    return u32_err;
}

//------------------------------------------------

U32 eMMC_CMD17(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf)
{
    #if defined(ENABLE_eMMC_RIU_MODE)&&ENABLE_eMMC_RIU_MODE
    return eMMC_CMD17_CIFD(u32_eMMCBlkAddr, pu8_DataBuf);
    #else
    return eMMC_CMD17_MIU(u32_eMMCBlkAddr, pu8_DataBuf);
    #endif
}

#if 0
static  U32 u32_TestCnt=0;
#define IF_TEST_RETRY(x)  (0==++u32_TestCnt%x)
#else
#define IF_TEST_RETRY(x)  (0)//==++u32_TestCnt%x)
#endif

U32 eMMC_CMD17_MIU(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_fifoclk=0, u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;
    U32 u32_DataDMAAddr, u32_DataMapAddr;

    // -------------------------------
    if(0 == eMMC_IF_TUNING_TTABLE())
        eMMC_FCIE_ErrHandler_RestoreClk();

    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;

    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    if(g_eMMCDrv.u32_DrvFlag & (DRV_FLAG_DDR_MODE|DRV_FLAG_SPEED_HS200) )
    {
        REG_FCIE_W(FCIE_TOGGLE_CNT, (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_SPEED_HS200) ? TOGGLE_CNT_512_CLK_R : TOGGLE_CNT_256_CLK_R);
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
        REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
        eMMC_hw_timer_delay(TIME_WAIT_FCIE_RST_TOGGLE_CNT); // Brian needs 2T
        REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
    }
    eMMC_FCIE_ClearEvents();
    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
    u32_DataMapAddr = eMMC_DMA_MAP_address((U32)pu8_DataBuf,eMMC_SECTOR_512BYTE,1);
    u32_DataDMAAddr = eMMC_translate_DMA_address_Ex(u32_DataMapAddr, eMMC_SECTOR_512BYTE);
    #if FICE_BYTE_MODE_ENABLE
    REG_FCIE_W(FCIE_SDIO_ADDR0, u32_DataDMAAddr & 0xFFFF);
    REG_FCIE_W(FCIE_SDIO_ADDR1, u32_DataDMAAddr >> 16);
    #else
    REG_FCIE_W(FCIE_MIU_DMA_15_0, (u32_DataDMAAddr>>MIU_BUS_WIDTH_BITS)&0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_26_16,(u32_DataDMAAddr>>MIU_BUS_WIDTH_BITS)>>16);
    #endif
    REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);
    u32_err = eMMC_FCIE_FifoClkRdy(0);
    if(IF_TEST_RETRY(23) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_fifoclk < eMMC_CMD_API_WAIT_FIFOCLK_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fifoclk++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: CMD17 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
            eMMC_FCIE_Init();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr,eMMC_SECTOR_512BYTE,1);
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CMD17 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }
    REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 17, eMMC_R1_BYTE_CNT);
    if(IF_TEST_RETRY(17) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD17 retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr,eMMC_SECTOR_512BYTE,1);
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD17 retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check FCIE
    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    REG_FCIE_W(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);
    #endif
    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
        BIT_MIU_LAST_DONE|BIT_CARD_DMA_END, TIME_WAIT_1_BLK_END);

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(IF_TEST_RETRY(33) || eMMC_ST_SUCCESS != u32_err || (u16_reg & BIT_SD_FCIE_ERR_FLAGS))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD17 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr,eMMC_SECTOR_512BYTE,1);
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD17_MIU;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD17 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(IF_TEST_RETRY(7) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_r1++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD17 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr,eMMC_SECTOR_512BYTE,1);
            goto LABEL_SEND_CMD;
        }
        else
        {   u32_err = eMMC_ST_ERR_CMD17_MIU;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD17 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }

    LABEL_END:
    if(g_eMMCDrv.u32_DrvFlag & (DRV_FLAG_DDR_MODE|DRV_FLAG_SPEED_HS200))
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);

    eMMC_DMA_UNMAP_address(u32_DataMapAddr,eMMC_SECTOR_512BYTE,1);
    eMMC_FCIE_CLK_DIS();
    return u32_err;
}

U32 eMMC_CMD17_CIFD(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_r1=0, u8_retry_fcie=0, u8_retry_cmd=0;

    REG_FCIE_W(FCIE_CIFD_WORD_CNT, 0);

    //if(0 == eMMC_IF_TUNING_TTABLE())
    //  eMMC_FCIE_ErrHandler_RestoreClk();

    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;

    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = BIT_SD_DATA_CIFD | g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    if(g_eMMCDrv.u32_DrvFlag & (DRV_FLAG_DDR_MODE|DRV_FLAG_SPEED_HS200) )
    {
        sgu8_IfNeedRestorePadType = g_eMMCDrv.u8_PadType;
        u32_err = eMMC_FCIE_EnableSDRMode();
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableSDRMode fail: %Xh\n", u32_err);
            return u32_err;
        }
    }
    eMMC_FCIE_ClearEvents();

    u32_err = eMMC_FCIE_SendCmd(u16_mode, u16_ctrl, u32_arg, 17, eMMC_R1_BYTE_CNT);
    if(IF_TEST_RETRY(31) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT  && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD17 CIFD retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD17 CIFD retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check FCIE
    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
        BIT_SD_DATA_END, TIME_WAIT_1_BLK_END);

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(IF_TEST_RETRY(28) || eMMC_ST_SUCCESS != u32_err || (u16_reg & BIT_SD_FCIE_ERR_FLAGS))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD17 CIFD Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD17_CIFD;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD17 CIFD Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(IF_TEST_RETRY(33) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_r1++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD17 CIFD check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }

        u32_err = eMMC_ST_ERR_CMD17_CIFD;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD17 CIFD check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // CMD17 ok, do things here
    eMMC_FCIE_GetCIFD(0, eMMC_SECTOR_512BYTE>>1, (U16*)pu8_DataBuf);

    LABEL_END:
    eMMC_FCIE_CLK_DIS();
    return u32_err;
}


U32 eMMC_CMD12(U16 u16_RCA)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;
    U8  u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;

    #if 1
    u32_arg = (u16_RCA<<16)|
        ((g_eMMCDrv.u32_eMMCFlag & eMMC_FLAG_HPI_CMD12)?1:0);
    #else
    u32_arg = (u16_RCA<<16);
    #endif

    #if defined(eMMC_UPDATE_FIRMWARE) && (eMMC_UPDATE_FIRMWARE)
    u32_arg = 0;
    #endif

    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
    g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_RSP_WAIT_D0H;

    //LABEL_SEND_CMD:
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 12, eMMC_R1b_BYTE_CNT);
    if(IF_TEST_RETRY(10) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD12 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_ReInit();
            //eMMC_FCIE_ErrHandler_Retry();
            //goto LABEL_SEND_CMD;
            return u32_err;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD12 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
    }
    else
    {   // check status
        REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

        if(IF_TEST_RETRY(3) || (u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR)))
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD12 check Reg.12: %Xh, %Xh, retry: %u\n",
                    u16_reg, u32_err, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_ReInit();
                //eMMC_FCIE_ErrHandler_Retry();
                //goto LABEL_SEND_CMD;
                return u32_err;
            }

            u32_err = eMMC_ST_ERR_CMD12;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD12 check Reg.12: %Xh, %Xh, retry: %u\n",
                u16_reg, u32_err, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {   u32_err = eMMC_CheckR1Error();
            if(IF_TEST_RETRY(13) || eMMC_ST_SUCCESS != u32_err)
            {
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
                {
                    u8_retry_r1++;
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                        "eMMC WARN: CMD12 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
                    //eMMC_FCIE_ErrHandler_Retry();
                    //goto LABEL_SEND_CMD;
                    return u32_err;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD12 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
                eMMC_FCIE_ErrHandler_Stop();
            }
            eMMC_KEEP_RSP(g_eMMCDrv.au8_Rsp, 12);
        }
    }

    eMMC_FCIE_CLK_DIS();
    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_RSP_WAIT_D0H;
    return u32_err;
}


U32 eMMC_CMD12_NoCheck(U16 u16_RCA)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl;

    #if 1
    u32_arg = (u16_RCA<<16)|
        ((g_eMMCDrv.u32_eMMCFlag & eMMC_FLAG_HPI_CMD12)?1:0);
    #else
    u32_arg = (u16_RCA<<16);
    #endif

    #if defined(eMMC_UPDATE_FIRMWARE) && (eMMC_UPDATE_FIRMWARE)
    u32_arg = 0;
    #endif

    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
    g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_RSP_WAIT_D0H;

    //LABEL_SEND_CMD:
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 12, eMMC_R1b_BYTE_CNT);

    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_RSP_WAIT_D0H;
    return u32_err;
}



U32 eMMC_CMD18(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf, U16 u16_BlkCnt)
{
    #if defined(ENABLE_eMMC_RIU_MODE)&&ENABLE_eMMC_RIU_MODE
    U16 u16_cnt;
    U32 u32_err;

    for(u16_cnt=0; u16_cnt<u16_BlkCnt; u16_cnt++)
    {
        u32_err = eMMC_CMD17_CIFD(u32_eMMCBlkAddr+u16_cnt, pu8_DataBuf+(u16_cnt<<9));
        if(eMMC_ST_SUCCESS != u32_err)
            return u32_err;
    }
    return eMMC_ST_SUCCESS;
    #else
    return eMMC_CMD18_MIU(u32_eMMCBlkAddr, pu8_DataBuf, u16_BlkCnt);
    #endif
}


U32 eMMC_CMD18_MIU(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf, U16 u16_BlkCnt)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_fifoclk=0, u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;
    U32 u32_DataDMAAddr, u32_DataMapAddr;

    // -------------------------------
    if(0 == eMMC_IF_TUNING_TTABLE())
        eMMC_FCIE_ErrHandler_RestoreClk();

    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;

    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH | BIT_SD_DMA_R_CLK_STOP;
    if(g_eMMCDrv.u32_DrvFlag & (DRV_FLAG_DDR_MODE|DRV_FLAG_SPEED_HS200) )
    {
        REG_FCIE_W(FCIE_TOGGLE_CNT, (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_SPEED_HS200) ? TOGGLE_CNT_512_CLK_R : TOGGLE_CNT_256_CLK_R);
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
        REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
        eMMC_hw_timer_delay(TIME_WAIT_FCIE_RST_TOGGLE_CNT); // Brian needs 2T
        REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
    }
    eMMC_FCIE_ClearEvents();
    REG_FCIE_W(FCIE_JOB_BL_CNT, u16_BlkCnt);
    u32_DataMapAddr = eMMC_DMA_MAP_address((U32)pu8_DataBuf,eMMC_SECTOR_512BYTE*u16_BlkCnt,1);
    u32_DataDMAAddr = eMMC_translate_DMA_address_Ex(u32_DataMapAddr, eMMC_SECTOR_512BYTE*u16_BlkCnt);
    #if FICE_BYTE_MODE_ENABLE
    REG_FCIE_W(FCIE_SDIO_ADDR0, u32_DataDMAAddr & 0xFFFF);
    REG_FCIE_W(FCIE_SDIO_ADDR1, u32_DataDMAAddr >> 16);
    #else
    REG_FCIE_W(FCIE_MIU_DMA_15_0, (u32_DataDMAAddr >> MIU_BUS_WIDTH_BITS)& 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_26_16,(u32_DataDMAAddr >> MIU_BUS_WIDTH_BITS)>>16);
    #endif
    REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);
    u32_err = eMMC_FCIE_FifoClkRdy(0);
    if(IF_TEST_RETRY(13) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_fifoclk < eMMC_CMD_API_WAIT_FIFOCLK_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fifoclk++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: CMD18 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
            eMMC_FCIE_Init();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr,eMMC_SECTOR_512BYTE*u16_BlkCnt,1);
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CMD18 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }
    REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);

    u32_err = eMMC_FCIE_SendCmd(u16_mode, u16_ctrl, u32_arg, 18, eMMC_R1_BYTE_CNT);
    if(IF_TEST_RETRY(20) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD18 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr,eMMC_SECTOR_512BYTE*u16_BlkCnt,1);
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD18 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check FCIE
    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    REG_FCIE_W(FCIE_MIE_INT_EN, BIT_MIU_LAST_DONE);
    #endif
    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
        BIT_MIU_LAST_DONE|BIT_CARD_DMA_END, TIME_WAIT_n_BLK_END*(1+(u16_BlkCnt>>11)));

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(IF_TEST_RETRY(13) || eMMC_ST_SUCCESS != u32_err || (u16_reg & BIT_SD_FCIE_ERR_FLAGS))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD18 Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);

            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr,eMMC_SECTOR_512BYTE*u16_BlkCnt,1);
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD18;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD18 Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(IF_TEST_RETRY(24) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_r1++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD18 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);

            eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr,eMMC_SECTOR_512BYTE*u16_BlkCnt,1);
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD18;
        eMMC_debug(1, 1, "eMMC Err: CMD18 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:
    if(g_eMMCDrv.u32_DrvFlag & (DRV_FLAG_DDR_MODE|DRV_FLAG_SPEED_HS200))
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);

    if(eMMC_ST_SUCCESS != eMMC_CMD12(g_eMMCDrv.u16_RCA))
        eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);

    eMMC_DMA_UNMAP_address(u32_DataMapAddr,eMMC_SECTOR_512BYTE*u16_BlkCnt,1);
    eMMC_FCIE_CLK_DIS();
    return u32_err;
}

// enable Reliable Write
U32 eMMC_CMD23(U16 u16_BlkCnt)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;
    U8  u8_retry_r1=0, u8_retry_fcie=0, u8_retry_cmd=0;

    u32_arg = u16_BlkCnt&0xFFFF; // don't set BIT24
    #if defined(eMMC_FEATURE_RELIABLE_WRITE) && eMMC_FEATURE_RELIABLE_WRITE
    u32_arg |= BIT31; // don't set BIT24
    #endif
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 23, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD23 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD23 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
    }
    else
    {   // check status
        REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

        if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR))
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD23 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_ReInit();
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD13;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD23 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {   // CMD13 ok, do things here
            u32_err = eMMC_CheckR1Error();
            if(eMMC_ST_SUCCESS != u32_err)
            {
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE()){
                    u8_retry_r1++;
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                        "eMMC WARN: CMD23 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
                    eMMC_FCIE_ErrHandler_ReInit();
                    eMMC_FCIE_ErrHandler_Retry();
                    goto LABEL_SEND_CMD;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD23 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
                eMMC_FCIE_ErrHandler_Stop();
            }
            eMMC_KEEP_RSP(g_eMMCDrv.au8_Rsp, 23);
        }
    }

    eMMC_FCIE_CLK_DIS();
    return u32_err;
}


U32 eMMC_CMD25(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf, U16 u16_BlkCnt)
{
    U32 u32_err=eMMC_ST_SUCCESS;
    #if defined(ENABLE_eMMC_RIU_MODE)&&ENABLE_eMMC_RIU_MODE
    U16 u16_cnt;
    #endif

    #if eMMC_BURST_LEN_AUTOCFG || ENABLE_eMMC_RIU_MODE
    U16 u16_RetryCnt=0;
    LABEL_CMD25:
    #endif
    // ------------------------------RIU mode
    #if defined(ENABLE_eMMC_RIU_MODE)&&ENABLE_eMMC_RIU_MODE
    for(u16_cnt=0; u16_cnt<u16_BlkCnt; u16_cnt++)
    {
        u32_err = eMMC_CMD24_CIFD(u32_eMMCBlkAddr+u16_cnt, pu8_DataBuf+(u16_cnt<<eMMC_SECTOR_BYTECNT_BITS));
        if(eMMC_ST_SUCCESS != u32_err)
            goto LABEL_CMD25_END;
    }
    #else
    // ------------------------------MIU mode
        #if defined(eMMC_BURST_LEN_AUTOCFG) && eMMC_BURST_LEN_AUTOCFG
    {
        U16 u16_blk_cnt, u16_blk_pos;

        u16_blk_pos = 0;

        if(g_eMMCDrv.BurstWriteLen_t.u16_BestBrustLen)
            while(u16_blk_pos < u16_BlkCnt)
            {
                u16_blk_cnt = u16_BlkCnt-u16_blk_pos > g_eMMCDrv.BurstWriteLen_t.u16_BestBrustLen ?
                    g_eMMCDrv.BurstWriteLen_t.u16_BestBrustLen : u16_BlkCnt-u16_blk_pos;
                u16_blk_cnt = u16_blk_cnt == g_eMMCDrv.BurstWriteLen_t.u16_WorstBrustLen ?
                    u16_blk_cnt/2 : u16_blk_cnt;

                u32_err = eMMC_CMD25_MIU(u32_eMMCBlkAddr+u16_blk_pos,
                    pu8_DataBuf+(u16_blk_pos<<eMMC_SECTOR_BYTECNT_BITS),
                    u16_blk_cnt);
                if(eMMC_ST_SUCCESS != u32_err)
                    goto LABEL_CMD25_END;

                u16_blk_pos += u16_blk_cnt;
            }
        else
            u32_err = eMMC_CMD25_MIU(u32_eMMCBlkAddr, pu8_DataBuf, u16_BlkCnt);
    }
        #else
    u32_err = eMMC_CMD25_MIU(u32_eMMCBlkAddr, pu8_DataBuf, u16_BlkCnt);
        #endif
    #endif

    #if eMMC_BURST_LEN_AUTOCFG || ENABLE_eMMC_RIU_MODE
    LABEL_CMD25_END:
    if(eMMC_ST_SUCCESS!=u32_err && u16_RetryCnt<eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT)
    {
        u16_RetryCnt++;
        goto LABEL_CMD25;
    }
    #endif

    return u32_err;
}

U32 eMMC_CMD25_NoSched(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf, U16 u16_BlkCnt)
{
    U32 u32_err=eMMC_ST_SUCCESS;
    U32 (*pFn)(U32, U16, U32) = sgpFn_eMMC_FCIE_WaitEvents_Ex;

    sgpFn_eMMC_FCIE_WaitEvents_Ex = eMMC_FCIE_PollingEvents;

    u32_err = eMMC_CMD25(u32_eMMCBlkAddr, pu8_DataBuf, u16_BlkCnt);

    sgpFn_eMMC_FCIE_WaitEvents_Ex = pFn;

    return u32_err;
}

#if defined(eMMC_BURST_LEN_AUTOCFG) && eMMC_BURST_LEN_AUTOCFG
void eMMC_DumpBurstLenTable(void)
{
    eMMC_debug(eMMC_DEBUG_LEVEL,0,"  eMMC Burst Len: \n");
    eMMC_debug(eMMC_DEBUG_LEVEL,0,"    best:  %4u blks, %4u.%u MB/s \n",
        g_eMMCDrv.BurstWriteLen_t.u16_BestBrustLen,
        g_eMMCDrv.BurstWriteLen_t.u16_BestMBPerSec, g_eMMCDrv.BurstWriteLen_t.u16_BestMBPerSecPoint);
    eMMC_debug(eMMC_DEBUG_LEVEL,0,"    worst: %4u blks, %4u.%u MB/s \n",
        g_eMMCDrv.BurstWriteLen_t.u16_WorstBrustLen,
        g_eMMCDrv.BurstWriteLen_t.u16_WorstMBPerSec, g_eMMCDrv.BurstWriteLen_t.u16_WorstMBPerSecPoint);
}

U32 eMMC_LoadBurstLenTable(void)
{
    U32 u32_err, u32_ErrRetry=0;
    U32 u32_ChkSum;

    LABEL_RETRY:
    u32_err = eMMC_CMD18(eMMC_BURST_LEN_BLK_0, gau8_eMMC_SectorBuf, 1);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: load Burst Len Table fail, %Xh\n", u32_err);
        goto LABEL_END_OF_NO_TABLE;
    }

    memcpy((U8*)&g_eMMCDrv.BurstWriteLen_t, gau8_eMMC_SectorBuf, sizeof(g_eMMCDrv.BurstWriteLen_t));
    u32_ChkSum = eMMC_ChkSum((U8*)&g_eMMCDrv.BurstWriteLen_t, sizeof(g_eMMCDrv.BurstWriteLen_t)-4);
    if(u32_ChkSum != g_eMMCDrv.BurstWriteLen_t.u32_ChkSum ||
        0 == g_eMMCDrv.BurstWriteLen_t.u16_BestBrustLen)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: no Burst Len Table. ChkSum: %Xh, BestLen: %Xh\n",
            g_eMMCDrv.BurstWriteLen_t.u32_ChkSum, g_eMMCDrv.BurstWriteLen_t.u16_BestBrustLen);
        u32_err = eMMC_ST_ERR_BLEN_CHKSUM;
        goto LABEL_END_OF_NO_TABLE;
    }

    if(u32_ErrRetry)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"eMMC: retry ok\n");
        //eMMC_dump_mem((U8*)&g_eMMCDrv.BurstWriteLen_t, sizeof(g_eMMCDrv.BurstWriteLen_t));
    }
    return u32_err;

    // ---------------------------------
    LABEL_END_OF_NO_TABLE:

    if(0==u32_ErrRetry)
    {
        //eMMC_dump_mem((U8*)&g_eMMCDrv.BurstWriteLen_t, sizeof(g_eMMCDrv.BurstWriteLen_t));
        u32_ErrRetry++;
        goto LABEL_RETRY;
    }
    return u32_err;
}

#define MAX_DETECT_BLK_CNT   2048 // 0.5MB
U32 eMMC_SaveBurstLenTable(void)
{
    U32 u32_Err;
    U32 u32_t0, u32_t1, u32_tMax=0, u32_tMin=(U32)(0-1), u32_tmp;
    U16 u16_BurstBlkCnt, u16_BestBlkCnt=0, u16_WorstBlkCnt=0;
    U16 u16_i;

    //eMMC_debug(eMMC_DEBUG_LEVEL,0,"\neMMC: burst len...\n");
    // --------------------------
    g_eMMCDrv.BurstWriteLen_t.u32_ChkSum = ~g_eMMCDrv.BurstWriteLen_t.u32_ChkSum;
    g_eMMCDrv.BurstWriteLen_t.u16_BestBrustLen = 0;
    g_eMMCDrv.BurstWriteLen_t.u16_WorstBrustLen = 0;

    //eMMC_CMD25(eMMC_TEST_BLK_0, gau8_eMMC_SectorBuf, eMMC_TEST_BLK_CNT);

    // --------------------------
    u16_BurstBlkCnt = 0;
    LABEL_DETECT:
    switch(u16_BurstBlkCnt)
    {
        //case 0:   u16_BurstBlkCnt = 8;  break; // 4KB
        case 0:   u16_BurstBlkCnt = 16;  break;// 8KB
        case 16:  u16_BurstBlkCnt = 32;  break;//16KB
        case 32:  u16_BurstBlkCnt = 64;  break;//32KB
        case 64:  u16_BurstBlkCnt = 128;  break;//64KB
        case 128: u16_BurstBlkCnt = 256;  break;//128KB
        case 256: u16_BurstBlkCnt = MAX_DETECT_BLK_CNT;
    }
    eMMC_hw_timer_start();
    u32_t0 = eMMC_hw_timer_tick();
    for(u16_i=0; u16_i<MAX_DETECT_BLK_CNT/u16_BurstBlkCnt; u16_i++)
    {
        u32_Err= eMMC_CMD25(eMMC_TEST_BLK_0, gau8_eMMC_SectorBuf, u16_BurstBlkCnt);
        if(eMMC_ST_SUCCESS != u32_Err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: w fail: %Xh\n", u32_Err);
            goto LABEL_END;
        }
    }
    eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH);
    u32_t1 = eMMC_hw_timer_tick();
    //eMMC_debug(0,0,"\n%u-%u=%8u, %3u ", u32_t1, u32_t0, u32_t1-u32_t0, u16_BurstBlkCnt);

    if(u32_tMin > u32_t1-u32_t0)
    {
        u32_tMin = u32_t1-u32_t0;
        u16_BestBlkCnt = u16_BurstBlkCnt;
    }
    if(u32_tMax < u32_t1-u32_t0)
    {
        u32_tMax = u32_t1-u32_t0;
        u16_WorstBlkCnt = u16_BurstBlkCnt;
    }

    if(MAX_DETECT_BLK_CNT != u16_BurstBlkCnt)
        goto LABEL_DETECT;

    // --------------------------
    g_eMMCDrv.BurstWriteLen_t.u16_BestBrustLen = u16_BestBlkCnt;
    g_eMMCDrv.BurstWriteLen_t.u16_WorstBrustLen = u16_WorstBlkCnt;

    u32_tmp = (U32)(MAX_DETECT_BLK_CNT<<eMMC_SECTOR_512BYTE_BITS)*4000 /
        (u32_tMin/(eMMC_HW_TIMER_HZ/4000));
    u32_tmp /= 0x100000/10;
    g_eMMCDrv.BurstWriteLen_t.u16_BestMBPerSec = u32_tmp/10;
    g_eMMCDrv.BurstWriteLen_t.u16_BestMBPerSecPoint = u32_tmp%10;

    u32_tmp = (U32)(MAX_DETECT_BLK_CNT<<eMMC_SECTOR_512BYTE_BITS)*4000 /
        (u32_tMax/(eMMC_HW_TIMER_HZ/4000));
    u32_tmp /= 0x100000/10;
    g_eMMCDrv.BurstWriteLen_t.u16_WorstMBPerSec = u32_tmp/10;
    g_eMMCDrv.BurstWriteLen_t.u16_WorstMBPerSecPoint = u32_tmp%10;

    g_eMMCDrv.BurstWriteLen_t.u32_ChkSum =
        eMMC_ChkSum((U8*)&g_eMMCDrv.BurstWriteLen_t, sizeof(g_eMMCDrv.BurstWriteLen_t)-4);

    u32_Err= eMMC_CMD25(eMMC_BURST_LEN_BLK_0, (U8*)&g_eMMCDrv.BurstWriteLen_t, 1);
    if(eMMC_ST_SUCCESS != u32_Err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: save fail: %Xh\n", u32_Err);
        goto LABEL_END;
    }

    // --------------------------
    LABEL_END:
    return u32_Err;

}
#endif


U32 eMMC_CMD25_MIU(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf, U16 u16_BlkCnt)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_fifoclk=0, u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;
    U32 u32_DataDMAAddr, u32_DataMapAddr;

    // -------------------------------
    // restore clock to max
    if(0 == eMMC_IF_TUNING_TTABLE())
        eMMC_FCIE_ErrHandler_RestoreClk();

    #if 0 //defined(eMMC_FEATURE_RELIABLE_WRITE) && eMMC_FEATURE_RELIABLE_WRITE
    eMMC_CMD23(u16_BlkCnt);
    #endif

    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    if(g_eMMCDrv.u32_DrvFlag & (DRV_FLAG_DDR_MODE|DRV_FLAG_SPEED_HS200) )
    {
        REG_FCIE_W(FCIE_TOGGLE_CNT, (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_SPEED_HS200) ? TOGGLE_CNT_512_CLK_W : TOGGLE_CNT_256_CLK_W);
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
    }
    eMMC_FCIE_ClearEvents();
    REG_FCIE_W(FCIE_JOB_BL_CNT, u16_BlkCnt);
    u32_DataMapAddr = eMMC_DMA_MAP_address((U32)pu8_DataBuf, eMMC_SECTOR_512BYTE*u16_BlkCnt,0);
    u32_DataDMAAddr = eMMC_translate_DMA_address_Ex(u32_DataMapAddr, eMMC_SECTOR_512BYTE*u16_BlkCnt);
    #if FICE_BYTE_MODE_ENABLE
    REG_FCIE_W(FCIE_SDIO_ADDR0, u32_DataDMAAddr & 0xFFFF);
    REG_FCIE_W(FCIE_SDIO_ADDR1, u32_DataDMAAddr >> 16);
    #else
    REG_FCIE_W(FCIE_MIU_DMA_15_0, (u32_DataDMAAddr >> MIU_BUS_WIDTH_BITS)& 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_26_16,(u32_DataDMAAddr >> MIU_BUS_WIDTH_BITS)>>16);
    #endif
    REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);
    u32_err = eMMC_FCIE_FifoClkRdy(BIT_DMA_DIR_W);
    if(IF_TEST_RETRY(32) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_fifoclk < eMMC_CMD_API_WAIT_FIFOCLK_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fifoclk++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: CMD25 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
            eMMC_FCIE_Init();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE*u16_BlkCnt,0);
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CMD25 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }
    REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 25, eMMC_R1_BYTE_CNT);
    if(IF_TEST_RETRY(23) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD25 retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
            eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE*u16_BlkCnt,0);
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD25 retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(IF_TEST_RETRY(13) || (u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR)) )
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE*u16_BlkCnt,0);
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD25;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    u32_err = eMMC_CheckR1Error();
    if(IF_TEST_RETRY(31) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_r1++;
            eMMC_debug(1, 1, "eMMC WARN: CMD25 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
            eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE*u16_BlkCnt,0);
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD25_CHK_R1;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD25 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // send data
    u32_err = eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        u32_err = eMMC_ST_ERR_CMD25_WAIT_D0H;
        goto LABEL_END;
    }

    REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W);
    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    REG_FCIE_W(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);
    #endif

    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
        BIT_CARD_DMA_END, TIME_WAIT_n_BLK_END*(1+(u16_BlkCnt>>9)));

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(IF_TEST_RETRY(43) || eMMC_ST_SUCCESS != u32_err || (u16_reg & (BIT_SD_W_FAIL|BIT_SD_W_CRC_ERR)))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE*u16_BlkCnt,0);
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD25;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:
    #if 1 //!(defined(eMMC_FEATURE_RELIABLE_WRITE) && eMMC_FEATURE_RELIABLE_WRITE)
    if(eMMC_ST_SUCCESS != eMMC_CMD12(g_eMMCDrv.u16_RCA))
        eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);
    #endif

    eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE*u16_BlkCnt,0);
    eMMC_FCIE_CLK_DIS();
    return u32_err;
}


U32 eMMC_CMD24(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf)
{
    #if defined(ENABLE_eMMC_RIU_MODE)&&ENABLE_eMMC_RIU_MODE
    return eMMC_CMD24_CIFD(u32_eMMCBlkAddr, pu8_DataBuf);
    #else
    return eMMC_CMD24_MIU(u32_eMMCBlkAddr, pu8_DataBuf);
    #endif
}

U32 eMMC_CMD24_MIU(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_fifoclk=0, u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;
    U32 u32_DataDMAAddr, u32_DataMapAddr;

    // -------------------------------
    if(0 == eMMC_IF_TUNING_TTABLE())
        eMMC_FCIE_ErrHandler_RestoreClk();

    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    if(g_eMMCDrv.u32_DrvFlag & (DRV_FLAG_DDR_MODE|DRV_FLAG_SPEED_HS200) ) {

        REG_FCIE_W(FCIE_TOGGLE_CNT, (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_SPEED_HS200) ? TOGGLE_CNT_512_CLK_W : TOGGLE_CNT_256_CLK_W);
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
    }
    eMMC_FCIE_ClearEvents();
    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
    u32_DataMapAddr = eMMC_DMA_MAP_address((U32)pu8_DataBuf, eMMC_SECTOR_512BYTE,0);
    u32_DataDMAAddr = eMMC_translate_DMA_address_Ex(u32_DataMapAddr, eMMC_SECTOR_512BYTE);
    #if FICE_BYTE_MODE_ENABLE
    REG_FCIE_W(FCIE_SDIO_ADDR0, u32_DataDMAAddr & 0xFFFF);
    REG_FCIE_W(FCIE_SDIO_ADDR1, u32_DataDMAAddr >> 16);
    #else
    REG_FCIE_W(FCIE_MIU_DMA_15_0, (u32_DataDMAAddr>>MIU_BUS_WIDTH_BITS)&0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_26_16,(u32_DataDMAAddr>>MIU_BUS_WIDTH_BITS)>>16);
    #endif
    REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);
    u32_err = eMMC_FCIE_FifoClkRdy(BIT_DMA_DIR_W);
    if(IF_TEST_RETRY(13) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_fifoclk < eMMC_CMD_API_WAIT_FIFOCLK_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fifoclk++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: CMD24 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
            eMMC_FCIE_Init();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE,0);
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CMD24 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }
    REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 24, eMMC_R1_BYTE_CNT);
    if(IF_TEST_RETRY(25) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD24 retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE,0);
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD24 retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(IF_TEST_RETRY(34) || (u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR)) )
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD24 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE,0);
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD25;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD24 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    u32_err = eMMC_CheckR1Error();
    if(IF_TEST_RETRY(31) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_r1++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD24 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE,0);
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD24_MIU_CHK_R1;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD24 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // send data
    u32_err = eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        u32_err = eMMC_ST_ERR_CMD24_MIU_WAIT_D0H;
        goto LABEL_END;
    }
    REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W);
    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    REG_FCIE_W(FCIE_MIE_INT_EN, BIT_CARD_DMA_END);
    #endif
    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
        BIT_CARD_DMA_END, TIME_WAIT_1_BLK_END);

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(IF_TEST_RETRY(11) || eMMC_ST_SUCCESS != u32_err || (u16_reg & (BIT_SD_W_FAIL|BIT_SD_W_CRC_ERR)))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Warn: CMD24 %sReg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
                (eMMC_ST_SUCCESS!=u32_err)?"TO ":"", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE,0);
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD24_MIU;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD24 %sReg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n",
            (eMMC_ST_SUCCESS!=u32_err)?"TO ":"", u16_reg, u32_err, u8_retry_fcie, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:
    eMMC_DMA_UNMAP_address(u32_DataMapAddr, eMMC_SECTOR_512BYTE,0);
    eMMC_FCIE_CLK_DIS();
    return u32_err;
}


U32 eMMC_CMD24_CIFD(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg, u16_i, *pu16_dat=(U16*)pu8_DataBuf;
    U8  u8_retry_r1=0, u8_retry_fcie=0, u8_retry_cmd=0;

    // -------------------------------
    #if 0
    if(0 == eMMC_IF_TUNING_TTABLE())
        eMMC_FCIE_ErrHandler_RestoreClk();
    #endif
    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:

    // fill CIFD with data
    REG_FCIE_W(FCIE_CIFD_WORD_CNT, 0);
    for(u16_i=0; u16_i<(FCIE_CIFD_BYTE_CNT>>1); u16_i++)
        REG_FCIE_W(FCIE_CIFD_ADDR(u16_i), pu16_dat[u16_i]);

    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = BIT_SD_DATA_CIFD | g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    if(g_eMMCDrv.u32_DrvFlag & (DRV_FLAG_DDR_MODE|DRV_FLAG_SPEED_HS200) )
    {
        sgu8_IfNeedRestorePadType = g_eMMCDrv.u8_PadType;
        u32_err = eMMC_FCIE_EnableSDRMode();
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableSDRMode fail: %Xh\n", u32_err);
            return u32_err;
        }
    }
    eMMC_FCIE_ClearEvents();

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 24, eMMC_R1_BYTE_CNT);
    if(IF_TEST_RETRY(23) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD24 CIFD retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD24 CIFD retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(IF_TEST_RETRY(19) || (u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR)) )
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD24 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD25;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD24 Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }
    u32_err = eMMC_CheckR1Error();
    if(IF_TEST_RETRY(17) || eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_r1++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD24 CIFD check R1 error: %Xh, Retry: %u, Arg: %Xh \n", u32_err, u8_retry_r1, u32_arg);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD24_CIFD_CHK_R1;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD24 CIFD check R1 error: %Xh, Retry: %u, Arg: %Xh \n", u32_err, u8_retry_r1, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // send data
    u32_err = eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        u32_err = eMMC_ST_ERR_CMD24_CIFD_WAIT_D0H;
        goto LABEL_END;
    }

    REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W);

    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
        BIT_SD_DATA_END, TIME_WAIT_1_BLK_END);

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(IF_TEST_RETRY(23) || eMMC_ST_SUCCESS != u32_err || (u16_reg & (BIT_SD_W_FAIL|BIT_SD_W_CRC_ERR)))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD24 TO Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD24_CIFD;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD24 CIFD Reg.12: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:
    if(u8_retry_fcie)
        eMMC_debug(0,1,"eMMC Info: retry ok\n");
    eMMC_FCIE_CLK_DIS();
    return u32_err;
}

U32 eMMC_GetR1(void)
{
    return (g_eMMCDrv.au8_Rsp[1]<<24) | (g_eMMCDrv.au8_Rsp[2]<<16)
        | (g_eMMCDrv.au8_Rsp[3]<<8) | g_eMMCDrv.au8_Rsp[4];
}

U32 eMMC_CheckR1Error(void)
{
    U32 u32_err = eMMC_ST_SUCCESS;

    eMMC_FCIE_GetCIFC(0, 3, (U16*)g_eMMCDrv.au8_Rsp);

    if(g_eMMCDrv.au8_Rsp[1] & (eMMC_ERR_R1_31_24>>24))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: eMMC_ST_ERR_R1_31_24 \n ");
        u32_err = eMMC_ST_ERR_R1_31_24;
        goto LABEL_CHECK_R1_END;
    }

    if(g_eMMCDrv.au8_Rsp[2] & (eMMC_ERR_R1_23_16>>16))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: eMMC_ST_ERR_R1_23_16 \n ");
        u32_err = eMMC_ST_ERR_R1_23_16;
        goto LABEL_CHECK_R1_END;
    }

    if(g_eMMCDrv.au8_Rsp[3] & (eMMC_ERR_R1_15_8>>8))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: eMMC_ST_ERR_R1_15_8 \n ");
        u32_err = eMMC_ST_ERR_R1_15_8;
        goto LABEL_CHECK_R1_END;
    }

    if(g_eMMCDrv.au8_Rsp[4] & (eMMC_ERR_R1_7_0>>0))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: eMMC_ST_ERR_R1_7_0 \n ");
        u32_err = eMMC_ST_ERR_R1_7_0;
        goto LABEL_CHECK_R1_END;
    }

LABEL_CHECK_R1_END:

    /*u8_cs = (eMMC_FCIE_CmdRspBufGet(3) & (eMMC_R1_CURRENT_STATE>>8))>>1;
    eMMC_debug(0,0,"card state: %d ", u8_cs);
    switch(u8_cs) {
        case 0:     eMMC_debug(0,0,"(idle)\n"); break;
        case 1:     eMMC_debug(0,0,"(ready)\n");    break;
        case 2:     eMMC_debug(0,0,"(ident)\n");    break;
        case 3:     eMMC_debug(0,0,"(stby)\n"); break;
        case 4:     eMMC_debug(0,0,"(tran)\n"); break;
        case 5:     eMMC_debug(0,0,"(data)\n"); break;
        case 6:     eMMC_debug(0,0,"(rcv)\n");  break;
        case 7:     eMMC_debug(0,0,"(prg)\n");  break;
        case 8:     eMMC_debug(0,0,"(dis)\n");  break;
        default:    eMMC_debug(0,0,"(?)\n");    break;
    }*/

    if(eMMC_ST_SUCCESS != u32_err)// && 0==eMMC_IF_TUNING_TTABLE())
    {
        eMMC_dump_mem(g_eMMCDrv.au8_Rsp, eMMC_R1_BYTE_CNT);
    }
    return u32_err;
}


// ====================================================
#if defined(eMMC_UPDATE_FIRMWARE) && (eMMC_UPDATE_FIRMWARE)
#define  UPFW_SEC_WIAT_CNT   0x1000000
#define  UPFW_SEC_BYTE_CNT   (128*1024)
static U32 eMMC_UpFW_Samsung_Wait(void)
{
    U32 u32_err, u32_cnt, u32_st;

    for(u32_cnt=0; u32_cnt<UPFW_SEC_WIAT_CNT; u32_cnt++)
    {
        eMMC_CMD13(g_eMMCDrv.u16_RCA);
        //eMMC_dump_mem(g_eMMCDrv.au8_Rsp, 6);
        u32_st = (g_eMMCDrv.au8_Rsp[1]<<24) | (g_eMMCDrv.au8_Rsp[2]<<16) |
            (g_eMMCDrv.au8_Rsp[3]<<8) | (g_eMMCDrv.au8_Rsp[4]<<0);
        if(0x900 == u32_st)
            break;
        else
            eMMC_debug(eMMC_DEBUG_LEVEL,0,"st: %Xh\n", u32_st);
        eMMC_hw_timer_delay(HW_TIMER_DELAY_1ms);
    }

    if(UPFW_SEC_WIAT_CNT == u32_cnt)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh\n", eMMC_ST_ERR_SEC_UPFW_TO);
        return eMMC_ST_ERR_SEC_UPFW_TO;
    }
    return eMMC_ST_SUCCESS;
}

U32 eMMC_UpFW_Samsung(U8 *pu8_FWBin)
{
    U16 u16_ctrl;
    U32 u32_err, u32_arg=0xEFAC60FC;

    // ---------------------------
    LABEL_CMD60:
    eMMC_debug(eMMC_DEBUG_LEVEL,0,"CMD60: %Xh\n", u32_arg);
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 60, eMMC_R1b_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD60 fail, %Xh \n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END_OF_UPFW;
    }
    u32_err = eMMC_UpFW_Samsung_Wait();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: timeout 1, Arg:%Xh \n", u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END_OF_UPFW;
    }
    if(0xEFAC60FC == u32_arg)
    {
        u32_arg=0xCBAD1160;
        goto LABEL_CMD60;
    }

    #if 1
    // ---------------------------
    eMMC_debug(eMMC_DEBUG_LEVEL,0,"erase ... \n");
    g_eMMCDrv.u32_eMMCFlag &= ~eMMC_FLAG_TRIM;
    u32_err = eMMC_EraseCMDSeq(0, 0);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: erase fail, %Xh \n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END_OF_UPFW;
    }
    u32_err = eMMC_UpFW_Samsung_Wait();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: timeout 2, %Xh \n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END_OF_UPFW;
    }
    #endif

    // ---------------------------
    eMMC_debug(eMMC_DEBUG_LEVEL,0,"write ... \n");
    u32_err = eMMC_CMD25_MIU(0, pu8_FWBin, UPFW_SEC_BYTE_CNT>>eMMC_SECTOR_512BYTE_BITS);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: write fail, %Xh \n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END_OF_UPFW;
    }
    u32_err = eMMC_UpFW_Samsung_Wait();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: timeout 3, %Xh \n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END_OF_UPFW;
    }

    // ---------------------------
    eMMC_debug(eMMC_DEBUG_LEVEL,0,"close ... \n");
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, 0, 28, eMMC_R1b_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD28 fail, %Xh \n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END_OF_UPFW;
    }
    u32_err = eMMC_UpFW_Samsung_Wait();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: timeout 4, %Xh \n", u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END_OF_UPFW;
    }

    LABEL_END_OF_UPFW:
    return u32_err;

}
#endif // eMMC_UPDATE_FIRMWARE

void HalFcie_SetFlag4Kernel2RuneMMC(void)
{
    REG_FCIE_CLRBIT(FCIE_REG16h, BIT_KERN_CHK_NAND_EMMC_MSK);
    REG_FCIE_SETBIT(FCIE_REG16h, BIT_KERN_EMMC);
}

bool HalFcie_CheckIfeMMCRun4Kernel(void)
{
    u16 u16_regval = 0;

    u16_regval = REG_FCIE(FCIE_REG16h);

    if( (u16_regval & BIT_KERN_CHK_NAND_EMMC) == BIT_KERN_CHK_NAND_EMMC )
    {
        return ( (u16_regval & BIT_KERN_EMMC) != BIT_KERN_EMMC ) ? 0 : 1;
    }
    return 0;
}

#endif // UNIFIED_eMMC_DRIVER

