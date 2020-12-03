/*
* mhal_sata_host.c- Sigmastar
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
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <registers.h>
#include <mhal_sata_host.h>
#include "ms_platform.h"

#define sata_reg_write16(val, addr) { (*((volatile unsigned short*)(addr))) = (unsigned short)(val); }
#define sata_reg_write8(val, addr) { (*((volatile unsigned char*)(addr))) = (unsigned char)(val);}
#define GET_REG8_ADDR(x, y)     (x+(y)*2)
#define GET_REG16_ADDR(x, y)    (x+(y)*4)
#define RIU_BASE_ADDR 0xFD000000

void _MHal_SATA_Trim_init(void)
{
    u32 u32Efuse_2, u32Efuse_d, u32Efuse_e, u32Efuse_f;
    u16 u16Temp;

    CLRREG16(BASE_REG_EFUSE_PA + REG_ID_03, BIT8);  //select efuse section 0~7
    u32Efuse_2 = INREG16(BASE_REG_EFUSE_PA + REG_ID_08) | ((INREG16(BASE_REG_EFUSE_PA + REG_ID_09)) << 16);

    SETREG16(BASE_REG_EFUSE_PA + REG_ID_03, BIT8);  //select efuse section 8,9,A,B,C,D,E,F
    u32Efuse_d = INREG16(BASE_REG_EFUSE_PA + REG_ID_18) | ((INREG16(BASE_REG_EFUSE_PA + REG_ID_19)) << 16);
    u32Efuse_e = INREG16(BASE_REG_EFUSE_PA + REG_ID_1A) | ((INREG16(BASE_REG_EFUSE_PA + REG_ID_1B)) << 16);
    u32Efuse_f = INREG16(BASE_REG_EFUSE_PA + REG_ID_1C) | ((INREG16(BASE_REG_EFUSE_PA + REG_ID_1D)) << 16);

    if ((u32Efuse_f >> 16) & (0x1<<14))
    {
        // SATA_TX_R50 trim
        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_0B << 1));
        u16Temp |= 0x0010;
        writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_0B << 1));

        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_2A << 1));
        u16Temp &= ~(0x0F80);
        u16Temp |= (u16)(((u32Efuse_2 & 0x03E0) >> 5) << 7); //read bit[9:5] to bit[11:7]
        writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_2A << 1));
        printk("SATA_TX_R50 trim=0x%04X\n", (u16)((u32Efuse_2 & 0x03E0) >> 5));

        // SATA_RX_R50 trim
        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_0E << 1));
        u16Temp |= 0x0004;
        writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_0E << 1));

        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_3C << 1));
        u16Temp &= ~(0x01F0);
        u16Temp |= (u16)((((u32Efuse_e >> 16) & 0xF800) >> 11) << 4); //read bit[15:11] to bit[8:4]
        writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_3C << 1));
        printk("SATA_RX_R50 trim=0x%04X\n", (u16)(((u32Efuse_e >> 16) & 0xF800) >> 11));

        // SATA_TX_R50_IBIAS trim
        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_46 << 1));
        u16Temp |= (0x01 <<13);
        writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_46 << 1));

        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_50 << 1));
        u16Temp &= ~(0xFC00);
        u16Temp |= (u16)((((u32Efuse_f >> 16) & 0x3F00) >> 8) << 10); //read bit[13:8] to bit[15:10]
        writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_50 << 1));
        printk("SATA_TX_R50_IBIAS trim=0x%04X\n", (u16)(((u32Efuse_f >> 16) & 0x3F00) >> 8));

        // SATA_RXPLL_ICTRL_CDR trim
        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_CHIP_INFORM_SHADOW << 1));
        if( (u16Temp >> 8 ) == 0 )
            u32Efuse_d = 0 ;

        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_REG_30 << 1));
        u16Temp &= ~(0x0070);
        u16Temp |= (u16)(((u32Efuse_d & 0x00E0) >> 5) << 4); //read bit[7:5] to bit[6:4]
        writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_REG_30 << 1));
        printk("SATA_RXPLL_ICTRL_CDR trim=0x%04X\n", (u16)((u32Efuse_d & 0x00E0) >> 5)  );
    }
    else
    {
        // SATA_TX_R50 trim , bit11:7 = 0x10
        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_0B << 1));
        u16Temp |= 0x0010;
        writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_0B << 1));

        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_2A << 1));
        u16Temp &= ~(0x0F80);
        u16Temp |= (u16)( 0x10 << 7); //read bit[9:5] to bit[11:7]
        writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_2A << 1));

        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_2A << 1));
        printk("SATA_TX_R50 trim , trim=0x%04X \n",  (u16Temp >> 7) &0x1F );


        // SATA_RX_R50 trim , bit8:4 = 0x10
        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_0E << 1));
        u16Temp |= 0x0004;
        writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_0E << 1));

        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_3C << 1));
        u16Temp &= ~(0x01F0);
        u16Temp |= (u16)( 0x10 << 4); //read bit[15:11] to bit[8:4]
        writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_3C << 1));

        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_3C << 1));
        printk("SATA_RX_R50 trim , trim=0x%04X \n",  (u16Temp>>4) &0x1F );


        // SATA_TX_R50_IBIAS trim , bit15:10 = 0x20
        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_46 << 1));
        u16Temp |= (0x01 <<13);
        writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_46 << 1));

        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_50 << 1));
        u16Temp &= ~(0xFC00);
        u16Temp |= (u16)(0x20 << 10); //read bit[13:8] to bit[15:10]
        writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_50 << 1));

        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHYD_REG_50 << 1));
        printk("SATA_TX_R50_IBIAS trim ,  trim=0x%04X \n", (u16Temp>>10) &0x3F );

        // SATA_RXPLL_ICTRL_CDR trim, U01 bit6:4 = 0x00 , U02 bit6:4 = 0x03
        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_REG_30 << 1));
        u16Temp &= ~(0x0070);
        if(Chip_Get_Revision() == 0x2)
        {
            u16Temp |= (0x3 << 4); //  bit[6:4]
        }
        writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_REG_30 << 1));

        u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_SATA_PHY_REG_30 << 1));
        printk("SATA_RXPLL_ICTRL_CDR  trim=0x%04X \n", (u16Temp>>4) &0x07 );

    }
}

int _MHal_SATA_Power_On(void)
{
    struct device_node *dev_node;
    struct clk **sata_clks;
    struct clk *clk_parent;
    int num_parents, i;

#if CONFIG_OF
    dev_node = of_find_compatible_node(NULL, NULL, SSTAR_SATA_DTS_NAME);
    if (!dev_node)
        return -ENODEV;

    num_parents = of_clk_get_parent_count(dev_node);
    if(num_parents > 0)
    {
        sata_clks = kzalloc((sizeof(struct clk *) * num_parents), GFP_KERNEL);
        if(sata_clks == NULL)
        {
            printk( "[SATA0]kzalloc failed!\n" );
            return -ENOMEM;
        }

        //enable all clk
        for(i = 0; i < num_parents; i++)
        {
            sata_clks[i] = of_clk_get(dev_node, i);
            if (IS_ERR(sata_clks[i]))
            {
                printk( "Fail to get SATA clk!\n" );
                clk_put(sata_clks[i]);
                kfree(sata_clks);
                return -ENXIO;
            }

            /* Get parent clock */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
            clk_parent = clk_hw_get_parent_by_index(__clk_get_hw(sata_clks[i]), 0)->clk;
