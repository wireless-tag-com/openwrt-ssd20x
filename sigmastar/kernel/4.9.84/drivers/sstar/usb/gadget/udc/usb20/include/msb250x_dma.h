/*
* msb250x_dma.h- Sigmastar
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
/*
* ms_dma.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: raul.wang <raul.wang@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef __MSB250X_DMA_H
#define __MSB250X_DMA_H

#if 0
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#if defined( CONFIG_ARM )
//#include <mach/hardware.h>
#include <asm/mach-types.h>
#endif
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>

#include <asm/byteorder.h>
#include <asm/dma.h>
#include <asm/io.h>
#include <asm/irq.h>
//#include <asm/system.h>
#include <asm/unaligned.h>

#include <linux/usb.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>

#include "msb250x_udc.h"
#include "msb250x_udc_reg.h"
#endif

u8 msb250x_dma_find_channel_by_ep(u8 ep_num);

void msb250x_dma_release_channel(s8 ch);

int msb250x_dma_setup_control(struct usb_ep *_ep,
                              struct msb250x_request *req,
                              u32 bytes);

void msb250x_dma_isr_handler(u8 ch,
                             struct msb250x_udc *dev);

#endif
