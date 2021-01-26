/*
* eMMC_hal_v5.c- Sigmastar
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

#ifdef IP_FCIE_VERSION_5

#ifdef STATIC_RELIABLE_TEST
#include "drvWDT.h"
#include "MsCommon.h"
#include "drvPM.h"
#endif

U16 const crc16_table[256] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

static inline U16 crc16_byte(U16 crc, const U8 data)
{
    return (crc >> 8) ^ crc16_table[(crc ^ data) & 0xff];
}

U16 crc16(U16 crc, U8 const *buffer, unsigned int len)
{
    while (len--)
        crc = crc16_byte(crc, *buffer++);
    return crc;
}

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

#define eMMC_FCIE_CLK_DIS()    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_CLK_EN)

#define eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT       0
#define eMMC_CMD_API_ERR_RETRY_CNT            0
#define eMMC_CMD_API_WAIT_FIFOCLK_RETRY_CNT   0

#define eMMC_CMD1_RETRY_CNT    0x8000
#define eMMC_CMD3_RETRY_CNT    0x5

//========================================================
// HAL APIs
//========================================================
U32 eMMC_FCIE_WaitEvents(U32 u32_RegAddr, U16 u16_Events, U32 u32_MicroSec)
{
    //eMMC_debug(0, 0, "\033[32m%s\033[m\r\n", __FUNCTION__);
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
            "eMMC Err: %u us, Reg.%04Xh: %04Xh, but wanted: %04Xh\r\n",
            u32_MicroSec,
            (u32_RegAddr-FCIE_MIE_EVENT)>>REG_OFFSET_SHIFT_BITS,
            u16_val, u16_Events);

        return eMMC_ST_ERR_TIMEOUT_WAIT_REG0;
    }

    return eMMC_ST_SUCCESS;
}

U32 eMMC_FCIE_WPollingEvents(U32 u32_RegAddr, U16 u16_Events)
{
    volatile U16 u16_val;
    while(1)
    {
        REG_FCIE_R(u32_RegAddr, u16_val);

        if(u16_Events == (u16_val & u16_Events))
            break;
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

    REG_FCIE_SETBIT(FCIE_EMMC_DEBUG_BUS1, BIT10|BIT8);
    REG_FCIE_CLRBIT(FCIE_EMMC_DEBUG_BUS1, BIT11|BIT9);

    for(u16_i=1; u16_i<8; u16_i++)
    {
        REG_FCIE_CLRBIT(FCIE_TEST_MODE, BIT_DEBUG_MODE_MASK);
        REG_FCIE_SETBIT(FCIE_TEST_MODE, u16_i<<BIT_DEBUG_MODE_SHIFT);

        REG_FCIE_R(FCIE_TEST_MODE, u16_reg);
        sgau16_eMMCDebugBus[u16_idx] = u16_reg;
        u16_idx++;
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "FCIE_TEST_MODE: %Xh\r\n", u16_reg);

        REG_FCIE_R(FCIE_EMMC_DEBUG_BUS0, u16_reg);
        sgau16_eMMCDebugBus[u16_idx] = u16_reg;
        u16_idx++;
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "FCIE_EMMC_DEBUG_BUS Rx38: %Xh\r\n", u16_reg);

        REG_FCIE_R(FCIE_EMMC_DEBUG_BUS1, u16_reg);
        sgau16_eMMCDebugBus[u16_idx] = u16_reg;
        u16_idx++;
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "FCIE_EMMC_DEBUG_BUS Rx39: %Xh\r\n", u16_reg);

    }

    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\n");

}

static U16 sgau16_eMMCDebugReg[100];
void eMMC_FCIE_DumpRegisters(void)
{
    volatile U16 u16_reg;
    U16 u16_i;

    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\r\n\r\nFCIE Reg:x%x(x%x)", FCIE_REG_BASE_ADDR, (FCIE_REG_BASE_ADDR&0x00FFFFFF)>>9);

    for(u16_i=0 ; u16_i<0x60; u16_i++)
    {
        if(0 == u16_i%8)
            eMMC_debug(eMMC_DEBUG_LEVEL,0,"\r\n%02Xh:| ", u16_i);

        REG_FCIE_R(GET_REG_ADDR(FCIE_REG_BASE_ADDR, u16_i), u16_reg);
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", u16_reg);
    }
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\r\n");

    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "FCIE JobCnt: \r\n");
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "JobCnt: %Xh \r\n", REG_FCIE(FCIE_JOB_BL_CNT));

    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\r\nFCIE CMDFIFO:");
    eMMC_FCIE_GetCMDFIFO(0, 0x09, (U16*)sgau16_eMMCDebugReg);
    for(u16_i=0 ; u16_i<0x20; u16_i++)
    {
        if(0 == u16_i%8)
            eMMC_debug(eMMC_DEBUG_LEVEL,0,"\r\n%02Xh:| ", u16_i);

        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", sgau16_eMMCDebugReg[u16_i]);
    }

    #if 0
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\n\nEMMCPLL Reg:");

    for(u16_i=0 ; u16_i<0x80; u16_i++)
    {
        if(0 == u16_i%8)
            eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n%02Xh:| ", u16_i);

        REG_FCIE_R(GET_REG_ADDR(EMMC_PLL_BASE, u16_i), u16_reg);
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", u16_reg);
    }
    #endif

    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\r\n\r\nPAD_CHIPTOP Reg:x%x(BK:x%x)", PAD_CHIPTOP_BASE, (PAD_CHIPTOP_BASE&0x00FFFFFF)>>9);

    for(u16_i=0 ; u16_i<0x80; u16_i++)
    {
        if(0 == u16_i%8)
            eMMC_debug(eMMC_DEBUG_LEVEL,0,"\r\n%02Xh:| ", u16_i);

        REG_FCIE_R(GET_REG_ADDR(PAD_CHIPTOP_BASE, u16_i), u16_reg);
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", u16_reg);
    }

    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "\r\n\r\nCLK_GEN Reg:x%x(BK:x%x)", CLKGEN0_BASE, (CLKGEN0_BASE&0x00FFFFFF)>>9);

    for(u16_i=0 ; u16_i<0x80; u16_i++)
    {
        if(0 == u16_i%8)
            eMMC_debug(eMMC_DEBUG_LEVEL,0,"\r\n%02Xh:| ", u16_i);

        REG_FCIE_R(GET_REG_ADDR(CLKGEN0_BASE, u16_i), u16_reg);
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "%04Xh ", u16_reg);
    }

    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "\r\n");
}

void eMMC_FCIE_ErrHandler_Stop(void)
{
    if(0==eMMC_IF_TUNING_TTABLE())
    {
        eMMC_DumpDriverStatus();
        eMMC_DumpPadClk();
        eMMC_FCIE_DumpRegisters();
        eMMC_FCIE_DumpDebugBus();
        //eMMC_Dump_eMMCStatus();
        eMMC_die("\n");
    }
    else {
        eMMC_FCIE_Init();
    }
}

U32 eMMC_FCIE_ErrHandler_Retry(void)
{
    eMMC_debug(0,0,"%s\r\n", __FUNCTION__);

    eMMC_FCIE_Init();

    return eMMC_ST_SUCCESS;
}


void eMMC_FCIE_ErrHandler_RestoreClk(void)
{
    if(FCIE_DEFAULT_CLK == g_eMMCDrv.u16_ClkRegVal) // no slow clk
        return;

    eMMC_clock_setting(FCIE_DEFAULT_CLK);

}


void eMMC_FCIE_ErrHandler_ReInit(void)
{
    U32 u32_err;
    U16 u16_RestoreClk=g_eMMCDrv.u16_ClkRegVal;
    U16 u16_Reg10 = g_eMMCDrv.u16_Reg10_Mode;
    U32 u32_DrvFlag = g_eMMCDrv.u32_DrvFlag;
    U8  u8_PadType = g_eMMCDrv.u8_PadType;

    //eMMC_debug(0,0,"%s\r\n", __FUNCTION__);

    g_eMMCDrv.u8_make_sts_err = 0;

    u32_err = eMMC_FCIE_Init();
    if(u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: FCIE_Init fail, %Xh\r\n", u32_err);
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

    u32_err = eMMC_SetBusWidth(EMMC_WORKING_BUS_CFG, 0);
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
    g_eMMCDrv.u8_PadType = u8_PadType;
    eMMC_pads_switch(g_eMMCDrv.u8_PadType);
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
    volatile U16 au16_tmp[FCIE_WAIT_RST_DONE_CNT] = {0};
    volatile U32 u32_i, u32_j, u32_err;

    for(u32_i=0; u32_i<TIME_WAIT_FCIE_RESET; u32_i++)
    {
        for(u32_j=0; u32_j<FCIE_WAIT_RST_DONE_CNT; u32_j++)
            REG_FCIE_W(FCIE_CMDFIFO_ADDR(u32_j), FCIE_WAIT_RST_DONE_D1+u32_j);

        for(u32_j=0; u32_j<FCIE_WAIT_RST_DONE_CNT; u32_j++)
            REG_FCIE_R(FCIE_CMDFIFO_ADDR(u32_j), au16_tmp[u32_j]);

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
    U16 u16Cnt;
    U16 u16_clk = g_eMMCDrv.u16_ClkRegVal;

    //eMMC_debug(1, 1, "eMMC_FCIE_Reset()\n");

    REG_FCIE_CLRBIT(FCIE_SD_CTRL, BIT_JOB_START); // clear for safe

    eMMC_clock_setting(FCIE_DEFO_SPEED_CLK); // instead reset clock source from UPLL to XTAL

    REG_FCIE_CLRBIT(FCIE_RST, BIT_FCIE_SOFT_RST_n); /* active low */

    u16Cnt = 0;
    while(1)
    {
        if((REG_FCIE(FCIE_RST)&BIT_RST_STS_MASK)==BIT_RST_STS_MASK) // reset success
            break;

        eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

        if(u16Cnt++>=1000)
            eMMC_debug(1, 0, "eMMC Err: FCIE reset fail!\n");
    }

    REG_FCIE_SETBIT(FCIE_RST, BIT_FCIE_SOFT_RST_n);
    u16Cnt = 0;
    while(1)
    {
        if((REG_FCIE(FCIE_RST)&BIT_RST_STS_MASK)==0) // reset success
            break;

        eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);

        if(u16Cnt++>=1000)
            eMMC_debug(1, 0, "eMMC Err: FCIE reset fail2!\n");
    }

    eMMC_clock_setting(u16_clk);
    return 0;

    #if 0
    U16 u16Reg = 0, u16Cnt;
    U32 u32_err = eMMC_ST_SUCCESS;
    U16 u16_clk = g_eMMCDrv.u16_ClkRegVal;
    
    REG_FCIE_CLRBIT(FCIE_SD_CTRL, BIT_JOB_START); // clear for safe
    eMMC_clock_setting(FCIE_DEFAULT_CLK); // speed up FCIE reset done
    REG_FCIE_W(FCIE_MIE_FUNC_CTL, BIT_MIE_FUNC_ENABLE); //I5:BIT_SD_EN

    // FCIE reset - set
    REG_FCIE_CLRBIT(FCIE_RST, BIT_FCIE_SOFT_RST_n); /* active low */

    // FCIE reset - wait
    u16Cnt=0;
    do
    {
        eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
        if(0x1000 == u16Cnt++)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: FCIE Reset fail: %Xh, Reg %Xh \n", eMMC_ST_ERR_FCIE_NO_CLK, u16Reg);
            return eMMC_ST_ERR_FCIE_NO_CLK;
        }

        REG_FCIE_R(FCIE_RST, u16Reg);

    }while (BIT_RST_STS_MASK  != (u16Reg  & BIT_RST_STS_MASK));

    //[FIXME] is there any method to check that reseting FCIE is done?

    // FCIE reset - clear
    REG_FCIE_SETBIT(FCIE_RST, BIT_FCIE_SOFT_RST_n);
    // FCIE reset - check

    eMMC_clock_setting(u16_clk);
    return u32_err;
    #endif
}


