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

#define SCALER_REGISTER_SPREAD       1UL

//PM
#define REG_PM_SLP_BASE              0x000E00UL
#define REG_CEC_BASE                 0x001100UL
#define REG_PM_MCU_BASE              0x001000UL

//NONPM
#define REG_MIU0_BASE                0x101200UL
#define REG_MIU1_BASE                0x100600UL
#define REG_CHIPTOP_BASE             0x101E00UL  // 0x1E00 - 0x1EFF
#define REG_UHC0_BASE                0x102400UL
#define REG_ADC_ATOP_BASE            0x102500UL  // 0x2500 - 0x25FF
#define REG_ADC_DTOP_BASE            0x102600UL  // 0x2600 - 0x26EF
#define REG_IPMUX_BASE               0x102E00UL
#if SCALER_REGISTER_SPREAD
#define REG_SCALER_BASE              0x130000UL
#else
#define REG_SCALER_BASE              0x102F00UL
#endif
#define REG_LPLL_BASE                0x103100UL
#define REG_MOD_BASE                 0x103200UL

#define REG_HDMI_BASE                0x102700UL  // 0x2700 - 0x27FF
#define REG_HDMI2_BASE               0x101A00UL
#define REG_DVI_ATOP_BASE            0x110900UL
#define REG_DVI_DTOP_BASE            0x110A00UL
#define REG_DVI_EQ_BASE              0x110A80UL     // EQ started from 0x80
#define REG_HDCP_BASE                0x110AC0UL     // HDCP started from 0xC0
#define REG_DVI_ATOP1_BASE           0x113200UL
#define REG_DVI_DTOP1_BASE           0x113300UL
#define REG_DVI_EQ1_BASE             0x113380UL     // EQ started from 0x80
#define REG_HDCP1_BASE               0x1133C0UL     // HDCP started from 0xC0
#define REG_DVI_ATOP2_BASE           0x113400UL
#define REG_DVI_DTOP2_BASE           0x113500UL
#define REG_DVI_EQ2_BASE             0x113580UL     // EQ started from 0x80
#define REG_HDCP2_BASE               0x1135C0UL     // HDCP started from 0xC0
#define REG_DVI_PS_BASE              0x113600UL // DVI power saving
#define REG_DVI_DTOP3_BASE           0x113700UL
#define REG_DVI_EQ3_BASE             0x113780UL     // EQ started from 0x80
#define REG_HDCP3_BASE               0x1137C0UL     // HDCP started from 0xC0
#define REG_MHL_TMDS_BASE            0x122700UL

#define REG_COMBO_PHY0_P0_BASE       0x170200UL


#define REG_DVI_ATOP_06_L    (REG_DVI_ATOP_BASE + 0x0C)
#define REG_DVI_ATOP_70_H    (REG_DVI_ATOP_BASE + 0xE1)
#define REG_DVI_ATOP_71_H    (REG_DVI_ATOP_BASE + 0xE3)
#define REG_DVI_ATOP_72_H    (REG_DVI_ATOP_BASE + 0xE4)
#define REG_DVI_ATOP_73_H    (REG_DVI_ATOP_BASE + 0xE6)
#define REG_DVI_ATOP1_06_L    (REG_DVI_ATOP1_BASE + 0x0C)
#define REG_DVI_ATOP1_70_H    (REG_DVI_ATOP1_BASE + 0xE1)
#define REG_DVI_ATOP1_71_H    (REG_DVI_ATOP1_BASE + 0xE3)
#define REG_DVI_ATOP2_06_L    (REG_DVI_ATOP2_BASE + 0x0C)
#define REG_DVI_ATOP2_70_H    (REG_DVI_ATOP2_BASE + 0xE1)
#define REG_DVI_ATOP2_71_H    (REG_DVI_ATOP2_BASE + 0xE3)
#define REG_DVI_EQ_00_L     (REG_DVI_EQ_BASE + 0x00)
#define REG_DVI_EQ_00_H     (REG_DVI_EQ_BASE + 0x01)
#define REG_DVI_EQ1_00_L     (REG_DVI_EQ1_BASE + 0x00)
#define REG_DVI_EQ1_00_H     (REG_DVI_EQ1_BASE + 0x01)
#define REG_DVI_EQ2_00_L     (REG_DVI_EQ2_BASE + 0x00)
#define REG_DVI_EQ2_00_H     (REG_DVI_EQ2_BASE + 0x01)
#define REG_DVI_EQ3_00_L     (REG_DVI_EQ3_BASE + 0x00)
#define REG_DVI_EQ3_00_H     (REG_DVI_EQ3_BASE + 0x01)
#define REG_HDCP_01_H       (REG_HDCP_BASE + 0x02)
#define REG_HDCP_15_L       (REG_HDCP_BASE + 0x2A)
#define REG_HDCP_15_H       (REG_HDCP_BASE + 0x2B)
#define REG_HDCP1_01_H       (REG_HDCP1_BASE + 0x02)
#define REG_HDCP1_15_L       (REG_HDCP1_BASE + 0x2A)
#define REG_HDCP1_15_H       (REG_HDCP1_BASE + 0x2B)
#define REG_HDCP2_01_H       (REG_HDCP2_BASE + 0x02)
#define REG_HDCP2_15_L       (REG_HDCP2_BASE + 0x2A)
#define REG_HDCP2_15_H       (REG_HDCP2_BASE + 0x2B)
#define REG_HDCP3_01_H       (REG_HDCP3_BASE + 0x02)
#define REG_HDCP3_15_L       (REG_HDCP3_BASE + 0x2A)
#define REG_HDCP3_15_H       (REG_HDCP3_BASE + 0x2B)
#define REG_HDMI2_26_L       (REG_HDMI2_BASE + 0x4C)
#define REG_HDMI2_26_H       (REG_HDMI2_BASE + 0x4D)
#define REG_HDMI2_27_L       (REG_HDMI2_BASE + 0x4E)
#define REG_HDMI2_27_H       (REG_HDMI2_BASE + 0x4F)
#define REG_MHL_TMDS_60_L       (REG_MHL_TMDS_BASE + 0xC0)
#define REG_MHL_TMDS_60_H       (REG_MHL_TMDS_BASE + 0xC1)
#define REG_MHL_TMDS_63_L       (REG_MHL_TMDS_BASE + 0xC6)
#define REG_MHL_TMDS_63_H       (REG_MHL_TMDS_BASE + 0xC7)

#define REG_COMBO_PHY0_P0_0C_H       (REG_COMBO_PHY0_P0_BASE + 0x19)

