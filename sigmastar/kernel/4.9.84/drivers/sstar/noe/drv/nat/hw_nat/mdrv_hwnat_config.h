/*
* mdrv_hwnat_config.h- Sigmastar
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
// (¡§MStar Confidential Information¡¨) by the recipien
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   MDRV_NOE_NAT_CONFIG.h
/// @brief  NOE NAT Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_NOE_HWNAT_CONFIG_H_
#define _MDRV_NOE_HWNAT_CONFIG_H_
//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  CONFIG
//-------------------------------------------------------------------------------------------------
#define HWNAT_BIT(x)   (((uint64_t) 1) << x)

#define HNAT_V2                                     HWNAT_BIT(0)
#ifdef CONFIG_NOE_HW_NAT_PACKET_SAMPLING
#define PACKET_SAMPLING                             HWNAT_BIT(1)
#else
#define PACKET_SAMPLING (0)
#endif /*CONFIG_NOE_HW_NAT_PACKET_SAMPLING */
#ifdef CONFIG_PPE_MIB
#define PPE_MIB                                     HWNAT_BIT(2)
#else
#define PPE_MIB (0)
#endif /* CONFIG_PPE_MIB */
#define NAT_IPV6                                    HWNAT_BIT(3)
#ifdef CONFIG_NOE_HW_NAT_PREBIND
#define NAT_PREBIND                                 HWNAT_BIT(4)
#else
#define NAT_PREBIND 0
#endif /* CONFIG_NOE_HW_NAT_PREBINDi */
#ifdef CONFIG_NOE_HW_NAT_ACCNT_MAINTAINER
#define ACCNT_MAINTAINER                            HWNAT_BIT(5)
#else
#define ACCNT_MAINTAINER 0
#endif /* CONFIG_NOE_HW_NAT_ACCNT_MAINTAINER */
#ifdef CONFIG_PPE_MCAST
#define PPE_MCAST                                   HWNAT_BIT(6)
#else
#define PPE_MCAST        0
#endif /*CONFIG_PPE_MCAST*/
#ifdef CONFIG_NOE_HW_NAT_PPTP_L2TP
#define PPTP_L2TP                                   HWNAT_BIT(7)
#else
#define PPTP_L2TP    0
#endif /*CONFIG_NOE_HW_NAT_PPTP_L2TP*/
#ifdef CONFIG_QDMA_SUPPORT_QOS
#define QDMA_SUPPORT_QOS                            HWNAT_BIT(8)
#else
#define QDMA_SUPPORT_QOS 0
#endif /*CONFIG_QDMA_SUPPORT_QOS*/
#ifdef CONFIG_WAN_TO_WLAN_SUPPORT_QOS
#define WAN_TO_WLAN_SUPPORT_QOS                     HWNAT_BIT(9)
#else
#define WAN_TO_WLAN_SUPPORT_QOS 0
#endif /*CONFIG_WAN_TO_WLAN_SUPPORT_QOS*/
#ifdef CONFIG_SUPPORT_WLAN_OPTIMIZE
#define WLAN_OPTIMIZE                               HWNAT_BIT(10)
#else
#define WLAN_OPTIMIZE 0
#endif /* CONFIG_SUPPORT_WLAN_OPTIMIZE_RX */
#ifdef CONFIG_NOE_HW_NAT_WIFI
#define HW_NAT_WIFI                                 HWNAT_BIT(11)
#else
#define HW_NAT_WIFI 0
#endif /*CONFIG_NOE_HW_NAT_WIFI*/
#ifdef CONFIG_NOE_HW_NAT_WIFI_NEW_ARCH
#define HW_NAT_WIFI_NEW_ARCH                        HWNAT_BIT(12)
#else
#define HW_NAT_WIFI_NEW_ARCH 0
#endif /*CONFIG_NOE_HW_NAT_WIFI_NEW_ARCH*/
#ifdef CONFIG_NOE_QDMA
#define HW_NAT_QDMA                                 HWNAT_BIT(13)
#else
#define HW_NAT_QDMA 0
#endif /*CONFIG_NOE_QDMA*/
#ifdef CONFIG_HW_NAT_NEW_ARCH_WDMA
#define HW_NAT_ARCH_WIFI_WDMA                       HWNAT_BIT(14)
#else
#define HW_NAT_ARCH_WIFI_WDMA 0
#endif /* CONFIG_HW_NAT_NEW_ARCH_WDMA */
#ifdef CONFIG_NOE_QDMATX_QDMARX
#define HW_NAT_QDMATX_QDMARX                        HWNAT_BIT(15)
#else
#define HW_NAT_QDMATX_QDMARX   0
#endif /* CONFIG_NOE_QDMATX_QDMARX */
#ifdef CONFIG_PSEUDO_SUPPORT
#define HW_NAT_PSEUDO_SUPPORT                       HWNAT_BIT(16)
#else
#define HW_NAT_PSEUDO_SUPPORT   0
#endif  /* CONFIG_PSEUDO_SUPPORT */
#ifdef CONFIG_HW_SFQ
#define HW_NAT_HW_SFQ                               HWNAT_BIT(17)
#else
#define HW_NAT_HW_SFQ       0
#endif /* CONFIG_HW_SFQ */
#ifdef CONFIG_APCLI_SUPPORT
#define HW_NAT_APCLI_SUPPORT                        HWNAT_BIT(18)
#else
#define HW_NAT_APCLI_SUPPORT   0
#endif /* CONFIG_APCLI_SUPPORT */
#ifdef CONFIG_WDS_SUPPORT
#define HW_NAT_WDS_SUPPORT                          HWNAT_BIT(19)
#else
#define HW_NAT_WDS_SUPPORT     0
#endif /* CONFIG_WDS_SUPPORT */
#ifdef CONFIG_MBSS_SUPPORT
#define HW_NAT_MBSS_SUPPORT                         HWNAT_BIT(20)
#else
#define HW_NAT_MBSS_SUPPORT    0
#endif /* CONFIG_MBSS_SUPPORT */
#ifdef  CONFIG_NOE_HW_VLAN_TX
#define HW_NAT_VLAN_TX                              HWNAT_BIT(21)
#else
#define HW_NAT_VLAN_TX         0
#endif  /* CONFIG_NOE_HW_VLAN_TX */
#ifdef CONFIG_NOE_HW_NAT_NIC_USB
#define HW_NAT_NIC_USB                              HWNAT_BIT(22)
#else
#define HW_NAT_NIC_USB         0
#endif /* CONFIG_NOE_HW_NAT_NIC_USB */
#ifdef CONFIG_NOE_GMAC2
#define HW_NAT_GMAC2                                HWNAT_BIT(23)
#else
#define HW_NAT_GMAC2           0
#endif /* CONFIG_NOE_GMAC2 */
#ifdef CONFIG_NOE_SPECIAL_TAG
#define HW_NAT_SPECIAL_TAG                          HWNAT_BIT(24)
#else
#define HW_NAT_SPECIAL_TAG     0
#endif /* CONFIG_NOE_SPECIAL_TAG */
#ifdef CONFIG_IMQ
#define HW_NAT_IMQ                                  HWNAT_BIT(25)
#else
#define HW_NAT_IMQ               0
#endif /* CONFIG_IMQ */
#ifdef CONFIG_RTDEV_MII
#define HW_NAT_MII                                  HWNAT_BIT(26)
#else
#define HW_NAT_MII               0
#endif /* CONFIG_RTDEV_MII */
#ifdef CONFIG_RTDEV_PCI
#define HW_NAT_PCI                                  HWNAT_BIT(27)
#else
#define HW_NAT_PCI               0
#endif /* CONFIG_RTDEV_PCI */
#ifdef CONFIG_RTDEV_USB
#define HW_NAT_USB                                  HWNAT_BIT(28)
#else
#define HW_NAT_USB               0
#endif  /* CONFIG_RTDEV_USB */
#ifdef CONFIG_RTDEV
#define HW_NAT_SP_DEV                               HWNAT_BIT(29)
#else
#define HW_NAT_SP_DEV            0
#endif  /* CONFIG_RTDEV */
#ifdef CONFIG_RLT_AP_SUPPORT
#define HW_NAT_AP_SUPPORT                           HWNAT_BIT(30)
#else
#define HW_NAT_AP_SUPPORT        0
#endif /* CONFIG_RLT_AP_SUPPORT */
#ifdef CONFIG_AP_MESH_SUPPORT
#define HW_NAT_AP_MESH_SUPPORT                      HWNAT_BIT(31)
#else
#define HW_NAT_AP_MESH_SUPPORT        0
#endif /* CONFIG_RLT_AP_SUPPORT */
#ifdef CONFIG_AP_MBSS_SUPPORT
#define HW_NAT_AP_MBSS_SUPPORT                      HWNAT_BIT(32)
#else
#define HW_NAT_AP_MBSS_SUPPORT    0
#endif /* CONFIG_AP_MBSS_SUPPORT */
#ifdef CONFIG_NOE_HW_NAT_AUTO_MODE
#define HW_NAT_AUTO_MODE                            HWNAT_BIT(33)
#else
#define HW_NAT_AUTO_MODE    0
#endif /* NOE_HW_NAT_AUTO_MODE */
#ifdef CONFIG_NOE_HW_NAT_SEMI_AUTO_MODE
#define HW_NAT_SEMI_AUTO_MODE                       HWNAT_BIT(34)
#else
#define HW_NAT_SEMI_AUTO_MODE       0
#endif /* CONFIG_HW_NAT_SEMI_AUTO_MODE */
#ifdef CONFIG_NOE_HW_NAT_MANUAL_MODE
#define HW_NAT_MANUAL_MODE                          HWNAT_BIT(35)
#else
#define HW_NAT_MANUAL_MODE           0
#endif /* CONFIG_HW_NAT_MANUAL_MODE*/
#ifdef CONFIG_NOE_HW_NAT_IPI
#define HW_NAT_IPI                                  HWNAT_BIT(36)
#else
#define HW_NAT_IPI                                  0
#endif /* CONFIG_NOE_HW_NAT_IPI */

