/*
* mstar_mci.h- Sigmastar
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
#ifndef MSTAR_MCI_H
#define MSTAR_MCI_H

#include "eMMC.h"

/******************************************************************************
* Function define for this driver
******************************************************************************/

/******************************************************************************
* Register Address Base
******************************************************************************/
#define CLK_400KHz       		400*1000
#define CLK_200MHz       		200*1000*1000

#define eMMC_GENERIC_WAIT_TIME  (HW_TIMER_DELAY_1s*10)
#define eMMC_READ_WAIT_TIME     (HW_TIMER_DELAY_500ms)

/******************************************************************************
* Low level type for this driver
******************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,20)
#if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
struct mstar_mci_host_next
{
    unsigned int                dma_len;
    s32                         cookie;
};
#endif
#endif

struct mstar_mci_host
{
    struct mmc_host			    *mmc;
    struct mmc_command		    *cmd;
    struct mmc_request		    *request;

    void __iomem			    *baseaddr;
    s32						    irq;

    u16						    sd_clk;
    u16						    sd_mod;

    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,20)
    #if defined(ENABLE_EMMC_ASYNC_IO) && ENABLE_EMMC_ASYNC_IO
    struct mstar_mci_host_next  next_data;
    struct work_struct          async_work;
    #endif
    #endif
    char                    name[16];	
};  /* struct mstar_mci_host*/

#define MSTAR_MCI_NAME "MSTAR_MCI"

#endif
