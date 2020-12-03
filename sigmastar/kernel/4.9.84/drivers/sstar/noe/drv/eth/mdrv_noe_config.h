/*
* mdrv_noe_config.h- Sigmastar
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
/// @file   MDrv_NOE_CONFIG.H
/// @brief  NOE Driver Interface
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _MDRV_NOE_CONFIG_H_
#define _MDRV_NOE_CONFIG_H_


// -----------------------------------------------------------------------------
// define start
// -----------------------------------------------------------------------------
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0))
#define NETDEV_LRO_SUPPORTED (1)
#else
#define NETDEV_LRO_SUPPORTED (0)
#endif


#define CONFIG_NOE_PM_SUPPORTED   (0)
//-------------------------------------------------------------------------------------------------
//  CONFIG
//-------------------------------------------------------------------------------------------------


/* compile flag for features */
#define DELAY_INT
#define CONFIG_NOE_RW_PDMAPTR_FROM_VAR
/*#define CONFIG_QDMA_QOS_WEB*/
#define CONFIG_QDMA_QOS_MARK
/*#define CONFIG_NOE_NAPI_TX_RX*/
/*#define CONFIG_NOE_NAPI_RX_ONLY*/

/***** FEATURE *****/
#ifdef  DELAY_INT
#define FE_DLY_INT  BIT(0)
#else
#define FE_DLY_INT  (0)
#endif /* DELAY_INT */
#ifdef  CONFIG_NOE_HW_LRO
#define FE_HW_LRO   BIT(1)
#else
#define FE_HW_LRO   (0)
#endif /* CONFIG_NOE_HW_LRO */
#ifdef  CONFIG_NOE_HW_LRO_FORCE
#define FE_HW_LRO_FPORT BIT(2)
#else
#define FE_HW_LRO_FPORT (0)
#endif /* CONFIG_NOE_HW_LRO_FORCE */
#ifdef  CONFIG_NOE_LRO
#define FE_SW_LRO   BIT(3)
#else
#define FE_SW_LRO   (0)
#endif /* CONFIG_NOE_LRO */
#ifdef  CONFIG_NOE_QDMA
#define FE_QDMA     BIT(4)
#else
#define FE_QDMA     (0)
#endif/* CONFIG_NOE_QDMA */
#ifdef  CONFIG_NOE_NAPI
#define FE_INT_NAPI BIT(5)
#else
#define FE_INT_NAPI (0)
#endif /* CONFIG_NOE_NAPI */
#ifdef  CONFIG_NOE_NETWORK_WORKQUEUE_BH
#define FE_INT_WORKQ    BIT(6)
#else
#define FE_INT_WORKQ    (0)
#endif /* CONFIG_NOE_NETWORK_WORKQUEUE_BH */
#ifdef  CONFIG_NOE_NETWORK_TASKLET_BH
#define FE_INT_TASKLET  BIT(7)
#else
#define FE_INT_TASKLET  (0)
#endif /* CONFIG_NOE_NETWORK_TASKLET_BH */
#ifdef  CONFIG_NOE_TX_RX_INT_SEPARATION
#define FE_IRQ_SEPARATE BIT(8)
#else
#define FE_IRQ_SEPARATE (0)
#endif /* CONFIG_NOE_TX_RX_INT_SEPARATION */
#ifdef  CONFIG_NOE_GMAC2
#define FE_GE2_SUPPORT  BIT(9)
#else
#define FE_GE2_SUPPORT  (0)
#endif /* CONFIG_NOE_GMAC2 */
#ifdef  CONFIG_NOE_ETHTOOL
#define FE_ETHTOOL  BIT(10)
#else
#define FE_ETHTOOL  (0)
#endif /* CONFIG_NOE_ETHTOOL */
#ifdef  CONFIG_NOE_CHECKSUM_OFFLOAD
#define FE_CSUM_OFFLOAD BIT(11)
#else
#define FE_CSUM_OFFLOAD (0)
#endif /* CONFIG_NOE_CHECKSUM_OFFLOAD */
#ifdef  CONFIG_NOE_TSO
#define FE_TSO      BIT(12)
#else
#define FE_TSO      (0)
#endif /* CONFIG_NOE_TSO */
#ifdef  CONFIG_NOE_TSOV6
#define FE_TSO_V6   BIT(13)
#else
#define FE_TSO_V6   (0)
#endif /* CONFIG_NOE_TSOV6 */
#ifdef  CONFIG_NOE_HW_VLAN_TX
#define FE_HW_VLAN_TX   BIT(14)
#else
#define FE_HW_VLAN_TX   (0)
#endif /* CONFIG_NOE_HW_VLAN_TX */
#ifdef  CONFIG_NOE_HW_VLAN_RX
#define FE_HW_VLAN_RX   BIT(15)
#else
#define FE_HW_VLAN_RX   (0)
#endif /* CONFIG_NOE_HW_VLAN_RX */
#ifdef  CONFIG_NOE_QDMA
#define FE_QDMA_TX  BIT(16)
#else
#define FE_QDMA_TX  (0)
#endif /* CONFIG_NOE_QDMA */
#ifdef  CONFIG_NOE_QDMATX_QDMARX
#define FE_QDMA_RX  BIT(17)
#else
#define FE_QDMA_RX  (0)
#endif /* CONFIG_NOE_QDMATX_QDMARX */
#ifdef  CONFIG_HW_SFQ
#define FE_HW_SFQ   BIT(18)
#else
#define FE_HW_SFQ   (0)
#endif /* CONFIG_HW_SFQ */
#ifdef  CONFIG_NOE_HW_IOCOHERENT
#define FE_HW_IOCOHERENT BIT(19)
#else
#define FE_HW_IOCOHERENT (0)
#endif /* CONFIG_NOE_HW_IOCOHERENT */
#if defined(CONFIG_MSTAR_ARM_BD_FPGA) || defined(CONFIG_NOE_FPGA_VERIFY)
#define FE_FPGA_MODE    BIT(20)
#else
#define FE_FPGA_MODE    (0)
#endif /* CONFIG_MSTAR_ARM_BD_FPGA */
#ifdef  CONFIG_NOE_HW_LRO
#define FE_HW_LRO_DBG   BIT(21)
#else
#define FE_HW_LRO_DBG   0
#endif /* CONFIG_NOE_HW_LRO */
#ifdef CONFIG_NOE_INT_DBG
#define FE_NOE_INT_DBG  BIT(22)
#else
#define FE_NOE_INT_DBG  (0)
#endif/* CONFIG_NOE_INT_DBG */
#ifdef CONFIG_USER_SNMPD
#define USER_SNMPD  BIT(23)
#else
#define USER_SNMPD  (0)
#endif /* CONFIG_USER_SNMPD */
#ifdef CONFIG_TASKLET_WORKQUEUE_SW
#define TASKLET_WORKQUEUE_SW    BIT(24)
#else
#define TASKLET_WORKQUEUE_SW    (0)
#endif /* CONFIG_TASKLET_WORKQUEUE_SW */
#if defined(CONFIG_NOE_HW_NAT) || defined(CONFIG_NOE_HW_NAT_MODULE) || defined(CONFIG_NOE_NAT_HW)
#define FE_HW_NAT   BIT(25)
#else
#define FE_HW_NAT   (0)
#endif /* CONFIG_NOE_HW_NAT ...*/
#ifdef  CONFIG_NOE_NAPI_TX_RX
#define FE_INT_NAPI_TX_RX   BIT(26)
#else
#define FE_INT_NAPI_TX_RX   (0)
#endif /* CONFIG_NOE_NAPI_TX_RX */
#ifdef  CONFIG_QDMA_MQ
#define QDMA_MQ       BIT(27)
#else
#define QDMA_MQ       (0)
#endif /* CONFIG_QDMA_MQ */
#ifdef  CONFIG_NOE_NAPI_RX_ONLY
#define FE_INT_NAPI_RX_ONLY BIT(28)
#else
#define FE_INT_NAPI_RX_ONLY (0)
#endif
#ifdef  CONFIG_QDMA_SUPPORT_QOS
#define FE_QDMA_FQOS    BIT(29)
#else
#define FE_QDMA_FQOS    (0)
#endif

