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
#include <linux/of_irq.h>

//------------------------------------------------------------------------------
//  Variables
//------------------------------------------------------------------------------
#define BASE_REG_NULL 0xFFFFFFFF

static U8 _pwmEnSatus[PWM_NUM] = { 0 };
static U32 _pwmPeriod[PWM_NUM] = { 0 };

//------------------------------------------------------------------------------
//  Local Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  External Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  Global Functions
//------------------------------------------------------------------------------
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
    if(u32Period < 2) {
        u32Period = 2;
    }
    else if(u32Period > 262144) {
        u32Period = 262144;
    }
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

//void DrvPWMSetFreqDiv( U8 u8Id, U8 u8Val )
//{
//    if( PWM0 == u8Id )
//    {
//        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM0_DIV, u8Val, PWM_CTRL_DIV_MSAK );
//    }
//    else if( PWM1 == u8Id )
//    {
//        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM1_DIV, u8Val, PWM_CTRL_DIV_MSAK );
//    }
//    else if( PWM2 == u8Id )
//    {
//        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM2_DIV, u8Val, PWM_CTRL_DIV_MSAK );
//    }
//    else if( PWM3 == u8Id )
//    {
//        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM3_DIV, u8Val, PWM_CTRL_DIV_MSAK );
//    }
//    else
//    {
//        printk(KERN_ERR "void DrvPWMSetDiv error!!!! (%x, %x)\r\n", u8Id, u8Val);
//    }
//}

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

//void DrvPWMPad_dump(void)
//{
//    int i;
//    for (i = 0; i < pwmNum; i++)
//    {
//        pwmPadTbl_t* pTbl = padTbl[i];
//        printk("[%s][%d] %d ------------------------------\n", __FUNCTION__, __LINE__, i);
//        while (1)
//        {
//            regSet_t* pRegSet = pTbl->regSet;
//            printk("[%s][%d]     ******************************\n", __FUNCTION__, __LINE__);
//            printk("[%s][%d]         pad Id = %d\n", __FUNCTION__, __LINE__, pTbl->u32PadId);
//            printk("[%s][%d]         (reg, val, msk) = (0x%08x, 0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__, pRegSet[0].u32Adr, pRegSet[0].u32Val, pRegSet[0].u32Msk);
//            printk("[%s][%d]         (reg, val, msk) = (0x%08x, 0x%08x, 0x%08x)\n", __FUNCTION__, __LINE__, pRegSet[1].u32Adr, pRegSet[1].u32Val, pRegSet[1].u32Msk);
//            if (PAD_UNKNOWN == pTbl->u32PadId)
//            {
//                break;
//            }
//            pTbl++;
//        }
//    }
//}

