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
* \ingroup hvsp_group
* @{
*/

#ifndef __DRV_COVER_H__
#define __DRV_COVER_H__
#include "drv_rgn_common.h"
//=============================================================================
// Defines
//=============================================================================
#define DRV_COVER_VERSION                        0x0100


//=============================================================================
// enum
//=============================================================================

typedef enum
{
    E_DRV_ISPSC0_COVER,
    E_DRV_ISPSC1_COVER,
    E_DRV_ISPSC2_COVER,
    E_DRV_ISPSC3_COVER,
    E_DRV_DIP_COVER,
    E_DRV_COVER_ID_NUM,
} DrvCoverIdType_e;

typedef enum
{
    E_DRV_COVER_CMDQ_VPE_ID_0,
    E_DRV_COVER_CMDQ_DIVP_ID_1,
    E_DRV_COVER_CMDQ_ID_NUM
}DrvCoverCmdqIdType_e;

//------------------------------------------------------------------------------
// BWin
//------------------------------------------------------------------------------
typedef enum
{
    E_DRV_COVER_WIN_ID_0,
    E_DRV_COVER_WIN_ID_1,
    E_DRV_COVER_WIN_ID_2,
    E_DRV_COVER_WIN_ID_3,
    E_DRV_COVER_WIN_ID_NUM,
} DrvCoverWinIdType_e;

/**
* Used to setup the error type of hvsp device
*/
typedef enum
{
    E_DRV_COVER_ERR_OK    =  0, ///< No Error
    E_DRV_COVER_ERR_FAULT = -1, ///< Fault
    E_DRV_COVER_ERR_INVAL = -2, ///< Invalid value
} DrvCoverErrType_e;

//=============================================================================
// struct
//=============================================================================

typedef struct
{
    u16 u16X;
    u16 u16Y;
    u16 u16Width;
    u16 u16Height;
} DrvCoverWindowConfig_t;

typedef struct
{
    u8 u8R;
    u8 u8G;
    u8 u8B;
} DrvCoverColorConfig_t;

//=============================================================================

//=============================================================================
#ifndef __DRV_COVER_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif

INTERFACE bool DrvCoverRgnInit(void);
INTERFACE bool DrvCoverSetWinSize(DrvCoverIdType_e eCoverId, DrvCoverWinIdType_e eWinId, DrvCoverWindowConfig_t *ptWinCfg);
INTERFACE bool DrvCoverSetColor(DrvCoverIdType_e eCoverId, DrvCoverWinIdType_e eWinId, DrvCoverColorConfig_t *ptColorCfg);
INTERFACE bool DrvCoverSetEnableWin(DrvCoverIdType_e eCoverId,  DrvCoverWinIdType_e eWinId, bool bEn);


#undef INTERFACE
#endif //
/** @} */ // end of hvsp_group
