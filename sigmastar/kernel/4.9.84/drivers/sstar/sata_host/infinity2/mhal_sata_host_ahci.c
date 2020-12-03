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
#include <linux/kernel.h>
#include <linux/pm.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/libata.h>
#include <linux/ahci_platform.h>
#include <linux/acpi.h>
#include <linux/pci_ids.h>

#include "ms_platform.h"
//#include "mhal_sata_host_ahci.h"
#include "mhal_sata_host_ahci.h"


#if 0
u32 ahci_reg_read(void __iomem * p_reg_addr)
{
    u32 data;
    phys_addr_t reg_addr = (phys_addr_t)p_reg_addr;

#if defined(CONFIG_ARM64)
    data = (readw(reg_addr + 0x04) << 16) + readw(reg_addr);
#else
    data = (ioread16((void __iomem *)(reg_addr + 0x04))<<16) + ioread16((void __iomem *)reg_addr);
#endif
    return data;
}
EXPORT_SYMBOL(ahci_reg_read);


void ahci_reg_write(u32 data, void __iomem * p_reg_addr)
{
    phys_addr_t reg_addr = (phys_addr_t)p_reg_addr;

#if defined(CONFIG_ARM64)
    writew(data & 0xFFFF, reg_addr);
    writew((data >> 16) & 0xFFFF, (reg_addr + 0x04));

#else
    iowrite16(data&0xFFFF, (void __iomem *)reg_addr);
    iowrite16((data >> 16)&0xFFFF,(void __iomem *)(reg_addr + 0x04));
#endif
}
EXPORT_SYMBOL(ahci_reg_write);
#endif


