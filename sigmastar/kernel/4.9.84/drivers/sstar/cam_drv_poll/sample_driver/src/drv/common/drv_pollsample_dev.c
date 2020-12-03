/*
* drv_pollsample_dev.c- Sigmastar
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
#include <mdrv_pollsample_io.h>
#include <mdrv_pollsample_module.h>

#define _WITH_CAM_DRV_POLL  1 // This option only can disable in Linux

#ifndef EFAULT
#define EFAULT          14      // for non-Linux OS
#endif

#if !_WITH_CAM_DRV_POLL
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/poll.h>
#endif

////////////////////////////////////////////////////////////////////////////////
#define EVENT_TIMER1        0x00000001
#define EVENT_TIMER2        0x00000002

struct _driver_private_data;
typedef struct _timer_cb_param
{
    u32  timer_event_id;
    u32  timer_event_bit;
    u32  timer_msec;
    struct _driver_private_data *dpd;
} timer_cb_param;

typedef struct _driver_private_data
{
    u32   eventGrp_id;
    timer_cb_param     event_timer1_param;
    timer_cb_param     event_timer2_param;
    CamOsTimer_t       event_timer1;
    CamOsTimer_t       event_timer2;
#if !_WITH_CAM_DRV_POLL
    s32   flag;
    wait_queue_head_t wait;
#endif
} driver_private_data;


static void MyTimerFunction(unsigned long data)
{
    driver_private_data *dpd;
    timer_cb_param *tcbp = (timer_cb_param *)data;

    if(tcbp)
    {
        dpd = tcbp->dpd;
#if _WITH_CAM_DRV_POLL
        CamDrvPollSetEvent(tcbp->timer_event_id, tcbp->timer_event_bit);
#else
        dpd->flag |= tcbp->timer_event_bit;
        wake_up_interruptible(&dpd->wait);
#endif
        CamOsPrintf("time_callback triggered(event=%d,bits=0x%x)\n", tcbp->timer_event_id, tcbp->timer_event_bit);

        if(tcbp->timer_event_bit == EVENT_TIMER1)
        {
            CamOsTimerModify(&dpd->event_timer1, tcbp->timer_msec);
        }
        else if(tcbp->timer_event_bit == EVENT_TIMER2)
        {
            CamOsTimerModify(&dpd->event_timer2, tcbp->timer_msec);
        }
    }
}

int pollsamp_open(struct inode *inode, struct file *filp)
{
    PollSampleDev_t *pPollSampleDev = CAM_OS_CONTAINER_OF(inode->i_cdev,PollSampleDev_t,m_cdev);
    driver_private_data *dpd;

    CamOsPrintf("%s: nTestNum = %d\n", __FUNCTION__, pPollSampleDev->nTestNum);

    dpd = (driver_private_data *)filp->private_data;
    if(!dpd)
    {
        dpd = (driver_private_data *)CamOsMemCalloc(1, sizeof(driver_private_data));
        filp->private_data = dpd;

        CamOsTimerInit(&dpd->event_timer1);
        CamOsTimerInit(&dpd->event_timer2);

#if _WITH_CAM_DRV_POLL
        dpd->eventGrp_id = CamDrvPollRegEventGrp();
#else
        dpd->flag = 0;
        init_waitqueue_head(&dpd->wait);
#endif
        CamOsPrintf("%s: eventId=%d\n", __func__, dpd->eventGrp_id);
    }
    return 0;
}

int pollsamp_release(struct inode *inode, struct file *filp)
{
    driver_private_data *dpd;

    CamOsPrintf("pollsample release!\n");
    dpd = (driver_private_data *)filp->private_data;
#if _WITH_CAM_DRV_POLL
    CamDrvPollDeRegEventGrp(dpd->eventGrp_id);
#endif
    CamOsTimerDelete(&dpd->event_timer1);
    CamOsTimerDelete(&dpd->event_timer2);
    return 0;
}

long pollsamp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    driver_private_data *dpd = (driver_private_data*)filp->private_data;
    int err = 0;
    s32 expire;

    // Should we do that?
    if(dpd == NULL)
    {
        CamOsPrintf("dpd NULL !!\n");
        return -EFAULT;
    }

    switch(cmd)
    {
        case IOCTL_POLLSAMPLE_START_TIMER:
            CamOsCopyFromUpperLayer(&expire, (void*)arg, sizeof(expire));
            if(expire < 0)
            {
                CamOsPrintf("get user data Failed !!\n");
                err = -EFAULT;
                break;
            }
            CamOsPrintf("IOCTL_POLLSAMPLE_START_TIMER: get expire=%d msec\n", expire);
            dpd->event_timer1_param.timer_event_id  = dpd->eventGrp_id;
            dpd->event_timer1_param.timer_event_bit = EVENT_TIMER1;
            dpd->event_timer1_param.timer_msec = expire;
            dpd->event_timer1_param.dpd = dpd;
            CamOsTimerAdd(&dpd->event_timer1, expire, (void *)&dpd->event_timer1_param, MyTimerFunction);
            dpd->event_timer2_param.timer_event_id  = dpd->eventGrp_id;
            dpd->event_timer2_param.timer_event_bit = EVENT_TIMER2;
            dpd->event_timer2_param.timer_msec = expire + 500;
            dpd->event_timer2_param.dpd = dpd;
            CamOsTimerAdd(&dpd->event_timer2, (expire+500), (void *)&dpd->event_timer2_param, MyTimerFunction);
            break;

        default:
            err = -EFAULT;
            break;
    }

    return err;
}

unsigned int pollsamp_poll(struct file *filp, poll_table *tpoll)
{
    driver_private_data *dpd = filp->private_data;
    unsigned int mask = 0;

#if _WITH_CAM_DRV_POLL
    u32 eventBits;

    eventBits = CamDrvPollEvent(dpd->eventGrp_id, EVENT_TIMER1 | EVENT_TIMER2, filp, tpoll);
    if((eventBits & EVENT_TIMER1) != 0)
    {
        CamOsPrintf("pollsamp_poll: got event bits=0x%x\n", eventBits);
        mask |= CAM_DEV_POLLIN;
    }
    if((eventBits & EVENT_TIMER2) != 0)
    {
        CamOsPrintf("pollsamp_poll: got event bits=0x%x\n", eventBits);
        mask |= CAM_DEV_POLLPRI;
    }
#else
    if (dpd)
    {
        poll_wait(filp, &dpd->wait, tpoll);
        if (dpd->flag)
        {
            if (dpd->flag & EVENT_TIMER1)
                mask |= POLLIN;
            if (dpd->flag & EVENT_TIMER2)
                mask |= POLLPRI;
            dpd->flag=0;
        }
    }
#endif
    return mask;
}
