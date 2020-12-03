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
#include <linux/printk.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/irq.h>
#include <linux/slab.h>

#include "MsTypes.h"
#include "mdrv_miu.h"
#include "mhal_miu.h"
#include "regMIU.h"
#include "ms_platform.h"
#include <asm/io.h>
#include "irqs.h"
#include <linux/of.h>
#include <linux/of_irq.h>

//-------------------------------------------------------------------------------------------------
//  Function prototype with weak symbol
//-------------------------------------------------------------------------------------------------

#if defined(CONFIG_MSTAR_MONET) || defined(CONFIG_MSTAR_MESSI) || defined(CONFIG_MSTAR_MASERATI) || defined(CONFIG_MSTAR_MANHATTAN) || defined(CONFIG_MSTAR_KANO)
MS_BOOL HAL_MIU_Protect(MS_U8 u8Blockx, MS_U16 *pu8ProtectId, MS_PHY phy64Start, MS_PHY phy64End, MS_BOOL bSetFlag) __attribute__((weak));
#else
MS_BOOL HAL_MIU_Protect(MS_U8 u8Blockx, MS_U16 *pu8ProtectId, MS_U32 u32BusStart, MS_U32 u32BusEnd, MS_BOOL bSetFlag) __attribute__((weak));
#endif
MS_BOOL HAL_MIU_GetHitProtectInfo(MS_U8 u8MiuSel, MIU_PortectInfo *pInfo) __attribute__((weak));
MS_BOOL HAL_MIU_ParseOccupiedResource(void) __attribute__((weak));
MS_U16* HAL_MIU_GetDefaultKernelProtectClientID(void) __attribute__((weak));

MS_BOOL HAL_MIU_SlitInit(void) __attribute__((weak));
MS_BOOL HAL_MIU_SetSlitRange(MS_U8 u8Blockx, MS_U16 *pu8ProtectId, MS_PHY u64BusStart, MS_PHY u64BusEnd, MS_BOOL  bSetFlag) __attribute__((weak));
MS_BOOL HAL_MIU_Slits(MS_U8 u8Blockx, MS_PHY u64SlitsStart, MS_PHY u64SlitsEnd, MS_BOOL bSetFlag) __attribute__((weak));

//--------------------------------------------------------------------------------------------------
//  Local variable
//--------------------------------------------------------------------------------------------------

#ifndef CONFIG_MP_CMA_PATCH_DEBUG_STATIC_MIU_PROTECT
static struct timer_list monitor_timer;
#endif
static int bMiuProtect_is_initialized = 0;

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
    MS_BOOL ret = FALSE;

    /* Parse the used client ID in hardware into software data structure */
    if (HAL_MIU_ParseOccupiedResource) {
        ret = HAL_MIU_ParseOccupiedResource();
    }
    else {
        ret = FALSE;
    }

    /* Initialize MIU slit setting */
    if (HAL_MIU_SlitInit) {
        ret = HAL_MIU_SlitInit();
    }

    u8_MiuWhiteListNum = IDNUM_KERNELPROTECT;

    return ret;
}

MS_BOOL MDrv_MIU_GetProtectInfo(MS_U8 u8MiuSel, MIU_PortectInfo *pInfo)
{
    return HAL_MIU_GetHitProtectInfo(u8MiuSel, pInfo);
}

