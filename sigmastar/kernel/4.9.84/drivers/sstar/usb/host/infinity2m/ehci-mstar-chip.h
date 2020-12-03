/*
* ehci-mstar-chip.h- Sigmastar
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
/*
 * eHCI host controller driver
 *
 * Copyright (C) 2012~2017 MStar Inc.
 *
 *
 * Date: 2015.12.11
 *   1. software patch of VFall state machine mistake is removed, all chips
 *      run with kernel 4.4.3 should be VFall hardware ECO
 */
#ifndef _EHCI_MSTAR_CHIP_H
#define _EHCI_MSTAR_CHIP_H

//---Port -----------------------------------------------------------------
/* If only 1 ports */
//#define DISABLE_SECOND_EHC

//------ Additional port enable (default: 2 ports) -------------------
#define ENABLE_THIRD_EHC

//#define ENABLE_FOURTH_EHC

//--------------------------------------------------------------------


//------ Battery charger -------------------- -------------------
//#define ENABLE_BATTERY_CHARGE
//#define ENABLE_APPLE_CHARGER_SUPPORT
//#define USB_NO_BC_FUNCTION
//#define USB_BATTERY_CHARGE_SETTING_1
//--------------------------------------------------------------------


//------ UTMI, USBC and UHC base address -----------------------------
//---- Port0
#define _MSTAR_UTMI0_BASE	(_MSTAR_USB_BASEADR+(0x42100*2))
#define _MSTAR_BC0_BASE     (_MSTAR_USB_BASEADR+(0x42200*2))
#define _MSTAR_USBC0_BASE   (_MSTAR_USB_BASEADR+(0x42300*2))
#define _MSTAR_UHC0_BASE	(_MSTAR_USB_BASEADR+(0x42400*2))

//---- Port1
#define _MSTAR_UTMI1_BASE   (_MSTAR_USB_BASEADR+(0x42500*2))
#define _MSTAR_BC1_BASE     (_MSTAR_USB_BASEADR+(0x42600*2))
#define _MSTAR_USBC1_BASE   (_MSTAR_USB_BASEADR+(0x42700*2))
#define _MSTAR_UHC1_BASE    (_MSTAR_USB_BASEADR+(0x42800*2))

//---- Port2
#define _MSTAR_UTMI2_BASE   (_MSTAR_USB_BASEADR+(0x42900*2))
#define _MSTAR_BC2_BASE     (_MSTAR_USB_BASEADR+(0x43000*2))
#define _MSTAR_USBC2_BASE   (_MSTAR_USB_BASEADR+(0x43100*2))
#define _MSTAR_UHC2_BASE    (_MSTAR_USB_BASEADR+(0x43200*2))

//---- Chiptop for Chip version
//#define MSTAR_CHIP_TOP_BASE	(_MSTAR_USB_BASEADR+(0x1E00*2))
//--------------------------------------------------------------------


#endif	/* _EHCI_MSTAR_CHIP_H */
