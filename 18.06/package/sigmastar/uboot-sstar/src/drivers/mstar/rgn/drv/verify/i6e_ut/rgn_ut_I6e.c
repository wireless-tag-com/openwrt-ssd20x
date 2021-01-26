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
#include <linux/file.h>

#include "cam_os_wrapper.h"
#include "mhal_common.h"
#include "mhal_rgn.h"
#include "rgn_capdef.h"
#include "rgn_file_access.h"


#define RGN_SCL_NUM 3
#define RGN_DIP_NUM 2
#define RGN_LDC_NUM 5
#define RGN_DEF_SCL_W 800
#define RGN_DEF_SCL_H 480
#define RGN_DEF_DIP_W 720
#define RGN_DEF_DIP_H 576

#define RGN_FMT_MAX 5

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
                           (port==E_MHAL_GOP_DIVP_PORT0)?"DIVP":\
                           (port==E_MHAL_GOP_LDC_PORT0) ?"LDC" :"N/A"
#define RGN_ATYPE2STR(type) (type==E_MHAL_GOP_GWIN_ALPHA_CONSTANT) ?"Constant":\
                            (type==    E_MHAL_GOP_GWIN_ALPHA_PIXEL)?"Pixel":"N/A"
#define RGN_SET_POS(idx,pos,max,baseW,baseH,winW,winH) {\
    for(idx=0;idx<max;idx++) {\
        pos[idx][0] = (baseW-winW)*idx/max;\
        pos[idx][1] = (baseH-winH)*idx/max;\
    }\
}
#define RGN_SET_ALL_GOP_ATTR(gopId,gwinId,eFmt,bCKeyEn,u32CKeyRGB,u32StrX,u32StrY,u32StrW,u32StrH,eAType,u32AValue,u8Alpha0,u8Alpha1,\
                             bGwinEn,u32GwinX,u32GwinY,u32GwinW,u32GwinH,u32GwinBufXOft) {\
    for(gopId=0;gopId<E_MHAL_GOP_TYPE_MAX;gopId++) {\
        RgnGopSetGopAttr(gopId,eFmt,bCKeyEn,u32CKeyRGB,u32StrX,u32StrY,u32StrW,u32StrH);\
        for(gwinId=0;gwinId<E_MHAL_GOP_GWIN_ID_MAX;gwinId++) {\
            RgnGopSetGwinAttr(gopId,gwinId,bGwinEn,u32GwinX,u32GwinY,u32GwinW,u32GwinH,u32GwinBufXOft,eAType,u32AValue,u8Alpha0,u8Alpha1);\
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
#define RgnScnprintf(buf, size, _fmt, _args...) scnprintf(buf, size, _fmt, ## _args)

typedef struct
{
    void *pvBufVirt;
    void *pvBufPhys;
    void *pvBufMiu;
    MS_U32 u32BufSize;
    char name[64];
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
    MHAL_RGN_GopWindowConfig_t stStrWin;
    MHAL_RGN_GopGwinAlphaType_e eAType[E_MHAL_GOP_GWIN_ID_MAX];
    MS_U32   u32AValue[E_MHAL_GOP_GWIN_ID_MAX];
    MS_U8    u8Alpha0[E_MHAL_GOP_GWIN_ID_MAX];
    MS_U8    u8Alpha1[E_MHAL_GOP_GWIN_ID_MAX];
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
    RgnBufInfo_t   stDipBuf[RGN_DIP_NUM];
    RgnBufInfo_t   stLdcBuf[RGN_LDC_NUM];
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
static int RgnOsdInfo[RGN_FMT_MAX][2] =
    {{200,200},{400,400},{256,256},{256,256},{400,400}};
static char RgnLdcBufName[RGN_LDC_NUM][64] = 
    {"RGN_LDC_BUF_DIST_MAP","RGN_LDC_BUF_IN_IMG",
     "RGN_LDC_BUF_POS_TBL","RGN_LDC_BUF_MASK_TBL","RGN_LDC_BUF_OUT_IMG"};
static int RgnLdcBufSize[RGN_LDC_NUM] =
    {0x1500, 512*960*2, 0xA0, 0x100, 480*960*2};

static MS_U32 RgnRGBWinYUV[4] = 
    {0xFF4C54,0x15952B,0x6B1DFF,0x80FF80};
static bool _gbSetPalette[E_MHAL_GOP_TYPE_MAX] = {0};

extern void RgnUtSclScript(MS_U32 u32MiuAddr0_Y, MS_U32 u32MiuAddr0_C,
                           MS_U32 u32MiuAddr1_Y, MS_U32 u32MiuAddr1_C,
                           MS_U32 u32MiuAddr2_Y, MS_U32 u32MiuAddr2_C);
extern void RgnUtDipScript(MS_U32 u32InY, MS_U32 u32InC,
                           MS_U32 u32OutY, MS_U32 u32OutC, MS_BOOL bOnlyTrig);
extern void RgnUtLdcScript(MS_U32 u32DistMap, MS_U32 u32InY, MS_U32 u32InC,
                           MS_U32 u32PosTbl, MS_U32 u32MaskTbl,
                           MS_U32 u32OutY, MS_U32 u32OutC, MS_BOOL bOnlyTrig);
extern void RgnUtReadReg(MS_U32 u32Bank);
extern void RgnUtWriteReg(MS_U32 u32Bank, MS_U32 u32Offset, MS_U32 u32Value);

//---------------------------------------------------------------------------
// Sub Function
//---------------------------------------------------------------------------
MS_BOOL _RgnLoadFile(unsigned long bufsize, void *pvBufVirt, char *fileName)
{
    unsigned long   readBufSize;
    struct file     *pstBinFd;
    if (strlen(fileName))
    {
        // Open file
        pstBinFd = OpenFile(fileName, O_RDONLY, 0);
        if (pstBinFd) {
            CamOsPrintf("Open file success: %s\r\n", fileName);
        } else {
            CamOsPrintf("Open file failed: %s\r\n", fileName);
            return FALSE;
        }
        CamOsPrintf("Reading file...\r\n");
        readBufSize = ReadFile(pstBinFd, pvBufVirt, bufsize);
        if (readBufSize == bufsize) {
            CamOsPrintf("Read file success: readBufSize = 0x%x\r\n", readBufSize);
        } else {
            CamOsPrintf("Read file failed: readBufSize = 0x%x\r\n", readBufSize);
            return FALSE;
        }
        CloseFile(pstBinFd);
    }
    return TRUE;
}
void _RgnAllocSclMem(void)
{
    int i;
    // SCL Buffer
    for(i=0;i<RGN_SCL_NUM;i++)
    {
        CamOsSnprintf(_r.stSclBuf[i].name,64,"RGN_SCL_BUF_%d",i);
        if(_r.stSclBuf[i].pvBufVirt) {
            CamOsPrintf("Memory Released : %s\n",_r.stSclBuf[i].name);
            CamOsDirectMemRelease(_r.stSclBuf[i].pvBufVirt,_r.stSclBuf[i].u32BufSize);
        }
        _r.stSclBuf[i].u32BufSize = RGN_DEF_SCL_W*RGN_DEF_SCL_H*2;
        CamOsDirectMemAlloc(_r.stSclBuf[i].name, _r.stSclBuf[i].u32BufSize, &(_r.stSclBuf[i].pvBufVirt), &(_r.stSclBuf[i].pvBufPhys), &(_r.stSclBuf[i].pvBufMiu));
        CamOsPrintf("Memory Allocated : %s\n",_r.stSclBuf[i].name);
    }

    // Call script
    RgnUtSclScript((MS_U32)(_r.stSclBuf[0].pvBufMiu),(MS_U32)(_r.stSclBuf[0].pvBufMiu+RGN_DEF_SCL_W*RGN_DEF_SCL_H),
                    (MS_U32)(_r.stSclBuf[1].pvBufMiu),(MS_U32)(_r.stSclBuf[1].pvBufMiu+RGN_DEF_SCL_W*RGN_DEF_SCL_H),
                    (MS_U32)(_r.stSclBuf[2].pvBufMiu),(MS_U32)(_r.stSclBuf[2].pvBufMiu+RGN_DEF_SCL_W*RGN_DEF_SCL_H));

    for(i=0;i<RGN_SCL_NUM;i++)
    {
        CamOsPrintf("%-22s : 0x%x\n",_r.stSclBuf[i].name,_r.stSclBuf[i].u32BufSize);
        CamOsPrintf("    Jlink : 0x%x\n",_r.stSclBuf[i].pvBufVirt);
    }
}
void _RgnAllocDipMem(void)
{
    int i;
    // DIP Buffer
    for(i=0;i<RGN_DIP_NUM;i++)
    {
        CamOsSnprintf(_r.stDipBuf[i].name,64,"RGN_DIP_BUF_%d",i);
        if(_r.stDipBuf[i].pvBufVirt) {
            CamOsPrintf("Memory Released : %s\n",_r.stDipBuf[i].name);
            CamOsDirectMemRelease(_r.stDipBuf[i].pvBufVirt,_r.stDipBuf[i].u32BufSize);
        }
        _r.stDipBuf[i].u32BufSize = RGN_DEF_DIP_W*RGN_DEF_DIP_H*2;
        CamOsDirectMemAlloc(_r.stDipBuf[i].name, _r.stDipBuf[i].u32BufSize, &(_r.stDipBuf[i].pvBufVirt), &(_r.stDipBuf[i].pvBufPhys), &(_r.stDipBuf[i].pvBufMiu));
        CamOsPrintf("Memory Allocated : %s\n",_r.stDipBuf[i].name);
    }
    
    RgnUtDipScript((MS_U32)(_r.stDipBuf[0].pvBufMiu),(MS_U32)(_r.stDipBuf[0].pvBufMiu+RGN_DEF_DIP_W*RGN_DEF_DIP_H),
                    (MS_U32)(_r.stDipBuf[1].pvBufMiu),(MS_U32)(_r.stDipBuf[1].pvBufMiu+RGN_DEF_DIP_W*RGN_DEF_DIP_H), 0);

    for(i=0;i<RGN_DIP_NUM;i++)
    {
        CamOsPrintf("%-21s : 0x%x\n",_r.stDipBuf[i].name,_r.stDipBuf[i].u32BufSize);
        CamOsPrintf("    Jlink : 0x%x\n",_r.stDipBuf[i].pvBufVirt);
    }
}
void _RgnAllocLdcMem(void)
{
    int i;
    // LDC Buffer
    for(i=0;i<RGN_LDC_NUM;i++)
    {
        CamOsSnprintf(_r.stLdcBuf[i].name,64,RgnLdcBufName[i]);
        if(_r.stLdcBuf[i].pvBufVirt) {
            CamOsPrintf("Memory Released : %s\n",_r.stLdcBuf[i].name);
            CamOsDirectMemRelease(_r.stLdcBuf[i].pvBufVirt,_r.stLdcBuf[i].u32BufSize);
        }
        _r.stLdcBuf[i].u32BufSize = RgnLdcBufSize[i];
        CamOsDirectMemAlloc(_r.stLdcBuf[i].name, _r.stLdcBuf[i].u32BufSize, &(_r.stLdcBuf[i].pvBufVirt), &(_r.stLdcBuf[i].pvBufPhys), &(_r.stLdcBuf[i].pvBufMiu));
        CamOsPrintf("Memory Allocated : %s\n",_r.stLdcBuf[i].name);
    }
    
    RgnUtLdcScript((MS_U32)(_r.stLdcBuf[0].pvBufMiu),
                    (MS_U32)(_r.stLdcBuf[1].pvBufMiu), (MS_U32)(_r.stLdcBuf[1].pvBufMiu+RgnLdcBufSize[1]/2),
                    (MS_U32)(_r.stLdcBuf[2].pvBufMiu), (MS_U32)(_r.stLdcBuf[3].pvBufMiu),
                    (MS_U32)(_r.stLdcBuf[4].pvBufMiu), (MS_U32)(_r.stLdcBuf[4].pvBufMiu+RgnLdcBufSize[4]/2), 0);

    for(i=0;i<RGN_LDC_NUM;i++)
    {
        CamOsPrintf("%-21s : 0x%x\n",_r.stLdcBuf[i].name,_r.stLdcBuf[i].u32BufSize);
        CamOsPrintf("    Jlink : 0x%x\n",_r.stLdcBuf[i].pvBufVirt);
    }
}
void _RgnAllocOsdMem(void)
{
    int i;
    // OSD Buffer
    for(i=0;i<RGN_FMT_MAX;i++)
    {
        CamOsSnprintf(_r.stOsdBuf[i].name,64,"RGN_OSD_BUF_%d",i);
        if(_r.stOsdBuf[i].pvBufVirt) {
            CamOsPrintf("Memory Released : %s\n",_r.stOsdBuf[i].name);
            CamOsDirectMemRelease(_r.stOsdBuf[i].pvBufVirt,_r.stOsdBuf[i].u32BufSize);
        }
        _r.stOsdBuf[i].u32BufSize = RGN_PIX2BYTE((RgnOsdInfo[i][0]*RgnOsdInfo[i][1]),i);
        CamOsDirectMemAlloc(_r.stOsdBuf[i].name, _r.stOsdBuf[i].u32BufSize, &(_r.stOsdBuf[i].pvBufVirt), &(_r.stOsdBuf[i].pvBufPhys), &(_r.stOsdBuf[i].pvBufMiu));
        CamOsPrintf("Memory Allocated : %s\n",_r.stOsdBuf[i].name);
    }
    
    for(i=0;i<RGN_FMT_MAX;i++)
    {
        CamOsPrintf("%-22s : %s\n",_r.stOsdBuf[i].name,RGN_FMT2STR(i));
        CamOsPrintf("    Jlink : 0x%x\n",_r.stOsdBuf[i].pvBufVirt);
    }
}

void _RgnReleaseMem(void)
{
    int i;
    for(i=0;i<RGN_FMT_MAX;i++)
    {
        if(_r.stOsdBuf[i].pvBufVirt) {
            CamOsPrintf("Memory Released : %s\n",_r.stOsdBuf[i].name);
            CamOsDirectMemRelease(_r.stOsdBuf[i].pvBufVirt,_r.stOsdBuf[i].u32BufSize);
        }
    }
    for(i=0;i<RGN_SCL_NUM;i++)
    {
        if(_r.stSclBuf[i].pvBufVirt) {
            CamOsPrintf("Memory Released : %s\n",_r.stSclBuf[i].name);
            CamOsDirectMemRelease(_r.stSclBuf[i].pvBufVirt,_r.stSclBuf[i].u32BufSize);
        }
    }
    for(i=0;i<RGN_DIP_NUM;i++)
    {
        if(_r.stDipBuf[i].pvBufVirt) {
            CamOsPrintf("Memory Released : %s\n",_r.stDipBuf[i].name);
            CamOsDirectMemRelease(_r.stDipBuf[i].pvBufVirt,_r.stDipBuf[i].u32BufSize);
        }
    }
    for(i=0;i<RGN_LDC_NUM;i++)
    {
        if(_r.stLdcBuf[i].pvBufVirt) {
            CamOsPrintf("Memory Released : %s\n",_r.stLdcBuf[i].name);
            CamOsDirectMemRelease(_r.stLdcBuf[i].pvBufVirt,_r.stLdcBuf[i].u32BufSize);
        }
    }
}
void _RgnPrintf(const char *fmt, int align, ...)
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
            CamOsPrintf("No Palette!Fmt=%s\r\n",RGN_FMT2STR(eFormat));
            break;
    }
}
void RgnGopSetGopAttr(int gopId, MHAL_RGN_GopPixelFormat_e eFmt,
                      MS_BOOL bCKeyEn, MS_U32 u32CKeyRGB,
                      MS_U32 u32X, MS_U32 u32Y, MS_U32 u32W, MS_U32 u32H)
{
    if(_r.stGop[gopId].eFmt!=eFmt) {_gbSetPalette[gopId] = 0;}
    _r.stGop[gopId].eFmt                = eFmt;
    _r.stGop[gopId].bCKeyEn             = bCKeyEn;
    _r.stGop[gopId].u32CKeyRGB          = u32CKeyRGB;
    _r.stGop[gopId].stStrWin.u32X       = u32X;
    _r.stGop[gopId].stStrWin.u32Y       = u32Y;
    _r.stGop[gopId].stStrWin.u32Width   = u32W;
    _r.stGop[gopId].stStrWin.u32Height  = u32H;
}
void RgnGopSetGwinAttr(int gopId, int gwinId, MS_BOOL bGwinEn,
                       MS_U32 u32X, MS_U32 u32Y, MS_U32 u32W, MS_U32 u32H,
                       MS_U32 u32GwinBufXOft,
                      MHAL_RGN_GopGwinAlphaType_e eAType, MS_U32 u32AValue,
                      MS_U8 u8Alpha0, MS_U8 u8Alpha1)
{
    _r.stGop[gopId].bGwinEn[gwinId]        = bGwinEn;
    _r.stGop[gopId].stGwin[gwinId].u32X    = u32X;
    _r.stGop[gopId].stGwin[gwinId].u32Y    = u32Y;
    _r.stGop[gopId].stGwin[gwinId].u32W    = u32W;
    _r.stGop[gopId].stGwin[gwinId].u32H    = u32H;
    _r.stGop[gopId].u32GwinBufXOft[gwinId] = u32GwinBufXOft;
    _r.stGop[gopId].eAType[gwinId]         = eAType;
    _r.stGop[gopId].u32AValue[gwinId]      = u32AValue;
    _r.stGop[gopId].u8Alpha0[gwinId]       = u8Alpha0;
    _r.stGop[gopId].u8Alpha1[gwinId]       = u8Alpha1;
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
void RgnGopProcess(void)
{
    int gopId, gwinId;
    unsigned int osdFlag;

    MHAL_RGN_GetGopBindOsd(&osdFlag);
    for(gopId=0;gopId<E_MHAL_GOP_TYPE_MAX;gopId++)
    {
        if(!(osdFlag&(1<<gopId))) { continue; }
        CamOsPrintf("---------Process GOP_%s---------\n",RGN_PORT2STR(gopId));
        MHAL_RGN_GopSetBaseWindow(gopId, &(_r.stGop[gopId].stStrWin), &(_r.stGop[gopId].stStrWin));
        if(!_gbSetPalette[gopId]) {
            RgnGopSetPalette(gopId,_r.stGop[gopId].eFmt);
            _gbSetPalette[gopId] = 1;
        } else {
            CamOsPrintf("Not set palette again, Fmt=%s\n",RGN_FMT2STR(_r.stGop[gopId].eFmt));
        }
        MHAL_RGN_GopSetColorkey(gopId,_r.stGop[gopId].bCKeyEn,
                                      ((_r.stGop[gopId].u32CKeyRGB)>>16)&0xFF,
                                      ((_r.stGop[gopId].u32CKeyRGB)>>8)&0xFF,
                                      (_r.stGop[gopId].u32CKeyRGB)&0xFF);
        MHAL_RGN_GopSetAlphaZeroOpaque(gopId,1,1,_r.stGop[gopId].eFmt);
        for(gwinId=0;gwinId<E_MHAL_GOP_GWIN_ID_MAX;gwinId++)
        {
            if(_r.stGop[gopId].bGwinEn[gwinId]) {
                MHAL_RGN_GopGwinSetPixelFormat(gopId,gwinId,_r.stGop[gopId].eFmt);
                MHAL_RGN_GopGwinSetWindow(gopId,gwinId,_r.stGop[gopId].stGwin[gwinId].u32W,
                                                       _r.stGop[gopId].stGwin[gwinId].u32H,
                                                       RGN_PIX2BYTE(RgnOsdInfo[_r.stGop[gopId].eFmt][0],_r.stGop[gopId].eFmt),
                                                       _r.stGop[gopId].stGwin[gwinId].u32X,
                                                       _r.stGop[gopId].stGwin[gwinId].u32Y);
                MHAL_RGN_GopGwinSetBufferXoffset(gopId,gwinId,_r.stGop[gopId].u32GwinBufXOft[gwinId]);
                MHAL_RGN_GopGwinSetBuffer(gopId,gwinId,(MS_U32)_r.stOsdBuf[_r.stGop[gopId].eFmt].pvBufMiu);
                MHAL_RGN_GopGwinEnable(gopId,gwinId);
                MHAL_RGN_GopSetAlphaType(gopId,gwinId,_r.stGop[gopId].eAType[gwinId],_r.stGop[gopId].u32AValue[gwinId]);
                MHAL_RGN_GopSetArgb1555AlphaVal(gopId,gwinId,E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0,_r.stGop[gopId].u8Alpha0[gwinId]);
                MHAL_RGN_GopSetArgb1555AlphaVal(gopId,gwinId,E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1,_r.stGop[gopId].u8Alpha1[gwinId]);
            } else {
                MHAL_RGN_GopGwinDisable(gopId,gwinId);
            }
        }
    }
}
void RgnCoverProcess(void)
{
    int coverId, bwinId;

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
}

// Clear all
void RgnTestCase0(int mode)
{
    int devId, winId;
    if(mode==0) {
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
        memset(_r.stGop,0,sizeof(RgnGopInfo_t)*E_MHAL_GOP_TYPE_MAX);
        memset(_r.stCover,0,sizeof(RgnGopInfo_t)*E_MHAL_COVER_TYPE_MAX);
    }
    else if(mode==1) {
        _RgnReleaseMem();
        memset(_r.stOsdBuf,0,sizeof(RgnBufInfo_t)*RGN_FMT_MAX);
        memset(_r.stSclBuf,0,sizeof(RgnBufInfo_t)*RGN_SCL_NUM);
        memset(_r.stDipBuf,0,sizeof(RgnBufInfo_t)*RGN_DIP_NUM);
        memset(_r.stLdcBuf,0,sizeof(RgnBufInfo_t)*RGN_LDC_NUM);
    }
    
}
// Create Buffer
void RgnTestCase1(int mode)
{
    switch(mode)
    {
    case 0:
        _RgnAllocOsdMem();
        break;
    case 1:
        _RgnAllocSclMem();
        break;
    case 2:
        _RgnAllocDipMem();
        break;
    case 3:
        _RgnAllocLdcMem();
        break;
    default:
        CamOsPrintf("Error mode[%d]!\n",mode);
    }
    
}
// GOP Bind OSD
void RgnTestCase2(int flag)
{
    MHAL_RGN_SetGopBindOsd(flag);
}
// Trigger DIP
void RgnTestCase3(int onlyTrig)
{
    RgnUtDipScript((MS_U32)(_r.stDipBuf[0].pvBufMiu),(MS_U32)(_r.stDipBuf[0].pvBufMiu+RGN_DEF_DIP_W*RGN_DEF_DIP_H),
                    (MS_U32)(_r.stDipBuf[1].pvBufMiu),(MS_U32)(_r.stDipBuf[1].pvBufMiu+RGN_DEF_DIP_W*RGN_DEF_DIP_H), onlyTrig);    
}
// Trigger LDC
void RgnTestCase4(int onlyTrig)
{
    RgnUtLdcScript((MS_U32)(_r.stLdcBuf[0].pvBufMiu),
                    (MS_U32)(_r.stLdcBuf[1].pvBufMiu), (MS_U32)(_r.stLdcBuf[1].pvBufMiu+RgnLdcBufSize[1]/2),
                    (MS_U32)(_r.stLdcBuf[2].pvBufMiu), (MS_U32)(_r.stLdcBuf[3].pvBufMiu),
                    (MS_U32)(_r.stLdcBuf[4].pvBufMiu), (MS_U32)(_r.stLdcBuf[4].pvBufMiu+RgnLdcBufSize[4]/2), onlyTrig);
}
// Reset SCL
void RgnTestCase5(void)
{
    RgnUtSclScript((MS_U32)(_r.stSclBuf[0].pvBufMiu),(MS_U32)(_r.stSclBuf[0].pvBufMiu+RGN_DEF_SCL_W*RGN_DEF_SCL_H),
                    (MS_U32)(_r.stSclBuf[1].pvBufMiu),(MS_U32)(_r.stSclBuf[1].pvBufMiu+RGN_DEF_SCL_W*RGN_DEF_SCL_H),
                    (MS_U32)(_r.stSclBuf[2].pvBufMiu),(MS_U32)(_r.stSclBuf[2].pvBufMiu+RGN_DEF_SCL_W*RGN_DEF_SCL_H));
}
// Process GOP
void RgnTestCase6(void)
{
    RgnGopProcess();
}
// GOP Gwin Size
void RgnTestCase10(int fmt, int gwinW, int gwinH)
{
    int gopId, gwinId;
    int gwinPos[E_MHAL_GOP_GWIN_ID_MAX][2];
    RGN_SET_POS(gwinId,gwinPos,E_MHAL_GOP_GWIN_ID_MAX,RGN_DEF_SCL_W,RGN_DEF_SCL_H,gwinW,gwinH);
    RGN_SET_ALL_GOP_ATTR(gopId,gwinId,fmt,0,0,0,0,RGN_DEF_SCL_W,RGN_DEF_SCL_H,0,0,0x0,0xFF,\
                         1,gwinPos[gwinId][0],gwinPos[gwinId][1],gwinW,gwinH,0);
    RgnGopProcess();
}
// GOP Color Key
void RgnTestCase11(int fmt, int cKeyEn, int cKeyRGB)
{
    int gopId, gwinId;
    int gwinPos[E_MHAL_GOP_GWIN_ID_MAX][2];
    RGN_SET_POS(gwinId,gwinPos,E_MHAL_GOP_GWIN_ID_MAX,RGN_DEF_SCL_W,RGN_DEF_SCL_H,200,200);
    RGN_SET_ALL_GOP_ATTR(gopId,gwinId,fmt,cKeyEn,cKeyRGB,0,0,RGN_DEF_SCL_W,RGN_DEF_SCL_H,0,0,0x0,0xFF,\
                         1,gwinPos[gwinId][0],gwinPos[gwinId][1],200,200,0);
    RgnGopProcess();
}
// GOP Alpha Type
void RgnTestCase12(int fmt, int aType, int alpha0, int alpha1)
{
    int gopId, gwinId;
    int gwinPos[E_MHAL_GOP_GWIN_ID_MAX][2];
    RGN_SET_POS(gwinId,gwinPos,E_MHAL_GOP_GWIN_ID_MAX,RGN_DEF_SCL_W,RGN_DEF_SCL_H,200,200);
    RGN_SET_ALL_GOP_ATTR(gopId,gwinId,fmt,0,0,0,0,RGN_DEF_SCL_W,RGN_DEF_SCL_H,aType,0,alpha0,alpha1,\
                         1,gwinPos[gwinId][0],gwinPos[gwinId][1],200,200,0);
    RgnGopProcess();
}
// GOP Gwin Buffer XOffset
void RgnTestCase13(int fmt, int xOffset)
{
    int gopId, gwinId;
    int gwinPos[E_MHAL_GOP_GWIN_ID_MAX][2];
    RGN_SET_POS(gwinId,gwinPos,E_MHAL_GOP_GWIN_ID_MAX,RGN_DEF_SCL_W,RGN_DEF_SCL_H,200,200);
    RGN_SET_ALL_GOP_ATTR(gopId,gwinId,fmt,0,0,0,0,RGN_DEF_SCL_W,RGN_DEF_SCL_H,0,0,0x0,0xFF,\
                         1,gwinPos[gwinId][0],gwinPos[gwinId][1],200,200,xOffset);
    RgnGopProcess();
}
// GOP Set GOP Attr
void RgnTestCase14(int gopId, int fmt, int cKeyEn, int cKeyRGB,
                   int strX, int strY, int strW, int strH)
{
    RgnGopSetGopAttr(gopId,fmt,cKeyEn,cKeyRGB,strX,strY,strW,strH);
}
// GOP Set Gwin Attr
void RgnTestCase15(int gopId, int gwinId, int gwinEn, int gwinX, int gwinY, int gwinW, int gwinH, int xOffset,
                   int aType, int alpha0, int alpha1)
{
    RgnGopSetGwinAttr(gopId,gwinId,gwinEn,gwinX,gwinY,gwinW,gwinH,xOffset,aType,0,alpha0,alpha1);
}
// COVER Bwin Size
void RgnTestCase20(int bwinW, int bwinH)
{
    int coverId, bwinId;
    int bwinPos[E_MHAL_COVER_LAYER_MAX][2];
    RGN_SET_POS(bwinId,bwinPos,E_MHAL_COVER_LAYER_MAX,RGN_DEF_SCL_W,RGN_DEF_SCL_H,bwinW,bwinH);
    RGN_SET_ALL_COVER_ATTR(coverId,bwinId,1,bwinPos[bwinId][0],bwinPos[bwinId][1],\
                           bwinW,bwinH,RgnRGBWinYUV[bwinId]);
    RgnCoverProcess();
}
// COVER Bwin Color
void RgnTestCase21(int color)
{
    int coverId, bwinId;
    int bwinPos[E_MHAL_COVER_LAYER_MAX][2];
    RGN_SET_POS(bwinId,bwinPos,E_MHAL_COVER_LAYER_MAX,RGN_DEF_SCL_W,RGN_DEF_SCL_H,200,200);
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

void RgnShowBufferInfo(void)
{
    int i;
    CamOsPrintf("======================================== Buffer Info ========================================\n");
    _RgnPrintf("ssssss",15,"","Fmt","pvBufVirt","pvBufPhys","pvBufMiu","u32BufSize");
    for(i=0;i<RGN_FMT_MAX;i++)
    {
        _RgnPrintf("ssxxxx",15, _r.stOsdBuf[i].name, RGN_FMT2STR(i),
                                _r.stOsdBuf[i].pvBufVirt,
                                _r.stOsdBuf[i].pvBufPhys,
                                _r.stOsdBuf[i].pvBufMiu,
                                _r.stOsdBuf[i].u32BufSize);
    }
    for(i=0;i<RGN_SCL_NUM;i++)
    {
        _RgnPrintf("ssxxxx",15,_r.stSclBuf[i].name,"NV12",
                               _r.stSclBuf[i].pvBufVirt,
                               _r.stSclBuf[i].pvBufPhys,
                               _r.stSclBuf[i].pvBufMiu,
                               _r.stSclBuf[i].u32BufSize);
    }
    for(i=0;i<RGN_DIP_NUM;i++)
    {
        _RgnPrintf("ssxxxx",15,_r.stDipBuf[i].name,"NV12",
                               _r.stDipBuf[i].pvBufVirt,
                               _r.stDipBuf[i].pvBufPhys,
                               _r.stDipBuf[i].pvBufMiu,
                               _r.stDipBuf[i].u32BufSize);
    }
    for(i=0;i<RGN_LDC_NUM;i++)
    {
        _RgnPrintf("ssxxxx",15,_r.stLdcBuf[i].name,"NV12",
                               _r.stLdcBuf[i].pvBufVirt,
                               _r.stLdcBuf[i].pvBufPhys,
                               _r.stLdcBuf[i].pvBufMiu,
                               _r.stLdcBuf[i].u32BufSize);
    }
    CamOsPrintf("======================================== Buffer End  ========================================\n");
}
void RgnShowGopInfo(void)
{
    int gopId, gwinId;
    char str[64];
    unsigned int osdFlag;

    CamOsPrintf("=========================================================== GOP Info ===========================================================\n");
    MHAL_RGN_GetGopBindOsd(&osdFlag); 
    CamOsPrintf("osdFlag = 0x%x\n", osdFlag);
    for(gopId=0;gopId<E_MHAL_GOP_TYPE_MAX;gopId++)
    {
        if(!(osdFlag&(1<<gopId))) { continue; }
        CamOsPrintf("----------------------------------------------------------- GOP_%s -----------------------------------------------------------\n",RGN_PORT2STR(gopId));
        _RgnPrintf("sssssssssss",12,"Fmt","CKeyEn","CKeyRGB","StrWinX","StrWinY","StrWinW","StrWinH","AlphaType","AlphaValue","Alpha0","Alpha1");
        _RgnPrintf("sdxddddsxxx",12,RGN_FMT2STR(_r.stGop[gopId].eFmt),
                                    _r.stGop[gopId].bCKeyEn,
                                    _r.stGop[gopId].u32CKeyRGB,
                                    _r.stGop[gopId].stStrWin.u32X,
                                    _r.stGop[gopId].stStrWin.u32Y,
                                    _r.stGop[gopId].stStrWin.u32Width,
                                    _r.stGop[gopId].stStrWin.u32Height,
                                    RGN_ATYPE2STR(_r.stGop[gopId].eAType[gwinId]),
                                    _r.stGop[gopId].u32AValue[gwinId],
                                    _r.stGop[gopId].u8Alpha0[gwinId],
                                    _r.stGop[gopId].u8Alpha1[gwinId]);
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
void RgnShowCoverInfo(void)
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
// Read register
void RgnTestCase97(int bank) {
    RgnUtReadReg(bank);
}
// Write register
void RgnTestCase98(int bank, int offset, int value) {
    RgnUtWriteReg(bank, offset, value);
}
// Show info
void RgnTestCase99(int mode) {
    if     (mode==0) { RgnShowBufferInfo(); }
    else if(mode==1) { RgnShowGopInfo();    }
    else if(mode==2) { RgnShowCoverInfo();  }
    else
    {
        CamOsPrintf("Error! No mode [%d]\n",mode);
    }
}

//---------------------------------------------------------------------------
// Menu Function
//---------------------------------------------------------------------------
void RgnTest(const char *buf, u32 n)
{
    int test_case;
    int args[11];
    const char *str = buf;
    int u32size = 0;

    u32size += sscanf(str, "%d", &test_case);
    str     += (u32size+1);

    if(test_case == 0) {
        sscanf(str,"%d",&args[0]);
        CamOsPrintf(">>> Clear all <<<\n");
        CamOsPrintf("Mode=%d\n",args[0]);
        CamOsPrintf("------------------\n");
        RgnTestCase0(args[0]); 
    }
    else if(test_case == 1) {
        sscanf(str,"%d",&args[0]);
        CamOsPrintf(">>> Create buffer <<<\n");
        CamOsPrintf("Mode=%d\n",args[0]);
        CamOsPrintf("------------------\n");
        RgnTestCase1(args[0]); 
    }
    else if(test_case == 2) {
        sscanf(str,"%d",&args[0]);
        CamOsPrintf(">>> GOP Bind OSD <<<\n");
        CamOsPrintf("Flag=%d\n",args[0]);
        CamOsPrintf("------------------\n");
        RgnTestCase2(args[0]);
    }
    else if(test_case == 3) {
        sscanf(str,"%d",&args[0]);
        CamOsPrintf(">>> Trigger DIP <<<\n");
        CamOsPrintf("Only trigger=%d\n",args[0]);
        CamOsPrintf("------------------\n");
        RgnTestCase3(args[0]);
    }
    else if(test_case == 4) {
        sscanf(str,"%d",&args[0]);
        CamOsPrintf(">>> Trigger LDC <<<\n");
        CamOsPrintf("Only trigger=%d\n",args[0]);
        CamOsPrintf("------------------\n");
        RgnTestCase4(args[0]);
    }
    else if(test_case == 5) {
        CamOsPrintf(">>> Reset SCL <<<\n");
        CamOsPrintf("------------------\n");
        RgnTestCase5();
    }
    else if(test_case == 6) {
        CamOsPrintf(">>> Process GOP <<<\n");
        CamOsPrintf("------------------\n");
        RgnTestCase6();
    }
    else if(test_case == 10) {
        sscanf(str,"%d %d %d",&args[0],&args[1],&args[2]);
        CamOsPrintf(">>> GOP Gwin Size <<<\n");
        CamOsPrintf("Fmt=%s, GwinW=%d, GwinH=%d\n",RGN_FMT2STR(args[0]),args[1],args[2]);
        CamOsPrintf("------------------\n");
        RgnTestCase10(args[0], args[1], args[2]);
    }
    else if(test_case == 11) {
        sscanf(str,"%d %d %d",&args[0],&args[1],&args[2]);
        CamOsPrintf(">>> GOP Color key <<<\n");
        CamOsPrintf("Fmt=%s, CKeyEn=%d, CKeyRGB=0x%x\n",RGN_FMT2STR(args[0]),args[1],args[2]);
        CamOsPrintf("------------------\n");
        RgnTestCase11(args[0], args[1], args[2]);
    }
    else if(test_case == 12) {
        sscanf(str,"%d %d %d %d",&args[0],&args[1],&args[2],&args[3]);
        CamOsPrintf(">>> GOP Alpha Type <<<\n");
        CamOsPrintf("Fmt=%s, AType=%s, Alpha0=0x%x, Alpha1=0x%x\n",
                     RGN_FMT2STR(args[0]), RGN_ATYPE2STR(args[1]),args[2],args[3]);
        CamOsPrintf("------------------\n");
        RgnTestCase12(args[0], args[1], args[2], args[3]);
    }
    else if(test_case == 13) {
        sscanf(str,"%d %d",&args[0],&args[1]);
        CamOsPrintf(">>> GOP Gwin Buffer XOffset <<<\n");
        CamOsPrintf("Fmt=%s, XOffset=%d\n", RGN_FMT2STR(args[0]),args[1]);
        CamOsPrintf("------------------\n");
        RgnTestCase13(args[0], args[1]);
    }
    else if(test_case == 14) {
        sscanf(str,"%d %d %d %d %d %d %d %d",
               &args[0],&args[1],&args[2],&args[3],&args[4],&args[5],
               &args[6],&args[7]);
        CamOsPrintf(">>> GOP Set GOP Attr <<<\n");
        CamOsPrintf("GopId=%d, Fmt=%s, CKeyEn=%d, CKeyRGB=0x%x\n",
                    args[0],RGN_FMT2STR(args[1]),args[2],args[3]);
        CamOsPrintf("StrWinX=%d, StrWinY=%d, StrWinW=%d, StrWinH=%d\n",
                    args[4],args[5],args[6],args[7]);
        CamOsPrintf("------------------\n");
        RgnTestCase14(args[0],args[1],args[2],args[3],args[4],args[5],
                      args[6],args[7]);
    }
    else if(test_case == 15) {
        sscanf(str,"%d %d %d %d %d %d %d %d %d %d %d",
               &args[0],&args[1],&args[2],&args[3],&args[4],&args[5],
               &args[6],&args[7],&args[8],&args[9],&args[10]);
        CamOsPrintf(">>> GOP Set Gwin Attr <<<\n");
        CamOsPrintf("GopId=%d, GwinId=%d, GwinEn=%d\n",args[0],args[1],args[2]);
        CamOsPrintf("GwinX=%d, GwinY=%d, GwinW=%d, GwinH=%d, XOffset=%d\n",
                    args[3],args[4],args[5],args[6],args[7]);
        CamOsPrintf("AType=%s, Alpha0=0x%x, Alpha1=0x%x\n",
                    RGN_ATYPE2STR(args[8]),args[9],args[10]);
        CamOsPrintf("------------------\n");
        RgnTestCase15(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7],
                      args[8],args[9],args[10]);
    }
    else if(test_case == 20) {
        sscanf(str,"%d %d",&args[0],&args[1]);
        CamOsPrintf(">>> COVER Bwin Size <<<\n");
        CamOsPrintf("BwinW=%d, BwinH=%d\n",args[0],args[1]);
        CamOsPrintf("------------------\n");
        RgnTestCase20(args[0], args[1]);
    }
    else if(test_case == 21) {
        sscanf(str,"%d",&args[0]);
        CamOsPrintf(">>> COVER Bwin Color <<<\n");
        CamOsPrintf("Color=0x%x\n",args[0]);
        CamOsPrintf("------------------\n");
        RgnTestCase21(args[0]);
    }
    else if(test_case == 22) {
        sscanf(str,"%d %d %d %d %d %d %d %d",
               &args[0],&args[1],&args[2],&args[3],&args[4],&args[5],
               &args[6],&args[7]);
        CamOsPrintf(">>> COVER Set Bwin Attr <<<\n");
        CamOsPrintf("CoverId=%d, BwinId=%d, BwinEn=%d, BwinX=%d, BwinY=%d\n",
                     args[0],args[1],args[2],args[3],args[4]);
        CamOsPrintf("BwinW=%d, BwinH=%d, Color=0x%d\n",args[5],args[6],args[7]);
        CamOsPrintf("------------------\n");
        RgnTestCase22(args[0],args[1],args[2],args[3],args[4],args[5],args[6],args[7]);
    }
    else if(test_case == 97) {
        sscanf(str,"%x",&args[0]);
        CamOsPrintf(">>> Read register <<<\n");
        CamOsPrintf("Bank = 0x%x\n",args[0]);
        CamOsPrintf("------------------\n");
        RgnTestCase97(args[0]);
    }
    else if(test_case == 98) {
        sscanf(str,"%x %x %x",&args[0],&args[1],&args[2]);
        CamOsPrintf(">>> Write register <<<\n");
        CamOsPrintf("Bank=0x%x, Offset=0x%x, Value=0x%x\n",args[0],args[1],args[2]);
        CamOsPrintf("------------------\n");
        RgnTestCase98(args[0],args[1],args[2]);
    }
    else if(test_case == 99) {
        sscanf(str,"%d",&args[0]);
        CamOsPrintf(">>> Show info <<<\n");
        CamOsPrintf("Mode = %d\n",args[0]);
        CamOsPrintf("------------------\n");
        RgnTestCase99(args[0]);
    }
    else
    {
        CamOsPrintf("Error! No test case [%d]\n",test_case);
    }
}

u32 RgnTestShow(char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    str += RgnScnprintf(str, end - str, "====================== Rgn Help ======================\n");
    str += RgnScnprintf(str, end - str, "Format :\n");
    str += RgnScnprintf(str, end - str, "   0(ARGB1555) / 1(ARGB4444) / 2(I2) / 3(I4) / 4(I8)\n");
    str += RgnScnprintf(str, end - str, "------------------------------------------------------\n");
    str += RgnScnprintf(str, end - str, "Clear all (0:Disable gop & cover, 1:Release Memory)\n");
    str += RgnScnprintf(str, end - str, "   echo 0 [mode] > test\n");
    str += RgnScnprintf(str, end - str, "Create buffer (0:OSD, 1:SCL, 2:DIP, 3:LDC)\n");
    str += RgnScnprintf(str, end - str, "   echo 1 [mode] > test\n");
    str += RgnScnprintf(str, end - str, "GOP Bind OSD\n");
    str += RgnScnprintf(str, end - str, "   echo 2 [flag] > test\n");
    str += RgnScnprintf(str, end - str, "Trigger DIP\n");
    str += RgnScnprintf(str, end - str, "   echo 3 [onlyTrig] > test\n");
    str += RgnScnprintf(str, end - str, "Trigger LDC\n");
    str += RgnScnprintf(str, end - str, "   echo 4 [onlyTrig] > test\n");
    str += RgnScnprintf(str, end - str, "Reset SCL\n");
    str += RgnScnprintf(str, end - str, "   echo 5 > test\n");
    str += RgnScnprintf(str, end - str, "Process GOP\n");
    str += RgnScnprintf(str, end - str, "   echo 6 > test\n");
    str += RgnScnprintf(str, end - str, "------------------------ GOP -------------------------\n");
    str += RgnScnprintf(str, end - str, "Gwin Size\n");
    str += RgnScnprintf(str, end - str, "   echo 10 [fmt] [gwinW] [gwinH] > test\n");
    str += RgnScnprintf(str, end - str, "Color key\n");
    str += RgnScnprintf(str, end - str, "   echo 11 [fmt] [cKeyEn] [cKeyRGB] > test\n");
    str += RgnScnprintf(str, end - str, "Alpha Type (0:constant, 1:pixel)\n");
    str += RgnScnprintf(str, end - str, "   echo 12 [fmt] [aType] [alpha0] [alpha1] > test\n");
    str += RgnScnprintf(str, end - str, "Gwin Buffer XOffset\n");
    str += RgnScnprintf(str, end - str, "   echo 13 [fmt] [xOffset] > test\n");
    str += RgnScnprintf(str, end - str, "Set GOP Attr\n");
    str += RgnScnprintf(str, end - str, "   echo 14 [gopId] [fmt] [cKeyEn] [cKeyRGB]\n");
    str += RgnScnprintf(str, end - str, "           [strWinX] [strWinY] [strWinW] [strWinH] > test\n");
    str += RgnScnprintf(str, end - str, "Set Gwin Attr\n");
    str += RgnScnprintf(str, end - str, "   echo 15 [gopId] [gwinId] [gwinEn] [gwinX] [gwinY]\n");
    str += RgnScnprintf(str, end - str, "           [gwinW] [gwinH] [xOffset]\n");
    str += RgnScnprintf(str, end - str, "           [aType] [alpha0] [alpha1] > test\n");
    str += RgnScnprintf(str, end - str, "----------------------- COVER ------------------------\n");
    str += RgnScnprintf(str, end - str, "Bwin Size\n");
    str += RgnScnprintf(str, end - str, "   echo 20 [bwinW] [bwinH] > test\n");
    str += RgnScnprintf(str, end - str, "Bwin Color\n");
    str += RgnScnprintf(str, end - str, "   echo 21 [color] > test\n");
    str += RgnScnprintf(str, end - str, "Set Bwin Attr\n");
    str += RgnScnprintf(str, end - str, "   echo 22 [coverId] [bwinId] [bBwinEn] [bwinX] [bwinY]\n");
    str += RgnScnprintf(str, end - str, "           [bwinW] [bwinH] [Color] > test\n");
    str += RgnScnprintf(str, end - str, "------------------------------------------------------\n");
    str += RgnScnprintf(str, end - str, "Read register\n");
    str += RgnScnprintf(str, end - str, "   echo 97 [bank] > test\n");
    str += RgnScnprintf(str, end - str, "Write register\n");
    str += RgnScnprintf(str, end - str, "   echo 98 [bank] [offset] [value] > test\n");
    str += RgnScnprintf(str, end - str, "Show info (0:Buffer, 1:GOP, 2:COVER)\n");
    str += RgnScnprintf(str, end - str, "   echo 99 [mode] > test\n");
    str += RgnScnprintf(str, end - str, "====================== Rgn End  ======================\n");
    end = end;
    return (str - buf);
}
void RgnTestInit(void)
{
    MHAL_RGN_GopInit();
    MHAL_RGN_CoverInit();
}
void RgnTestDeInit(void)
{
    MHAL_RGN_GopInit();
    MHAL_RGN_CoverInit();
    RgnTestCase0(0);
    RgnTestCase0(1);
}


