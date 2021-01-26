/*
* reset.S- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: joe.su <joe.su@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/
/*------------------------------------------------------------------------------
    Function Code
-------------------------------------------------------------------------------*/

	.align	5
.globl reset_cpu
reset_cpu:

    ldr   r1, =0x1F001CB8
    mov   r3, #0x00FF
    str   r3, [r1]
    mov   r3, #0x0079
	str   r3, [r1]
	nop
	nop
	nop
	nop

_loop_forever:
	b	_loop_forever
