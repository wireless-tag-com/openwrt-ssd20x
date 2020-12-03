/*
* mdrv_sca_io.h- Sigmastar
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
#ifndef _MDRV_SCA_IO_H
#define _MDRV_SCA_IO_H

//=============================================================================
// Includs
//=============================================================================
#include "mdrv_sca_st.h"

//=============================================================================
// Defines
//=============================================================================
// library information
#define MSIF_SCA_LIB_CODE               {'S','C','A','\0'}
#define MSIF_SCA_BUILDNUM               {'_','0','1','\0'}
#define MSIF_SCA_LIBVER                 (2)
#define MSIF_SCA_CHANGELIST             (677450)

//IO Ctrl defines:
#define IOCTL_SCA_CONNECT_NR                          (0)
#define IOCTL_SCA_DISCONNECT_NR                       (1)
#define IOCTL_SCA_SET_TIMING_WINDOW_NR                (2)
#define IOCTL_SCA_SET_MVOP_NR                         (3)
#define IOCTL_SCA_SET_DISPLAY_MUTE_NR                 (4)
#define IOCTL_SCA_YPBPR_VGA_MODE_MON_PAR_NR           (5)
#define IOCTL_SCA_GET_MODE_NR                         (6)
#define IOCTL_SCA_SET_VE_NR                           (7)
#define IOCTL_SCA_SET_WINDOW_ONOFF_NR                 (8)
#define IOCTL_SCA_SET_COLOR_KEY_NR                    (9)
#define IOCTL_SCA_SET_MVOP_BASEADDR_NR                (10)
#define IOCTL_SCA_GET_MVOP_BASEADDR_NR                (11)
#define IOCTL_SCA_CHANGE_WINDOW_NR                    (12)
#define IOCTL_SCA_SET_PICTURE_NR                      (13)
#define IOCTL_SCA_GET_PICTURE_NR                      (14)
#define IOCTL_SCA_SET_OUTPUT_TIMING_NR                (15)
#define IOCTL_SCA_CALIBRATION_NR                      (16)
#define IOCTL_SCA_LOAD_ADC_SETTING_NR                 (17)
#define IOCTL_SCA_AVD_CONNECT_NR                      (18)
#define IOCTL_SCA_AVD_CHEKC_VIDEO_STD_NR              (19)
#define IOCTL_SCA_SET_CONSTANTAPLHA_VALUE_NR          (20)
#define IOCTL_SCA_GET_LIB_VER_NR                      (21)
#define IOCTL_SCA_SET_DISP_INTR_NR		              (22)
#define IOCTL_SCA_GET_DISP_INTR_NR		              (23)
#define IOCTL_SCA_RW_REGISTER_NR                      (24)
#define IOCTL_SCA_GET_ACE_INFO_NR                     (25)
#define IOCTL_SCA_GET_DLC_INFO_NR                     (26)
#define IOCTL_SCA_GET_DISP_PATH_CONFIG_NR             (27)
#define IOCTL_SCA_SET_MIRRORCONFIG_NR                 (28)
#define IOCTL_SCA_SET_PQ_BIN_NR                       (29)
#define IOCTL_SCA_SET_DIP_CONFIG_NR                   (30)
#define IOCTL_SCA_GET_DISP_INTR_STATUS_NR             (31)
#define IOCTL_SCA_SET_HDMITX_CONFIG_NR                (32)
#define IOCTL_SCA_SET_DIP_WONCE_TRIG_BASE_NR          (33)
#define IOCTL_SCA_SET_FREEZE_CONFIG_NR                (34)
#define IOCTL_SCA_INIT_MST701_NR                      (35)
#define IOCTL_SCA_GEOMETRY_CALIBRATION_NR             (36)
#define IOCTL_SCA_LOAD_GEOMETRY_SETTING_NR            (37)
#define IOCTL_SCA_SET_ANALOG_POLLING_CONFIG_NR        (38)
#define IOCTL_SCA_SET_CLONE_SCREEN_CONFIG_NR          (39)
#define IOCTL_SCA_GET_CLONE_SCREEN_CONFIG_NR          (40)
#define IOCTL_SCA_SET_PNL_SCC_CONFIG_NR               (41)
#define IOCTL_SCA_SET_CONSTANTALPHA_STATE_NR          (42)
#define IOCTL_SCA_SET_PNL_TIMING_CONFIG_NR            (43)
#define IOCTL_SCA_GET_MONITOR_STATUS_CONFIG_NR        (44)
#define IOCTL_SCA_SET_USER_DISPLAY_CONFIG_NR          (45)
#define IOCTL_SCA_SET_DISPLAY_MUTE_COLOR_NR           (46)
#define IOCTL_SCA_SET_CLONE_SCREEN_RATIO_NR           (47)
#define IOCTL_SCA_SET_DLC_INIT_CONFIG_NR              (48)
#define IOCTL_SCA_SET_DLC_ONOFF_CONFIG_NR             (49)
#define IOCTL_SCA_SET_UEVENT_CONFIG_NR                (50)
#define IOCTL_SCA_SET_CVBSOUT_DAC_CONFIG_NR           (51)
#define IOCTL_SCA_SET_USER_DIPLAY_CONFIG_EX_NR        (52)
#define IOCTL_SCA_SET_CAMERA_INPUTTIMING_CONFIG_NR    (53)
#ifdef __BOOT_PNL__//paul_test
#define IOCTL_SCA_SET_PQ_BIN_IBC_NR                   (54)
#define IOCTL_SCA_SET_PQ_BIN_ICC_NR                   (55)
#define IOCTL_SCA_SET_PQ_BIN_IHC_NR                   (56)
#define IOCTL_SCA_MAX_NR                              (57)
#else
#define IOCTL_SCA_MAX_NR                              (54)
#endif




// use 'm' as magic number
#define IOCTL_SCA_MAGIC                               ('2')

#define IOCTL_SCA_CONNECT                        _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_CONNECT_NR)
#define IOCTL_SCA_DISCONNECT                     _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_DISCONNECT_NR)
#define IOCTL_SCA_SET_TIMING_WINDOW              _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_TIMING_WINDOW_NR)
#define IOCTL_SCA_SET_MVOP                       _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_MVOP_NR)
#define IOCTL_SCA_SET_DISPLAY_MUTE               _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_DISPLAY_MUTE_NR)
#define IOCTL_SCA_YPBPR_VGA_MODE_MON_PAR         _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_YPBPR_VGA_MODE_MON_PAR_NR)
#define IOCTL_SCA_GET_MODE                       _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_GET_MODE_NR)
#define IOCTL_SCA_SET_VE                         _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_VE_NR)
#define IOCTL_SCA_SET_WINDOW_ONOFF               _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_WINDOW_ONOFF_NR)
#define IOCTL_SCA_SET_COLOR_KEY                  _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_COLOR_KEY_NR)
#define IOCTL_SCA_SET_MVOP_BASEADDR              _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_MVOP_BASEADDR_NR)
#define IOCTL_SCA_GET_MVOP_BASEADDR              _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_GET_MVOP_BASEADDR_NR)
#define IOCTL_SCA_CHANGE_WINDOW                  _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_CHANGE_WINDOW_NR)
#define IOCTL_SCA_SET_PICTURE                    _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_PICTURE_NR)
#define IOCTL_SCA_GET_PICTURE                    _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_GET_PICTURE_NR)
#define IOCTL_SCA_SET_OUTPUT_TIMING              _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_OUTPUT_TIMING_NR)
#define IOCTL_SCA_CALIBRATION                    _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_CALIBRATION_NR)
#define IOCTL_SCA_LOAD_ADC_SETTING               _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_LOAD_ADC_SETTING_NR)
#define IOCTL_SCA_AVD_CONNECT                    _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_AVD_CONNECT_NR)
#define IOCTL_SCA_AVD_CHECK_VIDEO_STD            _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_AVD_CHEKC_VIDEO_STD_NR)
#define IOCTL_SCA_GET_LIB_VER                    _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_GET_LIB_VER_NR)
#define IOCTL_SCA_SET_CONSTANTALPHA_VALUE        _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_CONSTANTAPLHA_VALUE_NR)
#define IOCTL_SCA_SET_DISP_INTR		             _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_DISP_INTR_NR)
#define IOCTL_SCA_GET_DISP_INTR		             _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_GET_DISP_INTR_NR)
#define IOCTL_SCA_RW_REGISTER                    _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_RW_REGISTER_NR)
#define IOCTL_SCA_GET_ACE_INFO                   _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_GET_ACE_INFO_NR)
#define IOCTL_SCA_GET_DLC_INFO                   _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_GET_DLC_INFO_NR)
#define IOCTL_SCA_GET_DISP_PATH_CONFIG           _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_GET_DISP_PATH_CONFIG_NR)
#define IOCTL_SCA_SET_MIRROR_CONFIG              _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_MIRRORCONFIG_NR)
#define IOCTL_SCA_SET_PQ_BIN                     _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_PQ_BIN_NR)
#define IOCTL_SCA_SET_DIP_CONFIG                 _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_DIP_CONFIG_NR)
#define IOCTL_SCA_GET_DISP_INTR_STATUS           _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_GET_DISP_INTR_STATUS_NR)
#define IOCTL_SCA_SET_HDMITX_CONFIG              _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_HDMITX_CONFIG_NR)
#define IOCTL_SCA_SET_DIP_WONCE_BASE_CONFIG      _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_DIP_WONCE_TRIG_BASE_NR)
#define IOCTL_SCA_SET_FREEZE_CONFIG              _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_FREEZE_CONFIG_NR)
#define IOCTL_SCA_INIT_MST701                    _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_INIT_MST701_NR)
#define IOCTL_SCA_GEOMETRY_CALIBRATION           _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_GEOMETRY_CALIBRATION_NR)
#define IOCTL_SCA_LOAD_GEOMETRY_SETTING          _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_LOAD_GEOMETRY_SETTING_NR)
#define IOCTL_SCA_SET_ANALOG_POLLING_CONFIG      _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_ANALOG_POLLING_CONFIG_NR)
#define IOCTL_SCA_SET_CLONE_SCREEN_CONFIG        _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_CLONE_SCREEN_CONFIG_NR)
#define IOCTL_SCA_GET_CLONE_SCREEN_CONFIG        _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_GET_CLONE_SCREEN_CONFIG_NR)
#define IOCTL_SCA_SET_PNL_SSC_CONFIG             _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_PNL_SCC_CONFIG_NR)
#define IOCTL_SCA_SET_CONSTANTALPHA_STATE        _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_CONSTANTALPHA_STATE_NR)
#define IOCTL_SCA_SET_PNL_TIMING_CONFIG          _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_PNL_TIMING_CONFIG_NR)
#define IOCTL_SCA_GET_MONITOR_STATUS_CONFIG      _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_GET_MONITOR_STATUS_CONFIG_NR)
#define IOCTL_SCA_SET_USER_DISPLAY_CONFIG        _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_USER_DISPLAY_CONFIG_NR)
#define IOCTL_SCA_SET_DISPLAY_MUTE_COLOR         _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_DISPLAY_MUTE_COLOR_NR)
#define IOCTL_SCA_SET_CLONE_SCREEN_RATIO         _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_CLONE_SCREEN_RATIO_NR)
#define IOCTL_SCA_SET_DLC_INIT_CONFIG            _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_DLC_INIT_CONFIG_NR)
#define IOCTL_SCA_SET_DLC_ONOFF_CONFIG           _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_DLC_ONOFF_CONFIG_NR)
#define IOCTL_SCA_SET_UEVENT_CONFIG              _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_UEVENT_CONFIG_NR)
#define IOCTL_SCA_SET_CVBSOUT_DAC_CONFIG         _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_CVBSOUT_DAC_CONFIG_NR)
#define IOCTL_SCA_SET_USER_DISPLAY_CONFIG_EX     _IO(IOCTL_SCA_MAGIC, IOCTL_SCA_SET_USER_DIPLAY_CONFIG_EX_NR)
#define IOCTL_SCA_SET_CAMERA_INPUTTIMING_CONFIG  _IO(IOCTL_SCA_MAGIC, IOCTL_SCA_SET_CAMERA_INPUTTIMING_CONFIG_NR)
#ifdef __BOOT_PNL__//paul_test
#define IOCTL_SCA_SET_PQ_BIN_IBC                 _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_PQ_BIN_IBC_NR)
#define IOCTL_SCA_SET_PQ_BIN_ICC                 _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_PQ_BIN_ICC_NR)
#define IOCTL_SCA_SET_PQ_BIN_IHC                 _IO(IOCTL_SCA_MAGIC,  IOCTL_SCA_SET_PQ_BIN_IHC_NR)
#endif

#endif //_MDRV_GFLIP_IO_H