U32 eMMC_FCIE_Init(void)
{
    U32 u32_err;

    eMMC_PlatformResetPre();

    // ------------------------------------------
    #if 0 //eMMC_TEST_IN_DESIGN
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
    REG_FCIE_W(FCIE_MIE_FUNC_CTL, BIT_MIE_FUNC_ENABLE); //I5:BIT_SD_EN
    // all cmd are 5 bytes (excluding CRC)
    REG_FCIE_CLRBIT(FCIE_CMD_RSP_SIZE, BIT_CMD_SIZE_MASK);
    REG_FCIE_SETBIT(FCIE_CMD_RSP_SIZE, (eMMC_CMD_BYTE_CNT)<< BIT_CMD_SIZE_SHIFT);
    REG_FCIE_W(FCIE_SD_CTRL, 0);
    REG_FCIE_W(FCIE_SD_MODE, g_eMMCDrv.u16_Reg10_Mode);
    // default sector size: 0x200
    REG_FCIE_W(FCIE_BLK_SIZE,  eMMC_SECTOR_512BYTE);

    REG_FCIE_W(FCIE_RSP_SHIFT_CNT, 0);
    REG_FCIE_W(FCIE_RX_SHIFT_CNT, 0);
    //REG_FCIE_CLRBIT(FCIE_RX_SHIFT_CNT, BIT_RSTOP_SHIFT_SEL|BIT_RSTOP_SHIFT_TUNE_MASK);

    REG_FCIE_W(FCIE_WR_SBIT_TIMER, 0);
    REG_FCIE_W(FCIE_RD_SBIT_TIMER, 0);

    eMMC_FCIE_ClearEvents();
    eMMC_PlatformResetPost();

    return eMMC_ST_SUCCESS; // ok
}


U32 eMMC_FCIE_EnableSDRMode(void)
{
    U32 u32_err;
    u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);
    if(eMMC_ST_SUCCESS != u32_err){
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: set device 8b SDR fail: %Xh\r\n", u32_err);
        return u32_err;
    }

    u32_err = eMMC_SetBusWidth(EMMC_WORKING_BUS_CFG, 0);
    if(eMMC_ST_SUCCESS != u32_err){
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: set device 8b SDR fail: %Xh\r\n", u32_err);
        return u32_err;
    }

    eMMC_pads_switch(FCIE_eMMC_SDR);
    g_eMMCDrv.u16_Reg10_Mode = BIT_SD_DEFAULT_MODE_REG;
    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_DDR_MODE;

    return eMMC_ST_SUCCESS;
}

void eMMC_FCIE_ClearEvents(void)
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
    REG_FCIE_W1C(FCIE_SD_STATUS, BIT_SD_FCIE_ERR_FLAGS); // W1C
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

        if((u16_read0&BIT_SD_CARD_BUSY) ==0 && (u16_read1&BIT_SD_CARD_BUSY) ==0)
            break;

        if(u32_cnt > 500 && u32_us-u32_cnt > 1000)
        {
            eMMC_hw_timer_sleep(1);
            u32_cnt += 1000-2;
        }
    }

    return u32_cnt;
}

U32 eMMC_FCIE_WaitD0High(U32 u32_us)
{
    volatile U32 u32_cnt;

	REG_FCIE_SETBIT(FCIE_SD_MODE, BIT_CLK_EN);
	u32_cnt = eMMC_FCIE_WaitD0High_Ex(u32_us);

    if(u32_us == u32_cnt)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: wait D0 H timeout %u us\n", u32_cnt);
        return eMMC_ST_ERR_TIMEOUT_WAITD0HIGH;
    }

    return eMMC_ST_SUCCESS;
}

U32 eMMC_FCIE_SendCmd_Rboot
(
    U16 u16_Mode, U16 u16_Ctrl, U32 u32_Arg, U8 u8_CmdIdx, U8 u8_RspByteCnt
)
{
    //U32 u32_err;
    //U32 u32_Timeout = TIME_WAIT_DAT0_HIGH;
    U16 au16_tmp[3];

    REG_FCIE_CLRBIT(FCIE_CMD_RSP_SIZE, BIT_RSP_SIZE_MASK);
    REG_FCIE_SETBIT(FCIE_CMD_RSP_SIZE, u8_RspByteCnt & BIT_RSP_SIZE_MASK);

    REG_FCIE_W(FCIE_SD_MODE, u16_Mode);
    //   set cmd
    //   CMDFIFO(0) = 39:32 | 47:40
    //   CMDFIFO(1) = 23:16 | 31:24
    //   CMDFIFO(2) = (CIFC(2) & 0xFF00) | 15:8, ignore (CRC7 | end_bit).

    //check command FIFO for filled command value
    REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_CHK_CMD);

    while(1)
    {
        REG_FCIE_W(FCIE_CMDFIFO_ADDR(0),((u32_Arg>>24)<<8) | (0x40|u8_CmdIdx));
        REG_FCIE_W(FCIE_CMDFIFO_ADDR(1), (u32_Arg&0xFF00) | ((u32_Arg>>16)&0xFF));
        REG_FCIE_W(FCIE_CMDFIFO_ADDR(2),  u32_Arg&0xFF);

        REG_FCIE_R(FCIE_CMDFIFO_ADDR(0),au16_tmp[0]);
        REG_FCIE_R(FCIE_CMDFIFO_ADDR(1),au16_tmp[1]);
        REG_FCIE_R(FCIE_CMDFIFO_ADDR(2),au16_tmp[2]);

        if( au16_tmp[0] == (((u32_Arg>>24)<<8) | (0x40|u8_CmdIdx))&&
            au16_tmp[1] == ((u32_Arg&0xFF00) | ((u32_Arg>>16)&0xFF))&&
            au16_tmp[2] == (u32_Arg&0xFF))
            break;
    }

    //clear command check for reading response value
    REG_FCIE_CLRBIT(FCIE_SD_CTRL, BIT_CHK_CMD);

    //REG_WRITE_UINT16(FCIE_SD_CTRL, u16_Ctrl |BIT_JOB_START);

    return eMMC_ST_SUCCESS;

}

