/*
* ms_piu_timer.c- Sigmastar
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
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/clk.h>
#include <linux/sched_clock.h>
#include <linux/slab.h>


#include <asm/io.h>

#include "ms_platform.h"


//------------------------------------------------------------------------------
//  Macros
//------------------------------------------------------------------------------
#define	COPY_RTC_CNT 0
#define USE_ARM_GT      0 //# for better schedule resolution

#if COPY_RTC_CNT

#define TIMER_RTC_BASE          GET_REG_ADDR(MS_BASE_REG_RIU_PA, 0x000900)
#define CHIPTOP_BASE            GET_REG_ADDR(MS_BASE_REG_RIU_PA, 0x080F00)
#define CPY_RTC_TIMER_INTERVAL  1000 / HZ

//volatile PREG_TIMER_st g_pTimer2Regs = NULL;
volatile PREG_CHIPTOP_st g_pCHIPTOPReg = NULL;
volatile PREG_RTC_st g_pRTCReg = NULL;

struct timer_list cpy_rtc_timer;

extern DEVINFO_RTK_FLAG ms_devinfo_rtk_flag(void);
#endif


extern unsigned long XTAL_CLK;

#ifdef __arm__
void cap_delay_t(u32 tick)
{
	__asm__ __volatile__(
"1:	    nop\n"
"       subs %0, %0, #1\n"
"       bne 1b"
	:
	: "r" (tick));
}
#endif

#define TIMER_ENABLE			(0x1)
#define TIMER_TRIG				(0x2)
#define TIMER_INTERRUPT		    (0x100)
#define TIMER_CLEAR			    (0x4)
#define TIMER_CAPTURE       	(0x8)
#define ADDR_TIMER_MAX_LOW 	    (0x2<<2)
#define ADDR_TIMER_MAX_HIGH 	(0x3<<2)


struct ms_piu_timer {
	void __iomem			   *base;
	unsigned long				freq;
	int					irq;
};

struct ms_piu_timer_clockevent {
	struct clock_event_device   event;
	struct ms_piu_timer			timer;
	struct irqaction			irqaction;
};

struct ms_piu_timer_clocksource {
	struct ms_piu_timer			timer;
	struct clocksource			source;
};


static int timer_set_next_event(unsigned long next, struct clock_event_device *evt);
static int timer_set_state_periodic(struct clock_event_device *evt);
static int timer_set_state_oneshot(struct clock_event_device *evt);
static int timer_set_oneshot_stopped(struct clock_event_device *evt);
static int timer_set_state_shutdown(struct clock_event_device *evt);

static inline struct ms_piu_timer_clockevent *to_ms_clockevent(struct clock_event_device *evt)
{
	return container_of(evt, struct ms_piu_timer_clockevent, event);
}

static inline struct ms_piu_timer_clocksource *to_ms_clocksource(struct clocksource *cs)
{
	return container_of(cs, struct ms_piu_timer_clocksource, source);
}


static cycle_t timer_read(struct clocksource *cs)
{

	struct ms_piu_timer_clocksource *ms_cs=to_ms_clocksource(cs);
#if 0
	unsigned int high=INREG16(ms_cs->timer.base+(0x5<<2));
	unsigned int low0=INREG16(ms_cs->timer.base+(0x4<<2));
	unsigned int low1=INREG16(ms_cs->timer.base+(0x4<<2));
	if(low0>low1)
	{
		high=INREG16(ms_cs->timer.base+(0x5<<2));
	}
#else
    //hardware capture
    unsigned int low0=INREG16(ms_cs->timer.base+(0x4<<2));
    unsigned int high=INREG16(ms_cs->timer.base+(0x5<<2));
#endif
	return ((high&0xffff)<<16) + (low0&0xffff) ;

}



static struct ms_piu_timer_clocksource* sched_clocksource=NULL;
static u64 notrace timer_read_sched_clock(void)
{
	if(sched_clocksource)
	{
		return timer_read(&sched_clocksource->source);
	}
	return 0;
}

static int __init ms_piu_timer_clocksource_of_init(struct device_node *np)
{

	struct clk *clk;
	struct resource res;

	struct ms_piu_timer_clocksource *ms_cs = kzalloc(sizeof(*ms_cs), GFP_KERNEL);
	BUG_ON(ms_cs==NULL);
	BUG_ON(of_address_to_resource(np, 0, &res));

	clk = of_clk_get(np, 0);
	BUG_ON(IS_ERR(clk));


	ms_cs->timer.base=(void *)res.start;

	ms_cs->timer.freq=clk_get_rate(clk);

	clk_put(clk);

	ms_cs->source.name = "timer_clksrc";
	ms_cs->source.rating = 200;
	ms_cs->source.read = timer_read;
	ms_cs->source.mask = CLOCKSOURCE_MASK(32);
	ms_cs->source.flags	= CLOCK_SOURCE_IS_CONTINUOUS;


	/* setup timer 2 as free-running clocksource */
	//make sure timer 2 is disable
	CLRREG16(ms_cs->timer.base, TIMER_ENABLE);

	//set max period
	OUTREG16(ms_cs->timer.base+(0x2<<2),0xffff);
	OUTREG16(ms_cs->timer.base+(0x3<<2),0xffff);

	//enable timer 2
	SETREG16(ms_cs->timer.base, TIMER_ENABLE);


	clocksource_register_hz(&ms_cs->source, ms_cs->timer.freq);

	sched_clocksource=ms_cs;
	sched_clock_register(timer_read_sched_clock, 32, ms_cs->timer.freq);

	return 0;


}


