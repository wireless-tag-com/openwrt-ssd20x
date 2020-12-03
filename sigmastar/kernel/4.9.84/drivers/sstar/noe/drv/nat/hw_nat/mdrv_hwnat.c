/*
* mdrv_hwnat.c- Sigmastar
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
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/platform_device.h>

#include "mhal_hwnat.h"
#include "mdrv_hwnat.h"
#include "mdrv_hwnat_log.h"
#include "mdrv_hwnat_foe.h"
#include "mdrv_hwnat_util.h"
#include "mdrv_hwnat_ioctl.h"
#include "mdrv_hwnat_define.h"
#include "mdrv_hwnat_mcast.h"
#include "mdrv_hwnat_fast_path.h"
#include "mdrv_noe_def.h"

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define CB_OFF  10
#define HWNAT_GET_PACKET_IF(skb)                 skb->cb[CB_OFF + 6]
#define MIN_NET_DEVICE_FOR_MBSSID               0x00
#define MIN_NET_DEVICE_FOR_WDS                  0x10
#define MIN_NET_DEVICE_FOR_APCLI                0x20
#define MIN_NET_DEVICE_FOR_MESH                 0x30



//--------------------------------------------------------------------------------------------------
//  Global Variable and Functions
//--------------------------------------------------------------------------------------------------
struct foe_entry *ppe_foe_base;
EXPORT_SYMBOL(ppe_foe_base);
extern unsigned int web_sfq_enable;


//--------------------------------------------------------------------------------------------------
//  Local Functions
//--------------------------------------------------------------------------------------------------

static void _MDrv_HWNAT_Clear_Bind_Entry(struct neighbour *neigh);
static void _MDrv_HWNAT_Handle_Ac_Update(unsigned long unused);

void _MDrv_HWNAT_Set_Entry_Bind(struct sk_buff *skb, struct foe_entry *entry);
int32_t MDrv_HWNAT_Set_Force_Port(struct sk_buff *skb, struct foe_entry *entry, int gmac_no);
static int32_t _MDrv_HWNAT_Fill_L2(struct sk_buff *skb, struct foe_entry *entry);
static int32_t _MDrv_HWNAT_Fill_L3(struct sk_buff *skb, struct foe_entry *entry);
static int32_t _MDrv_HWNAT_Fill_L4(struct sk_buff *skb, struct foe_entry *entry);
void MDrv_NOE_NAT_Set_Dma_Ops(struct device *dev, bool coherent);
static int MDrv_HWNAT_Handle_NetEvent(struct notifier_block *unused, unsigned long event, void *ptr);
//--------------------------------------------------------------------------------------------------
//  Local Variable
//--------------------------------------------------------------------------------------------------
/*for 16 queue test*/
unsigned char queue_number;

static u64 hwnat_dmamask = ~(u32)0;
struct HWNAT_DEVICE _hwnat_info = {
    .pdev = NULL,
    .ppe_foe_base = NULL,
    .ppe_ps_base = NULL,
    .ppe_mib_base = NULL,
    .table_entries = FOE_4TB_SIZ,
    .hash_mode = DFL_FOE_HASH_MODE,
    .hash_debug = DEF_HASH_DBG_MODE,
    .lan_vid = CONFIG_NOE_HW_NAT_LAN_VLANID,
    .wan_vid = CONFIG_NOE_HW_NAT_WAN_VLANID,
    .bind_dir = BIDIRECTION,
    .debug_level = 0,
    .log_level = E_MDRV_HWNAT_MSG_CTRL_DBG,
};

static struct notifier_block Hnat_netevent_nb __read_mostly = {
    .notifier_call = MDrv_HWNAT_Handle_NetEvent,
};



DEFINE_TIMER(update_foe_ac_timer, _MDrv_HWNAT_Handle_Ac_Update, 0, 0);

int hwnat_info_region;
int getBrLan = 0;
uint32_t brNetmask;
uint32_t br0Ip;
char br0_mac_address[6];


struct timer_list hwnat_clear_entry_timer;

u8 USE_3T_UDP_FRAG;

struct pkt_parse_result ppe_parse_result;

struct hwnat_ac_args ac_info[64];   /* 1 for LAN, 2 for WAN */

struct net_device *dev;
int fast_bind;
u8 hash_cnt;
int DP_GMAC1;
int DPORT_GMAC2;

/*for 16 queue test*/
unsigned char queue_number;
u32 pptp_fast_path = 0;
u32 l2tp_fast_path = 0;


/* #define DSCP_REMARK_TEST */
/* #define PREBIND_TEST */

static int _MDrv_HWNAT_Get_Dev_Handler_Idx(struct net_device *dev)
{
    int i;
    if (dev == NULL)
        return -1;

    for (i = 0; i < MAX_IF_NUM; i++) {
        if (dst_port[i] == dev) {
            if (_hwnat_info.debug_level >= 1)
                NAT_PRINT("%s dst_port table has beed registered(%d)\n", dev->name, i);
            return i;
        }
    }
    return -1;
}

static void _MDrv_HWNAT_Clear_Entry(unsigned long data)
{
    HWNAT_MSG_DBG("HW_NAT work normally\n");
    MHal_HWNAT_Set_Miss_Action(E_HWNAT_FOE_SEARCH_MISS_FWD_CPU_BUILD_ENTRY);
    //del_timer_sync(&hwnat_clear_entry_timer);
}

uint16_t IS_IF_PCIE_WLAN(struct sk_buff *skb)
{
    if (IS_MAGIC_TAG_PROTECT_VALID_HEAD(skb))
        return IS_IF_PCIE_WLAN_HEAD(skb);
    else if (IS_MAGIC_TAG_PROTECT_VALID_TAIL(skb))
        return IS_IF_PCIE_WLAN_TAIL(skb);
    else if (IS_MAGIC_TAG_PROTECT_VALID_CB(skb))
        return IS_IF_PCIE_WLAN_CB(skb);
    else
        return 0;
}

uint16_t IS_IF_PCIE_WLAN_RX(struct sk_buff *skb)
{
    return IS_IF_PCIE_WLAN_HEAD(skb);
}

uint16_t IS_MAGIC_TAG_PROTECT_VALID(struct sk_buff *skb)
{
    if (IS_MAGIC_TAG_PROTECT_VALID_HEAD(skb))
        return IS_MAGIC_TAG_PROTECT_VALID_HEAD(skb);
    else if (IS_MAGIC_TAG_PROTECT_VALID_TAIL(skb))
        return IS_MAGIC_TAG_PROTECT_VALID_TAIL(skb);
    else if (IS_MAGIC_TAG_PROTECT_VALID_CB(skb))
        return IS_MAGIC_TAG_PROTECT_VALID_CB(skb);
    else
        return 0;
}

unsigned char *FOE_INFO_START_ADDR(struct sk_buff *skb)
{
    if (IS_MAGIC_TAG_PROTECT_VALID_HEAD(skb))
        return FOE_INFO_START_ADDR_HEAD(skb);
    else if (IS_MAGIC_TAG_PROTECT_VALID_TAIL(skb))
        return FOE_INFO_START_ADDR_TAIL(skb);
    else if (IS_MAGIC_TAG_PROTECT_VALID_CB(skb))
        return FOE_INFO_START_ADDR_CB(skb);

    NAT_PRINT("!!!FOE_INFO_START_ADDR Error!!!!\n");
    return FOE_INFO_START_ADDR_HEAD(skb);
}


uint16_t _MDrv_HWNAT_Get_Tx_Region(struct sk_buff *skb)
{
    if (IS_MAGIC_TAG_PROTECT_VALID_HEAD(skb) && IS_SPACE_AVAILABLE_HEAD(skb)) {
        FOE_INFO_START_ADDR(skb);
        FOE_TAG_PROTECT(skb) = FOE_TAG_PROTECT_HEAD(skb);
        FOE_ENTRY_NUM(skb) = FOE_ENTRY_NUM_HEAD(skb);
        FOE_ALG(skb) = FOE_ALG_HEAD(skb);
        FOE_AI(skb) = FOE_AI_HEAD(skb);
        FOE_SP(skb) = FOE_SP_HEAD(skb);
        FOE_MAGIC_TAG(skb) = FOE_MAGIC_TAG_HEAD(skb);
        if (_hwnat_info.features & HW_NAT_ARCH_WIFI_WDMA) {
            FOE_WDMA_ID(skb) = FOE_WDMA_ID_HEAD(skb);
            FOE_RX_ID(skb) = FOE_RX_ID_HEAD(skb);
            FOE_WC_ID(skb) = FOE_WC_ID_HEAD(skb);
            FOE_BSS_ID(skb) = FOE_BSS_ID_HEAD(skb);
        }
        if (_hwnat_info.features & PPTP_L2TP) {
            FOE_SOURCE(skb) = FOE_SOURCE_HEAD(skb);
            FOE_DEST(skb) = FOE_DEST_HEAD(skb);
        }
        hwnat_info_region = USE_HEAD_ROOM;
        return USE_HEAD_ROOM;   /* use headroom */
    }
    else if (IS_MAGIC_TAG_PROTECT_VALID_TAIL(skb) && IS_SPACE_AVAILABLE_TAIL(skb)) {
        FOE_INFO_START_ADDR(skb);
        FOE_TAG_PROTECT(skb) = FOE_TAG_PROTECT_TAIL(skb);
        FOE_ENTRY_NUM(skb) = FOE_ENTRY_NUM_TAIL(skb);
        FOE_ALG(skb) = FOE_ALG_TAIL(skb);
        FOE_AI(skb) = FOE_AI_TAIL(skb);
        FOE_SP(skb) = FOE_SP_TAIL(skb);
        FOE_MAGIC_TAG(skb) = FOE_MAGIC_TAG_TAIL(skb);
        if (_hwnat_info.features & HW_NAT_ARCH_WIFI_WDMA) {
            FOE_WDMA_ID(skb) = FOE_WDMA_ID_TAIL(skb);
            FOE_RX_ID(skb) = FOE_RX_ID_TAIL(skb);
            FOE_WC_ID(skb) = FOE_WC_ID_TAIL(skb);
            FOE_BSS_ID(skb) = FOE_BSS_ID_TAIL(skb);
        }
        if (_hwnat_info.features & PPTP_L2TP) {
            FOE_SOURCE(skb) = FOE_SOURCE_TAIL(skb);
            FOE_DEST(skb) = FOE_DEST_TAIL(skb);
        }
        hwnat_info_region = USE_TAIL_ROOM;
        return USE_TAIL_ROOM;   /* use tailroom */
    }
    else if (IS_MAGIC_TAG_PROTECT_VALID_CB(skb)) {
        FOE_INFO_START_ADDR(skb);
        FOE_TAG_PROTECT(skb) = FOE_TAG_PROTECT_CB0(skb);
        FOE_ENTRY_NUM(skb) = FOE_ENTRY_NUM_CB(skb);
        FOE_ALG(skb) = FOE_ALG_CB(skb);
        FOE_AI(skb) = FOE_AI_CB(skb);
        FOE_SP(skb) = FOE_SP_CB(skb);
        FOE_MAGIC_TAG(skb) = FOE_MAGIC_TAG_CB(skb);
        if (_hwnat_info.features & HW_NAT_ARCH_WIFI_WDMA) {
            FOE_WDMA_ID(skb) = FOE_WDMA_ID_CB(skb);
            FOE_RX_ID(skb) = FOE_RX_ID_CB(skb);
            FOE_WC_ID(skb) = FOE_WC_ID_CB(skb);
            FOE_BSS_ID(skb) = FOE_BSS_ID_CB(skb);
        }
        if (_hwnat_info.features & PPTP_L2TP) {
            //FOE_SOURCE(skb) = FOE_SOURCE_CB(skb);
            //FOE_DEST(skb) = FOE_DEST_CB(skb);
        }
        hwnat_info_region = USE_CB;
        return USE_CB;  /* use CB */
    }
    hwnat_info_region = ALL_INFO_ERROR;
    return ALL_INFO_ERROR;
}

uint16_t _MDrv_HWNAT_Remove_Vlan_Tag(struct sk_buff *skb)
{
    struct ethhdr *eth;
    struct vlan_ethhdr *veth;
    u16 vir_if_idx;

    veth = (struct vlan_ethhdr *)skb_mac_header(skb);
    /* something wrong */
    if ((veth->h_vlan_proto != htons(ETH_P_8021Q)) && (veth->h_vlan_proto != 0x5678)) {
        //if (pr_debug_ratelimited())
            NAT_PRINT("HNAT: Reentry packet is untagged frame?\n");
        return 65535;
    }

    vir_if_idx = ntohs(veth->h_vlan_TCI) & 0x3FFFF;

    if (skb_cloned(skb) || skb_shared(skb)) {
        struct sk_buff *new_skb;

        new_skb = skb_copy(skb, GFP_ATOMIC);
        kfree_skb(skb);
        if (!new_skb)
            return 65535;
        skb = new_skb;
    }

    /* remove VLAN tag */
    skb->data = skb_mac_header(skb);
    skb->mac_header = skb->mac_header + VLAN_HLEN;
    memmove(skb_mac_header(skb), skb->data, ETH_ALEN * 2);

    skb_pull(skb, VLAN_HLEN);
    skb->data += ETH_HLEN;  /* pointer to layer3 header */
    eth = (struct ethhdr *)skb_mac_header(skb);

    skb->protocol = eth->h_proto;

    return vir_if_idx;
}


static int _MDrv_HWNAT_Alloc_Tbl( struct HWNAT_DEVICE *phwnat)
{
    //int ret = 1;
    u32 ppe_phy_foebase_tmp;
    struct foe_entry *entry;
    struct device *dev = NULL;

    phwnat->foe_tbl_size = phwnat->table_entries* sizeof(struct foe_entry);
    MHAL_HWNAT_Get_Addr(&ppe_phy_foebase_tmp);
    dev = &(phwnat->pdev->dev);

    if (ppe_phy_foebase_tmp) {
        phwnat->ppe_phy_foe_base = (dma_addr_t)ppe_phy_foebase_tmp;
        phwnat->ppe_foe_base = (struct foe_entry *)ppe_virt_foe_base_tmp;
        HWNAT_MSG_DBG("***ppe_foe_base = %p\n", phwnat->ppe_foe_base);
        HWNAT_MSG_DBG("***PpeVirtFoeBase_tmp = %p\n", ppe_virt_foe_base_tmp);
        if (!phwnat->ppe_foe_base) {
            MHAL_HWNAT_Get_Addr(&ppe_phy_foebase_tmp);
            HWNAT_MSG_ERR("ioremap fail!!!!, Base Addr=%x\n", ppe_phy_foebase_tmp);
            return 0;
        }
    }
    else {
        phwnat->ppe_foe_base = dma_alloc_coherent(dev, phwnat->foe_tbl_size, &(phwnat->ppe_phy_foe_base), GFP_KERNEL);
        ppe_foe_base = phwnat->ppe_foe_base;
        ppe_virt_foe_base_tmp = phwnat->ppe_foe_base;
        HWNAT_MSG_DBG("init PpeVirtFoeBase_tmp = %p\n", ppe_virt_foe_base_tmp);
        HWNAT_MSG_DBG("init ppe_foe_base = %p\n", phwnat->ppe_foe_base);

        if (!phwnat->ppe_foe_base) {
            HWNAT_MSG_ERR("first ppe_phy_foe_base fail\n");
            return 0;
        }
    }

    if (!phwnat->ppe_foe_base) {
        HWNAT_MSG_ERR("ppe_foe_base== NULL\n");
        return 0;
    }

    memset(phwnat->ppe_foe_base, 0, phwnat->foe_tbl_size);
    entry = ((struct foe_entry *)phwnat->ppe_foe_base);


    if (phwnat->features & PACKET_SAMPLING) {
        phwnat->ps_tbl_size = phwnat->table_entries * sizeof(struct ps_entry);
        phwnat->ppe_ps_base = dma_alloc_coherent(dev, phwnat->ps_tbl_size, &phwnat->ppe_phy_ps_base, GFP_KERNEL);

        if (!phwnat->ppe_ps_base)
            return 0;

        memset(phwnat->ppe_ps_base, 0, phwnat->ps_tbl_size);
    }
    else {
        phwnat->ppe_ps_base = 0;
        phwnat->ppe_phy_ps_base = HWNAT_INVALID_PHY_ADDR;
    }

    if (phwnat->features & PPE_MIB) {
        phwnat->mib_tbl_size = phwnat->table_entries * sizeof(struct mib_entry);
        phwnat->ppe_mib_base = dma_alloc_coherent(dev, phwnat->mib_tbl_size, &phwnat->ppe_phy_mib_base, GFP_KERNEL);
        if (!phwnat->ppe_mib_base) {
            HWNAT_MSG_ERR("PPE MIB allocate memory fail");
            return 0;
        }
        memset(phwnat->ppe_mib_base, 0, phwnat->mib_tbl_size);
    }
    else {
        phwnat->ppe_mib_base = 0;
        phwnat->ppe_phy_mib_base  = HWNAT_INVALID_PHY_ADDR;
    }

    HWNAT_MSG_DBG("ppe_mib_base = %p\n", phwnat->ppe_mib_base);
    HWNAT_MSG_DBG("ppe_phy_mib_base = %x\n", phwnat->ppe_phy_mib_base);
    HWNAT_MSG_DBG("num_of_entry = %u\n",  phwnat->table_entries);
    HWNAT_MSG_DBG("sizeof(struct mib_entry) = %u\n",  sizeof(struct mib_entry));
    HWNAT_MSG_DBG("mib_tbl_size = %d\n",  phwnat->mib_tbl_size);

    return 1;
}





static MS_BOOL _MDrv_HWNAT_Get_Bridge_Info(void)
{
    struct net_device *br0_dev;
    struct in_device *br0_in_dev;

    br0_dev = dev_get_by_name(&init_net,"br0");
    if (br0_dev == NULL) {
        if (_hwnat_info.debug_level >= 7) {
            NAT_PRINT("br0_dev = NULL\n");
        }
        return FALSE;
    }
    br0_in_dev = in_dev_get(br0_dev);
    if (br0_in_dev == NULL) {
        if (_hwnat_info.debug_level >= 7) {
            NAT_PRINT("br0_in_dev = NULL\n");
        }
        return FALSE;
    }
    brNetmask = ntohl(br0_in_dev->ifa_list->ifa_mask);
    br0Ip = ntohl(br0_in_dev->ifa_list->ifa_address);

    if(br0_dev != NULL) {
        dev_put(br0_dev);
    }

    if (br0_in_dev != NULL) {
        in_dev_put(br0_in_dev);
    }

    NAT_PRINT("br0Ip = %x\n", br0Ip);
    NAT_PRINT("brNetmask = %x\n", brNetmask);
    getBrLan = 1;

    return TRUE;
}


