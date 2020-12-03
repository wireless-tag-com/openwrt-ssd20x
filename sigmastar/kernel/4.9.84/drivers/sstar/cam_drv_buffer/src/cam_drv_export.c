/*
* cam_drv_export.c- Sigmastar
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
///////////////////////////////////////////////////////////////////////////////
/// @file      cam_drv_export.c
/// @brief     Cam Drv Export Symbol Source File for Linux Kernel Space
///            Only Include This File in Linux Kernel
///////////////////////////////////////////////////////////////////////////////

#include <linux/module.h>
#include "cam_drv_buffer.h"

MODULE_LICENSE("GPL");

EXPORT_SYMBOL(CamDrvQueueInit);
EXPORT_SYMBOL(CamDrvQueueDeinit);
EXPORT_SYMBOL(CamDrvQueuePush);
EXPORT_SYMBOL(CamDrvQueuePop);
EXPORT_SYMBOL(CamDrvQueueQuery);

EXPORT_SYMBOL(CamDrvBuffInit);
EXPORT_SYMBOL(CamDrvBuffDeinit);
EXPORT_SYMBOL(CamDrvBuffAdd);
EXPORT_SYMBOL(CamDrvBuffGet);
EXPORT_SYMBOL(CamDrvBuffRecycle);
EXPORT_SYMBOL(CamDrvBuffFill);
EXPORT_SYMBOL(CamDrvBuffQuery);
EXPORT_SYMBOL(CamDrvBuffDone);
