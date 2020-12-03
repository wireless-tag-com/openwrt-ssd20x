/*
* mdrv_sata_io_st.h- Sigmastar
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
///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2008-2009 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// ("MStar Confidential Information") by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// @file   mdrv_sata_io_st.h
// @brief  GFlip KMD Driver Interface
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_SATA_IO_ST_H
#define _MDRV_SATA_IO_ST_H

//=============================================================================
// enum
//=============================================================================

//=============================================================================
// struct
//=============================================================================
typedef struct
{
    unsigned short  u16PortNo;
    unsigned short  u16GenNo;
    int                        s32Result;
} stSata_Loopback_Test, *pstSata_Loopback_Test;

typedef struct
{
    unsigned short  u16PortNo;
    unsigned short  u16GenNo;
    unsigned int      u32SSCEnable;
    int                        s32Result;
} stSata_Tx_Test, *pstSata_Tx_Test;

typedef struct
{
    unsigned short x;
    unsigned short y;    ///<start y of the window
    unsigned short width;
    unsigned short height;
} SATA_WINDOW_TYPE, *PSATA_WINDOW_TYPE;

typedef struct
{
    unsigned char u8SATAW_Status;
} SATA_INTR_STATUS, *PSATA_INTR_STATUS;

//=============================================================================

//=============================================================================

#endif //_MDRV_GFLIP_IO_H
