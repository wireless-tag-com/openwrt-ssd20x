/*
* hal_ive_simulate.c- Sigmastar
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
#include "hal_ive_simulate.h"

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/list.h>
// #include <string.h>

struct ive_simulation {
    struct list_head    list;
    struct work_struct  work_queue;
    ive_dev_data        *dev_data;
};

#if defined(IVE_SW_SIMULATE)

// We declare the function here because this API should not extern by default
void mdrv_ive_drv_isr_post_proc(struct work_struct *wq);

// Static work queue data
#define DATA_SIZE (10)
static struct ive_simulation g_list_data[DATA_SIZE];
static LIST_HEAD(g_work_queue_list);

/*******************************************************************************************************************
 * __get_work_queue_data
 *   Get a work queue data for simulation
 *   We need the paired API because work buffer can't free before work queue done
 *   Therefor a static linked list to prepared the work queue structure & relative data
 *
 * Parameters:
 *   none
 *
 * Return:
 *   work queue data
 */
static struct ive_simulation* __get_work_queue_data(void)
{
    int i;

    struct ive_simulation *list_data = NULL;

    // init g_work_queue_list
    if (list_empty(&g_work_queue_list)) {
        memset(g_list_data, 0, sizeof(g_list_data));
        IVE_MSG(IVE_MSG_DBG, "add work queue list\n");
        for (i=0; i<DATA_SIZE; i++) {
            list_add(&g_list_data[i].list, &g_work_queue_list);
        }
    }

    list_data = list_first_entry(&g_work_queue_list, struct ive_simulation, list);
    if (list_data != NULL) {
        list_del(g_work_queue_list.next);
    } else {
        IVE_MSG(IVE_MSG_ERR, "can't get work queue data\n");
    }

    return list_data;
}

/*******************************************************************************************************************
 * __return_work_queue_data
 *   return a work queue data for simulation
 *   We need the paired API because work buffer can't free before work queue done
 *   Therefor a static linked list to prepared the work queue structure & relative data
 *
 * Parameters:
 *   list_data: work queue data
 *
 * Return:
 *   none
 */
static void __return_work_queue_data(struct ive_simulation* list_data)
{
    list_add_tail(&list_data->list, &g_work_queue_list);
}

/*******************************************************************************************************************
 * ive_hal_op_simulate
 *   A dummy simulation function.
 *   There is nothing to do because work queue is a kernel thread and has no way
 *   to get data from user space.
 *
 * Parameters:
 *   handle: hal handle
 *
 * Return:
 *   none
 */
static void ive_hal_op_simulate(ive_hal_handle *handle)
{
#define BLOCK_SIZE (64*1024)

    int i, length = handle->reg_bank1.frame_width * handle->reg_bank1.frame_height;
    void *user_src0, *user_src1, *user_dst;
    u16 weight_x, weight_y;

    weight_x = handle->reg_bank1.add_weight_x;
    weight_y = handle->reg_bank1.add_weight_y;

    user_src0 = (void*)(handle->reg_bank1.src1_addr_high<<16 | handle->reg_bank1.src1_addr_low);
    user_src1 = (void*)(handle->reg_bank1.src2_addr_high<<16 | handle->reg_bank1.src2_addr_low);
    user_dst  = (void*)(handle->reg_bank1.dst1_addr_high<<16 | handle->reg_bank1.dst1_addr_low);

    IVE_MSG(IVE_MSG_DBG, "0x%p, 0x%p, 0x%p (%d x %d)\n", user_src0, user_src1, user_dst, handle->reg_bank1.frame_width, handle->reg_bank1.frame_height);

    for (i=0; i<length; i+=BLOCK_SIZE) {
        // re-calculate the last block size
        IVE_MSG(IVE_MSG_DBG, "run %d\n", i);
        msleep(10);
    }

    IVE_MSG(IVE_MSG_DBG, "end\n");
}

/*******************************************************************************************************************
 * mdrv_ive_drv_isr_simulate
 *   A simulation of mdrv_ive_drv_isr().
 *
 * Parameters:
 *   wq: work queue
 *
 * Return:
 *   none
 */
void mdrv_ive_drv_isr_simulate(struct work_struct *wq)
{
    struct ive_simulation  *simulation = container_of(wq, struct ive_simulation, work_queue);
    ive_dev_data *dev_data = simulation->dev_data;

    ive_hal_op_simulate(&dev_data->drv_handle.hal_handle);

    ive_drv_isr_handler(0, &dev_data->drv_handle);
    mdrv_ive_drv_isr_post_proc(&dev_data->work_queue);

    __return_work_queue_data(simulation);
}

/*******************************************************************************************************************
 * ive_hal_run_simulate
 *   A simulation for HW start.
 *
 * Parameters:
 *   handle: hal handle
 *
 * Return:
 *   none
 */
 void ive_hal_run_simulate(ive_hal_handle *handle)
{
    ive_drv_handle *drv_handle = container_of(handle, ive_drv_handle, hal_handle);
    ive_dev_data *dev_data = container_of(drv_handle, ive_dev_data, drv_handle);
    // struct ive_simulation *simulation = kzalloc(sizeof(struct ive_simulation), GFP_KERNEL);
    struct ive_simulation *simulation = __get_work_queue_data();

    IVE_MSG(IVE_MSG_DBG, "run simulation, dev_data = 0x%p\n", dev_data);

    simulation->dev_data = dev_data;
    INIT_WORK(&simulation->work_queue, mdrv_ive_drv_isr_simulate);
    schedule_work(&simulation->work_queue);
}
#endif
