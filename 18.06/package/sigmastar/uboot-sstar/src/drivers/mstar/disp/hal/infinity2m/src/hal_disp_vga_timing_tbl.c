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

//****************************************************
// Drive Chip           : CAMDRV_VGA
// Excel CodeGen Version: 1.05
// Excel SW      Version: 1.01
// Excel update date    : 2019/9/20 14:29
//****************************************************

#ifndef _HAL_DISP_VGA_TIMING_TBL_C_
#define _HAL_DISP_VGA_TIMING_TBL_C_

#include "hal_disp_vga_timing_tbl.h" 

//****************************************************
// INIT_HDMITX_ATOP
//****************************************************
u8 ST_INIT_HDMITX_ATOP_COMMON_TBL[INIT_HDMITX_ATOP_COMMON_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + REG_DATA_SIZE] =
{
 { DRV_REG(REG_HDMITX_ATOP_TB_17_L), 0xFF, 0x08,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_20_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_20_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_21_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_21_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_25_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_2C_L), 0xFF, 0x0f,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_30_L), 0xFF, 0x0f,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_31_L), 0xFF, 0x0f,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_32_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_33_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_34_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_35_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_39_L), 0xFF, 0x33,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_00_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_1C_L), 0xFF, 0x01,
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_1C_H), 0xFF, 0x05,
 },
};

