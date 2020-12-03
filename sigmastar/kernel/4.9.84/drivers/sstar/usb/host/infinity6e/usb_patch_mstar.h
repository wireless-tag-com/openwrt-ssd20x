/*
* usb_patch_mstar.h- Sigmastar
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
#ifndef _MPATCH_MACRO_H
#define _MPATCH_MACRO_H

#define MSTAR_MIU_BUS_BASE_NONE		0xFFFFFFFF

#define MSTAR_MIU0_BUS_BASE	0x20000000
#define MSTAR_MIU1_BUS_BASE	MSTAR_MIU_BUS_BASE_NONE


//=========================== Module:USB=======================================
#ifdef CONFIG_MP_USB_MSTAR
	#define MP_USB_MSTAR	1
#else
	#define MP_USB_MSTAR	0
#endif

#ifdef CONFIG_MP_USB_MSTAR_DEBUG
	#define MP_USB_MSTAR_DEBUG	1
#else
	#define MP_USB_MSTAR_DEBUG	0
#endif

//=========================== Module:USB=======================================
#ifdef CONFIG_MP_USB_STR_PATCH
	#define MP_USB_STR_PATCH	1
#else
	#define MP_USB_STR_PATCH	0
#endif

#ifdef CONFIG_MP_USB_STR_PATCH_DEBUG
	#define MP_USB_STR_PATCH_DEBUG	1
#else
	#define MP_USB_STR_PATCH_DEBUG	0
#endif

#endif //_MPATCH_MACRO_H
