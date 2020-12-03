/*
* eMMC_hal_speed.c- Sigmastar
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

// ==========================================================
U32 eMMC_FCIE_EnableSDRMode(void)
{
    U32 u32_ErrSpeed = eMMC_ST_SUCCESS, u32_ErrWidth = eMMC_ST_SUCCESS, u32_Err = eMMC_ST_SUCCESS;

    // ----------------------------------------
    // may call from any other interface status
    #if 0
    if(DRV_FLAG_SPEED_HS200 == eMMC_SPEED_MODE())
    {
        u32_ErrSpeed = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);
    }
    else if(DRV_FLAG_SPEED_HS400 == eMMC_SPEED_MODE())
    {
        u32_ErrSpeed = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);
        u32_ErrWidth = eMMC_SetBusWidth(g_eMMCDrv.u16_of_buswidth, 0);
    }
    else
    #else
    if(DRV_FLAG_SPEED_HS200 == eMMC_SPEED_MODE()||
        DRV_FLAG_SPEED_HS400 == eMMC_SPEED_MODE())
    {
        u32_Err = eMMC_FCIE_ErrHandler_ReInit_Ex();
        if(eMMC_ST_SUCCESS != u32_Err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh \n", u32_Err);
            return u32_Err;
        }
    }
    #endif
    {
        u32_ErrSpeed = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);
        u32_ErrWidth = eMMC_SetBusWidth(g_eMMCDrv.u16_of_buswidth, 0);
    }

    if(eMMC_ST_SUCCESS!=u32_ErrSpeed || eMMC_ST_SUCCESS!=u32_ErrWidth)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: %Xh %Xh\n", u32_ErrSpeed, u32_ErrWidth);
        return u32_ErrSpeed ? u32_ErrSpeed : u32_ErrWidth;
    }

    // ----------------------------------------
    // set to normal SDR 48MHz
    eMMC_pads_switch(FCIE_eMMC_SDR);
    eMMC_clock_setting(FCIE_DEFAULT_CLK);

    return eMMC_ST_SUCCESS;
}


// ===========================
#if !(defined(ENABLE_eMMC_ATOP) && ENABLE_eMMC_ATOP)
void eMMC_DumpDDR48TTable(void)
{
    U16 u16_i;

    eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n  eMMC DDR Timing Table: Cnt:%u CurIdx:%u \n",
        g_eMMCDrv.TimingTable_t.u8_SetCnt, g_eMMCDrv.TimingTable_t.u8_CurSetIdx);

    for(u16_i=0; u16_i<g_eMMCDrv.TimingTable_t.u8_SetCnt; u16_i++)
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"    Set:%u: clk:%02Xh, DQS:%02Xh, Cell:%02Xh \n",
            u16_i, g_eMMCDrv.TimingTable_t.Set[u16_i].u8_Clk,
            g_eMMCDrv.TimingTable_t.Set[u16_i].Param.u8_DQS,
            g_eMMCDrv.TimingTable_t.Set[u16_i].Param.u8_Cell);
}

void eMMC_FCIE_SetDDR48TimingReg(U8 u8_DQS, U8 u8_DelaySel)
{
    #ifndef IP_FCIE_VERSION_5
    REG_FCIE_CLRBIT(FCIE_SM_STS, BIT_DQS_MODE_MASK|BIT_DQS_DELAY_CELL_MASK);

    REG_FCIE_SETBIT(FCIE_SM_STS, u8_DQS<<BIT_DQS_MODE_SHIFT);
    REG_FCIE_SETBIT(FCIE_SM_STS, u8_DelaySel<<BIT_DQS_DELAY_CELL_SHIFT);
    #endif
}

// ===========================
#else // DDR52 (ATOP)
void eMMC_DumpATopTable(void)
{
    U16 u16_i;

    eMMC_debug(eMMC_DEBUG_LEVEL,0,"\n  eMMC ATop Timing Table: Ver:%Xh Cnt:%u CurIdx:%u \n",
        g_eMMCDrv.TimingTable_t.u32_VerNo,
        g_eMMCDrv.TimingTable_t.u8_SetCnt, g_eMMCDrv.TimingTable_t.u8_CurSetIdx);

    for(u16_i=0; u16_i<g_eMMCDrv.TimingTable_t.u8_SetCnt; u16_i++)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"    Set:%u: \n", u16_i);
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"      Clk: %04Xh, Reg2Ch: %02Xh, Skew4: %02Xh, Result: %08Xh, Idx: %u \n",
                g_eMMCDrv.TimingTable_t.Set[u16_i].u8_Clk,
            g_eMMCDrv.TimingTable_t.Set[u16_i].u8_Reg2Ch,
            g_eMMCDrv.TimingTable_t.Set[u16_i].u8_Skew4,
                g_eMMCDrv.TimingTable_t.Set[u16_i].u32_ScanResult,
                g_eMMCDrv.TimingTable_Ext_t.Set.u8_Skew4Idx);

            #if defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400
        eMMC_debug(eMMC_DEBUG_LEVEL,0,"      Skew2: %02Xh, RXDLL: %Xh, RXDLLCnt: %02Xh \n",
                g_eMMCDrv.TimingTable_t.Set[u16_i].u8_Skew2,
                g_eMMCDrv.TimingTable_t.Set[u16_i].u8_Cell,
                g_eMMCDrv.TimingTable_t.Set[u16_i].u8_CellCnt);
            #endif
    }
}

void eMMC_FCIE_SetATopTimingReg(U8 u8_SetIdx)
{
    #ifdef IP_FCIE_VERSION_5
    if(g_eMMCDrv.u8_PadType== FCIE_eMMC_DDR)
    {
        REG_FCIE_CLRBIT(reg_emmcpll_0x6c, BIT_DQS_MODE_MASK|BIT_DQS_DELAY_CELL_MASK);

        REG_FCIE_SETBIT(reg_emmcpll_0x6c, g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Skew4<<BIT_DQS_MODE_SHIFT);
        REG_FCIE_SETBIT(reg_emmcpll_0x6c, g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Cell<<BIT_DQS_DELAY_CELL_SHIFT);
    }

    #if (defined(ENABLE_eMMC_HS200) && ENABLE_eMMC_HS200) || \
        (defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400)
    else//HS400,HS200
    {
        if(g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Reg2Ch)
        {
            #ifdef REG_DIG_SKEW4_INV
            REG_FCIE_SETBIT(REG_DIG_SKEW4_INV, BIT_DIG_SKEW4_INV);       //skew4 inverse for muji
            #endif
            #ifdef REG_ANL_SKEW4_INV
            REG_FCIE_SETBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
            #endif
        }
        else
        {
            #ifdef REG_DIG_SKEW4_INV
            REG_FCIE_CLRBIT(REG_DIG_SKEW4_INV, BIT_DIG_SKEW4_INV);       //skew4 inverse for muji
            #endif
            #ifdef REG_ANL_SKEW4_INV
            REG_FCIE_CLRBIT(REG_ANL_SKEW4_INV, BIT_ANL_SKEW4_INV);
            #endif
        }

        REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
        REG_FCIE_SETBIT(reg_emmcpll_0x03,
            g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Skew4<<12);

        #if defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400
        eMMC_FCIE_SetDelayLatch(g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Cell);

        REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW2_MASK);
        REG_FCIE_SETBIT(reg_emmcpll_0x03,
                g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Skew2<<4);
        #endif
    }
    #endif

    #else
    if(g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Reg2Ch)
    {
        REG_FCIE_SETBIT(FCIE_SM_STS, BIT11);
    }
    else
    {
        REG_FCIE_CLRBIT(FCIE_SM_STS, BIT11);
    }

    REG_FCIE_CLRBIT(reg_emmcpll_0x03, BIT_SKEW4_MASK);
    REG_FCIE_SETBIT(reg_emmcpll_0x03,
    g_eMMCDrv.TimingTable_t.Set[u8_SetIdx].u8_Skew4<<12);
    #endif
}

void eMMC_FCIE_Apply_Reg(U8 u8_SetIdx)
{
    int i;

    for(i = 0; i < g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt; i ++)
    {
        if(g_eMMCDrv.TimingTable_G_t.RegSet[u8_SetIdx * g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt + i].u16_OpCode ==
            REG_OP_W)
        {
            REG_FCIE_W( RIU_BASE + g_eMMCDrv.TimingTable_G_t.RegSet[u8_SetIdx * g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt + i].u32_RegAddress,  //address = (bank address+ register offset) << 2
                    g_eMMCDrv.TimingTable_G_t.RegSet[u8_SetIdx * g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt + i].u16_RegValue);
        }
        else if(g_eMMCDrv.TimingTable_G_t.RegSet[u8_SetIdx * g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt + i].u16_OpCode ==
            REG_OP_CLRBIT)
        {
            REG_FCIE_CLRBIT( RIU_BASE + g_eMMCDrv.TimingTable_G_t.RegSet[u8_SetIdx * g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt + i].u32_RegAddress,  //address = (bank address+ register offset) << 2
                g_eMMCDrv.TimingTable_G_t.RegSet[u8_SetIdx * g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt + i].u16_RegValue);
        }
        else if(g_eMMCDrv.TimingTable_G_t.RegSet[u8_SetIdx * g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt + i].u16_OpCode ==
            REG_OP_SETBIT)
        {
            REG_FCIE_CLRBIT( RIU_BASE + g_eMMCDrv.TimingTable_G_t.RegSet[u8_SetIdx * g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt + i].u32_RegAddress,  //address = (bank address+ register offset) << 2
                g_eMMCDrv.TimingTable_G_t.RegSet[u8_SetIdx * g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt + i].u16_RegMask);

            REG_FCIE_SETBIT( RIU_BASE + g_eMMCDrv.TimingTable_G_t.RegSet[u8_SetIdx * g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt + i].u32_RegAddress,  //address = (bank address+ register offset) << 2
                g_eMMCDrv.TimingTable_G_t.RegSet[u8_SetIdx * g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt + i].u16_RegValue);
        }
        else if(g_eMMCDrv.TimingTable_G_t.RegSet[u8_SetIdx * g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt + i].u16_OpCode ==
            0)
           //do nothing here
           ;
        else
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR, 1, "unknown OpCode 0x%X\n", g_eMMCDrv.TimingTable_G_t.RegSet[u8_SetIdx * g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt + i].u16_OpCode);
            eMMC_die("");
        }
    }
}

void eMMC_DumpGeneralTable(void)
{
    int i, j;
    eMMC_debug(eMMC_DEBUG_LEVEL, 0, "OpCode Address Value Mask\n");

    for(i = 0; i< g_eMMCDrv.TimingTable_G_t.u8_SetCnt; i++)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL, 0, "==========Set 0x%X==========\n", i);

        for(j = 0; j < g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt; j++)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL, 0, "0x%X 0x%08X 0x%04X 0x%04X\n",
                g_eMMCDrv.TimingTable_G_t.RegSet[i*g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt+j].u16_OpCode,
                g_eMMCDrv.TimingTable_G_t.RegSet[i*g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt+j].u32_RegAddress,
                g_eMMCDrv.TimingTable_G_t.RegSet[i*g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt+j].u16_RegValue,
                g_eMMCDrv.TimingTable_G_t.RegSet[i*g_eMMCDrv.TimingTable_G_t.u8_RegisterCnt+j].u16_RegMask);
        }
    }
}

#endif


// if failed, must be board issue.
U32 eMMC_FCIE_ChooseSpeedMode(void)
{
    U32 u32_err = eMMC_ST_SUCCESS;

    #if defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400
    if(g_eMMCDrv.u8_ECSD196_DevType & eMMC_DEVTYPE_HS400_1_8V)
    {
        u32_err = eMMC_CMD13(g_eMMCDrv.u16_RCA);
        if(eMMC_ST_SUCCESS != u32_err)
            return u32_err;
        if(eMMC_GetR1() & eMMC_R1_DEVICE_IS_LOCKED)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL,1,"\neMMC Warn: HS400, but locked\n");
            return u32_err;
        }

        if(eMMC_ST_SUCCESS != eMMC_FCIE_EnableFastMode(FCIE_eMMC_HS400) )
        {
            u32_err = eMMC_FCIE_BuildHS200TimingTable(); // DS mode uses HS200 timing table
            if(eMMC_ST_SUCCESS != u32_err)
                return u32_err;

            // HS400 use DLL setting for DAT[0-7]
            u32_err = eMMC_FCIE_DetectHS400Timing();
            if(eMMC_ST_SUCCESS != u32_err)
            {
                    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
                               "eMMC Err: eMMC_FCIE_DetectHS400Timing fail: %Xh\n", u32_err);
                    eMMC_FCIE_ErrHandler_Stop();
                }
        }

        //eMMC_debug(0,0,"\neMMC: HS400 %uMHz \n", g_eMMCDrv.u32_ClkKHz/1000);
        return u32_err;
    }
    #endif

    #if defined(ENABLE_eMMC_HS200) && ENABLE_eMMC_HS200
    if(g_eMMCDrv.u8_ECSD196_DevType & eMMC_DEVTYPE_HS200_1_8V)
    {
        u32_err = eMMC_CMD13(g_eMMCDrv.u16_RCA);
        if(eMMC_ST_SUCCESS != u32_err)
            return u32_err;
        if(eMMC_GetR1() & eMMC_R1_DEVICE_IS_LOCKED)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL,1,"\neMMC Warn: HS200, but locked\n");
            return u32_err;
        }

        if(eMMC_ST_SUCCESS != eMMC_FCIE_EnableFastMode(FCIE_eMMC_HS200) )
            u32_err = eMMC_FCIE_BuildHS200TimingTable();

        //eMMC_debug(0,0,"\neMMC: HS200 %uMHz \n", g_eMMCDrv.u32_ClkKHz/1000);
        return u32_err;
    }
    #endif

    #if (defined(ENABLE_eMMC_ATOP)&&ENABLE_eMMC_ATOP) || (defined(IF_DETECT_eMMC_DDR_TIMING)&&IF_DETECT_eMMC_DDR_TIMING)
    if(g_eMMCDrv.u8_ECSD196_DevType & eMMC_DEVTYPE_DDR)
    {
        if(eMMC_ST_SUCCESS != eMMC_FCIE_EnableFastMode(FCIE_eMMC_DDR))
            u32_err = eMMC_FCIE_BuildDDRTimingTable();

        eMMC_debug(0,0,"\neMMC: DDR %uMHz \n", g_eMMCDrv.u32_ClkKHz/1000);
        return u32_err;
    }
    #endif

    //eMMC_debug(0,0,"\neMMC: SDR %uMHz\n", g_eMMCDrv.u32_ClkKHz/1000);
    return u32_err;
}


void eMMC_FCIE_ApplyTimingSet(U8 u8_Idx)
{
    // make sure a complete outside clock cycle
    REG_FCIE_CLRBIT(FCIE_SD_MODE, BIT_SD_CLK_EN);

    #if !(defined(ENABLE_eMMC_ATOP) && ENABLE_eMMC_ATOP)
    // DDR48
    eMMC_clock_setting(g_eMMCDrv.TimingTable_t.Set[u8_Idx].u8_Clk);
    eMMC_FCIE_SetDDR48TimingReg(
        g_eMMCDrv.TimingTable_t.Set[u8_Idx].Param.u8_DQS,
        g_eMMCDrv.TimingTable_t.Set[u8_Idx].Param.u8_Cell);
    g_eMMCDrv.TimingTable_t.u8_CurSetIdx = u8_Idx;
    #else
    // HS400 or HS200 or DDR52
    if( g_eMMCDrv.u8_PadType != FCIE_eMMC_HS400 ||g_eMMCDrv.TimingTable_G_t.u8_SetCnt == 0)
    {
        eMMC_clock_setting(g_eMMCDrv.TimingTable_t.Set[u8_Idx].u8_Clk);
        eMMC_FCIE_SetATopTimingReg(u8_Idx);
        g_eMMCDrv.TimingTable_t.u8_CurSetIdx = u8_Idx;
    }
    else
    //apply ext table
    {
        eMMC_clock_setting(g_eMMCDrv.TimingTable_G_t.u32_Clk);
        eMMC_FCIE_Apply_Reg(u8_Idx);
        g_eMMCDrv.TimingTable_G_t.u8_CurSetIdx = u8_Idx;
    }

    #endif
}


void eMMC_DumpTimingTable(void)
{
    #if !(defined(ENABLE_eMMC_ATOP) && ENABLE_eMMC_ATOP)
    eMMC_DumpDDR48TTable();
    #else
    eMMC_DumpATopTable();
    if( g_eMMCDrv.TimingTable_G_t.u8_SetCnt)
        eMMC_DumpGeneralTable();

    #endif
}


U32 eMMC_LoadTimingTable(U8 u8_PadType)
{
    U32 u32_err;
    U32 u32_ChkSum, u32_eMMCBlkAddr;

    // --------------------------------------

    switch(u8_PadType)
    {
        #if defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400
        case FCIE_eMMC_HS400:
            u32_eMMCBlkAddr = eMMC_HS400TABLE_BLK_0;
            break;
        #endif
        #if defined(ENABLE_eMMC_HS200) && ENABLE_eMMC_HS200
        case FCIE_eMMC_HS200:
            u32_eMMCBlkAddr = eMMC_HS200TABLE_BLK_0;
            break;
        #endif
        case FCIE_eMMC_DDR:
            u32_eMMCBlkAddr = eMMC_DDRTABLE_BLK_0;
            break;
        default:
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: unknown pad type %Xh\n", u8_PadType);
            return eMMC_ST_ERR_DDRT_NONA;
    }

    // --------------------------------------

    u32_err = eMMC_CMD18(u32_eMMCBlkAddr, gau8_eMMC_SectorBuf, 1);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: load 1 Table fail, %Xh\n", u32_err);

        u32_err = eMMC_CMD18(u32_eMMCBlkAddr+1, gau8_eMMC_SectorBuf, 1);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: load 2 Tables fail, %Xh\n", u32_err);
            goto LABEL_END_OF_NO_TABLE;
        }
    }

    // --------------------------------------
    memcpy((U8*)&g_eMMCDrv.TimingTable_t, gau8_eMMC_SectorBuf, sizeof(g_eMMCDrv.TimingTable_t));

    u32_ChkSum = eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_t, sizeof(g_eMMCDrv.TimingTable_t)-eMMC_TIMING_TABLE_CHKSUM_OFFSET);
    if(u32_ChkSum != g_eMMCDrv.TimingTable_t.u32_ChkSum)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: ChkSum error, no Table \n");
        printk("u32_ChkSum=%08X\n", u32_ChkSum);
        printk("g_eMMCDrv.TimingTable_t.u32_ChkSum=%08X\n", g_eMMCDrv.TimingTable_t.u32_ChkSum);
        eMMC_dump_mem((U8*)&g_eMMCDrv.TimingTable_t,sizeof(g_eMMCDrv.TimingTable_t));
        u32_err = eMMC_ST_ERR_DDRT_CHKSUM;
        goto LABEL_END_OF_NO_TABLE;
    }

    if(0==u32_ChkSum )
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: no Table \n");
        u32_err = eMMC_ST_ERR_DDRT_NONA;
        goto LABEL_END_OF_NO_TABLE;
    }

    #if defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400
    //read general table if hs400
    if(FCIE_eMMC_HS400 == u8_PadType)
    {
        u32_err = eMMC_CMD18(eMMC_HS400EXTTABLE_BLK_0, gau8_eMMC_SectorBuf, 1);
        if(eMMC_ST_SUCCESS != u32_err)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC WARN: load 1st Gen_TTable fail, %Xh\n", u32_err);

            u32_err = eMMC_CMD18(eMMC_HS400EXTTABLE_BLK_1, gau8_eMMC_SectorBuf, 1);
            if(eMMC_ST_SUCCESS != u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: load 2nd Gen_TTables fail, %Xh\n", u32_err);
                g_eMMCDrv.TimingTable_G_t.u8_SetCnt = 0;
                g_eMMCDrv.TimingTable_G_t.u32_ChkSum = g_eMMCDrv.TimingTable_G_t.u32_VerNo = 0;
                goto LABEL_END_OF_NO_GEN_TABLE;
            }
        }
        memcpy((U8*)&g_eMMCDrv.TimingTable_G_t, gau8_eMMC_SectorBuf, sizeof(g_eMMCDrv.TimingTable_G_t));

        u32_ChkSum = eMMC_ChkSum((U8*)&g_eMMCDrv.TimingTable_G_t.u32_VerNo,
                        (sizeof(g_eMMCDrv.TimingTable_G_t) - sizeof(U32)/*checksum*/));
        if(u32_ChkSum != g_eMMCDrv.TimingTable_G_t.u32_ChkSum)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: ChkSum error, no Gen_TTable \n");
            g_eMMCDrv.TimingTable_G_t.u8_SetCnt = 0;
            g_eMMCDrv.TimingTable_G_t.u32_ChkSum = g_eMMCDrv.TimingTable_G_t.u32_VerNo = 0;
            goto LABEL_END_OF_NO_GEN_TABLE;
        }

        if(0==u32_ChkSum)
        {
            eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: no Gen_TTable \n");
            g_eMMCDrv.TimingTable_G_t.u8_SetCnt = 0;
            g_eMMCDrv.TimingTable_G_t.u32_ChkSum = g_eMMCDrv.TimingTable_G_t.u32_VerNo = 0;
            goto LABEL_END_OF_NO_GEN_TABLE;
        }
    }
    LABEL_END_OF_NO_GEN_TABLE:
    #endif

    return eMMC_ST_SUCCESS;

    LABEL_END_OF_NO_TABLE:
    g_eMMCDrv.TimingTable_t.u8_SetCnt = 0;
    #if defined(ENABLE_eMMC_HS400) && ENABLE_eMMC_HS400
    g_eMMCDrv.TimingTable_G_t.u8_SetCnt = 0;
    g_eMMCDrv.TimingTable_G_t.u32_ChkSum = g_eMMCDrv.TimingTable_G_t.u32_VerNo = 0;
    #endif

    eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: load Tables fail, %Xh\n", u32_err);
    return u32_err;

}