u8 ST_INIT_HDMITX_ATOP_TBL[INIT_HDMITX_ATOP_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + HAL_DISP_VGA_ID_NUM]=
{
 { DRV_REG(REG_HDMITX_ATOP_TB_16_H), 0xFF, 0x00, /*480_60P*/
                    0x00, /*576_50P*/
                    0x00, /*720_50P*/
                    0x00, /*720_60P*/
                    0x00, /*1080_24P*/
                    0x00, /*1080_25P*/
                    0x00, /*1080_30P*/
                    0x00, /*1080_50P*/
                    0x00, /*1080_60P*/
                    0x07, /*3840x2160_30P*/
                    0x00, /*1024x768_60P*/
                    0x00, /*1280x800_60P*/
                    0x00, /*1280x1024_60P*/
                    0x00, /*1366x768_60P*/
                    0x00, /*1440x900_60P*/
                    0x00, /*1680x1050_60P*/
                    0x00, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_22_L), 0xFF, 0x14, /*480_60P*/
                    0x14, /*576_50P*/
                    0x14, /*720_50P*/
                    0x14, /*720_60P*/
                    0x14, /*1080_24P*/
                    0x14, /*1080_25P*/
                    0x14, /*1080_30P*/
                    0x14, /*1080_50P*/
                    0x14, /*1080_60P*/
                    0x2c, /*3840x2160_30P*/
                    0x14, /*1024x768_60P*/
                    0x14, /*1280x800_60P*/
                    0x14, /*1280x1024_60P*/
                    0x14, /*1366x768_60P*/
                    0x14, /*1440x900_60P*/
                    0x14, /*1680x1050_60P*/
                    0x14, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_22_H), 0xFF, 0x14, /*480_60P*/
                    0x14, /*576_50P*/
                    0x14, /*720_50P*/
                    0x14, /*720_60P*/
                    0x14, /*1080_24P*/
                    0x14, /*1080_25P*/
                    0x14, /*1080_30P*/
                    0x14, /*1080_50P*/
                    0x14, /*1080_60P*/
                    0x2c, /*3840x2160_30P*/
                    0x14, /*1024x768_60P*/
                    0x14, /*1280x800_60P*/
                    0x14, /*1280x1024_60P*/
                    0x14, /*1366x768_60P*/
                    0x14, /*1440x900_60P*/
                    0x14, /*1680x1050_60P*/
                    0x14, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_23_L), 0xFF, 0x14, /*480_60P*/
                    0x14, /*576_50P*/
                    0x14, /*720_50P*/
                    0x14, /*720_60P*/
                    0x14, /*1080_24P*/
                    0x14, /*1080_25P*/
                    0x14, /*1080_30P*/
                    0x14, /*1080_50P*/
                    0x14, /*1080_60P*/
                    0x2c, /*3840x2160_30P*/
                    0x14, /*1024x768_60P*/
                    0x14, /*1280x800_60P*/
                    0x14, /*1280x1024_60P*/
                    0x14, /*1366x768_60P*/
                    0x14, /*1440x900_60P*/
                    0x14, /*1680x1050_60P*/
                    0x14, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_23_H), 0xFF, 0x14, /*480_60P*/
                    0x14, /*576_50P*/
                    0x14, /*720_50P*/
                    0x14, /*720_60P*/
                    0x14, /*1080_24P*/
                    0x14, /*1080_25P*/
                    0x14, /*1080_30P*/
                    0x14, /*1080_50P*/
                    0x14, /*1080_60P*/
                    0x16, /*3840x2160_30P*/
                    0x14, /*1024x768_60P*/
                    0x14, /*1280x800_60P*/
                    0x14, /*1280x1024_60P*/
                    0x14, /*1366x768_60P*/
                    0x14, /*1440x900_60P*/
                    0x14, /*1680x1050_60P*/
                    0x14, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_24_L), 0xFF, 0x00, /*480_60P*/
                    0x00, /*576_50P*/
                    0x00, /*720_50P*/
                    0x00, /*720_60P*/
                    0x00, /*1080_24P*/
                    0x00, /*1080_25P*/
                    0x00, /*1080_30P*/
                    0x00, /*1080_50P*/
                    0x00, /*1080_60P*/
                    0x04, /*3840x2160_30P*/
                    0x00, /*1024x768_60P*/
                    0x00, /*1280x800_60P*/
                    0x00, /*1280x1024_60P*/
                    0x00, /*1366x768_60P*/
                    0x00, /*1440x900_60P*/
                    0x00, /*1680x1050_60P*/
                    0x00, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_24_H), 0xFF, 0x00, /*480_60P*/
                    0x00, /*576_50P*/
                    0x00, /*720_50P*/
                    0x00, /*720_60P*/
                    0x00, /*1080_24P*/
                    0x00, /*1080_25P*/
                    0x00, /*1080_30P*/
                    0x00, /*1080_50P*/
                    0x00, /*1080_60P*/
                    0x04, /*3840x2160_30P*/
                    0x00, /*1024x768_60P*/
                    0x00, /*1280x800_60P*/
                    0x00, /*1280x1024_60P*/
                    0x00, /*1366x768_60P*/
                    0x00, /*1440x900_60P*/
                    0x00, /*1680x1050_60P*/
                    0x00, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_25_L), 0xFF, 0x00, /*480_60P*/
                    0x00, /*576_50P*/
                    0x00, /*720_50P*/
                    0x00, /*720_60P*/
                    0x00, /*1080_24P*/
                    0x00, /*1080_25P*/
                    0x00, /*1080_30P*/
                    0x00, /*1080_50P*/
                    0x00, /*1080_60P*/
                    0x04, /*3840x2160_30P*/
                    0x00, /*1024x768_60P*/
                    0x00, /*1280x800_60P*/
                    0x00, /*1280x1024_60P*/
                    0x00, /*1366x768_60P*/
                    0x00, /*1440x900_60P*/
                    0x00, /*1680x1050_60P*/
                    0x00, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_30_H), 0xFF, 0x0f, /*480_60P*/
                    0x0f, /*576_50P*/
                    0x0f, /*720_50P*/
                    0x0f, /*720_60P*/
                    0x0f, /*1080_24P*/
                    0x0f, /*1080_25P*/
                    0x0f, /*1080_30P*/
                    0x0f, /*1080_50P*/
                    0x0f, /*1080_60P*/
                    0x08, /*3840x2160_30P*/
                    0x0f, /*1024x768_60P*/
                    0x0f, /*1280x800_60P*/
                    0x0f, /*1280x1024_60P*/
                    0x0f, /*1366x768_60P*/
                    0x0f, /*1440x900_60P*/
                    0x0f, /*1680x1050_60P*/
                    0x0f, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_31_H), 0xFF, 0x0f, /*480_60P*/
                    0x0f, /*576_50P*/
                    0x0f, /*720_50P*/
                    0x0f, /*720_60P*/
                    0x0f, /*1080_24P*/
                    0x0f, /*1080_25P*/
                    0x0f, /*1080_30P*/
                    0x0f, /*1080_50P*/
                    0x0f, /*1080_60P*/
                    0x08, /*3840x2160_30P*/
                    0x0f, /*1024x768_60P*/
                    0x0f, /*1280x800_60P*/
                    0x0f, /*1280x1024_60P*/
                    0x0f, /*1366x768_60P*/
                    0x0f, /*1440x900_60P*/
                    0x0f, /*1680x1050_60P*/
                    0x0f, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_32_L), 0xFF, 0x0f, /*480_60P*/
                    0x0f, /*576_50P*/
                    0x0f, /*720_50P*/
                    0x0f, /*720_60P*/
                    0x0f, /*1080_24P*/
                    0x0f, /*1080_25P*/
                    0x0f, /*1080_30P*/
                    0x0f, /*1080_50P*/
                    0x0f, /*1080_60P*/
                    0x08, /*3840x2160_30P*/
                    0x0f, /*1024x768_60P*/
                    0x0f, /*1280x800_60P*/
                    0x0f, /*1280x1024_60P*/
                    0x0f, /*1366x768_60P*/
                    0x0f, /*1440x900_60P*/
                    0x0f, /*1680x1050_60P*/
                    0x0f, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_39_H), 0xFF, 0x03, /*480_60P*/
                    0x03, /*576_50P*/
                    0x03, /*720_50P*/
                    0x03, /*720_60P*/
                    0x03, /*1080_24P*/
                    0x03, /*1080_25P*/
                    0x03, /*1080_30P*/
                    0x03, /*1080_50P*/
                    0x03, /*1080_60P*/
                    0x33, /*3840x2160_30P*/
                    0x03, /*1024x768_60P*/
                    0x03, /*1280x800_60P*/
                    0x03, /*1280x1024_60P*/
                    0x03, /*1366x768_60P*/
                    0x03, /*1440x900_60P*/
                    0x03, /*1680x1050_60P*/
                    0x03, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_3B_L), 0xFF, 0x22, /*480_60P*/
                    0x22, /*576_50P*/
                    0x22, /*720_50P*/
                    0x22, /*720_60P*/
                    0x22, /*1080_24P*/
                    0x22, /*1080_25P*/
                    0x22, /*1080_30P*/
                    0x22, /*1080_50P*/
                    0x22, /*1080_60P*/
                    0x66, /*3840x2160_30P*/
                    0x22, /*1024x768_60P*/
                    0x22, /*1280x800_60P*/
                    0x22, /*1280x1024_60P*/
                    0x22, /*1366x768_60P*/
                    0x22, /*1440x900_60P*/
                    0x22, /*1680x1050_60P*/
                    0x22, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_3B_H), 0xFF, 0x22, /*480_60P*/
                    0x22, /*576_50P*/
                    0x22, /*720_50P*/
                    0x22, /*720_60P*/
                    0x22, /*1080_24P*/
                    0x22, /*1080_25P*/
                    0x22, /*1080_30P*/
                    0x22, /*1080_50P*/
                    0x22, /*1080_60P*/
                    0x26, /*3840x2160_30P*/
                    0x22, /*1024x768_60P*/
                    0x22, /*1280x800_60P*/
                    0x22, /*1280x1024_60P*/
                    0x22, /*1366x768_60P*/
                    0x22, /*1440x900_60P*/
                    0x22, /*1680x1050_60P*/
                    0x22, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_TB_1D_L), 0xFF, 0x13, /*480_60P*/
                    0x13, /*576_50P*/
                    0x12, /*720_50P*/
                    0x12, /*720_60P*/
                    0x12, /*1080_24P*/
                    0x12, /*1080_25P*/
                    0x12, /*1080_30P*/
                    0x11, /*1080_50P*/
                    0x11, /*1080_60P*/
                    0x10, /*3840x2160_30P*/
                    0x12, /*1024x768_60P*/
                    0x12, /*1280x800_60P*/
                    0x12, /*1280x1024_60P*/
                    0x12, /*1366x768_60P*/
                    0x12, /*1440x900_60P*/
                    0x12, /*1680x1050_60P*/
                    0x11, /*1600x1200_60P*/
 },
};

