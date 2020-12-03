/*
* cam_os_informal_idr.c- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/


///////////////////////////////////////////////////////////////////////////////
/// @file      cam_os_informal_idr.c
/// @brief     Cam OS Informal IDR Source File for Linux User Space and RTK.
///            It's Not A Standard IDR Algorithm.
///////////////////////////////////////////////////////////////////////////////

#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
#include <stdio.h>
#include <string.h>
#include "cam_os_wrapper.h"
#include "cam_os_util_bitmap.h"

#define IDR_ENTRY_NUM   0x4000

typedef struct
{
    void **ppEntryPtr;
    unsigned long *pBitmap;
    unsigned long entry_num;
} CamOsInformalIdr_t, *pCamOsInformalIdr;

CamOsRet_e _CamOsIdrInit(CamOsIdr_t *ptIdr, u32 nEntryNum)
{
    CamOsRet_e eRet = CAM_OS_OK;
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)ptIdr;

    if (pInformalIdr)
    {
        pInformalIdr->entry_num = (nEntryNum)? nEntryNum : IDR_ENTRY_NUM;

        if (NULL == (pInformalIdr->ppEntryPtr = CamOsMemAlloc(sizeof(void *)*pInformalIdr->entry_num)))
            eRet = CAM_OS_ALLOCMEM_FAIL;

        if (NULL == (pInformalIdr->pBitmap = CamOsMemAlloc(sizeof(unsigned long)*CAM_OS_BITS_TO_LONGS(pInformalIdr->entry_num))))
        {
            CamOsMemRelease(pInformalIdr->ppEntryPtr);
            pInformalIdr->ppEntryPtr = NULL;
            eRet = CAM_OS_ALLOCMEM_FAIL;
        }

        memset(pInformalIdr->ppEntryPtr, 0, sizeof(void *)*pInformalIdr->entry_num);
        memset(pInformalIdr->pBitmap, 0, sizeof(unsigned long)*CAM_OS_BITS_TO_LONGS(pInformalIdr->entry_num));
    }
    else
        eRet = CAM_OS_PARAM_ERR;

    return eRet;
}

void _CamOsIdrDestroy(CamOsIdr_t *ptIdr)
{
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)ptIdr;

    if (pInformalIdr)
    {
        if (pInformalIdr->ppEntryPtr)
            CamOsMemRelease(pInformalIdr->ppEntryPtr);

        if (pInformalIdr->pBitmap)
            CamOsMemRelease(pInformalIdr->pBitmap);
    }
}

s32 _CamOsIdrAlloc(CamOsIdr_t *ptIdr, void *pDataPtr, s32 nStart, s32 nEnd)
{
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)ptIdr;
    s32 nEmptyID=-1;

    if (pInformalIdr && pDataPtr && pInformalIdr->ppEntryPtr)
    {
        if (nEnd < nStart || nEnd == 0)
            nEnd = pInformalIdr->entry_num - 1;

        nEmptyID = CAM_OS_FIND_NEXT_ZERO_BIT(pInformalIdr->pBitmap, pInformalIdr->entry_num, nStart);

        if (nEmptyID < nStart || nEmptyID > nEnd)
            nEmptyID = -1;
        else
        {
            pInformalIdr->ppEntryPtr[nEmptyID] = pDataPtr;
            CAM_OS_SET_BIT(nEmptyID, pInformalIdr->pBitmap);
        }
    }

    return nEmptyID;
}

void _CamOsIdrRemove(CamOsIdr_t *ptIdr, s32 nId)
{
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)ptIdr;

    if (pInformalIdr && pInformalIdr->ppEntryPtr)
    {
        pInformalIdr->ppEntryPtr[nId] = NULL;
        CAM_OS_CLEAR_BIT(nId, pInformalIdr->pBitmap);
    }
}

void *_CamOsIdrFind(CamOsIdr_t *ptIdr, s32 nId)
{
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)ptIdr;

    if (pInformalIdr && pInformalIdr->ppEntryPtr)
    {
        if (CAM_OS_TEST_BIT(nId, pInformalIdr->pBitmap))
        {
            return pInformalIdr->ppEntryPtr[nId];
        }
    }

    return NULL;
}
#endif
