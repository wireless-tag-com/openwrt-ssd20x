/*
* mdrv_noe_qdma.c- Sigmastar
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
/// @file   MDrv_NOE_PDMA.c
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------

#include "mdrv_noe.h"
#include "mdrv_noe_nat.h"
#include "mdrv_noe_dma.h"
#include "mhal_noe_reg.h"


//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------
//  Local Variable
//--------------------------------------------------------------------------------------------------

/* skb->mark to queue mapping table */
struct QDMA_txdesc *free_head;

/* ioctl */
unsigned int M2Q_table[64] = { 0 };
EXPORT_SYMBOL(M2Q_table);
unsigned int lan_wan_separate;
EXPORT_SYMBOL(lan_wan_separate);
struct sk_buff *magic_id = (struct sk_buff *)0xFFFFFFFF;

/* CONFIG_HW_SFQ */
unsigned int web_sfq_enable;
#define HW_SFQ_UP 3
#define HW_SFQ_DL 1

#define sfq_debug 0
struct SFQ_table *sfq0;
struct SFQ_table *sfq1;
struct SFQ_table *sfq2;
struct SFQ_table *sfq3;

#define KSEG1                   0xa0000000
#define PHYS_TO_VIRT(x)         phys_to_virt(x)
#define VIRT_TO_PHYS(x)         virt_to_phys(x)
/* extern void set_fe_dma_glo_cfg(void); */
struct parse_result sfq_parse_result;




static inline unsigned int _MDrv_NOE_QDMA_Calc_Frag_Txd_Num(unsigned int size)
{
    unsigned int frag_txd_num = 0;

    if (size == 0)
        return 0;
    while (size > 0) {
        if (size > MAX_QTXD_LEN) {
            frag_txd_num++;
            size -= MAX_QTXD_LEN;
        } else {
            frag_txd_num++;
            size = 0;
        }
    }
    return frag_txd_num;
}


static inline int _MDrv_NOE_QDMA_Get_Free_Txd(struct END_DEVICE *ei_local, int ring_no)
{
    unsigned int tmp_idx;

    tmp_idx = ei_local->free_txd_head[ring_no];
    ei_local->free_txd_head[ring_no] = ei_local->txd_pool_info[tmp_idx];
    atomic_sub(1, &ei_local->free_txd_num[ring_no]);
    return tmp_idx;
}

static inline unsigned int _MDrv_NOE_QDMA_Get_Txd_Phy_Adr(struct END_DEVICE *ei_local, unsigned int idx)
{
    return ei_local->phy_txd_pool + (idx * QTXD_LEN);
}


static inline void _MDrv_NOE_QDMA_Put_Back_Free_Txd(struct END_DEVICE *ei_local, int ring_no, int free_txd_idx)
{
    ei_local->txd_pool_info[ei_local->free_txd_tail[ring_no]] = free_txd_idx;
    ei_local->free_txd_tail[ring_no] = free_txd_idx;
}

static void _MDrv_NOE_QDMA_Init_Free_Txd(struct END_DEVICE *ei_local)
{
    int i;

    for (i = 0; i < GMAC1_TXQ_NUM; i++) {
        atomic_set(&ei_local->free_txd_num[i], GMAC1_TXQ_TXD_NUM);
        ei_local->free_txd_head[i] = GMAC1_TXQ_TXD_NUM * i;
        ei_local->free_txd_tail[i] = GMAC1_TXQ_TXD_NUM * (i + 1) - 1;
        ei_local->stats.min_free_txd[i] = GMAC1_TXQ_TXD_NUM;
    }
    for (i = 0; i < GMAC2_TXQ_NUM; i++) {
        atomic_set(&ei_local->free_txd_num[i + GMAC1_TXQ_NUM], GMAC2_TXQ_TXD_NUM);
        ei_local->free_txd_head[i + GMAC1_TXQ_NUM] = GMAC1_TXD_NUM + GMAC2_TXQ_TXD_NUM * i;
        ei_local->free_txd_tail[i + GMAC1_TXQ_NUM] = GMAC1_TXD_NUM + GMAC2_TXQ_TXD_NUM * (i + 1) - 1;
        ei_local->stats.min_free_txd[i + GMAC1_TXQ_NUM] = GMAC2_TXQ_TXD_NUM;
    }
}

static inline int _MDrv_NOE_QDMA_Map_Ring_No(int txd_idx)
{
    int i;

    if (txd_idx < GMAC1_TXD_NUM) {
        for (i = 0; i < GMAC1_TXQ_NUM; i++) {
            if (txd_idx < (GMAC1_TXQ_TXD_NUM * (i + 1)))
                return i;
        }
    }

    txd_idx -= GMAC1_TXD_NUM;
    for (i = 0; i < GMAC2_TXQ_NUM; i++) {
        if (txd_idx < (GMAC2_TXQ_TXD_NUM * (i + 1)))
            return (i + GMAC1_TXQ_NUM);
    }
    NOE_MSG_ERR("txd index out of range\n");
    return 0;
}

bool _MDrv_NOE_QDMA_Alloc_Txd(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    unsigned int txd_idx;
    struct noe_dma_info dma_info;
    int i = 0;
#if 0
    ei_local->txd_pool = dma_alloc_coherent(&ei_local->qdma_pdev->dev, QTXD_LEN * NUM_TX_DESC, &ei_local->phy_txd_pool, GFP_KERNEL);
#else
{
     MSYS_DMEM_INFO mem_info;
     int ret;
     mem_info.length = QTXD_LEN * NUM_TX_DESC;
     strcpy(mem_info.name, "txd_pool");
     if((ret=msys_request_dmem(&mem_info)))
     {
         NOE_MSG_ERR("unable to allocate DMEM for  %s!! error=%d\n", mem_info.name ,ret);
     }
     ei_local->phy_txd_pool = (dma_addr_t)((size_t)mem_info.phys);
     ei_local->txd_pool = (void *)((size_t)mem_info.kvirt);
}
#endif

    NOE_MSG_DBG("[QUE][TX_DESC] num=%d vir=0x%p phy_txd_pool=0x%p  len=%x\n", NUM_TX_DESC, ei_local->txd_pool, (void *)ei_local->phy_txd_pool, QTXD_LEN);

    if (!ei_local->txd_pool) {
        NOE_MSG_ERR("adapter->txd_pool allocation failed!\n");
        return 0;
    }
    NOE_MSG_DBG("skb_free: 0x%p.\n", ei_local->skb_free);
    /* set all txd_pool_info to 0. */
    for (i = 0; i < NUM_TX_DESC; i++) {
        ei_local->skb_free[i] = 0;
        ei_local->txd_pool_info[i] = i + 1;
        ei_local->txd_pool[i].txd_info3.LS_bit = 1;
        ei_local->txd_pool[i].txd_info3.OWN_bit = 1;
    }
    Chip_Flush_MIU_Pipe(); //L3

    _MDrv_NOE_QDMA_Init_Free_Txd(ei_local);

    /* get free txd from txd pool */
    txd_idx = _MDrv_NOE_QDMA_Get_Free_Txd(ei_local, 0);
    ei_local->tx_cpu_idx = txd_idx;
    NOE_MSG_DBG("tx_cpu_idx = 0x%x \n", ei_local->tx_cpu_idx);
    /* add null TXD for transmit */
    dma_info.adr_st.ctx_adr = _MDrv_NOE_QDMA_Get_Txd_Phy_Adr(ei_local, txd_idx);
    dma_info.adr_st.dtx_adr = _MDrv_NOE_QDMA_Get_Txd_Phy_Adr(ei_local, txd_idx);

    /* get free txd from txd pool */
    txd_idx = _MDrv_NOE_QDMA_Get_Free_Txd(ei_local, 0);
    ei_local->rls_cpu_idx = txd_idx;
    NOE_MSG_DBG("rls_cpu_idx = 0x%x \n", ei_local->rls_cpu_idx);
    /* add null TXD for release */
    dma_info.adr_st.crx_adr = _MDrv_NOE_QDMA_Get_Txd_Phy_Adr(ei_local, txd_idx);
    dma_info.adr_st.drx_adr = _MDrv_NOE_QDMA_Get_Txd_Phy_Adr(ei_local, txd_idx);

    MHal_NOE_DMA_Init(E_NOE_DMA_QUEUE, E_NOE_DIR_TX, E_NOE_RING_NO0, &dma_info);

    return 1;
}

