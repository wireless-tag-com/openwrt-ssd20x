/*
* pm.c- Sigmastar
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
#include <linux/suspend.h>
#include <linux/io.h>
#include <asm/suspend.h>
#include <asm/fncpy.h>
#include <asm/cacheflush.h>
#include "ms_platform.h"

#define FIN     printk(KERN_ERR"[%s]+++\n",__FUNCTION__)
#define FOUT    printk(KERN_ERR"[%s]---\n",__FUNCTION__)
#define HERE    printk(KERN_ERR"%s: %d\n",__FILE__,__LINE__)

extern void infinity_suspend_imi(void);
static void (*mstar_suspend_imi_fn)(void);
static void __iomem *suspend_imi_vbase;

static int mstar_suspend_ready(unsigned long ret)
{
    mstar_suspend_imi_fn = fncpy(suspend_imi_vbase, (void*)&infinity_suspend_imi, 0x1000);

    //flush cache to ensure memory is updated before self-refresh
    __cpuc_flush_kern_all();
    //flush tlb to ensure following translation is all in tlb
    local_flush_tlb_all();
    mstar_suspend_imi_fn();
    return 0;
}

static int mstar_suspend_enter(suspend_state_t state)
{
    FIN;
    switch (state) 
    {
        case PM_SUSPEND_MEM:
            printk(KERN_INFO "state = PM_SUSPEND_MEM\n");
            cpu_suspend(0, mstar_suspend_ready);
            break;
        default:
            return -EINVAL;
    }

    return 0;
}

struct platform_suspend_ops mstar_suspend_ops = {
    .enter    = mstar_suspend_enter,
    .valid    = suspend_valid_only_mem,
};


int __init mstar_pm_init(void)
{
    unsigned int resume_pbase = virt_to_phys(cpu_resume);
    suspend_imi_vbase = __arm_ioremap_exec(0xA0010000, 0x1000, false);  //put suspend code at IMI offset 64K;

    suspend_set_ops(&mstar_suspend_ops);

    OUTREG16(0x1F001CEC, (resume_pbase & 0xFFFF));
    OUTREG16(0x1F001CF0, ((resume_pbase >> 16) & 0xFFFF));

    printk(KERN_INFO "[%s] resume_pbase=0x%08X, suspend_imi_vbase=0x%08X\n", __func__, (unsigned int)resume_pbase, (unsigned int)suspend_imi_vbase);

    return 0;
}
