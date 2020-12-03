/*
* mdrv_wdt.c- Sigmastar
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
#include <linux/clk.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/of_platform.h>
#include "../include/ms_types.h"
#include "../include/ms_platform.h"
#include "mdrv_wdt.h"


//#define OPEN_WDT_DEBUG

#ifdef OPEN_WDT_DEBUG //switch printk
#define wdtDbg  printk
#else
#define wdtDbg(...)
#endif
#define wdtErr  printk
#define OPTIONS (WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING)
#define CONFIG_WATCHDOG_ATBOOT		(0)
#define CONFIG_WATCHDOG_DEFAULT_TIME	(10)   //unit=SEC
#define REPEAT_DELAY 500                                //unit=10ms

int tmr_margin	= CONFIG_WATCHDOG_DEFAULT_TIME;
//static bool nowayout	= WATCHDOG_NOWAYOUT;

struct ms_wdt {
	struct device		*dev;
	struct watchdog_device	wdt_device;
    void __iomem *reg_base;
	spinlock_t		lock;	
};
static int infinity_wdt_start(struct watchdog_device *wdd);
static int infinity_wdt_stop(struct watchdog_device *wdd);
static int infinity_wdt_set_heartbeat(struct watchdog_device *wdd, unsigned timeout);
static int infinity_wdt_set_timeout(struct watchdog_device *wdd, unsigned int timeout);
static int infinity_wdt_ping(struct watchdog_device *wdd);

static DEFINE_SPINLOCK(wdt_lock);

U32 g_u32Clock;
struct timer_list wdt_timer;
U8 watchdog_open_flag=0;
static unsigned long next_heartbeat;

static const struct watchdog_info infinity_wdt_ident = {
	.options          =     OPTIONS,
	.firmware_version =	0,
	.identity         =	"infinity Watchdog",
};


static struct watchdog_ops infinity_wdt_ops = {
	.owner = THIS_MODULE,
	.start = infinity_wdt_start,
	.stop = infinity_wdt_stop,
	.set_timeout= infinity_wdt_set_timeout,
	.ping = infinity_wdt_ping,
};

static struct watchdog_device infinity_wdd = {
	.info = &infinity_wdt_ident,
	.ops = &infinity_wdt_ops,

};

U32 __infinity_wdt_get_clk_rate(void)
{
    struct device_node *dev_node;
    struct platform_device *pdev;
    char compatible_name[64];
    struct clk         *clk;
    U32 rate;

    sprintf(compatible_name, "sstar,infinity-wdt"); //"sstar,cmdq0"
    dev_node = of_find_compatible_node(NULL, NULL, compatible_name);

    if (!dev_node) {
        if (!dev_node)
            return 0;
    }
    pdev = of_find_device_by_node(dev_node);
    if (!pdev) {
        of_node_put(dev_node);
        return 0;
    }

    clk = of_clk_get(pdev->dev.of_node, 0);
    if(IS_ERR(clk))
    {
        return 0;
    }

    rate = (U32)clk_get_rate(clk);
    clk_put(clk);
    return rate;
}

void check_osc_clk(void)
{
    g_u32Clock = __infinity_wdt_get_clk_rate();
    if (g_u32Clock == 0)
    {
        g_u32Clock = OSC_CLK_12000000;
    }
}


static void __infinity_wdt_stop(void)
{
    wdtDbg("__infinity_wdt_stop \n");
    OUTREG16(BASE_REG_WDT_PA + WDT_WDT_CLR, 0);
    OUTREG16(BASE_REG_WDT_PA + WDT_MAX_PRD_H, 0x0000);
    OUTREG16(BASE_REG_WDT_PA + WDT_MAX_PRD_L, 0x0000);
}

static int infinity_wdt_stop(struct watchdog_device *wdd)
{
    wdtDbg("[WatchDog]infinity_wdt_stop \n");

	spin_lock(&wdt_lock);
    del_timer(&wdt_timer);
	__infinity_wdt_stop();
	watchdog_open_flag=0;
	spin_unlock(&wdt_lock);

	return 0;
}

static void __infinity_wdt_start(U32 u32LaunchTim)
{
    wdtDbg("__infinity_wdt_start \n");
    OUTREG16(BASE_REG_WDT_PA + WDT_WDT_CLR, CLEAR_WDT);
    OUTREG16(BASE_REG_WDT_PA + WDT_MAX_PRD_H, (((g_u32Clock*u32LaunchTim)>>16) & 0xFFFF));
    OUTREG16(BASE_REG_WDT_PA + WDT_MAX_PRD_L, ((g_u32Clock*u32LaunchTim) & 0xFFFF));	
}
/*
static void infinity_wdt_timer(unsigned long data)
{
    unsigned long j;

    wdtDbg("[WatchDog]infinity_wdt_timer data=%lx \r\n",data);

    infinity_wdt_set_heartbeat(&infinity_wdd, data);
    j = jiffies;
    wdt_timer.expires = j + REPEAT_DELAY;
	next_heartbeat = wdt_timer.expires;
    add_timer(&wdt_timer);
}
*/
static int infinity_wdt_start(struct watchdog_device *wdd)
{
    unsigned long j;
	
    wdtDbg("[WatchDog]infinity_wdt_start \n");
	spin_lock(&wdt_lock);
	
	__infinity_wdt_stop();
	
    __infinity_wdt_start(tmr_margin);
	
    init_timer(&wdt_timer);
    
    //wdt_timer.data = (unsigned long)tmr_margin;
    //wdt_timer.function = infinity_wdt_timer;
	
    j = jiffies;
    wdt_timer.expires = j + REPEAT_DELAY*tmr_margin;
	next_heartbeat = wdt_timer.expires;
    //add_timer(&wdt_timer);
	watchdog_open_flag=1;
	spin_unlock(&wdt_lock);
	return 0;
}

