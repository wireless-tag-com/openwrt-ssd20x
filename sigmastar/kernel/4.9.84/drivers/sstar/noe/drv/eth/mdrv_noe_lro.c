/*
* mdrv_noe_lro.c- Sigmastar
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
/// @file   MDrv_NOE_Lro.c
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
#include "mhal_noe_lro.h"


//-------------------------------------------------------------------------------------------------
//  Function
//-------------------------------------------------------------------------------------------------

static int _MDrv_NOE_LRO_Alloc_Mem(struct net_device *dev, int i, int j, int skb_size)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    ei_local->netrx_skb_data[i][j] = MDrv_NOE_SkbData_Alloc(skb_size, GFP_KERNEL);
    if (!ei_local->netrx_skb_data[i][j]) {
        NOE_MSG_ERR("rx skbuff buffer allocation failed!\n");
        return 0;
    }
    return 1;
}

int MDrv_NOE_LRO_Init(struct net_device *dev)
{
    struct noe_dma_info info;
    struct END_DEVICE *ei_local = netdev_priv(dev);
    int skb_size;
    int i, j;
    unsigned int sip, dip, lan_ip;
    MSYS_DMEM_INFO mem_info;
    int ret;

    skb_size = SKB_DATA_ALIGN(MAX_LRO_RX_LENGTH + NET_IP_ALIGN) + SKB_DATA_ALIGN(sizeof(struct skb_shared_info));

    /* Initial RX Ring 1 ~ 3 */
    for (i = 1; i < MAX_RX_RING_NUM; i++) {
        //ei_local->rx_ring[i] = dma_alloc_coherent(dev->dev.parent, NUM_LRO_RX_DESC * sizeof(struct PDMA_rxdesc), &ei_local->phy_rx_ring[i], GFP_ATOMIC | __GFP_ZERO);
        mem_info.length = NUM_LRO_RX_DESC * sizeof(struct PDMA_rxdesc);
        snprintf(mem_info.name, 16, "rx_ring%d", i);
        if((ret=msys_request_dmem(&mem_info)))
        {
            NOE_MSG_ERR("unable to allocate DMEM for  %s!! error=%d\n", mem_info.name ,ret);
        }
        ei_local->phy_rx_ring[i] = (dma_addr_t)((size_t)mem_info.phys);
        ei_local->rx_ring[i] = (void *)((size_t)mem_info.kvirt);

        for (j = 0; j < NUM_LRO_RX_DESC; j++) {
            if(!_MDrv_NOE_LRO_Alloc_Mem(dev, i, j, skb_size)) {
                goto no_rx_mem;
            }

            memset(&ei_local->rx_ring[i][j], 0, sizeof(struct PDMA_rxdesc));
            ei_local->rx_ring[i][j].rxd_info2.DDONE_bit = 0;
            ei_local->rx_ring[i][j].rxd_info2.LS0 = 0;
            ei_local->rx_ring[i][j].rxd_info2.PLEN0 = SET_ADMA_RX_LEN0(MAX_LRO_RX_LENGTH);
            ei_local->rx_ring[i][j].rxd_info2.PLEN1 = SET_ADMA_RX_LEN1(MAX_LRO_RX_LENGTH >> 14);
            ei_local->rx_ring[i][j].rxd_info1.PDP0 = dma_map_single(dev->dev.parent, ei_local->netrx_skb_data[i][j] + NET_SKB_PAD, MAX_LRO_RX_LENGTH, DMA_FROM_DEVICE);
            if (unlikely(dma_mapping_error(ei_local->dev, ei_local->rx_ring[i][j].rxd_info1.PDP0))) {
                NOE_MSG_ERR("[%s]dma_map_single() failed...\n",__FUNCTION__);
                goto no_rx_mem;
            }
        }
        NOE_MSG_DUMP("\nphy_rx_ring[%d] = 0x%08x, rx_ring[%d] = 0x%p, NUM_LRO_RX_DESC=%d\n", i, (unsigned int)ei_local->phy_rx_ring[i], i, ei_local->rx_ring[i], NUM_LRO_RX_DESC);
    }

    info.ring_st.base_adr = (MS_U32) ei_local->phy_rx_ring[3];
    info.ring_st.max_cnt = cpu_to_le32((u32)NUM_LRO_RX_DESC);
    info.ring_st.cpu_idx = cpu_to_le32((u32)(NUM_LRO_RX_DESC - 1));
    MHal_NOE_DMA_Init(E_NOE_DMA_PACKET, E_NOE_DIR_RX, E_NOE_RING_NO3, &info);
    info.ring_st.base_adr = (MS_U32) ei_local->phy_rx_ring[2];
    MHal_NOE_DMA_Init(E_NOE_DMA_PACKET, E_NOE_DIR_RX, E_NOE_RING_NO2, &info);
    info.ring_st.base_adr = (MS_U32) ei_local->phy_rx_ring[1];
    MHal_NOE_DMA_Init(E_NOE_DMA_PACKET, E_NOE_DIR_RX, E_NOE_RING_NO1, &info);

    if (ei_local->features & FE_HW_LRO_FPORT) {
        MDrv_NOE_UTIL_Str_To_Ip(&sip, "10.10.10.3");
        MDrv_NOE_UTIL_Str_To_Ip(&dip, "10.10.10.254");
        MHal_NOE_LRO_Set_Ring_Cfg(E_NOE_RING_NO1, sip, 1122, dip, 3344);
        MHal_NOE_LRO_Set_Ring_Cfg(E_NOE_RING_NO2, sip, 5566, dip, 7788);
        MHal_NOE_LRO_Set_Ring_Cfg(E_NOE_RING_NO3, sip, 9900, dip, 99);
        MHal_NOE_LRO_Set_Cfg();
    } else {
        MDrv_NOE_UTIL_Str_To_Ip(&lan_ip, ei_local->lan_ip4_addr);
        MHal_NOE_LRO_Set_Ip(lan_ip);
        MHal_NOE_LRO_Set_Auto_Learn_Cfg();
    }

    return 0;

