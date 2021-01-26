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

#define __MHAL_PNL_C__
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_pnl_os.h"
#include "mhal_pnl_datatype.h"
#include "mhal_pnl.h"
#include "pnl_debug.h"
#include "drv_pnl_if.h"
//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local enum
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Structurs
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Private Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Pubic Functions
//-------------------------------------------------------------------------------------------------
bool MhalPnlCreateInstance(void **pCtx, MhalPnlLinkType_e enLinkType)
{
    return DrvPnlIfCreateInstance(pCtx, enLinkType, 0);
}

bool MhalPnlDestroyInstance(void *pCtx)
{
    bool bRet;

    if(pCtx == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet =  DrvPnlIfDestroyInstance(pCtx);
    }
    return bRet;
}


bool MhalPnlSetParamConfig(void *pCtx, MhalPnlParamConfig_t *pParamCfg)
{
    bool bRet = 1;

    if(pCtx == NULL || pParamCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfSetParamConfig(pCtx, pParamCfg);
    }
    return bRet;
}


bool MhalPnlGetParamConfig(void *pCtx, MhalPnlParamConfig_t *pParamCfg)
{
    bool bRet;

    if(pCtx == NULL || pParamCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfGetParamConfig(pCtx, pParamCfg);
    }
    return bRet;
}


bool MhalPnlSetMipiDsiConfig(void *pCtx, MhalPnlMipiDsiConfig_t *pMipiDsiCfg)
{
    bool bRet;

    if(pCtx == NULL || pMipiDsiCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfSetMipiDsiConfig(pCtx, pMipiDsiCfg);
    }
    return bRet;
}


bool MhalPnlGetMipiDsiConfig(void *pCtx, MhalPnlMipiDsiConfig_t *pMipiDsiCfg)
{
    bool bRet;

    if(pCtx == NULL || pMipiDsiCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfGetMipiDsiConfig(pCtx, pMipiDsiCfg);
    }
    return bRet;
}



bool MhalPnlSetSscConfig(void *pCtx, MhalPnlSscConfig_t *pSscCfg)
{
    bool bRet;

    if(pCtx == NULL || pSscCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfSetSscConfig(pCtx, pSscCfg);
    }
    return bRet;
}



bool MhalPnlSetTimingConfig(void *pCtx, MhalPnlTimingConfig_t *pTimingCfg)
{
    bool bRet;

    if(pCtx == NULL || pTimingCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfSetTimingConfig(pCtx, pTimingCfg);
    }
    return bRet;
}



bool MhalPnlGetTimingConfig(void *pCtx, MhalPnlTimingConfig_t *pTimingCfg)
{
    bool bRet;

    if(pCtx == NULL || pTimingCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfGetTimingConfig(pCtx, pTimingCfg);
    }
    return bRet;
}



bool MhalPnlSetPowerConfig(void *pCtx, MhalPnlPowerConfig_t *pPowerCfg)
{
    bool bRet;

    if(pCtx == NULL || pPowerCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfSetPowerConfig(pCtx, pPowerCfg);
    }
    return bRet;
}



bool MhalPnlGetPowerConfig(void *pCtx, MhalPnlPowerConfig_t *pPowerCfg)
{
    bool bRet;

    if(pCtx == NULL || pPowerCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfGetPowerConfig(pCtx, pPowerCfg);
    }
    return bRet;
}



bool MhalPnlSetBackLightOnOffConfig(void *pCtx, MhalPnlBackLightOnOffConfig_t *pBackLightOnOffCfg)
{
    bool bRet;

    if(pCtx == NULL || pBackLightOnOffCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfSetBackLightOnOffConfig(pCtx, pBackLightOnOffCfg);
    }
    return bRet;
}



bool MhalPnlGetBackLightOnOffConfig(void *pCtx, MhalPnlBackLightOnOffConfig_t *pBackLightOnOffCfg)
{
    bool bRet;

    if(pCtx == NULL || pBackLightOnOffCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfGetBackLightOnOffConfig(pCtx, pBackLightOnOffCfg);
    }
    return bRet;
}



bool MhalPnlSetBackLightLevelConfig(void *pCtx, MhalPnlBackLightLevelConfig_t *pBackLightLevelCfg)
{
    bool bRet;

    if(pCtx == NULL || pBackLightLevelCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfSetBackLightLevelConfig(pCtx, pBackLightLevelCfg);
    }
    return bRet;
}



bool MhalPnlGetBackLightLevelConfig(void *pCtx, MhalPnlBackLightLevelConfig_t *pBackLightLevelCfg)
{
    bool bRet;

    if(pCtx == NULL || pBackLightLevelCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfGetBackLightLevelConfig(pCtx, pBackLightLevelCfg);
    }
    return bRet;
}



bool MhalPnlSetDrvCurrentConfig(void *pCtx, MhalPnlDrvCurrentConfig_t *pDrvCurrentCfg)
{
    bool bRet;

    if(pCtx == NULL || pDrvCurrentCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfSetDrvCurrentConfig(pCtx, pDrvCurrentCfg);
    }
    return bRet;
}



bool MhalPnlSetTestPatternConfig(void *pCtx, MhalPnlTestPatternConfig_t *pTestPatternCfg)
{
    bool bRet;

    if(pCtx == NULL || pTestPatternCfg == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfSetTestPatternConfig(pCtx, pTestPatternCfg);
    }
    return bRet;
}

bool MhalPnlSetDebugLevel(void *pDbgLevel)
{
    bool bRet;

    if(pDbgLevel == NULL)
    {
        PNL_ERR("%s %d, Param Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        bRet = DrvPnlIfSetDbgLevel(pDbgLevel);
    }
    return bRet;
}
