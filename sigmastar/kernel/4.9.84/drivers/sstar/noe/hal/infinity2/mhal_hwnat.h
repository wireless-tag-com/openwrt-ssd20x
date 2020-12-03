/*
* mhal_hwnat.h- Sigmastar
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
/// @file   Mhal_hwnat.h
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////



#ifndef _MHAL_HWNAT_H_
#define _MHAL_HWNAT_H_

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include "mhal_porting.h"
//#include "mach/memory.h"
//#include "mstar/mstar_chip.h"
//#include "chip_setup.h"
#include "mhal_noe.h"
#include "mhal_noe_reg.h"
#include "mhal_noe_dma.h"
#include "mhal_hwnat_cfg.h"
#include "mhal_hwnat_entry.h"

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
#define HWNAT_INVALID_PHY_ADDR (0xFFFFFFFF)
#define IS_IPV6_FLAB_EBL() ((MHal_HWNAT_Is_Ipv6_Hash_Label() == TRUE)?1:0)



//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------
typedef enum {
    E_HWNAT_PKT_DST_ENGINE = 0,
    E_HWNAT_PKT_DST_CPU = 1,
}EN_HWNAT_PKT_DST_CFG;

typedef enum {
   E_HWNAT_FOE_SEARCH_MISS_DROP = 0,
   E_HWNAT_FOE_SEARCH_MISS_DROP2 = 1,
   E_HWNAT_FOE_SEARCH_MISS_ONLY_FWD_CPU = 2,
   E_HWNAT_FOE_SEARCH_MISS_FWD_CPU_BUILD_ENTRY = 3,
}EN_HWNAT_FOE_SEARCH_MISS_CFG;

typedef enum {
   E_HWNAT_HASH_DBG_NONE = 0,
   E_HWNAT_HASH_DBG_SRC_PORT,
   E_HWNAT_HASH_DBG_IPV4_SIP,
   E_HWNAT_HASH_DBG_IPV6_SIP,
}EN_HWNAT_HASH_DBG;

typedef enum {
   E_HWNAT_KA_CFG_NONE = 0,
   E_HWNAT_KA_CFG_UCOPKT2CPU = 1,
   E_HWNAT_KA_CFG_MCNPKT2CPU = 2,
   E_HWNAT_KA_CFG_ALOPKT2CPU = 3,
}EN_HWNAT_KA_CFG;

typedef enum {
    E_HWNAT_OPMODE_AUTO = 0,
    E_HWNAT_OPMODE_SEMI_AUTO = 1,
    E_HWNAT_OPMODE_MANUAL = 2,
}EN_HWNAT_OPMODE;


struct mhal_table_info {
    MS_U32 foe_adr;
    MS_U32 ps_adr;
    MS_U32 mib_adr;
    MS_U32 entries;
    MS_U8 hash_mode;
    MS_U8 hash_dbg;
    MS_BOOL ipv6;
    MS_BOOL prebind;
    MS_BOOL pptp_l2tp;
    MS_BOOL mcast;
    MS_BOOL qtx_qrx;
    MS_U8 op_mode;
};

struct mhal_hwnat_ka {
    MS_U8 cfg;  /* EN_HWNAT_KA_CFG */
    MS_U32 udp;
    MS_U32 tcp;
    MS_U32 ntu;
    MS_U8  pbnd_limit;
};

struct mhal_hwnat_age_out {
    MS_U16 udp_time;
    MS_U16 tcp_time;
    MS_U16 fin_time;
    MS_U16 ntu_time;
    MS_U8 unb_time;
    MS_U16 unb_pkt_cnt;
};

struct mhal_hwnat_rate {
    MS_U16 tbl_read_rate;
    MS_U16 bind_rate;
};

struct mhal_hwnat_engine {
    struct mhal_hwnat_ka ka;
    struct mhal_hwnat_age_out age_out;
    struct mhal_hwnat_rate rate;
};


struct mhal_hwnat_ac_info {
    MS_U8 idx;
    MS_U64 bytes;
    MS_U32 pkts;
};

//-------------------------------------------------------------------------------------------------
//  Function
//-------------------------------------------------------------------------------------------------

void MHal_HWNAT_Init(struct mhal_table_info *info);
void MHal_HWNAT_Set_DBG(EN_NOE_SEL enable);
void MHal_HWNAT_Enable_Cache(void);
void MHal_HWNAT_Map_ForcePort(void);
void MHal_HWNAT_Set_Pkt_Prot(void);
void MHal_HWNAT_Check_Switch_Vlan(MS_BOOL enable);
void MHal_HWNAT_Get_Ac_Info(struct mhal_hwnat_ac_info *info);
void MHal_HWNAT_Start_Engine(struct mhal_hwnat_engine *info);
void MHal_HWNAT_Stop_Engine(void);
void MHAL_HWNAT_Get_Addr(MS_U32 *pAddr);
void MHAL_HWNAT_Dump_Mib(MS_U32 entry_num);
MS_U32 MHAL_HWNAT_Get_TimeStamp(void);
void MHal_HWNAT_Set_Pkt_Dst(EN_NOE_GE_MAC ge, EN_HWNAT_PKT_DST_CFG dst);

void MHal_HWNAT_Get_Mib_Info(MS_U32 entry_num, MS_U64 *bytes, MS_U64 *pkts);
void MHal_HWNAT_Set_Unbind_Lifetime(MS_U8 lifetime);
void MHal_HWNAT_Set_KeepAlive_Interval(MS_U8 tcp_ka, MS_U8 udp_ka);
void MHal_HWNAT_Set_Bind_Lifetime(MS_U16 tcp_life, MS_U16 udp_life, MS_U16 fin_life);
void MHal_HWNAT_Set_Max_Entry_Limit(MS_U32 full, MS_U32 half, MS_U32 qurt);
void MHal_HWNAT_Set_Bind_Threshold(MS_U32 threshold);
MS_BOOL MHal_HWNAT_Is_Ipv6_Hash_Label(void);
void MHal_HWNAT_Set_Hash_Seed(MS_U16 seed);
void MHal_HWNAT_Set_Miss_Action(EN_HWNAT_FOE_SEARCH_MISS_CFG cfg);
void MHal_HWNAT_Dump_Cache_Entry(void);
void MHal_HWNAT_Clear_MIB_Counter(MS_U16 addr);

MS_BOOL MHal_HWNAT_Get_QoS_Status(void);
#endif /* _MHAL_HWNAT_H_ */

