/*
* mdrv_pwm.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: ken.chang <ken-ms.chang@sigmastar.com.tw>
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
#include "MsTypes.h"
#include <common.h>
#include "../gpio/infinity2m/gpio.h"

//------------------------------------------------------------------------------
//  Define
//------------------------------------------------------------------------------
#define BOOL    MS_BOOL
#define PWM_CLK                 12000000
#define PWM_GROUP_NUM           (1)
#define PWM_NUM                 (4)
//Common PWM registers
#define u16REG_PWM_SHIFT_L      (0x00*2)
#define u16REG_PWM_SHIFT_H      (0x01*2)
#define u16REG_PWM_DUTY_L       (0x02*2)
#define u16REG_PWM_DUTY_H       (0x03*2)
#define u16REG_PWM_PERIOD_L     (0x04*2) //reg_pwm0_period
#define u16REG_PWM_PERIOD_H     (0x05*2)
#define u16REG_PWM_DIV          (0x06*2)
#define u16REG_PWM_CTRL         (0x07*2)
    #define VDBEN_SW_BIT           0
    #define DBEN_BIT               1
    #define DIFF_P_EN_BIT          2
    #define SHIFT_GAT_BIT          3
    #define POLARITY_BIT           4

#define u16REG_PWM_SHIFT2       (0x08*2)
#define u16REG_PWM_DUTY2        (0x09*2)
#define u16REG_PWM_SHIFT3       (0x0A*2)
#define u16REG_PWM_DUTY3        (0x0B*2)
#define u16REG_PWM_SHIFT4       (0x0C*2)
#define u16REG_PWM_DUTY4        (0x0D*2)

#define u16REG_SW_RESET         (0x7F*2)
#define REG_CH_OFFSET           (0x20*2)

#define RIU_BASE_ADDR           0x1F000000

#define CHIPTOP_BANK_ADDR       0x101E00
#define BASE_REG_CHIPTOP_PA     RIU_BASE_ADDR+(CHIPTOP_BANK_ADDR*2)
#define REG_ID_07               (0x07*2)

#define PMSLEEP_BANK_ADDR       0x000E00
#define BASE_REG_PMSLEEP_PA     RIU_BASE_ADDR+(PMSLEEP_BANK_ADDR*2)
#define REG_ID_28               (0x28*2)

#define PWM_BANK_ADDR           0x001A00
#define BASE_REG_PWM_PA         RIU_BASE_ADDR+(PWM_BANK_ADDR*2)

#define BITS_PER_LONG           32
#define GENMASK(h, l)           (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define BASE_REG_NULL 0xFFFFFFFF
//------------------------------------------------------------------------------
//  Variables
//------------------------------------------------------------------------------
typedef struct
{
    U32         u32Adr;
    U32         u32Val;
    U32         u32Msk;
} regSet_t;

typedef struct
{
    U32         u32PadId;
    regSet_t    regSet[2];
} pwmPadTbl_t;

static pwmPadTbl_t padTbl_0[] =
{
    { PAD_GPIO0,        { { REG_ID_07, (1 <<  0), GENMASK(2, 0)}, { REG_ID_28, (0 <<  0), GENMASK(1, 0) } } },
    { PAD_TTL23,        { { REG_ID_07, (2 <<  0), GENMASK(2, 0)}, { REG_ID_28, (0 <<  0), GENMASK(1, 0) } } },
    { PAD_GPIO4,        { { REG_ID_07, (3 <<  0), GENMASK(2, 0)}, { REG_ID_28, (0 <<  0), GENMASK(1, 0) } } },
    { PAD_GPIO14,       { { REG_ID_07, (4 <<  0), GENMASK(2, 0)}, { REG_ID_28, (0 <<  0), GENMASK(1, 0) } } },
    { PAD_TTL0,         { { REG_ID_07, (5 <<  0), GENMASK(2, 0)}, { REG_ID_28, (0 <<  0), GENMASK(1, 0) } } },
    { PAD_PM_LED0,      { { REG_ID_07, (0 <<  0), GENMASK(2, 0)}, { REG_ID_28, (1 <<  0), GENMASK(1, 0) } } },
    { PAD_UNKNOWN,      { { REG_ID_07, (0 <<  0), GENMASK(2, 0)}, { REG_ID_28, (0 <<  0), GENMASK(1, 0) } } },
    { PAD_UNKNOWN,      { { REG_ID_07, (0 <<  0), GENMASK(2, 0)}, { REG_ID_28, (0 <<  0), GENMASK(1, 0) } } },
};

static pwmPadTbl_t padTbl_1[] =
{
    { PAD_GPIO1,        { { REG_ID_07, (1 <<  3), GENMASK(5, 3)}, { REG_ID_28, (0 <<  2), GENMASK(3, 2) } } },
    { PAD_TTL12,        { { REG_ID_07, (2 <<  3), GENMASK(5, 3)}, { REG_ID_28, (0 <<  2), GENMASK(3, 2) } } },
    { PAD_TTL22,        { { REG_ID_07, (3 <<  3), GENMASK(5, 3)}, { REG_ID_28, (0 <<  2), GENMASK(3, 2) } } },
    { PAD_GPIO5,        { { REG_ID_07, (4 <<  3), GENMASK(5, 3)}, { REG_ID_28, (0 <<  2), GENMASK(3, 2) } } },
    { PAD_SATA_GPIO,    { { REG_ID_07, (5 <<  3), GENMASK(5, 3)}, { REG_ID_28, (0 <<  2), GENMASK(3, 2) } } },
    { PAD_PM_LED1,      { { REG_ID_07, (0 <<  3), GENMASK(5, 3)}, { REG_ID_28, (1 <<  2), GENMASK(3, 2) } } },
    { PAD_UNKNOWN,      { { BASE_REG_NULL, 0, 0 },                { REG_ID_28, (0 <<  2), GENMASK(3, 2) } } },
    { PAD_UNKNOWN,      { { REG_ID_07, (0 <<  3), GENMASK(5, 3)}, { REG_ID_28, (0 <<  2), GENMASK(3, 2) } } },
};

static pwmPadTbl_t padTbl_2[] =
{
    { PAD_GPIO2,        { { REG_ID_07, (1 <<  6), GENMASK(8, 6)}, { REG_ID_28, (0 <<  6), GENMASK(7, 6) } } },
    { PAD_GPIO11,       { { REG_ID_07, (2 <<  6), GENMASK(8, 6)}, { REG_ID_28, (0 <<  6), GENMASK(7, 6) } } },
    { PAD_HDMITX_HPD,   { { REG_ID_07, (3 <<  6), GENMASK(8, 6)}, { REG_ID_28, (0 <<  6), GENMASK(7, 6) } } },
    { PAD_TTL21,        { { REG_ID_07, (4 <<  6), GENMASK(8, 6)}, { REG_ID_28, (0 <<  6), GENMASK(7, 6) } } },
    { PAD_FUART_TX,     { { REG_ID_07, (5 <<  6), GENMASK(8, 6)}, { REG_ID_28, (0 <<  6), GENMASK(7, 6) } } },
    { PAD_SD_D1,        { { REG_ID_07, (6 <<  6), GENMASK(8, 6)}, { REG_ID_28, (0 <<  6), GENMASK(7, 6) } } },
    { PAD_PM_IRIN,      { { REG_ID_07, (0 <<  6), GENMASK(8, 6)}, { REG_ID_28, (1 <<  6), GENMASK(7, 6) } } },
    { PAD_UNKNOWN,      { { BASE_REG_NULL, 0, 0 },                { REG_ID_28, (0 <<  6), GENMASK(7, 6) } } },
};

static pwmPadTbl_t padTbl_3[] =
{
    { PAD_GPIO3,        { { REG_ID_07, (1 <<  9), GENMASK(11, 9)}, { REG_ID_28, (0 <<  8), GENMASK(9, 8) } } },
    { PAD_GPIO7,        { { REG_ID_07, (2 <<  9), GENMASK(11, 9)}, { REG_ID_28, (0 <<  8), GENMASK(9, 8) } } },
    { PAD_GPIO12,       { { REG_ID_07, (3 <<  9), GENMASK(11, 9)}, { REG_ID_28, (0 <<  8), GENMASK(9, 8) } } },
    { PAD_TTL20,        { { REG_ID_07, (4 <<  9), GENMASK(11, 9)}, { REG_ID_28, (0 <<  8), GENMASK(9, 8) } } },
    { PAD_FUART_RX,     { { REG_ID_07, (5 <<  9), GENMASK(11, 9)}, { REG_ID_28, (0 <<  8), GENMASK(9, 8) } } },
    { PAD_PM_SD_CDZ,    { { REG_ID_07, (0 <<  9), GENMASK(11, 9)}, { REG_ID_28, (1 <<  8), GENMASK(9, 8) } } },
    { PAD_UNKNOWN,      { { BASE_REG_NULL, 0, 0 },                 { REG_ID_28, (0 <<  8), GENMASK(9, 8) } } },
    { PAD_UNKNOWN,      { { REG_ID_07, (0 <<  9), GENMASK(11, 9)}, { REG_ID_28, (0 <<  8), GENMASK(9, 8) } } },
};

static pwmPadTbl_t* padTbl[] =
{
    padTbl_0,
    padTbl_1,
    padTbl_2,
    padTbl_3,
};

static U8 _pwmEnSatus[PWM_NUM] = { 0 };
static U32 _pwmPeriod[PWM_NUM] = { 0 };

//------------------------------------------------------------------------------
//  Local Functions
//------------------------------------------------------------------------------
BOOL HAL_PWM_Write2Byte(U32 u32RegAddr, U16 u16Val)
{
    (*(volatile U32*)(BASE_REG_PWM_PA+((u32RegAddr & 0xFFFFFF00ul) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2))) = u16Val;
    return TRUE;
}

U16 HAL_PWM_Read2Byte(U32 u32RegAddr)
{
    return (*(volatile U32*)(BASE_REG_PWM_PA+((u32RegAddr & 0xFFFFFF00ul) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2)));
}

BOOL HAL_ChipTop_Write2Byte(U32 u32RegAddr, U16 u16Val)
{
    (*(volatile U32*)(BASE_REG_CHIPTOP_PA+((u32RegAddr & 0xFFFFFF00ul) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2))) = u16Val;
    return TRUE;
}

U16 HAL_ChipTop_Read2Byte(U32 u32RegAddr)
{
    return (*(volatile U32*)(BASE_REG_CHIPTOP_PA+((u32RegAddr & 0xFFFFFF00ul) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2)));
}

BOOL HAL_PMSLEEP_Write2Byte(U32 u32RegAddr, U16 u16Val)
{
    (*(volatile U32*)(BASE_REG_PMSLEEP_PA+((u32RegAddr & 0xFFFFFF00ul) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2))) = u16Val;
    return TRUE;
}

U16 HAL_PMSLEEP_Read2Byte(U32 u32RegAddr)
{
    return (*(volatile U32*)(BASE_REG_PMSLEEP_PA+((u32RegAddr & 0xFFFFFF00ul) << 1) + (((u32RegAddr & 0xFF)/ 2) << 2)));
}


//------------------------------------------------------------------------------
//  Global Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//  Function:   DrvPWMSetDuty
//
//  Description
//      Set Duty value
//
//  Parameters
//      u8Id:    [in] PWM ID
//      u16Val:  [in] Duty value
//
//  Return Value
//      None
//
void DrvPWMSetDuty(U8 u8Id, U32 u32Val)
{
    U32 u32Period;
    U32 u32Duty;
    U16 u16Temp;

    if (PWM_NUM <= u8Id)
        return;

    if (_pwmEnSatus[u8Id])
    {
        if (0 == u32Val)
        {
            u16Temp = HAL_PWM_Read2Byte(u16REG_SW_RESET);
            u16Temp = u16Temp &~(BIT0<<u8Id);
            u16Temp = u16Temp | ((BIT0<<u8Id)&(BIT0<<u8Id));
            HAL_PWM_Write2Byte(u16REG_SW_RESET, u16Temp);
        }
    }

    u32Period = _pwmPeriod[u8Id];
    u32Duty = ((u32Period * u32Val) / 100);

    printf("[PWM] set duty=0x%x\n", u32Duty);
    HAL_PWM_Write2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_DUTY_L, (u32Duty&0xFFFF));
    HAL_PWM_Write2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_DUTY_H, ((u32Duty>>16)&0x3));

    if (_pwmEnSatus[u8Id])
    {
        U32 reset = HAL_PWM_Read2Byte(u16REG_SW_RESET) & (BIT0<<u8Id);
        if (u32Val && reset)
            HAL_PWM_Write2Byte(u16REG_SW_RESET, HAL_PWM_Read2Byte(u16REG_SW_RESET) & ~(1<<u8Id));
    }
}

void DrvPWMGetDuty(U8 u8Id, U32* pu32Val)
{
    U32 u32Duty;

    *pu32Val = 0;
    if (PWM_NUM <= u8Id)
        return;

    u32Duty = HAL_PWM_Read2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_DUTY_L) | ((HAL_PWM_Read2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_DUTY_H) & 0x3) << 16);
    if (u32Duty)
    {
        U32 u32Period = _pwmPeriod[u8Id];
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
void DrvPWMSetPeriod(U8 u8Id, U32 u32Val)
{
    U32 u32Period;

    u32Period=(U32)PWM_CLK/u32Val;

    //[APN] range 2<=Period<=262144
    if(u32Period < 2)
        u32Period = 2;
    if(u32Period > 262144)
        u32Period = 262144;
    //[APN] PWM _PERIOD= (REG_PERIOD+1)
    u32Period--;
    
    printf("[PWM] set period==0x%x\n",u32Period);
    HAL_PWM_Write2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_PERIOD_L, (u32Period&0xFFFF));
    HAL_PWM_Write2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_PERIOD_H,  ((u32Period>>16)&0x3));
    _pwmPeriod[u8Id] = u32Period;
}

void DrvPWMGetPeriod(U8 u8Id, U32* pu32Val)
{
    U32 u32Period;

    u32Period = HAL_PWM_Read2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_PERIOD_L) | ((HAL_PWM_Read2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_PERIOD_H) & 0x3) << 16);
    if ((0 == _pwmPeriod[u8Id]) && (u32Period))
    {
        _pwmPeriod[u8Id] = u32Period;
    }
    *pu32Val = 0;
    if (u32Period)
    {
        *pu32Val = (U32)PWM_CLK/(u32Period+1);
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
void DrvPWMSetPolarity(U8 u8Id, U8 u8Val)
{
    U16 u16Temp;

    u16Temp = HAL_PWM_Read2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_CTRL);
    u16Temp = u16Temp &~(0x1<<POLARITY_BIT);
    u16Temp = u16Temp | ((u8Val<<POLARITY_BIT)&(0x1<<POLARITY_BIT));
    HAL_PWM_Write2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_CTRL, u16Temp);
}

void DrvPWMGetPolarity(U8 u8Id, U8* pu8Val)
{
    *pu8Val = (HAL_PWM_Read2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_CTRL) & (0x1<<POLARITY_BIT)) ? 1 : 0;
}

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
void DrvPWMSetDben(U8 u8Id, U8 u8Val)
{
    U16 u16Temp;

    u16Temp = HAL_PWM_Read2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_CTRL);
    u16Temp = u16Temp &~(0x1<<DBEN_BIT);
    u16Temp = u16Temp | ((u8Val<<DBEN_BIT)&(0x1<<DBEN_BIT));
    HAL_PWM_Write2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_CTRL, u16Temp);

    u16Temp = HAL_PWM_Read2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_CTRL);
    u16Temp = u16Temp &~(0x1<<VDBEN_SW_BIT);
    u16Temp = u16Temp | ((u8Val<<VDBEN_SW_BIT)&(0x1<<VDBEN_SW_BIT));
    HAL_PWM_Write2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_CTRL, u16Temp);
}

void DrvPWMEnable(U8 u8Id, U8 u8Val)
{
    if (PWM_NUM <= u8Id)
        return;
    DrvPWMSetDben(u8Id, 1);

    if(u8Val)
    {
        U32 u32DutyL = HAL_PWM_Read2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_DUTY_L);
        U32 u32DutyH = HAL_PWM_Read2Byte((u8Id*REG_CH_OFFSET) + u16REG_PWM_DUTY_H);
        if (u32DutyL || u32DutyH)
        {
            HAL_PWM_Write2Byte(u16REG_SW_RESET, HAL_PWM_Read2Byte(u16REG_SW_RESET) & ~(1<<u8Id));
        }
        else
        {
            HAL_PWM_Write2Byte(u16REG_SW_RESET, HAL_PWM_Read2Byte(u16REG_SW_RESET) | (1<<u8Id));
        }
    }
    else
    {
        HAL_PWM_Write2Byte(u16REG_SW_RESET, HAL_PWM_Read2Byte(u16REG_SW_RESET) | (1<<u8Id));
    }
    _pwmEnSatus[u8Id] = u8Val;
}

void DrvPWMEnableGet(U8 u8Id, U8* pu8Val)
{
    *pu8Val = 0;
    if (PWM_NUM <= u8Id)
        return;
    *pu8Val = _pwmEnSatus[u8Id];
}

void DrvPWMPadSet(U8 u8Id, U8 u8Val)
{
    pwmPadTbl_t* pTbl = NULL;
    U16 u16Temp;
    
    if (PWM_NUM <= u8Id)
    {
        return;
    }
    
    printf("[%s][%d] (pwmId, padId) = (%d, %d)\n", __FUNCTION__, __LINE__, u8Id, u8Val);
    pTbl = padTbl[u8Id];
    while (1)
    {
        if (u8Val == pTbl->u32PadId)
        {
            regSet_t* pRegSet = pTbl->regSet;
            if (BASE_REG_NULL != pRegSet[0].u32Adr)
            {
                u16Temp = HAL_ChipTop_Read2Byte(pRegSet[0].u32Adr);
                u16Temp = u16Temp &~(pRegSet[0].u32Msk);
                u16Temp = u16Temp | ((pRegSet[0].u32Val)&(pRegSet[0].u32Msk));
                HAL_ChipTop_Write2Byte(pRegSet[0].u32Adr, u16Temp);
            }
            if (BASE_REG_NULL != pRegSet[1].u32Adr)
            {
                u16Temp = HAL_PMSLEEP_Read2Byte(pRegSet[1].u32Adr);
                u16Temp = u16Temp &~(pRegSet[1].u32Msk);
                u16Temp = u16Temp | ((pRegSet[1].u32Val)&(pRegSet[1].u32Msk));
                HAL_PMSLEEP_Write2Byte(pRegSet[1].u32Adr, u16Temp);
            }
            break;
        }
        if (PAD_UNKNOWN == pTbl->u32PadId)
        {
            printf("[%s][%d] void DrvPWMEnable error!!!! (%x, %x)\r\n", __FUNCTION__, __LINE__, u8Id, u8Val);
            break;
        }
        pTbl++;
    }
}

void DrvPWMInit(U8 u8Id)
{
    U32 reset, u32Period;

    if (PWM_NUM <= u8Id)
        return;

    reset = HAL_PWM_Read2Byte(u16REG_SW_RESET) & (BIT0<<u8Id);
    DrvPWMGetPeriod(u8Id, &u32Period);
    if ((0 == reset) && (u32Period))
    {
        _pwmEnSatus[u8Id] = 1;
    }
    else
    {
        DrvPWMEnable(u8Id, 0);
    }
}
