/*
* dev_warp.c- Sigmastar
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
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/time.h>

#include <asm/io.h>

#include "ms_platform.h"
#include "mhal_warp.h"
#include "dev_warp.h"
#include "hal_clk.h"
#include "hal_warp.h"
#include "hal_ceva.h"

#include "dev_debug.h"

#define WARP_MAX_IMAGE_WIDTH     8189
#define WARP_MIN_IMAGE_WIDTH     16
#define WARP_MAX_IMAGE_HEIGHT    8189
#define WARP_MIN_IMAGE_HEIGHT    16
#define COEF_PREC               12
#define WS_COEF_PREC            24
#define ALIGNMENT_VALUE_2   2
#define ALIGNMENT_VALUE_4   4
#define ALIGNMENT_VALUE_16 16
#define ALIGNMENT_VALUE_32 32
#define ALIGNMENT_MASK_2          (ALIGNMENT_VALUE_2 - 1)
#define ALIGNMENT_MASK_4          (ALIGNMENT_VALUE_4 - 1)
#define ALIGNMENT_MASK_16          (ALIGNMENT_VALUE_16 - 1)
#define ALIGNMENT_MASK_32          (ALIGNMENT_VALUE_32 - 1)
#define CHECK_ALIGNEMT_2(a)       (((u32)(a) & ALIGNMENT_MASK_2) != 0)
#define CHECK_ALIGNEMT_4(a)       (((u32)(a) & ALIGNMENT_MASK_4) != 0)
#define CHECK_ALIGNEMT_16(a)       (((u32)(a) & ALIGNMENT_MASK_16) != 0)
#define CHECK_ALIGNEMT_32(a)       (((u32)(a) & ALIGNMENT_MASK_32) != 0)

#define ABS(a)   ((a)>(0) ? (a) : (-(a)))

typedef struct
{
    struct list_head        list;
    MHAL_WARP_CONFIG        *config;
    WARP_DEV_POST_CALLBACK   callback;
    void                    *user_data;
} warp_request_data;

/// @brief Data structure for driver
typedef struct
{
    warp_hal_handle          hal_handle;         // HAL handle for real HW configuration
    ceva_hal_handle         ceva_hal_handle;
    s32                     ceva_irq;
    WARP_DEV_STATE           state;              // device state
    struct list_head        request_list;       // request list to queue waiting requst
    struct work_struct      work_queue;         // work queue for post process after ISR
    struct mutex            mutex;              // for critical section
} warp_dev_handle;

#if LOG_WARP_TIMING
struct timespec gT_start;
struct timespec gT_end;
#endif
/*******************************************************************************************************************
 * warp_dev_check_config
 *   check config status before HW engine run
 *
 * Parameters:
 *   handle: device handle
 *   config: WARP configurations
 *
 * Return:
 *  WARP_DEV_PROC_STATE_E
 */
