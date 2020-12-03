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

struct miu_burst {
    short len;
    short idx;
};

struct miu_policy {
    char *name;
    char idx;
};

struct miu_reg_addr {
    u32 flowctrl0;
    u32 flowctrl1;
    u32 priority0;
    u32 priority1;
    u32 nolimit;
    u32 burst0;
    u32 burst1;
    u32 ctrl;
    u32 promote;
};

struct miu_reg_val {
    u32 flowctrl;
    u32 priority;
    u32 burst;
    u16 nolimit;
    u16 ctrl;
    u16 promote;
};

struct miu_policy_tbl {
    struct miu_reg_val val[MIU_NUM][MIU_GRP_NUM];
};

/*=============================================================*/
// Local variable
/*=============================================================*/

static struct miu_device miu_arb_dev[MIU_NUM];
static struct miu_arb arb_policy[MIU_NUM];
const static struct miu_burst burst_map[MIU_ARB_BURST_OPT] =
{
    // keep the last option for no limit, length 0 means no limit
    {8, 0}, {16, 1}, {32, 2}, {64, 3}, {0, MIU_ARB_BURST_NOLIM}
};
static int _cur_policy = 0;
const static struct miu_policy policy_map[MIU_ARB_POLICY_NUM] =
{
    {"round-robin", MIU_ARB_POLICY_RR},
    {"real-time",   MIU_ARB_POLICY_RT},
};
const static struct miu_reg_addr reg_tbl[MIU_GRP_NUM] =
{
    {(REG_ID_0A),(REG_ID_0B),(REG_ID_0C),(REG_ID_0D),(REG_ID_0E),(REG_ID_12),(REG_ID_13),(REG_ID_18),(REG_ID_1C)},
    {(REG_ID_2A),(REG_ID_2B),(REG_ID_2C),(REG_ID_2D),(REG_ID_2E),(REG_ID_32),(REG_ID_33),(REG_ID_38),(REG_ID_3C)},
    {(REG_ID_4A),(REG_ID_4B),(REG_ID_4C),(REG_ID_4D),(REG_ID_4E),(REG_ID_52),(REG_ID_53),(REG_ID_58),(REG_ID_5C)},
    {(REG_ID_6A),(REG_ID_6B),(REG_ID_6C),(REG_ID_6D),(REG_ID_6E),(REG_ID_72),(REG_ID_73),(REG_ID_78),(REG_ID_7C)},
};
// round-robin policy
static struct miu_policy_tbl policy_rr =
{
    {
    // miu0
    {
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
    },
    // miu1
    {
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
        {0x00000000, 0xAAAAAAAA, 0xAAAAAAAA, 0x0000, 0xC005, 0xFFFF},
    },
    }
};
// real-time path policy
static struct miu_policy_tbl policy_rt =
{
    {
    // miu0
    {
        {0x8021FF99, 0xA0AEA2BE, 0xAAA2AA82, 0x3020, 0xC305, 0xFDF9},
        {0x0000FFCC, 0xABEAA8A2, 0xA82AAAAE, 0x0002, 0xC105, 0xE7FF},
        {0x00000000, 0xAAB80A0A, 0xAA8ABAFA, 0x01CC, 0xC005, 0xFBFF},
        {0x10324800, 0xAAAAAAF3, 0xAAAAAA0C, 0x0002, 0xC305, 0xFFF0},
    },
    // miu1
    {
        {0x8021FF99, 0xA0AEA2BE, 0xAAA2AA82, 0x3020, 0xC305, 0xFDF9},
        {0x0000FFCC, 0xABEAA8A2, 0xA82AAAAE, 0x0002, 0xC105, 0xE7FF},
        {0x00000000, 0xAAB80A0A, 0xAA8ABAFA, 0x01CC, 0xC005, 0xFBFF},
        {0x10324800, 0xAAAAAAF3, 0xAAAAAA0C, 0x0002, 0xC305, 0xFFF0},
    },
    }
};
static struct miu_policy_tbl *arb_setting[MIU_ARB_POLICY_NUM] =
{
    &policy_rr, &policy_rt
};

static struct miu_policy_tbl policy_cur;

/*=============================================================*/
// Local function
/*=============================================================*/

static void _set_priority(unsigned char miu, int client, char pri)
{
    int base;
    int g, c, ofst;
    struct miu_reg_val *val;

    // update reg
    g = client / MIU_GRP_CLIENT_NUM;
    c = client % MIU_GRP_CLIENT_NUM;
    ofst = c << 1;
    val = &(policy_cur.val[miu][g]);
    val->priority = (val->priority & ~(0x3 << ofst)) | (pri << ofst);

    base = (miu == 0) ? BASE_REG_MIU_ARB_E_PA : BASE_REG_MIU1_ARB_E_PA;
    OUTREG16((base+reg_tbl[g].priority0), val->priority & 0xFFFF);
    OUTREG16((base+reg_tbl[g].priority1), val->priority >> 16);
}