#define MDRV_HWNAT_CONFIG_SET_FEATURES(hwnat_device)    \
{                   \
hwnat_device.features = 0;      \
hwnat_device.features |= HNAT_V2;   \
hwnat_device.features |= PACKET_SAMPLING;   \
hwnat_device.features |= PPE_MIB;   \
hwnat_device.features |= NAT_IPV6;  \
hwnat_device.features |= NAT_PREBIND;   \
hwnat_device.features |= ACCNT_MAINTAINER; \
hwnat_device.features |= PPE_MCAST; \
hwnat_device.features |= PPTP_L2TP; \
hwnat_device.features |= QDMA_SUPPORT_QOS; \
hwnat_device.features |= WAN_TO_WLAN_SUPPORT_QOS; \
hwnat_device.features |= WLAN_OPTIMIZE; \
hwnat_device.features |= HW_NAT_WIFI; \
hwnat_device.features |= HW_NAT_WIFI_NEW_ARCH; \
hwnat_device.features |= HW_NAT_QDMA; \
hwnat_device.features |= HW_NAT_ARCH_WIFI_WDMA; \
hwnat_device.features |= HW_NAT_QDMATX_QDMARX; \
hwnat_device.features |= HW_NAT_PSEUDO_SUPPORT; \
hwnat_device.features |= HW_NAT_HW_SFQ; \
hwnat_device.features |= HW_NAT_APCLI_SUPPORT; \
hwnat_device.features |= HW_NAT_WDS_SUPPORT; \
hwnat_device.features |= HW_NAT_MBSS_SUPPORT; \
hwnat_device.features |= HW_NAT_VLAN_TX; \
hwnat_device.features |= HW_NAT_NIC_USB; \
hwnat_device.features |= HW_NAT_PCI; \
hwnat_device.features |= HW_NAT_GMAC2; \
hwnat_device.features |= HW_NAT_SPECIAL_TAG; \
hwnat_device.features |= HW_NAT_IMQ; \
hwnat_device.features |= HW_NAT_MII; \
hwnat_device.features |= HW_NAT_PCI; \
hwnat_device.features |= HW_NAT_USB; \
hwnat_device.features |= HW_NAT_SP_DEV; \
hwnat_device.features |= HW_NAT_AP_SUPPORT; \
hwnat_device.features |= HW_NAT_AP_MESH_SUPPORT; \
hwnat_device.features |= HW_NAT_AP_MBSS_SUPPORT; \
hwnat_device.features |= HW_NAT_AUTO_MODE; \
hwnat_device.features |= HW_NAT_SEMI_AUTO_MODE; \
hwnat_device.features |= HW_NAT_MANUAL_MODE; \
hwnat_device.features |= HW_NAT_IPI; \
}


