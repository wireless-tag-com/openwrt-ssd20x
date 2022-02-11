/*
* sram.S- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: XXXX <XXXX@sigmastar.com.tw>
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
#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/asm-offsets.h>
#include <asm/hardware/cache-l2x0.h>

#define ARCH_us(x) (6*x)
    .align 3
    .globl sram_suspend_imi
    .globl v7_cpu_resume

/* A macro about using arch timer to delay
    inputs :
    t : us to delay
    l : For different branch naming
*/
.macro arch_usdelay t, l
    ldr   r8, =ARCH_us(\t)      /*delay tms*/
    mrrc  p15, 1, r9, r10, c14
    add   r9, r9, r8            /*Target tick*/
delay_retry\l:
    isb sy
    mrrc  p15, 1, r11, r12, c14
    cmp   r11, r9
    blt   delay_retry\l         /* jump to delay_retry in case r0 is bigger (N==1) */
.endm

ENTRY(sram_suspend_imi)
    //Below flow is run at SRAM(IMI)
    ldr   r1, =0xFD000000
    ldr   r3, =0x103300
    add   r2, r1, r3, lsl #1

    // 1. DDR enter self-refresh
    // wriu -w   0x1012e0  0x0000
    // wriu -w   0x101246  0xfffe
    // wriu -w   0x101266  0xffff
    // wriu -w   0x101286  0xffff
    // wriu -w   0x1012a6  0xffff
    // wriu -w   0x1011e6  0x00fe
    // wait  20

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
    arch_usdelay 1000,__LINE__

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
    //delay, DO NOT REMOVE THIS DELAY!!!!
    arch_usdelay 1000,__LINE__

    //wriu -w   0x101200  0x202e //Enter self-refresh
    ldr   r0, =0x202e
    str   r0, [r2, #0x00 << 1]
    //wait 10
    //delay
    arch_usdelay 1000,__LINE__

    // 05. AN power down
    //wriu -b 0x101203 0xF0 0xF0
    ldrb  r0, [r2, #0x05]
    orr   r0, r0, #0xF0
    strb  r0, [r2, #0x05]
    //wriu -b 0x101000 0x08 0x08
    //wriu -b 0x101000 0x10 0x10
    add   r2, r1, r5, lsl #1
    ldrb  r0, [r2, #0x00 << 1]
    orr   r0, r0, #0x08
    strb  r0, [r2, #0x00 << 1]
    ldrb  r0, [r2, #0x00 << 1]
    orr   r0, r0, #0x10
    strb  r0, [r2, #0x00 << 1]

    //wriu -b 0x101088 0x1f 0x00 // [4:0] turn-off rx ldo
    ldrb  r0, [r2, #0x88 << 1]
    and   r0, r0, #0xE0
    strb  r0, [r2, #0x88 << 1]
    
    //wriu -b 0x10108b 0x02 0x02 // [9] tree_ldo_pd
    ldrb  r0, [r2, #0x115]
    orr   r0, r0, #0x02
    strb  r0, [r2, #0x115]

    //wriu -b 0x101033 0xc0 0xc0 // [15:14] pll pd
//    ldrb  r0, [r2, #0x65]
//    orr   r0, r0, #0xc0
//    strb  r0, [r2, #0x65]

    
    //wriu -b 0x101054 0x70 0x70
    ldrb  r0, [r2, #0xA8]
    orr   r0, r0, #0x70
    strb  r0, [r2, #0xA8]
    //wriu -b 0x101008 0x3F 0x00
    ldrb  r0, [r2, #0x10]
    and   r0, r0, #0xC0
    strb  r0, [r2, #0x10]
 
	// set debug flag
	// wriu    0x100400 0x3333
	ldr r1, =0xFD000000
	ldr r3, =0x100400
	add   r2, r1, r3, lsl #1
	ldr r0, =0x3342
	str r0, [r2, #0x00 << 1]

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


	ldr r1, =0xFD000000

	// ETH power down
	ldr r3, =0x003300
	add   r2, r1, r3, lsl #1
	add r2, r2, #0x100

	//wriu 0x000033f2 0x80
	ldrh r0, =0x80
	strb r0, [r2, #0xE4]

	//wriu 0x000033f3 0x0f  //0xFD0067e5
	movw ip, #0x67E5
	movt ip, #0xFD00
	mov.w r0, 0xF
	strb.w r0, [ip]

	//wriu 0x0000323f 0x40 //0xFD00647D
	movw ip, #0x647D
	movt ip, #0xFD00
	mov.w r0, 0x40
	strb.w r0, [ip]

	//wriu 0x000032bb 0x40 //[6]: PD_REF //0xFD006575
	movw ip, #0x6575
	movt ip, #0xFD00
	mov.w r0, 0x40
	strb.w r0, [ip]

	//wriu 0x000032cc 0x08 //[4]: PD_ETHPLL_REG
	ldrh r0, =0x08
	strh r0, [r2, #0x98]

	//wriu 0x000032b7 0x10 //[4]: PD_ADC //0xFD00656D
	movw ip, #0x656D
	movt ip, #0xFD00
	mov.w r0, 0x10
	strb.w r0, [ip]

	//wriu 0x000032fc 0x02 //[1]: PD_REG25
	ldrh r0, =0x02
	strh r0, [r2, #0xF8]

	//wriu 0x000032fd 0x01 //[0]: PD_LDO11 //0xFD0065f9
	movw ip, #0x65f9
	movt ip, #0xFD00
	mov.w r0, 0x01
	strb.w r0, [ip]


	//wriu 0x000032d3 0xc0 //[15:14]: TX_OFF & LPF_INOFF //0xFD0065A5
	movw ip, #0x65A5
	movt ip, #0xFD00
	mov.w r0, 0xc0
	strb.w r0, [ip]

	//wriu 0x00003388 0x10 //[4]: RX_OFF
	ldr r3, =0x003300
	add   r2, r1, r3, lsl #1
	add r2, r2, #0x100
	ldrh r0, =0x10
	strh r0, [r2, #0x10]


	//wriu 0x000033a1 0x30 //0xFD006741
	movw ip, #0x6741
	movt ip, #0xFD00
	mov.w r0, 0x30
	strb.w r0, [ip]

	//[4]: PD_LPF_VBUF
	//[5]: PD_SADC
	//wriu 0x0000333a 0x03
	ldrh r0, =0x03
	sub r2, r2, #0x100
	strh r0, [r2, #74]
	//[0]: PD_TX_LD
	//[1]: PD_TX_IDAC
	//wriu 0x000033f1 0x3c  //0xFD0067E1
	movw ip, #0x67E1
	movt ip, #0xFD00
	mov.w r0, 0x3c
	strb.w r0, [ip]

	//[2]: PD_TX_VBGR
	//[3]: PD_TX_TRIMMING_DAC
	//[4]: PD_TX_LD_DIO
	//[5]: PD_TX_LDO

	// HDMI  power down
	ldr r3, =0x112600
	add   r2, r1, r3, lsl #1
	//wriu 0x0011262c 0x00
	ldrh r0, =0x00
	strh r0, [r2, #0x58]

	//wriu 0x0011262d 0x00   //0xFD224c59
	movw ip, #0x4c59
	movt ip, #0xFD22
	mov.w r0, 0x00
	strb.w r0, [ip]

	// [3:0]: EN_PSTDR_TAP_CH0
	// [7:4]: EN_PSTDR_TAP_CH1
	// [11:8]: EN_PSTDR_TAP_CH2
	//wriu 0x00112636 0x00
	ldrh r0, =0x00
	strh r0, [r2, #0x36 << 1]
	// [1]: EN_HDMITXPLL_XTAL
	//wriu 0x00112660 0xff
	ldrh r0, =0xff
	strh r0, [r2, #0x60 << 1]
	//wriu 0x00112661 0x0f   //0xFD224cC1
	movw ip, #0x4CC2
	movt ip, #0xFD22
	mov.w r0, 0x0f
	strb.w r0, [ip]
	// [3:0]: PD_PSTDRV_TAP_CH0
	// [7:4]: PD_PSTDRV_TAP_CH1
	// [11:8]: PD_PSTDRV_TAP_CH2
	//wriu 0x00112662 0xff
	ldrh r0, =0xff
	strh r0, [r2, #0x62 << 1]

	//wriu 0x00112663 0x0f      //0xFD224cC5
	movw ip, #0x4CC5
	movt ip, #0xFD22
	mov.w r0, 0x0f
	strb.w r0, [ip]

	// [3:0]: PD_PREDRV_TAP_CH0
	// [7:4]: PD_PREDRV_TAP_CH1
	// [11:8]: PD_PREDRV_TAP_CH2
	//wriu 0x00112664 0xff
	ldrh r0, =0xff
	strh r0, [r2, #0x64 << 1]

	//wriu 0x00112665 0x0f   //0xFD224cC9
	movw ip, #0x4CC9
	movt ip, #0xFD22
	mov.w r0, 0x0f
	strb.w r0, [ip]

	// [3:0]: PD_RTERM_CH
	// [7:4]: PD_LDO_PREDRV_CH
	// [11:8]: PD_LDO_MUX_CH
	//wriu 0x00112666 0x01
	ldrh r0, =0x01
	strh r0, [r2, #0x66 << 1]
	// [0]: PD_LDO_CLKTREE
	//wriu 0x00112668 0x01
	ldrh r0, =0x01
	strh r0, [r2, #0x68 << 1]
	// [0]: PD_HDMITXPLL
	//wriu 0x0011266a 0x1f
	ldrh r0, =0x1f
	strh r0, [r2, #0x6a << 1]
	// [3:0]: PD_DRV_BIASGEN_CH
	// [4]: PD_BIASGEN

	// IDAC power down
	ldr r3, =0x112700
	add   r2, r1, r3, lsl #1
	//wriu 0x0011272a 0x00
	ldrh r0, =0x00
	strh r0, [r2, #0x2a << 1]
	// [0]: EN_IDAC_B
	// [1]: EN_IDAC_G
	// [2]: EN_IDAC_R
	//wriu 0x0011272c 0x00
	ldrh r0, =0x00
	strh r0, [r2, #0x2c << 1]
	// [0]: EN_IDAC_REF
	//wriu 0x00112728 0x00
	ldrh r0, =0x00
	strh r0, [r2, #0x28 << 1]
	// [0]: EN_IDAC_B_DET
	// [1]: EN_IDAC_G_DET
	// [2]: EN_IDAC_R_DET
	//wriu 0x0011273e 0x01
	ldrh r0, =0x01
	strh r0, [r2, #0x3e << 1]
	// [0]: PD_LDO
	//wriu 0x00112720 0x00
	ldrh r0, =0x00
	strh r0, [r2, #0x20 << 1]
	// [0]: EN_GPIO_B
	// [1]: EN_GPIO_G
	// [2]: EN_GPIO_R

	// IDAC_PLL power down
	//wriu 0x0011276c 0x01
	ldrh r0, =0x01
	strh r0, [r2, #0x6c << 1]
	// [0]: PD_EMMCPLL

	// LPLL power down	
	ldr r3, =0x103300
	add   r2, r1, r3, lsl #1
	//wriu 0x00103381 0x80    //0xFD206701
	movw ip, #0x6701
	movt ip, #0xFD20
	mov.w r0, 0x80
	strb.w r0, [ip]

	//MIPI TX power down
	ldr r3, =0x152800
	add   r2, r1, r3, lsl #1
	//wriu 0x152800 0x40 //[6]: PD_LDO
	ldrh r0, =0x40
	strh r0, [r2, #0x00 << 1]
	//wriu 0x152802 0x03 //[0]: PD_IB_DPHY
	ldrh r0, =0x03
	strh r0, [r2, #0x02 << 1]
	//wriu 0x152808 0xa0 //[4]: LPTX0_EN; [6]: LPRX0_EN
	ldrh r0, =0xa0
	strh r0, [r2, #0x08 << 1]

	//wriu 0x152809 0x04 //[1:0]: HSTX0_EN[1:0]  //0xFD2A5011
	movw ip, #0x5011
	movt ip, #0xFD2A
	mov.w r0, 0x04
	strb.w r0, [ip]

	//wriu 0x152810 0xa0 //[4]: LPTX1_EN; [6]: LPRX1_EN
	ldrh r0, =0xa0
	strh r0, [r2, #0x10 << 1]
	//wriu 0x152811 0x04 //[1:0]: HSTX1_EN[1:0]   //0xFD2A5021

	movw ip, #0x5021
	movt ip, #0xFD2A
	mov.w r0, 0x04
	strb.w r0, [ip]

	//wriu 0x152818 0xa0 //[4]: LPTX2_EN; [6]: LPRX2_EN
	ldrh r0, =0xa0
	strh r0, [r2, #0x18 << 1]

	//wriu 0x152819 0x04 //[1:0]: HSTX2_EN[1:0]   //0xFD2A5031
	movw ip, #0x5031
	movt ip, #0xFD2A
	mov.w r0, 0x04
	strb.w r0, [ip]

	//wriu 0x152840 0xa0
	ldrh r0, =0xa0
	strh r0, [r2, #0x40 << 1]

	//wriu 0x152841 0x04              //0xFD2A5081
	movw ip, #0x5081
	movt ip, #0xFD2A
	mov.w r0, 0x04
	strb.w r0, [ip]

	//wriu 0x152846 0xa0
	ldrh r0, =0xa0
	strh r0, [r2, #0x46 << 1]
	//wriu 0x152847 0x04             //0xFD2A508C
	movw ip, #0x508C
	movt ip, #0xFD2A
	mov.w r0, 0x04
	strb.w r0, [ip]

	//wriu 0x15280b 0x08             //0xFD2A5015
	movw ip, #0x5015
	movt ip, #0xFD2A
	mov.w r0, 0x08
	strb.w r0, [ip]

	//wriu 0x152813 0x08             //0xFD2A5025
	movw ip, #0x5025
	movt ip, #0xFD2A
	mov.w r0, 0x08
	strb.w r0, [ip]
	//wriu 0x15281b 0x08            //0xFD2A5035
	movw ip, #0x5035
	movt ip, #0xFD2A
	mov.w r0, 0x08
	strb.w r0, [ip]
	//wriu 0x152843 0x08 //[2]: HSRX3_EN     //0xFD2A5085
	movw ip, #0x5085
	movt ip, #0xFD2A
	mov.w r0, 0x08
	strb.w r0, [ip]
	//wriu 0x152849 0x08 //[2]: HSRX4_EN    //0xFD2A5091
	movw ip, #0x5091
	movt ip, #0xFD2A
	mov.w r0, 0x08
	strb.w r0, [ip]
	//wriu 0x152806 0x02 //[0]: DPHY_CLKEN
	ldrh r0, =0x02
	strh r0, [r2, #0x06 << 1]

	// SATA power down
	ldr r3, =0x152700
	add   r2, r1, r3, lsl #1
	//wriu 0x00152740 0x01
	ldrh r0, =0x01
	strh r0, [r2, #0x40 << 1]
	// [0]: PD_SATA_TXPLL
	//wriu 0x00152760 0x01
	ldrh r0, =0x01
	strh r0, [r2, #0x60 << 1]
	// [0]: PD_SATA_RXPLL
	//wriu 0x00152763 0x04    //0xFD2A4EC5
	movw ip, #0x4EC5
	movt ip, #0xFD2A
	mov.w r0, 0x04
	strb.w r0, [ip]
	// [10]: PD_SATA_RXPLL_CDR
	//wriu 0x00152610 0xff
	ldrh r0, =0xff
	strh r0, [r2, #0x10 << 1]

        // BANK 1526, 16bit OFFSET 0x00[15:0]
	//wriu 0x00152600 0x105b
	ldr   r0, =0x105B
	str   r0, [r2, #0x00 << 1]

	//wriu 0x00152602 0x00
	ldrh r0, =0x00
	strh r0, [r2, #0x02 << 1]
	// [5]: EN_SSUSB_PLL_SSC
	// [6]: EN_SSUSB_RX_IMPCALIB

	// UPLL_0 power down
	ldr r3, =0x142000
	add   r2, r1, r3, lsl #1
	//wriu 0x00142000 0x32
	ldrh r0, =0x32
	strh r0, [r2, #0x00 << 1]
	//[1]: PD_UPLL
	//[4]: PD_CLKO_UPLL_20M
	//[5]: PD_CLKO_UPLL_320M
	//wriu 0x00142001 0x00 //[7]: EN_OV_UPLL_CP_SW    //0xFD284001
	movw ip, #0x4001
	movt ip, #0xFD28
	mov.w r0, 0x00
	strb.w r0, [ip]
	//wriu 0x00142002 0x00 //[7]: EN_UPLL_PRDT
	ldrh r0, =0x00
	strh r0, [r2, #0x02 << 1]
	//wriu 0x00142004 0x00 //[7:0]: EN_UPLL_TEST[7:0]
	ldrh r0, =0x00
	strh r0, [r2, #0x04 << 1]
	//wriu 0x00142005 0x00 //[7:0]: EN_UPLL_TEST[15:8]  //0xFD284009
	movw ip, #0x4009
	movt ip, #0xFD28
	mov.w r0, 0x00
	strb.w r0, [ip]
	//wriu 0x0014200e 0x04
	ldrh r0, =0x04
	strh r0, [r2, #0x0e << 1]
	//[0]: EN_CLKO_UPLL_384M
	//[1]: EN_UPLL_PRDT2
	//[2]: GCR_UPLL_PD_CLKO_AUDIO

	// set ckg_mcu to xtali
	ldr r1, =0xFD000000
	ldr r3, =0x103800
	add   r2, r1, r3, lsl #1
	ldrb  r0, [r2, #0x4]
	and   r0, r0, #0xDF
	strb  r0, [r2, #0x4]

	// UPLL_1 power down
	ldr r3, =0x141f00
	add   r2, r1, r3, lsl #1
	//wriu 0x00141f00 0x32
	ldrh r0, =0x32
	strh r0, [r2, #0x00 << 1]
	//[1]: PD_UPLL
	//[4]: PD_CLKO_UPLL_20M
	//[5]: PD_CLKO_UPLL_320M
	//wriu 0x00141f01 0x00 //[7]: EN_OV_UPLL_CP_SW  //0xFD283E01
	movw ip, #0x3E01
	movt ip, #0xFD28
	mov.w r0, 0x00
	strb.w r0, [ip]
	//wriu 0x00141f02 0x00 //[7]: EN_UPLL_PRDT
	ldrh r0, =0x00
	strh r0, [r2, #0x02 << 1]
	//wriu 0x00141f04 0x00 //[7:0]: EN_UPLL_TEST[7:0]
	ldrh r0, =0x00
	strh r0, [r2, #0x04 << 1]
	//wriu 0x00141f05 0x00 //[7:0]: EN_UPLL_TEST[15:8]  //0xFD283E09
	movw ip, #0x3E09
	movt ip, #0xFD28
	mov.w r0, 0x00
	strb.w r0, [ip]
	//wriu 0x00141f0e 0x04
	ldrh r0, =0x04
	strh r0, [r2, #0x0e << 1]
	//[0]: EN_CLKO_UPLL_384M
	//[1]: EN_UPLL_PRDT2
	//[2]: GCR_UPLL_PD_CLKO_AUDIO

	// set debug flag
	// wriu    0x100400 0x3333
	ldr r1, =0xFD000000
	ldr r3, =0x100400
	add   r2, r1, r3, lsl #1
	ldr r0, =0x3343
	str r0, [r2, #0x00 << 1]

	// USB2.0 P1 power down
	ldr r3, =0x142100
	add   r2, r1, r3, lsl #1
	//wriu 0x00142100 0x05
	ldrh r0, =0x05
	strh r0, [r2, #0x00 << 1]
	// [2]: PDN_REF
	// [6]: R_DP_PDEN
	// [7]: R_DM_PDEN
	//wriu 0x00142101 0xff         //0xFD284201
	movw ip, #0x4201
	movt ip, #0xFD28
	mov.w r0, 0xff
	strb.w r0, [ip]
	// [0]: HS_DM_PDN
	// [2]: HS_TED_PDN
	// [3]: HS_PREAMP_PDN
	// [4]: FL_XCVR_PDN
	// [6]: IREF_PDN
	// [7]: REG_PDN
	//wriu 0x00142108 0x80 //[7]PD_BG_CURRENT
	ldrh r0, =0x80
	strh r0, [r2, #0x08 << 1]
	//wriu 0x00142109 0x00
	ldrh r0, =0x00
	strh r0, [r2, #0x09 << 1]
	//wriu 0x00142110 0x00 //[6]HS_TXSER_EN
	ldrh r0, =0x00
	strh r0, [r2, #0x10 << 1]

	// USB2.0 P2 power down
	ldr r3, =0x142500
	add   r2, r1, r3, lsl #1
	//wriu 0x00142500 0x05
	ldrh r0, =0x05
	strh r0, [r2, #0x00 << 1]
	// [2]: PDN_REF
	// [6]: R_DP_PDEN
	// [7]: R_DM_PDEN
	//wriu 0x00142501 0xff   //0xFD284A01
	movw ip, #0x4A01
	movt ip, #0xFD28
	mov.w r0, 0xff
	strb.w r0, [ip]	
	// [0]: HS_DM_PDN
	// [2]: HS_TED_PDN
	// [3]: HS_PREAMP_PDN
	// [4]: FL_XCVR_PDN
	// [6]: IREF_PDN
	// [7]: REG_PDN
	//wriu 0x00142508 0x80 //[7]PD_BG_CURRENT
	ldrh r0, =0x80
	strh r0, [r2, #0x08 << 1]
	//wriu 0x00142509 0x00     //0xFD284A11
	movw ip, #0x4A11
	movt ip, #0xFD28
	mov.w r0, 0x00
	strb.w r0, [ip]	
	//wriu 0x00142510 0x00 //[6]HS_TXSER_EN
	ldrh r0, =0x00
	strh r0, [r2, #0x10 << 1]

	// USB2.0 P3 power down
	ldr r3, =0x142900
	add   r2, r1, r3, lsl #1
	//wriu 0x00142900 0x05
	ldrh r0, =0x05
	strh r0, [r2, #0x00 << 1]
	// [2]: PDN_REF
	// [6]: R_DP_PDEN
	// [7]: R_DM_PDEN
	//wriu 0x00142901 0xff  //0xFD285201
	movw ip, #0x5201
	movt ip, #0xFD28
	mov.w r0, 0xff
	strb.w r0, [ip]
	// [0]: HS_DM_PDN
	// [2]: HS_TED_PDN
	// [3]: HS_PREAMP_PDN
	// [4]: FL_XCVR_PDN
	// [6]: IREF_PDN
	// [7]: REG_PDN
	//wriu 0x00142908 0x80 //[7]PD_BG_CURRENT
	ldrh r0, =0x80
	strh r0, [r2, #0x08 << 1]
	//wriu 0x00142909 0x00   //0xFD285211
	movw ip, #0x5211
	movt ip, #0xFD28
	mov.w r0, 0x00
	strb.w r0, [ip]
	//wriu 0x00142910 0x00 //[6]HS_TXSER_EN
	ldrh r0, =0x00
	strh r0, [r2, #0x10 << 1]

	// AUDIO power down

	// BANK 1034, 16bit OFFSET 0x00[1:0]: reg_en_byp_inmux
	ldr r3, =0x103400
	add   r2, r1, r3, lsl #1
	ldr   r0, =0x0
	str   r0, [r2, #0x00 << 1]

	// BANK 1034, 16bit OFFSET 0x01[1:0]: reg_en_mute_inmux
	ldr   r0, =0x0
	str   r0, [r2, #0x02 << 1]

	// BANK 1034, 16bit OFFSET 0x03[0]: reg_pd_adc0
	ldr   r0, =0x1fff
	str   r0, [r2, #0x06 << 1]


//    //switch L3 bridge clock to MIU2X clock
//    OUTREG8(GET_REG8_ADDR(RIU_BASE_ADDR,   0x00102202), 0x84);
	movw ip, #0x4404
	movt ip, #0xFD20
	mov.w r0, 0x04
	strb.w r0, [ip]

//    OUTREG8(GET_REG8_ADDR(RIU_BASE_ADDR,   0x0010382E), 0x18);//switch MIU clock from xtal(12MHz) to MIU PLL
//    OUTREG8(GET_REG8_ADDR(RIU_BASE_ADDR,   0x00103840), 0x04);//switch clk_miu_sc_gp from clk_boot_p(12MHz) to clk_miu
//	//switch MIU clock from xtal(12MHz) to MIU PLL
//	//OUTREG8(GET_REG8_ADDR(RIU_BASE_ADDR,   0x0010382E), 0x18);
	movw ip, #0x705C
	movt ip, #0xFD20
	mov.w r0, 0x08
	strb.w r0, [ip]

	// set debug flag
	// wriu    0x100400 0x3333
	ldr r1, =0xFD000000
	ldr r3, =0x100400
	add   r2, r1, r3, lsl #1
	ldr r0, =0x3344
	str r0, [r2, #0x00 << 1]

//	//switch clk_miu_sc_gp from clk_boot_p(12MHz) to clk_miu
//	//OUTREG8(GET_REG8_ADDR(RIU_BASE_ADDR,   0x00103840), 0x04);
	movw ip, #0x7080
	movt ip, #0xFD20
	mov.w r0, 0x00
	strb.w r0, [ip]

	// set debug flag
	// wriu    0x100400 0x3333
	ldr r1, =0xFD000000
	ldr r3, =0x100400
	add   r2, r1, r3, lsl #1
	ldr r0, =0x3355
	str r0, [r2, #0x00 << 1]

//	// miupll power down
//	//swch 4
//	//wriu    0x00103103 0x01
	movw ip, #0x6205
	movt ip, #0xFD20
	mov.w r0, 0x01
	//mov.w r0, 0x1f  !!halt
	strb.w r0, [ip]

	// set debug flag
	// wriu    0x100400 0x3333
	ldr r1, =0xFD000000
	ldr r3, =0x100400
	add   r2, r1, r3, lsl #1
	ldr r0, =0x3366
	str r0, [r2, #0x00 << 1]

//	// armpll power down
//    OUTREG8(GET_REG8_ADDR(RIU_BASE_ADDR,   0x001020F8), 0x01);
//switch ARM clock from xtal(12MHz) to ARM PLL
	movw r4, #0x41F0
	movt r4, #0xFD20
	mov.w r0, 0x00
	strb.w r0, [r4]

	movw r4, #0x6445
	movt r4, #0xFD20
	mov.w r0, 0x01
	strb.w r0, [r4]

	// set debug flag
	// wriu    0x100400 0x3333
	ldr r1, =0xFD000000
	ldr r3, =0x100400
	add   r2, r1, r3, lsl #1
	ldr r0, =0x3333
	str r0, [r2, #0x00 << 1]

	// MPLL power down
	ldr r3, =0x103000
	add   r2, r1, r3, lsl #1

	//wriu 0x00103003 0x1f   //0xFD206005
	movw ip, #0x6005
	movt ip, #0xFD20
	mov.w r0, 0x10
	//mov.w r0, 0x1f  !!halt
	strb.w r0, [ip]


	//[0]: PD_MPLL
	//[1]: PD_MPLL_CLK_ADC_VCO_DIV2
	//[2]: PD_MPLL_CLK_ADC_VCO_DIV2_2
	//[3]: PD_MPLL_CLK_ADC_VCO_DIV2_3
	//[4]: PD_DIGCLK
	//wriu 0x00103004 0x00 // [0]: EN_MPLL_RST
	ldrh r0, =0x00
	strh r0, [r2, #0x04 << 1]
	//wriu 0x00103008 0x00
	ldrh r0, =0x00
	strh r0, [r2, #0x08 << 1]
	//[0]: EN_MPLL_TEST
	//[1]: EN_MPLL_OV_CP_SW
	//wriu 0x00103009 0x00    //0xFD206011
	movw ip, #0x6011
	movt ip, #0xFD20
	mov.w r0, 0x00
	strb.w r0, [ip]
	//[2]: EN_MPLL_XTAL
	//[7]: EN_MPLL_PRDT

    //wriu -b 0x101033 0xc0 0xc0 // [15:14] pll pd
    	movw r2, #0x2000
	movt r2, #0xFD20
	ldrb  r0, [r2, #0x65]
	orr   r0, r0, #0xc0
	strb  r0, [r2, #0x65]

	// set debug flag
	// wriu    0x100400 0x3333
	ldr r1, =0xFD000000
	ldr r3, =0x100400
	add   r2, r1, r3, lsl #1
	ldr r0, =0x3322
	str r0, [r2, #0x00 << 1]

	/*wfi*/
	wfi
	nop
	nop
	nop
	nop
	// soft reset
	// wriu    0x000e5C 0x79  //0xe, 0x2e=0x79
	ldr r3, =0x000e00
	add   r2, r1, r3, lsl #1
	ldr r0, =0x79
	str r0, [r2, #0x5C << 1]

ENDPROC(sram_suspend_imi)
.ltorg
