/*
* mdrv_miu.c- Sigmastar
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
///////////////////////////////////////////////////////////////////////////////////////////////////
//
// * Copyright (c) 2006 - 2007 Mstar Semiconductor, Inc.
// This program is free software.
// You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation;
// either version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   Mdrvl_miu.c
/// @brief  MIU Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/printk.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include "MsTypes.h"
#include "mdrv_miu.h"
#include "mhal_miu.h"
#include "regMIU.h"
#include <linux/interrupt.h>

#include <linux/string.h>
#include <asm/io.h>
#include <linux/irq.h>

#include "irqs.h"

#ifndef CONFIG_MP_CMA_PATCH_DEBUG_STATIC_MIU_PROTECT
        struct timer_list  monitor_timer;
        int gOpenTimeList = 0;
#endif

//-------------------------------------------------------------------------------------------------
//  Function prototype with weak symbol
//-------------------------------------------------------------------------------------------------

#if defined(CONFIG_MSTAR_MONET) || defined(CONFIG_MSTAR_MESSI) || defined(CONFIG_MSTAR_MASERATI) || defined(CONFIG_MSTAR_MANHATTAN) || defined(CONFIG_MSTAR_KANO)
    MS_BOOL HAL_MIU_Protect(MS_U8 u8Blockx, MS_U16 *pu8ProtectId, MS_PHY phy64Start, MS_PHY phy64End, MS_BOOL bSetFlag) __attribute__((weak));
#else
    MS_BOOL HAL_MIU_Protect(MS_U8 u8Blockx, MS_U16 *pu8ProtectId, MS_U32 u32BusStart, MS_U32 u32BusEnd, PROTECT_CTRL bSetFlag) __attribute__((weak));
#endif

MS_BOOL HAL_MIU_ParseOccupiedResource(void) __attribute__((weak));
MS_U16* HAL_MIU_GetDefaultClientID_KernelProtect(void) __attribute__((weak));
MS_BOOL HAL_MIU_SlitInit(void) __attribute__((weak));
MS_BOOL HAL_MIU_SetSlitRange(MS_U8 u8Blockx, MS_U16 *pu8ProtectId, MS_PHY u64BusStart, MS_PHY u64BusEnd, MS_BOOL  bSetFlag) __attribute__((weak));
MS_BOOL HAL_MIU_Slits(MS_U8 u8Blockx, MS_PHY u64SlitsStart, MS_PHY u64SlitsEnd, MS_BOOL bSetFlag) __attribute__((weak));
unsigned int u32MiuDramSize = 0x40000000UL;

//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------
static DEFINE_SPINLOCK(miu_lock);
MS_U8 u8_MiuWhiteListNum = 0;
//-------------------------------------------------------------------------------------------------
//  Local functions
//-------------------------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_MIU_Init
/// @brief \b Function  \b Description: parse occupied resource to software structure
/// @param None         \b IN :
/// @param None         \b OUT :
/// @param MS_BOOL      \b RET
/// @param None         \b GLOBAL :
////////////////////////////////////////////////////////////////////////////////

MS_BOOL MDrv_MIU_Init(void)
{
    MS_BOOL ret;

    /* Parse the used client ID in hardware into software data structure */
    if(HAL_MIU_ParseOccupiedResource)
    {
        ret = HAL_MIU_ParseOccupiedResource();
    }
    else
    {
        ret = FALSE;
    }

    /* Initialize MIU slit setting */
    if(HAL_MIU_SlitInit)
    {
        ret = HAL_MIU_SlitInit();
    }

    u8_MiuWhiteListNum = IDNUM_KERNELPROTECT;
    return ret;
}
#if (1 == CONFIG_MSTAR_MMAHEAP)
EXPORT_SYMBOL(MDrv_MIU_Init);
EXPORT_SYMBOL(u8_MiuWhiteListNum);
#endif
/////////////////////////////////////////////////////////////////////////
// @brief \n get IDenables in mi layer.
MS_BOOL MDrv_MIU_Get_IDEnables_Value(MS_U8 u8MiuDev, MS_U8 u8Blockx, MS_U8 u8ClientIndex)
{
   return HAL_MIU_Get_IDEnables_Value(u8MiuDev, u8Blockx, u8ClientIndex);
}
EXPORT_SYMBOL(MDrv_MIU_Get_IDEnables_Value);

