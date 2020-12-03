/*
* ms_complex_clk.c- Sigmastar
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
#include <linux/delay.h>
#include "ms_platform.h"
#include "registers.h"
#include "voltage_ctrl.h"


#define CLK_DEBUG  0

#if CLK_DEBUG
#define CLK_DBG(fmt, arg...) printk(KERN_DEBUG fmt, ##arg)
#else
#define CLK_DBG(fmt, arg...)
#endif
#define CLK_ERR(fmt, arg...) printk(KERN_ERR fmt, ##arg)

static long ms_cpuclk_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    //CLK_DBG("ms_cpuclk_round_rate = %lu\n", rate);

    if(rate < 200000000)
    {
        return 200000000;
    }
    else if(rate > 1400000000)
    {
        return 1400000000;
    }
    else
    {
        return rate;
    }
}

static unsigned long ms_cpuclk_recalc_rate(struct clk_hw *clk_hw, unsigned long parent_rate)
{
    unsigned long rate;
    U32 lpf_value;
    U32 post_div;

    //get LPF high
    lpf_value = INREG16(BASE_REG_RIU_PA + (0x1032A4 << 1)) +
                   (INREG16(BASE_REG_RIU_PA + (0x1032A6 << 1)) << 16);
    post_div = INREG16(BASE_REG_RIU_PA + (0x103232 << 1)) + 1;

    if(lpf_value == 0) // special handling for 1st time aquire after system boot
    {
        lpf_value= (INREG8(BASE_REG_RIU_PA + (0x1032C2 << 1)) <<  16) +
        (INREG8(BASE_REG_RIU_PA + (0x1032C1 << 1)) << 8) +
        INREG8(BASE_REG_RIU_PA + (0x1032C0 << 1));
        CLK_DBG("lpf_value = %u, post_div=%u\n", lpf_value, post_div);
    }

    /*
     * Calculate LPF value for DFS
     * LPF_value(5.19) = (432MHz / Ref_clk) * 2^19  =>  it's for post_div=2
     * Ref_clk = CPU_CLK * 2 / 32
     */
    rate = (div64_u64(432000000llu * 524288, lpf_value ) * 2 / post_div * 32 / 2);

    CLK_DBG("ms_cpuclk_recalc_rate = %lu, prate=%lu\n", rate, parent_rate);

    return rate;
}

void cpu_dvfs(U32 u32TargetLpf, U32 u32TargetPostDiv)
{
    U32 u32CurrentPostDiv = 0;
    U32 u32TempPostDiv = 0;

    u32CurrentPostDiv = INREGMSK16(BASE_REG_RIU_PA + (0x103232 << 1), 0x000F) + 1;

    if (u32TargetPostDiv > u32CurrentPostDiv)
    {
        u32TempPostDiv = u32CurrentPostDiv;
        while (u32TempPostDiv != u32TargetPostDiv)
        {
            u32TempPostDiv = u32TempPostDiv<<1;
            OUTREGMSK16(BASE_REG_RIU_PA + (0x103232 << 1), u32TempPostDiv-1, 0x000F);
        }
    }

    OUTREG16(BASE_REG_RIU_PA + (0x1032A8 << 1), 0x0000);                    //reg_lpf_enable = 0
    OUTREG16(BASE_REG_RIU_PA + (0x1032AE << 1), 0x000F);                    //reg_lpf_update_cnt = 32
    OUTREG16(BASE_REG_RIU_PA + (0x1032A4 << 1), u32TargetLpf&0xFFFF);       //set target freq to LPF high
    OUTREG16(BASE_REG_RIU_PA + (0x1032A6 << 1), (u32TargetLpf>>16)&0xFFFF); //set target freq to LPF high
    OUTREG16(BASE_REG_RIU_PA + (0x1032B0 << 1), 0x0001);                    //switch to LPF control
    SETREG16(BASE_REG_RIU_PA + (0x1032B2 << 1), BIT12);                     //from low to high
    OUTREG16(BASE_REG_RIU_PA + (0x1032A8 << 1), 0x0001);                    //reg_lpf_enable = 1
    while( !(INREG16(BASE_REG_RIU_PA + (0x1032BA << 1))&BIT0) ); //polling done
    OUTREG16(BASE_REG_RIU_PA + (0x1032A0 << 1), u32TargetLpf&0xFFFF);       //store freq to LPF low
    OUTREG16(BASE_REG_RIU_PA + (0x1032A2 << 1), (u32TargetLpf>>16)&0xFFFF); //store freq to LPF low

    if (u32TargetPostDiv < u32CurrentPostDiv)
    {
        u32TempPostDiv = u32CurrentPostDiv;
        while (u32TempPostDiv != u32TargetPostDiv)
        {
            u32TempPostDiv = u32TempPostDiv>>1;
            OUTREGMSK16(BASE_REG_RIU_PA + (0x103232 << 1), u32TempPostDiv-1, 0x000F);
        }
    }
}

