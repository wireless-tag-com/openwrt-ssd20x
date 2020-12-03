/*
* usb_pad.c- Sigmastar
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
#include <linux/types.h>
#include <linux/init.h>
#include <linux/irqchip.h>
#include <linux/of_platform.h>
#include <linux/of_fdt.h>
#include <linux/sys_soc.h>
#include <linux/slab.h>
#include <linux/suspend.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/timecounter.h>
#include <clocksource/arm_arch_timer.h>

#include <linux/gpio.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/memory.h>
#include <asm/io.h>
#include <asm/mach/map.h>
#include "gpio.h"
#include "registers.h"
#include "mcm_id.h"
#include "ms_platform.h"
#include "ms_types.h"
#include "_ms_private.h"
#include "usb_pad.h"


int Enable_USB_VBUS(int param)
{
    int ret;
    //int package = mstar_get_package_type();
    int power_en_gpio=-1;

    struct device_node *np;
    int pin_data;
    int port_num = param >> 16;
    int vbus_enable = param & 0xFF;
    if((vbus_enable<0 || vbus_enable>1) && (port_num<0 || port_num>1))
    {
        printk(KERN_ERR "[%s] param invalid:%d %d\n", __FUNCTION__, port_num, vbus_enable);
        return -EINVAL;
    }

    if (power_en_gpio<0)
    {
        if(0 == port_num)
        {
            np = of_find_node_by_path("/soc/Sstar-ehci-1");
        }
        else if (1 == port_num)
        {
            np = of_find_node_by_path("/soc/Sstar-ehci-2");
        }
        else
        {
            np = of_find_node_by_path("/soc/Sstar-ehci-3");
        }

        if(!of_property_read_u32(np, "power-enable-pad", &pin_data))
        {
            printk(KERN_ERR "Get power-enable-pad from DTS GPIO(%d)\n", pin_data);
            power_en_gpio = (unsigned char)pin_data;

            ret = gpio_request(power_en_gpio, "USB0-power-enable");
            if (ret < 0) {
                printk(KERN_INFO "Failed to request USB0-power-enable GPIO(%d)\n", power_en_gpio);
                power_en_gpio =-1;
                return ret;
            }
        }
    }

    if (power_en_gpio >= 0)
    {
        if(0 == vbus_enable) //disable vbus
        {
            gpio_direction_output(power_en_gpio, 0);
            printk(KERN_INFO "[%s] Disable USB VBUS GPIO(%d)\n", __FUNCTION__,power_en_gpio);
        }
        else if(1 == vbus_enable)
        {
            gpio_direction_output(power_en_gpio, 1);
            printk(KERN_INFO "[%s] Enable USB VBUS GPIO(%d)\n", __FUNCTION__,power_en_gpio);
        }
    }
    return 0;
}
EXPORT_SYMBOL(Enable_USB_VBUS);

int Get_USB_VBUS_Pin(struct device_node *np)
{
	int power_en_gpio=-1;
	int pin_data;
        
	if(!of_property_read_u32(np, "power-enable-pad", &pin_data))
	{
		printk(KERN_ERR "Get power-enable-pad from DTS GPIO(%d)\n", pin_data);
		power_en_gpio = (unsigned char)pin_data;
	}
	/*else
	{
		printk(KERN_ERR "Can't get power-enable-pad from DTS, set default GPIO(%d)\n", pin_data);
		power_en_gpio = PAD_PM_GPIO2;
	}*/
	return power_en_gpio;
}
EXPORT_SYMBOL(Get_USB_VBUS_Pin);

