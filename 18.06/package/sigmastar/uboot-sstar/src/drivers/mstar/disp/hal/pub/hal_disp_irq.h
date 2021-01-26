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

#ifndef _HAL_DISP_IRQ_H_
#define _HAL_DISP_IRQ_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_DISP_IRQ_IOCTL_ENABLE,
    E_HAL_DISP_IRQ_IOCTL_GET_FLAG,
    E_HAL_DISP_IRQ_IOCTL_CLEAR,

    E_HAL_DISP_IRQ_IOCTL_INTERNAL_SUPPORTED,
    E_HAL_DISP_IRQ_IOCTL_INTERNAL_GET_ID,
    E_HAL_DISP_IRQ_IOCTL_INTERNAL_ENABLE,
    E_HAL_DISP_IRQ_IOCTL_INTERNAL_GET_FLAG,
    E_HAL_DISP_IRQ_IOCTL_INTERNAL_CLEAR,
    E_HAL_DISP_IRQ_IOCTL_INTERNAL_GET_STATUS,

    E_HAL_DISP_IRQ_IOCTL_VGA_HPD_SUPPORTED,
    E_HAL_DISP_IRQ_IOCTL_VGA_HPD_GET_ID,
    E_HAL_DISP_IRQ_IOCTL_VGA_HPD_ENABLE,
    E_HAL_DISP_IRQ_IOCTL_VGA_HPD_GET_FLAG,
    E_HAL_DISP_IRQ_IOCTL_VGA_HPD_CLEAR,
    E_HAL_DISP_IRQ_IOCTL_VGA_HPD_GET_STATUS,
    E_HAL_DISP_IRQ_IOCTL_NUM,
}HalDispIrqIoCtlType_e;

typedef enum
{
    E_HAL_DISP_IRQ_TYPE_NONE                    = 0x0000,
    E_HAL_DISP_IRQ_TYPE_VSYNC                   = 0x0001,
    E_HAL_DISP_IRQ_TYPE_VDE                     = 0x0002,
    E_HAL_DISP_IRQ_TYPE_INTERNAL_VDE_NEGATIVE   = 0x0100,
    E_HAL_DISP_IRQ_TYPE_INTERNAL_VDE_POSITIVE   = 0x0200,
    E_HAL_DISP_IRQ_TYPE_INTERNAL_VSYNC_NEGATIVE = 0x0400,
    E_HAL_DISP_IRQ_TYPE_INTERNAL_VSYNC_POSITIVE = 0x0800,
    E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON              = 0x1000,
    E_HAL_DISP_IRQ_TYPE_VGA_HPD_OFF             = 0x2000,
}HalDispIrqType_e;



typedef struct
{
    HalDispIrqIoCtlType_e enType;
    void *pParam;
}HalDispIrqIoctlConfig_t;
//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------
#ifdef _HAL_DISP_IRQ_C_
#define INTERFACE

#else
#define INTERFACE extern
#endif

INTERFACE bool HalDispIrqIoCtl(HalDispIrqIoCtlType_e enIoctlType, HalDispIrqType_e enIrqType, void *pCfg);
INTERFACE void HalDispIrqSetDacEn(u8 u8Val);

#undef INTERFACE

#endif
