/*
* drv_isrcb_module.c- Sigmastar
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
#include <linux/init.h>
#include <drv_isrcb.h>

EXPORT_SYMBOL(ISRCB_RegisterCallback);
EXPORT_SYMBOL(ISRCB_UnRegisterCallback);
EXPORT_SYMBOL(ISRCB_Proc);

//int __init ISRCB_EarlyInit(void);
//early_initcall(ISRCB_EarlyInit);

int ISRCB_EarlyInit(void);
static int __init isrcb_init_driver(void)
{
    ISRCB_EarlyInit();
    return 0;
}

static void __exit isrcb_exit_driver(void)
{
}

subsys_initcall(isrcb_init_driver);
module_exit(isrcb_exit_driver);

MODULE_DESCRIPTION("Isp interrupt dispatcher driver");
MODULE_AUTHOR("SSTAR");
MODULE_LICENSE("GPL");
