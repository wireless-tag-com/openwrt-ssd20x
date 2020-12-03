/*
* sram.S- Sigmastar
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
/*------------------------------------------------------------------------------
    Function Code
-------------------------------------------------------------------------------*/
#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/asm-offsets.h>
#include <asm/hardware/cache-l2x0.h>

    .align 3
.globl sram_suspend_imi
.globl v7_cpu_resume

ENTRY(sram_suspend_imi)
    //Below flow is run at SRAM(IMI)

    // 1. DDR enter self-refresh
    //wriu -w   0x101246,0xFFFE
    //wriu -w   0x101266,0xFFFF
    //wriu -w   0x101286,0xFFFF
    //wriu -w   0x1012A6,0xFFFF
    //wriu -w   0x101106,0xFFFF
    //wriu -w   0x101126,0xFFFF

    ldr   r1, =0xFD000000
    ldr   r3, =0x101200
    ldr   r4, =0x101100
    ldr   r5, =0x101000
    add   r2, r1, r3, lsl #1
    ldr   r0, =0xFFFE
    str   r0, [r2, #0x46 << 1]
    ldr   r0, =0xFFFF
    str   r0, [r2, #0x66 << 1]
    str   r0, [r2, #0x86 << 1]
    str   r0, [r2, #0xA6 << 1]
    add   r2, r1, r4, lsl #1
    str   r0, [r2, #0x06 << 1]
    str   r0, [r2, #0x26 << 1]

	//delay
    ldr   r0,=0x10000
1:
    subs  r0, #1
    bne   1b

    //wriu -w   0x101218,0x0400 //reg_mrx
    //wriu -w   0x101200,0x002f //Bit[05]reg_auto_ref_off
    //wriu -w   0x101200,0x052e //trig precharge all
    //wriu -w   0x101200,0x002e
    //wriu -w   0x101200,0x032e
    //wriu -w   0x101200,0x002e
    //wriu -w   0x101206,0x1430
    
    //wriu -w   0x101246,0xFFFF
    //wriu -w   0x101200,0x202e

    add   r2, r1, r3, lsl #1
    ldr   r0, =0x0400
    str   r0, [r2, #0x18 << 1]
    ldr   r0, =0x002F
    str   r0, [r2, #0x00 << 1]
    ldr   r0, =0x052E
    str   r0, [r2, #0x00 << 1]
    ldr   r0, =0x002E
    str   r0, [r2, #0x00 << 1]
    ldr   r0, =0x032E
    str   r0, [r2, #0x00 << 1]
    ldr   r0, =0x002E
    str   r0, [r2, #0x00 << 1]
    ldr   r0, =0x1430
    str   r0, [r2, #0x06 << 1]

	//delay
    ldr   r0,=0x10000
1:
    subs  r0, #1
    bne   1b

    ldr   r0, =0xFFFF
    str   r0, [r2, #0x46 << 1]
    ldr   r0, =0x202E
    str   r0, [r2, #0x00 << 1]

	//delay
    ldr   r0,=0x10000
1:
    subs  r0, #1
    bne   1b

    // 2. AN power down
    //wriu -b 0x101203 0xF0 0xF0
    //wriu -b 0x101000 0x18 0x18
    //wriu -w 0x101054 0xc070
    //wriu -w 0x101008 0x0000

    ldrb  r0, [r2, #0x05]
    orr   r0, r0, #0xF0
    strb  r0, [r2, #0x05]
    add   r2, r1, r5, lsl #1
    ldrb  r0, [r2, #0x00 << 1]
    orr   r0, r0, #0x18
    strb  r0, [r2, #0x00 << 1]
    ldr   r0, =0xC070
    str   r0, [r2, #0x54 << 1]
    ldr   r0, =0x0000
    str   r0, [r2, #0x08 << 1]

	//delay
    ldr   r0,=0x10000
1:
    subs  r0, #1
    bne   1b

    // 3. Switch reg_ckg_mcu/reg_ckg_spi to xtal, or it will hang when resume
    ldr   r3, =0x000E00
    add   r2, r1, r3, lsl #1
    ldr   r0, [r2, #0x40 << 1]
    bic   r0, r0, #0x1 << 7
    bic   r0, r0, #0x1 << 14
    str   r0, [r2, #0x40 << 1]

    // 4. Set wake up source(SAR, WOL, RTC)
    ldr   r3, =0x000E00
    add   r2, r1, r3, lsl #1
    ldr   r0, [r2, #0x10 << 1]
    bic   r0, r0, #0x16        //[1]:SAR, [2]:WOL, [4]:RTC
    str   r0, [r2, #0x10 << 1]

    // 5. PM enter sleep
    //wriu    0x0000170e 0x03
    //wriu -w 0x00001710 0x007f
    //wriu    0x00003c24 0x30
    ldr   r3, =0x000E00
    ldr   r4, =0x000F00
    ldr   r5, =0x001700
    ldr   r6, =0x003C00
    add   r2, r1, r5, lsl #1
    ldr   r0, =0x03
    strb  r0, [r2, #0x0E << 1]
    ldr   r0, =0x007F
    str   r0, [r2, #0x10 << 1]
    add   r2, r1, r6, lsl #1
    ldr   r0, =0x30
    strb  r0, [r2, #0x24 << 1]

    //wriu    0x00000e38 0x0c
    //wriu -w 0x00000e24 0xbabe
    //wriu    0x00000e6e 0xa5
    add   r2, r1, r3, lsl #1
    ldr   r0, =0x0c
    strb  r0, [r2, #0x38 << 1]
    ldr   r0, =0xBABE
    str   r0, [r2, #0x24 << 1]
    //ldr   r0, =0xA5
    //strb  r0, [r2, #0x6E << 1]
    nop
    nop
    nop
    nop
    //wriu    0x00000f08 0x10
    add   r2, r1, r4, lsl #1
    ldr   r0, =0x10
    strb  r0, [r2, #0x08 << 1]
    nop
    nop
    nop
    nop






ENDPROC(sram_suspend_imi)
.ltorg