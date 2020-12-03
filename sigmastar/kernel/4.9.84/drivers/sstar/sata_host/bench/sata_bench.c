/*
* sata_bench.c- Sigmastar
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
#include <linux/module.h>
#include "ahci.h"
#include "cam_os_wrapper.h"

//==============================================================================
//
//                              GLOBAL VARIABLES
//
//==============================================================================
static unsigned int comreset_interval_us = 300;
module_param(comreset_interval_us, uint, 0);

static unsigned int comreset_repeat_times = 0;
module_param(comreset_repeat_times, uint, 0);

static unsigned int sata_preset = 1;
module_param(sata_preset, uint, 0);


//==============================================================================
//
//                              DEFINES
//
//==============================================================================
#define SSTAR_RIU_BASE      0xFD000000
#define SSTAR_HBA_BASE      (SSTAR_RIU_BASE+0x345000)
#define SSTAR_PORT_BASE     (SSTAR_RIU_BASE+0x345100)

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

CamOsThread tBenchTaskHandle;

static void ss_sata_preset(void)
{
    writew(0x0000,  (volatile void *)SSTAR_RIU_BASE + ((0x152500 + (0x00 << 1)) << 1));
    writew(0x0000,  (volatile void *)SSTAR_RIU_BASE + ((0x103800 + (0x6E << 1)) << 1));
    writew(0x0000,  (volatile void *)SSTAR_RIU_BASE + ((0x103800 + (0x6C << 1)) << 1));
    writew(0x0000,  (volatile void *)SSTAR_RIU_BASE + ((0x103800 + (0x46 << 1)) << 1));
    writew(0x0100,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x20 << 1)) << 1));
    writew(0x1008,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x30 << 1)) << 1));
    writew(0x0500,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x33 << 1)) << 1));
    writew(0x0001,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x05 << 1)) << 1));
    writew(0x0002,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x60 << 1)) << 1));
    writew(0x0062,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x70 << 1)) << 1));
    writew(0x8000,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x3E << 1)) << 1));
    writew(0x0002,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x04 << 1)) << 1));
    writew(0x0001,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x44 << 1)) << 1));
    writew(0x0000,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x44 << 1)) << 1));

    writew(0x440A,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x0A << 1)) << 1));
    writew(0x1905,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x26 << 1)) << 1));
    writew(0xB659,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x3E << 1)) << 1));
    writew(0xD819,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x40 << 1)) << 1));
    writew(0x4000,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x61 << 1)) << 1));
    writew(0x0044,  (volatile void *)SSTAR_RIU_BASE + ((0x152700 + (0x64 << 1)) << 1));
}

static void ss_sata_oob(void)
{
    writel(0x00000001, (volatile void *)SSTAR_HBA_BASE + (HOST_CTL));
    writel(0x00000000, (volatile void *)SSTAR_HBA_BASE + (HOST_CAP));
    writel(0x00000001, (volatile void *)SSTAR_HBA_BASE + (HOST_PORTS_IMPL));
    writel(0x00000001, (volatile void *)SSTAR_PORT_BASE + (PORT_SCR_CTL));
    writel(0x00000000, (volatile void *)SSTAR_PORT_BASE + (PORT_SCR_CTL));
    writel(0x02100000, (volatile void *)SSTAR_PORT_BASE + (PORT_LST_ADDR));
    writel(0x02000000, (volatile void *)SSTAR_PORT_BASE + (PORT_FIS_ADDR));
    writel(0x00000016, (volatile void *)SSTAR_PORT_BASE + (PORT_CMD));
}

static void ss_sata_bench(void)
{
    unsigned int counter=0;
    int i;

    if (sata_preset)
        ss_sata_preset();

    if (!comreset_repeat_times)
    {
        while (1)
        {
            ss_sata_oob();
            udelay(comreset_interval_us);
            if (CAM_OS_OK == CamOsThreadShouldStop())
                break;
        }
    }
    else
    {
        while (counter < comreset_repeat_times)
        {
            ss_sata_oob();
            udelay(comreset_interval_us);
            if (CAM_OS_OK == CamOsThreadShouldStop())
                break;
            counter++;
        }
    }
}

static int __init ss_sata_bench_init(void)
{
    CamOsThreadAttrb_t tAttr = {0};

    CamOsPrintf("[SATA Bench] Init()\r\n");

    tAttr.nPriority = 50;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&tBenchTaskHandle, &tAttr, (void *)ss_sata_bench, NULL);

    return 0;
}

static void __exit ss_sata_bench_exit(void)
{
    CamOsPrintf("[SATA Bench] Remove()\r\n");

    CamOsThreadStop(tBenchTaskHandle);

    return;
}
module_init(ss_sata_bench_init);
module_exit(ss_sata_bench_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SIGMASTAR");
MODULE_DESCRIPTION("SATA BENCH");
