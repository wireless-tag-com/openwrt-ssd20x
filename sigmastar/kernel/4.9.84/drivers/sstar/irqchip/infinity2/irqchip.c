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
#include <ms_msys.h>
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




/*           _ _ _ _ _ _ _ _ _ _                 			          */
/*        |                         |---------|       			   *//* 352 */
/*        |     MS_FIQ (32)   |               |      			   */
/*        |_ _ _ _ _ _ _ _ _ _|              |     			   */
/*        |                         |---------|      			   *//* 320 */
/*        |     MS_IRQ (32)   |              |         			   */
/*        |_ _ _ _ _ _ _ _ _ _|              |      			   */
/*        |                         |---------|       			   *//* 288 */
/*        |     pmuirq (32)    |               |        			   */
/*        |_ _ _ _ _ _ _ _ _ _|              |        			   */
/*        |                         |               |         			   *//* 256 */
/*        |     MS_FIQ (64)   |               | --ms_pmsleep     */
/*        |_ _ _ _ _ _ _ _ _ _|              |				   */
/*        |                          |              |   			          *//* 128 */
/*        |     MS_IRQ (64)   |    gic_spi |    		          */
/*        |_ _ _ _ _ _ _ _ _ _|	        |				   */
/*        |                             |           |     			   *//* 64 */
/*        |  ARM_INTERNAL(32) |          |     			   */
/*        |_ _ _ _ _ _ _ _ _ _ _ |-------   	  			   */
/*        |                         |                 				   *//* 32 */
/*        |      PPI (16)        |                				   */
/*        |_ _ _ _ _ _ _ _ _ _|                				   */
/*        |                         |                 				   */
/*        |      SGI (16)        |               				   */
/*        |_ _ _ _ _ _ _ _ _ _|                 				   */


static void ms_main_irq_ack(struct irq_data *d)
{
    s16 gic_spi_irq, ms_fiq, ms_fiq1;
    gic_spi_irq = d->hwirq;

    /* NOTE: only clear if it is FIQ */
//    if( d->hwirq < GIC_HWIRQ_MS_START )
//    {
//        return;
//    }
//    else
        if( gic_spi_irq >= GIC_SPI_MS_FIQ_START && gic_spi_irq < GIC_SPI_MS_FIQ_END )
    {
        ms_fiq = gic_spi_irq - GIC_SPI_MS_FIQ_START;

        OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_4C + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );

        if (ms_fiq ==  63)//PM_FIQ
            SETREG16( (BASE_REG_IRQ_PM + REG_ID_0C ), (1 << 12) );//clear timer2 (pm_fiq[12])
    }
	else if( gic_spi_irq >= GIC_SPI_MS_FIQ1_START && gic_spi_irq < GIC_SPI_MS_FIQ1_END )
    {
        ms_fiq1 = gic_spi_irq - GIC_SPI_MS_FIQ1_START;
        OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_4C + (ms_fiq1/16)*4 ) , (1 << (ms_fiq1%16)) );
    }
    if(d && d->chip && d->parent_data && d->parent_data->chip->irq_ack)
    {
        irq_chip_ack_parent(d);
    }
}

static void ms_main_irq_eoi(struct irq_data *d)
{
    s16 gic_spi_irq, ms_fiq, ms_fiq1;
    gic_spi_irq = d->hwirq;
    /* NOTE: only clear if it is FIQ */
//    if( d->hwirq < GIC_HWIRQ_MS_START )
//    {
//        return;
//    }
//    else 
if( gic_spi_irq >= GIC_SPI_MS_FIQ_START && gic_spi_irq < GIC_SPI_MS_FIQ_END )
    {
        ms_fiq = gic_spi_irq - GIC_SPI_MS_FIQ_START;

        OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_4C + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );

        if (ms_fiq ==  63)//PM_FIQ
            SETREG16( (BASE_REG_IRQ_PM + REG_ID_0C ), (1 << 12) );//clear timer2 (pm_fiq[12])
    }
	else if( gic_spi_irq >= GIC_SPI_MS_FIQ1_START && gic_spi_irq < GIC_SPI_MS_FIQ1_END )
    {
        ms_fiq1 = gic_spi_irq - GIC_SPI_MS_FIQ1_START;
        OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_4C + (ms_fiq1/16)*4 ) , (1 << (ms_fiq1%16)) );
    }
    irq_chip_eoi_parent(d);
}

