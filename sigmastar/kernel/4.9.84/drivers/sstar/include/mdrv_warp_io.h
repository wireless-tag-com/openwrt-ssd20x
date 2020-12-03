/*
* mdrv_warp_io.h- Sigmastar
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
#ifndef _MDRV_WARP_IO_H_
#define _MDRV_WARP_IO_H_

#include <linux/ioctl.h>

#define WARP_IOC_MAGIC 'I'
#define WARP_IOC_TRIGGER _IOW(WARP_IOC_MAGIC, 1, MHAL_WARP_CONFIG*)

#endif //_MDRV_WARP_IO_H_