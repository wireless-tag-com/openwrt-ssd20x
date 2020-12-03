/*
* mhal_pwm.c- Sigmastar
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
#include "mhal_pwm.h"
#include "gpio.h"
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of_irq.h>
#include <linux/time.h>
#include <linux/of_irq.h>

//------------------------------------------------------------------------------
//  Variables
//------------------------------------------------------------------------------
#define BASE_REG_NULL 0xFFFFFFFF

typedef struct
{
    u32         u32Adr;
    u32         u32Val;
    u32         u32Msk;
} regSet_t;

typedef struct
{
    u32         u32PadId;
    regSet_t    regSet[2];
} pwmPadTbl_t;

static pwmPadTbl_t padTbl_0[] =
{
    { PAD_PWM0,         { { BASE_REG_CHIPTOP_PA + REG_ID_07, (1 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  0), GENMASK(1, 0) } } },
    { PAD_GPIO14,       { { BASE_REG_CHIPTOP_PA + REG_ID_07, (2 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  0), GENMASK(1, 0) } } },
    { PAD_FUART_RX,     { { BASE_REG_CHIPTOP_PA + REG_ID_07, (3 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  0), GENMASK(1, 0) } } },
    { PAD_GPIO0,        { { BASE_REG_CHIPTOP_PA + REG_ID_07, (4 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  0), GENMASK(1, 0) } } },
    //// { PAD_SD1_IO0,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (5 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  0), GENMASK(1, 0) } } },
    // { PAD_PM_GPIO0,     { { BASE_REG_CHIPTOP_PA + REG_ID_07, (0 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (1 <<  0), GENMASK(1, 0) } } },
    { PAD_PM_GPIO0,     { { BASE_REG_NULL, 0, 0 },                                      { BASE_REG_PMSLEEP_PA + REG_ID_28, (1 <<  0), GENMASK(1, 0) } } },
    { PAD_UNKNOWN,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (0 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  0), GENMASK(1, 0) } } },
};

static pwmPadTbl_t padTbl_1[] =
{
    { PAD_PWM1,         { { BASE_REG_CHIPTOP_PA + REG_ID_07, (1 <<  3), GENMASK(5, 3)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  2), GENMASK(3, 2) } } },
    { PAD_GPIO15,       { { BASE_REG_CHIPTOP_PA + REG_ID_07, (2 <<  3), GENMASK(5, 3)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  2), GENMASK(3, 2) } } },
    { PAD_FUART_TX,     { { BASE_REG_CHIPTOP_PA + REG_ID_07, (3 <<  3), GENMASK(5, 3)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  2), GENMASK(3, 2) } } },
    { PAD_GPIO1,        { { BASE_REG_CHIPTOP_PA + REG_ID_07, (4 <<  3), GENMASK(5, 3)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  2), GENMASK(3, 2) } } },
    //// { PAD_SD1_IO1,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (5 <<  3), GENMASK(5, 3)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  2), GENMASK(3, 2) } } },
    // { PAD_PM_GPIO1,     { { BASE_REG_CHIPTOP_PA + REG_ID_07, (0 <<  3), GENMASK(5, 3)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (1 <<  2), GENMASK(3, 2) } } },
    { PAD_PM_GPIO1,     { { BASE_REG_NULL, 0, 0 },                                      { BASE_REG_PMSLEEP_PA + REG_ID_28, (1 <<  2), GENMASK(3, 2) } } },
    { PAD_UNKNOWN,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (0 <<  3), GENMASK(5, 3)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  2), GENMASK(3, 2) } } },
};

static pwmPadTbl_t padTbl_2[] =
{
    { PAD_GPIO14,       { { BASE_REG_CHIPTOP_PA + REG_ID_07, (1 <<  6), GENMASK(8, 6)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  6), GENMASK(7, 6) } } },
    { PAD_FUART_CTS,    { { BASE_REG_CHIPTOP_PA + REG_ID_07, (2 <<  6), GENMASK(8, 6)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  6), GENMASK(7, 6) } } },
    //// { PAD_SD1_IO0,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (3 <<  6), GENMASK(8, 6)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  6), GENMASK(7, 6) } } },
    { PAD_GPIO2,        { { BASE_REG_CHIPTOP_PA + REG_ID_07, (4 <<  6), GENMASK(8, 6)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  6), GENMASK(7, 6) } } },
    //// { PAD_SD1_IO2,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (5 <<  6), GENMASK(8, 6)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  6), GENMASK(7, 6) } } },
    // { PAD_PM_GPIO2,     { { BASE_REG_CHIPTOP_PA + REG_ID_07, (0 <<  6), GENMASK(8, 6)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (1 <<  6), GENMASK(7, 6) } } },
    // { PAD_PM_GPIO9,     { { BASE_REG_CHIPTOP_PA + REG_ID_07, (0 <<  6), GENMASK(8, 6)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (2 <<  6), GENMASK(7, 6) } } },
    { PAD_PM_GPIO2,     { { BASE_REG_NULL, 0, 0 },                                      { BASE_REG_PMSLEEP_PA + REG_ID_28, (1 <<  6), GENMASK(7, 6) } } },
    { PAD_PM_GPIO9,     { { BASE_REG_NULL, 0, 0 },                                      { BASE_REG_PMSLEEP_PA + REG_ID_28, (2 <<  6), GENMASK(7, 6) } } },
    { PAD_UNKNOWN,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (0 <<  6), GENMASK(8, 6)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  6), GENMASK(7, 6) } } },
};

static pwmPadTbl_t padTbl_3[] =
{
    { PAD_GPIO15,       { { BASE_REG_CHIPTOP_PA + REG_ID_07, (1 <<  9), GENMASK(11, 9)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  8), GENMASK(9, 8) } } },
    { PAD_FUART_RTS,    { { BASE_REG_CHIPTOP_PA + REG_ID_07, (2 <<  9), GENMASK(11, 9)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  8), GENMASK(9, 8) } } },
    //// { PAD_SD1_IO1,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (3 <<  9), GENMASK(11, 9)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  8), GENMASK(9, 8) } } },
    { PAD_GPIO3,        { { BASE_REG_CHIPTOP_PA + REG_ID_07, (4 <<  9), GENMASK(11, 9)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  8), GENMASK(9, 8) } } },
    //// { PAD_SD1_IO3,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (5 <<  9), GENMASK(11, 9)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  8), GENMASK(9, 8) } } },
    // { PAD_PM_GPIO3,     { { BASE_REG_CHIPTOP_PA + REG_ID_07, (0 <<  9), GENMASK(11, 9)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (1 <<  8), GENMASK(9, 8) } } },
    // { PAD_PM_GPIO7,     { { BASE_REG_CHIPTOP_PA + REG_ID_07, (0 <<  9), GENMASK(11, 9)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (2 <<  8), GENMASK(9, 8) } } },
    { PAD_PM_GPIO3,     { { BASE_REG_NULL, 0, 0 },                                       { BASE_REG_PMSLEEP_PA + REG_ID_28, (1 <<  8), GENMASK(9, 8) } } },
    { PAD_PM_GPIO7,     { { BASE_REG_NULL, 0, 0 },                                       { BASE_REG_PMSLEEP_PA + REG_ID_28, (2 <<  8), GENMASK(9, 8) } } },
    { PAD_UNKNOWN,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (0 <<  9), GENMASK(11, 9)}, { BASE_REG_PMSLEEP_PA + REG_ID_28, (0 <<  8), GENMASK(9, 8) } } },
};

static pwmPadTbl_t padTbl_4[] =
{
    //// { PAD_SD1_IO2,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (1 << 12), GENMASK(14, 12)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  0), GENMASK(0, 0) } } },
    { PAD_SPI0_CZ,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (2 << 12), GENMASK(14, 12)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  0), GENMASK(0, 0) } } },
    { PAD_GPIO4,        { { BASE_REG_CHIPTOP_PA + REG_ID_07, (3 << 12), GENMASK(14, 12)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  0), GENMASK(0, 0) } } },
    //// { PAD_SD1_IO4,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (4 << 12), GENMASK(14, 12)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  0), GENMASK(0, 0) } } },
    // { PAD_PM_LED0,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (0 << 12), GENMASK(14, 12)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (1 <<  0), GENMASK(0, 0) } } },
    { PAD_PM_LED0,      { { BASE_REG_NULL, 0, 0 },                                        { BASE_REG_PMSLEEP_PA + REG_ID_27, (1 <<  0), GENMASK(0, 0) } } },
    { PAD_UNKNOWN,      { { BASE_REG_CHIPTOP_PA + REG_ID_07, (0 << 12), GENMASK(14, 12)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  0), GENMASK(0, 0) } } },
};

static pwmPadTbl_t padTbl_5[] =
{
    //// { PAD_SD1_IO3,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (1 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  1), GENMASK(1, 1) } } },
    { PAD_SPI0_CK,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (2 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  1), GENMASK(1, 1) } } },
    { PAD_GPIO5,        { { BASE_REG_CHIPTOP_PA + REG_ID_02, (3 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  1), GENMASK(1, 1) } } },
    //// { PAD_SD1_IO5,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (4 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  1), GENMASK(1, 1) } } },
    // { PAD_PM_LED1,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (0 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (1 <<  1), GENMASK(1, 1) } } },
    { PAD_PM_LED1,      { { BASE_REG_NULL, 0, 0 },                                      { BASE_REG_PMSLEEP_PA + REG_ID_27, (1 <<  1), GENMASK(1, 1) } } },
    { PAD_UNKNOWN,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (0 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  1), GENMASK(1, 1) } } },
};

static pwmPadTbl_t padTbl_6[] =
{
    //// { PAD_SD1_IO4,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (1 <<  3), GENMASK(5, 3)}, { BASE_REG_NULL, 0, 0 } } },
    { PAD_SPI0_DI,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (2 <<  3), GENMASK(5, 3)}, { BASE_REG_NULL, 0, 0 } } },
    { PAD_GPIO6,        { { BASE_REG_CHIPTOP_PA + REG_ID_02, (3 <<  3), GENMASK(5, 3)}, { BASE_REG_NULL, 0, 0 } } },
    //// { PAD_SD1_IO6,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (4 <<  3), GENMASK(5, 3)}, { BASE_REG_NULL, 0, 0 } } },
    { PAD_UNKNOWN,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (0 <<  3), GENMASK(5, 3)}, { BASE_REG_NULL, 0, 0 } } },
};

static pwmPadTbl_t padTbl_7[] =
{
    //// { PAD_SD1_IO5,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (1 <<  6), GENMASK(8, 6)}, { BASE_REG_NULL, 0, 0 } } },
    { PAD_SPI0_DO,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (2 <<  6), GENMASK(8, 6)}, { BASE_REG_NULL, 0, 0 } } },
    { PAD_GPIO7,        { { BASE_REG_CHIPTOP_PA + REG_ID_02, (3 <<  6), GENMASK(8, 6)}, { BASE_REG_NULL, 0, 0 } } },
    //// { PAD_SD1_IO7,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (4 <<  6), GENMASK(8, 6)}, { BASE_REG_NULL, 0, 0 } } },
    { PAD_UNKNOWN,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (0 <<  6), GENMASK(8, 6)}, { BASE_REG_NULL, 0, 0 } } },
};

static pwmPadTbl_t padTbl_8[] =
{
    { PAD_GPIO0,        { { BASE_REG_CHIPTOP_PA + REG_ID_02, (1 <<  9), GENMASK(11, 9)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  2), GENMASK(2, 2) } } },
    { PAD_GPIO8,        { { BASE_REG_CHIPTOP_PA + REG_ID_02, (2 <<  9), GENMASK(11, 9)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  2), GENMASK(2, 2) } } },
    //// { PAD_SD1_IO8,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (3 <<  9), GENMASK(11, 9)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  2), GENMASK(2, 2) } } },
    //// { PAD_SR_IO14,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (4 <<  9), GENMASK(11, 9)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  2), GENMASK(2, 2) } } },
    // { PAD_PM_GPIO9,     { { BASE_REG_CHIPTOP_PA + REG_ID_02, (0 <<  9), GENMASK(11, 9)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (1 <<  2), GENMASK(2, 2) } } },
    { PAD_PM_GPIO9,     { { BASE_REG_NULL, 0, 0 },                                       { BASE_REG_PMSLEEP_PA + REG_ID_27, (1 <<  2), GENMASK(2, 2) } } },
    { PAD_UNKNOWN,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (0 <<  9), GENMASK(11, 9)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  2), GENMASK(2, 2) } } },
};

static pwmPadTbl_t padTbl_9[] =
{
    { PAD_GPIO1,        { { BASE_REG_CHIPTOP_PA + REG_ID_02, (1 << 12), GENMASK(14, 12)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  3), GENMASK(3, 3) } } },
    { PAD_PWM0,         { { BASE_REG_CHIPTOP_PA + REG_ID_02, (2 << 12), GENMASK(14, 12)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  3), GENMASK(3, 3) } } },
    { PAD_GPIO14,       { { BASE_REG_CHIPTOP_PA + REG_ID_02, (3 << 12), GENMASK(14, 12)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  3), GENMASK(3, 3) } } },
    //// { PAD_SR_IO15,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (4 << 12), GENMASK(14, 12)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  3), GENMASK(3, 3) } } },
    // { PAD_PM_LED0,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (0 << 12), GENMASK(14, 12)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (1 <<  3), GENMASK(3, 3) } } },
    { PAD_PM_LED0,      { { BASE_REG_NULL, 0, 0 },                                        { BASE_REG_PMSLEEP_PA + REG_ID_27, (1 <<  3), GENMASK(3, 3) } } },
    { PAD_UNKNOWN,      { { BASE_REG_CHIPTOP_PA + REG_ID_02, (0 << 12), GENMASK(14, 12)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  3), GENMASK(3, 3) } } },
};

static pwmPadTbl_t padTbl_10[] =
{
    //// { PAD_SD1_IO2,      { { BASE_REG_CHIPTOP_PA + REG_ID_04, (1 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  4), GENMASK(4, 4) } } },
    { PAD_SPI0_CZ,      { { BASE_REG_CHIPTOP_PA + REG_ID_04, (2 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  4), GENMASK(4, 4) } } },
    { PAD_GPIO4,        { { BASE_REG_CHIPTOP_PA + REG_ID_04, (3 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  4), GENMASK(4, 4) } } },
    //// { PAD_SD1_IO4,      { { BASE_REG_CHIPTOP_PA + REG_ID_04, (4 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  4), GENMASK(4, 4) } } },
    // { PAD_PM_LED0,      { { BASE_REG_CHIPTOP_PA + REG_ID_04, (0 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (1 <<  4), GENMASK(4, 4) } } },
    { PAD_PM_LED1,      { { BASE_REG_NULL, 0, 0 },                                      { BASE_REG_PMSLEEP_PA + REG_ID_27, (1 <<  4), GENMASK(4, 4) } } },
    { PAD_UNKNOWN,      { { BASE_REG_CHIPTOP_PA + REG_ID_04, (0 <<  0), GENMASK(2, 0)}, { BASE_REG_PMSLEEP_PA + REG_ID_27, (0 <<  4), GENMASK(4, 4) } } },
};

static pwmPadTbl_t* padTbl[] =
{
    padTbl_0,
    padTbl_1,
    padTbl_2,
    padTbl_3,
    padTbl_4,
    padTbl_5,
    padTbl_6,
    padTbl_7,
    padTbl_8,
    padTbl_9,
    padTbl_10,
};

static U8 _pwmEnSatus[PWM_NUM] = { 0 };
static U32 _pwmPeriod[PWM_NUM] = { 0 };
static U8 _pwmPolarity[PWM_NUM] = { 0 };
static U32 _pwmDuty[PWM_NUM][4] = {{0}}; //end  ( hardware support 4 set of duty )
static U8 _pwmDutyId[PWM_NUM] = { 0 }; 
static U32 _pwmShft[PWM_NUM][4] = {{0}}; //begin ( hardware support 4 set of shift )
static U8 _pwmShftId[PWM_NUM] = { 0 }; 
static U32 _pwmFreq[PWM_NUM] = { 0 };
static bool isSync=1; // isSync=0 --> need to sync register data from mem 
//static bool isEmptyRnd=0; //for get round num
//------------------------------------------------------------------------------
//  Local Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  External Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Global Functions
//------------------------------------------------------------------------------

//matt
#define  PWM_GROUP0_HOLD_INT        (1<<0)
#define  PWM_GROUP1_HOLD_INT        (1<<1)
#define  PWM_GROUP2_HOLD_INT        (1<<2)
#define  PWM_GROUP0_ROUND_INT       (1<<3)
#define  PWM_GROUP1_ROUND_INT       (1<<4)
#define  PWM_GROUP2_ROUND_INT       (1<<5)

int DrvPWMSetEndToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8DutyId, U32 u32Duty[3][4]);
int DrvPWMSetBeginToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8ShftId, U32 u32Shft[3][4]);
int DrvPWMSetPolarityExToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val);
void DrvPWMSetPeriodExToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Period);

unsigned long long timeStart,timeStop,timeVal;

static unsigned long long  getCurNs(void)
{
    struct timespec64 tv;
    unsigned long long curNs;
    
    getnstimeofday64(&tv);
    //printk("tv_sec= %lld\n",tv.tv_sec);
    //printk("tv_nsec= %ld\n",tv.tv_nsec);
    //printk("curNs = %lld\n",curNs);
    //printk("curNs = %lld\n",curNs);
    curNs = tv.tv_nsec;
    curNs += tv.tv_sec * 1000000000;
    return curNs;
}
    
    
void showBit(void *dev)
{
    struct mstar_pwm_chip *ms_chip = (struct mstar_pwm_chip*) dev;
    U16 test;
    test=INREG16(ms_chip->base +u16REG_PWM_INT )&0x3;
    (test&(1<<1))?(printk(" [ round int=1 ]\n")):(printk(" [ round int=0 ]\n"));
    (test&(1<<0))?(printk(" [ hold int=1 ]\n\n")):(printk(" [ hold int=0 ]\n\n")); 
}
void MDEV_PWM_MemToReg(struct mstar_pwm_chip* ms_chip, U8 u8Id)
{
    U8 u8GroupId=u8Id,pwmId;
    int idx;
    for(idx=0;idx<4;idx++)
    {
        pwmId=(u8GroupId<<2)+idx; //idx+(groupid*4)
        if(pwmId<PWM_NUM)
        {
            DrvPWMSetPolarityExToReg(ms_chip, pwmId ,_pwmPolarity[pwmId]);
            DrvPWMSetPeriodExToReg(ms_chip, pwmId , _pwmPeriod[pwmId]);
            DrvPWMSetBeginToReg(ms_chip, pwmId,_pwmShftId[pwmId],_pwmShft);
            DrvPWMSetEndToReg(ms_chip, pwmId ,_pwmDutyId[pwmId],_pwmDuty);	
        }
    }
}
void MDev_PWM_isr_round_Act(void *dev,U16 u16Id)
{
    struct mstar_pwm_chip *ms_chip = (struct mstar_pwm_chip*) dev;
    U16 group_round_id=u16Id;
    U32 u32Reg;
    
    timeStop=getCurNs(); //ns
    
    if(isSync) //no new data
    {
        CLRREG16(ms_chip->base + REG_GROUP_ENABLE, (1 << (group_round_id+ REG_GROUP_ENABLE_SHFT))); 
        u32Reg = (group_round_id << 0x7) + 0x40; // round bit
        OUTREG16(ms_chip->base + u32Reg, 0); // set round=0
        printk(" \nNONE of ROUND ! \n");  
        showBit(ms_chip);
        printk("Please open enable again !\n\n");  
        //isEmptyRnd=1;
        
        timeVal=(timeStop-timeStart)*_pwmFreq[group_round_id];
        timeVal/=1000000000;
	//do_div(timeVal,1000000000);
        printk("round take %lld times \n",timeVal);
        

    }    
    else //have new data
    {
        printk("Create New Round  !\n");
        CLRREG16(ms_chip->base + REG_GROUP_ENABLE, (1 << (group_round_id+ REG_GROUP_ENABLE_SHFT)));
        MDEV_PWM_MemToReg(ms_chip, group_round_id);
        /*
        DrvPWMSetPolarityExToReg(ms_chip,group_round_id,_pwmPolarity[group_round_id]);
        DrvPWMSetPeriodExToReg(ms_chip, group_round_id, _pwmPeriod[group_round_id]);
        DrvPWMSetBeginToReg(ms_chip, group_round_id,_pwmShftId[group_round_id],_pwmShft);
        DrvPWMSetEndToReg(ms_chip,group_round_id,_pwmDutyId[group_round_id],_pwmDuty);	
        */
        SETREG16(ms_chip->base + REG_GROUP_ENABLE, (1 << (group_round_id + REG_GROUP_ENABLE_SHFT)));
        isSync=1;
        printk("You can set Parameter now !\n");

        
        timeStart=getCurNs(); //new start round ms 
    }
}

