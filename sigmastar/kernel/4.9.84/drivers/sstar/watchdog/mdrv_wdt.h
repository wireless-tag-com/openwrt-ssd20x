/*
* mdrv_wdt.h- Sigmastar
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
#ifndef __MDRV_WDT_H
#define __MDRV_WDT_H

//#include <mach/ms_types.h>
//#include "mach/platform.h"
#include "../include/ms_types.h"
#include "../include/ms_platform.h"

#define BASE_REG_RIU_PA     (0x1F000000)
#define BASE_REG_WDT_PA             GET_REG_ADDR(BASE_REG_RIU_PA, 0x001800)
//#define BASE_REG_PM_PA             GET_REG_ADDR(CHICAGO_BASE_REG_RIU_PA, 0x000700)
#define BK_REG(reg)             ((reg) << 2)

#define WDT_WDT_CLR             BK_REG(0x00)
#define WDT_DUMMY_REG_1         BK_REG(0x01)
#define WDT_RST_RSTLEN          BK_REG(0x02)
#define WDT_INTR_PERIOD         BK_REG(0x03)
#define WDT_MAX_PRD_L           BK_REG(0x04)
#define WDT_MAX_PRD_H           BK_REG(0x05)


#define CLEAR_WDT   0x1

#define OSC_CLK_26000000 26000000
#define OSC_CLK_24000000 24000000
#define OSC_CLK_16369000 16369000
#define OSC_CLK_12000000 12000000
#define OSC_CLK_4000000   4000000

// Chip revisions
#define REVISION_U01 (0x0)
#define REVISION_U02 (0x1)
#define REVISION_U03 (0x2)
#define REVISION_U04 (0x3)
#define REVISION_UNKNOWN (0x4)

#define BIT_0	0x1
#define BIT_1	0x2
#define BIT_2	0x4
#define BIT_3	0x8
#define BIT_4	0x10
#define BIT_5	0x20
#define BIT_6	0x40
#define BIT_7	0x80
#define BIT_8	0x100
#define BIT_9	0x200
#define BIT_10	0x400
#define BIT_11	0x800
#define BIT_12	0x1000
#define BIT_13	0x2000
#define BIT_14	0x4000
#define BIT_15	0x8000

#endif
