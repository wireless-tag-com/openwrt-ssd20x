/*
* mdrv_noe.c- Sigmastar
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
/// @file   MDRV_NOE.c
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include "mdrv_noe.h"
#include "mdrv_noe_ioctl.h"
#include "mdrv_noe_ethtool.h"
#include "irqs.h"
#include "mdrv_noe_nat.h"
#include "mdrv_noe_dma.h"
#include <linux/ctype.h>


//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------

#if NETDEV_LRO_SUPPORTED
#define IS_TRAFFIC_RECEIVED_OFFLOAD(rx_skb, napi)   likely((rx_skb)->ip_summed == CHECKSUM_UNNECESSARY)
#define TRAFFIC_RECEIVED_OFFLOAD(ei_local, napi, rx_skb)      lro_receive_skb(&ei_local->lro_mgr, rx_skb, NULL);
#else
#define IS_TRAFFIC_RECEIVED_OFFLOAD(rx_skb, napi)   likely(((rx_skb)->ip_summed == CHECKSUM_UNNECESSARY) && ((napi) != NULL))
#define TRAFFIC_RECEIVED_OFFLOAD(ei_local, napi, rx_skb)      napi_gro_receive(napi, rx_skb);
#endif /* NETDEV_LRO_SUPPORTED */

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
//  Global Variable and Functions
//--------------------------------------------------------------------------------------------------
struct device *gdebug_class_dev;
struct net_device *g_dev_noe;
#ifdef CONFIG_NOE_NAT_HW
EXPORT_SYMBOL(g_dev_noe);
#endif

//--------------------------------------------------------------------------------------------------
//  Local Functions
//--------------------------------------------------------------------------------------------------

static int MDrv_NOE_Set_Mac2_Addr(struct net_device *dev, void *p);
static int MDrv_NOE_Set_Mac_Addr(struct net_device *dev, void *p);
static void MDrv_NOE_Set_Mac2(struct net_device *dev);
static void MDrv_NOE_Set_Mac(struct net_device *dev);
#if NETDEV_LRO_SUPPORTED
static int MDrv_NOE_Get_Skb_Header(struct sk_buff *skb, void **iphdr, void **tcph, u64 *hdr_flags, void *priv);
#endif
static int MDrv_NOE_Full_Poll(struct napi_struct *napi, int budget);
static int MDrv_NOE_RX_Poll(struct napi_struct *napi, int budget);
static int MDrv_NOE_TX_Poll(struct napi_struct *napi, int budget);
static void MDrv_NOE_Register_Func(struct END_DEVICE *ei_local);
static int __init MDrv_NOE_Init(struct net_device *dev);
static inline int MDrv_NOE_Init_Txp_Rxp(struct net_device *dev);
static inline int MDrv_NOE_Init_Txq_Rxp(struct net_device *dev);
static inline int MDrv_NOE_Init_Txq_Rxq(struct net_device *dev);
static int MDrv_NOE_Init_Dma(struct net_device *dev);
static void MDrv_NOE_Uninit(struct net_device *dev);
void virtif_setup_statistics(struct PSEUDO_ADAPTER *p_ad);
int MDrv_NOE_Virtif_Open(struct net_device *dev);
int MDrv_NOE_Virtif_Close(struct net_device *dev);
int MDrv_NOE_Virtif_Start_Xmit(struct sk_buff *p_skb, struct net_device *dev);
struct net_device_stats *MDrv_NOE_Virtif_Get_Stats(struct net_device *dev);
int MDrv_NOE_Virtif_Ioctl(struct net_device *net_dev, struct ifreq *ifr, int cmd);
static int MDrv_NOE_Change_Mtu(struct net_device *dev, int new_mtu);
static int MDrv_NOE_Disable_Clock(struct END_DEVICE *ei_local);
void MDrv_NOE_Virtif_Init(struct END_DEVICE *p_ad, struct net_device *net_dev);
int MDrv_NOE_Open(struct net_device *dev);
int MDrv_NOE_Close(struct net_device *dev);
static int MDrv_NOE_Start_Xmit(struct sk_buff *skb, struct net_device *dev);
struct net_device_stats *MDrv_NOE_Get_Stats(struct net_device *dev);
int MDrv_NOE_Ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
void MDrv_NOE_Setup_Fptable(struct net_device *dev);
static int MDrv_NOE_Eth_Recv(struct net_device *dev, struct napi_struct *napi, int budget);
static void MDrv_NOE_Reset_Statistics(struct END_DEVICE *ei_local);
static inline void MDrv_NOE_Init_Rx_Desc(struct PDMA_rxdesc *rx_ring, dma_addr_t dma_addr);
static void MDrv_NOE_Deinit_Dma(struct net_device *dev);
static irqreturn_t MDrv_NOE_NAPI_Interrupt(int irq, void *dev_id);
static irqreturn_t MDrv_NOE_NAPI_Separate_Interrupt(int irq, void *dev_id);
static irqreturn_t MDrv_NOE_Interrupt(int irq, void *dev_id);
static irqreturn_t MDrv_NOE_NAPI_RX_Interrupt(int irq, void *dev_id);
static irqreturn_t MDrv_NOE_NAPI_Separate_RX_Interrupt(int irq, void *dev_id);
static irqreturn_t MDrv_NOE_RX_Interrupt(int irq, void *dev_id);
static irqreturn_t MDrv_NOE_NAPI_TX_Interrupt(int irq, void *dev_id);
static irqreturn_t MDrv_NOE_NAPI_Separate_TX_Interrupt(int irq, void *dev_id);
static irqreturn_t MDrv_NOE_TX_Interrupt(int irq, void *dev_id);
static inline void MDrv_NOE_Recv(void);
static void MDrv_NOE_Tasklet_Recv(unsigned long unused);
static void MDrv_NOE_Workq_Recv(struct work_struct *work);
static int MDrv_NOE_Enable_Intr(struct net_device *dev);
static int MDrv_NOE_Disable_Intr(struct net_device *dev);
int MDrv_NOE_Set_Forward_Cfg(struct net_device *dev);
static int MDrv_NOE_Enable_Clock(struct END_DEVICE *ei_local);
void MDrv_NOE_Set_Io_Coherence(struct END_DEVICE *ei_local);
static int MDrv_NOE_Probe(struct platform_device *pdev);
static int MDrv_NOE_Remove(struct platform_device *pdev);


#if NETDEV_LRO_SUPPORTED
static void MDrv_NOE_Init_Napi_Lro(struct net_device *dev);
static int MDrv_NOE_Get_Skb_Header(struct sk_buff *skb, void **iphdr, void **tcph, u64 *hdr_flags, void *priv);
#ifdef CONFIG_INET_LRO
static inline void _MDrv_NOE_Flush_Lro_All(struct net_lro_mgr *lro_mgr) { lro_flush_all(lro_mgr); }
#else
static inline void _MDrv_NOE_Flush_Lro_All(struct net_lro_mgr *lro_mgr) { }
#endif
#endif /* NETDEV_LRO_SUPPORTED */

#ifdef CONFIG_PM
static int MDrv_NOE_Resume(struct platform_device *pdev);
static int MDrv_NOE_Suspend(struct platform_device *pdev, pm_message_t state);
#endif
//--------------------------------------------------------------------------------------------------
//  Local Variable
//--------------------------------------------------------------------------------------------------
static u64 noe_dmamask = ~(u32)0;
static int pending_recv;
unsigned char mac_uboot[6];
unsigned char mac1_uboot[6];


/* LRO support */
unsigned int lan_ip;
struct lro_para_struct lro_para;
int lro_flush_needed;

static const char *const _mdrv_noe_clk_source_name[] = {
    "ethif",
    "esw",
    "gp0",
    "gp1",
    "gp2",
    "sgmii_tx250m",
    "sgmii_rx250m",
    "sgmii_cdr_ref",
    "sgmii_cdr_fb",
    "trgpll",
    "sgmipll",
    "eth1pll",
    "eth2pll"
};

static const char noe_string[] = NOE_DRV_STRING;

static const struct of_device_id mstar_noe_of_ids[] = {
    {.compatible = NOE_COMPATIBLE_DEV_ID},
    {},
};

static struct platform_driver mstar_noe_driver = {
    .probe = MDrv_NOE_Probe,
    .remove = MDrv_NOE_Remove,
#ifdef CONFIG_PM
    .resume = MDrv_NOE_Resume,
    .suspend = MDrv_NOE_Suspend,
#endif
    .driver = {
            .name = noe_string,
            .owner = THIS_MODULE,
            .of_match_table = mstar_noe_of_ids,
    },
};


static inline MS_BOOL _MDrv_NOE_Is_IRQ0_Handler_Available(struct END_DEVICE *ei_local)
{
    if ((ei_local->irq_num < E_NOE_IRQ_MAX) && (ei_local->features & FE_IRQ_SEPARATE)) {
        return FALSE;
    }
    return TRUE;
}


/* Set the hardware MAC address. */
static int MDrv_NOE_Set_Mac_Addr(struct net_device *dev, void *p)
{
    struct sockaddr *addr = p;

    if (!is_valid_ether_addr(addr->sa_data))
        return -EADDRNOTAVAIL;

#ifndef CONFIG_NOE_NO_CHECK_ACTIVE_SETMAC
    if (netif_running(dev))
        return -EBUSY;
#endif
    memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
    MHal_NOE_Set_MAC_Address(E_NOE_GE_MAC1, dev->dev_addr);
    return 0;
}

static int MDrv_NOE_Set_Mac2_Addr(struct net_device *dev, void *p)
{
    struct sockaddr *addr = p;

    if (!is_valid_ether_addr(addr->sa_data))
        return -EADDRNOTAVAIL;

#ifndef CONFIG_NOE_NO_CHECK_ACTIVE_SETMAC
    if (netif_running(dev))
        return -EBUSY;
#endif
    memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);
    MHal_NOE_Set_MAC_Address(E_NOE_GE_MAC2, dev->dev_addr);
    return 0;
}

#if NETDEV_LRO_SUPPORTED
static int MDrv_NOE_Get_Skb_Header(struct sk_buff *skb, void **iphdr, void **tcph, u64 *hdr_flags, void *priv)
{
    struct iphdr *iph = NULL;
    int vhdr_len = 0;

    /* Make sure that this packet is Ethernet II, is not VLAN
     * tagged, is IPv4, has a valid IP header, and is TCP.
     */
    if (skb->protocol == 0x0081)
        vhdr_len = VLAN_HLEN;

    iph = (struct iphdr *)(skb->data + vhdr_len);
    if (iph->daddr != lro_para.lan_ip1)
        return -1;
    if (iph->protocol != IPPROTO_TCP)
        return -1;

    *iphdr = iph;
    *tcph = skb->data + (iph->ihl << 2) + vhdr_len;
    *hdr_flags = LRO_IPV4 | LRO_TCP;

    lro_flush_needed = 1;
    return 0;
}

static void MDrv_NOE_Init_Napi_Lro(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);

    ei_local->lro_mgr.dev = dev;
    memset(&ei_local->lro_mgr.stats, 0, sizeof(ei_local->lro_mgr.stats));
    ei_local->lro_mgr.features = LRO_F_NAPI;
    ei_local->lro_mgr.ip_summed = CHECKSUM_UNNECESSARY;
    ei_local->lro_mgr.ip_summed_aggr = CHECKSUM_UNNECESSARY;
    ei_local->lro_mgr.max_desc = ARRAY_SIZE(ei_local->lro_arr);
    ei_local->lro_mgr.max_aggr = 64;
    ei_local->lro_mgr.frag_align_pad = 0;
    ei_local->lro_mgr.lro_arr = ei_local->lro_arr;
    ei_local->lro_mgr.get_skb_header = MDrv_NOE_Get_Skb_Header;
    lro_flush_needed = 0;
}
#endif /* NETDEV_LRO_SUPPORTED */