static int infinity_wdt_set_timeout(struct watchdog_device *wdd, unsigned int timeout)
{
    unsigned long j;
    wdtDbg("[WatchDog]infinity_wdt_set_timeout=%d \n",timeout);
	if (timeout < 5)
		timeout=5;
		//return -EINVAL;
    j = jiffies;
    next_heartbeat = j + REPEAT_DELAY*timeout;
		
	 spin_lock(&wdt_lock);
	wdd->timeout = timeout;
	tmr_margin=timeout;
	wdt_timer.data = (unsigned long)tmr_margin;
    OUTREG16(BASE_REG_WDT_PA + WDT_MAX_PRD_H, (((g_u32Clock*tmr_margin)>>16) & 0xFFFF));
    OUTREG16(BASE_REG_WDT_PA + WDT_MAX_PRD_L, ((g_u32Clock*tmr_margin) & 0xFFFF));
	
	spin_unlock(&wdt_lock);
    wdtDbg("[WatchDog]infinity_wdt_set_timeout data=%lx \r\n",wdt_timer.data);
    return 0;
}

static int infinity_wdt_ping(struct watchdog_device *wdd)
{
	/* If we got a heartbeat pulse within the WDT_US_INTERVAL
	 * we agree to ping the WDT
	 */
    unsigned long j;
	 
	if (time_before(jiffies, next_heartbeat))
	{
       // wdtDbg("[WatchDog] infinity_wdt_ping tmr_margin=%lx \r\n",(jiffies-next_heartbeat));
		
		/* Ping the WDT */
		spin_lock(&wdt_lock);
		
        //OUTREG16(BASE_REG_WDT_PA + WDT_MAX_PRD_H, (((g_u32Clock*tmr_margin)>>16) & 0xFFFF));
        //OUTREG16(BASE_REG_WDT_PA + WDT_MAX_PRD_L, ((g_u32Clock*tmr_margin) & 0xFFFF));
        OUTREG16(BASE_REG_WDT_PA + WDT_WDT_CLR, CLEAR_WDT);
        j = jiffies;
       next_heartbeat = j + REPEAT_DELAY*tmr_margin;


		spin_unlock(&wdt_lock);
        wdtDbg("[WatchDog] infinity_wdt_ping tmr_margin=%x \r\n",tmr_margin);

		/* Re-set the timer interval */
		//mod_timer(&wdt_timer, jiffies + REPEAT_DELAY);
	} else
		pr_warn("Heartbeat lost! Will not ping the watchdog\n");
	return 0;
	
}

