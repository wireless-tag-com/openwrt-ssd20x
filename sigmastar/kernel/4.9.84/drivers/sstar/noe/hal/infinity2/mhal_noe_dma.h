/*
* mhal_noe_dma.h- Sigmastar
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
/// @file   MHAL_NOE_DMA.h
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MHAL_NOE_DMA_H_
#define _MHAL_NOE_DMA_H_



#if defined(CONFIG_QDMA_MQ)
#define GMAC1_TXQ_NUM 3
#define GMAC1_TXQ_TXD_NUM 512
#define GMAC1_TXD_NUM (GMAC1_TXQ_NUM * GMAC1_TXQ_TXD_NUM)
#define GMAC2_TXQ_NUM 1
#define GMAC2_TXQ_TXD_NUM 128
#define GMAC2_TXD_NUM (GMAC2_TXQ_NUM * GMAC2_TXQ_TXD_NUM)
#define NUM_TX_DESC (GMAC1_TXD_NUM + GMAC2_TXD_NUM)
#define TOTAL_TXQ_NUM (GMAC1_TXQ_NUM + GMAC2_TXQ_NUM)
#else
#define GMAC1_TXQ_NUM 1
#define GMAC1_TXQ_TXD_NUM  2048
#define GMAC1_TXD_NUM (GMAC1_TXQ_NUM * GMAC1_TXQ_TXD_NUM)
#define GMAC2_TXQ_NUM 1
#define GMAC2_TXQ_TXD_NUM 1024
#define GMAC2_TXD_NUM (GMAC2_TXQ_NUM * GMAC2_TXQ_TXD_NUM)
#define NUM_TX_DESC (GMAC1_TXD_NUM + GMAC2_TXD_NUM)
#define TOTAL_TXQ_NUM (GMAC1_TXQ_NUM + GMAC2_TXQ_NUM)
#endif

#define NUM_RX_DESC     2048
#define NUM_QRX_DESC 16
#define NUM_PQ 64
#define NUM_PQ_RESV 4
#define FFA 512
#define QUEUE_OFFSET 0x10
/* #define NUM_TX_DESC (NUM_PQ * NUM_PQ_RESV + FFA) */

#define NUM_TX_MAX_PROCESS NUM_TX_DESC
#define NUM_RX_MAX_PROCESS 16

#define MAX_RX_RING_NUM 4
#define NUM_LRO_RX_DESC 16

#define MAX_RX_LENGTH   1536

#define NUM_QDMA_PAGE       512
#define QDMA_PAGE_SIZE      2048

#define MAX_PACKET_SIZE 1514
#define MIN_PACKET_SIZE 60

#define MAX_PTXD_LEN 0x3fff /* 16k */
#define MAX_QTXD_LEN 0xffff



/*=========================================
 *    SFQ Table Format define
 *=========================================
 */
struct SFQ_INFO1_T {
    unsigned int VQHPTR;
};

struct SFQ_INFO2_T {
    unsigned int VQTPTR;
};

struct SFQ_INFO3_T {
    unsigned int QUE_DEPTH:16;
    unsigned int DEFICIT_CNT:16;
};

struct SFQ_INFO4_T {
    unsigned int RESV;
};

struct SFQ_INFO5_T {
    unsigned int PKT_CNT;
};

struct SFQ_INFO6_T {
    unsigned int BYTE_CNT;
};

struct SFQ_INFO7_T {
    unsigned int BYTE_CNT;
};

struct SFQ_INFO8_T {
    unsigned int RESV;
};

struct SFQ_table {
    struct SFQ_INFO1_T sfq_info1;
    struct SFQ_INFO2_T sfq_info2;
    struct SFQ_INFO3_T sfq_info3;
    struct SFQ_INFO4_T sfq_info4;
    struct SFQ_INFO5_T sfq_info5;
    struct SFQ_INFO6_T sfq_info6;
    struct SFQ_INFO7_T sfq_info7;
    struct SFQ_INFO8_T sfq_info8;
};


/*=========================================
 *    PDMA RX Descriptor Format define
 *=========================================
 */

struct PDMA_RXD_INFO1_T {
    unsigned int PDP0;
};

