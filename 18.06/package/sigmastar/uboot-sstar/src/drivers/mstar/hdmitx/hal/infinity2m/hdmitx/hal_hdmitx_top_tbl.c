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
// Drive Chip           : CAMDRV_HDMITX
// Excel CodeGen Version: 1.05
// Excel SW      Version: 1.01
// Excel update date    : 2019/9/23 17:41
//****************************************************

#ifndef _HAL_HDMITX_TOP_TBL_C_
#define _HAL_HDMITX_TOP_TBL_C_

#include "hal_hdmitx_top_tbl.h" 

//****************************************************
// HDMITX_ATOP
//****************************************************
MS_U8 ST_HDMITX_ATOP_COMMON_TBL[HDMITX_ATOP_COMMON_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + REG_DATA_SIZE] =
{
 { DRV_REG(REG_HDMITX_ATOP_17_L), 0xFF, 0x08,
 },
 { DRV_REG(REG_HDMITX_ATOP_20_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_20_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_21_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_21_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_25_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_2C_L), 0xFF, 0x0f,
 },
 { DRV_REG(REG_HDMITX_ATOP_30_L), 0xFF, 0x0f,
 },
 { DRV_REG(REG_HDMITX_ATOP_31_L), 0xFF, 0x0f,
 },
 { DRV_REG(REG_HDMITX_ATOP_32_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_33_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_34_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_35_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_39_L), 0xFF, 0x33,
 },
 { DRV_REG(REG_HDMITX_ATOP_00_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_ATOP_1C_L), 0xFF, 0x01,
 },
 { DRV_REG(REG_HDMITX_ATOP_1C_H), 0xFF, 0x05,
 },
 { DRV_REG(REG_DISP_TOP_00_H), 0xFF, 0x00,
 },
};

