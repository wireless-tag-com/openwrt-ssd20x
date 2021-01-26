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

#define _DRV_DISP_CTX_C_

#include "drv_disp_os.h"
#include "hal_disp_common.h"
#include "hal_disp_if.h"
#include "disp_debug.h"
#include "hal_disp_chip.h"
#include "hal_disp_st.h"
#include "drv_disp_ctx.h"
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DrvDispCtxContain_t gstDispCtxContainTbl[HAL_DISP_CTX_MAX_INST];
bool gbDispCtxInit = 0;

DrvDispCtxConfig_t *pgstCurDispDevCtx[HAL_DISP_DEVICE_MAX];
DrvDispCtxConfig_t *pgstCurVidLayerCtx[HAL_DISP_VIDLAYER_MAX];
DrvDispCtxConfig_t *pgstCurInputPortCtx[HAL_DISP_INPUTPORT_MAX];

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------
bool __DrvDispCtxBindInputPortWidthVidLayer(DrvDispCtxConfig_t *pstVidLayerCtx, u16 u16InputPortIdx)
{
    bool bRet = 1;
    DrvDispCtxVideoLayerContain_t *pstVidLayerContain = NULL;
    DrvDispCtxInputPortContain_t *pstInputPortContain = NULL;

    if(pstVidLayerCtx->enCtxType == E_DISP_CTX_TYPE_VIDLAYER && u16InputPortIdx != HAL_DISP_INPUTPORT_MAX)
    {
        pstVidLayerContain =
            pstVidLayerCtx->pstCtxContain->pstVidLayerContain[pstVidLayerCtx->u32Idx];

        pstInputPortContain =
            pstVidLayerCtx->pstCtxContain->pstInputPortContain[u16InputPortIdx];

        pstVidLayerContain->pstInputPortContain[u16InputPortIdx] = pstInputPortContain;
        pstInputPortContain->pstVidLayerContain = pstVidLayerContain;

        DISP_DBG(DISP_DBG_LEVEL_CTX, "%s %d, VideId=%ld, (%lx) InputPort:%d, (%lx)Bind\n",
            __FUNCTION__, __LINE__,
            pstVidLayerCtx->u32Idx, (u32)pstVidLayerContain, u16InputPortIdx, (u32)pstInputPortContain);
    }
    else
    {
       bRet = 0;
       DISP_ERR("%s %d, Ctx Type is Err %s, \n",
        __FUNCTION__, __LINE__,  PARSING_CTX_TYPE(pstVidLayerCtx->enCtxType));
    }
    return bRet;
}


bool __DrvDispCtxUnBindInputPortWidthVidLayer(DrvDispCtxConfig_t *pstInputPortCtx)
{
    bool bRet = 1;
    DrvDispCtxVideoLayerContain_t *pstVidLayerContain = NULL;
    if(pstInputPortCtx->enCtxType == E_DISP_CTX_TYPE_INPUTPORT)
    {
        pstVidLayerContain = pstInputPortCtx->pstCtxContain->pstInputPortContain[pstInputPortCtx->u32Idx]->pstVidLayerContain;
        pstVidLayerContain->pstInputPortContain[pstInputPortCtx->u32Idx] = NULL;
        pstInputPortCtx->pstCtxContain->pstInputPortContain[pstInputPortCtx->u32Idx]->pstVidLayerContain = NULL;

    DISP_DBG(DISP_DBG_LEVEL_CTX, "%s %d, VideId=%ld, InputPort:%ld, UnBind\n",
        __FUNCTION__, __LINE__, pstVidLayerContain->u32VidLayerId, pstInputPortCtx->u32Idx);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Ctx Type is Err %s, \n",
         __FUNCTION__, __LINE__,  PARSING_CTX_TYPE(pstInputPortCtx->enCtxType));

    }
    return bRet;
}

bool _DrvDispCtxInitDeviceContain(DrvDispCtxContain_t *pDispContain)
{
    u16 i;
    bool bRet = 1;

    for(i=0; i<HAL_DISP_DEVICE_MAX; i++)
    {
        pDispContain->bDevUsed[i] = 0;
        pDispContain->pstDevContain[i] = DrvDispOsMemAlloc(sizeof(DrvDispCtxDeviceContain_t));
        if(pDispContain->pstDevContain[i] == NULL)
        {
            bRet = 0;
            DISP_ERR("%s %d, Alloc Device Contain Fail\n", __FUNCTION__, __LINE__);
            break;
        }
    }
    return bRet;
}

