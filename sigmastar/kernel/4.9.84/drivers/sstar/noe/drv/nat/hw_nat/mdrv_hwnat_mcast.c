/*
* mdrv_hwnat_mcast.c- Sigmastar
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
/// @file   MDRV_NOE_MCAST_TBL.h
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------


#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/if_vlan.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <linux/ipv6.h>



#include "mhal_hwnat.h"
#include "mdrv_hwnat_mcast.h"
#include "mdrv_hwnat_foe.h"
#include "mdrv_hwnat_util.h"
#include "mdrv_hwnat_log.h"


//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------


#define MCAST_UPDATE_EXISTING_ENTRY(mcast_h, mcast_l, dst_mac, mc_px_en, mc_px_qos_en, mc_qos_qid) \
{ \
    if(dst_mac[0]==0x1 && dst_mac[1]==0x00) \
        mcast_h->mc_mpre_sel = 0; \
    else if(dst_mac[0]==0x33 && dst_mac[1]==0x33) \
        mcast_h->mc_mpre_sel = 1; \
    else \
        return 0; \
    mcast_h->mc_px_en = mc_px_en; \
    mcast_h->mc_px_qos_en = mc_px_qos_en; \
    mcast_h->mc_qos_qid = mc_qos_qid & 0xf; \
    mcast_h->mc_qos_qid54 = (mc_qos_qid & 0x30) >> 4; \
}

#define MCAST_SET_VALID_ENTRY(mcast_h, mcast_l, vlan_id, dst_mac, mc_px_en, mc_px_qos_en, mc_qos_qid) \
{\
    if(dst_mac[0]==0x1 && dst_mac[1]==0x00) \
        mcast_h->mc_mpre_sel = 0; \
    else if(dst_mac[0]==0x33 && dst_mac[1]==0x33) \
        mcast_h->mc_mpre_sel = 1; \
    else \
        return 0; \
    mcast_h->mc_vid = vlan_id;\
    mcast_h->mc_px_en = mc_px_en;\
    mcast_h->mc_px_qos_en = mc_px_qos_en;\
    mcast_l->mc_mac_addr[3] = dst_mac[2];\
    mcast_l->mc_mac_addr[2] = dst_mac[3];\
    mcast_l->mc_mac_addr[1] = dst_mac[4];\
    mcast_l->mc_mac_addr[0] = dst_mac[5];\
    mcast_h->valid = 1; \
    mcast_h->mc_qos_qid = mc_qos_qid & 0xf; \
    mcast_h->mc_qos_qid54 = (mc_qos_qid & 0x30) >> 4;\
    return 1; \
}


#define MCAST_DELETE_ENTRY(mcast_h, mcast_l, mc_px_en, mc_px_qos_en) \
{ \
    mcast_h->mc_px_en &= ~mc_px_en; \
    mcast_h->mc_px_qos_en &= ~mc_px_qos_en; \
    if(mcast_h->mc_px_en == 0 && mcast_h->mc_px_qos_en == 0) { \
        mcast_h->valid = 0; \
        mcast_h->mc_vid = 0; \
        mcast_h->mc_qos_qid = 0; \
        mcast_h->mc_qos_qid54 = 0; \
        memset(&mcast_l->mc_mac_addr, 0, 4); \
    }\
}

#define MCAST_INVALIDATE_ENTRY(mcast_h, mcast_l) \
{ \
    mcast_h->mc_px_en = 0; \
    mcast_h->mc_px_qos_en = 0; \
    mcast_h->valid = 0; \
    mcast_h->mc_vid = 0; \
    mcast_h->mc_qos_qid = 0; \
    mcast_h->mc_mpre_sel = 0; \
    memset(&mcast_l->mc_mac_addr, 0, 4); \
}

//--------------------------------------------------------------------------------------------------
//  Local Variable
//--------------------------------------------------------------------------------------------------





static int32_t _MDrv_HWNAT_Mcast_Get_Entry(uint16_t vlan_id, uint8_t *dst_mac)
{
    int i=0;

    for(i=0;i<MAX_MCAST_ENTRY;i++) {
        if((GET_PPE_MCAST_H(i)->mc_vid == vlan_id ) &&
            GET_PPE_MCAST_L(i)->mc_mac_addr[3] == dst_mac[2] &&
            GET_PPE_MCAST_L(i)->mc_mac_addr[2] == dst_mac[3] &&
            GET_PPE_MCAST_L(i)->mc_mac_addr[1] == dst_mac[4] &&
            GET_PPE_MCAST_L(i)->mc_mac_addr[0] == dst_mac[5]) {
            if(GET_PPE_MCAST_H(i)->mc_mpre_sel==0) {
                if(dst_mac[0]==0x1 && dst_mac[1]==0x00) {
                    return i;
                }
            }
            else if(GET_PPE_MCAST_H(i)->mc_mpre_sel==1) {
                if(dst_mac[0]==0x33 && dst_mac[1]==0x33) {
                    return i;
                }
            }
            else
                continue;
        }
    }
    for(i=0;i<MAX_MCAST_ENTRY16_63;i++) {
        if((GET_PPE_MCAST_H10(i)->mc_vid == vlan_id ) &&
            GET_PPE_MCAST_L10(i)->mc_mac_addr[3] == dst_mac[2] &&
            GET_PPE_MCAST_L10(i)->mc_mac_addr[2] == dst_mac[3] &&
            GET_PPE_MCAST_L10(i)->mc_mac_addr[1] == dst_mac[4] &&
            GET_PPE_MCAST_L10(i)->mc_mac_addr[0] == dst_mac[5]) {
            if(GET_PPE_MCAST_H10(i)->mc_mpre_sel==0) {
                if(dst_mac[0]==0x1 && dst_mac[1]==0x00) {
                    return (i + 16);
                }
            }
            else if(GET_PPE_MCAST_H10(i)->mc_mpre_sel==1) {
                if(dst_mac[0]==0x33 && dst_mac[1]==0x33) {
                    return (i + 16);
                }
            }
            else
                continue;
        }
    }
    return -1;
}




int MDrv_HWNAT_Mcast_Insert_Entry(uint16_t vlan_id, uint8_t *dst_mac, uint8_t mc_px_en, uint8_t mc_px_qos_en, uint8_t mc_qos_qid)
{
    int i=0;
    int entry_num;
    ppe_mcast_h *mcast_h;
    ppe_mcast_l *mcast_l;

    NAT_PRINT("[%s][%d]: vid=%x mac=%x:%x:%x:%x:%x:%x mc_px_en=%x mc_px_qos_en=%x, mc_qos_qid=%d \n",
        __FUNCTION__, __LINE__, vlan_id,
        dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5],
        mc_px_en, mc_px_qos_en,mc_qos_qid);

    if((entry_num = _MDrv_HWNAT_Mcast_Get_Entry(vlan_id, dst_mac)) >= 0) {
        MCAST_PRINT("update exist entry %d\n", entry_num);
        if(entry_num < MAX_MCAST_ENTRY) {
            mcast_h = GET_PPE_MCAST_H(entry_num);
            mcast_l = GET_PPE_MCAST_L(entry_num);
            MCAST_UPDATE_EXISTING_ENTRY(mcast_h, mcast_l, dst_mac, mc_px_en, mc_px_qos_en, mc_qos_qid);
        }
        else {
            mcast_h = GET_PPE_MCAST_H10(entry_num - 16);
            mcast_l = GET_PPE_MCAST_L10(entry_num - 16);
            MCAST_UPDATE_EXISTING_ENTRY(mcast_h, mcast_l, dst_mac, mc_px_en, mc_px_qos_en, mc_qos_qid);
        }
        return 1;
    }
    else { //create new entry
        for(i = 0; i < MAX_MCAST_ENTRY; i++) { // entry0 ~ entry15
            mcast_h = GET_PPE_MCAST_H(i);
            mcast_l = GET_PPE_MCAST_L(i);
            if(mcast_h->valid == 0) {
                MCAST_SET_VALID_ENTRY(mcast_h, mcast_l, vlan_id, dst_mac, mc_px_en, mc_px_qos_en, mc_qos_qid);
            }
        }
        for(i = 0; i < MAX_MCAST_ENTRY16_63; i++) { // entry16 ~ entry63
            mcast_h = GET_PPE_MCAST_H10(i);
            mcast_l = GET_PPE_MCAST_L10(i);
            if(mcast_h->valid == 0) {
                MCAST_SET_VALID_ENTRY(mcast_h, mcast_l, vlan_id, dst_mac, mc_px_en, mc_px_qos_en, mc_qos_qid);
            }
        }
    }

    MCAST_PRINT("HNAT: Multicast Table is FULL!!\n");
    return 0;
}

int MDrv_HWNAT_Mcast_Update_Qid(uint16_t vlan_id, uint8_t *dst_mac, uint8_t mc_qos_qid)
{
    int entry_num;
    ppe_mcast_h *mcast_h;
    NAT_PRINT("%s: vid=%x mac=%x:%x:%x:%x:%x:%x mc_qos_qid=%d\n", __FUNCTION__, vlan_id,
            dst_mac[0],dst_mac[1],dst_mac[2],dst_mac[3],dst_mac[4],dst_mac[5], mc_qos_qid);
    //update exist entry
    if((entry_num = _MDrv_HWNAT_Mcast_Get_Entry(vlan_id, dst_mac)) >= 0) {
        if(entry_num <= 15)
            mcast_h = GET_PPE_MCAST_H(entry_num);
        else
            mcast_h = GET_PPE_MCAST_H10(entry_num - 16);

        if (mc_qos_qid < 16){
            mcast_h->mc_qos_qid = mc_qos_qid;
        }
        else if (mc_qos_qid > 15){
            mcast_h->mc_qos_qid = mc_qos_qid & 0xf;
            mcast_h->mc_qos_qid54 = (mc_qos_qid & 0x30) >> 4;
        }
        else {
            NAT_PRINT("Error qid = %d\n", mc_qos_qid);
            return 0;
        }


        return 1;
    }
    return 0;
}

/*
 * Return:
 *      0: entry found
 *      1: entry not found
 */
