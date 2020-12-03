/*
* mdrv_msys_io.h- Sigmastar
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
#ifndef _MDRV_MSYS_IO_H_
#define _MDRV_MSYS_IO_H_

#define MSYS_IOCTL_MAGIC             'S'

#define IOCTL_MSYS_FIND_DMEM_BY_NAME        _IO(MSYS_IOCTL_MAGIC, 0x0F)
#define IOCTL_MSYS_REQUEST_DMEM             _IO(MSYS_IOCTL_MAGIC, 0x10)
#define IOCTL_MSYS_RELEASE_DMEM             _IO(MSYS_IOCTL_MAGIC, 0x11)
#define IOCTL_MSYS_FLUSH_CACHE              _IO(MSYS_IOCTL_MAGIC, 0x12)
#define IOCTL_MSYS_PHYS_TO_MIU              _IO(MSYS_IOCTL_MAGIC, 0x13)
#define IOCTL_MSYS_MIU_TO_PHYS              _IO(MSYS_IOCTL_MAGIC, 0x14)
#define IOCTL_MSYS_GET_RIU_MAP              _IO(MSYS_IOCTL_MAGIC, 0x15)
#define IOCTL_MSYS_FIX_DMEM                 _IO(MSYS_IOCTL_MAGIC, 0x16)
#define IOCTL_MSYS_UNFIX_DMEM               _IO(MSYS_IOCTL_MAGIC, 0x17)
#define IOCTL_MSYS_MIU_PROTECT              _IO(MSYS_IOCTL_MAGIC, 0x18)
#define IOCTL_MSYS_USER_TO_PHYSICAL         _IO(MSYS_IOCTL_MAGIC, 0x19)
#define IOCTL_MSYS_GET_SYSP_STRING          _IO(MSYS_IOCTL_MAGIC, 0x20)
#define IOCTL_MSYS_GET_DATAP_STRING         _IO(MSYS_IOCTL_MAGIC, 0x22)
#define IOCTL_MSYS_GET_PROPERTY_PATH        _IO(MSYS_IOCTL_MAGIC, 0x23)
#define IOCTL_MSYS_SET_PROPERTY_PATH        _IO(MSYS_IOCTL_MAGIC, 0x24)
#define IOCTL_MSYS_PRINT_PIU_TIMER_TICKS    _IO(MSYS_IOCTL_MAGIC, 0x30)
#define IOCTL_MSYS_GET_US_TICKS             _IO(MSYS_IOCTL_MAGIC, 0x31)
#define IOCTL_MSYS_GET_UDID                 _IO(MSYS_IOCTL_MAGIC, 0x32)
#define IOCTL_MSYS_BENCH_MEMORY             _IO(MSYS_IOCTL_MAGIC, 0x63)
#define IOCTL_MSYS_RESET_TO_UBOOT           _IO(MSYS_IOCTL_MAGIC, 0x77)
#define IOCTL_MSYS_READ_PM_TSENSOR          _IO(MSYS_IOCTL_MAGIC, 0x78)

#define IOCTL_MSYS_REQUEST_PROC_DEVICE      _IO(MSYS_IOCTL_MAGIC, 0x86)
#define IOCTL_MSYS_RELEASE_PROC_DEVICE      _IO(MSYS_IOCTL_MAGIC, 0x87)
#define IOCTL_MSYS_REQUEST_PROC_ATTRIBUTE   _IO(MSYS_IOCTL_MAGIC, 0x88)
#define IOCTL_MSYS_RELEASE_PROC_ATTRIBUTE   _IO(MSYS_IOCTL_MAGIC, 0x89)
#define IOCTL_MSYS_FLUSH_MEMORY             _IO(MSYS_IOCTL_MAGIC, 0x90)
//#define IOCTL_MSYS_ADMA			            _IO(MSYS_IOCTL_MAGIC, 0x91)
//#define IOCTL_MSYS_BDMA			            _IO(MSYS_IOCTL_MAGIC, 0x92)

#define IOCTL_MSYS_REQUEST_FREQUENCY        _IO(MSYS_IOCTL_MAGIC, 0x93)
#define IOCTL_MSYS_GET_CHIPVERSION          _IO(MSYS_IOCTL_MAGIC, 0x94)

#define IOCTL_SYS_MAXNR                     0xFF

#endif
