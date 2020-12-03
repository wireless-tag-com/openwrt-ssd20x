/*
* ms_msys_memory_bench.c- Sigmastar
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
#include <linux/kernel.h>
#include <asm/uaccess.h>  /* for get_fs*/
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>      /* for dma_alloc_coherent */
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/compaction.h> /*  for sysctl_compaction_handler*/
#include <asm/cacheflush.h>

#include "registers.h"
#include "cam_os_wrapper.h"

#define MSYS_ERROR(fmt, args...)    printk(KERN_ERR"MSYS: " fmt, ## args)
#define MSYS_WARN(fmt, args...)     printk(KERN_WARNING"MSYS: " fmt, ## args)
#define MSYS_WARNING(fmt, args...)     printk(KERN_WARNING"MSYS: " fmt, ## args)



extern struct miscdevice  sys_dev;
typedef unsigned int volatile ulv;
typedef unsigned long int volatile ullv;

/******************************************
 * Function prototypes and Global variables
 ******************************************/
//int TEST_SolidBitsComparison(ulv *pSrc, ulv *pDest, unsigned int nCount);


/******************************************
 * Extern
 ******************************************/

/******************************************
 * Functions
 ******************************************/

static int TEST_Memwrite(ulv * pDest, unsigned int nCount)
{
    register unsigned int val = 0xA5A4B5B4;
    ulv *p2 = NULL;
    unsigned int    nTest, i;
    for (nTest = 0; nTest < 10; nTest++)
    {
        p2 = (ulv *) pDest;
        for (i = 0; i < nCount; i++)
            *p2++ = val;
    }
    return nTest;
}

static int TEST_Memread(ulv * pSrc, unsigned int nCount)
{
    register unsigned int val;
    ulv *p1 = NULL;
    unsigned int    nTest, i;
    for (nTest = 0; nTest < 10; nTest++)
    {
        p1 = (ulv *) pSrc;
        for (i = 0; i < nCount; i++)
            val = *p1++;
    }
    return nTest;
}

static int TEST_Memcpy_mips(ulv * pSrc, ulv * pDest, unsigned int nCount)
{
    int nTest = 0;
//  for (nTest = 0; nTest < 10; nTest++)
//      memcpy_MIPS((void*)pDest, (void*)pSrc, nCount*sizeof(unsigned int));
    return nTest;
}

static int TEST_Memcpy(ulv * pSrc, ulv * pDest, unsigned int nCount)
{
    int nTest;
    for (nTest = 0; nTest < 10; nTest++)
        memcpy((void*)pDest, (void*)pSrc, nCount*sizeof(unsigned int));
    return nTest;
}

static int TEST_MemBandWidth_long(ulv * pSrc, ulv * pDest, unsigned int nCount)
{
    ullv *p1 = NULL;
    ullv *p2 = NULL;
    unsigned int    i;
    unsigned int    nTest;

    for (nTest = 0; nTest < 10; nTest++)
    {
        p1 = (ullv *) pSrc;
        p2 = (ullv *) pDest;

        for (i = 0; i < nCount; i++)
            *p2++ = *p1++;
    }

    return nTest;
}

int TEST_MemBandWidth(ulv * pSrc, ulv * pDest, unsigned int nCount)
{
    ulv *p1 = NULL;
    ulv *p2 = NULL;
    unsigned int    i;
    unsigned int    nTest;

    for (nTest = 0; nTest < 10; nTest++)
    {
        p1 = (ulv *) pSrc;
        p2 = (ulv *) pDest;

        for (i = 0; i < nCount; i++)
            *p2++ = *p1++;
    }

    return nTest;
}


int TEST_MemBandWidthRW(ulv * pSrc, ulv * pDest, unsigned int nCount, unsigned int step_size)
{
    ulv *p1 = NULL;
    ulv *p2 = NULL;
    //unsigned int  i;
    unsigned int    nTest;
    int Count;

    for (nTest = 0; nTest < 10 * step_size; nTest++)
    {
        p1 = (ulv *) pSrc;
        p2 = (ulv *) pDest;
        Count = nCount / step_size;
        //memcpy((void*)p2, (void*)p1, nCount*4);
        while(Count--)
        {
            *p2 = *p1;
            p2 += step_size;
            p1 += step_size;
        }
    }

    return nTest;
}

static int TEST_MemBandWidthR(ulv * pSrc, ulv * pDest, unsigned int nCount, unsigned int step_size)
{
    ulv *p1 = NULL;
    ulv *p2 = NULL;
    //unsigned int  i;
    unsigned int    nTest;
    int Count;

    for (nTest = 0; nTest < 10 * step_size; nTest++)
    {
        p1 = (ulv *) pSrc;
        p2 = (ulv *) pDest;
        Count = nCount / step_size;
        //memcpy((void*)p2, (void*)p1, nCount*4);
        while(Count--)
        {
            *p2 = *p1;
            p1 += step_size;
        }
    }

    return nTest;
}

