/*
* ms_rtc.c- Sigmastar
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

#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/rtc.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/ctype.h>

#include "ms_platform.h"
#include "ms_types.h"
#include "ms_msys.h"

#define RTC_DEBUG  0

#if RTC_DEBUG
#define RTC_DBG(fmt, arg...) printk(KERN_INFO fmt, ##arg)
#else
#define RTC_DBG(fmt, arg...)
#endif
#define RTC_ERR(fmt, arg...) printk(KERN_ERR fmt, ##arg)

#define REG_RTC_CTRL        0x00
    #define SOFT_RSTZ_BIT   BIT0
    #define CNT_EN_BIT      BIT1
    #define WRAP_EN_BIT     BIT2
    #define LOAD_EN_BIT     BIT3
    #define READ_EN_BIT     BIT4
    #define INT_MASK_BIT    BIT5
    #define INT_FORCE_BIT   BIT6
    #define INT_CLEAR_BIT   BIT7

#define REG_RTC_FREQ_CW_L   0x04
#define REG_RTC_FREQ_CW_H   0x08

#define REG_RTC_LOAD_VAL_L  0x0C
#define REG_RTC_LOAD_VAL_H  0x10

#define REG_RTC_MATCH_VAL_L 0x14
#define REG_RTC_MATCH_VAL_H 0x18

#define REG_RTC_CNT_VAL_L   0x20
#define REG_RTC_CNT_VAL_H   0x24

struct ms_rtc_info {
    struct platform_device *pdev;
    struct rtc_device *rtc_dev;
    struct device* rtc_devnode;
    void __iomem *rtc_base;
};

int auto_wakeup_delay_seconds = 0;

static ssize_t auto_wakeup_timer_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        size_t len;
        const char *str = buf;
        while (*str && !isspace(*str)) str++;
        len = str - buf;
        if(len)
        {
            auto_wakeup_delay_seconds = simple_strtoul(buf, NULL, 10);
            //printk("\nauto_wakeup_delay_seconds=%d\n", auto_wakeup_delay_seconds);
            return n;
        }
        return -EINVAL;
    }
    return -EINVAL;
}
static ssize_t auto_wakeup_timer_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;

    str += scnprintf(str, end - str, "%d\n", auto_wakeup_delay_seconds);
    return (str - buf);
}
DEVICE_ATTR(auto_wakeup_timer, 0644, auto_wakeup_timer_show, auto_wakeup_timer_store);


static int ms_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    unsigned long seconds;

    seconds = readw(info->rtc_base + REG_RTC_MATCH_VAL_L) | (readw(info->rtc_base + REG_RTC_MATCH_VAL_H) << 16);

    rtc_time_to_tm(seconds, &alarm->time);

    if( !(readw(info->rtc_base + REG_RTC_CTRL) & INT_MASK_BIT) )
        alarm->enabled = 1;

    RTC_DBG("ms_rtc_read_alarm[%d,%d,%d,%d,%d,%d], alarm_en=%d\n",
        alarm->time.tm_year,alarm->time.tm_mon,alarm->time.tm_mday,alarm->time.tm_hour,alarm->time.tm_min,alarm->time.tm_sec, alarm->enabled);


    return 0;
}

static int ms_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    unsigned long seconds;
    u16 reg;

    RTC_DBG("ms_rtc_set_alarm[%d,%d,%d,%d,%d,%d], alarm_en=%d\n",
        alarm->time.tm_year,alarm->time.tm_mon,alarm->time.tm_mday,alarm->time.tm_hour,alarm->time.tm_min,alarm->time.tm_sec, alarm->enabled);

    rtc_tm_to_time(&alarm->time, &seconds);

    writew((seconds & 0xFFFF), info->rtc_base + REG_RTC_MATCH_VAL_L);
    writew((seconds>>16) & 0xFFFF, info->rtc_base + REG_RTC_MATCH_VAL_H);

    reg = readw(info->rtc_base + REG_RTC_CTRL);
    if(alarm->enabled)
    {
        writew(reg & ~(INT_MASK_BIT), info->rtc_base + REG_RTC_CTRL);
    }
    else
    {
        writew(reg | INT_MASK_BIT, info->rtc_base + REG_RTC_CTRL);
    }

    return 0;
}

static int ms_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    unsigned long seconds;
    u16 reg;

    reg = readw(info->rtc_base + REG_RTC_CTRL);
    writew(reg | READ_EN_BIT, info->rtc_base + REG_RTC_CTRL);
    while(readw(info->rtc_base + REG_RTC_CTRL) & READ_EN_BIT);  //wait for HW latch done

    seconds = readw(info->rtc_base + REG_RTC_CNT_VAL_L) | (readw(info->rtc_base + REG_RTC_CNT_VAL_H) << 16);

    rtc_time_to_tm(seconds, tm);

    RTC_DBG("ms_rtc_read_time[%d,%d,%d,%d,%d,%d]\n",
        tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);

    return rtc_valid_tm(tm);
}

static int ms_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev);
    unsigned long seconds;
    u16 reg;

    RTC_DBG("ms_rtc_set_time[%d,%d,%d,%d,%d,%d]\n",
        tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);

    rtc_tm_to_time(tm, &seconds);
    writew(seconds & 0xFFFF, info->rtc_base + REG_RTC_LOAD_VAL_L);
    writew((seconds >> 16) & 0xFFFF, info->rtc_base + REG_RTC_LOAD_VAL_H);
    reg = readw(info->rtc_base + REG_RTC_CTRL);
    writew(reg | LOAD_EN_BIT, info->rtc_base + REG_RTC_CTRL);
    /* need to check carefully if we want to clear REG_RTC_LOAD_VAL_H for customer*/
    while(readw(info->rtc_base + REG_RTC_CTRL) & LOAD_EN_BIT);
    writew(0, info->rtc_base + REG_RTC_LOAD_VAL_H);

    return 0;
}

