/*
* hal_card_platform_c4.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: truman.yang <truman.yang@sigmastar.com.tw>
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

/***************************************************************************************************************
 *
 * FileName hal_card_platform_c4.c
 *     @author jeremy.wang (2010/10/14)
 * Desc:
 *     The platform Setting of all cards will run here.
 *     Every Project will have XX project name for different hal_card_platform_XX.c files
 *     The goal is that we don't need to change "other" HAL Level code.
 *
 *     The limitations were listed as below:
 *     (1) This c file belongs to HAL level.
 *     (2) Its h file is included by driver API level, not driver flow process.
 *     (3) IP Init, PAD setting, clock setting and power setting  belong to here.
 *     (4) Timer Setting doesn't belong to here, because it will be included by other HAL level.
 *     (5) FCIE/SDIO IP Reg Setting doesn't belong to here.
 *     (6) If we could, we don't need to change any code of hal_card_platform.h
 *
 ***************************************************************************************************************/

#include "../inc/hal_card_platform.h"
#include "../inc/hal_card_timer.h"

//***********************************************************************************************************
// Config Setting (Internel)
//***********************************************************************************************************

// Platform Register Basic Address
//-----------------------------------------------------------------------------------------------------------
#define A_PAD_TOP_BANK     GET_CARD_REG_ADDR(A_RIU_BASE, 0x0580)
#define A_PMU_BANK         GET_CARD_REG_ADDR(A_RIU_BASE, 0x1F80)
#define A_CHIPTOP_BANK     GET_CARD_REG_ADDR(A_RIU_BASE, 0x0F00)

// Clock Level Setting (From High Speed to Low Speed)
//-----------------------------------------------------------------------------------------------------------

#define CLK1_F          48000000
#define CLK1_E          40000000
#define CLK1_D          36000000
#define CLK1_C          32000000
#define CLK1_B          27625000
#define CLK1_A          24000000
#define CLK1_9          18420000  //18.42MHz
#define CLK1_8          15790000  //15.79MHz
#define CLK1_7          12000000  //12MHz
#define CLK1_6          300000
#define CLK1_5          0
#define CLK1_4          0
#define CLK1_3          0
#define CLK1_2          0
#define CLK1_1          0
#define CLK1_0          0

#define CLK2_F          48000000
#define CLK2_E          43200000
#define CLK2_D          36000000
#define CLK2_C          30750000
#define CLK2_B          27000000
#define CLK2_A          21500000
#define CLK2_9          20000000
#define CLK2_8          18000000
#define CLK2_7          15500000
#define CLK2_6          12000000
#define CLK2_5          300000
#define CLK2_4          0
#define CLK2_3          0
#define CLK2_2          0
#define CLK2_1          0
#define CLK2_0          0

#define CLK3_F          48000000
#define CLK3_E          43200000
#define CLK3_D          40000000
#define CLK3_C          36000000
#define CLK3_B          32000000
#define CLK3_A          24000000
#define CLK3_9          12000000
#define CLK3_8          300000
#define CLK3_7          0
#define CLK3_6          0
#define CLK3_5          0
#define CLK3_4          0
#define CLK3_3          0
#define CLK3_2          0
#define CLK3_1          0
#define CLK3_0          0


// Reg Dynamic Variable
//-----------------------------------------------------------------------------------------------------------
static volatile BusTimingEmType ge_BusTiming[3] = {0};


//***********************************************************************************************************
// IP Setting for Card Platform
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_InitIPOnce
 *     @author jeremy.wang (2012/6/7)
 * Desc:  IP Once Setting , it's about platform setting.
 *
 * @param eIP :
 ----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_IPOnceSetting(IPEmType eIP)
{
    CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PAD_TOP_BANK, 0x00), BIT01_T);         //reg_all_pad_in => Close
    CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PMU_BANK, 0x7F), BIT03_T);             //reg_all_pad_in => Close

}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_InitSetting
 *     @author jeremy.wang (2011/12/1)
 * Desc: IP Begin Setting before every operation, it's about platform setting.
 *
 * @param eIP : FCIE1/FCIE2/...
 ----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_IPBeginSetting(IPEmType eIP)
{

    if(eIP == EV_IP_FCIE1)
    {
    }
    else if(eIP == EV_IP_FCIE2)
    {
    }
    else if(eIP == EV_IP_FCIE3)
    {
    }


}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_IPEndSetting
 *     @author jeremy.wang (2012/5/3)
 * Desc: IP End Setting after every operation, it's about platform setting.
 *
 * @param eIP :
 ----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_IPEndSetting(IPEmType eIP)
{

    if(eIP == EV_IP_FCIE1)
    {
    }
    else if(eIP == EV_IP_FCIE2)
    {
    }
    else if(eIP == EV_IP_FCIE3)
    {
    }
}