#else
            clk_parent = clk_get_parent_by_index(sata_clks[i], 0);
#endif
            if(IS_ERR(clk_parent))
            {
                printk( "[SATA0]can't get parent clock\n" );
                clk_put(sata_clks[i]);
                kfree(sata_clks);
                return -ENXIO;
            }
            /* Set clock parent */
            clk_set_parent(sata_clks[i], clk_parent);
            clk_prepare_enable(sata_clks[i]);
            clk_put(sata_clks[i]);
        }
        kfree(sata_clks);
    }

    return 0;
#else
    return -1;
#endif
}

void MHal_SATA_Clock_Config(phys_addr_t misc_base, phys_addr_t port_base, bool enable)
{
    u16 u16Temp;

    // Enable MAC Clock, bank is chip dependent
    // [0] phy clk gated
    // [1] phy clk invert
    // [3:2] 0: 108M
    // [8] clk gated
    // [9] clk invert
    // [11:10] 0: 432M

    if(enable)
    {
        if(port_base == SATA_GHC_0_P0_ADDRESS_START)
        {
            //*** Bank 0x1525 h0000 => clear bit 12
            u16Temp = readw((volatile void *)misc_base + (0x00 << 2));
            u16Temp &= ~(0x1000); // reg_sata_swrst_all
            writew(u16Temp, (volatile void *)misc_base + (0x00 << 2));

            _MHal_SATA_Trim_init();// This function must run after "Bank 0x1525 h0000 => clear bit 12"

            //*** Bank 0x000E h003D => clear bit 0~5
            u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_PD_XTAL_HV << 1));
            u16Temp &= ~(SATA_HV_CRYSTAL_CLK_MASK);
            writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_PD_XTAL_HV << 1));

            if (_MHal_SATA_Power_On())
            {
                printk("[SATA0] Can't control clock with dtb, set registers directly!\n");

                //*** Bank 0x1038 h006E => 0x0000
                writew(0x0000, (volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_AXI << 1));

                //*** Bank 0x1038 h006C => 0x0000
                writew(0x0000, (volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_PM << 1));

                //*** Bank 0x1038 h0046 => 0x0000
                u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
                u16Temp &= ~(CKG_SATA_FCLK_MASK | CKG_SATA_FCLK_GATED | CKG_SATA_FCLK_PHY_MASK | CKG_SATA_FCLK_PHY_GATED);
                writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
            }
       }
    }
    else
    {
        if(port_base == SATA_GHC_0_P0_ADDRESS_START)
        {
            //*** Bank 0x000E h003D => set bit 0~5
            //u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_PD_XTAL_HV << 1));
            //u16Temp |= SATA_HV_CRYSTAL_CLK_MASK;
            //writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_PD_XTAL_HV << 1));

            //*** Bank 0x1038 h0046 => 0x0101
            u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
            u16Temp &= ~(CKG_SATA_FCLK_MASK | CKG_SATA_FCLK_GATED | CKG_SATA_FCLK_PHY_MASK | CKG_SATA_FCLK_PHY_GATED);
            u16Temp |= (CKG_SATA_FCLK_GATED | CKG_SATA_FCLK_PHY_GATED);
            writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));

            //*** Bank 0x1525 h0000 => set bit 12
            u16Temp = readw((volatile void *)misc_base + (0x00 << 2));
            u16Temp |= 0x1000; // reg_sata_swrst_all
            writew(u16Temp, (volatile void *)misc_base + (0x00 << 2));
        }
    }

    if(port_base == SATA_GHC_0_P0_ADDRESS_START)
        printk("[SATA0] Clock : %s\n", (enable ? "ON" : "OFF"));
}
//EXPORT_SYMBOL_GPL(MHal_SATA_Clock_Config);

