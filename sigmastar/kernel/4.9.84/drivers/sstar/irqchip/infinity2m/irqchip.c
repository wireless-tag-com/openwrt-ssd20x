/*
* irqchip.c- Sigmastar
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

/*         _ _ _ _ _ _ _ _ _ _                  */
/*        |                   |                 */
/*        |  PM_SLEEP_IRQ(32) |                 */
/*        |_ _ _ _ _ _ _ _ _ _|                 */
/*        |                   |                 */
/*        |    MS_FIQ (32)    |                 */
/*        |_ _ _ _ _ _ _ _ _ _| ms_fiq          */
/*        |                   |                 */
/*        |    MS_IRQ (64)    |                 */
/*        |_ _ _ _ _ _ _ _ _ _| ms_irq          */
/*        |                   |                 */
/*        |  ARM_INTERNAL(32) |                 */
/*        |_ _ _ _ _ _ _ _ _ _| gic_spi         */
/*        |                   |                 */
/*        |      PPI (16)     |                 */
/*        |_ _ _ _ _ _ _ _ _ _|                 */
/*        |                   |                 */
/*        |      SGI (16)     |                 */
/*        |_ _ _ _ _ _ _ _ _ _|                 */
/*                                              */

static void ms_main_irq_ack(struct irq_data *d)
{
    s16 ms_fiq;

    ms_fiq = d->hwirq - GIC_SPI_ARM_INTERNAL_NR - GIC_SPI_MS_IRQ_NR;

    if( ms_fiq >= 0 && ms_fiq < GIC_SPI_MS_FIQ_NR )
    {
        SETREG16( (BASE_REG_INTRCTL_PA + REG_ID_4C + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );
    }
    else if( ms_fiq >= GIC_SPI_MS_FIQ_NR )
    {
        pr_err("[%s] Unknown hwirq %lu, ms_fiq %d\n", __func__, d->hwirq, ms_fiq);
        return;
    }

    if(d && d->chip && d->parent_data && d->parent_data->chip->irq_ack)
    {
        irq_chip_ack_parent(d);
    }
}

static void ms_main_irq_eoi(struct irq_data *d)
{
    s16 ms_fiq;

    ms_fiq = d->hwirq - GIC_SPI_ARM_INTERNAL_NR - GIC_SPI_MS_IRQ_NR;

    if( ms_fiq >= 0 && ms_fiq < GIC_SPI_MS_FIQ_NR )
    {
        SETREG16( (BASE_REG_INTRCTL_PA + REG_ID_4C + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );
    }
    else if( ms_fiq >= GIC_SPI_MS_FIQ_NR )
    {
        pr_err("[%s] Unknown hwirq %lu, ms_fiq %d\n", __func__, d->hwirq, ms_fiq);
        return;
    }

    irq_chip_eoi_parent(d);
}

static void ms_main_irq_mask(struct irq_data *d)
{
    s16 ms_irq;
    s16 ms_fiq;

    ms_irq = d->hwirq - GIC_SPI_ARM_INTERNAL_NR;
    ms_fiq = d->hwirq - GIC_SPI_ARM_INTERNAL_NR - GIC_SPI_MS_IRQ_NR;
    pr_debug("[%s]  hwirq %lu\n", __func__, d->hwirq);

    if( ms_fiq >= 0 && ms_fiq < GIC_SPI_MS_FIQ_NR )
    {
        SETREG16( (BASE_REG_INTRCTL_PA + REG_ID_44 + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );
    }
    else if( ms_irq >=0 && ms_irq < GIC_SPI_MS_IRQ_NR )
    {
        SETREG16( (BASE_REG_INTRCTL_PA + REG_ID_54 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }

    irq_chip_mask_parent(d);
}

static void ms_main_irq_unmask(struct irq_data *d)
{
    s16 ms_irq;
    s16 ms_fiq;

    ms_irq = d->hwirq - GIC_SPI_ARM_INTERNAL_NR;
    ms_fiq = d->hwirq - GIC_SPI_ARM_INTERNAL_NR - GIC_SPI_MS_IRQ_NR;

    if( ms_fiq >= 0 && ms_fiq < GIC_SPI_MS_FIQ_NR )
    {
        CLRREG16( (BASE_REG_INTRCTL_PA + REG_ID_44 + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );
    }
    else if( ms_irq >=0 && ms_irq < GIC_SPI_MS_IRQ_NR )
    {
        CLRREG16( (BASE_REG_INTRCTL_PA + REG_ID_54 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, d->hwirq);
        return;
    }

    irq_chip_unmask_parent(d);
}

#ifdef CONFIG_SMP
static int ms_irq_set_affinity(struct irq_data *data, const struct cpumask *dest, bool force)
{
	//use a very simple implementation here...
	return irq_chip_set_affinity_parent(data,dest,true);
}
#endif

static int  ms_main_irq_set_type(struct irq_data *data, unsigned int flow_type)
{
    s16 ms_irq;
    s16 ms_fiq;

    if( (flow_type&IRQ_TYPE_EDGE_BOTH)==IRQ_TYPE_EDGE_BOTH)
    {
        pr_err("Not support IRQ_TYPE_EDGE_BOTH mode 0x%x\n",flow_type);
        return 0;
    }


    ms_irq = data->hwirq - GIC_SPI_ARM_INTERNAL_NR;
    ms_fiq = data->hwirq - GIC_SPI_ARM_INTERNAL_NR - GIC_SPI_MS_IRQ_NR;

    if( ms_fiq >= 0 && ms_fiq < GIC_SPI_MS_FIQ_NR )
    {
        if (flow_type&IRQ_TYPE_EDGE_FALLING)
            SETREG16( (BASE_REG_INTRCTL_PA + REG_ID_48 + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );
        else
            CLRREG16( (BASE_REG_INTRCTL_PA + REG_ID_48 + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );
    }
    else if( ms_irq >=0 && ms_irq < GIC_SPI_MS_IRQ_NR )
    {
        if (flow_type&IRQ_TYPE_LEVEL_LOW)
            SETREG16( (BASE_REG_INTRCTL_PA + REG_ID_58 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
        else
            CLRREG16( (BASE_REG_INTRCTL_PA + REG_ID_58 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
    }
    else
    {
        pr_err("[%s] Unknown hwirq %lu\n", __func__, data->hwirq);
        return -EINVAL;
    }
    irq_chip_set_type_parent(data, flow_type);
    return 0;

}

struct irq_chip ms_main_intc_irqchip = {
    .name = "MS_MAIN_INTC",
    .irq_ack = ms_main_irq_ack,
    .irq_eoi = ms_main_irq_eoi,
    .irq_mask = ms_main_irq_mask,
    .irq_unmask = ms_main_irq_unmask,
    .irq_set_type = ms_main_irq_set_type,
#ifdef CONFIG_SMP
	.irq_set_affinity=ms_irq_set_affinity,
#endif

};
EXPORT_SYMBOL(ms_main_intc_irqchip);

static int ms_main_intc_domain_translate(struct irq_domain *domain, struct irq_fwspec *fwspec,
                    unsigned long *hwirq, unsigned int *type)
{
    if (is_of_node(fwspec->fwnode)) {
        if (fwspec->param_count != 3)
            return -EINVAL;

        /* No PPI should point to this domain */
        if (fwspec->param[0] != 0)
            return -EINVAL;

        *hwirq = fwspec->param[1];
        *type = fwspec->param[2];
        return 0;
    }

    return -EINVAL;
}

static int ms_main_intc_domain_alloc(struct irq_domain *domain, unsigned int virq,
                                     unsigned int nr_irqs, void *data)
{
    struct irq_fwspec *fwspec = data;
    struct irq_fwspec parent_fwspec;
    irq_hw_number_t hwirq;
    unsigned int i;

    if (fwspec->param_count != 3)
        return -EINVAL;    /* Not GIC compliant */

    if (fwspec->param[0] != GIC_SPI)
        return -EINVAL;    /* No PPI should point to this domain */

    hwirq = fwspec->param[1];

    for (i = 0; i < nr_irqs; i++)
    {
        irq_domain_set_hwirq_and_chip(domain, virq + i, hwirq + i, &ms_main_intc_irqchip, NULL);
        pr_debug("[MS_MAIN_INTC] hw:%d -> v:%d\n", (unsigned int)hwirq+i, virq+i);
    }

    parent_fwspec = *fwspec;
    parent_fwspec.fwnode = domain->parent->fwnode;
    return irq_domain_alloc_irqs_parent(domain, virq, nr_irqs,
                        &parent_fwspec);
}

static void ms_main_intc_domain_free(struct irq_domain *domain, unsigned int virq, unsigned int nr_irqs)
{
    unsigned int i;

    for (i = 0; i < nr_irqs; i++) {
        struct irq_data *d = irq_domain_get_irq_data(domain, virq + i);
        irq_domain_reset_irq_data(d);
    }
}


struct irq_domain_ops ms_main_intc_domain_ops = {
    .translate  = ms_main_intc_domain_translate,
    .alloc      = ms_main_intc_domain_alloc,
    .free       = ms_main_intc_domain_free,
};


static int ms_irqchip_suspend(void)
{
    pr_debug("\nms_irqchip_suspend\n\n");
    return 0;
}

static void ms_irqchip_resume(void)
{
    pr_debug("\nms_irqchip_resume\n\n");

    //Patch for disable bypass IRQ/FIQ
    {
        u32 bypass = 0;
        bypass = INREG32(GIC_PHYS + 0x2000 + GIC_CPU_CTRL);
        bypass |= GICC_DIS_BYPASS_MASK;
        OUTREG32(GIC_PHYS + 0x2000 + GIC_CPU_CTRL, bypass | GICC_ENABLE);
    }
}

struct syscore_ops ms_irq_syscore_ops = {
    .suspend = ms_irqchip_suspend,
    .resume = ms_irqchip_resume,
};

static int __init ms_init_main_intc(struct device_node *np, struct device_node *interrupt_parent)
{
    struct irq_domain *parent_domain, *domain;

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

    //Patch for disable bypass IRQ/FIQ
    {
        u32 bypass = 0;
        bypass = INREG32(GIC_PHYS + 0x2000 + GIC_CPU_CTRL);
        bypass |= GICC_DIS_BYPASS_MASK;
        OUTREG32(GIC_PHYS + 0x2000 + GIC_CPU_CTRL, bypass | GICC_ENABLE);
    }

    domain = irq_domain_add_hierarchy(parent_domain, 0,
                    GIC_SPI_ARM_INTERNAL_NR+GIC_SPI_MS_IRQ_NR+GIC_SPI_MS_FIQ_NR,
                    np, &ms_main_intc_domain_ops, NULL);

    if (!domain)
    {
        pr_err("%s: %s allocat domain fail\n", __func__, np->name);
        return -ENOMEM;
    }

    register_syscore_ops(&ms_irq_syscore_ops);

    return 0;
}

IRQCHIP_DECLARE(ms_main_intc, "sstar,main-intc", ms_init_main_intc);
