/*
* mdrv_hwnat.h- Sigmastar
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
/// @brief  NOE NAT Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _MDRV_NOE_HWNAT_H_
#define _MDRV_NOE_HWNAT_H_
//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include "mdrv_hwnat_config.h"

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------


#define hwnat_vlan_tx_tag_present(__skb)     ((__skb)->vlan_tci & VLAN_TAG_PRESENT)
#define hwnat_vlan_tag_get(__skb)         ((__skb)->vlan_tci & ~VLAN_TAG_PRESENT)

enum foe_cpu_reason {
    TTL_0 = 0x02,                               /* IPv4(IPv6) TTL(hop limit) = 0 */
    HAS_OPTION_HEADER = 0x03,                   /* IPv4(IPv6) has option(extension) header */
    NO_FLOW_IS_ASSIGNED = 0x07,                 /* No flow is assigned */
    IPV4_WITH_FRAGMENT = 0x08,                  /* IPv4 HNAT doesn't support IPv4 /w fragment */
    IPV4_HNAPT_DSLITE_WITH_FRAGMENT = 0x09,     /* IPv4 HNAPT/DS-Lite doesn't support IPv4 /w fragment */
    IPV4_HNAPT_DSLITE_WITHOUT_TCP_UDP = 0x0A,   /* IPv4 HNAPT/DS-Lite can't find TCP/UDP sport/dport */
    IPV6_5T_6RD_WITHOUT_TCP_UDP = 0x0B,         /* IPv6 5T-route/6RD can't find TCP/UDP sport/dport */
    TCP_FIN_SYN_RST = 0x0C,                     /* Ingress packet is TCP fin/syn/rst (for IPv4 NAPT/DS-Lite or IPv6 5T-route/6RD) */
    UN_HIT = 0x0D,                              /* FOE Un-hit */
    HIT_UNBIND = 0x0E,                          /* FOE Hit unbind */
    HIT_UNBIND_RATE_REACH = 0x0F,               /* FOE Hit unbind & rate reach */
    HIT_BIND_TCP_FIN = 0x10,                    /* Hit bind PPE TCP FIN entry */
    HIT_BIND_TTL_1 = 0x11,                      /* Hit bind PPE entry and TTL(hop limit) = 1 and TTL(hot limit) - 1 */
    HIT_BIND_WITH_VLAN_VIOLATION = 0x12,        /* Hit bind and VLAN replacement violation (Ingress 1(0) VLAN layers and egress 4(3 or 4) VLAN layers) */
    HIT_BIND_KEEPALIVE_UC_OLD_HDR = 0x13,       /* Hit bind and keep alive with unicast old-header packet */
    HIT_BIND_KEEPALIVE_MC_NEW_HDR = 0x14,       /* Hit bind and keep alive with multicast new-header packet */
    HIT_BIND_KEEPALIVE_DUP_OLD_HDR = 0x15,      /* Hit bind and keep alive with duplicate old-header packet */
    HIT_BIND_FORCE_TO_CPU = 0x16,               /* FOE Hit bind & force to CPU */
    HIT_BIND_WITH_OPTION_HEADER = 0x17,         /* Hit bind and remove tunnel IP header, but inner IP has option/next header */
    HIT_BIND_EXCEED_MTU = 0x1C,                 /* Hit bind and exceed MTU */
    HIT_BIND_PACKET_SAMPLING = 0x1B,            /*  PS packet */
    HIT_BIND_MULTICAST_TO_CPU = 0x18,           /*  Switch clone multicast packet to CPU */
    HIT_BIND_MULTICAST_TO_GMAC_CPU = 0x19,      /*  Switch clone multicast packet to GMAC1 & CPU */
    HIT_PRE_BIND = 0x1A                         /*  Pre-bind */
};



