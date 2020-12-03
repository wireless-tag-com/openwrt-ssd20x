/*
* ceva_linkdrv-generic.h- Sigmastar
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
 * ceva_linkdrv-generic.h
 *
 *  Created on: Nov 13, 2013
 *  Author: Ido Reis <ido.reis@tandemg.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef CEVA_LINKDRV_GENERIC_H_
#define CEVA_LINKDRV_GENERIC_H_

#include <linux/kernel.h>
#include <asm/types.h>
#include <linux/ioctl.h>


/*!
 * Device name for kernel registration
 */
#define CEVA_PCIDEV_DEVICE_NAME                             "ceva_linkdrv_xm6_"
/*!
 * dma region size
 */
#define CEVADEV_PCI_DMA_MEM_SIZE_ORDER                          (20)
#define CEVADEV_PCI_DMA_MEM_SIZE                                (1<<CEVADEV_PCI_DMA_MEM_SIZE_ORDER) // (1024*1024*16)
/*!
 * kfifo max size
 */
#define CEVA_LINKDRV_FIFO_MAX_EVENTS                                      (128)

/*!
 * Device name for sharem memory driver
 */
#define CEVA_PROT_MEM_DRV_NAME                               "protected_mem_db"

/*!
 * debug statistics information
 */
struct ceva_linkdrv_debug_info_s{
    unsigned long int bypass_interrupts_success;
    unsigned long int bypass_interrupts_failed;
    unsigned long int events_recieved;
    unsigned long int fifo_full;
    unsigned long int generated_interrupts;
    unsigned long int generate_interrupt_failures;
    unsigned long int unhandled_irq;
    unsigned long int handled_irq;
    unsigned long int fifo_in_err;
};

/*!
 * container class for debug statistics
 *
 * we wrap the statistics with the union in order to allow developers
 * to modify the driver code, yet without updating any user code which
 * aligned to older structure.
 * user should allocate with the size of union ceva_linkdrv_debug_info,
 * but actual data copied will be at the size of actual data.
 */
union ceva_linkdrv_debug_info {
    struct ceva_linkdrv_debug_info_s data;
    unsigned long int raw[250];
};


/*!
 * data structure for boot loader
 */
struct boot_buffer_t {
    __u8    *addr;
    __u32   size;
};

struct boot_config_t {
    struct boot_buffer_t boot_image;    // address of boot_image is virtual address
    struct boot_buffer_t share_mem;     // address of share_mem is physical address
    struct boot_buffer_t ext_heap;      // address of ext_heap is physical address
    struct boot_buffer_t work_buffer;   // address of work_buffer is physical address
    __u32                custom_info;
};

/*!
 * bypass definitions
 */
#define CEVADEV_BYPASS__IN_BUF_OFFSET                                    (0x0)
#define CEVADEV_BYPASS__OUT_BUF_OFFSET                                (0x1000)
#define CEVADEV_BYPASS__BUF_LEN                                       (0x1000)
#define CEVADEV_BYPASS__EVENT                                           (0x10)

/*!
 * struct to maintain read and write operations
 *
 * read/write operations from user include offset variable
 * to read from (or write to). the read/write user functions
 * will pass an object of this struct as it's in/out buffer.
 * driver will first read the offset field (using copy_from_user)
 * and then perform the r/w from/to the passed buffer address
 */
struct RWBuffer {
    unsigned long offset;
    void *buf;
};

/*
 * ioctl commands
 */
#define IOC_CEVADRV_MAGIC            (0xFB)

#define IOC_CEVADRV_PRINT_DMA       _IOR(IOC_CEVADRV_MAGIC,   1, int)
#define IOC_CEVADRV_BYPASS_REQ      _IOW(IOC_CEVADRV_MAGIC,   2, int)
#define IOC_CEVADRV_READ_DEBUG      _IOWR(IOC_CEVADRV_MAGIC,  3, union ceva_linkdrv_debug_info)
#define IOC_CEVADRV_GENERATE_INT    _IOW(IOC_CEVADRV_MAGIC,   4, int)
#define IOC_CEVADRV_GET_PID_PROCID  _IOR(IOC_CEVADRV_MAGIC,   5, unsigned long)
#define IOC_CEVADRV_BOOT_UP         _IOR(IOC_CEVADRV_MAGIC,   6, struct boot_config_t)

#define IOC_CEVADRV_MAXNR            6

/*
 * log macros
 * ENABLE_DEBUG and ENABLE_TRACE are defined in the makefile
 */

#define CEVA_LOG_PREFIX "ceva_link"

#ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
# define PDLOGF printk
# define CEVA_TRACE KERN_DEBUG "[" CEVA_LOG_PREFIX "] T "
# define CEVA_DEBUG KERN_DEBUG "[" CEVA_LOG_PREFIX "] D "
# define CEVA_INFO KERN_INFO "[" CEVA_LOG_PREFIX "] I "
# define CEVA_WARN KERN_WARNING "[" CEVA_LOG_PREFIX "] W "
# define CEVA_ERR KERN_ERR "[" CEVA_LOG_PREFIX "] E "
#else
/* This one for user space */
# define PDLOGF printf
# define CEVA_TRACE "[" CEVA_LOG_PREFIX "] T "
# define CEVA_DEBUG "[" CEVA_LOG_PREFIX "] D "
# define CEVA_INFO "[" CEVA_LOG_PREFIX "] I "
# define CEVA_WARN "[" CEVA_LOG_PREFIX "] W "
# define CEVA_ERR "[" CEVA_LOG_PREFIX "] E "
#endif /* kernel/user */

#ifdef ENABLE_DEBUG
#define TRACE() PDLOGF(CEVA_TRACE "%s:%d", __FUNCTION__, __LINE__)
# define LOG_D(fmt, args...) PDLOGF(CEVA_DEBUG fmt "\n", ## args)
# define LOG_I(fmt, args...) PDLOGF(CEVA_INFO fmt "\n", ## args)
# define LOG_W(fmt, args...) PDLOGF(CEVA_WARN fmt "\n", ## args)
# define LOG_E(fmt, args...) PDLOGF(CEVA_ERR fmt "\n", ## args)
#else /* ENABLE_DEBUG */
# define TRACE()
# define LOG_D(fmt, args...)
# define LOG_I(fmt, args...)
# define LOG_W(fmt, args...)
# define LOG_E(fmt, args...)
#endif /* ENABLE_DEBUG */

#undef PDEBUG             /* undef it, just in case */
#define PDEBUG LOG_D

#ifdef ENABLE_TRACE /* PDEBUG must be defined for this to work */
#define FUNCTION_ENTRY() PDEBUG("in %s\n", __PRETTY_FUNCTION__)
#define FUNCTION_EXIT()  PDEBUG("leaving %s\n", __PRETTY_FUNCTION__)
#else /* ENABLE_TRACE */
#define FUNCTION_ENTRY()
#define FUNCTION_EXIT()
#endif /* ENABLE_TRACE */

#endif /* CEVA_LINKDRV_GENERIC_H_ */
