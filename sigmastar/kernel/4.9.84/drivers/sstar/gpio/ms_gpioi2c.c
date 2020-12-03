/*
* ms_gpioi2c.c- Sigmastar
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
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/ctype.h>

#include "mdrv_sw_iic.h"
#include "ms_msys.h"

#define GPIO_I2C_READ   0x01
#define GPIO_I2C_WRITE  0x03

struct cdev ms_gpioi2c_cdev;
int ms_gpioi2c_major;
int ms_gpioi2c_minor_start=0;
int ms_gpioi2c_dev_count=3;

static ssize_t access_retry_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        size_t len;
        const char *str = buf;
        while (*str && !isspace(*str)) str++;
        len = str - buf;
        if(len)
        {
            access_dummy_time = simple_strtoul(buf, NULL, 10);
            printk("\naccess_dummy_time=%d\n", access_dummy_time);
            return n;
        }
        return -EINVAL;
    }
    return -EINVAL;
}
static ssize_t access_retry_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%d\n", access_dummy_time);
    return (str - buf);
}
DEVICE_ATTR(access_retry, 0644, access_retry_show, access_retry_store);

static ssize_t gpioi2c_delay_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        size_t len;
        const char *str = buf;
        while (*str && !isspace(*str)) str++;
        len = str - buf;
        if(len)
        {
            gpioi2c_delay_us = simple_strtoul(buf, NULL, 10);
            printk("\ngpioi2c_delay_us=%d\n", gpioi2c_delay_us);
            return n;
        }
        return -EINVAL;
    }
    return -EINVAL;
}
static ssize_t gpioi2c_delay_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%d\n", gpioi2c_delay_us);
    return (str - buf);
}
DEVICE_ATTR(gpioi2c_delay, 0644, gpioi2c_delay_show, gpioi2c_delay_store);


unsigned char gpio_i2c_read(unsigned char devaddress, unsigned char address)
{
    int rxdata;

    MDrv_SW_IIC_Start(0);
    MDrv_SW_IIC_SendByte(0, devaddress, 0);
    MDrv_SW_IIC_SendByte(0, address, 0);
    MDrv_SW_IIC_Start(0);
    MDrv_SW_IIC_SendByte(0, (devaddress | 1), 0);
    rxdata = MDrv_SW_IIC_GetByte(0, FALSE);
    MDrv_SW_IIC_Stop(0);
    return rxdata;
}
EXPORT_SYMBOL(gpio_i2c_read);

void gpio_i2c_write(unsigned char devaddress, unsigned char address, unsigned char data)
{
    MDrv_SW_IIC_Start(0);
    MDrv_SW_IIC_SendByte(0, devaddress, 0);
    MDrv_SW_IIC_SendByte(0, address, 0);
    MDrv_SW_IIC_SendByte(0, data, 0);
    MDrv_SW_IIC_Stop(0);
}
EXPORT_SYMBOL(gpio_i2c_write);


long gpioi2c_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    unsigned int val;
	char device_addr, reg_addr;
	short reg_val;

	switch(cmd)
	{
		case GPIO_I2C_READ:
            //printk("GPIO_I2C_READ arg=0x%08X\n", *(unsigned int *)arg);
			val = *(unsigned int *)arg;
			device_addr = (val&0xff000000)>>24;
			reg_addr = (val&0xff0000)>>16;

			reg_val = gpio_i2c_read(device_addr, reg_addr);
			*(unsigned int *)arg = (val&0xffff0000)|reg_val;

			break;

		case GPIO_I2C_WRITE:
            //printk("GPIO_I2C_WRITE arg=0x%08X\n", *(unsigned int *)arg);
			val = *(unsigned int *)arg;
			device_addr = (val&0xff000000)>>24;
			reg_addr = (val&0xff0000)>>16;

			reg_val = val&0xffff;
			gpio_i2c_write(device_addr, reg_addr, reg_val);

			break;

		default:
			return -1;
	}
    return 0;
}


int gpioi2c_open(struct inode * inode, struct file * file)
{
    return 0;
}
int gpioi2c_close(struct inode * inode, struct file * file)
{
    return 0;
}

static struct file_operations gpioi2c_fops = {
    .owner      = THIS_MODULE,
    .unlocked_ioctl = gpioi2c_ioctl,
    .open       = gpioi2c_open,
    .release    = gpioi2c_close,
};

static int ms_gpioi2c_probe(struct platform_device *pdev)
{
    int err, sda, scl;
    dev_t dev;
    I2C_BusCfg_t gpioi2c_cfg;
    struct device* gpioi2c_dev;

    MDrv_IIC_Init();

    if(0 != of_property_read_u32(pdev->dev.of_node, "sda-gpio", &sda))
        return -EINVAL;

    if(0 != of_property_read_u32(pdev->dev.of_node, "scl-gpio", &scl))
        return -EINVAL;

    gpioi2c_cfg.u8ChIdx = 0;
    gpioi2c_cfg.u8PadSDA = sda;
    gpioi2c_cfg.u8PadSCL = scl;
    gpioi2c_cfg.u16SpeedKHz = 100; //gpioi2c no used
    gpioi2c_cfg.u8Enable = ENABLE;

    MDrv_SW_IIC_ConfigBus(&gpioi2c_cfg);

    if (0 != (err = alloc_chrdev_region(&dev, ms_gpioi2c_minor_start, ms_gpioi2c_dev_count, "mstar_gpioi2c")))
        return err;

    ms_gpioi2c_major = MAJOR(dev);

    cdev_init(&ms_gpioi2c_cdev, &gpioi2c_fops);
    ms_gpioi2c_cdev.owner=THIS_MODULE;

    if(0 != (err = cdev_add(&ms_gpioi2c_cdev, dev, ms_gpioi2c_dev_count)))
        return err;

    gpioi2c_dev = device_create(msys_get_sysfs_class(), NULL, dev, NULL, "gpioi2c");

    device_create_file(gpioi2c_dev, &dev_attr_gpioi2c_delay);
    device_create_file(gpioi2c_dev, &dev_attr_access_retry);

    printk("[gpioi2c] sda-gpio=%d, scl-gpio=%d\n", sda, scl);

    return err;
}

static int ms_gpioi2c_remove(struct platform_device *pdev)
{
    printk("[gpioi2c] removed\n");

    cdev_del(&ms_gpioi2c_cdev);
    unregister_chrdev_region(MKDEV(ms_gpioi2c_major, ms_gpioi2c_minor_start), ms_gpioi2c_dev_count);
    device_destroy(msys_get_sysfs_class(), MKDEV(ms_gpioi2c_major, ms_gpioi2c_dev_count));

    return 0;
}

static const struct of_device_id ms_gpioi2c_of_match_table[] = {
    { .compatible = "sstar,infinity-gpioi2c" },
    {}
};
MODULE_DEVICE_TABLE(of, ms_gpioi2c_of_match_table);

static struct platform_driver ms_gpioi2c_driver = {
    .remove = ms_gpioi2c_remove,
    .probe = ms_gpioi2c_probe,
    .driver = {
        .name = "ms_gpioi2c_driver",
        .owner = THIS_MODULE,
        .of_match_table = ms_gpioi2c_of_match_table,
    },
};

module_platform_driver(ms_gpioi2c_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("ms_sw_iic driver");
MODULE_LICENSE("GPL");