enum dst_port_num {
    DP_RA0 = 11,
    DP_RA1 = 12,
    DP_RA2 = 13,
    DP_RA3 = 14,
    DP_RA4 = 15,
    DP_RA5 = 16,
    DP_RA6 = 17,
    DP_RA7 = 18,
    DP_RA8 = 19,
    DP_RA9 = 20,
    DP_RA10 = 21,
    DP_RA11 = 22,
    DP_RA12 = 23,
    DP_RA13 = 24,
    DP_RA14 = 25,
    DP_RA15 = 26,
    DP_WDS0 = 27,
    DP_WDS1 = 28,
    DP_WDS2 = 29,
    DP_WDS3 = 30,
    DP_APCLI0 = 31,
    DP_MESH0 = 32,
    DP_RAI0 = 33,
    DP_RAI1 = 34,
    DP_RAI2 = 35,
    DP_RAI3 = 36,
    DP_RAI4 = 37,
    DP_RAI5 = 38,
    DP_RAI6 = 39,
    DP_RAI7 = 40,
    DP_RAI8 = 41,
    DP_RAI9 = 42,
    DP_RAI10 = 43,
    DP_RAI11 = 44,
    DP_RAI12 = 45,
    DP_RAI13 = 46,
    DP_RAI14 = 47,
    DP_RAI15 = 48,
    DP_WDSI0 = 49,
    DP_WDSI1 = 50,
    DP_WDSI2 = 51,
    DP_WDSI3 = 52,
    DP_APCLII0 = 53,
    DP_MESHI0 = 54,
    MAX_WIFI_IF_NUM = 59,
    DP_GMAC = 60,
    DP_GMAC2 = 61,
    DP_PCI = 62,
    DP_USB = 63,
    MAX_IF_NUM
};

struct pdma_rx_desc_info4 {
    u16 MAGIC_TAG_PROTECT;
    uint32_t foe_entry_num:14;
    uint32_t CRSN:5;
    uint32_t SPORT:4;
    uint32_t ALG:1;
    uint16_t IF:8;
    u8 WDMAID;
    uint16_t RXID:2;
    uint16_t WCID:8;
    uint16_t BSSID:6;
    u16 SOURCE;
    u16 DEST;
} __packed;

struct head_rx_descinfo4 {
    uint32_t foe_entry_num:14;
    uint32_t CRSN:5;
    uint32_t SPORT:4;
    uint32_t ALG:1;
    uint32_t IF:8;
    u16 MAGIC_TAG_PROTECT;
    u8 WDMAID;
    uint16_t RXID:2;
    uint16_t WCID:8;
    uint16_t BSSID:6;
    u16 SOURCE;
    u16 DEST;
} __packed;

struct cb_rx_desc_info4 {
    u16 MAGIC_TAG_PROTECT0;
    uint32_t foe_entry_num:14;
    uint32_t CRSN:5;
    uint32_t SPORT:4;
    uint32_t ALG:1;
    uint32_t IF:8;
    u16 MAGIC_TAG_PROTECT1;
    u8 WDMAID;
    uint16_t RXID:2;
    uint16_t WCID:8;
    uint16_t BSSID:6;
    u16 SOURCE;
    u16 DEST;
} __packed;

#ifndef NEXTHDR_IPIP
#define NEXTHDR_IPIP                    (4)
#endif /* NEXTHDR_IPIP */

#define FOE_MAGIC_PCI                   (0x73)
#define FOE_MAGIC_WLAN                  (0x74)
#define FOE_MAGIC_GE                    (0x75)
#define FOE_MAGIC_PPE                   (0x76)
#define TAG_PROTECT                     (0x6789)
#define USE_HEAD_ROOM                   (0)
#define USE_TAIL_ROOM                   (1)
#define USE_CB                          (2)
#define ALL_INFO_ERROR                  (3)


#define FOE_TAG_PROTECT(skb)            (((struct head_rx_descinfo4 *)((skb)->head))->MAGIC_TAG_PROTECT)
#define FOE_ENTRY_NUM(skb)              (((struct head_rx_descinfo4 *)((skb)->head))->foe_entry_num)
#define FOE_ALG(skb)                    (((struct head_rx_descinfo4 *)((skb)->head))->ALG)
#define FOE_AI(skb)                     (((struct head_rx_descinfo4 *)((skb)->head))->CRSN)
#define FOE_SP(skb)                     (((struct head_rx_descinfo4 *)((skb)->head))->SPORT)
#define FOE_MAGIC_TAG(skb)              (((struct head_rx_descinfo4 *)((skb)->head))->IF)
#define FOE_WDMA_ID(skb)                (((struct head_rx_descinfo4 *)((skb)->head))->WDMAID)
#define FOE_RX_ID(skb)                  (((struct head_rx_descinfo4 *)((skb)->head))->RXID)
#define FOE_WC_ID(skb)                  (((struct head_rx_descinfo4 *)((skb)->head))->WCID)
#define FOE_BSS_ID(skb)                 (((struct head_rx_descinfo4 *)((skb)->head))->BSSID)
#define FOE_SOURCE(skb)                 (((struct head_rx_descinfo4 *)((skb)->head))->SOURCE)
#define FOE_DEST(skb)                   (((struct head_rx_descinfo4 *)((skb)->head))->DEST)