static void _set_burst(unsigned char miu, int client, char burst)
{
    int base;
    int g, c, ofst, i = 0, idx = 0;
    struct miu_reg_val *val;

    do {
        if (burst_map[i].len == burst)
        {
            idx = burst_map[i].idx;
            break;
        }
        i++;
    } while(i < MIU_ARB_BURST_OPT);

    // update reg
    g = client / MIU_GRP_CLIENT_NUM;
    c = client % MIU_GRP_CLIENT_NUM;
    ofst = c << 1;
    val = &(policy_cur.val[miu][g]);
    if (idx == MIU_ARB_BURST_NOLIM)
    {
        val->burst = (val->burst & ~(0x3 << ofst));
        val->nolimit |= (1 << c);
    }
    else {
        val->burst = (val->burst & ~(0x3 << ofst)) | (idx << ofst);
        val->nolimit &= ~(1 << c);
    }

    base = (miu == 0) ? BASE_REG_MIU_ARB_E_PA : BASE_REG_MIU1_ARB_E_PA;
    OUTREG16((base+reg_tbl[g].burst0), val->burst & 0xFFFF);
    OUTREG16((base+reg_tbl[g].burst1), val->burst >> 16);
    OUTREG16((base+reg_tbl[g].nolimit), val->nolimit);
}

static void _set_promote(unsigned char miu, int client, char promote)
{
    int base;
    int g, c;
    struct miu_reg_val *val;

    // update reg
    g = client / MIU_GRP_CLIENT_NUM;
    c = client % MIU_GRP_CLIENT_NUM;
    val = &(policy_cur.val[miu][g]);
    if (promote)
    {
        val->promote |= (1 << c);
    }
    else {
        val->promote &= ~(1 << c);
    }

    base = (miu == 0) ? BASE_REG_MIU_ARB_E_PA : BASE_REG_MIU1_ARB_E_PA;
    OUTREG16((base+reg_tbl[g].promote), val->promote);
}

static void _set_flowctrl(unsigned char miu, int grp, struct arb_flowctrl *fctrl)
{
    int base;
    struct miu_reg_val *val;

    val = &(policy_cur.val[miu][grp]);

    // if only ID1 enabled, swap ID0 & ID1
    if (MIU_ARB_GET_CNT_EN(fctrl->cnt0_enable))
    {
        if (!MIU_ARB_GET_CNT_ID0_EN(fctrl->cnt0_enable))
        {
            MIU_ARB_SET_CNT_ID0(fctrl->cnt0_id, MIU_ARB_GET_CNT_ID1(fctrl->cnt0_id));
            MIU_ARB_SET_CNT_ID0_EN(fctrl->cnt0_enable, 1);
            MIU_ARB_SET_CNT_ID1_EN(fctrl->cnt0_enable, 0);
        }
        if (!MIU_ARB_GET_CNT_ID1_EN(fctrl->cnt0_enable))
        {
            // only ID0 enabled, set ID1 as ID0
            MIU_ARB_SET_CNT_ID1(fctrl->cnt0_id, MIU_ARB_GET_CNT_ID0(fctrl->cnt0_id));
        }
        val->flowctrl = (val->flowctrl & ~(0xFFFF)) | fctrl->cnt0_id | (fctrl->cnt0_period << 8);
    }
    if (MIU_ARB_GET_CNT_EN(fctrl->cnt1_enable))
    {
        if (!MIU_ARB_GET_CNT_ID0_EN(fctrl->cnt1_enable))
        {
            MIU_ARB_SET_CNT_ID0(fctrl->cnt1_id, MIU_ARB_GET_CNT_ID1(fctrl->cnt1_id));
            MIU_ARB_SET_CNT_ID0_EN(fctrl->cnt1_enable, 1);
            MIU_ARB_SET_CNT_ID1_EN(fctrl->cnt1_enable, 0);
        }
        if (!MIU_ARB_GET_CNT_ID1_EN(fctrl->cnt1_enable))
        {
            // only ID0 enabled, set ID1 as ID0
            MIU_ARB_SET_CNT_ID1(fctrl->cnt1_id, MIU_ARB_GET_CNT_ID0(fctrl->cnt1_id));
        }
        val->flowctrl = (val->flowctrl & ~(0xFFFF0000)) | (fctrl->cnt1_id << 16) | (fctrl->cnt1_period << 24);
    }

    // update reg
    base = (miu == 0) ? BASE_REG_MIU_ARB_E_PA : BASE_REG_MIU1_ARB_E_PA;
    OUTREG16((base+reg_tbl[grp].flowctrl0), val->flowctrl & 0xFFFF);
    OUTREG16((base+reg_tbl[grp].flowctrl1), val->flowctrl >> 16);
    val->ctrl &= ~(0x0300);
    if (MIU_ARB_GET_CNT_EN(fctrl->cnt0_enable))
    {
        val->ctrl |= 0x0100;
    }
    if (MIU_ARB_GET_CNT_EN(fctrl->cnt1_enable))
    {
        val->ctrl |= 0x0200;
    }
    OUTREG16((base+reg_tbl[grp].ctrl), val->ctrl);
}

