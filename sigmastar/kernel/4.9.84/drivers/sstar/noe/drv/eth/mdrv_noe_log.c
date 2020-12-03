/*
* mdrv_noe_log.c- Sigmastar
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
/// @file   MDRV_NOE_LOG.c
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include "mdrv_noe.h"

//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------

#define SKB_DUMP_LAYER_II  0
#if SKB_DUMP_LAYER_II
#define SKB_START_OFFSET (-14) /* dump Layer II*/
#else
#define SKB_START_OFFSET (0)  /* dump Layer III*/
#endif

#define SKB_END_OFFSET  (32) // (40)



//--------------------------------------------------------------------------------------------------
//  Local Variable
//--------------------------------------------------------------------------------------------------

static struct net_device *_noe_dev = NULL;

void MDrv_NOE_LOG_Set_Level(unsigned char level)
{
    struct END_DEVICE *ei_local = NULL;

    if (_noe_dev == NULL)
        return;

    ei_local = netdev_priv(_noe_dev);

    if (ei_local->log_level == level)
        return;

    ei_local->log_level = level;
    NOE_MSG_DBG("Set Log Level = 0x%x \n", ei_local->log_level);
    if (level >= E_MDRV_NOE_MSG_CTRL_DUMP) {
        MHal_NOE_Set_DBG(E_NOE_HAL_LOG_DBG);
    }
    else {
        MHal_NOE_Set_DBG(E_NOE_HAL_LOG_NONE);
    }

}

unsigned char MDrv_NOE_LOG_Get_Level(void)
{
    struct END_DEVICE *ei_local = NULL;

    if (_noe_dev == NULL)
        return E_MDRV_NOE_MSG_CTRL_NONE;

    ei_local = netdev_priv(_noe_dev);

    return ei_local->log_level;
}

void MDrv_NOE_LOG_Init(struct net_device *dev)
{
    _noe_dev = dev;
}

void MDrv_NOE_LOG_Dump_Skb(struct sk_buff* sk)
{
    int num = sk->len; //(sk->len < SKB_END_OFFSET)? sk->len : SKB_END_OFFSET;
    unsigned char *i;
    unsigned int j = 0;
    NOE_MSG_DUMP("[%s][%d] =========> \n",__FUNCTION__,__LINE__);
#if 0
    NOE_MSG_DUMP("skb_dump: from %s with len %d (%d) headroom=%d tailroom=%d\n",
            sk->dev? sk->dev->name:"ip stack", sk->len, sk->truesize,
            skb_headroom(sk),skb_tailroom(sk));
#endif
    NOE_MSG_DUMP("%s: head = 0x%p, data = 0x%p with len %d (%d)\n",
            sk->dev? sk->dev->name:" ",
            sk->head, sk->data, sk->len, sk->truesize);
    for(i = sk->data + SKB_START_OFFSET; i < sk->data + num; i++) {

        if ((j % 16) == 8)
            NOE_MSG_DUMP("\t");
        if ((j % 16) == 0)
            NOE_MSG_DUMP("\n");

        NOE_MSG_DUMP("%02x ", *(sk->data + j));
        j++;
    }
    NOE_MSG_DUMP("\n");
}

