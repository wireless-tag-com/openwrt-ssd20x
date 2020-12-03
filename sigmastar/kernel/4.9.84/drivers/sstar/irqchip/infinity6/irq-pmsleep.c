/*
* irq-pmsleep.c- Sigmastar
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
#include <linux/io.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/irqdomain.h>
#include <linux/of.h>
#include <linux/syscore_ops.h>
#include <asm/mach/irq.h>
#include <linux/irqchip/arm-gic.h>
#include <linux/irqchip.h>
#include <linux/irqdesc.h>

#include <dt-bindings/interrupt-controller/arm-gic.h>

#include "irqs.h"
#include "registers.h"

#include "_ms_private.h"
#include "ms_platform.h"
#include "ms_types.h"

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>

#define CONFIG_PATCH_STATUS_FAILS 1


#define PMGPIO_OEN                BIT0
#define PMGPIO_OUTPUT             BIT1
#define PMGPIO_INPUT              BIT2
#define PMGPIO_FIQ_MASK           BIT4
#define PMGPIO_FIQ_FROCE          BIT5
#define PMGPIO_FIQ_CLEAR          BIT6
#define PMGPIO_FIQ_POLARITY       BIT7
#define PMGPIO_FIQ_FINAL_STATUS   BIT8
#define PMGPIO_FIQ_RAW_STATUS     BIT9

static void ms_pm_irq_ack(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;
    pr_debug("[%s] hw:%d \n",__FUNCTION__, pmsleep_fiq);

    if(pmsleep_fiq < PMSLEEP_FIQ_END)
    {
        SETREG16(BASE_REG_PMGPIO_PA + (pmsleep_fiq << 2), PMGPIO_FIQ_CLEAR);
    }
	else
	{
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
	}
}

static void ms_pm_irq_eoi(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;
    pr_debug("[%s] hw:%d \n",__FUNCTION__, pmsleep_fiq);

    if(pmsleep_fiq < PMSLEEP_FIQ_END)
    {
        SETREG16(BASE_REG_PMGPIO_PA + (pmsleep_fiq << 2), PMGPIO_FIQ_CLEAR);
    }
	else
	{
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
	}
}

static void ms_pm_irq_mask(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;
    pr_debug("[%s] hw:%d \n",__FUNCTION__, pmsleep_fiq);

    if(pmsleep_fiq < PMSLEEP_FIQ_END)
    {
        SETREG16(BASE_REG_PMGPIO_PA + (pmsleep_fiq << 2), PMGPIO_FIQ_MASK);
    }
    else if(pmsleep_fiq < PMSLEEP_IRQ_END)
    {
        SETREG8(BASE_REG_PMSLEEP_PA + REG_ID_08, 1<<(pmsleep_fiq-PMSLEEP_IRQ_START) );
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }
}

static void ms_pm_irq_unmask(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;
    pr_debug("[%s] hw:%d \n",__FUNCTION__, pmsleep_fiq);

    if(pmsleep_fiq < PMSLEEP_FIQ_END)
    {
        CLRREG16(BASE_REG_PMGPIO_PA + (pmsleep_fiq << 2), PMGPIO_FIQ_MASK);
    }
    else if(pmsleep_fiq < PMSLEEP_IRQ_END)
    {
        CLRREG8(BASE_REG_PMSLEEP_PA + REG_ID_08, 1<<(pmsleep_fiq-PMSLEEP_IRQ_START) );
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }

    CLRREG16(BASE_REG_INTRCTL_PA + REG_ID_54, BIT2);
}

static int ms_pm_irq_set_type(struct irq_data *d, unsigned int type)
{
    U16 pmsleep_fiq;
    pr_debug("%s %d type:0x%08x\n", __FUNCTION__, __LINE__, type);

    pmsleep_fiq = d->hwirq;

    if(pmsleep_fiq < PMSLEEP_FIQ_END)
    {
        switch(type)
        {
        case IRQ_TYPE_EDGE_FALLING:
        case IRQ_TYPE_LEVEL_LOW:
            SETREG16(BASE_REG_PMGPIO_PA + (pmsleep_fiq << 2), PMGPIO_FIQ_POLARITY);
            break;
        case IRQ_TYPE_EDGE_RISING:
        case IRQ_TYPE_LEVEL_HIGH:
            CLRREG16(BASE_REG_PMGPIO_PA + (pmsleep_fiq << 2), PMGPIO_FIQ_POLARITY);
            break;
        default:
            return -EINVAL;
        }
    }
    else if(pmsleep_fiq < PMSLEEP_IRQ_END)
    {
        switch(type)
        {
        case IRQ_TYPE_EDGE_FALLING:
        case IRQ_TYPE_LEVEL_LOW:
            SETREG8(BASE_REG_PMSLEEP_PA + REG_ID_09, 1<<(pmsleep_fiq-PMSLEEP_IRQ_START) );
            break;
        case IRQ_TYPE_EDGE_RISING:
        case IRQ_TYPE_LEVEL_HIGH:
            CLRREG8(BASE_REG_PMSLEEP_PA + REG_ID_09, 1<<(pmsleep_fiq-PMSLEEP_IRQ_START) );
            break;
        default:
            return -EINVAL;
        }
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return -EINVAL;
    }

    return 0;
}

struct irq_chip ms_pm_intc_irqchip = {
    .name = "MS_PM_INTC",
    .irq_ack = ms_pm_irq_ack,
    .irq_eoi = ms_pm_irq_eoi,
    .irq_mask = ms_pm_irq_mask,
    .irq_unmask = ms_pm_irq_unmask,
    .irq_set_type = ms_pm_irq_set_type,
};
EXPORT_SYMBOL(ms_pm_intc_irqchip);

static DEFINE_SPINLOCK(ss_irq_controller_lock);

static void ms_handle_cascade_pm_irq(struct irq_desc *desc)
{
    unsigned int cascade_irq = 0xFFFFFFFF, i;
    unsigned int virq ;
    struct irq_chip *chip = irq_desc_get_chip(desc);
    struct irq_domain *domain = irq_desc_get_handler_data(desc);
    unsigned int final_status;

    if(!domain)
    {
        printk("[%s] err %d \n", __FUNCTION__, __LINE__);
        goto exit;
    }

    spin_lock(&ss_irq_controller_lock);
#ifdef CONFIG_PATCH_STATUS_FAILS
    for(i=0;i<=75;i++)
    {
        final_status =INREG16(BASE_REG_PMGPIO_PA+i*4);
        if(!(final_status&PMGPIO_FIQ_MASK))
        {
            if( (final_status&PMGPIO_FIQ_POLARITY)? !(final_status&PMGPIO_INPUT):(final_status&PMGPIO_INPUT))
            {
                cascade_irq = i;
                pr_debug("[%s] Get hwirq:%d, Reg:0x%04x\n", __FUNCTION__, cascade_irq, final_status);
                break;
            }
        }
    }
#else
    final_status = INREG16(BASE_REG_PMSLEEP_PA + REG_ID_04) | (INREG16(BASE_REG_PMSLEEP_PA + REG_ID_05)<<16);
    for(i=0;i<32;i++)
    {
        if(0 !=(final_status & (1<<i)))
        {
            cascade_irq = i;// + (GIC_HWIRQ_MS_START+GIC_SPI_MS_IRQ_NR+GIC_SPI_MS_FIQ_NR);
            break;
        }
    }
#endif
    spin_unlock(&ss_irq_controller_lock);

    if(0xFFFFFFFF==cascade_irq)
    {

#ifdef CONFIG_PATCH_STATUS_FAILS
        pr_err("[%s:%d] Clean all pmgpio status\n", __FUNCTION__, __LINE__);
        for(i=0;i<=75;i++)
        {
            SETREG16(BASE_REG_PMGPIO_PA+i*4, PMGPIO_FIQ_CLEAR);
        }
#else
        pr_err("[%s:%d] error final_status:%d 0x%04X virq:%d\n", __FUNCTION__, __LINE__, cascade_irq, final_status, virq);
        panic();
#endif
        chained_irq_exit(chip, desc);
        goto exit;
    }

    virq = irq_find_mapping(domain, cascade_irq);
    if(!virq)
    {
        printk("[%s] err %d \n", __FUNCTION__, __LINE__);
        goto exit;
    }
    pr_debug("%s %d final_status:%d 0x%04X virq:%d\n", __FUNCTION__, __LINE__, cascade_irq, final_status, virq);
    chained_irq_enter(chip, desc);
    generic_handle_irq(virq);

exit:
    chained_irq_exit(chip, desc);
}

static int ms_pm_intc_domain_translate(struct irq_domain *domain, struct irq_fwspec *fwspec,
                    unsigned long *hwirq, unsigned int *type)
{
    if (is_of_node(fwspec->fwnode)) {
        if (fwspec->param_count != 1)
            return -EINVAL;
        *hwirq = fwspec->param[0];
        return 0;
    }

    return -EINVAL;
}

static int ms_pm_intc_domain_alloc(struct irq_domain *domain, unsigned int virq,
                                     unsigned int nr_irqs, void *data)
{
    struct irq_fwspec *fwspec = data;
    struct irq_fwspec parent_fwspec;
    irq_hw_number_t hwirq;
    unsigned int i;

    if (fwspec->param_count != 1)
        return -EINVAL;

    hwirq = fwspec->param[0];

    for (i = 0; i < nr_irqs; i++)
    {
        irq_domain_set_info(domain, virq + i, hwirq + i , &ms_pm_intc_irqchip, NULL, handle_fasteoi_irq, NULL, NULL);
        pr_err("[MS_PM_INTC] hw:%d -> v:%d\n", (unsigned int)hwirq+i, virq+i);
    }

    parent_fwspec = *fwspec;

    return 0;
}

static void ms_pm_intc_domain_free(struct irq_domain *domain, unsigned int virq, unsigned int nr_irqs)
{
    unsigned int i;

    for (i = 0; i < nr_irqs; i++) {
        struct irq_data *d = irq_domain_get_irq_data(domain, virq + i);
        irq_domain_reset_irq_data(d);
    }
}


struct irq_domain_ops ms_pm_intc_domain_ops = {
    .translate  = ms_pm_intc_domain_translate,
    .alloc      = ms_pm_intc_domain_alloc,
    .free       = ms_pm_intc_domain_free,
};


static int __init ms_init_pm_intc(struct device_node *np, struct device_node *interrupt_parent)
{
    struct irq_domain *parent_domain, *ms_pm_irq_domain;
    int irq=0;
    if (!interrupt_parent)
    {
        pr_err("%s: %s no parent\n", __func__, np->name);
        return -ENODEV;
    }

    pr_err("%s: np->name=%s, parent=%s\n", __func__, np->name, interrupt_parent->name);

    parent_domain = irq_find_host(interrupt_parent);
    if (!parent_domain)
    {
        pr_err("%s: %s unable to obtain parent domain\n", __func__, np->name);
        return -ENXIO;
    }

    ms_pm_irq_domain = irq_domain_add_hierarchy(parent_domain, 0,
                    PMSLEEP_FIQ_NR,
                    np, &ms_pm_intc_domain_ops, NULL);

    if (!ms_pm_irq_domain)
    {
        pr_err("%s: %s allocat domain fail\n", __func__, np->name);
        return -ENOMEM;
    }

    irq = irq_of_parse_and_map(np, 0);
    if (!irq)
    {
        pr_err("Get irq err from DTS\n");
        return -EPROBE_DEFER;
    }

    irq_set_chained_handler_and_data(irq, ms_handle_cascade_pm_irq, ms_pm_irq_domain);

    return 0;
}

IRQCHIP_DECLARE(ms_pm_intc, "sstar,pm-intc", ms_init_pm_intc);
