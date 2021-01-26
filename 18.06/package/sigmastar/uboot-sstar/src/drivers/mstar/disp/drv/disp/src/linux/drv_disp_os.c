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

#include "drv_disp_os.h"
#include "disp_debug.h"
#include "cam_sysfs.h"
#include "cam_clkgen.h"
#include "mhal_common.h"
#include "mhal_disp_datatype.h"
#include "mhal_disp.h"


#include "imi_heap.h"
#include "mdrv_padmux.h"
#include "hal_disp_chip.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
struct device_node *gpDispDeviceNode = NULL;

//-------------------------------------------------------------------------------------------------
//  Internal Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Public Functions
//-------------------------------------------------------------------------------------------------
void * DrvDispOsMemAlloc(u32 u32Size)
{
    return CamOsMemAlloc(u32Size);
}

void DrvDispOsMemRelease(void *pPtr)
{
    CamOsMemRelease(pPtr);
}

void DrvDispOsMsSleep(u32 u32Msec)
{
    CamOsMsSleep(u32Msec);
}

void DrvDispOsUsSleep(u32 u32Usec)
{
    CamOsUsSleep(u32Usec);
}

s32 DrvDispOsImiHeapAlloc(u32 u32Size, void **ppAddr)
{
    s32 s32Ret;
    s32Ret = ImiHeapMalloc(0, u32Size, ppAddr);
    return s32Ret;
}

void DrvDispOsImiHeapFree(void *pAddr)
{
    ImiHeapFree(0, pAddr);
}

bool DrvDispOsPadMuxActive(void)
{
    bool bRet = mdrv_padmux_active() ? 1 : 0;
    return bRet;
}

