/*
* mhal_hwnat.c- Sigmastar
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
/// @file   Mhal_hwnat.c
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------

#include <linux/kernel.h>
#include <linux/delay.h>
#include "ms_platform.h"
#include "registers.h"
#include "mhal_noe_reg.h"
#include "mhal_noe.h"
#include "mhal_noe_lro.h"
#include "mhal_hwnat_cfg.h"
#include "mhal_hwnat.h"

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
enum {
    E_HWNAT_FOE_TBL_NUM_1K = 0,
    E_HWNAT_FOE_TBL_NUM_2K = 1,
    E_HWNAT_FOE_TBL_NUM_4K = 2,
    E_HWNAT_FOE_TBL_NUM_8K = 3,
    E_HWNAT_FOE_TBL_NUM_16K = 4,
};

enum {
    E_HWNAT_FOE_TBL_SIZE_64B = 0,
    E_HWNAT_FOE_TBL_SIZE_80B = 1,
};

struct mhal_hwnat_config {
    EN_NOE_SEL dbg_enable;
    EN_NOE_SEL prebind;
    EN_NOE_SEL mib;
    EN_NOE_SEL ps;
    EN_NOE_SEL pptp_l2tp;
    EN_NOE_SEL mcast;
    EN_NOE_SEL qtx_qrx;
    EN_HWNAT_OPMODE op_mode;
};



//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define MHAL_HWNAT_MUST(fmt, args...) printk(fmt, ##args)
#define MHAL_HWNAT_DBG_INFO(fmt, args...) \
{\
    if (_hwnat_config.dbg_enable == E_NOE_SEL_ENABLE)\
        printk(fmt, ##args);\
}

#define MAX_CACHE_LINE_NUM      32
//--------------------------------------------------------------------------------------------------
//  Local Functions
//--------------------------------------------------------------------------------------------------




//--------------------------------------------------------------------------------------------------
//  Variable
//--------------------------------------------------------------------------------------------------


static struct mhal_hwnat_config _hwnat_config = {
    .dbg_enable = E_NOE_SEL_DISABLE,
    .prebind = E_NOE_SEL_DISABLE,
    .mib = E_NOE_SEL_DISABLE,
    .ps = E_NOE_SEL_DISABLE,
    .pptp_l2tp = E_NOE_SEL_DISABLE,
    .mcast  = E_NOE_SEL_DISABLE,
    .qtx_qrx  = E_NOE_SEL_DISABLE,
    .op_mode = E_HWNAT_OPMODE_AUTO,
};




//--------------------------------------------------------------------------------------------------
//  Local Variable
//--------------------------------------------------------------------------------------------------

static void _NOE_HWNAT_RIU_REG_BITS_WRITE(void __iomem *Addr, MS_U32 Data, MS_U32 Offset, MS_U32 Len)
{
    unsigned int Mask = 0;
    unsigned int Value;
    unsigned int i;

    for (i = 0; i < Len; i++) {
        Mask |= 1 << (Offset + i);
    }

    Value = MHal_NOE_Read_Reg(Addr);
    Value &= ~Mask;
    Value |= (Data << Offset) & Mask;;
    MHal_NOE_Write_Reg(Addr, Value);
}



void MHal_HWNAT_Init(struct mhal_table_info *info)
{
    MHal_NOE_Write_Reg(PPE_FOE_BASE, info->foe_adr);
    if (info->ps_adr != HWNAT_INVALID_PHY_ADDR) {
        MHal_NOE_Write_Reg(PS_TB_BASE, info->ps_adr);
        _hwnat_config.ps = E_NOE_SEL_ENABLE;
    }
    if (info->mib_adr != HWNAT_INVALID_PHY_ADDR) {
        MHal_NOE_Write_Reg(MIB_TB_BASE, info->mib_adr);
        _hwnat_config.mib = E_NOE_SEL_ENABLE;
    }

    switch (info->entries) {
        case 1024:
            _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, FOE_TBL_NUM_1K, 0, 3);
            break;
        case 2048:
            _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, FOE_TBL_NUM_2K, 0, 3);
            break;
        case 4096:
            _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, FOE_TBL_NUM_4K, 0, 3);
            break;
        case 8192:
            _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, FOE_TBL_NUM_8K, 0, 3);
            break;
        case 16384:
            _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, FOE_TBL_NUM_16K, 0, 3);
            break;
        default:
            MHAL_HWNAT_MUST("Set as default value \n");
            _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, FOE_TBL_NUM_4K, 0, 3);
            break;
    }
    /* Set Hash Mode */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, info->hash_mode, 14, 2);
    MHal_NOE_Write_Reg(PPE_HASH_SEED, HASH_SEED);
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, info->hash_dbg, 18, 2);

    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, E_HWNAT_FOE_TBL_SIZE_80B, 3, 1); /* entry size = 80bytes */

    if (info->prebind == TRUE) {
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, 1, 6, 1);    /* pre-bind age enable */
        _hwnat_config.prebind = E_NOE_SEL_ENABLE;
    }
    _hwnat_config.pptp_l2tp = (info->pptp_l2tp == TRUE)?E_NOE_SEL_ENABLE:E_NOE_SEL_DISABLE;
    _hwnat_config.prebind = (info->prebind == TRUE)?E_NOE_SEL_ENABLE:E_NOE_SEL_DISABLE;
    _hwnat_config.mcast = (info->mcast == TRUE)?E_NOE_SEL_ENABLE:E_NOE_SEL_DISABLE;
    _hwnat_config.qtx_qrx = (info->qtx_qrx == TRUE)?E_NOE_SEL_ENABLE:E_NOE_SEL_DISABLE;
    _hwnat_config.op_mode = info->op_mode;
    /* Set action for FOE search miss */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, FOE_SEARCH_MISS_FWD_CPU_BUILD_ENTRY, 4, 2);
}


