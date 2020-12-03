/*
* mdrv_pollsample_io.h- Sigmastar
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
#ifndef _MDRV_POLLSAMPLE_IO_H_
#define _MDRV_POLLSAMPLE_IO_H_

////////////////////////////////////////////////////////////////////////////////
// Header Files
////////////////////////////////////////////////////////////////////////////////
#include <cam_os_util_ioctl.h>
////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#define MAGIC_POLLSAMPLE              ('p')
#define IOCTL_POLLSAMPLE_START_TIMER  CAM_OS_IOR(MAGIC_POLLSAMPLE, 0, unsigned int)
#define IOCTL_POLLSAMPLE_GET_TIME     CAM_OS_IOR(MAGIC_POLLSAMPLE, 1, unsigned int)

#endif//_MDRV_POLLSAMPLE_IO_H_
