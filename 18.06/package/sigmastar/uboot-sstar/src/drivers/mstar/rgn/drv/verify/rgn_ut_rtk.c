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

#include "cam_os_wrapper.h"
#include "mhal_common.h"
#include "sys_sys_isw_cli.h"
#include "mhal_rgn.h"
#include "rgn_capdef.h"

#include <stdarg.h>

#define RGN_SCL_NUM 3
#define RGN_DEF_BASE_W 800
#define RGN_DEF_BASE_H 480
#define RGN_DEF_BUF_SIZE RGN_DEF_BASE_W*RGN_DEF_BASE_H*2
#define RGN_FMT_MAX 5
#define RGN_PARSE_ERROR {\
    ret = eCLI_PARSE_INPUT_ERROR;\
    goto RGN_TEST_EXIT;\
}
#define RGN_PARSE(args...) {if(_RgnParsing(args)!=eCLI_PARSE_OK) { RGN_PARSE_ERROR; }}
#define RGN_PIX2BYTE(x, fmt) (fmt==E_MHAL_RGN_PIXEL_FORMAT_ARGB1555)||\
                             (fmt==E_MHAL_RGN_PIXEL_FORMAT_ARGB4444)?(x)*2:\
                             (fmt==      E_MHAL_RGN_PIXEL_FORMAT_I2)?(x)/4:\
                             (fmt==      E_MHAL_RGN_PIXEL_FORMAT_I4)?(x)/2:\
                             (fmt==      E_MHAL_RGN_PIXEL_FORMAT_I8)?(x)*1:(x)
#define RGN_FMT2STR(fmt) (fmt==E_MHAL_RGN_PIXEL_FORMAT_ARGB1555)?"ARGB1555":\
                         (fmt==E_MHAL_RGN_PIXEL_FORMAT_ARGB4444)?"ARGB4444":\
                         (fmt==      E_MHAL_RGN_PIXEL_FORMAT_I2)?"I2":\
                         (fmt==      E_MHAL_RGN_PIXEL_FORMAT_I4)?"I4":\
                         (fmt==      E_MHAL_RGN_PIXEL_FORMAT_I8)?"I8":"N/A"
#define RGN_PORT2STR(port) (port==E_MHAL_GOP_VPE_PORT0) ?"SCL0":\
                           (port==E_MHAL_GOP_VPE_PORT1) ?"SCL1":\
                           (port==E_MHAL_GOP_VPE_PORT2) ?"SCL2":\
                           (port==E_MHAL_GOP_VPE_PORT3) ?"SCL3":\
                           (port==E_MHAL_GOP_DIVP_PORT0)?"DIVP":"N/A"
#define RGN_ATYPE2STR(type) (type==E_MHAL_GOP_GWIN_ALPHA_CONSTANT) ?"Constant":\
                            (type==    E_MHAL_GOP_GWIN_ALPHA_PIXEL)?"Pixel":"N/A"
#define RGN_SET_CORNERS(pos,baseW,baseH,winW,winH) {\
    pos[0][0]=pos[0][1]=pos[1][1]=pos[2][0]=0;\
    pos[1][0]=pos[3][0]=baseW-winW;\
    pos[2][1]=pos[3][1]=baseH-winH;\
}
#define RGN_SET_ALL_GOP_ATTR(gopId,gwinId,eFmt,bCKeyEn,u32CKeyRGB,u32StrX,u32StrY,u32StrW,u32StrH,eAType,u32AValue,u8Alpha0,u8Alpha1,\
                             bGwinEn,u32GwinX,u32GwinY,u32GwinW,u32GwinH,u32GwinBufXOft) {\
    for(gopId=0;gopId<E_MHAL_GOP_TYPE_MAX;gopId++) {\
        RgnGopSetGopAttr(gopId,eFmt,bCKeyEn,u32CKeyRGB,u32StrX,u32StrY,u32StrW,u32StrH,eAType,u32AValue,u8Alpha0,u8Alpha1);\
        for(gwinId=0;gwinId<E_MHAL_GOP_GWIN_ID_MAX;gwinId++) {\
            RgnGopSetGwinAttr(gopId,gwinId,bGwinEn,u32GwinX,u32GwinY,u32GwinW,u32GwinH,u32GwinBufXOft);\
        }\
    }\
}
#define RGN_SET_ALL_COVER_ATTR(coverId,bwinId,bBwinEn,u32X,u32Y,u32W,u32H,u32Color) {\
    for(coverId=0;coverId<E_MHAL_COVER_TYPE_MAX;coverId++) {\
        for(bwinId=0;bwinId<E_MHAL_COVER_LAYER_MAX;bwinId++) {\
            RgnCoverSetBwinAttr(coverId,bwinId,bBwinEn,u32X,u32Y,u32W,u32H,u32Color);\
        }\
    }\
}

