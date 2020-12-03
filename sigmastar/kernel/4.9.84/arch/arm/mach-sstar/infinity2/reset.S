/*
* reset.S- Sigmastar
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

	.align	5
.globl cedric_reset_cpu
cedric_reset_cpu:

    mov   r3, #0x0000
    ldr   r1, =0xFD005C80
    str   r3, [r1]
    mov   r3, #0x0079
	ldr	  r1, =0xFD005CB8
	str   r3, [r1]
	nop
	nop
	nop
	nop

_loop_forever:
	b	_loop_forever
