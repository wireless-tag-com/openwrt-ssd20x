/*
* drv_ive.c- Sigmastar
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
#include "mdrv_ive.h"
#include "drv_ive.h"
#include "hal_ive.h"

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/spinlock.h>

#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/sched.h>

#include <linux/clk.h>
#include <linux/clk-provider.h>

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/list.h>

#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>

#define MINIMUM_WIDTH        16
#define MINIMUM_HEIGHT       5

typedef struct {
    struct list_head        list;
    ive_file_data           *file_data;
} ive_request_data;

/*******************************************************************************************************************
 * drv_ive_extract_request
 *   Extract a request from waiting list
 *   The request is removed from list
 *
 * Parameters:
 *   ive_drv_handle: driver handle
 *
 * Return:
 *   File data of IVE driver
 */
static ive_file_data* drv_ive_extract_request(ive_drv_handle *handle)
{
    struct platform_device *pdev = handle->pdev;
    ive_request_data *list_data = NULL;
    ive_file_data    *file_data = NULL;

    if (list_empty(&handle->request_list))
    {
        return NULL;
    }

    list_data = list_first_entry(&handle->request_list, ive_request_data, list);
    if (list_data != NULL) {
        list_del(handle->request_list.next);
        file_data = list_data->file_data;
        IVE_MSG(IVE_MSG_DBG, "extract: 0x%p, 0x%p\n", list_data, file_data);
        devm_kfree(&pdev->dev, list_data);
    } else {
        IVE_MSG(IVE_MSG_DBG, "extract: 0x%p\n", list_data);
    }

    return file_data;
}


/*******************************************************************************************************************
 * drv_ive_get_request
 *   Get a request from waiting list
 *   The request is still kpet in list
 *
 * Parameters:
 *   ive_drv_handle: driver handle
 *
 * Return:
 *   File data of IVE driver
 */
static ive_file_data* drv_ive_get_request(ive_drv_handle *handle)
{
    ive_request_data *list_data = NULL;

    if (list_empty(&handle->request_list))
    {
        return NULL;
    }

    list_data = list_first_entry(&handle->request_list, ive_request_data, list);
    if (list_data != NULL) {
        IVE_MSG(IVE_MSG_DBG, "get: 0x%p, 0x%p\n", list_data, list_data->file_data);
        return list_data->file_data;
    } else {
        IVE_MSG(IVE_MSG_DBG, "get: 0x%p\n", list_data);
    }

    return NULL;
}

/*******************************************************************************************************************
 * drv_ive_add_request
 *   Add a request to waiting list
 *
 * Parameters:
 *   ive_drv_handle: driver handle
 *   file_data: File data of IVE driver
 *
 * Return:
 *   none
 */
static void drv_ive_add_request(ive_drv_handle *handle, ive_file_data *file_data)
{
    struct platform_device *pdev = handle->pdev;
    ive_request_data *list_data;

    list_data = devm_kcalloc(&pdev->dev, 1, sizeof(ive_request_data), GFP_KERNEL);
    list_data->file_data = file_data;

    IVE_MSG(IVE_MSG_DBG, "add: 0x%p, 0x%p\n", list_data, file_data);

    if (list_data != NULL) {
        list_add_tail(&list_data->list, &handle->request_list);
    }
}

/*******************************************************************************************************************
 * drv_ive_check_config
 *   Check the configuration
 *
 * Parameters:
 *   config: IO configuration
 *
 * Return:
 *   standard return enum
 */
