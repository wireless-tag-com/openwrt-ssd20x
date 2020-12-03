/*
* mdrv_sar.c- Sigmastar
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
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/delay.h>

#include "../include/ms_types.h"
#include "../include/ms_platform.h"
#include "mdrv_sar.h"


//#define OPEN_SAR_DEBUG
static U32 _gMIO_MapBase = 0;

#ifdef OPEN_SAR_DEBUG //switch printk
#define sarDbg  printk
#else
#define sarDbg(...)
#endif

struct ms_sar
{
    struct device		*dev;
    void __iomem *reg_base;
};

static int ms_sar_open(struct inode *inode, struct file *file);
int ms_sar_get(int ch);
static long ms_sar_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
void ms_sar_hw_init(void);


static const struct file_operations sar_fops =
{
    .open     = ms_sar_open,
    .unlocked_ioctl = ms_sar_ioctl,

};
static struct miscdevice sar_miscdev = {MISC_DYNAMIC_MINOR, DEVICE_NAME, &sar_fops};


BOOL HAL_SAR_Write2Byte(U32 u32RegAddr, U16 u16Val)
{
    (*(volatile U32*)(_gMIO_MapBase+((u32RegAddr & 0xFFFFFF00ul) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2))) = u16Val;
    return TRUE;
}

U16 HAL_SAR_Read2Byte(U32 u32RegAddr)
{
    return (*(volatile U32*)(_gMIO_MapBase+((u32RegAddr & 0xFFFFFF00ul) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2)));
}

BOOL HAL_SAR_Write2ByteMask(U32 u32RegAddr, U16 u16Val, U16 u16Mask)
{
    u16Val = (HAL_SAR_Read2Byte(u32RegAddr) & ~u16Mask) | (u16Val & u16Mask);
    //sarDbg("sar IOMap base:%16llx u16Val:%4x\n", _gMIO_MapBase, u16Val);
    HAL_SAR_Write2Byte(u32RegAddr, u16Val);
    return TRUE;
}

static long ms_sar_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    S32 s32Err= 0;
    SAR_ADC_CONFIG_READ adcCfg;

    //printk("%s is invoked\n", __FUNCTION__);

    /*
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
     */
    if ((SARADC_IOC_MAGIC!= _IOC_TYPE(cmd)) || (_IOC_NR(cmd)> SARADC_IOC_MAXNR))
    {
        return -ENOTTY;
    }

    /*
     * the direction is a bitmask, and VERIFY_WRITE catches R/W
     * transfers. `Type' is user-oriented, while
     * access_ok is kernel-oriented, so the concept of "read" and
     * "write" is reversed
     */
    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        s32Err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        s32Err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }
    if (s32Err)
    {
        return -EFAULT;
    }



    switch(cmd)
    {
    case IOCTL_SAR_INIT:
        ms_sar_hw_init();
        break;

    case IOCTL_SAR_SET_CHANNEL_READ_VALUE:
        if(copy_from_user(&adcCfg, (SAR_ADC_CONFIG_READ __user *)arg, sizeof(SAR_ADC_CONFIG_READ)))
        {
            return EFAULT;
        }

        channel = adcCfg.channel_value & 3;

        adcCfg.adc_value = ms_sar_get(channel);
        sarDbg("channel = %d , adc =%d \n",channel, adcCfg.adc_value);

        if(copy_to_user((SAR_ADC_CONFIG_READ __user *)arg, &adcCfg, sizeof( SAR_ADC_CONFIG_READ)))
        {
            return EFAULT;
        }
        break;

    default:
        printk("ioctl: unknown command\n");
        return -ENOTTY;
    }
    return 0;

}

static int ms_sar_open(struct inode *inode, struct file *file)
{
    return 0;
}

void ms_sar_hw_init(void)
{
    HAL_SAR_Write2Byte(REG_SAR_CTRL0,0x0a20);
    //HAL_SAR_Write2Byte(REG_SAR_CKSAMP_PRD,0x0005);
    //HAL_SAR_Write2ByteMask(REG_SAR_CTRL0,0x4000,0x4000);
}
EXPORT_SYMBOL(ms_sar_hw_init);

