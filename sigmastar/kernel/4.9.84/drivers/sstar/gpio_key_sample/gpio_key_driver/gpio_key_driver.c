/*
* gpio_key_driver.c- Sigmastar
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
#include <linux/platform_device.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/leds.h>

#include <asm/mach-types.h>
 
static struct gpio_keys_button sstar_buttons[] = {
	{
		.code	= 0x7,
		.gpio	= 66,
		.active_low = 1,
		.desc	= "key wakeup",
		.type	= EV_KEY,
		.wakeup = 1,
		//.debounce_interval = 10,
	},
};

static struct gpio_keys_platform_data sstar_gpio_keys_data = {
	.buttons = sstar_buttons,
	.nbuttons = ARRAY_SIZE(sstar_buttons),
};


static struct platform_device sstar_gpio_keys = {
	.name		= "gpio-keys",
	.id		= -1,
	.dev.platform_data = &sstar_gpio_keys_data,
};


static struct platform_device *devices[] __initdata = {
	&sstar_gpio_keys,
};

static int __init sstar_init(void)
{

	return platform_add_devices(devices, ARRAY_SIZE(devices));
}

module_init(sstar_init);

MODULE_LICENSE("GPL");