static void ms_main_irq_mask(struct irq_data *d)
{
     s16 ms_irq;
    s16 ms_fiq;
    s16 gic_spi_irq;

    gic_spi_irq = d->hwirq;
//    pr_err("[@@-ms_irq_mask] gic_spi_irq %d\n", gic_spi_irq);
//    if( d->hwirq < GIC_HWIRQ_MS_START )
//    {
//        return;
//    }
//    else 
        if((gic_spi_irq >= GIC_SPI_MS_IRQ_START) && (gic_spi_irq < GIC_SPI_MS_IRQ_END))
	{
		/*MS_IRQ0*/
		ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ_START;
		SETREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_54 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
	}

	else if((gic_spi_irq >= GIC_SPI_MS_FIQ_START) && (gic_spi_irq < GIC_SPI_MS_FIQ_END))
	{
		/*MS_FIQ0*/
		ms_fiq = gic_spi_irq - GIC_SPI_MS_FIQ_START;
		SETREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_44 + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );
	}

	/*MS_FIQ 64!!!!*/
	else if (gic_spi_irq == INT_FIQ_64_SEC_GUARD_INT)
	{
		ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ1_START;
		SETREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_44 + 0 ) , (1 << (0)) );
	}
	/*MS_FIQ 65!!!!*/
	else if (gic_spi_irq == INT_FIQ_65_SD_CDZ_IN)
	{
		ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ1_START;
		SETREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_44 + 0 ) , (1 << (1)) );
	}
	/*MS_IRQ 1*/
	else if ((gic_spi_irq >= GIC_SPI_MS_IRQ1_START) && (gic_spi_irq < INT_FIQ_64_SEC_GUARD_INT))
	{
		ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ1_START;
		SETREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_54 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
	}

	else if ((gic_spi_irq > INT_FIQ_65_SD_CDZ_IN) && (gic_spi_irq < GIC_SPI_MS_IRQ1_END))
	{
		ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ1_START - 2;
		SETREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_54 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
	}

	else if((gic_spi_irq >= GIC_SPI_MS_FIQ1_START) && (gic_spi_irq < GIC_SPI_MS_FIQ1_END))
	{
		/*MS_FIQ1*/
		ms_fiq = gic_spi_irq - GIC_SPI_MS_FIQ1_START;
		SETREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_44 + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );
	}
	else if((gic_spi_irq == INT_169_REG_DUMMY_ECO_1_1) )
	{
	      /* sw interrupt, riu bank 0x101d, offset 0x2d[1] */
		SETREG16( (BASE_REG_MCU_ARM + REG_ID_2D + 0) , (1 << (1)) );
	}
	else if((gic_spi_irq == INT_168_REG_DUMMY_ECO_1_0) )
	{
	      /* sw interrupt, riu bank 0x101d, offset 0x2d[0] */
		SETREG16( (BASE_REG_MCU_ARM + REG_ID_2D + 0) , (1 << (0)) );
	}
	else if(((INT_162_IRQ_IN_0 <= gic_spi_irq) && (gic_spi_irq <= INT_165_FIQ_IN_1)) )
	{
		OUTREG32(GIC_PHYS + 0x1000  + + GIC_DIST_ENABLE_CLEAR + (d->hwirq / 32) * 4, (1<<(d->hwirq % 32)));
 	}
	else if((gic_spi_irq == INT_182_PMU_IRQ_0) ||(gic_spi_irq == INT_191_PMU_IRQ_1))
	{
	      /* PMU */
	}
    irq_chip_mask_parent(d);
}