U32 eMMC_FCIE_SendCmd
(
    U16 u16_Mode, U16 u16_Ctrl, U32 u32_Arg, U8 u8_CmdIdx, U8 u8_RspByteCnt
)
{
    U32 u32_err, u32_Timeout = TIME_WAIT_DAT0_HIGH;

    if(38 == u8_CmdIdx)
        u32_Timeout = TIME_WAIT_ERASE_DAT0_HIGH;


    REG_FCIE_CLRBIT(FCIE_CMD_RSP_SIZE, BIT_RSP_SIZE_MASK);

    if( g_eMMCDrv.u8_make_sts_err == FCIE_MAKE_CMD_RSP_ERR )
    {
        REG_FCIE_SETBIT(FCIE_CMD_RSP_SIZE, (u8_RspByteCnt+1) & BIT_RSP_SIZE_MASK);
    }
    else
    {
        REG_FCIE_SETBIT(FCIE_CMD_RSP_SIZE, u8_RspByteCnt & BIT_RSP_SIZE_MASK);
    }

    REG_FCIE_W(FCIE_SD_MODE, u16_Mode);
    //   set cmd
    //   CMDFIFO(0) = 39:32 | 47:40
    //   CMDFIFO(1) = 23:16 | 31:24
    //   CMDFIFO(2) = (CIFC(2) & 0xFF00) | 15:8, ignore (CRC7 | end_bit).

    //check command FIFO for filled command value
    //REG_FCIE_SETBIT(FCIE_SD_CTRL, BIT_CHK_CMD);
    //while(1)
    //{
//      eMMC_debug(0, 1, "FIFO 0 =  0x%4X\n",  ((u32_Arg>>24)<<8) | (0x40|u8_CmdIdx));
//      eMMC_debug(0, 1, "FIFO 1 =  0x%4X\n",  (u32_Arg&0xFF00) | ((u32_Arg>>16)&0xFF));
//      eMMC_debug(0, 1, "FIFO 2 =  0x%4X\n", u32_Arg&0xFF);
        REG_FCIE_W(FCIE_CMDFIFO_ADDR(0),((u32_Arg>>24)<<8) | (0x40|u8_CmdIdx));
        REG_FCIE_W(FCIE_CMDFIFO_ADDR(1), (u32_Arg&0xFF00) | ((u32_Arg>>16)&0xFF));
        REG_FCIE_W(FCIE_CMDFIFO_ADDR(2),  u32_Arg&0xFF);

        //REG_FCIE_R(FCIE_CMDFIFO_ADDR(0),au16_tmp[0]);
        //REG_FCIE_R(FCIE_CMDFIFO_ADDR(1),au16_tmp[1]);
        //REG_FCIE_R(FCIE_CMDFIFO_ADDR(2),au16_tmp[2]);

        //if(au16_tmp[0] == (((u32_Arg>>24)<<8) | (0x40|u8_CmdIdx))&&
        //  au16_tmp[1] == ((u32_Arg&0xFF00) | ((u32_Arg>>16)&0xFF))&&
        //  au16_tmp[2] == (u32_Arg&0xFF))
        //  break;
    //}

    //clear command check for reading response value
    //REG_FCIE_CLRBIT(FCIE_SD_CTRL, BIT_CHK_CMD);

    if(12 != u8_CmdIdx)
    {
        u32_err = eMMC_FCIE_WaitD0High(u32_Timeout);
        if(eMMC_ST_SUCCESS != u32_err)
            goto LABEL_SEND_CMD_ERROR;
    }

    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    REG_FCIE_W(FCIE_MIE_INT_EN, BIT_SD_CMD_END);
    #endif

    REG_FCIE_W(FCIE_SD_CTRL, u16_Ctrl);
    REG_FCIE_W(FCIE_SD_CTRL, u16_Ctrl |BIT_JOB_START);

    #if 1//defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    // wait event
    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_SD_CMD_END, HW_TIMER_DELAY_1s);
    #endif

    if(g_eMMCDrv.u32_DrvFlag & DRV_FLAG_RSP_WAIT_D0H){
        u32_err = eMMC_FCIE_WaitD0High(u32_Timeout);
        if(eMMC_ST_SUCCESS != u32_err)
            goto LABEL_SEND_CMD_ERROR;
    }

    LABEL_SEND_CMD_ERROR:
    return u32_err;

}

void eMMC_FCIE_GetCMDFIFO(U16 u16_WordPos, U16 u16_WordCnt, U16 *pu16_Buf)
{
    U16 u16_i;

    for(u16_i=0; u16_i<u16_WordCnt; u16_i++)
        REG_FCIE_R(FCIE_CMDFIFO_ADDR(u16_i), pu16_Buf[u16_i]);
}

U32 eMMC_WaitCIFD_Event(U16 u16_WaitEvent, U32  u32_MicroSec)
{
    volatile U32 u32_Count;
    volatile U16 u16_Reg;

    for (u32_Count=0; u32_Count < u32_MicroSec; u32_Count++)
    {
        REG_FCIE_R(NC_CIFD_EVENT, u16_Reg);
        if ((u16_Reg & u16_WaitEvent) == u16_WaitEvent)
            break;

        eMMC_hw_timer_delay(HW_TIMER_DELAY_1us);
    }
    if (u32_Count >= u32_MicroSec)
    {
        REG_FCIE_R(NC_CIFD_EVENT, u16_Reg);
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "CIFD Event Timout %X\n", u16_Reg);
    }
    return u32_Count;
}

U32 eMMC_WaitSetCIFD(U8 * pu8_DataBuf, U32 u32_ByteCnt)
{
    U16 u16_i, *pu16_Data = (U16*)pu8_DataBuf;

    if(u32_ByteCnt > FCIE_CIFD_BYTE_CNT)
    {
        return eMMC_ST_ERR_INVALID_PARAM;
    }

    for(u16_i=0; u16_i<(u32_ByteCnt>>1); u16_i++)
        REG_FCIE_W(NC_RBUF_CIFD_ADDR(u16_i), pu16_Data[u16_i]);

    REG_FCIE_SETBIT(NC_CIFD_EVENT, BIT_RBUF_FULL_TRI);

    if (eMMC_WaitCIFD_Event(BIT_RBUF_EMPTY, HW_TIMER_DELAY_500ms)== (HW_TIMER_DELAY_500ms ))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Error: CIFD timeout 0, ErrCode:%Xh\r\n", eMMC_ST_ERR_TIMEOUT_WAITCIFDEVENT);
        return eMMC_ST_ERR_TIMEOUT_WAITCIFDEVENT; // timeout
    }

    REG_FCIE_SETBIT(NC_CIFD_EVENT, BIT_RBUF_EMPTY);

    return eMMC_ST_SUCCESS;
}


U32 eMMC_WaitGetCIFD(U8 * pu8_DataBuf, U32 u32_ByteCnt)
{
    U16 u16_i, *pu16_Data = (U16*)pu8_DataBuf;

    if(u32_ByteCnt > FCIE_CIFD_BYTE_CNT)
    {
        return eMMC_ST_ERR_INVALID_PARAM;
    }
    if (eMMC_WaitCIFD_Event(BIT_WBUF_FULL, HW_TIMER_DELAY_500ms)== (HW_TIMER_DELAY_500ms ))
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "Error: CIFD timeout 0, ErrCode:%Xh\r\n", eMMC_ST_ERR_TIMEOUT_WAITCIFDEVENT);
        return eMMC_ST_ERR_TIMEOUT_WAITCIFDEVENT; // timeout
    }

    for(u16_i=0; u16_i<(u32_ByteCnt>>1); u16_i++)
        REG_FCIE_R(NC_WBUF_CIFD_ADDR(u16_i), pu16_Data[u16_i]);

    REG_FCIE_W(NC_CIFD_EVENT,BIT_WBUF_FULL);

    REG_FCIE_W(NC_CIFD_EVENT, BIT_WBUF_EMPTY_TRI);

    return eMMC_ST_SUCCESS;
}


//===================================================

//========================================================
// Send CMD HAL APIs
//========================================================
U32 eMMC_Identify(void)
{
    U32 u32_err = eMMC_ST_SUCCESS;
    U16 u16_i, u16_retry=0;
    U32 start_time;

    g_eMMCDrv.u16_RCA=1;

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
    if(eMMC_ST_SUCCESS != u32_err) {
        u16_retry++;
        goto LABEL_IDENTIFY_CMD0;
    }

    // CMD1
    start_time = get_timer(0);
    for(u16_i=0; u16_i<eMMC_CMD1_RETRY_CNT; u16_i++)
    {
        //eMMC_debug(eMMC_DEBUG_LEVEL, 1, "CMD1 try: %02Xh \r\n", u16_i);
        u32_err = eMMC_CMD1();
        if(eMMC_ST_SUCCESS == u32_err)
            break;

        eMMC_hw_timer_sleep(2);

        if(eMMC_ST_ERR_CMD1_DEV_NOT_RDY != u32_err) {
            u16_retry++;
            goto LABEL_IDENTIFY_CMD0;
        }
    }
    eMMC_debug(eMMC_DEBUG_LEVEL, 1, "CMD1 try: %02Xh Cost:%ld\r\n", u16_i, (get_timer(0)-start_time));

    if(eMMC_ST_SUCCESS != u32_err) {
        u16_retry++;
        eMMC_debug(eMMC_DEBUG_LEVEL, 1, "CMD1 wait eMMC device ready timeout\n");
        goto LABEL_IDENTIFY_CMD0;
    }

    // CMD2
    u32_err = eMMC_CMD2();
    if(eMMC_ST_SUCCESS != u32_err) {
        u16_retry++;
        goto LABEL_IDENTIFY_CMD0;
    }

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

U32 eMMC_CMD0_RBoot(U32 u32_Arg, U32 u32_Addr, U32 u32_ByteCnt)
{
    //U32 u32_err;
    U16 u16_mode;
    U16 u16_ctrl;
    U16 u16_Events;
    U32 u32_count = 0;

    if( (u32_Arg == 0) || (u32_Arg == 0xF0F0F0F0) )
    {
        u16_mode = BIT_SD_DEFAULT_MODE_REG;
        u16_ctrl = BIT_SD_CMD_EN;
        u16_Events = BIT_SD_CMD_END;
    }
    else if( u32_Arg == 0xFFFFFFFA )
    {
        u16_mode = BIT_SD_DEFAULT_MODE_REG|BIT_SD_DATA_WIDTH_8|BIT_SD_DMA_R_CLK_STOP;
        u16_ctrl = BIT_SD_CMD_EN | BIT_SD_DAT_EN;
        u16_Events = BIT_SD_CMD_END|BIT_DMA_END;

        REG_FCIE_W(FCIE_JOB_BL_CNT, u32_ByteCnt>>eMMC_SECTOR_512BYTE_BITS);

        if( (u32_Addr >> 16) == 0x1FC0 )
        {
            REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, u32_Addr & 0xFFFF);
            REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, 0);
        }
        else
        {
            REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, u32_Addr & 0xFFFF);
            REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, u32_Addr >> 16);
        }
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, u32_ByteCnt & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, u32_ByteCnt >> 16);
    }
    else
    {
        return eMMC_ST_ERR_PARAMETER;
    }


    eMMC_FCIE_SendCmd_Rboot(u16_mode, u16_ctrl, u32_Arg, 0, 0);


    if( u32_Arg == 0xFFFFFFFA )
    {
        //REG_FCIE_SETBIT(FCIE_BOOT, BIT_BOOTSRAM_ACCESS_SEL);
        //REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_BOOT_MODE_EN); // DMA to SRAM
        REG_FCIE_SETBIT(FCIE_BOOT, BIT2);
    }


    // job start
    REG_FCIE_W(FCIE_SD_CTRL, u16_ctrl);
    REG_FCIE_W(FCIE_SD_CTRL, u16_ctrl |BIT_JOB_START);

    eMMC_debug(0,1,"1\r\n");
    // wait event
    while(1)
    {
        if( (REG_FCIE(FCIE_MIE_EVENT) & u16_Events) == u16_Events )
            break;
        u32_count ++;
        eMMC_hw_timer_delay(1);
        if(u32_count > 0x1000)
        {
            eMMC_FCIE_ErrHandler_Stop();
            return eMMC_ST_ERR_TIMEOUT_WAIT_REG0;
        }
    }
    eMMC_debug(0,1,"2\r\n");

    REG_FCIE_W(FCIE_MIE_EVENT, REG_FCIE(FCIE_MIE_EVENT));


    if( u32_Arg == 0xFFFFFFFA )
    {
        //REG_FCIE_CLRBIT(FCIE_BOOT, BIT_BOOTSRAM_ACCESS_SEL);
        REG_FCIE_CLRBIT(FCIE_BOOT, BIT2);
    }

    return eMMC_ST_SUCCESS;
}

