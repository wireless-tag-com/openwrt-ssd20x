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

#define _HAL_DISP_SCALER_E_C_

#include "drv_disp_os.h"
#include "hal_disp_common.h"
#include "disp_debug.h"
#include "hal_disp_util.h"
#include "hal_disp_reg.h"
#include "hal_disp_chip.h"
#include "hal_disp_st.h"
#include "hal_disp_vga_timing_tbl.h"
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


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void HalDispSetMaceSrc(bool bExtVideo)
{
    W2BYTEMSK(REG_DISP_TOP_07_L, bExtVideo ? 1 : 0, 0x0001);
}

void HalDispSetPatGenMd(u8 u8Val)
{
    W2BYTEMSK(REG_DISP_TOP_07_L, u8Val << 1,  0x000E);
}


#if defined(HDMITX_VGA_SUPPORTED)

void HalDispSetSwReste(u8 u8Val)
{
    W2BYTEMSK(REG_DISP_TOP_00_L, ((u16)u8Val) << 8, 0xFF00);
}

void HalDispSetDacReset(u8 u8Val)
{
    W2BYTEMSK(REG_DISP_TOP_30_L, u8Val ? 0x0001 : 0x0000, 0x0001);
}


void HalDispSetFpllEn(u8 u8Val)
{
    W2BYTEMSK(REG_DISP_TOP_15_L, u8Val ? 0x0001: 0x0000, 0x0001);
}

void HalDispSetDacMux(u8 u8Val)
{
    W2BYTEMSK(REG_CHIPTOP_0A_L, u8Val ? 0x0001 : 0x0000, 0x0001);
}

void HalDispSetTgenHtt(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_11_L, u16Val, 0x1FFF);
}

void HalDispSetTgenVtt(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_12_L, u16Val, 0x1FFF);
}

void HalDispSetTgenHsyncSt(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_13_L, u16Val, 0x1FFF);
}

void HalDispSetTgenHsyncEnd(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_14_L, u16Val, 0x1FFF);
}

void HalDispSetTgenVsyncSt(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_15_L, u16Val, 0x1FFF);
}

void HalDispSetTgenVsyncEnd(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_16_L, u16Val, 0x1FFF);
}

void HalDispSetTgenHfdeSt(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_17_L, u16Val, 0x1FFF);
}

void HalDispSetTgenHfdeEnd(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_18_L, u16Val, 0x1FFF);
}

void HalDispSetTgenVfdeSt(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_19_L, u16Val, 0x1FFF);
}

void HalDispSetTgenVfdeEnd(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_1A_L, u16Val, 0x1FFF);
}

void HalDispSetTgenHdeSt(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_1C_L, u16Val, 0x1FFF);
}

void HalDispSetTgenHdeEnd(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_1D_L, u16Val, 0x1FFF);
}

void HalDispSetTgenVdeSt(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_1E_L, u16Val, 0x1FFF);
}

void HalDispSetTgenVdeEnd(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_1F_L, u16Val, 0x1FFF);
}

void HalDispSetTgenDacHsyncSt(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_23_L, u16Val, 0x1FFF);
}

void HalDispSetTgenDacHsyncEnd(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_24_L, u16Val, 0x1FFF);
}

void HalDispSetTgenDacHdeSt(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_27_L, u16Val, 0x1FFF);
}

void HalDispSetTgenDacHdeEnd(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_28_L, u16Val, 0x1FFF);
}

void HalDispSetTgenDacVdeSt(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_29_L, u16Val, 0x1FFF);
}

void HalDispSetTgenDacVdeEnd(u16 u16Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_2A_L, u16Val, 0x1FFF);
}

