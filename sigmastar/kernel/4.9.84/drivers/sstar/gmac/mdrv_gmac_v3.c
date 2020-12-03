/*
* mdrv_gmac_v3.c- Sigmastar
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
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// * Copyright (c) 2006 - 2007 Mstar Semiconductor, Inc.
// This program is free software.
// You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation;
// either version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   devGMAC.c
/// @brief  GMAC Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/module.h>
#include <linux/init.h>
//#include <linux/autoconf.h>
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
#ifdef CONFIG_OF
#include <linux/of_irq.h>
#endif
#if defined(CONFIG_MIPS)
#include <asm/mips-boards/prom.h>
#include "mhal_chiptop_reg.h"
#elif defined(CONFIG_ARM)
//#include <prom.h>
#include <asm/mach/map.h>
#elif defined(CONFIG_ARM64)
#include <asm/arm-boards/prom.h>
#include <asm/mach/map.h>
#endif
#include "mdrv_types.h"
//#include "mst_platform.h"
//#include "mdrv_system.h"
//#include "chip_int.h"
#include "mhal_gmac_v3.h"
#include "mdrv_gmac_v3.h"
//#include "chip_setup.h"

#ifdef CONFIG_GMAC_SUPPLY_RNG
#include <linux/input.h>
#include <random.h>
#include "mhal_rng_reg.h"
#endif

#include "ms_msys.h"
#include "ms_platform.h"

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
static struct net_device *gmac_dev;
struct device *gdebug_class_gmac_dev;
static int gb_tx_packet_dump_en=0;
static int gb_rx_packet_dump_en=0;


//-------------------------------------------------------------------------------------------------
//  GMAC Function
//-------------------------------------------------------------------------------------------------
static int  MDev_GMAC_tx (struct sk_buff *skb, struct net_device *dev);
static void MDev_GMAC_timer_callback( unsigned long value );
static int  MDev_GMAC_SwReset(struct net_device *dev);
//static void MDev_GMAC_Send_PausePkt(struct net_device* dev);

#ifdef RX_ZERO_COPY
static void MDev_GMAC_RX_DESC_Init_zero_copy(struct net_device *dev);
static void MDev_GMAC_RX_DESC_close_zero_copy(struct net_device *dev);
#else
static void MDev_GMAC_RX_DESC_Init_memcpy(struct net_device *dev);
static void MDev_GMAC_RX_DESC_Reset_memcpy(struct net_device *dev);
#endif

static void MDEV_GMAC_ENABLE_RX_REG(void);
static void MDEV_GMAC_DISABLE_RX_REG(void);
#ifdef RX_NAPI
static int  MDev_GMAC_RX_napi_poll(struct napi_struct *napi, int budget);
#endif

#ifndef NEW_TX_QUEUE
static int MDev_GMAC_CheckTSR(void);
#endif

#ifdef TX_NAPI
static void MDEV_GMAC_ENABLE_TX_REG(void);
static void MDEV_GMAC_DISABLE_TX_REG(void);
static int  MDev_GMAC_TX_napi_poll(struct napi_struct *napi, int budget);
#endif

#ifdef TX_DESC_MODE
static void MDev_GMAC_TX_Desc_Reset(struct net_device *dev);
static void MDev_GMAC_TX_Desc_Mode_Set(struct net_device *dev);
#endif

#ifdef TX_SOFTWARE_QUEUE
static void _MDev_GMAC_tx_reset_TX_SW_QUEUE(struct net_device* netdev);
#endif
//-------------------------------------------------------------------------------------------------
// PHY MANAGEMENT
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Access the PHY to determine the current Link speed and Mode, and update the
// MAC accordingly.
// If no link or auto-negotiation is busy, then no changes are made.
// Returns:  0 : OK
//              -1 : No link
//              -2 : AutoNegotiation still in progress
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_update_linkspeed (struct net_device *dev)
{
#ifdef CONFIG_GMAC_DETECT_FROM_PHY
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 bmsr, bmcr, lpa;
    u32 stat1000;
    u32 speed, duplex;

    if (LocPtr->phyaddr >= 32) return 0;
    /* Link status is latched, so read twice to get current value */
    MHal_GMAC_read_phy (LocPtr->phyaddr, MII_BMSR, &bmsr);
    MHal_GMAC_read_phy (LocPtr->phyaddr, MII_BMSR, &bmsr);

    /* No link */
    if (!(bmsr & BMSR_LSTATUS)){
    #ifdef GMAC_LINK_LED_CONTROL
        MHal_GMAC_link_led_off();
    #endif
        return -1;
    }
#ifdef GMAC_LINK_LED_CONTROL
    MHal_GMAC_link_led_on();
#endif
    MHal_GMAC_read_phy (LocPtr->phyaddr, MII_BMCR, &bmcr);

    /* AutoNegotiation is enabled */
    if (bmcr & BMCR_ANENABLE)
    {
        if (!(bmsr & BMSR_ANEGCOMPLETE))
        {
            GMAC_DBG("==> AutoNegotiation still in progress\n");
            return -2;
        }


        if(LocPtr->hardware_type == GMAC_GPHY)
        {
            MHal_GMAC_read_phy (LocPtr->phyaddr, MII_STAT1000, &stat1000);

            if(stat1000 & LPA_1000FULL)
            {
                speed = SPEED_1000;
                duplex = DUPLEX_FULL;
            }
            else if(stat1000 & LPA_1000HALF)
            {
                speed = SPEED_1000;
                duplex = DUPLEX_HALF;
            }
            else
            {
                MHal_GMAC_read_phy(LocPtr->phyaddr, MII_LPA, &lpa);
                if(lpa & LPA_100FULL)
                {
                    speed = SPEED_100;
                    duplex = DUPLEX_FULL;
                }
                else if (lpa & LPA_100HALF)
                {
                    speed = SPEED_100;
                    duplex = DUPLEX_HALF;
                }
                else if (lpa & LPA_10FULL)
                {
                    speed = SPEED_10;
                    duplex = DUPLEX_FULL;
                }
                else
                {
                    speed = SPEED_10;
                    duplex = DUPLEX_HALF;
                }
            }
        }
        else
        {
            MHal_GMAC_read_phy(LocPtr->phyaddr, MII_LPA, &lpa);
            if(lpa & LPA_100FULL)
            {
                speed = SPEED_100;
                duplex = DUPLEX_FULL;
            }
            else if (lpa & LPA_100HALF)
            {
                speed = SPEED_100;
                duplex = DUPLEX_HALF;
            }
            else if (lpa & LPA_10FULL)
            {
                speed = SPEED_10;
                duplex = DUPLEX_FULL;
            }
            else
            {
                speed = SPEED_10;
                duplex = DUPLEX_HALF;
            }
        }
    }
    else
    {
        speed = (bmcr & BMCR_SPEED1000) ? SPEED_1000 : ((bmcr & BMCR_SPEED100) ? SPEED_100 : SPEED_10);
        duplex = (bmcr & BMCR_FULLDPLX) ? DUPLEX_FULL : DUPLEX_HALF;
    }

    // Update the MAC //
    MHal_GMAC_update_speed_duplex(speed,duplex);
#else //defined(CONFIG_GMAC_FORCE_MAC_SPEED_100)
    MHal_GMAC_update_speed_duplex(SPEED_100, DUPLEX_FULL);
#endif
    return 0;
}

static int MDev_GMAC_get_info(struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 bmsr, bmcr, LocPtrA;
    u32 uRegStatus =0;

    // Link status is latched, so read twice to get current value //
    MHal_GMAC_read_phy (LocPtr->phyaddr, MII_BMSR, &bmsr);
    MHal_GMAC_read_phy (LocPtr->phyaddr, MII_BMSR, &bmsr);
    if (!(bmsr & BMSR_LSTATUS)){
        uRegStatus &= ~GMAC_ETHERNET_TEST_RESET_STATE;
        uRegStatus |= GMAC_ETHERNET_TEST_NO_LINK; //no link //
    }
    MHal_GMAC_read_phy (LocPtr->phyaddr, MII_BMCR, &bmcr);

    if (bmcr & BMCR_ANENABLE)
    {
        //AutoNegotiation is enabled //
        if (!(bmsr & BMSR_ANEGCOMPLETE))
        {
            uRegStatus &= ~GMAC_ETHERNET_TEST_RESET_STATE;
            uRegStatus |= GMAC_ETHERNET_TEST_AUTO_NEGOTIATION; //AutoNegotiation //
        }
        else
        {
            uRegStatus &= ~GMAC_ETHERNET_TEST_RESET_STATE;
            uRegStatus |= GMAC_ETHERNET_TEST_LINK_SUCCESS; //link success //
        }

        MHal_GMAC_read_phy (LocPtr->phyaddr, MII_LPA, &LocPtrA);
        if ((LocPtrA & LPA_100FULL) || (LocPtrA & LPA_100HALF))
        {
            uRegStatus |= GMAC_ETHERNET_TEST_SPEED_100M; //SPEED_100//
        }
        else
        {
            uRegStatus &= ~GMAC_ETHERNET_TEST_SPEED_100M; //SPEED_10//
        }

        if ((LocPtrA & LPA_100FULL) || (LocPtrA & LPA_10FULL))
        {
            uRegStatus |= GMAC_ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_FULL//
        }
        else
        {
            uRegStatus &= ~GMAC_ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_HALF//
        }
    }
    else
    {
        if(bmcr & BMCR_SPEED100)
        {
            uRegStatus |= GMAC_ETHERNET_TEST_SPEED_100M; //SPEED_100//
        }
        else
        {
            uRegStatus &= ~GMAC_ETHERNET_TEST_SPEED_100M; //SPEED_10//
        }

        if(bmcr & BMCR_FULLDPLX)
        {
            uRegStatus |= GMAC_ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_FULL//
        }
        else
        {
            uRegStatus &= ~GMAC_ETHERNET_TEST_DUPLEX_FULL; //DUPLEX_HALF//
        }
    }

    return uRegStatus;
}

//-------------------------------------------------------------------------------------------------
//Program the hardware MAC address from dev->dev_addr.
//-------------------------------------------------------------------------------------------------
void MDev_GMAC_update_mac_address (struct net_device *dev)
{
    u32 value;
    value = (dev->dev_addr[3] << 24) | (dev->dev_addr[2] << 16) | (dev->dev_addr[1] << 8) |(dev->dev_addr[0]);
    MHal_GMAC_Write_SA1L(value);
    value = (dev->dev_addr[5] << 8) | (dev->dev_addr[4]);
    MHal_GMAC_Write_SA1H(value);
}

//-------------------------------------------------------------------------------------------------
// ADDRESS MANAGEMENT
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Set the ethernet MAC address in dev->dev_addr
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_get_mac_address (struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    char addr[6];
    u32 HiAddr, LoAddr;

    // Check if bootloader set address in Specific-Address 1 //
    HiAddr = MHal_GMAC_get_SA1H_addr();
    LoAddr = MHal_GMAC_get_SA1L_addr();

    addr[0] = (LoAddr & 0xffUL);
    addr[1] = (LoAddr & 0xff00UL) >> 8;
    addr[2] = (LoAddr & 0xff0000UL) >> 16;
    addr[3] = (LoAddr & 0xff000000UL) >> 24;
    addr[4] = (HiAddr & 0xffUL);
    addr[5] = (HiAddr & 0xff00UL) >> 8;

    if (is_valid_ether_addr (addr))
    {
        GMAC_DBG("SA1_Valid!!!\n");
        memcpy (LocPtr->ThisBCE.sa1, &addr, 6);
        memcpy (dev->dev_addr, &addr, 6);
        return;
    }
    // Check if bootloader set address in Specific-Address 2 //
    HiAddr = MHal_GMAC_get_SA2H_addr();
    LoAddr = MHal_GMAC_get_SA2L_addr();
    addr[0] = (LoAddr & 0xffUL);
    addr[1] = (LoAddr & 0xff00UL) >> 8;
    addr[2] = (LoAddr & 0xff0000UL) >> 16;
    addr[3] = (LoAddr & 0xff000000UL) >> 24;
    addr[4] = (HiAddr & 0xffUL);
    addr[5] = (HiAddr & 0xff00UL) >> 8;

    if (is_valid_ether_addr (addr))
    {
        GMAC_DBG("SA2_Valid!!!\n");
        memcpy (LocPtr->ThisBCE.sa1, &addr, 6);
        memcpy (dev->dev_addr, &addr, 6);
        return;
    }
    else
    {
        GMAC_DBG("ALL Not Valid!!!, set default MAC addrees\n");
        LocPtr->ThisBCE.sa1[0]      = GMAC_MY_MAC[0];
        LocPtr->ThisBCE.sa1[1]      = GMAC_MY_MAC[1];
        LocPtr->ThisBCE.sa1[2]      = GMAC_MY_MAC[2];
        LocPtr->ThisBCE.sa1[3]      = GMAC_MY_MAC[3];
        LocPtr->ThisBCE.sa1[4]      = GMAC_MY_MAC[4];
        LocPtr->ThisBCE.sa1[5]      = GMAC_MY_MAC[5];
        memcpy (dev->dev_addr, LocPtr->ThisBCE.sa1, 6);
    }
}

#ifdef GMAC_URANUS_ETHER_ADDR_CONFIGURABLE
//-------------------------------------------------------------------------------------------------
// Store the new hardware address in dev->dev_addr, and update the MAC.
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_set_mac_address (struct net_device *dev, void *addr)
{
    struct sockaddr *address = addr;
    if (!is_valid_ether_addr (address->sa_data))
        return -EADDRNOTAVAIL;

    memcpy (dev->dev_addr, address->sa_data, dev->addr_len);
    MDev_GMAC_update_mac_address (dev);
    return 0;
}
#endif

//-------------------------------------------------------------------------------------------------
// Mstar Multicast hash rule
//-------------------------------------------------------------------------------------------------
//Hash_index[5] = da[5] ^ da[11] ^ da[17] ^ da[23] ^ da[29] ^ da[35] ^ da[41] ^ da[47]
//Hash_index[4] = da[4] ^ da[10] ^ da[16] ^ da[22] ^ da[28] ^ da[34] ^ da[40] ^ da[46]
//Hash_index[3] = da[3] ^ da[09] ^ da[15] ^ da[21] ^ da[27] ^ da[33] ^ da[39] ^ da[45]
//Hash_index[2] = da[2] ^ da[08] ^ da[14] ^ da[20] ^ da[26] ^ da[32] ^ da[38] ^ da[44]
//Hash_index[1] = da[1] ^ da[07] ^ da[13] ^ da[19] ^ da[25] ^ da[31] ^ da[37] ^ da[43]
//Hash_index[0] = da[0] ^ da[06] ^ da[12] ^ da[18] ^ da[24] ^ da[30] ^ da[36] ^ da[42]
//-------------------------------------------------------------------------------------------------
/*
static void MDev_GMAC_sethashtable(unsigned char *addr)
{
    u32 mc_filter[2];
    u32 uHashIdxBit;
    u32 uHashValue;
    u32 i;
    u32 tmpcrc;
    u32 uSubIdx;
    u64 macaddr;
    u64 mac[6];

    uHashValue = 0;
    macaddr = 0;

    // Restore mac //
    for(i = 0; i < 6;  i++)
    {
        mac[i] =(u64)addr[i];
    }

    // Truncate mac to u64 container //
    macaddr |=  mac[0] | (mac[1] << 8) | (mac[2] << 16);
    macaddr |=  (mac[3] << 24) | (mac[4] << 32) | (mac[5] << 40);

    // Caculate the hash value //
    for(uHashIdxBit = 0; uHashIdxBit < 6;  uHashIdxBit++)
    {
        tmpcrc = (macaddr & (0x1UL << uHashIdxBit)) >> uHashIdxBit;
        for(i = 1; i < 8;  i++)
        {
            uSubIdx = uHashIdxBit + (i * 6);
            tmpcrc = tmpcrc ^ ((macaddr >> uSubIdx) & 0x1);
        }
        uHashValue |= (tmpcrc << uHashIdxBit);
    }

    mc_filter[0] = MHal_GMAC_ReadReg32( GMAC_REG_ETH_HSL);
    mc_filter[1] = MHal_GMAC_ReadReg32( GMAC_REG_ETH_HSH);

    // Set the corrsponding bit according to the hash value //
    if(uHashValue < 32)
    {
        mc_filter[0] |= (0x1UL <<  uHashValue);
        MHal_GMAC_WritReg32( GMAC_REG_ETH_HSL, mc_filter[0] );
    }
    else
    {
        mc_filter[1] |= (0x1UL <<  (uHashValue - 32));
        MHal_GMAC_WritReg32( GMAC_REG_ETH_HSH, mc_filter[1] );
    }
}*/
//-------------------------------------------------------------------------------------------------
//Enable/Disable promiscuous and multicast modes.
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_set_rx_mode (struct net_device *dev)
{
    u32 uRegVal;
    uRegVal  = MHal_GMAC_Read_CFG();

    if (dev->flags & IFF_PROMISC)
    {   // Enable promiscuous mode //
        uRegVal |= GMAC_CAF;
    }
    else if (dev->flags & (~IFF_PROMISC))
    {   // Disable promiscuous mode //
        uRegVal &= ~GMAC_CAF;
    }
    MHal_GMAC_Write_CFG(uRegVal);

    if (dev->flags & IFF_ALLMULTI)
    {   // Enable all multicast mode //
        MHal_GMAC_update_HSH(-1,-1);
        uRegVal |= GMAC_MTI;
    }
    else if (dev->flags & IFF_MULTICAST)
    {   // Enable specific multicasts//
        //MDev_GMAC_sethashtable (dev);
        MHal_GMAC_update_HSH(-1,-1);
        uRegVal |= GMAC_MTI;
    }
    else if (dev->flags & ~(IFF_ALLMULTI | IFF_MULTICAST))
    {   // Disable all multicast mode//
        MHal_GMAC_update_HSH(0,0);
        uRegVal &= ~GMAC_MTI;
    }

    MHal_GMAC_Write_CFG(uRegVal);
}
//-------------------------------------------------------------------------------------------------
// IOCTL
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Enable/Disable MDIO
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_mdio_read (struct net_device *dev, int phy_id, int location)
{
    u32 value;
    MHal_GMAC_read_phy (phy_id, location, &value);
    return value;
}

