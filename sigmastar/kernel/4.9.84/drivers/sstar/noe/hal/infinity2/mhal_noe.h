/*
* mhal_noe.h- Sigmastar
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
// (“MStar Confidential Information”) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   Mhal_noe.h
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MHAL_NOE_H
#define MHAL_NOE_H

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include "mhal_porting.h"

#include "mhal_noe_dma.h"
//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define NOE_PHYS         0x1fc00000
#define NOE_SIZE         0x00200000



#define NOE_FALSE (0)
#define NOE_TRUE (1)

#define NOE_DISABLE (0)
#define NOE_ENABLE (1)

#define NOE_INVALID_PHY_ADDR (0xFFFFFFFF)
#define NOE_INVALID_CALC_IDX (0xFFFFFFFF)
#define NOE_SFQ_MAX_NUM (8)


/*
#if defined(CONFIG_MIPS)
    #define MHAL_NOE_MAP_VA_TO_PA(dev, va, size, dir)   ((MS_U32)virt_to_phys((MS_U32) (va)) - MSTAR_MIU0_BUS_BASE)
#elif defined(CONFIG_ARM) || defined(CONFIG_ARM64)
    #define MHAL_NOE_MAP_VA_TO_PA(dev, va, size, dir)  \
        (((u32)__virt_to_phys(va)) > MSTAR_MIU1_BUS_BASE)? ((va) - MSTAR_MIU1_BUS_BASE): ((va) - MSTAR_MIU0_BUS_BASE)
#else
    #error "ARCH is not supported"
#endif
*/

#define MHAL_NOE_IS_TXD_NOT_AVAILABLE(cpu_ptr) \
    ((cpu_ptr->txd_info2.NDP & 0x1) ||(cpu_ptr->txd_info3.OWN_bit == 0))


#define MHAL_NOE_MAX_ENTRIES_IN_LRO_TABLE (8)
#define MHAL_NOE_MAX_INFO_FOR_EACH_ENTRY_IN_LRO_TABLE (9)


//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
typedef enum {
    E_NOE_HAL_LOG_NONE = 0,
    E_NOE_HAL_LOG_DBG = BIT(0),
    E_NOE_HAL_LOG_INTR = BIT(1),
}EN_NOE_HAL_LOG;

typedef enum {
    E_NOE_SEL_DISABLE = 0,
    E_NOE_SEL_ENABLE = 1,
}EN_NOE_SEL;

typedef enum {
    E_NOE_RET_TRUE = 0,
    E_NOE_RET_FALSE = 1,
    E_NOE_RET_INVALID_PARAM, /* the input param is not valid */
    E_NOE_RET_DEVICE_BUSY, /* dev is busy */
    E_NOE_RET_TIMEOUT, /* operation is timeout */
}EN_NOE_RET;

typedef enum {
    E_NOE_SEL_PIN_MUX_INVALID = 0,
    E_NOE_SEL_PIN_MUX_GE2_TO_CHIPTOP,
    E_NOE_SEL_PIN_MUX_GE1_TO_CHIPTOP,
    E_NOE_SEL_PIN_MUX_GE1_TO_CHIPTOP_GE2_TO_CHIPTOP,
    E_NOE_SEL_PIN_MUX_MAX,
}EN_NOE_SEL_PIN_MUX;

typedef enum {
    E_NOE_INTR_CLR_EXCEPT_RX = 0,
    E_NOE_INTR_CLR_EXCEPT_TX = 1,
}EN_NOE_INTR_CLR_STATUS;

typedef enum {
    E_NOE_INTR_INFO_RLS_DLY = 0,
    E_NOE_INTR_INFO_RLS_DONE = 1,
}EN_NOE_INTR_INFO;


typedef enum {
    E_NOE_DLY_DISABLE = 0,
    E_NOE_DLY_ENABLE = 1,
}EN_NOE_DELAY;


typedef enum {
    E_NOE_GE_MAC1 = 0,
    E_NOE_GE_MAC2 = 1,
    E_NOE_GE_MAC_MAX,
}EN_NOE_GE_MAC;

