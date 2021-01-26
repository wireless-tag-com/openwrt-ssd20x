/*
* hal_card_platform_iNF6e.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: tunglin.hsieh <tunglin.hsieh@sigmastar.com.tw>
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
 * FileName hal_card_platform_iNF3.c
 *     @author jeremy.wang (2016/11/29)
 * Desc:
 *     The platform setting of all cards will run here.
 *     Because register setting that doesn't belong to FCIE/SDIO may have different register setting at different projects.
 *     The goal is that we don't need to change "other" HAL_XX.c Level code. (Timing, FCIE/SDIO)
 *
 *     The limitations were listed as below:
 *     (1) Each Project will have XX project name for different hal_card_platform_XX.c files.
 *     (2) IP init, PAD , clock, power and miu setting belong to here.
 *     (4) Timer setting doesn't belong to here, because it will be included by other HAL level.
 *     (5) FCIE/SDIO IP Reg Setting doesn't belong to here.
 *     (6) If we could, we don't need to change any code of hal_card_platform.h
 *
 ***************************************************************************************************************/

#include "../inc/hal_card_platform.h"
#include "../inc/hal_card_timer.h"
#include "../inc/hal_sdmmc_v5.h"
#include "../inc/ms_sdmmc_drv.h"

#define PORT_FROM_KERNEL (1)

//***********************************************************************************************************
// Config Setting (Internal)
//***********************************************************************************************************

// Platform Register Basic Address
//------------------------------------------------------------------------------------
#define A_CLKGEN_BANK       GET_CARD_REG_ADDR(A_RIU_BASE, 0x81C00)//1038h
    #define REG_CLK_SD0   (0x43)
    #define REG_CLK_SDIO0 (0x45)
#define A_PADTOP_BANK       GET_CARD_REG_ADDR(A_RIU_BASE, 0x81E00)//103Ch
#define A_PM_SLEEP_BANK     GET_CARD_REG_ADDR(A_RIU_BASE, 0x00700)//0Eh
#define A_PM_GPIO_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0x00780)//0Fh
#define A_CHIPTOP_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0x80F00)//101Eh
#define A_SC_GP_CTRL_BANK   GET_CARD_REG_ADDR(A_RIU_BASE, 0x89980)//1133h


// Clock Level Setting (From High Speed to Low Speed)
//-----------------------------------------------------------------------------------------------------------
#define CLK1_F          48000000
#define CLK1_E          43200000
#define CLK1_D          40000000
#define CLK1_C          36000000
#define CLK1_B          32000000
#define CLK1_A          20000000
#define CLK1_9          12000000
#define CLK1_8          300000
#define CLK1_7          0
#define CLK1_6          0
#define CLK1_5          0
#define CLK1_4          0
#define CLK1_3          0
#define CLK1_2          0
#define CLK1_1          0
#define CLK1_0          0

