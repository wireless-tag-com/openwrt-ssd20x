/*
* mdrv_noe_pdma.c- Sigmastar
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
//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------
//  Local Functions
//--------------------------------------------------------------------------------------------------





//--------------------------------------------------------------------------------------------------
//  Local Variable
//--------------------------------------------------------------------------------------------------


int MDrv_NOE_PDMA_Init_Rx(struct net_device *dev)
{
    int i;
    unsigned int skb_size;
    struct END_DEVICE *ei_local = netdev_priv(dev);
    struct noe_dma_info dma_info;

    skb_size = SKB_DATA_ALIGN(MAX_RX_LENGTH + NET_IP_ALIGN + NET_SKB_PAD) + SKB_DATA_ALIGN(sizeof(struct skb_shared_info));

    /* Initial RX Ring 0 */
#if 0
    ei_local->rx_ring[0] = dma_alloc_coherent(dev->dev.parent, NUM_RX_DESC *sizeof(struct PDMA_rxdesc), &ei_local->phy_rx_ring[0], GFP_ATOMIC | __GFP_ZERO);
#else
{
     MSYS_DMEM_INFO mem_info;
     int ret;
     mem_info.length = NUM_RX_DESC *sizeof(struct PDMA_rxdesc);
     strcpy(mem_info.name, "rx_ring0");
     if((ret=msys_request_dmem(&mem_info)))
     {
         NOE_MSG_ERR("unable to allocate DMEM for  %s!! error=%d\n", mem_info.name ,ret);
     }
     ei_local->phy_rx_ring[0] = (dma_addr_t)((size_t)mem_info.phys);
     ei_local->rx_ring[0] = (void *)((size_t)mem_info.kvirt);
}
#endif

    NOE_MSG_DBG("[PKT_RX_DESC] phy_rx_ring[0] = 0x%08x, rx_ring[0] = 0x%p\n", (unsigned int)ei_local->phy_rx_ring[0], (void *)ei_local->rx_ring[0]);

    for (i = 0; i < NUM_RX_DESC; i++) {
        ei_local->netrx_skb_data[0][i] = MDrv_NOE_SkbData_Alloc(skb_size, GFP_KERNEL);
        if (!ei_local->netrx_skb_data[0][i]) {
            NOE_MSG_ERR("rx skbuff buffer allocation failed!");
            goto no_rx_mem;
        }

        memset(&ei_local->rx_ring[0][i], 0, sizeof(struct PDMA_rxdesc));
        ei_local->rx_ring[0][i].rxd_info2.DDONE_bit = 0;
        ei_local->rx_ring[0][i].rxd_info2.LS0 = 0;
        ei_local->rx_ring[0][i].rxd_info2.PLEN0 = MAX_RX_LENGTH;
        ei_local->rx_ring[0][i].rxd_info1.PDP0 = dma_map_single(dev->dev.parent, ei_local->netrx_skb_data[0][i] + NET_SKB_PAD, MAX_RX_LENGTH, DMA_FROM_DEVICE);

        if (unlikely(dma_mapping_error(ei_local->dev, ei_local->rx_ring[0][i].rxd_info1.PDP0))) {
            NOE_MSG_ERR("[%s]dma_map_single() failed...\n", __func__);
            goto no_rx_mem;
        }
        NOE_MSG_DUMP("va=0x%p ,pa =0x%x \n", (ei_local->netrx_skb_data[0][i] + NET_SKB_PAD), ei_local->rx_ring[0][i].rxd_info1.PDP0);
    }
    Chip_Flush_MIU_Pipe(); //L3

    /* Tell the adapter where the RX rings are located. */
    dma_info.ring_st.base_adr = (MS_U32)ei_local->phy_rx_ring[0];
    dma_info.ring_st.max_cnt = cpu_to_le32((u32)NUM_RX_DESC);
    dma_info.ring_st.cpu_idx = cpu_to_le32((u32)(NUM_RX_DESC - 1));

    MHal_NOE_DMA_Init(E_NOE_DMA_PACKET, E_NOE_DIR_RX, E_NOE_RING_NO0, &dma_info);
    return 0;