static const struct rtc_class_ops ms_rtc_ops = {
    .read_time = ms_rtc_read_time,
    .set_time = ms_rtc_set_time,
    .read_alarm = ms_rtc_read_alarm,
    .set_alarm = ms_rtc_set_alarm,
};

static irqreturn_t ms_rtc_interrupt(s32 irq, void *dev_id)
{
    struct ms_rtc_info *info = dev_get_drvdata(dev_id);
    u16 reg;

    reg = readw(info->rtc_base + REG_RTC_CTRL);
    reg |= INT_CLEAR_BIT;
    writew(reg, info->rtc_base + REG_RTC_CTRL);
    RTC_DBG("RTC INTERRUPT\n");

    return IRQ_HANDLED;
}


#ifdef CONFIG_PM
static s32 ms_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
    if(auto_wakeup_delay_seconds)
    {
        struct rtc_time tm;
        struct rtc_wkalrm alarm;
        unsigned long seconds;
        ms_rtc_read_time(&pdev->dev, &tm);
        rtc_tm_to_time(&tm, &seconds);
        RTC_DBG("[%s]: Ready to use RTC alarm, time=%ld\n", __func__, seconds);
        seconds += auto_wakeup_delay_seconds;
        rtc_time_to_tm(seconds, &alarm.time);
        alarm.enabled=1;
        ms_rtc_set_alarm(&pdev->dev, &alarm);
    }
    return 0;
}

static s32 ms_rtc_resume(struct platform_device *pdev)
{
    return 0;
}
#endif

static int ms_rtc_remove(struct platform_device *pdev)
{
    struct clk *clk = of_clk_get(pdev->dev.of_node, 0);
    int ret = 0;

    if(IS_ERR(clk))
    {
        ret = PTR_ERR(clk);
        RTC_ERR("[%s]: of_clk_get failed\n", __func__);
        return ret;
    }
    clk_disable_unprepare(clk);
    clk_put(clk);

    return 0;
}

