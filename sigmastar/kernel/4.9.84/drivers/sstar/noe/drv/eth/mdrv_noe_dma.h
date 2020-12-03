/*
* mdrv_noe_dma.h- Sigmastar
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
/// @file   MDRV_NOE_DMA.h
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////



#ifndef _MDRV_NOE_DMA_H_
#define _MDRV_NOE_DMA_H_




struct parse_result {
    /* layer2 header */
    u8 dmac[6];
    u8 smac[6];

    /* vlan header */
    u16 vlan_tag;
    u16 vlan1_gap;
    u16 vlan1;
    u16 vlan2_gap;
    u16 vlan2;
    u16 vlan_layer;

    /* pppoe header */
    u32 pppoe_gap;
    u16 ppp_tag;
    u16 pppoe_sid;

    /* layer3 header */
    u16 eth_type;
    struct iphdr iph;
    struct ipv6hdr ip6h;

    /* layer4 header */
    struct tcphdr th;
    struct udphdr uh;

    u32 pkt_type;
    u8 is_mcast;
};

int MDrv_NOE_PDMA_Init_Rx(struct net_device *dev);
int MDrv_NOE_PDMA_Init_Tx(struct net_device *dev);
void MDrv_NOE_PDMA_Deinit_Rx(struct net_device *dev);
void MDrv_NOE_PDMA_Deinit_Tx(struct net_device *dev);
int MDrv_NOE_PDMA_Start_Xmit(struct sk_buff *skb, struct net_device *dev, int gmac_no);
int MDrv_NOE_PDMA_Xmit_Housekeeping(struct net_device *netdev, int budget);

int MDrv_NOE_QDMA_Init_Rx(struct net_device *dev);
int MDrv_NOE_QDMA_Init_Tx(struct net_device *dev);
void MDrv_NOE_QDMA_Deinit_Rx(struct net_device *dev);
void MDrv_NOE_QDMA_Deinit_Tx(struct net_device *dev);
int MDrv_NOE_QDMA_Start_Xmit(struct sk_buff *skb, struct net_device *dev, int gmac_no);
int MDrv_NOE_QDMA_Xmit_Housekeeping(struct net_device *netdev, int budget);
int MDrv_NOE_QDMA_Ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);


#endif /* _MDRV_NOE_DMA_H_ */