// reest, etc.
U32 eMMC_CMD0(U32 u32_Arg)
{
    U32 u32_err;
    U16 u16_ctrl = 0;
    U8  u8_retry_cmd=0;

    //eMMC_debug(0, 0, "\033[32m%s\033[m\r\n", __FUNCTION__);

    if( !g_eMMCDrv.u8_make_sts_err )
    {
        u16_ctrl = BIT_SD_CMD_EN;
    }
    else if( g_eMMCDrv.u8_make_sts_err == FCIE_MAKE_CMD_NO_RSP )
    {
        u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_ERR_DET_ON;
    }
    else if( g_eMMCDrv.u8_make_sts_err == FCIE_MAKE_RD_TOUT_ERR )
    {
        U32 u32_dma_addr;

        u16_ctrl = BIT_SD_CMD_EN | BIT_SD_DAT_EN | BIT_ERR_DET_ON;

        REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
        u32_dma_addr = eMMC_translate_DMA_address_Ex((U32)DMA_R_ADDR, eMMC_SECTOR_512BYTE, READ_FROM_eMMC);

        REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, u32_dma_addr & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, u32_dma_addr >> 16);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, eMMC_SECTOR_512BYTE & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, eMMC_SECTOR_512BYTE >> 16);

        REG_FCIE_W(FCIE_RD_SBIT_TIMER, BIT_RD_SBIT_TIMER_EN|0x01);
    }

    LABEL_SEND_CMD:
    eMMC_FCIE_ClearEvents();

    u32_err = eMMC_FCIE_SendCmd(g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_Arg, 0, 0);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if( !g_eMMCDrv.u8_disable_retry )
        {
            if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT)
            {
                u8_retry_cmd++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD0 retry:%u, %Xh \r\n", u8_retry_cmd, u32_err);
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD0 retry:%u, %Xh \r\n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD0 %Xh \r\n", u32_err);
            eMMC_FCIE_ErrHandler_Stop();
        }
    }

    //eMMC_FCIE_CLK_DIS();
    return u32_err;
}

// send OCR
U32 eMMC_CMD1(void)
{
    U32 u32_err, u32_arg;
    U16 u16_ctrl, u16_reg;

    //[7]     1.7-1.95V 0:high 1:dual
    //[14:08] 2.0-2.6 V
    //[23:15] 2.7-3.6 V
    //[30:29] (00b:10b)/(BYTE:SEC)
    u32_arg = BIT30 | (BIT23|BIT22|BIT21|BIT20|BIT19|BIT18|BIT17|BIT16|BIT15); //ARG[<=2G : >2G]/[x00_FF_80_00 : x40_FF_80_00]
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    eMMC_FCIE_ClearEvents();
    u32_err = eMMC_FCIE_SendCmd(
        g_eMMCDrv.u16_Reg10_Mode, u16_ctrl, u32_arg, 1, eMMC_R3_BYTE_CNT);


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
            "eMMC Warn: CMD1 no Rsp, SD_STS: %04Xh \n", u16_reg);

        return u32_err;
    }
    else
    {   // CMD1 ok, do things here
        //clear status may be BIT_SD_RSP_CRC_ERR
        REG_FCIE_W(FCIE_SD_STATUS, BIT_SD_FCIE_ERR_FLAGS);

        eMMC_FCIE_GetCMDFIFO(0, 3, (U16*)g_eMMCDrv.au8_Rsp); //RDY(2G):x80_FF_80_80/xC0_FF_80_80
        //eMMC_dump_mem(g_eMMCDrv.au8_Rsp, eMMC_R3_BYTE_CNT);

        if(0 == (g_eMMCDrv.au8_Rsp[1] & 0x80)) {
            u32_err = eMMC_ST_ERR_CMD1_DEV_NOT_RDY;
        }
        else {
            g_eMMCDrv.u8_IfSectorMode = (g_eMMCDrv.au8_Rsp[1]&BIT6)>>6;
        }
    }

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
                "eMMC Err: CMD2 SD_STS: %04Xh\n", u16_reg);
            return u32_err;
        }
        else
        {   // CMD2 ok, do things here (get CID)
            eMMC_FCIE_GetCMDFIFO(0, eMMC_R2_BYTE_CNT>>1, (U16*)g_eMMCDrv.au8_CID);
            //eMMC_dump_mem(g_eMMCDrv.au8_CID, eMMC_R2_BYTE_CNT);
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

    if(7 == u8_CmdIdx) {
        g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_RSP_WAIT_D0H;
    }
    else {
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
        if(!(7==u8_CmdIdx && u16_RCA!=g_eMMCDrv.u16_RCA)) 
        {
            if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR))
            {
                if(3 == u8_CmdIdx)
                {
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                        "eMMC WARN: CMD%u SD_STS: %04Xh, Retry: %u\n", u8_CmdIdx, u16_reg, u8_retry_fcie);
                    return u32_err;
                }
                if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
                {
                    u8_retry_fcie++;
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                        "eMMC WARN: CMD%u SD_STS: %04Xh, Retry: %u\n", u8_CmdIdx, u16_reg, u8_retry_fcie);
                    eMMC_FCIE_ErrHandler_Retry();
                    goto LABEL_SEND_CMD;
                }

                u32_err = eMMC_ST_ERR_CMD3_CMD7;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD%u SD_STS: %04Xh, Retry: %u\n", u8_CmdIdx, u16_reg, u8_retry_fcie);
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
            }
        }
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
                    "eMMC WARN: CMD9 SD_STS: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD9;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD9 SD_STS: %04Xh, Retry fail: %u\n", u16_reg, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {   // CMD2 ok, do things here
            eMMC_FCIE_GetCMDFIFO(0, eMMC_R2_BYTE_CNT>>1, (U16*)g_eMMCDrv.au8_CSD);
            //eMMC_dump_mem(g_eMMCDrv.au8_CSD, eMMC_R2_BYTE_CNT);
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
    if(eMMC_ST_SUCCESS != u32_err) {
        eMMC_debug(0,1,"eMMC Err: CMD8 fail\n");
        eMMC_dump_mem(gau8_eMMC_SectorBuf, 0x200);
        return u32_err;
    }

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
            default: eMMC_debug(0,1,"eMMC Err: eMMC BUS_WIDTH not support\n");
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
    g_eMMCDrv.u8_ECSD184_Stroe_Support = gau8_eMMC_SectorBuf[184];
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

    //for Max Enhance Size
    g_eMMCDrv.u8_ECSD157_MaxEnhSize_0= gau8_eMMC_SectorBuf[157];
    g_eMMCDrv.u8_ECSD158_MaxEnhSize_1= gau8_eMMC_SectorBuf[158];
    g_eMMCDrv.u8_ECSD159_MaxEnhSize_2= gau8_eMMC_SectorBuf[159];

    g_eMMCDrv.u8_u8_ECSD155_PartSetComplete = gau8_eMMC_SectorBuf[155];
    g_eMMCDrv.u8_ECSD166_WrRelParam = gau8_eMMC_SectorBuf[166];
    /*printf("[196] DEVICE TYPE = %02Xh\r\n", g_eMMCDrv.u8_ECSD196_DevType);
    printf("[197] DRIVER STRENGTH = %02Xh\r\n", g_eMMCDrv.u8_ECSD197_DriverStrength);
    printf("[185] HS_TIMING = %02Xh\r\n", g_eMMCDrv.u8_ECSD185_HsTiming);*/

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

U32 eMMC_CMD8_CIFD(U8 *pu8_DataBuf)
{
    U32 u32_err, u32_arg;
    volatile U16 u16_mode, u16_ctrl, u16_reg, u16_i;
    U8  u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;;

    // -------------------------------
    // send cmd
    u32_arg =  0;
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;

    LABEL_SEND_CMD:
    u16_mode = BIT_SD_DATA_CIFD | g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    eMMC_FCIE_ClearEvents();

    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);

    REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, eMMC_SECTOR_512BYTE & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, eMMC_SECTOR_512BYTE >> 16);

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
    //read for data
    for (u16_i=0; u16_i< (eMMC_SECTOR_512BYTE >> 6); u16_i++)
    {   // read data
        u32_err = eMMC_WaitGetCIFD((U8*)((U32)pu8_DataBuf + (u16_i << 6)), 0x40);
        if(u32_err != eMMC_ST_SUCCESS)
        {
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }

    // -------------------------------
    // check FCIE
    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
        BIT_DMA_END, TIME_WAIT_1_BLK_END);

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & BIT_SD_FCIE_ERR_FLAGS))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD8 CIFD SD_STS: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }

        u32_err = eMMC_ST_ERR_CMD8_CIFD;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD8 CIFD SD_STS: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
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


    LABEL_END:
    eMMC_FCIE_CLK_DIS();
    return u32_err;

}


