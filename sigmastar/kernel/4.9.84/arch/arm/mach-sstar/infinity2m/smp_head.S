/*
* smp_head.S- Sigmastar
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
#include <linux/linkage.h>
#include <linux/init.h>

/*	__CPUINIT */

#define ACTLR_SMP_ENABLE            0x0040
#define ACTLR_DODMBS_DISABLE        0x0400
#define ACTLR_L2RADIS_DISABLE       0x0800
#define ACTLR_L1RADIS_DISABLE       0x1000
#define ACTLR_L1PCTL_3_PREFETCH     0x6000
#define ACTLR_DDVM_DISABLE          0x8000

#define NSACR_CP11                  0x0800
#define NSACR_CP10                  0x0400

#define CPACR_CP11_FULL             0x00C00000
#define CPACR_CP10_FULL             0x00300000

.extern infinity2m_secondary_gic

/*
 * Realview/Versatile Express specific entry point for secondary CPUs.
 * This provides a "holding pen" into which all secondary cores are held
 * until we're ready for them to initialise.
 */
ENTRY(infinity2m_secondary_startup)
#ifdef CONFIG_LH_RTOS
    ldr 	r0, =0xA0001000
    mov 	sp, r0

    bl secure_cntvoff_init

    bl infinity2m_secondary_gic

    //ACTLR
    mrc	p15, 0, r0, c1, c0, 1
    orr	r0, r0, #(ACTLR_DDVM_DISABLE)
    orr	r0, r0, #(ACTLR_L1PCTL_3_PREFETCH)
    orr	r0, r0, #(ACTLR_L1RADIS_DISABLE)
    orr	r0, r0, #(ACTLR_L2RADIS_DISABLE)
    orr	r0, r0, #(ACTLR_DODMBS_DISABLE)
    orr	r0, r0, #(ACTLR_SMP_ENABLE)
    mcr	p15, 0, r0, c1, c0, 1
    isb

    //NSACR
    mrc p15, 0, r0, c1, c1, 2
    orr r0, r0, #(NSACR_CP11)
    orr r0, r0, #(NSACR_CP10)
    mcr p15, 0, r0, c1, c1, 2
    isb

    //switch to NS
    mrc	p15, 0, r1, c1, c1, 0
    orr	r0, r1, #1
    mcr	p15, 0, r0, c1, c1, 0
    isb

    //CPACR
    mrc p15, 0, r0, c1, c0, 2
    orr r0, r0, #(CPACR_CP11_FULL)
    orr r0, r0, #(CPACR_CP10_FULL)
    mcr p15, 0, r0, c1, c0, 2
    isb
#endif

    /* Native ARMv7 L1 invalide function */
    bl v7_invalidate_l1
	mrc	p15, 0, r0, c0, c0, 5
	and	r0, r0, #15
	adr	r4, 1f
	ldmia	r4, {r5, r6}
	sub	r4, r4, r5
	add	r6, r6, r4
pen:	ldr	r7, [r6]
	cmp	r7, r0
	bne	pen

	/*
	 * we've been released from the holding pen: secondary_stack
	 * should now contain the SVC stack for this core
	 */
	b	secondary_startup


	.align 2
1:	.long	.
	.long	pen_release
ENDPROC(infinity2m_secondary_startup)
