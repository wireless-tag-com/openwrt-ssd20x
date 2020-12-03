/*
* mdrv_sata_io.h- Sigmastar
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
// @file   mdrv_sca_io.h
// @brief  GFlip KMD Driver Interface
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MDRV_SATA_IO_H
#define _MDRV_SATA_IO_H

//=============================================================================
// Includs
//=============================================================================

//=============================================================================
// Defines
//=============================================================================
// library information
#define MSIF_SATA_LIB_CODE               {'S','A','T','\0'}
#define MSIF_SATA_BUILDNUM               {'_','0','1','\0'}
#define MSIF_SATA_LIBVER                 (2)
#define MSIF_SATA_CHANGELIST             (677450)

//IO Ctrl defines:

#define IOCTL_SATA_SET_LOOPBACK_TEST_NR             (0)
#define IOCTL_SATA_SET_CONFIG_NR               (1)
#define IOCTL_SATA_GET_INTERRUPT_STATUS_NR     (2)
#define IOCTL_SATA_SET_TX_TEST_HFTP_NR             (3)
#define IOCTL_SATA_SET_TX_TEST_MFTP_NR             (4)
#define IOCTL_SATA_SET_TX_TEST_LFTP_NR             (5)
#define IOCTL_SATA_SET_TX_TEST_LBP_NR             (6)
#define IOCTL_SATA_SET_TX_TEST_SSOP_NR             (7)

#define IOCTL_SATA_MAX_NR                      (8)


// use 'm' as magic number
#define IOCTL_SATA_MAGIC                   ('3')


#define IOCTL_SATA_SET_LOOPBACK_TEST             _IO(IOCTL_SATA_MAGIC,  IOCTL_SATA_SET_LOOPBACK_TEST_NR)
#define IOCTL_SATA_SET_CONFIG                _IO(IOCTL_SATA_MAGIC,  IOCTL_SATA_SET_CONFIG_NR)
#define IOCTL_SATA_GET_INTERRUPT_STATUS      _IO(IOCTL_SATA_MAGIC,  IOCTL_SATA_GET_INTERRUPT_STATUS_NR)
#define IOCTL_SATA_SET_TX_TEST_HFTP             _IO(IOCTL_SATA_MAGIC,  IOCTL_SATA_SET_TX_TEST_HFTP_NR)
#define IOCTL_SATA_SET_TX_TEST_MFTP             _IO(IOCTL_SATA_MAGIC,  IOCTL_SATA_SET_TX_TEST_MFTP_NR)
#define IOCTL_SATA_SET_TX_TEST_LFTP             _IO(IOCTL_SATA_MAGIC,  IOCTL_SATA_SET_TX_TEST_LFTP_NR)
#define IOCTL_SATA_SET_TX_TEST_LBP             _IO(IOCTL_SATA_MAGIC,  IOCTL_SATA_SET_TX_TEST_LBP_NR)
#define IOCTL_SATA_SET_TX_TEST_SSOP             _IO(IOCTL_SATA_MAGIC,  IOCTL_SATA_SET_TX_TEST_SSOP_NR)


#endif //_MDRV_GFLIP_IO_H