#ifdef  CONFIG_QDMA_QOS_WEB
#define QDMA_QOS_WEB    BIT(30)
#else
#define QDMA_QOS_WEB    (0)
#endif

#ifdef  CONFIG_QDMA_QOS_MARK
#define QDMA_QOS_MARK   BIT(31)
#else
#define QDMA_QOS_MARK   (0)
#endif


/***** ARCHITECTURE *****/
#ifdef CONFIG_NOE_GMAC2
#define GMAC2                       BIT(0)
#else
#define GMAC2 (0)
#endif /* CONFIG_NOE_GMAC2 */
#ifdef CONFIG_LAN_WAN_SUPPORT
#define LAN_WAN_SUPPORT             BIT(1)
#else
#define LAN_WAN_SUPPORT (0)
#endif /* CONFIG_LAN_WAN_SUPPORT */
#if  (defined(CONFIG_GE1_RGMII_AN) || defined(CONFIG_GE1_GMII_AN_INTPHY) || defined(CONFIG_GE1_GMII_AN_EXTPHY))
#define    GE1_RGMII_AN             BIT(2)
#else
#define    GE1_RGMII_AN    (0)
#endif /* CONFIG_GE1_RGMII_AN */
#ifdef CONFIG_GE1_RGMII_ONE_EPHY
#define    GE1_RGMII_ONE_EPHY       BIT(3)
#else
#define    GE1_RGMII_ONE_EPHY    (0)
#endif /* CONFIG_GE1_RGMII_ONE_EPHY */
#if     (defined(CONFIG_GE2_RGMII_AN) || defined(CONFIG_GE2_GMII_AN_EXTPHY))
#define    GE2_RGMII_AN             BIT(4)
#else
#define    GE2_RGMII_AN    (0)
#endif /* CONFIG_GE2_RGMII_AN */
#ifdef CONFIG_GE2_INTERNAL_GPHY
#define    GE2_INTERNAL_GPHY        BIT(5)
#else
#define    GE2_INTERNAL_GPHY    (0)
#endif /* CONFIG_GE2_INTERNAL_GPHY */