static WARP_DEV_PROC_STATE_E warp_dev_check_config(MHAL_WARP_CONFIG *config)
{
    int width = 0;
    int height = 0;
    int size = 0;
    int decimation_factor = 0;
    BOUND_BOX_HEARDER_T* ptbb_header;
    int tile_w = 0;
    int tile_h = 0;
    u32 virt_addr = 0;
    u32 phy_addr = 0;

    MHAL_WARP_IMAGE_DATA_T* input_data   = &config->input_data;
    MHAL_WARP_IMAGE_DATA_T* output_data  = &config->output_data;
    MHAL_WARP_IMAGE_DESC_T* input_image  = &config->input_image;
    MHAL_WARP_IMAGE_DESC_T* output_image = &config->output_image;

    MHAL_WARP_DISPLAY_TABLE_T* disp_table = &config->disp_table;
    MHAL_WARP_BOUND_BOX_TABLE_T* bound_box = &config->bb_table;
    int* coeff = config->coeff;
    MHAL_WARP_OP_MODE_E op = config->op_mode;

    //check config null or not
    if (NULL == config ||
        NULL == input_data || NULL == output_data ||
        NULL == disp_table ||
        NULL == bound_box)
    {
        return WARP_DEV_PROC_STATE_PARAM_ERROR;
    }

    //check image format
    if (input_image->format != output_image->format)
    {
        return WARP_DEV_PROC_STATE_PARAM_ERROR;
    }

    //check image size : input and output
    if (WARP_MIN_IMAGE_WIDTH  > input_image->width  ||
        WARP_MIN_IMAGE_HEIGHT > input_image->height ||
        WARP_MAX_IMAGE_WIDTH  < input_image->width  ||
        WARP_MAX_IMAGE_HEIGHT < input_image->height)
    {
        return WARP_DEV_PROC_STATE_PARAM_ERROR;
    }
    if (WARP_MIN_IMAGE_WIDTH  > output_image->width  ||
        WARP_MIN_IMAGE_HEIGHT > output_image->height ||
        WARP_MAX_IMAGE_WIDTH  < output_image->width  ||
        WARP_MAX_IMAGE_HEIGHT < output_image->height)
    {
        return WARP_DEV_PROC_STATE_PARAM_ERROR;
    }
    //check input image size restriction
    switch(input_image->format)
    {
        case MHAL_WARP_IMAGE_FORMAT_RGBA:
            if (CHECK_ALIGNEMT_4(input_image->width))
            {
                return WARP_DEV_PROC_STATE_IMAGE_INPUT_SIZE_ERROR;
            }
            break;
        case MHAL_WARP_IMAGE_FORMAT_YUV422:
            if (CHECK_ALIGNEMT_16(input_image->width))
            {
                return WARP_DEV_PROC_STATE_IMAGE_INPUT_SIZE_ERROR;
            }
            break;
        case MHAL_WARP_IMAGE_FORMAT_YUV420:
            if (CHECK_ALIGNEMT_16(input_image->width))
            {
                return WARP_DEV_PROC_STATE_IMAGE_INPUT_SIZE_ERROR;
            }
            if (CHECK_ALIGNEMT_2(input_image->height))
            {
                return WARP_DEV_PROC_STATE_IMAGE_INPUT_SIZE_ERROR;
            }
            break;
        default:
            return WARP_DEV_PROC_STATE_IMAGE_INPUT_SIZE_ERROR;
            break;
    }
    //check output image size restriction
    switch(output_image->format)
    {
        case MHAL_WARP_IMAGE_FORMAT_RGBA:
            if (CHECK_ALIGNEMT_16(output_image->width))
            {
                return WARP_DEV_PROC_STATE_IMAGE_OUTPUT_SIZE_ERROR;
            }
            break;
        case MHAL_WARP_IMAGE_FORMAT_YUV422:
            if (CHECK_ALIGNEMT_32(output_image->width))
            {
                return WARP_DEV_PROC_STATE_IMAGE_OUTPUT_SIZE_ERROR;
            }
            break;
        case MHAL_WARP_IMAGE_FORMAT_YUV420:
            if (CHECK_ALIGNEMT_32(output_image->width))
            {
                return WARP_DEV_PROC_STATE_IMAGE_OUTPUT_SIZE_ERROR;
            }
            if (CHECK_ALIGNEMT_2(output_image->height))
            {
                return WARP_DEV_PROC_STATE_IMAGE_OUTPUT_SIZE_ERROR;
            }
            break;
        default:
            return WARP_DEV_PROC_STATE_IMAGE_OUTPUT_SIZE_ERROR;
            break;
    }

    //check image buffer aligned or not
    if (CHECK_ALIGNEMT_16(input_data->data[MHAL_WARP_IMAGE_PLANE_RGBA]) ||
        CHECK_ALIGNEMT_16(output_data->data[MHAL_WARP_IMAGE_PLANE_RGBA]))
    {
        return WARP_DEV_PROC_STATE_ADDRESS_NOT_ALIGNED_ERROR;
    }

    if (config->input_image.format != MHAL_WARP_IMAGE_FORMAT_RGBA)              //UV
    {
        if (CHECK_ALIGNEMT_16(input_data->data[MHAL_WARP_IMAGE_PLANE_UV]) ||
            CHECK_ALIGNEMT_16(output_data->data[MHAL_WARP_IMAGE_PLANE_UV]))
        {
            return WARP_DEV_PROC_STATE_ADDRESS_NOT_ALIGNED_ERROR;
        }
    }
    //Check bb size and format from b.b header.
    phy_addr = (u32)Chip_MIU_to_Phys(bound_box->table);
    virt_addr = (u32)phys_to_virt(phy_addr);
    ptbb_header = (BOUND_BOX_HEARDER_T*)(virt_addr);
    tile_w = ptbb_header->tile_w;
    tile_h = ptbb_header->tile_h;
    width = (output_image->width + tile_w - 1) / tile_w;
    height = (output_image->height + tile_h - 1) / tile_h;
    size = sizeof(BOUND_BOX_HEARDER_T) + (width * height * sizeof(MHAL_WARP_BOUND_BOX_ENTRY_T) );

    if( config->bb_table.size  != size )
    {
        return WARP_DEV_PROC_STATE_BOUND_BOX_CALC_ERROR;
    }

    if( config->input_image.format != (MHAL_WARP_IMAGE_FORMAT_E)(ptbb_header->format - 1) )
    {
        return WARP_DEV_PROC_STATE_BOUND_BOX_FORMAT_ERROR;
    }

    //check bb table aligned or not
    if (CHECK_ALIGNEMT_16(bound_box->table))
    {
        return WARP_DEV_PROC_STATE_ADDRESS_NOT_ALIGNED_ERROR;
    }

    switch(op)
    {
        case MHAL_WARP_OP_MODE_MAP:
            //check disp. table aligned or not
            if (CHECK_ALIGNEMT_16(disp_table->table))
            {
                return WARP_DEV_PROC_STATE_ADDRESS_NOT_ALIGNED_ERROR;
            }

            //grid size 8x8 or 16x16
            if ( disp_table->resolution == MHAL_WARP_MAP_RESLOUTION_8X8)
            {
               decimation_factor = 8;
            }
            else if (disp_table->resolution == MHAL_WARP_MAP_RESLOUTION_16X16)
            {
                decimation_factor = 16;
            }
            else
            {
                //error grid size
                return WARP_DEV_PROC_STATE_DISP_MAP_CALC_ERROR;
            }

            //check Disp. size
            width = ((output_image->width + decimation_factor - 1) / decimation_factor) + 1;
            height = ((output_image->height + decimation_factor - 1) / decimation_factor) + 1;

            switch(disp_table->format)
            {
                case MHAL_WARP_MAP_FORMAT_ABSOLUTE:
                    width = ( (width + 1)/2 ) * 2;
                    size = (height * width * 2) * 4;
                    break;
                case MHAL_WARP_MAP_FORMAT_RELATIVE:
                    width = ( (width + 3)/4 ) * 4;
                    size =  (height * width * 2) * 4;
                    break;
                default:
                    return WARP_DEV_PROC_STATE_DISP_MAP_CALC_ERROR;

            }//end switch(format)

            if(disp_table->size != size)
            {
                return WARP_DEV_PROC_STATE_DISP_MAP_CALC_ERROR;
            }

            break;

        case MHAL_WARP_OP_MODE_PERSPECTIVE:

            //check persepective value range
            if ( ABS(coeff[MHAL_WARP_PERSECTIVE_COEFFS_C00]) >= (8 << COEF_PREC) )
            {
                return WARP_DEV_PROC_STATE_MATRIX_VALUES_ERROR;
            }
            else if ( ABS(coeff[MHAL_WARP_PERSECTIVE_COEFFS_C01]) >= (8 << COEF_PREC) )
            {
                return WARP_DEV_PROC_STATE_MATRIX_VALUES_ERROR;
            }
            else if ( ABS(coeff[MHAL_WARP_PERSECTIVE_COEFFS_C10]) >= (8 << COEF_PREC) )
            {
                return WARP_DEV_PROC_STATE_MATRIX_VALUES_ERROR;
            }
            else if ( ABS(coeff[MHAL_WARP_PERSECTIVE_COEFFS_C11]) >= (8 << COEF_PREC) )
            {
                return WARP_DEV_PROC_STATE_MATRIX_VALUES_ERROR;
            }
            else if ( ABS(coeff[MHAL_WARP_PERSECTIVE_COEFFS_C22]) >= (8 << WS_COEF_PREC) )
            {
                return WARP_DEV_PROC_STATE_MATRIX_VALUES_ERROR;
            }
            else if ( ABS(coeff[MHAL_WARP_PERSECTIVE_COEFFS_C20]) >= (1 << WS_COEF_PREC)>>9 ) // 1.0/512
            {
                return WARP_DEV_PROC_STATE_MATRIX_VALUES_ERROR;
            }
            else if ( ABS(coeff[MHAL_WARP_PERSECTIVE_COEFFS_C21]) >= (1 << WS_COEF_PREC)>>9 ) // 1.0/512
            {
                return WARP_DEV_PROC_STATE_MATRIX_VALUES_ERROR;
            }
            else if ( ABS(coeff[MHAL_WARP_PERSECTIVE_COEFFS_C02]) > (8191 << COEF_PREC) )
            {
                return WARP_DEV_PROC_STATE_MATRIX_VALUES_ERROR;
            }
            else if ( ABS(coeff[MHAL_WARP_PERSECTIVE_COEFFS_C12]) > (8191 << COEF_PREC) )
            {
                return WARP_DEV_PROC_STATE_MATRIX_VALUES_ERROR;
            }
            break;

        default :
                return WARP_DEV_PROC_STATE_OP_ERROR;
            break;
    }//end switch(op)

    return WARP_DEV_PROC_STATE_OK;
}

