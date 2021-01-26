/*
* MsDevice.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef __PRANA_DEVICE_H__
#define __PRANA_DEVICE_H__

#include "MsTypes.h"

#define DEVICE_NULL ((device_t)0)

typedef unsigned long device_t;

//
// device PM event
//
typedef enum
{
    E_EVT_SUSPEND   = 1,
    E_EVT_SHUTDOWN  = 2,
} eDEVEVT;

//
// poll types
//

#ifndef POLLIN
#define POLLIN      0x01
#endif

#ifndef POLLOUT
#define POLLOUT     0x02
#endif

#ifndef POLLHUP
#define POLLHUP     0x04
#endif

//
// device I/O operations
//
struct devio
{
    int     (*open) (device_t dev, int mode);
    int     (*close)(device_t dev);
//    int     (*read) (device_t dev, char *buf, size_t len, int off);
    int     (*read) (device_t dev, char *buf, MS_U32 len, int off);
//    int     (*write)(device_t dev, const char *buf, size_t len, int off);
    int     (*write)(device_t dev, const char *buf, MS_U32 len, int off);
    int     (*poll) (device_t dev, int types);
    int     (*ioctl)(device_t dev, int request, unsigned long arg);
    int     (*event)(eDEVEVT evt);
};


#define DEVICE_MAGIC    (('D' << 24) | ('E' << 16) | ('V' << 8) | 'I')

//
// device structure
//
#if 0
struct device
{
    int             magic;              // magic number
    int             refcnt;             // reference count
    int             flags;              // device characteristics
    struct devio    *devio;             // device i/o table
};
#endif

#define device_valid(dev) ((dev)->magic == DEVICE_MAGIC)
#endif /* __PRANA_DEVICE_H__ */

//MSTAR Module ID
/*******************************************************************************
!!!! WARNING NOT ALLOW MODIFY EXISTED VALUE !!!!
!!!! ONLY ALLOW ADD NEW VALUE AFTER END OF THIS LIST !!!!
*******************************************************************************/
#define MS_MODULE_HW                0x0000

#define MS_MODULE_CHIPTOP           0x0011
#define MS_MODULE_MIU               0x0012
#define MS_MODULE_WDT               0x003C
#define MS_MODULE_CFG               0x00F0                              // Chip hard configuration info
#define MS_MODULE_OTP               0x00F4
#define MS_MODULE_SPRAM             0x00F5

#define MS_MODULE_ACE               0x0101
#define MS_MODULE_AUDIO             0x0102
#define MS_MODULE_AVD               0x0103
#define MS_MODULE_BDMA              0x0104
#define MS_MODULE_DLC               0x0105
#define MS_MODULE_FLASH             0x0106
#define MS_MODULE_GE                0x0107
#define MS_MODULE_GOP               0x0108
#define MS_MODULE_HVD               0x0109
#define MS_MODULE_HWI2C             0x010a
#define MS_MODULE_IR                0x010b
#define MS_MODULE_IRQ               0x010c
#define MS_MODULE_JPD               0x010d
#define MS_MODULE_MBX               0x010e
#define MS_MODULE_MVD               0x010f
#define MS_MODULE_MVOP              0x0120
#define MS_MODULE_RVD               0x0122
#define MS_MODULE_SC                0x0123
#define MS_MODULE_TSP               0x0124
#define MS_MODULE_UART              0x0125
#define MS_MODULE_VPU               0x0126
#define MS_MODULE_XC                0x0127
#define MS_MODULE_PNL               0x0128
#define MS_MODULE_ISP               0x0129
#define MS_MODULE_PFSH              0x012a
#define MS_MODULE_GPIO              0x012b
#define MS_MODULE_PM                0x012c
#define MS_MODULE_VBI               0x012d
#define MS_MODULE_PCMCIA            0x012e
#define MS_MODULE_PIU               0x012f
#define MS_MODULE_MHEG5             0x0130
#define MS_MODULE_RTC_0             0x0131
#define MS_MODULE_RTC_2             0x0132
#define MS_MODULE_VIF               0x0133
#define MS_MODULE_MFE               0x0134
#define MS_MODULE_DIP               0x0135
#define MS_MODULE_RASP              0x0136
#define MS_MODULE_MMFILEIN          0x0137
#define MS_MODULE_MPIF              0x0138
#define MS_MODULE_GPD               0x0139
#define MS_MODULE_TSO               0x013a
#define MS_MODULE_IMI               0x013b    // for C3


#define MS_MODULE_SEM               0x02f0

#define MS_MODULE_CA                0x0300                              // CA reserved
#define MS_MODULE_NDS               0x0310                              // NDS
#define MS_MODULE_NSK               0x0311
#define MS_MODULE_CAM               0x0312
#define MS_MODULE_NGA               0x0313

#define MS_MODULE_PWM               0x1032
#define MS_MODULE_DMD               0x1033
#define MS_MODULE_PWS               0x1034
#define MS_MODULE_EFUSE             0x1035

#define MS_MODULE_FRC               0x2000                              // A5 only


