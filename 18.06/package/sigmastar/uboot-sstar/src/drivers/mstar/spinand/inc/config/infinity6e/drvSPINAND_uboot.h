/*
* drvSPINAND_uboot.h- Sigmastar
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

#ifndef _REG_SPINAND_H_
#define _REG_SPINAND_H_
//	#include "asm/arch/mach/platform.h"

//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------

// !!! Uranus Serial Flash Notes: !!!
//  - The clock of DMA & Read via XIU operations must be < 3*CPU clock
//  - The clock of DMA & Read via XIU operations are determined by only REG_ISP_CLK_SRC; other operations by REG_ISP_CLK_SRC only
//  - DMA program can't run on DRAM, but in flash ONLY
//  - DMA from SPI to DRAM => size/DRAM start/DRAM end must be 8-B aligned
#if defined(CONFIG_MS_SPINAND_QUAD_READ)
#define SUPPORT_SPINAND_QUAD (1)
#endif
#define BDMA_W_FLAG     TRUE
#define BDMA_REVERT     (0)

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#define REG(Reg_Addr)               (*(volatile U16*)(Reg_Addr))
#define REG_OFFSET_SHIFT_BITS       2
#define GET_REG_ADDRESS(x, y)          (x+((y)<<REG_OFFSET_SHIFT_BITS))

//check Bounding Type bank 0x101E , offset 0x48 bit[5:4]
#define CHIP_BOUND_TYPES            (BIT5|BIT4)
#define CHIP_BOUND_QFN88            0x00//00: QFN88
#define CHIP_BOUND_QFN128           (BIT4)//01: QFN128
#define CHIP_BOUND_BGA1             (BIT5)//10: BGA1
#define CHIP_BOUND_BGA2A2B          (BIT5|BIT4)//11: BGA2A & BGA2B

// BASEADDR & BK
//------------------------------
#define RIU_PM_BASE                         0x1F000000
#define RIU_BASE                            0x1F200000
//#define BASE_REG_ISP_ADDR			GET_BASE_ADDR_BY_BANK(IO_ADDRESS(MS_BASE_REG_RIU_PA), 0x000800)
//#define BASE_REG_FSP_ADDR			GET_BASE_ADDR_BY_BANK(IO_ADDRESS(MS_BASE_REG_RIU_PA), 0x001600)
//#define BASE_REG_QSPI_ADDR			GET_BASE_ADDR_BY_BANK(IO_ADDRESS(MS_BASE_REG_RIU_PA), 0x001700)
//#define BASE_REG_CHIPTOP_ADDR		GET_BASE_ADDR_BY_BANK(IO_ADDRESS(MS_BASE_REG_RIU_PA), 0x101E00)
//#define BASE_REG_BDMACh0_ADDR	    GET_BASE_ADDR_BY_BANK(IO_ADDRESS(MS_BASE_REG_RIU_PA), 0x100200)

#define BK_ISP                              0x1000 //08h
#define BK_PM_FSP                           0x2C00 //16h
#define BK_PM_QSPI                          0x2E00 //17h
#define BK_FSP                              0x1A00 //0Dh
#define BK_QSPI                             0x1C00 //0Eh
#define BK_PMSLP                            0x1C00 //0eh
#define BK_CLK0                             0x1600 //100bH
#define BK_BDMA                             0x0400 //1002H //ch0
#define BK_CHIP                             0x3C00 //101EH
#define BK_CLKGEN                           0x7000 //1038H
//#define FSP_REG_BASE_ADDR                   GET_REG_ADDR(RIU_BASE, BK_FSP)
//#define QSPI_REG_BASE_ADDR                  GET_REG_ADDR(RIU_BASE, BK_QSPI)
//#define PMSLP_REG_BASE_ADDR                 GET_REG_ADDR(RIU_BASE, BK_PMSLP)
//#define CLK0_REG_BASE_ADDR                  GET_REG_ADDR(RIU_BASE, BK_CLK0)
//#define BDMA_REG_BASE_ADDR                  GET_REG_ADDR(RIU_BASE, BK_BDMA)
//#define CHIP_REG_BASE_ADDR                  GET_REG_ADDR(RIU_BASE, BK_CHIP)

#define REG_BANK_TIMER                      (0x1800)
#define TIMER_REG_BASE_ADDR                 GET_REG_ADDRESS(RIU_PM_BASE, REG_BANK_TIMER)
#define TIMER1_ENABLE                       GET_REG_ADDRESS(TIMER_REG_BASE_ADDR, 0x20)
#define TIMER1_HIT                          GET_REG_ADDRESS(TIMER_REG_BASE_ADDR, 0x21)
#define TIMER1_MAX_LOW                      GET_REG_ADDRESS(TIMER_REG_BASE_ADDR, 0x22)
#define TIMER1_MAX_HIGH                     GET_REG_ADDRESS(TIMER_REG_BASE_ADDR, 0x23)
#define TIMER1_CAP_LOW                      GET_REG_ADDRESS(TIMER_REG_BASE_ADDR, 0x24)
#define TIMER1_CAP_HIGH                     GET_REG_ADDRESS(TIMER_REG_BASE_ADDR, 0x25)

//----- Chip flash -------------------------
#include"../drvSPINAND_reg.h"
#endif // _REG_SPINAND_H_