//***********************************************************************************************************
// PAD Setting for Card Platform
//***********************************************************************************************************
/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_InitPADPin
 *     @author jeremy.wang (2011/12/1)
 * Desc: Init PAD Pin Status ( pull enable, pull up/down, driving strength)
 *
 * @param ePAD : PAD
 * @param bTwoCard : two card(1 bit) or not
 ----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_InitPADPin(PADEmType ePAD, BOOL_T bTwoCard)
{

    if(ePAD == EV_PAD1) //Pad SD0
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PAD_TOP_BANK, 0x07), BIT13_T|BIT12_T|BIT11_T|BIT10_T|BIT09_T|BIT08_T);   //D3, D2, D1, D0, CMD, CLK => pull en
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PAD_TOP_BANK, 0x08), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T);   //D3, D2, D1, D0, CMD => pull up
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PAD_TOP_BANK, 0x08), BIT00_T);   //CLK => pull down
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PAD_TOP_BANK, 0x07), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T);   //D3, D2, D1, D0, CMD, CLK => drv: 0

    }
    else if(ePAD==EV_PAD2) //Pad SD2
    {

    }
    else if(ePAD==EV_PAD3)
    {

    }

}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_SetPADToPortPath
 *     @author jeremy.wang (2011/12/1)
 * Desc: Set PAD to connect IP Port
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param ePort : IP Port
 * @param ePAD : PAD (Could Set NOPAD for 1-bit two cards)
 * @param bTwoCard : 1-bit two cards or not
 ----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_SetPADToPortPath(IPEmType eIP, PortEmType ePort, PADEmType ePAD, BOOL_T bTwoCard)
{
    SET_CARD_PORT(eIP, ePort);

    if(eIP == EV_IP_FCIE1)
    {
        if(ePort == EV_PORT_SDIO1)
        {
            if(ePAD == EV_PAD1) //Pad SD0
            {
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PAD_TOP_BANK, 0x01), BIT10_T);   //reg_sdio_mode = 1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PAD_TOP_BANK, 0x01), BIT11_T);
            }
        }
    }
    else if(eIP == EV_IP_FCIE2)
    {
        if(ePort == EV_PORT_SDIO1)
        {
            if(ePAD == EV_PAD2) //Pad SD2
            {
            }
        }

    }
    else if(eIP == EV_IP_FCIE3)
    {
        if(ePort == EV_PORT_SDIO1)
        {
            if(ePAD == EV_PAD3) //Pad SD2
            {
            }
        }

    }


}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_PullPADPin
 *     @author jeremy.wang (2011/12/1)
 * Desc: Pull PAD Pin for Special Purpose (Avoid Power loss.., Save Power)
 *
 * @param ePAD :  PAD
 * @param ePinPull : Pull up/Pull down
 * @param bTwoCard :  two card(1 bit) or not
 ----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_PullPADPin(PADEmType ePAD, PinPullEmType ePinPull, BOOL_T bTwoCard)
{

    if(ePAD == EV_PAD1) //PAD_SD0
    {
        if(ePinPull ==EV_PULLDOWN)
        {
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PAD_TOP_BANK, 0x08), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T);   //D3, D2, D1, D0, CMD => pull down
        }
        else if(ePinPull == EV_PULLUP)
        {
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PAD_TOP_BANK, 0x08), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T);   //D3, D2, D1, D0, CMD => pull up

        }

    }
    else if(ePAD == EV_PAD2)
    {
        if(ePinPull ==EV_PULLDOWN)
        {

        }
        else if(ePinPull == EV_PULLUP)
        {

        }
    }
    else if(ePAD == EV_PAD3)
    {
        if(ePinPull ==EV_PULLDOWN)
        {

        }
        else if(ePinPull == EV_PULLUP)
        {

        }
    }




}


