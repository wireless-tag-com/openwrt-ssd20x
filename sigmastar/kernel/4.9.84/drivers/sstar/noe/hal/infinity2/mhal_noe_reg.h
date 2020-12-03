/*
* mhal_noe_reg.h- Sigmastar
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
/// @file   MHAL_NOE_REG.h
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MHAL_NOE_REG_H
#define MHAL_NOE_REG_H

/* bits range: for example BITS(4,0) = 0x0000001F */
#define BITS(m, n)   (~(BIT(m) - 1) & ((BIT(n) - 1) | BIT(n)))

#define ETHER_ADDR_LEN  6


//-------------------------------------------------------------------------------------------------
//  Register
//-------------------------------------------------------------------------------------------------
#if defined(CONFIG_ARM64)
#define NOE_RIU_REG_BASE(bank) (mstar_pm_base+ (bank << 9))
#else
#ifndef RIU_VIRT_BASE
#define RIU_VIRT_BASE                     0xFD000000
#endif
#define NOE_RIU_REG_BASE(bank) (RIU_VIRT_BASE + (bank << 9))
#endif /* defined(CONFIG_ARM64) */

#define NOE_RIU_REG8_ADDR(reg)                      ((reg << 1) - (reg & 1))
#define NOE_RIU_REG_ADDR(reg)                       ((reg) << 1)
#define NOE_RIU_BANK_ADR(x)                         (GET_BASE_ADDR_BY_BANK(BASE_REG_RIU_PA, (x)<<8))
#define NOE_RIU_REG(bank,oct_reg)                   INREG16( (NOE_RIU_BANK_ADR(bank)) + (NOE_RIU_REG_ADDR(oct_reg)) )

#define MHal_NOE_Read_Reg(phys)                     (*(volatile unsigned int *)(phys))
#define MHal_NOE_Write_Reg(phys, val)               ((*(volatile unsigned int *)(phys)) = (val))

#define NOE_RIU_REG_MASK_WRITE(reg, mask, value)    (reg) = ((reg) & (~(mask))) | (value)
#define NOE_RIU_REG_BITS_WRITE(reg, value, offset, len) \
{\
    MS_U32 mask = BITS((len - 1 + offset), offset);\
    MS_U32 tmp = MHal_NOE_Read_Reg(reg) & (~mask);\
    tmp = (value << offset) & mask; \
    MHal_NOE_Write_Reg(reg,tmp);\
}


#define NOE_RIU8_REG(bank,oct_reg)                  INREG8( (NOE_RIU_BANK_ADR(bank)) + (NOE_RIU_REG8_ADDR(oct_reg)) )
#define MHal_NOE_Read8_Reg(bank, oct_reg)           NOE_RIU8_REG(bank,oct_reg)
#define MHal_NOE_Write8_Reg(bank, oct_reg, val)     NOE_RIU8_REG(bank,oct_reg) = (val)

//-------------------------------------------------------------------------------------------------
//  SYS BANK
//-------------------------------------------------------------------------------------------------

#define NOE_RIU_BANK_SECGMAC4       (0x1224)
#define NOE_RIU_BANK_GMAC4          (0x121F)
#define NOE_RIU_BANK_ALBANY0        (0x0035)
#define NOE_RIU_BANK_ALBANY1        (0x0036)
#define NOE_RIU_BANK_CHIPTOP        (0x101E)
#define NOE_RIU_BANK_CLKGEN2        (0x100A)
#define NOE_RIU_BANK_PMSLEEP        (0x000E)
#define NOE_RIU_BANK_PMTOP          (0x001E)
#define NOE_RIU_BANK_ANA_MISC_GMAC  (0x110C)
#define NOE_RIU_BANK_CLKGEN0        (0x100B)
#define NOE_RIU_BANK_CLKGEN1        (0x1033)
#define NOE_RIU_BANK_NOE_MISC       (0x1214)

//-------------------------------------------------------------------------------------------------
//  GPHY Register Detail
//-------------------------------------------------------------------------------------------------
#define GPHY_MDIO_START_CL22            0x1
#define GPHY_MDIO_START_CL45            0x0
#define GPHY_MDIO_OPCODE_WRITE          0x1
#define GPHY_MDIO_OPCODE_CL22_READ      0x2
#define GPHY_MDIO_OPCODE_CL45_READ      0x3
#define GPHY_MDIO_OPCODE_CL45_ADDRESS   0x0
#define GPHY_MDIO_TA                    0x2
#define GPHY_MDIO_START_SHIFT           14
#define GPHY_MDIO_OPCODE_SHIFT          12
#define GPHY_MDIO_PHY_ADDRESS_SHIFT     7
#define GPHY_MDIO_REG_ADDRESS_SHIFT     2



#define GPHY_REG_BASIC                       (0)
#define GPHY_REG_STATUS                      (1)
#define GPHY_REG_LINK_PARTNER                (5)
#define GPHY_REG_CTRL1000                    0x09    /* 1000BASE-T control          */
#define GPHY_REG_STAT1000                    0x0a    /* 1000BASE-T status           */
#define GPHY_REG_PHYSR                       0x11



#define LPA_10HALF      0x0020  /* Can do 10mbps half-duplex   */
#define LPA_10FULL      0x0040  /* Can do 10mbps full-duplex   */
#define LPA_100HALF     0x0080  /* Can do 100mbps half-duplex  */
#define LPA_100FULL     0x0100  /* Can do 100mbps full-duplex  */
#define LPA_100BASE4    0x0200  /* Can do 100mbps 4k packets   */

#define LPA_DUPLEX      (LPA_10FULL | LPA_100FULL)
#define LPA_100         (LPA_100FULL | LPA_100HALF | LPA_100BASE4)
#define LPA_1000FULL        0x0800  /* Link partner 1000BASE-T full duplex */

#define DUPLEX_HALF         0x00
#define DUPLEX_FULL         0x01

#define PHY_CODE_SHIFT      (16)
#define PHY_REG_SHIFT       (18)
#define PHY_ADDR_SHIFT      (23)
#define PHY_RW_SHIFT        (28)
#define PHY_LOW_HIGH_SHIFT  (30)

#define PHY_CODE            (0x02)
#define PHY_LOW_HIGH        (0x01)
#define PHY_WRITE_OP        (0x01)
#define PHY_READ_OP         (0x02)

#define GPHY_AN_DONE         (0x1 << 5)
#define GPHY_LINK_UP         (0x1 << 2)


//-------------------------------------------------------------------------------------------------
//  NOE Register
//-------------------------------------------------------------------------------------------------

/* Base Address */
extern void __iomem *ethdma_sysctl_base;
#define NOE_REG_SYSCTL_BASE                     ethdma_sysctl_base
#define NOE_REG_FRAME_ENGINE_BASE               (NOE_REG_SYSCTL_BASE + 0x100000)
#define NOE_REG_PPE_BASE                        (NOE_REG_FRAME_ENGINE_BASE + 0x00C00)
#define NOE_REG_AC_BASE                         (NOE_REG_FRAME_ENGINE_BASE + 0x02000)
#define NOE_REG_METER_BASE                      (NOE_REG_FRAME_ENGINE_BASE + 0x02000)
#define NOE_REG_ETH_MAC_BASE                    (NOE_REG_FRAME_ENGINE_BASE + 0x10000)
#define NOE_REG_ETH_SW_BASE                     (NOE_REG_FRAME_ENGINE_BASE + 0x18000)


//-------------------------------------------------------------------------------------------------
//  SYSCTL_BASE Register Detail
//-------------------------------------------------------------------------------------------------
#define SYSCFG1                                 (NOE_REG_SYSCTL_BASE + 0x14)
#define CLK_CFG_0                               (NOE_REG_SYSCTL_BASE + 0x2C)
#define RSTCTRL                                 (NOE_REG_SYSCTL_BASE + 0x34)
#define RSTSTAT                                 (NOE_REG_SYSCTL_BASE + 0x38)
#define AGPIOCFG_REG                            (NOE_REG_SYSCTL_BASE + 0x3C)
#define PAD_RGMII2_MDIO_CFG                     (NOE_REG_SYSCTL_BASE + 0x58)
#define REG_NOE_IOC_ETH                         (NOE_REG_SYSCTL_BASE + 0x408)
        #define IOC_ETH_PDMA    BIT(0)
        #define IOC_ETH_QDMA    BIT(1)

