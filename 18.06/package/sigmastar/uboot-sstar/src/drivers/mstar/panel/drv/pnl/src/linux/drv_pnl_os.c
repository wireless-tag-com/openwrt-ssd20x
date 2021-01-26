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

#define _DRV_PNL_OS_C_

#include "drv_pnl_os.h"
#include "mhal_common.h"
#include "mdrv_padmux.h"
#include "pnl_debug.h"
#include "cam_clkgen.h"

#include "mhal_pnl_datatype.h"
#include "mhal_pnl.h"
#include "hal_pnl_chip.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

struct device_node *gpPnlDeviceNode = NULL;

//-------------------------------------------------------------------------------------------------
//  Internal Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Public Functions
//-------------------------------------------------------------------------------------------------


void * DrvPnlOsMemAlloc(u32 u32Size)
{
    return CamOsMemAlloc(u32Size);
}

void DrvPnlOsMemRelease(void *pPtr)
{
    CamOsMemRelease(pPtr);
}

void DrvPnlOsMsSleep(u32 u32Msec)
{
    CamOsMsSleep(u32Msec);
}

void DrvPnlOsUsSleep(u32 u32Usec)
{
    CamOsUsSleep(u32Usec);
}

bool DrvPnlOsPadMuxActive(void)
{
    bool bRet = mdrv_padmux_active() ? 1 : 0;
    return bRet;
}

