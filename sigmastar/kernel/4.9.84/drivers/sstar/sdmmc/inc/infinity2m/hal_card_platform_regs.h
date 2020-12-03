/*
* hal_card_platform_regs.h- Sigmastar
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

#define A_RIU_BASE          (0x1F000000)
//but this does send command error: EV_STS_MIE_TOUT
//---- sd & sdio ----
#define A_FCIE1_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0800)      //SDIO0_0_BANK 1410h (SD on i6)
#define A_FCIE1_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0880)      //SDIO0_1_BANK 1411h
#define A_FCIE1_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0900)      //SDIO0_2_BANK 1412h
#define A_FCIE2_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0800)      //FCIE0_0_BANK 1410h (SD1 on i6)
#define A_FCIE2_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0880)      //FCIE0_1_BANK 1411h
#define A_FCIE2_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0900)      //FCIE0_2_BANK 1412h

#define A_FCIE3_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0100)      //not used
#define A_FCIE3_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0180)
#define A_FCIE3_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0200)