static void MDev_GMAC_mdio_write (struct net_device *dev, int phy_id, int location, int value)
{
    MHal_GMAC_write_phy (phy_id, location, value);
}

//-------------------------------------------------------------------------------------------------
//ethtool support.
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_ethtool_ioctl (struct net_device *dev, void *useraddr)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 ethcmd;
    int res = 0;

    if (copy_from_user (&ethcmd, useraddr, sizeof (ethcmd)))
        return -EFAULT;

    spin_lock_irq (&LocPtr->irq_lock);

    switch (ethcmd)
    {
        case ETHTOOL_GSET:
        {
            struct ethtool_cmd ecmd = { ETHTOOL_GSET };
            res = mii_ethtool_gset (&LocPtr->mii, &ecmd);
            if (copy_to_user (useraddr, &ecmd, sizeof (ecmd)))
                res = -EFAULT;
            break;
        }
        case ETHTOOL_SSET:
        {
            struct ethtool_cmd ecmd;
            if (copy_from_user (&ecmd, useraddr, sizeof (ecmd)))
                res = -EFAULT;
            else
                res = mii_ethtool_sset (&LocPtr->mii, &ecmd);
            break;
        }
        case ETHTOOL_NWAY_RST:
        {
            res = mii_nway_restart (&LocPtr->mii);
            break;
        }
        case ETHTOOL_GLINK:
        {
            struct ethtool_value edata = { ETHTOOL_GLINK };
            edata.data = mii_link_ok (&LocPtr->mii);
            if (copy_to_user (useraddr, &edata, sizeof (edata)))
                res = -EFAULT;
            break;
        }
        default:
            res = -EOPNOTSUPP;
    }
    spin_unlock_irq (&LocPtr->irq_lock);
    return res;
}

//-------------------------------------------------------------------------------------------------
// User-space ioctl interface.
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    struct mii_ioctl_data *data = if_mii(rq);

    if (!netif_running(dev))
    {
        rq->ifr_metric = GMAC_ETHERNET_TEST_INIT_FAIL;
    }

    switch (cmd)
    {
        case SIOCGMIIPHY:
            data->phy_id = (LocPtr->phyaddr & 0x1F);
            return 0;

        case SIOCDEVPRIVATE:
            rq->ifr_metric = (MDev_GMAC_get_info(gmac_dev)|LocPtr->initstate);
            return 0;

//        case SIOCDEVON:
//            MHal_GMAC_Power_On_Clk();
//            return 0;
//
//        case SIOCDEVOFF:
//            MHal_GMAC_Power_Off_Clk();
//            return 0;

        case SIOCGMIIREG:
            // check PHY's register 1.
            if((data->reg_num & 0x1fUL) == 0x1)
            {
                // PHY's register 1 value is set by timer callback function.
                spin_lock_irq(&LocPtr->irq_lock);
                data->val_out = LocPtr->phy_status_register;
                spin_unlock_irq(&LocPtr->irq_lock);
            }
            else
            {
                MHal_GMAC_read_phy((LocPtr->phyaddr & 0x1FUL), (data->reg_num & 0x1fUL), (u32 *)&(data->val_out));
            }
            return 0;

        case SIOCSMIIREG:
            if (!capable(CAP_NET_ADMIN))
                return -EPERM;
            MHal_GMAC_write_phy((LocPtr->phyaddr & 0x1FUL), (data->reg_num & 0x1fUL), data->val_in);
            return 0;

        case SIOCETHTOOL:
            return MDev_GMAC_ethtool_ioctl (dev, (void *) rq->ifr_data);

        default:
            return -EOPNOTSUPP;
    }
}
//-------------------------------------------------------------------------------------------------
// MAC
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//Initialize and start the Receiver and Transmit subsystems
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_start (struct net_device *dev)
{
    struct GMAC_private *LocPtr;
    u32 uRegVal;

#ifdef RX_ZERO_COPY
    MDev_GMAC_RX_DESC_Init_zero_copy(dev);
#else
    MDev_GMAC_RX_DESC_Reset_memcpy(dev);
#endif

#ifdef TX_DESC_MODE
    MDev_GMAC_TX_Desc_Reset(dev);
    MDev_GMAC_TX_Desc_Mode_Set(dev);
#else
#ifdef TX_SOFTWARE_QUEUE
    _MDev_GMAC_tx_reset_TX_SW_QUEUE(dev);
#else
    LocPtr = (struct GMAC_private*) netdev_priv(dev);
    LocPtr->tx_index = 0;
#endif
#endif



    // Enable Receive and Transmit //
    uRegVal = MHal_GMAC_Read_CTL();
    uRegVal |= (GMAC_RE | GMAC_TE);
    MHal_GMAC_Write_CTL(uRegVal);
}

//-------------------------------------------------------------------------------------------------
// Open the ethernet interface
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_open (struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 uRegVal;
    int ret;

#ifdef RX_NAPI
    if(LocPtr->napi_rx.state == NAPI_STATE_SCHED)
    {
        napi_disable(&LocPtr->napi_rx);
        //GMAC_DBG("napi_disable RX!!!\n");
    }

    napi_enable(&LocPtr->napi_rx);
    //GMAC_DBG("napi_enable RX!!!\n");
    LocPtr->full_budge_count = 0;
    LocPtr->max_polling = 0;
    LocPtr->polling_count = 0;
    LocPtr->ROVR_count = 0;
#endif

#ifdef TX_NAPI
    if(LocPtr->napi_tx.state == NAPI_STATE_SCHED)
    {
        napi_disable(&LocPtr->napi_tx);
        GMAC_DBG("napi_disable TX!!!\n");
    }

    napi_enable(&LocPtr->napi_tx);
    GMAC_DBG("napi_enable TX!!!\n");
#endif

    spin_lock_irq (&LocPtr->irq_lock);
    ret = MDev_GMAC_update_linkspeed(dev);
    spin_unlock_irq (&LocPtr->irq_lock);

    if (!is_valid_ether_addr (dev->dev_addr))
       return -EADDRNOTAVAIL;

    uRegVal = MHal_GMAC_Read_CTL();
    uRegVal |= GMAC_CSR;
    MHal_GMAC_Write_CTL(uRegVal);
    // Enable PHY interrupt //
    MHal_GMAC_enable_phyirq ();

    // Enable MAC interrupts //
    MHal_GMAC_Write_IDR(0xFFFF);
#ifndef RX_DELAY_INTERRUPT
    uRegVal = GMAC_INT_RCOM | GMAC_INT_ENABLE;
    MHal_GMAC_Write_IER(uRegVal);
#else
    MHal_GMAC_Write_IER(GMAC_INT_ENABLE);
#endif

    LocPtr->ep_flag |= GMAC_EP_FLAG_OPEND;

    MDev_GMAC_start (dev);
    netif_start_queue (dev);

    add_timer(&LocPtr->Link_timer);

    /* check if network linked */
    if (-1 == ret)
    {
        MDEV_GMAC_DISABLE_RX_REG();
        netif_carrier_off(dev);
        LocPtr->ThisBCE.connected = 0;
    }
    else if(0 == ret)
    {
        netif_carrier_on(dev);
        LocPtr->ThisBCE.connected = 1;
    }

    return 0;
}
//-------------------------------------------------------------------------------------------------
// Close the interface
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_close (struct net_device *dev)
{
    u32 uRegVal;
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);

#ifdef RX_NAPI
    napi_disable(&LocPtr->napi_rx);
#endif

#ifdef TX_NAPI
    napi_disable(&LocPtr->napi_tx);
#endif

    //Disable Receiver and Transmitter //
    uRegVal = MHal_GMAC_Read_CTL();
    uRegVal &= ~(GMAC_TE | GMAC_RE);
    MHal_GMAC_Write_CTL(uRegVal);
    // Disable PHY interrupt //
    MHal_GMAC_disable_phyirq ();
#ifndef RX_DELAY_INTERRUPT
    //Disable MAC interrupts //
    uRegVal = GMAC_INT_RCOM | GMAC_INT_ENABLE;
    MHal_GMAC_Write_IDR(uRegVal);
#else
    MHal_GMAC_Write_IDR(GMAC_INT_ENABLE);
#endif
#ifdef RX_ZERO_COPY
    MDev_GMAC_RX_DESC_close_zero_copy(dev);
#endif
    netif_stop_queue (dev);
    netif_carrier_off(dev);
    del_timer(&LocPtr->Link_timer);
    LocPtr->ThisBCE.connected = 0;
    LocPtr->ep_flag &= (~GMAC_EP_FLAG_OPEND);

#ifdef TX_DESC_MODE
    MHal_GMAC_Write_LOW_PRI_TX_DESC_THRESHOLD(0x0);
#endif

#ifdef TX_SOFTWARE_QUEUE
    _MDev_GMAC_tx_reset_TX_SW_QUEUE(dev);
#endif

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Update the current statistics from the internal statistics registers.
//-------------------------------------------------------------------------------------------------
static struct net_device_stats * MDev_GMAC_stats (struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    int ale, lenerr, seqe, lcol, ecol;
    if (netif_running (dev))
    {
        LocPtr->stats.rx_packets += MHal_GMAC_Read_OK();            /* Good frames received */
        ale = MHal_GMAC_Read_ALE();
        LocPtr->stats.rx_frame_errors += ale;                       /* Alignment errors */
        lenerr = MHal_GMAC_Read_ELR();
        LocPtr->stats.rx_length_errors += lenerr;                   /* Excessive Length or Undersize Frame error */
        seqe = MHal_GMAC_Read_SEQE();
        LocPtr->stats.rx_crc_errors += seqe;                        /* CRC error */
        LocPtr->stats.rx_fifo_errors += MHal_GMAC_Read_ROVR();
        LocPtr->stats.rx_errors += ale + lenerr + seqe + MHal_GMAC_Read_SE() + MHal_GMAC_Read_RJB();
        LocPtr->stats.tx_packets += MHal_GMAC_Read_FRA();           /* Frames successfully transmitted */
        LocPtr->stats.tx_fifo_errors += MHal_GMAC_Read_TUE();       /* Transmit FIFO underruns */
        LocPtr->stats.tx_carrier_errors += MHal_GMAC_Read_CSE();    /* Carrier Sense errors */
        LocPtr->stats.tx_heartbeat_errors += MHal_GMAC_Read_SQEE(); /* Heartbeat error */
        lcol = MHal_GMAC_Read_LCOL();
        ecol = MHal_GMAC_Read_ECOL();
        LocPtr->stats.tx_window_errors += lcol;                     /* Late collisions */
        LocPtr->stats.tx_aborted_errors += ecol;                    /* 16 collisions */
        LocPtr->stats.collisions += MHal_GMAC_Read_SCOL() + MHal_GMAC_Read_MCOL() + lcol + ecol;
    }
    return &LocPtr->stats;
}

static int MDev_GMAC_TxReset(void)
{
    u32 val = MHal_GMAC_Read_CTL() & 0x000001FFUL;

    MHal_GMAC_Write_CTL((val & ~GMAC_TE));
    MHal_GMAC_Write_TCR(0);
    MHal_GMAC_Write_CTL((MHal_GMAC_Read_CTL() | GMAC_TE));
    return 0;
}

#if 0
static u8 pause_pkt[] =
{
    //DA - multicast
    0x01, 0x80, 0xC2, 0x00, 0x00, 0x01,
    //SA
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    //Len-Type
    0x88, 0x08,
    //Ctrl code
    0x00, 0x01,
    //Ctrl para 8192
    0x20, 0x00
};
#endif

void MDrv_GMAC_DumpMem(phys_addr_t addr, u32 len)
{
    u8 *ptr = (u8 *)addr;
    u32 i;

    printk("\n ===== Dump %lx =====\n", (long unsigned int)ptr);
    for (i=0; i<len; i++)
    {
        if ((u32)i%0x10UL ==0)
            printk("%lx: ", (long unsigned int)ptr);


            printk("%02x ", *ptr);

            if ((u32)i%0x10UL == 0x07UL)
                printk(" ");

        if ((u32)i%0x10UL == 0x0fUL)
            printk("\n");
    ptr++;
    }
    printk("\n");
}

#if 0
//Background send
// remove it due to directly MDev_GMAC_SwReset when RX overrun in GMAC due to h/w bug
static int MDev_GMAC_BGsend(struct net_device* dev, phys_addr_t addr, int len )
{
    dma_addr_t skb_addr;
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);

    if (NETDEV_TX_OK != MDev_GMAC_CheckTSR())
        return NETDEV_TX_BUSY;

    skb_addr = get_tx_addr();
    memcpy((void*)skb_addr,(void *)addr, len);

    LocPtr->stats.tx_bytes += len;

#ifdef CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
    if((unsigned int)skb_addr < 0xC0000000UL)
    {
        Chip_Flush_Memory_Range((unsigned int)skb_addr&0x0FFFFFFFUL, len);
    }
    else
    {
        Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
    }
    #elif defined(CONFIG_ARM)
        Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
    #else
        #ERROR
    #endif
#endif

    //Set address of the data in the Transmit Address register //
    MHal_GMAC_Write_TAR(skb_addr - LocPtr->RAM_VA_PA_OFFSET - GMAC_MIU0_BUS_BASE);

    // Set length of the packet in the Transmit Control register //
    MHal_GMAC_Write_TCR(len);

    return NETDEV_TX_OK;
}

static void MDev_GMAC_Send_PausePkt(struct net_device* dev)
{
    u32 val = MHal_GMAC_Read_CTL() & 0x000001FFUL;

    //Disable Rx
    MHal_GMAC_Write_CTL((val & ~GMAC_RE));
    memcpy(&pause_pkt[6], dev->dev_addr, 6);
    MDev_GMAC_BGsend(dev, (u32)pause_pkt, sizeof(pause_pkt));
    //Enable Rx
    MHal_GMAC_Write_CTL((MHal_GMAC_Read_CTL() | GMAC_RE));
}
#endif

#ifdef TX_ZERO_COPY
#ifdef TX_DESC_MODE
//-------------------------------------------------------------------------------------------------
//Transmit packet.
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_TX_Desc_Reset(struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);

    LocPtr->tx_desc_write_index = 0;
    LocPtr->tx_desc_read_index = 0;
    LocPtr->tx_desc_queued_number = 0;
    LocPtr->tx_desc_count= 0;
    LocPtr->tx_desc_full_count= 0;
}

static void MDev_GMAC_TX_Desc_Mode_Set(struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);

    MHal_GMAC_Write_LOW_PRI_TX_DESC_BASE(LocPtr->X_LP_DESC_BASE - GMAC_MIU0_BUS_BASE);
    MHal_GMAC_Write_LOW_PRI_TX_DESC_THRESHOLD(TX_LOW_PRI_DESC_NUMBER|GMAC_RW_TX_DESC_EN_W);
    return;
}

static u32 MDev_GMAC_TX_Free_sk_buff(struct net_device *dev, u32 work_done)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 free_sk_buffer_number, free_count, flags;

    spin_lock_irqsave(&LocPtr->tx_lock, flags);

    free_count = 0;
    free_sk_buffer_number = LocPtr->tx_desc_queued_number - (MHal_GMAC_Read_LOW_PRI_TX_DESC_QUEUED() & 0x03FFFUL);

    if(free_sk_buffer_number > 8)
    {
        free_sk_buffer_number = free_sk_buffer_number - 8;
    }
    else
    {
        free_sk_buffer_number = 0;
    }

    while(free_sk_buffer_number > free_count)
    {
        //GMAC_DBG("tx_desc_sk_buff_list = 0x%zx, tx_desc_list = 0x%zx, tx_desc_read_index = 0x%zx\n", (size_t)LocPtr->tx_desc_sk_buff_list[LocPtr->tx_desc_read_index], (size_t)LocPtr->tx_desc_list[LocPtr->tx_desc_read_index].addr, LocPtr->tx_desc_read_index);

        dma_unmap_single(&dev->dev, LocPtr->tx_desc_list[LocPtr->tx_desc_read_index].addr + GMAC_MIU0_BUS_BASE, LocPtr->tx_desc_list[LocPtr->tx_desc_read_index].low_tag & 0x03FFFUL, DMA_TO_DEVICE);
        dev_kfree_skb_any(LocPtr->tx_desc_sk_buff_list[LocPtr->tx_desc_read_index]);

        LocPtr->stats.tx_bytes += (LocPtr->tx_desc_list[LocPtr->tx_desc_read_index].low_tag & 0x03FFFUL);
        LocPtr->tx_desc_sk_buff_list[LocPtr->tx_desc_read_index]=NULL;
        LocPtr->tx_desc_read_index++;
        if(TX_LOW_PRI_DESC_NUMBER == LocPtr->tx_desc_read_index)
        {
            LocPtr->tx_desc_read_index = 0;
        }
//        EMAC_DBG("Free Next No.%d, MHal_EMAC_Read_LOW_PRI_TX_DESC_PTR() = 0x%zx\n", LocPtr->tx_desc_read_index, MHal_EMAC_Read_LOW_PRI_TX_DESC_PTR() & 0x03FFFUL);

        LocPtr->tx_desc_queued_number--;

        free_count++;
#ifdef TX_NAPI
        if(free_count + work_done > GMAC_TX_NAPI_WEIGHT)
            break;
#endif
    }
    spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
    //MHal_GMAC_Write_IDR(GMAC_INT_TCOM);

    return free_count;
}