void MHal_SATA_Clock_Config(phys_addr_t port_base, bool enable)
{
    u16 u16Temp;

    // Enable MAC Clock, bank is chip dependent
    // [0] clk gated
    // [1] clk invert
    // [4:2] 0: 240M, 1: 216M, 3: clk_miu

        printk("6+1 [SATA0] Clock : %s\n", (enable ? "ON" : "OFF"));

    if(enable)
    {
        if(port_base == SATA_GHC_0_P0_ADDRESS_START)
        {
            u16Temp = readw((volatile void *)MSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
            u16Temp &= ~(CKG_SATA_FCLK_MASK | CKG_SATA_FCLK_GATED);
            u16Temp |= CKG_SATA_FCLK_MIU_P;
            writew(u16Temp, (volatile void *)MSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
        }
        else if(port_base == SATA_GHC_1_P0_ADDRESS_START)
        {
            u16Temp = readw((volatile void *)MSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
            u16Temp &= ~((CKG_SATA_FCLK_MASK | CKG_SATA_FCLK_GATED) << 8);
            u16Temp |= (CKG_SATA_FCLK_MIU_P << 8);
            writew(u16Temp, (volatile void *)MSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
        }
    }
    else
    {
        if(port_base == SATA_GHC_0_P0_ADDRESS_START)
        {
            u16Temp = readw((volatile void *)MSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
            u16Temp &= ~(CKG_SATA_FCLK_MASK | CKG_SATA_FCLK_GATED);
            u16Temp |= CKG_SATA_FCLK_GATED;
            writew(u16Temp, (volatile void *)MSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
        }
        else if(port_base == SATA_GHC_1_P0_ADDRESS_START)
        {
            u16Temp = readw((volatile void *)MSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
            u16Temp &= ~((CKG_SATA_FCLK_MASK | CKG_SATA_FCLK_GATED) << 8);
            u16Temp |= (CKG_SATA_FCLK_GATED << 8);
            writew(u16Temp, (volatile void *)MSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
        }
    }

    if(port_base == SATA_GHC_0_P0_ADDRESS_START)
        printk("[SATA0] Clock : %s\n", (enable ? "ON" : "OFF"));
    else if(port_base == SATA_GHC_1_P0_ADDRESS_START)
        printk("[SATA1] Clock : %s\n", (enable ? "ON" : "OFF"));
}
//EXPORT_SYMBOL_GPL(MHal_SATA_Clock_Config);
EXPORT_SYMBOL(MHal_SATA_Clock_Config);




void MHal_SATA_Setup_Port_Implement(phys_addr_t misc_base, phys_addr_t port_base, phys_addr_t hba_base)
{
    printk("%s  misc_base = 0x%X , port_base = 0x%X , hba_base = 0x%X \n", __func__ , misc_base, port_base, hba_base );
    //  Init FW to trigger controller
    writew(0x0000, (volatile void *)hba_base + (MS_HOST_CAP));
    writew(0x0000, (volatile void *)hba_base + (MS_HOST_CAP + 0x4));

    printk("%s host_cap = 0x%X , host_cap + 4 = 0x%X \n", __func__  , readw( (volatile void *)hba_base + (MS_HOST_CAP))  , readw( (volatile void *)hba_base + (MS_HOST_CAP+0x04))  );

    // Port Implement
    writew(0x0001, (volatile void *)hba_base + (MS_HOST_PORTS_IMPL));
    writew(0x0000, (volatile void *)port_base + (MS_PORT_CMD));
    writew(0x0000, (volatile void *)port_base + (MS_PORT_CMD + 0x4));
}
EXPORT_SYMBOL(MHal_SATA_Setup_Port_Implement);
#if 0
static void ms_sata_clk_enable(void)
{
    // Enable Clock, bank is chip dependent
    writew(0x000c, (volatile void *)MSTAR_RIU_BASE + (0x100b64 << 1)); // [0] gate
    // [1] clk invert
    // [4:2] 0:240M, 1:216M, [3:clk_miu]
}
#endif
#if 0
static void ms_sata_clk_disable(void)
{
    MHal_SATA_Clock_Config(SATA_GHC_0_P0_ADDRESS_START, FALSE);
}
#endif

u32 MHal_SATA_get_max_speed(void)
{
    return E_PORT_SPEED_GEN3;
}
EXPORT_SYMBOL(MHal_SATA_get_max_speed);

void ss_sata_misc_init(void *mmio, int phy_mode, int n_ports)
{
    void __iomem *port_base = mmio + 0x200; //102C00<<2
    void __iomem *misc_base = mmio + 0x400; //102D00<<2
    u16 u16Temp;

    MHal_SATA_Clock_Config((phys_addr_t)port_base, TRUE);

    // ECO bit, to fix 1k boundary issue.
    u16Temp = readw((volatile void *)misc_base + SATA_MISC_AGEN_F_VAL);//
    u16Temp |= 0x8000;//
    writew(u16Temp, (volatile void *)misc_base + SATA_MISC_AGEN_F_VAL);//

    // [1] clk invert
    // [4:2] 0:240M, 1:216M, [3:clk_miu]
    writew(0x0001, (volatile void *)misc_base + SATA_MISC_HOST_SWRST);//0x50  a0
    writew(0x0000, (volatile void *)misc_base + SATA_MISC_HOST_SWRST);//0x50  a0
    //writew(0x0001, (volatile void *)misc_base + SATA_MISC_ACCESS_MODE);//0x18 30
    writew(0x0000, (volatile void *)misc_base + SATA_MISC_AMBA_MUXRST);//0x21 42
    writew(0x008C, (volatile void *)misc_base + SATA_MISC_AMBA_ARBIT);//0x2A  54

    // AHB Data FIFO  Setting
    writew(0x0000, (volatile void *)misc_base + SATA_MISC_HBA_HADDR);//0x25 4a//
    writew(0x0000, (volatile void *)misc_base + SATA_MISC_HBA_LADDR);//0x24 48//
    writew(0x1800, (volatile void *)misc_base + SATA_MISC_CMD_HADDR);//0x27 4E//
    writew(0x1000, (volatile void *)misc_base + SATA_MISC_CMD_LADDR);//0x26 4C//
    writew(0x0000, (volatile void *)misc_base + SATA_MISC_DATA_ADDR);//0x28 50//
    writew(0x0001, (volatile void *)misc_base + SATA_MISC_ENRELOAD);//0x29  52//
    writew(0x0001, (volatile void *)misc_base + SATA_MISC_ACCESS_MODE);//0x18 30//
#if 1
    u16Temp = readw((volatile void *)port_base + (0x2c << 1));
    u16Temp = u16Temp & (~E_PORT_SPEED_MASK);
    if(phy_mode == 0)
    {
        u16Temp = u16Temp | E_PORT_SPEED_GEN1;
    }
    else if(phy_mode == 1)
    {
        u16Temp = u16Temp | E_PORT_SPEED_GEN2;
    }
    else if(phy_mode == 2)
    {
        u16Temp = u16Temp | E_PORT_SPEED_GEN3;
    }
    else
    {
        u16Temp = u16Temp | MHal_SATA_get_max_speed();
    }
    writew(u16Temp, (volatile void *)port_base + (0x2c << 1));
    u16Temp = readw((volatile void *)port_base + (0x2c << 1));
    printk("%s MAC SATA SPEED= 0x%x\n", __func__ ,u16Temp);
#endif
}
EXPORT_SYMBOL(ss_sata_misc_init);

void ss_sata_phy_init(void *mmio, int phy_mode, int n_ports)
{
    u16 u16Temp;
    phys_addr_t hba_base = (phys_addr_t)mmio; //102B00<<1
    phys_addr_t port_base = (phys_addr_t)(mmio + 0x200); //102C00<<1
    u32 GHC_PHY = 0x0;

    if ((n_ports < 1) || (n_ports > 4))
        pr_err("ERROR: PORT num you set is WRONG!!!\n");

    if(port_base == SATA_GHC_0_P0_ADDRESS_START)
    {
        GHC_PHY = SATA_GHC_0_PHY;//0x103900
    }
    else if(port_base == SATA_GHC_1_P0_ADDRESS_START)
    {
        GHC_PHY = SATA_GHC_1_PHY;//0x162A00
    }

    //printk("sata phy init  A\n");
    //printk("Gen1 Tx swing\n");
    writew(0x2031, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x94) << 1));
    writew(0x3803, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x96) << 1));
    // EQ BW
    //printk("Gen1 EQ BW\n");
    u16Temp = readw((volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x92) << 1));
    u16Temp &= ~0x0060;
    u16Temp |= 0x0020;  // default is 0x01
    writew(u16Temp, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x92) << 1));
    // EQ STR
    //printk("Gen1 EQ STR\n");
    u16Temp = readw((volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x92) << 1));
    u16Temp &= ~0x001f;
    u16Temp |= 0x0003;  // default is 0x03
    writew(u16Temp, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x92) << 1));

    // Gen2
    // Tx swing
    //printk("Gen2 Tx swing\n");
    writew(0x3935, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x2c) << 1));
    writew(0x3920, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x2e) << 1));
    // EQ BW
    //printk("Gen2 EQ BW\n");
    u16Temp = readw((volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x50) << 1));
    u16Temp &= ~0x1800;
    u16Temp |= 0x0800;  // default is 0x01
    writew(u16Temp, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x50) << 1));
    // EQ STR
    //printk("Gen2 EQ STR\n");
    u16Temp = readw((volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x26) << 1));
    u16Temp &= ~0x001f;
    u16Temp |= 0x0003;  // default is 0x03
    writew(u16Temp, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x26) << 1));

    // Gen3
    // Tx swing
    //printk("Gen3 Tx swing\n");
    writew(0x3937, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x9a) << 1));
    writew(0x3921, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x9c) << 1));
    // EQ BW
    //printk("Gen3 EQ BW\n");
    u16Temp = readw((volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x98) << 1));
    u16Temp &= ~0x0060;
    u16Temp |= 0x0000;  // default is 0x00
    writew(u16Temp, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x98) << 1));
    // EQ STR
    //printk("Gen3 EQ EQ STR\n");
    //printk("EQ BW\n");
    u16Temp = readw((volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x98) << 1));
    u16Temp &= ~0x001f;
    u16Temp |= 0x0003;  // default is 0x03
    writew(u16Temp, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x98) << 1));

    // SSC setup
