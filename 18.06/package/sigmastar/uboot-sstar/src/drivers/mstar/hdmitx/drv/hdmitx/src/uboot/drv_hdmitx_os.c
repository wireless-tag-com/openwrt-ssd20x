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

#define _DRV_HDMITX_OS_C_
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------
#include "mhal_common.h"
#include "drv_hdmitx_os.h"
#include "apiHDMITx.h"
#include "regHDMITx.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define TICK_PER_ONE_MS             (1) //Note: confirm Kernel fisrt
#define HDMITX_OS_WAIT_FOREVER      (0xffffff00/TICK_PER_ONE_MS)


#define HDMITX_OS_ID_PREFIX         0x76540000
#define HDMITX_OS_ID_PREFIX_MASK    0xFFFF0000
#define HDMITX_OS_ID_MASK           0x0000FFFF //~HDMITX_OS_ID_PREFIX_MASK

#define HAS_FLAG(flag, bit)         ((flag) & (bit))
#define SET_FLAG(flag, bit)         ((flag)|= (bit))
#define RESET_FLAG(flag, bit)       ((flag)&= (~(bit)))

#if CAM_OS_EVENTGROUP
#define EVENT_MUTEX_LOCK()
#define EVENT_MUTEX_UNLOCK()
#else
#define EVENT_MUTEX_LOCK()
#define EVENT_MUTEX_UNLOCK()
#endif

#define EN_EVENT_GROUP           0
//-------------------------------------------------------------------------------------------------
//  Structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    void (*pfnFunc)(unsigned long nDataAddr);
}DrvHdmitxOsTimerNotifyFunc_t;

typedef struct
{
    MS_U32 u32Dummy;
#if defined(CAM_OS)
    CamOsTsem_t EventGroup;
#endif
}DrvHdmitxOsMutext_t;

//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
MS_BOOL                             bHdmitxOsInit = 0;
DrvHdmitxOsEventGroupInfoConfig_t   _HdmitxOs_EventGroup_Info[HDMITX_OS_EVENTGROUP_MAX];


MS_U16  gu16I2cId = 0;
MS_U8   gu8HpdGpioPin = 0;

extern MS_U8 gu8SwI2cSdaPin;
extern MS_U8 gu8SwI2cSclPin;
extern MS_BOOL gbSwI2cEn;
//-------------------------------------------------------------------------------------------------
//  Internal Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Public Functions
//-------------------------------------------------------------------------------------------------
MS_BOOL DrvHdmitxOsInit(void)
{
    if(bHdmitxOsInit == 0)
    {

        bHdmitxOsInit = 1;
    }
    else
    {

    }
    return TRUE;
}

void* DrvHdmitxOsMemAlloc(MS_U32 u32Size)
{
    return malloc(u32Size);
}

void DrvHdmitxOsMemRelease(void *pPtr)
{
    free(pPtr);
}

MS_U32 DrvHdmitxOsGetSystemTime (void)
{
    return 0;
}

MS_U64 DrvHdmitxOsGetSystemTimeStamp (void)
{
    return 0;
}

MS_U32 DrvHdmitxOsTimerDiffTimeFromNow(MS_U32 u32TaskTimer) //unit = ms
{
    return (DrvHdmitxOsGetSystemTime() - u32TaskTimer);
}

MS_BOOL DrvHdmitxOsCreateTimer(DrvHdmitxOsTimerConfig_t *pTimerCfg)
{
    return TRUE;
}

MS_BOOL DrvHdmitxOsDeleteTimer(DrvHdmitxOsTimerConfig_t *pTimerCfg)
{
    return TRUE;
}

MS_BOOL DrvHdmitxOsStopTimer(DrvHdmitxOsTimerConfig_t *pTimerCfg)
{
    return TRUE;
}

MS_BOOL DrvHdmitxOsStartTimer(DrvHdmitxOsTimerConfig_t *pTimerCfg)
{
    return TRUE;

}

MS_BOOL DrvHdmitxOsCreateSemaphore(DrvHdmitxOsSemConfig_t *pstSemaphoreCfg, MS_U32 u32InitVal)
{
    return TRUE;

}

MS_BOOL DrvHdmitxOsDestroySemaphore(DrvHdmitxOsSemConfig_t *pstSemaphoreCfg)
{
    return TRUE;

}

void DrvHdmitxOsObtainSemaphore(DrvHdmitxOsSemConfig_t *pstSemaphoreCfg)
{
}

void DrvHdmitxOsReleaseSemaphore(DrvHdmitxOsSemConfig_t *pstSemaphoreCfg)
{
}


MS_BOOL DrvHdmitxOsCreateMutex(DrvHdmitxOsMutexConfig_t *pstMutexCfg)
{
    return TRUE;
}

MS_BOOL DrvHdmitxOsDestroyMutex(DrvHdmitxOsMutexConfig_t *pstMutexCfg)
{
    return TRUE;
}

MS_BOOL DrvHdmitxOsObtainMutex(DrvHdmitxOsMutexConfig_t *pstMutexCfg)
{
    return TRUE;
}


MS_BOOL DrvHdmitxOsReleaseMutex(DrvHdmitxOsMutexConfig_t *pstMutexCfg)
{
    return TRUE;
}

MS_S32 DrvHdmitxOsCreateEventGroup (char *pEventName)
{
    return 1;
}

MS_BOOL DrvHdmitxOsDeleteEventGroup (MS_S32 s32EventGroupId)
{
    return TRUE;
}


MS_BOOL DrvHdmitxOsSetEvent (MS_S32 s32EventGroupId, MS_U32 u32EventFlag)
{
    return TRUE;
}


