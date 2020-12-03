/*
* sstar_100_phy.c- Sigmastar
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
#include <linux/phy.h>
#include <linux/module.h>
#include <linux/delay.h>


//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
#define NETPHYINFO_ENABLE               0

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#if NETPHYINFO_ENABLE
    #define NETPHY_PRINT(x, args...)        { printk(x, ##args); }
#else
    #define NETPHY_PRINT(x, args...)        { }
#endif

#define SSTAR_100_PHY_ID        0x11112222
#define SSTAR_100_PHY_ID_MSK    0xffffffff

typedef struct
{
    int cnt_restart;
} sstar_phy_priv_t;


//-------------------------------------------------------------------------------------------------
//  Implementation
//-------------------------------------------------------------------------------------------------
static int sstar_phy_config_init(struct phy_device *phydev)
{
    // printk("[%s][%d]\n", __FUNCTION__, __LINE__);
    // phy_write(phydev, MII_BMCR, 0x1000);
    // phy_write(phydev, MII_BMCR, 0x1000);
    return genphy_config_init(phydev);
    // return 0;
}

static int sstar_phy_reset(struct phy_device *phydev)
{
#if 0
    phy_write(phydev, MII_BMCR, BMCR_RESET);
    msleep(50);
    phy_write(phydev, MII_BMCR, 0);
    msleep(10);
    phy_write(phydev, MII_BMCR, 0x1000);
    phy_write(phydev, MII_BMCR, 0x1000);
#endif
    return 0;
}

#define PHY_AN(phydev, cnt)                                                     \
{                                                                               \
    (cnt)++;                                                                    \
    if (10 < (cnt))                                                             \
    {                                                                           \
        phy_write((phydev),  MII_BMCR, 0x1200);                                 \
        NETPHY_PRINT("[%s][%d] restart an process\n", __FUNCTION__, __LINE__);  \
        (cnt) = 0;                                                              \
        return 1;                                                               \
    }                                                                           \
}
        
static int sstar_phy_patch(struct phy_device *phydev)
{
    u32 hcd_link_st_ok, an_100t_link_st = 0;
    // static unsigned int phy_restart_cnt = 0;
    u32 an_state1 = 0;
    u32 an_state2 = 0;
    u32 an_state3 = 0;
    sstar_phy_priv_t* priv = (sstar_phy_priv_t*)phydev->priv;

    if (0 == phydev->link)
        return 0;
    if (SPEED_100 != phydev->speed)
        return 0;

    if (0xffff == (hcd_link_st_ok = phy_read(phydev, 0x21)))
        return 0;
    if (0xffff == (an_100t_link_st = phy_read(phydev, 0x22)))
        return 0;

    if ((!(hcd_link_st_ok & 0x100) && ((an_100t_link_st & 0x300) == 0x200)))
    {
        // phy_restart_cnt++;
        // gu32PhyResetCount1++;
        NETPHY_PRINT("[%s][%d] hcd_link_st_ok:0x%08x, an_100t_link_st:0x%08x\n", __FUNCTION__, __LINE__, hcd_link_st_ok, an_100t_link_st);
        PHY_AN(phydev, priv->cnt_restart);
    }
    else if (((hcd_link_st_ok & 0x100) && !(an_100t_link_st & 0x300)))
    {
        // phy_restart_cnt++;
        // gu32PhyResetCount1++;
        NETPHY_PRINT("[%s][%d] hcd_link_st_ok:0x%08x, an_100t_link_st:0x%08x\n", __FUNCTION__, __LINE__, hcd_link_st_ok, an_100t_link_st);
        PHY_AN(phydev, priv->cnt_restart);
    }

    /* Monitor AN state*/
    if (0xffff == (an_state1 = phy_read(phydev, 0x2e)))
        return 0;
    if (0xffff == (an_state2 = phy_read(phydev, 0x2e)))
        return 0;
    if (0xffff == (an_state3 = phy_read(phydev, 0x2e)))
        return 0;

    if ((an_state1 != an_state2) || (an_state1 != an_state3))
    {
        NETPHY_PRINT("[%s][%d] an_state 1:0x%08x, 2:0x%08x, 3:0x%08x\n", __FUNCTION__, __LINE__, an_state1, an_state2, an_state3);
        return 0;
    }

    if ((an_state1 & 0xf000) == 0x3000)
    {
        // phy_restart_cnt++;
        // gu32PhyResetCount3++;
        NETPHY_PRINT("[%s][%d] an_state=0x%x\n", __FUNCTION__, __LINE__, an_state1);
        PHY_AN(phydev, priv->cnt_restart);
    }
    else if ((an_state1 & 0xf000) == 0x2000)
    {
        // phy_restart_cnt++;
        // gu32PhyResetCount4++;
        NETPHY_PRINT("[%s][%d] an_state=0x%x\n", __FUNCTION__, __LINE__, an_state1);
        PHY_AN(phydev, priv->cnt_restart);
    }
    return 0;
}

