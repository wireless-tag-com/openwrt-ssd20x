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

#ifndef _DRV_HDMITX_IF_H_
#define _DRV_HDMITX_IF_H_


#ifdef _DRV_HDMITX_IF_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE MhalHdmitxRet_e DrvHdmitxIfCreateInstance(void **pCtx, u32 u32Id);
INTERFACE MhalHdmitxRet_e DrvHdmitxIfGetInstance(void **pCtx, u32 u32Id);
INTERFACE MhalHdmitxRet_e DrvHdmitxIfDestoryInstance(void *pCtx);
INTERFACE MhalHdmitxRet_e DrvHdmitxIfSetAttrBegin(void *pCtx);
INTERFACE MhalHdmitxRet_e DrvHdmitxIfSetAttr(void *pCtx, MhalHdmitxAttrConfig_t *pstAttrCfg);
INTERFACE MhalHdmitxRet_e DrvHdmitxIfSetAttrEnd(void *pCtx);
INTERFACE MhalHdmitxRet_e DrvHdmitxIfSetMute(void *pCtx, MhalHdmitxMuteConfig_t *pstMuteCfg);
INTERFACE MhalHdmitxRet_e DrvHdmitxIfSetSignal(void *pCtx, MhalHdmitxSignalConfig_t *pstSignalCfg);
INTERFACE MhalHdmitxRet_e DrvHdmitxIfGetSinkInfo(void *pCtx, MhalHdmitxSinkInfoConfig_t *pstSinkInfo);
INTERFACE MhalHdmitxRet_e DrvHdmitxIfSetInfoFrame(void *pCtx, MhalHdmitxInfoFrameConfig_t *pstInfoFrameCfg);
INTERFACE MhalHdmitxRet_e DrvHdmitxIfSetAnalogDrvCur(void *pCtx, MhalHdmitxAnaloDrvCurConfig_t *pstDrvCurCfg);
INTERFACE MhalHdmitxRet_e DrvHdmitxIfSetDebugLevel(void *pCtx, u32 u32DbgLevel);
INTERFACE MhalHdmitxRet_e DrvHdmitxIfSetHpdConfig(void *pCtx, MhalHdmitxHpdConfig_t *pstHpdCfg);
INTERFACE MhalHdmitxRet_e DrvHdmitxIfSetClk(void *pCtx, bool *pbEn, u32 *pu32ClkRate, u32 u32ClkNum);
INTERFACE MhalHdmitxRet_e DrvHdmitxIfGetClk(void *pCtx, bool *pbEn, u32 *pu32ClkRate, u32 u32ClkNum);

#endif
