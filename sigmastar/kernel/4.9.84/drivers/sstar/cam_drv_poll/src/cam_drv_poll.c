/*
* cam_drv_poll.c- Sigmastar
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
#include <cam_os_wrapper.h>
#include <cam_drv_poll.h>

#ifdef CAM_OS_RTK
#include "sys_MsWrapper_cus_os_flag.h"
#include "sys_MsWrapper_cus_os_sem.h"
#else
#include <linux/module.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/sched.h>
#endif


//=============================================================================

#define CAMDRVPOLL_EVENTGROUP_MAX          100

#ifndef TRUE
#define TRUE        1
#define FALSE       0
#endif

#ifdef CAM_OS_RTK
typedef struct
{
     u8          bUsed;
     u32         nEventGroup;  // event
     Ms_Flag_t   tWait;
     Ms_Mutex_t  tMutexEvent;
     u32         nWaitflag;  //reserved
} CamDrvPollEventInfo_t;

static                             Ms_Mutex_t _camPollEventGroup_Mutex;
#define EVENTABLE_MUTEX_LOCK()     MsMutexLock(&_camPollEventGroup_Mutex)
#define EVENTABLE_MUTEX_UNLOCK()   MsMutexUnlock(&_camPollEventGroup_Mutex)

#define EVENTLOCK_INIT             MsInitMutex
#define EVENTLOCK_LOCK             MsMutexLock
#define EVENTLOCK_UNLOCK           MsMutexUnlock
#define WAKEFLAG_INIT              MsFlagInit
#define WAKEFLAG_DEINIT(x)         MsFlagDestroy(x)
#define WAKEFLAG_WAKEUP(x,y)       MsFlagSetbits(x,y)
#else
typedef struct
{
     u8                 bUsed;
     u32                nEventGroup;  // event
     wait_queue_head_t  tWait;
     spinlock_t         tMutexEvent;

} CamDrvPollEventInfo_t;

static                             DEFINE_SPINLOCK(_camPollEventGroup_Mutex);
#define EVENTABLE_MUTEX_LOCK()     spin_lock(&_camPollEventGroup_Mutex)
#define EVENTABLE_MUTEX_UNLOCK()   spin_unlock(&_camPollEventGroup_Mutex)

#define EVENTLOCK_INIT             spin_lock_init
#define EVENTLOCK_LOCK             spin_lock
#define EVENTLOCK_UNLOCK           spin_unlock
#define WAKEFLAG_INIT              init_waitqueue_head
#define WAKEFLAG_DEINIT(x)
#define WAKEFLAG_WAKEUP(x,y)       wake_up_interruptible(x)
#endif

CamDrvPollEventInfo_t _camPollEventGroup_Info[CAMDRVPOLL_EVENTGROUP_MAX];


#define LOCK_EVENT(i)              EVENTLOCK_LOCK(&_camPollEventGroup_Info[i].tMutexEvent)
#define UNLOCK_EVENT(i)            EVENTLOCK_UNLOCK(&_camPollEventGroup_Info[i].tMutexEvent)
#define WAKE_UP(i,bits)            WAKEFLAG_WAKEUP(&_camPollEventGroup_Info[i].tWait, bits)


#define _CamDrvPollGetEvent(nEventID)      (_camPollEventGroup_Info[nEventID].nEventGroup)

#define _CamDrvPollCheckEventExist(nEventID) if ( (nEventID >= CAMDRVPOLL_EVENTGROUP_MAX) || \
                                            !_camPollEventGroup_Info[nEventID].bUsed )  \
                                            { return -1;  }


//=============================================================================
s32 CamDrvPollRegEventGrp(void)
{
    int i;

    //alloc an entry in the event table
    EVENTABLE_MUTEX_LOCK();
    for (i = 0; i < CAMDRVPOLL_EVENTGROUP_MAX; i++)
    {
        if (!_camPollEventGroup_Info[i].bUsed)
        {
            _camPollEventGroup_Info[i].bUsed = TRUE;
            _camPollEventGroup_Info[i].nEventGroup= 0;
            break;
        }
    }
    EVENTABLE_MUTEX_UNLOCK();

    if (i == CAMDRVPOLL_EVENTGROUP_MAX)
    {
        return -1;
    }

    EVENTLOCK_INIT(&_camPollEventGroup_Info[i].tMutexEvent);
    WAKEFLAG_INIT(&_camPollEventGroup_Info[i].tWait);

    return i;
}


void CamDrvPollDeRegEventGrp(u32 nEventID)
{
    EVENTABLE_MUTEX_LOCK();
    WAKEFLAG_DEINIT(&_camPollEventGroup_Info[nEventID].tWait);
    _camPollEventGroup_Info[nEventID].bUsed = FALSE;
    EVENTABLE_MUTEX_UNLOCK();
}


void CamDrvPollSetEvent(u32 nEventID, u32 nEventBits)
{
    //set event bits in the corresponding event entry
    LOCK_EVENT(nEventID);
    _camPollEventGroup_Info[nEventID].nEventGroup |= nEventBits;
    UNLOCK_EVENT(nEventID);

    WAKE_UP(nEventID, nEventBits);
}

s32 _CamDrvPollGetAndClearEvent(u32 nEventID, u32 nWaitBits)
{
   u32 eventBits;

   LOCK_EVENT(nEventID);
   eventBits = _camPollEventGroup_Info[nEventID].nEventGroup & nWaitBits;
   if (eventBits != 0)
   {
       _camPollEventGroup_Info[nEventID].nEventGroup &= ~eventBits;
   }
   UNLOCK_EVENT(nEventID);

   return eventBits;
}


#ifdef CAM_OS_RTK
s32 _CamDrvPollWaitEvent(u32 nEventID, u32 nWaitBits, s32 nTimeout)
{
    Ms_flag_value_t tFlagBit;
    s32 eventBits = 0;

    if (0 > nTimeout)  /* TIME_INFINITY */
    {
        do {
            MsFlagWait(&_camPollEventGroup_Info[nEventID].tWait,
                            (Ms_flag_value_t)nWaitBits,
                            MS_FLAG_WAITMODE_OR|MS_FLAG_WAITMODE_CLR);
            eventBits = _CamDrvPollGetAndClearEvent(nEventID, nWaitBits);
        } while (0 == eventBits);
   }
    else
    {
        do {
            tFlagBit = MsFlagTimedWait(&_camPollEventGroup_Info[nEventID].tWait,
                                   (Ms_flag_value_t)nWaitBits,
                                   MS_FLAG_WAITMODE_OR|MS_FLAG_WAITMODE_CLR,
                                   RTK_MS_TO_TICK(nTimeout));
            if (tFlagBit == 0) /* timeout */
            {
                break;
            }
            eventBits = _CamDrvPollGetAndClearEvent(nEventID, nWaitBits);
        } while (0 == eventBits);
    }

    return eventBits;
}
#endif

s32 CamDrvPollEvent(u32 nEventID, u32 nWaitBits, struct file *filp, poll_table *tPoll)
{
    u32 eventBits;

#ifdef CAM_OS_RTK
    u32 nTimeout = filp->nPollTimeout;

     _CamDrvPollCheckEventExist(nEventID);

    eventBits = _CamDrvPollGetAndClearEvent(nEventID, nWaitBits);
    if (0 >= eventBits)
    {
        eventBits = _CamDrvPollWaitEvent(nEventID, nWaitBits, nTimeout);
    }
#else

    _CamDrvPollCheckEventExist(nEventID);

    poll_wait(filp, &_camPollEventGroup_Info[nEventID].tWait, tPoll);

    eventBits = _CamDrvPollGetAndClearEvent(nEventID, nWaitBits);
#endif

    return eventBits;
}

#ifndef CAM_OS_RTK
EXPORT_SYMBOL(CamDrvPollRegEventGrp);
EXPORT_SYMBOL(CamDrvPollDeRegEventGrp);
EXPORT_SYMBOL(CamDrvPollSetEvent);
EXPORT_SYMBOL(CamDrvPollEvent);
#endif
