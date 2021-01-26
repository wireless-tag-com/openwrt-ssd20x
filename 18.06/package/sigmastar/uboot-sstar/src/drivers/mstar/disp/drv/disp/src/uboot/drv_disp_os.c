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

#define _DRV_DISP_OS_C_
#include <common.h>
#include <command.h>
#include <config.h>
#include <malloc.h>
#include <stdlib.h>

#include "drv_disp_os.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Internal Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Public Functions
//-------------------------------------------------------------------------------------------------
void * DrvDispOsMemAlloc(u32 u32Size)
{
    return malloc(u32Size);
}

void DrvDispOsMemRelease(void *pPtr)
{
    free(pPtr);
}

void DrvDispOsMsSleep(u32 u32Msec)
{
    mdelay(u32Msec);
}

void DrvDispOsUsSleep(u32 u32Usec)
{
    udelay(u32Usec);
}

s32 DrvDispOsImiHeapAlloc(u32 u32Size, void **ppAddr)
{
    return 0;
}

void DrvDispOsImiHeapFree(void *pAddr)
{

}

bool DrvDispOsPadMuxActive(void)
{
    return 0;
}

bool DrvDispOsCreateTask(DrvDispOsTaskConfig_t *pstTaskCfg, TaskEntryCb pTaskEntry, void *pDataPtr, char *pTaskName, bool bAuotStart)
{
    return 0;
}

bool DrvDispOsDestroyTask(DrvDispOsTaskConfig_t *pstTaskCfg)
{
    return 0;
}

bool DrvDispOsSetDeviceNode(void *pPlatFormDev)
{
    return 1;
}

bool DrvDispOsSetClkOn(void *pClkRate, u32 u32ClkRateSize)
{
    return 0;
}

bool DrvDispOsSetClkOff(void)
{
    return 0;
}


u32 DrvDispOsGetSystemTime (void)
{
    return 0;
}

u64 DrvDispOsGetSystemTimeStamp (void)
{
    return 0;
}

