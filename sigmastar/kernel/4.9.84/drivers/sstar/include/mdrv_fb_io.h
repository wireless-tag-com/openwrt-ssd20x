/*
* mdrv_fb_io.h- Sigmastar
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


/**
 * \defgroup fb_group  FrameBuffer driver
 * \note
 *
 * sysfs Node: /sys/devices/soc0/soc/soc:gop/gopinfo
 *
 * sysfs R/W mode: R/W
 *
 * sysfs Usage & Description: print fb/gop current status and settings
 *
 * @{
 */


#ifndef _MDRV_FB_IO_H
#define _MDRV_FB_IO_H

//=============================================================================
// Includs
//=============================================================================

//=============================================================================
// IOCTRL defines
//=============================================================================

// Use 'F' as magic number. In Documentation/ioctl-number.txt says 'F' for all linux/fb.h

#define G3D_IOC_MAGIC_INFINITY  'F' ///< The Type definition of IOCTL for fb driver

// number before 20 was set at fb.h, skip them

/**
* Used to get framebuffer physical address, return type is unsigned long long.
*/
#define IOCTL_FB_GETFBPHYADDRESS    _IO(G3D_IOC_MAGIC_INFINITY,21)

/**
* Used to get framebuffer memory size, return type is unsigned long.
*/
#define IOCTL_FB_GETVIDEOMEMSIZE    _IO(G3D_IOC_MAGIC_INFINITY,22)

/**
* Used to get framebuffer support information, return type is FB_GOP_SUPINF_CONFIG.
*/
#define IOCTL_FB_GETFBSUPPORTINF    _IO(G3D_IOC_MAGIC_INFINITY,23)

/**
* Used to get gwin's parameters including starting point and ending point, return type is FB_GOP_GWIN_CONFIG.
*/
#define IOCTL_FB_GETGWININFO        _IO(G3D_IOC_MAGIC_INFINITY,24)

/**
* Used to set gwin's parameters including starting point and ending point, using type is FB_GOP_GWIN_CONFIG.
*/
#define IOCTL_FB_SETGWININFO        _IO(G3D_IOC_MAGIC_INFINITY,25)

/**
* Used to get gop's conditon, enable or disable, return type is unsigned char.
*/
#define IOCTL_FB_GETENABLEGOP       _IO(G3D_IOC_MAGIC_INFINITY,26)

/**
* Used to set gop's conditon, enable or disable, using type is unsigned char.
*/
#define IOCTL_FB_SETENABLEGOP       _IO(G3D_IOC_MAGIC_INFINITY,27)

/**
* Used to get alpha blending conditon including disable or constant_alpha or pixel_alpha, return type is FB_GOP_ALPHA_CONFIG.
*/
#define IOCTL_FB_GETALPHA           _IO(G3D_IOC_MAGIC_INFINITY,28)

/**
* Used to set alpha blending conditon including disable or constant_alpha or pixel_alpha, using type is FB_GOP_ALPHA_CONFIG.
*/
#define IOCTL_FB_SETALPHA           _IO(G3D_IOC_MAGIC_INFINITY,29)

/**
* Used to get colorkey information, return type is FB_GOP_COLORKEY_CONFIG.
*/
#define IOCTL_FB_GETCOLORKEY        _IO(G3D_IOC_MAGIC_INFINITY,30)

/**
* Used to set colorkey information, using type is FB_GOP_COLORKEY_CONFIG.
*/
#define IOCTL_FB_SETCOLORKEY        _IO(G3D_IOC_MAGIC_INFINITY,31)

/**
* Used to use system imageblit, using type is fb_image, which is defined in "linux/fb.h".
*/
#define IOCTL_FB_IMAGEBLIT          _IO(G3D_IOC_MAGIC_INFINITY,32)

/**
* Used to set palette, using type is FB_GOP_PaletteEntry.
*/
#define IOCTL_FB_SETPALETTE         _IO(G3D_IOC_MAGIC_INFINITY,33)

/**
* Used to get settings, include memory width & height, display width & height, buffer number and stretch ratio.
*/
#define IOCTL_FB_GETGENERALCONFIG   _IO(G3D_IOC_MAGIC_INFINITY,34)

/**
* Used to set settings, include memory width & height, display width & height, buffer number and stretch ratio.
*/
#define IOCTL_FB_SETGENERALCONFIG   _IO(G3D_IOC_MAGIC_INFINITY,35)

/**
* Used to set gop's inverse color, enable or disable, using type is unsigned char.
*/
#define IOCTL_FB_SETENABLEINVCOLOR  _IO(G3D_IOC_MAGIC_INFINITY,36)

/**
* Used to set AE configurations, using type is FB_GOP_INVCOLOR_AE_CONFIG.
*/
#define IOCTL_FB_SETAECONFIG        _IO(G3D_IOC_MAGIC_INFINITY,37)

/**
* Used to set Y threshlod to do inverse color, using type is unsigned long.
*/
#define IOCTL_FB_SETYTHRES          _IO(G3D_IOC_MAGIC_INFINITY,38)

/**
* Used to set Scaler configurations, using type is FB_GOP_INVCOLOR_SCALER_CONFIG.
*/
#define IOCTL_FB_SETSCALERCONFIG    _IO(G3D_IOC_MAGIC_INFINITY,39)

/**
* Used to trigger driver to update AE information from ISP driver and return value to user, using type is FB_GOP_INVCOLOR_AE_CONFIG.
*/
#define IOCTL_FB_AUTOUPDATEAE       _IO(G3D_IOC_MAGIC_INFINITY,40)

/**
* Used to set sw inverse table, using type is FB_GOP_SW_INV_TABLE.
*/
#define IOCTL_FB_SWINVTABLE       _IO(G3D_IOC_MAGIC_INFINITY,41)


#endif //MDRV_FB_IO_H


/** @} */ // end of fb_group
