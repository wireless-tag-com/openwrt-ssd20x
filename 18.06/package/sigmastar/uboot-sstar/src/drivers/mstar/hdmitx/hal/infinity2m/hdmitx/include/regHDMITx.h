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

#ifndef _REG_HDMITX_H_
#define _REG_HDMITX_H_


//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define HDMITX_DTOP_BASE                (0x112500U)
#define HDMITX_ATOP_BASE                (0x112600U)
#define HDMITX_SC_GP_CTRL_BASE          (0x113300U)
#define HDMITX_CLKGEN_BASE              (0x103800U)

#define HDMITX_MISC_REG_BASE            (0x172A00U)
#define HDMITX_HDCP_REG_BASE            (0x172B00U)

//***** Bank 172A - MISC *****//
#define REG_MISC_CONFIG_00               0x00U
#define REG_MISC_CONFIG_01               0x01U
#define REG_MISC_CONFIG_02               0x02U
#define REG_MISC_CONFIG_03               0x03U
#define REG_MISC_CONFIG_04               0x04U
#define REG_MISC_CONFIG_05               0x05U
#define REG_MISC_CONFIG_06               0x06U
#define REG_MISC_CONFIG_07               0x07U
#define REG_MISC_CONFIG_08               0x08U
#define REG_MISC_CONFIG_09               0x09U
#define REG_MISC_STATUS_0A               0x0AU
#define REG_MISC_STATUS_0B               0x0BU
#define REG_MISC_CONFIG_0C               0x0CU
#define REG_MISC_STATUS_0D               0x0DU
#define REG_MISC_STATUS_0E               0x0EU
#define REG_MISC_STATUS_0F               0x0FU
#define REG_MISC_CONFIG_17               0x17U
#define REG_MISC_CONFIG_1B               0x1BU
#define REG_MISC_CONFIG_1C               0x1CU
#define REG_MISC_CONFIG_1D               0x1DU
#define REG_MISC_CONFIG_1E               0x1EU
#define REG_MISC_CONFIG_1F               0x1FU
#define REG_MISC_CONFIG_20               0x20U
#define REG_MISC_CONFIG_21               0x21U
#define REG_MISC_CONFIG_22               0x22U
#define REG_MISC_CONFIG_23               0x23U
#define REG_MISC_CONFIG_24               0x24U
#define REG_MISC_CONFIG_25               0x25U
#define REG_MISC_CONFIG_26               0x26U
#define REG_MISC_CONFIG_27               0x27U
#define REG_MISC_CONFIG_2A               0x2AU
#define REG_MISC_CONFIG_2B               0x2BU
#define REG_MISC_CONFIG_2C               0x2CU
#define REG_MISC_CONFIG_2D               0x2DU
#define REG_MISC_CONFIG_2E               0x2EU
#define REG_MISC_CONFIG_2F               0x2FU
#define REG_MISC_CONFIG_33               0x33U
#define REG_MISC_CONFIG_34               0x34U
#define REG_MISC_CONFIG_36               0x36U
#define REG_MISC_CONFIG_38               0x38U
#define REG_MISC_CONFIG_40               0x40U
#define REG_MISC_CONFIG_41               0x41U
#define REG_MISC_CONFIG_45               0x45U
#define REG_MISC_CONFIG_48               0x48U
#define REG_MISC_CONFIG_4D               0x4DU
#define REG_MISC_CONFIG_52               0x52U
#define REG_MISC_CONFIG_58               0x58U
#define REG_MISC_CONFIG_59               0x59U
#define REG_MISC_CONFIG_5D               0x5DU


