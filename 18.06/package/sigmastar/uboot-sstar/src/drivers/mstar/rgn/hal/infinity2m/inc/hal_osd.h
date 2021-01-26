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


#ifndef __HAL_OSD_H__
#define __HAL_OSD_H__
#include "hal_rgn_common.h"
//=============================================================================
// Defines
//=============================================================================
#ifndef bool
#define  bool   unsigned char
#endif
//=============================================================================
// enum
//=============================================================================
typedef enum
{
    E_HAL_OSD_PORT0,
    E_HAL_OSD_PORT1,
    E_HAL_OSD_ID_MAX
}HalOsdIdType_e;

typedef enum
{
    E_HAL_GOP_AE0 = 0,
    E_HAL_GOP_AE1,
    E_HAL_GOP_AE2,
    E_HAL_GOP_AE3,
    E_HAL_GOP_AE4,
    E_HAL_GOP_AE5,
    E_HAL_GOP_AE6,
    E_HAL_GOP_AE7
} HalOsdColorInvWindowIdType_e;

//=============================================================================
// struct
//=============================================================================

typedef struct
{
    u16 u16Th1;
    u16 u16Th2;
    u16 u16Th3;
    u16 u16X;
    u16 u16Y;
    u16 u16W;
    u16 u16H;
    u16 u16BlkNumX;
    u16 u16BlkNumY;
} HalOsdColorInvParamConfig_t;

//=============================================================================

//=============================================================================
#ifndef __HAL_OSD_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif
INTERFACE bool HalOsdColInvParamCheck(HalOsdColorInvParamConfig_t *ptColInvCfg);
INTERFACE void HalOsdSetOsdBypassForScEnable(HalOsdIdType_e eGopId, bool bEn);
INTERFACE void HalOsdSetOsdEnable(HalOsdIdType_e eOsdId, bool bEn);
INTERFACE bool HalOsdSetBindGop(u32 u32GopOsdFlag);
INTERFACE bool HalOsdCheckIdSupport(HalOsdIdType_e eOsdId);
INTERFACE void HalOsdSetColorInverseEn(HalOsdIdType_e eGopId, bool bEn);
INTERFACE void HalOsdSetColorInverseParam(HalOsdIdType_e eGopId, HalOsdColorInvWindowIdType_e eAeId, HalOsdColorInvParamConfig_t *ptColInvCfg);
INTERFACE void HalOsdSetColorInverseUpdate(HalOsdIdType_e eGopId);
INTERFACE void HalOsdWriteColorInverseData(HalOsdIdType_e eGopId, u16 addr, u32 wdata);
INTERFACE void HalOsdReadColorInverseData(HalOsdIdType_e eGopId, u16 addr, u32 *rdata);
#undef INTERFACE
#endif /* __HAL_GOP_H__ */
