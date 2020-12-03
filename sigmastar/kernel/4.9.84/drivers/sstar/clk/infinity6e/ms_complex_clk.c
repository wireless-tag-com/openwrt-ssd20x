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
#define CLK_DBG(fmt, arg...) printk(KERN_INFO fmt, ##arg)
#else
#define CLK_DBG(fmt, arg...)
#endif
#define CLK_ERR(fmt, arg...) printk(KERN_ERR fmt, ##arg)

static unsigned int ms_get_ddr_scl(void)
{
    unsigned int factor = INREG16(BASE_REG_RIU_PA + (0x1032A0 << 1)) | (((INREG16(BASE_REG_RIU_PA + (0x1032A2 << 1)) & 0xFF) <<16));

    if(!factor)
        factor = (INREG16(0x1F206580) | ((INREG16(0x1F206584) & 0xFF) << 16));

    CLK_DBG("ms_get_ddr_scl = 0x%X\n", factor);
    return factor;
}

static long ms_cpuclk_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    //CLK_DBG("ms_cpuclk_round_rate = %lu\n", rate);

    if(rate < 100000000)
    {
        return 100000000;
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

    rate = (parent_rate / ms_get_ddr_scl()) << 23;

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

static int ms_upll_utmi_enable(struct clk_hw *hw)
{
    CLK_DBG("\nms_upll_enable\n\n");
    OUTREG16(BASE_REG_UPLL0_PA + REG_ID_00, 0x00C0);
    OUTREG8(BASE_REG_UPLL0_PA + REG_ID_07, 0x01);

    CLRREG16(BASE_REG_UTMI0_PA + REG_ID_00, BIT15); //reg_pdn=0
    CLRREG16(BASE_REG_UTMI0_PA + REG_ID_04, BIT7); //pd_bg_current=0
    return 0;
}

static void ms_upll_utmi_disable(struct clk_hw *hw)
{
    CLK_DBG("\nms_upll_disable\n\n");
    OUTREG16(BASE_REG_UPLL0_PA + REG_ID_00, 0x01B2);
    OUTREG8(BASE_REG_UPLL0_PA + REG_ID_07, 0x02);

    SETREG16(BASE_REG_UTMI0_PA + REG_ID_00, BIT15); //reg_pdn=1
    SETREG16(BASE_REG_UTMI0_PA + REG_ID_04, BIT7); //pd_bg_current=1
}

static int ms_upll_utmi_is_enabled(struct clk_hw *hw)
{
    CLK_DBG("\nms_upll_is_enabled\n\n");
    return (INREG8(BASE_REG_UPLL0_PA + REG_ID_07) & BIT0);
}

static unsigned long ms_upll_utmi_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    CLK_DBG("\nms_upll_utmi_recalc_rate, parent_rate=%lu\n\n", parent_rate);
    return (parent_rate * 40);
}


struct clk_ops ms_upll_utmi_ops = {
    .enable = ms_upll_utmi_enable,
    .disable = ms_upll_utmi_disable,
    .is_enabled = ms_upll_utmi_is_enabled,
    .recalc_rate = ms_upll_utmi_recalc_rate,
};

static int ms_usb_enable(struct clk_hw *hw)
{
    CLK_DBG("\nms_usb_enable\n\n");

    OUTREG16(BASE_REG_UTMI0_PA + REG_ID_04, 0x0C2F);
    OUTREG16(BASE_REG_UTMI0_PA + REG_ID_04, 0x040F); //utmi0
    OUTREG16(BASE_REG_UTMI0_PA + REG_ID_00, 0x7F05);
    OUTREG8(BASE_REG_USB0_PA + REG_ID_00, 0x0A); //Disable MAC initial suspend, Reset UHC
    OUTREG8(BASE_REG_USB0_PA + REG_ID_00, 0x28); //Release UHC reset, enable UHC and OTG XIU function
    OUTREG16(BASE_REG_UTMI0_PA + REG_ID_11, 0x2088); //PLL_TEST[30:28]: 3'b101 for IBIAS current select
    OUTREG16(BASE_REG_UTMI0_PA + REG_ID_10, 0x8051); //PLL_TEST[15]: Bypass 480MHz clock divider
    OUTREG16(BASE_REG_UTMI0_PA + REG_ID_01, 0x2084); //Enable CLK12_SEL bit <2> for select low voltage crystal clock
    OUTREG16(BASE_REG_UTMI0_PA + REG_ID_04, 0x0426); //bit<7>: Power down UTMI port-0 bandgap current
    OUTREG16(BASE_REG_UTMI0_PA + REG_ID_00, 0x6BC3); //reg_pdn: bit<15>, bit <2> ref_pdn  # Turn on reference voltage and regulator
    //loop_delay_timer(TIMER_DELAY_100us);
    OUTREG16(BASE_REG_UTMI0_PA + REG_ID_00, 0x69C3); //Turn on UPLL, reg_pdn: bit<9>
    //loop_delay_timer(TIMER_DELAY_100us);
    OUTREG16(BASE_REG_UTMI0_PA + REG_ID_00, 0x0001); //Turn all (including hs_current) use override mode
    return 0;
}

