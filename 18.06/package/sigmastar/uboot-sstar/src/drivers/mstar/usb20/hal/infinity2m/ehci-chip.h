/*
* ehci-chip.h- Sigmastar
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


#ifndef __EHCI_CHIP_H__
#define __EHCI_CHIP_H__


#define EHCFLAG_NONE		0x0
#define EHCFLAG_DPDM_SWAP	0x1
#define EHCFLAG_XHC_COMP	0x2

/* RIU Base */
#if defined(__ARM__)
    #define HW_BASE     0x1f200000
#elif defined(__MIPS__)
    #define HW_BASE     0xbf200000
#else
    #error Unknow CPU core type
#endif

#if defined(CONFIG_ARM)
#define _MSTAR_PM_BASE         0x1F000000
#else
#define _MSTAR_PM_BASE         0xBF000000
#endif

/* xhc definition */
#define XHCI_0_BASE             (_MSTAR_PM_BASE+(0x380000 << 1))
#define XHCI_PORT0_ADDR         (XHCI_0_BASE+0x0430)
#define MSTAR_U3TOP0_BASE           (HW_BASE+(0x40200 << 1))
#define MSTAR_U3PHY_P0_D_BASE   (HW_BASE+(0x02000 << 1))
#define MSTAR_U3PHY_P0_A_BASE       (HW_BASE+(0x02100 << 1))

#define XHCI_1_BASE             (_MSTAR_PM_BASE+(0x390000 << 1))
#define XHCI_PORT1_ADDR         (XHCI_1_BASE+0x0430)
#define MSTAR_U3TOP1_BASE           (HW_BASE+(0x71900 << 1))
#define MSTAR_U3PHY_P1_D_BASE   (HW_BASE+(0x02200 << 1))
#define MSTAR_U3PHY_P1_A_BASE       (HW_BASE+(0x02300 << 1))

/* struct xhc_comp */
#define XHC_COMP_NONE           {0}
#define XHC_COMP_PORT0      {0, XHCI_PORT0_ADDR, MSTAR_U3TOP0_BASE, MSTAR_U3PHY_P0_A_BASE, MSTAR_U3PHY_P0_D_BASE, XHCI_0_BASE}
#define XHC_COMP_PORT1      {1, XHCI_PORT1_ADDR, MSTAR_U3TOP1_BASE, MSTAR_U3PHY_P1_A_BASE, MSTAR_U3PHY_P1_D_BASE, XHCI_1_BASE}

/*------ Register base mapping --------*/
/* Port0 */
#define MSTAR_UTMI_BASE  (HW_BASE+(0x42100 << 1))
#define MSTAR_USBC_BASE  (HW_BASE+(0x42300 << 1))
#define MSTAR_UHC_BASE   (HW_BASE+(0x42400 << 1))
#define MSTAR_BC_BASE	 (HW_BASE+(0x42200 << 1))

#if 0
	#define MSTAR_INIT_FLAG     EHCFLAG_DPDM_SWAP
#else
	#define MSTAR_INIT_FLAG     EHCFLAG_NONE
#endif

#define MSTAR_COMP0			XHC_COMP_NONE

/* Port1 */
#define MSTAR_UTMI1_BASE  (HW_BASE+(0x42500 << 1))
#define MSTAR_USBC1_BASE  (HW_BASE+(0x42700 << 1))
#define MSTAR_UHC1_BASE   (HW_BASE+(0x42800 << 1))
#define MSTAR_BC1_BASE	  (HW_BASE+(0x42600 << 1))

#if 0
	#define MSTAR_INIT1_FLAG	EHCFLAG_DPDM_SWAP
#else
	#define MSTAR_INIT1_FLAG	EHCFLAG_NONE
#endif

#if 1
	#define MSTAR_COMP1	XHC_COMP_NONE
#else
	#define MSTAR_COMP1     	XHC_COMP_PORT0
#endif
/* Port2 */
#define MSTAR_UTMI2_BASE  (HW_BASE+(0x42900 << 1))
#define MSTAR_USBC2_BASE  (HW_BASE+(0x43100 << 1))
#define MSTAR_UHC2_BASE   (HW_BASE+(0x43200 << 1))
#define MSTAR_BC2_BASE    (HW_BASE+(0x43000 << 1))

#if 0
	#define MSTAR_INIT2_FLAG     EHCFLAG_DPDM_SWAP
#else
	#define MSTAR_INIT2_FLAG     EHCFLAG_NONE
#endif

#if 0
	#define MSTAR_COMP2     	XHC_COMP_NONE
#else
	#define MSTAR_COMP2     	XHC_COMP_PORT0
#endif

//==================================USB 3.0======================================
#define REG_USB30_GPIO_CONTROL			0x1402FC
#define REG_USB30_GPIO_BANK				0x1026
#define REG_USB30_GPIO_OFFSET				0xA2 //bit0~bit7 offset addr
#define REG_USB30_GPIO_BIT					0x0  //bit8~bit10 bit count
#define REG_USB30_GPIO_LOW_ACTIVE		0x0  //Turn on 5V  ( 0:  GPIO High   1: GPIO Low) active

#define ENABLE_USB30_GPIO_PORT2
#define REG_USB30_PORT2_GPIO_CONTROL	0x1719FC
#define REG_USB30_PORT2_GPIO_BANK		0x1026
#define REG_USB30_PORT2_GPIO_OFFSET		0xA2 //bit0~bit7 offset addr
#define REG_USB30_PORT2_GPIO_BIT			0x2  //bit8~bit10 bit count
#define REG_USB30_PORT2_GPIO_LOW_ACTIVE	0x0  //Turn on 5V  ( 0:  GPIO High   1: GPIO Low) active
//=============================================================================

#endif /* __EHCI_CHIP_H__*/
