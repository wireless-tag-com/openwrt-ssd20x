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

#ifndef _DRV_HDMITX_CTX_H_
#define _DRV_HDMITX_CTX_H_


//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------


typedef struct
{
    bool bInit;
    bool bAvMute;
    bool bVideoOnOff;
    bool bAudioOnOff;
    bool bSignal;
    HalHdmitxStatusFlag_e enStatusFlag;
    HalHdmitxHpdConfig_t stHpdCfg;
    HalHdmitxAnaloDrvCurConfig_t stAnalogDrvCurCfg;
    HalHdmitxAttrConfig_t stAttrCfg;
    HalHdmitxInfoFrameConfig_t stInfoFrameCfg;
}DrvHdmitxCtxHalContain_t;

typedef struct
{
    bool bUsed;
    s32  s32CtxId;
    DrvHdmitxCtxHalContain_t *pstHalCtx;
}DrvHdmitxCtxConfig_t;

typedef struct
{
    s32  s32CtxId;
}DrvHdmitxCtxAllocConfig_t;

//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------
#ifndef _DRV_HDMITX_CTX_C_
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE bool DrvHdmitxCtxInit(void);
INTERFACE bool DrvHdmitxCtxDeInit(void);
INTERFACE bool DrvHdmitxCtxAllocate(DrvHdmitxCtxAllocConfig_t *pAllocCfg, DrvHdmitxCtxConfig_t **pCtx);
INTERFACE bool DrvHdmitxCtxFree(DrvHdmitxCtxConfig_t *pCtx);
INTERFACE bool DrvHdmitxCtxIsAllFree(void);
INTERFACE bool DrvHdmitxCtxGet(DrvHdmitxCtxAllocConfig_t *pAllocCfg, DrvHdmitxCtxConfig_t **pCtx);



#undef INTERFACE

#endif
