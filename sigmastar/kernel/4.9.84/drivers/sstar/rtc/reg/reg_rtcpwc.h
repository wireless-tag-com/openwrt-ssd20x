/*
* reg_rtcpwc.h- Sigmastar
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
/*
 * kernel_rtcpwc.h
 *
 *  Created on: 2017/8/29
 *      Author: titan.huang
 */

#ifndef __KERNEL_RTC_PWC_H__
#define __KERNEL_RTC_PWC_H__


#define RTCPWC_DIG2RTC_BASE_WR					(0x00 << 2)
	#define RTCPWC_DIG2RTC_BASE_WR_BIT			BIT1
	#define RTCPWC_DIG2RTC_BASE_RD				BIT2
       #define RTCPWC_DIG2RTC_CNT_RST_WR                 BIT3
       #define RTCPWC_DIG2RTC_ALARM_WR                    BIT4
       #define RTCPWC_DIG2RTC_SW0_WR                        BIT5
       #define RTCPWC_DIG2RTC_SW1_WR                        BIT6
       #define RTCPWC_DIG2RTC_SW0_RD                        BIT7
       #define RTCPWC_DIG2RTC_SW1_RD                        BIT8
//#define RTCPWC_DIG2RTC_CNT_RST_WR				(0x00 << 2)
//	#define RTCPWC_DIG2RTC_CNT_RST_WR_BIT		BIT3
#define RTCPWC_DIG2RTC_CNT_RD					(0x04) //(0x01 << 2)
	#define RTCPWC_DIG2RTC_CNT_RD_BIT			BIT0

#define RTCPWC_DIG2RTC_ISO_CTRL					(0x0C) //(0x03 << 2)
	#define RTCPWC_DIG2RTC_BASE_WR_MASK		BIT0|BIT1|BIT2
#define RTCPWC_DIG2RTC_WRDATA_L					(0x10) //(0x04 << 2)
#define RTCPWC_DIG2RTC_WRDATA_H					(0x14) //(0x05 << 2)

#define RTCPWC_DIG2RTC_SET						(0x18) //(0x06 << 2)
	#define RTCPWC_DIG2RTC_SET_BIT				BIT0
#define RTCPWC_RTC2DIG_VAILD					       (0x1C) //(0x07 << 2)
	#define RTCPWC_RTC2DIG_VAILD_BIT			       BIT0
#define RTCPWC_RTC2DIG_ISO_CTRL_ACK				(0x20) //(0x08 << 2)
	#define RTCPWC_RTC2DIG_ISO_CTRL_ACK_BIT		BIT3

#define RTCPWC_RTC2DIG_RDDATA_L					(0x24) //(0x09 << 2)
#define RTCPWC_RTC2DIG_RDDATA_H					(0x28) //(0x0A<< 2)

#define RTCPWC_RTC2DIG_CNT_UPDATING				(0x2C) //(0x0B << 2)
	#define RTCPWC_RTC2DIG_CNT_UPDATING_BIT	BIT0
#define RTCPWC_REG_RTC2DIG_RDDATA_CNT_L		(0x30) //(0x0C << 2)
#define RTCPWC_REG_RTC2DIG_RDDATA_CNT_H		(0x34) //(0x0D << 2)

#define RTCPWC_DIG2RTC_CNT_RD_TRIG				(0x38) //(0x0E << 2)
	#define RTCPWC_DIG2RTC_CNT_RD_TRIG_BIT		BIT0

#define RTCPWC_DIG2PWC_OPT                      (0x40) //(0x10 << 2)
    #define RTCPWC_SW_RST                       BIT8

#define RTCPWC_DIG2PWC_RTC_TESTBUS              (0x54) //(0x15 << 2)
    #define RTCPWC_ISO_EN                       BIT0
    #define RTCPWC_CLK_1K                       BIT5

#endif /* __KERNEL_RTC_PWC_H__ */

