/*
* usb_common_sstar.h- Sigmastar
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


#ifndef _USB_SSTAR_H
#define _USB_SSTAR_H

//#if (MP_USB_MSTAR==1)
#if 1

//Host
#ifndef MP_USB_MSTAR
#include <usb_patch_mstar.h>
#endif

#include "ehci-mstar.h"

//Core
#include "xhci-mstar.h"
#include <asm/io.h>
#if (_UTMI_PWR_SAV_MODE_ENABLE == 1) || defined(USB_MAC_SRAM_POWER_DOWN_ENABLE)
#include "bc-mstar.h"
#endif


#endif

#endif	/* _USB_SSTAR_H */
