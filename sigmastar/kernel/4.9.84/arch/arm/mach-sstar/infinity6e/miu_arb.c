/*
* miu_arb.c- Sigmastar
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
#include <linux/types.h>
#include <linux/init.h>
#include <linux/irqchip.h>
#include <linux/of_platform.h>
#include <linux/of_fdt.h>
#include <linux/sys_soc.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/clocksource.h>
#include <linux/gpio.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/memory.h>
#include <asm/io.h>
#include <asm/mach/map.h>
#include "gpio.h"
#include "registers.h"
#include "mcm_id.h"
#include "ms_platform.h"
#include "ms_types.h"
#include "_ms_private.h"

#include <linux/module.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/buffer_head.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include "cam_os_wrapper.h"
#include "miu_bw.h"

/*=============================================================*/
// Structure definition
/*=============================================================*/

struct miu_arb_handle {
    char name[12];          // device name (miu_arbx)
    char dump;              // dump mode: readable text, register table
    char group_selected;    // group selected, 0~(MIU_GRP_NUM-1)
    char client_selected;   // client selected, 1~(MIU_CLIENT_NUM-2)
};

struct miu_arb_grp_reg {
    u16 cfg;                // inner group arbitration config
        #define REG_IGRP_CFG(g)         (REG_ID_20 + BK_REG(0x10*g))
        #define IGCFG_ROUND_ROBIN_BIT   0x0001 // bit 0
        #define IGCFG_FIX_PRIO_BIT      0x0002 // bit 1
        #define IGCFG_MBR_LIMIT_EN_BIT  0x0004 // bit 2
        #define IGCFG_GRP_LIMIT_EN_BIT  0x0008 // bit 3
        #define IGCFG_FCTL0_EN_BIT      0x0100 // bit 8
        #define IGCFG_FCTL1_EN_BIT      0x0200 // bit 9
    u16 burst;              // bit[7:0] member burst length; bit[15:8] group burst length
        #define REG_IGRP_BURST(g)       (REG_ID_21 + BK_REG(0x10*g))
        #define IGBURST_MBR_SHIFT       0
        #define IGBURST_MBR_MASK        0x00FF
        #define IGBURST_GRP_SHIFT       8
        #define IGBURST_GRP_MASK        0xFF00
    u16 flowctrl0;
        #define REG_IGRP_FCTL0(g)       (REG_ID_2A + BK_REG(0x10*g))
        #define IGFCTL_ID0_SHIFT        0
        #define IGFCTL_ID0_MASK         0x000F
        #define IGFCTL_ID1_SHIFT        4
        #define IGFCTL_ID1_MASK         0x00F0
        #define IGFCTL_PERIOD_SHIFT     8
        #define IGFCTL_PERIOD_MASK      0xFF00
    u16 flowctrl1;
        #define REG_IGRP_FCTL1(g)       (REG_ID_2B + BK_REG(0x10*g))
    u16 mbr_priority;
        #define REG_IGRP_MBR_PRIO(g)    (REG_ID_2C + BK_REG(0x10*g))
        #define IGMBR_PRIO(mbr,pri)     (pri << mbr)
        #define IGMBR_PRIO_MASK(mbr)    (1 << mbr)
        #define IGMBR_PRIO_SHIFT(mbr)   (mbr)
    u16 mbr_nolimit;
        #define REG_IGRP_MBR_NOLIMIT(g) (REG_ID_2E + BK_REG(0x10*g))
        #define IGMBR_NOLIMIT_EN(mbr)   (1 << mbr)
};

struct miu_arb_reg {
    struct miu_arb_grp_reg grp[MIU_GRP_NUM];
    u16 cfg;                // outer groups arbitration config
        #define REG_OGRP_CFG            (REG_ID_7F)
        #define OGCFG_GRP_PRIO_SHIRT(p) (p << 1)
        #define OGCFG_GRP_PRIO_MASK(p)  (0x3 << (p << 1))
        #define OGCFG_FIX_PRIO_EN       0x0100  // bit 8
        #define OGCFG_ROUND_ROBIN_EN    0x1000  // bit 12
};

/*=============================================================*/
// Local variable
/*=============================================================*/

