/*
* mdrv_noe_log.h- Sigmastar
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
/// @file   MDRV_NOE_LOG.h
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _MDRV_NOE_LOG_H_
#define _MDRV_NOE_LOG_H_


typedef enum {
    E_MDRV_NOE_MSG_CTRL_NONE    = 0,
    E_MDRV_NOE_MSG_CTRL_ERR,
    E_MDRV_NOE_MSG_CTRL_DBG,
    E_MDRV_NOE_MSG_CTRL_DUMP,
}EN_MDRV_NOE_MSG_CTRL;


#define MDRV_NOE_MSG(type, format , args...)    \
    do{                                         \
        if (MDrv_NOE_LOG_Get_Level() >= type)    \
        {                                       \
            printk(format , ##args  );          \
        } \
    }while(0);

#define NOE_MSG_ERR(format, args...)  MDRV_NOE_MSG(E_MDRV_NOE_MSG_CTRL_ERR, format, ##args)
#define NOE_MSG_DBG(format, args...)  MDRV_NOE_MSG(E_MDRV_NOE_MSG_CTRL_DBG, format, ##args)
#define NOE_MSG_DUMP(format, args...) MDRV_NOE_MSG(E_MDRV_NOE_MSG_CTRL_DUMP, format, ##args)
#define NOE_MSG_MUST(format, args...) printk(format, ##args)


void MDrv_NOE_LOG_Set_Level(unsigned char level);
unsigned char MDrv_NOE_LOG_Get_Level(void);
void MDrv_NOE_LOG_Init(struct net_device *dev);
void MDrv_NOE_LOG_Dump_Skb(struct sk_buff* sk);
#endif /* _MDRV_NOE_LOG_H_ */
