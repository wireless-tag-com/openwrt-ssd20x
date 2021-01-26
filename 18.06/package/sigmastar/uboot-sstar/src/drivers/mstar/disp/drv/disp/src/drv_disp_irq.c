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

#define _DRV_DISP_IRQ_C_

#include "drv_disp_os.h"
#include "hal_disp_common.h"
#include "hal_disp_if.h"
#include "hal_disp_irq.h"
#include "disp_debug.h"
#include "mhal_common.h"
#include "mhal_disp_datatype.h"
#include "hal_disp_chip.h"
#include "hal_disp_st.h"
#include "drv_disp_ctx.h"
//-------------------------------------------------------------------------------------------------
//  Include Files
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define ENABLE_INTENAL_IRQ_DBG  0

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    u32 u32IrqNum;
    bool bInit;
}DrvDispIrqConfig_t;
//-------------------------------------------------------------------------------------------------
//  Variable
//-------------------------------------------------------------------------------------------------
DrvDispIrqConfig_t gstDispIrqCfg[HAL_DISP_DEVICE_IRQ_MAX];
bool gbDispIrqInit = 0;
extern u32 gu32InternalIsrFlag ;
//-------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------
void DrvDispIrqSetIsrNum(u32 u32DevId, u32 u32IsrNum)
{
    if(u32DevId >= HAL_DISP_DEVICE_IRQ_MAX)
    {
        DISP_ERR("%s %d, DevId too big: %ld\n", __FUNCTION__, __LINE__, u32DevId);
        return;
    }
    gstDispIrqCfg[u32DevId].bInit = 1;
    gstDispIrqCfg[u32DevId].u32IrqNum = u32IsrNum;
}

bool DrvDispIrqGetIsrNum(void *pDevCtx, u32 *pu32IsrNum)
{
    bool bRet = 1;
    DrvDispCtxConfig_t *pstDispCtx = (DrvDispCtxConfig_t *)pDevCtx;

    if(pstDispCtx->enCtxType == E_DISP_CTX_TYPE_DEVICE)
    {
        if(gstDispIrqCfg[pstDispCtx->u32Idx].bInit)
        {
            *pu32IsrNum = gstDispIrqCfg[pstDispCtx->u32Idx].u32IrqNum;
            DISP_DBG(DISP_DBG_LEVEL_IRQ, "%s %d, Id=%ld, Irq=%ld\n",
                __FUNCTION__, __LINE__,
                pstDispCtx->u32Idx, *pu32IsrNum);
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, Irq not init\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, CtxType=%s, Wrong\n",
            __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pstDispCtx->enCtxType));
    }
    return bRet;
}


bool DrvDispIrqEnable(void *pDevCtx, u32 u32DevIrq, bool bEnable)
{
    bool bRet = 1;
    DrvDispCtxConfig_t *pstDispCtx = (DrvDispCtxConfig_t *)pDevCtx;
    if(pstDispCtx->enCtxType == E_DISP_CTX_TYPE_DEVICE)
    {
        if(gstDispIrqCfg[pstDispCtx->u32Idx].u32IrqNum == u32DevIrq)
        {
            HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_ENABLE, E_HAL_DISP_IRQ_TYPE_VSYNC, (void *)&bEnable);
        }
        else
        {
            bRet = 0;
            DISP_ERR("%s %d, IrqNum not match %ld != %ld\n",
                __FUNCTION__, __LINE__, u32DevIrq, gstDispIrqCfg[pstDispCtx->u32Idx].u32IrqNum);
        }
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, CtxType=%s, Wrong\n",
            __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pstDispCtx->enCtxType));
    }
    return bRet;
}

bool DrvDispIrqGetFlag(void *pDevCtx, MHAL_DISP_IRQFlag_t *pstIrqFlag)
{
    bool bRet = 1;
    u32 u32IrqFlag;
    DrvDispCtxConfig_t *pstDispCtx = (DrvDispCtxConfig_t *)pDevCtx;

    if(pstDispCtx->enCtxType == E_DISP_CTX_TYPE_DEVICE)
    {
        HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_GET_FLAG, E_HAL_DISP_IRQ_TYPE_VSYNC, (void *)&u32IrqFlag);
        pstIrqFlag->u32IrqFlag = u32IrqFlag;
        pstIrqFlag->u32IrqMask = E_HAL_DISP_IRQ_TYPE_VSYNC;
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, CtxType=%s, Wrong\n",
            __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pstDispCtx->enCtxType));
    }
    return bRet;

}

bool DrvDispIrqClear(void *pDevCtx, void* pData)
{
    bool bRet = 1;
    DrvDispCtxConfig_t *pstDispCtx = (DrvDispCtxConfig_t *)pDevCtx;
    MHAL_DISP_IRQFlag_t *pstIrqFlag = (MHAL_DISP_IRQFlag_t *)pData;

    if(pstDispCtx->enCtxType == E_DISP_CTX_TYPE_DEVICE)
    {
        HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_CLEAR, pstIrqFlag->u32IrqFlag & pstIrqFlag->u32IrqMask, NULL);
    }
    else
    {
        bRet = 0;
        DISP_ERR("%s %d, CtxType=%s, Wrong\n",
            __FUNCTION__, __LINE__, PARSING_CTX_TYPE(pstDispCtx->enCtxType));
    }
    return bRet;
}