#define CLK2_F          48000000
#define CLK2_E          43200000
#define CLK2_D          40000000
#define CLK2_C          36000000
#define CLK2_B          32000000
#define CLK2_A          20000000
#define CLK2_9          12000000
#define CLK2_8          300000
#define CLK2_7          0
#define CLK2_6          0
#define CLK2_5          0
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
#define CLK3_A          20000000
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
* Function: Hal_CARD_IPOnceSetting
*     @author jeremy.wang (2015/7/17)
* Desc: IP once setting , it's about platform setting.
*
* @param eIP : FCIE1/FCIE2/...
----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_IPOnceSetting(IPEmType eIP)
{
	CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x50), BIT15_T);         //reg_all_pad_in => Close
    //CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_MCM_SC_GP_BANK, 0x09), 0xF000);      //Test

    if(eIP == EV_IP_FCIE1)
    {
#if PORT_FROM_KERNEL
        //CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT13_T | BIT12_T); //[B13:B12]reg_sd1_mode
        //CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT09_T);           //[B9]reg_sd1_cdz_mode
        //CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T | BIT09_T); //[B8:B9]/[reg_sd0_mode:reg_sd0_cdz_mode]
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK,0x25), BIT07_T); //[B3/B7]:[SDIO/SD]: select BOOT clock source (glitch free) - 0: select BOOT clock 12MHz (xtali), 1: select FCIE/SPI clock source
        //Enable Clock Source to avoid reset fail
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_SD0), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T); //[5:2]: Clk_Sel [1]: Clk_i [0]: Clk_g
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_SD0), BIT05_T); //[5]boot_Sel [4:2]: Clk_Sel [1]: Clk_i [0]: Clk_g
#else
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,0x45), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T); //[5]boot_Sel [4:2]: Clk_Sel [1]: Clk_i [0]: Clk_g
#endif
        Hal_SDMMC_SetSDIODevice(eIP, false);
    }
    else if(eIP == EV_IP_FCIE2)
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK,0x25), BIT03_T); //[B3/B7]:[SDIO/SD] select BOOT clock source (glitch free) - 0: select BOOT clock 12MHz (xtali), 1: select FCIE/SPI clock source
        //Enable Clock Source to avoid reset fail
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_SDIO0), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T); //[5]boot_Sel [4:2]: Clk_Sel [1]: Clk_i [0]: Clk_g
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_SDIO0), BIT05_T);
    }
}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_CARD_IPBeginSetting
*     @author jeremy.wang (2015/7/29)
* Desc: IP begin setting before every operation, it's about platform setting.
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
*     @author jeremy.wang (2015/7/29)
* Desc: IP end setting after every operation, it's about platform setting.
*
* @param eIP : FCIE1/FCIE2/...
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
*     @author jeremy.wang (2015/7/28)
* Desc: Init PAD Pin Status ( pull enable, pull up/down, driving strength)
*
* @param ePAD : PAD
* @param bTwoCard : two card(1 bit) or not
----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_InitPADPin(PADEmType ePAD, BOOL_T bTwoCard)
{

    if(ePAD == EV_PAD1) //PAD_SD
    {
        //reg_sd0_pe:D3, D2, D1, D0, CMD=> pull en
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x12), BIT04_T); //D1
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x13), BIT04_T); //D0
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x15), BIT04_T); //CMD
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x16), BIT04_T); //D3
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x17), BIT04_T); //D2

        //reg_sd0_drv: CLK, D3, D2, D1, D0, CMD => drv: 0
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x12), BIT07_T); //D1
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x13), BIT07_T); //D0
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x14), BIT07_T); //CLK
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x15), BIT07_T); //CMD
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x16), BIT07_T); //D3
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x17), BIT07_T); //D2
    }
    else if (ePAD == EV_PAD2) //PAD_NAND
    {
        //reg_sd1_pe:D3, D2, D1, D0, CMD=> pull en
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x27), BIT04_T); //D1
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x28), BIT04_T); //D0
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2A), BIT04_T); //CMD
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2B), BIT04_T); //D3
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2C), BIT04_T); //D2

        //reg_sd1_drv: CLK, D3, D2, D1, D0, CMD => drv: 0
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x27), BIT07_T); //D1
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x28), BIT07_T); //D0
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x29), BIT07_T); //CLK
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2A), BIT07_T); //CMD
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2B), BIT07_T); //D3
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2C), BIT07_T); //D2
    }

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_CARD_SetPADToPortPath
*     @author jeremy.wang (2015/7/28)
* Desc: Set PAD to connect IP Port
*
* @param eIP : FCIE1/FCIE2/...
* @param ePort : Port (But FCIE5 use it to decide FCIE or SDIO IP)
* @param ePAD : PAD
* @param bTwoCard : 1-bit two cards or not
----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_SetPADToPortPath(IPEmType eIP, PortEmType ePort, PADEmType ePAD, BOOL_T bTwoCard)
{
    SET_CARD_PORT(eIP, ePort);

    if(eIP == EV_IP_FCIE1)
    {
        if(ePAD == EV_PAD1)  //PAD_SD
        {
#if !PORT_FROM_KERNEL //uboot
            //OFF:x67 [B8:B9]/[reg_sd0_mode:reg_sd0_cdz_mode]; [B13:B12]reg_sd1_mode
            //OFF:x68 [B8:B9]reg_sd1_cdz_mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT13_T | BIT12_T | BIT09_T | BIT08_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT09_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T | BIT09_T);
#else //kernel, new setting
            //OFF:x67 [B8:B9]/[reg_sd0_mode:reg_sd0_cdz_mode]; [B13:B12]reg_sd1_mode
            //OFF:x68 [B8:B9]reg_sd1_cdz_mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT13_T | BIT12_T | BIT09_T | BIT08_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT09_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T | BIT09_T);

            //Make sure reg_spi0_mode != 3
            if((GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68) & (BIT02_T | BIT01_T | BIT00_T)) == (BIT01_T | BIT00_T))
            {
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT02_T | BIT01_T | BIT00_T);
            }
#endif
        }
    }
    else if(eIP == EV_IP_FCIE2)
    {
        if(ePAD == EV_PAD2)  //PAD_NAND
        {
            //SD mode
            //OFF:x67 [B8:B9]/[reg_sd0_mode:reg_sd0_cdz_mode]; [B13:B12]reg_sd1_mode
            //OFF:x68 [B9]reg_sd1_cdz_mode
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT12_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT09_T); 

            // Make sure reg_uart1_mode != 4
            if((GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x6D) & (BIT06_T | BIT05_T | BIT04_T)) == BIT06_T)
            {
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x6D), BIT06_T | BIT05_T | BIT04_T);
            }

            // Make sure reg_fuart_mode != 2 && 4
            if((GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x6E) & (BIT06_T | BIT05_T | BIT04_T)) == BIT05_T ||
               (GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x6E) & (BIT06_T | BIT05_T | BIT04_T)) == BIT06_T )
            {
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x6E), BIT06_T | BIT05_T | BIT04_T);
            }

            // Make sure reg_spi0_mode != 4
            if((GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68) & (BIT02_T | BIT01_T | BIT00_T)) == BIT02_T)
            {
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT02_T | BIT01_T | BIT00_T);
            }

            //Make sure reg_i2c2_mode != 2
            if((GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x6F) & (BIT10_T | BIT09_T| BIT08_T)) == BIT09_T)
            {
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x6F), BIT10_T | BIT09_T| BIT08_T);
            }


        }

    }

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_CARD_PullPADPin
*     @author jeremy.wang (2015/7/28)
 * Desc: Pull PAD Pin for Special Purpose (Avoid Power loss.., Save Power)