#define METER_BASE NOE_REG_METER_BASE
//-------------------------------------------------------------------------------------------------
//  FRAME_ENGINE_BASE Register Detail
//-------------------------------------------------------------------------------------------------
/* Register Categories Definition */
#define RAFRAMEENGINE_OFFSET                    (0x0000)
#define RAGDMA_OFFSET                           (0x0020)
#define PSE_RELATED                             (0x0040)
#define RAPSE_OFFSET                            (0x0040)
#define RAGDMA2_OFFSET                          (0x0060)
#define RACDMA_OFFSET                           (0x0080)
#define FE_GDM_RXID1_OFFSET                     (0x0130)
#define FE_GDM_RXID2_OFFSET                     (0x0134)
#define CDMA_RELATED                            (0x0400)
#define GDMA1_RELATED                           (0x0500)
#define PDMA_RELATED                            (0x0800)
#define RAPDMA_OFFSET                           (0x0800)
#define SDM_OFFSET                              (0x0C00)
#define RAPPE_OFFSET                            (0x0200)
#define RACMTABLE_OFFSET                        (0x0400)
#define ADMA_LRO_CTRL_OFFSET                    (0x0980)
#define PDMA_LRO_ATL_OVERFLOW_ADJ_OFFSET        (0x0990)
#define ADMA_DBG_OFFSET                         (0x0A30)
#define LRO_RXRING0_OFFSET                      (0x0B00)
#define LRO_RXRING1_OFFSET                      (0x0B40)
#define LRO_RXRING2_OFFSET                      (0x0B80)
#define LRO_RXRING3_OFFSET                      (0x0BC0)
#define POLICYTABLE_OFFSET                      (0x1000)
#define GDMA2_RELATED                           (0x1500)
#define QDMA_RELATED                            (0x1800)
#define SFQ_OFFSET                              (0x1A80)




#define FE_GLO_CFG                              (NOE_REG_FRAME_ENGINE_BASE + 0x000)
#define FE_RST_GL                               (NOE_REG_FRAME_ENGINE_BASE + 0x004)
#define FE_INT_STATUS2                          (NOE_REG_FRAME_ENGINE_BASE + 0x008)
#define FE_INT_ENABLE2                          (NOE_REG_FRAME_ENGINE_BASE + 0x00C)
        #define MAC2_LINK                       BIT(25)
        #define MAC1_LINK                       BIT(24)
#define FOE_TS_T                                (NOE_REG_FRAME_ENGINE_BASE + 0x010)
#define FOE_TS  FOE_TS_T
#define FC_DROP_STA                             (NOE_REG_FRAME_ENGINE_BASE + 0x018)
#define LRO_ALT_REFRESH_TIMER                   (NOE_REG_FRAME_ENGINE_BASE + 0x01C)
#define FE_INT_GRP                              (NOE_REG_FRAME_ENGINE_BASE + 0x020)
#define PSE_FQ_CFG                              (NOE_REG_FRAME_ENGINE_BASE + PSE_RELATED + 0x00)
#define CDMA_FC_CFG                             (NOE_REG_FRAME_ENGINE_BASE + PSE_RELATED + 0x04)
#define GDMA1_FC_CFG                            (NOE_REG_FRAME_ENGINE_BASE + PSE_RELATED + 0x08)
#define GDMA2_FC_CFG                            (NOE_REG_FRAME_ENGINE_BASE + PSE_RELATED + 0x0C)
#define CDMA_OQ_STA                             (NOE_REG_FRAME_ENGINE_BASE + PSE_RELATED + 0x10)
#define GDMA1_OQ_STA                            (NOE_REG_FRAME_ENGINE_BASE + PSE_RELATED + 0x14)
#define GDMA2_OQ_STA                            (NOE_REG_FRAME_ENGINE_BASE + PSE_RELATED + 0x18)
#define PSE_IQ_STA                              (NOE_REG_FRAME_ENGINE_BASE + PSE_RELATED + 0x1C)

#define FE_GDMA1_FWD_CFG                        (NOE_REG_FRAME_ENGINE_BASE + 0x500)
#define FE_GDMA2_FWD_CFG                        (NOE_REG_FRAME_ENGINE_BASE + 0x1500)

#define PDMA_FC_CFG                             (NOE_REG_FRAME_ENGINE_BASE + 0x100)
#define FE_GLO_MISC                             (NOE_REG_FRAME_ENGINE_BASE + 0x124)
#define FE_PSE_FREE                             (NOE_REG_FRAME_ENGINE_BASE + 0x240)
#define FE_DROP_FQ                              (NOE_REG_FRAME_ENGINE_BASE + 0x244)
#define FE_DROP_FC                              (NOE_REG_FRAME_ENGINE_BASE + 0x248)
#define FE_DROP_PPE                             (NOE_REG_FRAME_ENGINE_BASE + 0x24c)
#define PDMA_FE_ALT_CF8                         (NOE_REG_FRAME_ENGINE_BASE + 0x0300)
#define PDMA_FE_ALT_SGL_CFC                     (NOE_REG_FRAME_ENGINE_BASE + 0x0304)
#define PDMA_FE_ALT_SEQ_CFC                     (NOE_REG_FRAME_ENGINE_BASE + 0x0308)
#define CDMA_CSG_CFG                            (NOE_REG_FRAME_ENGINE_BASE + CDMA_RELATED + 0x00)
#define CDMP_IG_CTRL                            (NOE_REG_FRAME_ENGINE_BASE + CDMA_RELATED + 0x00)
#define CDMP_EG_CTRL                            (NOE_REG_FRAME_ENGINE_BASE + CDMA_RELATED + 0x04)
#define PPE_AC_BCNT0                            (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x000)
#define PPE_AC_PCNT0                            (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x004)
#define PPE_MTR_CNT0                            (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x200)
#define PPE_MTR_CNT63                           (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x2FC)
#define GDMA_TX_GBCNT0                          (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x300)
#define GDMA_TX_GPCNT0                          (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x304)
#define GDMA_TX_SKIPCNT0                        (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x308)
#define GDMA_TX_COLCNT0                         (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x30C)
#define GDMA_RX_GBCNT0                          (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x320)
#define GDMA_RX_GPCNT0                          (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x324)
#define GDMA_RX_OERCNT0                         (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x328)
#define GDMA_RX_FERCNT0                         (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x32C)
#define GDMA_RX_SERCNT0                         (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x330)
#define GDMA_RX_LERCNT0                         (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x334)
#define GDMA_RX_CERCNT0                         (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x338)
#define GDMA_RX_FCCNT1                          (NOE_REG_FRAME_ENGINE_BASE + RACMTABLE_OFFSET + 0x33C)
#define ADMA_LRO_CTRL_DW0                       (NOE_REG_FRAME_ENGINE_BASE + ADMA_LRO_CTRL_OFFSET + 0x00)
#define ADMA_LRO_CTRL_DW1                       (NOE_REG_FRAME_ENGINE_BASE + ADMA_LRO_CTRL_OFFSET + 0x04)
#define ADMA_LRO_CTRL_DW2                       (NOE_REG_FRAME_ENGINE_BASE + ADMA_LRO_CTRL_OFFSET + 0x08)
#define ADMA_LRO_CTRL_DW3                       (NOE_REG_FRAME_ENGINE_BASE + ADMA_LRO_CTRL_OFFSET + 0x0C)