void MDev_PWM_isr_hold_Act(void *dev,U16 val,U16 u16Id)
{ 
    U16 group_hold_id=u16Id;
    U16 group_hold_mode=val;
    struct mstar_pwm_chip *ms_chip = (struct mstar_pwm_chip*) dev;


    if(group_hold_mode==0)
    {
    		// (3.1) if mode 0
    		//     function, group_round_status --> return group_round_status[group_id];
    		// I6b0 --> period/begin/end/polar --> arguement in memory to HW register
        MDEV_PWM_MemToReg(ms_chip, group_hold_id);
        printk("3.1 done\n");       		
    }
    else if (group_hold_mode==1)
    {
    		// (3.2) if mode 1, do nothing and return
        printk("3.2 done\n");
    }
}

irqreturn_t PWM_IRQ(int irq, void *dev)
{
    struct mstar_pwm_chip *ms_chip = (struct mstar_pwm_chip*) dev;
    U16 group_id,group_hold_mode;

        // (1) read 0x75 --> reg_pwm_int, group_id = which group meet; both hold+round
    group_id = INREG16(ms_chip->base + u16REG_PWM_INT)&0x3F;
        // (2) read 0x77 --> hold mode 0 or 1, (?) each group should have its own mode, not share one mode (check with Ray and report to Oliver)	
    group_hold_mode = INREG16(ms_chip->base + u16REG_PWM_HOLD_MODE1) & BIT0;

    // 3 types of isr below.
    ///////////////////////////////////////////////////////
    // Part 1: use group_enable without sync. 
    /*
    if(group_id&PWM_GROUP0_HOLD_INT && group_id&PWM_GROUP0_ROUND_INT)
    {
        if (!(INREG16(ms_chip->base + REG_GROUP_JOIN)&0xF))
        {
            printk("Please sync first !");
            return IRQ_HANDLED;
        }
    }
    if(group_id&PWM_GROUP1_HOLD_INT && group_id&PWM_GROUP1_ROUND_INT)
    {
        if (!(INREG16(ms_chip->base + REG_GROUP_JOIN)&0xF0))
        {
            printk("Please sync first !");
            return IRQ_HANDLED;
        }
    }
    if(group_id&PWM_GROUP2_HOLD_INT && group_id&PWM_GROUP2_ROUND_INT)
    {
        if (!(INREG16(ms_chip->base + REG_GROUP_JOIN)&0x700)
        {
            printk("Please sync first !");
            return IRQ_HANDLED;
        }
    }
    */
    ///////////////////////////////////////////////////////
    // Part 2 : Hold interrupt isr
    
    if(group_id&PWM_GROUP0_HOLD_INT)
    {
        // (3)
        MDev_PWM_isr_hold_Act(ms_chip,group_hold_mode,0);
        // (4) 0x71 --> hold release
        CLRREG16(ms_chip->base + REG_GROUP_HOLD, (1 << (0+ REG_GROUP_HOLD_SHFT)));
        printk("Release hold mode done 1-0 !\n");
        showBit(ms_chip);
        isSync=1;
    }
    if(group_id&PWM_GROUP1_HOLD_INT)
    {
        MDev_PWM_isr_hold_Act(ms_chip,group_hold_mode,1);
        CLRREG16(ms_chip->base + REG_GROUP_HOLD, (1 << (1+ REG_GROUP_HOLD_SHFT)));
        printk("Release hold mode done 1-1 !\n");
        showBit(ms_chip);
        isSync=1;
    }
    if(group_id&PWM_GROUP2_HOLD_INT)
    {
        MDev_PWM_isr_hold_Act(ms_chip,group_hold_mode,2);
        CLRREG16(ms_chip->base + REG_GROUP_HOLD, (1 << (2+ REG_GROUP_HOLD_SHFT)));
        printk("Release hold mode done 1-2 !\n");
        showBit(ms_chip);
        isSync=1;
    }    

    ///////////////////////////////////////////////////////
    // Part 3 : Round interrupt isr
    
    if(group_id&PWM_GROUP0_ROUND_INT)
    {
        printk("2-0 done \n");
        MDev_PWM_isr_round_Act(ms_chip,0);   
    }
    if(group_id&PWM_GROUP1_ROUND_INT)
    {
        printk("2-1 done \n");
        MDev_PWM_isr_round_Act(ms_chip,1);
    }
    if(group_id&PWM_GROUP2_ROUND_INT)
    {
        printk("2-2 done \n");
        MDev_PWM_isr_round_Act(ms_chip,2);
    }

    return IRQ_HANDLED;
        
}