#define MDRV_HWNAT_CONFIG_DUMP_FEATURES()   \
{\
HWNAT_MSG_MUST("HNAT_V2                  :%d\n", HNAT_V2?1:0);  \
HWNAT_MSG_MUST("PACKET_SAMPLING          :%d\n", PACKET_SAMPLING?1:0);  \
HWNAT_MSG_MUST("PPE_MIB                  :%d\n", PPE_MIB?1:0);  \
HWNAT_MSG_MUST("NAT_IPV6                 :%d\n", NAT_IPV6?1:0); \
HWNAT_MSG_MUST("NAT_PREBIND              :%d\n", NAT_PREBIND?1:0);  \
HWNAT_MSG_MUST("ACCNT_MAINTAINER         :%d\n", ACCNT_MAINTAINER?1:0); \
HWNAT_MSG_MUST("PPE_MCAST                :%d\n", PPE_MCAST?1:0); \
HWNAT_MSG_MUST("PPTP_L2TP                :%d\n", PPTP_L2TP?1:0); \
HWNAT_MSG_MUST("QDMA_SUPPORT_QOS         :%d\n", QDMA_SUPPORT_QOS?1:0); \
HWNAT_MSG_MUST("WAN_TO_WLAN_SUPPORT_QOS  :%d\n", WAN_TO_WLAN_SUPPORT_QOS?1:0); \
HWNAT_MSG_MUST("WLAN_OPTIMIZE            :%d\n", WLAN_OPTIMIZE?1:0); \
HWNAT_MSG_MUST("HW_NAT_WIFI              :%d\n", HW_NAT_WIFI?1:0); \
HWNAT_MSG_MUST("HW_NAT_WIFI_NEW_ARCH     :%d\n", HW_NAT_WIFI_NEW_ARCH?1:0); \
HWNAT_MSG_MUST("HW_NAT_QDMA              :%d\n", HW_NAT_QDMA?1:0); \
HWNAT_MSG_MUST("HW_NAT_ARCH_WIFI_WDMA    :%d\n", HW_NAT_ARCH_WIFI_WDMA?1:0); \
HWNAT_MSG_MUST("HW_NAT_QDMATX_QDMARX     :%d\n", HW_NAT_QDMATX_QDMARX?1:0); \
HWNAT_MSG_MUST("HW_NAT_PSEUDO_SUPPORT    :%d\n", HW_NAT_PSEUDO_SUPPORT?1:0); \
HWNAT_MSG_MUST("HW_NAT_HW_SFQ            :%d\n", HW_NAT_HW_SFQ?1:0); \
HWNAT_MSG_MUST("HW_NAT_APCLI_SUPPORT     :%d\n", HW_NAT_APCLI_SUPPORT?1:0); \
HWNAT_MSG_MUST("HW_NAT_WDS_SUPPORT       :%d\n", HW_NAT_WDS_SUPPORT?1:0); \
HWNAT_MSG_MUST("HW_NAT_MBSS_SUPPORT      :%d\n", HW_NAT_MBSS_SUPPORT?1:0); \
HWNAT_MSG_MUST("HW_NAT_VLAN_TX           :%d\n", HW_NAT_VLAN_TX?1:0); \
HWNAT_MSG_MUST("HW_NAT_NIC_USB           :%d\n", HW_NAT_NIC_USB?1:0); \
HWNAT_MSG_MUST("HW_NAT_PCI               :%d\n", HW_NAT_PCI?1:0); \
HWNAT_MSG_MUST("HW_NAT_GMAC2             :%d\n", HW_NAT_GMAC2?1:0); \
HWNAT_MSG_MUST("HW_NAT_SPECIAL_TAG       :%d\n", HW_NAT_SPECIAL_TAG?1:0); \
HWNAT_MSG_MUST("HW_NAT_IMQ               :%d\n", HW_NAT_IMQ?1:0); \
HWNAT_MSG_MUST("HW_NAT_MII               :%d\n", HW_NAT_MII?1:0); \
HWNAT_MSG_MUST("HW_NAT_PCI               :%d\n", HW_NAT_PCI?1:0); \
HWNAT_MSG_MUST("HW_NAT_USB               :%d\n", HW_NAT_USB?1:0); \
HWNAT_MSG_MUST("HW_NAT_SP_DEV            :%d\n", HW_NAT_SP_DEV?1:0); \
HWNAT_MSG_MUST("HW_NAT_AP_SUPPORT        :%d\n", HW_NAT_AP_SUPPORT?1:0); \
HWNAT_MSG_MUST("HW_NAT_AP_MESH_SUPPORT   :%d\n", HW_NAT_AP_MESH_SUPPORT?1:0); \
HWNAT_MSG_MUST("HW_NAT_AP_MBSS_SUPPORT   :%d\n", HW_NAT_AP_MBSS_SUPPORT?1:0); \
HWNAT_MSG_MUST("HW_NAT_AUTO_MODE         :%d\n", HW_NAT_AUTO_MODE?1:0); \
HWNAT_MSG_MUST("HW_NAT_SEMI_AUTO_MODE    :%d\n", HW_NAT_SEMI_AUTO_MODE?1:0); \
HWNAT_MSG_MUST("HW_NAT_MANUAL_MODE       :%d\n", HW_NAT_MANUAL_MODE?1:0); \
HWNAT_MSG_MUST("HW_NAT_IPI               :%d\n", HW_NAT_IPI?1:0); \
}