static int MDev_GMAC_tx (struct sk_buff *skb, struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 flags;
    u32 dummy;
    dma_addr_t skb_addr;

#ifndef TX_COM_ENABLE
    MDev_GMAC_TX_Free_sk_buff(dev, 0);
#endif

    spin_lock_irqsave(&LocPtr->tx_lock, flags);

    if(LocPtr->tx_desc_queued_number > TX_DESC_REFILL_NUMBER)
    {
        //MHal_GMAC_Write_IER(GMAC_INT_TCOM);
    }

    if (skb->len > GMAC_MTU)
    {
        GMAC_DBG("Wrong Tx len:%u\n", skb->len);
        spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
        return NETDEV_TX_BUSY;
    }

   // Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);

    skb_addr = dma_map_single(&dev->dev, skb->data, skb->len, DMA_TO_DEVICE);

    if (dma_mapping_error(&dev->dev, skb_addr))
    {
        dev_kfree_skb_any(skb);
        printk(KERN_ERR"ERROR!![%s]%d\n",__FUNCTION__,__LINE__);
        dev->stats.tx_dropped++;
        goto out_unlock;
    }

    #ifdef CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
        Chip_Flush_Memory_Range((unsigned int)skb&0x0FFFFFFFUL, skb->len);
    #endif
    #endif

    LocPtr->tx_desc_sk_buff_list[LocPtr->tx_desc_write_index]= skb;
    LocPtr->tx_desc_list[LocPtr->tx_desc_write_index].addr = skb_addr - GMAC_MIU0_BUS_BASE;
    if(LocPtr->tx_desc_write_index == TX_LOW_PRI_DESC_NUMBER - 1)
    {
        LocPtr->tx_desc_list[LocPtr->tx_desc_write_index].low_tag = (skb->len & 0x3FFFUL) | GMAC_TX_DESC_WRAP;
    #ifdef CHIP_FLUSH_READ
        #if defined(CONFIG_MIPS)
            Chip_Flush_Memory_Range((u32)(&(LocPtr->tx_desc_list[LocPtr->tx_desc_write_index])) & 0x0FFFFFFF, sizeof(LocPtr->tx_desc_list[LocPtr->tx_desc_write_index]));
        #endif
    #endif
        LocPtr->tx_desc_write_index = 0;
    }
    else
    {
        LocPtr->tx_desc_list[LocPtr->tx_desc_write_index].low_tag = skb->len & 0x3FFFUL;
    #ifdef CHIP_FLUSH_READ
        #if defined(CONFIG_MIPS)
            Chip_Flush_Memory_Range((u32)(&(LocPtr->tx_desc_list[LocPtr->tx_desc_write_index])) & 0x0FFFFFFF, sizeof(LocPtr->tx_desc_list[LocPtr->tx_desc_write_index]));
        #endif
    #endif
        LocPtr->tx_desc_write_index++;
    }

    //GMAC_DBG("tx_desc_sk_buff_list = 0x%zx, tx_desc_list = 0x%zx, tx_desc_write_index = 0x%zx\n", LocPtr->tx_desc_sk_buff_list[LocPtr->tx_desc_write_index], (size_t)LocPtr->tx_desc_list[LocPtr->tx_desc_write_index].addr, (size_t)LocPtr->tx_desc_write_index);

    //Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
    //dummy = MHal_GMAC_ReadRam32((u32)&LocPtr->tx_desc_list[LocPtr->tx_desc_write_index], 0);
    //udelay(100);

    //Chip_Flush_Cache_Range_VA_PA((u32)(&(LocPtr->tx_desc_list[LocPtr->tx_desc_write_index])),(u32)(&(LocPtr->tx_desc_list[LocPtr->tx_desc_write_index])) - LocPtr->RAM_VA_PA_OFFSET ,sizeof(LocPtr->tx_desc_list[LocPtr->tx_desc_write_index]));

    if(LocPtr->tx_desc_count % 2 == 0)
    {
        //EMAC_DBG("0x%zx\n", LocPtr->tx_desc_trigger_flag);
        MHal_GMAC_Write_LOW_PRI_TX_DESC_TRANSMIT0(0x1);
    }
    else
    {
        //EMAC_DBG("0x%zx\n", LocPtr->tx_desc_trigger_flag);
        MHal_GMAC_Write_LOW_PRI_TX_DESC_TRANSMIT1(0x1);
    }

    if(LocPtr->tx_desc_write_index == TX_LOW_PRI_DESC_NUMBER - 1)
    {
        LocPtr->tx_desc_write_index = 0;
    }
    else
    {
        LocPtr->tx_desc_write_index++;
    }

    LocPtr->tx_desc_count++;
    LocPtr->tx_desc_queued_number++;

    if( LocPtr->tx_desc_queued_number == TX_LOW_PRI_DESC_NUMBER)
    {
        LocPtr->tx_desc_full_count++;
        GMAC_DBG("TX descriptor full : %d\n", LocPtr->tx_desc_full_count);
        netif_stop_queue(dev);
    }

    out_unlock:
    spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
    return NETDEV_TX_OK;

}
#else
#ifdef TX_SOFTWARE_QUEUE
//-------------------------------------------------------------------------------------------------
//Transmit packet.
//-------------------------------------------------------------------------------------------------
//  read skb from TX_SW_QUEUE to HW,
//  !!!! NO SPIN LOCK INSIDE !!!!
static int MDev_GMAC_GetTXFIFOIdle(void)
{

    u32 tsrval = 0;
    u8  avlfifo[8] = {0};
    u8  avlfifoidx;
    u8  avlfifoval = 0;

    tsrval = MHal_GMAC_Read_TSR();
    avlfifo[0] = ((tsrval & GMAC_IDLETSR) != 0)? 1 : 0;
    avlfifo[1] = ((tsrval & GMAC_BNQ)!= 0)? 1 : 0;
    avlfifo[2] = ((tsrval & GMAC_TBNQ) != 0)? 1 : 0;
    avlfifo[3] = ((tsrval & GMAC_FBNQ) != 0)? 1 : 0;
    avlfifo[4] = ((tsrval & GMAC_FIFO1IDLE) !=0)? 1 : 0;
    avlfifo[5] = ((tsrval & GMAC_FIFO2IDLE) != 0)? 1 : 0;
    avlfifo[6] = ((tsrval & GMAC_FIFO3IDLE) != 0)? 1 : 0;
    avlfifo[7] = ((tsrval & GMAC_FIFO4IDLE) != 0)? 1 : 0;

    avlfifoval = 0;
    for(avlfifoidx = 0; avlfifoidx < 8; avlfifoidx++)
    {
        avlfifoval += avlfifo[avlfifoidx];
    }

    if (avlfifoval > 4)
    {
        return avlfifoval-4;
    }
    return 0;
}

static void _MDev_GMAC_tx_read_TX_SW_QUEUE(int txIdleCount,struct net_device *dev,int intr)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
//  int txIdleCount=MDev_EMAC_GetTXFIFOIdle();

    while(txIdleCount>0){

        struct tx_ring *txq=&(LocPtr->tx_swq[LocPtr->tx_rdidx]);
        if(txq->used == TX_DESC_WROTE)
        {
            Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
            //Chip_Flush_Memory_Range((size_t)txq->skb->data,txq->skb->len);
            MHal_GMAC_Write_TAR(txq->skb_physaddr - GMAC_MIU0_BUS_BASE);
            MHal_GMAC_Write_TCR(txq->skb->len);
            txq->used=TX_DESC_READ;
            LocPtr->tx_rdidx ++;
            if(TX_SW_QUEUE_SIZE==LocPtr->tx_rdidx)
            {
                LocPtr->tx_rdwrp++;
                LocPtr->tx_rdidx =0;
            }
        }else{
            break;
        }
        txIdleCount--;
    }

}

//  clear skb from TX_SW_QUEUE
//  !!!! NO SPIN LOCK INSIDE !!!!
static void _MDev_GMAC_tx_clear_TX_SW_QUEUE(int txIdleCount,struct net_device *dev,int intr)
{

    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    int clearcnt=0;
    int fifoCount=0;//LocPtr->tx_rdidx-LocPtr->tx_clidx;
    if(0==txIdleCount)return;

    fifoCount=LocPtr->tx_rdidx-LocPtr->tx_clidx;
    if(fifoCount<0)fifoCount=LocPtr->tx_rdidx+(TX_SW_QUEUE_SIZE-LocPtr->tx_clidx);

    /*
     * "fifoCount" is the count of the packets that has been sent to the EMAC HW.
     * "(TX_FIFO_SIZE-txIdleCount)" is the packet count that has not yet been sent out completely by EMAC HW
     */
    clearcnt = fifoCount-(TX_FIFO_SIZE-txIdleCount);
    if((clearcnt > TX_FIFO_SIZE) || (clearcnt < 0)){
        printk(KERN_ERR"fifoCount in _MDev_EMAC_tx_clear_TX_SW_QUEUE() ERROR!! fifoCount=%d intr=%d, %d, %d, %d, %d\n",fifoCount,intr,LocPtr->tx_rdidx,LocPtr->tx_clidx,txIdleCount,TX_FIFO_SIZE);
    }

    while(clearcnt>0)
    {
        struct tx_ring *txq=&(LocPtr->tx_swq[LocPtr->tx_clidx]);
        if(TX_DESC_READ==txq->used)
        {

            dma_unmap_single(&dev->dev, txq->skb_physaddr, txq->skb->len, DMA_TO_DEVICE);

            LocPtr->stats.tx_bytes += txq->skb->len;

            dev_kfree_skb_any(txq->skb);

            txq->used = TX_DESC_CLEARED;
            txq->skb=NULL;
            LocPtr->tx_clidx++;
            if(TX_SW_QUEUE_SIZE==LocPtr->tx_clidx)
            {
                LocPtr->tx_clwrp++;
                LocPtr->tx_clidx =0;
            }
        }else{
            break;
        }
        clearcnt--;
    }
}

static void _MDev_GMAC_tx_reset_TX_SW_QUEUE(struct net_device* netdev)
{
    struct GMAC_private *LocPtr;
    u32 i=0;
    LocPtr = (struct GMAC_private*) netdev_priv(netdev);
    for (i=0;i<TX_SW_QUEUE_SIZE;i++)
    {
        if(LocPtr->tx_swq[i].skb != NULL)
        {
            dma_unmap_single(&netdev->dev, LocPtr->tx_swq[i].skb_physaddr, LocPtr->tx_swq[i].skb->len, DMA_TO_DEVICE);
            dev_kfree_skb_any(LocPtr->tx_swq[i].skb);

        }
        LocPtr->tx_swq[i].skb = NULL;
        LocPtr->tx_swq[i].used = TX_DESC_CLEARED;
        LocPtr->tx_swq[i].skb_physaddr = 0;
    }
    LocPtr->tx_clidx = 0;
    LocPtr->tx_wridx = 0;
    LocPtr->tx_rdidx = 0;
    LocPtr->tx_clwrp = 0;
    LocPtr->tx_wrwrp = 0;
    LocPtr->tx_rdwrp = 0;
    LocPtr->tx_swq_full_cnt=0;
}

static int MDev_GMAC_tx (struct sk_buff *skb, struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    unsigned long flags;
    dma_addr_t skb_addr;

    spin_lock_irqsave(&LocPtr->tx_lock, flags);

    if (skb->len > GMAC_MTU)
    {
        GMAC_DBG("Wrong Tx len:%u\n", skb->len);
        spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
        return NETDEV_TX_BUSY;
    }

    {
        int txIdleCount=0;//MDev_EMAC_GetTXFIFOIdle();
        //FIFO full, loop until HW empty then try again
        //This is an abnormal condition as the upper network tx_queue should already been stopped by "netif_stop_queue(dev)" in code below
        if( LocPtr->tx_swq[LocPtr->tx_wridx].used > TX_DESC_CLEARED)
        {
            printk(KERN_ERR"ABNORMAL !! %d, %d, %d, %d\n",LocPtr->tx_wridx,LocPtr->tx_rdidx,LocPtr->tx_clidx, LocPtr->tx_swq[LocPtr->tx_wridx].used );
            BUG();
            /*
            txIdleCount=MDev_EMAC_GetTXFIFOIdle();
            while(0==txIdleCount)
            {
                txIdleCount=MDev_EMAC_GetTXFIFOIdle();
            }
            _MDev_EMAC_tx_clear_TX_SW_QUEUE(txIdleCount,dev,TX_SW_QUEUE_IN_GENERAL_TX);
            */
        }

        Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);

        //map skbuffer for DMA
        skb_addr = dma_map_single(&dev->dev, skb->data, skb->len, DMA_TO_DEVICE);

        if (dma_mapping_error(&dev->dev, skb_addr))
        {
            dev_kfree_skb_any(skb);
            printk(KERN_ERR"ERROR!![%s]%d\n",__FUNCTION__,__LINE__);
            dev->stats.tx_dropped++;

            goto out_unlock;
        }

        LocPtr->tx_swq[LocPtr->tx_wridx].skb = skb;
        LocPtr->tx_swq[LocPtr->tx_wridx].skb_physaddr= skb_addr;
        LocPtr->tx_swq[LocPtr->tx_wridx].used = TX_DESC_WROTE;
        LocPtr->tx_wridx ++;
        if(TX_SW_QUEUE_SIZE==LocPtr->tx_wridx)
        {
            LocPtr->tx_wridx=0;
            LocPtr->tx_wrwrp++;
        }


        //if FIFO is full, netif_stop_queue
        if( LocPtr->tx_swq[LocPtr->tx_wridx].used > TX_DESC_CLEARED)
        {
            LocPtr->tx_swq_full_cnt++;
            netif_stop_queue(dev);
        }

        // clear & read to HW FIFO
        txIdleCount = MDev_GMAC_GetTXFIFOIdle();

        _MDev_GMAC_tx_clear_TX_SW_QUEUE(txIdleCount,dev,TX_SW_QUEUE_IN_GENERAL_TX);
        _MDev_GMAC_tx_read_TX_SW_QUEUE(txIdleCount,dev,TX_SW_QUEUE_IN_GENERAL_TX);
    }

out_unlock:
    spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
    return NETDEV_TX_OK;
}
#endif
#endif
#else
#ifdef NEW_TX_QUEUE
static int MDev_GMAC_tx (struct sk_buff *skb, struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    unsigned long flags;
    dma_addr_t skb_addr;

    //spin_lock_irqsave(&LocPtr->tx_lock, flags);
    if (skb->len > GMAC_MTU)
    {
        GMAC_DBG("Wrong Tx len:%u\n", skb->len);
        //spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
        return NETDEV_TX_BUSY;
    }

    if (MHal_GMAC_New_TX_QUEUE_OVRN_Get() == 1)
    {
        //GMAC_DBG("New_TX_QUEUE_OVRN\n");
        //spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
        return NETDEV_TX_BUSY;
    }

    spin_lock_irqsave(&LocPtr->tx_lock, flags);
    skb_addr = LocPtr->TX_BUFFER_BASE + LocPtr->RAM_VA_PA_OFFSET + TX_BUFF_ENTRY_SIZE * LocPtr->tx_index;
    LocPtr->tx_index ++;
    LocPtr->tx_index = LocPtr->tx_index % TX_BUFF_ENTRY_NUMBER;
    spin_unlock_irqrestore(&LocPtr->tx_lock, flags);


    if (!skb_addr)
    {
        GMAC_DBG("Can not get memory from GMAC area\n");
        //spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
        return -ENOMEM;
    }

    memcpy((void*)skb_addr, skb->data, skb->len);


    LocPtr->stats.tx_bytes += skb->len;

#ifdef GMAC_CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
    if((u32)skb_addr < 0xC0000000UL)
    {
        Chip_Flush_Memory_Range((unsigned int)skb_addr&0x0FFFFFFFUL, skb->len);
    }
    else
    {
        Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
    }
    #elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
        TO_DO;Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
    #else
        #ERROR
    #endif
#endif

    MHal_GMAC_Write_TAR(skb_addr - LocPtr->RAM_VA_PA_OFFSET - GMAC_MIU0_BUS_BASE);
    MHal_GMAC_Write_TCR(skb->len);

    //netif_stop_queue (dev);
    dev->trans_start = jiffies;
    dev_kfree_skb_irq(skb);
    //spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
    return NETDEV_TX_OK;
}
#elif defined(TX_DESC_MODE)
//-------------------------------------------------------------------------------------------------
//Transmit packet.
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_TX_Desc_Reset(struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);

    LocPtr->tx_desc_write_index = 0;
    LocPtr->tx_desc_read_index = 0;
    LocPtr->tx_desc_queued_number = 0;
    LocPtr->tx_desc_count = 0;
    LocPtr->tx_desc_full_count = 0;
    LocPtr->tx_index = 0;
    MHal_GMAC_Write_LOW_PRI_TX_DESC_THRESHOLD(0x0);
}

static void MDev_GMAC_TX_Desc_Mode_Set(struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);

    MHal_GMAC_Write_LOW_PRI_TX_DESC_BASE(LocPtr->TX_LP_DESC_BASE - GMAC_MIU0_BUS_BASE);
    MHal_GMAC_Write_LOW_PRI_TX_DESC_THRESHOLD(TX_LOW_PRI_DESC_NUMBER|GMAC_RW_TX_DESC_EN_W);
    return;
}