//+++[Only4I6e]
void MDEV_PWM_AllGrpEnable(struct mstar_pwm_chip *ms_chip)
{
    //Dummy func
}
//---[Only4I6e]

void DrvPWMInit(struct mstar_pwm_chip *ms_chip, U8 u8Id)
{
    U32 reset, u32Period;

    if (PWM_NUM <= u8Id)
        return;

    reset = INREG16(ms_chip->base + u16REG_SW_RESET) & (BIT0<<u8Id);
    DrvPWMGetPeriod(ms_chip, u8Id, &u32Period);
    if ((0 == reset) && (u32Period))
    {
        _pwmEnSatus[u8Id] = 1;
    }
    else
    {
        DrvPWMEnable(ms_chip, u8Id, 0);
    }
}

void DrvPWMSetDuty(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Val)
{
    U32 u32Period = 0x00000000;
    U32 u32Duty = 0x00000000;

    if (PWM_NUM <= u8Id)
        return;

    if (_pwmEnSatus[u8Id])
    {
        if (0 == u32Val)
            OUTREGMSK16(ms_chip->base + u16REG_SW_RESET, BIT0<<u8Id, BIT0<<u8Id);
    }

    u32Period = _pwmPeriod[u8Id];
    u32Duty = ((u32Period * u32Val) / 100);

    pr_err("reg=0x%08X clk=%d, u32Duty=0x%x\n", (U32)(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L), (U32)(clk_get_rate(ms_chip->clk)), u32Duty);
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L, (u32Duty&0xFFFF));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_H, ((u32Duty>>16)&0x3));

    if (_pwmEnSatus[u8Id])
    {
        U32 reset = INREG16(ms_chip->base + u16REG_SW_RESET) & (BIT0<<u8Id);
        if (u32Val && reset)
            CLRREG16(ms_chip->base + u16REG_SW_RESET, 1<<u8Id);
    }
}

