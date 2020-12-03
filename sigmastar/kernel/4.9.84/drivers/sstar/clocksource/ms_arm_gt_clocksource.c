/*
* ms_arm_gt_clocksource.c- Sigmastar
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

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/cpu.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/sched_clock.h>

#include <asm/cputype.h>

#define GT_COUNTER0	0x00
#define GT_COUNTER1	0x04

#define GT_CONTROL	0x08
#define GT_CONTROL_TIMER_ENABLE		BIT(0)  /* this bit is NOT banked */
#define GT_CONTROL_COMP_ENABLE		BIT(1)	/* banked */
#define GT_CONTROL_IRQ_ENABLE		BIT(2)	/* banked */
#define GT_CONTROL_AUTO_INC		BIT(3)	/* banked */

#define GT_INT_STATUS	0x0c
#define GT_INT_STATUS_EVENT_FLAG	BIT(0)

#define GT_COMP0	0x10
#define GT_COMP1	0x14
#define GT_AUTO_INC	0x18

/*
 * We are expecting to be clocked by the ARM peripheral clock.
 *
 * Note: it is assumed we are using a prescaler value of zero, so this is
 * the units for all operations.
 */
static void __iomem *gt_base;
static unsigned long gt_clk_rate;


/*
 * To get the value from the Global Timer Counter register proceed as follows:
 * 1. Read the upper 32-bit timer counter register
 * 2. Read the lower 32-bit timer counter register
 * 3. Read the upper 32-bit timer counter register again. If the value is
 *  different to the 32-bit upper value read previously, go back to step 2.
 *  Otherwise the 64-bit timer counter value is correct.
 */
static u64 gt_counter_read(void)
{
	u64 counter;
	u32 lower;
	u32 upper, old_upper;

	upper = readl_relaxed(gt_base + GT_COUNTER1);
	do {
		old_upper = upper;
		lower = readl_relaxed(gt_base + GT_COUNTER0);
		upper = readl_relaxed(gt_base + GT_COUNTER1);
	} while (upper != old_upper);

	counter = upper;
	counter <<= 32;
	counter |= lower;
	return counter;
}


static cycle_t gt_clocksource_read(struct clocksource *cs)
{
	return gt_counter_read();
}

static struct clocksource gt_clocksource = {
	.name	= "arm_global_timer",
	.rating	= 300,
	.read	= gt_clocksource_read,
	.mask	= CLOCKSOURCE_MASK(64),
	.flags	= CLOCK_SOURCE_IS_CONTINUOUS,
};

#if 1//def CONFIG_CLKSRC_ARM_GLOBAL_TIMER_SCHED_CLOCK
static u64 notrace gt_sched_clock_read(void)
{
	return gt_counter_read();
}
#endif

static void __init gt_clocksource_init(void)
{
	writel(0, gt_base + GT_CONTROL);
	writel(0, gt_base + GT_COUNTER0);
	writel(0, gt_base + GT_COUNTER1);
	/* enables timer on all the cores */
	writel(GT_CONTROL_TIMER_ENABLE, gt_base + GT_CONTROL);

#if 1//def CONFIG_CLKSRC_ARM_GLOBAL_TIMER_SCHED_CLOCK
	sched_clock_register(gt_sched_clock_read, 64, gt_clk_rate);
#endif
	clocksource_register_hz(&gt_clocksource, gt_clk_rate);
}


static void __init ms_gt_clocksource_of_init(struct device_node *np)
{
	struct clk *gt_clk;
	int err = 0;

	/*
	 * In A9 r2p0 the comparators for each processor with the global timer
	 * fire when the timer value is greater than or equal to. In previous
	 * revisions the comparators fired when the timer value was equal to.
	 */
	if (read_cpuid_part() == ARM_CPU_PART_CORTEX_A9
	    && (read_cpuid_id() & 0xf0000f) < 0x200000) {
		pr_warn("global-timer: non support for this cpu version.\n");
		return;
	}

	gt_base = of_iomap(np, 0);
	if (!gt_base) {
		pr_warn("global-timer: invalid base address\n");
		return;
	}

	gt_clk = of_clk_get(np, 0);
	if (!IS_ERR(gt_clk)) {
		err = clk_prepare_enable(gt_clk);
		if (err)
			goto out_unmap;
	} else {
		pr_warn("global-timer: clk not found\n");
		err = -EINVAL;
		goto out_unmap;
	}

	gt_clk_rate = clk_get_rate(gt_clk);

	/* Immediately configure the timer on the boot CPU */
	gt_clocksource_init();

    if (!IS_ERR(gt_clk)) {
        clk_put(gt_clk)
    }

	return;

out_unmap:
    if (!IS_ERR(gt_clk)) {
        clk_put(gt_clk)
    }
	iounmap(gt_base);
	WARN(err, "ARM Global timer register failed (%d)\n", err);
}

/* Only tested on r2p2 and r3p0  */
CLOCKSOURCE_OF_DECLARE(arm_gt_clocksource, "sstar,arm-gt-clocksource", ms_gt_clocksource_of_init);