bool _MDrv_NOE_QDMA_SFQ_Init(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    dma_addr_t sfq_phy[8];
    struct SFQ_table *sfq[8] = {NULL};
    struct noe_sfq_base adr_info;
    int i = 0, j = 0;
    int vq_num[8] = {
        VQ_NUM0,
        VQ_NUM1,
        VQ_NUM2,
        VQ_NUM3,
        VQ_NUM4,
        VQ_NUM5,
        VQ_NUM6,
        VQ_NUM7,
    };

    for (i = 0; i< 8; i++) {
        sfq[i] = dma_alloc_coherent(&ei_local->qdma_pdev->dev, vq_num[i] * sizeof(struct SFQ_table), &sfq_phy[i], GFP_KERNEL);
        if (unlikely(!sfq[i])) {
            NOE_MSG_ERR("QDMA SFQ%d VQ not available...\n",i);
            return 1;
        }
        memset(&sfq[i], 0x0, vq_num[i] * sizeof(struct SFQ_table));
        for (j = 0; j < vq_num[i]; j++) {
            sfq[i][j].sfq_info1.VQHPTR = 0xdeadbeef;
            sfq[i][j].sfq_info2.VQTPTR = 0xdeadbeef;
        }
        adr_info.phy_adr[i] = (MS_U32) sfq_phy[i];
        //NOE_MSG_DUMP("*****(sfq_phy,sfq_virt)[%d] = (0x%lx ,0x%llx)*******\n", i, sfq_phy[0], sfq[i]);

    }

    MHal_NOE_DMA_SFQ_Init(&adr_info);
    return 0;
}

static bool _MDrv_NOE_QDMA_Init_Fq(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    /* struct QDMA_txdesc *free_head = NULL; */
    dma_addr_t phy_free_head;
    dma_addr_t phy_free_tail;
    unsigned int *free_page_head = NULL;
    dma_addr_t phy_free_page_head;
    int i;
    struct noe_fq_base info;
#if 0
    free_head = dma_alloc_coherent(&ei_local->qdma_pdev->dev, NUM_QDMA_PAGE * QTXD_LEN, &phy_free_head, GFP_KERNEL);
#else
{
     MSYS_DMEM_INFO mem_info;
     int ret;
     mem_info.length = NUM_QDMA_PAGE * QTXD_LEN;
     strcpy(mem_info.name, "QDMA_DFQ");
     if((ret=msys_request_dmem(&mem_info)))
     {
         NOE_MSG_ERR("unable to allocate DMEM for  %s!! error=%d\n", mem_info.name ,ret);
     }
    phy_free_head = (dma_addr_t)((size_t)mem_info.phys);
    free_head = (void *)((size_t)mem_info.kvirt);
}
#endif
    if (unlikely(!free_head)) {
        NOE_MSG_ERR("QDMA FQ decriptor not available...\n");
        return 0;
    }

    NOE_MSG_DBG("[QUE_TX][FreeQueue] phy=0x%x ,vir= 0x%p  \n", phy_free_head, free_head);
    memset(free_head, 0x0, QTXD_LEN * NUM_QDMA_PAGE);
    Chip_Flush_MIU_Pipe(); //L3

#if 0
    free_page_head = dma_alloc_coherent(&ei_local->qdma_pdev->dev, NUM_QDMA_PAGE * QDMA_PAGE_SIZE, &phy_free_page_head, GFP_KERNEL);
#else
    {
        MSYS_DMEM_INFO mem_info;
        int ret;
        mem_info.length = NUM_QDMA_PAGE * QDMA_PAGE_SIZE;
        strcpy(mem_info.name, "QDMA_FQ");
        if((ret=msys_request_dmem(&mem_info)))
        {
            NOE_MSG_ERR("unable to allocate DMEM for  %s!! error=%d\n", mem_info.name ,ret);
        }
        phy_free_page_head = mem_info.phys;
        free_page_head = (phys_addr_t *)((size_t)mem_info.kvirt);
   }
#endif
    if (unlikely(!free_page_head)) {
        NOE_MSG_ERR("QDMA FQ page not available...\n");
        return 0;
    }
    for (i = 0; i < NUM_QDMA_PAGE; i++) {
        free_head[i].txd_info1.SDP = (phy_free_page_head + (i * QDMA_PAGE_SIZE));
        if (i < (NUM_QDMA_PAGE - 1)) {
            free_head[i].txd_info2.NDP = (phy_free_head + ((i + 1) * QTXD_LEN));
            NOE_MSG_DUMP("NDP[%4d] = 0x%x  \n", i, free_head[i].txd_info2.NDP);
        }
        free_head[i].txd_info3.SDL = QDMA_PAGE_SIZE;
    }
    Chip_Flush_MIU_Pipe(); //L3
    phy_free_tail = (phy_free_head + (u32)((NUM_QDMA_PAGE - 1) * QTXD_LEN));

    NOE_MSG_DBG("phy_free(head, tail):(0x%p,0x%p)\n",(void *)phy_free_head, (void *)phy_free_tail);

    info.head = (MS_U32)phy_free_head;
    info.tail = (MS_U32)phy_free_tail;
    info.txd_num = NUM_TX_DESC;
    info.page_num = NUM_QDMA_PAGE;
    info.page_size = QDMA_PAGE_SIZE;

    MHal_NOE_DMA_FQ_Init(&info);
    NOE_MSG_DBG("[TXD_NUM]GMAC1:%d; GMAC2:%d; Total:%d\n", GMAC1_TXD_NUM, GMAC2_TXD_NUM, NUM_TX_DESC);
    ei_local->free_head = free_head;
    ei_local->phy_free_head = phy_free_head;
    ei_local->free_page_head = free_page_head;
    ei_local->phy_free_page_head = phy_free_page_head;
    ei_local->tx_ring_full = 0;
    return 1;
}

int sfq_prot;