static bool _flowctrl_is_enable(unsigned char miu, short client, unsigned char *period)
{
    int g, i;
    struct arb_flowctrl *f;

    g = client / MIU_GRP_CLIENT_NUM;
    i = client % MIU_GRP_CLIENT_NUM;
    f = &arb_policy[miu].fctrl[g];

    *period = 0;
    if (MIU_ARB_GET_CNT_ID0_EN(f->cnt0_enable) && (MIU_ARB_GET_CNT_ID0(f->cnt0_id) == i))
    {
        *period = f->cnt0_period;
        return 1;
    }
    else if (MIU_ARB_GET_CNT_ID1_EN(f->cnt0_enable) && (MIU_ARB_GET_CNT_ID1(f->cnt0_id) == i))
    {
        *period = f->cnt0_period;
        return 1;
    }
    else if (MIU_ARB_GET_CNT_ID0_EN(f->cnt1_enable) && (MIU_ARB_GET_CNT_ID0(f->cnt1_id) == i))
    {
        *period = f->cnt1_period;
        return 1;
    }
    else if (MIU_ARB_GET_CNT_ID1_EN(f->cnt1_enable) && (MIU_ARB_GET_CNT_ID1(f->cnt1_id) == i))
    {
        *period = f->cnt1_period;
        return 1;
    }
    return 0;
}

static bool _flowctrl_enable(unsigned char miu, short client, bool enable, char period)
{
    int g, i;
    struct arb_flowctrl *f;

    g = client / MIU_GRP_CLIENT_NUM;
    i = client % MIU_GRP_CLIENT_NUM;
    f = &arb_policy[miu].fctrl[g];

    if (enable)
    {
        do {
            // enabled group flow control cnt0 has the same period
            if (MIU_ARB_GET_CNT_EN(f->cnt0_enable) && (f->cnt0_period == period))
            {
                if (!MIU_ARB_GET_CNT_ID0_EN(f->cnt0_enable))
                {
                    //id0 not used
                    MIU_ARB_SET_CNT_ID0_EN(f->cnt0_enable, 1);
                    MIU_ARB_SET_CNT_ID0(f->cnt0_id, i);
                    break;
                }
                else if (!MIU_ARB_GET_CNT_ID1_EN(f->cnt0_enable))
                {
                    //id1 not used
                    MIU_ARB_SET_CNT_ID1_EN(f->cnt0_enable, 1);
                    MIU_ARB_SET_CNT_ID1(f->cnt0_id, i);
                    break;
                }
            }
            // enabled group flow control cnt1 has the same period
            if (MIU_ARB_GET_CNT_EN(f->cnt1_enable) && (f->cnt1_period == period))
            {
                if (!MIU_ARB_GET_CNT_ID0_EN(f->cnt1_enable))
                {
                    //id0 not used
                    MIU_ARB_SET_CNT_ID0_EN(f->cnt1_enable, 1);
                    MIU_ARB_SET_CNT_ID0(f->cnt1_id, i);
                    break;
                }
                else if (!MIU_ARB_GET_CNT_ID1_EN(f->cnt1_enable))
                {
                    //id1 not used
                    MIU_ARB_SET_CNT_ID1_EN(f->cnt1_enable, 1);
                    MIU_ARB_SET_CNT_ID1(f->cnt1_id, i);
                    break;
                }
            }
            if (!MIU_ARB_GET_CNT_EN(f->cnt0_enable))
            {
                MIU_ARB_SET_CNT_ID0_EN(f->cnt0_enable, 1);
                f->cnt0_period = period;
                MIU_ARB_SET_CNT_ID0(f->cnt0_id, i);
                break;
            }
            if (!MIU_ARB_GET_CNT_EN(f->cnt1_enable))
            {
                MIU_ARB_SET_CNT_ID0_EN(f->cnt1_enable, 1);
                f->cnt1_period = period;
                MIU_ARB_SET_CNT_ID0(f->cnt1_id, i);
                break;
            }
            return 1; // not free one, failed

        } while(1);
    }
    else
    {
        // disable client flow control
        if (MIU_ARB_GET_CNT_ID0_EN(f->cnt0_enable) && (MIU_ARB_GET_CNT_ID0(f->cnt0_id) == i))
        {
            MIU_ARB_SET_CNT_ID0_EN(f->cnt0_enable, 0);
        }
        else if (MIU_ARB_GET_CNT_ID1_EN(f->cnt0_enable) && (MIU_ARB_GET_CNT_ID1(f->cnt0_id) == i))
        {
            MIU_ARB_SET_CNT_ID1_EN(f->cnt0_enable, 0);
        }
        else if (MIU_ARB_GET_CNT_ID0_EN(f->cnt1_enable) && (MIU_ARB_GET_CNT_ID0(f->cnt1_id) == i))
        {
            MIU_ARB_SET_CNT_ID0_EN(f->cnt1_enable, 0);
        }
        else if (MIU_ARB_GET_CNT_ID1_EN(f->cnt1_enable) && (MIU_ARB_GET_CNT_ID1(f->cnt1_id) == i))
        {
            MIU_ARB_SET_CNT_ID1_EN(f->cnt1_enable, 0);
        }
        else
        {
            //nothing changed
            return 0;
        }
    }
    _set_flowctrl(miu, g, f);

    return 0;
}

