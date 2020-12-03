/*
* mdrv_noe_lro.h- Sigmastar
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
/// @file   MDRV_NOE_LRO.h
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_NOE_LRO_H_
#define _MDRV_NOE_LRO_H_

#if FE_HW_LRO
int MDrv_NOE_LRO_Init(struct net_device *dev);
int MDrv_NOE_LRO_Recv(struct net_device *dev, struct napi_struct *napi, int budget);
void MDrv_NOE_LRO_Deinit(struct net_device *dev);
#else
static inline int MDrv_NOE_LRO_Init(struct net_device *dev) {return -ENOMEM;}
static inline int MDrv_NOE_LRO_Recv(struct net_device *dev, struct napi_struct *napi, int budget) {return (budget + 1);}
static inline void MDrv_NOE_LRO_Deinit(struct net_device *dev) {}
#endif
#endif /* _MDRV_NOE_LRO_H_ */
