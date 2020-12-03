/*
* mdrv_ive.c- Sigmastar
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

// #include <linux/module.h>
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

#include "mdrv_ive_io.h"
#include "mdrv_ive_io_st.h"
#include "drv_ive.h"

#if defined(CONFIG_CHIP_INFINITY2)
#include "hal_clk.h"
#endif

#define MDRV_IVE_DEVICE_COUNT                   (1) // How many device will be installed
#define MDRV_IVE_NAME           "mstar_ive"
#define MDRV_IVE_MINOR                          (0)
#define MDRV_IVE_CLASS_NAME     "mstar_ive_class"

#if !defined(CONFIG_CHIP_INFINITY2)  //Only used by I5, I6E, except infinity 2
//-------------------------------------------------------------------------------------------------
// Driver Data Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Clock Enable
//-------------------------------------------------------------------------------------------------

/*******************************************************************************************************************
 * mdrv_ive_clock_init
 *   init device clock
 *
 * Parameters:
 *   dev_data:  device data
 *
 * Return:
 *   0: OK, othes: failed
 */
static int mdrv_ive_clock_init(ive_dev_data *dev_data)
{
    int i, ret = 0;
    struct clk *clk_parent;

    IVE_MSG(IVE_MSG_DBG, "init clock\n");

    dev_data->clk_num = of_clk_get_parent_count(dev_data->pdev->dev.of_node);
    if (dev_data->clk_num <= 0) {
        IVE_MSG(IVE_MSG_ERR, "incorrect clock number (%d)\n", dev_data->clk_num);
        return -1;
    }

    IVE_MSG(IVE_MSG_DBG, "dev_data->clk_num = %d\n", dev_data->clk_num);

    dev_data->clk = devm_kcalloc(&dev_data->pdev->dev, 1, (sizeof(struct clk *) * dev_data->clk_num), GFP_KERNEL);
    if (dev_data->clk == NULL) {
        IVE_MSG(IVE_MSG_ERR, "can't allocate buffer for clock\n");
        return -1;
    }

    for (i=0; i<dev_data->clk_num; i++) {
        dev_data->clk[i] = of_clk_get(dev_data->pdev->dev.of_node, i);
        if (IS_ERR(dev_data->clk[i])) {
            IVE_MSG(IVE_MSG_ERR, "can't get clock %s\n", of_clk_get_parent_name(dev_data->pdev->dev.of_node, i));
            ret = -1;
            goto ERROR;
        }

        if (i == 0)
        {
            //clk_parent = clk_get_parent_by_index(dev_data->clk[i], 0);
            clk_parent = clk_hw_get_parent_by_index(__clk_get_hw(dev_data->clk[i]), 0)->clk;
            if(IS_ERR(clk_parent))
            {
                IVE_MSG(IVE_MSG_ERR, "can't get parent clock\n");
                ret = -1;
                goto ERROR;
            }

            /* Set clock parent */
            if (clk_set_parent(dev_data->clk[i], clk_parent))
            {
                IVE_MSG(IVE_MSG_ERR, "can't set parent clock\n");
                ret = -1;
                goto ERROR;
            }
        }
    }

    return ret;

ERROR:
    devm_kfree(&dev_data->pdev->dev, dev_data->clk);
    dev_data->clk = NULL;
    dev_data->clk_num = 0;

    return ret;
}

/*******************************************************************************************************************
 * mdrv_ive_clock_release
 *   release device clock
 *
 * Parameters:
 *   dev_data:  device data
 *
 * Return:
 *   none
 */
static void mdrv_ive_clock_release(ive_dev_data *dev_data)
{
    int i;

    IVE_MSG(IVE_MSG_DBG, "release clock\n");


    for (i=0; i<dev_data->clk_num; i++) {
        clk_put(dev_data->clk[i]);
    }

    devm_kfree(&dev_data->pdev->dev, dev_data->clk);
}

/*******************************************************************************************************************
 * mdrv_ive_clock_en
 *   enable device clock
 *
 * Parameters:
 *   dev_data:  device data
 *
 * Return:
 *   0: OK, othes: failed
 */
static int mdrv_ive_clock_en(ive_dev_data *dev_data)
{
    int i, ret = 0;

    IVE_MSG(IVE_MSG_DBG, "enable clock\n");

    for (i=0; i<dev_data->clk_num; i++) {
        ret = clk_prepare_enable(dev_data->clk[i]);
        if (ret) {
            IVE_MSG(IVE_MSG_ERR, "can't enable clock\n");
            return -1;
        }
    }

    return 0;
}

