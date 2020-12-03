/*
* mdrv_noe_ioctl.h- Sigmastar
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
/// @file   MDRV_NOE_IOCTL.c
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _MDRV_NOE_IOCTL_H_
#define _MDRV_NOE_IOCTL_H_


//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include "mdrv_noe_def.h"

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------

/* ioctl commands */
#define NOE_SW_IOCTL                  0x89F0
#define NOE_ESW_REG_READ                0x89F1
#define NOE_ESW_REG_WRITE               0x89F2
#define NOE_MII_READ                    0x89F3
#define NOE_MII_WRITE                   0x89F4
#define NOE_ESW_INGRESS_RATE            0x89F5
#define NOE_ESW_EGRESS_RATE         0x89F6
#define NOE_ESW_PHY_DUMP                0x89F7
#define NOE_QDMA_REG_READ               0x89F8
#define NOE_QDMA_REG_WRITE          0x89F9
#define NOE_QDMA_QUEUE_MAPPING        0x89FA
#define NOE_QDMA_READ_CPU_CLK         0x89FB
#define NOE_MII_READ_CL45             0x89FC
#define NOE_MII_WRITE_CL45            0x89FD
#define NOE_QDMA_SFQ_WEB_ENABLE       0x89FE
#define NOE_SET_LAN_IP              0x89FF

/* switch ioctl commands */
#define SW_IOCTL_SET_EGRESS_RATE        0x0000
#define SW_IOCTL_SET_INGRESS_RATE       0x0001
#define SW_IOCTL_SET_VLAN               0x0002
#define SW_IOCTL_DUMP_VLAN              0x0003
#define SW_IOCTL_DUMP_TABLE             0x0004
#define SW_IOCTL_ADD_L2_ADDR            0x0005
#define SW_IOCTL_DEL_L2_ADDR            0x0006
#define SW_IOCTL_ADD_MCAST_ADDR         0x0007
#define SW_IOCTL_DEL_MCAST_ADDR         0x0008
#define SW_IOCTL_DUMP_MIB               0x0009
#define SW_IOCTL_ENABLE_IGMPSNOOP       0x000A
#define SW_IOCTL_DISABLE_IGMPSNOOP      0x000B
#define SW_IOCTL_SET_PORT_TRUNKING      0x000C
#define SW_IOCTL_GET_PORT_TRUNKING      0x000D
#define SW_IOCTL_SET_PORT_MIRROR        0x000E
#define SW_IOCTL_GET_PHY_STATUS         0x000F
#define SW_IOCTL_READ_REG               0x0010
#define SW_IOCTL_WRITE_REG              0x0011
#define SW_IOCTL_QOS_EN                 0x0012
#define SW_IOCTL_QOS_SET_TABLE2TYPE     0x0013
#define SW_IOCTL_QOS_GET_TABLE2TYPE     0x0014
#define SW_IOCTL_QOS_SET_PORT2TABLE     0x0015
#define SW_IOCTL_QOS_GET_PORT2TABLE     0x0016
#define SW_IOCTL_QOS_SET_PORT2PRI       0x0017
#define SW_IOCTL_QOS_GET_PORT2PRI       0x0018
#define SW_IOCTL_QOS_SET_DSCP2PRI       0x0019
#define SW_IOCTL_QOS_GET_DSCP2PRI       0x001a
#define SW_IOCTL_QOS_SET_PRI2QUEUE      0x001b
#define SW_IOCTL_QOS_GET_PRI2QUEUE      0x001c
#define SW_IOCTL_QOS_SET_QUEUE_WEIGHT   0x001d
#define SW_IOCTL_QOS_GET_QUEUE_WEIGHT   0x001e
#define SW_IOCTL_SET_PHY_TEST_MODE      0x001f
#define SW_IOCTL_GET_PHY_REG            0x0020
#define SW_IOCTL_SET_PHY_REG            0x0021
#define SW_IOCTL_VLAN_TAG               0x0022



#define REG_ESW_WT_MAC_MFC              0x0010
#define REG_ESW_ISC                     0x0018
#define REG_ESW_WT_MAC_ATA1             0x0074
#define REG_ESW_WT_MAC_ATA2             0x0078
#define REG_ESW_WT_MAC_ATWD             0x007C
#define REG_ESW_WT_MAC_ATC              0x0080
#define REG_ESW_TABLE_TSRA1             0x0084
#define REG_ESW_TABLE_TSRA2             0x0088
#define REG_ESW_TABLE_ATRD              0x008C
#define REG_ESW_VLAN_VTCR               0x0090
#define REG_ESW_VLAN_VAWD1              0x0094
#define REG_ESW_VLAN_VAWD2              0x0098
#define REG_ESW_VLAN_ID_BASE            0x0100

#define REG_ESW_VLAN_MEMB_BASE          0x0070
#define REG_ESW_TABLE_SEARCH            0x0024
#define REG_ESW_TABLE_STATUS0           0x0028
#define REG_ESW_TABLE_STATUS1           0x002C
#define REG_ESW_TABLE_STATUS2           0x0030
#define REG_ESW_WT_MAC_AD0              0x0034
#define REG_ESW_WT_MAC_AD1              0x0038
#define REG_ESW_WT_MAC_AD2              0x003C

/*10/100 phy cal*/
#define NOE_VBG_IEXT_CALIBRATION        0x0040
#define NOE_TXG_R50_CALIBRATION     0x0041
#define NOE_TXG_OFFSET_CALIBRATION  0x0042
#define NOE_TXG_AMP_CALIBRATION     0x0043




#define REG_ESW_MAX                     0x00FC
#define REG_HQOS_MAX                    0x3FFF

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------

struct ra_mii_ioctl_data {
    unsigned int  phy_id;
    unsigned int  reg_num;
    unsigned int  val_in;
    unsigned int  val_out;
    unsigned int  port_num;
    unsigned int  dev_addr;
    unsigned int  reg_addr;
};


struct noe_reg {
    unsigned int off;
    unsigned int val;
};

#endif /* _MDRV_NOE_IOCTL_H_ */