typedef struct
{
    void *pvBufVirt;
    void *pvBufPhys;
    void *pvBufMiu;
    MS_U32 u32BufSize;
} RgnBufInfo_t;
typedef struct
{
    MS_U32 u32X;
    MS_U32 u32Y;
    MS_U32 u32W;
    MS_U32 u32H;
} RgnWin_t;
typedef struct
{
    MHAL_RGN_GopPixelFormat_e eFmt;
    MS_BOOL  bCKeyEn;
    MS_U32   u32CKeyRGB;
    RgnWin_t stStrWin;
    MHAL_RGN_GopGwinAlphaType_e eAType;
    MS_U32   u32AValue;
    MS_U8    u8Alpha0;
    MS_U8    u8Alpha1;
    MS_BOOL  bGwinEn[E_MHAL_GOP_GWIN_ID_MAX];
    RgnWin_t stGwin[E_MHAL_GOP_GWIN_ID_MAX];
    MS_U32   u32GwinBufXOft[E_MHAL_GOP_GWIN_ID_MAX];
} RgnGopInfo_t;
typedef struct
{
    MS_BOOL  bBwinEn[E_MHAL_COVER_LAYER_MAX];
    RgnWin_t stBwin[E_MHAL_COVER_LAYER_MAX];
    MS_U32   u32Color[E_MHAL_COVER_LAYER_MAX];
} RgnCoverInfo_t;
typedef struct
{
    RgnBufInfo_t   stOsdBuf[RGN_FMT_MAX];
    RgnBufInfo_t   stSclBuf[RGN_SCL_NUM];
    RgnGopInfo_t   stGop[E_MHAL_GOP_TYPE_MAX];
    RgnCoverInfo_t stCover[E_MHAL_COVER_TYPE_MAX];
} RgnTestInfo_t;

static RgnTestInfo_t _r;
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
static int RgnOsdInfo[RGN_FMT_MAX][2] = {{400,400},{400,400},{256,256},{256,256},{400,400}};
static MS_U32 RgnRGBWinYUV[4] = {0xFF4C54,0x15952B,0x6B1DFF,0x80FF80};
static int t_start, t_end;

extern unsigned long geGopOsdFlag;
extern void RgnUtSclScript(MS_U32 u32MiuAddr0, MS_U32 u32MiuAddr1, MS_U32 u32MiuAddr2)
;
int RgnTest(CLI_t *pCli, char *p);
int RgnHelp(CLI_t *pCli, char *p);
CliParseToken_t g_atRgnMenuTbl[] =
{
    {"test" , "test [case] [args...]", ">>>>> Enter 'h' for detail command <<<<<", RgnTest, NULL},
	{"h"    , "Show all command", "", RgnHelp, NULL},
    PARSE_TOKEN_DELIMITER
};

