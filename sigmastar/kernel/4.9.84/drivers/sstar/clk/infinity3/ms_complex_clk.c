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
#include <linux/of_address.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/cpu.h>
#include <linux/delay.h>

#include "ms_types.h"
#include "ms_platform.h"
#include "infinity3/registers.h"

#if defined (CONFIG_MS_CPU_FREQ) && defined (CONFIG_MS_GPIO)
extern u8 enable_scaling_voltage;
#include "infinity3/gpio.h"
#include "mdrv_gpio.h"
int vid_0 = -1;
int vid_1 = -1;
#else
u8 enable_scaling_voltage=0;
#define MDrv_GPIO_Set_High(x) {}
#define MDrv_GPIO_Set_Low(x) {}
#define MDrv_GPIO_Pad_Set(x) {}
int vid_0 = -1;
int vid_1 = -1;
#endif

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

    if(rate <= 450000000)       //request 400M-450M=400M
    {
        return 400000000;
    }
    else if(rate <= 650000000)  //request 450M-650M=600M
    {
        return 600000000;
    }
    else if(rate <= 850000000)  //request 650M-850M=800M
    {
        return 800000000;
    }
    else                        //request 850M-1000M=1000M
    {
        return 1000000000;
    }
}

static unsigned long ms_cpuclk_recalc_rate(struct clk_hw *clk_hw, unsigned long parent_rate)
{
    unsigned long rate;

    rate = (parent_rate / ms_get_ddr_scl()) << 23;

    CLK_DBG("ms_cpuclk_recalc_rate = %lu, prate=%lu\n", rate, parent_rate);

    return rate;
}

void cpu_dvfs(U32 u32TargetSet)
{
    OUTREG16(BASE_REG_RIU_PA + (0x1032A4 << 1), u32TargetSet&0xFFFF); //set target freq to LPF high
    OUTREG16(BASE_REG_RIU_PA + (0x1032A6 << 1), (u32TargetSet>>16)&0xFFFF);
    OUTREG16(BASE_REG_RIU_PA + (0x1032B0 << 1), 0x0001); //switch to LPF control
    OUTREG16(BASE_REG_RIU_PA + (0x1032AA << 1), 0x0006); //mu[2:0]
    OUTREG16(BASE_REG_RIU_PA + (0x1032AE << 1), 0x0008); //lpf_update_cnt[7:0]
    SETREG16(BASE_REG_RIU_PA + (0x1032B2 << 1), BIT12);  //from low to high
    OUTREG16(BASE_REG_RIU_PA + (0x1032A8 << 1), 0x0000); //toggle LPF enable
    OUTREG16(BASE_REG_RIU_PA + (0x1032A8 << 1), 0x0001);
    while( !(INREG16(BASE_REG_RIU_PA + (0x1032BA << 1))&BIT0) ); //polling done
    OUTREG16(BASE_REG_RIU_PA + (0x1032A8 << 1), 0x0000);
    OUTREG16(BASE_REG_RIU_PA + (0x1032A0 << 1), u32TargetSet&0xFFFF);  //store freq to LPF low
    OUTREG16(BASE_REG_RIU_PA + (0x1032A2 << 1), (u32TargetSet>>16)&0xFFFF);
}



