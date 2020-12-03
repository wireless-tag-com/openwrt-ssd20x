/*
* cam_drv_buffer.h- Sigmastar
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
/// @file      cam_drv_buffer.h
/// @brief     Cam Drv Buffer  Header File for
///            1. RTK OS
///            2. Linux User Space
///            3. Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#ifndef __CAM_DRV_BUFFER_H__
#define __CAM_DRV_BUFFER_H__

#define CAM_DRV_BUFFER_VERSION "v0.0.1"

#include "cam_os_wrapper.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef enum
{
    CAM_DRV_OK = 0,
    CAM_DRV_NULL_PTR = 1,
    CAM_DRV_FREE_ERROR = 2,
    CAM_DRV_QUEUE_EMPTY = 3,
    CAM_DRV_QUEUE_FULL = 4,
    CAM_DRV_THREAD_RACE = 5,
    CAM_DRV_OUT_OF_RANGE = 6,
    CAM_DRV_FAIL = 7,
} CamDrvRet_e;

typedef struct CamDrvQueueNode_s CamDrvQueueNode_t;

struct CamDrvQueueNode_s
{
    void * pData;
    CamDrvQueueNode_t* pNext;
} ;

typedef struct CamDrvQueue_s
{
    CamDrvQueueNode_t* pHead;
    CamDrvQueueNode_t* pTail;
    CamOsAtomic_t  count;
} CamDrvQueue_t;

typedef struct QueueOps_s
{
    u32 nBufferSize;
    u32 nThreadSize;
    CamDrvQueueNode_t *pInsertNode;
    CamDrvQueue_t* pInvalidHandle;
    CamDrvQueue_t* pValidHandle;
    CamDrvQueue_t* pFillHandle;
    CamDrvQueue_t* pReadyHandle;
} QueueOps_t;

//=============================================================================
// Description:
//      Queue handle initialization
// Parameters:
//      param[in]  max_consumer:  Maximum consumer thread count
// Return:
//     Queue handle pointer
//=============================================================================
CamDrvQueue_t* CamDrvQueueInit(u32 nMaxConsumer);

//=============================================================================
// Description:
//      Queue handle Release
// Parameters:
//      param[in]  ptQueue: Queue handle.
// Return:
//      CAM_DRV_OK is returned if successful; otherwise, returns CamDrvRet_e.
//=============================================================================
CamDrvRet_e CamDrvQueueDeinit(CamDrvQueue_t* ptQueue);

//=============================================================================
// Description:
//      Enqueue one block buffer
// Parameters:
//      param[in]  ptQueue: Queue handle.
//      param[in]  ptPushNode: Push node into queue. If ptPushNode == NULL , will use the memory allocation
//      param[in]  ptData: input data
// Return:
//      CAM_DRV_OK is returned if successful; otherwise, returns CamDrvRet_e.
//=============================================================================
CamDrvRet_e CamDrvQueuePush(CamDrvQueue_t* ptQueue, CamDrvQueueNode_t* ptPushNode, void* ptData);

//=============================================================================
// Description:
//      Dequeue one block buffer
// Parameters:
//      param[in]  ptQueue: Queue handle.
// Return:
//      Queue node  pointer
//=============================================================================
CamDrvQueueNode_t* CamDrvQueuePop(CamDrvQueue_t* ptQueue);

//=============================================================================
// Description:
//      Query single buffer info
// Parameters:
//      param[in]  ptQueue: Queue handle.
//      param[out]  count: How many node inside ptQueue.
// Return:
//      CAM_DRV_OK is returned if successful; otherwise, returns CamDrvRet_e.
//=============================================================================
CamDrvRet_e CamDrvQueueQuery(CamDrvQueue_t* ptQueue, s32* ptCount);

//=============================================================================
// Description:
//      Buffer handle initialization
// Parameters:
//      param[in]  Buffer_size: The node size of Invalid linked list
//      param[in]  max_consumer:  Maximum consumer thread count
// Return:
//      Buffer handle pointer
//=============================================================================
QueueOps_t* CamDrvBuffInit(u32  nBufferSize, u32 nMaxConsumer);

//=============================================================================
// Description:
//      Buffer handle Release
// Parameters:
//      param[in]  ptBuff: Buffer handle
// Return:
//      CAM_DRV_OK is returned if successful; otherwise, returns CamDrvRet_e.
//=============================================================================
CamDrvRet_e CamDrvBuffDeinit(QueueOps_t* ptBuff);

//=============================================================================
// Description:
//      Add new buffer into valid linked-list
// Parameters:
//      param[in]  param[in]  ptBuff: Buffer handle
//      param[in]  ptData: input data
// Return:
//      CAM_DRV_OK is returned if successful; otherwise, returns CamDrvRet_e.
//=============================================================================
CamDrvRet_e CamDrvBuffAdd(QueueOps_t* ptBuff,  void* ptData);

//=============================================================================
// Description:
//      Query buffer info for specific linked-list
// Parameters:
//      param[in]  param[in]  ptBuff: Buffer handle
//      param[in]  size: the number of new buffer
//      param[in]  ptData: output data
// Return:
//      CAM_DRV_OK is returned if successful; otherwise, returns CamDrvRet_e.
//=============================================================================
CamDrvRet_e CamDrvBuffQuery(QueueOps_t* ptBuff, s32* ptInvalidSize, s32* ptValidSize, s32* ptFillSize, s32* ptReadySize);

//=============================================================================
// Description:
//      Get  buffer from ready linked-list
// Parameters:
//      param[in]  param[in]  ptBuff: Buffer handle
//      param[in]  ptData: output data
// Return:
//      CAM_DRV_OK is returned if successful; otherwise, returns CamDrvRet_e.
//=============================================================================
CamDrvRet_e CamDrvBuffGet(QueueOps_t* ptBuff, void** pptData);

//=============================================================================
// Description:
//      Fill  buffer from valid into filling linked-list
// Parameters:
//      param[in]  param[in]  ptBuff: Buffer handle
//      param[in]  ptData: output data
// Return:
//      CAM_DRV_OK is returned if successful; otherwise, returns CamDrvRet_e.
//=============================================================================
CamDrvRet_e CamDrvBuffFill(QueueOps_t* ptBuff, void** pptData);

//=============================================================================
// Description:
//      Recycle unused buffer from valid into invalid linked-list
// Parameters:
//      param[in]  param[in]  ptBuff: Buffer handle
//      param[in]  ptData: output data
// Return:
//      CAM_DRV_OK is returned if successful; otherwise, returns CamDrvRet_e.
//=============================================================================
CamDrvRet_e CamDrvBuffRecycle(QueueOps_t* ptBuff, void** pptData);

//=============================================================================
// Description:
//      Extract buffer from fill into ready linked-list
// Parameters:
//      param[in]  param[in]  ptBuff: Buffer handle
// Return:
//      CAM_DRV_OK is returned if successful; otherwise, returns CamDrvRet_e.
//=============================================================================
CamDrvRet_e CamDrvBuffDone(QueueOps_t* ptBuff);

#endif /* __CAM_DRV_BUFFER_H__ */