static int _MDrv_HWNAT_Is_BrLan_Subnet(struct sk_buff *skb)
{
    struct iphdr *iph = NULL;
    uint32_t daddr = 0;
    uint32_t saddr = 0;
    u32 eth_type, ppp_tag = 0;
    struct vlan_hdr *vh = NULL;
    struct ethhdr *eth = NULL;
    struct pppoe_hdr *peh = NULL;
    u8 vlan1_gap = 0;
    u8 vlan2_gap = 0;
    u8 pppoe_gap = 0;

    struct vlan_hdr pseudo_vhdr;

    eth = (struct ethhdr *)skb->data;
    if(is_multicast_ether_addr(&eth->h_dest[0]))
        return 0;
    eth_type = eth->h_proto;
    if ((eth_type == htons(ETH_P_8021Q)) ||
        (((eth_type) & 0x00FF) == htons(ETH_P_8021Q)) || hwnat_vlan_tx_tag_present(skb)) {

        if (_hwnat_info.features & HW_NAT_VLAN_TX) {
            pseudo_vhdr.h_vlan_TCI = htons(hwnat_vlan_tag_get(skb));
            pseudo_vhdr.h_vlan_encapsulated_proto = eth->h_proto;
            vh = (struct vlan_hdr *)&pseudo_vhdr;
            vlan1_gap = VLAN_HLEN;
        }
        else {
            vlan1_gap = VLAN_HLEN;
            vh = (struct vlan_hdr *)(skb->data + ETH_HLEN);
        }
        /* VLAN + PPPoE */
        if (ntohs(vh->h_vlan_encapsulated_proto) == ETH_P_PPP_SES) {
            pppoe_gap = 8;
            eth_type = vh->h_vlan_encapsulated_proto;
            /* Double VLAN = VLAN + VLAN */
        }
        else if ((vh->h_vlan_encapsulated_proto == htons(ETH_P_8021Q)) ||
               ((vh->h_vlan_encapsulated_proto) & 0x00FF) == htons(ETH_P_8021Q)) {

            vlan2_gap = VLAN_HLEN;
            vh = (struct vlan_hdr *)(skb->data + ETH_HLEN + VLAN_HLEN);
            /* VLAN + VLAN + PPPoE */
            if (ntohs(vh->h_vlan_encapsulated_proto) == ETH_P_PPP_SES) {
                pppoe_gap = 8;
                eth_type = vh->h_vlan_encapsulated_proto;
            }else
                eth_type = vh->h_vlan_encapsulated_proto;
        }
    }
    else if (ntohs(eth_type) == ETH_P_PPP_SES) {
        /* PPPoE + IP */
        pppoe_gap = 8;
        peh = (struct pppoe_hdr *)(skb->data + ETH_HLEN + vlan1_gap);
        ppp_tag = peh->tag[0].tag_type;
    }

    if (getBrLan == 0) {
        if (_MDrv_HWNAT_Get_Bridge_Info() == FALSE)  /*br0 get fail*/
            return 0;
    }
    /* set layer4 start addr */
    if ((eth_type == htons(ETH_P_IP)) || (eth_type == htons(ETH_P_PPP_SES) && ppp_tag == htons(PPP_IP))) {
        iph = (struct iphdr *)(skb->data + ETH_HLEN + vlan1_gap + vlan2_gap + pppoe_gap);
        daddr = ntohl(iph->daddr);
        saddr = ntohl(iph->saddr);
    }

    if (((br0Ip & brNetmask) == (daddr & brNetmask)) && ((daddr & brNetmask) == (saddr & brNetmask)))
        return 1;
    return 0;
}

static int _MDrv_HWNAT_Is_Rx_Short_Cut(struct sk_buff *skb)
{
    struct iphdr *iph = NULL;
    uint32_t daddr;

    if (getBrLan == 0) {
        if (_MDrv_HWNAT_Get_Bridge_Info() == FALSE) /*br0 get fail*/
            return 0;
    }

    iph = (struct iphdr *)(skb->data);
    daddr = ntohl(iph->daddr);
    if ((br0Ip & brNetmask) == (daddr & brNetmask))
        return 1;
    else
        return 0;
}

static uint32_t _MDrv_HWNAT_Get_Extif_Idx(struct net_device *dev)
{

    if (_hwnat_info.features & HW_NAT_MBSS_SUPPORT) {
        if (dev == dst_port[DP_RA1])
            return DP_RA1;
        else if (dev == dst_port[DP_RA2])
            return DP_RA2;
        else if (dev == dst_port[DP_RA3])
            return DP_RA3;
        else if (dev == dst_port[DP_RA4])
            return DP_RA4;
        else if (dev == dst_port[DP_RA5])
            return DP_RA5;
        else if (dev == dst_port[DP_RA6])
            return DP_RA6;
        else if (dev == dst_port[DP_RA7])
            return DP_RA7;
        else if (dev == dst_port[DP_RA8])
            return DP_RA8;
        else if (dev == dst_port[DP_RA9])
            return DP_RA9;
        else if (dev == dst_port[DP_RA10])
            return DP_RA10;
        else if (dev == dst_port[DP_RA11])
            return DP_RA11;
        else if (dev == dst_port[DP_RA12])
            return DP_RA12;
        else if (dev == dst_port[DP_RA13])
            return DP_RA13;
        else if (dev == dst_port[DP_RA14])
            return DP_RA14;
        else if (dev == dst_port[DP_RA15])
            return DP_RA15;
    }

    if (_hwnat_info.features & HW_NAT_WDS_SUPPORT) {
        if (dev == dst_port[DP_WDS0])
            return DP_WDS0;
        else if (dev == dst_port[DP_WDS1])
            return DP_WDS1;
        else if (dev == dst_port[DP_WDS2])
            return DP_WDS2;
        else if (dev == dst_port[DP_WDS3])
            return DP_WDS3;
    }

    if (_hwnat_info.features & HW_NAT_APCLI_SUPPORT) {
        if (dev == dst_port[DP_APCLI0])
            return DP_APCLI0;
        else if (dev == dst_port[DP_APCLII0])
            return DP_APCLII0;
    }

    if (_hwnat_info.features & HW_NAT_AP_MESH_SUPPORT) {
        if (dev == dst_port[DP_MESH0])
            return DP_MESH0;
        else if (dev == dst_port[DP_MESHI0])
            return DP_MESHI0;

    }

    if (IS_SPECIAL_DEV_OR_AP(_hwnat_info.features)) {
        if (dev == dst_port[DP_RAI0])
            return DP_RAI0;

        if (_hwnat_info.features & HW_NAT_AP_MBSS_SUPPORT) {
            if (dev == dst_port[DP_RAI1])
                return DP_RAI1;
            else if (dev == dst_port[DP_RAI2])
                return DP_RAI2;
            else if (dev == dst_port[DP_RAI3])
                return DP_RAI3;
            else if (dev == dst_port[DP_RAI4])
                return DP_RAI4;
            else if (dev == dst_port[DP_RAI5])
                return DP_RAI5;
            else if (dev == dst_port[DP_RAI6])
                return DP_RAI6;
            else if (dev == dst_port[DP_RAI7])
                return DP_RAI7;
            else if (dev == dst_port[DP_RAI8])
                return DP_RAI8;
            else if (dev == dst_port[DP_RAI9])
                return DP_RAI9;
            else if (dev == dst_port[DP_RAI10])
                return DP_RAI10;
            else if (dev == dst_port[DP_RAI11])
                return DP_RAI11;
            else if (dev == dst_port[DP_RAI12])
                return DP_RAI12;
            else if (dev == dst_port[DP_RAI13])
                return DP_RAI13;
            else if (dev == dst_port[DP_RAI14])
                return DP_RAI14;
            else if (dev == dst_port[DP_RAI15])
                return DP_RAI15;
        }
    }

    if (_hwnat_info.features & HW_NAT_WDS_SUPPORT) {
        if (dev == dst_port[DP_WDSI0])
            return DP_WDSI0;
        else if (dev == dst_port[DP_WDSI1])
            return DP_WDSI1;
        else if (dev == dst_port[DP_WDSI2])
            return DP_WDSI2;
        else if (dev == dst_port[DP_WDSI3])
            return DP_WDSI3;
    }


    if (_hwnat_info.features & HW_NAT_NIC_USB) {
        if (dev == dst_port[DP_PCI])
            return DP_PCI;
        else if (dev == dst_port[DP_USB])
            return DP_USB;
    }

    if (dev == dst_port[DP_RA0])
        return DP_RA0;

    NAT_PRINT("HNAT: %s The interface %p is unknown \n", __func__, dev);

    return 0;
}


/* push different VID for WiFi pseudo interface or USB external NIC */
uint32_t _MDrv_HWNAT_Handle_Extif_Rx(struct sk_buff *skb)
{
    u16 vir_if_idx = 0;
    int i = 0;
    int dev_match = 0;
    struct ethhdr *eth = NULL;

    if (((_hwnat_info.features & HW_NAT_WIFI) == 0) && ((_hwnat_info.features & HW_NAT_NIC_USB) == 0)) {
        return 1;
    }

    eth = (struct ethhdr *)skb_mac_header(skb);

    /* PPE can only handle IPv4/IPv6/PPP packets */
    if (((skb->protocol != htons(ETH_P_8021Q)) &&
         (skb->protocol != htons(ETH_P_IP)) && (skb->protocol != htons(ETH_P_IPV6)) &&
         (skb->protocol != htons(ETH_P_PPP_SES)) && (skb->protocol != htons(ETH_P_PPP_DISC))) ||
        is_multicast_ether_addr(&eth->h_dest[0])) {
        return 1;
    }

    if (_hwnat_info.features & HW_NAT_WIFI_NEW_ARCH) {
        for (i = 0; i < MAX_IF_NUM; i++) {
            if (dst_port[i] == skb->dev) {
                vir_if_idx = i;
                dev_match = 1;
                /* NAT_PRINT("Interface=%s, vir_if_idx=%x\n", skb->dev->name, vir_if_idx); */
                break;
            }
        }
        if (dev_match == 0) {
            if (pr_debug_ratelimited())
                NAT_PRINT("UnKnown Interface (%s), vir_if_idx=%x\n", skb->dev->name, vir_if_idx);
            /* kfree_skb(skb); */
            return 1;
        }
    }
    else {
        vir_if_idx = _MDrv_HWNAT_Get_Extif_Idx(skb->dev);
        skb_set_network_header(skb, 0);
        if (_hwnat_info.features & WLAN_OPTIMIZE) {
            /*ppe_rx_parse_layer_info(skb);*/
            if (_MDrv_HWNAT_Is_Rx_Short_Cut(skb))
                return 1;   /* Bridge ==> sw path (rps) */
        }
    }
    /* push vlan tag to stand for actual incoming interface, */
    /* so HNAT module can know the actual incoming interface from vlan id. */
    skb_push(skb, ETH_HLEN);/* pointer to layer2 header before calling hard_start_xmit */
#ifdef CONFIG_SUPPORT_WLAN_QOS
    MDrv_HWNAT_Set_Qid(skb);
#endif

    skb->vlan_proto = htons(ETH_P_8021Q);
    if (_hwnat_info.features & HW_NAT_VLAN_TX) {
        skb->vlan_tci |= VLAN_TAG_PRESENT;
        skb->vlan_tci |= vir_if_idx;
    }
    else {
        #if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
        skb = __vlan_put_tag(skb, skb->vlan_proto, vir_if_idx);
        #else
        skb = vlan_insert_tag(skb, skb->vlan_proto, vir_if_idx);
        #endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0) */
    }
    /* redirect to PPE */
    FOE_AI_HEAD(skb) = UN_HIT;
    FOE_AI_TAIL(skb) = UN_HIT;
    FOE_TAG_PROTECT_HEAD(skb) = TAG_PROTECT;
    FOE_TAG_PROTECT_TAIL(skb) = TAG_PROTECT;
    FOE_MAGIC_TAG_HEAD(skb) = FOE_MAGIC_PPE;
    FOE_MAGIC_TAG_TAIL(skb) = FOE_MAGIC_PPE;


    if (_hwnat_info.features & HW_NAT_WIFI_NEW_ARCH)
        skb->dev = dst_port[DP_GMAC1];  /* we use GMAC1 to send the packet to PPE */
    else
        skb->dev = dst_port[DP_GMAC];   /* we use GMAC1 to send the packet to PPE */

#ifdef CONFIG_SUPPORT_WLAN_QOS
    if (_hwnat_info.debug_level >= 2)
        NAT_PRINT("skb->dev = %s\n", skb->dev);
    if ((skb->dev == NULL) || ((skb->dev != dst_port[DPORT_GMAC2]) && (skb->dev != dst_port[DP_GMAC1]))) {
        if (_hwnat_info.features & HW_NAT_WIFI_NEW_ARCH)
            skb->dev = dst_port[DP_GMAC1];  /* we use GMAC1 to send the packet to PPE */
        else
            skb->dev = dst_port[DP_GMAC];   /* we use GMAC1 to send the packet to PPE */
    }
#endif


    dev_queue_xmit(skb);
    return 0;
}

uint32_t _MDrv_HWNAT_Handle_Pptp_L2tp_Pingpong(struct sk_buff *skb, struct foe_entry *entry)
{
    unsigned int addr = 0;
    unsigned int src_ip = 0;

    /* PPTP/L2TP use this pingpong tech */
    /* NAT_PRINT("get fast_path ping pong skb_length is %d\n", skb->len); */
    if ((pptp_fast_path || l2tp_fast_path) && ((skb->len == 110) || (skb->len == 98))) {
        if (FOE_AI(skb) == UN_HIT) {
            dev_kfree_skb_any(skb); /*avoid memory leak */
            return 0;
        }
        /* wan->lan ping-pong, pass up to tx handler for binding */
        /* NAT_PRINT("Parse ping pong packets FOE_AI(skb)= 0x%2x!!\n", FOE_AI(skb)); */
        /* get start addr for each layer */
        if (MDrv_HWNAT_Pptp_Lan_Parse_Layer(skb)) {
            memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
            dev_kfree_skb_any(skb);
            return 0;
        }
        if (_hwnat_info.debug_level >= 1)
            NAT_PRINT("_MDrv_HWNAT_Fill_L2\n");
        /* Set Layer2 Info */
        if (_MDrv_HWNAT_Fill_L2(skb, entry)) {
            memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
            dev_kfree_skb_any(skb);
            return 0;
        }

        if (_hwnat_info.debug_level >= 1)
            NAT_PRINT("_MDrv_HWNAT_Fill_L3\n");
        /* Set Layer3 Info */
        if (_MDrv_HWNAT_Fill_L3(skb, entry)) {
            memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
            dev_kfree_skb_any(skb);
            return 0;
        }

        if (_hwnat_info.debug_level >= 1)
            NAT_PRINT("_MDrv_HWNAT_Fill_L4\n");
        /* Set Layer4 Info */
        if (_MDrv_HWNAT_Fill_L4(skb, entry)) {
            memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
            dev_kfree_skb_any(skb);
            return 0;
        }

        /* Set force port info to 1  */
        /* if (MDrv_HWNAT_Set_Force_Port(skb, entry, gmac_no)) */
        if (MDrv_HWNAT_Set_Force_Port(skb, entry, 1)) {
            memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
            dev_kfree_skb_any(skb);
            return 0;
        }
        /* Enter binding state */
        _MDrv_HWNAT_Set_Entry_Bind(skb, entry);
        fast_bind = 1;
        addr = ((htons(entry->ipv4_hnapt.sport) << 16) | htons(entry->ipv4_hnapt.dport));
        src_ip = (htonl(entry->ipv4_hnapt.sip));
        MDrv_HWNAT_Pptp_L2tp_Update_Entry(ppe_parse_result.iph.protocol, addr, src_ip, FOE_ENTRY_NUM(skb));
        /*free pingpong packet */
        dev_kfree_skb_any(skb);
        return 0;
    }
    return 1;
}

uint32_t _MDrv_HWNAT_Handle_Extif_Pingpong(struct sk_buff *skb)
{
    struct ethhdr *eth = NULL;
    u16 vir_if_idx = 0;
    struct net_device *dev;

    if (((_hwnat_info.features & HW_NAT_WIFI) == 0) && ((_hwnat_info.features & HW_NAT_NIC_USB) == 0)) {
        return 1;
    }
    vir_if_idx = _MDrv_HWNAT_Remove_Vlan_Tag(skb);

    /* recover to right incoming interface */
    if (vir_if_idx < MAX_IF_NUM && dst_port[vir_if_idx]) {
        skb->dev = dst_port[vir_if_idx];
    }
    else {
        if (_hwnat_info.debug_level >= 1)
            NAT_PRINT("%s : HNAT: unknown interface (vir_if_idx=%d)\n", __func__, vir_if_idx);
        return 1;
    }

    eth = (struct ethhdr *)skb_mac_header(skb);

    if (eth->h_dest[0] & 1) {
        if (memcmp(eth->h_dest, skb->dev->broadcast, ETH_ALEN) == 0) {
            skb->pkt_type = PACKET_BROADCAST;
        } else {
            skb->pkt_type = PACKET_MULTICAST;
        }
    }
    else {

        skb->pkt_type = PACKET_OTHERHOST;
        for (vir_if_idx = 0; vir_if_idx < MAX_IF_NUM; vir_if_idx++) {
            dev = dst_port[vir_if_idx];
            if (dev !=NULL && memcmp(eth->h_dest, dev->dev_addr, ETH_ALEN) == 0) {
                skb->pkt_type = PACKET_HOST;
                break;
            }
        }
    }

    return 1;
}

uint32_t _MDrv_HWNAT_Handle_Keep_Alive(struct sk_buff *skb, struct foe_entry *entry)
{
    struct ethhdr *eth = NULL;
    u16 eth_type = ntohs(skb->protocol);
    u32 vlan1_gap = 0;
    u32 vlan2_gap = 0;
    u32 pppoe_gap = 0;
    struct vlan_hdr *vh;
    struct iphdr *iph = NULL;
    struct tcphdr *th = NULL;
    struct udphdr *uh = NULL;

/* try to recover to original SMAC/DMAC, but we don't have such information.*/
/* just use SMAC as DMAC and set Multicast address as SMAC.*/
    eth = (struct ethhdr *)(skb->data - ETH_HLEN);

    MDrv_HWNAT_Util_Memcpy(eth->h_dest, eth->h_source, ETH_ALEN);
    MDrv_HWNAT_Util_Memcpy(eth->h_source, eth->h_dest, ETH_ALEN);
    eth->h_source[0] = 0x1; /* change to multicast packet, make bridge not learn this packet */
    if (eth_type == ETH_P_8021Q) {
        vlan1_gap = VLAN_HLEN;
        vh = (struct vlan_hdr *)skb->data;

        if (ntohs(vh->h_vlan_TCI) == _hwnat_info.wan_vid) {
            /* It make packet like coming from LAN port */
            vh->h_vlan_TCI = htons(_hwnat_info.lan_vid);
        }
        else {
            /* It make packet like coming from WAN port */
            vh->h_vlan_TCI = htons(_hwnat_info.wan_vid);
        }

        if (ntohs(vh->h_vlan_encapsulated_proto) == ETH_P_PPP_SES) {
            pppoe_gap = 8;
        }
        else if (ntohs(vh->h_vlan_encapsulated_proto) == ETH_P_8021Q) {
            vlan2_gap = VLAN_HLEN;
            vh = (struct vlan_hdr *)(skb->data + VLAN_HLEN);

            /* VLAN + VLAN + PPPoE */
            if (ntohs(vh->h_vlan_encapsulated_proto) == ETH_P_PPP_SES) {
                pppoe_gap = 8;
            }
            else {
                /* VLAN + VLAN + IP */
                eth_type = ntohs(vh->h_vlan_encapsulated_proto);
            }
        }
        else {
            /* VLAN + IP */
            eth_type = ntohs(vh->h_vlan_encapsulated_proto);
        }
    }

    /* Only Ipv4 NAT need KeepAlive Packet to refresh iptable */
    if (eth_type == ETH_P_IP) {
        iph = (struct iphdr *)(skb->data + vlan1_gap + vlan2_gap + pppoe_gap);
        /* Recover to original layer 4 header */
        if (iph->protocol == IPPROTO_TCP) {
            th = (struct tcphdr *)((uint8_t *)iph + iph->ihl * 4);
            MDrv_HWNAT_Util_Calc_Tcphdr(entry, iph, th);
        }
        else if (iph->protocol == IPPROTO_UDP) {
            uh = (struct udphdr *)((uint8_t *)iph + iph->ihl * 4);
            MDrv_HWNAT_Util_Calc_Udphdr(entry, iph, uh);
        }
        /* Recover to original layer 3 header */
        MDrv_HWNAT_Util_Calc_Iphdr(entry, iph);
        skb->pkt_type = PACKET_HOST;
    }
    else if (eth_type == ETH_P_IPV6) {
        skb->pkt_type = PACKET_HOST;
    }
    else {
        skb->pkt_type = PACKET_HOST;
    }
/* Ethernet driver will call eth_type_trans() to update skb->pkt_type.*/
/* If(destination mac != my mac)*/
/*   skb->pkt_type=PACKET_OTHERHOST;*/
/* In order to pass ip_rcv() check, we change pkt_type to PACKET_HOST here*/
/*  skb->pkt_type = PACKET_HOST;*/
    return 1;
}

