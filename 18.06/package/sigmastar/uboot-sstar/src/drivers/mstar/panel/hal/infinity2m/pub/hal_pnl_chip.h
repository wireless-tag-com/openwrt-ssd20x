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

#ifndef _HAL_PNL_CHIP_H_
#define _HAL_PNL_CHIP_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define CHIP_REVISION_U01   1
#define CHIP_REVISION_U02   2

#define HAL_PNL_CTX_INST_MAX        1


#define HAL_PNL_SIGNAL_CTRL_CH_MAX  5


#define HAL_PNL_MOD_CH_MAX   5
#define HLA_PNL_CH_SWAP_MAX  5

#define HAL_PNL_MIPI_DSI_FLAG_DELAY                         0xFE
#define HAL_PNL_MIPI_DSI_FLAG_END_OF_TABLE                  0xFF   // END OF REGISTERS MARKER


#define HAL_PNL_MIPI_DSI_DCS_MAXIMUM_RETURN_PACKET_SIZE	    0x37
#define HAL_PNL_MIPI_DSI_DCS_SHORT_PACKET_ID_0              0x05
#define HAL_PNL_MIPI_DSI_DCS_SHORT_PACKET_ID_1              0x15
#define HAL_PNL_MIPI_DSI_DCS_LONG_WRITE_PACKET_ID	        0x39
#define HAL_PNL_MIPI_DSI_DCS_READ_PACKET_ID                 0x06

#define HAL_PNL_MIPI_DSI_GERNERIC_SHORT_PACKET_ID_0         0x03
#define HAL_PNL_MIPI_DSI_GERNERIC_SHORT_PACKET_ID_1         0x13
#define HAL_PNL_MIPI_DSI_GERNERIC_SHORT_PACKET_ID_2         0x23
#define HAL_PNL_MIPI_DSI_GERNERIC_LONG_WRITE_PACKET_ID	    0x29
#define HAL_PNL_MIPI_DSI_GERNERIC_READ_0_PARAM_PACKET_ID    0x04
#define HAL_PNL_MIPI_DSI_GERNERIC_READ_1_PARAM_PACKET_ID    0x14
#define HAL_PNL_MIPI_DSI_GERNERIC_READ_2_PARAM_PACKET_ID    0x24

#define AS_UINT32(x)    (*(volatile u32 *)((void*)x))
#define ALIGN_TO(x, n)  (((x) + ((n) - 1)) & ~((n) - 1))

#define HAL_PNL_CLK_NUM             5

#define CLK_MHZ(x)                  (x*1000000)
#define HAL_PNL_CLK_HDMI_RATE       1  // clk_hdmi = 1 << 2
#define HAL_PNL_CLK_DAC_RATE        1  // clk_dac  = 1 << 2
#define HAL_PNL_CLK_SC_PIXEL        10 // clk_sc_pixl = 0x0A << 2
#define HAL_PNL_CLK_MIPI_DSI        0  // clk_mipi_tx_dsi = 0,
#define HAL_PNL_CLK_MIPI_DSI_ABP    1  // clk_mipi_tx_dsi_abp = 1 << 2

#define HAL_PNL_CLK_ON_SETTING \
{ \
    1, 1, 1, 1, 1,\
}

#define HAL_PNL_CLK_OFF_SETTING \
{ \
    0, 0, 0, 0, 0,\
}


#define HAL_PNL_CLK_RATE_SETTING \
{ \
    HAL_PNL_CLK_HDMI_RATE, \
    HAL_PNL_CLK_DAC_RATE, \
    HAL_PNL_CLK_SC_PIXEL, \
    HAL_PNL_CLK_MIPI_DSI, \
    HAL_PNL_CLK_MIPI_DSI_ABP, \
}

#define HAL_PNL_CLK_MUX_ATTR \
{ \
    1, 1, 1, 1, 1, \
}

#define HAL_PNL_CLK_NAME \
{   \
    "CLK_HDMI",         \
    "CLK_DAC",          \
    "CLK_SC_PIXEL",     \
    "CLK_MIPI_DSI",     \
    "CLK_MIPI_DSI_ABP", \
}

#define EFUSE_WORLD_WIDE_CHINA     0x00
#define EFUSE_WORLD_WIDE_GLOBAL    0x01
#define EFUSE_WORLD_WIDE_REGRET    0x03

#define EFUSE_APP_OPT_NVR_0        0x00
#define EFUSE_APP_OPT_NVR_1        0x03

#define EFUSE_FEATURE_OPT_SSR621D  0x01
//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------


#endif