typedef enum {
    E_NOE_DIR_RX = BIT(0),
    E_NOE_DIR_TX = BIT(1),
    E_NOE_DIR_BOTH = (E_NOE_DIR_RX | E_NOE_DIR_TX),
}EN_NOE_DIR;

typedef enum {
    E_NOE_DMA_PACKET = BIT(0),
    E_NOE_DMA_QUEUE = BIT(1),
    E_NOE_DMA_QUEUE_WITH_SFQ = (E_NOE_DMA_QUEUE | BIT(2)),
}EN_NOE_DMA;

typedef enum {
    E_NOE_RING_NO0 = 0,
    E_NOE_RING_NO1 = 1,
    E_NOE_RING_NO2 = 2,
    E_NOE_RING_NO3 = 3,
    E_NOE_RING_MAX,
}EN_NOE_RING;

typedef enum {
    E_NOE_VQ_NO0 = 0,
    E_NOE_VQ_NO1 = 1,
    E_NOE_VQ_NO2 = 2,
    E_NOE_VQ_NO3 = 3,
    E_NOE_VQ_MAX,
}EN_NOE_VQ;

typedef enum {
    E_NOE_QDMA_INFO_CNT = 0,
    E_NOE_QDMA_INFO_FQ,
    E_NOE_QDMA_INFO_SCH,
    E_NOE_QDMA_INFO_FC,
    E_NOE_QDMA_INFO_FSM,
    E_NOE_QDMA_INFO_VQ,
    E_NOE_QDMA_INFO_PQ,
    E_NOE_QDMA_INFO_MAX,
}EN_NOE_QDMA_INFO_TYPE;

typedef enum {
    E_NOE_PDMA_INFO_DBG = 0,
    E_NOE_PDMA_INFO_MAX,
}EN_NOE_PDMA_INFO_TYPE;

typedef enum {
    E_NOE_QDMA_TX_FORWARD = 0,
    E_NOE_QMDA_TX_RELEASE,
}EN_NOE_QDMA_TX_TYPE;

typedef enum {
    E_NOE_LRO_CTRL_AGG_CNT = 0,
    E_NOE_LRO_CTRL_AGG_TIME,
    E_NOE_LRO_CTRL_AGE_TIME,
    E_NOE_LRO_CTRL_BW_THRESHOLD,
    E_NOE_LRO_CTRL_SWITCH, /* enable /disable */
}EN_NOE_LRO_CTRL_TYPE;


typedef enum {
    E_NOE_SPEED_1000,
    E_NOE_SPEED_100,
    E_NOE_SPEED_10,
    E_NOE_SPEED_INVALID
}EN_NOE_SPEED;

typedef enum {
    E_NOE_DUPLEX_FULL,
    E_NOE_DUPLEX_HALF,
    E_NOE_DUPLEX_INVALID
}EN_NOE_DUPLEX;


typedef enum {
    E_NOE_GDM_FWD_CPU = 0,
    E_NOE_GDM_FWD_ENG
}EN_NOE_GDM_FWD;

typedef enum {
    E_NOE_IRQ_0 = 0,
    E_NOE_IRQ_1 = 1,
    E_NOE_IRQ_2 = 2,
    E_NOE_IRQ_MAX
}EN_NOE_IRQ;

typedef enum {
    E_NOE_HAL_STATUS_IDLE = 0,
    E_NOE_HAL_STATUS_RUNNING,
    E_NOE_HAL_STATUS_SUSPEND,
}EN_NOE_HAL_STATUS;

struct noe_dma_adr {
    MS_PHYADDR ctx_adr;
    MS_PHYADDR dtx_adr;
    MS_PHYADDR crx_adr;
    MS_PHYADDR drx_adr;
};

struct noe_ring_info {
    MS_PHYADDR base_adr;
    MS_U32 max_cnt;
    MS_U32 cpu_idx;
    MS_U32 dma_idx;
};


struct noe_dma_info {
    union {
        struct noe_ring_info ring_st;
        struct noe_dma_adr adr_st;
    };
};


struct noe_intr_info {
    MS_U32 fe_intr_enable;
    MS_U32 fe_intr_status;
    MS_U32 fe_intr_mask;
    MS_U32 delay_intr_cfg;
    MS_U32 qfe_intr_enable;
    MS_U32 qfe_intr_status;
    MS_U32 qfe_intr_mask;

};