no_rx_mem:
    return -ENOMEM;
}

int MDrv_NOE_PDMA_Init_Tx(struct net_device *dev)
{
    int i;
    struct END_DEVICE *ei_local = netdev_priv(dev);
    struct noe_dma_info dma_info;
    for (i = 0; i < NUM_TX_DESC; i++)
        ei_local->skb_free[i] = 0;

    ei_local->tx_ring_full = 0;
    ei_local->free_idx = 0;
    ei_local->tx_ring0 = dma_alloc_coherent(dev->dev.parent, NUM_TX_DESC * sizeof(struct PDMA_txdesc), &ei_local->phy_tx_ring0, GFP_ATOMIC | __GFP_ZERO);
    NOE_MSG_DBG("[PKT][TX_DESC] phy_tx_ring = 0x%08x, tx_ring = 0x%p\n", (unsigned int)ei_local->phy_tx_ring0, (void *)ei_local->tx_ring0);

    for (i = 0; i < NUM_TX_DESC; i++) {
        memset(&ei_local->tx_ring0[i], 0, sizeof(struct PDMA_txdesc));
        ei_local->tx_ring0[i].txd_info2.LS0_bit = 1;
        ei_local->tx_ring0[i].txd_info2.DDONE_bit = 1;
    }

    dma_info.ring_st.base_adr = (MS_U32)ei_local->phy_tx_ring0;
    dma_info.ring_st.max_cnt = cpu_to_le32((MS_U32)NUM_TX_DESC);
    dma_info.ring_st.cpu_idx = 0;
    MHal_NOE_DMA_Init(E_NOE_DMA_PACKET, E_NOE_DIR_TX, E_NOE_RING_NO0, &dma_info);

#ifdef CONFIG_NOE_RW_PDMAPTR_FROM_VAR
    ei_local->tx_cpu_owner_idx0 = 0;
#endif
    return 0;
}

void MDrv_NOE_PDMA_Deinit_Rx(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    int i;

    /* free RX Ring */

    //if (ei_local->rx_ring[0])
    //    dma_free_coherent(dev->dev.parent, NUM_RX_DESC * sizeof(struct PDMA_rxdesc), ei_local->rx_ring[0], ei_local->phy_rx_ring[0]);
    /* free RX data */
    for (i = 0; i < NUM_RX_DESC; i++) {
        if (ei_local->netrx_skb_data[0][i]) {
            MDrv_NOE_SkbData_Free(ei_local->netrx_skb_data[0][i]);
            ei_local->netrx_skb_data[0][i] = NULL;
        }
    }
}

void MDrv_NOE_PDMA_Deinit_Tx(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    int i;

    /* free TX Ring */
    if (ei_local->tx_ring0)
        dma_free_coherent(dev->dev.parent, NUM_TX_DESC * sizeof(struct PDMA_txdesc), ei_local->tx_ring0, ei_local->phy_tx_ring0);

    /* free TX data */
    for (i = 0; i < NUM_TX_DESC; i++) {
        if ((ei_local->skb_free[i] != 0) && (ei_local->skb_free[i] != (struct sk_buff *)0xFFFFFFFF))
            dev_kfree_skb_any(ei_local->skb_free[i]);
    }
}



/* @brief cal txd number for a page
 *
 *  @parm size
 *
 *  @return frag_txd_num
 */
static inline unsigned int _MDrv_NOE_PDMA_Calc_Frag_Txd_Num(unsigned int size)
{
    unsigned int frag_txd_num = 0;

    if (size == 0)
        return 0;
    while (size > 0) {
        if (size > MAX_PTXD_LEN) {
            frag_txd_num++;
            size -= MAX_PTXD_LEN;
        } else {
            frag_txd_num++;
            size = 0;
        }
    }
    return frag_txd_num;
}

