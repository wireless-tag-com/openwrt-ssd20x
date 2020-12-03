/*
* cpufreq.c- Sigmastar
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
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/clk-provider.h>

#include "ms_types.h"
#include "ms_platform.h"
#include "infinity/registers.h"

u8 enable_scaling_voltage = 1;
u32 sidd_th_100x = 1243;  //sidd threshold=12.74mA

extern void ms_cpuclk_dvfs_disable(void);
extern int bga_vid_0;
extern int bga_vid_1;
extern int qfp_vid_0;

static int ms_cpufreq_verify(struct cpufreq_policy *policy)
{
    if (policy->cpu)
        return -EINVAL;

    cpufreq_verify_within_cpu_limits(policy);
    return 0;
}

static int ms_cpufreq_target(struct cpufreq_policy *policy, unsigned int target_freq, unsigned int relation)
{
    struct cpufreq_freqs freqs;
    int ret;

    freqs.old = policy->cur;
    freqs.new = target_freq;

	if (freqs.old == freqs.new)
		return 0;

    cpufreq_freq_transition_begin(policy, &freqs);
    ret = clk_set_rate(policy->clk, target_freq * 1000);
    cpufreq_freq_transition_end(policy, &freqs, ret);

    return ret;
}

static ssize_t show_scaling_voltage(struct kobject *kobj, struct attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%d\n", enable_scaling_voltage);

    return (str - buf);
}

static ssize_t store_scaling_voltage(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
    u32 enable;
    sscanf(buf, "%d", &enable);
    if(enable)
    {
        enable_scaling_voltage=1;
        pr_info("[CPUFREQ] voltage-scaling ON\n");
    }
    else
    {
        enable_scaling_voltage=0;
        pr_info("[CPUFREQ] voltage-scaling OFF\n");
    }
    return count;
}
define_one_global_rw(scaling_voltage);

static ssize_t show_cpufreq_testout(struct kobject *kobj, struct attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    u16 reg_value = 0;
    u32 freq = 0;

    reg_value = (INREG8(BASE_REG_RIU_PA + (0x101EE2 << 1)) | (INREG8(BASE_REG_RIU_PA + (0x101EE2 << 1)+1)<<8));
    freq = (reg_value * 4000)/83333;

    str += scnprintf(str, end - str, "CPU freq = %d MHz\n", freq);

    return (str - buf);
}

static ssize_t store_cpufreq_testout(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
    u32 enable;
    sscanf(buf, "%d", &enable);
    if(enable)
    {
        pr_info("[CPUFREQ] Freq testout On\n");
        OUTREG8(BASE_REG_RIU_PA + (0x102216 << 1), 0x01);
        OUTREG8(BASE_REG_RIU_PA + (0x101EEE << 1), 0x04);
        OUTREG8(BASE_REG_RIU_PA + (0x101EEA << 1), 0x04);
        OUTREG8(BASE_REG_RIU_PA + (0x101EEA << 1)+1, 0x40);
        OUTREG8(BASE_REG_RIU_PA + (0x101EEC << 1), 0x01);
        OUTREG8(BASE_REG_RIU_PA + (0x101EE0 << 1)+1, 0x00);
        OUTREG8(BASE_REG_RIU_PA + (0x101EE0 << 1)+1, 0x80);
    }

    return count;
}
define_one_global_rw(cpufreq_testout);


static ssize_t show_vid_gpio_map(struct kobject *kobj, struct attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "\nBGA_VID_0=%d, BGA_VID_1=%d, QFP_VID_0=%d\n", bga_vid_0, bga_vid_1, qfp_vid_0);

    return (str - buf);
}
define_one_global_ro(vid_gpio_map);


static int ms_cpufreq_init(struct cpufreq_policy *policy)
{
    int reg, sidd;
    MS_PACKAGE_TYPE package;

    package = Chip_Get_Package_Type();

    if (policy->cpu != 0)
        return -EINVAL;

    policy->min = 400000;
    policy->max = 800000;
    policy->cpuinfo.min_freq = 400000;
    policy->cpuinfo.max_freq = 1000000;
    policy->cpuinfo.transition_latency = 100000;
    policy->clk = of_clk_get(of_find_node_by_type(NULL, "cpu"), 0);

    //2016.08.09: use LPF to scale freq.
    OUTREG16(BASE_REG_RIU_PA + (0x1032A0 << 1), INREG16(BASE_REG_RIU_PA + (0x1032C0 << 1)));  //store freq to LPF low
    OUTREG16(BASE_REG_RIU_PA + (0x1032A2 << 1), INREG16(BASE_REG_RIU_PA + (0x1032C2 << 1)));

    reg = (INREG16(BASE_REG_EFUSE_PA + REG_ID_06) >> 12) | ((INREG16(BASE_REG_EFUSE_PA + REG_ID_07) & 0x3F) << 4);
    sidd = reg * 20;  // sidd = reg / 5 => multiplied by 100 => sidd_100x = reg * 20
    if(sidd >= sidd_th_100x)
    {
        enable_scaling_voltage=0;
        pr_info("voltage-scaling OFF\n");
        ms_cpuclk_dvfs_disable();
    }

    pr_info("[%s] Current clk=%lu, sidd_100x=%d, th_100x=%d\n", __func__, __clk_get_rate(policy->clk), sidd, sidd_th_100x);

    return 0;
}

static int ms_cpufreq_exit(struct cpufreq_policy *policy)
{
    if (policy && !IS_ERR(policy->clk))
        clk_put(policy->clk);

    return 0;
}

static struct cpufreq_driver ms_cpufreq_driver = {
    .verify = ms_cpufreq_verify,
    .target = ms_cpufreq_target,
    .get    = cpufreq_generic_get,
    .init   = ms_cpufreq_init,
    .exit   = ms_cpufreq_exit,
    .name   = "Mstar cpufreq",
};

static int ms_cpufreq_probe(struct platform_device *pdev)
{
    int ret;

    ret = cpufreq_sysfs_create_file(&scaling_voltage.attr);
    ret |= cpufreq_sysfs_create_file(&cpufreq_testout.attr);
    ret |= cpufreq_sysfs_create_file(&vid_gpio_map.attr);


    if (ret)
    {
        pr_err("[%s] create file fail\n", __func__);
    }

    return cpufreq_register_driver(&ms_cpufreq_driver);
}

static int ms_cpufreq_remove(struct platform_device *pdev)
{
    return cpufreq_unregister_driver(&ms_cpufreq_driver);
}

static const struct of_device_id ms_cpufreq_of_match_table[] = {
    { .compatible = "sstar,infinity-cpufreq" },
    {}
};
MODULE_DEVICE_TABLE(of, ms_cpufreq_of_match_table);

static struct platform_driver ms_cpufreq_platdrv = {
    .driver = {
        .name  = "ms_cpufreq",
        .owner = THIS_MODULE,
        .of_match_table = ms_cpufreq_of_match_table,
    },
    .probe  = ms_cpufreq_probe,
    .remove = ms_cpufreq_remove,
};

module_platform_driver(ms_cpufreq_platdrv);
