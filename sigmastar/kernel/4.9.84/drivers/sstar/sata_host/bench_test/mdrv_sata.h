/*
* mdrv_sata.h- Sigmastar
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


#include <asm/io.h>
#include "ms_platform.h"
#include <linux/pfn.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>          /* seems do not need this */
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <asm/uaccess.h>            /* access_ok(), and VERIFY_WRITE/READ */
#include <linux/ioctl.h>            /* for _IO() macro */
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/time.h>             /* do_gettimeofday() */


#define FUNC_MSG(fmt, args...)            ({do{printf(ASCII_COLOR_END"%s[%d] ",__FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}while(0);})
#define FUNC_ERR(fmt, args...)            ({do{printf(ASCII_COLOR_RED"%s[%d] ",__FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}while(0);})

#define DIP_DEBUG 0

#if (DIP_DEBUG==1)
    #define DIPDBG(fmt, args...)     printf(fmt, ## args)
#else
    #define DIPDBG(fmt, args...)
#endif

//#define  DIPIO_DEBUG_ENABLE

#define DIP_DBG_LV_IOCTL            1
#define DIP_DBG_LV_0                1


#ifdef   DIPIO_DEBUG_ENABLE
#define DIPIO_ASSERT(_con)   \
    do {\
        if (!(_con)) {\
            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                   __FILE__, __LINE__, #_con);\
            BUG();\
        }\
    } while (0)
/*
#define DIP_DBG(dbglv, _fmt, args...)             \
    do{\
        printk(KERN_INFO _fmt, ##args);\
    } while(0)
*/
//#define DIP_DBG(dbglv, fmt, arg...)   printk(KERN_INFO fmt, ##arg)
#define     DIP_DBG(dbglv, _fmt, _args...)  printk(KERN_DEBUG fmt, ##arg)

#else
#define     DIPIO_ASSERT(arg)
#define     DIP_DBG(dbglv, _fmt, _args...)

#endif