//---------------------------------------------------------------------------
// Sub Function
//---------------------------------------------------------------------------
int _RgnParsing(CLI_t *pCli, int numArgs, ...)
{
    MS_U32 value;
    va_list list;
    int i;

    va_start(list, numArgs);
    for(i=0;i<numArgs;i++)
    {
        if(CliTokenPopNum(pCli, &value, 10) != eCLI_PARSE_OK) { return eCLI_PARSE_INPUT_ERROR;}
        *(va_arg(list, int *)) = value;
    }
    va_end(list);
    return eCLI_PARSE_OK;
}
void _RgnAllocMem()
{
    int i;
    char str[64];
    // SCL Buffer
    for(i=0;i<RGN_SCL_NUM;i++)
    {
        CamOsSnprintf(str,64,"RGN_SCL_BUF_%d",i);
        if(_r.stSclBuf[i].pvBufVirt) {
            CamOsPrintf("Memory Released : %s\n",str);
            CamOsDirectMemRelease(_r.stSclBuf[i].pvBufVirt,_r.stSclBuf[i].u32BufSize);
        }
        _r.stSclBuf[i].u32BufSize = RGN_DEF_BUF_SIZE;
        CamOsDirectMemAlloc(str, _r.stSclBuf[i].u32BufSize, &(_r.stSclBuf[i].pvBufVirt), &(_r.stSclBuf[i].pvBufPhys), &(_r.stSclBuf[i].pvBufMiu));
        CamOsPrintf("Memory Allocated : %s\n",str);
    }
    // OSD Buffer
    for(i=0;i<RGN_FMT_MAX;i++)
    {
        CamOsSnprintf(str,64,"RGN_OSD_BUF_%d",i);
        if(_r.stOsdBuf[i].pvBufVirt) {
            CamOsPrintf("Memory Released : %s\n",str);
            CamOsDirectMemRelease(_r.stOsdBuf[i].pvBufVirt,_r.stOsdBuf[i].u32BufSize);
        }
        _r.stOsdBuf[i].u32BufSize = RGN_PIX2BYTE((RgnOsdInfo[i][0]*RgnOsdInfo[i][1]),i);
        CamOsDirectMemAlloc(str, _r.stOsdBuf[i].u32BufSize, &(_r.stOsdBuf[i].pvBufVirt), &(_r.stOsdBuf[i].pvBufPhys), &(_r.stOsdBuf[i].pvBufMiu));
        CamOsPrintf("Memory Allocated : %s\n",str);
    }

    // Call scl script
    RgnUtSclScript((MS_U32)(_r.stSclBuf[0].pvBufMiu),(MS_U32)(_r.stSclBuf[1].pvBufMiu),(MS_U32)(_r.stSclBuf[2].pvBufMiu));

    // Print info
    CamOsPrintf("++++++++++++++++++++++++++++++\n");
    for(i=0;i<RGN_FMT_MAX;i++)
    {
        CamOsSnprintf(str,64,"RGN_OSD_BUF_%d",i);
        CamOsPrintf("%s : %s\n",str,RGN_FMT2STR(i));
        CamOsPrintf("    Jlink : 0x%x\n",_r.stOsdBuf[i].pvBufPhys);
    }
    CamOsPrintf("------------------------------\n");
    for(i=0;i<RGN_SCL_NUM;i++)
    {
        CamOsSnprintf(str,64,"RGN_SCL_BUF_%d",i);
        CamOsPrintf("%s\n",str);
        CamOsPrintf("    Jlink : 0x%x\n",_r.stSclBuf[i].pvBufPhys);
    }
    CamOsPrintf("++++++++++++++++++++++++++++++\n");
}
void _RgnReleaseMem()
{
    int i;
    char str[64];
    for(i=0;i<RGN_FMT_MAX;i++)
    {
        CamOsSnprintf(str,64,"RGN_OSD_BUF_%d",i);
        if(_r.stOsdBuf[i].pvBufVirt) {
            CamOsPrintf("Memory Released : %s\n",str);
            CamOsDirectMemRelease(_r.stOsdBuf[i].pvBufVirt,_r.stOsdBuf[i].u32BufSize);
        }
    }
    for(i=0;i<RGN_SCL_NUM;i++)
    {
        CamOsSnprintf(str,64,"RGN_SCL_BUF_%d",i);
        if(_r.stSclBuf[i].pvBufVirt) {
            CamOsPrintf("Memory Released : %s\n",str);
            CamOsDirectMemRelease(_r.stSclBuf[i].pvBufVirt,_r.stSclBuf[i].u32BufSize);
        }
    }
}
int _RgnPrintf(const char *fmt, int align, ...)
{
    va_list list;
    int i;
    char str[64];
    int numArgs = strlen(fmt);
    va_start(list, align);
    for(i=0;i<numArgs;i++)
    {
        if(fmt[i]=='s') {
            CamOsSnprintf(str,64,"%%-%ds",align);
            CamOsPrintf(str,va_arg(list, char *));
        } else if(fmt[i]=='d') {
            CamOsSnprintf(str,64,"%%-%dd",align);
            CamOsPrintf(str,va_arg(list, int));
        } else if(fmt[i]=='x') {
            CamOsSnprintf(str,64,"0x%%-%dx",align-2);
            CamOsPrintf(str,va_arg(list, int));
        } else {
            CamOsPrintf("\nError printf format!!!\n");
            break;
        }
    }
    CamOsPrintf("\n");
    va_end(list);
    return eCLI_PARSE_OK;
}
//---------------------------------------------------------------------------
// Main Function
//---------------------------------------------------------------------------
void RgnGopSetPalette(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopPixelFormat_e eFormat)
{
    int i;
    switch(eFormat) {
        case E_MHAL_RGN_PIXEL_FORMAT_I2:
            CamOsPrintf("Set I2 Palette...\r\n");
            for(i=0;i<4;i++) {
                MHAL_RGN_GopSetPalette(eGopId, (MS_U8)(Palette_I2[i][1]>>8), (MS_U8)(Palette_I2[i][1]&0xFF),
                                               (MS_U8)(Palette_I2[i][0]>>8), (MS_U8)(Palette_I2[i][0]&0xFF), (MS_U8)i);
            }
            // end set
            MHAL_RGN_GopSetPalette(eGopId,0,0,0,0,255);
            break;
        case E_MHAL_RGN_PIXEL_FORMAT_I4:
            CamOsPrintf("Set I4 Palette...\r\n");
            for(i=0;i<16;i++) {
                MHAL_RGN_GopSetPalette(eGopId, (MS_U8)(Palette_I4[i][1]>>8), (MS_U8)(Palette_I4[i][1]&0xFF),
                                               (MS_U8)(Palette_I4[i][0]>>8), (MS_U8)(Palette_I4[i][0]&0xFF), (MS_U8)i);
            }
            // end set
            MHAL_RGN_GopSetPalette(eGopId,0,0,0,0,255);
            break;
        case E_MHAL_RGN_PIXEL_FORMAT_I8:
            CamOsPrintf("Set I8 Palette...\r\n");
            for(i=0;i<256;i++) {
                MHAL_RGN_GopSetPalette(eGopId, (MS_U8)(Palette_I8[i][1]>>8), (MS_U8)(Palette_I8[i][1]&0xFF),
                                               (MS_U8)(Palette_I8[i][0]>>8), (MS_U8)(Palette_I8[i][0]&0xFF), (MS_U8)i);
            }
            break;
        default:
            //CamOsPrintf("No Palette!Fmt=%d\r\n",eFormat);
            break;
    }
}
void RgnGopSetGopAttr(int gopId, MHAL_RGN_GopPixelFormat_e eFmt,
                      MS_BOOL bCKeyEn, MS_U32 u32CKeyRGB,
                      MS_U32 u32X, MS_U32 u32Y, MS_U32 u32W, MS_U32 u32H,
                      MHAL_RGN_GopGwinAlphaType_e eAType, MS_U32 u32AValue,
                      MS_U8 u8Alpha0, MS_U8 u8Alpha1)
{
    _r.stGop[gopId].eFmt          = eFmt;
    _r.stGop[gopId].bCKeyEn       = bCKeyEn;
    _r.stGop[gopId].u32CKeyRGB    = u32CKeyRGB;
    _r.stGop[gopId].stStrWin.u32X = u32X;
    _r.stGop[gopId].stStrWin.u32Y = u32Y;
    _r.stGop[gopId].stStrWin.u32W = u32W;
    _r.stGop[gopId].stStrWin.u32H = u32H;
    _r.stGop[gopId].eAType        = eAType;
    _r.stGop[gopId].u32AValue     = u32AValue;
    _r.stGop[gopId].u8Alpha0      = u8Alpha0;
    _r.stGop[gopId].u8Alpha1      = u8Alpha1;
}
void RgnGopSetGwinAttr(int gopId, int gwinId, MS_BOOL bGwinEn,
                       MS_U32 u32X, MS_U32 u32Y, MS_U32 u32W, MS_U32 u32H,
                       MS_U32 u32GwinBufXOft)
{
    _r.stGop[gopId].bGwinEn[gwinId]        = bGwinEn;
    _r.stGop[gopId].stGwin[gwinId].u32X    = u32X;
    _r.stGop[gopId].stGwin[gwinId].u32Y    = u32Y;
    _r.stGop[gopId].stGwin[gwinId].u32W    = u32W;
    _r.stGop[gopId].stGwin[gwinId].u32H    = u32H;
    _r.stGop[gopId].u32GwinBufXOft[gwinId] = u32GwinBufXOft;
}
void RgnCoverSetBwinAttr(int coverId, int bwinId, MS_BOOL bBwinEn,
                         MS_U32 u32X, MS_U32 u32Y, MS_U32 u32W, MS_U32 u32H,
                         MS_U32 u32Color)
{
    _r.stCover[coverId].bBwinEn[bwinId]        = bBwinEn;
    _r.stCover[coverId].stBwin[bwinId].u32X    = u32X;
    _r.stCover[coverId].stBwin[bwinId].u32Y    = u32Y;
    _r.stCover[coverId].stBwin[bwinId].u32W    = u32W;
    _r.stCover[coverId].stBwin[bwinId].u32H    = u32H;
    _r.stCover[coverId].u32Color[bwinId]       = u32Color;
}
void RgnGopProcess()
{
    int gopId, gwinId;
    t_start = DrvTimerGlobalTimerGetTick();
    MHAL_RGN_GopInit();
    for(gopId=0;gopId<E_MHAL_GOP_TYPE_MAX;gopId++)
    {
        if(gopId==E_MHAL_GOP_VPE_PORT3) {continue;}
        MHAL_RGN_GopSetBaseWindow(gopId,_r.stGop[gopId].stStrWin.u32W,_r.stGop[gopId].stStrWin.u32H);
        RgnGopSetPalette(gopId,_r.stGop[gopId].eFmt);
        MHAL_RGN_GopSetColorkey(gopId,_r.stGop[gopId].bCKeyEn,
                                      ((_r.stGop[gopId].u32CKeyRGB)>>16)&0xFF,
                                      ((_r.stGop[gopId].u32CKeyRGB)>>8)&0xFF,
                                      (_r.stGop[gopId].u32CKeyRGB)&0xFF);
        MHAL_RGN_GopSetAlphaZeroOpaque(gopId,1,1,_r.stGop[gopId].eFmt);
        for(gwinId=0;gwinId<E_MHAL_GOP_GWIN_ID_MAX;gwinId++)
        {
            MHAL_RGN_GopGwinSetPixelFormat(gopId,gwinId,_r.stGop[gopId].eFmt);
            MHAL_RGN_GopGwinSetWindow(gopId,gwinId,_r.stGop[gopId].stGwin[gwinId].u32W,
                                                   _r.stGop[gopId].stGwin[gwinId].u32H,
                                                   RGN_PIX2BYTE(RgnOsdInfo[_r.stGop[gopId].eFmt][0],_r.stGop[gopId].eFmt),
                                                   _r.stGop[gopId].stGwin[gwinId].u32X,
                                                   _r.stGop[gopId].stGwin[gwinId].u32Y);
            MHAL_RGN_GopGwinSetBufferXoffset(gopId,gwinId,_r.stGop[gopId].u32GwinBufXOft[gwinId]);
            MHAL_RGN_GopGwinSetBuffer(gopId,gwinId,(MS_U32)_r.stOsdBuf[_r.stGop[gopId].eFmt].pvBufMiu);
            MHAL_RGN_GopGwinEnable(gopId,gwinId);
            MHAL_RGN_GopSetAlphaType(gopId,gwinId,_r.stGop[gopId].eAType,_r.stGop[gopId].u32AValue);
            MHAL_RGN_GopSetArgb1555AlphaVal(gopId,gwinId,E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0,_r.stGop[gopId].u8Alpha0);
            MHAL_RGN_GopSetArgb1555AlphaVal(gopId,gwinId,E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1,_r.stGop[gopId].u8Alpha1);
        }
    }
    t_end = DrvTimerGlobalTimerGetTick();
    CamOsPrintf("[%s]Process time = %d us\n",__FUNCTION__,DrvTimerStdaTimerTick2Us(t_end-t_start));
}
void RgnCoverProcess()
{
    int coverId, bwinId;
    t_start = DrvTimerGlobalTimerGetTick();
    MHAL_RGN_CoverInit();
    for(coverId=0;coverId<E_MHAL_COVER_TYPE_MAX;coverId++)
    {
        if(coverId==E_MHAL_COVER_VPE_PORT3)
            continue;
        for(bwinId=0;bwinId<E_MHAL_COVER_LAYER_MAX;bwinId++)
        {
            MHAL_RGN_CoverSetColor(coverId,bwinId,_r.stCover[coverId].u32Color[bwinId]);
            MHAL_RGN_CoverSetWindow(coverId,bwinId,_r.stCover[coverId].stBwin[bwinId].u32X,
                                                   _r.stCover[coverId].stBwin[bwinId].u32Y,
                                                   _r.stCover[coverId].stBwin[bwinId].u32W,
                                                   _r.stCover[coverId].stBwin[bwinId].u32H);
            MHAL_RGN_CoverEnable(coverId,bwinId);
        }
    }
    t_end = DrvTimerGlobalTimerGetTick();
    CamOsPrintf("[%s]Process time = %d us\n",__FUNCTION__,DrvTimerStdaTimerTick2Us(t_end-t_start));
}