bool DrvPnlOsSetDeviceNode(void *pPlatFormDev)
{
    gpPnlDeviceNode = ((struct platform_device *)pPlatFormDev)->dev.of_node;

    if(gpPnlDeviceNode)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool DrvPnlOsSetClkOn(void *pClkRate, u32 u32ClkRateSize)
{
    bool bRet = TRUE;
    u32 num_parents, idx;
    struct clk **pnl_clks;
    struct clk_hw *hw_parent;
    unsigned long clk_rate;
    unsigned long *clk_rate_buf = (unsigned long *)pClkRate;
    bool abClkMuxAttr[HAL_PNL_CLK_NUM] = HAL_PNL_CLK_MUX_ATTR;

    if(gpPnlDeviceNode && pClkRate && u32ClkRateSize == HAL_PNL_CLK_NUM)
    {
        num_parents = CamOfClkGetParentCount(gpPnlDeviceNode);
        for(idx=0;idx<u32ClkRateSize; idx++)
        {
            PNL_DBG(PNL_DBG_LEVEL_CLK, "%s %d, CLK_%d = %ld\n", __FUNCTION__, __LINE__, idx, clk_rate_buf[idx]);
        }

        PNL_DBG(PNL_DBG_LEVEL_CLK, "%s %d num_parents:%d \n", __FUNCTION__, __LINE__, num_parents);
        if(num_parents ==  HAL_PNL_CLK_NUM)
        {
            pnl_clks = CamOsMemAlloc((sizeof(struct clk *) * num_parents));

            if(pnl_clks == NULL)
            {
                PNL_ERR( "%s %d Alloc pnl_clks is NULL\n" , __FUNCTION__, __LINE__);
                return 0;
            }

            for(idx=0; idx<num_parents; idx++)
            {
                pnl_clks[idx] = of_clk_get(gpPnlDeviceNode, idx);
                if (IS_ERR(pnl_clks[idx]))
                {
                    PNL_ERR( "%s %d, Fail to get [Pnl] %s\n" ,
                        __FUNCTION__, __LINE__, CamOfClkGetParentName(gpPnlDeviceNode, idx) );
                    CamOsMemRelease(pnl_clks);
                    return 0;
                }

                if(abClkMuxAttr[idx] == 1)
                {
                    clk_rate = clk_rate_buf[idx];
                    hw_parent = CamClkGetParentByIndex(__CamClkGetHw(pnl_clks[idx]), clk_rate);
                    CamClkSetParent(pnl_clks[idx], hw_parent->clk);
                }
                else
                {
                    clk_rate = CamClkRoundRate(pnl_clks[idx], clk_rate_buf[idx]);
                    CamClkSetRate(pnl_clks[idx], clk_rate + 1000000);
                }

                PNL_DBG(PNL_DBG_LEVEL_CLK, "%s %d, [Pnl] num_parents:%d-%d %20s %ld\n" ,
                            __FUNCTION__, __LINE__,
                            num_parents, idx+1,
                            CamOfClkGetParentName(gpPnlDeviceNode, idx),
                            clk_rate);

                CamClkPrepareEnable(pnl_clks[idx]);
                clk_put(pnl_clks[idx]);
            }

            CamOsMemRelease(pnl_clks);
        }
        else
        {
            bRet = FALSE;
            PNL_ERR("%s %d, num_parents %d != %d\n", __FUNCTION__, __LINE__, num_parents, HAL_PNL_CLK_NUM);
        }
    }
    else
    {
        bRet = FALSE;
        PNL_ERR("%s %d, Param Null, DeviceNode:%x, ClkRate:%x, ClkSize:%ld\n",
            __FUNCTION__, __LINE__, gpPnlDeviceNode, pClkRate, u32ClkRateSize);
    }
    return bRet;
}

bool DrvPnlOsSetClkOff(void)
{
    bool bRet = TRUE;
    u32 num_parents, idx;
    struct clk **pnl_clks;

    if(gpPnlDeviceNode)
    {
        num_parents = CamOfClkGetParentCount(gpPnlDeviceNode);

        PNL_DBG(PNL_DBG_LEVEL_CLK, "%s %d num_parents:%d\n" , __FUNCTION__, __LINE__, num_parents);
        if(num_parents == HAL_PNL_CLK_NUM)
        {
            pnl_clks = CamOsMemAlloc((sizeof(struct clk *) * num_parents));

            if(pnl_clks == NULL)
            {
                PNL_ERR( "%s %d Alloc pnl_clks is NULL\n" , __FUNCTION__, __LINE__);
                return 0;
            }


            for(idx=0; idx<num_parents; idx++)
            {
                pnl_clks[idx] = of_clk_get(gpPnlDeviceNode, idx);
                if (IS_ERR(pnl_clks[idx]))
                {
                    PNL_ERR( "%s %d, Fail to get [Pnl] %s\n" ,
                        __FUNCTION__, __LINE__, CamOfClkGetParentName(gpPnlDeviceNode, idx) );
                    CamOsMemRelease(pnl_clks);
                    return 0;
                }
                else
                {
                    PNL_DBG(PNL_DBG_LEVEL_CLK,"%s %d, [Pnl] %d-%d, %20s Off\n",
                        __FUNCTION__, __LINE__, num_parents, idx+1,
                        CamOfClkGetParentName(gpPnlDeviceNode, idx));

                    CamClkDisableUnprepare(pnl_clks[idx]);
                }
                clk_put(pnl_clks[idx]);
            }
            CamOsMemRelease(pnl_clks);
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



EXPORT_SYMBOL(MhalPnlCreateInstance);
EXPORT_SYMBOL(MhalPnlDestroyInstance);
EXPORT_SYMBOL(MhalPnlSetParamConfig);
EXPORT_SYMBOL(MhalPnlGetParamConfig);
EXPORT_SYMBOL(MhalPnlSetMipiDsiConfig);
EXPORT_SYMBOL(MhalPnlGetMipiDsiConfig);
EXPORT_SYMBOL(MhalPnlSetSscConfig);
EXPORT_SYMBOL(MhalPnlSetTimingConfig);
EXPORT_SYMBOL(MhalPnlGetTimingConfig);
EXPORT_SYMBOL(MhalPnlSetPowerConfig);
EXPORT_SYMBOL(MhalPnlGetPowerConfig);
EXPORT_SYMBOL(MhalPnlSetBackLightOnOffConfig);
EXPORT_SYMBOL(MhalPnlGetBackLightOnOffConfig);
EXPORT_SYMBOL(MhalPnlSetBackLightLevelConfig);
EXPORT_SYMBOL(MhalPnlGetBackLightLevelConfig);
EXPORT_SYMBOL(MhalPnlSetDrvCurrentConfig);
EXPORT_SYMBOL(MhalPnlSetTestPatternConfig);
EXPORT_SYMBOL(MhalPnlSetDebugLevel);
EXPORT_SYMBOL(DrvPnlOsMemRelease);
EXPORT_SYMBOL(DrvPnlOsMemAlloc);