struct noe_rx_mac_info {
    MS_U32 good_cnt;
    MS_U32 good_pkt;
    MS_U32 overflow_err;
    MS_U32 fcs_err;
    MS_U32 ser_cnt;
    MS_U32 ler_pkt;
    MS_U32 chk_err;
    MS_U32 flow_ctrl;
};

struct noe_tx_mac_info {
    MS_U32 skip_cnt;
    MS_U32 collision_cnt;
    MS_U32 good_cnt;
    MS_U32 good_pkt;
};

struct noe_mac_status {
    MS_U32 control;
    MS_U32 status;
};

struct noe_mac_info {
    struct noe_rx_mac_info rx;
    struct noe_tx_mac_info tx;
    struct noe_mac_status stat;
};

struct noe_pse_info {
    MS_U32 min_free_cnt;
    MS_U32 free_cnt;
    MS_U32 fq_drop_cnt;
    MS_U32 fc_drop_cnt;
    MS_U32 ppe_drop_cnt;
};

struct noe_qdma_cnt {
    MS_U8 pq_no;
    MS_U32 pkt_cnt;
    MS_U32 drop_cnt;
};

struct noe_qdma_fq {
    MS_U32 sw_fq;
    MS_U32 hw_fq;
};


struct noe_sch_rate {
    MS_U32 max_rate;
    MS_U8 max_en;
};

struct noe_qdma_sch {
    struct noe_sch_rate sch[2];
};


struct noe_qdma_drop_thres {
    MS_U8 en;
    MS_U8 ffa;
    MS_U8 mode;
    MS_U8 fst_vq_en;
    MS_U8 fst_vq_mode;
};

struct noe_qdma_fc {
    struct noe_qdma_drop_thres sw;
    struct noe_qdma_drop_thres hw;
};

struct noe_qdma_fsm {
    MS_U8 vqtb;
    MS_U8 fq;
    MS_U8 tx;
    MS_U8 rx;
    MS_U16 rls;
    MS_U16 fwd;
};

struct noe_qdma_vq {
    MS_U8 vq_num[E_NOE_VQ_MAX];
};

struct noe_qdma_pq {
    MS_U8 queue;
    MS_U8 min_en;
    MS_U32 min_rate;
    MS_U8 max_en;
    MS_U32 max_rate;
    MS_U32 queue_head;
    MS_U32 queue_tail;
    MS_U8 sch;
    MS_U8 weight;
    MS_U8 hw_resv;
    MS_U8 sw_resv;
    MS_U16 txd_cnt;
};

struct noe_mac_link_status {
    MS_BOOL link_up;
    EN_NOE_SPEED speed;
    EN_NOE_DUPLEX duplex;
};

struct noe_sfq_base {
    MS_PHYADDR phy_adr[NOE_SFQ_MAX_NUM];
};

struct noe_pdma_dbg {
    MS_U32 rx[2];
    MS_U32 tx[2];
};

struct noe_fq_base {
    MS_U32 head;
    MS_U32 tail;
    MS_U32 txd_num;
    MS_U32 page_num;
    MS_U32 page_size;
};

struct noe_lro_calc_idx {
    MS_U32 ring1;
    MS_U32 ring2;
    MS_U32 ring3;
};

struct noe_sys {
    void __iomem *sysctl_base;
};

struct noe_irq {
    MS_U8 num;
    unsigned int irq[E_NOE_IRQ_MAX];
};

struct lro_ctrl {
    MS_U32 agg_cnt;
    MS_U32 agg_time;
    MS_U32 age_time;
    MS_U32 threshold;
};

struct lro_tbl{
    MS_BOOL valid;
    MS_U32 tlb_info[MHAL_NOE_MAX_INFO_FOR_EACH_ENTRY_IN_LRO_TABLE];
    MS_U32 priority;
};

//-------------------------------------------------------------------------------------------------
//  Function
//-------------------------------------------------------------------------------------------------