#if 1  // new settting to fixed PMP mode issue with JMB575
    // STEP1[10:0]
    writew(0x2002, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x64) << 1));  // [12]: 0 Stop sequence mode, 1: Reverse sequence mode
    // SET[15:0]
    writew(0x0aff, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x60) << 1));
    // SET[23:16]
    writeb(0x17, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x62) << 1));
    // SPAN[14:0]
    writew(0x0495, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x66) << 1));
#else
    writew(0x3007, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x64) << 1));
    writew(0xdf0a, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x60) << 1));
    writeb(0x24, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x62) << 1));
    writew(0x02e8, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x66) << 1));
#endif


    //printk("sata SATA PHY OOB setting ..\n");
    writew(0x96de, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));
    //writew(0x031e, MSTAR_RIU_BASE + (0x103934 << 1));
    writew(0x0f20, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x06) << 1));
    writew(0x341f, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x08) << 1));

    //printk("sata SATA PHY setting ..\n");
    //asm volatile( "loop: b loop");

#if 1  // new setting
    writew(0x0000, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x22) << 1));
    writew(0x0000, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x24) << 1));
    //writew(0x0a3d, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x60) << 1));
    //writeb(0x17, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x62) << 1));
    writew(0x9103, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0c) << 1));
    writeb(0x06, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x3e) << 1));
    writew(0x0000, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x10) << 1));  // Enable clocks
