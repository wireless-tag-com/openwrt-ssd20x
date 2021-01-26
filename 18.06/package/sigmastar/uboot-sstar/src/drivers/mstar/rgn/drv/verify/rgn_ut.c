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

//==============================================================================
//
//                              INCLUDE FILES
//
//==============================================================================

#include <linux/pfn.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>          /* seems do not need this */
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/time.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/string.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/time.h>

#include "cam_os_wrapper.h"
#include "mhal_rgn.h"
#include "rgn_file_access.h"

MODULE_AUTHOR("SIGMASTAR");
MODULE_DESCRIPTION("RGN ut");

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define IP_NAME                 "RGN"

#define UT_RGN_PIX2BYTE(x, fmt)     (fmt)==E_MHAL_RGN_PIXEL_FORMAT_ARGB8888 ? (x) * 4 : \
                                    (fmt)==E_MHAL_RGN_PIXEL_FORMAT_ARGB1555 || \
                                    (fmt)==E_MHAL_RGN_PIXEL_FORMAT_ARGB4444 || \
                                    (fmt)==E_MHAL_RGN_PIXEL_FORMAT_RGB565   || \
                                    (fmt)==E_MHAL_RGN_PIXEL_FORMAT_UV8Y8    ? (x) * 2 : \
                                    (fmt)==E_MHAL_RGN_PIXEL_FORMAT_I8       ? (x) * 1 : \
                                    (fmt)==E_MHAL_RGN_PIXEL_FORMAT_I4       ? (x) / 2 : \
                                    (fmt)==E_MHAL_RGN_PIXEL_FORMAT_I2       ? (x) / 4 : \
                                                                             (x)
/*
#define UT_RGN_NUM2PIX(fmt)     (fmt)==0 ? E_MHAL_RGN_PIXEL_FORMAT_ARGB1555 : \
                                (fmt)==1 ? E_MHAL_RGN_PIXEL_FORMAT_ARGB4444 : \
                                (fmt)==2 ? E_MHAL_RGN_PIXEL_FORMAT_I2 : \
                                (fmt)==3 ? E_MHAL_RGN_PIXEL_FORMAT_I4 : \
                                (fmt)==4 ? E_MHAL_RGN_PIXEL_FORMAT_I8 : \
                                (fmt)==5 ? E_MHAL_RGN_PIXEL_FORMAT_RGB565 : \
                                (fmt)==6 ? E_MHAL_RGN_PIXEL_FORMAT_ARGB8888 : \
                                (fmt)==7 ? E_MHAL_RGN_PIXEL_FORMAT_UV8Y8 : \
                                          E_MHAL_RGN_PIXEL_FORMAT_MAX
*/

#define MIU1_PHY_BASE   0xA0000000
#define MIU1_MIU_BASE   0x80000000


//==============================================================================
//
//                               STRUCTURE
//
//==============================================================================



//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================

//======================================
// Global
//======================================
static char FileName[128] = {0};
module_param_string(FileName, FileName, 128, 0);

// Buffer size
static unsigned int BufW = 0;	
module_param(BufW, int, S_IRUGO|S_IWUSR);
static unsigned int BufH = 0;	
module_param(BufH, int, S_IRUGO|S_IWUSR);
static unsigned int BufFmt = 0;	
module_param(BufFmt, int, S_IRUGO|S_IWUSR);

static unsigned int TestChipCapEn = 0;	
module_param(TestChipCapEn, int, S_IRUGO|S_IWUSR);

//======================================
// GOP
//======================================
static unsigned int GOPId = -1;	
module_param(GOPId, int, S_IRUGO|S_IWUSR);
static unsigned int MIU = 0;
module_param(MIU,  int, S_IRUGO|S_IWUSR);


// Stretch window
static unsigned int StWinW = 0;	
module_param(StWinW, int, S_IRUGO|S_IWUSR);
static unsigned int StWinH = 0;	
module_param(StWinH, int, S_IRUGO|S_IWUSR);

// Color key
static unsigned int CKeyEn = 0;	
module_param(CKeyEn, int, S_IRUGO|S_IWUSR);
static unsigned int CKeyRGB = 0;	
module_param(CKeyRGB, int, S_IRUGO|S_IWUSR);

// Alpha 0 opaque or not
static unsigned int AZero = 0;	
module_param(AZero, int, S_IRUGO|S_IWUSR);


//======================================
// GWIN
//======================================

// GWin0
static unsigned int Gwin0_En = 0;
module_param(Gwin0_En,    int, S_IRUGO|S_IWUSR);
static unsigned int Gwin0_X = 0;
module_param(Gwin0_X,     int, S_IRUGO|S_IWUSR);
static unsigned int Gwin0_Y = 0;
module_param(Gwin0_Y,     int, S_IRUGO|S_IWUSR);
static unsigned int Gwin0_W = 0;
module_param(Gwin0_W,     int, S_IRUGO|S_IWUSR);
static unsigned int Gwin0_H = 0;
module_param(Gwin0_H,     int, S_IRUGO|S_IWUSR);
static unsigned int Gwin0_Fmt = -1;
module_param(Gwin0_Fmt,   int, S_IRUGO|S_IWUSR);
static unsigned int Gwin0_Std = 0;
module_param(Gwin0_Std,   int, S_IRUGO|S_IWUSR);
static unsigned int Gwin0_AType = 0;
module_param(Gwin0_AType, int, S_IRUGO|S_IWUSR);
static unsigned int Gwin0_AVal = 0xFF;
module_param(Gwin0_AVal,  int, S_IRUGO|S_IWUSR);

