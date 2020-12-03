/*
* cam_fs_export.c- Sigmastar
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
/// @file      cam_fs_export.c
/// @brief     Cam FS Export Symbol Source File for Linux Kernel Space
///            Only Include This File in Linux Kernel
///////////////////////////////////////////////////////////////////////////////

#include <linux/module.h>
#include "cam_fs_wrapper.h"

EXPORT_SYMBOL(CamFsOpen);
EXPORT_SYMBOL(CamFsClose);
EXPORT_SYMBOL(CamFsRead);
EXPORT_SYMBOL(CamFsWrite);
EXPORT_SYMBOL(CamFsSeek);