void _DrvDispCtxDeInitDeviceContain(DrvDispCtxContain_t *pDispContain)
{
    u16 i;

    for(i=0; i<HAL_DISP_DEVICE_MAX; i++)
    {
        if(pDispContain->pstDevContain[i])
        {
            DrvDispOsMemRelease(pDispContain->pstDevContain[i]);
            pDispContain->pstDevContain[i] = NULL;
            pDispContain->bDevUsed[i] = 0;
        }
    }
}

bool _DrvDispCtxInitVidLayerContain(DrvDispCtxContain_t *pDispContain)
{
    u16 i;
    bool bRet = 1;

    for(i=0; i<HAL_DISP_VIDLAYER_MAX; i++)
    {
        pDispContain->bVidLayerUsed[i] = 0;
        pDispContain->pstVidLayerContain[i] = DrvDispOsMemAlloc(sizeof(DrvDispCtxVideoLayerContain_t));
        if(pDispContain->pstVidLayerContain[i] == NULL)
        {
            bRet = 0;
            DISP_ERR("%s %d, Alloc VidLayer Contain Fail\n", __FUNCTION__, __LINE__);
            break;
        }
    }
    return bRet;
}
void _DrvDispCtxDeInitVidLayerContain(DrvDispCtxContain_t *pDispContain)
{
    u16 i;

    for(i=0; i<HAL_DISP_VIDLAYER_MAX; i++)
    {
        if(pDispContain->pstVidLayerContain[i])
        {
            DrvDispOsMemRelease(pDispContain->pstVidLayerContain[i]);
            pDispContain->pstVidLayerContain[i] = NULL;
            pDispContain->bVidLayerUsed[i] = 0;
        }
    }
}

bool _DrvDispCtxInitInputPortContain(DrvDispCtxContain_t *pDispContain)
{
    u16 i;
    bool bRet = 1;

    for(i=0; i<HAL_DISP_INPUTPORT_MAX; i++)
    {
        pDispContain->bInputPortUsed[i] = 0;
        pDispContain->pstInputPortContain[i] = DrvDispOsMemAlloc(sizeof(DrvDispCtxInputPortContain_t));
        if(pDispContain->pstInputPortContain[i] == NULL)
        {
            bRet = 0;
            DISP_ERR("%s %d, Alloc InputPort Contain Fail\n", __FUNCTION__, __LINE__);
            break;
        }
    }
    return bRet;

}

void _DrvDispCtxDeInitInputPortContain(DrvDispCtxContain_t *pDispCtxContain)
{
    u16 i;

    for(i=0; i<HAL_DISP_INPUTPORT_MAX; i++)
    {
        if(pDispCtxContain->pstInputPortContain[i])
        {
            DrvDispOsMemRelease(pDispCtxContain->pstInputPortContain[i]);
            pDispCtxContain->pstInputPortContain[i] = NULL;
            pDispCtxContain->bInputPortUsed[i] = 0;
        }
    }
}

bool _DrvDispCtxInitHalContain(DrvDispCtxContain_t *pDispContain)
{
    bool bRet = 1;
    pDispContain->pstHalHwCtx = DrvDispOsMemAlloc(sizeof(DrvDispCtxHwContain_t));

    if(pDispContain->pstHalHwCtx == NULL)
    {
        bRet = 0;
        DISP_ERR("%s %d, Alloc Hal Contain Fail\n", __FUNCTION__, __LINE__);
    }

    return bRet;
}

void _DrvDispCtxDeInitHalContain(DrvDispCtxContain_t *pDispContain)
{
    if(pDispContain->pstHalHwCtx)
    {
        DrvDispOsMemRelease(pDispContain->pstHalHwCtx);
        pDispContain->pstHalHwCtx = NULL;
    }
}