// Clear all
void RgnTestCase0()
{
    int devId, winId;
    for(devId=0;devId<E_MHAL_GOP_TYPE_MAX;devId++)
    {
        for(winId=0;winId<E_MHAL_GOP_GWIN_ID_MAX;winId++)
        {
            MHAL_RGN_GopGwinDisable(devId,winId);
        }
    }
    CamOsPrintf("All GOP disabled!\n");
    for(devId=0;devId<E_MHAL_COVER_TYPE_MAX;devId++)
    {
        for(winId=0;winId<E_MHAL_COVER_LAYER_MAX;winId++)
        {
            MHAL_RGN_CoverDisable(devId,winId);
        }
    }
    CamOsPrintf("All COVER disabled!\n");
    _RgnReleaseMem();
    memset(&_r,0,sizeof(RgnTestInfo_t));
}
// Create Buffer
void RgnTestCase1()
{
    _RgnAllocMem();
}
// GOP Bind OSD
void RgnTestCase2(int flag)
{
    MHAL_RGN_SetGopBindOsd(flag);
}
// GOP Gwin Size
void RgnTestCase10(int fmt, int gwinW, int gwinH)
{
    int gopId, gwinId;
    int gwinPos[4][2];
    RGN_SET_CORNERS(gwinPos,RGN_DEF_BASE_W,RGN_DEF_BASE_H,gwinW,gwinH);
    RGN_SET_ALL_GOP_ATTR(gopId,gwinId,fmt,0,0,0,0,RGN_DEF_BASE_W,RGN_DEF_BASE_H,0,0,0x0,0xFF,\
                         1,gwinPos[gwinId][0],gwinPos[gwinId][1],gwinW,gwinH,0);
    RgnGopProcess();
}
// GOP Color Key
void RgnTestCase11(int fmt, int cKeyEn, int cKeyRGB)
{
    int gopId, gwinId;
    int gwinPos[4][2];
    RGN_SET_CORNERS(gwinPos,RGN_DEF_BASE_W,RGN_DEF_BASE_H,200,200);
    RGN_SET_ALL_GOP_ATTR(gopId,gwinId,fmt,cKeyEn,cKeyRGB,0,0,RGN_DEF_BASE_W,RGN_DEF_BASE_H,0,0,0x0,0xFF,\
                         1,gwinPos[gwinId][0],gwinPos[gwinId][1],200,200,0);
    RgnGopProcess();
}
// GOP Alpha Type
void RgnTestCase12(int fmt, int aType, int alpha0, int alpha1)
{
    int gopId, gwinId;
    int gwinPos[4][2];
    RGN_SET_CORNERS(gwinPos,RGN_DEF_BASE_W,RGN_DEF_BASE_H,200,200);
    RGN_SET_ALL_GOP_ATTR(gopId,gwinId,fmt,0,0,0,0,RGN_DEF_BASE_W,RGN_DEF_BASE_H,aType,0,alpha0,alpha1,\
                         1,gwinPos[gwinId][0],gwinPos[gwinId][1],200,200,0);
    RgnGopProcess();
}
// GOP Gwin Buffer XOffset
void RgnTestCase13(int fmt, int xOffset)
{
    int gopId, gwinId;
    int gwinPos[4][2];
    RGN_SET_CORNERS(gwinPos,RGN_DEF_BASE_W,RGN_DEF_BASE_H,200,200);
    RGN_SET_ALL_GOP_ATTR(gopId,gwinId,fmt,0,0,0,0,RGN_DEF_BASE_W,RGN_DEF_BASE_H,0,0,0x0,0xFF,\
                         1,gwinPos[gwinId][0],gwinPos[gwinId][1],200,200,xOffset);
    RgnGopProcess();
}
// GOP Set GOP Attr
void RgnTestCase14(int gopId, int fmt, int cKeyEn, int cKeyRGB,
                   int strX, int strY, int strW, int strH,
                   int aType, int alpha0, int alpha1)
{
    RgnGopSetGopAttr(gopId,fmt,cKeyEn,cKeyRGB,strX,strY,strW,strH,aType,0,alpha0,alpha1);
    RgnGopProcess();
}
// GOP Set Gwin Attr
void RgnTestCase15(int gopId, int gwinId, int gwinEn, int gwinX, int gwinY, int gwinW, int gwinH, int xOffset)
{
    RgnGopSetGwinAttr(gopId,gwinId,gwinEn,gwinX,gwinY,gwinW,gwinH,xOffset);
    RgnGopProcess();
}
// COVER Bwin Size
void RgnTestCase20(int bwinW, int bwinH)
{
    int coverId, bwinId;
    int bwinPos[4][2];
    RGN_SET_CORNERS(bwinPos,RGN_DEF_BASE_W,RGN_DEF_BASE_H,bwinW,bwinH);
    RGN_SET_ALL_COVER_ATTR(coverId,bwinId,1,bwinPos[bwinId][0],bwinPos[bwinId][1],\
                           bwinW,bwinH,RgnRGBWinYUV[bwinId]);
    RgnCoverProcess();
}
// COVER Bwin Color
void RgnTestCase21(int color)
{
    int coverId, bwinId;
    int bwinPos[4][2];
    RGN_SET_CORNERS(bwinPos,RGN_DEF_BASE_W,RGN_DEF_BASE_H,200,200);
    RGN_SET_ALL_COVER_ATTR(coverId,bwinId,1,bwinPos[bwinId][0],bwinPos[bwinId][1],\
                           200,200,color);
    RgnCoverProcess();
}
// COVER Set Bwin Attr
void RgnTestCase22(int coverId, int bwinId, int bwinEn, int bwinX, int bwinY,
                   int bwinW, int bwinH, int color)
{
    RgnCoverSetBwinAttr(coverId,bwinId,bwinEn,bwinX,bwinY,bwinW,bwinH,color);
    RgnCoverProcess();
}