static int MDev_GMAC_tx (struct sk_buff *skb, struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    unsigned long flags;
    dma_addr_t skb_addr;

    if (skb->len > GMAC_MTU)
    {
        GMAC_DBG("Wrong Tx len:%u\n", skb->len);
        //spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
        return NETDEV_TX_BUSY;
    }

    if (MHal_GMAC_LOW_PRI_TX_DESC_MODE_OVRN_Get() == 1)
    {
        //EMAC_DBG("New_TX_QUEUE_OVRN\n");
        //spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
        return NETDEV_TX_BUSY;
    }

    spin_lock_irqsave(&LocPtr->tx_lock, flags);

    skb_addr = LocPtr->TX_BUFFER_BASE + LocPtr->RAM_VA_PA_OFFSET + TX_BUFF_ENTRY_SIZE * LocPtr->tx_index;
    LocPtr->tx_index ++;
    LocPtr->tx_index = LocPtr->tx_index % TX_BUFF_ENTRY_NUMBER;

    memcpy((void*)skb_addr, skb->data, skb->len);

    if (!skb_addr)
    {
        GMAC_DBG("Can not get memory from GMAC area\n");
        spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
        return -ENOMEM;
    }

    // Store packet information (to free when Tx completed) //
    LocPtr->stats.tx_bytes += skb->len;


#ifdef CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
    if((unsigned int)skb_addr < 0xC0000000UL)
    {
        Chip_Flush_Memory_Range((unsigned int)skb_addr&0x0FFFFFFFUL, skb->len);
    }
    else
    {
        Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
    }
    #elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
        TO_DO;Chip_Flush_Memory_Range((unsigned int)skb_addr&0x0FFFFFFFUL, skb->len);
        //Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
    #else
        #ERROR
    #endif
#endif

    LocPtr->tx_desc_list[LocPtr->tx_desc_write_index].addr = skb_addr - LocPtr->RAM_VA_PA_OFFSET - GMAC_MIU0_BUS_BASE;

    if(LocPtr->tx_desc_write_index == TX_LOW_PRI_DESC_NUMBER - 1)
    {
        LocPtr->tx_desc_list[LocPtr->tx_desc_write_index].low_tag = (skb->len & 0x3FFFUL) | GMAC_TX_DESC_WRAP;
#ifdef GMAC_CHIP_FLUSH_READ
    Chip_Flush_Memory_Range((u32)(&(LocPtr->tx_desc_list[LocPtr->tx_desc_write_index])) & 0x0FFFFFFF, sizeof(LocPtr->tx_desc_list[LocPtr->tx_desc_write_index]));
#endif

        LocPtr->tx_desc_write_index = 0;
    }
    else
    {
        LocPtr->tx_desc_list[LocPtr->tx_desc_write_index].low_tag = skb->len & 0x3FFFUL;
#ifdef GMAC_CHIP_FLUSH_READ
    Chip_Flush_Memory_Range((u32)(&(LocPtr->tx_desc_list[LocPtr->tx_desc_write_index])) & 0x0FFFFFFF, sizeof(LocPtr->tx_desc_list[LocPtr->tx_desc_write_index]));
#endif
        LocPtr->tx_desc_write_index++;
    }

    if(LocPtr->tx_desc_count % 2 == 0)
    {
        //GMAC_DBG("0x%zx\n", LocPtr->tx_desc_trigger_flag);
        MHal_GMAC_Write_LOW_PRI_TX_DESC_TRANSMIT0(0x1);
    }
    else
    {
        //GMAC_DBG("0x%zx\n", LocPtr->tx_desc_trigger_flag);
        MHal_GMAC_Write_LOW_PRI_TX_DESC_TRANSMIT1(0x1);
    }

    LocPtr->tx_desc_count++;

    spin_unlock_irqrestore(&LocPtr->tx_lock, flags);

    //netif_stop_queue (dev);
    dev->trans_start = jiffies;
    dev_kfree_skb_irq(skb);

    return NETDEV_TX_OK;
}
#else
//-------------------------------------------------------------------------------------------------
//Transmit packet.
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_CheckTSR(void)
{
    u32 tsrval = 0;

    #ifdef GMAC_TX_QUEUE_4
    u8  avlfifo[8] = {0};
    u8  avlfifoidx;
    u8  avlfifoval = 0;

    //for (check = 0; check < GMAC_CHECK_CNT; check++)
    {
        tsrval = MHal_GMAC_Read_TSR();

        avlfifo[0] = ((tsrval & GMAC_IDLETSR) != 0)? 1 : 0;
        avlfifo[1] = ((tsrval & GMAC_BNQ)!= 0)? 1 : 0;
        avlfifo[2] = ((tsrval & GMAC_TBNQ) != 0)? 1 : 0;
        avlfifo[3] = ((tsrval & GMAC_FBNQ) != 0)? 1 : 0;
        avlfifo[4] = ((tsrval & GMAC_FIFO1IDLE) !=0)? 1 : 0;
        avlfifo[5] = ((tsrval & GMAC_FIFO2IDLE) != 0)? 1 : 0;
        avlfifo[6] = ((tsrval & GMAC_FIFO3IDLE) != 0)? 1 : 0;
        avlfifo[7] = ((tsrval & GMAC_FIFO4IDLE) != 0)? 1 : 0;

        avlfifoval = 0;

        for(avlfifoidx = 0; avlfifoidx < 8; avlfifoidx++)
        {
            avlfifoval += avlfifo[avlfifoidx];
        }

        if (avlfifoval > 4)
            return NETDEV_TX_OK;
    }
    #else
    //for (check = 0; check < GMAC_CHECK_CNT; check++)
    {
        tsrval = MHal_GMAC_Read_TSR();

        // check GMAC_FIFO1IDLE is ok for gmac one queue
        if ((tsrval & GMAC_IDLETSR) && (tsrval & GMAC_FIFO1IDLE))
            return NETDEV_TX_OK;
    }
    #endif

    //GMAC_DBG("Err CheckTSR:0x%x\n", tsrval);
    //MDev_GMAC_TxReset();

    return NETDEV_TX_BUSY;
}

static int MDev_GMAC_tx (struct sk_buff *skb, struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    unsigned long flags;
    dma_addr_t skb_addr;

    //spin_lock_irqsave(&LocPtr->tx_lock, flags);

    if (skb->len > GMAC_MTU)
    {
        GMAC_DBG("Wrong Tx len:%u\n", skb->len);
        //spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
        return NETDEV_TX_BUSY;
    }

    if (NETDEV_TX_OK != MDev_GMAC_CheckTSR())
    {
        //spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
        return NETDEV_TX_BUSY; //check
    }

    spin_lock_irqsave(&LocPtr->tx_lock, flags);

    skb_addr = LocPtr->TX_BUFFER_BASE + LocPtr->RAM_VA_PA_OFFSET + TX_BUFF_ENTRY_SIZE * LocPtr->tx_index;
    LocPtr->tx_index ++;
    LocPtr->tx_index = LocPtr->tx_index % TX_BUFF_ENTRY_NUMBER;

    spin_unlock_irqrestore(&LocPtr->tx_lock, flags);

    memcpy((void*)skb_addr, skb->data, skb->len);

    if (!skb_addr)
    {
        GMAC_DBG("Can not get memory from EMAC area\n");
        //spin_unlock_irqrestore(&LocPtr->tx_lock, flags);
        return -ENOMEM;
    }

    // Store packet information (to free when Tx completed) //
    LocPtr->stats.tx_bytes += skb->len;


#ifdef GMAC_CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
    if((unsigned int)skb_addr < 0xC0000000UL)
    {
        Chip_Flush_Memory_Range((unsigned int)skb_addr&0x0FFFFFFFUL, skb->len);
    }
    else
    {
        Chip_Flush_Memory_Range(0, 0xFFFFFFFFUL);
    }
    #elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
    Chip_Flush_Cache_Range((size_t)skb->data, skb->len);
    #else
        #ERROR
    #endif
#endif
    //Moniter TX packet
    if(gb_tx_packet_dump_en)
        MDrv_GMAC_DumpMem(skb_addr, skb->len);

    //Set address of the data in the Transmit Address register //
    MHal_GMAC_Write_TAR(skb_addr - LocPtr->RAM_VA_PA_OFFSET - GMAC_MIU0_BUS_BASE);

    // Set length of the packet in the Transmit Control register //
    MHal_GMAC_Write_TCR(skb->len);

    //netif_stop_queue (dev);
    dev->trans_start = jiffies;
    dev_kfree_skb_irq(skb);
    //spin_unlock_irqrestore(&LocPtr->tx_lock, flags);

    return NETDEV_TX_OK;
}
#endif
#endif

#ifdef RX_ZERO_COPY
//-------------------------------------------------------------------------------------------------
// Extract received frame from buffer descriptors and sent to upper layers.
// (Called from interrupt context)
// (Enable RX software discriptor)
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_RX_DESC_Init_zero_copy(struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    struct sk_buff *skb = NULL;
    u32 skb_addr;
    u32 rx_desc_index;
    int retry_cnt = 0;

    MDev_GMAC_RX_DESC_close_zero_copy(dev);

    memset((void *)LocPtr->RX_DESC_BASE + LocPtr->RAM_VA_PA_OFFSET, 0x00UL, RX_DESC_TABLE_SIZE);

    for(rx_desc_index = 0; rx_desc_index < RX_DESC_NUMBER; rx_desc_index++)
    {
        skb = alloc_skb(RX_BUFF_ENTRY_SIZE, GFP_ATOMIC);

        while (!skb) {
            if (retry_cnt > 10) {
                GMAC_DBG("MDev_GMAC_RX_DESC_Init_zero_copy: alloc skb fail for %d times!\n", retry_cnt);
                BUG_ON(1);
            }
            skb = alloc_skb(RX_BUFF_ENTRY_SIZE, GFP_ATOMIC);
            retry_cnt++;
        }
        retry_cnt = 0;

        if((skb_addr = (u32)__virt_to_phys((unsigned long)skb->data)) > MSTAR_MIU1_BUS_BASE)
        {
            skb_addr -= MSTAR_MIU1_BUS_BASE;
        }
        else
        {
            skb_addr -= MSTAR_MIU0_BUS_BASE;
        }

        LocPtr->rx_desc_sk_buff_list[rx_desc_index] = skb;
        if(rx_desc_index < (RX_DESC_NUMBER - 1))
        {
            LocPtr->rx_desc_list[rx_desc_index].addr = (u32)skb_addr;
        }
        else
        {
            LocPtr->rx_desc_list[rx_desc_index].addr = (u32)skb_addr + GMAC_DESC_WRAP;
        }

        skb = NULL;
    }

    LocPtr->rx_desc_read_index = 0;

    MHal_GMAC_Write_RBQP(LocPtr->RX_DESC_BASE - GMAC_MIU0_BUS_BASE);
    //MHal_GMAC_Write_BUFF(GMAC_CLEAR_BUFF);

    return;
}

static void MDev_GMAC_RX_DESC_close_zero_copy(struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 rx_desc_index;

    for(rx_desc_index = 0; rx_desc_index < RX_DESC_NUMBER; rx_desc_index++)
    {
        if(LocPtr->rx_desc_sk_buff_list[rx_desc_index])
            dev_kfree_skb_any(LocPtr->rx_desc_sk_buff_list[rx_desc_index]);
        LocPtr->rx_desc_sk_buff_list[rx_desc_index] = NULL;
    }
    return;
}

static int MDev_GMAC_rx (struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 pktlen=0;
    u32 received_number=0;
    struct sk_buff *skb = NULL;
    int retry_cnt = 0;

    do
    {
        if(!((LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr) & GMAC_DESC_DONE))
        {
            if (LocPtr->rx_desc_list[(LocPtr->rx_desc_read_index + RX_DESC_NUMBER - 1) % RX_DESC_NUMBER].addr & GMAC_DESC_DONE)
                LocPtr->rx_desc_list[(LocPtr->rx_desc_read_index + RX_DESC_NUMBER - 1) % RX_DESC_NUMBER].addr &= ~GMAC_DESC_DONE;
            break;
        }

        pktlen = ((LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].high_tag & 0x7UL) << 11) | (LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].low_tag& 0x7ffUL);    /* Length of frame including FCS */

        if (pktlen > GMAC_MTU || pktlen < 64)
        {
            //GMAC_DBG("Packet RX too large!!(pktlen = %d)\n", pktlen);
            LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr &= ~GMAC_DESC_DONE;
            Chip_Flush_MIU_Pipe();
            LocPtr->rx_desc_read_index++;
            if (LocPtr->rx_desc_read_index == RX_DESC_NUMBER)
            {
                LocPtr->rx_desc_read_index = 0;
            }

            LocPtr->stats.rx_length_errors++;
            LocPtr->stats.rx_errors++;
            LocPtr->stats.rx_dropped++;
            continue;
        }

        pktlen = pktlen - 4;

        skb_put(LocPtr->rx_desc_sk_buff_list[LocPtr->rx_desc_read_index], pktlen);

        LocPtr->rx_desc_sk_buff_list[LocPtr->rx_desc_read_index]->protocol = eth_type_trans (LocPtr->rx_desc_sk_buff_list[LocPtr->rx_desc_read_index], dev);
        dev->last_rx = jiffies;

        if(((LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].low_tag & GMAC_DESC_TCP ) || (LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].low_tag & GMAC_DESC_UDP )) && \
            (LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].low_tag & GMAC_DESC_IP_CSUM) && \
            (LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].low_tag & GMAC_DESC_TCP_UDP_CSUM) )
        {
            LocPtr->rx_desc_sk_buff_list[LocPtr->rx_desc_read_index]->ip_summed = CHECKSUM_UNNECESSARY;
        }

    #ifdef RX_NAPI
    #ifdef RX_GRO
        napi_gro_receive(&LocPtr->napi_rx,LocPtr->rx_desc_sk_buff_list[LocPtr->rx_desc_read_index]);
    #else
        netif_receive_skb(LocPtr->rx_desc_sk_buff_list[LocPtr->rx_desc_read_index]);
    #endif
    #else
        netif_rx(LocPtr->rx_desc_sk_buff_list[LocPtr->rx_desc_read_index]);
    #endif

        received_number++;
        LocPtr->stats.rx_bytes += pktlen;

        if (LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].low_tag& GMAC_MULTICAST)
        {
            LocPtr->stats.multicast++;
        }

        skb = alloc_skb(RX_BUFF_ENTRY_SIZE, GFP_ATOMIC);

        while (!skb) {
            if (retry_cnt > 10) {
                GMAC_DBG("MDev_GMAC_rx: alloc skb fail for %d times!\n", retry_cnt);
                BUG_ON(1);
            }
            skb = alloc_skb(RX_BUFF_ENTRY_SIZE, GFP_ATOMIC);
            retry_cnt++;
        }

        LocPtr->rx_desc_sk_buff_list[LocPtr->rx_desc_read_index] = skb;
        skb = NULL;

        if(LocPtr->rx_desc_read_index < (RX_DESC_NUMBER - 1))
        {
            if((LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr = (u32)__virt_to_phys((unsigned long)LocPtr->rx_desc_sk_buff_list[LocPtr->rx_desc_read_index]->data)) > MSTAR_MIU1_BUS_BASE)
            {
                LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr -= MSTAR_MIU1_BUS_BASE;
            }
            else
            {
                LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr -= MSTAR_MIU0_BUS_BASE;
            }
        }
        else
        {
            if((LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr = (u32)__virt_to_phys((unsigned long)LocPtr->rx_desc_sk_buff_list[LocPtr->rx_desc_read_index]->data)) > MSTAR_MIU1_BUS_BASE)
            {
                LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr -= MSTAR_MIU1_BUS_BASE;
                LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr += GMAC_DESC_WRAP;
            }
            else
            {
                LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr -= MSTAR_MIU0_BUS_BASE;
                LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr += GMAC_DESC_WRAP;
            }
        }

        Chip_Flush_MIU_Pipe();

        LocPtr->rx_desc_read_index++;
        if (LocPtr->rx_desc_read_index == RX_DESC_NUMBER)
        {
            LocPtr->rx_desc_read_index = 0;
        }

    #ifdef RX_NAPI
        if(received_number >= LocPtr->napi_rx.weight) {
            break;
        }
    #endif

    }while(1);

    return received_number;
}
#else //#ifdef GMAC_RX_SOFTWARE_DESCRIPTOR

//-------------------------------------------------------------------------------------------------
// Extract received frame from buffer descriptors and sent to upper layers.
// (Called from interrupt context)
// (Disable RX software discriptor)
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_RX_DESC_Init_memcpy(struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 rx_desc_index;

    memset((u8*)LocPtr->RAM_VA_PA_OFFSET + LocPtr->RX_BUFFER_BASE, 0x00UL, RX_BUFF_SIZE);

    for(rx_desc_index = 0; rx_desc_index < RX_DESC_NUMBER; rx_desc_index++)
    {
        if(rx_desc_index < (RX_DESC_NUMBER - 1))
        {
            LocPtr->rx_desc_list[rx_desc_index].addr = LocPtr->RX_BUFFER_BASE - GMAC_MIU0_BUS_BASE + RX_BUFF_ENTRY_SIZE * rx_desc_index;
        }
        else
        {
            LocPtr->rx_desc_list[rx_desc_index].addr = (LocPtr->RX_BUFFER_BASE - GMAC_MIU0_BUS_BASE + RX_BUFF_ENTRY_SIZE * rx_desc_index) | GMAC_DESC_WRAP;
        }
    }

    return;
}

static void MDev_GMAC_RX_DESC_Reset_memcpy(struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 rx_desc_index;

    for(rx_desc_index = 0; rx_desc_index < RX_DESC_NUMBER; rx_desc_index++)
    {
        LocPtr->rx_desc_list[rx_desc_index].addr &= ~GMAC_DESC_DONE;
    }

    // Program address of descriptor list in Rx Buffer Queue register //
    MHal_GMAC_Write_RBQP(LocPtr->RX_DESC_BASE - GMAC_MIU0_BUS_BASE);

    //Reset buffer index//
    LocPtr->rx_desc_read_index = 0;

    return;
}

