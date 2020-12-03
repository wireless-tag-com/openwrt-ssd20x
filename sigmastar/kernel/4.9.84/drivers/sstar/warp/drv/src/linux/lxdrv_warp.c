/*
* lxdrv_warp.c- Sigmastar
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
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/spinlock.h>

#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/sched.h>

#include <linux/clk.h>
#include <linux/clk-provider.h>

#include <linux/mm.h>
#include <linux/dma-mapping.h>

#include "lxdrv_warp.h"
#include "mdrv_warp_io.h"

#include "dev_debug.h"

#define STAND_ALONE  (1)    //trun on/off module_init and module_exit

#define WARP_LXDRV_DEVICE_COUNT   (1) // How many device will be installed
#define WARP_LXDRV_NAME           "mstar_warp"
#define WARP_LXDRV_MINOR          (0)
#define WARP_LXDRV_CLASS_NAME     "mstar_warp_class"

/*******************************************************************************************************************
 * warp_lxdrv_post_proc
 *   Post process after IRQ triggered
 *
 * Parameters:
 *   device:
 *
 *
 * Return:
 *
 */
MS_BOOL warp_lxdrv_post_proc(MHAL_WARP_INST_HANDLE instance, void *user_data)
{
    warp_file_data *file_data = (warp_file_data*)user_data;
    MHAL_WARP_INSTANCE_STATE_E state;

    state = MHAL_WARP_CheckState(file_data->instance);
    if (state != MHAL_WARP_INSTANCE_STATE_READY)
    {
        DEV_MSG(DEV_MSG_ERR, "unexpect state %d\n", state);
    }

    // set ready and wake up waiting thread/process
    wake_up_interruptible(&file_data->wait_queue);

    return 0;
}

/*******************************************************************************************************************
 * warp_lxdrv_isr_proc
 *   ISR handler
 *
 * Parameters:
 *   irq:      IRQ
 *   dev_data: Device data which is assigned from request_irq()
 *
 * Return:
 *   Always IRQ_HANDLED to stop parsing ISR
 */
irqreturn_t  warp_lxdrv_isr_proc(s32 irq, void* data)
{
    warp_dev_data  *dev_data = (warp_dev_data*)data;
    MHAL_WARP_ISR_STATE_E state;

    state = MHAL_WARP_IsrProc(dev_data->dev_handle);
    switch(state)
    {
        case MHAL_WARP_ISR_STATE_DONE:
            return IRQ_HANDLED;

        default:
            return IRQ_NONE;
    }

    return IRQ_NONE;
}

/*******************************************************************************************************************
 * warp_lxdrv_open
 *   File open handler
 *   The device can has a instance at the same time, and the open
 *   operator also enable the request ISR.
 *
 * Parameters:
 *   inode: inode
 *   filp:  file structure
 *
 * Return:
 *   standard return value
 */

static s32 warp_lxdrv_open(struct inode *inode, struct file *filp)
{
    warp_dev_data    *dev_data;
    warp_file_data  *file_data;
    s32 err = 0;

    //get dev_data struct pointer
    dev_data = container_of(inode->i_cdev, warp_dev_data, cdev);

    //allocate buffer for file_data
    file_data = devm_kcalloc(&dev_data->pdev->dev, 1, sizeof(warp_file_data), GFP_KERNEL);
    if (file_data == NULL)
    {
         DEV_MSG(DEV_MSG_ERR, "error: can't allocate buffer\n");
         return -ENOSPC; //?
    }

    DEV_MSG(DEV_MSG_DBG, "filp: 0x%p, file_data: 0x%p\n", filp, file_data);

    //Assgin dev_data and keep file_data in the file structure
    if (MHAL_SUCCESS != MHAL_WARP_CreateInstance(dev_data->dev_handle, &file_data->instance))
    {
        err = -EIO;
        goto ERROR_1;
    }

    DEV_MSG(DEV_MSG_DBG, "filp: 0x%p, file_data: 0x%p, instance: 0x%p\n", filp, file_data, file_data->instance);

    file_data->dev_data = dev_data;
    filp->private_data = file_data;

    //Init wait queue
    init_waitqueue_head(&file_data->wait_queue);

    return 0;

ERROR_1:
    devm_kfree(&dev_data->pdev->dev, file_data);
    return err;
}