static IVE_IOC_ERROR drv_ive_check_config(ive_ioc_config *config)
{
    switch (config->op_type)
    {
        case IVE_IOC_OP_TYPE_SAD:
            break;

        case IVE_IOC_OP_TYPE_NCC:
        case IVE_IOC_OP_TYPE_MAP:
        case IVE_IOC_OP_TYPE_HISTOGRAM:
        case IVE_IOC_OP_TYPE_BAT:
            return IVE_IOC_ERROR_NONE;

        default:
            if (config->input.width != config->output.width || config->input.height != config->output.height) {
                IVE_MSG(IVE_MSG_ERR, "Input and output size are not the same\n");
                return IVE_IOC_ERROR_IN_OUT_SIZE_DIFFERENT;
            }
            break;
    }

    if (config->input.width < MINIMUM_WIDTH || config->input.height < MINIMUM_HEIGHT) {
        IVE_MSG(IVE_MSG_ERR, "The minimum withd/height is %d\n", MINIMUM_WIDTH);
        return IVE_IOC_ERROR_IMG_TOO_SMALL;
    }

    return IVE_IOC_ERROR_NONE;
}

/*******************************************************************************************************************
 * ive_drv_sync_image
 *   Sync image
 *
 * Parameters:
 *   handle: device handle
 *   image: image
 *   direct: DMA directon
 *
 * Return:
 *   None
 */
static void ive_drv_sync_image(ive_drv_handle *handle, ive_ioc_image *image, enum dma_data_direction direct)
{
#ifdef USE_MIU_DIRECT
    IVE_MSG(IVE_MSG_DBG, "sync buffer %p, but nothing to do for MIU buffer\n", image->address[0]);
#else
    int i, size, byte_per_pixel;

    switch (image->format)
    {
        case IVE_IOC_IMAGE_FORMAT_B8C1:
            byte_per_pixel = sizeof(u8);
            IVE_MSG(IVE_MSG_DBG, "case %d: byte_per_pixel = %d (%d)", image->format, byte_per_pixel, sizeof(u64) );
            break;

        case IVE_IOC_IMAGE_FORMAT_B16C1:
            byte_per_pixel = sizeof(u16);
            IVE_MSG(IVE_MSG_DBG, "case %d: byte_per_pixel = %d (%d)", image->format, byte_per_pixel, sizeof(u64) );
            break;

        case IVE_IOC_IMAGE_FORMAT_B32C1:
            byte_per_pixel = sizeof(u32);
            IVE_MSG(IVE_MSG_DBG, "case %d: byte_per_pixel = %d (%d)", image->format, byte_per_pixel, sizeof(u64) );
            break;

        case IVE_IOC_IMAGE_FORMAT_B64C1:
            byte_per_pixel = sizeof(u64);
            IVE_MSG(IVE_MSG_DBG, "case %d: byte_per_pixel = %d (%d)", image->format, byte_per_pixel, sizeof(u64) );
            break;

        case IVE_IOC_IMAGE_FORMAT_B8C3_PACK:
            byte_per_pixel = sizeof(u8) * 3;
            IVE_MSG(IVE_MSG_DBG, "case %d: byte_per_pixel = %d (%d)", image->format, byte_per_pixel, sizeof(u64) );
            break;

        case IVE_IOC_IMAGE_FORMAT_B8C3_PLAN:
            size = image->stride[0] * image->height * sizeof(u8);
            dma_sync_single_for_device(NULL, (dma_addr_t)image->address[0], size, direct);
            dma_sync_single_for_device(NULL, (dma_addr_t)image->address[1], size, direct);
            dma_sync_single_for_device(NULL, (dma_addr_t)image->address[2], size, direct);
            return;

        case IVE_IOC_IMAGE_FORMAT_420SP:
            size = image->stride[0] * image->height * sizeof(u8);
            dma_sync_single_for_device(NULL, (dma_addr_t)image->address[0], size, direct);
            dma_sync_single_for_device(NULL, (dma_addr_t)image->address[1], size/2, direct);
            return;

        case IVE_IOC_IMAGE_FORMAT_422SP:
            size = image->stride[0] * image->height * sizeof(u8);
            dma_sync_single_for_device(NULL, (dma_addr_t)image->address[0], size, direct);
            dma_sync_single_for_device(NULL, (dma_addr_t)image->address[1], size, direct);
            return;

        default:
            return;
    }

    // sync for single channel, HW use multi channel to represent multi inputs
    for (i=0; i<3; i++)
    {
        if (image->address[i] != NULL)
        {
            size = image->stride[i] * image->height * byte_per_pixel;
            dma_sync_single_for_device(NULL, (dma_addr_t)image->address[i], size, direct);
            IVE_MSG(IVE_MSG_DBG, "sync buffer %p, format %x, size %d * %d * %d = %d\n", image->address[i], image->format, image->stride[i], image->height, byte_per_pixel, size);
        }
    }
#endif
}