static void ms_usb_disable(struct clk_hw *hw)
{
    CLK_DBG("\nms_usb_disable\n\n");
}

static int ms_usb_is_enabled(struct clk_hw *hw)
{
    CLK_DBG("\nms_usb_is_enabled\n\n");
    return (INREG8(BASE_REG_UTMI0_PA + REG_ID_00) == 0x0001);
}

struct clk_ops ms_usb_ops = {
    .enable = ms_usb_enable,
    .disable = ms_usb_disable,
    .is_enabled = ms_usb_is_enabled,
};

static int ms_venpll_enable(struct clk_hw *hw)
{
    CLRREG16(BASE_REG_VENPLL_PA + REG_ID_01, BIT8); //reg_ven_pll_pd=0
    return 0;
}

static void ms_venpll_disable(struct clk_hw *hw)
{
    SETREG16(BASE_REG_VENPLL_PA + REG_ID_01, BIT8); //reg_ven_pll_pd=1
}

static int ms_venpll_is_enabled(struct clk_hw *hw)
{
    return ((INREG16(BASE_REG_VENPLL_PA + REG_ID_01) & 0x0100) == 0x0000);
}

static unsigned long ms_venpll_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    return (parent_rate * INREG8(BASE_REG_VENPLL_PA + REG_ID_03) / 2);
}

static long ms_venpll_round_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long *parent_rate)
{
    if(rate <= 300000000)
    {
        return 300000000;
    }
    else if(rate <= 312000000)
    {
        return 312000000;
    }
    else if(rate <= 324000000)
    {
        return 324000000;
    }
    else if(rate <= 336000000)
    {
        return 336000000;
    }
    else if(rate <= 348000000)
    {
        return 348000000;
    }
    else{
        return 348000000;
    }
}

static int ms_venpll_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    if ((rate == 348000000) || (rate == 336000000) || (rate == 324000000) || (rate == 312000000) || (rate == 300000000)) {
        int val = rate * 2 / 24000000;
        OUTREG8(BASE_REG_VENPLL_PA + REG_ID_03, val);//reg_ven_pll_loop_div_second
    } else {
        CLK_ERR("\nunsupported venpll rate %lu\n\n", rate);
        return -1;
    }
    return 0;
}

struct clk_ops ms_venpll_ops = {
    .enable = ms_venpll_enable,
    .disable = ms_venpll_disable,
    .is_enabled = ms_venpll_is_enabled,
    .round_rate = ms_venpll_round_rate,
    .recalc_rate = ms_venpll_recalc_rate,
    .set_rate = ms_venpll_set_rate,
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
    else if(!strcmp(node->name, "CLK_utmi"))
    {
        CLK_ERR("Find %s, hook ms_upll_ops\n", node->name);
        init->ops = &ms_upll_utmi_ops;
    }
    else if(!strcmp(node->name, "CLK_usb"))
    {
        CLK_ERR("Find %s, hook ms_usb_ops\n", node->name);
        init->ops = &ms_usb_ops;
    }
    else if(!strcmp(node->name, "CLK_ven_pll"))  
    {
        CLK_ERR("Find %s, hook ms_venpll_ops\n", node->name);
        init->ops = &ms_venpll_ops;
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