/*******************************************************************************************************************
 * warp_lxdrv_release
 *   File close handler
 *   The operator will release ISR
 *
 * Parameters:
 *   inode: inode
 *   filp:  file structure
 *
 * Return:
 *   standard return value
 */
static s32 warp_lxdrv_release(struct inode *inode, struct file *filp)
{
    warp_file_data *file_data;
    warp_dev_data  *dev_data ;

    file_data = (warp_file_data*)filp->private_data;
    dev_data = file_data->dev_data;

    DEV_MSG(DEV_MSG_DBG, "filp: 0x%p\n", filp);

    MHAL_WARP_DestroyInstance(file_data->instance);

    // Release memory
    devm_kfree(&dev_data->pdev->dev, file_data);

    return 0;
}
/*******************************************************************************************************************
 * warp_lxdrv_ioctl_trigger
 *   IOCTL handler for WARP_IOC_TRIGGER
 *
 * Parameters:
 *   file_data: file private data
 *   arg:       argument, a pointer of warp_lxdrv_ioctl from userspace
 *
 * Return:
 *   0 indicates success, others indicate failure
 */
static s32 warp_lxdrv_ioctl_trigger(warp_file_data *file_data, unsigned long arg)
{
    unsigned long proc_config_status = 0;
    MHAL_WARP_CONFIG config;

    if (MHAL_WARP_INSTANCE_STATE_READY != MHAL_WARP_CheckState(file_data->instance))
    {
        DEV_MSG(DEV_MSG_ERR, "One instance can request once at the same time only\n");
        return E_MHAL_ERR_BUSY;
    }

    //get config setting from user space
    proc_config_status = copy_from_user(&config, (void*)arg, sizeof(MHAL_WARP_CONFIG) );
    if(proc_config_status != 0)
    {
        DEV_MSG(DEV_MSG_ERR, "Can't copy config from user space\n");
        return E_MHAL_ERR_ILLEGAL_PARAM;
    }

    if (MHAL_SUCCESS != MHAL_WARP_Trigger(file_data->instance, &config, warp_lxdrv_post_proc, (void*)file_data))
    {
        DEV_MSG(DEV_MSG_ERR, "Can't process warp\n");
        return E_MHAL_ERR_NOT_CONFIG;
    }

    return MHAL_SUCCESS;

}

/*******************************************************************************************************************
 * warp_lxdrv_ioctl
 *   IOCTL handler entry for file operator
 *
 * Parameters:
 *   filp: pointer of file structure
 *   cmd:  command
 *   arg:  argument from user space
 *
 * Return:
 *   standard return value
 */
long warp_lxdrv_ioctl(struct file *filp, u32 cmd, unsigned long arg)
{
    warp_file_data *file_data;
    warp_dev_data  *dev_data;
    s32 err = MHAL_SUCCESS;

    //get file_data and device_data struct pointer
    file_data = (warp_file_data*)filp->private_data;
    dev_data = file_data->dev_data;

    DEV_MSG(DEV_MSG_DBG, "filp: 0x%p, file_data: 0x%p\n", filp, file_data);

    // Enter critical section
    mutex_lock(&dev_data->mutex);

    switch(cmd)
    {
        case WARP_IOC_TRIGGER:
                err = warp_lxdrv_ioctl_trigger(file_data, arg);
                break;

        default:
                err = ESRCH;
                break;
    }

//RETURN:
    // Leave critical section
    mutex_unlock(&dev_data->mutex);

    return err;
}

/*******************************************************************************************************************
 * warp_lxdrv_ioctl
 *   poll handler entry for file operator
 *
 * Parameters:
 *   filp: pointer of file structure
 *   wait: wait queue
 *
 * Return:
 *   only 0 or POLLIN | POLLRDNORM
 */
