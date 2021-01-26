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

#include "cam_os_wrapper.h"
#include "mhal_common.h"
#include "mhal_cmdq.h"
#include "mhal_rgn.h"
#include "rgn_file_access.h"
#include "hal_rgn_common.h"
#include "drv_rgn_common.h"
typedef enum
{
    E_RGN_UT_WALK_ID_0 = 0,
    E_RGN_UT_WALK_ID_1,
    E_RGN_UT_WALK_ID_2,
    E_RGN_UT_WALK_ID_3,
    E_RGN_UT_WALK_ID_4,
    E_RGN_UT_WALK_ID_5,
    E_RGN_UT_WALK_ID_6,
    E_RGN_UT_WALK_ID_7,
    E_RGN_UT_WALK_ID_8,
    E_RGN_UT_WALK_ID_9,
    E_RGN_UT_WALK_ID_NUM,
}HalRgnUTWalkType_e;
typedef struct
{
    bool bEn;
    CamOsThread Id;
    bool bSet;
    bool bpause;
}UTestTask_t;
typedef struct
{
    u32 *u32W;
    u32 *u32H;
    u32 *u32X;
    u32 *u32Y;
    u32 *u32MaxW;
    u32 *u32MaxH;
    u32 u32winType;
    u32 u32Id;
    u32 u32winId;
    u8 *bXRev;
    u8 *bYRev;
}UTestRamdomWalker_t;
typedef struct
{
    void    *pvOutBufVirt;
    void    *pvOutBufPhys;
    void    *pvOutBufMiu;
}OutBuf_t;
#define OUT_BUF_NUM 3


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
#define Default_WIN_Shift  3
#define Default_GWIN_MAX  4
#define Default_Str_Width  800
#define Default_Str_Height 480
#define Default_Buffer_Width  400
#define Default_Buffer_Height 400
#define Default_Buffer_size Default_Buffer_Width*Default_Buffer_Height*2
#define RGN_RIGHT 0
#define RGN_LEFT 1
#define RGN_LOWER 0
#define RGN_UPPER 1
#define RGN_RL 1
#define RGN_RU 2
#define RGN_LL 3
#define RGN_LU 0
#define RGN_CLOCKWISE 0
#define RGN_CLOCKWISE_REVERSE 1
#define AUTOTRIGGERDIP 0
#define UT_GET_GOP_ID_FROM_PORT(port) (!strcmp(port,"Port0")) ? E_MHAL_GOP_VPE_PORT0 :\
                                      (!strcmp(port,"Port1")) ? E_MHAL_GOP_VPE_PORT1 :\
                                      (!strcmp(port,"Port2")) ? E_MHAL_GOP_VPE_PORT2 :\
                                      (!strcmp(port,"Port3")) ? E_MHAL_GOP_VPE_PORT3 :\
                                      (!strcmp(port,"DIP")) ? E_MHAL_GOP_DIVP_PORT0 :\
                                      E_MHAL_GOP_TYPE_MAX

#define UT_GET_GOP_FMT_FROM_FMT(port) (!strcmp(port,"ARGB1555")) ? E_MHAL_RGN_PIXEL_FORMAT_ARGB1555 :\
                                      (!strcmp(port,"ARGB4444")) ? E_MHAL_RGN_PIXEL_FORMAT_ARGB4444 :\
                                      (!strcmp(port,"I2")) ? E_MHAL_RGN_PIXEL_FORMAT_I2 :\
                                      (!strcmp(port,"I4")) ? E_MHAL_RGN_PIXEL_FORMAT_I4 :\
                                      (!strcmp(port,"I8")) ? E_MHAL_RGN_PIXEL_FORMAT_I8 :\
                                      E_MHAL_RGN_PIXEL_FORMAT_MAX


#define UT_GET_COV_ID_FROM_PORT(port) (!strcmp(port,"Port0")) ? E_MHAL_COVER_VPE_PORT0 :\
                                      (!strcmp(port,"Port1")) ? E_MHAL_COVER_VPE_PORT1 :\
                                      (!strcmp(port,"Port2")) ? E_MHAL_COVER_VPE_PORT2 :\
                                      (!strcmp(port,"Port3")) ? E_MHAL_COVER_VPE_PORT3 :\
                                      (!strcmp(port,"DIP")) ? E_MHAL_COVER_DIVP_PORT0 :\
                                      E_MHAL_GOP_TYPE_MAX

#define UT_GET_WALK_ID_FROM_STR(x) (!strcmp(x,"0")) ? E_RGN_UT_WALK_ID_0 :\
                                      (!strcmp(x,"1")) ? E_RGN_UT_WALK_ID_1 :\
                                      (!strcmp(x,"2")) ? E_RGN_UT_WALK_ID_2 :\
                                      (!strcmp(x,"3")) ? E_RGN_UT_WALK_ID_3 :\
                                      (!strcmp(x,"4")) ? E_RGN_UT_WALK_ID_4 :\
                                      (!strcmp(x,"5")) ? E_RGN_UT_WALK_ID_5 :\
                                      (!strcmp(x,"6")) ? E_RGN_UT_WALK_ID_6 :\
                                      (!strcmp(x,"7")) ? E_RGN_UT_WALK_ID_7 :\
                                      (!strcmp(x,"8")) ? E_RGN_UT_WALK_ID_8 :\
                                      (!strcmp(x,"9")) ? E_RGN_UT_WALK_ID_9 :\
                                      E_RGN_UT_WALK_ID_NUM

#define DrvRgnScnprintf(buf, size, _fmt, _args...)        scnprintf(buf, size, _fmt, ## _args)

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
/*
// Buffer size
static unsigned int BufW = 0;	
static unsigned int BufH = 0;	
static unsigned int BufFmt = 0;	

static unsigned int TestChipCapEn = 0;	


// Input buffer (for OSD)

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
*/
static char FileName[128] = {0};
static  void    *pvInBufVirt = NULL;
static  void    *pvInBufPhys;
static  void    *pvInBufMiu;
static  int    buffersize = Default_Buffer_size;
static  int    BufferStride = Default_Buffer_Width;

//static  OutBuf_t outBuf[OUT_BUF_NUM];
//static  int      outbuffersize = 800*480*2;
static  u32      COVER_COLOR[4] = {0xFF4C54,0x15952B,0x6B1DFF,0x80FF80};