//------------------------------------------------------------------------------
// Internal Isr
//------------------------------------------------------------------------------

#if HAL_DISP_INTERNAL_ISR_SUPPORT
static irqreturn_t _DrvDispIrqInternalIsrCb(int eIntNum, void* pstDevParam)
{
    u32 u32Flag;
#if ENABLE_INTENAL_IRQ_DBG
    static u8 u8Num[4] = {0, 0, 0, 0};
    static u32 u32UnknownFlag[50];
    static u8 u8Cnt = 0;
    u8 i;
#endif

    HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_INTERNAL_GET_FLAG,
                    E_HAL_DISP_IRQ_TYPE_INTERNAL_TIMEZONE,
                    (void *)&u32Flag);

    HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_INTERNAL_CLEAR, u32Flag, NULL);

#if ENABLE_INTENAL_IRQ_DBG

    if(u32Flag == E_HAL_DISP_IRQ_TYPE_INTERNAL_VSYNC_POSITIVE)
    {
        u8Num[0]++;
    }
    else if(u32Flag == E_HAL_DISP_IRQ_TYPE_INTERNAL_VDE_POSITIVE)
    {
        u8Num[1]++;
    }
    else if(u32Flag == E_HAL_DISP_IRQ_TYPE_INTERNAL_VDE_NEGATIVE)
    {
        u8Num[2]++;
    }
    else
    {
        u32UnknownFlag[u8Num[3]] = u32Flag;
        u8Num[3]++;
    }
    u8Cnt++;
    if(u8Cnt==150)
    {
        DISP_DBG(DISP_DBG_LEVEL_IRQ_INTERNAL, "%s %d:: BP:%d, Act:%d FP:%d, Unknown:%d\n", __FUNCTION__, __LINE__, u8Num[0], u8Num[1], u8Num[2], u8Num[3]);

        if(u8Num[3])
        {
            for(i=0; i<u8Num[3]; i++)
            {
                DISP_DBG(DISP_DBG_LEVEL_IRQ_INTERNAL, "%x\n", u32UnknownFlag[i]);
            }
        }

        u8Cnt =0;
        u8Num[0] = 0;
        u8Num[1] = 0;
        u8Num[2] = 0;
        u8Num[3] = 0;
    }
#endif

    return IRQ_HANDLED;
}
#endif

bool _DrvDispCreateTimeZoneIsr(void)
{
#if HAL_DISP_INTERNAL_ISR_SUPPORT
    s32 s32IrqRet;
    u32 u32IrqNum;
    bool bSupported;

    HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_INTERNAL_SUPPORTED, E_HAL_DISP_IRQ_TYPE_NONE, (void *)&bSupported);
    if(bSupported)
    {
        u8 u8DeviceIdx;
        bool bEnable;

        HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_INTERNAL_GET_ID, E_HAL_DISP_IRQ_TYPE_NONE, (void *)&u8DeviceIdx);
        u32IrqNum = gstDispIrqCfg[u8DeviceIdx].u32IrqNum;

        s32IrqRet = request_irq(u32IrqNum, _DrvDispIrqInternalIsrCb, IRQF_TRIGGER_NONE, "mdisp_interisr", NULL);
        if(s32IrqRet)
        {
            DISP_ERR("Attach Irq Fail\n");
            return 0;
        }
        else
        {
            DISP_DBG(DISP_DBG_LEVEL_IRQ_INTERNAL, "%s %d, IrqNum=%d\n",
                __FUNCTION__, __LINE__, u32IrqNum);
        }

        disable_irq(u32IrqNum);
        enable_irq(u32IrqNum);

        bEnable = 1;
        HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_INTERNAL_ENABLE,
                        E_HAL_DISP_IRQ_TYPE_INTERNAL_TIMEZONE,
                        (void *)&bEnable);
    }
    else
    {
        DISP_DBG(DISP_DBG_LEVEL_IRQ_INTERNAL, "%s %d, Not Support\n", __FUNCTION__, __LINE__);
    }
#endif

    return 1;
}


#if HAL_DISP_INTERNAL_ISR_SUPPORT
static irqreturn_t _DrvDispIrqVgaHpdIsrCb(int eIntNum, void* pstDevParam)
{
    u32 u32Flag;

    HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_VGA_HPD_GET_FLAG,
                    E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON_OFF,
                    (void *)&u32Flag);

    HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_VGA_HPD_CLEAR, u32Flag, NULL);

    HalDispIrqSetDacEn((u32Flag & E_HAL_DISP_IRQ_TYPE_VGA_HPD_OFF) ? 0 : 1);

    DISP_DBG(DISP_DBG_LEVEL_IRQ_VGA_HPD, "%s %d VgaHpd:%s\n",
        __FUNCTION__, __LINE__, (u32Flag & E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON) ? "ON":"OFF");

    return IRQ_HANDLED;
}
#endif