static void ms_main_irq_unmask(struct irq_data *d)
{
    s16 ms_irq;
    s16 ms_fiq;
    s16 gic_spi_irq;

    gic_spi_irq = d->hwirq;
//	if ((gic_spi_irq != INT_IRQ_00_INT_UART0) || (gic_spi_irq != INT_FIQ_00_INT_TIMER0))
   //    if( d->hwirq < GIC_HWIRQ_MS_START )
//    {
//        return;
//    }
//    else 
        if((gic_spi_irq >= GIC_SPI_MS_IRQ_START) && (gic_spi_irq < GIC_SPI_MS_IRQ_END))
	{
		/*MS_IRQ0*/
		ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ_START;
            CLRREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_54 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
	}

	else if((gic_spi_irq >= GIC_SPI_MS_FIQ_START) && (gic_spi_irq < GIC_SPI_MS_FIQ_END))
	{
		/*MS_FIQ0*/
		ms_fiq = gic_spi_irq - GIC_SPI_MS_FIQ_START;
		CLRREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_44 + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );
	}

	/*MS_FIQ 64!!!!*/
	else if (gic_spi_irq == INT_FIQ_64_SEC_GUARD_INT)
	{
		ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ1_START;
		CLRREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_44 + 0 ) , (1 << (0)) );
	}
	/*MS_FIQ 65!!!!*/
	else if (gic_spi_irq == INT_FIQ_65_SD_CDZ_IN)
	{
		ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ1_START;
		CLRREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_44 + 0 ) , (1 << (1)) );
	}
	/*MS_IRQ 1*/
	else if ((gic_spi_irq >= GIC_SPI_MS_IRQ1_START) && (gic_spi_irq < INT_FIQ_64_SEC_GUARD_INT))
	{
		ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ1_START;
		CLRREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_54 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
	}

	else if ((gic_spi_irq > INT_FIQ_65_SD_CDZ_IN) && (gic_spi_irq < GIC_SPI_MS_IRQ1_END))
	{
		ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ1_START - 2;
		CLRREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_54 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
	}

	/*MS_FIQ 1*/

	else if((gic_spi_irq >= GIC_SPI_MS_FIQ1_START) && (gic_spi_irq < GIC_SPI_MS_FIQ1_END))
	{
		/*MS_FIQ1*/
		ms_fiq = gic_spi_irq - GIC_SPI_MS_FIQ1_START;
		CLRREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_44 + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );
	}
	else if((gic_spi_irq == INT_169_REG_DUMMY_ECO_1_1) )
	{
	      /* sw interrupt, riu bank 0x101d, offset 0x2d[1] */
		CLRREG16( (BASE_REG_MCU_ARM + REG_ID_2D + 0) , (1 << (1)) );
	}
	else if((gic_spi_irq == INT_168_REG_DUMMY_ECO_1_0) )
	{
	      /* sw interrupt, riu bank 0x101d, offset 0x2d[0] */
		CLRREG16( (BASE_REG_MCU_ARM + REG_ID_2D + 0) , (1 << (0)) );
	}
	else if(((INT_162_IRQ_IN_0 <= gic_spi_irq) && (gic_spi_irq <= INT_165_FIQ_IN_1)) )
	{
		OUTREG32(GIC_PHYS + 0x1000  + GIC_DIST_ENABLE_SET + (d->hwirq / 32) * 4, (1<<(d->hwirq % 32)));
 	}
	else if((gic_spi_irq == INT_182_PMU_IRQ_0) ||(gic_spi_irq == INT_191_PMU_IRQ_1))
	{
	      /* PMU */
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
	s16 gic_spi_irq;

    gic_spi_irq = data->hwirq;

    if( (flow_type&IRQ_TYPE_EDGE_BOTH)==IRQ_TYPE_EDGE_BOTH)
    {
        pr_err("could not support IRQ_TYPE_EDGE_BOTH mode 0x%x\n",flow_type);
        return 0;
    }

    if( data->hwirq < GIC_HWIRQ_MS_START )
    {
        return 0;
    }

    else if((gic_spi_irq >= GIC_SPI_MS_IRQ_START) && (gic_spi_irq < GIC_SPI_MS_IRQ_END))
    {
        /*MS_IRQ0*/
        ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ_START;

        if (flow_type&IRQ_TYPE_LEVEL_LOW)
            SETREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_58 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
        else
            CLRREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_58 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
    }

    else if((gic_spi_irq >= GIC_SPI_MS_FIQ_START) && (gic_spi_irq < GIC_SPI_MS_FIQ_END))
    {
            /*MS_FIQ0*/
            ms_fiq = gic_spi_irq - GIC_SPI_MS_FIQ_START;

        if (flow_type&IRQ_TYPE_EDGE_FALLING)
            SETREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_48 + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );
        else
            CLRREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_48 + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );
    }

    /*MS_FIQ 64!!!!*/
    else if (gic_spi_irq == INT_FIQ_64_SEC_GUARD_INT)
    {
        ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ1_START;

        if (flow_type&IRQ_TYPE_EDGE_FALLING)
            SETREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_48 + 0 ) , (1 << (0)) );
        else
            CLRREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_48 + 0 ) , (1 << (0)) );

    }

    /*MS_FIQ 65!!!!*/
    else if (gic_spi_irq == INT_FIQ_65_SD_CDZ_IN)
    {
        ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ1_START;

        if (flow_type&IRQ_TYPE_EDGE_FALLING)
            SETREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_48 + 0 ) , (1 << (1)) );
        else
            CLRREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_48 + 0 ) , (1 << (1)) );
    }

    else if((gic_spi_irq >= GIC_SPI_MS_IRQ1_START) && (gic_spi_irq < INT_FIQ_64_SEC_GUARD_INT))
    {
        /*MS_IRQ1*/
        ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ1_START;

        if (flow_type&IRQ_TYPE_LEVEL_LOW)
            SETREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_58 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
        else
            CLRREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_58 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
    }

    else if ((gic_spi_irq > INT_FIQ_65_SD_CDZ_IN) && (gic_spi_irq < GIC_SPI_MS_IRQ1_END))
    {
        ms_irq = gic_spi_irq - GIC_SPI_MS_IRQ1_START - 2;

        if (flow_type&IRQ_TYPE_LEVEL_LOW)
            SETREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_58 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );
        else
            CLRREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_58 + (ms_irq/16)*4 ) , (1 << (ms_irq%16)) );;
    }

    else if((gic_spi_irq >= GIC_SPI_MS_FIQ1_START) && (gic_spi_irq < GIC_SPI_MS_FIQ1_END))
    {
        /*MS_FIQ1*/
        ms_fiq = gic_spi_irq - GIC_SPI_MS_FIQ1_START;

        if (flow_type&IRQ_TYPE_EDGE_FALLING)
            SETREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_48 + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );
        else
        CLRREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_48 + (ms_fiq/16)*4 ) , (1 << (ms_fiq%16)) );
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
    printk("%s TBD\r\n", __FUNCTION__);
    //Patch for disable bypass IRQ/FIQ
    {
//        u32 bypass = 0;
//        bypass = INREG32(GIC_PHYS + 0x2000 + GIC_CPU_CTRL);
//        bypass |= GICC_DIS_BYPASS_MASK;
//        OUTREG32(GIC_PHYS + 0x2000 + GIC_CPU_CTRL, bypass | GICC_ENABLE);
    }
}

