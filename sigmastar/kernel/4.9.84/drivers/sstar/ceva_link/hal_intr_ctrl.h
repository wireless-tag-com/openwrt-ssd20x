/*
* hal_intr_ctrl.h- Sigmastar
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
#ifndef HAL_INTR_CTRL_1_H
#define HAL_INTR_CTRL_1_H

#include "hal_intr_ctrl_reg.h"

typedef enum {
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_0_TO_HOST_3 = 0x00000010, // 36
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_0_TO_HOST_2 = 0x00000020, // 37
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_0_TO_HOST_1 = 0x00000040, // 38
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_0_TO_HOST_4 = 0x00000080, // 39
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_1_TO_HOST_3 = 0x00000100, // 40
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_1_TO_HOST_2 = 0x00000200, // 41
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_1_TO_HOST_0 = 0x00000400, // 42
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_1_TO_HOST_4 = 0x00000800, // 43
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_2_TO_HOST_3 = 0x00001000, // 44
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_2_TO_HOST_1 = 0x00002000, // 45
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_2_TO_HOST_0 = 0x00004000, // 46
    /*
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_2_TO_HOST_4 = 0x00008000, // 47
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_3_TO_HOST_2 = 0x00010000, // 48
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_3_TO_HOST_1 = 0x00020000, // 49
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_3_TO_HOST_0 = 0x00040000, // 50
    INTR_CTRL_HAL_SOFT_INTERRUPT_HOST_3_TO_HOST_4 = 0x00080000, // 51
    */
} INTR_CTRL_HAL_SOFT_INTERRUPT;

typedef struct
{
    phys_addr_t base_addr;
    int_ctl_hal_reg register;
} intr_ctrl_hal_handle;

void intr_ctrl_hal_init(intr_ctrl_hal_handle *handle, phys_addr_t base_addr);
void intr_ctrl_hal_software_interrupt(intr_ctrl_hal_handle *handle, INTR_CTRL_HAL_SOFT_INTERRUPT interrupt);

#endif // HAL_INTR_CTRL_1_H