int MDrv_HWNAT_Mcast_Delete_Entry(uint16_t vlan_id, uint8_t *dst_mac, uint8_t mc_px_en, uint8_t mc_px_qos_en, uint8_t mc_qos_qid)
{
    int entry_num;
    ppe_mcast_h *mcast_h;
    ppe_mcast_l *mcast_l;
    NAT_PRINT("%s: vid=%x mac=%x:%x:%x:%x:%x:%x mc_px_en=%x mc_px_qos_en=%x mc_qos_qid=%d\n", __FUNCTION__, vlan_id, dst_mac[0],dst_mac[1],dst_mac[2],dst_mac[3],dst_mac[4],dst_mac[5], mc_px_en, mc_px_qos_en, mc_qos_qid);
    if((entry_num = _MDrv_HWNAT_Mcast_Get_Entry(vlan_id, dst_mac)) >= 0) {
        NAT_PRINT("entry_num = %d\n", entry_num);
        if (entry_num <= 15){
            mcast_h = GET_PPE_MCAST_H(entry_num);
            mcast_l = GET_PPE_MCAST_L(entry_num);
            MCAST_DELETE_ENTRY(mcast_h, mcast_l, mc_px_en, mc_px_qos_en);
        }
        else if (entry_num > 15){
            mcast_h = GET_PPE_MCAST_H10(entry_num - 16);
            mcast_l = GET_PPE_MCAST_L10(entry_num - 16);
            MCAST_DELETE_ENTRY(mcast_h, mcast_l, mc_px_en, mc_px_qos_en);
        }
        return 0;
    }
    else {
        NAT_PRINT("MDrv_HWNAT_Mcast_Delete_Entry fail: entry_number = %d\n", entry_num);
        return 1;
    }
}