//***** Bank 172B - HDCP *****//
#define REG_HDCP_TX_RI_00               0x00U
#define REG_HDCP_TX_MODE_01             0x01U	// Pj[7:0] : 61h[7:0]; Tx_mode[7:0] : 61h[15:8]
#define REG_HDCP_TX_COMMAND_02          0x02U
#define REG_HDCP_TX_RI127_03            0x03U       // RI[15:0] 127th frame : 63[15:0]
#define REG_HDCP_TX_LN_04               0x04U	// Ln[55:0] : 64h[7:0] ~ 67h[7:0]
#define REG_HDCP_TX_LN_SEED_07          0x07U	// Ln seed[7:0] : 67h[15:8]
#define REG_HDCP_TX_AN_08               0x08U	// An[63:0] : 68[7:0] ~ 6B[15:8]
#define REG_HDCP_TX_MI_0C               0x0CU	// Mi[63:0] : 6C[7:0] ~ 6F[15:8]

// --------- BK1125 HDMIA_DTOP ---------- //
#define REG_HDMITX_PD                       0x00U
    #define REG_HDMITX_PD_MSK                   0x0001
    #define REG_HDMITX_PD_EN                    0x0001

#define REG_HDMITX_SW_RST                   0x00U
    #define REG_HDMITX_SW_RST_MSK               0x0002
    #define REG_HDMITX_SW_RST_EN                0x0002

#define REG_HDMITX_MODE                     0x01U
    #define REG_HDMITX_MODE_MSK                 0x0001
    #define REG_HDMITX_MODE_HDMI                0x0001
    #define REG_HDMITX_MODE_DVI                 0x0000


#define REG_HDMITX_AUDIO_LAYOUT             0x01U
    #define REG_HDMITX_AUDIO_LAYOUT_MSK         0x0002
    #define REG_HDMITX_AUDIO_LAYOUT_2_CH        0x0000
    #define REG_HDMITX_AUDIO_LAYOUT_8_CH        0x0002

#define REG_HDMITX_AUIDO_SP                 0x01U
    #define REG_HDMITX_AUDIO_SP_MSK             0x0F00
    #define REG_HDMITX_AUDIO_SP_SUBPACKET_NONE  0x0000
    #define REG_HDMITX_AUDIO_SP_SUBPACKET_0     0x0100
    #define REG_HDMITX_AUDIO_SP_SUBPACKET_01    0x0300
    #define REG_HDMITX_AUDIO_SP_SUBPACKET_012   0x0700
    #define REG_HDMITX_AUDIO_SP_SUBPACKET_0123  0x0F00

#define REG_HDMITX_AVMUTE                   0x01U
    #define REG_HDMITX_AVMUTE_MSK               0x4000
    #define REG_HDMITX_AVMUTE_ON                0x4000

#define REG_HDMITX_AUDIO_MUTE              0x01U
    #define REG_HDMITX_AUDIO_MUTE_MSK           0x1000
    #define REG_HDMITX_AUDIO_MUTE_EN            0x1000

#define REG_HDMITX_VIDEO_MUTE               0x01U
    #define REG_HDMITX_VIDEO_MUTE_MSK           0x2000
    #define REG_HDMITX_VIDEO_MUTE_EN            0x2000

#define REG_HDMITX_CH0_MUTE                 0x05U
    #define REG_HDMITX_CH0_MUTE_MSK             0x0008
    #define REG_HDMITX_CH0_MUTE_ON              0x0008
    #define REG_HDMITX_CH0_MUTE_OFF             0x0000


#define REG_HDMITX_CH1_MUTE                 0x05U
    #define REG_HDMITX_CH1_MUTE_MSK             0x0010
    #define REG_HDMITX_CH1_MUTE_ON              0x0010
    #define REG_HDMITX_CH1_MUTE_OFF             0x0000

#define REG_HDMITX_CH2_MUTE                 0x05U
    #define REG_HDMITX_CH2_MUTE_MSK             0x0020
    #define REG_HDMITX_CH2_MUTE_ON              0x0020
    #define REG_HDMITX_CH2_MUTE_OFF             0x0000

#define REG_HDMITX_CSC_SEL                  0x06U
    #define REG_HDMITX_CSC_SEL_MSK              0x000F
    #define REG_HDMITX_CSC_SEL_422_10_TO_8      0x0001
    #define REG_HDMITX_CSC_SEL_Y_TO_R           0x0002
    #define REG_HDMITX_CSC_SEL_R_TO_Y           0x0004
    #define REG_HDMITX_CSC_SEL_444_TO_422       0x0008
    #define REG_HDMITX_CSC_SEL_BYPASS           0x0000

