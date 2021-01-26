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

#define _DRV_HDMITX_MODULE_C_

#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "ms_msys.h"
#include "cam_os_wrapper.h"
#include "cam_sysfs.h"
#include "hdmitx_sysfs.h"
#include "mhal_common.h"
#include "drv_hdmitx_module.h"

//-------------------------------------------------------------------------------------------------
// Define & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Prototype
//-------------------------------------------------------------------------------------------------
static int DrvHdmitxModuleProbe(struct platform_device *pdev);
static int DrvHdmitxModuleRemove(struct platform_device *pdev);
static int DrvHdmitxModuleSuspend(struct platform_device *dev, pm_message_t state);
static int DrvHdmitxModuleResume(struct platform_device *dev);
extern void _DrvHdmitxModuleInit(void);

//-------------------------------------------------------------------------------------------------
// Structure
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------
extern DrvHdmitxModuleDevice_t _tHdmitxDevice;
extern struct class * _tHdmitxClass;
extern char * HdmitxClassName;
extern struct platform_device stDrvHdmitxPlatformDevice;


struct of_device_id _HdmitxMatchTable[] =
{
    { .compatible = "sstar,hdmitx" },
    {}
};


struct platform_driver stDrvHdmitxPlatformDriver =
{
    .probe      = DrvHdmitxModuleProbe,
    .remove     = DrvHdmitxModuleRemove,
    .suspend    = DrvHdmitxModuleSuspend,
    .resume     = DrvHdmitxModuleResume,
    .driver =
    {
        .name   = DRV_HDMITX_DEVICE_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(_HdmitxMatchTable),
    },
};

//-------------------------------------------------------------------------------------------------
// internal function
//-------------------------------------------------------------------------------------------------


//==============================================================================
static int DrvHdmitxModuleSuspend(struct platform_device *dev, pm_message_t state)
{
    int ret = 0;
    return ret;
}

static int DrvHdmitxModuleResume(struct platform_device *dev)
{
    int ret = 0;
    return ret;
}


void _DrvHdmitxRemove(void)
{
    if(_tHdmitxDevice.cdev.count)
    {
        cdev_del(&_tHdmitxDevice.cdev);
    }
    stDrvHdmitxPlatformDevice.dev.of_node=NULL;
    _tHdmitxClass = NULL;
}

void _DrvHdmitxGetIrqNum(struct platform_device *pDev, u8 u8Idx, u8 u8DevId)
{
    unsigned int HdmitxIrqId = 0; //INT_IRQ_AU_SYSTEM;
    HdmitxIrqId  = CamOfIrqToResource(pDev->dev.of_node, u8Idx, NULL);

    if (!HdmitxIrqId)
    {
        printk("[HDMITXMODULE] Can't Get SCL_IRQ\n");
    }
    else
    {
        //DrvHdmitxIrqSetIsrNum(u8DevId, HdmitxIrqId);
    }
}


//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------
static int DrvHdmitxModuleProbe(struct platform_device *pdev)
{
    //unsigned char ret;
    int s32Ret;
    dev_t  dev;
    //struct resource *res_irq;
    //struct device_node *np;

    if(_tHdmitxDevice.s32Major == 0)
    {
        s32Ret                  = alloc_chrdev_region(&dev, _tHdmitxDevice.s32Minor, DRV_HDMITX_DEVICE_COUNT, DRV_HDMITX_DEVICE_NAME);
        _tHdmitxDevice.s32Major  = MAJOR(dev);
    }

    _tHdmitxClass = msys_get_sysfs_class();
    if(!_tHdmitxClass)
    {
        _tHdmitxClass = CamClassCreate(THIS_MODULE, HdmitxClassName);
    }

    if(IS_ERR(_tHdmitxClass))
    {
        printk("Failed at CamClassCreate().Please exec [mknod] before operate the device/n");
    }
    else
    {
        //_tHdmitxDevice.devicenode->dma_mask=&u64SclHvsp_DmaMask;
        //_tHdmitxDevice.devicenode->coherent_dma_mask=u64SclHvsp_DmaMask;
    }
    //probe
    stDrvHdmitxPlatformDevice.dev.of_node = pdev->dev.of_node;

    //create device
    _DrvHdmitxModuleInit();

#if defined(SCLOS_TYPE_LINUX_TEST)
    //UTest_init(); ToDo: [Ryan] UTest_Init
#endif
    return 0;
}
static int DrvHdmitxModuleRemove(struct platform_device *pdev)
{
    _DrvHdmitxRemove();
    //ToDo
    CamDeviceUnregister(_tHdmitxDevice.devicenode);
    return 0;
}

int DrvHdmitxModuleInit(void)
{
    int ret = 0;

    ret = CamPlatformDriverRegister(&stDrvHdmitxPlatformDriver);
    if (!ret)
    {
        //printk("[HDMITX] CamPlatformDriverRegister success\n");
    }
    else
    {
        printk( "[HDMITX] CamPlatformDriverRegister failed\n");
        CamPlatformDriverUnregister(&stDrvHdmitxPlatformDriver);
    }

    return ret;
}
void DrvHdmitxModuleExit(void)
{
    /*de-initial the who GFLIPDriver */
    CamPlatformDriverUnregister(&stDrvHdmitxPlatformDriver);
}

module_init(DrvHdmitxModuleInit);
module_exit(DrvHdmitxModuleExit);

MODULE_AUTHOR("CAMDRIVER");
MODULE_DESCRIPTION("camdriver sclhvsp ioctrl driver");
MODULE_LICENSE("PROPRIETARY");