static int MDev_GMAC_rx (struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    unsigned char *p_recv;
    u32 pktlen;
    u32 retval=0;
    u32 received_number=0;
    struct sk_buff *skb;

    // If any Ownership bit is 1, frame received.
    //while ( (dlist->descriptors[LocPtr->rxBuffIndex].addr )& GMAC_DESC_DONE)
    do
    {
#ifdef GMAC_CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
        Chip_Read_Memory_Range((u32)(&(LocPtr->rx_desc_list[LocPtr->rx_desc_read_index])) & 0x0FFFFFFFUL, sizeof(struct rx_descriptor));
    #elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
        Chip_Inv_Cache_Range((u32)(&LocPtr->rx_desc_list[LocPtr->rx_desc_read_index]), sizeof(struct rx_descriptor));
    #else
        #ERROR
    #endif
#endif
        if (!(LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr & GMAC_DESC_DONE)) {
            if (LocPtr->rx_desc_list[(LocPtr->rx_desc_read_index + RX_DESC_NUMBER - 1) % RX_DESC_NUMBER].addr & GMAC_DESC_DONE)
                LocPtr->rx_desc_list[(LocPtr->rx_desc_read_index + RX_DESC_NUMBER - 1) % RX_DESC_NUMBER].addr &= ~GMAC_DESC_DONE;
                break;
        }

        p_recv = (char *) ((((LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr) & 0xFFFFFFFFUL) + LocPtr->RAM_VA_PA_OFFSET + GMAC_MIU0_BUS_BASE) & ~(GMAC_DESC_DONE | GMAC_DESC_WRAP));
        pktlen = ((LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].high_tag & 0x7) << 11) | (LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].low_tag & 0x7ffUL);    /* Length of frame including FCS */

        skb = alloc_skb (pktlen + 6, GFP_ATOMIC);

        if (skb != NULL)
        {
            skb_reserve (skb, 2);
    #ifdef GMAC_CHIP_FLUSH_READ
        #if defined(CONFIG_MIPS)
           if((u32)p_recv < 0xC0000000UL)
           {
               Chip_Read_Memory_Range((u32)(p_recv) & 0x0FFFFFFFUL, pktlen);
           }
           else
           {
               Chip_Read_Memory_Range(0, 0xFFFFFFFFUL);
           }
        #elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
        Chip_Inv_Cache_Range((size_t)p_recv, pktlen);
        #else
            #ERROR
        #endif
    #endif
            if(gb_rx_packet_dump_en)
                MDrv_GMAC_DumpMem((phys_addr_t)p_recv, pktlen);

            memcpy(skb_put(skb, pktlen), p_recv, pktlen);
            skb->protocol = eth_type_trans (skb, dev);
            dev->last_rx = jiffies;
            LocPtr->stats.rx_bytes += pktlen;


        #ifdef GMAC_RX_CHECKSUM
            if(((LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].low_tag & GMAC_DESC_TCP ) || (LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].low_tag & GMAC_DESC_UDP )) && \
               (LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].low_tag & GMAC_DESC_IP_CSUM) && \
               (LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].low_tag & GMAC_DESC_TCP_UDP_CSUM) )
            {
                skb->ip_summed = CHECKSUM_UNNECESSARY;
            }
            else
            {
                skb->ip_summed = CHECKSUM_NONE;
            }
        #endif

        #ifdef RX_NAPI
        #ifdef RX_GRO
            retval = napi_gro_receive(&LocPtr->napi_rx, skb);
        #else
            retval = netif_receive_skb(skb);
        #endif
        #else
            retval = netif_rx(skb);
        #endif

        received_number++;
        }
        else
        {
            GMAC_DBG("alloc_skb fail!!!\n");
            //LocPtr->stats.rx_dropped += 1;
        }

        if (LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].low_tag & GMAC_MULTICAST)
        {
            LocPtr->stats.multicast++;
        }
        LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr  &= ~GMAC_DESC_DONE;  /* reset ownership bit */
#ifdef GMAC_CHIP_FLUSH_READ
    #if defined(CONFIG_MIPS)
        Chip_Flush_Memory_Range((u32)(&(LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr)) & 0x0FFFFFFFUL, sizeof(LocPtr->rx_desc_list[LocPtr->rx_desc_read_index].addr));
    #elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
        Chip_Flush_Cache_Range((u32)(&LocPtr->rx_desc_list[LocPtr->rx_desc_read_index]), sizeof(struct rx_descriptor));
    #else
        #ERROR
    #endif
#endif

        //wrap after last buffer //
        LocPtr->rx_desc_read_index++;
        if (LocPtr->rx_desc_read_index == RX_DESC_NUMBER)
        {
            LocPtr->rx_desc_read_index = 0;
        }

    #ifdef RX_NAPI
        if(received_number >= LocPtr->napi_rx.weight) {
                break;
            }
    #else
        if(received_number >= GMAC_RX_NAPI_WEIGHT) {
            break;
        }
    #endif

    }while(1);
    return received_number;
}
#endif //#ifdef GMAC_RX_SOFTWARE_DESCRIPTOR

// Enable MAC interrupts
static void MDEV_GMAC_ENABLE_RX_REG(void)
{
    u32 uRegVal;
    //printk( KERN_ERR "[EMAC] %s\n" , __FUNCTION__);
#ifndef RX_DELAY_INTERRUPT
    // disable MAC interrupts
    uRegVal = GMAC_INT_RCOM | GMAC_INT_ENABLE;
    MHal_GMAC_Write_IER(uRegVal);
#else
    uRegVal = GMAC_INT_ENABLE;
    MHal_GMAC_Write_IER(uRegVal);
    // enable delay interrupt
    uRegVal = MHal_GMAC_Read_Network_config_register3();
    uRegVal |= 0x00000080UL;
    MHal_GMAC_Write_Network_config_register3(uRegVal);
#endif
}

// Disable MAC interrupts
static void MDEV_GMAC_DISABLE_RX_REG(void)
{
    u32 uRegVal;
    //printk( KERN_ERR "[EMAC] %s\n" , __FUNCTION__);
#ifndef RX_DELAY_INTERRUPT
    // Enable MAC interrupts
    uRegVal = GMAC_INT_RCOM | GMAC_INT_ENABLE;
    MHal_GMAC_Write_IDR(uRegVal);
#else
    uRegVal = 0xFFFF;
    MHal_GMAC_Write_IDR(uRegVal);
    // disable delay interrupt
    uRegVal = MHal_GMAC_Read_Network_config_register3();
    uRegVal &= ~(0x00000080UL);
    MHal_GMAC_Write_Network_config_register3(uRegVal);
#endif
}

#ifdef RX_NAPI
static int MDev_GMAC_RX_napi_poll(struct napi_struct *napi, int budget)
{
    struct GMAC_private  *LocPtr = container_of(napi, struct GMAC_private, napi_rx);
    struct net_device *dev = LocPtr->dev;
    int work_done = 0;

    work_done = MDev_GMAC_rx(dev);

    if (work_done < budget) {
        napi_complete(napi);
        // enable MAC interrupt
        MDEV_GMAC_ENABLE_RX_REG();
    }

    return work_done;
}
#endif

#ifdef TX_NAPI
// Enable MAC interrupts
static void MDEV_GMAC_ENABLE_TX_REG(void)
{
    MHal_GMAC_Write_IER(GMAC_INT_TCOM);
}

// Disable MAC interrupts
static void MDEV_GMAC_DISABLE_TX_REG(void)
{
    MHal_GMAC_Write_IDR(GMAC_INT_TCOM);
}

static int MDev_GMAC_TX_napi_poll(struct napi_struct *napi, int budget)
{
    struct GMAC_private  *LocPtr = container_of(napi, struct GMAC_private, napi_tx);
    struct net_device *dev = LocPtr->dev;
    unsigned long flags = 0;
    int work_done = 0;
    int count = 0;

    //GMAC_DBG("MDev_GMAC_TX_napi_poll, budget = %d\n", budget);

    while(work_done < budget)
    {
        //GMAC_DBG("work_done = %d, count = %d\n", work_done, count);

        if((count = MDev_GMAC_TX_Free_sk_buff(dev, work_done)) == 0)
            break;

        work_done += count;
    }

    /* If budget not fully consumed, exit the polling mode */
    if (work_done < budget) {
        napi_complete(napi);
        // enable MAC interrupt
        spin_lock_irqsave(&LocPtr->irq_lock, flags);
        MDEV_GMAC_ENABLE_TX_REG();
        spin_unlock_irqrestore(&LocPtr->irq_lock, flags);
    }
    else
    {
//        GMAC_DBG("TX Over Budget!!! \n");
    }

    return work_done;
}
#endif

#ifdef TX_COM_ENABLE
irqreturn_t MDev_GMAC_interrupt(int irq,void *dev_id)
{
    struct net_device *dev = (struct net_device *) dev_id;
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 intstatus=0;
    u32 delay_int_status=0;
    unsigned long flags;
#ifdef TX_SOFTWARE_QUEUE
    int txIdleCount=0;//MDev_EMAC_GetTXFIFOIdle();
#endif

#ifndef RX_ZERO_COPY
    u32 wp = 0;
#endif

    spin_lock_irqsave(&LocPtr->irq_lock, flags);
    //MAC Interrupt Status register indicates what interrupts are pending.
    //It is automatically cleared once read.
    delay_int_status = MHal_GMAC_Read_Delay_interrupt_status();
    intstatus = MHal_GMAC_Read_ISR() & (~(MHal_GMAC_Read_IMR())) & GMAC_INT_MASK;

#ifndef RX_ZERO_COPY
    wp = MHal_GMAC_Read_Network_config_register2() & 0x00100000UL;
    if(wp)
    {
        GMAC_DBG("GMAC HW write invalid address");
    }
#endif

    //while((delay_int_status & 0x8000UL) || (intstatus = (MHal_GMAC_Read_ISR() & ~MHal_GMAC_Read_IMR() & GMAC_INT_MASK )) )
    {
        if (intstatus & GMAC_INT_RBNA)
        {
            GMAC_DBG("RBNA!!!!\n");
            LocPtr->stats.rx_dropped ++;
            //LocPtr->ThisUVE.flagRBNA = 1;
            //write 1 clear
            MHal_GMAC_Write_RSR(GMAC_BNA);
        }

        // Transmit complete //
        if (intstatus & GMAC_INT_TCOM)
        {

            // The TCOM bit is set even if the transmission failed. //
            if (intstatus & (GMAC_INT_TUND | GMAC_INT_RTRY))
            {
                LocPtr->stats.tx_errors += 1;
                if(intstatus & GMAC_INT_TUND)
                {
                    //write 1 clear
                    MHal_GMAC_Write_TSR(GMAC_UND);

                    //Reset TX engine
                    MDev_GMAC_TxReset();
                    GMAC_DBG ("Transmit TUND error, TX reset\n");
                }
            }

        #ifdef TX_DESC_MODE
        #ifdef TX_NAPI
            /* Receive packets are processed by poll routine. If not running start it now. */
            if (napi_schedule_prep(&LocPtr->napi_tx)) {
                MDEV_GMAC_DISABLE_TX_REG();
                __napi_schedule(&LocPtr->napi_tx);
            }
        #else
            MDev_GMAC_TX_Free_sk_buff(dev, 0);
        #endif
            if (((LocPtr->ep_flag&GMAC_EP_FLAG_SUSPENDING)==0) && netif_queue_stopped (dev) && (LocPtr->tx_desc_queued_number < TX_DESC_REFILL_NUMBER));
                netif_wake_queue(dev);
        #elif defined(TX_SOFTWARE_QUEUE)
            if (((LocPtr->ep_flag&GMAC_EP_FLAG_SUSPENDING)==0) && netif_queue_stopped (dev))
                netif_wake_queue(dev);
            LocPtr->tx_irqcnt++;
            txIdleCount=MDev_GMAC_GetTXFIFOIdle();
            while(txIdleCount>0 && (LocPtr->tx_rdidx != LocPtr->tx_wridx))
            {

                _MDev_GMAC_tx_clear_TX_SW_QUEUE(txIdleCount,dev,TX_SW_QUEUE_IN_IRQ);
                _MDev_GMAC_tx_read_TX_SW_QUEUE(txIdleCount,dev,TX_SW_QUEUE_IN_IRQ);
                txIdleCount=MDev_GMAC_GetTXFIFOIdle();
            }
        #else
            if (((LocPtr->ep_flag&GMAC_EP_FLAG_SUSPENDING)==0) && netif_queue_stopped (dev))
                netif_wake_queue(dev);
        #endif
        }

        if(intstatus&GMAC_INT_DONE)
        {
            LocPtr->ThisUVE.flagISR_INT_DONE = 0x01UL;
        }

        //RX Overrun //
        if(intstatus & GMAC_INT_ROVR)
        {
            GMAC_DBG("ROVR!!!!\n");
            LocPtr->stats.rx_dropped++;
            LocPtr->ROVR_count++;
            //write 1 clear
            MHal_GMAC_Write_RSR(GMAC_RSROVR);

#ifdef GMAC_ROVR_SW_RESET
            if (LocPtr->ROVR_count >= 6)
            {
                MDev_GMAC_SwReset(dev);
            }
#endif
        }
        else
        {
            LocPtr->ROVR_count = 0;
        }

        // Receive complete //
        if(delay_int_status & 0x8000UL)
        {
        #ifdef RX_NAPI
            /* Receive packets are processed by poll routine. If not running start it now. */
            if (napi_schedule_prep(&LocPtr->napi_rx)) {
                MDEV_GMAC_DISABLE_RX_REG();
                __napi_schedule(&LocPtr->napi_rx);
            }
        #else
            MDev_GMAC_rx(dev);
        #endif
        }
        //delay_int_status = MHal_GMAC_Read_Delay_interrupt_status();
    }
    spin_unlock_irqrestore(&LocPtr->irq_lock, flags);
    return IRQ_HANDLED;
}
#else
irqreturn_t MDev_GMAC_interrupt(int irq,void *dev_id)
{
    struct net_device *dev = (struct net_device *) dev_id;
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 intstatus = 0;
    u32 delay_int_status = 0;
    unsigned long flags;
#ifdef TX_SOFTWARE_QUEUE
    int txIdleCount=0;//MDev_EMAC_GetTXFIFOIdle();
#endif

#ifndef RX_ZERO_COPY
    u32 wp = 0;
#endif

    spin_lock_irqsave(&LocPtr->irq_lock, flags);
    //MAC Interrupt Status register indicates what interrupts are pending.
    //It is automatically cleared once read.
    delay_int_status = MHal_GMAC_Read_Delay_interrupt_status();
    intstatus = MHal_GMAC_Read_ISR() & (~(MHal_GMAC_Read_IMR())) & GMAC_INT_MASK;

#ifndef RX_ZERO_COPY
    wp = MHal_GMAC_Read_Network_config_register2() & 0x00100000UL;
    if(wp)
    {
        GMAC_DBG("GMAC HW write invalid address");
    }
#endif

    //while((delay_int_status & 0x8000UL) || (intstatus = (MHal_GMAC_Read_ISR() & ~MHal_GMAC_Read_IMR() & GMAC_INT_MASK )) )
    {
        if (((LocPtr->ep_flag&GMAC_EP_FLAG_SUSPENDING)==0) && netif_queue_stopped (dev))
        {
            netif_wake_queue(dev);
        }

        if (intstatus & GMAC_INT_RBNA)
        {
            GMAC_DBG("RBNA!!!!\n");
            LocPtr->stats.rx_dropped ++;
            //LocPtr->ThisUVE.flagRBNA = 1;
            //write 1 clear
            MHal_GMAC_Write_RSR(GMAC_BNA);
#ifdef RX_NAPI
            if (napi_schedule_prep(&LocPtr->napi_rx)) {
                MDEV_GMAC_DISABLE_RX_REG();
                __napi_schedule(&LocPtr->napi_rx);
            }
#else
            MDEV_GMAC_DISABLE_RX_REG();
            MDev_GMAC_rx(dev);
            MHal_GMAC_Write_RSR(GMAC_BNA);
            MDEV_GMAC_ENABLE_RX_REG();
#endif
        }

        // The TCOM bit is set even if the transmission failed. //
        if (intstatus & (GMAC_INT_TUND | GMAC_INT_RTRY))
        {
            LocPtr->stats.tx_errors += 1;

            if(intstatus & GMAC_INT_TUND)
            {
                //write 1 clear
                MHal_GMAC_Write_TSR(GMAC_UND);

                //Reset TX engine
                MDev_GMAC_TxReset();
                GMAC_DBG ("Transmit TUND error, TX reset\n");
            }
            else
            {
                GMAC_DBG("GMAC_INT_RTRY!!!!\n");
            }
        }

        if(intstatus&GMAC_INT_DONE)
        {
            LocPtr->ThisUVE.flagISR_INT_DONE = 0x01UL;
        }

        //RX Overrun //
        if(intstatus & GMAC_INT_ROVR)
        {
            GMAC_DBG("ROVR!!!!\n");
            LocPtr->stats.rx_dropped++;
            LocPtr->ROVR_count++;
            //write 1 clear
            MHal_GMAC_Write_RSR(GMAC_RSROVR);
#ifdef GMAC_ROVR_SW_RESET
            if (LocPtr->ROVR_count >= 6)
            {
                MDev_GMAC_SwReset(dev);
            }
#endif
        }
        else
        {
            LocPtr->ROVR_count = 0;
        }

        // Receive complete //
        if(delay_int_status & 0x8000UL)
        {
            //MDev_GMAC_TX_Free_sk_buff(dev, 0);
        #ifdef RX_NAPI
            /* Receive packets are processed by poll routine. If not running start it now. */
            if (napi_schedule_prep(&LocPtr->napi_rx)) {
                MDEV_GMAC_DISABLE_RX_REG();
                __napi_schedule(&LocPtr->napi_rx);
            }
        #else
            MDEV_GMAC_DISABLE_RX_REG();
            MDev_GMAC_rx(dev);
            MDEV_GMAC_ENABLE_RX_REG();
        #endif
        }
        //delay_int_status = MHal_GMAC_Read_Delay_interrupt_status();
    }
    spin_unlock_irqrestore(&LocPtr->irq_lock, flags);
    return IRQ_HANDLED;
}
#endif
//-------------------------------------------------------------------------------------------------
// GMAC Hardware register set
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_ScanPhyAddr(struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    unsigned char addr = 1; // because address 0 = broadcast, RTL8211E will reply to broadcast addr
    u32 value = 0;


    //MHal_GMAC_enable_mdi();
    do
    {
        value = 0;
        MHal_GMAC_read_phy(addr, MII_BMSR, &value);
        if (0 != value && 0x0000FFFFUL != value)
        {
            GMAC_DBG("[ PHY Addr ] ==> :%u BMSR = %04x\n", addr, value);
            if(LocPtr->padmux_type != GMAC1_EPHY) //Extenal phy
            {
                u32 value1 = 0,value2 = 0;
                MHal_GMAC_read_phy(addr, MII_PHYSID1, &value1);
                MHal_GMAC_read_phy(addr, MII_PHYSID2, &value2);
                GMAC_DBG("[ PHY ID ]: 0x%04x 0x%04x\n", (unsigned short)value1, (unsigned short)value2);
            }
            break;
        }
    }while(++addr && addr < 32);

    LocPtr->phyaddr = addr;

    if (LocPtr->phyaddr >= 32)
    {
        addr = 0;
        MHal_GMAC_read_phy(addr, MII_BMSR, &value);
        if (0 != value && 0x0000FFFFUL != value)
        {
            GMAC_DBG("[ PHY Addr ] ==> :%u BMSR = %08x\n", addr, value);
            LocPtr->phyaddr = 0;
        }
        else
        {
            GMAC_DBG("Wrong PHY Addr, maybe MoCA?\n");
            LocPtr->phyaddr = 32;
        }
    }

    //MHal_GMAC_disable_mdi();
    return 0;
}
/*
static void Rtl_Patch(struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 val;

    MHal_GMAC_read_phy(LocPtr->phyaddr, 25, &val);
    MHal_GMAC_write_phy(LocPtr->phyaddr, 25, 0x400UL);
    MHal_GMAC_read_phy(LocPtr->phyaddr, 25, &val);
}

static void MDev_GMAC_Patch_PHY(struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 val;

    MHal_GMAC_read_phy(LocPtr->phyaddr, 2, &val);
    if (GMAC_RTL_8210 == val)
        Rtl_Patch(dev);
}*/

