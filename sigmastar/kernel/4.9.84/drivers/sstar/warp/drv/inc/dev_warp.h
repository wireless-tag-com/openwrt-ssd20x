/*
* dev_warp.h- Sigmastar
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
#ifndef _DEV_WARP_H_
#define _DEV_WARP_H_

#include <linux/kernel.h>

typedef enum
{
    WARP_DEV_STATE_READY        = 0,
    WARP_DEV_STATE_PROCESSING   = 1,
    WARP_DEV_STATE_DONE         = 2,
    WARP_DEV_STATE_AXI_ERROR    = 3
} WARP_DEV_STATE;

typedef enum
{
    WARP_DEV_PROC_STATE_OK                          = 0x0000,
    WARP_DEV_PROC_STATE_PARAM_ERROR                 = 0x1000,
    WARP_DEV_PROC_STATE_ADDRESS_NOT_ALIGNED_ERROR   = 0x1001,
    WARP_DEV_PROC_STATE_BOUND_BOX_CALC_ERROR        = 0x1002,
    WARP_DEV_PROC_STATE_BOUND_BOX_FORMAT_ERROR   	= 0x1003,
    WARP_DEV_PROC_STATE_MATRIX_VALUES_ERROR         = 0x1004,
    WARP_DEV_PROC_STATE_OP_ERROR                    = 0x1005,
    WARP_DEV_PROC_STATE_OUT_OF_MEMORY               = 0x1006,
    WARP_DEV_PROC_STATE_HW_UNAVAILABLE              = 0x1007,
    WARP_DEV_PROC_STATE_MEMORY_FAILURE              = 0x1008,
    WARP_DEV_PROC_STATE_BUSY                        = 0x1009,
    WARP_DEV_PROC_STATE_PROC_CONFIG_ERROR           = 0x100a,
    WARP_DEV_PROC_STATE_CLK_ERROR                   = 0x100b,
    WARP_DEV_PROC_STATE_DISP_MAP_CALC_ERROR        	= 0x100c,
    WARP_DEV_PROC_STATE_IMAGE_INPUT_SIZE_ERROR      = 0x100d,
    WARP_DEV_PROC_STATE_IMAGE_OUTPUT_SIZE_ERROR     = 0x100e
} WARP_DEV_PROC_STATE_E;

typedef struct
{
    unsigned short tile_h;
    unsigned short tile_w;
    unsigned short format;
    unsigned short dummy[5];

}BOUND_BOX_HEARDER_T;


typedef void (*WARP_DEV_POST_CALLBACK)(MHAL_WARP_DEV_HANDLE, void *, WARP_DEV_STATE);

WARP_DEV_STATE warp_dev_isr_proc(MHAL_WARP_DEV_HANDLE device);
s32 warp_dev_create(MHAL_WARP_DEV_HANDLE *device, phys_addr_t base_warp, phys_addr_t base_sys, phys_addr_t base_axi2miu0, phys_addr_t base_axi2miu1, phys_addr_t base_axi2miu2, phys_addr_t base_axi2miu3);
s32 warp_dev_destroy(MHAL_WARP_DEV_HANDLE device);
WARP_DEV_PROC_STATE_E warp_dev_trigger(MHAL_WARP_DEV_HANDLE device, MHAL_WARP_CONFIG *config, WARP_DEV_POST_CALLBACK callback, void *user_data);

#endif //_DEV_WARP_H_