static int TEST_MemBandWidthW(ulv * pSrc, ulv * pDest, unsigned int nCount, unsigned int step_size)
{
    ulv *p1 = NULL;
    ulv *p2 = NULL;
    //unsigned int  i;
    unsigned int    nTest;
    int Count;

    for (nTest = 0; nTest < 10 * step_size; nTest++)
    {
        p1 = (ulv *) pSrc;
        p2 = (ulv *) pDest;
        Count = nCount / step_size;
        //memcpy((void*)p2, (void*)p1, nCount*4);
        while(Count--)
        {
            *p2 = *p1;
            p2 += step_size;
        }
    }

    return nTest;
}



void msys_bench_memory(unsigned int uMemSize)
{
    unsigned int    nLoop = 0;
    unsigned int    nAllocBytes;
    unsigned int    nBufSize;
    unsigned int    nCount;
    unsigned int    PAGE_MASK1 = 0x0FFF;
    void *pBuf = NULL;
    volatile void *pAlignedBuf = NULL;
    volatile unsigned int *pSrc;
    volatile unsigned int *pDest;
    unsigned int bus_addr;
    struct timespec tss, tse;
    int             nDelay;
    int             nTestCount = 0;
    int             nSize;
    int i = 0;

    nBufSize    = (unsigned int) (uMemSize << 20);
    nAllocBytes = nBufSize + 4096;

    MSYS_WARNING("\n>>>> sys_memory_benchmark0\n");
    pBuf=dma_alloc_coherent(sys_dev.this_device, PAGE_ALIGN(nAllocBytes), &bus_addr, GFP_KERNEL);

    if(pBuf==NULL)
    {
        MSYS_ERROR("error while allocating DMA buffer for benchmark...\n");
        return;
    }

    MSYS_WARNING(" Allocated %d bytes at 0x%08x\n", nAllocBytes, (unsigned int) pBuf);

    if ((unsigned int) pBuf % 4096) {
        pAlignedBuf = (void volatile *) (((unsigned int) pBuf + 4096)
                & PAGE_MASK1);
        MSYS_WARNING(" Aligned at 0x%08x\n", (unsigned int) pAlignedBuf);
    } else {
        pAlignedBuf = pBuf;
    }

    /* Show information */
    nCount = (nBufSize / 2) / sizeof(unsigned int);

    pSrc = (ulv *) pAlignedBuf;
    pDest = (ulv *) ((unsigned int) pAlignedBuf + (nBufSize / 2));

    MSYS_WARNING(" Read from : %p\n", pSrc);
    MSYS_WARNING(" Write to  : %p\n", pDest);

    nSize = nCount * sizeof(unsigned int);

    MSYS_WARNING(" Size : %x\n", nSize);

    MSYS_WARNING("\nMemory read/write test\n");
    nLoop = 0;

    MSYS_WARNING("\n(1) Memory read/write test through 32-bit pointer access\n");

    tss = CURRENT_TIME;
    nTestCount = TEST_MemBandWidth(pSrc, pDest, nCount);
    tse = CURRENT_TIME;

    nDelay = (tse.tv_sec - tss.tv_sec) * 1000 + tse.tv_nsec / 1000000
            - tss.tv_nsec / 1000000;

    MSYS_WARNING("Read/Write %3d: %d times, %8d, %4d msec => %6d KB/sec\n",
            nLoop, nTestCount, nSize, nDelay,
            (((nSize * nTestCount) / 1024) * 1000) / nDelay);

    MSYS_WARNING("\n(2) Memory read/write test through 32-bit pointer access\n");

    tss = CURRENT_TIME;
    nTestCount = TEST_MemBandWidth_long(pSrc, pDest, nCount);
    tse = CURRENT_TIME;

    nDelay = (tse.tv_sec - tss.tv_sec) * 1000 + tse.tv_nsec / 1000000
            - tss.tv_nsec / 1000000;

    MSYS_WARNING("Read/Write %3d: %d times, %8d bytes, %4d msec => %6d KB/sec\n",
            nLoop, nTestCount, nSize, nDelay,
            (((nSize * nTestCount) / 1024) * 1000) / nDelay);

    MSYS_WARNING("\n(3) Memory read/write test through memcpy()\n");

    tss = CURRENT_TIME;
    nTestCount = TEST_Memcpy(pSrc, pDest, nCount);
    tse = CURRENT_TIME;

    nDelay = (tse.tv_sec - tss.tv_sec) * 1000 + tse.tv_nsec / 1000000
            - tss.tv_nsec / 1000000;

    MSYS_WARNING("Read/Write %3d: %d times, %8d bytes, %4d msec => %6d KB/sec\n",
            nLoop, nTestCount, nSize, nDelay,
            (((nSize * nTestCount) / 1024) * 1000) / nDelay);

    MSYS_WARNING("\n(4) Memory read/write test through memcpy(prefetch version)\n");

    tss = CURRENT_TIME;
    nTestCount = TEST_Memcpy_mips(pSrc, pDest, nCount);
    tse = CURRENT_TIME;

    nDelay = (tse.tv_sec - tss.tv_sec) * 1000 + tse.tv_nsec / 1000000
            - tss.tv_nsec / 1000000;

    MSYS_WARNING("Read/Write %3d: %d times, %8d bytes, %4d msec => %6d KB/sec\n",
            nLoop, nTestCount, nSize, nDelay,
            (((nSize * nTestCount) / 1024) * 1000) / nDelay);

    MSYS_WARNING("\n(5) Memory read test\n");

    tss = CURRENT_TIME;
    nTestCount = TEST_Memread(pSrc, nCount);
    tse = CURRENT_TIME;

    nDelay = (tse.tv_sec - tss.tv_sec) * 1000 + tse.tv_nsec / 1000000
            - tss.tv_nsec / 1000000;

    MSYS_WARNING("Read  %3d: %d times, %8d bytes, %4d msec => %6d KB/sec\n", nLoop,
            nTestCount, nSize, nDelay,
            (((nSize * nTestCount) / 1024) * 1000) / nDelay);

    MSYS_WARNING("\n(6) Memory write test\n");

    tss = CURRENT_TIME;
    nTestCount = TEST_Memwrite(pDest, nCount);
    tse = CURRENT_TIME;

    nDelay = (tse.tv_sec - tss.tv_sec) * 1000 + tse.tv_nsec / 1000000
            - tss.tv_nsec / 1000000;

    MSYS_WARNING("Write %3d: %d times, %8d bytes, %4d msec => %6d KB/sec\n", nLoop,
            nTestCount, nSize, nDelay,
            (((nSize * nTestCount) / 1024) * 1000) / nDelay);

    //=============================

    MSYS_WARNING("\n(7) Memory read/write test\n");

    for (i = 1; i < 513; i = i << 1)
    {
        tss = CURRENT_TIME;

        nTestCount = TEST_MemBandWidthRW(pSrc, pDest, nCount, i);

        tse = CURRENT_TIME;

        nDelay = (tse.tv_sec - tss.tv_sec) * 1000 + tse.tv_nsec / 1000000
                - tss.tv_nsec / 1000000;

        MSYS_WARNING("Read/Write  %8d bytes, skip %4d bytes %4d msec => %6d KB/sec\n",
                nSize, i * 4, nDelay,
                ((((nSize / i) * nTestCount) / 1024) * 1000) / nDelay);
    }

    MSYS_WARNING("\n(8) Memory read test\n");

    for (i = 1; i < 513; i = i << 1)
    {
        tss = CURRENT_TIME;

        nTestCount = TEST_MemBandWidthR(pSrc, pDest, nCount, i);

        tse = CURRENT_TIME;

        nDelay = (tse.tv_sec - tss.tv_sec) * 1000 + tse.tv_nsec / 1000000
                - tss.tv_nsec / 1000000;

        MSYS_WARNING("Read  %8d bytes, skip %4d bytes %4d msec => %6d KB/sec\n",
                nSize, i * 4, nDelay,
                ((((nSize / i) * nTestCount) / 1024) * 1000) / nDelay);
    }

    MSYS_WARNING("\n(9) Memory write test\n");

    for (i = 1; i < 513; i = i << 1)
    {
        tss = CURRENT_TIME;

        nTestCount = TEST_MemBandWidthW(pSrc, pDest, nCount, i);

        tse = CURRENT_TIME;

        nDelay = (tse.tv_sec - tss.tv_sec) * 1000 + tse.tv_nsec / 1000000
                - tss.tv_nsec / 1000000;

        MSYS_WARNING("Write  %8d bytes, skip %4d bytes %4d msec => %6d KB/sec\n",
                nSize, i * 4, nDelay,
                ((((nSize / i) * nTestCount) / 1024) * 1000) / nDelay);
    }


    MSYS_WARNING("\n<<<< sys_memory_benchmark0\n");
    dma_free_coherent(sys_dev.this_device, nAllocBytes,pBuf,bus_addr);
    //  munlock((void *) pBuf, nAllocBytes);
    //  free((void *) pBuf);
}
