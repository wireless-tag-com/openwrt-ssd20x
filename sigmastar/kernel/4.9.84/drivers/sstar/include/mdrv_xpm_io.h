/*
* mdrv_xpm_io.h- Sigmastar
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
#ifndef _MDRV_XPM_IO_H_
#define _MDRV_XPM_IO_H_

#define XPM_IOCTL_MAGIC             'S'

#define		IOCTL_XPM_REGISTER_SOURCE			_IO(XPM_IOCTL_MAGIC, 0x81)
#define		IOCTL_XPM_DEREGISTER_SOURCE			_IO(XPM_IOCTL_MAGIC, 0x82)
#define 	IOCTL_XPM_MAXNR    					0x83


#endif