static u32 MDev_GMAC_HW_Reg_init(struct net_device *dev)
{
    struct GMAC_private *LocPtr = (struct GMAC_private*) netdev_priv(dev);
    u32 word_ETH_CFG;
    u32 word_ETH_CTL;
    u32 config3_Value = 0;
    u32 config2_Value = 0;
    u32 uNegPhyVal = 0;
#ifdef HW_TX_CHECKSUM
    u32 config4_Value = 0;
#endif
#ifdef CONFIG_MSTAR_GMAC_JUMBO_PACKET
    u32 config5_Value = 0;
#endif

    if (!LocPtr->ThisUVE.initedGMAC)
    {
        MHal_GMAC_Power_On_Clk();
        if((LocPtr->padmux_type==GMAC0_RMII_EXT_EPHY) || (LocPtr->padmux_type==GMAC1_RMII_EXT_EPHY))
            config2_Value = CONFIG2_VAL|GMAC_RMII_12|GMAC_RMII;
        else
            config2_Value = CONFIG2_VAL;


        MHal_GMAC_Write_Network_config_register2(config2_Value);

        LocPtr->phyaddr = MHal_GMAC_PHY_ADDR();

        if(LocPtr->phyaddr==0)
        {
            if (MDev_GMAC_ScanPhyAddr(dev) < 0)
            return -1;
        }

        //MDev_GMAC_Patch_PHY(dev);
    }

#ifdef RX_DELAY_INTERRUPT
    config3_Value = DELAY_INTERRUPT_CONFIG;//0xFF050080;
#endif

#ifdef GMAC_RX_CHECKSUM
    config3_Value = config3_Value | GMAC_RX_CHECKSUM_ENABLE;
#endif

#ifdef RX_DESC_MODE
    config3_Value = config3_Value | GMAC_SOFTWARE_DESCRIPTOR_ENABLE;
#endif

#ifdef HW_TX_CHECKSUM
    dev->features |= NETIF_F_IP_CSUM;
    config3_Value = config3_Value | GMAC_TX_CHECKSUM_ENABLE;
#endif

    MHal_GMAC_Write_Network_config_register3(config3_Value);

#ifdef HW_TX_CHECKSUM
    dev->features |= NETIF_F_IPV6_CSUM;
    config4_Value = MHal_GMAC_Read_Network_config_register4() | GMAC_TX_V6_CHECKSUM_ENABLE;
    MHal_GMAC_Write_Network_config_register4(config4_Value);
#endif

#ifdef CONFIG_MSTAR_GMAC_JUMBO_PACKET
    config5_Value = MHal_GMAC_Read_Network_config_register5() | GMAC_TX_CHECKSUM_ENABLE;
    config5_Value &= 0xffff0000UL;
    config5_Value |= GMAC_TX_JUMBO_FRAME_ENABLE;
    MHal_GMAC_Write_Network_config_register5(config5_Value);
#endif

#ifdef NEW_TX_QUEUE
    MHal_GMAC_New_TX_QUEUE_Enable();
    MHal_GMAC_New_TX_QUEUE_Threshold_Set(TX_BUFF_ENTRY_NUMBER);
#endif /* NEW_TX_QUEUE */

    if(!(LocPtr->ep_flag & GMAC_EP_FLAG_SUSPENDING))
        MDev_GMAC_get_mac_address (dev);    // Get ethernet address and store it in dev->dev_addr //

    MDev_GMAC_update_mac_address (dev); // Program ethernet address into MAC //

    if (!LocPtr->ThisUVE.initedGMAC)
    {
        MHal_GMAC_write_phy(LocPtr->phyaddr, MII_BMCR, 0x1000UL);
        // IMPORTANT: Run NegotiationPHY() before writing REG_ETH_CFG.
        uNegPhyVal = MHal_GMAC_NegotiationPHY( LocPtr->phyaddr );
        if(uNegPhyVal == 0x01UL)
        {
            LocPtr->ThisUVE.flagMacTxPermit = 0x01UL;
            LocPtr->ThisBCE.duplex = 1;

        }
        else if(uNegPhyVal == 0x02UL)
        {
            LocPtr->ThisUVE.flagMacTxPermit = 0x01UL;
            LocPtr->ThisBCE.duplex = 2;
        }

        // ETH_CFG Register -----------------------------------------------------
        word_ETH_CFG = 0x00000C00UL;        // Init: CLK = 0x3
        // (20070808) IMPORTANT: REG_ETH_CFG:bit1(FD), 1:TX will halt running RX frame, 0:TX will not halt running RX frame.
        // If always set FD=0, no CRC error will occur. But throughput maybe need re-evaluate.
        // IMPORTANT: (20070809) NO_MANUAL_SET_DUPLEX : The real duplex is returned by "negotiation"
        if(LocPtr->ThisBCE.speed     == GMAC_SPEED_100) word_ETH_CFG |= 0x00000001UL;
        if(LocPtr->ThisBCE.duplex    == 2)              word_ETH_CFG |= 0x00000002UL;
        if(LocPtr->ThisBCE.cam       == 1)              word_ETH_CFG |= 0x00000200UL;
        if(LocPtr->ThisBCE.rcv_bcast == 0)              word_ETH_CFG |= 0x00000020UL;
        if(LocPtr->ThisBCE.rlf       == 1)              word_ETH_CFG |= 0x00000100UL;

        MHal_GMAC_Write_CFG(word_ETH_CFG);
        // ETH_CTL Register -----------------------------------------------------

        if(LocPtr->ThisBCE.wes == 1)
        {
            word_ETH_CTL = MHal_GMAC_Read_CTL();
            word_ETH_CTL |= 0x00000080UL;
            MHal_GMAC_Write_CTL(word_ETH_CTL);
        }

        LocPtr->ThisUVE.flagPowerOn = 1;
        LocPtr->ThisUVE.initedGMAC  = 1;
    }

    MHal_GMAC_HW_init();
    return 0;
}


//-------------------------------------------------------------------------------------------------
// GMAC init Variable
//-------------------------------------------------------------------------------------------------
extern phys_addr_t memblock_start_of_DRAM(void);
extern phys_addr_t memblock_size_of_first_region(void);

static phys_addr_t MDev_GMAC_MemInit(struct net_device *dev)
{
    struct GMAC_private *LocPtr =(struct GMAC_private *) netdev_priv(dev);
    phys_addr_t alloRAM_PA_BASE;
    phys_addr_t alloRAM_SIZE;
    phys_addr_t *alloRAM_VA_BASE;
    MSYS_DMEM_INFO mem_info;
    int ret=0;

/*    get_boot_mem_info(GMAC_MEM, &alloRAM_PA_BASE, &alloRAM_SIZE);

#if defined (CONFIG_ARM64)
    // get gmac addr only from mboot
    //alloRAM_PA_BASE = memblock_start_of_DRAM() + memblock_size_of_first_region();
#endif
    alloRAM_VA_BASE = (phys_addr_t *)ioremap(alloRAM_PA_BASE, alloRAM_SIZE); //map buncing buffer from PA to VA
*/
    //alloRAM_SIZE = (sizeof(struct GMAC_private)+ 0x3FFFUL) & ~0x3FFFUL;
    alloRAM_SIZE = 0;
    alloRAM_SIZE += RX_DESC_TABLE_SIZE;
#ifndef RX_ZERO_COPY
	alloRAM_SIZE += RX_BUFF_SIZE;
#endif
#ifdef TX_DESC_MODE
	alloRAM_SIZE += TX_LOW_PRI_DESC_TABLE_SIZE;
#else
	alloRAM_SIZE += TX_BUFF_ENTRY_NUMBER*TX_BUFF_ENTRY_SIZE;
#endif


    mem_info.length = alloRAM_SIZE;
    strcpy(mem_info.name, "GMAC_BUFF");
    if((ret=msys_request_dmem(&mem_info)))
    {
        panic("unable to locate DMEM for EMAC alloRAM!! error=%d\n",ret);
    }
    alloRAM_PA_BASE = mem_info.phys;
    alloRAM_VA_BASE = (phys_addr_t *)((size_t)mem_info.kvirt);

    //GMAC_DBG("alloRAM_VA_BASE = 0x%zx\n", (size_t)alloRAM_VA_BASE);
    //GMAC_DBG("alloRAM_PA_BASE= 0x%zx\n", (size_t)alloRAM_PA_BASE);
    //GMAC_DBG("alloRAM_SIZE= 0x%zx\n", (size_t)alloRAM_SIZE);
    BUG_ON(!alloRAM_VA_BASE);

    memset((phys_addr_t *)alloRAM_VA_BASE,0x00UL,alloRAM_SIZE);

    //LocPtr->RAM_VA_BASE       = ((phys_addr_t)alloRAM_VA_BASE + sizeof(struct GMAC_private) + 0x3FFFUL) & ~0x3FFFUL;   // IMPORTANT: Let lowest 14 bits as zero.
    //LocPtr->RAM_PA_BASE       = ((phys_addr_t)alloRAM_PA_BASE + sizeof(struct GMAC_private) + 0x3FFFUL) & ~0x3FFFUL;   // IMPORTANT: Let lowest 14 bits as zero.
    LocPtr->RAM_VA_BASE       = (phys_addr_t)alloRAM_VA_BASE;
    LocPtr->RAM_PA_BASE       = (phys_addr_t)alloRAM_PA_BASE;
    LocPtr->RAM_VA_PA_OFFSET  = LocPtr->RAM_VA_BASE - LocPtr->RAM_PA_BASE;  // IMPORTANT_TRICK_20070512
    LocPtr->RX_DESC_BASE      = LocPtr->RAM_PA_BASE;
#ifndef RX_ZERO_COPY
    LocPtr->RX_BUFFER_BASE    = LocPtr->RAM_PA_BASE + RX_DESC_TABLE_SIZE;
    #ifdef TX_DESC_MODE
    LocPtr->TX_LP_DESC_BASE   = ((LocPtr->RAM_PA_BASE + RX_DESC_TABLE_SIZE + RX_BUFF_SIZE) + 0x0FFFUL) & ~0x0FFFUL;
    LocPtr->TX_BUFFER_BASE    = LocPtr->TX_LP_DESC_BASE + TX_LOW_PRI_DESC_TABLE_SIZE;
    #else
    LocPtr->TX_BUFFER_BASE    = LocPtr->RAM_PA_BASE + (RX_DESC_TABLE_SIZE + RX_BUFF_SIZE);
    #endif
#else
    #ifdef TX_DESC_MODE
    LocPtr->TX_LP_DESC_BASE   = ((LocPtr->RAM_PA_BASE + RX_DESC_TABLE_SIZE) + 0x0FFFUL) & ~0x0FFFUL;
    LocPtr->TX_BUFFER_BASE    = LocPtr->TX_LP_DESC_BASE + TX_LOW_PRI_DESC_TABLE_SIZE;
    #else
    LocPtr->TX_BUFFER_BASE    = LocPtr->RAM_PA_BASE + RX_DESC_TABLE_SIZE;
    #endif
#endif

    //GMAC_DBG("RAM_VA_BASE       = 0x%zx\n", (size_t)LocPtr->RAM_VA_BASE);
    //GMAC_DBG("RAM_PA_BASE       = 0x%zx\n", (size_t)LocPtr->RAM_PA_BASE);
    //GMAC_DBG("RAM_VA_PA_OFFSET  = 0x%zx\n", (size_t)LocPtr->RAM_VA_PA_OFFSET);
    GMAC_DBG("RX_DESC_BASE      = 0x%zx, size = 0x%zx\n", (size_t)LocPtr->RX_DESC_BASE, (size_t)RX_DESC_TABLE_SIZE);
#ifndef RX_ZERO_COPY
    GMAC_DBG("RX_BUFFER_BASE    = 0x%zx, size = 0x%zx\n", (size_t)LocPtr->RX_BUFFER_BASE, (size_t)RX_BUFF_SIZE);
#endif
#ifdef TX_DESC_MODE
    GMAC_DBG("TX_LP_DESC_BASE   = 0x%zx, size = 0x%zx\n", (size_t)LocPtr->TX_LP_DESC_BASE, (size_t)TX_LOW_PRI_DESC_TABLE_SIZE);
#endif
    GMAC_DBG("TX_BUFFER_BASE    = 0x%zx, size = 0x%zx\n", (size_t)LocPtr->TX_BUFFER_BASE, (size_t)(TX_BUFF_ENTRY_NUMBER*TX_BUFF_ENTRY_SIZE));

#ifdef RX_DESC_MODE
    LocPtr->rx_desc_list = (struct rx_descriptor *)(LocPtr->RX_DESC_BASE + LocPtr->RAM_VA_PA_OFFSET);
#endif

#ifdef TX_DESC_MODE
    LocPtr->tx_desc_list = (struct tx_descriptor *)(LocPtr->TX_LP_DESC_BASE + LocPtr->RAM_VA_PA_OFFSET);
#endif

#ifndef RX_ZERO_COPY
    MDev_GMAC_RX_DESC_Init_memcpy(dev);
#endif

    memset(&LocPtr->ThisBCE,0x00UL,sizeof(BasicConfigGMAC));
    memset(&LocPtr->ThisUVE,0x00UL,sizeof(UtilityVarsGMAC));

    LocPtr->ThisBCE.wes         = 0;                    // 0:Disable, 1:Enable (WR_ENABLE_STATISTICS_REGS)
    LocPtr->ThisBCE.duplex      = 2;                    // 1:Half-duplex, 2:Full-duplex
    LocPtr->ThisBCE.cam         = 0;                    // 0:No CAM, 1:Yes
    LocPtr->ThisBCE.rlf         = 0;                    // 0:No, 1:Yes receive long frame(1522)
    LocPtr->ThisBCE.rcv_bcast   = 1;
    LocPtr->ThisBCE.speed       = GMAC_SPEED_100;
    LocPtr->ThisBCE.connected   = 0;
    return (phys_addr_t)alloRAM_VA_BASE;
}

//-------------------------------------------------------------------------------------------------
// Initialize the ethernet interface
// @return TRUE : Yes
// @return FALSE : FALSE
//-------------------------------------------------------------------------------------------------
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
static const struct net_device_ops mstar_lan_netdev_ops = {
        .ndo_open               = MDev_GMAC_open,
        .ndo_stop               = MDev_GMAC_close,
        .ndo_start_xmit         = MDev_GMAC_tx,
        .ndo_set_mac_address    = MDev_GMAC_set_mac_address,
        .ndo_set_rx_mode        = MDev_GMAC_set_rx_mode,
        .ndo_do_ioctl           = MDev_GMAC_ioctl,
        .ndo_get_stats          = MDev_GMAC_stats,
};

static int MDev_GMAC_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
    struct GMAC_private *LocPtr =(struct GMAC_private *) netdev_priv(dev);
#ifdef CONFIG_GMAC_EPHY_LPA_FORCE_SPEED
    u32 bmsr, bmcr, lpa, adv, neg;
    u32 lpa1000, ctrl1000;
#endif

    mii_ethtool_gset (&LocPtr->mii, cmd);