*
 * @param ePAD : PAD
 * @param ePinPull : Pull up/Pull down
 * @param bTwoCard :  two card(1 bit) or not
----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_PullPADPin(PADEmType ePAD, PinPullEmType ePinPull, BOOL_T bTwoCard)
{
    if(ePAD == EV_PAD1) //PAD_SD
    {
        if(ePinPull ==EV_PULLDOWN)
        {
            //reg_sd0_pe: D3, D2, D1, D0, CMD=> pull dis
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x12), BIT04_T); //D1
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x13), BIT04_T); //D0
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x15), BIT04_T); //CMD
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x16), BIT04_T); //D3
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x17), BIT04_T); //D2

#if PORT_FROM_KERNEL
            #if 0
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08), BIT02_T | BIT03_T);         //sd mode = 0            
            #else
            //OFF:x67 [B8:B9]/[reg_sd0_mode:reg_sd0_cdz_mode]; [B13:B12]reg_sd1_mode
            //OFF:x68 [B8:B9]reg_sd1_cdz_mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT13_T | BIT12_T | BIT09_T | BIT08_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT09_T | BIT08_T);
            #endif
#else
            aaa
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08), BIT03_T);         //sd mode2 = 0
#endif
                                                                                       //
            //CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08), BIT08_T);         //sdio mode1 = 0

            //SD_ClK
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x14), BIT02_T); //reg_sd0_gpio_oen_4
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x14), BIT01_T); //reg_sd0_gpio_out_4

            //SD_CMD
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x15), BIT02_T); //reg_sd0_gpio_oen_5
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x15), BIT01_T); //reg_sd0_gpio_out_5

            //SD_D0
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x13), BIT02_T); //reg_sd0_gpio_oen_3
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x13), BIT01_T); //reg_sd0_gpio_out_3

            //SD_D1
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x12), BIT02_T); //reg_sd0_gpio_oen_2
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x12), BIT01_T); //reg_sd0_gpio_out_2

            //SD_D2
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x17), BIT02_T); //reg_sd0_gpio_oen_7
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x17), BIT01_T); //reg_sd0_gpio_out_7

            //SD_D3
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x16), BIT02_T); //reg_sd0_gpio_oen_6
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x16), BIT01_T); //reg_sd0_gpio_out_6
        }
        else if(ePinPull == EV_PULLUP)
        {
            //reg_sd0_pe:D3, D2, D1, D0, CMD=> pull en
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x12), BIT04_T); //D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x13), BIT04_T); //D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x15), BIT04_T); //CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x16), BIT04_T); //D3
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x17), BIT04_T); //D2

            //SD_CLK
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x14), BIT02_T); // reg_sd0_gpio_oen_4

            //SD_CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x15), BIT02_T); // reg_sd0_gpio_oen_5

            //SD_D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x13), BIT02_T); // reg_sd0_gpio_oen_3

            //SD_D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x12), BIT02_T); // reg_sd0_gpio_oen_2

            //SD_D2
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x17), BIT02_T); // reg_sd0_gpio_oen_7

            //SD_D3
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x16), BIT02_T); // reg_sd0_gpio_oen_6
#if PORT_FROM_KERNEL
            #if 0
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08), BIT02_T | BIT03_T);  //sd mode2 = 0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08), BIT02_T );           //sd mode2 = 1
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08), BIT08_T);            //sdio mode1 =0
            #else
            //OFF:x67 [B8:B9]/[reg_sd0_mode:reg_sd0_cdz_mode]; [B13:B12]reg_sd1_mode
            //OFF:x68 [B8:B9]reg_sd1_cdz_mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT13_T | BIT12_T | BIT09_T | BIT08_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT08_T | BIT09_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T | BIT09_T);
            #endif
