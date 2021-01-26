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


/**
* \ingroup hvsp_group
* @{
*/

#ifndef __DRV_GOP_H__
#define __DRV_GOP_H__
#include "drv_rgn_common.h"
#include "hal_gop.h"

//=============================================================================
// Defines
//=============================================================================
#define DRV_GOP_VERSION                        0x0100



//=============================================================================
// enum
//=============================================================================

typedef enum
{
    E_DRV_GOP_ID_0 = 0,
    E_DRV_GOP_ID_1,
    E_DRV_GOP_ID_2,
    E_DRV_GOP_ID_3,
    E_DRV_GOP_ID_4,
    E_DRV_GOP_ID_5,
    E_DRV_DISPGOP_ID_0,
    E_DRV_DISPGOP_ID_1,
    E_DRV_GOP_UNBIND,
} DrvGopIdType_e;


typedef enum
{
    E_DRV_GOP_OUT_FMT_RGB,
    E_DRV_GOP_OUT_FMT_YUV,
    E_DRV_GOP_OUT_FMT_NUM,
} DrvGopOutFmtType_e;

typedef enum
{
    E_DRV_GOP_DISPLAY_MD_INTERLACE,
    E_DRV_GOP_DISPLAY_MD_PROGRESS,
    E_DRV_GOP_DISPLAY_MD_NUM,
} DrvGopDisplayModeType_e;


typedef enum
{
    E_DRV_GOP_DEST_OP,
    E_DRV_GOP_DEST_NUM,
} DrvGopDestType_e;

typedef enum
{
    E_DRV_GOP_STRETCH_RATIO_1 = 1,
    E_DRV_GOP_STRETCH_RATIO_2 = 2,
    E_DRV_GOP_STRETCH_RATIO_4 = 4,
    E_DRV_GOP_STRETCH_RATIO_8 = 8,
} DrvGopStretchRatioType_e;
//------------------------------------------------------------------------------
// GWin
//------------------------------------------------------------------------------
typedef enum
{
    E_DRV_GOP_GWIN_ID_0,
    E_DRV_GOP_GWIN_ID_1,
    E_DRV_GOP_GWIN_ID_2,
    E_DRV_GOP_GWIN_ID_3,
    E_DRV_GOP_GWIN_ID_4,
    E_DRV_GOP_GWIN_ID_5,
    E_DRV_GOP_GWIN_ID_6,
    E_DRV_GOP_GWIN_ID_7,
} DrvGopGwinIdType_e;

typedef enum
{
    E_DRV_GOP_GWIN_SRC_FMT_RGB1555    = 0x00,
    E_DRV_GOP_GWIN_SRC_FMT_RGB565     = 0x01,
    E_DRV_GOP_GWIN_SRC_FMT_ARGB4444   = 0x02,
    E_DRV_GOP_GWIN_SRC_FMT_2266       = 0x03,
    E_DRV_GOP_GWIN_SRC_FMT_I8_PALETTE = 0x04,
    E_DRV_GOP_GWIN_SRC_FMT_ARGB8888   = 0x05,
    E_DRV_GOP_GWIN_SRC_FMT_ARGB1555   = 0x06,
    E_DRV_GOP_GWIN_SRC_FMT_ABGR8888   = 0x07,
    E_DRV_GOP_GWIN_SRC_FMT_UV7Y8      = 0x08,
    E_DRV_GOP_GWIN_SRC_FMT_UV8Y8      = 0x09,
    E_DRV_GOP_GWIN_SRC_FMT_RGBA5551   = 0x0A,
    E_DRV_GOP_GWIN_SRC_FMT_RGBA4444   = 0x0B,
    E_DRV_GOP_GWIN_SRC_I4_PALETTE = 0x0D,
    E_DRV_GOP_GWIN_SRC_I2_PALETTE = 0x0E,
    E_DRV_GOP_GWIN_SRC_FMT_NUM        = 0x0F,
} DrvGopGwinSrcFmtType_e;

typedef enum
{
    E_DRV_GOP_GWIN_ALPHA_CONSTANT,
    E_DRV_GOP_GWIN_ALPHA_PIXEL,
    E_DRV_GOP_GWIN_ALPHA_NUM,
} DrvGopGwinAlphaType_e;

typedef enum
{
    E_DRV_GOP_GWIN_ARGB1555_DEFINE_ALPHA0 = 0,
    E_DRV_GOP_GWIN_ARGB1555_DEFINE_ALPHA1 = 1,
    E_DRV_GOP_GWIN_ARGB1555_DEFINE_NOUSE = 2,
} DrvGopGwinArgb1555Def_e;


/**
* Used to setup the error type of hvsp device
*/
typedef enum
{
    E_DRV_GOP_ERR_OK    =  0, ///< No Error
    E_DRV_GOP_ERR_FAULT = -1, ///< Fault
    E_DRV_GOP_ERR_INVAL = -2, ///< Invalid value
} DrvGopErrType_e;
//=============================================================================
// struct
//=============================================================================

typedef struct
{
    u16 u16X;
    u16 u16Y;
    u16 u16Width;
    u16 u16Height;
} DrvGopWindowConfig_t;

