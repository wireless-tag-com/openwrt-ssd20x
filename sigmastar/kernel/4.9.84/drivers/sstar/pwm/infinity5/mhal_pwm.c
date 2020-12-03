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
    U8  u8ClK86MHz = 0x00;

    if (PWM_NUM <= u8Id)
        return;

    if (_pwmEnSatus[u8Id])
    {
        if (0 == u32Val)
            OUTREGMSK16(ms_chip->base + u16REG_SW_RESET, BIT0<<u8Id, BIT0<<u8Id);
    }
    u8ClK86MHz = INREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_1C, (PMSLEEP_86MHz_VAL << PMSLEEP_86MHz_POS)) >> PMSLEEP_86MHz_POS;

    u32Period = _pwmPeriod[u8Id];
    u32Duty = ((u32Period * u32Val) / 100);

    pr_err("reg=0x%08X clk=%d, u32Duty=0x%x\n", (U32)(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L), u8ClK86MHz == PMSLEEP_86MHz_VAL ? CLOCK_SRC_86MHZ : (U32)(clk_get_rate(ms_chip->clk)), u32Duty);
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
    // Original clock source(12MHz) is unable to generate the frequency higher than 6MHz(12MHz/2). So use 86MHz as source.
    if(u32Val > (U32)(clk_get_rate(ms_chip->clk)) / 2) {
        u32Period = CLOCK_SRC_86MHZ / u32Val;
        OUTREGMSK16( BASE_REG_PMSLEEP_PA + REG_ID_1C, (PMSLEEP_86MHz_VAL << PMSLEEP_86MHz_POS), (PMSLEEP_86MHz_VAL << PMSLEEP_86MHz_POS));
        CLRREG16( BASE_REG_CLKGEN_PA + REG_ID_6D, 0x1 << DIGPM_86MHz_POS);
    }
    else {
        u32Period=(U32)(clk_get_rate(ms_chip->clk))/u32Val;
        CLRREG16( BASE_REG_PMSLEEP_PA + REG_ID_1C, (PMSLEEP_86MHz_VAL << PMSLEEP_86MHz_POS));
        OUTREGMSK16( BASE_REG_CLKGEN_PA + REG_ID_6D, 0x1 << DIGPM_86MHz_POS, 0x1 << DIGPM_86MHz_POS);
    }
    //[APN] range 2<=Period<=262144
    if(u32Period < 2) {
        u32Period = 2;
    }
    else if(u32Period > 262144) {
        u32Period = 262144;
    }
    //[APN] PWM _PERIOD= (REG_PERIOD+1)
    u32Period--;

    pr_err("reg=0x%08X clk=%d, period=0x%x\n", (U32)(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L), u32Val > (U32)(clk_get_rate(ms_chip->clk)) / 2 ? CLOCK_SRC_86MHZ :(U32)(clk_get_rate(ms_chip->clk)), u32Period);

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