void HalDispDumpRegTab(u8 *pData, u16 u16RegNum, u16 u16DataSize, u8 u8DataOffset)
{
    u16 i, j;

    for(i=0; i< u16RegNum; i++)
    {
        u32 u32Addr;
        u8 u8Mask, u8Value;

        j = i *  (REG_ADDR_SIZE+REG_MASK_SIZE+u16DataSize);

        u32Addr = ((u32)pData[j]) << 16 | ((u32)pData[j+1]) << 8 | (u32)(pData[j+2]);
        u8Mask  = pData[j+3];
        u8Value = pData[j+4+u8DataOffset];

        //DISP_ERR("0x%06x 0x%02x 0x%02x\n", u32Addr, u8Value, u8Mask);
        WBYTEMSK(u32Addr, u8Value, u8Mask);
    }
}


void HalDispSetDacTrimming(u16 u16R, u16 u16G, u16 u16B)
{
    DISP_DBG(DISP_DBG_LEVEL_COLOR, "%s %d DacTrim(0x%x, 0x%x, 0x%x)\n", __FUNCTION__, __LINE__, u16B, u16G, u16B);

    W2BYTEMSK(REG_DISP_DAC_1A_L, (u16B & 0x007F), 0x007F);
    W2BYTEMSK(REG_DISP_DAC_1B_L, (u16G & 0x007F), 0x007F);
    W2BYTEMSK(REG_DISP_DAC_1C_L, (u16R & 0x007F), 0x007F);
}

void HalDispGetDacTriming(u16 *pu16R, u16 *pu16G, u16 *pu16B)
{
    *pu16B = R2BYTE(REG_DISP_DAC_1A_L) & 0x007F;
    *pu16G = R2BYTE(REG_DISP_DAC_1B_L) & 0x007F;
    *pu16R = R2BYTE(REG_DISP_DAC_1C_L) & 0x007F;
}

void HalDispSetVgaHpdInit(void)
{
    W2BYTEMSK(REG_DISP_TOP_34_L, 0x0001, 0x0001); // dac_hpd_en
    W2BYTEMSK(REG_DISP_TOP_37_L, 0x0110, 0x0FFF); // dac_hpd_len
    W2BYTEMSK(REG_DISP_TOP_38_L, 0x00FF, 0x00FF); // dac_hpd_delay
    W2BYTEMSK(REG_DISP_TOP_39_L, 0x0050, 0x00FF); // dac_hpd_data_code
    W2BYTEMSK(REG_DISP_DAC_1E_L, 0x0002, 0x0003); // threshold,
    W2BYTEMSK(REG_DISP_DAC_14_L, 0x0007, 0x000F); // [0]:reg_en_hd_dac_b_det, [1]:reg_en_hd_dac_g_det [2]:reg_en_hd_dac_r_det
}


void HalDispSetHdmitxSsc(u16 u16Step, u16 u16Span)
{
    W2BYTEMSK(REG_HDMITX_ATOP_05_L, u16Step, 0x07FF);
    W2BYTEMSK(REG_HDMITX_ATOP_06_L, u16Span, 0x7FFF);
}

void HalDispSetClkHdmi(bool bEn, u32 u32ClkRate)
{
    u16 u16RegVal;

    u16RegVal = (u32ClkRate <= 0) ? 0x00 << 2 :
                (u32ClkRate <= 1) ? 0x01 << 2 :
                                    0x00 << 2;

    u16RegVal |= bEn ? 0x0000 : 0x0001;
    W2BYTEMSK(REG_SC_CTRL_35_L, u16RegVal, 0x000F);
}

void HalDispGetClkHdmi(bool *pbEn, u32 *pu32ClkRate)
{
    u16 u16RegVal;
    u16RegVal = R2BYTE(REG_SC_CTRL_35_L);

    *pbEn = u16RegVal & 0x0001 ? 0 : 1;
    *pu32ClkRate = ((u16RegVal >> 2) & 0x03) == 0x0000 ?  0 :
                   ((u16RegVal >> 2) & 0x03) == 0x0001 ?  1 :
                                                          999;
}