//****************************************************
// INIT_FPLL_CTRL
//****************************************************
u8 ST_INIT_FPLL_CTRL_COMMON_TBL[INIT_FPLL_CTRL_COMMON_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + REG_DATA_SIZE] =
{
 { DRV_REG(REG_DISP_TOP_TB_1B_H), 0xFF, 0x08,
 },
 { DRV_REG(REG_DISP_TOP_OP2_TB_10_L), 0xFF, 0xD0,
 },
 { DRV_REG(REG_DISP_TOP_OP2_TB_10_H), 0xFF, 0x40,
 },
 { DRV_REG(REG_DAC_ATOP_TB_34_L), 0xFF, 0x02,
 },
 { DRV_REG(REG_DAC_ATOP_TB_35_L), 0xFF, 0x00,
 },
};

u8 ST_INIT_FPLL_CTRL_TBL[INIT_FPLL_CTRL_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + HAL_DISP_VGA_ID_NUM]=
{
 { DRV_REG(REG_DISP_TOP_TB_10_L), 0xFF, 0xA1, /*480_60P*/
                    0x00, /*576_50P*/
                    0xA2, /*720_50P*/
                    0xA2, /*720_60P*/
                    0xA2, /*1080_24P*/
                    0xA2, /*1080_25P*/
                    0xA2, /*1080_30P*/
                    0xA2, /*1080_50P*/
                    0xA2, /*1080_60P*/
                    0xA2, /*3840x2160_30P*/
                    0x31, /*1024x768_60P*/
                    0x25, /*1280x800_60P*/
                    0xB1, /*1280x1024_60P*/
                    0x7E, /*1366x768_60P*/
                    0xCB, /*1440x900_60P*/
                    0xDD, /*1680x1050_60P*/
                    0xAA, /*1600x1200_60P*/
 },
 { DRV_REG(REG_DISP_TOP_TB_10_H), 0xFF, 0xEF, /*480_60P*/
                    0x00, /*576_50P*/
                    0x8B, /*720_50P*/
                    0x8B, /*720_60P*/
                    0x8B, /*1080_24P*/
                    0x8B, /*1080_25P*/
                    0x8B, /*1080_30P*/
                    0x8B, /*1080_50P*/
                    0x8B, /*1080_60P*/
                    0x8B, /*3840x2160_30P*/
                    0x2C, /*1024x768_60P*/
                    0x42, /*1280x800_60P*/
                    0x02, /*1280x1024_60P*/
                    0x47, /*1366x768_60P*/
                    0x63, /*1440x900_60P*/
                    0x9C, /*1680x1050_60P*/
                    0xAA, /*1600x1200_60P*/
 },
 { DRV_REG(REG_DISP_TOP_TB_11_L), 0xFF, 0x3F, /*480_60P*/
                    0x40, /*576_50P*/
                    0x2E, /*720_50P*/
                    0x2E, /*720_60P*/
                    0x2E, /*1080_24P*/
                    0x2E, /*1080_25P*/
                    0x2E, /*1080_30P*/
                    0x2E, /*1080_50P*/
                    0x2E, /*1080_60P*/
                    0x2E, /*3840x2160_30P*/
                    0x35, /*1024x768_60P*/
                    0x29, /*1280x800_60P*/
                    0x20, /*1280x1024_60P*/
                    0x28, /*1366x768_60P*/
                    0x20, /*1440x900_60P*/
                    0x17, /*1680x1050_60P*/
                    0x2A, /*1600x1200_60P*/
 },
 { DRV_REG(REG_DISP_TOP_TB_1A_L), 0xFF, 0xC0, /*480_60P*/
                    0xC0, /*576_50P*/
                    0xD3, /*720_50P*/
                    0xB1, /*720_60P*/
                    0x5B, /*1080_24P*/
                    0x69, /*1080_25P*/
                    0xB1, /*1080_30P*/
                    0x69, /*1080_50P*/
                    0x63, /*1080_60P*/
                    0x63, /*3840x2160_30P*/
                    0xEF, /*1024x768_60P*/
                    0x01, /*1280x800_60P*/
                    0x54, /*1280x1024_60P*/
                    0xEE, /*1366x768_60P*/
                    0xD4, /*1440x900_60P*/
                    0xB8, /*1680x1050_60P*/
                    0x8D, /*1600x1200_60P*/
 },
 { DRV_REG(REG_DISP_TOP_TB_1A_H), 0xFF, 0x00, /*480_60P*/
                    0x00, /*576_50P*/
                    0x02, /*720_50P*/
                    0x01, /*720_60P*/
                    0x01, /*1080_24P*/
                    0x01, /*1080_25P*/
                    0x01, /*1080_30P*/
                    0x01, /*1080_50P*/
                    0x03, /*1080_60P*/
                    0x03, /*3840x2160_30P*/
                    0x01, /*1024x768_60P*/
                    0x03, /*1280x800_60P*/
                    0x02, /*1280x1024_60P*/
                    0x02, /*1366x768_60P*/
                    0x02, /*1440x900_60P*/
                    0x01, /*1680x1050_60P*/
                    0x01, /*1600x1200_60P*/
 },
 { DRV_REG(REG_DAC_ATOP_TB_33_L), 0xFF, 0x00, /*480_60P*/
                    0x00, /*576_50P*/
                    0x00, /*720_50P*/
                    0x00, /*720_60P*/
                    0x00, /*1080_24P*/
                    0x00, /*1080_25P*/
                    0x00, /*1080_30P*/
                    0x00, /*1080_50P*/
                    0x00, /*1080_60P*/
                    0x00, /*3840x2160_30P*/
                    0x00, /*1024x768_60P*/
                    0x00, /*1280x800_60P*/
                    0x01, /*1280x1024_60P*/
                    0x00, /*1366x768_60P*/
                    0x00, /*1440x900_60P*/
                    0x00, /*1680x1050_60P*/
                    0x00, /*1600x1200_60P*/
 },
 { DRV_REG(REG_DAC_ATOP_TB_37_L), 0xFF, 0x04, /*480_60P*/
                    0x04, /*576_50P*/
                    0x02, /*720_50P*/
                    0x02, /*720_60P*/
                    0x02, /*1080_24P*/
                    0x02, /*1080_25P*/
                    0x02, /*1080_30P*/
                    0x01, /*1080_50P*/
                    0x01, /*1080_60P*/
                    0x01, /*3840x2160_30P*/
                    0x02, /*1024x768_60P*/
                    0x02, /*1280x800_60P*/
                    0x01, /*1280x1024_60P*/
                    0x02, /*1366x768_60P*/
                    0x02, /*1440x900_60P*/
                    0x02, /*1680x1050_60P*/
                    0x01, /*1600x1200_60P*/
 },
};