static void MDrv_NOE_Reset_Statistics(struct END_DEVICE *ei_local)
{
    ei_local->stat.tx_packets = 0;
    ei_local->stat.tx_bytes = 0;
    ei_local->stat.tx_dropped = 0;
    ei_local->stat.tx_errors = 0;
    ei_local->stat.tx_aborted_errors = 0;
    ei_local->stat.tx_carrier_errors = 0;
    ei_local->stat.tx_fifo_errors = 0;
    ei_local->stat.tx_heartbeat_errors = 0;
    ei_local->stat.tx_window_errors = 0;

    ei_local->stat.rx_packets = 0;
    ei_local->stat.rx_bytes = 0;
    ei_local->stat.rx_dropped = 0;
    ei_local->stat.rx_errors = 0;
    ei_local->stat.rx_length_errors = 0;
    ei_local->stat.rx_over_errors = 0;
    ei_local->stat.rx_crc_errors = 0;
    ei_local->stat.rx_frame_errors = 0;
    ei_local->stat.rx_fifo_errors = 0;
    ei_local->stat.rx_missed_errors = 0;

    ei_local->stat.collisions = 0;
}

static inline void MDrv_NOE_Init_Rx_Desc(struct PDMA_rxdesc *rx_ring, dma_addr_t dma_addr)
{
    rx_ring->rxd_info1.PDP0 = dma_addr;
    rx_ring->rxd_info2.PLEN0 = MAX_RX_LENGTH;
    rx_ring->rxd_info2.LS0 = 0;
    rx_ring->rxd_info2.DDONE_bit = 0;
}

static int MDrv_NOE_Eth_Recv(struct net_device *dev, struct napi_struct *napi, int budget)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    struct PSEUDO_ADAPTER *p_ad = netdev_priv(ei_local->pseudo_dev);
    struct sk_buff *rx_skb;
    unsigned int length = 0;
    int rx_processed = 0;
    struct PDMA_rxdesc *rx_ring, *rx_ring_next;
    unsigned int rx_dma_owner_idx, rx_next_idx;
    void *rx_data, *rx_data_next, *new_data;
    unsigned int skb_size;

#ifdef CONFIG_NOE_RW_PDMAPTR_FROM_VAR
    rx_dma_owner_idx = (ei_local->rx_calc_idx[0] + 1) % NUM_RX_DESC;
#else
    rx_dma_owner_idx = (MHal_NOE_DMA_Get_Calc_Idx(E_NOE_DIR_RX) + 1) % NUM_RX_DESC;
#endif
    rx_ring = &ei_local->rx_ring[0][rx_dma_owner_idx];
    rx_data = ei_local->netrx_skb_data[0][rx_dma_owner_idx];

    skb_size = SKB_DATA_ALIGN(MAX_RX_LENGTH + NET_IP_ALIGN + NET_SKB_PAD) + SKB_DATA_ALIGN(sizeof(struct skb_shared_info));

    for (;;) {
        dma_addr_t dma_addr;

        if ((rx_processed++ > budget) || (rx_ring->rxd_info2.DDONE_bit == 0))
            break;

        rx_next_idx = (rx_dma_owner_idx + 1) % NUM_RX_DESC;
        rx_ring_next = &ei_local->rx_ring[0][rx_next_idx];
        rx_data_next = ei_local->netrx_skb_data[0][rx_next_idx];
        prefetch(rx_ring_next);

        /* We have to check the free memory size is big enough
         * before pass the packet to cpu
         */
        new_data = MDrv_NOE_SkbData_Alloc(skb_size, GFP_ATOMIC);

        if (unlikely(!new_data)) {
            NOE_MSG_ERR("skb not available...\n");
            goto skb_err;
        }

        dma_addr = dma_map_single(dev->dev.parent, new_data + NET_SKB_PAD, MAX_RX_LENGTH, DMA_FROM_DEVICE);

        if (unlikely(dma_mapping_error(dev->dev.parent, dma_addr))) {
            NOE_MSG_ERR("[%s]dma_map_single() failed...\n", __func__);
            MDrv_NOE_SkbData_Free(new_data);
            goto skb_err;
        }

        rx_skb = MDrv_NOE_Skb_Build(rx_data, skb_size);

        if (unlikely(!rx_skb)) {
            put_page(virt_to_head_page(rx_data));
            NOE_MSG_ERR("build_skb failed\n");
            goto skb_err;
        }
        skb_reserve(rx_skb, NET_SKB_PAD + NET_IP_ALIGN);

        length = rx_ring->rxd_info2.PLEN0;
        dma_unmap_single(dev->dev.parent, rx_ring->rxd_info1.PDP0, length, DMA_FROM_DEVICE);

        prefetch(rx_skb->data);

        /* skb processing */
        skb_put(rx_skb, length);

        /* rx packet from GE2 */
        if (rx_ring->rxd_info4.SP == 2) {
            if (likely(ei_local->pseudo_dev)) {
                rx_skb->dev = ei_local->pseudo_dev;
                rx_skb->protocol = eth_type_trans(rx_skb, ei_local->pseudo_dev);
            } else {
                NOE_MSG_ERR("pseudo_dev is still not initialize ");
                NOE_MSG_ERR("but receive packet from GMAC2\n");
            }
        } else {
            rx_skb->dev = dev;
            rx_skb->protocol = eth_type_trans(rx_skb, dev);
        }

        /* rx checksum offload */
        if (rx_ring->rxd_info4.L4VLD)
            rx_skb->ip_summed = CHECKSUM_UNNECESSARY;
        else
            rx_skb->ip_summed = CHECKSUM_NONE;

#ifdef CONFIG_NOE_NAT_HW
        if (IS_VALID_NOE_HWNAT_HOOK_RX) {
            *(uint32_t *)(FOE_INFO_START_ADDR_HEAD(rx_skb)) = *(uint32_t *)&rx_ring->rxd_info4;
            *(uint32_t *)(FOE_INFO_START_ADDR_TAIL(rx_skb) + 2) = *(uint32_t *)&rx_ring->rxd_info4;
            FOE_ALG_HEAD(rx_skb) = 0;
            FOE_ALG_TAIL(rx_skb) = 0;
            FOE_MAGIC_TAG_HEAD(rx_skb) = FOE_MAGIC_GE;
            FOE_MAGIC_TAG_TAIL(rx_skb) = FOE_MAGIC_GE;
            FOE_TAG_PROTECT_HEAD(rx_skb) = TAG_PROTECT;
            FOE_TAG_PROTECT_TAIL(rx_skb) = TAG_PROTECT;
        }
#endif

        MDrv_NOE_LOG_Dump_Skb(rx_skb);
/* IS_VALID_NOE_HWNAT_HOOK_RX return 1 --> continue
 * IS_VALID_NOE_HWNAT_HOOK_RX return 0 --> FWD & without netif_rx
 */
#ifdef CONFIG_NOE_NAT_HW
        if ((IS_NOT_VALID_NOE_HWNAT_HOOK_RX) || (IS_VALID_NOE_HWNAT_HOOK_RX && NOE_HWNAT_HOOK_RX(rx_skb))) {
#endif
            if (!(ei_local->features & FE_SW_LRO)) {
                if (ei_local->features & FE_INT_NAPI) {
                    /* napi_gro_receive(napi, rx_skb); */
                    netif_receive_skb(rx_skb);
                }
                else
                    netif_rx(rx_skb);
            }
            else {
                if (IS_TRAFFIC_RECEIVED_OFFLOAD(rx_skb , napi)) {
                    TRAFFIC_RECEIVED_OFFLOAD(ei_local, napi, rx_skb);
                }
                else
                {
                    if (ei_local->features & FE_INT_NAPI)
                        netif_receive_skb(rx_skb);
                    else
                        netif_rx(rx_skb);
                }
            }
#ifdef CONFIG_NOE_NAT_HW
        }
#endif

        if (rx_ring->rxd_info4.SP == 2) {
            p_ad->stat.rx_packets++;
            p_ad->stat.rx_bytes += length;
        } else {
            ei_local->stat.rx_packets++;
            ei_local->stat.rx_bytes += length;
        }

        /* init RX desc. */
        MDrv_NOE_Init_Rx_Desc(rx_ring, dma_addr);
        ei_local->netrx_skb_data[0][rx_dma_owner_idx] = new_data;
        /* make sure that all changes to the dma ring are flushed before
         * we continue
         */
        wmb();

        MHal_NOE_DMA_Update_Calc_Idx(E_NOE_DIR_RX, rx_dma_owner_idx);
#ifdef CONFIG_NOE_RW_PDMAPTR_FROM_VAR
        ei_local->rx_calc_idx[0] = rx_dma_owner_idx;
#endif

        /* Update to Next packet point that was received.
         */
#ifdef CONFIG_NOE_RW_PDMAPTR_FROM_VAR
        rx_dma_owner_idx = rx_next_idx;
#else
        rx_dma_owner_idx = (MHal_NOE_DMA_Get_Calc_Idx(E_NOE_DIR_RX) + 1) % NUM_RX_DESC;
#endif

        /* use prefetched variable */
        rx_ring = rx_ring_next;
        rx_data = rx_data_next;
    }           /* for */

    if (lro_flush_needed) {
#if NETDEV_LRO_SUPPORTED
        _MDrv_NOE_Flush_Lro_All(&ei_local->lro_mgr);
#endif /* NETDEV_LRO_SUPPORTED */
        lro_flush_needed = 0;
    }

    return rx_processed;

skb_err:
    /* rx packet from GE2 */
    if (rx_ring->rxd_info4.SP == 2)
        p_ad->stat.rx_dropped++;
    else
        ei_local->stat.rx_dropped++;

    /* Discard the rx packet */
    MDrv_NOE_Init_Rx_Desc(rx_ring, rx_ring->rxd_info1.PDP0);

    /* make sure that all changes to the dma ring
     * are flushed before we continue
     */
    wmb();

    MHal_NOE_DMA_Update_Calc_Idx(E_NOE_DIR_RX, rx_dma_owner_idx);
#ifdef CONFIG_NOE_RW_PDMAPTR_FROM_VAR
    ei_local->rx_calc_idx[0] = rx_dma_owner_idx;
#endif

    return (budget + 1);
}

static int MDrv_NOE_Full_Poll(struct napi_struct *napi, int budget)
{
    struct END_DEVICE *ei_local = container_of(napi, struct END_DEVICE, napi);
    struct net_device *netdev = ei_local->netdev;
    //unsigned long reg_int_val_rx, reg_int_val_tx;
    //unsigned long reg_int_mask_rx, reg_int_mask_tx;
    unsigned long flags;
    int tx_done = 0, rx_done = 0;

    if (MHal_NOE_Get_Sep_Intr_Status(E_NOE_DIR_TX) == NOE_TRUE) {
        tx_done = ei_local->ei_xmit_housekeeping(netdev, NUM_TX_MAX_PROCESS);
    }
    if (MHal_NOE_Get_Sep_Intr_Status(E_NOE_DIR_RX) == NOE_TRUE) {
        rx_done = ei_local->ei_eth_recv(netdev, napi, budget);
    }

    if (rx_done >= budget)
        return budget;

    napi_complete(napi);

    spin_lock_irqsave(&ei_local->irq_lock, flags);

    MHal_NOE_Clear_Sep_Intr_Status(E_NOE_DIR_TX);
    MHal_NOE_Clear_Sep_Intr_Status(E_NOE_DIR_RX);
    MHal_NOE_Enable_Sep_Intr(E_NOE_DIR_TX);
    MHal_NOE_Enable_Sep_Intr(E_NOE_DIR_RX);
    spin_unlock_irqrestore(&ei_local->irq_lock, flags);

    return rx_done;
}

