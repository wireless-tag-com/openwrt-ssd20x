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
* \ingroup osd_group
* @{
*/

#ifndef __DRV_OSD_H__
#define __DRV_OSD_H__
#include "drv_rgn_common.h"

//=============================================================================
// Defines
//=============================================================================
#define DRV_OSD_VERSION                        0x0300



//=============================================================================
// enum
//=============================================================================

typedef enum
{
    E_DRV_OSD_OUT_FMT_RGB,
    E_DRV_OSD_OUT_FMT_YUV,
    E_DRV_OSD_OUT_FMT_NUM,
} DrvOsdOutFmtType_e;
typedef enum
{
    E_DRV_OSD_PORT0,
    E_DRV_OSD_PORT1,
    E_DRV_OSD_PORT2,
    E_DRV_OSD_PORT3,
    E_DRV_OSD_DIP,
    E_DRV_OSD_LDC,
    E_DRV_OSD_ID_MAX
}DrvOsdId_e;

typedef enum
{
    E_DRV_OSD_VPE_CMDQ_ID_0,
    E_DRV_OSD_DIVP_CMDQ_ID_1,
    E_DRV_OSD_LDC_CMDQ_ID_1,
    E_DRV_OSD_CMDQ_ID_NUM,
} DrvOsdCmdqIdType_e;

typedef enum
{
    E_DRV_OSD_ERR_OK    =  0, ///< No Error
    E_DRV_OSD_ERR_FAULT = -1, ///< Fault
    E_DRV_OSD_ERR_INVAL = -2, ///< Invalid value
} DrvOsdErrType_e;

typedef enum
{
    E_DRV_OSD_AE0 = 0,
    E_DRV_OSD_AE1,
    E_DRV_OSD_AE2,
    E_DRV_OSD_AE3,
    E_DRV_OSD_AE4,
    E_DRV_OSD_AE5,
    E_DRV_OSD_AE6,
    E_DRV_OSD_AE7,
    E_DRV_OSD_AE_MAX
} DrvOsdColorInvWindowIdType_e;

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
} DrvOsdColorInvParamConfig_t;

//=============================================================================

//=============================================================================
#ifndef __DRV_OSD_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE bool DrvOsdSetEnable(DrvOsdId_e eGopId, bool bEn);
INTERFACE bool DrvOsdSetBindGop(u32 u32GopOsdFlag);
INTERFACE bool DrvOsdInit(void);
INTERFACE bool DrvOsdSetColorInverseEn(DrvOsdId_e eGopId, bool bEn);
INTERFACE bool DrvOsdSetColorInverseParam(DrvOsdId_e eGopId, DrvOsdColorInvWindowIdType_e eAeId, DrvOsdColorInvParamConfig_t *ptColInvCfg);
INTERFACE bool DrvOsdSetColorInverseUpdate(DrvOsdId_e eGopId);
INTERFACE bool DrvOsdWriteColorInverseData(DrvOsdId_e eGopId, u16 addr, u32 wdata);
INTERFACE bool DrvOsdReadColorInverseData(DrvOsdId_e eGopId, u16 addr, u32 *rdata);


#undef INTERFACE
#endif //
/** @} */ // end of hvsp_group
