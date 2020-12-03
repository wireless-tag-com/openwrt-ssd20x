/*
* hal_intr_ctrl.c- Sigmastar
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
#include "hal_intr_ctrl.h"
#include "ms_platform.h"
#include <linux/delay.h>

#define LOW_U16(value)   (((u32)(value))&0x0000FFFF)
#define HIGH_U16(value)  ((((u32)(value))&0xFFFF0000)>>16)

#define MAKE_U32(high, low) ((((u32)high)<<16) || low)

#if 1
#define REGR(base,idx)      ms_readw(((uint)base+(idx)*4))
#define REGW(base,idx,val)  ms_writew(val,((uint)base+(idx)*4))
#else
#define REGR(base,idx)      ms_readw(((uint)base+(idx)*4))
#define REGW(base,idx,val)  do{IVE_MSG(IVE_MSG_DBG, "write 0x%08X = 0x%04X\n", ((uint)base+(idx)*4), val); ms_writew(val,((uint)base+(idx)*4));} while(0)
#endif

void intr_ctrl_hal_init(intr_ctrl_hal_handle *handle, phys_addr_t base_addr)
{
    memset(handle, 0, sizeof(handle[0]));
    handle->base_addr = base_addr;
}

void intr_ctrl_hal_software_interrupt(intr_ctrl_hal_handle *handle, INTR_CTRL_HAL_SOFT_INTERRUPT interrupt)
{
        handle->register.reg22 = REGR(handle->base_sys, 0x22);
        handle->register.reg_hst1_fiq_force_47_32 |= interrupt;
        REGW(handle->base_addr, 0x22, handle->register.reg22);
}