//****************************************************
// INIT_IDAC_ATOP
//****************************************************
u8 ST_INIT_IDAC_ATOP_COMMON_TBL[INIT_IDAC_ATOP_COMMON_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + REG_DATA_SIZE] =
{
 { DRV_REG(REG_DAC_ATOP_TB_15_L), 0xFF, 0x07,
 },
 { DRV_REG(REG_DAC_ATOP_TB_16_L), 0xFF, 0x01,
 },
 { DRV_REG(REG_DAC_ATOP_TB_1F_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_DAC_ATOP_TB_00_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_DAC_ATOP_TB_1D_L), 0x01, 0x01,
 },
 { DRV_REG(REG_DAC_ATOP_TB_29_L), 0x01, 0x01,
 },
 { DRV_REG(REG_DAC_ATOP_TB_36_L), 0xFF, 0x00,
 },
};

u8 ST_INIT_IDAC_ATOP_TBL[INIT_IDAC_ATOP_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + HAL_DISP_VGA_ID_NUM]=
{
 { DRV_REG(REG_DAC_ATOP_TB_0B_L), 0x01, 0x00, /*480_60P*/
                    0x00, /*576_50P*/
                    0x00, /*720_50P*/
                    0x00, /*720_60P*/
                    0x00, /*1080_24P*/
                    0x00, /*1080_25P*/
                    0x00, /*1080_30P*/
                    0x00, /*1080_50P*/
                    0x00, /*1080_60P*/
                    0x00, /*3840x2160_30P*/
                    0x01, /*1024x768_60P*/
                    0x01, /*1280x800_60P*/
                    0x00, /*1280x1024_60P*/
                    0x00, /*1366x768_60P*/
                    0x01, /*1440x900_60P*/
                    0x01, /*1680x1050_60P*/
                    0x00, /*1600x1200_60P*/
 },
 { DRV_REG(REG_DAC_ATOP_TB_0B_L), 0x02, 0x00, /*480_60P*/
                    0x00, /*576_50P*/
                    0x00, /*720_50P*/
                    0x00, /*720_60P*/
                    0x00, /*1080_24P*/
                    0x00, /*1080_25P*/
                    0x00, /*1080_30P*/
                    0x00, /*1080_50P*/
                    0x00, /*1080_60P*/
                    0x00, /*3840x2160_30P*/
                    0x02, /*1024x768_60P*/
                    0x00, /*1280x800_60P*/
                    0x00, /*1280x1024_60P*/
                    0x00, /*1366x768_60P*/
                    0x00, /*1440x900_60P*/
                    0x00, /*1680x1050_60P*/
                    0x00, /*1600x1200_60P*/
 },
};