static int timer_set_state_periodic(struct clock_event_device *evt)
{
    unsigned short ctl=TIMER_INTERRUPT;
    unsigned long interval;
    struct ms_piu_timer_clockevent *ms_ce=to_ms_clockevent(evt);

    interval = (ms_ce->timer.freq / HZ)  ;
    OUTREG16(ms_ce->timer.base + ADDR_TIMER_MAX_LOW, (interval &0xffff));
    OUTREG16(ms_ce->timer.base + ADDR_TIMER_MAX_HIGH, (interval >>16));
    ctl|=TIMER_ENABLE;
    SETREG16(ms_ce->timer.base, ctl);

    return 0;
}

static int timer_set_state_oneshot(struct clock_event_device *evt)
{
    unsigned short ctl=TIMER_INTERRUPT;
    struct ms_piu_timer_clockevent *ms_ce=to_ms_clockevent(evt);

    /* period set, and timer enabled in 'next_event' hook */
    ctl|=TIMER_TRIG;
    SETREG16(ms_ce->timer.base, ctl);

    return 0;
}

static int timer_set_oneshot_stopped(struct clock_event_device *evt)
{
    unsigned short ctl=TIMER_INTERRUPT;
    struct ms_piu_timer_clockevent *ms_ce=to_ms_clockevent(evt);

    /* period set, and timer enabled in 'next_event' hook */
    ctl&=~TIMER_TRIG;
    SETREG16(ms_ce->timer.base, ctl);

    return 0;
}

static int timer_set_state_shutdown(struct clock_event_device *evt)
{
    unsigned short ctl=TIMER_INTERRUPT;
    struct ms_piu_timer_clockevent *ms_ce=to_ms_clockevent(evt);

    ctl|=TIMER_ENABLE;
    CLRREG16(ms_ce->timer.base, ctl);

    return 0;
}

static int timer_set_next_event(unsigned long next, struct clock_event_device *evt)
{

	struct ms_piu_timer_clockevent *ms_ce=to_ms_clockevent(evt);
	//stop timer
	//OUTREG16(clkevt_base, 0x0);

	//set period
	OUTREG16(ms_ce->timer.base + ADDR_TIMER_MAX_LOW, (next &0xffff));
	OUTREG16(ms_ce->timer.base + ADDR_TIMER_MAX_HIGH, (next >>16));

	//enable timer
	SETREG16(ms_ce->timer.base, TIMER_TRIG|TIMER_INTERRUPT);//default

	return 0;
}