static int MDrv_NOE_RX_Poll(struct napi_struct *napi, int budget)
{
    struct END_DEVICE *ei_local = container_of(napi, struct END_DEVICE, napi_rx);
    struct net_device *netdev = ei_local->netdev;
    //unsigned long reg_int_val_rx;
    //unsigned long reg_int_mask_rx;
    unsigned long flags;
    int rx_done = 0;

    if (MHal_NOE_Get_Sep_Intr_Status(E_NOE_DIR_RX) == NOE_TRUE) {
        MHal_NOE_Clear_Sep_Intr_Status(E_NOE_DIR_RX);
        rx_done = ei_local->ei_eth_recv(netdev, napi, budget);
    }

    if (rx_done >= budget)
        return budget;

    napi_complete(napi);

    spin_lock_irqsave(&ei_local->irq_lock, flags);

    MHal_NOE_Enable_Sep_Intr(E_NOE_DIR_RX);
    spin_unlock_irqrestore(&ei_local->irq_lock, flags);

    return rx_done;
}

static int MDrv_NOE_TX_Poll(struct napi_struct *napi, int budget)
{
    struct END_DEVICE *ei_local =
        container_of(napi, struct END_DEVICE, napi_tx);
    struct net_device *netdev = ei_local->netdev;
    //unsigned long reg_int_val_tx;
    //unsigned long reg_int_mask_tx;
    unsigned long flags;
    int tx_done = 0;

    if (MHal_NOE_Get_Sep_Intr_Status(E_NOE_DIR_TX) == NOE_TRUE) {
        MHal_NOE_Clear_Sep_Intr_Status(E_NOE_DIR_TX);
        tx_done = ei_local->ei_xmit_housekeeping(netdev, NUM_TX_MAX_PROCESS);
    }
    napi_complete(napi);

    spin_lock_irqsave(&ei_local->irq_lock, flags);
    MHal_NOE_Enable_Sep_Intr(E_NOE_DIR_TX);
    spin_unlock_irqrestore(&ei_local->irq_lock, flags);

    return 1;
}

static void MDrv_NOE_Register_Func(struct END_DEVICE *ei_local)
{
    /* TX handling */

    if (ei_local->features & FE_QDMA_TX) {
        ei_local->ei_start_xmit = MDrv_NOE_QDMA_Start_Xmit;
        ei_local->ei_xmit_housekeeping = MDrv_NOE_QDMA_Xmit_Housekeeping;
        MHal_NOE_Init_Sep_Intr(E_NOE_DMA_QUEUE, E_NOE_DIR_TX);
    }
    else {
        ei_local->ei_start_xmit = MDrv_NOE_PDMA_Start_Xmit;
        ei_local->ei_xmit_housekeeping = MDrv_NOE_PDMA_Xmit_Housekeeping;
        MHal_NOE_Init_Sep_Intr(E_NOE_DMA_PACKET, E_NOE_DIR_TX);
    }

    /* RX handling */
    if (ei_local->features & FE_QDMA_RX) {
        MHal_NOE_Init_Sep_Intr(E_NOE_DMA_QUEUE, E_NOE_DIR_RX);
    } else {
        MHal_NOE_Init_Sep_Intr(E_NOE_DMA_PACKET, E_NOE_DIR_RX);
    }

    /* HW LRO handling */
    if (ei_local->features & FE_HW_LRO)
        ei_local->ei_eth_recv = MDrv_NOE_LRO_Recv;
    else
        ei_local->ei_eth_recv = MDrv_NOE_Eth_Recv;

    /* HW NAT handling */
    if (!(ei_local->features & FE_HW_NAT)) {
#ifdef CONFIG_NOE_NAT_HW
        NOE_HWNAT_HOOK_RX_INIT;
        NOE_HWNAT_HOOK_TX_INIT;
#endif
    }
}

static int __init MDrv_NOE_Init(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);

    MHal_NOE_Reset_FE();

    if (ei_local->features & FE_INT_NAPI) {
        /* we run 2 devices on the same DMA ring */
        /* so we need a dummy device for NAPI to work */
        init_dummy_netdev(&ei_local->dummy_dev);

        if (ei_local->features & FE_INT_NAPI_TX_RX) {
            netif_napi_add(&ei_local->dummy_dev, &ei_local->napi_rx, MDrv_NOE_RX_Poll, NOE_NAPI_WEIGHT);
            netif_napi_add(&ei_local->dummy_dev, &ei_local->napi_tx, MDrv_NOE_TX_Poll, NOE_NAPI_WEIGHT);
        }
        else if (ei_local->features & FE_INT_NAPI_RX_ONLY) {
            netif_napi_add(&ei_local->dummy_dev, &ei_local->napi_rx, MDrv_NOE_RX_Poll, NOE_NAPI_WEIGHT);
        }
        else {
            netif_napi_add(&ei_local->dummy_dev, &ei_local->napi, MDrv_NOE_Full_Poll, NOE_NAPI_WEIGHT);
        }
    }

    spin_lock_init(&ei_local->page_lock);
    spin_lock_init(&ei_local->irq_lock);
    ether_setup(dev);

#if NETDEV_LRO_SUPPORTED
    if (ei_local->features & FE_SW_LRO)
        MDrv_NOE_Init_Napi_Lro(dev);
#endif /* NETDEV_LRO_SUPPORTED */

    MDrv_NOE_Register_Func(ei_local);

    /* init  my IP */
    strncpy(ei_local->lan_ip4_addr, FE_DEFAULT_LAN_IP, IP4_ADDR_LEN);

    return 0;
}

static void MDrv_NOE_Uninit(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);

    unregister_netdev(dev);
    free_netdev(dev);

    if (ei_local->features & FE_GE2_SUPPORT) {
        unregister_netdevice(ei_local->pseudo_dev);
        free_netdev(ei_local->pseudo_dev);
    }

    NOE_MSG_DBG("Free ei_local and unregister netdev...\n");

    MDrv_NOE_PROC_Exit();
}

static void MDrv_NOE_Set_Mac(struct net_device *dev)
{
    /* If the mac address is invalid, use random mac address  */
    if (is_valid_ether_addr(mac_uboot))
    {
        ether_addr_copy(dev->dev_addr, mac_uboot);
        NOE_MSG_ERR("Get MAC address from u-boot\n");
    }else
    if (!is_valid_ether_addr(dev->dev_addr)) {
        random_ether_addr(dev->dev_addr);
        NOE_MSG_DBG("generated random MAC address %pM\n", dev->dev_addr);
        dev->addr_assign_type = NET_ADDR_RANDOM;
    }
    NOE_MSG_DBG("MAC address %pM\n", dev->dev_addr);
    MHal_NOE_Set_MAC_Address(E_NOE_GE_MAC1, dev->dev_addr);
}

static void MDrv_NOE_Set_Mac2(struct net_device *dev)
{
    if (is_valid_ether_addr(mac1_uboot))
    {
        ether_addr_copy(dev->dev_addr, mac1_uboot);
        NOE_MSG_ERR("Get MAC address from u-boot\n");

    }else
    /* If the mac address is invalid, use random mac address  */
    if (!is_valid_ether_addr(dev->dev_addr)) {
        random_ether_addr(dev->dev_addr);
        NOE_MSG_DBG("generated random MAC address %pM\n", dev->dev_addr);
        dev->addr_assign_type = NET_ADDR_RANDOM;
    }
    NOE_MSG_DBG("MAC2 address %pM\n", dev->dev_addr);
    MHal_NOE_Set_MAC_Address(E_NOE_GE_MAC2, dev->dev_addr);
}

#ifdef CONFIG_NOE_RW_PDMAPTR_FROM_VAR
static void MDrv_NOE_Init_Rx_Cal_Idx(struct END_DEVICE *ei_local)
{
    struct noe_lro_calc_idx calc_idx;
    ei_local->rx_calc_idx[0] = MHal_NOE_DMA_Get_Calc_Idx(E_NOE_DIR_RX);
    if (ei_local->features & FE_HW_LRO) {
        MHal_NOE_LRO_Get_Calc_Idx(&calc_idx);
        ei_local->rx_calc_idx[1] = calc_idx.ring1;
        ei_local->rx_calc_idx[2] = calc_idx.ring2;
        ei_local->rx_calc_idx[3] = calc_idx.ring3;
    }
}
#endif

static inline int MDrv_NOE_Init_Txp_Rxp(struct net_device *dev)
{
    int err;
    struct END_DEVICE *ei_local = netdev_priv(dev);

    if (MHal_NOE_Dma_Is_Idle(E_NOE_DMA_PACKET) != E_NOE_RET_TRUE)
        return -1;

    err = MDrv_NOE_PDMA_Init_Rx(dev);
    if (err)
        return err;

    if (ei_local->features & FE_HW_LRO) {
        err = MDrv_NOE_LRO_Init(dev);
        if (err)
            return err;
    }

#ifdef CONFIG_NOE_RW_PDMAPTR_FROM_VAR
    MDrv_NOE_Init_Rx_Cal_Idx(ei_local);
#endif

    err = MDrv_NOE_PDMA_Init_Tx(dev);
    if (err)
        return err;

    MHal_NOE_Dma_Init_Global_Config(E_NOE_DMA_PACKET);

    /* enable RXD prefetch of ADMA */
    MHal_NOE_LRO_Set_Prefetch();
    return 0;
}

static inline int MDrv_NOE_Init_Txq_Rxp(struct net_device *dev)
{
    int err;
    struct END_DEVICE *ei_local = netdev_priv(dev);

    if (MHal_NOE_Dma_Is_Idle(E_NOE_DMA_PACKET) != E_NOE_RET_TRUE) {
        NOE_MSG_DBG("PDMA is busy!\n");
        return -1;
    }

    if (MHal_NOE_Dma_Is_Idle(E_NOE_DMA_QUEUE) != E_NOE_RET_TRUE) {
        NOE_MSG_DBG("QDMA is busy!\n");
        return -1;
    }
    NOE_MSG_DBG("[%s][%d] feature = 0x%x \n", __FUNCTION__, __LINE__, ei_local->features);
    err = MDrv_NOE_QDMA_Init_Rx(dev);
    if (err)
        return err;

    err = MDrv_NOE_PDMA_Init_Rx(dev);
    if (err)
        return err;

    if (ei_local->features & FE_HW_LRO) {
        err = MDrv_NOE_LRO_Init(dev);
        if (err)
            return err;
    }
#ifdef CONFIG_NOE_RW_PDMAPTR_FROM_VAR
    MDrv_NOE_Init_Rx_Cal_Idx(ei_local);
#endif

    err = MDrv_NOE_QDMA_Init_Tx(dev);
    if (err)
        return err;

    MHal_NOE_Dma_Init_Global_Config(E_NOE_DMA_PACKET);
    MHal_NOE_Dma_Init_Global_Config((ei_local->features & FE_HW_SFQ)? E_NOE_DMA_QUEUE_WITH_SFQ :E_NOE_DMA_QUEUE);

    /* enable RXD prefetch of ADMA */
    MHal_NOE_LRO_Set_Prefetch();
    return 0;
}

static inline int MDrv_NOE_Init_Txq_Rxq(struct net_device *dev)
{
    int err;
    struct END_DEVICE *ei_local = netdev_priv(dev);


    if (MHal_NOE_Dma_Is_Idle(E_NOE_DMA_QUEUE) != E_NOE_RET_TRUE)
        return -1;

    err = MDrv_NOE_QDMA_Init_Rx(dev);
    if (err)
        return err;

#ifdef CONFIG_NOE_RW_PDMAPTR_FROM_VAR
    MDrv_NOE_Init_Rx_Cal_Idx(ei_local);
#endif

    err = MDrv_NOE_QDMA_Init_Tx(dev);
    if (err)
        return err;

    MHal_NOE_Dma_Init_Global_Config((ei_local->features & FE_HW_SFQ)? E_NOE_DMA_QUEUE_WITH_SFQ :E_NOE_DMA_QUEUE);

    return 0;
}

static int MDrv_NOE_Init_Dma(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);

    if ((ei_local->features & FE_QDMA_TX) && (ei_local->features & FE_QDMA_RX)) {
        return MDrv_NOE_Init_Txq_Rxq(dev);
    }

    if (ei_local->features & FE_QDMA_TX) {
        return MDrv_NOE_Init_Txq_Rxp(dev);
    }
    else {
        return MDrv_NOE_Init_Txp_Rxp(dev);
    }
}