//***********************************************************************************************************
// Clock Setting for Card Platform
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_SetClock
 *     @author jeremy.wang (2011/12/14)
 * Desc: Set Clock Level by Real Clock from IP
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param u32KClkFromIP : Clock Value From IP Source Set
 ----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_SetClock(IPEmType eIP, U32_T u32ClkFromIPSet)
{

    if(eIP == EV_IP_FCIE1)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x25),BIT10_T|BIT09_T|BIT08_T|BIT07_T|BIT06_T); //[10:8]: Clk_Sel [7]: Clk_i [6]: Clk_g

        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x30), BIT10_T|BIT09_T|BIT08_T|BIT07_T|BIT06_T|BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T);

        switch(u32ClkFromIPSet)
        {
            case CLK1_F:      //48000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x25),BIT10_T|BIT09_T); //6
                break;
            case CLK1_E:      //40000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x25),BIT10_T|BIT08_T); //5
                break;
            case CLK1_D:      //36000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x25),BIT10_T); //4
                break;
            case CLK1_C:      //32000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x25),BIT09_T|BIT08_T); //3
                break;
            case CLK1_B:      //27625KHz ==> spread clock
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x25),BIT10_T|BIT09_T|BIT08_T); //7: spd clk

                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x30), BIT00_T);
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x30), BIT03_T); //4
                break;
            case CLK1_A:      //24000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x25),BIT09_T); //2
                break;
            case CLK1_9:      //18420KHz ==> spread clock
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x25),BIT10_T|BIT09_T|BIT08_T); //7: spd clk

                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x30), BIT00_T);
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x30), BIT03_T|BIT02_T); //6
                break;

            case CLK1_8:      //15790KHz ==> spread clock
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x25),BIT10_T|BIT09_T|BIT08_T); //7: spd clk

                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x30), BIT00_T);
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x30), BIT03_T|BIT02_T|BIT01_T); //7
                break;

            case CLK1_7:      //12000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK,0x25),BIT08_T); //0
                break;

            case CLK1_6:      //300KHz
                break;

            /*
            case CLK1_5:
                break;
            case CLK1_4:
                break;
            case CLK1_3:
                break;
            case CLK1_2:
                break;
            case CLK1_1:
                break;
            case CLK1_0:
                break;*/

        }

    }
    else if(eIP == EV_IP_FCIE2)
    {

        switch(u32ClkFromIPSet)
        {
            /*
            case CLK2_F:      //48000KHz
                break;
            case CLK2_E:      //43200KHz
                break;
            case CLK2_D:      //36000KHz
                break;
            case CLK2_C:      //30750KHz
                break;
            case CLK2_B:      //27000KHz
                break;
            case CLK2_A:      //21500KHz
                break;
            case CLK2_9:       //20000KHz
                break;
            case CLK2_8:      //18000KHz
                break;
            case CLK2_7:      //15500KHz
                break;
            case CLK2_6:      //12000KHz
                break;
            case CLK2_5:      //300KHz
                break;
            case CLK2_4:
                break;
            case CLK_3:
                break;
            case CLK2_2:
                break;
            case CLK2_1:
                break;
            case CLK2_0:
                break;*/

        }

    }
    else if(eIP == EV_IP_FCIE3)
    {
        switch(u32ClkFromIPSet)
        {
            /*
            case CLK3_F:      //48000KHz
                break;
            case CLK3_E:      //43200KHz
                break;
            case CLK3_D:      //40000KHz
                break;
            case CLK3_C:      //36000KHz
                break;
            case CLK3_B:      //32000KHz
                break;
            case CLK3_A:      //24000KHz
                break;
            case CLK3_9:      //12000KHz
                break;
            case CLK3_8:      //300KHz
                break;
            case CLK2_4:
                break;
            case CLK_3:
                break;
            case CLK2_2:
                break;
            case CLK2_1:
                break;
            case CLK2_0:
                break;*/

        }

    }

}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_FindClockSetting
 *     @author jeremy.wang (2012/5/22)
 * Desc: Find Real Clock Level Setting by Reference Clock
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param u32ReffClk : Reference Clock Value
 * @param u8PassLevel : Pass Level to Clock Speed
 * @param u8DownLevel : Down Level to Decrease Clock Speed
 *
 * @return U32_T  : Real Clock
 ----------------------------------------------------------------------------------------------------------*/