#define REG_HDMITX_DATA_PERIOD_ISLAND       0x08U
    #define REG_HDMITX_DATA_PERIOD_ISLAND_MSK   0x00FF


#define REG_HDMITX_SYNC_OUT_POL             0x0FU
    #define REG_HDMITX_VSYNC_OUT_POL_MSK        0x0004
    #define REG_HDMITX_VSYNC_OUT_POL_NORMRAL    0x0000
    #define REG_HDMITX_VSYNC_OUT_POL_INVERT     0x0004

    #define REG_HDMITX_HSYNC_OUT_POL_MSK        0x0008
    #define REG_HDMITX_HSYNC_OUT_POL_NORMRAL    0x0000
    #define REG_HDMITX_HSYNC_OUT_POL_INVERT     0x0008

#define REG_HDMITX_PKT_EN                   0x1BU
    #define REG_HDMITX_PKT_SPD_MSK              0x0001
    #define REG_HDMITX_PKT_SPD_EN               0x0001
    #define REG_HDMITX_PKT_PKT0_MSK             0x0001
    #define REG_HDMITX_PKT_PKT0_EN              0x0001

    #define REG_HDMITX_PKT_VS_MSK               0x0002
    #define REG_HDMITX_PKT_VS_EN                0x0002
    #define REG_HDMITX_PKT_PKT1_MSK             0x0002
    #define REG_HDMITX_PKT_PKT1_EN              0x0002

    #define REG_HDMITX_PKT_AUDIO_MSK            0x0004
    #define REG_HDMITX_PKT_AUDIO_EN             0x0004

    #define REG_HDMITX_PKT_ACR_MSK              0x0008
    #define REG_HDMITX_PKT_ACR_EN               0x0008

    #define REG_HDMITX_PKT_ADO_MSK              0x0010
    #define REG_HDMITX_PKT_ADO_EN               0x0010

    #define REG_HDMITX_PKT_AVI_MSK              0x0020
    #define REG_HDMITX_PKT_AVI_EN               0x0020

    #define REG_HDMITX_PKT_GC_MSK               0x0040
    #define REG_HDMITX_PKT_GC_EN                0x0040

    #define REG_HDMITX_PKT_NULL_MSK             0x0080
    #define REG_HDMITX_PKT_NULL_EN              0x0080

    #define REG_HDMITX_PKT_PKT2_MSK             0x0100
    #define REG_HDMITX_PKT_PKT2_EN              0x0100

#define REG_PKT_RDY_SEL                     0x1DU
    #define REG_PKT0_RDY_MSK                    0x0001
    #define REG_PKT0_RDY_EN                     0x0001

    #define REG_PKT1_RDY_MSK                    0x0002
    #define REG_PKT1_RDY_EN                     0x0002

    #define REG_PKT2_RDY_MSK                    0x0004
    #define REG_PKT2_RDY_EN                     0x0004

#define REG_HDMITX_AUIDO_SF                 0x20U
    #define REG_HDMITX_AUDIO_SF_MSK     0x000F

#define REG_HDMITX_ACR_MOD                      0x30U
    #define REG_HDMITX_ACR_MOD_MSK                  0x0001
    #define REG_HDMITX_ACR_MOD_FROM_COUNTER         0x0001
    #define REG_HDMITX_ACR_MOD_FROM_REG             0x0000

#define REG_HDMITX_ACR_CTS_1                    0x31U
    #define REG_HDMITX_ACR_CTS_1_MSK                0x00FF
    #define REG_HDMITX_ACR_CTS_1_VAL(x)             (x & 0x000FF)

#define REG_HDMITX_ACR_CTS_2                    0x31U
    #define REG_HDMITX_ACR_CTS_2_MSK                0xFF00
    #define REG_HDMITX_ACR_CTS_2_VAL(x)            (x&0x0FF00)

