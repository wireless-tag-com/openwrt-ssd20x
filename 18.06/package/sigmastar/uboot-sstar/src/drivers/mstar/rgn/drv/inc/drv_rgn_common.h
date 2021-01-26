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

#ifndef __DRV_RGN_COMMON_H__
#define __DRV_RGN_COMMON_H__
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
    E_DRV_RGN_VPE_CMDQ_ID_0,
    E_DRV_RGN_DIVP_CMDQ_ID_1,
    E_DRV_RGN_LDC_CMDQ_ID_2,
    E_DRV_RGN_CMDQ_ID_NUM,
} DrvRgnCmdqIdType_e;
typedef enum
{
    E_DRV_RGN_INIT_COVER = 0x1,
    E_DRV_RGN_INIT_GOP = 0x2,
} DrvRgnInitType_e;

//=============================================================================
// struct
//=============================================================================
//=============================================================================

//=============================================================================
#ifndef __DRV_RGN_COMMON_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif
INTERFACE bool DrvRgnInit(void);
INTERFACE bool DrvRgnDeinit(void);
INTERFACE bool DrvRgnUpdateReg(void);
INTERFACE bool DrvRgnGetOsdBindGop(unsigned int *eGopNum);
INTERFACE bool DrvRgnSetOsdBindGop(unsigned int eGopNum);
INTERFACE bool DrvRgnSetCmdq(MHAL_CMDQ_CmdqInterface_t *pstCmdq,DrvRgnCmdqIdType_e eCmdqId);
INTERFACE void DrvRgnUseCmdq(bool bEn);

#undef INTERFACE
#endif //
/** @} */ // end of hvsp_group
