/*
* cache.S- Sigmastar
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
//*
//* $Id: //DAILEO/Columbus/IPCamera/source/iNfinity/iNfinity_ROM/source/boot.S#3 $
//* $Header: //DAILEO/Columbus/IPCamera/source/iNfinity/iNfinity_ROM/source/boot.S#3 $
//* $Date: 2015/06/16 $
//* $DateTime: 2015/06/16 15:12:51 $
//* $Change: 1258698 $
//* $File: //DAILEO/Columbus/IPCamera/source/iNfinity/iNfinity_ROM/source/boot.S $
//* $Revision: #3 $
//*

//#include "include/platform.h"
//#include "include/arm.include"
//#include "include/macro.include"
//#include "drv_arm.inc"

.text
.arm

////// CODE

cpu_start_init_cache: .global     cpu_start_init_cache

			mov 	r0, #0
			mcr	  p15, 0, r0, c8, c7, 0
			// Invalidate Inst TLB and Data TLB
			mcr 	p15, 0, r0, c7, c5, 0
			// Invalidate I Cache
			// Invalidate D-cache by set/way
			// Note: for Cortex-A9, there is no instruction for invalidating
			// the whole D-cache. Need to invalidate line by line.
			// Read cache size from the Cache Size Identification Register
			mrc p15, 1, r3, c0, c0, 0			// Read current Cache Size Identification Register
			mov r1, #0x200
			sub r1, r1, #1

			and r3, r1, r3, LSR #13 			// r3 = (number of sets - 1)
			mov r0, #0							// r0 -> way counter
	150:
			mov r1, #0							// r1 -> set counter
	152:
			mov r2, r0, LSL #30 			
			orr r2, r2, r1, LSL #5					// r2 -> set/way cache-op format
			mcr p15, 0, r2, c7, c6, 2			// Invalidate line described by r2
			add r1, r1, #1						// Increment set counter
			cmp r1, r3							// Check if the last set is reached...
			ble 152b							// ...if not, continue the set_loop...
			add r0, r0, #1						// ...else, Increment way counter
			cmp r0, #4							// Check if the last way is reached...
			blt 150b							// ...if not, continue the way_loop
				@//Enable dcaches	 @//D-cache is controlled by bit 2
				mrc		  p15, 0, r0, c1, c0, 0 	  @//read CP15 register 1
				orr		 r0, r0, #(0x1 <<2) 		 @//enable D Cache
				mcr		  p15, 0, r0, c1, c0, 0 	  @//write CP15 register 1

			// Enable Program Flow Prediction
			// Branch prediction is controlled by the System Control Register:
			// Set Bit 11 to enable branch prediciton and return stack
			// Turning on branch prediction requires a general enable
			// CP15, c1. Control Register
			// Bit 11 [Z] bit Program flow prediction:
			// 0 = Program flow prediction disabled
			// 1 = Program flow prediction enabled.
			mrc p15, 0, r0, c1, c0, 0			// Read System Control Register
			orr r0, r0, #(0x1 <<11)
			mcr p15, 0, r0, c1, c0, 0			// Write System Control Register

			// Enable D-side prefetch
			// Bit 2 [DP] Dside prefetch:
			// 0 = Dside prefetch disabled
			// 1 = Dside prefetch enabled.
			mrc p15, 0, r0, c1, c0, 1			// Read Auxiliary Control Register
			orr r0, r0, #(0x1 <<2)				// Enable Dside prefetch
			mcr p15, 0, r0, c1, c0, 1			// Write Auxiliary Control Register

			bx lr

invalidate_v7_cache: .global	  invalidate_v7_cache

					mov 	r0, #0
					mcr 	p15, 0, r0, c8, c7, 0	   // Invalidate Inst TLB and Data TLB
					mcr 	p15, 0, r0, c7, c5, 0	   // Invalidate I Cache

					// Must iterate over the caches in order to synthesise a complete clean
					// of data/unified cache
					mrc 	p15, 1, r0, c0, c0, 1	   // read Cache Level ID register (clidr)
					ands	r3, r0, #0x7000000		   // extract level of coherency from clidr
					mov 	r3, r3, lsr #23 		   // left align level of coherency bit field
					beq 	124f					   // if loc is 0, then no need to clean

					mov 	r10, #0 				   // start clean at cache level 0 (in r10)
			92:
					add 	r2, r10, r10, lsr #1	   // work out 3x current cache level
					mov 	r1, r0, lsr r2			   // extract cache type bits from clidr
					and 	r1, r1, #7				   // mask of the bits for current cache only
					cmp 	r1, #2					   // see what cache we have at this level
					blt 	119f					   // skip if no cache, or just i-cache
					mcr 	p15, 2, r10, c0, c0, 0	   // select current cache level in cssr
					mov 	r1, #0
					mcr 	p15, 0, r1, c7, c5, 4	   // prefetchflush to synch the new cssr&csidr
					mrc 	p15, 1, r1, c0, c0, 0	   // read the new csidr
					and 	r2, r1, #7				   // extract the length of the cache lines
					add 	r2, r2, #4				   // add 4 (line length offset)
					ldr 	r6, =0x3ff
					ands	r6, r6, r1, lsr #3		   // find maximum number on the way size
					.WORD	0xE16F5F16				   //DCI   0xE16F5F16 @CLZ r5, r6 @ find bit position of way size increment
					ldr 	r7, =0x7fff
					ands	r7, r7, r1, lsr #13 	   // extract max number of the index size
			109:
					mov 	r8, r6					   // create working copy of max way size
			111:
					orr 	r11, r10, r8, lsl r5	   // factor way and cache number into r11
					orr 	r11, r11, r7, lsl r2	   // factor index number into r11
					mcr 	p15, 0, r11, c7, c6, 2	   // invalidate by set/way
					subs	r8, r8, #1				   // decrement the way
					bge 	111b
					subs	r7, r7, #1				   // decrement the index
					bge 	109b
			119:
					add 	r10, r10, #2			   // increment cache number
					cmp 	r3, r10
					bgt 	92b

			124:

					//
					// If Cortex A Class handle secondary cores
					//
//					mrc 	p15, 0, r0, c0, c0, 5
//					and 	r0, #0x3
//					cmp 	r0, #0

//					bne 	__secondary_a_core

					//// back to parent routine
					bx	   lr


invalidate_SCU: .global	  invalidate_SCU

			/*set scu enable bit in scu*/
			ldr r7, =0x16000000
			ldr r0, [r7]
			orr r0, r0, #0x1
			str r0, [r7]

						/*invalidate scu*/
			ldr r7, =0x1600000c
			ldr r6, =0xffff
			str r6, [r7]
			bx	   lr



