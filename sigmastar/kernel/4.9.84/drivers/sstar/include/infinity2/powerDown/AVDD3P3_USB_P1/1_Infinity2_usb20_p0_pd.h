/*
* 1_Infinity2_usb20_p0_pd.h- Sigmastar
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
// USB2.0 P1 power down
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00103a00), 0x05);
// [2]: PDN_REF
// [6]: R_DP_PDEN
// [7]: R_DM_PDEN
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00103a01), 0xff);
// [0]: HS_DM_PDN
// [2]: HS_TED_PDN
// [3]: HS_PREAMP_PDN
// [4]: FL_XCVR_PDN
// [6]: IREF_PDN
// [7]: REG_PDN
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00103a08), 0x80);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00103a09), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00103a10), 0x00);
