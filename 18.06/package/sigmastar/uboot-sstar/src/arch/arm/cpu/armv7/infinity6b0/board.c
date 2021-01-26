/*
* board.c- Sigmastar
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

#include <common.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	/* arch number*/
	gd->bd->bi_arch_number = MACH_TYPE_INFINITY6B0;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = BOOT_PARAMS;

    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}