static void MDrv_NOE_Deinit_Dma(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);

    if (ei_local->features & FE_QDMA_TX)
        MDrv_NOE_QDMA_Deinit_Tx(dev);
    else
        MDrv_NOE_PDMA_Deinit_Tx(dev);

    if (!(ei_local->features & FE_QDMA_RX))
        MDrv_NOE_PDMA_Deinit_Rx(dev);
#ifdef CONFIG_NOE_QDMA
    MDrv_NOE_QDMA_Deinit_Rx(dev);
#endif
    if (ei_local->features & FE_HW_LRO)
        MDrv_NOE_LRO_Deinit(dev);

    NOE_MSG_DBG("Free TX/RX Ring Memory!\n");
}

static int MDrv_NOE_Reset_Thread(void *data)
{
    struct END_DEVICE *ei_local = netdev_priv(g_dev_noe);
    unsigned long flags;
    for (;;) {
        ei_local->fe_reset_times++;
        spin_lock_irqsave(&ei_local->page_lock, flags);
        MHal_NOE_Do_Reset();
        spin_unlock_irqrestore(&ei_local->page_lock, flags);

        msleep(FE_RESET_POLLING_MS);
        if (kthread_should_stop())
            break;
    }
    return 0;
}

static irqreturn_t MDrv_NOE_NAPI_RxOnly_Interrupt(int irq, void *dev_id)
{
    struct net_device *dev = (struct net_device *)dev_id;
    struct END_DEVICE *ei_local = netdev_priv(dev);
    unsigned long flags;


    if (likely(napi_schedule_prep(&ei_local->napi_rx))) {
        spin_lock_irqsave(&ei_local->irq_lock, flags);

        /* Clear RX interrupt status */
        MHal_NOE_Clear_Sep_Intr_Status(E_NOE_DIR_RX);

        /* Disable RX interrupt */
        MHal_NOE_Disable_Sep_Intr(E_NOE_DIR_RX);

        spin_unlock_irqrestore(&ei_local->irq_lock, flags);

        __napi_schedule(&ei_local->napi_rx);
    }

    return IRQ_HANDLED;
}

static irqreturn_t MDrv_NOE_NAPI_Interrupt(int irq, void *dev_id)
{
    struct net_device *dev = (struct net_device *)dev_id;
    struct END_DEVICE *ei_local = netdev_priv(dev);
    unsigned long flags;

    MDrv_NOE_MAC_Detect_Link_Status(dev);

    if (likely(napi_schedule_prep(&ei_local->napi))) {
        spin_lock_irqsave(&ei_local->irq_lock, flags);
        /* Disable TX/RX interrupt */
        MHal_NOE_Disable_Sep_Intr(E_NOE_DIR_BOTH);
        spin_unlock_irqrestore(&ei_local->irq_lock, flags);

        __napi_schedule(&ei_local->napi);
    }

    return IRQ_HANDLED;
}

static irqreturn_t MDrv_NOE_NAPI_Separate_Interrupt(int irq, void *dev_id)
{
    struct net_device *dev = (struct net_device *)dev_id;
    struct END_DEVICE *ei_local = netdev_priv(dev);
    //unsigned int reg_int_mask;
    unsigned long flags;

    MDrv_NOE_MAC_Detect_Link_Status(dev);

    if (likely(napi_schedule_prep(&ei_local->napi_tx))) {
        spin_lock_irqsave(&ei_local->irq_lock, flags);
        /* Disable TX interrupt */
        MHal_NOE_Disable_Sep_Intr(E_NOE_DIR_TX);
        spin_unlock_irqrestore(&ei_local->irq_lock, flags);

        __napi_schedule(&ei_local->napi_tx);
    }

    if (likely(napi_schedule_prep(&ei_local->napi_rx))) {
        spin_lock_irqsave(&ei_local->irq_lock, flags);
        /* Disable RX interrupt */
        MHal_NOE_Disable_Sep_Intr(E_NOE_DIR_RX);
        spin_unlock_irqrestore(&ei_local->irq_lock, flags);

        __napi_schedule(&ei_local->napi_rx);
    }

    return IRQ_HANDLED;
}

static irqreturn_t MDrv_NOE_Interrupt(int irq, void *dev_id)
{
    unsigned int recv = 0;

    unsigned int transmit __maybe_unused = 0;
    unsigned long flags;

    struct net_device *dev = (struct net_device *)dev_id;
    struct END_DEVICE *ei_local = netdev_priv(dev);

    if (!dev) {
        NOE_MSG_ERR("net_interrupt(): irq for unknown device.\n");
        return IRQ_NONE;
    }

    spin_lock_irqsave(&ei_local->irq_lock, flags);

    MHal_NOE_Get_Intr_Status((ei_local->features & FE_QDMA)?(E_NOE_DMA_PACKET|E_NOE_DMA_QUEUE):E_NOE_DMA_PACKET, &recv, &transmit);
    ei_local->ei_xmit_housekeeping(dev, NUM_TX_MAX_PROCESS);
    MHal_NOE_Clear_Intr_Status((ei_local->features & FE_QDMA)?(E_NOE_DMA_PACKET|E_NOE_DMA_QUEUE):E_NOE_DMA_PACKET);


    if (((recv == 1) || (pending_recv == 1)) && (ei_local->tx_ring_full == 0)) {
        MHal_NOE_Enable_Intr_Status(E_NOE_DMA_PACKET);
        pending_recv = 0;

        if (ei_local->features & FE_INT_WORKQ)
            schedule_work(&ei_local->rx_wq);
        else
            tasklet_hi_schedule(&ei_local->rx_tasklet);
    }
    else if (recv == 1 && ei_local->tx_ring_full == 1) {
        pending_recv = 1;
    }
    else if ((recv == 0) && (transmit == 0)) {
        MDrv_NOE_MAC_Detect_Link_Status(dev);
    }
    spin_unlock_irqrestore(&ei_local->irq_lock, flags);

    return IRQ_HANDLED;
}

static irqreturn_t MDrv_NOE_NAPI_RX_Interrupt(int irq, void *dev_id)
{
    struct net_device *netdev = (struct net_device *)dev_id;
    struct END_DEVICE *ei_local;
    unsigned long flags;

    if (unlikely(!netdev)) {
        NOE_MSG_ERR("net_interrupt(): irq %x for unknown device.\n", irq);
        return IRQ_NONE;
    }
    ei_local = netdev_priv(netdev);

    if (likely(MHal_NOE_Get_Sep_Intr_Status(E_NOE_DIR_RX) == NOE_TRUE)) {
        if (likely(napi_schedule_prep(&ei_local->napi))) {
            spin_lock_irqsave(&ei_local->irq_lock, flags);
            /* Disable RX/TX interrupt */
            MHal_NOE_Disable_Sep_Intr(E_NOE_DIR_BOTH);
            spin_unlock_irqrestore(&ei_local->irq_lock, flags);
            __napi_schedule(&ei_local->napi);
        }
    }
    else {
        spin_lock_irqsave(&ei_local->irq_lock, flags);
        /* Ack other interrupt status except TX irqs */
        MHal_NOE_Clear_Sep_Intr_Specific_Status(E_NOE_DIR_RX, E_NOE_INTR_CLR_EXCEPT_TX);
        spin_unlock_irqrestore(&ei_local->irq_lock, flags);
    }

    return IRQ_HANDLED;
}

static irqreturn_t MDrv_NOE_NAPI_Separate_RX_Interrupt(int irq, void *dev_id)
{
    struct net_device *netdev = (struct net_device *)dev_id;
    struct END_DEVICE *ei_local;
    unsigned long flags;

    if (unlikely(!netdev)) {
        NOE_MSG_ERR("%s: irq %x for unknown device.\n", __FUNCTION__, irq);
        return IRQ_NONE;
    }
    ei_local = netdev_priv(netdev);
    if (likely(MHal_NOE_Get_Sep_Intr_Status(E_NOE_DIR_RX) == NOE_TRUE )) {
        if (likely(napi_schedule_prep(&ei_local->napi_rx))) {
            spin_lock_irqsave(&ei_local->irq_lock, flags);
            /* Clear RX interrupt status */
            MHal_NOE_Clear_Sep_Intr_Status(E_NOE_DIR_RX);
            /* Clear RX interrupt status */
            MHal_NOE_Disable_Sep_Intr(E_NOE_DIR_RX);
            spin_unlock_irqrestore(&ei_local->irq_lock, flags);
            __napi_schedule(&ei_local->napi_rx);
        }
    } else {
        spin_lock_irqsave(&ei_local->irq_lock, flags);
        /* Ack other interrupt status except TX irqs */
        MHal_NOE_Clear_Sep_Intr_Specific_Status(E_NOE_DIR_RX, E_NOE_INTR_CLR_EXCEPT_TX);
        spin_unlock_irqrestore(&ei_local->irq_lock, flags);
    }

    return IRQ_HANDLED;
}

static irqreturn_t MDrv_NOE_RX_Interrupt(int irq, void *dev_id)
{
    unsigned int recv = 0;
    unsigned long flags;

    struct net_device *netdev = (struct net_device *)dev_id;
    struct END_DEVICE *ei_local;

    if (unlikely(!netdev)) {
        NOE_MSG_ERR("%s: irq %x for unknown device.\n", __FUNCTION__, irq);
        return IRQ_NONE;
    }
    ei_local = netdev_priv(netdev);

    spin_lock_irqsave(&ei_local->irq_lock, flags);
    if (MHal_NOE_Get_Sep_Intr_Status(E_NOE_DIR_RX) == NOE_TRUE)
        recv = 1;

    /* Clear RX interrupt status */
    MHal_NOE_Clear_Sep_Intr_Status(E_NOE_DIR_RX);

    if (likely(((recv == 1) || (pending_recv == 1)) && (ei_local->tx_ring_full == 0))) {
        /* Disable RX interrupt */
        MHal_NOE_Disable_Sep_Intr(E_NOE_DIR_RX);
        pending_recv = 0;

        if (likely(ei_local->features & FE_INT_TASKLET))
            tasklet_hi_schedule(&ei_local->rx_tasklet);
        else
            schedule_work(&ei_local->rx_wq);
    }
    else if (recv == 1 && ei_local->tx_ring_full == 1) {
        pending_recv = 1;
    }
    else if ((recv == 0)) {
        MDrv_NOE_MAC_Detect_Link_Status(netdev);
    }

    spin_unlock_irqrestore(&ei_local->irq_lock, flags);

    return IRQ_HANDLED;
}

static irqreturn_t MDrv_NOE_NAPI_TX_Interrupt(int irq, void *dev_id)
{
    struct net_device *netdev = (struct net_device *)dev_id;
    struct END_DEVICE *ei_local;
    unsigned long flags;

    if (unlikely(!netdev)) {
        NOE_MSG_ERR("%s: irq %x for unknown device.\n", __FUNCTION__, irq);
        return IRQ_NONE;
    }

    MDrv_NOE_MAC_Detect_Link_Status(netdev);
    ei_local = netdev_priv(netdev);
    if (likely(MHal_NOE_Get_Sep_Intr_Status(E_NOE_DIR_TX) == NOE_TRUE)) {
        if (likely(napi_schedule_prep(&ei_local->napi))) {
            spin_lock_irqsave(&ei_local->irq_lock, flags);
            MHal_NOE_Disable_Sep_Intr(E_NOE_DIR_BOTH);
            spin_unlock_irqrestore(&ei_local->irq_lock, flags);
            __napi_schedule(&ei_local->napi);
        }
    }
    else {
        spin_lock_irqsave(&ei_local->irq_lock, flags);
        /* Ack other interrupt status except RX irqs */
        MHal_NOE_Clear_Sep_Intr_Specific_Status(E_NOE_DIR_TX, E_NOE_INTR_CLR_EXCEPT_RX);
        spin_unlock_irqrestore(&ei_local->irq_lock, flags);
    }

    return IRQ_HANDLED;
}

