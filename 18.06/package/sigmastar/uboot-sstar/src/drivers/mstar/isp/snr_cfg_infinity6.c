/*
* snr_cfg_infinity6.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: eroy.yang <eroy.yang@sigmastar.com.tw>
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

#include "snr_cfg_infinity6.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define MAX_SUPPORT_SNR                     (2)
#define INVALID_PAD_SEL                     (0xFF)

#define CHIPTOP_REG_BASE                    (0x1F203C00)    // 0x101E
#define CLKGEN_REG_BASE                     (0x1F207000)    // 0x1038
#define VIFTOP_REG_BASE                     (0x1F263200)    // 0x1319

#define CHIPTOP_TO_SR0_BT656_REG_OFST       (0x15)
#define CHIPTOP_TO_SR0_BT656_REG_MASK       (0x0070)
#define CHIPTOP_TO_SR0_MIPI_REG_OFST        (0x15)
#define CHIPTOP_TO_SR0_MIPI_REG_MASK        (0x0380)
#define CHIPTOP_TO_SR0_PAR_REG_OFST         (0x15)
#define CHIPTOP_TO_SR0_PAR_REG_MASK         (0x1C00)

#define CHIPTOP_TO_SR_MODE_REG_OFST         (0x06)
#define CHIPTOP_TO_SR_MODE_REG_MASK         (0x0007)

#define CHIPTOP_TO_CCIR_MODE_REG_OFST         (0x0f)
#define CHIPTOP_TO_CCIR_MODE_REG_MASK         (0x0030)

#define CHIPTOP_TO_SR_MCLK_MODE_REG_OFST    (0x06)
#define CHIPTOP_TO_SR_MCLK_MODE_REG_MASK    (0x0080)

#define CHIPTOP_TO_SR_PDN_MODE_REG_OFST    (0x06)
#define CHIPTOP_TO_SR_PDN_MODE_REG_MASK    (0x0600)

#define CHIPTOP_TO_SR_RST_MODE_REG_OFST    (0x06)
#define CHIPTOP_TO_SR_RST_MODE_REG_MASK    (0x1800)


#define CLKGEN_TO_CLK_SR0_REG_OFST          (0x62)
#define CLKGEN_TO_CLK_SR0_REG_MASK          (0xFF00)

#define VIFTOP_TO_SR0_RST_REG_OFST          (0x00)
#define VIFTOP_TO_SR0_RST_REG_MASK          (0x0004)
#define VIFTOP_TO_SR0_POWER_DOWN_REG_OFST   (0x00)
#define VIFTOP_TO_SR0_POWER_DOWN_REG_MASK   (0x0008)


#define REG_WORD(base, idx)                 (*(((volatile unsigned short*)(base))+2*(idx)))
#define REG_WRITE(base, idx, val)           REG_WORD(base, idx) = (val)
#define REG_READ(base, idx)                 REG_WORD(base, idx)

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

static int m_iSnrBusType[MAX_SUPPORT_SNR];
//static int m_iSnrPadSel[MAX_SUPPORT_SNR];
static int m_iSnrMclkSel[MAX_SUPPORT_SNR];

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

void Set_SNR_BusType(int iSnrId, int iBusType)
{
    m_iSnrBusType[iSnrId] = iBusType;
}

void Set_SNR_PdnPad(int mode)
{
    u16 u16RegReadVal = 0;

    u16RegReadVal = REG_READ(CHIPTOP_REG_BASE, CHIPTOP_TO_SR_PDN_MODE_REG_OFST);
    u16RegReadVal &= ~(CHIPTOP_TO_SR_PDN_MODE_REG_MASK);
    u16RegReadVal |= ( (mode << 9) & CHIPTOP_TO_SR_PDN_MODE_REG_MASK);
    REG_WRITE(CHIPTOP_REG_BASE, CHIPTOP_TO_SR_PDN_MODE_REG_OFST, u16RegReadVal);

    return;
}

void Set_SNR_RstPad(int mode)
{
    u16 u16RegReadVal = 0;


    u16RegReadVal = REG_READ(CHIPTOP_REG_BASE, CHIPTOP_TO_SR_RST_MODE_REG_OFST);
    u16RegReadVal &= ~(CHIPTOP_TO_SR_RST_MODE_REG_MASK);
    u16RegReadVal |= ( (mode << 11) & CHIPTOP_TO_SR_RST_MODE_REG_MASK);
    REG_WRITE(CHIPTOP_REG_BASE, CHIPTOP_TO_SR_RST_MODE_REG_OFST, u16RegReadVal);

    return;
}

void Set_SNR_IOPad(int mode)
{
    u16 u16RegReadVal = 0;


    u16RegReadVal = REG_READ(CHIPTOP_REG_BASE, CHIPTOP_TO_SR_MCLK_MODE_REG_OFST); //sr mode always be 0
    u16RegReadVal &= ~(CHIPTOP_TO_SR_MCLK_MODE_REG_MASK);
    u16RegReadVal |= ( (1 << 7) & CHIPTOP_TO_SR_MCLK_MODE_REG_MASK);
    REG_WRITE(CHIPTOP_REG_BASE, CHIPTOP_TO_SR_MCLK_MODE_REG_OFST, u16RegReadVal); //mclk mode always be 1


    u16RegReadVal = REG_READ(CHIPTOP_REG_BASE, CHIPTOP_TO_SR_MODE_REG_OFST); //sr mode always be 0
    u16RegReadVal &= ~(CHIPTOP_TO_SR_MODE_REG_MASK);
    u16RegReadVal |= ( (0 << 0) & CHIPTOP_TO_SR_MODE_REG_MASK);
    REG_WRITE(CHIPTOP_REG_BASE, CHIPTOP_TO_SR_MODE_REG_OFST, u16RegReadVal);


    u16RegReadVal = REG_READ(CHIPTOP_REG_BASE, CHIPTOP_TO_CCIR_MODE_REG_OFST); //ccir mode always be 0
    u16RegReadVal &= ~(CHIPTOP_TO_CCIR_MODE_REG_MASK);
    u16RegReadVal |= ( (0 << 0) & CHIPTOP_TO_CCIR_MODE_REG_MASK);
    REG_WRITE(CHIPTOP_REG_BASE, CHIPTOP_TO_CCIR_MODE_REG_OFST, u16RegReadVal);

}

void Set_SNR_MCLK(int iSnrId, int iEnable, int iSpeedIdx)
{
    u16 u16RegReadVal = 0;
    tSensorConfig stSnrCfg;
        
    m_iSnrMclkSel[iSnrId] = iSpeedIdx;
    
    if (iEnable == 0)
        stSnrCfg.tRegSnrClkCfg.reg_ckg_sr_mclk_disable_clock = 1;
    else
        stSnrCfg.tRegSnrClkCfg.reg_ckg_sr_mclk_disable_clock = 0;
    
    stSnrCfg.tRegSnrClkCfg.reg_ckg_sr_mclk_invert_clock = 0;
    
    switch(iSpeedIdx) {
    case SNR_MCLK_27M:
    case SNR_MCLK_72M:
    case SNR_MCLK_61P7M:
    case SNR_MCLK_54M:
    case SNR_MCLK_48M:
    case SNR_MCLK_43P2M:
    case SNR_MCLK_36M:
    case SNR_MCLK_24M:
    case SNR_MCLK_21P6M:
    case SNR_MCLK_12M:
    case SNR_MCLK_5P4M:
    case SNR_MCLK_LPLL:
    case SNR_MCLK_LPLL_DIV2:
    case SNR_MCLK_LPLL_DIV4:
    case SNR_MCLK_LPLL_DIV8:
    case SNR_MCLK_ARMPLL_37P125:
        stSnrCfg.tRegSnrClkCfg.reg_ckg_sr_mclk_select_clock_source = iSpeedIdx;
        break;
    default:
        break;
    }


    u16RegReadVal = REG_READ(CLKGEN_REG_BASE, CLKGEN_TO_CLK_SR0_REG_OFST);
    u16RegReadVal &= ~(CLKGEN_TO_CLK_SR0_REG_MASK);
    u16RegReadVal |= ((u16)stSnrCfg.nRegSnrClkCfg & CLKGEN_TO_CLK_SR0_REG_MASK);
    REG_WRITE(CLKGEN_REG_BASE, CLKGEN_TO_CLK_SR0_REG_OFST, u16RegReadVal);


}

void SNR_PowerDown(int iSnrId, int iVal)
{
    u16 u16RegReadVal = 0;
    tSensorConfig stSnrCfg;
    
    switch(iSnrId)
    {
    case 0: 
        stSnrCfg.tRegVifSnrCtl.reg_vif_ch_sensor_pwrdn = iVal;
        u16RegReadVal = REG_READ(VIFTOP_REG_BASE, VIFTOP_TO_SR0_POWER_DOWN_REG_OFST);
        u16RegReadVal &= ~(VIFTOP_TO_SR0_POWER_DOWN_REG_MASK);
        u16RegReadVal |= ((u16)stSnrCfg.nRegVifSnrCtl & VIFTOP_TO_SR0_POWER_DOWN_REG_MASK);
        REG_WRITE(VIFTOP_REG_BASE, VIFTOP_TO_SR0_POWER_DOWN_REG_OFST, u16RegReadVal);
        break;
    default:
        break;
    }
}

void SNR_Reset(int iSnrId, int iVal)
{
    u16 u16RegReadVal = 0;
    tSensorConfig stSnrCfg;
    
    switch(iSnrId)
    {
    case 0:
        stSnrCfg.tRegVifSnrCtl.reg_vif_ch_sensor_rst = iVal;
        u16RegReadVal = REG_READ(VIFTOP_REG_BASE, VIFTOP_TO_SR0_RST_REG_OFST);
        u16RegReadVal &= ~(VIFTOP_TO_SR0_RST_REG_MASK);
        u16RegReadVal |= ((u16)stSnrCfg.nRegVifSnrCtl & VIFTOP_TO_SR0_RST_REG_MASK);
        REG_WRITE(VIFTOP_REG_BASE, VIFTOP_TO_SR0_RST_REG_OFST, u16RegReadVal);
        break;
    default:
        break;
    }
}
