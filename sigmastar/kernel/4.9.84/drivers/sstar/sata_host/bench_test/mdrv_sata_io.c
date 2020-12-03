/*
* mdrv_sata_io.c- Sigmastar
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
#include <linux/pfn.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>          /* seems do not need this */
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/time.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/string.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>

//#include "irqs.h"
#include "ms_platform.h"
#include "ms_msys.h"

#include "mdrv_sata.h"
#include "mdrv_sata_io_st.h"
#include "mdrv_sata_io.h"
//#include "MsTypes.h"
#include "mhal_sata_bench_test.h"

int debug_level;
module_param(debug_level, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug_level, "\nLevels:\n"
                 " [1] Debug level 1\n"
                 " [2] Debug level 2\n"
                 " [3] Debug level 3\n"
                 " [4] Debug level 4");

#define MDRV_SATA_DEVICE_COUNT   1
#define MDRV_SATA_NAME           "msata"
#define MAX_FILE_HANDLE_SUPPRT  64
#define MDRV_NAME_SATA           "msata"
#define MDRV_MAJOR_SATA          0xea
#define MDRV_MINOR_SATA          0x09

#define CMD_PARSING(x)  (x==IOCTL_SATA_SET_LOOPBACK_TEST ?         "IOCTL_SATA_SET_LOOPBACK_TEST" : \
                         x==IOCTL_SATA_SET_CONFIG ?            "IOCTL_SATA_SET_CONFIG" : \
                         x==IOCTL_SATA_GET_INTERRUPT_STATUS ?  "IOCTL_SATA_GET_INTERRUPT_STATUS" : \
                         "unknown")

int sstar_sata_drv_open(struct inode *inode, struct file *filp);
int sstar_sata_drv_release(struct inode *inode, struct file *filp);
long sstar_sata_drv_ioctl(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg);
static int sstar_sata_drv_probe(struct platform_device *pdev);
static int sstar_sata_drv_remove(struct platform_device *pdev);
static int sstar_sata_drv_suspend(struct platform_device *dev, pm_message_t state);
static int sstar_sata_drv_resume(struct platform_device *dev);
static void sstar_sata_drv_platfrom_release(struct device *device);
static unsigned int sstar_sata_drv_poll(struct file *filp, struct poll_table_struct *wait);


//-------------------------------------------------------------------------------------------------

typedef struct
{
    int s32Major;
    int s32Minor;
    int refCnt;
    struct cdev cdev;
    struct file_operations fops;
} SATA_DEV;

static SATA_DEV _devSATA =
{
    .s32Major = MDRV_MAJOR_SATA,
    .s32Minor = MDRV_MINOR_SATA,
    .refCnt = 0,
    .cdev =
    {
        .kobj = {.name = MDRV_NAME_SATA, },
        .owner = THIS_MODULE,
    },
    .fops =
    {
        .open = sstar_sata_drv_open,
        .release = sstar_sata_drv_release,
        .unlocked_ioctl = sstar_sata_drv_ioctl,
        .poll = sstar_sata_drv_poll,
    }
};

static struct class * sata_class;
static char * sata_classname = "sstar_sata_class";

static struct platform_driver Sstar_sata_driver =
{
    .probe 		= sstar_sata_drv_probe,
    .remove 	= sstar_sata_drv_remove,
    .suspend    = sstar_sata_drv_suspend,
    .resume     = sstar_sata_drv_resume,

    .driver = {
        .name	= "msata",
        .owner  = THIS_MODULE,
    }
};

static u64 sg_sstar_device_sata_dmamask = 0xffffffffUL;

static struct platform_device sg_mdrv_sata_device =
{
    .name = "msata",
    .id = 0,
    .dev =
    {
        .release = sstar_sata_drv_platfrom_release,
        .dma_mask = &sg_sstar_device_sata_dmamask,
        .coherent_dma_mask = 0xffffffffUL
    }
};
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// IOCtrl Driver interface functions
//-------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------