void MHal_HWNAT_Enable_Cache(void)
{
    /* clear cache table before enabling cache */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 1, 9, 1);
    _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 0, 9, 1);

    /* Cache enable */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 1, 0, 1);
}

void MHal_HWNAT_Set_Miss_Action(EN_HWNAT_FOE_SEARCH_MISS_CFG cfg)
{
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, cfg, 4, 2);
}

void MHal_HWNAT_Set_Hash_Seed(MS_U16 seed)
{
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_HASH_SEED, seed, 0, 16);
}

MS_BOOL MHal_HWNAT_Is_Ipv6_Hash_Label(void)
{
    return  (MHal_NOE_Read_Reg(PPE_FLOW_SET) & BIT_IPV6_HASH_FLAB)? TRUE : FALSE;

}

static void _MHal_HWNAT_Set_KA(struct mhal_hwnat_ka *info)
{
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, info->cfg, 12, 2);
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_KA, MHAL_HWNAT_KA_T, 0, 16);
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_KA, info->tcp, 16, 8);
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_KA, info->udp, 24, 8);
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_BIND_LMT_1, info->ntu, 16, 8);

    if (_hwnat_config.prebind == E_NOE_SEL_ENABLE)
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_BIND_LMT_1, info->pbnd_limit, 24, 8);
}

