/*
* io.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: edie.chen <edie.chen@sigmastar.com.tw>
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

#ifndef __ASM_ARM_ARCH_IO_H
#define __ASM_ARM_ARCH_IO_H

//------------------------------------------------------------------------------
//  Include Files
//------------------------------------------------------------------------------

/*------------------------------------------------------------------------------
    Constant
-------------------------------------------------------------------------------*/

/* max of IO Space */
#define IO_SPACE_LIMIT 0xffffffff

/* Constants of CEDRIC RIU */
#define IO_PHYS         0x1F000000 //sync with platform.h
#define IO_OFFSET       0x00000000
#define IO_SIZE         0x00400000
#define IO_VIRT         (IO_PHYS + IO_OFFSET)
#define io_p2v(pa)      ((pa) + IO_OFFSET)
#define io_v2p(va)      ((va) - IO_OFFSET)
#define IO_ADDRESS(x)   io_p2v(x)

/*------------------------------------------------------------------------------
    Macro
-------------------------------------------------------------------------------*/

/* read register by byte */
#define ms_readb(a) (*(volatile unsigned char *)IO_ADDRESS(a))

/* read register by word */
#define ms_readw(a) (*(volatile unsigned short *)IO_ADDRESS(a))

/* read register by long */
#define ms_readl(a) (*(volatile unsigned int *)IO_ADDRESS(a))

/* write register by byte */
#define ms_writeb(v,a) (*(volatile unsigned char *)IO_ADDRESS(a) = (v))

/* write register by word */
#define ms_writew(v,a) (*(volatile unsigned short *)IO_ADDRESS(a) = (v))

/* write register by long */
#define ms_writel(v,a) (*(volatile unsigned int *)IO_ADDRESS(a) = (v))


//------------------------------------------------------------------------------
//
//  Macros:  INREGx/OUTREGx/SETREGx/CLRREGx
//
//  This macros encapsulates basic I/O operations.
//  Memory address space operation is used on all platforms.
//
#define INREG8(x)           ms_readb(x)
#define OUTREG8(x, y)       ms_writeb((u8)(y), x)
#define SETREG8(x, y)       OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y)       OUTREG8(x, INREG8(x)&~(y))

#define INREG16(x)          ms_readw(x)
#define OUTREG16(x, y)      ms_writew((u16)(y), x)
#define SETREG16(x, y)      OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y)      OUTREG16(x, INREG16(x)&~(y))

#define INREG32(x)          ms_readl(x)
#define OUTREG32(x, y)      ms_writel((u32)(y), x)
#define SETREG32(x, y)      OUTREG32(x, INREG32(x)|(y))
#define CLRREG32(x, y)      OUTREG32(x, INREG32(x)&~(y))

#define BIT_0       (1<<0)
#define BIT_1       (1<<1)
#define BIT_2       (1<<2)
#define BIT_3       (1<<3)
#define BIT_4       (1<<4)
#define BIT_5       (1<<5)
#define BIT_6       (1<<6)
#define BIT_7       (1<<7)
#define BIT_8       (1<<8)
#define BIT_9       (1<<9)
#define BIT_10      (1<<10)
#define BIT_11      (1<<11)
#define BIT_12      (1<<12)
#define BIT_13      (1<<13)
#define BIT_14      (1<<14)
#define BIT_15      (1<<15)


//------------------------------------------------------------------------------
//
//  Function: read_chip_revision
//
//  This inline function returns the chip revision (for drivers)
//
static inline u32 read_chip_revision(void)
{
//!!!!CEDRIC
//	u32 u32_rev = *((volatile u32*)(0xA0003C00+(0x67<<2)));
//    return ((u32_rev & 0xff00)>>8);

	return 0x01;
}

#ifdef CONFIG_BOOT_TIME_ANALYSIS
#define BOOT_TIME_MSG(...) printk("BOOT_TIME> %d ms : ", (get_timer2_counter()/12)/1000); \
						   printk(__VA_ARGS__);
#else
#define BOOT_TIME_MSG(...)
#endif

//// for boot time analysis
//#define REG_TIMER2_BASE             0xA0007780
//#define REG_TIMER2_MAX_L            (REG_TIMER2_BASE + 0x0)  //0xA0007780
//#define REG_TIMER2_MAX_H            (REG_TIMER2_BASE + 0x4)  //0xA0007784
//#define REG_TIMER2_CAPCNT_L       	(REG_TIMER2_BASE + 0x8)  //0xA0007788
//#define REG_TIMER2_CAPCNT_H       	(REG_TIMER2_BASE + 0xC)  //0xA000778C
//#define REG_TIMER2_CTRL           	(REG_TIMER2_BASE + 0x10) //0xA0007790
//
//#define TIMER_ENABLE    			(0x03<<8)
//#define TIMER_CLEAR      			(0x02)
//#define CAPTURE_ENABLE              (0x01)
//
//static inline uint8_t is_timer2_start(void)
//{
//    if(INREG16(REG_TIMER2_CTRL) & TIMER_ENABLE) return 1;
//    else return 0;
//}
//
//static inline uint32_t get_timer2_counter(void)
//{
//	static uint8_t enable = 0;
//	volatile u32 u32temp;
//
//	if(!enable)
//	{
//		enable = 1;
//		if(!is_timer2_start())
//		{
//			OUTREG16(REG_TIMER2_MAX_L, 0xffff);
//	    	OUTREG16(REG_TIMER2_MAX_H, 0xffff);
//			OUTREG16(REG_TIMER2_CTRL, TIMER_ENABLE|TIMER_CLEAR);
//			OUTREG16(REG_TIMER2_CTRL, TIMER_ENABLE);
//		}
//	}
//
//    SETREG16( REG_TIMER2_CTRL, CAPTURE_ENABLE );
//    u32temp = ((INREG16(REG_TIMER2_CAPCNT_L) & 0xFFFF) |
//              ((INREG16(REG_TIMER2_CAPCNT_H) & 0xFFFF) << 16));
//    u32temp = ((INREG16(REG_TIMER2_CAPCNT_L) & 0xFFFF) |
//              ((INREG16(REG_TIMER2_CAPCNT_H) & 0xFFFF) << 16));
//    return u32temp;
//}


#endif /* __ASM_ARM_ARCH_IO_H */