#define REG_HDMITX_ACR_CTS_3                    0x33U
    #define REG_HDMITX_ACR_CTS_3_MSK                0x000F
    #define REG_HDMITX_ACR_CTS_3_VAL(x)            ((x & 0x0F0000) >> 16)

#define REG_HDMITX_ACR_N_1                      0x32U
    #define REG_HDMITX_ACR_N_1_MSK                  0x00FF
    #define REG_HDMITX_ACR_N_1_VAL(x)               (x & 0x000FF)

#define REG_HDMITX_ACR_N_2                      0x32U
    #define REG_HDMITX_ACR_N_2_MSK                  0xFF00
    #define REG_HDMITX_ACR_N_2_VAL(x)               (x & 0x0FF00)

#define REG_HDMITX_ACR_N_3                      0x33U
    #define REG_HDMITX_ACR_N_3_MSK                  0x00F0
    #define REG_HDMITX_ACR_N_3_VAL(x)               ((x & 0xF0000)>>8)

#define REG_HDMITX_PKT_AVI_LEN                  (0x0D)
#define REG_HDMITX_PKT_AVI_0                    ((0x34U<<1)+0)
#define REG_HDMITX_PKT_AVI_1                    ((0x34U<<1)+1)
#define REG_HDMITX_PKT_AVI_2                    ((0x35U<<1)+0)
#define REG_HDMITX_PKT_AVI_3                    ((0x35U<<1)+1)
#define REG_HDMITX_PKT_AVI_4                    ((0x36U<<1)+0)
#define REG_HDMITX_PKT_AVI_5                    ((0x36U<<1)+1)
#define REG_HDMITX_PKT_AVI_6                    ((0x37U<<1)+0)
#define REG_HDMITX_PKT_AVI_7                    ((0x38U<<1)+0)
#define REG_HDMITX_PKT_AVI_8                    ((0x38U<<1)+1)
#define REG_HDMITX_PKT_AVI_9                    ((0x39U<<1)+0)
#define REG_HDMITX_PKT_AVI_10                   ((0x39U<<1)+1)
#define REG_HDMITX_PKT_AVI_11                   ((0x3AU<<1)+0)
#define REG_HDMITX_PKT_AVI_12                   ((0x3AU<<1)+1)
#define REG_HDMITX_PKT_AVI_13                   ((0x3BU<<1)+0)

#define REG_HDMITX_PKT_ADO_LEN                  (0x06)
#define REG_HDMITX_PKT_ADO_0                    ((0x3CU<<1)+0)
#define REG_HDMITX_PKT_ADO_1                    ((0x3CU<<1)+1)
#define REG_HDMITX_PKT_ADO_2                    ((0x3DU<<1)+0)
#define REG_HDMITX_PKT_ADO_3                    ((0x3DU<<1)+1)
#define REG_HDMITX_PKT_ADO_4                    ((0x3EU<<1)+0)
#define REG_HDMITX_PKT_ADO_5                    ((0x3EU<<1)+1)
#define REG_HDMITX_PKT_ADO_6                    ((0x3FU<<1)+0)