/*******************************************************************************************************************
 * mdrv_ive_clock_dis
 *   disable device clock
 *
 * Parameters:
 *   dev_data:  device data
 *
 * Return:
 *   0: OK, othes: failed
 */
static void mdrv_ive_clock_dis(ive_dev_data *dev_data)
{
    int i = 0;

    IVE_MSG(IVE_MSG_DBG, "disable clock\n");

    for (i=0; i<dev_data->clk_num; i++) {
        clk_disable_unprepare(dev_data->clk[i]);
    }
}
#endif  //#if !defined(CONFIG_CHIAO_INFINITY2)  //Only used by I5, I6E, except infinity 2

//-------------------------------------------------------------------------------------------------
// File operations
//-------------------------------------------------------------------------------------------------

void mdrv_ive_drv_isr_post_proc(struct work_struct *wq)
{
    ive_dev_data  *dev_data = container_of(wq, ive_dev_data, work_queue);
    ive_file_data *file_data;

    // Enter cirtical section
    mutex_lock(&dev_data->mutex);

    file_data = ive_drv_post_process(&dev_data->drv_handle);

#if !defined(CONFIG_CHIP_INFINITY2)  //Only used by I5, I6E, except infinity 2
    mdrv_ive_clock_dis(dev_data);
#endif

    // Leave critical section
    mutex_unlock(&dev_data->mutex);

    if (file_data == NULL) {
        IVE_MSG(IVE_MSG_ERR, "isr post process get NULL of file_data!!\n");
        return;
    }

    IVE_MSG(IVE_MSG_DBG, "post porcess 0x%p\n", &file_data->wait_queue);

    // set ready and wake up waiting thread/process
    wake_up_interruptible(&file_data->wait_queue);
}

/*******************************************************************************************************************
 * mdrv_ive_drv_isr
 *   ISR handler
 *
 * Parameters:
 *   irq:      IRQ
 *   dev_data: Device data which is assigned from request_irq()
 *
 * Return:
 *   Always IRQ_HANDLED to stop parsing ISR
 */
irqreturn_t  mdrv_ive_drv_isr(int irq, void* data)
{
    ive_dev_data  *dev_data = (ive_dev_data*)data;
    IVE_DRV_STATE  state;

    state = ive_drv_isr_handler(irq, &dev_data->drv_handle);
    switch(state) {
        case IVE_DRV_STATE_DONE:
            INIT_WORK(&dev_data->work_queue, mdrv_ive_drv_isr_post_proc);
            schedule_work(&dev_data->work_queue);
            return IRQ_HANDLED;

        default:
            return IRQ_NONE;
    }

    return IRQ_NONE;
}

/*******************************************************************************************************************
 * mdrv_ive_drv_open
 *   File open handler
 *   The device can has a instance at the same time, and the open
 *   operator also enable the clock and request q ISR.
 *
 * Parameters:
 *   inode: inode
 *   filp:  file structure
 *
 * Return:
 *   standard return value
 */
int mdrv_ive_drv_open(struct inode *inode, struct file *filp)
{
    ive_dev_data    *dev_data = container_of(inode->i_cdev, ive_dev_data, cdev);
    ive_file_data  *file_data;

#if !defined(CONFIG_CHIP_INFINITY2)  //Only used by I5, I6E, except infinity 2
    int err;
#endif

    // allocate buffer
    file_data = devm_kcalloc(&dev_data->pdev->dev, 1, sizeof(ive_file_data), GFP_KERNEL);
    if (file_data == NULL) {
        IVE_MSG(IVE_MSG_ERR, "error: can't allocate buffer\n");
        return -ENOSPC;
    }

    IVE_MSG(IVE_MSG_DBG, "filp: 0x%p, file_data: 0x%p\n", filp, file_data);

    // Assgin dev_data and keep file_data in the file structure
    file_data->state = IVE_FILE_STATE_READY;
    file_data->dev_data = dev_data;
    filp->private_data = file_data;

    // Init wait queue
    init_waitqueue_head(&file_data->wait_queue);

#if !defined(CONFIG_CHIP_INFINITY2)  //Only used by I5, I6E, except infinity 2
    // Enable clock
    err = mdrv_ive_clock_en(dev_data);
    if (err != 0) {
        err = -EIO;
        goto ERROR_1;
    }
#endif

    return 0;

#if !defined(CONFIG_CHIP_INFINITY2)  //Only used by I5, I6E, except infinity 2
ERROR_1:
    devm_kfree(&dev_data->pdev->dev, file_data);

    return err;
#endif
}


