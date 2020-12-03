/*
* mdrv_video.h- Sigmastar
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

#ifndef _MDRV_VIDEO_H_
#define _MDRV_VIDEO_H_

/* 
 * NOTE:
 * This is old version naming of MFE driver. It wiil be deprecated in the future.
 *
 */

/* definition of structures */

#ifndef _MDRV_MMFE_IO_H_

#include <mdrv_mfe_io.h>

#endif//_MDRV_MMFE_IO_H_

typedef mmfe_reqbuf     mvideo_reqbuf;
typedef mmfe_buffer     mvideo_buffer;
typedef mmfe_params     mvideo_params;
typedef mmfe_control    mvideo_control;

/* definition of ioctl codes */

#define MVIDIOC_S_PARM      IOCTL_MFE_S_PARM
#define MVIDIOC_G_PARM      IOCTL_MFE_G_PARM
#define MVIDIOC_S_CTRL      IOCTL_MFE_S_CTRL
#define MVIDIOC_G_CTRL      IOCTL_MFE_G_CTRL
#define MVIDIOC_STREAMON    IOCTL_MFE_STREAMON
#define MVIDIOC_STREAMOFF   IOCTL_MFE_STREAMOFF
#define MVIDIOC_REQBUF      IOCTL_MFE_REQBUF
#define MVIDIOC_S_PICT      IOCTL_MFE_S_PICT
#define MVIDIOC_G_BITS      IOCTL_MFE_G_BITS
#define MVIDIOC_FLUSH       IOCTL_MFE_FLUSH

#endif//_MDRV_VIDEO_H_

