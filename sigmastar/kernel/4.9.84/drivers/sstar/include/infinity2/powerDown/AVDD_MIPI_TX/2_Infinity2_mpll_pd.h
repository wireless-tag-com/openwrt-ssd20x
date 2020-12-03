/*
* 2_Infinity2_mpll_pd.h- Sigmastar
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
//<MStar Software>
//******************************************************************************
// MStar Software
// Copyright (c) 2010 - 2012 MStar Semiconductor, Inc. All rights reserved.
// All software, firmware and related documentation herein ("MStar Software") are
// intellectual property of MStar Semiconductor, Inc. ("MStar") and protected by
// law, including, but not limited to, copyright law and international treaties.
// Any use, modification, reproduction, retransmission, or republication of all
// or part of MStar Software is expressly prohibited, unless prior written
// permission has been granted by MStar.
//
//******************************************************************************
//<MStar Software>
// MPLL power down
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00110b03), 0x1f);
//[0]: PD_MPLL
//[1]: PD_MPLL_CLK_ADC_VCO_DIV2
//[2]: PD_MPLL_CLK_ADC_VCO_DIV2_2
//[3]: PD_MPLL_CLK_ADC_VCO_DIV2_3
//[4]: PD_DIGCLK
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00110b04), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00110b08), 0x00);
//[0]: EN_MPLL_TEST
//[1]: EN_MPLL_OV_CP_SW
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00110b09), 0x00);
//[2]: EN_MPLL_XTAL
//[7]: EN_MPLL_PRDT