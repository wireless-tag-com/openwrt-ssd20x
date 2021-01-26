/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#define _DRV_GOP_MODULE_C_

#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "ms_msys.h"
#include "cam_os_wrapper.h"
#include "gop_sysfs.h"
#include "mhal_common.h"
#include "cam_sysfs.h"

//-------------------------------------------------------------------------------------------------
// Define & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_GOP_DEVICE_COUNT    1
#define DRV_GOP_DEVICE_NAME     "mgop"
#define DRV_GOP_DEVICE_MAJOR    0xFF
#define DRV_GOP_DEVICE_MINOR    0xFB

//-------------------------------------------------------------------------------------------------
// Prototype
//-------------------------------------------------------------------------------------------------
static int DrvGopModuleProbe(struct platform_device *pdev);
static int DrvGopModuleRemove(struct platform_device *pdev);
static int DrvGopModuleSuspend(struct platform_device *dev, pm_message_t state);
static int DrvGopModuleResume(struct platform_device *dev);

//-------------------------------------------------------------------------------------------------
// Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    int s32Major;
    int s32Minor;
    int refCnt;
    struct cdev cdev;
    struct file_operations fops;
    struct device *devicenode;
}DrvGopModuleDevice_t;

//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------
static DrvGopModuleDevice_t _tGopDevice =
{
    .s32Major = DRV_GOP_DEVICE_MAJOR,
    .s32Minor = DRV_GOP_DEVICE_MINOR,
    .refCnt = 0,
    .devicenode = NULL,
    .cdev =
    {
        .kobj = {.name= DRV_GOP_DEVICE_NAME, },
        .owner = THIS_MODULE,
    },
    /*
    .fops =
    {
        .open = DrvGopModuleOpen,
        .release = DrvGopModuleRelease,
        .unlocked_ioctl = DrvGopModuleIoctl,
        .poll = DrvGopModulePoll,
    },*/
};

static struct class * _tGopClass = NULL;
static char * GopClassName = "m_gop_class";


static const struct of_device_id _GopMatchTable[] =
{
    { .compatible = "sstar,gop" },
    {}
};

static struct platform_driver stDrvGopPlatformDriver =
{
    .probe      = DrvGopModuleProbe,
    .remove     = DrvGopModuleRemove,
    .suspend    = DrvGopModuleSuspend,
    .resume     = DrvGopModuleResume,
    .driver =
    {
        .name   = DRV_GOP_DEVICE_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(_GopMatchTable),
    },
};

static u64 u64Gop_DmaMask = 0xffffffffUL;

static struct platform_device stDrvGopPlatformDevice =
{
    .name = DRV_GOP_DEVICE_NAME,
    .id = 0,
    .dev =
    {
        .of_node = NULL,
        .dma_mask = &u64Gop_DmaMask,
        .coherent_dma_mask = 0xffffffffUL
    }
};

u32 _gu32GopDbgLevel = 0;

//-------------------------------------------------------------------------------------------------
// internal function
//-------------------------------------------------------------------------------------------------


//==============================================================================
static int DrvGopModuleSuspend(struct platform_device *dev, pm_message_t state)
{
    int ret = 0;
    return ret;
}

static int DrvGopModuleResume(struct platform_device *dev)
{
    int ret = 0;
    printk("[GOPMODULE] %s\n",__FUNCTION__);
    return ret;
}


void _DrvGopRemove(void)
{
    printk("[GOPMODULE] %s\n",__FUNCTION__);
    if(_tGopDevice.cdev.count)
    {
        cdev_del(&_tGopDevice.cdev);
    }
    stDrvGopPlatformDevice.dev.of_node=NULL;
    _tGopClass = NULL;
}

