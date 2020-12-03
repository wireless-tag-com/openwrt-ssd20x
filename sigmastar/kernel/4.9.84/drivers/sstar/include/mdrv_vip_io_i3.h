/*
* mdrv_vip_io_i3.h- Sigmastar
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
 * \defgroup vip_group  VIP driver
 * \note
 *
 * sysfs Node: /sys/devices/platform/mvip1.0/bypass
 *
 * sysfs R/W mode: W
 *
 * sysfs Usage & Description: W:echo 1 open bypass ,echo 0 close.
 *
 * sysfs Node: /sys/devices/platform/mhvsp1.0/ckPQ
 *
 * sysfs R/W mode: W
 *
 * sysfs Usage & Description: echo 1 PQ already setting check , echo 2 Auto test PQ
 *
 * sysfs Node: /sys/devices/platform/mhvsp1.0/ckCMDQ
 *
 * sysfs R/W mode: W
 *
 * sysfs Usage & Description: echo 1 CMDQ already setting check , echo 2 Auto test CMDQ
 *
 * @{
 */
#ifndef _MDRV_VIP_IO_H
#define _MDRV_VIP_IO_H

//=============================================================================
// Includs
//=============================================================================


//=============================================================================
// IOCTRL defines
//=============================================================================
#define IOCTL_VIP_SET_PEAKING_CONFIG_NR                        (1)  ///< The IOCTL NR definition,IOCTL_VIP_SET_PEAKING_CONFIG
#define IOCTL_VIP_SET_DLC_HISTOGRAM_CONFIG_NR                  (2)  ///< The IOCTL NR definition,IOCTL_VIP_SET_DLC_HISTOGRAM_CONFIG
#define IOCTL_VIP_GET_DLC_HISTOGRAM_REPORT_NR                  (3)  ///< The IOCTL NR definition,IOCTL_VIP_GET_DLC_HISTOGRAM_REPORT
#define IOCTL_VIP_SET_DLC_CONFIG_NR                            (4)  ///< The IOCTL NR definition,IOCTL_VIP_SET_DLC_CONFIG
#define IOCTL_VIP_SET_LCE_CONFIG_NR                            (6)  ///< The IOCTL NR definition,IOCTL_VIP_SET_LCE_CONFIG
#define IOCTL_VIP_SET_UVC_CONFIG_NR                            (7)  ///< The IOCTL NR definition,IOCTL_VIP_SET_UVC_CONFIG
#define IOCTL_VIP_SET_IHC_CONFIG_NR                            (8)  ///< The IOCTL NR definition,IOCTL_VIP_SET_IHC_CONFIG
#define IOCTL_VIP_SET_ICE_CONFIG_NR                            (9)  ///< The IOCTL NR definition,IOCTL_VIP_SET_ICE_CONFIG
#define IOCTL_VIP_SET_IHC_ICE_ADP_Y_CONFIG_NR                  (10) ///< The IOCTL NR definition,IOCTL_VIP_SET_IHC_ICE_ADP_Y_CONFIG
#define IOCTL_VIP_SET_IBC_CONFIG_NR                            (11) ///< The IOCTL NR definition,IOCTL_VIP_SET_IBC_CONFIG
#define IOCTL_VIP_SET_FCC_CONFIG_NR                            (12) ///< The IOCTL NR definition,IOCTL_VIP_SET_FCC_CONFIG
#define IOCTL_VIP_SET_VIP_CONFIG_NR                            (13) ///< The IOCTL NR definition,IOCTL_VIP_SET_VIP_CONFIG
#define IOCTL_VIP_CMDQ_WRITE_CONFIG_NR                         (14) ///< The IOCTL NR definition,IOCTL_VIP_CMDQ_WRITE_CONFIG
#define IOCTL_VIP_SET_LDC_CONFIG_NR                            (15) ///< The IOCTL NR definition,IOCTL_VIP_SET_LDC_CONFIG
#define IOCTL_VIP_SET_LDC_MD_CONFIG_NR                         (16) ///< The IOCTL NR definition,IOCTL_VIP_SET_LDC_MD_CONFIG
#define IOCTL_VIP_SET_LDC_DMAP_CONFIG_NR                       (17) ///< The IOCTL NR definition,IOCTL_VIP_SET_LDC_DMAP_CONFIG
#define IOCTL_VIP_SET_LDC_SRAM_CONFIG_NR                       (18) ///< The IOCTL NR definition,IOCTL_VIP_SET_LDC_SRAM_CONFIG
#define IOCTL_VIP_SET_ACK_CONFIG_NR                            (19) ///< The IOCTL NR definition,IOCTL_VIP_SET_ACK_CONFIG
#define IOCTL_VIP_SET_NLM_CONFIG_NR                            (20) ///< The IOCTL NR definition,IOCTL_VIP_SET_NLM_CONFIG
#define IOCTL_VIP_SET_VTRACK_CONFIG_NR                         (22) ///< The IOCTL NR definition,IOCTL_VIP_SET_VTRACK_CONFIG
#define IOCTL_VIP_SET_VTRACK_ONOFF_CONFIG_NR                   (23) ///< The IOCTL NR definition,IOCTL_VIP_SET_VTRACK_ONOFF_CONFIG
#define IOCTL_VIP_AIP_CONFIG_NR                                (24) ///< The IOCTL NR definition,IOCTL_VIP_AIP_CONFIG
#define IOCTL_VIP_AIP_SRAM_CONFIG_NR                           (25) ///< The IOCTL NR definition,IOCTL_VIP_AIP_SRAM_CONFIG
#define IOCTL_VIP_SET_MCNR_CONFIG_NR                           (26)  ///< The IOCTL NR definition,IOCTL_VIP_SET_MCNR_CONFIG
#define IOCLT_VIP_GET_VERSION_CONFIG_NR                        (27)  ///< The IOCTL NR definition, IOCLT_VIP_GET_VERSION_CONFIG_NR
#define IOCLT_VIP_SET_ALLVIP_CONFIG_NR                         (IOCLT_VIP_GET_VERSION_CONFIG_NR+1) ///< The IOCTL NR for vip driver
#define IOCTL_VIP_MAX_NR                                       (IOCLT_VIP_SET_ALLVIP_CONFIG_NR+1) ///< The Max IOCTL NR for vip driver


