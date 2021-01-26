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

#define _DRV_PNL_MODULE_C_

#include "cam_sysfs.h"
#include "drv_pnl_os.h"
#include "ms_msys.h"
#include "pnl_sysfs.h"
#include "pnl_debug.h"
#include "drv_pnl_module.h"
//-------------------------------------------------------------------------------------------------
// Define & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Prototype
//-------------------------------------------------------------------------------------------------
static int DrvPnlModuleProbe(struct platform_device *pdev);
static int DrvPnlModuleRemove(struct platform_device *pdev);
static int DrvPnlModuleSuspend(struct platform_device *dev, pm_message_t state);
static int DrvPnlModuleResume(struct platform_device *dev);

extern void _DrvPnlModuleInit(void);

//-------------------------------------------------------------------------------------------------
// Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------
extern DrvPnlModuleDevice_t _tPnlDevice;
extern struct class * _tPnlClass;
extern char * PnlClassName;
extern struct platform_device stDrvPnlPlatformDevice;


struct of_device_id _PnlMatchTable[] =
{
    { .compatible = "sstar,pnl" },
    {}
};

struct platform_driver stDrvPnlPlatformDriver =
{
    .probe      = DrvPnlModuleProbe,
    .remove     = DrvPnlModuleRemove,
    .suspend    = DrvPnlModuleSuspend,
    .resume     = DrvPnlModuleResume,
    .driver =
    {
        .name   = DRV_PNL_DEVICE_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(_PnlMatchTable),
    },
};

//-------------------------------------------------------------------------------------------------
// internal function
//-------------------------------------------------------------------------------------------------


//==============================================================================
static int DrvPnlModuleSuspend(struct platform_device *dev, pm_message_t state)
{
    int ret = 0;
    return ret;
}

static int DrvPnlModuleResume(struct platform_device *dev)
{
    int ret = 0;
    PNL_DBG(PNL_DBG_LEVEL_MODULE, "[PNL] %s\n",__FUNCTION__);
    return ret;
}


void _DrvPnlRemove(void)
{
    PNL_DBG(PNL_DBG_LEVEL_MODULE, "[PNL] %s\n",__FUNCTION__);
    if(_tPnlDevice.cdev.count)
    {
        cdev_del(&_tPnlDevice.cdev);
    }
    stDrvPnlPlatformDevice.dev.of_node=NULL;
    _tPnlClass = NULL;
}

//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------
static int DrvPnlModuleProbe(struct platform_device *pdev)
{
    //unsigned char ret;
    int s32Ret;
    dev_t  dev;
    //struct resource *res_irq;
    //struct device_node *np;
    PNL_DBG(PNL_DBG_LEVEL_MODULE, "[PNL] %s:%d\n",__FUNCTION__,__LINE__);

    if(_tPnlDevice.s32Major == 0)
    {
        s32Ret                  = alloc_chrdev_region(&dev, _tPnlDevice.s32Minor, DRV_PNL_DEVICE_COUNT, DRV_PNL_DEVICE_NAME);
        _tPnlDevice.s32Major  = MAJOR(dev);
    }

    _tPnlClass = msys_get_sysfs_class();
    if(!_tPnlClass)
    {
        _tPnlClass = CamClassCreate(THIS_MODULE, PnlClassName);
    }
    if(IS_ERR(_tPnlClass))
    {
        PNL_ERR("Failed at CamClassCreate().Please exec [mknod] before operate the device/n");
    }
    else
    {
        //_tPnlDevice.devicenode->dma_mask=&u64SclHvsp_DmaMask;
        //_tPnlDevice.devicenode->coherent_dma_mask=u64SclHvsp_DmaMask;
    }
    //probe
    PNL_DBG(PNL_DBG_LEVEL_MODULE, "[PNL] %s\n",__FUNCTION__);
    stDrvPnlPlatformDevice.dev.of_node = pdev->dev.of_node;

    //create device
    _DrvPnlModuleInit();

#if defined(SCLOS_TYPE_LINUX_TEST)
    //UTest_init(); ToDo: [Ryan] UTest_Init
#endif
    return 0;
}
static int DrvPnlModuleRemove(struct platform_device *pdev)
{
    PNL_DBG(PNL_DBG_LEVEL_MODULE, "[PNL] %s\n",__FUNCTION__);
    _DrvPnlRemove();
    //ToDo
    CamDeviceUnregister(_tPnlDevice.devicenode);
    return 0;
}

int DrvPnlModuleInit(void)
{
    int ret = 0;

    PNL_DBG(PNL_DBG_LEVEL_MODULE, "[PNL] %s:%d\n",__FUNCTION__,__LINE__);


    ret = CamPlatformDriverRegister(&stDrvPnlPlatformDriver);
    if (!ret)
    {
        PNL_DBG(PNL_DBG_LEVEL_MODULE, "[PNL] CamPlatformDriverRegister success\n");
    }
    else
    {
        PNL_ERR( "[SCLHVSP_1] CamPlatformDriverRegister failed\n");
        CamPlatformDriverUnregister(&stDrvPnlPlatformDriver);
    }

    return ret;
}
void DrvPnlModuleExit(void)
{
    /*de-initial the who GFLIPDriver */
    PNL_DBG(PNL_DBG_LEVEL_MODULE, "[PNL] %s\n",__FUNCTION__);
    CamPlatformDriverUnregister(&stDrvPnlPlatformDriver);
}

module_init(DrvPnlModuleInit);
module_exit(DrvPnlModuleExit);

MODULE_AUTHOR("CAMDRIVER");
MODULE_DESCRIPTION("camdriver panel ioctrl driver");
MODULE_LICENSE("PROPRIETARY");