/*******************************************************************************************************************
 * ive_drv_set_images
 *   Set & sync input/output images
 *
 * Parameters:
 *   handle: device handle
 *   config: configuration
 *
 * Return:
 *   None
 */
static void ive_drv_set_images(ive_drv_handle *handle, ive_ioc_config *config)
{
    ive_hal_set_images(&handle->hal_handle, &config->input, &config->output);

    switch (config->op_type)
    {
        case IVE_IOC_OP_TYPE_NCC:
            ive_drv_sync_image(handle, &config->input,  DMA_TO_DEVICE);
            break;

        default:
            ive_drv_sync_image(handle, &config->input,  DMA_TO_DEVICE);
            ive_drv_sync_image(handle, &config->output, DMA_FROM_DEVICE);
            break;
    }
}


/*******************************************************************************************************************
 * ive_drv_isr_handler
 *   ISR handler
 *
 * Parameters:
 *   irq:    interrupt number
 *   handle: device handle
 *
 * Return:
 *   None
 */
IVE_DRV_STATE  ive_drv_isr_handler(int irq, ive_drv_handle *handle)
{
    IVE_MSG(IVE_MSG_DBG, "Interrupt: 0x%X\n", ive_hal_get_irq(&handle->hal_handle));

    // IVE_HAL_IRQ_MASK_FRAME_DONE
    if(ive_hal_get_irq_check(&handle->hal_handle, IVE_HAL_IRQ_MASK_FRAME_DONE)) {
        handle->dev_state = IVE_DRV_STATE_DONE;

        // Clear IRQ
        ive_hal_clear_irq(&handle->hal_handle, IVE_HAL_IRQ_MASK_FRAME_DONE);

        return IVE_DRV_STATE_DONE;
    }

    return IVE_DRV_STATE_PROCESSING;
}

/*******************************************************************************************************************
 * ive_drv_init
 *   Init IVEG settings
 *
 * Parameters:
 *   handle: device handle
 *   pdev:          platform device
 *   base_addr0:    base addr of HW register bank 0
 *   base_addr1:    base addr of HW register bank 1
 *
 * Return:
 *   none
 */
int ive_drv_init(ive_drv_handle *handle, struct platform_device *pdev, phys_addr_t base_addr0, phys_addr_t base_addr1)
{
    IVE_MSG(IVE_MSG_DBG, "name: %s, addr 0x%08X, 0x%08X\n", pdev->name, base_addr0, base_addr1);

    memset(handle, 0, sizeof(ive_drv_handle));

    ive_hal_init(&handle->hal_handle, base_addr0, base_addr1);
    INIT_LIST_HEAD(&handle->request_list);
    IVE_MSG(IVE_MSG_DBG, "list_empty = %d\n", list_empty(&handle->request_list));

    handle->pdev = pdev;
    handle->dev_state = IVE_DRV_STATE_READY;

    return 0;
}

/*******************************************************************************************************************
 * ive_drv_release
 *   Release IVEG settings
 *
 * Parameters:
 *   handle: device handle
 *
 * Return:
 *   none
 */
void ive_drv_release(ive_drv_handle *handle)
{
}

/*******************************************************************************************************************
 * ive_drv_start
 *   Start HW engine
 *
 * Parameters:
 *   handle: device handle
 *   config: IVE configurations
 *
 * Return:
 *   none
 */
