/*
* voltage_ctrl.h- Sigmastar
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
#ifndef __VOLTAGE_CTRL_H
#define __VOLTAGE_CTRL_H

#include "voltage_ctrl_demander.h"

#define VOLTAGE_CORE_850   850
#define VOLTAGE_CORE_900   900
#define VOLTAGE_CORE_950   950
#define VOLTAGE_CORE_1000 1000


void set_core_voltage(VOLTAGE_DEMANDER_E demander, int mV);
int  get_core_voltage(void);
int  core_voltage_available(unsigned int **voltages, unsigned int *num);
int  core_voltage_pin(unsigned int **pins, unsigned int *num);

#endif  //__VOLTAGE_CTRL_H
