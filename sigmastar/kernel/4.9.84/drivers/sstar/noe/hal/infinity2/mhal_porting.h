/*
* mhal_porting.h- Sigmastar
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
/// @file   Mhal_porting.h
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef MHAL_NOE_PORTING_H
#define MHAL_NOE_PORTING_H

#include "mdrv_types.h"

#ifndef MS_BOOL
#define MS_BOOL BOOL
#endif
#ifndef MS_PHYADDR
typedef size_t                      MS_PHYADDR;
#endif
#ifndef MS_U8
#define MS_U8 U8
#endif
#ifndef MS_U16
#define MS_U16 U16
#endif
#ifndef MS_U32
#define MS_U32 U32
#endif
#ifndef MS_U64
#define MS_U64 U64
#endif


#endif
