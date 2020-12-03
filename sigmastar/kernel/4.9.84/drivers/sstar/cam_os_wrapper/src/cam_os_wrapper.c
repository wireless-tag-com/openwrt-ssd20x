/*
* cam_os_wrapper.c- Sigmastar
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


///////////////////////////////////////////////////////////////////////////////
/// @file      cam_os_wrapper.c
/// @brief     Cam OS Wrapper Source File for
///            1. RTK OS
///            2. Linux User Space
///            3. Linux Kernel Space
///////////////////////////////////////////////////////////////////////////////

#if defined(__KERNEL__)
#define CAM_OS_LINUX_KERNEL
#endif

#ifdef CAM_OS_RTK
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "time.h"
#include "sys_sys.h"
#include "sys_sys_math64.h"
#include "sys_sys_tools.h"
#include "sys_sys_core.h"
#include "sys_MsWrapper_cus_os_flag.h"
#include "sys_MsWrapper_cus_os_sem.h"
#include "sys_MsWrapper_cus_os_util.h"
#include "sys_MsWrapper_cus_os_timer.h"
#include "sys_sys_isw_uart.h"
#include "sys_rtk_hp.h"
#include "hal_drv_util.h"
#include "sys_time.h"
#include "sys_arm_arch_timer.h"
#include "drv_bus_axi.h"
#include "drv_int_ctrl_pub_api.h"
#include "cam_os_wrapper.h"
#include "cam_os_util.h"
#include "cam_os_util_list.h"
#include "cam_os_util_bitmap.h"

#define OS_NAME "RTK"

#define CAM_OS_THREAD_STACKSIZE_DEFAULT         8192

typedef void *CamOsThreadEntry_t(void *);

typedef struct
{
    MsTaskId_e eHandleObj;
    CamOsThreadEntry_t *pfnEntry;
    void *pArg;
    Ms_Flag_t tExitFlag;
    void *pStack;
} CamOsThreadHandleRtk_t, *pCamOsThreadHandleRtk;

typedef struct
{
    u32 nInited;
    Ms_Mutex_t tMutex;
} CamOsMutexRtk_t, *pCamOsMutexRtk;

typedef struct
{
    u32 nInited;
    Ms_DynSemaphor_t Tsem;
} CamOsTsemRtk_t, *pCamOsTsemRtk;

typedef struct
{
    u32 nInited;
    u32 nReadCount;
    Ms_Mutex_t tRMutex;
    Ms_DynSemaphor_t WTsem;
} CamOsRwsemRtk_t, *pCamOsRwsemRtk;

typedef struct
{
    u32 nInited;
    Ms_DynSemaphor_t Tsem;
} CamOsTcondRtk_t, *pCamOsTcondRtk;

typedef struct
{
    unsigned long nFlags;
} CamOsSpinlockRtk_t, *pCamOsSpinlockRtk;

typedef struct
{
    MsTimerId_e eTimerID;
    void (*pfnCallback)(unsigned long);
    void *pDataPtr;
} CamOsTimerRtk_t, *pCamOsTimerRtk;

typedef struct
{
    u8 nPoolID;
    u32 nObjSize;
} CamOsMemCacheRtk_t, *pCamOsMemCacheRtk;

static Ms_Mutex_t _gtSelfInitLock = {0};
static Ms_Mutex_t _gtMemLock = {0};

static u32 _gTimeOfDayOffsetSec = 0;
static u32 _gTimeOfDayOffsetNanoSec = 0;

_Static_assert(sizeof(CamOsMutex_t) >= sizeof(Ms_Flag_t) + 4, "CamOsMutex_t size define not enough!");
_Static_assert(sizeof(CamOsTsem_t) >= sizeof(CamOsTsemRtk_t), "CamOsTsem_t size define not enough!");
_Static_assert(sizeof(CamOsRwsem_t) >= sizeof(CamOsRwsemRtk_t), "CamOsRwsem_t size define not enough!");
_Static_assert(sizeof(CamOsTcond_t) >= sizeof(CamOsTcondRtk_t), "CamOsTcond_t size define not enough!");
_Static_assert(sizeof(CamOsSpinlock_t) >= sizeof(CamOsSpinlockRtk_t), "CamOsSpinlock_t size define not enough!");
_Static_assert(sizeof(CamOsTimer_t) >= sizeof(CamOsTimerRtk_t), "CamOsTimer_t size define not enough!");
_Static_assert(sizeof(CamOsMemCache_t) >= sizeof(CamOsMemCacheRtk_t), "CamOsMemCache_t size define not enough!");

CAM_OS_DECLARE_BITMAP(aThreadStopBitmap, RTK_MAX_TASKS);
static u8 nThreadStopBitmapInited=0;

#elif defined(CAM_OS_LINUX_USER)
#define _GNU_SOURCE
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/time.h>
#ifndef NO_MDRV_MSYS
#include <mdrv_msys_io.h>
#include <mdrv_msys_io_st.h>
#include "mdrv_verchk.h"
#endif
#include <sys/mman.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <stdarg.h>
#include "time.h"
#include "cam_os_wrapper.h"
#include "cam_os_util.h"
#include "cam_os_util_list.h"
#include "cam_os_util_bitmap.h"

#define OS_NAME "LINUX USER"

typedef void *CamOsThreadEntry_t(void *);

typedef struct
{
    pthread_t tThreadHandle;
    CamOsThreadEntry_t *pfnEntry;
    void *pArg;
    u32 nPid;
    CamOsTsem_t tEntryIn;
} CamOsThreadHandleLinuxUser_t, *pCamOsThreadHandleLinuxUser;

typedef struct
{
    u32 nInited;
    pthread_mutex_t tMutex;
} CamOsMutexLU_t, *pCamOsMutexLU;

typedef struct
{
    u32 nInited;
    sem_t tSem;
} CamOsTsemLU_t, *pCamOsTsemLU;

typedef struct
{
    u32 nInited;
    pthread_rwlock_t tRwsem;
} CamOsRwsemLU_t, *pCamOsRwsemLU;

typedef struct
{
    u32 nInited;
    pthread_mutex_t tMutex;
    pthread_cond_t tCondition;
} CamOsTcondLU_t, *pCamOsTcondLU;

typedef struct
{
    u32 nInited;
    pthread_spinlock_t tLock;
} CamOsSpinlockLU_t, *pCamOsSpinlockLU;

typedef struct
{
    timer_t tTimerID;
} CamOsTimerLU_t, *pCamOsTimerLU;

typedef struct
{
    u32 nIdrSize;
    void *pEntryPtr;
} CamOsIdrLU_t, *pCamOsIdrLU;

static pthread_mutex_t _gtSelfInitLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _gtMemLock = PTHREAD_MUTEX_INITIALIZER;

_Static_assert(sizeof(CamOsMutex_t) >= sizeof(pthread_mutex_t) + 4, "CamOsMutex_t size define not enough! %d");
_Static_assert(sizeof(CamOsTsem_t) >= sizeof(CamOsTsemLU_t), "CamOsTsem_t size define not enough!");
_Static_assert(sizeof(CamOsRwsem_t) >= sizeof(CamOsRwsemLU_t), "CamOsRwsem_t size define not enough!");
_Static_assert(sizeof(CamOsTcond_t) >= sizeof(CamOsTcondLU_t), "CamOsTcond_t size define not enough!");
_Static_assert(sizeof(CamOsSpinlock_t) >= sizeof(CamOsSpinlockLU_t), "CamOsSpinlock_t size define not enough!");
_Static_assert(sizeof(CamOsTimer_t) >= sizeof(CamOsTimerLU_t), "CamOsTimer_t size define not enough!");
_Static_assert(sizeof(CamOsTimespec_t) == sizeof(struct timespec), "CamOsTimespec_t size define error!");
_Static_assert(sizeof(CamOsIdr_t) >= sizeof(CamOsIdrLU_t), "CamOsIdr_t size define not enough!");

#elif defined(CAM_OS_LINUX_KERNEL)
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/math64.h>
#include <linux/mm.h>
#include <linux/wait.h>
#include <linux/vmalloc.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/scatterlist.h>
#include <linux/slab_def.h>
#include <linux/idr.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h>
#include <asm/page.h>
#include <ms_msys.h>
#include <ms_platform.h>
#include "cam_os_wrapper.h"
#include "cam_os_util.h"
#include "cam_os_util_list.h"
#include "cam_os_util_bitmap.h"

#define OS_NAME "LINUX KERNEL"

#define CAM_OS_THREAD_STACKSIZE_DEFAULT         8192
#define KMALLOC_THRESHOLD_SIZE                  (PAGE_SIZE >> 1)
#define LOG_MAX_TRACE_LEN                       256
#define LINUX_KERNEL_MAX_IRQ                    256

typedef struct
{
    u32 nInited;
    struct mutex tMutex;
} CamOsMutexLK_t, *pCamOsMutexLK;

typedef struct
{
    u32 nInited;
    struct semaphore tSem;
} CamOsTsemLK_t, *pCamOsTsemLK;

typedef struct
{
    u32 nInited;
    struct rw_semaphore tRwsem;
} CamOsRwsemLK_t, *pCamOsRwsemLK;

typedef struct
{
    u32 nInited;
    struct completion tCompletion;
} CamOsTcondLK_t, *pCamOsTcondLK;

typedef struct
{
    u32 nInited;
    spinlock_t tLock;
    unsigned long nFlags;
} CamOsSpinlockLK_t, *pCamOsSpinlockLK;

typedef struct
{
    struct timer_list tTimerID;
} CamOsTimerLK_t, *pCamOsTimerLK;

typedef struct
{
    struct kmem_cache *ptKmemCache;
} CamOsMemCacheLK_t, *pCamOsMemCacheLK;

typedef struct
{
    struct idr tIdr;
} CamOsIdrLK_t, *pCamOsIdrLK;

typedef s32 CamOsThreadEntry_t(void *);

static CamOsIrqHandler CamOsIrqHandlerList[LINUX_KERNEL_MAX_IRQ] = {0};

static irqreturn_t CamOsIrqCommonHandler(u32 nIrq, void *pDevId)
{
    if (nIrq < LINUX_KERNEL_MAX_IRQ && CamOsIrqHandlerList[nIrq] != NULL)
    {
        (CamOsIrqHandlerList[nIrq])(nIrq, pDevId);
    }

    return IRQ_HANDLED;
}

extern int msys_find_dmem_by_phys(unsigned long long phys, MSYS_DMEM_INFO* pdmem);

static DEFINE_MUTEX(_gtSelfInitLock);
static DEFINE_MUTEX(_gtMemLock);

_Static_assert(sizeof(CamOsMutex_t) >= sizeof(struct mutex) + 4, "CamOsMutex_t size define not enough! %d");
_Static_assert(sizeof(CamOsTsem_t) >= sizeof(CamOsTsemLK_t), "CamOsTsem_t size define not enough!");
_Static_assert(sizeof(CamOsRwsem_t) >= sizeof(CamOsRwsemLK_t), "CamOsRwsem_t size define not enough!");
_Static_assert(sizeof(CamOsTcond_t) >= sizeof(CamOsTcondLK_t), "CamOsTcond_t size define not enough!");
_Static_assert(sizeof(CamOsSpinlock_t) >= sizeof(CamOsSpinlockLK_t), "CamOsSpinlock_t size define not enough!");
_Static_assert(sizeof(CamOsTimespec_t) == sizeof(struct timespec), "CamOsTimespec_t size define error!");
_Static_assert(sizeof(CamOsTimer_t) >= sizeof(CamOsTimerLK_t), "CamOsTimer_t size define not enough!");
_Static_assert(sizeof(CamOsMemCache_t) >= sizeof(CamOsMemCacheLK_t), "CamOsMemCache_t size define not enough!");
_Static_assert(sizeof(CamOsAtomic_t) == sizeof(atomic_t), "CamOsAtomic_t size define not enough!");
_Static_assert(sizeof(CamOsIdr_t) >= sizeof(CamOsIdrLK_t), "CamOsIdr_t size define not enough!");

#endif

// common macro define
#define RIU_BASE_ADDR           0x1F000000
#define RIU_MEM_SIZE_OFFSET     0x2025A4
#define RIU_CHIP_ID_OFFSET      0x003C00

#define ASSIGN_POINTER_VALUE(a, b) if((a))*(a)=(b)

#define INIT_MAGIC_NUM          0x55AA5AA5

#define CAM_OS_MAX_LIST_LENGTH_BITS 20

#define CAM_OS_WARN_TRACE_LR
#ifdef CAM_OS_WARN_TRACE_LR
#define CAM_OS_WARN(x)      CamOsPrintf("%s "x", LR:0x%08X\n", __FUNCTION__, __builtin_return_address(0))
#else
#define CAM_OS_WARN(x)      CamOsPrintf("%s "x"\n", __FUNCTION__)
#endif

typedef struct MemoryList_t
{
    struct CamOsListHead_t tList;
    void *pPhysPtr;
    void *pVirtPtr;
    void *pMemifoPtr;
    char *szName;
    u32  nSize;
} MemoryList_t;

static MemoryList_t _gtMemList;

static s32 _gnDmemDbgListInited = 0;

char *CamOsVersion(void)
{
    return CAM_OS_WRAPPER_VERSION;
}

void CamOsPrintf(const char *szFmt, ...)
{
#ifdef CAM_OS_RTK
    va_list tArgs;
    unsigned int u32MsgLen = 0;
    char nLineStr[256] = {0};

    va_start(tArgs, szFmt);
    u32MsgLen = vsnprintf(nLineStr, sizeof(nLineStr), szFmt, tArgs);
    if (u32MsgLen >= sizeof(nLineStr))
    {
        nLineStr[sizeof(nLineStr)-1] = '\0';  /* even the 'vsnprintf' commond will do it */
        nLineStr[sizeof(nLineStr)-2] = '\n';
        nLineStr[sizeof(nLineStr)-3] = '.';
        nLineStr[sizeof(nLineStr)-4] = '.';
        nLineStr[sizeof(nLineStr)-5] = '.';
    }
    send_msg(nLineStr);
    va_end(tArgs);
#elif defined(CAM_OS_LINUX_USER)
    va_list tArgs;

    va_start(tArgs, szFmt);
    vfprintf(stderr, szFmt, tArgs);
    va_end(tArgs);
#elif defined(CAM_OS_LINUX_KERNEL)
#if defined(CONFIG_MS_MSYS_LOG)
    va_list tArgs;
    unsigned int u32MsgLen = 0;
    char szLogStr[LOG_MAX_TRACE_LEN]={'a'};
    va_start(tArgs, szFmt);
    u32MsgLen = vsnprintf(szLogStr, LOG_MAX_TRACE_LEN, szFmt, tArgs);
    va_end(tArgs);
    if (u32MsgLen >= LOG_MAX_TRACE_LEN)
    {
        szLogStr[LOG_MAX_TRACE_LEN-1] = '\0';  /* even the 'vsnprintf' commond will do it */
        szLogStr[LOG_MAX_TRACE_LEN-2] = '\n';
        szLogStr[LOG_MAX_TRACE_LEN-3] = '.';
        szLogStr[LOG_MAX_TRACE_LEN-4] = '.';
        szLogStr[LOG_MAX_TRACE_LEN-5] = '.';
    }
    msys_prints(szLogStr, u32MsgLen);
#else
    va_list tArgs;
    va_start(tArgs, szFmt);
    vprintk(szFmt, tArgs);
    va_end(tArgs);
#endif
#endif
}

void CamOsPrintString(const char *szStr)
{
#ifdef CAM_OS_RTK
    send_msg((char *)szStr);
#elif defined(CAM_OS_LINUX_USER)
    printf(szStr);
#elif defined(CAM_OS_LINUX_KERNEL)
#if defined(CONFIG_MS_MSYS_LOG)
    msys_prints(szStr, strlen(szStr));
#else
    printk(szStr);
#endif
#endif
}

#ifdef CAM_OS_RTK
static char* _CamOsAdvance(char* pBuf)
{

    char* pNewBuf = pBuf;

    /* Skip over nonwhite space */
    while((*pNewBuf != ' ')  && (*pNewBuf != '\t') &&
            (*pNewBuf != '\n') && (*pNewBuf != '\0'))
    {
        pNewBuf++;
    }

    /* Skip white space */
    while((*pNewBuf == ' ')  || (*pNewBuf == '\t') ||
            (*pNewBuf == '\n') || (*pNewBuf == '\0'))
    {
        pNewBuf++;
    }

    return pNewBuf;
}