int _MDrv_HWNAT_Handle_Hitbind_Force_Cpu(struct sk_buff *skb, struct foe_entry *entry)
{
    u16 vir_if_idx = 0;
    if (_hwnat_info.features & HW_NAT_QDMA) {
        vir_if_idx = _MDrv_HWNAT_Remove_Vlan_Tag(skb);
        if (vir_if_idx != 65535) {
            if (vir_if_idx >= FOE_4TB_SIZ) {
                if (_hwnat_info.debug_level >= 1)
                    NAT_PRINT("%s, entry_index error(%u)\n", __func__, vir_if_idx);
                vir_if_idx = FOE_ENTRY_NUM(skb);
                kfree_skb(skb);
                return 0;
            }
            entry = FOE_ENTRY_BASE_BY_IDX(_hwnat_info.ppe_foe_base, vir_if_idx);
        }
    }
    if (IS_IPV4_HNAT(entry) || IS_IPV4_HNAPT(entry))
        skb->dev = dst_port[entry->ipv4_hnapt.act_dp];
    else if (IS_IPV4_DSLITE(entry))
        skb->dev = dst_port[entry->ipv4_dslite.act_dp];
    else if (IS_IPV6_3T_ROUTE(entry))
        skb->dev = dst_port[entry->ipv6_3t_route.act_dp];
    else if (IS_IPV6_5T_ROUTE(entry))
        skb->dev = dst_port[entry->ipv6_5t_route.act_dp];
    else if (IS_IPV6_6RD(entry))
        skb->dev = dst_port[entry->ipv6_6rd.act_dp];
    else
        return 1;
    /* interface is unknown */
    if (!skb->dev) {
        if (_hwnat_info.debug_level >= 1)
            NAT_PRINT("%s, interface is unknown\n", __func__);
        kfree_skb(skb);
        return 0;
    }
    skb_set_network_header(skb, 0);
    skb_push(skb, ETH_HLEN);    /* pointer to layer2 header */
    dev_queue_xmit(skb);
    return 0;
}

int _MDrv_HWNAT_Handle_Hitbind_Force_Mcast_To_Wifi(struct sk_buff *skb)
{
    int i = 0;
    struct sk_buff *skb2;

    if (((_hwnat_info.features & HW_NAT_WIFI) == 0) && ((_hwnat_info.features & HW_NAT_NIC_USB) == 0)) {
        kfree_skb(skb);
        return 0;
    }

    if ((_hwnat_info.features & HW_NAT_PSEUDO_SUPPORT) == 0) {
        /*if we only use GMAC1, we need to use vlan id to identify LAN/WAN port*/
        /*otherwise, CPU send untag packet to switch so we don't need to*/
        /*remove vlan tag before sending to WiFi interface*/
        _MDrv_HWNAT_Remove_Vlan_Tag(skb);   /* pointer to layer3 header */
    }
    skb_set_network_header(skb, 0);
    skb_push(skb, ETH_HLEN);    /* pointer to layer2 header */

    if (_hwnat_info.features & HW_NAT_WIFI) {
        for (i = DP_RA0; i < MAX_WIFI_IF_NUM; i++) {
            if (dst_port[i]) {
                skb2 = skb_clone(skb, GFP_ATOMIC);

                if (!skb2)
                    return -ENOMEM;

                skb2->dev = dst_port[i];
                dev_queue_xmit(skb2);
            }
        }
    }

    if (_hwnat_info.features & HW_NAT_NIC_USB) {
        if (dst_port[DP_PCI]) {
            skb2 = skb_clone(skb, GFP_ATOMIC);

            if (!skb2)
                return -ENOMEM;

            skb2->dev = dst_port[DP_PCI];
            dev_queue_xmit(skb2);
        }
    }
    kfree_skb(skb);

    return 0;
}

int32_t MDrv_HWNAT_Handle_Rx(struct sk_buff *skb)
{
    struct foe_entry *entry = FOE_ENTRY_BASE_BY_PKT(_hwnat_info.ppe_foe_base, skb);
    struct vlan_ethhdr *veth;
    if (_hwnat_info.debug_level >= 7)
        MDrv_HWNAT_Dump_Skb(skb);


#if PPTP_L2TP
    /*FP to-lan on/off*/
    if (_hwnat_info.features & PPTP_L2TP) {
        if (pptp_fast_path && (skb->len != (124 - 14))) {
            int ret = 1000;
            /*remove pptp/l2tp header, tx to PPE */
            ret = MDrv_HWNAT_Pptp_Lan(skb);
            /*ret 0, remove header ok */
            if (ret == 0)
                return ret;
        }
        if (l2tp_fast_path && (skb->len != (124 - 14))) {
            int ret = 1000;
            /*remove pptp/l2tp header, tx to PPE */
            ret = MDrv_HWNAT_L2tp_Lan(skb);
            if (ret == 0)
                return ret;
        }
    }
#endif
    if (_hwnat_info.debug_level >= 7)
        NAT_PRINT("[%s][%d] FOE_SP(skb) = %d \n",__FUNCTION__,__LINE__, FOE_SP(skb));
    if (_hwnat_info.features & HW_NAT_QDMA) {
        /* QDMA QoS remove CPU reason, we use special tag to identify force to CPU
         * Notes: CPU reason & Entry ID fileds are invalid at this moment
         */
        if (FOE_SP(skb) == 5) {
            veth = (struct vlan_ethhdr *)skb_mac_header(skb);

            if (veth->h_vlan_proto == 0x5678) {
                if (_hwnat_info.features & PPTP_L2TP) {
                    u16 vir_if_idx = 0;
                    vir_if_idx = _MDrv_HWNAT_Remove_Vlan_Tag(skb);
                    entry = FOE_ENTRY_BASE_BY_IDX(_hwnat_info.ppe_foe_base, vir_if_idx);
                }
                else {
                    return _MDrv_HWNAT_Handle_Hitbind_Force_Cpu(skb, entry);
                }
            }
        }
    }
    /* the incoming packet is from PCI or WiFi interface */
    if (((FOE_MAGIC_TAG(skb) == FOE_MAGIC_PCI) || (FOE_MAGIC_TAG(skb) == FOE_MAGIC_WLAN))) {
        return _MDrv_HWNAT_Handle_Extif_Rx(skb);
    }
    else if (FOE_AI(skb) == HIT_BIND_FORCE_TO_CPU) {
        if (_hwnat_info.features & PPTP_L2TP) {
            if (pptp_fast_path) {
                /*to ppp0, add pptp/l2tp header and send out */
                int ret = 1000;
                /* NAT_PRINT("PPTP LAN->WAN  get bind packet!!\n"); */
                ret = MDrv_HWNAT_Pptp_Wan(skb);
                return ret;
            }
            else if (l2tp_fast_path) {
                /*to ppp0, add pptp/l2tp header and send out */
                int ret = 1000;

                ret = MDrv_HWNAT_L2tp_Wan(skb);
                /* NAT_PRINT("L2TP LAN->WAN fast send bind packet and return %d!!\n", ret); */
                return ret;
            }
        }
        return _MDrv_HWNAT_Handle_Hitbind_Force_Cpu(skb, entry);

        /* handle the incoming packet which came back from PPE */
    }
    else if ((IS_IF_PCIE_WLAN_RX(skb) && ((FOE_SP(skb) == 0) || (FOE_SP(skb) == 5))) &&
           (FOE_AI(skb) != HIT_BIND_KEEPALIVE_UC_OLD_HDR) &&
           (FOE_AI(skb) != HIT_BIND_KEEPALIVE_MC_NEW_HDR) &&
           (FOE_AI(skb) != HIT_BIND_KEEPALIVE_DUP_OLD_HDR)) {
        if (_hwnat_info.features & PPTP_L2TP) {
            if (_MDrv_HWNAT_Handle_Pptp_L2tp_Pingpong(skb, entry) == 0) {
                return 0;
            }
        }
        return _MDrv_HWNAT_Handle_Extif_Pingpong(skb);
    }
    else if (FOE_AI(skb) == HIT_BIND_KEEPALIVE_UC_OLD_HDR) {
        if (_hwnat_info.debug_level >= 3)
            NAT_PRINT("Got HIT_BIND_KEEPALIVE_UC_OLD_HDR packet (hash index=%d)\n", FOE_ENTRY_NUM(skb));
        if (_hwnat_info.features & PPTP_L2TP) {
            if (pptp_fast_path) {
                dev_kfree_skb_any(skb);
                return 0;
            }
        }
        return 1;
    }
    else if (FOE_AI(skb) == HIT_BIND_MULTICAST_TO_CPU || FOE_AI(skb) == HIT_BIND_MULTICAST_TO_GMAC_CPU) {
        return _MDrv_HWNAT_Handle_Hitbind_Force_Mcast_To_Wifi(skb);
    }
    else if (FOE_AI(skb) == HIT_BIND_KEEPALIVE_MC_NEW_HDR) {
        if (_hwnat_info.debug_level >= 3) {
            NAT_PRINT("Got HIT_BIND_KEEPALIVE_MC_NEW_HDR packet (hash index=%d)\n", FOE_ENTRY_NUM(skb));
        }

        if (_MDrv_HWNAT_Handle_Keep_Alive(skb, entry)) {
            return 1;
        }
    } else if (FOE_AI(skb) == HIT_BIND_KEEPALIVE_DUP_OLD_HDR) {
        if (_hwnat_info.debug_level >= 3)
            NAT_PRINT("RxGot HIT_BIND_KEEPALIVE_DUP_OLD_HDR packe (hash index=%d)\n", FOE_ENTRY_NUM(skb));
        if (_hwnat_info.features & PPTP_L2TP) {
            if (pptp_fast_path) {
                dev_kfree_skb_any(skb);
                return 0;
            }
        }
        return 1;
    }
    return 1;
}

int32_t _MDrv_HWNAT_Get_Pppoe_Sid(struct sk_buff *skb, uint32_t vlan_gap, u16 *sid, uint16_t *ppp_tag)
{
    struct pppoe_hdr *peh = NULL;

    peh = (struct pppoe_hdr *)(skb->data + ETH_HLEN + vlan_gap);

    if (_hwnat_info.debug_level >= 6) {
        NAT_PRINT("\n==============\n");
        NAT_PRINT(" Ver=%d\n", peh->ver);
        NAT_PRINT(" Type=%d\n", peh->type);
        NAT_PRINT(" Code=%d\n", peh->code);
        NAT_PRINT(" sid=%x\n", ntohs(peh->sid));
        NAT_PRINT(" Len=%d\n", ntohs(peh->length));
        NAT_PRINT(" tag_type=%x\n", ntohs(peh->tag[0].tag_type));
        NAT_PRINT(" tag_len=%d\n", ntohs(peh->tag[0].tag_len));
        NAT_PRINT("=================\n");
    }

    *ppp_tag = peh->tag[0].tag_type;
    if (_hwnat_info.features & NAT_IPV6) {
        if (peh->ver != 1 || peh->type != 1 || (*ppp_tag != htons(PPP_IP) && *ppp_tag != htons(PPP_IPV6))) {
            return 1;
        }
    }
    else {
        if (peh->ver != 1 || peh->type != 1 || *ppp_tag != htons(PPP_IP))
            return 1;
    }

    *sid = peh->sid;
    return 0;
}

/* HNAT_V2 can push special tag */
int32_t _MDrv_HWNAT_Is_Special_Tag(uint16_t eth_type)
{
    /* Please modify this function to speed up the packet with special tag
     * Ex:
     *    Ralink switch = 0x81xx
     *    Realtek switch = 0x8899
     */
    if ((eth_type & 0x00FF) == htons(ETH_P_8021Q)) {   /* Ralink Special Tag: 0x81xx */
        ppe_parse_result.vlan_tag = eth_type;
        return 1;
    } else {
        return 0;
    }
}

int32_t _MDrv_HWNAT_Is_802_1q(uint16_t eth_type)
{
    if (eth_type == htons(ETH_P_8021Q)) {
        ppe_parse_result.vlan_tag = eth_type;
        return 1;
    } else {
        return 0;
    }
}

int32_t _MDrv_HWNAT_Is_Hw_Vlan_Tx(struct sk_buff *skb)
{
    if (_hwnat_info.features & HW_NAT_VLAN_TX) {
        if (hwnat_vlan_tx_tag_present(skb)) {
            ppe_parse_result.vlan_tag = htons(ETH_P_8021Q);
            return 1;
        } else {
            return 0;
        }
    }
    return 0;
}