static int infinity_wdt_set_heartbeat(struct watchdog_device *wdd, unsigned timeout)
{
	//struct ms_wdt *wdt = watchdog_get_drvdata(wdd);
	if (timeout < 1)
		return -EINVAL;
    //if(timeout>40)///for test
	//	return -EINVAL;
	wdtDbg("[WatchDog]infinity_wdt_set_heartbeat \n");
	
    OUTREG16(BASE_REG_WDT_PA + WDT_WDT_CLR, CLEAR_WDT);

	wdd->timeout = timeout;
    
	return 0;
}

static int infinity_wdt_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct device *dev;
    struct ms_wdt *wdt;	
    int started = 0;
    struct resource *res;
	
    wdtDbg("[WatchDog]infinity_wdt_probe \n");
	dev = &pdev->dev;
	wdt = devm_kzalloc(dev, sizeof(*wdt), GFP_KERNEL);
	if (!wdt)
		return -ENOMEM;
	
	wdt->dev = &pdev->dev;
	   
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
    {
		wdtDbg("[%s]: failed to get IORESOURCE_MEM\n", __func__);
		return -ENODEV;
	}
    wdt->reg_base = devm_ioremap_resource(&pdev->dev, res);
	check_osc_clk();
   	wdt->wdt_device = infinity_wdd;

	watchdog_set_drvdata(&wdt->wdt_device, wdt);

	if (infinity_wdt_set_heartbeat(&infinity_wdd, tmr_margin)) {
		started = infinity_wdt_set_heartbeat(&infinity_wdd,
					CONFIG_WATCHDOG_DEFAULT_TIME);

		if (started == 0)
			dev_info(dev,
			   "tmr_margin value out of range, default %d used\n",
			       CONFIG_WATCHDOG_DEFAULT_TIME);
		else
			dev_info(dev, "default timer value is out of range, "
							"cannot start\n");
	}

	//watchdog_set_nowayout(&infinity_wdd, nowayout);

	ret = watchdog_register_device(&infinity_wdd);
	if (ret) {
		dev_err(dev, "cannot register watchdog (%d)\n", ret);
		goto err;
	}
  
    //init_timer(&wdt_timer);
    
    //wdt_timer.data = (unsigned long)tmr_margin;
    //wdt_timer.function = infinity_wdt_timer;
    //infinity_wdt_start(&infinity_wdd);

	return 0;

 err:
	watchdog_unregister_device(&infinity_wdd);

	return ret;
}

static int infinity_wdt_remove(struct platform_device *dev)
{
    wdtDbg("[WatchDog]infinity_wdt_remove \n");
    infinity_wdt_stop(&infinity_wdd);
	watchdog_unregister_device(&infinity_wdd);

	return 0;
}

static void infinity_wdt_shutdown(struct platform_device *dev)
{
    wdtDbg("[WatchDog]infinity_wdt_shutdown \n");

	infinity_wdt_stop(&infinity_wdd);
}

#ifdef CONFIG_PM

static int infinity_wdt_suspend(struct platform_device *dev, pm_message_t state)
{
    wdtDbg("[WatchDog]infinity_wdt_suspend \n");
    if(watchdog_open_flag==1) {
        infinity_wdt_stop(&infinity_wdd);
    }
	return 0;
}

static int infinity_wdt_resume(struct platform_device *dev)
{
    wdtDbg("[WatchDog]infinity_wdt_resume \n");
	/* Restore watchdog state. */
    if(watchdog_open_flag==1) {
        infinity_wdt_start(&infinity_wdd);
    }
	return 0;
}

#else
#define infinity_wdt_suspend NULL
#define infinity_wdt_resume  NULL
#endif /* CONFIG_PM */




static const struct of_device_id ms_watchdog_of_match_table[] = {
    { .compatible = "sstar,infinity-wdt" },
    {}
};
MODULE_DEVICE_TABLE(of, ms_watchdog_of_match_table);

static struct platform_driver infinity_wdt_driver = {
	.probe		= infinity_wdt_probe,
	.remove		= infinity_wdt_remove,
	.shutdown	= infinity_wdt_shutdown,
#ifdef CONFIG_PM	
	.suspend	= infinity_wdt_suspend,
	.resume		= infinity_wdt_resume,
#endif	
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "infinity-wdt",
        .of_match_table = ms_watchdog_of_match_table,
	},
};

module_platform_driver(infinity_wdt_driver);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("infinity Watchdog Device Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
MODULE_ALIAS("platform:infinity-wdt");


