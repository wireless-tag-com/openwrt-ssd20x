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

#ifndef _DRV_PNL_OS_H_
#define _DRV_PNL_OS_H_

#include "drv_pnl_os_header.h"

#ifdef _DRV_PNL_OS_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif


INTERFACE void * DrvPnlOsMemAlloc(u32 u32Size);
INTERFACE void DrvPnlOsMemRelease(void *pPtr);
INTERFACE void DrvPnlOsMsSleep(u32 u32Msec);
INTERFACE void DrvPnlOsUsSleep(u32 u32Usec);
INTERFACE bool DrvPnlOsPadMuxActive(void);
INTERFACE bool DrvPnlOsSetDeviceNode(void *pPlatFormDev);
INTERFACE bool DrvPnlOsSetClkOn(void *pClkRate, u32 u32ClkRateSize);
INTERFACE bool DrvPnlOsSetClkOff(void);

#undef INTERFACE

#endif
