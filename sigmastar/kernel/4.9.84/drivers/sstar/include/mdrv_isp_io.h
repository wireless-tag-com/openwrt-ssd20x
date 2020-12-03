/*
* mdrv_isp_io.h- Sigmastar
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
#ifndef _MDRV_ISP_IO_H
#define _MDRV_ISP_IO_H

//=============================================================================
// Includs
//=============================================================================

//=============================================================================
// Defines
//=============================================================================

#define ISP_IOCTL_MAGIC 'I'
#define IOCTL_ISP_CLOCK_CTL			_IO(ISP_IOCTL_MAGIC, 0x1)
#define IOCTL_ISP_IQ_RGBCCM         _IO(ISP_IOCTL_MAGIC, 0x2)
#define IOCTL_ISP_IQ_YUVCCM         _IO(ISP_IOCTL_MAGIC, 0x3)
#define IOCTL_ISP_GET_ISP_FLAG      _IO(ISP_IOCTL_MAGIC, 0x4)
#define IOCTL_ISP_GET_INFO          _IO(ISP_IOCTL_MAGIC, 0x5)
#define IOCTL_ISP_GET_MEM_INFO      _IO(ISP_IOCTL_MAGIC, 0x6)
#define IOCTL_ISP_GET_AE_IMG_INFO   _IO(ISP_IOCTL_MAGIC, 0x7)
#define IOCTL_ISP_SET_AE_DGAIN      _IO(ISP_IOCTL_MAGIC, 0x8)
#define IOCTL_ISP_SET_FIFO_MASK     _IO(ISP_IOCTL_MAGIC, 0x9)
#define IOCTL_ISP_UPDATE_AE_IMG_INFO   _IO(ISP_IOCTL_MAGIC, 0xa)
#define IOCTL_ISP_TRIGGER_WDMA      _IO(ISP_IOCTL_MAGIC, 0xb)
#define IOCTL_ISP_SKIP_FRAME        _IO(ISP_IOCTL_MAGIC, 0xc)
#define IOCTL_ISP_INIT              _IO(ISP_IOCTL_MAGIC, 0xd)
#endif //_MDRV_GFLIP_IO_H
