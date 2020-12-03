/*
* mdrv_noe_phy.c- Sigmastar
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
/// @file   MDRV_NOE_PHY.c
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include "mdrv_noe.h"
#include "mdrv_noe_phy.h"

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
typedef void (*PFN_SET_CFG)(u32, struct st_drv_phy_config *);

typedef enum {
    E_NOE_PHY_ID_MARVELL = 0,
    E_NOE_PHY_ID_VTSS,
    E_NOE_PHY_ID_ATHEROS,
    E_NOE_PHY_ID_REALTEK,
    /* add id below */

    /* add id above */
    E_NOE_PHY_ID_MAX,
    E_NOE_PHY_ID_INVALID = E_NOE_PHY_ID_MAX,
}EN_NOE_PHY_ID;

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Local Functions
//--------------------------------------------------------------------------------------------------
static void _MDrv_NOE_PHY_Set_Vtss_Config(u32 phy_address, struct st_drv_phy_config *info);
static void _MDrv_NOE_PHY_Set_Marvell_Config(u32 phy_address, struct st_drv_phy_config *info);
static void _MDrv_NOE_PHY_Set_Atheros_Config(u32 phy_address, struct st_drv_phy_config *info);
static void _MDrv_NOE_PHY_Set_Realtek_Config(u32 phy_address, struct st_drv_phy_config *info);


//--------------------------------------------------------------------------------------------------
//  Local Variable
//--------------------------------------------------------------------------------------------------
static PFN_SET_CFG _pfn_set_config[E_NOE_PHY_ID_MAX] = {
    [E_NOE_PHY_ID_MARVELL] = _MDrv_NOE_PHY_Set_Marvell_Config,
    [E_NOE_PHY_ID_VTSS] = _MDrv_NOE_PHY_Set_Vtss_Config,
    [E_NOE_PHY_ID_ATHEROS] = _MDrv_NOE_PHY_Set_Atheros_Config,
    [E_NOE_PHY_ID_REALTEK] = _MDrv_NOE_PHY_Set_Realtek_Config,
};


static EN_NOE_PHY_ID _MDrv_NOE_PHY_Get_PHY_ID(u32 phy_address)
{
    u32 phy_id0 = 0, phy_id1 = 0;

    if (MHal_NOE_Read_Mii_Mgr(phy_address, MII_PHYSID1, &phy_id0) != E_NOE_RET_TRUE) {
        NOE_MSG_ERR("\n Read PhyID 1 is Fail!!  0x%x\n", phy_id0);
        phy_id0 = 0;
    }
    if (MHal_NOE_Read_Mii_Mgr(phy_address, MII_PHYSID2, &phy_id1) != E_NOE_RET_TRUE) {
        NOE_MSG_ERR("\n Read PhyID 1 is Fail!!  0x%x\n", phy_id1);
        phy_id1 = 0;
    }

    if ((phy_id0 == EV_MARVELL_PHY_ID0) && (phy_id1 == EV_MARVELL_PHY_ID1))
        return E_NOE_PHY_ID_MARVELL;

    if ((phy_id0 == EV_ATHEROS_PHY_ID0) && (phy_id1 == EV_ATHEROS_PHY_ID1))
        return E_NOE_PHY_ID_ATHEROS;

    if ((phy_id0 == EV_VTSS_PHY_ID0) && (phy_id1 == EV_VTSS_PHY_ID1))
        return E_NOE_PHY_ID_VTSS;

    if ((phy_id0 == EV_REALTEK_PHY_ID0) && ((phy_id1 & EV_REALTEK_PHY_ID1) == EV_REALTEK_PHY_ID1))
        return E_NOE_PHY_ID_REALTEK;

    return E_NOE_PHY_ID_INVALID;

}