#if (sfq_debug)
int udp_source_port;
int tcp_source_port;
int ack_packt;
#endif
static int _MDrv_NOE_QDMA_SFQ_Parse_Net_Layer(struct sk_buff *skb)
{
    struct vlan_hdr *vh_sfq = NULL;
    struct ethhdr *eth_sfq = NULL;
    struct iphdr *iph_sfq = NULL;
    struct ipv6hdr *ip6h_sfq = NULL;
    struct tcphdr *th_sfq = NULL;
    struct udphdr *uh_sfq = NULL;

    memset(&sfq_parse_result, 0, sizeof(sfq_parse_result));
    eth_sfq = (struct ethhdr *)skb->data;
    ether_addr_copy(sfq_parse_result.dmac, eth_sfq->h_dest);
    ether_addr_copy(sfq_parse_result.smac, eth_sfq->h_source);
    /* memcpy(sfq_parse_result.dmac, eth_sfq->h_dest, ETH_ALEN); */
    /* memcpy(sfq_parse_result.smac, eth_sfq->h_source, ETH_ALEN); */
    sfq_parse_result.eth_type = eth_sfq->h_proto;

    if (sfq_parse_result.eth_type == htons(ETH_P_8021Q)) {
        sfq_parse_result.vlan1_gap = VLAN_HLEN;
        vh_sfq = (struct vlan_hdr *)(skb->data + ETH_HLEN);
        sfq_parse_result.eth_type = vh_sfq->h_vlan_encapsulated_proto;
    } else {
        sfq_parse_result.vlan1_gap = 0;
    }

    /* set layer4 start addr */
    if ((sfq_parse_result.eth_type == htons(ETH_P_IP)) || (sfq_parse_result.eth_type == htons(ETH_P_PPP_SES) && sfq_parse_result.ppp_tag == htons(PPP_IP))) {
        iph_sfq = (struct iphdr *)(skb->data + ETH_HLEN + (sfq_parse_result.vlan1_gap));

        /* prepare layer3/layer4 info */
        memcpy(&sfq_parse_result.iph, iph_sfq, sizeof(struct iphdr));
        if (iph_sfq->protocol == IPPROTO_TCP) {
            th_sfq = (struct tcphdr *)(skb->data + ETH_HLEN + (sfq_parse_result.vlan1_gap) + (iph_sfq->ihl * 4));
            memcpy(&sfq_parse_result.th, th_sfq, sizeof(struct tcphdr));
#if (sfq_debug)
            tcp_source_port = ntohs(sfq_parse_result.th.source);
            udp_source_port = 0;
            /* tcp ack packet */
            if (ntohl(sfq_parse_result.iph.saddr) == 0xa0a0a04)
                ack_packt = 1;
            else
                ack_packt = 0;
#endif
            sfq_prot = 2;   /* IPV4_HNAPT */
            if (iph_sfq->frag_off & htons(IP_MF | IP_OFFSET))
                return 1;
        } else if (iph_sfq->protocol == IPPROTO_UDP) {
            uh_sfq = (struct udphdr *)(skb->data + ETH_HLEN + (sfq_parse_result.vlan1_gap) + iph_sfq->ihl * 4);
            memcpy(&sfq_parse_result.uh, uh_sfq, sizeof(struct udphdr));
#if (sfq_debug)
            udp_source_port = ntohs(sfq_parse_result.uh.source);
            tcp_source_port = 0;
            ack_packt = 0;
#endif
            sfq_prot = 2;   /* IPV4_HNAPT */
            if (iph_sfq->frag_off & htons(IP_MF | IP_OFFSET))
                return 1;
        } else {
            sfq_prot = 1;
        }
    } else if (sfq_parse_result.eth_type == htons(ETH_P_IPV6) || (sfq_parse_result.eth_type == htons(ETH_P_PPP_SES) && sfq_parse_result.ppp_tag == htons(PPP_IPV6))) {
        ip6h_sfq = (struct ipv6hdr *)(skb->data + ETH_HLEN + (sfq_parse_result.vlan1_gap));
        if (ip6h_sfq->nexthdr == NEXTHDR_TCP) {
            sfq_prot = 4;   /* IPV6_5T */
#if (sfq_debug)
            if (ntohl(sfq_parse_result.ip6h.saddr.s6_addr32[3]) ==
                8)
                ack_packt = 1;
            else
                ack_packt = 0;
#endif
        } else if (ip6h_sfq->nexthdr == NEXTHDR_UDP) {
#if (sfq_debug)
            ack_packt = 0;
#endif
            sfq_prot = 4;   /* IPV6_5T */

        } else {
            sfq_prot = 3;   /* IPV6_3T */
        }
    }
    return 0;
}