void DrvPWMGetDuty(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32* pu32Val)
{
    U32 u32Duty;

    *pu32Val = 0;
    if (PWM_NUM <= u8Id)
        return;
    u32Duty = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L) | ((INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_H) & 0x3) << 16);
    if (u32Duty)
    {
        U32 u32Period = _pwmPeriod[u8Id];
        // DrvPWMGetPeriod(ms_chip, u8Id, &u32Period);
        if (u32Period)
        {
            *pu32Val = (u32Duty * 100)/u32Period;
        }
    }
}

//------------------------------------------------------------------------------
//
//  Function:   DrvPWMSetPeriod
//
//  Description
//      Set Period value
//
//  Parameters
//      u8Id:    [in] PWM ID
//      u16Val:  [in] Period value
//
//  Return Value
//      None
//
void DrvPWMSetPeriod(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Val)
{
    U32 u32Period = 0x00000000;

    u32Period=(U32)(clk_get_rate(ms_chip->clk))/u32Val;

    //[APN] range 2<=Period<=262144
    if(u32Period < 2)
        u32Period = 2;
    if(u32Period > 262144)
        u32Period = 262144;
    //[APN] PWM _PERIOD= (REG_PERIOD+1)
    u32Period--;

    pr_err("reg=0x%08X clk=%d, period=0x%x\n", (U32)(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L), (U32)(clk_get_rate(ms_chip->clk)), u32Period);

    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L, (u32Period&0xFFFF));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_H, ((u32Period>>16)&0x3));

    _pwmPeriod[u8Id] = u32Period;
}