static int _MDrv_NOE_PDMA_Fill_Txd(struct net_device *dev, unsigned long *tx_cpu_owner_idx, struct sk_buff *skb, int gmac_no)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    struct PDMA_txdesc *tx_ring = &ei_local->tx_ring0[*tx_cpu_owner_idx];
    struct PDMA_TXD_INFO2_T txd_info2_tmp;
    struct PDMA_TXD_INFO4_T txd_info4_tmp;

    tx_ring->txd_info1.SDP0 = virt_to_phys(skb->data);
    txd_info2_tmp.SDL0 = skb->len;
    txd_info4_tmp.FPORT = gmac_no;
    txd_info4_tmp.TSO = 0;

    if (ei_local->features & FE_CSUM_OFFLOAD) {
        if (skb->ip_summed == CHECKSUM_PARTIAL)
            txd_info4_tmp.TUI_CO = 7;
        else
            txd_info4_tmp.TUI_CO = 0;
    }

    if (ei_local->features & FE_HW_VLAN_TX) {
        if (skb_vlan_tag_present(skb))
            txd_info4_tmp.VLAN_TAG = 0x10000 | skb_vlan_tag_get(skb);
        else
            txd_info4_tmp.VLAN_TAG = 0;
    }
#ifdef CONFIG_NOE_NAT_HW
    if (FOE_MAGIC_TAG(skb) == FOE_MAGIC_PPE) {
        if (IS_VALID_NOE_HWNAT_HOOK_RX) {
            /* PPE */
            txd_info4_tmp.FPORT = 4;
            FOE_MAGIC_TAG(skb) = 0;
        }
    }
#endif

    txd_info2_tmp.LS0_bit = 1;
    txd_info2_tmp.DDONE_bit = 0;

    tx_ring->txd_info4 = txd_info4_tmp;
    tx_ring->txd_info2 = txd_info2_tmp;

    return 0;
}

static int _MDrv_NOE_PDMA_Tso_Fill_Data(struct END_DEVICE *ei_local, unsigned int frag_offset, unsigned int frag_size, unsigned long *tx_cpu_owner_idx, unsigned int nr_frags, int gmac_no)
{
    struct PSEUDO_ADAPTER *p_ad;
    unsigned int size;
    unsigned int frag_txd_num;
    struct PDMA_txdesc *tx_ring;

    frag_txd_num = _MDrv_NOE_PDMA_Calc_Frag_Txd_Num(frag_size);
    tx_ring = &ei_local->tx_ring0[*tx_cpu_owner_idx];

    while (frag_txd_num > 0) {
        if (frag_size < MAX_PTXD_LEN)
            size = frag_size;
        else
            size = MAX_PTXD_LEN;

        if (ei_local->skb_txd_num % 2 == 0) {
            *tx_cpu_owner_idx = (*tx_cpu_owner_idx + 1) % NUM_TX_DESC;
            tx_ring = &ei_local->tx_ring0[*tx_cpu_owner_idx];

            while (tx_ring->txd_info2.DDONE_bit == 0) {
                if (gmac_no == 2) {
                    p_ad = netdev_priv(ei_local->pseudo_dev);
                    p_ad->stat.tx_errors++;
                } else {
                    ei_local->stat.tx_errors++;
                }
            }
            tx_ring->txd_info1.SDP0 = frag_offset;
            tx_ring->txd_info2.SDL0 = size;
            if (((nr_frags == 0)) && (frag_txd_num == 1))
                tx_ring->txd_info2.LS0_bit = 1;
            else
                tx_ring->txd_info2.LS0_bit = 0;
            tx_ring->txd_info2.DDONE_bit = 0;
            tx_ring->txd_info4.FPORT = gmac_no;
        } else {
            tx_ring->txd_info3.SDP1 = frag_offset;
            tx_ring->txd_info2.SDL1 = size;
            if (((nr_frags == 0)) && (frag_txd_num == 1))
                tx_ring->txd_info2.LS1_bit = 1;
            else
                tx_ring->txd_info2.LS1_bit = 0;
        }
        frag_offset += size;
        frag_size -= size;
        frag_txd_num--;
        ei_local->skb_txd_num++;
    }

    return 0;
}