int32_t _MDrv_HWNAT_Parse_Pkt_Layer(struct sk_buff *skb)
{
    struct vlan_hdr *vh = NULL;
    struct ethhdr *eth = NULL;
    struct iphdr *iph = NULL;
    struct ipv6hdr *ip6h = NULL;
    struct tcphdr *th = NULL;
    struct udphdr *uh = NULL;
    u8 ipv6_head_len = 0;
    struct vlan_hdr pseudo_vhdr;

    memset(&ppe_parse_result, 0, sizeof(ppe_parse_result));

    eth = (struct ethhdr *)skb->data;
    MDrv_HWNAT_Util_Memcpy(ppe_parse_result.dmac, eth->h_dest, ETH_ALEN);
    MDrv_HWNAT_Util_Memcpy(ppe_parse_result.smac, eth->h_source, ETH_ALEN);
    ppe_parse_result.eth_type = eth->h_proto;
/* we cannot speed up multicase packets because both wire and wireless PCs might join same multicast group. */
    if (is_multicast_ether_addr(&eth->h_dest[0]))
        ppe_parse_result.is_mcast = 1;
    else
        ppe_parse_result.is_mcast = 0;
    if (_MDrv_HWNAT_Is_802_1q(ppe_parse_result.eth_type) ||
        _MDrv_HWNAT_Is_Special_Tag(ppe_parse_result.eth_type) || _MDrv_HWNAT_Is_Hw_Vlan_Tx(skb)) {
        if (_hwnat_info.features & HW_NAT_VLAN_TX) {
            ppe_parse_result.vlan1_gap = 0;
            ppe_parse_result.vlan_layer++;
            pseudo_vhdr.h_vlan_TCI = htons(hwnat_vlan_tag_get(skb));
            pseudo_vhdr.h_vlan_encapsulated_proto = eth->h_proto;
            vh = (struct vlan_hdr *)&pseudo_vhdr;
        }
        else {
            ppe_parse_result.vlan1_gap = VLAN_HLEN;
            ppe_parse_result.vlan_layer++;
            vh = (struct vlan_hdr *)(skb->data + ETH_HLEN);
        }
        ppe_parse_result.vlan1 = vh->h_vlan_TCI;
        /* VLAN + PPPoE */
        if (ntohs(vh->h_vlan_encapsulated_proto) == ETH_P_PPP_SES) {
            ppe_parse_result.pppoe_gap = 8;
            if (_MDrv_HWNAT_Get_Pppoe_Sid(skb, ppe_parse_result.vlan1_gap,
                      &ppe_parse_result.pppoe_sid,
                      &ppe_parse_result.ppp_tag)) {
                return 1;
            }
            ppe_parse_result.eth_type = vh->h_vlan_encapsulated_proto;
            /* Double VLAN = VLAN + VLAN */
        } else if (_MDrv_HWNAT_Is_802_1q(vh->h_vlan_encapsulated_proto) ||
               _MDrv_HWNAT_Is_Special_Tag(vh->h_vlan_encapsulated_proto)) {
            ppe_parse_result.vlan2_gap = VLAN_HLEN;
            ppe_parse_result.vlan_layer++;
            vh = (struct vlan_hdr *)(skb->data + ETH_HLEN + ppe_parse_result.vlan1_gap);
            ppe_parse_result.vlan2 = vh->h_vlan_TCI;

            /* VLAN + VLAN + PPPoE */
            if (ntohs(vh->h_vlan_encapsulated_proto) == ETH_P_PPP_SES) {
                ppe_parse_result.pppoe_gap = 8;
                if (_MDrv_HWNAT_Get_Pppoe_Sid
                    (skb,
                     (ppe_parse_result.vlan1_gap + ppe_parse_result.vlan2_gap),
                     &ppe_parse_result.pppoe_sid, &ppe_parse_result.ppp_tag)) {
                    return 1;
                }
                ppe_parse_result.eth_type = vh->h_vlan_encapsulated_proto;
            } else if (_MDrv_HWNAT_Is_802_1q(vh->h_vlan_encapsulated_proto)) {
                /* VLAN + VLAN + VLAN */
                ppe_parse_result.vlan_layer++;
                vh = (struct vlan_hdr *)(skb->data + ETH_HLEN +
                             ppe_parse_result.vlan1_gap + VLAN_HLEN);

                /* VLAN + VLAN + VLAN */
                if (_MDrv_HWNAT_Is_802_1q(vh->h_vlan_encapsulated_proto))
                    ppe_parse_result.vlan_layer++;
            } else {
                /* VLAN + VLAN + IP */
                ppe_parse_result.eth_type = vh->h_vlan_encapsulated_proto;
            }
        } else {
            /* VLAN + IP */
            ppe_parse_result.eth_type = vh->h_vlan_encapsulated_proto;
        }
    } else if (ntohs(ppe_parse_result.eth_type) == ETH_P_PPP_SES) {
        /* PPPoE + IP */
        ppe_parse_result.pppoe_gap = 8;
        if (_MDrv_HWNAT_Get_Pppoe_Sid(skb, ppe_parse_result.vlan1_gap,
                  &ppe_parse_result.pppoe_sid,
                  &ppe_parse_result.ppp_tag)) {
            return 1;
        }
    }
    /* set layer2 start addr */

    skb_set_mac_header(skb, 0);

    /* set layer3 start addr */
    skb_set_network_header(skb, ETH_HLEN + ppe_parse_result.vlan1_gap +
                   ppe_parse_result.vlan2_gap + ppe_parse_result.pppoe_gap);

    /* set layer4 start addr */
    if ((ppe_parse_result.eth_type == htons(ETH_P_IP)) ||
        (ppe_parse_result.eth_type == htons(ETH_P_PPP_SES) &&
        (ppe_parse_result.ppp_tag == htons(PPP_IP)))) {
        iph = (struct iphdr *)skb_network_header(skb);
        memcpy(&ppe_parse_result.iph, iph, sizeof(struct iphdr));

        if (iph->protocol == IPPROTO_TCP) {
            skb_set_transport_header(skb, ETH_HLEN + ppe_parse_result.vlan1_gap +
                         ppe_parse_result.vlan2_gap +
                         ppe_parse_result.pppoe_gap + (iph->ihl * 4));
            th = (struct tcphdr *)skb_transport_header(skb);

            memcpy(&ppe_parse_result.th, th, sizeof(struct tcphdr));
            ppe_parse_result.pkt_type = IPV4_HNAPT;
            if (iph->frag_off & htons(IP_MF | IP_OFFSET))
                return 1;
        }
        else if (iph->protocol == IPPROTO_UDP) {
            skb_set_transport_header(skb, ETH_HLEN + ppe_parse_result.vlan1_gap +
                         ppe_parse_result.vlan2_gap +
                         ppe_parse_result.pppoe_gap + (iph->ihl * 4));
            uh = (struct udphdr *)skb_transport_header(skb);
            memcpy(&ppe_parse_result.uh, uh, sizeof(struct udphdr));
            ppe_parse_result.pkt_type = IPV4_HNAPT;
            if (iph->frag_off & htons(IP_MF | IP_OFFSET))
            {
                if (USE_3T_UDP_FRAG == 0)
                    return 1;
            }
        }
        else if (iph->protocol == IPPROTO_GRE) {
            if ((_hwnat_info.features & PPTP_L2TP) == 0) {
                /* do nothing */
                return 1;
            }
        }
        else if (iph->protocol == IPPROTO_IPV6) {
            ip6h = (struct ipv6hdr *)((uint8_t *)iph + iph->ihl * 4);
            memcpy(&ppe_parse_result.ip6h, ip6h, sizeof(struct ipv6hdr));

            if (ip6h->nexthdr == NEXTHDR_TCP) {
                skb_set_transport_header(skb, ETH_HLEN + ppe_parse_result.vlan1_gap +
                             ppe_parse_result.vlan2_gap +
                             ppe_parse_result.pppoe_gap +
                             (sizeof(struct ipv6hdr)));

                th = (struct tcphdr *)skb_transport_header(skb);

                memcpy(&ppe_parse_result.th.source, &th->source, sizeof(th->source));
                memcpy(&ppe_parse_result.th.dest, &th->dest, sizeof(th->dest));
            }
            else if (ip6h->nexthdr == NEXTHDR_UDP) {
                skb_set_transport_header(skb, ETH_HLEN + ppe_parse_result.vlan1_gap +
                             ppe_parse_result.vlan2_gap +
                             ppe_parse_result.pppoe_gap +
                             (sizeof(struct ipv6hdr)));

                uh = (struct udphdr *)skb_transport_header(skb);
                memcpy(&ppe_parse_result.uh.source, &uh->source, sizeof(uh->source));
                memcpy(&ppe_parse_result.uh.dest, &uh->dest, sizeof(uh->dest));
            }
            ppe_parse_result.pkt_type = IPV6_6RD;
        }
        else {
            /* Packet format is not supported */
            return 1;
        }

    }
    else if (ppe_parse_result.eth_type == htons(ETH_P_IPV6) ||
           (ppe_parse_result.eth_type == htons(ETH_P_PPP_SES) &&
            ppe_parse_result.ppp_tag == htons(PPP_IPV6))) {
        ip6h = (struct ipv6hdr *)skb_network_header(skb);
        memcpy(&ppe_parse_result.ip6h, ip6h, sizeof(struct ipv6hdr));

        if (ip6h->nexthdr == NEXTHDR_TCP) {
            skb_set_transport_header(skb, ETH_HLEN + ppe_parse_result.vlan1_gap +
                         ppe_parse_result.vlan2_gap +
                         ppe_parse_result.pppoe_gap +
                         (sizeof(struct ipv6hdr)));

            th = (struct tcphdr *)skb_transport_header(skb);
            memcpy(&ppe_parse_result.th, th, sizeof(struct tcphdr));
            ppe_parse_result.pkt_type = IPV6_5T_ROUTE;
        }
        else if (ip6h->nexthdr == NEXTHDR_UDP) {
            skb_set_transport_header(skb, ETH_HLEN + ppe_parse_result.vlan1_gap +
                         ppe_parse_result.vlan2_gap +
                         ppe_parse_result.pppoe_gap +
                         (sizeof(struct ipv6hdr)));
            uh = (struct udphdr *)skb_transport_header(skb);
            memcpy(&ppe_parse_result.uh, uh, sizeof(struct udphdr));
            ppe_parse_result.pkt_type = IPV6_5T_ROUTE;
        }
        else if (ip6h->nexthdr == NEXTHDR_IPIP) {
            ipv6_head_len = sizeof(struct iphdr);
            memcpy(&ppe_parse_result.iph, ip6h + ipv6_head_len, sizeof(struct iphdr));
            ppe_parse_result.pkt_type = IPV4_DSLITE;
        }
        else {
            ppe_parse_result.pkt_type = IPV6_3T_ROUTE;
        }

    }
    else {
        return 1;
    }

    if (_hwnat_info.debug_level >= 6) {
        NAT_PRINT("--------------\n");
        NAT_PRINT("DMAC:%02X:%02X:%02X:%02X:%02X:%02X\n",
             ppe_parse_result.dmac[0], ppe_parse_result.dmac[1],
             ppe_parse_result.dmac[2], ppe_parse_result.dmac[3],
             ppe_parse_result.dmac[4], ppe_parse_result.dmac[5]);
        NAT_PRINT("SMAC:%02X:%02X:%02X:%02X:%02X:%02X\n",
             ppe_parse_result.smac[0], ppe_parse_result.smac[1],
             ppe_parse_result.smac[2], ppe_parse_result.smac[3],
             ppe_parse_result.smac[4], ppe_parse_result.smac[5]);
        NAT_PRINT("Eth_Type=%x\n", ppe_parse_result.eth_type);
        if (ppe_parse_result.vlan1_gap > 0)
            NAT_PRINT("VLAN1 ID=%x\n", ntohs(ppe_parse_result.vlan1));

        if (ppe_parse_result.vlan2_gap > 0)
            NAT_PRINT("VLAN2 ID=%x\n", ntohs(ppe_parse_result.vlan2));

        if (ppe_parse_result.pppoe_gap > 0) {
            NAT_PRINT("PPPOE Session ID=%x\n", ppe_parse_result.pppoe_sid);
            NAT_PRINT("PPP Tag=%x\n", ntohs(ppe_parse_result.ppp_tag));
        }
        NAT_PRINT("PKT_TYPE=%s\n",
             ppe_parse_result.pkt_type ==
             0 ? "IPV4_HNAT" : ppe_parse_result.pkt_type ==
             1 ? "IPV4_HNAPT" : ppe_parse_result.pkt_type ==
             3 ? "IPV4_DSLITE" : ppe_parse_result.pkt_type ==
             5 ? "IPV6_ROUTE" : ppe_parse_result.pkt_type == 7 ? "IPV6_6RD" : "Unknown");
        if (ppe_parse_result.pkt_type == IPV4_HNAT) {
            NAT_PRINT("SIP=%s\n", MDrv_HWNAT_Util_Ip_To_Str(ntohl(ppe_parse_result.iph.saddr)));
            NAT_PRINT("DIP=%s\n", MDrv_HWNAT_Util_Ip_To_Str(ntohl(ppe_parse_result.iph.daddr)));
            NAT_PRINT("TOS=%x\n", ntohs(ppe_parse_result.iph.tos));
        } else if (ppe_parse_result.pkt_type == IPV4_HNAPT) {
            NAT_PRINT("SIP=%s\n", MDrv_HWNAT_Util_Ip_To_Str(ntohl(ppe_parse_result.iph.saddr)));
            NAT_PRINT("DIP=%s\n", MDrv_HWNAT_Util_Ip_To_Str(ntohl(ppe_parse_result.iph.daddr)));
            NAT_PRINT("TOS=%x\n", ntohs(ppe_parse_result.iph.tos));

            if (ppe_parse_result.iph.protocol == IPPROTO_TCP) {
                NAT_PRINT("TCP SPORT=%d, %d\n", ntohs(ppe_parse_result.th.source), ntohs(th->source));
                NAT_PRINT("TCP DPORT=%d, %d\n", ntohs(ppe_parse_result.th.dest), ntohs(th->dest));
            } else if (ppe_parse_result.iph.protocol == IPPROTO_UDP) {
                NAT_PRINT("UDP SPORT=%d, %d\n", ntohs(ppe_parse_result.uh.source), ntohs(uh->source));
                NAT_PRINT("UDP DPORT=%d, %d\n", ntohs(ppe_parse_result.uh.dest), ntohs(uh->dest));
            }
        } else if (ppe_parse_result.pkt_type == IPV6_5T_ROUTE) {
            NAT_PRINT("ING SIPv6->DIPv6: %08X:%08X:%08X:%08X:%d-> %08X:%08X:%08X:%08X:%d\n",
                 ntohl(ppe_parse_result.ip6h.saddr.s6_addr32[0]), ntohl(ppe_parse_result.ip6h.saddr.s6_addr32[1]),
                 ntohl(ppe_parse_result.ip6h.saddr.s6_addr32[2]), ntohl(ppe_parse_result.ip6h.saddr.s6_addr32[3]),
                 ntohs(ppe_parse_result.th.source), ntohl(ppe_parse_result.ip6h.daddr.s6_addr32[0]),
                 ntohl(ppe_parse_result.ip6h.daddr.s6_addr32[1]), ntohl(ppe_parse_result.ip6h.daddr.s6_addr32[2]),
                 ntohl(ppe_parse_result.ip6h.daddr.s6_addr32[3]), ntohs(ppe_parse_result.th.dest));
        } else if (ppe_parse_result.pkt_type == IPV6_6RD) {
            /* fill in ipv4 6rd entry */
            NAT_PRINT("packet_type = IPV6_6RD\n");
            NAT_PRINT("SIP=%s\n", MDrv_HWNAT_Util_Ip_To_Str(ntohl(ppe_parse_result.iph.saddr)));
            NAT_PRINT("DIP=%s\n", MDrv_HWNAT_Util_Ip_To_Str(ntohl(ppe_parse_result.iph.daddr)));

            NAT_PRINT("Checksum=%x\n", ntohs(ppe_parse_result.iph.check));
            NAT_PRINT("ipV4 ID =%x\n", ntohs(ppe_parse_result.iph.id));
            NAT_PRINT("Flag=%x\n", ntohs(ppe_parse_result.iph.frag_off) >> 13);
            NAT_PRINT("TTL=%x\n", ppe_parse_result.iph.ttl);
            NAT_PRINT("TOS=%x\n", ppe_parse_result.iph.tos);
        }
    }

    return 0;
}

static int32_t _MDrv_HWNAT_Fill_L2(struct sk_buff *skb, struct foe_entry *entry)
{
    /* if this entry is already in binding state, skip it */
    if (entry->bfib1.state == BIND) {
        return 1;
    }

    /* Set VLAN Info - VLAN1/VLAN2 */
    /* Set Layer2 Info - DMAC, SMAC */
    if ((ppe_parse_result.pkt_type == IPV4_HNAT) || (ppe_parse_result.pkt_type == IPV4_HNAPT)) {
        if (entry->ipv4_hnapt.bfib1.pkt_type == IPV4_DSLITE) {  /* DS-Lite WAN->LAN */
            if (_hwnat_info.features & NAT_IPV6) {
                MDrv_HWNAT_Foe_Set_High_Mac(entry->ipv4_dslite.dmac_hi, ppe_parse_result.dmac);
                MDrv_HWNAT_Foe_Set_Low_Mac(entry->ipv4_dslite.dmac_lo, ppe_parse_result.dmac);
                MDrv_HWNAT_Foe_Set_High_Mac(entry->ipv4_dslite.smac_hi, ppe_parse_result.smac);
                MDrv_HWNAT_Foe_Set_Low_Mac(entry->ipv4_dslite.smac_lo, ppe_parse_result.smac);
                entry->ipv4_dslite.vlan1 = ntohs(ppe_parse_result.vlan1);
                entry->ipv4_dslite.pppoe_id = ntohs(ppe_parse_result.pppoe_sid);
                if (_hwnat_info.features & HW_NAT_ARCH_WIFI_WDMA)
                    entry->ipv4_dslite.vlan2_winfo = ntohs(ppe_parse_result.vlan2);
                else
                    entry->ipv4_dslite.vlan2 = ntohs(ppe_parse_result.vlan2);
                entry->ipv4_dslite.etype = ntohs(ppe_parse_result.vlan_tag);
            }
            else {
                return 1;
            }
        }
        else {  /* IPv4 WAN<->LAN */
            MDrv_HWNAT_Foe_Set_High_Mac(entry->ipv4_hnapt.dmac_hi, ppe_parse_result.dmac);
            MDrv_HWNAT_Foe_Set_Low_Mac(entry->ipv4_hnapt.dmac_lo, ppe_parse_result.dmac);
            MDrv_HWNAT_Foe_Set_High_Mac(entry->ipv4_hnapt.smac_hi, ppe_parse_result.smac);
            MDrv_HWNAT_Foe_Set_Low_Mac(entry->ipv4_hnapt.smac_lo, ppe_parse_result.smac);
            entry->ipv4_hnapt.vlan1 = ntohs(ppe_parse_result.vlan1);
            entry->ipv4_hnapt.pppoe_id = ntohs(ppe_parse_result.pppoe_sid);
            if (_hwnat_info.features & HW_NAT_ARCH_WIFI_WDMA)
                entry->ipv4_dslite.vlan2_winfo = ntohs(ppe_parse_result.vlan2);
            else
                entry->ipv4_hnapt.vlan2 = ntohs(ppe_parse_result.vlan2);
            entry->ipv4_hnapt.etype = ntohs(ppe_parse_result.vlan_tag);
        }
    }
    else {
        if (_hwnat_info.features & NAT_IPV6) {
            MDrv_HWNAT_Foe_Set_High_Mac(entry->ipv6_5t_route.dmac_hi, ppe_parse_result.dmac);
            MDrv_HWNAT_Foe_Set_Low_Mac(entry->ipv6_5t_route.dmac_lo, ppe_parse_result.dmac);
            MDrv_HWNAT_Foe_Set_High_Mac(entry->ipv6_5t_route.smac_hi, ppe_parse_result.smac);
            MDrv_HWNAT_Foe_Set_Low_Mac(entry->ipv6_5t_route.smac_lo, ppe_parse_result.smac);
            entry->ipv6_5t_route.vlan1 = ntohs(ppe_parse_result.vlan1);
            entry->ipv6_5t_route.pppoe_id = ntohs(ppe_parse_result.pppoe_sid);
            if (_hwnat_info.features & HW_NAT_ARCH_WIFI_WDMA)
                entry->ipv4_dslite.vlan2_winfo = ntohs(ppe_parse_result.vlan2);
            else
                entry->ipv6_5t_route.vlan2 = ntohs(ppe_parse_result.vlan2);
            entry->ipv6_5t_route.etype = ntohs(ppe_parse_result.vlan_tag);
        }
        else {
            return 1;
        }
    }

/* VLAN Layer:*/
/* 0: outgoing packet is untagged packet*/
/* 1: outgoing packet is tagged packet*/
/* 2: outgoing packet is double tagged packet*/
/* 3: outgoing packet is triple tagged packet*/
/* 4: outgoing packet is fourfold tagged packet*/
    entry->bfib1.vlan_layer = ppe_parse_result.vlan_layer;

    if (ppe_parse_result.pppoe_gap)
        entry->bfib1.psn = 1;
    else
        entry->bfib1.psn = 0;
    entry->ipv4_hnapt.bfib1.vpm = 1;    /* 0x8100 */
    return 0;
}

static uint16_t _MDrv_HWNAT_Get_Chkbase(struct iphdr *iph)
{
    u16 org_chksum = ntohs(iph->check);
    u16 org_tot_len = ntohs(iph->tot_len);
    u16 org_id = ntohs(iph->id);
    u16 chksum_tmp, tot_len_tmp, id_tmp;
    u32 tmp = 0;
    u16 chksum_base = 0;

    chksum_tmp = ~(org_chksum);
    tot_len_tmp = ~(org_tot_len);
    id_tmp = ~(org_id);
    tmp = chksum_tmp + tot_len_tmp + id_tmp;
    tmp = ((tmp >> 16) & 0x7) + (tmp & 0xFFFF);
    tmp = ((tmp >> 16) & 0x7) + (tmp & 0xFFFF);
    chksum_base = tmp & 0xFFFF;

    return chksum_base;
}