//static DEFINE_SPINLOCK(hw_sys_timer_lock);
#if COPY_RTC_CNT

#define RTC_FREQ 32768
static void ms_copy_rtc_cnt(unsigned long dummy)
{
    u32 rtc0, rtc1, secl, sech;
    u32 u32RTCCounts;
    static u32 dtmr = 0, drtc = 0;

    //rtc freq 32k
    rtc0 = INREG16(&g_pCHIPTOPReg->u16DUMMY_RTC_SECOND_CNT_H) << 16;
    rtc0 += INREG16(&g_pCHIPTOPReg->u16DUMMY_RTC_SECOND_CNT_L);
    rtc0 = rtc0 * RTC_FREQ + INREG16(&g_pCHIPTOPReg->u16DUMMY_RTC_SUB_CNT);

    secl = INREG16(&g_pRTCReg->RTC_RAW_SEC_CNT_L);
    sech = INREG16(&g_pRTCReg->RTC_RAW_SEC_CNT_H);
    u32RTCCounts = INREG16(&g_pRTCReg->RTC_RAW_SUB_CNT);

    // Lock mutex
    OUTREG16(&g_pCHIPTOPReg->u16DUMMY_RTC_SUB_CNT, 0x8000);

    // Update RTC sec in CHIPTOP dummy register for GPS
    OUTREG16(&g_pCHIPTOPReg->u16DUMMY_RTC_SECOND_CNT_L, secl);
    OUTREG16(&g_pCHIPTOPReg->u16DUMMY_RTC_SECOND_CNT_H, sech);

    // Reset Timer2
    //OUTREG16(&g_pTimer2Regs->TIMER_MAX_L, 0x0);

    // Update RTC sub cnt in CHIPTOP dummy register for GPS
    OUTREG16(&g_pCHIPTOPReg->u16DUMMY_RTC_SUB_CNT, u32RTCCounts);

    // Calculate tmr to rtc ratio
    rtc1 = (sech << 16) + secl;
    rtc1 = rtc1 * RTC_FREQ + u32RTCCounts;
    drtc += (rtc1 - rtc0);
    dtmr += timer_read(&clocksource_timer) - dtmr;

    if (drtc > RTC_FREQ) // accumulate for 1 sec
    {
        OUTREG16(&g_pCHIPTOPReg->REG_CHIPTOP_1F, (dtmr / drtc));
        dtmr = drtc = 0;
    }

    cpy_rtc_timer.expires = jiffies + msecs_to_jiffies(CPY_RTC_TIMER_INTERVAL);
    add_timer(&cpy_rtc_timer);
}

static void ms_copy_rtc_cnt_init(void)
{
    g_pRTCReg = (PREG_RTC_st)(TIMER_RTC_BASE);
    //g_pTimer2Regs = (PREG_TIMER_st)(TIMER_CLKSRC_BASE);
    g_pCHIPTOPReg = (PREG_CHIPTOP_st)(CHIPTOP_BASE);

	OUTREG16(&g_pCHIPTOPReg->u16DUMMY_RTC_SECOND_CNT_L, INREG16(&g_pRTCReg->RTC_SECOND_CNT_L));
	OUTREG16(&g_pCHIPTOPReg->u16DUMMY_RTC_SECOND_CNT_H, INREG16(&g_pRTCReg->RTC_SECOND_CNT_H));
	OUTREG16(&g_pCHIPTOPReg->u16DUMMY_RTC_SUB_CNT, INREG16(&g_pRTCReg->RTC_SUB_CNT));

	init_timer(&cpy_rtc_timer);
    cpy_rtc_timer.expires = jiffies + msecs_to_jiffies(CPY_RTC_TIMER_INTERVAL);
	cpy_rtc_timer.data = 0;
	cpy_rtc_timer.function = ms_copy_rtc_cnt;
    add_timer(&cpy_rtc_timer);

    //call right now here
    ms_copy_rtc_cnt(0);
}
#endif

