/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#define _HAL_DISP_PQ_C_

#include "drv_disp_os.h"
#include "hal_disp_common.h"
#include "disp_debug.h"
#include "hal_disp_util.h"
#include "hal_disp_reg.h"
#include "hal_disp_pq.h"
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
HalDispPqHwContext_t gstHwPqCtx;

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

// ----------------- BW Extension -----------------
void _HalDispPqSetBw2sbri(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1A_L, u16Val, 0x00FF);
}

void _HalDispPqSetBleEn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_16_L, u16Val ? 0x0001 : 0x0000 , 0x0001);
}

void _HalDispPqSetWleEn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_16_L, u16Val ? 0x0002 : 0x0000 , 0x0002);
}


void _HalDispPqSetBlackStart(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_17_L, u16Val, 0x007F);
}

void _HalDispPqSetWhiteStart(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_16_L, (u16Val & 0x007F) << 8, 0x7F00);
}

void _HalDispPqSetBlackSlop(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_0A_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetWhiteSlop(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_0A_L, (u16Val & 0x00FF), 0x00FF);
}

// ----------------- Lpf -----------------
void _HalDispPqSetLpfY(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1C_L, (u16Val & 0x03) << 2, 0x000C);
}

void _HalDispPqSetLpfC(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1C_L, (u16Val & 0x03) << 4, 0x0030);
}

// ----------------- DLC/DCR -----------------
void _HalDispPqSetDcrEn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_4C_L, u16Val ? 0x0100 : 0x0000, 0x0100);
}

void _HalDispPqSetDcrOffset(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_55_L, u16Val, 0x0FFF);
}

void _HalDispPqSetYGain(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_4D_L, u16Val, 0x00FF);
}

void _HalDispPqSetYGainOffset(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_4F_L, u16Val, 0x0FFF);
}

void _HalDispPqSetDlcEn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_56_L, u16Val ? 0x0800 : 0x0000, 0x0800);
}

void _HalDispPqSetDlcGain(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_51_L, u16Val, 0x00FF);
}

void _HalDispPqSetDlcOffset(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_50_L, u16Val, 0x0FFF);
}

void _HalDispPqSetDlcHact(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_6B_L, u16Val, 0x0FFF);
}

void _HalDispPqSetDlcHblank(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_6D_L, u16Val, 0x00FF);
}

void _HalDispPqSetDlcVact(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_6C_L, u16Val, 0x0FFF);
}

void _HalDispPqSetDlcPwmDuty(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_53_L, u16Val, 0xFFFF);
}

void _HalDispPqSetDlcPwmPeriod(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_52_L, u16Val, 0xFFFF);
}

//----------------- HCoring -----------------
void _HalDispPqHCoringSetYBand1En( u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1D_L, u16Val ? 0x0001 : 0x0000, 0x0001);
}

void _HalDispPqHCoringSetYBand2En( u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1D_L, u16Val ? 0x0002 : 0x0000, 0x0002);
}

void _HalDispPqHCoringSetYDitherEn( u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1D_L, u16Val ? 0x0004 : 0x0000, 0x0004);
}

void _HalDispPqHCoringSetYTableStep( u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1D_L, (u16Val & 0x07) << 4, 0x0070);
}

void _HalDispPqHCoringSetCBand1En( u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1D_L, u16Val ? 0x0100 : 0x0000, 0x0100);
}

void _HalDispPqHCoringSetCBand2En( u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1D_L, u16Val ? 0x0200 : 0x0000, 0x0200);
}

void _HalDispPqHCoringSetCDitherEn( u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1D_L, u16Val ? 0x0400 : 0x0000, 0x0400);
}

void _HalDispPqHCoringSetCTableStep( u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1D_L, (u16Val & 0x07) << 12, 0x7000);
}

void _HalDispPqHCoringSetPcMode(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1D_L, u16Val ? 0x0008 : 0x0000, 0x0008);
}

void _HalDispPqHCoringSetHighPassEn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1D_L, u16Val ? 0x0080 : 0x0000, 0x0080);
}

