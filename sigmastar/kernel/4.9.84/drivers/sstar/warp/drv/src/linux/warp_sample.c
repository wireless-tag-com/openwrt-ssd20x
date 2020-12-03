/*
* warp_sample.c- Sigmastar
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
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include "mhal_warp.h"
#include "irqs.h"


//the example for config
//MHAL_WARP_CONFIG config =
//{
//    .op_mode = MHAL_WARP_OP_MODE_MAP,
//    .disp_table = {.resolution = MHAL_WARP_MAP_RESLOUTION_16X16, .format = MHAL_WARP_MAP_FORMAT_ABSOLUTE, .size = 263296, .absolute_table = NULL/*mi_3840x2160_disp_484x136_abs.bin*/},
//    .bb_table = {.size = 816, .table = NULL /*bb_undist.bin.bin*/},
//    .coeff = {0, 0, 0, 0, 0, 0, 0, 0, 0},
//    .input_image = {.format = MHAL_WARP_IMAGE_FORMAT_YUV420, .width = 3840, .height = 2160},
//    .input_data = {.num_planes = 2, .data[0] = NULL/*input Y buffer*/, .data[1] = NULL/*input UV buffer*/},
//    .output_image = {.format = MHAL_WARP_IMAGE_FORMAT_YUV420, .width = 3840, .height = 2160},
//    .output_data = {.num_planes = 2, .data[0] = NULL/*output Y buffer*/, .data[1] = NULL/*output UV buffer*/},
//    .afbc_en = 0,
//    .fill_value = {0, 0x80},
//    .axi_max_read_burst_size = 8,
//    .axi_max_write_burst_size = 8,
//    .axi_max_read_outstanding = 8,
//    .axi_max_write_outstanding = 8,
//    .debug_bypass_displacement_en = 0,
//    .debug_bypass_interp_en = 0,
//    .output_tiles_width = 160,
//    .output_tiles_height = 64,
//    .num_output_tiles = 816
//};

irqreturn_t  warp_sample_isr(s32 irq, void* data)
{
    MHAL_WARP_DEV_HANDLE device = (MHAL_WARP_DEV_HANDLE*)data;
    MHAL_WARP_ISR_STATE_E state;

    state = MHAL_WARP_IsrProc(device);
    switch(state)
    {
        case MHAL_WARP_ISR_STATE_DONE:
            return IRQ_HANDLED;

        default:
            return IRQ_NONE;
    }

    return IRQ_NONE;
}

MS_BOOL warp_sample_callback(MHAL_WARP_INST_HANDLE instance, void *user_data)
{
    struct semaphore *sema = (struct semaphore *)user_data;

    printk("callback is called\n");

    up(sema);

    return 1;
}

void warp_sample(MHAL_WARP_DEV_HANDLE device, MHAL_WARP_CONFIG* config)
{
    MHAL_WARP_DEV_HANDLE local_device = device;
    MHAL_WARP_INST_HANDLE instance;
    struct semaphore sema;
    s32 ret;

    sema_init(&sema, 0);

    // create a local device if caller does not pass a device
    if (local_device == NULL)
    {
        ret = MHAL_WARP_CreateDevice(0, &local_device);
        if (ret != MHAL_SUCCESS)
        {
            printk("can't create device!!\n");
            return;
        }

        // Register a ISR
        // irqs.h is for dtsi, c source has to add 32 if we use the define
        ret = request_irq(INT_IRQ_78_WARP2RIU_INT+32, warp_sample_isr, IRQ_TYPE_LEVEL_HIGH, "warp isr", local_device);
        if (ret != 0)
        {
            printk("can't request a irq\n");
            goto RETURN_0;
        }

    }

    // create instance
    ret = MHAL_WARP_CreateInstance(local_device, &instance);
    if (ret != MHAL_SUCCESS)
    {
        printk("can't create instance!!\n");
        goto RETURN_1;
    }

    // proce a image
    ret = MHAL_WARP_Trigger(instance, config, warp_sample_callback, &sema);
    if (ret != MHAL_SUCCESS)
    {
        printk("can't process image!!\n");
        goto RETURN_2;
    }

    down(&sema);

    printk("process done\n");

RETURN_2:
    MHAL_WARP_DestroyInstance(instance);

RETURN_1:
    if (local_device != device)
    {
        free_irq(INT_IRQ_78_WARP2RIU_INT, local_device);
    }

RETURN_0:
    if (local_device != device)
    {
        MHAL_WARP_DestroyDevice(local_device);
    }
}