static int MDrv_NOE_QDMA_Eth_Send(struct END_DEVICE *ei_local, struct net_device *dev, struct sk_buff *skb, int gmac_no, int ring_no)
{
    unsigned int length = skb->len;
    struct QDMA_txdesc *cpu_ptr, *prev_cpu_ptr;
    struct QDMA_txdesc dummy_desc;
    struct PSEUDO_ADAPTER *p_ad;
    unsigned long flags;
    unsigned int next_txd_idx, qidx;

    cpu_ptr = &dummy_desc;
    /* 2. prepare data */
    dma_sync_single_for_device(&ei_local->qdma_pdev->dev, virt_to_phys(skb->data), skb->len, DMA_TO_DEVICE);
    /* cpu_ptr->txd_info1.SDP = VIRT_TO_PHYS(skb->data); */
    cpu_ptr->txd_info1.SDP = virt_to_phys(skb->data);
    cpu_ptr->txd_info3.SDL = skb->len;
    cpu_ptr->txd_info4.SDL = ((skb->len) >> 14);
    if (ei_local->features & FE_HW_SFQ) {
        _MDrv_NOE_QDMA_SFQ_Parse_Net_Layer(skb);
        cpu_ptr->txd_info4.VQID0 = 1;   /* 1:HW hash 0:CPU */
        cpu_ptr->txd_info3.PROT = sfq_prot;
        /* no vlan */
        cpu_ptr->txd_info3.IPOFST = 14 + (sfq_parse_result.vlan1_gap);
    }
    cpu_ptr->txd_info4.FPORT = gmac_no;

    if (ei_local->features & FE_CSUM_OFFLOAD) {
        if (skb->ip_summed == CHECKSUM_PARTIAL)
            cpu_ptr->txd_info4.TUI_CO = 7;
        else
            cpu_ptr->txd_info4.TUI_CO = 0;
    }

    if (ei_local->features & FE_HW_VLAN_TX) {
        if (skb_vlan_tag_present(skb)) {
            cpu_ptr->txd_info4.VLAN_TAG = 0x10000 | skb_vlan_tag_get(skb);
        } else {
            cpu_ptr->txd_info4.VLAN_TAG = 0;
        }
    }
    cpu_ptr->txd_info4.QID = 0;
    cpu_ptr->txd_info3.QID = ring_no;

    if ((ei_local->features & QDMA_QOS_MARK) && (skb->mark != 0)) {
        if (skb->mark < 64) {
            qidx = M2Q_table[skb->mark];
            cpu_ptr->txd_info4.QID = ((qidx & 0x30) >> 4);
            cpu_ptr->txd_info3.QID = (qidx & 0x0f);
        } else {
            NOE_MSG_ERR("skb->mark out of range\n");
            cpu_ptr->txd_info3.QID = 0;
            cpu_ptr->txd_info4.QID = 0;
        }
    }
    /* QoS Web UI used */

    if ((ei_local->features & QDMA_QOS_WEB) && (lan_wan_separate == 1)) {
        if (web_sfq_enable == 1 && (skb->mark == 2)) {
            if (gmac_no == 1)
                cpu_ptr->txd_info3.QID = HW_SFQ_DL;
            else
                cpu_ptr->txd_info3.QID = HW_SFQ_UP;
        } else if (gmac_no == 2) {
            cpu_ptr->txd_info3.QID += 8;
        }
    }

#ifdef CONFIG_NOE_NAT_HW
    if (FOE_MAGIC_TAG(skb) == FOE_MAGIC_PPE) {
        if (IS_VALID_NOE_HWNAT_HOOK_RX) {
            cpu_ptr->txd_info4.FPORT = 4;   /* PPE */
            FOE_MAGIC_TAG(skb) = 0;
        }
    }
#endif

    /* dma_sync_single_for_device(NULL, virt_to_phys(skb->data), */
    /* skb->len, DMA_TO_DEVICE); */
    cpu_ptr->txd_info3.SWC_bit = 1;

    /* 5. move CPU_PTR to new TXD */
    cpu_ptr->txd_info4.TSO = 0;
    cpu_ptr->txd_info3.LS_bit = 1;
    cpu_ptr->txd_info3.OWN_bit = 0;
    next_txd_idx = _MDrv_NOE_QDMA_Get_Free_Txd(ei_local, ring_no);
    cpu_ptr->txd_info2.NDP = _MDrv_NOE_QDMA_Get_Txd_Phy_Adr(ei_local, next_txd_idx);
    spin_lock_irqsave(&ei_local->page_lock, flags);
    prev_cpu_ptr = ei_local->txd_pool + ei_local->tx_cpu_idx;
    /* update skb_free */
    ei_local->skb_free[ei_local->tx_cpu_idx] = skb;
    /* update tx cpu idx */
    ei_local->tx_cpu_idx = next_txd_idx;
    /* update txd info */
    prev_cpu_ptr->txd_info1 = dummy_desc.txd_info1;
    prev_cpu_ptr->txd_info2 = dummy_desc.txd_info2;
    prev_cpu_ptr->txd_info4 = dummy_desc.txd_info4;
    prev_cpu_ptr->txd_info3 = dummy_desc.txd_info3;
    Chip_Flush_MIU_Pipe(); //L3
    /* NOTE: add memory barrier to avoid
     * DMA access memory earlier than memory written
     */
    wmb();
    /* update CPU pointer */
    MHal_NOE_QDMA_Update_Tx(E_NOE_QDMA_TX_FORWARD, _MDrv_NOE_QDMA_Get_Txd_Phy_Adr(ei_local, ei_local->tx_cpu_idx));
    spin_unlock_irqrestore(&ei_local->page_lock, flags);

    if (ei_local->features & FE_GE2_SUPPORT) {
        if (gmac_no == 2) {
            if (ei_local->pseudo_dev) {
                p_ad = netdev_priv(ei_local->pseudo_dev);
                p_ad->stat.tx_packets++;

                p_ad->stat.tx_bytes += length;
            }
        } else {
            ei_local->stat.tx_packets++;
            ei_local->stat.tx_bytes += skb->len;
        }
    } else {
        ei_local->stat.tx_packets++;
        ei_local->stat.tx_bytes += skb->len;
    }
    if (ei_local->features & FE_INT_NAPI) {
        if (ei_local->tx_full == 1) {
            ei_local->tx_full = 0;
            netif_wake_queue(dev);
        }
    }

    return length;
}