void _HalDispPqHCoringSetTable(u16 u16Table0, u16 u16Table1, u16 u16Table2, u16 u16Table3)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1E_L, u16Table0 << 0, 0x00FF);
    W2BYTEMSK(REG_DISP_TOP_MACE_1E_L, u16Table1 << 8, 0xFF00);
    W2BYTEMSK(REG_DISP_TOP_MACE_1F_L, u16Table2 << 0, 0x00FF);
    W2BYTEMSK(REG_DISP_TOP_MACE_1F_L, u16Table3 << 8, 0xFF00);
}

// ----------------- Peaking -----------------
void _HalDispPqPeakingSetPeakEn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_12_L, u16Val ? 0x0400 : 0x0000, 0x0400);
}

void _HalDispPqPeakingSetLtiEn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_12_L, u16Val ? 0x0800 : 0x0000, 0x0800);
}

void _HalDispPqPeakingSetLtiMedianFilterOn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_12_L, u16Val ? 0x1000 : 0x0000, 0x1000);
}

void _HalDispPqPeakingSetCtiEn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_12_L, u16Val ? 0x2000 : 0x0000, 0x2000);
}

void _HalDispPqPeakingSetCtiMedianFilterOn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_12_L, u16Val ? 0x4000 : 0x0000, 0x4000);
}

void _HalDispPqPeakingSetDiffAdapEn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_12_L, u16Val ? 0x8000 : 0x0000, 0x8000);
}

void _HalDispPqPeakingSetBand1Coef(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_13_L, u16Val, 0x003F);
}

void _HalDispPqPeakingSetBand1Step(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_13_L, (u16Val & 0x0003) << 6, 0x00C0);
}

void _HalDispPqPeakingSetBand2Coef(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_13_L, (u16Val & 0x003F) << 8, 0x3F00);
}

void _HalDispPqPeakingSetBand2Step(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_13_L, (u16Val & 0x0003) << 14, 0xC000);
}

void _HalDispPqPeakingSetLtiCoef(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_14_L, u16Val, 0x003F);
}

void _HalDispPqPeakingSetLtiStep(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_14_L, (u16Val & 0x0003) << 6, 0x00C0);
}

void _HalDispPqPeakingSetPeakTerm1Sel(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_14_L, (u16Val & 0x0003) << 8, 0x0300);
}

void _HalDispPqPeakingSetPeakTerm2Sel(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_14_L, (u16Val & 0x0003) << 10, 0x0C00);
}

void _HalDispPqPeakingSetPeakTerm3Sel(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_14_L, (u16Val & 0x0003) << 12, 0x3000);
}

void _HalDispPqPeakingSetPeakTerm4Sel(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_14_L, (u16Val & 0x0003) << 14, 0xC000);
}

void _HalDispPqPeakingSetCoringTh1(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_15_L, u16Val, 0x000F);
}

void _HalDispPqPeakingSetCoringTh2(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_15_L, (u16Val & 0x000F) << 4, 0x00F0);
}

void _HalDispPqPeakingSetCtiCoef(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_15_L, (u16Val & 0x001F) << 8, 0x1F00);
}

void _HalDispPqPeakingSetCtiStep(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_15_L, (u16Val & 0x0003) << 13, 0x6000);
}

void _HalDispPqPeakingSetBand1PosLimitTh(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_18_L, (u16Val & 0x00FF) << 0, 0x00FF);
}

void _HalDispPqPeakingSetBand1NegLimitTh(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_18_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqPeakingSetBand2PosLimitTh(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_19_L, (u16Val & 0x00FF) << 0, 0x00FF);
}

void _HalDispPqPeakingSetBand2NegLimitTh(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_19_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqPeakingSetPosLimitTh(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1B_L, (u16Val & 0x00FF) << 0, 0x00FF);
}

void _HalDispPqPeakingSetNegLimitTh(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1B_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqPeakingSetBand2DiffAdapEn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1C_L, u16Val ? 0x0001 : 0x0000, 0x0001);
}

