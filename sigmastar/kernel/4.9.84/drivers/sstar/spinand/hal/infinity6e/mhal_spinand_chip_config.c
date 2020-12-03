/*
* mhal_spinand_chip_config.c- Sigmastar
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
#include "mhal_spinand.h"
#include "reg_spinand.h"

extern hal_fsp_t _hal_fsp;

//check Bounding Type bank 0x101E , offset 0x48 bit[5:4]
#define CHIP_BOUND_TYPES            (0x30)
#define CHIP_BOUND_QFN88            0x00//00: QFN88
#define CHIP_BOUND_QFN128           (0x10)//01: QFN128
#define CHIP_BOUND_BGA1             (0x20)//10: BGA1
#define CHIP_BOUND_BGA2A2B          (0x30)//11: BGA2A & BGA2B

void HAL_SPINAND_Chip_Config(void)
{
    U8 u8BoundID;

    //chekc boundID to select pm-spi or non-pm spi
    u8BoundID = CHIP_READ(0x48) & CHIP_BOUND_TYPES;
    switch(u8BoundID)
    {
        case CHIP_BOUND_QFN88:
            _hal_fsp.u32FspBaseAddr = I3_RIU_PM_BASE + BK_PM_FSP;
            _hal_fsp.u32QspiBaseAddr = I3_RIU_PM_BASE + BK_PM_QSPI;
            break;
        default:
            _hal_fsp.u32FspBaseAddr = I3_RIU_BASE + BK_FSP;
            _hal_fsp.u32QspiBaseAddr = I3_RIU_BASE + BK_QSPI;
            break;
    }

}
