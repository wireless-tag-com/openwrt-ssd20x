/*
* mdrv_gmac_v3.h- Sigmastar
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
/// @file   GMAC.h
/// @brief  GMAC Driver Interface
///////////////////////////////////////////////////////////////////////////////////////////////////


// -----------------------------------------------------------------------------
// Linux GMAC.h define start
// -----------------------------------------------------------------------------

#ifndef __DRV_GMAC_H_
#define __DRV_GMAC_H_

#define GMAC_DBG(fmt, args...)              {printk("Mstar_gmac: "); printk(fmt, ##args);}
#define GMAC_INFO                           {printk("Line:%u\n", __LINE__);}
#define GMAC_DRVNAME                        "mstar gmac"
#define GMAC_DRV_VERSION                    "3.0.0"
#define GMAC_TEST_STRING_LEN                0
#define GMAC_PRIV_FLAGS_STRING_LEN          0

//-------------------------------------------------------------------------------------------------
//  Define Enable or Compiler Switches
//-------------------------------------------------------------------------------------------------
#define GMAC_MTU                            (1518)
//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define GMAC_EP_FLAG_OPEND                  0X00000001UL
#define GMAC_EP_FLAG_SUSPENDING             0X00000002UL
#define GMAC_EP_FLAG_SUSPENDING_OPEND       0X00000004UL

#define GMAC_ETHERNET_TEST_NO_LINK          0x00000000UL
#define GMAC_ETHERNET_TEST_AUTO_NEGOTIATION 0x00000001UL
#define GMAC_ETHERNET_TEST_LINK_SUCCESS     0x00000002UL
#define GMAC_ETHERNET_TEST_RESET_STATE      0x00000003UL
#define GMAC_ETHERNET_TEST_SPEED_100M       0x00000004UL
#define GMAC_ETHERNET_TEST_DUPLEX_FULL      0x00000008UL
#define GMAC_ETHERNET_TEST_INIT_FAIL        0x00000010UL

#define GMAC_RX_TMR                         (0)
#define GMAC_LINK_TMR                       (1)

#define GMAC_CHECK_LINK_TIME                (HZ)
#define GMAC_CHECK_CNT                      (500000)
#define GMAC_RTL_8210                       (0x1CUL)
//--------------------------------------------------------------------------------------------------
//  Global variable
//--------------------------------------------------------------------------------------------------
u8 GMAC_MY_MAC[6] = { 0x00, 0x55, 0x66, 0x00, 0x00, 0x01 };
//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
#ifdef TX_DESC_MODE
struct tx_descriptor
{
  u32   addr;
  u32   low_tag;
  u32   reserve0;
  u32   reserve1;
};
#endif

#ifdef TX_SOFTWARE_QUEUE
struct tx_ring
{
    u8 used;
    struct sk_buff *skb;        /* holds skb until xmit interrupt completes */
    dma_addr_t skb_physaddr;    /* phys addr from pci_map_single */
};
#endif

#ifdef RX_DESC_MODE
struct rx_descriptor
{
  u32   addr;
  u32   low_tag;
  u32   high_tag;
  u32   reserve;
};
#endif

struct _BasicConfigGMAC
{
    u8 connected;           // 0:No, 1:Yes    <== (20070515) Wait for Julian's reply
    u8 speed;               // 10:10Mbps, 100:100Mbps
    // ETH_CTL Register:
    u8 wes;                 // 0:Disable, 1:Enable (WR_ENABLE_STATISTICS_REGS)
    // ETH_CFG Register:
    u8 duplex;              // 1:Half-duplex, 2:Full-duplex
    u8 cam;                 // 0:No CAM, 1:Yes
    u8 rcv_bcast;           // 0:No, 1:Yes
    u8 rlf;                 // 0:No, 1:Yes receive long frame(1522)
    // MAC Address:
    u8 sa1[6];              // Specific Addr 1 (MAC Address)
    u8 sa2[6];              // Specific Addr 2
    u8 sa3[6];              // Specific Addr 3
    u8 sa4[6];              // Specific Addr 4
};
typedef struct _BasicConfigGMAC BasicConfigGMAC;

