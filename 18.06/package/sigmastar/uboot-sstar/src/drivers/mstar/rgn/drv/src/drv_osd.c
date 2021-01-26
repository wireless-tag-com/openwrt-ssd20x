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

#define __DRV_OSD_C__


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_osd.h"
#include "hal_osd.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    bool bOsdEn;
    bool bBypassEn;
    bool bColorInv;
    DrvOsdColorInvParamConfig_t stColorInvCfg[E_DRV_OSD_AE_MAX];
 } _DrvOsdLocalSettingsConfig_t;

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
_DrvOsdLocalSettingsConfig_t _tOsdpLocSettings[HAL_RGN_OSD_NUM]; 

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

bool _DrvOsdTransId(DrvOsdId_e eOsdId, HalOsdIdType_e *pHalId)
{
    bool bRet = TRUE;
    *pHalId = (HalOsdIdType_e)eOsdId;
    if(!HalOsdCheckIdSupport(*pHalId))
    {
        bRet = 0;
    }
    return bRet;
}
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
bool DrvOsdInit(void)
{
    bool bRet = TRUE;
    HalOsdIdType_e eHalOsdId;
    DrvOsdId_e eOsdId;
    bRet = HalRgnCheckBindRation((HalRgnOsdFlag_e*)&geGopOsdFlag);
    for(eOsdId=E_DRV_OSD_PORT0;eOsdId<E_DRV_OSD_ID_MAX;eOsdId++)
    {
        if(_DrvOsdTransId(eOsdId, &eHalOsdId) == TRUE)
        {    
            _tOsdpLocSettings[eHalOsdId].bBypassEn = 1;
            _tOsdpLocSettings[eHalOsdId].bOsdEn = 0;
            HalOsdSetOsdBypassForScEnable(eHalOsdId,TRUE);
            HalOsdSetOsdEnable(eHalOsdId,0);
        }
    }
    HalOsdSetBindGop(geGopOsdFlag);
    return bRet;
}
bool DrvOsdSetBindGop(u32 u32GopOsdFlag)
{
    bool bRet = TRUE;
    HalOsdSetBindGop(u32GopOsdFlag);
    return bRet;
}
bool DrvOsdSetEnable(DrvOsdId_e eOsdId, bool bEn)
{
    bool bRet = TRUE;
    HalOsdIdType_e eHalOsdId;
    if(_DrvOsdTransId(eOsdId, &eHalOsdId) == TRUE)
    {
        if(_tOsdpLocSettings[eHalOsdId].bOsdEn != bEn)
        {
            _tOsdpLocSettings[eHalOsdId].bOsdEn = bEn;
            HalOsdSetOsdEnable(eHalOsdId,bEn);
        }
        else
        {
            DRVRGNHLDBG("%s %d: Not need to exec\n", __FUNCTION__, __LINE__);
        }
    }
    return bRet;
}

bool DrvOsdSetColorInverseEn(DrvOsdId_e eOsdId, bool bEn) {
    bool bRet = TRUE;
    HalOsdIdType_e eHalOsdId;
    if(_DrvOsdTransId(eOsdId, &eHalOsdId) == TRUE)
    {    
        _tOsdpLocSettings[eHalOsdId].bColorInv = bEn;
        HalOsdSetColorInverseEn(eHalOsdId, bEn);
    }
    else
    {
        DRVRGNERR("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvOsdSetColorInverseParam(DrvOsdId_e eOsdId, DrvOsdColorInvWindowIdType_e eAeId, DrvOsdColorInvParamConfig_t *ptColInvCfg)
{
    bool bRet = TRUE;
    HalOsdIdType_e eHalOsdId;
    HalOsdColorInvParamConfig_t tHalColInvCfg;
    memcpy(&tHalColInvCfg,ptColInvCfg,sizeof(HalOsdColorInvParamConfig_t));
    if(_DrvOsdTransId(eOsdId, &eHalOsdId) && HalOsdColInvParamCheck(&tHalColInvCfg))
    {
        memcpy(&_tOsdpLocSettings[eHalOsdId].stColorInvCfg[eAeId],ptColInvCfg,sizeof(DrvOsdColorInvParamConfig_t));
        HalOsdSetColorInverseParam(eHalOsdId, (HalOsdColorInvWindowIdType_e)eAeId, &tHalColInvCfg);
    }
    else
    {
        DRVRGNERR("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvOsdSetColorInverseUpdate(DrvOsdId_e eOsdId)
{
    bool bRet = TRUE;
    HalOsdIdType_e eHalOsdId;
    if(_DrvOsdTransId(eOsdId, &eHalOsdId) == TRUE)
    {
        HalOsdSetColorInverseUpdate(eHalOsdId);
    }
    else
    {
        DRVRGNERR("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvOsdWriteColorInverseData(DrvOsdId_e eOsdId, u16 addr, u32 wdata)
{
    bool bRet = TRUE;
    HalOsdIdType_e eHalOsdId;
    if(_DrvOsdTransId(eOsdId, &eHalOsdId) == TRUE)
    {
        HalOsdWriteColorInverseData(eHalOsdId, addr, wdata);
    }
    else
    {
        DRVRGNERR("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvOsdReadColorInverseData(DrvOsdId_e eOsdId, u16 addr, u32 *rdata)
{
    bool bRet = TRUE;
    HalOsdIdType_e eHalOsdId;
    if(_DrvOsdTransId(eOsdId, &eHalOsdId) == TRUE)
    {
        HalOsdReadColorInverseData(eHalOsdId, addr, rdata);
    }
    else
    {
        DRVRGNERR("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}