static irqreturn_t MDrv_NOE_NAPI_Separate_TX_Interrupt(int irq, void *dev_id)
{
    struct net_device *netdev = (struct net_device *)dev_id;
    struct END_DEVICE *ei_local;
    unsigned long flags;

    if (unlikely(!netdev)) {
        NOE_MSG_ERR("%s: irq %x for unknown device.\n", __FUNCTION__, irq);
        return IRQ_NONE;
    }
    ei_local = netdev_priv(netdev);

    if (likely(MHal_NOE_Get_Sep_Intr_Status(E_NOE_DIR_TX) == NOE_TRUE)) {
        if (likely(napi_schedule_prep(&ei_local->napi_tx))) {
            spin_lock_irqsave(&ei_local->irq_lock, flags);

            /* Disable TX interrupt */
            MHal_NOE_Disable_Sep_Intr(E_NOE_DIR_TX);
            spin_unlock_irqrestore(&ei_local->irq_lock, flags);
            __napi_schedule(&ei_local->napi_tx);
        }
    } else {
        spin_lock_irqsave(&ei_local->irq_lock, flags);

        /* Ack other interrupt status except RX irqs */
        MHal_NOE_Clear_Sep_Intr_Specific_Status(E_NOE_DIR_TX, E_NOE_INTR_CLR_EXCEPT_RX);
        spin_unlock_irqrestore(&ei_local->irq_lock, flags);
    }

    return IRQ_HANDLED;
}

static irqreturn_t MDrv_NOE_TX_Interrupt(int irq, void *dev_id)
{
    struct net_device *netdev = (struct net_device *)dev_id;
    struct END_DEVICE *ei_local;
    unsigned long flags;

    if (unlikely(!netdev)) {
        NOE_MSG_ERR("%s: irq %x for unknown device.\n", __FUNCTION__, irq);
        return IRQ_NONE;
    }

    ei_local = netdev_priv(netdev);

    spin_lock_irqsave(&ei_local->irq_lock, flags);


    if (likely(MHal_NOE_Get_Sep_Intr_Status(E_NOE_DIR_TX) == NOE_TRUE)) {
        MHal_NOE_Disable_Sep_Intr(E_NOE_DIR_TX);
        ei_local->ei_xmit_housekeeping(netdev, NUM_TX_MAX_PROCESS);
        /* Enable TX interrupt */
        MHal_NOE_Enable_Sep_Intr(E_NOE_DIR_TX);
    }
    /* Ack other interrupt status except RX irqs */
    MHal_NOE_Clear_Sep_Intr_Specific_Status(E_NOE_DIR_TX, E_NOE_INTR_CLR_EXCEPT_RX);
    spin_unlock_irqrestore(&ei_local->irq_lock, flags);

    return IRQ_HANDLED;
}

static inline void MDrv_NOE_Recv(void)
{
    struct net_device *dev = g_dev_noe;
    struct END_DEVICE *ei_local = netdev_priv(dev);
    int rx_processed;
    unsigned long flags;

    if (ei_local->tx_ring_full == 0) {
        rx_processed = ei_local->ei_eth_recv(dev, NULL, NUM_RX_MAX_PROCESS);
        if (rx_processed > NUM_RX_MAX_PROCESS) {
            if (likely(ei_local->features & FE_INT_TASKLET))
                tasklet_hi_schedule(&ei_local->rx_tasklet);
            else
                schedule_work(&ei_local->rx_wq);
        } else {
            spin_lock_irqsave(&ei_local->irq_lock, flags);
            /* Enable RX interrupt */
            MHal_NOE_Enable_Sep_Intr(E_NOE_DIR_RX);
            spin_unlock_irqrestore(&ei_local->irq_lock, flags);
        }
    } else {
        if (likely(ei_local->features & FE_INT_TASKLET))
            tasklet_schedule(&ei_local->rx_tasklet);
        else
            schedule_work(&ei_local->rx_wq);
    }
}

static void MDrv_NOE_Tasklet_Recv(unsigned long unused)
{
    MDrv_NOE_Recv();
}

static void MDrv_NOE_Workq_Recv(struct work_struct *work)
{
    MDrv_NOE_Recv();
}

static int MDrv_NOE_Enable_Intr(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    int err0 = 0, err1 = 0, err2 = 0;
    unsigned long flags;

    if (ei_local->irq_attached == TRUE)
        return 0;

    if (_MDrv_NOE_Is_IRQ0_Handler_Available(ei_local)) {
        if (ei_local->features & FE_INT_NAPI)
            if (ei_local->features & FE_INT_NAPI_TX_RX)
                err0 = request_irq(ei_local->irq0, MDrv_NOE_NAPI_Separate_Interrupt, IRQF_TRIGGER_HIGH, dev->name, dev);
            else if (ei_local->features & FE_INT_NAPI_RX_ONLY)
                err0 = request_irq(ei_local->irq0, MDrv_NOE_NAPI_RxOnly_Interrupt, IRQF_TRIGGER_HIGH, dev->name, dev);
            else
                err0 = request_irq(ei_local->irq0, MDrv_NOE_NAPI_Interrupt, IRQF_TRIGGER_HIGH, dev->name, dev);
        else
            err0 = request_irq(ei_local->irq0, MDrv_NOE_Interrupt, IRQF_TRIGGER_HIGH, dev->name, dev);

#if defined(CONFIG_MP_PLATFORM_GIC_SET_MULTIPLE_CPUS)
        irq_set_affinity_hint(ei_local->irq0, cpu_online_mask);
        irq_set_affinity(ei_local->irq0, cpu_online_mask);
#endif
    }

    if (ei_local->features & FE_IRQ_SEPARATE) {
        if (ei_local->features & FE_INT_NAPI) {
            if (ei_local->features & FE_INT_NAPI_TX_RX) {
                NOE_MSG_DBG("TX/RX NAPI Sep\n");
                err1 = request_irq(ei_local->irq1, MDrv_NOE_NAPI_Separate_TX_Interrupt, IRQF_TRIGGER_HIGH, "eth_tx", dev);
                err2 = request_irq(ei_local->irq2, MDrv_NOE_NAPI_Separate_RX_Interrupt, IRQF_TRIGGER_HIGH, "eth_rx", dev);
            }
            else if (ei_local->features & FE_INT_NAPI_RX_ONLY) {
                NOE_MSG_DBG("RX NAPI\n");
                err1 = request_irq(ei_local->irq1, MDrv_NOE_TX_Interrupt, IRQF_TRIGGER_HIGH, "eth_tx", dev);
                err2 = request_irq(ei_local->irq2, MDrv_NOE_NAPI_Separate_RX_Interrupt, IRQF_TRIGGER_HIGH, "eth_rx", dev);
            }
            else {
                NOE_MSG_DBG("TX/RX NAPI\n");
                err1 = request_irq(ei_local->irq1, MDrv_NOE_NAPI_TX_Interrupt, IRQF_TRIGGER_HIGH, "eth_tx", dev);
                err2 = request_irq(ei_local->irq2, MDrv_NOE_NAPI_RX_Interrupt, IRQF_TRIGGER_HIGH, "eth_rx", dev);
            }
        }
        else {
            err1 = request_irq(ei_local->irq1, MDrv_NOE_TX_Interrupt, IRQF_TRIGGER_HIGH, "eth_tx", dev);
            err2 = request_irq(ei_local->irq2, MDrv_NOE_RX_Interrupt, IRQF_TRIGGER_HIGH, "eth_rx", dev);
        }
#if defined(CONFIG_MP_PLATFORM_GIC_SET_MULTIPLE_CPUS)
        irq_set_affinity_hint(ei_local->irq1, cpu_online_mask);
        irq_set_affinity_hint(ei_local->irq2, cpu_online_mask);
        irq_set_affinity(ei_local->irq1, cpu_online_mask);
        irq_set_affinity(ei_local->irq2, cpu_online_mask);
#endif

    }

    if (err0 | err1 | err2) {
        NOE_MSG_ERR("Fail to request irq. %d, %d, %d\n", err0, err1, err2);
        return (err0 | err1 | err2);
    }
    MHal_NOE_Enable_Link_Intr();

    spin_lock_irqsave(&ei_local->irq_lock, flags);

    MHal_NOE_Set_Intr_Mask((ei_local->features & FE_DLY_INT)?E_NOE_DLY_ENABLE:E_NOE_DLY_DISABLE);

    /* Enable PDMA interrupts */
    if (ei_local->features & FE_DLY_INT)
        MHal_NOE_Enable_Sep_Delay_Intr(E_NOE_DMA_PACKET, E_NOE_DLY_ENABLE);
    else
        MHal_NOE_Enable_Sep_Delay_Intr(E_NOE_DMA_PACKET, E_NOE_DLY_DISABLE);

    if (ei_local->features & FE_QDMA) {
        /* Enable QDMA interrupts */
        if ((ei_local->features & FE_DLY_INT) || (ei_local->features & FE_INT_NAPI_RX_ONLY))
            MHal_NOE_Enable_Sep_Delay_Intr(E_NOE_DMA_QUEUE, E_NOE_DLY_ENABLE);
        else
            MHal_NOE_Enable_Sep_Delay_Intr(E_NOE_DMA_QUEUE, E_NOE_DLY_DISABLE);
    }

    /* IRQ separation settings */
    if (ei_local->features & FE_IRQ_SEPARATE) {
        if (ei_local->features & FE_DLY_INT)
            MHal_NOE_Set_Grp_Intr(NOE_TRUE, (ei_local->features & FE_INT_NAPI_RX_ONLY)?TRUE:FALSE);
        else
            MHal_NOE_Set_Grp_Intr(NOE_FALSE, FALSE);
    }

    if (ei_local->features & FE_INT_TASKLET) {
        tasklet_init(&ei_local->rx_tasklet, MDrv_NOE_Tasklet_Recv, 0);
    }
    else if (ei_local->features & FE_INT_WORKQ) {
        INIT_WORK(&ei_local->rx_wq, MDrv_NOE_Workq_Recv);
    }
    else {
        if (ei_local->features & FE_INT_NAPI_TX_RX) {
            napi_enable(&ei_local->napi_tx);
            napi_enable(&ei_local->napi_rx);
        }
        else if (ei_local->features & FE_INT_NAPI_RX_ONLY) {
            napi_enable(&ei_local->napi_rx);
        }
        else {
            napi_enable(&ei_local->napi);
        }
    }

    ei_local->irq_attached = TRUE;
    spin_unlock_irqrestore(&ei_local->irq_lock, flags);
    return 0;
}

static int MDrv_NOE_Disable_Intr(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);

    if (ei_local->irq_attached == FALSE)
        return 0;

    if (_MDrv_NOE_Is_IRQ0_Handler_Available(ei_local)) {
        #if defined(CONFIG_MP_PLATFORM_GIC_SET_MULTIPLE_CPUS)
                irq_set_affinity_hint(ei_local->irq0, NULL);
        #endif
        free_irq(ei_local->irq0, dev);
    }

    if (ei_local->features & FE_IRQ_SEPARATE) {
        #if defined(CONFIG_MP_PLATFORM_GIC_SET_MULTIPLE_CPUS)
                irq_set_affinity_hint(ei_local->irq1, NULL);
                irq_set_affinity_hint(ei_local->irq2, NULL);
        #endif
        free_irq(ei_local->irq1, dev);
        free_irq(ei_local->irq2, dev);
    }

    cancel_work_sync(&ei_local->reset_task);

    if (ei_local->features & FE_INT_WORKQ)
        cancel_work_sync(&ei_local->rx_wq);
    else if (ei_local->features & FE_INT_TASKLET)
        tasklet_kill(&ei_local->rx_tasklet);
    if (ei_local->features & FE_INT_NAPI) {
        if (ei_local->features & FE_INT_NAPI_TX_RX) {
            napi_disable(&ei_local->napi_tx);
            napi_disable(&ei_local->napi_rx);
        }
        else if (ei_local->features & FE_INT_NAPI_RX_ONLY) {
            napi_disable(&ei_local->napi_rx);
        }
        else {
            napi_disable(&ei_local->napi);
        }
    }

    ei_local->irq_attached = FALSE;
    return 0;
}