void HalDispSetClkDac(bool bEn, u32 u32ClkRate)
{
    u16 u16RegVal;

    u16RegVal = (u32ClkRate <= 0) ? 0x00 << 2 :
                (u32ClkRate <= 1) ? 0x01 << 2 :
                                    0x00 << 2;
    u16RegVal |= bEn ? 0x0000 : 0x0001;
    W2BYTEMSK(REG_SC_CTRL_36_L, u16RegVal, 0x000F);
}

void HalDispGetClkDac(bool *pbEn, u32 *pu32ClkRate)
{
    u16 u16RegVal;
    u16RegVal = R2BYTE(REG_SC_CTRL_36_L);

    *pbEn = u16RegVal & 0x0001 ? 0 : 1;
    *pu32ClkRate = ((u16RegVal >> 2) & 0x03) == 0x0000 ?  0 :
                   ((u16RegVal >> 2) & 0x03) == 0x0001 ?  1 :
                                                          999;
}
#endif

void HalDispSetClkDisp432(bool bEn, u32 u32ClkRate)
{
    u16 u16RegVal;

    u16RegVal = (u32ClkRate <= CLK_MHZ(432)) ? 0x00 << 2 : 0x01<<2;

    u16RegVal |= bEn ? 0x0000 : 0x0001;
    W2BYTEMSK(REG_CLKGEN_53_L, u16RegVal, 0x000F);
}

void HalDispGetClkDisp432(bool *pbEn, u32 *pu32ClkRate)
{
    u16 u16RegVal;
    u16RegVal = R2BYTE(REG_CLKGEN_53_L);

    *pbEn = u16RegVal & 0x0001 ? 0 : 1;
    *pu32ClkRate = ((u16RegVal >> 2) & 0x03) == 0x0000 ?  CLK_MHZ(432) : 999;
}

void HalDispSetClkDisp216(bool bEn, u32 u32ClkRate)
{
    u16 u16RegVal;

    u16RegVal = (u32ClkRate <= CLK_MHZ(108)) ? 0x01 << 10 :
                (u32ClkRate <= CLK_MHZ(216)) ? 0x00 << 10 :
                                               0x00;

    u16RegVal |= bEn ? 0x0000 : 0x0100;
    W2BYTEMSK(REG_CLKGEN_53_L, u16RegVal, 0x0F00);
}

void HalDispGetClkDisp216(bool *pbEn, u32 *pu32ClkRate)
{
    u16 u16RegVal;
    u16RegVal = R2BYTE(REG_CLKGEN_53_L);

    *pbEn = u16RegVal & 0x0100 ? 0 : 1;
    *pu32ClkRate = ((u16RegVal >> 10) & 0x03) == 0x0000 ?  CLK_MHZ(216) :
                   ((u16RegVal >> 10) & 0x03) == 0x0001 ?  CLK_MHZ(108) :
                                                           0;
}

void HalDispSetFrameColor(u8 u8R, u8 u8G, u8 u8B)
{
    u16 u16Val;

    u16Val = (u8B & 0x1F) | (u16)(u8G & 0x1F) << 5 | (u16)(u8R & 0x1F) << 10;
    W2BYTEMSK(REG_DISP_TOP_OP2_1B_L, u16Val, 0x7FFF);
}

void HalDispSetFrameColorForce(u8 u8Val)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_1B_L, u8Val ? 0x8000 : 0x0000, 0x8000);
}

void HalDispSetDispWinColor(u8 u8R, u8 u8G, u8 u8B)
{
    u16 u16Val;

    u16Val = (u8B & 0x1F) | (u16)(u8G & 0x1F) << 5 | (u16)(u8R & 0x1F) << 10;
    W2BYTEMSK(REG_DISP_TOP_OP2_20_L, u16Val, 0x7FFF);
}

void HalDispSetDispWinColorForce(bool bEn)
{
    W2BYTEMSK(REG_DISP_TOP_OP2_20_L, bEn ? 0x8000 : 0x0000, 0x8000);
}

