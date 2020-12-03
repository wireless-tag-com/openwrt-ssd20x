/*
* boot_loader.h- Sigmastar
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
#ifndef _BOOT_LOADER_H_
#define _BOOT_LOADER_H_

#include <linux/types.h>
#include "ceva_linkdrv_xm6.h"
#include "hal_ceva.h"
#include "hal_timer.h"
#include "ceva_linkdrv-generic.h"

int dsp_boot_up(ceva_hal_handle *handle, dev_dsp_buf *copier_buf, void *share_mem_virt, dsp_mem_info_t *dsp_mem_info);
void dsp_shut_down(ceva_hal_handle *handle);
//int dsp_dma_image_ext_transfer(struct boot_config_t boot_config);
int dsp_dma_image_ext_transfer(struct boot_config_t boot_config,u8 *image_virt);

#if 0
void MemoryClear(phys_addr_t pa_addr,unsigned int size);
#endif

#define _FAST_DMA_BOOT_
//#undef _FAST_DMA_BOOT_

//#define _Measure_BootUp_Time_
#undef _Measure_BootUp_Time_

#endif // _BOOT_LOADER_H_
