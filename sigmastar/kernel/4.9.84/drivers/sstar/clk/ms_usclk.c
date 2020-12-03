/*
* ms_usclk.c- Sigmastar
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
 * /sys/class/clk
 * usclk: usclk {
        compatible = "usclk";
        clocks = <&foo 15>, <&bar>;
        clock-count = <2>;
    };
*/
#include <linux/clk-provider.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/init.h>

#define DRIVER_NAME    "usclk"

struct usclk_data {
    struct clk* clk;
    struct clk_hw *hw;
    int enabled;
};

struct usclk_data *g_pdata;
struct class *g_clk_class;
struct device **g_dev;
u32 g_clock_count;

static ssize_t enable_show(struct device *dev, struct device_attribute *attr,
        char *buf)
{
    struct usclk_data *pdata = dev_get_drvdata(dev);

    if(!pdata->hw->init->ops->is_enabled)
        return scnprintf(buf, PAGE_SIZE, "%u\n", pdata->enabled);
    else
        return scnprintf(buf, PAGE_SIZE, "%u\n", pdata->hw->init->ops->is_enabled(pdata->hw));
}

static ssize_t enable_store(struct device *dev, struct device_attribute *attr,
        const char *buf, size_t count)
{
    unsigned long enable;
    int ret;
    struct usclk_data *pdata = dev_get_drvdata(dev);

    ret = kstrtoul(buf, 0, &enable);
    if (ret)
        return -EINVAL;

    enable = !!enable;

    if (enable)
        pdata->hw->init->ops->enable(pdata->hw);
    else
        pdata->hw->init->ops->disable(pdata->hw);

    pdata->enabled = enable;

    return count;
}

static DEVICE_ATTR(enable, 0644, enable_show, enable_store);

static ssize_t set_rate_show(struct device *dev, struct device_attribute *attr,
        char *buf)
{
    struct usclk_data *pdata = dev_get_drvdata(dev);

    return scnprintf(buf, PAGE_SIZE, "%lu\n", clk_hw_get_rate(pdata->hw));
}

static ssize_t set_rate_store(struct device *dev, struct device_attribute *attr,
        const char *buf, size_t count)
{
    int ret = 0;
    unsigned long rate;
    struct usclk_data *pdata = dev_get_drvdata(dev);

    ret = kstrtoul(buf, 0, &rate);
    if (ret)
    {
        return -EINVAL;
    }

    rate = clk_hw_round_rate(pdata->hw, rate);
    ret = clk_set_rate(pdata->hw->clk, rate);
    if (ret)
    {
        return -EBUSY;
    }
    else
    {
        pr_info("round rate to %lu\n", rate);
    }

    return count;
}

static DEVICE_ATTR(rate, 0644, set_rate_show, set_rate_store);

static const struct attribute *usclk_attrs[] = {
    &dev_attr_enable.attr,
    &dev_attr_rate.attr,
    NULL
};

static const struct attribute_group usclk_attr_grp = {
    .attrs = (struct attribute **)usclk_attrs,
};

static int usclk_setup(void)
{
    int ret;
    int i;
    struct device_node *np = of_find_compatible_node(NULL, NULL, "usclk");


//    printk(KERN_INFO"setup ms_usclk interface\n");
    ret = of_property_read_u32(np, "clock-count", &g_clock_count);
    if (ret || !g_clock_count)
        return ret;

    g_pdata = kzalloc(g_clock_count * sizeof(*g_pdata), GFP_KERNEL);
    if (!g_pdata)
        return -ENOMEM;

    g_dev = kzalloc(g_clock_count * sizeof(struct dev*), GFP_KERNEL);
    if (!g_dev)
        return -ENOMEM;

    g_clk_class = class_create(THIS_MODULE, "usclk");
    if (!g_clk_class) {
        pr_err("create class fail\n");
        goto err_free;
    }



    for (i = 0; i < g_clock_count; i++) {

        g_pdata[i].clk = of_clk_get(np, i);
        if (IS_ERR(g_pdata[i].clk))
        {
            continue;
        }
        g_pdata[i].hw = __clk_get_hw(g_pdata[i].clk);
        if (IS_ERR(g_pdata[i].hw)) {
            pr_warn("input clock #%u not found\n", i);
            clk_put(g_pdata[i].clk);
            continue;
        }

        g_dev[i] = device_create(g_clk_class, NULL, MKDEV(0, 0), NULL,
                of_clk_get_parent_name(np, i));

        if (!g_dev[i]) {
            pr_warn("unable to create device #%d\n", i);
            continue;
        }

        dev_set_drvdata(g_dev[i], &g_pdata[i]);
        if(0!=sysfs_create_group(&g_dev[i]->kobj, &usclk_attr_grp))
        {
            pr_warn("create device #%d failed...\n", i);
        }
    }

    printk(KERN_WARNING "ms_usclk: initialized\n");
    return 0;

err_free:
    if(g_pdata)
        kfree(g_pdata);

    if(g_dev)
        kfree(g_dev);

    return ret;
}
//

#ifdef CONFIG_MS_USCLK_MODULE

static int __init ms_usclk_module_init(void)
{
//    int retval=0;
//    retval = platform_driver_register(&ms_ir_driver);

    return usclk_setup();
//    return retval;
}

static void __exit ms_usclk_module_exit(void)
{
    int i;
//    platform_driver_unregis.ter(&ms_ir_driver);

    for (i = 0; i < g_clock_count; i++) {
        if(g_dev[i])
        {
            sysfs_remove_group(&g_dev[i]->kobj, &usclk_attr_grp);
            device_destroy(g_clk_class, g_dev[i]->devt);
        }
        if (!IS_ERR(g_pdata[i].clk))
        {
            clk_put(g_pdata[i].clk);
        }
    }

    if(g_clk_class)
        class_destroy(g_clk_class);

    if(g_pdata)
        kfree(g_pdata);

    if(g_dev)
        kfree(g_dev);

}


module_init(ms_usclk_module_init);
module_exit(ms_usclk_module_exit);


MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("usclk driver");
MODULE_LICENSE("GPL");

#else
late_initcall(usclk_setup);
#endif