static u32 warp_lxdrv_poll(struct file *filp, struct poll_table_struct *wait)
{
    warp_file_data *file_data = (warp_file_data*)filp->private_data;
    MHAL_WARP_INSTANCE_STATE_E state;

    DEV_MSG(DEV_MSG_DBG, "polling, file_data 0x%p, instance 0x%p\n", file_data, file_data->instance);

    state = MHAL_WARP_CheckState(file_data->instance);

    DEV_MSG(DEV_MSG_DBG, "polling 0x%p 0x%X\n", &file_data->wait_queue, state);

    // Check before wait
    switch(state )
    {
        case MHAL_WARP_INSTANCE_STATE_DONE:
            DEV_MSG(DEV_MSG_DBG, "Operation ready before polling\n");
            MHAL_WARP_ReadyForNext(file_data->instance);
            return POLLIN | POLLRDNORM;

        case MHAL_WARP_INSTANCE_STATE_READY:
            DEV_MSG(DEV_MSG_DBG, "Operation ready before polling\n");
            return POLLIN | POLLRDNORM;

        case MHAL_WARP_INSTANCE_STATE_AXI_ERROR:
        case MHAL_WARP_INSTANCE_STATE_PROC_ERROR:
            DEV_MSG(DEV_MSG_ERR, "AXI error occurs (occurs before polling)\n");
            MHAL_WARP_ReadyForNext(file_data->instance);
            return POLLERR;

        case MHAL_WARP_INSTANCE_STATE_PROCESSING:
            break;

        default:
            DEV_MSG(DEV_MSG_ERR, "unexpect state %d (occurs before polling)\n", state);
            break;
    }

    poll_wait(filp, &file_data->wait_queue, wait); //add file_data to wait list, then block until HW finish (isr_post_process)

    state = MHAL_WARP_CheckState(file_data->instance);

    // Check after wait
    switch(state )
    {
        case MHAL_WARP_INSTANCE_STATE_DONE:
            DEV_MSG(DEV_MSG_DBG, "Operation ready after polling\n");
            MHAL_WARP_ReadyForNext(file_data->instance);
            return POLLIN | POLLRDNORM;

        case MHAL_WARP_INSTANCE_STATE_AXI_ERROR:
        case MHAL_WARP_INSTANCE_STATE_PROC_ERROR:
            DEV_MSG(DEV_MSG_ERR, "AXI error occurs\n");
            MHAL_WARP_ReadyForNext(file_data->instance);
            return POLLERR;

        case MHAL_WARP_INSTANCE_STATE_PROCESSING:
            break;

        default:
            DEV_MSG(DEV_MSG_ERR, "unexpect state %d\n", state);
            break;
    }

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Platform functions
//-------------------------------------------------------------------------------------------------
// Use a struct to gather all global variable
static struct
{
    s32 major;              // cdev major number
    s32 minor_star;         // begining of cdev minor number
    s32 reg_count;          // registered count
    struct class *class;    //class pointer
} g_warp_drv = {0, 0, 0, NULL};

static const struct file_operations warp_fops = {
    .owner          = THIS_MODULE,
    .open           = warp_lxdrv_open,
    .release        = warp_lxdrv_release,
    .unlocked_ioctl = warp_lxdrv_ioctl,
    .poll           = warp_lxdrv_poll,
};

/*******************************************************************************************************************
 * warp_lxdrv_probe
 *   Platform device prob handler
 *
 * Parameters:
 *   pdev: platfrom device
 *
 * Return:
 *   standard return value
 */
static s32 warp_lxdrv_probe(struct platform_device *pdev)
{
    s32 err;
    warp_dev_data *dev_data;
    struct device *dev;

    // create drv data buffer
    dev_data = devm_kcalloc(&pdev->dev, 1, sizeof(warp_dev_data), GFP_KERNEL);
    if (dev_data == NULL)
    {
        DEV_MSG(DEV_MSG_ERR, "can't allocate dev data buffer\n");
        return -ENOMEM;
    }
    DEV_MSG(DEV_MSG_DBG, "dev_data: 0x%p (size = %d)\n", dev_data, sizeof(warp_dev_data));

    //Inti mutex
    mutex_init(&dev_data->mutex);

    // Init dev_data
    dev_data->pdev = pdev;

    err = MHAL_WARP_CreateDevice(0, &dev_data->dev_handle);
    if (err != MHAL_SUCCESS)
    {
        DEV_MSG(DEV_MSG_ERR, "can't init device\n");
        err = -ENODEV;
        goto ERROR_1;
    }

    // Retrieve IRQ
    dev_data->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
    if (dev_data->irq < 0)
    {
        DEV_MSG(DEV_MSG_ERR, "can't find IRQ\n");
        err = -ENODEV;
        goto ERROR_1;
    }

    // Register a ISR
    err = request_irq(dev_data->irq, warp_lxdrv_isr_proc, 0, "warp isr", dev_data);
    if (err != 0)
    {
        DEV_MSG(DEV_MSG_ERR, "warp interrupt failed (irq: %d, errno:%d)\n", dev_data->irq, err);
        err = -ENODEV;
        goto ERROR_1;
    }

    // Add cdev
    cdev_init(&dev_data->cdev, &warp_fops);
    err= cdev_add(&dev_data->cdev, MKDEV(g_warp_drv.major, g_warp_drv.minor_star + g_warp_drv.reg_count), 1);
    if (err)
    {
        DEV_MSG(DEV_MSG_ERR, "Unable add a character device\n");
        goto ERROR_2;
    }

    // Create a instance in class
    dev = device_create(g_warp_drv.class,
                        NULL,
                        MKDEV(g_warp_drv.major, g_warp_drv.minor_star + g_warp_drv.reg_count),
                        dev_data,
                        WARP_LXDRV_NAME);

    if (IS_ERR(dev))
    {
        DEV_MSG(DEV_MSG_ERR, "can't create device\n");
        err = -ENODEV;
        goto ERROR_3;
    }

    // Increase registered count
    g_warp_drv.reg_count++;

    dev_set_drvdata(&pdev->dev, dev_data);

    return 0;

ERROR_3:
    cdev_del(&dev_data->cdev);

ERROR_2:
    free_irq(dev_data->irq, dev_data);

ERROR_1:
    devm_kfree(&dev_data->pdev->dev, dev_data);

    return err;
}

/*******************************************************************************************************************
 * warp_lxdrv_remove
 *   Platform device remove handler
 *
 * Parameters:
 *   pdev: platfrom device
 *
 * Return:
 *   standard return value
 */
static s32 warp_lxdrv_remove(struct platform_device *pdev)
{
   warp_dev_data* dev_data;

    dev_data = (warp_dev_data*)dev_get_drvdata(&pdev->dev);

    DEV_MSG(DEV_MSG_DBG, "dev_data: 0x%p\n", dev_data);

    //IRQ release
    free_irq(dev_data->irq, dev_data);

    MHAL_WARP_DestroyDevice(dev_data->dev_handle);

    //instance release
    device_destroy(g_warp_drv.class, dev_data->cdev.dev);

    //cdev delete (fops)
    cdev_del(&dev_data->cdev);

    //dev_data release
    devm_kfree(&dev_data->pdev->dev, dev_data);

    return 0;
}

/*******************************************************************************************************************
 * warp_lxdrv_suspend
 *   Platform device suspend handler, but nothing to do here
 *
 * Parameters:
 *   pdev: platfrom device
 *
 * Return:
 *   standard return value
 */
static s32 warp_lxdrv_suspend(struct platform_device *pdev, pm_message_t state)
{
    warp_dev_data *dev_data = dev_get_drvdata(&pdev->dev);

    DEV_MSG(DEV_MSG_DBG, "dev_data: 0x%p\n", dev_data);

    return 0;
}

/*******************************************************************************************************************
 * warp_lxdrv_resume
 *   Platform device resume handler, but nothing to do here
 *
 * Parameters:
 *   pdev: platfrom device
 *
 * Return:
 *   standard return value
 */
static s32 warp_lxdrv_resume(struct platform_device *pdev)
{
    warp_dev_data *dev_data = dev_get_drvdata(&pdev->dev);

    DEV_MSG(DEV_MSG_DBG, "dev_data: 0x%p\n", dev_data);

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Data strucure for device driver
//-------------------------------------------------------------------------------------------------
static const struct of_device_id warp_lxdrv_match[] = {
    {
        .compatible = "sstar,infinity2-warp",
        /*.data = NULL,*/
    },
    {},
};

static struct platform_driver warp_lxdrv_driver = {
    .probe      = warp_lxdrv_probe,
    .remove     = warp_lxdrv_remove,
    .suspend    = warp_lxdrv_suspend,
    .resume     = warp_lxdrv_resume,

    .driver = {
        .of_match_table = of_match_ptr(warp_lxdrv_match),
        .name   = "mstar_warp",
        .owner  = THIS_MODULE,
    }
};

/*************************************************************************************************
 * warp_lxdrv_module_init
 *   module init function
 *
 * Parameters:
 *   N/A
 *
 * Return:
 *   standard return value
 */
s32 warp_lxdrv_module_init(void)
{
    s32 err;
    dev_t dev;

    DEV_MSG(DEV_MSG_DBG, "Moudle Init\n");

    // Allocate cdev id
    err = alloc_chrdev_region(&dev, WARP_LXDRV_MINOR, WARP_LXDRV_DEVICE_COUNT, WARP_LXDRV_NAME);
    if (err)
    {
        DEV_MSG(DEV_MSG_ERR, "Unable allocate cdev id\n");
        return err;
    }

    g_warp_drv.major = MAJOR(dev);
    g_warp_drv.minor_star = MINOR(dev);
    g_warp_drv.reg_count = 0;

    // Register device class
    g_warp_drv.class = class_create(THIS_MODULE, WARP_LXDRV_CLASS_NAME);

    if (IS_ERR(g_warp_drv.class))
    {
        DEV_MSG(DEV_MSG_ERR, "Failed at class_create().Please exec [mknod] before operate the device\n");
        err = PTR_ERR(g_warp_drv.class);
        goto ERR_RETURN_1;
    }

    // Register platform driver
    err = platform_driver_register(&warp_lxdrv_driver);
    if (err != 0)
    {
        DEV_MSG(DEV_MSG_ERR, "Fail at platform_driver_register().\n");
        goto ERR_RETURN_2;
    }

    DEV_MSG(DEV_MSG_DBG, "pass all warp_lxdrv_module_init function. \n");

    return 0;

ERR_RETURN_2:
    class_destroy(g_warp_drv.class);

ERR_RETURN_1:
    unregister_chrdev_region(MKDEV(g_warp_drv.major, g_warp_drv.minor_star), WARP_LXDRV_DEVICE_COUNT);

    return err;
}

/*******************************************************************************************************************
 * warp_lxdrv_module_exit
 *   module exit function
 *
 * Parameters:
 *   N/A
 *
 * Return:
 *   standard return value
 */
void warp_lxdrv_module_exit(void)
{
    /*de-initial the who GFLIPDriver */
    DEV_MSG(DEV_MSG_DBG, "Modules Exit\n");

    platform_driver_unregister(&warp_lxdrv_driver);
    class_destroy(g_warp_drv.class);
    unregister_chrdev_region(MKDEV(g_warp_drv.major, g_warp_drv.minor_star), WARP_LXDRV_DEVICE_COUNT);
}

#if STAND_ALONE
module_init(warp_lxdrv_module_init);
module_exit(warp_lxdrv_module_exit);
#endif

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("WARP ioctrl driver");
MODULE_LICENSE("GPL");