int MDrv_NOE_Set_Forward_Cfg(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);

    if (ei_local->features & FE_HW_VLAN_TX) {
        MHal_NOE_Set_Vlan_Info();
    }

    MHal_NOE_Offoad_Checksum(E_NOE_GE_MAC1, (ei_local->features & FE_CSUM_OFFLOAD)?NOE_ENABLE:NOE_DISABLE);
    if (ei_local->features & FE_GE2_SUPPORT) {
        MHal_NOE_Offoad_Checksum(E_NOE_GE_MAC2, (ei_local->features & FE_CSUM_OFFLOAD)?NOE_ENABLE:NOE_DISABLE);
    }

    if (ei_local->features & FE_HW_VLAN_TX)
        dev->features |= NETIF_F_HW_VLAN_CTAG_TX;

    if (ei_local->features & FE_HW_VLAN_RX)
        dev->features |= NETIF_F_HW_VLAN_CTAG_RX;

    if (ei_local->features & FE_CSUM_OFFLOAD) {
        if (ei_local->features & FE_HW_LRO)
            dev->features |= NETIF_F_HW_CSUM;
        else
            /* Can checksum TCP/UDP over IPv4 */
            dev->features |= NETIF_F_IP_CSUM;

        if (ei_local->features & FE_TSO) {
            dev->features |= NETIF_F_SG;
            dev->features |= NETIF_F_TSO;
        }

        if (ei_local->features & FE_TSO_V6) {
            dev->features |= NETIF_F_TSO6;
            /* Can checksum TCP/UDP over IPv6 */
            dev->features |= NETIF_F_IPV6_CSUM;
        }
    }
    else {
        /* disable checksum TCP/UDP over IPv4 */
        dev->features &= ~NETIF_F_IP_CSUM;
    }

    dev->vlan_features = dev->features;

    MHal_NOE_GLO_Reset();

    return 1;
}



void virtif_setup_statistics(struct PSEUDO_ADAPTER *p_ad)
{
    p_ad->stat.tx_packets = 0;
    p_ad->stat.tx_bytes = 0;
    p_ad->stat.tx_dropped = 0;
    p_ad->stat.tx_errors = 0;
    p_ad->stat.tx_aborted_errors = 0;
    p_ad->stat.tx_carrier_errors = 0;
    p_ad->stat.tx_fifo_errors = 0;
    p_ad->stat.tx_heartbeat_errors = 0;
    p_ad->stat.tx_window_errors = 0;

    p_ad->stat.rx_packets = 0;
    p_ad->stat.rx_bytes = 0;
    p_ad->stat.rx_dropped = 0;
    p_ad->stat.rx_errors = 0;
    p_ad->stat.rx_length_errors = 0;
    p_ad->stat.rx_over_errors = 0;
    p_ad->stat.rx_crc_errors = 0;
    p_ad->stat.rx_frame_errors = 0;
    p_ad->stat.rx_fifo_errors = 0;
    p_ad->stat.rx_missed_errors = 0;

    p_ad->stat.collisions = 0;
}

int MDrv_NOE_Virtif_Open(struct net_device *dev)
{
    struct PSEUDO_ADAPTER *p_pseudo_ad = netdev_priv(dev);

    NOE_MSG_DBG("open %s\n", dev->name);

    virtif_setup_statistics(p_pseudo_ad);

    netif_start_queue(p_pseudo_ad->pseudo_dev);

    return 0;
}

int MDrv_NOE_Virtif_Close(struct net_device *dev)
{
    struct PSEUDO_ADAPTER *p_pseudo_ad = netdev_priv(dev);

    NOE_MSG_DBG("%s: ===> MDrv_NOE_Virtif_Close\n", dev->name);

    netif_stop_queue(p_pseudo_ad->pseudo_dev);

    return 0;
}

int MDrv_NOE_Virtif_Start_Xmit(struct sk_buff *p_skb, struct net_device *dev)
{
    struct PSEUDO_ADAPTER *p_pseudo_ad = netdev_priv(dev);
    struct END_DEVICE *ei_local;

    if (!(p_pseudo_ad->primary_dev->flags & IFF_UP)) {
        NOE_MSG_ERR("primary dev is not up!\n");
        dev_kfree_skb_any(p_skb);
        return 0;
    }
    /* p_skb->cb[40]=0x5a; */
    p_skb->dev = p_pseudo_ad->primary_dev;
    ei_local = netdev_priv(p_pseudo_ad->primary_dev);
    ei_local->ei_start_xmit(p_skb, p_pseudo_ad->primary_dev, 2);
    return 0;
}

struct net_device_stats *MDrv_NOE_Virtif_Get_Stats(struct net_device *dev)
{
    struct PSEUDO_ADAPTER *p_ad = netdev_priv(dev);

    return &p_ad->stat;
}

int MDrv_NOE_Virtif_Ioctl(struct net_device *net_dev, struct ifreq *ifr, int cmd)
{
    struct ra_mii_ioctl_data mii;
    unsigned long ret;

    switch (cmd) {
    case NOE_MII_READ:
        ret = copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
        MHal_NOE_Read_Mii_Mgr(mii.phy_id, mii.reg_num, &mii.val_out);
        ret = copy_to_user(ifr->ifr_data, &mii, sizeof(mii));
        break;

    case NOE_MII_WRITE:
        ret = copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
        MHal_NOE_Write_Mii_Mgr(mii.phy_id, mii.reg_num, mii.val_in);
        break;
    default:
        return -EOPNOTSUPP;
    }

    return 0;
}

static int MDrv_NOE_Change_Mtu(struct net_device *dev, int new_mtu)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);

    if (!ei_local) {
        pr_emerg("%s: %s passed a non-existent private pointer from net_dev!\n", dev->name, __func__);
        return -ENXIO;
    }

    if ((new_mtu > 4096) || (new_mtu < 64))
        return -EINVAL;

    if (new_mtu > 1500)
        return -EINVAL;

    dev->mtu = new_mtu;

    return 0;
}

static void _MDrv_NOE_Enable(struct END_DEVICE *ei_local)
{

    struct noe_sys sys_info;
    /* Set sysctl addr base */
    if (netif_running(ei_local->netdev)) {
        MHal_NOE_Init(NULL);
    }
    else {
        sys_info.sysctl_base = (void __iomem *)(ei_local->netdev->base_addr );
        MHal_NOE_Init(&sys_info);
    }
    //backup MAC address which uboot set before software reset
    MHal_NOE_Get_MAC_Address(E_NOE_GE_MAC1, mac_uboot);
    MHal_NOE_Get_MAC_Address(E_NOE_GE_MAC2, mac1_uboot);

    MHal_NOE_Reset_SW();

    /* Set Pin Mux */
    MHal_NOE_Set_Pin_Mux(ei_local->pin_mux);

    MDrv_NOE_Enable_Clock(ei_local);

    MDrv_NOE_Set_Io_Coherence(ei_local);

    MDrv_NOE_Set_Mac(ei_local->netdev);
}

static int MDrv_NOE_Enable_Clock(struct END_DEVICE *ei_local)
{
#ifndef CONFIG_MSTAR_ARM_BD_FPGA

#endif
    return 0;
}

static int MDrv_NOE_Disable_Clock(struct END_DEVICE *ei_local)
{
#ifndef CONFIG_MSTAR_ARM_BD_FPGA
#endif
    return 0;
}

static struct ethtool_ops mdrv_noe_ethtool_ops = {
    .set_settings = MDrv_NOE_ETHTOOL_Set_Settings,
    .get_settings = MDrv_NOE_ETHTOOL_Get_Settings,
    .get_link = MDrv_NOE_ETHTOOL_Get_Link,
};

static struct ethtool_ops mdrv_noe_virt_ethtool_ops = {
    .set_settings = MDrv_NOE_ETHTOOL_Virt_Set_Settings,
    .get_settings = MDrv_NOE_ETHTOOL_Virt_Get_Settings,
    .get_link = MDrv_NOE_ETHTOOL_Virt_Get_Link,
};

static const struct net_device_ops virtualif_netdev_ops = {
    .ndo_open = MDrv_NOE_Virtif_Open,
    .ndo_stop = MDrv_NOE_Virtif_Close,
    .ndo_start_xmit = MDrv_NOE_Virtif_Start_Xmit,
    .ndo_get_stats = MDrv_NOE_Virtif_Get_Stats,
    .ndo_set_mac_address = MDrv_NOE_Set_Mac2_Addr,
    .ndo_change_mtu = MDrv_NOE_Change_Mtu,
    .ndo_do_ioctl = MDrv_NOE_Virtif_Ioctl,
    .ndo_validate_addr = eth_validate_addr,
};

void MDrv_NOE_Virtif_Init(struct END_DEVICE *p_ad, struct net_device *net_dev)
{
    int index;
    struct net_device *dev;
    struct PSEUDO_ADAPTER *p_pseudo_ad;
    struct END_DEVICE *ei_local = netdev_priv(net_dev);


    for (index = 0; index < MAX_PSEUDO_ENTRY; index++) {
        dev = alloc_etherdev_mqs(sizeof(struct PSEUDO_ADAPTER), GMAC2_TXQ_NUM, 1);
        netif_carrier_off(dev);
        if (!dev) {
            NOE_MSG_ERR("alloc_etherdev for PSEUDO_ADAPTER failed.\n");
            return;
        }

        netif_set_real_num_tx_queues(dev, GMAC2_TXQ_NUM);
        netif_set_real_num_rx_queues(dev, 1);

        MDrv_NOE_Set_Mac2(dev);

        ether_setup(dev);
        p_pseudo_ad = netdev_priv(dev);

        p_pseudo_ad->pseudo_dev = dev;
        p_pseudo_ad->primary_dev = net_dev;
        p_ad->pseudo_dev = dev;

        dev->netdev_ops = &virtualif_netdev_ops;

        if (ei_local->features & FE_HW_LRO)
            dev->features |= NETIF_F_HW_CSUM;
        else
            /* Can checksum TCP/UDP over IPv4 */
            dev->features |= NETIF_F_IP_CSUM;

        if (ei_local->features & FE_TSO) {
            dev->features |= NETIF_F_SG;
            dev->features |= NETIF_F_TSO;
        }

        if (ei_local->features & FE_TSO_V6) {
            dev->features |= NETIF_F_TSO6;
            /* Can checksum TCP/UDP over IPv6 */
            dev->features |= NETIF_F_IPV6_CSUM;
        }

        dev->vlan_features = dev->features;

        if (ei_local->features & FE_ETHTOOL) {
            dev->ethtool_ops = &mdrv_noe_virt_ethtool_ops;
            MDrv_NOE_ETHTOOL_Virt_Init(dev);
        }

        /* Register this device */
        register_netdevice(dev);
    }
}

void MDrv_NOE_Set_Lro_Ip(char *lan_ip_addr)
{
    unsigned int lan_ip;
    MDrv_NOE_UTIL_Str_To_Ip(&lan_ip, lan_ip_addr);
    lan_ip = htonl(lan_ip);
    lro_para.lan_ip1 = lan_ip;

    NOE_MSG_DBG("[%s]lan_ip_addr = %s (lan_ip = 0x%x)\n", __func__, lan_ip_addr, lan_ip);
}

