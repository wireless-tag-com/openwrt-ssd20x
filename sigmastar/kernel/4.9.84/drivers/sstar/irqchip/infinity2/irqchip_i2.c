/*
* irqchip_i2.c- Sigmastar
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
#include <linux/irqchip/arm-gic.h>
#include <linux/irqchip.h>
#include <linux/io.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/irqdomain.h>
#include <linux/of.h>
#include <linux/syscore_ops.h>
#include <asm/mach/irq.h>
#include <irqchip.h>
#include "infinity2/irqs.h"
#include "infinity2/registers.h"
#include "_ms_private.h"
#include "ms_platform.h"
#include "ms_types.h"
#include "infinity2/gpio.h"
#include <ms_msys.h>
static DEFINE_SPINLOCK(infinity_irq_controller_lock);





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



#if 0
static void ms_pm_sleep_irq_ack(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;

    if(pmsleep_fiq < PM_GPIO_INT_END)
        SETREG16(BASE_REG_PMGPIO_PA + (pmsleep_fiq << 2), BIT6);
    else if(pmsleep_fiq >= PMSLEEP_IRQ_START)
    {

    }
}

static void ms_pm_sleep_irq_mask(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;

     if(pmsleep_fiq < PM_GPIO_INT_END)
        SETREG16(BASE_REG_PMGPIO_PA + (pmsleep_fiq << 2), BIT4);

    else if(pmsleep_fiq >= PMSLEEP_IRQ_START)
    {
        SETREG8(BASE_REG_PMSLEEP_PA + REG_ID_08, 1<<(pmsleep_fiq-PMSLEEP_IRQ_START) );
    }

}

static void ms_pm_sleep_irq_unmask(struct irq_data *d)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;

     if(pmsleep_fiq < PM_GPIO_INT_END)
        CLRREG16(BASE_REG_PMGPIO_PA + (pmsleep_fiq << 2), BIT4);

    else if(pmsleep_fiq >= PMSLEEP_IRQ_START)
    {
        CLRREG16(BASE_REG_PMSLEEP_PA + REG_ID_08, 1<<(pmsleep_fiq-PMSLEEP_IRQ_START) );
    }

}

static int ms_pm_sleep_irq_set_type(struct irq_data *d, unsigned int type)
{
    U16 pmsleep_fiq;

    pmsleep_fiq = d->hwirq;

    if(pmsleep_fiq < PM_GPIO_INT_END)
    {
        if(type&IRQ_TYPE_EDGE_FALLING)
            SETREG16(BASE_REG_PMGPIO_PA + (pmsleep_fiq << 2), BIT7);
        else
            CLRREG16(BASE_REG_PMGPIO_PA + (pmsleep_fiq << 2), BIT7);
    }

    else if(pmsleep_fiq < PMSLEEP_IRQ_END)
    {
        if(type&IRQ_TYPE_LEVEL_LOW)
            SETREG8(BASE_REG_PMSLEEP_PA + REG_ID_09, 1<<(pmsleep_fiq-PMSLEEP_IRQ_START) );
        else
            CLRREG8(BASE_REG_PMSLEEP_PA + REG_ID_09, 1<<(pmsleep_fiq-PMSLEEP_IRQ_START) );
    }
    return 0;
}
#endif



static void ms_eoi_irq(struct irq_data *d)
{
    s16 gic_spi_irq, ms_fiq, ms_fiq1;

    gic_spi_irq = d->hwirq - GIC_SGI_NR - GIC_PPI_NR;

    /* NOTE: only clear if it is FIQ */
    if( d->hwirq < GIC_HWIRQ_MS_START )
    {
        return;
    }
    else if( gic_spi_irq >= GIC_SPI_MS_FIQ_START && gic_spi_irq < GIC_SPI_MS_FIQ_END )
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
}

static void ms_mask_irq(struct irq_data *d)
{
    s16 ms_irq;
    s16 ms_fiq;
    s16 gic_spi_irq;

    gic_spi_irq = d->hwirq - GIC_SGI_NR - GIC_PPI_NR;
//    pr_err("[@@-ms_irq_mask] gic_spi_irq %d\n", gic_spi_irq);

    if( d->hwirq < GIC_HWIRQ_MS_START )
    {
        return;
    }
    else if((gic_spi_irq >= GIC_SPI_MS_IRQ_START) && (gic_spi_irq < GIC_SPI_MS_IRQ_END))
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
    else
    {
        pr_err("[ms_irq_mask] Unknown hwirq %lu from GIC\n", d->hwirq);
    }
}

