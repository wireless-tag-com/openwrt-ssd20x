/*
* mdrv_noe_ethtool.c- Sigmastar
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
/// @file   MDRV_NOE_ETHTOOL.c
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include "mdrv_noe.h"
#include <linux/mii.h>

//--------------------------------------------------------------------------------------------------
//  Local Variable
//--------------------------------------------------------------------------------------------------
static struct net_device *ethtool_dev = NULL;





unsigned char MDrv_NOE_ETHTOOL_Get_Phy_Address(void)
{
    struct END_DEVICE *ei_local;

    if (ethtool_dev != NULL) {
        ei_local = netdev_priv(ethtool_dev);
        if (!ei_local->netdev)
            return 0;
        return ei_local->mii_info.phy_id;
    }
    return 0;
}

u32 MDrv_NOE_ETHTOOL_Get_Link(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);

    return mii_link_ok(&ei_local->mii_info);
}

int MDrv_NOE_ETHTOOL_Set_Settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    mii_ethtool_sset(&ei_local->mii_info, cmd);
    NOE_MSG_DUMP("%s,%d  %d,%d,%d\n",__FUNCTION__,__LINE__,cmd->duplex, cmd->port ,cmd->autoneg);

    return 0;
}

int MDrv_NOE_ETHTOOL_Get_Settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);

    mii_ethtool_gset(&ei_local->mii_info, cmd);
    NOE_MSG_DUMP("%s,%d  %d,%d,%d\n",__FUNCTION__,__LINE__,cmd->duplex, cmd->port ,cmd->autoneg);
    return 0;
}

static int _MDrv_NOE_ETHTOOL_MDIO_Read(struct net_device *dev, int phy_id, int location)
{
    unsigned int result;
    struct END_DEVICE *ei_local = netdev_priv(dev);

    MHal_NOE_Read_Mii_Mgr((unsigned int)ei_local->mii_info.phy_id, (unsigned int)location, &result);
    NOE_MSG_DUMP("\n%s mii.o query= phy_id:%d\n",dev->name, phy_id);
    NOE_MSG_DUMP("address:%d retval:%x\n", location, result);
    return (int)result;
}


static void _MDrv_NOE_ETHTOOL_MDIO_Write(struct net_device *dev, int phy_id, int location, int value)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);

    MHal_NOE_Write_Mii_Mgr((unsigned int)ei_local->mii_info.phy_id, (unsigned int)location, (unsigned int)value);
    NOE_MSG_DUMP("mii.o write= phy_id:%d\n", phy_id);
    NOE_MSG_DUMP("address:%d value:%x\n", location, value);
}



u32 MDrv_NOE_ETHTOOL_Virt_Get_Link(struct net_device *dev)
{
    struct PSEUDO_ADAPTER *pseudo = netdev_priv(dev);
    struct END_DEVICE *ei_local = netdev_priv(ethtool_dev);

    if (ei_local->features & FE_GE2_SUPPORT)
        return mii_link_ok(&pseudo->mii_info);
    else
        return 0;
}


int MDrv_NOE_ETHTOOL_Virt_Set_Settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
    struct PSEUDO_ADAPTER *pseudo = netdev_priv(dev);
    struct END_DEVICE *ei_local = netdev_priv(ethtool_dev);

    if (ei_local->features & FE_GE2_SUPPORT)
        mii_ethtool_sset(&pseudo->mii_info, cmd);
    return 0;
}


int MDrv_NOE_ETHTOOL_Virt_Get_Settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
    struct PSEUDO_ADAPTER *pseudo = netdev_priv(dev);
    struct END_DEVICE *ei_local = netdev_priv(ethtool_dev);

    if (ei_local->features & FE_GE2_SUPPORT)
        mii_ethtool_gset(&pseudo->mii_info, cmd);
    return 0;
}

static int _MDrv_NOE_ETHTOOL_MDIO_Virt_Read(struct net_device *dev, int phy_id, int location)
{
    unsigned int result;
    struct PSEUDO_ADAPTER *pseudo = netdev_priv(dev);
    struct END_DEVICE *ei_local = netdev_priv(ethtool_dev);

    if (ei_local->features & FE_GE2_SUPPORT) {
        MHal_NOE_Read_Mii_Mgr((unsigned int)pseudo->mii_info.phy_id, (unsigned int)location, &result);
        NOE_MSG_DUMP("%s mii.o query= phy_id:%d,\n", dev->name, phy_id);
        NOE_MSG_DUMP("address:%d retval:%d\n", location, result);
        return (int)result;
    }
    return 0;
}

static void _MDrv_NOE_ETHTOOL_MDIO_Virt_Write(struct net_device *dev, int phy_id, int location, int value)
{
    struct PSEUDO_ADAPTER *pseudo = netdev_priv(dev);
    struct END_DEVICE *ei_local = netdev_priv(ethtool_dev);

    if (ei_local->features & FE_GE2_SUPPORT) {
        MHal_NOE_Write_Mii_Mgr((unsigned int)pseudo->mii_info.phy_id, (unsigned int)location, (unsigned int)value);
    }

    NOE_MSG_DUMP("mii.o write= phy_id:%d\n", phy_id);
    NOE_MSG_DUMP("address:%d value:%d\n)", location, value);
}

void MDrv_NOE_ETHTOOL_Init(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);

    /* store global info */
    ethtool_dev = dev;
    /* init mii structure */
    ei_local->mii_info.dev = dev;
    ei_local->mii_info.mdio_read = _MDrv_NOE_ETHTOOL_MDIO_Read;
    ei_local->mii_info.mdio_write = _MDrv_NOE_ETHTOOL_MDIO_Write;
    ei_local->mii_info.phy_id_mask = 0x1f;
    ei_local->mii_info.reg_num_mask = 0x1f;
    ei_local->mii_info.supports_gmii = mii_check_gmii_support(&ei_local->mii_info);

    /* TODO:   phy_id: 0~4 */
    ei_local->mii_info.phy_id = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
}

void MDrv_NOE_ETHTOOL_Virt_Init(struct net_device *dev)
{
    struct PSEUDO_ADAPTER *p_pseudo_ad = netdev_priv(dev);

    /* init mii structure */
    p_pseudo_ad->mii_info.dev = dev;
    p_pseudo_ad->mii_info.mdio_read = _MDrv_NOE_ETHTOOL_MDIO_Virt_Read;
    p_pseudo_ad->mii_info.mdio_write = _MDrv_NOE_ETHTOOL_MDIO_Virt_Write;
    p_pseudo_ad->mii_info.phy_id_mask = 0x1f;
    p_pseudo_ad->mii_info.reg_num_mask = 0x1f;
    p_pseudo_ad->mii_info.phy_id = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
    p_pseudo_ad->mii_info.supports_gmii = mii_check_gmii_support(&p_pseudo_ad->mii_info);

}


