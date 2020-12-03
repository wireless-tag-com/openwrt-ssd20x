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

    if (0 == u32Val)
    {
        OUTREGMSK16(ms_chip->base + u16REG_SW_RESET, BIT0<<u8Id, BIT0<<u8Id);
        return;
    }

    u32Period = INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_L) + ((INREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_PERIOD_H)<<16));
    u32Duty = ((u32Period * u32Val) / 100);

    pr_err("reg=0x%08X clk=%d, u32Duty=0x%x\n", (U32)(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L), (U32)(clk_get_rate(ms_chip->clk)), u32Duty);
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_L, (u32Duty&0xFFFF));
    OUTREG16(ms_chip->base + (u8Id*0x80) + u16REG_PWM_DUTY_H, ((u32Duty>>16)&0x3));
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
    U32 u32Period;

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
}
void DrvPWMEnable(struct mstar_pwm_chip *ms_chip, U8 u8Id, U8 u8Val)
{
    if(u8Val)
        CLRREG16(ms_chip->base + u16REG_SW_RESET, 1<<u8Id);
    else
        SETREG16(ms_chip->base + u16REG_SW_RESET, 1<<u8Id);
}

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
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT0, BIT1|BIT0);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT1|BIT0);
        }else if(u8Val==PAD_GPIO12){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT1, BIT1|BIT0);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT1|BIT0);
        }else if(u8Val==PAD_FUART_RX){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT1|BIT0, BIT1|BIT0);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT1|BIT0);
        }else if(u8Val==PAD_PM_GPIO0){
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, BIT0, BIT1|BIT0);
        }else if(u8Val==PAD_PM_GPIO6){
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, BIT1, BIT1|BIT0);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, 0x0, BIT1|BIT0);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT1|BIT0);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }
    }
    else if( 1 == u8Id )
    {
        if(u8Val==PAD_PWM1){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT2, BIT3|BIT2);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT3|BIT2);
        }else if(u8Val==PAD_GPIO13){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT3, BIT3|BIT2);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT3|BIT2);
        }else if(u8Val==PAD_FUART_TX){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT3|BIT2, BIT3|BIT2);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT3|BIT2);
        }else if(u8Val==PAD_PM_GPIO1){
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, BIT2, BIT3|BIT2);
        }else if(u8Val==PAD_PM_GPIO5){
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, BIT3, BIT3|BIT2);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, 0x0, BIT3|BIT2);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT3|BIT2);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }
    }
    else if( 2 == u8Id )
    {
        if(u8Val==PAD_GPIO14){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT4, BIT5|BIT4);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT7|BIT6);
        }else if(u8Val==PAD_FUART_CTS){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT5, BIT5|BIT4);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT7|BIT6);
        }else if(u8Val==PAD_NAND_DA2){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT5|BIT4, BIT5|BIT4);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT7|BIT6);
        }else if(u8Val==PAD_PM_GPIO2){
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, BIT6, BIT7|BIT6);
        }else if(u8Val==PAD_PM_GPIO9){
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, BIT7, BIT7|BIT6);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, 0x0, BIT5|BIT4);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT7|BIT6);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");

        }
    }
    else if( 3 == u8Id )
    {
        if(u8Val==PAD_GPIO15){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT6, BIT7|BIT6);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT9|BIT8);
        }else if(u8Val==PAD_FUART_RTS){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT7, BIT7|BIT6);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT9|BIT8);
        }else if(u8Val==PAD_NAND_DA3){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT7|BIT6, BIT7|BIT6);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT9|BIT8);
        }else if(u8Val==PAD_PM_GPIO3){
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, BIT8, BIT9|BIT8);
        }else if(u8Val==PAD_PM_GPIO10){
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, BIT9, BIT9|BIT8);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, 0x0, BIT7|BIT6);
            OUTREGMSK16(BASE_REG_PMSLEEP_PA + REG_ID_28, 0x0, BIT9|BIT8);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");
        }
    }
    else if( 4 == u8Id )
    {
        if(u8Val==PAD_NAND_DA4){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT8, BIT9|BIT8);
        }else if(u8Val==PAD_SPI0_CZ){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT9, BIT9|BIT8);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, 0x0, BIT9|BIT8);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");

        }
    }
    else if( 5 == u8Id )
    {
        if(u8Val==PAD_NAND_DA5){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT10, BIT11|BIT10);
        }else if(u8Val==PAD_SPI0_CK){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT11, BIT11|BIT10);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, 0x0, BIT11|BIT10);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");

        }
    }
    else if( 6 == u8Id )
    {
        if(u8Val==PAD_NAND_DA6){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT12, BIT13|BIT12);
        }else if(u8Val==PAD_SPI0_DI){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT13, BIT13|BIT12);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, 0x0, BIT13|BIT12);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");

        }
    }
    else if( 7 == u8Id )
    {
        if(u8Val==PAD_NAND_DA7){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT14, BIT15|BIT14);
        }else if(u8Val==PAD_SPI0_DO){
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, BIT15, BIT15|BIT14);
        }else{
            OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, 0x0, BIT15|BIT14);
            if(u8Val!=PAD_UNKNOWN)
                printk("PWM pad set failed\n");

        }
    }
    else
    {
        printk(KERN_ERR "void DrvPWMEnable error!!!! (%x, %x)\r\n", u8Id, u8Val);
    }

	if(PAD_SPI0_CK == u8Val || PAD_SPI0_DI == u8Val || PAD_SPI0_DO == u8Val || PAD_SPI0_CZ == u8Val)
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