#if 0
static int sstar_phy_update_link(struct phy_device *phydev)
{
    int status;

    /* Do a fake read */
    status = phy_read(phydev, MII_BMSR);
    if (status < 0)
        return status;

    /* Read link and autonegotiation status */
    status = phy_read(phydev, MII_BMSR);
    if (status < 0)
        return status;

    if ((status & BMSR_LSTATUS) == 0)
        phydev->link = 0;
    else
        phydev->link = 1;

    return 0;
}
#endif

#if 0
static int _sstar_phy_read_status(struct phy_device *phydev)
{
    int adv;
    // int err;
    int lpa;
    // int lpagb = 0;
    // int common_adv;
    // int common_adv_gb = 0;
    int bmcr = 0;
    int bmsr = 0;
    unsigned int neg;

#if 0
    /* Update the link, but return if there was an error */
    err = sstar_phy_update_link(phydev);
    if (err)
        return err;
#else
    bmsr = phy_read(phydev, MII_BMSR);
    bmsr = phy_read(phydev, MII_BMSR);
    if (bmsr < 0)
        return bmsr;

    if ((bmsr & BMSR_LSTATUS) == 0)
    {
        phydev->link = 0;
    }
    else
    {
        phydev->link = 1;
    }
#endif

    phydev->lp_advertising = 0;
    bmcr = phy_read(phydev, MII_BMCR);
    // phydev->autoneg = (bmcr & BMCR_ANENABLE) ? BMCR_ANENABLE : AUTONEG_DISABLE;
    // if (AUTONEG_ENABLE != phydev->autoneg) {
    if (!(bmcr & BMCR_ANENABLE))
    {
        if (bmcr < 0)
            return bmcr;

        if (bmcr & BMCR_FULLDPLX)
            phydev->duplex = DUPLEX_FULL;
        else
            phydev->duplex = DUPLEX_HALF;

        if (bmcr & BMCR_SPEED100)
            phydev->speed = SPEED_100;
        else
            phydev->speed = SPEED_10;

        phydev->pause = 0;
        phydev->asym_pause = 0;
        return 0;
    }

    // if (AUTONEG_ENABLE == phydev->autoneg) {
    // if (bmcr & BMCR_ANENABLE)
    {
        //AutoNegotiation is enabled //
        lpa = phy_read(phydev, MII_LPA);
        /* For Link Parterner adopts force mode and EPHY used,
         * EPHY LPA reveals all zero value.
         * EPHY would be forced to Full-Duplex mode.
         */
        if (!lpa)
        {
            /* 100Mbps Full-Duplex */
            if (bmcr & BMCR_SPEED100)
                lpa |= LPA_100FULL;
            else /* 10Mbps Full-Duplex */
                lpa |= LPA_10FULL;
        }
        if (lpa < 0)
            return lpa;

        phydev->lp_advertising |= mii_lpa_to_ethtool_lpa_t(lpa);

        adv = phy_read(phydev, MII_ADVERTISE);
        if (adv < 0)
            return adv;

        neg = lpa & adv;

        phydev->speed = SPEED_10;
        phydev->duplex = DUPLEX_HALF;
        phydev->pause = 0;
        phydev->asym_pause = 0;

        if (neg & LPA_100FULL)
        {
            phydev->speed = SPEED_100;
            phydev->duplex = DUPLEX_FULL;
        }
        else if (neg & LPA_100HALF)
        {
            phydev->speed = SPEED_100;
            phydev->duplex = DUPLEX_HALF;
        }
        else if (neg & LPA_10FULL)
        {
            phydev->speed = SPEED_10;
            phydev->duplex = DUPLEX_FULL;
        }
        else if (neg & LPA_10HALF)
        {
            phydev->speed = SPEED_10;
            phydev->duplex = DUPLEX_HALF;
        }
        else
        {
            phydev->speed = SPEED_10;
            phydev->duplex = DUPLEX_HALF;
            NETPHY_PRINT("[%s][%d] No speed and mode found (LPA=0x%8x, ADV=0x%8x)\n", __FUNCTION__, __LINE__, lpa, adv);
        }
        if (phydev->duplex == DUPLEX_FULL) {
            phydev->pause = lpa & LPA_PAUSE_CAP ? 1 : 0;
            phydev->asym_pause = lpa & LPA_PAUSE_ASYM ? 1 : 0;
        }
        return 0;
    }
    return 0;
}
#endif

