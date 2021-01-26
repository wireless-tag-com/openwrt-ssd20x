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

#ifndef _DRV_HDHMITX_OS_H_
#define _DRV_HDHMITX_OS_H_

#include "drv_hdmitx_os_header.h"
//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#define PRINT_NONE    "\33[m"
#define PRINT_RED     "\33[1;31m"
#define PRINT_YELLOW  "\33[1;33m"
#define PRINT_GREEN   "\33[1;32m"


#define HDMITX_OS_EVENTGROUP_MAX     4//(64)


//-------------------------------------------------------------------------------------------------
//  Debug Info Macro
//-------------------------------------------------------------------------------------------------
#define MS_CRITICAL_MSG(x)      x       // for dump critical message
#define MS_FATAL_MSG(fmt,...) printf( "[MS_FATAL]: %s: %d \n"  fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__)

#if defined (CAMDRV_DEBUG)
  #define MS_DEBUG_MSG(x)       x
#elif defined (MS_OPTIMIZE)
  #define MS_DEBUG_MSG(x)               // retail version remove debug message
#endif


//user-defined warning
#ifdef BLOCK_ASSERT
    #define MS_ASSERT(_bool_)                                                                                   \
            {                                                                                                   \
                if (!(_bool_))                                                                                  \
                {                                                                                               \
                    printf("\033[35m ASSERT FAIL: %s, %s %s %d\033[35m\n", #_bool_, __FILE__, __PRETTY_FUNCTION__, __LINE__);    \
                    panic("camdriver panic\n");                                                                     \
                    *(volatile unsigned int *)(ULONG_MAX) = 0;                                                         \
                }                                                                                               \
            }
#else
    #define MS_ASSERT(_bool_)                                                                                   \
            {                                                                                                   \
                if (!(_bool_))                                                                                  \
                {                                                                                               \
                    printf("\033[35m ASSERT FAIL: %s, %s %s %d\033[35m\n", #_bool_, __FILE__, __PRETTY_FUNCTION__, __LINE__);    \
                    panic("camdriver panic\n");                                                                     \
                }                                                                                               \
            }
#endif

//-------------------------------------------------------------------------------------------------
//  MASK, BIT  BMASK Macro
//-------------------------------------------------------------------------------------------------


#define MASK(x)     (((1<<(x##_BITS))-1) << x##_SHIFT)
#ifndef BIT	//for Linux_kernel type, BIT redefined in <linux/bitops.h>
#define BIT(_bit_)                  (1 << (_bit_))
#endif
#define BIT_(x)                     BIT(x) //[OBSOLETED] //TODO: remove it later
#define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
#define BMASK(_bits_)               (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

#ifdef _DRV_HDMITX_OS_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE MS_BOOL DrvHdmitxOsInit(void);
INTERFACE MS_U32  DrvHdmitxOsGetSystemTime (void);
INTERFACE MS_U64  DrvHdmitxOsGetSystemTimeStamp (void);
INTERFACE MS_U32  DrvHdmitxOsTimerDiffTimeFromNow(MS_U32 MS_U32TaskTimer);
INTERFACE MS_BOOL DrvHdmitxOsCreateTimer(DrvHdmitxOsTimerConfig_t *pTimerCfg);
INTERFACE MS_BOOL DrvHdmitxOsDeleteTimer(DrvHdmitxOsTimerConfig_t *pTimerCfg);
INTERFACE MS_BOOL DrvHdmitxOsStopTimer(DrvHdmitxOsTimerConfig_t *pTimerCfg);
INTERFACE MS_BOOL DrvHdmitxOsStartTimer(DrvHdmitxOsTimerConfig_t *pTimerCfg);
INTERFACE MS_BOOL DrvHdmitxOsCreateMutex(DrvHdmitxOsMutexConfig_t *pstMutexCfg);
INTERFACE MS_BOOL DrvHdmitxOsDestroyMutex(DrvHdmitxOsMutexConfig_t *pstMutexCfg);
INTERFACE MS_BOOL DrvHdmitxOsObtainMutex(DrvHdmitxOsMutexConfig_t *pstMutexCfg);
INTERFACE MS_BOOL DrvHdmitxOsReleaseMutex(DrvHdmitxOsMutexConfig_t *pstMutexCfg);
INTERFACE MS_S32  DrvHdmitxOsCreateEventGroup (char *pEventName);
INTERFACE MS_BOOL DrvHdmitxOsDeleteEventGroup (MS_S32 s32EventGroupId);
INTERFACE MS_BOOL DrvHdmitxOsSetEvent (MS_S32 s32EventGroupId, MS_U32 u32EventFlag);
INTERFACE MS_U32  DrvHdmitxOsGetEvent(MS_S32 s32EventGroupId);
INTERFACE MS_BOOL DrvHdmitxOsWaitEvent (MS_S32 s32EventGroupId, MS_U32 u32WaitEventFlag, MS_U32 *pu32RetrievedEventFlag, DrvHdmitxOsEventWaitMoodeType_e eWaitMode, MS_U32 u32WaitMs);
INTERFACE MS_BOOL DrvHdmitxOsAttachInterrupt (MS_U32 u32IntNum, InterruptCallBack pIntCb);
INTERFACE MS_BOOL DrvHdmitxOsDetachInterrupt (MS_U32 u32IntNum);
INTERFACE MS_BOOL DrvHdmitxOsEnableInterrupt (MS_U32 u32IntNum);
INTERFACE MS_BOOL DrvHdmitxOsDisableInterrupt (MS_U32 u32IntNum);
INTERFACE MS_U32  DrvHdmitxOsDisableAllInterrupts(void);
INTERFACE MS_BOOL DrvHdmitxOsRestoreAllInterrupts(MS_U32 u32OldInterrupts);
INTERFACE MS_BOOL DrvHdmitxOsCreateSemaphore(DrvHdmitxOsSemConfig_t *pstSemaphoreCfg, MS_U32 u32InitVal);
INTERFACE MS_BOOL DrvHdmitxOsDestroySemaphore(DrvHdmitxOsSemConfig_t *pstSemaphoreCfg);
INTERFACE void    DrvHdmitxOsObtainSemaphore(DrvHdmitxOsSemConfig_t *pstSemaphoreCfg);
INTERFACE void    DrvHdmitxOsReleaseSemaphore(DrvHdmitxOsSemConfig_t *pstSemaphoreCfg);
INTERFACE MS_BOOL DrvHdmitxOsCreateTask(DrvHdmitxOsTaskConfig_t *pstTaskCfg, TaskEntryCb pTaskEntry, void *pDataPtr, char *pTaskName, MS_BOOL bAuotStart);
INTERFACE MS_BOOL DrvHdmitxOsDestroyTask(DrvHdmitxOsTaskConfig_t *pstTaskCfg);
INTERFACE MS_BOOL DrvHdmitxOsGetMmioBase(MS_VIRT *pu32BaseAddr, MS_PHY *pu32BaseSize, DrvHdmitxOsMmioType_e enType);
INTERFACE MS_BOOL DrvHdmitxOsSetGpioValue(MS_U8 u8GpioNum, MS_U8 u8Val);
INTERFACE MS_BOOL DrvHdmitxOsGetGpioValue(MS_U8 u8GpioNum);
INTERFACE MS_BOOL DrvHdmitxOsGpioRequestOutput(MS_U8 u8GpioNum);
INTERFACE MS_BOOL DrvHdmitxOsGpioRequestInput(MS_U8 u8GpioNum);
INTERFACE MS_BOOL DrvHdmitxOsGpioIrq(MS_U8 u8GpioNum, InterruptCb pIsrCb, MS_BOOL bEn);
INTERFACE void    DrvHdmitxOsSetHpdIrq(MS_BOOL bEn);
INTERFACE MS_BOOL DrvHdmitxOsSetI2cAdapter(MS_U16 u16Id);
INTERFACE MS_BOOL DrvHdmitxOsSetI2cWriteBytes(MS_U8 u8SlaveAdr, MS_U8 u8SubAdr, MS_U8 *pBuf, MS_U16 u16BufLen, MS_BOOL bSendStop);
INTERFACE MS_BOOL DrvHdmitxOsSetI2cReadBytes(MS_U8 u8SlaveAdr, MS_U8 u8SubAdr, MS_U8 *pBuf, MS_U16 u16BufLen);
INTERFACE void*   DrvHdmitxOsMemAlloc(MS_U32 u32Size);
INTERFACE void    DrvHdmitxOsMemRelease(void *pPtr);
INTERFACE void    DrvHdmitxOsMsSleep(MS_U32 u32Msec);
INTERFACE void    DrvHdmitxOsUsSleep(MS_U32 u32Usec);
INTERFACE void    DrvHdmitxOsSetI2cId(MS_U16 u16Id);
INTERFACE MS_U16  DrvHdmitxOsGetI2cId(void);
INTERFACE MS_BOOL DrvHdmitxOsSetDeviceNode(void *pPlatFormDev);
INTERFACE MS_BOOL DrvHdmitxOsSetClkOn(void *pClkRate, MS_U32 u32ClkRateSize);
INTERFACE MS_BOOL DrvHdmitxOsSetClkOff(void);
INTERFACE MS_BOOL DrvHdmitxOsSetHpdGpinPin(MS_U8 u8Pin);
INTERFACE MS_U8   DrvHdmitxOsGetHpdGpinPin(void);
INTERFACE void    DrvHdmitxOsSetSwI2cPin(MS_U8 u8SdaPin, MS_U8 u8SclPin);
INTERFACE void    DrvHdmitxOsSetSwI2cEn(MS_BOOL bEn);
INTERFACE MS_U32  DrvHdmitxOsGetCpuSpeedMhz(void);

#undef INTERFACE

#endif
