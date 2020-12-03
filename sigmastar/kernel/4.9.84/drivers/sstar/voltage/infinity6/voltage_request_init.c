/*
* voltage_request_init.c- Sigmastar
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
#include <linux/kernel.h>
#include "registers.h"
#include "ms_platform.h"
#include "voltage_ctrl.h"
#include "voltage_ctrl_demander.h"

void voltage_request_miu(void)
{
    unsigned int miupll_freq_Mhz = 0;
    unsigned int miu_data_rate = 0;

    // Check MIUPLL and determine core voltage request
    miupll_freq_Mhz = 24 * INREGMSK16(BASE_REG_MIUPLL_PA + REG_ID_03, 0x00FF) / ((INREGMSK16(BASE_REG_MIUPLL_PA + REG_ID_03, 0x0700) >> 8) + 2);
    miu_data_rate = 1 << (INREGMSK16(BASE_REG_MIU_PA + REG_ID_01, 0x0300) >> 8);
    if (miupll_freq_Mhz > 480 || (miu_data_rate <=4 && miupll_freq_Mhz > 240))
        set_core_voltage(VOLTAGE_DEMANDER_MIU, VOLTAGE_CORE_1000);
    else
        set_core_voltage(VOLTAGE_DEMANDER_MIU, VOLTAGE_CORE_900);

    return;
}

int voltage_request_chip(void)
{
    voltage_request_miu();

    return 0;
}
