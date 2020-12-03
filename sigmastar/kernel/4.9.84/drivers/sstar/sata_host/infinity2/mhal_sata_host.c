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
#include <mhal_sata_host.h>
#include <linux/delay.h>
#include "ms_platform.h"
extern ptrdiff_t mstar_pm_base;
#define sata_reg_write16(val, addr) { (*((volatile unsigned short*)(addr))) = (unsigned short)(val); }
#define sata_reg_write8(val, addr) { (*((volatile unsigned char*)(addr))) = (unsigned char)(val);}
#define GET_REG8_ADDR(x, y)     (x+(y)*2)
#define GET_REG16_ADDR(x, y)    (x+(y)*4)
#define RIU_BASE_ADDR 0xFD000000

static void MHal_SATA_Clock_Config(phys_addr_t misc_base, phys_addr_t port_base, bool enable)
{
    u16 u16Temp;

    // Enable MAC Clock, bank is chip dependent
    // [0] clk gated
    // [1] clk invert
    // [4:2] 0: 240M, 1: 216M, 3: clk_miu

    if(enable)
    {
        if(port_base == SATA_GHC_0_P0_ADDRESS_START)
        {
            u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
            u16Temp &= ~(CKG_SATA_FCLK_MASK | CKG_SATA_FCLK_GATED);
            u16Temp |= CKG_SATA_FCLK_MIU_P;
            writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
        }
        else if(port_base == SATA_GHC_1_P0_ADDRESS_START)
        {
            u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
            u16Temp &= ~((CKG_SATA_FCLK_MASK | CKG_SATA_FCLK_GATED) << 8);
            u16Temp |= (CKG_SATA_FCLK_MIU_P << 8);
            writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
        }
    }
    else
    {
        if(port_base == SATA_GHC_0_P0_ADDRESS_START)
        {
            u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
            u16Temp &= ~(CKG_SATA_FCLK_MASK | CKG_SATA_FCLK_GATED);
            u16Temp |= CKG_SATA_FCLK_GATED;
            writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
        }
        else if(port_base == SATA_GHC_1_P0_ADDRESS_START)
        {
            u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
            u16Temp &= ~((CKG_SATA_FCLK_MASK | CKG_SATA_FCLK_GATED) << 8);
            u16Temp |= (CKG_SATA_FCLK_GATED << 8);
            writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
        }
    }

    if(port_base == SATA_GHC_0_P0_ADDRESS_START)
        printk("[SATA0] Clock : %s\n", (enable ? "ON" : "OFF"));
    else if(port_base == SATA_GHC_1_P0_ADDRESS_START)
        printk("[SATA1] Clock : %s\n", (enable ? "ON" : "OFF"));
}
//EXPORT_SYMBOL_GPL(MHal_SATA_Clock_Config);

static u32 MHal_SATA_get_max_speed(void)
{
    return E_PORT_SPEED_GEN3;
}
//EXPORT_SYMBOL_GPL(MHal_SATA_get_max_speed);