#ifdef DSCP_REMARK_TEST
#define HW_NAT_DSCP_REMARK_TEST  BIT(0)
#else
#define HW_NAT_DSCP_REMARK_TEST  0
#endif /* DSCP_REMARK_TEST */
#ifdef VLAN_LAYER_TEST
#define HW_NAT_VLAN_LAYER_TEST   BIT(1)
#else
#define HW_NAT_VLAN_LAYER_TEST  0
#endif /* VLAN_LAYER_TEST */
#ifdef FORCE_UP_TEST
#define HW_NAT_FORCE_UP_TEST    BIT(2)
#else
#define HW_NAT_FORCE_UP_TEST   0
#endif /* HW_NAT_FORCE_UP_TEST */
#ifdef VPRI_REMARK_TEST
#define HW_NAT_VPRI_REMARK_TEST BIT(3)
#else
#define HW_NAT_VPRI_REMARK_TEST  0
#endif /* VPRI_REMARK_TEST */
#ifdef PREBIND_TEST
#define HW_NAT_PREBIND_TEST     BIT(4)
#else
#define HW_NAT_PREBIND_TEST      0
#endif /* PREBIND_TEST  */



#define MDRV_HWNAT_CONFIG_SET_TEST(hwnat_device)    \
{                   \
hwnat_device.test = 0;      \
hwnat_device.test |= HW_NAT_DSCP_REMARK_TEST;      \
hwnat_device.test |= HW_NAT_VLAN_LAYER_TEST;      \
hwnat_device.test |= HW_NAT_FORCE_UP_TEST;      \
hwnat_device.test |= HW_NAT_VPRI_REMARK_TEST;      \
hwnat_device.test |= HW_NAT_PREBIND_TEST;      \
}