static int32_t _MDrv_HWNAT_Fill_L3(struct sk_buff *skb, struct foe_entry *entry)
{
    /* IPv4 or IPv4 over PPPoE */
    if ((ppe_parse_result.eth_type == htons(ETH_P_IP)) ||
        (ppe_parse_result.eth_type == htons(ETH_P_PPP_SES) &&
         ppe_parse_result.ppp_tag == htons(PPP_IP))) {
        if ((ppe_parse_result.pkt_type == IPV4_HNAT) ||
            (ppe_parse_result.pkt_type == IPV4_HNAPT)) {
            if (entry->ipv4_hnapt.bfib1.pkt_type == IPV4_DSLITE) {  /* DS-Lite WAN->LAN */
                if (_hwnat_info.features & NAT_IPV6) {
                    if (_hwnat_info.features & PPE_MIB) {
                        entry->ipv4_dslite.iblk2.mibf = 1;
                    }
                    entry->ipv4_dslite.bfib1.rmt = 1;   /* remove outer IPv6 header */
                    entry->ipv4_dslite.iblk2.dscp = ppe_parse_result.iph.tos;
                }

            } else {

                entry->ipv4_hnapt.new_sip = ntohl(ppe_parse_result.iph.saddr);
                entry->ipv4_hnapt.new_dip = ntohl(ppe_parse_result.iph.daddr);
                entry->ipv4_hnapt.iblk2.dscp = ppe_parse_result.iph.tos;
                if (_hwnat_info.features & PPE_MIB) {
                    entry->ipv4_dslite.iblk2.mibf = 1;
                }
            }
        }
        else if (ppe_parse_result.pkt_type == IPV6_6RD) {
            /* fill in ipv4 6rd entry */
            entry->ipv6_6rd.tunnel_sipv4 = ntohl(ppe_parse_result.iph.saddr);
            entry->ipv6_6rd.tunnel_dipv4 = ntohl(ppe_parse_result.iph.daddr);
            entry->ipv6_6rd.hdr_chksum = _MDrv_HWNAT_Get_Chkbase(&ppe_parse_result.iph);
            entry->ipv6_6rd.flag = (ntohs(ppe_parse_result.iph.frag_off) >> 13);
            entry->ipv6_6rd.ttl = ppe_parse_result.iph.ttl;
            entry->ipv6_6rd.dscp = ppe_parse_result.iph.tos;
            if (_hwnat_info.features & PPE_MIB) {
                entry->ipv4_dslite.iblk2.mibf = 1;
            }
            MHal_HWNAT_Set_Hash_Seed(ntohs(ppe_parse_result.iph.id));
            entry->ipv6_6rd.per_flow_6rd_id = 1;
            /* IPv4 DS-Lite and IPv6 6RD shall be turn on by SW during initialization */
            entry->bfib1.pkt_type = IPV6_6RD;
        }
    }
    /* IPv6 or IPv6 over PPPoE */
    else if ((_hwnat_info.features & NAT_IPV6) && (ppe_parse_result.eth_type == htons(ETH_P_IPV6) ||
         (ppe_parse_result.eth_type == htons(ETH_P_PPP_SES) &&
          ppe_parse_result.ppp_tag == htons(PPP_IPV6)))) {
        if (ppe_parse_result.pkt_type == IPV6_3T_ROUTE ||
            ppe_parse_result.pkt_type == IPV6_5T_ROUTE) {
            /* incoming packet is 6RD and need to remove outer IPv4 header */
            if (entry->bfib1.pkt_type == IPV6_6RD) {
                entry->ipv6_3t_route.bfib1.rmt = 1;
                entry->ipv6_3t_route.iblk2.dscp = (ppe_parse_result.ip6h.priority << 4 | (ppe_parse_result.ip6h.flow_lbl[0] >> 4));
                if (_hwnat_info.features & PPE_MIB) {
                    entry->ipv4_dslite.iblk2.mibf = 1;
                }

            } else {
                /* fill in ipv6 routing entry */
                entry->ipv6_3t_route.ipv6_sip0 = ntohl(ppe_parse_result.ip6h.saddr.s6_addr32[0]);
                entry->ipv6_3t_route.ipv6_sip1 = ntohl(ppe_parse_result.ip6h.saddr.s6_addr32[1]);
                entry->ipv6_3t_route.ipv6_sip2 = ntohl(ppe_parse_result.ip6h.saddr.s6_addr32[2]);
                entry->ipv6_3t_route.ipv6_sip3 = ntohl(ppe_parse_result.ip6h.saddr.s6_addr32[3]);

                entry->ipv6_3t_route.ipv6_dip0 = ntohl(ppe_parse_result.ip6h.daddr.s6_addr32[0]);
                entry->ipv6_3t_route.ipv6_dip1 = ntohl(ppe_parse_result.ip6h.daddr.s6_addr32[1]);
                entry->ipv6_3t_route.ipv6_dip2 = ntohl(ppe_parse_result.ip6h.daddr.s6_addr32[2]);
                entry->ipv6_3t_route.ipv6_dip3 = ntohl(ppe_parse_result.ip6h.daddr.s6_addr32[3]);
                entry->ipv6_3t_route.iblk2.dscp = (ppe_parse_result.ip6h.priority << 4 | (ppe_parse_result.ip6h.flow_lbl[0] >> 4));
                if (_hwnat_info.features & PPE_MIB) {
                    entry->ipv4_dslite.iblk2.mibf = 1;
                }
            }

        } else if (ppe_parse_result.pkt_type == IPV4_DSLITE) {
            /* fill in DSLite entry */
            entry->ipv4_dslite.tunnel_sipv6_0 = ntohl(ppe_parse_result.ip6h.saddr.s6_addr32[0]);
            entry->ipv4_dslite.tunnel_sipv6_1 = ntohl(ppe_parse_result.ip6h.saddr.s6_addr32[1]);
            entry->ipv4_dslite.tunnel_sipv6_2 = ntohl(ppe_parse_result.ip6h.saddr.s6_addr32[2]);
            entry->ipv4_dslite.tunnel_sipv6_3 = ntohl(ppe_parse_result.ip6h.saddr.s6_addr32[3]);

            entry->ipv4_dslite.tunnel_dipv6_0 = ntohl(ppe_parse_result.ip6h.daddr.s6_addr32[0]);
            entry->ipv4_dslite.tunnel_dipv6_1 = ntohl(ppe_parse_result.ip6h.daddr.s6_addr32[1]);
            entry->ipv4_dslite.tunnel_dipv6_2 = ntohl(ppe_parse_result.ip6h.daddr.s6_addr32[2]);
            entry->ipv4_dslite.tunnel_dipv6_3 = ntohl(ppe_parse_result.ip6h.daddr.s6_addr32[3]);
            if (_hwnat_info.features & PPE_MIB) {
                entry->ipv4_dslite.iblk2.mibf = 1;
            }
            memcpy(entry->ipv4_dslite.flow_lbl, ppe_parse_result.ip6h.flow_lbl, sizeof(ppe_parse_result.ip6h.flow_lbl));
            entry->ipv4_dslite.priority = ppe_parse_result.ip6h.priority;
            entry->ipv4_dslite.hop_limit = ppe_parse_result.ip6h.hop_limit;
            /* IPv4 DS-Lite and IPv6 6RD shall be turn on by SW during initialization */
            entry->bfib1.pkt_type = IPV4_DSLITE;
        };
    }
    else
        return 1;

    return 0;
}

static int32_t _MDrv_HWNAT_Fill_L4(struct sk_buff *skb, struct foe_entry *entry)
{
    if (ppe_parse_result.pkt_type == IPV4_HNAPT) {
        /* DS-LIte WAN->LAN */
        if (entry->ipv4_hnapt.bfib1.pkt_type == IPV4_DSLITE)
            return 0;
        /* Set Layer4 Info - NEW_SPORT, NEW_DPORT */
        if (ppe_parse_result.iph.protocol == IPPROTO_TCP) {
            entry->ipv4_hnapt.new_sport = ntohs(ppe_parse_result.th.source);
            entry->ipv4_hnapt.new_dport = ntohs(ppe_parse_result.th.dest);
            entry->ipv4_hnapt.bfib1.udp = TCP;
        } else if (ppe_parse_result.iph.protocol == IPPROTO_UDP) {
            entry->ipv4_hnapt.new_sport = ntohs(ppe_parse_result.uh.source);
            entry->ipv4_hnapt.new_dport = ntohs(ppe_parse_result.uh.dest);
            entry->ipv4_hnapt.bfib1.udp = UDP;

            /* if UDP checksum is zero, it cannot be accelerated by HNAT */
            /* we found some protocols, such as IPSEC-NAT-T, are possible to hybrid
             * udp zero checksum and non-zero checksum in the same session,
             * so we disable HNAT acceleration for all UDP flows
             */
            /* if(entry->ipv4_hnapt.new_sport==4500 && entry->ipv4_hnapt.new_dport==4500) */
        }
    }
    else if (ppe_parse_result.pkt_type == IPV4_HNAT) {
        /* do nothing */
    }
    else if (ppe_parse_result.pkt_type == IPV6_1T_ROUTE) {
        /* do nothing */
    }
    else if ((_hwnat_info.features & NAT_IPV6) && (ppe_parse_result.pkt_type == IPV6_3T_ROUTE)) {
        /* do nothing */
    }
    else if ((_hwnat_info.features & NAT_IPV6) && (ppe_parse_result.pkt_type == IPV6_5T_ROUTE)) {
        /* do nothing */
    }

    return 0;
}

static void _MDrv_HWNAT_Set_InfoBlock2(struct _info_blk2 *iblk2, uint32_t fpidx, uint32_t port_mg, uint32_t port_ag)
{
    /* we need to lookup another multicast table if this is multicast flow */
    if (ppe_parse_result.is_mcast) {
        iblk2->mcast = 1;
        if (_hwnat_info.features & HW_NAT_ARCH_WIFI_WDMA) {
            if (fpidx == 3)
                fpidx = 0;  /* multicast flow not go to WDMA */
        }
    }
    else {
        iblk2->mcast = 0;
    }

    /* 0:PSE,1:GSW, 2:GMAC,4:PPE,5:QDMA,7=DROP */
    iblk2->dp = fpidx;

    if ((_hwnat_info.features &  HW_NAT_QDMA) == 0)
        iblk2->fqos = 0;    /* PDMA MODE should not goes to QoS */

    iblk2->acnt = port_ag;
}



static void _MDrv_HWNAT_Set_Blk2_Qid(struct sk_buff *skb, struct foe_entry *entry, int gmac_no)
{
    unsigned int qidx;
    if (IS_IPV4_GRP(entry)) {
        if (skb->mark > 63)
            skb->mark = 0;
        qidx = M2Q_table[skb->mark];
        entry->ipv4_hnapt.iblk2.qid1 = ((qidx & 0x30) >> 4);
        entry->ipv4_hnapt.iblk2.qid = (qidx & 0x0f);
        if (_hwnat_info.features & HW_NAT_PSEUDO_SUPPORT) {
            if (lan_wan_separate == 1 && gmac_no == 2) {
                entry->ipv4_hnapt.iblk2.qid += 8;
                if (_hwnat_info.features & HW_NAT_HW_SFQ) {
                    if (web_sfq_enable == 1 && (skb->mark == 2))
                        entry->ipv4_hnapt.iblk2.qid = HWSFQUP;
                }
            }
            if ((lan_wan_separate == 1) && (gmac_no == 1)) {
                if (_hwnat_info.features & HW_NAT_HW_SFQ) {
                    if (web_sfq_enable == 1 && (skb->mark == 2))
                        entry->ipv4_hnapt.iblk2.qid = HWSFQDL;
                }
            }
        }
    }
    else if (IS_IPV6_GRP(entry)) {
        if (_hwnat_info.features & HW_NAT_PSEUDO_SUPPORT) {
            if (skb->mark > 63)
                skb->mark = 0;
            qidx = M2Q_table[skb->mark];
            entry->ipv6_3t_route.iblk2.qid1 = ((qidx & 0x30) >> 4);
            entry->ipv6_3t_route.iblk2.qid = (qidx & 0x0f);
            if (lan_wan_separate == 1 && gmac_no == 2) {
                entry->ipv6_3t_route.iblk2.qid += 8;
                if (_hwnat_info.features & HW_NAT_HW_SFQ) {
                    if (web_sfq_enable == 1 && (skb->mark == 2))
                        entry->ipv6_3t_route.iblk2.qid = HWSFQUP;
                }
            }
            if ((lan_wan_separate == 1) && (gmac_no == 1)) {
                if (_hwnat_info.features & HW_NAT_HW_SFQ) {
                    if (web_sfq_enable == 1 && (skb->mark == 2))
                        entry->ipv6_3t_route.iblk2.qid = HWSFQDL;
                }
            }
        }
    }
}

