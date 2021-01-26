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

#ifndef _MHAL_RGN_
#define _MHAL_RGN_

#include "mhal_common.h"
#include "mhal_cmdq.h"
#include "mhal_rgn_datatype.h"

/*Setup cmdq*/
MS_S32 MHAL_RGN_SetupCmdQ(MHAL_CMDQ_CmdqInterface_t* pstCmdInf,MHAL_RGN_CmdqType_e eRgnCmdqId);
/*Cover init*/
MS_S32 MHAL_RGN_CoverInit(void);
MS_S32 MHAL_RGN_CoverDeinit(void);

/*Set cover color NA:[31~24] Cr:[23~16] Y:[15~8] Cb:[7~0]*/
MS_S32 MHAL_RGN_CoverSetColor(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer, MS_U32 u32Color);
/*Cover  size*/
MS_S32 MHAL_RGN_CoverSetWindow(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer, MS_U32 u32X, MS_U32 u32Y, MS_U32 u32Width, MS_U32 u32Height);
/*Enable cover*/
MS_S32 MHAL_RGN_CoverEnable(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer);
/*Disable cover*/
MS_S32 MHAL_RGN_CoverDisable(MHAL_RGN_CoverType_e eCoverId, MHAL_RGN_CoverLayerId_e eLayer);

/*Gop init*/
MS_S32 MHAL_RGN_GopInit(void);
MS_S32 MHAL_RGN_GopDeinit(void);
/*Gop Bind Osd (can't dynamic)*/
MS_S32 MHAL_RGN_SetGopBindOsd(MS_U32 eGopNum);
MS_S32 MHAL_RGN_GetGopBindOsd(MS_U32 *eGopNum);

/*Set palette*/
MS_S32 MHAL_RGN_GopSetPalette(MHAL_RGN_GopType_e eGopId, MS_U8 u8Alpha, MS_U8 u8Red, MS_U8 u8Green, MS_U8 u8Blue, MS_U8 u8Idx);
/*Set base gop window size*/
MS_S32 MHAL_RGN_GopSetBaseWindow(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopWindowConfig_t *ptSrcWinCfg, MHAL_RGN_GopWindowConfig_t *ptDstWinCfg);
/* Set mirror flip */
MS_S32 MHAL_RGN_GopSetMirFlip(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopMirFlip_e eMode);
/*Set base gop gwin pix format*/
MS_S32 MHAL_RGN_GopGwinSetPixelFormat(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MHAL_RGN_GopPixelFormat_e eFormat);
/*Set base gop gwin osd window*/
MS_S32 MHAL_RGN_GopGwinSetWindow(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MS_U32 u32Width, MS_U32 u32Height, MS_U32 u32Stride, MS_U32 u32X, MS_U32 u32Y);
/*Set base gop gwin buffer H offset*/
MS_S32 MHAL_RGN_GopGwinSetBufferXoffset(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MS_U16 u16Xoffset);
/*Set base gop gwin osd bufdfer addr*/
MS_S32 MHAL_RGN_GopGwinSetBuffer(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MS_PHYADDR phyAddr);
/*enable Gwin*/
MS_S32 MHAL_RGN_GopGwinEnable(MHAL_RGN_GopType_e eGopId,  MHAL_RGN_GopGwinId_e eGwinId);
/*disable Gwin*/
MS_S32 MHAL_RGN_GopGwinDisable(MHAL_RGN_GopType_e eGopId,  MHAL_RGN_GopGwinId_e eGwinId);
/*Set gop colorkey*/
MS_S32 MHAL_RGN_GopSetColorkey(MHAL_RGN_GopType_e eGopId, MS_BOOL bEn, MS_U8 u8R, MS_U8 u8G, MS_U8 u8B);
/*Set gop alpha blending type*/
MS_S32 MHAL_RGN_GopSetAlphaType(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MHAL_RGN_GopGwinAlphaType_e eAlphaType, MS_U8 u8ConstAlphaVal);
/*Set gop ARGB1555 alpha value for alpha0 or alpha1*/
MS_S32 MHAL_RGN_GopSetArgb1555AlphaVal(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopGwinId_e eGwinId, MHAL_RGN_GopGwinArgb1555Def_e eAlphaType, MS_U8 u8AlphaVal);
/*Set gop color inverse*/
MS_S32 MHAL_RGN_GopSetColorInverseEnable(MHAL_RGN_GopType_e eGopId, MS_BOOL bEn);
MS_S32 MHAL_RGN_GopSetColorInverseParam(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopColorInvWindowIdType_e eAeId, MHAL_RGN_GopColorInvConfig_t tColInvCfg);
MS_S32 MHAL_RGN_GopSetColorInverseUpdate(MHAL_RGN_GopType_e eGopId);
MS_S32 MHAL_RGN_GopWriteColorInverseData(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopColorInvWindowIdType_e eAeId, MS_U16 u16DataLength, MS_U32 *pu32WData);
MS_S32 MHAL_RGN_GopReadColorInverseData(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopColorInvWindowIdType_e eAeId, MS_U16 u16DataLength, MS_U32 *pu32RData);
/*Set gop alpha 0 is opaque or not*/
MS_S32 MHAL_RGN_GopSetAlphaZeroOpaque(MHAL_RGN_GopType_e eGopId, MS_BOOL bEn, MS_BOOL bConstAlpha, MHAL_RGN_GopPixelFormat_e eFormat);
//------------------------------------------------------------------------------
/// @brief: Get Chip Capability
/// @param[in]  eType : reference to MHAL_RGN_ChipCapType_e
///             eFormat: Color format
/// @return corresponding capability to type
//------------------------------------------------------------------------------
MS_S32 MHAL_RGN_GetChipCapability(MHAL_RGN_ChipCapType_e eType, MHAL_RGN_GopPixelFormat_e eFormat);

MS_S32 MHAL_RGN_GopSetVideoTimingInfo(MHAL_RGN_GopType_e eGopId, MHAL_RGN_GopVideoTimingInfoConfig_t *ptTimingCfg);

#endif //_MHAL_RGN_