void DrvPWMPadSet(U8 u8Id, U8 u8Val)
{
    //reg_pwm0_mode [1:0]
    //reg_pwm1_mode [3:2]
    //reg_pwm2_mode [5:4]
    //reg_pwm3_mode [7:6]
    //reg_pwm4_mode [9:8]
    //reg_pwm5_mode [11:10]
    //reg_pwm6_mode [13:12]
    //reg_pwm7_mode [15:14]
    if( 0 == u8Id )
    {
        if(u8Val==PAD_PWM0){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT0, BIT1|BIT0);
        }else if(u8Val==PAD_MIPI_TX_IO0){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT1, BIT1|BIT0);
        }else if(u8Val==PAD_SNR3_D0){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT1|BIT0, BIT1|BIT0);
        }else{
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, 0x0, BIT1|BIT0);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }
    }
    else if( 1 == u8Id )
    {
        if(u8Val==PAD_PWM1){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT2, BIT2|BIT3);
        }else if(u8Val==PAD_MIPI_TX_IO1){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT3, BIT2|BIT3);
        }else if(u8Val==PAD_SNR3_D1){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT2|BIT3, BIT2|BIT3);
        }else{
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, 0x0, BIT2|BIT3);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }

    }
    else if( 2 == u8Id )
    {
        if(u8Val==PAD_GPIO8){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT4, BIT5|BIT4);
        }else if(u8Val==PAD_MIPI_TX_IO2){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT5, BIT5|BIT4);
        }else if(u8Val==PAD_SNR3_D2){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT5|BIT4, BIT5|BIT4);
		}else{
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, 0x0, BIT5|BIT4);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");

        }
    }
    else if( 3 == u8Id )
    {
        if(u8Val==PAD_GPIO9){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT6, BIT7|BIT6);
        }else if(u8Val==PAD_MIPI_TX_IO3){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT7, BIT7|BIT6);
        }else if(u8Val==PAD_SNR3_D3){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT7|BIT6, BIT7|BIT6);
		}else{
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, 0x0, BIT7|BIT6);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }
    }
    else if( 4 == u8Id )
    {
        if(u8Val==PAD_GPIO10){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT8, BIT9|BIT8);
        }else if(u8Val==PAD_MIPI_TX_IO4){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT9, BIT9|BIT8);
		}else if(u8Val==PAD_SNR3_D8){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT9|BIT8, BIT9|BIT8);
        }else{
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, 0x0, BIT9|BIT8);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }
    }
    else if( 5 == u8Id )
    {
        if(u8Val==PAD_GPIO11){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT10, BIT11|BIT10);
        }else if(u8Val==PAD_MIPI_TX_IO5){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT11, BIT11|BIT10);
        }else if(u8Val==PAD_SNR3_D9){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT11|BIT10, BIT11|BIT10);
		}else{
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, 0x0, BIT11|BIT10);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");

        }
    }
    else if( 6 == u8Id )
    {
        if(u8Val==PAD_GPIO12){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT12, BIT13|BIT12);
        }else if(u8Val==PAD_MIPI_TX_IO6){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT13, BIT13|BIT12);
        }else if(u8Val==PAD_PM_LED0){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT13|BIT12, BIT13|BIT12);
		}else{
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, 0x0, BIT13|BIT12);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");

        }
    }
    else if( 7 == u8Id )
    {
        if(u8Val==PAD_GPIO13){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT14, BIT15|BIT14);
        }else if(u8Val==PAD_MIPI_TX_IO7){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT15, BIT15|BIT14);
        }else if(u8Val==PAD_PM_LED1){
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, BIT15|BIT14, BIT15|BIT14);
		}else{
            OUTREGMSK16(BASE_REG_PADTOP1_PA + REG_ID_09, 0x0, BIT15|BIT14);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");

        }
    }
    else
    {
        printk(KERN_ERR "void DrvPWMEnable error!!!! (%x, %x)\r\n", u8Id, u8Val);
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
    u32JoinMask = 0xF << ((u8GroupId << 2) + REG_GROUP_JOIN_SHFT);
    u32JoinMask = INREG16(ms_chip->base + REG_GROUP_JOIN) & u32JoinMask;
    u32JoinMask |= 1 << (u8GroupId + PWM_NUM);

    if (u8Val)
    {
        SETREG16(ms_chip->base + REG_GROUP_ENABLE, (1 << (u8GroupId + REG_GROUP_ENABLE_SHFT)));
        CLRREG16(ms_chip->base + u16REG_SW_RESET, u32JoinMask);
    }
    else
    {
        CLRREG16(ms_chip->base + REG_GROUP_ENABLE, (1 << (u8GroupId + REG_GROUP_ENABLE_SHFT)));
        SETREG16(ms_chip->base + u16REG_SW_RESET, u32JoinMask);
    }
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

int DrvPWMGroupGetRoundNum(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U16* pu16Val)
{
    U32 u32Reg;

    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;
    u32Reg = (u8GroupId << 0x7) + 0x40;
    *pu16Val = INREG16(ms_chip->base + u32Reg) & 0xFFFF;
    return 1;
}

int DrvPWMGroupSetRound(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U16 u16Val)
{
    U32 u32Reg;

    if (PWM_GROUP_NUM <= u8GroupId)
        return 0;
    u32Reg = (u8GroupId << 0x7) + 0x40;
    OUTREG16(ms_chip->base + u32Reg, u16Val);
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

int DrvPWMSetEnd(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8DutyId, U32 u32Val)
{
    U32 u32Period;
    U32 u32Duty;

    if (PWM_NUM <= u8Id)
        return 0;
/*
    if (0 == u32Val)
    {
        OUTREGMSK16(ms_chip->base + u16REG_SW_RESET, BIT0<<u8Id, BIT0<<u8Id);
        return;
    }
*/

    // u32Period = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L) + ((INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_H)<<16));
    u32Period = _pwmPeriod[u8Id];
    u32Duty = ((u32Period * u32Val) / 1000);

    if (u32Duty & 0xFFFC0000)
    {
        printk("[%s][%d] too large duty 0x%08x (18 bits in max)\n", __FUNCTION__, __LINE__, u32Duty);
    }
/*
    pr_err("reg=0x%08X clk=%d, u32Duty=0x%x\n", (U32)(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L), (U32)(clk_get_rate(ms_chip->clk)), u32Duty);
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L, (u32Duty&0xFFFF));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_H, ((u32Duty>>16)&0x3));
*/
    if (0 == u8DutyId)
    {
        OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L, (u32Duty&0xFFFF));
        OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_H, ((u32Duty>> 16)&0x0003));
    }
    else
    {
        OUTREG16(ms_chip->base + (u8Id*0x80) + (u8DutyId << 3) + 28, (u32Duty&0xFFFF));
    }
    return 1;
}