static void _MHal_HWNAT_Set_Flow(MS_BOOL enable)
{
    u32 ppe_flow_set = MHal_NOE_Read_Reg(PPE_FLOW_SET);

    /* FOE engine need to handle unicast/multicast/broadcast flow */
    if (enable) {
        ppe_flow_set |= (BIT_IPV4_NAPT_EN | BIT_IPV4_NAT_EN);

        ppe_flow_set |= (BIT_IPV4_NAT_FRAG_EN | BIT_UDP_IP4F_NAT_EN);   /* ip fragment */
        if (_hwnat_config.pptp_l2tp == E_NOE_SEL_DISABLE) {
            ppe_flow_set |= (BIT_IPV4_HASH_GREK);
        }
        ppe_flow_set |= (BIT_IPV4_DSL_EN | BIT_IPV6_6RD_EN | BIT_IPV6_3T_ROUTE_EN | BIT_IPV6_5T_ROUTE_EN);
        /* ppe_flow_set |= (BIT_IPV6_HASH_FLAB); // flow label */
        ppe_flow_set |= (BIT_IPV6_HASH_GREK);

    } else {
        ppe_flow_set &= ~(BIT_IPV4_NAPT_EN | BIT_IPV4_NAT_EN);
        ppe_flow_set &= ~(BIT_IPV4_NAT_FRAG_EN);

        ppe_flow_set &= ~(BIT_IPV4_DSL_EN | BIT_IPV6_6RD_EN | BIT_IPV6_3T_ROUTE_EN | BIT_IPV6_5T_ROUTE_EN);
        /* ppe_flow_set &= ~(BIT_IPV6_HASH_FLAB); */
        ppe_flow_set &= ~(BIT_IPV6_HASH_GREK);


    }

    MHal_NOE_Write_Reg(PPE_FLOW_SET, ppe_flow_set);
}


static void _MHal_HWNAT_Set_Ageout(struct mhal_hwnat_age_out *info)
{
    /* set Bind Non-TCP/UDP Age Enable */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, DFL_FOE_NTU_AGE, 7, 1);

    /* set Unbind State Age Enable */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, DFL_FOE_UNB_AGE, 8, 1);

    /* set min threshold of packet count for aging out at unbind state */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_UNB_AGE, info->unb_pkt_cnt, 16, 16);

    /* set Delta time for aging out an unbind FOE entry */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_UNB_AGE, info->unb_time, 0, 8);

    if (_hwnat_config.op_mode != E_HWNAT_OPMODE_MANUAL) {
        /* set Bind TCP Age Enable */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, DFL_FOE_TCP_AGE, 9, 1);

        /* set Bind UDP Age Enable */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, DFL_FOE_UDP_AGE, 10, 1);

        /* set Bind TCP FIN Age Enable */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, DFL_FOE_FIN_AGE, 11, 1);

        /* set Delta time for aging out an bind UDP FOE entry */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_BND_AGE0, info->udp_time, 0, 16);

        /* set Delta time for aging out an bind Non-TCP/UDP FOE entry */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_BND_AGE0, info->udp_time, 16, 16);

        /* set Delta time for aging out an bind TCP FIN FOE entry */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_BND_AGE1, info->fin_time, 16, 16);

        /* set Delta time for aging out an bind TCP FOE entry */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_BND_AGE1, info->tcp_time, 0, 16);
    }
    else {
        /* fix TCP last ACK issue */
        /* Only need to enable Bind TCP FIN aging out function */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_CFG, DFL_FOE_FIN_AGE, 11, 1);
        /* set Delta time for aging out an bind TCP FIN FOE entry */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_BND_AGE1, info->fin_time, 16, 16);
    }
}