#define REG_HDMITX_PKT_ASP_CS_1                 ((0x21U<<1)+0)
#define REG_HDMITX_PKT_ASP_CS_2                 ((0x21U<<1)+1)
#define REG_HDMITX_PKT_ASP_CS_3                 ((0x22U<<1)+0)
#define REG_HDMITX_PKT_ASP_CS_4                 ((0x22U<<1)+1)
#define REG_HDMITX_PKT_ASP_CS_5                 ((0x23U<<1)+0)
#define REG_HDMITX_PKT_ASP_CS_6                 ((0x23U<<1)+1)
#define REG_HDMITX_PKT_ASP_CS_7                 ((0x24U<<1)+0)
#define REG_HDMITX_PKT_ASP_CS_8                 ((0x24U<<1)+1)
#define REG_HDMITX_PKT_ASP_CS_9                 ((0x25U<<1)+0)
#define REG_HDMITX_PKT_ASP_CS_10                ((0x25U<<1)+1)
#define REG_HDMITX_PKT_ASP_CS_11                ((0x26U<<1)+0)
#define REG_HDMITX_PKT_ASP_CS_12                ((0x26U<<1)+1)
#define REG_HDMITX_PKT_ASP_CS_13                ((0x27U<<1)+0)
#define REG_HDMITX_PKT_ASP_CS_14                ((0x27U<<1)+1)
#define REG_HDMITX_PKT_ASP_CS_15                ((0x28U<<1)+0)
#define REG_HDMITX_PKT_ASP_CS_16                ((0x28U<<1)+1)
#define REG_HDMITX_PKT_ASP_CS_17                ((0x29U<<1)+0)
#define REG_HDMITX_PKT_ASP_CS_18                ((0x29U<<1)+1)
#define REG_HDMITX_PKT_ASP_CS_19                ((0x2AU<<1)+0)
#define REG_HDMITX_PKT_ASP_CS_20                ((0x2AU<<1)+1)
#define REG_HDMITX_PKT_ASP_CS_21                ((0x2BU<<1)+0)
#define REG_HDMITX_PKT_ASP_CS_22                ((0x2BU<<1)+1)
#define REG_HDMITX_PKT_ASP_CS_23                ((0x2CU<<1)+0)
#define REG_HDMITX_PKT_ASP_CS_24                ((0x2CU<<1)+1)


#define REG_HDMITX_PKT_SPD_LEN                  (0x19)
#define REG_HDMITX_PKT_SPD_HB0                  ((0x40U<<1)+0)
#define REG_HDMITX_PKT_SPD_HB1                  ((0x40U<<1)+1)
#define REG_HDMITX_PKT_SPD_HB2                  ((0x41U<<1)+0)

#define REG_HDMITX_PKT_SPD_0                    ((0x42U<<1)+0)
#define REG_HDMITX_PKT_SPD_1                    ((0x42U<<1)+1)
#define REG_HDMITX_PKT_SPD_2                    ((0x43U<<1)+0)
#define REG_HDMITX_PKT_SPD_3                    ((0x43U<<1)+1)
#define REG_HDMITX_PKT_SPD_4                    ((0x44U<<1)+0)
#define REG_HDMITX_PKT_SPD_5                    ((0x44U<<1)+1)
#define REG_HDMITX_PKT_SPD_6                    ((0x45U<<1)+0)

#define REG_HDMITX_PKT_SPD_7                    ((0x46U<<1)+0)
#define REG_HDMITX_PKT_SPD_8                    ((0x46U<<1)+1)
#define REG_HDMITX_PKT_SPD_9                    ((0x47U<<1)+0)
#define REG_HDMITX_PKT_SPD_10                   ((0x47U<<1)+1)
#define REG_HDMITX_PKT_SPD_11                   ((0x48U<<1)+0)
#define REG_HDMITX_PKT_SPD_12                   ((0x48U<<1)+1)
#define REG_HDMITX_PKT_SPD_13                   ((0x49U<<1)+0)

#define REG_HDMITX_PKT_SPD_14                   ((0x4AU<<1)+0)
#define REG_HDMITX_PKT_SPD_15                   ((0x4AU<<1)+1)
#define REG_HDMITX_PKT_SPD_16                   ((0x4BU<<1)+0)
#define REG_HDMITX_PKT_SPD_17                   ((0x4BU<<1)+1)
#define REG_HDMITX_PKT_SPD_18                   ((0x4CU<<1)+0)
#define REG_HDMITX_PKT_SPD_19                   ((0x4CU<<1)+1)
#define REG_HDMITX_PKT_SPD_20                   ((0x4DU<<1)+0)

#define REG_HDMITX_PKT_SPD_21                   ((0x4EU<<1)+0)
#define REG_HDMITX_PKT_SPD_22                   ((0x4EU<<1)+1)
#define REG_HDMITX_PKT_SPD_23                   ((0x4FU<<1)+0)
#define REG_HDMITX_PKT_SPD_24                   ((0x4FU<<1)+1)
#define REG_HDMITX_PKT_SPD_25                   ((0x50U<<1)+0)


