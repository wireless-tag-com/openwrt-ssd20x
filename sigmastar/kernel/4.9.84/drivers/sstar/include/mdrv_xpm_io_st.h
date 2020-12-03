/*
* mdrv_xpm_io_st.h- Sigmastar
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
#ifndef _MDRV_XPM_IO_ST_H_
#define _MDRV_XPM_IO_ST_H_


typedef enum
{
	XPM_STATE_SUSPENDING		=0x01,
	XPM_STATE_SUSPENDED			=0x02,
	XPM_STATE_SUSPEND_ABORT		=0x04,
	XPM_STATE_WAKEUPED			=0x08,
	XPM_STATE_END			=0xFFFFFFFF
}EN_XPM_STATE;


typedef enum
{
	XPM_SOURCE_ACTIVE		=0x10,
	XPM_SOURCE_STANDBY		=0x20,
	XPM_SOURCE_STATUS_END=0xFFFFFFFF,
}EN_XPM_SOURCE_STATUS;

#endif