static void ms_unmask_irq(struct irq_data *d)
{
    s16 ms_irq;
    s16 ms_fiq;
    s16 gic_spi_irq;

    gic_spi_irq = d->hwirq - GIC_SGI_NR - GIC_PPI_NR;
//	if ((gic_spi_irq != INT_IRQ_00_INT_UART0) || (gic_spi_irq != INT_FIQ_00_INT_TIMER0))
//		printk("ms_unmask_irq: %d/n", gic_spi_irq);

    if( d->hwirq < GIC_HWIRQ_MS_START )
    {
        return;
    }
    else if((gic_spi_irq >= GIC_SPI_MS_IRQ_START) && (gic_spi_irq < GIC_SPI_MS_IRQ_END))
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
        pr_err("[ms_unmask_irq] Unknown hwirq %lu from GIC\n", d->hwirq);
    }

}

void ms_mask_irq_all(void)
{
    /* mask FIQ0 0 - 63 */
    OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_44) , 0xFFFF );
    OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_45) , 0xFFFF );
    OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_46) , 0xFFFF );
    OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_47) , 0xFFFF );

    /* mask IRQ0 0 - 63 */
    OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_54) , 0xFFFF );
    OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_55) , 0xFFFF );
    OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_56) , 0xFFFF );
    OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_57) , 0xFFFF );
	/* mask FIQ1 0 - 63 */
    OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_44) , 0xFFFF );
    OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_45) , 0xFFFF );
    OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_46) , 0xFFFF );
    OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_47) , 0xFFFF );

    /* mask IRQ1 0 - 63 */
    OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_54) , 0xFFFF );
    OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_55) , 0xFFFF );
    OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_56) , 0xFFFF );
    OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_57) , 0xFFFF );
}

void ms_unmask_irq_all(void)
{
	/* unmask FIQ 0 - 63 */
       OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_44) , 0 );
       OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_45) , 0 );
	OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_46) , 0 );
       OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_47) , 0 );
	/* unmask IRQ 0 - 63 */
	OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_54) , 0 );
	OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_55) , 0 );
	OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_56) , 0 );
	OUTREG16( (BASE_REG_INTRCTL_PA0 + REG_ID_57) , 0 );

	/* unmask IRQ 1 - 63 */
	OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_44) , 0 );
	OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_45) , 0 );
	OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_46) , 0 );
	OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_47) , 0 );
	 /* unmask IRQ 1 - 63 */
	OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_54) , 0 );
	OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_55) , 0 );
	OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_56) , 0 );
	OUTREG16( (BASE_REG_INTRCTL_PA1 + REG_ID_57) , 0 );
}

static int  ms_set_type_irq(struct irq_data *data, unsigned int flow_type)
{
	s16 ms_irq;
	s16 ms_fiq;
	s16 gic_spi_irq;

    gic_spi_irq = data->hwirq - GIC_SGI_NR - GIC_PPI_NR;

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
        pr_err("[ms_irq_set_polarity] Unknown hwirq %lu from GIC\n", data->hwirq);
    }

    return 0;
}

#if 1
static void ms_handle_cascade_pm_irq(unsigned int irq, struct irq_desc *desc)
{
    unsigned int cascade_irq = 0xFFFFFFFF, i;
    struct irq_chip *chip = irq_get_chip(irq);
//    unsigned int fiq_final_status = INREG16(BASE_REG_PMSLEEP_PA + REG_ID_04) | (INREG16(BASE_REG_PMSLEEP_PA + REG_ID_05)<<16);
    unsigned int irq_final_status = INREG16(BASE_REG_PMSLEEP_PA + REG_ID_0E)& 0xFF;
//    unsigned int irq_gpio_status = INREG16(BASE_REG_PMGPIO_PA)& 0xFF;

    chained_irq_enter(chip, desc);

    spin_lock(&infinity_irq_controller_lock);
    {
        for(i=0;i<PMSLEEP_IRQ_NR;i++)
        {
            if(0 !=(irq_final_status & (1<<i)))
            {
                cascade_irq = i + (GIC_HWIRQ_MS_START+GIC_SPI_MS_IRQ1_END+PM_GPIO_INT_NR);
                break;
            }
        }
        if(cascade_irq == 0xFFFFFFFF)
        {
            for(i=0;i<0x50;i++)
            {
                if(0 != ((INREG16(BASE_REG_PMGPIO_PA+(i<<2))) & BIT8))
                {
                    cascade_irq = i + (GIC_HWIRQ_MS_START+GIC_SPI_MS_IRQ1_END);
                }
            }
        }
    }
    spin_unlock(&infinity_irq_controller_lock);

    if(0xFFFFFFFF==cascade_irq)
        BUG();
    generic_handle_irq(cascade_irq);

    chained_irq_exit(chip, desc);
}
#endif