// set eMMC device & pad registers (no macro timing registers, since also involved in tuning procedure)
U32 eMMC_FCIE_EnableFastMode_Ex(U8 u8_PadType)
{
    U32 u32_err=eMMC_ST_SUCCESS;

    switch(u8_PadType)
    {
        case FCIE_eMMC_DDR:
            u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HIGH);
            if(eMMC_ST_SUCCESS!=u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: enable HighSpeed fail: %Xh\n", u32_err);
                return u32_err;
            }
            u32_err = eMMC_SetBusWidth(g_eMMCDrv.u16_of_buswidth, 1);
            if(eMMC_ST_SUCCESS!=u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: enable DDR fail: %Xh\n", u32_err);
                return u32_err;
            }

            if(0==g_eMMCDrv.TimingTable_t.u8_SetCnt)
                #if defined(ENABLE_eMMC_ATOP)&&ENABLE_eMMC_ATOP
                eMMC_clock_setting(gau8_eMMCPLLSel_52[0]);
                #else
                eMMC_clock_setting(gau8_FCIEClkSel[0]);
                #endif
            break;

        #if (defined(ENABLE_eMMC_HS200)&&ENABLE_eMMC_HS200) || \
            (defined(ENABLE_eMMC_HS400)&&ENABLE_eMMC_HS400)
        case FCIE_eMMC_HS200:
            u32_err = eMMC_SetBusWidth(g_eMMCDrv.u16_of_buswidth, 0); // disable DDR
            if(eMMC_ST_SUCCESS!=u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: HS200 disable DDR fail: %Xh\n", u32_err);
                return u32_err;
            }
            u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HS200);
            if(eMMC_ST_SUCCESS!=u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: enable HS200 fail: %Xh\n", u32_err);
                return u32_err;
            }
            if(0==g_eMMCDrv.TimingTable_t.u8_SetCnt)
                eMMC_clock_setting(gau8_eMMCPLLSel_200[0]);

            break;
        #endif

        #if defined(ENABLE_eMMC_HS400)&&ENABLE_eMMC_HS400
        case FCIE_eMMC_HS400:
            u32_err = eMMC_SetBusWidth(g_eMMCDrv.u16_of_buswidth, 1); // enable DDR
            if(eMMC_ST_SUCCESS!=u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: HS400 enable DDR fail: %Xh\n", u32_err);
                return u32_err;
            }
            u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HS400);
            if(eMMC_ST_SUCCESS!=u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: enable HS400 fail: %Xh\n", u32_err);
                return u32_err;
            }
            if(0==g_eMMCDrv.TimingTable_t.u8_SetCnt)
                eMMC_clock_setting(gau8_eMMCPLLSel_200[0]);
            break;
        #endif
        #if defined(ENABLE_eMMC_HS400_5_1)&&ENABLE_eMMC_HS400_5_1
        case FCIE_eMMC_HS400_5_1:
            u32_err = eMMC_SetBusWidth(g_eMMCDrv.u16_of_buswidth, 2); // enable DDR
            if(eMMC_ST_SUCCESS!=u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: HS400 5.1 enable DDR fail: %Xh\n", u32_err);
                return u32_err;
            }
            u32_err = eMMC_SetBusSpeed(eMMC_SPEED_HS400);
            if(eMMC_ST_SUCCESS!=u32_err)
            {
                eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: enable HS400 5.1 fail: %Xh\n", u32_err);
                return u32_err;
            }
            if(0==g_eMMCDrv.TimingTable_t.u8_SetCnt)
                eMMC_clock_setting(gau8_eMMCPLLSel_200[0]);

            break;
        #endif

    }

    // --------------------------------------
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,
            "eMMC Err: set ECSD fail: %Xh\n", u32_err);
        return u32_err;
    }

    #ifndef IP_FCIE_VERSION_5
    g_eMMCDrv.u16_Reg10_Mode &= ~BIT_SD_DATA_SYNC;
    #endif

    eMMC_pads_switch(u8_PadType);
    #if defined(ENABLE_eMMC_HS400)&&ENABLE_eMMC_HS400
    if(g_eMMCDrv.TimingTable_G_t.u8_SetCnt && u8_PadType == FCIE_eMMC_HS400)
        eMMC_FCIE_ApplyTimingSet(g_eMMCDrv.TimingTable_G_t.u8_CurSetIdx);
        else
    #endif
    if(g_eMMCDrv.TimingTable_t.u8_SetCnt)
        eMMC_FCIE_ApplyTimingSet(eMMC_TIMING_SET_MAX);


    return u32_err;
}


