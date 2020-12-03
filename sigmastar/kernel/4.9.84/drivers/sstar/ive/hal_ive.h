/*
* hal_ive.h- Sigmastar
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
#ifndef _HAL_IVE_H_
#define _HAL_IVE_H_

#include <linux/kernel.h>
#include "hal_ive_reg.h"
#include "mdrv_ive_io_st.h"

typedef enum
{
    IVE_HAL_IRQ_MASK_FRAME_DONE                     = 0x00000001,   // bit 0, frame done
    IVE_HAL_IRQ_MASK_Y_CNT_HIT_0                    = 0x00000002,   // bit 1, Y line counter 0 hit
    IVE_HAL_IRQ_MASK_Y_CNT_HIT_1                    = 0x00000004,   // bit 2, Y line counter 1 hit
    IVE_HAL_IRQ_MASK_Y_CNT_HIT_2                    = 0x00000008,   // bit 3, Y line counter 2 hit
    IVE_HAL_IRQ_MASK_ALL                            = 0xF000000F    // ALL bits
} IVE_HAL_IRQ_MASK;

typedef struct
{
    phys_addr_t base_addr0;
    ive_hal_reg_bank0 reg_bank0;

    phys_addr_t base_addr1;
    ive_hal_reg_bank1 reg_bank1;
} ive_hal_handle;

void ive_hal_init(ive_hal_handle *handle, phys_addr_t base_addr0, phys_addr_t base_addr1);

void ive_hal_set_irq_mask(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask);
void ive_hal_clear_irq(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask);
IVE_HAL_IRQ_MASK ive_hal_get_irq_check(ive_hal_handle *handle, IVE_HAL_IRQ_MASK mask);
u16 ive_hal_get_irq(ive_hal_handle *handle);

void ive_hal_set_operation(ive_hal_handle *handle, IVE_IOC_OP_TYPE op_type);
void ive_hal_set_images(ive_hal_handle *handle, ive_ioc_image *input, ive_ioc_image *output);

void ive_hal_set_coeff_filter(ive_hal_handle *handle, ive_ioc_coeff_filter *coeff);
void ive_hal_set_coeff_csc(ive_hal_handle *handle, ive_ioc_coeff_csc *coeff);
void ive_hal_set_coeff_sobel(ive_hal_handle *handle, ive_ioc_coeff_sobel *coeff);
void ive_hal_set_coeff_mag_and_ang(ive_hal_handle *handle, ive_ioc_coeff_mag_and_ang *coeff);
void ive_hal_set_coeff_ord_stat_filter(ive_hal_handle *handle, ive_ioc_coeff_ord_stat_filter *coeff);
void ive_hal_set_coeff_bernsen(ive_hal_handle *handle, ive_ioc_coeff_bernsen *coeff);
void ive_hal_set_coeff_dilate(ive_hal_handle *handle, ive_ioc_coeff_dilate *coeff);
void ive_hal_set_coeff_erode(ive_hal_handle *handle, ive_ioc_coeff_erode *coeff);
void ive_hal_set_coeff_thresh(ive_hal_handle *handle, ive_ioc_coeff_thresh *coeff);
void ive_hal_set_coeff_thresh_s16(ive_hal_handle *handle, ive_ioc_coeff_thresh_s16 *coeff);
void ive_hal_set_coeff_thresh_u16(ive_hal_handle *handle, ive_ioc_coeff_thresh_u16 *coeff);
void ive_hal_set_coeff_add(ive_hal_handle *handle, ive_ioc_coeff_add *coeff);
void ive_hal_set_coeff_sub(ive_hal_handle *handle, ive_ioc_coeff_sub *coeff);
void ive_hal_set_coeff_16to8(ive_hal_handle *handle, ive_ioc_coeff_16to8 *coeff);
void ive_hal_set_coeff_map(ive_hal_handle *handle, u64 map_addr);
void ive_hal_set_coeff_integral(ive_hal_handle *handle, ive_ioc_coeff_integral *coeff);
void ive_hal_set_coeff_sad(ive_hal_handle *handle, ive_ioc_coeff_sad *coeff);
void ive_hal_set_coeff_ncc(ive_hal_handle *handle, u64 output_addr);
void ive_hal_set_coeff_lbp(ive_hal_handle *handle, ive_ioc_coeff_lbp *coeff);
void ive_hal_set_coeff_bat(ive_hal_handle *handle, ive_ioc_coeff_bat *coeff);
void ive_hal_set_coeff_adp_thresh(ive_hal_handle *handle, ive_ioc_coeff_adp_thresh *coeff);
void ive_hal_set_coeff_matrix_transform(ive_hal_handle *handle, ive_ioc_coeff_matrix_transform *coeff);
void ive_hal_set_coeff_image_dot(ive_hal_handle *handle, ive_ioc_coeff_image_dot *coeff);

void ive_hal_start(ive_hal_handle *handle);
void ive_hal_sw_reset(ive_hal_handle *handle);

void ive_hal_reg_dump(ive_hal_handle *handle);

void ive_hal_miu_set(ive_hal_handle *handle, int miu_state);

#endif // _HAL_IVE_H_
