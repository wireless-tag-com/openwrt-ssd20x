/*
* chicago_rtc.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
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


#ifndef	__CHICAGO_RTC_H__
#define	__CHICAGO_RTC_H__

//------------------------------------------------------------------------------
//  Include Files
//------------------------------------------------------------------------------
#include "asm/arch/mach/ms_types.h"

//------------------------------------------------------------------------------
//  Macros
//------------------------------------------------------------------------------
typedef struct
{
    U16 RTC_CTRL;           // 0x00
    U16 u16REG_RESERVED_00;

    U16 RTC_LOAD_VAL_L;     // 0x01
    U16 u16REG_RESERVED_01;

    U16 RTC_LOAD_VAL_H;     // 0x02
    U16 u16REG_RESERVED_02;

    U16 RTC_SECOND_CNT_L;   // 0x03
    U16 u16REG_RESERVED_03;

    U16 RTC_SECOND_CNT_H;   // 0x04
    U16 u16REG_RESERVED_04;

    U16 RTC_SUB_CNT;        // 0x05
    U16 u16REG_RESERVED_05;

    U16 RTC_MSKCMP_SUB_CNT; // 0x06
    U16 u16REG_RESERVED_06;

    U16 RTC_FREQ_CW;        // 0x07
    U16 u16REG_RESERVED_07;

    U16 RTC_MATCH_VAL_L;    // 0x08
    U16 u16REG_RESERVED_08;

    U16 RTC_MATCH_VAL_H;    // 0x09
    U16 u16REG_RESERVED_09;

    U16 RTC_CTRL_2;         // 0x0A
    U16 u16REG_RESERVED_0A;

    U16 RTC_RAW_SEC_CNT_L;  // 0x0B
    U16 u16REG_RESERVED_0B;

    U16 RTC_RAW_SEC_CNT_H;  // 0x0C
    U16 u16REG_RESERVED_0C;

    U16 RTC_RAW_SUB_CNT;    // 0x0D
    U16 u16REG_RESERVED_0D;

    U16 RTC_SNAP_SHOT;      // 0x0E
    U16 u16REG_RESERVED_0E;

    U16 RTC_FSM_STATE;      // 0x0F
    U16 u16REG_RESERVED_0F;

    U16 RTC_INIT_FLAG_L;    // 0x10
    U16 u16REG_RESERVED_10;

    U16 RTC_INIT_FLAG_H;    // 0x11
    U16 u16REG_RESERVED_11;

    U16 RTC_OFFSET_FLAG_L;  // 0x12
    U16 u16REG_RESERVED_12;

    U16 RTC_OFFSET_FLAG_H;  // 0x13
    U16 u16REG_RESERVED_13;

    U16 RTC_ALARM_FLAG_L;   // 0x14
    U16 u16REG_RESERVED_14;

    U16 RTC_ALARM_FLAG_H;   // 0x15
    U16 u16REG_RESERVED_15;

    U16 RTC_EVENT1_FLAG_L;  // 0x16
    U16 u16REG_RESERVED_16;

    U16 RTC_EVENT1_FLAG_H;  // 0x17
    U16 u16REG_RESERVED_17;

    U16 RTC_EVENT2_FLAG_L;  // 0x18
    U16 u16REG_RESERVED_18;

    U16 RTC_EVENT2_FLAG_H;  // 0x19
    U16 u16REG_RESERVED_19;

    U16 RTC_EVENT3_FLAG_L;  // 0x1A
    U16 u16REG_RESERVED_1A;

    U16 RTC_EVENT3_FLAG_H;  // 0x1B
    U16 u16REG_RESERVED_1B;

    U16 RTC_CURRENT_FLAG_L; // 0x1C
    U16 u16REG_RESERVED_1C;

    U16 RTC_CURRENT_FLAG_H; // 0x1D
    U16 u16REG_RESERVED_1D;

    U16 RTC_FLAG_UPDATE;    // 0x1E
    U16 u16REG_RESERVED_1E;

    U16 RTC_INTERRUPT_CTRL; // 0x1F
    U16 u16REG_RESERVED_1F;

    U16 RTC_LIFE_0;         // 0x20
    U16 u16REG_RESERVED_20;

    U16 RTC_LIFE_1;         // 0x21
    U16 u16REG_RESERVED_21;

    U16 RTC_LIFE_2;         // 0x22
    U16 u16REG_RESERVED_22;

    U16 RTC_LIFE_3;         // 0x23
    U16 u16REG_RESERVED_23;

    U16 RTC_PAD_PASSWD;     // 0x24
    U16 u16REG_RESERVED_24;

    U16 RTC_SPARE_0;        // 0x25
    U16 u16REG_RESERVED_25;

    U16 RTC_SPARE_1;        // 0x26
    U16 u16REG_RESERVED_26;

    U16 RTC_WATCH_DOG_CTRL; // 0x27
    U16 u16REG_RESERVED_27;

    U16 RTC_TURN_ON_CTRL;   // 0x28
    U16 u16REG_RESERVED_28;

    U16 RTC_ANALOG_TEST;    // 0x29
    U16 u16REG_RESERVED_29;

    U16 RTC_POWER_STATUS;   // 0x2A
    U16 u16REG_RESERVED_2A;

    U16 RTC_IRQ_MASK;       // 0x2B
    U16 u16REG_RESERVED_2B;

    U16 RTC_IRQ_FORCE;      // 0x2C
    U16 u16REG_RESERVED_2C;

    U16 RTC_IRQ_CLR;        // 0x2D
    U16 u16REG_RESERVED_2D;

    U16 RTC_IRQ_RAW_STATUS; // 0x2E
    U16 u16REG_RESERVED_2E;

    U16 RTC_INT_PASSWD;     // 0x2F
    U16 u16REG_RESERVED_2F;
} REG_RTC_st, *PREG_RTC_st;


#endif // __CHICAGO_RTC_H__

/* 	END */