MS_BOOL MDrv_MIU_GetProtectInfo(MS_U8 u8MiuDev, MIU_PortectInfo *pInfo)
{
    return HAL_MIU_GetProtectInfo(u8MiuDev, pInfo);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MIU_GetDefaultClientID_KernelProtect()
/// @brief \b Function \b Description:  Get default client id array pointer for protect kernel
/// @param <RET>           \b     : The pointer of Array of client IDs
////////////////////////////////////////////////////////////////////////////////
MS_U16* MDrv_MIU_GetDefaultClientID_KernelProtect(void)
{
    if(HAL_MIU_GetDefaultClientID_KernelProtect)
    {
        return HAL_MIU_GetDefaultClientID_KernelProtect();
    }
    else
    {
        return NULL;
    }
}
#if (1 == CONFIG_MSTAR_MMAHEAP)
EXPORT_SYMBOL(MDrv_MIU_GetDefaultClientID_KernelProtect);
#endif
#include <linux/slab.h>

#ifdef CONFIG_MP_CMA_PATCH_DEBUG_STATIC_MIU_PROTECT
static irqreturn_t MDrv_MIU_Protect_interrupt(s32 irq, void *dev_id)
{
        MS_U8 u8MiuDev = 0;
        MIU_PortectInfo *pInfo = kmalloc(sizeof(MIU_PortectInfo), GFP_KERNEL);
        MDrv_MIU_GetProtectInfo(u8MiuDev, pInfo);
        BUG();
        return IRQ_HANDLED;
}
#else
//create timer process
static void MDev_timer_callback(unsigned long value)
{
        MS_U8 u8MiuDev = 0;
        MIU_PortectInfo pInfo;

        pInfo.bHit = FALSE;

        MDrv_MIU_GetProtectInfo(u8MiuDev, &pInfo);
        if(pInfo.bHit)
                /* panic("MIU Protect hit!\n"); */
                printk(KERN_ERR "MIU Protect hit!\n");

        mod_timer(&monitor_timer, jiffies+1*HZ);
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MIU_Protect()
/// @brief \b Function \b Description:  Enable/Disable MIU Protection mode
/// @param u8Blockx        \b IN     : MIU Block to protect (0 ~ 3)
/// @param *pu8ProtectId   \b IN     : Allow specified client IDs to write
/// @param u32Start        \b IN     : Starting address(bus address)
/// @param u32End          \b IN     : End address(bus address)
/// @param bSetFlag        \b IN     : Disable or Enable MIU protection
///                                      - -Disable(0)
///                                      - -Enable(1)
/// @param <OUT>           \b None    :
/// @param <RET>           \b None    :
/// @param <GLOBAL>        \b None    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MIU_Protect(
                          MS_U8    u8Blockx,
                          MS_U16    *pu8ProtectId,
                          MS_PHY   u64BusStart,
                          MS_PHY   u64BusEnd,
                          PROTECT_CTRL  bSetFlag
                         )
{
    MS_BOOL Result = FALSE;

#ifdef CONFIG_MP_CMA_PATCH_DEBUG_STATIC_MIU_PROTECT
                int rc = 0;
#endif
    /*Case of former MIU protect*/
    if((u8Blockx >= E_PROTECT_0) && (u8Blockx < E_SLIT_0))
    {
        if(HAL_MIU_Protect)
        {
            Result = HAL_MIU_Protect(u8Blockx, pu8ProtectId, u64BusStart, u64BusEnd, bSetFlag);
        }
    }
    /*Case of MIU slits*/
    else if (u8Blockx == E_SLIT_0)
    {
        if(HAL_MIU_SetSlitRange)
        {
            Result = HAL_MIU_SetSlitRange(u8Blockx, pu8ProtectId, u64BusStart, u64BusEnd, bSetFlag);
        }
    }

#ifdef CONFIG_MP_CMA_PATCH_DEBUG_STATIC_MIU_PROTECT
        rc = request_irq(( GIC_PPI_NR + GIC_SGI_NR + INT_IRQ_35_MIU_INT), MDrv_MIU_Protect_interrupt, IRQF_TRIGGER_HIGH, "MIU_Protect", NULL);
#else
        if(gOpenTimeList == 0 )
        {
                init_timer( &monitor_timer );
            monitor_timer.function = MDev_timer_callback;
            monitor_timer.expires = jiffies+HZ;
                add_timer(&monitor_timer);
                gOpenTimeList++;
        }
#endif


        return Result;
}

#if (1 == CONFIG_MSTAR_MMAHEAP)
EXPORT_SYMBOL(MDrv_MIU_Protect);
#endif

MS_BOOL MDrv_MIU_Save(void)
{
        MS_BOOL Result = FALSE;
        #if 0
        ///TODO:now k6l do not have HAL_MIU_Save,even main trunk k6l do not have.
        //BUT now we need open CONFIG_MSTAR_MIU,though not use MDrv_MIU_Save ,
         //here we keep MDrv_MIU_Save ,but git a BUG_ON for build success.
        //Future owner should add HAL_MIU_Save for k6l if k6l need both STR and CONFIG_MSTAR_MIU enable.

        Result = HAL_MIU_Save();
        #else
        BUG_ON(1);//give this BUG_ON to show owner that we need HAL_MIU_Save for k6l.
        #endif
        return Result;
}

MS_BOOL MDrv_MIU_Restore(void)
{
        MS_BOOL Result = FALSE;
        #if 0
        ///TODO:now k6l do not have HAL_MIU_Restore,even main trunk k6l do not have.
        //BUT now we need open CONFIG_MSTAR_MIU,though not use MDrv_MIU_Restore ,
         //here we keep MDrv_MIU_Restore ,but git a BUG_ON for build success.
        //Future owner should add HAL_MIU_Save for k6l if k6l need both STR and CONFIG_MSTAR_MIU enable

        Result = HAL_MIU_Restore();
        #else
        BUG_ON(1);//give this BUG_ON to show owner that we need HAL_MIU_Restore for k6l.
        #endif
        return Result;
}

unsigned int MDrv_MIU_ProtectDramSize(void)
{
    return HAL_MIU_ProtectDramSize();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_SetSsc()
/// @brief \b Function \b Description: _MDrv_MIU_SetSsc, @Step & Span
/// @param u16Fmodulation   \b IN : 20KHz ~ 40KHz (Input Value = 20 ~ 40)
/// @param u16FDeviation    \b IN  : under 0.1% ~ 3% (Input Value = 1 ~ 30)
/// @param bEnable          \b IN    :
/// @param None             \b OUT  :
/// @param None             \b RET  :
/// @param None             \b GLOBAL :
////////////////////////////////////////////////////////////////////////////////
unsigned char MDrv_MIU_SetSsc(unsigned short u16Fmodulation, unsigned short u16FDeviation, unsigned char bEnable)
{
    return (unsigned char)HAL_MIU_SetSsc((MS_U8)0, (MS_U16)u16Fmodulation, (MS_U16)u16FDeviation, (MS_BOOL)bEnable);
}
EXPORT_SYMBOL(MDrv_MIU_SetSsc);

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: _MDrv_MIU_SetSscValue()
/// @brief \b Function \b Description: _MDrv_MIU_SetSscValue, @Step & Span
/// @param u8Miu            \b IN   : 0: MIU0 1:MIU1
/// @param u16Fmodulation   \b IN   : 20KHz ~ 40KHz (Input Value = 20 ~ 40)
/// @param u16FDeviation    \b IN   : under 0.1% ~ 3% (Input Value = 1 ~ 30)
/// @param bEnable          \b IN   :
/// @param None             \b OUT  :
/// @param None             \b RET  :
/// @param None             \b GLOBAL :
////////////////////////////////////////////////////////////////////////////////
unsigned char MDrv_MIU_SetSscValue(unsigned char u8MiuDev, unsigned short u16Fmodulation, unsigned short u16FDeviation, unsigned char bEnable)
{
    return (unsigned char)HAL_MIU_SetSsc((MS_U8)u8MiuDev, (MS_U16)u16Fmodulation, (MS_U16)u16FDeviation, (MS_BOOL)bEnable);
}
EXPORT_SYMBOL(MDrv_MIU_SetSscValue);

//#ifdef CONFIG_MP_CMA_PATCH_DEBUG_STATIC_MIU_PROTECT
////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function  \b Name: MDrv_MIU_GetProtectInfo()
/// @brief \b Function  \b Description:  This function for querying client ID info
/// @param u8MiuDev     \b IN   : select MIU0 or MIU1
/// @param eClientID    \b IN   : Client ID
/// @param pInfo        \b OUT  : Client Info
/// @param None \b RET:   0: Fail 1: Ok
////////////////////////////////////////////////////////////////////////////////

//#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MIU_Slits()
/// @brief \b Function \b Description:  Enable/Disable MIU Slit
/// @param u8Blockx        \b IN     : MIU Block to protect (E_MIU_SLIT_0)
/// @param u32Start        \b IN     : Starting address(bus address)
/// @param u32End          \b IN     : End address(bus address)
/// @param bSetFlag        \b IN     : Disable or Enable MIU protection
///                                      - -Disable(0)
///                                      - -Enable(1)
/// @param <OUT>           \b None    :
/// @param <RET>           \b None    :
/// @param <GLOBAL>        \b None    :
////////////////////////////////////////////////////////////////////////////////
MS_BOOL MDrv_MIU_Slits(MS_U8 u8Blockx, MS_PHY u64SlitsStart, MS_PHY u64SlitsEnd, MS_BOOL bSetFlag)
{
    unsigned long flags;
    MS_BOOL Result = FALSE;

    spin_lock_irqsave(&miu_lock, flags);

    if(HAL_MIU_Slits)
    {
        Result = HAL_MIU_Slits(u8Blockx, u64SlitsStart, u64SlitsEnd, bSetFlag);
    }

    spin_unlock_irqrestore(&miu_lock, flags);

    return Result;
}

static int mstar_miu_drv_probe(struct platform_device *pdev)
{
    u32MiuDramSize = MDrv_MIU_ProtectDramSize();
    return 0;
}

static const struct of_device_id ms_uart_of_match_table[] = {
    { .compatible = "mstar,miu" },
    {}
};

static int mstar_miu_drv_remove(struct platform_device *pdev)
{
        del_timer(&monitor_timer);
    return 0;
}

static int mstar_miu_drv_suspend(struct platform_device *dev, pm_message_t state)
{
#ifndef CONFIG_MSTAR_CMAPOOL
#if 0
        ///TODO:now k6l do not have HAL_MIU_Save,even main trunk k6l do not have.
        //BUT now we need open CONFIG_MSTAR_MIU,
         //here we keep mstar_miu_drv_suspend ,but git a BUG_ON for build success.
        //Future owner should add HAL_MIU_Save for k6l if k6l need both STR and CONFIG_MSTAR_MIU enable

    HAL_MIU_Save();
#else
    BUG_ON(1);
#endif


#endif
    return 0;
}

static int mstar_miu_drv_resume(struct platform_device *dev)
{
#ifndef CONFIG_MSTAR_CMAPOOL

#if 0
        ///TODO:now k6l do not have HAL_MIU_Restore,even main trunk k6l do not have.
        //BUT now we need open CONFIG_MSTAR_MIU,
         //here we keep mstar_miu_drv_resume ,but git a BUG_ON for build success.
        //Future owner should add HAL_MIU_Restore for k6l if k6l need both STR and CONFIG_MSTAR_MIU enable

    HAL_MIU_Restore();
#else
    BUG_ON(1);
#endif

#endif
    return 0;
}

#if defined (CONFIG_ARM64)
static struct of_device_id mstarmiu_of_device_ids[] = {
         {.compatible = "mstar-miu"},
         {},
};
#endif

static struct platform_driver Mstar_miu_driver = {
    .probe      = mstar_miu_drv_probe,
    .remove     = mstar_miu_drv_remove,
    .suspend    = mstar_miu_drv_suspend,
    .resume     = mstar_miu_drv_resume,
    .driver = {
    .name       = "Mstar-miu",
#if defined(CONFIG_ARM64)
    .of_match_table = mstarmiu_of_device_ids,
#endif
    .owner  = THIS_MODULE,
    }
};
EXPORT_SYMBOL(u32MiuDramSize);
static int __init mstar_miu_drv_init_module(void)
{
//    retval = platform_driver_register(&Mstar_miu_driver);
    int ret = 0;
    ret = platform_driver_register(&Mstar_miu_driver);
    if (ret)
    {
        printk("Register Mstar MIU Platform Driver Failed!");
    }
    return ret;
}

static void __exit mstar_miu_drv_exit_module(void)
{
    platform_driver_unregister(&Mstar_miu_driver);
}

module_init(mstar_miu_drv_init_module);
module_exit(mstar_miu_drv_exit_module);

MODULE_AUTHOR("MSTAR");
MODULE_DESCRIPTION("MIU driver");
MODULE_LICENSE("GPL");