#define TX_BASE_PTR0                            (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x000)
#define TX_MAX_CNT0                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x004)
#define TX_CTX_IDX0                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x008)
#define TX_DTX_IDX0                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x00C)
#define TX_BASE_PTR1                            (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x010)
#define TX_MAX_CNT1                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x014)
#define TX_CTX_IDX1                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x018)
#define TX_DTX_IDX1                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x01C)
#define TX_BASE_PTR2                            (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x020)
#define TX_MAX_CNT2                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x024)
#define TX_CTX_IDX2                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x028)
#define TX_DTX_IDX2                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x02C)
#define TX_BASE_PTR3                            (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x030)
#define TX_MAX_CNT3                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x034)
#define TX_CTX_IDX3                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x038)
#define TX_DTX_IDX3                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x03C)
#define RX_BASE_PTR0                            (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x100)
#define RX_MAX_CNT0                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x104)
#define RX_CALC_IDX0                            (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x108)
#define RX_DRX_IDX0                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x10C)
#define RX_BASE_PTR1                            (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x110)
#define RX_MAX_CNT1                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x114)
#define RX_CALC_IDX1                            (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x118)
#define RX_DRX_IDX1                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x11C)
#define RX_BASE_PTR2                            (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x120)
#define RX_MAX_CNT2                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x124)
#define RX_CALC_IDX2                            (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x128)
#define RX_DRX_IDX2                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x12C)
#define RX_BASE_PTR3                            (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x130)
#define RX_MAX_CNT3                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x134)
#define RX_CALC_IDX3                            (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x138)
#define RX_DRX_IDX3                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x13C)
#define PDMA_INFO                               (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x200)
#define PDMA_GLO_CFG                            (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x204)
#define PDMA_RST_IDX                            (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x208)
#define PDMA_RST_CFG                            (PDMA_RST_IDX)
#define DLY_INT_CFG                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x20C)
#define FREEQ_THRES                             (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x210)
#define INT_STATUS                              (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x220)
#define FE_INT_STATUS                           (INT_STATUS)
        #define RX_COHERENT                     BIT(31)
        #define RX_DLY_INT                      BIT(30)
        #define TX_COHERENT                     BIT(29)
        #define TX_DLY_INT                      BIT(28)
        #define RING3_RX_DLY_INT                BIT(27)
        #define RING2_RX_DLY_INT                BIT(26)
        #define RING1_RX_DLY_INT                BIT(25)
        #define RXD_ERROR                       BIT(24)
        #define ALT_RPLC_INT3                   BIT(23)
        #define ALT_RPLC_INT2                   BIT(22)
        #define ALT_RPLC_INT1                   BIT(21)
        #define RX_DONE_INT3                    BIT(19)
        #define RX_DONE_INT2                    BIT(18)
        #define RX_DONE_INT1                    BIT(17)
        #define RX_DONE_INT0                    BIT(16)
        #define TX_DONE_INT3                    BIT(3)
        #define TX_DONE_INT2                    BIT(2)
        #define TX_DONE_INT1                    BIT(1)
        #define TX_DONE_INT0                    BIT(0)
#define INT_MASK                                (NOE_REG_FRAME_ENGINE_BASE + PDMA_RELATED + 0x228)
#define FE_INT_ENABLE                           (INT_MASK)
#define PDMA_INT_GRP1                           (NOE_REG_FRAME_ENGINE_BASE + RAPDMA_OFFSET + 0x250)
#define PDMA_INT_GRP2                           (NOE_REG_FRAME_ENGINE_BASE + RAPDMA_OFFSET + 0x254)
#define SCH_Q01_CFG                             (NOE_REG_FRAME_ENGINE_BASE + RAPDMA_OFFSET + 0x280)
#define SCH_Q23_CFG                             (NOE_REG_FRAME_ENGINE_BASE + RAPDMA_OFFSET + 0x284)
#define GDMA1_FWD_CFG                           (NOE_REG_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x00)
#define GDMA1_SHPR_CFG                          (NOE_REG_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x04)
#define GDMA1_SCH_CFG                           GDMA1_SHPR_CFG

#define GDMA1_MAC_ADRL                          (NOE_REG_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x08)
#define GDMA1_MAC_ADRH                          (NOE_REG_FRAME_ENGINE_BASE + GDMA1_RELATED + 0x0C)

#define GDMA2_FWD_CFG                           (NOE_REG_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x00)
#define GDMA2_SHPR_CFG                          (NOE_REG_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x04)
#define GDMA2_SCH_CFG                           GDMA2_SHPR_CFG
#define GDMA2_MAC_ADRL                          (NOE_REG_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x08)
#define GDMA2_MAC_ADRH                          (NOE_REG_FRAME_ENGINE_BASE + GDMA2_RELATED + 0x0C)


#define  QTX_CFG_0                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x000)
#define  QTX_SCH_0                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x004)
#define  QTX_HEAD_0                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x008)
#define  QTX_TAIL_0                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x00C)
#define  QTX_CFG_1                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x010)
#define  QTX_SCH_1                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x014)
#define  QTX_HEAD_1                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x018)
#define  QTX_TAIL_1                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x01C)
#define  QTX_CFG_2                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x020)
#define  QTX_SCH_2                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x024)
#define  QTX_HEAD_2                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x028)
#define  QTX_TAIL_2                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x02C)
#define  QTX_CFG_3                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x030)
#define  QTX_SCH_3                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x034)
#define  QTX_HEAD_3                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x038)
#define  QTX_TAIL_3                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x03C)
#define  QTX_CFG_4                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x040)
#define  QTX_SCH_4                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x044)
#define  QTX_HEAD_4                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x048)
#define  QTX_TAIL_4                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x04C)
#define  QTX_CFG_5                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x050)
#define  QTX_SCH_5                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x054)
#define  QTX_HEAD_5                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x058)
#define  QTX_TAIL_5                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x05C)
#define  QTX_CFG_6                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x060)
#define  QTX_SCH_6                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x064)
#define  QTX_HEAD_6                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x068)
#define  QTX_TAIL_6                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x06C)
#define  QTX_CFG_7                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x070)
#define  QTX_SCH_7                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x074)
#define  QTX_HEAD_7                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x078)
#define  QTX_TAIL_7                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x07C)
#define  QTX_CFG_8                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x080)
#define  QTX_SCH_8                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x084)
#define  QTX_HEAD_8                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x088)
#define  QTX_TAIL_8                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x08C)
#define  QTX_CFG_9                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x090)
#define  QTX_SCH_9                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x094)
#define  QTX_HEAD_9                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x098)
#define  QTX_TAIL_9                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x09C)
#define  QTX_CFG_10                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0A0)
#define  QTX_SCH_10                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0A4)
#define  QTX_HEAD_10                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0A8)
#define  QTX_TAIL_10                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0AC)
#define  QTX_CFG_11                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0B0)
#define  QTX_SCH_11                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0B4)
#define  QTX_HEAD_11                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0B8)
#define  QTX_TAIL_11                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0BC)
#define  QTX_CFG_12                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0C0)
#define  QTX_SCH_12                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0C4)
#define  QTX_HEAD_12                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0C8)
#define  QTX_TAIL_12                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0CC)
#define  QTX_CFG_13                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0D0)
#define  QTX_SCH_13                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0D4)
#define  QTX_HEAD_13                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0D8)
#define  QTX_TAIL_13                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0DC)
#define  QTX_CFG_14                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0E0)
#define  QTX_SCH_14                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0E4)
#define  QTX_HEAD_14                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0E8)
#define  QTX_TAIL_14                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0EC)
#define  QTX_CFG_15                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0F0)
#define  QTX_SCH_15                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0F4)
#define  QTX_HEAD_15                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0F8)
#define  QTX_TAIL_15                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x0FC)
#define  QRX_BASE_PTR_0                         (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x100)
#define  QRX_MAX_CNT_0                          (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x104)
#define  QRX_CRX_IDX_0                          (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x108)
#define  QRX_DRX_IDX_0                          (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x10C)
#define  QRX_BASE_PTR_1                         (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x110)
#define  QRX_MAX_CNT_1                          (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x114)
#define  QRX_CRX_IDX_1                          (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x118)
#define  QRX_DRX_IDX_1                          (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x11C)

#define  VQTX_TB_BASE_0                         (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x180)
#define  VQTX_TB_BASE_1                         (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x184)
#define  VQTX_TB_BASE_2                         (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x188)
#define  VQTX_TB_BASE_3                         (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x18C)

