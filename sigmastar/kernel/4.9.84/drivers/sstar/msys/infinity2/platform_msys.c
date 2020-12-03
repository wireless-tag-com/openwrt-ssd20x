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
    if(freq_info->padnum != PAD_CLOCK_OUT)
    {
        MSYS_ERROR("Not implement PAD: %d for this platform\n", freq_info->padnum);
        return -EINVAL;
    }

    if(freq_info->bEnable)
    {
        if(freq_info->freq != FREQ_24MHZ)
        {
            MSYS_ERROR("Not implement FREQ: %d for this platform\n", freq_info->freq);
            return -EINVAL;
        }

        if(freq_info->bInvert != false)
            MSYS_WARN("Not support invert clock in this platform");

        switch(freq_info->padnum)
        {
            case PAD_CLOCK_OUT:
                // reg_ext_xtali_se_enb[1]=0
                CLRREG16(BASE_REG_PMSLEEP_PA + REG_ID_30, BIT1);//enable clock
                break;
            default:
                MSYS_ERROR("Not implement PAD: %d for this platform\n", freq_info->padnum);
                break;
        }
    }
    else //disable clk
    {
        switch(freq_info->padnum)
        {
            case PAD_CLOCK_OUT:
                // reg_ext_xtali_se_enb[1]=1
                SETREG16(BASE_REG_PMSLEEP_PA + REG_ID_30, BIT1); //disable clk
                break;
            default:
                MSYS_ERROR("Not implement PAD: %d for this platform\n", freq_info->padnum);
                break;
        }
    }
    return 0;
}
