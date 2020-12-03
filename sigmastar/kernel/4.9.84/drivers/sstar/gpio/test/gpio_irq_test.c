/*
* gpio_irq_test.c- Sigmastar
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


///////////////////////////////////////////////////////////////////////////////
/// @file      gpio_irq_test.c
/// @brief     GPIO IRQ Test Code for Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/types.h>
#include <linux/sched.h>
#include "cam_os_wrapper.h"
#include <linux/gpio.h>
#include <linux/interrupt.h>

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("SStar GPIO IRQ Test");
MODULE_LICENSE("GPL");

int pin = 0;
module_param(pin, int, 0);
int level = 0;
module_param(level, int, 0);

s32 gpio_set_success = 0;
u32 gpio_irq_num = 0;

irqreturn_t gpio_test_isr(int irq, void *dev_instance)
{
    CamOsTimespec_t ptRes;
    CamOsGetMonotonicTime(&ptRes);
    printk("%s  [%d.%09d]\n", __func__, ptRes.nSec, ptRes.nNanoSec);

    return IRQ_NONE;
}

static int __init GpioIrqTestInit(void)
{
    gpio_set_success = 0;

    if(gpio_request(pin, "gpio_irq_test") < 0)
    {
        printk("request gpio[%d] failed...\n", pin);
        return 0;
    }

    if (gpio_direction_input(pin) < 0) {
        printk("gpio_direction_input[%d] failed...\n", pin);
        return 0;
    }

    gpio_irq_num = gpio_to_irq(pin);
    if (request_irq(gpio_irq_num, gpio_test_isr, (level==0)? IRQF_TRIGGER_FALLING : IRQF_TRIGGER_RISING, "PAD", NULL))
    {
        printk(KERN_ERR"can't allocate irq\n");
        return 0;
    }

    gpio_set_success = 1;

    return 0;
}

static void __exit GpioIrqTestExit(void)
{
    if (gpio_set_success)
        free_irq(gpio_irq_num, NULL);
    gpio_free(pin);
}

module_init(GpioIrqTestInit);
module_exit(GpioIrqTestExit);