#define IS_SPACE_AVAILABLED_HEAD(skb)   ((((skb_headroom(skb) >= FOE_INFO_LEN) ? 1 : 0)))
#define IS_SPACE_AVAILABLE_HEAD(skb)    ((((skb_headroom(skb) >= FOE_INFO_LEN) ? 1 : 0)))
#define FOE_INFO_START_ADDR_HEAD(skb)   (skb->head)

#define FOE_TAG_PROTECT_HEAD(skb)       (((struct head_rx_descinfo4 *)((skb)->head))->MAGIC_TAG_PROTECT)
#define FOE_ENTRY_NUM_HEAD(skb)         (((struct head_rx_descinfo4 *)((skb)->head))->foe_entry_num)
#define FOE_ALG_HEAD(skb)               (((struct head_rx_descinfo4 *)((skb)->head))->ALG)
#define FOE_AI_HEAD(skb)                (((struct head_rx_descinfo4 *)((skb)->head))->CRSN)
#define FOE_SP_HEAD(skb)                (((struct head_rx_descinfo4 *)((skb)->head))->SPORT)
#define FOE_MAGIC_TAG_HEAD(skb)         (((struct head_rx_descinfo4 *)((skb)->head))->IF)

#define FOE_WDMA_ID_HEAD(skb)           (((struct head_rx_descinfo4 *)((skb)->head))->WDMAID)
#define FOE_RX_ID_HEAD(skb)             (((struct head_rx_descinfo4 *)((skb)->head))->RXID)
#define FOE_WC_ID_HEAD(skb)             (((struct head_rx_descinfo4 *)((skb)->head))->WCID)
#define FOE_BSS_ID_HEAD(skb)            (((struct head_rx_descinfo4 *)((skb)->head))->BSSID)

#define FOE_SOURCE_HEAD(skb)            (((struct head_rx_descinfo4 *)((skb)->head))->SOURCE)
#define FOE_DEST_HEAD(skb)              (((struct head_rx_descinfo4 *)((skb)->head))->DEST)
#define IS_SPACE_AVAILABLED_TAIL(skb)   (((skb_tailroom(skb) >= FOE_INFO_LEN) ? 1 : 0))
#define IS_SPACE_AVAILABLE_TAIL(skb)    (((skb_tailroom(skb) >= FOE_INFO_LEN) ? 1 : 0))
#define FOE_INFO_START_ADDR_TAIL(skb)   ((unsigned char *)(long)(skb_end_pointer(skb) - FOE_INFO_LEN))

#define FOE_TAG_PROTECT_TAIL(skb)       (((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->MAGIC_TAG_PROTECT)
#define FOE_ENTRY_NUM_TAIL(skb)         (((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->foe_entry_num)
#define FOE_ALG_TAIL(skb)               (((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->ALG)
#define FOE_AI_TAIL(skb)                (((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->CRSN)
#define FOE_SP_TAIL(skb)                (((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->SPORT)
#define FOE_MAGIC_TAG_TAIL(skb)         (((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->IF)

#define FOE_SOURCE_TAIL(skb)            (((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->SOURCE)
#define FOE_DEST_TAIL(skb)              (((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->DEST)

#define FOE_WDMA_ID_TAIL(skb)           (((struct pdma_rx_desc_info4 *)((skb)->head))->WDMAID)
#define FOE_RX_ID_TAIL(skb)             (((struct pdma_rx_desc_info4 *)((skb)->head))->RXID)
#define FOE_WC_ID_TAIL(skb)             (((struct pdma_rx_desc_info4 *)((skb)->head))->WCID)
#define FOE_BSS_ID_TAIL(skb)            (((struct pdma_rx_desc_info4 *)((skb)->head))->BSSID)

/* change the position of skb_CB if necessary */
#define CB_OFFSET                       (40)
#define IS_SPACE_AVAILABLE_CB(skb)      (1)
#define FOE_INFO_START_ADDR_CB(skb)     (skb->cb +  CB_OFFSET)
#define FOE_TAG_PROTECT_CB0(skb)        (((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->MAGIC_TAG_PROTECT0)
#define FOE_TAG_PROTECT_CB1(skb)        (((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->MAGIC_TAG_PROTECT1)
#define FOE_ENTRY_NUM_CB(skb)           (((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->foe_entry_num)
#define FOE_ALG_CB(skb)                 (((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->ALG)
#define FOE_AI_CB(skb)                  (((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->CRSN)
#define FOE_SP_CB(skb)                  (((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->SPORT)
#define FOE_MAGIC_TAG_CB(skb)           (((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->IF)

#if defined(CONFIG_NOE_HW_NAT_PPTP_L2TP)
#define FOE_SOURCE_CB(skb)  (((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->SOURCE)
#define FOE_DEST_CB(skb)    (((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->DEST)
#endif


