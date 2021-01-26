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

#ifndef _DRV_PNL_MODULE_H_
#define _DRV_PNL_MODULE_H_

//-------------------------------------------------------------------------------------------------
// Define & Macro
//-------------------------------------------------------------------------------------------------
#define DRV_PNL_DEVICE_COUNT    1
#define DRV_PNL_DEVICE_NAME     "mpnl"
#define DRV_PNL_DEVICE_MAJOR    0xEB
#define DRV_PNL_DEVICE_MINOR    0x10


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
}DrvPnlModuleDevice_t;


//-------------------------------------------------------------------------------------------------
// Varialbe
//-------------------------------------------------------------------------------------------------
#ifdef _PNL_MODULE_C_

DrvPnlModuleDevice_t _tPnlDevice =
{
    .s32Major = DRV_PNL_DEVICE_MAJOR,
    .s32Minor = DRV_PNL_DEVICE_MINOR,
    .refCnt = 0,
    .devicenode = NULL,
    .cdev =
    {
        .kobj = {.name= DRV_PNL_DEVICE_NAME, },
        .owner = THIS_MODULE,
    },
    /*
    .fops =
    {
        .open = DrvPnlModuleOpen,
        .release = DrvPnlModuleRelease,
        .unlocked_ioctl = DrvPnlModuleIoctl,
        .poll = DrvPnlModulePoll,
    },*/
};

struct class * _tPnlClass = NULL;
char * PnlClassName = "m_pnl_class";


u64 u64Pnl_DmaMask = 0xffffffffUL;

struct platform_device stDrvPnlPlatformDevice =
{
    .name = DRV_PNL_DEVICE_NAME,
    .id = 0,
    .dev =
    {
        .of_node = NULL,
        .dma_mask = &u64Pnl_DmaMask,
        .coherent_dma_mask = 0xffffffffUL
    }
};


#endif

//-------------------------------------------------------------------------------------------------
// Prototype
//-------------------------------------------------------------------------------------------------

#ifdef _DRV_PNL_MODULE_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif



#undef INTERFACE

#endif