#ifdef CONFIG_GMAC_EPHY_LPA_FORCE_SPEED
    if (cmd->autoneg == AUTONEG_ENABLE) {
        MHal_GMAC_read_phy(LocPtr->phyaddr, MII_BMSR, &bmsr);
        MHal_GMAC_read_phy(LocPtr->phyaddr, MII_BMSR, &bmsr);
        if (bmsr & BMSR_ANEGCOMPLETE) {
            /* For Link Parterner adopts force mode and EPHY used,
             * EPHY LPA reveals all zero value.
             * EPHY would be forced to Full-Duplex mode.
             */
            if (cmd->lp_advertising == 0) {
                MHal_GMAC_read_phy(LocPtr->phyaddr, MII_BMCR, &bmcr);

                if(LocPtr->hardware_type == GMAC_GPHY)
                {
                    if (bmcr & BMCR_SPEED1000)
                        lpa1000 = LPA_1000FULL;

                    if (bmcr & BMCR_SPEED100)
                        lpa = LPA_100FULL;
                    else
                        lpa = LPA_10FULL;

                    MHal_GMAC_read_phy(LocPtr->phyaddr, MII_ADVERTISE, &adv);
                    neg = adv & lpa;

                    MHal_GMAC_read_phy(LocPtr->phyaddr, MII_CTRL1000, &ctrl1000);

                    if((lpa1000 & LPA_1000FULL) && (ctrl1000 & ADVERTISE_1000FULL))
                    {
                        ethtool_cmd_speed_set(cmd, SPEED_1000);
                    }
                    else
                    {
                        if (neg & LPA_100FULL)
                        {
                            ethtool_cmd_speed_set(cmd, SPEED_100);
                        }
                        else
                        {
                            ethtool_cmd_speed_set(cmd, SPEED_10);
                        }
                    }
                }
                else
                {
                    if (bmcr & BMCR_SPEED100)
                        lpa = LPA_100FULL;
                    else
                        lpa = LPA_10FULL;

                    MHal_GMAC_read_phy(LocPtr->phyaddr, MII_ADVERTISE, &adv);
                    neg = adv & lpa;

                    if (neg & LPA_100FULL)
                    {
                        ethtool_cmd_speed_set(cmd, SPEED_100);
                    }
                    else
                    {
                        ethtool_cmd_speed_set(cmd, SPEED_10);
                    }
                }
                cmd->duplex = 0x01;
                LocPtr->mii.full_duplex = cmd->duplex;
            }
        }
    }
#endif /* CONFIG_GMAC_EPHY_LPA_FORCE_SPEED */

    return 0;
}

static int MDev_GMAC_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
    struct GMAC_private *LocPtr =(struct GMAC_private *) netdev_priv(dev);

    mii_ethtool_sset (&LocPtr->mii, cmd);

    return 0;
}

static int MDev_GMAC_nway_reset(struct net_device *dev)
{
    struct GMAC_private *LocPtr =(struct GMAC_private *) netdev_priv(dev);

    mii_nway_restart (&LocPtr->mii);

    return 0;
}

static u32 MDev_GMAC_get_link(struct net_device *dev)
{
    u32	u32data;
    struct GMAC_private *LocPtr =(struct GMAC_private *) netdev_priv(dev);

    u32data = mii_link_ok (&LocPtr->mii);

    return u32data;
}

static void MDev_GMAC_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
    strlcpy(info->driver, GMAC_DRVNAME, sizeof(info->driver));
    strlcpy(info->version, GMAC_DRV_VERSION, sizeof(info->version));

    return;
}

#define GMAC_STATS_STRING_LEN 23

static char mstar_gmac_string[GMAC_STATS_STRING_LEN][ETH_GSTRING_LEN]=
{
    {"rx_packets"},
    {"rx_bytes"},
    {"rx_errors"},
    {"rx_dropped"},
    {"rx_length_errors"},
    {"rx_over_errors"},
    {"rx_crc_errors"},
    {"rx_frame_errors"},
    {"rx_fifo_errors"},
    {"rx_missed_errors"},
    {"rx_compressed"},
    {"tx_packets"},
    {"tx_bytes"},
    {"tx_errors"},
    {"tx_dropped"},
    {"tx_aborted_errors"},
    {"tx_carrier_errors"},
    {"tx_fifo_errors"},
    {"tx_heartbeat_errors"},
    {"tx_window_errors"},
    {"tx_compressed"},
    {"multicast"},
    {"collisions"}
};

static u64 *mstar_gmac_stats[GMAC_STATS_STRING_LEN]; //Read only, don't write

static void MDev_GMAC_ethtool_stats_init(struct net_device *dev)
{
    struct GMAC_private *LocPtr =(struct GMAC_private *) netdev_priv(dev);

    mstar_gmac_stats[0] = (u64 *)&LocPtr->stats.rx_packets;
    mstar_gmac_stats[1] = (u64 *)&LocPtr->stats.rx_bytes;
    mstar_gmac_stats[2] = (u64 *)&LocPtr->stats.rx_errors;
    mstar_gmac_stats[3] = (u64 *)&LocPtr->stats.rx_dropped;
    mstar_gmac_stats[4] = (u64 *)&LocPtr->stats.rx_length_errors;
    mstar_gmac_stats[5] = (u64 *)&LocPtr->stats.rx_over_errors;
    mstar_gmac_stats[6] = (u64 *)&LocPtr->stats.rx_crc_errors;
    mstar_gmac_stats[7] = (u64 *)&LocPtr->stats.rx_frame_errors;
    mstar_gmac_stats[8] = (u64 *)&LocPtr->stats.rx_fifo_errors;
    mstar_gmac_stats[9] = (u64 *)&LocPtr->stats.rx_missed_errors;
    mstar_gmac_stats[10] = (u64 *)&LocPtr->stats.rx_compressed;
    mstar_gmac_stats[11] = (u64 *)&LocPtr->stats.tx_packets;
    mstar_gmac_stats[12] = (u64 *)&LocPtr->stats.tx_bytes;
    mstar_gmac_stats[13] = (u64 *)&LocPtr->stats.tx_errors;
    mstar_gmac_stats[14] = (u64 *)&LocPtr->stats.tx_dropped;
    mstar_gmac_stats[15] = (u64 *)&LocPtr->stats.tx_aborted_errors;
    mstar_gmac_stats[16] = (u64 *)&LocPtr->stats.tx_carrier_errors;
    mstar_gmac_stats[17] = (u64 *)&LocPtr->stats.tx_fifo_errors;
    mstar_gmac_stats[18] = (u64 *)&LocPtr->stats.tx_heartbeat_errors;
    mstar_gmac_stats[19] = (u64 *)&LocPtr->stats.tx_window_errors;
    mstar_gmac_stats[20] = (u64 *)&LocPtr->stats.tx_compressed;
    mstar_gmac_stats[21] = (u64 *)&LocPtr->stats.multicast;
    mstar_gmac_stats[22] = (u64 *)&LocPtr->stats.collisions;

}

static void MDev_GMAC_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
    char *p = (char *)data;
    unsigned int i;

    for (i = 0; i < GMAC_STATS_STRING_LEN; i++)
    {
        sprintf(p, mstar_gmac_string[i]);
        p += ETH_GSTRING_LEN;
    }

}

static void MDev_GMAC_get_ethtool_stats(struct net_device *dev, struct ethtool_stats *e_state, u64 *data)
{
    unsigned int i;
    //GMAC_DBG("MDev_GMAC_get_ethtool_stats\n");

    MDev_GMAC_stats(dev);

    for(i = 0; i < GMAC_STATS_STRING_LEN; i++)
    {
        data[i] = *(u32 *)mstar_gmac_stats[i];
    }
}

static int MDev_GMAC_sset_count(struct net_device *dev, int type)
{
    //GMAC_DBG("MDev_GMAC_sset_count\n");
    switch(type)
    {
        case ETH_SS_TEST:
            return GMAC_TEST_STRING_LEN;
        case ETH_SS_STATS:
            return GMAC_STATS_STRING_LEN;
        case ETH_SS_PRIV_FLAGS:
            return GMAC_PRIV_FLAGS_STRING_LEN;
        default:
            return -EOPNOTSUPP;
    }
}

#define GMAC_REGS_LEN 0xC0

static int MDev_GMAC_get_regs_len(struct net_device *dev)
{
    return GMAC_REGS_LEN * sizeof(u32);
}

static u32 MDev_GMAC_get_regs_transform(u32 reg_data)
{
    return ((reg_data & 0x000000FF) << 8) |  ((reg_data & 0x0000FF00) >> 8) | ((reg_data & 0x00FF0000) << 8) | ((reg_data & 0xFF000000) >> 8);
}

static void MDev_GMAC_get_regs(struct net_device *dev, struct ethtool_regs *regs, void *data)
{
    u32 *data_32 = data;
    u32 register_index;
    u32 address_counter;

    register_index = 0;
    address_counter = 0;

    while(register_index < 0x40)
    {
        data_32[register_index] = MDev_GMAC_get_regs_transform(MHal_GMAC_ReadReg32(0x4UL * address_counter));
        register_index++;
        address_counter++;
    }

    address_counter = 0;

    while(register_index < 0x80)
    {
        data_32[register_index] = MDev_GMAC_get_regs_transform(MHal_GMAC_ReadReg32(0x4UL * address_counter + 0x100UL));
        register_index++;
        address_counter++;
    }

    address_counter = 0;

    while(register_index < 0xC0)
    {
        data_32[register_index] = MDev_GMAC_get_regs_transform(MHal_GMAC_ReadReg32(0x4UL * address_counter + 0x400UL));
        register_index++;
        address_counter++;
    }

}

static int MDev_GMAC_get_coalesce(struct net_device *dev, struct ethtool_coalesce *e_coalesce)
{
#ifdef RX_NAPI
    struct GMAC_private *LocPtr =(struct GMAC_private *) netdev_priv(dev);
#endif
    u32 register_config;

    register_config = MHal_GMAC_Read_Network_config_register3();

    e_coalesce->rx_coalesce_usecs           = (register_config & (~GMAC_INT_DELAY_TIME_MASK)) >> GMAC_INT_DELAY_TIME_SHIFT;
    e_coalesce->rx_max_coalesced_frames     = (register_config & (~GMAC_INT_DELAY_NUMBER_MASK)) >> GMAC_INT_DELAY_NUMBER_SHIFT;
#ifdef RX_NAPI
    e_coalesce->rx_max_coalesced_frames_irq = LocPtr->napi_rx.weight;
#endif
    e_coalesce->use_adaptive_rx_coalesce    = true;

    return 0;
}

static int MDev_GMAC_set_coalesce(struct net_device *dev, struct ethtool_coalesce *e_coalesce)
{
#ifdef RX_NAPI
    struct GMAC_private *LocPtr =(struct GMAC_private *) netdev_priv(dev);
#endif
    u32 register_config;

    if(e_coalesce->rx_coalesce_usecs > 0xFF)
        return -EINVAL;
    if(e_coalesce->rx_max_coalesced_frames > 0xFF)
        return -EINVAL;
    if(e_coalesce->rx_max_coalesced_frames_irq > 0x40)
        return -EINVAL;

    register_config = MHal_GMAC_Read_Network_config_register3();
    register_config = register_config & GMAC_INT_DELAY_TIME_MASK & GMAC_INT_DELAY_NUMBER_MASK;
    register_config = register_config | e_coalesce->rx_coalesce_usecs << GMAC_INT_DELAY_TIME_SHIFT;
    register_config = register_config | e_coalesce->rx_max_coalesced_frames << GMAC_INT_DELAY_NUMBER_SHIFT;
    MHal_GMAC_Write_Network_config_register3(register_config);

#ifdef RX_NAPI
    LocPtr->napi_rx.weight = e_coalesce->rx_max_coalesced_frames_irq;
#endif

    return 0;
}

static void MDev_GMAC_get_ringparam(struct net_device *dev, struct ethtool_ringparam *e_ringparam)
{
    struct GMAC_private *LocPtr =(struct GMAC_private *) netdev_priv(dev);

    e_ringparam->rx_max_pending = RX_DESC_NUMBER;
    e_ringparam->tx_max_pending = TX_BUFF_ENTRY_NUMBER;
    e_ringparam->rx_pending = LocPtr->rx_ring_entry_number;
    e_ringparam->tx_pending = LocPtr->tx_ring_entry_number;

    return;
}

//static int MDev_GMAC_set_ringparam(struct net_device *dev, struct ethtool_ringparam *e_ringparam)
//{
//    struct GMAC_private *LocPtr =(struct GMAC_private *) netdev_priv(dev);
//
//    if(e_ringparam->rx_pending > RX_DESC_NUMBER)
//        return -EINVAL;
//    if(e_ringparam->tx_pending > TX_BUFF_ENTRY_NUMBER)
//        return -EINVAL;
//
//    LocPtr->rx_ring_entry_number = e_ringparam->rx_pending;
//    LocPtr->tx_ring_entry_number = e_ringparam->tx_pending;
//
//    return 0;
//}

static u32 MDev_GMAC_get_msglevel(struct net_device *dev)
{
    struct GMAC_private *LocPtr =(struct GMAC_private *) netdev_priv(dev);
    return LocPtr->msglvl;
}

static void MDev_GMAC_set_msglevel(struct net_device *dev, u32 data)
{
    struct GMAC_private *LocPtr =(struct GMAC_private *) netdev_priv(dev);
    LocPtr->msglvl = data;
    return;
}

static const struct ethtool_ops mstar_ethtool_ops = {
    .get_settings       = MDev_GMAC_get_settings,
    .set_settings       = MDev_GMAC_set_settings,
    .nway_reset         = MDev_GMAC_nway_reset,
    .get_link           = MDev_GMAC_get_link,
    .get_drvinfo        = MDev_GMAC_get_drvinfo,
    .get_sset_count     = MDev_GMAC_sset_count,
    .get_ethtool_stats  = MDev_GMAC_get_ethtool_stats,
    .get_strings        = MDev_GMAC_get_strings,
    .get_regs           = MDev_GMAC_get_regs,
    .get_regs_len       = MDev_GMAC_get_regs_len,
    .get_coalesce       = MDev_GMAC_get_coalesce,
    .set_coalesce       = MDev_GMAC_set_coalesce,
    .get_ringparam      = MDev_GMAC_get_ringparam,
    //.set_ringparam      = MDev_GMAC_set_ringparam,
    .get_msglevel       = MDev_GMAC_get_msglevel,
    .set_msglevel       = MDev_GMAC_set_msglevel,
};
#endif
//-------------------------------------------------------------------------------------------------
// Restar the ethernet interface
// @return TRUE : Yes
// @return FALSE : FALSE
//-------------------------------------------------------------------------------------------------
static int MDev_GMAC_SwReset(struct net_device *dev)
{
#if 0
    struct GMAC_private *LocPtr =(struct GMAC_private *) netdev_priv(dev);
    u32 oldCFG, oldCTL;
    u32 retval;

    MDev_GMAC_get_mac_address (dev);
    oldCFG = MHal_GMAC_Read_CFG();
    oldCTL = MHal_GMAC_Read_CTL() & ~(GMAC_TE | GMAC_RE);

    netif_stop_queue (dev);

    retval = MHal_GMAC_Read_Network_config_register2();
    MHal_GMAC_Write_Network_config_register2(retval & 0x00000FFFUL);
    MHal_GMAC_Write_Network_config_register2(retval);

    MDev_GMAC_HW_Reg_init(dev);
    MHal_GMAC_Write_CFG(oldCFG);
    MHal_GMAC_Write_CTL(oldCTL);
    MHal_GMAC_enable_mdi ();
    MDev_GMAC_update_mac_address (dev); // Program ethernet address into MAC //
    MDev_GMAC_update_linkspeed (dev);
    MHal_GMAC_Write_IER(GMAC_INT_ENABLE);
    MDev_GMAC_start (dev);
    MDev_GMAC_set_rx_mode(dev);
    netif_start_queue (dev);
    LocPtr->ROVR_count = 0;
#ifdef HW_TX_CHECKSUM
    retval = MHal_GMAC_Read_Network_config_register3() | GMAC_TX_CHECKSUM_ENABLE;
    MHal_GMAC_Write_Network_config_register3(retval);
    dev->features |= NETIF_F_IP_CSUM;
#endif

#else
    /* only trigger hw restart */
    //struct GMAC_private *LocPtr =(struct GMAC_private *) netdev_priv(dev);
    u32 uRegVal;

    /* Disable Receiver and Transmitter */
    uRegVal = MHal_GMAC_Read_CTL();
    uRegVal &= ~(GMAC_TE | GMAC_RE);
    MHal_GMAC_Write_CTL(uRegVal);

    /* Enable Receive and Transmit */
    uRegVal = MHal_GMAC_Read_CTL();
    uRegVal |= (GMAC_RE | GMAC_TE);
    MHal_GMAC_Write_CTL(uRegVal);
#endif

    return 0;
}

#if defined (CONFIG_OF)
static struct of_device_id mstargmac_of_device_ids[] = {
    {.compatible = "mstar-gmac"},
    {},
};
#endif