// GWin1
static unsigned int Gwin1_En = 0;
module_param(Gwin1_En,    int, S_IRUGO|S_IWUSR);
static unsigned int Gwin1_X = 0;
module_param(Gwin1_X,     int, S_IRUGO|S_IWUSR);
static unsigned int Gwin1_Y = 0;
module_param(Gwin1_Y,     int, S_IRUGO|S_IWUSR);
static unsigned int Gwin1_W = 0;
module_param(Gwin1_W,     int, S_IRUGO|S_IWUSR);
static unsigned int Gwin1_H = 0;
module_param(Gwin1_H,     int, S_IRUGO|S_IWUSR);
static unsigned int Gwin1_Fmt = -1;
module_param(Gwin1_Fmt,   int, S_IRUGO|S_IWUSR);
static unsigned int Gwin1_Std = 0;
module_param(Gwin1_Std,   int, S_IRUGO|S_IWUSR);
static unsigned int Gwin1_AType = 0;
module_param(Gwin1_AType, int, S_IRUGO|S_IWUSR);
static unsigned int Gwin1_AVal = 0xFF;
module_param(Gwin1_AVal,  int, S_IRUGO|S_IWUSR);

//======================================
// Color Inverse
//======================================
static unsigned int CI_En = 0;
module_param(CI_En,  int, S_IRUGO|S_IWUSR);
static unsigned int CI_Id = 0;
module_param(CI_Id,  int, S_IRUGO|S_IWUSR);
static unsigned int CI_Th1 = 0x80;
module_param(CI_Th1,  int, S_IRUGO|S_IWUSR);
static unsigned int CI_Th2 = 0x80;
module_param(CI_Th2,  int, S_IRUGO|S_IWUSR);
static unsigned int CI_Th3 = 0;
module_param(CI_Th3,  int, S_IRUGO|S_IWUSR);
static unsigned int CI_W = 128;
module_param(CI_W,  int, S_IRUGO|S_IWUSR);
static unsigned int CI_H = 128;
module_param(CI_H,  int, S_IRUGO|S_IWUSR);
static unsigned int CI_X = 0;
module_param(CI_X,  int, S_IRUGO|S_IWUSR);
static unsigned int CI_Y = 0;
module_param(CI_Y,  int, S_IRUGO|S_IWUSR);
static unsigned int CI_BlkNumX = 16;
module_param(CI_BlkNumX,  int, S_IRUGO|S_IWUSR);
static unsigned int CI_BlkNumY = 16;
module_param(CI_BlkNumY,  int, S_IRUGO|S_IWUSR);

//======================================
// Cover
//======================================
// 0:SCL0 1:SCL1 2:SCL2 3:SCL3
static unsigned int CoverId = -1;
module_param(CoverId,  int, S_IRUGO|S_IWUSR);
// bit0:Bwin0 // bit1:Bwin1 // bit2:Bwin2 // bit3:Bwin3
static unsigned int Bwin_En = -1;
module_param(Bwin_En,  int, S_IRUGO|S_IWUSR);
// 4 BWins Postion 
// Value | Bwin0    | Bwin1    | Bwin2    | Bwin3
// 0     | LeftTop  | RightTop | RightBtn | LeftBtn
// 1     | RightTop | RightBtn | LeftBtn  | LeftTop
// 2     | RightBtn | LeftBtn  | LeftTop  | RightTop
// 3     | LeftBtn  | LeftTop  | RightTop | RightBtn


static unsigned int Bwin_Pos_Md = -1;
module_param(Bwin_Pos_Md,  int, S_IRUGO|S_IWUSR);
// 4 BWins VYU 
// Value | Bwin0    | Bwin1    | Bwin2    | Bwin3
// 0     | R        | G        | B        | W
// 1     | G        | B        | W        | R
// 2     | B        | W        | R        | G
// 3     | W        | R        | G        | B

static unsigned int Bwin_VYU_Md = -1;
module_param(Bwin_VYU_Md,  int, S_IRUGO|S_IWUSR);
// Bwin size
static unsigned int Bwin_W = 0;
module_param(Bwin_W,  int, S_IRUGO|S_IWUSR);
static unsigned int Bwin_H = 0;
module_param(Bwin_H,  int, S_IRUGO|S_IWUSR);
// Customized position (if don't use pos_md)
static unsigned int Bwin_X = 0;
module_param(Bwin_X,  int, S_IRUGO|S_IWUSR);
static unsigned int Bwin_Y = 0;
module_param(Bwin_Y,  int, S_IRUGO|S_IWUSR);
// Customized VYU (if don't use vyu_md)
static unsigned int Bwin_VYU = 0;
module_param(Bwin_VYU,  int, S_IRUGO|S_IWUSR);

// Palette[i][0]=G+B, Palette[i][1]=A+R 
static MS_U16 Palette_I2[4][2] =
{{0xFFFF,0x00FF}, {0x0000,0x00FF}, {0xFF00,0x0000}, {0x00FF,0x0000}};

static MS_U16 Palette_I4[16][2] =
{{0x0000,0x0000}, {0x0000,0x0080}, {0x8000,0x0000}, {0x0080,0x0000},
 {0x8080,0x0000}, {0x0080,0x0080}, {0x8000,0x0080}, {0x8080,0x0080},
 {0x00FF,0x0000}, {0xFF00,0x0000}, {0x0000,0x00FF}, {0xFFFF,0x0000},
 {0x00FF,0x00FF}, {0xFF00,0x00FF}, {0xC0C0,0x00C0}, {0xFFFF,0x00FF}};