int32_t MDrv_HWNAT_Set_Force_Port(struct sk_buff *skb, struct foe_entry *entry, int gmac_no)
{
    /* Set force port info */
    if (_hwnat_info.features &  HW_NAT_QDMA) {
        _MDrv_HWNAT_Set_Blk2_Qid(skb, entry, gmac_no);
    }

    /* CPU need to handle traffic between WLAN/PCI and GMAC port */
    if ((strncmp(skb->dev->name, "eth", 3) != 0)) {
        if (((_hwnat_info.features & HW_NAT_WIFI) == 0) &&
            ((_hwnat_info.features & HW_NAT_NIC_USB) == 0) &&
            ((_hwnat_info.features & PPTP_L2TP) == 0)) {
            return 1;
        }

        /*PPTP/L2TP LAN->WAN  bind to CPU*/
        if (IS_IPV4_GRP(entry)) {
            if (_hwnat_info.features &  HW_NAT_QDMA) {
                entry->ipv4_hnapt.bfib1.vpm = 0;    /* etype remark */

                if (_hwnat_info.features & HW_NAT_ARCH_WIFI_WDMA) {
                    if (gmac_no == 3) {
                        entry->ipv4_hnapt.iblk2.fqos = 0;/*wifi hw_nat not support QoS */
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv4_hnapt.iblk2, 3, 0x3F, 0x3F);    /* 3=WDMA */
                    }
                    else {
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv4_hnapt.iblk2, 0, 0x3F, 0x3F); /* 0=PDMA */
                        if ((_hwnat_info.features & WAN_TO_WLAN_SUPPORT_QOS) && (MHal_HWNAT_Get_QoS_Status() == TRUE))
                            entry->ipv4_hnapt.iblk2.fqos = 1;
                        else
                            entry->ipv4_hnapt.iblk2.fqos = 0;
                    }
                }
                else {
                    if (_hwnat_info.features & HW_NAT_QDMATX_QDMARX)
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv4_hnapt.iblk2, 5, 0x3F, 0x3F);    /* 5=QDMA */
                    else
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv4_hnapt.iblk2, 0, 0x3F, 0x3F);    /* 0=PDMA */

                    if (FOE_SP(skb) == 5)    /* wifi to wifi not go to pse port6 */
                        entry->ipv4_hnapt.iblk2.fqos = 0;
                    else {
                        if ((_hwnat_info.features & WAN_TO_WLAN_SUPPORT_QOS) && (MHal_HWNAT_Get_QoS_Status() == TRUE))
                            entry->ipv4_hnapt.iblk2.fqos = 1;
                        else
                            entry->ipv4_hnapt.iblk2.fqos = 0;
                    }

                    if (_hwnat_info.features & HW_NAT_ARCH_WIFI_WDMA) {
                        if (gmac_no == 3) {
                            entry->ipv4_hnapt.iblk2.wdmaid = (FOE_WDMA_ID(skb) & 0x01);
                            entry->ipv4_hnapt.iblk2.winfo = 1;
                            entry->ipv4_hnapt.vlan2_winfo =
                                ((FOE_RX_ID(skb) & 0x03) << 14) | ((FOE_WC_ID(skb) & 0xff) << 6) |
                                (FOE_BSS_ID(skb) & 0x3f);
                        }else {
                            if (ppe_parse_result.vlan1 == 0) {
                                entry->ipv4_hnapt.vlan1 = FOE_ENTRY_NUM(skb);
                                entry->ipv4_hnapt.etype = ntohs(0x5678);
                                entry->bfib1.vlan_layer = 1;
                            }else if (ppe_parse_result.vlan2 == 0) {
                                entry->ipv4_hnapt.vlan1 = FOE_ENTRY_NUM(skb);
                                entry->ipv4_hnapt.etype = ntohs(0x5678);
                                entry->ipv4_hnapt.vlan2_winfo = ntohs(ppe_parse_result.vlan1);
                                entry->bfib1.vlan_layer = 2;
                            } else {
                                entry->ipv4_hnapt.vlan1 = FOE_ENTRY_NUM(skb);
                                entry->ipv4_hnapt.etype = ntohs(0x5678);
                                entry->ipv4_hnapt.vlan2_winfo = ntohs(ppe_parse_result.vlan1);
                                entry->bfib1.vlan_layer = 3;
                            }
                        }
                    }
                    else {
                        if (ppe_parse_result.vlan1 == 0) {
                            entry->ipv4_hnapt.vlan1 = FOE_ENTRY_NUM(skb);
                            entry->ipv4_hnapt.etype = ntohs(0x5678);
                            entry->bfib1.vlan_layer = 1;
                        }else if (ppe_parse_result.vlan2 == 0) {
                            entry->ipv4_hnapt.vlan1 = FOE_ENTRY_NUM(skb);
                            entry->ipv4_hnapt.etype = ntohs(0x5678);
                            entry->ipv4_hnapt.vlan2 = ntohs(ppe_parse_result.vlan1);
                            entry->bfib1.vlan_layer = 2;
                        } else {
                            entry->ipv4_hnapt.vlan1 = FOE_ENTRY_NUM(skb);
                            entry->ipv4_hnapt.etype = ntohs(0x5678);
                            entry->ipv4_hnapt.vlan2 = ntohs(ppe_parse_result.vlan1);
                            entry->bfib1.vlan_layer = 3;
                        }
                    }
                }
            }
            else {
                if (gmac_no == 3) {
                    _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv4_hnapt.iblk2, 3, 0x3F, 0x3F);    /* 3=WDMA */
                    entry->ipv4_hnapt.iblk2.fqos = 0;
                    entry->ipv4_hnapt.iblk2.wdmaid = (FOE_WDMA_ID(skb) & 0x01);
                    entry->ipv4_hnapt.iblk2.winfo = 1;
                    entry->ipv4_hnapt.vlan2_winfo =
                        ((FOE_RX_ID(skb) & 0x03) << 14) | ((FOE_WC_ID(skb) & 0xff) << 6) |
                        (FOE_BSS_ID(skb) & 0x3f);
                }
                else {
                    _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv4_hnapt.iblk2, 0, 0x3F, 0x3F); /* 0=CPU fastpath*/
                }
                entry->ipv4_hnapt.iblk2.fqos = 0;   /* PDMA MODE should not goes to QoS */
            }
        }
        else if (IS_IPV6_GRP(entry)) {
            if (_hwnat_info.features &  HW_NAT_QDMA) {
                if (_hwnat_info.features & HW_NAT_QDMATX_QDMARX) {
                    if (_hwnat_info.features & HW_NAT_ARCH_WIFI_WDMA) {
                        if (gmac_no == 3) {
                            _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv6_3t_route.iblk2, 3, 0x3F, 0x3F); /* 3=WDMA */
                            entry->ipv6_3t_route.iblk2.fqos = 0; /* wifi hw_nat not support qos */
                        }
                        else
                            _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv6_3t_route.iblk2, 5, 0x3F, 0x3F);/* 0=CPU fastpath */
                    }
                    else {
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv6_3t_route.iblk2, 5, 0x3F, 0x3F);/* 0=CPU fastpath */
                        if (FOE_SP(skb) == 5) {
                            entry->ipv6_3t_route.iblk2.fqos = 0;    /* wifi to wifi not go to pse port6 */
                        }
                        else {
                            if ((_hwnat_info.features & WAN_TO_WLAN_SUPPORT_QOS) && (MHal_HWNAT_Get_QoS_Status() == TRUE))
                                entry->ipv6_3t_route.iblk2.fqos = 1;
                            else
                                entry->ipv6_3t_route.iblk2.fqos = 0;
                        }

                    }

                }
                else {
                    if (_hwnat_info.features & HW_NAT_ARCH_WIFI_WDMA) {
                        if (gmac_no == 3) {
                            _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv6_3t_route.iblk2, 3, 0x3F, 0x3F);/* 3=WDMA */
                            entry->ipv6_3t_route.iblk2.fqos = 0;    /* wifi hw_nat not support qos */
                        }
                        else {
                            _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv6_3t_route.iblk2, 0, 0x3F, 0x3F);
                            if ((_hwnat_info.features & WAN_TO_WLAN_SUPPORT_QOS) && (MHal_HWNAT_Get_QoS_Status() == TRUE))
                                entry->ipv6_3t_route.iblk2.fqos = 1;
                            else
                                entry->ipv6_3t_route.iblk2.fqos = 0;

                        }
                    }
                    else {
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv6_3t_route.iblk2, 0, 0x3F, 0x3F); /* 0=PDMA */
                        if (FOE_SP(skb) == 5) {
                            entry->ipv6_3t_route.iblk2.fqos = 0;    /* wifi to wifi not go to pse port6 */
                        }
                        else {
                            if ((_hwnat_info.features & WAN_TO_WLAN_SUPPORT_QOS) && (MHal_HWNAT_Get_QoS_Status() == TRUE))
                                entry->ipv6_3t_route.iblk2.fqos = 1;
                            else
                                entry->ipv6_3t_route.iblk2.fqos = 0;
                        }
                    }
                }

                if (_hwnat_info.features & HW_NAT_ARCH_WIFI_WDMA) {
                    if (gmac_no == 3) {
                        entry->ipv6_3t_route.iblk2.wdmaid = (FOE_WDMA_ID(skb) & 0x01);
                        entry->ipv6_3t_route.iblk2.winfo = 1;
                        entry->ipv6_3t_route.vlan2_winfo =
                            ((FOE_RX_ID(skb) & 0x03) << 14) | ((FOE_WC_ID(skb) & 0xff) << 6) |
                            (FOE_BSS_ID(skb) & 0x3f);
                    }else {
                        if (ppe_parse_result.vlan1 == 0) {
                            entry->ipv6_3t_route.vlan1 = FOE_ENTRY_NUM(skb);
                            entry->ipv6_3t_route.etype = ntohs(0x5678);
                            entry->bfib1.vlan_layer = 1;
                        }else if (ppe_parse_result.vlan2 == 0) {
                            entry->ipv6_3t_route.vlan1 = FOE_ENTRY_NUM(skb);
                            entry->ipv6_3t_route.etype = ntohs(0x5678);
                            entry->ipv6_3t_route.vlan2_winfo = ntohs(ppe_parse_result.vlan1);
                            entry->bfib1.vlan_layer = 2;
                        } else {
                            entry->ipv6_3t_route.vlan1 = FOE_ENTRY_NUM(skb);
                            entry->ipv6_3t_route.etype = ntohs(0x5678);
                            entry->ipv6_3t_route.vlan2_winfo = ntohs(ppe_parse_result.vlan1);
                            entry->bfib1.vlan_layer = 3;
                        }
                    }
                }
                else {
                    if (ppe_parse_result.vlan1 == 0) {
                        entry->ipv6_3t_route.vlan1 = FOE_ENTRY_NUM(skb);
                        entry->ipv6_3t_route.etype = ntohs(0x5678);
                        entry->bfib1.vlan_layer = 1;
                    }else if (ppe_parse_result.vlan2 == 0) {
                        entry->ipv6_3t_route.vlan1 = FOE_ENTRY_NUM(skb);
                        entry->ipv6_3t_route.etype = ntohs(0x5678);
                        entry->ipv6_3t_route.vlan2 = ntohs(ppe_parse_result.vlan1);
                        entry->bfib1.vlan_layer = 2;
                    } else {
                        entry->ipv6_3t_route.vlan1 = FOE_ENTRY_NUM(skb);
                        entry->ipv6_3t_route.etype = ntohs(0x5678);
                        entry->ipv6_3t_route.vlan2 = ntohs(ppe_parse_result.vlan1);
                        entry->bfib1.vlan_layer = 3;
                    }
                }
            }
            else {

                if (_hwnat_info.features & HW_NAT_ARCH_WIFI_WDMA) {
                    if (gmac_no == 3) {
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv6_3t_route.iblk2, 3, 0x3F, 0x3F);/* 3=WDMA */
                        entry->ipv6_3t_route.iblk2.wdmaid = (FOE_WDMA_ID(skb) & 0x01);
                        entry->ipv6_3t_route.iblk2.winfo = 1;
                        entry->ipv6_3t_route.vlan2_winfo =
                            ((FOE_RX_ID(skb) & 0x03) << 14) | ((FOE_WC_ID(skb) & 0xff) << 6) |
                            (FOE_BSS_ID(skb) & 0x3f);
                    } else
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv6_3t_route.iblk2, 0, 0x3F, 0x3F); /* 0=cpu fastpath */
                }
                else
                    _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv6_3t_route.iblk2, 0, 0x3F, 0x3F); /* 0=cpu fastpath */
            }
        }

    }
    else {
        if (_hwnat_info.features &  HW_NAT_QDMA) {
            if (IS_IPV4_GRP(entry)) {
                if (((FOE_MAGIC_TAG(skb) == FOE_MAGIC_PCI) ||
                    (FOE_MAGIC_TAG(skb) == FOE_MAGIC_WLAN))) {
                    if ((_hwnat_info.features & WAN_TO_WLAN_SUPPORT_QOS) && (MHal_HWNAT_Get_QoS_Status() == TRUE))
                        entry->ipv4_hnapt.iblk2.fqos = 1;
                    else
                        entry->ipv4_hnapt.iblk2.fqos = 0;

                }
                else {
                    if (FOE_SP(skb) == 5) {
                        entry->ipv4_hnapt.iblk2.fqos = 0;
                    }
                    else {

                        if ((_hwnat_info.features & QDMA_SUPPORT_QOS) && (MHal_HWNAT_Get_QoS_Status() == TRUE)) {
                            entry->ipv4_hnapt.iblk2.fqos = 1;
                        }
                        else {
                            entry->ipv4_hnapt.iblk2.fqos = 0;
                        }
                    }
                }
            }
            else if (IS_IPV6_GRP(entry)) {
                if (((FOE_MAGIC_TAG(skb) == FOE_MAGIC_PCI) ||
                    (FOE_MAGIC_TAG(skb) == FOE_MAGIC_WLAN))) {
                    if ((_hwnat_info.features & WAN_TO_WLAN_SUPPORT_QOS) && (MHal_HWNAT_Get_QoS_Status() == TRUE))
                        entry->ipv6_5t_route.iblk2.fqos = 1;
                    else
                        entry->ipv6_5t_route.iblk2.fqos = 0;

                }
                else {
                    if (FOE_SP(skb) == 5) {
                        entry->ipv6_5t_route.iblk2.fqos = 0;
                    }
                    else {
                        if ((_hwnat_info.features & QDMA_SUPPORT_QOS) && (MHal_HWNAT_Get_QoS_Status() == TRUE)) {
                            entry->ipv6_5t_route.iblk2.fqos = 1;
                        }
                        else {
                            entry->ipv6_5t_route.iblk2.fqos = 0;
                        }
                    }
                }
            }
        }

        if (_hwnat_info.features & HW_NAT_PSEUDO_SUPPORT) {
            if (gmac_no == 1) {
                if ((_hwnat_info.bind_dir == DOWNSTREAM_ONLY) || (_hwnat_info.bind_dir == BIDIRECTION)) {
                    if (IS_IPV4_GRP(entry))
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv4_hnapt.iblk2, 1, 0x3F, 1);
                    else if (IS_IPV6_GRP(entry))
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv6_5t_route.iblk2, 1, 0x3F, 1);
                }
                else {
                    return 1;
                }
            }
            else if (gmac_no == 2) {
                if ((_hwnat_info.bind_dir == UPSTREAM_ONLY) || (_hwnat_info.bind_dir == BIDIRECTION)) {
                    if (IS_IPV4_GRP(entry))
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv4_hnapt.iblk2, 2, 0x3F, 2);
                    else if (IS_IPV6_GRP(entry))
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv6_5t_route.iblk2, 2, 0x3F, 2);
                }
                else {
                    return 1;
                }
            }
        }
        else {
            if (IS_IPV4_GRP(entry)) {
                if ((entry->ipv4_hnapt.vlan1 & VLAN_VID_MASK) == _hwnat_info.lan_vid) {
                    if ((_hwnat_info.bind_dir == DOWNSTREAM_ONLY) || (_hwnat_info.bind_dir == BIDIRECTION)) {
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv4_hnapt.iblk2, 1, 0x3F, 1);
                    } else {
                        return 1;
                    }
                } else if ((entry->ipv4_hnapt.vlan1 & VLAN_VID_MASK) == _hwnat_info.wan_vid) {
                    if ((_hwnat_info.bind_dir == UPSTREAM_ONLY) || (_hwnat_info.bind_dir == BIDIRECTION)) {
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv4_hnapt.iblk2, 1, 0x3F, 2);
                    } else {
                        return 1;
                    }
                } else {    /* one-arm */
                    _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv4_hnapt.iblk2, 1, 0x3F, 1);
                }
            }
            else if (IS_IPV6_GRP(entry)) {
                if ((entry->ipv6_5t_route.vlan1 & VLAN_VID_MASK) == _hwnat_info.lan_vid) {
                    if ((_hwnat_info.bind_dir == DOWNSTREAM_ONLY) || (_hwnat_info.bind_dir == BIDIRECTION)) {
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv6_5t_route.iblk2, 1, 0x3F, 1);
                    }
                    else {
                        return 1;
                    }
                }
                else if ((entry->ipv6_5t_route.vlan1 & VLAN_VID_MASK) == _hwnat_info.wan_vid) {
                    if ((_hwnat_info.bind_dir == UPSTREAM_ONLY) || (_hwnat_info.bind_dir == BIDIRECTION)) {
                        _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv6_5t_route.iblk2, 1, 0x3F, 2);
                    }
                    else {
                        return 1;
                    }
                }
                else {  /* one-arm */
                    _MDrv_HWNAT_Set_InfoBlock2(&entry->ipv6_5t_route.iblk2, 1, 0x3F, 1);
                }
            }
        }
    }

    return 0;
}

static MS_BOOL _MDrv_HWNAT_Get_Extif_Offset(struct sk_buff *skb, u32 *retoff)
{
    u32 offset = 0;

    *retoff = 0;
    /* Set actual output port info */
    if (IS_SPECIAL_DEV(_hwnat_info.features) && (strncmp(skb->dev->name, "rai", 3) == 0)) {
        if (_hwnat_info.features & HW_NAT_AP_MESH_SUPPORT) {
            if (HWNAT_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_MESH)
                offset = (HWNAT_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_MESH + DP_MESHI0);
            else
                offset = HWNAT_GET_PACKET_IF(skb) + DP_RAI0;
        }

        if (_hwnat_info.features & HW_NAT_APCLI_SUPPORT) {
            if (HWNAT_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_APCLI)
                offset = (HWNAT_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_APCLI + DP_APCLII0);
            else
                offset = HWNAT_GET_PACKET_IF(skb) + DP_RAI0;
        }
        if (_hwnat_info.features & HW_NAT_WDS_SUPPORT) {
            if (HWNAT_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_WDS)
                offset = (HWNAT_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_WDS + DP_WDSI0);
            else
                offset = HWNAT_GET_PACKET_IF(skb) + DP_RAI0;
        }

    }
    else if (strncmp(skb->dev->name, "ra", 2) == 0) {
        if (_hwnat_info.features & HW_NAT_AP_MESH_SUPPORT) {
            if (HWNAT_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_MESH)
                offset = (HWNAT_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_MESH + DP_MESH0);
            else
                offset = HWNAT_GET_PACKET_IF(skb) + DP_RA0;
        }
        if (_hwnat_info.features & HW_NAT_APCLI_SUPPORT) {
            if (HWNAT_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_APCLI)
                offset = (HWNAT_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_APCLI + DP_APCLI0);
            else
                offset = HWNAT_GET_PACKET_IF(skb) + DP_RA0;
        }
        if (_hwnat_info.features & HW_NAT_WDS_SUPPORT) {
            if (HWNAT_GET_PACKET_IF(skb) >= MIN_NET_DEVICE_FOR_WDS)
                offset = (HWNAT_GET_PACKET_IF(skb) - MIN_NET_DEVICE_FOR_WDS + DP_WDS0);
            else
                offset = HWNAT_GET_PACKET_IF(skb) + DP_RA0;
        }
    }
    else if (_hwnat_info.features & HW_NAT_NIC_USB) {
        if (strncmp(skb->dev->name, "eth0", 4) == 0)
            offset = DP_PCI;
        else if (strncmp(skb->dev->name, "eth1", 4) == 0)
            offset = DP_USB;
    }
    else if (strncmp(skb->dev->name, "eth2", 4) == 0)
        offset = DP_GMAC;
    else if ((_hwnat_info.features & HW_NAT_PSEUDO_SUPPORT) && (strncmp(skb->dev->name, "eth3", 4) == 0))
        offset = DP_GMAC2;
    else {
        if (pr_debug_ratelimited())
            NAT_PRINT("HNAT: unknown interface %s\n", skb->dev->name);
        return FALSE;
    }

    *retoff = offset;

    return TRUE;
}


uint32_t _MDrv_HWNAT_Set_Extif_Num(struct sk_buff *skb, struct foe_entry *entry)
{
    u32 offset = 0;

    if (((_hwnat_info.features & HW_NAT_WIFI) == 0) && ((_hwnat_info.features & HW_NAT_NIC_USB) == 0)) {
        return 0;
    }

    if ((_hwnat_info.features & HW_NAT_WIFI_NEW_ARCH) == 0x0) {
        /* Set actual output port info */
        if (_MDrv_HWNAT_Get_Extif_Offset(skb, &offset) == FALSE) {
            return 1;
        }
    }

    if (_hwnat_info.features & HW_NAT_WIFI_NEW_ARCH) {
        int dev_match = -1;

        dev_match = _MDrv_HWNAT_Get_Dev_Handler_Idx(skb->dev);
        if (dev_match < 0) {
            if (_hwnat_info.debug_level >= 1)
                NAT_PRINT("UnKnown Interface %s \n", skb->dev->name);
            return 1;
        }
        else {
            offset = dev_match;
        }
    }

    if (IS_IPV4_HNAT(entry) || IS_IPV4_HNAPT(entry))
        entry->ipv4_hnapt.act_dp = offset;
    else if (IS_IPV4_DSLITE(entry)) {
        entry->ipv4_dslite.act_dp = offset;
    }
    else if (IS_IPV6_3T_ROUTE(entry)) {
        entry->ipv6_3t_route.act_dp = offset;
    }
    else if (IS_IPV6_5T_ROUTE(entry)) {
        entry->ipv6_5t_route.act_dp = offset;
    }
    else if (IS_IPV6_6RD(entry)) {
        entry->ipv6_6rd.act_dp = offset;
    }
    else {
        return 1;
    }
    return 0;
}

void _MDrv_HWNAT_Set_Entry_Bind(struct sk_buff *skb, struct foe_entry *entry)
{
    u32 current_time;
    /* Set Current time to time_stamp field in information block 1 */
    current_time = MHAL_HWNAT_Get_TimeStamp() & 0xFFFF;
    entry->bfib1.time_stamp = (uint16_t)current_time;

    /* Ipv4: TTL / Ipv6: Hot Limit filed */
    entry->ipv4_hnapt.bfib1.ttl = DFL_FOE_TTL_REGEN;
    /* enable cache by default */
    entry->ipv4_hnapt.bfib1.cah = 1;


    if (_hwnat_info.features & PACKET_SAMPLING) {
        entry->ipv4_hnapt.bfib1.ps = 1;
    }

    if (_hwnat_info.features & NAT_PREBIND) {
        entry->udib1.preb = 1;
    }
    else {
        /* Change Foe Entry State to Binding State */
        entry->bfib1.state = BIND;
        /* Dump Binding Entry */
        if (_hwnat_info.debug_level >= 1) {
            NAT_PRINT(" Bind \n");
            MDrv_HWNAT_Foe_Dump_Entry(FOE_ENTRY_NUM(skb));
        }
    }

    /*make sure write dram correct*/
    wmb();
}

void _MDrv_HWNAT_Regist_Dev_Handler(struct net_device *dev)
{
    int i;
    for (i = 0; i < MAX_IF_NUM; i++) {
        if (dst_port[i] == dev) {
            NAT_PRINT("%s dst_port table has beed registered(%d)\n", dev->name, i);
            return;
        }
        if (dst_port[i] == NULL) {
            dst_port[i] = dev;
            break;
        }
    }
    NAT_PRINT("ineterface %s register (%d)\n", dev->name, i);
}

void _MDrv_HWNAT_Unregist_Dev_Handler(struct net_device *dev)
{
    int i;
    for (i = 0; i < MAX_IF_NUM; i++) {
        if (dst_port[i] == dev) {
            dst_port[i] = NULL;
            break;
        }
    }
    NAT_PRINT("ineterface %s unregister (%d)\n", dev->name, i);
}



int _MDrv_HWNAT_Get_Done_Bit(struct sk_buff *skb, struct foe_entry * entry)
{
    int done_bit = 0;
    if (IS_IPV4_HNAT(entry) || IS_IPV4_HNAPT(entry)) {
        done_bit = entry->ipv4_hnapt.resv1;
    }
    else if (_hwnat_info.features & NAT_IPV6) {
        if (IS_IPV4_DSLITE(entry)) {
            done_bit = entry->ipv4_dslite.resv1;
        } else if (IS_IPV6_3T_ROUTE(entry)) {
            done_bit = entry->ipv6_3t_route.resv1;
        } else if (IS_IPV6_5T_ROUTE(entry)) {
            done_bit = entry->ipv6_5t_route.resv1;
        } else if (IS_IPV6_6RD(entry)) {
            done_bit = entry->ipv6_6rd.resv1;
        } else {
            HWNAT_MSG_ERR("get packet format something wrong\n");
            return 0;
        }
    }
    if ((done_bit != 0) && (done_bit !=1)){
        HWNAT_MSG_DBG("done bit something wrong, done_bit = %d\n", done_bit);
        done_bit = 0;
    }
    //HWNAT_MSG_DBG("index = %d, done_bit=%d\n", FOE_ENTRY_NUM(skb), done_bit);
    return done_bit;
}

void _MDrv_HWNAT_Set_Done_Bit(struct foe_entry * entry)
{
    if (IS_IPV4_HNAT(entry) || IS_IPV4_HNAPT(entry)) {
        entry->ipv4_hnapt.resv1 = 1;
    }
    else if (_hwnat_info.features & NAT_IPV6) {
        if (IS_IPV4_DSLITE(entry)) {
            entry->ipv4_dslite.resv1 = 1;
        } else if (IS_IPV6_3T_ROUTE(entry)) {
            entry->ipv6_3t_route.resv1 = 1;
        } else if (IS_IPV6_5T_ROUTE(entry)) {
            entry->ipv6_5t_route.resv1 = 1;
        } else if (IS_IPV6_6RD(entry)) {
            entry->ipv6_6rd.resv1 = 1;
        } else {
            HWNAT_MSG_ERR("set packet format something wrong\n");
        }
    }
    wmb();
}

int _MDrv_HWNAT_Is_Wireless_Interface(struct sk_buff *skb)
{
    if ((strncmp(skb->dev->name, "rai", 3) == 0) ||
        (strncmp(skb->dev->name, "apclii", 6) == 0) ||
        (strncmp(skb->dev->name, "wdsi", 4) == 0) ||
        (strncmp(skb->dev->name, "wlan", 4) == 0))
        return 1;
    else
        return 0;
}



static void _MDrv_HWNAT_Handle_Mcast_Entry(struct sk_buff *skb, int gmac_no)
{
    if (ppe_parse_result.is_mcast) {
        MDrv_HWNAT_Mcast_Update_Qid(ppe_parse_result.vlan1, ppe_parse_result.dmac, M2Q_table[skb->mark]);
        if (_hwnat_info.features & HW_NAT_PSEUDO_SUPPORT) {
                if (lan_wan_separate == 1 && gmac_no == 2) {
                    MDrv_HWNAT_Mcast_Update_Qid(ppe_parse_result.vlan1, ppe_parse_result.dmac, M2Q_table[skb->mark] + 8);
                    if (_hwnat_info.features & HW_NAT_HW_SFQ) {
                        if (web_sfq_enable == 1 && (skb->mark == 2))
                            MDrv_HWNAT_Mcast_Update_Qid(ppe_parse_result.vlan1, ppe_parse_result.dmac, HWSFQUP);
                        /* queue3 */
                    }
                }
                if ((lan_wan_separate == 1) && (gmac_no == 1)) {
                    if (_hwnat_info.features & HW_NAT_HW_SFQ) {
                        if (web_sfq_enable == 1 && (skb->mark == 2))
                            MDrv_HWNAT_Mcast_Update_Qid(ppe_parse_result.vlan1, ppe_parse_result.dmac, HWSFQDL);
                        /* queue0 */
                    }
                }
        }


    }
}