static char *_dump_as_text(char *str, char *end, unsigned char miu)
{
    int c;
    unsigned char period;

    str += scnprintf(str, end - str, "Num:IP_name   [pri][burst][promote][ctrlflow]\n");
    for(c = 1; c < MIU_ARB_CLIENT_NUM; c++) // skip client 0
    {
        if (!miu_client_reserved(c))
        {
            if (!_flowctrl_is_enable(miu, c, &period))
            {
                str += scnprintf(str, end - str, "%3d:%s   %d     %2d        %d\n",
                                        c, miu_client_id_to_name(c),
                                        arb_policy[miu].priority[c],
                                        arb_policy[miu].burst[c],
                                        arb_policy[miu].promote[c]);
            }
            else {
                str += scnprintf(str, end - str, "%3d:%s   %d     %2d        %d      0x%02X\n",
                                        c, miu_client_id_to_name(c),
                                        arb_policy[miu].priority[c],
                                        arb_policy[miu].burst[c],
                                        arb_policy[miu].promote[c],
                                        period);
            }
        }
    }
    return str;
}

static char *_dump_as_reg(char *str, char *end, unsigned char miu)
{
    int g;
    struct miu_reg_val *val;

    // miu0
    str += scnprintf(str, end - str, "    // miu%d\n", miu);
    str += scnprintf(str, end - str, "    {\n");

    for(g = 0; g < MIU_GRP_NUM; g++)
    {
        val = &(policy_cur.val[miu][g]);
        str += scnprintf(str, end - str,
                        "        {0x%08X, 0x%08X, 0x%08X, 0x%04X, 0x%04X, 0x%04X},\n",
                        val->flowctrl, val->priority, val->burst, val->nolimit, val->ctrl, val->promote);
    }
    str += scnprintf(str, end - str, "    },\n");

    return str;
}

static void _load_miu_arb(int miu, int grp, struct miu_reg_val *val)
{
    int c, i;
    struct arb_flowctrl *f;

    c = grp * MIU_GRP_CLIENT_NUM;
    for(i = 0; i < MIU_GRP_CLIENT_NUM; i++)
    {
        arb_policy[miu].priority[c] = (val->priority >> (i<<1)) & 0x3;
        if (val->nolimit & (1 << i))
        {
            arb_policy[miu].burst[c] = 0;
        }
        else {
            arb_policy[miu].burst[c] = burst_map[(val->burst >> (i<<1)) & 0x3].len;
        }
        if (val->promote & (1 << i))
        {
            arb_policy[miu].promote[c] = 1;
        }
        else {
            arb_policy[miu].promote[c] = 0;
        }
        c++;
    }
    f = &arb_policy[miu].fctrl[grp];
    if (val->ctrl & 0x0100)
    {
        f->cnt0_id = val->flowctrl & 0xFF;
        f->cnt0_period = (val->flowctrl >> 8) & 0xFF;
        MIU_ARB_SET_CNT_ID0_EN(f->cnt0_enable, 1);
        // ID1 is identical with ID0
        if ((MIU_ARB_GET_CNT_ID0(f->cnt0_id)) == (MIU_ARB_GET_CNT_ID1(f->cnt0_id)))
        {
            MIU_ARB_SET_CNT_ID1_EN(f->cnt0_enable, 0);
        }
        else {
            MIU_ARB_SET_CNT_ID1_EN(f->cnt0_enable, 1);
        }
    }
    else {
        MIU_ARB_SET_CNT_ID0(f->cnt0_id, 0);
        MIU_ARB_SET_CNT_ID1(f->cnt0_id, 0);
        MIU_ARB_SET_CNT_ID0_EN(f->cnt0_enable, 0);
        MIU_ARB_SET_CNT_ID1_EN(f->cnt0_enable, 0);
        f->cnt0_period = 0x00;
    }
    if (val->ctrl & 0x0200)
    {
        f->cnt1_id = (val->flowctrl >> 16) & 0xFF;
        f->cnt1_period = (val->flowctrl >> 24) & 0xFF;
        MIU_ARB_SET_CNT_ID0_EN(f->cnt1_enable, 1);
        if ((MIU_ARB_GET_CNT_ID0(f->cnt1_id)) == (MIU_ARB_GET_CNT_ID1(f->cnt1_id)))
        {
            MIU_ARB_SET_CNT_ID1_EN(f->cnt1_enable, 0);
        }
        else {
            MIU_ARB_SET_CNT_ID1_EN(f->cnt1_enable, 1);
        }
    }
    else {
        MIU_ARB_SET_CNT_ID0(f->cnt1_id, 0);
        MIU_ARB_SET_CNT_ID1(f->cnt1_id, 0);
        MIU_ARB_SET_CNT_ID0_EN(f->cnt1_enable, 0);
        MIU_ARB_SET_CNT_ID1_EN(f->cnt1_enable, 0);
        f->cnt1_period = 0x00;
    }
}

