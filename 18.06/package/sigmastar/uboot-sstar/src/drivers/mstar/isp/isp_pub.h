/*
* isp_pub.h- Sigmastar
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

#ifndef ISP_H

typedef enum { //for reg_sr_mode
  SR_PAD_CFG_1 = 1,
  SR_PAD_CFG_2 = 2,
  SR_PAD_CFG_3 = 3,
  SR_PAD_CFG_10BITS = 4,
  SR_PAD_CFG_4 = 4,
  SR_PAD_CFG_12BITS = 5,
  SR_PAD_CFG_5 = 5,
  SR_PAD_CFG_6 = 6,
}SR_PAD_CFG;

void Select_SR_IOPad(SR_PAD_CFG cfg);
void Set_SR_MCLK(u8 enable, u8 mclk_speed);
void ISP_SrPwdn(int id,int val);
void ISP_SrRst(int id,int val);

#endif
