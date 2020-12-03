/*
* miu_monitor.c- Sigmastar
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

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/printk.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include<linux/interrupt.h>
#include <linux/string.h>

#include "ms_platform.h"
#include "registers.h"

//-------------------------------------------------------------------------------------------------
//  Function prototype with weak symbol
//-------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------

struct timer_list  monitor_timer;
extern const char* miu_client_id_to_name(U16 id);
static void MDev_timer_callback(unsigned long value)
{	
	u16 u16status;
	u32 u32hit_address;
	u16status=INREG16( (BASE_REG_MIU_PA+REG_ID_6F));
	if(u16status & BIT4)
	{
		u32hit_address = (INREG16( (BASE_REG_MIU_PA+REG_ID_6D)) + ((INREG16( (BASE_REG_MIU_PA+REG_ID_6E)))<<16) )<<5;
		printk("WARN! addr:0x%08x stat:0x%04x\n",u32hit_address, u16status);
		printk("WARN! Check %s\n", miu_client_id_to_name((u16status>>8)&0x7f) );
	}
	mod_timer(&monitor_timer, jiffies+3*HZ);
}

static int __init mstar_miu_drv_init_module(void)
{

    int ret = 0;
	init_timer( &monitor_timer );
    monitor_timer.function = MDev_timer_callback;
    monitor_timer.expires = jiffies+HZ;
	add_timer(&monitor_timer);
    return ret;
}

static void __exit mstar_miu_drv_exit_module(void)
{

	del_timer(&monitor_timer);
}

module_init(mstar_miu_drv_init_module);
module_exit(mstar_miu_drv_exit_module);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("MIU driver");
MODULE_LICENSE("GPL");