#else
            //OFF:x67 [B8:B9]/[reg_sd0_mode:reg_sd0_cdz_mode]; [B13:B12]reg_sd1_mode
            //OFF:x68 [B8:B9]reg_sd1_cdz_mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT13_T | BIT12_T | BIT09_T | BIT08_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT08_T | BIT09_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T | BIT09_T);
#endif
        }
    }
    else if(ePAD == EV_PAD2) //PAD_NAND
    {
        if(ePinPull ==EV_PULLDOWN)
        {
            //D3, D2, D1, D0, CMD=> pull dis
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x27), BIT04_T); //D1
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x28), BIT04_T); //D0
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2A), BIT04_T); //CMD
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2B), BIT04_T); //D3
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2C), BIT04_T); //D2

            //OFF:x67 [B8:B9]/[reg_sd0_mode:reg_sd0_cdz_mode]; [B13:B12]reg_sd1_mode
            //OFF:x68 [B8:B9]reg_sd1_cdz_mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT13_T | BIT12_T | BIT09_T | BIT08_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT08_T | BIT09_T);


            //SD_ClK
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x29), BIT02_T);           // output mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x29), BIT01_T);           // output:0

            //SD_CMD
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2A), BIT02_T);           // output mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2A), BIT01_T);           // output:0

            //SD_D0
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x28), BIT02_T);           // output mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x28), BIT01_T);           // output:0

            //SD_D1
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x27), BIT02_T);           // output mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x27), BIT01_T);           // output:0

            //SD_D2
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2C), BIT02_T);           // output mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2C), BIT01_T);           // output:0

            //SD_D3
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2B), BIT02_T);           // output mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2B), BIT01_T);           // output:0
        }
        else if(ePinPull == EV_PULLUP)
        {
            //D3, D2, D1, D0, CMD=> pull en
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x27), BIT04_T); //D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x28), BIT04_T); //D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2A), BIT04_T); //CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2B), BIT04_T); //D3
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2C), BIT04_T); //D2

            //SD_CLK
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x29), BIT02_T); // input mode

            //SD_CMD
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2A), BIT02_T); // input mode

            //SD_D0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x28), BIT02_T); // input mode

            //SD_D1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x27), BIT02_T); // input mode

            //SD_D2
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2C), BIT02_T); // input mode

            //SD_D3
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2B), BIT02_T); // input mode


            //OFF:x67 [B8:B9]/[reg_sd0_mode:reg_sd0_cdz_mode]; [B13:B12]reg_sd1_mode
            //OFF:x68 [B8:B9]reg_sd1_cdz_mode
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT12_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT08_T);
        }

    }
}