static s32 _CamOsVsscanf(char* szBuf, char* szFmt, va_list tArgp)
{
    char*    pFmt;
    char*    pBuf;
    char*    pnSval;
    u32*     pnU32Val;
    s32*     pnS32Val;
    u64*     pnU64Val;
    s64*     pnS64Val;
    double*  pdbDval;
    float*   pfFval;
    s32      nCount = 0;

    pBuf = szBuf;

    for(pFmt = szFmt; *pFmt; pFmt++)
    {
        if(*pFmt == '%')
        {
            pFmt++;
            if(strncmp(pFmt, "u", 1) == 0)
            {
                pnU32Val = va_arg(tArgp, u32 *);
                sscanf(pBuf, "%u", pnU32Val);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if(strncmp(pFmt, "d", 1) == 0)
            {
                pnS32Val = va_arg(tArgp, s32 *);
                sscanf(pBuf, "%d", pnS32Val);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if(strncmp(pFmt, "llu", 3) == 0)
            {
                pnU64Val = va_arg(tArgp, u64 *);
                sscanf(pBuf, "%llu", pnU64Val);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if(strncmp(pFmt, "lld", 3) == 0)
            {
                pnS64Val = va_arg(tArgp, s64 *);
                sscanf(pBuf, "%lld", pnS64Val);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if(strncmp(pFmt, "f", 1) == 0)
            {
                pfFval = va_arg(tArgp, float *);
                sscanf(pBuf, "%f", pfFval);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if(strncmp(pFmt, "lf", 2) == 0)
            {
                pdbDval = va_arg(tArgp, double *);
                sscanf(pBuf, "%lf", pdbDval);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else if(strncmp(pFmt, "s", 1) == 0)
            {
                pnSval = va_arg(tArgp, char *);
                sscanf(pBuf, "%s", pnSval);
                pBuf = _CamOsAdvance(pBuf);
                nCount++;
            }
            else
            {
                CamOsPrintf("%s error: unsupported format (\%%s)\n", __FUNCTION__, pFmt);
            }
        }
    }

    return nCount;
}

static s32 _CamOsGetString(char* szBuf, s32 nMaxLen,  s32 nEcho)
{
    s32 nLen;
    static char ch = '\0';

    nLen = 0;
    while(1)
    {
        szBuf[nLen] = get_char();

        // To ignore one for (\r,\n) or (\n, \r) pair
        if((szBuf[nLen] == '\n' && ch == '\r') || (szBuf[nLen] == '\r' && ch == '\n'))
        {
            ch = '\0';
            continue;
        }
        ch = szBuf[nLen];
        if(ch == '\n' || ch == '\r')
        {
            if(nEcho)
                CamOsPrintf("\n");
            break;
        }
        if(nLen < (nMaxLen - 1))
        {
            if(ch == '\b')  /* Backspace? */
            {
                if(nLen <= 0)
                    CamOsPrintf("\007");
                else
                {
                    CamOsPrintf("\b \b");
                    nLen --;
                }
                continue;
            }
            nLen++;
        }
        if(nEcho)
            CamOsPrintf("%c", ch);
    }
    szBuf[nLen] = '\0';
    return nLen;
}

static s32 _CamOsVfscanf(const char *szFmt, va_list tArgp)
{
    s32 nCount;
    char szCommandBuf[128];

    _CamOsGetString(szCommandBuf, sizeof(szCommandBuf), 1);

    nCount = _CamOsVsscanf(szCommandBuf, (char *)szFmt, tArgp);
    return nCount;
}
#endif

s32 CamOsScanf(const char *szFmt, ...)
{
#ifdef CAM_OS_RTK
    s32 nCount = 0;
    va_list tArgp;

    va_start(tArgp, szFmt);
    nCount = _CamOsVfscanf(szFmt, tArgp);
    va_end(tArgp);
    return nCount;
#elif defined(CAM_OS_LINUX_USER)
    s32 nCount = 0;
    va_list tArgp;

    va_start(tArgp, szFmt);
    nCount = vfscanf(stdin, szFmt, tArgp);
    va_end(tArgp);
    return nCount;
#elif defined(CAM_OS_LINUX_KERNEL)
    return 0;
#endif
}

s32 CamOsGetChar(void)
{
#ifdef CAM_OS_RTK
    s32 Ret;
    Ret = get_char();
    CamOsPrintf("\n");
    return Ret;
#elif defined(CAM_OS_LINUX_USER)
    return getchar();
#elif defined(CAM_OS_LINUX_KERNEL)
    return 0;
#endif
}

s32 CamOsSnprintf(char *szBuf, u32 nSize, const char *szFmt, ...)
{
    va_list tArgs;
    s32 i;

    va_start(tArgs, szFmt);
    i = vsnprintf(szBuf, nSize, szFmt, tArgs);
    va_end(tArgs);

    return i;
}

void CamOsHexdump(char *szBuf, u32 nSize)
{
    int i, j;
    int cx = 0;
    char szLine[80] = {0};

    CamOsPrintf("\nOffset(h)  00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F\n"
                "-----------------------------------------------------------\n");

    if ((u32)szBuf % 16)
    {
        cx = 0;
        cx += snprintf(szLine + cx, sizeof(szLine) - cx, "%08X  ", (u32)szBuf & 0xFFFFFFF0);

        for (i = 0; i < (u32)szBuf % 16; i++)
        {
            cx += snprintf(szLine + cx, sizeof(szLine) - cx, "   ");
            szLine[i + 62] = ' ';
            if (i % 8 == 0)
            {
                cx += snprintf(szLine + cx, sizeof(szLine) - cx, " ");
            }
        }
    }

    for (i = 0; i < nSize; i++)
    {
        if ((i + (u32)szBuf) % 16 == 0)
        {
            cx = 0;
            cx += snprintf(szLine + cx, sizeof(szLine) - cx, "%08X  ", (u32)szBuf + i);
        }
        if ((i + (u32)szBuf) % 8 == 0)
        {
            cx += snprintf(szLine + cx, sizeof(szLine) - cx, " ");
        }

        cx += snprintf(szLine + cx, sizeof(szLine) - cx, "%02X ", szBuf[i]);

        if (((unsigned char*)szBuf)[i] >= ' ' && ((unsigned char*)szBuf)[i] <= '~')
        {
            szLine[(i + (u32)szBuf) % 16 + 62] = ((unsigned char*)szBuf)[i];
        }
        else
        {
            szLine[(i + (u32)szBuf) % 16 + 62] = '.';
        }

        if ((i + (u32)szBuf) % 16 == 15)
        {
            szLine[59] = ' ';
            szLine[60] = ' ';
            szLine[61] = '|';
            szLine[78] = '|';
            szLine[79] = 0;
            CamOsPrintf("%s\n", szLine);
        }
        else if (i == nSize-1)
        {
            for (j = ((i + (u32)szBuf) + 1) % 16; j < 16; j++)
            {
                cx += snprintf(szLine + cx, sizeof(szLine) - cx, "   ");
                szLine[j + 62] = ' ';
            }
            if (((i + (u32)szBuf) + 1) % 16 <= 8)
            {
                cx += snprintf(szLine + cx, sizeof(szLine) - cx, " ");
            }
            szLine[59] = ' ';
            szLine[60] = ' ';
            szLine[61] = '|';
            szLine[78] = '|';
            szLine[79] = 0;
            CamOsPrintf("%s\n", szLine);
        }
    }
    CamOsPrintf("\n");
}

void CamOsMsSleep(u32 nMsec)
{
#ifdef CAM_OS_RTK
    MsSleep(RTK_MS_TO_TICK(nMsec));
#elif defined(CAM_OS_LINUX_USER)
    usleep((useconds_t)nMsec * 1000);
#elif defined(CAM_OS_LINUX_KERNEL)
    msleep(nMsec);
#endif
}

void CamOsUsSleep(u32 nUsec)
{
#ifdef CAM_OS_RTK
    MsSleep(RTK_MS_TO_TICK(nUsec / 1000));
#elif defined(CAM_OS_LINUX_USER)
    usleep((useconds_t)nUsec);
#elif defined(CAM_OS_LINUX_KERNEL)
    usleep_range(nUsec, nUsec + (nUsec>>4));
#endif
}

void CamOsMsDelay(u32 nMsec)
{
#ifdef CAM_OS_RTK
    u64 nTicks = arch_counter_get_cntpct();

    while (((arch_counter_get_cntpct() - nTicks) / 6000) < nMsec) {}
#elif defined(CAM_OS_LINUX_USER)
    CamOsTimespec_t tTvStart = {0}, tTvEnd = {0};
    CamOsGetMonotonicTime(&tTvStart);
    do
    {
        CamOsGetMonotonicTime(&tTvEnd);
    }
    while (CamOsTimeDiff(&tTvStart, &tTvEnd, CAM_OS_TIME_DIFF_MS) < nMsec);
#elif defined(CAM_OS_LINUX_KERNEL)
    mdelay(nMsec);
#endif
}

void CamOsUsDelay(u32 nUsec)
{
#ifdef CAM_OS_RTK
    u64 nTicks = arch_counter_get_cntpct();

    while (((arch_counter_get_cntpct() - nTicks) / 6) < nUsec) {}
#elif defined(CAM_OS_LINUX_USER)
    CamOsTimespec_t tTvStart = {0}, tTvEnd = {0};
    CamOsGetMonotonicTime(&tTvStart);
    do
    {
        CamOsGetMonotonicTime(&tTvEnd);
    }
    while (CamOsTimeDiff(&tTvStart, &tTvEnd, CAM_OS_TIME_DIFF_US) < nUsec);
#elif defined(CAM_OS_LINUX_KERNEL)
    udelay(nUsec);
#endif
}

#ifdef CAM_OS_RTK
static void TimeNormalise(u32 *nSec, s32 *nNanoSec)
{
    while(*nNanoSec >= 1000000000)
    {
        ++(*nSec);
        *nNanoSec -= 1000000000;
    }

    while(*nNanoSec <= 0)
    {
        --(*nSec);
        *nNanoSec += 1000000000;
    }

    return;
}
#endif

void CamOsGetTimeOfDay(CamOsTimespec_t *ptRes)
{
#ifdef CAM_OS_RTK
    u32 nCurrSec = 0;
    u32 nCurrNanoSec = 0;

    if(ptRes)
    {
        CamOsGetMonotonicTime(ptRes);
        nCurrSec = ptRes->nSec + _gTimeOfDayOffsetSec;
        nCurrNanoSec = ptRes->nNanoSec + _gTimeOfDayOffsetNanoSec;
        TimeNormalise(&nCurrSec, (s32 *)&nCurrNanoSec);
        ptRes->nSec = nCurrSec;
        ptRes->nNanoSec = nCurrNanoSec;
    }
#elif defined(CAM_OS_LINUX_USER)
    struct timeval tTV;
    if(ptRes)
    {
        gettimeofday(&tTV, NULL);
        ptRes->nSec = tTV.tv_sec;
        ptRes->nNanoSec = tTV.tv_usec * 1000;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    struct timeval tTv;
    if(ptRes)
    {
        do_gettimeofday(&tTv);
        ptRes->nSec = tTv.tv_sec;
        ptRes->nNanoSec = tTv.tv_usec * 1000;
    }
#endif
}

void CamOsSetTimeOfDay(const CamOsTimespec_t *ptRes)
{
#ifdef CAM_OS_RTK
    CamOsTimespec_t tCurr = {0};
    if(ptRes)
    {
        // Calculate time offset
        CamOsGetMonotonicTime(&tCurr);
        _gTimeOfDayOffsetSec = ptRes->nSec - tCurr.nSec;
        _gTimeOfDayOffsetNanoSec = ptRes->nNanoSec - tCurr.nNanoSec;

        // Save time to RTC
        DrvRtcInit();
        DrvRtcSetSecondCount(ptRes->nSec);
    }
#elif defined(CAM_OS_LINUX_USER)
    struct timeval tTV;
    if(ptRes)
    {
        tTV.tv_sec = ptRes->nSec;
        tTV.tv_usec = ptRes->nNanoSec / 1000;
        settimeofday(&tTV, NULL);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    struct timespec tTs;
    if(ptRes)
    {
        tTs.tv_sec = ptRes->nSec;
        tTs.tv_nsec = ptRes->nNanoSec;
        do_settimeofday(&tTs);
    }
#endif
}

void CamOsGetMonotonicTime(CamOsTimespec_t *ptRes)
{
#ifdef CAM_OS_RTK
    u64 nTicks = arch_counter_get_cntpct();
    ptRes->nSec = nTicks / 6000000;
    ptRes->nNanoSec = (u32)((nTicks % 6000000) * 1000 / 6);
#elif defined(CAM_OS_LINUX_USER)
    clock_gettime(CLOCK_MONOTONIC, (struct timespec *)ptRes);
#elif defined(CAM_OS_LINUX_KERNEL)
    getrawmonotonic((struct timespec *)ptRes);
#endif
}

s64 CamOsTimeDiff(CamOsTimespec_t *ptStart, CamOsTimespec_t *ptEnd, CamOsTimeDiffUnit_e eUnit)
{
    if (ptStart && ptEnd)
    {
        switch (eUnit)
        {
        case CAM_OS_TIME_DIFF_SEC:
            return (s64)ptEnd->nSec - ptStart->nSec;
        case CAM_OS_TIME_DIFF_MS:
            return ((s64)ptEnd->nSec - ptStart->nSec)*1000 + ((s64)ptEnd->nNanoSec - ptStart->nNanoSec)/1000000;
        case CAM_OS_TIME_DIFF_US:
            return ((s64)ptEnd->nSec - ptStart->nSec)*1000000 + ((s64)ptEnd->nNanoSec - ptStart->nNanoSec)/1000;
        case CAM_OS_TIME_DIFF_NS:
            return ((s64)ptEnd->nSec - ptStart->nSec)*1000000000 + ((s64)ptEnd->nNanoSec - ptStart->nNanoSec);
        default:
            return 0;
        }
    }
    else
        return 0;
}

#ifdef CAM_OS_RTK
static void _CamOSThreadEntry(void *pEntryData)
{
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)pEntryData;

    if (ptTaskHandle->pfnEntry)
    {
        ptTaskHandle->pfnEntry(ptTaskHandle->pArg);
    }

    MsFlagSetbits(&ptTaskHandle->tExitFlag, 0x00000001);
}

static void _CamOsThreadEmptyParser(vm_msg_t *ptMessage)
{

}
#elif defined(CAM_OS_LINUX_USER)
static void _CamOSThreadEntry(void *pEntryData)
{
    CamOsThreadHandleLinuxUser_t *ptTaskHandle = (CamOsThreadHandleLinuxUser_t *)pEntryData;

    ptTaskHandle->nPid = CamOsThreadGetID();
    CamOsTsemUp(&ptTaskHandle->tEntryIn);

    if (ptTaskHandle->pfnEntry)
    {
        ptTaskHandle->pfnEntry(ptTaskHandle->pArg);
    }
}
#endif

CamOsRet_e CamOsThreadCreate(CamOsThread *ptThread,
                             CamOsThreadAttrb_t *ptAttrb,
                             void *(*pfnStartRoutine)(void *),
                             void *pArg)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = NULL;
    MsTaskCreateArgs_t tTaskArgs = {0};
    u32 nPrio = 100;
    u32 nStkSz = CAM_OS_THREAD_STACKSIZE_DEFAULT;

    if (!nThreadStopBitmapInited)
    {
        CAM_OS_BITMAP_CLEAR(aThreadStopBitmap);
        nThreadStopBitmapInited = 1;
    }

    if(ptAttrb != NULL)
    {
        if((ptAttrb->nPriority > 0) && (ptAttrb->nPriority < 100))
        {
            nPrio = ptAttrb->nPriority * 2;
        }
        nStkSz = (ptAttrb->nStackSize) ? ptAttrb->nStackSize : CAM_OS_THREAD_STACKSIZE_DEFAULT;
    }

    *ptThread = (void*) - 1;
    do
    {
        if(!(ptTaskHandle = MsCallocateMem(sizeof(CamOsThreadHandleRtk_t))))
        {
            CAM_OS_WARN("alloc handle fail");
            eRet = CAM_OS_ALLOCMEM_FAIL;
            break;
        }

        ptTaskHandle->pfnEntry = pfnStartRoutine;
        ptTaskHandle->pArg   = pArg;
        if(!(ptTaskHandle->pStack = MsAllocateMem((nStkSz) ? nStkSz : CAM_OS_THREAD_STACKSIZE_DEFAULT)))
        {
            CAM_OS_WARN("alloc stack fail");
            eRet = CAM_OS_ALLOCMEM_FAIL;
            break;
        }

        memset(&ptTaskHandle->tExitFlag, 0, sizeof(Ms_Flag_t));
        MsFlagInit(&ptTaskHandle->tExitFlag);
        //VEN_TEST_CHECK_RESULT((pTaskHandle->exit_flag.FlagId >> 0)  && (pTaskHandle->exit_flag.FlagState == RTK_FLAG_INITIALIZED));

        tTaskArgs.Priority = (nPrio >= 0 && nPrio <= 200) ? nPrio : 100;
        tTaskArgs.StackSize = (nStkSz) ? nStkSz : CAM_OS_THREAD_STACKSIZE_DEFAULT;
        tTaskArgs.pStackTop = (u32*)ptTaskHandle->pStack;
        tTaskArgs.AppliInit = &_CamOSThreadEntry;
        tTaskArgs.AppliParser = _CamOsThreadEmptyParser;
        tTaskArgs.pInitArgs = ptTaskHandle;
        tTaskArgs.TaskId = &ptTaskHandle->eHandleObj;
        tTaskArgs.ImmediatelyStart = TRUE;
        tTaskArgs.TimeSliceMax = 10;
        tTaskArgs.TimeSliceLeft = 10;
        strncpy(tTaskArgs.TaskName, (ptAttrb && ptAttrb->szName)? ptAttrb->szName : "CamOsWrp", sizeof(tTaskArgs.TaskName));

        if(MS_OK != MsCreateTask(&tTaskArgs))
        {
            CAM_OS_WARN("create fail");
            eRet = CAM_OS_FAIL;
            break;
        }

        *ptThread = ptTaskHandle;
    }
    while(0);

    if(!*ptThread)
    {
        if(ptTaskHandle)
        {
            if(ptTaskHandle->pStack)
            {
                MsReleaseMemory(ptTaskHandle->pStack);
            }
            MsFlagDestroy(&ptTaskHandle->tExitFlag);
            MsReleaseMemory(ptTaskHandle);
        }
    }

    //CamOsPrintf("%s get taskid: %d(%s)  priority: %d\n", __FUNCTION__, (u32)ptTaskHandle->eHandleObj, tTaskArgs.TaskName, tTaskArgs.Priority);
#elif defined(CAM_OS_LINUX_USER)
    CamOsThreadHandleLinuxUser_t *ptTaskHandle = NULL;
    struct sched_param tSched;
    pthread_attr_t tAttr;
    s32 nNiceVal = 0;

    *ptThread = NULL;
    do
    {
        if(!(ptTaskHandle = CamOsMemCalloc(sizeof(CamOsThreadHandleLinuxUser_t), 1)))
        {
            CAM_OS_WARN("alloc handle fail");
            eRet = CAM_OS_ALLOCMEM_FAIL;
            break;
        }

        ptTaskHandle->pfnEntry = pfnStartRoutine;
        ptTaskHandle->pArg = pArg;
        ptTaskHandle->nPid = 0;
        CamOsTsemInit(&ptTaskHandle->tEntryIn, 0);

        if(ptAttrb != NULL)
        {
            do
            {
                pthread_attr_init(&tAttr);

                // Set SCHED_RR priority before thread created.
                if((ptAttrb->nPriority > 70) && (ptAttrb->nPriority <= 100))
                {
                    pthread_attr_getschedparam(&tAttr, &tSched);
                    pthread_attr_setinheritsched(&tAttr, PTHREAD_EXPLICIT_SCHED);
                    pthread_attr_setschedpolicy(&tAttr, SCHED_RR);
                    if (ptAttrb->nPriority < 95)    // nPriority 71~94 mapping to Linux PrioRT 1~94
                        tSched.sched_priority = (ptAttrb->nPriority - 71) * 93 / 23 + 1;
                    else                            // nPriority 95~99 mapping to Linux PrioRT 95~99
                        tSched.sched_priority = (ptAttrb->nPriority < 100)? ptAttrb->nPriority : 99;
                    if(0 != pthread_attr_setschedparam(&tAttr, &tSched))
                    {
                        CAM_OS_WARN("set priority fail");
                        eRet = CAM_OS_FAIL;
                        break;
                    }
                }

                if(0 != ptAttrb->nStackSize)
                {
                    if(0 != pthread_attr_setstacksize(&tAttr, (size_t) ptAttrb->nStackSize))
                    {
                        eRet = CAM_OS_FAIL;
                        CAM_OS_WARN("set stack size fail");
                        break;
                    }
                }
                pthread_create(&ptTaskHandle->tThreadHandle, &tAttr, (void *)_CamOSThreadEntry, ptTaskHandle);
            }
            while(0);
            pthread_attr_destroy(&tAttr);
        }
        else
        {
            pthread_create(&ptTaskHandle->tThreadHandle, NULL, (void *)_CamOSThreadEntry, ptTaskHandle);
        }

        if (ptAttrb && ptAttrb->szName)
        {
            CamOsThreadSetName((CamOsThread *)ptTaskHandle, ptAttrb->szName);
        }

        // Set SCHED_OTHER priority after thread created.
        if(ptAttrb && (ptAttrb->nPriority > 0) && (ptAttrb->nPriority <= 70))
        {
            if (CamOsTsemTimedDown(&ptTaskHandle->tEntryIn, 1000) == CAM_OS_OK && ptTaskHandle->nPid != 0)
            {
                if (ptAttrb->nPriority <= 50)
                {
                    nNiceVal = 19 - ptAttrb->nPriority * 19 / 50;
                }
                else
                {
                    nNiceVal = 50 - ptAttrb->nPriority;
                }

                setpriority(PRIO_PROCESS, ptTaskHandle->nPid, nNiceVal);
            }
            else
            {
                CAM_OS_WARN("set priority fail");
            }
        }

        *ptThread = (CamOsThread *)ptTaskHandle;
    }
    while(0);

    if(!*ptThread)
    {
        if(ptTaskHandle)
        {
            CamOsMemRelease(ptTaskHandle);
        }
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    struct task_struct *tpThreadHandle;
    struct sched_param tSched = { .sched_priority = 0 };
    s32 nNiceVal = 0;

    tpThreadHandle = kthread_run((CamOsThreadEntry_t *)pfnStartRoutine, pArg, (ptAttrb && ptAttrb->szName)? ptAttrb->szName : "CAMOS");

    if(ptAttrb != NULL)
    {
        if((ptAttrb->nPriority > 0) && (ptAttrb->nPriority <= 70))
        {
            if (ptAttrb->nPriority <= 50)
            {
                nNiceVal = 19 - ptAttrb->nPriority * 19 / 50;
            }
            else
            {
                nNiceVal = 50 - ptAttrb->nPriority;
            }

            set_user_nice(tpThreadHandle, nNiceVal);
        }
        else if((ptAttrb->nPriority > 70) && (ptAttrb->nPriority <= 100))
        {
            if (ptAttrb->nPriority < 95)    // nPriority 71~94 mapping to Linux PrioRT 1~94
                tSched.sched_priority = (ptAttrb->nPriority - 71) * 93 / 23 + 1;
            else                            // nPriority 95~99 mapping to Linux PrioRT 95~99
                tSched.sched_priority = (ptAttrb->nPriority < 100)? ptAttrb->nPriority : 99;
            if(sched_setscheduler(tpThreadHandle, SCHED_RR, &tSched) != 0)
            {
                CAM_OS_WARN("set priority fail");
            }
        }
    }

    *ptThread = (CamOsThread *)tpThreadHandle;
#endif

    // coverity[leaked_storage]
    return eRet;
}

CamOsRet_e CamOsThreadChangePriority(CamOsThread tThread, u32 nPriority)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)tThread;
    u32 nPrio = 100;
    if(ptTaskHandle)
    {
        if(nPriority > 0 && nPriority < 100)
        {
            nPrio = nPriority * 2;
        }
        MsChangeTaskPriority(ptTaskHandle->eHandleObj, (nPrio >= 0 && nPrio <= 200) ? nPrio : 100);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsThreadHandleLinuxUser_t *ptTaskHandle = (CamOsThreadHandleLinuxUser_t *)tThread;
    struct sched_param tSched = { .sched_priority = 0 };
    s32 nNiceVal = 0;

    if(ptTaskHandle && (nPriority >= 0) && (nPriority <= 70))
    {
        nPriority = nPriority? nPriority : 50;
        if (nPriority <= 50)
        {
            nNiceVal = 19 - nPriority * 19 / 50;
        }
        else
        {
            nNiceVal = 50 - nPriority;
        }

        setpriority(PRIO_PROCESS, ptTaskHandle->nPid, nNiceVal);
        tSched.sched_priority = 0;
        if(pthread_setschedparam(ptTaskHandle->tThreadHandle, SCHED_OTHER, &tSched) != 0)
        {
            CAM_OS_WARN("set priority fail");
            eRet = CAM_OS_FAIL;
        }
    }
    else if(ptTaskHandle && (nPriority > 70) && (nPriority <= 100))
    {
        if (nPriority < 95)     // nPriority 71~94 mapping to Linux PrioRT 1~94
            tSched.sched_priority = (nPriority - 71) * 93 / 23 + 1;
        else                    // nPriority 95~99 mapping to Linux PrioRT 95~99
            tSched.sched_priority = (nPriority < 100)? nPriority : 99;
        if(pthread_setschedparam(ptTaskHandle->tThreadHandle, SCHED_RR, &tSched) != 0)
        {
            CAM_OS_WARN("set priority fail");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    struct task_struct *tpThreadHandle = (struct task_struct *)tThread;
    struct sched_param tSched = { .sched_priority = 0 };
    s32 nNiceVal = 0;

    if(tpThreadHandle && (nPriority >= 0) && (nPriority <= 70))
    {
        nPriority = nPriority? nPriority : 50;
        if (nPriority <= 50)
        {
            nNiceVal = 19 - nPriority * 19 / 50;
        }
        else
        {
            nNiceVal = 50 - nPriority;
        }

        set_user_nice(tpThreadHandle, nNiceVal);
        tSched.sched_priority = 0;
        if(sched_setscheduler(tpThreadHandle, SCHED_NORMAL, &tSched) != 0)
        {
            CAM_OS_WARN("set priority fail");
            eRet = CAM_OS_FAIL;
        }
    }
    else if(tpThreadHandle && (nPriority > 70) && (nPriority <= 100))
    {
        if (nPriority < 95)     // nPriority 71~94 mapping to Linux PrioRT 1~94
            tSched.sched_priority = (nPriority - 71) * 93 / 23 + 1;
        else                    // nPriority 95~99 mapping to Linux PrioRT 95~99
            tSched.sched_priority = (nPriority < 100)? nPriority : 99;
        if(sched_setscheduler(tpThreadHandle, SCHED_RR, &tSched) != 0)
        {
            CAM_OS_WARN("set priority fail");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsThreadSchedule(u8 bInterruptible, u32 nMsec)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    if(nMsec == CAM_OS_MAX_TIMEOUT)
    {
        // Avoid u32 overflow, put nMsec to MsSleep without convert
        if(CAM_OS_OK != MsSleep(nMsec))
            eRet = CAM_OS_TIMEOUT;
    }
    else if(nMsec > 0)
    {
        if(CAM_OS_OK != MsSleep(RTK_MS_TO_TICK(nMsec)))
            eRet = CAM_OS_TIMEOUT;
    }
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    if(nMsec == CAM_OS_MAX_TIMEOUT)
    {
        if(bInterruptible)
            set_current_state(TASK_INTERRUPTIBLE);
        else
            set_current_state(TASK_UNINTERRUPTIBLE);

        schedule();
    }
    else if(nMsec > 0)
    {
        if(bInterruptible)
        {
            if(0 == schedule_timeout(msecs_to_jiffies(nMsec) + 1))
                eRet = CAM_OS_TIMEOUT;
        }
        else
        {
            schedule_timeout_uninterruptible(msecs_to_jiffies(nMsec) + 1);
        }
    }
#endif
    return eRet;
}

CamOsRet_e CamOsThreadWakeUp(CamOsThread tThread)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)tThread;
    if(ptTaskHandle)
    {
        RtkWakeUpTask(ptTaskHandle->eHandleObj);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct task_struct *tpThreadHandle = (struct task_struct *)tThread;
    if(tpThreadHandle)
    {
        wake_up_process(tpThreadHandle);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsThreadJoin(CamOsThread tThread)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)tThread;
    if(ptTaskHandle)
    {
        MsFlagWait(&ptTaskHandle->tExitFlag, 0x00000001, RTK_FLAG_WAITMODE_AND | RTK_FLAG_WAITMODE_CLR);
        MsFlagDestroy(&ptTaskHandle->tExitFlag);
        MsDeleteTask(ptTaskHandle->eHandleObj);

        if(ptTaskHandle->pStack)
        {
            MsReleaseMemory(ptTaskHandle->pStack);
        }
        MsReleaseMemory(ptTaskHandle);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsThreadHandleLinuxUser_t *ptTaskHandle = (CamOsThreadHandleLinuxUser_t *)tThread;
    if(ptTaskHandle && ptTaskHandle->tThreadHandle)
    {
        pthread_join(ptTaskHandle->tThreadHandle, NULL);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CAM_OS_WARN("not support in "OS_NAME);
    eRet = CAM_OS_FAIL;
#endif
    return eRet;
}

CamOsRet_e CamOsThreadStop(CamOsThread tThread)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)tThread;
    if(ptTaskHandle)
    {
        MsTaskId_e eHandleObj = ptTaskHandle->eHandleObj;
        CAM_OS_SET_BIT(ptTaskHandle->eHandleObj, aThreadStopBitmap);
        CamOsThreadJoin(tThread);
        CAM_OS_CLEAR_BIT(eHandleObj, aThreadStopBitmap);
    }
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct task_struct *tpThreadHandle = (struct task_struct *)tThread;
    s32 nErr;
    if(tpThreadHandle)
    {
        if(0 != (nErr = kthread_stop(tpThreadHandle)))
        {
            CAM_OS_WARN("stop fail");
            CamOsPrintf("%s Err=%d\n", __FUNCTION__, nErr);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsThreadShouldStop(void)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    if (!CAM_OS_TEST_BIT(MsCurrTask(), aThreadStopBitmap))
        eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    if(kthread_should_stop())
    {
        eRet = CAM_OS_OK;
    }
    else
    {
        eRet = CAM_OS_FAIL;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsThreadSetName(CamOsThread tThread, const char *szName)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)tThread;
    if(ptTaskHandle && szName)
    {
        MsSetTaskName(ptTaskHandle->eHandleObj, szName);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsThreadHandleLinuxUser_t *ptTaskHandle = (CamOsThreadHandleLinuxUser_t *)tThread;
    if(ptTaskHandle && ptTaskHandle->tThreadHandle)
    {
        if(strlen(szName) >= 16) // Linux limitation
            return CAM_OS_PARAM_ERR;
#if defined(__GLIBC__) && defined(__GLIBC_PREREQ) && __GLIBC_PREREQ(2, 12)
        if(pthread_setname_np(ptTaskHandle->tThreadHandle, szName) != 0)
        {
            eRet = CAM_OS_PARAM_ERR;
        }
#else
        if(ptTaskHandle->tThreadHandle == pthread_self())
        {
            if(prctl(PR_SET_NAME, szName) != 0)
                eRet = CAM_OS_PARAM_ERR;
        }
        else
        {
            CAM_OS_WARN("not support set by other thread (in uclibc?)");
        }
#endif
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CAM_OS_WARN("not support in "OS_NAME);
    eRet = CAM_OS_FAIL;
#endif

    return eRet;
}

CamOsRet_e CamOsThreadGetName(CamOsThread tThread, char *szName, u32 nLen)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsThreadHandleRtk_t *ptTaskHandle = (CamOsThreadHandleRtk_t *)tThread;
    if(ptTaskHandle && szName)
    {
        MsGetTaskName(ptTaskHandle->eHandleObj, szName, nLen);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsThreadHandleLinuxUser_t *ptTaskHandle = (CamOsThreadHandleLinuxUser_t *)tThread;
    if(ptTaskHandle && ptTaskHandle->tThreadHandle)
    {
        if(nLen < 16) // Linux limitation
            return CAM_OS_PARAM_ERR;
#if defined(__GLIBC__) && defined(__GLIBC_PREREQ) && __GLIBC_PREREQ(2, 12)
        if(pthread_getname_np(ptTaskHandle->tThreadHandle, szName, nLen) != 0)
        {
            eRet = CAM_OS_PARAM_ERR;
        }
#else
        if(ptTaskHandle->tThreadHandle == pthread_self())
        {
            if(prctl(PR_GET_NAME, szName) != 0)
                eRet = CAM_OS_PARAM_ERR;
        }
        else
        {
            CAM_OS_WARN("not support set by other thread (in uclibc?)");
        }
#endif
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CAM_OS_WARN("not support in "OS_NAME);
    eRet = CAM_OS_FAIL;
#endif
    return eRet;
}

u32 CamOsThreadGetID()
{
#ifdef CAM_OS_RTK
    return MsCurrTask();
#elif defined(CAM_OS_LINUX_USER)
    return (u32)syscall(__NR_gettid);
#elif defined(CAM_OS_LINUX_KERNEL)
    return current->tgid;
#endif
}

CamOsRet_e CamOsMutexInit(CamOsMutex_t *ptMutex)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMutexRtk_t *ptHandle = (CamOsMutexRtk_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            MsMutexLock(&_gtSelfInitLock);
            if(ptHandle->nInited != INIT_MAGIC_NUM)
            {
                if(CUS_OS_OK != MsInitMutex(&ptHandle->tMutex))
                {
                    CAM_OS_WARN("init fail");
                    eRet = CAM_OS_FAIL;
                }
                else
                    ptHandle->nInited = INIT_MAGIC_NUM;
            }
            MsMutexUnlock(&_gtSelfInitLock);
        }
        else
        {
            CAM_OS_WARN("already inited");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsMutexLU_t *ptHandle = (CamOsMutexLU_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            pthread_mutex_lock(&_gtSelfInitLock);
            if(ptHandle->nInited != INIT_MAGIC_NUM)
            {
                if(0 != pthread_mutex_init(&ptHandle->tMutex, NULL))
                {
                    CAM_OS_WARN("init fail");
                    eRet = CAM_OS_FAIL;
                }
                else
                    ptHandle->nInited = INIT_MAGIC_NUM;
            }
            pthread_mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CAM_OS_WARN("already inited");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMutexLK_t *ptHandle = (CamOsMutexLK_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            mutex_lock(&_gtSelfInitLock);
            if(ptHandle->nInited != INIT_MAGIC_NUM)
            {
                mutex_init(&ptHandle->tMutex);
                ptHandle->nInited = INIT_MAGIC_NUM;
            }
            mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CAM_OS_WARN("already inited");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsMutexDestroy(CamOsMutex_t *ptMutex)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMutexRtk_t *ptHandle = (CamOsMutexRtk_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsMutexLU_t *ptHandle = (CamOsMutexLU_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
        {
            pthread_mutex_destroy(&ptHandle->tMutex);
            ptHandle->nInited = 0;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMutexLK_t *ptHandle = (CamOsMutexLK_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsMutexLock(CamOsMutex_t *ptMutex)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMutexRtk_t *ptHandle = (CamOsMutexRtk_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsMutexInit(ptMutex);
        }

        if(CUS_OS_OK != MsMutexLock(&ptHandle->tMutex))
        {
            CAM_OS_WARN("lock fail");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    s32 nErr = 0;
    CamOsMutexLU_t *ptHandle = (CamOsMutexLU_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsMutexInit(ptMutex);
        }

        if(0 != (nErr = pthread_mutex_lock(&ptHandle->tMutex)))
        {
            CAM_OS_WARN("lock fail");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMutexLK_t *ptHandle = (CamOsMutexLK_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsMutexInit(ptMutex);
        }

        mutex_lock(&ptHandle->tMutex);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsMutexTryLock(CamOsMutex_t *ptMutex)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMutexRtk_t *ptHandle = (CamOsMutexRtk_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsMutexInit(ptMutex);
        }

        if(CUS_OS_UNIT_NOAVAIL == MsMutexTryLock(&ptHandle->tMutex))
        {
            eRet = CAM_OS_RESOURCE_BUSY;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    s32 nErr = 0;
    CamOsMutexLU_t *ptHandle = (CamOsMutexLU_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsMutexInit(ptMutex);
        }

        if(0 != (nErr = pthread_mutex_trylock(&ptHandle->tMutex)))
        {
            if(nErr == EAGAIN)
            {
                eRet = CAM_OS_RESOURCE_BUSY;
            }
            else
            {
                CAM_OS_WARN("lock fail");
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMutexLK_t *ptHandle = (CamOsMutexLK_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsMutexInit(ptMutex);
        }

        if(0 != mutex_trylock(&ptHandle->tMutex))
            eRet = CAM_OS_RESOURCE_BUSY;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsMutexUnlock(CamOsMutex_t *ptMutex)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMutexRtk_t *ptHandle = (CamOsMutexRtk_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsMutexInit(ptMutex);
        }

        if(CUS_OS_OK != MsMutexUnlock(&ptHandle->tMutex))
        {
            CAM_OS_WARN("unlock fail");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    s32 nErr = 0;
    CamOsMutexLU_t *ptHandle = (CamOsMutexLU_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsMutexInit(ptMutex);
        }

        if(0 != (nErr = pthread_mutex_unlock(&ptHandle->tMutex)))
        {
            CAM_OS_WARN("unlock fail");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMutexLK_t *ptHandle = (CamOsMutexLK_t *)ptMutex;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsMutexInit(ptMutex);
        }

        mutex_unlock(&ptHandle->tMutex);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTsemInit(CamOsTsem_t *ptTsem, u32 nVal)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            MsMutexLock(&_gtSelfInitLock);

            if(CUS_OS_OK != MsCreateDynSemExtend(&ptHandle->Tsem, CAM_OS_MAX_INT - 1, nVal))
            {
                CAM_OS_WARN("init fail");
                eRet = CAM_OS_FAIL;
            }
            else
                ptHandle->nInited = INIT_MAGIC_NUM;

            MsMutexUnlock(&_gtSelfInitLock);
        }
        else
        {
            CAM_OS_WARN("already inited");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            pthread_mutex_lock(&_gtSelfInitLock);

            if(0 != sem_init(&ptHandle->tSem, 1, nVal))
            {
                CAM_OS_WARN("init fail");
                eRet = CAM_OS_FAIL;
            }
            else
                ptHandle->nInited = INIT_MAGIC_NUM;

            pthread_mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CAM_OS_WARN("already inited");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            mutex_lock(&_gtSelfInitLock);

            sema_init(&ptHandle->tSem, nVal);
            ptHandle->nInited = INIT_MAGIC_NUM;

            mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CAM_OS_WARN("already inited");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTsemDeinit(CamOsTsem_t *ptTsem)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
        {
            MsDestroyDynSem(&ptHandle->Tsem);
            ptHandle->nInited = 0;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else if(0 != sem_destroy(&ptHandle->tSem))
        {
            ptHandle->nInited = 0;
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

void CamOsTsemUp(CamOsTsem_t *ptTsem)
{
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            MsProduceDynSem(&ptHandle->Tsem);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            sem_post(&ptHandle->tSem);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            up(&ptHandle->tSem);
    }
#endif
}

void CamOsTsemDown(CamOsTsem_t *ptTsem)
{
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            MsConsumeDynSem(&ptHandle->Tsem);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            sem_wait(&ptHandle->tSem);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            down(&ptHandle->tSem);
    }
#endif
}

CamOsRet_e CamOsTsemDownInterruptible(CamOsTsem_t *ptTsem)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
            MsConsumeDynSem(&ptHandle->Tsem);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
            sem_wait(&ptHandle->tSem);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
        {
            if(-EINTR == down_interruptible(&ptHandle->tSem))
            {
                eRet = CAM_OS_INTERRUPTED;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTsemTimedDown(CamOsTsem_t *ptTsem, u32 nMsec)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else if(CUS_OS_NO_MESSAGE == MsConsumeDynSemDelay(&ptHandle->Tsem, RTK_MS_TO_TICK(nMsec)))
            eRet = CAM_OS_TIMEOUT;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    struct timespec tFinalTime;
    s64 nNanoDelay = 0;

    if(ptHandle)
    {
        if(clock_gettime(CLOCK_REALTIME, &tFinalTime) == -1)
            CAM_OS_WARN("clock_gettime fail");

        nNanoDelay = (nMsec * 1000000LL) + tFinalTime.tv_nsec;
        tFinalTime.tv_sec += (nNanoDelay / 1000000000LL);
        tFinalTime.tv_nsec = nNanoDelay % 1000000000LL;

        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else if(0 != sem_timedwait(&ptHandle->tSem, &tFinalTime))
        {
            if(errno == ETIMEDOUT)
            {
                eRet = CAM_OS_TIMEOUT;
            }
            else
            {
                CAM_OS_WARN("down fail");
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else if(0 != down_timeout(&ptHandle->tSem, msecs_to_jiffies(nMsec)))
            eRet = CAM_OS_TIMEOUT;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTsemTryDown(CamOsTsem_t *ptTsem)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTsemRtk_t *ptHandle = (CamOsTsemRtk_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else if(CUS_OS_UNIT_NOAVAIL == MsPollDynSem(&ptHandle->Tsem))
        {
            eRet = CAM_OS_RESOURCE_BUSY;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTsemLU_t *ptHandle = (CamOsTsemLU_t *)ptTsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else if(0 != sem_trywait(&ptHandle->tSem))
        {
            if(errno == EAGAIN)
            {
                eRet = CAM_OS_RESOURCE_BUSY;
            }
            else
            {
                CAM_OS_WARN("down fail");
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTsemLK_t *ptHandle = (CamOsTsemLK_t *)ptTsem;
    s32 nErr;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else if(0 != (nErr = down_trylock(&ptHandle->tSem)))
        {
            CAM_OS_WARN("down fail");
            eRet = CAM_OS_RESOURCE_BUSY;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsRwsemInit(CamOsRwsem_t *ptRwsem)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            if(CUS_OS_OK != MsInitMutex(&ptHandle->tRMutex) ||
               CUS_OS_OK != MsCreateDynSem(&ptHandle->WTsem, 1))
            {
                CAM_OS_WARN("init fail");
                eRet = CAM_OS_FAIL;
            }
            else
            {
                ptHandle->nReadCount = 0;
                ptHandle->nInited = INIT_MAGIC_NUM;
            }
        }
        else
        {
            CAM_OS_WARN("already inited");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            pthread_mutex_lock(&_gtSelfInitLock);

            if(0 != pthread_rwlock_init(&ptHandle->tRwsem, NULL))
            {
                CAM_OS_WARN("init fail");
                eRet = CAM_OS_FAIL;
            }
            else
                ptHandle->nInited = INIT_MAGIC_NUM;

            pthread_mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CAM_OS_WARN("already inited");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            mutex_lock(&_gtSelfInitLock);

            init_rwsem(&ptHandle->tRwsem);
            ptHandle->nInited = INIT_MAGIC_NUM;

            mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CAM_OS_WARN("already inited");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsRwsemDeinit(CamOsRwsem_t *ptRwsem)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
        {
            MsDestroyDynSem(&ptHandle->WTsem);
            ptHandle->nInited = 0;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else if(0 != pthread_rwlock_destroy(&ptHandle->tRwsem))
        {
            ptHandle->nInited = 0;
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

void CamOsRwsemUpRead(CamOsRwsem_t *ptRwsem)
{
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
        {
            MsMutexLock(&ptHandle->tRMutex);
            ptHandle->nReadCount--;
            if(ptHandle->nReadCount == 0)
                MsProduceDynSem(&ptHandle->WTsem);
            MsMutexUnlock(&ptHandle->tRMutex);
        }
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            pthread_rwlock_unlock(&ptHandle->tRwsem);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            up_read(&ptHandle->tRwsem);
    }
#endif
}

void CamOsRwsemUpWrite(CamOsRwsem_t *ptRwsem)
{
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            MsProduceDynSem(&ptHandle->WTsem);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            pthread_rwlock_unlock(&ptHandle->tRwsem);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            up_write(&ptHandle->tRwsem);
    }
#endif
}

void CamOsRwsemDownRead(CamOsRwsem_t *ptRwsem)
{
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
        {
            MsMutexLock(&ptHandle->tRMutex);
            ptHandle->nReadCount++;
            if(ptHandle->nReadCount == 1)
                MsConsumeDynSem(&ptHandle->WTsem);
            MsMutexUnlock(&ptHandle->tRMutex);
        }
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            pthread_rwlock_rdlock(&ptHandle->tRwsem);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            down_read(&ptHandle->tRwsem);
    }
#endif
}

void CamOsRwsemDownWrite(CamOsRwsem_t *ptRwsem)
{
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            MsConsumeDynSem(&ptHandle->WTsem);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            pthread_rwlock_wrlock(&ptHandle->tRwsem);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            down_write(&ptHandle->tRwsem);
    }
#endif
}

CamOsRet_e CamOsRwsemTryDownRead(CamOsRwsem_t *ptRwsem)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else if(CUS_OS_UNIT_NOAVAIL == MsMutexTryLock(&ptHandle->tRMutex))
            eRet = CAM_OS_RESOURCE_BUSY;
        else
        {
            ptHandle->nReadCount++;
            if(ptHandle->nReadCount == 1)
            {
                if(CUS_OS_UNIT_NOAVAIL == MsPollDynSem(&ptHandle->WTsem))
                {
                    eRet = CAM_OS_RESOURCE_BUSY;
                    ptHandle->nReadCount--;
                }
            }
            MsMutexUnlock(&ptHandle->tRMutex);
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    s32 nErr;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else if(0 != (nErr = pthread_rwlock_tryrdlock(&ptHandle->tRwsem)))
        {
            if(nErr == EBUSY)
            {
                eRet = CAM_OS_RESOURCE_BUSY;
            }
            else
            {
                CAM_OS_WARN("lock fail");
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    s32 nErr;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else if(1 != (nErr = down_read_trylock(&ptHandle->tRwsem)))
        {
            eRet = CAM_OS_RESOURCE_BUSY;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsRwsemTryDownWrite(CamOsRwsem_t *ptRwsem)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsRwsemRtk_t *ptHandle = (CamOsRwsemRtk_t *)ptRwsem;

    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else if(CUS_OS_UNIT_NOAVAIL == MsPollDynSem(&ptHandle->WTsem))
        {
            eRet = CAM_OS_RESOURCE_BUSY;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsRwsemLU_t *ptHandle = (CamOsRwsemLU_t *)ptRwsem;
    s32 nErr;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else if(0 != (nErr = pthread_rwlock_trywrlock(&ptHandle->tRwsem)))
        {
            if(nErr == EBUSY)
            {
                eRet = CAM_OS_RESOURCE_BUSY;
            }
            else
            {
                CAM_OS_WARN("lock fail");
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsRwsemLK_t *ptHandle = (CamOsRwsemLK_t *)ptRwsem;
    s32 nErr;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else if(1 != (nErr = down_write_trylock(&ptHandle->tRwsem)))
        {
            eRet = CAM_OS_RESOURCE_BUSY;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTcondInit(CamOsTcond_t *ptTcond)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTcondRtk_t *ptHandle = (CamOsTcondRtk_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            MsMutexLock(&_gtSelfInitLock);

            if(CUS_OS_OK != MsCreateDynSem(&ptHandle->Tsem, 0))
            {
                CAM_OS_WARN("create fail");
                eRet = CAM_OS_FAIL;
            }
            else
                ptHandle->nInited = INIT_MAGIC_NUM;

            MsMutexUnlock(&_gtSelfInitLock);
        }
        else
        {
            CAM_OS_WARN("already inited");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondLU_t *ptHandle = (CamOsTcondLU_t *)ptTcond;
    pthread_condattr_t cattr;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            pthread_mutex_lock(&_gtSelfInitLock);

            if(0 != pthread_condattr_init(&cattr) ||
                    0 != pthread_condattr_setclock(&cattr, CLOCK_MONOTONIC))
            {
                CAM_OS_WARN("pthread_condattr_init fail");
                eRet = CAM_OS_FAIL;
            }

            if(0 != pthread_cond_init(&ptHandle->tCondition, &cattr))
            {
                CAM_OS_WARN("pthread_cond_init fail");
                eRet = CAM_OS_FAIL;
            }
            if(0 != pthread_mutex_init(&ptHandle->tMutex, NULL))
            {
                CAM_OS_WARN("pthread_mutex_init fail");
                eRet = CAM_OS_FAIL;
            }
            else
                ptHandle->nInited = INIT_MAGIC_NUM;

            pthread_mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CAM_OS_WARN("already inited");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            mutex_lock(&_gtSelfInitLock);

            init_completion(&ptHandle->tCompletion);
            ptHandle->nInited = INIT_MAGIC_NUM;

            mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CAM_OS_WARN("already inited");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTcondDeinit(CamOsTcond_t *ptTcond)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTcondRtk_t *ptHandle = (CamOsTcondRtk_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
        {
            MsDestroyDynSem(&ptHandle->Tsem);
            ptHandle->nInited = 0;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondLU_t *ptHandle = (CamOsTcondLU_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
        {
            pthread_cond_destroy(&ptHandle->tCondition);
            pthread_mutex_destroy(&ptHandle->tMutex);
            ptHandle->nInited = 0;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
            ptHandle->nInited = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

void CamOsTcondSignal(CamOsTcond_t *ptTcond)
{
#ifdef CAM_OS_RTK
    CamOsTcondRtk_t *ptHandle = (CamOsTcondRtk_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            MsProduceSafeDynSem(&ptHandle->Tsem, 0);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondLU_t *ptHandle = (CamOsTcondLU_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
        {
            pthread_mutex_lock(&ptHandle->tMutex);
            pthread_cond_signal(&ptHandle->tCondition);
            pthread_mutex_unlock(&ptHandle->tMutex);
        }
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            complete(&ptHandle->tCompletion);
    }
#endif
}

void CamOsTcondSignalAll(CamOsTcond_t *ptTcond)
{
#ifdef CAM_OS_RTK
    CamOsTcondRtk_t *ptHandle = (CamOsTcondRtk_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            MsProduceSafeDynSem(&ptHandle->Tsem, 1);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondLU_t *ptHandle = (CamOsTcondLU_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
        {
            pthread_mutex_lock(&ptHandle->tMutex);
            pthread_cond_broadcast(&ptHandle->tCondition);
            pthread_mutex_unlock(&ptHandle->tMutex);
        }
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
        {
            complete_all(&ptHandle->tCompletion);
        }
    }
#endif
}

void CamOsTcondWait(CamOsTcond_t *ptTcond)
{
#ifdef CAM_OS_RTK
    CamOsTcondRtk_t *ptHandle = (CamOsTcondRtk_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
            MsConsumeAllDynSem(&ptHandle->Tsem);
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondLU_t *ptHandle = (CamOsTcondLU_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
        {
            pthread_mutex_lock(&ptHandle->tMutex);
            pthread_cond_wait(&ptHandle->tCondition, &ptHandle->tMutex);
            pthread_mutex_unlock(&ptHandle->tMutex);
        }
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;

    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
            CAM_OS_WARN("not inited");
        else
        {
            reinit_completion(&ptHandle->tCompletion);
            wait_for_completion(&ptHandle->tCompletion);
        }
    }
#endif
}

CamOsRet_e CamOsTcondTimedWait(CamOsTcond_t *ptTcond, u32 nMsec)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTcondRtk_t *ptHandle = (CamOsTcondRtk_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else if(CUS_OS_NO_MESSAGE == MsConsumeAllDynSemDelay(&ptHandle->Tsem, RTK_MS_TO_TICK(nMsec)))
            eRet = CAM_OS_TIMEOUT;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondLU_t *ptHandle = (CamOsTcondLU_t *)ptTcond;
    s32 nErr = 0;
    struct timespec tFinalTime;
    s64 nNanoDelay = 0;

    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
        {
            if(clock_gettime(CLOCK_MONOTONIC, &tFinalTime) == -1)
                CAM_OS_WARN("clock_gettime fail");

            nNanoDelay = (nMsec * 1000000LL) + tFinalTime.tv_nsec;
            tFinalTime.tv_sec += (nNanoDelay / 1000000000LL);
            tFinalTime.tv_nsec = nNanoDelay % 1000000000LL;

            pthread_mutex_lock(&ptHandle->tMutex);

            nErr = pthread_cond_timedwait(&ptHandle->tCondition, &ptHandle->tMutex,
                                          &tFinalTime);

            pthread_mutex_unlock(&ptHandle->tMutex);

            if(!nErr)
            {
                eRet = CAM_OS_OK;
            }
            else if(nErr == ETIMEDOUT)
            {
                eRet = CAM_OS_TIMEOUT;
            }
            else
            {
                CAM_OS_WARN("pthread_cond_timedwait fail");
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
        {
            reinit_completion(&ptHandle->tCompletion);
            if(!wait_for_completion_timeout(&ptHandle->tCompletion, msecs_to_jiffies(nMsec)))
            {
                eRet = CAM_OS_TIMEOUT;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTcondWaitInterruptible(CamOsTcond_t *ptTcond)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTcondWait(ptTcond);
#elif defined(CAM_OS_LINUX_USER)
    CamOsTcondWait(ptTcond);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
        {
            reinit_completion(&ptHandle->tCompletion);
            if (-ERESTARTSYS == wait_for_completion_interruptible(&ptHandle->tCompletion))
                eRet = CAM_OS_INTERRUPTED;
        }
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTcondTimedWaitInterruptible(CamOsTcond_t *ptTcond, u32 nMsec)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    eRet = CamOsTcondTimedWait(ptTcond, nMsec);
#elif defined(CAM_OS_LINUX_USER)
    eRet = CamOsTcondTimedWait(ptTcond, nMsec);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    s32 nWaitRet;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
        {
            reinit_completion(&ptHandle->tCompletion);
            nWaitRet = wait_for_completion_interruptible_timeout(&ptHandle->tCompletion, msecs_to_jiffies(nMsec));
            if (nWaitRet == -ERESTARTSYS)
            {
                eRet = CAM_OS_INTERRUPTED;
            }
            else if (nWaitRet == 0)
            {
                eRet = CAM_OS_TIMEOUT;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsTcondWaitActive(CamOsTcond_t *ptTcond)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTcondRtk_t *ptHandle = (CamOsTcondRtk_t *)ptTcond;
    s16 nSemCount;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
        {
            if(CUS_OS_OK == MsGetDynSemCount(&ptHandle->Tsem, &nSemCount))
            {
                if(nSemCount == 0)
                    eRet = CAM_OS_FAIL;
            }
            else
                eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTcondLK_t *ptHandle = (CamOsTcondLK_t *)ptTcond;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("not inited");
            eRet = CAM_OS_FAIL;
        }
        else
        {
            if(!completion_done(&ptHandle->tCompletion))
            {
                eRet = CAM_OS_FAIL;
            }
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsSpinInit(CamOsSpinlock_t *ptSpinlock)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsSpinlockRtk_t *ptHandle = (CamOsSpinlockRtk_t *)ptSpinlock;
    if(ptHandle)
    {
        ptHandle->nFlags = 0;
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsSpinlockLU_t *ptHandle = (CamOsSpinlockLU_t *)ptSpinlock;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            pthread_mutex_lock(&_gtSelfInitLock);
            if(ptHandle->nInited != INIT_MAGIC_NUM)
            {
                pthread_spin_init(&ptHandle->tLock, 0);
                ptHandle->nInited = INIT_MAGIC_NUM;
            }
            pthread_mutex_unlock(&_gtSelfInitLock);
        }
        else
        {
            CAM_OS_WARN("already inited");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsSpinlockLK_t *ptHandle = (CamOsSpinlockLK_t *)ptSpinlock;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            if (CamOsInInterrupt() != CAM_OS_OK)
            {
                mutex_lock(&_gtSelfInitLock);
                if(ptHandle->nInited != INIT_MAGIC_NUM)
                {
                    spin_lock_init(&ptHandle->tLock);
                    ptHandle->nInited = INIT_MAGIC_NUM;
                }
                mutex_unlock(&_gtSelfInitLock);
            }
            else
            {
                if(ptHandle->nInited != INIT_MAGIC_NUM)
                {
                    spin_lock_init(&ptHandle->tLock);
                    ptHandle->nInited = INIT_MAGIC_NUM;
                }
            }
        }
        else
        {
            CAM_OS_WARN("already inited");
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsSpinLock(CamOsSpinlock_t *ptSpinlock)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsSpinlockRtk_t *ptHandle = (CamOsSpinlockRtk_t *)ptSpinlock;
    if(ptHandle)
    {
        RtkEnterRegion();
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsSpinlockLU_t *ptHandle = (CamOsSpinlockLU_t *)ptSpinlock;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsSpinInit(ptSpinlock);
        }

        pthread_spin_lock(&ptHandle->tLock);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsSpinlockLK_t *ptHandle = (CamOsSpinlockLK_t *)ptSpinlock;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsSpinInit(ptSpinlock);
        }

        spin_lock(&ptHandle->tLock);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsSpinUnlock(CamOsSpinlock_t *ptSpinlock)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsSpinlockRtk_t *ptHandle = (CamOsSpinlockRtk_t *)ptSpinlock;
    if(ptHandle)
    {
        RtkLeaveRegion();
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CamOsSpinlockLU_t *ptHandle = (CamOsSpinlockLU_t *)ptSpinlock;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsSpinInit(ptSpinlock);
        }

        pthread_spin_unlock(&ptHandle->tLock);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsSpinlockLK_t *ptHandle = (CamOsSpinlockLK_t *)ptSpinlock;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsSpinInit(ptSpinlock);
        }

        spin_unlock(&ptHandle->tLock);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsSpinLockIrqSave(CamOsSpinlock_t *ptSpinlock)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsSpinlockRtk_t *ptHandle = (CamOsSpinlockRtk_t *)ptSpinlock;
    if(ptHandle)
    {
        RtkEnterRegionSaveIrq(&ptHandle->nFlags);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsSpinlockLK_t *ptHandle = (CamOsSpinlockLK_t *)ptSpinlock;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsSpinInit(ptSpinlock);
        }

        spin_lock_irqsave(&ptHandle->tLock, ptHandle->nFlags);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

CamOsRet_e CamOsSpinUnlockIrqRestore(CamOsSpinlock_t *ptSpinlock)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsSpinlockRtk_t *ptHandle = (CamOsSpinlockRtk_t *)ptSpinlock;
    if(ptHandle)
    {
        RtkLeaveRegionRestoreIrq(&ptHandle->nFlags);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsSpinlockLK_t *ptHandle = (CamOsSpinlockLK_t *)ptSpinlock;
    if(ptHandle)
    {
        if(ptHandle->nInited != INIT_MAGIC_NUM)
        {
            CamOsSpinInit(ptSpinlock);
        }

        spin_unlock_irqrestore(&ptHandle->tLock, ptHandle->nFlags);
    }
    else
    {
        eRet = CAM_OS_PARAM_ERR;
    }
#endif
    return eRet;
}

void* CamOsMemAlloc(u32 nSize)
{
#ifdef CAM_OS_RTK
    return MsAllocateMem(nSize);
#elif defined(CAM_OS_LINUX_USER)
    return malloc(nSize);
#elif defined(CAM_OS_LINUX_KERNEL)
    if (nSize > KMALLOC_THRESHOLD_SIZE)
        return vzalloc(nSize);
    else
        return kzalloc(nSize, GFP_KERNEL);
#endif
}

void* CamOsMemCalloc(u32 nNum, u32 nSize)
{
#ifdef CAM_OS_RTK
    return MsCallocateMem(nNum * nSize);
#elif defined(CAM_OS_LINUX_USER)
    return calloc(nNum, nSize);
#elif defined(CAM_OS_LINUX_KERNEL)
    if ((nNum * nSize) > KMALLOC_THRESHOLD_SIZE)
        return vzalloc(nNum * nSize);
    else
        return kzalloc(nNum * nSize, GFP_KERNEL);
#endif
}

void* CamOsMemRealloc(void* pPtr, u32 nSize)
{
#ifdef CAM_OS_RTK
    return MsMemoryReAllocate(pPtr, nSize);
#elif defined(CAM_OS_LINUX_USER)
    return realloc(pPtr, nSize);
#elif defined(CAM_OS_LINUX_KERNEL)
    void *pAddr;

    if (nSize > KMALLOC_THRESHOLD_SIZE)
        pAddr = vzalloc(nSize);
    else
        pAddr = kzalloc(nSize, GFP_KERNEL);

    if(pPtr && pAddr)
    {
        memcpy(pAddr, pPtr, nSize);
        kvfree(pPtr);
    }
    return pAddr;
#endif
}

void CamOsMemFlush(void* pPtr, u32 nSize)
{
#ifdef CAM_OS_RTK
    sys_flush_data_cache_buffer((u32)pPtr, nSize);
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement cache flush in linux user space.
    CAM_OS_WARN("not support in "OS_NAME);
#elif defined(CAM_OS_LINUX_KERNEL)
    //flush_icache_range((unsigned long)pPtr, nSize);
    Chip_Flush_Cache_Range((unsigned long)pPtr, nSize);
    Chip_Flush_Memory();
#endif
}

void CamOsMemInvalidate(void* pPtr, u32 nSize)
{
#ifdef CAM_OS_RTK
    sys_Invalidate_data_cache_buffer((u32)pPtr, nSize);
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement cache flush in linux user space.
    CAM_OS_WARN("not support in "OS_NAME);
#elif defined(CAM_OS_LINUX_KERNEL)
    Chip_Inv_Cache_Range((unsigned long)pPtr, nSize);
#endif
}

void CamOsMemRelease(void* pPtr)
{
#ifdef CAM_OS_RTK
    if(pPtr)
    {
        MsReleaseMemory(pPtr);
    }
#elif defined(CAM_OS_LINUX_USER)
    if(pPtr)
    {
        free(pPtr);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(pPtr)
    {
        kvfree(pPtr);
    }
#endif
}

static s32 _CheckDmemInfoListInited(void)
{
#ifdef CAM_OS_RTK
    MsMutexLock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_USER)
    pthread_mutex_lock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_KERNEL)
    mutex_lock(&_gtMemLock);
#endif
    if(!_gnDmemDbgListInited)
    {
        memset(&_gtMemList, 0, sizeof(MemoryList_t));
        CAM_OS_INIT_LIST_HEAD(&_gtMemList.tList);

        _gnDmemDbgListInited = 1;
    }
#ifdef CAM_OS_RTK
    MsMutexUnlock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_USER)
    pthread_mutex_unlock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_KERNEL)
    mutex_unlock(&_gtMemLock);
#endif

    return 0;
}

// Porting from stdint.h for Linux user space
#ifdef CAM_OS_LINUX_USER
/*
 * GCC doesn't provide an appropriate macro for [u]intptr_t
 * For now, use __PTRDIFF_TYPE__
 */
#if defined(__PTRDIFF_TYPE__)
typedef signed __PTRDIFF_TYPE__ intptr_t;
typedef unsigned __PTRDIFF_TYPE__ uintptr_t;
#else
/*
 * Fallback to hardcoded values,
 * should be valid on cpu's with 32bit int/32bit void*
 */
typedef signed long intptr_t;
typedef unsigned long uintptr_t;
#endif
#endif

CamOsRet_e CamOsDirectMemAlloc(const char* szName,
                               u32 nSize,
                               void** ppVirtPtr,
                               void** ppPhysPtr,
                               void** ppMiuPtr)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    u8 nAllocSucc = TRUE;
    void *pNonCachePtr = NULL;

    pNonCachePtr = MsAllocateNonCacheMemExt(nSize, 12);
    nAllocSucc &= MsIsHeapMemory(pNonCachePtr);

    if((u32)pNonCachePtr & ((1 << 6) - 1))
    {
        nAllocSucc &= FALSE;
        MsReleaseMemory(pNonCachePtr);
    }

    if(nAllocSucc == TRUE)
    {
        ASSIGN_POINTER_VALUE(ppVirtPtr, pNonCachePtr);
        ASSIGN_POINTER_VALUE(ppPhysPtr, MsVA2PA(pNonCachePtr));
        ASSIGN_POINTER_VALUE(ppMiuPtr, (void *)HalUtilPHY2MIUAddr((u32)*ppPhysPtr));

        /*CamOsPrintf("%s    0x%08X  0x%08X  0x%08X\n",
                __FUNCTION__,
                (u32)*ppVirtPtr,
                (u32)*ppPhysPtr,
                (u32)*ppMiuPtr);*/

        _CheckDmemInfoListInited();

        MsMutexLock(&_gtMemLock);
        MemoryList_t* ptNewEntry = (MemoryList_t*) MsAllocateMem(sizeof(MemoryList_t));
        ptNewEntry->pPhysPtr = *ppPhysPtr;
        ptNewEntry->pVirtPtr = *ppVirtPtr;
        ptNewEntry->pMemifoPtr = NULL;
        ptNewEntry->szName = (char *)MsAllocateMem(strlen(szName) + 1);
        if(ptNewEntry->szName)
            strncpy(ptNewEntry->szName, szName, strlen(szName));
        ptNewEntry->nSize = nSize;
        CAM_OS_LIST_ADD_TAIL(&(ptNewEntry->tList), &_gtMemList.tList);
        MsMutexUnlock(&_gtMemLock);
    }
    else
    {
        ASSIGN_POINTER_VALUE(ppVirtPtr, NULL);
        ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
        ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);
        eRet = CAM_OS_FAIL;
    }
#elif defined(CAM_OS_LINUX_USER)
#ifndef NO_MDRV_MSYS
    s32 nMsysFd = -1;
    s32 nMemFd = -1;
    MSYS_DMEM_INFO * ptMsysMem = NULL;
    unsigned char* pMmapPtr = NULL;
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    do
    {
        //Check request name to avoid allocate same dmem address.
        _CheckDmemInfoListInited();

        pthread_mutex_lock(&_gtMemLock);
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

            if(ptTmp->pPhysPtr && ptTmp->pVirtPtr && ptTmp->szName && 0 == strcmp(szName, ptTmp->szName))
            {
                fprintf(stderr, "%s request same dmem name: %s\n", __FUNCTION__, szName);
                eRet = CAM_OS_PARAM_ERR;
            }
        }
        pthread_mutex_unlock(&_gtMemLock);
        if(eRet == CAM_OS_PARAM_ERR)
        {
            ASSIGN_POINTER_VALUE(ppVirtPtr, NULL);
            ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
            ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);
            break;
        }

        if(0 > (nMsysFd = open("/dev/msys", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        if(0 > (nMemFd = open("/dev/mem", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/mem failed!!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        ptMsysMem = (MSYS_DMEM_INFO *) malloc(sizeof(MSYS_DMEM_INFO));
        MSYS_ADDR_TRANSLATION_INFO tAddrInfo;
        FILL_VERCHK_TYPE(tAddrInfo, tAddrInfo.VerChk_Version, tAddrInfo.VerChk_Size,
                         IOCTL_MSYS_VERSION);
        FILL_VERCHK_TYPE(*ptMsysMem, ptMsysMem->VerChk_Version,
                         ptMsysMem->VerChk_Size, IOCTL_MSYS_VERSION);

        ptMsysMem->length = nSize;
        snprintf(ptMsysMem->name, sizeof(ptMsysMem->name), "%s", szName);

        if(ioctl(nMsysFd, IOCTL_MSYS_REQUEST_DMEM, ptMsysMem))
        {
            ptMsysMem->length = 0;
            fprintf(stderr, "%s [%s][%d]Request Direct Memory Failed!!\n", __FUNCTION__, szName, (u32)nSize);
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }

        if(ptMsysMem->length < nSize)
        {
            ioctl(nMsysFd, IOCTL_MSYS_RELEASE_DMEM, ptMsysMem);
            fprintf(stderr, "%s [%s]Request Direct Memory Failed!! because dmem size <%d>smaller than <%d>\n",
                    __FUNCTION__, szName, ptMsysMem->length, (u32)nSize);
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }

        tAddrInfo.addr = ptMsysMem->phys;
        ASSIGN_POINTER_VALUE(ppPhysPtr, (void *)(uintptr_t)ptMsysMem->phys);
        if(ioctl(nMsysFd, IOCTL_MSYS_PHYS_TO_MIU, &tAddrInfo))
        {
            ioctl(nMsysFd, IOCTL_MSYS_RELEASE_DMEM, ptMsysMem);
            fprintf(stderr, "%s [%s][%d]IOCTL_MSYS_PHYS_TO_MIU Failed!!\n", __FUNCTION__, szName, (u32)nSize);
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }
        ASSIGN_POINTER_VALUE(ppMiuPtr, (void *)(uintptr_t)tAddrInfo.addr);
        pMmapPtr = mmap(0, ptMsysMem->length, PROT_READ | PROT_WRITE, MAP_SHARED,
                        nMemFd, ptMsysMem->phys);
        if(pMmapPtr == (void *) - 1)
        {
            ioctl(nMsysFd, IOCTL_MSYS_RELEASE_DMEM, ptMsysMem);
            fprintf(stderr, "%s failed!! physAddr<0x%x> size<0x%x> errno<%d, %s> \n",
                    __FUNCTION__,
                    (u32)ptMsysMem->phys,
                    (u32)ptMsysMem->length, errno, strerror(errno));
            free(ptMsysMem);
            eRet = CAM_OS_FAIL;
            break;
        }
        ASSIGN_POINTER_VALUE(ppVirtPtr, pMmapPtr);

        fprintf(stderr, "%s <%s> physAddr<0x%x> size<%d>\n",
                __FUNCTION__,
                szName, (u32)ptMsysMem->phys,
                (u32)ptMsysMem->length);

        pthread_mutex_lock(&_gtMemLock);
        MemoryList_t* ptNewEntry = (MemoryList_t*) malloc(sizeof(MemoryList_t));
        ptNewEntry->pPhysPtr = *ppPhysPtr;
        ptNewEntry->pVirtPtr = *ppVirtPtr;
        ptNewEntry->pMemifoPtr = (void *) ptMsysMem;
        ptNewEntry->szName = strdup(szName);
        ptNewEntry->nSize = ptMsysMem->length;
        CAM_OS_LIST_ADD_TAIL(&(ptNewEntry->tList), &_gtMemList.tList);
        pthread_mutex_unlock(&_gtMemLock);
    }
    while(0);

    if(nMsysFd >= 0)
    {
        close(nMsysFd);
    }
    if(nMemFd >= 0)
    {
        close(nMemFd);
    }
#else
    _CheckDmemInfoListInited();
#endif
#elif defined(CAM_OS_LINUX_KERNEL)
    MSYS_DMEM_INFO *ptDmem = NULL;
    MemoryList_t* ptNewEntry = NULL;
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    ASSIGN_POINTER_VALUE(ppVirtPtr, NULL);
    ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
    ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);

    do
    {
        //Check request name to avoid allocate same dmem address.
        _CheckDmemInfoListInited();

        mutex_lock(&_gtMemLock);
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

            if(ptTmp->pPhysPtr && ptTmp->pVirtPtr && ptTmp->szName && 0 == strcmp(szName, ptTmp->szName))
            {
                printk(KERN_WARNING "%s name conflict: %s\n", __FUNCTION__, szName);
                eRet = CAM_OS_PARAM_ERR;
            }
        }
        mutex_unlock(&_gtMemLock);
        if(eRet == CAM_OS_PARAM_ERR)
        {
            ASSIGN_POINTER_VALUE(ppVirtPtr, NULL);
            ASSIGN_POINTER_VALUE(ppPhysPtr, NULL);
            ASSIGN_POINTER_VALUE(ppMiuPtr, NULL);
            break;
        }

        if(0 == (ptDmem = (MSYS_DMEM_INFO *)kzalloc(sizeof(MSYS_DMEM_INFO), GFP_KERNEL)))
        {
            printk(KERN_WARNING "%s alloc DMEM INFO fail\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        snprintf(ptDmem->name, 15, szName);
        ptDmem->length = nSize;

        if(0 != msys_request_dmem(ptDmem))
        {
            printk(KERN_WARNING "%s request dmem fail\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        ASSIGN_POINTER_VALUE(ppVirtPtr, (void *)(uintptr_t)ptDmem->kvirt);
        ASSIGN_POINTER_VALUE(ppMiuPtr, (void *)(uintptr_t)Chip_Phys_to_MIU(ptDmem->phys));
        ASSIGN_POINTER_VALUE(ppPhysPtr, (void *)(uintptr_t)ptDmem->phys);

        printk(KERN_INFO "%s <%s> physAddr<0x%08X> size<%d>\n",
               __FUNCTION__,
               szName,
               (u32)ptDmem->phys,
               (u32)ptDmem->length);

        mutex_lock(&_gtMemLock);
        if(0 == (ptNewEntry = (MemoryList_t*) kzalloc(sizeof(MemoryList_t), GFP_KERNEL)))
        {
            printk(KERN_WARNING "%s alloc entry fail\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }
        ptNewEntry->pPhysPtr = *ppPhysPtr;
        ptNewEntry->pVirtPtr = *ppVirtPtr;
        ptNewEntry->pMemifoPtr = (void *) ptDmem;
        if(0 == (ptNewEntry->szName = (char *)kzalloc(strlen(szName) + 1, GFP_KERNEL)))
        {
            printk(KERN_WARNING "%s alloc entry name fail\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }
        strncpy(ptNewEntry->szName, szName, strlen(szName));
        ptNewEntry->nSize = ptDmem->length;
        CAM_OS_LIST_ADD_TAIL(&(ptNewEntry->tList), &_gtMemList.tList);
        mutex_unlock(&_gtMemLock);
    }
    while(0);

    if(eRet == CAM_OS_FAIL)
    {
        if(ptDmem)
        {
            if(ptDmem->phys)
            {
                msys_release_dmem(ptDmem);
            }
            kfree(ptDmem);
        }

        if(ptNewEntry)
        {
            if(ptNewEntry->szName)
            {
                kfree(ptNewEntry->szName);
            }
            kfree(ptNewEntry);
        }
    }
#endif
    return eRet;
}

CamOsRet_e CamOsDirectMemRelease(void* pPtr, u32 nSize)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    if(pPtr)
    {
        _CheckDmemInfoListInited();

        MsMutexLock(&_gtMemLock);
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

            if(ptTmp->pPhysPtr == pPtr || ptTmp->pVirtPtr == pPtr)
            {
                if(ptTmp->szName)
                    MsReleaseMemory(ptTmp->szName);
                CAM_OS_LIST_DEL(ptPos);
                MsReleaseMemory(ptTmp);
            }
        }
        MsMutexUnlock(&_gtMemLock);

        MsReleaseMemory(pPtr);
    }
#elif defined(CAM_OS_LINUX_USER)
#ifndef NO_MDRV_MSYS
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;
    s32 nMsysfd = -1;
    s32 nErr = 0;
    MSYS_DMEM_INFO *pMsysMem = NULL;

    if(pPtr)
    {
        do
        {
            if(0 > (nMsysfd = open("/dev/msys", O_RDWR | O_SYNC)))
            {
                fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
                eRet = CAM_OS_FAIL;
                break;
            }

            _CheckDmemInfoListInited();

            pthread_mutex_lock(&_gtMemLock);
            CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
            {
                ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

                if(ptTmp->pPhysPtr == pPtr || ptTmp->pVirtPtr == pPtr)
                {
                    pMsysMem = (MSYS_DMEM_INFO *) ptTmp->pMemifoPtr;
                    break;
                }
            }
            pthread_mutex_unlock(&_gtMemLock);
            if(pMsysMem == NULL)
            {
                fprintf(stderr, "%s find Msys_DMEM_Info node failed!! <0x%08X>\n", __FUNCTION__, (u32)pPtr);
                eRet = CAM_OS_FAIL;
                break;
            }

            if(nSize != pMsysMem->length)
            {
                nErr = munmap((void *)ptTmp->pVirtPtr, pMsysMem->length);
                if(0 != nErr)
                {
                    fprintf(stderr, "%s munmap failed!! <0x%08X> size<%d> err<%d> errno<%d, %s>\n",
                            __FUNCTION__, (u32)ptTmp->pVirtPtr, (u32)pMsysMem->length, nErr, errno, strerror(errno));
                }
            }
            else
            {
                nErr = munmap((void *)ptTmp->pVirtPtr, nSize);
                if(0 != nErr)
                {
                    fprintf(stderr, "%s munmap failed!! <0x%08X> size<%d> err<%d> errno<%d, %s>\n",
                            __FUNCTION__, (u32)ptTmp->pVirtPtr, (u32)nSize, nErr, errno, strerror(errno));
                }
            }

            if(ioctl(nMsysfd, IOCTL_MSYS_RELEASE_DMEM, pMsysMem))
            {
                fprintf(stderr, "%s : IOCTL_MSYS_RELEASE_DMEM error physAddr<0x%x>\n", __FUNCTION__, (u32)pMsysMem->phys);
                eRet = CAM_OS_FAIL;
                break;
            }
            if(pMsysMem)
            {
                free(pMsysMem);
                pMsysMem = NULL;
            }
            pthread_mutex_lock(&_gtMemLock);
            CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
            {
                ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

                if(ptTmp->pPhysPtr == pPtr || ptTmp->pVirtPtr == pPtr)
                {
                    if(ptTmp->szName)
                        free(ptTmp->szName);
                    CAM_OS_LIST_DEL(ptPos);
                    free(ptTmp);
                }
            }
            pthread_mutex_unlock(&_gtMemLock);
        }
        while(0);

        if(nMsysfd >= 0)
        {
            close(nMsysfd);
        }
    }
#endif
#elif defined(CAM_OS_LINUX_KERNEL)
    MSYS_DMEM_INFO *tpDmem = NULL;
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    if(pPtr)
    {
        do
        {
            _CheckDmemInfoListInited();

            mutex_lock(&_gtMemLock);
            CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
            {
                ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

                //printk("search tmp->ptr: %08X  %s\n", (u32)ptTmp->pVirtPtr, ptTmp->szName);

                if(ptTmp->pPhysPtr == pPtr || ptTmp->pVirtPtr == pPtr)
                {
                    tpDmem = ptTmp->pMemifoPtr;
                    //printk("search(2) pdmem->name: %s\n", tpDmem->name);
                    break;
                }
            }
            mutex_unlock(&_gtMemLock);
            if(tpDmem == NULL)
            {
                printk(KERN_WARNING "%s find node fail <0x%08X>\n", __FUNCTION__, (u32)ptTmp->pVirtPtr);
                eRet = CAM_OS_FAIL;
                break;
            }

            msys_release_dmem(tpDmem);

            if(tpDmem)
            {
                kfree(tpDmem);
                tpDmem = NULL;
            }
            mutex_lock(&_gtMemLock);
            CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
            {
                ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

                if(ptTmp->pPhysPtr == pPtr || ptTmp->pVirtPtr == pPtr)
                {
                    if(ptTmp->szName)
                        kfree(ptTmp->szName);
                    CAM_OS_LIST_DEL(ptPos);
                    kfree(ptTmp);
                }
            }
            mutex_unlock(&_gtMemLock);
        }
        while(0);
    }
#endif
    return eRet;
}

CamOsRet_e CamOsDirectMemFlush(void* pPtr)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    if(pPtr)
    {
        eRet = CAM_OS_FAIL;

        _CheckDmemInfoListInited();

        MsMutexLock(&_gtMemLock);
        CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
        {
            ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

            if(ptTmp->pPhysPtr == pPtr || ptTmp->pVirtPtr == pPtr)
            {
                eRet = CAM_OS_OK;
                sys_flush_data_cache_buffer((u32)MsVA2PA(ptTmp->pVirtPtr), ptTmp->nSize);
            }
        }
        MsMutexUnlock(&_gtMemLock);
    }
#elif defined(CAM_OS_LINUX_USER)
#ifndef NO_MDRV_MSYS
    s32 nMsysFd = -1;

    do
    {
        if(0 > (nMsysFd = open("/dev/msys", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        if(ioctl(nMsysFd, IOCTL_MSYS_FLUSH_MEMORY, 1))
        {
            fprintf(stderr, "%s IOCTL_MSYS_FLUSH_MEMORY Failed!!\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }
    }
    while(0);

    if(nMsysFd >= 0)
    {
        close(nMsysFd);
    }
#endif
#elif defined(CAM_OS_LINUX_KERNEL)
    flush_cache_all();
    Chip_Flush_Memory();
#endif
    return eRet;
}

CamOsRet_e CamOsDirectMemStat(void)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    _CheckDmemInfoListInited();

    MsMutexLock(&_gtMemLock);
    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
    {
        ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

        if(ptTmp->pVirtPtr)
        {
            CamOsPrintf("%s memory allocated %p %s\n", __FUNCTION__, ptTmp->pVirtPtr, ptTmp->szName);
        }
    }
    MsMutexUnlock(&_gtMemLock);
#elif defined(CAM_OS_LINUX_USER)
#ifndef NO_MDRV_MSYS
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    _CheckDmemInfoListInited();

    pthread_mutex_lock(&_gtMemLock);
    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
    {
        ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

        if(ptTmp->pVirtPtr)
        {
            fprintf(stderr, "%s memory allocated 0x%08X %s\n", __FUNCTION__, (u32)ptTmp->pVirtPtr, ptTmp->szName);
        }
    }
    pthread_mutex_unlock(&_gtMemLock);
#endif
#elif defined(CAM_OS_LINUX_KERNEL)
    struct CamOsListHead_t *ptPos, *ptQ;
    MemoryList_t* ptTmp;

    _CheckDmemInfoListInited();

    mutex_lock(&_gtMemLock);
    CAM_OS_LIST_FOR_EACH_SAFE(ptPos, ptQ, &_gtMemList.tList)
    {
        ptTmp = CAM_OS_LIST_ENTRY(ptPos, MemoryList_t, tList);

        if(ptTmp->pVirtPtr)
        {
            printk(KERN_WARNING "%s allocated 0x%08X %s\n", __FUNCTION__, (u32)ptTmp->pVirtPtr, ptTmp->szName);
        }
    }
    mutex_unlock(&_gtMemLock);
#endif
    return eRet;
}

void* CamOsDirectMemPhysToMiu(void* pPtr)
{
#ifdef CAM_OS_RTK
    return (void *)HalUtilPHY2MIUAddr((u32)pPtr);
#elif defined(CAM_OS_LINUX_USER)
    void *nMiuAddr = 0;
#ifndef NO_MDRV_MSYS
    s32 nMsysFd = -1;
    MSYS_ADDR_TRANSLATION_INFO tAddrInfo;

    do
    {
        if(0 > (nMsysFd = open("/dev/msys", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
            break;
        }

        FILL_VERCHK_TYPE(tAddrInfo, tAddrInfo.VerChk_Version, tAddrInfo.VerChk_Size,
                         IOCTL_MSYS_VERSION);

        tAddrInfo.addr = (uintptr_t)pPtr;
        if(ioctl(nMsysFd, IOCTL_MSYS_PHYS_TO_MIU, &tAddrInfo))
        {
            fprintf(stderr, "%s IOCTL_MSYS_PHYS_TO_MIU Failed!!\n", __FUNCTION__);
            break;
        }
        nMiuAddr = (void *)(uintptr_t)tAddrInfo.addr;
    }
    while(0);

    if(nMsysFd >= 0)
    {
        close(nMsysFd);
    }
#endif
    return nMiuAddr;
#elif defined(CAM_OS_LINUX_KERNEL)
    if((u32)pPtr & 0xF0000000)
    {
        pPtr = (void *)(uintptr_t)Chip_Phys_to_MIU((uintptr_t)pPtr);
    }
    else
    {
        CAM_OS_WARN("wrong addr");
        CamOsPrintf("%s input MIU addr 0x%08X\n", __FUNCTION__, (u32)pPtr);
    }

    return pPtr;
#endif
}

void* CamOsDirectMemMiuToPhys(void* pPtr)
{
#ifdef CAM_OS_RTK
    return (void *)HalUtilMIU2PHYAddr((u32)pPtr);
#elif defined(CAM_OS_LINUX_USER)
    void *nPhysAddr = 0;
#ifndef NO_MDRV_MSYS
    s32 nMsysFd = -1;
    MSYS_ADDR_TRANSLATION_INFO tAddrInfo;

    do
    {
        if(0 > (nMsysFd = open("/dev/msys", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
            break;
        }

        FILL_VERCHK_TYPE(tAddrInfo, tAddrInfo.VerChk_Version, tAddrInfo.VerChk_Size,
                         IOCTL_MSYS_VERSION);

        tAddrInfo.addr = (uintptr_t)pPtr;
        if(ioctl(nMsysFd, IOCTL_MSYS_MIU_TO_PHYS, &tAddrInfo))
        {
            fprintf(stderr, "%s IOCTL_MSYS_MIU_TO_PHYS Failed!!\n", __FUNCTION__);
            break;
        }
        nPhysAddr = (void *)(uintptr_t)tAddrInfo.addr;
    }
    while(0);

    if(nMsysFd >= 0)
    {
        close(nMsysFd);
    }
#endif
    return nPhysAddr;
#elif defined(CAM_OS_LINUX_KERNEL)
    if((u32)pPtr & 0xF0000000)
    {
        CAM_OS_WARN("wrong addr");
        CamOsPrintf("%s input PHYS addr 0x%08X\n", __FUNCTION__, (u32)pPtr);
    }
    else
    {
        pPtr = (void *)(uintptr_t)Chip_MIU_to_Phys((uintptr_t)pPtr);
    }

    return pPtr;
#endif
}

void* CamOsDirectMemPhysToVirt(void* pPtr)
{
#ifdef CAM_OS_RTK
    return MsPA2VA(pPtr);
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement PhysToVirt in linux user space.
    CAM_OS_WARN("not support in "OS_NAME);
    return NULL;
#elif defined(CAM_OS_LINUX_KERNEL)
#if 0
    return (void *)phys_to_virt((unsigned long)pPtr);
#else
    MSYS_DMEM_INFO dmem;
    u64 kptr = 0;
    if(0 == msys_find_dmem_by_phys((u64)(u32)pPtr, &dmem))
    {
        kptr = dmem.kvirt;
        kptr += ((u64)(u32)pPtr - dmem.phys);
    }
    else
    {
        CamOsPrintf("CamOs WARNING: PhysToVirt not found 0x%08X in msys\n", (u32)pPtr);
    }

    return (void*)(u32)kptr;
#endif
#endif
}

void* CamOsDirectMemVirtToPhys(void* pPtr)
{
#ifdef CAM_OS_RTK
    return MsVA2PA(pPtr);
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement VirtToPhys in linux user space.
    CAM_OS_WARN("not support in "OS_NAME);
    return NULL;
#elif defined(CAM_OS_LINUX_KERNEL)
    return (void *)virt_to_phys(pPtr);
#endif
}

void* CamOsPhyMemMap(void* pPhyPtr, u32 nSize, u8 bNonCache)
{
#ifdef CAM_OS_RTK
    if (bNonCache)
    {
        return MsPA2VA(pPhyPtr);
    }
    else
    {
        return pPhyPtr;
    }
#elif defined(CAM_OS_LINUX_USER)
    void* pMmapPtr = NULL;
#ifndef NO_MDRV_MSYS
    s32 nMsysFd = -1;
    s32 nMemFd = -1;
    MSYS_ADDR_TRANSLATION_INFO tAddrInfo;
    unsigned long long nCpuBusAddr;

    if((u32)pPhyPtr & 0xF0000000)
    {
        if(0 > (nMsysFd = open("/dev/msys", O_RDWR | O_SYNC)))
        {
            fprintf(stderr, "%s open /dev/msys failed!!\n", __FUNCTION__);
            return NULL;
        }

        FILL_VERCHK_TYPE(tAddrInfo, tAddrInfo.VerChk_Version, tAddrInfo.VerChk_Size,
                         IOCTL_MSYS_VERSION);

        tAddrInfo.addr = (uintptr_t)pPhyPtr;
        if(ioctl(nMsysFd, IOCTL_MSYS_PHYS_TO_MIU, &tAddrInfo))
        {
            fprintf(stderr, "%s IOCTL_MSYS_PHYS_TO_MIU Failed!!\n", __FUNCTION__);
            close(nMsysFd);
            return NULL;
        }
        nCpuBusAddr = (uintptr_t)tAddrInfo.addr;

        close(nMsysFd);
    }
    else
    {
        CAM_OS_WARN("wrong addr");
        CamOsPrintf("%s input MIU addr 0x%08X\n", __FUNCTION__, (u32)pPhyPtr);
        return NULL;
    }

    if(0 > (nMemFd = open("/dev/mem", O_RDWR | O_SYNC)))
    {
        fprintf(stderr, "%s open /dev/mem failed!!\n", __FUNCTION__);
        return NULL;
    }

    pMmapPtr = mmap(0, nSize, PROT_READ | PROT_WRITE, MAP_SHARED, nMemFd, nCpuBusAddr);

    if(nMemFd >= 0)
        close(nMemFd);

    if(pMmapPtr == (void *) - 1)
        return NULL;
#endif
    return pMmapPtr;
#elif defined(CAM_OS_LINUX_KERNEL)
    unsigned long long nCpuBusAddr;
    void *pVirtPtr = NULL;
    int nRet, i, j, k;
    struct sg_table *pSgTable;
    struct scatterlist *pScatterList;
    int nPageCount = 0;
    struct page **ppPages;
    pgprot_t tPgProt;

    if(!nSize)
        return NULL;

    if(bNonCache)
        tPgProt = pgprot_writecombine(PAGE_KERNEL);
    else
        tPgProt = PAGE_KERNEL;

    pSgTable = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
    if(!pSgTable)
    {
        CAM_OS_WARN("kmalloc fail");
        return NULL;
    }

    nRet = sg_alloc_table(pSgTable, 1, GFP_KERNEL);

    if(unlikely(nRet))
    {
        CAM_OS_WARN("sg_alloc_table fail");
        kfree(pSgTable);
        return NULL;
    }

    if((u32)pPhyPtr & 0xF0000000)
    {
        nCpuBusAddr = (uintptr_t)Chip_Phys_to_MIU((uintptr_t)pPhyPtr);
    }
    else
    {
        CAM_OS_WARN("wrong addr");
        CamOsPrintf("%s input MIU addr 0x%08X\n", __FUNCTION__, (u32)pPhyPtr);
    }

    sg_set_page(pSgTable->sgl, pfn_to_page(__phys_to_pfn(nCpuBusAddr)), PAGE_ALIGN(nSize), 0);

    for_each_sg(pSgTable->sgl, pScatterList, pSgTable->nents, i)
    {
        nPageCount += pScatterList->length / PAGE_SIZE;
    }

    ppPages = vmalloc(sizeof(struct page*)*nPageCount);

    if(ppPages == NULL)
    {
        CAM_OS_WARN("vmalloc fail");
        sg_free_table(pSgTable);
        kfree(pSgTable);
        return NULL;
    }

    for_each_sg(pSgTable->sgl, pScatterList, pSgTable->nents, k)
    {
        nPageCount = PAGE_ALIGN(pScatterList->length) / PAGE_SIZE;
        for(j = 0; j < nPageCount; j++)
            ppPages[i + j] = sg_page(pScatterList) + j;
        i += nPageCount;
    }
    pVirtPtr = vmap(ppPages, i, VM_MAP, tPgProt);

    vfree(ppPages);
    sg_free_table(pSgTable);
    kfree(pSgTable);

    return pVirtPtr;
#endif
}

void CamOsPhyMemUnMap(void* pVirtPtr, u32 nSize)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)
    s32 nErr;

    nErr = munmap(pVirtPtr, nSize);
    if(0 != nErr)
    {
        fprintf(stderr, "%s munmap failed!! <0x%08X> size<%d> err<%d> errno<%d, %s>\n",
                __FUNCTION__, (u32)pVirtPtr, nSize, nErr, errno, strerror(errno));
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    vunmap(pVirtPtr);
#endif
}

CamOsRet_e CamOsMemCacheCreate(CamOsMemCache_t *ptMemCache, char *szName, u32 nSize, u8 bHwCacheAlign)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsMemCacheRtk_t *ptHandle = (CamOsMemCacheRtk_t *)ptMemCache;

    if(ptHandle)
    {
        if(RTK_OK == MsFindBestPool(nSize, &ptHandle->nPoolID))
        {
            ptHandle->nObjSize = nSize;
        }
        else
        {
            CAM_OS_WARN("no satisfactory mem pool");
            eRet = CAM_OS_FAIL;
        }
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    struct kmem_cache *ptKmemCache;
    CamOsMemCacheLK_t *ptHandle = (CamOsMemCacheLK_t *)ptMemCache;

    if(ptHandle)
    {
        ptKmemCache = kmem_cache_create(szName, nSize, bHwCacheAlign ? SLAB_HWCACHE_ALIGN : 0, 0, NULL);

        if(ptKmemCache)
        {
            ptHandle->ptKmemCache = ptKmemCache;
        }
        else
        {
            ptHandle->ptKmemCache = NULL;
            eRet = CAM_OS_FAIL;
        }
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#endif
    return eRet;
}

void CamOsMemCacheDestroy(CamOsMemCache_t *ptMemCache)
{
#ifdef CAM_OS_RTK
    CamOsMemCacheRtk_t *ptHandle = (CamOsMemCacheRtk_t *)ptMemCache;

    if(ptHandle)
        memset(ptHandle, 0, sizeof(CamOsMemCacheRtk_t));
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMemCacheLK_t *ptHandle = (CamOsMemCacheLK_t *)ptMemCache;

    if(ptHandle && ptHandle->ptKmemCache)
    {
        kmem_cache_destroy(ptHandle->ptKmemCache);
        ptHandle->ptKmemCache = NULL;
    }
#endif
}

void *CamOsMemCacheAlloc(CamOsMemCache_t *ptMemCache)
{
#ifdef CAM_OS_RTK
    CamOsMemCacheRtk_t *ptHandle = (CamOsMemCacheRtk_t *)ptMemCache;

    if(ptHandle)
        return MsGetPoolMemory(ptHandle->nObjSize);
    else
        return NULL;
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
    return NULL;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMemCacheLK_t *ptHandle = (CamOsMemCacheLK_t *)ptMemCache;

    if(ptHandle && ptHandle->ptKmemCache)
        return kmem_cache_alloc(ptHandle->ptKmemCache, GFP_KERNEL);
    else
        return NULL;
#endif
}

void CamOsMemCacheFree(CamOsMemCache_t *ptMemCache, void *pObjPtr)
{
#ifdef CAM_OS_RTK
    MsReleaseMemory(pObjPtr);
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMemCacheLK_t *ptHandle = (CamOsMemCacheLK_t *)ptMemCache;

    if(ptHandle && ptHandle->ptKmemCache)
        kmem_cache_free(ptHandle->ptKmemCache, pObjPtr);
#endif
}

void CamOsMiuPipeFlush(void)
{
#ifdef CAM_OS_RTK
    DrvChipFlushMiuPipe();
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
#elif defined(CAM_OS_LINUX_KERNEL)
    Chip_Flush_MIU_Pipe();
#endif
}

CamOsRet_e CamOsPropertySet(const char *szKey, const char *szValue)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)
    void *pLibHandle = NULL;
    s32(*dlsym_property_set)(const char *szKey, const char *szValue) = NULL;

    do
    {
        pLibHandle = dlopen("libat.so", RTLD_NOW);
        if(NULL == pLibHandle)
        {
            fprintf(stderr, "%s : load libat.so error\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }

        dlsym_property_set = dlsym(pLibHandle, "property_set");
        if(NULL == dlsym_property_set)
        {
            fprintf(stderr, "%s : dlsym property_set failed, %s\n", __FUNCTION__, dlerror());
            eRet = CAM_OS_FAIL;
            break;
        }

        dlsym_property_set(szKey, szValue);
    }
    while(0);

    if(pLibHandle)
    {
        dlclose(pLibHandle);
    }
#elif defined(CAM_OS_LINUX_KERNEL)

#endif
    return eRet;
}

CamOsRet_e CamOsPropertyGet(const char *szKey, char *szValue, const char *szDefaultValue)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)
    void *pLibHandle = NULL;
    s32(*dlsym_property_get)(const char *szKey, char *szValue, const char *szDefaultValue) = NULL;

    do
    {
        pLibHandle = dlopen("libat.so", RTLD_NOW);
        if(NULL == pLibHandle)
        {
            fprintf(stderr, "%s : load libat.so error\n", __FUNCTION__);
            eRet = CAM_OS_FAIL;
            break;
        }
        dlsym_property_get = dlsym(pLibHandle, "property_get");
        if(NULL == dlsym_property_get)
        {
            fprintf(stderr, "%s : dlsym property_get failed, %s\n", __FUNCTION__, dlerror());
            eRet = CAM_OS_FAIL;
            break;
        }

        dlsym_property_get(szKey, szValue, szDefaultValue);
    }
    while(0);

    if(pLibHandle)
    {
        dlclose(pLibHandle);
    }
#elif defined(CAM_OS_LINUX_KERNEL)

#endif
    return eRet;
}

u64 CamOsMathDivU64(u64 nDividend, u64 nDivisor, u64 *pRemainder)
{
#ifdef CAM_OS_RTK
    return ss_div64_u64_rem(nDividend, nDivisor, pRemainder);
#elif defined(CAM_OS_LINUX_USER)
    *pRemainder = nDividend % nDivisor;
    return nDividend / nDivisor;
#elif defined(CAM_OS_LINUX_KERNEL)
    return div64_u64_rem(nDividend, nDivisor, pRemainder);
#endif
}

s64 CamOsMathDivS64(s64 nDividend, s64 nDivisor, s64 *pRemainder)
{
#ifdef CAM_OS_RTK
    s64 nQuotient = ss_div64_s64(nDividend, nDivisor);
    *pRemainder = nDividend - nDivisor * nQuotient;
    return nQuotient;
#elif defined(CAM_OS_LINUX_USER)
    *pRemainder = nDividend % nDivisor;
    return nDividend / nDivisor;
#elif defined(CAM_OS_LINUX_KERNEL)
    s64 nQuotient = div64_s64(nDividend, nDivisor);
    *pRemainder = nDividend - nDivisor * nQuotient;
    return nQuotient;
#endif
}

u32 CamOsCopyFromUpperLayer(void *pTo, const void *pFrom, u32 nLen)
{
#ifdef CAM_OS_RTK
    memcpy(pTo, pFrom, nLen);
    return 0;
#elif defined(CAM_OS_LINUX_USER)
    memcpy(pTo, pFrom, nLen);
    return 0;
#elif defined(CAM_OS_LINUX_KERNEL)
    return copy_from_user(pTo, pFrom, nLen);
#endif
}

u32 CamOsCopyToUpperLayer(void *pTo, const void * pFrom, u32 nLen)
{
#ifdef CAM_OS_RTK
    memcpy(pTo, pFrom, nLen);
    return 0;
#elif defined(CAM_OS_LINUX_USER)
    memcpy(pTo, pFrom, nLen);
    return 0;
#elif defined(CAM_OS_LINUX_KERNEL)
    return copy_to_user(pTo, pFrom, nLen);
#endif
}

#ifdef CAM_OS_RTK
void _CamOsTimerCallback(MsTimerId_e eTimerID, u32 nHandleAddr)
{
    CamOsTimerRtk_t *ptHandle = (CamOsTimerRtk_t *)nHandleAddr;

    //CamOsPrintf("%s: eTimerID = 0x%x\n", __FUNCTION__, eTimerID);

    if(ptHandle)
    {
        ptHandle->pfnCallback((u32)ptHandle->pDataPtr);
    }
}
#endif

CamOsRet_e CamOsTimerInit(CamOsTimer_t *ptTimer)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTimerRtk_t *ptHandle = (CamOsTimerRtk_t *)ptTimer;
    unsigned long flags = 0;

    if(ptHandle)
    {
        RtkEnterRegionSaveIrq(&flags);
        if(ptHandle->eTimerID != INIT_MAGIC_NUM && MsIsTimerActive(ptHandle->eTimerID))
        {
            MsStopTimer(ptHandle->eTimerID);
        }
        ptHandle->eTimerID = INIT_MAGIC_NUM;
        RtkLeaveRegionRestoreIrq(&flags);
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement timer in linux user space.
    CAM_OS_WARN("not support in "OS_NAME);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTimerLK_t *ptHandle = (CamOsTimerLK_t *)ptTimer;
    if(ptHandle)
    {
        init_timer(&ptHandle->tTimerID);
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#endif
    return eRet;
}

u32 CamOsTimerDelete(CamOsTimer_t *ptTimer)
{
#ifdef CAM_OS_RTK
    CamOsTimerRtk_t *ptHandle = (CamOsTimerRtk_t *)ptTimer;
    unsigned long flags = 0;
    unsigned long nRet = 0;

    if(ptHandle)
    {
        // MsStopTimer return the number of remain ticks, 1 means timeout
        RtkEnterRegionSaveIrq(&flags);
        nRet = (MsStopTimer(ptHandle->eTimerID) > 1) ? 1 : 0;
        RtkLeaveRegionRestoreIrq(&flags);
        return nRet;
    }
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement timer in linux user space.
    CAM_OS_WARN("not support in "OS_NAME);
    return 0;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTimerLK_t *ptHandle = (CamOsTimerLK_t *)ptTimer;
    if(ptHandle)
    {
        return del_timer(&ptHandle->tTimerID);
    }
#endif
    return 0;
}

u32 CamOsTimerDeleteSync(CamOsTimer_t *ptTimer)
{
#ifdef CAM_OS_RTK
    CamOsTimerRtk_t *ptHandle = (CamOsTimerRtk_t *)ptTimer;
    unsigned long flags = 0;
    unsigned long nRet = 0;

    if(ptHandle)
    {
        // MsStopTimer return the number of remain ticks, 1 means timeout
        RtkEnterRegionSaveIrq(&flags);
        nRet = (MsStopTimer(ptHandle->eTimerID) > 1) ? 1 : 0;
        RtkLeaveRegionRestoreIrq(&flags);
        return nRet;
    }
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement timer in linux user space.
    CAM_OS_WARN("not support in "OS_NAME);
    return 0;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTimerLK_t *ptHandle = (CamOsTimerLK_t *)ptTimer;
    if(ptHandle)
    {
        return del_timer_sync(&ptHandle->tTimerID);
    }
#endif
    return 0;
}

CamOsRet_e CamOsTimerAdd(CamOsTimer_t *ptTimer, u32 nMsec, void *pDataPtr,
                         void (*pfnFunc)(unsigned long nDataAddr))
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTimerRtk_t *ptHandle = (CamOsTimerRtk_t *)ptTimer;
    unsigned long flags = 0;
    unsigned long nRet = 0;

    if(ptHandle)
    {
        if(ptHandle->eTimerID != INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("init timer first");
            eRet = CAM_OS_FAIL;
        }

        RtkEnterRegionSaveIrq(&flags);
        ptHandle->pfnCallback = pfnFunc;
        ptHandle->pDataPtr = pDataPtr;
        nRet = MsStartCbTimerMs(&ptHandle->eTimerID, _CamOsTimerCallback, (u32)ptHandle, nMsec, 0);
        RtkLeaveRegionRestoreIrq(&flags);

        if(nRet != CUS_OS_OK)
        {
            CAM_OS_WARN("start timer fail");
            eRet = CAM_OS_FAIL;
        }
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement timer in linux user space.
    CAM_OS_WARN("not support in "OS_NAME);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTimerLK_t *ptHandle = (CamOsTimerLK_t *)ptTimer;
    if(ptHandle)
    {
        ptHandle->tTimerID.expires = jiffies + msecs_to_jiffies(nMsec);
        ptHandle->tTimerID.function = pfnFunc;
        ptHandle->tTimerID.data = (unsigned long)pDataPtr;
        add_timer(&ptHandle->tTimerID);
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#endif
    return eRet;
}

CamOsRet_e CamOsTimerModify(CamOsTimer_t *ptTimer, u32 nMsec)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    CamOsTimerRtk_t *ptHandle = (CamOsTimerRtk_t *)ptTimer;
    unsigned long flags = 0;
    unsigned long nRet = 0;

    if(ptHandle)
    {
        if(ptHandle->eTimerID == INIT_MAGIC_NUM)
        {
            CAM_OS_WARN("add timer first");
            eRet = CAM_OS_FAIL;
        }

        RtkEnterRegionSaveIrq(&flags);
        MsStopTimer(ptHandle->eTimerID);
        nRet = MsStartCbTimerMs(&ptHandle->eTimerID, _CamOsTimerCallback, (u32)ptHandle, nMsec, 0);
        RtkLeaveRegionRestoreIrq(&flags);

        if(nRet != CUS_OS_OK)
        {
            CAM_OS_WARN("start timer fail");
            eRet = CAM_OS_FAIL;
        }
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#elif defined(CAM_OS_LINUX_USER)
    // TODO: implement timer in linux user space.
    CAM_OS_WARN("not support in "OS_NAME);
    eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsTimerLK_t *ptHandle = (CamOsTimerLK_t *)ptTimer;
    if(ptHandle)
    {
        mod_timer(&ptHandle->tTimerID, jiffies + msecs_to_jiffies(nMsec));
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#endif
    return eRet;
}

s32 CamOsAtomicRead(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return ptAtomic->nCounter;
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return ptAtomic->nCounter;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_read((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

void CamOsAtomicSet(CamOsAtomic_t *ptAtomic, s32 nValue)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        ptAtomic->nCounter = nValue;
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        ptAtomic->nCounter = nValue;
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        atomic_set((atomic_t *)ptAtomic, nValue);
    }
#endif
}

s32 CamOsAtomicAddReturn(CamOsAtomic_t *ptAtomic, s32 nValue)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return __sync_add_and_fetch(&ptAtomic->nCounter, nValue);
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return __sync_add_and_fetch(&ptAtomic->nCounter, nValue);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_add_return(nValue, (atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicSubReturn(CamOsAtomic_t *ptAtomic, s32 nValue)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return __sync_sub_and_fetch(&ptAtomic->nCounter, nValue);
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return __sync_sub_and_fetch(&ptAtomic->nCounter, nValue);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_sub_return(nValue, (atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicSubAndTest(CamOsAtomic_t *ptAtomic, s32 nValue)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return !(__sync_sub_and_fetch(&ptAtomic->nCounter, nValue));
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return !(__sync_sub_and_fetch(&ptAtomic->nCounter, nValue));
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_sub_and_test(nValue, (atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicIncReturn(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return __sync_add_and_fetch(&ptAtomic->nCounter, 1);
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return __sync_add_and_fetch(&ptAtomic->nCounter, 1);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_inc_return((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicDecReturn(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return __sync_sub_and_fetch(&ptAtomic->nCounter, 1);
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return __sync_sub_and_fetch(&ptAtomic->nCounter, 1);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_dec_return((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicIncAndTest(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return !(__sync_add_and_fetch(&ptAtomic->nCounter, 1));
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return !(__sync_add_and_fetch(&ptAtomic->nCounter, 1));
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_inc_and_test((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicDecAndTest(CamOsAtomic_t *ptAtomic)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return !(__sync_sub_and_fetch(&ptAtomic->nCounter, 1));
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return !(__sync_sub_and_fetch(&ptAtomic->nCounter, 1));
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_dec_and_test((atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicAddNegative(CamOsAtomic_t *ptAtomic, s32 nValue)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return (__sync_add_and_fetch(&ptAtomic->nCounter, nValue) < 0);
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return (__sync_add_and_fetch(&ptAtomic->nCounter, nValue) < 0);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_add_negative(nValue, (atomic_t *)ptAtomic);
    }
#endif
    return 0;
}

s32 CamOsAtomicCompareAndSwap(CamOsAtomic_t *ptAtomic, s32 nOldValue, s32 nNewValue)
{
#ifdef CAM_OS_RTK
    if(ptAtomic)
    {
        return __sync_bool_compare_and_swap(&ptAtomic->nCounter, nOldValue, nNewValue);
    }
#elif defined(CAM_OS_LINUX_USER)
    if(ptAtomic)
    {
        return __sync_bool_compare_and_swap(&ptAtomic->nCounter, nOldValue, nNewValue);
    }
#elif defined(CAM_OS_LINUX_KERNEL)
    if(ptAtomic)
    {
        return atomic_cmpxchg( (atomic_t *)ptAtomic, nOldValue,  nNewValue);    // return TRUE: CAS doing,
    }
#endif
    return 0;
}

s32 CamOsAtomicAndFetch(CamOsAtomic_t *ptAtomic, s32 nValue)
{
    if(ptAtomic)
    {
        return __sync_and_and_fetch(&ptAtomic->nCounter, nValue);
    }
    else
    {
        CAM_OS_WARN("null atomic handle");
        return 0;
    }
}

s32 CamOsAtomicFetchAnd(CamOsAtomic_t *ptAtomic, s32 nValue)
{
    if(ptAtomic)
    {
        return __sync_fetch_and_and(&ptAtomic->nCounter, nValue);
    }
    else
    {
        CAM_OS_WARN("null atomic handle");
        return 0;
    }
}

s32 CamOsAtomicNandFetch(CamOsAtomic_t *ptAtomic, s32 nValue)
{
    if(ptAtomic)
    {
        return __sync_nand_and_fetch(&ptAtomic->nCounter, nValue);
    }
    else
    {
        CAM_OS_WARN("null atomic handle");
        return 0;
    }
}

s32 CamOsAtomicFetchNand(CamOsAtomic_t *ptAtomic, s32 nValue)
{
    if(ptAtomic)
    {
        return __sync_fetch_and_nand(&ptAtomic->nCounter, nValue);
    }
    else
    {
        CAM_OS_WARN("null atomic handle");
        return 0;
    }
}

s32 CamOsAtomicOrFetch(CamOsAtomic_t *ptAtomic, s32 nValue)
{
    if(ptAtomic)
    {
        return __sync_or_and_fetch(&ptAtomic->nCounter, nValue);
    }
    else
    {
        CAM_OS_WARN("null atomic handle");
        return 0;
    }
}

s32 CamOsAtomicFetchOr(CamOsAtomic_t *ptAtomic, s32 nValue)
{
    if(ptAtomic)
    {
        return __sync_fetch_and_or(&ptAtomic->nCounter, nValue);
    }
    else
    {
        CAM_OS_WARN("null atomic handle");
        return 0;
    }
}

s32 CamOsAtomicXorFetch(CamOsAtomic_t *ptAtomic, s32 nValue)
{
    if(ptAtomic)
    {
        return __sync_xor_and_fetch(&ptAtomic->nCounter, nValue);
    }
    else
    {
        CAM_OS_WARN("null atomic handle");
        return 0;
    }
}

s32 CamOsAtomicFetchXor(CamOsAtomic_t *ptAtomic, s32 nValue)
{
    if(ptAtomic)
    {
        return __sync_fetch_and_xor(&ptAtomic->nCounter, nValue);
    }
    else
    {
        CAM_OS_WARN("null atomic handle");
        return 0;
    }
}

#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
extern CamOsRet_e _CamOsIdrInit(CamOsIdr_t *ptIdr, u32 nEntryNum);
extern void _CamOsIdrDestroy(CamOsIdr_t *ptIdr);
extern s32 _CamOsIdrAlloc(CamOsIdr_t *ptIdr, void *pPtr, s32 nStart, s32 nEnd);
extern void _CamOsIdrRemove(CamOsIdr_t *ptIdr, s32 nId);
extern void *_CamOsIdrFind(CamOsIdr_t *ptIdr, s32 nId);
#endif

CamOsRet_e CamOsIdrInit(CamOsIdr_t *ptIdr)
{
    CamOsRet_e eRet = CAM_OS_OK;
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    eRet = _CamOsIdrInit(ptIdr, 0);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsIdrLK_t *ptHandle = (CamOsIdrLK_t *)ptIdr;

    if(ptHandle)
    {
        idr_init(&ptHandle->tIdr);
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#endif
    return eRet;
}

CamOsRet_e CamOsIdrInitEx(CamOsIdr_t *ptIdr, u32 nEntryNum)
{
    CamOsRet_e eRet = CAM_OS_OK;
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    eRet = _CamOsIdrInit(ptIdr, nEntryNum);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsIdrLK_t *ptHandle = (CamOsIdrLK_t *)ptIdr;

    if(ptHandle)
    {
        idr_init(&ptHandle->tIdr);
    }
    else
        eRet = CAM_OS_PARAM_ERR;
#endif
    return eRet;
}

void CamOsIdrDestroy(CamOsIdr_t *ptIdr)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    _CamOsIdrDestroy(ptIdr);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsIdrLK_t *ptHandle = (CamOsIdrLK_t *)ptIdr;

    if(ptHandle)
    {
        idr_destroy(&ptHandle->tIdr);
    }
#endif
}

s32 CamOsIdrAlloc(CamOsIdr_t *ptIdr, void *pPtr, s32 nStart, s32 nEnd)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    return _CamOsIdrAlloc(ptIdr, pPtr, nStart, nEnd);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsIdrLK_t *ptHandle = (CamOsIdrLK_t *)ptIdr;

    if(ptHandle)
    {
        return idr_alloc(&ptHandle->tIdr, pPtr, nStart, nEnd, GFP_KERNEL);
    }
    else
        return -1;
#endif
}

void CamOsIdrRemove(CamOsIdr_t *ptIdr, s32 nId)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    _CamOsIdrRemove(ptIdr, nId);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsIdrLK_t *ptHandle = (CamOsIdrLK_t *)ptIdr;

    if(ptHandle)
    {
        idr_remove(&ptHandle->tIdr, nId);
    }
#endif
}

void *CamOsIdrFind(CamOsIdr_t *ptIdr, s32 nId)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    return _CamOsIdrFind(ptIdr, nId);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsIdrLK_t *ptHandle = (CamOsIdrLK_t *)ptIdr;

    if(ptHandle)
    {
        return idr_find(&ptHandle->tIdr, nId);
    }
    else
        return NULL;
#endif
}


CamOsMemSize_e CamOsPhysMemSize(void)
{
#ifdef CAM_OS_RTK
    return (CamOsMemSize_e)((*(volatile u32 *)(RIU_BASE_ADDR + RIU_MEM_SIZE_OFFSET) & 0xF000) >> 12);
#elif defined(CAM_OS_LINUX_USER)
    void *map_addr = 0;
    s32 nMemFd = -1;
    u32 nRegs = 0;
    u32 nPageSize;

    nPageSize = getpagesize();

    do
    {
        if(0 > (nMemFd = open("/dev/mem", O_RDWR | O_SYNC)))
        {
            CAM_OS_WARN("open /dev/mem fail");
            break;
        }

        map_addr = mmap(0, nPageSize, PROT_READ | PROT_WRITE, MAP_SHARED, nMemFd, (RIU_BASE_ADDR + RIU_MEM_SIZE_OFFSET) & ~(nPageSize - 1));

        if(map_addr == MAP_FAILED)
        {
            CAM_OS_WARN("mmap fail");
            break;
        }
        else
        {
            nRegs = *(u32 *)(map_addr + RIU_MEM_SIZE_OFFSET % nPageSize);
            if(munmap(map_addr, nPageSize))
            {
                CamOsPrintf("%s: mumap %p is error(%s)\n", __func__,  map_addr, strerror(errno));
            }
        }
    }
    while(0);

    if(nMemFd >= 0)
    {
        close(nMemFd);
    }

    //CamOsPrintf("CamOsPhysMemSize: %d\n", (nRegs & 0xF000) >> 12);

    return (CamOsMemSize_e)((nRegs & 0xF000) >> 12);
#elif defined(CAM_OS_LINUX_KERNEL)
    CamOsMemSize_e eMemSize=CAM_OS_MEM_UNKNOWN;
    void *pRegs;

    if((pRegs = ioremap(RIU_BASE_ADDR + RIU_MEM_SIZE_OFFSET, 4)) != NULL)
    {
        eMemSize = (CamOsMemSize_e)((readl(pRegs) & 0xF000) >> 12);
        iounmap(pRegs);
    }
    else
    {
        CAM_OS_WARN("ioremap fail");
    }

    return eMemSize;
#endif
}

u32 CamOsChipId(void)
{
#ifdef CAM_OS_RTK
    return *(volatile u32 *)(RIU_BASE_ADDR + RIU_CHIP_ID_OFFSET) & 0x3FFFFF;
#elif defined(CAM_OS_LINUX_USER)
    void *map_addr = 0;
    s32 nMemFd = -1;
    u32 nRegs = 0;
    u32 nPageSize;

    nPageSize = getpagesize();

    do
    {
        if(0 > (nMemFd = open("/dev/mem", O_RDWR | O_SYNC)))
        {
            CAM_OS_WARN("open /dev/mem fail");
            break;
        }

        map_addr = mmap(0, nPageSize, PROT_READ | PROT_WRITE, MAP_SHARED, nMemFd, (RIU_BASE_ADDR + RIU_CHIP_ID_OFFSET) & ~(nPageSize - 1));

        if(map_addr == MAP_FAILED)
        {
            CAM_OS_WARN("mmap fail");
            break;
        }
        else
        {
            nRegs = *(u32 *)(map_addr + RIU_CHIP_ID_OFFSET % nPageSize);
            if(munmap(map_addr, nPageSize))
            {
                CamOsPrintf("%s: mumap %p is error(%s)\n", __func__,  map_addr, strerror(errno));
            }
        }
    }
    while(0);

    if(nMemFd >= 0)
    {
        close(nMemFd);
    }

    //CamOsPrintf("CamOsChipId: %d\n", nRegs & 0x3FFFFF);

    return nRegs & 0x3FFFFF;
#elif defined(CAM_OS_LINUX_KERNEL)
    u32 nId=0;
    void *pRegs;

    if((pRegs = ioremap(RIU_BASE_ADDR + RIU_CHIP_ID_OFFSET, 4)) != NULL)
    {
        nId = readl(pRegs) & 0x3FFFFF;
        iounmap(pRegs);
    }
    else
    {
        CAM_OS_WARN("ioremap fail");
    }

    return nId;
#endif
}

CamOsRet_e CamOsIrqRequest(u32 nIrq, CamOsIrqHandler pfnHandler, const char *szName, void *pDevId)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    IntInitParam_u uInitParam = {{0}};
    uInitParam.intc.eMap = INTC_MAP_IRQ;
    uInitParam.intc.ePriority = INTC_PRIORITY_7;
    uInitParam.intc.pfnIsr = pfnHandler;
    uInitParam.intc.pDevId = pDevId;
    DrvInitInterrupt(&uInitParam, nIrq);
    DrvUnmaskInterrupt(nIrq);
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
#elif defined(CAM_OS_LINUX_KERNEL)
    s32 nResult = 0;

    if (nIrq < LINUX_KERNEL_MAX_IRQ)
    {
        CamOsIrqHandlerList[nIrq] = pfnHandler;
        if ((nResult = request_irq(nIrq, (irq_handler_t)CamOsIrqCommonHandler, IRQF_SHARED | IRQF_ONESHOT, szName, pDevId)))
        {
            CAM_OS_WARN("request_irq fail");
            CamOsPrintf("%s irq(%d) err(%d)\n", __FUNCTION__ , nIrq, nResult);
            eRet = CAM_OS_FAIL;
        }
    }
    else
    {
        eRet = CAM_OS_FAIL;
    }
#endif
    return eRet;
}

void CamOsIrqFree(u32 nIrq, void *pDevId)
{
#ifdef CAM_OS_RTK
    return;
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
#elif defined(CAM_OS_LINUX_KERNEL)
    free_irq(nIrq, pDevId);
#endif
}

void CamOsIrqEnable(u32 nIrq)
{
#ifdef CAM_OS_RTK
    DrvUnmaskInterrupt(nIrq);
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
#elif defined(CAM_OS_LINUX_KERNEL)
    enable_irq(nIrq);
#endif
}

void CamOsIrqDisable(u32 nIrq)
{
#ifdef CAM_OS_RTK
    DrvMaskInterrupt(nIrq);
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
#elif defined(CAM_OS_LINUX_KERNEL)
    disable_irq(nIrq);
#endif
}

CamOsRet_e FORCE_INLINE CamOsInInterrupt(void)
{
    CamOsRet_e eRet = CAM_OS_OK;
#ifdef CAM_OS_RTK
    if(!RtkRunInIsrContext())
        eRet = CAM_OS_FAIL;
#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
#elif defined(CAM_OS_LINUX_KERNEL)
    if(!in_interrupt())
        eRet = CAM_OS_FAIL;
#endif
    return eRet;
}

void FORCE_INLINE CamOsMemoryBarrier(void)
{
    asm volatile("": : :"memory");
}

void FORCE_INLINE CamOsSmpMemoryBarrier(void)
{
#ifdef CAM_OS_RTK

#elif defined(CAM_OS_LINUX_USER)
    CAM_OS_WARN("not support in "OS_NAME);
#elif defined(CAM_OS_LINUX_KERNEL)
    smp_mb();
#endif
}

#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_KERNEL)
char _szErrStrBuf[128];
#endif

char *CamOsStrError(s32 nErrNo)
{
#ifdef CAM_OS_RTK
    sprintf(_szErrStrBuf, "errno: %d", nErrNo);
    return _szErrStrBuf;
#elif defined(CAM_OS_LINUX_USER)
    return strerror(nErrNo);
#elif defined(CAM_OS_LINUX_KERNEL)
    sprintf(_szErrStrBuf, "errno: %d", nErrNo);
    return _szErrStrBuf;
#endif
}

void CamOsPanic(const char *szMessage)
{
#ifdef CAM_OS_RTK
    CamOsPrintf("%s: %s\n", __FUNCTION__, szMessage);
    RtkExceptionRoutine(240, 0);    // SYSTEM_ASSERT = 240
#elif defined(CAM_OS_LINUX_USER)
    CamOsPrintf("%s: %s\n", __FUNCTION__, szMessage);
    abort();
#elif defined(CAM_OS_LINUX_KERNEL)
    panic(szMessage);
#endif
}

long CamOsStrtol(const char *szStr, char** szEndptr, s32 nBase)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    return strtol(szStr, szEndptr, nBase);
#elif defined(CAM_OS_LINUX_KERNEL)
    return simple_strtol(szStr, szEndptr, nBase);
#endif
}

unsigned long CamOsStrtoul(const char *szStr, char** szEndptr, s32 nBase)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    return strtoul(szStr, szEndptr, nBase);
#elif defined(CAM_OS_LINUX_KERNEL)
    return simple_strtoul(szStr, szEndptr, nBase);
#endif
}

unsigned long long CamOsStrtoull(const char *szStr, char** szEndptr, s32 nBase)
{
#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
    return strtoull(szStr, szEndptr, nBase);
#elif defined(CAM_OS_LINUX_KERNEL)
    return simple_strtoull(szStr, szEndptr, nBase);
#endif
}

unsigned long _CamOsFindFirstZeroBit(unsigned long *pAddr, unsigned long nSize,
                                     unsigned long nOffset)
{
    unsigned long *pLongBitmap = pAddr + (nOffset / CAM_OS_BITS_PER_LONG);
    unsigned long nResult = nOffset & ~(CAM_OS_BITS_PER_LONG - 1), nTemp;

    if(nOffset >= nSize)
        return nSize;

    nSize -= nResult;
    nOffset %= CAM_OS_BITS_PER_LONG;
    if(nOffset)
    {
        nTemp = *(pLongBitmap++);
        nTemp |= ~0UL >> (CAM_OS_BITS_PER_LONG - nOffset);
        if(nSize < CAM_OS_BITS_PER_LONG)
            goto IN_FIRST_BIT;

        if(~nTemp)
            goto IN_OTHER_BIT;

        nSize -= CAM_OS_BITS_PER_LONG;
        nResult += CAM_OS_BITS_PER_LONG;
    }

    //while (nSize > CAM_OS_BITS_PER_LONG)
    while(nSize & ~(CAM_OS_BITS_PER_LONG - 1))
    {
        if(~(nTemp = *pLongBitmap))
            goto IN_OTHER_BIT;

        nResult += CAM_OS_BITS_PER_LONG;
        nSize -= CAM_OS_BITS_PER_LONG;
        pLongBitmap++;
    }

    if(!nSize)
        return nResult;

    nTemp = *pLongBitmap;

IN_FIRST_BIT:
    nTemp |= ~0UL << nSize;
    if(nTemp == ~0UL)
        return nResult + nSize;

IN_OTHER_BIT:
    return nResult + CAM_OS_FFZ(nTemp);
}

static struct CamOsListHead_t *_CamOsListMerge(void *priv,
        int (*cmp)(void *priv, struct CamOsListHead_t *a,
                   struct CamOsListHead_t *b),
        struct CamOsListHead_t *a, struct CamOsListHead_t *b)
{
    struct CamOsListHead_t head, *tail = &head;

    while(a && b)
    {
        /* if equal, take 'a' -- important for sort stability */
        if((*cmp)(priv, a, b) <= 0)
        {
            tail->pNext = a;
            a = a->pNext;
        }
        else
        {
            tail->pNext = b;
            b = b->pNext;
        }
        tail = tail->pNext;
    }
    tail->pNext = a ? : b;
    return head.pNext;
}

static void _CamOsListMergeAndRestoreBackLinks(void *priv,
        int (*cmp)(void *priv, struct CamOsListHead_t *a,
                   struct CamOsListHead_t *b),
        struct CamOsListHead_t *head,
        struct CamOsListHead_t *a, struct CamOsListHead_t *b)
{
    struct CamOsListHead_t *tail = head;
    u8 count = 0;

    while(a && b)
    {
        /* if equal, take 'a' -- important for sort stability */
        if((*cmp)(priv, a, b) <= 0)
        {
            tail->pNext = a;
            a->pPrev = tail;
            a = a->pNext;
        }
        else
        {
            tail->pNext = b;
            b->pPrev = tail;
            b = b->pNext;
        }
        tail = tail->pNext;
    }
    tail->pNext = a ? : b;

    do
    {
        /*
         * In worst cases this loop may run many iterations.
         * Continue callbacks to the client even though no
         * element comparison is needed, so the client's cmp()
         * routine can invoke cond_resched() periodically.
         */
        if(!(++count))
            (*cmp)(priv, tail->pNext, tail->pNext);

        tail->pNext->pPrev = tail;
        tail = tail->pNext;
    }
    while(tail->pNext);

    tail->pNext = head;
    head->pPrev = tail;
}

void CamOsListSort(void *priv, struct CamOsListHead_t *head,
                   int (*cmp)(void *priv, struct CamOsListHead_t *a,
                              struct CamOsListHead_t *b))
{
    struct CamOsListHead_t *part[CAM_OS_MAX_LIST_LENGTH_BITS + 1]; /* sorted partial lists
						-- last slot is a sentinel */
    int lev;  /* index into part[] */
    int max_lev = 0;
    struct CamOsListHead_t *list;

    if(CAM_OS_LIST_EMPTY(head))
        return;

    memset(part, 0, sizeof(part));

    head->pPrev->pNext = NULL;
    list = head->pNext;

    while(list)
    {
        struct CamOsListHead_t *cur = list;
        list = list->pNext;
        cur->pNext = NULL;

        for(lev = 0; part[lev]; lev++)
        {
            cur = _CamOsListMerge(priv, cmp, part[lev], cur);
            part[lev] = NULL;
        }
        if(lev > max_lev)
        {
            if(lev >= CAM_OS_ARRAY_SIZE(part) - 1)
            {
                CAM_OS_WARN("list too long");
                lev--;
            }
            max_lev = lev;
        }
        part[lev] = cur;
    }

    for(lev = 0; lev < max_lev; lev++)
        if(part[lev])
            list = _CamOsListMerge(priv, cmp, part[lev], list);

    _CamOsListMergeAndRestoreBackLinks(priv, cmp, head, part[max_lev], list);
}
