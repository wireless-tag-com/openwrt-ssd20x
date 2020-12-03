/*
* hal_card_intr_v5.c- Sigmastar
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

/***************************************************************************************************************
 *
 * FileName hal_card_intr_v5.c
 *     @author jeremy.wang (2015/10/29)
 * Desc:
 *     The Interrupt behavior of all cards will run here.
 *     The goal is that we don't need to change HAL Level code (But its h file code)
 *
 *     The limitations were listed as below:
 *     (1) This c file belongs to HAL level.
 *     (2) Its h file is included by driver API or HAL level, not driver flow process.
 *     (3) MIE Event Int and Card Change Event Int function belong to here.
 *     (4) FCIE/SDIO IP interrupt register function belong to here.
 *     (5) Because ISR belongs to OS design, so we must use OS define option to separate them.
 *     (6) This c file could not use project/cpu/icver/specific define option here, but its h file could.
 *
 *     P.S. EN_XX for ON/OFF Define, V_XX  for Value Define,
 *          RT_XX for Retry Times Define, WT_XX for Wait Time Define, M_XX for Mask Value Define
 *
 ***************************************************************************************************************/

#include "../inc/hal_card_intr_v5.h"

//***********************************************************************************************************
// Config Setting (Internel)
//***********************************************************************************************************

// Enable Setting
//-----------------------------------------------------------------------------------------------------------
#define EN_TRFUNC         (FALSE)

// Value Setting
//-----------------------------------------------------------------------------------------------------------
#define V_MAXSLOTS          2


//***********************************************************************************************************


// Reg Static Init Setting
//-----------------------------------------------------------------------------------------------------------
//#define V_SD_MIE_INT_INIT   (R_SD_CMD_END_EN|R_SD_DATA_END_EN|R_CARD_DMA_END_EN|R_MMA_LSDONE_END_EN)

// Mask Range
//-----------------------------------------------------------------------------------------------------------
#define M_CARD_INT_EN         (R_DATA_END_IEN|R_CMD_END_IEN|R_BUSY_END_IEN)

// Mask Reg Value
//-----------------------------------------------------------------------------------------------------------
#define M_REG_DCRCERR(IP)              (CARD_REG(A_SD_STS_REG(IP)) & M_SD_DCRCSTS)  //0x1F


//IP_FCIE or IP_SDIO Register Basic Address
//-----------------------------------------------------------------------------------------------------------
#define A_SD_REG_POS(IP)                GET_CARD_BANK(IP, 0)

#define A_MIE_EVENT_REG(IP)             GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x00)
#define A_MIE_INT_EN_REG(IP)            GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x01)

#define A_MIE_FUNC_CTL_REG(IP)          GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x07)
#define A_SD_CTL_REG(IP)                GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0C)
#define A_SD_STS_REG(IP)                GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0D)


// Reg Dynamic Variable
//-----------------------------------------------------------------------------------------------------------
static volatile U16_T   gu16_MIEINT_Mode[3] = {0};
static volatile U16_T   gu16_MIEEvent_ForInt[3] = {0};   //MIEEvent for Interrupt
static volatile U16_T   gu16_MIEIntEN_ForSDIO[3] = {0};
static volatile BOOL_T  gb_StopWaitMIE[3] = {0};

// Trace Funcion
//-----------------------------------------------------------------------------------------------------------
#if (EN_TRFUNC)
    #define TR_INT(p)           p
#else
    #define TR_INT(p)
#endif

BOOL_T _CARD_INT_SaveMIEEvent(IpOrder eIP)
{
    U16_T u16Reg = CARD_REG(A_MIE_EVENT_REG(eIP));

    if ( CARD_REG(A_MIE_FUNC_CTL_REG(eIP)) & R_EMMC_EN ) //Emmc Event
        return (FALSE);

    gu16_MIEEvent_ForInt[eIP] |= u16Reg;  //Summary All 0x00Reg Event

    u16Reg &= CARD_REG(A_MIE_INT_EN_REG(eIP));

    /****** Clean Card MIE Event *******/
    CARD_REG(A_MIE_EVENT_REG(eIP)) = 0;
    CARD_REG(A_MIE_EVENT_REG(eIP)) = (M_CARD_INT_EN & u16Reg);

    if(gu16_MIEEvent_ForInt[eIP] & R_BUSY_END_INT)
    {
        CARD_REG_CLRBIT(A_SD_CTL_REG(eIP), R_BUSY_DET_ON );
        CARD_REG_CLRBIT(A_MIE_INT_EN_REG(eIP), R_BUSY_END_INT );
    }

#if 0 // For Debug whether event clear or not
    sdmmc_print("Debug: [MIE_EVENT_REG]=       ");
    sdmmc_print("0x%04X", CARD_REG(A_MIE_EVENT_REG(eIP)));
    sdmmc_print("\r\n");

    while(1)
    {
    }
#endif

    return (TRUE);
}

