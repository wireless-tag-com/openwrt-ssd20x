/*
* xhci-mstar-chip.h- Sigmastar
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
 * xHCI host controller driver
 *
 * Copyright (C) 2013 MStar Inc.
 *
 */

#ifndef _XHCI_MSTAR_CIP_H
#define _XHCI_MSTAR_CIP_H

#include "ehci-mstar.h"

// ----- Don't modify it !----------
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#define XHCI_PA_PATCH   1
#else
#define XHCI_PA_PATCH   0
#endif
#define XHCI_FLUSHPIPE_PATCH  1
//------------------------------

#define XHCI_CHIRP_PATCH  1
#define ENABLE_XHCI_SSC   1

#if (ENABLE_XHCI_SSC)
#define XHCI_SSC_TX_SYNTH_SET_C0			0x9374
#define XHCI_SSC_TX_SYNTH_SET_C2			0x18
#define XHCI_SSC_TX_SYNTH_STEP_C4			0x7002
#define XHCI_SSC_TX_SYNTH_SPAN_C6			0x04D8
#endif

#define ENABLE_SECOND_XHC

#define XHCI_TX_SWING_PATCH  1

//------ for test -----------------
//#define XHCI_CURRENT_SHARE_PATCH 0   //Only for USB3; will cause USB2 chirp handshake fail.
#define XHCI_ENABLE_DEQ  0
#define XHCI_ENABLE_TESTBUS  0
//--------------------------------

//Inter packet delay setting for all chips
#define XHCI_IPACKET_DELAY_PATCH

#define XHCI_DISABLE_COMPLIANCE
#define XHCI_DISABLE_TESTMODE
#define XHCI_SSDISABLED_PATCH
#define XHCI_HC_RESET_PATCH

#define MSTAR_LOST_SLOT_PATCH 0

#define XHCI_TX_ERR_EVENT_PATCH

//#define XHCI_ENABLE_PPC

//--------  Setting option  -----------

#define XHCI_ENABLE_240MHZ

#define XHCI_ENABLE_LASTDOWNZ

//--------------------------------


// --------- ECO option ---------
#define XHCI_ENABLE_LOOPBACK_ECO
#define LOOPBACK_ECO_OFFSET		0x20*2
#define LOOPBACK_ECO_BIT		BIT4|BIT5

//--------------------------------


//--------  U3 PHY IP  -----------
#define XHCI_PHY_MS28

#ifdef XHCI_PHY_MS28
#define GCR_USB3RX0_RCTRL		(0x08*2)
#define GCR_USB3TX0_RT			(0x10*2)
#define GCR_USB3RX1_RCTRL		(0x08*2)
#define GCR_USB3TX1_RT			(0x10*2)

#define USB30RX0_EFUSE_BITOFFSET	8
#define USB30TX0_EFUSE_BITOFFSET	0
#define USB30RX1_EFUSE_BITOFFSET	24
#define USB30TX1_EFUSE_BITOFFSET	16
#endif

//#define XHCI_2PORTS

#define XHCI_COMPANION

//#define XHCI_ENABLE_PD_OVERRIDE

#define U3PHY_RX_DETECT_POWER_SAVING

#if defined(XHCI_2PORTS)
#define XHCI_SSDISABLE_POWERDOWN_PATCH
#endif

//#define XHCI_PHY_ENABLE_RX_LOCK

//#define XHCI_PWS_P2

//--------------------------------

#if defined(CONFIG_ARM64)
	extern ptrdiff_t mstar_pm_base;
	#define _MSTAR_PM_BASE         (mstar_pm_base)
#elif defined(CONFIG_ARM)
	#define _MSTAR_PM_BASE         0xFD000000
#else
	#define _MSTAR_PM_BASE         0xBF000000
#endif

//------ U3TOP, DTOP, ATOP and XHC base address -----------------------------
//---- Port0
//#define _MSTAR_U3PHY_DTOP_BASE (_MSTAR_USB_BASEADR+(0x11C00*2))
//#define _MSTAR_U3PHY_ATOP_BASE (_MSTAR_USB_BASEADR+(0x22100*2))
//#define _MSTAR_U3UTMI_BASE     0
//#define _MSTAR_U3TOP_BASE      (_MSTAR_USB_BASEADR+(0x40200*2))
//#define _MSTAR_XHCI_BASE       (_MSTAR_PM_BASE+(0x380000*2))
//#define _MSTAR_U3BC_BASE       0

#define _MSTAR_U3PHY_DTOP0_BASE (_MSTAR_USB_BASEADR+(0x02000*2))
#define _MSTAR_U3PHY_ATOP0_BASE (_MSTAR_USB_BASEADR+(0x02100*2))
#define _MSTAR_U3TOP0_BASE      (_MSTAR_USB_BASEADR+(0x40200*2))
#define _MSTAR_XHCI0_BASE       (_MSTAR_PM_BASE+(0x380000*2))
#define _MSTAR_U3UTMI0_BASE     0
#define _MSTAR_U3BC0_BASE       0

//---- Port1
#ifdef ENABLE_SECOND_XHC
#define _MSTAR_U3PHY_DTOP1_BASE (_MSTAR_USB_BASEADR+(0x02200*2))
#define _MSTAR_U3PHY_ATOP1_BASE (_MSTAR_USB_BASEADR+(0x02300*2))
#define _MSTAR_U3TOP1_BASE      (_MSTAR_USB_BASEADR+(0x71900*2))
#define _MSTAR_XHCI1_BASE       (_MSTAR_PM_BASE+(0x390000*2))
#define _MSTAR_U3UTMI1_BASE     0
#define _MSTAR_U3BC1_BASE       0
#endif

#define XHC_HSPORT_OFFSET	0x420
#define XHC_SSPORT_OFFSET	0x430

#if defined(XHCI_PHY_EFUSE)
	#define RTERM_XHC_BANK		(0x50*4)	//bank 0x50
#endif

#endif	/* _XHCI_MSTAR_CIP_H */