static int MDrv_NOE_QDMA_Eth_Tso_Send(struct END_DEVICE *ei_local, struct net_device *dev, struct sk_buff *skb, int gmac_no, int ring_no)
{
    unsigned int length = skb->len;
    struct QDMA_txdesc *cpu_ptr, *prev_cpu_ptr;
    struct QDMA_txdesc dummy_desc;
    struct QDMA_txdesc init_dummy_desc;
    int ctx_idx;
    struct iphdr *iph = NULL;
    struct QDMA_txdesc *init_cpu_ptr;
    struct tcphdr *th = NULL;
    struct skb_frag_struct *frag;
    unsigned int nr_frags = skb_shinfo(skb)->nr_frags;
    unsigned int len, size, offset, frag_txd_num, qidx;
    unsigned long flags;
    int i;
    int init_qid, init_qid1;
    struct ipv6hdr *ip6h = NULL;
    struct PSEUDO_ADAPTER *p_ad;

    init_cpu_ptr = &init_dummy_desc;
    cpu_ptr = &init_dummy_desc;

    len = length - skb->data_len;
    dma_sync_single_for_device(&ei_local->qdma_pdev->dev, virt_to_phys(skb->data), len, DMA_TO_DEVICE);
    offset = virt_to_phys(skb->data);
    cpu_ptr->txd_info1.SDP = offset;
    if (len > MAX_QTXD_LEN) {
        cpu_ptr->txd_info3.SDL = 0x3FFF;
        cpu_ptr->txd_info4.SDL = 0x3;
        cpu_ptr->txd_info3.LS_bit = 0;
        len -= MAX_QTXD_LEN;
        offset += MAX_QTXD_LEN;
    } else {
        cpu_ptr->txd_info3.SDL = (len & 0x3FFF);
        cpu_ptr->txd_info4.SDL = len >> 14;
        cpu_ptr->txd_info3.LS_bit = nr_frags ? 0 : 1;
        len = 0;
    }
    if (ei_local->features & FE_HW_SFQ) {
        _MDrv_NOE_QDMA_SFQ_Parse_Net_Layer(skb);

        cpu_ptr->txd_info4.VQID0 = 1;
        cpu_ptr->txd_info3.PROT = sfq_prot;
        /* no vlan */
        cpu_ptr->txd_info3.IPOFST = 14 + (sfq_parse_result.vlan1_gap);
    }
    if (gmac_no == 1)
        cpu_ptr->txd_info4.FPORT = 1;
    else
        cpu_ptr->txd_info4.FPORT = 2;

    cpu_ptr->txd_info4.TSO = 0;
    cpu_ptr->txd_info4.QID = 0;
    cpu_ptr->txd_info3.QID = ring_no;
    if ((ei_local->features & QDMA_QOS_MARK) && (skb->mark != 0)) {
        if (skb->mark < 64) {
            qidx = M2Q_table[skb->mark];
            cpu_ptr->txd_info4.QID = ((qidx & 0x30) >> 4);
            cpu_ptr->txd_info3.QID = (qidx & 0x0f);
        } else {
            NOE_MSG_DUMP("skb->mark out of range\n");
            cpu_ptr->txd_info3.QID = 0;
            cpu_ptr->txd_info4.QID = 0;
        }
    }

    if (ei_local->features & FE_CSUM_OFFLOAD) {
        if (skb->ip_summed == CHECKSUM_PARTIAL)
            cpu_ptr->txd_info4.TUI_CO = 7;
        else
            cpu_ptr->txd_info4.TUI_CO = 0;
    }

    if (ei_local->features & FE_HW_VLAN_TX) {
        if (skb_vlan_tag_present(skb)) {
            cpu_ptr->txd_info4.VLAN_TAG = 0x10000 | skb_vlan_tag_get(skb);
        } else {
            cpu_ptr->txd_info4.VLAN_TAG = 0;
        }
    }
    if ((ei_local->features & FE_GE2_SUPPORT) && (lan_wan_separate == 1)) {
        if (web_sfq_enable == 1 && (skb->mark == 2)) {
            if (gmac_no == 1)
                cpu_ptr->txd_info3.QID = HW_SFQ_DL;
            else
                cpu_ptr->txd_info3.QID = HW_SFQ_UP;
        } else if (gmac_no == 2) {
            cpu_ptr->txd_info3.QID += 8;
        }
    }

    /*debug multi tx queue */
    init_qid = cpu_ptr->txd_info3.QID;
    init_qid1 = cpu_ptr->txd_info4.QID;

#ifdef CONFIG_NOE_NAT_HW
    if (FOE_MAGIC_TAG(skb) == FOE_MAGIC_PPE) {
        if (IS_VALID_NOE_HWNAT_HOOK_RX) {
            cpu_ptr->txd_info4.FPORT = 4;   /* PPE */
            FOE_MAGIC_TAG(skb) = 0;
        }
    }
#endif

    cpu_ptr->txd_info3.SWC_bit = 1;

    ctx_idx = _MDrv_NOE_QDMA_Get_Free_Txd(ei_local, ring_no);
    cpu_ptr->txd_info2.NDP = _MDrv_NOE_QDMA_Get_Txd_Phy_Adr(ei_local, ctx_idx);
    /*prev_cpu_ptr->txd_info1 = dummy_desc.txd_info1;
     *prev_cpu_ptr->txd_info2 = dummy_desc.txd_info2;
     *prev_cpu_ptr->txd_info3 = dummy_desc.txd_info3;
     *prev_cpu_ptr->txd_info4 = dummy_desc.txd_info4;
     */
    if (len > 0) {
        frag_txd_num = _MDrv_NOE_QDMA_Calc_Frag_Txd_Num(len);
        for (frag_txd_num = frag_txd_num; frag_txd_num > 0;
             frag_txd_num--) {
            if (len < MAX_QTXD_LEN)
                size = len;
            else
                size = MAX_QTXD_LEN;

            cpu_ptr = (ei_local->txd_pool + (ctx_idx));
            dummy_desc.txd_info1 = cpu_ptr->txd_info1;
            dummy_desc.txd_info2 = cpu_ptr->txd_info2;
            dummy_desc.txd_info3 = cpu_ptr->txd_info3;
            dummy_desc.txd_info4 = cpu_ptr->txd_info4;
            prev_cpu_ptr = cpu_ptr;
            cpu_ptr = &dummy_desc;
            cpu_ptr->txd_info3.QID = init_qid;
            cpu_ptr->txd_info4.QID = init_qid1;
            cpu_ptr->txd_info1.SDP = offset;
            cpu_ptr->txd_info3.SDL = (size & 0x3FFF);
            cpu_ptr->txd_info4.SDL = size >> 14;
            if ((nr_frags == 0) && (frag_txd_num == 1))
                cpu_ptr->txd_info3.LS_bit = 1;
            else
                cpu_ptr->txd_info3.LS_bit = 0;
            cpu_ptr->txd_info3.OWN_bit = 0;
            cpu_ptr->txd_info3.SWC_bit = 1;
            if (cpu_ptr->txd_info3.LS_bit == 1)
                ei_local->skb_free[ctx_idx] = skb;
            else
                ei_local->skb_free[ctx_idx] = magic_id;
            ctx_idx = _MDrv_NOE_QDMA_Get_Free_Txd(ei_local, ring_no);
            cpu_ptr->txd_info2.NDP = _MDrv_NOE_QDMA_Get_Txd_Phy_Adr(ei_local, ctx_idx);
            prev_cpu_ptr->txd_info1 = dummy_desc.txd_info1;
            prev_cpu_ptr->txd_info2 = dummy_desc.txd_info2;
            prev_cpu_ptr->txd_info3 = dummy_desc.txd_info3;
            prev_cpu_ptr->txd_info4 = dummy_desc.txd_info4;
            offset += size;
            len -= size;
        }
    }

    for (i = 0; i < nr_frags; i++) {
        /* 1. set or get init value for current fragment */
        offset = 0;
        frag = &skb_shinfo(skb)->frags[i];
        len = frag->size;
        frag_txd_num = _MDrv_NOE_QDMA_Calc_Frag_Txd_Num(len);
        for (frag_txd_num = frag_txd_num;
             frag_txd_num > 0; frag_txd_num--) {
            /* 2. size will be assigned to SDL
             * and can't be larger than MAX_TXD_LEN
             */
            if (len < MAX_QTXD_LEN)
                size = len;
            else
                size = MAX_QTXD_LEN;

            /* 3. Update TXD info */
            cpu_ptr = (ei_local->txd_pool + (ctx_idx));
            dummy_desc.txd_info1 = cpu_ptr->txd_info1;
            dummy_desc.txd_info2 = cpu_ptr->txd_info2;
            dummy_desc.txd_info3 = cpu_ptr->txd_info3;
            dummy_desc.txd_info4 = cpu_ptr->txd_info4;
            prev_cpu_ptr = cpu_ptr;
            cpu_ptr = &dummy_desc;
            cpu_ptr->txd_info3.QID = init_qid;
            cpu_ptr->txd_info4.QID = init_qid1;
            cpu_ptr->txd_info1.SDP = dma_map_page(&ei_local->qdma_pdev->dev, frag->page.p, frag->page_offset + offset, size, DMA_TO_DEVICE);
            if (unlikely(dma_mapping_error (&ei_local->qdma_pdev->dev, cpu_ptr->txd_info1.SDP))) {
                NOE_MSG_ERR("[%s] dma_map_page() failed...\n", __func__);
            }

            cpu_ptr->txd_info3.SDL = (size & 0x3FFF);
            cpu_ptr->txd_info4.SDL = size >> 14;
            cpu_ptr->txd_info4.SDL = (size >> 14);
            if ((i == (nr_frags - 1)) && (frag_txd_num == 1))
                cpu_ptr->txd_info3.LS_bit = 1;
            else
                cpu_ptr->txd_info3.LS_bit = 0;
            cpu_ptr->txd_info3.OWN_bit = 0;
            cpu_ptr->txd_info3.SWC_bit = 1;
            /* 4. Update skb_free for housekeeping */
            if (cpu_ptr->txd_info3.LS_bit == 1)
                ei_local->skb_free[ctx_idx] = skb;
            else
                ei_local->skb_free[ctx_idx] = magic_id;

            /* 5. Get next TXD */
            ctx_idx = _MDrv_NOE_QDMA_Get_Free_Txd(ei_local, ring_no);
            cpu_ptr->txd_info2.NDP = _MDrv_NOE_QDMA_Get_Txd_Phy_Adr(ei_local, ctx_idx);
            prev_cpu_ptr->txd_info1 = dummy_desc.txd_info1;
            prev_cpu_ptr->txd_info2 = dummy_desc.txd_info2;
            prev_cpu_ptr->txd_info3 = dummy_desc.txd_info3;
            prev_cpu_ptr->txd_info4 = dummy_desc.txd_info4;
            Chip_Flush_MIU_Pipe(); //L3
            /* 6. Update offset and len. */
            offset += size;
            len -= size;
        }
    }

    if (skb_shinfo(skb)->gso_segs > 1) {
        MDrv_NOE_Update_Tso_Len(skb->len);
        /* TCP over IPv4 */
        iph = (struct iphdr *)skb_network_header(skb);
        if ((iph->version == 4) && (iph->protocol == IPPROTO_TCP)) {
            th = (struct tcphdr *)skb_transport_header(skb);
            init_cpu_ptr->txd_info4.TSO = 1;
            th->check = htons(skb_shinfo(skb)->gso_size);
            dma_sync_single_for_device(&ei_local->qdma_pdev->dev, virt_to_phys(th), sizeof(struct tcphdr), DMA_TO_DEVICE);
        }

        if (ei_local->features & FE_TSO_V6) {
            ip6h = (struct ipv6hdr *)skb_network_header(skb);
            if ((ip6h->version == 6) && (ip6h->nexthdr == NEXTHDR_TCP)) {
                th = (struct tcphdr *)skb_transport_header(skb);
                init_cpu_ptr->txd_info4.TSO = 1;
                th->check = htons(skb_shinfo(skb)->gso_size);

                dma_sync_single_for_device(NULL, virt_to_phys(th), sizeof(struct tcphdr), DMA_TO_DEVICE);
            }
        }
        if (ei_local->features & FE_HW_SFQ) {
            init_cpu_ptr->txd_info4.VQID0 = 1;
            init_cpu_ptr->txd_info3.PROT = sfq_prot;
            /* no vlan */
            init_cpu_ptr->txd_info3.IPOFST = 14 + (sfq_parse_result.vlan1_gap);
        }
    }
    /* dma_cache_sync(NULL, skb->data, skb->len, DMA_TO_DEVICE); */

    init_cpu_ptr->txd_info3.OWN_bit = 0;
    spin_lock_irqsave(&ei_local->page_lock, flags);
    prev_cpu_ptr = ei_local->txd_pool + ei_local->tx_cpu_idx;
    ei_local->skb_free[ei_local->tx_cpu_idx] = magic_id;
    ei_local->tx_cpu_idx = ctx_idx;
    prev_cpu_ptr->txd_info1 = init_dummy_desc.txd_info1;
    prev_cpu_ptr->txd_info2 = init_dummy_desc.txd_info2;
    prev_cpu_ptr->txd_info4 = init_dummy_desc.txd_info4;
    prev_cpu_ptr->txd_info3 = init_dummy_desc.txd_info3;
    Chip_Flush_MIU_Pipe(); //L3

    /* NOTE: add memory barrier to avoid
     * DMA access memory earlier than memory written
     */
    wmb();
    MHal_NOE_QDMA_Update_Tx(E_NOE_QDMA_TX_FORWARD,_MDrv_NOE_QDMA_Get_Txd_Phy_Adr(ei_local, ei_local->tx_cpu_idx));
    spin_unlock_irqrestore(&ei_local->page_lock, flags);

    if (ei_local->features & FE_GE2_SUPPORT) {
        if (gmac_no == 2) {
            if (ei_local->pseudo_dev) {
                p_ad = netdev_priv(ei_local->pseudo_dev);
                p_ad->stat.tx_packets++;
                p_ad->stat.tx_bytes += length;
            }
        } else {
            ei_local->stat.tx_packets++;
            ei_local->stat.tx_bytes += skb->len;
        }
    } else {
        ei_local->stat.tx_packets++;
        ei_local->stat.tx_bytes += skb->len;
    }
    if (ei_local->features & FE_INT_NAPI) {
        if (ei_local->tx_full == 1) {
            ei_local->tx_full = 0;
            netif_wake_queue(dev);
        }
    }

    return length;
}

