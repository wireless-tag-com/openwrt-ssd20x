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

#ifndef _HAL_DISP_MOP_H_
#define _HAL_DISP_MOP_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------

typedef enum
{
    E_HAL_DISP_MOPG_ID_00     = 0,
    E_HAL_DISP_MOPG_ID_01     = 1,
    E_HAL_DISP_MOPG_ID_02     = 2,
    E_HAL_DISP_MOPG_ID_03     = 3,
    E_HAL_DISP_MOPG_ID_04     = 4,
    E_HAL_DISP_MOPG_ID_05     = 5,
    E_HAL_DISP_MOPG_ID_06     = 6,
    E_HAL_DISP_MOPG_ID_07     = 7,
    E_HAL_DISP_MOPG_ID_08     = 8,
    E_HAL_DISP_MOPG_ID_09     = 9,
    E_HAL_DISP_MOPG_ID_10     = 10,
    E_HAL_DISP_MOPG_ID_11     = 11,
    E_HAL_DISP_MOPG_ID_12     = 12,
    E_HAL_DISP_MOPG_ID_13     = 13,
    E_HAL_DISP_MOPG_ID_14     = 14,
    E_HAL_DISP_MOPG_ID_15     = 15,
    E_HAL_DISP_MOPG_ID_NUM    = 16,
    E_HAL_DISP_MOPS_ID_00     = 0,
} HalDispMopgGwinId_e;

typedef enum
{
    E_HAL_DISP_LAYER_MOPG_ID     = 0,
    E_HAL_DISP_LAYER_MOPS_ID     = 1,
    E_HAL_DISP_LAYER_ID_NUM      = 2,
} HalDispVideoLayerId_e;

typedef enum
{
    E_HAL_DISP_MOPROT_ROT0_ID     = 0,
    E_HAL_DISP_MOPROT_ROT1_ID     = 1,
    E_HAL_DISP_MOPROT_NUM      = 2,
} HalDispMopRotId_e;

typedef enum
{
    E_HAL_DISP_LAYER_MOPG01_01          = 0,
    E_HAL_DISP_LAYER_MOPG01_11          = 1,
    E_HAL_DISP_LAYER_MOPG01_21          = 2,
    E_HAL_DISP_LAYER_MOPG01_31          = 3,
    E_HAL_DISP_LAYER_MOPG01_41          = 4,
    E_HAL_DISP_LAYER_MOPG01_51          = 5,
    E_HAL_DISP_LAYER_MOPG01_61          = 6,
    E_HAL_DISP_LAYER_MOPG01_71          = 7,
    E_HAL_DISP_LAYER_MOPG01_SHADOW_NUM  = 8,
} HalDispMopg01SwShadow_e;

typedef enum
{
    E_HAL_DISP_LAYER_MOPG02_01          = 0,
    E_HAL_DISP_LAYER_MOPG02_11          = 1,
    E_HAL_DISP_LAYER_MOPG02_21          = 2,
    E_HAL_DISP_LAYER_MOPG02_31          = 3,
    E_HAL_DISP_LAYER_MOPG02_41          = 4,
    E_HAL_DISP_LAYER_MOPG02_51          = 5,
    E_HAL_DISP_LAYER_MOPG02_61          = 6,
    E_HAL_DISP_LAYER_MOPG02_71          = 7,
    E_HAL_DISP_LAYER_MOPG02_SHADOW_NUM  = 8,
} HalDispMopg02SwShadow_e;


typedef enum
{
    E_HAL_DISP_LAYER_MOPS_51          = 0,
    E_HAL_DISP_LAYER_MOPS_SHADOW_NUM  = 1,
} HalDispMopsSwShadow_e;


//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifdef _HAL_DISP_MOP_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif
//SW shadow
INTERFACE void HalMopsShadowInit(void);

