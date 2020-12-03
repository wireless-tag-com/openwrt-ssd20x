/*
* drv_ive.h- Sigmastar
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
#ifndef _IVE_DRV_H_
#define _IVE_DRV_H_

#include <linux/kernel.h>
#include "mdrv_ive.h"
#include "hal_ive.h"
#include "mdrv_ive_io_st.h"

IVE_DRV_STATE  ive_drv_isr_handler(int irq, ive_drv_handle *handle);

int ive_drv_init(ive_drv_handle *handle, struct platform_device *pdev, phys_addr_t base_addr0, phys_addr_t base_addr1);
void ive_drv_release(ive_drv_handle *handle);
IVE_IOC_ERROR ive_drv_process(ive_drv_handle *handle, ive_file_data *file_data);
ive_file_data* ive_drv_post_process(ive_drv_handle *handle);

#endif