/* QDMA functions */


int MDrv_NOE_QDMA_Init_Rx(struct net_device *dev)
{
    int i;
    struct noe_dma_info dma_info;
    struct END_DEVICE *ei_local = netdev_priv(dev);

    /* Initial QDMA RX Ring */
#if 0
    ei_local->qrx_ring = dma_alloc_coherent(&ei_local->qdma_pdev->dev, NUM_QRX_DESC * sizeof(struct PDMA_rxdesc), &ei_local->phy_qrx_ring, GFP_ATOMIC | __GFP_ZERO);
#else
{
     MSYS_DMEM_INFO mem_info;
     int ret;
     mem_info.length = NUM_QRX_DESC * sizeof(struct PDMA_rxdesc);
     strcpy(mem_info.name, "qrx_ring");
     if((ret=msys_request_dmem(&mem_info)))
     {
         NOE_MSG_ERR("unable to allocate DMEM for  %s!! error=%d\n", mem_info.name ,ret);
     }
     ei_local->phy_qrx_ring = (dma_addr_t)((size_t)mem_info.phys);
     ei_local->qrx_ring = (void *)((size_t)mem_info.kvirt);
}
#endif


    if(ei_local->qrx_ring == NULL)
    {
        NOE_MSG_ERR("[%s][%d]dma alloc FAIL!! %u\n",__FUNCTION__,__LINE__, NUM_QRX_DESC*sizeof(struct PDMA_rxdesc));
        goto no_rx_mem;
    }

    for (i = 0; i < NUM_QRX_DESC; i++) {
        ei_local->netrx0_skbuf[i] = netdev_alloc_skb(dev, MAX_RX_LENGTH + NET_IP_ALIGN);
        if (!ei_local->netrx0_skbuf[i]) {
            NOE_MSG_ERR("rx skbuff buffer allocation failed!");
            goto no_rx_mem;
        }

        memset(&ei_local->qrx_ring[i], 0, sizeof(struct PDMA_rxdesc));
        ei_local->qrx_ring[i].rxd_info2.DDONE_bit = 0;
        ei_local->qrx_ring[i].rxd_info2.LS0 = 0;
        ei_local->qrx_ring[i].rxd_info2.PLEN0 = MAX_RX_LENGTH;
        ei_local->qrx_ring[i].rxd_info1.PDP0 = dma_map_single(&ei_local->qdma_pdev->dev, ei_local->netrx0_skbuf[i]->data, MAX_RX_LENGTH + NET_IP_ALIGN, PCI_DMA_FROMDEVICE);
        NOE_MSG_DUMP("%p ,PDP0 = 0x%X \n", ei_local->netrx0_skbuf[i]->data, ei_local->qrx_ring[i].rxd_info1.PDP0);
        if (unlikely(dma_mapping_error(ei_local->dev, ei_local->qrx_ring[i].rxd_info1.PDP0))) {
            NOE_MSG_ERR("[%s]dma_map_single() failed...\n", __func__);
            goto no_rx_mem;
        }
    }
    Chip_Flush_MIU_Pipe(); //L3

    NOE_MSG_DBG("\n[QUE_RX] phy = 0x%p, vir = 0x%p\n", (void *)ei_local->phy_qrx_ring, ei_local->qrx_ring);

    /* Tell the adapter where the RX rings are located. */
    dma_info.ring_st.base_adr = ((MS_U32)ei_local->phy_qrx_ring);
    dma_info.ring_st.max_cnt = cpu_to_le32((MS_U32)NUM_QRX_DESC);
    dma_info.ring_st.cpu_idx = cpu_to_le32((MS_U32)(NUM_QRX_DESC - 1));

    MHal_NOE_DMA_Init(E_NOE_DMA_QUEUE, E_NOE_DIR_RX, E_NOE_RING_NO0, &dma_info);
    ei_local->rx_ring[0] = ei_local->qrx_ring;

    return 0;

no_rx_mem:
    return -ENOMEM;
}