static void _MDrv_NOE_PHY_Set_Vtss_Config(u32 phy_address, struct st_drv_phy_config *info)
{
    u32 reg_value;
    MHal_NOE_Write_Mii_Mgr(phy_address, 31, 1);
    MHal_NOE_Read_Mii_Mgr(phy_address, MII_NCONFIG, &reg_value);
    NOE_MSG_DBG("Vitesse phy skew: %x --> ", reg_value);
    reg_value |= (0x3 << 12);
    reg_value &= ~(0x3 << 14);
    MHal_NOE_Write_Mii_Mgr(phy_address, MII_NCONFIG, reg_value);
    MHal_NOE_Write_Mii_Mgr(phy_address, 31, 0);
}

static void _MDrv_NOE_PHY_Set_Marvell_Config(u32 phy_address, struct st_drv_phy_config *info)
{
    u32 reg_value;
    if (phy_address == CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2) {
        MHal_NOE_Read_Mii_Mgr(phy_address, MII_CTRL1000, &reg_value);
        /* turn off 1000Base-T Advertisement
         * (9.9=1000Full, 9.8=1000Half)
         */
        reg_value &= ~(3 << 8);
        MHal_NOE_Write_Mii_Mgr(phy_address, MII_CTRL1000, reg_value);

        MHal_NOE_Read_Mii_Mgr(phy_address, MII_NWAYTEST, &reg_value);
        /* Add delay to RX_CLK for RXD Outputs */
        reg_value |= 1 << 7;
        MHal_NOE_Write_Mii_Mgr(phy_address, MII_NWAYTEST, reg_value);

        MHal_NOE_Read_Mii_Mgr(phy_address, MII_BMCR, &reg_value);
        reg_value |= 1 << 15;   /* PHY Software Reset */
        MHal_NOE_Write_Mii_Mgr(phy_address, MII_BMCR, reg_value);
    }
    MHal_NOE_Read_Mii_Mgr(phy_address, MII_CTRL1000, &reg_value);
    /* turn off 1000Base-T Advertisement
     * (9.9=1000Full, 9.8=1000Half)
     */
    reg_value &= ~(3 << 8);
    MHal_NOE_Write_Mii_Mgr(phy_address, MII_CTRL1000, reg_value);

    /*10Mbps, debug */
    MHal_NOE_Write_Mii_Mgr(phy_address, MII_ADVERTISE, 0x461);

    MHal_NOE_Read_Mii_Mgr(phy_address, MII_BMCR, &reg_value);
    reg_value |= 1 << 9;    /* restart AN */
    MHal_NOE_Write_Mii_Mgr(phy_address, MII_BMCR, reg_value);

}

static void _MDrv_NOE_PHY_Set_Atheros_Config(u32 phy_address, struct st_drv_phy_config *info)
{
    u32 reg_value;
    if (info->mii_force_mode == NOE_ENABLE) {
#if defined(CONFIG_NOE_FPGA_VERIFY) || defined(CONFIG_MSTAR_ARM_BD_FPGA)
        /* turn off 1000Base-T Advertisement  (9.9=1000Full, 9.8=1000Half) */
        MHal_NOE_Read_Mii_Mgr(phy_address, MII_CTRL1000, &reg_value);
        reg_value &= ~(ADVERTISE_1000HALF|ADVERTISE_1000FULL);
        MHal_NOE_Write_Mii_Mgr(phy_address, MII_CTRL1000, reg_value);

        /*10Mbps, debug*/
        reg_value = ADVERTISE_PAUSE_CAP|ADVERTISE_10FULL|ADVERTISE_10HALF|ADVERTISE_CSMA;
        MHal_NOE_Write_Mii_Mgr(phy_address, MII_ADVERTISE, reg_value);

        MHal_NOE_Read_Mii_Mgr(phy_address, MII_BMCR, &reg_value);
        reg_value |= BMCR_ANENABLE|BMCR_ANENABLE; //restart AN
        MHal_NOE_Write_Mii_Mgr(phy_address, MII_BMCR, reg_value);

        NOE_MSG_DBG("phy addr: %d, Setup 10Mbps\n", phy_address);
#endif /* CONFIG_NOE_FPGA_VERIFY */
    }
    else {
        /* Set Auto Polling Mode */
        MHal_NOE_Read_Mii_Mgr(phy_address, MII_CTRL1000, &reg_value);
        reg_value |= (3<<8);
        MHal_NOE_Write_Mii_Mgr(phy_address, MII_CTRL1000, reg_value);

        reg_value = ADVERTISE_PAUSE_CAP|ADVERTISE_100FULL|ADVERTISE_100HALF|ADVERTISE_10FULL|ADVERTISE_10HALF|ADVERTISE_CSMA;
        MHal_NOE_Write_Mii_Mgr(phy_address, MII_ADVERTISE, reg_value);

        MHal_NOE_Read_Mii_Mgr(phy_address, MII_BMCR, &reg_value);
        reg_value |= (BMCR_ANENABLE | BMCR_ANRESTART) ;
        MHal_NOE_Write_Mii_Mgr(phy_address, MII_BMCR, reg_value);
    }


}

