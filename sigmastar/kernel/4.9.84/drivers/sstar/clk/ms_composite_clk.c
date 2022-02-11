
/*
* ms_composite_clk.c- Sigmastar
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
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/cpu.h>
#ifdef CONFIG_PM_SLEEP
#include <linux/syscore_ops.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#define DEBUG_AUTOEN 0
#endif
#include "ms_types.h"
#include "ms_platform.h"


struct ms_clk_mux {
    struct clk_hw   hw;
    void __iomem    *reg;
    u32             *table;
    u32             mask;
    u8              shift;
    u8              flags;
    u8              glitch;  //this is specific usage for MSTAR
    spinlock_t      *lock;
#ifdef CONFIG_PM_SLEEP
    u8              parent;  //store parent index for resume
    struct clk_gate *gate;
    struct list_head list;
#endif
};

#ifdef CONFIG_PM_SLEEP
static u8 _is_syscore_register = 0;
LIST_HEAD(ms_clk_mux_list);
#endif

#define to_ms_clk_mux(_hw) container_of(_hw, struct ms_clk_mux, hw)

static u8 ms_clk_mux_get_parent(struct clk_hw *hw)
{
    struct ms_clk_mux *mux = to_ms_clk_mux(hw);
    int num_parents = clk_hw_get_num_parents(hw);
    u32 val;

    val = clk_readl(mux->reg) >> mux->shift;
    val &= mux->mask;

    if (val >= num_parents)
        return -EINVAL;

    return val;
}

static int ms_clk_mux_set_parent(struct clk_hw *hw, u8 index)
{
    struct ms_clk_mux *mux = to_ms_clk_mux(hw);
    u32 val;
    unsigned long flags = 0;

    if ((mux->table)){
        index = mux->table[index];
    }
    else
    {
        if (mux->flags & CLK_MUX_INDEX_BIT)
            index = (1 << ffs(index));

        if (mux->flags & CLK_MUX_INDEX_ONE)
            index++;
    }

    if (mux->lock)
        spin_lock_irqsave(mux->lock, flags);

    if (mux->flags & CLK_MUX_HIWORD_MASK)
    {
        val = mux->mask << (mux->shift + 16);
    }
    else
    {
        val = clk_readl(mux->reg);
        val &= ~(mux->mask << mux->shift);
    }

//    //switch to glitch-free mux(set 0)
    if (mux->glitch)
    {
        val &= ~(1 << mux->glitch);
        clk_writel(val, mux->reg);
    }

    val |= index << mux->shift;
    clk_writel(val, mux->reg);

    //switch back to original mux(set 1)
    if (mux->glitch)
    {
        val |= 1 << mux->glitch;
        clk_writel(val, mux->reg);
    }

    if (mux->lock)
        spin_unlock_irqrestore(mux->lock, flags);

    return 0;
}

static unsigned long ms_clk_mux_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    struct ms_clk_mux *mux = to_ms_clk_mux(hw);

    if (mux->glitch)
    {
        if(clk_readl(mux->reg) & (1 << mux->glitch))
        {
            pr_debug("\n <%s> parent_rate=%lu, glitch-mux=1\n\n", hw->init->name, parent_rate);
            return parent_rate;
        }
        else
        {
            pr_debug("\n <%s> parent_rate=%lu, glitch-mux=0\n\n", hw->init->name, parent_rate);
            return 12000000;
        }
    }

    pr_debug("\n <%s> parent_rate=%lu, no glitch-mux\n\n", hw->init->name, parent_rate);
    return parent_rate;

}

#ifdef CONFIG_PM_SLEEP
static int ms_clk_mux_suspend(void)
{
    struct ms_clk_mux *mux;
    struct clk *clk;
    //keep parent index for restoring clocks in resume
    list_for_each_entry(mux, &ms_clk_mux_list, list)
    {
        if (mux && mux->hw.clk) {
            clk = mux->hw.clk;
            pr_debug("clk %s cnt %d parent %d\n", __clk_get_name(clk),
                                                __clk_get_enable_count(clk),
                                                ms_clk_mux_get_parent(&mux->hw));
            mux->parent = ms_clk_mux_get_parent(&mux->hw);
        }

        if (mux && mux->gate) {
            //clock already disabled, not need to restore it in resume
            if (clk_gate_ops.is_enabled(&mux->gate->hw) == 0) {
                mux->gate = NULL;
            }
        }
    }
    return 0;
}

static void ms_clk_mux_resume(void)
{
    struct ms_clk_mux *mux;

    //restore auto-enable clocks in list
    list_for_each_entry(mux, &ms_clk_mux_list, list)
    {
        if (mux && mux->gate && mux->hw.clk) {
            //Don't set the CLK_mcu, because of mux_glitch!=0, it will gate the clock.
             if (strcmp(__clk_get_name(mux->hw.clk), "CLK_mcu")) {
                ms_clk_mux_set_parent(&mux->hw, mux->parent);
            }
            clk_gate_ops.enable(&mux->gate->hw);
        }
    }
}

#if DEBUG_AUTOEN //debug
ssize_t clk_auto_show_write( struct file * file,  const char __user * buf,
                     size_t count, loff_t *ppos)
{
    printk("clk_auto_showwr\r\n");
    return 1;
}

static ssize_t clk_auto_show(struct seq_file *s, void *pArg)
{
    struct ms_clk_mux *mux;
    struct clk *clk;

    //keep parent index for restoring clocks in resume
    list_for_each_entry(mux, &ms_clk_mux_list, list)
    {
        if (mux && mux->gate) {
            clk = mux->hw.clk;

            if(!&mux->hw){
                ms_clk_mux_set_parent(&mux->hw, mux->parent);
            clk_gate_ops.enable(&mux->gate->hw);
            printk("clk %s cnt %d parent %d\r\n", __clk_get_name(clk), __clk_get_enable_count(clk), mux->parent);
            }
            printk("clk %s cnt %d parent %d\r\n", __clk_get_name(clk), __clk_get_enable_count(clk), mux->parent);
        }
    #if 0
        if (mux && mux->gate) {
            //clock already disabled, not need to restore it in resume
            if (clk_gate_ops.is_enabled(&mux->gate->hw) == 0) {
                mux->gate = NULL;
            }
                clk = mux->hw.clk;
                if(mux->hw.clk){
//                    seq_printf(s, "clk %s cnt %d", __clk_get_name(clk), __clk_get_enable_count(clk));
                    printk("clk %s cnt %d\r\n", __clk_get_name(clk), __clk_get_enable_count(clk));
                    pr_err("clk %s cnt %d parent %d\n", __clk_get_name(clk),
                                                __clk_get_enable_count(clk),
                                                ms_clk_mux_get_parent(&mux->hw));
                    ms_clk_mux_set_parent(&mux->hw, mux->parent);
                }
        }
        #endif
    }
    return 0;
}

static int clk_auto_open(struct inode *inode, struct file *file)
{
    single_open(file, clk_auto_show, PDE_DATA(inode));
    return 0;
}

static const struct file_operations clk_auto_showfops = {
    .owner   = THIS_MODULE,
    .open = clk_auto_open,
    .read = seq_read,
    .write = clk_auto_show_write,
    .llseek  = seq_lseek,
    .release = single_release,
};
struct proc_dir_entry *g_pClk_auto_proc = NULL;
void clk_auto_proc(void)
{
    struct  proc_dir_entry  *entry;

    g_pClk_auto_proc = proc_mkdir("clk_autoen", NULL);

    entry =  proc_create("dump_auto_en", 0666, g_pClk_auto_proc, &clk_auto_showfops);
    if (!entry)
    {
        printk(KERN_ERR "failed  to  create  procfs  file.\n");
    }
}

#endif

struct syscore_ops ms_clk_mux_syscore_ops = {
    .suspend = ms_clk_mux_suspend,
    .resume = ms_clk_mux_resume,
};
#endif

struct clk_ops ms_clk_mux_ops = {
    .get_parent = ms_clk_mux_get_parent,
    .set_parent = ms_clk_mux_set_parent,
    .determine_rate = __clk_mux_determine_rate,
    .recalc_rate = ms_clk_mux_recalc_rate,
};


static void __init ms_clk_composite_init(struct device_node *node)
{
    const char *clk_name = node->name;
    int num_parents;
    const char **parent_names;
    struct ms_clk_mux *mux = NULL;
    struct clk_gate *gate = NULL;
    void __iomem *reg;
    u32 i, mux_shift, mux_width, mux_glitch, bit_idx, auto_enable,ignore;
    struct clk *clk;
    unsigned int flag = 0;

    num_parents = of_clk_get_parent_count(node);
    if(num_parents<0)
        num_parents = 0;
    parent_names = kzalloc((sizeof(char *) * num_parents), GFP_KERNEL);
    mux = kzalloc(sizeof(*mux), GFP_KERNEL);
    gate = kzalloc(sizeof(*gate), GFP_KERNEL);

    if (!parent_names || !mux || !gate)
    {
        pr_err("<%s> allocate mem fail\n", clk_name);
        goto fail;
    }

    reg = of_iomap(node, 0);
    if (!reg)
    {
        pr_err("<%s> map region fail\n", clk_name);
        goto fail;
    }

    for (i = 0; i < num_parents; i++)
        parent_names[i] = of_clk_get_parent_name(node, i);

    mux->reg = reg;
    gate->reg = reg;
    gate->flags = CLK_GATE_SET_TO_DISABLE;

    //flag = CLK_GET_RATE_NOCACHE | CLK_IGNORE_UNUSED; //remove ignore_unused flag when all drivers use clk framework, so some clks will be gated
    if(!of_property_read_u32(node, "ignore", &ignore))
    {
        if(ignore)
        {
            pr_debug("<%s> ignore gate clock\n",clk_name);
            flag = CLK_IGNORE_UNUSED;
        }
    }
    if(of_property_read_u32(node, "mux-shift", &mux_shift))
    {
        pr_debug("<%s> no mux-shift, treat as gate clock\n", clk_name);
        mux->shift = 0xFF;
    }
    else
        mux->shift = (u8)mux_shift;

    if(of_property_read_u32(node, "mux-width", &mux_width))
    {
        printk("<%s> no mux-width, set to default 2 bits\n", clk_name);
        mux->mask = BIT(2) - 1;
    }
    else
        mux->mask = BIT((u8)mux_width) - 1;

    if(of_property_read_u32(node, "glitch-shift", &mux_glitch))
    {
        mux->glitch = 0;
    }
    else
        mux->glitch = (u8)mux_glitch;

    if(of_property_read_u32(node, "gate-shift", &bit_idx))
    {
        pr_debug("<%s> no gate-shift, can not be gated\n", clk_name);
        gate->bit_idx = 0xFF;
    }
    else
        gate->bit_idx = (u8)bit_idx;

    pr_debug("[%s]\nmux->reg=0x%08X\nmux->shift=%d\nmux->width=%d\nmux->glitch=%d\ngate->bit_idx=%d\n",
                      clk_name, (unsigned int)mux->reg, mux->shift, mux_width, mux->glitch, gate->bit_idx);

    if(mux->shift != 0xFF && gate->bit_idx != 0xFF)
    {
        clk = clk_register_composite(NULL, clk_name, parent_names, num_parents,
                                    &mux->hw, &ms_clk_mux_ops,
                                    &mux->hw, &ms_clk_mux_ops,
                                    &gate->hw, &clk_gate_ops, flag);
    }
    else if(mux->shift != 0xFF)
    {
        clk = clk_register_composite(NULL, clk_name, parent_names, num_parents,
                                    &mux->hw, &ms_clk_mux_ops,
                                    &mux->hw, &ms_clk_mux_ops,
                                    NULL, NULL, flag);
        kfree(gate);
        gate = NULL;
    }
    else if(gate->bit_idx != 0xFF)
    {
        clk = clk_register_composite(NULL, clk_name, parent_names, num_parents,
                                    NULL, NULL,
                                    NULL, NULL,
                                    &gate->hw, &clk_gate_ops, flag);
//        kfree(mux);
    }
    else
    {
        pr_err("clock <%s> info err\n", clk_name);
        goto fail;
    }

    if (IS_ERR(clk))
    {
        pr_err("%s: register clock <%s> fail\n", __func__, clk_name);
        goto fail;
    }

    of_clk_add_provider(node, of_clk_src_simple_get, clk);
    clk_register_clkdev(clk, clk_name, NULL);

    if(!of_property_read_u32(node, "auto-enable", &auto_enable))
    {
        if (auto_enable)
        {
            clk_prepare_enable(clk);
#ifdef CONFIG_PM_SLEEP
            //keep auto-enable clocks into list for restoring clock in resume
            mux->gate = gate;
            list_add(&mux->list, &ms_clk_mux_list);
#endif
            pr_debug("clk_prepare_enable <%s>\n", clk_name);
        }
    }
#ifdef CONFIG_PM_SLEEP
    if (!_is_syscore_register) {
        register_syscore_ops(&ms_clk_mux_syscore_ops);
        _is_syscore_register = 1;
        #if DEBUG_AUTOEN
                clk_auto_proc();
        #endif
    }
#endif
    return;

fail:
    kfree(parent_names);
    kfree(mux);
    kfree(gate);
    return;
}

CLK_OF_DECLARE(ms_clk_composite, "sstar,composite-clock", ms_clk_composite_init);