static void MHal_SATA_HW_Inital(phys_addr_t misc_base, phys_addr_t port_base, phys_addr_t hba_base)
{
    u16 u16Temp;
    u32 GHC_PHY = 0x0;
    int phy_mode;

    if(port_base == SATA_GHC_0_P0_ADDRESS_START)
    {
        GHC_PHY = SATA_GHC_0_PHY;//0x103900
    }
    else if(port_base == SATA_GHC_1_P0_ADDRESS_START)
    {
        GHC_PHY = SATA_GHC_1_PHY;//0x162A00
    }

    MHal_SATA_Clock_Config(misc_base,port_base, 1);
    //u16Temp = readw((volatile void *)SSTAR_RIU_BASE + (REG_CKG_SATA_FCLK << 1));
    //printk("SATA_CLK_REG = 0x%x\n", u16Temp);

    printk("RIU_BASE = 0x%x, GHC_PHY = 0x%x\n", SSTAR_RIU_BASE, GHC_PHY);
    //RIU_BASE = xfd000000, misc_base : xfd205a00 , hba_base : xfd205800 , port_base : xfd226400 , GHC_PHY : 103900

    // ECO bit, to fix 1k boundary issue.

    //u16Temp = readw((const volatile void *)SSTAR_RIU_BASE + (0x2d35 << 1));
    u16Temp = readw((volatile void *)misc_base + SATA_MISC_AGEN_F_VAL);
    u16Temp |= 0x8000;
    writew(u16Temp, (volatile void *)misc_base + SATA_MISC_AGEN_F_VAL);

    writew(0x0001, (volatile void *)misc_base + SATA_MISC_HOST_SWRST);//25
    writew(0x0000, (volatile void *)misc_base + SATA_MISC_HOST_SWRST);//25
    writew(0x0000, (volatile void *)misc_base + SATA_MISC_AMBA_MUXRST);//21
    writew(0x008C, (volatile void *)misc_base + SATA_MISC_AMBA_ARBIT);//2
    //writew(0x0003, (volatile void *)misc_base +(0x55<<2));//FPGA mode
    // AHB Data FIFO  Setting
    writew(0x0000, (volatile void *)misc_base + SATA_MISC_HBA_HADDR);
    writew(0x0000, (volatile void *)misc_base + SATA_MISC_HBA_LADDR);
    writew(0x1800, (volatile void *)misc_base + SATA_MISC_CMD_HADDR);
    writew(0x1000, (volatile void *)misc_base + SATA_MISC_CMD_LADDR);
    writew(0x0000, (volatile void *)misc_base + SATA_MISC_DATA_ADDR);
    writew(0x0001, (volatile void *)misc_base + SATA_MISC_ENRELOAD);
    writew(0x0001, (volatile void *)misc_base + SATA_MISC_ACCESS_MODE);

    //printk("check sata speed !!!!!!\n");
    u16Temp = readw((volatile void *)port_base + (0x2c << 1));
    u16Temp = u16Temp & (~E_PORT_SPEED_MASK);
    u16Temp = u16Temp | MHal_SATA_get_max_speed();
    writew(u16Temp, (volatile void *)port_base + (0x2c << 1));
    u16Temp = readw((volatile void *)port_base + (0x2c << 1));
    printk("MAC SATA SPEED= 0x%x\n", u16Temp);

    if((u16Temp & E_PORT_SPEED_MASK) == E_PORT_SPEED_GEN1)
        phy_mode = 0;
    else if((u16Temp & E_PORT_SPEED_MASK) == E_PORT_SPEED_GEN2)
        phy_mode = 1;
    else
        phy_mode = 2;

    //printk("sata hal PHY init !!!!!!\n");
    //printk("Gen1 Tx swing\n");
    writew(0x2031, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x94) << 1));
    writew(0x3803, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x96) << 1));
    // EQ BW
    //printk("Gen1 EQ BW\n");
    u16Temp = readw((volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x92) << 1));
    u16Temp &= ~0x0060;
    u16Temp |= 0x0020;  // default is 0x01
    writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x92) << 1));
    // EQ STR
    //printk("Gen1 EQ STR\n");
    u16Temp = readw((volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x92) << 1));
    u16Temp &= ~0x001f;
    u16Temp |= 0x0003;  // default is 0x03
    writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x92) << 1));

    // Gen2
    // Tx swing
    //printk("Gen2 Tx swing\n");
    writew(0x3935, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x2c) << 1));
    writew(0x3920, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x2e) << 1));
    // EQ BW
    //printk("Gen2 EQ BW\n");
    u16Temp = readw((volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x50) << 1));
    u16Temp &= ~0x1800;
    u16Temp |= 0x0800;  // default is 0x01
    writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x50) << 1));
    // EQ STR
    //printk("Gen2 EQ STR\n");
    u16Temp = readw((volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x26) << 1));
    u16Temp &= ~0x001f;
    u16Temp |= 0x0003;  // default is 0x03
    writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x26) << 1));

    // Gen3
    // Tx swing
    //printk("Gen3 Tx swing\n");
    writew(0x3937, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x9a) << 1));
    writew(0x3921, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x9c) << 1));
    // EQ BW
    //printk("Gen3 EQ BW\n");
    u16Temp = readw((volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x98) << 1));
    u16Temp &= ~0x0060;
    u16Temp |= 0x0000;  // default is 0x00
    writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x98) << 1));
    // EQ STR
    //printk("Gen3 EQ EQ STR\n");
    //printk("EQ BW\n");
    u16Temp = readw((volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x98) << 1));
    u16Temp &= ~0x001f;
    u16Temp |= 0x0003;  // default is 0x03
    writew(u16Temp, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x98) << 1));

    // SSC setup
#if 1  // new settting to fixed PMP mode issue with JMB575
    // STEP1[10:0]
    writew(0x2002, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x64) << 1));  // [12]: 0 Stop sequence mode, 1: Reverse sequence mode
    // SET[15:0]
    writew(0x0aff, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x60) << 1));
    // SET[23:16]
    writeb(0x17, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x62) << 1));
    // SPAN[14:0]
    writew(0x0495, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x66) << 1));
#else
    writew(0x3007, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x64) << 1));
    writew(0xdf0a, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x60) << 1));
    writeb(0x24, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x62) << 1));
    writew(0x02e8, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x66) << 1));
#endif


    //printk("sata SATA PHY OOB setting ..\n");
    writew(0x96de, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));
    //writew(0x031e, SSTAR_RIU_BASE + (0x103934 << 1));
    writew(0x0f20, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x06) << 1));
    writew(0x341f, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x08) << 1));

    //printk("sata SATA PHY setting ..\n");
    //asm volatile( "loop: b loop");