static MS_U16 Palette_I8[256][2] =
{{0x0000,0x0000}, {0x0000,0x0080}, {0x8000,0x0000}, {0x8000,0x0080}, {0x0080,0x0000}, {0x0080,0x0080}, {0x8080,0x0000}, {0xc0c0,0x00c0},
 {0xdcc0,0x00c0}, {0xcaf0,0x00a6}, {0x2000,0x0040}, {0x2000,0x0060}, {0x2000,0x0080}, {0x2000,0x00a0}, {0x2000,0x00c0}, {0x2000,0x00e0},
 {0x4000,0x0000}, {0x4000,0x0020}, {0x4000,0x0040}, {0x4000,0x0060}, {0x4000,0x0080}, {0x4000,0x00a0}, {0x4000,0x00c0}, {0x4000,0x00e0},
 {0x6000,0x0000}, {0x6000,0x0020}, {0x6000,0x0040}, {0x6000,0x0060}, {0x6000,0x0080}, {0x6000,0x00a0}, {0x6000,0x00c0}, {0x6000,0x00e0},
 {0x8000,0x0000}, {0x8000,0x0020}, {0x8000,0x0040}, {0x8000,0x0060}, {0x8000,0x0080}, {0x8000,0x00a0}, {0x8000,0x00c0}, {0x8000,0x00e0},
 {0xa000,0x0000}, {0xa000,0x0020}, {0xa000,0x0040}, {0xa000,0x0060}, {0xa000,0x0080}, {0xa000,0x00a0}, {0xa000,0x00c0}, {0xa000,0x00e0},
 {0xc000,0x0000}, {0xc000,0x0020}, {0xc000,0x0040}, {0xc000,0x0060}, {0xc000,0x0080}, {0xc000,0x00a0}, {0xc000,0x00c0}, {0xc000,0x00e0},
 {0xe000,0x0000}, {0xe000,0x0020}, {0xe000,0x0040}, {0xe000,0x0060}, {0xe000,0x0080}, {0xe000,0x00a0}, {0xe000,0x00c0}, {0xe000,0x00e0}, // h3F
 {0x0040,0x0000}, {0x0040,0x0020}, {0x0040,0x0040}, {0x0040,0x0060}, {0x0040,0x0080}, {0x0040,0x00a0}, {0x0040,0x00c0}, {0x0040,0x00e0},
 {0x2040,0x0000}, {0x2040,0x0020}, {0x2040,0x0040}, {0x2040,0x0060}, {0x2040,0x0080}, {0x2040,0x00a0}, {0x2040,0x00c0}, {0x2040,0x00e0},
 {0x4040,0x0000}, {0x4040,0x0020}, {0x4040,0x0040}, {0x4040,0x0060}, {0x4040,0x0080}, {0x4040,0x00a0}, {0x4040,0x00c0}, {0x4040,0x00e0},
 {0x6040,0x0000}, {0x6040,0x0020}, {0x6040,0x0040}, {0x6040,0x0060}, {0x6040,0x0080}, {0x6040,0x00a0}, {0x6040,0x00c0}, {0x6040,0x00e0},
 {0x8040,0x0000}, {0x8040,0x0020}, {0x8040,0x0040}, {0x8040,0x0060}, {0x8040,0x0080}, {0x8040,0x00a0}, {0x8040,0x00c0}, {0x8040,0x00e0},
 {0xa040,0x0000}, {0xa040,0x0020}, {0xa040,0x0040}, {0xa040,0x0060}, {0xa040,0x0080}, {0xa040,0x00a0}, {0xa040,0x00c0}, {0xa040,0x00e0},
 {0xc040,0x0000}, {0xc040,0x0020}, {0xc040,0x0040}, {0xc040,0x0060}, {0xc040,0x0080}, {0xc040,0x00a0}, {0xc040,0x00c0}, {0xc040,0x00e0},
 {0xe040,0x0000}, {0xe040,0x0020}, {0xe040,0x0040}, {0xe040,0x0060}, {0xe040,0x0080}, {0xe040,0x00a0}, {0xe040,0x00c0}, {0xe040,0x00e0}, // h7F
 {0x0080,0x0000}, {0x0080,0x0020}, {0x0080,0x0040}, {0x0080,0x0060}, {0x0080,0x0080}, {0x0080,0x00a0}, {0x0080,0x00c0}, {0x0080,0x00e0},
 {0x2080,0x0000}, {0x2080,0x0020}, {0x2080,0x0040}, {0x2080,0x0060}, {0x2080,0x0080}, {0x2080,0x00a0}, {0x2080,0x00c0}, {0x2080,0x00e0},
 {0x4080,0x0000}, {0x4080,0x0020}, {0x4080,0x0040}, {0x4080,0x0060}, {0x4080,0x0080}, {0x4080,0x00a0}, {0x4080,0x00c0}, {0x4080,0x00e0},
 {0x6080,0x0000}, {0x6080,0x0020}, {0x6080,0x0040}, {0x6080,0x0060}, {0x6080,0x0080}, {0x6080,0x00a0}, {0x6080,0x00c0}, {0x6080,0x00e0},
 {0x8080,0x0000}, {0x8080,0x0020}, {0x8080,0x0040}, {0x8080,0x0060}, {0x8080,0x0080}, {0x8080,0x00a0}, {0x8080,0x00c0}, {0x8080,0x00e0},
 {0xa080,0x0000}, {0xa080,0x0020}, {0xa080,0x0040}, {0xa080,0x0060}, {0xa080,0x0080}, {0xa080,0x00a0}, {0xa080,0x00c0}, {0xa080,0x00e0},
 {0xc080,0x0000}, {0xc080,0x0020}, {0xc080,0x0040}, {0xc080,0x0060}, {0xc080,0x0080}, {0xc080,0x00a0}, {0xc080,0x00c0}, {0xc080,0x00e0},
 {0xe080,0x0000}, {0xe080,0x0020}, {0xe080,0x0040}, {0xe080,0x0060}, {0xe080,0x0080}, {0xe080,0x00a0}, {0xe080,0x00c0}, {0xe080,0x00e0}, // hBF
 {0x00c0,0x0000}, {0x00c0,0x0020}, {0x00c0,0x0040}, {0x00c0,0x0060}, {0x00c0,0x0080}, {0x00c0,0x00a0}, {0x00c0,0x00c0}, {0x00c0,0x00e0},
 {0x20c0,0x0000}, {0x20c0,0x0020}, {0x20c0,0x0040}, {0x20c0,0x0060}, {0x20c0,0x0080}, {0x20c0,0x00a0}, {0x20c0,0x00c0}, {0x20c0,0x00e0},
 {0x40c0,0x0000}, {0x40c0,0x0020}, {0x40c0,0x0040}, {0x40c0,0x0060}, {0x40c0,0x0080}, {0x40c0,0x00a0}, {0x40c0,0x00c0}, {0x40c0,0x00e0},
 {0x60c0,0x0000}, {0x60c0,0x0020}, {0x60c0,0x0040}, {0x60c0,0x0060}, {0x60c0,0x0080}, {0x60c0,0x00a0}, {0x60c0,0x00c0}, {0x60c0,0x00e0},
 {0x80c0,0x0000}, {0x80c0,0x0020}, {0x80c0,0x0040}, {0x80c0,0x0060}, {0x80c0,0x0080}, {0x80c0,0x00a0}, {0x80c0,0x00c0}, {0x80c0,0x00e0},
 {0xa0c0,0x0000}, {0xa0c0,0x0020}, {0xa0c0,0x0040}, {0xa0c0,0x0060}, {0xa0c0,0x0080}, {0xa0c0,0x00a0}, {0xa0c0,0x00c0}, {0xa0c0,0x00e0},
 {0xc0c0,0x0000}, {0xc0c0,0x0020}, {0xc0c0,0x0040}, {0xc0c0,0x0060}, {0xc0c0,0x0080}, {0xc0c0,0x00a0}, {0xfbf0,0x00ff}, {0xa0a4,0x00a0},
 {0x8080,0x0080}, {0x0000,0x00ff}, {0xff00,0x0000}, {0xff00,0x00ff}, {0x00ff,0x0000}, {0x00ff,0x00ff}, {0xffff,0x0000}, {0xffff,0x00ff}};

