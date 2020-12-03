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
#ifndef _REG_MIU_H_
#define _REG_MIU_H_

//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

#ifndef BIT
#define BIT(_bit_)                  (1 << (_bit_))
#endif

#define BITS_RANGE(range)           (BIT(((1)?range)+1) - BIT((0)?range))
#define BITS_RANGE_VAL(x, range)    ((x & BITS_RANGE(range)) >> ((0)?range))

#define MIU_REG_BASE                (0x1200UL)
#define MIU1_REG_BASE               (0x0D00UL)
#define MIU2_REG_BASE               (0x62000UL)
#define PM_REG_BASE                 (0x1E00UL)
#define MIU_ATOP_BASE               (0x1000UL)
#define MIU1_ATOP_BASE              (0x0B00UL)
#define MIU2_ATOP_BASE              (0x62100UL)
#define CHIP_TOP_BASE               (0x1E00UL)
#define MIU_ARB_REG_BASE            (0x1100UL)
#define MIU1_ARB_REG_BASE           (0x0C00UL)
#define MIU2_ARB_REG_BASE           (0x62300UL)

#define MIU_PROTECT_EN_INTERNAL     (MIU_REG_BASE+0xD2UL)
#define MIU_PROTECT_DDR_SIZE        (MIU_REG_BASE+0xD3UL)
#define MIU_PROTECT_DDR_SIZE_MASK   BITS_RANGE(11:8)
#define MIU_PROTECT_DDR_32MB        (0x50UL)
#define MIU_PROTECT_DDR_64MB        (0x60UL)
#define MIU_PROTECT_DDR_128MB       (0x70UL)
#define MIU_PROTECT_DDR_256MB       (0x80UL)
#define MIU_PROTECT_DDR_512MB       (0x90UL)
#define MIU_PROTECT_DDR_1024MB      (0xA0UL)
#define MIU_PROTECT_DDR_2048MB      (0xB0UL)

#define MIU_PROTECT0_ID0            (MIU_REG_BASE+0x2EUL)
#define MIU_PROTECT0_ID_ENABLE      (MIU_REG_BASE+0x20UL)
#define MIU_PROTECT1_ID_ENABLE      (MIU_REG_BASE+0x22UL)
#define MIU_PROTECT2_ID_ENABLE      (MIU_REG_BASE+0x24UL)
#define MIU_PROTECT3_ID_ENABLE      (MIU_REG_BASE+0x26UL)
#define MIU_PROTECT0_MSB            (MIU_REG_BASE+0xD0UL)
#define MIU_PROTECT0_START          (MIU_REG_BASE+0xC0UL)
#define MIU_PROTECT1_START          (MIU_REG_BASE+0xC4UL)
#define MIU_PROTECT2_START          (MIU_REG_BASE+0xC8UL)
#define MIU_PROTECT3_START          (MIU_REG_BASE+0xCCUL)
#define REG_MIU_PROTECT_LOADDR      (0x6DUL << 1)   //0xDE
#define REG_MIU_PROTECT_HIADDR      (0x6EUL << 1)   //0xDE
#define REG_MIU_PROTECT_STATUS      (0x6FUL << 1)   //0xDE

// MIU selection registers
#define REG_MIU_SEL0                (MIU_REG_BASE+0xf0UL)  //0x12F0
#define REG_MIU_SEL1                (MIU_REG_BASE+0xf2UL)  //0x12F1
#define REG_MIU_SEL2                (MIU_REG_BASE+0xf4UL)  //0x12F2
#define REG_MIU_SEL3                (MIU_REG_BASE+0xf6UL)  //0x12F3
#define REG_MIU_SELX(x)             (0xF0UL+x*2)

// MIU1
#define MIU1_PROTECT_EN             (MIU1_REG_BASE+0xD2UL)
#define MIU1_PROTECT_DDR_SIZE       (MIU1_REG_BASE+0xD3UL)
#define MIU1_PROTECT0_ID0           (MIU1_REG_BASE+0x2EUL)
#define MIU1_PROTECT0_ID_ENABLE     (MIU1_REG_BASE+0x20UL)
#define MIU1_PROTECT1_ID_ENABLE     (MIU1_REG_BASE+0x22UL)
#define MIU1_PROTECT2_ID_ENABLE     (MIU1_REG_BASE+0x24UL)
#define MIU1_PROTECT3_ID_ENABLE     (MIU1_REG_BASE+0x26UL)
#define MIU1_PROTECT0_MSB           (MIU1_REG_BASE+0xD0UL)
#define MIU1_PROTECT0_START         (MIU1_REG_BASE+0xC0UL)
#define MIU1_PROTECT1_START         (MIU1_REG_BASE+0xC4UL)
#define MIU1_PROTECT2_START         (MIU1_REG_BASE+0xC8UL)
#define MIU1_PROTECT3_START         (MIU1_REG_BASE+0xCCUL)

#define REG_MIU_I64_MODE            (BIT7)
#define REG_MIU_INIT_DONE           (BIT15)

// Protection Status
#define REG_MIU_PROTECT_LOG_CLR     (BIT0)
#define REG_MIU_PROTECT_IRQ_MASK    (BIT1)
#define REG_MIU_PROTECT_HIT_FALG    (BIT4)
#define REG_MIU_PROTECT_HIT_ID      14:8
#define REG_MIU_PROTECT_HIT_NO      7:5

#endif // _REG_MIU_H_