//***********************************************************************************************************
// Clock Setting for Card Platform
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_CARD_SetClock
*     @author jeremy.wang (2015/7/23)
 * Desc: Set Clock Level by Real Clock from IP
*
 * @param eIP : FCIE1/FCIE2/...
 * @param u32ClkFromIPSet : Clock Value From IP Source Set
----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_SetClock(IPEmType eIP, U32_T u32ClkFromIPSet)
{
    if(eIP == EV_IP_FCIE1)
    {
#define REG_CLK_SD0   (0x43) //new
#define REG_CLK_SDIO0 (0x45) //org
#if PORT_FROM_KERNEL
    #define FCIE1_CLK REG_CLK_SD0
#else
    #define FCIE1_CLK REG_CLK_SDIO0
#endif
#if PORT_FROM_KERNEL
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE1_CLK), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T); //[5]Boot_Sel [4:2]: Clk_Sel [1]: Clk_i [0]: Clk_g
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE1_CLK), BIT05_T); //boot sel
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK,0x25), BIT07_T); //[B3/B7]:[SDIO/SD] select BOOT clock source (glitch free) - 0: select BOOT clock 12MHz (xtali), 1: select FCIE/SPI clock source
#else
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,0x45), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T); //[5]Boot_Sel [4:2]: Clk_Sel [1]: Clk_i [0]: Clk_g
#endif
        switch(u32ClkFromIPSet)
        {
            case CLK1_F:      //48000KHz
                break;
            case CLK1_E:      //43200KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE1_CLK), BIT02_T); //1
                break;
            case CLK1_D:      //40000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE1_CLK), BIT03_T); //2
                break;
            case CLK1_C:      //36000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE1_CLK), BIT03_T|BIT02_T); //3
                break;
            case CLK1_B:      //32000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE1_CLK), BIT04_T); //4
                break;
            case CLK1_A:      //20000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE1_CLK), BIT04_T|BIT02_T); //5
                break;
            case CLK1_9:      //12000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE1_CLK), BIT04_T|BIT03_T); //6
                break;
            case CLK1_8:      //300KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE1_CLK), BIT04_T|BIT03_T|BIT02_T); //7
                break;
        }
    }
    else if(eIP == EV_IP_FCIE2)
    {
        #define FCIE2_CLK REG_CLK_SDIO0
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE2_CLK), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T); //[5]Boot_Sel [4:2]: Clk_Sel [1]: Clk_i [0]: Clk_g
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE2_CLK), BIT05_T); //boot sel
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK,0x25), BIT07_T | BIT03_T); //[B3/B7]:[SDIO/SD] select BOOT clock source (glitch free) - 0: select BOOT clock 12MHz (xtali), 1: select FCIE/SPI clock source

        switch(u32ClkFromIPSet)
        {
            case CLK2_F:      //48000KHz
                break;
            case CLK2_E:      //43200KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE2_CLK), BIT02_T); //1
                break;
            case CLK2_D:      //40000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE2_CLK), BIT03_T); //2
                break;
            case CLK2_C:      //36000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE2_CLK), BIT03_T|BIT02_T); //3
                break;
            case CLK2_B:      //32000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE2_CLK), BIT04_T); //4
                break;
            case CLK2_A:      //20000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE2_CLK), BIT04_T|BIT02_T); //5
                break;
            case CLK2_9:      //12000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE2_CLK), BIT04_T|BIT03_T); //6
                break;
            case CLK2_8:      //300KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,FCIE2_CLK), BIT04_T|BIT03_T|BIT02_T); //7
                break;
        }
    }
}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_CARD_FindClockSetting
*     @author jeremy.wang (2015/7/20)
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
 *     @author jeremy.wang (2015/7/20)
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
* Function: Hal_CARD_PowerOn
*     @author jeremy.wang (2015/7/29)
* Desc: Power ON Card Power
*
* @param ePAD : PAD
* @param u16DelayMs : Delay ms for stable power
----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_PowerOn(PADEmType ePAD, U16_T u16DelayMs)
{
    if(ePAD==EV_PAD1) //PAD_SD
    {
#if PORT_FROM_KERNEL
        #if 0
        if((CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x03)) & (BIT02_T | BIT01_T | BIT00_T)) == BIT00_T)
        {//make sure it's reg_fuart_mode != 1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x03), BIT02_T);//reg_faurt_mode |= 0x04; (=5)
        }
        #else
        if((CARD_REG(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x6E)) & (BIT06_T | BIT05_T | BIT04_T)) == BIT04_T)
        {//make sure it's reg_fuart_mode != 1
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x6E), BIT06_T);//reg_faurt_mode |= 0x04; (=5)
        }
        #endif