//-------------------------------------------------------------------------------------------------
// Detect MAC and PHY and perform initialization
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// GMAC Timer set for Receive function
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_timer_callback(unsigned long value)
{
    int ret = 0;
    struct GMAC_private *LocPtr = (struct GMAC_private *) netdev_priv(gmac_dev);
    static u32 /*bmsr,*/ time_count = 0;
    // Normally, time out is set 1 Sec.
    LocPtr->Link_timer.expires = jiffies + GMAC_CHECK_LINK_TIME;

#ifndef RX_DELAY_INTERRUPT
    if (GMAC_RX_TMR == value)
    {
        MHal_GMAC_timer_callback(value);
        return;
    }
#endif

    spin_lock_irq (&LocPtr->irq_lock);
    ret = MDev_GMAC_update_linkspeed(gmac_dev);
    spin_unlock_irq (&LocPtr->irq_lock);
    if (0 == ret)
    {
        if (!LocPtr->ThisBCE.connected)
        {
            #ifndef RX_ZERO_COPY
            MDev_GMAC_RX_DESC_Reset_memcpy(gmac_dev);
            #endif
            MDEV_GMAC_ENABLE_RX_REG();
            LocPtr->ThisBCE.connected = 1;
            netif_carrier_on(gmac_dev);
        }
        /*
        // Link status is latched, so read twice to get current value //
        MHal_GMAC_read_phy (LocPtr->phyaddr, MII_BMSR, &bmsr);
        MHal_GMAC_read_phy (LocPtr->phyaddr, MII_BMSR, &bmsr);
        time_count = 0;
        spin_lock_irq (&LocPtr->irq_lock);
        LocPtr->phy_status_register = bmsr;
        spin_unlock_irq (&LocPtr->irq_lock);
        */
    }
    else    //no link
    {
        if(LocPtr->ThisBCE.connected) {
            LocPtr->ThisBCE.connected = 0;
            netif_carrier_off(gmac_dev);
        }
        // If disconnected is over 3 Sec, the real value of PHY's status register will report to application.
        if(time_count > 30) {
            /*
            // Link status is latched, so read twice to get current value //
            MHal_GMAC_read_phy (LocPtr->phyaddr, MII_BMSR, &bmsr);
            MHal_GMAC_read_phy (LocPtr->phyaddr, MII_BMSR, &bmsr);
            spin_lock_irq (&LocPtr->irq_lock);
            LocPtr->phy_status_register = bmsr;
            spin_unlock_irq (&LocPtr->irq_lock);
            */
            // Report to kernel.
            netif_carrier_off(gmac_dev);
            LocPtr->ThisBCE.connected = 0;
            MDEV_GMAC_DISABLE_RX_REG();
        }
        else if(time_count <= 30){
            time_count++;
            // Time out is set 100ms. Quickly checks next phy status.
            LocPtr->Link_timer.expires = jiffies + (GMAC_CHECK_LINK_TIME / 10);
        }
    }

    add_timer(&LocPtr->Link_timer);
}

//-------------------------------------------------------------------------------------------------
// GMAC MACADDR Setup
//-------------------------------------------------------------------------------------------------

#define MACADDR_FORMAT "XX:XX:XX:XX:XX:XX"
#ifdef CONFIG_MS_GMAC //be used to built-in
static int __init macaddr_auto_config_setup(char *addrs)
{
    if (strlen(addrs) == strlen(MACADDR_FORMAT)
        && ':' == addrs[2]
        && ':' == addrs[5]
        && ':' == addrs[8]
        && ':' == addrs[11]
        && ':' == addrs[14]
       )
    {
        addrs[2]  = '\0';
        addrs[5]  = '\0';
        addrs[8]  = '\0';
        addrs[11] = '\0';
        addrs[14] = '\0';

        GMAC_MY_MAC[0] = (u8)simple_strtoul(&(addrs[0]),  NULL, 16);
        GMAC_MY_MAC[1] = (u8)simple_strtoul(&(addrs[3]),  NULL, 16);
        GMAC_MY_MAC[2] = (u8)simple_strtoul(&(addrs[6]),  NULL, 16);
        GMAC_MY_MAC[3] = (u8)simple_strtoul(&(addrs[9]),  NULL, 16);
        GMAC_MY_MAC[4] = (u8)simple_strtoul(&(addrs[12]), NULL, 16);
        GMAC_MY_MAC[5] = (u8)simple_strtoul(&(addrs[15]), NULL, 16);

        /* set back to ':' or the environment variable would be destoried */ // REVIEW: this coding style is dangerous
        addrs[2]  = ':';
        addrs[5]  = ':';
        addrs[8]  = ':';
        addrs[11] = ':';
        addrs[14] = ':';
    }

    return 1;
}

__setup("macaddr=", macaddr_auto_config_setup);
#endif
//-------------------------------------------------------------------------------------------------
// GMAC init module
//-------------------------------------------------------------------------------------------------
int GMAC_PAD_MODE;
int gIrq_from_dts=0;

static int MDev_GMAC_init(void)
{
    struct GMAC_private *LocPtr;
#ifdef RX_ZERO_COPY
    GMAC_DBG("RX_ZERO_COPY!!!\n");
#endif
#ifdef RX_NAPI
    GMAC_DBG("RX_NAPI!!!RX_NAPI_WEIGHT = %d\n", GMAC_RX_NAPI_WEIGHT);
#endif
#ifdef RX_GRO
    GMAC_DBG("RX_GRO!!!\n");
#endif
#ifdef RX_DELAY_INTERRUPT
    GMAC_DBG("DELAY_NUMBER = %d, DELAY_TIME = %d\n", DELAY_NUMBER, DELAY_TIME);
#endif


    if(gmac_dev)
        return -1;

    gmac_dev = alloc_etherdev(sizeof(*LocPtr));
    LocPtr = netdev_priv(gmac_dev);

    if (gmac_dev == NULL)
    {
        GMAC_DBG( KERN_ERR "No GMAC dev mem!\n" );
        return -ENOMEM;
    }

    if (LocPtr == NULL)
    {
        GMAC_DBG("LocPtr fail\n");
        return -ENOMEM;
    }

#ifdef RX_NAPI
    netif_napi_add(gmac_dev, &LocPtr->napi_rx, MDev_GMAC_RX_napi_poll, GMAC_RX_NAPI_WEIGHT);
#endif

#ifdef TX_NAPI
    netif_napi_add(gmac_dev, &LocPtr->napi_tx, MDev_GMAC_TX_napi_poll, GMAC_TX_NAPI_WEIGHT);
#endif

    if(GMAC_PAD_MODE != 0)
    {
        GMAC_DBG("GMAC_PAD_MODE = %d\n", GMAC_PAD_MODE);
        LocPtr->padmux_type = GMAC_PAD_MODE;
    }
    else
    {
        GMAC_DBG("GMAC_PAD_MODE not set DEFAULT_PADMUX = %u\n", DEFAULT_PADMUX);
        LocPtr->padmux_type = DEFAULT_PADMUX;
    }

    LocPtr->hardware_type = MHal_GMAC_Hardware_check(LocPtr->padmux_type);
    LocPtr->dev                 = gmac_dev;
    LocPtr->phy_status_register = 0x78c9UL;         // 0x78c9: link is down.
    LocPtr->initstate           = 0;
    LocPtr->phyaddr             = 0;
    spin_lock_init (&LocPtr->irq_lock);
    spin_lock_init (&LocPtr->tx_lock);
    ether_setup (gmac_dev);
    gmac_dev->base_addr     = (long) MHal_GMAC_REG_ADDR_BASE();
#if 0 //fdef CONFIG_OF
    gmac_dev->irq           = gIrq_from_dts;
#else
    gmac_dev->irq           = MHal_GMAC_IRQ();
#endif
    gmac_dev->tx_queue_len  = GMAC_MAX_TX_QUEUE;
    gmac_dev->netdev_ops    = &mstar_lan_netdev_ops;

    if( MDev_GMAC_MemInit(gmac_dev) == 0)
    {
        GMAC_DBG("Memery init fail!!\n");
        goto end;
    }

    LocPtr->rx_ring_entry_number = RX_DESC_NUMBER;
    LocPtr->tx_ring_entry_number = TX_BUFF_ENTRY_NUMBER;

    if ( MDev_GMAC_HW_Reg_init(gmac_dev) < 0)
    {
        GMAC_DBG("Hardware Register init fail!!\n");
        goto end;
    }

    //Support for ethtool //
    LocPtr->mii.dev = gmac_dev;
    LocPtr->mii.mdio_read = MDev_GMAC_mdio_read;
    LocPtr->mii.mdio_write = MDev_GMAC_mdio_write;
    LocPtr->mii.phy_id = LocPtr->phyaddr;
    if(LocPtr->hardware_type == GMAC_GPHY)
    {
        LocPtr->mii.supports_gmii = true;
    }
    gmac_dev->ethtool_ops = &mstar_ethtool_ops;
    MDev_GMAC_ethtool_stats_init(gmac_dev);

    //Install the interrupt handler //
    //Notes: Modify linux/kernel/irq/manage.c  /* interrupt.h */
    if (request_irq(gmac_dev->irq, MDev_GMAC_interrupt, 0, gmac_dev->name, gmac_dev))
        goto end;

#if defined(CONFIG_MP_PLATFORM_GIC_SET_MULTIPLE_CPUS) && defined(CONFIG_MP_PLATFORM_INT_1_to_1_SPI)
    irq_set_affinity_hint(gmac_dev->irq, cpu_online_mask);
    irq_set_affinity(gmac_dev->irq, cpu_online_mask);
#endif

    //Determine current link speed //
    spin_lock_irq (&LocPtr->irq_lock);
    (void) MDev_GMAC_update_linkspeed (gmac_dev);
    spin_unlock_irq (&LocPtr->irq_lock);

    init_timer(&LocPtr->Link_timer);
    LocPtr->Link_timer.data = GMAC_LINK_TMR;
    LocPtr->Link_timer.function = MDev_GMAC_timer_callback;
    LocPtr->Link_timer.expires = jiffies + GMAC_CHECK_LINK_TIME;

    return register_netdev (gmac_dev);

end:
    free_netdev(gmac_dev);
    gmac_dev = 0;
    LocPtr->initstate = GMAC_ETHERNET_TEST_INIT_FAIL;
    GMAC_DBG( KERN_ERR "Init GMAC error!\n" );
    return -ENOSYS;
}
//-------------------------------------------------------------------------------------------------
// GMAC exit module
//-------------------------------------------------------------------------------------------------
static void MDev_GMAC_exit(void)
{
    if (gmac_dev)
    {
        unregister_netdev(gmac_dev);
        free_netdev(gmac_dev);
    }
}

static int mstar_gmac_drv_suspend(struct platform_device *dev, pm_message_t state)
{
    struct net_device *netdev=(struct net_device*)dev->dev.platform_data;
    struct GMAC_private *LocPtr= (struct GMAC_private*) netdev_priv(netdev);
    u32 uRegVal;
    printk(KERN_INFO "mstar_gmac_drv_suspend\n");

    if(!netdev)
    {
        return -ENODEV;
    }

    LocPtr = (struct GMAC_private*) netdev_priv(netdev);
    LocPtr->ep_flag |= GMAC_EP_FLAG_SUSPENDING;
    netif_stop_queue (netdev);

    disable_irq(netdev->irq);
    del_timer(&LocPtr->Link_timer);

    //Disable Receiver and Transmitter //
    uRegVal = MHal_GMAC_Read_CTL();
    uRegVal &= ~(GMAC_TE | GMAC_RE);
    MHal_GMAC_Write_CTL(uRegVal);

    // Disable PHY interrupt //
    MHal_GMAC_disable_phyirq ();
#ifndef RX_DELAY_INTERRUPT
    //Disable MAC interrupts //
    uRegVal = GMAC_INT_RCOM | GMAC_INT_ENABLE;
    MHal_GMAC_Write_IDR(uRegVal);
#else
    MHal_GMAC_Write_IDR(GMAC_INT_ENABLE);
#endif
    MHal_GMAC_Power_Off_Clk();

    if(LocPtr->ep_flag & GMAC_EP_FLAG_OPEND)
    {
        MDev_GMAC_close(netdev);
        LocPtr->ep_flag |= GMAC_EP_FLAG_SUSPENDING_OPEND;
    }

    return 0;
}
static int mstar_gmac_drv_resume(struct platform_device *dev)
{
    struct net_device *netdev = (struct net_device*)dev->dev.platform_data;
    struct GMAC_private *LocPtr = (struct GMAC_private *) netdev_priv(netdev);

    printk(KERN_INFO "mstar_gmac_drv_resume\n");

    if(!netdev)
    {
        return -ENODEV;
    }

    MDev_GMAC_HW_Reg_init(netdev);
    LocPtr->ep_flag &= ~GMAC_EP_FLAG_SUSPENDING;
    enable_irq(netdev->irq);

    if(LocPtr->ep_flag & GMAC_EP_FLAG_SUSPENDING_OPEND)
    {
        if(0>MDev_GMAC_open(netdev))
        {
            printk(KERN_WARNING "Driver GMAC: open failed after resume\n");
            return -ENOSYS;
        }
        LocPtr->ep_flag &= ~GMAC_EP_FLAG_SUSPENDING_OPEND;
    }

    return 0;
}

static ssize_t tx_packet_dump_en_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{

    int val = simple_strtoul(buf, NULL, 10);
    if(val)
        gb_tx_packet_dump_en=1;
    else
        gb_tx_packet_dump_en=0;

    return count;
}
static ssize_t tx_packet_dump_en_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "tx_en=%d tx_en=%d\n", gb_tx_packet_dump_en, gb_rx_packet_dump_en);
}
DEVICE_ATTR(tx_packet_dump_en, 0644, tx_packet_dump_en_show, tx_packet_dump_en_store);

static ssize_t rx_packet_dump_en_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{

    int val = simple_strtoul(buf, NULL, 10);
    if(val)
        gb_rx_packet_dump_en=1;
    else
        gb_rx_packet_dump_en=0;

    return count;
}
static ssize_t rx_packet_dump_en_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "tx_en=%d tx_en=%d\n", gb_tx_packet_dump_en, gb_rx_packet_dump_en);
}
DEVICE_ATTR(rx_packet_dump_en, 0644, rx_packet_dump_en_show, rx_packet_dump_en_store);

static ssize_t gmac_mdio_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    unsigned int physaddr, reg=0, value=0;

    physaddr = MHal_GMAC_PHY_ADDR();
    str += scnprintf(str, end - str, "%s physaddr:%d\n", gmac_dev->name , physaddr);
    for(reg=0; reg<32; reg++)
    {
        if(reg%8==0)  str += scnprintf(str, end - str, "%02d: ", reg);
        MHal_GMAC_read_phy(physaddr, reg, &value);
        str += scnprintf(str, end - str, "0x%04x ", value);
        if(reg%8==7)  str += scnprintf(str, end - str, "\n");
    }

    return (str - buf);
}

static ssize_t gmac_mdio_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    unsigned int physaddr, reg=0, value=0;
    unsigned char token[16];

    sscanf(buf, "%s", token);
    if (0 == strcasecmp(token, "r"))
    {
        sscanf(buf, "%s %d %d", token, &physaddr, &reg);
        MHal_GMAC_read_phy(physaddr, reg, &value);
        printk("mdio %s phyaddr:%d reg:%d val:0x%04x\n", token, physaddr, reg, value);
    }
    else if (0 == strcasecmp(token, "w"))
    {
        sscanf(buf, "%s %d %d 0x%x", token, &physaddr, &reg, &value);
        MHal_GMAC_write_phy(physaddr, reg, value);
        printk("mdio %s phyaddr:%d reg:%d val:0x%04x\n", token, physaddr, reg, value);
    }
    else if (0 == strcasecmp(token, "s"))
    {
        MDev_GMAC_ScanPhyAddr(gmac_dev);
    }
    else
    {
        printk("\nUsage: echo r [phyaddr] [reg] > mdio (r 3 0)\n");
        printk("Usage: echo w [phyaddr] [reg] [value_hex] > mdio (w 3 0 0x9140)\n");
    }

    return n;
}
DEVICE_ATTR(gmac_mdio, 0644, gmac_mdio_show, gmac_mdio_store);

static int mstar_gmac_drv_probe(struct platform_device *pdev)
{
    int retval=0;

    if( !(pdev->name) || strcmp(pdev->name,"Mstar-gmac")
        || pdev->id!=0)
    {
        retval = -ENXIO;
    }
#ifdef CONFIG_OF
    gIrq_from_dts = irq_of_parse_and_map(pdev->dev.of_node, 0);
#endif
    gdebug_class_gmac_dev = device_create(msys_get_sysfs_class(), NULL, MKDEV(241, 3), NULL, "gmac");
    device_create_file(gdebug_class_gmac_dev, &dev_attr_tx_packet_dump_en);
    device_create_file(gdebug_class_gmac_dev, &dev_attr_rx_packet_dump_en);
    device_create_file(gdebug_class_gmac_dev, &dev_attr_gmac_mdio);

    retval = MDev_GMAC_init();
    if(!retval)
    {
        pdev->dev.platform_data=gmac_dev;
    }
    return retval;
}

static int mstar_gmac_drv_remove(struct platform_device *pdev)
{
    if( !(pdev->name) || strcmp(pdev->name,"Mstar-gmac")
        || pdev->id!=0)
    {
        return -1;
    }
    MDev_GMAC_exit();
    pdev->dev.platform_data=NULL;
    return 0;
}

static struct platform_driver Mstar_gmac_driver = {
    .probe      = mstar_gmac_drv_probe,
    .remove     = mstar_gmac_drv_remove,
    .suspend    = mstar_gmac_drv_suspend,
    .resume     = mstar_gmac_drv_resume,
    .driver = {
        .name   = "Mstar-gmac",
#if defined(CONFIG_OF)
        .of_match_table = mstargmac_of_device_ids,
#endif
        .owner  = THIS_MODULE,
    }
};

static int __init mstar_gmac_drv_init_module(void)
{
    gmac_dev=NULL;
    return platform_driver_register(&Mstar_gmac_driver);
}

static void __exit mstar_gmac_drv_exit_module(void)
{
    platform_driver_unregister(&Mstar_gmac_driver);
    gmac_dev=NULL;
    return;
}


module_init(mstar_gmac_drv_init_module);
module_exit(mstar_gmac_drv_exit_module);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("GMAC Ethernet driver");
MODULE_LICENSE("GPL");