static void _MHal_HWNAT_Set_Global_Config(MS_BOOL enable)
{
    if (enable == TRUE) {
        /* Use default values on P7 */
        /* PPE Engine Enable */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_GLO_CFG, 1, 0, 1);

        /* TSID Enable */
        MHAL_HWNAT_DBG_INFO("TSID Enable\n");
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_GLO_CFG, 1, 1, 1);

        if (_hwnat_config.mcast == E_NOE_SEL_ENABLE) {
            /* Enable multicast table lookup */
            _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_GLO_CFG, 1, 7, 1);
            _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_GLO_CFG, 0, 12, 2); /* Decide by PPE entry hash index */
            _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_MCAST_PPSE, 0, 0, 4);   /* multicast port0 map to PDMA */
            _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_MCAST_PPSE, 1, 4, 4);   /* multicast port1 map to GMAC1 */
            _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_MCAST_PPSE, 2, 8, 4);   /* multicast port2 map to GMAC2 */
            _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_MCAST_PPSE, 5, 12, 4);  /* multicast port3 map to QDMA */
        }

        if (_hwnat_config.qtx_qrx == E_NOE_SEL_ENABLE) {
            MHal_NOE_Write_Reg(PPE_DFT_CPORT, 0x55555555);   /* default CPU port is port5 (QDMA) */
        }
        else {
            MHal_NOE_Write_Reg(PPE_DFT_CPORT, 0);    /* default CPU port is port0 (PDMA) */
        }

        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_DFT_CPORT, 1, 31, 1);

        if(_hwnat_config.ps == E_NOE_SEL_ENABLE) {
            /* MHal_NOE_Write_Reg(PS_CFG, 1); //Enable PacketSampling */
            MHal_NOE_Write_Reg(PS_CFG, 0x3); /* Enable PacketSampling, Disable Aging */
        }

        if (_hwnat_config.mib == E_NOE_SEL_ENABLE) {
            MHal_NOE_Write_Reg(MIB_CFG, 0x03);   /* Enable MIB & read clear */
            MHal_NOE_Write_Reg(MIB_CAH_CTRL, 0x01);  /* enable mib cache */
        }


        /* PPE Packet with TTL=0 */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_GLO_CFG, DFL_TTL0_DRP, 4, 1);

    } else {

        /* PPE Engine Disable */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_GLO_CFG, 0, 0, 1);

        if(_hwnat_config.ps == E_NOE_SEL_ENABLE) {
            MHal_NOE_Write_Reg(PS_CFG, 0);   /* Disable PacketSampling */
        }
        if (_hwnat_config.mib == E_NOE_SEL_ENABLE) {
            MHal_NOE_Write_Reg(MIB_CFG, 0x00);   /* Disable MIB */
        }
    }
}

void MHal_HWNAT_Set_KeepAlive_Interval(MS_U8 tcp_ka, MS_U8 udp_ka)
{
    /* Keep alive time for bind FOE TCP entry */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_KA, tcp_ka, 16, 8);

    /* Keep alive timer for bind FOE UDP entry */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_KA, udp_ka, 24, 8);

}


void MHal_HWNAT_Set_Bind_Lifetime(MS_U16 tcp_life, MS_U16 udp_life, MS_U16 fin_life)
{
    /* set Delta time for aging out an bind UDP FOE entry */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_BND_AGE0, udp_life, 0, 16);

    /* set Delta time for aging out an bind TCP FIN FOE entry */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_BND_AGE1, fin_life, 16, 16);

    /* set Delta time for aging out an bind TCP FOE entry */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_BND_AGE1, tcp_life, 0, 16);

}


void MHal_HWNAT_Set_Unbind_Lifetime(MS_U8 lifetime)
{
    /* set Delta time for aging out an unbind FOE entry */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_UNB_AGE, lifetime, 0, 8);

}

void MHal_HWNAT_Set_Max_Entry_Limit(MS_U32 full, MS_U32 half, MS_U32 qurt)
{
    /* Allowed max entries to be build during a time stamp unit */

    /* smaller than 1/4 of total entries */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_LMT1, qurt, 0, 14);

    /* between 1/2 and 1/4 of total entries */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_LMT1, half, 16, 14);

    /* between full and 1/2 of total entries */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_LMT2, full, 0, 14);

}

static void _MHal_HWNAT_Set_Bind_Rate(struct mhal_hwnat_rate * rate)
{
    /* Allowed max entries to be build during a time stamp unit */

    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_LMT1, MHAL_HWNAT_LMT_QURT, 0, 14);
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_LMT1, MHAL_HWNAT_LMT_HALF, 16, 14);
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_LMT2, MHAL_HWNAT_LMT_FULL, 0, 14);

    /* Set reach bind rate for unbind state */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_BNDR, rate->bind_rate, 0, 16);

    if (_hwnat_config.prebind == E_NOE_SEL_ENABLE) {
        _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_FOE_BNDR, rate->tbl_read_rate, 16, 16);
    }
}