// CMD8: send EXT_CSD
U32 eMMC_CMD8_MIU(U8 *pu8_DataBuf)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;
    U32 u32_dma_addr;

    // -------------------------------


    // -------------------------------
    // send cmd
    u32_arg =  0;
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN ;

    LABEL_SEND_CMD:
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;

    eMMC_FCIE_ClearEvents();
    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
    u32_dma_addr = eMMC_translate_DMA_address_Ex((U32)pu8_DataBuf, eMMC_SECTOR_512BYTE, READ_FROM_eMMC);

    REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, u32_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, u32_dma_addr >> 16);

    REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, eMMC_SECTOR_512BYTE & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, eMMC_SECTOR_512BYTE >> 16);



    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 8, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD8 retry:%u, %Xh \r\n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }

        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD8 retry:%u, %Xh \r\n", u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    // -------------------------------
    // check FCIE

    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
        BIT_DMA_END, TIME_WAIT_1_BLK_END);

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & BIT_SD_FCIE_ERR_FLAGS))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD8 SD_STS: %04Xh, Err: %Xh, Retry: %u\r\n", u16_reg, u32_err, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD8_MIU;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD8 SD_STS: %04Xh, Err: %Xh, Retry: %u\r\n", u16_reg, u32_err, u8_retry_fcie);
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
                "eMMC WARN: CMD8 check R1 error: %Xh, Retry: %u\r\n", u32_err, u8_retry_r1);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD8_MIU;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD8 check R1 error: %Xh, Retry: %u\r\n", u32_err, u8_retry_r1);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

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
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: PwrOffNotif already set: %u, now: %u\n",
            u8_OldECSD34, u8_SetECSD34);
        return eMMC_ST_SUCCESS;
    }

    u32_err = eMMC_ModifyExtCSD(eMMC_ExtCSD_WByte, 34, u8_SetECSD34); // PWR_OFF_NOTIFICATION
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

#define BITS_MSK_DRIVER_STRENGTH    0xF0
#define BITS_MSK_TIMING         0x0F

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
            //eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC set bus HIGH speed\r\n");
            break;
        case eMMC_SPEED_HS200:
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_SPEED_HS200;
            //eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC set bus HS200 speed\r\n");
            break;
        case eMMC_SPEED_HS400:
            g_eMMCDrv.u32_DrvFlag |= DRV_FLAG_SPEED_HS400;
            //eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC set bus HS400 speed\r\n");
            break;
        default:
            //eMMC_debug(eMMC_DEBUG_LEVEL,1,"eMMC set bus LOW speed\r\n");
            break;
    }

    return u32_err;
}

//U32 eMMC_SetDrivingStrength(U8 u8Driving)
//{
//    U32 u32_err;
//
//    g_eMMCDrv.u8_ECSD185_HsTiming &= ~BITS_MSK_DRIVER_STRENGTH;
//    g_eMMCDrv.u8_ECSD185_HsTiming |= u8Driving;
//
//    //printf("\r\n\r\n%s(%02Xh)\r\n", __FUNCTION__, g_eMMCDrv.u8_ECSD185_HsTiming);
//    u32_err = eMMC_ModifyExtCSD(eMMC_ExtCSD_WByte, 185, g_eMMCDrv.u8_ECSD185_HsTiming); // HS_TIMING, HS200
//    if(eMMC_ST_SUCCESS != u32_err) {
//        printf("eMMC Err: %s() fail %Xh\r\n", __FUNCTION__, u32_err);
//        return eMMC_ST_ERR_SET_DRV_STRENGTH;
//    }
//
//    return eMMC_ST_SUCCESS;
//}

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
    if(u8_IfDDR == 2 && g_eMMCDrv.u8_ECSD184_Stroe_Support )
    {
        eMMC_debug(0,1,"Enhance Strobe\n");
        u8_value |= BIT7; // Enhanced Storbe
    }
    if(BIT2 == u8_value) {
        return eMMC_ST_ERR_PARAMETER;
    }

    // -------------------------------
    u32_err = eMMC_ModifyExtCSD(eMMC_ExtCSD_WByte, 183, u8_value); // BUS_WIDTH
    if(eMMC_ST_SUCCESS != u32_err) {
        return u32_err;
    }

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

    eMMC_debug(eMMC_DEBUG_LEVEL,1,"set %u bus width\n", u8_BusWidth);
    return u32_err;
}

U32 eMMC_ModifyExtCSD(U8 u8_AccessMode, U8 u8_ByteIdx, U8 u8_Value)
{
    U32 u32_arg, u32_err;

    //eMMC_debug(0, 1, "ByteIdx: %d(%03X), value %02X\r\n", u8_ByteIdx, u8_ByteIdx, u8_Value);

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
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: eMMC: %Xh \r\n", u32_err);
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
                    "eMMC WARN: CMD6 SD_STS: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);

                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }
            u32_err = eMMC_ST_ERR_CMD6;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD6 SD_STS: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
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
                    eMMC_FCIE_ErrHandler_Retry();
                    goto LABEL_SEND_CMD;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD6 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
                eMMC_FCIE_ErrHandler_Stop();
            }
        }
    }

    eMMC_FCIE_CLK_DIS();
    g_eMMCDrv.u32_DrvFlag &= ~DRV_FLAG_RSP_WAIT_D0H;
    return u32_err;
}


U32 eMMC_EraseCMDSeq(U32 u32_eMMCBlkAddr_start, U32 u32_eMMCBlkAddr_end)
{
    U32 u32_err;

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
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD%u retry:%u, %Xh \r\n",
                u8_CmdIdx, u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }

        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD%u retry:%u, %Xh \r\n",
            u8_CmdIdx, u8_retry_cmd, u32_err);
        eMMC_FCIE_ErrHandler_Stop();
    }
    else
    {
        // check status
        REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

        if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR))
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD%u SD_STS: %04Xh, Retry: %u\r\n", u8_CmdIdx, u16_reg, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD3_CMD7;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD%u SD_STS: %04Xh, Retry: %u\r\n", u8_CmdIdx, u16_reg, u8_retry_fcie);
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
                        "eMMC WARN: CMD%u check R1 error: %Xh, retry: %u\r\n",
                        u8_CmdIdx, u32_err, u8_retry_r1);
                    eMMC_FCIE_ErrHandler_Retry();
                    goto LABEL_SEND_CMD;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD%u check R1 error: %Xh, retry: %u\r\n",
                    u8_CmdIdx, u32_err, u8_retry_r1);
                eMMC_FCIE_ErrHandler_Stop();
            }
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
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD38 retry:%u, %Xh \r\n",
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
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD38 SD_STS: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD3_CMD7;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD38 SD_STS: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {   // CMD38 ok, do things here
            u32_err = eMMC_CheckR1Error();
            if(eMMC_ST_SUCCESS != u32_err)
            {
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE()){
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
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
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
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD13 SD_STS: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD13;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD13 SD_STS: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
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
                        "eMMC WARN: CMD13 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
                    eMMC_FCIE_ErrHandler_Retry();
                    goto LABEL_SEND_CMD;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD13 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
                eMMC_FCIE_ErrHandler_Stop();
            }
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
                    "eMMC WARN: CMD16 SD_STS: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD16;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD16 SD_STS: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
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
    U8  u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;
    U32 u32_dma_addr;

    // -------------------------------


    // -------------------------------
    // send cmd
    if( !g_eMMCDrv.u8_make_sts_err )
    {
        u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;
    }
    else
    {
        u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN | BIT_ERR_DET_ON;
    }

    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;

    eMMC_FCIE_ClearEvents();
    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
    u32_dma_addr = eMMC_translate_DMA_address_Ex((U32)pu8_DataBuf, eMMC_SECTOR_512BYTE, READ_FROM_eMMC);

    REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, u32_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, u32_dma_addr >> 16);

    if( !g_eMMCDrv.u8_make_sts_err )
    {
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, eMMC_SECTOR_512BYTE & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, eMMC_SECTOR_512BYTE >> 16);
        REG_FCIE_W(FCIE_BLK_SIZE,  eMMC_SECTOR_512BYTE);
    }
    else if( g_eMMCDrv.u8_make_sts_err == FCIE_MAKE_RD_CRC_ERR )
    {
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (eMMC_SECTOR_512BYTE+1) & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, (eMMC_SECTOR_512BYTE+1) >> 16);
        REG_FCIE_W(FCIE_BLK_SIZE, eMMC_SECTOR_512BYTE+1);
    }

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 17, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if( !g_eMMCDrv.u8_disable_retry )
        {
            if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_cmd++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD17 retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD17 retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
        else
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD17 %Xh, Arg: %Xh \r\n", u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }

    // -------------------------------
    // check FCIE
    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
        BIT_DMA_END, TIME_WAIT_1_BLK_END);

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & BIT_SD_FCIE_ERR_FLAGS))
    {
        if( !g_eMMCDrv.u8_disable_retry )
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD17 SD_STS: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD17_MIU;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD17 SD_STS: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
        else
        {
            u32_err = eMMC_ST_ERR_CMD17_MIU;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD17 SD_STS: %04Xh, Err: %Xh, Arg: %Xh\r\n", u16_reg, u32_err, u32_arg);
            REG_FCIE_CLRBIT(FCIE_RSP_SHIFT_CNT, BIT_RSP_SHIFT_SEL|BIT_RSP_SHIFT_TUNE_MASK);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if( !g_eMMCDrv.u8_disable_retry )
        {
            if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_r1++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD17 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
                eMMC_FCIE_ErrHandler_Retry();
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
        else
        {
            u32_err = eMMC_ST_ERR_CMD17_MIU;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD17 check R1 error: %Xh, Arg: %Xh\r\n", u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }

    LABEL_END:
    eMMC_FCIE_CLK_DIS();
    return u32_err;
}

U32 eMMC_CMD17_CIFD(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg, u16_i;
    U8  u8_retry_r1=0, u8_retry_fcie=0, u8_retry_cmd=0;

    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;

    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = BIT_SD_DATA_CIFD | g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    eMMC_FCIE_ClearEvents();
//    REG_FCIE_CLRBIT(FCIE_DDR_MODE, BIT10);
    REG_FCIE_SETBIT(FCIE_DDR_MODE, BIT1|BIT2|BIT3);

    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
    REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, eMMC_SECTOR_512BYTE & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, eMMC_SECTOR_512BYTE >> 16);

    u32_err = eMMC_FCIE_SendCmd(u16_mode, u16_ctrl, u32_arg, 17, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT  && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD17 CIFD retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD17 CIFD retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
        for (u16_i=0; u16_i< (eMMC_SECTOR_512BYTE >> 6); u16_i++)
        {   // read data
            u32_err = eMMC_WaitGetCIFD((U8*)((U32)pu8_DataBuf + (u16_i << 6)), 0x40);
            if(u32_err != eMMC_ST_SUCCESS)
            {
                eMMC_FCIE_ErrHandler_Stop();
                goto LABEL_END;
            }
        }
        eMMC_dump_mem(pu8_DataBuf,512);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    //read for data
    for (u16_i=0; u16_i< (eMMC_SECTOR_512BYTE >> 6); u16_i++)
    {   // read data
        u32_err = eMMC_WaitGetCIFD((U8*)((U32)pu8_DataBuf + (u16_i << 6)), 0x40);
        if(u32_err != eMMC_ST_SUCCESS)
        {
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }

    // -------------------------------
    // check FCIE
    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
                     BIT_DMA_END, TIME_WAIT_1_BLK_END);

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & BIT_SD_FCIE_ERR_FLAGS))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD17 CIFD SD_STS: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\r\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD17_CIFD;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD17 CIFD SD_STS: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\r\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
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
                "eMMC WARN: CMD17 CIFD check R1 error: %Xh, Retry: %u, Arg: %Xh\r\n", u32_err, u8_retry_r1, u32_arg);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }

        u32_err = eMMC_ST_ERR_CMD17_CIFD;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD17 CIFD check R1 error: %Xh, Retry: %u, Arg: %Xh\r\n", u32_err, u8_retry_r1, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:
    REG_FCIE_CLRBIT(FCIE_DDR_MODE, BIT1|BIT2|BIT3);
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
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD12 retry:%u, %Xh \n", u8_retry_cmd, u32_err);
            eMMC_FCIE_ErrHandler_ReInit();
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
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD12 check SD_STS: %Xh, %Xh, retry: %u\n",
                    u16_reg, u32_err, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_ReInit();
                return u32_err;
            }

            u32_err = eMMC_ST_ERR_CMD12;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD12 check SD_STS: %Xh, %Xh, retry: %u\n",
                u16_reg, u32_err, u8_retry_fcie);
            eMMC_FCIE_ErrHandler_Stop();
        }
        else
        {   u32_err = eMMC_CheckR1Error();
            if(eMMC_ST_SUCCESS != u32_err)
            {
                if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
                {
                    u8_retry_r1++;
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                        "eMMC WARN: CMD12 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
                    return u32_err;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD12 check R1 error: %Xh, Retry: %u\n", u32_err, u8_retry_r1);
                eMMC_FCIE_ErrHandler_Stop();
            }
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
    return u32_err;
    #else
    return eMMC_CMD18_MIU(u32_eMMCBlkAddr, pu8_DataBuf, u16_BlkCnt);
    #endif
}

U32 eMMC_CMD18_MIU(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf, U16 u16_BlkCnt)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;
    U32 u32_dma_addr;

    // -------------------------------


    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;

    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;

    eMMC_FCIE_ClearEvents();
    REG_FCIE_W(FCIE_JOB_BL_CNT, u16_BlkCnt);
    u32_dma_addr = eMMC_translate_DMA_address_Ex((U32)pu8_DataBuf, eMMC_SECTOR_512BYTE*u16_BlkCnt, READ_FROM_eMMC);


    REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, u32_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, u32_dma_addr >> 16);

    REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (eMMC_SECTOR_512BYTE*u16_BlkCnt) & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16,(eMMC_SECTOR_512BYTE*u16_BlkCnt) >> 16);

    u32_err = eMMC_FCIE_SendCmd(u16_mode, u16_ctrl, u32_arg, 18, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
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

    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
        BIT_DMA_END, TIME_WAIT_n_BLK_END*(1+(u16_BlkCnt>>9)));

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & BIT_SD_FCIE_ERR_FLAGS))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD18 SD_STS: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);

            eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Retry();

            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD18;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD18 SD_STS: %04Xh, Err: %Xh, Retry: %u\n", u16_reg, u32_err, u8_retry_fcie);
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
                "eMMC WARN: CMD18 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);

            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD18;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD18 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
        eMMC_FCIE_ErrHandler_Stop();
        goto LABEL_END;
    }

    LABEL_END:

    if(eMMC_ST_SUCCESS != eMMC_CMD12(g_eMMCDrv.u16_RCA))
        eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);

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
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
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
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD23 SD_STS: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD13;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD23 SD_STS: %04Xh, Retry: %u\n", u16_reg, u8_retry_fcie);
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
                    eMMC_FCIE_ErrHandler_Retry();
                    goto LABEL_SEND_CMD;
                }
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC Err: CMD23 check R1 error: %Xh, retry: %u\n", u32_err, u8_retry_r1);
                eMMC_FCIE_ErrHandler_Stop();
            }
        }
    }

    eMMC_FCIE_CLK_DIS();
    return u32_err;
}