#ifndef CONFIG_MAC_TO_GIGAPHY_MODE_ADDR
#define CONFIG_MAC_TO_GIGAPHY_MODE_ADDR (0)
#endif /* CONFIG_MAC_TO_GIGAPHY_MODE_ADDR */
#ifndef CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2
#define CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2 (0)
#endif /* CONFIG_MAC_TO_GIGAPHY_MODE_ADDR2 */

#if ((!defined(CONFIG_GE1_RGMII_NONE)) && (!defined(CONFIG_NOE_GMAC2)))
    #define NOE_CFG_PIN_MUX_SEL (E_NOE_SEL_PIN_MUX_GE1_TO_CHIPTOP)
#elif (defined(CONFIG_GE1_RGMII_NONE) && (defined(CONFIG_NOE_GMAC2)))
    #define NOE_CFG_PIN_MUX_SEL (E_NOE_SEL_PIN_MUX_GE2_TO_CHIPTOP)
#elif ((!defined(CONFIG_GE1_RGMII_NONE)) && (defined(CONFIG_NOE_GMAC2)))
    #define NOE_CFG_PIN_MUX_SEL (E_NOE_SEL_PIN_MUX_GE1_TO_CHIPTOP_GE2_TO_CHIPTOP)
#endif



//-------------------------------------------------------------------------------------------------
//  COMPATIBILITY
//-------------------------------------------------------------------------------------------------

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
    #define skb_vlan_tag_present(__skb)     vlan_tx_tag_present(__skb)
    #define skb_vlan_tag_get(__skb)         vlan_tx_tag_get(__skb)
    #define skb_vlan_tag_get_id(__skb)      vlan_tx_tag_get_id(__skb)
    #define skb_free_frag(new_data)         dev_kfree_skb_any(new_data)
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0)
    #define ether_addr_copy(dst, src)       memcpy(dst, src, ETH_ALEN)
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0) */
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0) */



#if 1
    #define NOE_SETUP_COHERENT_DMA_OPS(dev_ptr)
    #define NOE_SETUP_NONCOHERENT_DMA_OPS(dev_ptr)
#elif defined(CONFIG_ARM)
    #define NOE_SETUP_COHERENT_DMA_OPS(dev_ptr) set_dma_ops(dev_ptr, &coherent_swiotlb_dma_ops)
    #define NOE_SETUP_NONCOHERENT_DMA_OPS(dev_ptr) set_dma_ops(dev_ptr, &noncoherent_swiotlb_dma_ops);
