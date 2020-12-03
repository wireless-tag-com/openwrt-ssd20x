/*
* mdrv_pnl_io.h- Sigmastar
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
 * \defgroup pnl_group  PNL driver
 * @{
 */
#ifndef _MDRV_PNL_IO_H
#define _MDRV_PNL_IO_H

//=============================================================================
// Includs
//=============================================================================


//=============================================================================
// IOCTRL defines
//=============================================================================


#define IOCTL_PNL_SET_TIMING_CONFIG_NR          (0)///< The IOCTL NR definition,IOCTL_PNL_SET_TIMING_CONFIG
#define IOCTL_PNL_SET_LPLL_CONFIG_NR          (1)///< The IOCTL NR definition,IOCTL_PNL_SET_TIMING_CONFIG
#define IOCLT_PNL_GET_VERSION_CONFIG_NR         (2)///< The IOCTL NR definition, IOCTL_PNL_GET_VERSION_CONFIG
#define IOCTL_PNL_MAX_NR                        (IOCLT_PNL_GET_VERSION_CONFIG_NR+1)///< The Max IOCTL NR for pnl driver

// use 'm' as magic number
#define IOCTL_PNL_MAGIC                         ('8')///< The Type definition of IOCTL for pnl driver
/**
* Used to set Panel timing LPLL timing, use ST_IOCTL_HVSP_INPUT_CONFIG.
*/
#define IOCTL_PNL_SET_TIMING_CONFIG             _IO(IOCTL_PNL_MAGIC,  IOCTL_PNL_SET_TIMING_CONFIG_NR)
/**
* Used to set LPLL timing, use ST_IOCTL_HVSP_INPUT_CONFIG.
*/
#define IOCTL_PNL_SET_LPLL_CONFIG             _IO(IOCTL_PNL_MAGIC,  IOCTL_PNL_SET_LPLL_CONFIG_NR)


/**
* Used to get version, use ST_IOCTL_PNL_GET_VERSION_CONFIG.
*/
#define IOCTL_PNL_GET_VERSION_CONFIG            _IO(IOCTL_PNL_MAGIC,  IOCLT_PNL_GET_VERSION_CONFIG_NR)

#endif //
/** @} */ // end of pnl_group
