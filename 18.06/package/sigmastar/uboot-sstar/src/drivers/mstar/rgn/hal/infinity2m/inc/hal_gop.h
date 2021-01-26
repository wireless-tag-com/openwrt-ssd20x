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


#ifndef __HAL_GOP_H__
#define __HAL_GOP_H__
#include "hal_rgn_common.h"
//=============================================================================
// Defines
//=============================================================================
#ifndef bool
#define  bool   unsigned char
#endif

#define GOP_BANK_ALLBANK_DOUBLE_WR 0x0100
#define GOP_BANK_FORCE_WR 0x0200
#define GOP_BANK_ONEBANK_DOUBLE_WR 0x0400
#define HAL_GOP_GET_GWIN_ID(eGopId,eGwinId) (E_HAL_ISPGOP_ID_01+(eGopId<<4))
//=============================================================================
// enum
//=============================================================================
typedef enum
{
    E_HAL_ISPGOP_ID_00     = 0x00,
    E_HAL_ISPGOP_ID_01     = 0x01,
    E_HAL_ISPGOP_ID_0_ST   = 0x02,
    E_HAL_ISPGOP_ID_10     = 0x10,
    E_HAL_ISPGOP_ID_11     = 0x11,
    E_HAL_ISPGOP_ID_1_ST   = 0x12,
    E_HAL_ISPGOP_ID_MAX    = 0x12,
} HalGopIdType_e;
typedef enum
{
    E_HAL_GOP_GWIN_ID_0   = 0,
    E_HAL_GOP_GWIN_ID_NUM = 1,
} HalGopGwinIdType_e;

typedef enum
{
    E_HAL_GOP_DISPLAY_MODE_INTERLACE = 0x00,
    E_HAL_GOP_DISPLAY_MODE_PROGRESS  = 0x01,
} HalGopDisplayModeType_e;

typedef enum
{
    E_HAL_GOP_OUT_FMT_RGB = 0x00,
    E_HAL_GOP_OUT_FMT_YUV = 0x01,
} HalGopOutFormatType_e;

typedef enum
{
    E_HAL_GOP_GWIN_SRC_I8_PALETTE = 0,
    E_HAL_GOP_GWIN_SRC_I4_PALETTE = 0x01,
    E_HAL_GOP_GWIN_SRC_I2_PALETTE = 0x02,
    E_HAL_GOP_GWIN_SRC_ARGB1555   = 0x03,
    E_HAL_GOP_GWIN_SRC_ARGB4444   = 0x04,
    E_HAL_GOP_GWIN_SRC_RGB1555    = 0x05,
    E_HAL_GOP_GWIN_SRC_RGB565     = 0x06,
    E_HAL_GOP_GWIN_SRC_2266       = 0x07,
    E_HAL_GOP_GWIN_SRC_ARGB8888   = 0x08,
    E_HAL_GOP_GWIN_SRC_ABGR8888   = 0x09,
    E_HAL_GOP_GWIN_SRC_UV7Y8      = 0x0A,
    E_HAL_GOP_GWIN_SRC_UV8Y8      = 0x0B,
    E_HAL_GOP_GWIN_SRC_RGBA5551   = 0x0C,
    E_HAL_GOP_GWIN_SRC_RGBA4444   = 0x0D,
    E_HAL_GOP_GWIN_SRC_NOTSUPPORT   = 0xFF,
} HalGopGwinSrcFormat_e;