struct _UtilityVarsGMAC
{
    u32 cntChkINTCounter;
    u32 readIdxRBQP;        // Reset = 0x00000000
    u32 rxOneFrameAddr;     // Reset = 0x00000000 (Store the Addr of "ReadONE_RX_Frame")
    // Statistics Counters : (accumulated)
    u32 cntREG_ETH_FRA;
    u32 cntREG_ETH_SCOL;
    u32 cntREG_ETH_MCOL;
    u32 cntREG_ETH_OK;
    u32 cntREG_ETH_SEQE;
    u32 cntREG_ETH_ALE;
    u32 cntREG_ETH_DTE;
    u32 cntREG_ETH_LCOL;
    u32 cntREG_ETH_ECOL;
    u32 cntREG_ETH_TUE;
    u32 cntREG_ETH_CSE;
    u32 cntREG_ETH_RE;
    u32 cntREG_ETH_ROVR;
    u32 cntREG_ETH_SE;
    u32 cntREG_ETH_ELR;
    u32 cntREG_ETH_RJB;
    u32 cntREG_ETH_USF;
    u32 cntREG_ETH_SQEE;
    // Interrupt Counter :
    u32 cntHRESP;           // Reset = 0x0000
    u32 cntROVR;            // Reset = 0x0000
    u32 cntLINK;            // Reset = 0x0000
    u32 cntTIDLE;           // Reset = 0x0000
    u32 cntTCOM;            // Reset = 0x0000
    u32 cntTBRE;            // Reset = 0x0000
    u32 cntRTRY;            // Reset = 0x0000
    u32 cntTUND;            // Reset = 0x0000
    u32 cntTOVR;            // Reset = 0x0000
    u32 cntRBNA;            // Reset = 0x0000
    u32 cntRCOM;            // Reset = 0x0000
    u32 cntDONE;            // Reset = 0x0000
    // Flags:
    u8  flagMacTxPermit;    // 0:No,1:Permitted.  Initialize as "permitted"
    u8  flagISR_INT_RCOM;
    u8  flagISR_INT_RBNA;
    u8  flagISR_INT_DONE;
    u8  flagPowerOn;        // 0:Poweroff, 1:Poweron
    u8  initedGMAC;         // 0:Not initialized, 1:Initialized.
    u8  flagRBNA;
    // Misc Counter:
    u32 cntRxFrames;        // Reset = 0x00000000 (Counter of RX frames,no matter it's me or not)
    u32 cntReadONE_RX;      // Counter for ReadONE_RX_Frame
    u32 cntCase20070806;
    u32 cntChkToTransmit;
    // Misc Variables:
    u32 mainThreadTasks;    // (20071029_CHARLES) b0=Poweroff,b1=Poweron
};
typedef struct _UtilityVarsGMAC UtilityVarsGMAC;

struct GMAC_private
{
    struct net_device *dev;
    struct net_device_stats stats;
    struct mii_if_info mii;             /* ethtool support */
    struct timer_list Link_timer;
    u32 padmux_type;
    u32 hardware_type;
    u32 initstate;
    u32 msglvl;
    BasicConfigGMAC ThisBCE;
    UtilityVarsGMAC ThisUVE;
    /* Memory */
    phys_addr_t     RAM_VA_BASE;
    phys_addr_t     RAM_PA_BASE;
    phys_addr_t     RAM_VA_PA_OFFSET;
    phys_addr_t     RX_DESC_BASE;
#ifndef RX_ZERO_COPY
    phys_addr_t     RX_BUFFER_BASE;
#endif
#ifdef TX_DESC_MODE
    phys_addr_t     TX_LP_DESC_BASE;
    phys_addr_t     TX_HP_DESC_BASE;
#endif
    phys_addr_t     TX_BUFFER_BASE;
    /* PHY */
    u8  phyaddr;
    u32 phy_status_register;
    u32 phy_type;             /* type of PHY (PHY_ID) */
    spinlock_t irq_lock;                /* lock for MDI interface */
    spinlock_t tx_lock;                 /* lock for MDI interface */
    short phy_media;                    /* media interface type */
    /* Transmit */
    u32 tx_index;
    u32 tx_ring_entry_number;
#ifdef TX_DESC_MODE
    struct tx_descriptor *tx_desc_list;
    struct sk_buff *tx_desc_sk_buff_list[TX_LOW_PRI_DESC_NUMBER];
    u32 tx_desc_write_index;
    u32 tx_desc_read_index;
    u32 tx_desc_queued_number;
    u32 tx_desc_count;
    u32 tx_desc_full_count;
#endif
#ifdef TX_SOFTWARE_QUEUE
    struct tx_ring tx_swq[TX_SW_QUEUE_SIZE];
    unsigned int tx_rdidx;              /* TX_SW_QUEUE read to hw index */
    unsigned int tx_wridx;              /* TX_SW_QUEUE write index */
    unsigned int tx_clidx;              /* TX_SW_QUEUE clear index */

    unsigned int tx_rdwrp;              /* TX_SW_QUEUE read to hw index wrap*/
    unsigned int tx_wrwrp;              /* TX_SW_QUEUE write index wrap*/
    unsigned int tx_clwrp;              /* TX_SW_QUEUE clear index wrap */
    unsigned int tx_swq_full_cnt;       /* TX_SW_QUEUE full stopped count*/

    unsigned int irqcnt;
    unsigned int tx_irqcnt;
#endif
#ifdef TX_NAPI
    struct napi_struct napi_tx;
#endif
    /* Receive */
    u32 ROVR_count;
    u32 full_budge_count;
    u32 polling_count;
    u32 max_polling;
    u32 rx_ring_entry_number;
#ifdef RX_DESC_MODE
    struct rx_descriptor *rx_desc_list;
    u32 rx_desc_read_index;
#ifdef RX_ZERO_COPY
    struct sk_buff *rx_desc_sk_buff_list[RX_DESC_NUMBER];
#endif
#endif
#ifdef RX_NAPI
    struct napi_struct napi_rx;
#endif
    /* suspend/resume */
    u32 ep_flag;
};

#endif
// -----------------------------------------------------------------------------
// Linux GMAC.h End
// -----------------------------------------------------------------------------
