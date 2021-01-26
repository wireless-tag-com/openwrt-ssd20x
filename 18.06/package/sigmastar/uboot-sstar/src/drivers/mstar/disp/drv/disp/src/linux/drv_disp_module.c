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

#define _DRV_DISP_MODULE_C_

#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "ms_msys.h"
#include "drv_disp_os.h"
#include "cam_sysfs.h"
#include "disp_sysfs.h"
#include "disp_debug.h"

#include "mhal_common.h"
#include "mhal_disp_datatype.h"
#include "hal_disp_chip.h"
#include "hal_disp_st.h"
#include "drv_disp_ctx.h"
#include "drv_disp_irq.h"
#include "drv_disp_module.h"
//-------------------------------------------------------------------------------------------------
// Define & Macro
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Prototype
//-------------------------------------------------------------------------------------------------
static int DrvDispModuleProbe(struct platform_device *pdev);
static int DrvDispModuleRemove(struct platform_device *pdev);
static int DrvDispModuleSuspend(struct platform_device *dev, pm_message_t state);
static int DrvDispModuleResume(struct platform_device *dev);

extern void _DrvDispModuleInit(void);
//-------------------------------------------------------------------------------------------------
// Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------
extern DrvDispModuleDevice_t _tDispDevice;
extern struct class * _tDispClass;
extern char * DispClassName;
extern struct platform_device stDrvDispPlatformDevice;

struct of_device_id _DispMatchTable[] =
{
    { .compatible = "sstar,disp" },
    {}
};

struct platform_driver stDrvDispPlatformDriver =
{
    .probe      = DrvDispModuleProbe,
    .remove     = DrvDispModuleRemove,
    .suspend    = DrvDispModuleSuspend,
    .resume     = DrvDispModuleResume,
    .driver =
    {
        .name   = DRV_DISP_DEVICE_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(_DispMatchTable),
    },
};


//-------------------------------------------------------------------------------------------------
// internal function
//-------------------------------------------------------------------------------------------------


//==============================================================================
static int DrvDispModuleSuspend(struct platform_device *dev, pm_message_t state)
{
    int ret = 0;
    return ret;
}

static int DrvDispModuleResume(struct platform_device *dev)
{
    int ret = 0;
    DISP_DBG(DISP_DBG_LEVEL_IO, "[DISP] %s\n",__FUNCTION__);
    return ret;
}


void _DrvDispRemove(void)
{
    DISP_DBG(DISP_DBG_LEVEL_IO, "[DISP] %s\n",__FUNCTION__);
    if(_tDispDevice.cdev.count)
    {
        cdev_del(&_tDispDevice.cdev);
    }
    stDrvDispPlatformDevice.dev.of_node=NULL;
    _tDispClass = NULL;
}

//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------
static int DrvDispModuleProbe(struct platform_device *pdev)
{
    //unsigned char ret;
    int s32Ret;
    dev_t  dev;
    //struct resource *res_irq;
    //struct device_node *np;
    DISP_DBG(DISP_DBG_LEVEL_IO, "[DISP] %s:%d\n",__FUNCTION__,__LINE__);

    if(_tDispDevice.s32Major == 0)
    {
        s32Ret                  = alloc_chrdev_region(&dev, _tDispDevice.s32Minor, DRV_DISP_DEVICE_COUNT, DRV_DISP_DEVICE_NAME);
        _tDispDevice.s32Major  = MAJOR(dev);
    }

    _tDispClass = msys_get_sysfs_class();
    if(!_tDispClass)
    {
        _tDispClass = CamClassCreate(THIS_MODULE, DispClassName);
    }
    if(IS_ERR(_tDispClass))
    {
        DISP_ERR("Failed at CamClassCreate().Please exec [mknod] before operate the device/n");
    }
    else
    {
        //_tDispDevice.devicenode->dma_mask=&u64SclHvsp_DmaMask;
        //_tDispDevice.devicenode->coherent_dma_mask=u64SclHvsp_DmaMask;
    }
    //probe
    DISP_DBG(DISP_DBG_LEVEL_IO, "[DISP] %s\n",__FUNCTION__);
    stDrvDispPlatformDevice.dev.of_node = pdev->dev.of_node;

    //create device
    _DrvDispModuleInit();

#if defined(SCLOS_TYPE_LINUX_TEST)
    //UTest_init(); ToDo: [Ryan] UTest_Init
#endif
    return 0;
}
static int DrvDispModuleRemove(struct platform_device *pdev)
{
    DISP_DBG(DISP_DBG_LEVEL_IO, "[DISP] %s\n",__FUNCTION__);
    _DrvDispRemove();
    //ToDo
    CamDeviceUnregister(_tDispDevice.devicenode);
    return 0;
}

int DrvDispModuleInit(void)
{
    int ret = 0;

    DISP_DBG(DISP_DBG_LEVEL_IO, "[DISP] %s:%d\n",__FUNCTION__,__LINE__);


    ret = CamPlatformDriverRegister(&stDrvDispPlatformDriver);
    if (!ret)
    {
        DISP_DBG(DISP_DBG_LEVEL_IO, "[DISP] CamPlatformDriverRegister success\n");
    }
    else
    {
        DISP_ERR( "[SCLHVSP_1] CamPlatformDriverRegister failed\n");
        CamPlatformDriverUnregister(&stDrvDispPlatformDriver);
    }

    return ret;
}
void DrvDispModuleExit(void)
{
    /*de-initial the who GFLIPDriver */
    DISP_DBG(DISP_DBG_LEVEL_IO, "[DISP] %s\n",__FUNCTION__);
    CamPlatformDriverUnregister(&stDrvDispPlatformDriver);
}

module_init(DrvDispModuleInit);
module_exit(DrvDispModuleExit);

MODULE_AUTHOR("CAMDRIVER");
MODULE_DESCRIPTION("camdriver disp ioctrl driver");
MODULE_LICENSE("PROPRIETARY");
