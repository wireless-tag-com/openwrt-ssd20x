/*
* mdrv_hwnat_log.c- Sigmastar
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
/// @file   MDRV_NOE_NAT.h
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------

#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/if_vlan.h>
#include <net/ipv6.h>
#include <net/ip.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <linux/inetdevice.h>
#include <net/rtnetlink.h>
#include <net/netevent.h>

#include "mhal_hwnat.h"
#include "mdrv_hwnat.h"
#include "mdrv_hwnat_foe.h"
#include "mdrv_hwnat_util.h"
#include "mdrv_hwnat_ioctl.h"
#include "mdrv_hwnat_log.h"

#include "mdrv_hwnat_mcast.h"

#if (PPTP_L2TP)
#include "fast_path.h"
#endif

//--------------------------------------------------------------------------------------------------
//  Global Variable and Functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  Local Variable
//--------------------------------------------------------------------------------------------------

static struct HWNAT_DEVICE *_p_hwnat_info = NULL;
//--------------------------------------------------------------------------------------------------
//  Local Functions
//--------------------------------------------------------------------------------------------------



void FOE_INFO_DUMP(struct sk_buff *skb)
{
    //HWNAT_MSG_DBG("FOE_INFO_START_ADDR(skb) =%p\n", FOE_INFO_START_ADDR(skb));
    HWNAT_MSG_DBG("FOE_TAG_PROTECT(skb) =%x\n", FOE_TAG_PROTECT(skb));
    HWNAT_MSG_DBG("FOE_ENTRY_NUM(skb) =%x\n", FOE_ENTRY_NUM(skb));
    HWNAT_MSG_DBG("FOE_ALG(skb) =%x\n", FOE_ALG(skb));
    HWNAT_MSG_DBG("FOE_AI(skb) =%x\n", FOE_AI(skb));
    HWNAT_MSG_DBG("FOE_SP(skb) =%x\n", FOE_SP(skb));
    HWNAT_MSG_DBG("FOE_MAGIC_TAG(skb) =%x\n", FOE_MAGIC_TAG(skb));

#if defined(CONFIG_NOE_HW_NAT_PPTP_L2TP)
    HWNAT_MSG_DBG("FOE_SOURCE(skb) =%x\n", FOE_SOURCE(skb));
    HWNAT_MSG_DBG("FOE_SOURCE(skb) =%x\n", FOE_SOURCE(skb));
#endif
}

void FOE_INFO_DUMP_TAIL(struct sk_buff *skb)
{
#if 0
    HWNAT_MSG_DBG("FOE_INFO_START_ADDR_TAIL(skb) =%p\n", FOE_INFO_START_ADDR_TAIL(skb));
    HWNAT_MSG_DBG("FOE_TAG_PROTECT_TAIL(skb) =%x\n", FOE_TAG_PROTECT_TAIL(skb));
    HWNAT_MSG_DBG("FOE_ENTRY_NUM_TAIL(skb) =%x\n", FOE_ENTRY_NUM_TAIL(skb));
    HWNAT_MSG_DBG("FOE_ALG_TAIL(skb) =%x\n", FOE_ALG_TAIL(skb));
    HWNAT_MSG_DBG("FOE_AI_TAIL(skb) =%x\n", FOE_AI_TAIL(skb));
    HWNAT_MSG_DBG("FOE_SP_TAIL(skb) =%x\n", FOE_SP_TAIL(skb));
    HWNAT_MSG_DBG("FOE_MAGIC_TAG_TAIL(skb) =%x\n", FOE_MAGIC_TAG_TAIL(skb));
#if defined(CONFIG_NOE_HW_NAT_PPTP_L2TP)
    HWNAT_MSG_DBG("FOE_SOURCE_TAIL(skb) =%x\n", FOE_SOURCE_TAIL(skb));
    HWNAT_MSG_DBG("FOE_SOURCE_TAIL(skb) =%x\n", FOE_SOURCE_TAIL(skb));
#endif
#endif
}


static uint8_t *show_cpu_reason(struct sk_buff *skb)
{
    static u8 buf[32];

    switch (FOE_AI(skb)) {
    case TTL_0:
        return "IPv4(IPv6) TTL(hop limit)\n";
    case HAS_OPTION_HEADER:
        return "Ipv4(IPv6) has option(extension) header\n";
    case NO_FLOW_IS_ASSIGNED:
        return "No flow is assigned\n";
    case IPV4_WITH_FRAGMENT:
        return "IPv4 HNAT doesn't support IPv4 /w fragment\n";
    case IPV4_HNAPT_DSLITE_WITH_FRAGMENT:
        return "IPv4 HNAPT/DS-Lite doesn't support IPv4 /w fragment\n";
    case IPV4_HNAPT_DSLITE_WITHOUT_TCP_UDP:
        return "IPv4 HNAPT/DS-Lite can't find TCP/UDP sport/dport\n";
    case IPV6_5T_6RD_WITHOUT_TCP_UDP:
        return "IPv6 5T-route/6RD can't find TCP/UDP sport/dport\n";
    case TCP_FIN_SYN_RST:
        return "Ingress packet is TCP fin/syn/rst\n";
    case UN_HIT:
        return "FOE Un-hit\n";
    case HIT_UNBIND:
        return "FOE Hit unbind\n";
    case HIT_UNBIND_RATE_REACH:
        return "FOE Hit unbind & rate reach\n";
    case HIT_BIND_TCP_FIN:
        return "Hit bind PPE TCP FIN entry\n";
    case HIT_BIND_TTL_1:
        return "Hit bind PPE entry and TTL(hop limit) = 1 and TTL(hot limit) - 1\n";
    case HIT_BIND_WITH_VLAN_VIOLATION:
        return "Hit bind and VLAN replacement violation\n";
    case HIT_BIND_KEEPALIVE_UC_OLD_HDR:
        return "Hit bind and keep alive with unicast old-header packet\n";
    case HIT_BIND_KEEPALIVE_MC_NEW_HDR:
        return "Hit bind and keep alive with multicast new-header packet\n";
    case HIT_BIND_KEEPALIVE_DUP_OLD_HDR:
        return "Hit bind and keep alive with duplicate old-header packet\n";
    case HIT_BIND_FORCE_TO_CPU:
        return "FOE Hit bind & force to CPU\n";
    case HIT_BIND_EXCEED_MTU:
        return "Hit bind and exceed MTU\n";
    case HIT_BIND_MULTICAST_TO_CPU:
        return "Hit bind multicast packet to CPU\n";
    case HIT_BIND_MULTICAST_TO_GMAC_CPU:
        return "Hit bind multicast packet to GMAC & CPU\n";
    case HIT_PRE_BIND:
        return "Pre bind\n";
    }

    sprintf(buf, "CPU Reason Error - %X\n", FOE_AI(skb));
    return buf;
}

uint32_t MDrv_HWNAT_Dump_TxSkb(struct sk_buff *skb)
{
    int i;

    HWNAT_MSG_DBG("\nTx===<FOE_Entry=%d>=====\n", FOE_ENTRY_NUM(skb));
    HWNAT_MSG_DBG("Tx handler skb_headroom size = %u, skb->head = %p, skb->data = %p\n",
         skb_headroom(skb), skb->head, skb->data);
    for (i = 0; i < skb_headroom(skb); i++) {
        HWNAT_MSG_DBG("tx_skb->head[%d]=%x\n", i, *(unsigned char *)(skb->head + i));
        /* HWNAT_MSG_DBG("%02X-",*((unsigned char*)i)); */
    }

    HWNAT_MSG_DBG("==================================\n");
    return 1;
}

