/*
* drv_pollsample_module.c- Sigmastar
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
#include <linux/fs.h>
#include "mdrv_pollsample_module.h"

static struct file_operations pollsample_fops =
{
    .owner   =   THIS_MODULE,
    .open    = pollsamp_open,
    .release = pollsamp_release,
    .unlocked_ioctl = pollsamp_ioctl,
    .poll    = pollsamp_poll,
};

#define CHRDEV_NAME "pollsample"
static struct class *chrdev_class = NULL;
static struct device *chrdev_device = NULL;
static dev_t chrdev_devno;

PollSampleDev_t tPollSampleDev;

static __init int pollsample_init(void)
{
    int ret = 0, err = 0;

    printk(KERN_ALERT "pollsample_init!\n");

    // alloc character device number
    ret = alloc_chrdev_region(&chrdev_devno, 0, 1, CHRDEV_NAME);
    if(ret)
    {
        printk(KERN_ALERT " alloc_chrdev_region failed!\n");
        goto PROBE_ERR;
    }
    printk(KERN_ALERT " major:%d minor:%d\n", MAJOR(chrdev_devno), MINOR(chrdev_devno));

    tPollSampleDev.nTestNum = 12345;

    cdev_init(&tPollSampleDev.m_cdev, &pollsample_fops);
    tPollSampleDev.m_cdev.owner = THIS_MODULE;
    // add a character device
    err = cdev_add(&tPollSampleDev.m_cdev, chrdev_devno, 1);
    if(err)
    {
        printk(KERN_ALERT " cdev_add failed!\n");
        goto PROBE_ERR;
    }

    // create the device class
    chrdev_class = class_create(THIS_MODULE, CHRDEV_NAME);
    if(IS_ERR(chrdev_class))
    {
        printk(KERN_ALERT " class_create failed!\n");
        goto PROBE_ERR;
    }

    // create the device node in /dev
    chrdev_device = device_create(chrdev_class, NULL, chrdev_devno,
                                  NULL, CHRDEV_NAME);
    if(NULL == chrdev_device)
    {
        printk(KERN_ALERT " device_create failed!\n");
        goto PROBE_ERR;
    }

    printk(KERN_ALERT " pollsample_init ok!\n");
    return 0;

PROBE_ERR:
    if(err)
        cdev_del(&tPollSampleDev.m_cdev);
    if(ret)
        unregister_chrdev_region(chrdev_devno, 1);
    return -1;
}

static __exit void pollsample_exit(void)
{
    printk(KERN_ALERT " pollsample_exit!\n");

    cdev_del(&tPollSampleDev.m_cdev);
    unregister_chrdev_region(chrdev_devno, 1);

    device_destroy(chrdev_class, chrdev_devno);
    class_destroy(chrdev_class);
    //return 0;
}

module_init(pollsample_init);
module_exit(pollsample_exit);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("pollsample driver");
MODULE_LICENSE("GPL");