void RgnShowBufferInfo()
{
    int i;
    char str[64];
    CamOsPrintf("======================================== Buffer Info ========================================\n");
    _RgnPrintf("ssssss",15,"","Fmt","pvBufVirt","pvBufPhys","pvBufMiu","u32BufSize");
    for(i=0;i<RGN_FMT_MAX;i++)
    {
        CamOsSnprintf(str,64,"RGN_OSD_BUF_%d",i);
        _RgnPrintf("ssxxxx",15,str,RGN_FMT2STR(i),
                                   _r.stOsdBuf[i].pvBufVirt,
                                   _r.stOsdBuf[i].pvBufPhys,
                                   _r.stOsdBuf[i].pvBufMiu,
                                   _r.stOsdBuf[i].u32BufSize);
    }
    for(i=0;i<RGN_SCL_NUM;i++)
    {
        CamOsSnprintf(str,64,"RGN_SCL_BUF_%d",i);
        _RgnPrintf("ssxxxx",15,str,"NV12",
                                   _r.stSclBuf[i].pvBufVirt,
                                   _r.stSclBuf[i].pvBufPhys,
                                   _r.stSclBuf[i].pvBufMiu,
                                   _r.stSclBuf[i].u32BufSize);
    }
    CamOsPrintf("======================================== Buffer End  ========================================\n");
}
void RgnShowGopInfo()
{
    int gopId, gwinId;
    char str[64];
    CamOsPrintf("=========================================================== GOP Info ===========================================================\n");
    CamOsPrintf("geGopOsdFlag = 0x%x\n", geGopOsdFlag);
    for(gopId=0;gopId<E_MHAL_GOP_TYPE_MAX;gopId++)
    {
        if(!(geGopOsdFlag&(1<<gopId))) { continue; }
        CamOsPrintf("----------------------------------------------------------- GOP_%s -----------------------------------------------------------\n",RGN_PORT2STR(gopId));
        _RgnPrintf("sssssssssss",12,"Fmt","CKeyEn","CKeyRGB","StrWinX","StrWinY","StrWinW","StrWinH","AlphaType","AlphaValue","Alpha0","Alpha1");
        _RgnPrintf("sdxddddsxxx",12,RGN_FMT2STR(_r.stGop[gopId].eFmt),
                                    _r.stGop[gopId].bCKeyEn,
                                    _r.stGop[gopId].u32CKeyRGB,
                                    _r.stGop[gopId].stStrWin.u32X,
                                    _r.stGop[gopId].stStrWin.u32Y,
                                    _r.stGop[gopId].stStrWin.u32W,
                                    _r.stGop[gopId].stStrWin.u32H,
                                    RGN_ATYPE2STR(_r.stGop[gopId].eAType),
                                    _r.stGop[gopId].u32AValue,
                                    _r.stGop[gopId].u8Alpha0,
                                    _r.stGop[gopId].u8Alpha1);
        CamOsPrintf("\n");
        _RgnPrintf("sssssss",12,"","GwinEn","GwinX","GwinY","GwinW","GwinH","GwinBufXOft");
        for(gwinId=0;gwinId<E_MHAL_GOP_GWIN_ID_MAX;gwinId++)
        {
            CamOsSnprintf(str,64,"Gwin_%d",gwinId);
            _RgnPrintf("sdddddd",12,str,_r.stGop[gopId].bGwinEn[gwinId],
                                        _r.stGop[gopId].stGwin[gwinId].u32X,
                                        _r.stGop[gopId].stGwin[gwinId].u32Y,
                                        _r.stGop[gopId].stGwin[gwinId].u32W,
                                        _r.stGop[gopId].stGwin[gwinId].u32H,
                                        _r.stGop[gopId].u32GwinBufXOft[gwinId]);
        }
    }
    CamOsPrintf("=========================================================== GOP End  ===========================================================\n");
}
void RgnShowCoverInfo()
{
    int coverId, bwinId;
    char str[64];
    CamOsPrintf("======================================== COVER Info ========================================\n");
    for(coverId=0;coverId<E_MHAL_COVER_TYPE_MAX;coverId++)
    {
        CamOsPrintf("---------------------------------------- COVER_%s ----------------------------------------\n",RGN_PORT2STR(coverId));
        _RgnPrintf("sssssss",12,"","BwinEn","BwinX","BwinY","BwinW","BwinH","Color");
        for(bwinId=0;bwinId<E_MHAL_COVER_LAYER_MAX;bwinId++)
        {
            CamOsSnprintf(str,64,"Bwin_%d",bwinId);
            _RgnPrintf("sdddddx",12,str, _r.stCover[coverId].bBwinEn[bwinId],
                                         _r.stCover[coverId].stBwin[bwinId].u32X,
                                         _r.stCover[coverId].stBwin[bwinId].u32Y,
                                         _r.stCover[coverId].stBwin[bwinId].u32W,
                                         _r.stCover[coverId].stBwin[bwinId].u32H,
                                         _r.stCover[coverId].u32Color[bwinId]);
        }
    }
    CamOsPrintf("======================================== COVER End  ========================================\n");
}
// Show info
int RgnTestCase99(int mode) {
    if     (mode==0) { RgnShowBufferInfo(); }
    else if(mode==1) { RgnShowGopInfo();    }
    else if(mode==2) { RgnShowCoverInfo();  }
    else
    {
        CamOsPrintf("Error! No mode [%d]\n",mode);
        return eCLI_PARSE_INPUT_ERROR;
    }
    return eCLI_PARSE_OK;
}

