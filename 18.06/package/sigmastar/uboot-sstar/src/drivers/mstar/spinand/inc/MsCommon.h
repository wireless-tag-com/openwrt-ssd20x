/*
* MsCommon.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: edie.chen <edie.chen@sigmastar.com.tw>
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

#ifndef _MS_COMMON_H_
#define _MS_COMMON_H_

#if defined(MSOS_TYPE_LINUX_KERNEL)
#include "mach/platform.h"
#include <linux/kernel.h>
#include <linux/interrupt.h>

#endif

#ifndef __UBOOT__
#ifndef MSOS_TYPE_LINUX_KERNEL
#include <stdio.h>
#include <stdlib.h>
#else
#endif


#include <stdarg.h>

#include "MsTypes.h"
#include "MsIRQ.h"
//#include "MsVersion.h"
#include "MsOS.h"                                                       // Plan to be obsoleted in next generation.
#include "MsDevice.h"
//#include <setjmp.h>
#endif
//-------------------------------------------------------------------------------------------------
// Defines
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
#define GEN_EXCEP   { while(1); }

//#define REG(addr) (*(volatile U32 *)(addr))
#if 0
#ifdef __aeon__

//-------------------------------------------------------------------------------------------------
//  In order to keep the compatiblity of the source code from Venus,
//  keep these memory address translation for a while.
//  They will be removed in the future.
//-------------------------------------------------------------------------------------------------
#define CACHED_BASE            ((void *)0x00000000)
#define UNCACHED_BASE       ((void *)0x80000000)

#define CACHED_SIZE         ((void *)0x20000000)
#define UNCACHED_SIZE        ((void *)0x20000000)
//  0xA0000000~0xA000FFFF belongs to RIU
//  0xA1000000~           belongs to SPI

//cached/unchched segment
#define KSEG0_BASE        CACHED_BASE
#define KSEG1_BASE        UNCACHED_BASE
#define KSEG0_SIZE        CACHED_SIZE
#define KSEG1_SIZE        UNCACHED_SIZE

//cached <-> uncached
#define KSEG02KSEG1(addr)  ((void *)((U32)(addr)|0x80000000))
#define KSEG12KSEG0(addr)  ((void *)((U32)(addr)&~0x80000000))

//virtual <-> physical
#define VA2PA(addr)         ((void *)(((U32)addr) & 0x1fffffff))
#define PA2KSEG0(addr)         ((void *)(((U32)addr) | 0x00000000))
#define PA2KSEG1(addr)         ((void *)(((U32)addr) | 0x80000000))
#endif

#if defined(__mips__)

//cached/unchched segment
#define KSEG0_BASE                ((void *)0x80000000)
#define KSEG1_BASE                ((void *)0xa0000000)
#define KSEG0_SIZE                0x20000000
#define KSEG1_SIZE                0x20000000

//cached addr <-> unchched addr
#define KSEG02KSEG1(addr)       ((void *)((MS_U32)(addr)|0x20000000))  //cached -> unchched
#define KSEG12KSEG0(addr)       ((void *)((MS_U32)(addr)&~0x20000000)) //unchched -> cached

//virtual addr <-> physical addr
#define VA2PA(addr)             ((void *)(((MS_U32)addr) & 0x1fffffff)) //virtual -> physical
#define PA2KSEG0(addr)             ((void *)(((MS_U32)addr) | 0x80000000)) //physical -> cached
#define PA2KSEG1(addr)             ((void *)(((MS_U32)addr) | 0xa0000000)) //physical -> unchched
#endif
#endif

#if 0
//cached/unchched segment
#define KSEG0_BASE        ((void *)0x80000000)
#define KSEG1_BASE        ((void *)0xa0000000)
#define KSEG0_SIZE        0x20000000
#define KSEG1_SIZE        0x20000000

//cached <-> unchched
#define KSEG02KSEG1(addr)  ((void *)((U32)(addr)|0x20000000))
#define KSEG12KSEG0(addr)  ((void *)((U32)(addr)&~0x20000000))

//virtual <-> physical
#define VA2PA(addr)         ((void *)(((U32)addr) & 0x1fffffff))
#define PA2KSEG0(addr)         ((void *)(((U32)addr) | 0x80000000))
#define PA2KSEG1(addr)         ((void *)(((U32)addr) | 0xa0000000))
#endif

//user-defined assert
#ifdef MS_DEBUG
#define MS_ASSERT(_bool_)                                                                                      \
        {                                                                                                   \
            if ( ! ( _bool_ ) )                                                                             \
            {                                                                                               \
/*                UTL_printf("ASSERT FAIL: %s, %s %s %d\n", #_bool_, __FILE__, __PRETTY_FUNCTION__, __LINE__); */ \
/*                MAsm_CPU_SwDbgBp();                                                                          */ \
            }                                                                                               \
        }
