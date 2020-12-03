/*
* ms_msys_perf_test.c- Sigmastar
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

extern struct miscdevice  sys_dev;

#if 1
#define dmac_map_area			__glue(_CACHE,_dma_map_area)
#define dmac_unmap_area 		__glue(_CACHE,_dma_unmap_area)
extern void dmac_map_area(const void *, size_t, int);
extern void dmac_unmap_area(const void *, size_t, int);
#else
#define dmac_map_area(a, b, c) {}
#define dmac_unmap_area(a, b, c) {}
#endif


#include <linux/time.h>

#define STR_IMI         "IMI"
#define STR_MIU         "MIU"
#define STR_CACHE       "CACHE"
#define STR_CPUINFO     "CPUINFO"
#define STR_ALL         "ALL"

////////////////////////////////////////////
#define STR_CPUINFO_CA9         "ARMv7 CA9"
#define STR_CPUINFO_CA7         "ARMv7 CA7"
#define STR_CPUINFO_CA53        "ARMv8 CA53"
#define STR_CPUINFO_NULL        "CPU_NULL"
#define CPU_PART_CA7            0xC07
#define CPU_PART_CA9            0xC09
#define CPU_PART_CA53           0xD03
#define CPU_PART_NULL           0x000

#define IMI_ADDR_INVALID        0xFFFFFFFF
#define IMI_SIZE_INVALID        0xFFFFFFFF



static char* _perf_fileBuf = NULL;
static char* _perf_fileptr_write = NULL;
static char* _perf_fileptr_read = NULL;
static struct file *_fp = NULL;
#define FILEBUF_SIZE        8192

//////////////////////////////////////////////////////////////////
/// performance test file buffer
//////////////////////////////////////////////////////////////////
#define PERF_BUF_ALLOC() \
({ \
    _perf_fileptr_read = _perf_fileptr_write = _perf_fileBuf = kzalloc(FILEBUF_SIZE, GFP_KERNEL); \
    (_perf_fileBuf); \
})

#define PERF_BUF_FREE() \
{ \
    if (_perf_fileBuf) \
    { \
        kfree(_perf_fileBuf); \
        _perf_fileptr_read = _perf_fileptr_write = _perf_fileBuf = NULL; \
    } \
}


#if 0

#define PERF_BUF_SYNC() \
{ \
    if ((_perf_fileptr_read) && (_perf_fileptr_read != _perf_fileptr_write) && (_fp)) \
    { \
        _fp->f_op->write(_fp, _perf_fileptr_read, strlen(_perf_fileptr_read), &_fp->f_pos); \
        _perf_fileptr_read = _perf_fileptr_write; \
    } \
}

#else

#define PERF_BUF_SYNC() \
{ \
    if ((_perf_fileptr_read) && (_perf_fileptr_read != _perf_fileptr_write) && (_fp)) \
    { \
        vfs_write(_fp, _perf_fileptr_read, strlen(_perf_fileptr_read), &_fp->f_pos); \
        _perf_fileptr_read = _perf_fileptr_write; \
    } \
}

#endif

#define PERF_BUF_RESET()        { _perf_fileptr_read = _perf_fileptr_write = _perf_fileBuf; memset(_perf_fileBuf, 0, FILEBUF_SIZE); }

#define PERF_BUF_PUT(format, ...) \
{ \
    if (_perf_fileptr_write) \
    { \
        _perf_fileptr_write += sprintf(_perf_fileptr_write, format, ##__VA_ARGS__); \
    } \
}

//////////////////////////////////////////////////////////////////
/// performance test command maker
//////////////////////////////////////////////////////////////////
#define PERF_TEST_CMD_MAKE_CPUINFO(cmdBuf) \
                sprintf((cmdBuf), "%s", STR_CPUINFO)
#define PERF_TEST_CMD_MAKE_CACHE(cmdBuf, size, loop) \
                sprintf((cmdBuf), "%s %d %d", STR_CACHE, (size), (loop))
#define PERF_TEST_CMD_MAKE_MIU(cmdBuf, cache, size, loop, scheme) \
                sprintf((cmdBuf), "%s %d %s %d %d %d %d", STR_MIU, (cache), STR_MIU, (cache), (loop), (size), (scheme))
#define PERF_TEST_CMD_MAKE_IMI(cmdBuf, cache, size, loop, scheme) \
                sprintf((cmdBuf), "%s %d %s %d %d %d %d", STR_IMI, (cache), STR_IMI, (cache), (loop), (size), (scheme))

//////////////////////////////////////////////////////////////////
/// performance test file
//////////////////////////////////////////////////////////////////
#define PERF_TEST_FILENAME_MAKE(prefix, pattern, postfix) \
{ \
    strcat((prefix), "_"); \
    strcat((prefix), (pattern)); \
    strcat((prefix), (postfix)); \
}

#define PERF_TEST_FILE_OPEN(filename) \
({ \
    _fp = filp_open((filename), O_CREAT|O_RDWR|O_TRUNC, 0777); \
    (_fp); \
})

#define PERF_TEST_FILE_CLOSE() \
{ \
    if (_fp) \
    { \
        filp_close(_fp, NULL); \
        _fp = NULL; \
    } \
}

static struct timespec time_start;
static void _time_start(void)
{
    getnstimeofday(&time_start);
}

static unsigned long _time_end(void)
{
    unsigned long delta;
    struct timespec ct;

    getnstimeofday(&ct);
    // delta = ct.tv_nsec + ((time_start.tv_sec != ct.tv_sec) ? NSEC_PER_SEC : 0) - time_start.tv_nsec;
    // delta = (ct.tv_sec - time_start.tv_sec)*1000000 + (ct.tv_nsec - time_start.tv_nsec)/1000;
    delta = (ct.tv_sec - time_start.tv_sec)*1000 + (ct.tv_nsec - time_start.tv_nsec)/1000000;
    // printk("[%s][%d] (start, end, delta) = (%d, %d) (%d, %d) %d\n", __FUNCTION__, __LINE__, (int)time_start.tv_sec, (int)time_start.tv_nsec, (int)ct.tv_sec, (int)ct.tv_nsec, (int)delta);
    return delta;
}

static unsigned long _time_end_ns(void)
{
    unsigned long delta;
    struct timespec ct;

    getnstimeofday(&ct);
    delta = (ct.tv_sec - time_start.tv_sec)*1000000000 + (ct.tv_nsec - time_start.tv_nsec);
    // printk("[%s][%d] (start, end, delta) = (%d, %d) (%d, %d) %d\n", __FUNCTION__, __LINE__, (int)time_start.tv_sec, (int)time_start.tv_nsec, (int)ct.tv_sec, (int)ct.tv_nsec, (int)delta);
    return delta;
}

#if 0
static void hex_dump(char* pBuf, int size)
{
    int i;

    for (i = 0; i < size; i++)
    {
        if ((i&0xF) == 0x0)
        {
            printk("\n");
        }
        printk("%02x ", pBuf[i]);
    }
    printk("\n");
}
#endif


#define ALLOC_RETRY_COUNT 16
static int perf_test_malloc(void** ppBuf, dma_addr_t* pPhys_addr, int size, int bCache)
{
    void* pBuf = NULL;
    dma_addr_t phys_addr = 0;
    int i;

    for (i = 0; i < ALLOC_RETRY_COUNT; i++)
    {
        if (bCache)
        {
            pBuf = (void*)__get_free_pages(GFP_KERNEL, get_order(size));
            phys_addr = __pa(pBuf);
        }
        else
        {
            // pBuf = (void*)dma_alloc_coherent(sys_dev.this_device, PAGE_ALIGN(size), &phys_addr, GFP_DMA);
            pBuf = (void*)dma_alloc_coherent(sys_dev.this_device, PAGE_ALIGN(size), &phys_addr, GFP_KERNEL);
        }
        if (pBuf)
        {
            break;
        }
        sysctl_compaction_handler(NULL, 1, NULL, NULL, NULL);
        msleep(1000);
    }
    if (NULL == pBuf)
    {
        return 0;
    }
    *ppBuf = pBuf;
    *pPhys_addr = phys_addr;
    return 1;
}

static void perf_test_mfree(void* pBuf, dma_addr_t phys_addr, int size, int bCache)
{
    if (NULL == pBuf)
    {
        return;
    }
    if (bCache)
    {
        free_pages((long)pBuf, get_order(size));
    }
    else
    {
        pBuf = (void*)dma_alloc_coherent(sys_dev.this_device, PAGE_ALIGN(size), &phys_addr, GFP_DMA);
        dma_free_coherent(sys_dev.this_device, PAGE_ALIGN(size), pBuf, phys_addr);
    }
}

typedef struct
{
    int bCache;
    int bIMI;
    void* pBuf;
    dma_addr_t phys_addr;
} bufInfo;

#ifndef CONFIG_THUMB2_KERNEL
void _bench_neon_memcpy(void*, void*, unsigned int);
void _bench_memcpy(void*, void*, unsigned int);
#endif
typedef enum {
    PERF_TEST_SCHEME_CRT_MEMCPY,
    PERF_TEST_SCHEME_BENCH_MEMCPY,
    PERF_TEST_SCHEME_BENCH_NEON_MEMCPY,
    PERF_TEST_SCHEME_ASSIGN_WRITE_ONLY,
    PERF_TEST_SCHEME_ASSIGN_READ_ONLY,
} perf_test_scheme_t;

static ssize_t perf_test_memcpy(const char* buf, size_t n)
{
    int i;
    unsigned long duration = 0;
    int bitrate;
    unsigned char dst[16];
    unsigned char src[16];
    int iteration, size;
    unsigned int scheme = 0; // 0: CRT memcpy, 1: _bench_memcpy, 2: _bench_neon_memcpy

    bufInfo bufInfoDst = {0} , bufInfoSrc = {0};

    sscanf(buf, "%s %d %s %d %d %d %d", dst, &bufInfoDst.bCache, src, &bufInfoSrc.bCache, &iteration, &size, &scheme);
    printk("INPUT: %s %d %s %d %d %d %d\n", dst, bufInfoDst.bCache, src, bufInfoSrc.bCache, iteration, size, scheme);

    if (0 == strcasecmp(dst, STR_IMI))
    {
        bufInfoDst.bIMI = 1;
        if ((IMI_ADDR_PHYS_1 == IMI_ADDR_INVALID) || (IMI_SIZE_1 == IMI_SIZE_INVALID))
        {
            printk("[%s][%d] invalid IMI address, size 0x%08x, 0x%08x\n", __FUNCTION__, __LINE__, IMI_ADDR_PHYS_1, IMI_SIZE_1);
            goto jump_fail;
        }
        if (bufInfoDst.bCache)
        {
            bufInfoDst.pBuf = (void*)ioremap_cache(IMI_ADDR_PHYS_1, IMI_SIZE_1);
        }
        else
        {
            bufInfoDst.pBuf = (void*)ioremap_nocache(IMI_ADDR_PHYS_1, IMI_SIZE_1);
        }
    }
    else
    {
        bufInfoDst.bIMI = 0;
        perf_test_malloc(&(bufInfoDst.pBuf), &(bufInfoDst.phys_addr), size, bufInfoDst.bCache);
    }
    if (0 == strcasecmp(src, STR_IMI))
    {
        bufInfoSrc.bIMI = 1;
        if ((IMI_ADDR_PHYS_2 == IMI_ADDR_INVALID) || (IMI_SIZE_2 == IMI_SIZE_INVALID))
        {
            printk("[%s][%d] invalid IMI address, size 0x%08x, 0x%08x\n", __FUNCTION__, __LINE__, IMI_ADDR_PHYS_2, IMI_SIZE_2);
            goto jump_fail;
        }
        if (bufInfoSrc.bCache)
        {
            bufInfoSrc.pBuf = (void*)ioremap_cache(IMI_ADDR_PHYS_2, IMI_SIZE_2);
        }
        else
        {
            bufInfoSrc.pBuf = (void*)ioremap_nocache(IMI_ADDR_PHYS_2, IMI_SIZE_2);
        }
    }
    else
    {
        bufInfoSrc.bIMI = 0;
        perf_test_malloc(&(bufInfoSrc.pBuf), &(bufInfoSrc.phys_addr), size, bufInfoSrc.bCache);
    }
    if ((NULL == bufInfoSrc.pBuf) || (NULL == bufInfoDst.pBuf))
    {
        printk("[%s][%d] alloc/ioremap fail\n", __FUNCTION__, __LINE__);
        goto jump_fail;
    }
    printk("[%s][%d] ==============================================\n", __FUNCTION__, __LINE__);
    printk("[%s][%d] dst (tag, cache, addr) = (%s, %d, 0x%08x)\n", __FUNCTION__, __LINE__, dst, bufInfoDst.bCache, (int)bufInfoDst.pBuf);
    printk("[%s][%d] src (tag, cache, addr) = (%s, %d, 0x%08x)\n", __FUNCTION__, __LINE__, src, bufInfoSrc.bCache, (int)bufInfoSrc.pBuf);
    _time_start();
    for (i= 0; i< iteration; i++)
    {
        switch(scheme) {
            case PERF_TEST_SCHEME_CRT_MEMCPY:
                memcpy((void*)bufInfoDst.pBuf, (void*)bufInfoSrc.pBuf, size);
                break;
            case PERF_TEST_SCHEME_BENCH_MEMCPY:
#ifndef CONFIG_THUMB2_KERNEL
                _bench_memcpy((void*)bufInfoDst.pBuf, (void*)bufInfoSrc.pBuf, size);
#else
                printk("CONFIG_THUMB2_KERNEL defined, NEON memcpy not supported!");
#endif
                break;
            case PERF_TEST_SCHEME_BENCH_NEON_MEMCPY:
#ifndef CONFIG_THUMB2_KERNEL
                _bench_neon_memcpy((void*)bufInfoDst.pBuf, (void*)bufInfoSrc.pBuf, size);
#else
                printk("CONFIG_THUMB2_KERNEL defined, NEON memcpy not supported!");
#endif
                break;
            case PERF_TEST_SCHEME_ASSIGN_WRITE_ONLY:
                {
                    register unsigned int j = 0;
                    unsigned int v = 0x55;
                    for(j=0;j<size/4;j++) {
                        ((unsigned int*)bufInfoDst.pBuf)[j] = v;
                    }
                }
                break;
            case PERF_TEST_SCHEME_ASSIGN_READ_ONLY:
                {
                    register unsigned int j = 0;
                    unsigned int v = 0x55;
                    for(j=0;j<size/4;j++) {
                        v  = ((unsigned int*)bufInfoDst.pBuf)[j] ;
                    }
                }
                break;
            default:
                printk("[%s][%d] invalid scheme(%d)\n", __FUNCTION__, __LINE__, scheme);
                goto jump_fail;
                break;
        }
    }
    duration = _time_end();
    iteration /= 1000;
    printk("[%s][%d] (iteration, iteration*size) = (%d, %d) time = %d\n", __FUNCTION__, __LINE__, iteration, iteration*size, (int)duration);
    bitrate = ((iteration*size)/duration);
    printk("[%s][%d] bit rate = %d\n", __FUNCTION__, __LINE__, bitrate);
    PERF_BUF_PUT("%d,", bitrate);
jump_fail:
    if (bufInfoDst.pBuf)
    {
        if (bufInfoDst.bIMI)
        {
            iounmap(bufInfoDst.pBuf);
        }
        else
        {
            perf_test_mfree(bufInfoDst.pBuf, bufInfoDst.phys_addr, size, bufInfoDst.bCache);
        }
    }
    if (bufInfoSrc.pBuf)
    {
        if (bufInfoSrc.bIMI)
        {
            iounmap(bufInfoSrc.pBuf);
        }
        else
        {
            perf_test_mfree(bufInfoSrc.pBuf, bufInfoSrc.phys_addr, size, bufInfoSrc.bCache);
        }
    }
    return n;
}

void* g_pBuf_Dst = NULL;
void* g_pBuf_Src = NULL;
#define PTEST_BUFFER_COUNT 16

ssize_t perf_test_memcpy_cacheable(const char* buf, size_t n)
{
    int i, j;
    unsigned long long duration = 0;
    int bitrate;
    int dst_offset=0;
    int temp_size;

    void* pBuf_Dst = NULL;
    void* pBuf_Src = NULL;

    char token[16];
    int batomic=0;
    unsigned long long size=0;
    unsigned long long iteration=0;        
    sscanf(buf, "%s %lld %lld %d %d", token, &size, &iteration, &batomic, &dst_offset);

    temp_size = size+PTEST_BUFFER_COUNT*dst_offset;

    if(!g_pBuf_Dst)
        g_pBuf_Dst = (void*)__get_free_pages(GFP_KERNEL, get_order(temp_size));
    if(!g_pBuf_Src)
        g_pBuf_Src = (void*)__get_free_pages(GFP_KERNEL, get_order(temp_size));

    printk("g_pBuf_Dst:%p g_pBuf_Src:%p size:0x%x\n",g_pBuf_Dst, g_pBuf_Src, temp_size);

    printk("pBuf_Dst  , pBuf_Src  , size, bitrate\n");
    for(j=0;j<PTEST_BUFFER_COUNT;j++)
    {
        pBuf_Dst = g_pBuf_Dst+j*dst_offset;
        pBuf_Src = g_pBuf_Src+(PTEST_BUFFER_COUNT-j)*dst_offset;
        memcpy(pBuf_Dst, pBuf_Src, size);
        if(batomic){
            local_irq_disable();
            preempt_disable();
        }

        _time_start();

        for (i=0; i< iteration; i++)
        {
            memcpy(pBuf_Dst, pBuf_Src, size);
        }
        duration = _time_end_ns();
        if(batomic){
            local_irq_enable();
            preempt_enable();
        }
        bitrate = ((iteration*size)/1024/1024*1000000000/duration);
        printk("0x%8x, 0x%08x, %lld, %d\n",(unsigned int)pBuf_Dst,(unsigned int)pBuf_Src,  size, bitrate);
    }
    printk("\n");

    if(g_pBuf_Dst)
    {
        free_pages((long)g_pBuf_Dst, get_order(temp_size));
        g_pBuf_Dst=NULL;
    }
    if(g_pBuf_Src)
    {
        free_pages((long)g_pBuf_Src, get_order(temp_size));
        g_pBuf_Dst=NULL;
    }

    return n;
}

#define INNER_CLEAN(buf, size)          dmac_map_area((buf), (size), DMA_TO_DEVICE)
#define INNER_INV(buf, size)            dmac_map_area((buf), (size), DMA_FROM_DEVICE)
// cache 32768 100000 // cache size loop
static ssize_t perf_test_cache(const char* buf, size_t n)
{
    unsigned char t1[16];
    int s32BufSize = 0;
    int max_loop = 10000;
    unsigned long int delta1 = 0;
    unsigned long int delta2 = 0;
    int i;
    void* pBuf;
    dma_addr_t phys_addr;

    sscanf(buf, "%s %d %d", t1, &s32BufSize, &max_loop);
    perf_test_malloc(&pBuf, &phys_addr, s32BufSize, 1);
    if (NULL == pBuf)
    {
        printk("[%s][%d] fail allocating memory with size %d)\n", __FUNCTION__, __LINE__, s32BufSize);
        return n;
    }
    PERF_BUF_PUT("%d,", s32BufSize);
    // invalidate cache without memset
    delta1 = delta2 = 0;
    for (i = 0; i < max_loop; i++)
    {
        _time_start();
        INNER_INV(pBuf, s32BufSize); // dmac_map_area(pBuf, s32BufSize, DMA_FROM_DEVICE);
        delta1 += _time_end_ns();
        _time_start();
        outer_inv_range(phys_addr, phys_addr + s32BufSize);
        delta2 += _time_end_ns();
    }
    printk("[%s][%d] Invalidate without dirty : (delta1, delta2) = (%d, %d)\n", __FUNCTION__, __LINE__, (int)(delta1/max_loop), (int)(delta2/max_loop));
    PERF_BUF_PUT("%d,%d,", (int)(delta1/max_loop), (int)(delta2/max_loop));

    // clean cache without memset
    delta1 = delta2 = 0;
    for (i = 0; i < max_loop; i++)
    {
        _time_start();
        INNER_CLEAN(pBuf, s32BufSize); // dmac_map_area(pBuf, s32BufSize, DMA_TO_DEVICE);
        delta1 += _time_end_ns();
        _time_start();
        outer_clean_range(phys_addr, phys_addr + s32BufSize);
        delta2 += _time_end_ns();
    }
    printk("[%s][%d] clean without dirty : (delta1, delta2) = (%d, %d)\n", __FUNCTION__, __LINE__, (int)(delta1/max_loop), (int)(delta2/max_loop));
    PERF_BUF_PUT("%d,%d,", (int)(delta1/max_loop), (int)(delta2/max_loop));

    // printk("[%s][%d] ==========================================================\n", __FUNCTION__, __LINE__);
    // invalidate cache with memset
    delta1 = delta2 = 0;
    for (i = 0; i < max_loop; i++)
    {
        memset(pBuf, 0x00, s32BufSize);
        _time_start();
        INNER_INV(pBuf, s32BufSize); // dmac_map_area(pBuf, s32BufSize, DMA_FROM_DEVICE);
        delta1 += _time_end_ns();
        _time_start();
        outer_inv_range(phys_addr, phys_addr + s32BufSize);
        delta2 += _time_end_ns();
    }
    printk("[%s][%d] Invalidate with dirty : (delta1, delta2) = (%d, %d)\n", __FUNCTION__, __LINE__, (int)(delta1/max_loop), (int)(delta2/max_loop));
    PERF_BUF_PUT("%d,%d,", (int)(delta1/max_loop), (int)(delta2/max_loop));

    // clean cache with memset
    delta1 = delta2 = 0;
    for (i = 0; i < max_loop; i++)
    {
        memset(pBuf, 0x00, s32BufSize);
        _time_start();
        INNER_CLEAN(pBuf, s32BufSize); // dmac_map_area(pBuf, s32BufSize, DMA_TO_DEVICE);
        delta1 += _time_end_ns();
        _time_start();
        outer_clean_range(phys_addr, phys_addr + s32BufSize);
        delta2 += _time_end_ns();
    }
    printk("[%s][%d] clean with dirty : (delta1, delta2) = (%d, %d)\n", __FUNCTION__, __LINE__, (int)(delta1/max_loop), (int)(delta2/max_loop));
    PERF_BUF_PUT("%d,%d,", (int)(delta1/max_loop), (int)(delta2/max_loop));
    PERF_BUF_PUT("\n");
    perf_test_mfree(pBuf, phys_addr, s32BufSize, 1);
    return n;
}

// MIDR
#define ARM_MIDR_READ() \
    ({ \
        int val; \
        asm volatile("mrc p15, 0, r0, c0, c0, 0\n"); \
        asm volatile("str r0, %[reg]\n" : [reg]"=m"(val)); \
        val; \
    })
#define ARM_MIDR_WRITE(val) \
    { \
        asm volatile("ldr r0, %[reg]\n" : :[reg]"m"((val))); \
        asm volatile("mcr p15, 0, r0, c0, c0, 0\n"); \
    }

// TLBTR
#define ARM_TLBTR_READ() \
    ({ \
        int val; \
        asm volatile("mrc p15, 0, r0, c0, c0, 3\n"); \
        asm volatile("str r0, %[reg]\n" : [reg]"=m"(val)); \
        val; \
    })

// MPIDR
#define ARM_MPIDR_READ() \
    ({ \
        int val; \
        asm volatile("mrc p15, 0, r0, c0, c0, 5\n"); \
        asm volatile("str r0, %[reg]\n" : [reg]"=m"(val)); \
        val; \
    })

// CSSELR
#define ARM_CSSELR_READ() \
    ({ \
        int val; \
        asm volatile("mrc p15, 2, r0, c0, c0, 0\n"); \
        asm volatile("str r0, %[reg]\n" : [reg]"=m"(val)); \
        val; \
    })
#define ARM_CSSELR_WRITE(val) \
    { \
        asm volatile("ldr r0, %[reg]\n" : :[reg]"m"((val))); \
        asm volatile("mcr p15, 2, r0, c0, c0, 0\n"); \
    }

// CCSIDR_L1_I
#define ARM_CCSIDR_L1_I_READ() \
    ({ \
        int val; \
        val = ARM_CSSELR_READ(); \
        val &= 0xFFFFFFF0; \
        val |= 0x00000001; \
        ARM_CSSELR_WRITE(val); \
        asm volatile("mrc p15, 1, r0, c0, c0, 0\n"); \
        asm volatile("str r0, %[reg]\n" : [reg]"=m"(val)); \
        val; \
    })

// CCSIDR_L1_D
#define ARM_CCSIDR_L1_D_READ() \
    ({ \
        int val; \
        val = ARM_CSSELR_READ(); \
        val &= 0xFFFFFFF0; \
        val |= 0x00000000; \
        ARM_CSSELR_WRITE(val); \
        asm volatile("mrc p15, 1, r0, c0, c0, 0\n"); \
        asm volatile("str r0, %[reg]\n" : [reg]"=m"(val)); \
        val; \
    })

// CCSIDR_L2
#define ARM_CCSIDR_L2_READ() \
    ({ \
        int val; \
        val = ARM_CSSELR_READ(); \
        val &= 0xFFFFFFF0; \
        val |= 0x00000002; \
        ARM_CSSELR_WRITE(val); \
        asm volatile("mrc p15, 1, r0, c0, c0, 0\n"); \
        asm volatile("str r0, %[reg]\n" : [reg]"=m"(val)); \
        val; \
    })

// CLIDR
#define ARM_CLIDR_READ() \
    ({ \
        int val; \
        asm volatile("mrc p15, 1, r0, c0, c0, 1\n"); \
        asm volatile("str r0, %[reg]\n" : [reg]"=m"(val)); \
        val; \
    })


#define ARM_CTR_READ() \
    ({ \
        int val; \
        asm volatile("mrc p15, 0, r0, c0, c0, 1\n"); \
        asm volatile("str r0, %[reg]\n" : [reg]"=m"(val)); \
        val; \
    })

#define CCSIDR_DECODE(CCSIDR, WT, WB, RA, WA, NumSet, Associate, LineSize) \
    { \
        (WT) = ((CCSIDR) >> 31) & 0x00000001; \
        (WB) = ((CCSIDR) >> 30) & 0x00000001; \
        (RA) = ((CCSIDR) >> 29) & 0x00000001; \
        (WA) = ((CCSIDR) >> 28) & 0x00000001; \
        (NumSet) = ((CCSIDR) >> 13) & 0x00007FFF; \
        (Associate) = ((CCSIDR) >>  3) & 0x000003FF; \
        (LineSize) = ((CCSIDR) >>  0) & 0x00000007; \
    }
#define CACHE_SIZE(NumSet, Associate, LineSize)         (((((NumSet)+1) * (LineSize))*((Associate)+1))>>5)
#define CCSIDR_DUMP(str, WT, WB, RA, WA, NumSet, Associate, LineSize) \
    { \
        printk("[%s][%d] %s\n", __FUNCTION__, __LINE__, (str)); \
        PERF_BUF_PUT("%s\n", (str)); \
        printk("[%s][%d]         (WT, WB, RA, WA) = (%d, %d, %d, %d)\n", __FUNCTION__, __LINE__, (WT), (WB), (RA), (WA)); \
        PERF_BUF_PUT(",Write through, %d\n", (WT)); \
        PERF_BUF_PUT(",Write back, %d\n", (WB)); \
        PERF_BUF_PUT(",Read allocate, %d\n", (RA)); \
        PERF_BUF_PUT(",Write allocate, %d\n", (WA)); \
        printk("[%s][%d]         %d NumSet\n", __FUNCTION__, __LINE__, ((NumSet)+1)); \
        PERF_BUF_PUT(",NumSet, %d\n", ((NumSet)+1)); \
        printk("[%s][%d]         %d ways\n", __FUNCTION__, __LINE__, ((Associate)+1)); \
        PERF_BUF_PUT(",Ways, %d\n", ((Associate)+1)); \
        printk("[%s][%d]         line size = %d bytes\n", __FUNCTION__, __LINE__, ((LineSize)<<5) ); \
        PERF_BUF_PUT(",Line size (Bytes), %d\n", ((LineSize)<<5) ); \
        printk("[%s][%d]         cache size = %d KB\n", __FUNCTION__, __LINE__, CACHE_SIZE((NumSet), (Associate), (LineSize))); \
        PERF_BUF_PUT(",Cache size (KBytes), %d\n", CACHE_SIZE((NumSet), (Associate), (LineSize))); \
    }

u32 msys_l2x0_size = 0;
u32 msys_l2x0_ways = 0;
u32 msys_l2x0_linesize = 32; // constant for PL310

static ssize_t perf_test_cpuinfo(const char* buf, size_t n)
{
    unsigned int MIDR = 0;
    unsigned int MPIDR = 0;
    unsigned int CCSIDR_L1_I = 0;
    unsigned int CCSIDR_L1_D = 0;
    unsigned int CCSIDR_L2 = 0;
    unsigned int CTR = 0;
    unsigned int cpuPart = 0;
    char* strCPU = STR_CPUINFO_NULL;
    int WT, WB, RA, WA, NumSet, Associate, LineSize;
    int cpu;
    int cpuNR = 0;
    unsigned int CLIDR = ARM_CLIDR_READ();

    for_each_online_cpu(cpu)
    {
        cpuNR++;
    }
    MIDR = ARM_MIDR_READ();
    MPIDR = ARM_MPIDR_READ();
    CCSIDR_L1_I = ARM_CCSIDR_L1_I_READ();
    CCSIDR_L1_D = ARM_CCSIDR_L1_D_READ();
    CCSIDR_L2 = 0;
    if (CLIDR & 0x00000038)
        CCSIDR_L2 = ARM_CCSIDR_L2_READ();
    // CCSIDR_L2 = ARM_CCSIDR_L2_READ();
    CTR = ARM_CTR_READ();
    cpuPart = ((MIDR >> 4) & 0x00000FFF);
    switch (cpuPart)
    {
    case CPU_PART_CA7:
        strCPU = STR_CPUINFO_CA7;
        break;
    case CPU_PART_CA9:
        strCPU = STR_CPUINFO_CA9;
        break;
    case CPU_PART_CA53:
        strCPU = STR_CPUINFO_CA53;
        break;
    default:
        break;
    }
    // printk("[%s][%d] (MIDR, MPIDR) = (0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__, MIDR, MPIDR);
    printk("[%s][%d] (CPU type, core) = (%s, %d)\n", __FUNCTION__, __LINE__, strCPU, cpuNR);
    PERF_BUF_PUT("CPU,%s\n", strCPU);
    PERF_BUF_PUT("Core number,%d\n", cpuNR);
    CCSIDR_DECODE(CCSIDR_L1_I, WT, WB, RA, WA, NumSet, Associate, LineSize);
    CCSIDR_DUMP("L1 instruction cache information", WT, WB, RA, WA, NumSet, Associate, LineSize);
    CCSIDR_DECODE(CCSIDR_L1_D, WT, WB, RA, WA, NumSet, Associate, LineSize);
    CCSIDR_DUMP("L1 data cache information", WT, WB, RA, WA, NumSet, Associate, LineSize);
    if (CPU_PART_CA9 != cpuPart)
    {
        CCSIDR_DECODE(CCSIDR_L2, WT, WB, RA, WA, NumSet, Associate, LineSize);
        CCSIDR_DUMP("L2 cache information", WT, WB, RA, WA, NumSet, Associate, LineSize);
    }
    else
    {
        printk("[%s][%d] %s\n", __FUNCTION__, __LINE__, "L2 cache information");
        PERF_BUF_PUT("%s\n", "L2 cache information");
        printk("[%s][%d]         %d ways\n", __FUNCTION__, __LINE__, msys_l2x0_ways);
        PERF_BUF_PUT(",Ways,%d\n", msys_l2x0_ways);
        printk("[%s][%d]         line size = %d bytes\n", __FUNCTION__, __LINE__, msys_l2x0_linesize);
        PERF_BUF_PUT(",Line size (Bytes), %d\n", msys_l2x0_linesize);
        printk("[%s][%d]         cache size = %d KB\n", __FUNCTION__, __LINE__, (msys_l2x0_size >> 10));
        PERF_BUF_PUT(",Cache size(KBytes), %d\n", (msys_l2x0_size >> 10));
    }

    {
        unsigned int CLIDR = ARM_CLIDR_READ();
        printk("[%s][%d]         CLIDR = 0x%08x\n", __FUNCTION__, __LINE__, CLIDR);
    }


    return n;
}


// all /vendor/2222.txt
// cpuinfo
// MIU 1 MIU 1 10000 65536 0
// IMI 1 IMI 1 10000 65536 0
// cache 32768 100000

char cmdBuf[128] = { 0 };

static ssize_t perf_test_all(const char* buf, size_t n)
{
    // char buf[100];
    char temp[8];
    char filename[128];
    char filename_org[128];
    mm_segment_t old_fs;

    if (NULL == PERF_BUF_ALLOC())
    {
        printk("[%s][%d] kzalloc fail with size %d\n", __FUNCTION__, __LINE__, FILEBUF_SIZE);
        return n;
    }
    sscanf(buf, "%s %s", temp, filename_org);
#if 1
    // cpu information
    strcpy(filename, filename_org);
    PERF_TEST_FILENAME_MAKE(filename, STR_CPUINFO, ".csv");
    if (PERF_TEST_FILE_OPEN(filename))
    {
        old_fs = get_fs();
        set_fs(get_ds());
        PERF_TEST_CMD_MAKE_CPUINFO(cmdBuf);
        perf_test_cpuinfo(cmdBuf, strlen(cmdBuf));
        PERF_BUF_SYNC();
        PERF_BUF_RESET();
        PERF_TEST_FILE_CLOSE(); // filp_close(fp, NULL);
        set_fs(old_fs);
    }
    // cache testing
    strcpy(filename, filename_org);
    PERF_TEST_FILENAME_MAKE(filename, STR_CACHE, ".csv");
    if (PERF_TEST_FILE_OPEN(filename))
    {
        int size[] = { 4096, 8192, 16394, 32768, 65536, 131072 };
        int i = 0;
        int iteration = 10000;

        old_fs = get_fs();
        set_fs(get_ds());
        PERF_BUF_PUT("Size,Bytes\n");
        PERF_BUF_PUT("Iteration,%d\n", iteration);
        PERF_BUF_PUT("Time,Nanoseconds\n");
        PERF_BUF_PUT("Size,L1(Inv),L2(Inv),L1(Clean),L2(Clean),L1(Inv dirty),L2(Inv dirty),L1(Clean dirty),L2(Clean dirty)\n");
        for (i = 0 ; i < sizeof(size)/sizeof(size[0]); i++)
        {
            PERF_TEST_CMD_MAKE_CACHE(cmdBuf, size[i], iteration);
            perf_test_cache(cmdBuf, strlen(cmdBuf));
        }
        PERF_BUF_SYNC();
        PERF_BUF_RESET();
        PERF_TEST_FILE_CLOSE(); // filp_close(fp, NULL);
        set_fs(old_fs);
    }
    // MIU testing
    strcpy(filename, filename_org);
    PERF_TEST_FILENAME_MAKE(filename, STR_MIU, ".csv");
    if (PERF_TEST_FILE_OPEN(filename))
    {
        // int size[] = { 32768, 65536, 131072, 262144, 524288, 1048576, 2097152 };
        int size[] = { 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152 };
        int iter[] = { 1000000, 1000000, 100000, 10000, 10000, 10000, 10000, 10000, 10000, 10000};
        int i = 0;
        int iteration = 10000;

        old_fs = get_fs();
        set_fs(get_ds());
        PERF_BUF_PUT("Size,Bytes\n");
        PERF_BUF_PUT("Iteration,%d\n", iteration);
        PERF_BUF_PUT("Bit rate,MBytes/Sec\n\n\n\n");
        // non-cache
        PERF_BUF_PUT("%s noncache\n", STR_MIU);
        PERF_BUF_PUT("Size,");
        for (i = 0 ; i < sizeof(size)/sizeof(size[0]); i++)
        {
            PERF_BUF_PUT("%d,", size[i]);
        }
        PERF_BUF_PUT("\n");
        PERF_BUF_PUT("Bit rate,");
        for (i = 0 ; i < sizeof(size)/sizeof(size[0]); i++)
        {
            PERF_TEST_CMD_MAKE_MIU(cmdBuf, 0, size[i], iteration, 0);
            perf_test_memcpy(cmdBuf, strlen(cmdBuf));
        }
        PERF_BUF_PUT("\n");
        PERF_BUF_PUT("\n");
        // cache
        PERF_BUF_PUT("%s cache\n", STR_MIU);
        PERF_BUF_PUT("Size,");
        for (i = 0 ; i < sizeof(size)/sizeof(size[0]); i++)
        {
            PERF_BUF_PUT("%d,", size[i]);
        }
        PERF_BUF_PUT("\n");
        PERF_BUF_PUT("Bit rate,");
        for (i = 0 ; i < sizeof(size)/sizeof(size[0]); i++)
        {
            PERF_TEST_CMD_MAKE_MIU(cmdBuf, 1, size[i], iter[i], 0);
            perf_test_memcpy(cmdBuf, strlen(cmdBuf));
        }
        PERF_BUF_PUT("\n");
        PERF_BUF_SYNC();
        PERF_BUF_RESET();
        PERF_TEST_FILE_CLOSE(); // filp_close(fp, NULL);
        set_fs(old_fs);
    }
    // IMI testing
    strcpy(filename, filename_org);
    PERF_TEST_FILENAME_MAKE(filename, STR_IMI, ".csv");
    if (PERF_TEST_FILE_OPEN(filename))
    {
        int size[] = { 32768}; // , 65536, 131072, 262144, 524288, 1048576, 2097152 };
        int i = 0;
        int iteration = 10000;

        old_fs = get_fs();
        set_fs(get_ds());

        PERF_BUF_PUT("Size,Bytes\n");
        PERF_BUF_PUT("Iteration,%d\n", iteration);
        PERF_BUF_PUT("Bit rate,MBytes/Sec\n\n\n\n");
        // non-cache
        PERF_BUF_PUT("%s noncache\n", STR_IMI);
        PERF_BUF_PUT("Size,");
        for (i = 0 ; i < sizeof(size)/sizeof(size[0]); i++)
        {
            PERF_BUF_PUT("%d,", size[i]);
        }
        PERF_BUF_PUT("\n");
        PERF_BUF_PUT("Bit rate,");
        for (i = 0 ; i < sizeof(size)/sizeof(size[0]); i++)
        {
            PERF_TEST_CMD_MAKE_IMI(cmdBuf, 0, size[i], 10000, 0);
            perf_test_memcpy(cmdBuf, strlen(cmdBuf));
        }
        PERF_BUF_PUT("\n");
        PERF_BUF_SYNC();
        PERF_BUF_RESET();
        PERF_TEST_FILE_CLOSE(); // filp_close(fp, NULL);
        set_fs(old_fs);
    }
#endif
    PERF_BUF_FREE();
    return n;
}

static ssize_t perf_test_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    int n;

    n = sprintf(str, "[%s][%d] this is a perf_test_show\n", __FUNCTION__, __LINE__);
    return n;
}

static ssize_t perf_test_entry(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    unsigned char token[16];
    sscanf(buf, "%s", token);
    if (0 == strcasecmp(token, STR_CPUINFO))
    {
        return perf_test_cpuinfo(buf, n);
    }
    else if (0 == strcasecmp(token, STR_IMI))
    {
        return perf_test_memcpy(buf, n);
    }
    else if (0 == strcasecmp(token, STR_MIU))
    {
        return perf_test_memcpy(buf, n);
    }
    else if (0 == strcasecmp(token, STR_CACHE))
    {
        return perf_test_cache(buf, n);
    }
    else if (0 == strcasecmp(token, STR_ALL))
    {
        return perf_test_all(buf, n);
    }
    else if (0 == strcasecmp(token, "tc"))
    {
        return perf_test_memcpy_cacheable(buf, n);
    }

    return n;
}
DEVICE_ATTR(perf_test, 0644, perf_test_show, perf_test_entry);

int msys_perf_test_init(void)
{
    device_create_file(sys_dev.this_device, &dev_attr_perf_test);
    return 0;
}
device_initcall(msys_perf_test_init);
