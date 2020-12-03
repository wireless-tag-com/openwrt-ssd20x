/*
* mdrv_noe_mac.c- Sigmastar
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
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (“MStar Confidential Information”) by the recipien
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   MDRV_NOE.c
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------

#include <linux/module.h>
#include <linux/init.h>
#include <generated/autoconf.h>
#include <linux/mii.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/pci.h>
#include <linux/crc32.h>
#include <linux/ethtool.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/version.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

#if defined(CONFIG_MIPS)
#include <asm/mips-boards/prom.h>
#include "mhal_chiptop_reg.h"
#elif defined(CONFIG_ARM)
#include <asm/prom.h>
#include <asm/mach/map.h>
#elif defined(CONFIG_ARM64)
#include <asm/arm-boards/prom.h>
#include <asm/mach/map.h>
#endif


#include "mdrv_types.h"

#include "irqs.h"
#include "mdrv_noe.h"
#include "mdrv_noe_phy.h"

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------




//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
//  Global Variable and Functions
//--------------------------------------------------------------------------------------------------



static void _MDrv_NOE_MAC_Set_Link(struct END_DEVICE *ei_local)
{
    MS_BOOL bAnSupported = MHal_NOE_Support_Auto_Polling();
    struct st_drv_phy_config phy_cfg[E_NOE_GE_MAC_MAX];
    phy_cfg[E_NOE_GE_MAC1].ge = E_NOE_GE_MAC1;
    phy_cfg[E_NOE_GE_MAC2].ge = E_NOE_GE_MAC2;

    if(ei_local->architecture & GE1_RGMII_AN) {
        phy_cfg[E_NOE_GE_MAC1].mii_force_mode = (bAnSupported == TRUE)?NOE_DISABLE:NOE_ENABLE;
        if (bAnSupported == FALSE) {
            phy_cfg[E_NOE_GE_MAC1].speed = E_NOE_SPEED_1000;
            phy_cfg[E_NOE_GE_MAC1].duplex = E_NOE_DUPLEX_FULL;
        }
        if (phy_cfg[E_NOE_GE_MAC1].mii_force_mode == NOE_DISABLE) {
            phy_cfg[E_NOE_GE_MAC1].speed = E_NOE_SPEED_INVALID;
            phy_cfg[E_NOE_GE_MAC1].duplex = E_NOE_DUPLEX_INVALID;
        }
        #if defined(CONFIG_NOE_FPGA_VERIFY) || defined(CONFIG_MSTAR_ARM_BD_FPGA)
        phy_cfg[E_NOE_GE_MAC1].mii_force_mode = NOE_ENABLE;
        phy_cfg[E_NOE_GE_MAC1].speed = FPGA_NOE_PHY_SPEED;
        phy_cfg[E_NOE_GE_MAC1].duplex = FPGA_NOE_PHY_DUPLEX;
        #endif /* CONFIG_NOE_FPGA_VERIFY */
    }

    if (ei_local->features & FE_GE2_SUPPORT) {
        if (ei_local->architecture & GE2_RGMII_AN) {
            #ifdef CONFIG_MDIO_IC1819
            phy_cfg[E_NOE_GE_MAC2].mii_force_mode = NOE_ENABLE;
            if (NOE_ENABLE)
            #else
            phy_cfg[E_NOE_GE_MAC2].mii_force_mode = (bAnSupported == TRUE)?NOE_DISABLE:NOE_ENABLE;
            if (bAnSupported == FALSE)
            #endif
            {
                phy_cfg[E_NOE_GE_MAC2].speed = E_NOE_SPEED_1000;
                phy_cfg[E_NOE_GE_MAC2].duplex = E_NOE_DUPLEX_FULL;
            }
            if (phy_cfg[E_NOE_GE_MAC2].mii_force_mode == NOE_DISABLE) {
                phy_cfg[E_NOE_GE_MAC2].speed = E_NOE_SPEED_INVALID;
                phy_cfg[E_NOE_GE_MAC2].duplex = E_NOE_DUPLEX_INVALID;
            }
            #if defined(CONFIG_NOE_FPGA_VERIFY) || defined(CONFIG_MSTAR_ARM_BD_FPGA)
            phy_cfg[E_NOE_GE_MAC2].mii_force_mode = NOE_ENABLE;
            phy_cfg[E_NOE_GE_MAC2].speed = FPGA_NOE_PHY_SPEED;
            phy_cfg[E_NOE_GE_MAC2].duplex = FPGA_NOE_PHY_DUPLEX;
            #endif /* CONFIG_NOE_FPGA_VERIFY */
        }
    }

    if (ei_local->features & FE_GE2_SUPPORT) {
        MDrv_NOE_PHY_Set_Config(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2, &phy_cfg[E_NOE_GE_MAC2]);
        MHal_NOE_Force_Link_Mode(E_NOE_GE_MAC2, phy_cfg[E_NOE_GE_MAC2].speed, phy_cfg[E_NOE_GE_MAC2].duplex);
    }

    if (ei_local->architecture & GE1_RGMII_AN) {
        MDrv_NOE_PHY_Set_Config(CONFIG_MAC_TO_GIGAPHY_MODE_ADDR, &phy_cfg[E_NOE_GE_MAC1]);
        MHal_NOE_Force_Link_Mode(E_NOE_GE_MAC1, phy_cfg[E_NOE_GE_MAC1].speed, phy_cfg[E_NOE_GE_MAC1].duplex);
    }

    if (bAnSupported == TRUE) {
        if ((phy_cfg[E_NOE_GE_MAC1].mii_force_mode != NOE_ENABLE) || (phy_cfg[E_NOE_GE_MAC2].mii_force_mode != NOE_ENABLE))
            MHal_NOE_Set_Auto_Polling(E_NOE_SEL_ENABLE);
    }
    else
        MHal_NOE_Set_Auto_Polling(E_NOE_SEL_DISABLE);
}


void MDrv_NOE_MAC_Init(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    _MDrv_NOE_MAC_Set_Link(ei_local);
}


void MDrv_NOE_MAC_Detect_Link_Status(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    struct PSEUDO_ADAPTER *p_ad = netdev_priv(ei_local->pseudo_dev);
    struct noe_mac_link_status status;


    if (MHal_NOE_Get_Link_Intr_Status()) {
        MHal_NOE_Disable_Link_Intr();
        MHal_NOE_MAC_Get_Link_Status(E_NOE_GE_MAC1, &status);

        if (!netif_carrier_ok(dev)) {
            if (status.link_up == TRUE)
                netif_carrier_on(dev);
        }
        else {
            if (status.link_up == FALSE)
                netif_carrier_off(dev);
        }

        if (ei_local->features & FE_GE2_SUPPORT) {
            p_ad = netdev_priv(ei_local->pseudo_dev);
            MHal_NOE_MAC_Get_Link_Status(E_NOE_GE_MAC2, &status);
            if  (!netif_carrier_ok(ei_local->pseudo_dev)) {
                if (status.link_up == TRUE)
                    netif_carrier_on(ei_local->pseudo_dev);
            }
            else {
                if (status.link_up == FALSE)
                    netif_carrier_off(ei_local->pseudo_dev);
            }
        }
        MHal_NOE_Enable_Link_Intr();
    }

}