#define  QDMA_INFO                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x200)
#define  QDMA_GLO_CFG                           (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x204)
#define  QDMA_RST_IDX                           (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x208)
#define  QDMA_RST_CFG                           (QDMA_RST_IDX)
#define  QDMA_DELAY_INT                         (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x20C)
#define  QDMA_FC_THRES                          (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x210)
#define  QDMA_TX_SCH                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x214)
#define  QDMA_INT_STS                           (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x218)
#define  QFE_INT_STATUS                         (QDMA_INT_STS)
#define  QDMA_INT_MASK                          (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x21C)
#define  QFE_INT_ENABLE                         (QDMA_INT_MASK)
#define  QDMA_TRTCM                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x220)
#define  QDMA_INT_GRP1                          (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x220)
#define  QDMA_DATA0                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x224)
#define  QDMA_INT_GRP2                          (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x224)
#define  QDMA_DATA1                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x228)
#define  QDMA_RED_THRES                         (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x22C)
#define  QDMA_TEST                              (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x230)
#define  QDMA_DMA                               (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x234)
#define  QDMA_BMU                               (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x238)
#define  QDMA_HRED1                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x240)
#define  QDMA_HRED2                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x244)
#define  QDMA_SRED1                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x248)
#define  QDMA_SRED2                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x24C)
#define  QTX_MIB_IF                             (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x2BC)
#define  QTX_CTX_PTR                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x300)
#define  QTX_DTX_PTR                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x304)
#define  QTX_FWD_CNT                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x308)
#define  QTX_CRX_PTR                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x310)
#define  QTX_DRX_PTR                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x314)
#define  QTX_RLS_CNT                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x318)
#define  QDMA_FQ_HEAD                           (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x320)
#define  QDMA_FQ_TAIL                           (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x324)
#define  QDMA_FQ_CNT                            (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x328)
#define  QDMA_FQ_BLEN                           (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x32C)
#define  QTX_Q0MIN_BK                           (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x350)
#define  QTX_Q1MIN_BK                           (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x354)
#define  QTX_Q2MIN_BK                           (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x358)
#define  QTX_Q3MIN_BK                           (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x35C)
#define  QTX_Q0MAX_BK                           (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x360)
#define  QTX_Q1MAX_BK                           (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x364)
#define  QTX_Q2MAX_BK                           (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x368)
#define  QTX_Q3MAX_BK                           (NOE_REG_FRAME_ENGINE_BASE + QDMA_RELATED + 0x36C)


#define LRO_RX1_DLY_INT                         (NOE_REG_FRAME_ENGINE_BASE + 0x0a70)
#define LRO_RX2_DLY_INT                         (NOE_REG_FRAME_ENGINE_BASE + 0x0a74)
#define LRO_RX3_DLY_INT                         (NOE_REG_FRAME_ENGINE_BASE + 0x0a78)


#define PDMA_LRO_ATL_OVERFLOW_ADJ               (NOE_REG_FRAME_ENGINE_BASE + PDMA_LRO_ATL_OVERFLOW_ADJ_OFFSET)
#define LRO_ALT_SCORE_DELTA                     (NOE_REG_FRAME_ENGINE_BASE + 0x0a4c)
#define LRO_MAX_AGG_TIME                        (NOE_REG_FRAME_ENGINE_BASE + 0x0a5c)


#define LRO_RX_RING0_DIP_DW0                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING0_OFFSET + 0x04)
#define LRO_RX_RING0_DIP_DW1                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING0_OFFSET + 0x08)
#define LRO_RX_RING0_DIP_DW2                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING0_OFFSET + 0x0C)
#define LRO_RX_RING0_DIP_DW3                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING0_OFFSET + 0x10)
#define LRO_RX_RING0_CTRL_DW1                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING0_OFFSET + 0x28)
#define LRO_RX_RING0_CTRL_DW2                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING0_OFFSET + 0x2C)
#define LRO_RX_RING0_CTRL_DW3                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING0_OFFSET + 0x30)
#define LRO_RX_RING1_STP_DTP_DW                 (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING1_OFFSET + 0x00)
#define LRO_RX_RING1_DIP_DW0                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING1_OFFSET + 0x04)
#define LRO_RX_RING1_DIP_DW1                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING1_OFFSET + 0x08)
#define LRO_RX_RING1_DIP_DW2                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING1_OFFSET + 0x0C)
#define LRO_RX_RING1_DIP_DW3                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING1_OFFSET + 0x10)
#define LRO_RX_RING1_SIP_DW0                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING1_OFFSET + 0x14)
#define LRO_RX_RING1_SIP_DW1                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING1_OFFSET + 0x18)
#define LRO_RX_RING1_SIP_DW2                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING1_OFFSET + 0x1C)
#define LRO_RX_RING1_SIP_DW3                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING1_OFFSET + 0x20)
#define LRO_RX_RING1_CTRL_DW0                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING1_OFFSET + 0x24)
#define LRO_RX_RING1_CTRL_DW1                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING1_OFFSET + 0x28)
#define LRO_RX_RING1_CTRL_DW2                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING1_OFFSET + 0x2C)
#define LRO_RX_RING1_CTRL_DW3                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING1_OFFSET + 0x30)
#define LRO_RX_RING2_STP_DTP_DW                 (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING2_OFFSET + 0x00)
#define LRO_RX_RING2_DIP_DW0                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING2_OFFSET + 0x04)
#define LRO_RX_RING2_DIP_DW1                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING2_OFFSET + 0x08)
#define LRO_RX_RING2_DIP_DW2                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING2_OFFSET + 0x0C)
#define LRO_RX_RING2_DIP_DW3                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING2_OFFSET + 0x10)
#define LRO_RX_RING2_SIP_DW0                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING2_OFFSET + 0x14)
#define LRO_RX_RING2_SIP_DW1                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING2_OFFSET + 0x18)
#define LRO_RX_RING2_SIP_DW2                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING2_OFFSET + 0x1C)
#define LRO_RX_RING2_SIP_DW3                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING2_OFFSET + 0x20)
#define LRO_RX_RING2_CTRL_DW0                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING2_OFFSET + 0x24)
#define LRO_RX_RING2_CTRL_DW1                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING2_OFFSET + 0x28)
#define LRO_RX_RING2_CTRL_DW2                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING2_OFFSET + 0x2C)
#define LRO_RX_RING2_CTRL_DW3                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING2_OFFSET + 0x30)

#define LRO_RX_RING3_STP_DTP_DW                 (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING3_OFFSET + 0x00)
#define LRO_RX_RING3_DIP_DW0                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING3_OFFSET + 0x04)
#define LRO_RX_RING3_DIP_DW1                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING3_OFFSET + 0x08)
#define LRO_RX_RING3_DIP_DW2                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING3_OFFSET + 0x0C)
#define LRO_RX_RING3_DIP_DW3                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING3_OFFSET + 0x10)
#define LRO_RX_RING3_SIP_DW0                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING3_OFFSET + 0x14)
#define LRO_RX_RING3_SIP_DW1                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING3_OFFSET + 0x18)
#define LRO_RX_RING3_SIP_DW2                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING3_OFFSET + 0x1C)
#define LRO_RX_RING3_SIP_DW3                    (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING3_OFFSET + 0x20)
#define LRO_RX_RING3_CTRL_DW0                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING3_OFFSET + 0x24)
#define LRO_RX_RING3_CTRL_DW1                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING3_OFFSET + 0x28)
#define LRO_RX_RING3_CTRL_DW2                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING3_OFFSET + 0x2C)
#define LRO_RX_RING3_CTRL_DW3                   (NOE_REG_FRAME_ENGINE_BASE + LRO_RXRING3_OFFSET + 0x30)
#define ADMA_TX_DBG0                            (NOE_REG_FRAME_ENGINE_BASE + ADMA_DBG_OFFSET + 0x00)
#define ADMA_TX_DBG1                            (NOE_REG_FRAME_ENGINE_BASE + ADMA_DBG_OFFSET + 0x04)
#define ADMA_RX_DBG0                            (NOE_REG_FRAME_ENGINE_BASE + ADMA_DBG_OFFSET + 0x08)
#define ADMA_RX_DBG1                            (NOE_REG_FRAME_ENGINE_BASE + ADMA_DBG_OFFSET + 0x0C)