typedef enum
{
    //GOP0 src color enum
    E_HAL_GOP00_GWIN_SRC_I8_PALETTE = 0,
    E_HAL_GOP00_GWIN_SRC_I4_PALETTE = 0x01,
    E_HAL_GOP00_GWIN_SRC_I2_PALETTE = 0x02,
    E_HAL_GOP00_GWIN_SRC_ARGB1555   = 0x03,
    E_HAL_GOP00_GWIN_SRC_ARGB4444   = 0x04,
    //GOP1 src color enum
    E_HAL_GOP10_GWIN_SRC_RGB1555    = 0x00,
    E_HAL_GOP10_GWIN_SRC_RGB565     = 0x01,
    E_HAL_GOP10_GWIN_SRC_ARGB4444   = 0x02,
    E_HAL_GOP10_GWIN_SRC_2266       = 0x03,
    E_HAL_GOP10_GWIN_SRC_I8_PALETTE = 0x04,
    E_HAL_GOP10_GWIN_SRC_ARGB8888   = 0x05,
    E_HAL_GOP10_GWIN_SRC_ARGB1555   = 0x06,
    E_HAL_GOP10_GWIN_SRC_ABGR8888   = 0x07,
    E_HAL_GOP10_GWIN_SRC_UV7Y8      = 0x08,
    E_HAL_GOP10_GWIN_SRC_UV8Y8      = 0x09,
    E_HAL_GOP10_GWIN_SRC_RGBA5551   = 0x0A,
    E_HAL_GOP10_GWIN_SRC_RGBA4444   = 0x0B,
    E_HAL_GOP10_GWIN_SRC_I4_PALETTE = 0x0D,
    E_HAL_GOP10_GWIN_SRC_I2_PALETTE = 0x0E,
    E_HAL_GOP_GWIN_SRC_VAL_NOTSUPPORT   = 0xFF,
} HalGopGwinSrcFormatRealVal_e;


typedef enum
{
    E_HAL_GOP_ALLBANK_DOUBLE_WR = GOP_BANK_ALLBANK_DOUBLE_WR,
    E_HAL_GOP_FORCE_WR = GOP_BANK_FORCE_WR,
    E_HAL_GOP_ONEBANK_DOUBLE_WR = GOP_BANK_ONEBANK_DOUBLE_WR,
} HalGopWrRegType_e;

typedef enum
{
    E_HAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0 = 0,
    E_HAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1 = 1,
} HalGopGwinArgb1555Def_e;
typedef enum
{
    E_HAL_GOP_GWIN_ALPHA_DEFINE_ALPHA0 = 0,
    E_HAL_GOP_GWIN_ALPHA_DEFINE_ALPHA1 = 1,
    E_HAL_GOP_GWIN_ALPHA_DEFINE_CONST = 2,
} HalGopGwinAlphaDef_e;
typedef enum
{
    E_HAL_GOP_SHADOW_h11 = 0,
    E_HAL_GOP_SHADOW_h12 = 1,
    E_HAL_GOP_SHADOW_h20 = 2,
    E_HAL_GOP_SHADOW_h24 = 3,
    E_HAL_GOP_SHADOW_h28 = 4,
    E_HAL_GOP_SHADOW_Num = 5,
}HalGopShadow_e;
typedef enum
{
    E_HAL_GWIN_SHADOW_h0 = 0,
    E_HAL_GWIN_SHADOW_h8 = 1,
    E_HAL_GWIN_SHADOW_Num = 2,
}HalGwinShadow_e;

//=============================================================================
// struct
//=============================================================================
typedef struct
{
    u16 u16X;
    u16 u16Y;
    u16 u16Width;
    u16 u16Height;
} HalGopWindowType_t;
typedef struct
{
    u8 u8ConstantAlpahValue;
    u8 u8Argb1555Alpha0Val; // Be use when register alpha 0 not equal register constant alpha.
    u8 u8Argb1555Alpha1Val;
}HalGopGwinAlphaVal_t;
typedef struct
{
    HalGopDisplayModeType_e eDisplayMode;
    HalGopOutFormatType_e eOutFormat;
    HalGopWindowType_t tStretchWindow;
} HalGopParamConfig_t;