void _DrvGopModuleInit(void)
{
    int s32Ret;
    dev_t  dev;

    if(_tGopDevice.s32Major)
    {
        dev     = MKDEV(_tGopDevice.s32Major, _tGopDevice.s32Minor);
        if(!_tGopClass)
        {
        #ifndef CHIP_MV2
            _tGopClass = msys_get_sysfs_class();
        #endif
            if(!_tGopClass)
            {
                _tGopClass = CamClassCreate(THIS_MODULE, GopClassName);
            }

        }
        else
        {
            cdev_init(&_tGopDevice.cdev, &_tGopDevice.fops);
            if (0 != (s32Ret= cdev_add(&_tGopDevice.cdev, dev, DRV_GOP_DEVICE_COUNT)))
            {
                printk( "[GOPMODULE] Unable add a character device\n");
            }
        }
        //ToDo
        if(_tGopDevice.devicenode==NULL)
        {
            _tGopDevice.devicenode = CamDeviceCreate(_tGopClass, NULL, dev,NULL, DRV_GOP_DEVICE_NAME);
            DrvGopSysfsInit(_tGopDevice.devicenode);
            printk( "[GOPMODULE] register sysfs\n");
        }

        if(stDrvGopPlatformDevice.dev.of_node==NULL)
        {
            stDrvGopPlatformDevice.dev.of_node = of_find_compatible_node(NULL, NULL, "sstar,gop");
        }
        if(stDrvGopPlatformDevice.dev.of_node==NULL)
        {
            printk("[GOPMODULE] Gopt Device mode Fail!!\n");
        }

    }
}



//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------
static int DrvGopModuleProbe(struct platform_device *pdev)
{
    //unsigned char ret;
    int s32Ret;
    dev_t  dev;
    //struct resource *res_irq;
    //struct device_node *np;
    printk("[GOPMODULE] %s:%d\n",__FUNCTION__,__LINE__);

    if(_tGopDevice.s32Major == 0)
    {
        s32Ret                  = alloc_chrdev_region(&dev, _tGopDevice.s32Minor, DRV_GOP_DEVICE_COUNT, DRV_GOP_DEVICE_NAME);
        _tGopDevice.s32Major  = MAJOR(dev);
    }

#ifndef CHIP_MV2
    _tGopClass = msys_get_sysfs_class();
#endif

    if(!_tGopClass)
    {
        _tGopClass = CamClassCreate(THIS_MODULE, GopClassName);
    }
    if(IS_ERR(_tGopClass))
    {
        printk("[GOPMODULE]Failed at class_create().Please exec [mknod] before operate the device/n");
    }
    else
    {
        //_tGopDevice.devicenode->dma_mask=&u64SclHvsp_DmaMask;
        //_tGopDevice.devicenode->coherent_dma_mask=u64SclHvsp_DmaMask;
    }
    //probe
    printk("[GOPMODULE] %s\n",__FUNCTION__);
    stDrvGopPlatformDevice.dev.of_node = pdev->dev.of_node;

    //create device
    _DrvGopModuleInit();

    return 0;
}
static int DrvGopModuleRemove(struct platform_device *pdev)
{
    printk("[GOPMODULE] %s\n",__FUNCTION__);
    _DrvGopRemove();
    //ToDo
    CamDeviceUnregister(_tGopDevice.devicenode);
    return 0;
}

int DrvGopModuleInit(void)
{
    int ret = 0;

    printk("[GOPMODULE] %s:%d\n",__FUNCTION__,__LINE__);


    ret = CamPlatformDriverRegister(&stDrvGopPlatformDriver);
    if (!ret)
    {
        printk("[GOPMODULE] platform_driver_register success\n");
    }
    else
    {
        printk( "[GOPMODULE] platform_driver_register failed\n");
        CamPlatformDriverUnregister(&stDrvGopPlatformDriver);
    }

    return ret;
}
void DrvGopModuleExit(void)
{
    /*de-initial the who GFLIPDriver */
    printk("[GOPMODULE] %s\n",__FUNCTION__);
    CamPlatformDriverUnregister(&stDrvGopPlatformDriver);
}

module_init(DrvGopModuleInit);
module_exit(DrvGopModuleExit);

MODULE_AUTHOR("SIGMASTAR");
MODULE_DESCRIPTION("sstar gop ioctrl driver");
MODULE_LICENSE("PROPRIETARY");