u32 MHal_SATA_get_max_speed(void)
{
    return E_PORT_SPEED_GEN3;
}
//EXPORT_SYMBOL_GPL(MHal_SATA_get_max_speed);

void MHal_SATA_Setup_Port_Implement(phys_addr_t misc_base, phys_addr_t port_base, phys_addr_t hba_base)
{
    //  Init FW to trigger controller
    writel(0x00000000, (volatile void *)hba_base + (SS_HOST_CAP));

    // Port Implement
    writel(0x00000001, (volatile void *)hba_base + (SS_HOST_PORTS_IMPL));
    writel(0x00000000, (volatile void *)port_base + (SS_PORT_CMD));
}
//EXPORT_SYMBOL_GPL(MHal_SATA_Setup_Port_Implement);

#ifndef CONFIG_SS_SATA_AHCI_PLATFORM_HOST
void MHal_SATA_HW_Inital(phys_addr_t misc_base, phys_addr_t port_base, phys_addr_t hba_base)
{
    //u16 u16Temp;
    u32 GHC_PHY_ANA = 0x0, u32Temp;
    int phy_mode;

    if(port_base == SATA_GHC_0_P0_ADDRESS_START)
    {
        GHC_PHY_ANA = SATA_GHC_0_PHY_ANA;//0x152700
    }

    MHal_SATA_Clock_Config(misc_base, port_base, 1);
    //u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
    //printk("SATA_CLK_REG = 0x%x\n", u16Temp);

    printk("RIU_BASE = 0x%x, GHC_PHY_ANA = 0x%x\n", SSTAR_RIU_BASE, GHC_PHY_ANA);
    //RIU_BASE = xfd000000, misc_base : xfd205a00 , hba_base : xfd345000 , port_base : xfd345080 , GHC_PHY : 152600

    //printk("check sata speed !!!!!!\n");
    u32Temp = readl((volatile void *)port_base + 0x2c);
    u32Temp = u32Temp & (~E_PORT_SPEED_MASK);
    u32Temp = u32Temp | MHal_SATA_get_max_speed();
    writel(u32Temp, (volatile void *)port_base + 0x2c);
    u32Temp = readl((volatile void *)port_base + 0x2c);
    printk("MAC SATA SPEED= 0x%x\n", u32Temp);

    if((u32Temp & E_PORT_SPEED_MASK) == E_PORT_SPEED_GEN1)
        phy_mode = 0;
    else if((u32Temp & E_PORT_SPEED_MASK) == E_PORT_SPEED_GEN2)
        phy_mode = 1;
    else
        phy_mode = 2;

    //printk("sata hal PHY init !!!!!!\n");
    //*** Bank 0x1527 h0005 => 0x00:Gen1, 0x01:Gen2, 0x10:Gen3
    switch (phy_mode)
    {
        case 0:
            writew(0x00, (volatile void *) GHC_PHY_ANA + (0x05 << 1));
            break;
        case 1:
            writew(0x01, (volatile void *) GHC_PHY_ANA + (0x05 << 1));
            break;
        case 2:
        default:
            writew(0x10, (volatile void *) GHC_PHY_ANA + (0x05 << 1));
            break;
    }

    writel(0x00000001, (volatile void *) hba_base + SS_HOST_CTL); // reset HBA

}
//EXPORT_SYMBOL_GPL(MHal_SATA_HW_Inital);

phys_addr_t MHal_SATA_bus_address(phys_addr_t phy_address)
{
    //phys_addr_t bus_address;

    //printk("phy addr = 0x%llx\n", phy_address);
    if (phy_address >= MIU_INTERVAL_SATA)
    {
        //printk("select MIU1, bus addr = 0x%8.8x\n", phy_address + 0x20000000);
        //return phy_address + MIU_INTERVAL_SATA;
        return phy_address + 0x20000000;
    }
    else
    {
        //printk("select MIU0, bus addr = 0x%8.8x\n", phy_address - 0x20000000);
        //return phy_address - MIU_INTERVAL_SATA;
        return phy_address - 0x20000000;
    }
}
//EXPORT_SYMBOL_GPL(MHal_SATA_bus_address);
#endif
