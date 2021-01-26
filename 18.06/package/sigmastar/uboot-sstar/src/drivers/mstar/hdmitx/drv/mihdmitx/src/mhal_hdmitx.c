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


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "mhal_common.h"
#include "drv_hdmitx_os.h"
#include "hdmitx_debug.h"
#include "mhal_hdmitx_datatype.h"
#include "mhal_hdmitx.h"
#include "drv_hdmitx_if.h"

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
MhalHdmitxRet_e MhalHdmitxCreateInstance(void **pCtx, u32 u32Id)
{
    MhalHdmitxRet_e enRet;

    if(pCtx == NULL)
    {
        enRet = E_MHAL_HDMITX_RET_CFGERR;
        HDMITX_ERR("%s %d, Param NULL\n", __FUNCTION__, __LINE__);
    }
    else
    {
        enRet = DrvHdmitxIfCreateInstance(pCtx, u32Id);
    }

    return enRet;
}

MhalHdmitxRet_e MhalHdmitxGetInstance(void **pCtx, u32 u32Id)
{
    MhalHdmitxRet_e enRet;

    if(pCtx == NULL)
    {
        enRet = E_MHAL_HDMITX_RET_CFGERR;
        HDMITX_ERR("%s %d, Param NULL\n", __FUNCTION__, __LINE__);
    }
    else
    {
        enRet = DrvHdmitxIfGetInstance(pCtx, u32Id);
    }

    return enRet;

}

MhalHdmitxRet_e MhalHdmitxDestroyInstance(void *pCtx)
{
    MhalHdmitxRet_e enRet;

    if(pCtx == NULL)
    {
        enRet = E_MHAL_HDMITX_RET_CFGERR;
        HDMITX_ERR("%s %d, Param NULL\n", __FUNCTION__, __LINE__);
    }
    else
    {
        enRet = DrvHdmitxIfDestoryInstance(pCtx);
    }

    return enRet;
}

MhalHdmitxRet_e MhalHdmitxSetAttrBegin(void *pCtx)
{
    MhalHdmitxRet_e enRet;

    if(pCtx == NULL)
    {
        enRet = E_MHAL_HDMITX_RET_CFGERR;
        HDMITX_ERR("%s %d, Param NULL\n", __FUNCTION__, __LINE__);
    }
    else
    {
        enRet = DrvHdmitxIfSetAttrBegin(pCtx);
    }

    return enRet;
}

MhalHdmitxRet_e MhalHdmitxSetAttr(void *pCtx, MhalHdmitxAttrConfig_t *pstAttrCfg)
{
    MhalHdmitxRet_e enRet;

    if(pCtx == NULL || pstAttrCfg == NULL)
    {
        enRet = E_MHAL_HDMITX_RET_CFGERR;
        HDMITX_ERR("%s %d, Param NULL\n", __FUNCTION__, __LINE__);
    }
    else
    {
        enRet = DrvHdmitxIfSetAttr(pCtx, pstAttrCfg);
    }

    return enRet;
}

MhalHdmitxRet_e MhalHdmitxSetAttrEnd(void *pCtx)
{
    MhalHdmitxRet_e enRet;

    if(pCtx == NULL)
    {
        enRet = E_MHAL_HDMITX_RET_CFGERR;
        HDMITX_ERR("%s %d, Param NULL\n", __FUNCTION__, __LINE__);
    }
    else
    {
        enRet = DrvHdmitxIfSetAttrEnd(pCtx);
    }

    return enRet;
}

MhalHdmitxRet_e MhalHdmitxSetMute(void *pCtx, MhalHdmitxMuteConfig_t *pstMuteCfg)
{
    MhalHdmitxRet_e enRet;

    if(pCtx == NULL || pstMuteCfg == NULL)
    {
        enRet = E_MHAL_HDMITX_RET_CFGERR;
        HDMITX_ERR("%s %d, Param NULL\n", __FUNCTION__, __LINE__);
    }
    else
    {
        enRet = DrvHdmitxIfSetMute(pCtx, pstMuteCfg);
    }

    return enRet;
}

MhalHdmitxRet_e MhalHdmitxSetSignal(void *pCtx, MhalHdmitxSignalConfig_t *pstSignalCfg)
{
    MhalHdmitxRet_e enRet;

    if(pCtx == NULL || pstSignalCfg == NULL)
    {
        enRet = E_MHAL_HDMITX_RET_CFGERR;
        HDMITX_ERR("%s %d, Param NULL\n", __FUNCTION__, __LINE__);
    }
    else
    {
        enRet = DrvHdmitxIfSetSignal(pCtx, pstSignalCfg);
    }

    return enRet;
}


MhalHdmitxRet_e MhalHdmitxGetSinkInfo(void *pCtx, MhalHdmitxSinkInfoConfig_t *pstSinkInfoCfg)
{
    MhalHdmitxRet_e enRet;

    if(pCtx == NULL || pstSinkInfoCfg == NULL)
    {
        enRet = E_MHAL_HDMITX_RET_CFGERR;
        HDMITX_ERR("%s %d, Param NULL\n", __FUNCTION__, __LINE__);
    }
    else
    {
        enRet = DrvHdmitxIfGetSinkInfo(pCtx, pstSinkInfoCfg);
    }

    return enRet;
}


MhalHdmitxRet_e MhalHdmitxSetInfoFrame(void *pCtx, MhalHdmitxInfoFrameConfig_t *pstInfoFrameCfg)
{
    MhalHdmitxRet_e enRet;

    if(pCtx == NULL || pstInfoFrameCfg == NULL)
    {
        enRet = E_MHAL_HDMITX_RET_CFGERR;
        HDMITX_ERR("%s %d, Param NULL\n", __FUNCTION__, __LINE__);
    }
    else
    {
        enRet = DrvHdmitxIfSetInfoFrame(pCtx, pstInfoFrameCfg);
    }

    return enRet;
}


MhalHdmitxRet_e MhalHdmitxSetAnalogDrvCur(void *pCtx, MhalHdmitxAnaloDrvCurConfig_t *pstDrvCurCfg)
{
    MhalHdmitxRet_e enRet;

    if(pCtx == NULL || pstDrvCurCfg == NULL)
    {
        enRet = E_MHAL_HDMITX_RET_CFGERR;
        HDMITX_ERR("%s %d, Param NULL\n", __FUNCTION__, __LINE__);
    }
    else
    {
        enRet = DrvHdmitxIfSetAnalogDrvCur(pCtx, pstDrvCurCfg);
    }

    return enRet;
}

MhalHdmitxRet_e MhalHdmitxSetDebugLevel(void *pCtx, u32 u32DbgLevel)
{
    MhalHdmitxRet_e enRet;

    enRet = DrvHdmitxIfSetDebugLevel(pCtx, u32DbgLevel);

    return enRet;
}


MhalHdmitxRet_e MhalHdmitxSetHpdConfig(void *pCtx, MhalHdmitxHpdConfig_t *pstHpdCfg)
{
    MhalHdmitxRet_e enRet;

    if(pCtx == NULL || pstHpdCfg == NULL)
    {
        enRet = E_MHAL_HDMITX_RET_CFGERR;
        HDMITX_ERR("%s %d, Param NULL\n", __FUNCTION__, __LINE__);
    }
    else
    {
        enRet = DrvHdmitxIfSetHpdConfig(pCtx, pstHpdCfg);
    }

    return enRet;
}

