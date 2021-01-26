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

#include "hal_rgn_common.h"

#define RGN_UT_LBYTE(addr)  ((addr/8)&0xFFFF)
#define RGN_UT_HBYTE(addr)  (((addr/8)>>16)&0xFFFF)
#define RGN_UT_Y2C(addr)    (addr+800*480)

void RgnUtSclScript(MS_U32 u32MiuAddr0, MS_U32 u32MiuAddr1, MS_U32 u32MiuAddr2)
{
    //CamOsPrintf("0:0x%x, L:0x%x, H:0x%x\n",u32MiuAddr0,RGN_UT_LBYTE(u32MiuAddr0),RGN_UT_HBYTE(u32MiuAddr0));

    //CLK
    HalRgnWrite2Byte(0x1038C2, 0x0000, E_HAL_RGN_CMDQ_ID_NUM); //clk idclk
    HalRgnWrite2Byte(0x1038C6, 0x0000, E_HAL_RGN_CMDQ_ID_NUM); //clk idclk
    HalRgnWrite2Byte(0x1038C8, 0x0000, E_HAL_RGN_CMDQ_ID_NUM); //clk fclk
    HalRgnWrite2Byte(0x1038CA, 0x0000, E_HAL_RGN_CMDQ_ID_NUM); //clk fclk2
    HalRgnWrite2Byte(0x1038CC, 0x0000, E_HAL_RGN_CMDQ_ID_NUM); //clk_odclk
    HalRgnWrite2Byte(0x113348, 0x0000, E_HAL_RGN_CMDQ_ID_NUM); //clk_idclk
    
    // Set Output PATGEN
    HalRgnWrite2Byte(0x121a20, 0x0000, E_HAL_RGN_CMDQ_ID_NUM); //10[8] , frame color

    HalRgnWrite2Byte(0x121802, 0x0001, E_HAL_RGN_CMDQ_ID_NUM); //0[0] , sw_reset_en

    // output Y2R
    HalRgnWrite2Byte(0x121a32, 0x1001, E_HAL_RGN_CMDQ_ID_NUM); // cm_en
    HalRgnWrite2Byte(0x121a34, 0x8080, E_HAL_RGN_CMDQ_ID_NUM); // cm_en
    HalRgnWrite2Byte(0x121a36, 0x0cc4, E_HAL_RGN_CMDQ_ID_NUM); // cm_11
    HalRgnWrite2Byte(0x121a38, 0x0950, E_HAL_RGN_CMDQ_ID_NUM); // cm_12
    HalRgnWrite2Byte(0x121a3A, 0x3ffc, E_HAL_RGN_CMDQ_ID_NUM); // cm_13
    HalRgnWrite2Byte(0x121a3C, 0x397e, E_HAL_RGN_CMDQ_ID_NUM); // cm_21
    HalRgnWrite2Byte(0x121a3E, 0x0950, E_HAL_RGN_CMDQ_ID_NUM); // cm_22
    HalRgnWrite2Byte(0x121a40, 0x3cde, E_HAL_RGN_CMDQ_ID_NUM); // cm_23
    HalRgnWrite2Byte(0x121a42, 0x3ffe, E_HAL_RGN_CMDQ_ID_NUM); // cm_31
    HalRgnWrite2Byte(0x121a44, 0x0950, E_HAL_RGN_CMDQ_ID_NUM); // cm_32
    HalRgnWrite2Byte(0x121a46, 0x1024, E_HAL_RGN_CMDQ_ID_NUM); // cm_33

    // Set INPUT PATGEN
    HalRgnWrite2Byte(0x121882, 0x0030, E_HAL_RGN_CMDQ_ID_NUM);

    HalRgnWrite2Byte(0x1218E0, 0x0003, E_HAL_RGN_CMDQ_ID_NUM); // input tgen mode
    HalRgnWrite2Byte(0x1218E2, 0x0001, E_HAL_RGN_CMDQ_ID_NUM); // vs st
    HalRgnWrite2Byte(0x1218E4, 0x0003, E_HAL_RGN_CMDQ_ID_NUM); // vs end
    HalRgnWrite2Byte(0x1218E6, 0x0021, E_HAL_RGN_CMDQ_ID_NUM); // v st -f
    HalRgnWrite2Byte(0x1218E8, 0x0200, E_HAL_RGN_CMDQ_ID_NUM); // v end -f
    HalRgnWrite2Byte(0x1218EA, 0x0021, E_HAL_RGN_CMDQ_ID_NUM); // v st -m
    HalRgnWrite2Byte(0x1218EC, 0x0200, E_HAL_RGN_CMDQ_ID_NUM); // v end -m
    HalRgnWrite2Byte(0x1218EE, 0x0300, E_HAL_RGN_CMDQ_ID_NUM); // v total
    HalRgnWrite2Byte(0x1218F2, 0x0004, E_HAL_RGN_CMDQ_ID_NUM); // hs start
    HalRgnWrite2Byte(0x1218F4, 0x0033, E_HAL_RGN_CMDQ_ID_NUM); // hs end
    HalRgnWrite2Byte(0x1218F6, 0x005C, E_HAL_RGN_CMDQ_ID_NUM); // h st -f
    HalRgnWrite2Byte(0x1218F8, 0x037B, E_HAL_RGN_CMDQ_ID_NUM); // h end -f
    HalRgnWrite2Byte(0x1218FA, 0x005C, E_HAL_RGN_CMDQ_ID_NUM); // h st -m
    HalRgnWrite2Byte(0x1218FC, 0x037B, E_HAL_RGN_CMDQ_ID_NUM); // h end -m
    HalRgnWrite2Byte(0x1218FE, 0x039F, E_HAL_RGN_CMDQ_ID_NUM); // h total
    HalRgnWrite2Byte(0x121880, 0x8003, E_HAL_RGN_CMDQ_ID_NUM); // pat en
    HalRgnWrite2Byte(0x121884, 0x3C64, E_HAL_RGN_CMDQ_ID_NUM); // Hsize Vsize
    HalRgnWrite2Byte(0x121888, 0x0C01, E_HAL_RGN_CMDQ_ID_NUM); // R2Y

    //SET OUTPUT TGEN
    HalRgnWrite2Byte(0x121A02, 0x0001, E_HAL_RGN_CMDQ_ID_NUM); // vs   start   // 1 ~ 3 (3 th VSYNC)
    HalRgnWrite2Byte(0x121A04, 0x0003, E_HAL_RGN_CMDQ_ID_NUM); // vs   end
    HalRgnWrite2Byte(0x121A06, 0x0021, E_HAL_RGN_CMDQ_ID_NUM); // v st -f  // 4 ~ 32 (29 th back porch), 33 ~ 512 (480 th display area), 513 ~ 524, 0 (13 th front porch)
    HalRgnWrite2Byte(0x121A08, 0x0200, E_HAL_RGN_CMDQ_ID_NUM); // v end -f
    HalRgnWrite2Byte(0x121A0A, 0x0021, E_HAL_RGN_CMDQ_ID_NUM); // v st -m  // No used.
    HalRgnWrite2Byte(0x121A0C, 0x0200, E_HAL_RGN_CMDQ_ID_NUM); // v end -m
    HalRgnWrite2Byte(0x121A0E, 0x0300, E_HAL_RGN_CMDQ_ID_NUM); // v total  // 524 (525 th)
    HalRgnWrite2Byte(0x121A12, 0x0004, E_HAL_RGN_CMDQ_ID_NUM); // hs start // 4 ~ 51 ( 48 Tcph HSYNC)
    HalRgnWrite2Byte(0x121A14, 0x0033, E_HAL_RGN_CMDQ_ID_NUM); // hs end
    HalRgnWrite2Byte(0x121A16, 0x005C, E_HAL_RGN_CMDQ_ID_NUM); // h st -f  // 52 ~ 91 ( 40 Tcph back porch), 92 ~ 891 (800 Tcph display area), 892 ~ 927, 0 ~ 3 (40 Tcph front porch)
    HalRgnWrite2Byte(0x121A18, 0x037B, E_HAL_RGN_CMDQ_ID_NUM); // h end -f
    HalRgnWrite2Byte(0x121A1A, 0x005C, E_HAL_RGN_CMDQ_ID_NUM); // h st -m  // No used.
    HalRgnWrite2Byte(0x121A1C, 0x037B, E_HAL_RGN_CMDQ_ID_NUM); // h end -m
    HalRgnWrite2Byte(0x121A1E, 0x039F, E_HAL_RGN_CMDQ_ID_NUM); // h total  // 927 (928 Tcph)

    // Crop0
    HalRgnWrite2Byte(0x121900, 0x0002, E_HAL_RGN_CMDQ_ID_NUM); //Crop0 type
    HalRgnWrite2Byte(0x121902, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); //Crop0 in x size
    HalRgnWrite2Byte(0x121904, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); //Crop0 in y size
    HalRgnWrite2Byte(0x121908, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); //Crop0 out x size
    HalRgnWrite2Byte(0x12190C, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); //Crop0 out y size

    // Crop10
    HalRgnWrite2Byte(0x121910, 0x0002, E_HAL_RGN_CMDQ_ID_NUM); //Crop10 type
    HalRgnWrite2Byte(0x121912, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); //Crop10 in x size
    HalRgnWrite2Byte(0x121914, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); //Crop10 in y size
    HalRgnWrite2Byte(0x121918, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); //Crop10 out x size
    HalRgnWrite2Byte(0x12191C, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); //Crop10 out y size
    
    // Crop11
    HalRgnWrite2Byte(0x121920, 0x0002, E_HAL_RGN_CMDQ_ID_NUM); //Crop11 type
    HalRgnWrite2Byte(0x121922, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); //Crop11 in x size
    HalRgnWrite2Byte(0x121924, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); //Crop11 in y size
    HalRgnWrite2Byte(0x121928, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); //Crop11 out x size
    HalRgnWrite2Byte(0x12192C, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); //Crop11 out y size

    // Crop2
    HalRgnWrite2Byte(0x121930, 0x0002, E_HAL_RGN_CMDQ_ID_NUM); //Crop2 type
    HalRgnWrite2Byte(0x121932, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); //Crop2 in x size
    HalRgnWrite2Byte(0x121934, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); //Crop2 in y size
    HalRgnWrite2Byte(0x121938, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); //Crop2 out x size
    HalRgnWrite2Byte(0x12193C, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); //Crop2 out y size

    // Set HVSP1 SIZE
    HalRgnWrite2Byte(0x121040, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); // H size (before scaling)
    HalRgnWrite2Byte(0x121042, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); // V size (before scaling)
    HalRgnWrite2Byte(0x121044, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); // H size (after scaling)
    HalRgnWrite2Byte(0x121046, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); // V size (after scaling)

    // Set HVSP2 SIZE
    HalRgnWrite2Byte(0x121140, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); // H size (before scaling)
    HalRgnWrite2Byte(0x121142, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); // V size (before scaling)
    HalRgnWrite2Byte(0x121144, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); // H size (after scaling)
    HalRgnWrite2Byte(0x121146, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); // V size (after scaling)

    // Set SC0_FRM_W DMA
    HalRgnWrite2Byte(0x121302, 0x8703, E_HAL_RGN_CMDQ_ID_NUM); //[15]:dma_en
    HalRgnWrite2Byte(0x121310, RGN_UT_LBYTE(u32MiuAddr0), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr0 of Y
    HalRgnWrite2Byte(0x121312, RGN_UT_HBYTE(u32MiuAddr0), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr0 of Y
    HalRgnWrite2Byte(0x121314, RGN_UT_LBYTE(u32MiuAddr0), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr1 of Y
    HalRgnWrite2Byte(0x121316, RGN_UT_HBYTE(u32MiuAddr0), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr1 of Y
    HalRgnWrite2Byte(0x121318, RGN_UT_LBYTE(u32MiuAddr0), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr2 of Y
    HalRgnWrite2Byte(0x12131A, RGN_UT_HBYTE(u32MiuAddr0), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr2 of Y
    HalRgnWrite2Byte(0x12131C, RGN_UT_LBYTE(u32MiuAddr0), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr3 of Y
    HalRgnWrite2Byte(0x12131E, RGN_UT_HBYTE(u32MiuAddr0), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr3 of Y
    HalRgnWrite2Byte(0x121320, RGN_UT_LBYTE(RGN_UT_Y2C(u32MiuAddr0)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr0 of C
    HalRgnWrite2Byte(0x121322, RGN_UT_HBYTE(RGN_UT_Y2C(u32MiuAddr0)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr0 of C
    HalRgnWrite2Byte(0x121324, RGN_UT_LBYTE(RGN_UT_Y2C(u32MiuAddr0)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr1 of C
    HalRgnWrite2Byte(0x121326, RGN_UT_HBYTE(RGN_UT_Y2C(u32MiuAddr0)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr1 of C
    HalRgnWrite2Byte(0x121328, RGN_UT_LBYTE(RGN_UT_Y2C(u32MiuAddr0)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr2 of C
    HalRgnWrite2Byte(0x12132A, RGN_UT_HBYTE(RGN_UT_Y2C(u32MiuAddr0)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr2 of C
    HalRgnWrite2Byte(0x12132C, RGN_UT_LBYTE(RGN_UT_Y2C(u32MiuAddr0)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr3 of C
    HalRgnWrite2Byte(0x12132E, RGN_UT_HBYTE(RGN_UT_Y2C(u32MiuAddr0)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr3 of C
    HalRgnWrite2Byte(0x121330, 0x0818, E_HAL_RGN_CMDQ_ID_NUM); // [4:3] max base idx num = 3
    HalRgnWrite2Byte(0x121334, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); // set H size
    HalRgnWrite2Byte(0x121336, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); // set V size
    HalRgnWrite2Byte(0x12133C, 0x000c, E_HAL_RGN_CMDQ_ID_NUM); // set 444 to 420 mode

    // Set SC1_FRM_W DMA
    HalRgnWrite2Byte(0x121308, 0x8703, E_HAL_RGN_CMDQ_ID_NUM); //[15]:dma_en
    HalRgnWrite2Byte(0x1213A0, RGN_UT_LBYTE(u32MiuAddr1), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr0 of Y
    HalRgnWrite2Byte(0x1213A2, RGN_UT_HBYTE(u32MiuAddr1), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr0 of Y
    HalRgnWrite2Byte(0x1213A4, RGN_UT_LBYTE(u32MiuAddr1), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr1 of Y
    HalRgnWrite2Byte(0x1213A6, RGN_UT_HBYTE(u32MiuAddr1), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr1 of Y
    HalRgnWrite2Byte(0x1213A8, RGN_UT_LBYTE(u32MiuAddr1), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr2 of Y
    HalRgnWrite2Byte(0x1213AA, RGN_UT_HBYTE(u32MiuAddr1), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr2 of Y
    HalRgnWrite2Byte(0x1213AC, RGN_UT_LBYTE(u32MiuAddr1), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr3 of Y
    HalRgnWrite2Byte(0x1213AE, RGN_UT_HBYTE(u32MiuAddr1), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr3 of Y
    HalRgnWrite2Byte(0x1213B0, RGN_UT_LBYTE(RGN_UT_Y2C(u32MiuAddr1)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr0 of C
    HalRgnWrite2Byte(0x1213B2, RGN_UT_HBYTE(RGN_UT_Y2C(u32MiuAddr1)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr0 of C
    HalRgnWrite2Byte(0x1213B4, RGN_UT_LBYTE(RGN_UT_Y2C(u32MiuAddr1)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr1 of C
    HalRgnWrite2Byte(0x1213B6, RGN_UT_HBYTE(RGN_UT_Y2C(u32MiuAddr1)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr1 of C
    HalRgnWrite2Byte(0x1213B8, RGN_UT_LBYTE(RGN_UT_Y2C(u32MiuAddr1)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr2 of C
    HalRgnWrite2Byte(0x1213BA, RGN_UT_HBYTE(RGN_UT_Y2C(u32MiuAddr1)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr2 of C
    HalRgnWrite2Byte(0x1213BC, RGN_UT_LBYTE(RGN_UT_Y2C(u32MiuAddr1)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr3 of C
    HalRgnWrite2Byte(0x1213BE, RGN_UT_HBYTE(RGN_UT_Y2C(u32MiuAddr1)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr3 of C
    HalRgnWrite2Byte(0x1213C0, 0x0818, E_HAL_RGN_CMDQ_ID_NUM); // [4:3] max base idx num = 3
    HalRgnWrite2Byte(0x1213C4, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); // set H size
    HalRgnWrite2Byte(0x1213C6, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); // set V size
    HalRgnWrite2Byte(0x1213CC, 0x000c, E_HAL_RGN_CMDQ_ID_NUM); // set 444 to 420 mode

    // Set SC2_FRM_W DMA
    HalRgnWrite2Byte(0x121402, 0x8703, E_HAL_RGN_CMDQ_ID_NUM); //[15]:dma_en
    HalRgnWrite2Byte(0x121410, RGN_UT_LBYTE(u32MiuAddr2), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr0 of Y
    HalRgnWrite2Byte(0x121412, RGN_UT_HBYTE(u32MiuAddr2), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr0 of Y
    HalRgnWrite2Byte(0x121414, RGN_UT_LBYTE(u32MiuAddr2), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr1 of Y
    HalRgnWrite2Byte(0x121416, RGN_UT_HBYTE(u32MiuAddr2), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr1 of Y
    HalRgnWrite2Byte(0x121418, RGN_UT_LBYTE(u32MiuAddr2), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr2 of Y
    HalRgnWrite2Byte(0x12141A, RGN_UT_HBYTE(u32MiuAddr2), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr2 of Y
    HalRgnWrite2Byte(0x12141C, RGN_UT_LBYTE(u32MiuAddr2), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr3 of Y
    HalRgnWrite2Byte(0x12141E, RGN_UT_HBYTE(u32MiuAddr2), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr3 of Y
    HalRgnWrite2Byte(0x121420, RGN_UT_LBYTE(RGN_UT_Y2C(u32MiuAddr2)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr0 of C
    HalRgnWrite2Byte(0x121422, RGN_UT_HBYTE(RGN_UT_Y2C(u32MiuAddr2)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr0 of C
    HalRgnWrite2Byte(0x121424, RGN_UT_LBYTE(RGN_UT_Y2C(u32MiuAddr2)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr1 of C
    HalRgnWrite2Byte(0x121426, RGN_UT_HBYTE(RGN_UT_Y2C(u32MiuAddr2)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr1 of C
    HalRgnWrite2Byte(0x121428, RGN_UT_LBYTE(RGN_UT_Y2C(u32MiuAddr2)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr2 of C
    HalRgnWrite2Byte(0x12142A, RGN_UT_HBYTE(RGN_UT_Y2C(u32MiuAddr2)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr2 of C
    HalRgnWrite2Byte(0x12142C, RGN_UT_LBYTE(RGN_UT_Y2C(u32MiuAddr2)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr3 of C
    HalRgnWrite2Byte(0x12142E, RGN_UT_HBYTE(RGN_UT_Y2C(u32MiuAddr2)), E_HAL_RGN_CMDQ_ID_NUM); // set baseaddr3 of C
    HalRgnWrite2Byte(0x121430, 0x0818, E_HAL_RGN_CMDQ_ID_NUM); // [4:3] max base idx num = 3
    HalRgnWrite2Byte(0x121434, 0x0320, E_HAL_RGN_CMDQ_ID_NUM); // set H size
    HalRgnWrite2Byte(0x121436, 0x01e0, E_HAL_RGN_CMDQ_ID_NUM); // set V size
    HalRgnWrite2Byte(0x12143C, 0x000c, E_HAL_RGN_CMDQ_ID_NUM); // set 444 to 420 mode

    //MIU setting
    HalRgnWrite2Byte(0x121362, 0x0810, E_HAL_RGN_CMDQ_ID_NUM); // [15:8]: reg_sc0_dbg_r_req_len, for Y channel (default 8)
                                                                 // [7:0]: reg_sc1_dbg_r_req_th, for Y channel (default 8)
    HalRgnWrite2Byte(0x12253e, 0x0404, E_HAL_RGN_CMDQ_ID_NUM); // [15:8]: reg_sc1_dbg_r_req_len_c, for C channel (default 4)
                                                                 // [7:0]: reg_sc1_dbg_r_req_th_c, for C channel (default 4)

    //---------- de-assert reset --------------
    HalRgnWrite2Byte(0x121802, 0x0000, E_HAL_RGN_CMDQ_ID_NUM); //0[0] , sw_reset_en
}
