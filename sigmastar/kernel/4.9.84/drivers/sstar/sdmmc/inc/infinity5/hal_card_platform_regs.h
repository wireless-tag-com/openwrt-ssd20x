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

#define A_FCIE1_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0800)      //SDIO0_0_BANK
#define A_FCIE1_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0880)      //SDIO0_1_BANK
#define A_FCIE1_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0900)      //SDIO0_2_BANK

#define A_FCIE2_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0100)      //FCIE0_0_BANK
#define A_FCIE2_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0180)      //FCIE0_1_BANK
#define A_FCIE2_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0200)      //FCIE0_2_BANK

#define A_FCIE3_0_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0100)
#define A_FCIE3_1_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0180)
#define A_FCIE3_2_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0xA0200)

