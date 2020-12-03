/*
* clk.c- Sigmastar
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
/*
 * ms_clk-cedric.c
 *
 *  Created on: 2015�~5��4��
 *      Author: Administrator
 */
#include <linux/kernel.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/of.h>

#include "cedric/irqs.h"
#include "ms_platform.h"



#define PERI_PHYS 0x16000000



static unsigned long clk_cpu_recalc_rate(struct clk_hw *hwclk, unsigned long parent_rate)
{

    return (parent_rate>>1)*ms_readl(0x1F22184C);
}

static const struct clk_ops clk_cpu_ops = {
    .recalc_rate = clk_cpu_recalc_rate,
//    .round_rate = clk_cpu_round_rate,
//    .set_rate = clk_cpu_set_rate,
};




#if defined(CONFIG_HAVE_ARM_TWD)
#include <asm/smp_twd.h>
DEFINE_TWD_LOCAL_TIMER(twd_local_timer, (PERI_PHYS+0x0600), GIC_ID_LOCAL_TIMER_IRQ);
#endif


static struct clk_onecell_data clk_cpu_data;
static struct clk_hw clk_cpu_hw;
static void __init ms_clk_cpu_of_init(struct device_node *node)
{

    struct clk_init_data init;
    const char *parent_name;

    clk_cpu_data.clk_num=2;
    clk_cpu_data.clks = kzalloc(clk_cpu_data.clk_num*sizeof(struct clk *),GFP_KERNEL);
    BUG_ON(!clk_cpu_data.clks);

    parent_name = of_clk_get_parent_name(node, 0);

    clk_cpu_hw.init = &init;
    init.ops = &clk_cpu_ops;
    init.name=node->name;
    init.parent_names = parent_name ? &parent_name : NULL;
    init.num_parents = parent_name ? 1 : 0;
    init.flags = (parent_name)? 0 : CLK_IS_ROOT;


    clk_cpu_data.clks[0]=clk_register(NULL, &clk_cpu_hw);;
    BUG_ON(IS_ERR(clk_cpu_data.clks[0]));


    clk_cpu_data.clks[1]=clk_register_fixed_factor(NULL, "periclk", node->name, 0, 1, 2);
    BUG_ON(IS_ERR(clk_cpu_data.clks[1]));


    of_clk_add_provider(node, of_clk_src_onecell_get, &clk_cpu_data);


//  since we are not using GIC, need to register TWD manually
#if defined(CONFIG_HAVE_ARM_TWD)
    {
        clk_register_clkdev(clk_cpu_data.clks[1], NULL, "smp_twd");

        if (twd_local_timer_register(&twd_local_timer))
        {
            pr_err("twd_local_timer_register failed!!!\n");
        }
        else
        {
            pr_err("twd_local_timer_register success... \n");
        }
    }

#endif
}

static void __init ms_clk_uart_of_init(struct device_node *node)
{
    struct clk *clk;
    const char *clk_name = node->name;
    int num_parents;
    const char **parent_names;
    struct clk_mux *mux = NULL;
    struct clk_gate *gate = NULL;
    void __iomem *reg;
    unsigned int mux_width;
    int i = 0;

    pr_info("%s: %s", __func__, clk_name);

    num_parents = of_clk_get_parent_count(node);
    if (num_parents < 2) {
        pr_err("%s: %s must have at least 2 parents\n", __func__, node->name);
        return;
    }

    parent_names = kzalloc((sizeof(char *) * num_parents), GFP_KERNEL);
    mux = kzalloc(sizeof(*mux), GFP_KERNEL);
    gate = kzalloc(sizeof(*gate), GFP_KERNEL);

    if (!parent_names || !mux || !gate)
    {
        pr_err("%s: failed to allocate memory\n", __func__);
        goto fail;
    }

    for (i = 0; i < num_parents; i++)
        parent_names[i] = of_clk_get_parent_name(node, i);

    reg = of_iomap(node, 0);
    if (!reg) {
        pr_err("%s: could not map region\n", __func__);
        goto fail;       
    }

    mux->reg = reg;
    if(of_property_read_u32(node, "mux-shift", (unsigned int *)&mux->shift))
    {
        pr_err("%s: failed to read mux-shift\n", __func__);
        mux->shift = 0;
    }
    if(of_property_read_u32(node, "mux-width", &mux_width))
    {
        pr_err("%s: failed to read mux-width\n", __func__);
        mux_width = 3;
    }
    mux->mask = BIT(mux_width) - 1;
    //mux->flags = CLK_SET_RATE_PARENT;

    gate->reg = reg;
    if(of_property_read_u32(node, "gate-shift", (unsigned int *)&gate->bit_idx))
    {
        pr_err("%s: failed to read gate-shift\n", __func__);
        gate->bit_idx = 0;
    }

    pr_debug("%s: mux->reg=0x%08X\nmux->shift=%d\nmux->mask=0x%08X\n", __func__, (unsigned int)mux->reg, mux->shift, mux->mask);

    clk = clk_register_composite(NULL, clk_name, parent_names, num_parents,
                     &mux->hw, &clk_mux_ops,
                     NULL, NULL,
                     &gate->hw, &clk_gate_ops,
                     0);

    if (IS_ERR(clk))
    {
        pr_err("%s: failed to register clock %s\n", __func__, clk_name);
        goto fail;
    }

    of_clk_add_provider(node, of_clk_src_simple_get, clk);
    clk_register_clkdev(clk, clk_name, NULL);
    return;

fail:
    kfree(parent_names);
    kfree(mux);
    kfree(gate);
    return;
}

CLK_OF_DECLARE(cedric_uart_clk, "sstar,cedric-uartclk", ms_clk_uart_of_init);
CLK_OF_DECLARE(cedric_cpu_clk, "sstar,cedric-cpuclk",ms_clk_cpu_of_init);
