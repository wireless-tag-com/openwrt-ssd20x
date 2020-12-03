/*
* regMIU.h- Sigmastar
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
//<MStar Software>
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// * Copyright (c) 2010 - 2012 MStar Semiconductor, Inc.
// This program is free software.
// You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation;
// either version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// file    regMIU.h
/// @brief  MIU Control Register Definition
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _REG_MIU_H_
#define _REG_MIU_H_


//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#ifndef BIT
#define BIT(_bit_)                  (1 << (_bit_))
#endif

#define BITS_RANGE(range)                       (BIT(((1)?range)+1) - BIT((0)?range))
#define BITS_RANGE_VAL(x, range)                ((x & BITS_RANGE(range)) >> ((0)?range))
#define MIU_REG_BASE                            (0x1200UL)
#define MIU1_REG_BASE                           (0x0600UL)
#define MIU2_REG_BASE                           (0x62000UL)
#define PM_REG_BASE                             (0x1E00UL)
#define MIU_ATOP_BASE                           (0x10D00UL)
// #define MIU1_ATOP_BASE                          (0x61600UL)
// #define MIU2_ATOP_BASE                          (0x62100UL)
#define CHIP_TOP_BASE                           (0x1E00UL)
#define MIU_ARB_REG_BASE                        (0x61500UL)
#define MIU1_ARB_REG_BASE                       (0x62200UL)
#define MIU2_ARB_REG_BASE                       (0x62300UL)

#define PM_CHIP_REVISION                        (PM_REG_BASE+0x03UL) // 0x1E03
#define DDR_FREQ_SET_0                          (MIU_REG_BASE+0x20UL) // 0x1220
#define DDR_FREQ_SET_1                          (MIU_REG_BASE+0x21UL) //0x1221
#define DDR_FREQ_DIV_1                          (MIU_REG_BASE+0x25UL) //0x1225
#define DDR_FREQ_INPUT_DIV_2                    (MIU_REG_BASE+0x26UL) //0x1226
#define DDR_FREQ_LOOP_DIV_2                     (MIU_REG_BASE+0x27UL) //0x1227
#define DDR_CLK_SELECT                          (MIU_REG_BASE+0x3eUL) //0x123E
#define DDR_FREQ_STATUS                         (MIU_REG_BASE+0x3fUL) //0x123F

#define MIU_RQ0L_MASK                           (MIU_REG_BASE+0x46UL)
#define MIU_RQ0H_MASK                           (MIU_REG_BASE+0x47UL)
#define MIU_RQ1L_MASK                           (MIU_REG_BASE+0x66UL)
#define MIU_RQ1H_MASK                           (MIU_REG_BASE+0x67UL)
#define MIU_RQ2L_MASK                           (MIU_REG_BASE+0x86UL)
#define MIU_RQ2H_MASK                           (MIU_REG_BASE+0x87UL)
#define MIU_RQX_MASK(Reg, Group)                (Reg = (Group < 4 )?  (MIU_REG_BASE + 0x46UL + 0x20UL*Group) : (MIU_ARB_REG_BASE + 0x06UL + 0x20UL * (Group - 4)))
#define MIU_RQX_HPMASK(Reg, Group)              (Reg = (Group < 4 )?  (MIU_REG_BASE + 0x48UL + 0x20UL*Group) : (MIU_ARB_REG_BASE + 0x08UL + 0x20UL * (Group - 4)))

#define MIU_PROTECT_EN_INTERNAL                 (MIU_REG_BASE+0xD2UL)
#define MIU_PROTECT_DDR_SIZE                    (MIU_REG_BASE+0xD3UL)
#define MIU_PROTECT_DDR_SIZE_MASK               BITS_RANGE(11:8)
#define MIU_PROTECT_DDR_32MB                    (0x50UL)
#define MIU_PROTECT_DDR_64MB                    (0x60UL)
#define MIU_PROTECT_DDR_128MB                   (0x70UL)
#define MIU_PROTECT_DDR_256MB                   (0x80UL)
#define MIU_PROTECT_DDR_512MB                   (0x90UL)
#define MIU_PROTECT_DDR_1024MB                  (0xA0UL)
#define MIU_PROTECT_DDR_2048MB                  (0xB0UL)


#define MIU_PROTECT0_ID0                        (MIU_REG_BASE+0x2EUL)
#define MIU_BW_REQUEST                          (MIU_REG_BASE+0x1AUL)
#define MIU_BW_RESULT                           (MIU_REG_BASE+0x1CUL)
#define MIU_PROTECT0_ID_ENABLE                  (MIU_REG_BASE+0x20UL)
#define MIU_PROTECT1_ID_ENABLE                  (MIU_REG_BASE+0x22UL)
#define MIU_PROTECT2_ID_ENABLE                  (MIU_REG_BASE+0x24UL)
#define MIU_PROTECT3_ID_ENABLE                  (MIU_REG_BASE+0x26UL)
#define MIU_PROTECT0_MSB                        (MIU_REG_BASE+0xD0UL)
#define MIU_PROTECT0_START                      (MIU_REG_BASE+0xC0UL)
#define MIU_PROTECT1_START                      (MIU_REG_BASE+0xC4UL)
#define MIU_PROTECT2_START                      (MIU_REG_BASE+0xC8UL)
#define MIU_PROTECT3_START                      (MIU_REG_BASE+0xCCUL)
#define REG_MIU_PROTECT_ENGINE                 (0x69UL << 1)	
#define REG_MIU_PROTECT_LOADDR                  (0x6DUL << 1)	//0xDE
#define REG_MIU_PROTECT_HIADDR                  (0x6EUL << 1)	//0xDE
#define REG_MIU_GROUP_PRIORITY                  (0x7FUL << 1)
#define REG_MIU_PROTECT_STATUS                  (0x6FUL << 1)	//0xDE

// MIU selection registers
#define REG_MIU_SEL0                            (MIU_REG_BASE+0xf0UL)  //0x12F0
#define REG_MIU_SEL1                            (MIU_REG_BASE+0xf2UL)  //0x12F1
#define REG_MIU_SEL2                            (MIU_REG_BASE+0xf4UL)  //0x12F2
#define REG_MIU_SEL3                            (MIU_REG_BASE+0xf6UL)  //0x12F3
#define REG_MIU_SELX(x)                         (0xF0UL+x*2)

//MIU1
#define MIU1_PROTECT_EN                          (MIU1_REG_BASE+0xD2UL)
#define MIU1_PROTECT_DDR_SIZE                    (MIU1_REG_BASE+0xD3UL)
#define MIU1_PROTECT0_ID0                        (MIU1_REG_BASE+0x2EUL)
#define MIU1_BW_REQUEST                          (MIU1_REG_BASE+0x1AUL)
#define MIU1_BW_RESULT                           (MIU1_REG_BASE+0x1CUL)
#define MIU1_PROTECT0_ID_ENABLE                  (MIU1_REG_BASE+0x20UL)
#define MIU1_PROTECT1_ID_ENABLE                  (MIU1_REG_BASE+0x22UL)
#define MIU1_PROTECT2_ID_ENABLE                  (MIU1_REG_BASE+0x24UL)
#define MIU1_PROTECT3_ID_ENABLE                  (MIU1_REG_BASE+0x26UL)
#define MIU1_PROTECT0_MSB                        (MIU1_REG_BASE+0xD0UL)
#define MIU1_PROTECT0_START                      (MIU1_REG_BASE+0xC0UL)
#define MIU1_PROTECT1_START                      (MIU1_REG_BASE+0xC4UL)
#define MIU1_PROTECT2_START                      (MIU1_REG_BASE+0xC8UL)
#define MIU1_PROTECT3_START                      (MIU1_REG_BASE+0xCCUL)
#define MIU1_RQX_MASK(Reg, Group)                (Reg = (Group < 4 )?  (MIU1_REG_BASE + 0x46UL + 0x20UL*Group) : (MIU1_ARB_REG_BASE + 0x06UL + 0x20UL * (Group - 4)))
#define MIU1_RQX_HPMASK(Reg, Group)              (Reg = (Group < 4 )?  (MIU1_REG_BASE + 0x48UL + 0x20UL*Group) : (MIU1_ARB_REG_BASE + 0x08UL + 0x20UL * (Group - 4)))

//MIU2
#define MIU2_PROTECT_EN                          (MIU2_REG_BASE+0xD2)
#define MIU2_PROTECT_DDR_SIZE                    (MIU2_REG_BASE+0xD3)
#define MIU2_PROTECT0_ID0                        (MIU2_REG_BASE+0x2E)
#define MIU2_BW_REQUEST                          (MIU2_REG_BASE+0x1A)
#define MIU2_BW_RESULT                           (MIU2_REG_BASE+0x1C)
#define MIU2_PROTECT0_ID_ENABLE                  (MIU2_REG_BASE+0x20)
#define MIU2_PROTECT1_ID_ENABLE                  (MIU2_REG_BASE+0x22)
#define MIU2_PROTECT2_ID_ENABLE                  (MIU2_REG_BASE+0x24)
#define MIU2_PROTECT3_ID_ENABLE                  (MIU2_REG_BASE+0x26)
#define MIU2_PROTECT0_MSB                        (MIU2_REG_BASE+0xD0)
#define MIU2_PROTECT0_START                      (MIU2_REG_BASE+0xC0)
#define MIU2_PROTECT1_START                      (MIU2_REG_BASE+0xC4)
#define MIU2_PROTECT2_START                      (MIU2_REG_BASE+0xC8)
#define MIU2_PROTECT3_START                      (MIU2_REG_BASE+0xCC)
#define MIU2_RQX_MASK(Reg, Group)                (Reg = (Group < 4 )?  (MIU2_REG_BASE + 0x46UL + 0x20UL*Group) : (MIU2_ARB_REG_BASE + 0x06UL + 0x20UL * (Group - 4)))
#define MIU2_RQX_HPMASK(Reg, Group)              (Reg = (Group < 4 )?  (MIU2_REG_BASE + 0x48UL + 0x20UL*Group) : (MIU2_ARB_REG_BASE + 0x08UL + 0x20UL * (Group - 4)))


#define REG_MIU_I64_MODE                         (BIT7)
#define REG_MIU_INIT_DONE                        (BIT15)

//Protection Status
#define REG_MIU_PROTECT_LOG_CLR                 (BIT0)
#define REG_MIU_PROTECT_IRQ_MASK                (BIT1)
#define REG_MIU_PROTECT_HIT_FALG                (BIT4)
#define REG_MIU_PROTECT_HIT_ID                  14:8
#define REG_MIU_PROTECT_HIT_NO                  7:5

// MIU Scramble
#define REG_MIU_SCRAMBLE_EN                     (MIU_REG_BASE+0x06UL)

//MIU Bus Width
#define REG_MI64_FORCE                          (CHIP_TOP_BASE+0x40UL)

//-------------------------------------------------------------------------------------------------
//MAU
//
//-------------------------------------------------------------------------------------------------
#define    RIUBASE_MAU0                           0x1840UL
#define    RIUBASE_MAU1                           0x1860UL


//-------------------------------------------------------------------------------------------------
//  MIU ATOP registers
//-------------------------------------------------------------------------------------------------
#define MIU_DDFSTEP                             (0x28UL)//0x110D28
#define MIU_SSC_EN                              (0x29UL)//0x110D29
#define MIU_DDFSPAN                             (0x2AUL)//0x110D2A
#define MIU_DDFSET                              (0x30UL)
#define MIU_PLL_INPUT_DIV_2ND                 (0x34UL) // no this reg in Einstein
#define MIU_PLL_LOOP_DIV_2ND                    (0x34UL)
//xxx_div_first
#define MIU_PLLCTRL                             (0x36UL)
#define MIU_DDRPLL_DIV_FIRST                    (0x37UL)

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------


#endif // _REG_MIU_H_

