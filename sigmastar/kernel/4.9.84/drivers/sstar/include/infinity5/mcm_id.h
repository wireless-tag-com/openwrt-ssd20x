/*
* mcm_id.h- Sigmastar
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
#ifndef ___MCM_ID_H
#define ___MCM_ID_H

#define REG_MCM_DIG_GP      (BASE_REG_MCM_DIG_GP_PA)
#define REG_MCM_MCU51       (REG_MCM_DIG_GP+(0x0<<2)  )
#define REG_MCM_URDMA       (REG_MCM_DIG_GP+(0x0<<2)+1)
#define REG_MCM_BDMA        (REG_MCM_DIG_GP+(0x1<<2)  )

#define REG_MCM_SC_GP1      (BASE_REG_MCM_SC_GP_PA)
#define REG_MCM_DIP_R       (REG_MCM_SC_GP1+(0x0<<2)  )
#define REG_MCM_DIP_W       (REG_MCM_SC_GP1+(0x0<<2)+1)
#define REG_MCM_LDC         (REG_MCM_SC_GP1+(0x1<<2)  )
#define REG_MCM_SC2_FRM     (REG_MCM_SC_GP1+(0x1<<2)+1)
#define REG_MCM_SC3_FRM     (REG_MCM_SC_GP1+(0x2<<2)  )
#define REG_MCM_RSC         (REG_MCM_SC_GP1+(0x2<<2)+1)
#define REG_MCM_SC1_DBG     (REG_MCM_SC_GP1+(0x3<<2)  )
#define REG_MCM_CMDQ0       (REG_MCM_SC_GP1+(0x3<<2)+1)
#define REG_MCM_MOVDMA0     (REG_MCM_SC_GP1+(0x4<<2)  )
#define REG_MCM_EMAC        (REG_MCM_SC_GP1+(0x4<<2)+1)
//      REG_MCM_GE
#define REG_MCM_3DNR0_R     (REG_MCM_SC_GP1+(0x5<<2)+1)
#define REG_MCM_3DNR0_W     (REG_MCM_SC_GP1+(0x6<<2)  )
#define REG_MCM_GOP4        (REG_MCM_SC_GP1+(0x6<<2)+1)
#define REG_MCM_ISP_DMAG0   (REG_MCM_SC_GP1+(0x7<<2)  )
#define REG_MCM_ISP_DMAG1   (REG_MCM_SC_GP1+(0x7<<2)+1)
#define REG_MCM_ISP_DMAG2   (REG_MCM_SC_GP1+(0x8<<2)  )
#define REG_MCM_GOP2        (REG_MCM_SC_GP1+(0x8<<2)+1)
#define REG_MCM_GOP3        (REG_MCM_SC_GP1+(0x9<<2)  )
#define REG_MCM_ISP_DMAG    (REG_MCM_SC_GP1+(0x9<<2)+1)
#define REG_MCM_ISP_STA     (REG_MCM_SC_GP1+(0xA<<2)  )
#define REG_MCM_ISP_STA1    (REG_MCM_SC_GP1+(0xA<<2)+1)
#define REG_MCM_CMDQ1       (REG_MCM_SC_GP1+(0xB<<2)  )
#define REG_MCM_MOVDMA1     (REG_MCM_SC_GP1+(0xB<<2)+1)
#define REG_MCM_IVE         (REG_MCM_SC_GP1+(0xC<<2)  )
#define REG_MCM_SC_ROT_R    (REG_MCM_SC_GP1+(0xC<<2)+1)
#define REG_MCM_SC_AIP_W    (REG_MCM_SC_GP1+(0xD<<2)  )
#define REG_MCM_SC0_FRM     (REG_MCM_SC_GP1+(0xD<<2)+1)
#define REG_MCM_SC0_SNP     (REG_MCM_SC_GP1+(0xE<<2)  )
#define REG_MCM_SC1_FRM     (REG_MCM_SC_GP1+(0xE<<2)+1)
#define REG_MCM_GOP0        (REG_MCM_SC_GP1+(0xF<<2)  )
#define REG_MCM_3DNR1_R     (REG_MCM_SC_GP1+(0xF<<2)+1)

#define REG_MCM_SC_GP2      (BASE_REG_MCM_SC_GP2_PA)
#define REG_MCM_3DNR1_W     (REG_MCM_SC_GP2+(0x0<<2)  )
#define REG_MCM_CMDQ2       (REG_MCM_SC_GP2+(0x0<<2)+1)
#define REG_MCM_AESDMA      (REG_MCM_SC_GP2+(0x1<<2)  )
#define REG_MCM_USB20       (REG_MCM_SC_GP2+(0x1<<2)+1)
#define REG_MCM_USB20_H     (REG_MCM_SC_GP2+(0x2<<2)  )
#define REG_MCM_JPE         (REG_MCM_SC_GP2+(0x2<<2)+1)
//      REG_MCM_XX
//      REG_MCM_DIP1_R
//      REG_MCM_DIP1_W
#define REG_MCM_GOP1        (REG_MCM_SC_GP2+(0x4<<2)+1)
#define REG_MCM_BACH        (REG_MCM_SC_GP2+(0x5<<2)  )
//      REG_MCM_XX
#define REG_MCM_CMDQ3       (REG_MCM_SC_GP2+(0x6<<2)  )
#define REG_MCM_SDIO        (REG_MCM_SC_GP2+(0x6<<2)+1)
#define REG_MCM_FCIE        (REG_MCM_SC_GP2+(0x7<<2)  )

#define REG_MCM_VHE_GP      (BASE_REG_MCM_VHE_GP_PA)
#define REG_MCM_VEN         (REG_MCM_VHE_GP+(0x0<<2)  )

#define MCM_ID_ALL          (0xFF)

#endif