static irqreturn_t ms_timer_interrupt(int irq, void *dev_id)
{

	struct clock_event_device *evt = dev_id;
#if defined(CONFIG_MSTAR_CEDRIC) && defined(CONFIG_PROFILING) && defined(CONFIG_OPROFILE)
	int cpu = smp_processor_id();
    extern u32 armpmu_enable_flag[];
#endif

	evt->event_handler(evt);

/* Mantis 212633 oprofile can't work */
#if defined(CONFIG_MSTAR_CEDRIC) && defined(CONFIG_PROFILING) && defined(CONFIG_OPROFILE)
    /* local_timer call pmu_handle_irq instead of PMU interrupt can't raise by itself. */
    if (armpmu_enable_flag[cpu]) {
        extern irqreturn_t armpmu_handle_irq(int irq_num, void *dev);
        armpmu_handle_irq(cpu, NULL);
    }
#endif


    return IRQ_HANDLED;

}



static int __init ms_piu_timer_clockevent_of_init(struct device_node *np)
{
	struct ms_piu_timer_clockevent *ms_ce = kzalloc(sizeof(*ms_ce), GFP_KERNEL);

	struct clk *clk;
	struct resource res;

	BUG_ON(ms_ce==NULL);
	BUG_ON(of_address_to_resource(np, 0, &res));

	clk = of_clk_get(np, 0);
		BUG_ON(IS_ERR(clk));

	ms_ce->timer.base=(void *)res.start;

	ms_ce->timer.freq=clk_get_rate(clk);

	clk_put(clk);

	ms_ce->timer.irq =irq_of_parse_and_map(np, 0);
    printk("%s: irq %d", __func__, ms_ce->timer.irq);
	if (ms_ce->timer.irq == 0)
	{
		panic("No IRQ for clock event timer");
	}

	ms_ce->event.name		= "timer_clkevt";
	ms_ce->event.shift		= 32;
	ms_ce->event.features   = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT;
#if 1
	ms_ce->event.set_state_periodic	= timer_set_state_periodic;
	ms_ce->event.set_state_oneshot	= timer_set_state_oneshot;
	ms_ce->event.set_state_oneshot_stopped	= timer_set_oneshot_stopped;
	ms_ce->event.set_state_shutdown	= timer_set_state_shutdown;
#else
	ms_ce->event.set_mode	= timer_set_mode;
#endif
	ms_ce->event.set_next_event	= timer_set_next_event;
	ms_ce->event.rating		= 200;
	ms_ce->event.cpumask	= cpu_all_mask;
	ms_ce->event.irq=ms_ce->timer.irq;

	ms_ce->irqaction.name = "MS Timer Tick";
	ms_ce->irqaction.flags = IRQF_TIMER | IRQF_IRQPOLL;
	ms_ce->irqaction.handler = ms_timer_interrupt;
    ms_ce->irqaction.dev_id = &ms_ce->event;




	clockevents_calc_mult_shift(&ms_ce->event,ms_ce->timer.freq,5);
	ms_ce->event.max_delta_ns = clockevent_delta2ns(0xffffffff, &ms_ce->event);
	ms_ce->event.min_delta_ns = clockevent_delta2ns(0xf, &ms_ce->event);
	BUG_ON( setup_irq(ms_ce->event.irq, &ms_ce->irqaction) );

	clockevents_register_device( &ms_ce->event);


#if COPY_RTC_CNT
    //put it last, because it will use the clock event device
    if (DEVINFO_RTK_FLAG_1 == ms_devinfo_rtk_flag())
        ms_copy_rtc_cnt_init();
#endif


    return 0;
}


CLOCKSOURCE_OF_DECLARE(ms_piu_timer_cs, "sstar,piu-clocksource", ms_piu_timer_clocksource_of_init);
CLOCKSOURCE_OF_DECLARE(ms_piu_timer_ce, "sstar,piu-clockevent", ms_piu_timer_clockevent_of_init);
