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

#ifndef _DRV_PNL_CTX_H_
#define _DRV_PNL_CTX_H_


//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_PNL_CTX_MAX_INST    2

//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_PNL_CTX_FLAG_NONE            = 0x00000000,
    E_HAL_PNL_CTX_FLAG_PARAM           = 0x00000001,
    E_HAL_PNL_CTX_FLAG_MIPIDSI         = 0x00000002,
    E_HAL_PNL_CTX_FLAG_SSC             = 0x00000004,
    E_HAL_PNL_CTX_FLAG_TIMING          = 0x00000008,
    E_HAL_PNL_CTX_FLAG_POWER           = 0x00000010,
    E_HAL_PNL_CTX_FLAG_BACKLIGHT_ONOFF = 0x00000020,
    E_HAL_PNL_CTX_FLAG_BACKLIGHT_LEVEL = 0x00000040,
    E_HAL_PNL_CTX_FLAG_CURRENT         = 0x00000080,
    E_HAL_PNL_CTX_FLAG_TEST_PAT        = 0x00000100,
}DrvPnlCtxHalContainFlag_e;


typedef struct
{
    DrvPnlCtxHalContainFlag_e enFlag;
    HalPnlLinkType_e enLinkType;
    HalPnlParamConfig_t stParamCfg;
    HalPnlMipiDsiConfig_t stMipiDisCfg;
    HalPnlSscConfig_t stSscCfg;
    HalPnlTimingConfig_t stTimingCfg;
    HalPnlPowerConfig_t stPowerCfg;
    HalPnlBackLightOnOffConfig_t stBackLightOnOffCfg;
    HalPnlBackLightLevelConfig_t stBackLightLevelCfg;
    HalPnlCurrentConfig_t stCurrentCfg;
    HalPnlTestPatternConfig_t stTestPatCfg;
}DrvPnlCtxHalContain_t;


typedef struct
{
    HalPnlHwTimeGenConfig_t stTimeGenCfg;
    HalPnlHwLpllConfig_t stLplLConfig;
    HalPnlHwPolarityConfig_t stPolarityConfig;
    HalPnlHwMipiDsiConfig_t stMipiDsiCfg;
}DrvPnlCtxHwContain_t;

typedef struct
{
    bool bUsed;
    u16  u16Id;
    s16  s16CtxId;
    DrvPnlCtxHalContain_t *pstHalCtx;
    DrvPnlCtxHwContain_t *pstHwCtx;
}DrvPnlCtxConfig_t;

typedef struct
{
    s16  s16CtxId;
}DrvPnlCtxAllocConfig_t;

//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------
#ifndef _DRV_PNL_CTX_C_
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE bool DrvPnlCtxInit(void);
INTERFACE bool DrvPnlCtxDeInit(void);
INTERFACE bool DrvPnlCtxAllocate(DrvPnlCtxAllocConfig_t *pAllocCfg, DrvPnlCtxConfig_t **pCtx);
INTERFACE bool DrvPnlCtxFree(DrvPnlCtxConfig_t *pCtx);
INTERFACE bool DrvPnlCtxIsAllFree(void);
INTERFACE bool DrvPnlCtxGet(DrvPnlCtxAllocConfig_t *pAllocCfg, DrvPnlCtxConfig_t **pCtx);


#undef INTERFACE

#endif