//MOPG settings
INTERFACE void HalDispMopgSwReset(void);
INTERFACE void HalDispMopgSetAutoStretchWinSizeEn(bool bEnAuto);
INTERFACE void HalDispMopgSetStretchWinSize(u16 u16Width, u16 u16Height);
INTERFACE void HalDispMopgSetPipeDelay(u8 u8PipeDelay);
INTERFACE void HalDispMopgInit(void);
INTERFACE void HalDispMopgGwinEn(HalDispMopgGwinId_e eMopgId, bool bEn);
INTERFACE void HalDispMopgSetYAddr(HalDispMopgGwinId_e eMopgId, u32 u32PhyAddr);
INTERFACE void HalDispMopgSetAddr16Offset(HalDispMopgGwinId_e eMopgId, u8 u8Offset);
INTERFACE void HalDispMopgSetCAddr(HalDispMopgGwinId_e eMopgId, u32 u32PhyAddr);
INTERFACE void HalDispMopgSetGwinParam(HalDispMopgGwinId_e eMopgId, u16 u16Hst, u16 u16Vst, u16 u16Width, u16 u16Height);
INTERFACE void HalDispMopgSetPitch(HalDispMopgGwinId_e eMopgId, u16 u16Pitch);
INTERFACE void HalDispMopgSetSourceParam(HalDispMopgGwinId_e eMopgId, u16 u16SrcWidth, u16 u16SrcHeight);
INTERFACE void HalDispMopgSetHScaleFac(HalDispMopgGwinId_e eMopgId, u16 u16HRatio);
INTERFACE void HalDispMopgSetVScaleFac(HalDispMopgGwinId_e eMopgId, u16 u16VRatio);
INTERFACE void HalDispMopgSetRblkHstr(HalDispMopgGwinId_e eMopgId, u8 u8Hshift);
INTERFACE void HalDispMopgSetLineBufStr(HalDispMopgGwinId_e eMopgId, u16 u16GwinHstr, u8 u8WinPri);

//MOPS settings
INTERFACE void HalDispMopsSwReset(void);
INTERFACE void HalDispMopsSetAutoStretchWinSizeEn(bool bEnAuto);
INTERFACE void HalDispMopsSetStretchWinSize(u16 u16Width, u16 u16Height);
INTERFACE void HalDispMopsSetPipeDelay(u8 u8PipeDelay);
INTERFACE void HalDispMopsInit(void);
INTERFACE void HalDispMopsGwinEn(bool bEn);
INTERFACE void HalDispMopsSetYAddr(u32 u32PhyAddr);
INTERFACE void HalDispMopsSetAddr16Offset(u8 u8Offset);
INTERFACE void HalDispMopsSetCAddr(u32 u32PhyAddr);
INTERFACE void HalDispMopsSetGwinParam(u16 u16Hst, u16 u16Vst, u16 u16Width, u16 u16Height);
INTERFACE void HalDispMopsSetPitch(u16 u16Pitch);
INTERFACE void HalDispMopsSetSourceParam(u16 u16SrcWidth, u16 u16SrcHeight);
INTERFACE void HalDispMopsSetHScaleFac(u16 u16HRatio);
INTERFACE void HalDispMopsSetVScaleFac(u16 u16VRatio);
INTERFACE void HalDispMopsSetRblkHstr(u8 u8Hshift);

//MOP common
INTERFACE void HalDispMopDbBfWr(void);
INTERFACE void HalDispMopSetClk(bool bEn, u32 u32ClkRate);
INTERFACE void HalDispMopGetClk(bool *pbEn, u32 *pu32ClkRate);

//MOP Rotate
INTERFACE void HalDispMopRotDbBfWr(HalDispMopRotId_e eMopRotId);
INTERFACE void HalDispMopRot0DbfEn(bool bEn);
INTERFACE void HalDispMopRot1DbfEn(bool bEn);
INTERFACE void HalDispMopRot0En(bool bEn);
INTERFACE void HalDispMopRot1En(bool bEn);
INTERFACE void HalDispMopRot0SourceWidth(u16 u16inputWidth);
INTERFACE void HalDispMopRot0SourceHeight(u16 u16inputHeight);
INTERFACE void HalDispMopRot1SourceWidth(u16 u16inputWidth);
INTERFACE void HalDispMopRot1SourceHeight(u16 u16inputHeight);
INTERFACE void HalDispMopRot0SetReadYAddr(u32 u32PhyAddr);
INTERFACE void HalDispMopRot0SetReadCAddr(u32 u32PhyAddr);
INTERFACE void HalDispMopRot1SetReadYAddr(u32 u32PhyAddr);
INTERFACE void HalDispMopRot1SetReadCAddr(u32 u32PhyAddr);
INTERFACE void HalDispMopRot0SetWriteYAddr(u32 u32PhyAddr);
INTERFACE void HalDispMopRot0SetWriteCAddr(u32 u32PhyAddr);
INTERFACE void HalDispMopRot1SetWriteYAddr(u32 u32PhyAddr);
INTERFACE void HalDispMopRot1SetWriteCAddr(u32 u32PhyAddr);
INTERFACE void HalDispMopRot0SetRotateMode(HalDispRotateMode_e eRotAng);
INTERFACE void HalDispMopRot1SetRotateMode(HalDispRotateMode_e eRotAng);
INTERFACE void HalDispMopRot0SetPixDummy(u16 u16DummyPix);
INTERFACE void HalDispMopRot1SetPixDummy(u16 u16DummyPix);
INTERFACE void HalDispMopRotInit(void);


#undef INTERFACE
#endif