#define FE_GDM_RXID1        (NOE_REG_FRAME_ENGINE_BASE + FE_GDM_RXID1_OFFSET)
    #define GDM_VLAN_PRI7_RXID_SEL  BITS(30, 31)
    #define GDM_VLAN_PRI6_RXID_SEL  BITS(28, 29)
    #define GDM_VLAN_PRI5_RXID_SEL  BITS(26, 27)
    #define GDM_VLAN_PRI4_RXID_SEL  BITS(24, 25)
    #define GDM_VLAN_PRI3_RXID_SEL  BITS(22, 23)
    #define GDM_VLAN_PRI2_RXID_SEL  BITS(20, 21)
    #define GDM_VLAN_PRI1_RXID_SEL  BITS(18, 19)
    #define GDM_VLAN_PRI0_RXID_SEL  BITS(16, 17)
    #define GDM_TCP_ACK_RXID_SEL    BITS(4, 5)
    #define GDM_TCP_ACK_WZPC    BIT(3)
    #define GDM_RXID_PRI_SEL    BITS(0, 2)


#define FE_GDM_RXID2                            (NOE_REG_FRAME_ENGINE_BASE + FE_GDM_RXID2_OFFSET)
        #define GDM_STAG7_RXID_SEL  BITS(30, 31)
        #define GDM_STAG6_RXID_SEL  BITS(28, 29)
        #define GDM_STAG5_RXID_SEL  BITS(26, 27)
        #define GDM_STAG4_RXID_SEL  BITS(24, 25)
        #define GDM_STAG3_RXID_SEL  BITS(22, 23)
        #define GDM_STAG2_RXID_SEL  BITS(20, 21)
        #define GDM_STAG1_RXID_SEL  BITS(18, 19)
        #define GDM_STAG0_RXID_SEL  BITS(16, 17)
        #define GDM_PID2_RXID_SEL   BITS(2, 3)
        #define GDM_PID1_RXID_SEL   BITS(0, 1)


#define QDMA_PAGE                               (NOE_REG_FRAME_ENGINE_BASE + 0x19f0)

/*SFQ use*/
#define VQTX_TB_BASE0                           (NOE_REG_FRAME_ENGINE_BASE + 0x1980)
#define VQTX_TB_BASE1                           (NOE_REG_FRAME_ENGINE_BASE + 0x1984)
#define VQTX_TB_BASE2                           (NOE_REG_FRAME_ENGINE_BASE + 0x1988)
#define VQTX_TB_BASE3                           (NOE_REG_FRAME_ENGINE_BASE + 0x198C)

#define VQTX_GLO                                (NOE_REG_FRAME_ENGINE_BASE + SFQ_OFFSET)
#define VQTX_INVLD_PTR                          (NOE_REG_FRAME_ENGINE_BASE + SFQ_OFFSET + 0x0C)
#define VQTX_NUM                                (NOE_REG_FRAME_ENGINE_BASE + SFQ_OFFSET + 0x10)
#define VQTX_SCH                                (NOE_REG_FRAME_ENGINE_BASE + SFQ_OFFSET + 0x18)
#define VQTX_HASH_CFG                           (NOE_REG_FRAME_ENGINE_BASE + SFQ_OFFSET + 0x20)
#define VQTX_HASH_SD                            (NOE_REG_FRAME_ENGINE_BASE + SFQ_OFFSET + 0x24)
#define VQTX_VLD_CFG                            (NOE_REG_FRAME_ENGINE_BASE + SFQ_OFFSET + 0x30)
#define VQTX_MIB_IF                             (NOE_REG_FRAME_ENGINE_BASE + SFQ_OFFSET + 0x3C)
#define VQTX_MIB_PCNT                           (NOE_REG_FRAME_ENGINE_BASE + SFQ_OFFSET + 0x40)
#define VQTX_MIB_BCNT0                          (NOE_REG_FRAME_ENGINE_BASE + SFQ_OFFSET + 0x44)
#define VQTX_MIB_BCNT1                          (NOE_REG_FRAME_ENGINE_BASE + SFQ_OFFSET + 0x48)
        #define VQTX_0_BIND_QID (PQ0 << 0)
        #define VQTX_1_BIND_QID (PQ1 << 8)
        #define VQTX_2_BIND_QID (PQ2 << 16)
        #define VQTX_3_BIND_QID (PQ3 << 24)
        #define VQTX_4_BIND_QID (PQ4 << 0)
        #define VQTX_5_BIND_QID (PQ5 << 8)
        #define VQTX_6_BIND_QID (PQ6 << 16)
        #define VQTX_7_BIND_QID (PQ7 << 24)

#define VQTX_TB_BASE4                           (NOE_REG_FRAME_ENGINE_BASE + 0x1990)
#define VQTX_TB_BASE5                           (NOE_REG_FRAME_ENGINE_BASE + 0x1994)
#define VQTX_TB_BASE6                           (NOE_REG_FRAME_ENGINE_BASE + 0x1998)
#define VQTX_TB_BASE7                           (NOE_REG_FRAME_ENGINE_BASE + 0x199C)
#define VQTX_0_3_BIND_QID                       (NOE_REG_FRAME_ENGINE_BASE + SFQ_OFFSET + 0x140)
#define VQTX_4_7_BIND_QID                       (NOE_REG_FRAME_ENGINE_BASE + SFQ_OFFSET + 0x144)


#define POLICY_TBL_BASE                         (NOE_REG_FRAME_ENGINE_BASE + POLICYTABLE_OFFSET)
#define AC_BASE                                 (NOE_REG_FRAME_ENGINE_BASE + 0x2000)



//-------------------------------------------------------------------------------------------------
//  PPE_BASE Register Detail
//-------------------------------------------------------------------------------------------------
#define PPE_GLO_CFG                             (NOE_REG_PPE_BASE + 0x200)
    #define DFL_TTL0_DRP        (0)
#define PPE_FLOW_CFG                            (NOE_REG_PPE_BASE + 0x204)
#define PPE_FLOW_SET                            PPE_FLOW_CFG
    #define BIT_FBC_FOE             (1<<0)
    #define BIT_FMC_FOE             (1<<1)
    #define BIT_FUC_FOE             (1<<2)
    #define BIT_UDP_IP4F_NAT_EN     (1<<7)
    #define BIT_IPV6_3T_ROUTE_EN    (1<<8)
    #define BIT_IPV6_5T_ROUTE_EN    (1<<9)
    #define BIT_IPV6_6RD_EN         (1<<10)
    #define BIT_IPV4_NAT_EN         (1<<12)
    #define BIT_IPV4_NAPT_EN        (1<<13)
    #define BIT_IPV4_DSL_EN         (1<<14)
    #define BIT_IP_PROT_CHK_BLIST   (1<<16)
    #define BIT_IPV4_NAT_FRAG_EN    (1<<17)
    #define BIT_IPV6_HASH_FLAB      (1<<18)
    #define BIT_IPV4_HASH_GREK      (1<<19)
    #define BIT_IPV6_HASH_GREK      (1<<20)

#define PPE_IP_PROT_CHK                         (NOE_REG_PPE_BASE + 0x208)
#define PPE_IP_PROT_0                           (NOE_REG_PPE_BASE + 0x20C)
#define PPE_IP_PROT_1                           (NOE_REG_PPE_BASE + 0x210)
#define PPE_IP_PROT_2                           (NOE_REG_PPE_BASE + 0x214)
#define PPE_IP_PROT_3                           (NOE_REG_PPE_BASE + 0x218)
#define PPE_TB_CFG                              (NOE_REG_PPE_BASE + 0x21C)
#define PPE_FOE_CFG                             PPE_TB_CFG
    #define FOE_SEARCH_MISS_DROP  (0)
    #define FOE_SEARCH_MISS_DROP2 (1)
    #define FOE_SEARCH_MISS_ONLY_FWD_CPU (2)
    #define FOE_SEARCH_MISS_FWD_CPU_BUILD_ENTRY (3)

    #define FOE_TBL_NUM_1K     (0)
    #define FOE_TBL_NUM_2K     (1)
    #define FOE_TBL_NUM_4K     (2)
    #define FOE_TBL_NUM_8K     (3)
    #define FOE_TBL_NUM_16K    (4)

