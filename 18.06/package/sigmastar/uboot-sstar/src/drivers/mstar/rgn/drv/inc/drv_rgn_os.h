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

#ifndef __DRV_RGN_OS_H__
#define __DRV_RGN_OS_H__
#include "drv_rgn_common.h"
#include "drv_gop.h"

//=============================================================================

//=============================================================================
#ifndef __DRV_RGN_OS_C__
#define INTERFACE extern
#else
#define INTERFACE
#endif
INTERFACE bool DrvRgnOsSetGopClkEnable(DrvGopIdType_e eGopId);
INTERFACE bool DrvRgnOsSetGopClkDisable(DrvGopIdType_e eGopId,bool bEn);

#endif