//****************************************************
// INIT_DAC_TGEN
//****************************************************
u8 ST_INIT_DAC_TGEN_COMMON_TBL[INIT_DAC_TGEN_COMMON_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + REG_DATA_SIZE] =
{
 { DRV_REG(REG_DISP_TOP_OP2_TB_23_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_DISP_TOP_OP2_TB_23_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_DISP_TOP_OP2_TB_24_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_DISP_TOP_OP2_TB_29_H), 0xFF, 0x00,
 },
};

u8 ST_INIT_DAC_TGEN_TBL[INIT_DAC_TGEN_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + HAL_DISP_VGA_ID_NUM]=
{
 { DRV_REG(REG_DISP_TOP_OP2_TB_24_L), 0xFF, 0x3D, /*480_60P*/
                    0x3F, /*576_50P*/
                    0x27, /*720_50P*/
                    0x27, /*720_60P*/
                    0x2B, /*1080_24P*/
                    0x2B, /*1080_25P*/
                    0x2B, /*1080_30P*/
                    0x2B, /*1080_50P*/
                    0x2B, /*1080_60P*/
                    0x57, /*3840x2160_30P*/
                    0x87, /*1024x768_60P*/
                    0x7F, /*1280x800_60P*/
                    0x6F, /*1280x1024_60P*/
                    0x8E, /*1366x768_60P*/
                    0x97, /*1440x900_60P*/
                    0xAF, /*1680x1050_60P*/
                    0xBF, /*1600x1200_60P*/
 },
 { DRV_REG(REG_DISP_TOP_OP2_TB_27_L), 0xFF, 0x7A, /*480_60P*/
                    0x84, /*576_50P*/
                    0x04, /*720_50P*/
                    0x04, /*720_60P*/
                    0xC0, /*1080_24P*/
                    0xC0, /*1080_25P*/
                    0xC0, /*1080_30P*/
                    0xC0, /*1080_50P*/
                    0xC0, /*1080_60P*/
                    0x80, /*3840x2160_30P*/
                    0x28, /*1024x768_60P*/
                    0x48, /*1280x800_60P*/
                    0x68, /*1280x1024_60P*/
                    0x64, /*1366x768_60P*/
                    0x80, /*1440x900_60P*/
                    0xC8, /*1680x1050_60P*/
                    0xF0, /*1600x1200_60P*/
 },
 { DRV_REG(REG_DISP_TOP_OP2_TB_27_H), 0xFF, 0x00, /*480_60P*/
                    0x00, /*576_50P*/
                    0x01, /*720_50P*/
                    0x01, /*720_60P*/
                    0x00, /*1080_24P*/
                    0x00, /*1080_25P*/
                    0x00, /*1080_30P*/
                    0x00, /*1080_50P*/
                    0x00, /*1080_60P*/
                    0x01, /*3840x2160_30P*/
                    0x01, /*1024x768_60P*/
                    0x01, /*1280x800_60P*/
                    0x01, /*1280x1024_60P*/
                    0x01, /*1366x768_60P*/
                    0x01, /*1440x900_60P*/
                    0x01, /*1680x1050_60P*/
                    0x01, /*1600x1200_60P*/
 },
 { DRV_REG(REG_DISP_TOP_OP2_TB_28_L), 0xFF, 0x49, /*480_60P*/
                    0x53, /*576_50P*/
                    0x03, /*720_50P*/
                    0x03, /*720_60P*/
                    0x3F, /*1080_24P*/
                    0x3F, /*1080_25P*/
                    0x3F, /*1080_30P*/
                    0x3F, /*1080_50P*/
                    0x3F, /*1080_60P*/
                    0x7F, /*3840x2160_30P*/
                    0x27, /*1024x768_60P*/
                    0x47, /*1280x800_60P*/
                    0x67, /*1280x1024_60P*/
                    0xB9, /*1366x768_60P*/
                    0x1F, /*1440x900_60P*/
                    0x57, /*1680x1050_60P*/
                    0x2F, /*1600x1200_60P*/
 },
 { DRV_REG(REG_DISP_TOP_OP2_TB_28_H), 0xFF, 0x03, /*480_60P*/
                    0x03, /*576_50P*/
                    0x06, /*720_50P*/
                    0x06, /*720_60P*/
                    0x08, /*1080_24P*/
                    0x08, /*1080_25P*/
                    0x08, /*1080_30P*/
                    0x08, /*1080_50P*/
                    0x08, /*1080_60P*/
                    0x10, /*3840x2160_30P*/
                    0x05, /*1024x768_60P*/
                    0x06, /*1280x800_60P*/
                    0x06, /*1280x1024_60P*/
                    0x06, /*1366x768_60P*/
                    0x07, /*1440x900_60P*/
                    0x08, /*1680x1050_60P*/
                    0x08, /*1600x1200_60P*/
 },
 { DRV_REG(REG_DISP_TOP_OP2_TB_29_L), 0xFF, 0x24, /*480_60P*/
                    0x2C, /*576_50P*/
                    0x19, /*720_50P*/
                    0x19, /*720_60P*/
                    0x29, /*1080_24P*/
                    0x29, /*1080_25P*/
                    0x29, /*1080_30P*/
                    0x29, /*1080_50P*/
                    0x29, /*1080_60P*/
                    0x52, /*3840x2160_30P*/
                    0x23, /*1024x768_60P*/
                    0x1C, /*1280x800_60P*/
                    0x29, /*1280x1024_60P*/
                    0x1B, /*1366x768_60P*/
                    0x1F, /*1440x900_60P*/
                    0x24, /*1680x1050_60P*/
                    0x31, /*1600x1200_60P*/
 },
 { DRV_REG(REG_DISP_TOP_OP2_TB_2A_L), 0xFF, 0x03, /*480_60P*/
                    0x6B, /*576_50P*/
                    0xE8, /*720_50P*/
                    0xE8, /*720_60P*/
                    0x60, /*1080_24P*/
                    0x60, /*1080_25P*/
                    0x60, /*1080_30P*/
                    0x60, /*1080_50P*/
                    0x60, /*1080_60P*/
                    0xC1, /*3840x2160_30P*/
                    0x22, /*1024x768_60P*/
                    0x3B, /*1280x800_60P*/
                    0x28, /*1280x1024_60P*/
                    0x1A, /*1366x768_60P*/
                    0xA2, /*1440x900_60P*/
                    0x3D, /*1680x1050_60P*/
                    0xE0, /*1600x1200_60P*/
 },
 { DRV_REG(REG_DISP_TOP_OP2_TB_2A_H), 0xFF, 0x02, /*480_60P*/
                    0x02, /*576_50P*/
                    0x02, /*720_50P*/
                    0x02, /*720_60P*/
                    0x04, /*1080_24P*/
                    0x04, /*1080_25P*/
                    0x04, /*1080_30P*/
                    0x04, /*1080_50P*/
                    0x04, /*1080_60P*/
                    0x08, /*3840x2160_30P*/
                    0x03, /*1024x768_60P*/
                    0x03, /*1280x800_60P*/
                    0x04, /*1280x1024_60P*/
                    0x03, /*1366x768_60P*/
                    0x03, /*1440x900_60P*/
                    0x04, /*1680x1050_60P*/
                    0x04, /*1600x1200_60P*/
 },
};