static void ive_drv_start(ive_drv_handle *handle, ive_ioc_config *config)
{
    IVE_MSG(IVE_MSG_DBG, "op_type: 0x%X\n", config->op_type);

    ive_hal_sw_reset(&handle->hal_handle);

    ive_hal_set_operation(&handle->hal_handle, config->op_type);

    ive_drv_set_images(handle, config);

    switch (config->op_type) {
        case IVE_IOC_OP_TYPE_FILTER:
            ive_hal_set_coeff_filter(&handle->hal_handle, &config->coeff_filter);
            break;

        case IVE_IOC_OP_TYPE_CSC:
            ive_hal_set_coeff_csc(&handle->hal_handle, &config->coeff_csc);
            break;

        case IVE_IOC_OP_TYPE_FILTER_AND_CSC:
            ive_hal_set_coeff_filter(&handle->hal_handle, &config->coeff_filter_csc.filter);
            ive_hal_set_coeff_csc(&handle->hal_handle, &config->coeff_filter_csc.csc);
            break;

        case IVE_IOC_OP_TYPE_SOBEL:
            ive_hal_set_coeff_sobel(&handle->hal_handle, &config->coeff_sobel);
            break;

        case IVE_IOC_OP_TYPE_MAG_AND_ANG:
            ive_hal_set_coeff_mag_and_ang(&handle->hal_handle, &config->coeff_mag_and_ang);
            break;

        case IVE_IOC_OP_TYPE_ORD_STA_FILTER:
            ive_hal_set_coeff_ord_stat_filter(&handle->hal_handle, &config->coeff_ord_stat_filter);
            break;

        case IVE_IOC_OP_TYPE_BERNSEN:
            ive_hal_set_coeff_bernsen(&handle->hal_handle, &config->coeff_bernsen);
            break;

        case IVE_IOC_OP_TYPE_DILATE:
            ive_hal_set_coeff_dilate(&handle->hal_handle, &config->coeff_dilate);
            break;

        case IVE_IOC_OP_TYPE_ERODE:
            ive_hal_set_coeff_erode(&handle->hal_handle, &config->coeff_erode);
            break;

        case IVE_IOC_OP_TYPE_THRESH:
            ive_hal_set_coeff_thresh(&handle->hal_handle, &config->coeff_thresh);
            break;

        case IVE_IOC_OP_TYPE_THRESH_S16:
            ive_hal_set_coeff_thresh_s16(&handle->hal_handle, &config->coeff_thresh_s16);
            break;

        case IVE_IOC_OP_TYPE_THRESH_U16:
            ive_hal_set_coeff_thresh_u16(&handle->hal_handle, &config->coeff_thresh_u16);
            break;

        case IVE_IOC_OP_TYPE_ADD:
            ive_hal_set_coeff_add(&handle->hal_handle, &config->coeff_add);
            break;

        case IVE_IOC_OP_TYPE_SUB:
            ive_hal_set_coeff_sub(&handle->hal_handle, &config->coeff_sub);
            break;

        case IVE_IOC_OP_TYPE_16BIT_TO_8BIT:
            ive_hal_set_coeff_16to8(&handle->hal_handle, &config->coeff_16to8);
            break;

        case IVE_IOC_OP_TYPE_MAP:
            ive_hal_set_coeff_map(&handle->hal_handle, (uintptr_t)config->coeff_map.map);
            break;

        case IVE_IOC_OP_TYPE_INTEGRAL:
            ive_hal_set_coeff_integral(&handle->hal_handle, &config->coeff_integral);
            break;

        case IVE_IOC_OP_TYPE_SAD:
            ive_hal_set_coeff_sad(&handle->hal_handle, &config->coeff_sad);
            break;

        case IVE_IOC_OP_TYPE_NCC:
            ive_hal_set_coeff_ncc(&handle->hal_handle, (uintptr_t)config->coeff_ncc);
            break;

        case IVE_IOC_OP_TYPE_LBP:
            ive_hal_set_coeff_lbp(&handle->hal_handle, &config->coeff_lbp);
            break;

        case IVE_IOC_OP_TYPE_BAT:
            ive_hal_set_coeff_bat(&handle->hal_handle, &config->coeff_bat);
            break;

        case IVE_IOC_OP_TYPE_ADP_THRESH:
            ive_hal_set_coeff_adp_thresh(&handle->hal_handle, &config->coeff_adp_thresh);
            break;

        case IVE_IOC_OP_TYPE_MATRIX_TRANSFORM:
            ive_hal_set_coeff_matrix_transform(&handle->hal_handle, &config->coeff_matrix_transform);
            break;

        case IVE_IOC_OP_TYPE_IMAGE_DOT:
            ive_hal_set_coeff_image_dot(&handle->hal_handle, &config->coeff_image_dot);
            break;

        default:
            break;
    }

    ive_hal_clear_irq(&handle->hal_handle, IVE_HAL_IRQ_MASK_ALL);
    ive_hal_set_irq_mask(&handle->hal_handle, IVE_HAL_IRQ_MASK_FRAME_DONE);

    ive_hal_start(&handle->hal_handle);
}