static int _MDrv_NOE_PDMA_Tso_Fill_Frag(struct net_device *netdev, struct sk_buff *skb, unsigned long *tx_cpu_owner_idx, int gmac_no)
{
    struct END_DEVICE *ei_local = netdev_priv(netdev);
    struct PSEUDO_ADAPTER *p_ad;
    unsigned int size;
    unsigned int frag_txd_num;
    struct skb_frag_struct *frag;
    unsigned int nr_frags;
    unsigned int frag_offset, frag_size;
    struct PDMA_txdesc *tx_ring;
    int i = 0, j = 0, unmap_idx = 0;

    nr_frags = skb_shinfo(skb)->nr_frags;
    tx_ring = &ei_local->tx_ring0[*tx_cpu_owner_idx];

    for (i = 0; i < nr_frags; i++) {
        frag = &skb_shinfo(skb)->frags[i];
        frag_offset = frag->page_offset;
        frag_size = frag->size;
        frag_txd_num = _MDrv_NOE_PDMA_Calc_Frag_Txd_Num(frag_size);

        while (frag_txd_num > 0) {
            if (frag_size < MAX_PTXD_LEN)
                size = frag_size;
            else
                size = MAX_PTXD_LEN;

            if (ei_local->skb_txd_num % 2 == 0) {
                *tx_cpu_owner_idx = (*tx_cpu_owner_idx + 1) % NUM_TX_DESC;
                tx_ring = &ei_local->tx_ring0[*tx_cpu_owner_idx];

                while (tx_ring->txd_info2.DDONE_bit == 0) {
                    if (gmac_no == 2) {
                        p_ad = netdev_priv(ei_local->pseudo_dev);
                        p_ad->stat.tx_errors++;
                    } else {
                        ei_local->stat.tx_errors++;
                    }
                }

                tx_ring->txd_info1.SDP0 = dma_map_page(netdev->dev.parent, frag->page.p, frag_offset, size, DMA_TO_DEVICE);
                if (unlikely(dma_mapping_error(netdev->dev.parent, tx_ring->txd_info1.SDP0))) {
                    NOE_MSG_ERR("[%s]dma_map_page() failed\n", __func__);
                    goto err_dma;
                }

                tx_ring->txd_info2.SDL0 = size;

                if ((frag_txd_num == 1) && (i == (nr_frags - 1)))
                    tx_ring->txd_info2.LS0_bit = 1;
                else
                    tx_ring->txd_info2.LS0_bit = 0;
                tx_ring->txd_info2.DDONE_bit = 0;
                tx_ring->txd_info4.FPORT = gmac_no;
            } else {
                tx_ring->txd_info3.SDP1 = dma_map_page(netdev->dev.parent, frag->page.p, frag_offset, size, DMA_TO_DEVICE);
                if (unlikely(dma_mapping_error(netdev->dev.parent, tx_ring->txd_info3.SDP1))) {
                    NOE_MSG_ERR("[%s]dma_map_page() failed\n", __func__);
                    goto err_dma;
                }
                tx_ring->txd_info2.SDL1 = size;
                if ((frag_txd_num == 1) && (i == (nr_frags - 1)))
                    tx_ring->txd_info2.LS1_bit = 1;
                else
                    tx_ring->txd_info2.LS1_bit = 0;
            }
            frag_offset += size;
            frag_size -= size;
            frag_txd_num--;
            ei_local->skb_txd_num++;
        }
    }

    return 0;

err_dma:
    /* unmap dma */
    j = *tx_cpu_owner_idx;
    unmap_idx = i;
    for (i = 0; i < unmap_idx; i++) {
        frag = &skb_shinfo(skb)->frags[i];
        frag_offset = frag->page_offset;
        frag_size = frag->size;
        frag_txd_num = _MDrv_NOE_PDMA_Calc_Frag_Txd_Num(frag_size);

        while (frag_txd_num > 0) {
            if (frag_size < MAX_PTXD_LEN)
                size = frag_size;
            else
                size = MAX_PTXD_LEN;
            if (ei_local->skb_txd_num % 2 == 0) {
                j = (j + 1) % NUM_TX_DESC;
                dma_unmap_page(netdev->dev.parent, ei_local->tx_ring0[j].txd_info1.SDP0, ei_local->tx_ring0[j].txd_info2.SDL0, DMA_TO_DEVICE);
                /* reinit txd */
                ei_local->tx_ring0[j].txd_info2.LS0_bit = 1;
                ei_local->tx_ring0[j].txd_info2.DDONE_bit = 1;
            } else {
                dma_unmap_page(netdev->dev.parent, ei_local->tx_ring0[j].txd_info3.SDP1, ei_local->tx_ring0[j].txd_info2.SDL1, DMA_TO_DEVICE);
                /* reinit txd */
                ei_local->tx_ring0[j].txd_info2.LS1_bit = 1;
            }
            frag_offset += size;
            frag_size -= size;
            frag_txd_num--;
            ei_local->skb_txd_num++;
        }
    }

    return -1;
}

