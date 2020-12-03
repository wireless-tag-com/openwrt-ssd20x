/*
* voltage_ctrl_demander.h- Sigmastar
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
#ifndef __VOLTAGE_CTRL_DEMANDER_H
#define __VOLTAGE_CTRL_DEMANDER_H

#define FOREACH_DEMANDER(DEMANDER) \
            DEMANDER(VOLTAGE_DEMANDER_CPUFREQ)      \
            DEMANDER(VOLTAGE_DEMANDER_TEMPERATURE)  \
            DEMANDER(VOLTAGE_DEMANDER_VENC)         \
            DEMANDER(VOLTAGE_DEMANDER_MIU)          \
            DEMANDER(VOLTAGE_DEMANDER_USER)         \
            DEMANDER(VOLTAGE_DEMANDER_MAX)          \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum {
    FOREACH_DEMANDER(GENERATE_ENUM)
} VOLTAGE_DEMANDER_E;

#endif  //__VOLTAGE_CTRL_DEMANDER_H