//---------------------------------------------------------------------------
// Menu Function
//---------------------------------------------------------------------------
int RgnTest(CLI_t *pCli, char *p)
{
    int ret = eCLI_PARSE_OK;
    int test_case;
    int args[11];

    RGN_PARSE(pCli,1,&test_case);

    if(test_case == 0) { RgnTestCase0(); }
    else if(test_case == 1) { RgnTestCase1(); }
    else if(test_case == 2) {
        RGN_PARSE(pCli,1,&args[0]);
        CamOsPrintf(">>> GOP Bind OSD <<<\n");
        CamOsPrintf("Flag=%d\n",args[0]);
        CamOsPrintf("------------------\n");
        RgnTestCase2(args[0]);
    }
    else if(test_case == 10) {
        RGN_PARSE(pCli,3,&args[0],&args[1],&args[2]);
        CamOsPrintf(">>> GOP Gwin Size <<<\n");
        CamOsPrintf("Fmt=%s, GwinW=%d, GwinH=%d\n",RGN_FMT2STR(args[0]),args[1],args[2]);
        CamOsPrintf("------------------\n");
        RgnTestCase10(args[0], args[1], args[2]);
    }
    else if(test_case == 11) {
        RGN_PARSE(pCli,3,&args[0],&args[1],&args[2]);
        CamOsPrintf(">>> GOP Color key <<<\n");
        CamOsPrintf("Fmt=%s, CKeyEn=%d, CKeyRGB=0x%x\n",RGN_FMT2STR(args[0]),args[1],args[2]);
        CamOsPrintf("------------------\n");
        RgnTestCase11(args[0], args[1], args[2]);
    }
    else if(test_case == 12) {
        RGN_PARSE(pCli,4,&args[0],&args[1],&args[2],&args[3]);
        CamOsPrintf(">>> GOP Alpha Type <<<\n");
        CamOsPrintf("Fmt=%s, AType=%s, Alpha0=0x%x, Alpha1=0x%x\n",
                     RGN_FMT2STR(args[0]), RGN_ATYPE2STR(args[1]),args[2],args[3]);
        CamOsPrintf("------------------\n");
        RgnTestCase12(args[0], args[1], args[2], args[3]);
    }
    else if(test_case == 13) {
        RGN_PARSE(pCli,2,&args[0],&args[1]);
        CamOsPrintf(">>> GOP Gwin Buffer XOffset <<<\n");
        CamOsPrintf("Fmt=%s, XOffset=%d\n", RGN_FMT2STR(args[0]),args[1]);
        CamOsPrintf("------------------\n");
        RgnTestCase13(args[0], args[1]);
    }
    else if(test_case == 14) {
        RGN_PARSE(pCli,11,&args[0],&args[1],&args[2],&args[3],&args[4],&args[5],\
                          &args[6],&args[7],&args[8],&args[9],&args[10]);
        CamOsPrintf(">>> GOP Set GOP Attr <<<\n");
        CamOsPrintf("GopId=%d, Fmt=%s, CKeyEn=%d, CKeyRGB=0x%x\n",
                    args[0],RGN_FMT2STR(args[1]),args[2],args[3]);
        CamOsPrintf("StrWinX=%d, StrWinY=%d, StrWinW=%d, StrWinH=%d\n",
                    args[4],args[5],args[6],args[7]);
        CamOsPrintf("AType=%s, Alpha0=0x%x, Alpha1=0x%x\n",
                    RGN_ATYPE2STR(args[8]),args[9],args[10]);
        CamOsPrintf("------------------\n");
        RgnTestCase14(args[0],args[1],args[2],args[3],args[4],args[5],
                      args[6],args[7],args[8],args[9],args[10]);
    }
    else if(test_case == 15) {
        RGN_PARSE(pCli,8,&args[0],&args[1],&args[2],&args[3],&args[4],&args[5],&args[6],&args[7]);
        CamOsPrintf(">>> GOP Set Gwin Attr <<<\n");
        CamOsPrintf("GopId=%d, GwinId=%d, GwinEn=%d\n",args[0],args[1],args[2]);
        CamOsPrintf("GwinX=%d, GwinY=%d, GwinW=%d, GwinH=%d, XOffset=%d\n",
                    args[3],args[4],args[5],args[6],args[7]);
        CamOsPrintf("------------------\n");
        RgnTestCase15(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]);
    }
    else if(test_case == 20) {
        RGN_PARSE(pCli,2,&args[0],&args[1]);
        CamOsPrintf(">>> COVER Bwin Size <<<\n");
        CamOsPrintf("BwinW=%d, BwinH=%d\n",args[0],args[1]);
        CamOsPrintf("------------------\n");
        RgnTestCase20(args[0], args[1]);
    }
    else if(test_case == 21) {
        RGN_PARSE(pCli,1,&args[0]);
        CamOsPrintf(">>> COVER Bwin Color <<<\n");
        CamOsPrintf("Color=0x%x\n",args[0]);
        CamOsPrintf("------------------\n");
        RgnTestCase21(args[0]);
    }
    else if(test_case == 22) {
        RGN_PARSE(pCli,8,&args[0],&args[1],&args[2],&args[3],&args[4],\
                         &args[5],&args[6],&args[7]);
        CamOsPrintf(">>> COVER Set Bwin Attr <<<\n");
        CamOsPrintf("CoverId=%d, BwinId=%d, BwinEn=%d, BwinX=%d, BwinY=%d\n",
                     args[0],args[1],args[2],args[3],args[4]);
        CamOsPrintf("BwinW=%d, BwinH=%d, Color=0x%d\n",args[5],args[6],args[7]);
        CamOsPrintf("------------------\n");
        RgnTestCase22(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]);
    }
    else if(test_case == 99) {
        RGN_PARSE(pCli,1,&args[0]);
        CamOsPrintf(">>> Show info <<<\n");
        CamOsPrintf("Mode = %d\n",args[0]);
        CamOsPrintf("------------------\n");
        ret = RgnTestCase99(args[0]);
    }
    else
    {
        CamOsPrintf("Error! No test case [%d]\n",test_case);
        RGN_PARSE_ERROR;
    }