static struct miu_device arb_dev[MIU_NUM];
static struct miu_arb_handle arb_handle[MIU_NUM];
// default policy
static struct miu_arb_reg arb_policy_def = {
    {
        {   // group 0
            .cfg = 0x801D,
            .burst = 0x1008,
            .flowctrl0 = 0x0000,
            .flowctrl1 = 0x0000,
            .mbr_priority = 0x0000,
            .mbr_nolimit = 0x0000
        },
        {   // group 1
            .cfg = 0x801D,
            .burst = 0x2010,
            .flowctrl0 = 0x0000,
            .flowctrl1 = 0x0000,
            .mbr_priority = 0x0000,
            .mbr_nolimit = 0x0000
        },
        {   // group 2
            .cfg = 0x801D,
            .burst = 0x2010,
            .flowctrl0 = 0x0000,
            .flowctrl1 = 0x0000,
            .mbr_priority = 0x0000,
            .mbr_nolimit = 0x0000
        },
    },
    .cfg = 0x9000,
};

// BGA2 default policy
static struct miu_arb_reg arb_policy_bga2_def = {
    {
        {   // group 0
            .cfg = 0x800D,
            .burst = 0x2010,
            .flowctrl0 = 0x0000,
            .flowctrl1 = 0x0000,
            .mbr_priority = 0x0000,
            .mbr_nolimit = 0x0000
        },
        {   // group 1
            .cfg = 0x800D,
            .burst = 0x2010,
            .flowctrl0 = 0x0000,
            .flowctrl1 = 0x0000,
            .mbr_priority = 0x0180,
            .mbr_nolimit = 0x0000
        },
        {   // group 2
            .cfg = 0x800D,
            .burst = 0x2010,
            .flowctrl0 = 0x0000,
            .flowctrl1 = 0x0000,
            .mbr_priority = 0x0000,
            .mbr_nolimit = 0x0000
        },
    },
    .cfg = 0x9000,
};

static struct miu_arb_reg *arb_policies[MIU_ARB_POLICY_NUM] = {
    &arb_policy_def, &arb_policy_bga2_def,
};

/*=============================================================*/
// Local function
/*=============================================================*/
static int _get_reg_base(int miu)
{
    if (miu == 0) {
        return BASE_REG_MIU_PA;
    }
    printk(KERN_ERR "MIU%d reg base not assigend\n", miu);
    return 0;
}

static void _load_group_settings(struct miu_arb_grp_reg *greg, int base, int g)
{
    if (greg) {
        greg->cfg = INREG16(base+REG_IGRP_CFG(g));
        greg->burst = INREG16(base+REG_IGRP_BURST(g));
        greg->flowctrl0 = INREG16(base+REG_IGRP_FCTL0(g));
        greg->flowctrl1 = INREG16(base+REG_IGRP_FCTL1(g));
        greg->mbr_priority = INREG16(base+REG_IGRP_MBR_PRIO(g));
        greg->mbr_nolimit = INREG16(base+REG_IGRP_MBR_NOLIMIT(g));
    }
}

static void _load_policy(int miu, int idx)
{
    int base;
    int g;
    struct miu_arb_reg *reg;

    if (idx >= MIU_ARB_POLICY_NUM) {
        return;
    }

    base = _get_reg_base(miu);
    if (!base) {
        return;
    }

    reg = arb_policies[idx];
    for(g = 0; g < MIU_GRP_NUM; g++)
    {
        OUTREG16(base+REG_IGRP_CFG(g), reg->grp[g].cfg);
        OUTREG16(base+REG_IGRP_BURST(g), reg->grp[g].burst);
        OUTREG16(base+REG_IGRP_FCTL0(g), reg->grp[g].flowctrl0);
        OUTREG16(base+REG_IGRP_FCTL1(g), reg->grp[g].flowctrl1);
        OUTREG16(base+REG_IGRP_MBR_PRIO(g), reg->grp[g].mbr_priority);
        OUTREG16(base+REG_IGRP_MBR_NOLIMIT(g), reg->grp[g].mbr_nolimit);
    }
    OUTREG16((base+REG_OGRP_CFG), reg->cfg);
}