static int _MDrv_NOE_PDMA_Tso_Fill_Txd(struct net_device *dev, unsigned long *tx_cpu_owner_idx, struct sk_buff *skb, int gmac_no)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    struct iphdr *iph = NULL;
    struct ipv6hdr *ip6h = NULL;
    struct tcphdr *th = NULL;
    unsigned int nr_frags = skb_shinfo(skb)->nr_frags;
    unsigned int len, offset;
    int err;
    struct PDMA_txdesc *tx_ring = &ei_local->tx_ring0[*tx_cpu_owner_idx];

    tx_ring->txd_info4.FPORT = gmac_no;
    tx_ring->txd_info4.TSO = 0;

    if (skb->ip_summed == CHECKSUM_PARTIAL)
        tx_ring->txd_info4.TUI_CO = 7;
    else
        tx_ring->txd_info4.TUI_CO = 0;

    if (ei_local->features & FE_HW_VLAN_TX) {
        if (skb_vlan_tag_present(skb))
            tx_ring->txd_info4.VLAN_TAG = 0x10000 | skb_vlan_tag_get(skb);
        else
            tx_ring->txd_info4.VLAN_TAG = 0;
    }
#ifdef CONFIG_NOE_NAT_HW
    if (FOE_MAGIC_TAG(skb) == FOE_MAGIC_PPE) {
        if (IS_VALID_NOE_HWNAT_HOOK_RX) {
            /* PPE */
            tx_ring->txd_info4.FPORT = 4;
            FOE_MAGIC_TAG(skb) = 0;
        }
    }
#endif
    ei_local->skb_txd_num = 1;

    /* skb data handle */
    len = skb->len - skb->data_len;
    offset = virt_to_phys(skb->data);
    tx_ring->txd_info1.SDP0 = offset;
    if (len < MAX_PTXD_LEN) {
        tx_ring->txd_info2.SDL0 = len;
        tx_ring->txd_info2.LS0_bit = nr_frags ? 0 : 1;
        len = 0;
    } else {
        tx_ring->txd_info2.SDL0 = MAX_PTXD_LEN;
        tx_ring->txd_info2.LS0_bit = 0;
        len -= MAX_PTXD_LEN;
        offset += MAX_PTXD_LEN;
    }

    if (len > 0)
        _MDrv_NOE_PDMA_Tso_Fill_Data(ei_local, offset, len, tx_cpu_owner_idx, nr_frags, gmac_no);

    /* skb fragments handle */
    if (nr_frags > 0) {
        err = _MDrv_NOE_PDMA_Tso_Fill_Frag(dev, skb, tx_cpu_owner_idx, gmac_no);
        if (unlikely(err))
            return err;
    }

    /* fill in MSS info in tcp checksum field */
    if (skb_shinfo(skb)->gso_segs > 1) {
        MDrv_NOE_Update_Tso_Len(skb->len);
        /* TCP over IPv4 */
        iph = (struct iphdr *)skb_network_header(skb);
        if ((iph->version == 4) && (iph->protocol == IPPROTO_TCP)) {
            th = (struct tcphdr *)skb_transport_header(skb);
            tx_ring->txd_info4.TSO = 1;
            th->check = htons(skb_shinfo(skb)->gso_size);
            dma_sync_single_for_device(dev->dev.parent, virt_to_phys(th), sizeof(struct tcphdr), DMA_TO_DEVICE);
        }

        /* TCP over IPv6 */
        if (ei_local->features & FE_TSO_V6) {
            ip6h = (struct ipv6hdr *)skb_network_header(skb);
            if ((ip6h->nexthdr == NEXTHDR_TCP) && (ip6h->version == 6)) {
                th = (struct tcphdr *)skb_transport_header(skb);
                tx_ring->txd_info4.TSO = 1;
                th->check = htons(skb_shinfo(skb)->gso_size);
                dma_sync_single_for_device(dev->dev.parent, virt_to_phys(th), sizeof(struct tcphdr), DMA_TO_DEVICE);
            }
        }
    }
    tx_ring->txd_info2.DDONE_bit = 0;

    return 0;
}