int ms_sar_get(int ch)
{
    U16 value=0;
    U32 count=0;
    HAL_SAR_Write2ByteMask(REG_SAR_CTRL0,BIT14, 0x4000);
    while(HAL_SAR_Read2Byte(REG_SAR_CTRL0)&BIT14 && count<100000)
    {
        udelay(1);
        count++;
    }

    switch(ch)
    {
    case 0:
        HAL_SAR_Write2ByteMask(REG_SAR_AISEL_CTRL, BIT0, BIT0);
        value=HAL_SAR_Read2Byte(REG_SAR_CH1_DATA);
        break;
    case 1:
        HAL_SAR_Write2ByteMask(REG_SAR_AISEL_CTRL, BIT1, BIT1);
        value=HAL_SAR_Read2Byte(REG_SAR_CH2_DATA);
        break;
    case 2:
        HAL_SAR_Write2ByteMask(REG_SAR_AISEL_CTRL, BIT2, BIT2);
        value=HAL_SAR_Read2Byte(REG_SAR_CH3_DATA);
        break;
    case 3:
        HAL_SAR_Write2ByteMask(REG_SAR_AISEL_CTRL, BIT3, BIT3);
        value=HAL_SAR_Read2Byte(REG_SAR_CH4_DATA);
        break;
    default:
        printk(KERN_ERR "error channel,support SAR0,SAR1,SAR2,SAR3\n");
        break;
    }
    return  value;
}
EXPORT_SYMBOL(ms_sar_get);


static int infinity_sar_probe(struct platform_device *pdev)
{
    int retval;
    struct device *dev;
    struct ms_sar *sar;
    struct resource *res;
    struct clk *clk;
    struct clk_hw *hw_parent;

    sarDbg("[SAR] infinity_sar_probe \n");
    // printk("[SAR] infinity_sar_probe \n");
    dev = &pdev->dev;
    sar = devm_kzalloc(dev, sizeof(*sar), GFP_KERNEL);
    if (!sar)
        return -ENOMEM;

    sar->dev = &pdev->dev;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res)
    {
        sarDbg("[%s]: failed to get IORESOURCE_MEM\n", __func__);
        return -ENODEV;
    }
    sar->reg_base = devm_ioremap_resource(&pdev->dev, res);
    _gMIO_MapBase=(U32)sar->reg_base;

    //2. set clk
    clk = of_clk_get(pdev->dev.of_node, 0);
    if(IS_ERR(clk))
    {
        retval = PTR_ERR(clk);
        sarDbg("[%s]: of_clk_get failed\n", __func__);
    }
    else
    {
        /* select clock mux */
        hw_parent = clk_hw_get_parent_by_index(__clk_get_hw(clk), 0);  // select mux 0
        sarDbg( "[%s]parent_num:%d parent[0]:%s\n", __func__,
                clk_hw_get_num_parents(__clk_get_hw(clk)), clk_hw_get_name(hw_parent));
        clk_set_parent(clk, hw_parent->clk);

        clk_prepare_enable(clk);
        sarDbg("[SAR] clk_prepare_enable\n");
    }

    misc_register(&sar_miscdev);
    return 0;
}

static int infinity_sar_remove(struct platform_device *pdev)
{
    struct clk *clk;
    sarDbg("[SAR]infinity_sar_remove \n");
    misc_deregister(&sar_miscdev);

    clk = of_clk_get(pdev->dev.of_node, 0);
    if (IS_ERR(clk))
    {
        sarDbg( "[SAR] Fail to get clk!\n" );

    }
    else
    {
        clk_disable_unprepare(clk);
        clk_put(clk);
    }

    return 0;
}

#ifdef CONFIG_PM

static int infinity_sar_suspend(struct platform_device *dev, pm_message_t state)
{
    sarDbg("[SAR]infinity_sar_suspend \n");
    return 0;
}

static int infinity_sar_resume(struct platform_device *dev)
{
    sarDbg("[SAR]infinity_sar_resume \n");
    return 0;
}

#else
#define infinity_sar_suspend NULL
#define infinity_sar_resume  NULL
#endif /* CONFIG_PM */




static const struct of_device_id ms_sar_of_match_table[] =
{
    { .compatible = "sstar,infinity-sar" },
    {}
};
MODULE_DEVICE_TABLE(of, ms_sar_of_match_table);

static struct platform_driver infinity_sar_driver =
{
    .probe		= infinity_sar_probe,
    .remove		= infinity_sar_remove,
#ifdef CONFIG_PM
    .suspend	= infinity_sar_suspend,
    .resume		= infinity_sar_resume,
#endif
    .driver		= {
        .owner	= THIS_MODULE,
        .name	= "infinity-sar",
        .of_match_table = ms_sar_of_match_table,
    },
};

module_platform_driver(infinity_sar_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("infinity Sar Device Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:infinity-sar");