int _MDrv_SATAIO_IOC_Set_LoopbackTest(struct file *filp, unsigned long arg)
{
    int ret  = 0;
    stSata_Loopback_Test stCfg;
    int result = 0;
    if(copy_from_user(&stCfg, (stSata_Loopback_Test __user *)arg, sizeof(stSata_Loopback_Test)))
    {
        printk("SATA FAIL %d ...\n", __LINE__);
        return -EFAULT;
    }

    //printk("[%s][%d] Port = %d, Gen = %d\n", __func__, __LINE__, stCfg.u16PortNo, stCfg.u16GenNo);
    result = (int)MHal_SATA_LoopBack_Test((u8)stCfg.u16PortNo, (u8)stCfg.u16GenNo);
    stCfg.s32Result = result;

    if (copy_to_user((stSata_Loopback_Test __user *)arg, &stCfg, sizeof(stSata_Loopback_Test)))
    {
        printk("SATA FAIL %d ...\n", __LINE__);
        return -EFAULT;
    }

    return ret;
}

int _MDrv_SATAIO_IOC_Set_SATA_Config(struct file *filp, unsigned long arg)
{
    int ret  = 0;

    return ret;
}

int _MDrv_SATAIO_IO_Get_Interrupt_Status(struct file *filp, unsigned long arg)
{
    //SATA_INTR_STATUS stIntrStatus;

    return 0;
}

int _MDrv_SATAIO_IOC_Set_TxTest_HFTP(struct file *filp, unsigned long arg)
{
    int ret  = 0;
    stSata_Tx_Test stCfg;
    int result = 0;
    if(copy_from_user(&stCfg, (stSata_Tx_Test __user *)arg, sizeof(stSata_Tx_Test)))
    {
        printk("SATA SET TX TEST HFTP FAIL %d ...\n", __LINE__);
        return -EFAULT;
    }

    //printk("[%s][%d] Port = %d, Gen = %d\n", __func__, __LINE__, stCfg.u16PortNo, stCfg.u16GenNo);
    result = (int)MHal_SATA_Tx_Test_Phy_Initialize((u8)stCfg.u16PortNo, (u8)stCfg.u16GenNo);
    stCfg.s32Result = result;
    MHal_SATA_Tx_Test_Pattern_HFTP((u8)stCfg.u16PortNo);
    MHal_SATA_Tx_Test_SSC((u8)stCfg.u16PortNo, (u8)stCfg.u32SSCEnable);

    if (copy_to_user((stSata_Tx_Test __user *)arg, &stCfg, sizeof(stSata_Tx_Test)))
    {
        printk("SATA SET TX TEST HFTP FAIL %d ...\n", __LINE__);
        return -EFAULT;
    }

    return ret;
}

int _MDrv_SATAIO_IOC_Set_TxTest_MFTP(struct file *filp, unsigned long arg)
{
    int ret  = 0;
    stSata_Tx_Test stCfg;
    int result = 0;
    if(copy_from_user(&stCfg, (stSata_Tx_Test __user *)arg, sizeof(stSata_Tx_Test)))
    {
        printk("SATA SET TX TEST MFTP FAIL %d ...\n", __LINE__);
        return -EFAULT;
    }

    result = (int)MHal_SATA_Tx_Test_Phy_Initialize((u8)stCfg.u16PortNo, (u8)stCfg.u16GenNo);
    stCfg.s32Result = result;
    MHal_SATA_Tx_Test_Pattern_MFTP((u8)stCfg.u16PortNo);
    MHal_SATA_Tx_Test_SSC((u8)stCfg.u16PortNo, (u8)stCfg.u32SSCEnable);

    if (copy_to_user((stSata_Tx_Test __user *)arg, &stCfg, sizeof(stSata_Tx_Test)))
    {
        printk("SATA SET TX TEST MFTP FAIL %d ...\n", __LINE__);
        return -EFAULT;
    }

    return ret;
}