#if 0  //
    writeb(0x0f, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0b) << 1) - 1);  // [10]: OOB pattern select
    writew(0x848f, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));
    writew(0x848e, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));
#else
    writeb(0x0b, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0b) << 1) - 1);  // [10]: OOB pattern select
    writew(0x9a8f, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));  // [0]: Reset
    writew(0x9a8e, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));
#endif
    writew(0x831e, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x68) << 1));
    writew(0x031e, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0xc0) << 1));
    writew(0x031e, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0xc2) << 1));
    writeb(0x30, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x19) << 1) - 1);
    //writew(0x9110, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x2c) << 1));  // new add
    writeb(0x80, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x3c) << 1));
    writeb(0x50, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x33) << 1) - 1);  // [12]: reg_force_tx_oob_pat

    if(phy_mode == 1)  // Gen2
        writew(0x9114, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x2c) << 1));// for Gen2
    else
        writew(0x9115, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x2c) << 1));// for Gen3 and Gen1

    if(phy_mode == 0)  // for Gen1
    {
        writew(0x0811, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x94) << 1));
        writew(0x1115, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x9a) << 1));
        writew(0x1108, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x9c) << 1));
        // Override speed
        writeb(0x00, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x25) << 1) - 1);  // 0x00:Gen1, 0x50:Gen2, 0xA0:Gen3
    }
    else if(phy_mode == 1)  // for Gen2
    {
        writew(0x0811, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x94) << 1));
        writew(0x1115, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x9a) << 1));
        writew(0x1108, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x9c) << 1));
        // Override speed
        writeb(0x50, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x25) << 1) - 1);  // 0x00:Gen1, 0x50:Gen2, 0xA0:Gen3
    }
    else  // for Gen3
    {
        writew(0x0811, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x94) << 1));
        writew(0x111a, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x9a) << 1));
        writew(0x110a, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x9c) << 1));
        // Override speed
        writeb(0xA0, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x25) << 1) - 1);  // 0x00:Gen1, 0x50:Gen2, 0xA0:Gen3
    }

    writew(0x2200, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0e) << 1));  // [9]: Enable host

#else

    writew(0x0000, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x22) << 1));
    writew(0x0000, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x24) << 1));
    writew(0x0a3d, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x60) << 1));
    writeb(0x17, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x62) << 1));
    writew(0x9103, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0c) << 1));
    writeb(0x06, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x3e) << 1));
    writew(0x0000, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x10) << 1));  // Enable clocks
    writeb(0x0b, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0b) << 1) - 1);  // [10]: OOB pattern select
    writew(0x9a8f, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));  // [0]: Reset
    writew(0x9a8e, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));
    writew(0x831e, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x68) << 1));
    writew(0x031e, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0xc0) << 1));
    writew(0x031e, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0xc2) << 1));
    writeb(0x30, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x19) << 1) - 1);
    //writew(0x9110, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x2c) << 1));  // new add
    writeb(0x80, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x3c) << 1));
    writeb(0x50, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x33) << 1) - 1);  // [12]: reg_force_tx_oob_pat
    writew(0x2200, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0e) << 1));  // [9]: Enable host

#endif

    writew(0x0001, (volatile void *) hba_base + MS_HOST_CTL); // reset HBA

}
EXPORT_SYMBOL(ss_sata_phy_init);