// use 'm' as magic number
#define IOCTL_VIP_MAGIC                                        ('3') ///< The Type definition of IOCTL for vip driver
/**
* Used to set CMDQ cmd, use ST_IOCTL_VIP_CMDQ_CONFIG.
*/
#define IOCTL_VIP_CMDQ_WRITE_CONFIG                            _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_CMDQ_WRITE_CONFIG_NR)
/**
* Used to set LDC, use ST_IOCTL_VIP_LDC_CONFIG.
*/
#define IOCTL_VIP_SET_LDC_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_LDC_CONFIG_NR)
/**
* Used to set LDC mode, use ST_IOCTL_VIP_LDC_MD_CONFIG.
*/
#define IOCTL_VIP_SET_LDC_MD_CONFIG                            _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_LDC_MD_CONFIG_NR)
/**
* Used to set LDC DMAP address, use ST_IOCTL_VIP_LDC_DMAP_CONFIG.
*/
#define IOCTL_VIP_SET_LDC_DMAP_CONFIG                          _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_LDC_DMAP_CONFIG_NR)
/**
* Used to set LDC SRAM address, use ST_IOCTL_VIP_LDC_SRAM_CONFIG.
*/
#define IOCTL_VIP_SET_LDC_SRAM_CONFIG                          _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_LDC_SRAM_CONFIG_NR)
/**
* Used to set PK, use ST_IOCTL_VIP_PEAKING_CONFIG.
*/
#define IOCTL_VIP_SET_PEAKING_CONFIG                           _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_PEAKING_CONFIG_NR)
/**
* Used to set DLC hist, use ST_IOCTL_VIP_DLC_HISTOGRAM_CONFIG.
*/
#define IOCTL_VIP_SET_DLC_HISTOGRAM_CONFIG                     _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_DLC_HISTOGRAM_CONFIG_NR)
/**
* Used to get DLC hist, use ST_IOCTL_VIP_DLC_HISTOGRAM_REPORT.
*/
#define IOCTL_VIP_GET_DLC_HISTOGRAM_REPORT                     _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_GET_DLC_HISTOGRAM_REPORT_NR)
/**
* Used to set DLC, use ST_IOCTL_VIP_DLC_CONFIG.
*/
#define IOCTL_VIP_SET_DLC_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_DLC_CONFIG_NR)
/**
* Used to set LCE, use ST_IOCTL_VIP_LCE_CONFIG.
*/
#define IOCTL_VIP_SET_LCE_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_LCE_CONFIG_NR)
/**
* Used to set UVC, use ST_IOCTL_VIP_UVC_CONFIG.
*/
#define IOCTL_VIP_SET_UVC_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_UVC_CONFIG_NR)
/**
* Used to set IHC, use ST_IOCTL_VIP_IHC_CONFIG.
*/
#define IOCTL_VIP_SET_IHC_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_IHC_CONFIG_NR)
/**
* Used to set ICE, use ST_IOCTL_VIP_ICC_CONFIG.
*/
#define IOCTL_VIP_SET_ICE_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_ICE_CONFIG_NR)
/**
* Used to set IHCICC, use ST_IOCTL_VIP_IHCICC_CONFIG.
*/
#define IOCTL_VIP_SET_IHC_ICE_ADP_Y_CONFIG                     _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_IHC_ICE_ADP_Y_CONFIG_NR)
/**
* Used to set IBC, use ST_IOCTL_VIP_IBC_CONFIG.
*/
#define IOCTL_VIP_SET_IBC_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_IBC_CONFIG_NR)
/**
* Used to set FCC, use ST_IOCTL_VIP_FCC_CONFIG.
*/
#define IOCTL_VIP_SET_FCC_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_FCC_CONFIG_NR)
/**
* Used to set ACK, use ST_IOCTL_VIP_ACK_CONFIG.
*/
#define IOCTL_VIP_SET_ACK_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_ACK_CONFIG_NR)
/**
* Used to set NLM, use ST_IOCTL_VIP_NLM_CONFIG.
*/
#define IOCTL_VIP_SET_NLM_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_NLM_CONFIG_NR)
/**
* Used to set VIPLB, use ST_IOCTL_VIP_CONFIG.
*/
#define IOCTL_VIP_SET_VIP_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_VIP_CONFIG_NR)
/**
* Used to set AIP, use ST_IOCTL_VIP_AIP_CONFIG.
*/
#define IOCTL_VIP_SET_AIP_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_AIP_CONFIG_NR)
/**
* Used to set AIP, use ST_IOCTL_VIP_AIP_SRAM_CONFIG.
*/
#define IOCTL_VIP_SET_AIP_SRAM_CONFIG                          _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_AIP_SRAM_CONFIG_NR)
/**
* Used to set MCNR , use ST_IOCTL_VIP_MCNR_CONFIG.
*/
#define IOCTL_VIP_SET_MCNR_CONFIG                               _IO(IOCTL_VIP_MAGIC,  IOCTL_VIP_SET_MCNR_CONFIG_NR)

/**
* Used to get version, use ST_IOCTL_VIP_GET_VERSION_CONFIG.
*/
#define IOCTL_VIP_GET_VERSION_CONFIG                           _IO(IOCTL_VIP_MAGIC,  IOCLT_VIP_GET_VERSION_CONFIG_NR)
/**
* Used to set all vip config, use ST_IOCTL_VIP_AllSET_CONFIG.
*/
#define IOCLT_VIP_SET_ALLVIP_CONFIG                             _IO(IOCTL_VIP_MAGIC,  IOCLT_VIP_SET_ALLVIP_CONFIG_NR)
#endif //
/** @} */ // end of hvsp_group
