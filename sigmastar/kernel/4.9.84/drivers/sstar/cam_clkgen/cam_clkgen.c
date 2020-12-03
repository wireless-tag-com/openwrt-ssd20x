/*
* cam_clkgen.c- Sigmastar
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

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>

unsigned long CamClkGetRate(struct clk *clk)
{
    return clk_get_rate(clk);
}

unsigned long CamClkHwGetNumParents(const struct clk_hw *hw)
{
    return clk_hw_get_num_parents(hw);
}

struct clk_hw *CamClkGetParentByIndex(const struct clk_hw *hw, unsigned int index)
{
    return clk_hw_get_parent_by_index(hw, index);
}

int CamClkPrepare(struct clk *clk)
{
    return clk_prepare(clk);
}

int CamClkEnable(struct clk *clk)
{
    return clk_enable(clk);
}

void CamClkUnprepare(struct clk *clk)
{
    clk_unprepare(clk);
}

void CamClkDisable(struct clk *clk)
{
    clk_disable(clk);
}

/* clk_disable_unprepare helps cases using clk_disable in non-atomic context. */
static inline void CamClkDisableUnprepare(struct clk *clk)
{
    CamClkDisable(clk);
    CamClkUnprepare(clk);
}

int CamClkSetRate(struct clk *clk, unsigned long rate)
{
    return clk_set_rate(clk, rate);
}

int CamClkSetParent(struct clk *clk, struct clk *parent)
{
    return clk_set_parent(clk, parent);
}

long CamClkRoundRate(struct clk *clk, unsigned long rate)
{
    return clk_round_rate(clk, rate);
}

struct clk_hw *__CamClkGetHw(struct clk *clk)
{
    return __clk_get_hw(clk);
}

/**
 * of_clk_get_parent_count() - Count the number of clocks a device node has
 * @np: device node to count
 *
 * Returns: The number of clocks that are possible parents of this node
 */
unsigned int CamOfClkGetParentCount(struct device_node *np)
{
    int count;

    count = of_count_phandle_with_args(np, "clocks", "#clock-cells");
    if (count < 0)
        return 0;

    return count;
}

const char *CamOfClkGetParentName(struct device_node *np, int index)
{
    return of_clk_get_parent_name(np, index);
}


/**
 * of_address_to_resource - Translate device tree address and return as resource
 *
 * Note that if your address is a PIO address, the conversion will fail if
 * the physical address can't be internally converted to an IO token with
 * pci_address_to_pio(), that is because it's either called to early or it
 * can't be matched to any host bridge IO space
 */
int msys_of_address_to_resource(struct device_node *dev, int index,
			   struct resource *r)
{
	return of_address_to_resource(dev, index, r);
}

int msys_of_irq_to_resource(struct device_node *dev, int index,
			   struct resource *r)
{
	return of_irq_to_resource(dev, index, r);
}

int msys_of_property_read_u32_index(const struct device_node *np,
				       const char *propname,
				       u32 index, u32 *out_value)
{
    return of_property_read_u32_index(np, propname, index, out_value);
}

int msys_of_property_read_variable_u32_array(const struct device_node *np,
			       const char *propname, u32 *out_values,
			       size_t sz_min, size_t sz_max)
{
    return of_property_read_variable_u32_array(np, propname, out_values, sz_min, sz_max);
}
/**
 * platform_get_resource - get a resource for a device
 * @dev: platform device
 * @type: resource type
 * @num: resource index
 */
struct resource *msys_platform_get_resource(struct platform_device *dev,
				       unsigned int type, unsigned int num)
{
    return platform_get_resource(dev, type, num);
}

/**
 * platform_driver_unregister - unregister a driver for platform-level devices
 * @drv: platform driver structure
 */
void msys_platform_driver_unregister(struct platform_driver *drv)
{
    platform_driver_unregister(drv);
}


/**
 * platform_get_irq_byname - get an IRQ for a device by name
 * @dev: platform device
 * @name: IRQ name
 */
int msys_platform_get_irq_byname(struct platform_device *dev, const char *name)
{
    return platform_get_irq_byname(dev, name);
}

int __must_check msys_sysfs_create_file(struct kobject *kobj, const struct attribute * attr)
{
    return sysfs_create_file_ns(kobj, attr, NULL);
}

int msys_sysfs_create_files(struct kobject *kobj, const struct attribute **ptr)
{
    return sysfs_create_files(kobj, ptr);
}

int msys_sysfs_create_link(struct kobject *kobj, struct kobject *target, const char *name)
{
    return sysfs_create_link(kobj, target, name);
}

void msys_sysfs_remove_file(struct kobject *kobj, const struct attribute *attr, const void *ns)
{
	sysfs_remove_file_ns(kobj, attr, NULL);
}

void msys_sysfs_remove_files(struct kobject *kobj, const struct attribute **attr)
{
	sysfs_remove_files(kobj, attr);
}

EXPORT_SYMBOL(msys_sysfs_remove_files);
EXPORT_SYMBOL(msys_sysfs_remove_file);
EXPORT_SYMBOL(msys_sysfs_create_link);
EXPORT_SYMBOL(msys_sysfs_create_files);
EXPORT_SYMBOL(msys_sysfs_create_file);
EXPORT_SYMBOL(msys_platform_get_irq_byname);
EXPORT_SYMBOL(msys_platform_driver_unregister);
EXPORT_SYMBOL(msys_platform_get_resource);
EXPORT_SYMBOL(msys_of_property_read_variable_u32_array);
EXPORT_SYMBOL(msys_of_property_read_u32_index);
EXPORT_SYMBOL(msys_of_irq_to_resource);
EXPORT_SYMBOL(msys_of_address_to_resource);
EXPORT_SYMBOL(CamOfClkGetParentName);
EXPORT_SYMBOL(CamOfClkGetParentCount);
EXPORT_SYMBOL(__CamClkGetHw);
EXPORT_SYMBOL(CamClkRoundRate);
EXPORT_SYMBOL(CamClkSetParent);
EXPORT_SYMBOL(CamClkSetRate);
EXPORT_SYMBOL(CamClkPrepare);
EXPORT_SYMBOL(CamClkDisable);
EXPORT_SYMBOL(CamClkEnable);
EXPORT_SYMBOL(CamClkUnprepare);
EXPORT_SYMBOL(CamClkGetParentByIndex);
EXPORT_SYMBOL(CamClkHwGetNumParents);
EXPORT_SYMBOL(CamClkGetRate);

