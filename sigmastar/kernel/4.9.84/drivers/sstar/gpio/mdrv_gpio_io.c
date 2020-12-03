/*
* mdrv_gpio_io.c- Sigmastar
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
//#include "MsCommon.h"
//#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/semaphore.h>


#include <linux/err.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pm.h>

//#include "mst_devid.h"

#include "mdrv_gpio_io.h"
#include "mhal_gpio.h"
#include "mdrv_gpio.h"
#include "ms_platform.h"
#include "gpio.h"


//#include "mdrv_probe.h"

//-------------------------------------------------------------------------------------------------
//  Driver Compiler Options
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#define GPIO_DBG_ENABLE              0

#if GPIO_DBG_ENABLE
#define GPIO_DBG(_f)                 (_f)
#else
#define GPIO_DBG(_f)
#endif

#if 0
#define LINE_DBG()                  printf("GPIO %d\n", __LINE__)
#else
#define LINE_DBG()
#endif

#define GPIO_PRINT(fmt, args...)        //printk("\n[GPIO][%05d] " fmt, __LINE__, ## args)

typedef struct
{
    S32                          s32MajorGPIO;
    S32                          s32MinorGPIO;
    struct cdev                 cDevice;
    struct file_operations      GPIOFop;
    struct fasync_struct        *async_queue; /* asynchronous readers */
} GPIO_ModHandle_t;


#define MOD_GPIO_DEVICE_COUNT         1
#define MOD_GPIO_NAME                 "ModGPIO"

#define MDRV_NAME_GPIO                  "gpio"
#define MDRV_MAJOR_GPIO                 0x9b
#define MDRV_MINOR_GPIO                 0x00

//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------

static struct device *dev;

//static struct class *gpio_class;

int camdriver_gpio_request(struct gpio_chip *chip, unsigned offset)
{
    MDrv_GPIO_Pad_Set(offset);
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_request offset=%d\n",offset);
    return 0;
}

static void camdriver_gpio_free(struct gpio_chip *chip, unsigned offset)
{
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_free\n");
}

void camdriver_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
    if(value==0)
        MDrv_GPIO_Pull_Low(offset);
    else
        MDrv_GPIO_Pull_High(offset);
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_set\n");
}

int camdriver_gpio_get(struct gpio_chip *chip, unsigned offset)
{
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_get\n");
    return MDrv_GPIO_Pad_Read(offset);
}

int camdriver_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_direction_input\n");
    MDrv_GPIO_Pad_Odn(offset);
    return 0;
}

int camdriver_gpio_direction_output(struct gpio_chip *chip, unsigned offset,
                    int value)
{
    MDrv_GPIO_Pad_Oen(offset);
    if(value==0)
        MDrv_GPIO_Pull_Low(offset);
    else
        MDrv_GPIO_Pull_High(offset);
    GPIO_PRINT("[camdriver-gpio]camdriver_gpio_direction_output\n");
    return 0;
}

int camdriver_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
    int virq;

    virq = MDrv_GPIO_To_Irq(offset);
    if (virq < 0)
        return -ENXIO;

    GPIO_PRINT("%s virq:%d \n", __FUNCTION__, virq);

    return virq;
}

static struct gpio_chip camdriver_gpio_chip = {
    .label          = "gpio",
    .request        = camdriver_gpio_request,
    .free           = camdriver_gpio_free,
    .direction_input    = camdriver_gpio_direction_input,
    .get            = camdriver_gpio_get,
    .direction_output   = camdriver_gpio_direction_output,
    .set            = camdriver_gpio_set,
    .to_irq         = camdriver_gpio_to_irq,
    .base           = 0,
};


static const struct of_device_id camdriver_gpio_of_match[] = {
    { .compatible = "sstar,gpio" },
    { },
};

static int camdriver_gpio_probe(struct platform_device *pdev)
{
    const struct of_device_id *match;
    int ret;
/*
    struct resource *res;
    void __iomem *base;
    int gpionum;
    struct device_node  *node = pdev->dev.of_node;
*/
    dev = &pdev->dev;
    GPIO_PRINT("\n++[camdriver-gpio]camdriver_gpio_probe start\n");
/*
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    base = (void *)(IO_ADDRESS(res->start));
    gPadBaseAddr=(U32)base;
    res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    base = (void *)(IO_ADDRESS(res->start));
    gChipBaseAddr=(U32)base;
    res = platform_get_resource(pdev, IORESOURCE_MEM, 2);
    base = (void *)(IO_ADDRESS(res->start));
    gPmGpioBaseAddr=(U32)base;
    res = platform_get_resource(pdev, IORESOURCE_MEM, 3);
    base = (void *)(IO_ADDRESS(res->start));
    gPmSleepBaseAddr=(U32)base;
    GPIO_PRINT("gPadBaseAddr=%x\n",gPadBaseAddr);
    GPIO_PRINT("gChipBaseAddr=%x\n",gChipBaseAddr);
    GPIO_PRINT("gPmGpioBaseAddr=%x\n",gPmGpioBaseAddr);
*/
    match = of_match_device(camdriver_gpio_of_match, &pdev->dev);
    if (!match) {
        printk("Err:[gpio] No dev found\n");
        return -ENODEV;
    }
//    of_property_read_u32(node, "gpio-num", &gpionum);

    camdriver_gpio_chip.ngpio = GPIO_NR;
    camdriver_gpio_chip.of_node = pdev->dev.of_node;
    ret = gpiochip_add(&camdriver_gpio_chip);
    if (ret < 0) {
        printk("[gpio] add err\n");
        return ret;
    }

    GPIO_PRINT("--[camdriver-gpio]camdriver_gpio_probe end\n");

    MDrv_GPIO_Init();
    printk(KERN_WARNING"GPIO: probe end");
    return 0;
}

static struct platform_driver camdriver_gpio_driver = {
    .driver     = {
        .name   = "gpio",
        .owner  = THIS_MODULE,
        .of_match_table = camdriver_gpio_of_match,
    },
    .probe      = camdriver_gpio_probe,
};



void __mod_gpio_init(void)
{
    //GPIO chiptop initialization
    MDrv_GPIO_Init();
}


static int __init camdriver_gpio_init(void)
{
    return platform_driver_register(&camdriver_gpio_driver);
}
postcore_initcall(camdriver_gpio_init);

EXPORT_SYMBOL(camdriver_gpio_to_irq);
EXPORT_SYMBOL(camdriver_gpio_direction_output);
EXPORT_SYMBOL(camdriver_gpio_request);
EXPORT_SYMBOL(camdriver_gpio_set);
EXPORT_SYMBOL(camdriver_gpio_get);
EXPORT_SYMBOL(camdriver_gpio_direction_input);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("GPIO driver");
MODULE_LICENSE("GPL");
