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

#define eMMC_GENERIC_WAIT_TIME  (HW_TIMER_DELAY_1s*3)
#define eMMC_READ_WAIT_TIME     (HW_TIMER_DELAY_500ms)

/******************************************************************************
* Low level type for this driver
******************************************************************************/
struct mstar_mci_host
{
    struct mmc_host			    *mmc;
    struct mmc_command		    *cmd;
    struct mmc_request		    *request;

    void __iomem			    *baseaddr;
    s32						    irq;

    u16						    sd_clk;
    u16						    sd_mod;

    /* Flag indicating when the command has been sent.          */
    /* This is used to work out whether or not to send the stop */
    u32						flags;
    u32                     present;
    char                    name[16];
};  /* struct mstar_mci_host*/


#define MSTAR_MCI_NAME "MSTAR_MCI"
#endif