U32 eMMC_FCIE_EnableFastMode(U8 u8_PadType)
{
    U32 u32_err;

    if(u8_PadType == g_eMMCDrv.u8_PadType)
        return eMMC_ST_SUCCESS;

    u32_err = eMMC_LoadTimingTable(u8_PadType);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Warn: no Timing Table, %Xh\n", u32_err);
        return u32_err;
    }
    #ifndef IP_FCIE_VERSION_5
    #if defined(ENABLE_eMMC_ATOP) && ENABLE_eMMC_ATOP
    {
        U16 u16_OldClkRegVal = g_eMMCDrv.u16_ClkRegVal;
        // --------------------------------
        // ATOP: must init emmpll before pad
        eMMC_clock_setting(g_eMMCDrv.TimingTable_t.Set[0].u8_Clk);
        // --------------------------------
        eMMC_clock_setting(u16_OldClkRegVal);
    }
    #endif
    #endif
    u32_err = eMMC_FCIE_EnableFastMode_Ex(u8_PadType);
    if(eMMC_ST_SUCCESS != u32_err)
    {
        eMMC_debug(eMMC_DEBUG_LEVEL_ERROR,1,"eMMC Err: EnableDDRMode_Ex fail, %Xh\n", u32_err);
        eMMC_die("");
        return u32_err;
    }


    return eMMC_ST_SUCCESS;
}



