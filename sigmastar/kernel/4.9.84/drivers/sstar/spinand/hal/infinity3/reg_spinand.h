/*
* reg_spinand.h- Sigmastar
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
#ifndef _REG_SPINAND_H_
#define _REG_SPINAND_H_


//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------
// !!! Uranus Serial Flash Notes: !!!
//  - The clock of DMA & Read via XIU operations must be < 3*CPU clock
//  - The clock of DMA & Read via XIU operations are determined by only REG_ISP_CLK_SRC; other operations by REG_ISP_CLK_SRC only
//  - DMA program can't run on DRAM, but in flash ONLY
//  - DMA from SPI to DRAM => size/DRAM start/DRAM end must be 8-B aligned


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

// BASEADDR & BK
//------------------------------
#define I3_RIU_PM_BASE                      0xFD000000
#define I3_RIU_BASE                         0xFD200000

#define BK_FSP                              0x2C00
#define BK_QSPI                             0x2E00
#define BK_PMSLP                            0x1C00
#define BK_CLK0                             0x1600
#define BK_BDMA                             0x0400
#define BK_CHIP                             0x3C00
#define FSP_REG_BASE_ADDR                   GET_REG_ADDR(I3_RIU_BASE, BK_FSP)
#define QSPI_REG_BASE_ADDR                  GET_REG_ADDR(I3_RIU_BASE, BK_QSPI)
#define PMSLP_REG_BASE_ADDR                 GET_REG_ADDR(I3_RIU_BASE, BK_PMSLP)
#define CLK0_REG_BASE_ADDR                  GET_REG_ADDR(I3_RIU_BASE, BK_CLK0)
#define BDMA_REG_BASE_ADDR                  GET_REG_ADDR(I3_RIU_BASE, BK_BDMA)
#define CHIP_REG_BASE_ADDR                  GET_REG_ADDR(I3_RIU_BASE, BK_CHIP)

//----- Chip flash -------------------------
//	#include "mhal_chiptop_reg.h"

// CLK_GEN0
#define REG_CLK0_CKG_SPI            0x16
#define CLK0_CKG_SPI_MASK           BMASK(4:2)
#define CLK0_CKG_SPI_XTALI              BITS(4:2, 0)
#define CLK0_CKG_SPI_27MHZ              BITS(4:2, 1)
#define CLK0_CKG_SPI_36MHZ              BITS(4:2, 2)
#define CLK0_CKG_SPI_43MHZ              BITS(4:2, 3)
#define CLK0_CKG_SPI_54MHZ              BITS(4:2, 4)
#define CLK0_CKG_SPI_72MHZ              BITS(4:2, 5)
#define CLK0_CKG_SPI_86MHZ              BITS(4:2, 6)
#define CLK0_CKG_SPI_108MHZ             BITS(4:2, 7)
#define CLK0_CLK_SWITCH_MASK        BMASK(5:5)
#define CLK0_CLK_SWITCH_OFF           BITS(5:5, 0)
#define CLK0_CLK_SWITCH_ON            BITS(5:5, 1)

//CHIP_TOP
#define REG_CHIPTOP_DUMMY3          0x1F
#define CHIP_CS_PAD1                0x100
// PM_SLEEP CMD.
#define REG_PM_CKG_SPI              0x20 // Ref spec. before using these setting.
#define PM_SPI_CLK_SEL_MASK         BMASK(13:10)
#define PM_SPI_CLK_XTALI                BITS(13:10, 0)
#define PM_SPI_CLK_27MHZ                BITS(13:10, 1)
#define PM_SPI_CLK_36MHZ                BITS(13:10, 2)
#define PM_SPI_CLK_43MHZ                BITS(13:10, 3)
#define PM_SPI_CLK_54MHZ                BITS(13:10, 4)
#define PM_SPI_CLK_72MHZ                BITS(13:10, 5)
#define PM_SPI_CLK_86MHZ                BITS(13:10, 6)
#define PM_SPI_CLK_108MHZ               BITS(13:10, 7)
#define PM_SPI_CLK_24MHZ                BITS(13:10, 15)
#define PM_SPI_CLK_SWITCH_MASK      BMASK(14:14)
#define PM_SPI_CLK_SWITCH_OFF           BITS(14:14, 0)
#define PM_SPI_CLK_SWITCH_ON            BITS(14:14, 1)

#define CHK_NUM_WAITDONE     20000

// FSP Register
#define REG_FSP_WRITE_BUFF              0x60*2
#define REG8_FSP_COMMAND       0x00
#define REG8_FSP_COMMAND1      0x01
#define REG8_FSP_RADDR_L       0x03
#define REG8_FSP_RADRR_M       0x02
#define REG8_FSP_RADDR_H       0x01
#define REG8_FSP_WADDR_L       0x04
#define REG8_FSP_WADDR_M       0x03
#define REG8_FSP_WADDR_H       0x02

#define REG8_FSP_WDATA         0x05
#define REG8_FSP_WDATA10       0x10

#define REG_FSP_QUAD_MODE			    0x75*2
#define REG_FSP_READ_BUFF                   0x65*2
#define REG_FSP_WRITE_SIZE                  0x6A*2
#define REG_FSP_READ_SIZE                   0x6B*2
#define REG_FSP_CTRL                        0x6C*2
#define REG_FSP_TRIGGER                     0x6D*2
#define TRIGGER_FSP            1
#define REG_FSP_DONE                        0x6E*2
#define DONE_FSP               1
#define REG_FSP_AUTO_CHECK_ERROR            0x6E*2
#define AUTOCHECK_ERROR_FSP    2
#define REG_FSP_CLEAR_DONE                  0x6F*2
#define CLEAR_DONE_FSP         1
#define REG_FSP_WRITE_BUFF2                 0x70*2
#define FSP_WRITE_BUF_JUMP_OFFSET 0x0A
#define REG_FSP_CTRL2                       0x75*2
#define REG_FSP_CTRL3                       0x75*2
#define REG_FSP_CTRL4                       0x76*2
#define REG_FSP_WDATA              0x00
#define REG_FSP_WBF_SIZE_OUTSIDE            0x78*2
#define REG_FSP_WBF_OUTSIDE                 0x79*2


#define REG_FSP_RDATA          0x05
#define MAX_WRITE_BUF_CNT      17
#define SINGLE_WRITE_SIZE      15

#define REG_FSP_RSIZE              0x0B
#define MAX_READ_BUF_CNT       10

#define ENABLE_FSP_QUAD           1
#define ENABLE_FSP             1
#define RESET_FSP              2
#define INT_FSP                4
#define AUTOCHECK_FSP          8
#define ENABLE_SEC_CMD         0x8000
#define ENABLE_THR_CMD         0x4000
#define ENCMD2_3               0xF000
#define FLASH_BUSY_BIT             1
#define FLASH_WRITE_ENANLE         2
#define FLASH_BUSY_BIT_EREASE      3
// writet protect register
#define WRITE_PROTECT_ENABLE       0xFC
#define WRITE_PROTECT_DISABLE      0x00

//QSPI Register
#define REG_SPI_BURST_WRITE                 0x0A
#define REG_SPI_DISABLE_BURST 0x02
#define REG_SPI_ENABLE_BURST  0x01
#define REG_SPI_WRAP_VAL                    0x54
#define REG_SPI_WRAP_BIT_OFFSET 0x8
#define REG_SPI_WRAP_BIT_MASK   0xF
#define REG_SPI_CKG_SPI                     0x70
#define REG_SPI_USER_DUMMY_EN       0x10
#define REG_SPI_DUMMY_CYC_SINGLE    0x07
#define REG_SPI_DUMMY_CYC_DUAL      0x03
#define REG_SPI_DUMMY_CYC_QUAD      0x01
#define REG_SPI_MODE_SEL                    0x72
#define REG_SPI_NORMAL_MODE         0x00
#define REG_SPI_FAST_READ           0x01
#define REG_SPI_CMD_3B              0x02
#define REG_SPI_CMD_BB              0x03
#define REG_SPI_CMD_6B              0x0A
#define REG_SPI_CMD_EB              0x0B
#define REG_SPI_CMD_0B              0x0C
#define REG_SPI_CMD_4EB             0x0D
#define REG_SPI_FUNC_SET                    0x7D
#define REG_SPI_ADDR2_EN    0x800
#define REG_SPI_DUMMY_EN    0x1000
//only for two plane nand
#define REG_SPI_WRAP_EN     0x2000

#define MASK(x)                     (((1<<(x##_BITS))-1) << x##_SHIFT)
#define _BIT(x)                      (1<<(x))

#define BMASK(bits)                 (_BIT(((1)?bits)+1)-_BIT(((0)?bits)))
#define BITS(bits,value)            ((_BIT(((1)?bits)+1)-_BIT(((0)?bits))) & (value<<((0)?bits)))

#define REG_PM_SPI_IS_GPIO          0x35
#define PM_SPI_HOLD_GPIO_MASK            BMASK(14:14)
#define PM_SPI_HOLD_IS_GPIO                  BITS(14:14, 1)
#define PM_SPI_HOLD_NOT_GPIO                 BITS(14:14, 0)
#define PM_SPI_WP_GPIO_MASK            BMASK(7:7)
#define PM_SPI_WP_IS_GPIO                  BITS(7:7, 1)
#define PM_SPI_WP_NOT_GPIO                 BITS(7:7, 0)

#endif // _REG_SPINAND_H_
