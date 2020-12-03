/*
* mhal_sata_bench_test.c- Sigmastar
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
// * Copyright (c) 2006 - 2007 MStar Semiconductor, Inc.
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
/// file    mhal_sata_host.c
/// @brief  SATA Host Hal Driver
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <linux/kernel.h>
#include <asm/io.h>
#include <mhal_sata_bench_test.h>
#include <linux/delay.h>
#include "ms_platform.h"
extern ptrdiff_t mstar_pm_base;
#define sata_reg_write16(val, addr) { (*((volatile unsigned short*)(addr))) = (unsigned short)(val); }
#define sata_reg_write8(val, addr) { (*((volatile unsigned char*)(addr))) = (unsigned char)(val);}
#define GET_REG8_ADDR(x, y)     (x+(y)*2)
#define GET_REG16_ADDR(x, y)    (x+(y)*4)
#define RIU_BASE_ADDR 0xFD000000

#define _PK_L_(bank, addr)   ((((u32)bank) << 8) | (u32)((addr)*2))

void HAL_SATA_INFO_dump_Reg(u32 BankNo, u16 RegOffset, u32 len)
{
    u32 addr, l_offset;
    u8 l_buf[128], token_buf[8];
    u32 start;

    start = _PK_L_(BankNo, RegOffset);

    for (addr = start, l_offset = 0; addr < (start + (len * 2)); addr += 2)
    {
        if ((addr & 0xFF) == 0)
        {
            printk(KERN_INFO "BK x%04X :", (addr >> 8));
        }
        if ((addr & 0xF) == 0)
        {
            l_offset = 0;
            snprintf(token_buf, 8, "%02X:", ((0xFF & addr) >> 1));
            strcpy(l_buf + l_offset, token_buf);
            l_offset += strlen(token_buf);
        }

        snprintf(token_buf, 8, " %04X", readw((volatile void *)MSTAR_RIU_BASE + (addr << 1)));
        strcpy(l_buf + l_offset, token_buf);
        l_offset += strlen(token_buf);

        if ((addr & 0xF) == 0xE)
        {
            printk(KERN_INFO "%s\n", l_buf);
        }
    }
    printk(KERN_INFO "Dump End\n");
}


int MHal_SATA_LoopBack_Test(u8 port_base, u8 gen_num)
{
    u16 u16Temp;
    u32 GHC_PHY = 0x0;
    u8 u8Temp;
    u32 timeout = 0;
    //u32 GHC_CLK;
    // unsigned volatile short debreg;
    //unsigned volatile char debreg_b;
    int bTestResult = 0;

    //printk("MHal_SATA_LoopBack_Test: port = %d, gen = %d\n", port_base, gen_num);

    if((port_base >= 2) || ((gen_num == 0) || (gen_num >= 4)))
    {
        printk("Invalid port base(%d) and gen number(%d).\n", port_base, gen_num);
        return 0;
    }

    if(port_base == 0)
        GHC_PHY = SATA_GHC_0_PHY;//0x103900
    else if(port_base == 1)
        GHC_PHY = SATA_GHC_1_PHY;//0x162A00

    // Enable MAC Clock, bank is chip dependent
    writew(0x0c0c, (volatile void *)MSTAR_RIU_BASE + (0x100b64 << 1)); // [0] gate


    writew(0x0000, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x10) << 1));
    writeb(0x50, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x33) << 1) - 1);

    //Syhtnesizer setup
    writew(0x0a3d, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x60) << 1));
    writeb(0x17, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x62) << 1));

    //TX loopback RX
    writew(0x0000, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x22) << 1));
    writew(0x0000, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x24) << 1));

    // Override speed
    if(gen_num == 1)  // for Gen1
    {
        writeb(0x00, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x25) << 1) - 1);  // 0x00:Gen1, 0x50:Gen2, 0xA0:Gen3
    }
    else if(gen_num == 2)  // for Gen2
    {
        writeb(0x50, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x25) << 1) - 1);  // 0x00:Gen1, 0x50:Gen2, 0xA0:Gen3
    }
    else  // for Gen3
    {
        writeb(0xA0, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x25) << 1) - 1);  // 0x00:Gen1, 0x50:Gen2, 0xA0:Gen3
    }

    writeb(0x03, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x20) << 1));

    writeb(0x8c, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x03) << 1) - 1);
    writew(0x9103, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0c) << 1));
    writeb(0x06, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x3e) << 1));

    timeout = 0;
    //Wait TXPLL lock (300us) => Bit 5 = 1 (when lock)
    // Detection clock is stable or not. 0: No, 1: Yes
    do
    {
        msleep_interruptible(1);  // sleep 1 ms
        u8Temp = readb((volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0d) << 1) - 1);
    }
    while(((u8Temp & 0x20) == 0) && (timeout++ <= 100));
    if(timeout >= 100)
    {
        printk("[Port%d][Gen%d]Wait TXPLL lock Time out!!!!!!\n", port_base, gen_num);
    }

    //Read TXPLL frequency count => 13'h31c +/- 2, Detection clock count [13:0]
    u16Temp = readw((volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x1a) << 1));
    u16Temp &= 0x3fff;
    //printk("[Port%d][Gen%d]TXPLL frequency count  = 0x%x\n", port_base, gen_num, u16Temp);


    writeb(0x83, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x7c) << 1));
    writew(0x5555, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x34) << 1));
    writeb(0x05, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x36) << 1));
    writeb(0x00, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x15) << 1) - 1);

    //Refresh clock stable
    writeb(0x38, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0f) << 1) - 1);
    writeb(0x40, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x13) << 1) - 1);
    writeb(0x88, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x03) << 1) - 1);
    writew(0x5183, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x32) << 1));

    timeout = 0;
    //Wait RXPLL lock (300us) => Bit 5 (when lock)
    // Detection clock is stable or not. 0: No, 1: Yes
    do
    {
        msleep_interruptible(1);  // sleep 1 ms
        u8Temp = readb((volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0d) << 1) - 1);
    }
    while(((u8Temp & 0x20) == 0) && (timeout++ <= 100));
    if(timeout >= 100)
    {
        printk("[Port%d][Gen%d]Wait RXPLL lock Time out!!!!!!\n", port_base, gen_num);
    }

    //Read RXPLL frequency count => 13'h31c +/- 2, Detection clock count [13:0]
    u16Temp = readw((volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x1a) << 1));
    u16Temp &= 0x3fff;
    //printk("[Port%d][Gen%d]RXPLL frequency count  = 0x%x\n", port_base, gen_num, u16Temp);

    //[4]: Enable PRBS7 pattern
    writeb(0x50, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x13) << 1) - 1);
    msleep_interruptible(1);  // sleep 1 ms

    //TE: Check Error count valid or not => Bit 5 = 1 or not
    //If fail	 bad phase and ignore error count check
    u8Temp = readb((volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x13) << 1) - 1);
    //printk("[Port%d][Gen%d]RX PRBS7 error count[5] = 0x%x\n", port_base, gen_num, u8Temp);
    if(u8Temp & 0x20)
    {
        bTestResult = 1;
        printk("================  [Port%d][Gen%d]RX PRBS7 loopback check success ================\n", port_base, gen_num);
    }
    else
    {
        bTestResult = 0;
        printk("!!!!!!!!!!!!!!!!!!!! [Port%d][Gen%d]RX PRBS7 loopback check error!!!!!!!!!!!!!!!!!!!!!!\n", port_base, gen_num);
    }

    u16Temp = readw((volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x16) << 1));
    //printk("[Port%d][Gen%d]RX PRBS7 error count (20b) = 0x%x\n", port_base, gen_num, u16Temp);

    //HAL_SATA_INFO_dump_Reg((GHC_PHY >> 8), 0, 0x80);

    return bTestResult;
}
EXPORT_SYMBOL_GPL(MHal_SATA_LoopBack_Test);

int MHal_SATA_Tx_Test_Phy_Initialize(u8 port_base, u8 gen_num)
{
    u32 GHC_PHY = 0x0;
    u8 u8Temp;
    u32 timeout = 0;
    int bTestResult = 0;

    //printk("[MHal_SATA_Tx_Test_Phy_Initialize]: port = %d, gen = %d\n", port_base, gen_num);

    if((port_base >= 2) || ((gen_num == 0) || (gen_num >= 4)))
    {
        printk("Invalid port base(%d) and gen number(%d).\n", port_base, gen_num);
        return 0;
    }

    // Enable MAC Clock, bank is chip dependent
    writew(0x0c0c, (volatile void *)MSTAR_RIU_BASE + (0x100b64 << 1)); // [0] gate

    if(port_base == 0)
        GHC_PHY = SATA_GHC_0_PHY;//0x103900
    else if(port_base == 1)
        GHC_PHY = SATA_GHC_1_PHY;//0x162A00

    // Initial (If initial code is load ready, ignore it)
    writew(0x0000, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x22) << 1));
    writew(0x0000, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x24) << 1));
    writew(0x8104, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0c) << 1));
    writew(0x2200, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0e) << 1));
    writew(0x0000, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x10) << 1));
    writeb(0x0f, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0b) << 1) - 1);

    writew(0x848f, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));
    writew(0x848e, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x02) << 1));
    writeb(0x20, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x19) << 1) - 1);

    if(gen_num == 1)  // for Gen1
    {
        writew(0x9115, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x2c) << 1));
    }
    else if(gen_num == 2)  // for Gen2
    {
        writew(0x9114, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x2c) << 1));
    }
    else  // for Gen3
    {
        writew(0x9115, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x2c) << 1));
    }

    writeb(0x50, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x33) << 1) - 1);
    writeb(0x03, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x98) << 1));

    writew(0xc023, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x92) << 1));
    writeb(0x03, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x26) << 1));

    writew(0x5e03, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x98) << 1));

    if(gen_num == 1)  // for Gen1
    {
        writew(0x0811, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x94) << 1));
        writew(0x1115, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x9a) << 1));
        writew(0x1108, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x9c) << 1));
    }
    else if(gen_num == 2)  // for Gen2
    {
        writew(0x0811, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x94) << 1));
        writew(0x1115, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x9a) << 1));
        writew(0x1108, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x9c) << 1));
    }
    else  // for Gen3
    {
        writew(0x0811, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x94) << 1));
        writew(0x111a, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x9a) << 1));
        writew(0x110a, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x9c) << 1));
    }

    writew(0x1001, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x96) << 1));
    writew(0x1108, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x2e) << 1));
    writew(0x50c0, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x30) << 1));
    writeb(0x58, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x43) << 1) - 1);
    writeb(0x0b, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x7f) << 1) - 1);
    writew(0x5003, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x32) << 1));
    writeb(0x08, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x4d) << 1) - 1);
    // Initial end

    // Synthesizer setup
    writew(0x0aff, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x60) << 1));
    writeb(0x17, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x62) << 1));


    writeb(0x03, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x20) << 1));
    // Override speed
    if(gen_num == 1)  // for Gen1
    {
        writeb(0x00, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x25) << 1) - 1);  // 0x00:Gen1, 0x50:Gen2, 0xA0:Gen3
    }
    else if(gen_num == 2)  // for Gen2
    {
        writeb(0x50, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x25) << 1) - 1);  // 0x00:Gen1, 0x50:Gen2, 0xA0:Gen3
    }
    else  // for Gen3
    {
        writeb(0xA0, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x25) << 1) - 1);  // 0x00:Gen1, 0x50:Gen2, 0xA0:Gen3
    }

    writeb(0x3a, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0f) << 1) - 1);

    bTestResult = 1;
    timeout = 0;
    //Wait TXPLL lock (300us) => Bit 5 = 1 (when lock)
    // Detection clock is stable or not. 0: No, 1: Yes
    do
    {
        msleep_interruptible(1);  // sleep 1 ms
        u8Temp = readb((volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x0d) << 1) - 1);
    }
    while(((u8Temp & 0x20) == 0) && (timeout++ <= 100));
    if(timeout >= 100)
    {
        printk("[Port%d][Gen%d]Wait TXPLL lock Time out!!!!!!\n", port_base, gen_num);
        return 0;
    }

    // Switch to test mode
    writeb(0xe9, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x7c) << 1));

    return bTestResult;
}

void MHal_SATA_Tx_Test_SSC(u8 port_base, u8 bEnable)
{
    u32 GHC_PHY = 0x0;

    if(port_base >= 2)
    {
        printk("Invalid port base(%d).\n", port_base);
        return;
    }

    if(port_base == 0)
        GHC_PHY = SATA_GHC_0_PHY;//0x103900
    else if(port_base == 1)
        GHC_PHY = SATA_GHC_1_PHY;//0x162A00

    if(bEnable)
    {
        // SSC setup
        // STEP1[10:0]
        writew(0x2002, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x64) << 1));  // [12]: 0 Stop sequence mode, 1: Reverse sequence mode
        // SET[15:0]
        writew(0x0aff, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x60) << 1));
        // SET[23:16]
        writeb(0x17, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x62) << 1));
        // SPAN[14:0]
        writew(0x0495, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x66) << 1));
    }
    else
    {
        // SSC setup
        // STEP1[10:0]
        writew(0x0000, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x64) << 1));  // [12]: 0 Stop sequence mode, 1: Reverse sequence mode
    }

}

void MHal_SATA_Tx_Test_Pattern_HFTP(u8 port_base)
{
    u32 GHC_PHY = 0x0;

    if(port_base >= 2)
    {
        printk("Invalid port base(%d).\n", port_base);
        return;
    }

    if(port_base == 0)
        GHC_PHY = SATA_GHC_0_PHY;//0x103900
    else if(port_base == 1)
        GHC_PHY = SATA_GHC_1_PHY;//0x162A00

    // Program pattern as HFTP
    writew(0x4a4a, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x34) << 1));
    writew(0x4a4a, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x36) << 1));
    writew(0x4a4a, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x38) << 1));
    writew(0x4a4a, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x3a) << 1));

}

void MHal_SATA_Tx_Test_Pattern_MFTP(u8 port_base)
{
    u32 GHC_PHY = 0x0;

    if(port_base >= 2)
    {
        printk("Invalid port base(%d).\n", port_base);
        return;
    }

    if(port_base == 0)
        GHC_PHY = SATA_GHC_0_PHY;//0x103900
    else if(port_base == 1)
        GHC_PHY = SATA_GHC_1_PHY;//0x162A00

    // Program pattern as MFTP
    writew(0x7878, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x34) << 1));
    writew(0x7878, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x36) << 1));
    writew(0x7878, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x38) << 1));
    writew(0x7878, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x3a) << 1));

}

void MHal_SATA_Tx_Test_Pattern_LFTP(u8 port_base)
{
    u32 GHC_PHY = 0x0;

    if(port_base >= 2)
    {
        printk("Invalid port base(%d).\n", port_base);
        return;
    }

    if(port_base == 0)
        GHC_PHY = SATA_GHC_0_PHY;//0x103900
    else if(port_base == 1)
        GHC_PHY = SATA_GHC_1_PHY;//0x162A00

    // Program pattern as LFTP
    writew(0x7e7e, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x34) << 1));
    writew(0x7e7e, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x36) << 1));
    writew(0x7e7e, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x38) << 1));
    writew(0x7e7e, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x3a) << 1));

}

void MHal_SATA_Tx_Test_Pattern_LBP(u8 port_base)
{
    u32 GHC_PHY = 0x0;

    if(port_base >= 2)
    {
        printk("Invalid port base(%d).\n", port_base);
        return;
    }

    if(port_base == 0)
        GHC_PHY = SATA_GHC_0_PHY;//0x103900
    else if(port_base == 1)
        GHC_PHY = SATA_GHC_1_PHY;//0x162A00

    // Program pattern as LBP
    writew(0x8b0c, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x34) << 1));
    writew(0x6b0c, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x36) << 1));
    writew(0x8b0c, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x38) << 1));
    writew(0x6b0c, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x3a) << 1));

}

void MHal_SATA_Tx_Test_Pattern_SSOP(u8 port_base)
{
    u32 GHC_PHY = 0x0;

    if(port_base >= 2)
    {
        printk("Invalid port base(%d).\n", port_base);
        return;
    }

    if(port_base == 0)
        GHC_PHY = SATA_GHC_0_PHY;//0x103900
    else if(port_base == 1)
        GHC_PHY = SATA_GHC_1_PHY;//0x162A00

    // Program pattern as SSOP
    writew(0x7f7f, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x34) << 1));
    writew(0x7f7f, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x36) << 1));
    writew(0x7f7f, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x38) << 1));
    writew(0x7f7f, (volatile void *)MSTAR_RIU_BASE + ((GHC_PHY | 0x3a) << 1));

}