/*******************************************************************************************************************
 * mdrv_ive_drv_release
 *   File close handler
 *   The operator will release clock & ISR
 *
 * Parameters:
 *   inode: inode
 *   filp:  file structure
 *
 * Return:
 *   standard return value
 */
int mdrv_ive_drv_release(struct inode *inode, struct file *filp)
{
    ive_file_data *file_data = (ive_file_data*)filp->private_data;
    ive_dev_data  *dev_data = file_data->dev_data;

    IVE_MSG(IVE_MSG_DBG, "filp: 0x%p\n", filp);

    // Release memory
    devm_kfree(&dev_data->pdev->dev, file_data);

    return 0;
}

/*******************************************************************************************************************
 * mdrv_ive_drv_ioctl_process
 *   IOCTL handler for IVE_IOC_PROCESS
 *
 * Parameters:
 *   file_data: file private data
 *   arg:       argument, a pointer of ive_ioc_config from userspace
 *
 * Return:
 *   IVE_IOC_RET
 */
static IVE_IOC_ERROR mdrv_ive_drv_ioctl_process(ive_file_data *file_data, unsigned long arg)
{
    if (file_data->state != IVE_FILE_STATE_READY) {
        IVE_MSG(IVE_MSG_ERR, "One file can request once at the same time only\n");
        return IVE_IOC_ERROR_BUSY;
    }

    if (copy_from_user(&file_data->ioc_config, (void*)arg, sizeof(ive_ioc_config)) != 0) {
        IVE_MSG(IVE_MSG_ERR, "Can't copy config from user space\n");
        return IVE_IOC_ERROR_PROC_CONFIG;
    }

    file_data->user_io_config = (ive_ioc_config*)arg;

    return ive_drv_process(&(file_data->dev_data->drv_handle), file_data);
}

/*******************************************************************************************************************
 * mdrv_ive_drv_ioctl
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
long mdrv_ive_drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    ive_file_data *file_data = (ive_file_data*)filp->private_data;
    ive_dev_data  *dev_data = file_data->dev_data;
    IVE_IOC_ERROR err = IVE_IOC_ERROR_NONE;

    IVE_MSG(IVE_MSG_DBG, "filp: 0x%p, command: 0x%X\n", filp, cmd);

    // Enter cirtical section
    mutex_lock(&dev_data->mutex);

#if !defined(CONFIG_CHIP_INFINITY2)  //Only used by I5, I6E, except infinity 2
    if (mdrv_ive_clock_en(dev_data)) {
        err = IVE_IOC_ERROR_CLK;
        goto RETURN;
    }
#endif

    switch(cmd) {
        case IVE_IOC_PROCESS:
            err = mdrv_ive_drv_ioctl_process(file_data, arg);
            break;

        default:
            err = ESRCH;
            break;
    }

#if !defined(CONFIG_CHIP_INFINITY2)  //Only used by I5, I6E, except infinity 2
RETURN:
#endif

    // Leave critical section
    mutex_unlock(&dev_data->mutex);

    return err;
}

/*******************************************************************************************************************
 * mdrv_ive_drv_ioctl
 *   poll handler entry for file operator
 *
 * Parameters:
 *   filp: pointer of file structure
 *   wait: wait queue
 *
 * Return:
 *   only 0 or POLLIN | POLLRDNORM
 */