#define PPE_TB_BASE                             (NOE_REG_PPE_BASE + 0x220)
#define PPE_FOE_BASE                            (PPE_TB_BASE)
#define PPE_TB_USED                             (NOE_REG_PPE_BASE + 0x224)
#define PPE_BNDR                                (NOE_REG_PPE_BASE + 0x228)
#define PPE_FOE_BNDR                            PPE_BNDR
#define PPE_BIND_LMT_0                          (NOE_REG_PPE_BASE + 0x22C)
#define PPE_FOE_LMT1                            (PPE_BIND_LMT_0)
#define PPE_BIND_LMT_1                          (NOE_REG_PPE_BASE + 0x230)
#define PPE_FOE_LMT2                            PPE_BIND_LMT_1
#define PPE_KA                                  (NOE_REG_PPE_BASE + 0x234)
#define PPE_FOE_KA                              PPE_KA
#define PPE_UNB_AGE                             (NOE_REG_PPE_BASE + 0x238)
#define PPE_FOE_UNB_AGE                         PPE_UNB_AGE
#define PPE_BND_AGE_0                           (NOE_REG_PPE_BASE + 0x23C)
#define PPE_FOE_BND_AGE0                        PPE_BND_AGE_0
#define PPE_BND_AGE_1                           (NOE_REG_PPE_BASE + 0x240)
#define PPE_FOE_BND_AGE1                        PPE_BND_AGE_1
#define PPE_HASH_SEED                           (NOE_REG_PPE_BASE + 0x244)

#define PPE_MCAST_L_10                          (NOE_REG_PPE_BASE + 0x00)
#define PPE_MCAST_H_10                          (NOE_REG_PPE_BASE + 0x04)
#define PPE_DFT_CPORT                           (NOE_REG_PPE_BASE + 0x248)
#define PPE_MCAST_PPSE                          (NOE_REG_PPE_BASE + 0x284)
#define PPE_MCAST_L_0                           (NOE_REG_PPE_BASE + 0x288)
#define PPE_MCAST_H_0                           (NOE_REG_PPE_BASE + 0x28C)
#define PPE_MCAST_L_1                           (NOE_REG_PPE_BASE + 0x290)
#define PPE_MCAST_H_1                           (NOE_REG_PPE_BASE + 0x294)
#define PPE_MCAST_L_2                           (NOE_REG_PPE_BASE + 0x298)
#define PPE_MCAST_H_2                           (NOE_REG_PPE_BASE + 0x29C)
#define PPE_MCAST_L_3                           (NOE_REG_PPE_BASE + 0x2A0)
#define PPE_MCAST_H_3                           (NOE_REG_PPE_BASE + 0x2A4)
#define PPE_MCAST_L_4                           (NOE_REG_PPE_BASE + 0x2A8)
#define PPE_MCAST_H_4                           (NOE_REG_PPE_BASE + 0x2AC)
#define PPE_MCAST_L_5                           (NOE_REG_PPE_BASE + 0x2B0)
#define PPE_MCAST_H_5                           (NOE_REG_PPE_BASE + 0x2B4)
#define PPE_MCAST_L_6                           (NOE_REG_PPE_BASE + 0x2BC)
#define PPE_MCAST_H_6                           (NOE_REG_PPE_BASE + 0x2C0)
#define PPE_MCAST_L_7                           (NOE_REG_PPE_BASE + 0x2C4)
#define PPE_MCAST_H_7                           (NOE_REG_PPE_BASE + 0x2C8)
#define PPE_MCAST_L_8                           (NOE_REG_PPE_BASE + 0x2CC)
#define PPE_MCAST_H_8                           (NOE_REG_PPE_BASE + 0x2D0)
#define PPE_MCAST_L_9                           (NOE_REG_PPE_BASE + 0x2D4)
#define PPE_MCAST_H_9                           (NOE_REG_PPE_BASE + 0x2D8)
#define PPE_MCAST_L_A                           (NOE_REG_PPE_BASE + 0x2DC)
#define PPE_MCAST_H_A                           (NOE_REG_PPE_BASE + 0x2E0)
#define PPE_MCAST_L_B                           (NOE_REG_PPE_BASE + 0x2E4)
#define PPE_MCAST_H_B                           (NOE_REG_PPE_BASE + 0x2E8)
#define PPE_MCAST_L_C                           (NOE_REG_PPE_BASE + 0x2EC)
#define PPE_MCAST_H_C                           (NOE_REG_PPE_BASE + 0x2F0)
#define PPE_MCAST_L_D                           (NOE_REG_PPE_BASE + 0x2F4)
#define PPE_MCAST_H_D                           (NOE_REG_PPE_BASE + 0x2F8)
#define PPE_MCAST_L_E                           (NOE_REG_PPE_BASE + 0x2FC)
#define PPE_MCAST_H_E                           (NOE_REG_PPE_BASE + 0x2E0)
#define PPE_MCAST_L_F                           (NOE_REG_PPE_BASE + 0x300)
#define PPE_MCAST_H_F                           (NOE_REG_PPE_BASE + 0x304)
#define PPE_MTU_DRP                             (NOE_REG_PPE_BASE + 0x308)
#define PPE_MTU_VLYR_0                          (NOE_REG_PPE_BASE + 0x30C)
#define PPE_MTU_VLYR_1                          (NOE_REG_PPE_BASE + 0x310)
#define PPE_MTU_VLYR_2                          (NOE_REG_PPE_BASE + 0x314)
#define PPE_VPM_TPID                            (NOE_REG_PPE_BASE + 0x318)
#define CAH_CTRL                                (NOE_REG_PPE_BASE + 0x320)
#define CAH_TAG_SRH                             (NOE_REG_PPE_BASE + 0x324)
#define CAH_LINE_RW                             (NOE_REG_PPE_BASE + 0x328)
#define CAH_WDATA                               (NOE_REG_PPE_BASE + 0x32C)
#define CAH_RDATA                               (NOE_REG_PPE_BASE + 0x330)

#define PS_CFG                                  (NOE_REG_PPE_BASE + 0x400)
#define PS_FBC                                  (NOE_REG_PPE_BASE + 0x404)
#define PS_TB_BASE                              (NOE_REG_PPE_BASE + 0x408)
#define PS_TME_SMP                              (NOE_REG_PPE_BASE + 0x40C)

#define MIB_CFG                                 (NOE_REG_PPE_BASE + 0x334)
#define MIB_TB_BASE                             (NOE_REG_PPE_BASE + 0x338)
#define MIB_SER_CR                              (NOE_REG_PPE_BASE + 0x33C)
#define MIB_SER_R0                              (NOE_REG_PPE_BASE + 0x340)
#define MIB_SER_R1                              (NOE_REG_PPE_BASE + 0x344)
#define MIB_SER_R2                              (NOE_REG_PPE_BASE + 0x348)
#define MIB_CAH_CTRL                            (NOE_REG_PPE_BASE + 0x350)

//-------------------------------------------------------------------------------------------------
//  ETH_MAC_BASE Register Detail
//-------------------------------------------------------------------------------------------------

#define PHY_CONTROL_0       0x0004


/* SW_INT_STATUS */
#define ESW_PHY_POLLING     (NOE_REG_ETH_MAC_BASE + 0x0000)
#define MDIO_PHY_CONTROL_0  (NOE_REG_ETH_MAC_BASE + PHY_CONTROL_0)
#define NOE_MAC_P1_MCR      (NOE_REG_ETH_MAC_BASE + 0x0100)
#define NOE_MAC_P1_SR       (NOE_REG_ETH_MAC_BASE + 0x0108)
#define MAC1_WOL            (NOE_REG_ETH_MAC_BASE + 0x0110)
    #define WOL_INT_CLR     BIT(17)
    #define WOL_INT_EN      BIT(1)
    #define WOL_EN          BIT(0)
#define NOE_MAC_P2_MCR      (NOE_REG_ETH_MAC_BASE + 0x0200)
#define NOE_MAC_P2_SR       (NOE_REG_ETH_MAC_BASE + 0x0208)

//-------------------------------------------------------------------------------------------------
//  ETH_SW_BASE Register Detail
//-------------------------------------------------------------------------------------------------






#define RSTCTL_FE_RST       BIT(6)
#define RSTCTL_SYS_RST      BIT(0)
#define RSTSTAT_SWSYSRST    BIT(2)
#define RLS_COHERENT        BIT(29)
#define RLS_DLY_INT         BIT(28)
#define RLS_DONE_INT        BIT(0)

