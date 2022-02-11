/*
* mdrv_wakeup.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: edie.chen <edie.chen@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/miscdevice.h> /* for MODULE_ALIAS_MISCDEV */
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/of_platform.h>
#include "../include/ms_types.h"
#include "../include/ms_platform.h"
#include "../include/mdrv_gpio_io.h"
#include "../include/mdrv_gpio.h"
#include "../include/ms_platform.h"
#include "../include/infinity2m/gpio.h"

static int _wakeNum = 0;
unsigned int _wakeSource[3]={PAD_UNKNOWN, PAD_UNKNOWN, PAD_UNKNOWN};

static const struct of_device_id wakeup_gpio_of_match[] = {
    { .compatible = "sstar,wakeup-gpio" },
    { },
};

static irqreturn_t sstar_gpio_wake_interrupt(int irq, void *dev_id)
{
	/* Need to do something with serial port right after wake-up? */
	return IRQ_HANDLED;
}

static int sstar_wakeup_probe(struct platform_device *pdev)
{
    const struct of_device_id *match;
    int ret;
    int i;


	match = of_match_device(wakeup_gpio_of_match, &pdev->dev);

    if (!match) {
        printk("Err:[gpio] No dev found\n");
        return -ENODEV;
    }

	of_property_read_u32(pdev->dev.of_node, "wakeup_num", &_wakeNum);

    if (of_property_read_u32_array(pdev->dev.of_node, "wakeup_source", (unsigned int*)_wakeSource, _wakeNum))
    {
        pr_err(">> [wakeup] Err: Could not get dts [wakeup_source] option!\n");
        return 1;
    }
    for(i = 0; i < _wakeNum; i++)
    {
        if(_wakeSource[i] != PAD_UNKNOWN){
            ret = gpio_request(_wakeSource[i], "wakeup_source");

            if (ret < 0) {
                printk(KERN_ERR " Could not request  wake GPIO: %i\n",
                _wakeSource[i]);
            }
            ret = request_irq(gpio_to_irq(_wakeSource[i]), &sstar_gpio_wake_interrupt, IRQF_TRIGGER_RISING|IRQF_SHARED, "wakeup_source", pdev);
            if (ret) {
                gpio_free(_wakeSource[i]);
                printk(KERN_ERR " No interrupt for wake GPIO: %i\n", _wakeSource[i]);
            }
            gpio_direction_input(_wakeSource[i]);
            device_init_wakeup(&pdev->dev, 1);
            enable_irq_wake(gpio_to_irq(_wakeSource[i]));
        }
    }
	return ret;
}

#ifdef CONFIG_PM

static int sstar_wakeup_suspend(struct platform_device *dev, pm_message_t state)
{
        int i = 0;
        for(i = 0; i < _wakeNum; i++)
        {
            printk("%s\r\n", __FUNCTION__);
            enable_irq_wake(gpio_to_irq(_wakeSource[i]));
        }
	return 0;
}

static int sstar_wakeup_resume(struct platform_device *dev)
{
        int i ;
        for(i = 0; i < _wakeNum; i++)
        {

            /*pm.c resume_irq.c: if (desc->istate & IRQS_SUSPENDED)*/
            disable_irq(gpio_to_irq(_wakeSource[i]));
            enable_irq(gpio_to_irq(_wakeSource[i]));
        }
	return 0;
}

#else
#define sstar_wdt_suspend NULL
#define sstar_wdt_resume  NULL
#endif /* CONFIG_PM */

//module_platform_driver(infinity_wdt_driver);

static struct platform_driver wakeupdriver = {
    .driver     = {
        .name   = "wakeup",
        .owner  = THIS_MODULE,
        .of_match_table = wakeup_gpio_of_match,
    },
    .probe      = sstar_wakeup_probe,
	#ifdef CONFIG_PM
	.suspend	= sstar_wakeup_suspend,
	.resume		= sstar_wakeup_resume,
#endif
};
module_platform_driver(wakeupdriver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("sstar wakeup Device Driver");
MODULE_LICENSE("GPL");