UTestTask_t gstUtGopTaskCfg;
UTestTask_t gstUtCovTaskCfg;
u32 gu32BwinColor[E_MHAL_COVER_TYPE_MAX][E_MHAL_COVER_LAYER_MAX];
u8 gu8BwinEn[E_MHAL_COVER_TYPE_MAX][E_MHAL_COVER_LAYER_MAX];
u32 gu32BwinX[E_MHAL_COVER_TYPE_MAX][E_MHAL_COVER_LAYER_MAX];
u32 gu32BwinY[E_MHAL_COVER_TYPE_MAX][E_MHAL_COVER_LAYER_MAX];
u32 gu32BwinW[E_MHAL_COVER_TYPE_MAX][E_MHAL_COVER_LAYER_MAX];
u32 gu32BwinH[E_MHAL_COVER_TYPE_MAX][E_MHAL_COVER_LAYER_MAX];
MHAL_RGN_GopPixelFormat_e genFmt[E_MHAL_GOP_TYPE_MAX];
u16 gu16Xoffset[E_MHAL_GOP_TYPE_MAX][E_MHAL_GOP_GWIN_ID_MAX];
u32 gu32StrWidth[E_MHAL_GOP_TYPE_MAX];
u32 gu32StrHeight[E_MHAL_GOP_TYPE_MAX];
MHAL_RGN_GopGwinAlphaType_e genAlphaType[E_MHAL_GOP_TYPE_MAX][E_MHAL_GOP_GWIN_ID_MAX];
u32 gu32Alpha0[E_MHAL_GOP_TYPE_MAX][E_MHAL_GOP_GWIN_ID_MAX];
u32 gu32Alpha1[E_MHAL_GOP_TYPE_MAX][E_MHAL_GOP_GWIN_ID_MAX];
u8 gu8ColorKeyR[E_MHAL_GOP_TYPE_MAX];
u8 gu8ColorKeyG[E_MHAL_GOP_TYPE_MAX];
u8 gu8ColorKeyB[E_MHAL_GOP_TYPE_MAX];
u8 gu8ColorKeyEn[E_MHAL_GOP_TYPE_MAX];
u8 gu8GwinEn[E_MHAL_GOP_TYPE_MAX][E_MHAL_GOP_GWIN_ID_MAX];
u32 gu32GwinX[E_MHAL_GOP_TYPE_MAX][E_MHAL_GOP_GWIN_ID_MAX];
u32 gu32GwinY[E_MHAL_GOP_TYPE_MAX][E_MHAL_GOP_GWIN_ID_MAX];
u32 gu32GwinW[E_MHAL_GOP_TYPE_MAX][E_MHAL_GOP_GWIN_ID_MAX];
u32 gu32GwinH[E_MHAL_GOP_TYPE_MAX][E_MHAL_GOP_GWIN_ID_MAX];
u8 gwinYRev[E_MHAL_GOP_TYPE_MAX][E_MHAL_GOP_GWIN_ID_MAX];
u8 gwinXRev[E_MHAL_GOP_TYPE_MAX][E_MHAL_GOP_GWIN_ID_MAX];
u8 bwinYRev[E_MHAL_COVER_TYPE_MAX][E_MHAL_COVER_LAYER_MAX];
u8 bwinXRev[E_MHAL_COVER_TYPE_MAX][E_MHAL_COVER_LAYER_MAX];

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
void _RGN_GopSetPalette(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopPixelFormat_e eFormat)
{
    int i;
    switch(eFormat) {
        case E_MHAL_RGN_PIXEL_FORMAT_I2:
            CamOsPrintf("Set I2 Palette...\r\n");            
            for(i=0;i<4;i++) {
                MHAL_RGN_GopSetPalette(eGopId, (MS_U8)(Palette_I2[i][1]>>8), (MS_U8)(Palette_I2[i][1]&0xFF),
                                               (MS_U8)(Palette_I2[i][0]>>8), (MS_U8)(Palette_I2[i][0]&0xFF), (MS_U8)i);
            }
            MHAL_RGN_GopSetPalette(eGopId, (MS_U8)0, (MS_U8)0,
                                           (MS_U8)0, (MS_U8)0, (MS_U8)255);
            break;
        case E_MHAL_RGN_PIXEL_FORMAT_I4:
            CamOsPrintf("Set I4 Palette...\r\n");
            for(i=0;i<16;i++) {
                MHAL_RGN_GopSetPalette(eGopId, (MS_U8)(Palette_I4[i][1]>>8), (MS_U8)(Palette_I4[i][1]&0xFF),
                                               (MS_U8)(Palette_I4[i][0]>>8), (MS_U8)(Palette_I4[i][0]&0xFF), (MS_U8)i);
            }
            MHAL_RGN_GopSetPalette(eGopId, (MS_U8)0, (MS_U8)0,
                                           (MS_U8)0, (MS_U8)0, (MS_U8)255);
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
MS_BOOL _InputBufferLoad(unsigned long bufsize, void *pvInBufVirt)
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
void Rgn_UT_Destroy_Task(UTestTask_t *pCfg)
{
    pCfg->bEn = 0;
    pCfg->bSet = 0;
    if(pCfg->Id)
    {
        if(!CamOsThreadStop(pCfg->Id))
            pCfg->Id = NULL;
    }
}

void Rgn_UT_Case0(void)
{
    MHAL_RGN_GopType_e enGop;
    MHAL_RGN_GopGwinId_e enGwin;
    MHAL_RGN_CoverType_e enId;
    MHAL_RGN_CoverLayerId_e enBwin;
    DRVRGNERR("%s",__FUNCTION__);
    for(enId=E_MHAL_COVER_VPE_PORT0;enId<E_MHAL_COVER_TYPE_MAX;enId++)
    {
        for(enBwin=E_MHAL_COVER_LAYER_0;enBwin<E_MHAL_COVER_LAYER_MAX;enBwin++)
        {
            MHAL_RGN_CoverDisable(enId,enBwin);
        }
    }
    for(enGop=E_MHAL_GOP_VPE_PORT0;enGop<E_MHAL_GOP_TYPE_MAX;enGop++)
    {
        for(enGwin=E_MHAL_GOP_GWIN_ID_0;enGwin<E_MHAL_GOP_GWIN_ID_MAX;enGwin++)
        {
            MHAL_RGN_GopGwinDisable(enGop,enGwin);
        }
    }
    if(pvInBufVirt)
    {
        CamOsDirectMemRelease(pvInBufVirt,buffersize);
        pvInBufVirt = NULL;
        pvInBufPhys = NULL;
        pvInBufMiu = NULL;
    }
    Rgn_UT_Destroy_Task(&gstUtGopTaskCfg);
    Rgn_UT_Destroy_Task(&gstUtCovTaskCfg);
}
void Rgn_UT_Case1(void)
{
    MHAL_RGN_GopType_e enGop;
    MHAL_RGN_GopGwinId_e enGwin;
    MHAL_RGN_GopWindowConfig_t tSrcWinCfg;
    DRVRGNERR("%s",__FUNCTION__);
    CamOsDirectMemAlloc("RGN_IN_BUF", buffersize, &pvInBufVirt, &pvInBufPhys, &pvInBufMiu);
    MHAL_RGN_GopInit();
    for(enGop=E_MHAL_GOP_VPE_PORT0;enGop<E_MHAL_GOP_TYPE_MAX;enGop++)
    {
        tSrcWinCfg.u32Height = Default_Str_Height;
        tSrcWinCfg.u32Width = Default_Str_Width;
        MHAL_RGN_GopSetBaseWindow(enGop,&tSrcWinCfg,&tSrcWinCfg);
        //_RGN_GopSetPalette(enGop,E_MHAL_RGN_PIXEL_FORMAT_I2);
        MHAL_RGN_GopSetColorkey(enGop,1,0x80,0x80,0x80);
        MHAL_RGN_GopSetAlphaZeroOpaque(enGop,1,1,E_MHAL_RGN_PIXEL_FORMAT_ARGB1555);
        for(enGwin=E_MHAL_GOP_GWIN_ID_0;enGwin<E_MHAL_GOP_GWIN_ID_MAX;enGwin++)
        {
            MHAL_RGN_GopGwinSetPixelFormat(enGop,enGwin,E_MHAL_RGN_PIXEL_FORMAT_I2);
            MHAL_RGN_GopGwinSetWindow(enGop,enGwin,64,64,64,0,0);
            MHAL_RGN_GopGwinSetBufferXoffset(enGop,enGwin,5);
            MHAL_RGN_GopGwinSetBuffer(enGop,enGwin,(u32)pvInBufMiu);
            MHAL_RGN_GopGwinEnable(enGop,enGwin);
            MHAL_RGN_GopSetAlphaType(enGop,enGwin,E_MHAL_GOP_GWIN_ALPHA_CONSTANT,20);
            MHAL_RGN_GopSetArgb1555AlphaVal(enGop,enGwin,E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0,10);
            MHAL_RGN_GopSetArgb1555AlphaVal(enGop,enGwin,E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1,50);
            //MHAL_RGN_GopGwinDisable(enGop,enGwin);
        }
    }
    MHAL_RGN_GopDeinit();
}
void Rgn_UT_Case2(void)
{
    MHAL_RGN_CoverType_e enId;
    MHAL_RGN_CoverLayerId_e enBwin;
    DRVRGNERR("%s",__FUNCTION__);
    MHAL_RGN_CoverInit();
    for(enId=E_MHAL_COVER_VPE_PORT0;enId<E_MHAL_COVER_TYPE_MAX;enId++)
    {
        if(enId==E_MHAL_COVER_VPE_PORT3)
            continue;
        for(enBwin=E_MHAL_COVER_LAYER_0;enBwin<E_MHAL_COVER_LAYER_MAX;enBwin++)
        {
            MHAL_RGN_CoverSetColor(enId,enBwin,0x123456);
            MHAL_RGN_CoverSetWindow(enId,enBwin,10,20,30,40);
            MHAL_RGN_CoverEnable(enId,enBwin);
        }
    }
    MHAL_RGN_CoverDeinit();
}
void Rgn_UT_CaseG(const char *buf, u32 n)
{
    char port[8];
    int u32gwin = 0;
    int u32para[5];
    MHAL_RGN_GopType_e enGop;
    const char *str = buf;
    //"Test Case 3 : BY Port Test GWIN, echo 3 (PORT_ID) (GWINID) (bEn,x,y,w,h) > test\n"
    sscanf(str, "%s", port);
    str+= strlen(port);
    sscanf(str+1, "%d %d %d %d %d %d", &u32gwin,&u32para[0],&u32para[1],&u32para[2],&u32para[3],&u32para[4]);
    DRVRGNDBGERR("RGN UT G OSD:%s u32size:%d u32gwin:%d [en,x,y,w,h] = [%d,%d,%d,%d,%d]\n",
        port,strlen(port),u32gwin,u32para[0],u32para[1],u32para[2],u32para[3],u32para[4]);
    enGop = UT_GET_GOP_ID_FROM_PORT(port);
    MHAL_RGN_GopGwinSetWindow(enGop,u32gwin,u32para[3],u32para[4],BufferStride,u32para[1],u32para[2]);
    if(u32para[0])
    {
        MHAL_RGN_GopGwinEnable(enGop,u32gwin);
    }
    else
    {
        MHAL_RGN_GopGwinDisable(enGop,u32gwin);
    }
    gu8GwinEn[enGop][u32gwin] = u32para[0];
    gu32GwinX[enGop][u32gwin] = u32para[1];
    gu32GwinY[enGop][u32gwin] = u32para[2];
    gu32GwinW[enGop][u32gwin] = u32para[3];
    gu32GwinH[enGop][u32gwin] = u32para[4];
}
void Rgn_UT_CaseB(const char *buf, u32 n)
{
    char port[8];
    int u32bwin = 0;
    int u32para[5];
    MHAL_RGN_CoverType_e enCov;
    const char *str = buf;
    //"Test Case 4 : BY Port Test BWIN, echo 4 (PORT_ID) (BWINID) (x,y,w,h) > test\n"
    sscanf(str, "%s", port);
    str+= strlen(port);
    sscanf(str+1, "%d %d %d %d %d %d", &u32bwin,&u32para[0],&u32para[1],&u32para[2],&u32para[3],&u32para[4]);
    DRVRGNDBGERR("RGN UT B OSD:%s u32size:%d u32bwin:%d [en,x,y,w,h] = [%d,%d,%d,%d,%d]\n",
        port,strlen(port),u32bwin,u32para[0],u32para[1],u32para[2],u32para[3],u32para[4]);
    enCov = UT_GET_COV_ID_FROM_PORT(port);
    MHAL_RGN_CoverSetWindow(enCov,u32bwin,u32para[1],u32para[2],u32para[3],u32para[4]);
    if(u32para[0])
    {
        MHAL_RGN_CoverEnable(enCov,u32bwin);
    }
    else
    {
        MHAL_RGN_CoverDisable(enCov,u32bwin);
    }
    gu8BwinEn[enCov][u32bwin] = u32para[0];
    gu32BwinX[enCov][u32bwin] = u32para[1];
    gu32BwinY[enCov][u32bwin] = u32para[2];
    gu32BwinW[enCov][u32bwin] = u32para[3];
    gu32BwinH[enCov][u32bwin] = u32para[4];
}
void Rgn_UT_CaseC(const char *buf, u32 n)
{
    char port[8];
    int u32para[4];
    const char *str = buf;
    MHAL_RGN_GopType_e enGop;
    //"Test Case C : Change Color Key, echo 7 (PORT_ID) (en,r,g,b) > test\n"
    sscanf(str, "%s", port);
    str+= strlen(port);
    sscanf(str+1, "%d %d %d %d",&u32para[0],&u32para[1],&u32para[2],&u32para[3]);
    DRVRGNERR("RGN UT C OSD:%s u32size:%d [en,r,g,b] = [%d,%d,%d,%d]\n",
        port,strlen(port),u32para[0],u32para[1],u32para[2],u32para[3]);
    enGop = UT_GET_GOP_ID_FROM_PORT(port);
    MHAL_RGN_GopSetColorkey(enGop,u32para[0],u32para[1],u32para[2],u32para[3]);
    gu8ColorKeyR[enGop] = u32para[1];
    gu8ColorKeyG[enGop] = u32para[2];
    gu8ColorKeyB[enGop] = u32para[3];
    gu8ColorKeyEn[enGop] = u32para[0];
}
void Rgn_UT_CaseA(const char *buf, u32 n)
{
    char port[8];
    int u32gwin = 0;
    int u32para[4];
    const char *str = buf;
    MHAL_RGN_GopType_e enGop;
    //"Test Case A : Change Alpha, echo 8 (PORT_ID)(GWINID) (type,con,a0,a1) > test\n"
    sscanf(str, "%s", port);
    str+= strlen(port);
    sscanf(str+1, "%d %d", &u32gwin,&u32para[0]);
    str = str+1+4;
    enGop = UT_GET_GOP_ID_FROM_PORT(port);
    if(u32para[0]==E_MHAL_GOP_GWIN_ALPHA_CONSTANT)
    {
        sscanf(str, "%d",&u32para[1]);
        DRVRGNERR("RGN UT A OSD:%s u32size:%d u32gwin:%d [type,con] = [%d,%d]\n",
            port,strlen(port),u32gwin,u32para[0],u32para[1]);
    }
    else
    {
        sscanf(str, "%d %d",&u32para[1],&u32para[2]);
        DRVRGNERR("RGN UT A OSD:%s u32size:%d u32gwin:%d [type,a0,a1] = [%d,%d,%d]\n",
            port,strlen(port),u32gwin,u32para[0],u32para[1],u32para[2]);
        MHAL_RGN_GopSetArgb1555AlphaVal(enGop,u32gwin,E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0,u32para[1]);
        MHAL_RGN_GopSetArgb1555AlphaVal(enGop,u32gwin,E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1,u32para[2]);
    }
    MHAL_RGN_GopSetAlphaType(enGop,u32gwin,u32para[0],u32para[1]);
    genAlphaType[enGop][u32gwin] = u32para[0];
    gu32Alpha0[enGop][u32gwin] = u32para[1];
    gu32Alpha1[enGop][u32gwin] = u32para[2];
}
void Rgn_UT_CaseV(const char *buf, u32 n)
{
    char port[8];
    int u32bwin = 0;
    int u32para[3];
    MHAL_RGN_CoverType_e enCov;
    const char *str = buf;
    //"Test Case V : Change BWIN Value, echo 9 (PORT_ID) (BWIN) (r,g,b) > test\n"
    sscanf(str, "%s", port);
    str+= strlen(port);
    sscanf(str+1, "%d %d %d %d",&u32bwin,&u32para[0],&u32para[1],&u32para[2]);
    DRVRGNERR("RGN UT V OSD:%s u32size:%d u32bwin:%d [r,g,b] = [%d,%d,%d]\n",
        port,strlen(port),u32bwin,u32para[0],u32para[1],u32para[2]);
    enCov = UT_GET_COV_ID_FROM_PORT(port);
    MHAL_RGN_CoverSetColor(enCov,u32bwin,(u32para[2]|u32para[1]<<8|u32para[0]<<16));
    gu32BwinColor[enCov][u32bwin] = (u32para[2]|u32para[1]<<8|u32para[0]<<16);
}
void Rgn_UT_CaseF(const char *buf, u32 n)
{
    char port[8];
    char Fmt[16];
    int i;
    MHAL_RGN_GopType_e enGop;
    MHAL_RGN_GopPixelFormat_e enFmt;
    const char *str = buf;
    //"Test Case F : Change Fmt WIN, echo F (PORT_ID) (Fmt)> test\n"
    sscanf(str, "%s %s", port,Fmt);
    DRVRGNERR("RGN UT F OSD:%s u32size:%d Fmt:%s\n",
        port,strlen(port),Fmt);
    enGop = UT_GET_GOP_ID_FROM_PORT(port);
    enFmt = UT_GET_GOP_FMT_FROM_FMT(Fmt);
    for(i=0;i<E_MHAL_GOP_GWIN_ID_MAX;i++)
    {
        MHAL_RGN_GopGwinSetPixelFormat(enGop,i,enFmt);
    }
    _RGN_GopSetPalette(enGop,enFmt);
    genFmt[enGop] = enFmt;
}
void Rgn_UT_CaseM(const char *buf, u32 n)
{
    const char *str = buf;
    MHAL_RGN_GopType_e enGop;
    MHAL_RGN_GopGwinId_e enGwin;
    char File[128] = {0};
    int size;
    //"Test Case M : Change Fmt WIN, echo F (PORT_ID) (Fmt)> test\n"
    sscanf(str, "%s %d %d", File,&size,&BufferStride);
    if(!strcmp(File,FileName))
    {
        buffersize = size;
        if(pvInBufVirt==NULL)
        {
            CamOsDirectMemAlloc("RGN_IN_BUF", buffersize, &pvInBufVirt, &pvInBufPhys, &pvInBufMiu);
            _InputBufferLoad(buffersize,pvInBufVirt);
        }
    }
    else
    {
        if(pvInBufVirt)
        {
            CamOsDirectMemRelease(pvInBufVirt,buffersize);
            pvInBufVirt = NULL;
            pvInBufPhys = NULL;
            pvInBufMiu = NULL;
        }
        buffersize = size;
        memcpy(FileName,File,128);
        CamOsDirectMemAlloc("RGN_IN_BUF", buffersize, &pvInBufVirt, &pvInBufPhys, &pvInBufMiu);
        _InputBufferLoad(buffersize,pvInBufVirt);
    }
    for(enGop=E_MHAL_GOP_VPE_PORT0;enGop<E_MHAL_GOP_TYPE_MAX;enGop++)
    {
        for(enGwin=E_MHAL_GOP_GWIN_ID_0;enGwin<E_MHAL_GOP_GWIN_ID_MAX;enGwin++)
        {
            MHAL_RGN_GopGwinSetBuffer(enGop,enGwin,(u32)pvInBufMiu);
        }
    }
    DRVRGNERR("RGN UT M Dir:%s size:%d\n",FileName,buffersize);
}
void Rgn_UT_CaseS(const char *buf, u32 n)
{
    char port[8];
    int u32para[2];
    MHAL_RGN_GopType_e enGop;
    MHAL_RGN_GopWindowConfig_t tSrcWinCfg;
    const char *str = buf;
    //"Test Case S : Change Str WIN, echo S (PORT_ID) (w,h)> test\n"
    sscanf(str, "%s", port);
    str+= strlen(port);
    sscanf(str+1, "%d %d", &u32para[0],&u32para[1]);
    DRVRGNERR("RGN UT S OSD:%s u32size:%d [w,h] = [%d,%d]\n",
        port,strlen(port),u32para[0],u32para[1]);
    enGop = UT_GET_GOP_ID_FROM_PORT(port);
    tSrcWinCfg.u32Width = u32para[0];
    tSrcWinCfg.u32Height = u32para[1];
    MHAL_RGN_GopSetBaseWindow(enGop,&tSrcWinCfg,&tSrcWinCfg);
    gu32StrWidth[enGop] = u32para[0];
    gu32StrHeight[enGop] = u32para[1];
}
void Rgn_UT_CaseX(const char *buf, u32 n)
{
    char port[8];
    int u32gwin = 0;
    int u32para;
    MHAL_RGN_GopType_e enGop;
    const char *str = buf;
    //"Test Case X : Change XOffset, echo X (PORT_ID) (GWINID) val > test\n"
    sscanf(str, "%s", port);
    str+= strlen(port);
    sscanf(str+1, "%d %d", &u32gwin,&u32para);
    DRVRGNERR("RGN UT X OSD:%s u32size:%d u32gwin:%d [x] = [%d]\n",
        port,strlen(port),u32gwin,u32para);
    enGop = UT_GET_GOP_ID_FROM_PORT(port);
    MHAL_RGN_GopGwinSetBufferXoffset(enGop,u32gwin,(u16)u32para);
    gu16Xoffset[enGop][u32gwin] = u32para;
    
}
void Rgn_UT_StrCatPort(char *str, char *port,const char *val)
{
    memset(str,0,64);
    strcpy(str,port);
    strcat(str,val);

}
void Rgn_UT_Strcat(char *str,const char *val)
{
    strcat(str,val);
}
void Rgn_UT_CallWindowCase
(char *str, char *port,const char *Gwin,const char *offsetX,const char *offsetY,const char *Width,const char *Height)
{
    Rgn_UT_StrCatPort(str,port,Gwin);
    Rgn_UT_Strcat(str,offsetX);
    Rgn_UT_Strcat(str," ");
    Rgn_UT_Strcat(str,offsetY);
    Rgn_UT_Strcat(str," ");
    Rgn_UT_Strcat(str,Width);
    Rgn_UT_Strcat(str," ");
    Rgn_UT_Strcat(str,Height);

}
void Rgn_UT_Case3(const char *buf, u32 n)
{
    char port[8];
    char fmt[16];
    char strA[64];
    int u32gwin = 0;
    char Width[8];
    char Height[8];
    char offsetX[8];
    char offsetY[8];
    char gwin[8];
    int u32Width = 0;
    int u32Height = 0;
    int g;
    MHAL_RGN_GopWindowConfig_t tSrcWinCfg;
    MHAL_RGN_GopType_e enGop;
    const char *str = buf;
    //"Test Case 3 : BY Port Test GWIN open 4 place, echo 3 (PORT_ID) (Fmt)(GWINNUM)[w h] > test\n"
    sscanf(str, "%s %s", port,fmt);
    str+= strlen(port);
    str++;
    str+= strlen(fmt);
    sscanf(str+1, "%d %s %s", &u32gwin,Width,Height);
    sscanf(str+1, "%d %d %d", &u32gwin,&u32Width,&u32Height);
    DRVRGNERR("RGN UT 3 OSD:%s u32size:%d u32gwin cnt:%d Fmt:%s\n",
        port,strlen(port),u32gwin,fmt);
    Rgn_UT_CaseF(buf,n);
    enGop = UT_GET_GOP_ID_FROM_PORT(port);
    MHAL_RGN_GopSetColorkey(enGop,gu8ColorKeyEn[enGop],gu8ColorKeyR[enGop],gu8ColorKeyG[enGop],gu8ColorKeyB[enGop]);
    tSrcWinCfg.u32Width = gu32StrWidth[enGop];
    tSrcWinCfg.u32Height = gu32StrWidth[enGop];
    MHAL_RGN_GopSetBaseWindow(enGop,&tSrcWinCfg,&tSrcWinCfg);
    DrvRgnScnprintf(offsetX,8,"%d",(gu32StrWidth[enGop]-u32Width));
    DrvRgnScnprintf(offsetY,8,"%d",(gu32StrHeight[enGop]-u32Height));
    for(g=0;g<u32gwin;g++)
    {
        DrvRgnScnprintf(gwin,8," %d 1 ",g);
        Rgn_UT_CallWindowCase(strA,port,gwin,(g==1||g==3) ? offsetX :"0",(g==2||g==3) ? offsetY :"0",Width,Height);
        Rgn_UT_CaseG(strA,n);
    }
}
void Rgn_UT_Case4(const char *buf, u32 n)
{
    char port[8];
    char strA[64];
    int u32bwin = 0;
    char Width[8];
    char Height[8];
    char offsetX[8];
    char offsetY[8];
    int u32Width = 0;
    int u32Height = 0;
    int b;
    char bwin[8];
    MHAL_RGN_GopType_e enCov;
    const char *str = buf;
    //"Test Case 4 : BY Port Test GWIN open 4 place, echo 3 (PORT_ID) (BWINNUM)[w h] > test\n"
    sscanf(str, "%s", port);
    str+= strlen(port);
    sscanf(str+1, "%d %s %s", &u32bwin,Width,Height);
    sscanf(str+1, "%d %d %d", &u32bwin,&u32Width,&u32Height);
    DRVRGNERR("RGN UT 4 OSD:%s u32size:%d u32bwin cnt:%d\n",
        port,strlen(port),u32bwin);
    enCov = UT_GET_COV_ID_FROM_PORT(port);
    DrvRgnScnprintf(offsetX,8,"%d",(gu32StrWidth[enCov]-u32Width));
    DrvRgnScnprintf(offsetY,8,"%d",(gu32StrHeight[enCov]-u32Height));
    for(b=0;b<u32bwin;b++)
    {
        MHAL_RGN_CoverSetColor(enCov,b,gu32BwinColor[enCov][b]);
        DrvRgnScnprintf(bwin,8," %d 1 ",b);
        Rgn_UT_CallWindowCase(strA,port,bwin,(b==1||b==3) ? offsetX :"0",(b==2||b==3) ? offsetY :"0",Width,Height);
        Rgn_UT_CaseB(strA,n);
    }
}
void UT_Walk_Move_Protect(UTestRamdomWalker_t *pCfg,bool bXY)
{
    if(bXY)
    {
        if((*(pCfg->u32X)+*(pCfg->u32W))>*(pCfg->u32MaxW))
        {
            *(pCfg->u32X) = *(pCfg->u32MaxW)-*(pCfg->u32W);
        }
        if((*(pCfg->u32Y)+*(pCfg->u32H))>*(pCfg->u32MaxH))
        {
            *(pCfg->u32Y) = *(pCfg->u32MaxH)-*(pCfg->u32H);
        }
        if((*(pCfg->u32X)+*(pCfg->u32W))<*(pCfg->u32W))
        {
            *(pCfg->u32X) = 0;
        }
        if((*(pCfg->u32Y)+*(pCfg->u32H))<*(pCfg->u32H))
        {
            *(pCfg->u32Y) = 0;
        }
    }
    else
    {
        if((*(pCfg->u32X)+*(pCfg->u32W))>*(pCfg->u32MaxW))
        {
            *(pCfg->u32W) = *(pCfg->u32MaxW)-*(pCfg->u32X);
        }
        if((*(pCfg->u32Y)+*(pCfg->u32H))>*(pCfg->u32MaxH))
        {
            *(pCfg->u32H) = *(pCfg->u32MaxH)-*(pCfg->u32Y);
        }
        if((s32)*(pCfg->u32W)<=0)
        {
            *(pCfg->u32W) = 1;
        }
        if((s32)*(pCfg->u32H)<=0)
        {
            *(pCfg->u32H) = 1;
        }
    }
}
void UT_Walk_Move_Original(UTestRamdomWalker_t *pCfg,bool bReverse)
{
    if(bReverse == RGN_RU){
        *(pCfg->u32X) = *(pCfg->u32MaxW)-*(pCfg->u32W);
        *(pCfg->u32Y) = 0;
    }else if(bReverse == RGN_RL){
        *(pCfg->u32X) = *(pCfg->u32MaxW)-*(pCfg->u32W);
        *(pCfg->u32Y) = *(pCfg->u32MaxH)-*(pCfg->u32H);
    }else if(bReverse == RGN_LL){
        *(pCfg->u32X) = 0;
        *(pCfg->u32Y) = *(pCfg->u32MaxH)-*(pCfg->u32H);
    }else{
        *(pCfg->u32X) = 0;
        *(pCfg->u32Y) = 0;
    }
    
}
void UT_Walk_Move_NextLine(UTestRamdomWalker_t *pCfg,bool bReverse)
{
    if(bReverse){
        *(pCfg->u32X) = *(pCfg->u32MaxW)-*(pCfg->u32W);
    }else{
        *(pCfg->u32X) = 0;
    }
    *(pCfg->u32Y) += *(pCfg->u32H);
}
void UT_Walk_Move_NextCol(UTestRamdomWalker_t *pCfg,bool bReverse)
{
    if(bReverse){
        *(pCfg->u32Y) = *(pCfg->u32MaxH)-*(pCfg->u32H);
    }else{
        *(pCfg->u32Y) = 0;
    }
    *(pCfg->u32X) += *(pCfg->u32W);
}
void UT_Walk_Move_ClockwiseOneStep(s32 s32Offset,u32 *u32P)
{
    s32 s32P;
    s32P = *u32P;
    if(s32P+s32Offset > 0)
    {
        *u32P+= s32Offset;
    }
    else
    {
        *u32P = 0;
    }
}
void UT_Walk_SetWinInfo(UTestRamdomWalker_t *pstCfg,u32 *Id,u32 *winId,u32 *winType)
{
    pstCfg->u32winType = *winType;
    pstCfg->u32Id = *Id;
    pstCfg->u32winId = *winId;
}
void UT_Walk_GetInfo(UTestRamdomWalker_t *pstCfg)
{
    pstCfg->u32W = pstCfg->u32winType ? &gu32BwinW[pstCfg->u32Id][pstCfg->u32winId] : &gu32GwinW[pstCfg->u32Id][pstCfg->u32winId];
    pstCfg->u32H = pstCfg->u32winType ? &gu32BwinH[pstCfg->u32Id][pstCfg->u32winId] : &gu32GwinH[pstCfg->u32Id][pstCfg->u32winId];
    pstCfg->u32X = pstCfg->u32winType ? &gu32BwinX[pstCfg->u32Id][pstCfg->u32winId] : &gu32GwinX[pstCfg->u32Id][pstCfg->u32winId];
    pstCfg->u32Y = pstCfg->u32winType ? &gu32BwinY[pstCfg->u32Id][pstCfg->u32winId] : &gu32GwinY[pstCfg->u32Id][pstCfg->u32winId];
    pstCfg->u32MaxW = &gu32StrWidth[pstCfg->u32Id];
    pstCfg->u32MaxH = &gu32StrHeight[pstCfg->u32Id];
    pstCfg->bXRev = pstCfg->u32winType ? &bwinXRev[pstCfg->u32Id][pstCfg->u32winId] : &gwinXRev[pstCfg->u32Id][pstCfg->u32winId];
    pstCfg->bYRev = pstCfg->u32winType ? &bwinYRev[pstCfg->u32Id][pstCfg->u32winId] : &gwinYRev[pstCfg->u32Id][pstCfg->u32winId];
}
bool UT_Walk_InDisplayWinUnderCorner(UTestRamdomWalker_t *pCfg,bool bLeft)
{
    u32 u32ValX = 0;
    u32 u32ValY = 0;
    if(bLeft == RGN_RL || bLeft == RGN_RU)
    {
        u32ValX = (*(pCfg->u32MaxW)-*(pCfg->u32W));
    }
    if(bLeft == RGN_RL || bLeft == RGN_LL)
    {
        u32ValY = (*(pCfg->u32MaxH)-*(pCfg->u32H));
    }
    if((*(pCfg->u32X) == u32ValX)&&(*(pCfg->u32Y) == u32ValY))
    {
        return 1;
    }
    return 0;
}
bool UT_Walk_InDisplayWinRLSide(UTestRamdomWalker_t *pCfg,bool bLeft)
{
    u32 u32Val = 0;
    if(!bLeft)
    {
        u32Val = (*(pCfg->u32MaxW)-*(pCfg->u32W));
    }
    if(*(pCfg->u32X) == u32Val)
    {
        return 1;
    }
    return 0;
}
bool UT_Walk_InDisplayWinULSide(UTestRamdomWalker_t *pCfg,bool bUpper)
{
    u32 u32Val = 0;
    if(!bUpper)
    {
        u32Val = ((*(pCfg->u32MaxH)-*(pCfg->u32H)));
    }
    if(*(pCfg->u32Y) == u32Val)
    {
        return 1;
    }
    return 0;
}
void UT_Walk_Move_RightDirect(UTestRamdomWalker_t *pCfg,bool bReverse)
{
    u8 idx;
    for(idx=0;idx<4;idx++)
    {
        if(UT_Walk_InDisplayWinUnderCorner(pCfg,idx))
        {
            if(idx==RGN_LU)
            {
                *pCfg->bXRev = 0;
                *pCfg->bYRev = 0;
            }
            if(idx==RGN_RU)
            {
                *pCfg->bXRev = 1;
                *pCfg->bYRev = 0;
            }
            if(idx==RGN_LL)
            {
                *pCfg->bXRev = 0;
                *pCfg->bYRev = 1;
            }
            if(idx==RGN_RL)
            {
                *pCfg->bXRev = 1;
                *pCfg->bYRev = 1;
            }
            break;
        }
    }

}
void _UT_Walk_H_Move_Straight(UTestRamdomWalker_t *pstCfg,bool bReverse)
{
    s32 s32Val = bReverse ? Default_WIN_Shift*(-1): Default_WIN_Shift;
    if(UT_Walk_InDisplayWinUnderCorner(pstCfg,bReverse ? RGN_LL : RGN_RL))
    {
        UT_Walk_Move_Original(pstCfg,bReverse ? RGN_RU : RGN_LU);
    }
    else if(UT_Walk_InDisplayWinRLSide(pstCfg,bReverse))
    {
        UT_Walk_Move_NextLine(pstCfg,bReverse);
    }
    else
    {
        UT_Walk_Move_ClockwiseOneStep(s32Val,(pstCfg->u32X));
    }
    UT_Walk_Move_Protect(pstCfg,1);
}
void UT_Walk_H_Move_Straight(u32 *Id,u32 *winId,u32 *winType)
{
    UTestRamdomWalker_t stCfg;
    UT_Walk_SetWinInfo(&stCfg,Id,winId,winType);
    UT_Walk_GetInfo(&stCfg);
    _UT_Walk_H_Move_Straight(&stCfg,*(stCfg.bXRev));
}
void _UT_Walk_V_Move_Straight(UTestRamdomWalker_t *pstCfg,bool bReverse)
{
    s32 s32Val = bReverse ? Default_WIN_Shift*(-1): Default_WIN_Shift;
    if(UT_Walk_InDisplayWinUnderCorner(pstCfg,bReverse ? RGN_RU : RGN_RL))
    {
        UT_Walk_Move_Original(pstCfg,bReverse ? RGN_LL : RGN_LU);
    }
    else if(UT_Walk_InDisplayWinULSide(pstCfg,bReverse))
    {
        UT_Walk_Move_NextCol(pstCfg,bReverse);
    }
    else
    {
        UT_Walk_Move_ClockwiseOneStep(s32Val,(pstCfg->u32Y));
    }
    UT_Walk_Move_Protect(pstCfg,1);
}
void UT_Walk_V_Move_Straight(u32 *Id,u32 *winId,u32 *winType)
{
    UTestRamdomWalker_t stCfg;
    UT_Walk_SetWinInfo(&stCfg,Id,winId,winType);
    UT_Walk_GetInfo(&stCfg);
    _UT_Walk_V_Move_Straight(&stCfg,*(stCfg.bYRev));
}
void _UT_Walk_Move_Cross(UTestRamdomWalker_t *pstCfg)
{
    s32 s32ValX;
    s32 s32ValY;
    if(UT_Walk_InDisplayWinRLSide(pstCfg,*(pstCfg->bXRev)))
    {
        *(pstCfg->bXRev) ^= 1;
    }
    if(UT_Walk_InDisplayWinULSide(pstCfg,*(pstCfg->bYRev)))
    {
        *(pstCfg->bYRev) ^= 1;
    }
    s32ValX = *(pstCfg->bXRev) ? Default_WIN_Shift*(-1): Default_WIN_Shift;
    s32ValY = *(pstCfg->bYRev) ? Default_WIN_Shift*(-1): Default_WIN_Shift;
    UT_Walk_Move_ClockwiseOneStep(s32ValY,(pstCfg->u32Y));
    UT_Walk_Move_ClockwiseOneStep(s32ValX,(pstCfg->u32X));
    UT_Walk_Move_Protect(pstCfg,1);
}
void UT_Walk_Move_Cross(u32 *Id,u32 *winId,u32 *winType)
{
    UTestRamdomWalker_t stCfg;
    UT_Walk_SetWinInfo(&stCfg,Id,winId,winType);
    UT_Walk_GetInfo(&stCfg);
    _UT_Walk_Move_Cross(&stCfg);
}
void _UT_Walk_Move_Circle(UTestRamdomWalker_t *pstCfg,bool bReverse)
{
    s32 s32Val;
    u32 *u32ValOri;
    u8 *u8bRev;
    u32ValOri = (pstCfg->u32Y);
    u8bRev = (pstCfg->bYRev);
    if((UT_Walk_InDisplayWinRLSide(pstCfg,RGN_RIGHT)&&(*(pstCfg->bYRev)==bReverse)) ||
    (UT_Walk_InDisplayWinRLSide(pstCfg,RGN_LEFT)&&(*(pstCfg->bYRev)!=bReverse)))
    {
        u32ValOri = (pstCfg->u32Y);
        u8bRev = (pstCfg->bYRev);
    }
    if((UT_Walk_InDisplayWinULSide(pstCfg,RGN_UPPER)&&(*(pstCfg->bXRev)==bReverse)) ||
    (UT_Walk_InDisplayWinULSide(pstCfg,RGN_LOWER)&&(*(pstCfg->bXRev)!=bReverse)))
    {
        u32ValOri = (pstCfg->u32X);
        u8bRev = (pstCfg->bXRev);
    }
    s32Val = *u8bRev ? Default_WIN_Shift*(-1): Default_WIN_Shift;
    UT_Walk_Move_ClockwiseOneStep(s32Val,u32ValOri);
    UT_Walk_Move_Protect(pstCfg,1);
    UT_Walk_Move_RightDirect(pstCfg,bReverse);
}
void UT_Walk_Move_Circle(u32 *Id,u32 *winId,u32 *winType)
{
    UTestRamdomWalker_t stCfg;
    UT_Walk_SetWinInfo(&stCfg,Id,winId,winType);
    UT_Walk_GetInfo(&stCfg);
    _UT_Walk_Move_Circle(&stCfg,RGN_CLOCKWISE);
}
void UT_Walk_Move_Counterclockwise(u32 *Id,u32 *winId,u32 *winType)
{
    UTestRamdomWalker_t stCfg;
    UT_Walk_SetWinInfo(&stCfg,Id,winId,winType);
    UT_Walk_GetInfo(&stCfg);
    _UT_Walk_Move_Circle(&stCfg,RGN_CLOCKWISE_REVERSE);
}
void _UT_Walk_Move_Scale(UTestRamdomWalker_t *pstCfg,bool bReverse)
{
    s32 s32ValW;
    s32 s32ValH;
    if(UT_Walk_InDisplayWinRLSide(pstCfg,RGN_RIGHT)||*(pstCfg->u32W)<=10)
    {
        *(pstCfg->bXRev) ^= 1;
    }
    if(UT_Walk_InDisplayWinULSide(pstCfg,RGN_LOWER)||*(pstCfg->u32H)<=10)
    {
        *(pstCfg->bYRev) ^= 1;
    }
    s32ValW = *(pstCfg->bXRev) ? Default_WIN_Shift*(-1): Default_WIN_Shift;
    s32ValH = *(pstCfg->bYRev) ? Default_WIN_Shift*(-1): Default_WIN_Shift;
    UT_Walk_Move_ClockwiseOneStep(s32ValW,(pstCfg->u32W));
    UT_Walk_Move_ClockwiseOneStep(s32ValH,(pstCfg->u32H));
    UT_Walk_Move_Protect(pstCfg,0);
}
void UT_Walk_Move_Scale(u32 *Id,u32 *winId,u32 *winType)
{
    UTestRamdomWalker_t stCfg;
    UT_Walk_SetWinInfo(&stCfg,Id,winId,winType);
    UT_Walk_GetInfo(&stCfg);
    _UT_Walk_Move_Scale(&stCfg,RGN_CLOCKWISE);
}
void UT_Walk_Move_Scroll(u32 *Id,u32 *winId,u32 *winType)
{
    gu16Xoffset[*Id][*winId]++;
    if(gu16Xoffset[*Id][*winId]>(BufferStride-gu32GwinW[*Id][*winId]))
    {
        gu16Xoffset[*Id][*winId] = 0;
    }
    MHAL_RGN_GopGwinSetBufferXoffset(*Id,*winId,(u16)gu16Xoffset[*Id][*winId]);
}
void *UT_Switch_WalkType(HalRgnUTWalkType_e enType)
{
    void  (*pfForSet)(u32 *,u32 *,u32 *);
    switch(enType)
    {
        case E_RGN_UT_WALK_ID_0:
        case E_RGN_UT_WALK_ID_4:
            pfForSet = UT_Walk_H_Move_Straight;
            break;
        case E_RGN_UT_WALK_ID_1:
        case E_RGN_UT_WALK_ID_5:
            pfForSet = UT_Walk_V_Move_Straight;
            break;
        case E_RGN_UT_WALK_ID_2:
            pfForSet = UT_Walk_Move_Cross;
            break;
        case E_RGN_UT_WALK_ID_3:
            pfForSet = UT_Walk_Move_Circle;
            break;
        case E_RGN_UT_WALK_ID_7:
            pfForSet = UT_Walk_Move_Counterclockwise;
            break;
        case E_RGN_UT_WALK_ID_6:
        case E_RGN_UT_WALK_ID_8:
            pfForSet = UT_Walk_Move_Scale;
            break;
        case E_RGN_UT_WALK_ID_9:
            pfForSet = UT_Walk_Move_Scroll;
            break;
        default:
            pfForSet = NULL;
            break;
    }
    return pfForSet;
}
void UT_Set_Reverse(MHAL_RGN_GopType_e enGop,int gwinid,HalRgnUTWalkType_e enType,u32 u32WinType)
{
    u8 *u8RevX;
    u8 *u8RevY;
    u8RevX = u32WinType ? &bwinXRev[enGop][gwinid] : &gwinXRev[enGop][gwinid];
    u8RevY = u32WinType ? &bwinYRev[enGop][gwinid] : &gwinYRev[enGop][gwinid];
    switch(enType)
    {
        case E_RGN_UT_WALK_ID_0:
            *u8RevX = 0;
            break;
        case E_RGN_UT_WALK_ID_4:
            *u8RevX = 1;
            break;
        case E_RGN_UT_WALK_ID_1:
            *u8RevY = 0;
            break;
        case E_RGN_UT_WALK_ID_5:
            *u8RevY = 1;
            break;
        case E_RGN_UT_WALK_ID_3:
            *u8RevX = 0;
            *u8RevY = 0;
            break;
        case E_RGN_UT_WALK_ID_7:
            *u8RevX = 0;
            *u8RevY = 0;
            break;
        case E_RGN_UT_WALK_ID_2:
            *u8RevX = 0;
            *u8RevY = 0;
            break;
        case E_RGN_UT_WALK_ID_8:
            *u8RevX = 0;
            *u8RevY = 0;
            break;
        default:
            break;
    }
}
void UT_GwinRamdomWalk(MHAL_RGN_GopType_e enGop,int gwinid,HalRgnUTWalkType_e enType,char *port)
{
    void  (*pfForSet)(u32 *,u32 *,u32 *);
    char str[64];
    int u32WinType = 0;
    pfForSet = UT_Switch_WalkType(enType);
    if(pfForSet)
    {
        pfForSet(&enGop,&gwinid,&u32WinType);
        DrvRgnScnprintf(str,64,"%s %d %hhd %d %d %d %d"
            ,port,gwinid,gu8GwinEn[enGop][gwinid],
            gu32GwinX[enGop][gwinid],gu32GwinY[enGop][gwinid],gu32GwinW[enGop][gwinid],gu32GwinH[enGop][gwinid]);
        Rgn_UT_CaseG(str,0);
    }
}
void UT_BwinRamdomWalk(MHAL_RGN_CoverType_e enCov,int bwinid,HalRgnUTWalkType_e enType,char *port)
{
    void  (*pfForSet)(void *,void *,void *);
    char str[64];
    int u32WinType = 1;
    pfForSet = UT_Switch_WalkType(enType);
    if(pfForSet)
    {
        pfForSet(&enCov,&bwinid,&u32WinType);
        DrvRgnScnprintf(str,64,"%s %d %hhd %d %d %d %d"
            ,port,bwinid,gu8BwinEn[enCov][bwinid],
            gu32BwinX[enCov][bwinid],gu32BwinY[enCov][bwinid],gu32BwinW[enCov][bwinid],gu32BwinH[enCov][bwinid]);
        Rgn_UT_CaseB(str,0);
    }
}
void *Rgn_UT_Case5(void *buf)
{
    char walktype[Default_GWIN_MAX][8];
    char port[8];
    int u32gwin = 0;
    const char *str = buf;
    int idx;
    MHAL_RGN_GopType_e enGop;
    HalRgnUTWalkType_e enType[Default_GWIN_MAX];
    //"Test Case 5 : BY Port Test GWIN Ramdom Walk, echo 5(PORT_ID) (GWINNUM) (walk type 0123) > test\n"
    sscanf(buf, "%s", port);
    str+= strlen(port);
    sscanf(str, "%d %s %s %s %s",&u32gwin,walktype[0],walktype[1],walktype[2],walktype[3]);
    DRVRGNERR("RGN UT 5 walktype:%s walktype:%s walktype:%s walktype:%s num:%d %s @%x\n",walktype[0],walktype[1],walktype[2],walktype[3],u32gwin,port,(MS_U32)buf);
    for(idx=0;idx<u32gwin;idx++)
    {
        enType[idx] = UT_GET_WALK_ID_FROM_STR(walktype[idx]);
    }
    enGop = UT_GET_GOP_ID_FROM_PORT(port);
    for(idx=0;idx<u32gwin;idx++)
    {
        UT_Set_Reverse(enGop,idx,enType[idx],0);
    }
    while(gstUtGopTaskCfg.bEn)
    {
        while(gstUtGopTaskCfg.bpause)
        {
            CamOsMsSleep(50);
        }
        for(idx=0;idx<u32gwin;idx++)
        {
            UT_GwinRamdomWalk(enGop,idx,enType[idx],port);
        }
        gstUtGopTaskCfg.bSet = 1;
    #if AUTOTRIGGERDIP
        if(enGop==E_MHAL_GOP_DIVP_PORT0)
        {
            HalRgnWrite2ByteMsk(0x123BF8,0x8000,0x8000,1);
        }
    #endif
        CamOsMsSleep(50);
    }
    return NULL;
}
void *Rgn_UT_Case6(void *buf)
{
    char walktype[Default_GWIN_MAX][8];
    char port[8];
    int u32bwin = 0;
    const char *str = buf;
    MHAL_RGN_CoverType_e enCov;
    HalRgnUTWalkType_e enType[Default_GWIN_MAX];
    int idx;
    //"Test Case 6 : BY Port Test BWIN Ramdom Walk, echo 6(PORT_ID) (BWINNUM) (walk type 0123) > test\n"
    sscanf(str, "%s", port);
    str+= strlen(port);
    sscanf(str, "%d %s %s %s %s",&u32bwin,walktype[0],walktype[1],walktype[2],walktype[3]);
    DRVRGNERR("RGN UT 6 walktype:%s walktype:%s walktype:%s walktype:%s num:%d %s\n",walktype[0],walktype[1],walktype[2],walktype[3],u32bwin,port);
    enCov = UT_GET_GOP_ID_FROM_PORT(port);
    for(idx=0;idx<u32bwin;idx++)
    {
        enType[idx] = UT_GET_WALK_ID_FROM_STR(walktype[idx]);
    }
    for(idx=0;idx<u32bwin;idx++)
    {
        UT_Set_Reverse(enCov,idx,enType[idx],1);
    }
    while(gstUtCovTaskCfg.bEn)
    {
        while(gstUtCovTaskCfg.bpause)
        {
            CamOsMsSleep(50);
        }
        for(idx=0;idx<u32bwin;idx++)
        {
            UT_BwinRamdomWalk(enCov,idx,enType[idx],port);
        }
        gstUtCovTaskCfg.bSet = 1;
        CamOsMsSleep(50);
    }
    return NULL;
}
void Rgn_UT_Create_Task(void *pf,void *buf,UTestTask_t *pCfg)
{
    CamOsThreadAttrb_t stCfg;
    stCfg.nPriority = 50;
    stCfg.nStackSize = 0;
    pCfg->bEn = 1;
    pCfg->bSet = 0;
    if (CamOsThreadCreate(&pCfg->Id,&stCfg,pf,buf))
    {
        DRVRGNERR("[UT]%s create task fail\n", __FUNCTION__);
        return;
    }
}
void Rgn_UT_CmdParser(const char *buf, u32 n)
{
    char cmd;
    int u32size = 0;
    const char *str = buf;
    //1.choose cmd
    //2.parser buf
    u32size +=sscanf(str, "%c", &cmd);
    str+=(u32size+1);
    DRVRGNERR("RGN UT CMD:%c u32size:%d n:%d @%x\n", cmd,u32size,n,(u32)str);
    switch(cmd)
    {
        case '0':
            Rgn_UT_Case0();
            break;
        case '1':
            Rgn_UT_Case1();
            break;
        case '2':
            Rgn_UT_Case2();
            break;
        case '3':
            Rgn_UT_Case3(str,n-(u32size+1));
            break;
        case '4':
            Rgn_UT_Case4(str,n-(u32size+1));
            break;
        case '5':
            Rgn_UT_Create_Task(Rgn_UT_Case5,(void *)str,&gstUtGopTaskCfg);
            while(!gstUtGopTaskCfg.bSet)
            {
                CamOsMsSleep(1);
            }
            break;
        case '6':
            Rgn_UT_Create_Task(Rgn_UT_Case6,(void *)str,&gstUtCovTaskCfg);
            while(!gstUtCovTaskCfg.bSet)
            {
                CamOsMsSleep(1);
            }
            break;
        case '7':
            gstUtGopTaskCfg.bpause ^= 1;
            break;
        case '8':
            gstUtCovTaskCfg.bpause ^= 1;
            break;
        case 'G':
            Rgn_UT_CaseG(str,n-(u32size+1));
            break;
        case 'B':
            Rgn_UT_CaseB(str,n-(u32size+1));
            break;
        case 'C':
            Rgn_UT_CaseC(str,n-(u32size+1));
            break;
        case 'A':
            Rgn_UT_CaseA(str,n-(u32size+1));
            break;
        case 'V':
            Rgn_UT_CaseV(str,n-(u32size+1));
            break;
        case 'F':
            Rgn_UT_CaseF(str,n-(u32size+1));
            break;
        case 'M':
            Rgn_UT_CaseM(str,n-(u32size+1));
            break;
        case 'S':
            Rgn_UT_CaseS(str,n-(u32size+1));
            break;
        case 'X':
            Rgn_UT_CaseX(str,n-(u32size+1));
            break;
        default:
            DRVRGNERR("RGN UT CMD NOT SUPPORT:%c\n", cmd);
            break;
    }
}
u32 RgnTestShow(char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    str += DrvRgnScnprintf(str, end - str, "=====================RGN Test====================\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case 1 : Mhal gop API, echo 1 > test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case 2 : Mhal cover API, echo 2 > test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case 3 : BY Port Test GWIN open 4 place, echo 3 (PORT_ID)(Fmt)(GWINNUM)[w h] > test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case 4 : BY Port Test BWIN open 4 place, echo 4 (PORT_ID) (BWINNUM)[w h] > test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case 5 : BY Port Test GWIN Ramdom Walk, echo 5 (PORT_ID)(GWINNUM) (walk type 0123) > test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case 6 : BY Port Test BWIN Ramdom Walk, echo 6 (PORT_ID)(BWINNUM) (walk type 0123) > test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case G : BY Port Test GWIN, echo 3 (PORT_ID) (GWINID) (en,x,y,w,h) > test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case B : BY Port Test BWIN, echo 4 (PORT_ID) (BWINID) (en,x,y,w,h) > test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case C : Change Color Key, echo 7 (PORT_ID) (en,r,g,b) > test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case A : Change Alpha, echo 8 (PORT_ID) (GWINID) (type,con,a0,a1) > test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case V : Change BWIN Value, echo 9 (PORT_ID) (BWIN) (r,g,b) > test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case F : Change Fmt WIN, echo F (PORT_ID) (Fmt)> test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case M : Dump Mem to Dram, echo M (PATH)(size)(stride)> test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case S : Change Str WIN, echo S (PORT_ID) (w,h)> test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case X : Change XOffset, echo X (PORT_ID) (GWINID) val > test\n");
    str += DrvRgnScnprintf(str, end - str, "Test Case 0 : Disable all,\n");
    str += DrvRgnScnprintf(str, end - str, "---------------------WALK TYPE--------------------\n");
    str += DrvRgnScnprintf(str, end - str, "0:H Move Straight\n");
    str += DrvRgnScnprintf(str, end - str, "1:V Move Straight\n");
    str += DrvRgnScnprintf(str, end - str, "2:Move Cross without scale\n");
    str += DrvRgnScnprintf(str, end - str, "3:Move Clockwise Circle\n");
    str += DrvRgnScnprintf(str, end - str, "4:H Move Straight Reverse\n");
    str += DrvRgnScnprintf(str, end - str, "5:V Move Straight Reverse\n");
    str += DrvRgnScnprintf(str, end - str, "6:Move Cross with scale\n");
    str += DrvRgnScnprintf(str, end - str, "7:Move Counterclockwise Circle\n");
    str += DrvRgnScnprintf(str, end - str, "8:Scale\n");
    str += DrvRgnScnprintf(str, end - str, "9:Scrolling text\n");
    str += DrvRgnScnprintf(str, end - str, "=====================RGN Test====================\n");
    end = end;
    return (str - buf);
}
void Rgn_UT_init(void)
{
    MHAL_RGN_GopType_e enGop;
    MHAL_RGN_GopGwinId_e enGwin;
    MHAL_RGN_CoverType_e enId;
    MHAL_RGN_CoverLayerId_e enBwin;
    //int i;
    //char str[64];

    gstUtGopTaskCfg.Id = NULL;
    gstUtCovTaskCfg.Id = NULL;
    for(enId=E_MHAL_COVER_VPE_PORT0;enId<E_MHAL_COVER_TYPE_MAX;enId++)
    {
        for(enBwin=E_MHAL_COVER_LAYER_0;enBwin<E_MHAL_COVER_LAYER_MAX;enBwin++)
        {
            gu32BwinColor[enId][enBwin] = COVER_COLOR[enBwin];
            gu8BwinEn[enId][enBwin] = 0;
            gu32BwinX[enId][enBwin] = 0;
            gu32BwinY[enId][enBwin] = 0;
            gu32BwinW[enId][enBwin] = 0;
            gu32BwinH[enId][enBwin] = 0;
        }
    }
    for(enGop=E_MHAL_GOP_VPE_PORT0;enGop<E_MHAL_GOP_TYPE_MAX;enGop++)
    {
        gu8ColorKeyR[enGop] = 0;
        gu8ColorKeyG[enGop] = 0;
        gu8ColorKeyB[enGop] = 0;
        gu8ColorKeyEn[enGop] = 0;
        gu32StrWidth[enGop] = Default_Str_Width;
        gu32StrHeight[enGop] = Default_Str_Height;
        genFmt[enGop] = E_MHAL_RGN_PIXEL_FORMAT_I8;
        _RGN_GopSetPalette(enGop,E_MHAL_RGN_PIXEL_FORMAT_I8);
        for(enGwin=E_MHAL_GOP_GWIN_ID_0;enGwin<E_MHAL_GOP_GWIN_ID_MAX;enGwin++)
        {
            gu8GwinEn[enGop][enGwin] = 0;
            gu32GwinX[enGop][enGwin] = 0;
            gu32GwinY[enGop][enGwin] = 0;
            gu32GwinW[enGop][enGwin] = 0;
            gu32GwinH[enGop][enGwin] = 0;
            genAlphaType[enGop][enGwin] = E_MHAL_GOP_GWIN_ALPHA_CONSTANT;
            gu32Alpha0[enGop][enGwin] = 0;
            gu32Alpha1[enGop][enGwin] = 0xFF;
            gu16Xoffset[enGop][enGwin] = 0;
        }
    }
    MHAL_RGN_GopInit();
    MHAL_RGN_CoverInit();
    // create buffer for SCL
    /*
    for(i=0;i<OUT_BUF_NUM;i++) {
        DrvRgnScnprintf(str,64,"RGN_OUT_BUF_%d",i);
        CamOsDirectMemAlloc(str, outbuffersize, &(outBuf[i].pvOutBufVirt), &(outBuf[i].pvOutBufPhys), &(outBuf[i].pvOutBufMiu));
    }*/
}
void Rgn_UT_deInit(void)
{
    //int i;
    /*
    for(i=0;i<OUT_BUF_NUM;i++) {
        if(outBuf[i].pvOutBufVirt)
        {
            CamOsDirectMemRelease(outBuf[i].pvOutBufVirt,outbuffersize);
            outBuf[i].pvOutBufVirt = NULL;
        }
    }*/
    //MHAL_RGN_GopDeinit();
    //MHAL_RGN_CoverDeinit();
}
