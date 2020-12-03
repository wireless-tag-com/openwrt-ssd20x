/*
* bc-mstar.h- Sigmastar
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
#ifndef __BC_MSTAR_H
#define __BC_MSTAR_H


extern void usb_bc_enable(struct usb_hcd *hcd, bool enable);
extern int apple_charger_support(struct usb_hcd *hcd, struct usb_device *udev);

extern void usb_power_saving_enable(struct usb_hcd *hcd, bool enable);
extern void usb20mac_sram_power_saving(struct usb_hcd *hcd, bool enable);
extern void usb30mac_sram_power_saving(struct usb_hcd *hcd, bool enable);
#endif