struct irq_chip ms_pm_sleep_irqchip = {
    .name = "PMSLEEP",
    .irq_ack = ms_pm_sleep_irq_ack,
    .irq_mask = ms_pm_sleep_irq_mask,
    .irq_unmask = ms_pm_sleep_irq_unmask,
    .irq_set_type = ms_pm_sleep_irq_set_type,
};
EXPORT_SYMBOL(ms_pm_sleep_irqchip);

static int pmsleep_irq_domain_map(struct irq_domain *d, unsigned int irq, irq_hw_number_t hw)
{
    irq_set_chip_and_handler(irq, &ms_pm_sleep_irqchip, handle_edge_irq);
    set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
    return 0;
}

struct irq_domain_ops ms_irq_domain_ops = {
    .map = pmsleep_irq_domain_map,
    .xlate = irq_domain_xlate_onecell,
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

static int __init infinity_init_irqchip(struct device_node *np, struct device_node *interrupt_parent)
{
    int hwirq_base = 0;
    int virq_base;
    struct irq_domain *infinity_irq_domain;
    //hook MSTAR interrupt-controller function
    gic_arch_extn.irq_ack = ms_eoi_irq;
    gic_arch_extn.irq_eoi = ms_eoi_irq;
    gic_arch_extn.irq_mask = ms_mask_irq;
    gic_arch_extn.irq_unmask = ms_unmask_irq;
    gic_arch_extn.irq_set_type= ms_set_type_irq;

    //Patch for disable bypass IRQ/FIQ
    {
        u32 bypass = 0;
        bypass = INREG32(GIC_PHYS + 0x0100 + GIC_CPU_CTRL);
        bypass |= GICC_DIS_BYPASS_MASK;
        OUTREG32(GIC_PHYS + 0x0100 + GIC_CPU_CTRL, bypass | GICC_ENABLE);
    }

    /* virq_base value is 256 */
    virq_base = irq_alloc_descs(-1, 0, PMSLEEP_IRQ_END, 0); //79 is pm_gpio
    if(virq_base < 0 )
    {
        pr_warn("infinity_init_irqchip: Couldn't allocate IRQ numbers\n");
        virq_base = 0;
    }

    infinity_irq_domain = irq_domain_add_legacy(np, PMSLEEP_IRQ_END, virq_base, hwirq_base, &ms_irq_domain_ops, NULL);

    if(!infinity_irq_domain)
    {
        pr_err("infinity_init_irqchip: Failedto create irqdomain\n");
    }

    /* unmask pm domain IRQ ctrl */
    CLRREG16( (BASE_REG_IRQ_PM + REG_ID_14) , (1 << 02) );//pm_sleep
    /* unmask pm domain FIQ ctrl */
    CLRREG16( (BASE_REG_IRQ_PM + REG_ID_04) , (1 << 12) );//timer2

    /* U02 eco patch, mask FIQ interrupt*/
    if(msys_get_chipVersion())
        OUTREGMSK16((BASE_REG_MCU_ARM+REG_ID_52), 0xFF, 0xFF);

    irq_set_chained_handler(GIC_SGI_NR+GIC_PPI_NR+INT_IRQ_63_IRQ_FRM_PM, ms_handle_cascade_pm_irq); 


/*
    for(irq=virq_base;irq<virq_base+PMSLEEP_FIQ_END;irq++)
    {
        irq_set_chip_and_handler(irq, &ms_pm_sleep_irqchip, handle_level_irq);
        set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
    }
*/
    register_syscore_ops(&ms_irq_syscore_ops);

    return 0;
}
IRQCHIP_DECLARE(infinity2, "mstar,intrctl-infinity2", infinity_init_irqchip);