typedef struct
{
    DrvGopGwinIdType_e     eGwinId;
    DrvGopGwinSrcFmtType_e eSrcFmt;
    DrvGopWindowConfig_t   tDisplayWin;
    u32 u32BaseAddr;
    u16 u16Base_XOffset;
    u32 u32Base_YOffset;
} DrvGopGwinConfig_t;

typedef struct
{
    DrvGopOutFmtType_e      eOutFmt;
    DrvGopDisplayModeType_e eDisplyMode;
    DrvGopWindowConfig_t    tOsdDisplayWindow;
    u8  u8GWinNum;
    DrvGopGwinConfig_t *pstGwinCfg;
    //DrvGopStretchRatioType_e eStretchHRatio;
} DrvGopModeConfig_t;

typedef struct
{
    DrvGopGwinAlphaType_e eAlphaType;
    DrvGopGwinArgb1555Def_e ePixelAlphaType;
    HalGopGwinAlphaVal_t tAlphaVal;
} DrvGopGwinAlphaConfig_t;

typedef struct
{
    bool bEn;
    u8 u8R;
    u8 u8G;
    u8 u8B;
} DrvGopColorKeyConfig_t;

typedef struct
{
    u16 u16Size;
    u8 *pu8Palette_A;
    u8 *pu8Palette_R;
    u8 *pu8Palette_G;
    u8 *pu8Palette_B;
} DrvGopGwinPaletteConfig_t;

typedef struct
{
    u8 u8Idx;
    u8 u8A;
    u8 u8R;
    u8 u8G;
    u8 u8B;
} DrvGopPaletteConfig_t;

typedef struct
{
    u16 u16Htotal;
    u16 u16Vtotal;
    u16 u16HdeStart;
    u16 u16VdeStart;
    u16 u16Width;
    u16 u16Height;
    bool bInterlaceMode;
    bool bYuvOutput;
} DrvGopVideoTimingInfoConfig_t;

//=============================================================================

//=============================================================================
#ifndef __DRV_GOP_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE bool DrvGopRgnInit(void);
INTERFACE bool DrvGopSetBindOsd(void);
INTERFACE u8 DrvGopGetGwinEnableCount(DrvGopIdType_e eGopId);
INTERFACE bool DrvGopRgnSetEnable(DrvGopIdType_e eGopId,  DrvGopGwinIdType_e eGwinId, bool bEn);
INTERFACE bool DrvGopUpdateBase(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, u32 u32BaseAddr);
INTERFACE bool DrvGopUpdateBufferXoffset(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, u16 u16Hoffset);
INTERFACE bool DrvGopSetMirror(DrvGopIdType_e eGopId, bool bEn);
INTERFACE bool DrvGopSetFlip(DrvGopIdType_e eGopId, bool bEn);
INTERFACE bool DrvGopSetMemPitchDirect(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, u32 u32MemPitch);
INTERFACE bool DrvGopSetSrcFmt(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, DrvGopGwinSrcFmtType_e eSrcFmt);
INTERFACE bool DrvGopSetAlphaBlending(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, DrvGopGwinAlphaConfig_t *ptAlphaCfg);
INTERFACE bool DrvGopSetColorKey(DrvGopIdType_e eGopId, DrvGopColorKeyConfig_t *ptColorKeyCfg, bool bVYU);
INTERFACE bool DrvGopSetPaletteRiuOneMem(DrvGopIdType_e eGopId, DrvGopPaletteConfig_t *ptCfg);
INTERFACE bool DrvGopSetGwinSizeWithoutFmt(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, DrvGopWindowConfig_t *ptWinCfg);
INTERFACE bool DrvGopSetStretchWindowSize(DrvGopIdType_e eGopId, DrvGopWindowConfig_t *ptSrcWinCfg, DrvGopWindowConfig_t *ptDstWinCfg);
INTERFACE bool DrvGopSetArgb1555AlphaDefVal(DrvGopIdType_e eGopId,  DrvGopGwinIdType_e eGwinId, DrvGopGwinArgb1555Def_e eAlphaType, MS_U8 u8AlphaVal);
INTERFACE bool DrvGopSetAlphaZeroOpaque(DrvGopIdType_e eGopId, bool bEn,bool bConAlpha,DrvGopGwinSrcFmtType_e eSrcFmt);
INTERFACE bool DrvGopGetEnableGwin(DrvGopIdType_e eGopId,  DrvGopGwinIdType_e eGwinId, bool *bEn);
INTERFACE bool DrvGopGetAlpha(DrvGopIdType_e eGopId, DrvGopGwinIdType_e eGwinId, DrvGopGwinAlphaConfig_t *pstAlphaCfg);
INTERFACE bool DrvGopGetColorKey(DrvGopIdType_e eGopId, DrvGopColorKeyConfig_t *pstColorKeyCfg);
INTERFACE bool DrvGopSetVideoTimingInfo(DrvGopIdType_e eGopId, DrvGopVideoTimingInfoConfig_t *ptTimingCfg);

#undef INTERFACE
#endif //
/** @} */ // end of hvsp_group
