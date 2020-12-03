/*
* hal_intr_ctrl_reg.h- Sigmastar
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
#ifndef HAL_INTR_CTRL_1_REG_H
#define HAL_INTR_CTRL_1_REG_H

#include <linux/kernel.h>

typedef struct
{
    union
    {
        struct
        {
            u16 reg_hst1_fiq_force_47_32:16;
        };
        u16 reg22;
    };

} int_ctl_hal_reg;
#endif
#endif //HAL_INTR_CTRL_1_REG_H