#define MDRV_HWNAT_CONFIG_DUMP_TEST()   \
{\
HWNAT_MSG_MUST("HW_NAT_DSCP_REMARK_TEST  :%d\n", HW_NAT_DSCP_REMARK_TEST?1:0); \
HWNAT_MSG_MUST("HW_NAT_VLAN_LAYER_TEST   :%d\n", HW_NAT_VLAN_LAYER_TEST?1:0); \
HWNAT_MSG_MUST("HW_NAT_FORCE_UP_TEST     :%d\n", HW_NAT_FORCE_UP_TEST?1:0); \
HWNAT_MSG_MUST("HW_NAT_VPRI_REMARK_TEST  :%d\n", HW_NAT_VPRI_REMARK_TEST?1:0); \
HWNAT_MSG_MUST("HW_NAT_PREBIND_TEST      :%d\n", HW_NAT_PREBIND_TEST?1:0); \
}


#if defined(CONFIG_NOE_HW_NAT_TBL_1K)
#define FOE_4TB_SIZ     1024
#elif defined(CONFIG_NOE_HW_NAT_TBL_2K)
#define FOE_4TB_SIZ     2048
#elif defined(CONFIG_NOE_HW_NAT_TBL_4K)
#define FOE_4TB_SIZ     4096
#elif defined(CONFIG_NOE_HW_NAT_TBL_8K)
#define FOE_4TB_SIZ     8192
#elif defined(CONFIG_NOE_HW_NAT_TBL_16K)
#define FOE_4TB_SIZ     16384
#endif /*CONFIG_NOE_HW_NAT_TBL*/