void MHal_HWNAT_Start_Engine(struct mhal_hwnat_engine *info)
{
    /* 0~63 Metering group */
    /* PpeSetMtrByteInfo(1, 500, 3); //TokenRate=500=500KB/s, MaxBkSize= 3 (32K-1B) */
    /* PpeSetMtrPktInfo(1, 5, 3);  //1 pkts/sec, MaxBkSize=3 (32K-1B) */
    /* Set PPE Flow Set */
    _MHal_HWNAT_Set_Flow(TRUE);

    /* Set Auto Age-Out Function */
    _MHal_HWNAT_Set_Ageout(&(info->age_out));

    /* Set PPE FOE KEEPALIVE TIMER */
    _MHal_HWNAT_Set_KA(&(info->ka));
    /* Set PPE FOE Bind Rate */
    _MHal_HWNAT_Set_Bind_Rate(&(info->rate));

    /* Set PPE Global Configuration */
    _MHal_HWNAT_Set_Global_Config(TRUE);
}




void MHal_HWNAT_Stop_Engine(void)
{
    /* Set PPE FOE ENABLE */
    _MHal_HWNAT_Set_Global_Config(FALSE);

    /* Set PPE Flow Set */
    _MHal_HWNAT_Set_Flow(FALSE);

    /* Free FOE table */
    ///foe_free_tbl(FOE_4TB_SIZ);
}


void MHal_HWNAT_Map_ForcePort(void)
{
#if 0
    if (0)
    {
        /* index 0 = force port 0
         * index 1 = force port 1
         * ...........
         * index 7 = force port 7
         * index 8 = no force port
         * index 9 = force to all ports
         */
        MHal_NOE_Write_Reg(PPE_FP_BMAP_0, 0x00020001);
        MHal_NOE_Write_Reg(PPE_FP_BMAP_1, 0x00080004);
        MHal_NOE_Write_Reg(PPE_FP_BMAP_2, 0x00200010);
        MHal_NOE_Write_Reg(PPE_FP_BMAP_3, 0x00800040);
        MHal_NOE_Write_Reg(PPE_FP_BMAP_4, 0x003F0000);
    }
#endif
}


void MHal_HWNAT_Get_Mib_Info(MS_U32 entry_num, MS_U64 *bytes, MS_U64 *pkts)
{
    MHal_NOE_Write_Reg(MIB_SER_CR, entry_num | (1 << 16));
    while (1) {
        if (!((MHal_NOE_Read_Reg(MIB_SER_CR) & 0x10000) >> 16))
            break;
    }
    wmb();
    *bytes = MHal_NOE_Read_Reg(MIB_SER_R1) & 0xffff;  /* byte cnt bit47 ~ bit32 */
    *bytes = (*bytes << 32) | MHal_NOE_Read_Reg(MIB_SER_R0);   /* byte cnt bit31~ bit0 */
    *pkts = MHal_NOE_Read_Reg(MIB_SER_R2) & 0xffffff;    /* packet cnt bit39 ~ bit16 */
    *pkts = (*pkts << 16) |((MHal_NOE_Read_Reg(MIB_SER_R1) & 0xffff0000) >> 16);

}



