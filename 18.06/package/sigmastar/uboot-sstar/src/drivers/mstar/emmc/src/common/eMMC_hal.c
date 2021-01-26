/*
* eMMC_hal.c- Sigmastar
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

#ifndef IP_FCIE_VERSION_5

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


#if eMMC_RSP_FROM_RAM
    #define eMMC_KEEP_RSP(pu8_OneRspBuf, u8_CmdIdx)         \
        if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_RSPFROMRAM_SAVE)\
            eMMC_KeepRsp(pu8_OneRspBuf, u8_CmdIdx)
#else
    #define eMMC_KEEP_RSP(pu8_OneRspBuf, u8_CmdIdx)  // NULL to save CPU a JMP/RET time
#endif


#define eMMC_FCIE_CLK_DIS()    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN)

#define eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT       30
#define eMMC_CMD_API_ERR_RETRY_CNT            30
#define eMMC_CMD_API_WAIT_FIFOCLK_RETRY_CNT   30

#define eMMC_CMD1_RETRY_CNT    0x1000
#define eMMC_CMD3_RETRY_CNT    0x5

//========================================================
// HAL APIs
//========================================================
U32 eMMC_FCIE_WaitEvents(U32 u32_RegAddr, U16 u16_Events, U32 u32_MicroSec)
{
    #if defined(ENABLE_eMMC_INTERRUPT_MODE)&&ENABLE_eMMC_INTERRUPT_MODE
    return eMMC_WaitCompleteIntr(u32_RegAddr, u16_Events, u32_MicroSec);
    #else
    return eMMC_FCIE_PollingEvents(u32_RegAddr, u16_Events, u32_MicroSec);
    #endif
}

U32 eMMC_FCIE_PollingEvents(U32 u32_RegAddr, U16 u16_Events, U32 u32_MicroSec)
{
    volatile U32 u32_i;
    volatile U16 u16_val;

    for(u32_i=0; u32_i<u32_MicroSec; u32_i++)
    {
        eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        REG_FCIE_R(u32_RegAddr, u16_val);
        if(u16_Events == (u16_val & u16_Events))
            break;
    }

    if(u32_i == u32_MicroSec)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: %u us, Reg.%04Xh: %04Xh, but wanted: %04Xh\n",
            u32_MicroSec, 
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
    if(0==eMMC_IF_DDRT_TUNING())
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

U32 eMMC_FCIE_ErrHandler_Retry(void)
{
    static U8 u8_IfToggleDataSync=0;

    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "eMMC_FCIE_ErrHandler_Retry\r\n");
        
    eMMC_FCIE_Init();

    if((++u8_IfToggleDataSync) & 1)
    {
        g_eMMCDrv.u16_Reg10_Mode ^= BIT_SD_DATA_SYNC;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,0,"eMMC retry by DATA_SYNC: %Xh, ori:%Xh\n", 
            g_eMMCDrv.u16_Reg10_Mode&BIT_SD_DATA_SYNC, REG_FCIE(FCIE_SD_MODE));

        return eMMC_ST_SUCCESS;
    }

    // -------------------------------------------------
    if(0 ==(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_INIT_DONE) )
        return eMMC_ST_SUCCESS;

    // -------------------------------------------------
    if(0 == (g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE))
    {
        if(gau16_FCIEClkSel[eMMC_FCIE_VALID_CLK_CNT-1] == g_eMMCDrv.u16_ClkRegVal)
            return eMMC_ST_ERR_NO_SLOWER_CLK;
    }
    else
    {
        if(g_eMMCDrv.DDRTable.u8_SetCnt-1 == g_eMMCDrv.DDRTable.u8_CurSetIdx)
            return eMMC_ST_ERR_NO_SLOWER_CLK;
    }
    
    if(0 == (g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE))
    {
        U8 u8_i;

        for(u8_i=0; u8_i<eMMC_FCIE_VALID_CLK_CNT; u8_i++)
            if(gau16_FCIEClkSel[u8_i] == g_eMMCDrv.u16_ClkRegVal)
                break;
        
        eMMC_clock_setting(gau16_FCIEClkSel[u8_i+1]); 
        eMMC_debug(eMMC_DEBUG_LEVEL_WARNING,0,
            "eMMC Warn: slow SDR clk to %Xh\n", g_eMMCDrv.u16_ClkRegVal);
    }
    else
    {
        eMMC_FCIE_ApplyDDRTSet(g_eMMCDrv.DDRTable.u8_CurSetIdx+1);       
        eMMC_debug(eMMC_DEBUG_LEVEL_WARNING,0,
            "eMMC Warn: slow DDR clk to %Xh\n", g_eMMCDrv.u16_ClkRegVal);
    }
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "eMMC_FCIE_ErrHandler_Retry end\r\n");
    return eMMC_ST_SUCCESS;
}


void eMMC_FCIE_ErrHandler_RestoreClk(void)
{
    if(FCIE_DEFAULT_CLK == g_eMMCDrv.u16_ClkRegVal) // no slow clk
        return; 

    if(0 == (g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE))
        eMMC_clock_setting(gau16_FCIEClkSel[0]);
    else
        eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);
}


void eMMC_FCIE_ErrHandler_ReInit(void)
{
    U32 u32_err;
    U16 u16_RestoreClk=g_eMMCDrv.u16_ClkRegVal;
    U16 u16_Reg10 = g_eMMCDrv.u16_Reg10_Mode;
    U32 u32_DrvFlag = g_eMMCDrv.u32_DrvFlag;
//  U8  u8_eMMC_Identify_retry;
   
    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC_FCIE_ErrHandler_ReInit\r\n");
   

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

    u32_err = eMMC_SetBusWidth(8, 0);
    if(u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SetBusWidth fail, %Xh\n", u32_err);
        goto LABEL_REINIT_END;
    }

    u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);
    if(u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: SetBusSpeed fail, %Xh\n", u32_err);
        goto LABEL_REINIT_END;
    }

    eMMC_clock_setting(u16_RestoreClk);
    g_eMMCDrv.u32_DrvFlag = u32_DrvFlag;
    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
    {
        u32_err = eMMC_FCIE_EnableDDRMode_Ex();
        if(u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableDDRMode_Ex fail, %Xh\n", u32_err);
            goto LABEL_REINIT_END;
        }
    }

    g_eMMCDrv.u16_Reg10_Mode = u16_Reg10;
    REG_FCIE_W(FCIE_SD_MODE, g_eMMCDrv.u16_Reg10_Mode);

    LABEL_REINIT_END:
    if(u32_err)
        eMMC_die("\n");
        
    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC_FCIE_ErrHandler_ReInit end\r\n");
}


#define FCIE_WAIT_RST_DONE_D1  0x5555
#define FCIE_WAIT_RST_DONE_D2  0xAAAA
#define FCIE_WAIT_RST_DONE_CNT 3

void eMMC_FCIE_CheckResetDone(void)
{
    volatile U16 au16_tmp[FCIE_WAIT_RST_DONE_CNT], u16_i;
    
    while(1){
        for(u16_i=0; u16_i<FCIE_WAIT_RST_DONE_CNT; u16_i++){
            REG_FCIE_W(FCIE_CIFC_ADDR(u16_i), FCIE_WAIT_RST_DONE_D1);
            REG_FCIE_R(FCIE_CIFC_ADDR(u16_i), au16_tmp[u16_i]);
        }
        for(u16_i=0; u16_i<FCIE_WAIT_RST_DONE_CNT; u16_i++)
            if(FCIE_WAIT_RST_DONE_D1 != au16_tmp[u16_i])
                break;
        if(FCIE_WAIT_RST_DONE_CNT == u16_i)
            break;
    }
    
    while(1){
        for(u16_i=0; u16_i<FCIE_WAIT_RST_DONE_CNT; u16_i++){
            REG_FCIE_W(FCIE_CIFC_ADDR(u16_i), FCIE_WAIT_RST_DONE_D2);
            REG_FCIE_R(FCIE_CIFC_ADDR(u16_i), au16_tmp[u16_i]);
        }       
        for(u16_i=0; u16_i<FCIE_WAIT_RST_DONE_CNT; u16_i++)
            if(FCIE_WAIT_RST_DONE_D2 != au16_tmp[u16_i])
                break;
        if(FCIE_WAIT_RST_DONE_CNT == u16_i)
            break;
    }
}

U32 eMMC_FCIE_Reset(void)   
{
    U16 au16_tmp[FCIE_WAIT_RST_DONE_CNT];
    U32 u32_i, u32_j, u32_err = eMMC_ST_SUCCESS;
    U16 u16_clk = g_eMMCDrv.u16_ClkRegVal;

    eMMC_clock_setting(gau16_FCIEClkSel[0]); // speed up FCIE reset done
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
    
    eMMC_PlatformResetPre();

    // ------------------------------------------
    #if eMMC_TEST_IN_DESIGN
    {
        
        printf("xxx");
        
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
    //REG_FCIE_W(FCIE_BOOT_CONFIG, 0); // set by eMMC_pads_switch
    REG_FCIE_W(FCIE_PWR_SAVE_MODE, 0);
    eMMC_FCIE_ClearEvents();
    eMMC_PlatformResetPost();

    return eMMC_ST_SUCCESS; // ok
}


#if 1// defined(IF_DETECT_eMMC_DDR_TIMING) && IF_DETECT_eMMC_DDR_TIMING
void eMMC_DumpDDRTTable(void)
{
    U16 u16_i;
    
    eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n    eMMC DDR Timing Table: Cnt:%u CurIdx:%u \n",
        g_eMMCDrv.DDRTable.u8_SetCnt, g_eMMCDrv.DDRTable.u8_CurSetIdx);
    
    for(u16_i=0; u16_i<g_eMMCDrv.DDRTable.u8_SetCnt; u16_i++)
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"      Set:%u: clk:%Xh, DQS:%Xh, Cell:%Xh \n",
            u16_i, g_eMMCDrv.DDRTable.Set[u16_i].u16_Clk,
            g_eMMCDrv.DDRTable.Set[u16_i].Param.u8_DQS,
            g_eMMCDrv.DDRTable.Set[u16_i].Param.u8_Cell);
}

U32 eMMC_LoadDDRTTable(void)
{
    U32 u32_err;
    U32 u32_ChkSum;

    printf("eMMC_LoadDDRTTable\r\n");
    u32_err = eMMC_CMD18(eMMC_DDRTABLE_BLK_0, gau8_eMMC_SectorBuf, 1);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: load 1 DDRT Table fail, %Xh\n", u32_err);
        
        u32_err = eMMC_CMD18(eMMC_DDRTABLE_BLK_1, gau8_eMMC_SectorBuf, 1);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: load 2 DDRT Tables fail, %Xh\n", u32_err);
            goto LABEL_END_OF_NO_TABLE;
        }
    }

    memcpy((U8*)&g_eMMCDrv.DDRTable, gau8_eMMC_SectorBuf, sizeof(g_eMMCDrv.DDRTable));

    u32_ChkSum = eMMC_ChkSum((U8*)&g_eMMCDrv.DDRTable, sizeof(g_eMMCDrv.DDRTable)-4);
    if(u32_ChkSum != g_eMMCDrv.DDRTable.u32_ChkSum)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: ChkSum error, no DDRT Table \n");
        u32_err = eMMC_ST_ERR_DDRT_CHKSUM;
        goto LABEL_END_OF_NO_TABLE;
    }   

    if(0 == u32_ChkSum && 
        g_eMMCDrv.DDRTable.Set[0].u16_Clk == g_eMMCDrv.DDRTable.Set[1].u16_Clk)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: no DDRT Table \n");
        u32_err = eMMC_ST_ERR_DDRT_NONA;
        goto LABEL_END_OF_NO_TABLE;
    }

    #if 0
    for(u16_i=0; u16_i<eMMC_FCIE_DDRT_SET_CNT; u16_i++)
        eMMC_debug(eMMC_DEBUG_LEVEL_HIGH,0,"\n Set:%u: clk:%Xh, DQS:%Xh, Cell:%Xh \n",
                u16_i, g_eMMCDrv.DDRTable.Set[u16_i].u16_Clk,
                g_eMMCDrv.DDRTable.Set[u16_i].Param.u8_DQS,
                g_eMMCDrv.DDRTable.Set[u16_i].Param.u8_Cell);
    #endif
    return eMMC_ST_SUCCESS;

    LABEL_END_OF_NO_TABLE:
    g_eMMCDrv.DDRTable.u8_SetCnt = 0;
    return u32_err;
    
}


U32 eMMC_FCIE_EnableDDRMode_Ex(void)
{
    U32 u32_err;
    
    u32_err = eMMC_SetBusWidth(8, 1);
    if(eMMC_ST_SUCCESS != u32_err){
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
            "eMMC Err: set device 8b DDR fail: %Xh\n", u32_err);
        return u32_err;
    }
    
    eMMC_pads_switch(FCIE_eMMC_DDR);
    //g_eMMCDrv.u16_Reg10_Mode |= BIT_SD_DATA_SYNC;
    g_eMMCDrv.u16_Reg10_Mode &= ~BIT_SD_DATA_SYNC;
    g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_DDR_MODE;

    return u32_err;
}

U32 eMMC_FCIE_EnableDDRMode(void)
{
    U32 u32_err;

    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
        return eMMC_ST_SUCCESS;
        
    u32_err = eMMC_LoadDDRTTable();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: no DDR Timing Table, %Xh\n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_FCIE_EnableDDRMode_Ex();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableDDRMode_Ex fail, %Xh\n", u32_err);
        eMMC_die("");  
        return u32_err;
    }

    eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);
    
    return eMMC_ST_SUCCESS;
}


void eMMC_FCIE_SetDDRTimingReg(U8 u8_DQS, U8 u8_DelaySel)
{
    REG_FCIE_CLRBIT(FCIE_SM_STS, BIT_DQS_MODE_MASK|BIT_DQS_DELAY_CELL_MASK);

    REG_FCIE_SETBIT(FCIE_SM_STS, u8_DQS<<BIT_DQS_MDOE_SHIFT); 
    REG_FCIE_SETBIT(FCIE_SM_STS, u8_DelaySel<<BIT_DQS_DELAY_CELL_SHIFT); 
}


void eMMC_FCIE_ApplyDDRTSet(U8 u8_DDRTIdx)
{
    eMMC_clock_setting(g_eMMCDrv.DDRTable.Set[u8_DDRTIdx].u16_Clk);
    eMMC_FCIE_SetDDRTimingReg(
        g_eMMCDrv.DDRTable.Set[u8_DDRTIdx].Param.u8_DQS, 
        g_eMMCDrv.DDRTable.Set[u8_DDRTIdx].Param.u8_Cell);

    g_eMMCDrv.DDRTable.u8_CurSetIdx = u8_DDRTIdx;
}
#endif

U32 eMMC_FCIE_EnableSDRMode(void)
{
    U32 u32_err;

    u32_err = eMMC_SetBusWidth(8, 0);
    if(eMMC_ST_SUCCESS != u32_err){
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: set device 8b SDR fail: %Xh\n", u32_err);
        return u32_err;
    }
    
    eMMC_pads_switch(FCIE_eMMC_SDR);
    g_eMMCDrv.u16_Reg10_Mode = BIT_SD_DEFAULT_MODE_REG;
    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_SPEED_MASK)
        g_eMMCDrv.u16_Reg10_Mode |= BIT_SD_DATA_SYNC;
    else
        g_eMMCDrv.u16_Reg10_Mode &= ~BIT_SD_DATA_SYNC;
    
    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_DDR_MODE;

    return eMMC_ST_SUCCESS;
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
    }
    REG_FCIE_W1C(FCIE_SD_STATUS, BIT_SD_FCIE_ERR_FLAGS); // W1C
    REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_MIU_REQUEST_RST);
}


U32 eMMC_FCIE_WaitD0High_Ex(U32 u32_Timeout)
{
   volatile U32 u32_cnt;
    U16 u16_read0=0, u16_read1=0;
    
   for(u32_cnt=0; u32_cnt < u32_Timeout; u32_cnt++)
   {
       REG_FCIE_R(FCIE_SD_STATUS, u16_read0);
       eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
       REG_FCIE_R(FCIE_SD_STATUS, u16_read1);

       if((u16_read0&BIT_SD_CARD_D0_ST) && (u16_read1&BIT_SD_CARD_D0_ST))
           break;
   }

   return u32_cnt;
}

U32 eMMC_FCIE_WaitD0High(U32 u32_Timeout)
{
    volatile U32 u32_cnt;

    u32_cnt = eMMC_FCIE_WaitD0High_Ex(u32_Timeout);

    if(u32_Timeout == u32_cnt)
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

U32 eMMC_FCIE_SendCmdttt
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
    
    printf("eMMC_FCIE_SendCmdttt: u16_Ctrl : %X\r\n", u16_Ctrl);
    while(1);
    
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

//===================================================
#if eMMC_RSP_FROM_RAM
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
        eMMC_debug(0,1,"eMMC Info: no rsp, %u %u \n", u8_CmdIdx, u16_idx);
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

    LABEL_IDENTIFY_CMD0:
    if(eMMC_ST_SUCCESS != eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: WaitD0High TO\n");
        eMMC_FCIE_ErrHandler_Stop();
    }
    eMMC_RST_L();  eMMC_hw_timer_delay(HW_TIMER_DELAY_1ms); //eMMC_hw_timer_sleep(1);   
    eMMC_RST_H();  eMMC_hw_timer_delay(HW_TIMER_DELAY_1ms); //eMMC_hw_timer_sleep(1);
    
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

        eMMC_hw_timer_delay(HW_TIMER_DELAY_1ms);

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
    u32_arg = BIT30 | (BIT23|BIT22|BIT21|BIT20|BIT19|BIT18|BIT17|BIT16|BIT15);
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
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
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
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
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
                
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
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

    printf("eMMC_CSD_Config\r\n");

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
        printf("a.eMMC_CSD_Config.. g_eMMCDrv.u32_SEC_COUNT = 0\r\n");
        g_eMMCDrv.u32_SEC_COUNT = 0;
    }
    else
    {
        g_eMMCDrv.u8_C_SIZE_MULT = ((g_eMMCDrv.au8_CSD[10]&3)<<1)+
            ((g_eMMCDrv.au8_CSD[11]&0x80)>>7);  
        
        g_eMMCDrv.u32_SEC_COUNT = 
            ((g_eMMCDrv.u16_C_SIZE+1)*
            (1<<(g_eMMCDrv.u8_C_SIZE_MULT+2))*
            ((1<<g_eMMCDrv.u8_R_BL_LEN)>>9)) - 8; //-8: //Toshiba CMD18 access the last block report out of range error   
            
        printf("b.eMMC_CSD_Config.. g_eMMCDrv.u32_SEC_COUNT = %X\r\n", g_eMMCDrv.u32_SEC_COUNT);
    }
    // ------------------------------
    //ttt
    g_eMMCDrv.u8_ERASE_GRP_SIZE = (g_eMMCDrv.au8_CSD[10]&0x7C)>>2;
    
    printf("g_eMMCDrv.u8_ERASE_GRP_SIZE : %X\r\n", g_eMMCDrv.u8_ERASE_GRP_SIZE);
    
    g_eMMCDrv.u8_ERASE_GRP_MULT = ((g_eMMCDrv.au8_CSD[10]&0x03)<<3)+
        ((g_eMMCDrv.au8_CSD[11]&0xE0)>>5);
    printf("g_eMMCDrv.u8_ERASE_GRP_MULT : %X\r\n", g_eMMCDrv.u8_ERASE_GRP_MULT);    
        
    g_eMMCDrv.u32_EraseUnitSize = (g_eMMCDrv.u8_ERASE_GRP_SIZE+1)*
        (g_eMMCDrv.u8_ERASE_GRP_MULT+1);
        
    printf("g_eMMCDrv.u32_EraseUnitSize : %X\r\n", g_eMMCDrv.u32_EraseUnitSize);    
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
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
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
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
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

    printf("eMMC_ExtCSD_Config.. g_eMMCDrv.u32_SEC_COUNT = %X\r\n", g_eMMCDrv.u32_SEC_COUNT);

    //-------------------------------
    if(0 == g_eMMCDrv.u32_BOOT_SEC_COUNT)
         g_eMMCDrv.u32_BOOT_SEC_COUNT = gau8_eMMC_SectorBuf[226] * 128 * 2;

    //--------------------------------
    g_eMMCDrv.u8_BUS_WIDTH = gau8_eMMC_SectorBuf[183];
    switch(g_eMMCDrv.u8_BUS_WIDTH)
    {
        case 0:  g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_1;  break;
        case 1:  g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_4;  break;
        case 2:  g_eMMCDrv.u8_BUS_WIDTH = BIT_SD_DATA_WIDTH_8;  break;
        default: eMMC_debug(0,1,"eMMC Err: eMMC BUS_WIDTH not support \n");
            while(1);
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
            eMMC_debug(0,1,"eMMC Warn: not support Reliable-W\n");
            g_eMMCDrv.u16_ReliableWBlkCnt = 0; // can not support Reliable Write
        }
        #endif
    }

    //--------------------------------
    g_eMMCDrv.u8_ErasedMemContent = gau8_eMMC_SectorBuf[181];

    //--------------------------------
    printf("gau8_eMMC_SectorBuf[224] : %X\r\n", gau8_eMMC_SectorBuf[224]);
    if (gau8_eMMC_SectorBuf[224] != 0)
    {
            g_eMMCDrv.u32_EraseUnitSize = gau8_eMMC_SectorBuf[224] << 10;
    }
    printf ("ExtCSD : g_eMMCDrv.u32_EraseUnitSize : %X\r\n", g_eMMCDrv.u32_EraseUnitSize);
    
    return eMMC_ST_SUCCESS;
}


U32 eMMC_CMD8(U8 *pu8_DataBuf)
{
    #if 0//defined(ENABLE_eMMC_RIU_MODE)&&ENABLE_eMMC_RIU_MODE
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
    U32 u32_dma_addr;

    // -------------------------------
    #if 0==IF_IP_VERIFY
    if(g_eMMCDrv.u32_DrvFlag&DRV_FLAG_DDR_MODE){
        if(0 == eMMC_IF_DDRT_TUNING()) 
        {
            if(eMMC_DDRT_SET_MAX != g_eMMCDrv.DDRTable.u8_CurSetIdx)
                eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);
        }
    }
    else{ // SDR Mode
        //if(FCIE_DEFAULT_CLK != g_eMMCDrv.u16_ClkRegVal)
        //  eMMC_clock_setting(FCIE_DEFAULT_CLK);
    }
    #endif
    
    // -------------------------------
    // send cmd
    u32_arg =  0;
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;

    LABEL_SEND_CMD:
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
    {
        REG_FCIE_W(FCIE_TOGGLE_CNT, BITS_8_R_TOGGLE_CNT);
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
        REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
        eMMC_hw_timer_delay(TIME_WAIT_FCIE_RST_TOGGLE_CNT); // Brian needs 2T
        REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);      
    }
    eMMC_FCIE_ClearEvents();
    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
    
    printf("pu8_DataBuf: %X\r\n", (U32)pu8_DataBuf);
    
  u32_dma_addr = eMMC_translate_DMA_address_Ex((U32)pu8_DataBuf, eMMC_SECTOR_512BYTE, READ_FROM_eMMC); 
        
  #if FICE_BYTE_MODE_ENABLE
    REG_FCIE_W(FCIE_SDIO_ADDR0, u32_dma_addr & 0xFFFF); 
    REG_FCIE_W(FCIE_SDIO_ADDR1, u32_dma_addr >> 16);
    #else
    
    xxx
    REG_FCIE_W(FCIE_MIU_DMA_15_0, (u32_dma_addr>>MIU_BUS_WIDTH_BITS)&0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_26_16,(u32_dma_addr>>MIU_BUS_WIDTH_BITS)>>16);
    #endif
    REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);
    u32_err = eMMC_FCIE_FifoClkRdy(0);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_fifoclk < eMMC_CMD_API_WAIT_FIFOCLK_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_fifoclk++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: CMD8 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
            eMMC_FCIE_Init();
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
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD8 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
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
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD8 Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Retry();
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
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_r1++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                "eMMC WARN: CMD8 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD8_MIU;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD8 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:
    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);

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
    //u16_mode |= BIT_SD_DATA_CIFD;
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;

    LABEL_SEND_CMD:
    u16_mode = BIT_SD_DATA_CIFD | g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    eMMC_FCIE_ClearEvents();

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 8, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
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
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
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
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
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


U32 eMMC_SetBusSpeed(U8 u8_BusSpeed)
{
    U32 u32_err;

    u32_err = eMMC_ModifyExtCSD(eMMC_ExtCSD_WByte, 185, u8_BusSpeed);
    if(eMMC_ST_SUCCESS != u32_err)
        return u32_err;

    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_SPEED_MASK;
    switch(u8_BusSpeed)
    {
        case eMMC_SPEED_HIGH: 
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_SPEED_HIGH;
            g_eMMCDrv.u16_Reg10_Mode |= BIT_SD_DATA_SYNC;
            //eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC set bus HIGH speed\n");
            break;
        case eMMC_SPEED_HS200: 
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_SPEED_HS200;
            g_eMMCDrv.u16_Reg10_Mode |= BIT_SD_DATA_SYNC;
            //eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC set bus HS200 speed\n");
            break;
        default:
            g_eMMCDrv.u16_Reg10_Mode &= ~BIT_SD_DATA_SYNC;
            //eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC set bus LOW speed\n");
    }

    return u32_err;
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
        u8_value |= BIT2;
    if(BIT2 == u8_value)
        return eMMC_ST_ERR_PARAMETER;

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

    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC_ModifyExtCSD %X\r\n", u32_arg);

    u32_err = eMMC_CMD6(u32_arg);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC: %Xh \n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_CMD13(g_eMMCDrv.u16_RCA);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC: %Xh \n", u32_err);
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

    printf("eMMC_CMD6\r\n");


    u32_arg = u32_Arg;
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
    g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_RSP_WAIT_D0H;

    LABEL_SEND_CMD:
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 6, eMMC_R1b_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD6 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
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
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD6 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
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
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING()){
                    u8_retry_r1++;  
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                        "eMMC WARN: CMD6 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
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

    printf("eMMC_EraseCMDSeq : %X, %X\r\n", u32_eMMCBlkAddr_start, u32_eMMCBlkAddr_end);


    u32_err = eMMC_CMD35_CMD36(u32_eMMCBlkAddr_start, 35);
    if(eMMC_ST_SUCCESS != u32_err)
        return u32_err;

    u32_err = eMMC_CMD35_CMD36(u32_eMMCBlkAddr_end, 36);
    if(eMMC_ST_SUCCESS != u32_err)
        return u32_err;

    u32_err = eMMC_CMD38();
    if(eMMC_ST_SUCCESS != u32_err)
        return u32_err;

    return u32_err;
}

U32 eMMC_CMD35_CMD36(U32 u32_eMMCBlkAddr, U8 u8_CmdIdx)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;
    U8  u8_retry_r1=0, u8_retry_fcie=0, u8_retry_cmd=0;

    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
    
    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, u8_CmdIdx, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
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

        if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR))
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
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
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING()){
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
        }       
    }

    eMMC_FCIE_CLK_DIS();
    return u32_err;
}

U32 eMMC_CMD38(void)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;
    U8  u8_retry_r1=0, u8_retry_fcie=0, u8_retry_cmd=0;
    
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;
    if(g_eMMCDrv.u32_eMMCFlag & eMMC_FLAG_TRIM)
        u32_arg =  0x1;
    else
        u32_arg =  0x0;

    g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_RSP_WAIT_D0H;

    LABEL_SEND_CMD:
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 38, eMMC_R1b_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD38 retry:%u, %Xh \n", 
                u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }

        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD38 retry:%u, %Xh \n", 
            u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();    
    }
    else
    {   
        // check status
        REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

        if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR))
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                    "eMMC WARN: CMD38 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD3_CMD7;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                "eMMC Err: CMD38 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {   // CMD3 ok, do things here
            u32_err = eMMC_CheckR1Error();
            if(eMMC_ST_SUCCESS != u32_err)
            {
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING()){
                    u8_retry_r1++;  
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                        "eMMC WARN: CMD38 check R1 error: %Xh, retry: %u\n", 
                        u32_err, u8_retry_r1);
                    eMMC_FCIE_ErrHandler_Retry();
                    goto LABEL_SEND_CMD;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                    "eMMC Err: CMD38 check R1 error: %Xh, retry: %u\n", 
                    u32_err, u8_retry_r1);
                eMMC_FCIE_ErrHandler_Stop();
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
    U8  u8_retry_r1=0, u8_retry_fcie=0, u8_retry_cmd=0;
    
    u32_arg = (u16_RCA<<16) |
        ((g_eMMCDrv.u32_eMMCFlag & eMMC_FLAG_HPI_CMD13)?1:0);
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 13, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD13 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
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
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                    "eMMC WARN: CMD13 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
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
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING()){
                    u8_retry_r1++;  
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                        "eMMC WARN: CMD13 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
                    eMMC_FCIE_ErrHandler_Retry();
                    goto LABEL_SEND_CMD;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                    "eMMC Err: CMD13 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
                eMMC_FCIE_ErrHandler_Stop();
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
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
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
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
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
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING()){
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

U32 eMMC_CMD17_MIU(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_fifoclk=0, u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;
    U32 u32_dma_addr;

    // -------------------------------
    #if 0==IF_IP_VERIFY
    if(g_eMMCDrv.u32_DrvFlag&DRV_FLAG_DDR_MODE){
        if(0 == eMMC_IF_DDRT_TUNING()) 
        {
            if(eMMC_DDRT_SET_MAX != g_eMMCDrv.DDRTable.u8_CurSetIdx)
                eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);
        }
    }
    else{ // SDR Mode
        if(FCIE_DEFAULT_CLK != g_eMMCDrv.u16_ClkRegVal)
            eMMC_clock_setting(FCIE_DEFAULT_CLK);
    }
    #endif
    
    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;

    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
    {
        REG_FCIE_W(FCIE_TOGGLE_CNT, BITS_8_R_TOGGLE_CNT);
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
        REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
        eMMC_hw_timer_delay(TIME_WAIT_FCIE_RST_TOGGLE_CNT); // Brian needs 2T
        REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
    }
    eMMC_FCIE_ClearEvents();
    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
    u32_dma_addr = eMMC_translate_DMA_address_Ex((U32)pu8_DataBuf, eMMC_SECTOR_512BYTE, READ_FROM_eMMC);
    #if FICE_BYTE_MODE_ENABLE
    REG_FCIE_W(FCIE_SDIO_ADDR0, u32_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_SDIO_ADDR1, u32_dma_addr >> 16);
    #else
    REG_FCIE_W(FCIE_MIU_DMA_15_0, (u32_dma_addr>>MIU_BUS_WIDTH_BITS)&0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_26_16,(u32_dma_addr>>MIU_BUS_WIDTH_BITS)>>16);
    #endif  
    REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);
    u32_err = eMMC_FCIE_FifoClkRdy(0);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_fifoclk < eMMC_CMD_API_WAIT_FIFOCLK_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_fifoclk++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: CMD17 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
            eMMC_FCIE_Init();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CMD17 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }
    REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 17, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD17 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD17 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
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
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & BIT_SD_FCIE_ERR_FLAGS))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD17 Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD17_MIU;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD17 Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_r1++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD17 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        else
        {   u32_err = eMMC_ST_ERR_CMD17_MIU;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD17 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }

    LABEL_END:
    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);

    eMMC_FCIE_CLK_DIS();
    return u32_err;
}

U32 eMMC_CMD17_CIFD(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_r1=0, u8_retry_fcie=0, u8_retry_cmd=0;

    REG_FCIE_W(FCIE_CIFD_WORD_CNT, 0);

    #if 0==IF_IP_VERIFY
    if(FCIE_DEFAULT_CLK != g_eMMCDrv.u16_ClkRegVal)
        eMMC_clock_setting(FCIE_DEFAULT_CLK);
    #endif
    
    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;

    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = BIT_SD_DATA_CIFD | g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    eMMC_FCIE_ClearEvents();
    
    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 17, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT  && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD17 CIFD retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD17 CIFD retry:%u, %Xh \n", u8_retry_cmd, u32_err);
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
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                "eMMC WARN: CMD17 CIFD Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD17_CIFD;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
            "eMMC Err: CMD17 CIFD Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;     
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_r1++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                "eMMC WARN: CMD17 CIFD check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }

        u32_err = eMMC_ST_ERR_CMD17_CIFD;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD17 CIFD check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
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
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
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

        if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR))
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD12 check reg.12h error: %Xh, %Xh, retry: %u\n", 
                    u16_reg, u32_err, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_ReInit();
                //eMMC_FCIE_ErrHandler_Retry();
                //goto LABEL_SEND_CMD;
                return u32_err;
            }

            u32_err = eMMC_ST_ERR_CMD12;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD12 check reg.12h error: %Xh, %Xh, retry: %u\n", 
                u16_reg, u32_err, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {   u32_err = eMMC_CheckR1Error();
            if(eMMC_ST_SUCCESS != u32_err)
            {
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
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
    #else
    return eMMC_CMD18_MIU(u32_eMMCBlkAddr, pu8_DataBuf, u16_BlkCnt);
    #endif
}

U32 eMMC_CMD18_MIU(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf, U16 u16_BlkCnt)
{
    U32 u32_err, u32_err_12, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_fifoclk=0, u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;
    U32 u32_dma_addr;

    // -------------------------------
    #if 0==IF_IP_VERIFY
    if(g_eMMCDrv.u32_DrvFlag&DRV_FLAG_DDR_MODE){
        
        printf("eMMC_CMD18_MIU DRV_FLAG_DDR_MODE\r\n");
        if(0 == eMMC_IF_DDRT_TUNING()) 
        {
            if(eMMC_DDRT_SET_MAX != g_eMMCDrv.DDRTable.u8_CurSetIdx)
                eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);
        }
        else
        {
            printf("eMMC_IF_DDRT_TUNING\r\n");
        }
    }
    else{ // SDR Mode
        if(FCIE_DEFAULT_CLK != g_eMMCDrv.u16_ClkRegVal)
            eMMC_clock_setting(FCIE_DEFAULT_CLK);
    }
    #endif
    
    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;

    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
    {
        REG_FCIE_W(FCIE_TOGGLE_CNT, BITS_8_R_TOGGLE_CNT);
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
        REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
        eMMC_hw_timer_delay(TIME_WAIT_FCIE_RST_TOGGLE_CNT); // Brian needs 2T
        REG_FCIE_CLRBIT(FCIE_MACRO_REDNT, BIT_TOGGLE_CNT_RST);
    }
    eMMC_FCIE_ClearEvents();
    REG_FCIE_W(FCIE_JOB_BL_CNT, u16_BlkCnt);
    u32_dma_addr = eMMC_translate_DMA_address_Ex((U32)pu8_DataBuf, eMMC_SECTOR_512BYTE*u16_BlkCnt, READ_FROM_eMMC);
    #if FICE_BYTE_MODE_ENABLE
    REG_FCIE_W(FCIE_SDIO_ADDR0, u32_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_SDIO_ADDR1, u32_dma_addr >> 16);
    #else
    REG_FCIE_W(FCIE_MIU_DMA_15_0, (u32_dma_addr >> MIU_BUS_WIDTH_BITS)& 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_26_16,(u32_dma_addr >> MIU_BUS_WIDTH_BITS)>>16);
    #endif  
    REG_FCIE_CLRBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);
    u32_err = eMMC_FCIE_FifoClkRdy(0);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_fifoclk < eMMC_CMD_API_WAIT_FIFOCLK_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_fifoclk++;     
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: CMD18 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
            eMMC_FCIE_Init();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CMD18 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }
    REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);

    u32_err = eMMC_FCIE_SendCmd(u16_mode, u16_ctrl, u32_arg, 18, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD18 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
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
        BIT_MIU_LAST_DONE|BIT_CARD_DMA_END, TIME_WAIT_n_BLK_END*(1+(u16_BlkCnt>>9)));

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & BIT_SD_FCIE_ERR_FLAGS))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                "eMMC WARN: CMD18 Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);

            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            
                        //while(1);
            
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
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_r1++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                "eMMC WARN: CMD18 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);

            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD18;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
            "eMMC Err: CMD18 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:
    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
    
    u32_err_12 = eMMC_CMD12(g_eMMCDrv.u16_RCA);
    eMMC_FCIE_CLK_DIS();
    return u32_err ? u32_err : u32_err_12;
}

// enable Reliable Write
U32 eMMC_CMD23(U16 u16_BlkCnt) 
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;
    U8  u8_retry_r1=0, u8_retry_fcie=0, u8_retry_cmd=0;

    u32_arg = u16_BlkCnt&0xFFFF; // don't set BIT24
    #if eMMC_FEATURE_RELIABLE_WRITE
    u32_arg |= BIT31; // don't set BIT24
    #endif
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:
    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 23, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD23 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
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
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD23 Reg.12: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
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
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING()){
                    u8_retry_r1++;
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                        "eMMC WARN: CMD23 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
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
    #if defined(ENABLE_eMMC_RIU_MODE)&&ENABLE_eMMC_RIU_MODE
    U16 u16_cnt;
    U32 u32_err;
    
    for(u16_cnt=0; u16_cnt<u16_BlkCnt; u16_cnt++)
    {
        u32_err = eMMC_CMD24_CIFD(u32_eMMCBlkAddr+u16_cnt, pu8_DataBuf+(u16_cnt<<9));
        if(eMMC_ST_SUCCESS != u32_err)
            return u32_err;
    }
    #else
    return eMMC_CMD25_MIU(u32_eMMCBlkAddr, pu8_DataBuf, u16_BlkCnt);
    #endif
}

U32 eMMC_CMD25_MIU(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf, U16 u16_BlkCnt)
{
    U32 u32_err, u32_err_12=eMMC_ST_SUCCESS, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_fifoclk=0, u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;
    U32 u32_dma_addr;
    #if eMMC_FEATURE_RELIABLE_WRITE
    U16 u16_BlkCntTotal = u16_BlkCnt;
    #endif

    // -------------------------------
    #if 0==IF_IP_VERIFY
    if(g_eMMCDrv.u32_DrvFlag&DRV_FLAG_DDR_MODE){
        if(0 == eMMC_IF_DDRT_TUNING()) 
        {
            if(eMMC_DDRT_SET_MAX != g_eMMCDrv.DDRTable.u8_CurSetIdx)
                eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);
        }
    }
    else{ // SDR Mode
        if(FCIE_DEFAULT_CLK != g_eMMCDrv.u16_ClkRegVal)
            eMMC_clock_setting(FCIE_DEFAULT_CLK);
    }
    #endif
    
    // -------------------------------
    // send cmd 
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    
    // -------------------------------
    #if eMMC_FEATURE_RELIABLE_WRITE
    LABEL_RELIABLE_W_START:
    if(g_eMMCDrv.u16_ReliableWBlkCnt)
    {
        u16_BlkCnt = u16_BlkCntTotal > g_eMMCDrv.u16_ReliableWBlkCnt ? 
            g_eMMCDrv.u16_ReliableWBlkCnt : u16_BlkCntTotal;
        u32_err = eMMC_CMD23(u16_BlkCnt);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CMD23 fail: %Xh\n", u32_err);
            return u32_err;
        }   
    }
    else
        u16_BlkCnt = u16_BlkCntTotal;
    #endif

    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
    {
        REG_FCIE_W(FCIE_TOGGLE_CNT, BITS_8_W_TOGGLE_CNT);
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
    }
    eMMC_FCIE_ClearEvents();
    REG_FCIE_W(FCIE_JOB_BL_CNT, u16_BlkCnt);
    u32_dma_addr = eMMC_translate_DMA_address_Ex((U32)pu8_DataBuf, eMMC_SECTOR_512BYTE*u16_BlkCnt, WRITE_TO_eMMC);
    #if FICE_BYTE_MODE_ENABLE
    REG_FCIE_W(FCIE_SDIO_ADDR0, u32_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_SDIO_ADDR1, u32_dma_addr >> 16);
    #else
    REG_FCIE_W(FCIE_MIU_DMA_15_0, (u32_dma_addr >> MIU_BUS_WIDTH_BITS)& 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_26_16,(u32_dma_addr >> MIU_BUS_WIDTH_BITS)>>16);
    #endif  
    REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);
    u32_err = eMMC_FCIE_FifoClkRdy(BIT_DMA_DIR_W);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_fifoclk < eMMC_CMD_API_WAIT_FIFOCLK_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_fifoclk++;     
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: CMD25 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
            eMMC_FCIE_Init();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CMD25 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }
    REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 25, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD25 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD25 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_r1++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                "eMMC WARN: CMD25 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);

            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD25_CHK_R1;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD25 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
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
        BIT_CARD_DMA_END, TIME_WAIT_n_BLK_END*(1+(u16_BlkCnt>>7))); 

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);        
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & (BIT_SD_W_FAIL|BIT_SD_W_CRC_ERR)))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                "eMMC WARN: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);

            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD25;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
            "eMMC Err: CMD25 Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    #if eMMC_FEATURE_RELIABLE_WRITE
    if(g_eMMCDrv.u16_ReliableWBlkCnt)
    {
        u32_eMMCBlkAddr += u16_BlkCnt;
        pu8_DataBuf += u16_BlkCnt * eMMC_SECTOR_512BYTE;
        u16_BlkCntTotal -= u16_BlkCnt;
        if(u16_BlkCntTotal)
            goto LABEL_RELIABLE_W_START;
    }
    #endif
    
    LABEL_END:
    #if eMMC_FEATURE_RELIABLE_WRITE
    if(eMMC_ST_SUCCESS != u32_err || 0==g_eMMCDrv.u16_ReliableWBlkCnt)
    #endif
        u32_err_12 = eMMC_CMD12(g_eMMCDrv.u16_RCA);

    eMMC_FCIE_CLK_DIS();
    return u32_err ? u32_err : u32_err_12;
}


U32 eMMC_CMD24(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf)
{
    #if defined(ENABLE_eMMC_RIU_MODE)&&ENABLE_eMMC_RIU_MODE
    return eMMC_CMD24_CIFD(u32_eMMCBlkAddr, pu8_DataBuf);
    #else
        #if eMMC_FEATURE_RELIABLE_WRITE
    return eMMC_CMD25(u32_eMMCBlkAddr, pu8_DataBuf, 1);
        #else
    return eMMC_CMD24_MIU(u32_eMMCBlkAddr, pu8_DataBuf);
        #endif
    #endif
}

U32 eMMC_CMD24_MIU(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_fifoclk=0, u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;
    U32 u32_dma_addr;

    // -------------------------------
    #if 0==IF_IP_VERIFY
    if(g_eMMCDrv.u32_DrvFlag&DRV_FLAG_DDR_MODE){
        if(0 == eMMC_IF_DDRT_TUNING()) 
        {
            if(eMMC_DDRT_SET_MAX != g_eMMCDrv.DDRTable.u8_CurSetIdx)
                eMMC_FCIE_ApplyDDRTSet(eMMC_DDRT_SET_MAX);
        }
    }
    else{ // SDR Mode
        if(FCIE_DEFAULT_CLK != g_eMMCDrv.u16_ClkRegVal)
            eMMC_clock_setting(FCIE_DEFAULT_CLK);
    }
    #endif
    
    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_DDR_MODE)
    {
        REG_FCIE_W(FCIE_TOGGLE_CNT, BITS_8_W_TOGGLE_CNT);
        REG_FCIE_SETBIT(FCIE_MACRO_REDNT, BIT_MACRO_DIR);
    }
    eMMC_FCIE_ClearEvents();
    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
    u32_dma_addr = eMMC_translate_DMA_address_Ex((U32)pu8_DataBuf, eMMC_SECTOR_512BYTE, WRITE_TO_eMMC);
    #if FICE_BYTE_MODE_ENABLE
    REG_FCIE_W(FCIE_SDIO_ADDR0, u32_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_SDIO_ADDR1, u32_dma_addr >> 16);
    #else
    REG_FCIE_W(FCIE_MIU_DMA_15_0, (u32_dma_addr>>MIU_BUS_WIDTH_BITS)&0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_26_16,(u32_dma_addr>>MIU_BUS_WIDTH_BITS)>>16);
    #endif  
    REG_FCIE_SETBIT(FCIE_MMA_PRI_REG, BIT_DMA_DIR_W);
    u32_err = eMMC_FCIE_FifoClkRdy(BIT_DMA_DIR_W);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_fifoclk < eMMC_CMD_API_WAIT_FIFOCLK_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_fifoclk++;         
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: CMD24 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
            eMMC_FCIE_Init();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: CMD24 wait FIFOClk retry: %u \n", u8_retry_fifoclk);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }
    REG_FCIE_SETBIT(FCIE_PATH_CTRL, BIT_MMA_EN);

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 24, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD24 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD24 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(eMMC_ST_SUCCESS != u32_err)
    {       
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_r1++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD24 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD24_MIU_CHK_R1;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD24 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
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
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & (BIT_SD_W_FAIL|BIT_SD_W_CRC_ERR)))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD24 Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD24_MIU;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD24 Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:
    eMMC_FCIE_CLK_DIS();
    return u32_err;
}


U32 eMMC_CMD24_CIFD(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg, u16_i, *pu16_dat=(U16*)pu8_DataBuf;
    U8  u8_retry_r1=0, u8_retry_fcie=0, u8_retry_cmd=0;

    // -------------------------------
    // fill CIFD with data
    REG_FCIE_W(FCIE_CIFD_WORD_CNT, 0);

    #if 0==IF_IP_VERIFY
    if(FCIE_DEFAULT_CLK != g_eMMCDrv.u16_ClkRegVal)
        eMMC_clock_setting(FCIE_DEFAULT_CLK);
    #endif
    
    for(u16_i=0; u16_i<(FCIE_CIFD_BYTE_CNT>>1); u16_i++)
        REG_FCIE_W(FCIE_CIFD_ADDR(u16_i), pu16_dat[u16_i]);
    
    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = BIT_SD_DATA_CIFD | g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    eMMC_FCIE_ClearEvents();    
    
    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 24, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD24 CIFD retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD24 CIFD retry:%u, %Xh \n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(eMMC_ST_SUCCESS != u32_err)
    {           
        if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_r1++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
                "eMMC WARN: CMD24 CIFD check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD24_CIFD_CHK_R1;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, 
            "eMMC Err: CMD24 CIFD check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
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
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & (BIT_SD_W_FAIL|BIT_SD_W_CRC_ERR)))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_DDRT_TUNING())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD24 CIFD Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD24_CIFD;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD24 CIFD Reg.12: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:
    eMMC_FCIE_CLK_DIS();
    return u32_err;
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
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_dump_mem(g_eMMCDrv.au8_Rsp, eMMC_R1_BYTE_CNT);
        
        if(24==g_eMMCDrv.au8_Rsp[0] || 25==g_eMMCDrv.au8_Rsp[0] || 
           18==g_eMMCDrv.au8_Rsp[0] || 12==g_eMMCDrv.au8_Rsp[0])
            eMMC_FCIE_ErrHandler_ReInit();
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

    #if eMMC_FEATURE_RELIABLE_WRITE
    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: please disable Reliable Write \n");
    while(1);
    #endif

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
#endif  

#endif // #ifndef IP_FCIE_VERSION_5

