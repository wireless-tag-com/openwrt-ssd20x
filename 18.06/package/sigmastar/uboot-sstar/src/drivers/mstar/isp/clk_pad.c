/*
* clk_pad.c- Sigmastar
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

#include "isp_hal.h"
#include "infinity_reg_isp0.h"
#include "isp_pub.h"

void Select_SR_IOPad(SR_PAD_CFG cfg)
{
  chiptop_sensor_mode *sensor_mode = (chiptop_sensor_mode *)(BANK_BASE_ADDR(RIUBASE_CHIPTOP) + OFFSET_CHIPTOP_TO_SENSOR_MODE);
  sensor_mode->reg_sr_mode     = cfg;
}

void Set_SR_MCLK(u8 enable, u8 mclk_speed)
{
    clk_sensor *sensor = (clk_sensor *)(BANK_BASE_ADDR(RIUBASE_CLKGEN)+OFFSET_CLKGEN_TO_CLK_SENSOR); //(MsIoMapGetRiuBase(RIUBASE_CLKGEN) + OFFSET_CLKGEN_TO_CLK_SENSOR);
    /*
    [4:2]: Select clock source
    000: 27 MHz
    001: 21.6 MHz
    010: xtal (12) MHz
    011: 5.4 MHz
    100: 36 MHz
    101: 54 MHz
    110: 43.2 MHz
    111: 61.7 MHz"
    */

    switch(mclk_speed) {
    case CUS_CMU_CLK_5P4M:
        sensor->reg_ckg_sr_mclk_select_clock_source = SEN_MAIN_CLK_5P4M; //5.4MHz
        break;
    case CUS_CMU_CLK_12M:
        sensor->reg_ckg_sr_mclk_select_clock_source = SEN_MAIN_CLK_12M; //12MHz
        break;
    case CUS_CMU_CLK_21P6M:
        sensor->reg_ckg_sr_mclk_select_clock_source = SEN_MAIN_CLK_21P6M; //21.6MHz
        break;
    case CUS_CMU_CLK_27M:
        sensor->reg_ckg_sr_mclk_select_clock_source = SEN_MAIN_CLK_27M; //27MHz
        break;
    case CUS_CMU_CLK_36M:
        sensor->reg_ckg_sr_mclk_select_clock_source = SEN_MAIN_CLK_36M; //36MHz
        break;
    case CUS_CMU_CLK_54M:
        sensor->reg_ckg_sr_mclk_select_clock_source = SEN_MAIN_CLK_54M; //54MHz
        break;
    case CUS_CMU_CLK_61P7M:
        sensor->reg_ckg_sr_mclk_select_clock_source = SEN_MAIN_CLK_61P7M; //61.7MHz
        break;
    }

    if (enable == 0)
        sensor->reg_ckg_sr_mclk_disable_clock = 1;
    else
        sensor->reg_ckg_sr_mclk_disable_clock = 0;
}

void ISP_SrPwdn(int id,int val)
{
    infinity_reg_isp0 *isp0 = (infinity_reg_isp0*)BANK_BASE_ADDR(RIUBASE_ISP_0);
    switch(id)
    {
        case 0:
            isp0->reg_sensor1_rst = val;
            break;
        case 1:
            isp0->reg_sensor2_rst = val;
            break;
    }
}

void ISP_SrRst(int id,int val)
{
    infinity_reg_isp0 *isp0 = (infinity_reg_isp0*)BANK_BASE_ADDR(RIUBASE_ISP_0);
    switch(id)
    {
        case 0:
            isp0->reg_sensor1_pwrdn = val;
            break;
        case 1:
            isp0->reg_sensor2_pwrdn = val;
            break;
    }
}