void DrvPWMGetPeriod(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32* pu32Val)
{
    U32 u32Period;

    u32Period = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L) | ((INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_H) & 0x3) << 16);
    if ((0 == _pwmPeriod[u8Id]) && (u32Period))
    {
        _pwmPeriod[u8Id] = u32Period;
    }
    *pu32Val = 0;
    if (u32Period)
    {
        *pu32Val = (U32)(clk_get_rate(ms_chip->clk))/(u32Period+1);
    }
}

//------------------------------------------------------------------------------
//
//  Function:   DrvPWMSetPolarity
//
//  Description
//      Set Polarity value
//
//  Parameters
//      u8Id:   [in] PWM ID
//      u8Val:  [in] Polarity value
//
//  Return Value
//      None
//
void DrvPWMSetPolarity(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    OUTREGMSK16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (u8Val<<POLARITY_BIT), (0x1<<POLARITY_BIT));
}

void DrvPWMGetPolarity(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8* pu8Val)
{
    *pu8Val = (INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL) & (0x1<<POLARITY_BIT)) ? 1 : 0;
}

#if 0
void DrvPWMSetFreqDiv( U8 u8Id, U8 u8Val )
{
    if( PWM0 == u8Id )
    {
        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM0_DIV, u8Val, PWM_CTRL_DIV_MSAK );
    }
    else if( PWM1 == u8Id )
    {
        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM1_DIV, u8Val, PWM_CTRL_DIV_MSAK );
    }
    else if( PWM2 == u8Id )
    {
        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM2_DIV, u8Val, PWM_CTRL_DIV_MSAK );
    }
    else if( PWM3 == u8Id )
    {
        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM3_DIV, u8Val, PWM_CTRL_DIV_MSAK );
    }
    else
    {
        printk(KERN_ERR "void DrvPWMSetDiv error!!!! (%x, %x)\r\n", u8Id, u8Val);
    }

}
#endif

//------------------------------------------------------------------------------
//
//  Function:   DrvPWMSetDben
//
//  Description
//      Enable/Disable Dben function
//
//  Parameters
//      u8Id:   [in] PWM ID
//      u8Val:  [in] On/Off value
//
//  Return Value
//      None
//

void DrvPWMSetDben(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    OUTREGMSK16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (u8Val<<DBEN_BIT), (0x1<<DBEN_BIT));
    OUTREGMSK16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (u8Val<<VDBEN_SW_BIT), (0x1<<VDBEN_SW_BIT));
}

void DrvPWMEnable(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    if (PWM_NUM <= u8Id)
        return;
    DrvPWMSetDben(ms_chip, u8Id, 1);

    if(u8Val)
    {
        U32 u32DutyL = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L);
        U32 u32DutyH = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_H);
        if (u32DutyL || u32DutyH)
        {
            CLRREG16(ms_chip->base + u16REG_SW_RESET, 1<<u8Id);
        }
        else
        {
            SETREG16(ms_chip->base + u16REG_SW_RESET, 1<<u8Id);
        }
    }
    else
    {
        SETREG16(ms_chip->base + u16REG_SW_RESET, 1<<u8Id);
    }
    _pwmEnSatus[u8Id] = u8Val;
}

void DrvPWMEnableGet(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8* pu8Val)
{
    *pu8Val = 0;
    if (PWM_NUM <= u8Id)
        return;
    *pu8Val = _pwmEnSatus[u8Id];
}

/*
void DrvPWMPad_dump(void)
{
    int i;
    for (i = 0; i < pwmNum; i++)
    {
        pwmPadTbl_t* pTbl = padTbl[i];
        printk("[%s][%d] %d ------------------------------\n", __FUNCTION__, __LINE__, i);
        while (1)
        {
            regSet_t* pRegSet = pTbl->regSet;
            printk("[%s][%d]     ******************************\n", __FUNCTION__, __LINE__);
            printk("[%s][%d]         pad Id = %d\n", __FUNCTION__, __LINE__, pTbl->u32PadId);
            printk("[%s][%d]         (reg, val, msk) = (0x%08x, 0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__, pRegSet[0].u32Adr, pRegSet[0].u32Val, pRegSet[0].u32Msk);
            printk("[%s][%d]         (reg, val, msk) = (0x%08x, 0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__, pRegSet[1].u32Adr, pRegSet[1].u32Val, pRegSet[1].u32Msk);
            if (PAD_UNKNOWN == pTbl->u32PadId)
            {
                break;
            }
            pTbl++;
        }
    }
}
*/