void DrvPWMPadSet(U8 u8Id, U8 u8Val)
{
    u16 u16EjMode = 0;
    u8  u8PadConflict = 0;

    //reg_pwm0_mode [2:0]
    //reg_pwm1_mode [5:3]
    //reg_pwm2_mode [10:8]
    //reg_pwm3_mode [14:12]
    //reg_pwm4_mode [1:0]
    //reg_pwm5_mode [4:3]
    //reg_pwm6_mode [9:8]
    //reg_pwm7_mode [13:12]
    if( 0 == u8Id )
    {
        if(u8Val==PAD_PWM0){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT0, BIT2|BIT1|BIT0);
        }else if(u8Val==PAD_GPIO12){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT1, BIT2|BIT1|BIT0);
        }else if(u8Val==PAD_FUART_RX){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT1|BIT0, BIT2|BIT1|BIT0);
        }else if(u8Val==PAD_LCD_VSYNC){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT2, BIT2|BIT1|BIT0);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, 0x0, BIT2|BIT1|BIT0);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }
    }
    else if( 1 == u8Id )
    {
        if(u8Val==PAD_PWM1){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT3, BIT5|BIT4|BIT3);
        }else if(u8Val==PAD_GPIO13){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT4, BIT5|BIT4|BIT3);
        }else if(u8Val==PAD_FUART_TX){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT4|BIT3, BIT5|BIT4|BIT3);
        }else if(u8Val==PAD_LCD_HSYNC){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT5, BIT5|BIT4|BIT3);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, 0x0, BIT5|BIT4|BIT3);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }
    }
    else if( 2 == u8Id )
    {
        if(u8Val==PAD_GPIO14){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT8, BIT10|BIT9|BIT8);
        }else if(u8Val==PAD_FUART_CTS){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT9, BIT10|BIT9|BIT8);
        }else if(u8Val==PAD_NAND_DA2){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT9|BIT8, BIT10|BIT9|BIT8);
        }else if(u8Val==PAD_SNR0_GPIO6){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT10, BIT10|BIT9|BIT8);
        }else if(u8Val==PAD_SNR0_GPIO4){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT10|BIT8, BIT10|BIT9|BIT8);
        }else if(u8Val==PAD_LCD_D0){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT10|BIT9, BIT10|BIT9|BIT8);
        }else if(u8Val==PAD_PWM0){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT10|BIT9|BIT8, BIT10|BIT9|BIT8);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, 0x0, BIT10|BIT9|BIT8);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }
    }
    else if( 3 == u8Id )
    {
        if(u8Val==PAD_GPIO15){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT12, BIT14|BIT13|BIT12);
        }else if(u8Val==PAD_FUART_RTS){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT13, BIT14|BIT13|BIT12);
        }else if(u8Val==PAD_NAND_DA3){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT13|BIT12, BIT14|BIT13|BIT12);
        }else if(u8Val==PAD_SNR1_GPIO6){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT14, BIT14|BIT13|BIT12);
        }else if(u8Val==PAD_LCD_D1){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT14|BIT12, BIT14|BIT13|BIT12);
        }else if(u8Val==PAD_PWM1){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, BIT14|BIT13, BIT14|BIT13|BIT12);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_04, 0x0, BIT14|BIT13|BIT12);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }
    }
    else if( 4 == u8Id )
    {
        if(u8Val==PAD_NAND_DA4){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_05, BIT0, BIT1|BIT0);
        }else if(u8Val==PAD_SPI0_CZ){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_05, BIT1, BIT1|BIT0);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_05, 0x0, BIT1|BIT0);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }
    }
    else if( 5 == u8Id )
    {
        if(u8Val==PAD_NAND_DA5){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_05, BIT3, BIT4|BIT3);
        }else if(u8Val==PAD_SPI0_CK){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_05, BIT4, BIT4|BIT3);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_05, 0x0, BIT4|BIT3);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }
    }
    else if( 6 == u8Id )
    {
        if(u8Val==PAD_NAND_DA6){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_05, BIT8, BIT9|BIT8);
        }else if(u8Val==PAD_SPI0_DI){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_05, BIT9, BIT9|BIT8);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_05, 0x0, BIT9|BIT8);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }
    }
    else if( 7 == u8Id )
    {
        if(u8Val==PAD_NAND_DA7){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_05, BIT12, BIT13|BIT12);
        }else if(u8Val==PAD_SPI0_DO){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_05, BIT13, BIT13|BIT12);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_05, 0x0, BIT13|BIT12);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }
    }
    else
    {
        printk(KERN_ERR "void DrvPWMEnable error!!!! (%x, %x)\r\n", u8Id, u8Val);
    }

    // check if IO PAD conflict with EJtag
    u16EjMode = INREG16(BASE_REG_CHIPTOP_PA + REG_ID_0F);
    u16EjMode &= (BIT1|BIT0);
    if((u16EjMode == BIT0) && (PAD_FUART_RX == u8Val || PAD_FUART_TX == u8Val || PAD_FUART_CTS == u8Val || PAD_FUART_RTS == u8Val))
    {
        u8PadConflict = 1;
	}
	else if((u16EjMode == BIT1) && (PAD_SPI0_CZ == u8Val || PAD_SPI0_CK == u8Val || PAD_SPI0_DI == u8Val || PAD_SPI0_DO == u8Val))
    {
        u8PadConflict = 1;
    }
    if(u8PadConflict)
    {
        printk("IO PAD conflict turn off jtag. Disable EJTAG!\n");
        OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_0F, 0x0, BIT1|BIT0);//Disable EJtag
    }
}

int DrvPWMGroupCap(void)
{
    return 0;
}

int DrvPWMGroupShift(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Val)
{
    return 0;
}

int DrvPWMGroupJoin(struct mstar_pwm_chip* ms_chip, U8 u8PWMId, U8 u8Val)
{
    return 0;
}

int DrvPWMGroupEnable(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U8 u8Val)
{
    return 0;
}

int DrvPWMGroupIsEnable(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U8* pu8Val)
{
    return 0;
}

int DrvPWMGroupSetRound(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U16 u16Val)
{
    return 0;
}

int DrvPWMGroupStop(struct mstar_pwm_chip *ms_chip, U8 u8GroupId, U8 u8Val)
{
    return 0;
}

int DrvPWMGroupHold(struct mstar_pwm_chip *ms_chip, U8 u8GroupId, U8 u8Val)
{
    return 0;
}

//+++[Only4I6e]
int DrvPWMGroupGetRoundNum(struct mstar_pwm_chip* ms_chip, U8 u8GroupId, U16* u16Val)
{
    return 0;
}

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
    return 0;
}

int DrvPWMSetBegin(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8ShftId, U32 u32Val)
{
    return 0;
}

int DrvPWMSetPolarityEx(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    return 0;
}

void DrvPWMSetPeriodEx(struct mstar_pwm_chip *ms_chip, U8 u8Id, U32 u32Val)
{
    return;
}

#if 0
int DrvPWMSetMPluse(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    return 0;
}
#endif

int DrvPWMDiv(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    return 0;
}

int DrvPWMGroupInfo(struct mstar_pwm_chip *ms_chip, char* buf_start, char* buf_end)
{
    return 0;
}

irqreturn_t PWM_IRQ(int irq, void *data)
{
    //Only4i6e
    return IRQ_NONE;
}