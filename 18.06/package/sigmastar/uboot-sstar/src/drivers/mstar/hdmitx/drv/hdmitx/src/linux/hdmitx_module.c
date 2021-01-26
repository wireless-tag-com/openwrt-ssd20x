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

#define _HDMITX_MODULE_C_

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
#include "drv_hdmitx_os.h"
#include "drv_hdmitx_module.h"

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
void _DrvHdmiTxUpdateHpdIrq(struct platform_device *pDev)
{
    u32 u32HpdInt = 0;
    if(CamofPropertyReadU32(pDev->dev.of_node, "hpd_irq", &u32HpdInt))
    {
        //printf(PRINT_RED "%s %d, No hpd_irq property \n" PRINT_NONE, __FUNCTION__, __LINE__);
        u32HpdInt = 0;
    }
    DrvHdmitxOsSetHpdIrq(u32HpdInt ? 1 : 0);
}

void _DrvHdmitxUpdateI2cId(struct platform_device *pDev)
{
    u32 u32Val;

    if(CamofPropertyReadU32(pDev->dev.of_node, "i2c_id", &u32Val))
    {
        u32Val = 0;
    }

    DrvHdmitxOsSetI2cId(u32Val);
}


void _DrvHdmitxUpdateHpdGpio(struct platform_device *pDev)
{
    u32 u32Val;

    if(CamofPropertyReadU32(pDev->dev.of_node, "hpd_gpio", &u32Val))
    {
        printf(PRINT_RED "%s %d, No hpd_gpio property \n" PRINT_NONE, __FUNCTION__, __LINE__);
    }
    else
    {

        MS_U8 u8Pin = u32Val & 0xFF;
        DrvHdmitxOsSetHpdGpinPin(u8Pin);
        DrvHdmitxOsGpioRequestInput(u8Pin);
    }
}

void _DrvHdmixUpdateSWDdcGpio(struct platform_device *pDev)
{
    u32 u32SwI2c = 0;
    u32 u32I2cSda = 0;
    u32 u32I2cScl = 0;
    if(CamofPropertyReadU32(pDev->dev.of_node, "i2c_sw", &u32SwI2c))
    {
        printf(PRINT_RED "%s %d, No hpd_gpio property \n" PRINT_NONE, __FUNCTION__, __LINE__);
        u32SwI2c = 0;
    }

    DrvHdmitxOsSetSwI2cEn(u32SwI2c ? TRUE : FALSE);

    if(u32SwI2c)
    {
        if(CamofPropertyReadU32(pDev->dev.of_node, "i2c_sda_gpio", &u32I2cSda))
        {
            printf(PRINT_RED "%s %d, No i2c_sda_gpio property \n" PRINT_NONE, __FUNCTION__, __LINE__);
            u32I2cSda = 0;
        }

        if(CamofPropertyReadU32(pDev->dev.of_node, "i2c_scl_gpio", &u32I2cScl))
        {
            printf(PRINT_RED "%s %d, No i2c_sda_gpio property \n" PRINT_NONE, __FUNCTION__, __LINE__);
            u32I2cSda = 0;
        }

        DrvHdmitxOsSetSwI2cPin(u32I2cSda, u32I2cScl);
    }
}

void _DrvHdmitxModuleInit(void)
{
    int s32Ret;
    dev_t  dev;

    if(_tHdmitxDevice.s32Major)
    {
        dev     = MKDEV(_tHdmitxDevice.s32Major, _tHdmitxDevice.s32Minor);
        if(!_tHdmitxClass)
        {
            _tHdmitxClass = msys_get_sysfs_class();
            if(!_tHdmitxClass)
            {
                _tHdmitxClass = CamClassCreate(THIS_MODULE, HdmitxClassName);
            }
        }
        else
        {
            cdev_init(&_tHdmitxDevice.cdev, &_tHdmitxDevice.fops);
            if (0 != (s32Ret= cdev_add(&_tHdmitxDevice.cdev, dev, DRV_HDMITX_DEVICE_COUNT)))
            {
                printk( "[HDMITX] Unable add a character device\n");
            }
        }
        //ToDo
        if(_tHdmitxDevice.devicenode==NULL)
        {
            _tHdmitxDevice.devicenode = CamDeviceCreate(_tHdmitxClass, NULL, dev,NULL, DRV_HDMITX_DEVICE_NAME);
            DrvHdmitxSysfsInit(_tHdmitxDevice.devicenode);
        }

        if(stDrvHdmitxPlatformDevice.dev.of_node==NULL)
        {
            stDrvHdmitxPlatformDevice.dev.of_node = of_find_compatible_node(NULL, NULL, "sstar,hdmitx");
        }
        if(stDrvHdmitxPlatformDevice.dev.of_node==NULL)
        {
            printk("[HDMITX INIT] Get Device mode Fail!!\n");
        }

        _DrvHdmitxUpdateI2cId(&stDrvHdmitxPlatformDevice);


        _DrvHdmitxUpdateHpdGpio(&stDrvHdmitxPlatformDevice);
        _DrvHdmiTxUpdateHpdIrq(&stDrvHdmitxPlatformDevice);
        _DrvHdmixUpdateSWDdcGpio(&stDrvHdmitxPlatformDevice);
        DrvHdmitxOsSetDeviceNode(&stDrvHdmitxPlatformDevice);

    }
}