bool _DrvDispCtxAllocDevContain(DrvDispCtxAllocConfig_t *pstAllocCfg, DrvDispCtxConfig_t **pCtx)
{
    u16 i, j, u16EmptyIdx, u16DevIdx;
    bool bRet = 1;
    DrvDispCtxConfig_t *pstDispCtx = NULL;

    u16EmptyIdx = HAL_DISP_CTX_MAX_INST;
    u16DevIdx = HAL_DISP_DEVICE_MAX;

    for(i=0; i<HAL_DISP_CTX_MAX_INST; i++)
    {
        for(j=0; j< HAL_DISP_DEVICE_MAX; j++)
        {
            if(gstDispCtxContainTbl[i].bDevUsed[j] == 0)
            {
                u16EmptyIdx = i;
                u16DevIdx = j;
                break;
            }
        }
    }

    if(u16EmptyIdx != HAL_DISP_CTX_MAX_INST && u16DevIdx != HAL_DISP_DEVICE_MAX)
    {
        pstDispCtx = DrvDispOsMemAlloc(sizeof(DrvDispCtxConfig_t));

        if(pstDispCtx)
        {
            pstDispCtx->enCtxType = E_DISP_CTX_TYPE_DEVICE;
            pstDispCtx->u32Idx = u16DevIdx;
            pstDispCtx->pstCtxContain = &gstDispCtxContainTbl[u16EmptyIdx];

            memset(pstDispCtx->pstCtxContain->pstDevContain[u16DevIdx], 0, sizeof(DrvDispCtxDeviceContain_t));
            memset(pstDispCtx->pstCtxContain->pstHalHwCtx, 0, sizeof(DrvDispCtxHwContain_t));

            pstDispCtx->pstCtxContain->bDevUsed[u16DevIdx] = 1;
            pstDispCtx->pstCtxContain->pstDevContain[u16DevIdx]->u32DevId = pstAllocCfg->u32Id;
            pstDispCtx->pstCtxContain->stMemAllcCfg.alloc = pstAllocCfg->stMemAllcCfg.alloc;
            pstDispCtx->pstCtxContain->stMemAllcCfg.free = pstAllocCfg->stMemAllcCfg.free;
            *pCtx = pstDispCtx;
        }
        else
        {
            bRet = 0;
            *pCtx = NULL;
            DISP_ERR("%s %d, Alloc Ctx Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = 0;
        *pCtx = NULL;
        DISP_ERR("%s %d, Alloc Dev Contain Fail\n", __FUNCTION__, __LINE__);
    }

    return bRet;
}

bool _DrvDispCtxFreeDevContain(DrvDispCtxConfig_t *pCtx)
{
    DrvDispCtxDeviceContain_t *pstDevContain = NULL;
    bool bRet = 1;

    if(pCtx->enCtxType == E_DISP_CTX_TYPE_DEVICE)
    {
        pstDevContain = pCtx->pstCtxContain->pstDevContain[pCtx->u32Idx];
        memset(pstDevContain, 0, sizeof(DrvDispCtxDeviceContain_t));
        pCtx->pstCtxContain->bDevUsed[pCtx->u32Idx] = 0;
        DrvDispOsMemRelease(pCtx);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Ctx Type Err, %s\n",
            __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pCtx->enCtxType));
    }
    return bRet;
}

