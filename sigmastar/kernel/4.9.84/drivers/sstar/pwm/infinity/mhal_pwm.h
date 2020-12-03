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

#include "mdrv_types.h"

//------------------------------------------------------------------------------
//  Constants
//------------------------------------------------------------------------------
//PWM0
#define u16REG_PWM0_DIV         BK_REG(0x68)
#define u16REG_RESERVED_4       BK_REG(0x68)

#define u16REG_PWM0_DUTY        BK_REG(0x69)
#define u16REG_RESERVED_3       BK_REG(0x69)

#define u16REG_PWM0_PERIOD      BK_REG(0x6A)
#define u16REG_RESERVED_2       BK_REG(0x6A)

//PWM1
#define u16REG_PWM1_DIV         BK_REG(0x6B)
#define u16REG_RESERVED_7       BK_REG(0x6B)

#define u16REG_PWM1_DUTY        BK_REG(0x6C)
#define u16REG_RESERVED_6       BK_REG(0x6C)

#define u16REG_PWM1_PERIOD      BK_REG(0x6D)
#define u16REG_RESERVED_5       BK_REG(0x6D)

//PWM2
#define u16REG_PWM2_DIV         BK_REG(0x78)
#define u16REG_RESERVED_A       BK_REG(0x78)

#define u16REG_PWM2_DUTY        BK_REG(0x79)
#define u16REG_RESERVED_9       BK_REG(0x79)

#define u16REG_PWM2_PERIOD      BK_REG(0x7A)
#define u16REG_RESERVED_8       BK_REG(0x7A)

//PWM3
#define u16REG_PWM3_PERIOD      BK_REG(0x7B)
#define u16REG_RESERVED_B       BK_REG(0x7B)

#define u16REG_PWM3_DUTY        BK_REG(0x7C)
#define u16REG_RESERVED_C       BK_REG(0x7C)

#define u16REG_PWM3_DIV         BK_REG(0x7D)
#define u16REG_RESERVED_D       BK_REG(0x7D)

/////////////////////////////////////////

//#define DEFAULT_PWM_ID

// For future reference
#define DEFAULT_PWM_CLK         12000000
#define DEFAULT_DIV_CNT         0// 0.000001 X 250 = 0.00025 sec = 0.25 ms
#define DEFAULT_PERIOD          101//=0X2EE0 => 1ms{1000HZ=12000000/12000}


#define DEFAULT_POLARITY        1   // 1 => 0 - L - duty - H - period, 0 => 0 - H - duty - L - period
#define DEFAULT_DBEN            0   // double buffer for Period Reset

#define PWM0                    0x00
#define PWM1                    0x01
#define PWM2                    0x02
#define PWM3                    0x03
#define PWM4                    0x04
#define PWM5                    0x05
#define PMW_DEFAULT             0xFF

//PWMX_CTRL
#define PWM_CTRL_DIV_MSAK           0x000000FF
#define PWM_CTRL_POLARITY           0x00000100
#define PWM_CTRL_DBEN               0x00001000

//Mask
#define PWM_PERIOD_MASK             0x0000FFFF
#define PWM_DUTY_MASK               PWM_PERIOD_MASK
#define PWM_CTRL_MASK               0x000015FF //0001 0101 1111 1111, BIT9,11,12 will not be tested

//------------------------------------------------------------------------------
//  Export Functions
//------------------------------------------------------------------------------
//void DrvBoostInit(void);
//void DrvBoostReset(void);
void DrvPWMInit(U8 u8Id);
//void DrvPWMReset(void);
//void DrvBacklightSet(U8 u8Level, U8 u8IsSave);
//U8 DrvBacklightGet(void);
//void DrvBacklightOn(void);
//void DrvBacklightOff(void);
//void DrvPWMSetEn(U8 u8Id, U8 u8Val);
void DrvPWMSetPeriod( U8 u8Id, U16 u16Val );
void DrvPWMSetDuty( U8 u8Id, U16 u16Val );
void DrvPWMEnable( U8 u8Id, U8 u8Val);
void DrvPWMSetPolarity( U8 u8Id, U8 u8Val );

//+++[Only4I6e]
int DrvPWMGroupGetRoundNum(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U16* u16Val);
int DrvPWMGroupGetHoldM1(struct mstar_pwm_chip *ms_chip);
int DrvPWMGroupHoldM1(struct mstar_pwm_chip *ms_chip, U8 u8Val);
int DrvPWMDutyQE0(struct mstar_pwm_chip *ms_chip, U8 u8GroupId, U8 u8Val);
int DrvPWMGetOutput(struct mstar_pwm_chip *ms_chip, U8* pu8Output);
//---[Only4I6e]

//-----------------------------------------------------------------------------



#endif  //__PWM_H