static char *_dump_as_text(char *str, char *end, unsigned char miu)
{
    int g, mbr, c, base;
    u16 outer_cfg, mbr_burst, mbr_prio;
    struct miu_arb_grp_reg greg;

    base = _get_reg_base(miu);
    if (!base) {
        return str;
    }

    outer_cfg = INREG16(base+REG_OGRP_CFG);
    for(g = 0; g < MIU_GRP_NUM; g++)
    {
        /* Group settings */
        _load_group_settings(&greg, base, g);
        str += scnprintf(str, end - str, ASCII_COLOR_BLUE"==== Group %d ================================\n"ASCII_COLOR_END, g);
        str += scnprintf(str, end - str, " Outer-Prio: ");
        if (outer_cfg & OGCFG_ROUND_ROBIN_EN) {
            str += scnprintf(str, end - str, "Round-Robin\n");
        }
        else if (outer_cfg & OGCFG_FIX_PRIO_EN) {
            int p = 0;
            while(p < MIU_ARB_OG_PRIO_NUM) {
                if (((outer_cfg & OGCFG_GRP_PRIO_MASK(p)) >> OGCFG_GRP_PRIO_MASK(p)) == g) {
                    break;
                }
                p++;
            };
            str += scnprintf(str, end - str, "%d\n", p);
        }
        str += scnprintf(str, end - str, " Inner-Prio: %s\n", (greg.cfg & IGCFG_ROUND_ROBIN_BIT) ?
                                                         "Round-Robin" :
                                                         "Fix priority");
        str += scnprintf(str, end - str, " Burst     : %d\n", (greg.cfg & IGCFG_GRP_LIMIT_EN_BIT) ?
                                                         (greg.burst & IGBURST_GRP_MASK) >> IGBURST_GRP_SHIFT :
                                                         0); // no limit
        str += scnprintf(str, end - str, " FlowCtrl0 : ");
        if (greg.cfg & IGCFG_FCTL0_EN_BIT) {
            c = (g * MIU_GRP_CLIENT_NUM) + ((greg.flowctrl0 & IGFCTL_ID0_MASK) >> IGFCTL_ID0_SHIFT);
            if (!miu_client_reserved(c)) {
                str += scnprintf(str, end - str, "%s, ", miu_client_id_to_name(c));
            }
            c = (g * MIU_GRP_CLIENT_NUM) + ((greg.flowctrl0 & IGFCTL_ID1_MASK) >> IGFCTL_ID1_SHIFT);
            if (!miu_client_reserved(c)) {
                str += scnprintf(str, end - str, "%s, ", miu_client_id_to_name(c));
            }
            str += scnprintf(str, end - str, "Preiod 0x02%X", (greg.flowctrl0 & IGFCTL_PERIOD_MASK) >> IGFCTL_PERIOD_SHIFT);
        }
        else {
            str += scnprintf(str, end - str, "None\n");
        }
        str += scnprintf(str, end - str, " FlowCtrl1 : ");
        if (greg.cfg & IGCFG_FCTL1_EN_BIT) {
            c = (g * MIU_GRP_CLIENT_NUM) + ((greg.flowctrl1 & IGFCTL_ID0_MASK) >> IGFCTL_ID0_SHIFT);
            if (!miu_client_reserved(c)) {
                str += scnprintf(str, end - str, "%s, ", miu_client_id_to_name(c));
            }
            c = (g * MIU_GRP_CLIENT_NUM) + ((greg.flowctrl1 & IGFCTL_ID1_MASK) >> IGFCTL_ID1_SHIFT);
            if (!miu_client_reserved(c)) {
                str += scnprintf(str, end - str, "%s, ", miu_client_id_to_name(c));
            }
            str += scnprintf(str, end - str, "Preiod 0x02%X", (greg.flowctrl1 & IGFCTL_PERIOD_MASK) >> IGFCTL_PERIOD_SHIFT);
        }
        else {
            str += scnprintf(str, end - str, "None\n");
        }

        /* Merber settings */
        str += scnprintf(str, end - str, ASCII_COLOR_BLUE"---------------------------------------------\n"ASCII_COLOR_END);
        str += scnprintf(str, end - str, "Id:Client\tBurst\tPriority\n");
        for (mbr = 0; mbr < MIU_GRP_CLIENT_NUM; mbr++) {
            c = (g * MIU_GRP_CLIENT_NUM) + mbr;
            if (!miu_client_reserved(c)) {
                if ((greg.cfg & IGCFG_MBR_LIMIT_EN_BIT) && !(greg.mbr_nolimit & IGMBR_NOLIMIT_EN(mbr))) {
                    mbr_burst = (greg.burst & IGBURST_MBR_MASK) >> IGBURST_MBR_SHIFT;
                }
                else {
                    mbr_burst = 0; // not limit
                }
                if (greg.cfg & IGCFG_ROUND_ROBIN_BIT) {
                    mbr_prio = (greg.mbr_priority & IGBURST_MBR_MASK) >> IGBURST_MBR_SHIFT;
                }
                else {
                    mbr_prio = 0xFF; // TODO: parse ID for fixed priority
                }
                str += scnprintf(str, end - str, "%2d:%s\t%d\t%d\n", c, miu_client_id_to_name(c), mbr_burst, mbr_prio);
            }
        }
    }

    return str;
}

