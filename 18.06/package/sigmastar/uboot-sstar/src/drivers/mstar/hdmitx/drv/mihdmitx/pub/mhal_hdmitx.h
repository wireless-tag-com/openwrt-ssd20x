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
 * \defgroup HAL_HDMITX_group  HAL_HDMITX driver
 * @{
 */
#ifndef __MHAL_HDMITX_H__
#define __MHAL_HDMITX_H__

#include "mhal_hdmitx_datatype.h"
//=============================================================================
// API
//=============================================================================


#ifndef __MHAL_HDMITX_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif

// Create / Get/ Destroy Instance
INTERFACE MhalHdmitxRet_e MhalHdmitxCreateInstance(void **pCtx, u32 u32Id);
INTERFACE MhalHdmitxRet_e MhalHdmitxGetInstance(void **pCtx, u32 u32Id);
INTERFACE MhalHdmitxRet_e MhalHdmitxDestroyInstance(void *pCtx);


// Hdmitx Attr
INTERFACE MhalHdmitxRet_e MhalHdmitxSetAttrBegin(void *pCtx);
INTERFACE MhalHdmitxRet_e MhalHdmitxSetAttr(void *pCtx, MhalHdmitxAttrConfig_t *pstAttrCfg);
INTERFACE MhalHdmitxRet_e MhalHdmitxSetAttrEnd(void *pCtx);


// Mute for Video/Audio/AvMute
INTERFACE MhalHdmitxRet_e MhalHdmitxSetMute(void *pCtx, MhalHdmitxMuteConfig_t *pstMuteCfg);

// Output Signal on/off
INTERFACE MhalHdmitxRet_e MhalHdmitxSetSignal(void *pCtx, MhalHdmitxSignalConfig_t *pstSignalCfg);

// Analog Driving Current
INTERFACE MhalHdmitxRet_e MhalHdmitxSetAnalogDrvCur(void *pCtx, MhalHdmitxAnaloDrvCurConfig_t *pstDrvCurCfg);

// info Frame
INTERFACE MhalHdmitxRet_e MhalHdmitxSetInfoFrame(void *pCtx, MhalHdmitxInfoFrameConfig_t *pstInfoFrameCfg);

// Sink Info
INTERFACE MhalHdmitxRet_e MhalHdmitxGetSinkInfo(void *pCtx, MhalHdmitxSinkInfoConfig_t *pstSinkInfoCfg);

// Debug Level
INTERFACE MhalHdmitxRet_e MhalHdmitxSetDebugLevel(void *pCtx, u32 u32DbgLevel);

// Hpd GpioNum
INTERFACE MhalHdmitxRet_e MhalHdmitxSetHpdConfig(void *pCtx, MhalHdmitxHpdConfig_t *pstHpdCfg);

#endif