/*******************************************************************************************************************
 * warp_dev_start
 *   Start HW engine
 *
 * Parameters:
 *   handle: device handle
 *   config: WARP configurations
 *
 * Return:
 *
 */
static void warp_dev_start(warp_dev_handle *handle, MHAL_WARP_CONFIG *config)
{
    warp_hal_handle* hal_handle;     // HAL handle for real HW configuration
    MHAL_WARP_OP_MODE_E op_mode;
    BOUND_BOX_HEARDER_T* bb_header;
    s32 bb_header_len = 0;
    u32 virt_addr = 0;
    u32 phy_addr = 0;

    hal_handle = &handle->hal_handle;
    op_mode = config->op_mode;

    phy_addr = (u32)Chip_MIU_to_Phys(config->bb_table.table);
    virt_addr = (u32)phys_to_virt(phy_addr);

    bb_header = (BOUND_BOX_HEARDER_T*)virt_addr;
    bb_header_len = sizeof(BOUND_BOX_HEARDER_T);

    DEV_MSG(DEV_MSG_DBG, "op_mode: %d\n", op_mode ); //disp. map or perspective
    DEV_MSG(DEV_MSG_DBG, "base address = 0x%08x\n", hal_handle->base_addr); //base address

    //set AXI bus
    warp_hal_set_axi(hal_handle, (HAL_WARP_CONFIG *)config);

    //set image buffer pointer
    warp_hal_set_image_point(hal_handle, (HAL_WARP_CONFIG *)config);

    //set output tile size
    warp_hal_set_output_tile(hal_handle, bb_header->tile_w, bb_header->tile_h);

    //set image size
    warp_hal_set_image_size(hal_handle, (HAL_WARP_CONFIG *)config);

    //point to displacement map
    warp_hal_set_disp(hal_handle, (HAL_WARP_CONFIG *)config);

    //point to B.B table
    warp_hal_set_bb(hal_handle, (s32)(config->bb_table.table + bb_header_len));

    //set perspective transform coefficient matrix
    warp_hal_set_pers_matirx(hal_handle, (HAL_WARP_CONFIG *)config);

    //set out of range pixel fill value
    warp_hal_set_out_of_range(hal_handle, (HAL_WARP_CONFIG *)config);

    //set configure, and enable hw engine
    warp_hal_set_config(hal_handle, (HAL_WARP_CONFIG *)config);
}