#if defined(CONFIG_NOE_HW_NAT_HASH0)
#define DFL_FOE_HASH_MODE   0
#elif defined(CONFIG_NOE_HW_NAT_HASH1)
#define DFL_FOE_HASH_MODE   1
#elif defined(CONFIG_NOE_HW_NAT_HASH2)
#define DFL_FOE_HASH_MODE   2
#elif defined(CONFIG_NOE_HW_NAT_HASH3)
#define DFL_FOE_HASH_MODE   3
#elif defined(CONFIG_NOE_HW_NAT_HASH_DBG)
#define DFL_FOE_HASH_MODE   0
#endif /*CONFIG_NOE_HW_NAT_HASH*/

#if defined(CONFIG_NOE_HW_NAT_HASH_DBG_SPORT)
#define DEF_HASH_DBG_MODE  1
#elif defined(CONFIG_NOE_HW_NAT_HASH_DBG_IPV6_SIP)
#define DEF_HASH_DBG_MODE  3
#elif defined(CONFIG_NOE_HW_NAT_HASH_DBG_IPV4_SIP)
#define DEF_HASH_DBG_MODE  2
#else
#define DEF_HASH_DBG_MODE  0
#endif /*CONFIG_NOE_HW_NAT_HASH_DBG*/



#define IS_SPECIAL_DEV(x) ((x) & (HW_NAT_MII | HW_NAT_PCI | HW_NAT_USB | HW_NAT_SP_DEV))
#define IS_APMBSS_MBSS_DEV(x) ((x) & (HW_NAT_AP_MBSS_SUPPORT | HW_NAT_MBSS_SUPPORT))
#define IS_SPECIAL_DEV_OR_AP(x) ((x) & (HW_NAT_MII | HW_NAT_PCI | HW_NAT_USB | HW_NAT_SP_DEV | HW_NAT_AP_SUPPORT))

#define IS_QDMA_MCAST_SUPPORT(x)                ((x & HW_NAT_QDMA) && (x & PPE_MCAST))
//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
#define HWSFQUP (3)
#define HWSFQDL (1)


enum BindDir {
    UPSTREAM_ONLY = 0,  /* only speed up upstream flow */
    DOWNSTREAM_ONLY = 1,    /* only speed up downstream flow */
    BIDIRECTION = 2     /* speed up bi-direction flow */
};

#ifndef CONFIG_NOE_HW_NAT_ACL_DLTA
#define CONFIG_NOE_HW_NAT_ACL_DLTA 3
#endif



#define DFL_FOE_TTL_REGEN       (1)   /* TTL = TTL -1 */
#define DFL_FOE_TCP_KA          (CONFIG_NOE_HW_NAT_TCP_KA)
#define DFL_FOE_UDP_KA          (CONFIG_NOE_HW_NAT_UDP_KA)
#define DFL_FOE_NTU_KA          (CONFIG_NOE_HW_NAT_NTU_KA)
#define DFL_FOE_BNDR            (CONFIG_NOE_HW_NAT_BINDING_THRESHOLD)