static unsigned int mdrv_ive_drv_poll(struct file *filp, struct poll_table_struct *wait)
{
    ive_file_data *file_data = (ive_file_data*)filp->private_data;

    IVE_MSG(IVE_MSG_DBG, "polling 0x%p 0x%X\n", &file_data->wait_queue, file_data->state);

    if (file_data->state == IVE_FILE_STATE_READY)
    {
        return POLLIN | POLLRDNORM;
    }

    poll_wait(filp, &file_data->wait_queue, wait);

    switch(file_data->state)
    {
        case IVE_FILE_STATE_DONE:
            file_data->state = IVE_FILE_STATE_READY;
            return POLLIN | POLLRDNORM;

        default:
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
    int major;              // cdev major number
    int minor_star;         // begining of cdev minor number
    int reg_count;          // registered count
    struct class *class;    // class pointer
} g_ive_drv = {0, 0, 0, NULL};

static const struct file_operations ive_fops = {
	.owner          = THIS_MODULE,
    .open           = mdrv_ive_drv_open,
    .release        = mdrv_ive_drv_release,
    .unlocked_ioctl = mdrv_ive_drv_ioctl,
    .poll           = mdrv_ive_drv_poll,
};

/*******************************************************************************************************************
 * mdrv_ive_drv_probe
 *   Platform device prob handler
 *
 * Parameters:
 *   pdev: platfrom device
 *
 * Return:
 *   standard return value
 */
static int mdrv_ive_drv_probe(struct platform_device *pdev)
{
    int err;
    ive_dev_data *dev_data;
    struct resource *res0, *res1;
    struct device *dev;

    // create drv data buffer
    dev_data = devm_kcalloc(&pdev->dev, 1, sizeof(ive_dev_data), GFP_KERNEL);
    if (dev_data == NULL) {
        IVE_MSG(IVE_MSG_ERR, "can't allocate dev data buffer\n");
        return -ENOMEM;
    }

    IVE_MSG(IVE_MSG_DBG, "dev_data: 0x%p (size = %d)\n", dev_data, sizeof(ive_dev_data));
    // Get base address
    res0 = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    res1 = platform_get_resource(pdev, IORESOURCE_MEM, 1);
    if (res0 == NULL || res1 == NULL ) {
        IVE_MSG(IVE_MSG_ERR, "can't find base address\n");
        err = -ENODEV;
        goto ERROR_1;
    }

    mutex_init(&dev_data->mutex);

    // Init dev_data
    dev_data->pdev = pdev;
    if (ive_drv_init(&dev_data->drv_handle, pdev, res0->start, res1->start) < 0)
    {
        IVE_MSG(IVE_MSG_ERR, "can't init driver\n");
        err = -ENODEV;
        goto ERROR_1;
    }

    // Init clock
#if !defined(CONFIG_CHIP_INFINITY2)  //Only used by I5, I6E, except infinity 2
    if (mdrv_ive_clock_init(dev_data)) {
        IVE_MSG(IVE_MSG_ERR, "can't init clock\n");
        err = -ENODEV;
        goto ERROR_1;
    }
#else  //Only for infinity 2
    if (ive_clk_hal_init()) {
        IVE_MSG(IVE_MSG_ERR, "can't init clock - I2\n");
        err = -ENODEV;
        goto ERROR_1;
    }
#endif

    // Retrieve IRQ
    dev_data->irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
    if (dev_data->irq < 0) {
        IVE_MSG(IVE_MSG_ERR, "can't find IRQ\n");
        err = -ENODEV;
        goto ERROR_2;
    }

    // Register a ISR
    err = request_irq(dev_data->irq, mdrv_ive_drv_isr, IRQ_TYPE_LEVEL_HIGH, "ive isr", dev_data);
    if (err != 0) {
        IVE_MSG(IVE_MSG_ERR, "isp interrupt failed (irq: %d, errno:%d)\n", dev_data->irq, err);
        err = -ENODEV;
        goto ERROR_2;
    }

    // Add cdev
    cdev_init(&dev_data->cdev, &ive_fops);
    err= cdev_add(&dev_data->cdev, MKDEV(g_ive_drv.major, g_ive_drv.minor_star + g_ive_drv.reg_count), 1);
    if (err) {
        IVE_MSG(IVE_MSG_ERR, "Unable add a character device\n");
        goto ERROR_3;
    }

    // Create a instance in class
    dev = device_create(g_ive_drv.class,
                        NULL,
                        MKDEV(g_ive_drv.major, g_ive_drv.minor_star + g_ive_drv.reg_count),
                        dev_data,
                        MDRV_IVE_NAME"%d", g_ive_drv.minor_star + g_ive_drv.reg_count);
    if (IS_ERR(dev)) {
        IVE_MSG(IVE_MSG_ERR, "can't create device\n");
        err = -ENODEV;
        goto ERROR_4;
    }

    // Increase registered count
    g_ive_drv.reg_count++;

    dev_set_drvdata(&pdev->dev, dev_data);

    return 0;

ERROR_4:
    cdev_del(&dev_data->cdev);

ERROR_3:
    free_irq(dev_data->irq, dev_data);

ERROR_2:
#if !defined(CONFIG_CHIP_INFINITY2)  //Only used by I5, I6E, except infinity 2
    mdrv_ive_clock_release(dev_data);
#endif

ERROR_1:
    devm_kfree(&dev_data->pdev->dev, dev_data);

    return err;
}

/*******************************************************************************************************************
 * mdrv_ive_drv_remove
 *   Platform device remove handler
 *
 * Parameters:
 *   pdev: platfrom device
 *
 * Return:
 *   standard return value
 */
static int mdrv_ive_drv_remove(struct platform_device *pdev)
{
    ive_dev_data *dev_data = dev_get_drvdata(&pdev->dev);

    IVE_MSG(IVE_MSG_DBG, "dev_data: 0x%p\n", dev_data);

#if !defined(CONFIG_CHIP_INFINITY2)  //Only used by I5, I6E, except infinity 2
    mdrv_ive_clock_release(dev_data);
#endif

    free_irq(dev_data->irq, dev_data);

    ive_drv_release(&dev_data->drv_handle);

    device_destroy(g_ive_drv.class, dev_data->cdev.dev);
    cdev_del(&dev_data->cdev);

    devm_kfree(&dev_data->pdev->dev, dev_data);

    return 0;
}


/*******************************************************************************************************************
 * mdrv_ive_drv_suspend
 *   Platform device suspend handler, but nothing to do here
 *
 * Parameters:
 *   pdev: platfrom device
 *
 * Return:
 *   standard return value
 */
static int mdrv_ive_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
    ive_dev_data *dev_data = dev_get_drvdata(&pdev->dev);

    IVE_MSG(IVE_MSG_DBG, "dev_data: 0x%p\n", dev_data);

    return 0;
}