static inline int MDrv_NOE_PDMA_Eth_Send(struct net_device *dev, struct sk_buff *skb, int gmac_no, unsigned int num_of_frag)
{
    unsigned int length = skb->len;
    struct END_DEVICE *ei_local = netdev_priv(dev);
#ifdef CONFIG_NOE_RW_PDMAPTR_FROM_VAR
    unsigned long tx_cpu_owner_idx0 = ei_local->tx_cpu_owner_idx0;
#else
    unsigned long tx_cpu_owner_idx0 = MHal_NOE_DMA_Get_Calc_Idx(E_NOE_DIR_TX);
#endif
    struct PSEUDO_ADAPTER *p_ad;
    int err;

    while (ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit == 0) {
        if (gmac_no == 2) {
            if (ei_local->pseudo_dev) {
                p_ad = netdev_priv(ei_local->pseudo_dev);
                p_ad->stat.tx_errors++;
            } else {
                NOE_MSG_ERR("pseudo_dev is still not initialize ");
                NOE_MSG_ERR("but receive packet from GMAC2\n");
            }
        } else {
            ei_local->stat.tx_errors++;
        }
    }

    if (num_of_frag > 1)
        err = _MDrv_NOE_PDMA_Tso_Fill_Txd(dev, &tx_cpu_owner_idx0, skb, gmac_no);
    else
        err = _MDrv_NOE_PDMA_Fill_Txd(dev, &tx_cpu_owner_idx0, skb, gmac_no);
    if (err)
        return err;

    tx_cpu_owner_idx0 = (tx_cpu_owner_idx0 + 1) % NUM_TX_DESC;
    while (ei_local->tx_ring0[tx_cpu_owner_idx0].txd_info2.DDONE_bit == 0) {
        if (gmac_no == 2) {
            p_ad = netdev_priv(ei_local->pseudo_dev);
            p_ad->stat.tx_errors++;
        } else {
            ei_local->stat.tx_errors++;
        }
    }
#ifdef CONFIG_NOE_RW_PDMAPTR_FROM_VAR
    ei_local->tx_cpu_owner_idx0 = tx_cpu_owner_idx0;
#endif
    /* make sure that all changes to the dma ring are flushed before we
     * continue
     */
    wmb();

    MHal_NOE_DMA_Update_Calc_Idx(E_NOE_DIR_TX,cpu_to_le32((u32)tx_cpu_owner_idx0));

    if (gmac_no == 2) {
        p_ad = netdev_priv(ei_local->pseudo_dev);
        p_ad->stat.tx_packets++;
        p_ad->stat.tx_bytes += length;
    }
    else {
        ei_local->stat.tx_packets++;
        ei_local->stat.tx_bytes += length;
    }

    return length;
}

int MDrv_NOE_PDMA_Start_Xmit(struct sk_buff *skb, struct net_device *dev, int gmac_no)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    unsigned long tx_cpu_owner_idx;
    unsigned int tx_cpu_owner_idx_next, tx_cpu_owner_idx_next2;
    unsigned int num_of_txd, num_of_frag;
    unsigned int nr_frags = skb_shinfo(skb)->nr_frags, i;
    struct skb_frag_struct *frag;
    struct PSEUDO_ADAPTER *p_ad;
    unsigned int tx_cpu_cal_idx;

