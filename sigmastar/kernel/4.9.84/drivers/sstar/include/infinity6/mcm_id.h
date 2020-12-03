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

#define MCM_ID_START                (0)
#define MCM_ID_MCU51                (MCM_ID_START+0)
#define MCM_ID_URDMA                (MCM_ID_START+1)
#define MCM_ID_BDMA                 (MCM_ID_START+2)
#define MCM_ID_MFE                  (MCM_ID_START+3)
#define MCM_ID_JPE                  (MCM_ID_START+4)
#define MCM_ID_BACH                 (MCM_ID_START+5)
#define MCM_ID_FILE                 (MCM_ID_START+6)
#define MCM_ID_UHC0                 (MCM_ID_START+7)
#define MCM_ID_EMAC                 (MCM_ID_START+8)
#define MCM_ID_CMDQ                 (MCM_ID_START+9)
#define MCM_ID_ISP_DNR              (MCM_ID_START+10)
#define MCM_ID_ISP_DMA              (MCM_ID_START+11)
#define MCM_ID_GOP0                 (MCM_ID_START+12)
#define MCM_ID_SC_DNR               (MCM_ID_START+13)
#define MCM_ID_SC_DNR_SAD           (MCM_ID_START+14)
#define MCM_ID_SC_CROP              (MCM_ID_START+15)
#define MCM_ID_SC1_FRM              (MCM_ID_START+16)
#define MCM_ID_SC1_SNP              (MCM_ID_START+17)
#define MCM_ID_SC1_DBG              (MCM_ID_START+18)
#define MCM_ID_SC2_FRM              (MCM_ID_START+19)
#define MCM_ID_SC3_FRM              (MCM_ID_START+20)
#define MCM_ID_FCIE                 (MCM_ID_START+21)
#define MCM_ID_SDIO                 (MCM_ID_START+22)
#define MCM_ID_SC1_SNPI             (MCM_ID_START+23)
#define MCM_ID_SC2_SNPI             (MCM_ID_START+24)
#define MCM_ID_CMDQ1                (MCM_ID_START+25)
#define MCM_ID_CMDQ2                (MCM_ID_START+26)
#define MCM_ID_GOP1                 (MCM_ID_START+27)
#define MCM_ID_GOP2                 (MCM_ID_START+28)
#define MCM_ID_UHC1                 (MCM_ID_START+29)
#define MCM_ID_IVE                  (MCM_ID_START+30)
#define MCM_ID_VHE                  (MCM_ID_START+31)
#define MCM_ID_END                  (MCM_ID_START+32)
#define MCM_ID_ALL                  (256)


#define OFFSET_MCM_DIG_GP_START    (BASE_REG_MCM_DIG_GP_PA+0x0)
#define OFFSET_MCM_ID_MCU51        (OFFSET_MCM_DIG_GP_START+(0x0<<2)  )
#define OFFSET_MCM_ID_URDMA        (OFFSET_MCM_DIG_GP_START+(0x0<<2)+1)
#define OFFSET_MCM_ID_BDMA         (OFFSET_MCM_DIG_GP_START+(0x1<<2)  )


#define OFFSET_MCM_SC_GP_START     (BASE_REG_MCM_SC_GP_PA+0x10)
#define OFFSET_MCM_ID_MFE          (OFFSET_MCM_SC_GP_START+(0x0<<2)  )
#define OFFSET_MCM_ID_JPE          (OFFSET_MCM_SC_GP_START+(0x0<<2)+1)
#define OFFSET_MCM_ID_BACH         (OFFSET_MCM_SC_GP_START+(0x1<<2)  )
#define OFFSET_MCM_ID_FILE         (OFFSET_MCM_SC_GP_START+(0x1<<2)+1)
#define OFFSET_MCM_ID_UHC0         (OFFSET_MCM_SC_GP_START+(0x2<<2)  )
#define OFFSET_MCM_ID_EMAC         (OFFSET_MCM_SC_GP_START+(0x2<<2)+1)
#define OFFSET_MCM_ID_CMDQ         (OFFSET_MCM_SC_GP_START+(0x3<<2)  )
#define OFFSET_MCM_ID_ISP_DNR      (OFFSET_MCM_SC_GP_START+(0x3<<2)+1)
#define OFFSET_MCM_ID_ISP_DMA      (OFFSET_MCM_SC_GP_START+(0x4<<2)  )
#define OFFSET_MCM_ID_GOP0         (OFFSET_MCM_SC_GP_START+(0x4<<2)+1)
#define OFFSET_MCM_ID_SC_DNR       (OFFSET_MCM_SC_GP_START+(0x5<<2)  )
#define OFFSET_MCM_ID_SC_DNR_SAD   (OFFSET_MCM_SC_GP_START+(0x5<<2)+1)
#define OFFSET_MCM_ID_SC_CROP      (OFFSET_MCM_SC_GP_START+(0x6<<2)  )
#define OFFSET_MCM_ID_SC1_FRM      (OFFSET_MCM_SC_GP_START+(0x6<<2)+1)
#define OFFSET_MCM_ID_SC1_SNP      (OFFSET_MCM_SC_GP_START+(0x7<<2)  )
#define OFFSET_MCM_ID_SC1_DBG      (OFFSET_MCM_SC_GP_START+(0x7<<2)+1)
#define OFFSET_MCM_ID_SC2_FRM      (OFFSET_MCM_SC_GP_START+(0x8<<2)  )
#define OFFSET_MCM_ID_SC3_FRM      (OFFSET_MCM_SC_GP_START+(0x8<<2)+1)
#define OFFSET_MCM_ID_FCIE         (OFFSET_MCM_SC_GP_START+(0x9<<2)  )
#define OFFSET_MCM_ID_SDIO         (OFFSET_MCM_SC_GP_START+(0x9<<2)+1)
#define OFFSET_MCM_ID_SC1_SNPI     (OFFSET_MCM_SC_GP_START+(0xA<<2)  )
#define OFFSET_MCM_ID_SC2_SNPI     (OFFSET_MCM_SC_GP_START+(0xA<<2)+1)
#define OFFSET_MCM_ID_CMDQ1        (OFFSET_MCM_SC_GP_START+(0xB<<2)  )
#define OFFSET_MCM_ID_CMDQ2        (OFFSET_MCM_SC_GP_START+(0xB<<2)+1)
#define OFFSET_MCM_ID_GOP1         (OFFSET_MCM_SC_GP_START+(0xD<<2)  )
#define OFFSET_MCM_ID_GOP2         (OFFSET_MCM_SC_GP_START+(0xD<<2)+1)
#define OFFSET_MCM_ID_UHC1         (OFFSET_MCM_SC_GP_START+(0xE<<2)  )
#define OFFSET_MCM_ID_IVE          (OFFSET_MCM_SC_GP_START+(0xE<<2)+1)


#define OFFSET_MCM_VHE_GP_START    (BASE_REG_MCM_VHE_GP_PA+0x30)
#define OFFSET_MCM_ID_VHE          (OFFSET_MCM_VHE_GP_START+(0x0<<2)  )

#endif