/*******************************************************************************************************************
 * mdrv_ive_drv_resume
 *   Platform device resume handler, but nothing to do here
 *
 * Parameters:
 *   pdev: platfrom device
 *
 * Return:
 *   standard return value
 */
static int mdrv_ive_drv_resume(struct platform_device *pdev)
{
    ive_dev_data *dev_data = dev_get_drvdata(&pdev->dev);

    IVE_MSG(IVE_MSG_DBG, "dev_data: 0x%p\n", dev_data);

    return 0;
}


//-------------------------------------------------------------------------------------------------
// Data strucure for device driver
//-------------------------------------------------------------------------------------------------
static const struct of_device_id mdrv_iveg_match[] = {
    {
        .compatible = "sstar,infinity-ive",
        /*.data = NULL,*/
    },
    {},
};


static struct platform_driver mdrv_ive_driver = {
    .probe      = mdrv_ive_drv_probe,
    .remove     = mdrv_ive_drv_remove,
    .suspend    = mdrv_ive_drv_suspend,
    .resume     = mdrv_ive_drv_resume,

    .driver = {
        .of_match_table = of_match_ptr(mdrv_iveg_match),
        .name   = "mstar_ive",
        .owner  = THIS_MODULE,
    }
};

//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------

/*******************************************************************************************************************
 * _mdrv_ive_module_init
 *   module init function
 *
 * Parameters:
 *   N/A
 *
 * Return:
 *   standard return value
 */
int mdrv_ive_module_init(void)
{
    int err;
    dev_t dev;

    IVE_MSG(IVE_MSG_DBG, "Moudle Init\n");

    // Allocate cdev id
    err = alloc_chrdev_region(&dev, MDRV_IVE_MINOR, MDRV_IVE_DEVICE_COUNT, MDRV_IVE_NAME);
    if (err) {
        IVE_MSG(IVE_MSG_ERR, "Unable allocate cdev id\n");
        return err;
    }

    g_ive_drv.major = MAJOR(dev);
    g_ive_drv.minor_star = MINOR(dev);
    g_ive_drv.reg_count = 0;

    // Register device class
    g_ive_drv.class = class_create(THIS_MODULE, MDRV_IVE_CLASS_NAME);
    if (IS_ERR(g_ive_drv.class)) {
        IVE_MSG(IVE_MSG_ERR, "Failed at class_create().Please exec [mknod] before operate the device/n");
        err = PTR_ERR(g_ive_drv.class);
        goto ERR_RETURN_1;
    }

    // Register platform driver
    err = platform_driver_register(&mdrv_ive_driver);
    if (err != 0) {
        goto ERR_RETURN_2;
    }

    return 0;

ERR_RETURN_2:
    class_destroy(g_ive_drv.class);

ERR_RETURN_1:
    unregister_chrdev_region(MKDEV(g_ive_drv.major, g_ive_drv.minor_star), MDRV_IVE_DEVICE_COUNT);

    return err;
}

/*******************************************************************************************************************
 * mdrv_ive_module_exit
 *   module exit function
 *
 * Parameters:
 *   N/A
 *
 * Return:
 *   standard return value
 */
void mdrv_ive_module_exit(void)
{
    /*de-initial the who GFLIPDriver */
    IVE_MSG(IVE_MSG_DBG, "Modules Exit\n");

    platform_driver_unregister(&mdrv_ive_driver);
    class_destroy(g_ive_drv.class);
    unregister_chrdev_region(MKDEV(g_ive_drv.major, g_ive_drv.minor_star), MDRV_IVE_DEVICE_COUNT);
}

module_init(mdrv_ive_module_init);
module_exit(mdrv_ive_module_exit);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("IVE ioctrl driver");
MODULE_LICENSE("GPL");