MS_U8 ST_HDMITX_ATOP_TBL[HDMITX_ATOP_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + HAL_HDMITX_ID_NUM]=
{
 { DRV_REG(REG_HDMITX_ATOP_16_H), 0xFF, 0x00, /*480_60P*/
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
                    0x00, /*1280x1024_60P*/
                    0x00, /*1366x768_60P*/
                    0x00, /*1440x900_60P*/
                    0x00, /*1280x800_60P*/
                    0x00, /*1680x1050_60P*/
                    0x00, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_22_L), 0xFF, 0x14, /*480_60P*/
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
                    0x14, /*1280x1024_60P*/
                    0x14, /*1366x768_60P*/
                    0x14, /*1440x900_60P*/
                    0x14, /*1280x800_60P*/
                    0x14, /*1680x1050_60P*/
                    0x14, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_22_H), 0xFF, 0x14, /*480_60P*/
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
                    0x14, /*1280x1024_60P*/
                    0x14, /*1366x768_60P*/
                    0x14, /*1440x900_60P*/
                    0x14, /*1280x800_60P*/
                    0x14, /*1680x1050_60P*/
                    0x14, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_23_L), 0xFF, 0x14, /*480_60P*/
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
                    0x14, /*1280x1024_60P*/
                    0x14, /*1366x768_60P*/
                    0x14, /*1440x900_60P*/
                    0x14, /*1280x800_60P*/
                    0x14, /*1680x1050_60P*/
                    0x14, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_23_H), 0xFF, 0x14, /*480_60P*/
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
                    0x14, /*1280x1024_60P*/
                    0x14, /*1366x768_60P*/
                    0x14, /*1440x900_60P*/
                    0x14, /*1280x800_60P*/
                    0x14, /*1680x1050_60P*/
                    0x14, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_24_L), 0xFF, 0x00, /*480_60P*/
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
                    0x00, /*1280x1024_60P*/
                    0x00, /*1366x768_60P*/
                    0x00, /*1440x900_60P*/
                    0x00, /*1280x800_60P*/
                    0x00, /*1680x1050_60P*/
                    0x00, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_24_H), 0xFF, 0x00, /*480_60P*/
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
                    0x00, /*1280x1024_60P*/
                    0x00, /*1366x768_60P*/
                    0x00, /*1440x900_60P*/
                    0x00, /*1280x800_60P*/
                    0x00, /*1680x1050_60P*/
                    0x00, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_25_L), 0xFF, 0x00, /*480_60P*/
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
                    0x00, /*1280x1024_60P*/
                    0x00, /*1366x768_60P*/
                    0x00, /*1440x900_60P*/
                    0x00, /*1280x800_60P*/
                    0x00, /*1680x1050_60P*/
                    0x00, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_30_H), 0xFF, 0x0f, /*480_60P*/
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
                    0x0f, /*1280x1024_60P*/
                    0x0f, /*1366x768_60P*/
                    0x0f, /*1440x900_60P*/
                    0x0f, /*1280x800_60P*/
                    0x0f, /*1680x1050_60P*/
                    0x0f, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_31_H), 0xFF, 0x0f, /*480_60P*/
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
                    0x0f, /*1280x1024_60P*/
                    0x0f, /*1366x768_60P*/
                    0x0f, /*1440x900_60P*/
                    0x0f, /*1280x800_60P*/
                    0x0f, /*1680x1050_60P*/
                    0x0f, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_32_L), 0xFF, 0x0f, /*480_60P*/
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
                    0x0f, /*1280x1024_60P*/
                    0x0f, /*1366x768_60P*/
                    0x0f, /*1440x900_60P*/
                    0x0f, /*1280x800_60P*/
                    0x0f, /*1680x1050_60P*/
                    0x0f, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_39_H), 0xFF, 0x03, /*480_60P*/
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
                    0x03, /*1280x1024_60P*/
                    0x03, /*1366x768_60P*/
                    0x03, /*1440x900_60P*/
                    0x03, /*1280x800_60P*/
                    0x03, /*1680x1050_60P*/
                    0x03, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_3B_L), 0xFF, 0x22, /*480_60P*/
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
                    0x22, /*1280x1024_60P*/
                    0x22, /*1366x768_60P*/
                    0x22, /*1440x900_60P*/
                    0x22, /*1280x800_60P*/
                    0x22, /*1680x1050_60P*/
                    0x22, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_3B_H), 0xFF, 0x22, /*480_60P*/
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
                    0x22, /*1280x1024_60P*/
                    0x22, /*1366x768_60P*/
                    0x22, /*1440x900_60P*/
                    0x22, /*1280x800_60P*/
                    0x22, /*1680x1050_60P*/
                    0x22, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_ATOP_1D_L), 0xFF, 0x13, /*480_60P*/
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
                    0x12, /*1280x1024_60P*/
                    0x12, /*1366x768_60P*/
                    0x12, /*1440x900_60P*/
                    0x12, /*1280x800_60P*/
                    0x12, /*1680x1050_60P*/
                    0x11, /*1600x1200_60P*/
 },
};

//****************************************************
// HDMITX_DTOP_TIMING
//****************************************************
MS_U8 ST_HDMITX_DTOP_TIMING_COMMON_TBL[HDMITX_DTOP_TIMING_COMMON_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + REG_DATA_SIZE] =
{
 { DRV_REG(REG_HDMITX_DTOP_12_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_DTOP_12_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_DTOP_13_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_DTOP_14_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_DTOP_14_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_DTOP_15_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_DTOP_18_H), 0xFF, 0x00,
 },
};

