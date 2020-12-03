/*
* hal_warp.h- Sigmastar
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
#ifndef _HAL_WARP_H_
#define _HAL_WARP_H_

#include "hal_warp_reg.h"
#include "hal_warp_data.h"

#include <linux/kernel.h>

#define AXI_MAX_WRITE_BURST_SIZE  (16)
#define AXI_MAX_READ_BURST_SIZE   (16)
#define AXI_MAX_WRITE_OUTSTANDING (16)
#define AXI_MAX_READ_OUTSTANDING  (16)

#define DEBUG_BYPASS_DISPLACEMENT_EN    (0)
#define DEBUG_BYPASS_INTERP_EN  (0)

typedef struct
{
    phys_addr_t base_addr;
    warp_hal_reg_bank reg_bank;

} warp_hal_handle;

void warp_hal_set_axi(warp_hal_handle* handle, HAL_WARP_CONFIG* config);
void warp_hal_set_image_point(warp_hal_handle* handle, HAL_WARP_CONFIG* config);
void warp_hal_set_image_size(warp_hal_handle* handle, HAL_WARP_CONFIG* config);
void warp_hal_set_output_tile(warp_hal_handle* handle, u16 tile_w, u16 tile_h);
void warp_hal_set_disp(warp_hal_handle* handle, HAL_WARP_CONFIG* config);
 void warp_hal_set_bb(warp_hal_handle* handle, s32 table_addr);
void warp_hal_set_pers_matirx(warp_hal_handle* handle, HAL_WARP_CONFIG* config);
void warp_hal_set_out_of_range(warp_hal_handle* handle, HAL_WARP_CONFIG* config);
void warp_hal_set_config(warp_hal_handle* handle, HAL_WARP_CONFIG* config);
void warp_hal_get_hw_status(warp_hal_handle* handle);
void warp_hal_init(warp_hal_handle *handle, phys_addr_t base_addr);

#endif //_HAL_WARP_H_
