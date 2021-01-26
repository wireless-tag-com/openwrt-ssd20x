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

#ifndef _MHAL_RGN_DATATYPE_H_
#define _MHAL_RGN_DATATYPE_H_
#include "mhal_common.h"

typedef enum
{
    E_MHAL_COVER_VPE_PORT0 = 0,
    E_MHAL_COVER_VPE_PORT1,
    E_MHAL_COVER_VPE_PORT2,
    E_MHAL_COVER_VPE_PORT3,
    E_MHAL_COVER_DIVP_PORT0,
    E_MHAL_COVER_TYPE_MAX,
}MHAL_RGN_CoverType_e;

typedef enum
{
    E_MHAL_COVER_LAYER_0 = 0,
    E_MHAL_COVER_LAYER_1,
    E_MHAL_COVER_LAYER_2,
    E_MHAL_COVER_LAYER_3,
    E_MHAL_COVER_LAYER_MAX
}MHAL_RGN_CoverLayerId_e;

typedef enum
{
    E_MHAL_GOP_VPE_PORT0 = 0,
    E_MHAL_GOP_VPE_PORT1,
    E_MHAL_GOP_VPE_PORT2, // I5 not support
    E_MHAL_GOP_VPE_PORT3,
    E_MHAL_GOP_DIVP_PORT0,
    E_MHAL_GOP_LDC_PORT0,
    E_MHAL_GOP_TYPE_MAX
}MHAL_RGN_GopType_e;
typedef enum
{
    E_MHAL_OSD_FLAG_PORT0 = 0x1,
    E_MHAL_OSD_FLAG_PORT1 = 0x2,
    E_MHAL_OSD_FLAG_PORT2 = 0x4,
    E_MHAL_OSD_FLAG_PORT3 = 0x8,
    E_MHAL_OSD_FLAG_DIP   = 0x10,
    E_MHAL_OSD_FLAG_LDC   = 0x20,
}MHAL_RGN_GopOsdFlag_e;

typedef enum
{
    E_MHAL_RGN_VPE_CMDQ = 0,
    E_MHAL_RGN_DIVP_CMDQ,
    E_MHAL_RGN_LDC_CMDQ,
    E_MHAL_RGN_CMDQ_TYPE_MAX
}MHAL_RGN_CmdqType_e;

//----------------------------------------------------------------------------
// GWin
//------------------------------------------------------------------------------
typedef enum
{
    E_MHAL_GOP_GWIN_ID_0 = 0,
    E_MHAL_GOP_GWIN_ID_1,
    E_MHAL_GOP_GWIN_ID_2,
    E_MHAL_GOP_GWIN_ID_3,
    E_MHAL_GOP_GWIN_ID_4,
    E_MHAL_GOP_GWIN_ID_5,
    E_MHAL_GOP_GWIN_ID_6,
    E_MHAL_GOP_GWIN_ID_7,
    E_MHAL_GOP_GWIN_ID_MAX
}MHAL_RGN_GopGwinId_e;

typedef enum
{
    E_MHAL_GOP_GWIN_ALPHA_CONSTANT,
    E_MHAL_GOP_GWIN_ALPHA_PIXEL,
    E_MHAL_GOP_GWIN_ALPHA_NUM,
} MHAL_RGN_GopGwinAlphaType_e;

typedef enum
{
    E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA0 = 0,
    E_MHAL_GOP_GWIN_ARGB1555_DEFINE_ALPHA1 = 1,
} MHAL_RGN_GopGwinArgb1555Def_e;

//=============================================================================
// struct
//=============================================================================

typedef enum
{
    E_MHAL_RGN_PIXEL_FORMAT_ARGB1555 = 0,
    E_MHAL_RGN_PIXEL_FORMAT_ARGB4444,
    E_MHAL_RGN_PIXEL_FORMAT_I2,
    E_MHAL_RGN_PIXEL_FORMAT_I4,
	E_MHAL_RGN_PIXEL_FORMAT_I8,
	E_MHAL_RGN_PIXEL_FORMAT_RGB565,
	E_MHAL_RGN_PIXEL_FORMAT_ARGB8888,
	E_MHAL_RGN_PIXEL_FORMAT_UV8Y8,
    E_MHAL_RGN_PIXEL_FORMAT_MAX
}MHAL_RGN_GopPixelFormat_e;

typedef enum 
{
    E_MHAL_RGN_MIRFLIP_NONE,
    E_MHAL_RGN_MIRFLIP_MIRROR,
    E_MHAL_RGN_MIRFLIP_FLIP,
    E_MHAL_RGN_MIRFLIP_MIRROR_FLIP,
    E_MHAL_RGN_MIRFLIP_MAX
}MHAL_RGN_GopMirFlip_e;

typedef enum 
{
	E_MHAL_RGN_CHIP_OSD_FMT_SUPPORT,
	E_MHAL_RGN_CHIP_OSD_OVERLAP_SUPPORT,
	E_MHAL_RGN_CHIP_OSD_XPOS_OVERLAP_SUPPORT,
	E_MHAL_RGN_CHIP_OSD_WIDTH_ALIGNMENT, // pixels
	E_MHAL_RGN_CHIP_OSD_XPOS_ALIGNMENT,  // pixels
	E_MHAL_RGN_CHIP_OSD_PITCH_ALIGNMENT, // bytes
	E_MHAL_RGN_CHIP_OSD_HW_GWIN_CNT,
	E_MHAL_RGN_CHIP_MAX
}MHAL_RGN_ChipCapType_e;

typedef enum
{
    E_MHAL_RGN_GOP_AE0 = 0,
    E_MHAL_RGN_GOP_AE1,
    E_MHAL_RGN_GOP_AE2,
    E_MHAL_RGN_GOP_AE3,
    E_MHAL_RGN_GOP_AE4,
    E_MHAL_RGN_GOP_AE5,
    E_MHAL_RGN_GOP_AE6,
    E_MHAL_RGN_GOP_AE7,
    E_MHAL_RGN_GOP_MAX
} MHAL_RGN_GopColorInvWindowIdType_e;

typedef struct
{
    MS_U16 u16Th1; // the pivot of the range you want to color inverse or not
    MS_U16 u16Th2; // if      abs(Yavg - Th1) < Th2 then do color inverse
    MS_U16 u16Th3; // else if abs(Yavg - Th1) > Th3 then NOT color inverse
    MS_U16 u16X;   // Window offset X
    MS_U16 u16Y;   // Window offset Y
    MS_U16 u16W;   // Window width
    MS_U16 u16H;   // Window height
    MS_U16 u16BlkNumX; // X divide block number
    MS_U16 u16BlkNumY; // Y divide block number
} MHAL_RGN_GopColorInvConfig_t;

typedef struct
{
    MS_U32 u32X;
    MS_U32 u32Y;
    MS_U32 u32Width;
    MS_U32 u32Height;
} MHAL_RGN_GopWindowConfig_t;

typedef struct
{
    MS_U16 u16Htotal;
    MS_U16 u16Vtotal;
    MS_U16 u16HdeStart;
    MS_U16 u16VdeStart;
    MS_U16 u16Width;
    MS_U16 u16Height;
    MS_BOOL bInterlaceMode;
    MS_BOOL bYuvOutput;
} MHAL_RGN_GopVideoTimingInfoConfig_t;

#endif //_MHAL_RGN_DATATYPE_H_
