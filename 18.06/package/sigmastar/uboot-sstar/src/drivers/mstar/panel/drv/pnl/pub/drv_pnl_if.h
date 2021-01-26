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

#ifndef _DRV_PNL_IF_H_
#define _DRV_PNL_IF_H_


#ifdef _DRV_PNL_IF_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif


INTERFACE bool DrvPnlIfCreateInstance(void **pCtx, MhalPnlLinkType_e enLinkType, u16 u16Id);
INTERFACE bool DrvPnlIfDestroyInstance(void *pCtx);
INTERFACE bool DrvPnlIfGetInstance(void **pCtx, MhalPnlLinkType_e enLinkType, u16 u16Id);
INTERFACE bool DrvPnlIfSetParamConfig(void *pCtx, MhalPnlParamConfig_t *pstParamCfg);
INTERFACE bool DrvPnlIfGetParamConfig(void *pCtx, MhalPnlParamConfig_t *pstParamCfg);
INTERFACE bool DrvPnlIfSetMipiDsiConfig(void *pCtx, MhalPnlMipiDsiConfig_t *pstMipiDsiCfg);
INTERFACE bool DrvPnlIfGetMipiDsiConfig(void *pCtx, MhalPnlMipiDsiConfig_t *pstMipiDsiCfg);
INTERFACE bool DrvPnlIfSetSscConfig(void *pCtx, MhalPnlSscConfig_t *pstSscCfg);
INTERFACE bool DrvPnlIfSetTimingConfig(void *pCtx, MhalPnlTimingConfig_t *pstTimingCfg);
INTERFACE bool DrvPnlIfGetTimingConfig(void *pCtx, MhalPnlTimingConfig_t *pstTimingCfg);
INTERFACE bool DrvPnlIfSetPowerConfig(void *pCtx, MhalPnlPowerConfig_t *pstPowerCfg);
INTERFACE bool DrvPnlIfGetPowerConfig(void *pCtx, MhalPnlPowerConfig_t *pstPowerCfg);
INTERFACE bool DrvPnlIfSetBackLightOnOffConfig(void *pCtx, MhalPnlBackLightOnOffConfig_t *pstBackLightOnOffCfg);
INTERFACE bool DrvPnlIfGetBackLightOnOffConfig(void *pCtx, MhalPnlBackLightOnOffConfig_t *pstBackLightOnOffCfg);
INTERFACE bool DrvPnlIfSetBackLightLevelConfig(void *pCtx, MhalPnlBackLightLevelConfig_t *pstBackLightLevelCfg);
INTERFACE bool DrvPnlIfGetBackLightLevelConfig(void *pCtx, MhalPnlBackLightLevelConfig_t *pstBackLightLevelCfg);
INTERFACE bool DrvPnlIfSetDrvCurrentConfig(void *pCtx, MhalPnlDrvCurrentConfig_t *pstCurrentCfg);
INTERFACE bool DrvPnlIfSetTestPatternConfig(void *pCtx, MhalPnlTestPatternConfig_t *pstTestPatternCfg);
INTERFACE bool DrvPnlIfSetDbgLevel(void *pDbgLevel);
INTERFACE bool DrvPnlIfSetClkConfig(void *pCtx, bool *pbEn, u32 *pu32ClkRate, u32 u32ClkNum);
INTERFACE bool DrvPnlIfGetClkConfig(void *pCtx, bool *pbEn, u32 *pu32ClkRate, u32 u32ClkNum);

#undef INTERFACE
#endif