static void _load_policy(int idx)
{
    int base;
    int m, g;
    struct miu_reg_val *val;

    if (idx >= MIU_ARB_POLICY_NUM)
    {
        return;
    }
    memcpy(&policy_cur, arb_setting[idx], sizeof(struct miu_policy_tbl));

    for(m = 0; m < MIU_NUM; m++)
    {
        base = (m == 0) ? BASE_REG_MIU_ARB_E_PA : BASE_REG_MIU1_ARB_E_PA;
        for(g = 0; g < MIU_GRP_NUM; g++)
        {
            // register settings
            val = &(policy_cur.val[m][g]);
            OUTREG16((base+reg_tbl[g].flowctrl0), val->flowctrl & 0xFFFF);
            OUTREG16((base+reg_tbl[g].flowctrl1), val->flowctrl >> 16);
            OUTREG16((base+reg_tbl[g].priority0), val->priority & 0xFFFF);
            OUTREG16((base+reg_tbl[g].priority1), val->priority >> 16);
            OUTREG16((base+reg_tbl[g].burst0), val->burst & 0xFFFF);
            OUTREG16((base+reg_tbl[g].burst1), val->burst >> 16);
            OUTREG16((base+reg_tbl[g].nolimit), val->nolimit);
            OUTREG16((base+reg_tbl[g].ctrl), val->ctrl);
            OUTREG16((base+reg_tbl[g].promote), val->promote);
            // update readable info
            _load_miu_arb(m, g, val);
        }
    }
    _cur_policy = idx;
}

static ssize_t client_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    unsigned char m = MIU_IDX(dev->kobj.name[7]);
    u32 input = 0;

    if(!strncmp(buf, "all", strlen("all")))
    {
        arb_policy[m].client_selected = MIU_ARB_CLIENT_ALL;
        return count;
    }

    input = simple_strtoul(buf, NULL, 10);
    if (input < MIU_ARB_CLIENT_NUM)
    {
        arb_policy[m].client_selected = input;
    }
    else
    {
        printk(KERN_ERR "Invalid client %d\n", input);
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
        str += scnprintf(str, end - str, "Num:IP_name     ");
    }
    str += scnprintf(str, end - str, "\n");

    for(i = 0; i < MIU_GRP_CLIENT_NUM; i++)
    {
        for(g = 0; g < MIU_GRP_NUM; g++)
        {
            c = (g * MIU_GRP_CLIENT_NUM) + i;
            if (c != arb_policy[m].client_selected)
            {
                if (!miu_client_reserved(c))
                {
                    str += scnprintf(str, end - str, "%3d:%s  ", c, miu_client_id_to_name(c));
                }
                else
                {
                    str += scnprintf(str, end - str, "%3d:            ", c);
                }
            }
            else {
                str += scnprintf(str, end - str, ASCII_COLOR_GREEN"%3d:%s  "ASCII_COLOR_END, c, miu_client_id_to_name(c));
            }
        }
        str += scnprintf(str, end - str, "\n");
    }
    return (str - buf);
}

DEVICE_ATTR(client, 0644, client_show, client_store);

static ssize_t priority_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int c;
    u32 input = 0;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    input = simple_strtoul(buf, NULL, 10);
    if (input > MIU_ARB_PRIO_4TH)
    {
        printk(KERN_ERR "Invalid priority %d\n", input);
        return count;
    }
    c = arb_policy[m].client_selected;
    if (c != MIU_ARB_CLIENT_ALL)
    {
        if (!miu_client_reserved(c))
        {
            arb_policy[m].priority[c] = input;
            _set_priority(m, c, input);
        }
    }
    else
    {
        // all clients set to the same priority
        for (c = 0; c < MIU_ARB_CLIENT_NUM; c++)
        {
            arb_policy[m].priority[c] = input;
            _set_priority(m, c, input);
        }
    }
    return count;
}

static ssize_t priority_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int c;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    str += scnprintf(str, end - str, "Priority (highest -> lowest):\n");
    str += scnprintf(str, end - str, "0, 1, 2, 3\n\n");

    c = arb_policy[m].client_selected;
    if (c != MIU_ARB_CLIENT_ALL)
    {
        str += scnprintf(str, end - str, "Num:IP_name   [pri]\n");
        str += scnprintf(str, end - str, "%3d:%s[  %d]\n", c, miu_client_id_to_name(c), arb_policy[m].priority[c]);
    }
    else
    {
        int g, i;

        for(g = 0; g < MIU_GRP_NUM; g++)
        {
            str += scnprintf(str, end - str, "Num:IP_name        ");
        }
        str += scnprintf(str, end - str, "\n");

        for(i = 0; i < MIU_GRP_CLIENT_NUM; i++)
        {
            for(g = 0; g < MIU_GRP_NUM; g++)
            {
                c = (g * MIU_GRP_CLIENT_NUM) + i;
                if (!miu_client_reserved(c))
                {
                    str += scnprintf(str, end - str, "%3d:%s[%d]  ", c, miu_client_id_to_name(c), arb_policy[m].priority[c]);
                }
                else
                {
                    str += scnprintf(str, end - str, "%3d:               ", c);
                }
            }
            str += scnprintf(str, end - str, "\n");
        }
    }
    return (str - buf);
}

