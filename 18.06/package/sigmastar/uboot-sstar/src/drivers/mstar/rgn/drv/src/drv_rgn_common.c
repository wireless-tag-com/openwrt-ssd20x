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

#define __DRV_RGN_COMMON_C__


//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "drv_rgn_common.h"
#include "rgn_sysfs.h"

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

bool _DrvRgnTransCmdqId(DrvRgnCmdqIdType_e eCmdqId,HalRgnCmdqIdType_e *pHalCmdqId)
{
    bool bRet = TRUE;

    if(eCmdqId == E_DRV_RGN_VPE_CMDQ_ID_0)
    {
        *pHalCmdqId = E_HAL_RGN_CMDQ_VPE_ID_0;
        return bRet;
    }

    if(eCmdqId == E_DRV_RGN_DIVP_CMDQ_ID_1)
    {
        *pHalCmdqId = E_HAL_RGN_CMDQ_DIVP_ID_1;
        return bRet;
    }

    if(eCmdqId == E_DRV_RGN_LDC_CMDQ_ID_2)
    {
        *pHalCmdqId = E_HAL_RGN_CMDQ_LDC_ID_2;
        return bRet;
    }

    return FALSE;
}
//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
bool DrvRgnUpdateReg(void)
{
    HalRgnUpdateReg();
    return 1;
}
bool DrvRgnInit(void)
{
    HalRgnInit();
    HalRgnInitCmdq();
    return 1;
}
bool DrvRgnDeinit(void)
{
    HalRgnDeinit();
    HalRgnInitCmdq();
    return 1;
}
bool DrvRgnSetOsdBindGop(unsigned int eGopNum)
{
    HalRgnOsdFlag_e eOsdNum = eGopNum;
    if(HalRgnCheckBindRation(&eOsdNum))
    {
        geGopOsdFlag = eGopNum;
        return 1;
    }
    else
    {
        return 0;
    }
}
bool DrvRgnGetOsdBindGop(unsigned int *eGopNum)
{
    *eGopNum = geGopOsdFlag;
    return 1;
}
void DrvRgnUseCmdq(bool bEn)
{
    HalRgnUseCmdq(bEn);
}
bool DrvRgnSetCmdq(MHAL_CMDQ_CmdqInterface_t *pstCmdq,DrvRgnCmdqIdType_e eCmdqId)
{
    bool bRet = FALSE;
    HalRgnCmdqIdType_e eHalCmdqId;
    if(_DrvRgnTransCmdqId(eCmdqId, &eHalCmdqId) == TRUE)
    {
        HalRgnSetCmdq(pstCmdq,eHalCmdqId);
    }
    bRet = TRUE;
    return bRet;
}