void MHal_HWNAT_Set_Pkt_Prot(void)
{
    /* IP Protocol Field for IPv4 NAT or IPv6 3-tuple flow */
    /* Don't forget to turn on related bits in PPE_IP_PROT_CHK register if you want to support
     * another IP protocol.
     */
    /* FIXME: enable it to support IP fragement */
    MHal_NOE_Write_Reg(PPE_IP_PROT_CHK, 0xFFFFFFFF); /* IPV4_NXTH_CHK and IPV6_NXTH_CHK */
    if (_hwnat_config.pptp_l2tp)
       _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_IP_PROT_0, IP_PROTO_GRE, 0, 8);
    /* _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_IP_PROT_0, IP_PROTO_GRE, 0, 8); */
    /* _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_IP_PROT_0, IP_PROTO_TCP, 8, 8); */
    /* _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_IP_PROT_0, IP_PROTO_UDP, 16, 8); */
    /* _NOE_HWNAT_RIU_REG_BITS_WRITE(PPE_IP_PROT_0, IP_PROTO_IPV6, 24, 8); */
}

void MHal_HWNAT_Check_Switch_Vlan(MS_BOOL enable)
{

}


void MHal_HWNAT_Get_Ac_Info(struct mhal_hwnat_ac_info *info)
{
    info->bytes = MHal_NOE_Read_Reg(NOE_REG_AC_BASE + (info->idx * 16));
    info->bytes += ((MS_U64)(MHal_NOE_Read_Reg(NOE_REG_AC_BASE + (info->idx * 16) + 4)) << 32); /* 64bit bytes cnt */
    info->pkts = MHal_NOE_Read_Reg(NOE_REG_AC_BASE + info->idx * 16 +8);
}

void MHAL_HWNAT_Get_Addr(MS_U32 *pAddr)
{
    *pAddr = MHal_NOE_Read_Reg(PPE_FOE_BASE);
}

void MHAL_HWNAT_Dump_Mib(MS_U32 entry_num)
{
    unsigned int byt_l = 0;
    unsigned long long byt_h = 0;
    unsigned int pkt_l = 0;

    unsigned int long pkt_h = 0;

    MHal_NOE_Write_Reg(MIB_SER_CR, entry_num | (1 << 16));
    while (1) {
        if (!((MHal_NOE_Read_Reg(MIB_SER_CR) & 0x10000) >> 16))
            break;
    }
    /*make sure write dram correct*/
    wmb();
    byt_l = MHal_NOE_Read_Reg(MIB_SER_R0);   /* byte cnt bit31~ bit0 */
    byt_h = MHal_NOE_Read_Reg(MIB_SER_R1) & 0xffff;  /* byte cnt bit47 ~ bit0 */
    pkt_l = (MHal_NOE_Read_Reg(MIB_SER_R1) & 0xffff0000) >> 16;
    pkt_h = MHal_NOE_Read_Reg(MIB_SER_R2) & 0xffffff;    /* packet cnt bit39 ~ bit16 */

    MHAL_HWNAT_DBG_INFO("************PPE Entry = %d ************\n", entry_num);
    MHAL_HWNAT_DBG_INFO("Packet Cnt = %lu\n", (pkt_h << 16) + pkt_l);

    MHAL_HWNAT_DBG_INFO("Byte Cnt h = %llu\n", byt_h);
    MHAL_HWNAT_DBG_INFO("Byte Cnt l = %u\n", byt_l);
}


MS_U32 MHAL_HWNAT_Get_TimeStamp(void)
{
    return MHal_NOE_Read_Reg(FOE_TS_T);
}


void MHal_HWNAT_Set_DBG(EN_NOE_SEL enable)
{
     _hwnat_config.dbg_enable = enable;
}