DEVICE_ATTR(priority, 0644, priority_show, priority_store);

static ssize_t burst_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int c, i = 0, burst = 0xFFFF;
    u32 input = 0;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    input = simple_strtoul(buf, NULL, 10);
    do {
        if (burst_map[i].len == input)
        {
            burst = input;
            break;
        }
    } while(++i < MIU_ARB_BURST_OPT);
    if (burst == 0xFFFF)
    {
        printk(KERN_ERR "Invalid burst %d\n", input);
        return count;
    }

    c = arb_policy[m].client_selected;
    printk(KERN_ERR "set client %d burst %d\n", c, input);
    if (c != MIU_ARB_CLIENT_ALL)
    {
        if (!miu_client_reserved(c))
        {
            arb_policy[m].burst[c] = burst;
            _set_burst(m, c, burst);
        }
    }
    else
    {
        // all clients set to the same burst
        for (c = 0; c < MIU_ARB_CLIENT_NUM; c++)
        {
            arb_policy[m].burst[c] = burst;
            _set_burst(m, c, burst);
        }
    }
    return count;
}

static ssize_t burst_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i, c;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    str += scnprintf(str, end - str, "Burst Option:\n");
    for(i = 0; i < MIU_ARB_BURST_NOLIM; i++)
    {
        str += scnprintf(str, end - str, " %d,", burst_map[i].len);
    }
    str += scnprintf(str, end - str, " %d(No Limited)\n\n", burst_map[MIU_ARB_BURST_NOLIM].len);

    c = arb_policy[m].client_selected;
    if (c != MIU_ARB_CLIENT_ALL)
    {
        str += scnprintf(str, end - str, "Num:IP_name   [burst]\n");
        if (arb_policy[m].burst[c] == burst_map[MIU_ARB_BURST_NOLIM].len)
        {
            str += scnprintf(str, end - str, "%3d:%s"ASCII_COLOR_RED"[   %2d]"ASCII_COLOR_END,
                                            c, miu_client_id_to_name(c), arb_policy[m].burst[c]);
        }
        else {
            str += scnprintf(str, end - str, "%3d:%s[   %2d]", c, miu_client_id_to_name(c), arb_policy[m].burst[c]);
        }
        str += scnprintf(str, end - str, "\n");
    }
    else
    {
        int g;

        for(g = 0; g < MIU_GRP_NUM; g++)
        {
            str += scnprintf(str, end - str, "Num:IP_name            ");
        }
        str += scnprintf(str, end - str, "\n");

        for(i = 0; i < MIU_GRP_CLIENT_NUM; i++)
        {
            for(g = 0; g < MIU_GRP_NUM; g++)
            {
                c = (g * MIU_GRP_CLIENT_NUM) + i;
                if (!miu_client_reserved(c))
                {
                    if (arb_policy[m].burst[c] != burst_map[MIU_ARB_BURST_NOLIM].len)
                    {
                        str += scnprintf(str, end - str, "%3d:%s[   %2d]  ",
                                                        c, miu_client_id_to_name(c), arb_policy[m].burst[c]);
                    }
                    else {
                        str += scnprintf(str, end - str, "%3d:%s"ASCII_COLOR_RED"[   %2d]  "ASCII_COLOR_END,
                                                        c, miu_client_id_to_name(c), arb_policy[m].burst[c]);
                    }
                }
                else
                {
                    str += scnprintf(str, end - str, "%3d:                   ", c);
                }
            }
            str += scnprintf(str, end - str, "\n");
        }
    }
    return (str - buf);
}

DEVICE_ATTR(burst, 0644, burst_show, burst_store);

static ssize_t promote_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int c;
    u32 input = 0;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    input = simple_strtoul(buf, NULL, 10);
    input = input ? 1 : 0;

    c = arb_policy[m].client_selected;
    if (c != MIU_ARB_CLIENT_ALL)
    {
        if (!miu_client_reserved(c))
        {
            arb_policy[m].promote[c] = input;
            _set_promote(m, c, input);
        }
    }
    else
    {
        // all clients set to the same promote
        for (c = 0; c < MIU_ARB_CLIENT_NUM; c++)
        {
            arb_policy[m].promote[c] = input;
            _set_promote(m, c, input);
        }
    }
    return count;
}