static char *_dump_as_reg(char *str, char *end, unsigned char miu)
{
    int g;
    int base;
    struct miu_arb_grp_reg greg;

    base = _get_reg_base(miu);
    if (!base) {
        return str;
    }

    // miu0
    str += scnprintf(str, end - str, "    // miu%d\n", miu);
    str += scnprintf(str, end - str, "    {\n");

    for(g = 0; g < MIU_GRP_NUM; g++)
    {
        _load_group_settings(&greg, base, g);
        str += scnprintf(str, end - str, "        {   // group %d\n", g);
        str += scnprintf(str, end - str, "            .cfg = 0x%04X,\n", greg.cfg);
        str += scnprintf(str, end - str, "            .burst = 0x%04X,\n", greg.burst);
        str += scnprintf(str, end - str, "            .flowctrl0 = 0x%04X,\n", greg.flowctrl0);
        str += scnprintf(str, end - str, "            .flowctrl1 = 0x%04X,\n", greg.flowctrl1);
        str += scnprintf(str, end - str, "            .mbr_priority = 0x%04X,\n", greg.mbr_priority);
        str += scnprintf(str, end - str, "            .mbr_nolimit = 0x%04X,\n", greg.mbr_nolimit);
        str += scnprintf(str, end - str, "        },\n");
    }
    str += scnprintf(str, end - str, "    },\n");
    str += scnprintf(str, end - str, "    .cfg = 0x%04X,\n", INREG16(base+REG_OGRP_CFG));

    return str;
}

static ssize_t client_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    unsigned char m = MIU_IDX(dev->kobj.name[7]);
    u32 c = 0;

    c = simple_strtoul(buf, NULL, 10);
    if (c < MIU_ARB_CLIENT_NUM)
    {
        if (!miu_client_reserved(c)) {
            arb_handle[m].client_selected = c;
        }
        else {
            printk(KERN_ERR "Invalid client %d\n", c);
        }
    }
    else
    {
        printk(KERN_ERR "Invalid client %d\n", c);
        return count;
    }
    return count;
}

static ssize_t client_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int c, g, i;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    for(g = 0; g < MIU_GRP_NUM; g++)
    {
        str += scnprintf(str, end - str, "ID:IP_name\t");
    }
    str += scnprintf(str, end - str, "\n");

    for(i = 0; i < MIU_GRP_CLIENT_NUM; i++)
    {
        for(g = 0; g < MIU_GRP_NUM; g++)
        {
            c = (g * MIU_GRP_CLIENT_NUM) + i;
            if (c != arb_handle[m].client_selected)
            {
                if (!miu_client_reserved(c))
                {
                    str += scnprintf(str, end - str, "%2d:%s\t", c, miu_client_id_to_name(c));
                }
                else
                {
                    str += scnprintf(str, end - str, "%2d:       \t", c);
                }
            }
            else {
                str += scnprintf(str, end - str, ASCII_COLOR_GREEN"%3d:%s\t"ASCII_COLOR_END, c, miu_client_id_to_name(c));
            }
        }
        str += scnprintf(str, end - str, "\n");
    }
    return (str - buf);
}

