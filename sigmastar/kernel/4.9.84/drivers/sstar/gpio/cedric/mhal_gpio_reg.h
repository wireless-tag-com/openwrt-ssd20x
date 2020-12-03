/*
* mhal_gpio_reg.h- Sigmastar
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
#ifndef _REG_GPIO_H_
#define _REG_GPIO_H_

//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------
#define GPIO_UNIT_NUM               198

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define REG_MIPS_BASE               0xFD000000//Use 8 bit addressing

#define REG_ALL_PAD_IN              (0x101ea0)   //set all pads (except SPI) as input
#define REG_LVDS_BASE               (0x103200)
#define REG_LVDS_BANK               REG_LVDS_BASE


////8 bit define
#define REG_PMGPIO_00  0x0f00
#define REG_PMGPIO_01  0x0f01
#define REG_GPIO1_00   0x101A00////0
#define REG_GPIO1_01   0x101A01
#define REG_GPIO1_02   0x101A02////1
#define REG_GPIO1_03   0x101A03
#define REG_GPIO1_04   0x101A04////2
#define REG_GPIO1_05   0x101A05
#define REG_GPIO1_06   0x101A06////3
#define REG_GPIO1_07   0x101A07
#define REG_GPIO1_20   0x101A20 ////10
#define REG_GPIO1_21   0x101A21
#define REG_GPIO1_22   0x101A22 ////11
#define REG_GPIO1_23   0x101A23
#define REG_GPIO1_24   0x101A24 ////12
#define REG_GPIO1_25   0x101A25
#define REG_GPIO1_26   0x101A26 ////13
#define REG_GPIO1_27   0x101A27
#define REG_GPIO1_2A   0x101A2A ////15
#define REG_GPIO1_2B   0x101A2B
#define REG_GPIO1_2C   0x101A2C ////16
#define REG_GPIO1_2D   0x101A2D




#define GPIO_OEN    0   //set o to nake output
#define GPIO_ODN    1

#define IN_HIGH    1   //input high
#define IN_LOW     0   //input low

#define OUT_HIGH    1   //output high
#define OUT_LOW     0   //output low

#define MHal_GPIO_REG(addr)             (*(volatile U8*)(REG_MIPS_BASE + (((addr) & ~1)<<1) + (addr & 1)))

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

#endif // _REG_GPIO_H_