struct PDMA_RXD_INFO2_T {
    unsigned int PLEN1:2;
    unsigned int LRO_AGG_CNT:8;
    unsigned int REV:5;
    unsigned int TAG:1;
    unsigned int PLEN0:14;
    unsigned int LS0:1;
    unsigned int DDONE_bit:1;
};

struct PDMA_RXD_INFO3_T {
    unsigned int VID:16;
    unsigned int TPID:16;
};

struct PDMA_RXD_INFO4_T {
    unsigned int FOE_ENTRY:14;
    unsigned int CRSN:5;
    unsigned int SP:4;
    unsigned int L4F:1;
    unsigned int L4VLD:1;
    unsigned int TACK:1;
    unsigned int IP4F:1;
    unsigned int IP4:1;
    unsigned int IP6:1;
    unsigned int UN_USE1:3;
};

struct PDMA_rxdesc {
    struct PDMA_RXD_INFO1_T rxd_info1;
    struct PDMA_RXD_INFO2_T rxd_info2;
    struct PDMA_RXD_INFO3_T rxd_info3;
    struct PDMA_RXD_INFO4_T rxd_info4;
#ifdef CONFIG_32B_DESC
    unsigned int rxd_info5;
    unsigned int rxd_info6;
    unsigned int rxd_info7;
    unsigned int rxd_info8;
#endif
};

/*=========================================
 *    PDMA TX Descriptor Format define
 *=========================================
 */
struct PDMA_TXD_INFO1_T {
    unsigned int SDP0;
};

struct PDMA_TXD_INFO2_T {
    unsigned int SDL1:14;
    unsigned int LS1_bit:1;
    unsigned int BURST_bit:1;
    unsigned int SDL0:14;
    unsigned int LS0_bit:1;
    unsigned int DDONE_bit:1;
};

struct PDMA_TXD_INFO3_T {
    unsigned int SDP1;
};

struct PDMA_TXD_INFO4_T {
    unsigned int VLAN_TAG:17;   /* INSV(1)+VPRI(3)+CFI(1)+VID(12) */
    unsigned int RESV:2;
    unsigned int UDF:6;
    unsigned int FPORT:3;
    unsigned int TSO:1;
    unsigned int TUI_CO:3;
};

struct PDMA_txdesc {
    struct PDMA_TXD_INFO1_T txd_info1;
    struct PDMA_TXD_INFO2_T txd_info2;
    struct PDMA_TXD_INFO3_T txd_info3;
    struct PDMA_TXD_INFO4_T txd_info4;
#ifdef CONFIG_32B_DESC
    unsigned int txd_info5;
    unsigned int txd_info6;
    unsigned int txd_info7;
    unsigned int txd_info8;
#endif
};

/*=========================================
 *    QDMA TX Descriptor Format define
 *=========================================
 */
struct QDMA_TXD_INFO1_T {
    unsigned int SDP;
};

struct QDMA_TXD_INFO2_T {
    unsigned int NDP;
};

struct QDMA_TXD_INFO3_T {
    unsigned int QID:4; /* Q0~Q15 */
    /* unsigned int    VQID                  : 10; */
    unsigned int PROT:3;
    unsigned int IPOFST:7;
    unsigned int SWC_bit:1;
    unsigned int BURST_bit:1;
    unsigned int SDL:14;
    unsigned int LS_bit:1;
    unsigned int OWN_bit:1;
};

struct QDMA_TXD_INFO4_T {
    unsigned int VLAN_TAG:17;   /* INSV(1)+VPRI(3)+CFI(1)+VID(12) */
    unsigned int VQID0:1;
    unsigned int SDL:2;
    unsigned int QID:2; /* Q16~Q63 */
    unsigned int RESV:3;
    unsigned int FPORT:3;
    unsigned int TSO:1;
    unsigned int TUI_CO:3;
};

struct QDMA_txdesc {
    struct QDMA_TXD_INFO1_T txd_info1;
    struct QDMA_TXD_INFO2_T txd_info2;
    struct QDMA_TXD_INFO3_T txd_info3;
    struct QDMA_TXD_INFO4_T txd_info4;
#ifdef CONFIG_32B_DESC
    unsigned int txd_info5;
    unsigned int txd_info6;
    unsigned int txd_info7;
    unsigned int txd_info8;
#endif
};

#define QTXD_LEN (sizeof(struct QDMA_txdesc))


#endif /* _MHAL_NOE_DMA_H_ */


