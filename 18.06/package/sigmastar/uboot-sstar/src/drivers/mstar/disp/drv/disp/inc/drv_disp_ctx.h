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

#ifndef _DRV_DISP_CTX_H_
#define _DRV_DISP_CTX_H_


//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_CTX_INVAILD_IDX     0xFFFF
//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_DISP_CTX_TYPE_DEVICE,
    E_DISP_CTX_TYPE_VIDLAYER,
    E_DISP_CTX_TYPE_INPUTPORT,
    E_DISP_CTX_TYPE_MAX,
}DrvDispCtxType_e;

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef s32 (*pDispCtxMemAlloc)(u8 *pu8Name, u32 size, u64 *pu64PhyAddr);
typedef s32 (*pDispCtxMemFree)(u64 u64PhyAddr);

typedef struct
{
    pDispCtxMemAlloc alloc;
    // Success return 0
    pDispCtxMemFree free;
}DrvDispCtxMemAllocConfig_t;

typedef struct
{
    bool bUsed;
    u32 u32PortId;
    bool bEnable;
    bool bDisplay;
    bool bBeginEnd;
    HalDispVideoFrameData_t stFrameData;
    HalDispInputPortAttr_t stAttr;
    HalDispVidWinRect_t stCrop;
    HalDispInputPortRotate_t stRot;
    void *pstVidLayerContain;
}DrvDispCtxInputPortContain_t;

typedef struct
{
    u32 u32VidLayerId;
    void *pstDevCtx;
    bool bEnable;
    HalDispVideoLayerAttr_t stAttr;
    HalDispVideoLayerCompressAttr_t stCompress;
    u32 u32Priority;
    DrvDispCtxInputPortContain_t *pstInputPortContain[HAL_DISP_INPUTPORT_NUM];
}DrvDispCtxVideoLayerContain_t;


typedef struct
{
    u32  u32DevId;
    bool bEnable;
    u32  u32BgColor;
    u32  u32BindVideoLayerNum;
    u32  u32Interface;
    HalDispDeviceTimingConfig_t stDevTimingCfg;
    HalDispCvbsParam_t stCvbsParam;
    HalDispHdmiParam_t stHdmiParam;
    HalDispVgaParam_t  stVgaParam;
    HalDispLcdParam_t  stLcdParam;
    HalDispGammaParam_t stGammaParam;
    HalDispColorTemp_t stColorTemp;
    DrvDispCtxVideoLayerContain_t *pstVidLayeCtx[HAL_DISP_VIDLAYER_MAX];
    void *pstDevContain;
}DrvDispCtxDeviceContain_t;


typedef struct
{
    HalDispHwDmaConfig_t stDamCfg;
}DrvDispCtxHwContain_t;

typedef struct
{
    bool bDevUsed[HAL_DISP_DEVICE_MAX];
    bool bVidLayerUsed[HAL_DISP_VIDLAYER_MAX];
    bool bInputPortUsed[HAL_DISP_INPUTPORT_MAX];
    DrvDispCtxDeviceContain_t *pstDevContain[HAL_DISP_DEVICE_MAX];
    DrvDispCtxVideoLayerContain_t *pstVidLayerContain[HAL_DISP_VIDLAYER_MAX];
    DrvDispCtxInputPortContain_t *pstInputPortContain[HAL_DISP_INPUTPORT_MAX];
    DrvDispCtxHwContain_t  *pstHalHwCtx;
    DrvDispCtxMemAllocConfig_t stMemAllcCfg;
}DrvDispCtxContain_t;

typedef struct
{
    DrvDispCtxType_e enCtxType;
    u32 u32Idx;
    DrvDispCtxContain_t *pstCtxContain;
}DrvDispCtxConfig_t;

typedef struct
{
    DrvDispCtxType_e enType;
    u32 u32Id;
    DrvDispCtxConfig_t *pstBindCtx;
    DrvDispCtxMemAllocConfig_t stMemAllcCfg;
}DrvDispCtxAllocConfig_t;
//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------
#ifndef _DRV_DISP_CTX_C_
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE bool DrvDispCtxInit(void);
INTERFACE bool DrvDispCtxDeInit(void);
INTERFACE bool DrvDispCtxAllocate(DrvDispCtxAllocConfig_t *pAllocCfg, DrvDispCtxConfig_t **pCtx);
INTERFACE bool DrvDispCtxFree(DrvDispCtxConfig_t *pCtx);
INTERFACE bool DrvDispCtxIsAllFree(void);
INTERFACE bool DrvDispCtxSetCurCtx(DrvDispCtxConfig_t *pCtx, u32 u32Idx);
INTERFACE bool DrvDispCtxGetCurCtx(DrvDispCtxType_e enCtxType, u32 u32Idx, DrvDispCtxConfig_t **pCtx);

#undef INTERFACE

#endif
