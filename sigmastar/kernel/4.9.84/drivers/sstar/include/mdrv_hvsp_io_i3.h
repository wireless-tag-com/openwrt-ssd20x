/*
* mdrv_hvsp_io_i3.h- Sigmastar
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
 * \defgroup hvsp_group  HVSP driver
 * \note
 *
 * sysfs Node: /sys/devices/platform/mhvsp1.0/clk
 *
 * sysfs R/W mode: R/W
 *
 * sysfs Usage & Description: R:print explain W:control clk by explain.
 *
 * sysfs Node: /sys/devices/platform/mhvsp1.0/ckcrop
 *
 * sysfs R/W mode: R/W
 *
 * sysfs Usage & Description: R:print vsync count,err times W: if 0 close ptgen ,if echo 1 open check crop ,echo 2 open check hvsp and dma count
 *
 * sysfs Node: /sys/devices/platform/mhvsp1.0/ptgen
 *
 * sysfs R/W mode: R/W
 *
 * sysfs Usage & Description: R:print explain W: if 0 close ptgen ,if echo 1 open static ptgen ,echo 2 open dynamic ptgen,echo 3 open scl time gen
 *
 * @{
 */

#ifndef _MDRV_HVSP_IO_H
#define _MDRV_HVSP_IO_H

//=============================================================================
// Includs
//=============================================================================


//=============================================================================
// IOCTRL defines
//=============================================================================

#define IOCTL_HVSP_SET_IN_CONFIG_NR           (0)   ///< The IOCTL NR definition,IOCTL_HVSP_SET_IN_CONFIG
#define IOCTL_HVSP_SET_OUT_CONFIG_NR          (1)   ///< The IOCTL NR definition,IOCTL_HVSP_SET_OUT_CONFIG
#define IOCTL_HVSP_SET_SCALING_CONFIG_NR      (2)   ///< The IOCTL NR definition,IOCTL_HVSP_SET_SCALING_CONFIG
#define IOCTL_HVSP_REQ_MEM_CONFIG_NR          (3)   ///< The IOCTL NR definition,IOCTL_HVSP_REQ_MEM_CONFIG
#define IOCTL_HVSP_SET_MISC_CONFIG_NR         (4)   ///< The IOCTL NR definition,IOCTL_HVSP_SET_MISC_CONFIG
#define IOCTL_HVSP_SET_POST_CROP_CONFIG_NR    (5)   ///< The IOCTL NR definition,IOCTL_HVSP_SET_POST_CROP_CONFIG
#define IOCTL_HVSP_GET_PRIVATE_ID_CONFIG_NR   (6)   ///< The IOCTL NR definition,IOCTL_HVSP_GET_PRIVATE_ID_CONFIG
#define IOCTL_HVSP_GET_INFORM_CONFIG_NR       (7)   ///< The IOCTL NR definition,IOCTL_HVSP_GET_INFORM_CONFIG
#define IOCTL_HVSP_RELEASE_MEM_CONFIG_NR      (8)   ///< The IOCTL NR definition,IOCTL_HVSP_RELEASE_MEM_CONFIG
#define IOCTL_HVSP_SET_OSD_CONFIG_NR          (9)   ///< The IOCTL NR definition,IOCTL_HVSP_SET_OSD_CONFIG
#define IOCTL_HVSP_SET_FB_MANAGE_CONFIG_NR    (10)  ///< The IOCTL NR definition,IOCTL_HVSP_SET_FB_MANAGE_CONFIG
#define IOCTL_HVSP_SET_PRIMASK_CONFIG_NR      (11)  ///< The IOCTL NR definition, IOCLT_HVSP_GET_VERSION_CONFIG_NR
#define IOCTL_HVSP_PRIMASK_TRIGGER_CONFIG_NR  (12)  ///< The IOCTL NR definition, IOCLT_HVSP_GET_VERSION_CONFIG_NR
#define IOCTL_HVSP_SET_VTRACK_CONFIG_NR       (13) ///< The IOCTL NR definition,IOCTL_VIP_SET_VTRACK_CONFIG
#define IOCTL_HVSP_SET_VTRACK_ONOFF_CONFIG_NR (14) ///< The IOCTL NR definition,IOCTL_VIP_SET_VTRACK_ONOFF_CONFIG
#define IOCLT_HVSP_GET_VERSION_CONFIG_NR      (15)  ///< The IOCTL NR definition, IOCLT_HVSP_GET_VERSION_CONFIG_NR
#define IOCTL_HVSP_MAX_NR                     (IOCLT_HVSP_GET_VERSION_CONFIG_NR+1)  ///< The Max IOCTL NR for hvsp driver


