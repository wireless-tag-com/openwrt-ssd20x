/*
* cam_os_wrapper_test.c- Sigmastar
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


#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "cam_os_wrapper.h"

#ifdef CAM_OS_RTK
#include "sys_sys_core.h"
#include "sys_sys_isw_cli.h"
#endif

static u32 _gMutexTestCnt = 0;
static u32 _gTestCnt = 0;
static void _TestCamOsThread(void);
static void _TestCamOsThreadPriority(void);
static void _TestCamOsMutex(void);
static void _TestCamOsDmem(void);
static void _TestCamOsMem(void);
static void _TestCamOsPhyMapVirt(void);
static void _TestCamOsTimer(void);
static void _TestCamOsTsem(void);
static void _TestCamOsRwsem(void);
static void _TestCamOsDiv64(void);
static void _TestCamOsSystemTime(void);
static void _TestCamOsPhysMemSize(void);
static void _TestCamOsChipId(void);
static void _TestCamOsTcond(void);
static void _TestCamOsBitmap(void);
static void _TestCamOsHash(void);
static void _TestCamOsIdr(void);
#ifdef CAM_OS_RTK
static void _TestCamOsTimerCallback(void);
static void _TestCamOsCacheAlloc(void);
#endif
static void _TestCamOsPanic(void);
static void _TestCamOsSpinlock(void);

typedef void (*_TEST_FUNC_PTR)(void);

typedef struct
{
    _TEST_FUNC_PTR pFnTest;
    u8             *pTestDesc;
} CamOsWrapperTestItem_t, *pCamOsWrapperTestItem;

static const CamOsWrapperTestItem_t aCamOsWrapperTestItemTbl[] =
{
    {_TestCamOsThread,          (u8*)"test thread function"},
    {_TestCamOsThreadPriority,  (u8*)"test thread priority function"},
    {_TestCamOsMutex,           (u8*)"test mutex function"},
    {_TestCamOsDmem,            (u8*)"test direct memory allocation"},
    {_TestCamOsMem,             (u8*)"test memory allocation"},
    {_TestCamOsPhyMapVirt,      (u8*)"test physical memory map to virtual address"},
    {_TestCamOsTimer,           (u8*)"test timer function"},
    {_TestCamOsTsem,            (u8*)"test thread semaphore function"},
    {_TestCamOsRwsem,           (u8*)"test rw semaphore function"},
    {_TestCamOsTcond,           (u8*)"test thread condition wait function"},
    {_TestCamOsBitmap,          (u8*)"test bitmap operation"},
    {_TestCamOsHash,            (u8*)"test hash operation"},
    {_TestCamOsIdr,             (u8*)"test IDR operation"},
    {_TestCamOsDiv64,           (u8*)"test 64-bit division"},
    {_TestCamOsSystemTime,      (u8*)"test system time function"},
    {_TestCamOsPhysMemSize,     (u8*)"test physical memory size"},
    {_TestCamOsChipId,          (u8*)"test chip ID"},
#ifdef CAM_OS_RTK
    {_TestCamOsTimerCallback,   (u8*)"test timer callback function"},
    {_TestCamOsCacheAlloc,      (u8*)"test cache memory allocation"},
#endif
    {_TestCamOsPanic,           (u8*)"test panic"},
    {_TestCamOsSpinlock,        (u8*)"test spinlock"},
    {NULL,                      (u8*)"test all function"},
};

#define TEST_ITEM_NUM (sizeof(aCamOsWrapperTestItemTbl)/sizeof(CamOsWrapperTestItem_t))

static void _CamOsWrapperShowTestMenu(void)
{
    s32 i;

    CamOsPrintf("cam_os_wrapper test menu: \n");

    for (i = 0; i < TEST_ITEM_NUM; i++)
    {
        CamOsPrintf("\t%2d) %s\r\n", i, aCamOsWrapperTestItemTbl[i].pTestDesc);
    }
}

#ifdef CAM_OS_RTK
s32 CamOsWrapperTest(CLI_t *pCli, char *p)
{
    s32 i, nParamCnt;
    u32  nCaseNum = 0;
    char *pEnd;


    nParamCnt = CliTokenCount(pCli);

    if(nParamCnt < 1)
    {
        _CamOsWrapperShowTestMenu();
        return eCLI_PARSE_INPUT_ERROR;
    }

    for(i = 0; i < nParamCnt; i++)
    {
        pCli->tokenLvl++;
        p = CliTokenPop(pCli);
        if(i == 0)
        {
            //CLIDEBUG(("p: %s, len: %d\n", p, strlen(p)));
            //*pV = _strtoul(p, &pEnd, base);
            nCaseNum = strtoul(p, &pEnd, 10);
            //CLIDEBUG(("*pEnd = %d\n", *pEnd));
            if(p == pEnd || *pEnd)
            {
                CamOsPrintf("Invalid input\n");
                return eCLI_PARSE_ERROR;
            }
        }
    }

    if (nCaseNum < TEST_ITEM_NUM)
    {
        if (nCaseNum == (TEST_ITEM_NUM - 1))
        {
            for (i = 0; i < TEST_ITEM_NUM; i++)
            {
                if (aCamOsWrapperTestItemTbl[i].pFnTest)
                {
                    CamOsPrintf("===============================================\r\n");
                    CamOsPrintf("%s\r\n", aCamOsWrapperTestItemTbl[i].pTestDesc);
                    CamOsPrintf("===============================================\r\n");
                    aCamOsWrapperTestItemTbl[i].pFnTest();
                    CamOsPrintf("\r\n");
                }
            }
        }
        else
        {
            if (aCamOsWrapperTestItemTbl[nCaseNum].pFnTest)
            {
                CamOsPrintf("===============================================\r\n");
                CamOsPrintf("%s\r\n", aCamOsWrapperTestItemTbl[nCaseNum].pTestDesc);
                CamOsPrintf("===============================================\r\n");
                aCamOsWrapperTestItemTbl[nCaseNum].pFnTest();
            }
        }
    }
    else
    {
        _CamOsWrapperShowTestMenu();
        return eCLI_PARSE_ERROR;
    }

    return eCLI_PARSE_OK;
}
#else
int main(int argc, char *argv[])
{
    u32 nCaseNum = 0;
    s32 i;

    if(argc < 2)
    {
        _CamOsWrapperShowTestMenu();
        return -1;
    }

    nCaseNum = atoi(argv[1]);

    if (nCaseNum < TEST_ITEM_NUM)
    {
        if (nCaseNum == (TEST_ITEM_NUM - 1))
        {
            for (i = 0; i < TEST_ITEM_NUM; i++)
            {
                if (aCamOsWrapperTestItemTbl[i].pFnTest)
                {
                    CamOsPrintf("===============================================\r\n");
                    CamOsPrintf("%s\r\n", aCamOsWrapperTestItemTbl[i].pTestDesc);
                    CamOsPrintf("===============================================\r\n");
                    aCamOsWrapperTestItemTbl[i].pFnTest();
                    CamOsPrintf("\r\n");
                }
            }
        }
        else
        {
            if (aCamOsWrapperTestItemTbl[nCaseNum].pFnTest)
            {
                CamOsPrintf("===============================================\r\n");
                CamOsPrintf("%s\r\n", aCamOsWrapperTestItemTbl[nCaseNum].pTestDesc);
                CamOsPrintf("===============================================\r\n");
                aCamOsWrapperTestItemTbl[nCaseNum].pFnTest();
            }
        }
    }
    else
    {
        _CamOsWrapperShowTestMenu();
        return -1;
    }

    return 0;
}
#endif

CamOsAtomic_t _gtThreadAtomic;

static void _CamOsThreadTestEntry0(void *pUserdata)
{
    s32 *pnArg = (s32 *)pUserdata;
    s32 i = 0;

    for(i = 0; i < 5; i++)
    {
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _CamOsThreadTestEntry0 (sleep %dms)  count: %d\n", __LINE__, *pnArg, i);
        CamOsMsSleep(*pnArg);
    }
}

static int _CamOsThreadTestEntry1(void *pUserData)
{
    s32 nTestCounter=0;

    CamOsMsSleep(10);

    while (nTestCounter < 10000)
    {
        nTestCounter++;
        if ((nTestCounter % 500) == 0)
            CamOsPrintf("%s run...\n", __FUNCTION__);
    }

    CamOsAtomicIncReturn(&_gtThreadAtomic);
    while (CamOsAtomicRead(&_gtThreadAtomic) < 3)
        CamOsMsSleep(100);

    CamOsPrintf("%s free run end\n", __FUNCTION__);
#ifdef CAM_OS_RTK
    while (CamOsAtomicRead(&_gtThreadAtomic) < 4)
    {
        if (CamOsAtomicRead(&_gtThreadAtomic) == 3)
        {
            CamOsPrintf("%s enter CamOsThreadSchedule\n", __FUNCTION__);
            CamOsThreadSchedule(0, 2000);
            CamOsPrintf("%s leave CamOsThreadSchedule\n", __FUNCTION__);
            CamOsAtomicIncReturn(&_gtThreadAtomic);
        }

        CamOsPrintf("%s run...\n", __FUNCTION__);
        CamOsMsSleep(300);
    }
#endif
    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static int _CamOsThreadTestEntry2(void *pUserData)
{
    s32 nTestCounter=0;

    CamOsMsSleep(10);

    while (nTestCounter < 10000)
    {
        nTestCounter++;
        if ((nTestCounter % 500) == 0)
            CamOsPrintf("%s run...\n", __FUNCTION__);
    }

    CamOsAtomicIncReturn(&_gtThreadAtomic);
    while (CamOsAtomicRead(&_gtThreadAtomic) < 3)
        CamOsMsSleep(100);

    CamOsPrintf("%s free run end\n", __FUNCTION__);
#ifdef CAM_OS_RTK
    while (CamOsAtomicRead(&_gtThreadAtomic) < 6)
    {
        if (CamOsAtomicRead(&_gtThreadAtomic) == 4)
        {
            CamOsPrintf("%s enter CamOsThreadSchedule\n", __FUNCTION__);
            CamOsThreadSchedule(1, CAM_OS_MAX_TIMEOUT);
            CamOsPrintf("%s leave CamOsThreadSchedule\n", __FUNCTION__);
            CamOsAtomicIncReturn(&_gtThreadAtomic);
        }
        CamOsPrintf("%s run...\n", __FUNCTION__);
        CamOsMsSleep(300);
    }
#endif
    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static void _TestCamOsThread(void)
{
    CamOsThread TaskHandle0, TaskHandle1;
    CamOsThreadAttrb_t tAttr = {0};
    s32 nTaskArg0 = 1000, nTaskArg1 = 1500;
    char szTaskName0[32], szTaskName1[32];

    tAttr.nPriority = 50;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle0, &tAttr, (void *)_CamOsThreadTestEntry0, (void *)&nTaskArg0);
    //CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsThread get taskid: %d\n", __LINE__, TaskHandle0.eHandleObj);

    tAttr.nPriority = 50;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle1, &tAttr, (void *)_CamOsThreadTestEntry0, (void *)&nTaskArg1);
    //CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsThread get taskid: %d\n", __LINE__, TaskHandle1.eHandleObj);

    CamOsMsSleep(1000);
    CamOsThreadGetName(TaskHandle0, szTaskName0, sizeof(szTaskName0));
    CamOsThreadGetName(TaskHandle1, szTaskName1, sizeof(szTaskName1));
    CamOsPrintf("Test Get Name: %s %s\n", szTaskName0, szTaskName1);

    CamOsMsSleep(1000);
    CamOsThreadSetName(TaskHandle0, "ABCDEFGHIJKLMNO");
    CamOsThreadSetName(TaskHandle1, "abcdefghijklmno");

    CamOsMsSleep(1000);
    CamOsThreadGetName(TaskHandle0, szTaskName0, sizeof(szTaskName0));
    CamOsThreadGetName(TaskHandle1, szTaskName1, sizeof(szTaskName1));
    CamOsPrintf("Test Get Name: %s %s\n", szTaskName0, szTaskName1);

    CamOsThreadJoin(TaskHandle0);
    CamOsThreadJoin(TaskHandle1);

    CamOsAtomicSet(&_gtThreadAtomic, 0);
    CamOsPrintf("### Priority: ThreadTest1 < ThreadTest2\n");
    tAttr.nPriority = 20;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle0, &tAttr, (void *)_CamOsThreadTestEntry1, NULL);
    tAttr.nPriority = 50;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle1, &tAttr, (void *)_CamOsThreadTestEntry2, NULL);

    while (CamOsAtomicRead(&_gtThreadAtomic) < 2)
        CamOsMsSleep(100);

    CamOsMsSleep(1000);

    CamOsAtomicIncReturn(&_gtThreadAtomic);
#ifdef CAM_OS_RTK
    while (CamOsAtomicRead(&_gtThreadAtomic) < 4)
        CamOsMsSleep(100);

    CamOsMsSleep(2000);

    CamOsPrintf("### Wake up _CamOsThreadTestEntry2\n");
    CamOsThreadWakeUp(TaskHandle1);

    CamOsMsSleep(1000);

    CamOsAtomicIncReturn(&_gtThreadAtomic);
#endif
    CamOsThreadJoin(TaskHandle0);
    CamOsThreadJoin(TaskHandle1);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsThread delete task\n", __LINE__);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsThread test end!!!\n", __LINE__);
}

static int _CamOsThreadPrioTest(void *pUserData)
{
    u32 *nThreadId = (u32 *)pUserData;
    u32 i;

    CamOsPrintf("%s start Thread%d\n", __FUNCTION__, *nThreadId);
    for (i = 0; i < 10; i++)
    {
        CamOsPrintf("Thread%d running\n", *nThreadId);
        CamOsMsSleep(1000);
    }
    CamOsPrintf("%s break ThreadId%d\n", __FUNCTION__, *nThreadId);

    return 0;
}

static void _TestCamOsThreadPriority(void)
{
#define THREAD_PRIORITY_TEST_NUM    100
    CamOsThread TaskHandle[THREAD_PRIORITY_TEST_NUM];
    s32 nTaskArg[THREAD_PRIORITY_TEST_NUM];
    CamOsThreadAttrb_t tAttr = {0};
    u32 nThreadPrioCnt;
    char szThreadName[16];

    for (nThreadPrioCnt=0; nThreadPrioCnt<THREAD_PRIORITY_TEST_NUM; nThreadPrioCnt++)
    {
        tAttr.nPriority = nThreadPrioCnt;
        tAttr.nStackSize = 0;
        tAttr.szName = "cam_os_thread";
        nTaskArg[nThreadPrioCnt] = nThreadPrioCnt;
        CamOsThreadCreate(&TaskHandle[nThreadPrioCnt], &tAttr, (void *)_CamOsThreadPrioTest, (void *)&nTaskArg[nThreadPrioCnt]);
    }

    for (nThreadPrioCnt=0; nThreadPrioCnt<THREAD_PRIORITY_TEST_NUM; nThreadPrioCnt++)
    {
        CamOsSnprintf(szThreadName, sizeof(szThreadName), "ThreadPrio%d", nThreadPrioCnt);
        CamOsThreadSetName(TaskHandle[nThreadPrioCnt], szThreadName);
    }

    for (nThreadPrioCnt=0; nThreadPrioCnt<THREAD_PRIORITY_TEST_NUM; nThreadPrioCnt++)
    {
        CamOsThreadJoin(TaskHandle[nThreadPrioCnt]);
    }

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsThreadPriority test end!!!\n", __LINE__);
}

static void _CamOsMutexTestEntry0(void *pUserdata)
{
    CamOsMutex_t *ptMutex = (CamOsMutex_t *)pUserdata;
    u32 i = 0;

    for(i = 0; i < 100; i++)
    {
        CamOsMutexLock(ptMutex);
        _gMutexTestCnt++;
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _CamOsThreadTestEntry0 start  count: %d\n", __LINE__, _gMutexTestCnt);
        CamOsMsSleep(3);
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _CamOsThreadTestEntry0 end    count: %d\n", __LINE__, _gMutexTestCnt);
        CamOsMutexUnlock(ptMutex);
    }
}

static void _CamOsMutexTestEntry1(void *pUserdata)
{
    CamOsMutex_t *ptMutex = (CamOsMutex_t *)pUserdata;
    u32 i = 0;

    for(i = 0; i < 100; i++)
    {
        CamOsMutexLock(ptMutex);
        _gMutexTestCnt++;
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _CamOsThreadTestEntry1 start  count: %d\n", __LINE__, _gMutexTestCnt);
        CamOsMsSleep(2);
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _CamOsThreadTestEntry1 end    count: %d\n", __LINE__, _gMutexTestCnt);
        CamOsMutexUnlock(ptMutex);
    }
}

static void _CamOsMutexTestEntry2(void *pUserdata)
{
    CamOsMutex_t *ptMutex = (CamOsMutex_t *)pUserdata;
    u32 i = 0;

    for(i = 0; i < 100; i++)
    {
        CamOsMutexLock(ptMutex);
        _gMutexTestCnt++;
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _CamOsThreadTestEntry2 start  count: %d\n", __LINE__, _gMutexTestCnt);
        CamOsMsSleep(5);
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _CamOsThreadTestEntry2 end    count: %d\n", __LINE__, _gMutexTestCnt);
        CamOsMutexUnlock(ptMutex);
    }
}

static void _TestCamOsMutex(void)
{
    CamOsThread TaskHandle0, TaskHandle1, TaskHandle2;
    CamOsThreadAttrb_t tAttr = {0};
    CamOsMutex_t tCamOsMutex;

    CamOsMutexInit(&tCamOsMutex);

    tAttr.nPriority = 50;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle0, &tAttr, (void *)_CamOsMutexTestEntry0, &tCamOsMutex);

    tAttr.nPriority = 50;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle1, &tAttr, (void *)_CamOsMutexTestEntry1, &tCamOsMutex);

    tAttr.nPriority = 100;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle2, &tAttr, (void *)_CamOsMutexTestEntry2, &tCamOsMutex);

    CamOsThreadJoin(TaskHandle0);
    CamOsThreadJoin(TaskHandle1);
    CamOsThreadJoin(TaskHandle2);

    CamOsMutexDestroy(&tCamOsMutex);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsMutex delete task\n", __LINE__);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsMutex test end!!!\n", __LINE__);
}

static void _TestCamOsDmem(void)
{
#ifdef CAM_OS_RTK
    void *pVirtPtr = NULL;
    void *pMiuAddr = NULL;
    void *pPhysAddr = NULL;

    CamOsDirectMemAlloc("TESTDMEM", 1025, &pVirtPtr, &pPhysAddr, &pMiuAddr);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsDirectMemAlloc get addr:\n    pVirtPtr 0x%08X\n    pPhysAddr 0x%08X\n    pMiuAddr 0x%08X\n",
                __LINE__, (u32)pVirtPtr, (u32)pPhysAddr, (u32)pMiuAddr);

    CamOsPrintf("\nTest Phys/Vitr/Miu Address translation\n");
    CamOsPrintf("    PhysToMiu   0x%08X -> 0x%08X\n", (u32)pPhysAddr, (u32)CamOsDirectMemPhysToMiu((void *)pPhysAddr));
    CamOsPrintf("    MiuToPhys   0x%08X -> 0x%08X\n", (u32)pMiuAddr,  (u32)CamOsDirectMemMiuToPhys((void *)pMiuAddr));
    CamOsPrintf("    PhysToVirt  0x%08X -> 0x%08X\n", (u32)pPhysAddr, (u32)CamOsDirectMemPhysToVirt((void *)pPhysAddr));
    CamOsPrintf("    VirtToPhys  0x%08X -> 0x%08X\n", (u32)pVirtPtr,  (u32)CamOsDirectMemVirtToPhys((void *)pVirtPtr));

    CamOsPrintf("\nTest Cache / Noncache Access\n");
    CamOsPrintf("    write cached   ptr=0x%08x, value = 0x%08x\n", (u32)pPhysAddr, 0x12345678);
    *(u32 *)pPhysAddr = 0x12345678;
    CamOsPrintf("    read cached    ptr=0x%08x, value = 0x%08x\n", (u32)pPhysAddr, *(u32 *)pPhysAddr);

    CamOsPrintf("    read uncached  ptr=0x%08x, value = 0x%08x\n", (u32)pVirtPtr, *(u32 *)pVirtPtr);

    CamOsPrintf("    write uncached ptr=0x%08x, value = 0x%08x\n", (u32)pVirtPtr, 0x87654321);
    *(u32 *)pVirtPtr = 0x87654321;
    CamOsPrintf("    read cached    ptr=0x%08x, value = 0x%08x\n", (u32)pPhysAddr, *(u32 *)pPhysAddr);
    CamOsPrintf("    read uncached  ptr=0x%08x, value = 0x%08x\n", (u32)pVirtPtr, *(u32 *)pVirtPtr);

    CamOsPrintf("    flush cached   ptr=0x%08x\n", (u32)pPhysAddr);
    CamOsDirectMemFlush(pVirtPtr);
    CamOsPrintf("    read cached    ptr=0x%08x, value = 0x%08x\n", (u32)pPhysAddr, *(u32 *)pPhysAddr);
    CamOsPrintf("    read uncached  ptr=0x%08x, value = 0x%08x\n", (u32)pVirtPtr, *(u32 *)pVirtPtr);

    CamOsDirectMemRelease((u8 *)pVirtPtr, 1025);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem test end!!!\n", __LINE__);
#else
    void *pVirtPtr = NULL;
    void *pMiuAddr = NULL;

    CamOsDirectMemAlloc("TESTDMEM", 1025, &pVirtPtr, NULL, &pMiuAddr);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsDirectMemAlloc get addr:  pVirtPtr 0x%08X  pMiuAddr 0x%08X\n",
                __LINE__, (u32)pVirtPtr, (u32)pMiuAddr);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: read uncached ptr=0x%x, value = 0x%x\n", __LINE__, (u32)pVirtPtr, *(u32 *)pVirtPtr);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: write uncached ptr=0x%x, value = 0x%x\n", __LINE__, (u32)pVirtPtr, 0x87654321);
    *(u32 *)pVirtPtr = 0x87654321;

    CamOsDirectMemFlush(pVirtPtr);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsDmem: read uncached ptr=0x%x, value = 0x%x\n", __LINE__, (u32)pVirtPtr, *(u32 *)pVirtPtr);

    CamOsDirectMemRelease((u8 *)pVirtPtr, 1025);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d]_TestCamOsDmem test end!!!\n", __LINE__);
#endif
}

static void _TestCamOsMem(void)
{
    void *pUserPtr = NULL;

    // test CamOsMemAlloc
    pUserPtr = CamOsMemAlloc(2048);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMemAlloc get addr:  pUserPtr 0x%08X\n", __LINE__, (u32)pUserPtr);

    memset(pUserPtr, 0x5A, 2048);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] value in address 0x%08X is 0x%08X\n", __LINE__, (u32)pUserPtr, *(u32 *)pUserPtr);

    CamOsMemRelease(pUserPtr);
    pUserPtr = NULL;
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMemRelease free buffer\n", __LINE__);

    // test CamOsMemCalloc
    pUserPtr = CamOsMemCalloc(2048, 1);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMemCalloc get addr:  pUserPtr 0x%08X\n", __LINE__, (u32)pUserPtr);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] value in address 0x%08X is 0x%08X\n", __LINE__, (u32)pUserPtr, *(u32 *)pUserPtr);

    CamOsMemRelease(pUserPtr);
    pUserPtr = NULL;
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMemRelease free buffer\n", __LINE__);

    // test CamOsMemRealloc
    pUserPtr = CamOsMemAlloc(2048);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CAM_OS_MemMalloc get addr:  pUserPtr 0x%08X\n", __LINE__, (u32)pUserPtr);

    pUserPtr = CamOsMemRealloc(pUserPtr, 4096);
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMemRealloc get addr:  pUserPtr 0x%08X\n", __LINE__, (u32)pUserPtr);

    CamOsMemRelease(pUserPtr);
    pUserPtr = NULL;
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMemRelease free buffer\n", __LINE__);

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] _TestCamOsMem test end!!!\n", __LINE__);
}

static void _TestCamOsPhyMapVirt(void)
{
    void *pVirtPtr;
    #define TEST_PHY_MEM_ADDR 0X20200000
    #define TEST_PHY_MEM_SIZE 0x100000
    CamOsPrintf("Test cache mapping:\n");
    pVirtPtr = CamOsPhyMemMap((void *)TEST_PHY_MEM_ADDR, TEST_PHY_MEM_SIZE, 1);
    CamOsPrintf("        PhysAddr: 0x%08X\n", TEST_PHY_MEM_ADDR);
    CamOsPrintf("        VirtAddr: 0x%08X\n", (u32)pVirtPtr);
    CamOsPhyMemUnMap(pVirtPtr, TEST_PHY_MEM_SIZE);
}

static void _TestCamOsTimer(void)
{
    s32 nCnt = 0;
    CamOsTimespec_t tTvStart = {0}, tTvEnd = {0};

    for(nCnt = 0; nCnt <= 10; nCnt++)
    {
        CamOsGetMonotonicTime(&tTvStart);
        CamOsMsSleep(1000);
        CamOsGetMonotonicTime(&tTvEnd);
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMsSleep(1000)  Now: %d.%09d    Diff: %dms\n", __LINE__, tTvEnd.nSec, tTvEnd.nNanoSec,
                    (s32)CamOsTimeDiff(&tTvStart, &tTvEnd, CAM_OS_TIME_DIFF_MS));
    }

    for(nCnt = 0; nCnt <= 10; nCnt++)
    {
        CamOsGetMonotonicTime(&tTvStart);
        CamOsMsSleep(1);
        CamOsGetMonotonicTime(&tTvEnd);
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMsSleep(1)  Now: %d.%09d    Diff: %dms\n", __LINE__, tTvEnd.nSec, tTvEnd.nNanoSec,
                    (s32)CamOsTimeDiff(&tTvStart, &tTvEnd, CAM_OS_TIME_DIFF_MS));
    }

    for(nCnt = 0; nCnt <= 10; nCnt++)
    {
        CamOsGetMonotonicTime(&tTvStart);
        CamOsUsSleep(100);
        CamOsGetMonotonicTime(&tTvEnd);
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsUsSleep(100)  Now: %d.%09d    Diff: %dus\n", __LINE__, tTvEnd.nSec, tTvEnd.nNanoSec,
                    (s32)CamOsTimeDiff(&tTvStart, &tTvEnd, CAM_OS_TIME_DIFF_US));
    }

    for(nCnt = 0; nCnt <= 10; nCnt++)
    {
        CamOsGetMonotonicTime(&tTvStart);
        CamOsUsSleep(1);
        CamOsGetMonotonicTime(&tTvEnd);
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsUsSleep(1)  Now: %d.%09d    Diff: %dus\n", __LINE__, tTvEnd.nSec, tTvEnd.nNanoSec,
                    (s32)CamOsTimeDiff(&tTvStart, &tTvEnd, CAM_OS_TIME_DIFF_US));
    }

    for(nCnt = 0; nCnt <= 10; nCnt++)
    {
        CamOsGetMonotonicTime(&tTvStart);
        CamOsMsDelay(1);
        CamOsGetMonotonicTime(&tTvEnd);
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsMsDelay(1)  Now: %d.%09d    Diff: %dus\n", __LINE__, tTvEnd.nSec, tTvEnd.nNanoSec,
                    (s32)CamOsTimeDiff(&tTvStart, &tTvEnd, CAM_OS_TIME_DIFF_US));
    }

    for(nCnt = 0; nCnt <= 10; nCnt++)
    {
        CamOsGetMonotonicTime(&tTvStart);
        CamOsUsDelay(1);
        CamOsGetMonotonicTime(&tTvEnd);
        CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] CamOsUsDelay(1)  Now: %d.%09d    Diff: %dns\n", __LINE__, tTvEnd.nSec, tTvEnd.nNanoSec,
                    (s32)CamOsTimeDiff(&tTvStart, &tTvEnd, CAM_OS_TIME_DIFF_NS));
    }
}

static s32 _CamOsTsemTestEntry0(void *pUserData)
{
    CamOsTsem_t *pSem = (CamOsTsem_t *)pUserData;
    CamOsRet_e eRet;

    CamOsMsSleep(1000);

    _gTestCnt++;//1

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(10);

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    if (_gTestCnt != 2)
    {
        CamOsPrintf("%s: step 1 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsMsSleep(300);

    _gTestCnt++;//3

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    _gTestCnt++;//4

    CamOsMsSleep(5000);

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(10);

    CamOsPrintf("%s CamOsTsemTimedDown start\n", __FUNCTION__);
    eRet = CamOsTsemTimedDown(pSem, 3000);
    CamOsPrintf("%s CamOsTsemTimedDown end (%s)\n", __FUNCTION__, (eRet == CAM_OS_OK) ? "wakened" : "timeout");

    if (_gTestCnt != 5)
    {
        CamOsPrintf("%s: step 2 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsMsSleep(1000);

    _gTestCnt++;//6

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(10);

    CamOsPrintf("%s CamOsTsemTimedDown start\n", __FUNCTION__);
    eRet = CamOsTsemTimedDown(pSem, 5000);
    CamOsPrintf("%s CamOsTsemTimedDown end (%s)\n", __FUNCTION__, (eRet == CAM_OS_OK) ? "wakened" : "timeout");

    if (eRet != CAM_OS_OK)
    {
        CamOsPrintf("%s: step 3 fail!(eRet=%d)\n", __FUNCTION__, eRet);
        return -1;
    }

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static s32 _CamOsTsemTestEntry1(void *pUserData)
{
    CamOsTsem_t *pSem = (CamOsTsem_t *)pUserData;
    CamOsRet_e eRet;

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    if (_gTestCnt != 1)
    {
        CamOsPrintf("%s: step 1 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsMsSleep(1000);

    _gTestCnt++;//2

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    if (_gTestCnt != 3)
    {
        CamOsPrintf("%s: step 2 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    CamOsMsSleep(3000);

    CamOsPrintf("%s CamOsTsemDown start\n", __FUNCTION__);
    CamOsTsemDown(pSem);
    CamOsPrintf("%s CamOsTsemDown end\n", __FUNCTION__);

    if (_gTestCnt != 4)
    {
        CamOsPrintf("%s: step 3 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsMsSleep(1000);

    _gTestCnt++;//5

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsTsemTimedDown start\n", __FUNCTION__);
    eRet = CamOsTsemTimedDown(pSem, 3000);
    CamOsPrintf("%s CamOsTsemTimedDown end (%s)\n", __FUNCTION__, (eRet == CAM_OS_OK) ? "wakened" : "timeout");

    if (_gTestCnt != 6)
    {
        CamOsPrintf("%s: step 4 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsTsemTimedDown start\n", __FUNCTION__);
    eRet = CamOsTsemTimedDown(pSem, 3000);
    CamOsPrintf("%s CamOsTsemTimedDown end (%s)\n", __FUNCTION__, (eRet == CAM_OS_OK) ? "wakened" : "timeout");

    if (eRet != CAM_OS_TIMEOUT)
    {
        CamOsPrintf("%s: step 5 fail!(eRet=%d)\n", __FUNCTION__, eRet);
        return -1;
    }

    CamOsPrintf("%s CamOsTsemUp\n", __FUNCTION__);
    CamOsTsemUp(pSem);


    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static s32 _CamOsRwsemTestEntry0(void *pUserData)
{
    CamOsRwsem_t *tpRwsem = (CamOsRwsem_t *)pUserData;

    CamOsPrintf("%s CamOsRwsemDownRead start\n", __FUNCTION__);
    CamOsRwsemDownRead(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownRead end\n", __FUNCTION__);

    CamOsMsSleep(2000);

    if (_gTestCnt != 0)
    {
        CamOsPrintf("%s: step 1 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsRwsemUpRead\n", __FUNCTION__);
    CamOsRwsemUpRead(tpRwsem);

    CamOsMsSleep(50);

    if (_gTestCnt != 1)
    {
        CamOsPrintf("%s: step 2 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsRwsemDownWrite start\n", __FUNCTION__);
    CamOsRwsemDownWrite(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownWrite end\n", __FUNCTION__);

    CamOsMsSleep(1500);

    _gTestCnt++;

    CamOsPrintf("%s CamOsRwsemUpWrite\n", __FUNCTION__);
    CamOsRwsemUpWrite(tpRwsem);

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static s32 _CamOsRwsemTestEntry1(void *pUserData)
{
    CamOsRwsem_t *tpRwsem = (CamOsRwsem_t *)pUserData;
    CamOsRet_e eRet;

    CamOsMsSleep(20);
    CamOsPrintf("%s CamOsRwsemTryDownRead start\n", __FUNCTION__);
    if (CAM_OS_OK != (eRet = CamOsRwsemTryDownRead(tpRwsem)))
    {
        CamOsPrintf("%s: step 1 fail!(eRet=%d)\n", __FUNCTION__, eRet);
        return -1;
    }
    CamOsPrintf("%s CamOsRwsemTryDownRead end\n", __FUNCTION__);

    CamOsMsSleep(2000);

    if (_gTestCnt != 0)
    {
        CamOsPrintf("%s: step 2 fail!\n", __FUNCTION__);
        return -1;
    }

    CamOsPrintf("%s CamOsRwsemUpRead\n", __FUNCTION__);
    CamOsRwsemUpRead(tpRwsem);

    CamOsMsSleep(50);

    if (_gTestCnt != 1)
    {
        CamOsPrintf("%s: step 3 fail!\n", __FUNCTION__);
        return -1;
    }

    CamOsMsSleep(1000);

    CamOsPrintf("%s CamOsRwsemDownWrite start\n", __FUNCTION__);
    CamOsRwsemDownWrite(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownWrite end\n", __FUNCTION__);

    if (_gTestCnt != 2)
    {
        CamOsPrintf("%s: step 4 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    _gTestCnt++;

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsRwsemUpWrite\n", __FUNCTION__);
    CamOsRwsemUpWrite(tpRwsem);

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static s32 _CamOsRwsemTestEntry2(void *pUserData)
{
    CamOsRwsem_t *tpRwsem = (CamOsRwsem_t *)pUserData;
    CamOsRet_e eRet;

    CamOsMsSleep(50);

    CamOsPrintf("%s CamOsRwsemTryDownWrite start\n", __FUNCTION__);
    if (CAM_OS_RESOURCE_BUSY != (eRet = CamOsRwsemTryDownWrite(tpRwsem)))
    {
        CamOsPrintf("%s: step 1 fail!(eRet=%d)\n", __FUNCTION__, eRet);
        return -1;
    }
    CamOsPrintf("%s CamOsRwsemTryDownWrite end\n", __FUNCTION__);

    CamOsPrintf("%s CamOsRwsemDownWrite start\n", __FUNCTION__);
    CamOsRwsemDownWrite(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownWrite end\n", __FUNCTION__);

    _gTestCnt++;

    CamOsPrintf("%s CamOsRwsemUpWrite\n", __FUNCTION__);
    CamOsRwsemUpWrite(tpRwsem);

    CamOsMsSleep(200);

    CamOsPrintf("%s CamOsRwsemDownRead start\n", __FUNCTION__);
    CamOsRwsemDownRead(tpRwsem);
    CamOsPrintf("%s CamOsRwsemDownRead end\n", __FUNCTION__);

    if (_gTestCnt != 2 && _gTestCnt != 3)
    {
        CamOsPrintf("%s: step 2 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsRwsemUpRead\n", __FUNCTION__);
    CamOsRwsemUpRead(tpRwsem);

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static void _TestCamOsTsem(void)
{
    static CamOsThread TaskHandle0, TaskHandle1;
    CamOsTsem_t tSem;

    _gTestCnt = 0;

    CamOsTsemInit(&tSem, 0);
    CamOsThreadCreate(&TaskHandle0, NULL, (void *)_CamOsTsemTestEntry0, &tSem);
    CamOsThreadCreate(&TaskHandle1, NULL, (void *)_CamOsTsemTestEntry1, &tSem);

    CamOsThreadJoin(TaskHandle0);
    CamOsThreadJoin(TaskHandle1);

    CamOsTsemDeinit(&tSem);
}

static void _TestCamOsRwsem(void)
{
    static CamOsThread TaskHandle0, TaskHandle1, TaskHandle2;
    CamOsRwsem_t tRwsem;

    _gTestCnt = 0;

    CamOsRwsemInit(&tRwsem);
    CamOsThreadCreate(&TaskHandle0, NULL, (void *)_CamOsRwsemTestEntry0, &tRwsem);
    CamOsThreadCreate(&TaskHandle1, NULL, (void *)_CamOsRwsemTestEntry1, &tRwsem);
    CamOsThreadCreate(&TaskHandle2, NULL, (void *)_CamOsRwsemTestEntry2, &tRwsem);

    CamOsThreadJoin(TaskHandle0);
    CamOsThreadJoin(TaskHandle1);
    CamOsThreadJoin(TaskHandle2);

    CamOsRwsemDeinit(&tRwsem);
}

static void _TestCamOsDiv64(void)
{
    u64 nDividendU64 = 0, nDivisorU64 = 0, nResultU64 = 0, nRemainderU64 = 0;
    s64 nDividendS64 = 0, nDivisorS64 = 0, nResultS64 = 0, nRemainderS64 = 0;

    CamOsPrintf("Unsigned 64 bit dividend:");
    CamOsScanf("%llu", &nDividendU64);
    CamOsPrintf("Unsigned 64 bit divisor:");
    CamOsScanf("%llu", &nDivisorU64);

    CamOsPrintf("Directly: %llu / %llu = %llu    remaind %llu\n", nDividendU64, nDivisorU64, nDividendU64 / nDivisorU64, nDividendU64 % nDivisorU64);
    nResultU64 = CamOsMathDivU64(nDividendU64, nDivisorU64, &nRemainderU64);
    CamOsPrintf("By Div64: %llu / %llu = %llu    remaind %llu\n", nDividendU64, nDivisorU64, nResultU64, nRemainderU64);


    CamOsPrintf("Signed 64 bit dividend:");
    CamOsScanf("%lld", &nDividendS64);
    CamOsPrintf("Signed 64 bit divisor:");
    CamOsScanf("%lld", &nDivisorS64);

    CamOsPrintf("Directly: %lld / %lld = %lld    remaind %lld\n", nDividendS64, nDivisorS64, nDividendS64 / nDivisorS64, nDividendS64 % nDivisorS64);
    nResultS64 = CamOsMathDivS64(nDividendS64, nDivisorS64, &nRemainderS64);
    CamOsPrintf("By Div64: %lld / %lld = %lld    remaind %lld\n", nDividendS64, nDivisorS64, nResultS64, nRemainderS64);
}

static void _TestCamOsSystemTime(void)
{
    s32 nCnt = 0;
    CamOsTimespec_t tTs;
    struct tm * tTm;
    time_t nRawTime;

    for(nCnt = 0; nCnt < 10; nCnt++)
    {
        CamOsGetTimeOfDay(&tTs);
        nRawTime = (time_t)tTs.nSec;
        tTm = localtime (&nRawTime);
        CamOsPrintf("RawSecond: %d  ->  %d/%02d/%02d [%d]  %02d:%02d:%02d\n",
                    tTs.nSec,
                    tTm->tm_year+1900,
                    tTm->tm_mon+1,
                    tTm->tm_mday,
                    tTm->tm_wday,
                    tTm->tm_hour,
                    tTm->tm_min,
                    tTm->tm_sec);

        tTs.nSec += 90000;
        CamOsSetTimeOfDay(&tTs);

        CamOsMsSleep(3000);
    }

    for(nCnt = 0; nCnt < 10; nCnt++)
    {
        CamOsGetTimeOfDay(&tTs);
        nRawTime = (time_t)tTs.nSec;
        tTm = localtime (&nRawTime);
        CamOsPrintf("RawSecond: %d  ->  %d/%02d/%02d [%d]  %02d:%02d:%02d\n",
                    tTs.nSec,
                    tTm->tm_year+1900,
                    tTm->tm_mon+1,
                    tTm->tm_mday,
                    tTm->tm_wday,
                    tTm->tm_hour,
                    tTm->tm_min,
                    tTm->tm_sec);

        tTs.nSec -= 90000;
        CamOsSetTimeOfDay(&tTs);

        CamOsMsSleep(3000);
    }
}

static void _TestCamOsPhysMemSize(void)
{
    CamOsMemSize_e eMemSize;
    eMemSize = CamOsPhysMemSize();
    CamOsPrintf("System has %dMB physical memory\n", 1<<(u32)eMemSize);
}

static void _TestCamOsChipId(void)
{
    CamOsPrintf("Chip ID: 0x%X\n", CamOsChipId());
}

static s32 _CamOsTcondTestEntry0(void *pUserData)
{
    CamOsTcond_t *pCond = (CamOsTcond_t *)pUserData;

    CamOsMsSleep(300);

    _gTestCnt++;//1

    CamOsPrintf("%s CamOsTcondSignal\n", __FUNCTION__);
    CamOsTcondSignal(pCond);

    CamOsMsSleep(100);

    CamOsPrintf("%s CamOsTcondSignal\n", __FUNCTION__);
    CamOsTcondSignal(pCond);

    CamOsMsSleep(100);

    _gTestCnt++;//3

    CamOsPrintf("%s CamOsTcondSignalAll\n", __FUNCTION__);
    CamOsTcondSignalAll(pCond);

    CamOsPrintf("%s CamOsTcondTimedWait start\n", __FUNCTION__);
    if (CamOsTcondTimedWait(pCond, 500) != CAM_OS_TIMEOUT)
    {
        CamOsPrintf("%s: step 1 fail!\n", __FUNCTION__);
        return -1;
    }
    CamOsPrintf("%s CamOsTcondTimedWait end(timeout)\n", __FUNCTION__);

    CamOsMsSleep(2000);

    _gTestCnt++;

    CamOsPrintf("%s CamOsTcondSignal\n", __FUNCTION__);
    CamOsTcondSignal(pCond);

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static s32 _CamOsTcondTestEntry1(void *pUserData)
{
    CamOsTcond_t *pCond = (CamOsTcond_t *)pUserData;
    CamOsRet_e eRet;

    CamOsPrintf("%s CamOsTcondWait start\n", __FUNCTION__);
    CamOsTcondWait(pCond);
    CamOsPrintf("%s CamOsTcondWait end\n", __FUNCTION__);

    if (_gTestCnt != 1)
    {
        CamOsPrintf("%s: step 1 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    _gTestCnt++;//2

    CamOsPrintf("%s CamOsTcondWait start\n", __FUNCTION__);
    CamOsTcondWait(pCond);
    CamOsPrintf("%s CamOsTcondWait end\n", __FUNCTION__);

    if (_gTestCnt != 3)
    {
        CamOsPrintf("%s: step 2 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsTcondTimedWait start\n", __FUNCTION__);
    eRet = CamOsTcondTimedWait(pCond, 1000);
    if (eRet != CAM_OS_TIMEOUT || _gTestCnt != 3)
    {
        CamOsPrintf("%s: step 3 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }
    CamOsPrintf("%s CamOsTcondTimedWait end(timeout)\n", __FUNCTION__);

    _gTestCnt++;

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static s32 _CamOsTcondTestEntry2(void *pUserData)
{
    CamOsRet_e eRet;

    CamOsTcond_t *pCond = (CamOsTcond_t *)pUserData;

    CamOsMsSleep(10);

    CamOsPrintf("%s CamOsTcondWait start\n", __FUNCTION__);
    CamOsTcondWait(pCond);
    CamOsPrintf("%s CamOsTcondWait end\n", __FUNCTION__);

    if (_gTestCnt != 2)
    {
        CamOsPrintf("%s: step 1 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }

    CamOsPrintf("%s CamOsTcondWait start\n", __FUNCTION__);
    CamOsTcondWait(pCond);
    CamOsPrintf("%s CamOsTcondWait end\n", __FUNCTION__);

    if (_gTestCnt != 3)
    {
        CamOsPrintf("%s: step 2 fail!\n", __FUNCTION__);
        return -1;
    }

    CamOsPrintf("%s CamOsTcondTimedWait start\n", __FUNCTION__);
    eRet = CamOsTcondTimedWait(pCond, 5000);
    if (eRet != CAM_OS_OK || _gTestCnt != 5)
    {
        CamOsPrintf("%s: step 3 fail!(_gTestCnt=%d)\n", __FUNCTION__, _gTestCnt);
        return -1;
    }
    CamOsPrintf("%s CamOsTcondTimedWait end(wakend)\n", __FUNCTION__);

    CamOsPrintf("%s break\n", __FUNCTION__);

    return 0;
}

static void _TestCamOsTcond(void)
{
    static CamOsThread TaskHandle0, TaskHandle1, TaskHandle2;
    CamOsTcond_t tCond;

    _gTestCnt = 0;
    CamOsTcondInit(&tCond);

    CamOsThreadCreate(&TaskHandle0, NULL, (void *)_CamOsTcondTestEntry0, &tCond);
    CamOsThreadCreate(&TaskHandle1, NULL, (void *)_CamOsTcondTestEntry1, &tCond);
    CamOsThreadCreate(&TaskHandle2, NULL, (void *)_CamOsTcondTestEntry2, &tCond);

    CamOsThreadJoin(TaskHandle0);
    CamOsThreadJoin(TaskHandle1);
    CamOsThreadJoin(TaskHandle2);

    CamOsTcondDeinit(&tCond);
}

static void _TestCamOsBitmap(void)
{
    #define BITMAP_BITS 128
    CAM_OS_DECLARE_BITMAP(aBitmap, BITMAP_BITS);

    CAM_OS_BITMAP_CLEAR(aBitmap);
    CamOsPrintf("Set bit 0, 1, 2, 37, 98\n");
    CAM_OS_SET_BIT(0, aBitmap);
    CAM_OS_SET_BIT(1, aBitmap);
    CAM_OS_SET_BIT(2, aBitmap);
    CAM_OS_SET_BIT(37, aBitmap);
    CAM_OS_SET_BIT(98, aBitmap);
    CamOsPrintf("\ttest bit 0:   %d\n", CAM_OS_TEST_BIT(0, aBitmap));
    CamOsPrintf("\ttest bit 1:   %d\n", CAM_OS_TEST_BIT(1, aBitmap));
    CamOsPrintf("\ttest bit 2:   %d\n", CAM_OS_TEST_BIT(2, aBitmap));
    CamOsPrintf("\ttest bit 3:   %d\n", CAM_OS_TEST_BIT(3, aBitmap));
    CamOsPrintf("\ttest bit 30:  %d\n", CAM_OS_TEST_BIT(30, aBitmap));
    CamOsPrintf("\ttest bit 37:  %d\n", CAM_OS_TEST_BIT(37, aBitmap));
    CamOsPrintf("\ttest bit 80:  %d\n", CAM_OS_TEST_BIT(80, aBitmap));
    CamOsPrintf("\ttest bit 98:  %d\n", CAM_OS_TEST_BIT(98, aBitmap));
    CamOsPrintf("\ttest bit 127: %d\n", CAM_OS_TEST_BIT(127, aBitmap));

    CamOsPrintf("\tfirst zero bit: %u\n", CAM_OS_FIND_FIRST_ZERO_BIT(aBitmap, BITMAP_BITS));
    CamOsPrintf("Clear bit 2, 98\n");
    CAM_OS_CLEAR_BIT(2, aBitmap);
    CAM_OS_CLEAR_BIT(98, aBitmap);
    CamOsPrintf("\ttest bit 2:   %d\n", CAM_OS_TEST_BIT(2, aBitmap));
    CamOsPrintf("\ttest bit 98:  %d\n", CAM_OS_TEST_BIT(98, aBitmap));
    CamOsPrintf("\tfirst zero bit: %u\n", CAM_OS_FIND_FIRST_ZERO_BIT(aBitmap, BITMAP_BITS));
}

struct HashTableElement_t
{
    struct CamOsHListNode_t tHentry;
    u32 nKey;
    u32 nData;
};

static void _TestCamOsHash(void)
{
    u32 nItemNum;
    CAM_OS_DEFINE_HASHTABLE(aHashTable, 8);
    struct HashTableElement_t tHListNode0, tHListNode1, tHListNode2, tHListNode3, tHListNode4, *ptHListNode;

    tHListNode0.nKey = 102;
    tHListNode0.nData = 1021;
    tHListNode1.nKey = 1872;
    tHListNode1.nData = 18721;
    tHListNode2.nKey = 102;
    tHListNode2.nData = 1022;
    tHListNode3.nKey = 1872;
    tHListNode3.nData = 18722;
    tHListNode4.nKey = 102;
    tHListNode4.nData = 1023;
    CamOsPrintf("Add 3 items with key 102 and 2 items with key 1872\r\n");
    CAM_OS_HASH_ADD(aHashTable, &tHListNode0.tHentry, tHListNode0.nKey);
    CAM_OS_HASH_ADD(aHashTable, &tHListNode1.tHentry, tHListNode1.nKey);
    CAM_OS_HASH_ADD(aHashTable, &tHListNode2.tHentry, tHListNode2.nKey);
    CAM_OS_HASH_ADD(aHashTable, &tHListNode3.tHentry, tHListNode3.nKey);
    CAM_OS_HASH_ADD(aHashTable, &tHListNode4.tHentry, tHListNode4.nKey);
    CamOsPrintf("Get items with key 102: \r\n");
    nItemNum = 0;
    CAM_OS_HASH_FOR_EACH_POSSIBLE(aHashTable, ptHListNode, tHentry, 102)
    {
        CamOsPrintf("\titem %u: data=%u\r\n", nItemNum, ptHListNode->nData);
        nItemNum++;
    }

    CamOsPrintf("Get items with key 1872: \r\n");
    nItemNum = 0;
    CAM_OS_HASH_FOR_EACH_POSSIBLE(aHashTable, ptHListNode, tHentry, 1872)
    {
        CamOsPrintf("\titem %u: data=%u\r\n", nItemNum, ptHListNode->nData);
        nItemNum++;
    }

    CamOsPrintf("Delete one items with key 1872.\r\n");
    CAM_OS_HASH_DEL(&tHListNode3.tHentry);
    CamOsPrintf("Get items with key 1872: \r\n");
    nItemNum = 0;
    CAM_OS_HASH_FOR_EACH_POSSIBLE(aHashTable, ptHListNode, tHentry, 1872)
    {
        CamOsPrintf("\titem %u: data=%u\n", nItemNum, ptHListNode->nData);
        nItemNum++;
    }
}

static void _TestCamOsIdr(void)
{
    CamOsIdr_t tIdr;
    u32 nIdrData1=11111, nIdrData2=22222, nIdrData3=33333, *pnIdrDataPtr;
    s32 nIdrId1, nIdrId2, nIdrId3;

    if (CAM_OS_OK == CamOsIdrInit(&tIdr))
    {
        CamOsPrintf("Alloc data1(=%u) in 100~200\r\n", nIdrData1);
        nIdrId1 = CamOsIdrAlloc(&tIdr, (void *)&nIdrData1, 100, 200);
        CamOsPrintf("Alloc data2(=%u) in 100~200\r\n", nIdrData2);
        nIdrId2 = CamOsIdrAlloc(&tIdr, (void *)&nIdrData2, 100, 200);
        CamOsPrintf("Alloc data3(=%u) in 500~\r\n", nIdrData3);
        nIdrId3 = CamOsIdrAlloc(&tIdr, (void *)&nIdrData3, 500, 0);
        pnIdrDataPtr = (u32*)CamOsIdrFind(&tIdr, nIdrId1);
        CamOsPrintf("ID1 = %d, find data = %u\r\n", nIdrId1, *pnIdrDataPtr);
        pnIdrDataPtr = (u32*)CamOsIdrFind(&tIdr, nIdrId2);
        CamOsPrintf("ID2 = %d, find data = %u\r\n", nIdrId2, *pnIdrDataPtr);
        pnIdrDataPtr = (u32*)CamOsIdrFind(&tIdr, nIdrId3);
        CamOsPrintf("ID3 = %d, find data = %u\r\n", nIdrId3, *pnIdrDataPtr);

        CamOsPrintf("Remove ID3(=%d) ... ", nIdrId3);
        CamOsIdrRemove(&tIdr, nIdrId3);
        if (NULL == CamOsIdrFind(&tIdr, nIdrId3))
            CamOsPrintf("success!\n");
        else
            CamOsPrintf("fail!\n");

        CamOsIdrDestroy(&tIdr);
    }
    else
        CamOsPrintf("CamOsIdrInit fail!\n");
}

#ifdef CAM_OS_RTK
static void _TimerCallback(unsigned long nDataAddr)
{
    unsigned long *pnTimerMs = (unsigned long *)nDataAddr;

    CamOsPrintf("%s: timer ms=%lu\n", __FUNCTION__, *pnTimerMs);
}

static void _TestCamOsTimerCallback(void)
{
    CamOsTimer_t tTimer;
    unsigned long nTimerMs;

    CamOsPrintf("[Step 1] add timer to 2000ms, then sleep 2100ms ... \n");
    CamOsTimerInit(&tTimer);
    nTimerMs = 2000;
    CamOsTimerAdd(&tTimer, nTimerMs, (void*)&nTimerMs, _TimerCallback);
    CamOsMsSleep(2100);
    if (CamOsTimerDelete(&tTimer))
        CamOsPrintf("fail!\n\n");
    else
        CamOsPrintf("success!\n\n");

    CamOsPrintf("[Step 2] add timer to 1000ms, modify to 1500ms in 300ms, then sleep 1600ms ... \n");
    CamOsTimerInit(&tTimer);
    nTimerMs = 1000;
    CamOsTimerAdd(&tTimer, nTimerMs, (void*)&nTimerMs, _TimerCallback);
    CamOsMsSleep(300);
    nTimerMs = 1500;
    CamOsTimerModify(&tTimer, nTimerMs);
    CamOsMsSleep(1600);
    if (CamOsTimerDelete(&tTimer))
        CamOsPrintf("fail!\n\n");
    else
        CamOsPrintf("success!\n\n");

    CamOsPrintf("[Step 3] add timer to 3000ms, sleep 1000ms, then delete timer ... \n");
    CamOsTimerInit(&tTimer);
    nTimerMs = 3000;
    CamOsTimerAdd(&tTimer, nTimerMs, (void*)&nTimerMs, _TimerCallback);
    CamOsMsSleep(1000);
    if (!CamOsTimerDelete(&tTimer))
        CamOsPrintf("fail!\n\n");
    else
        CamOsPrintf("success!\n\n");
}

#define MEMORY_CACHE_OBJECT_SIZE    0x50
static void _TestCamOsCacheAlloc(void)
{
    CamOsMemCache_t tMemCache;
    void *pMemCacheObj1, *pMemCacheObj2, *pMemCacheObj3;

    CamOsPrintf("Test non-HW cache alignment mapping:\n");
    if (CAM_OS_OK == CamOsMemCacheCreate(&tMemCache, "MemCacheTest", MEMORY_CACHE_OBJECT_SIZE, 0))
    {
        pMemCacheObj1 = CamOsMemCacheAlloc(&tMemCache);
        CamOsPrintf("Object1 address: 0x%08X\n", pMemCacheObj1);
        pMemCacheObj2 = CamOsMemCacheAlloc(&tMemCache);
        CamOsPrintf("Object2 address: 0x%08X\n", pMemCacheObj2);
        pMemCacheObj3 = CamOsMemCacheAlloc(&tMemCache);
        CamOsPrintf("Object3 address: 0x%08X\n", pMemCacheObj3);
        CamOsMemCacheFree(&tMemCache, pMemCacheObj1);
        CamOsMemCacheFree(&tMemCache, pMemCacheObj2);
        CamOsMemCacheFree(&tMemCache, pMemCacheObj3);
        CamOsMemCacheDestroy(&tMemCache);
    }
    else
        CamOsPrintf("CamOsMemCacheCreate fail!\n");
}
#endif

static void _TestCamOsPanic(void)
{
    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] Test CamOsPanic\n", __LINE__);
    CamOsPanic("Test OS Panic...");
}

static void _CamOsThreadTestSpinlock0(void *pUserdata)
{
    s32 i = 0;

    for (i=0; i<500; i++)
    {
        CamOsPrintf("Thread0 (%d)\n", i);
    }
}

static void _CamOsThreadTestSpinlock1(void *pUserdata)
{
    s32 i = 0;
    u32 *SpinlockTestMode = (u32 *)pUserdata;
#ifdef CAM_OS_RTK
    MsIntMask_e mask = 0;
#endif
    CamOsSpinlock_t stSpinlock;

    for (i=0; i<100; i++)
    {
        CamOsPrintf("Thread1 (%d)\n", i);
    }

    CamOsSpinInit(&stSpinlock);
    switch (*SpinlockTestMode)
    {
    case 0:
        CamOsSpinLock(&stSpinlock);
        break;
    case 1:
        CamOsSpinLockIrqSave(&stSpinlock);
        break;
    case 2:
#ifdef CAM_OS_RTK
        mask = MsDisableInterrupt();
#endif
        CamOsSpinLockIrqSave(&stSpinlock);
        break;
    default:
        break;
    }

    for (i=100; i<200; i++)
    {
        CamOsPrintf("\033[1;34mThread1 in Spinlock (%d)\033[m\n", i);
    }

    switch (*SpinlockTestMode)
    {
    case 0:
        CamOsSpinUnlock(&stSpinlock);
        break;
    case 1:
    case 2:
        CamOsSpinUnlockIrqRestore(&stSpinlock);
        break;
    default:
        break;
    }

    for (i=200; i<500; i++)
    {
        CamOsPrintf("Thread1 (%d)\n", i);
    }

#ifdef CAM_OS_RTK
    if (*SpinlockTestMode == 2)
        MsEnableInterrupt(mask);
#endif
}

static void _TestCamOsSpinlock(void)
{
    CamOsThread TaskHandle0, TaskHandle1;
    CamOsThreadAttrb_t tAttr = {0};
    u32 SpinlockTestMode = 2;   // 0: CamOsSpinLock.
                                // 1: CamOsSpinLockIrqSave.
                                // 2: CamOsSpinLockIrqSave after disable system interrupt.

    CamOsPrintf("[CAM_OS_WRAPPER_TEST:%d] Test CamOsSpinlock\n", __LINE__);

    tAttr.nPriority = 50;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle0, &tAttr, (void *)_CamOsThreadTestSpinlock0, NULL);

    tAttr.nPriority = 50;
    tAttr.nStackSize = 0;
    CamOsThreadCreate(&TaskHandle1, &tAttr, (void *)_CamOsThreadTestSpinlock1, (void *)&SpinlockTestMode);

    CamOsThreadJoin(TaskHandle0);
    CamOsThreadJoin(TaskHandle1);
}