HAL_DISP_VGA_INFO stHAL_DISP_VGA_TIMING_TBL[HAL_DISP_VGA_TAB_NUM]=
{
    {*ST_INIT_HDMITX_ATOP_TBL,INIT_HDMITX_ATOP_REG_NUM, HAL_DISP_VGA_IP_NORMAL},
    {*ST_INIT_HDMITX_ATOP_COMMON_TBL,INIT_HDMITX_ATOP_COMMON_REG_NUM, HAL_DISP_VGA_IP_COMMON},
    {*ST_INIT_FPLL_CTRL_TBL,INIT_FPLL_CTRL_REG_NUM, HAL_DISP_VGA_IP_NORMAL},
    {*ST_INIT_FPLL_CTRL_COMMON_TBL,INIT_FPLL_CTRL_COMMON_REG_NUM, HAL_DISP_VGA_IP_COMMON},
    {*ST_INIT_IDAC_ATOP_TBL,INIT_IDAC_ATOP_REG_NUM, HAL_DISP_VGA_IP_NORMAL},
    {*ST_INIT_IDAC_ATOP_COMMON_TBL,INIT_IDAC_ATOP_COMMON_REG_NUM, HAL_DISP_VGA_IP_COMMON},
    {*ST_INIT_DAC_TGEN_TBL,INIT_DAC_TGEN_REG_NUM, HAL_DISP_VGA_IP_NORMAL},
    {*ST_INIT_DAC_TGEN_COMMON_TBL,INIT_DAC_TGEN_COMMON_REG_NUM, HAL_DISP_VGA_IP_COMMON},
};
#endif