DEVICE_ATTR(client, 0644, client_show, client_store);

static ssize_t group_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    unsigned char m = MIU_IDX(dev->kobj.name[7]);
    u32 g = 0;

    g = simple_strtoul(buf, NULL, 10);
    if (g < MIU_GRP_NUM)
    {
        arb_handle[m].group_selected = g;
    }
    else
    {
        printk(KERN_ERR "Invalid group %d\n", g);
        return count;
    }
    return count;
}

static ssize_t group_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int c, g, i;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    for(g = 0; g < MIU_GRP_NUM; g++)
    {
        str += scnprintf(str, end - str, "ID:IP_name\t");
    }
    str += scnprintf(str, end - str, "\n");

    for(i = 0; i < MIU_GRP_CLIENT_NUM; i++)
    {
        for(g = 0; g < MIU_GRP_NUM; g++)
        {
            c = (g * MIU_GRP_CLIENT_NUM) + i;
            if (g != arb_handle[m].group_selected)
            {
                if (!miu_client_reserved(c))
                {
                    str += scnprintf(str, end - str, "%2d:%s\t", c, miu_client_id_to_name(c));
                }
                else
                {
                    str += scnprintf(str, end - str, "%2d:       \t", c);
                }
            }
            else
            {
                if (!miu_client_reserved(c))
                {
                    str += scnprintf(str, end - str, ASCII_COLOR_GREEN"%3d:%s\t"ASCII_COLOR_END, c, miu_client_id_to_name(c));
                }
                else
                {
                    str += scnprintf(str, end - str, ASCII_COLOR_GREEN"%3d:       \t"ASCII_COLOR_END, c);
                }
            }
        }
        str += scnprintf(str, end - str, "\n");
    }
    return (str - buf);
}

DEVICE_ATTR(group, 0644, group_show, group_store);

static ssize_t group_burst_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int g, base, burst;
    struct miu_arb_grp_reg greg;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    burst = simple_strtoul(buf, NULL, 10);
    g = arb_handle[m].group_selected;

    printk(KERN_ERR "Set group %d burst %d\n", g, burst);
    if (g < MIU_GRP_NUM)
    {
        base = _get_reg_base(m);
        if (!base) {
            return count;
        }

        _load_group_settings(&greg, base, g);
        if (burst == 0) {
            greg.cfg &= ~(IGCFG_GRP_LIMIT_EN_BIT);
            greg.burst &= ~(IGBURST_GRP_MASK);
        }
        else {
            greg.cfg |= IGCFG_GRP_LIMIT_EN_BIT;
            greg.burst = (greg.burst & ~(IGBURST_GRP_MASK)) | (burst << IGBURST_GRP_SHIFT);
        }
        OUTREG16(base+REG_IGRP_CFG(g), greg.cfg);
        OUTREG16(base+REG_IGRP_BURST(g), greg.burst);
    }
    return count;
}

static ssize_t group_burst_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int base, g;
    struct miu_arb_grp_reg greg;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    g = arb_handle[m].group_selected;
    if (g < MIU_GRP_NUM)
    {
        base = _get_reg_base(m);
        if (!base) {
            return (str - buf);
        }
        _load_group_settings(&greg, base, g);
        str += scnprintf(str, end - str, "Group %d burst: %d\n", g, (greg.cfg & IGCFG_GRP_LIMIT_EN_BIT) ?
                                                         (greg.burst & IGBURST_GRP_MASK) >> IGBURST_GRP_SHIFT :
                                                         0); // no limit
    }

    return (str - buf);
}

DEVICE_ATTR(group_burst, 0644, group_burst_show, group_burst_store);

