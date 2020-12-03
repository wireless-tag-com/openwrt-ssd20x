/*
* platform_msys.c- Sigmastar
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

#include <asm/uaccess.h>
#include <linux/errno.h>
#include "ms_platform.h"
#include "gpio.h"
#include "registers.h"
#include "mdrv_msys_io_st.h"

#define MSYS_ERROR(fmt, args...)    printk(KERN_ERR"MSYS: " fmt, ## args)
#define MSYS_WARN(fmt, args...)     printk(KERN_WARNING"MSYS: " fmt, ## args)

int msys_request_freq(MSYS_FREQGEN_INFO *freq_info)
{
    MSYS_ERROR("Not implement PAD: %d for this platform\n", freq_info->padnum);
    MSYS_ERROR("%s not support for this platform\n", __FUNCTION__);
    return -EPERM;
}
