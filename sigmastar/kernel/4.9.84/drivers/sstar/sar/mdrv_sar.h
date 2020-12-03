/*
* mdrv_sar.h- Sigmastar
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
#ifndef __MDRV_SAR_H
#define __MDRV_SAR_H

//#include <mach/ms_types.h>
//#include "mach/platform.h"
#include "../include/ms_types.h"
#include "../include/ms_platform.h"
#include "mdrv_sar_io.h"

#define DEVICE_NAME    "sar"

#define BASE_REG_RIU_PA     (0x1F000000)
//#define BASE_REG_WDT_PA             GET_REG_ADDR(BASE_REG_RIU_PA, 0x001800)
//#define BASE_REG_PM_PA             GET_REG_ADDR(CHICAGO_BASE_REG_RIU_PA, 0x000700)
#define BK_REG(reg)             ((reg) << 2)

static int channel = 1;
//static int mode = 0;

#define SARADC_IOC_MAXNR               2

#define REG_SAR_CTRL0               0x00*2
#define REG_SAR_CKSAMP_PRD         0x01*2
#define REG_SAR_GCR_SAR_CH8        0x02*2
#define REG_SAR_AISEL_CTRL         0x11*2
#define REG_SAR_GPIO_CTRL          0x12*2
#define REG_SAR_INT_MASK            0x14*2
#define REG_SAR_INT_CLR             0x15*2
#define REG_SAR_INT_FORCE           0x16*2
#define REG_SAR_INT_STATUS          0x17*2
#define REG_SAR_CMP_OUT_RDY         0x18*2
#define REG_SAR_CH_REF_V_SEL        0x19*2
#define REG_SAR_CH1_UPB             0x20*2
#define REG_SAR_CH2_UPB             0x21*2
#define REG_SAR_CH3_UPB             0x22*2
#define REG_SAR_CH4_UPB             0x23*2
#define REG_SAR_CH5_UPB             0x24*2
#define REG_SAR_CH6_UPB             0x25*2
#define REG_SAR_CH7_UPB             0x26*2
#define REG_SAR_CH8_UPB             0x27*2
#define REG_SAR_CH1_LOB             0x30*2
#define REG_SAR_CH2_LOB             0x31*2
#define REG_SAR_CH3_LOB             0x32*2
#define REG_SAR_CH4_LOB             0x33*2
#define REG_SAR_CH5_LOB             0x34*2
#define REG_SAR_CH6_LOB             0x35*2
#define REG_SAR_CH7_LOB             0x36*2
#define REG_SAR_CH8_LOB             0x37*2
#define REG_SAR_CH1_DATA            0x40*2
#define REG_SAR_CH2_DATA            0x41*2
#define REG_SAR_CH3_DATA            0x42*2
#define REG_SAR_CH4_DATA            0x43*2
#define REG_SAR_CH5_DATA            0x44*2
#define REG_SAR_CH6_DATA            0x45*2
#define REG_SAR_CH7_DATA            0x46*2
#define REG_SAR_CH8_DATA            0x47*2
#define REG_SAR_SMCARD_CTRL         0x50*2
#define REG_SAR_FCIE_INT_CTRL       0x51*2
#define REG_SAR_INT_DIRECT2TOP_SEL  0x60*2

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
