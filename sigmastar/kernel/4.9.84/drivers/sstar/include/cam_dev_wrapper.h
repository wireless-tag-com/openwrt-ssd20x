/*
* cam_dev_wrapper.h- Sigmastar
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
#ifndef CAM_DEV_WRAPPER_H_
#define CAM_DEV_WRAPPER_H_

#include "cam_os_wrapper.h"

#ifdef CAM_OS_RTK
#include <cam_dev_register.h>
#define CAM_DEV_POLLIN     0x1
#define CAM_DEV_POLLPRI    0x2
#define CAM_DEV_POLLOUT    0x4
#define CAM_DEV_POLLERR    0x8
#define CAM_DEV_POLLRDNORM 0x40

typedef void poll_table;

struct pollfd {
    int    fd;
    short  events;
    short  revents;
};
#elif defined(__KERNEL__)
#include <linux/fs.h>
#include <linux/poll.h>
#define CAM_DEV_POLLRDNORM  POLLRDNORM
#define CAM_DEV_POLLIN     POLLIN
#define CAM_DEV_POLLPRI    POLLPRI
#define CAM_DEV_POLLOUT    POLLOUT
#define CAM_DEV_POLLERR    POLLERR
#define CAM_DEV_POLLRDNORM POLLRDNORM
#else
#include <poll.h>
#define CAM_DEV_POLLRDNORM  POLLRDNORM
#define CAM_DEV_POLLIN     POLLIN
#define CAM_DEV_POLLPRI    POLLPRI
#define CAM_DEV_POLLOUT    POLLOUT
#define CAM_DEV_POLLERR    POLLERR
#define CAM_DEV_POLLRDNORM POLLRDNORM
#endif

#ifndef __KERNEL__
int CamDevOpen(char* name);
int CamDevClose(int fd);
int CamDevIoctl(int fd, unsigned long request, void *param);
int CamDevPoll(struct pollfd *fds, int nfds, int timeout);

void* CamDevMmap(int length,int fd,int offset);
int CamDevMunmap(int fd,void* start,int length);
#endif

#endif
