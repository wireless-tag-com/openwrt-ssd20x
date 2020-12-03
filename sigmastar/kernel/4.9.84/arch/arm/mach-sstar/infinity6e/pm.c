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
#include <linux/delay.h>


#define FIN     printk(KERN_ERR"[%s]+++\n",__FUNCTION__)
#define FOUT    printk(KERN_ERR"[%s]---\n",__FUNCTION__)
#define HERE    printk(KERN_ERR"%s: %d\n",__FILE__,__LINE__)

extern void sram_suspend_imi(void);
static void (*mstar_suspend_imi_fn)(void);
static void __iomem *suspend_imi_vbase;

static int mstar_suspend_ready(unsigned long ret)
{
    mstar_suspend_imi_fn = fncpy(suspend_imi_vbase, (void*)&sram_suspend_imi, 0x1000);

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

static void TurnOnISOFlow(void)
{
retry:
    // Turn-on ISO flow
    OUTREG8(0x1F00680C, 0x00);
    mdelay(2);
    if ((INREG8(0x1F006820) & 0x08) != 0x00) {
        goto retry;
    }
    OUTREG8(0x1F00680C, 0x01);
    mdelay(2);
    if ((INREG8(0x1F006820) & 0x08) != 0x08) {
        goto retry;
    }
    OUTREG8(0x1F00680C, 0x03);
    mdelay(2);
    if ((INREG8(0x1F006820) & 0x08) != 0x00){
        goto retry;
    }
    OUTREG8(0x1F00680C, 0x07);
    mdelay(2);
    if ((INREG8(0x1F006820) & 0x08) != 0x08){
        goto retry;
    }
    OUTREG8(0x1F00680C, 0x05);
    mdelay(2);
    if ((INREG8(0x1F006820) & 0x08) != 0x00) {
        goto retry;;
    }
    OUTREG8(0x1F00680C, 0x01);
    mdelay(2);
    if ((INREG8(0x1F006820) & 0x08) != 0x08) {
        goto retry;
    }
    OUTREG8(0x1F00680C, 0x00);
    mdelay(2);
    if ((INREG8(0x1F006820) & 0x08) != 0x00) {
        goto retry;
    }
}

int __init mstar_pm_init(void)
{
    unsigned int resume_pbase = virt_to_phys(cpu_resume);
    suspend_imi_vbase = __arm_ioremap_exec(0xA0010000, 0x1000, false);  //put suspend code at IMI offset 64K;

    suspend_set_ops(&mstar_suspend_ops);

#if 0
    OUTREG16(0x1F001CEC, (resume_pbase & 0xFFFF));
    OUTREG16(0x1F001CF0, ((resume_pbase >> 16) & 0xFFFF));
#else
    OUTREG16(0x1F006810, (resume_pbase & 0xFFFF));
    OUTREG16(0x1F006800, 0x0020);

    // Turn-on ISO flow
    TurnOnISOFlow();

    OUTREG16(0x1F006810, ((resume_pbase >> 16) & 0xFFFF));
    OUTREG16(0x1F006800, 0x0040);

    // Turn-on ISO flow
    TurnOnISOFlow();
    OUTREG16(0x1F006800, 0x0000);
#endif

    printk(KERN_INFO "[%s] resume_pbase=0x%08X, suspend_imi_vbase=0x%08X\n", __func__, (unsigned int)resume_pbase, (unsigned int)suspend_imi_vbase);
    return 0;
}
