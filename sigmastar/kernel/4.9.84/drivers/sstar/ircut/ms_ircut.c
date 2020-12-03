/*
* ms_ircut.c- Sigmastar
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
#include <linux/types.h>
#include <linux/timer.h>
#include <linux/miscdevice.h> /* for MODULE_ALIAS_MISCDEV */
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/uio_driver.h>
#include <linux/slab.h> /* kmalloc, kfree */
#include <linux/device.h> /* class_create */
#include <linux/kobject.h> /* kobject_uevent */
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/irq.h>
#include <linux/of_irq.h>
#include <linux/of_device.h>
#include "irqs.h"
#include "gpio.h"
#include "mdrv_types.h"

extern U8 MDrv_GPIO_Pad_Read(U8 u8IndexGPIO);
extern void MDrv_GPIO_Pad_Set(U8 u8IndexGPIO);
extern void MDrv_GPIO_Pad_Odn(U8 u8IndexGPIO);


//#define IRCUT_DEBUG  1
u32 mGpioNum=78;
u32 mIntNum=182;
#ifdef IRCUT_DEBUG
#define IRCUT_DBG(fmt, arg...) printk(KERN_INFO fmt, ##arg)
#else
#define IRCUT_DBG(fmt, arg...)
#endif
#define IRCUT_ERR(fmt, arg...) printk(KERN_ERR fmt, ##arg)


static const struct of_device_id ms_ircut_of_match_table[] = {
    { .compatible = "sstar,infinity-ircut" },
    {}
};

MODULE_DEVICE_TABLE(of, ms_ircut_of_match_table);
static irqreturn_t ircut_handler(int irq, struct uio_info *dev_info)
{
    struct irq_data *data;
    int level;

    level= MDrv_GPIO_Pad_Read(mGpioNum);
    IRCUT_DBG("ms_ircut_level=%d\n",level);
    IRCUT_DBG("ms_ircut_level irq=%d\n",irq);
    data = irq_get_irq_data(irq);
    //data->chip->irq_set_type(data, level?1:0);
    if(!data)
        return -ENODEV;
    data->chip->irq_set_type(data, level?IRQ_TYPE_EDGE_FALLING:IRQ_TYPE_EDGE_RISING);

    return IRQ_HANDLED;
}

static int ms_ircut_probe(struct platform_device *pdev)
{
    //struct resource *res;
    int ret = 0;
    struct uio_info *info;
	const struct of_device_id *match;
	struct device_node	*node = pdev->dev.of_node;
    struct irq_data *data;
    int level;

    IRCUT_DBG("ms_ircut_probe\n");

	match = of_match_device(ms_ircut_of_match_table, &pdev->dev);
	if (!match) {
		printk("Error:[infinity-ircut] No device match found\n");
		return -ENODEV;
	}

	if (of_property_read_u32(node, "ircut-gpio-num", &mGpioNum)) {
		pr_err("%s get failed\n", "ircut-gpio-num");
		return -ENODEV;
	}
    IRCUT_DBG("11111111 u8GpioNum=%d\n",mGpioNum);

	MDrv_GPIO_Pad_Set(mGpioNum);
	MDrv_GPIO_Pad_Odn(mGpioNum);
    info = kzalloc(sizeof(struct uio_info), GFP_KERNEL);
    if (!info)
        return -ENOMEM;


	/*res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res)
    {
		IRCUT_ERR("[%s]: failed to get IORESOURCE_IRQ\n", __func__);
		return -ENODEV;
	}
	info->irq=res->start;

    IRCUT_DBG("info->irq=%ld\n",info->irq);
*/
    info->mem[0].addr = (unsigned long)kmalloc(1024,GFP_KERNEL);

    if(info->mem[0].addr == 0)
    {
        IRCUT_ERR("Invalid memory resource\n");
        kfree(info);
        return -ENOMEM;
    }

    info->mem[0].memtype = UIO_MEM_LOGICAL;
    info->mem[0].size = 1024;
    info->version = "0.1";
    info->name="ircut";
	info->irq=irq_of_parse_and_map(pdev->dev.of_node, 0);
    info->irq_flags = IRQF_SHARED;
    info->handler = ircut_handler;
    mIntNum=(int)info->irq;
    ret = uio_register_device(&pdev->dev, info);
    if (ret)
    {
        IRCUT_ERR("uio_register failed %d\n",ret);
        //iounmap(info->mem[0].internal_addr);
       // printk("uio_register failed %d\n",ret);
        kfree(info);
        return -ENODEV;
    }
    platform_set_drvdata(pdev, info);

    level= MDrv_GPIO_Pad_Read(mGpioNum);
    IRCUT_DBG("ms_ircut_probe,level=%d,info->irq=%ld\n",level,info->irq);

    data = irq_get_irq_data(mIntNum);
    data->chip->irq_set_type(data, level?1:0);

    return 0;
}

static int ms_ircut_remove(struct platform_device *pdev)
{
	//uio_unregister_device(&ircut_info);
    IRCUT_DBG("ms_ircut_remove\n");
	return 0;
}
#ifdef CONFIG_PM
static int ms_ircut_suspend(struct platform_device *pdev, pm_message_t state)
{
	//uio_unregister_device(&ircut_info);
    IRCUT_DBG("ms_ircut_suspend\n");
	return 0;
}

static int ms_ircut_resume(struct platform_device *pdev)
{
	//uio_unregister_device(&ircut_info);
    IRCUT_DBG("ms_ircut_resume\n");
	MDrv_GPIO_Pad_Set(mGpioNum);
	MDrv_GPIO_Pad_Odn(mGpioNum);
	return 0;
}
#endif



static struct platform_driver ms_ircut_driver = {
	.probe		= ms_ircut_probe,
	.remove		= ms_ircut_remove,
#ifdef CONFIG_PM
	.suspend	= ms_ircut_suspend,
	.resume		= ms_ircut_resume,
#endif
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "infinity-ircut",
        .of_match_table = ms_ircut_of_match_table,
	},
};
module_platform_driver(ms_ircut_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("ms ircut Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ms-ircut");