bool _DrvDispCtxAllocVidLayerContain(DrvDispCtxAllocConfig_t *pstAllocCfg, DrvDispCtxConfig_t **pCtx)
{
    u16 i, j, u16EmptyIdx, u16VidLayerIdx;
    DrvDispCtxConfig_t *pstDispCtx = NULL;
    bool bRet = 1;

    u16EmptyIdx = HAL_DISP_CTX_MAX_INST;
    u16VidLayerIdx = HAL_DISP_VIDLAYER_MAX;

    for(i=0; i<HAL_DISP_CTX_MAX_INST; i++)
    {
        for(j=0; j< HAL_DISP_VIDLAYER_MAX; j++)
        {
            if(gstDispCtxContainTbl[i].bVidLayerUsed[j] == 0)
            {
                u16EmptyIdx = i;
                u16VidLayerIdx = j;
                break;
            }
        }
    }

    if(u16EmptyIdx != HAL_DISP_CTX_MAX_INST && u16VidLayerIdx != HAL_DISP_VIDLAYER_MAX)
    {
        pstDispCtx = DrvDispOsMemAlloc(sizeof(DrvDispCtxConfig_t));

        if(pstDispCtx)
        {
            pstDispCtx->enCtxType = E_DISP_CTX_TYPE_VIDLAYER;
            pstDispCtx->u32Idx = u16VidLayerIdx;
            pstDispCtx->pstCtxContain = &gstDispCtxContainTbl[u16EmptyIdx];

            memset(pstDispCtx->pstCtxContain->pstVidLayerContain[u16VidLayerIdx], 0, sizeof(DrvDispCtxVideoLayerContain_t));
            pstDispCtx->pstCtxContain->bVidLayerUsed[u16VidLayerIdx] = 1;
            pstDispCtx->pstCtxContain->pstVidLayerContain[u16VidLayerIdx]->u32VidLayerId = pstAllocCfg->u32Id;
            *pCtx = pstDispCtx;
        }
        else
        {
            *pCtx = NULL;
            bRet = 0;
            DISP_ERR("%s %d, Alloc Ctx Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        *pCtx = NULL;
        bRet = 0;
        DISP_ERR("%s %d, Alloc VidLayer Contain Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}

bool _DrvDispCtxFreeVidLayerContain(DrvDispCtxConfig_t *pCtx)
{
    DrvDispCtxVideoLayerContain_t *pstVidLayerContain = NULL;
    bool bRet = 1;

    if(pCtx->enCtxType == E_DISP_CTX_TYPE_VIDLAYER)
    {
        pstVidLayerContain = pCtx->pstCtxContain->pstVidLayerContain[pCtx->u32Idx];
        memset(pstVidLayerContain, 0, sizeof(DrvDispCtxInputPortContain_t));
        pCtx->pstCtxContain->bVidLayerUsed[pCtx->u32Idx] = 0;
        DrvDispOsMemRelease(pCtx);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Ctx Type Err, %s\n",
            __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pCtx->enCtxType));
    }
    return bRet;
}

bool _DrvDispCtxAllocInputPortContain(DrvDispCtxAllocConfig_t *pstAllocCfg, DrvDispCtxConfig_t **pCtx)
{
    u16 i, j, u16EmptyIdx, u16InputPortIdx;
    DrvDispCtxConfig_t *pstDispCtx = NULL;
    bool bRet = 1;

    u16EmptyIdx = HAL_DISP_CTX_MAX_INST;
    u16InputPortIdx = HAL_DISP_INPUTPORT_MAX;

    for(i=0; i<HAL_DISP_CTX_MAX_INST; i++)
    {
        for(j=0; j< HAL_DISP_INPUTPORT_MAX; j++)
        {
            if(gstDispCtxContainTbl[i].bInputPortUsed[j] == 0)
            {
                u16EmptyIdx = i;
                u16InputPortIdx = j;
                break;
            }
        }
    }

    if(u16EmptyIdx != HAL_DISP_CTX_MAX_INST && u16InputPortIdx != HAL_DISP_INPUTPORT_MAX)
    {
        pstDispCtx = DrvDispOsMemAlloc(sizeof(DrvDispCtxConfig_t));

        if(pstDispCtx)
        {
            pstDispCtx->enCtxType = E_DISP_CTX_TYPE_INPUTPORT;
            pstDispCtx->u32Idx = u16InputPortIdx;
            pstDispCtx->pstCtxContain = &gstDispCtxContainTbl[u16EmptyIdx];

            memset(pstDispCtx->pstCtxContain->pstInputPortContain[u16InputPortIdx], 0, sizeof(DrvDispCtxInputPortContain_t));

            if(__DrvDispCtxBindInputPortWidthVidLayer(pstAllocCfg->pstBindCtx, u16InputPortIdx))
            {
                pstDispCtx->pstCtxContain->bInputPortUsed[u16InputPortIdx] = 1;
                pstDispCtx->pstCtxContain->pstInputPortContain[u16InputPortIdx]->u32PortId = pstAllocCfg->u32Id;
                *pCtx = pstDispCtx;
            }
            else
            {
                DrvDispOsMemRelease(pstDispCtx);
                *pCtx = NULL;
                bRet = 0;
                DISP_ERR("%s %d, Bind Fail\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            *pCtx = NULL;
            bRet = 0;
            DISP_ERR("%s %d, Alloc Ctx Fail\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        *pCtx = NULL;
        bRet = 0;
        DISP_ERR("%s %d, Alloc InputPort Contain Fail\n", __FUNCTION__, __LINE__);
    }
    return bRet;
}


bool _DrvDispCtxFreeInputPortContain(DrvDispCtxConfig_t *pCtx)
{
    DrvDispCtxInputPortContain_t *pstInputPortContain = NULL;
    bool bRet = 1;

    if(pCtx->enCtxType == E_DISP_CTX_TYPE_INPUTPORT)
    {
        if(__DrvDispCtxUnBindInputPortWidthVidLayer(pCtx))
        {
            pstInputPortContain = pCtx->pstCtxContain->pstInputPortContain[pCtx->u32Idx];
            memset(pstInputPortContain, 0, sizeof(DrvDispCtxInputPortContain_t));
            pCtx->pstCtxContain->bInputPortUsed[pCtx->u32Idx] = 0;
            DrvDispOsMemRelease(pCtx);
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, UnBind Fail\n", __FUNCTION__, __LINE__ );
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Ctx Type Err, %s\n",
            __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pCtx->enCtxType));
    }
    return bRet;
}
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
bool DrvDispCtxInit(void)
{
    u16 i;
    bool bRet = 1;

    if(gbDispCtxInit == 1)
    {
        return 1;
    }

    HalDispIfInit();

    for(i=0; i<HAL_DISP_CTX_MAX_INST; i++)
    {
        // Device
        if(_DrvDispCtxInitDeviceContain(&gstDispCtxContainTbl[i]) == FALSE)
        {
            _DrvDispCtxDeInitDeviceContain(&gstDispCtxContainTbl[i]);
            bRet = 0;
            break;
        }

        // VideoLayer
        if(_DrvDispCtxInitVidLayerContain(&gstDispCtxContainTbl[i]) == FALSE)
        {
            _DrvDispCtxDeInitVidLayerContain(&gstDispCtxContainTbl[i]);
            _DrvDispCtxDeInitDeviceContain(&gstDispCtxContainTbl[i]);
            bRet = 0;
            break;
        }

        //InputPort
        if(_DrvDispCtxInitInputPortContain(&gstDispCtxContainTbl[i]) == FALSE)
        {
            _DrvDispCtxDeInitInputPortContain(&gstDispCtxContainTbl[i]);
            _DrvDispCtxDeInitVidLayerContain(&gstDispCtxContainTbl[i]);
            _DrvDispCtxDeInitDeviceContain(&gstDispCtxContainTbl[i]);
            bRet = 0;
            break;
        }

        // HalContain
        if(_DrvDispCtxInitHalContain(&gstDispCtxContainTbl[i]) == FALSE)
        {
            _DrvDispCtxDeInitHalContain(&gstDispCtxContainTbl[i]);
            _DrvDispCtxDeInitInputPortContain(&gstDispCtxContainTbl[i]);
            _DrvDispCtxDeInitVidLayerContain(&gstDispCtxContainTbl[i]);
            _DrvDispCtxDeInitDeviceContain(&gstDispCtxContainTbl[i]);
            bRet = 0;
            break;
        }

        // MemAlloc
        gstDispCtxContainTbl[i].stMemAllcCfg.alloc = NULL;
        gstDispCtxContainTbl[i].stMemAllcCfg.free = NULL;
    }

    gbDispCtxInit = 1;
    return bRet;
}


bool DrvDispCtxDeInit(void)
{
    u16 i;
    bool bRet = 1;

    if(gbDispCtxInit == 0)
    {
        bRet = 0;
        DISP_ERR("%s %d, Ctx not Init\n", __FUNCTION__, __LINE__);
    }
    else
    {
        for(i=0; i<HAL_DISP_CTX_MAX_INST; i++)
        {
            _DrvDispCtxDeInitHalContain(&gstDispCtxContainTbl[i]);
            _DrvDispCtxDeInitInputPortContain(&gstDispCtxContainTbl[i]);
            _DrvDispCtxDeInitVidLayerContain(&gstDispCtxContainTbl[i]);
            _DrvDispCtxDeInitDeviceContain(&gstDispCtxContainTbl[i]);
            gstDispCtxContainTbl[i].stMemAllcCfg.alloc = NULL;
            gstDispCtxContainTbl[i].stMemAllcCfg.free = NULL;
        }

        for(i=0; i< HAL_DISP_DEVICE_MAX; i++)
        {
            pgstCurDispDevCtx[i] = NULL;
        }

        for(i=0; i< HAL_DISP_VIDLAYER_MAX; i++)
        {
            pgstCurVidLayerCtx[i] = NULL;
        }

        for(i=0; i< HAL_DISP_INPUTPORT_MAX; i++)
        {
            pgstCurInputPortCtx[i] = NULL;
        }

        HalDispIfDeInit();
        gbDispCtxInit = 0;
    }

    return bRet;

}

bool DrvDispCtxAllocate(DrvDispCtxAllocConfig_t *pstAllocCfg, DrvDispCtxConfig_t **pCtx)
{
    bool bRet = 1;

    if(pstAllocCfg->enType == E_DISP_CTX_TYPE_DEVICE)
    {
        bRet = _DrvDispCtxAllocDevContain(pstAllocCfg, pCtx);
    }
    else if(pstAllocCfg->enType == E_DISP_CTX_TYPE_VIDLAYER)
    {
        bRet = _DrvDispCtxAllocVidLayerContain(pstAllocCfg, pCtx);
    }
    else if(pstAllocCfg->enType == E_DISP_CTX_TYPE_INPUTPORT)
    {
        bRet = _DrvDispCtxAllocInputPortContain(pstAllocCfg, pCtx);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Alloc Type is Err %s\n",
            __FUNCTION__, __LINE__,
            PARSING_CTX_TYPE(pstAllocCfg->enType));
    }

    return bRet;
}

bool DrvDispCtxFree(DrvDispCtxConfig_t *pCtx)
{
    bool bRet = 1;

    if(pCtx->enCtxType == E_DISP_CTX_TYPE_DEVICE)
    {
        bRet = _DrvDispCtxFreeDevContain(pCtx);

    }
    else if(pCtx->enCtxType == E_DISP_CTX_TYPE_VIDLAYER)
    {
        bRet = _DrvDispCtxFreeVidLayerContain(pCtx);
    }
    else if(pCtx->enCtxType == E_DISP_CTX_TYPE_INPUTPORT)
    {
        bRet = _DrvDispCtxFreeInputPortContain(pCtx);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, Alloc Type is Err %s\n",
            __FUNCTION__, __LINE__,
            PARSING_CTX_TYPE(pCtx->enCtxType));
    }
    return bRet;
}

bool DrvDispCtxIsAllFree(void)
{
    u16 i, j;
    bool bContainUse = 0;

    for(i=0; i< HAL_DISP_CTX_MAX_INST; i++)
    {

        for(j=0; j< HAL_DISP_DEVICE_MAX; j++)
        {
            bContainUse |= gstDispCtxContainTbl[i].bDevUsed[j];
        }

        for(j=0; j< HAL_DISP_VIDLAYER_MAX; j++)
        {
            bContainUse |= gstDispCtxContainTbl[i].bVidLayerUsed[j];
        }

        for(j=0; j< HAL_DISP_INPUTPORT_MAX; j++)
        {
            bContainUse |= gstDispCtxContainTbl[i].bInputPortUsed[j];
        }
    }

    return bContainUse ? 0 : 1;
}

bool DrvDispCtxSetCurCtx(DrvDispCtxConfig_t *pCtx, u32 u32Idx)
{
    bool bRet = TRUE;;
    if(pCtx->enCtxType == E_DISP_CTX_TYPE_DEVICE && u32Idx < HAL_DISP_DEVICE_MAX)
    {
        pgstCurDispDevCtx[u32Idx] = pCtx;
    }
    else if(pCtx->enCtxType == E_DISP_CTX_TYPE_VIDLAYER && u32Idx < HAL_DISP_VIDLAYER_MAX)
    {
        pgstCurVidLayerCtx[u32Idx] = pCtx;
    }
    else if(pCtx->enCtxType == E_DISP_CTX_TYPE_INPUTPORT&& u32Idx < HAL_DISP_INPUTPORT_MAX)
    {
        pgstCurInputPortCtx[u32Idx] = pCtx;
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, SetCurCtx Type is Err %s\n",
            __FUNCTION__, __LINE__,
            PARSING_CTX_TYPE(pCtx->enCtxType));
    }
    return bRet;
}

bool DrvDispCtxGetCurCtx(DrvDispCtxType_e enCtxType, u32 u32Idx, DrvDispCtxConfig_t **pCtx)
{
    bool bRet = TRUE;

    if(enCtxType == E_DISP_CTX_TYPE_DEVICE && u32Idx < HAL_DISP_DEVICE_MAX)
    {
        *pCtx = pgstCurDispDevCtx[u32Idx];

    }
    else if(enCtxType == E_DISP_CTX_TYPE_VIDLAYER && u32Idx < HAL_DISP_VIDLAYER_MAX)
    {
        *pCtx = pgstCurVidLayerCtx[u32Idx];
    }
    else if(enCtxType == E_DISP_CTX_TYPE_INPUTPORT && u32Idx < HAL_DISP_INPUTPORT_MAX)
    {
        *pCtx = pgstCurInputPortCtx[u32Idx];
    }
    else
    {
        bRet = FALSE;
        *pCtx = NULL;
        DISP_ERR("%s %d, GetCurCtx Fail, Type=%s, Idx=%ld\n",
            __FUNCTION__, __LINE__,
            PARSING_CTX_TYPE(enCtxType), u32Idx);
    }
    return bRet;
}


