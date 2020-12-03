/*
* mdrv_hwnat_log.h- Sigmastar
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
#ifndef _MDRV_HWNAT_LOG_H_
#define _MDRV_HWNAT_LOG_H_

#define HWNAT_DBG 1

#if HWNAT_DBG
#define NAT_PRINT(fmt, args...) printk(fmt, ## args)
#else
#define NAT_PRINT(fmt, args...) { }
#endif


typedef enum {
    E_MDRV_HWNAT_MSG_CTRL_NONE    = 0,
    E_MDRV_HWNAT_MSG_CTRL_ERR     ,
    E_MDRV_HWNAT_MSG_CTRL_WARN    ,
    E_MDRV_HWNAT_MSG_CTRL_DBG     ,
    E_MDRV_HWNAT_MSG_CTRL_DUMP    ,
}EN_MDRV_HWNAT_MSG_CTRL;


#define  MDRV_HWNAT_DUMP_ALL (E_MDRV_HWNAT_MSG_CTRL_DBG)

#define MDRV_HWNAT_MSG(type, format , args...)    \
    do{                                         \
        if (MDrv_HWNAT_LOG_Get_Level() >= type)    \
        {                                       \
            printk(format , ##args  );          \
        } \
    }while(0);

#define HWNAT_MSG_ERR(format, args...)  MDRV_HWNAT_MSG(E_MDRV_HWNAT_MSG_CTRL_ERR, format, ##args)
#define HWNAT_MSG_WARN(format, args...)  MDRV_HWNAT_MSG(E_MDRV_HWNAT_MSG_CTRL_WARN, format, ##args)
#define HWNAT_MSG_DBG(format, args...)  MDRV_HWNAT_MSG(E_MDRV_HWNAT_MSG_CTRL_DBG, format, ##args)
#define HWNAT_MSG_DUMP(format, args...) MDRV_HWNAT_MSG(E_MDRV_HWNAT_MSG_CTRL_DUMP, format, ##args)
#define HWNAT_MSG_MUST(format, args...)  printk(format, ##args)

void MDrv_HWNAT_LOG_Set_Level(unsigned char level);
unsigned char MDrv_HWNAT_LOG_Get_Level(void);
void MDrv_HWNAT_LOG_Init(void *dev);
void MDrv_HWNAT_LOG_Dump_Skb(struct sk_buff* sk);
uint32_t MDrv_HWNAT_Dump_Skb(struct sk_buff *skb);
uint32_t MDrv_HWNAT_Dump_TxSkb(struct sk_buff *skb);
void FOE_INFO_DUMP_TAIL(struct sk_buff *skb);
void FOE_INFO_DUMP(struct sk_buff *skb);

#endif /* _MDRV_NOE_LOG_H_ */
