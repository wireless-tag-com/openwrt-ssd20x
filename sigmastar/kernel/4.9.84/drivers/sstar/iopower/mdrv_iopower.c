/*
* mdrv_iopower.c- Sigmastar
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
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/io.h>
#include "mdrv_types.h"
#include <linux/ctype.h>
#include <linux/delay.h>
#include "iopow.h"
#include "ms_platform.h"
#include "ms_msys.h"

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------
#define IOPOW_DBG_ENABLE                  0

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#if IOPOW_DBG_ENABLE
    #define IOPOW_DBG_PRINT(x, args...)           { printk(x, ##args); }
#else
    #define IOPOW_DBG_PRINT(x, args...)           { }
#endif

struct ss_iopow_info {
       struct platform_device *pdev;
       void __iomem *reg_base;
};

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Implementation
//-------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Function    : ss_iopow_Set_IO_Voltage
//  Description : This function is used for set io voltage.
//------------------------------------------------------------------------------
static void ss_iopow_Set_IO_Voltage(struct device *dev, u32 offset, u32 shift, u32 val)
{
    struct ss_iopow_info *info = dev_get_drvdata(dev);
    u16 reg;

    reg = readw(info->reg_base + offset);
    reg &= ~(1<<shift);
    writew(reg | (val<<shift), info->reg_base + offset);
}

static int _mdrv_iopow_dts(struct device *dev)
{

    int ret = 0;
    int val;

    ret = of_property_read_u32(dev->of_node, "pm_sar_atop_vddp1", &val);
    if (ret < 0 && ret != -EINVAL)
    {
        IOPOW_DBG_PRINT("[%s][%d] of_property_read_u32 (pm_sar_atop_vddp1) fail\n", __FUNCTION__, __LINE__);
    }
    else
    {
        IOPOW_DBG_PRINT("[%s][%d] pm_sar_atop_vddp1= (%d)\n", __FUNCTION__, __LINE__, val);
        ss_iopow_Set_IO_Voltage(dev, PM_SAR_ATOP_VDDP1, PM_SAR_ATOP_VDDP1_BIT, val);
    }

    ret = of_property_read_u32(dev->of_node, "pm_sar_atop_pmspi", &val);
    if (ret < 0 && ret != -EINVAL)
    {
        IOPOW_DBG_PRINT("[%s][%d] of_property_read_u32 fail (pm_sar_atop_pmspi)\n", __FUNCTION__, __LINE__);
    }
    else
    {
        IOPOW_DBG_PRINT("[%s][%d] pm_sar_atop_pmspi (%d)\n", __FUNCTION__, __LINE__, val);
        ss_iopow_Set_IO_Voltage(dev, PM_SAR_ATOP_LDO18, PM_SAR_ATOP_LDO18_BIT, val);
    }

    return 0;
}

static int sstar_iopow_probe(struct platform_device *pdev)
{
    struct ss_iopow_info *info;
    struct resource *res;

    info = devm_kzalloc(&pdev->dev, sizeof(struct ss_iopow_info), GFP_KERNEL);
    if (!info)
        return -ENOMEM;

    IOPOW_DBG_PRINT("IOPOW initial\n");

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res)
    {
       IOPOW_DBG_PRINT("[%s]: failed to get IORESOURCE_MEM\n", __func__);
       return -ENODEV;
    }

    info->reg_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(info->reg_base))
            return PTR_ERR(info->reg_base);

    info->pdev = pdev;
    platform_set_drvdata(pdev, info);

    _mdrv_iopow_dts(&pdev->dev);

    return 0;
}

static const struct of_device_id sstar_iopow_of_match[] = {
    { .compatible = "sstar-iopower" },
    { },
};

static struct platform_driver sstar_iopow_driver = {
    .driver     = {
        .name   = "iopower",
        .owner  = THIS_MODULE,
        .of_match_table = sstar_iopow_of_match,
    },
    .probe      = sstar_iopow_probe,
};

static int __init sstar_iopow_init(void)
{
    return platform_driver_register(&sstar_iopow_driver);
}
postcore_initcall_sync(sstar_iopow_init);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("io power driver");
MODULE_LICENSE("GPL");