int _MDrv_SATAIO_IOC_Set_TxTest_LFTP(struct file *filp, unsigned long arg)
{
    int ret  = 0;
    stSata_Tx_Test stCfg;
    int result = 0;
    if(copy_from_user(&stCfg, (stSata_Tx_Test __user *)arg, sizeof(stSata_Tx_Test)))
    {
        printk("SATA SET TX TEST LFTP FAIL %d ...\n", __LINE__);
        return -EFAULT;
    }

    result = (int)MHal_SATA_Tx_Test_Phy_Initialize((u8)stCfg.u16PortNo, (u8)stCfg.u16GenNo);
    stCfg.s32Result = result;
    MHal_SATA_Tx_Test_Pattern_LFTP((u8)stCfg.u16PortNo);
    MHal_SATA_Tx_Test_SSC((u8)stCfg.u16PortNo, (u8)stCfg.u32SSCEnable);

    if (copy_to_user((stSata_Tx_Test __user *)arg, &stCfg, sizeof(stSata_Tx_Test)))
    {
        printk("SATA SET TX TEST LFTP FAIL %d ...\n", __LINE__);
        return -EFAULT;
    }

    return ret;
}

int _MDrv_SATAIO_IOC_Set_TxTest_LBP(struct file *filp, unsigned long arg)
{
    int ret  = 0;
    stSata_Tx_Test stCfg;
    int result = 0;
    if(copy_from_user(&stCfg, (stSata_Tx_Test __user *)arg, sizeof(stSata_Tx_Test)))
    {
        printk("SATA SET TX TEST LBP FAIL %d ...\n", __LINE__);
        return -EFAULT;
    }

    result = (int)MHal_SATA_Tx_Test_Phy_Initialize((u8)stCfg.u16PortNo, (u8)stCfg.u16GenNo);
    stCfg.s32Result = result;
    MHal_SATA_Tx_Test_Pattern_LBP((u8)stCfg.u16PortNo);
    MHal_SATA_Tx_Test_SSC((u8)stCfg.u16PortNo, (u8)stCfg.u32SSCEnable);

    if (copy_to_user((stSata_Tx_Test __user *)arg, &stCfg, sizeof(stSata_Tx_Test)))
    {
        printk("SATA SET TX TEST LBP FAIL %d ...\n", __LINE__);
        return -EFAULT;
    }

    return ret;
}

int _MDrv_SATAIO_IOC_Set_TxTest_SSOP(struct file *filp, unsigned long arg)
{
    int ret  = 0;
    stSata_Tx_Test stCfg;
    int result = 0;
    if(copy_from_user(&stCfg, (stSata_Tx_Test __user *)arg, sizeof(stSata_Tx_Test)))
    {
        printk("SATA SET TX TEST SSOP FAIL %d ...\n", __LINE__);
        return -EFAULT;
    }

    result = (int)MHal_SATA_Tx_Test_Phy_Initialize((u8)stCfg.u16PortNo, (u8)stCfg.u16GenNo);
    stCfg.s32Result = result;
    MHal_SATA_Tx_Test_Pattern_SSOP((u8)stCfg.u16PortNo);
    MHal_SATA_Tx_Test_SSC((u8)stCfg.u16PortNo, (u8)stCfg.u32SSCEnable);

    if (copy_to_user((stSata_Tx_Test __user *)arg, &stCfg, sizeof(stSata_Tx_Test)))
    {
        printk("SATA SET TX TEST SSOP FAIL %d ...\n", __LINE__);
        return -EFAULT;
    }

    return ret;
}

