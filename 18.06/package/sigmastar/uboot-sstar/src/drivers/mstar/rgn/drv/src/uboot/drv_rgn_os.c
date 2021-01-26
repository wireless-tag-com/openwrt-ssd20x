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

#include "drv_rgn_os.h"
#include "hal_gop.h"
bool _DrvGopTransId(DrvGopIdType_e eGopId, HalGopIdType_e *pHalId);
bool DrvRgnOsSetGopClkEnable(DrvGopIdType_e eGopId)
{
    HalGopIdType_e enType;
    if(!_DrvGopTransId(eGopId,&enType))
    {
        return 0;
    }
    return HalGopSetClkEnable(enType);
}

bool DrvRgnOsSetGopClkDisable(DrvGopIdType_e eGopId,bool bEn)
{
    HalGopIdType_e enType;
    if(!_DrvGopTransId(eGopId,&enType))
    {
        return 0;
    }
    return HalGopSetClkDisable(enType,bEn);
}