#endif
        #if 1
        //Before I6E: sd card power pin is PAD_FUART_RTS
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x25), BIT02_T); // reg_fuart_gpio_oen_3
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x25), BIT01_T); // reg_fuart_gpio_out_3
        #endif
        #if 1
        //I6E_ECO: sd card power enable. (Change fuart_gpio to sd0_gpio0)
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x10), BIT02_T); //reg_sd0_gpio_oen_0
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x10), BIT01_T); //reg_sd0_gpio_out_0
        #endif
    }
    else if(ePAD==EV_PAD2) //PAD_NAND
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2D), BIT02_T);   // reg_sd1_gpio_oen_7
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2D), BIT01_T);   // reg_sd1_gpio_out_7

    }

    Hal_Timer_mDelay(u16DelayMs);

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_CARD_PowerOff
*     @author jeremy.wang (2015/7/29)
* Desc: Power Off Card Power
*
* @param ePAD : PAD
* @param u16DelayMs :  Delay ms to confirm no any spower
----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_PowerOff(PADEmType ePAD, U16_T u16DelayMs)
{
    if( (ePAD==EV_PAD1)) //PAD_SD
    {
        #if 1
        //Before I6E: sd card power pin is PAD_FUART_RTS
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x25), BIT02_T); // reg_fuart_gpio_oen_3
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x25), BIT01_T); // reg_fuart_gpio_out_3
        #endif
        #if 1
        //I6E_ECO: sd card power enable. (Change fuart_gpio to sd0_gpio0)
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x10), BIT02_T); //reg_sd0_gpio_oen_0
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x10), BIT01_T); //reg_sd0_gpio_out_0
        #endif
    }
    else if(ePAD==EV_PAD2) //PAD_NAND
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2D), BIT02_T);   // reg_sd1_gpio_oen_7
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2D), BIT01_T);   // reg_sd1_gpio_out_7

    }

    Hal_Timer_mDelay(u16DelayMs);
}


