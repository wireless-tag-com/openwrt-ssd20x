/*
* mdrv_device_id.h- Sigmastar
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
#ifndef __MDRV_DEVICE_ID_H__
#define __MDRV_DEVICE_ID_H__

typedef enum
{
CHIP_REVISION_U01=0x00,
CHIP_REVISION_U02,
CHIP_REVISION_U03,
CHIP_REVISION_U04,
CHIP_REVISION_U05,
CHIP_REVISION_U06,
CHIP_REVISION_U07,
CHIP_REVISION_U08,
CHIP_REVISION_U09,
CHIP_REVISION_U0A,
CHIP_REVISION_U0B,
CHIP_REVISION_U0C,
CHIP_REVISION_U0D,
CHIP_REVISION_U0E,
CHIP_REVISION_U0F,
CHIP_REVISION_U10,
CHIP_REVISION_U11,
CHIP_REVISION_U12,
CHIP_REVISION_U13,
CHIP_REVISION_U14,
CHIP_REVISION_U15,
CHIP_REVISION_U16,
CHIP_REVISION_U17,
CHIP_REVISION_U18,
CHIP_REVISION_U19,
CHIP_REVISION_U1A,
CHIP_REVISION_U1B,
CHIP_REVISION_U1C,
CHIP_REVISION_U1D,
CHIP_REVISION_U1E,
CHIP_REVISION_U1F,
CHIP_REVISION_END=0xFF

}MS_CHIP_REVISION;



typedef enum
{
DEVICE_ID_CEDRIC    =           (0x50),
DEVICE_ID_CHICAGO   =           (0x70),
DEVICE_ID_INFINITY  =           (0xAE),
DEVICE_ID_INFINITY5 =           (0xED),
DEVICE_ID_END=0xFFFFFFFF
}MS_DEVICE_ID;


#define KERNEL_310_VER      "3.10"
#define KERNEL_318_VER      "3.18"


#endif
