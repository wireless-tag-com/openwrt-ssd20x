/*
* ehci_usb.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
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

#ifndef EHCI_USB_H
#define EHCI_USB_H

#include "MsTypes.h"
//#include <ShareType.h>



extern U32 MsOS_VA2PA(U32 addr);
extern U32 MsOS_PA2KSEG0(U32 addr);
extern U32 MsOS_PA2KSEG1(U32 addr);

#define USB_DELAY(_msec)           wait_ms(_msec)
#if defined(CONFIG_JANUS) || defined(CONFIG_MARIA10)
#define KSEG02KSEG1(addr)  ((void *)((U32)(addr)|0x80000000))
#define KSEG12KSEG0(addr)  ((void *)((U32)(addr)&~0x80000000))
#elif defined(__ARM__)
#if defined(CONFIG_ARCH_CHICAGO)
#define KSEG02KSEG1(addr)  addr
#define KSEG12KSEG0(addr)  addr
#else
#define KSEG02KSEG1(addr)  MsOS_PA2KSEG1(MsOS_VA2PA((U32)addr))
#define KSEG12KSEG0(addr)  MsOS_PA2KSEG0(MsOS_VA2PA((U32)addr))
#endif
#else
#define KSEG02KSEG1(addr)       ((void *)((UINT32)(addr)|0x20000000))  //cached -> unchched
#define KSEG12KSEG0(addr)       ((void *)((UINT32)(addr)&~0x20000000)) //unchched -> cached
#endif

#if (defined (CONFIG_TITANIA4)||defined(CONFIG_URANUS4)||defined(CONFIG_JANUS))||defined(CONFIG_MARIA10)||defined(CONFIG_KRONUS) ||defined(CONFIG_KAISERIN)
#define VA2PA(a)  (a)
#define PA2VA(a)  (a)
#elif defined(__ARM__)
//#define VA2PA(a)  MsOS_VA2PA((U32)a)
//#define PA2VA(a)  MsOS_PA2KSEG1((U32)a) //// mapping to uncache address
#if ENABLE_EAGLE
#define MIU0_BUS_BASE_ADDR	CONFIG_MIU0_BUSADDR
#define MIU0_PHY_BASE_ADDR	0x00000000UL
#define MIU1_BUS_BASE_ADDR	CONFIG_MIU1_BUSADDR
#define MIU1_PHY_BASE_ADDR	0x20000000UL
#elif ENABLE_AMBER3 || \
      ENABLE_AGATE  || \
	  ENABLE_EDISON
#define MIU0_BUS_BASE_ADDR	CONFIG_MIU0_BUSADDR
#define MIU0_PHY_BASE_ADDR	0x00000000UL
#define MIU1_BUS_BASE_ADDR	CONFIG_MIU1_BUSADDR
#define MIU1_PHY_BASE_ADDR	0x80000000UL
#elif ENABLE_NIKE || ENABLE_NADAL
#define MIU0_BUS_BASE_ADDR	CONFIG_MIU0_BUSADDR
#define MIU0_PHY_BASE_ADDR	0x00000000UL
#define MIU1_BUS_BASE_ADDR	CONFIG_MIU1_BUSADDR
#define MIU1_PHY_BASE_ADDR	0x40000000UL
#else
#if defined(CONFIG_ARCH_CHICAGO)
#define MIU0_BUS_BASE_ADDR	0x20000000UL
#define MIU0_PHY_BASE_ADDR	0x00000000UL
#define MIU1_BUS_BASE_ADDR	0xffffffffUL
#define MIU1_PHY_BASE_ADDR	0x80000000UL
#else
#define MIU0_BUS_BASE_ADDR	0x40000000UL
#define MIU0_PHY_BASE_ADDR	0x00000000UL
#define MIU1_BUS_BASE_ADDR	0xffffffffUL
#define MIU1_PHY_BASE_ADDR	0x80000000UL
#endif
#endif
/* transmit between BUS and USB PHY Addr */
extern U32 VA2PA(U32 u32_DMAAddr);
extern U32 PA2VA(U32 u32_DMAAddr);

#else
#define VA2PA(a)  ((a) & 0x1FFFFFFF)
#define PA2VA(a)  ((a) | 0xA0000000) //// mapping to uncache address
#endif

/* Requests: bRequest << 8 | bmRequestType */
#define RH_GET_STATUS           0x0080
#define RH_CLEAR_FEATURE        0x0100
#define RH_SET_FEATURE          0x0300
#define RH_SET_ADDRESS		0x0500
#define RH_GET_DESCRIPTOR	0x0680
#define RH_SET_DESCRIPTOR       0x0700
#define RH_GET_CONFIGURATION	0x0880
#define RH_SET_CONFIGURATION	0x0900
#define RH_GET_STATE            0x0280
#define RH_GET_INTERFACE        0x0A80
#define RH_SET_INTERFACE        0x0B00
#define RH_SYNC_FRAME           0x0C80

#if !defined(CONFIG_JANUS) && !defined(CONFIG_MARIA10)
#define _USB_FLUSH_BUFFER   1
#else
#define _USB_FLUSH_BUFFER   0
#endif

#endif