//***********************************************************************************************************
// Card Detect and GPIO Setting for Card Platform
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_CARD_InitGPIO
*     @author jeremy.wang (2015/7/30)
* Desc: Init GPIO Setting for CDZ or other GPIO (Pull high/low and driving, base SD/GPIO mode setting)
*
* @param eGPIO : GPIO1/GPIO2/...
* @param bEnable : Enable GPIO or disable GPIO to avoid loss power
----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_InitGPIO(GPIOEmType eGPIO, BOOL_T bEnable)
{
    if( eGPIO==EV_GPIO1 ) //EV_GPIO1 for Slot 0
    {
        #if 0
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_SLEEP_BANK, 0x28), BIT14_T);   //SD_CDZ mode en
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_GPIO_BANK, 0x47), BIT00_T);    //input mode
        #else
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x11), BIT02_T); //PAD_SD0_CDZ:reg_sd0_gpio_oen_1
        #endif
    }
    else if( eGPIO==EV_GPIO2 ) //EV_GPIO2 for Slot 1
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x26), BIT02_T); //PAD_SD1_CDZ:reg_sd0_gpio_oen_0
    }

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_CARD_GetGPIOState
*     @author jeremy.wang (2015/7/30)
* Desc: Get GPIO input mode value (Include input mode setting)
*
* @param eGPIO : GPIO1/GPIO2/...
*
* @return BOOL_T  : TRUE or FALSE
----------------------------------------------------------------------------------------------------------*/
BOOL_T Hal_CARD_GetGPIOState(GPIOEmType eGPIO)
{
    U16_T u16Reg = 0;

    if( eGPIO==EV_GPIO1 ) //EV_GPIO1 for Slot 0
    {
        #if (EN_SDMMC_CDZREV)
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x11), BIT02_T);    //PAD_SD0_CDZ:reg_sd0_gpio_oen_1
        u16Reg = CARD_REG(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x11)) & BIT00_T; //PAD_SD0_CDZ:reg_sd0_gpio_in_1
        #else
        u16Reg = CARD_REG(GET_CARD_REG_ADDR(A_PM_GPIO_BANK, 0x47)) & BIT02_T;
        #endif
        if(!u16Reg) //Low Active
            return (TRUE);
        else
            return (FALSE);

    }
    if( eGPIO==EV_GPIO2 ) //EV_GPIO2 for Slot 1
    {
        #if (EN_SDMMC_CDZREV)
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x26), BIT02_T);    //PAD_SD1_CDZ:reg_sd1_gpio_oen_0
        u16Reg = CARD_REG(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x26)) & BIT00_T; //PAD_SD1_CDZ:reg_sd1_gpio_in_0
        #else
        u16Reg = CARD_REG(GET_CARD_REG_ADDR(A_PM_GPIO_BANK, 0x47)) & BIT02_T;
        #endif
        if(!u16Reg) //Low Active
            return (TRUE);
        else
            return (FALSE);
    }

    return (FALSE);
}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_CARD_SetGPIOState
*     @author jeremy.wang (2015/7/30)
* Desc: Set GPIO output mode value (Include output mode setting), it's for SDIO WIFI control using
*
* @param eGPIO : GPIO1/GPIO2/...
* @param bOutputState : TRUE or FALSE
----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_SetGPIOState(GPIOEmType eGPIO, BOOL_T bOutputState)
{

    /*if( eGPIO==EV_GPIO1 ) //EV_GPIO1 for Slot 0
    {
        CARD_REG16_OFF(GET_CARD_REG_ADDR(A_PMGPIO_BANK, 0x05), BIT00_T);           //PMU_GPIO_OUT_EN

        if(bOutputState)
        {
            CARD_REG16_ON(GET_CARD_REG_ADDR(A_PMGPIO_BANK, 0x07), BIT00_T);        //PMU_GPIO_OUT=1
        }
        else
        {
            CARD_REG16_OFF(GET_CARD_REG_ADDR(A_PMGPIO_BANK, 0x07), BIT00_T);       //PMU_GPIO_OUT=0
        }
    }
    if( eGPIO==EV_GPIO2 ) //EV_GPIO2 for Slot 1
    {
    }
    if( eGPIO==EV_GPIO3 ) //EV_GPIO2 for Slot 1
    {
    }

    // Add a 500ms Delay after card removing to avoid the next card inserting issue
    if(bOutputState==1)
    {
        Hal_Timer_mSleep(500);
    }
    else // For 0->1, 1->0 stable
    {
        Hal_Timer_mSleep(1);
    }*/

}

/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_CARD_GetGPIONum
*     @author jeremy.wang (2015/7/30)
* Desc: Get GPIO number for special platform (like Linux) to use it get irq number
*
* @param eGPIO : GPIO1/GPIO2/...
*
* @return U32_T  : GPIO number
----------------------------------------------------------------------------------------------------------*/
U32_T Hal_CARD_GetGPIONum(GPIOEmType eGPIO)
{
    S32_T s32GPIO = -1;

    /*if( eGPIO==EV_GPIO1 ) //EV_GPIO1 for Slot 0
    {
        //s32GPIO = DrvPadmuxGetGpio(IO_CHIP_INDEX_SD_CDZ);
    }
    else if( eGPIO==EV_GPIO2)
    {
    }*/

    if(s32GPIO>0)
        return (U32_T)s32GPIO;
    else
        return 0;
}