MS_U8 ST_HDMITX_DTOP_TIMING_TBL[HDMITX_DTOP_TIMING_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + HAL_HDMITX_ID_NUM]=
{
 { DRV_REG(REG_HDMITX_DTOP_10_L), 0xFF, 0x59, /*480_60P*/
                    0x5F, /*576_50P*/
                    0xBB, /*720_50P*/
                    0x71, /*720_60P*/
                    0xBD, /*1080_24P*/
                    0x4F, /*1080_25P*/
                    0x97, /*1080_30P*/
                    0x4F, /*1080_50P*/
                    0x97, /*1080_60P*/
                    0x2F, /*3840x2160_30P*/
                    0x3F, /*1024x768_60P*/
                    0x97, /*1280x1024_60P*/
                    0xFF, /*1366x768_60P*/
                    0x6F, /*1440x900_60P*/
                    0x8F, /*1280x800_60P*/
                    0xBF, /*1680x1050_60P*/
                    0x6F, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_DTOP_10_H), 0xFF, 0x03, /*480_60P*/
                    0x03, /*576_50P*/
                    0x07, /*720_50P*/
                    0x06, /*720_60P*/
                    0xA, /*1080_24P*/
                    0xA, /*1080_25P*/
                    0x08, /*1080_30P*/
                    0xA, /*1080_50P*/
                    0x08, /*1080_60P*/
                    0x11, /*3840x2160_30P*/
                    0x05, /*1024x768_60P*/
                    0x06, /*1280x1024_60P*/
                    0x06, /*1366x768_60P*/
                    0x07, /*1440x900_60P*/
                    0x06, /*1280x800_60P*/
                    0x08, /*1680x1050_60P*/
                    0x08, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_DTOP_11_L), 0xFF, 0xC, /*480_60P*/
                    0x70, /*576_50P*/
                    0xED, /*720_50P*/
                    0xED, /*720_60P*/
                    0x64, /*1080_24P*/
                    0x64, /*1080_25P*/
                    0x64, /*1080_30P*/
                    0x64, /*1080_50P*/
                    0x64, /*1080_60P*/
                    0xC9, /*3840x2160_30P*/
                    0x25, /*1024x768_60P*/
                    0x29, /*1280x1024_60P*/
                    0x1D, /*1366x768_60P*/
                    0xA5, /*1440x900_60P*/
                    0x3E, /*1280x800_60P*/
                    0x40, /*1680x1050_60P*/
                    0xE1, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_DTOP_11_H), 0xFF, 0x02, /*480_60P*/
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
                    0x04, /*1280x1024_60P*/
                    0x03, /*1366x768_60P*/
                    0x03, /*1440x900_60P*/
                    0x03, /*1280x800_60P*/
                    0x04, /*1680x1050_60P*/
                    0x04, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_DTOP_13_L), 0xFF, 0x3D, /*480_60P*/
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
                    0x6F, /*1280x1024_60P*/
                    0x8E, /*1366x768_60P*/
                    0x97, /*1440x900_60P*/
                    0x7F, /*1280x800_60P*/
                    0xAF, /*1680x1050_60P*/
                    0xBF, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_DTOP_15_L), 0xFF, 0x05, /*480_60P*/
                    0x04, /*576_50P*/
                    0x04, /*720_50P*/
                    0x04, /*720_60P*/
                    0x04, /*1080_24P*/
                    0x04, /*1080_25P*/
                    0x04, /*1080_30P*/
                    0x04, /*1080_50P*/
                    0x04, /*1080_60P*/
                    0x09, /*3840x2160_30P*/
                    0x05, /*1024x768_60P*/
                    0x02, /*1280x1024_60P*/
                    0x02, /*1366x768_60P*/
                    0x05, /*1440x900_60P*/
                    0x05, /*1280x800_60P*/
                    0x05, /*1680x1050_60P*/
                    0x02, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_DTOP_16_L), 0xFF, 0x7A, /*480_60P*/
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
                    0x68, /*1280x1024_60P*/
                    0x66, /*1366x768_60P*/
                    0x80, /*1440x900_60P*/
                    0x48, /*1280x800_60P*/
                    0xC8, /*1680x1050_60P*/
                    0xF0, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_DTOP_16_H), 0xFF, 0x00, /*480_60P*/
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
                    0x01, /*1280x1024_60P*/
                    0x01, /*1366x768_60P*/
                    0x01, /*1440x900_60P*/
                    0x01, /*1280x800_60P*/
                    0x01, /*1680x1050_60P*/
                    0x01, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_DTOP_17_L), 0xFF, 0x49, /*480_60P*/
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
                    0x67, /*1280x1024_60P*/
                    0xBB, /*1366x768_60P*/
                    0x1F, /*1440x900_60P*/
                    0x47, /*1280x800_60P*/
                    0x57, /*1680x1050_60P*/
                    0x2F, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_DTOP_17_H), 0xFF, 0x03, /*480_60P*/
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
                    0x06, /*1280x1024_60P*/
                    0x06, /*1366x768_60P*/
                    0x07, /*1440x900_60P*/
                    0x06, /*1280x800_60P*/
                    0x08, /*1680x1050_60P*/
                    0x08, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_DTOP_18_L), 0xFF, 0x24, /*480_60P*/
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
                    0x29, /*1280x1024_60P*/
                    0x1B, /*1366x768_60P*/
                    0x1F, /*1440x900_60P*/
                    0x1C, /*1280x800_60P*/
                    0x24, /*1680x1050_60P*/
                    0x31, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_DTOP_19_L), 0xFF, 0x03, /*480_60P*/
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
                    0x28, /*1280x1024_60P*/
                    0x1A, /*1366x768_60P*/
                    0xA2, /*1440x900_60P*/
                    0x3B, /*1280x800_60P*/
                    0x3D, /*1680x1050_60P*/
                    0xE0, /*1600x1200_60P*/
 },
 { DRV_REG(REG_HDMITX_DTOP_19_H), 0xFF, 0x02, /*480_60P*/
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
                    0x04, /*1280x1024_60P*/
                    0x03, /*1366x768_60P*/
                    0x03, /*1440x900_60P*/
                    0x03, /*1280x800_60P*/
                    0x04, /*1680x1050_60P*/
                    0x04, /*1600x1200_60P*/
 },
};