static int ms_rtc_probe(struct platform_device *pdev)
{
    struct ms_rtc_info *info;
    struct resource *res;
    struct clk *clk;
    dev_t dev;
    int ret = 0;
    u16 reg;
    u32 rate;
    int rc;

    info = devm_kzalloc(&pdev->dev, sizeof(struct ms_rtc_info), GFP_KERNEL);
    if (!info)
        return -ENOMEM;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
    {
		RTC_ERR("[%s]: failed to get IORESOURCE_MEM\n", __func__);
		return -ENODEV;
	}
    info->rtc_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(info->rtc_base))
        return PTR_ERR(info->rtc_base);
    info->pdev = pdev;

    res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res)
    {
		RTC_ERR("[%s]: failed to get IORESOURCE_IRQ\n", __func__);
		return -ENODEV;
	}
    rc = request_irq(res->start, ms_rtc_interrupt, IRQF_SHARED, "ms_rtc", &pdev->dev);
    if (rc)
    {
        RTC_ERR("[%s]: request_irq()is failed. return code=%d\n", __func__, rc);
    }
    platform_set_drvdata(pdev, info);

    info->rtc_dev = devm_rtc_device_register(&pdev->dev,
                dev_name(&pdev->dev), &ms_rtc_ops,
                THIS_MODULE);

    if (IS_ERR(info->rtc_dev)) {
        ret = PTR_ERR(info->rtc_dev);
        RTC_ERR("[%s]: unable to register device (err=%d).\n", __func__, ret);
        return ret;
    }

    //Note: is it needed?
    //device_set_wakeup_capable(&pdev->dev, 1);
    //device_wakeup_enable(&pdev->dev);

    //init rtc
    RTC_DBG("[%s]: hardware initialize\n", __func__);
    //1. release reset
    reg = readw(info->rtc_base + REG_RTC_CTRL);
    if( !(reg & SOFT_RSTZ_BIT) )
    {
        reg |= SOFT_RSTZ_BIT;
        writew(reg, info->rtc_base + REG_RTC_CTRL);
    }
    //2. set frequency
    clk = of_clk_get(pdev->dev.of_node, 0);
    if(IS_ERR(clk))
    {
        ret = PTR_ERR(clk);
        RTC_ERR("[%s]: of_clk_get failed\n", __func__);
        return ret;
    }

    /* Try to determine the frequency from the device tree */
    if (of_property_read_u32(pdev->dev.of_node, "clock-frequency", &rate))
    {
    	rate = clk_get_rate(clk);
    }
    else
    {
        clk_set_rate(clk, rate);
    }

    clk_prepare_enable(clk);
    RTC_ERR("[%s]: rtc setup, frequency=%lu\n", __func__, clk_get_rate(clk));
    writew(rate & 0xFFFF, info->rtc_base + REG_RTC_FREQ_CW_L);
    writew((rate >>16)  & 0xFFFF, info->rtc_base + REG_RTC_FREQ_CW_H);

    //3. enable counter
    reg |= CNT_EN_BIT;
    writew(reg, info->rtc_base + REG_RTC_CTRL);

    if (0 != (ret = alloc_chrdev_region(&dev, 0, 1, "ms_rtc")))
        return ret;

    info->rtc_devnode = device_create(msys_get_sysfs_class(), NULL, dev, NULL, "ms_rtc");
    device_create_file(info->rtc_devnode, &dev_attr_auto_wakeup_timer);

    clk_put(clk);

    return ret;
}

static const struct of_device_id ms_rtc_of_match_table[] = {
    { .compatible = "sstar,infinity-rtc" },
    {}
};
MODULE_DEVICE_TABLE(of, ms_rtc_of_match_table);

static struct platform_driver ms_rtc_driver = {
    .remove = ms_rtc_remove,
    .probe = ms_rtc_probe,
#ifdef CONFIG_PM
    .suspend = ms_rtc_suspend,
    .resume = ms_rtc_resume,
#endif
    .driver = {
        .name = "ms_rtc",
        .owner = THIS_MODULE,
        .of_match_table = ms_rtc_of_match_table,
    },
};

module_platform_driver(ms_rtc_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("MStar RTC Driver");
MODULE_LICENSE("GPL v2");