//==============================================================================
long sstar_sata_drv_ioctl(struct file *filp, unsigned int u32Cmd, unsigned long u32Arg)
{
    //    int err = 0;
    int retval = 0;

    if(_devSATA.refCnt <= 0)
    {
        printk("[SATA] SATAIO_IOCTL refCnt =%d!!! \n", _devSATA.refCnt);
        return -EFAULT;
    }
    /* check u32Cmd valid */
    if(IOCTL_SATA_MAGIC == _IOC_TYPE(u32Cmd))
    {
        if(_IOC_NR(u32Cmd) >= IOCTL_SATA_MAX_NR)
        {
            printk("[SATA] IOCtl NR Error!!! (Cmd=%x)\n", u32Cmd);
            return -ENOTTY;
        }
    }
    else
    {
        printk("[SATA] IOCtl MAGIC Error!!! (Cmd=%x)\n", u32Cmd);
        return -ENOTTY;
    }
#if 0
    /* verify Access */
    if (_IOC_DIR(u32Cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void __user *)u32Arg, _IOC_SIZE(u32Cmd));
    }
    else if (_IOC_DIR(u32Cmd) & _IOC_WRITE)
    {
        err =  !access_ok(VERIFY_READ, (void __user *)u32Arg, _IOC_SIZE(u32Cmd));
    }
    if (err)
    {
        return -EFAULT;
    }
#endif

    /* not allow query or command once driver suspend */

    //printk("[SATA] IOCTL: ==%s==\n", (CMD_PARSING(u32Cmd)));
    //printk("[SATA] === SATAIO_IOCTL %d === \n", (u32Cmd));

    switch(u32Cmd)
    {
        case IOCTL_SATA_SET_LOOPBACK_TEST:
            retval = _MDrv_SATAIO_IOC_Set_LoopbackTest(filp, u32Arg);
            break;

        case IOCTL_SATA_SET_CONFIG:
            retval = _MDrv_SATAIO_IOC_Set_SATA_Config(filp, u32Arg);
            break;

        case IOCTL_SATA_GET_INTERRUPT_STATUS:
            retval = _MDrv_SATAIO_IO_Get_Interrupt_Status(filp, u32Arg);
            break;

        case IOCTL_SATA_SET_TX_TEST_HFTP:
            retval = _MDrv_SATAIO_IOC_Set_TxTest_HFTP(filp, u32Arg);
            break;

        case IOCTL_SATA_SET_TX_TEST_MFTP:
            retval = _MDrv_SATAIO_IOC_Set_TxTest_MFTP(filp, u32Arg);
            break;

        case IOCTL_SATA_SET_TX_TEST_LFTP:
            retval = _MDrv_SATAIO_IOC_Set_TxTest_LFTP(filp, u32Arg);
            break;

        case IOCTL_SATA_SET_TX_TEST_LBP:
            retval = _MDrv_SATAIO_IOC_Set_TxTest_LBP(filp, u32Arg);
            break;

        case IOCTL_SATA_SET_TX_TEST_SSOP:
            retval = _MDrv_SATAIO_IOC_Set_TxTest_SSOP(filp, u32Arg);
            break;

        default:  /* redundant, as cmd was checked against MAXNR */
            printk("[SATA] ERROR IOCtl number %x\n ", u32Cmd);
            retval = -ENOTTY;
            break;
    }

    return retval;
}


static unsigned int sstar_sata_drv_poll(struct file *filp, struct poll_table_struct *wait)
{
#if 0
    if (atomic_read(&SATAW_intr_count) > 0)
    {
        atomic_set(&SATAW_intr_count, 0);
        return POLLIN | POLLRDNORM; /* readable */
    }
#endif
    return 0;
}

static int sstar_sata_drv_probe(struct platform_device *pdev)
{
    printk("[SATA] sstar_sata_drv_probe\n");

    return 0;
}

static int sstar_sata_drv_remove(struct platform_device *pdev)
{
    printk("[SATA] sstar_sata_drv_remove\n");

    return 0;
}

static int sstar_sata_drv_suspend(struct platform_device *dev, pm_message_t state)
{
    printk("[SATA] sstar_sata_drv_suspend\n");

    return 0;
}

