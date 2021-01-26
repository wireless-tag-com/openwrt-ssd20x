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

#ifndef _DRV_HDMITX_MODULE_H
#define _DRV_HDMITX_MODULE_H

//-------------------------------------------------------------------------------------------------
// Define & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_HDMITX_DEVICE_COUNT    1
#define DRV_HDMITX_DEVICE_NAME     "mhdmitx"
#define DRV_HDMITX_DEVICE_MAJOR    0xEA
#define DRV_HDMITX_DEVICE_MINOR    0x10





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
}DrvHdmitxModuleDevice_t;


//-------------------------------------------------------------------------------------------------
// Varialbe
//-------------------------------------------------------------------------------------------------
#ifdef _HDMITX_MODULE_C_
DrvHdmitxModuleDevice_t _tHdmitxDevice =
{
    .s32Major = DRV_HDMITX_DEVICE_MAJOR,
    .s32Minor = DRV_HDMITX_DEVICE_MINOR,
    .refCnt = 0,
    .devicenode = NULL,
    .cdev =
    {
        .kobj = {.name= DRV_HDMITX_DEVICE_NAME, },
        .owner = THIS_MODULE,
    },
    /*
    .fops =
    {
        .open = DrvHdmitxModuleOpen,
        .release = DrvHdmitxModuleRelease,
        .unlocked_ioctl = DrvHdmitxModuleIoctl,
        .poll = DrvHdmitxModulePoll,
    },*/
};

struct class * _tHdmitxClass = NULL;
char * HdmitxClassName = "m_hdmitx_class";



u64 u64Hdmitx_DmaMask = 0xffffffffUL;

struct platform_device stDrvHdmitxPlatformDevice =
{
    .name = DRV_HDMITX_DEVICE_NAME,
    .id = 0,
    .dev =
    {
        .of_node = NULL,
        .dma_mask = &u64Hdmitx_DmaMask,
        .coherent_dma_mask = 0xffffffffUL
    }
};

#endif

//-------------------------------------------------------------------------------------------------
// Prototype
//-------------------------------------------------------------------------------------------------
#ifdef _DRV_HDMITX_MODULE_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif



#undef INTERFACE

#endif
