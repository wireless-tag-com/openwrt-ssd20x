/*
* ms_ir.h- Sigmastar
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
 * ms_ir.h
 *
 *  Created on:
 *      Author: Administrator
 */

#ifndef _MS_IR_H_
#define _MS_IR_H_
#include "ms_types.h"

#define IR_MODE_FULLDECODE    1
#define IR_MODE_RAWDATA       2
#define IR_MODE_SWDECODE      3    //not support now
#define IR_MODE_SWDECODE_KON  4    //not support now

#define XTAL_CLOCK_FREQ             12000000    //12 MHz
//-------------------------------------------------------------------------------------------
// IR system parameter define for H/W setting (Please don't modify them)
//-------------------------------------------------------------------------------------------
#define IR_CKDIV_NUM                        ((XTAL_CLOCK_FREQ+500000)/1000000)
#define IR_CLK                              (XTAL_CLOCK_FREQ/1000000)

#define irGetMinCnt(time, tolerance)        ((u32)(((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1))*((double)1-tolerance)))
#define irGetMaxCnt(time, tolerance)        ((u32)(((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1))*((double)1+tolerance)))
#define irGetCnt(time)                      ((u32)((double)time*((double)IR_CLK)/(IR_CKDIV_NUM+1)))

// 90Mhz
#define IR_RP_TIMEOUT           irGetCnt(IR_TIMEOUT_CYC)
#define IR_HDC_UPB              irGetMaxCnt(IR_HEADER_CODE_TIME, 0.2)
#define IR_HDC_LOB              irGetMinCnt(IR_HEADER_CODE_TIME, 0.2)
#define IR_OFC_UPB              irGetMaxCnt(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_LOB              irGetMinCnt(IR_OFF_CODE_TIME, 0.2)
#define IR_OFC_RP_UPB           irGetMaxCnt(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_OFC_RP_LOB           irGetMinCnt(IR_OFF_CODE_RP_TIME, 0.2)
#define IR_LG01H_UPB            irGetMaxCnt(IR_LOGI_01H_TIME, 0.35)
#define IR_LG01H_LOB            irGetMinCnt(IR_LOGI_01H_TIME, 0.3)
#define IR_LG0_UPB              irGetMaxCnt(IR_LOGI_0_TIME, 0.2)
#define IR_LG0_LOB              irGetMinCnt(IR_LOGI_0_TIME, 0.2)
#define IR_LG1_UPB              irGetMaxCnt(IR_LOGI_1_TIME, 0.2)
#define IR_LG1_LOB              irGetMinCnt(IR_LOGI_1_TIME, 0.2)

struct IR_KeyInfo
{
    U8 u8Key;
    U8 u8System;
    U8 u8Flag;
    U8 u8Valid;
};

#define MaxQueue 100
struct Key_Queue
{
    u32 item[MaxQueue];
    int front;
    int rear;
};

#endif /*_MS_IR_H_ */