static ssize_t group_member_burst_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int g, base, burst;
    struct miu_arb_grp_reg greg;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    burst = simple_strtoul(buf, NULL, 10);
    g = arb_handle[m].group_selected;

    printk(KERN_ERR "Set group %d member burst %d\n", g, burst);
    if (g < MIU_GRP_NUM)
    {
        base = _get_reg_base(m);
        if (!base) {
            return count;
        }

        _load_group_settings(&greg, base, g);
        if (burst == 0) {
            greg.cfg &= ~(IGCFG_MBR_LIMIT_EN_BIT);
            greg.burst &= ~(IGBURST_MBR_MASK);
        }
        else {
            greg.cfg |= IGCFG_MBR_LIMIT_EN_BIT;
            greg.burst = (greg.burst & ~(IGBURST_MBR_MASK)) | (burst << IGBURST_MBR_SHIFT);
        }
        OUTREG16(base+REG_IGRP_CFG(g), greg.cfg);
        OUTREG16(base+REG_IGRP_BURST(g), greg.burst);
    }
    return count;
}

static ssize_t group_member_burst_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int base, g;
    struct miu_arb_grp_reg greg;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    g = arb_handle[m].group_selected;
    if (g < MIU_GRP_NUM)
    {
        base = _get_reg_base(m);
        if (!base) {
            return (str - buf);
        }
        _load_group_settings(&greg, base, g);
        str += scnprintf(str, end - str, "Group %d member burst: %d\n", g, (greg.cfg & IGCFG_MBR_LIMIT_EN_BIT) ?
                                                         (greg.burst & IGBURST_MBR_MASK) >> IGBURST_MBR_SHIFT :
                                                         0); // no limit
    }

    return (str - buf);
}

DEVICE_ATTR(group_member_burst, 0644, group_member_burst_show, group_member_burst_store);

static ssize_t client_nolimit_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int c, g, mbr, base, nolimit;
    struct miu_arb_grp_reg greg;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    nolimit = simple_strtoul(buf, NULL, 10);
    c = arb_handle[m].client_selected;
    g = c / MIU_GRP_CLIENT_NUM;
    mbr = c % MIU_GRP_CLIENT_NUM;

    printk(KERN_ERR "Set client %d burst length to %s\n", c, nolimit ? "no limited" : "limited");
    if (c < MIU_ARB_CLIENT_NUM)
    {
        base = _get_reg_base(m);
        if (!base) {
            return count;
        }

        _load_group_settings(&greg, base, g);
        if (nolimit == 0) {
            greg.mbr_nolimit &= ~(IGMBR_NOLIMIT_EN(mbr));
            if (!(greg.cfg & IGCFG_MBR_LIMIT_EN_BIT)) {
                printk(KERN_ERR "Conflict: group %d member limit is disabled\n", g);
            }
        }
        else {
            greg.mbr_nolimit |= IGMBR_NOLIMIT_EN(mbr);
            if (!(greg.cfg & IGCFG_MBR_LIMIT_EN_BIT)) {
                printk(KERN_ERR "NoNeed: group %d member limit is disabled already\n", g);
            }
        }
        OUTREG16(base+REG_IGRP_MBR_NOLIMIT(g), greg.mbr_nolimit);
    }
    return count;
}

static ssize_t client_nolimit_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int base, c, g, mbr, burst;
    struct miu_arb_grp_reg greg;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    c = arb_handle[m].client_selected;
    g = c / MIU_GRP_CLIENT_NUM;
    mbr = c % MIU_GRP_CLIENT_NUM;

    if (c < MIU_ARB_CLIENT_NUM)
    {
        base = _get_reg_base(m);
        if (!base) {
            return (str - buf);
        }

        _load_group_settings(&greg, base, g);
        if (!(greg.cfg & IGCFG_MBR_LIMIT_EN_BIT) || (greg.mbr_nolimit & IGMBR_NOLIMIT_EN(mbr))) {
            // group member burst is no-limited or the specified member has limit mask to 1
            burst = 0;
        }
        else {
            burst = (greg.burst & IGBURST_MBR_MASK) >> IGBURST_MBR_SHIFT;
        }
        str += scnprintf(str, end - str, "Client %d burst length: %d, (0: no-limited)\n", c, burst);
    }

    return (str - buf);
}

DEVICE_ATTR(client_nolimit, 0644, client_nolimit_show, client_nolimit_store);

