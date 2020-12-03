/*
* hal_card_platform_config.h- Sigmastar
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

//
#include "gpio.h"

//
#define IP_0_TYPE   IP_TYPE_SDIO
#define IP_1_TYPE   IP_TYPE_NONE
#define IP_2_TYPE   IP_TYPE_NONE

/* --------------------------------------------------------
< WT_POWERUP >
SD Spec:
- This delay should be sufficient to allow the power supply to reach the minimum voltage.
HW measure:
- About 5x us is enough.

< WT_POWERON >
SD Spec:
- This delay must be at least 74 clock sizes, or 1 ms, or the time required to reach a stable voltage.

< WT_POWEROFF >
SD Spec:
- the card VDD shall be once lowered to less than 0.5Volt for a minimum period of 1ms.
HW measure:
- SD_3V3 has 2K resistance to gnd: 30 ms.
- SD_3V3 does Not have any resistance to gnd: 1500 ms.
-------------------------------------------------------- */
#define WT_POWERUP                  1    //(ms)
#define WT_POWERON                  1    //(ms)
#define WT_POWEROFF                 30   //(ms) Here is only for default, real value will be from DTS.

#define WT_EVENT_RSP                10      //(ms)
#define WT_EVENT_READ               2000    //(ms)
#define WT_EVENT_WRITE              3000    //(ms)

#define DEF_CDZ_PAD_SLOT0           (PAD_PM_SD_CDZ)
#define DEF_CDZ_PAD_SLOT1           0
#define DEF_CDZ_PAD_SLOT2           0

#define DEF_PWR_PAD_SLOT0           (PAD_TTL0)
#define DEF_PWR_PAD_SLOT1           0
#define DEF_PWR_PAD_SLOT2           0

#define EN_MSYS_REQ_DMEM            (FALSE)