bool _DrvDispIrqCreateVgaHpdIsr(void)
{
#if HAL_DISP_INTERNAL_ISR_SUPPORT
    s32 s32IrqRet;
    u32 u32IrqNum;
    bool bSupported;

    HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_VGA_HPD_SUPPORTED, E_HAL_DISP_IRQ_TYPE_NONE, (void *)&bSupported);
    if(bSupported)
    {
        u8 u8DeviceIdx;
        bool bEnable;

        HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_VGA_HPD_GET_ID, E_HAL_DISP_IRQ_TYPE_NONE, (void *)&u8DeviceIdx);
        u32IrqNum = gstDispIrqCfg[u8DeviceIdx].u32IrqNum;

        s32IrqRet = request_irq(u32IrqNum, _DrvDispIrqVgaHpdIsrCb, IRQF_TRIGGER_NONE, "mdisp_vgahpdisr", NULL);
        if(s32IrqRet)
        {
            DISP_ERR("Attach Irq Fail\n");
            return 0;
        }
        else
        {
            DISP_DBG(DISP_DBG_LEVEL_IRQ_VGA_HPD, "%s %d, IrqNum=%d\n",
                __FUNCTION__, __LINE__, u32IrqNum);
        }

        disable_irq(u32IrqNum);
        enable_irq(u32IrqNum);

        bEnable = 1;
        HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_VGA_HPD_ENABLE,
                        E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON_OFF,
                        (void *)&bEnable);
    }
    else
    {
        DISP_DBG(DISP_DBG_LEVEL_IRQ_VGA_HPD, "%s %d, Not Support\n", __FUNCTION__, __LINE__);
    }
#endif
    return 1;
}

bool _DrvDispIrqDestroyTimeZoneIsr(void)
{
#if HAL_DISP_INTERNAL_ISR_SUPPORT
    u32 u32IrqNum;
    bool bSupported;

    HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_INTERNAL_SUPPORTED, E_HAL_DISP_IRQ_TYPE_NONE, (void *)&bSupported);

    if(bSupported)
    {
        u8 u8DeviceIdx;
        bool bEnable;

        HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_INTERNAL_GET_ID, E_HAL_DISP_IRQ_TYPE_NONE, (void *)&u8DeviceIdx);
        u32IrqNum = gstDispIrqCfg[u8DeviceIdx].u32IrqNum;
        free_irq(u32IrqNum, NULL);
        disable_irq(u32IrqNum);
        bEnable = 0;
        HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_INTERNAL_ENABLE,
                        E_HAL_DISP_IRQ_TYPE_INTERNAL_TIMEZONE,
                        (void *)&bEnable);
    }
    else
    {
        DISP_DBG(DISP_DBG_LEVEL_IRQ_INTERNAL, "%s %d, Not Support\n", __FUNCTION__, __LINE__);
    }
#endif
    return 1;
}

bool _DrvDispIrqDestroyVgaHpdIsr(void)
{
#if HAL_DISP_INTERNAL_ISR_SUPPORT
    u32 u32IrqNum;
    bool bSupported;

    HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_VGA_HPD_SUPPORTED, E_HAL_DISP_IRQ_TYPE_NONE, (void *)&bSupported);

    if(bSupported)
    {
        u8 u8DeviceIdx;
        bool bEnable;

        HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_VGA_HPD_GET_ID, E_HAL_DISP_IRQ_TYPE_NONE, (void *)&u8DeviceIdx);
        u32IrqNum = gstDispIrqCfg[u8DeviceIdx].u32IrqNum;
        free_irq(u32IrqNum, NULL);
        disable_irq(u32IrqNum);
        bEnable = 0;
        HalDispIrqIoCtl(E_HAL_DISP_IRQ_IOCTL_VGA_HPD_ENABLE,
                        E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON_OFF,
                        (void *)&bEnable);
    }
    else
    {
        DISP_DBG(DISP_DBG_LEVEL_IRQ_VGA_HPD, "%s %d, Not Support\n", __FUNCTION__, __LINE__);
    }
#endif
    return 1;
}



bool DrvDispIrqCreateInternalIsr(void)
{
    bool bRet = 1;

    bRet &= _DrvDispCreateTimeZoneIsr();
    //bRet &= _DrvDispIrqCreateVgaHpdIsr();
    return bRet;
}


bool DrvDispIrqDestroyInternalIsr(void)
{
    bool bRet = 1;

    bRet &= _DrvDispIrqDestroyTimeZoneIsr();
    //bRet &= _DrvDispIrqDestroyVgaHpdIsr();
    return bRet;
}