/* NOE SYS Init Information */
void MHal_NOE_Init(struct noe_sys *info);
void MHAL_NOE_Get_Interrupt(struct noe_irq *info);


void MHal_NOE_Write_Bits_Zero(void __iomem *addr, MS_U32 bit, MS_U32 len);
void MHal_NOE_Write_Bits_One(void __iomem *addr, MS_U32 bit, MS_U32 len);
EN_NOE_RET MHal_NOE_Set_Pin_Mux(EN_NOE_SEL_PIN_MUX mux);
void MHal_NOE_Set_MAC_Address(EN_NOE_GE_MAC ge, unsigned char p[6]);
void MHal_NOE_Get_MAC_Address(EN_NOE_GE_MAC ge, unsigned char* p);
void MHal_NOE_Reset_SW(void);
void MHal_NOE_Stop(void);
void MHal_NOE_Reset_FE(void);
void MHal_NOE_Reset_GMAC(void);
MS_BOOL MHal_NOE_Support_Auto_Polling(void);
void MHal_NOE_Set_Auto_Polling(EN_NOE_SEL enable);
void MHal_NOE_Force_Link_Mode(EN_NOE_GE_MAC ge, EN_NOE_SPEED speed, EN_NOE_DUPLEX duplex);
void MHal_NOE_MAC_Get_Link_Status(EN_NOE_GE_MAC ge, struct noe_mac_link_status *link_status);
void MHal_NOE_Set_Vlan_Info(void);

MS_BOOL MHal_NOE_GPHY_Get_Link_Info(MS_U8 phy_addr);

/* DMA */
void MHal_NOE_Offoad_Checksum(EN_NOE_GE_MAC ge, unsigned char offload);
void MHal_NOE_GLO_Reset(void);
void MHal_NOE_LRO_Set_Prefetch(void);
void MHal_NOE_DMA_Init(EN_NOE_DMA dma, EN_NOE_DIR dir, EN_NOE_RING ring_no, struct noe_dma_info *dma_info);
void MHal_NOE_Dma_Init_Global_Config(EN_NOE_DMA dma);
EN_NOE_RET MHal_NOE_Dma_Is_Idle(EN_NOE_DMA dma);
MS_U32 MHal_NOE_DMA_Get_Calc_Idx(EN_NOE_DIR dir);
void MHal_NOE_DMA_Update_Calc_Idx(EN_NOE_DIR dir, MS_U32 rx_dma_owner_idx);
void MHal_NOE_DMA_SFQ_Init(struct noe_sfq_base *adr_info);
void MHal_NOE_DMA_FQ_Init(struct noe_fq_base *info);
MS_U32 MHal_NOE_QDMA_Get_Tx(void);
void MHal_NOE_QDMA_Update_Tx(EN_NOE_QDMA_TX_TYPE type, MS_U32 adr);
EN_NOE_RET MHal_NOE_LRO_Set_Ring_Cfg(EN_NOE_RING ring_idx, unsigned int sip, unsigned int dip, unsigned int sport, unsigned int dport);
void MHal_NOE_LRO_Set_Cfg(void);
void MHal_NOE_LRO_Set_Auto_Learn_Cfg(void);
void MHal_NOE_LRO_Set_Ip(unsigned int lan_ip);
void MHal_NOE_LRO_Reset_Rx_Ring(EN_NOE_RING ring_no, u32 phy_addr, u32 desc_num, u32 desc_idx);
void MHal_NOE_LRO_Get_Calc_Idx(struct noe_lro_calc_idx *idx);
void MHal_NOE_LRO_Update_Calc_Idx(MS_U32 ring_no, MS_U32 dma_owner_idx);
void MHal_NOE_LRO_Control(EN_NOE_LRO_CTRL_TYPE type, MS_U32 param);
void MHAL_NOE_LRO_Get_Control(EN_NOE_RING ring, struct lro_ctrl *info);
void MHAL_NOE_LRO_Get_Table(MS_U32 entry, struct lro_tbl *info);
/* IO Coherence */
void MHal_NOE_IO_Enable_Coherence(MS_BOOL qos);

/* Reset Engine */
EN_NOE_RET MHal_NOE_Need_Reset(void);
void MHal_NOE_Do_Reset(void);