static BOOL_T _CARD_INT_DetectSDIOInt(IpOrder eIP)
{
    U16_T u16Reg = CARD_REG(A_MIE_EVENT_REG(eIP));

    if( (u16Reg & R_SDIO_INT) && (gu16_MIEIntEN_ForSDIO[eIP] == R_SDIO_INT_IEN) )
    {
        //printk("_M[%04X]_", CARD_REG(A_MIE_EVENT_REG(eIP)));
        CARD_REG(A_MIE_EVENT_REG(eIP)) = R_SDIO_INT;    //Clear SDIO_EVENT to avoid trig int again

        return (TRUE);
    }

    return (FALSE);

}

void Hal_CARD_INT_MIEIntCtrl(IpOrder eIP, BOOL_T bEnable)
{
    gu16_MIEINT_Mode[eIP] = bEnable;
}

BOOL_T Hal_CARD_INT_MIEIntRunning(IpOrder eIP)
{
    return gu16_MIEINT_Mode[eIP];
}

void Hal_CARD_INT_SetMIEIntEn(IpOrder eIP, U16_T u16RegMIEIntEN)
{
    if (gu16_MIEINT_Mode[eIP])
    {
        //Enable MIE_INT_EN
        CARD_REG(A_MIE_INT_EN_REG(eIP)) = (u16RegMIEIntEN | gu16_MIEIntEN_ForSDIO[eIP]);
    }
    else
    {
        //Clear MIE_INT_EN to Avoid Interrupt
        CARD_REG(A_MIE_INT_EN_REG(eIP)) = 0;
    }
}

void Hal_CARD_INT_SetMIEIntEn_ForSDIO(IpOrder eIP, BOOL_T bEnable)
{
    if (bEnable)
    {
        //Clear SDIO Int to avoid trig Int after SDIO Int Enable
        CARD_REG(A_MIE_EVENT_REG(eIP)) = R_SDIO_INT;

        gu16_MIEIntEN_ForSDIO[eIP] = R_SDIO_INT_IEN;
        CARD_REG_SETBIT(A_MIE_INT_EN_REG(eIP), R_SDIO_INT_IEN);
    }
    else
    {
        gu16_MIEIntEN_ForSDIO[eIP] = 0;
        CARD_REG_CLRBIT(A_MIE_INT_EN_REG(eIP), R_SDIO_INT_IEN);
    }
}

void Hal_CARD_INT_ClearMIEEvent(IpOrder eIP)
{
    gu16_MIEEvent_ForInt[eIP] = 0;
}

U16_T Hal_CARD_INT_GetMIEEvent(IpOrder eIP)
{
    return gu16_MIEEvent_ForInt[eIP];
}


//***********************************************************************************************************
// Interrupt Handler Function
//***********************************************************************************************************

//###########################################################################################################
#if(D_OS == D_OS__LINUX)
//###########################################################################################################
static DECLARE_WAIT_QUEUE_HEAD(fcie1_mieint_wait);
static DECLARE_WAIT_QUEUE_HEAD(fcie2_mieint_wait);
static DECLARE_WAIT_QUEUE_HEAD(fcie3_mieint_wait);

#endif
//###########################################################################################################


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_INT_WaitMIEEvent
 *     @author jeremy.wang (2011/12/20)
 * Desc: Set WaitQueue Handler to Wait MIE Event
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param u16ReqEvent : MIE Event for waiting
 * @param u32WaitMs : Waiting Time (ms)
 *
 * @return BOOL_T  : TRUE (Success), FALSE (Timeout)
 ----------------------------------------------------------------------------------------------------------*/
