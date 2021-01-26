/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef _DRV_PNL_OS_HEADER_H_
#define _DRV_PNL_OS_HEADER_H_

#include <linux/pfn.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>          /* seems do not need this */
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/string.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>

#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>


#include "cam_os_wrapper.h"


//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define PNL_IO_OFFSET 0xDE000000

#define PNL_IO_ADDRESS(x)           ( (u32)(x) + PNL_IO_OFFSET )
//#define __io_address(n)       ((void __iomem __force *)PNL_IO_ADDRESS(n))

/* read register by byte */
#define pnl_readb(a) (*(volatile unsigned char *)PNL_IO_ADDRESS(a))

/* read register by word */
#define pnl_readw(a) (*(volatile unsigned short *)PNL_IO_ADDRESS(a))

/* read register by long */
#define pnl_readl(a) (*(volatile unsigned int *)PNL_IO_ADDRESS(a))

/* write register by byte */
#define pnl_writeb(v,a) (*(volatile unsigned char *)PNL_IO_ADDRESS(a) = (v))

/* write register by word */
#define pnl_writew(v,a) (*(volatile unsigned short *)PNL_IO_ADDRESS(a) = (v))

/* write register by long */
#define pnl_writel(v,a) (*(volatile unsigned int *)PNL_IO_ADDRESS(a) = (v))





#define READ_BYTE(x)         pnl_readb(x)
#define READ_WORD(x)         pnl_readw(x)
#define READ_LONG(x)         pnl_readl(x)
#define WRITE_BYTE(x, y)     pnl_writeb((u8)(y), x)
#define WRITE_WORD(x, y)     pnl_writew((u16)(y), x)
#define WRITE_LONG(x, y)     pnl_writel((u32)(y), x)

#endif