static int ms_cpuclk_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    int ret = 0;

    unsigned int lpf_value;
    unsigned int post_div = 2;

    CLK_DBG("ms_cpuclk_set_rate = %lu\n", rate);

    /*
     * The default of post_div is 2, choose appropriate post_div by CPU clock.
     */
    if (rate >= 800000000)
        post_div = 2;
    else if (rate >= 400000000)
        post_div = 4;
    else if (rate >= 200000000)
        post_div = 8;
    else
        post_div = 16;

    /*
     * Calculate LPF value for DFS
     * LPF_value(5.19) = (432MHz / Ref_clk) * 2^19  =>  it's for post_div=2
     * Ref_clk = CPU_CLK * 2 / 32
     */

    lpf_value = (U32)(div64_u64(432000000llu * 524288, (rate*2/32) * post_div / 2));

    cpu_dvfs(lpf_value, post_div);

    return ret;
}

void ms_cpuclk_init(struct clk_hw *clk_hw)
{
    return;
}


void ms_cpuclk_dvfs_disable(void)
{
    return;
}
EXPORT_SYMBOL(ms_cpuclk_dvfs_disable);

struct clk_ops ms_cpuclk_ops = {
    .round_rate = ms_cpuclk_round_rate,
    .recalc_rate = ms_cpuclk_recalc_rate,
    .set_rate = ms_cpuclk_set_rate,
    .init = ms_cpuclk_init,
};


static void __init ms_clk_complex_init(struct device_node *node)
{
    struct clk *clk;
    struct clk_hw *clk_hw = NULL;
    struct clk_init_data *init = NULL;
    struct clk_ops *clk_ops =NULL;
    const char **parent_names = NULL;
    u32 i;

    clk_hw = kzalloc(sizeof(*clk_hw), GFP_KERNEL);
    init = kzalloc(sizeof(*init), GFP_KERNEL);
    clk_ops = kzalloc(sizeof(*clk_ops), GFP_KERNEL);
    if (!clk_hw || !init || !clk_ops)
        goto fail;

    clk_hw->init = init;
    init->name = node->name;
    init->ops = clk_ops;

    //hook callback ops for cpuclk
    if(!strcmp(node->name, "CLK_cpupll_clk"))
    {
        CLK_ERR("Find %s, hook ms_cpuclk_ops\n", node->name);
        init->ops = &ms_cpuclk_ops;
    }
    else
    {
        CLK_DBG("Find %s, but no ops\n", node->name);
    }

    init->num_parents = of_clk_get_parent_count(node);
    if (init->num_parents < 1)
    {
        CLK_ERR("[%s] %s have no parent\n", __func__, node->name);
        goto fail;
    }

    parent_names = kzalloc(sizeof(char *) * init->num_parents, GFP_KERNEL);
    if (!parent_names)
        goto fail;

    for (i = 0; i < init->num_parents; i++)
        parent_names[i] = of_clk_get_parent_name(node, i);

    init->parent_names = parent_names;
    clk = clk_register(NULL, clk_hw);
    if(IS_ERR(clk))
    {
        CLK_ERR("[%s] Fail to register %s\n", __func__, node->name);
        goto fail;
    }
    else
    {
        CLK_DBG("[%s] %s register success\n", __func__, node->name);
    }
    of_clk_add_provider(node, of_clk_src_simple_get, clk);
    clk_register_clkdev(clk, node->name, NULL);
    return;

fail:
    kfree(parent_names);
    kfree(clk_ops);
    kfree(init);
    kfree(clk_hw);
}

CLK_OF_DECLARE(ms_clk_complex, "sstar,complex-clock", ms_clk_complex_init);