void MHal_HWNAT_Set_Pkt_Dst(EN_NOE_GE_MAC ge, EN_HWNAT_PKT_DST_CFG dst)
{
    u32 data = 0;

    if (ge == E_NOE_GE_MAC1) {
        data = MHal_NOE_Read_Reg(FE_GDMA1_FWD_CFG);
        if (dst == E_HWNAT_PKT_DST_ENGINE) {
            data &= ~0x7777;
            /* Uni-cast frames forward to PPE */
            data |= GDM1_UFRC_P_PPE;
            /* Broad-cast MAC address frames forward to PPE */
            data |= GDM1_BFRC_P_PPE;
            /* Multi-cast MAC address frames forward to PPE */
            data |= GDM1_MFRC_P_PPE;
            /* Other MAC address frames forward to PPE */
            data |= GDM1_OFRC_P_PPE;

        } else {
            data &= ~0x7777;
            /* Uni-cast frames forward to CPU */
            data |= GDM1_UFRC_P_CPU;
            /* Broad-cast MAC address frames forward to CPU */
            data |= GDM1_BFRC_P_CPU;
            /* Multi-cast MAC address frames forward to CPU */
            data |= GDM1_MFRC_P_CPU;
            /* Other MAC address frames forward to CPU */
            data |= GDM1_OFRC_P_CPU;
        }

        MHal_NOE_Write_Reg(FE_GDMA1_FWD_CFG, data);
    }
    if (ge == E_NOE_GE_MAC2) {
        data = MHal_NOE_Read_Reg(FE_GDMA2_FWD_CFG);

        if (dst == E_HWNAT_PKT_DST_ENGINE) {
            data &= ~0x7777;
            /* Uni-cast frames forward to PPE */
            data |= GDM1_UFRC_P_PPE;
            /* Broad-cast MAC address frames forward to PPE */
            data |= GDM1_BFRC_P_PPE;
            /* Multi-cast MAC address frames forward to PPE */
            data |= GDM1_MFRC_P_PPE;
            /* Other MAC address frames forward to PPE */
            data |= GDM1_OFRC_P_PPE;
        }
        else {
            data &= ~0x7777;
            /* Uni-cast frames forward to CPU */
            data |= GDM1_UFRC_P_CPU;
            /* Broad-cast MAC address frames forward to CPU */
            data |= GDM1_BFRC_P_CPU;
            /* Multi-cast MAC address frames forward to CPU */
            data |= GDM1_MFRC_P_CPU;
            /* Other MAC address frames forward to CPU */
            data |= GDM1_OFRC_P_CPU;
        }
        MHal_NOE_Write_Reg(FE_GDMA2_FWD_CFG, data);
    }
}


static MS_BOOL _MHal_HWNAT_Is_CacheReq_Done(void)
{
    int count = 1000;

    /* waiting for 1sec to make sure action was finished */
    do {
        if (((MHal_NOE_Read_Reg(CAH_CTRL) >> 8) & 0x1) == 0)
            return 1;
        msleep(1);
    } while (--count);

    return 0;
}