int DrvPWMSetBegin(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8ShftId, U32 u32Val)
{
    U32 u32Period;
    U32 u32Shft;

    if (PWM_NUM <= u8Id)
        return 0;
/*
    if (0 == u32Val)
    {
        OUTREGMSK16(ms_chip->base + u16REG_SW_RESET, BIT0<<u8Id, BIT0<<u8Id);
        return;
    }
*/
    // u32Period = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L) + ((INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_H)<<16));
    u32Period = _pwmPeriod[u8Id];
    u32Shft = ((u32Period * u32Val) / 1000);

    if (u32Shft & 0xFFFC0000)
    {
        printk("[%s][%d] too large shift 0x%08x (18 bits in max)\n", __FUNCTION__, __LINE__, u32Shft);
    }
/*
    pr_err("reg=0x%08X clk=%d, u32Duty=0x%x\n", (U32)(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L), (U32)(clk_get_rate(ms_chip->clk)), u32Duty);
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L, (u32Duty&0xFFFF));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_H, ((u32Duty>>16)&0x3));
*/
    if (0 == u8ShftId)
    {
        OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_SHIFT_L, (u32Shft&0xFFFF));
        OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_SHIFT_H, ((u32Shft>> 16)&0x0003));
    }
    else
    {
        OUTREG16(ms_chip->base + (u8Id*0x80) + (u8ShftId << 3) + 24, (u32Shft&0xFFFF));
    }
    return 1;
}

int DrvPWMSetPolarityEx(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    if (PWM_NUM <= u8Id)
        return 0;
    if (u8Val)
        SETREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (0x1<<POLARITY_BIT));
    else
        CLRREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_CTRL, (0x1<<POLARITY_BIT));
    return 1;
}

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

    pr_err("reg=0x%08X clk=%d, period=0x%x\n", (U32)(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L), (U32)(clk_get_rate(ms_chip->clk)), u32Period);

    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L, (u32Period&0xFFFF));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_H, ((u32Period>>16)&0x3));
    _pwmPeriod[u8Id] = u32Period;
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

irqreturn_t PWM_IRQ(int irq, void *data)
{
    //Only4i6e
    return IRQ_NONE;
}