/*******************************************************************************************************************
 * warp_dev_extract_request
 *   Extract a request from waiting list
 *   The request is removed from list
 *
 * Parameters:
 *   ive_drv_handle: driver handle
 *
 * Return:
 *   File data of IVE driver
 */
static warp_request_data* warp_dev_extract_request(warp_dev_handle *handle)
{
    warp_request_data *list_data = NULL;

    // check resuest list is empty.
    if (list_empty(&handle->request_list))
    {
        return NULL;
    }

    // get data from list
    list_data = list_first_entry(&handle->request_list, warp_request_data, list);
    if (list_data != NULL)
    {
        list_del(handle->request_list.next);    //delete first data in list
    }

    DEV_MSG(DEV_MSG_DBG, "extract: 0x%p\n", list_data);

    return list_data;
}

/*******************************************************************************************************************
 * warp_dev_get_request
 *   Get a request from waiting list
 *   The request is still kpet in list
 *
 * Parameters:
 *   warp_dev_handle: driver handle
 *
 * Return:
 *   File data of WARP driver
 */
static warp_request_data* warp_dev_get_request(warp_dev_handle *handle)
{
    warp_request_data *list_data = NULL;

    //check if request list is empty
    if (list_empty(&handle->request_list))
    {
        return NULL;
    }

    //get data from request list
    list_data = list_first_entry(&handle->request_list, warp_request_data, list);
    if (list_data != NULL)
    {
        DEV_MSG(DEV_MSG_DBG, "get: 0x%p, 0x%p\n", list_data, list_data->config);
        return list_data;
    }
    else
    {
        DEV_MSG(DEV_MSG_DBG, "get: 0x%p\n", list_data);
    }

    return NULL;
}
/*******************************************************************************************************************
 * warp_dev_add_request
 *   Add a request to waiting list
 *
 * Parameters:
 *   warp_dev_handle: driver handle
 *   file_data: File data of WARP driver
 *
 * Return:
 *   none
 */
