/*
* mdrv_pollsample_module.h- Sigmastar
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
#ifndef _MDRV_POLLSAMPLE_MODULE_H_
#define _MDRV_POLLSAMPLE_MODULE_H_

#include "cam_os_wrapper.h"
#include "cam_drv_poll.h"

#if defined(__KERNEL__)
#include <linux/cdev.h>
#endif

typedef struct
{
#if defined(__KERNEL__)
    struct cdev m_cdev; // character device
#else
    void*       m_cdev;
#endif
    // Device private data
    int         nTestNum;
} PollSampleDev_t;

int pollsamp_open(struct inode *inode, struct file *filp);
int pollsamp_release(struct inode *inode, struct file *filp);
long pollsamp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
unsigned int pollsamp_poll(struct file *filp, poll_table *tpoll);

#endif /* _MDRV_POLLSAMPLE_MODULE_H_ */