BOOL_T Hal_CARD_INT_WaitMIEEvent(IpOrder eIP, U16_T u16ReqEvent, U32_T u32WaitMs)
{
//###########################################################################################################
#if (D_OS == D_OS__LINUX)
//###########################################################################################################
    long ret;
    if (eIP == IP_ORDER_0)
        ret = wait_event_timeout(fcie1_mieint_wait, ((gu16_MIEEvent_ForInt[eIP]& u16ReqEvent) == u16ReqEvent) || gb_StopWaitMIE[eIP], msecs_to_jiffies(u32WaitMs+WT_INT_RISKTIME));
    else if (eIP == IP_ORDER_1)
        ret = wait_event_timeout(fcie2_mieint_wait, ((gu16_MIEEvent_ForInt[eIP]& u16ReqEvent) == u16ReqEvent) || gb_StopWaitMIE[eIP], msecs_to_jiffies(u32WaitMs+WT_INT_RISKTIME));
    else if (eIP == IP_ORDER_2)
        ret = wait_event_timeout(fcie3_mieint_wait, ((gu16_MIEEvent_ForInt[eIP]& u16ReqEvent) == u16ReqEvent) || gb_StopWaitMIE[eIP], msecs_to_jiffies(u32WaitMs+WT_INT_RISKTIME));

    if (ret == 0)
    {
        // It "may" be time out.
    }

    if ((gu16_MIEEvent_ForInt[eIP] & u16ReqEvent) != u16ReqEvent)
        return FALSE;

    return TRUE;
//###########################################################################################################
#endif

    /****** Not Support (Ex: D_OS_ARCH = D_OS_UBOOT) ******/
    return FALSE;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_INT_StopWaitMIEEventCtrl
 *     @author jeremy.wang (2011/12/20)
 * Desc: Stop Wait MIE Event to Avoid Long Time Waiting
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param bEnable : Enable (TRUE) or Disable (FALSE)
 ----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_INT_StopWaitMIEEventCtrl(IpOrder eIP, BOOL_T bEnable)
{
    gb_StopWaitMIE[eIP] = bEnable;

//###########################################################################################################
#if (D_OS == D_OS__LINUX)
//###########################################################################################################
    if(gb_StopWaitMIE[eIP])
    {
         /****** Break Current Wait Event *******/
        if (eIP == IP_ORDER_0)
            wake_up(&fcie1_mieint_wait);
        else if (eIP == IP_ORDER_1)
            wake_up(&fcie2_mieint_wait);
        else if (eIP == IP_ORDER_2)
            wake_up(&fcie3_mieint_wait);
    }
#endif
//###########################################################################################################

}


//***********************************************************************************************************
// Interrupt Function
//***********************************************************************************************************

//###########################################################################################################
#if(D_OS == D_OS__LINUX)
//###########################################################################################################
#include <linux/mmc/host.h>
#include "../inc/ms_sdmmc_lnx.h"
#include "linux/moduleparam.h"
/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_INT_MIE
 *     @author jeremy.wang (2012/2/13)
 * Desc:
 *
 * @param irq :
 * @param p_dev_id :
 *
 * @return irqreturn_t  :
 ----------------------------------------------------------------------------------------------------------*/
unsigned int gu32_sdio_int_count=0;
module_param(gu32_sdio_int_count, uint, S_IRUGO);
unsigned int gu32_sd_irq_count=0;
module_param(gu32_sd_irq_count, uint, S_IRUGO);

irqreturn_t Hal_CARD_INT_MIE(int irq, void *p_dev_id)
{
    irqreturn_t irq_t = IRQ_NONE;
    IntSourceStruct* pstIntSource = p_dev_id;
    IpOrder eIP  = pstIntSource->eIP;
    struct ms_sdmmc_slot  *p_sdmmc_slot = pstIntSource->p_data;

    gu32_sd_irq_count++;

    if(_CARD_INT_DetectSDIOInt(eIP))
    {
        mmc_signal_sdio_irq(p_sdmmc_slot->mmc);
        irq_t = IRQ_HANDLED;
        gu32_sdio_int_count++;
    }

    if(_CARD_INT_SaveMIEEvent(eIP))
    {
        if (eIP == IP_ORDER_0)
            wake_up(&fcie1_mieint_wait);
        else if (eIP == IP_ORDER_1)
            wake_up(&fcie2_mieint_wait);
        else if (eIP == IP_ORDER_2)
            wake_up(&fcie3_mieint_wait);

        irq_t = IRQ_HANDLED;
    }

    return irq_t;
}


//###########################################################################################################
#endif