static ssize_t promote_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int c;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    str += scnprintf(str, end - str, "Promote Setting:\n");
    str += scnprintf(str, end - str, "0 -> disable\n");
    str += scnprintf(str, end - str, "1 -> enable\n\n");

    c = arb_policy[m].client_selected;
    if (c != MIU_ARB_CLIENT_ALL)
    {
        str += scnprintf(str, end - str, "Num:IP_name   [promote]\n");
        str += scnprintf(str, end - str, "%3d:%s[      %d]\n", c, miu_client_id_to_name(c), arb_policy[m].promote[c]);
    }
    else
    {
        int i, g;

        for(g = 0; g < MIU_GRP_NUM; g++)
        {
            str += scnprintf(str, end - str, "Num:IP_name        ");
        }
        str += scnprintf(str, end - str, "\n");

        for(i = 0; i < MIU_GRP_CLIENT_NUM; i++)
        {
            for(g = 0; g < MIU_GRP_NUM; g++)
            {
                c = (g * MIU_GRP_CLIENT_NUM) + i;
                if (!miu_client_reserved(c))
                {
                    str += scnprintf(str, end - str, "%3d:%s[%d]  ", c, miu_client_id_to_name(c), arb_policy[m].promote[c]);
                }
                else
                {
                    str += scnprintf(str, end - str, "%3d:               ", c);
                }
            }
            str += scnprintf(str, end - str, "\n");
        }
    }
    return (str - buf);
}

DEVICE_ATTR(promote, 0644, promote_show, promote_store);

static ssize_t flowctrl_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int c;
    ssize_t ret = count;
    u32 enable, period, en_tmp;
    unsigned char peri_tmp;
    char *pt, *opt;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    c = arb_policy[m].client_selected;
    if ((c == MIU_ARB_CLIENT_ALL) || (c >= MIU_ARB_CLIENT_NUM) || miu_client_reserved(c))
        return count;

    // check input parameters
    do {
        pt = kmalloc(strlen(buf)+1, GFP_KERNEL);
        strcpy(pt, buf);
        if ((opt = strsep(&pt, ";, ")) != NULL)
        {
            enable = simple_strtoul(opt, NULL, 10);
            if (enable)
            {
                if ((opt = strsep(&pt, ";, ")) == NULL)
                {
                    ret = 0;
                    break;
                }
                period = simple_strtoul(opt, NULL, 10);
                if (!period || (period > MIU_ARB_CNT_PERIOD_MAX))
                {
                    printk(KERN_ERR "Invalid period %d (1-%d)\n", period, MIU_ARB_CNT_PERIOD_MAX);
                    ret = 0;
                    break;
                }
            }
        }
        else {
            ret = 0;
        }
        break;
    }while(1);

    kfree(pt);
    if (ret == 0)
    {
        printk(KERN_ERR "Usage: echo [0/1] [period] > flowctrl\n");
        return count;
    }

    if (enable)
    {
        // to keep the original setting
        en_tmp = _flowctrl_is_enable(m, c, &peri_tmp);
        _flowctrl_enable(m, c, 0, 0);
        // restore the original settings if failed
        if (_flowctrl_enable(m, c, enable, period) && en_tmp)
        {
            _flowctrl_enable(m, c, en_tmp, peri_tmp);
        }
    }
    else
    {
        // disable client flow control
        _flowctrl_enable(m, c, 0, 0);
    }

    return count;
}

static ssize_t flowctrl_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int c, g;
    struct arb_flowctrl *f;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    str += scnprintf(str, end - str, "Flow Control:\n");
    str += scnprintf(str, end - str, "echo [id] > client\n");
    str += scnprintf(str, end - str, "enable:  echo 1 [period] > flowctrl\n");
    str += scnprintf(str, end - str, "disable: echo 0 > flowctrl\n");

    str += scnprintf(str, end - str, "\nNum:IP_name   [period]\n");
    for(g = 0; g < MIU_GRP_NUM; g++)
    {
        f = &arb_policy[m].fctrl[g];
        if (MIU_ARB_GET_CNT_EN(f->cnt0_enable) || MIU_ARB_GET_CNT_EN(f->cnt1_enable))
        {
            if (MIU_ARB_GET_CNT_ID0_EN(f->cnt0_enable))
            {
                c = (g * MIU_GRP_CLIENT_NUM) + MIU_ARB_GET_CNT_ID0(f->cnt0_id);
                str += scnprintf(str, end - str, "%3d:%s[  0x%02X]\n", c, miu_client_id_to_name(c), f->cnt0_period);
            }
            if (MIU_ARB_GET_CNT_ID1_EN(f->cnt0_enable))
            {
                c = (g * MIU_GRP_CLIENT_NUM) + MIU_ARB_GET_CNT_ID1(f->cnt0_id);
                str += scnprintf(str, end - str, "%3d:%s[  0x%02X]\n", c, miu_client_id_to_name(c), f->cnt0_period);
            }
            if (MIU_ARB_GET_CNT_ID0_EN(f->cnt1_enable))
            {
                c = (g * MIU_GRP_CLIENT_NUM) + MIU_ARB_GET_CNT_ID0(f->cnt1_id);
                str += scnprintf(str, end - str, "%3d:%s[  0x%02X]\n", c, miu_client_id_to_name(c), f->cnt1_period);
            }
            if (MIU_ARB_GET_CNT_ID1_EN(f->cnt1_enable))
            {
                c = (g * MIU_GRP_CLIENT_NUM) + MIU_ARB_GET_CNT_ID1(f->cnt1_id);
                str += scnprintf(str, end - str, "%3d:%s[  0x%02X]\n", c, miu_client_id_to_name(c), f->cnt1_period);
            }
        }
    }

    return (str - buf);
}