U32_T Hal_CARD_FindClockSetting(IPEmType eIP, U32_T u32ReffClk, U8_T u8PassLevel, U8_T u8DownLevel)
{
    U8_T  u8LV = u8PassLevel;
    U32_T u32RealClk = 0;
    U32_T u32ClkArr[3][16] = { \
        {CLK1_F, CLK1_E, CLK1_D, CLK1_C, CLK1_B, CLK1_A, CLK1_9, CLK1_8, CLK1_7, CLK1_6, CLK1_5, CLK1_4, CLK1_3, CLK1_2, CLK1_1, CLK1_0} \
       ,{CLK2_F, CLK2_E, CLK2_D, CLK2_C, CLK2_B, CLK2_A, CLK2_9, CLK2_8, CLK2_7, CLK2_6, CLK2_5, CLK2_4, CLK2_3, CLK2_2, CLK2_1, CLK2_0} \
       ,{CLK3_F, CLK3_E, CLK3_D, CLK3_C, CLK3_B, CLK3_A, CLK3_9, CLK3_8, CLK3_7, CLK3_6, CLK3_5, CLK3_4, CLK3_3, CLK3_2, CLK3_1, CLK3_0} };

    for(; u8LV<16; u8LV++)
    {
        if( (u32ReffClk >= u32ClkArr[eIP][u8LV]) || (u8LV==15) || (u32ClkArr[eIP][u8LV+1]==0) )
        {
            u32RealClk = u32ClkArr[eIP][u8LV];
            break;
        }
    }

    /****** For decrease clock speed******/
    if( (u8DownLevel) && (u32RealClk) && ((u8LV+u8DownLevel)<=15) )
    {
        if(u32ClkArr[eIP][u8LV+u8DownLevel]>0) //Have Level for setting
            u32RealClk = u32ClkArr[eIP][u8LV+u8DownLevel];
    }

    return u32RealClk;
}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_SetBusTiming
 *     @author jeremy.wang (2015/1/20)
 * Desc: Platform Setting for different Bus Timing
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param eBusTiming : LOW/DEF/HS/SDR12/DDR...
 ----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_SetBusTiming(IPEmType eIP, BusTimingEmType eBusTiming)
{
    ge_BusTiming[eIP] = eBusTiming;
}


//***********************************************************************************************************
// Power and Voltage Setting for Card Platform
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_SetPADPower
 *     @author jeremy.wang (2012/1/4)
 * Desc: Set PAD Power to Different Voltage
 *
 * @param ePAD : PAD
 * @param ePADVdd : LOW/MIDDLE/HIGH Voltage Level
 ----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_SetPADPower(PADEmType ePAD, PADVddEmType ePADVdd)
{

    if(ePAD == EV_PAD1) //Pad SD0
    {
    }
    else if(ePAD == EV_PAD2) //PAD_SD2
    {

    }
    else if(ePAD == EV_PAD3)
    {
        if(ePADVdd == EV_NORVOL)
        {
            //CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PMU_BANK,0x20), BIT00_T);                   //reg_ldovio3_a_en
        }
    }
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_PowerOn
 *     @author jeremy.wang (2011/12/13)
 * Desc: Power on Card Power
 *
 * @param ePAD : PAD
 * @param u16DelayMs : Delay ms for Stable Power
 ----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_PowerOn(PADEmType ePAD, U16_T u16DelayMs)
{
    if(ePAD==EV_PAD1)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PMU_BANK, 0x7E), BIT00_T);            //CARD_POWER_CTL1  //output mode
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PMU_BANK, 0x7D), BIT00_T);            // output:0

    }
    else if(ePAD==EV_PAD2)
    {
    }

    Hal_Timer_mDelay(u16DelayMs);

}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_PowerOff
 *     @author jeremy.wang (2011/12/13)
 * Desc: Power off Card Power
 *
 * @param ePAD : PAD
 * @param u16DelayMs :  Delay ms to Confirm No Power
 ----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_PowerOff(PADEmType ePAD, U16_T u16DelayMs)
{

    if(ePAD==EV_PAD1)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PMU_BANK, 0x7E), BIT00_T);     //CARD_POWER_CTL1  //output mode
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PMU_BANK, 0x7D), BIT00_T);      // output:1
    }
    else if(ePAD==EV_PAD2)
    {
    }

    Hal_Timer_mDelay(u16DelayMs);
}


//***********************************************************************************************************
// Card Detect and GPIO Setting for Card Platform
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_GetGPIONum
 *     @author jeremy.wang (2012/5/22)
 * Desc:
 *
 * @param eGPIO :
 *
 * @return U32_T  :
 ----------------------------------------------------------------------------------------------------------*/