no_rx_mem:
    NOE_MSG_ERR("%s fail \n", __FUNCTION__);
    return -ENOMEM;
}



void MDrv_NOE_LRO_Deinit(struct net_device *dev)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    int i, j;

    for (i = 1; i < MAX_RX_RING_NUM; i++) {
        /* free RX Ring */
        //dma_free_coherent(dev->dev.parent, NUM_LRO_RX_DESC * sizeof(struct PDMA_rxdesc), ei_local->rx_ring[i], ei_local->phy_rx_ring[i]);
        /* free RX data */
        for (j = 0; j < NUM_LRO_RX_DESC; j++) {
            MDrv_NOE_SkbData_Free(ei_local->netrx_skb_data[i][j]);
            ei_local->netrx_skb_data[i][j] = NULL;
        }
    }
}

static inline void _MDrv_NOE_LRO_Init_Rx_Desc(struct END_DEVICE *ei_local, struct PDMA_rxdesc *rx_ring, unsigned int rx_ring_no, dma_addr_t dma_addr)
{
    if (rx_ring_no != 0) {
        /* lro ring */
        rx_ring->rxd_info2.PLEN0 = SET_ADMA_RX_LEN0(MAX_LRO_RX_LENGTH);
        rx_ring->rxd_info2.PLEN1 = SET_ADMA_RX_LEN1(MAX_LRO_RX_LENGTH >> 14);
    }
    else{
        /* normal ring */
        rx_ring->rxd_info2.PLEN0 = MAX_RX_LENGTH;
    }
    rx_ring->rxd_info1.PDP0 = dma_addr;
    rx_ring->rxd_info2.LS0 = 0;
    rx_ring->rxd_info2.DDONE_bit = 0;

    memset(&rx_ring->rxd_info3, 0 ,sizeof(struct PDMA_RXD_INFO3_T));
    memset(&rx_ring->rxd_info4, 0 ,sizeof(struct PDMA_RXD_INFO4_T));

    Chip_Flush_MIU_Pipe(); //L3
}

static int _MDrv_NOE_LRO_Get_Rx_Ring(struct END_DEVICE *ei_local, unsigned int rx_idx[])
{
    int i;

    for (i = 0; i < MAX_RX_RING_NUM; i++) {
        if (ei_local->rx_ring[i][rx_idx[i]].rxd_info2.DDONE_bit == 1){
            return i;
        }
    }
    return 0;
}