#elif defined(CONFIG_ARM64)
    #define NOE_SETUP_COHERENT_DMA_OPS(dev_ptr) arch_setup_dma_ops(dev_ptr, 0, 0, NULL, TRUE)
    #define NOE_SETUP_NONCOHERENT_DMA_OPS(dev_ptr) arch_setup_dma_ops(dev_ptr, 0, 0, NULL, FALSE)
#endif /* CONFIG_ARM64 */


#if 0/// defined(CONFIG_MIPS) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
    #define NOE_MAP_VA_TO_PA(dev, va, size, dir)    MHAL_NOE_MAP_VA_TO_PA(dev, va, size, dir)
    #define NOE_MAP_IS_ERROR_PA(dev, pa)            (0)
#else
    #define NOE_MAP_VA_TO_PA(dev, va, size, dir)    dma_map_single(dev, va, size, dir)
    #define NOE_MAP_IS_ERROR_PA(dev, pa)            dma_mapping_error(dev, pa)
#endif


#if defined(CONFIG_MIPS) || defined(CONFIG_ARM)
#define NOE_DEV_DMA_MASK                            (0xFFFFFFFF)
#define NOE_DEV_COHERENT_DMA_MASK                   (0xFFFFFFFF)
#elif defined(CONFIG_ARM64)
#define NOE_DEV_DMA_MASK                            (0xFFFFFFFF)
#define NOE_DEV_COHERENT_DMA_MASK                   (0xFFFFFFFF)
#else
#define NOE_DEV_DMA_MASK                            (0xFFFFFFFF)
#define NOE_DEV_COHERENT_DMA_MASK                   (0xFFFFFFFF)
#endif

// -----------------------------------------------------------------------------
// DBG
// -----------------------------------------------------------------------------
#define MAX_PSEUDO_ENTRY 1
#define NOE_DBG_PRINT

//-------------------------------------------------------------------------------------------------
//  MACRO
//-------------------------------------------------------------------------------------------------

#define MDrv_NOE_CONFIG_Set_Features(end_device)    \
{                   \
end_device->features = 0;       \
end_device->features |= FE_DLY_INT; \
end_device->features |= FE_HW_LRO;  \
end_device->features |= FE_HW_LRO_FPORT;\
end_device->features |= FE_HW_LRO_DBG;  \
end_device->features |= FE_SW_LRO;  \
end_device->features |= FE_QDMA;    \
end_device->features |= FE_INT_NAPI;    \
end_device->features |= FE_INT_WORKQ;   \
end_device->features |= FE_INT_TASKLET; \
end_device->features |= FE_IRQ_SEPARATE;\
end_device->features |= FE_GE2_SUPPORT; \
end_device->features |= FE_ETHTOOL; \
end_device->features |= FE_CSUM_OFFLOAD;\
end_device->features |= FE_TSO;     \
end_device->features |= FE_TSO_V6;  \
end_device->features |= FE_HW_VLAN_TX;  \
end_device->features |= FE_HW_VLAN_RX;  \
end_device->features |= FE_QDMA_TX; \
end_device->features |= FE_QDMA_RX; \
end_device->features |= FE_HW_SFQ;  \
end_device->features |= FE_HW_IOCOHERENT; \
end_device->features |= FE_FPGA_MODE;   \
end_device->features |= FE_HW_NAT;  \
end_device->features |= FE_INT_NAPI_TX_RX; \
end_device->features |= FE_INT_NAPI_RX_ONLY; \
end_device->features |= FE_QDMA_FQOS;   \
end_device->features |= QDMA_QOS_WEB;   \
end_device->features |= QDMA_QOS_MARK;  \
}

#define MDrv_NOE_CONFIG_Set_Architecture(end_device)              \
{                                                       \
end_device->architecture = 0;                           \
end_device->architecture |= GMAC2;                      \
end_device->architecture |= LAN_WAN_SUPPORT;            \
end_device->architecture |= GE1_RGMII_AN;               \
end_device->architecture |= GE1_RGMII_ONE_EPHY;         \
end_device->architecture |= GE2_RGMII_AN;               \
end_device->architecture |= GE2_INTERNAL_GPHY;          \
}

#endif /* _MDRV_NOE_CONFIG_H_ */