static MS_BOOL _MDrv_HWNAT_Set_UDP_Frag(struct sk_buff *skb)
{
    if (_hwnat_info.features & WLAN_OPTIMIZE) {
        if (_MDrv_HWNAT_Is_BrLan_Subnet(skb) == FALSE) {
            if (!_MDrv_HWNAT_Is_Wireless_Interface(skb)) {
                USE_3T_UDP_FRAG = 0;
                return FALSE;
            }
            else
                USE_3T_UDP_FRAG = 1;
        }
        else
            USE_3T_UDP_FRAG = 0;
    }
    else {
        if (_MDrv_HWNAT_Is_BrLan_Subnet(skb)) {
            USE_3T_UDP_FRAG = 1;
        }
        else
            USE_3T_UDP_FRAG = 0;

    }
    return TRUE;
}

static MS_BOOL _MDrv_HWNAT_Reach_Unbind_Rate(struct sk_buff *skb, struct foe_entry *entry)
{
    if (_hwnat_info.features & HW_NAT_SEMI_AUTO_MODE) {
        if (IS_MAGIC_TAG_PROTECT_VALID(skb) &&
            (FOE_AI(skb) == HIT_UNBIND_RATE_REACH) &&
            (FOE_ALG(skb) == 0) && (_MDrv_HWNAT_Get_Done_Bit(skb, entry) == 0)) {
            HWNAT_MSG_DUMP("ppe driver set entry index = %d\n", FOE_ENTRY_NUM(skb));
            return TRUE;
        }
    }
    else {
        if (IS_MAGIC_TAG_PROTECT_VALID(skb) &&
            (FOE_AI(skb) == HIT_UNBIND_RATE_REACH) &&
            (FOE_ALG(skb) == 0)) {
            return TRUE;
        }
    }
    return FALSE;
}

static int32_t _MDrv_HWNAT_Handle_PPTP_L2TP_Tx(struct sk_buff *skb, struct foe_entry *entry)
{
    if (pptp_fast_path) {
        if (FOE_MAGIC_TAG(skb) == FOE_MAGIC_FASTPATH) {
            FOE_MAGIC_TAG(skb) = 0;
            if (_hwnat_info.debug_level >= 1)
                NAT_PRINT("MDrv_HWNAT_Handle_Tx FOE_MAGIC_FASTPATH\n");
            hash_cnt++;
            if ((FOE_AI(skb) == HIT_UNBIND_RATE_REACH) &&
                (skb->len > 128) && (hash_cnt % 32 == 1)) {
                MDrv_HWNAT_Send_Hash_Pkt(skb);
            }
            memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
            return 1;
        }
    }
    /*WAN->LAN accelerate*/
    if (l2tp_fast_path) {
        if (FOE_MAGIC_TAG(skb) == FOE_MAGIC_FASTPATH) {
            FOE_MAGIC_TAG(skb) = 0;
            if (_hwnat_info.debug_level >= 1)
                NAT_PRINT("MDrv_HWNAT_Handle_Tx FOE_MAGIC_FASTPATH\n");
            hash_cnt++;
            /* if((FOE_AI(skb) == HIT_UNBIND_RATE_REACH) && (skb->len > 1360)) */
            if ((FOE_AI(skb) == HIT_UNBIND_RATE_REACH) &&
                (skb->len > 1360) && (hash_cnt % 32 == 1)) {
                /* NAT_PRINT("hash_cnt is %d\n", hash_cnt); */
                MDrv_HWNAT_L2tp_Send_Hash_Pkt(skb);
            }
            memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
            return 1;
        }
    }

    if (IS_MAGIC_TAG_PROTECT_VALID(skb) &&
        (FOE_AI(skb) == HIT_UNBIND_RATE_REACH) &&
        (FOE_ALG(skb) == 0)) {
        struct iphdr *iph = NULL;
        struct udphdr *uh = NULL;

        iph = (struct iphdr *)(skb->data + 14 + VLAN_LEN);
        uh = (struct udphdr *)(skb->data + 14 + VLAN_LEN + 20);

        /* NAT_PRINT("iph->protocol is 0x%2x\n",iph->protocol); */
        /* NAT_PRINT("uh->dest is %4x\n",uh->dest); */
        /* NAT_PRINT("uh->source is %4x\n",uh->source); */
        /* NAT_PRINT("FOE_AI(skb) is 0x%x\n",FOE_AI(skb)); */

        /* skb_dump(skb); */
        if ((iph->protocol == IPPROTO_GRE) || (ntohs(uh->dest) == 1701)) {
            /*BIND flow using pptp/l2tp packets info */
            /* skb_dump(skb); */
            /* NAT_PRINT("LAN->WAN  TxH HIT_UNBIND_RATE_REACH\n"); */
            /* MDrv_HWNAT_Foe_Dump_Entry(FOE_ENTRY_NUM(skb)); */
            if (_hwnat_info.debug_level >= 1)
                NAT_PRINT("LAN->WAN  TxH PMDrv_HWNAT_Pptp_Wan_Parse_Layer\n");

            if (pptp_fast_path) {
                if (MDrv_HWNAT_Pptp_Wan_Parse_Layer(skb)) {
                    memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
                    return 1;
                }
            }

            if (l2tp_fast_path) {
                if (MDrv_HWNAT_L2tp_Wan_Parse_Layer(skb)) {
                    memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
                    return 1;
                }
            }

            /*layer2 keep original */
            if (_hwnat_info.debug_level >= 1)
                NAT_PRINT("Lan -> Wan _MDrv_HWNAT_Fill_L2\n");
            /* Set Layer2 Info */
            if (_MDrv_HWNAT_Fill_L2(skb, entry)) {
                memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
                /*already bind packet, return 1 to go out */
                return 1;
            }

            /* Set Layer3 Info */
            if (_MDrv_HWNAT_Fill_L3(skb, entry)) {
                memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
                return 1;
            }

            /* Set Layer4 Info */
            if (_MDrv_HWNAT_Fill_L4(skb, entry)) {
                memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
                return 1;
            }

            /* Set force port to CPU!!!  */
            if (MDrv_HWNAT_Set_Force_Port(skb, entry, 777)) {
                memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
                return 1;
            }
            if (_hwnat_info.debug_level >= 1)
                NAT_PRINT("_MDrv_HWNAT_Set_Entry_Bind\n");
            /* Set Pseudo Interface info in Foe entry */
            /* Enter binding state */
            _MDrv_HWNAT_Set_Entry_Bind(skb, entry);
            return 1;
        }
    }
    return 0;
}


int32_t MDrv_HWNAT_Handle_Tx(struct sk_buff *skb, int gmac_no)
{
    struct foe_entry *entry = FOE_ENTRY_BASE_BY_PKT(_hwnat_info.ppe_foe_base, skb);
    struct ps_entry *ps_entry = NULL;
    u8 which_region;

    if (_hwnat_info.features & PACKET_SAMPLING)
        ps_entry = PS_ENTRY_BASE_BY_PKT(_hwnat_info.ppe_ps_base, skb);

    which_region = _MDrv_HWNAT_Get_Tx_Region(skb);
    if (which_region == ALL_INFO_ERROR) {
        if (pr_debug_ratelimited())
            NAT_PRINT("MDrv_HWNAT_Handle_Tx : ALL_INFO_ERROR %04X,%04X\n", FOE_TAG_PROTECT_HEAD(skb), FOE_TAG_PROTECT_TAIL(skb));
        return 1;
    }
    if (FOE_ENTRY_NUM(skb) == 0x3fff) {
        if (_hwnat_info.debug_level >= 1)
            NAT_PRINT("FOE_ENTRY_NUM(skb)= 0x%x\n", FOE_ENTRY_NUM(skb));
        return 1;
    }

    if (FOE_ENTRY_NUM(skb) > FOE_4TB_SIZ) {
        if (_hwnat_info.debug_level >= 1)
            NAT_PRINT("FOE_ENTRY_NUM(skb)= 0x%x\n", FOE_ENTRY_NUM(skb));
        return 1;
    }

    if (_hwnat_info.debug_level >= 7)
        MDrv_HWNAT_Dump_TxSkb(skb);

     /* Packet is interested by ALG?*/
     /* Yes: Don't enter binind state*/
     /* No: If flow rate exceed binding threshold, enter binding state.*/
    if (_hwnat_info.features & PPTP_L2TP) {
        if (_MDrv_HWNAT_Handle_PPTP_L2TP_Tx(skb, entry) == 1) {
            return 1;
        }
    }

    if (_MDrv_HWNAT_Reach_Unbind_Rate(skb, entry)) {

        if (_MDrv_HWNAT_Set_UDP_Frag(skb) == FALSE) {
            return 1;
        }

        if (_hwnat_info.debug_level >= 6) {
            NAT_PRINT(" which_region = %d\n", which_region);
        }

        /* get start addr for each layer */
        if (_MDrv_HWNAT_Parse_Pkt_Layer(skb)) {
            memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
            return 1;
        }
        /* Set Layer2 Info */
        if (_MDrv_HWNAT_Fill_L2(skb, entry)) {
            memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
            return 1;
        }
        /* Set Layer3 Info */
        if (_MDrv_HWNAT_Fill_L3(skb, entry)) {
            memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
            return 1;
        }

        /* Set Layer4 Info */
        if (_MDrv_HWNAT_Fill_L4(skb, entry)) {
            memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
            return 1;
        }

        /* Set force port info */
        if (MDrv_HWNAT_Set_Force_Port(skb, entry, gmac_no)) {
            memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
            return 1;
        }

        /* Set Pseudo Interface info in Foe entry */
        if (_MDrv_HWNAT_Set_Extif_Num(skb, entry)) {
            memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
            return 1;
        }
        if (IS_QDMA_MCAST_SUPPORT(_hwnat_info.features)) {
            _MDrv_HWNAT_Handle_Mcast_Entry(skb, gmac_no);
        }

        if (_hwnat_info.features & PPE_MIB) {
            MHal_HWNAT_Clear_MIB_Counter(FOE_ENTRY_NUM(skb));
        }

        if (_hwnat_info.features & HW_NAT_AUTO_MODE) {
            /* Enter binding state */
            _MDrv_HWNAT_Set_Entry_Bind(skb, entry);
        }
        else if (_hwnat_info.features & HW_NAT_SEMI_AUTO_MODE) {
            _MDrv_HWNAT_Set_Done_Bit(entry);
        }

        if (_hwnat_info.features & PACKET_SAMPLING) {
            /*add sampling policy here*/
            ps_entry->en = 0x1 << 1;
            ps_entry->pkt_cnt = 0x10;
        }

    }
    else if (IS_MAGIC_TAG_PROTECT_VALID(skb) && (FOE_AI(skb) == HIT_BIND_PACKET_SAMPLING)) {
        /* this is duplicate packet in PS function*/
        /* just drop it */
        NAT_PRINT("PS drop#%d\n", FOE_ENTRY_NUM(skb));
        memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
        return 0;
    }
    else if (IS_MAGIC_TAG_PROTECT_VALID(skb) &&
          (FOE_AI(skb) == HIT_BIND_KEEPALIVE_MC_NEW_HDR ||
          (FOE_AI(skb) == HIT_BIND_KEEPALIVE_DUP_OLD_HDR))) {
        /*this is duplicate packet in keepalive new header mode*/
        /*just drop it */
        if (_hwnat_info.debug_level >= 3)
            NAT_PRINT("TxGot HITBIND_KEEPALIVE_DUP_OLD packe (%d)\n", FOE_ENTRY_NUM(skb));
        memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
        return 0;
    }
    else if (IS_MAGIC_TAG_PROTECT_VALID(skb) &&
            (FOE_AI(skb) == HIT_UNBIND_RATE_REACH) && (FOE_ALG(skb) == 1)) {
        if (_hwnat_info.debug_level >= 3)
            NAT_PRINT("FOE_ALG=1 (Entry=%d)\n", FOE_ENTRY_NUM(skb));
    }
    else if ((_hwnat_info.features & NAT_PREBIND) && (FOE_AI(skb) == HIT_PRE_BIND)) {
        if (entry->udib1.preb && entry->bfib1.state != BIND) {
            entry->bfib1.state = BIND;
            entry->udib1.preb = 0;
            /* Dump Binding Entry */
            if (_hwnat_info.debug_level >= 1)
                MDrv_HWNAT_Foe_Dump_Entry(FOE_ENTRY_NUM(skb));
        }
        else {
            /* drop duplicate prebind notify packet */
            memset(FOE_INFO_START_ADDR(skb), 0, FOE_INFO_LEN);
            return 0;
        }
    }

    return 1;
}


static int _MDrv_HWNAT_Init_Hash_Mode(struct HWNAT_DEVICE *phwnat)
{
    struct mhal_table_info info = {0};
    /* Allocate FOE table base */
    if (!_MDrv_HWNAT_Alloc_Tbl(phwnat))
        return 0;

    info.foe_adr = phwnat->ppe_phy_foe_base;
    info.ps_adr = phwnat->ppe_phy_ps_base;
    info.mib_adr = phwnat->ppe_phy_mib_base;
    info.entries = phwnat->table_entries;
    info.hash_dbg = phwnat->hash_debug;
    info.ipv6 = TRUE;
    info.prebind = (phwnat->features & NAT_PREBIND)?TRUE:FALSE;
    info.hash_mode = phwnat->hash_mode;
    info.mcast = (phwnat->features & PPE_MCAST)?TRUE:FALSE;
    info.qtx_qrx = (phwnat->features & HW_NAT_QDMATX_QDMARX)?TRUE:FALSE;

    if (phwnat->features == HW_NAT_SEMI_AUTO_MODE)
        info.op_mode = E_HWNAT_OPMODE_SEMI_AUTO;
    else if (phwnat->features == HW_NAT_MANUAL_MODE)
        info.op_mode = E_HWNAT_OPMODE_MANUAL;
    else
        info.op_mode = E_HWNAT_OPMODE_AUTO;

    MHal_HWNAT_Init(&info);

    return 1;
}