void MHal_HWNAT_Dump_Cache_Entry(void)
{
    int line = 0;
    int state = 0;
    int tag = 0;
    int cah_en = 0;
    int i = 0;

    cah_en = MHal_NOE_Read_Reg(CAH_CTRL) & 0x1;

    if (!cah_en) {
        MHAL_HWNAT_MUST("Cache is not enabled\n");
        return;
    }

    /* cache disable */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 0, 0, 1);

    MHAL_HWNAT_MUST(" No--|---State---|----Tag-----\n");
    MHAL_HWNAT_MUST("-----+-----------+------------\n");
    for (line = 0; line < MAX_CACHE_LINE_NUM; line++) {
        /* set line number */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_LINE_RW, line, 0, 15);

        /* OFFSET_RW = 0x1F (Get Entry Number) */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_LINE_RW, 0x1F, 16, 8);

        /* software access cache command = read */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 2, 12, 2);

        /* trigger software access cache request */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 1, 8, 1);

        if (_MHal_HWNAT_Is_CacheReq_Done()) {
            tag = (MHal_NOE_Read_Reg(CAH_RDATA) & 0xFFFF);
            state = ((MHal_NOE_Read_Reg(CAH_RDATA) >> 16) & 0x3);
            MHAL_HWNAT_MUST("%04d | %s   | %05d\n", line,
                 (state == 3) ? " Lock  " :
                 (state == 2) ? " Dirty " :
                 (state == 1) ? " Valid " : "Invalid", tag);
        }
        else {
            MHAL_HWNAT_MUST("%s is timeout (%d)\n", __func__, line);
        }

        /* software access cache command = read */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 3, 12, 2);

        MHal_NOE_Write_Reg(CAH_WDATA, 0);

        /* trigger software access cache request */
        _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 1, 8, 1);

        if (!_MHal_HWNAT_Is_CacheReq_Done())
            MHAL_HWNAT_MUST("%s is timeout (%d)\n", __func__, line);
        /* dump first 16B for each foe entry */
        MHAL_HWNAT_MUST("==========<Flow Table Entry=%d >===============\n", tag);
        for (i = 0; i < 16; i++) {
            _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_LINE_RW, i, 16, 8);

            /* software access cache command = read */
            _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 2, 12, 2);

            /* trigger software access cache request */
            _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 1, 8, 1);

            if (_MHal_HWNAT_Is_CacheReq_Done())
                MHAL_HWNAT_MUST("%02d  %08X\n", i, MHal_NOE_Read_Reg(CAH_RDATA));
            else
                MHAL_HWNAT_MUST("%s is timeout (%d)\n", __func__, line);

            /* software access cache command = write */
            _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 3, 12, 2);

            MHal_NOE_Write_Reg(CAH_WDATA, 0);

            /* trigger software access cache request */
            _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 1, 8, 1);

            if (!_MHal_HWNAT_Is_CacheReq_Done())
                MHAL_HWNAT_MUST("%s is timeout (%d)\n", __func__, line);
        }
        MHAL_HWNAT_MUST("=========================================\n");
    }

    /* clear cache table before enabling cache */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 1, 9, 1);
    _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 0, 9, 1);

    /* cache enable */
    _NOE_HWNAT_RIU_REG_BITS_WRITE(CAH_CTRL, 1, 0, 1);

}


void MHal_HWNAT_Set_Bind_Threshold(MS_U32 threshold)
{
    MHal_NOE_Write_Reg(PPE_FOE_BNDR, threshold);
}



void MHal_HWNAT_Clear_MIB_Counter(MS_U16 addr)
{
    int count = 100000;

    MHal_NOE_Write_Reg(MIB_SER_CR, addr | (1 << 16));
    do{
        if (!((MHal_NOE_Read_Reg(MIB_SER_CR) & 0x10000) >> 16))
        break;
        //usleep_range(100, 110);
    }while (--count);
    MHal_NOE_Read_Reg(MIB_SER_R0);
    MHal_NOE_Read_Reg(MIB_SER_R1);
    MHal_NOE_Read_Reg(MIB_SER_R1);
    MHal_NOE_Read_Reg(MIB_SER_R2);

}



MS_BOOL MHal_HWNAT_Get_QoS_Status(void)
{
    unsigned int queue, queue_no;
    unsigned int temp = MHal_NOE_Read_Reg(QDMA_TX_SCH);
    if ((temp & 0x00000800) || (temp & 0x08000000)) {
        return TRUE;
    }
    for (queue = 0; queue < NUM_PQ; queue++) {
        queue_no = queue % 16;
        if (queue < 16) {
            MHal_NOE_Write_Reg(QDMA_PAGE, 0);
        }
        else if (queue > 15 && queue <= 31) {
            MHal_NOE_Write_Reg(QDMA_PAGE, 1);
        }
        else if (queue > 31 && queue <= 47) {
            MHal_NOE_Write_Reg(QDMA_PAGE, 2);
        }
        else if (queue > 47 && queue <= 63) {
            MHal_NOE_Write_Reg(QDMA_PAGE, 3);
        }
        temp = MHal_NOE_Read_Reg(QTX_CFG_0 + (QUEUE_OFFSET * queue_no) + 0x4);
        MHal_NOE_Write_Reg(QDMA_PAGE, 0);
        if ((temp & 0x8000000) || (temp & 0x800)) {
            return TRUE;
        }
    }
    return FALSE;
}