int MDrv_NOE_Open(struct net_device *dev)
{
    int err;
    struct END_DEVICE *ei_local;

    ei_local = netdev_priv(dev);
    if (!ei_local) {
        NOE_MSG_ERR("%s: MDrv_NOE_Open passed a non-existent device!\n", dev->name);
        return -ENXIO;
    }

    if (!try_module_get(THIS_MODULE)) {
        NOE_MSG_ERR("%s: Cannot reserve module\n", __func__);
        return -1;
    }

    if (ei_local->features & FE_INT_NAPI)
        NOE_MSG_MUST("Bottom Half : NAPI\n");
    else if (ei_local->features & FE_INT_TASKLET)
        NOE_MSG_MUST("Bottom Half : Tasklet\n");
    else if (ei_local->features & FE_INT_WORKQ)
        NOE_MSG_MUST("Bottom Half : Workqueue\n");

    MDrv_NOE_Reset_Statistics(ei_local);
    err = MDrv_NOE_Init_Dma(dev);
    if (err)
        return err;

    MHal_NOE_Reset_GMAC();

    /* initialize fe and switch register */
    MDrv_NOE_MAC_Init(dev);

    if (ei_local->features & FE_GE2_SUPPORT) {
        if (!ei_local->pseudo_dev)
            MDrv_NOE_Virtif_Init(ei_local, dev);

        if (!ei_local->pseudo_dev) {
            NOE_MSG_ERR("Open pseudo_dev failed.\n");
        }
        else
            MDrv_NOE_Virtif_Open(ei_local->pseudo_dev);
    }

    if (ei_local->features & FE_SW_LRO)
        MDrv_NOE_Set_Lro_Ip(ei_local->lan_ip4_addr);

    MDrv_NOE_Set_Forward_Cfg(dev);

    if ((MHal_NOE_Need_Reset() == E_NOE_RET_TRUE) && (ei_local->features & FE_HW_LRO)) {
        ei_local->kreset_task = kthread_create(MDrv_NOE_Reset_Thread, NULL, "FE_reset_kthread");
        if (IS_ERR(ei_local->kreset_task))
            return PTR_ERR(ei_local->kreset_task);
        wake_up_process(ei_local->kreset_task);
    }

    netif_start_queue(dev);
    MDrv_NOE_Enable_Intr(dev);

    return 0;
}

int MDrv_NOE_Close(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);

    MHal_NOE_Reset_FE();
    if ((MHal_NOE_Need_Reset() == E_NOE_RET_TRUE) && (ei_local->features & FE_HW_LRO))
        kthread_stop(ei_local->kreset_task);

    netif_stop_queue(dev);
    MHal_NOE_Stop();

    MDrv_NOE_Disable_Intr(dev);

    if (ei_local->features & FE_GE2_SUPPORT)
        MDrv_NOE_Virtif_Close(ei_local->pseudo_dev);

    MDrv_NOE_Deinit_Dma(dev);


    module_put(THIS_MODULE);

    return 0;
}

static int MDrv_NOE_Start_Xmit(struct sk_buff *skb, struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    return ei_local->ei_start_xmit(skb, dev, 1);
}

struct net_device_stats *MDrv_NOE_Get_Stats(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    return &ei_local->stat;
}



int MDrv_NOE_Ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    unsigned int lan_ip;
    int ret = 0;
    unsigned long result;
    struct END_DEVICE *ei_local = netdev_priv(dev);
    struct ra_mii_ioctl_data mii;

    if (ei_local->features & FE_QDMA) {
        ret = MDrv_NOE_QDMA_Ioctl(dev, ifr, cmd);
        if (ret <= 0)
            return ret;
    }

    spin_lock_irq(&ei_local->page_lock);
    switch (cmd) {
    case NOE_MII_READ:
        result = copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
        MHal_NOE_Read_Mii_Mgr(mii.phy_id, mii.reg_num, &mii.val_out);
        result = copy_to_user(ifr->ifr_data, &mii, sizeof(mii));
        break;

    case NOE_MII_WRITE:
        result = copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
        MHal_NOE_Write_Mii_Mgr(mii.phy_id, mii.reg_num, mii.val_in);
        break;
    case NOE_MII_READ_CL45:
        result = copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
        MHal_NOE_Read45_Mii_Mgr(mii.port_num, mii.dev_addr, mii.reg_addr, &mii.val_out);
        result = copy_to_user(ifr->ifr_data, &mii, sizeof(mii));
        break;
    case NOE_MII_WRITE_CL45:
        result = copy_from_user(&mii, ifr->ifr_data, sizeof(mii));
        MHal_NOE_Write45_Mii_Mgr(mii.port_num, mii.dev_addr, mii.reg_addr, mii.val_in);
        break;
    case NOE_SET_LAN_IP:
        result = copy_from_user(ei_local->lan_ip4_addr, ifr->ifr_data, IP4_ADDR_LEN);
        NOE_MSG_DUMP("NOE_SET_LAN_IP: %s\n", ei_local->lan_ip4_addr);

        if (ei_local->features & FE_SW_LRO)
            MDrv_NOE_Set_Lro_Ip(ei_local->lan_ip4_addr);

        if (ei_local->features & FE_HW_LRO) {
            MDrv_NOE_UTIL_Str_To_Ip(&lan_ip, ei_local->lan_ip4_addr);
            MHal_NOE_LRO_Set_Ip(lan_ip);
        }

        break;

    default:
        ret = -EOPNOTSUPP;
        break;
    }

    spin_unlock_irq(&ei_local->page_lock);
    return ret;
}

static const struct net_device_ops mdrv_noe_netdev_ops = {
    .ndo_init = MDrv_NOE_Init,
    .ndo_uninit = MDrv_NOE_Uninit,
    .ndo_open = MDrv_NOE_Open,
    .ndo_stop = MDrv_NOE_Close,
    .ndo_start_xmit = MDrv_NOE_Start_Xmit,
    .ndo_get_stats = MDrv_NOE_Get_Stats,
    .ndo_set_mac_address = MDrv_NOE_Set_Mac_Addr,
    .ndo_change_mtu = MDrv_NOE_Change_Mtu,
    .ndo_do_ioctl = MDrv_NOE_Ioctl,
    .ndo_validate_addr = eth_validate_addr,
#ifdef CONFIG_NET_POLL_CONTROLLER
    .ndo_poll_controller = MDrv_NOE_Full_Poll,
#endif
};

void MDrv_NOE_Setup_Fptable(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);

    dev->netdev_ops = &mdrv_noe_netdev_ops;

    if (ei_local->features & FE_ETHTOOL)
        dev->ethtool_ops = &mdrv_noe_ethtool_ops;

#define TX_TIMEOUT (5 * HZ)
    dev->watchdog_timeo = TX_TIMEOUT;
}

void MDrv_NOE_Set_Io_Coherence(struct END_DEVICE *ei_local)
{
    if (ei_local->features & FE_HW_IOCOHERENT) {
        NOE_MSG_DBG("[NOE] HW IO coherent is enabled !\n");
        MHal_NOE_IO_Enable_Coherence((ei_local->features & FE_QDMA_FQOS)?TRUE:FALSE);
        NOE_SETUP_COHERENT_DMA_OPS(ei_local->dev);
    } else {
        NOE_MSG_DBG("[NOE] HW IO coherent is disabled !\n");
        NOE_SETUP_NONCOHERENT_DMA_OPS(ei_local->dev);
    }
}

#ifdef CONFIG_PM
static int MDrv_NOE_Resume(struct platform_device *pdev)
{
    struct END_DEVICE *ei_local = netdev_priv(g_dev_noe);
    NOE_MSG_DBG("[NOE] Resume!\n");
    _MDrv_NOE_Enable(ei_local);
    if (netif_running(ei_local->netdev)) {
        NOE_MSG_DBG("noe open\n");
        MDrv_NOE_Open(ei_local->netdev);
    }
    return 0;
}

static inline void _MDrv_NOE_Dma_DeInit(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    MHal_NOE_Dma_DeInit_Global_Config(E_NOE_DMA_PACKET);

    MHal_NOE_Dma_DeInit_Global_Config((ei_local->features & FE_HW_SFQ)? E_NOE_DMA_QUEUE_WITH_SFQ :E_NOE_DMA_QUEUE);
}


static int MDrv_NOE_Suspend(struct platform_device *pdev, pm_message_t state)
{
    struct END_DEVICE *ei_local = netdev_priv(g_dev_noe);
    NOE_MSG_DBG("[NOE] Suspend!\n");
    MDrv_NOE_Close(ei_local->netdev);
    MDrv_NOE_Disable_Clock(ei_local);
    _MDrv_NOE_Dma_DeInit(ei_local->netdev);
    return 0;
}
#endif
/* static struct wakeup_source eth_wake_lock; */

static ssize_t ms_noe_debug_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
   
    return (str - buf);
}

static ssize_t ms_noe_debug_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {

        u32 input;
        input = simple_strtoul(buf, NULL, 10);
        NOE_MSG_ERR("debug input=%d\n", input);

        if(input == 0)
        {
            MHal_NOE_Reset_FE();
            return n;
        }
        else if(input == 1)
        {
            MHal_NOE_Stop();
            return n;
        }
        else if(input == 2)
        {
            MDrv_NOE_Disable_Intr(g_dev_noe);
            return n;
        }
        else if(input == 3)
        {
            MDrv_NOE_Deinit_Dma(g_dev_noe);
            return n;
        }
        else if(input == 4)
        {
            MDrv_NOE_Init_Dma(g_dev_noe);
            return n;
        }
        else if(input == 5)
        {
            MDrv_NOE_Enable_Intr(g_dev_noe);
            return n;
        }
        else if(input == 6)
        {
            MDrv_NOE_MAC_Init(g_dev_noe);
            MDrv_NOE_Set_Forward_Cfg(g_dev_noe);
            return n;
        }
        else if(input == 7)
        {
            MHal_NOE_Reset_SW();
            return n;
        }
        else if (input == 8)
        {
            struct END_DEVICE *ei_local = netdev_priv(g_dev_noe);
            _MDrv_NOE_Enable(ei_local);
        }
        return n;

    }
    return -EINVAL;
}
DEVICE_ATTR(debug, 0644, ms_noe_debug_show, ms_noe_debug_store);


static ssize_t noe_mdio_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    struct END_DEVICE *ei_local = netdev_priv(g_dev_noe);
    unsigned int physaddr, reg=0, value=0;

    physaddr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR;
    str += scnprintf(str, end - str, "%s physaddr:%d\n",g_dev_noe->name , physaddr);
    for(reg=0; reg<32; reg++)
    {
        if(reg%8==0)  str += scnprintf(str, end - str, "%02d: ", reg);
        MHal_NOE_Read_Mii_Mgr(physaddr, reg, &value);
        str += scnprintf(str, end - str, "0x%04x ", value);
        if(reg%8==7)  str += scnprintf(str, end - str, "\n");
    }

    if (ei_local->features & FE_GE2_SUPPORT)
    {
        physaddr = CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2;
        str += scnprintf(str, end - str, "%s physaddr:%d\n",ei_local->pseudo_dev->name , physaddr);
        for(reg=0; reg<32; reg++)
        {
            if(reg%8==0)  str += scnprintf(str, end - str, "%02d: ", reg);
            MHal_NOE_Read_Mii_Mgr(physaddr, reg, &value);
            str += scnprintf(str, end - str, "0x%04x ", value);
            if(reg%8==7)  str += scnprintf(str, end - str, "\n");
        }

    }
    return (str - buf);
}

static ssize_t noe_mdio_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    unsigned int physaddr, reg=0, value=0;
    unsigned char token[16];

    sscanf(buf, "%s", token);
    if (0 == strcasecmp(token, "r"))
    {
        sscanf(buf, "%s %d %d", token, &physaddr, &reg);
        MHal_NOE_Read_Mii_Mgr(physaddr, reg, &value);
        printk("mdio %s phyaddr:%d reg:%d val:0x%04x\n", token, physaddr, reg, value);
    }
    else if (0 == strcasecmp(token, "w"))
    {
        sscanf(buf, "%s %d %d 0x%x", token, &physaddr, &reg, &value);
        MHal_NOE_Write_Mii_Mgr(physaddr, reg, value);
        printk("mdio %s phyaddr:%d reg:%d val:0x%04x\n", token, physaddr, reg, value);
    }
    else
    {
        printk("\nUsage: echo r [phyaddr] [reg] > mdio (r 3 0)\n");
        printk("Usage: echo w [phyaddr] [reg] [value_hex] > mdio (w 3 0 0x9140)\n");
    }

    return n;
}
DEVICE_ATTR(mdio, 0644, noe_mdio_show, noe_mdio_store);

