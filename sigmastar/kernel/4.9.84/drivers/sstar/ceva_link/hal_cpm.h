/*
* hal_cpm.h- Sigmastar
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
#ifndef _HAL_CMP_H_
#define _HAL_CMP_H_

#include <linux/kernel.h>

typedef struct
{
    phys_addr_t base_addr;
} cpm_hal_handle;

void cmp_hal_init(cpm_hal_handle *handle, phys_addr_t base_addr);
void cmp_hal_mcci_write(cpm_hal_handle *handle, u32 index, u32 value);
u16 cmp_hal_mcci_read(cpm_hal_handle *handle, u32 index);

#endif // _HAL_CMP_H_
