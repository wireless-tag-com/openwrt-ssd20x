/*
* snr_cfg_infinity6.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: eroy.yang <eroy.yang@sigmastar.com.tw>
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

#ifndef _SNR_CFG_H_
#define _SNR_CFG_H_

typedef unsigned short u16;

typedef enum 
{
    SNR_MCLK_27M        = 0,
    SNR_MCLK_72M        = 1,
    SNR_MCLK_61P7M      = 2,
    SNR_MCLK_54M        = 3,
    SNR_MCLK_48M        = 4,
    SNR_MCLK_43P2M      = 5,
    SNR_MCLK_36M        = 6,
    SNR_MCLK_24M        = 7,
    SNR_MCLK_21P6M      = 8,
    SNR_MCLK_12M        = 9, 
    SNR_MCLK_5P4M       = 10,
    SNR_MCLK_LPLL       = 11,
    SNR_MCLK_LPLL_DIV2  = 12,
    SNR_MCLK_LPLL_DIV4  = 13,
    SNR_MCLK_LPLL_DIV8  = 14,
    SNR_MCLK_ARMPLL_37P125  = 15,
    SNR_MCLK_MAX        = 0xFF
} SNR_MCLK_FREQ;

typedef enum
{
    VIF_SNR_PAD_PARALLEL_MODE = 0,
    VIF_SNR_PAD_MIPI_1LANE_MODE,
    VIF_SNR_PAD_MIPI_2LANE_MODE,
    VIF_SNR_PAD_MIPI_4LANE_MODE,
    VIF_SNR_PAD_BT656_MODE,
    VIF_SNR_PAD_MODE_MAX
} VIF_SNR_PAD_MODE;

typedef struct
{
    union
    {
        struct
        {
            u16 reg_ckg_sr_disable_clock              :1;
            u16 reg_ckg_sr_invert_clock               :1;
            u16 reg_ckg_sr_select_clock_source        :2;
            u16                                       :4;
            u16 reg_ckg_sr_mclk_disable_clock         :1;
            u16 reg_ckg_sr_mclk_invert_clock          :1;
            u16 reg_ckg_sr_mclk_select_clock_source   :4;
            u16                                       :2;
        } tRegSnrClkCfg;
        u16 nRegSnrClkCfg;
    };
    union
    {
        struct
        {
            u16                                       :4;
            u16 reg_chiptop_sr_bt656_mode             :3;
            u16 reg_chiptop_sr_mipi_mode              :3;
            u16 reg_chiptop_sr_par_mode               :3;
            u16                                       :3;
        } tRegSnrPadSel;
        u16 nRegSnrPadSel;
    };
    union
    {
        struct
        {
            u16 reg_vif_ch_sensor_sw_rstz             :1;
            u16 reg_vif_ch_if_state_rst               :1;
            u16 reg_vif_ch_sensor_rst                 :1;
            u16 reg_vif_ch_sensor_pwrdn               :1;
            u16 reg_vif_ch_hdr_en                     :1;
            u16 reg_vif_ch_hdr_sel                    :3;
            u16 reg_vif_ch_src_sel                    :5;
            u16                                       :2;
            u16 reg_vif_ch_en                         :1;
        } tRegVifSnrCtl;
        u16 nRegVifSnrCtl;
    };
} tSensorConfig;

void Set_SNR_BusType(int iSnrId, int iBusType);
void Set_SNR_IOPad(int mode);
void Set_SNR_MCLK(int iSnrId, int iEnable, int iSpeed);
void SNR_PowerDown(int iSnrId, int iVal);
void SNR_Reset(int iSnrId, int iVal);
void Set_SNR_PdnPad(int mode);
void Set_SNR_RstPad(int mode);

#endif