#ifdef CONFIG_NOE_NAT_HW
    if (IS_VALID_NOE_HWNAT_HOOK_TX) {
        if (FOE_MAGIC_TAG(skb) != FOE_MAGIC_PPE)
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


    spin_lock(&ei_local->page_lock);
    dma_sync_single_for_device(dev->dev.parent, virt_to_phys(skb->data), skb->len, DMA_TO_DEVICE);

#ifdef CONFIG_NOE_RW_PDMAPTR_FROM_VAR
    tx_cpu_owner_idx = ei_local->tx_cpu_owner_idx0;
#else
    tx_cpu_owner_idx = MHal_NOE_DMA_Get_Calc_Idx(E_NOE_DIR_TX);
#endif

    if (ei_local->features & FE_TSO) {
        num_of_txd = _MDrv_NOE_PDMA_Calc_Frag_Txd_Num(skb->len - skb->data_len);
        if (nr_frags != 0) {
            for (i = 0; i < nr_frags; i++) {
                frag = &skb_shinfo(skb)->frags[i];
                num_of_txd += _MDrv_NOE_PDMA_Calc_Frag_Txd_Num(frag->size);
            }
        }
        num_of_frag = num_of_txd;
        num_of_txd = (num_of_txd + 1) >> 1;
    } else {
        num_of_frag = 1;
        num_of_txd = 1;
    }

    tx_cpu_owner_idx_next = (tx_cpu_owner_idx + num_of_txd) % NUM_TX_DESC;

    if ((ei_local->skb_free[tx_cpu_owner_idx_next] == 0) &&
        (ei_local->skb_free[tx_cpu_owner_idx] == 0)) {
        if (MDrv_NOE_PDMA_Eth_Send(dev, skb, gmac_no, num_of_frag) < 0) {
            dev_kfree_skb_any(skb);
            if (gmac_no == 2) {
                p_ad = netdev_priv(ei_local->pseudo_dev);
                p_ad->stat.tx_dropped++;
            } else {
                ei_local->stat.tx_dropped++;
            }
            goto tx_err;
        }

        tx_cpu_owner_idx_next2 = (tx_cpu_owner_idx_next + 1) % NUM_TX_DESC;

        if (ei_local->skb_free[tx_cpu_owner_idx_next2] != 0)
            ei_local->tx_ring_full = 1;
    }
    else {
        if (gmac_no == 2) {
            p_ad = netdev_priv(ei_local->pseudo_dev);
            p_ad->stat.tx_dropped++;
        } else {
            ei_local->stat.tx_dropped++;
        }

        dev_kfree_skb_any(skb);
        spin_unlock(&ei_local->page_lock);
        return NETDEV_TX_OK;
    }

    /* SG: use multiple TXD to send the packet (only have one skb) */
    tx_cpu_cal_idx = (tx_cpu_owner_idx + num_of_txd - 1) % NUM_TX_DESC;
    ei_local->skb_free[tx_cpu_cal_idx] = skb;
    while (--num_of_txd) {
        /* MAGIC ID */
        ei_local->skb_free[(--tx_cpu_cal_idx) % NUM_TX_DESC] = (struct sk_buff *)0xFFFFFFFF;
    }
tx_err:
    spin_unlock(&ei_local->page_lock);
    return NETDEV_TX_OK;
}

int MDrv_NOE_PDMA_Xmit_Housekeeping(struct net_device *netdev, int budget)
{
    struct END_DEVICE *ei_local = netdev_priv(netdev);
    struct PDMA_txdesc *tx_desc;
    unsigned long skb_free_idx;
    int tx_processed = 0;

    tx_desc = ei_local->tx_ring0;
    skb_free_idx = ei_local->free_idx;

    while (budget && (ei_local->skb_free[skb_free_idx] != 0) && (tx_desc[skb_free_idx].txd_info2.DDONE_bit == 1)) {
        if (ei_local->skb_free[skb_free_idx] != (struct sk_buff *)0xFFFFFFFF)
            dev_kfree_skb_any(ei_local->skb_free[skb_free_idx]);

        ei_local->skb_free[skb_free_idx] = 0;
        skb_free_idx = (skb_free_idx + 1) % NUM_TX_DESC;
        budget--;
        tx_processed++;
    }

    ei_local->tx_ring_full = 0;
    ei_local->free_idx = skb_free_idx;

    return tx_processed;
}

