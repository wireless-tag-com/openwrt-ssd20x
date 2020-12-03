/*
* ETH_TX_10T_ALL_ONE.h- Sigmastar
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
// Reset
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00000e60), 0x40);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00101ea1), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000032fc), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000032fd), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000033a1), 0x90);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000032cc), 0x40);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000032bb), 0x04);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0000338f), 0x02);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000033d5), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000033d9), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000033f7), 0x02);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000033fb), 0x02);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000033fd), 0x02);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0000333a), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000033f1), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0000338a), 0x01);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000032c4), 0x44);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00003380), 0x30);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0000323b), 0x01);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000033c5), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00003330), 0x43);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000033e8), 0x06);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0000312b), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x000033e8), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0000312b), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00003187), 0x14);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0000312d), 0x04);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00003180), 0xff);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0000317e), 0xff);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0000317f), 0x1e);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0000317a), 0xff);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0000317b), 0xff);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0000317c), 0xff);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0000317d), 0xff);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x0000317d), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00003101), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00003101), 0x80);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00003101), 0x80);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00003101), 0x80);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00003101), 0x00);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00003173), 0x0c);
OUTREG8(GET_REG8_ADDRE(RIU_BASE_ADDR, 0x00003140), 0xd2);