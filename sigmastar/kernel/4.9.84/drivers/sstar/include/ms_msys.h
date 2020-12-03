/*
* ms_msys.h- Sigmastar
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
#ifndef _MS_MSYS_H_
#define _MS_MSYS_H_

#include "mdrv_msys_io_st.h"
#include "linux/proc_fs.h"

extern int msys_release_dmem(MSYS_DMEM_INFO *mem_info);
extern int msys_request_dmem(MSYS_DMEM_INFO *mem_info);
extern void ms_records_irq(MSYS_IRQ_INFO *irq_info);
extern void msys_dump_irq_info(void);
extern void ms_records_sirq(MSYS_IRQ_INFO *irq_info);
extern void msys_dump_sirq_info(void);
extern void ms_dump_irq_count(void);
extern struct proc_dir_entry* msys_get_proc_zen(void);
extern struct proc_dir_entry* msys_get_proc_zen_kernel(void);
extern struct proc_dir_entry* msys_get_proc_zen_mi(void);
extern struct proc_dir_entry* msys_get_proc_zen_omx(void);
extern struct class *msys_get_sysfs_class(void);

extern void ms_record_large_latency_in_top(MSYS_IRQ_INFO *irq_info);
void msys_print(const char *fmt, ...);
void msys_prints(const char *string, int length);
void msys_prints(const char *string, int length);
extern int msys_read_uuid(unsigned long long* udid);
extern int msys_dma_blit(MSYS_DMA_BLIT *cfg);
extern int msys_dma_fill(MSYS_DMA_FILL *cfg);
extern int msys_dma_copy(MSYS_DMA_COPY *cfg);
#if defined(CONFIG_MS_BDMA_LINE_OFFSET_ON)
extern int msys_dma_fill_lineoffset(MSYS_DMA_FILL_BILT *pstDmaCfg);
extern int msys_dma_copy_lineoffset(MSYS_DMA_BLIT *cfg);
#endif
extern int ssys_get_HZ(void);

#endif
