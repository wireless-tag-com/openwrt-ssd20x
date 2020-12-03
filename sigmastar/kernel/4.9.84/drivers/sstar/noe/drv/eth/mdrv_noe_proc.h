/*
* mdrv_noe_proc.h- Sigmastar
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
/// @file   MDRV_NOE_PROC.h
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_NOE_PROC_H_
#define _MDRV_NOE_PROC_H_


#define NOE_PROC_LRO_STATS          "lro_stats"
#define NOE_PROC_TSO_LEN            "tso_len"
#define NOE_PROC_DIR                "noe"
#define NOE_PROC_SKBFREE            "skb_free"
#define NOE_PROC_TX_RING            "tx_ring"
#define NOE_PROC_RX_RING            "rx_ring"
#define NOE_PROC_LRO_RX_RING1       "lro_rx_ring1"
#define NOE_PROC_LRO_RX_RING2       "lro_rx_ring2"
#define NOE_PROC_LRO_RX_RING3       "lro_rx_ring3"
#define NOE_PROC_NUM_OF_TXD         "num_of_txd"
#define NOE_PROC_TSO_LEN            "tso_len"
#define NOE_PROC_LRO_STATS          "lro_stats"
#define NOE_PROC_HW_LRO_STATS       "hw_lro_stats"
#define NOE_PROC_HW_LRO_AUTO_TLB    "hw_lro_auto_tlb"
#define NOE_PROC_HW_IO_COHERENT     "hw_iocoherent"
#define NOE_PROC_GMAC               "gmac"
#define NOE_PROC_GMAC2              "gmac2"
#define NOE_PROC_CP0                "cp0"
#define NOE_PROC_QSCH               "qsch"
#define NOE_PROC_READ_VAL           "regread_value"
#define NOE_PROC_WRITE_VAL          "regwrite_value"
#define NOE_PROC_ADDR               "reg_addr"
#define NOE_PROC_CTL                "procreg_control"
#define NOE_PROC_RXDONE_INTR        "rxdone_intr_count"
#define NOE_PROC_ESW_INTR           "esw_intr_count"
#define NOE_PROC_ESW_CNT            "esw_cnt"
#define NOE_PROC_ETH_CNT            "eth_cnt"
#define NOE_PROC_SNMP               "snmp"
#define NOE_PROC_SET_LAN_IP         "set_lan_ip"
#define NOE_PROC_SCHE               "schedule" /* TASKLET_WORKQUEUE_SW */
#define NOE_PROC_QDMA               "qdma"
#define NOE_PROC_INT_DBG            "int_dbg"
#define NOE_PROC_PIN_MUX            "mux"
#define NOE_PROC_LOG_CTRL           "log"

struct mdrv_noe_proc_intr {
    unsigned int RX_COHERENT_CNT;
    unsigned int RX_DLY_INT_CNT;
    unsigned int TX_COHERENT_CNT;
    unsigned int TX_DLY_INT_CNT;
    unsigned int RING3_RX_DLY_INT_CNT;
    unsigned int RING2_RX_DLY_INT_CNT;
    unsigned int RING1_RX_DLY_INT_CNT;
    unsigned int RXD_ERROR_CNT;
    unsigned int ALT_RPLC_INT3_CNT;
    unsigned int ALT_RPLC_INT2_CNT;
    unsigned int ALT_RPLC_INT1_CNT;
    unsigned int RX_DONE_INT3_CNT;
    unsigned int RX_DONE_INT2_CNT;
    unsigned int RX_DONE_INT1_CNT;
    unsigned int RX_DONE_INT0_CNT;
    unsigned int TX_DONE_INT3_CNT;
    unsigned int TX_DONE_INT2_CNT;
    unsigned int TX_DONE_INT1_CNT;
    unsigned int TX_DONE_INT0_CNT;
};



struct PDMA_LRO_AUTO_TLB_INFO0_T {
    unsigned int DTP:16;
    unsigned int STP:16;
};

struct PDMA_LRO_AUTO_TLB_INFO1_T {
    unsigned int SIP0:32;
};

struct PDMA_LRO_AUTO_TLB_INFO2_T {
    unsigned int SIP1:32;
};

struct PDMA_LRO_AUTO_TLB_INFO3_T {
    unsigned int SIP2:32;
};

struct PDMA_LRO_AUTO_TLB_INFO4_T {
    unsigned int SIP3:32;
};

struct PDMA_LRO_AUTO_TLB_INFO5_T {
    unsigned int VLAN_VID0:32;
};

struct PDMA_LRO_AUTO_TLB_INFO6_T {
    unsigned int VLAN_VID1:16;
    unsigned int VLAN_VID_VLD:4;
    unsigned int CNT:12;
};

struct PDMA_LRO_AUTO_TLB_INFO7_T {
    unsigned int DW_LEN:32;
};

struct PDMA_LRO_AUTO_TLB_INFO8_T {
    unsigned int DIP_ID:2;
    unsigned int IPV6:1;
    unsigned int IPV4:1;
    unsigned int RESV:27;
    unsigned int VALID:1;
};

struct PDMA_LRO_AUTO_TLB_INFO {
    struct PDMA_LRO_AUTO_TLB_INFO0_T auto_tlb_info0;
    struct PDMA_LRO_AUTO_TLB_INFO1_T auto_tlb_info1;
    struct PDMA_LRO_AUTO_TLB_INFO2_T auto_tlb_info2;
    struct PDMA_LRO_AUTO_TLB_INFO3_T auto_tlb_info3;
    struct PDMA_LRO_AUTO_TLB_INFO4_T auto_tlb_info4;
    struct PDMA_LRO_AUTO_TLB_INFO5_T auto_tlb_info5;
    struct PDMA_LRO_AUTO_TLB_INFO6_T auto_tlb_info6;
    struct PDMA_LRO_AUTO_TLB_INFO7_T auto_tlb_info7;
    struct PDMA_LRO_AUTO_TLB_INFO8_T auto_tlb_info8;
};



int MDrv_NOE_Update_Tso_Len(int tso_len);
int MDrv_NOE_PROC_Init(struct net_device *dev);
void MDrv_NOE_PROC_Exit(void);
#if FE_HW_LRO
int MDrv_NOE_LRO_PROC_Init(struct proc_dir_entry *proc_reg_dir, struct net_device *dev);
void MDrv_NOE_LRO_PROC_Exit(struct proc_dir_entry *proc_reg_dir);
void MDrv_NOE_LRO_PROC_Update_Flush_Stats(unsigned int ring_num, struct PDMA_rxdesc *rx_ring);
void MDrv_NOE_LRO_PROC_Update_Stats(unsigned int ring_num, struct PDMA_rxdesc *rx_ring);
#else
static inline int MDrv_NOE_LRO_PROC_Init(struct proc_dir_entry *proc_reg_dir, struct net_device *dev) { return 0;}
static inline void MDrv_NOE_LRO_PROC_Exit(struct proc_dir_entry *proc_reg_dir) {}
static inline void MDrv_NOE_LRO_PROC_Update_Flush_Stats(unsigned int ring_num, struct PDMA_rxdesc *rx_ring) {}
static inline void MDrv_NOE_LRO_PROC_Update_Stats(unsigned int ring_num, struct PDMA_rxdesc *rx_ring) {}
#endif /* FE_HW_LRO */

#endif /* _MDRV_NOE_PROC_H_ */
