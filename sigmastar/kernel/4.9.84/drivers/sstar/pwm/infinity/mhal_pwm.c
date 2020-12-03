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
#include <linux/kernel.h>
#include "ms_platform.h"
#include "ms_types.h"
#include "registers.h"
#include "mhal_pwm.h"
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

void DrvPWMSetDuty( U8 u8Id, U16 u16Val )
{
    U32 u32Period = 0x00000000;
    U16 u16Duty = 0x0000;

    u16Val = u16Val & PWM_DUTY_MASK;

    if( PWM0 == u8Id )
    {
        u32Period = INREG16(BASE_REG_PMSLEEP_PA + u16REG_PWM0_PERIOD);
        u16Duty=u32Period*u16Val/100;
        OUTREG16(BASE_REG_PMSLEEP_PA + u16REG_PWM0_DUTY, u16Duty);
    }
    else if( PWM1 == u8Id )
    {
        u32Period = INREG16(BASE_REG_PMSLEEP_PA + u16REG_PWM1_PERIOD);
        u16Duty=u32Period*u16Val/100;
        OUTREG16(BASE_REG_PMSLEEP_PA + u16REG_PWM1_DUTY, u16Duty);
    }
    else if( PWM2 == u8Id )
    {
        u32Period = INREG16(BASE_REG_PMSLEEP_PA + u16REG_PWM2_PERIOD);
        u16Duty=u32Period*u16Val/100;
        OUTREG16(BASE_REG_PMSLEEP_PA + u16REG_PWM2_DUTY, u16Duty);
    }
    else if( PWM3 == u8Id )
    {
        u32Period = INREG16(BASE_REG_PMSLEEP_PA + u16REG_PWM3_PERIOD);
        u16Duty=u32Period*u16Val/100;
        OUTREG16(BASE_REG_PMSLEEP_PA + u16REG_PWM3_DUTY, u16Duty);
    }
    else
    {
        printk(KERN_INFO "DrvPWMSetDuty error!!!! (%x, %x)\r\n", u8Id, u16Val);
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

void DrvPWMSetPeriod( U8 u8Id, U16 u16Val )
{
    U32 u32TempValue;

    u32TempValue=DEFAULT_PWM_CLK/u16Val;
    u16Val = u32TempValue & PWM_PERIOD_MASK;

    printk(KERN_INFO "DrvPWMSetPeriod !!!! (%x, %x)\r\n", u32TempValue, u16Val);

    if( PWM0 == u8Id )
    {
        OUTREG16(BASE_REG_PMSLEEP_PA + u16REG_PWM0_PERIOD, u16Val);
    }
    else if( PWM1 == u8Id )
    {
        OUTREG16(BASE_REG_PMSLEEP_PA + u16REG_PWM1_PERIOD, u16Val);
    }
    else if( PWM2 == u8Id )
    {
        OUTREG16(BASE_REG_PMSLEEP_PA + u16REG_PWM2_PERIOD, u16Val);
    }
    else if( PWM3 == u8Id )
    {
        OUTREG16(BASE_REG_PMSLEEP_PA + u16REG_PWM3_PERIOD, u16Val);
    }
    else
    {
        printk(KERN_ERR "void DrvPWMSetPeriod error!!!! (%x, %x)\r\n", u8Id, u16Val);
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

void DrvPWMSetPolarity( U8 u8Id, U8 u8Val )
{
    if( PWM0 == u8Id )
    {
        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM0_DIV, (u8Val<<8), PWM_CTRL_POLARITY );
    }
    else if( PWM1 == u8Id )
    {
        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM1_DIV, (u8Val<<8), PWM_CTRL_POLARITY );
    }
    else if( PWM2 == u8Id )
    {
        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM2_DIV, (u8Val<<8), PWM_CTRL_POLARITY );
    }
    else if( PWM3 == u8Id )
    {
        OUTREGMSK16( BASE_REG_PMSLEEP_PA + u16REG_PWM3_DIV, (u8Val<<8), PWM_CTRL_POLARITY );
    }
    else
    {
        printk(KERN_ERR "void DrvPWMSetPolarity error!!!! (%x, %x)\r\n", u8Id, u8Val);
    }

}

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

void DrvPWMSetDben( U8 u8Id, U8 u8Val )
{
    if( PWM0 == u8Id )
    {
        OUTREGMSK16(  BASE_REG_PMSLEEP_PA + u16REG_PWM0_DIV, (u8Val<<12), PWM_CTRL_DBEN );
    }
    else if( PWM1 == u8Id )
    {
        OUTREGMSK16(  BASE_REG_PMSLEEP_PA + u16REG_PWM1_DIV, (u8Val<<12), PWM_CTRL_DBEN );
    }
    else if( PWM2 == u8Id )
    {
        OUTREGMSK16(  BASE_REG_PMSLEEP_PA + u16REG_PWM2_DIV, (u8Val<<12), PWM_CTRL_DBEN );
    }
    else if( PWM3 == u8Id )
    {
        OUTREGMSK16(  BASE_REG_PMSLEEP_PA + u16REG_PWM3_DIV, (u8Val<<12), PWM_CTRL_DBEN );
    }
    else
    {
        printk(KERN_ERR "void DrvPWMSetDben error!!!! (%x, %x)\r\n", u8Id, u8Val);
    }

}

void DrvPWMEnable( U8 u8Id, U8 u8Val )
{
    if( PWM0 == u8Id )
    {
        //reg_pwm0_mode = BIT[1:0]
        OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, u8Val, 0x3);
    }
    else if( PWM1 == u8Id )
    {
        //reg_pwm1_mode=BIT[3:2]
        OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, (u8Val<<2), 0xC);
    }
    else if( PWM2 == u8Id )
    {
        //reg_pwm2_mode=BIT[4]
        OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, (u8Val<<4), 0x10);
    }
    else if( PWM3 == u8Id )
    {
        //reg_pwm3_mode=BIT[6]
        OUTREGMSK16(BASE_REG_CHIPTOP_PA + REG_ID_07, (u8Val<<6), 0x40);
    }
    else
    {
        printk(KERN_ERR "void DrvPWMEnable error!!!! (%x, %x)\r\n", u8Id, u8Val);
    }
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

//------------------------------------------------------------------------------
//
//  Function:   DrvPWMInit
//
//  Description
//      PWM init function
//
//  Parameters
//      u8CustLevel:   [in] Cust Level
//
//  Return Value
//      None
//------------------------------------------------------------------------------
void DrvPWMInit( U8 u8Id )
{
    //printk(KERN_INFO "+DrvPWMInit\r\n");

    DrvPWMSetFreqDiv( u8Id, DEFAULT_DIV_CNT );
    //DrvPWMSetPeriod( u8Id, DEFAULT_PERIOD );

    DrvPWMSetPolarity( u8Id, DEFAULT_POLARITY );

    DrvPWMSetDben( u8Id, DEFAULT_DBEN );

    //printk(KERN_INFO "-DrvPWMInit\r\n");
}

irqreturn_t PWM_IRQ(int irq, void *data)
{
    //Only4i6e
    return IRQ_NONE;
}