#define FOE_WDMA_ID_CB(skb)             (((struct cb_rx_desc_info4 *)((skb)->head))->WDMAID)
#define FOE_RX_ID_CB(skb)               (((struct cb_rx_desc_info4 *)((skb)->head))->RXID)
#define FOE_WC_ID_CB(skb)               (((struct cb_rx_desc_info4 *)((skb)->head))->WCID)
#define FOE_BSS_ID_CB(skb)              (((struct cb_rx_desc_info4 *)((skb)->head))->BSSID)

#define IS_MAGIC_TAG_PROTECT_VALID_HEAD(skb)    (FOE_TAG_PROTECT_HEAD(skb) == TAG_PROTECT)
#define IS_MAGIC_TAG_PROTECT_VALID_TAIL(skb)    (FOE_TAG_PROTECT_TAIL(skb) == TAG_PROTECT)
#define IS_MAGIC_TAG_PROTECT_VALID_CB(skb)      ((FOE_TAG_PROTECT_CB0(skb) == TAG_PROTECT) && (FOE_TAG_PROTECT_CB0(skb) == FOE_TAG_PROTECT_CB1(skb)))

#define IS_IF_PCIE_WLAN_HEAD(skb)               ((FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_PCI) || (FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_WLAN) || (FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_GE))
#define IS_IF_PCIE_WLAN_TAIL(skb)               ((FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_PCI) || (FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_WLAN))
#define IS_IF_PCIE_WLAN_CB(skb)                 ((FOE_MAGIC_TAG_CB(skb) == FOE_MAGIC_PCI) || (FOE_MAGIC_TAG_CB(skb) == FOE_MAGIC_WLAN))



#define FOE_SOURCE(skb)         (((struct head_rx_descinfo4 *)((skb)->head))->SOURCE)
#define FOE_DEST(skb)           (((struct head_rx_descinfo4 *)((skb)->head))->DEST)
#define FOE_SOURCE_HEAD(skb)    (((struct head_rx_descinfo4 *)((skb)->head))->SOURCE)
#define FOE_DEST_HEAD(skb)      (((struct head_rx_descinfo4 *)((skb)->head))->DEST)
#define FOE_SOURCE_TAIL(skb)    (((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->SOURCE)
#define FOE_DEST_TAIL(skb)      (((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->DEST)



#define FOE_WDMA_ID(skb)        (((struct head_rx_descinfo4 *)((skb)->head))->WDMAID)
#define FOE_RX_ID(skb)          (((struct head_rx_descinfo4 *)((skb)->head))->RXID)
#define FOE_WC_ID(skb)          (((struct head_rx_descinfo4 *)((skb)->head))->WCID)
#define FOE_BSS_ID(skb)         (((struct head_rx_descinfo4 *)((skb)->head))->BSSID)

#define FOE_WDMA_ID_HEAD(skb)   (((struct head_rx_descinfo4 *)((skb)->head))->WDMAID)
#define FOE_RX_ID_HEAD(skb)     (((struct head_rx_descinfo4 *)((skb)->head))->RXID)
#define FOE_WC_ID_HEAD(skb)     (((struct head_rx_descinfo4 *)((skb)->head))->WCID)
#define FOE_BSS_ID_HEAD(skb)    (((struct head_rx_descinfo4 *)((skb)->head))->BSSID)

#define FOE_WDMA_ID_TAIL(skb)   (((struct pdma_rx_desc_info4 *)((skb)->head))->WDMAID)
#define FOE_RX_ID_TAIL(skb)     (((struct pdma_rx_desc_info4 *)((skb)->head))->RXID)
#define FOE_WC_ID_TAIL(skb)     (((struct pdma_rx_desc_info4 *)((skb)->head))->WCID)
#define FOE_BSS_ID_TAIL(skb)    (((struct pdma_rx_desc_info4 *)((skb)->head))->BSSID)



/* macros */
#define magic_tag_set_zero(skb) \
{ \
    if ((FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_PCI) || (FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_WLAN) || (FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_GE)) { \
        if (IS_SPACE_AVAILABLE_HEAD(skb)) \
            FOE_MAGIC_TAG_HEAD(skb) = 0; \
    } \
    if ((FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_PCI) || (FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_WLAN) || (FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_GE)) { \
        if (IS_SPACE_AVAILABLE_TAIL(skb)) \
            FOE_MAGIC_TAG_TAIL(skb) = 0; \
    } \
}



