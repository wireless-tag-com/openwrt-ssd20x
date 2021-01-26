/*
* common.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: jiang.ann <jiang.ann@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <linux/string.h>
#include <config.h>
#include <usb.h>

#define __DEBUG_ON__

#if defined(__DEBUG_ON__)
	//#include <stdio.h>
	#define	MS_MSG	printf
#else
	#define	MS_MSG(fmt, ...)
#endif

/* Debuging Message Switch */
#define USB_LIB_DEBUG	1
//#define USB_LIB_BULK_DEBUG	1
#define USB_GLUE_DEBUG	1

#endif /* __COMMON_H__ */