void DrvPWMPadSet(U8 u8Id, U8 u8Val)
{
    pwmPadTbl_t* pTbl = NULL;
    if (PWM_NUM <= u8Id)
    {
        // printk(KERN_ERR "[%s][%d] void DrvPWMEnable error!!!! (%x, %x)\r\n", __FUNCTION__, __LINE__, u8Id, u8Val);
        return;
    }
    // printk("[%s][%d] (pwmId, padId) = (%d, %d)\n", __FUNCTION__, __LINE__, u8Id, u8Val);
    pTbl = padTbl[u8Id];
    while (1)
    {
        if (u8Val == pTbl->u32PadId)
        {
            regSet_t* pRegSet = pTbl->regSet;
            if (BASE_REG_NULL != pRegSet[0].u32Adr)
            {
                OUTREGMSK16(pRegSet[0].u32Adr, pRegSet[0].u32Val, pRegSet[0].u32Msk);
            }
            if (BASE_REG_NULL != pRegSet[1].u32Adr)
            {
                OUTREGMSK16(pRegSet[1].u32Adr, pRegSet[1].u32Val, pRegSet[1].u32Msk);
            }
            break;
        }
        if (PAD_UNKNOWN == pTbl->u32PadId)
        {
            printk(KERN_ERR "[%s][%d] void DrvPWMEnable error!!!! (%x, %x)\r\n", __FUNCTION__, __LINE__, u8Id, u8Val);
            break;
        }
        pTbl++;
    }
}

int DrvPWMGroupCap(void)
{
    return (PWM_GROUP_NUM) ? 1 : 0;
}

int DrvPWMGroupJoin(struct mstar_pwm_chip* ms_chip, U8 u8PWMId, U8 u8Val)
{
    if (PWM_NUM <= u8PWMId)
        return 0;
    if(u8Val)
        SETREG16(ms_chip->base + REG_GROUP_JOIN, (1 << (u8PWMId + REG_GROUP_JOIN_SHFT)));
    else
        CLRREG16(ms_chip->base + REG_GROUP_JOIN, (1 << (u8PWMId + REG_GROUP_JOIN_SHFT)));
    return 1;
}

int DrvPWMGroupEnable(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U8 u8Val)
{
    U32 u32JoinMask;

    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;
   
    if(!isSync)
    {
        MDEV_PWM_MemToReg(ms_chip, u8GroupId);
        printk("MEM to REG done !\n");    
    }

    u32JoinMask = 0xF << ((u8GroupId << 2) + REG_GROUP_JOIN_SHFT);
    u32JoinMask = INREG16(ms_chip->base + REG_GROUP_JOIN) & u32JoinMask;
    u32JoinMask |= 1 << (u8GroupId + PWM_NUM);

    if (u8Val)
    {
        SETREG16(ms_chip->base + REG_GROUP_ENABLE, (1 << (u8GroupId + REG_GROUP_ENABLE_SHFT)));
        CLRREG16(ms_chip->base + u16REG_SW_RESET, u32JoinMask);
        isSync=1; //dont need to sync until new data in
    }
    else
    {
        CLRREG16(ms_chip->base + REG_GROUP_ENABLE, (1 << (u8GroupId + REG_GROUP_ENABLE_SHFT))); //ori 
        SETREG16(ms_chip->base + u16REG_SW_RESET, u32JoinMask);
        
    }

    timeStart=getCurNs(); // start round ns 

    return 1;
}

int DrvPWMGroupIsEnable(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U8* pu8Val)
{
    *pu8Val = 0;
    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;
    *pu8Val = (INREG16(ms_chip->base + REG_GROUP_ENABLE) >> (u8GroupId + REG_GROUP_ENABLE_SHFT)) & 0x1;
    return 1;
}

int DrvPWMGroupGetRoundNum(struct mstar_pwm_chip* ms_chip, U8 u8PwmId, U16* u16Val)
{
    timeStop=getCurNs(); //ns	
    
    if (PWM_NUM <= u8PwmId)
        return 0;
/*
    printk("timeStart= %lld \n",timeStart);
    printk("timeStop= %lld \n",timeStop);
    printk("period=%d \n",_pwmPeriod[roundNum]);
    printk("freq=%d \n",_pwmFreq[roundNum]);
    printk("bebore divide %lld \n",(timeStop-timeStart));
    printk("after divide %lld \n",(timeStop-timeStart)/1000000000);
*/
    //if(isEmptyRnd)
    //    printk("None of Round\n");
    //else
    //{

        printk("freq=%d \n",_pwmFreq[u8PwmId]);
        timeVal=(timeStop-timeStart)*_pwmFreq[u8PwmId];
        timeVal/=1000000000;
        //do_div(timeVal,1000000000);
        printk("round take %lld times \n",timeVal);
    //}
    return 1;
}

int DrvPWMGroupSetRound(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U16 u16Val)
{
    U32 u32Reg;

    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;
    u32Reg = (u8GroupId << 0x7) + 0x40; //(GroupId * 0x20<<2) + 0x10<<2
    OUTREG16(ms_chip->base + u32Reg, u16Val);

    //isEmptyRnd=0;
    timeStart=getCurNs(); //new start round ns 
    return 1;
}

int DrvPWMGroupStop(struct mstar_pwm_chip *ms_chip, U8 u8GroupId, U8 u8Val)
{
    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;

    if (u8Val)
        SETREG16(ms_chip->base + REG_GROUP_STOP, (1 << (u8GroupId + REG_GROUP_STOP_SHFT)));
    else
        CLRREG16(ms_chip->base + REG_GROUP_STOP, (1 << (u8GroupId + REG_GROUP_STOP_SHFT)));

    return 1;
}

int DrvPWMGroupHold(struct mstar_pwm_chip *ms_chip, U8 u8GroupId, U8 u8Val)
{
    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;

    if (u8Val)
        SETREG16(ms_chip->base + REG_GROUP_HOLD, (1 << (u8GroupId + REG_GROUP_HOLD_SHFT)));
    else
        CLRREG16(ms_chip->base + REG_GROUP_HOLD, (1 << (u8GroupId + REG_GROUP_HOLD_SHFT)));

    return 1;
}
//+++[Only4I6e]
int DrvPWMGroupGetHoldM1(struct mstar_pwm_chip *ms_chip)
{
#if 0
    return INREG16(ms_chip->base + REG_GROUP_HOLD_MODE1);
#else
    //printk("\n[WARN][%s L%d] Only4i6e\n", __FUNCTION__, __LINE__);
    return 1;
#endif
}

