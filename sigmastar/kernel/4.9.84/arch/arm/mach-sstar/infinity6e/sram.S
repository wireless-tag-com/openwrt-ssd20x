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

    // 04. LPDDR2 enter self-refresh
    //wriu -w   0x1012e0,0x0000
    //wriu -w   0x101246,0xFFFE
    //wriu -w   0x101266,0xFFFF
    //wriu -w   0x101286,0xFFFF
    //wriu -w   0x1012A6,0xFFFF
    //wriu -w   0x1011E6,0x00FE
    //wait 20

    ldr   r1, =0xFD000000
    ldr   r3, =0x101200
    ldr   r4, =0x101100
    ldr   r5, =0x101000
    add   r2, r1, r3, lsl #1
    ldr   r0, =0x0000
    str   r0, [r2, #0xE0 << 1]
    ldr   r0, =0xFFFE
    str   r0, [r2, #0x46 << 1]
    ldr   r0, =0xFFFF
    str   r0, [r2, #0x66 << 1]
    str   r0, [r2, #0x86 << 1]
    str   r0, [r2, #0xA6 << 1]
    add   r2, r1, r4, lsl #1
    ldr   r0, =0x00FE
    str   r0, [r2, #0xE6 << 1]
    //delay
    ldr   r0,=0x10000
1:
    subs  r0, #1
    bne   1b

    //Pre-charge all then one refresh
    //wriu -w   0x101218  0x0400
    add   r2, r1, r3, lsl #1
    ldr   r0, =0x0400
    str   r0, [r2, #0x18 << 1]
    //wriu -b   0x101200  0x30 0x20
    ldrb  r0, [r2, #0x00]
    and   r0, r0, #0xCF
    orr   r0, r0, #0x20
    strb  r0, [r2, #0x00]
    //wriu -b   0x101201  0x1e 0x04
    ldrb  r0, [r2, #0x01]
    and   r0, r0, #0xE1
    orr   r0, r0, #0x04
    strb  r0, [r2, #0x01]
    //wriu -b   0x101201  0x01 0x00
    ldrb  r0, [r2, #0x01]
    and   r0, r0, #0xFE
    strb  r0, [r2, #0x01]
    //wriu -b   0x101201  0x01 0x01
    ldrb  r0, [r2, #0x01]
    orr   r0, r0, #0x01
    strb  r0, [r2, #0x01]
    //wriu -b   0x101201  0x01 0x00
    ldrb  r0, [r2, #0x01]
    and   r0, r0, #0xFE
    strb  r0, [r2, #0x01]
    //wriu -b   0x101201  0x1e 0x02
    ldrb  r0, [r2, #0x01]
    and   r0, r0, #0xE1
    orr   r0, r0, #0x02
    strb  r0, [r2, #0x01]
    //wriu -b   0x101201  0x01 0x00
    ldrb  r0, [r2, #0x01]
    and   r0, r0, #0xFE
    strb  r0, [r2, #0x01]
    //wriu -b   0x101201  0x01 0x01
    ldrb  r0, [r2, #0x01]
    orr   r0, r0, #0x01
    strb  r0, [r2, #0x01]
    //wriu -b   0x101201  0x01 0x00
    ldrb  r0, [r2, #0x01]
    and   r0, r0, #0xFE
    strb  r0, [r2, #0x01]
    //wriu -w   0x101200  0x202e
    ldr   r0, =0x202e
    str   r0, [r2, #0x00 << 1]
    //wait 10
    //delay
    ldr   r0,=0x10000
1:
    subs  r0, #1
    bne   1b

    // 05. AN power down
    //wriu -b 0x101203 0xF0 0xF0
    ldrb  r0, [r2, #0x05]
    orr   r0, r0, #0xF0
    strb  r0, [r2, #0x05]
    //wriu -b 0x101000 0x08 0x08
    //wriu -b 0x101000 0x10 0x10
    add   r2, r1, r5, lsl #1
    ldrb  r0, [r2, #0x00 << 1]
    orr   r0, r0, #0x18
    strb  r0, [r2, #0x00 << 1]
    //wriu -b 0x101054 0x70 0x70
    ldrb  r0, [r2, #0xA8]
    orr   r0, r0, #0x70
    strb  r0, [r2, #0xA8]
    //wriu -b 0x101008 0x3F 0x00
    ldrb  r0, [r2, #0x10]
    and   r0, r0, #0xC0
    strb  r0, [r2, #0x10]

     // 06. power down
    //wriu -w   0x00003408 0x01
    //wriu -w   0x00003400 0x20
    // Turn-on ISO flow
    //wriu -w   0x00003406 0x01
    //wait 1
    //wriu -w   0x00003406 0x03
    //wait 1
    //wriu -w   0x00003406 0x07
    //wait 1
    //wriu -w   0x00003406 0x05
    //wait 1
    //wriu -w   0x00003406 0x01
    //wait 1
    //wriu -w   0x00003406 0x00
    //wait 1
    //wriu -w   0x00003400 0x00
    //wriu -w   0x0000341E 0x00
    // Turn-on ISO flow
    //wriu -w   0x00003406 0x01
    //wait 1
    //wriu -w   0x00003406 0x03
    //wait 1
    //wriu -w   0x00003406 0x07
    //wait 1
    //wriu -w   0x00003406 0x05
    //wait 1
    //wriu -w   0x00003406 0x01
    //wait 1
    //wriu -w   0x00003406 0x00
    //wait 1

    ldr   r1, =0xFD000000
    ldr   r3, =0x003400
    add   r2, r1, r3, lsl #1

    // power down
    ldr   r0, =0x0000
    str   r0, [r2, #0x00 << 1]
    ldr   r0, =0x0000
    str   r0, [r2, #0x1E << 1]

    ldr   r0, =0x0001
    str   r0, [r2, #0x06 << 1]
    //delay
    ldr   r0,=0x11000
1:
    subs  r0, #1
    bne   1b

    ldr   r0, =0x0003
    str   r0, [r2, #0x06 << 1]
    //delay
    ldr   r0,=0x11000
1:
    subs  r0, #1
    bne   1b

    ldr   r0, =0x0007
    str   r0, [r2, #0x06 << 1]
    //delay
    ldr   r0,=0x11000
1:
    subs  r0, #1
    bne   1b

    ldr   r0, =0x0005
    str   r0, [r2, #0x06 << 1]
    //delay
    ldr   r0,=0x11000
1:
    subs  r0, #1
    bne   1b

    ldr   r0, =0x0001
    str   r0, [r2, #0x06 << 1]
    //delay
    ldr   r0,=0x11000
1:
    subs  r0, #1
    bne   1b

    ldr   r0, =0x0000
    str   r0, [r2, #0x06 << 1]
    //delay
    ldr   r0,=0x11000
1:
    subs  r0, #1
    bne   1b
    nop
    nop
    nop
    nop


ENDPROC(sram_suspend_imi)
.ltorg