RGN_TEST_EXIT:
    return ret;
}

int RgnHelp(CLI_t *pCli, char *p)
{
    CamOsPrintf("====================== Rgn Help ======================\n");
    CamOsPrintf("Format :\n");
    CamOsPrintf("   0(ARGB1555) / 1(ARGB4444) / 2(I2) / 3(I4) / 4(I8)\n");
    CamOsPrintf("------------------------------------------------------\n");
    CamOsPrintf("Clear all\n");
    CamOsPrintf("   test 0\n");
    CamOsPrintf("Create OSD & SCL buffer\n");
    CamOsPrintf("   test 1\n");
    CamOsPrintf("GOP Bind OSD\n");
    CamOsPrintf("   test 2 [flag]\n");
    CamOsPrintf("------------------------ GOP -------------------------\n");
    CamOsPrintf("Gwin Size\n");
    CamOsPrintf("   test 10 [fmt] [gwinW] [gwinH]\n");
    CamOsPrintf("Color key\n");
    CamOsPrintf("   test 11 [fmt] [cKeyEn] [cKeyRGB]\n");
    CamOsPrintf("Alpha Type (0:constant, 1:pixel)\n");
    CamOsPrintf("   test 12 [fmt] [aType] [alpha0] [alpha1]\n");
    CamOsPrintf("Gwin Buffer XOffset\n");
    CamOsPrintf("   test 13 [fmt] [xOffset]\n");
    CamOsPrintf("Set GOP Attr\n");
    CamOsPrintf("   test 14 [gopId] [fmt] [cKeyEn] [cKeyRGB]\n");
    CamOsPrintf("           [strWinX] [strWinY] [strWinW] [strWinH]\n");
    CamOsPrintf("           [aType] [alpha0] [alpha1]\n");
    CamOsPrintf("Set Gwin Attr\n");
    CamOsPrintf("   test 15 [gopId] [gwinId] [gwinEn] [gwinX] [gwinY]\n");
    CamOsPrintf("           [gwinW] [gwinH] [xOffset]\n");
    CamOsPrintf("----------------------- COVER ------------------------\n");
    CamOsPrintf("Bwin Size\n");
    CamOsPrintf("   test 20 [bwinW] [bwinH]\n");
    CamOsPrintf("Bwin Color\n");
    CamOsPrintf("   test 21 [color]\n");
    CamOsPrintf("Set Bwin Attr\n");
    CamOsPrintf("   test 22 [coverId] [bwinId] [bBwinEn] [bwinX] [bwinY]\n");
    CamOsPrintf("           [bwinW] [bwinH] [Color]\n");
    CamOsPrintf("------------------------------------------------------\n");
    CamOsPrintf("Show info (0:Buffer, 1:GOP, 2:COVER)\n");
    CamOsPrintf("   test 99 [mode]\n");
    CamOsPrintf("====================== Rgn End  ======================\n");
    return eCLI_PARSE_OK;
}