int MDrv_NOE_QDMA_Init_Tx(struct net_device *dev)
{
    bool pass;
    struct END_DEVICE *ei_local = netdev_priv(dev);

    if (ei_local->features & FE_HW_SFQ)
        _MDrv_NOE_QDMA_SFQ_Init(dev);

    /*tx desc alloc, add a NULL TXD to HW */
    pass = _MDrv_NOE_QDMA_Alloc_Txd(dev);
    if (!pass) {
        NOE_MSG_ERR("fail to allocate qdma txd!\n");
        return -1;
    }

    pass = _MDrv_NOE_QDMA_Init_Fq(dev);
    if (!pass) {
        NOE_MSG_ERR("fail to allocate qdma free desc!\n");
        return -1;
    }

    return 0;
}

void MDrv_NOE_QDMA_Deinit_Rx(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    int i;

    /* free RX Ring */
#if 0
    if (ei_local->qrx_ring)
        dma_free_coherent(NULL, NUM_QRX_DESC * sizeof(struct PDMA_rxdesc), ei_local->qrx_ring, ei_local->phy_qrx_ring);
#endif
    /* free RX data */
    for (i = 0; i < NUM_RX_DESC; i++) {
        if (ei_local->netrx0_skbuf[i]) {
            dev_kfree_skb_any(ei_local->netrx0_skbuf[i]);
            ei_local->netrx0_skbuf[i] = NULL;
        }
    }
}

void MDrv_NOE_QDMA_Deinit_Tx(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    int i;

    /* free TX Ring */
#if 0
    if (ei_local->txd_pool)
        dma_free_coherent(&ei_local->qdma_pdev->dev, NUM_TX_DESC * QTXD_LEN, ei_local->txd_pool, ei_local->phy_txd_pool);
    if (ei_local->free_head)
        dma_free_coherent(&ei_local->qdma_pdev->dev, NUM_QDMA_PAGE * QTXD_LEN, ei_local->free_head, ei_local->phy_free_head);
    if (ei_local->free_page_head)
        dma_free_coherent(&ei_local->qdma_pdev->dev, NUM_QDMA_PAGE * QDMA_PAGE_SIZE, ei_local->free_page_head, ei_local->phy_free_page_head);
#endif

    /* free TX data */
    for (i = 0; i < NUM_TX_DESC; i++) {
        if ((ei_local->skb_free[i] != (struct sk_buff *)0xFFFFFFFF) && (ei_local->skb_free[i] != 0))
            dev_kfree_skb_any(ei_local->skb_free[i]);
    }
}



int MDrv_NOE_QDMA_Start_Xmit(struct sk_buff *skb, struct net_device *dev, int gmac_no)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    unsigned int num_of_txd = 0;
    unsigned int nr_frags = skb_shinfo(skb)->nr_frags, i;
    struct skb_frag_struct *frag;
    struct PSEUDO_ADAPTER *p_ad;
    int ring_no;

    ring_no = skb->queue_mapping + (gmac_no - 1) * GMAC1_TXQ_NUM;
#ifdef CONFIG_NOE_NAT_HW
    if (IS_VALID_NOE_HWNAT_HOOK_TX) {
        if (NOE_HWNAT_HOOK_TX(skb, gmac_no) != 1) {
            dev_kfree_skb_any(skb);
            return 0;
        }
    }
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
    dev->trans_start = jiffies; /* save the timestamp */
#else
    {
        struct netdev_queue *txq = netdev_get_tx_queue(dev, 0);

        if (txq->trans_start != jiffies)
            txq->trans_start = jiffies;
    }
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0) */

    /*spin_lock_irqsave(&ei_local->page_lock, flags); */

    /* check free_txd_num before calling rt288_eth_send() */

    if (ei_local->features & FE_TSO) {
        num_of_txd += _MDrv_NOE_QDMA_Calc_Frag_Txd_Num(skb->len - skb->data_len);
        if (nr_frags != 0) {
            for (i = 0; i < nr_frags; i++) {
                frag = &skb_shinfo(skb)->frags[i];
                num_of_txd += _MDrv_NOE_QDMA_Calc_Frag_Txd_Num(frag->size);
            }
        }
    } else {
        num_of_txd = 1;
    }

    if (likely(atomic_read(&ei_local->free_txd_num[ring_no]) > (num_of_txd + 1))) {
        if (num_of_txd == 1)
            MDrv_NOE_QDMA_Eth_Send(ei_local, dev, skb, gmac_no, ring_no);
        else
            MDrv_NOE_QDMA_Eth_Tso_Send(ei_local, dev, skb, gmac_no, ring_no);
    } else {
        if (ei_local->features & FE_GE2_SUPPORT) {
            if (gmac_no == 2) {
                if (ei_local->pseudo_dev) {
                    p_ad = netdev_priv(ei_local->pseudo_dev);
                    p_ad->stat.tx_dropped++;
                }
            } else {
                ei_local->stat.tx_dropped++;
            }
        } else {
            ei_local->stat.tx_dropped++;
        }
        /* kfree_skb(skb); */
        dev_kfree_skb_any(skb);
        /* spin_unlock_irqrestore(&ei_local->page_lock, flags); */
        return 0;
    }
    /* spin_unlock_irqrestore(&ei_local->page_lock, flags); */
    return 0;
}