U32 eMMC_CMD25(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf, U16 u16_BlkCnt)
{
    U32 u32_err=eMMC_ST_SUCCESS;
    #if defined(ENABLE_eMMC_RIU_MODE) && ENABLE_eMMC_RIU_MODE
    U16 u16_cnt;
    #endif
    #if eMMC_BURST_LEN_AUTOCFG || ENABLE_eMMC_RIU_MODE
    U16 u16_RetryCnt=0;
    LABEL_CMD25:
    #endif
    // ------------------------------RIU mode
    #if defined(ENABLE_eMMC_RIU_MODE) && ENABLE_eMMC_RIU_MODE
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

#if 1 //defined ENABLE_eMMC_HS200 && ENABLE_eMMC_HS200

static const U8 hs200_tunning_pattern_128[128] = {
    0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xCC, 0xCC, 0xCC, 0x33, 0xCC, 0xCC,
    0xCC, 0x33, 0x33, 0xCC, 0xCC, 0xCC, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF,
    0xFF, 0xFF, 0xDD, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xBB, 0xFF, 0xFF, 0xFF, 0xBB,
    0xBB, 0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF, 0xFF, 0x77, 0x77, 0xFF, 0x77, 0xBB, 0xDD, 0xEE, 0xFF,
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xCC, 0xCC, 0xCC, 0x33, 0xCC,
    0xCC, 0xCC, 0x33, 0x33, 0xCC, 0xCC, 0xCC, 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE,
    0xFF, 0xFF, 0xFF, 0xDD, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD, 0xFF, 0xFF, 0xFF, 0xBB, 0xFF, 0xFF, 0xFF,
    0xBB, 0xBB, 0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF, 0xFF, 0x77, 0x77, 0xFF, 0x77, 0xBB, 0xDD, 0xEE
};

static const U8 hs200_tunning_pattern_64[64] = {
    0xFF, 0x0F, 0xFF, 0x00, 0xFF, 0xCC, 0xC3, 0xCC, 0xC3, 0x3C, 0xCC, 0xFF, 0xFE, 0xFF, 0xFE, 0xEF,
    0xFF, 0xDF, 0xFF, 0xDD, 0xFF, 0xFB, 0xFF, 0xFB, 0xBF, 0xFF, 0x7F, 0xFF, 0x77, 0xF7, 0xBD, 0xEF,
    0xFF, 0xF0, 0xFF, 0xF0, 0x0F, 0xFC, 0xCC, 0x3C, 0xCC, 0x33, 0xCC, 0xCF, 0xFF, 0xEF, 0xFF, 0xEE,
    0xFF, 0xFD, 0xFF, 0xFD, 0xDF, 0xFF, 0xBF, 0xFF, 0xBB, 0xFF, 0xF7, 0xFF, 0xF7, 0x7F, 0x7B, 0xDE
};


// read out from RIU then compare pattern is slow
U32 eMMC_CMD21_CIFD(void)
{
    U32 u32_err = 0;
    U16 u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN, u16_reg;
    U16 u16_mode, u16_i;
    U8 i;

    //printf("%s()\r\n", __FUNCTION__);

    eMMC_FCIE_ClearEvents();

    if(g_eMMCDrv.u8_BUS_WIDTH == BIT_SD_DATA_WIDTH_4) {
        REG_FCIE_W(FCIE_BLK_SIZE,  64); // 64 bytes tuning pattern
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, 64 & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, 64 >> 16);
    } else if(g_eMMCDrv.u8_BUS_WIDTH == BIT_SD_DATA_WIDTH_8) {
        REG_FCIE_W(FCIE_BLK_SIZE,  128); // 128 bytes tuning pattern
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, 128 & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, 128 >> 16);
    } else {
        u32_err = eMMC_ST_ERR_CMD21_ONE_BIT;
        printf("eMMC Warn: g_eMMCDrv.u8_BUS_WIDTH = %02Xh\r\n", g_eMMCDrv.u8_BUS_WIDTH);
        goto ErrorHandle;
    }

    if(!(g_eMMCDrv.u8_ECSD196_DevType & BIT4)) {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: eMMC not support HS200 1.8V\r\n");
        u32_err = eMMC_ST_ERR_CMD21_NO_HS200_1_8V;
        goto ErrorHandle;
    }

    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);

    u16_mode = BIT_SD_DATA_CIFD | g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;

    u32_err = eMMC_FCIE_SendCmd(u16_mode, u16_ctrl, 0, 21, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err) {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: CMD55 send CMD fail: %08Xh\r\n", u32_err);
        goto ErrorHandle;
    }

    //read for data
    for (u16_i=0; u16_i< (128 >> 6); u16_i++)
    {   // read data
        u32_err = eMMC_WaitGetCIFD((U8*)((U32)gau8_eMMC_SectorBuf + (u16_i << 6)), 0x40);
        if(u32_err != eMMC_ST_SUCCESS)
        {
            eMMC_FCIE_ErrHandler_Stop();
            goto ErrorHandle;
        }
    }

    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_DMA_END, TIME_WAIT_1_BLK_END);
    if(u32_err) {
        goto ErrorHandle;
    }

    // check status
    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

    if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR)) {

        u32_err = eMMC_ST_ERR_CMD21_NO_RSP;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: CMD6 no Rsp, SD_STS: %04Xh \r\n", u16_reg);
        goto ErrorHandle;

    } else if(u16_reg &BIT_SD_R_CRC_ERR) {

        u32_err = eMMC_ST_ERR_CMD21_DATA_CRC;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: CMD6 data CRC err, SD_STS: %04Xh \r\n", u16_reg);
        goto ErrorHandle;

    }else {
        if(g_eMMCDrv.u8_BUS_WIDTH == BIT_SD_DATA_WIDTH_4)
        {
            if(memcmp((void*)hs200_tunning_pattern_64, (void*)gau8_eMMC_SectorBuf, 64)) {
                printf("tuning pattern 4bit width:");
                for(i=0; i<64; i++) {
                    if(i%16==0) printf("\r\n\t");
                    printf("%02X ", gau8_eMMC_SectorBuf[i]);
                }
                printf("\r\n");
                u32_err = eMMC_ST_ERR_CMD21_DATA_CMP;
                goto ErrorHandle;
            }
        }
        else if(g_eMMCDrv.u8_BUS_WIDTH == BIT_SD_DATA_WIDTH_8)
        {
            if(memcmp((void*)hs200_tunning_pattern_128, (void*)gau8_eMMC_SectorBuf, 128)) {
                printf("tuning pattern 8bit width:");
                for(i=0; i<128; i++) {
                    if(i%16==0) printf("\r\n\t");
                    printf("%02X ", gau8_eMMC_SectorBuf[i]);
                }
                printf("\r\n");
                u32_err = eMMC_ST_ERR_CMD21_DATA_CMP;
                goto ErrorHandle;
            }
        }
    }