#ifndef CONFIG_NOE_HW_NAT_PBND_RD_LMT
#define DFL_PBND_RD_LMT         (1)
#else
#define DFL_PBND_RD_LMT         (CONFIG_NOE_HW_NAT_PBND_RD_LMT)
#endif

#ifndef CONFIG_NOE_HW_NAT_PBND_RD_PRD
#define DFL_PBND_RD_PRD         (1)
#else
#define DFL_PBND_RD_PRD         (CONFIG_NOE_HW_NAT_PBND_RD_PRD)
#endif
#define DFL_FOE_UNB_MNP         (CONFIG_NOE_HW_NAT_UNB_MNP)
#define DFL_FOE_ACL_DLTA        (CONFIG_NOE_HW_NAT_ACL_DLTA)
#define DFL_FOE_UNB_DLTA        (CONFIG_NOE_HW_NAT_UNB_DLTA)
#define DFL_FOE_NTU_DLTA        (CONFIG_NOE_HW_NAT_NTU_DLTA)
#define DFL_FOE_UDP_DLTA        (CONFIG_NOE_HW_NAT_UDP_DLTA)
#define DFL_FOE_FIN_DLTA        (CONFIG_NOE_HW_NAT_FIN_DLTA)
#define DFL_FOE_TCP_DLTA        (CONFIG_NOE_HW_NAT_TCP_DLTA)

#define WIFI_INFO_LEN           ((HW_NAT_ARCH_WIFI_WDMA)?3:0)
#define FOE_INFO_LEN            ((PPTP_L2TP)? (10 + WIFI_INFO_LEN):(6 + WIFI_INFO_LEN))
#define FOE_MAGIC_FASTPATH      (0x77)
#define FOE_MAGIC_L2TPPATH      (0x78)
#define DFL_FOE_KA              ((HNAT_V2)?E_HWNAT_KA_CFG_ALOPKT2CPU:E_HWNAT_KA_CFG_NONE)


#define MDRV_HWNAT_CONFIG_DUMP_VALUE()  \
{\
HWNAT_MSG_MUST("DFL_FOE_TTL_REGEN        :%d\n", DFL_FOE_TTL_REGEN); \
HWNAT_MSG_MUST("DFL_FOE_TCP_KA           :%d\n", DFL_FOE_TCP_KA   ); \
HWNAT_MSG_MUST("DFL_FOE_UDP_KA           :%d\n", DFL_FOE_UDP_KA   ); \
HWNAT_MSG_MUST("DFL_FOE_NTU_KA           :%d\n", DFL_FOE_NTU_KA   ); \
HWNAT_MSG_MUST("DFL_FOE_BNDR             :%d\n", DFL_FOE_BNDR     ); \
HWNAT_MSG_MUST("DFL_PBND_RD_LMT          :%d\n", DFL_PBND_RD_LMT  ); \
HWNAT_MSG_MUST("DFL_PBND_RD_PRD          :%d\n", DFL_PBND_RD_PRD  ); \
HWNAT_MSG_MUST("DFL_FOE_UNB_MNP          :%d\n", DFL_FOE_UNB_MNP  ); \
HWNAT_MSG_MUST("DFL_FOE_ACL_DLTA         :%d\n", DFL_FOE_ACL_DLTA ); \
HWNAT_MSG_MUST("DFL_FOE_UNB_DLTA         :%d\n", DFL_FOE_UNB_DLTA ); \
HWNAT_MSG_MUST("DFL_FOE_NTU_DLTA         :%d\n", DFL_FOE_NTU_DLTA ); \
HWNAT_MSG_MUST("DFL_FOE_UDP_DLTA         :%d\n", DFL_FOE_UDP_DLTA ); \
HWNAT_MSG_MUST("DFL_FOE_FIN_DLTA         :%d\n", DFL_FOE_FIN_DLTA ); \
HWNAT_MSG_MUST("DFL_FOE_TCP_DLTA         :%d\n", DFL_FOE_TCP_DLTA ); \
}

#endif /* _MDRV_NOE_HWNAT_CONFIG_H_ */
