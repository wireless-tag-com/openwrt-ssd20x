/*
* mdrv_spinand_dev.c- Sigmastar
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
#include "mdrv_spinand_dev.h"

SPI_NAND_WP_t _pstWriteProtectTable_W25N01GV[] =
{
    //      BPX             Mask       Lower Bound     Upper Bound
    {   BITS(6:2,  0),   BMASK(6:2),   0x00000000,     0x00000000  },
    {   BITS(6:2,  2),   BMASK(6:2),   0x07FC0000,     0x07FFFFFF  },
    {   BITS(6:2,  4),   BMASK(6:2),   0x07F80000,     0x07FFFFFF  },
    {   BITS(6:2,  6),   BMASK(6:2),   0x07F00000,     0x07FFFFFF  },
    {   BITS(6:2,  8),   BMASK(6:2),   0x07E00000,     0x07FFFFFF  },
    {   BITS(6:2, 10),   BMASK(6:2),   0x07C00000,     0x07FFFFFF  },
    {   BITS(6:2, 12),   BMASK(6:2),   0x07800000,     0x07FFFFFF  },
    {   BITS(6:2, 14),   BMASK(6:2),   0x07000000,     0x07FFFFFF  },
    {   BITS(6:2, 16),   BMASK(6:2),   0x06000000,     0x07FFFFFF  },
    {   BITS(6:2, 18),   BMASK(6:2),   0x04000000,     0x07FFFFFF  },
    {   BITS(6:2,  3),   BMASK(6:2),   0x00000000,     0x0003FFFF  },
    {   BITS(6:2,  5),   BMASK(6:2),   0x00000000,     0x0007FFFF  },
    {   BITS(6:2,  7),   BMASK(6:2),   0x00000000,     0x000FFFFF  },
    {   BITS(6:2,  9),   BMASK(6:2),   0x00000000,     0x001FFFFF  },
    {   BITS(6:2, 11),   BMASK(6:2),   0x00000000,     0x003FFFFF  },
    {   BITS(6:2, 13),   BMASK(6:2),   0x00000000,     0x007FFFFF  },
    {   BITS(6:2, 15),   BMASK(6:2),   0x00000000,     0x00FFFFFF  },
    {   BITS(6:2, 17),   BMASK(6:2),   0x00000000,     0x01FFFFFF  },
    {   BITS(6:2, 19),   BMASK(6:2),   0x00000000,     0x03FFFFFF  },
    {   BITS(6:2, 21),   BMASK(6:2),   0x00000000,     0x07FFFFFF  },
    {   BITS(6:2,  0),   BMASK(6:2),   0xFFFFFFFF,     0xFFFFFFFF  },
};

SPI_NAND_DEVICE_t _gtSpinandWpTable[] =
{
    {{0xEF, 0xAA, 0x21}, _pstWriteProtectTable_W25N01GV},

    {{0x00, 0x00, 0x00}, NULL},
};

