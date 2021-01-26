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

#define _DRV_PNL_CTX_C_

//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_pnl_os.h"
#include "hal_pnl_common.h"
#include "pnl_debug.h"
#include "hal_pnl_chip.h"
#include "hal_pnl_st.h"
#include "drv_pnl_ctx.h"
#include "drv_pnl_os.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DrvPnlCtxConfig_t gstPnlCtxTbl[HAL_PNL_CTX_INST_MAX];
bool gbPnlCtxInit = FALSE;

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
bool DrvPnlCtxInit(void)
{
    u16 i;
    bool bRet = 1;

    if(gbPnlCtxInit)
    {
        return 1;
    }

    gbPnlCtxInit = 1;

    for(i=0; i<HAL_PNL_CTX_INST_MAX; i++)
    {
        gstPnlCtxTbl[i].bUsed = 0;
        gstPnlCtxTbl[i].s16CtxId = -1;

        gstPnlCtxTbl[i].pstHalCtx = DrvPnlOsMemAlloc(sizeof(DrvPnlCtxHalContain_t));

        if(gstPnlCtxTbl[i].pstHalCtx == NULL)
        {
            bRet = 0;
            PNL_ERR("[PNLCTX] %s %d, Allocate Mhal Ctx Fail\n", __FUNCTION__, __LINE__);
            break;
        }

        gstPnlCtxTbl[i].pstHwCtx = DrvPnlOsMemAlloc(sizeof(DrvPnlCtxHwContain_t));

        if(gstPnlCtxTbl[i].pstHwCtx == NULL)
        {
            DrvPnlOsMemRelease(gstPnlCtxTbl[i].pstHalCtx);
            bRet = 0;

            PNL_ERR("[PNLCTX] %s %d, Allocate Hal Ctx Fail\n", __FUNCTION__, __LINE__);
            break;
        }
    }

    return bRet;
}


bool DrvPnlCtxDeInit(void)
{
    u16 i;

    if(gbPnlCtxInit == 0)
    {
        PNL_ERR("%s %d, Pnl Ctx not Inited\n", __FUNCTION__, __LINE__);
        return 0;
    }

    for(i=0; i<HAL_PNL_CTX_INST_MAX; i++)
    {
        gstPnlCtxTbl[i].bUsed = 0;
        gstPnlCtxTbl[i].s16CtxId= -1;
        gstPnlCtxTbl[i].u16Id = i;
        DrvPnlOsMemRelease(gstPnlCtxTbl[i].pstHalCtx);
        DrvPnlOsMemRelease(gstPnlCtxTbl[i].pstHwCtx);
        gstPnlCtxTbl[i].pstHalCtx = NULL;
        gstPnlCtxTbl[i].pstHwCtx = NULL;
    }
    gbPnlCtxInit = 0;

    return 1;

}

bool DrvPnlCtxIsAllFree(void)
{
    u16 i;
    bool bFree = 1;

    for(i=0; i<HAL_PNL_CTX_INST_MAX; i++)
    {
        if(gstPnlCtxTbl[i].bUsed)
        {
            bFree = 0;
            break;
        }
    }
    return bFree;
}

bool DrvPnlCtxGet(DrvPnlCtxAllocConfig_t *pAllocCfg, DrvPnlCtxConfig_t **pCtx)
{
    u16 i;
    bool bRet = 1;

    if(pAllocCfg == NULL)
    {
        PNL_ERR("[PNLCTX] %40s %d, CtxConfig is Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        *pCtx = NULL;

        for(i=0; i<HAL_PNL_CTX_INST_MAX; i++)
        {
            if(gstPnlCtxTbl[i].bUsed == 1 && gstPnlCtxTbl[i].s16CtxId == pAllocCfg->s16CtxId)
            {
                *pCtx = &gstPnlCtxTbl[i];
                PNL_DBG(PNL_DBG_LEVEL_CTX, "[PNLCTX] %40s %d, Ctx Allocated, Id=%d\n",
                    __FUNCTION__, __LINE__, gstPnlCtxTbl[i].u16Id);
                break;
            }
        }
    }

    if(*pCtx == NULL)
    {
        bRet = 0;
    }
    else
    {
        bRet = 1;
    }

    return bRet;
}

bool DrvPnlCtxAllocate(DrvPnlCtxAllocConfig_t *pAllocCfg, DrvPnlCtxConfig_t **pCtx)
{
    u16 i;
    bool bRet = 1;

    if(pAllocCfg == NULL)
    {
        PNL_ERR("[PNLCTX] %40s %d, CtxConfig is Empty\n", __FUNCTION__, __LINE__);
        bRet = 0;
    }
    else
    {
        *pCtx = NULL;

        for(i=0; i<HAL_PNL_CTX_INST_MAX; i++)
        {
            if(gstPnlCtxTbl[i].bUsed == 0)
            {
                gstPnlCtxTbl[i].bUsed = 1;
                gstPnlCtxTbl[i].s16CtxId = pAllocCfg->s16CtxId;
                memset(gstPnlCtxTbl[i].pstHalCtx, 0, sizeof(DrvPnlCtxHalContain_t));
                memset(gstPnlCtxTbl[i].pstHwCtx, 0, sizeof(DrvPnlCtxHwContain_t));

                *pCtx = &gstPnlCtxTbl[i];
                PNL_DBG(PNL_DBG_LEVEL_CTX, "[PNLCTX] %40s %d, Ctx Allocated, Id=%d\n",
                    __FUNCTION__, __LINE__, gstPnlCtxTbl[i].u16Id);
                break;
            }
        }
    }

    if(*pCtx == NULL)
    {
        bRet = 0;
    }
    else
    {
        bRet = 1;
    }

    return bRet;
}

bool DrvPnlCtxFree(DrvPnlCtxConfig_t *pCtx)
{
    bool bRet = 1;

    if(pCtx == NULL)
    {
        bRet = 0;
        PNL_ERR("[PNLCTX] %40s %d, CtxConfig is Empty\n", __FUNCTION__, __LINE__);
    }
    else
    {
        pCtx->bUsed = 0;
        pCtx->s16CtxId = -1;
        memset(pCtx->pstHalCtx, 0, sizeof(DrvPnlCtxHalContain_t));
        memset(pCtx->pstHwCtx, 0, sizeof(DrvPnlCtxHwContain_t));
    }
    return bRet;
}


