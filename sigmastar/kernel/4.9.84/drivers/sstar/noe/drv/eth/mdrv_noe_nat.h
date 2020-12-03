/*
* mdrv_noe_nat.h- Sigmastar
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
/// @file   MDRV_NOE.h
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_NOE_NAT_H_
#define _MDRV_NOE_NAT_H_

#ifdef CONFIG_NOE_NAT_HW
//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------

#include "mdrv_hwnat.h"
//--------------------------------------------------------------------------------------------------
//  Constant definition
//--------------------------------------------------------------------------------------------------
extern int (*noe_nat_hook_rx)(struct sk_buff *skb);
extern int (*noe_nat_hook_tx)(struct sk_buff *skb, int gmac_no);
#define  NOE_HWNAT_HOOK_RX(skb)             noe_nat_hook_rx(skb)
#define  NOE_HWNAT_HOOK_TX(skb,no)          noe_nat_hook_tx(skb, no)
#define  IS_VALID_NOE_HWNAT_HOOK_RX         (noe_nat_hook_rx != NULL)
#define  IS_VALID_NOE_HWNAT_HOOK_TX         (noe_nat_hook_tx != NULL)
#define  IS_NOT_VALID_NOE_HWNAT_HOOK_RX         (noe_nat_hook_rx == NULL)
#define  IS_NOT_VALID_NOE_HWNAT_HOOK_TX         (noe_nat_hook_tx == NULL)
#define  NOE_HWNAT_HOOK_RX_INIT             (noe_nat_hook_rx = NULL)
#define  NOE_HWNAT_HOOK_TX_INIT             (noe_nat_hook_tx = NULL)

//-------------------------------------------------------------------------------------------------
//  Data structure
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Functions
//-------------------------------------------------------------------------------------------------

#endif /* CONFIG_NOE_NAT_HW  */
#endif /* _MDRV_NOE_H_ */
