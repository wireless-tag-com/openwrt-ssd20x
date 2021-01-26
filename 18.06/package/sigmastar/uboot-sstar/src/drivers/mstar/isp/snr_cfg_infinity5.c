/*
* snr_cfg_infinity5.c- Sigmastar
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

#include "snr_cfg_infinity5.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define MAX_SUPPORT_SNR                     (2)
#define INVALID_PAD_SEL                     (0xFF)

#define CHIPTOP_REG_BASE                    (0x1F203C00)    // 0x101E
#define CLKGEN_REG_BASE                     (0x1F207000)    // 0x1038
#define VIFTOP_REG_BASE                     (0x1F263800)    // 0x131C

#define CHIPTOP_TO_SR0_BT656_REG_OFST       (0x15)
#define CHIPTOP_TO_SR0_BT656_REG_MASK       (0x0070)
#define CHIPTOP_TO_SR0_MIPI_REG_OFST        (0x15)
#define CHIPTOP_TO_SR0_MIPI_REG_MASK        (0x0380)
#define CHIPTOP_TO_SR0_PAR_REG_OFST         (0x15)
#define CHIPTOP_TO_SR0_PAR_REG_MASK         (0x1C00)

#define CHIPTOP_TO_SR1_BT656_REG_OFST       (0x18)
#define CHIPTOP_TO_SR1_BT656_REG_MASK       (0x0010)
#define CHIPTOP_TO_SR1_MIPI_REG_OFST        (0x18)
#define CHIPTOP_TO_SR1_MIPI_REG_MASK        (0x0180)
#define CHIPTOP_TO_SR1_PAR_REG_OFST         (0x18)
#define CHIPTOP_TO_SR1_PAR_REG_MASK         (0x0400)

#define CLKGEN_TO_CLK_SR0_REG_OFST          (0x62)
#define CLKGEN_TO_CLK_SR0_REG_MASK          (0xFF00)
#define CLKGEN_TO_CLK_SR1_REG_OFST          (0x56)
#define CLKGEN_TO_CLK_SR1_REG_MASK          (0x00FF)

#define VIFTOP_TO_SR0_RST_REG_OFST          (0x00)
#define VIFTOP_TO_SR0_RST_REG_MASK          (0x0004)
#define VIFTOP_TO_SR0_POWER_DOWN_REG_OFST   (0x00)
#define VIFTOP_TO_SR0_POWER_DOWN_REG_MASK   (0x0008)

#define VIFTOP_TO_SR1_RST_REG_OFST          (0x40)
#define VIFTOP_TO_SR1_RST_REG_MASK          (0x0004)
#define VIFTOP_TO_SR1_POWER_DOWN_REG_OFST   (0x40)
#define VIFTOP_TO_SR1_POWER_DOWN_REG_MASK   (0x0008)

#define REG_WORD(base, idx)                 (*(((volatile unsigned short*)(base))+2*(idx)))
#define REG_WRITE(base, idx, val)           REG_WORD(base, idx) = (val)
#define REG_READ(base, idx)                 REG_WORD(base, idx)

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

static int m_iSnrBusType[MAX_SUPPORT_SNR];
static int m_iSnrPadSel[MAX_SUPPORT_SNR];
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

static void _Set_SNR0_IOPad(int iPadSel)
{
    int iSnrId = 0;
    u16 u16RegReadVal = 0;
    tSensorConfig stSnrCfg;
    
    switch(m_iSnrBusType[iSnrId])
    {
    case VIF_SNR_PAD_PARALLEL_MODE:
        if (iPadSel == 1 || iPadSel == 2 || iPadSel == 6)
            m_iSnrPadSel[iSnrId] = iPadSel;
        else
            m_iSnrPadSel[iSnrId] = INVALID_PAD_SEL;

        if (m_iSnrPadSel[iSnrId] != INVALID_PAD_SEL)
        {
            stSnrCfg.tRegSnrPadSel.reg_chiptop_sr_par_mode = iPadSel;
            
            u16RegReadVal = REG_READ(CHIPTOP_REG_BASE, CHIPTOP_TO_SR0_PAR_REG_OFST);
            u16RegReadVal &= ~(CHIPTOP_TO_SR0_PAR_REG_MASK);
            u16RegReadVal |= (((u16)stSnrCfg.nRegSnrPadSel) & CHIPTOP_TO_SR0_PAR_REG_MASK);
            REG_WRITE(CHIPTOP_REG_BASE, CHIPTOP_TO_SR0_PAR_REG_OFST, u16RegReadVal);
        }
        break;
    case VIF_SNR_PAD_MIPI_1LANE_MODE:
        if (iPadSel == 1 || iPadSel == 2 || iPadSel == 3 || iPadSel == 4 || iPadSel == 5)
            m_iSnrPadSel[iSnrId] = iPadSel;
        else
            m_iSnrPadSel[iSnrId] = INVALID_PAD_SEL;

        if (m_iSnrPadSel[iSnrId] != INVALID_PAD_SEL)
        {
            stSnrCfg.tRegSnrPadSel.reg_chiptop_sr_mipi_mode = iPadSel;
            
            u16RegReadVal = REG_READ(CHIPTOP_REG_BASE, CHIPTOP_TO_SR0_MIPI_REG_OFST);
            u16RegReadVal &= ~(CHIPTOP_TO_SR0_MIPI_REG_MASK);
            u16RegReadVal |= (((u16)stSnrCfg.nRegSnrPadSel) & CHIPTOP_TO_SR0_MIPI_REG_MASK);
            REG_WRITE(CHIPTOP_REG_BASE, CHIPTOP_TO_SR0_MIPI_REG_OFST, u16RegReadVal);
        }
        break;
    case VIF_SNR_PAD_MIPI_2LANE_MODE:
        if (iPadSel == 1 || iPadSel == 2 || iPadSel == 3 || iPadSel == 4 || iPadSel == 5)
            m_iSnrPadSel[iSnrId] = iPadSel;
        else
            m_iSnrPadSel[iSnrId] = INVALID_PAD_SEL;

        if (m_iSnrPadSel[iSnrId] != INVALID_PAD_SEL)
        {
            stSnrCfg.tRegSnrPadSel.reg_chiptop_sr_mipi_mode = iPadSel;
            
            u16RegReadVal = REG_READ(CHIPTOP_REG_BASE, CHIPTOP_TO_SR0_MIPI_REG_OFST);
            u16RegReadVal &= ~(CHIPTOP_TO_SR0_MIPI_REG_MASK);
            u16RegReadVal |= (((u16)stSnrCfg.nRegSnrPadSel) & CHIPTOP_TO_SR0_MIPI_REG_MASK);
            REG_WRITE(CHIPTOP_REG_BASE, CHIPTOP_TO_SR0_MIPI_REG_OFST, u16RegReadVal);
        }
        break;
    case VIF_SNR_PAD_MIPI_4LANE_MODE:
        if (iPadSel == 1 || iPadSel == 4)
            m_iSnrPadSel[iSnrId] = iPadSel;
        else
            m_iSnrPadSel[iSnrId] = INVALID_PAD_SEL;
        
        if (m_iSnrPadSel[iSnrId] != INVALID_PAD_SEL)
        {
            stSnrCfg.tRegSnrPadSel.reg_chiptop_sr_mipi_mode = iPadSel;

            u16RegReadVal = REG_READ(CHIPTOP_REG_BASE, CHIPTOP_TO_SR0_MIPI_REG_OFST);
            u16RegReadVal &= ~(CHIPTOP_TO_SR0_MIPI_REG_MASK);
            u16RegReadVal |= (((u16)stSnrCfg.nRegSnrPadSel) & CHIPTOP_TO_SR0_MIPI_REG_MASK);
            REG_WRITE(CHIPTOP_REG_BASE, CHIPTOP_TO_SR0_MIPI_REG_OFST, u16RegReadVal);
        }
        break;
    case VIF_SNR_PAD_BT656_MODE:
        if (iPadSel == 1 || iPadSel == 2 || iPadSel == 3 || iPadSel == 4)
            m_iSnrPadSel[iSnrId] = iPadSel;
        else
            m_iSnrPadSel[iSnrId] = INVALID_PAD_SEL;

        if (m_iSnrPadSel[iSnrId] != INVALID_PAD_SEL)
        {
            stSnrCfg.tRegSnrPadSel.reg_chiptop_sr_bt656_mode = iPadSel;

            u16RegReadVal = REG_READ(CHIPTOP_REG_BASE, CHIPTOP_TO_SR0_BT656_REG_OFST);
            u16RegReadVal &= ~(CHIPTOP_TO_SR0_BT656_REG_MASK);
            u16RegReadVal |= (((u16)stSnrCfg.nRegSnrPadSel) & CHIPTOP_TO_SR0_BT656_REG_MASK);
            REG_WRITE(CHIPTOP_REG_BASE, CHIPTOP_TO_SR0_BT656_REG_OFST, u16RegReadVal);
        }
        break;
    default:
        break;
    }
}

static void _Set_SNR1_IOPad(int iPadSel)
{
    int iSnrId = 1;
    u16 u16RegReadVal = 0;
    tSensorConfig stSnrCfg;
    
    switch(m_iSnrBusType[iSnrId])
    {
    case VIF_SNR_PAD_PARALLEL_MODE:
        if (iPadSel == 1)
            m_iSnrPadSel[iSnrId] = iPadSel;
        else
            m_iSnrPadSel[iSnrId] = INVALID_PAD_SEL;

        if (m_iSnrPadSel[iSnrId] != INVALID_PAD_SEL)
        {
            stSnrCfg.tRegSnrPadSel.reg_chiptop_sr_par_mode = iPadSel;

            u16RegReadVal = REG_READ(CHIPTOP_REG_BASE, CHIPTOP_TO_SR1_PAR_REG_OFST);
            u16RegReadVal &= ~(CHIPTOP_TO_SR1_PAR_REG_MASK);
            u16RegReadVal |= (((u16)stSnrCfg.nRegSnrPadSel) & CHIPTOP_TO_SR1_PAR_REG_MASK);
            REG_WRITE(CHIPTOP_REG_BASE, CHIPTOP_TO_SR1_PAR_REG_OFST, u16RegReadVal);
        }
        break;
    case VIF_SNR_PAD_MIPI_1LANE_MODE:
        if (iPadSel == 1 || iPadSel == 2)
            m_iSnrPadSel[iSnrId] = iPadSel;
        else
            m_iSnrPadSel[iSnrId] = INVALID_PAD_SEL;

        if (m_iSnrPadSel[iSnrId] != INVALID_PAD_SEL)
        {
            stSnrCfg.tRegSnrPadSel.reg_chiptop_sr_mipi_mode = iPadSel;

            u16RegReadVal = REG_READ(CHIPTOP_REG_BASE, CHIPTOP_TO_SR1_MIPI_REG_OFST);
            u16RegReadVal &= ~(CHIPTOP_TO_SR1_MIPI_REG_MASK);
            u16RegReadVal |= (((u16)stSnrCfg.nRegSnrPadSel) & CHIPTOP_TO_SR1_MIPI_REG_MASK);
            REG_WRITE(CHIPTOP_REG_BASE, CHIPTOP_TO_SR1_MIPI_REG_OFST, u16RegReadVal);
        }
        break;
    case VIF_SNR_PAD_MIPI_2LANE_MODE:
        if (iPadSel == 1)
            m_iSnrPadSel[iSnrId] = iPadSel;
        else
            m_iSnrPadSel[iSnrId] = INVALID_PAD_SEL;

        if (m_iSnrPadSel[iSnrId] != INVALID_PAD_SEL)
        {
            stSnrCfg.tRegSnrPadSel.reg_chiptop_sr_mipi_mode = iPadSel;
            
            u16RegReadVal = REG_READ(CHIPTOP_REG_BASE, CHIPTOP_TO_SR1_MIPI_REG_OFST);
            u16RegReadVal &= ~(CHIPTOP_TO_SR1_MIPI_REG_MASK);
            u16RegReadVal |= (((u16)stSnrCfg.nRegSnrPadSel) & CHIPTOP_TO_SR1_MIPI_REG_MASK);
            REG_WRITE(CHIPTOP_REG_BASE, CHIPTOP_TO_SR1_MIPI_REG_OFST, u16RegReadVal);
        }
        break;
    case VIF_SNR_PAD_MIPI_4LANE_MODE:
        m_iSnrPadSel[iSnrId] = INVALID_PAD_SEL;
        break;
    case VIF_SNR_PAD_BT656_MODE:
        if (iPadSel == 1)
            m_iSnrPadSel[iSnrId] = iPadSel;
        else
            m_iSnrPadSel[iSnrId] = INVALID_PAD_SEL;
        
        if (m_iSnrPadSel[iSnrId] != INVALID_PAD_SEL)
        {
            stSnrCfg.tRegSnrPadSel.reg_chiptop_sr_bt656_mode = iPadSel;

            u16RegReadVal = REG_READ(CHIPTOP_REG_BASE, CHIPTOP_TO_SR1_BT656_REG_OFST);
            u16RegReadVal &= ~(CHIPTOP_TO_SR1_BT656_REG_MASK);
            u16RegReadVal |= (((u16)stSnrCfg.nRegSnrPadSel) & CHIPTOP_TO_SR1_BT656_REG_MASK);
            REG_WRITE(CHIPTOP_REG_BASE, CHIPTOP_TO_SR1_BT656_REG_OFST, u16RegReadVal);
        }
        break;
    default:
        break;
    }
}

void Set_SNR_IOPad(int iSnrId, int iPadSel)
{
    if (iSnrId == 0) {
        _Set_SNR0_IOPad(iPadSel);
    }
    else if (iSnrId == 1) {
        _Set_SNR1_IOPad(iPadSel);
    }
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
        stSnrCfg.tRegSnrClkCfg.reg_ckg_sr_mclk_select_clock_source = iSpeedIdx;
        break;
    default:
        break;
    }

    if (iSnrId == 0)
    {
        u16RegReadVal = REG_READ(CLKGEN_REG_BASE, CLKGEN_TO_CLK_SR0_REG_OFST);
        u16RegReadVal &= ~(CLKGEN_TO_CLK_SR0_REG_MASK);
        u16RegReadVal |= ((u16)stSnrCfg.nRegSnrClkCfg & CLKGEN_TO_CLK_SR0_REG_MASK);
        REG_WRITE(CLKGEN_REG_BASE, CLKGEN_TO_CLK_SR0_REG_OFST, u16RegReadVal);
    }
    else if (iSnrId == 1)
    {
        u16RegReadVal = REG_READ(CLKGEN_REG_BASE, CLKGEN_TO_CLK_SR1_REG_OFST);
        u16RegReadVal &= ~(CLKGEN_TO_CLK_SR1_REG_MASK);
        u16RegReadVal |= ((u16)stSnrCfg.nRegSnrClkCfg & CLKGEN_TO_CLK_SR1_REG_MASK);
        REG_WRITE(CLKGEN_REG_BASE, CLKGEN_TO_CLK_SR1_REG_OFST, u16RegReadVal);
    }
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
    case 1:
        stSnrCfg.tRegVifSnrCtl.reg_vif_ch_sensor_pwrdn = iVal;
        u16RegReadVal = REG_READ(VIFTOP_REG_BASE, VIFTOP_TO_SR1_POWER_DOWN_REG_OFST);
        u16RegReadVal &= ~(VIFTOP_TO_SR1_POWER_DOWN_REG_MASK);
        u16RegReadVal |= ((u16)stSnrCfg.nRegVifSnrCtl & VIFTOP_TO_SR1_POWER_DOWN_REG_MASK);
        REG_WRITE(VIFTOP_REG_BASE, VIFTOP_TO_SR1_POWER_DOWN_REG_OFST, u16RegReadVal);
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
    case 1:
        stSnrCfg.tRegVifSnrCtl.reg_vif_ch_sensor_rst = iVal;
        u16RegReadVal = REG_READ(VIFTOP_REG_BASE, VIFTOP_TO_SR1_RST_REG_OFST);
        u16RegReadVal &= ~(VIFTOP_TO_SR1_RST_REG_MASK);
        u16RegReadVal |= ((u16)stSnrCfg.nRegVifSnrCtl & VIFTOP_TO_SR1_RST_REG_MASK);
        REG_WRITE(VIFTOP_REG_BASE, VIFTOP_TO_SR1_RST_REG_OFST, u16RegReadVal);
        break;
    default:
        break;
    }
}