#define VOLTAGE_CORE_850   850
#define VOLTAGE_CORE_900   900
#define VOLTAGE_CORE_950   950
#define VOLTAGE_CORE_1000 1000
int g_sCurrentTemp = 35;
int g_sCurrentTempThreshLo=35;
int g_sCurrentTempThreshHi=50;
int g_sCurrentVoltageCore=VOLTAGE_CORE_1000;
void set_core_voltage (int vcore)
{
    if(enable_scaling_voltage)
    {
        switch (vcore)
        {
        case VOLTAGE_CORE_850:
            if(vid_0 != -1)
            {
                MDrv_GPIO_Pad_Set(vid_0);
                MDrv_GPIO_Set_Low(vid_0);
            }
            if(vid_1 != -1)
            {
                MDrv_GPIO_Pad_Set(vid_1);
                MDrv_GPIO_Set_Low(vid_1);
            }
            g_sCurrentVoltageCore = VOLTAGE_CORE_850;
            break;

        case VOLTAGE_CORE_900:
            if(vid_0 != -1)
            {
                MDrv_GPIO_Pad_Set(vid_0);
                MDrv_GPIO_Set_High(vid_0);
            }
            if(vid_1 != -1)
            {
                MDrv_GPIO_Pad_Set(vid_1);
                MDrv_GPIO_Set_Low(vid_1);
            }
            g_sCurrentVoltageCore = VOLTAGE_CORE_900;
            break;

        case VOLTAGE_CORE_950:
            if(vid_0 != -1)
            {
                MDrv_GPIO_Pad_Set(vid_0);
                MDrv_GPIO_Set_Low(vid_0);
            }
            if(vid_1 != -1)
            {
                MDrv_GPIO_Pad_Set(vid_1);
                MDrv_GPIO_Set_High(vid_1);
            }
            g_sCurrentVoltageCore = VOLTAGE_CORE_950;
            break;

        case VOLTAGE_CORE_1000:
            if(vid_0 != -1)
            {
                MDrv_GPIO_Pad_Set(vid_0);
                MDrv_GPIO_Set_High(vid_0);
            }
            if(vid_1 != -1)
            {
                MDrv_GPIO_Pad_Set(vid_1);
                MDrv_GPIO_Set_High(vid_1);
            }
            g_sCurrentVoltageCore = VOLTAGE_CORE_1000;
            break;

        }
    }
    CLK_DBG("CurrentVoltageCore = %d\n", g_sCurrentVoltageCore);
}
static int ms_cpuclk_set_rate(struct clk_hw *clk_hw, unsigned long rate, unsigned long parent_rate)
{
    int ret = 0;

//    if(abs(rate - __clk_get_rate(clk_hw->clk)) < 100000000)  //if it's the same setting, just return to save time
//        return 0;

    CLK_DBG("ms_cpuclk_set_rate = %lu\n", rate);

    if(rate == 1000000000)
    {
        set_core_voltage(VOLTAGE_CORE_1000);
        cpu_dvfs(0x374BC7);
    }
    else
    {
        CLK_DBG("ms_get_temp = %d\n", g_sCurrentTemp);
        #if 0
        if(g_sCurrentVoltageCore==VOLTAGE_CORE_1000 && g_sCurrentTemp>g_sCurrentTempThreshHi )
            set_core_voltage(VOLTAGE_CORE_900);
        if(g_sCurrentVoltageCore==VOLTAGE_CORE_900 && g_sCurrentTemp<g_sCurrentTempThreshLo )
            set_core_voltage(VOLTAGE_CORE_1000);
        #else
        if(g_sCurrentVoltageCore==VOLTAGE_CORE_1000)
            set_core_voltage(VOLTAGE_CORE_900);
        #endif

        if(rate == 400000000)
        {
            cpu_dvfs(0x8A3D71);
        }
        else if(rate == 600000000)
        {
            cpu_dvfs(0x5C28F6);
        }
        else if(rate == 800000000)
        {
            cpu_dvfs(0x451EB9);
        }
    }

    return ret;
}

void ms_cpuclk_init(struct clk_hw *clk_hw)
{
    struct device_node *np;
    u32 val;

    if((np=of_find_node_by_name(NULL, "cpufreq")))
    {
        if(!of_property_read_u32(np, "vid0-gpio", &val))
            vid_0 = val;
        else
            vid_0 = -1;

        if(!of_property_read_u32(np, "vid1-gpio", &val))
            vid_1 = val;
        else
            vid_1 = -1;
        if(vid_0!=-1 || vid_1!=-1)
            CLK_ERR("[%s] get dvfs gpio %s %s\n", __func__, (vid_0 != -1)?"vid_0":"", (vid_1 != -1)?"vid_1":"");
    }
    else
    {
        vid_0 = -1;
        vid_1 = -1;
        CLK_ERR("[%s] can't get cpufreq node for dvfs\n", __func__);
    }
}


void ms_cpuclk_dvfs_disable(void)
{
    if(vid_0 != -1)
    {
        MDrv_GPIO_Pad_Set(vid_0);
        MDrv_GPIO_Set_High(vid_0);
    }
    if(vid_1 != -1)
    {
        MDrv_GPIO_Pad_Set(vid_1);
        MDrv_GPIO_Set_High(vid_1);
    }
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
