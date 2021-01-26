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

#include <linux/delay.h>

#include "hal_rgn_common.h"
#include "hal_rgn_util.h"

#define SCL_ADDR_DIV   8
#define DIP_ADDR_DIV   16
#define LDC_R_ADDR_DIV 32
#define LDC_W_ADDR_DIV 8
#define RGN_UT_LBYTE(addr,div_val)  ((addr/div_val)&0xFFFF)
#define RGN_UT_HBYTE(addr,div_val)  (((addr/div_val)>>16)&0xFFFF)

#define RGN_UT_R2BYTE(u32Reg) (RIU_READ_2BYTE((u32Reg) << 1))
#define RGN_UT_W2BYTE_MSK(u32Reg,u16Val,u16Mask) \
    RIU_WRITE_2BYTE((u32Reg)<< 1 , (RGN_UT_R2BYTE(u32Reg) & ~(u16Mask)) | ((u16Val) & (u16Mask)))

void RgnUtSclScript(MS_U32 u32MiuAddr0_Y, MS_U32 u32MiuAddr0_C,
                    MS_U32 u32MiuAddr1_Y, MS_U32 u32MiuAddr1_C,
                    MS_U32 u32MiuAddr2_Y, MS_U32 u32MiuAddr2_C)
{
    RGN_UT_W2BYTE_MSK(0x121802, 0x0008, 0xFFFF); // 0[0] , sw_reset_en

    // Set SC1SC2HK FORCE
    RGN_UT_W2BYTE_MSK(0x121804, 0x8000, 0xFFFF); // open sc2_frm dma & sc1_frm dma
    RGN_UT_W2BYTE_MSK(0x121806, 0x8303, 0xFF0F); // 0xF0 is gop binding map
    
    //Check Mask
    RGN_UT_W2BYTE_MSK(0x121566, 0x0007, 0xFFFF);

    // Set Output PATGEN
    RGN_UT_W2BYTE_MSK(0x121a20, 0x0000, 0xFFFF); //10[8] , frame color

    // output Y2R
    RGN_UT_W2BYTE_MSK(0x121a32, 0x1001, 0xFFFF); // cm_en
    RGN_UT_W2BYTE_MSK(0x121a34, 0x8080, 0xFFFF); // cm_en
    RGN_UT_W2BYTE_MSK(0x121a36, 0x0cc4, 0xFFFF); // cm_11
    RGN_UT_W2BYTE_MSK(0x121a38, 0x0950, 0xFFFF); // cm_12
    RGN_UT_W2BYTE_MSK(0x121a3A, 0x3ffc, 0xFFFF); // cm_13
    RGN_UT_W2BYTE_MSK(0x121a3C, 0x397e, 0xFFFF); // cm_21
    RGN_UT_W2BYTE_MSK(0x121a3E, 0x0950, 0xFFFF); // cm_22
    RGN_UT_W2BYTE_MSK(0x121a40, 0x3cde, 0xFFFF); // cm_23
    RGN_UT_W2BYTE_MSK(0x121a42, 0x3ffe, 0xFFFF); // cm_31
    RGN_UT_W2BYTE_MSK(0x121a44, 0x0950, 0xFFFF); // cm_32
    RGN_UT_W2BYTE_MSK(0x121a46, 0x1024, 0xFFFF); // cm_33

    // Set INPUT PATGEN     256x480
    RGN_UT_W2BYTE_MSK(0x121882, 0x0030, 0xFFFF);
    
    RGN_UT_W2BYTE_MSK(0x1218E0, 0x0003, 0xFFFF); // input tgen mode
    RGN_UT_W2BYTE_MSK(0x1218E2, 0x0001, 0xFFFF); // vs st
    RGN_UT_W2BYTE_MSK(0x1218E4, 0x0003, 0xFFFF); // vs end
    RGN_UT_W2BYTE_MSK(0x1218E6, 0x0021, 0xFFFF); // v st -f
    RGN_UT_W2BYTE_MSK(0x1218E8, 0x0200, 0xFFFF); // v end -f
    RGN_UT_W2BYTE_MSK(0x1218EA, 0x0021, 0xFFFF); // v st -m
    RGN_UT_W2BYTE_MSK(0x1218EC, 0x0200, 0xFFFF); // v end -m
    RGN_UT_W2BYTE_MSK(0x1218EE, 0x0300, 0xFFFF); // v total
    RGN_UT_W2BYTE_MSK(0x1218F2, 0x0004, 0xFFFF); // hs start
    RGN_UT_W2BYTE_MSK(0x1218F4, 0x0033, 0xFFFF); // hs end
    RGN_UT_W2BYTE_MSK(0x1218F6, 0x005C, 0xFFFF); // h st -f
    RGN_UT_W2BYTE_MSK(0x1218F8, 0x037B, 0xFFFF); // h end -f
    RGN_UT_W2BYTE_MSK(0x1218FA, 0x005C, 0xFFFF); // h st -m
    RGN_UT_W2BYTE_MSK(0x1218FC, 0x037B, 0xFFFF); // h end -m
    RGN_UT_W2BYTE_MSK(0x1218FE, 0x039F, 0xFFFF); // h total
    RGN_UT_W2BYTE_MSK(0x121880, 0x8003, 0xFFFF); // pat en
    RGN_UT_W2BYTE_MSK(0x121884, 0x3C64, 0xFFFF); // Hsize Vsize
    RGN_UT_W2BYTE_MSK(0x121888, 0x0C01, 0xFFFF); // R2Y

    //SET OUTPUT TGEN
    RGN_UT_W2BYTE_MSK(0x121A02, 0x0001, 0xFFFF); // vs   start   // 1 ~ 3 (3 th VSYNC)
    RGN_UT_W2BYTE_MSK(0x121A04, 0x0003, 0xFFFF); // vs   end
    RGN_UT_W2BYTE_MSK(0x121A06, 0x0021, 0xFFFF); // v st -f  // 4 ~ 32 (29 th back porch), 33 ~ 512 (480 th display area), 513 ~ 524, 0 (13 th front porch)
    RGN_UT_W2BYTE_MSK(0x121A08, 0x0200, 0xFFFF); // v end -f
    RGN_UT_W2BYTE_MSK(0x121A0A, 0x0021, 0xFFFF); // v st -m  // No used.
    RGN_UT_W2BYTE_MSK(0x121A0C, 0x0200, 0xFFFF); // v end -m
    RGN_UT_W2BYTE_MSK(0x121A0E, 0x0300, 0xFFFF); // v total  // 524 (525 th)
    RGN_UT_W2BYTE_MSK(0x121A12, 0x0004, 0xFFFF); // hs start // 4 ~ 51 ( 48 Tcph HSYNC)
    RGN_UT_W2BYTE_MSK(0x121A14, 0x0033, 0xFFFF); // hs end
    RGN_UT_W2BYTE_MSK(0x121A16, 0x005C, 0xFFFF); // h st -f  // 52 ~ 91 ( 40 Tcph back porch), 92 ~ 891 (800 Tcph display area), 892 ~ 927, 0 ~ 3 (40 Tcph front porch)
    RGN_UT_W2BYTE_MSK(0x121A18, 0x037B, 0xFFFF); // h end -f
    RGN_UT_W2BYTE_MSK(0x121A1A, 0x005C, 0xFFFF); // h st -m  // No used.
    RGN_UT_W2BYTE_MSK(0x121A1C, 0x037B, 0xFFFF); // h end -m
    RGN_UT_W2BYTE_MSK(0x121A1E, 0x039F, 0xFFFF); // h total  // 927 (928 Tcph)
    RGN_UT_W2BYTE_MSK(0x121A48, 0x0039, 0xFFFF);

    // Crop0
    RGN_UT_W2BYTE_MSK(0x121900, 0x0002, 0xFFFF); //Crop0 type
    RGN_UT_W2BYTE_MSK(0x121902, 0x0320, 0xFFFF); //Crop0 in x size
    RGN_UT_W2BYTE_MSK(0x121904, 0x01e0, 0xFFFF); //Crop0 in y size
    RGN_UT_W2BYTE_MSK(0x121908, 0x0320, 0xFFFF); //Crop0 out x size
    RGN_UT_W2BYTE_MSK(0x12190C, 0x01e0, 0xFFFF); //Crop0 out y size
    
    // Crop10
    RGN_UT_W2BYTE_MSK(0x121910, 0x0002, 0xFFFF); //Crop10 type
    RGN_UT_W2BYTE_MSK(0x121912, 0x0320, 0xFFFF); //Crop10 in x size
    RGN_UT_W2BYTE_MSK(0x121914, 0x01e0, 0xFFFF); //Crop10 in y size
    RGN_UT_W2BYTE_MSK(0x121918, 0x0320, 0xFFFF); //Crop10 out x size
    RGN_UT_W2BYTE_MSK(0x12191C, 0x01e0, 0xFFFF); //Crop10 out y size
    
    // Crop11
    RGN_UT_W2BYTE_MSK(0x121920, 0x0002, 0xFFFF); //Crop11 type
    RGN_UT_W2BYTE_MSK(0x121922, 0x0320, 0xFFFF); //Crop11 in x size
    RGN_UT_W2BYTE_MSK(0x121924, 0x01e0, 0xFFFF); //Crop11 in y size
    RGN_UT_W2BYTE_MSK(0x121928, 0x0320, 0xFFFF); //Crop11 out x size
    RGN_UT_W2BYTE_MSK(0x12192C, 0x01e0, 0xFFFF); //Crop11 out y size
    
    // Crop2
    RGN_UT_W2BYTE_MSK(0x121930, 0x0002, 0xFFFF); //Crop2 type
    RGN_UT_W2BYTE_MSK(0x121932, 0x0320, 0xFFFF); //Crop2 in x size
    RGN_UT_W2BYTE_MSK(0x121934, 0x01e0, 0xFFFF); //Crop2 in y size
    RGN_UT_W2BYTE_MSK(0x121938, 0x0320, 0xFFFF); //Crop2 out x size
    RGN_UT_W2BYTE_MSK(0x12193C, 0x01e0, 0xFFFF); //Crop2 out y size

    // Set HVSP1 SIZE
    RGN_UT_W2BYTE_MSK(0x12110E, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x121110, 0x0000, 0xFFFF); // H Ration
    
    RGN_UT_W2BYTE_MSK(0x121112, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x121114, 0x0000, 0xFFFF); // V Ration
    
    RGN_UT_W2BYTE_MSK(0x121140, 0x0320, 0xFFFF); // H size (before scaling)
    RGN_UT_W2BYTE_MSK(0x121142, 0x01e0, 0xFFFF); // V size (before scaling)
    RGN_UT_W2BYTE_MSK(0x121144, 0x0320, 0xFFFF); // H size (after scaling)
    RGN_UT_W2BYTE_MSK(0x121146, 0x01e0, 0xFFFF); // V size (after scaling)
    
    // Set HVSP2 SIZE
    RGN_UT_W2BYTE_MSK(0x12120E, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x121210, 0x0000, 0xFFFF); // H Ration
                                       
    RGN_UT_W2BYTE_MSK(0x121212, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x121214, 0x0000, 0xFFFF); // V Ration
    
    RGN_UT_W2BYTE_MSK(0x121240, 0x0320, 0xFFFF); // H size (before scaling)
    RGN_UT_W2BYTE_MSK(0x121242, 0x01e0, 0xFFFF); // V size (before scaling)
    RGN_UT_W2BYTE_MSK(0x121244, 0x0320, 0xFFFF); // H size (after scaling)
    RGN_UT_W2BYTE_MSK(0x121246, 0x01e0, 0xFFFF); // V size (after scaling)

    // Set SC0_FRM_W DMA
    RGN_UT_W2BYTE_MSK(0x121310, RGN_UT_LBYTE(u32MiuAddr0_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr0 of Y
    RGN_UT_W2BYTE_MSK(0x121312, RGN_UT_HBYTE(u32MiuAddr0_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr0 of Y
    RGN_UT_W2BYTE_MSK(0x121314, RGN_UT_LBYTE(u32MiuAddr0_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr1 of Y
    RGN_UT_W2BYTE_MSK(0x121316, RGN_UT_HBYTE(u32MiuAddr0_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr1 of Y
    RGN_UT_W2BYTE_MSK(0x121318, RGN_UT_LBYTE(u32MiuAddr0_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr2 of Y
    RGN_UT_W2BYTE_MSK(0x12131A, RGN_UT_HBYTE(u32MiuAddr0_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr2 of Y
    RGN_UT_W2BYTE_MSK(0x12131C, RGN_UT_LBYTE(u32MiuAddr0_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr3 of Y
    RGN_UT_W2BYTE_MSK(0x12131E, RGN_UT_HBYTE(u32MiuAddr0_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr3 of Y
    RGN_UT_W2BYTE_MSK(0x121320, RGN_UT_LBYTE(u32MiuAddr0_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr0 of C
    RGN_UT_W2BYTE_MSK(0x121322, RGN_UT_HBYTE(u32MiuAddr0_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr0 of C
    RGN_UT_W2BYTE_MSK(0x121324, RGN_UT_LBYTE(u32MiuAddr0_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr1 of C
    RGN_UT_W2BYTE_MSK(0x121326, RGN_UT_HBYTE(u32MiuAddr0_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr1 of C
    RGN_UT_W2BYTE_MSK(0x121328, RGN_UT_LBYTE(u32MiuAddr0_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr2 of C
    RGN_UT_W2BYTE_MSK(0x12132A, RGN_UT_HBYTE(u32MiuAddr0_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr2 of C
    RGN_UT_W2BYTE_MSK(0x12132C, RGN_UT_LBYTE(u32MiuAddr0_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr3 of C
    RGN_UT_W2BYTE_MSK(0x12132E, RGN_UT_HBYTE(u32MiuAddr0_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr3 of C
    RGN_UT_W2BYTE_MSK(0x121330, 0x0818, 0xFFFF); // [4:3] max base idx num = 3
    RGN_UT_W2BYTE_MSK(0x121334, 0x0320, 0xFFFF); // set H size
    RGN_UT_W2BYTE_MSK(0x121336, 0x01e0, 0xFFFF); // set V size
    RGN_UT_W2BYTE_MSK(0x12133C, 0x000c, 0xFFFF); // set 444 to 420 mode
    RGN_UT_W2BYTE_MSK(0x121302, 0x8703, 0xFFFF); //[15]:dma_en

    // Set SC1_FRM_W DMA
    RGN_UT_W2BYTE_MSK(0x1213A0, RGN_UT_LBYTE(u32MiuAddr1_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr0 of Y
    RGN_UT_W2BYTE_MSK(0x1213A2, RGN_UT_HBYTE(u32MiuAddr1_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr0 of Y
    RGN_UT_W2BYTE_MSK(0x1213A4, RGN_UT_LBYTE(u32MiuAddr1_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr1 of Y
    RGN_UT_W2BYTE_MSK(0x1213A6, RGN_UT_HBYTE(u32MiuAddr1_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr1 of Y
    RGN_UT_W2BYTE_MSK(0x1213A8, RGN_UT_LBYTE(u32MiuAddr1_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr2 of Y
    RGN_UT_W2BYTE_MSK(0x1213AA, RGN_UT_HBYTE(u32MiuAddr1_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr2 of Y
    RGN_UT_W2BYTE_MSK(0x1213AC, RGN_UT_LBYTE(u32MiuAddr1_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr3 of Y
    RGN_UT_W2BYTE_MSK(0x1213AE, RGN_UT_HBYTE(u32MiuAddr1_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr3 of Y
    RGN_UT_W2BYTE_MSK(0x1213B0, RGN_UT_LBYTE(u32MiuAddr1_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr0 of C
    RGN_UT_W2BYTE_MSK(0x1213B2, RGN_UT_HBYTE(u32MiuAddr1_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr0 of C
    RGN_UT_W2BYTE_MSK(0x1213B4, RGN_UT_LBYTE(u32MiuAddr1_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr1 of C
    RGN_UT_W2BYTE_MSK(0x1213B6, RGN_UT_HBYTE(u32MiuAddr1_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr1 of C
    RGN_UT_W2BYTE_MSK(0x1213B8, RGN_UT_LBYTE(u32MiuAddr1_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr2 of C
    RGN_UT_W2BYTE_MSK(0x1213BA, RGN_UT_HBYTE(u32MiuAddr1_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr2 of C
    RGN_UT_W2BYTE_MSK(0x1213BC, RGN_UT_LBYTE(u32MiuAddr1_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr3 of C
    RGN_UT_W2BYTE_MSK(0x1213BE, RGN_UT_HBYTE(u32MiuAddr1_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr3 of C
    RGN_UT_W2BYTE_MSK(0x1213C0, 0x0818, 0xFFFF); // [4:3] max base idx num = 3
    RGN_UT_W2BYTE_MSK(0x1213C4, 0x0320, 0xFFFF); // set H size
    RGN_UT_W2BYTE_MSK(0x1213C6, 0x01E0, 0xFFFF); // set V size
    RGN_UT_W2BYTE_MSK(0x1213CC, 0x000c, 0xFFFF); // set 444 to 420 mode
    RGN_UT_W2BYTE_MSK(0x121308, 0x8703, 0xFFFF); //[15]:dma_en

    // Set SC2_FRM_W DMA
    RGN_UT_W2BYTE_MSK(0x121410, RGN_UT_LBYTE(u32MiuAddr2_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr0 of Y
    RGN_UT_W2BYTE_MSK(0x121412, RGN_UT_HBYTE(u32MiuAddr2_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr0 of Y
    RGN_UT_W2BYTE_MSK(0x121414, RGN_UT_LBYTE(u32MiuAddr2_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr1 of Y
    RGN_UT_W2BYTE_MSK(0x121416, RGN_UT_HBYTE(u32MiuAddr2_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr1 of Y
    RGN_UT_W2BYTE_MSK(0x121418, RGN_UT_LBYTE(u32MiuAddr2_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr2 of Y
    RGN_UT_W2BYTE_MSK(0x12141A, RGN_UT_HBYTE(u32MiuAddr2_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr2 of Y
    RGN_UT_W2BYTE_MSK(0x12141C, RGN_UT_LBYTE(u32MiuAddr2_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr3 of Y
    RGN_UT_W2BYTE_MSK(0x12141E, RGN_UT_HBYTE(u32MiuAddr2_Y,SCL_ADDR_DIV), 0xFFFF); // set baseaddr3 of Y
    RGN_UT_W2BYTE_MSK(0x121420, RGN_UT_LBYTE(u32MiuAddr2_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr0 of C
    RGN_UT_W2BYTE_MSK(0x121422, RGN_UT_HBYTE(u32MiuAddr2_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr0 of C
    RGN_UT_W2BYTE_MSK(0x121424, RGN_UT_LBYTE(u32MiuAddr2_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr1 of C
    RGN_UT_W2BYTE_MSK(0x121426, RGN_UT_HBYTE(u32MiuAddr2_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr1 of C
    RGN_UT_W2BYTE_MSK(0x121428, RGN_UT_LBYTE(u32MiuAddr2_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr2 of C
    RGN_UT_W2BYTE_MSK(0x12142A, RGN_UT_HBYTE(u32MiuAddr2_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr2 of C
    RGN_UT_W2BYTE_MSK(0x12142C, RGN_UT_LBYTE(u32MiuAddr2_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr3 of C
    RGN_UT_W2BYTE_MSK(0x12142E, RGN_UT_HBYTE(u32MiuAddr2_C,SCL_ADDR_DIV), 0xFFFF); // set baseaddr3 of C
    RGN_UT_W2BYTE_MSK(0x121430, 0x0818, 0xFFFF); // [4:3] max base idx num = 3
    RGN_UT_W2BYTE_MSK(0x121434, 0x0320, 0xFFFF); // set H size
    RGN_UT_W2BYTE_MSK(0x121436, 0x01e0, 0xFFFF); // set V size
    RGN_UT_W2BYTE_MSK(0x12143C, 0x000c, 0xFFFF); // set 444 to 420 mode
    RGN_UT_W2BYTE_MSK(0x121402, 0x8703, 0xFFFF); //[15]:dma_en

    //MIU setting
    RGN_UT_W2BYTE_MSK(0x121362, 0x0810, 0xFFFF); // [15:8]: reg_sc0_dbg_r_req_len, for Y channel (default 8)
                                                 // [7:0]: reg_sc1_dbg_r_req_th, for Y channel (default 8)
    
    RGN_UT_W2BYTE_MSK(0x12253e, 0x0404, 0xFFFF); // [15:8]: reg_sc1_dbg_r_req_len_c, for C channel (default 4)
                                                 // [7:0]: reg_sc1_dbg_r_req_th_c, for C channel (default 4)
    
    //---------- de-assert reset --------------
    RGN_UT_W2BYTE_MSK(0x121802, 0x0000, 0xFFFF); // 0[0] , sw_reset_en

}
void RgnUtDipScript(MS_U32 u32InY, MS_U32 u32InC,
                    MS_U32 u32OutY, MS_U32 u32OutC, MS_BOOL bOnlyTrig)
{
    int count = 0;
    
if(!bOnlyTrig) {
    RGN_UT_W2BYTE_MSK(0x122AFE, 0x0001, 0xFFFF); // h7F [0]reg_sw_rst
    
    // DIP TOP
    RGN_UT_W2BYTE_MSK(0x123A16, 0x1000, 0xFFFF); // h0B [12]f422en_osd    
    RGN_UT_W2BYTE_MSK(0x123A60, 0x0020, 0xFFFF); // h30 [5]reg_dip_pdw_src_422(0:RGB 1:YUV) [0]reg_dip_f422en
    RGN_UT_W2BYTE_MSK(0x123A62, 0x0000, 0xFFFF); // h31 [0]reg_dip_r2y_en
    RGN_UT_W2BYTE_MSK(0x123AD0, 0x0000, 0xFFFF); // h68 [1:0]reg_422to444_md
    RGN_UT_W2BYTE_MSK(0x123AD8, 0x1200, 0xFFFF); // h6C [12]reg_4k_left_only=1 [10:9]reg_4k_422to444_md
    RGN_UT_W2BYTE_MSK(0x123ADA, 0x8168, 0xFFFF); // h6D [15]4k_path_on=1 [11:0]4k_h_size=width/2
    RGN_UT_W2BYTE_MSK(0x123ADE, 0x0002, 0xFFFF); // h6F [1:0]reg_dipr_src_sel
    
    // DIPW
    RGN_UT_W2BYTE_MSK(0x123B02, 0x1000, 0xFFFF); // h01 [12:8]reg_dip_422to420_ratio=0x10 [5:4]reg_dip_format(0:yc422 1:rgb565 2:argb8888 3:yc420)
    RGN_UT_W2BYTE_MSK(0x123B04, 0x04FF, 0xFFFF); // h02 [15]reg_dip_yc_swap [14]reg_dipw_cbcr_swap [10]reg_dip_pdw_off=1 (0:enable 1:disable) [7:0]reg_dip_argb_alpha=0xFF
    RGN_UT_W2BYTE_MSK(0x123B06, 0x0000, 0xFFFF); // h03 [14]reg_dip_420_linear   [11]MIU mode=0(0:128 1:256)

    RGN_UT_W2BYTE_MSK(0x123B20, RGN_UT_LBYTE(u32OutY,DIP_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x123B22, RGN_UT_HBYTE(u32OutY,DIP_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x123B40, RGN_UT_LBYTE(u32OutC,DIP_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x123B42, RGN_UT_HBYTE(u32OutC,DIP_ADDR_DIV), 0xFFFF);

    RGN_UT_W2BYTE_MSK(0x123B3E, 0x02D0, 0xFFFF); // h1F [11:0]reg_dip_fetch_num=width
    RGN_UT_W2BYTE_MSK(0x123B5E, 0x0240, 0xFFFF); // h2F [11:0]reg_dip_vcnt_num=height
    RGN_UT_W2BYTE_MSK(0x123B7E, 0x02D0, 0xFFFF); // h3F [11:0]reg_dip_ln_offset=width
    RGN_UT_W2BYTE_MSK(0x123BDE, 0x400A, 0xFFFF); // h6F [14:8]reg_dip_wreq_max=0x40 [5:0]reg_dip_wreq_thrd=0xA(Less than (DIPW Hsize/8)-1)
    RGN_UT_W2BYTE_MSK(0x123BFC, 0x0010, 0xFFFF); // h7E [11] reg_dip_y2r_en [6:0]reg_dip_tile_req_num=0x10(even and smaller than (DIPW Hsize/16))
    
    // DIPR
    RGN_UT_W2BYTE_MSK(0x123B0E, 0x0000, 0xFFFF); // h07 [14]reg_dipr_420_linear
    RGN_UT_W2BYTE_MSK(0x123B4E, 0x02D0, 0xFFFF); // h27 [11:0]reg_dipr_fetch_num=width
    RGN_UT_W2BYTE_MSK(0x123B6E, 0x0240, 0xFFFF); // h37 [11:0]reg_dipr_vcnt_num=height
    RGN_UT_W2BYTE_MSK(0x123BE8, 0x02D0, 0xFFFF); // h74 [12:0]reg_dipr_sw_ln_offset

    RGN_UT_W2BYTE_MSK(0x123BF0, RGN_UT_LBYTE(u32InY,DIP_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x123BF2, RGN_UT_HBYTE(u32InY,DIP_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x123BF4, RGN_UT_LBYTE(u32InC,DIP_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x123BF6, RGN_UT_HBYTE(u32InC,DIP_ADDR_DIV), 0xFFFF);

    RGN_UT_W2BYTE_MSK(0x123BF8, 0x0000, 0xFFFF); // h7C [3]reg_dipr_420to422 [2]reg_dipr_cbcr_swap [0]reg_dipr_yc_swap
    RGN_UT_W2BYTE_MSK(0x123BFA, 0x0F18, 0xFFFF); // h7D [12:8]reg_dipr_hi_pri_num=0xF [7:6]reg_dipr_format [5:0]reg_dipr_req_thrd=0x18
    RGN_UT_W2BYTE_MSK(0x123BFE, 0x402C, 0xFFFF); // h7F [14:8]reg_dipr_req_max=0x40  [6:0]reg_dipr_tile_req_num=factor(DIPR hsize/16)-1(only YUV420 needs to set)
    
    // Mirror Flip
    RGN_UT_W2BYTE_MSK(0x123B16, 0x0080, 0xFFFF); // h0B [10]reg_v_flip [9]reg_h_mirror [7:0]default value=0x80
    
    // HVSP
    RGN_UT_W2BYTE_MSK(0x123C0E, 0x0000, 0xFFFF); // h07 [15:0]reg_scale_factor_ho
    RGN_UT_W2BYTE_MSK(0x123C10, 0x0000, 0xFFFF); // h08 [8]reg_scale_ho_en [23:16]reg_scale_factor_ho
    RGN_UT_W2BYTE_MSK(0x123C12, 0x0000, 0xFFFF); // h09 [15:0]reg_scale_factor_ve
    RGN_UT_W2BYTE_MSK(0x123C14, 0x0000, 0xFFFF); // h0A [8]reg_scale_ve_en [23:16]reg_scale_factor_ve
    RGN_UT_W2BYTE_MSK(0x123C16, 0x0000, 0xFFFF); // h0B [11:9]reg_mode_c_ve [8]reg_mode_y_ve [3:1]reg_mode_c_ho [0]reg_mode_y_ho
    RGN_UT_W2BYTE_MSK(0x123C44, 0x0000, 0xFFFF); // h22 [12:0]reg_h_size
    RGN_UT_W2BYTE_MSK(0x123C46, 0x0000, 0xFFFF); // h23 [11:0]reg_v_size
    RGN_UT_W2BYTE_MSK(0x123C48, 0x0000, 0xFFFF); // h24 [12:0]reg_scl_h_size
    RGN_UT_W2BYTE_MSK(0x123C4A, 0x0000, 0xFFFF); // h25 [11:0]reg_scl_v_size
    RGN_UT_W2BYTE_MSK(0x123CE0, 0x0006, 0xFFFF); // h70 [2:1]reg_dip_hvsp_sel(0:dwin0 1:dwin1 2:dip 3:disable)
    
    // Enable DIPW/DIPR
    RGN_UT_W2BYTE_MSK(0x123B04, 0x0000, 0x0400); // h02 [10]reg_dip_pdw_off=0(0:enable 1:disable)
    RGN_UT_W2BYTE_MSK(0x123BFA, 0x4000, 0x4000); // h7D [14]reg_dipr_en=1
}

    // Clear interrupts
    RGN_UT_W2BYTE_MSK(0x123B12, 0x00FF, 0x00FF); // h09 [7:0]reg_dipw_irq_clr
    RGN_UT_W2BYTE_MSK(0x123B52, 0x00FF, 0x00FF); // h29 [7:0]reg_dipw_cmdq_irq_clr
    RGN_UT_W2BYTE_MSK(0x123B72, 0x00FF, 0x00FF); // h39 [7:0]reg_dipr_irq_clr
    RGN_UT_W2BYTE_MSK(0x123B92, 0x00FF, 0x00FF); // h49 [7:0]reg_dipr_cmdq_irq_clr
    RGN_UT_W2BYTE_MSK(0x123BB2, 0x00FF, 0x00FF); // h59 [7:0]reg_dipw_ext_irq_clr
    RGN_UT_W2BYTE_MSK(0x123BD2, 0x00FF, 0x00FF); // h69 [7:0]reg_dipw_ext_cmdq_irq_clr
    // Trigger
    RGN_UT_W2BYTE_MSK(0x123BF8, 0x8000, 0x8000); // h7C [15]reg_dipr_sw_trig

    count = 0;
    while(1)
    {
        if(RGN_UT_R2BYTE(0x123BB4)) { // h5A [7:0]reg_dipw_ext_irq_final_status
            printk("DIP frame done!\n");
            break;
        }
        count++;
        if(count>100) {
            printk("Error!DIP time out!\n");
            break;
        }
        msleep_interruptible(10);  // sleep 10 ms
    }
}
void RgnUtLdcScript(MS_U32 u32DistMap, MS_U32 u32InY, MS_U32 u32InC,
                    MS_U32 u32PosTbl, MS_U32 u32MaskTbl,
                    MS_U32 u32OutY, MS_U32 u32OutC, MS_BOOL bOnlyTrig)
{
    int count = 0;

    RGN_UT_W2BYTE_MSK(0x143d00, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d00, 0x0001, 0xFFFF);
if(!bOnlyTrig) {
    RGN_UT_W2BYTE_MSK(0x143c00, 0x01f5, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c02, 0x0020, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c04, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c06, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c08, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c0a, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c0c, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c0e, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c10, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c12, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c14, 0x0003, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c16, 0x0200, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c18, 0x03c0, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c1a, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c1c, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c1e, 0x01e0, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c20, 0x03c0, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c22, 0x0004, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c24, 0x0040, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c26, 0x9001, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c28, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c2a, 0x00ff, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c2c, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c2e, 0x00ff, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c30, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c32, 0x00ff, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c34, 0x8040, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c36, 0x000B, 0xFFFF); // h1B [3:0] reg_vsync_width
    RGN_UT_W2BYTE_MSK(0x143d04, 0x2402, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d06, RGN_UT_LBYTE(u32DistMap,LDC_R_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d08, RGN_UT_HBYTE(u32DistMap,LDC_R_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d0a, RGN_UT_LBYTE(u32InY,LDC_R_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d0c, RGN_UT_HBYTE(u32InY,LDC_R_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d0e, RGN_UT_LBYTE(u32InC,LDC_R_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d10, RGN_UT_HBYTE(u32InC,LDC_R_ADDR_DIV), 0xFFFF);
    
    RGN_UT_W2BYTE_MSK(0x143d16, 0x0200, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d18, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d1a, 0x0000, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d1c, 0x0200, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d1e, 0x03c0, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d20, RGN_UT_LBYTE(u32PosTbl,LDC_R_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d22, RGN_UT_HBYTE(u32PosTbl,LDC_R_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d24, RGN_UT_LBYTE(u32MaskTbl,LDC_R_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d26, RGN_UT_HBYTE(u32MaskTbl,LDC_R_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d60, 0xfffe, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d68, 0x0001, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d6c, 0xfffe, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143d74, 0x0001, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c44, 0xfc00, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c48, RGN_UT_LBYTE(u32OutY,LDC_W_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c4a, RGN_UT_HBYTE(u32OutY,LDC_W_ADDR_DIV), 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c4c, RGN_UT_LBYTE(u32OutC,LDC_W_ADDR_DIV), 0xFFFF);    
    RGN_UT_W2BYTE_MSK(0x143c4e, RGN_UT_HBYTE(u32OutC,LDC_W_ADDR_DIV), 0xFFFF);    
    RGN_UT_W2BYTE_MSK(0x143c7c, 0x01e0, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c7e, 0x03c0, 0xFFFF);
    RGN_UT_W2BYTE_MSK(0x143c40, 0x8703, 0xFFFF);
}
    RGN_UT_W2BYTE_MSK(0x143d6A, 0x0001, 0xFFFF); // h35 [15:8]write 1 to clear interrupt(clk_miu domain)
                                                 //     [7:0]write 1 to clear interrupt(clk_ldc domain)
    RGN_UT_W2BYTE_MSK(0x143d76, 0x0001, 0xFFFF); // h3B [15:8]write 1 to clear interrupt for command queue(clk_miu domain)
                                                 //     [7:0]write 1 to clear interrupt for command queue(clk_ldc domain)
    // Trigger
    RGN_UT_W2BYTE_MSK(0x143d02, 0x0001, 0xFFFF);
    count = 0;
    while(1)
    {
        if(RGN_UT_R2BYTE(0x143d66)) { // h33 [15:0]reg_ldc_irq_final_status
            printk("LDC frame done!\n");
            break;
        }
        count++;
        if(count>100) {
            printk("Error!LDC time out!\n");
            break;
        }
        msleep_interruptible(10);  // sleep 10 ms
    }
}
void RgnUtReadReg(MS_U32 u32Bank)
{
    int i;
    for(i=0;i<0x80;i++) {
        if(i%8==0) {printk("\n%02x",i);}
        printk(" %04x",RGN_UT_R2BYTE((u32Bank<<8)|(i<<1)));
    }
    printk("\n");
}
void RgnUtWriteReg(MS_U32 u32Bank, MS_U32 u32Offset, MS_U32 u32Value)
{
    RGN_UT_W2BYTE_MSK((u32Bank<<8)|(u32Offset<<1), u32Value, 0xFFFF);
}