static int sstar_phy_read_status(struct phy_device *phydev)
{
    int ret;
    // printk("[%s][%d]\n", __FUNCTION__, __LINE__);

    ret = genphy_read_status(phydev);
    // ret = _sstar_phy_read_status(phydev);
    if ((0 == ret) && (1 == phydev->link))
        sstar_phy_patch(phydev);
    return ret;
}

static int sstar_phy_config_aneg(struct phy_device *phydev)
{
    // printk("[%s][%d]\n", __FUNCTION__, __LINE__);
#if 0
    phy_write(phydev, MII_BMCR, 0x1000);
    phy_write(phydev, MII_BMCR, 0x1000);
#endif
    // genphy_config_aneg(phydev);
    return genphy_config_aneg(phydev);
    // genphy_restart_aneg(phydev);
    // return 0;
}


static int sstar_phy_probe(struct phy_device *phydev)
{
    // printk("[%s][%d]\n", __FUNCTION__, __LINE__);
    if (NULL == (phydev->priv = kzalloc(sizeof(sstar_phy_priv_t), GFP_KERNEL)))
        return -ENOMEM;
    // printk("[%s][%d]\n", __FUNCTION__, __LINE__);
    return 0;
}

static void sstar_phy_remove(struct phy_device *phydev)
{
    // printk("[%s][%d]\n", __FUNCTION__, __LINE__);
    if (phydev->priv)
        kfree(phydev->priv);
    // printk("[%s][%d]\n", __FUNCTION__, __LINE__);
}


static struct phy_driver __maybe_unused sstar_phy_drvs[] = {
    {
        .phy_id         = SSTAR_100_PHY_ID,
        .phy_id_mask    = SSTAR_100_PHY_ID_MSK,
        .name           = "SStar 10/100 Ethernet Phy",
        .features       = (PHY_BASIC_FEATURES | SUPPORTED_Pause | SUPPORTED_Asym_Pause),
        .config_aneg    = sstar_phy_config_aneg,
        .read_status    = sstar_phy_read_status,
        .config_init    = sstar_phy_config_init,
        .soft_reset     = sstar_phy_reset,
        .probe          = sstar_phy_probe,
        .remove         = sstar_phy_remove,
    }, 
};

module_phy_driver(sstar_phy_drvs);

static struct mdio_device_id __maybe_unused sstar_phy_tbl[] = {
	{ SSTAR_100_PHY_ID, SSTAR_100_PHY_ID_MSK },
	{ }
};
MODULE_DEVICE_TABLE(mdio, sstar_phy_tbl);

MODULE_DESCRIPTION("SStar 100 PHY driver");
MODULE_LICENSE("GPL");