static int sstar_sata_drv_resume(struct platform_device *dev)
{
    printk("[SATA] sstar_sata_drv_resume\n");

    return 0;
}

static void sstar_sata_drv_platfrom_release(struct device *device)
{
    printk("[SATA] sstar_sata_drv_platfrom_release\n");
}

int sstar_sata_drv_open(struct inode *inode, struct file *filp)
{
    //printk("[SATA] SATA DRIVER OPEN\n");

    _devSATA.refCnt++;

    //printk("[SATA] Open refCnt = %d\n", _devSATA.refCnt);

    return 0;
}

int sstar_sata_drv_release(struct inode *inode, struct file *filp)
{
    //printk("[SATA] SATA DRIVER RELEASE\n");
    _devSATA.refCnt--;

    //    free_irq(INT_IRQ_SATAW, MDrv_SATAW_isr);

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------
int _MDrv_SATAIO_ModuleInit(void)
{
    int ret = 0;
    dev_t  dev;
    int s32Ret;
    printk("[SATAIO]_Init \n");

    if(_devSATA.s32Major)
    {
        dev = MKDEV(_devSATA.s32Major, _devSATA.s32Minor);
        s32Ret = register_chrdev_region(dev, MDRV_SATA_DEVICE_COUNT, MDRV_SATA_NAME);
    }
    else
    {
        s32Ret = alloc_chrdev_region(&dev, _devSATA.s32Minor, MDRV_SATA_DEVICE_COUNT, MDRV_SATA_NAME);
        _devSATA.s32Major = MAJOR(dev);
    }

    if (0 > s32Ret)
    {
        printk("[SATA] Unable to get major %d\n", _devSATA.s32Major);
        return s32Ret;
    }

    cdev_init(&_devSATA.cdev, &_devSATA.fops);
    if (0 != (s32Ret = cdev_add(&_devSATA.cdev, dev, MDRV_SATA_DEVICE_COUNT)))
    {
        printk("[SATA] Unable add a character device\n");
        unregister_chrdev_region(dev, MDRV_SATA_DEVICE_COUNT);
        return s32Ret;
    }

    sata_class = class_create(THIS_MODULE, sata_classname);
    if(IS_ERR(sata_class))
    {
        printk(KERN_WARNING"Failed at class_create().Please exec [mknod] before operate the device/n");
    }
    else
    {
        device_create(sata_class, NULL, dev, NULL, "msata");
    }
    /* initial the whole SATA Driver */
    ret = platform_driver_register(&Sstar_sata_driver);

    if (!ret)
    {
        ret = platform_device_register(&sg_mdrv_sata_device);
        if (ret)    /* if register device fail, then unregister the driver.*/
        {
            platform_driver_unregister(&Sstar_sata_driver);
            printk("[SATA] register failed\n");

        }
        else
        {
            printk("[SATA] register success\n");
        }
    }

    return ret;
}


void _MDrv_SATAIO_ModuleExit(void)
{
    /*de-initial the who GFLIPDriver */
    printk("[SATAIO]_Exit \n");
    cdev_del(&_devSATA.cdev);
    device_destroy(sata_class, MKDEV(_devSATA.s32Major, _devSATA.s32Minor));
    class_destroy(sata_class);
    unregister_chrdev_region(MKDEV(_devSATA.s32Major, _devSATA.s32Minor), MDRV_SATA_DEVICE_COUNT);
    platform_device_unregister(&sg_mdrv_sata_device);
    platform_driver_unregister(&Sstar_sata_driver);
}

module_init(_MDrv_SATAIO_ModuleInit);
module_exit(_MDrv_SATAIO_ModuleExit);

MODULE_AUTHOR("SIGMASTAR");
MODULE_DESCRIPTION("SATA Bench test driver");
MODULE_LICENSE("GPL");