typedef struct
{
    HalGopGwinSrcFormat_e eSrcFmt;
    HalGopWindowType_t tDispWindow;
    u32 u32BaseAddr;
    u16 u16Base_XOffset;
    u32 u32Base_YOffset;
} HalGopGwinParamConfig_t;
typedef struct
{
    u8 u8Idx;
    u8 u8A;
    u8 u8R;
    u8 u8G;
    u8 u8B;
} HalGopPaletteConfig_t;
typedef struct
{
    bool bEn;
    u8 u8R;
    u8 u8G;
    u8 u8B;
} HalGopColorKeyConfig_t;
typedef struct
{
    HalGopIdType_e eGopId;
    HalGopGwinIdType_e eGwinId;
    u32 u32Reg;
    u32 u16Val;
    u32 u16Msk;
    u32 u32RegType;
    HalRgnCmdqIdType_e eCmdqId;
} HalGopShadowConfig_t;

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
} HalGopVideoTimingInfoConfig_t;

//=============================================================================

//=============================================================================
#ifndef __HAL_GOP_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif
INTERFACE void HalGopGetClkNum(HalGopIdType_e eGopId, u32 *u32clknum);
INTERFACE void HalGopSetAlphaPointVal(HalGopGwinAlphaDef_e enType,HalGopGwinAlphaVal_t *tAlphaVal,u8 **p8Alpha);
INTERFACE bool HalGopSetClkEnable(HalGopIdType_e eGopId);
INTERFACE bool HalGopSetClkDisable(HalGopIdType_e eGopId,bool bEn);
INTERFACE void HalGopMiuSelect(HalGopIdType_e eGopId, u32 *u32BaseAddr);
INTERFACE void HalGopUpdateBaseXoffset(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, u16 u16Xoffset);
INTERFACE void HalGopUpdateBase(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, u32 u32BaseAddr);
INTERFACE void HalGopSetGwinMemPitchDirect(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, u32 u32MemPitch);
INTERFACE void HalGopUpdateParam(HalGopIdType_e eGopId, HalGopParamConfig_t *ptParamCfg);
INTERFACE bool HalGopCheckIdSupport(HalGopIdType_e eGopId);
INTERFACE void HalGopUpdateGwinParam(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId,HalGopGwinParamConfig_t *ptParamCfg);
INTERFACE void HalGopSetStretchWindowSize(HalGopIdType_e eGopId, HalGopWindowType_t *ptSrcWinCfg, HalGopWindowType_t *ptDstWinCfg);
INTERFACE void HalGopSetGwinSize(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, HalGopWindowType_t *ptGwinCfg, HalGopGwinSrcFormat_e eSrcFmt);
INTERFACE void HalGopSetGwinSrcFmt(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, HalGopGwinSrcFormat_e eSrcFmt);
INTERFACE void HalGopSetAlphaBlending(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, bool bConstantAlpha, u8 Alpha);
INTERFACE void HalGopSetColorKey(HalGopIdType_e eGopId, HalGopColorKeyConfig_t *ptCfg, bool bVYU);
INTERFACE void HalGopSetPaletteRiu(HalGopIdType_e eGopId, HalGopPaletteConfig_t *ptCfg);
INTERFACE void HalGopSetEnableGwin(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, bool bEn);
INTERFACE void HalGopBindOsd(HalGopIdType_e eGopId);
INTERFACE void HalGopSetArgb1555Alpha(HalGopIdType_e eGopId, HalGopGwinIdType_e eGwinId, HalGopGwinArgb1555Def_e eAlphaType, u8 eAlphaVal);
INTERFACE void HalGopSetAlphaInvert(HalGopIdType_e eGopId, bool bInv);
INTERFACE bool HalGopCheckAlphaZeroOpaque(HalGopIdType_e eGopId,bool bEn,bool bConAlpha,HalGopGwinSrcFormat_e eFmt);
INTERFACE void HalGopSetVideoTimingInfo(HalGopIdType_e eGopId, HalGopVideoTimingInfoConfig_t *ptVideoTimingCfg);
#undef INTERFACE
#endif /* __HAL_GOP_H__ */