#define REG_HDMITX_PKT_VS_LEN                   (0x1B)

#define REG_HDMITX_PKT_VS_HB0                   ((0x52U<<1)+0)
#define REG_HDMITX_PKT_VS_HB1                   ((0x52U<<1)+1)
#define REG_HDMITX_PKT_VS_HB2                   ((0x53U<<1)+0)

#define REG_HDMITX_PKT_VS_0                     ((0x54U<<1)+0)
#define REG_HDMITX_PKT_VS_1                     ((0x54U<<1)+1)
#define REG_HDMITX_PKT_VS_2                     ((0x55U<<1)+0)
#define REG_HDMITX_PKT_VS_3                     ((0x55U<<1)+1)
#define REG_HDMITX_PKT_VS_4                     ((0x56U<<1)+0)
#define REG_HDMITX_PKT_VS_5                     ((0x56U<<1)+1)
#define REG_HDMITX_PKT_VS_6                     ((0x57U<<1)+0)

#define REG_HDMITX_PKT_VS_7                     ((0x58U<<1)+0)
#define REG_HDMITX_PKT_VS_8                     ((0x58U<<1)+1)
#define REG_HDMITX_PKT_VS_9                     ((0x59U<<1)+0)
#define REG_HDMITX_PKT_VS_10                    ((0x59U<<1)+1)
#define REG_HDMITX_PKT_VS_11                    ((0x5AU<<1)+0)
#define REG_HDMITX_PKT_VS_12                    ((0x5AU<<1)+1)
#define REG_HDMITX_PKT_VS_13                    ((0x5BU<<1)+0)

#define REG_HDMITX_PKT_VS_14                    ((0x5CU<<1)+0)
#define REG_HDMITX_PKT_VS_15                    ((0x5CU<<1)+1)
#define REG_HDMITX_PKT_VS_16                    ((0x5DU<<1)+0)
#define REG_HDMITX_PKT_VS_17                    ((0x5DU<<1)+1)
#define REG_HDMITX_PKT_VS_18                    ((0x5EU<<1)+0)
#define REG_HDMITX_PKT_VS_19                    ((0x5EU<<1)+1)
#define REG_HDMITX_PKT_VS_20                    ((0x5FU<<1)+0)

#define REG_HDMITX_PKT_VS_21                    ((0x60U<<1)+0)
#define REG_HDMITX_PKT_VS_22                    ((0x60U<<1)+1)
#define REG_HDMITX_PKT_VS_23                    ((0x61U<<1)+0)
#define REG_HDMITX_PKT_VS_24                    ((0x61U<<1)+1)
#define REG_HDMITX_PKT_VS_25                    ((0x62U<<1)+0)
#define REG_HDMITX_PKT_VS_26                    ((0x62U<<1)+1)
#define REG_HDMITX_PKT_VS_27                    ((0x63U<<1)+0)


// --------- BK1126 HDMITx ATOP --------- //
#define REG_HDMITX_ATOP_16              0x16U
#define REG_HDMITX_ATOP_18              0x18U
#define REG_HDMITX_ATOP_19              0x19U
#define REG_HDMITX_ATOP_1A              0x1AU
#define REG_HDMITX_ATOP_33              0x33U
#define REG_HDMITX_ATOP_34              0x34U
#define REG_HDMITX_ATOP_35              0x35U
#define REG_HDMITX_ATOP_38              0x38U

#define REG_HDMITX_ATOP_22              0x22U
#define REG_HDMITX_ATOP_23              0x23U
#define REG_HDMITX_ATOP_24              0x24U
#define REG_HDMITX_ATOP_25              0x25U


#define REG_HDMITX_CLK_HDMI             (0x35U)
#define REG_HDMITX_CLK_DISP_432         (0x53U)
#define REG_HDMITX_CLK_DISP_216         (0x53U)


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

#endif // _REG_HDMITX_H_

