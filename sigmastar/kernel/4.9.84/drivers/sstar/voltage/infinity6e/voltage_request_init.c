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

int voltage_request_chip(void)
{
    //TODO: need to check MIU 4x/8x and MIUPLL clock for giving voltage in MIU point of view
    //FIXME: In bring up stage, giving 1.0v to user demander for adjusting easily later by
    //       echo [850/900/950/1000] > /sys/devices/system/voltage/core/voltage_current
    set_core_voltage(VOLTAGE_DEMANDER_USER, VOLTAGE_CORE_1000);
    return 0;
}