#define FE_INT_ALL          (TX_DONE_INT3 | TX_DONE_INT2 | TX_DONE_INT1 | TX_DONE_INT0 | RX_DONE_INT0 | RX_DONE_INT1 | RX_DONE_INT2 | RX_DONE_INT3)
#define QFE_INT_ALL         (RLS_DONE_INT | RX_DONE_INT0 | RX_DONE_INT1 | RX_DONE_INT2 | RX_DONE_INT3)
#define QFE_INT_DLY_INIT    (RLS_DLY_INT | RX_DLY_INT)
#define RX_INT_ALL          (RX_DONE_INT0 | RX_DONE_INT1 | RX_DONE_INT2 | RX_DONE_INT3 | RX_DLY_INT)
#define TX_INT_ALL          (TX_DONE_INT0 | TX_DLY_INT)
#define RX_INT_DONE         (RX_DONE_INT0 | RX_DONE_INT1 | RX_DONE_INT2 | RX_DONE_INT3)
#define TX_INT_DONE         (TX_DONE_INT3 | TX_DONE_INT2 | TX_DONE_INT1 | TX_DONE_INT0)



#define P5_LINK_CH          BIT(5)
#define P4_LINK_CH          BIT(4)
#define P3_LINK_CH          BIT(3)
#define P2_LINK_CH          BIT(2)
#define P1_LINK_CH          BIT(1)
#define P0_LINK_CH          BIT(0)

#define RX_BUF_ALLOC_SIZE   2000
#define FASTPATH_HEADROOM   64

#define ETHER_BUFFER_ALIGN  32  /* /// Align on a cache line */

#define ETHER_ALIGNED_RX_SKB_ADDR(addr) ((((unsigned long)(addr) + ETHER_BUFFER_ALIGN - 1) & ~(ETHER_BUFFER_ALIGN - 1)) - (unsigned long)(addr))


#define MAX_PSEUDO_ENTRY               1






#define DELAY_INT_INIT      0x8F0FBF1F //0x8F0FBF0F
#define FE_INT_DLY_INIT     (TX_DLY_INT | RX_DLY_INT)



/* LRO RX ring mode */
#define PDMA_RX_NORMAL_MODE         (0x0)
#define PDMA_RX_PSE_MODE            (0x1)
#define PDMA_RX_FORCE_PORT          (0x2)
#define PDMA_RX_AUTO_LEARN          (0x3)

#define ADMA_RX_RING0   (0)
#define ADMA_RX_RING1   (1)
#define ADMA_RX_RING2   (2)
#define ADMA_RX_RING3   (3)

#define ADMA_RX_LEN0_MASK   (0x3fff)
#define ADMA_RX_LEN1_MASK   (0x3)

#define SET_ADMA_RX_LEN0(x)    ((x) & ADMA_RX_LEN0_MASK)
#define SET_ADMA_RX_LEN1(x)    ((x) & ADMA_RX_LEN1_MASK)

#define PQ0 0
#define PQ1 1
#define PQ2 15
#define PQ3 16
#define PQ4 30
#define PQ5 31
#define PQ6 43
#define PQ7 63


#define VQ_NUM0 128
#define VQ_NUM1 128
#define VQ_NUM2 128
#define VQ_NUM3 128
#define VQ_NUM4 128
#define VQ_NUM5 128
#define VQ_NUM6 128
#define VQ_NUM7 128
#define VQTX_NUM_0  (3 << 0)
#define VQTX_NUM_1  (3 << 4)
#define VQTX_NUM_2  (3 << 8)
#define VQTX_NUM_3  (3 << 12)
#define VQTX_NUM_4  (3 << 16)
#define VQTX_NUM_5  (3 << 20)
#define VQTX_NUM_6  (3 << 24)
#define VQTX_NUM_7  (3 << 28)


#define VQTX_MIB_EN BIT(17)





#define GDM_PRI_PID              (0)
#define GDM_PRI_VLAN_PID         (1)
#define GDM_PRI_ACK_PID          (2)
#define GDM_PRI_VLAN_ACK_PID     (3)
#define GDM_PRI_ACK_VLAN_PID     (4)

#define SET_GDM_VLAN_PRI_RXID_SEL(x, y) \
{ \
unsigned int reg_val = sys_reg_read(FE_GDM_RXID1); \
reg_val &= ~(0x03 << (((x) << 1) + 16));   \
reg_val |= ((y) & 0x3) << (((x) << 1) + 16);  \
sys_reg_write(FE_GDM_RXID1, reg_val); \
}

#define SET_GDM_TCP_ACK_RXID_SEL(x) \
{ \
unsigned int reg_val = sys_reg_read(FE_GDM_RXID1); \
reg_val &= ~(GDM_TCP_ACK_RXID_SEL);   \
reg_val |= ((x) & 0x3) << 4;  \
sys_reg_write(FE_GDM_RXID1, reg_val); \
}

#define SET_GDM_TCP_ACK_WZPC(x) \
{ \
unsigned int reg_val = sys_reg_read(FE_GDM_RXID1); \
reg_val &= ~(GDM_TCP_ACK_WZPC);   \
reg_val |= ((x) & 0x1) << 3;  \
sys_reg_write(FE_GDM_RXID1, reg_val); \
}

#define SET_GDM_RXID_PRI_SEL(x) \
{ \
unsigned int reg_val = sys_reg_read(FE_GDM_RXID1); \
reg_val &= ~(GDM_RXID_PRI_SEL);   \
reg_val |= (x) & 0x7;  \
sys_reg_write(FE_GDM_RXID1, reg_val); \
}

#define GDM_STAG_RXID_SEL(x, y) \
{ \
unsigned int reg_val = sys_reg_read(FE_GDM_RXID2); \
reg_val &= ~(0x03 << (((x) << 1) + 16));   \
reg_val |= ((y) & 0x3) << (((x) << 1) + 16);  \
sys_reg_write(FE_GDM_RXID2, reg_val); \
}

#define SET_GDM_PID2_RXID_SEL(x) \
{ \
unsigned int reg_val = sys_reg_read(FE_GDM_RXID2); \
reg_val &= ~(GDM_PID2_RXID_SEL);   \
reg_val |= ((x) & 0x3) << 2;  \
sys_reg_write(FE_GDM_RXID2, reg_val); \
}

#define SET_GDM_PID1_RXID_SEL(x) \
{ \
unsigned int reg_val = sys_reg_read(FE_GDM_RXID2); \
reg_val &= ~(GDM_PID1_RXID_SEL);   \
reg_val |= ((x) & 0x3);  \
sys_reg_write(FE_GDM_RXID2, reg_val); \
}



#define PORT0_PKCOUNT       (0xb01100e8)
#define PORT1_PKCOUNT       (0xb01100ec)
#define PORT2_PKCOUNT       (0xb01100f0)
#define PORT3_PKCOUNT       (0xb01100f4)
#define PORT4_PKCOUNT       (0xb01100f8)
#define PORT5_PKCOUNT       (0xb01100fc)


/* ====================================== */
#define GDM1_DISPAD       BIT(18)
#define GDM1_DISCRC       BIT(17)

#define GDM1_ICS_EN         (0x1 << 22)
#define GDM1_TCS_EN         (0x1 << 21)
#define GDM1_UCS_EN         (0x1 << 20)
#define GDM1_JMB_EN         (0x1 << 19)
#define GDM1_STRPCRC        (0x1 << 16)

#if defined (CONFIG_NOE_QDMATX_QDMARX)
#define GDM1_UFRC_P_CPU     (5 << 12)
#else
#define GDM1_UFRC_P_CPU     (0 << 12)
#endif  /* CONFIG_NOE_QDMATX_QDMARX */
#define GDM1_UFRC_P_PPE     (4 << 12)

#if defined (CONFIG_NOE_QDMATX_QDMARX)
#define GDM1_BFRC_P_CPU     (5 << 8)
#else
#define GDM1_BFRC_P_CPU     (0 << 8)
#endif /* CONFIG_NOE_QDMATX_QDMARX */
#define GDM1_BFRC_P_PPE     (4 << 8)

#if defined (CONFIG_NOE_QDMATX_QDMARX)
#define GDM1_MFRC_P_CPU     (5 << 4)
#else
#define GDM1_MFRC_P_CPU     (0 << 4)
#endif /* CONFIG_NOE_QDMATX_QDMARX */
#define GDM1_MFRC_P_PPE     (4 << 4)