#define FOE_ENTRY_BASE_BY_PKT(base, skb)  (&((struct foe_entry *)base)[FOE_ENTRY_NUM(skb)])
#define FOE_ENTRY_BASE_BY_IDX(base, idx)  (&((struct foe_entry *)base)[idx])
#define PS_ENTRY_BASE_BY_PKT(base, skb)  (&((struct ps_entry *)base)[FOE_ENTRY_NUM(skb)])
#define PS_ENTRY_BASE_BY_IDX(base, idx)  (&((struct ps_entry *)base)[idx])
#define MIB_ENTRY_BASE_BY_PKT(base, skb)  (&((struct mib_entry *)base)[FOE_ENTRY_NUM(skb)])
#define MIB_ENTRY_BASE_BY_IDX(base, idx)  (&((struct mib_entry *)base)[idx])

//-------------------------------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------------------------------

#if defined(CONFIG_NOE_NAT_HW)
void MDrv_NOE_NAT_Set_Magic_Tag_Zero(struct sk_buff *skb);
void MDrv_NOE_NAT_Check_Magic_Tag(struct sk_buff *skb);
void MDrv_NOE_NAT_Set_Headroom_Zero(struct sk_buff *skb);
void MDrv_NOE_NAT_Set_Tailroom_Zero(struct sk_buff *skb);
void MDrv_NOE_NAT_Copy_Headroom(u8 *data, struct sk_buff *skb);
void MDrv_NOE_NAT_Copy_Tailroom(u8 *data, int size, struct sk_buff *skb);
#else
static inline void MDrv_NOE_NAT_Set_Magic_Tag_Zero(struct sk_buff *skb) {}
static inline void MDrv_NOE_NAT_Check_Magic_Tag(struct sk_buff *skb) {}
static inline void MDrv_NOE_NAT_Set_Headroom_Zero(struct sk_buff *skb) {}
static inline void MDrv_NOE_NAT_Set_Tailroom_Zero(struct sk_buff *skb) {}
static inline void MDrv_NOE_NAT_Copy_Headroom(u8 *data, struct sk_buff *skb) {}
static inline void MDrv_NOE_NAT_Copy_Tailroom(u8 *data, int size, struct sk_buff *skb) {}
#endif /* CONFIG_NOE_NAT_HW */



static inline void MDrv_NOE_NAT_Set_L2tp_Unhit(struct iphdr *iph, struct sk_buff *skb)
{
#if PPTP_L2TP
    /* only clear headeroom for TCP OR not L2TP packets */
    if ((iph->protocol == 0x6) || (ntohs(udp_hdr(skb)->dest) != 1701)) {
        if (IS_SPACE_AVAILABLED_HEAD(skb)) {
            FOE_MAGIC_TAG(skb) = 0;
            FOE_AI(skb) = UN_HIT;
        }
    }
#endif
}

static inline void MDrv_NOE_NAT_Set_L2tp_Fast_Path(u32 l2tp_fast_path, u32 pptp_fast_path)
{
#if PPTP_L2TP
    l2tp_fast_path = 1;
    pptp_fast_path = 0;
#endif
}

static inline void MDrv_NOE_NAT_Clear_L2tp_Fast_Path(u32 l2tp_fast_path)
{
#if PPTP_L2TP
    l2tp_fast_path = 0;
#endif
}



struct HWNAT_DEVICE {
    struct platform_device *pdev;
    dma_addr_t ppe_phy_foe_base;
    void *ppe_foe_base; //struct foe_entry *
    dma_addr_t ppe_phy_ps_base;
    void *ppe_ps_base; //struct ps_entry *
    dma_addr_t ppe_phy_mib_base;
    void *ppe_mib_base; //struct mib_entry *
    uint64_t features;
    u8 test;
    u32 table_entries;
    int fast_bind;
    u8 hash_cnt;
    int DP_GMAC1;
    int DPORT_GMAC2;
    u8 hash_mode;
    u8 hash_debug;
    u32 foe_tbl_size;
    u32 ps_tbl_size;
    u32 mib_tbl_size;
    uint16_t  lan_vid;
    uint16_t  wan_vid;
    unsigned char bind_dir;
    int debug_level;
    u8 log_level;
};


extern struct HWNAT_DEVICE _hwnat_info;
extern struct net_device *dst_port[MAX_IF_NUM];
extern u32 pptp_fast_path;
extern u32 l2tp_fast_path;
extern int (*noe_nat_hook_rx)(struct sk_buff *skb);
extern int (*noe_nat_hook_tx)(struct sk_buff *skb, int gmac_no);
extern void (*ppe_dev_register_hook)(struct net_device *dev);
extern void (*ppe_dev_unregister_hook)(struct net_device *dev);

#endif /* _MDRV_NOE_HWNAT_H_ */