DEVICE_ATTR(flowctrl, 0644, flowctrl_show, flowctrl_store);

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
    arb_policy[m].dump = input;
    return count;
}

static ssize_t dump_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    unsigned char m = MIU_IDX(dev->kobj.name[7]);

    str += scnprintf(str, end - str, "Dump Settings:\n");
    str += scnprintf(str, end - str, "text: echo 0 > dump\n");
    str += scnprintf(str, end - str, "reg : echo 1 > dump\n");
    str += scnprintf(str, end - str, "cat dump\n\n");

    switch(arb_policy[m].dump) {
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

static ssize_t policy_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    u32 input = 0, i;

    input = simple_strtoul(buf, NULL, 10);
    if (input >= MIU_ARB_POLICY_NUM)
    {
        printk(KERN_ERR "Invalid policy %d\n", input);
        for(i = 0; i < MIU_ARB_POLICY_NUM; i++)
        {
            printk(KERN_ERR "%d: %s\n", policy_map[i].idx, policy_map[i].name);
        }
        return count;
    }
    _load_policy(input);

    return count;
}

static ssize_t policy_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    int i;

    str += scnprintf(str, end - str, "Policy:\n");
    for(i = 0; i < MIU_ARB_POLICY_NUM; i++)
    {
        if (i == _cur_policy)
        {
            str += scnprintf(str, end - str, ASCII_COLOR_GREEN"%d: %s\n"ASCII_COLOR_END, policy_map[i].idx, policy_map[i].name);
        }
        else {
            str += scnprintf(str, end - str, "%d: %s\n", policy_map[i].idx, policy_map[i].name);
        }
    }

    return (str - buf);
}

DEVICE_ATTR(policy, 0644, policy_show, policy_store);

void create_miu_bw_node(struct bus_type *miu_subsys)
{
    int ret = 0, i;

    memset(arb_policy, 0, sizeof(arb_policy));

    for(i = 0; i < MIU_NUM; i++)
    {
        strcpy(arb_policy[i].name, "miu_arb0");
        arb_policy[i].name[7] += i;

        miu_arb_dev[i].index = 0;
        miu_arb_dev[i].dev.kobj.name = (const char *)arb_policy[i].name;
        miu_arb_dev[i].dev.bus = miu_subsys;

        ret = device_register(&miu_arb_dev[i].dev);
        if (ret) {
            printk(KERN_ERR "Failed to register %s device!! %d\n",miu_arb_dev[i].dev.kobj.name,ret);
            return;
        }

        device_create_file(&miu_arb_dev[i].dev, &dev_attr_client);
        device_create_file(&miu_arb_dev[i].dev, &dev_attr_priority);
        device_create_file(&miu_arb_dev[i].dev, &dev_attr_burst);
        device_create_file(&miu_arb_dev[i].dev, &dev_attr_promote);
        device_create_file(&miu_arb_dev[i].dev, &dev_attr_flowctrl);
        device_create_file(&miu_arb_dev[i].dev, &dev_attr_dump);
        device_create_file(&miu_arb_dev[i].dev, &dev_attr_policy);
    }
    _load_policy(MIU_ARB_POLICY_RT);

    //adjust group 3 to lowest priority, all groups have same burst length
    OUTREG16(BASE_REG_MIU_ARB_B_PA + REG_ID_27, 0xC0);
    OUTREG16(BASE_REG_MIU_ARB_B_PA + REG_ID_31, 0x8080);
    OUTREG16(BASE_REG_MIU_ARB_B_PA + REG_ID_32, 0xAA);
    OUTREG16(BASE_REG_MIU_ARB_B_PA + REG_ID_2E, 0x00);
    OUTREG16(BASE_REG_MIU1_ARB_B_PA + REG_ID_27, 0xC0);
    OUTREG16(BASE_REG_MIU1_ARB_B_PA + REG_ID_31, 0x8080);
    OUTREG16(BASE_REG_MIU1_ARB_B_PA + REG_ID_32, 0xAA);
    OUTREG16(BASE_REG_MIU1_ARB_B_PA + REG_ID_2E, 0x00);
    //VEN_W high priority enable
    OUTREG16(BASE_REG_MIU_ARB_E_PA + REG_ID_64, 0xFFFD);
    OUTREG16(BASE_REG_MIU1_ARB_E_PA + REG_ID_64, 0xFFFD);
}
