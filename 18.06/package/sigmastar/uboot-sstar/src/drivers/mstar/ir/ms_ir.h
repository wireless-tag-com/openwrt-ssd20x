/*
 * ms_ir.h
 *
 *  Created on:
 *      Author: Administrator
 */

#ifndef _MS_IR_H_
#define _MS_IR_H_
#include "asm/arch/mach/ms_types.h"

#define IR_TYPE_OLD                 0
#define IR_TYPE_NEW                 1
#define IR_TYPE_MSTAR_DTV           2
#define IR_TYPE_MSTAR_RAW           3
#define IR_TYPE_RC_V16              4
#define IR_TYPE_CUS03_DTV           5
#define IR_TYPE_MSTAR_FANTASY       6
#define IR_TYPE_MSTAR_SZ1           7
#define IR_TYPE_CUS08_RC5           8
#define IR_TYPE_CUS21SH             9
#define IR_TYPE_RCMM               10

#define IR_TYPE_SEL  IR_TYPE_MSTAR_DTV


// IR Header code define
#define IR_HEADER_CODE0         0x80    // Custom 0
#define IR_HEADER_CODE1         0x7F    // Custom 1

// IR Timing define
#define IR_HEADER_CODE_TIME     9000    // us
#define IR_OFF_CODE_TIME        4500    // us
#define IR_OFF_CODE_RP_TIME     2500    // us
#define IR_LOGI_01H_TIME        560     // us
#define IR_LOGI_0_TIME          1120    // us
#define IR_LOGI_1_TIME          2240    // us
#define IR_TIMEOUT_CYC          140000  // us

#define IR_EVENT_TIMEOUT        220



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


#define START_UP 0x46

struct IR_KeyInfo
{
    U8 u8Key;
    U8 u8System;
    U8 u8Flag;
    U8 u8Valid;
};

u8 _mdrv_get_key(u8 *key_value);
int _mdrv_ir_init(void);

#endif /*_MS_IR_H_ */