#if defined (CONFIG_NOE_QDMATX_QDMARX)
#define GDM1_OFRC_P_CPU     (5 << 0)
#else
#define GDM1_OFRC_P_CPU     (0 << 0)
#endif /* CONFIG_NOE_QDMATX_QDMARX */
#define GDM1_OFRC_P_PPE     (4 << 0)

#define ICS_GEN_EN          (0 << 2)
#define UCS_GEN_EN          (0 << 1)
#define TCS_GEN_EN          (0 << 0)

#define MDIO_CFG_GP1_FC_TX  BIT(11)
#define MDIO_CFG_GP1_FC_RX  BIT(10)

/* ====================================== */
/* ====================================== */
#define GP1_LNK_DWN     BIT(9)
#define GP1_AN_FAIL     BIT(8)
/* ====================================== */
/* ====================================== */
#define PSE_RESET       BIT(0)
/* ====================================== */
#define PST_DRX_IDX3       BIT(19)
#define PST_DRX_IDX2       BIT(18)
#define PST_DRX_IDX1       BIT(17)
#define PST_DRX_IDX0       BIT(16)
#define PST_DTX_IDX3       BIT(3)
#define PST_DTX_IDX2       BIT(2)
#define PST_DTX_IDX1       BIT(1)
#define PST_DTX_IDX0       BIT(0)

#define RX_2B_OFFSET      BIT(31)
#define CSR_CLKGATE_BYP   BIT(30)
#define MULTI_EN      BIT(10)
#define DESC_32B_EN   BIT(8)
#define TX_WB_DDONE       BIT(6)
#define RX_DMA_BUSY       BIT(3)
#define TX_DMA_BUSY       BIT(1)
#define RX_DMA_EN         BIT(2)
#define TX_DMA_EN         BIT(0)

#define PDMA_BT_SIZE_4DWORDS        (0 << 4)
#define PDMA_BT_SIZE_8DWORDS        (1 << 4)
#define PDMA_BT_SIZE_16DWORDS       (2 << 4)
#define PDMA_BT_SIZE_32DWORDS       (3 << 4)

#define ADMA_RX_BT_SIZE_4DWORDS     (0 << 11)
#define ADMA_RX_BT_SIZE_8DWORDS     (1 << 11)
#define ADMA_RX_BT_SIZE_16DWORDS    (2 << 11)
#define ADMA_RX_BT_SIZE_32DWORDS    (3 << 11)

/* Register bits.
 */

#define MACCFG_RXEN BIT(2)
#define MACCFG_TXEN BIT(3)
#define MACCFG_PROMISC  BIT(18)
#define MACCFG_RXMCAST  BIT(19)
#define MACCFG_FDUPLEX  BIT(20)
#define MACCFG_PORTSEL  BIT(27)
#define MACCFG_HBEATDIS BIT(28)

#define DMACTL_SR   BIT(1)  /* Start/Stop Receive */
#define DMACTL_ST   BIT(13) /* Start/Stop Transmission Command */

#define DMACFG_SWR  BIT(0)  /* Software Reset */
#define DMACFG_BURST32      (32 << 8)

#define DMASTAT_TS      0x00700000  /* Transmit Process State */
#define DMASTAT_RS      0x000e0000  /* Receive Process State */

#define MACCFG_INIT     0   /* (MACCFG_FDUPLEX) // | MACCFG_PORTSEL) */

/* Descriptor bits.
 */
#define R_OWN       0x80000000  /* Own Bit */
#define RD_RER      0x02000000  /* Receive End Of Ring */
#define RD_LS       0x00000100  /* Last Descriptor */
#define RD_ES       0x00008000  /* Error Summary */
#define RD_CHAIN    0x01000000  /* Chained */

/* Word 0 */
#define T_OWN       0x80000000  /* Own Bit */
#define TD_ES       0x00008000  /* Error Summary */

/* Word 1 */
#define TD_LS       0x40000000  /* Last Segment */
#define TD_FS       0x20000000  /* First Segment */
#define TD_TER      0x08000000  /* Transmit End Of Ring */
#define TD_CHAIN    0x01000000  /* Chained */

#define TD_SET      0x08000000  /* Setup Packet */

#define POLL_DEMAND 1

#define RSTCTL  (0x34)
#define RSTCTL_RSTENET1 BIT(19)
#define RSTCTL_RSTENET2 BIT(20)

//#define INIT_VALUE_OF_RT2883_PSE_FQ_CFG       0xff908000
#define INIT_VALUE_OF_PSE_FQFC_CFG      0x80504000
#define INIT_VALUE_OF_FORCE_100_FD      0x1001BC01
#define INIT_VALUE_OF_FORCE_1000_FD     0x1F01DC01





#define PHY_ENABLE_AUTO_NEGO    0x1000
#define PHY_RESTART_AUTO_NEGO   0x0200

/* PHY_STAT_REG = 1; */
#define PHY_AUTO_NEGO_COMP  0x0020
#define PHY_LINK_STATUS     0x0004

/* PHY_AUTO_NEGO_REG = 4; */
#define PHY_CAP_10_HALF     0x0020
#define PHY_CAP_10_FULL     0x0040
#define PHY_CAP_100_HALF    0x0080
#define PHY_CAP_100_FULL    0x0100






#define DMA_GLO_CFG PDMA_GLO_CFG

#if defined(CONFIG_NOE_QDMATX_QDMARX)
#define GDMA1_FWD_PORT 0x5555
#define GDMA2_FWD_PORT 0x5555
#elif defined(CONFIG_NOE_PDMATX_QDMARX)
#define GDMA1_FWD_PORT 0x5555
#define GDMA2_FWD_PORT 0x5555
#else
#define GDMA1_FWD_PORT 0x0000
#define GDMA2_FWD_PORT 0x0000
#endif

#if defined(CONFIG_NOE_QDMATX_QDMARX)
#define CONFIG_NOE_REG_RX_CALC_IDX0 QRX_CRX_IDX_0
#define CONFIG_NOE_REG_RX_CALC_IDX1 QRX_CRX_IDX_1
#elif defined(CONFIG_NOE_PDMATX_QDMARX)
#define CONFIG_NOE_REG_RX_CALC_IDX0 QRX_CRX_IDX_0
#define CONFIG_NOE_REG_RX_CALC_IDX1 QRX_CRX_IDX_1
#else
#define CONFIG_NOE_REG_RX_CALC_IDX0 RX_CALC_IDX0
#define CONFIG_NOE_REG_RX_CALC_IDX1 RX_CALC_IDX1
#endif
#define CONFIG_NOE_REG_RX_CALC_IDX2 RX_CALC_IDX2
#define CONFIG_NOE_REG_RX_CALC_IDX3 RX_CALC_IDX3
#define CONFIG_NOE_REG_FE_INT_STATUS FE_INT_STATUS
#define CONFIG_NOE_REG_FE_INT_ALL FE_INT_ALL
#define CONFIG_NOE_REG_FE_INT_ENABLE FE_INT_ENABLE
#define CONFIG_NOE_REG_FE_INT_DLY_INIT FE_INT_DLY_INIT
#define CONFIG_NOE_REG_FE_INT_SETTING (RX_DONE_INT0 | RX_DONE_INT1 | TX_DONE_INT0 | TX_DONE_INT1 | TX_DONE_INT2 | TX_DONE_INT3)
#define QFE_INT_SETTING (RX_DONE_INT0 | RX_DONE_INT1 | TX_DONE_INT0 | TX_DONE_INT1 | TX_DONE_INT2 | TX_DONE_INT3)
#define CONFIG_NOE_REG_TX_DLY_INT TX_DLY_INT
#define CONFIG_NOE_REG_TX_DONE_INT0 TX_DONE_INT0
#define CONFIG_NOE_REG_DLY_INT_CFG DLY_INT_CFG


#define QUEUE_OFFSET 0x10


#define IP_PROTO_GRE  47
#define IP_PROTO_TCP  6
#define IP_PROTO_UDP  17
#define IP_PROTO_IPV6 41



#endif  /* MHAL_NOE_REG_H */