struct syscore_ops ms_irq_syscore_ops = {
    .suspend = ms_irqchip_suspend,
    .resume = ms_irqchip_resume,
};
extern CHIP_VERSION msys_get_chipVersion(void);

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
        bypass = INREG32(GIC_PHYS + 0x0100 + GIC_CPU_CTRL);
        bypass |= GICC_DIS_BYPASS_MASK;
        OUTREG32(GIC_PHYS + 0x0100 + GIC_CPU_CTRL, bypass | GICC_ENABLE);
    }
    domain = irq_domain_add_hierarchy(parent_domain, 0,
                    GIC_SPI_ARM_INTERNAL_NR+GIC_SPI_MS_IRQ_NR+GIC_SPI_MS_FIQ_NR+
                    GIC_SPI_MS_FIQ1_NR+GIC_SPI_MS_IRQ1_NR+GIC_SPI_MS_EXT_NR,
                    np, &ms_main_intc_domain_ops, NULL);
 
    if (!domain)
    {
        pr_err("%s: %s allocat domain fail\n", __func__, np->name);
        return -ENOMEM;
    }

    /* U02 eco patch, mask FIQ interrupt*/
    if(msys_get_chipVersion())
        OUTREGMSK16((BASE_REG_MCU_ARM+REG_ID_52), 0xFF, 0xFF);

    register_syscore_ops(&ms_irq_syscore_ops);

    return 0;
}

IRQCHIP_DECLARE(ms_main_intc, "sstar,main-intc", ms_init_main_intc);
