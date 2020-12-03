/*
* mdrv_hwnat_ioctl.c- Sigmastar
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
/// @file   MDRV_HWNAT_IOCTL.c
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ip.h>
#include <linux/ipv6.h>



#include "mhal_hwnat.h"
#include "mdrv_hwnat.h"
#include "mdrv_hwnat_ioctl.h"
#include "mdrv_hwnat_foe.h"
#include "mdrv_hwnat_util.h"
#include "mdrv_hwnat_log.h"
#include "mdrv_hwnat_mcast.h"
#include "mdrv_hwnat_api.h"


//--------------------------------------------------------------------------------------------------
//  Local Functions
//--------------------------------------------------------------------------------------------------



static long MDrv_HWNAT_IOCTL_Control_Device(struct file *file, unsigned int cmd, unsigned long arg);
static void MDrv_HWNAT_IOCTL_Dump_Entry(unsigned int entry_num, unsigned long *pkt_cnt, unsigned long *byte_cnt);
static int MDrv_HWNAT_IOCTL_Clear_All_Entries(void);
static void MDrv_HWNAT_IOCTL_Dump_Dram_Entry(uint32_t entry_num);
static int32_t MDrv_HWNAT_IOCTL_Get_Ac_Info(struct hwnat_ac_args *opt3);


const struct file_operations hw_nat_fops = {
    unlocked_ioctl:MDrv_HWNAT_IOCTL_Control_Device,
    compat_ioctl:MDrv_HWNAT_IOCTL_Control_Device,
};

static long MDrv_HWNAT_IOCTL_Control_Device(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct hwnat_args *opt = (struct hwnat_args *)arg;
    struct hwnat_tuple *opt2 =(struct hwnat_tuple *)arg;
    struct hwnat_tuple *opt2_k;
    struct hwnat_ac_args *opt3 = (struct hwnat_ac_args *)arg;
    struct hwnat_ac_args *opt3_k;
    struct hwnat_config_args *opt4 = (struct hwnat_config_args *)arg;
    struct hwnat_config_args *opt4_k;
    struct hwnat_mcast_args *opt5 = (struct hwnat_mcast_args *)arg;
    struct hwnat_mcast_args *opt5_k;
    struct hwnat_mib_args *opt6 = (struct hwnat_mib_args *)arg;
    struct hwnat_mib_args *opt6_k;
    unsigned long tx_pkt_cnt;
    unsigned long tx_byte_cnt;
    unsigned long rx_pkt_cnt;
    unsigned long rx_byte_cnt;
    struct hwnat_args *opt1;
    int size;

    size = sizeof(struct hwnat_args) + sizeof(struct hwnat_tuple) * 1024 * 16;

    switch (cmd) {
    case HW_NAT_ADD_ENTRY:
        opt2_k = kmalloc(sizeof(*opt2_k), GFP_KERNEL);
        if (copy_from_user(opt2_k, opt2, sizeof(*opt2_k)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        opt2_k->result = MDrv_HWNAT_Add_Foe_Entry(opt2_k);
        kfree(opt2_k);
        break;
    case HW_NAT_DEL_ENTRY:
        HWNAT_MSG_DBG("HW_NAT_DEL_ENTRY\n");
        opt2_k = kmalloc(sizeof(*opt2_k), GFP_KERNEL);
        if (copy_from_user(opt2_k, opt2, sizeof(*opt2_k)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        opt2_k->result = MDrv_HWNAT_Delete_Foe_Entry(opt2_k);
        kfree(opt2_k);
        break;
    case HW_NAT_GET_ALL_ENTRIES:

        opt1 = kmalloc(size, GFP_KERNEL);
        if (copy_from_user(opt1, opt, size))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        opt1->result = MDrv_HWNAT_Foe_Get_All_Entries(opt1);
        if (copy_to_user(opt, opt1, size))
            HWNAT_MSG_ERR("copy_to_user fail\n");

        kfree(opt1);
        break;
    case HW_NAT_BIND_ENTRY:
        opt1 = kmalloc(sizeof(*opt1), GFP_KERNEL);
        if (copy_from_user(opt1, opt, sizeof(struct hwnat_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        opt1->result = MDrv_HWNAT_Foe_Set_Bind(opt1);
        kfree(opt1);
        break;
    case HW_NAT_UNBIND_ENTRY:
        opt1 = kmalloc(sizeof(*opt1), GFP_KERNEL);
        if (copy_from_user(opt1, opt, sizeof(struct hwnat_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        opt1->result = MDrv_HWNAT_Foe_Set_Unbind(opt1);
        kfree(opt1);
        break;
    case HW_NAT_DROP_ENTRY:
        opt1 = kmalloc(sizeof(*opt1), GFP_KERNEL);
        if (copy_from_user(opt1, opt, sizeof(struct hwnat_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        opt1->result = MDrv_HWNAT_Foe_Drop_Entry(opt1);
        kfree(opt1);
        break;
    case HW_NAT_INVALID_ENTRY:
        opt1 = kmalloc(sizeof(*opt1), GFP_KERNEL);
        if (copy_from_user(opt1, opt, sizeof(struct hwnat_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        opt1->result = MDrv_HWNAT_Foe_Delete_Entry(opt1->entry_num);
        kfree(opt1);
        break;
    case HW_NAT_DUMP_ENTRY:
        opt1 = kmalloc(size, GFP_KERNEL);
        if (copy_from_user(opt1, opt, sizeof(struct hwnat_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        MDrv_HWNAT_Foe_Dump_Entry(opt1->entry_num);
        kfree(opt1);
        break;
    case HW_NAT_DUMP_CACHE_ENTRY:
        MHal_HWNAT_Dump_Cache_Entry();
        break;
    case HW_NAT_DEBUG:  /* For Debug */
        opt1 = kmalloc(size, GFP_KERNEL);
        if (copy_from_user(opt1, opt, sizeof(struct hwnat_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        _hwnat_info.debug_level = opt1->debug;
        kfree(opt1);
        break;
    case HW_NAT_GET_AC_CNT:
        opt3_k = kmalloc(sizeof(*opt3_k), GFP_KERNEL);
        if (copy_from_user(opt3_k, opt3, sizeof(*opt3_k)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        opt3_k->result = MDrv_HWNAT_IOCTL_Get_Ac_Info(opt3_k);
        kfree(opt3_k);
        break;
    case HW_NAT_BIND_THRESHOLD:
        opt4_k = kmalloc(sizeof(*opt4_k), GFP_KERNEL);
        if (copy_from_user(opt4_k, opt4, sizeof(struct hwnat_config_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        MHal_HWNAT_Set_Bind_Threshold(opt4_k->bind_threshold);
        opt4_k->result = HWNAT_SUCCESS;
        kfree(opt4_k);
        break;
    case HW_NAT_MAX_ENTRY_LMT:
        opt4_k = kmalloc(sizeof(*opt4_k), GFP_KERNEL);
        if (copy_from_user(opt4_k, opt4, sizeof(struct hwnat_config_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        MHal_HWNAT_Set_Max_Entry_Limit(opt4_k->foe_full_lmt, opt4_k->foe_half_lmt, opt4_k->foe_qut_lmt);
        opt4_k->result = HWNAT_SUCCESS;
        kfree(opt4_k);
        break;
    case HW_NAT_KA_INTERVAL:
        opt4_k = kmalloc(sizeof(*opt4_k), GFP_KERNEL);
        if (copy_from_user(opt4_k, opt4, sizeof(struct hwnat_config_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        MHal_HWNAT_Set_KeepAlive_Interval(opt4->foe_tcp_ka, opt4->foe_udp_ka);
        opt4_k->result = HWNAT_SUCCESS;
        kfree(opt4_k);
        break;
    case HW_NAT_UB_LIFETIME:
        opt4_k = kmalloc(sizeof(*opt4_k), GFP_KERNEL);
        if (copy_from_user(opt4_k, opt4, sizeof(struct hwnat_config_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        MHal_HWNAT_Set_Unbind_Lifetime(opt4_k->foe_unb_dlta);
        opt4_k->result = HWNAT_SUCCESS;
        kfree(opt4_k);
        break;
    case HW_NAT_BIND_LIFETIME:
        opt4_k = kmalloc(sizeof(*opt4_k), GFP_KERNEL);
        if (copy_from_user(opt4_k, opt4, sizeof(struct hwnat_config_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        MHal_HWNAT_Set_Bind_Lifetime(opt4_k->foe_tcp_dlta, opt4_k->foe_udp_dlta, opt4_k->foe_fin_dlta);
        opt4_k->result = HWNAT_SUCCESS;
        kfree(opt4_k);
        break;
    case HW_NAT_BIND_DIRECTION:
        opt4_k = kmalloc(sizeof(*opt4_k), GFP_KERNEL);
        if (copy_from_user(opt4_k, opt4, sizeof(struct hwnat_config_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        _hwnat_info.bind_dir = opt4_k->bind_dir;
        kfree(opt4_k);
        break;
    case HW_NAT_VLAN_ID:
        opt4_k = kmalloc(sizeof(*opt4_k), GFP_KERNEL);
        if (copy_from_user(opt4_k, opt4, sizeof(struct hwnat_config_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        _hwnat_info.wan_vid = opt4_k->wan_vid;
        _hwnat_info.lan_vid = opt4_k->lan_vid;
        kfree(opt4_k);
        break;
    case HW_NAT_MCAST_INS:
        opt5_k = kmalloc(sizeof(*opt5_k), GFP_KERNEL);
        if (copy_from_user(opt5_k, opt5, sizeof(struct hwnat_mcast_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        MDrv_HWNAT_Mcast_Insert_Entry(opt5_k->mc_vid, opt5_k->dst_mac, opt5_k->mc_px_en, opt5_k->mc_px_qos_en, opt5_k->mc_qos_qid);
        kfree(opt5_k);
        break;
    case HW_NAT_MCAST_DEL:
        opt5_k = kmalloc(sizeof(*opt5_k), GFP_KERNEL);
        if (copy_from_user(opt5_k, opt5, sizeof(struct hwnat_mcast_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        MDrv_HWNAT_Mcast_Delete_Entry(opt5->mc_vid, opt5->dst_mac, opt5->mc_px_en, opt5->mc_px_qos_en, opt5->mc_qos_qid);
        kfree(opt5_k);
        break;
    case HW_NAT_MCAST_DUMP:
        MDrv_HWNAT_Mcast_Dump();
        break;
    case HW_NAT_MIB_DUMP:
        opt6_k = kmalloc(sizeof(*opt6_k), GFP_KERNEL);
        if (copy_from_user(opt6_k, opt6, sizeof(struct hwnat_mib_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        MDrv_HWNAT_IOCTL_Dump_Entry(opt6_k->entry_num, &tx_pkt_cnt, &tx_byte_cnt);
        kfree(opt6_k);
        break;
    case HW_NAT_MIB_DRAM_DUMP:
        opt6_k = kmalloc(sizeof(*opt6_k), GFP_KERNEL);
        if (copy_from_user(opt6_k, opt6, sizeof(struct hwnat_mib_args)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        MDrv_HWNAT_IOCTL_Dump_Dram_Entry(opt6_k->entry_num);
        kfree(opt6_k);
        break;
    case HW_NAT_MIB_GET:
        opt2_k = kmalloc(sizeof(*opt2_k), GFP_KERNEL);
        if (copy_from_user(opt2_k, opt2, sizeof(*opt2_k)))
            HWNAT_MSG_ERR("copy_from_user fail\n");
        opt2_k->result = MDrv_HWNAT_Get_Ppe_Mib_Info(opt2_k, &tx_pkt_cnt, &tx_byte_cnt, &rx_pkt_cnt, &rx_byte_cnt);
        HWNAT_MSG_DBG("!!!!, tx byte = %lu\n", tx_byte_cnt);
        HWNAT_MSG_DBG("!!!!, tx pkt = %lu\n", tx_pkt_cnt);
        HWNAT_MSG_DBG("!!!!, rx byte = %lu\n", rx_byte_cnt);
        HWNAT_MSG_DBG("!!!!, rx pkt = %lu\n", rx_pkt_cnt);
        kfree(opt2_k);
        break;
    case HW_NAT_TBL_CLEAR:
        MDrv_HWNAT_IOCTL_Clear_All_Entries();
        break;
    case HW_NAT_DPORT:
        MDrv_HWNAT_Dump_Dev_Handler();
        break;
    case HW_NAT_SYS_CONFIG:
        MDRV_HWNAT_CONFIG_DUMP_FEATURES();
        MDRV_HWNAT_CONFIG_DUMP_TEST();
        MDRV_HWNAT_CONFIG_DUMP_VALUE();
        break;
    default:
        break;
    }
    return 0;
}



int MDrv_HWNAT_IOCTL_Register_Handler(void)
{
    int result = 0;

    result = register_chrdev(HW_NAT_MAJOR, HW_NAT_DEVNAME, &hw_nat_fops);
    if (result < 0) {
        NAT_PRINT(KERN_WARNING "hw_nat: can't get major %d\n", HW_NAT_MAJOR);
        return result;
    }

    if (HW_NAT_MAJOR == 0)
        HWNAT_MSG_ERR("HNAT Major num=%d\n", result);

    return 0;
}

void MDrv_HWNAT_IOCTL_Unregister_Handler(void)
{
    unregister_chrdev(HW_NAT_MAJOR, HW_NAT_DEVNAME);
}




int MDrv_HWNAT_Reply_Ppe_Entry_Idx(struct hwnat_tuple *opt, unsigned int entry_num)
{
    struct foe_entry *entry = &ppe_foe_base[entry_num];
    struct foe_pri_key key;
    int32_t hash_index;

    if(opt->pkt_type == IPV4_NAPT) {
        key.ipv4_hnapt.sip = entry->ipv4_hnapt.new_dip;
        key.ipv4_hnapt.dip = entry->ipv4_hnapt.new_sip;
        key.ipv4_hnapt.sport = entry->ipv4_hnapt.new_dport;
        key.ipv4_hnapt.dport = entry->ipv4_hnapt.new_sport;
        key.ipv4_hnapt.is_udp = opt->is_udp;

    }
    else if (opt->pkt_type == IPV6_ROUTING) {
        if (_hwnat_info.features & NAT_IPV6) {
            key.ipv6_routing.sip0 = entry->ipv6_5t_route.ipv6_dip0;
            key.ipv6_routing.sip1 = entry->ipv6_5t_route.ipv6_dip1;
            key.ipv6_routing.sip2 = entry->ipv6_5t_route.ipv6_dip2;
            key.ipv6_routing.sip3 = entry->ipv6_5t_route.ipv6_dip3;
            key.ipv6_routing.dip0 = entry->ipv6_5t_route.ipv6_sip0;
            key.ipv6_routing.dip1 = entry->ipv6_5t_route.ipv6_sip1;
            key.ipv6_routing.dip2 = entry->ipv6_5t_route.ipv6_sip2;
            key.ipv6_routing.dip3 = entry->ipv6_5t_route.ipv6_sip3;
            key.ipv6_routing.sport = entry->ipv6_5t_route.dport;
            key.ipv6_routing.dport = entry->ipv6_5t_route.sport;
            key.ipv6_routing.is_udp=opt->is_udp;
        }
    }
    entry = NULL;
    key.pkt_type = opt->pkt_type;
    hash_index = MDrv_HWNAT_Get_Mib_Entry_Idx(&key, entry);
    if (_hwnat_info.debug_level >= 1) {
        HWNAT_MSG_DBG("reply entry idx = %d\n", hash_index);
    }
    return hash_index;
}


static void MDrv_HWNAT_IOCTL_Dump_Dram_Entry(uint32_t entry_num)
{
    struct mib_entry *mib_entry = NULL;

    if (_hwnat_info.features & PPE_MIB) {
        mib_entry = MIB_ENTRY_BASE_BY_IDX(_hwnat_info.ppe_mib_base, entry_num);

        HWNAT_MSG_MUST("***********DRAM PPE Entry = %d*********\n", entry_num);
        HWNAT_MSG_MUST("PpeMibBase = %p\n", _hwnat_info.ppe_mib_base);
        HWNAT_MSG_MUST("DRAM Packet_CNT H = %u\n", mib_entry->pkt_cnt_h);
        HWNAT_MSG_MUST("DRAM Packet_CNT L = %u\n", mib_entry->pkt_cnt_l);
        HWNAT_MSG_MUST("DRAM Byte_CNT H = %u\n", mib_entry->byt_cnt_h);
        HWNAT_MSG_MUST("DRAM Byte_CNT L = %u\n", mib_entry->byt_cnt_l);
    }

}

static void MDrv_HWNAT_IOCTL_Dump_Entry(unsigned int entry_num, unsigned long *pkt_cnt, unsigned long *byte_cnt)
{
    MS_U64 bytes = 0;
    MS_U64 pkts = 0;

    MHal_HWNAT_Get_Mib_Info(entry_num, &bytes, &pkts);
    HWNAT_MSG_MUST("************PPE Entry = %d ************\n", entry_num);
    HWNAT_MSG_MUST("Packet Cnt = %llu\n", pkts);
    HWNAT_MSG_MUST("Byte Cnt  = %llu\n", bytes);
    *pkt_cnt = pkts;
    *byte_cnt = bytes;
}


int MDrv_HWNAT_Get_Ppe_Mib_Info(struct hwnat_tuple *opt, unsigned long *tx_pkt_cnt, unsigned long *tx_byte_cnt,
          unsigned long *rx_pkt_cnt, unsigned long *rx_byte_cnt)
{
    struct foe_pri_key key;
    struct foe_entry *entry = NULL;
    int32_t hash_index;
    int32_t rply_idx;
    /*HWNAT_MSG_DBG("sip = %x, dip=%x, sp=%d, dp=%d\n", opt->ing_sipv4, opt->ing_dipv4, opt->ing_sp, opt->ing_dp);*/
    if ((opt->pkt_type) == IPV4_NAPT) {
        key.ipv4_hnapt.sip=opt->ing_sipv4;
        key.ipv4_hnapt.dip=opt->ing_dipv4;
        key.ipv4_hnapt.sport=opt->ing_sp;
        key.ipv4_hnapt.dport=opt->ing_dp;
        key.ipv4_hnapt.is_udp=opt->is_udp;
    } else if ((opt->pkt_type) == IPV6_ROUTING) {
        key.ipv6_routing.sip0=opt->ing_sipv6_0;
        key.ipv6_routing.sip1=opt->ing_sipv6_1;
        key.ipv6_routing.sip2=opt->ing_sipv6_2;
        key.ipv6_routing.sip3=opt->ing_sipv6_3;
        key.ipv6_routing.dip0=opt->ing_dipv6_0;
        key.ipv6_routing.dip1=opt->ing_dipv6_1;
        key.ipv6_routing.dip2=opt->ing_dipv6_2;
        key.ipv6_routing.dip3=opt->ing_dipv6_3;
        key.ipv6_routing.sport=opt->ing_sp;
        key.ipv6_routing.dport=opt->ing_dp;
        key.ipv6_routing.is_udp=opt->is_udp;
    }

    key.pkt_type = opt->pkt_type;
    hash_index = MDrv_HWNAT_Get_Mib_Entry_Idx(&key, entry);

    if(hash_index != -1) {
        MDrv_HWNAT_IOCTL_Dump_Entry(hash_index, tx_pkt_cnt, tx_byte_cnt);
        rply_idx = MDrv_HWNAT_Reply_Ppe_Entry_Idx(opt, hash_index);
        if(rply_idx != -1) {
            MDrv_HWNAT_IOCTL_Dump_Entry(rply_idx, rx_pkt_cnt, rx_byte_cnt);
        } else if (rply_idx == -1) {
            *rx_pkt_cnt = 0;
            *rx_byte_cnt = 0;
        }
        return HWNAT_SUCCESS;
    }

    return HWNAT_FAIL;

}
EXPORT_SYMBOL(MDrv_HWNAT_Get_Ppe_Mib_Info);




static int32_t MDrv_HWNAT_IOCTL_Get_Ac_Info(struct hwnat_ac_args *opt3)
{
    extern struct hwnat_ac_args ac_info[64];
    struct mhal_hwnat_ac_info info;
    unsigned int ag_idx = 0;
    ag_idx = opt3->ag_index;
    if (ag_idx > 63)
        return HWNAT_FAIL;
    memset(&info, 0, sizeof(struct mhal_hwnat_ac_info));
    MHal_HWNAT_Get_Ac_Info(&info);
    if (_hwnat_info.features & ACCNT_MAINTAINER) {
        ac_info[ag_idx].ag_byte_cnt += info.bytes;
        ac_info[ag_idx].ag_pkt_cnt += info.pkts;
        opt3->ag_byte_cnt = ac_info[ag_idx].ag_byte_cnt;
        opt3->ag_pkt_cnt = ac_info[ag_idx].ag_pkt_cnt;
    }
    else {
        opt3->ag_byte_cnt = info.bytes;
        opt3->ag_pkt_cnt = info.pkts;
    }
    return HWNAT_SUCCESS;
}




static int MDrv_HWNAT_IOCTL_Clear_All_Entries(void)
{
    u32 foe_tbl_size;
    MHal_HWNAT_Set_Miss_Action(E_HWNAT_FOE_SEARCH_MISS_ONLY_FWD_CPU);
    foe_tbl_size = FOE_4TB_SIZ * sizeof(struct foe_entry);
    memset(_hwnat_info.ppe_foe_base, 0, foe_tbl_size);
    MHal_HWNAT_Enable_Cache();  /*clear HWNAT cache */
    MHal_HWNAT_Set_Miss_Action(E_HWNAT_FOE_SEARCH_MISS_FWD_CPU_BUILD_ENTRY);

    return HWNAT_SUCCESS;
}


void MDrv_HWNAT_Dump_Dev_Handler(void)
{
    int i;
    for (i = 0; i < MAX_IF_NUM; i++) {
        if (dst_port[i] != NULL)
            HWNAT_MSG_MUST("dst_port[%d] = %s\n", i, dst_port[i]->name);
    }
}