// use 'm' as magic number
#define IOCTL_HVSP_MAGIC                       ('1')///< The Type definition of IOCTL for hvsp driver
/**
* Used to set Input MUX,capture window, use ST_IOCTL_HVSP_INPUT_CONFIG.
*/
#define IOCTL_HVSP_SET_IN_CONFIG              _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_SET_IN_CONFIG_NR)
/**
* Used to set output configuration, use ST_IOCTL_HVSP_OUTPUT_CONFIG.
*/
#define IOCTL_HVSP_SET_OUT_CONFIG             _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_SET_OUT_CONFIG_NR)
/**
* Used to set HVSP configuration and set post crop ,line,frame buffer size, use ST_IOCTL_HVSP_SCALING_CONFIG.
*/
#define IOCTL_HVSP_SET_SCALING_CONFIG         _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_SET_SCALING_CONFIG_NR)
/**
* Used to allocate DNR buffer and set framebuffer configuration, use ST_IOCTL_HVSP_REQ_MEM_CONFIG.
*/
#define IOCTL_HVSP_REQ_MEM_CONFIG             _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_REQ_MEM_CONFIG_NR)
/**
* Used to set register without interface, use ST_IOCTL_HVSP_MISC_CONFIG.
*/
#define IOCTL_HVSP_SET_MISC_CONFIG            _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_SET_MISC_CONFIG_NR)
/**
* Used to set post crop if need, use ST_IOCTL_HVSP_POSTCROP_CONFIG.
*/
#define IOCTL_HVSP_SET_POST_CROP_CONFIG       _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_SET_POST_CROP_CONFIG_NR)
/**
* Used to get mutiinst private id, use ST_IOCTL_HVSP_PRIVATE_ID_CONFIG.
*/
#define IOCTL_HVSP_GET_PRIVATE_ID_CONFIG      _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_GET_PRIVATE_ID_CONFIG_NR)
/**
* Used to get display size and crop information, use ST_IOCTL_HVSP_SCINFORM_CONFIG.
*/
#define IOCTL_HVSP_GET_INFORM_CONFIG          _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_GET_INFORM_CONFIG_NR)
/**
* Used to releace DNR buffer, use void.
*/
#define IOCTL_HVSP_RELEASE_MEM_CONFIG         _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_RELEASE_MEM_CONFIG_NR)
/**
* Used to Set OSD Configuration, use ST_IOCTL_HVSP_OSD_CONFIG.
*/
#define IOCTL_HVSP_SET_OSD_CONFIG               _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_SET_OSD_CONFIG_NR)
/**
* Used to Set FB Configuration,debug, use ST_IOCTL_HVSP_SET_FB_MANAGE_CONFIG.
*/
#define IOCTL_HVSP_SET_FB_MANAGE_CONFIG        _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_SET_FB_MANAGE_CONFIG_NR)
/**
* Used to get version, use ST_IOCTL_HVSP_GET_VERSION_CONFIG.
*/
#define IOCTL_HVSP_GET_VERSION_CONFIG            _IO(IOCTL_HVSP_MAGIC,  IOCLT_HVSP_GET_VERSION_CONFIG_NR)
/**
* Used to Set MASK Configuration, use ST_IOCTL_HVSP_PRIMASK_CONFIG.
*/
#define IOCTL_HVSP_SET_PRIMASK_CONFIG               _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_SET_PRIMASK_CONFIG_NR)
/**
* Used to Set MASK Trigger, use ST_IOCTL_HVSP_PRIMASK_TRIGGER_CONFIG.
*/
#define IOCTL_HVSP_PRIMASK_TRIGGER_CONFIG               _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_PRIMASK_TRIGGER_CONFIG_NR)
/**
* Used to set VTRACK, use ST_IOCTL_VIP_VTRACK_CONFIG.
*/
#define IOCTL_HVSP_SET_VTRACK_CONFIG                            _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_SET_VTRACK_CONFIG_NR)
/**
* Used to set VTACK ON, use ST_IOCTL_VIP_VTRACK_ONOFF_CONFIG.
*/
#define IOCTL_HVSP_SET_VTRACK_ONOFF_CONFIG                      _IO(IOCTL_HVSP_MAGIC,  IOCTL_HVSP_SET_VTRACK_ONOFF_CONFIG_NR)

#endif //
/** @} */ // end of hvsp_group