/* MII MGR */
void MHal_NOE_Init_Mii_Mgr(EN_NOE_GE_MAC ge, u32 addr, unsigned char force_mode);
void MHal_NOE_Dump_Mii_Mgr(int port_no, int from, int to, int is_local, int page_no);
EN_NOE_RET MHal_NOE_Write_Mii_Mgr(u32 addr, u32 reg, u32 write_data);
EN_NOE_RET MHal_NOE_Read_Mii_Mgr(u32 addr, u32 reg, u32 *read_data);
EN_NOE_RET MHal_NOE_Write45_Mii_Mgr(u32 port_num, u32 dev_addr, u32 reg_addr, u32 write_data);
EN_NOE_RET MHal_NOE_Read45_Mii_Mgr(u32 port_num, u32 dev_addr, u32 reg_addr, u32 *read_data);

/* Interrupt */
void MHal_NOE_Set_Grp_Intr(unsigned char delay_intr, unsigned char rx_only);

void MHal_NOE_Init_Sep_Intr(EN_NOE_DMA dma, EN_NOE_DIR dir);
void MHal_NOE_Clear_Sep_Intr_Status(EN_NOE_DIR dir);
void MHal_NOE_Enable_Sep_Intr(EN_NOE_DIR dir);
void MHal_NOE_Disable_Sep_Intr(EN_NOE_DIR dir);
void MHal_NOE_Enable_Sep_Delay_Intr(EN_NOE_DMA dma, EN_NOE_DELAY delay_intr);
void MHal_NOE_Clear_Sep_Intr_Specific_Status(EN_NOE_DIR dir, EN_NOE_INTR_CLR_STATUS status);
MS_U8 MHal_NOE_Get_Sep_Intr_Status(EN_NOE_DIR dir);

void MHal_NOE_Enable_Link_Intr(void);
void MHal_NOE_Disable_Link_Intr(void);
MS_BOOL MHal_NOE_Get_Link_Intr_Status(void);
void MHal_NOE_Clear_Link_Intr_Status(void);

void MHal_NOE_Get_Intr_Status(EN_NOE_DMA dma, MS_U32 *recv, MS_U32 *xmit);
void MHal_NOE_Clear_Intr_Status(EN_NOE_DMA dma);
void MHal_NOE_Enable_Intr_Status(EN_NOE_DMA dma);
void MHal_NOE_DMA_Enable_Specific_Intr(EN_NOE_DMA dma, EN_NOE_INTR_INFO e_intr);

void MHal_NOE_Set_Intr_Mask(EN_NOE_DELAY e_dly);


/* Dump DBG Info  */
void MHal_NOE_Set_DBG(EN_NOE_HAL_LOG level);
EN_NOE_RET MHal_NOE_LRO_Get_Ring_Info(EN_NOE_DMA dma, EN_NOE_DIR dir, EN_NOE_RING ring_no, struct noe_dma_info *info);
void MHal_NOE_Get_Intr_Info(struct noe_intr_info *info);
void MHal_NOE_Get_Mac_Info(EN_NOE_GE_MAC ge, struct noe_mac_info *info);
void MHal_NOE_Get_Pse_Info(struct noe_pse_info *info);
void MHal_NOE_Get_Qdma_Info(EN_NOE_QDMA_INFO_TYPE type, void *info);
void MHal_NOE_Get_Pdma_Info(EN_NOE_PDMA_INFO_TYPE type, void *info);

/* I/O Control for QDMA */
MS_U32 MHal_NOE_DMA_Get_Queue_Cfg(MS_U32 queue_no);
void MHal_NOE_DMA_Set_Queue_Cfg(MS_U32 queue_no, MS_U32 cfg);


void MHal_NOE_Set_WoL(MS_BOOL enable);
void MHal_NOE_Dma_DeInit_Global_Config(EN_NOE_DMA dma);
#ifdef CONFIG_MDIO_IC1819
extern unsigned int mdio_bb_read(int phy,int reg);
extern unsigned int mdio_bb_write(unsigned int phy,unsigned int reg,unsigned int val);
#endif
#endif /* MHAL_NOE_H */