uint32_t MDrv_HWNAT_Dump_Skb(struct sk_buff *skb)
{
    struct foe_entry *entry;

    if (_p_hwnat_info == NULL)
        return 0;

    entry =  (((struct foe_entry *)(_p_hwnat_info->ppe_foe_base)) + FOE_ENTRY_NUM(skb));

    HWNAT_MSG_DBG("\nRx===<FOE_Entry=%d>=====\n", FOE_ENTRY_NUM(skb));
    HWNAT_MSG_DBG("RcvIF=%s\n", skb->dev->name);
    HWNAT_MSG_DBG("FOE_Entry=%d\n", FOE_ENTRY_NUM(skb));
    HWNAT_MSG_DBG("CPU Reason=%s", show_cpu_reason(skb));
    HWNAT_MSG_DBG("ALG=%d\n", FOE_ALG(skb));
    HWNAT_MSG_DBG("SP=%d\n", FOE_SP(skb));

    /* some special alert occurred, so entry_num is useless (just skip it) */
    if (FOE_ENTRY_NUM(skb) == 0x3fff)
        return 1;

    /* PPE: IPv4 packet=IPV4_HNAT IPv6 packet=IPV6_ROUTE */
    if (IS_IPV4_GRP(entry)) {
        HWNAT_MSG_DBG("Information Block 1=%x\n", entry->ipv4_hnapt.info_blk1);
        HWNAT_MSG_DBG("SIP=%s\n", MDrv_HWNAT_Util_Ip_To_Str(entry->ipv4_hnapt.sip));
        HWNAT_MSG_DBG("DIP=%s\n", MDrv_HWNAT_Util_Ip_To_Str(entry->ipv4_hnapt.dip));
        HWNAT_MSG_DBG("SPORT=%d\n", entry->ipv4_hnapt.sport);
        HWNAT_MSG_DBG("DPORT=%d\n", entry->ipv4_hnapt.dport);
        HWNAT_MSG_DBG("Information Block 2=%x\n", entry->ipv4_hnapt.info_blk2);
    }
    else if ((_hwnat_info.features & NAT_IPV6) && (IS_IPV6_GRP(entry))) {
        HWNAT_MSG_DBG("Information Block 1=%x\n", entry->ipv6_5t_route.info_blk1);
        HWNAT_MSG_DBG("IPv6_SIP=%08X:%08X:%08X:%08X\n",
              entry->ipv6_5t_route.ipv6_sip0,
              entry->ipv6_5t_route.ipv6_sip1,
              entry->ipv6_5t_route.ipv6_sip2, entry->ipv6_5t_route.ipv6_sip3);
        HWNAT_MSG_DBG("IPv6_DIP=%08X:%08X:%08X:%08X\n",
              entry->ipv6_5t_route.ipv6_dip0,
              entry->ipv6_5t_route.ipv6_dip1,
              entry->ipv6_5t_route.ipv6_dip2, entry->ipv6_5t_route.ipv6_dip3);
        if (IS_IPV6_FLAB_EBL()) {
            HWNAT_MSG_DBG("Flow Label=%08X\n", (entry->ipv6_5t_route.sport << 16) |
                  (entry->ipv6_5t_route.dport));
        } else {
            HWNAT_MSG_DBG("SPORT=%d\n", entry->ipv6_5t_route.sport);
            HWNAT_MSG_DBG("DPORT=%d\n", entry->ipv6_5t_route.dport);
        }
        HWNAT_MSG_DBG("Information Block 2=%x\n", entry->ipv6_5t_route.info_blk2);
    }
    else
        HWNAT_MSG_DBG("unknown Pkt_type=%d\n", entry->bfib1.pkt_type);
    HWNAT_MSG_DBG("==================================\n");
    return 1;
}

void MDrv_HWNAT_LOG_Set_Level(unsigned char level)
{

    if (_p_hwnat_info == NULL)
        return;


    _p_hwnat_info->log_level = level;

    if (level & E_MDRV_HWNAT_MSG_CTRL_DUMP) {
        MHal_HWNAT_Set_DBG(E_NOE_SEL_ENABLE);
    }
    else {
        MHal_HWNAT_Set_DBG(E_NOE_SEL_DISABLE);
    }
}

unsigned char MDrv_HWNAT_LOG_Get_Level(void)
{

    if (_p_hwnat_info == NULL)
        return E_MDRV_HWNAT_MSG_CTRL_NONE;


    return _p_hwnat_info->log_level;
}

void MDrv_HWNAT_LOG_Init(void *info)
{
    _p_hwnat_info = (struct HWNAT_DEVICE *)info;
}