int MDrv_NOE_QDMA_Xmit_Housekeeping(struct net_device *netdev, int budget)
{
    struct END_DEVICE *ei_local = netdev_priv(netdev);

    dma_addr_t dma_ptr;
    struct QDMA_txdesc *cpu_ptr = NULL;
    unsigned int tmp_offset = 0;
    unsigned int ctx_offset = 0;
    unsigned int dtx_offset = 0;
    unsigned int rls_cnt[TOTAL_TXQ_NUM] = { 0 };
    int ring_no;
    int i;
    dma_addr_t tmp_ptr;
    //unsigned long u32VAddr = (unsigned long)ei_local->txd_pool;
    //unsigned long u32Size = NUM_TX_DESC * QTXD_LEN;

    dma_ptr = (dma_addr_t)MHal_NOE_QDMA_Get_Tx();
    ctx_offset = ei_local->rls_cpu_idx;
    dtx_offset = (dma_ptr - ei_local->phy_txd_pool) / QTXD_LEN;
    cpu_ptr = (ei_local->txd_pool + (ctx_offset));

    //Chip_Inv_Cache_Range(u32VAddr, u32Size);
#ifdef CONFIG_NOE_FLUSH_MEM
    Chip_Flush_Cache_Range(u32VAddr, u32Size);
#endif

    while (ctx_offset != dtx_offset) {
        /* 1. keep cpu next TXD */
        if (MHAL_NOE_IS_TXD_NOT_AVAILABLE(cpu_ptr)) {
            break;
        }
        tmp_ptr = (dma_addr_t)cpu_ptr->txd_info2.NDP;
        if (tmp_ptr == 0)
            break;
        /* atomic_add(1, &ei_local->free_txd_num[ring_no]); */
        /* 3. update ctx_offset and free skb memory */
        tmp_offset = ctx_offset;
        ctx_offset = (tmp_ptr - ei_local->phy_txd_pool) / QTXD_LEN;

        if (ei_local->features & FE_TSO) {
            if (ei_local->skb_free[ctx_offset] != magic_id) {
                if (ei_local->skb_free[ctx_offset] != NULL) {
                    dev_kfree_skb_any(ei_local->skb_free[ctx_offset]);
                }
                else {
                    ctx_offset = tmp_offset;
                    break;
                }
            }
        }
        else {
            if (ei_local->skb_free[ctx_offset] != NULL) {
                dev_kfree_skb_any(ei_local->skb_free[ctx_offset]);
            }
            else {
                ctx_offset = tmp_offset;
                break;
            }
        }
        /* 3. release TXD */
        ring_no = _MDrv_NOE_QDMA_Map_Ring_No(tmp_offset);
        rls_cnt[ring_no]++;
        _MDrv_NOE_QDMA_Put_Back_Free_Txd(ei_local, ring_no, tmp_offset);
        ei_local->skb_free[ctx_offset] = 0;
        cpu_ptr->txd_info1.SDP = 0;
        /* 4. update cpu_ptr */
        cpu_ptr = (ei_local->txd_pool + ctx_offset);
        Chip_Flush_MIU_Pipe(); //L3
    }

    for (i = 0; i < TOTAL_TXQ_NUM; i++) {
        if (rls_cnt[i] > 0)
            atomic_add(rls_cnt[i], &ei_local->free_txd_num[i]);
        if (atomic_read(&ei_local->free_txd_num[i]) < ei_local->stats.min_free_txd[i])
            ei_local->stats.min_free_txd[i] = atomic_read(&ei_local->free_txd_num[i]);
    }
    ei_local->rls_cpu_idx = ctx_offset;
    netif_wake_queue(netdev);
    if (ei_local->features & FE_GE2_SUPPORT)
        netif_wake_queue(ei_local->pseudo_dev);
    ei_local->tx_ring_full = 0;
    MHal_NOE_QDMA_Update_Tx(E_NOE_QMDA_TX_RELEASE, (ei_local->phy_txd_pool + (ctx_offset * QTXD_LEN)));

    return 0;
}

int MDrv_NOE_QDMA_Ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    struct noe_reg reg;
#ifdef NOE_IOCTL_VERSION_4
    unsigned int page = 0, pq_no;
#endif
    int ret = 0;
    struct END_DEVICE *ei_local = netdev_priv(dev);

    spin_lock_irq(&ei_local->page_lock);

    switch (cmd) {
    case NOE_QDMA_REG_READ:
        ret = copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
        if (ret) {
            ret = -EFAULT;
            break;
        }
        if (reg.off > REG_HQOS_MAX) {
            ret = -EINVAL;
            break;
        }
#ifdef NOE_IOCTL_VERSION_4
        /* q16~q31: 0x100 <= reg.off < 0x200
         * q32~q47: 0x200 <= reg.off < 0x300
         * q48~q63: 0x300 <= reg.off < 0x400
         */
        if (reg.off >= 0x100 && reg.off < 0x200) {
            page = 1;
            reg.off = reg.off - 0x100;
            pq_no = 16 + reg.off / 16;
        } else if (reg.off >= 0x200 && reg.off < 0x300) {
            page = 2;
            reg.off = reg.off - 0x200;
            pq_no = 32 + reg.off / 16;
        } else if (reg.off >= 0x300 && reg.off < 0x400) {
            page = 3;
            reg.off = reg.off - 0x300;
            pq_no = 48 + reg.off / 16;
        } else {
            page = 0;
            pq_no = reg.off / 16;
        }

        NOE_MSG_DUMP("page=%d, reg.off =%x\n", page, reg.off);
        reg.val = MHal_NOE_DMA_Get_Queue_Cfg(pq_no);
#else
        reg.val = MHal_NOE_DMA_Get_Queue_Cfg(reg.off);
        NOE_MSG_DUMP("[%s][%d]read reg off:%x val:%x\n",__FUNCTION__,__LINE__, reg.off, reg.val);
#endif
        ret = copy_to_user(ifr->ifr_data, &reg, sizeof(reg));
        if (ret)
            ret = -EFAULT;
        break;
    case NOE_QDMA_REG_WRITE:
        ret = copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
        if (ret) {
            ret = -EFAULT;
            break;
        }
        if (reg.off > REG_HQOS_MAX) {
            ret = -EINVAL;
            break;
        }
#ifdef NOE_IOCTL_VERSION_4
        /* q16~q31: 0x100 <= reg.off < 0x200
         * q32~q47: 0x200 <= reg.off < 0x300
         * q48~q63: 0x300 <= reg.off < 0x400
         */
        if (reg.off >= 0x100 && reg.off < 0x200) {
            page = 1;
            reg.off = reg.off - 0x100;
            pq_no = 16 + reg.off / 16;
        } else if (reg.off >= 0x200 && reg.off < 0x300) {
            page = 2;
            reg.off = reg.off - 0x200;
            pq_no = 32 + reg.off / 16;
        } else if (reg.off >= 0x300 && reg.off < 0x400) {
            page = 3;
            reg.off = reg.off - 0x300;
            pq_no = 48 + reg.off / 16;
        } else {
            page = 0;
            pq_no = reg.off / 16;
        }
        NOE_MSG_DUMP("reg.val =%x\n", reg.val);
        MHal_NOE_DMA_Set_Queue_Cfg(pq_no, reg.val);
        /* NOE_MSG_DUMP("write reg off:%x val:%x\n", reg.off, reg.val); */
#else
        NOE_MSG_DUMP("[%s][%d]reg.off,val =0x%x,0x%x\n",__FUNCTION__,__LINE__,reg.off, reg.val);
        MHal_NOE_DMA_Set_Queue_Cfg(reg.off, reg.val);
        NOE_MSG_DUMP("[%s][%d]write reg off:%x val:%x\n",__FUNCTION__,__LINE__, reg.off, reg.val);
#endif
        break;
    case NOE_QDMA_QUEUE_MAPPING:
        ret = copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
        if (ret) {
            ret = -EFAULT;
            break;
        }
        if ((reg.off & 0x100) == 0x100) {
            lan_wan_separate = 1;
            reg.off &= 0xff;
        } else {
            lan_wan_separate = 0;
        }
        M2Q_table[reg.off] = reg.val;
        break;
    case NOE_QDMA_SFQ_WEB_ENABLE:
        if (ei_local->features & FE_HW_SFQ) {
            ret = copy_from_user(&reg, ifr->ifr_data, sizeof(reg));
            if (ret) {
                ret = -EFAULT;
                break;
            }
            if ((reg.val) == 0x1)
                web_sfq_enable = 1;
            else
                web_sfq_enable = 0;
        } else {
            ret = -EINVAL;
        }
        break;
    default:
        ret = 1;
        break;
    }

    spin_unlock_irq(&ei_local->page_lock);
    return ret;
}
