/*
* mhal_pwm.h- Sigmastar
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
#ifndef __PWM_H
#define __PWM_H

#include <linux/clk.h>
#include <linux/pwm.h>
#include "ms_platform.h"
#include "registers.h"
#include "mdrv_types.h"

#if 0
#define MS_PWM_INFO(x, args...)    printk(x, ##args)
#define MS_PWM_DBG(x, args...)     printk(x, ##args)
#else
#define MS_PWM_INFO(x, args...)    {}
#define MS_PWM_DBG(x, args...)     {}
#endif

struct mstar_pwm_chip {
	struct pwm_chip chip;
	struct clk *clk;
	void __iomem *base;
    u32 *pad_ctrl;
    void* group_data;
    int irq;
};

//------------------------------------------------------------------------------
//  Constants
//------------------------------------------------------------------------------
#define PWM_GROUP_NUM           (1)
#define PWM_NUM                 (4)
//Common PWM registers
#define u16REG_PWM_SHIFT_L      (0x0 << 2)
#define u16REG_PWM_SHIFT_H      (0x1 << 2)
#define u16REG_PWM_DUTY_L       (0x2 << 2)
#define u16REG_PWM_DUTY_H       (0x3 << 2)
#define u16REG_PWM_PERIOD_L     (0x4 << 2) //reg_pwm0_period
#define u16REG_PWM_PERIOD_H     (0x5 << 2)
#define u16REG_PWM_DIV          (0x6 << 2)
#define u16REG_PWM_CTRL         (0x7 << 2)
    #define VDBEN_SW_BIT           0
    #define DBEN_BIT               1
    #define DIFF_P_EN_BIT          2
    #define SHIFT_GAT_BIT          3
    #define POLARITY_BIT           4

#define u16REG_PWM_SHIFT2       (0x8 << 2)
#define u16REG_PWM_DUTY2        (0x9 << 2)
#define u16REG_PWM_SHIFT3       (0xA << 2)
#define u16REG_PWM_DUTY3        (0xB << 2)
#define u16REG_PWM_SHIFT4       (0xC << 2)
#define u16REG_PWM_DUTY4        (0xD << 2)

#define REG_GROUP_HOLD          (0x71 << 2)
    #define REG_GROUP_HOLD_SHFT     (0x0)

#define REG_GROUP_STOP          (0x72 << 2)
    #define REG_GROUP_STOP_SHFT     (0x0)

#define REG_GROUP_ENABLE        (0x73 << 2)
    #define REG_GROUP_ENABLE_SHFT   (0x0)

#define REG_GROUP_JOIN          (0x74 << 2)
    #define REG_GROUP_JOIN_SHFT     (0x0)

//+++[Only4I6e]
#define REG_GROUP_INT           (0x75 << 2)
    #define REG_GROUP_HOLD_INT_SHFT (0x0)
    #define REG_GROUP_RUND_INT_SHFT (0x3)

#define REG_PWM_DUTY_QE0        (0x76 << 2)
    #define REG_PWM_DUTY_QE0_SHFT   (0x0)

#define REG_GROUP_HOLD_MODE1    (0x77 << 2)
    #define REG_GROUP_HALD_MD1_SHFT (0x0)

#define REG_PWM_OUT             (0x7E << 2)
    #define REG_PWM_OUT_SHFT        (0x0)
//---[Only4I6e]

#define u16REG_SW_RESET         (0x7F << 2)

//------------------------------------------------------------------------------
//  Export Functions
//------------------------------------------------------------------------------
//void DrvBoostInit(void);
//void DrvBoostReset(void);
//void DrvPWMInit(U8 u8Id);
//void DrvPWMReset(void);
//void DrvBacklightSet(U8 u8Level, U8 u8IsSave);
//U8 DrvBacklightGet(void);
//void DrvBacklightOn(void);
//void DrvBacklightOff(void);
//void DrvPWMSetEn(U8 u8Id, U8 u8Val);
void DrvPWMInit(struct mstar_pwm_chip *ms_chip, U8 u8Id);
void DrvPWMSetPeriod(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Val);
void DrvPWMGetPeriod(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32* pu32Val);
void DrvPWMSetDuty(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Val);
void DrvPWMGetDuty(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32* pu32Val);
void DrvPWMEnable(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val);
void DrvPWMEnableGet(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8* pu8Val);
void DrvPWMSetPolarity(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val);
void DrvPWMGetPolarity(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8* pu8Val);
void DrvPWMPadSet(U8 u8Id, U8 u8Val);
void DrvPWMSetDben(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val);

int DrvPWMSetBegin(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8DutyId, U32 u32Val);
int DrvPWMSetEnd(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8ShftId, U32 u32Val);
int DrvPWMSetPolarityEx(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val);
void DrvPWMSetPeriodEx(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Val);
int DrvPWMDiv(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val); // workaround for group hold/round malfunctional when div is zero


int DrvPWMGroupCap(void);
int DrvPWMGroupJoin(struct mstar_pwm_chip* ms_chip, U8 u8PWMId, U8 u8Val);
int DrvPWMGroupEnable(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U8 u8Val);
int DrvPWMGroupIsEnable(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U8* pu8Val);
int DrvPWMGroupSetRound(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U16 u16Val);
int DrvPWMGroupStop(struct mstar_pwm_chip *ms_chip, U8 u8GroupId, U8 u8Val);
int DrvPWMGroupHold(struct mstar_pwm_chip *ms_chip, U8 u8GroupId, U8 u8Val);
int DrvPWMGroupInfo(struct mstar_pwm_chip *ms_chip, char* buf_start, char* buf_end);

//+++[Only4I6e]
int DrvPWMGroupGetRoundNum(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U16* u16Val);
int DrvPWMGroupGetHoldM1(struct mstar_pwm_chip *ms_chip);
int DrvPWMGroupHoldM1(struct mstar_pwm_chip *ms_chip, U8 u8Val);
int DrvPWMDutyQE0(struct mstar_pwm_chip *ms_chip, U8 u8GroupId, U8 u8Val);
int DrvPWMGetOutput(struct mstar_pwm_chip *ms_chip, U8* pu8Output);
//---[Only4I6e]

//-----------------------------------------------------------------------------



#endif  //__PWM_H
