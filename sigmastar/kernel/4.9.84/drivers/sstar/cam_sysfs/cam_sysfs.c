/*
* cam_sysfs.c- Sigmastar
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
#include <linux/pwm.h>
#include <linux/gpio.h>
/**
 * of_address_to_resource - Translate device tree address and return as resource
 *
 * Note that if your address is a PIO address, the conversion will fail if
 * the physical address can't be internally converted to an IO token with
 * pci_address_to_pio(), that is because it's either called to early or it
 * can't be matched to any host bridge IO space
 */
int CamOfAddressToResource(struct device_node *dev, int index,
                           struct resource *r)
{
        return of_address_to_resource(dev, index, r);
}

int CamOfIrqToResource(struct device_node *dev, int index,
                           struct resource *r)
{
        return of_irq_to_resource(dev, index, r);
}

int CamOfPropertyReadU32Index(const struct device_node *np,
                                       const char *propname,
                                       u32 index, u32 *out_value)
{
    return of_property_read_u32_index(np, propname, index, out_value);
}

int CamOfPropertyReadVariableU32Array(const struct device_node *np,
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
struct resource *CamPlatformGetResource(struct platform_device *dev,
                                       unsigned int type, unsigned int num)
{
    return platform_get_resource(dev, type, num);
}

int CamPlatformDriverRegister(struct platform_driver *drv)
{
    return platform_driver_register(drv);
}

/**
 * platform_driver_unregister - unregister a driver for platform-level devices
 * @drv: platform driver structure
 */
void CamPlatformDriverUnregister(struct platform_driver *drv)
{
    platform_driver_unregister(drv);
}

/**
 * platform_get_irq_byname - get an IRQ for a device by name
 * @dev: platform device
 * @name: IRQ name
 */
int CamPlatformGetIrqByname(struct platform_device *dev, const char *name)
{
    return platform_get_irq_byname(dev, name);
}

int __must_check CamSysfsCreateFile(struct kobject *kobj, const struct attribute * attr)
{
    return sysfs_create_file_ns(kobj, attr, NULL);
}

int CamSysfsCreateFiles(struct kobject *kobj, const struct attribute **ptr)
{
    return sysfs_create_files(kobj, ptr);
}

int CamSysfsCreateLink(struct kobject *kobj, struct kobject *target, const char *name)
{
    return sysfs_create_link(kobj, target, name);
}

void CamSysfsRemoveFile(struct kobject *kobj, const struct attribute *attr, const void *ns)
{
        sysfs_remove_file_ns(kobj, attr, NULL);
}

void CamSysfsRemoveFiles(struct kobject *kobj, const struct attribute **attr)
{
        sysfs_remove_files(kobj, attr);
}

struct device *CamDeviceCreate(struct class *class, struct device *parent,
                             dev_t devt, void *drvdata, const char *fmt, ...)
{
    return device_create(class, parent, devt, drvdata, fmt);
}

int CamDeviceCreateFile(struct device *dev,
                       const struct device_attribute *attr)
{
    return device_create_file(dev, attr);
}

void CamDeviceDestroy(struct class *class, dev_t devt)
{
    return device_destroy(class, devt);
}

void CamDeviceRemoveFile(struct device *dev,
                        const struct device_attribute *attr)
{
    return device_remove_file(dev, attr);
}

void CamDeviceUnregister(struct device *dev)
{
    return device_unregister(dev);
}

struct kobject *CamKobjectCreateAndAdd(const char *name, struct kobject *parent)
{
    return kobject_create_and_add(name, parent);
}

struct class * __must_check CamClassCreate(struct module *owner,
                                                  const char *name)
{
    return class_create(owner, name);
}

void CamClassDestroy(struct class *cls)
{
    class_destroy(cls);
}

int CamIoremapPageRange(unsigned long addr,
                       unsigned long end, phys_addr_t phys_addr, pgprot_t prot)
{
    return  ioremap_page_range(addr, end, phys_addr, prot);
}

void CamDevmKfree(struct device *dev, void *p)
{
    devm_kfree(dev, p);
}

void * CamDevmKmalloc(struct device *dev, size_t size, gfp_t gfp)
{
    return devm_kmalloc(dev, size, gfp);
}

int CamOfPropertyReadU32Array(const struct device_node *np,
                               const char *propname, u32 *out_values,
                               size_t sz)
{
    return of_property_read_u32_array(np, propname, out_values, sz);
}

int CamGpioRequest(unsigned gpio, const char *label)
{
        return gpio_request(gpio, label);
}

unsigned int CamIrqOfParseAndMap(struct device_node *dev, int index)
{
    return irq_of_parse_and_map(dev, index);
}

int CamofPropertyReadU32(const struct device_node *np,
                                       const char *propname,
                                       u32 *out_value)
{
        return of_property_read_u32(np, propname, out_value);
}

struct workqueue_struct* CamCreatesiglethreadWorkqueue(const char *fmt)
{
    return create_singlethread_workqueue(fmt);
}


EXPORT_SYMBOL(CamCreatesiglethreadWorkqueue);
EXPORT_SYMBOL(CamofPropertyReadU32);
EXPORT_SYMBOL(CamIrqOfParseAndMap);
EXPORT_SYMBOL(CamGpioRequest);
EXPORT_SYMBOL(CamDevmKmalloc);
EXPORT_SYMBOL(CamDevmKfree);
EXPORT_SYMBOL(CamIoremapPageRange);
EXPORT_SYMBOL(CamClassDestroy);
EXPORT_SYMBOL(CamClassCreate);
EXPORT_SYMBOL(CamKobjectCreateAndAdd);
EXPORT_SYMBOL(CamDeviceUnregister);
EXPORT_SYMBOL(CamDeviceRemoveFile);
EXPORT_SYMBOL(CamDeviceDestroy);
EXPORT_SYMBOL(CamDeviceCreateFile);
EXPORT_SYMBOL(CamDeviceCreate);
EXPORT_SYMBOL(CamSysfsRemoveFiles);
EXPORT_SYMBOL(CamSysfsRemoveFile);
EXPORT_SYMBOL(CamSysfsCreateLink);
EXPORT_SYMBOL(CamSysfsCreateFiles);
EXPORT_SYMBOL(CamSysfsCreateFile);
EXPORT_SYMBOL(CamPlatformDriverRegister);
EXPORT_SYMBOL(CamPlatformGetIrqByname);
EXPORT_SYMBOL(CamPlatformDriverUnregister);
EXPORT_SYMBOL(CamPlatformGetResource);
EXPORT_SYMBOL(CamOfPropertyReadVariableU32Array);
EXPORT_SYMBOL(CamOfPropertyReadU32Index);
EXPORT_SYMBOL(CamOfIrqToResource);
EXPORT_SYMBOL(CamOfAddressToResource);
EXPORT_SYMBOL(CamOfPropertyReadU32Array);