bool DrvDispOsCreateTask(DrvDispOsTaskConfig_t *pstTaskCfg, TaskEntryCb pTaskEntry, void *pDataPtr, char *pTaskName, bool bAuotStart)
{
    CamOsThreadAttrb_t stThreadAttrCfg;
    bool bRet = TRUE;

    stThreadAttrCfg.nPriority = 50;
    stThreadAttrCfg.nStackSize = 0;
    stThreadAttrCfg.szName = pTaskName;

    if(CamOsThreadCreate(&pstTaskCfg->stThreadCfg, &stThreadAttrCfg, pTaskEntry, pDataPtr) == CAM_OS_OK)
    {

        if(bAuotStart)
        {
            if(CamOsThreadWakeUp(pstTaskCfg->stThreadCfg) == CAM_OS_OK)
            {
                bRet = TRUE;
                pstTaskCfg->s32Id = 1;

            }
            else
            {
                bRet = FALSE;
                pstTaskCfg->s32Id = -1;
                DISP_ERR("%s %d, CamOsThreadWakeUp Fail\n", __FUNCTION__, __LINE__);
            }
        }
    }
    else
    {
        pstTaskCfg->s32Id = -1;
        bRet = FALSE;
        DISP_ERR("%s %d, CamOsThreadCreate Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool DrvDispOsDestroyTask(DrvDispOsTaskConfig_t *pstTaskCfg)
{
    if(CamOsThreadStop(pstTaskCfg->stThreadCfg) == CAM_OS_OK)
    {
        pstTaskCfg->s32Id = -1;
        return TRUE;
    }
    else
    {
        DISP_ERR("%s %d, CamOsThreadStop Fail\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
}

bool DrvDispOsSetDeviceNode(void *pPlatFormDev)
{
    gpDispDeviceNode = ((struct platform_device *)pPlatFormDev)->dev.of_node;

    if(gpDispDeviceNode)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool DrvDispOsSetClkOn(void *pClkRate, u32 u32ClkRateSize)
{
    bool bRet = TRUE;
    u32 num_parents, idx;
    struct clk **disp_clks;
    struct clk_hw *hw_parent;
    unsigned long clk_rate;
    unsigned long *clk_rate_buf = (unsigned long *)pClkRate;
    bool abClkMuxAttr[HAL_DISP_CLK_NUM] = HAL_DISP_CLK_MUX_ATTR;

    if(gpDispDeviceNode && pClkRate && u32ClkRateSize == HAL_DISP_CLK_NUM)
    {
        num_parents = CamOfClkGetParentCount(gpDispDeviceNode);

        DISP_DBG(DISP_DBG_LEVEL_CLK, "%s %d num_parents:%d \n", __FUNCTION__, __LINE__, num_parents);
        for(idx=0;idx<u32ClkRateSize; idx++)
        {
            DISP_DBG(DISP_DBG_LEVEL_CLK, "%s %d, CLK_%d = %ld\n", __FUNCTION__, __LINE__, idx, clk_rate_buf[idx]);
        }

        if(num_parents ==  HAL_DISP_CLK_NUM)
        {
            disp_clks = CamOsMemAlloc((sizeof(struct clk *) * num_parents));

            if(disp_clks == NULL)
            {
                DISP_ERR( "%s %d Alloc disp_clks is NULL\n" , __FUNCTION__, __LINE__);
                return 0;
            }

            for(idx=0; idx<num_parents; idx++)
            {
                disp_clks[idx] = of_clk_get(gpDispDeviceNode, idx);
                if (IS_ERR(disp_clks[idx]))
                {
                    DISP_ERR( "%s %d, Fail to get [Disp] %s\n" ,
                        __FUNCTION__, __LINE__, CamOfClkGetParentName(gpDispDeviceNode, idx) );
                    CamOsMemRelease(disp_clks);
                    return 0;
                }

                if(abClkMuxAttr[idx] == 1)
                {
                    clk_rate = clk_rate_buf[idx];
                    hw_parent = CamClkGetParentByIndex(__CamClkGetHw(disp_clks[idx]), clk_rate);
                    CamClkSetParent(disp_clks[idx], hw_parent->clk);
                }
                else
                {
                    clk_rate = CamClkRoundRate(disp_clks[idx], clk_rate_buf[idx]);
                    CamClkSetRate(disp_clks[idx], clk_rate + 1000000);
                }

                DISP_DBG(DISP_DBG_LEVEL_CLK, "%s %d, [Disp] %d-%d %15s %ld\n" ,
                            __FUNCTION__, __LINE__,
                            num_parents, idx+1,
                            CamOfClkGetParentName(gpDispDeviceNode, idx),
                            clk_rate);

                CamClkPrepareEnable(disp_clks[idx]);
                clk_put(disp_clks[idx]);
            }

            CamOsMemRelease(disp_clks);
        }
        else
        {
            bRet = FALSE;
            DISP_ERR("%s %d, num_parents %d != %d\n", __FUNCTION__, __LINE__, num_parents, HAL_DISP_CLK_NUM);
        }
    }
    else
    {
        bRet = FALSE;
        DISP_ERR("%s %d, Param Null, DeviceNode:%x, ClkRate:%x, ClkSize:%ld\n",
            __FUNCTION__, __LINE__, gpDispDeviceNode, pClkRate, u32ClkRateSize);
    }
    return bRet;
}

bool DrvDispOsSetClkOff(void)
{
    bool bRet = TRUE;
    u32 num_parents, idx;
    struct clk **disp_clks;

    if(gpDispDeviceNode)
    {
        num_parents = CamOfClkGetParentCount(gpDispDeviceNode);

        DISP_DBG(DISP_DBG_LEVEL_CLK, "%s %d num_parents:%d\n" , __FUNCTION__, __LINE__, num_parents);
        if(num_parents == HAL_DISP_CLK_NUM)
        {
            disp_clks = CamOsMemAlloc((sizeof(struct clk *) * num_parents));

            if(disp_clks == NULL)
            {
                DISP_ERR( "%s %d Alloc disp_clks is NULL\n" , __FUNCTION__, __LINE__);
                return 0;
            }


            for(idx=0; idx<num_parents; idx++)
            {
                disp_clks[idx] = of_clk_get(gpDispDeviceNode, idx);
                if (IS_ERR(disp_clks[idx]))
                {
                    DISP_ERR( "%s %d, Fail to get [Disp] %s\n" ,
                        __FUNCTION__, __LINE__, CamOfClkGetParentName(gpDispDeviceNode, idx) );
                    CamOsMemRelease(disp_clks);
                    return 0;
                }
                else
                {
                    DISP_DBG(DISP_DBG_LEVEL_CLK,"%s %d, [Disp] %d-%d, %15s Off\n",
                        __FUNCTION__, __LINE__, num_parents, idx+1,
                        CamOfClkGetParentName(gpDispDeviceNode, idx));
                    CamClkDisableUnprepare(disp_clks[idx]);
                }
                clk_put(disp_clks[idx]);
            }
            CamOsMemRelease(disp_clks);
        }
        else
        {
            bRet = FALSE;
        }
    }
    else
    {
        bRet = FALSE;
    }
    return bRet;
}


u32 DrvDispOsGetSystemTime (void)
{
    CamOsTimespec_t ts;
    CamOsGetTimeOfDay(&ts);
    return ts.nSec* 1000+ ts.nNanoSec/1000000;
}

u64 DrvDispOsGetSystemTimeStamp (void)
{
    u64 u64TimeStamp;
    CamOsTimespec_t stRes;
    CamOsGetMonotonicTime(&stRes);
    u64TimeStamp =(u64)stRes.nSec* 1000000ULL+ (u64)(stRes.nNanoSec/1000LL);
    return u64TimeStamp;
}

EXPORT_SYMBOL(MHAL_DISP_DeviceSetBackGroundColor);
EXPORT_SYMBOL(MHAL_DISP_VideoLayerUnBind);
EXPORT_SYMBOL(MHAL_DISP_VideoLayerCreateInstance);
EXPORT_SYMBOL(MHAL_DISP_VideoLayerDestoryInstance);
EXPORT_SYMBOL(MHAL_DISP_DeviceCreateInstance);
EXPORT_SYMBOL(MHAL_DISP_InputPortSetAttr);
EXPORT_SYMBOL(MHAL_DISP_DeviceSetHdmiParam);
EXPORT_SYMBOL(MHAL_DISP_VideoLayerBind);
EXPORT_SYMBOL(MHAL_DISP_DeviceEnable);
EXPORT_SYMBOL(MHAL_DISP_InputPortShow);
EXPORT_SYMBOL(MHAL_DISP_InputPortAttrEnd);
EXPORT_SYMBOL(MHAL_DISP_ClearDevInterrupt);
EXPORT_SYMBOL(MHAL_DISP_VideoLayerEnable);
EXPORT_SYMBOL(MHAL_DISP_DeviceSetOutputTiming);
EXPORT_SYMBOL(MHAL_DISP_DeviceDestroyInstance);
EXPORT_SYMBOL(MHAL_DISP_InputPortCreateInstance);
EXPORT_SYMBOL(MHAL_DISP_InputPortEnable);
EXPORT_SYMBOL(MHAL_DISP_InitPanelConfig);
EXPORT_SYMBOL(MHAL_DISP_GetDevIrq);
EXPORT_SYMBOL(MHAL_DISP_GetDevIrqFlag);
EXPORT_SYMBOL(MHAL_DISP_InitMmapConfig);
EXPORT_SYMBOL(MHAL_DISP_DeviceAttach);
EXPORT_SYMBOL(MHAL_DISP_InputPortAttrBegin);
EXPORT_SYMBOL(MHAL_DISP_InputPortFlip);
EXPORT_SYMBOL(MHAL_DISP_DeviceDetach);
EXPORT_SYMBOL(MHAL_DISP_DeviceSetCvbsParam);
EXPORT_SYMBOL(MHAL_DISP_InputPortDestroyInstance);
EXPORT_SYMBOL(MHAL_DISP_VideoLayerCheckBufferFired);
EXPORT_SYMBOL(MHAL_DISP_InputPortHide);
EXPORT_SYMBOL(MHAL_DISP_DeviceSetVgaParam);
EXPORT_SYMBOL(MHAL_DISP_VideoLayerSetAttr);
EXPORT_SYMBOL(MHAL_DISP_DeviceAddOutInterface);
EXPORT_SYMBOL(MHAL_DISP_EnableDevIrq);
EXPORT_SYMBOL(MHAL_DISP_DbgLevel);
EXPORT_SYMBOL(MHAL_DISP_InputPortRotate);
EXPORT_SYMBOL(MHAL_DISP_InputPortSetCropAttr);
EXPORT_SYMBOL(MHAL_DISP_DeviceSetColorTempeture);
EXPORT_SYMBOL(MHAL_DISP_DeviceSetLcdParam);
EXPORT_SYMBOL(MHAL_DISP_DeviceSetGammaParam);
EXPORT_SYMBOL(MHAL_DISP_DeviceGetTimeZone);
EXPORT_SYMBOL(MHAL_DISP_DeviceGetDisplayInfo);
EXPORT_SYMBOL(MHAL_DISP_DeviceGetInstance);

EXPORT_SYMBOL(DrvDispOsMemRelease);
EXPORT_SYMBOL(DrvDispOsMemAlloc);
EXPORT_SYMBOL(DrvDispOsCreateTask);
EXPORT_SYMBOL(DrvDispOsDestroyTask);
EXPORT_SYMBOL(DrvDispOsMsSleep);