ErrorHandle:

    //REG_FCIE_W(FCIE_SDIO_CTRL, BIT_SDIO_BLK_MODE | eMMC_SECTOR_512BYTE); // restore anyway...
    REG_FCIE_W(FCIE_BLK_SIZE,  eMMC_SECTOR_512BYTE);

    if(u32_err)
        printf("[%s] L: %d --> %s() error %08Xh\r\n", __FILE__, __LINE__, __FUNCTION__, u32_err);

    return u32_err;

}

// use memcmp to confirm tuning pattern
U32 eMMC_CMD21_MIU(void)
{
    U32 u32_err = 0;
    U16 u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN | BIT_SD_DAT_EN;
    U16 u16_reg;
    U16 u16_mode;
    U8 i;
    U32 u32_dma_addr;// add for MIU

    //printf("%s()\r\n", __FUNCTION__);
    eMMC_FCIE_ClearEvents();

    if(g_eMMCDrv.u8_BUS_WIDTH == BIT_SD_DATA_WIDTH_4) {
        REG_FCIE_W(FCIE_BLK_SIZE,  64); // 64 bytes tuning pattern
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, 64 & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, 64 >> 16);
    } else if(g_eMMCDrv.u8_BUS_WIDTH == BIT_SD_DATA_WIDTH_8) {
        REG_FCIE_W(FCIE_BLK_SIZE,  128); // 128 bytes tuning pattern
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, 128 & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, 128 >> 16);
    } else {
        u32_err = eMMC_ST_ERR_CMD21_ONE_BIT;
        printf("eMMC Warn: g_eMMCDrv.u8_BUS_WIDTH = %02Xh\r\n", g_eMMCDrv.u8_BUS_WIDTH);
        goto ErrorHandle;
    }

    if(!(g_eMMCDrv.u8_ECSD196_DevType & BIT4)) {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: eMMC not support HS200 1.8V\r\n");
        u32_err = eMMC_ST_ERR_CMD21_NO_HS200_1_8V;
        goto ErrorHandle;
    }

    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH; // edit for MIU
    //printf("\033[7;35mFCIE_SD_MODE=%04X\033[m\r\n", REG_FCIE(FCIE_SD_MODE));
    //printf("\033[7;35mu16_mode=%04X\033[m\r\n", u16_mode);

    // add for MIU start
    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
    u32_dma_addr = eMMC_translate_DMA_address_Ex((U32)gau8_eMMC_SectorBuf, eMMC_SECTOR_512BYTE, READ_FROM_eMMC);

    REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, u32_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, u32_dma_addr >> 16);

    // add for MIU end
    u32_err = eMMC_FCIE_SendCmd(u16_mode, u16_ctrl, 0, 21, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err) {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: CMD21 send CMD fail: %08Xh\r\n", u32_err);
        goto ErrorHandle;
    }

    //u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_MIU_LAST_DONE|BIT_CARD_DMA_END, TIME_WAIT_1_BLK_END); // edit for MIU
    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT, BIT_DMA_END, TIME_WAIT_n_BLK_END);
    if(u32_err) {
        //goto ErrorHandle;
        eMMC_DumpPadClk();
        eMMC_FCIE_DumpRegisters();
        while(1);
    }

    // check status
    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);

    if(u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR)) {

        u32_err = eMMC_ST_ERR_CMD21_NO_RSP;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: CMD6 no Rsp, SD_STS: %04Xh \r\n", u16_reg);
        goto ErrorHandle;

    } else if(u16_reg &BIT_SD_R_CRC_ERR) {

        u32_err = eMMC_ST_ERR_CMD21_DATA_CRC;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Warn: CMD6 data CRC err, SD_STS: %04Xh \r\n", u16_reg);
        goto ErrorHandle;

    } else {
        if(g_eMMCDrv.u8_BUS_WIDTH == BIT_SD_DATA_WIDTH_4)
        {
            if(memcmp((void*)hs200_tunning_pattern_64, (void*)gau8_eMMC_SectorBuf, 64)) {
                printf("tuning pattern 4bit width:");
                for(i=0; i<64; i++) {
                    if(i%16==0) printf("\r\n\t");
                    printf("%02X ", gau8_eMMC_SectorBuf[i]);
                }
                printf("\r\n");
                u32_err = eMMC_ST_ERR_CMD21_DATA_CMP;
                goto ErrorHandle;
            }
        }
        else if(g_eMMCDrv.u8_BUS_WIDTH == BIT_SD_DATA_WIDTH_8)
        {
            if(memcmp((void*)hs200_tunning_pattern_128, (void*)gau8_eMMC_SectorBuf, 128)) {
                printf("tuning pattern 8bit width:");
                for(i=0; i<128; i++) {
                    if(i%16==0) printf("\r\n\t");
                    printf("%02X ", gau8_eMMC_SectorBuf[i]);
                }
                printf("\r\n");
                u32_err = eMMC_ST_ERR_CMD21_DATA_CMP;
                goto ErrorHandle;
            }
        }
    }

ErrorHandle:

    //REG_FCIE_W(FCIE_SDIO_CTRL, BIT_SDIO_BLK_MODE | eMMC_SECTOR_512BYTE); // restore anyway...
    REG_FCIE_W(FCIE_BLK_SIZE,  eMMC_SECTOR_512BYTE);

    if(u32_err)
        printf("[%s] L: %d --> %s() error %08Xh\r\n", __FILE__, __LINE__, __FUNCTION__, u32_err);

    return u32_err;

}

// eMMC CMD21 adtc, R1, fix 128 clock, for HS200 only
//  64 bytes in 4 bits mode
// 128 bytes in 8 bits mode

U32 eMMC_CMD21(void) // send tuning block
{
#if 1
    return eMMC_CMD21_MIU(); // fast
#else
    return eMMC_CMD21_CIFD(); // slow
#endif
}

#endif


#ifdef STATIC_RELIABLE_TEST
extern int reliable_test_sleep; //ms
//extern int enable_pm_sleep_flag;
#endif