static int phy_poll_reset(u32 phy_address)
{
	/* Poll until the reset bit clears (50ms per retry == 0.6 sec) */
	unsigned int retries = 12;
	int ret;

	do {
		msleep(50);
		MHal_NOE_Read_Mii_Mgr(phy_address, MII_BMCR, &ret);
	} while (ret & BMCR_RESET && --retries);
	if (ret & BMCR_RESET)
		return -ETIMEDOUT;

	/* Some chips (smsc911x) may still need up to another 1ms after the
	 * BMCR_RESET bit is cleared before they are usable.
	 */
	msleep(1);
	return 0;
}

static void _MDrv_NOE_PHY_Set_Realtek_Config(u32 phy_address, struct st_drv_phy_config *info)
{
    u32 reg_value;

    MHal_NOE_Read_Mii_Mgr(phy_address, MII_BMCR, &reg_value);
    reg_value |= BMCR_RESET;
    MHal_NOE_Write_Mii_Mgr(phy_address, MII_BMCR, reg_value);
    phy_poll_reset(phy_address);

    /* Set Speed */
    MHal_NOE_Read_Mii_Mgr(phy_address, MII_ADVERTISE, &reg_value);
    reg_value |= ADVERTISE_10FULL | ADVERTISE_10HALF;
    reg_value |= ADVERTISE_100HALF | ADVERTISE_100FULL;
    reg_value &= (~ADVERTISE_PAUSE_CAP);
    reg_value |= ADVERTISE_CSMA;
    MHal_NOE_Write_Mii_Mgr(phy_address, MII_ADVERTISE, reg_value);

    MHal_NOE_Read_Mii_Mgr(phy_address, MII_CTRL1000, &reg_value);
    reg_value |= (ADVERTISE_1000HALF | ADVERTISE_1000FULL);
    MHal_NOE_Write_Mii_Mgr(phy_address, MII_CTRL1000, reg_value);


    /* restart AN */
    MHal_NOE_Read_Mii_Mgr(phy_address, MII_BMCR, &reg_value);
    reg_value |= (BMCR_ANENABLE | BMCR_ANRESTART);
    MHal_NOE_Write_Mii_Mgr(phy_address, MII_BMCR, reg_value);

}


void MDrv_NOE_PHY_Set_Config(u32 phy_addr, struct st_drv_phy_config *info)
{
    EN_NOE_PHY_ID id;
    MHal_NOE_Init_Mii_Mgr(info->ge, phy_addr, info->mii_force_mode);

    id = _MDrv_NOE_PHY_Get_PHY_ID(phy_addr);
    NOE_MSG_DBG("[GE%d] phy id: %d, phy addr: %d\n", (info->ge +1), id, phy_addr);

    if (id == E_NOE_PHY_ID_INVALID) {
        NOE_MSG_ERR("Invalid phy id  in GE%d\n", (info->ge +1));
        return;
    }
    _pfn_set_config[id](phy_addr, info);
}




