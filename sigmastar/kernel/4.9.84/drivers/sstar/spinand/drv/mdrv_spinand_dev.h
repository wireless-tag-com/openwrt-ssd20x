/*
* mdrv_spinand_dev.h- Sigmastar
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
#ifndef __MDRV_SPINAND_DEV_H__
#define __MDRV_SPINAND_DEV_H__

#include "inc/mdrv_spinand_common.h"

//Bit operation
#ifndef BIT
#define BIT(_bit_)                  (1 << (_bit_))
#endif
#define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
#define BMASK(_bits_)               (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))

typedef struct _SPI_NAND_WP
{
    U8   u8BlockProtectBits;  // Block Protection Bits
    U8   u8BlockProtectMask;  // Block Protection Mask
    U32  u32LowerBound;
    U32  u32UpperBound;
} SPI_NAND_WP_t;

typedef struct _SPI_NAND_DEVICE
{
    U8 pu8MID[3];
    SPI_NAND_WP_t *pWriteProtectTable;
} SPI_NAND_DEVICE_t;

extern SPI_NAND_DEVICE_t _gtSpinandWpTable[];

#endif