int MDrv_NOE_LRO_Recv(struct net_device *dev, struct napi_struct *napi, int budget)
{
    struct END_DEVICE *ei_local = netdev_priv(dev);
    struct PSEUDO_ADAPTER *p_ad = netdev_priv(ei_local->pseudo_dev);
    struct sk_buff *rx_skb;
    struct PDMA_rxdesc *rx_ring, *rx_ring_next;
    void *rx_data, *rx_data_next, *new_data;
    unsigned int length = 0;
    unsigned int rx_ring_no = 0, rx_ring_no_next = 0;
    unsigned int rx_dma_owner_idx, rx_dma_owner_idx_next;
    unsigned int rx_dma_owner_lro[MAX_RX_RING_NUM];
    unsigned int skb_size, map_size;
    int rx_processed = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
        DEFINE_DMA_ATTRS(attrs);
#else
        unsigned long attrs;
#endif


    /* get cpu owner indexes of rx rings */
    rx_dma_owner_lro[0] = (ei_local->rx_calc_idx[0] + 1) % NUM_RX_DESC;
    rx_dma_owner_lro[1] = (ei_local->rx_calc_idx[1] + 1) % NUM_LRO_RX_DESC;
    rx_dma_owner_lro[2] = (ei_local->rx_calc_idx[2] + 1) % NUM_LRO_RX_DESC;
    rx_dma_owner_lro[3] = (ei_local->rx_calc_idx[3] + 1) % NUM_LRO_RX_DESC;
    rx_ring_no =  _MDrv_NOE_LRO_Get_Rx_Ring(ei_local, rx_dma_owner_lro);
    rx_dma_owner_idx = rx_dma_owner_lro[rx_ring_no];
    rx_ring = &ei_local->rx_ring[rx_ring_no][rx_dma_owner_idx];
    rx_data = ei_local->netrx_skb_data[rx_ring_no][rx_dma_owner_idx];

    for (;;) {
        dma_addr_t dma_addr;

        if ((rx_processed++ > budget) || (rx_ring->rxd_info2.DDONE_bit == 0)) {
            break;
        }

        /* prefetch the next handling RXD */
        if (rx_ring_no == 0) {
            rx_dma_owner_lro[rx_ring_no] = (rx_dma_owner_idx + 1) % NUM_RX_DESC;
            skb_size = SKB_DATA_ALIGN(MAX_RX_LENGTH + NET_IP_ALIGN + NET_SKB_PAD) + SKB_DATA_ALIGN(sizeof(struct skb_shared_info));
            map_size = MAX_RX_LENGTH;
        }
        else {
            rx_dma_owner_lro[rx_ring_no] = (rx_dma_owner_idx + 1) % NUM_LRO_RX_DESC;
            skb_size = SKB_DATA_ALIGN(MAX_LRO_RX_LENGTH + NET_IP_ALIGN + NET_SKB_PAD) + SKB_DATA_ALIGN(sizeof(struct skb_shared_info));
            map_size = MAX_LRO_RX_LENGTH;
        }

        rx_ring_no_next =  _MDrv_NOE_LRO_Get_Rx_Ring(ei_local, rx_dma_owner_lro);
        rx_dma_owner_idx_next = rx_dma_owner_lro[rx_ring_no_next];
        rx_ring_next = &ei_local->rx_ring[rx_ring_no_next][rx_dma_owner_idx_next];
        rx_data_next = ei_local->netrx_skb_data[rx_ring_no_next][rx_dma_owner_idx_next];
        prefetch(rx_ring_next);

        /* We have to check the free memory size is big enough
         * before pass the packet to cpu
         */
        new_data = MDrv_NOE_SkbData_Alloc(skb_size, GFP_ATOMIC);

        if (unlikely(!new_data)) {
            NOE_MSG_ERR("skb not available...\n");
            goto skb_err;
        }

        //dma_addr = dma_map_single(dev->dev.parent, new_data + NET_SKB_PAD, map_size, DMA_FROM_DEVICE);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0))
        dma_set_attr(DMA_ATTR_SKIP_CPU_SYNC, &attrs);//edie.chen--4.9 not support
        dma_addr = dma_map_single_attrs(dev->dev.parent, new_data + NET_SKB_PAD, map_size, DMA_FROM_DEVICE, &attrs); //edie.chen--4.9 not support
#else
        attrs = DMA_ATTR_SKIP_CPU_SYNC;
        dma_addr = dma_map_single_attrs(dev->dev.parent, new_data + NET_SKB_PAD, map_size, DMA_FROM_DEVICE, attrs);
#endif
        
        if (unlikely(dma_mapping_error(ei_local->dev, dma_addr))) {
            NOE_MSG_ERR("[%s]dma_map_single() failed...\n", __func__);
            MDrv_NOE_SkbData_Free(new_data);
            goto skb_err;
        }

        rx_skb = MDrv_NOE_Skb_Build(rx_data, skb_size);
        if (unlikely(!rx_skb)) {
            NOE_MSG_ERR("[%s][%d]build_skb failed\n",__FUNCTION__,__LINE__);
            dma_unmap_single(dev->dev.parent, dma_addr, map_size, DMA_FROM_DEVICE);
            MDrv_NOE_SkbData_Free(new_data);
            NOE_MSG_ERR("call MDrv_NOE_SkbData_Free to free\n");
            goto skb_err;
        }

        skb_reserve(rx_skb, NET_SKB_PAD + NET_IP_ALIGN);

        length = (rx_ring->rxd_info2.PLEN1 << 14) | rx_ring->rxd_info2.PLEN0;

        dma_unmap_single(dev->dev.parent, rx_ring->rxd_info1.PDP0, length, DMA_FROM_DEVICE);

        prefetch(rx_skb->data);

        /* skb processing */
        skb_put(rx_skb, length);

        /* rx packet from GE2 */
        if (rx_ring->rxd_info4.SP == 2) {
            if (ei_local->pseudo_dev) {
                rx_skb->dev = ei_local->pseudo_dev;
                rx_skb->protocol = eth_type_trans(rx_skb, ei_local->pseudo_dev);
            }
            else {
                NOE_MSG_ERR("pseudo_dev is still not initialize ");
                NOE_MSG_ERR("but receive packet from GMAC2\n");
            }
        }
        else {
            rx_skb->dev = dev;
            rx_skb->protocol = eth_type_trans(rx_skb, dev);
        }

        /* rx checksum offload */
        if (likely(rx_ring->rxd_info4.L4VLD))
            rx_skb->ip_summed = CHECKSUM_UNNECESSARY;
        else
            rx_skb->ip_summed = CHECKSUM_NONE;

        if (ei_local->features & FE_HW_NAT) {
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
        }
        /* HW LRO aggregation statistics */
        if (ei_local->features & FE_HW_LRO_DBG) {
            MDrv_NOE_LRO_PROC_Update_Stats(rx_ring_no, rx_ring);
            MDrv_NOE_LRO_PROC_Update_Flush_Stats(rx_ring_no, rx_ring);
        }

/* IS_VALID_NOE_HWNAT_HOOK_RX return 1 --> continue
 * IS_VALID_NOE_HWNAT_HOOK_RX return 0 --> FWD & without netif_rx
 */
#ifdef CONFIG_NOE_NAT_HW
        if ((IS_NOT_VALID_NOE_HWNAT_HOOK_RX) || (IS_VALID_NOE_HWNAT_HOOK_RX && NOE_HWNAT_HOOK_RX(rx_skb))) {
#endif
            if (ei_local->features & FE_INT_NAPI) {
                /* napi_gro_receive(napi, rx_skb); */
                netif_receive_skb(rx_skb);
            }
            else {
                netif_rx(rx_skb);
            }
#ifdef CONFIG_NOE_NAT_HW
        }
#endif

        if (rx_ring->rxd_info4.SP == 2) {
            p_ad->stat.rx_packets++;
            p_ad->stat.rx_bytes += length;
        }
        else {
            ei_local->stat.rx_packets++;
            ei_local->stat.rx_bytes += length;
        }

        /* Init RX desc. */
        _MDrv_NOE_LRO_Init_Rx_Desc(ei_local, rx_ring, rx_ring_no, dma_addr);
        ei_local->netrx_skb_data[rx_ring_no][rx_dma_owner_idx] = new_data;
        /* make sure that all changes to the dma ring are flushed before
          * we continue
          */
        wmb();
        MHal_NOE_LRO_Update_Calc_Idx(rx_ring_no, rx_dma_owner_idx);
#ifdef CONFIG_NOE_RW_PDMAPTR_FROM_VAR
        ei_local->rx_calc_idx[rx_ring_no] = rx_dma_owner_idx;
#endif
        /* use prefetched variable */
        rx_dma_owner_idx = rx_dma_owner_idx_next;
        rx_ring_no = rx_ring_no_next;
        rx_ring = rx_ring_next;
        rx_data = rx_data_next;
    }   /* for */

    return rx_processed;

skb_err:
    /* rx packet from GE2 */
    if (rx_ring->rxd_info4.SP == 2)
        p_ad->stat.rx_dropped++;
    else
        ei_local->stat.rx_dropped++;

    /* Discard the rx packet */
    _MDrv_NOE_LRO_Init_Rx_Desc(ei_local, rx_ring, rx_ring_no, rx_ring->rxd_info1.PDP0);
    MHal_NOE_LRO_Update_Calc_Idx(rx_ring_no, rx_dma_owner_idx);
#ifdef CONFIG_NOE_RW_PDMAPTR_FROM_VAR
    ei_local->rx_calc_idx[rx_ring_no] = rx_dma_owner_idx;
#endif
    return (budget + 1);
}


