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

#define __DRV_COVER_C__
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_cover.h"
#include "hal_cover.h"

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
    bool bBwinEn;
    DrvCoverWindowConfig_t tWinSet;
    DrvCoverColorConfig_t tWinColorSet;
} _DrvCoverWinLocalSettingsConfig_t;

typedef struct
{
    bool bDbEn;
    _DrvCoverWinLocalSettingsConfig_t tCoverWinsSet[E_HAL_COVER_BWIN_ID_NUM];
} _DrvCoverLocalSettingsConfig_t;

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

_DrvCoverLocalSettingsConfig_t _tCoverLocSettings[E_DRV_COVER_ID_NUM];

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

bool _DrvCoverTransId(DrvCoverIdType_e eCoverId, HalCoverIdType_e *pHalId)
{
    bool bRet = TRUE;

    switch(eCoverId)
    {
        case E_DRV_ISPSC0_COVER:
            *pHalId = E_HAL_ISPSCL0_COVER;
            break;
            
        case E_DRV_ISPSC1_COVER:
            *pHalId = E_HAL_ISPSCL1_COVER;
            break;
            
        case E_DRV_ISPSC2_COVER:
            *pHalId = E_HAL_ISPSCL2_COVER;
            break;
            
        case E_DRV_ISPSC3_COVER:
            *pHalId = E_HAL_ISPSCL3_COVER;
            break;

        case E_DRV_DIP_COVER:
            *pHalId = E_HAL_DIP_COVER_ID;
            break;

        default:
            *pHalId = E_HAL_COVER_ID_NUM;
            bRet = FALSE;
            break;

    }
    return bRet;
}
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------


bool DrvCoverRgnInit(void)
{
    bool bRet = TRUE;
    u8 u8CoverId;
    u8 u8WinId;
    HalCoverWindowType_t tHalWinCfg;
    HalCoverColorType_t tHalType;
    for(u8CoverId = E_DRV_ISPSC0_COVER; u8CoverId < E_DRV_COVER_ID_NUM; u8CoverId++)
    {
        if(!HalCoverCheckIdSupport(u8CoverId))
        {
            continue;
        }
        _tCoverLocSettings[u8CoverId].bDbEn = FALSE;

        for(u8WinId = E_DRV_COVER_WIN_ID_0; u8WinId < E_DRV_COVER_WIN_ID_NUM; u8WinId++)
        {
            _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].bBwinEn = FALSE;
            tHalType.u8R =  _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinColorSet.u8R= 0;
            tHalType.u8G =  _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinColorSet.u8G= 0;
            tHalType.u8B =  _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinColorSet.u8B = 0;
            tHalWinCfg.u16X =  _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinSet.u16X = 0;
            tHalWinCfg.u16Y =  _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinSet.u16Y= 0;
            tHalWinCfg.u16Width=_tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinSet.u16Width= 0;
            tHalWinCfg.u16Height=_tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].tWinSet.u16Height= 0;

            HalCoverSetEnableWin((DrvCoverIdType_e)u8CoverId, (HalCoverWinIdType_e)u8WinId,
                _tCoverLocSettings[u8CoverId].tCoverWinsSet[u8WinId].bBwinEn);
            HalCoverSetWindowSize((DrvCoverIdType_e)u8CoverId, (HalCoverWinIdType_e)u8WinId, &tHalWinCfg);
            HalCoverSetColor((DrvCoverIdType_e)u8CoverId, (HalCoverWinIdType_e)u8WinId,&tHalType);
        }
    }

    return bRet;
}
bool DrvCoverSetWinSize(DrvCoverIdType_e eCoverId, DrvCoverWinIdType_e eWinId, DrvCoverWindowConfig_t *ptWinCfg)
{
    bool bRet = FALSE;
    HalCoverIdType_e eHalCoverId;
    HalCoverWindowType_t tHalWinCfg;

    if(_DrvCoverTransId(eCoverId, &eHalCoverId) == TRUE)
    {
        if(HalCoverCheckIdSupport(eHalCoverId))
        {
            if(_tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinSet.u16X != ptWinCfg->u16X ||
            _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinSet.u16Y != ptWinCfg->u16Y ||
            _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinSet.u16Width != ptWinCfg->u16Width ||
            _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinSet.u16Height != ptWinCfg->u16Height)
            {
                tHalWinCfg.u16X = _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinSet.u16X = ptWinCfg->u16X;
                tHalWinCfg.u16Y = _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinSet.u16Y = ptWinCfg->u16Y;
                tHalWinCfg.u16Width = _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinSet.u16Width = ptWinCfg->u16Width;
                tHalWinCfg.u16Height = _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinSet.u16Height = ptWinCfg->u16Height;
                HalCoverSetWindowSize(eHalCoverId, (HalCoverWinIdType_e)eWinId, &tHalWinCfg);
            }
            else
            {
                DRVRGNHLDBG("%s %d: Not need to exec\n", __FUNCTION__, __LINE__);
            }
            bRet = TRUE;
        }
    }
    else
    {
        DRVRGNERR("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvCoverSetColor(DrvCoverIdType_e eCoverId, DrvCoverWinIdType_e eWinId, DrvCoverColorConfig_t *ptColorCfg)
{
    bool bRet = FALSE;
    HalCoverIdType_e eHalCoverId;
    HalCoverColorType_t tHalType;
    if(_DrvCoverTransId(eCoverId, &eHalCoverId) == TRUE && (HalCoverCheckIdSupport(eHalCoverId)))
    {
        if(HalCoverCheckIdSupport(eHalCoverId))
        {
            if(_tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinColorSet.u8R != ptColorCfg->u8R ||
            _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinColorSet.u8G != ptColorCfg->u8G ||
            _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinColorSet.u8B != ptColorCfg->u8B)
            {
                tHalType.u8R =  _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinColorSet.u8R= ptColorCfg->u8R;
                tHalType.u8G =  _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinColorSet.u8G= ptColorCfg->u8G;
                tHalType.u8B =  _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].tWinColorSet.u8B = ptColorCfg->u8B;
                HalCoverSetColor(eHalCoverId, (HalCoverWinIdType_e)eWinId, &tHalType);
            }
            else
            {
                DRVRGNHLDBG("%s %d: Not need to exec\n", __FUNCTION__, __LINE__);
            }
            bRet = TRUE;
        }
    }
    else
    {
        DRVRGNERR("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}

bool DrvCoverSetEnableWin(DrvCoverIdType_e eCoverId,  DrvCoverWinIdType_e eWinId, bool bEn)
{
    bool bRet = FALSE;
    HalCoverIdType_e eHalCoverId;
    if(_DrvCoverTransId(eCoverId, &eHalCoverId) == TRUE)
    {
        if(HalCoverCheckIdSupport(eHalCoverId))
        {
            if(_tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].bBwinEn != bEn)
            {
                _tCoverLocSettings[eCoverId].tCoverWinsSet[eWinId].bBwinEn= bEn;
                HalCoverSetEnableWin(eHalCoverId, (HalCoverWinIdType_e)eWinId, bEn);
            }
            else
            {
                DRVRGNHLDBG("%s %d: Not need to exec\n", __FUNCTION__, __LINE__);
            }
            bRet = TRUE;
        }
    }
    else
    {
        DRVRGNERR("%s %d: Fail ID\n", __FUNCTION__, __LINE__);
        bRet = FALSE;
    }
    return bRet;
}
