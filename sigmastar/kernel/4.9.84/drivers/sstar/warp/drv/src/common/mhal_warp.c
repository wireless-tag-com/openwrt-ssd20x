/*
* mhal_warp.c- Sigmastar
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
#include "mhal_warp.h"

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mutex.h>

#include "infinity2/irqs.h"
#include "dev_warp.h"
#include "dev_debug.h"

#define RIU_BASE_ADDR   (0x1F000000)
#define BANK_CAL(addr)  ((addr<<9) + (RIU_BASE_ADDR))

#define BANK_CEVA_SYS0  (BANK_CAL(0x1128))
#define BANK_CEVA_BUS0  (BANK_CAL(0x1126))
#define BANK_CEVA_BUS1  (BANK_CAL(0x1127))
#define BANK_CWVA_WARP  (BANK_CAL(0x1A24))

typedef struct
{
    MHAL_WARP_DEV_HANDLE        device;
    MHAL_WARP_INSTANCE_STATE_E  state;
    MHAL_WARP_CONFIG            config;
    MHAL_WARP_CALLBACK          callback;
    void                        *user_data;
    struct mutex                mutex;          // for critical section
} mhal_warp_instance;

static void dump_config(MHAL_WARP_CONFIG* config)
{
    DEV_MSG(DEV_MSG_DBG, "config %p\n", config);

    DEV_MSG(DEV_MSG_DBG, ".op_mode = %d,\n", config->op_mode);
    DEV_MSG(DEV_MSG_DBG, ".disp_table = {%d, %d, %d, 0x%p},\n", config->disp_table.resolution, config->disp_table.format,config->disp_table.size, (void*)(MS_U32)config->disp_table.table);
    DEV_MSG(DEV_MSG_DBG, ".bb_table = {%d, 0x%p},\n", config->bb_table.size,(void*)(MS_U32)config->bb_table.table);
    DEV_MSG(DEV_MSG_DBG, ".coeff = {%d, %d, %d, %d, %d, %d, %d, %d, %d},\n", config->coeff[0], config->coeff[1], config->coeff[2], config->coeff[3], config->coeff[4], config->coeff[5], config->coeff[6], config->coeff[7], config->coeff[8]);
    DEV_MSG(DEV_MSG_DBG, ".input_image = {%d, %d, %d},\n", config->input_image.format, config->input_image.width, config->input_image.height);
    DEV_MSG(DEV_MSG_DBG, ".input_data = {%d, 0x%p, 0x%p},\n", config->input_data.num_planes, (void*)(MS_U32)config->input_data.data[0], (void*)(MS_U32)config->input_data.data[1]);
    DEV_MSG(DEV_MSG_DBG, ".output_image = {%d, %d, %d},\n", config->output_image.format, config->output_image.width, config->output_image.height);
    DEV_MSG(DEV_MSG_DBG, ".output_data = {%d, 0x%p, 0x%p},\n", config->output_data.num_planes, (void*)(MS_U32)config->output_data.data[0], (void*)(MS_U32)config->output_data.data[1]);
    DEV_MSG(DEV_MSG_DBG, ".fill_value = {%d, %d},\n", config->fill_value[0], config->fill_value[1]);
}

MS_U32 MHAL_WARP_GetIrqNum(void)
{
    MS_U32 irqid = 0;
    irqid = INT_IRQ_78_WARP2RIU_INT+32;
    return irqid;
}

MS_S32 MHAL_WARP_CreateDevice(MS_U32 id, MHAL_WARP_DEV_HANDLE *device)
{
    s32 ret;

    ret = warp_dev_create(device, BANK_CWVA_WARP, BANK_CEVA_SYS0, BANK_CEVA_BUS0, BANK_CEVA_BUS0 + 0x100, BANK_CEVA_BUS1, BANK_CEVA_BUS1 + 0x100);
    if (ret != 0)
    {
        DEV_MSG(DEV_MSG_ERR, "failed to init device\n");
        return E_MHAL_ERR_INVALID_DEVID;
    }

    DEV_MSG(DEV_MSG_DBG, "device buffer 0x%p\n", *device);

    return MHAL_SUCCESS;
}

MS_S32 MHAL_WARP_DestroyDevice(MHAL_WARP_DEV_HANDLE device)
{
    return warp_dev_destroy(device);
}

MS_S32 MHAL_WARP_CreateInstance(MHAL_WARP_DEV_HANDLE device, MHAL_WARP_INST_HANDLE *instance)
{
    mhal_warp_instance *inst_handle;

    inst_handle = kcalloc(1, sizeof(mhal_warp_instance), GFP_KERNEL);
    if (inst_handle == NULL)
    {
        DEV_MSG(DEV_MSG_ERR, "can't allocate buffer for instance\n");
        return E_MHAL_ERR_NOMEM;
    }

    DEV_MSG(DEV_MSG_DBG, "device: 0x%p, instance buffer 0x%p\n", device, inst_handle);

    inst_handle->device = device;
    inst_handle->state = MHAL_WARP_INSTANCE_STATE_READY;
    mutex_init(&inst_handle->mutex);

    *instance = (MHAL_WARP_INST_HANDLE)inst_handle;

    return MHAL_SUCCESS;
}

MS_S32 MHAL_WARP_DestroyInstance(MHAL_WARP_INST_HANDLE instance)
{
    mhal_warp_instance *inst_handle = (mhal_warp_instance*)instance;

    DEV_MSG(DEV_MSG_DBG, "destroy instance 0x%p\n", inst_handle);

    switch (MHAL_WARP_CheckState(instance))
    {
        case MHAL_WARP_INSTANCE_STATE_PROCESSING:
        case MHAL_WARP_INSTANCE_STATE_DONE:
            DEV_MSG(DEV_MSG_ERR, "instance 0x%p is still processing\n", inst_handle);
            return E_MHAL_ERR_BUSY;

       default:
            break;
    }

    mutex_destroy(&inst_handle->mutex);
    kfree(inst_handle);

    return MHAL_SUCCESS;
}

void MHAL_WARP_PostProc(MHAL_WARP_DEV_HANDLE device, void *user_data, WARP_DEV_STATE dev_state)
{
    mhal_warp_instance *inst_handle = (mhal_warp_instance*)user_data;

    if (inst_handle == NULL)
    {
        DEV_MSG(DEV_MSG_ERR, "can't get instance\n");
        return;
    }

    DEV_MSG(DEV_MSG_DBG, "post process instance 0x%p\n", inst_handle);

    mutex_lock(&inst_handle->mutex);

    switch (dev_state)
    {
        case WARP_DEV_STATE_DONE:
            inst_handle->state = MHAL_WARP_INSTANCE_STATE_DONE;
            break;

        case WARP_DEV_STATE_READY:
            inst_handle->state = MHAL_WARP_INSTANCE_STATE_READY;
            break;

        case WARP_DEV_STATE_AXI_ERROR:
        default:
            inst_handle->state = MHAL_WARP_INSTANCE_STATE_AXI_ERROR;
            break;

    }

    mutex_unlock(&inst_handle->mutex);

    DEV_MSG(DEV_MSG_DBG, "instance 0x%p state is %d\n", inst_handle, inst_handle->state);

    if (inst_handle->callback)
    {
        if (inst_handle->callback((MHAL_WARP_INST_HANDLE)inst_handle, inst_handle->user_data))
        {
            MHAL_WARP_ReadyForNext((MHAL_WARP_INST_HANDLE)inst_handle);
        }
    }
}


MS_S32 MHAL_WARP_Trigger(MHAL_WARP_INST_HANDLE instance, MHAL_WARP_CONFIG* config, MHAL_WARP_CALLBACK callback, void *user_data)
{
    mhal_warp_instance *inst_handle = (mhal_warp_instance*)instance;
    WARP_DEV_PROC_STATE_E ret;

    mutex_lock(&inst_handle->mutex);

    // Process can be start only if instance is READY
    if (inst_handle->state != MHAL_WARP_INSTANCE_STATE_READY)
    {
        DEV_MSG(DEV_MSG_ERR, "instance 0x%p is not ready to service\n", inst_handle);
        mutex_unlock(&inst_handle->mutex);
        return E_MHAL_ERR_BUSY;
    }

    DEV_MSG(DEV_MSG_DBG, "instance 0x%p start process\n", inst_handle);

    // dump config (debug only)
    dump_config(config);

    memcpy(&inst_handle->config, config, sizeof(MHAL_WARP_CONFIG));
    inst_handle->callback = callback;
    inst_handle->user_data = user_data;

    ret = warp_dev_trigger(inst_handle->device, &inst_handle->config, MHAL_WARP_PostProc, inst_handle);
    if (ret != WARP_DEV_PROC_STATE_OK)
    {
        DEV_MSG(DEV_MSG_ERR, "failed to process instance 0x%p, err no. 0x%X\n", inst_handle, ret);
        mutex_unlock(&inst_handle->mutex);
        return E_MHAL_ERR_NOT_CONFIG;
    }

    inst_handle->state = MHAL_WARP_INSTANCE_STATE_PROCESSING;

    mutex_unlock(&inst_handle->mutex);

    return MHAL_SUCCESS;
}

MHAL_WARP_INSTANCE_STATE_E MHAL_WARP_CheckState(MHAL_WARP_INST_HANDLE instance)
{
    mhal_warp_instance *inst_handle = (mhal_warp_instance*)instance;

    DEV_MSG(DEV_MSG_DBG, "instance 0x%p state is %d\n", inst_handle, inst_handle->state);

    return inst_handle->state;
}

MHAL_WARP_INSTANCE_STATE_E MHAL_WARP_ReadyForNext(MHAL_WARP_INST_HANDLE instance)
{
    mhal_warp_instance *inst_handle = (mhal_warp_instance*)instance;

    // We seperate state of DONE and READY for multi-instance.
    // All instance can service one process only, and other request should be wait until state read is set.
    // Linux kernel calls poll as a barrier before because we change the state READY here
    if (inst_handle->state == MHAL_WARP_INSTANCE_STATE_DONE)
    {
        mutex_lock(&inst_handle->mutex);

        DEV_MSG(DEV_MSG_DBG, "change instance 0x%p state to ready\n", inst_handle);
        inst_handle->state = MHAL_WARP_INSTANCE_STATE_READY;

        mutex_unlock(&inst_handle->mutex);
    }

    return inst_handle->state;
}


MHAL_WARP_ISR_STATE_E MHAL_WARP_IsrProc(MHAL_WARP_DEV_HANDLE device)
{
    WARP_DEV_STATE state;

    DEV_MSG(DEV_MSG_DBG, "ISR for device 0x%p\n", device);

    state = warp_dev_isr_proc(device);
    if (state == WARP_DEV_STATE_DONE)
    {
        return MHAL_WARP_ISR_STATE_DONE;
    }

    return MHAL_WARP_ISR_STATE_PROCESSING;
}

#if 0
EXPORT_SYMBOL(MHAL_WARP_GetIrqNum);
EXPORT_SYMBOL(MHAL_WARP_CreateDevice);
EXPORT_SYMBOL(MHAL_WARP_DestroyDevice);
EXPORT_SYMBOL(MHAL_WARP_CreateInstance);
EXPORT_SYMBOL(MHAL_WARP_DestroyInstance);
EXPORT_SYMBOL(MHAL_WARP_PostProc);
EXPORT_SYMBOL(MHAL_WARP_Trigger);
EXPORT_SYMBOL(MHAL_WARP_CheckState);
EXPORT_SYMBOL(MHAL_WARP_ReadyForNext);
EXPORT_SYMBOL(MHAL_WARP_IsrProc);
#endif