#if (D_OS == D_OS__LINUX)
#include <mach/irqs.h>
#include <linux/irq.h>
extern void ms_irq_set_polarity(struct irq_data *data,unsigned char polarity);
#endif

/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_CARD_SetGPIOIntAttr
*     @author jeremy.wang (2015/7/30)
* Desc: Set GPIO Interrupt Attribute (Option 1..5), it could design for different requirement
*
* @param eGPIO : GPIO1/GPIO2/...
* @param eGPIOOPT : Option1/Option2/...
----------------------------------------------------------------------------------------------------------*/
void Hal_CARD_SetGPIOIntAttr(GPIOEmType eGPIO, GPIOOptEmType eGPIOOPT)
{

#if (D_OS == D_OS__LINUX)
    struct irq_data sd_irqdata;
    struct irq_chip *chip;

    if( eGPIO==EV_GPIO1 ) //EV_GPIO1 for Slot 0
    {
        sd_irqdata.irq = INT_PMU_SD_DETECT0;
        chip = irq_get_chip(sd_irqdata.irq);

        if(eGPIOOPT==EV_GPIO_OPT1) //clear interrupt
        {
            chip->irq_ack(&sd_irqdata);
            //CARD_REG16_ON(GET_CARD_REG_ADDR(A_PMSLEEP_BANK, 0x75),  BIT00_T);
        }
        else if((eGPIOOPT==EV_GPIO_OPT2))
        {
        }
        else if((eGPIOOPT==EV_GPIO_OPT3))  //sd polarity _HI Trig for remove
        {
            ms_irq_set_polarity(&sd_irqdata , 0);
            //CARD_REG16_OFF(GET_CARD_REG_ADDR(A_PMSLEEP_BANK, 0x7B),  BIT00_T);
        }
        else if((eGPIOOPT==EV_GPIO_OPT4)) //sd polarity _LO Trig for insert
        {
            ms_irq_set_polarity(&sd_irqdata , 1);
            //CARD_REG16_ON(GET_CARD_REG_ADDR(A_PMSLEEP_BANK, 0x7B),  BIT00_T);

        }
    }
    else if( eGPIO==EV_GPIO2)
    {
    }
    else if(eGPIO==EV_GPIO3)
    {

    }
#endif


}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_CARD_GPIOIntFilter
*     @author jeremy.wang (2015/7/30)
* Desc: GPIO Interrupt Filter, it could design to filter GPIO Interrupt (Many sources belong to the same one)
*
* @param eGPIO : GPIO1/GPIO2/...
*
* @return BOOL_T  : TRUE or FALSE
----------------------------------------------------------------------------------------------------------*/
BOOL_T Hal_CARD_GPIOIntFilter(GPIOEmType eGPIO)
{

    /*if( eGPIO==EV_GPIO1 ) //EV_GPIO1 for Slot 0
    {
        return (TRUE);
    }
    else if( eGPIO==EV_GPIO2 )
    {
         return (TRUE);
    }
    else if( eGPIO==EV_GPIO3 )
    {
         return (TRUE);
    }*/

    return (FALSE);
}


//***********************************************************************************************************
// MIU Setting for Card Platform
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_CARD_TransMIUAddr
*     @author jeremy.wang (2015/7/31)
* Desc: Transfer original address to HW special dma address (MIU0/MIU1)
*
* @param u32Addr : Original address
*
* @return U32_T  : DMA address
----------------------------------------------------------------------------------------------------------*/
U32_T Hal_CARD_TransMIUAddr(U32_T u32Addr)
{
    if (u32Addr > 0x20000000)
    {
        return u32Addr - 0x20000000;
    }
    printf("Invalid addr :%X\n", u32Addr);
    return u32Addr;
}