// Input buffer (for OSD)
static volatile void    *pvInBufVirt;
static volatile void    *pvInBufPhys;
static volatile void    *pvInBufMiu;
static unsigned long    buffersize;

// Base buffer (for SCL, DIP)
static volatile void    *pvBaseInBufVirt;
static volatile void    *pvBaseInBufPhys;
static volatile void    *pvBaseInBufMiu;
static volatile void    *pvBaseOutBufVirt;
static volatile void    *pvBaseOutBufPhys;
static volatile void    *pvBaseOutBufMiu;

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================
static MS_BOOL _InputBufferLoad(unsigned long bufsize, void *pvInBufVirt)
{
    unsigned long   readBufSize;
    struct file     *pstBinFd;
    
    if (strlen(FileName)) {
        // Open file
        pstBinFd = OpenFile(FileName, O_RDONLY, 0);

        if (pstBinFd) {
            CamOsPrintf("[RGN]Open file success: %s\r\n", FileName);
        }
        else {
            CamOsPrintf("[RGN]Open file failed: %s\r\n", FileName);
            return FALSE;
        }
        
        CamOsPrintf("[RGN_UT]Reading file...\r\n");
        readBufSize = ReadFile(pstBinFd, pvInBufVirt, bufsize);
        if (readBufSize == bufsize) {
            CamOsPrintf("[RGN_UT]Read file success: readBufSize = 0x%x\r\n", readBufSize);            
        }else{
            CamOsPrintf("[RGN_UT]Read file failed: readBufSize = 0x%x\r\n", readBufSize);
            return FALSE;
        }

        CloseFile(pstBinFd);
    }
    return TRUE;
}

static void _RGN_GopSetPalette(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopPixelFormat_e eFormat)
{
    int i;
    switch(eFormat) {
        case E_MHAL_RGN_PIXEL_FORMAT_I2:
            CamOsPrintf("Set I2 Palette...\r\n");            
            for(i=0;i<4;i++) {
                MHAL_RGN_GopSetPalette(eGopId, (MS_U8)(Palette_I2[i][1]>>8), (MS_U8)(Palette_I2[i][1]&0xFF),
                                               (MS_U8)(Palette_I2[i][0]>>8), (MS_U8)(Palette_I2[i][0]&0xFF), (MS_U8)i);
            }
            break;
        case E_MHAL_RGN_PIXEL_FORMAT_I4:
            CamOsPrintf("Set I4 Palette...\r\n");
            for(i=0;i<16;i++) {
                MHAL_RGN_GopSetPalette(eGopId, (MS_U8)(Palette_I4[i][1]>>8), (MS_U8)(Palette_I4[i][1]&0xFF),
                                               (MS_U8)(Palette_I4[i][0]>>8), (MS_U8)(Palette_I4[i][0]&0xFF), (MS_U8)i);
            }
            break;
        case E_MHAL_RGN_PIXEL_FORMAT_I8:
            CamOsPrintf("Set I8 Palette...\r\n");        
            for(i=0;i<256;i++) {
                MHAL_RGN_GopSetPalette(eGopId, (MS_U8)(Palette_I8[i][1]>>8), (MS_U8)(Palette_I8[i][1]&0xFF),
                                               (MS_U8)(Palette_I8[i][0]>>8), (MS_U8)(Palette_I8[i][0]&0xFF), (MS_U8)i);
            }
            break;
        default:
            CamOsPrintf("No Palette!Fmt=%d\r\n",eFormat); 
            break;
    }
}