MS_U32 DrvHdmitxOsGetEvent(MS_S32 s32EventGroupId)
{
    return 1;
}

MS_BOOL DrvHdmitxOsWaitEvent (MS_S32 s32EventGroupId,
                     MS_U32 u32WaitEventFlag,
                     MS_U32 *pu32RetrievedEventFlag,
                     DrvHdmitxOsEventWaitMoodeType_e eWaitMode,
                     MS_U32 u32WaitMs)
{
    return TRUE;
}


MS_BOOL DrvHdmitxOsAttachInterrupt (MS_U32 u32IntNum, InterruptCallBack pIntCb)
{
    return TRUE;
}

MS_BOOL DrvHdmitxOsDetachInterrupt (MS_U32 u32IntNum)
{
    return TRUE;
}


MS_BOOL DrvHdmitxOsEnableInterrupt (MS_U32 u32IntNum)
{
    return TRUE;
}

MS_BOOL DrvHdmitxOsDisableInterrupt (MS_U32 u32IntNum)
{
    return TRUE;
}

MS_U32 DrvHdmitxOsDisableAllInterrupts(void)
{
    return 0;
}


MS_BOOL DrvHdmitxOsRestoreAllInterrupts(MS_U32 u32OldInterrupts)
{
    return TRUE;
}


MS_BOOL DrvHdmitxOsCreateTask(DrvHdmitxOsTaskConfig_t *pstTaskCfg, TaskEntryCb pTaskEntry, void *pDataPtr, char *pTaskName, MS_BOOL bAuotStart)
{
    return TRUE;
}


MS_BOOL DrvHdmitxOsDestroyTask(DrvHdmitxOsTaskConfig_t *pstTaskCfg)
{
    return TRUE;
}


MS_BOOL DrvHdmitxOsGetMmioBase(MS_VIRT *pu32BaseAddr, MS_PHY *pu32BaseSize, DrvHdmitxOsMmioType_e enType)
{
    MS_BOOL bRet = TRUE;

    switch(enType)
    {
        case E_HDMITX_OS_MMIO_PM:
            *pu32BaseAddr = 0x1F000000;
            *pu32BaseSize = 0x00400000UL;
             break;

        case E_HDMITX_OS_MMIO_NONEPM:
            *pu32BaseAddr = 0x1F000000;
            *pu32BaseSize = 0x00200000UL;
             break;

        default:
            bRet = FALSE;
            break;
    }
    return bRet;
}

MS_BOOL DrvHdmitxOsGpioRequestOutput(MS_U8 u8GpioNum)
{
    return TRUE;

}

MS_BOOL DrvHdmitxOsGpioRequestInput(MS_U8 u8GpioNum)
{
    return TRUE;

}

MS_BOOL DrvHdmitxOsGetGpioValue(MS_U8 u8GpioNum)
{
    return TRUE;
}

MS_BOOL DrvHdmitxOsSetGpioValue(MS_U8 u8GpioNum, MS_U8 u8Val)
{
    return TRUE;
}

MS_BOOL DrvHdmitxOsGpioIrq(MS_U8 u8GpioNum, InterruptCb pIsrCb, MS_BOOL bEn)
{
    return TRUE;
}

void DrvHdmitxOsSetHpdIrq(MS_BOOL bEn)
{

}

MS_BOOL DrvHdmitxOsSetI2cAdapter(MS_U16 u16Id)
{
    return TRUE;

}

MS_BOOL DrvHdmitxOsSetI2cReadBytes(MS_U8 u8SlaveAdr, MS_U8 u8SubAdr, MS_U8 *pBuf, MS_U16 u16BufLen)
{
    return TRUE;
}

MS_BOOL DrvHdmitxOsSetI2cWriteBytes(MS_U8 u8SlaveAdr, MS_U8 u8SubAdr, MS_U8 *pBuf, MS_U16 u16BufLen, MS_BOOL bSendStop)
{
    return TRUE;
}

void DrvHdmitxOsMsSleep(MS_U32 u32Msec)
{
    mdelay(u32Msec);
}

void DrvHdmitxOsUsSleep(MS_U32 u32Usec)
{
    udelay(u32Usec);
}

void DrvHdmitxOsSetI2cId(MS_U16 u16Id)
{
    gu16I2cId = u16Id;
}

MS_U16 DrvHdmitxOsGetI2cId(void)
{
    return gu16I2cId;
}

MS_BOOL DrvHdmitxOsSetDeviceNode(void *pPlatFormDev)
{
    return FALSE;
}

MS_BOOL DrvHdmitxOsSetClkOn(void *pClkRate, MS_U32 u32ClkRateSize)
{
    return FALSE;
}

MS_BOOL DrvHdmitxOsSetClkOff(void)
{
    return FALSE;
}

MS_BOOL DrvHdmitxOsSetHpdGpinPin(MS_U8 u8Pin)
{
    gu8HpdGpioPin = u8Pin;
    return TRUE;
}

MS_U8 DrvHdmitxOsGetHpdGpinPin(void)
{
    return gu8HpdGpioPin;
}

void DrvHdmitxOsSetSwI2cPin(MS_U8 u8SdaPin, MS_U8 u8SclPin)
{
    gu8SwI2cSdaPin = u8SdaPin;
    gu8SwI2cSclPin = u8SclPin;
}

void DrvHdmitxOsSetSwI2cEn(MS_BOOL bEn)
{
    gbSwI2cEn = bEn;
}

MS_U32 DrvHdmitxOsGetCpuSpeedMhz(void)
{
    return 1000;
}