void MDrv_HWNAT_Mcast_Dump(void)
{
    int i;
    ppe_mcast_h *mcast_h;
    ppe_mcast_l *mcast_l;
    NAT_PRINT("MAC | VID | PortMask | QosPortMask \n");
    for(i=0;i<MAX_MCAST_ENTRY;i++) {
        mcast_h = GET_PPE_MCAST_H(i);
        mcast_l = GET_PPE_MCAST_L(i);
        NAT_PRINT("%x:%x:%x:%x  %d  %c%c%c%c %c%c%c%c (QID=%d, mc_mpre_sel=%d)\n",
                mcast_l->mc_mac_addr[3],
                mcast_l->mc_mac_addr[2],
                mcast_l->mc_mac_addr[1],
                mcast_l->mc_mac_addr[0],
                mcast_h->mc_vid,
                (mcast_h->mc_px_en & 0x08)?'1':'-',
                (mcast_h->mc_px_en & 0x04)?'1':'-',
                (mcast_h->mc_px_en & 0x02)?'1':'-',
                (mcast_h->mc_px_en & 0x01)?'1':'-',
                (mcast_h->mc_px_qos_en & 0x08)?'1':'-',
                (mcast_h->mc_px_qos_en & 0x04)?'1':'-',
                (mcast_h->mc_px_qos_en & 0x02)?'1':'-',
                (mcast_h->mc_px_qos_en & 0x01)?'1':'-',
                 mcast_h->mc_qos_qid + ((mcast_h->mc_qos_qid54) << 4),
                 mcast_h->mc_mpre_sel);
    }
    for(i=0;i<MAX_MCAST_ENTRY16_63;i++) {
        mcast_h = GET_PPE_MCAST_H10(i);
        mcast_l = GET_PPE_MCAST_L10(i);
        NAT_PRINT("%x:%x:%x:%x  %d  %c%c%c%c %c%c%c%c (QID=%d, mc_mpre_sel=%d)\n",
                mcast_l->mc_mac_addr[3],
                mcast_l->mc_mac_addr[2],
                mcast_l->mc_mac_addr[1],
                mcast_l->mc_mac_addr[0],
                mcast_h->mc_vid,
                (mcast_h->mc_px_en & 0x08)?'1':'-',
                (mcast_h->mc_px_en & 0x04)?'1':'-',
                (mcast_h->mc_px_en & 0x02)?'1':'-',
                (mcast_h->mc_px_en & 0x01)?'1':'-',
                (mcast_h->mc_px_qos_en & 0x08)?'1':'-',
                (mcast_h->mc_px_qos_en & 0x04)?'1':'-',
                (mcast_h->mc_px_qos_en & 0x02)?'1':'-',
                (mcast_h->mc_px_qos_en & 0x01)?'1':'-',
                 mcast_h->mc_qos_qid + ((mcast_h->mc_qos_qid54) << 4),
                 mcast_h->mc_mpre_sel);
    }

}

void MDrv_HWNAT_Mcast_Delete_All(void)
{
    int i;
    ppe_mcast_h *mcast_h;
    ppe_mcast_l *mcast_l;
    for(i=0;i<MAX_MCAST_ENTRY;i++) { // entry0 ~ entry15
        mcast_h = GET_PPE_MCAST_H(i);
        mcast_l = GET_PPE_MCAST_L(i);
        MCAST_INVALIDATE_ENTRY(mcast_h, mcast_l);
    }
    for(i=0;i<MAX_MCAST_ENTRY16_63;i++) { // entry16 ~ entry63
        mcast_h = GET_PPE_MCAST_H10(i);
        mcast_l = GET_PPE_MCAST_L10(i);
        MCAST_INVALIDATE_ENTRY(mcast_h, mcast_l);
    }
}