/*******************************************************************************************************************
 * ive_drv_start_proc
 *   Start IVE process image
 *
 * Parameters:
 *   handle: device handle
 *
 * Return:
 *   IVE_IOC_ERROR
 */
IVE_IOC_ERROR ive_drv_process(ive_drv_handle *handle, ive_file_data *file_data)
{
    IVE_IOC_ERROR ret = IVE_IOC_ERROR_NONE;
#if 0 // multi instance test
    static int pause_count = 0;
#endif

    ret = drv_ive_check_config(&file_data->ioc_config);
    if (ret != IVE_IOC_ERROR_NONE) {
        return ret;
    }

    file_data->state = IVE_FILE_STATE_IN_QUEUE;
    drv_ive_add_request(handle, file_data);

    // do nothing if hw is not ready
    if ((handle->dev_state != IVE_DRV_STATE_READY)
#if 0 // multi instance test
        || (pause_count++ < 10)
#endif
       )
    {
        IVE_MSG(IVE_MSG_DBG, "HW is busy\n");
        return IVE_IOC_ERROR_NONE;
    }

    file_data = drv_ive_get_request(handle);
    if (file_data == NULL) {
        IVE_MSG(IVE_MSG_DBG, "no more request in queue\n");
        return IVE_IOC_ERROR_MEMROY_FAILURE;
    }

    file_data->state = IVE_FILE_STATE_PROCESSING;
    handle->dev_state = IVE_DRV_STATE_PROCESSING;

    IVE_MSG(IVE_MSG_DBG, "process: %p ; %p ; %p\n", file_data->ioc_config.input.address[0], file_data->ioc_config.input.address[1], file_data->ioc_config.input.address[2]);

    ive_drv_start(handle, &file_data->ioc_config);

    return ret;
}

/*******************************************************************************************************************
 * ive_drv_post_process
 *   Post process after IVE HW done
 *
 * Parameters:
 *   handle: device handle
 *
 * Return:
 *   IVE_IOC_ERROR
 */
ive_file_data* ive_drv_post_process(ive_drv_handle *handle)
{
    ive_file_data *previous_file_data, *next_file_data;

    previous_file_data = drv_ive_extract_request(handle);
    next_file_data = drv_ive_get_request(handle);

    if (previous_file_data != NULL) {
        previous_file_data->state = IVE_FILE_STATE_DONE;
    }

    if (next_file_data == NULL) {
        IVE_MSG(IVE_MSG_DBG, "no more request in queue\n");
        handle->dev_state = IVE_DRV_STATE_READY;
    } else {
        IVE_MSG(IVE_MSG_DBG, "process: 0x%p, 0x%p, 0x%p\n", next_file_data->ioc_config.input.address[0], next_file_data->ioc_config.input.address[1], next_file_data->ioc_config.output.address[2]);
        next_file_data->state = IVE_FILE_STATE_PROCESSING;
        ive_drv_start(handle, &next_file_data->ioc_config);
    }

    return previous_file_data;
}