static void warp_dev_add_request(warp_dev_handle *handle, MHAL_WARP_CONFIG *config, WARP_DEV_POST_CALLBACK callback, void *user_data)
{
    warp_request_data *list_data;

    //setup request data
    list_data = kcalloc(1, sizeof(warp_request_data), GFP_KERNEL);
    list_data->config    = config;
    list_data->callback  = callback;
    list_data->user_data = user_data;

    DEV_MSG(DEV_MSG_DBG, "add: 0x%p, 0x%p\n", list_data, config);

    //add request data to list
    if (list_data != NULL)
    {
        list_add_tail(&list_data->list, &handle->request_list);
    }
}

/*******************************************************************************************************************
 * warp_dev_trigger
 *   Start WARP process image
 *
 * Parameters:
 *   handle: device handle
 *
 * Return:
 *   WARP_DEV_PROC_STATE_E
 */
WARP_DEV_PROC_STATE_E warp_dev_trigger(MHAL_WARP_DEV_HANDLE device, MHAL_WARP_CONFIG *config, WARP_DEV_POST_CALLBACK callback, void *user_data)
{
    warp_dev_handle *handle = (warp_dev_handle*)device;
    WARP_DEV_PROC_STATE_E ret = WARP_DEV_PROC_STATE_OK;
    warp_request_data *request;

    //Check the configuration, such as image, disp. table, ......
    ret = warp_dev_check_config(config);
    if(ret != WARP_DEV_PROC_STATE_OK)
    {
        return ret;
    }

    mutex_lock(&handle->mutex);

    //add a request to linked list
    warp_dev_add_request(handle, config, callback, user_data);

    // do nothing if hw is not ready
    if (handle->state != WARP_DEV_STATE_READY)
    {
        DEV_MSG(DEV_MSG_DBG, "HW is busy\n");
        ret = WARP_DEV_PROC_STATE_OK;
        goto RETURN;
    }

    //get a request from liked list
    request = warp_dev_get_request(handle);
    if (request == NULL) //no more request in queue
    {
        DEV_MSG(DEV_MSG_DBG, "no more request in queue\n");
        ret = WARP_DEV_PROC_STATE_MEMORY_FAILURE;
        goto RETURN;
    }

    //run warp
    handle->state = WARP_DEV_STATE_PROCESSING;

    DEV_MSG(DEV_MSG_DBG, "process: %p\n", (void*)(u32)config->input_data.data[0]);

    //set miu bus
    ceva_hal_set_axi2miu(&handle->ceva_hal_handle);

    //enable ceva warp of wrapper
    ceva_hal_enable_warp(&handle->ceva_hal_handle);

#if LOG_WARP_TIMING
    getnstimeofday(&gT_start);
#endif

    //enable warp
    warp_dev_start(handle, config); //start HW engine

RETURN:
    mutex_unlock(&handle->mutex);

    return ret;
}