//****************************************************
// HDMITX_DTOP
//****************************************************
MS_U8 ST_HDMITX_DTOP_COMMON_TBL[HDMITX_DTOP_COMMON_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + REG_DATA_SIZE] =
{
 { DRV_REG(REG_HDMITX_DTOP_08_L), 0xFF, 0x2B,
 },
 { DRV_REG(REG_HDMITX_DTOP_09_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_DTOP_09_H), 0xFF, 0x7D,
 },
 { DRV_REG(REG_HDMITX_DTOP_0A_L), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_DTOP_0A_H), 0xFF, 0x10,
 },
 { DRV_REG(REG_HDMITX_DTOP_0B_H), 0xFF, 0x00,
 },
 { DRV_REG(REG_HDMITX_DTOP_1E_L), 0xFF, 0x0A,
 },
 { DRV_REG(REG_HDMITX_DTOP_1F_L), 0x01, 0x01,
 },
 { DRV_REG(REG_HDMITX_DTOP_00_L), 0xFF, 0x01,
 },
 { DRV_REG(REG_HDMITX_DTOP_01_L), 0x04, 0x04,
 },
};

MS_U8 ST_HDMITX_DTOP_TBL[HDMITX_DTOP_REG_NUM][REG_ADDR_SIZE + REG_MASK_SIZE + HAL_HDMITX_ID_NUM]=
{
 { DRV_REG(REG_HDMITX_DTOP_0B_L), 0xFF, 0x00, /*480_60P*/
                    0x00, /*576_50P*/
                    0x01, /*720_50P*/
                    0x00, /*720_60P*/
                    0x01, /*1080_24P*/
                    0x01, /*1080_25P*/
                    0x00, /*1080_30P*/
                    0x01, /*1080_50P*/
                    0x03, /*1080_60P*/
                    0x03, /*3840x2160_30P*/
                    0x00, /*1024x768_60P*/
                    0x00, /*1280x1024_60P*/
                    0x00, /*1366x768_60P*/
                    0x00, /*1440x900_60P*/
                    0x00, /*1280x800_60P*/
                    0x00, /*1680x1050_60P*/
                    0x00, /*1600x1200_60P*/
 },
};

HAL_HDMITX_INFO stHAL_HDMITX_TOP_TBL[HAL_HDMITX_TAB_NUM]=
{
    {*ST_HDMITX_ATOP_TBL,HDMITX_ATOP_REG_NUM, HAL_HDMITX_IP_NORMAL},
    {*ST_HDMITX_ATOP_COMMON_TBL,HDMITX_ATOP_COMMON_REG_NUM, HAL_HDMITX_IP_COMMON},
    {*ST_HDMITX_DTOP_TIMING_TBL,HDMITX_DTOP_TIMING_REG_NUM, HAL_HDMITX_IP_NORMAL},
    {*ST_HDMITX_DTOP_TIMING_COMMON_TBL,HDMITX_DTOP_TIMING_COMMON_REG_NUM, HAL_HDMITX_IP_COMMON},
    {*ST_HDMITX_DTOP_TBL,HDMITX_DTOP_REG_NUM, HAL_HDMITX_IP_NORMAL},
    {*ST_HDMITX_DTOP_COMMON_TBL,HDMITX_DTOP_COMMON_REG_NUM, HAL_HDMITX_IP_COMMON},
};
#endif