static void _MDrv_HWNAT_Set_Dst_Port(uint32_t ebl)
{
    if (ebl) {
        if ((_hwnat_info.features & HW_NAT_WIFI_NEW_ARCH) == 0x0) {
            if (_hwnat_info.features & HW_NAT_WIFI) {
                dst_port[DP_RA0] = dev_get_by_name(&init_net,"ra0");
                if (IS_APMBSS_MBSS_DEV(_hwnat_info.features)) {
                    dst_port[DP_RA1] = dev_get_by_name(&init_net,"ra1");
                    dst_port[DP_RA2] = dev_get_by_name(&init_net,"ra2");
                    dst_port[DP_RA3] = dev_get_by_name(&init_net,"ra3");
                    dst_port[DP_RA4] = dev_get_by_name(&init_net,"ra4");
                    dst_port[DP_RA5] = dev_get_by_name(&init_net,"ra5");
                    dst_port[DP_RA6] = dev_get_by_name(&init_net,"ra6");
                    dst_port[DP_RA7] = dev_get_by_name(&init_net,"ra7");
                    if (_hwnat_info.features & HW_NAT_MBSS_SUPPORT) {
                        dst_port[DP_RA8] = dev_get_by_name(&init_net,"ra8");
                        dst_port[DP_RA9] = dev_get_by_name(&init_net,"ra9");
                        dst_port[DP_RA10] = dev_get_by_name(&init_net,"ra10");
                        dst_port[DP_RA11] = dev_get_by_name(&init_net,"ra11");
                        dst_port[DP_RA12] = dev_get_by_name(&init_net,"ra12");
                        dst_port[DP_RA13] = dev_get_by_name(&init_net,"ra13");
                        dst_port[DP_RA14] = dev_get_by_name(&init_net,"ra14");
                        dst_port[DP_RA15] = dev_get_by_name(&init_net,"ra15");
                    }
                }
                if (_hwnat_info.features & HW_NAT_WDS_SUPPORT) {
                    dst_port[DP_WDS0] = dev_get_by_name(&init_net,"wds0");
                    dst_port[DP_WDS1] = dev_get_by_name(&init_net,"wds1");
                    dst_port[DP_WDS2] = dev_get_by_name(&init_net,"wds2");
                    dst_port[DP_WDS3] = dev_get_by_name(&init_net,"wds3");
                }

                if (_hwnat_info.features & HW_NAT_APCLI_SUPPORT)
                    dst_port[DP_APCLI0] = dev_get_by_name(&init_net,"apcli0");

                if (_hwnat_info.features & HW_NAT_AP_MESH_SUPPORT)
                    dst_port[DP_MESH0] = dev_get_by_name(&init_net,"mesh0");
                if (IS_SPECIAL_DEV(_hwnat_info.features)) {
                    dst_port[DP_RAI0] = dev_get_by_name(&init_net,"rai0");
                    if (_hwnat_info.features & HW_NAT_AP_MBSS_SUPPORT) {
                        dst_port[DP_RAI1] = dev_get_by_name(&init_net,"rai1");
                        dst_port[DP_RAI2] = dev_get_by_name(&init_net,"rai2");
                        dst_port[DP_RAI3] = dev_get_by_name(&init_net,"rai3");
                        dst_port[DP_RAI4] = dev_get_by_name(&init_net,"rai4");
                        dst_port[DP_RAI5] = dev_get_by_name(&init_net,"rai5");
                        dst_port[DP_RAI6] = dev_get_by_name(&init_net,"rai6");
                        dst_port[DP_RAI7] = dev_get_by_name(&init_net,"rai7");
                        dst_port[DP_RAI8] = dev_get_by_name(&init_net,"rai8");
                        dst_port[DP_RAI9] = dev_get_by_name(&init_net,"rai9");
                        dst_port[DP_RAI10] = dev_get_by_name(&init_net,"rai10");
                        dst_port[DP_RAI11] = dev_get_by_name(&init_net,"rai11");
                        dst_port[DP_RAI12] = dev_get_by_name(&init_net,"rai12");
                        dst_port[DP_RAI13] = dev_get_by_name(&init_net,"rai13");
                        dst_port[DP_RAI14] = dev_get_by_name(&init_net,"rai14");
                        dst_port[DP_RAI15] = dev_get_by_name(&init_net,"rai15");
                    }
                }
                if (_hwnat_info.features & HW_NAT_APCLI_SUPPORT)
                    dst_port[DP_APCLII0] = dev_get_by_name(&init_net,"apclii0");
                if (_hwnat_info.features & HW_NAT_WDS_SUPPORT) {
                    dst_port[DP_WDSI0] = dev_get_by_name(&init_net,"wdsi0");
                    dst_port[DP_WDSI1] = dev_get_by_name(&init_net,"wdsi1");
                    dst_port[DP_WDSI2] = dev_get_by_name(&init_net,"wdsi2");
                    dst_port[DP_WDSI3] = dev_get_by_name(&init_net,"wdsi3");
                }

                if (_hwnat_info.features & HW_NAT_AP_MESH_SUPPORT)
                    dst_port[DP_MESHI0] = dev_get_by_name(&init_net,"meshi0");

                if (_hwnat_info.features & HW_NAT_NIC_USB) {
                    dst_port[DP_PCI] = dev_get_by_name(&init_net,"eth0");   /* PCI interface name */
                    dst_port[DP_USB] = dev_get_by_name(&init_net,"eth1");   /* USB interface name */
                }
            }
        }


        if (_hwnat_info.features & HW_NAT_WIFI_NEW_ARCH) {
            struct net_device *dev;
            dev = MDrv_NOE_Get_Dev(E_NOE_DEV_MAIN);
            _MDrv_HWNAT_Regist_Dev_Handler(dev);
            DP_GMAC1 = _MDrv_HWNAT_Get_Dev_Handler_Idx(dev);
            HWNAT_MSG_DBG("%s ifindex =%x\n", dev->name, DP_GMAC1);
            if (_hwnat_info.features & HW_NAT_PSEUDO_SUPPORT) {
                dev = MDrv_NOE_Get_Dev(E_NOE_DEV_PSEUDO);
                if (dev != NULL) {
                    _MDrv_HWNAT_Regist_Dev_Handler(dev);
                    DPORT_GMAC2 = _MDrv_HWNAT_Get_Dev_Handler_Idx(dev);
                    HWNAT_MSG_DBG("%s ifindex =%x\n", dev->name, DPORT_GMAC2);
                }
                else {
                    HWNAT_MSG_WARN("cannot find noe gmac2\n");
                }
            }
        }
        else {
            dst_port[DP_GMAC] = MDrv_NOE_Get_Dev(E_NOE_DEV_MAIN);
            if (_hwnat_info.features & HW_NAT_PSEUDO_SUPPORT) {
                dst_port[DP_GMAC2] = MDrv_NOE_Get_Dev(E_NOE_DEV_PSEUDO);
            }
        }
    }
    else {
        if (_hwnat_info.features & HW_NAT_WIFI_NEW_ARCH) {
            int j = 0;
            for (j = 0; j < MAX_IF_NUM; j++) {
                if (dst_port[j]) {
                    dev_put(dst_port[j]);
                    dst_port[j] = NULL;
                }
            }
        }
        else {

            if (_hwnat_info.features & HW_NAT_WIFI) {
                if (dst_port[DP_RA0])
                    dev_put(dst_port[DP_RA0]);
                if (dst_port[DP_RA1])
                    dev_put(dst_port[DP_RA1]);
                if (dst_port[DP_RA2])
                    dev_put(dst_port[DP_RA2]);
                if (dst_port[DP_RA3])
                    dev_put(dst_port[DP_RA3]);
                if (dst_port[DP_RA4])
                    dev_put(dst_port[DP_RA4]);
                if (dst_port[DP_RA5])
                    dev_put(dst_port[DP_RA5]);
                if (dst_port[DP_RA6])
                    dev_put(dst_port[DP_RA6]);
                if (dst_port[DP_RA7])
                    dev_put(dst_port[DP_RA7]);
                if (dst_port[DP_RA8])
                    dev_put(dst_port[DP_RA8]);
                if (dst_port[DP_RA9])
                    dev_put(dst_port[DP_RA9]);
                if (dst_port[DP_RA10])
                    dev_put(dst_port[DP_RA10]);
                if (dst_port[DP_RA11])
                    dev_put(dst_port[DP_RA11]);
                if (dst_port[DP_RA12])
                    dev_put(dst_port[DP_RA12]);
                if (dst_port[DP_RA13])
                    dev_put(dst_port[DP_RA13]);
                if (dst_port[DP_RA14])
                    dev_put(dst_port[DP_RA14]);
                if (dst_port[DP_RA15])
                    dev_put(dst_port[DP_RA15]);
                if (dst_port[DP_WDS0])
                    dev_put(dst_port[DP_WDS0]);
                if (dst_port[DP_WDS1])
                    dev_put(dst_port[DP_WDS1]);
                if (dst_port[DP_WDS2])
                    dev_put(dst_port[DP_WDS2]);
                if (dst_port[DP_WDS3])
                    dev_put(dst_port[DP_WDS3]);
                if (dst_port[DP_APCLI0])
                    dev_put(dst_port[DP_APCLI0]);
                if (dst_port[DP_MESH0])
                    dev_put(dst_port[DP_MESH0]);
                if (dst_port[DP_RAI0])
                    dev_put(dst_port[DP_RAI0]);
                if (dst_port[DP_RAI1])
                    dev_put(dst_port[DP_RAI1]);
                if (dst_port[DP_RAI2])
                    dev_put(dst_port[DP_RAI2]);
                if (dst_port[DP_RAI3])
                    dev_put(dst_port[DP_RAI3]);
                if (dst_port[DP_RAI4])
                    dev_put(dst_port[DP_RAI4]);
                if (dst_port[DP_RAI5])
                    dev_put(dst_port[DP_RAI5]);
                if (dst_port[DP_RAI6])
                    dev_put(dst_port[DP_RAI6]);
                if (dst_port[DP_RAI7])
                    dev_put(dst_port[DP_RAI7]);
                if (dst_port[DP_RAI8])
                    dev_put(dst_port[DP_RAI8]);
                if (dst_port[DP_RAI9])
                    dev_put(dst_port[DP_RAI9]);
                if (dst_port[DP_RAI10])
                    dev_put(dst_port[DP_RAI10]);
                if (dst_port[DP_RAI11])
                    dev_put(dst_port[DP_RAI11]);
                if (dst_port[DP_RAI12])
                    dev_put(dst_port[DP_RAI12]);
                if (dst_port[DP_RAI13])
                    dev_put(dst_port[DP_RAI13]);
                if (dst_port[DP_RAI14])
                    dev_put(dst_port[DP_RAI14]);
                if (dst_port[DP_RAI15])
                    dev_put(dst_port[DP_RAI15]);
                if (dst_port[DP_APCLII0])
                    dev_put(dst_port[DP_APCLII0]);
                if (dst_port[DP_WDSI0])
                    dev_put(dst_port[DP_WDSI0]);
                if (dst_port[DP_WDSI1])
                    dev_put(dst_port[DP_WDSI1]);
                if (dst_port[DP_WDSI2])
                    dev_put(dst_port[DP_WDSI2]);
                if (dst_port[DP_WDSI3])
                    dev_put(dst_port[DP_WDSI3]);


                if (dst_port[DP_MESHI0])
                    dev_put(dst_port[DP_MESHI0]);
                if (dst_port[DP_GMAC])
                    dev_put(dst_port[DP_GMAC]);
                if (_hwnat_info.features & HW_NAT_PSEUDO_SUPPORT) {
                    if (dst_port[DP_GMAC2])
                        dev_put(dst_port[DP_GMAC2]);
                }
            }
            if (_hwnat_info.features & HW_NAT_NIC_USB) {
                if (dst_port[DP_PCI])
                    dev_put(dst_port[DP_PCI]);
                if (dst_port[DP_USB])
                    dev_put(dst_port[DP_USB]);
            }
        }
    }
}



static void _MDrv_HWNAT_Handle_Ac_Update(unsigned long unused)
{
    int i = 0;
    struct mhal_hwnat_ac_info info;

    for (i = 1; i <= 2; i++) {
        memset(&info, 0, sizeof(struct mhal_hwnat_ac_info));
        info.idx = i;
        MHal_HWNAT_Get_Ac_Info(&info);
        ac_info[i].ag_byte_cnt += info.bytes;
        ac_info[i].ag_pkt_cnt += info.pkts;
    }
    update_foe_ac_timer.expires = jiffies + 16 * HZ;
    add_timer(&update_foe_ac_timer);
}

static void _MDrv_HWNAT_Init_Ac_Update(void)
{
    ac_info[1].ag_byte_cnt = 0;
    ac_info[1].ag_pkt_cnt = 0;
    ac_info[2].ag_byte_cnt = 0;
    ac_info[2].ag_pkt_cnt = 0;
    ac_info[3].ag_byte_cnt = 0;
    ac_info[3].ag_pkt_cnt = 0;
    ac_info[4].ag_byte_cnt = 0;
    ac_info[4].ag_pkt_cnt = 0;
    ac_info[5].ag_byte_cnt = 0;
    ac_info[5].ag_pkt_cnt = 0;
    ac_info[6].ag_byte_cnt = 0;
    ac_info[6].ag_pkt_cnt = 0;
}

static void _MDrv_HWNAT_Set_Ac_Update(int ebl)
{
    if (_hwnat_info.features & ACCNT_MAINTAINER) {
        if (ebl) {
            _MDrv_HWNAT_Init_Ac_Update();
            update_foe_ac_timer.expires = jiffies + HZ;
            add_timer(&update_foe_ac_timer);
        } else {
            if (timer_pending(&update_foe_ac_timer))
                del_timer_sync(&update_foe_ac_timer);
        }
    }
}

static void _MDrv_HWNAT_Clear_Bind_Entry(struct neighbour *neigh)
{
    int hash_index, clear;
    struct foe_entry *entry;
    u32 * daddr = (u32 *)neigh->primary_key;
    const u8 *addrtmp;
    u8 mac0,mac1,mac2,mac3,mac4,mac5;
    u32 dip;
    dip = (u32)(*daddr);
    clear = 0;
    addrtmp = neigh->ha;
    mac0 = (u8)(*addrtmp);
    mac1 = (u8)(*(addrtmp+1));
    mac2 = (u8)(*(addrtmp+2));
    mac3 = (u8)(*(addrtmp+3));
    mac4 = (u8)(*(addrtmp+4));
    mac5 = (u8)(*(addrtmp+5));

        for (hash_index = 0; hash_index < FOE_4TB_SIZ; hash_index++) {
        entry = FOE_ENTRY_BASE_BY_IDX(_hwnat_info.ppe_foe_base, hash_index);
        if(entry->bfib1.state == BIND) {
            /*NAT_PRINT("before old mac= %x:%x:%x:%x:%x:%x, new_dip=%x\n",
                entry->ipv4_hnapt.dmac_hi[3],
                entry->ipv4_hnapt.dmac_hi[2],
                entry->ipv4_hnapt.dmac_hi[1],
                entry->ipv4_hnapt.dmac_hi[0],
                entry->ipv4_hnapt.dmac_lo[1],
                entry->ipv4_hnapt.dmac_lo[0], entry->ipv4_hnapt.new_dip);
            */
            if (entry->ipv4_hnapt.new_dip == ntohl(dip)) {
                if ((entry->ipv4_hnapt.dmac_hi[3] != mac0) ||
                    (entry->ipv4_hnapt.dmac_hi[2] != mac1) ||
                    (entry->ipv4_hnapt.dmac_hi[1] != mac2) ||
                    (entry->ipv4_hnapt.dmac_hi[0] != mac3) ||
                    (entry->ipv4_hnapt.dmac_lo[1] != mac4) ||
                    (entry->ipv4_hnapt.dmac_lo[0] != mac5)) {
                    NAT_PRINT("%s: state=%d\n",__func__,neigh->nud_state);
                    MHal_HWNAT_Set_Miss_Action(E_HWNAT_FOE_SEARCH_MISS_ONLY_FWD_CPU);

                    entry->ipv4_hnapt.udib1.state = INVALID;
                    entry->ipv4_hnapt.udib1.time_stamp = MHAL_HWNAT_Get_TimeStamp() & 0xFF;
                    MHal_HWNAT_Enable_Cache();
                    mod_timer(&hwnat_clear_entry_timer, jiffies + 3 * HZ);

                    NAT_PRINT("delete old entry: dip =%x\n", ntohl(dip));

                    NAT_PRINT("old mac= %x:%x:%x:%x:%x:%x, dip=%x\n",
                            entry->ipv4_hnapt.dmac_hi[3],
                            entry->ipv4_hnapt.dmac_hi[2],
                            entry->ipv4_hnapt.dmac_hi[1],
                            entry->ipv4_hnapt.dmac_hi[0],
                            entry->ipv4_hnapt.dmac_lo[1],
                            entry->ipv4_hnapt.dmac_lo[0],
                            ntohl(dip));
                    NAT_PRINT("new mac= %x:%x:%x:%x:%x:%x, dip=%x\n", mac0, mac1, mac2, mac3, mac4, mac5, ntohl(dip));

                }
            }
        }
    }
}

static int MDrv_HWNAT_Handle_NetEvent(struct notifier_block *unused, unsigned long event, void *ptr)
{
        struct net_device *dev = NULL;
        struct neighbour *neigh = NULL;
        int err = 0;

        switch (event) {
        case NETEVENT_NEIGH_UPDATE:
                neigh = ptr;
                dev = neigh->dev;
                if (dev)
                    _MDrv_HWNAT_Clear_Bind_Entry(neigh);
                if (err)
                    HWNAT_MSG_ERR("failed to handle neigh update (err %d)\n", err);
                break;
        }

        return NOTIFY_DONE;
}





static int32_t _MDrv_HWNAT_Module_Init(void)
{
    /*PPE Enabled: GMAC<->PPE<->CPU*/
    /*PPE Disabled: GMAC<->CPU*/
    struct mhal_hwnat_engine engine;
    HWNAT_MSG_DBG("NOE HW NAT Module Enabled\n");

    _hwnat_info.pdev = platform_device_alloc("HW_NAT", PLATFORM_DEVID_AUTO);

    if (!_hwnat_info.pdev) {
        HWNAT_MSG_ERR("[%s][%d] NULL dev \n",__FUNCTION__,__LINE__);
        return -ENOMEM;
    }
    _hwnat_info.pdev->dev.dma_mask = &hwnat_dmamask;
    _hwnat_info.pdev->dev.coherent_dma_mask  = 0xffffffff;
    MDrv_NOE_NAT_Set_Dma_Ops(&_hwnat_info.pdev->dev, FALSE);
    MDRV_HWNAT_CONFIG_SET_FEATURES(_hwnat_info);
    MDRV_HWNAT_CONFIG_SET_TEST(_hwnat_info);

    MDrv_HWNAT_LOG_Init(&_hwnat_info);
    /* Register ioctl handler */
    //MDrv_HWNAT_IOCTL_Init(&_hwnat_info);
    /* Set PPE FOE Hash Mode */
    if (!_MDrv_HWNAT_Init_Hash_Mode(&_hwnat_info)) {
        HWNAT_MSG_ERR("memory allocation failed\n");
        return -ENOMEM; /* memory allocation failed */
    }

    /* Get net_device structure of Dest Port */
    _MDrv_HWNAT_Set_Dst_Port(1);

    /* Register ioctl handler */
    MDrv_HWNAT_IOCTL_Register_Handler();

    MHal_HWNAT_Map_ForcePort();
    MHal_HWNAT_Set_Pkt_Prot();
    MHal_HWNAT_Enable_Cache();
    MHal_HWNAT_Check_Switch_Vlan(FALSE);
    _MDrv_HWNAT_Set_Ac_Update(1);
    /* Initialize PPE related register */
    engine.rate.bind_rate = DFL_FOE_BNDR;
    engine.rate.tbl_read_rate = DFL_PBND_RD_PRD;
    engine.ka.cfg = DFL_FOE_KA;
    engine.ka.udp = DFL_FOE_TCP_KA;
    engine.ka.tcp = DFL_FOE_UDP_KA;
    engine.ka.ntu = DFL_FOE_NTU_KA;
    engine.ka.pbnd_limit = DFL_PBND_RD_LMT;
    engine.age_out.udp_time = DFL_FOE_UDP_DLTA;
    engine.age_out.tcp_time = DFL_FOE_TCP_DLTA;
    engine.age_out.fin_time = DFL_FOE_FIN_DLTA;
    engine.age_out.ntu_time = DFL_FOE_NTU_DLTA;
    engine.age_out.unb_time = DFL_FOE_UNB_DLTA;
    engine.age_out.unb_pkt_cnt = DFL_FOE_UNB_MNP;

    MHal_HWNAT_Start_Engine(&engine);
    /* In manual mode, PPE always reports UN-HIT CPU reason, so we don't need to process it */
    /* Register RX/TX hook point */
    if ((_hwnat_info.features & HW_NAT_MANUAL_MODE) == 0x0) {
        noe_nat_hook_tx = MDrv_HWNAT_Handle_Tx;
        noe_nat_hook_rx = MDrv_HWNAT_Handle_Rx;
    }

    if (_hwnat_info.features & HW_NAT_WIFI_NEW_ARCH) {
        ppe_dev_register_hook = _MDrv_HWNAT_Regist_Dev_Handler;
        ppe_dev_unregister_hook = _MDrv_HWNAT_Unregist_Dev_Handler;
    }
    /* Set GMAC fowrards packet to PPE */
    MHal_HWNAT_Set_Pkt_Dst(E_NOE_GE_MAC1, E_HWNAT_PKT_DST_ENGINE);
    MHal_HWNAT_Set_Pkt_Dst(E_NOE_GE_MAC2, E_HWNAT_PKT_DST_ENGINE);

    if (_hwnat_info.features & PPTP_L2TP) {
        MDrv_HWNAT_Pptp_L2tp_Init();
    }
    register_netevent_notifier(&Hnat_netevent_nb);
    init_timer(&hwnat_clear_entry_timer);
    hwnat_clear_entry_timer.function = _MDrv_HWNAT_Clear_Entry ;
    return 0;
}

static void _MDrv_HWNAT_Module_Exit(void)
{
    HWNAT_MSG_DBG("Ralink HW NAT Module Disabled\n");

    /* Set GMAC fowrards packet to CPU */
    MHal_HWNAT_Set_Pkt_Dst(E_NOE_GE_MAC1, E_HWNAT_PKT_DST_CPU);
    MHal_HWNAT_Set_Pkt_Dst(E_NOE_GE_MAC2, E_HWNAT_PKT_DST_CPU);

    /* Unregister RX/TX hook point */
    noe_nat_hook_rx = NULL;
    noe_nat_hook_tx = NULL;
    if (_hwnat_info.features & HW_NAT_WIFI_NEW_ARCH) {
        ppe_dev_register_hook = NULL;
        ppe_dev_unregister_hook = NULL;
    }

    /* Restore PPE related register */
    /* ppe_eng_stop(); */
    /* iounmap(ppe_foe_base); */

    /* Unregister ioctl handler */
    MDrv_HWNAT_IOCTL_Unregister_Handler();
    MHal_HWNAT_Check_Switch_Vlan(1);
    _MDrv_HWNAT_Set_Ac_Update(0);

    if (IS_QDMA_MCAST_SUPPORT(_hwnat_info.features)) {
        MDrv_HWNAT_Mcast_Delete_All();
    }

    /* Release net_device structure of Dest Port */
    _MDrv_HWNAT_Set_Dst_Port(0);

    if (_hwnat_info.features & PPTP_L2TP) {
        MDrv_HWNAT_Pptp_L2tp_Clean();
    }
    unregister_netevent_notifier(&Hnat_netevent_nb);
}

module_init(_MDrv_HWNAT_Module_Init);
module_exit(_MDrv_HWNAT_Module_Exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HWNAT\n");