int DrvPWMGroupHoldM1(struct mstar_pwm_chip *ms_chip, U8 u8Val)
{
#if 0
    if (u8Val) {
        SETREG16(ms_chip->base + REG_GROUP_HOLD_MODE1, 1);
        printk("[%s L%d] hold mode1 en!(keep low)\n", __FUNCTION__, __LINE__);
    }
    else {
        CLRREG16(ms_chip->base + REG_GROUP_HOLD_MODE1, 0);
        printk("[%s L%d] hold mode1 dis!\n", __FUNCTION__, __LINE__);
    }
#else
    //printk("\n[WARN][%s L%d] Only4i6e\n", __FUNCTION__, __LINE__);
#endif
    return 1;
}

int DrvPWMDutyQE0(struct mstar_pwm_chip *ms_chip, U8 u8GroupId, U8 u8Val)
{
#if 0
    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;

    printk("[%s L%d] grp:%d x%x(%d)\n", __FUNCTION__, __LINE__, u8GroupId, u8Val, u8Val);
    if (u8Val)
        SETREG16(ms_chip->base + REG_PWM_DUTY_QE0, (1 << (u8GroupId + REG_PWM_DUTY_QE0_SHFT)));
    else
        CLRREG16(ms_chip->base + REG_PWM_DUTY_QE0, (1 << (u8GroupId + REG_PWM_DUTY_QE0_SHFT)));
#else
    //printk("\n[WARN][%s L%d] Only4i6e id:%d\n", __FUNCTION__, __LINE__, u8GroupId);
#endif
    return 1;
}

int DrvPWMGetOutput(struct mstar_pwm_chip *ms_chip, U8* pu8Output)
{
#if 0
    *pu8Output = INREG16(ms_chip->base + REG_PWM_OUT);
    printk("[%s L%d] output:x%x\n", __FUNCTION__, __LINE__, *pu8Output);
#else
    //printk("\n[WARN][%s L%d] Only4i6e\n", __FUNCTION__, __LINE__);
#endif
    return 1;
}
//---[Only4I6e]
// I6b0 --> end --> arguement in memory
int DrvPWMSetEnd(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8DutyId, U32 u32Val)
{
    U32 u32Period;
    U32 u32Duty;

    if (PWM_NUM <= u8Id)
        return 0;

    // u32Period = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L) + ((INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_H)<<16));
    u32Period = _pwmPeriod[u8Id];
    u32Duty = ((u32Period * u32Val) / 1000);
    //matt
    _pwmDutyId[u8Id]=u8DutyId;
    _pwmDuty[u8Id][u8DutyId]=u32Duty;
    isSync=0;
    
    if (u32Duty & 0xFFFC0000)
    {
        printk("[%s][%d] too large duty 0x%08x (18 bits in max)\n", __FUNCTION__, __LINE__, u32Duty);
    }
    return 1;
}

int DrvPWMSetEndToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8DutyId, U32 u32Duty[3][4])
{
    int id;                     
    for(id=0; id<=u8DutyId;id++){	
        if (0 == id)
        {
            OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L, (u32Duty[u8Id][id]&0xFFFF));
            OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_H, ((u32Duty[u8Id][id]>> 16)&0x0003));
        }
        else
        {
            OUTREG16(ms_chip->base + (u8Id*0x80) + (id << 3) + 28, (u32Duty[u8Id][id]&0xFFFF));
        }
        return 1;
    }
return 1;
}

// I6b0 --> begin --> arguement in memory
int DrvPWMSetBegin(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8ShftId, U32 u32Val)
{
    U32 u32Period;
    U32 u32Shft;

    if (PWM_NUM <= u8Id)
        return 0;

    u32Period = _pwmPeriod[u8Id];
    u32Shft = ((u32Period * u32Val) / 1000);
//matt
    _pwmShftId[u8Id]=u8ShftId;
    _pwmShft[u8Id][u8ShftId]=u32Shft;
    isSync=0;
    
    if (u32Shft & 0xFFFC0000)
    {
        printk("[%s][%d] too large shift 0x%08x (18 bits in max)\n", __FUNCTION__, __LINE__, u32Shft);
    }

    return 1;
}

int DrvPWMSetBeginToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8ShftId, U32 u32Shft[3][4])
{
    int id;                         
    for(id=0; id<u8ShftId;id++){	
        if (0 == id)
        {
            OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_SHIFT_L, (u32Shft[u8Id][id]&0xFFFF));
            OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_SHIFT_H, ((u32Shft[u8Id][id]>> 16)&0x0003));
        }
        else
        {
            OUTREG16(ms_chip->base + (u8Id*0x80) + (id << 3) + 24, (u32Shft[u8Id][id]&0xFFFF));
        }
        return 1;		
    }
    return 1;	
}

// I6b0 --> polarity --> arguement in memory
int DrvPWMSetPolarityEx(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    if (PWM_NUM<= u8Id)
        return 0;
    //matt
    _pwmPolarity[u8Id]=u8Val;
    isSync=0;
	
    return 1;
}

int DrvPWMSetPolarityExToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    if (u8Val)
        SETREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (0x1<<POLARITY_BIT));
    else
        CLRREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (0x1<<POLARITY_BIT));
    return 1;
}

// I6b0 --> period --> arguement in memory
void DrvPWMSetPeriodEx(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Val)
{
    U32 u32Period;
    U32 u32Div;

    u32Div = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DIV); // workaround

    u32Period=(U32)(clk_get_rate(ms_chip->clk))/u32Val;
    u32Period /= (u32Div + 1); // workaround

    //[APN] range 2<=Period<=262144
    if(u32Period < 2)
        u32Period = 2;
    if(u32Period > 262144)
        u32Period = 262144;

    //[APN] PWM _PERIOD= (REG_PERIOD+1)
    u32Period--;
	
    //matt
    _pwmPeriod[u8Id] = u32Period;
    //_pwmFreq[u8Id] = (U32)(clk_get_rate(ms_chip->clk))/(_pwmPeriod[u8Id] * (u32Div+1));//for round number
    _pwmFreq[u8Id]=u32Val;
    isSync=0;

    pr_err("reg=0x%08X clk=%d, period=0x%x\n", (U32)(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L), (U32)(clk_get_rate(ms_chip->clk)), u32Period);

}

void DrvPWMSetPeriodExToReg(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Period)
{
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L, (u32Period&0xFFFF));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_H, ((u32Period>>16)&0x3));
}

#if 0
int DrvPWMSetMPluse(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    if (PWM_NUM <= u8Id)
        return 0;
    if (u8Val)
        SETREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (0x1<<DIFF_P_EN_BIT));
    else
        CLRREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (0x1<<DIFF_P_EN_BIT));
    return 1;
}
#endif