#if 1  // new setting
    writew(0x0000, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x22) << 1));
    writew(0x0000, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x24) << 1));
    //writew(0x0a3d, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x60) << 1));
    //writeb(0x17, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x62) << 1));
    writew(0x9103, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x0c) << 1));
    writeb(0x06, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x3e) << 1));
    writew(0x0000, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x10) << 1));  // Enable clocks
#if 0  //
    writeb(0x0f, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x0b) << 1) - 1);  // [10]: OOB pattern select
    writew(0x848f, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));
    writew(0x848e, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));
#else
    writeb(0x0b, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x0b) << 1) - 1);  // [10]: OOB pattern select
    writew(0x9a8f, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));  // [0]: Reset
    writew(0x9a8e, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));
#endif
    writew(0x831e, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x68) << 1));
    writew(0x031e, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0xc0) << 1));
    writew(0x031e, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0xc2) << 1));
    writeb(0x30, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x19) << 1) - 1);
    //writew(0x9110, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x2c) << 1));  // new add
    writeb(0x80, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x3c) << 1));
    writeb(0x50, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x33) << 1) - 1);  // [12]: reg_force_tx_oob_pat

    if(phy_mode == 1)  // Gen2
        writew(0x9114, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x2c) << 1));// for Gen2
    else
        writew(0x9115, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x2c) << 1));// for Gen3 and Gen1

    if(phy_mode == 0)  // for Gen1
    {
        writew(0x080e, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x94) << 1));
        writew(0x1115, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x9a) << 1));
        writew(0x1108, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x9c) << 1));
        // Override speed
        writeb(0x00, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x25) << 1) - 1);  // 0x00:Gen1, 0x50:Gen2, 0xA0:Gen3
    }
    else if(phy_mode == 1)  // for Gen2
    {
        writew(0x0811, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x94) << 1));
        writew(0x1115, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x9a) << 1));
        writew(0x1108, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x9c) << 1));
        // Override speed
        writeb(0x50, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x25) << 1) - 1);  // 0x00:Gen1, 0x50:Gen2, 0xA0:Gen3
    }
    else  // for Gen3
    {
        writew(0x0811, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x94) << 1));
        writew(0x111a, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x9a) << 1));
        writew(0x110a, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x9c) << 1));
        // Override speed
        writeb(0xA0, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x25) << 1) - 1);  // 0x00:Gen1, 0x50:Gen2, 0xA0:Gen3
    }

    writew(0x2200, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x0e) << 1));  // [9]: Enable host

#else

    writew(0x0000, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x22) << 1));
    writew(0x0000, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x24) << 1));
    writew(0x0a3d, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x60) << 1));
    writeb(0x17, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x62) << 1));
    writew(0x9103, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x0c) << 1));
    writeb(0x06, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x3e) << 1));
    writew(0x0000, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x10) << 1));  // Enable clocks
    writeb(0x0b, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x0b) << 1) - 1);  // [10]: OOB pattern select
    writew(0x9a8f, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));  // [0]: Reset
    writew(0x9a8e, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));
    writew(0x831e, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x68) << 1));
    writew(0x031e, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0xc0) << 1));
    writew(0x031e, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0xc2) << 1));
    writeb(0x30, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x19) << 1) - 1);
    //writew(0x9110, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x2c) << 1));  // new add
    writeb(0x80, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x3c) << 1));
    writeb(0x50, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x33) << 1) - 1);  // [12]: reg_force_tx_oob_pat
    writew(0x2200, (volatile void *)SSTAR_RIU_BASE + ((GHC_PHY | 0x0e) << 1));  // [9]: Enable host

#endif

    writew(0x0001, (volatile void *) hba_base + SS_HOST_CTL); // reset HBA

}
//EXPORT_SYMBOL_GPL(MHal_SATA_HW_Inital);

static void MHal_SATA_Setup_Port_Implement(phys_addr_t misc_base, phys_addr_t port_base, phys_addr_t hba_base)
{
    //  Init FW to trigger controller
    writew(0x0000, (volatile void *)hba_base + (SS_HOST_CAP));
    writew(0x0000, (volatile void *)hba_base + (SS_HOST_CAP + 0x4));

    // Port Implement
    writew(0x0001, (volatile void *)hba_base + (SS_HOST_PORTS_IMPL));
    writew(0x0000, (volatile void *)port_base + (SS_PORT_CMD));
    writew(0x0000, (volatile void *)port_base + (SS_PORT_CMD + 0x4));
}
//EXPORT_SYMBOL_GPL(MHal_SATA_Setup_Port_Implement);

static phys_addr_t MHal_SATA_bus_address(phys_addr_t phy_address)
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