U32 eMMC_CMD25_MIU(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf, U16 u16_BlkCnt)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg;
    U8  u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;
    U32 u32_dma_addr;

    // -------------------------------
    // restore clock to max


    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);

    eMMC_FCIE_ClearEvents();

    REG_FCIE_W(FCIE_JOB_BL_CNT, u16_BlkCnt);
    u32_dma_addr = eMMC_translate_DMA_address_Ex((U32)pu8_DataBuf, eMMC_SECTOR_512BYTE*u16_BlkCnt, WRITE_TO_eMMC);

    REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, u32_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, u32_dma_addr >> 16);

    if( g_eMMCDrv.u8_make_sts_err == FCIE_MAKE_WR_TOUT_ERR )
    {
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (eMMC_SECTOR_512BYTE) & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, (eMMC_SECTOR_512BYTE) >> 16);
    }
    else
    {
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (eMMC_SECTOR_512BYTE*u16_BlkCnt) & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, (eMMC_SECTOR_512BYTE*u16_BlkCnt) >> 16);
    }

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 25, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if( !g_eMMCDrv.u8_disable_retry )
        {
            if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_cmd++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD25 retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
                eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);
                eMMC_FCIE_ErrHandler_ReInit();
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD25 retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
        else
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD25 %Xh, Arg: %Xh \n", u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_ReInit();
            //eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR)))
    {
        if( !g_eMMCDrv.u8_disable_retry )
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD25 SD_STS: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\r\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
                eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);
                eMMC_FCIE_ErrHandler_ReInit();
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD25;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD25 SD_STS: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
        else
        {
            u32_err = eMMC_ST_ERR_CMD25;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD25 SD_STS: %04Xh, Err: %Xh, Arg: %Xh\n", u16_reg, u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_ReInit();
            //eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }


    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if( !g_eMMCDrv.u8_disable_retry )
        {
            if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_r1++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD25 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
                eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);
                eMMC_FCIE_ErrHandler_ReInit();
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }
            u32_err = eMMC_ST_ERR_CMD25_CHK_R1;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD25 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
        else
        {
            u32_err = eMMC_ST_ERR_CMD25_CHK_R1;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD25 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
            //eMMC_FCIE_ErrHandler_ReInit();
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }

    // -------------------------------
    // send data
    u32_err = eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        u32_err = eMMC_ST_ERR_CMD25_WAIT_D0H;
        goto LABEL_END;
    }


    if( g_eMMCDrv.u8_make_sts_err == FCIE_MAKE_WR_TOUT_ERR )
    {
        REG_FCIE_W(FCIE_WR_SBIT_TIMER, BIT_WR_SBIT_TIMER_EN|0x02);
        REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W|BIT_ERR_DET_ON);
        REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W|BIT_ERR_DET_ON|BIT_JOB_START);
    }
    else
    {
        REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W);
        REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W|BIT_JOB_START);
    }

    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    REG_FCIE_W(FCIE_MIE_INT_EN, BIT_DMA_END);
    #endif

    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
        BIT_DMA_END, TIME_WAIT_n_BLK_END*(1+(u16_BlkCnt>>7))<< 2);


    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & (BIT_SD_W_FAIL|BIT_SD_W_CRC_ERR)))
    {
        if( !g_eMMCDrv.u8_disable_retry )
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD25 SD_STS: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
                eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);
                eMMC_FCIE_ErrHandler_ReInit();
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }

            u32_err = eMMC_ST_ERR_CMD25;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD25 SD_STS: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
        else
        {
            u32_err = eMMC_ST_ERR_CMD25;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD25 SD_STS: %04Xh, Err: %Xh, Arg: %Xh\n", u16_reg, u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }

    LABEL_END:
    if(eMMC_ST_SUCCESS != eMMC_CMD12(g_eMMCDrv.u16_RCA))
        eMMC_CMD12_NoCheck(g_eMMCDrv.u16_RCA);

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
    U8  u8_retry_fcie=0, u8_retry_r1=0, u8_retry_cmd=0;
    U32 u32_dma_addr;

    // -------------------------------

    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;

    eMMC_FCIE_ClearEvents();
    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);
    u32_dma_addr = eMMC_translate_DMA_address_Ex((U32)pu8_DataBuf, eMMC_SECTOR_512BYTE, WRITE_TO_eMMC);

    REG_FCIE_W(FCIE_MIU_DMA_ADDR_15_0, u32_dma_addr & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_ADDR_31_16, u32_dma_addr >> 16);

    if( g_eMMCDrv.u8_make_sts_err == FCIE_MAKE_WR_CRC_ERR )
    {
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (eMMC_SECTOR_512BYTE-1) & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16,( eMMC_SECTOR_512BYTE-1) >> 16);
        REG_FCIE_W(FCIE_BLK_SIZE, eMMC_SECTOR_512BYTE-1);
    }
    else
    {
        REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, (eMMC_SECTOR_512BYTE) & 0xFFFF);
        REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16,( eMMC_SECTOR_512BYTE) >> 16);
        REG_FCIE_W(FCIE_BLK_SIZE, eMMC_SECTOR_512BYTE);
    }



    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 24, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if( !g_eMMCDrv.u8_disable_retry )
        {
            if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_cmd++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD24 retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
                eMMC_FCIE_ErrHandler_ReInit();
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD24 retry:%u, %Xh, Arg: %Xh \n", u8_retry_cmd, u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
        else
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD24 %Xh, Arg: %Xh \r\n", u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_ReInit();
            goto LABEL_END;
        }
    }
    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & (BIT_SD_RSP_TIMEOUT|BIT_SD_RSP_CRC_ERR)))
    {
        if( !g_eMMCDrv.u8_disable_retry )
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD24 SD_STS: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
                eMMC_FCIE_ErrHandler_ReInit();
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }
            u32_err = eMMC_ST_ERR_CMD24_MIU;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD24 SD_STS: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
        else
        {
            u32_err = eMMC_ST_ERR_CMD24_MIU;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD24 SD_STS: %04Xh, Err: %Xh, Arg: %Xh\r\n", u16_reg, u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }

    // -------------------------------
    // check device
    u32_err = eMMC_CheckR1Error();
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if( !g_eMMCDrv.u8_disable_retry )
        {
            if(u8_retry_r1 < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_r1++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD24 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
                eMMC_FCIE_ErrHandler_ReInit();
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }
            u32_err = eMMC_ST_ERR_CMD24_MIU_CHK_R1;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD24 check R1 error: %Xh, Retry: %u, Arg: %Xh\n", u32_err, u8_retry_r1, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
        else
        {
            u32_err = eMMC_ST_ERR_CMD24_MIU_CHK_R1;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD24 check R1 error: %Xh, Arg: %Xh\n", u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }

    // -------------------------------
    // send data
    u32_err = eMMC_FCIE_WaitD0High(TIME_WAIT_DAT0_HIGH);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        u32_err = eMMC_ST_ERR_CMD24_MIU_WAIT_D0H;
        goto LABEL_END;
    }

    if( g_eMMCDrv.u8_make_sts_err == FCIE_MAKE_WR_CRC_ERR )
    {
        REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W|BIT_ERR_DET_ON);
        REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W|BIT_JOB_START|BIT_ERR_DET_ON);
    }
    else
    {
        REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W);
        REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W|BIT_JOB_START);
    }

    #if defined(ENABLE_eMMC_INTERRUPT_MODE) && ENABLE_eMMC_INTERRUPT_MODE
    REG_FCIE_W(FCIE_MIE_INT_EN, BIT_DMA_END);
    #endif
    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
        BIT_DMA_END, TIME_WAIT_1_BLK_END);

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & (BIT_SD_W_FAIL|BIT_SD_W_CRC_ERR)))
    {
        if( !g_eMMCDrv.u8_disable_retry )
        {
            if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
            {
                u8_retry_fcie++;
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                    "eMMC WARN: CMD24 SD_STS: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
                eMMC_FCIE_ErrHandler_ReInit();
                eMMC_FCIE_ErrHandler_Retry();
                goto LABEL_SEND_CMD;
            }
            u32_err = eMMC_ST_ERR_CMD24_MIU;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD24 SD_STS: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
        else
        {
            u32_err = eMMC_ST_ERR_CMD24_MIU;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC Err: CMD24 SD_STS: %04Xh, Err: %Xh, Arg: %Xh\n", u16_reg, u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }

    LABEL_END:
    eMMC_FCIE_CLK_DIS();
    return u32_err;
}

U32 eMMC_CMD24_CIFD(U32 u32_eMMCBlkAddr, U8 *pu8_DataBuf)
{
    U32 u32_err, u32_arg;
    U16 u16_mode, u16_ctrl, u16_reg, u16_i;
    U8  u8_retry_r1=0, u8_retry_fcie=0, u8_retry_cmd=0;

    // -------------------------------


    // -------------------------------
    // send cmd
    u16_ctrl = BIT_SD_CMD_EN | BIT_SD_RSP_EN;

    LABEL_SEND_CMD:
    u32_arg =  u32_eMMCBlkAddr << (g_eMMCDrv.u8_IfSectorMode?0:eMMC_SECTOR_512BYTE_BITS);
    u16_mode = BIT_SD_DATA_CIFD | g_eMMCDrv.u16_Reg10_Mode | g_eMMCDrv.u8_BUS_WIDTH;
    eMMC_FCIE_ClearEvents();

    REG_FCIE_W(FCIE_JOB_BL_CNT, 1);

    REG_FCIE_W(FCIE_MIU_DMA_LEN_15_0, eMMC_SECTOR_512BYTE & 0xFFFF);
    REG_FCIE_W(FCIE_MIU_DMA_LEN_31_16, eMMC_SECTOR_512BYTE >> 16);

    u32_err = eMMC_FCIE_SendCmd(
        u16_mode, u16_ctrl, u32_arg, 24, eMMC_R1_BYTE_CNT);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        if(u8_retry_cmd < eMMC_FCIE_CMD_RSP_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_cmd++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC WARN: CMD24 CIFD retry:%u, %Xh, Arg: %Xh \r\n", u8_retry_cmd, u32_err, u32_arg);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "eMMC Err: CMD24 CIFD retry:%u, %Xh, Arg: %Xh \r\n", u8_retry_cmd, u32_err, u32_arg);
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
                "eMMC WARN: CMD24 CIFD check R1 error: %Xh, Retry: %u, Arg: %Xh \n", u32_err, u8_retry_r1, u32_arg);
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
    REG_FCIE_W(FCIE_SD_CTRL, BIT_SD_DAT_EN|BIT_SD_DAT_DIR_W|BIT_JOB_START);

    for (u16_i=0; u16_i< (eMMC_SECTOR_512BYTE >> 6); u16_i++)
    {
        u32_err = eMMC_WaitSetCIFD( (U8*)( (U32) pu8_DataBuf + (u16_i << 6)), 0x40);
        if(u32_err != eMMC_ST_SUCCESS)
        {
            eMMC_FCIE_ErrHandler_Stop();
            goto LABEL_END;
        }
    }

    u32_err = eMMC_FCIE_WaitEvents(FCIE_MIE_EVENT,
                 BIT_DMA_END, TIME_WAIT_1_BLK_END);

    REG_FCIE_R(FCIE_SD_STATUS, u16_reg);
    if(eMMC_ST_SUCCESS != u32_err || (u16_reg & (BIT_SD_W_FAIL|BIT_SD_W_CRC_ERR)))
    {
        if(u8_retry_fcie < eMMC_CMD_API_ERR_RETRY_CNT && 0==eMMC_IF_TUNING_TTABLE())
        {
            u8_retry_fcie++;
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
                "eMMC WARN: CMD24 CIFD SD_STS: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
            eMMC_FCIE_ErrHandler_Retry();
            goto LABEL_SEND_CMD;
        }
        u32_err = eMMC_ST_ERR_CMD24_CIFD;
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1,
            "eMMC Err: CMD24 CIFD SD_STS: %04Xh, Err: %Xh, Retry: %u, Arg: %Xh\n", u16_reg, u32_err, u8_retry_fcie, u32_arg);
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
    //U8 u8_cs = 0;

    eMMC_FCIE_GetCMDFIFO(0, 3, (U16*)g_eMMCDrv.au8_Rsp);

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

#endif

#endif  //#ifdef IP_FCIE_VERSION_5
