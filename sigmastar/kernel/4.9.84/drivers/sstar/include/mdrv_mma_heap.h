/*
* mdrv_mma_heap.h- Sigmastar
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
#include <linux/types.h>
#include <linux/string.h>



#define MMA_HEAP_NAME_LENG  32

//enable max mma areas be a large value .
#define MAX_MMA_AREAS 30

struct MMA_BootArgs_Config {
     int miu;//input :from bootargs or dts
     unsigned long size;//input :from bootargs or dts
     char name[MMA_HEAP_NAME_LENG];//input :from bootargs or dts
     unsigned long max_start_offset_to_curr_bus_base;
     unsigned long max_end_offset_to_curr_bus_base;//input:for vdec use.

     phys_addr_t reserved_start;//out: reserved_start
};
