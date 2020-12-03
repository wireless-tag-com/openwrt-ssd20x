/*
* hal_cpm.c- Sigmastar
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
#include "hal_cpm.h"
#include "ms_platform.h"
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <asm/page.h>

#include "hal_debug.h"


#define LOW_U16(value)   (((u32)(value))&0x0000FFFF)
#define HIGH_U16(value)  ((((u32)(value))&0xFFFF0000)>>16)

#define MAKE_U32(high, low) ((((u32)high)<<16) || low)

#if (HAL_MSG_LEVL < HAL_MSG_DBG)
#define REGR(base,idx)      *((volatile u32*)((u32)(base)+(idx)))
#define REGW(base,idx,val)  do{*((volatile u32*)((u32)(base)+(idx))) = (val);} while(0)
#else
#define REGR(base,idx)      *((volatile u32*)((uint)(base)+(idx)))
#define REGW(base,idx,val)  do{HAL_MSG(HAL_MSG_DBG, "write 0x%08X = 0x%04X\n", ((uint)(base)+(idx)), val); *((volatile u32*)((uint)(base)+(idx))) = (val);} while(0)
#endif

void cmp_hal_init(cpm_hal_handle *handle, phys_addr_t base_addr)

{
    memset(handle, 0, sizeof(handle[0]));
    handle->base_addr = (phys_addr_t)ioremap_nocache(base_addr, 0x100);

    HAL_MSG(HAL_MSG_DBG, "base addr 0x%X, remapped 0x%X\n", base_addr, handle->base_addr);
}

void cmp_hal_mcci_write(cpm_hal_handle *handle, u32 index, u32 value)
{
    index *= 4;

    REGW(handle->base_addr, index, value);
}

u16 cmp_hal_mcci_read(cpm_hal_handle *handle, u32 index)
{
		u16 ret=0;
    index *= 4;

    ret=REGR(handle->base_addr, index);
    return ret;
}