#else
#define MS_ASSERT(_bool_)                                                                                      \
        {                                                                                                   \
            if ( ! ( _bool_ ) )                                                                             \
            {                                                                                               \
/*                UTL_printf("ASSERT FAIL: %s %s %s %d\n", #_bool_, __FILE__, __PRETTY_FUNCTION__, __LINE__); */ \
            }                                                                                               \
        }
#endif


#ifndef MIN
#define MIN(_a_, _b_)               ((_a_) < (_b_) ? (_a_) : (_b_))
#endif
#ifndef MAX
#define MAX(_a_, _b_)               ((_a_) > (_b_) ? (_a_) : (_b_))
#endif


#define ALIGN_4(_x_)                (((_x_) + 3) & ~3)
#define ALIGN_8(_x_)                (((_x_) + 7) & ~7)
#define ALIGN_16(_x_)               (((_x_) + 15) & ~15)           // No data type specified, optimized by complier
#define ALIGN_32(_x_)               (((_x_) + 31) & ~31)           // No data type specified, optimized by complier

#define MASK(x)     (((1<<(x##_BITS))-1) << x##_SHIFT)


//!!! avoid warning of out-of-range integer !!!//
#if defined(MSOS_TYPE_NOS)	// for chip back verificatoin
#define BIT(_bit_)                  (1UL << (_bit_))
#else

#ifndef BIT
#define BIT(_bit_)                  (1 << (_bit_))
#endif

#endif

#define BIT_(x)                     BIT(x) //[OBSOLETED] //TODO: remove it later
#define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
#define BMASK(_bits_)               (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))


#if defined(MSOS_TYPE_LINUX_KERNEL)

#define READ_BYTE(x)         ms_readb(x)
#define READ_WORD(x)         ms_readw(x)
#define READ_LONG(x)         ms_readl(x)
#define WRITE_BYTE(x, y)     ms_writeb((MS_U8)(y), x)
#define WRITE_WORD(x, y)     ms_writew((MS_U16)(y), x)
#define WRITE_LONG(x, y)     ms_writel((MS_U32)(y), x)

#else

#define READ_BYTE(_reg)             (*(volatile MS_U8*)(_reg))
#define READ_WORD(_reg)             (*(volatile MS_U16*)(_reg))
#define READ_LONG(_reg)             (*(volatile MS_U32*)(_reg))

#if ( defined(MSOS_TYPE_CE) || defined(MSOS_TYPE_NOS) )
    #define WRITE_BYTE(_reg, _val)      (*((volatile MS_U8*)(_reg))) = (MS_U8)(_val)
    #define WRITE_WORD(_reg, _val)      (*((volatile MS_U16*)(_reg))) = (MS_U16)(_val)
#else
    #define WRITE_BYTE(_reg, _val)      { (*((volatile MS_U8*)(_reg))) = (MS_U8)(_val); }
    #define WRITE_WORD(_reg, _val)      { (*((volatile MS_U16*)(_reg))) = (MS_U16)(_val); }
#endif
#define WRITE_LONG(_reg, _val)      { (*((volatile MS_U32*)(_reg))) = (MS_U32)(_val); }
#endif


#endif // _MS_COMMON_H_
