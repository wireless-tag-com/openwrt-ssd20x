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

#define _DISP_MODULE_C_

#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include "ms_msys.h"
#include "cam_sysfs.h"
#include "drv_disp_os.h"
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

//-------------------------------------------------------------------------------------------------
// Structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Variable
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
// internal function
//-------------------------------------------------------------------------------------------------


//==============================================================================
void _DrvDispGetIrqNum(struct platform_device *pDev, u8 u8Idx, u8 u8DevId)
{
    unsigned int DispIrqId = 0; //INT_IRQ_AU_SYSTEM;

    DispIrqId  = CamOfIrqToResource(pDev->dev.of_node, u8Idx, NULL);

    if (!DispIrqId)
    {
        DISP_ERR("[DISPMODULE] Can't Get SCL_IRQ, Idx=%d, DevId=%d\n", u8Idx,u8DevId);
    }
    else
    {
        DrvDispIrqSetIsrNum(u8DevId, DispIrqId);
    }
}


void _DrvDispModuleInit(void)
{
    int s32Ret;
    dev_t  dev;

    if(_tDispDevice.s32Major)
    {
        dev     = MKDEV(_tDispDevice.s32Major, _tDispDevice.s32Minor);
        if(!_tDispClass)
        {
            _tDispClass = msys_get_sysfs_class();
            if(!_tDispClass)
            {
                _tDispClass = CamClassCreate(THIS_MODULE, DispClassName);
            }

        }
        else
        {
            cdev_init(&_tDispDevice.cdev, &_tDispDevice.fops);
            if (0 != (s32Ret= cdev_add(&_tDispDevice.cdev, dev, DRV_DISP_DEVICE_COUNT)))
            {
                DISP_ERR( "[DISP] Unable add a character device\n");
            }
        }
        //ToDo
        if(_tDispDevice.devicenode==NULL)
        {
            _tDispDevice.devicenode = CamDeviceCreate(_tDispClass, NULL, dev,NULL, DRV_DISP_DEVICE_NAME);
            DrvDispSysfsInit(_tDispDevice.devicenode);
        }

        if(stDrvDispPlatformDevice.dev.of_node==NULL)
        {
            stDrvDispPlatformDevice.dev.of_node = of_find_compatible_node(NULL, NULL, "sstar,disp");
        }
        if(stDrvDispPlatformDevice.dev.of_node==NULL)
        {
            DISP_ERR("[DISP INIT] Get Device mode Fail!!\n");
        }

        //Get IRQ
    }
    _DrvDispGetIrqNum(&stDrvDispPlatformDevice, 0, 0);
    _DrvDispGetIrqNum(&stDrvDispPlatformDevice, 1, 1);
    _DrvDispGetIrqNum(&stDrvDispPlatformDevice, 2, 2);
    DrvDispOsSetDeviceNode(&stDrvDispPlatformDevice);
}



//-------------------------------------------------------------------------------------------------
// Module functions
//-------------------------------------------------------------------------------------------------

