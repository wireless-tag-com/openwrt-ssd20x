/*
* mhal_sata_host_ahci.c- Sigmastar
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

#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/delay.h>
#include "ms_platform.h"
#include "mdrv_types.h"
#include "mhal_sata_host.h"
#include "mhal_sata_host_ahci.h"

//#define USE_MIPSPLL

#if 0
static void ss_sata_clk_enable(void)
{
    // Enable Clock, bank is chip dependent
    writew(0x000c, (volatile void *)SSTAR_RIU_BASE + (0x100b64 << 1)); // [0] gate
    // [1] clk invert
    // [4:2] 0:240M, 1:216M, [3:clk_miu]
}
#endif
#if 0
static void ss_sata_clk_disable(void)
{
    MHal_SATA_Clock_Config(SATA_MISC_0_ADDRESS_START, SATA_GHC_0_P0_ADDRESS_START, FALSE);
}
#endif

void ss_sata_misc_init(void *mmio, int n_ports)
{
    void __iomem *port_base = mmio + 0x100;   //1A2880<<2
    void __iomem *misc_base = mmio - 0xA0600; //152500<<2

    MHal_SATA_Clock_Config((phys_addr_t)misc_base, (phys_addr_t)port_base, TRUE);
}

void ss_sata_phy_init(void *mmio,int phy_mode, int n_ports)
{
    //phys_addr_t hba_base = (phys_addr_t)mmio; //1A2800<<1
    phys_addr_t port_base = (phys_addr_t)(mmio + 0x100); //1A2880<<1
    u32 GHC_PHY_ANA = 0x0;
    u16 u16Temp;

    if ((n_ports < 1) || (n_ports > 4))
        pr_err("ERROR: PORT num you set is WRONG!!!\n");

    if(port_base == SATA_GHC_0_P0_ADDRESS_START)
    {
        GHC_PHY_ANA = SATA_GHC_0_PHY_ANA;//0x152700
    }

    //printk("sata phy init  A\n");

    //*** Bank 0x1527 h0020 => 0x0100
    writew(0x0100,  (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_REG_20 << 1));

    //*** bringup script setting , Bank 0x1527 h0030 => 0x1008 , and need to apply efuse  SATA_RXPLL_ICTRL_CDR trim setting 
    u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_REG_30 << 1));
    u16Temp = (u16Temp & ~0x0F) | 0x0008;
    u16Temp = u16Temp | 0x1000;
    writew(u16Temp,  (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_REG_30 << 1));

    //*** Bank 0x1527 h0033 => 0x0500
    writew(0x0500,  (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_REG_33 << 1));

    //*** Bank 0x1527 h0060 => 0x0002
    writew(0x0002,  (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_TXPLL_DET_SW << 1));

    //*** Bank 0x1527 h0070 => 0x0062
    writew(0x0062,  (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_REG_70 << 1));

    //*** Bank 0x1527 h003E => 0x8000
    writew(0x8000,  (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_REG_3E << 1));
#ifdef USE_MIPSPLL
    //*** Bank 0x1032 h0011 => enable bit11
    u16Temp = readw((volatile void *)SSTAR_RIU_BASE + ((0x103200 + (0x11<<1)) << 1));
    u16Temp |= 0x0800;
    printk("Bank 0x1032 0x11 = 0x%x\n", u16Temp);
    writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + ((0x103200 + (0x11<<1)) << 1));

    //*** Bank 0x1527 h0004 => 0x0000
    writew(0x0000,  (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_CLK_PMALIVE_SEL << 1));
#else
    //*** Bank 0x1527 h0004 => 0x0002
    writew(0x0002,  (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_CLK_PMALIVE_SEL << 1));
#endif

    //*** Bank 0x1527 h0044 => set bit 0
    u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_SYNTH_SLD << 1));
    u16Temp |= 0x1;
    writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_SYNTH_SLD << 1));
    u16Temp &= ~0x1;
    writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_SYNTH_SLD << 1));

    //*** Bank 0x1526 h000A => 0x440A
    writew(0x440A,  (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_0A << 1));
    //*** Bank 0x1526 h0026 => 0x1905
    writew(0x1905,  (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_26 << 1));
    //*** Bank 0x1526 h003E => 0xB659
    writew(0xB659,  (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_3E << 1));
    //*** Bank 0x1526 h0040 => 0xD819
    writew(0xD819,  (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_40 << 1));
    //*** Bank 0x1526 h0061 => 0x4000
    writew(0x4000,  (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_61 << 1));
    //*** Bank 0x1526 h0064 => 0x0044
    writew(0x0044,  (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_64 << 1));

    //*** Bank 0x1527 h0005 => 0x0001
    writew(0x02, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY_ANA + (0x5 << 1)) << 1));
}