int DrvPWMDiv(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    if (PWM_NUM <= u8Id)
        return 0;
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DIV, u8Val);
    //matt
    printk("mhal  DrvPWMDiv done !\n");
        
    return 1;
}

int DrvPWMGroupInfo(struct mstar_pwm_chip *ms_chip, char* buf_start, char* buf_end)
{
    char *str = buf_start;
    char *end = buf_end;
    int i;
    // U32 tmp;
    U32 u32Period, u32Polarity; // , u32MPluse;
    // U32 u32Shft0, u32Shft1, u32Shft2, u32Shft3;
    U32 u32Shft0;
    // U32 u32Duty0, u32Duty1, u32Duty2, u32Duty3;
    U32 u32Duty0;
    U32 u32SyncStatus;
    U32 u32ResetStatus;
    U32 u32GroupEnable, u32GroupReset, u32GroupHold, u32GroupStop, u32GroupRound;
    U32 clk = (U32)clk_get_rate(ms_chip->clk);
    U32 u32Div;

    if (0 == DrvPWMGroupCap())
    {
        str += scnprintf(str, end - str, "This chip does not support motor interface\n");
        return (str - buf_start);
    }

    str += scnprintf(str, end - str, "================================================\n");
    for (i = 0; i < PWM_GROUP_NUM; i++)
    {
        U32 pwmIdx;
        U32 j;

        // group enable
        u32GroupEnable = (INREG16(ms_chip->base + REG_GROUP_ENABLE) >> i) & 0x1;
        // group reset 
        u32GroupReset = (INREG16(ms_chip->base + u16REG_SW_RESET) >> (i + PWM_NUM)) & 0x1;
        // hold
        u32GroupHold = (INREG16(ms_chip->base + REG_GROUP_HOLD) >> (i + REG_GROUP_HOLD_SHFT)) & 0x1;
        // stop
        u32GroupStop = (INREG16(ms_chip->base + REG_GROUP_STOP) >> (i + REG_GROUP_STOP_SHFT)) & 0x1;
        // round
        u32GroupRound = INREG16(ms_chip->base + ((i << 0x7) + 0x40)) & 0xFFFF;

        str += scnprintf(str, end - str, "Group %d\n", i);
        pwmIdx = (i << 2);
        str += scnprintf(str, end - str, "\tmember\t\t");
        for (j = pwmIdx; j < pwmIdx + 4; j++)
        {
            if (j < PWM_NUM)
            {
                str += scnprintf(str, end - str, "%d ", j);
            }
        }
        str += scnprintf(str, end - str, "\n");
        str += scnprintf(str, end - str, "\tenable status\t%d\n", u32GroupEnable);
        str += scnprintf(str, end - str, "\tReset status\t%d\n", u32GroupReset);
        str += scnprintf(str, end - str, "\tHold\t\t%d\n", u32GroupHold);
        str += scnprintf(str, end - str, "\tStop\t\t%d\n", u32GroupStop);
        str += scnprintf(str, end - str, "\tRound\t\t%d\n", u32GroupRound);
    }

    str += scnprintf(str, end - str, "================================================\n");
    for (i = 0; i < PWM_NUM; i++)
    {
        // Polarity
        u32Polarity = (INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_CTRL) >> POLARITY_BIT) & 0x1;
        // u32MPluse = (INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_CTRL) >> DIFF_P_EN_BIT) & 0x1;
        // Period
#if 0
        if ((tmp = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_PERIOD_H)))
            printk("[%s][%d] pwmId %d period_h is not zero (0x%08x)\n", __FUNCTION__, __LINE__, i, tmp);
#endif
        u32Period = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_PERIOD_L);
        // Shift
#if 0
        if ((tmp = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_SHIFT_H)))
            printk("[%s][%d] pwmId %d shift_h is not zero (0x%08x)\n", __FUNCTION__, __LINE__, i, tmp);
#endif
        u32Shft0 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_SHIFT_L);
        // u32Shft1 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_SHIFT2);
        // u32Shft2 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_SHIFT3);
        // u32Shft3 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_SHIFT4);
        // Duty
#if 0
        if ((tmp = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_DUTY_H)))
            printk("[%s][%d] pwmId %d duty_h is not zero (0x%08x)\n", __FUNCTION__, __LINE__, i, tmp);
#endif
        u32Duty0 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_DUTY_L);
        // u32Duty1 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_DUTY2);
        // u32Duty2 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_DUTY3);
        // u32Duty3 = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_DUTY4);
        // sync mode status
        u32SyncStatus = (INREG16(ms_chip->base + REG_GROUP_JOIN) >> (i + REG_GROUP_JOIN_SHFT)) & 0x1;
        // rest status
        u32ResetStatus = (INREG16(ms_chip->base + u16REG_SW_RESET) >> i) & 0x1;
        u32Div = INREG16(ms_chip->base + (i*0x80) + u16REG_PWM_DIV); // workaround


        // output to buffer
        str += scnprintf(str, end - str, "Pwm %d\n", i);
        str += scnprintf(str, end - str, "\tPad\t\t0x%08x\n", ms_chip->pad_ctrl[i]);
        str += scnprintf(str, end - str, "\tSync status\t%d\n", u32SyncStatus);
        str += scnprintf(str, end - str, "\tReset status\t%d\n", u32ResetStatus);
        str += scnprintf(str, end - str, "\tPolarity\t%d\n", u32Polarity);
#if 0
        str += scnprintf(str, end - str, "\tPeriod\t\t0x%08x\n", u32Period);
        str += scnprintf(str, end - str, "\tBegin\t\t0x%08x\n", u32Shft0);
        str += scnprintf(str, end - str, "\tEnd\t\t0x%08x\n", u32Duty0);
#endif
        u32Period++;
        u32Shft0++;
        u32Duty0++;
        u32Shft0 = (1000 * u32Shft0)/u32Period;
        u32Duty0 = (1000 * u32Duty0)/u32Period;
        // u32Period = ((u32Div+1)*clk)/u32Period;
        u32Period = clk/u32Period/(u32Div+1);

        str += scnprintf(str, end - str, "\tPeriod\t\t%d\n", u32Period);
        // str += scnprintf(str, end - str, "\tBegin\t\t0x%08x 0x%08x 0x%08x 0x%08x\n", u32Shft0, u32Shft1, u32Shft2, u32Shft3);
        // str += scnprintf(str, end - str, "\tEnd\t\t0x%08x 0x%08x 0x%08x 0x%08x\n", u32Duty0, u32Duty1, u32Duty2, u32Duty3);
        str += scnprintf(str, end - str, "\tBegin\t\t%d\n", u32Shft0);
        str += scnprintf(str, end - str, "\tEnd\t\t%d\n", u32Duty0);
    }
    // str += scnprintf(str, end - str, "This is a test\n");
    return (str - buf_start);
}