static void _RGN_TestChipCapability(void)
{
    CamOsPrintf("=====GetChipCapability=====\r\n");
    CamOsPrintf("-----Format Support-----\r\n");
    CamOsPrintf("ARGB8888:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_FMT_SUPPORT,E_MHAL_RGN_PIXEL_FORMAT_ARGB8888));
    CamOsPrintf("ARGB1555:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_FMT_SUPPORT,E_MHAL_RGN_PIXEL_FORMAT_ARGB1555));
    CamOsPrintf("ARGB4444:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_FMT_SUPPORT,E_MHAL_RGN_PIXEL_FORMAT_ARGB4444));
    CamOsPrintf("RGB565:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_FMT_SUPPORT,E_MHAL_RGN_PIXEL_FORMAT_RGB565));
    CamOsPrintf("UV8Y8:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_FMT_SUPPORT,E_MHAL_RGN_PIXEL_FORMAT_UV8Y8));
    CamOsPrintf("I8:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_FMT_SUPPORT,E_MHAL_RGN_PIXEL_FORMAT_I8));
    CamOsPrintf("I4:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_FMT_SUPPORT,E_MHAL_RGN_PIXEL_FORMAT_I4));
    CamOsPrintf("I2:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_FMT_SUPPORT,E_MHAL_RGN_PIXEL_FORMAT_I2));
    
    CamOsPrintf("-----Width Alignment-----\r\n");
    CamOsPrintf("ARGB8888:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_WIDTH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_ARGB8888));
    CamOsPrintf("ARGB1555:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_WIDTH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_ARGB1555));
    CamOsPrintf("ARGB4444:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_WIDTH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_ARGB4444));
    CamOsPrintf("RGB565:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_WIDTH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_RGB565));
    CamOsPrintf("UV8Y8:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_WIDTH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_UV8Y8));
    CamOsPrintf("I8:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_WIDTH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_I8));
    CamOsPrintf("I4:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_WIDTH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_I4));
    CamOsPrintf("I2:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_WIDTH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_I2));

    CamOsPrintf("-----XPosition Alignment-----\r\n");
    CamOsPrintf("ARGB8888:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_XPOS_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_ARGB8888));
    CamOsPrintf("ARGB1555:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_XPOS_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_ARGB1555));
    CamOsPrintf("ARGB4444:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_XPOS_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_ARGB4444));
    CamOsPrintf("RGB565:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_XPOS_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_RGB565));
    CamOsPrintf("UV8Y8:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_XPOS_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_UV8Y8));
    CamOsPrintf("I8:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_XPOS_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_I8));
    CamOsPrintf("I4:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_XPOS_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_I4));
    CamOsPrintf("I2:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_XPOS_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_I2));

    CamOsPrintf("-----Pitch Alignment-----\r\n");
    CamOsPrintf("ARGB8888:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_PITCH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_ARGB8888));
    CamOsPrintf("ARGB1555:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_PITCH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_ARGB1555));
    CamOsPrintf("ARGB4444:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_PITCH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_ARGB4444));
    CamOsPrintf("RGB565:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_PITCH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_RGB565));
    CamOsPrintf("UV8Y8:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_PITCH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_UV8Y8));
    CamOsPrintf("I8:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_PITCH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_I8));
    CamOsPrintf("I4:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_PITCH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_I4));
    CamOsPrintf("I2:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_PITCH_ALIGNMENT,E_MHAL_RGN_PIXEL_FORMAT_I2));

    CamOsPrintf("GWin number:%d\r\n",MHAL_RGN_GetChipCapability(E_MHAL_RGN_CHIP_OSD_HW_GWIN_CNT,0));
    CamOsPrintf("=======================\r\n");
}

//------------------------------------------------------------------------------
//  Function    : RGNProbe
//  Description :
//------------------------------------------------------------------------------
static int RGNProbe(void)
{
    unsigned int i;
    //MS_BOOL bVYU;
    MS_U16 POS_MD[4][2] = {{0,0},{0,0},{0,0},{0,0}}; // [BwinId][Position]
    MS_U32 YUV_MD[4] = {0xFF4C54,0x15952B,0x6B1DFF,0x80FF80};    // [BwinId]
    MHAL_RGN_GopColorInvConfig_t tColInvCfg;
    MS_U16 u16DataLength;
    MS_U32 *pu32Data;
    
    if(TestChipCapEn) {
        _RGN_TestChipCapability();
    }
   
    // GOP
    if(GOPId!=-1) {
        CamOsPrintf("========Global========\r\n");
        CamOsPrintf("GOPId   = %d\r\n", GOPId);
        CamOsPrintf("StWinW  = %d\r\n", StWinW);
        CamOsPrintf("StWinH  = %d\r\n", StWinH);
        CamOsPrintf("CKeyEn  = %d\r\n", CKeyEn);
        CamOsPrintf("CKeyRGB = %d\r\n", CKeyRGB);
        CamOsPrintf("======================\r\n");
        
        if(Gwin0_En==0 && Gwin1_En==0) {
            MHAL_RGN_GopGwinDisable((MHAL_RGN_GopType_e)GOPId, E_MHAL_GOP_GWIN_ID_0);
            MHAL_RGN_GopGwinDisable((MHAL_RGN_GopType_e)GOPId, E_MHAL_GOP_GWIN_ID_1);        
            CamOsPrintf("No GWin enable!!!\r\n");
        } else {
            CamOsPrintf("Set Palette...\r\n");
            _RGN_GopSetPalette((MHAL_RGN_GopType_e)GOPId, (MHAL_RGN_GopPixelFormat_e)BufFmt);
            CamOsPrintf("Set Stretch Window...\r\n");
            MHAL_RGN_GopSetBaseWindow((MHAL_RGN_GopType_e)GOPId, (MS_U32)StWinW, (MS_U32)StWinH);
            CamOsPrintf("Set Color key...\r\n");
            //bVYU = ((MHAL_RGN_GopPixelFormat_e)BufFmt==E_MHAL_RGN_PIXEL_FORMAT_UV8Y8);
            MHAL_RGN_GopSetColorkey((MHAL_RGN_GopType_e)GOPId, (MS_BOOL)CKeyEn,
                                    (MS_U8)((CKeyRGB>>16)&0xFF), (MS_U8)((CKeyRGB>>8)&0xFF), (MS_U8)(CKeyRGB&0xFF));
            //MHAL_RGN_GopSetColorkey((MHAL_RGN_GopType_e)GOPId, FALSE,
            //                        (MS_U8)((CKeyRGB>>16)&0xFF), (MS_U8)((CKeyRGB>>8)&0xFF), (MS_U8)(CKeyRGB&0xFF), !bVYU);
            MHAL_RGN_GopSetAlphaZeroOpaque((MHAL_RGN_GopType_e)GOPId, (MS_BOOL)AZero, Gwin0_En?(!Gwin0_AType):(!Gwin1_AType), (MHAL_RGN_GopPixelFormat_e)BufFmt);
            
            if(Gwin0_En) {
                if(Gwin0_W==0)   {Gwin0_W   = BufW;}
                if(Gwin0_H==0)   {Gwin0_H   = BufH;}
                if(Gwin0_Fmt==-1){Gwin0_Fmt = BufFmt;}
                if(Gwin0_Std==0) {Gwin0_Std = UT_RGN_PIX2BYTE(BufW,BufFmt);}
                CamOsPrintf("++++++++GWin0++++++++\r\n");
                CamOsPrintf("x = %d\r\n",Gwin0_X);
                CamOsPrintf("y = %d\r\n",Gwin0_Y);
                CamOsPrintf("w = %d\r\n",Gwin0_W);
                CamOsPrintf("h = %d\r\n",Gwin0_H);
                CamOsPrintf("fmt   = %d\r\n",Gwin0_Fmt);
                CamOsPrintf("std   = %d\r\n",Gwin0_Std);
                CamOsPrintf("aType = %d\r\n",Gwin0_AType);
                CamOsPrintf("aVal  = %d\r\n",Gwin0_AVal);
                CamOsPrintf("+++++++++++++++++++++\r\n");
                    
                MHAL_RGN_GopGwinEnable((MHAL_RGN_GopType_e)GOPId, E_MHAL_GOP_GWIN_ID_0);
                CamOsPrintf("Set Pixel Format...\r\n");
                MHAL_RGN_GopGwinSetPixelFormat((MHAL_RGN_GopType_e)GOPId, E_MHAL_GOP_GWIN_ID_0, (MHAL_RGN_GopPixelFormat_e)Gwin0_Fmt);
                CamOsPrintf("Set Buffer... MIU address=0x%x\r\n", pvInBufMiu);
                MHAL_RGN_GopGwinSetBuffer((MHAL_RGN_GopType_e)GOPId, E_MHAL_GOP_GWIN_ID_0, pvInBufMiu);
                CamOsPrintf("Set GWin Window...\r\n");
                MHAL_RGN_GopGwinSetWindow((MHAL_RGN_GopType_e)GOPId, E_MHAL_GOP_GWIN_ID_0, 
                                          (MS_U32)Gwin0_W, (MS_U32)Gwin0_H, (MS_U32)Gwin0_Std, (MS_U32)Gwin0_X, (MS_U32)Gwin0_Y);
                CamOsPrintf("Set AlphaType...\r\n");
                MHAL_RGN_GopSetAlphaType((MHAL_RGN_GopType_e)GOPId, E_MHAL_GOP_GWIN_ID_0, (MHAL_RGN_GopGwinAlphaType_e)Gwin0_AType, (MS_U8)Gwin0_AVal);
                CamOsPrintf("Set GWin0 done!!!\r\n");
            }
            
            if(Gwin1_En) {
                if(Gwin1_W==0)   {Gwin1_W   = BufW;}
                if(Gwin1_H==0)   {Gwin1_H   = BufH;}
                if(Gwin1_Fmt==-1){Gwin1_Fmt = BufFmt;}
                if(Gwin1_Std==0) {Gwin1_Std = UT_RGN_PIX2BYTE(BufW,BufFmt);}
                CamOsPrintf("++++++++GWin1++++++++\r\n");
                CamOsPrintf("x = %d\r\n",Gwin1_X);
                CamOsPrintf("y = %d\r\n",Gwin1_Y);
                CamOsPrintf("w = %d\r\n",Gwin1_W);
                CamOsPrintf("h = %d\r\n",Gwin1_H);
                CamOsPrintf("fmt   = %d\r\n",Gwin1_Fmt);
                CamOsPrintf("std   = %d\r\n",Gwin1_Std);
                CamOsPrintf("aType = %d\r\n",Gwin1_AType);
                CamOsPrintf("aVal  = %d\r\n",Gwin1_AVal);
                CamOsPrintf("+++++++++++++++++\r\n");
                MHAL_RGN_GopGwinEnable((MHAL_RGN_GopType_e)GOPId, E_MHAL_GOP_GWIN_ID_1);
                MHAL_RGN_GopGwinSetPixelFormat((MHAL_RGN_GopType_e)GOPId, E_MHAL_GOP_GWIN_ID_1, (MHAL_RGN_GopPixelFormat_e)Gwin1_Fmt);
                MHAL_RGN_GopGwinSetBuffer((MHAL_RGN_GopType_e)GOPId, E_MHAL_GOP_GWIN_ID_1, pvInBufMiu);
                MHAL_RGN_GopGwinSetWindow((MHAL_RGN_GopType_e)GOPId, E_MHAL_GOP_GWIN_ID_1, 
                                          (MS_U32)Gwin1_W, (MS_U32)Gwin1_H, (MS_U32)Gwin1_Std, (MS_U32)Gwin1_X, (MS_U32)Gwin1_Y);
                MHAL_RGN_GopSetAlphaType((MHAL_RGN_GopType_e)GOPId, E_MHAL_GOP_GWIN_ID_1, (MHAL_RGN_GopGwinAlphaType_e)Gwin1_AType, (MS_U8)Gwin1_AVal);
                CamOsPrintf("Set GWin1 done!!!\r\n");
            }

            // Color inverse
            if(CI_En==1) {
                MHAL_RGN_GopSetColorInverseEnable((MHAL_RGN_GopType_e)GOPId, 1);
                CamOsPrintf("------ColorInv------\r\n");
                CamOsPrintf("Id  = %d\r\n",CI_Id);
                CamOsPrintf("Th1 = %d\r\n",CI_Th1);
                CamOsPrintf("Th2 = %d\r\n",CI_Th2);
                CamOsPrintf("Th3 = %d\r\n",CI_Th3);
                CamOsPrintf("W   = %d\r\n",CI_W);
                CamOsPrintf("H   = %d\r\n",CI_H);
                CamOsPrintf("X   = %d\r\n",CI_X);
                CamOsPrintf("Y   = %d\r\n",CI_Y);
                CamOsPrintf("BlkNumX  = %d\r\n",CI_BlkNumX);
                CamOsPrintf("BlkNumY  = %d\r\n",CI_BlkNumY);
                CamOsPrintf("--------------------\r\n");
                tColInvCfg.u16Th1 = CI_Th1;
                tColInvCfg.u16Th2 = CI_Th2;
                tColInvCfg.u16Th3 = CI_Th3;
                tColInvCfg.u16W = CI_W;
                tColInvCfg.u16H = CI_H;
                tColInvCfg.u16X = CI_X;
                tColInvCfg.u16Y = CI_Y;
                tColInvCfg.u16BlkNumX= CI_BlkNumX;
                tColInvCfg.u16BlkNumY= CI_BlkNumY;
                MHAL_RGN_GopSetColorInverseParam((MHAL_RGN_GopType_e)GOPId,(MHAL_RGN_GopColorInvWindowIdType_e)CI_Id,tColInvCfg);
                CamOsPrintf("Color inverse setting done!!!\r\n");
            } else if(CI_En==-1) {
                MHAL_RGN_GopSetColorInverseUpdate((MHAL_RGN_GopType_e)GOPId);
                CamOsPrintf("Color inverse update!!!\r\n");
            } else if(CI_En==2) { // read AE data
                u16DataLength = (CI_BlkNumX*CI_BlkNumY+31)/32; // ceiling
                pu32Data = (MS_U32 *)CamOsMemAlloc(u16DataLength*sizeof(MS_U32));
                MHAL_RGN_GopReadColorInverseData((MHAL_RGN_GopType_e)GOPId, (MHAL_RGN_GopColorInvWindowIdType_e)CI_Id, u16DataLength, pu32Data);
                CamOsPrintf("=======Read Color Inverse=======\r\n");
                for(i=0;i<u16DataLength;i++) {
                    CamOsPrintf("Addr=%d, Data=0x%x\r\n",i,*(pu32Data+i));
                }
                CamOsPrintf("================================\r\n");
                CamOsDirectMemRelease(pu32Data,u16DataLength*sizeof(MS_U32));
            } else if(CI_En==3) { // write AE data
                u16DataLength = (CI_BlkNumX*CI_BlkNumY+31)/32; // ceiling
                pu32Data = (MS_U32 *)CamOsMemAlloc(u16DataLength*sizeof(MS_U32));
                CamOsPrintf("Write Color Inverse\r\n");
                for(i=0;i<u16DataLength;i++) {
                    *(pu32Data+i) = 0x55555555;
                }                
                MHAL_RGN_GopWriteColorInverseData((MHAL_RGN_GopType_e)GOPId, (MHAL_RGN_GopColorInvWindowIdType_e)CI_Id, u16DataLength, pu32Data);
                MHAL_RGN_GopSetColorInverseUpdate((MHAL_RGN_GopType_e)GOPId);
                CamOsDirectMemRelease(pu32Data,u16DataLength*sizeof(MS_U32));
            } else {
                MHAL_RGN_GopSetColorInverseEnable((MHAL_RGN_GopType_e)GOPId, 0);
                CamOsPrintf("Color inverse disable!!!\r\n");
            }
        }
    }

    // Cover
    if(CoverId!=-1) {
        CamOsPrintf("========Global========\r\n");
        CamOsPrintf("CoverId     = %d\r\n", CoverId);
        CamOsPrintf("StWinW      = %d\r\n", StWinW);
        CamOsPrintf("StWinH      = %d\r\n", StWinH);
        CamOsPrintf("Bwin_En     = %d\r\n", Bwin_En);
        CamOsPrintf("Bwin_Pos_Md = %d\r\n", Bwin_Pos_Md);
        CamOsPrintf("Bwin_VYU_Md = %d\r\n", Bwin_VYU_Md);
        CamOsPrintf("Bwin_X      = %d\r\n", Bwin_X);
        CamOsPrintf("Bwin_Y      = %d\r\n", Bwin_Y);
        CamOsPrintf("Bwin_W      = %d\r\n", Bwin_W);
        CamOsPrintf("Bwin_H      = %d\r\n", Bwin_H);
        CamOsPrintf("======================\r\n");
        
        // Init Mode
        POS_MD[1][0] = StWinW-Bwin_W;
        POS_MD[2][0] = StWinW-Bwin_W;
        POS_MD[2][1] = StWinH-Bwin_H;
        POS_MD[3][1] = StWinH-Bwin_H;
                
        // 4 Bwins
        for(i=0;i<4;i++) {
            if((Bwin_En>>i)&1) {
                MHAL_RGN_CoverEnable(CoverId,i);
                if(Bwin_Pos_Md!=-1) {
                    MHAL_RGN_CoverSetWindow(CoverId,i,POS_MD[(i+Bwin_Pos_Md)%4][0],
                                                      POS_MD[(i+Bwin_Pos_Md)%4][1],
                                                      Bwin_W,Bwin_H);
                } else {
                    MHAL_RGN_CoverSetWindow(CoverId,i,Bwin_X,Bwin_Y,Bwin_W,Bwin_H);
                }
                if(Bwin_VYU_Md!=-1) {
                    MHAL_RGN_CoverSetColor(CoverId,i,YUV_MD[(i+Bwin_VYU_Md)%4]);
                    CamOsPrintf("Bwin%d: 0x%x\r\n",i,YUV_MD[(i+Bwin_VYU_Md)%4]);
                } else {
                    MHAL_RGN_CoverSetColor(CoverId,i,Bwin_VYU);
                }
                
            } else {
                MHAL_RGN_CoverDisable(CoverId,i);
            }
        }
    }
    return 0;
}

//------------------------------------------------------------------------------
//  Function    : RGNRemove
//  Description :
//------------------------------------------------------------------------------
static int RGNRemove(void)
{
    return 0;
}

static int  __init rgn_init(void)
{
    MHAL_RGN_GopInit();
    MHAL_RGN_CoverInit();

    if(GOPId!=-1) {
        // Allocate memory
        buffersize = (UT_RGN_PIX2BYTE(BufW, BufFmt)) * BufH;
        
        if(MIU==0) {
            CamOsDirectMemAlloc("RGN_IN_BUF", buffersize, &pvInBufVirt, &pvInBufPhys, &pvInBufMiu);
        } else {
            pvInBufPhys = (void *)MIU1_PHY_BASE;
            pvInBufVirt = (void *)ioremap(MIU1_PHY_BASE, buffersize);
            pvInBufMiu  = (void *)MIU1_MIU_BASE;
        }
        CamOsPrintf("===========InBuf==========\r\n");
        CamOsPrintf("InBuf width = %d\r\n", BufW);
        CamOsPrintf("InBuf height= %d\r\n", BufH);
        CamOsPrintf("InBuf fmt   = %d\r\n", BufFmt);
        CamOsPrintf("InBuf size  = 0x%x\r\n", buffersize);
        CamOsPrintf("pvInBufVirt = 0x%x\r\n", pvInBufVirt);
        CamOsPrintf("pvInBufPhys = 0x%x\r\n", pvInBufPhys);
        CamOsPrintf("pvInBufMiu  = 0x%x\r\n", pvInBufMiu);
        CamOsPrintf("===========================\r\n");
        
        // Load file into buffer
        if(_InputBufferLoad(buffersize, pvInBufVirt)==0) {
            return -1;
        }        
    }
    
    RGNProbe();
    
    CamOsDirectMemAlloc("RGN_BASE_IN_BUF", StWinW*StWinH*2, &pvBaseInBufVirt, &pvBaseInBufPhys, &pvBaseInBufMiu);
    CamOsPrintf("===========BaseInBuf==========\r\n");
    CamOsPrintf("InBuf size= 0x%x\r\n", StWinW*StWinH*2);
    CamOsPrintf("InBufVirt = 0x%x\r\n", pvBaseInBufVirt);
    CamOsPrintf("InBufPhys = 0x%x\r\n", pvBaseInBufPhys);
    CamOsPrintf("InBufMiu  = 0x%x\r\n", pvBaseInBufMiu);
    CamOsPrintf("==============================\r\n");
    
    CamOsDirectMemAlloc("RGN_BASE_OUT_BUF", StWinW*StWinH*2, &pvBaseOutBufVirt, &pvBaseOutBufPhys, &pvBaseOutBufMiu);
    CamOsPrintf("===========BaseOutBuf==========\r\n");
    CamOsPrintf("OutBuf size= 0x%x\r\n", StWinW*StWinH*2);
    CamOsPrintf("OutBufVirt = 0x%x\r\n", pvBaseOutBufVirt);
    CamOsPrintf("OutBufPhys = 0x%x\r\n", pvBaseOutBufPhys);
    CamOsPrintf("OutBufMiu  = 0x%x\r\n", pvBaseOutBufMiu);
    CamOsPrintf("===============================\r\n");
    
    return 0;
}

static void __exit rgn_exit(void)
{
    RGNRemove();
    // Release memory
    if(MIU==0) {
        CamOsDirectMemRelease(pvInBufVirt, buffersize);
    }
    CamOsDirectMemRelease(pvBaseInBufVirt, StWinW*StWinH*2);
    CamOsDirectMemRelease(pvBaseOutBufVirt, StWinW*StWinH*2);
    CamOsPrintf("RGN UT remove done!\r\n");
}

module_init(rgn_init);
module_exit(rgn_exit);