/*******************************************************************************************************************
 * warp_dev_post_proc
 *   Post process after WARP HW done
 *
 * Parameters:
 *   wq: work queue
 *
 * Return:
 *   none
 */
void warp_dev_post_proc(struct work_struct *wq)
{
    warp_dev_handle *handle;
    warp_request_data *current_request = NULL;
    warp_request_data *next_request = NULL;
    warp_request_data temp_request;

    handle = (warp_dev_handle*)container_of(wq, warp_dev_handle, work_queue);

    mutex_lock(&handle->mutex);

    DEV_MSG(DEV_MSG_DBG, "post proc handle: 0x%p\n",  handle);

    //get processed file_data from request list, then delete it in the list.
    current_request = warp_dev_extract_request(handle);
    if (current_request == NULL)
    {
        DEV_MSG(DEV_MSG_ERR, "can't get current request\n");
        goto RETURN;
    }

    // copy current to temporary instance
    memcpy(&temp_request, current_request, sizeof(temp_request));

    // free buffer
    kfree(current_request);

    //get current file_data from request list, it will be used by warp_dev_start().
    next_request = warp_dev_get_request(handle);

    //reset ceva warp of wrapper
    ceva_hal_reset_warp(&handle->ceva_hal_handle);

    if (next_request == NULL)
    {
         DEV_MSG(DEV_MSG_DBG, "no more request in queue\n");
         handle->state = WARP_DEV_STATE_READY;

        //Run callback to notify caller to do post-process
        if (temp_request.callback)
        {
            temp_request.callback(handle, temp_request.user_data, handle->state);
        }
    }
    else
    {
        //Run callback to notify caller to do post-process
        if (temp_request.callback)
        {
            temp_request.callback(handle, temp_request.user_data, handle->state);
        }

        //run warp
        handle->state = WARP_DEV_STATE_PROCESSING;
        DEV_MSG(DEV_MSG_DBG, "process: %p\n", (void*)(u32)next_request->config->input_data.data[0]);

         //set miu bus
         ceva_hal_set_axi2miu(&handle->ceva_hal_handle);

         //enable ceva warp of wrapper
         ceva_hal_enable_warp(&handle->ceva_hal_handle);

         //enable warp
        warp_dev_start(handle, next_request->config);
    }

RETURN:
    mutex_unlock(&handle->mutex);
}

/*******************************************************************************************************************
 * warp_dev_isr_proc
 *   ISR handler, check and clear ISR
 *
 * Parameters:
 *   irq:    interrupt number
 *   handle: device handle
 *
 * Return:
 *   None
 */
