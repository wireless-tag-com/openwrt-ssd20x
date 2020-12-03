/*
* mdrv_hook.c- Sigmastar
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
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (“MStar Confidential Information”) by the recipien
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   hook.c
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------



#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/ipv6.h>

#include "mdrv_hwnat.h"

//--------------------------------------------------------------------------------------------------
//  Global Variable
//--------------------------------------------------------------------------------------------------


struct net_device   *dst_port[MAX_IF_NUM];
EXPORT_SYMBOL(dst_port);

struct foe_entry *ppe_virt_foe_base_tmp;
EXPORT_SYMBOL(ppe_virt_foe_base_tmp);

int (*noe_nat_hook_rx)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(noe_nat_hook_rx);

int (*noe_nat_hook_tx)(struct sk_buff *skb, int gmac_no) = NULL;
EXPORT_SYMBOL(noe_nat_hook_tx);

void (*ppe_dev_register_hook)(struct net_device *dev);
EXPORT_SYMBOL(ppe_dev_register_hook);

void (*ppe_dev_unregister_hook)(struct net_device *dev);
EXPORT_SYMBOL(ppe_dev_unregister_hook);

//--------------------------------------------------------------------------------------------------
//  Global Function
//--------------------------------------------------------------------------------------------------



void  MDrv_NOE_NAT_Set_Magic_Tag_Zero(struct sk_buff *skb)
{
    if ((FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_PCI) ||
        (FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_WLAN) ||
        (FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_GE)) {
        if (IS_SPACE_AVAILABLE_HEAD(skb))
            FOE_MAGIC_TAG_HEAD(skb) = 0;
    }
    if ((FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_PCI) ||
        (FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_WLAN) ||
        (FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_GE)) {
        if (IS_SPACE_AVAILABLE_TAIL(skb))
            FOE_MAGIC_TAG_TAIL(skb) = 0;
    }
}

void MDrv_NOE_NAT_Check_Magic_Tag(struct sk_buff *skb)
{
    if (IS_SPACE_AVAILABLE_HEAD(skb)) {
        FOE_MAGIC_TAG_HEAD(skb) = 0;
        FOE_AI_HEAD(skb) = UN_HIT;
    }
    if (IS_SPACE_AVAILABLE_TAIL(skb)) {
        FOE_MAGIC_TAG_TAIL(skb) = 0;
        FOE_AI_TAIL(skb) = UN_HIT;
    }
}

void MDrv_NOE_NAT_Set_Headroom_Zero(struct sk_buff *skb)
{
    if (skb->cloned != 1) {
        if (IS_MAGIC_TAG_PROTECT_VALID_HEAD(skb) ||
            (FOE_MAGIC_TAG(skb) == FOE_MAGIC_PPE)) {
            if (IS_SPACE_AVAILABLE_HEAD(skb))
                memset(FOE_INFO_START_ADDR_HEAD(skb), 0, FOE_INFO_LEN);
        }
    }
}

void MDrv_NOE_NAT_Set_Tailroom_Zero(struct sk_buff *skb)
{
    if (skb->cloned != 1) {
        if (IS_MAGIC_TAG_PROTECT_VALID_TAIL(skb) ||
            (FOE_MAGIC_TAG(skb) == FOE_MAGIC_PPE)) {
            if (IS_SPACE_AVAILABLE_TAIL(skb))
                memset(FOE_INFO_START_ADDR_TAIL(skb), 0, FOE_INFO_LEN);
        }
    }
}

void MDrv_NOE_NAT_Copy_Headroom(u8 *data, struct sk_buff *skb)
{
    memcpy(data, skb->head, FOE_INFO_LEN);
}

void MDrv_NOE_NAT_Copy_Tailroom(u8 *data, int size, struct sk_buff *skb)
{
    memcpy((data + size - FOE_INFO_LEN), (skb_end_pointer(skb) - FOE_INFO_LEN), FOE_INFO_LEN);
}

void MDrv_NOE_NAT_Set_Dma_Ops(struct device *dev, bool coherent)
{
#if defined(CONFIG_ARM64)
    #if LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0)
    if(coherent)
        set_dma_ops(dev, &coherent_swiotlb_dma_ops);
    else
        set_dma_ops(dev, &noncoherent_swiotlb_dma_ops);
    #else
    arch_setup_dma_ops(dev, 0, 0, NULL, coherent);
    #endif
#endif
}

EXPORT_SYMBOL(MDrv_NOE_NAT_Set_Dma_Ops);

