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

#ifndef _DRV_DISP_OS_H_
#define _DRV_DISP_OS_H_

#include "drv_disp_os_header.h"

#ifdef _DRV_DISP_OS_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE void *DrvDispOsMemAlloc(u32 u32Size);
INTERFACE void DrvDispOsMemRelease(void *pPtr);
INTERFACE void DrvDispOsMsSleep(u32 u32Msec);
INTERFACE void DrvDispOsUsSleep(u32 u32Usec);
INTERFACE s32  DrvDispOsImiHeapAlloc(u32 u32Size, void **ppAddr);
INTERFACE void DrvDispOsImiHeapFree(void *pAddr);
INTERFACE bool DrvDispOsPadMuxActive(void);
INTERFACE bool DrvDispOsSetDeviceNode(void *pPlatFormDev);
INTERFACE bool DrvDispOsSetClkOn(void *pClkRate, u32 u32ClkRateSize);
INTERFACE bool DrvDispOsSetClkOff(void);
INTERFACE bool DrvDispOsCreateTask(DrvDispOsTaskConfig_t *pstTaskCfg, TaskEntryCb pTaskEntry, void *pDataPtr, char *pTaskName, bool bAuotStart);
INTERFACE bool DrvDispOsDestroyTask(DrvDispOsTaskConfig_t *pstTaskCfg);
INTERFACE u32  DrvDispOsGetSystemTime (void);
INTERFACE u64  DrvDispOsGetSystemTimeStamp (void);

#undef INTERFACE

#endif