WARP_DEV_STATE warp_dev_isr_proc(MHAL_WARP_DEV_HANDLE device)
{
    warp_dev_handle *handle = (warp_dev_handle*)device;
    u32 irq_status;
    u32 irq_ceva;

#if LOG_WARP_TIMING
    getnstimeofday(&gT_end);
    DEV_MSG(DEV_MSG_WRN, "Warp proc %lld us\n", (long long)gT_end.tv_sec * 1000000 + (long long)gT_end.tv_nsec/1000 - (long long)gT_start.tv_sec*1000000 - (long long)gT_start.tv_nsec/1000);
#endif

    // get warp IRQ status from target.
    irq_ceva = (u32)ceva_hal_get_irq_status(&handle->ceva_hal_handle);

    // do nothing if WARP IRQ is not triggered
    if((irq_ceva & handle->ceva_irq) == 0)
    {
        if (irq_ceva == 0)
        {
            DEV_MSG(DEV_MSG_ERR, "unexpected irq: 0x%X (0x%X), status = 0x%X\n", irq_ceva, handle->ceva_irq, irq_status);
        }
        return handle->state;
    }

    // check and clear IRQ status
    warp_hal_get_hw_status(&handle->hal_handle);                    //read status register (read /clear)
    irq_status = (u32)handle->hal_handle.reg_bank.sta.fields.oc;    //operation complete bit

    DEV_MSG(DEV_MSG_DBG, "handle: 0x%p, irq: 0x%X (0x%X), status = 0x%X\n", handle, irq_ceva, handle->ceva_irq, irq_status);

    // do nothing if WARP internal status is not enabled
    if (!irq_status)
    {
        DEV_MSG(DEV_MSG_ERR, "WARP status is not triggered: 0x%X (0x%X), status = 0x%X\n", irq_ceva, handle->ceva_irq, irq_status);
        return handle->state;
    }

    // Check AXI bus error bit of satus register
    if (handle->hal_handle.reg_bank.sta.fields.axierr) // we should use HAL function, not check register directly
    {
        DEV_MSG(DEV_MSG_ERR, "AXI ERROR: 0x%X (0x%X), status = 0x%X, err = 0x%X\n", irq_ceva, handle->ceva_irq, irq_status, handle->hal_handle.reg_bank.sta.fields.axierr);
        handle->state = WARP_DEV_STATE_AXI_ERROR;
    } else {
        handle->state = WARP_DEV_STATE_DONE;
    }

    DEV_MSG(DEV_MSG_DBG, "start post proc handle: 0x%p\n", handle);

    INIT_WORK(&handle->work_queue, warp_dev_post_proc);
    schedule_work(&handle->work_queue);

    return handle->state;
}

/*******************************************************************************************************************
 * warp_dev_destroy
 *   Release WARP settings
 *
 * Parameters:
 *   handle: device handle
 *
 * Return:
 *   none
 */
s32 warp_dev_destroy(MHAL_WARP_DEV_HANDLE device)
{
    warp_dev_handle *handle = (warp_dev_handle*)device;

    if (handle->state != WARP_DEV_STATE_READY)
        return E_MHAL_ERR_BUSY;

    mutex_destroy(&handle->mutex);
    kfree(handle);

    return MHAL_SUCCESS;
}
/*******************************************************************************************************************
 * warp_dev_create
 *   create WARP device
 *
 * Parameters:
 *   handle: device handle
 *   pdev:          platform device
 *   base_addr:    base addr of HW register bank
 *
 * Return:
 *   none
 */
s32 warp_dev_create(MHAL_WARP_DEV_HANDLE *device, phys_addr_t base_warp, phys_addr_t base_sys, phys_addr_t base_axi2miu0, phys_addr_t base_axi2miu1, phys_addr_t base_axi2miu2, phys_addr_t base_axi2miu3)
{
    warp_dev_handle *handle;

    handle = kcalloc(1, sizeof(warp_dev_handle), GFP_KERNEL);
    if (handle == NULL)
    {
        DEV_MSG(DEV_MSG_ERR, "can't allocate buffer for device\n");
        return E_MHAL_ERR_NOMEM;
    }

    DEV_MSG(DEV_MSG_DBG, "addr 0x%08X\n", base_warp);

    memset(handle, 0, sizeof(warp_dev_handle));

    //initialize hal_handle, setting physical address
    warp_hal_init(&handle->hal_handle, base_warp);
    ceva_hal_init(&handle->ceva_hal_handle, base_sys, base_axi2miu0, base_axi2miu1, base_axi2miu2, base_axi2miu3);

    //set warp IRQ for ceva wrapper
    handle->ceva_irq =  CEVA_HAL_IRQ_WARP;

    //initialize request list
    INIT_LIST_HEAD(&handle->request_list);
    DEV_MSG(DEV_MSG_DBG, "list_empty = %d\n", list_empty(&handle->request_list));

    // enable IRQ
    // ceva_hal_enable_irq(&handle->ceva_hal_handle,  handle->ceva_irq_target,  handle->ceva_irq);

    handle->state = WARP_DEV_STATE_READY;
    mutex_init(&handle->mutex);

    *device = (MHAL_WARP_DEV_HANDLE)handle;

    return MHAL_SUCCESS;
}