U32_T Hal_CARD_GetGPIONum(GPIOEmType eGPIO)
{
    S32_T s32GPIO = -1;
    if( eGPIO==EV_GPIO1 ) //EV_GPIO1 for Slot 0
    {
        //s32GPIO = DrvPadmuxGetGpio(IO_CHIP_INDEX_SD_CDZ);
    }
    else if( eGPIO==EV_GPIO2)
    {
    }

    if(s32GPIO>0)
        return (U32_T)s32GPIO;
    else
        return 0;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_InitGPIO
 *     @author jeremy.wang (2012/5/22)
 * Desc:
 *
 * @param eGPIO :
 * @param bEnable :
 ----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_InitGPIO(GPIOEmType eGPIO, BOOL_T bEnable)
{
    if( eGPIO==EV_GPIO1 ) //EV_GPIO1 for Slot 0
    {
        if(bEnable)
        {
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PMU_BANK, 0x7C), BIT07_T);       //ip cdz mode en
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PMU_BANK, 0x7F), BIT01_T|BIT00_T);   //pull up //ps en
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PMU_BANK, 0x7F), BIT02_T);   //drv: 0
        }
        else
        {
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PMU_BANK, 0x7C), BIT07_T);       //ip cdz mode en
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PMU_BANK, 0x7F), BIT00_T);        //pe en
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PMU_BANK, 0x7F), BIT02_T|BIT01_T);   //drv: 0 //pull down
        }
    }
    else if( eGPIO==EV_GPIO2 ) //EV_GPIO2 for Slot 1
    {
        if(bEnable)
        {
        }
        else
        {
        }
    }


}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_GetGPIOState
 *     @author jeremy.wang (2012/5/22)
 * Desc: Get CDZ GPIO State
 *
 * @param eGPIO : GPIO1/GPIO2/GPIO3...
 *
 * @return BOOL_T  : Insert (TRUE) or Remove (FALSE)
 ----------------------------------------------------------------------------------------------------------*/
BOOL_T Hal_CARD_GetGPIOState(GPIOEmType eGPIO)
{
    if( eGPIO==EV_GPIO1 ) //EV_GPIO1 for Slot 0
    {
        if(CARD_REG(GET_CARD_REG_ADDR(A_PMU_BANK, 0x04)) & BIT12_T) // CDZ SD0
            return (FALSE);
        else
            return (TRUE);

    }
    if( eGPIO==EV_GPIO2 ) //EV_GPIO1 for Slot 0
    {
    }

    return (FALSE);
}

#if (D_OS == D_OS__LINUX)
#include <mach/irqs.h>
#include <linux/irq.h>
extern void ms_irq_set_polarity(struct irq_data *data,unsigned char polarity);
#endif
/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_SetGPIOIntAttr
 *     @author jeremy.wang (2012/5/22)
 * Desc:
 *
 * @param eGPIO :
 * @param eGPIOOPT :
 ----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_SetGPIOIntAttr(GPIOEmType eGPIO, GPIOOptEmType eGPIOOPT)
{

#if (D_OS == D_OS__LINUX)

    struct irq_data sd_irqdata;
    struct irq_chip *chip;

    if( eGPIO==EV_GPIO1 ) //EV_GPIO1 for Slot 0
    {
        sd_irqdata.irq = INT_PMU_SD_DETECT;
        chip = irq_get_chip(sd_irqdata.irq);

        if(eGPIOOPT==EV_GPIO_OPT1) //clear interrupt
        {
            chip->irq_ack(&sd_irqdata);
        }
        else if((eGPIOOPT==EV_GPIO_OPT2))
        {
        }
        else if((eGPIOOPT==EV_GPIO_OPT3))  //sd polarity _HI Trig for remove
        {
            //ms_irq_set_polarity(&sd_irqdata , 0); //C4 Edge Trig, Don't need to set polarity
        }
        else if((eGPIOOPT==EV_GPIO_OPT4)) //sd polarity _LO Trig for insert
        {
            //ms_irq_set_polarity(&sd_irqdata , 1); //C4 Edge Trig, Don't need to set polarity

        }

    }
    else if( eGPIO==EV_GPIO2)
    {

        if(eGPIOOPT==EV_GPIO_OPT1)
        {
        }
        else if((eGPIOOPT==EV_GPIO_OPT2))
        {
        }
        else if((eGPIOOPT==EV_GPIO_OPT3))
        {
        }
        else if((eGPIOOPT==EV_GPIO_OPT4))
        {
        }


    }

#endif

}



/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_GPIOIntFilter
 *     @author jeremy.wang (2012/5/9)
 * Desc:
 *
 * @param eGPIO :
 *
 * @return BOOL_T  :
 ----------------------------------------------------------------------------------------------------------*/
BOOL_T Hal_CARD_GPIOIntFilter(GPIOEmType eGPIO)
{

    if( eGPIO==EV_GPIO1 ) //EV_GPIO1 for Slot 0
    {
        return (TRUE);
    }
    else if( eGPIO==EV_GPIO2 )
    {
         return (TRUE);
    }

    return (FALSE);
}

//***********************************************************************************************************
// MIU Setting for Card Platform
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_CARD_TransMIUAddr
 *     @author jeremy.wang (2012/6/7)
 * Desc:
 *
 * @param u32Addr :
 *
 * @return U32_T  :
 ----------------------------------------------------------------------------------------------------------*/
U32_T Hal_CARD_TransMIUAddr(U32_T u32Addr)
{
    return u32Addr;
}




















