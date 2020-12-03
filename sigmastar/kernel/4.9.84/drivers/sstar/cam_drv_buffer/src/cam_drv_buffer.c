/*
* cam_drv_buffer.c- Sigmastar
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
/// @file      cam_drv_buffer.c
/// @brief     Cam Drv Buffer Source File for
///            1. RTK OS
///            2. Linux User Space
///            3. Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#if defined(__KERNEL__)
#define CAM_OS_LINUX_KERNEL
#endif

#ifdef CAM_OS_RTK
#error Cam_Drv_Buffer is unsupport RTK OS now!
#elif defined(CAM_OS_LINUX_USER)
#error Cam_Drv_Buffer is unsupport kernel user space now!
#elif defined(CAM_OS_LINUX_KERNEL)
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/math64.h>
#include <linux/mm.h>
#include <linux/wait.h>
#include <linux/vmalloc.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/scatterlist.h>
#include <linux/slab_def.h>
#include <linux/idr.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h>
#include <asm/page.h>
#include <ms_msys.h>
#include <ms_platform.h>
#include "cam_os_wrapper.h"
#include "cam_drv_buffer.h"
#endif

#define MAXFREE 150

CamDrvQueue_t* CamDrvQueueInit(u32 nMaxConsumer)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    CamDrvQueue_t* pQueue;
    u32 nMaxHpSize;

    pQueue = CamOsMemCalloc(1, sizeof(CamDrvQueue_t));

    if(!pQueue)
    {
        CamOsPrintf("%s alloc mem fail\n", __FUNCTION__,__LINE__);
    }

    do
    {
        nMaxHpSize = nMaxConsumer;
        pQueue->pHead = NULL;
        pQueue->pTail = NULL;
        CamOsAtomicSet(&pQueue->count, 0);
    }while(0);

    return pQueue;
#endif
}

CamDrvRet_e CamDrvQueueDeinit(CamDrvQueue_t* ptQueue)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptQueue)
    {
        CamOsMemRelease(ptQueue);
    }

    return  CAM_DRV_OK;
#endif
}

CamDrvRet_e CamDrvQueuePush(CamDrvQueue_t* ptQueue, CamDrvQueueNode_t* ptPushNode, void* ptData)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    CamDrvQueueNode_t* p;

    if(ptPushNode == NULL)
        return CAM_DRV_NULL_PTR;

    ptPushNode->pNext = NULL;
    ptPushNode->pData = ptData;
    CamOsSmpMemoryBarrier();
    do
    {
        p = ptQueue->pTail;
    }while(!CamOsAtomicCompareAndSwap((CamOsAtomic_t *)&ptQueue->pTail, (s32)p, (s32)ptPushNode));

    if(!CamOsAtomicRead(&ptQueue->count))
        ptQueue->pHead = ptQueue->pTail;
    else
        p->pNext = ptPushNode;

    CamOsAtomicIncReturn(&ptQueue->count);

    return CAM_DRV_OK;
#endif
}

CamDrvQueueNode_t* CamDrvQueuePop(CamDrvQueue_t* ptQueue)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    CamDrvQueueNode_t* p;
    CamDrvQueueNode_t* pn;

    if(!CamOsAtomicRead(&ptQueue->count))
        return NULL;

    do
    {
        p = ptQueue->pHead;
        CamOsSmpMemoryBarrier();
        if(p != ptQueue->pHead)
            continue;
        pn = p->pNext;
        if(pn != p->pNext)
            return NULL;
    } while(!CamOsAtomicCompareAndSwap((CamOsAtomic_t *)&ptQueue->pHead, (s32)p, (s32)pn));

    CamOsAtomicDecReturn(&ptQueue->count);
    if(!CamOsAtomicRead(&ptQueue->count))
    {
        ptQueue->pHead =  NULL;
        ptQueue->pTail = NULL;
    }

    return  p;
#endif
}

CamDrvRet_e CamDrvQueueQuery(CamDrvQueue_t* ptQueue, s32* ptCount)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    *ptCount = CamOsAtomicRead(&ptQueue->count);
    return CAM_DRV_OK;
#endif
}

QueueOps_t* CamDrvBuffInit(u32  nBufferSize, u32 nMaxConsumer)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    QueueOps_t* pOps;
    u32 i;

    pOps =  CamOsMemCalloc(1, sizeof(QueueOps_t));
    if(pOps == NULL)
        goto exit;

    pOps->nBufferSize = nBufferSize;
    pOps->pInsertNode = CamOsMemCalloc(pOps->nBufferSize, sizeof(CamDrvQueueNode_t));
    if(pOps->pInsertNode == NULL)
        goto exit;

    do
    {
        pOps->nThreadSize = nMaxConsumer;
        pOps->pInvalidHandle = CamDrvQueueInit(pOps->nThreadSize);
        pOps->pValidHandle = CamDrvQueueInit(pOps->nThreadSize);
        pOps->pFillHandle = CamDrvQueueInit(pOps->nThreadSize);
        pOps->pReadyHandle = CamDrvQueueInit(pOps->nThreadSize);

        if(!(pOps->pInvalidHandle && pOps->pValidHandle && pOps->pFillHandle && pOps->pReadyHandle))
        {
            if(pOps->pInvalidHandle)
            {
                if(CamDrvQueueDeinit(pOps->pInvalidHandle))
                    CamOsPrintf("%s:%d free mem fail\n\r", __FUNCTION__,__LINE__);
                else
                    pOps->pInvalidHandle = NULL;
            }

            if(pOps->pValidHandle)
            {
                if(CamDrvQueueDeinit(pOps->pValidHandle))
                    CamOsPrintf("%s:%d free mem fail\n\r", __FUNCTION__,__LINE__);
                else
                    pOps->pValidHandle = NULL;
            }

            if(pOps->pFillHandle)
            {
                if(CamDrvQueueDeinit(pOps->pFillHandle))
                    CamOsPrintf("%s:%d free mem fail\n\r", __FUNCTION__,__LINE__);
                else
                    pOps->pFillHandle = NULL;
            }

            if(pOps->pReadyHandle)
            {
                if(CamDrvQueueDeinit(pOps->pReadyHandle))
                    CamOsPrintf("%s:%d free mem fail\n\r", __FUNCTION__,__LINE__);
                else
                    pOps->pReadyHandle = NULL;
            }

            break;
        }

        for(i=0; i<pOps->nBufferSize; i++)
        {
            if(CamDrvQueuePush(pOps->pInvalidHandle, &pOps->pInsertNode[i], NULL))
                goto exit;
        }

        return pOps;
    } while(0);

exit:

    if(pOps->pInsertNode)
    {
        CamOsMemRelease(pOps->pInsertNode);
        pOps->pInsertNode = NULL;
    }

    if(pOps)
    {
        CamOsMemRelease(pOps);
        pOps = NULL;
    }

    return pOps;
#endif
}

CamDrvRet_e CamDrvBuffDeinit(QueueOps_t* ptBuff)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptBuff->pInvalidHandle)
    {
        if(CamDrvQueueDeinit(ptBuff->pInvalidHandle))
            CamOsPrintf("%s:%d free mem fail\n\r", __FUNCTION__,__LINE__);
        else
            ptBuff->pInvalidHandle = NULL;
    }

    if(ptBuff->pValidHandle)
    {
        if(CamDrvQueueDeinit(ptBuff->pValidHandle))
            CamOsPrintf("%s:%d free mem fail\n\r", __FUNCTION__,__LINE__);
        else
            ptBuff->pValidHandle = NULL;
    }

    if(ptBuff->pFillHandle)
    {
        if(CamDrvQueueDeinit(ptBuff->pFillHandle))
            CamOsPrintf("%s:%d free mem fail\n\r", __FUNCTION__,__LINE__);
        else
            ptBuff->pFillHandle = NULL;
    }

    if(ptBuff->pReadyHandle)
    {
        if(CamDrvQueueDeinit(ptBuff->pReadyHandle))
            CamOsPrintf("%s:%d free mem fail\n\r", __FUNCTION__,__LINE__);
        else
            ptBuff->pReadyHandle = NULL;
    }

    if(ptBuff->pInsertNode)
    {
        CamOsMemRelease(ptBuff->pInsertNode);
        ptBuff->pInsertNode = NULL;
    }

    if(ptBuff)
    {
        CamOsMemRelease(ptBuff);
        ptBuff = NULL;
    }

    return CAM_DRV_OK;
#endif
}

CamDrvRet_e CamDrvBuffAdd(QueueOps_t* ptBuff,  void* ptData)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    s32 nCount;
    CamDrvQueueNode_t* pTempNode;

    CamDrvQueueQuery(ptBuff->pInvalidHandle,  &nCount);
    if(nCount ==  0)
        return CAM_DRV_OUT_OF_RANGE;

    pTempNode = CamDrvQueuePop(ptBuff->pInvalidHandle);
    if(pTempNode == NULL)
        return CAM_DRV_FAIL;
    if(CamDrvQueuePush(ptBuff->pValidHandle, pTempNode, ptData))
        return CAM_DRV_FAIL;

    return CAM_DRV_OK;
#endif
}

CamDrvRet_e CamDrvBuffFill(QueueOps_t* ptBuff, void** pptData)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    s32 nCount;
    CamDrvQueueNode_t* pTempNode = NULL;

    CamDrvQueueQuery(ptBuff->pValidHandle,  &nCount);
    if(nCount ==  0)
        return CAM_DRV_OUT_OF_RANGE;

    pTempNode = CamDrvQueuePop(ptBuff->pValidHandle);
    if(pTempNode == NULL)
        return CAM_DRV_FAIL;
    *pptData = pTempNode->pData;
    if(CamDrvQueuePush(ptBuff->pFillHandle, pTempNode, pTempNode->pData))
        return CAM_DRV_FAIL;

    return CAM_DRV_OK;
#endif
}

CamDrvRet_e CamDrvBuffDone(QueueOps_t* ptBuff)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    s32 nCount;
    CamDrvQueueNode_t* pTempNode = NULL;

    CamDrvQueueQuery(ptBuff->pFillHandle,  &nCount);
    if(nCount ==  0)
        return CAM_DRV_OUT_OF_RANGE;

    pTempNode = CamDrvQueuePop(ptBuff->pFillHandle);
    if(pTempNode == NULL)
        return CAM_DRV_FAIL;
    if(CamDrvQueuePush(ptBuff->pReadyHandle, pTempNode, pTempNode->pData))
        return CAM_DRV_FAIL;

    return CAM_DRV_OK;
#endif
}

CamDrvRet_e CamDrvBuffGet(QueueOps_t* ptBuff, void** pptData)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    s32 nCount;
    CamDrvQueueNode_t* pTempNode = NULL;

    CamDrvQueueQuery(ptBuff->pReadyHandle,  &nCount);
    if(nCount ==  0)
        return CAM_DRV_OUT_OF_RANGE;

    pTempNode = CamDrvQueuePop(ptBuff->pReadyHandle);
    if(pTempNode == NULL)
        return CAM_DRV_FAIL;
    *pptData = pTempNode->pData;
    if(CamDrvQueuePush(ptBuff->pInvalidHandle, pTempNode, NULL))
        return CAM_DRV_FAIL;

    return CAM_DRV_OK;
#endif
}

CamDrvRet_e CamDrvBuffRecycle(QueueOps_t* ptBuff, void** pptData)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    s32 nCount;
    CamDrvQueueNode_t* pTempNode = NULL;

    CamDrvQueueQuery(ptBuff->pValidHandle,  &nCount);
    if(nCount ==  0)
        return CAM_DRV_OUT_OF_RANGE;

    pTempNode = CamDrvQueuePop(ptBuff->pValidHandle);
    if(pTempNode == NULL)
        return CAM_DRV_FAIL;
    *pptData = pTempNode->pData;
    if(CamDrvQueuePush(ptBuff->pInvalidHandle, pTempNode, NULL))
        return CAM_DRV_FAIL;

    return CAM_DRV_OK;
#endif
}

CamDrvRet_e CamDrvBuffQuery(QueueOps_t* ptBuff, s32* ptInvalidSize, s32* ptValidSize, s32* ptFillSize, s32* ptReadySize)
{
    // query buffer(data) or linked-list size
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)

#elif defined(CAM_OS_LINUX_KERNEL)
    s32 nCount;

    CamDrvQueueQuery(ptBuff->pInvalidHandle,  &nCount);
    *ptInvalidSize = nCount;
    CamDrvQueueQuery(ptBuff->pValidHandle,  &nCount);
    *ptValidSize = nCount;
    CamDrvQueueQuery(ptBuff->pFillHandle,  &nCount);
    *ptFillSize  = nCount;
    CamDrvQueueQuery(ptBuff->pReadyHandle,  &nCount);
    *ptReadySize  = nCount;

    return CAM_DRV_OK;
#endif
}