static ssize_t client_prioprity_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int c, g, mbr, base, priority;
    struct miu_arb_grp_reg greg;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    priority = simple_strtoul(buf, NULL, 10);
    c = arb_handle[m].client_selected;
    g = c / MIU_GRP_CLIENT_NUM;
    mbr = c % MIU_GRP_CLIENT_NUM;

    printk(KERN_ERR "Set client %d priority to %d\n", c, priority ? 1 : 0);
    if (c < MIU_ARB_CLIENT_NUM)
    {
        base = _get_reg_base(m);
        if (!base) {
            return count;
        }

        _load_group_settings(&greg, base, g);
        greg.mbr_priority = (greg.mbr_priority & ~(IGMBR_PRIO_MASK(mbr))) | IGMBR_PRIO(mbr, (priority ? 1 : 0));
        OUTREG16(base+REG_IGRP_MBR_PRIO(g), greg.mbr_priority);
    }
    return count;
}

static ssize_t client_prioprity_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int base, c, g, mbr;
    struct miu_arb_grp_reg greg;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    c = arb_handle[m].client_selected;
    g = c / MIU_GRP_CLIENT_NUM;
    mbr = c % MIU_GRP_CLIENT_NUM;

    if (c < MIU_ARB_CLIENT_NUM)
    {
        base = _get_reg_base(m);
        if (!base) {
            return (str - buf);
        }

        _load_group_settings(&greg, base, g);
        str += scnprintf(str, end - str, "Client %d priority: %d\n", c,
                                        ((greg.mbr_priority & IGMBR_PRIO_MASK(mbr)) >> IGMBR_PRIO_SHIFT(mbr)));
    }

    return (str - buf);
}

DEVICE_ATTR(client_priority, 0644, client_prioprity_show, client_prioprity_store);

static ssize_t dump_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    u32 input = 0;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    input = simple_strtoul(buf, NULL, 10);
    if (input >= MIU_ARB_DUMP_MAX)
    {
        printk(KERN_ERR "Invalid dump mode %d (0: text; 1: reg)\n", input);
        return count;
    }
    arb_handle[m].dump = input;
    return count;
}

static ssize_t dump_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    str += scnprintf(str, end - str, "Dump Settings:\n");
    str += scnprintf(str, end - str, "text: echo 0 > dump\n");
    str += scnprintf(str, end - str, "reg : echo 1 > dump\n\n");

    switch(arb_handle[m].dump) {
    case MIU_ARB_DUMP_TEXT:
        str = _dump_as_text(str, end, m);
        break;
    case MIU_ARB_DUMP_REG:
        str = _dump_as_reg(str, end, m);
        break;
    default:
        return 0;
    }
    return (str - buf);
}

DEVICE_ATTR(dump, 0644, dump_show, dump_store);

void create_miu_arb_node(struct bus_type *miu_subsys)
{
    int ret = 0, i;
    int package;

    package = Chip_Get_Package_Type();
    memset(arb_handle, 0, sizeof(arb_handle));

    for(i = 0; i < MIU_NUM; i++)
    {
        strcpy(arb_handle[i].name, "miu_arb0");
        arb_handle[i].name[7] += i;

        arb_dev[i].index = 0;
        arb_dev[i].dev.kobj.name = (const char *)arb_handle[i].name;
        arb_dev[i].dev.bus = miu_subsys;

        ret = device_register(&arb_dev[i].dev);
        if (ret) {
            printk(KERN_ERR "Failed to register %s device!! %d\n", arb_dev[i].dev.kobj.name, ret);
            return;
        }

        device_create_file(&arb_dev[i].dev, &dev_attr_client);
        device_create_file(&arb_dev[i].dev, &dev_attr_group);
        device_create_file(&arb_dev[i].dev, &dev_attr_group_burst);
        device_create_file(&arb_dev[i].dev, &dev_attr_group_member_burst);
        device_create_file(&arb_dev[i].dev, &dev_attr_client_nolimit);
        device_create_file(&arb_dev[i].dev, &dev_attr_client_priority);
        device_create_file(&arb_dev[i].dev, &dev_attr_dump);

        if ((package == MS_I6E_PACKAGE_BGA2_DDR3) || (package == MS_I6E_PACKAGE_BGA_LPDDR2)) {
            _load_policy(i, MIU_ARB_POLICY_BGA2_DEF);
        }
        else {
            _load_policy(i, MIU_ARB_POLICY_DEF);
        }
    }
}