#ifdef CONFIG_MDIO_IC1819
static ssize_t noe_mdio_sw_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    char *end = buf + PAGE_SIZE;
    struct END_DEVICE *ei_local = netdev_priv(g_dev_noe);
    unsigned int i, physaddr, reg=0, value=0;

    for(i = 0; i < 1; i++)
    {
        physaddr = i;
        str += scnprintf(str, end - str, "%s physaddr:%d\n",ei_local->pseudo_dev->name , physaddr);
        for(reg=0; reg<32; reg++)
        {
            if(reg%8==0)  str += scnprintf(str, end - str, "%02d: ", reg);
            value = mdio_bb_read(physaddr, reg);
            str += scnprintf(str, end - str, "0x%04x ", value);
            if(reg%8==7)  str += scnprintf(str, end - str, "\n");
        }
    }
    return (str - buf);
}

static ssize_t noe_mdio_sw_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    unsigned int physaddr, reg=0, value=0;
    unsigned char token[16];

    sscanf(buf, "%s", token);
    if (0 == strcasecmp(token, "r"))
    {
        sscanf(buf, "%s %d %d", token, &physaddr, &reg);
        value = mdio_bb_read(physaddr, reg);
        printk("mdio %s phyaddr:%d reg:%d val:0x%04x\n", token, physaddr, reg, value);
    }
    else if (0 == strcasecmp(token, "w"))
    {
        sscanf(buf, "%s %d %d 0x%x", token, &physaddr, &reg, &value);
        mdio_bb_write(physaddr, reg, value);
        printk("mdio %s phyaddr:%d reg:%d val:0x%04x\n", token, physaddr, reg, value);
    }
    else
    {
        printk("\nUsage: echo r [phyaddr] [reg] > mdio (r 3 0)\n");
        printk("Usage: echo w [phyaddr] [reg] [value_hex] > mdio (w 3 0 0x9140)\n");
    }

    return n;
}
DEVICE_ATTR(mdio_sw, 0644, noe_mdio_sw_show, noe_mdio_sw_store);
#endif

static ssize_t noe_io_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    unsigned int addr=0, value=0;
    unsigned char token[16];

    sscanf(buf, "%s", token);
    if (0 == strcasecmp(token, "r"))
    {
        sscanf(buf, "%s 0x%x", token, &addr);
        value = (*(volatile unsigned int *)(g_dev_noe->base_addr+addr));
        printk("noe_io %s addr:0x%08x val:0x%08x\n", token, addr, value);
    }
    else if (0 == strcasecmp(token, "w"))
    {
        sscanf(buf, "%s 0x%x 0x%x", token, &addr, &value);
        (*(volatile unsigned int *)(g_dev_noe->base_addr+addr)) = (value);
        printk("noe_io %s addr:0x%08x val:0x%08x\n", token, addr, value);
    }
    else
    {
        printk("\nUsage: echo r [addr_hex] > noe_io\n");
        printk("Usage: echo w [addr_hex] [value_hex] > noe_io\n");
    }

    return n;
}
DEVICE_ATTR(noe_io, 0200, NULL, noe_io_store);

static int MDrv_NOE_Probe(struct platform_device *pdev)
{
    struct END_DEVICE *ei_local;
    struct net_device *netdev;
    struct device_node *node = NULL;
    const char *mac_addr;
    int ret;
    void __iomem *ethdma_sysctl_base = NULL;
    struct noe_irq irq_info;

    netdev = alloc_etherdev_mqs(sizeof(struct END_DEVICE), GMAC1_TXQ_NUM, 1);
    netif_carrier_off(netdev);
    if (!netdev)
        return -ENOMEM;

    MDrv_NOE_LOG_Init(netdev);
    SET_NETDEV_DEV(netdev, &pdev->dev);

    g_dev_noe = netdev;
    ei_local = netdev_priv(netdev);
    ei_local->dev = &pdev->dev;
    ei_local->dev->dma_mask = &noe_dmamask;
    ei_local->dev->coherent_dma_mask  = 0xffffffff;
    ei_local->netdev = netdev;
    ei_local->irq_attached = FALSE;
    MDrv_NOE_CONFIG_Set_Features(ei_local);

    MDrv_NOE_CONFIG_Set_Architecture(ei_local);
    MDrv_NOE_LOG_Init(netdev);
    MDrv_NOE_LOG_Set_Level(E_MDRV_NOE_MSG_CTRL_DBG);

    NOE_MSG_DBG("features=0x%x,architecture=0x%x\n", ei_local->features, ei_local->architecture);

    if ((ei_local->features & FE_HW_IOCOHERENT) && (ei_local->features & FE_QDMA_FQOS)) {
        ei_local->qdma_pdev = platform_device_alloc("QDMA", PLATFORM_DEVID_AUTO);
        if (!ei_local->qdma_pdev) {
            dev_err(&pdev->dev, "QDMA platform device allocate fail!\n");
            ret = -ENOMEM;
            goto err_free_dev;
        }

        ei_local->qdma_pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);
        ei_local->qdma_pdev->dev.dma_mask = &ei_local->qdma_pdev->dev.coherent_dma_mask;
    }
    else {
        ei_local->qdma_pdev = pdev;
    }
    /* iomap registers */
#ifdef CONFIG_OF
    node = of_parse_phandle(pdev->dev.of_node, NOE_COMPATIBLE_ETHSYS, 0);
    if (node != NULL) {
        ethdma_sysctl_base = of_iomap(node, 0);
    }
    else {
        ethdma_sysctl_base = (void *)ioremap(NOE_PHYS, NOE_SIZE);
    }
#endif

    if (ethdma_sysctl_base == NULL) {
        ethdma_sysctl_base = (void *)ioremap(NOE_PHYS, NOE_SIZE);
    }

    if (IS_ERR(ethdma_sysctl_base)) {
        NOE_MSG_ERR("no ethdma_sysctl_base found\n");
        return -ENOMEM;
    }
    NOE_MSG_DBG("ethdma_sysctl_base = 0x%p\n", ethdma_sysctl_base);

    /* get MAC address */
#ifdef CONFIG_OF
    mac_addr = of_get_mac_address(pdev->dev.of_node);
#endif
    if (mac_addr)
        ether_addr_copy(netdev->dev_addr, mac_addr);

    /* Set Pin Mux */
    ei_local->pin_mux = NOE_CFG_PIN_MUX_SEL;
    
    NOE_MSG_DBG("pin sel = %4d\n", ei_local->pin_mux);
    /* get IRQs */
    MHAL_NOE_Get_Interrupt(&irq_info);
    ei_local->irq_num = irq_info.num;
#ifdef CONFIG_OF
    if (_MDrv_NOE_Is_IRQ0_Handler_Available(ei_local) == FALSE) {
        ei_local->irq1 = platform_get_irq(pdev, 0);
        ei_local->irq2 = platform_get_irq(pdev, 1);
    }
    else {
        ei_local->irq0 = platform_get_irq(pdev, 0);
        ei_local->irq1 = platform_get_irq(pdev, 1);
        ei_local->irq2 = platform_get_irq(pdev, 2);

        if ((int)ei_local->irq0 < 0) {
            NOE_MSG_DBG("no irq0 info in dts. set by irq_info\n");
            ei_local->irq0 = irq_info.irq[E_NOE_IRQ_0];
        }
    }

    if ((int)ei_local->irq1 < 0) {
        NOE_MSG_DBG("no irq1 info in dts. set by irq_info\n");
        ei_local->irq1 = irq_info.irq[E_NOE_IRQ_1];
    }

    if ((int)ei_local->irq2 < 0) {
        NOE_MSG_DBG("no irq2 info in dts. set by irq_info\n");
        ei_local->irq2 = irq_info.irq[E_NOE_IRQ_2];
    }
#else
    ei_local->irq0 = irq_info.irq[E_NOE_IRQ_0];
    ei_local->irq1 = irq_info.irq[E_NOE_IRQ_1];
    ei_local->irq2 = irq_info.irq[E_NOE_IRQ_2];
#endif

    NOE_MSG_DBG("irqs(%d) = %4d, %4d, %4d\n", ei_local->irq_num, ei_local->irq0, ei_local->irq1, ei_local->irq2);

    MDrv_NOE_Setup_Fptable(netdev);

    netif_set_real_num_tx_queues(netdev, GMAC1_TXQ_NUM);
    netif_set_real_num_rx_queues(netdev, 1);

    netdev->addr_len = 6;
    netdev->base_addr = (unsigned long)(ethdma_sysctl_base);

    _MDrv_NOE_Enable(ei_local);

    gdebug_class_dev = device_create(msys_get_sysfs_class(), NULL, MKDEV(241, 2), NULL, "noe");
    device_create_file(gdebug_class_dev, &dev_attr_debug);
    device_create_file(gdebug_class_dev, &dev_attr_mdio);
    device_create_file(gdebug_class_dev, &dev_attr_noe_io);

    #ifdef CONFIG_MDIO_IC1819
    device_create_file(gdebug_class_dev, &dev_attr_mdio_sw);

    //set ic1819 force link
    mdio_bb_write(0, 255, 3);   //change to page3, reg:0xff, val:0x03
    mdio_bb_write(0, 93, 256);  //reg:5D, val:0x100
    mdio_bb_write(0, 255, 0);   //change back to page0
    #endif

    /* net_device structure Init */
    NOE_MSG_DBG("NOE Ethernet Driver Initialization.\n");
    NOE_MSG_DBG("%s  %d rx/%d tx descriptors allocated, mtu = %d!\n", "noe", NUM_RX_DESC, NUM_TX_DESC, netdev->mtu);

    if (ei_local->features & FE_INT_NAPI)
        NOE_MSG_DBG("NAPI enable, Tx Ring = %d, Rx Ring = %d\n", NUM_TX_DESC, NUM_RX_DESC);

    if (ei_local->features & FE_ETHTOOL)
        MDrv_NOE_ETHTOOL_Init(netdev);

    ret = MDrv_NOE_PROC_Init(netdev);
    if (ret) {
        dev_err(&pdev->dev, "error set debug proc\n");
        goto err_free_dev;
    }
    /* Register net device for the driver */
    ret = register_netdev(netdev);
    if (ret) {
        dev_err(&pdev->dev, "error bringing up device\n");
        goto err_free_dev;
    }

    /*keep power domain on*/
    device_init_wakeup(&pdev->dev, true);

    return 0;

err_free_dev:
    free_netdev(netdev);
    return ret;
}

static int MDrv_NOE_Remove(struct platform_device *pdev)
{
    struct END_DEVICE *ei_local = netdev_priv(g_dev_noe);
    MDrv_NOE_Disable_Clock(ei_local);
    return 0;
}

struct net_device *MDrv_NOE_Get_Dev(EN_NOE_DEV_IDX idx)
{

    struct END_DEVICE *ei_local = netdev_priv(g_dev_noe);
    char ifname[IFNAMSIZ];

    if (idx == E_NOE_DEV_MAIN) {
        if ((ei_local != NULL) && (ei_local->netdev != NULL)) {
            return ei_local->netdev;
        }
    }
    else if (idx == E_NOE_DEV_PSEUDO) {
        if ((ei_local != NULL) && (ei_local->pseudo_dev != NULL)) {
            return ei_local->pseudo_dev;
        }
    }
    else if (idx == E_NOE_DEV_LAN) {
        if ((ei_local != NULL) && (ei_local->netdev != NULL)) {
            sprintf(ifname,"%s.1",ei_local->netdev->name);
            return dev_get_by_name(&init_net, ifname);
        }
    }
    else if (idx == E_NOE_DEV_WAN) {
        if ((ei_local != NULL) && (ei_local->netdev != NULL)) {
            sprintf(ifname,"%s.2",ei_local->netdev->name);
            return dev_get_by_name(&init_net, ifname);
        }
    }

    return NULL;
}

EXPORT_SYMBOL(MDrv_NOE_Get_Dev);

module_platform_driver(mstar_noe_driver);
MODULE_DESCRIPTION("NOE Ethernet driver");
MODULE_LICENSE("GPL");