MS_BOOL MDrv_MIU_Get_IDEnables_Value(MS_U8 u8MiuSel, MS_U8 u8Blockx, MS_U8 u8ClientIndex)
{
   return HAL_MIU_GetProtectIdEnVal(u8MiuSel, u8Blockx, u8ClientIndex);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief \b Function \b Name: MDrv_MIU_GetDefaultClientID_KernelProtect()
/// @brief \b Function \b Description:  Get default client id array pointer for protect kernel
/// @param <RET>           \b     : The pointer of Array of client IDs
////////////////////////////////////////////////////////////////////////////////
MS_U16* MDrv_MIU_GetDefaultClientID_KernelProtect(void)
{
    if (HAL_MIU_GetDefaultKernelProtectClientID) {
        return HAL_MIU_GetDefaultKernelProtectClientID();
    }
    else {
        return NULL;
    }
}

#ifdef CONFIG_MP_CMA_PATCH_DEBUG_STATIC_MIU_PROTECT
static irqreturn_t MDrv_MIU_Protect_interrupt(s32 irq, void *dev_id)
{
    MS_U8 u8MiuSel = 0;
    MIU_PortectInfo pInfo;

    memset(&pInfo, 0 , sizeof(pInfo));

    MDrv_MIU_GetProtectInfo(u8MiuSel, &pInfo);
    if (pInfo.bHit)
        BUG();
    return IRQ_HANDLED;
}
#else
//create timer process
static void MDev_timer_callback(unsigned long value)
{
    MS_U8 u8MiuSel = 0;
    MIU_PortectInfo stProtInfo;

    for (u8MiuSel = 0; u8MiuSel < MIU_MAX_DEVICE; u8MiuSel++) {
        stProtInfo.bHit = FALSE;

        MDrv_MIU_GetProtectInfo(u8MiuSel, &stProtInfo);
        if (stProtInfo.bHit) {
            panic("MIU %d Protect hit!\n", u8MiuSel);
        }
    }
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
MS_BOOL MDrv_MIU_Protect(   MS_U8   u8Blockx,
                            MS_U16  *pu8ProtectId,
                            MS_PHY  u64BusStart,
                            MS_PHY  u64BusEnd,
                            MS_BOOL bSetFlag)
{
    MS_BOOL Result = FALSE;

    /* Case of former MIU protect */
    if ((u8Blockx >= E_PROTECT_0) && (u8Blockx < E_SLIT_0))
    {
        if (HAL_MIU_Protect) {
            if (TRUE != (Result = HAL_MIU_Protect(u8Blockx, pu8ProtectId, u64BusStart, u64BusEnd, bSetFlag)))
            {
                goto MDrv_MIU_Protect_Exit;
            }
        }
    }
    /* Case of MIU slits */
    else if (u8Blockx == E_SLIT_0)
    {
        if (HAL_MIU_SetSlitRange) {
            if (TRUE != (Result = HAL_MIU_SetSlitRange(u8Blockx, pu8ProtectId, u64BusStart, u64BusEnd, bSetFlag)))
            {
                goto MDrv_MIU_Protect_Exit;
            }
        }
    }


    if(!bMiuProtect_is_initialized)
    {
#ifdef CONFIG_MP_CMA_PATCH_DEBUG_STATIC_MIU_PROTECT
        struct device_node  *dev_node = NULL;
        int rc = 0;
        int iIrqNum = 0;
        dev_node = of_find_compatible_node(NULL, NULL, "sstar,miu");
        if (!dev_node)
        {
            printk("[MIU Protecrt] find node Fail\r\n");
            Result = FALSE;
            goto MDrv_MIU_Protect_Exit;
        }

        iIrqNum = irq_of_parse_and_map(dev_node, 0);

        if(0 != (rc = request_irq(iIrqNum, MDrv_MIU_Protect_interrupt, IRQF_TRIGGER_HIGH, "MIU_Protect", NULL)))
        {
            printk("[MIU Protecrt] request_irq [%d] Fail, Err:%d\r\n", iIrqNum, rc);
            Result = FALSE;
            goto MDrv_MIU_Protect_Exit;
        }
#else
        init_timer(&monitor_timer);
        monitor_timer.function = MDev_timer_callback;
        monitor_timer.expires  = jiffies+HZ;
        add_timer(&monitor_timer);
#endif
        bMiuProtect_is_initialized++;
    }


MDrv_MIU_Protect_Exit:
    return Result;
}

unsigned int MDrv_MIU_ProtectDramSize(void)
{
    return HAL_MIU_ProtectDramSize();
}

int MDrv_MIU_ClientIdToName(MS_U16 clientId, char *clientName)
{
    return HAL_MIU_ClientIdToName(clientId, clientName);
}

MS_BOOL MDrv_MIU_Slits(MS_U8 u8Blockx, MS_PHY u64SlitsStart, MS_PHY u64SlitsEnd, MS_BOOL bSetFlag)
{
    unsigned long flags;
    MS_BOOL Result = FALSE;

    spin_lock_irqsave(&miu_lock, flags);

    if (HAL_MIU_Slits) {
        Result = HAL_MIU_Slits(u8Blockx, u64SlitsStart, u64SlitsEnd, bSetFlag);
    }

    spin_unlock_irqrestore(&miu_lock, flags);

    return Result;
}

int MDrv_MIU_Enable_Goup1_Smaller_Group2(unsigned char u8Enable)
{
    unsigned long flags;
    int ret = 0 ;

    spin_lock_irqsave(&miu_lock, flags);
    ret = HAL_MIU_Enable_Goup1_Smaller_Group2(u8Enable);
    spin_unlock_irqrestore(&miu_lock, flags);

    return ret;
}
EXPORT_SYMBOL(MDrv_MIU_Enable_Goup1_Smaller_Group2);

#ifdef CONFIG_MSTAR_MMAHEAP
EXPORT_SYMBOL(MDrv_MIU_Init);
EXPORT_SYMBOL(u8_MiuWhiteListNum);
EXPORT_SYMBOL(MDrv_MIU_GetDefaultClientID_KernelProtect);
EXPORT_SYMBOL(MDrv_MIU_Protect);
#endif
EXPORT_SYMBOL(MDrv_MIU_Get_IDEnables_Value);

#ifdef CONFIG_MIU_HW_MMU
static DEFINE_SPINLOCK(mmu_lock);

#ifdef CONFIG_MMU_INTERRUPT_ENABLE
static int mmu_isr_init=0;
MDrv_MMU_Callback MDrv_MMU_Notify=NULL;
#endif

int MDrv_MMU_SetRegion(unsigned short u16Region, unsigned short u16ReplaceRegion)
{
    unsigned long flags;
    int ret;

    if(u16Region ==u16ReplaceRegion)
    {
        spin_lock_irqsave(&mmu_lock, flags);
        ret = HAL_MMU_SetRegion(u16Region);
        spin_unlock_irqrestore(&mmu_lock, flags);
    }
    else
    {
#ifndef CONFIG_FPGA
        if(Chip_Get_Revision() == 0x1)
        {
            printk("[%s] not support , chipId = %d \n", __FUNCTION__, Chip_Get_Revision() );
            return -1;
        }
#endif
        spin_lock_irqsave(&mmu_lock, flags);
        ret = HAL_MMU_SetRegionReplaceable(u16Region,u16ReplaceRegion);
        spin_unlock_irqrestore(&mmu_lock, flags);
    }

    return ret;
}

int MDrv_MMU_SetPageSize(unsigned char u8PgSz256En)
{
    if(0 != u8PgSz256En)
    {
        printk("[%s] not support u8PgSz256En = %d \n", __FUNCTION__, u8PgSz256En);
        return -1 ;
    }

    return 0;
}

int MDrv_MMU_Map(unsigned short u16PhyAddrEntry, unsigned short u16VirtAddrEntry)
{
    unsigned long flags;
    int ret;

    if (u16VirtAddrEntry == MMU_INVALID_ENTRY_VAL)
    {
        printk("[%s] Virtual address entry is invalid(0x%X)\n", __FUNCTION__, u16VirtAddrEntry);
        return -1;
    }

    spin_lock_irqsave(&mmu_lock, flags);
    ret = HAL_MMU_Map(u16PhyAddrEntry, u16VirtAddrEntry);
    spin_unlock_irqrestore(&mmu_lock, flags);

    return ret;
}

unsigned short MDrv_MMU_MapQuery(unsigned short u16PhyAddrEntry)
{
    unsigned long flags;
    unsigned short entry;

    spin_lock_irqsave(&mmu_lock, flags);
    entry = HAL_MMU_MapQuery(u16PhyAddrEntry);
    spin_unlock_irqrestore(&mmu_lock, flags);

    return entry;
}

int MDrv_MMU_UnMap(unsigned short u16PhyAddrEntry)
{
    unsigned long flags;
    int ret;

    spin_lock_irqsave(&mmu_lock, flags);
    ret = HAL_MMU_UnMap(u16PhyAddrEntry);
    spin_unlock_irqrestore(&mmu_lock, flags);

    return ret;
}

int MDrv_MMU_AddClientId(unsigned short u16ClientId)
{
    unsigned long flags;
    int ret;

#ifndef CONFIG_FPGA
    if(Chip_Get_Revision() == 0x2)
    {
        printk("[%s] not support , chipId = %d \n", __FUNCTION__, Chip_Get_Revision() );
        return -1;
    }
#endif

    spin_lock_irqsave(&mmu_lock, flags);
    ret = HAL_MMU_AddClientId(u16ClientId);
    spin_unlock_irqrestore(&mmu_lock, flags);

    return ret;
}

int MDrv_MMU_RemoveClientId(unsigned short u16ClientId)
{
    unsigned long flags;
    int ret;

#ifndef CONFIG_FPGA
    if(Chip_Get_Revision() == 0x2)
    {
        printk("[%s] not support , chipId = %d \n", __FUNCTION__, Chip_Get_Revision() );
        return -1;
    }
#endif

    spin_lock_irqsave(&mmu_lock, flags);
    ret = HAL_MMU_RemoveClientId(u16ClientId);
    spin_unlock_irqrestore(&mmu_lock, flags);

    return ret;
}

#ifdef CONFIG_MMU_INTERRUPT_ENABLE
static irqreturn_t MDrv_MMU_interrupt(s32 irq, void *dev_id)
{
    unsigned long flags;
    unsigned int status;
    unsigned short u16PhyAddrEntry, u16ClientId;
    unsigned char u8IsWriteCmd;
    char name[32]={0};

    spin_lock_irqsave(&mmu_lock, flags);
    status = HAL_MMU_Status(&u16PhyAddrEntry, &u16ClientId, &u8IsWriteCmd);

    if (MDrv_MMU_Notify != NULL)
    {
        MDrv_MMU_Notify(status, u16PhyAddrEntry, u16ClientId, u8IsWriteCmd);
    }
    else
    {
        HAL_MIU_ClientIdToName(u16ClientId , name );
        printk("[%s] Status=0x%x, PhyAddrEntry=0x%x, ClientId=0x%x, name =%s , IsWrite=%d\n", __FUNCTION__,
                                                                                   status,
                                                                                   u16PhyAddrEntry,
                                                                                   u16ClientId,
                                                                                   name,
                                                                                   u8IsWriteCmd);
    }

    spin_unlock_irqrestore(&mmu_lock, flags);

    return IRQ_HANDLED;
}

void MDrv_MMU_CallbackFunc(MDrv_MMU_Callback pFuncPtr)
{
    unsigned long flags;

    spin_lock_irqsave(&mmu_lock, flags);
    MDrv_MMU_Notify = pFuncPtr;
    spin_unlock_irqrestore(&mmu_lock, flags);
}
EXPORT_SYMBOL(MDrv_MMU_CallbackFunc);
#endif

int MDrv_MMU_Enable(unsigned char u8Enable)
{
    unsigned long flags;
    int ret;

#ifdef CONFIG_MMU_INTERRUPT_ENABLE
    if (!mmu_isr_init)
    {
        struct device_node  *dev_node = NULL;
        int rc = 0;
        int iIrqNum = 0;
        dev_node = of_find_compatible_node(NULL, NULL, "sstar,mmu");
        if (!dev_node)
        {
            printk("[MMU] find node Fail\r\n");
        }

        iIrqNum = irq_of_parse_and_map(dev_node, 0);

        if(0 != (rc = request_irq(iIrqNum, MDrv_MMU_interrupt, IRQF_TRIGGER_HIGH, "MMU", NULL)))
        {
            printk("[MMU] request_irq [%d] Fail, Err:%d\r\n", iIrqNum, rc);
        }
        mmu_isr_init = 1;
    }
#endif

    spin_lock_irqsave(&mmu_lock, flags);
    ret = HAL_MMU_Enable(u8Enable);
    spin_unlock_irqrestore(&mmu_lock, flags);

    return ret;
}

int MDrv_MMU_Reset(void)
{
    unsigned long flags;
    int ret;

    spin_lock_irqsave(&mmu_lock, flags);
    ret = HAL_MMU_Reset();
    spin_unlock_irqrestore(&mmu_lock, flags);

    return ret;
}

#ifndef CONFIG_MMU_INTERRUPT_ENABLE
unsigned int MDrv_MMU_Status(unsigned short *u16PhyAddrEntry, unsigned short *u16ClientId, unsigned char *u8IsWriteCmd)
{
    unsigned long flags;
    unsigned int status;

    spin_lock_irqsave(&mmu_lock, flags);
    status = HAL_MMU_Status(u16PhyAddrEntry, u16ClientId, u8IsWriteCmd);
    spin_unlock_irqrestore(&mmu_lock, flags);

    return status;
}
#endif

EXPORT_SYMBOL(MDrv_MMU_SetRegion);
EXPORT_SYMBOL(MDrv_MMU_SetPageSize);
EXPORT_SYMBOL(MDrv_MMU_Map);
EXPORT_SYMBOL(MDrv_MMU_MapQuery);
EXPORT_SYMBOL(MDrv_MMU_UnMap);
EXPORT_SYMBOL(MDrv_MMU_AddClientId);
EXPORT_SYMBOL(MDrv_MMU_RemoveClientId);
EXPORT_SYMBOL(MDrv_MMU_Enable);
EXPORT_SYMBOL(MDrv_MMU_Reset);
#ifndef CONFIG_MMU_INTERRUPT_ENABLE
EXPORT_SYMBOL(MDrv_MMU_Status);
#endif
#endif

static int mstar_miu_drv_probe(struct platform_device *pdev)
{
    return 0;
}

static int mstar_miu_drv_remove(struct platform_device *pdev)
{
    return 0;
}

static int mstar_miu_drv_suspend(struct platform_device *dev, pm_message_t state)
{
    return 0;
}

static int mstar_miu_drv_resume(struct platform_device *dev)
{
    return 0;
}

#if defined (CONFIG_ARM64)
static struct of_device_id mstarmiu_of_device_ids[] = {
     {.compatible = "sstar-miu"},
     {},
};
#endif

static struct platform_driver Sstar_miu_driver = {
    .probe      = mstar_miu_drv_probe,
    .remove     = mstar_miu_drv_remove,
    .suspend    = mstar_miu_drv_suspend,
    .resume     = mstar_miu_drv_resume,
    .driver = {
    .name   = "Sstar-miu",
#if defined(CONFIG_ARM64)
    .of_match_table = mstarmiu_of_device_ids,
#endif
    .owner  = THIS_MODULE,
    }
};

static int __init mstar_miu_drv_init_module(void)
{
    int ret = 0;

    ret = platform_driver_register(&Sstar_miu_driver);

    if (ret) {
        printk("Register MIU Driver Fail");
    }
    return ret;
}

static void __exit mstar_miu_drv_exit_module(void)
{
    platform_driver_unregister(&Sstar_miu_driver);
}

module_init(mstar_miu_drv_init_module);
module_exit(mstar_miu_drv_exit_module);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("MIU driver");
MODULE_LICENSE("GPL");