void _HalDispPqPeakingSetBand1DiffAdapEn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_1C_L, u16Val ? 0x0002 : 0x0000, 0x0002);
}

void _HalDispPqSetFccCbT1(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_02_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccCrT1(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_02_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetFccCbT2(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_03_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccCrT2(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_03_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetFccCbT3(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_04_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccCrT3(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_04_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetFccCbT4(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_05_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccCrT4(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_05_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetFccCbT5(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_06_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccCrT5(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_06_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetFccCbT6(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_07_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccCrT6(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_07_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetFccCbT7(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_08_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccCrT7(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_08_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetFccCbT8(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_09_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccCrT8(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_09_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetFccCbCrD1DD1U(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_0B_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccCbCrD2DD2U(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_0B_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetFccCbCrD3DD3U(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_0C_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccCbCrD4DD4U(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_0C_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetFccCbCrD5DD5U(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_0D_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccCbCrD6DD6U(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_0D_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetFccCbCrD7DD7U(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_0E_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccCbCrD8DD8U(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_0E_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetFccCbCrD9(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_0F_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccKT2KT1(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_10_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccKT4KT3(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_10_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetFccKT6KT5(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_11_L, u16Val, 0x00FF);
}

void _HalDispPqSetFccKT8KT7(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_11_L, (u16Val & 0x00FF) << 8, 0xFF00);
}

void _HalDispPqSetFccEn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_MACE_12_L, u16Val, 0x00FF);
}

void _HalDispPqSetGammaR(u8 *pu8R)
{
    u32 i;
    u16 u16Val;

    for(i=0; i<32; i+=2)
    {
        u16Val = pu8R[i] | ((u16)pu8R[i+1]) << 8;
        W2BYTE(REG_DISP_TOP_GAMMA_01_L + i, u16Val);
    }

    W2BYTEMSK(REG_DISP_TOP_GAMMA_11_L,  pu8R[32], 0x00FF);
}

void _HalDispPqSetGammaG(u8 *pu8G)
{
    u32 i;
    u16 u16Val;

    for(i=0; i<32; i+=2)
    {
        u16Val = pu8G[i] | ((u16)pu8G[i+1]) << 8;
        W2BYTE(REG_DISP_TOP_GAMMA_12_L + i, u16Val);
    }

    W2BYTEMSK(REG_DISP_TOP_GAMMA_22_L,  pu8G[32], 0x00FF);
}

void _HalDispPqSetGammaB(u8 *pu8B)
{
    u32 i;
    u16 u16Val;

    for(i=0; i<32; i+=2)
    {
        u16Val = pu8B[i] | ((u16)pu8B[i+1]) << 8;
        W2BYTE(REG_DISP_TOP_GAMMA_23_L + i, u16Val);
    }

    W2BYTEMSK(REG_DISP_TOP_GAMMA_33_L,  pu8B[32], 0x00FF);
}

void _HalDispPqSetGammaEn(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_GAMMA_00_L, u16Val ? 0x0001 : 0x0000, 0x0001);
}

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void HalDispPqGetHwCtx(HalDispPqHwContext_t **pstPqCtx)
{
    *pstPqCtx = &gstHwPqCtx;
}

void HalDispPqSetHwCtxFlag(HalDispPqFlag_e enFlag)
{
    gstHwPqCtx.enFlag |= enFlag;
}

void HalDispPqClearHwCtxFlag(HalDispPqFlag_e enFlag)
{
    gstHwPqCtx.enFlag &= ~enFlag;
}

void HalDispPqSetBwExtensionConfig(HalDispPqBwExtensionConfig_t *pstBwCfg)
{
    if(pstBwCfg->bUpdate)
    {
        DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d\n", __FUNCTION__, __LINE__);
        HalDispPqSetHwCtxFlag(E_HAL_DISP_PQ_FLAG_BW);
        _HalDispPqSetBw2sbri(pstBwCfg->u8Bw2sbri);
        _HalDispPqSetBleEn(pstBwCfg->u8BleEn);
        _HalDispPqSetWleEn(pstBwCfg->u8WleEn);
        _HalDispPqSetBlackSlop(pstBwCfg->u8BlackSlop);
        _HalDispPqSetBlackStart(pstBwCfg->u8BlackStart);
        _HalDispPqSetWhiteSlop(pstBwCfg->u8WhiteSlop);
        _HalDispPqSetWhiteStart(pstBwCfg->u8WhiteStart);
        pstBwCfg->bUpdate = 0;
    }
}

void HalDispPqSetLpfConfig(HalDispPqLpfConfig_t *pstLpfCfg)
{
    if(pstLpfCfg->bUpdate)
    {
        DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d\n", __FUNCTION__, __LINE__);
        HalDispPqSetHwCtxFlag(E_HAL_DISP_PQ_FLAG_LPF);
        _HalDispPqSetLpfY(pstLpfCfg->u8LpfY);
        _HalDispPqSetLpfC(pstLpfCfg->u8LpfC);
        pstLpfCfg->bUpdate = 0;
    }
}

void HalDispPqSetDlcDcrConfig(HalDispPqDlcDcrConfig_t *pstDlcDcrCfg)
{
    if(pstDlcDcrCfg->bUpdate)
    {
        DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d\n", __FUNCTION__, __LINE__);
        HalDispPqSetHwCtxFlag(E_HAL_DISP_PQ_FLAG_DLC_DCR);

        _HalDispPqSetDcrEn(pstDlcDcrCfg->u16DcrEn);
        _HalDispPqSetDcrOffset(pstDlcDcrCfg->u16DcrOffset);

        _HalDispPqSetYGain(pstDlcDcrCfg->u16YGain);
        _HalDispPqSetYGainOffset(pstDlcDcrCfg->u16YGainOffset);

        _HalDispPqSetDlcHact(pstDlcDcrCfg->u16Hactive);
        _HalDispPqSetDlcVact(pstDlcDcrCfg->u16Vactive);
        _HalDispPqSetDlcHblank(pstDlcDcrCfg->u16Hblank);
        _HalDispPqSetDlcPwmDuty(pstDlcDcrCfg->u16PwmDuty);
        _HalDispPqSetDlcPwmPeriod(pstDlcDcrCfg->u16PwmPeriod);

        _HalDispPqSetDlcEn(pstDlcDcrCfg->u16DlcEn);
        _HalDispPqSetDlcGain(pstDlcDcrCfg->u16DlcGain);
        _HalDispPqSetDlcOffset(pstDlcDcrCfg->u16DlcOffset);

        pstDlcDcrCfg->bUpdate = 0;
    }
}

void HalDispPqSetHCoringConfig(HalDispPqHCoringConfig_t *pstHCoringCfg)
{
    if(pstHCoringCfg->bUpdate)
    {
        DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d\n", __FUNCTION__, __LINE__);
        HalDispPqSetHwCtxFlag(E_HAL_DISP_PQ_FLAG_HCORING);
        _HalDispPqHCoringSetYBand1En(pstHCoringCfg->u8YBand1HCoringEn);
        _HalDispPqHCoringSetYBand2En(pstHCoringCfg->u8YBand2HCoringEn);
        _HalDispPqHCoringSetYDitherEn(pstHCoringCfg->u8HCoringYDither_En);
        _HalDispPqHCoringSetYTableStep(pstHCoringCfg->u8YTableStep);

        _HalDispPqHCoringSetCBand1En(pstHCoringCfg->u8CBand1HCoringEn);
        _HalDispPqHCoringSetCBand2En(pstHCoringCfg->u8CBand2HCoringEn);
        _HalDispPqHCoringSetCDitherEn(pstHCoringCfg->u8HCoringCDither_En);
        _HalDispPqHCoringSetCTableStep(pstHCoringCfg->u8CTableStep);

        _HalDispPqHCoringSetPcMode(pstHCoringCfg->u8PcMode);
        _HalDispPqHCoringSetHighPassEn(pstHCoringCfg->u8HighPassEn);

        _HalDispPqHCoringSetTable(
            pstHCoringCfg->u8CoringTable0, pstHCoringCfg->u8CoringTable1,
            pstHCoringCfg->u8CoringTable2, pstHCoringCfg->u8CoringTable3);
        pstHCoringCfg->bUpdate = 0;
    }
}


void HalDispPqSetPeaking(HalDispPqPeakingConfig_t *pstPeakingCfg)
{

    if(pstPeakingCfg->bUpdate)
    {
        DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d, Coef1=%x, Coef2:%x\n",
            __FUNCTION__, __LINE__, pstPeakingCfg->u8Band1Coef, pstPeakingCfg->u8Band2Coef);

        HalDispPqSetHwCtxFlag(E_HAL_DISP_PQ_FLAG_PEAKING);
        _HalDispPqPeakingSetPeakEn(pstPeakingCfg->u8PeakingEn);
        _HalDispPqPeakingSetLtiEn(pstPeakingCfg->u8LtiEn);
        _HalDispPqPeakingSetLtiMedianFilterOn(pstPeakingCfg->u8LtiMediaFilterOn);
        _HalDispPqPeakingSetCtiEn(pstPeakingCfg->u8CtiEn);
        _HalDispPqPeakingSetCtiMedianFilterOn(pstPeakingCfg->u8CtiMediaFilterOn);
        _HalDispPqPeakingSetDiffAdapEn(pstPeakingCfg->u8DiffAdapEn);
        _HalDispPqPeakingSetBand1Coef(pstPeakingCfg->u8Band1Coef);
        _HalDispPqPeakingSetBand1Step(pstPeakingCfg->u8Band1Step);
        _HalDispPqPeakingSetBand2Coef(pstPeakingCfg->u8Band2Coef);
        _HalDispPqPeakingSetBand2Step(pstPeakingCfg->u8Band2Step);
        _HalDispPqPeakingSetLtiCoef(pstPeakingCfg->u8LtiCoef);
        _HalDispPqPeakingSetLtiStep(pstPeakingCfg->u8LtiStep);
        _HalDispPqPeakingSetPeakTerm1Sel(pstPeakingCfg->u8PeakingTerm1Sel);
        _HalDispPqPeakingSetPeakTerm2Sel(pstPeakingCfg->u8PeakingTerm2Sel);
        _HalDispPqPeakingSetPeakTerm3Sel(pstPeakingCfg->u8PeakingTerm3Sel);
        _HalDispPqPeakingSetPeakTerm4Sel(pstPeakingCfg->u8PeakingTerm4Sel);
        _HalDispPqPeakingSetCoringTh1(pstPeakingCfg->u8CoringTh1);
        _HalDispPqPeakingSetCoringTh2(pstPeakingCfg->u8CoringTh2);
        _HalDispPqPeakingSetCtiCoef(pstPeakingCfg->u8CtiCoef);
        _HalDispPqPeakingSetCtiStep(pstPeakingCfg->u8CtiStep);
        _HalDispPqPeakingSetBand1PosLimitTh(pstPeakingCfg->u8Band1PosLimitTh);
        _HalDispPqPeakingSetBand1NegLimitTh(pstPeakingCfg->u8Band1NegLimitTh);
        _HalDispPqPeakingSetBand2PosLimitTh(pstPeakingCfg->u8Band2PosLimitTh);
        _HalDispPqPeakingSetBand2NegLimitTh(pstPeakingCfg->u8Band2NegLimitTh);
        _HalDispPqPeakingSetPosLimitTh(pstPeakingCfg->u8PosLimitTh);
        _HalDispPqPeakingSetNegLimitTh(pstPeakingCfg->u8NegLimitTh);
        _HalDispPqPeakingSetBand2DiffAdapEn(pstPeakingCfg->u8Band2DiffAdapEn);
        _HalDispPqPeakingSetBand1DiffAdapEn(pstPeakingCfg->u8Band1DiffAdapEn);
        pstPeakingCfg->bUpdate = 0;
    }
}

void HalDispPqSetFccConfig(HalDispPqFccConfig_t *pstFccCfg)
{
    if(pstFccCfg->bUpdate)
    {
        DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d\n", __FUNCTION__, __LINE__);
        HalDispPqSetHwCtxFlag(E_HAL_DISP_PQ_FLAG_FCC);
        _HalDispPqSetFccCbT1(pstFccCfg->u8Cb_T1);
        _HalDispPqSetFccCrT1(pstFccCfg->u8Cr_T1);
        _HalDispPqSetFccCbT2(pstFccCfg->u8Cb_T2);
        _HalDispPqSetFccCrT2(pstFccCfg->u8Cr_T2);
        _HalDispPqSetFccCbT3(pstFccCfg->u8Cb_T3);
        _HalDispPqSetFccCrT3(pstFccCfg->u8Cr_T3);
        _HalDispPqSetFccCbT4(pstFccCfg->u8Cb_T4);
        _HalDispPqSetFccCrT4(pstFccCfg->u8Cr_T4);
        _HalDispPqSetFccCbT5(pstFccCfg->u8Cb_T5);
        _HalDispPqSetFccCrT5(pstFccCfg->u8Cr_T5);
        _HalDispPqSetFccCbT6(pstFccCfg->u8Cb_T6);
        _HalDispPqSetFccCrT6(pstFccCfg->u8Cr_T6);
        _HalDispPqSetFccCbT7(pstFccCfg->u8Cb_T7);
        _HalDispPqSetFccCrT7(pstFccCfg->u8Cr_T7);
        _HalDispPqSetFccCbT8(pstFccCfg->u8Cb_T8);
        _HalDispPqSetFccCrT8(pstFccCfg->u8Cr_T8);
        _HalDispPqSetFccCbCrD1DD1U(pstFccCfg->u8CbCr_D1D_D1U);
        _HalDispPqSetFccCbCrD2DD2U(pstFccCfg->u8CbCr_D2D_D2U);
        _HalDispPqSetFccCbCrD3DD3U(pstFccCfg->u8CbCr_D3D_D3U);
        _HalDispPqSetFccCbCrD4DD4U(pstFccCfg->u8CbCr_D4D_D4U);
        _HalDispPqSetFccCbCrD5DD5U(pstFccCfg->u8CbCr_D5D_D5U);
        _HalDispPqSetFccCbCrD6DD6U(pstFccCfg->u8CbCr_D6D_D6U);
        _HalDispPqSetFccCbCrD7DD7U(pstFccCfg->u8CbCr_D7D_D7U);
        _HalDispPqSetFccCbCrD8DD8U(pstFccCfg->u8CbCr_D8D_D8U);
        _HalDispPqSetFccCbCrD9(pstFccCfg->u8CbCr_D9);
        _HalDispPqSetFccKT2KT1(pstFccCfg->u8K_T2_K_T1);
        _HalDispPqSetFccKT4KT3(pstFccCfg->u8K_T4_K_T3);
        _HalDispPqSetFccKT6KT5(pstFccCfg->u8K_T6_K_T5);
        _HalDispPqSetFccKT8KT7(pstFccCfg->u8K_T8_K_T7);
        _HalDispPqSetFccEn(pstFccCfg->u8En);
        pstFccCfg->bUpdate = 0;
    }
}

void HalDispPqSetGammaConfig(HalDispPqGammaConfig_t *pstGammaCfg)
{
    if(pstGammaCfg->bUpdate)
    {
        DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d\n", __FUNCTION__, __LINE__);
        HalDispPqSetHwCtxFlag(E_HAL_DISP_PQ_FLAG_GAMMA);
        _HalDispPqSetGammaR(pstGammaCfg->u8R);
        _HalDispPqSetGammaG(pstGammaCfg->u8G);
        _HalDispPqSetGammaB(pstGammaCfg->u8B);
        _HalDispPqSetGammaEn(pstGammaCfg->u8En);
        pstGammaCfg->bUpdate = 0;
    }
}

