/*
* hal_card_platform.c- Sigmastar
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
 * FileName hal_card_platform_iNF5.c
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
#include "gpio.h"
#include "padmux.h"
#include "mdrv_gpio.h"
#include "mdrv_padmux.h"
#include "hal_card_platform_pri_config.h"

//***********************************************************************************************************
// Config Setting (Internal)
//***********************************************************************************************************
// Platform Register Basic Address
//------------------------------------------------------------------------------------
#define A_CLKGEN_BANK       GET_CARD_REG_ADDR(A_RIU_BASE, 0x81C00)//1038h
#define A_PADTOP_BANK       GET_CARD_REG_ADDR(A_RIU_BASE, 0x81E00)//103Ch
#define A_GPI_INT_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0x81E80)//103Dh
#define A_PM_SLEEP_BANK     GET_CARD_REG_ADDR(A_RIU_BASE, 0x00700)//0Eh
#define A_PM_GPIO_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0x00780)//0Fh
#define A_CHIPTOP_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0x80F00)//101Eh
//#define A_MCM_SC_GP_BANK    GET_CARD_REG_ADDR(A_RIU_BASE, 0x89900)//1132h
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



#define REG_CLK_IP_SD   (0x43)
#define REG_CLK_IP_SDIO (0x45)


#define pr_sd_err(fmt, arg...)  printk(fmt, ##arg)

static volatile U16_T _gu16PowerPadNumForEachIp[IP_TOTAL] = {PAD_UNKNOWN};
static volatile U16_T _gu16CdzPadNumForEachIp[IP_TOTAL] = {PAD_UNKNOWN};


//***********************************************************************************************************
// IP Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_IPOnceSetting(IpOrder eIP)
{
    IpSelect eIpSel = (IpSelect)eIP;

#if (FORCE_SWITCH_PAD)
    // reg_all_pad_in => Close
    CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x50), BIT15_T);
#endif

    // Clock Source
    if (eIpSel == IP_SD)
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK,0x25), BIT07_T);
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SD), BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T);
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SD), BIT05_T); // boot sel
    }
    else if (eIpSel == IP_SDIO)
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK,0x25), BIT03_T);
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SDIO), BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T);
    }
}

//***********************************************************************************************************
// PAD Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_InitPADPin(IpOrder eIP, PadOrder ePAD)
{
    PadSelect ePadSel = (PadSelect)ePAD;

    if (ePadSel == PAD_SD)
    {
        // Driving for PAD_SD_CLK, PAD_SD_CMD, PAD_SD_D0 ~ PAD_SD_D3
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x32), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T);   //CLK, D3, D2, D1, D0, CMD => drv: 0 low , 1 high
    }
    else if (ePadSel == PAD_SD1)
    {
        // PAD_SD1_IO0 ~ PAD_SD1_IO5 -> Driving for D0,D1,D2,D3,CMD,CLK
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x30), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T);
    }

    // Pull Down
    Hal_CARD_PullPADPin(eIP, ePAD, EV_PULLDOWN);
}

BOOL_T Hal_CARD_GetPadInfoCdzPad(IpOrder eIP, U32_T *nPadNum)
{
#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
    return TRUE;
#else
    return TRUE;
#endif
}

BOOL_T Hal_CARD_GetPadInfoPowerPad(IpOrder eIP, U32_T *nPadNum)
{
#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
    return TRUE;
#else
    return TRUE;
#endif
}

void Hal_CARD_ConfigSdPad(IpOrder eIP, PadOrder ePAD) //Hal_CARD_SetPADToPortPath
{
    IpSelect eIpSel = (IpSelect)eIP;
    PadSelect ePadSel = (PadSelect)ePAD;

#if (PADMUX_SET == PADMUX_SET_BY_FUNC)

    if (0 == mdrv_padmux_active())
    {
        if (eIpSel == IP_SD)
        {
            if (ePadSel == PAD_SD)
            {
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SD_MODE);
            }
        }
        else if (eIpSel == IP_SDIO)
        {
            if (ePadSel == PAD_SD1)
            {
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SDIO_MODE);
            }
        }
    }

#else //

    if (eIpSel == IP_SD)
    {
        if (ePadSel == PAD_SD)
        {
            // SD mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08), BIT02_T | BIT03_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08), BIT02_T);

            // Make sure reg_spi1_mode != 3
            if((GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0C) & (BIT06_T | BIT05_T | BIT04_T)) == (BIT05_T | BIT04_T))
            {
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0C), BIT06_T | BIT05_T | BIT04_T);
            }
        }
    }
    else if (eIpSel == IP_SDIO)
    {
        if (ePadSel == PAD_SD1)
        {
            U16_T temp;

            // SD mode
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08), BIT08_T);

            // Make sure reg_spi0_mode != 4
            if((GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0C) & (BIT02_T | BIT01_T | BIT00_T)) == BIT02_T)
            {
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0C), BIT02_T | BIT01_T | BIT00_T);
            }

            // Make sure reg_uart0_mode != 4
            if((GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x03) & (BIT06_T | BIT05_T | BIT04_T)) == BIT06_T)
            {
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x03), BIT06_T | BIT05_T | BIT04_T);
            }

            // Make sure reg_fuart_mode != 4 && 6
            temp = CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x03)) & (BIT02_T | BIT01_T | BIT00_T);
            if(temp == (BIT02_T) || temp == (BIT02_T|BIT01_T) )
            {
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x03), BIT02_T | BIT01_T | BIT00_T);
            }

            // PWM is not cleared
            // ...

            //Make sure reg_ttl_mode != 1
            if((GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0F) & (BIT07_T | BIT06_T)) == BIT06_T)
            {
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0F), BIT06_T | BIT07_T);
            }

            //Make sure reg_dmic_mode != 2
            if((GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0F) & (BIT09_T | BIT08_T)) == BIT09_T)
            {
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0F), BIT09_T | BIT08_T);
            }

            //Make sure reg_i2s_mode != 2
            if((GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0F) & (BIT11_T | BIT10_T)) == BIT11_T)
            {
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0F), BIT11_T | BIT10_T);
            }
        }
    }

#endif
}

void Hal_CARD_ConfigPowerPad(IpOrder eIP, U16_T nPadNum)
{
    IpSelect eIpSel = (IpSelect)eIP;
    U16_T nPadNo = nPadNum;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }

    if (nPadNo == PAD_UNKNOWN)
    {
        // Save Power PadNum
        _gu16PowerPadNumForEachIp[(U16_T)eIpSel] = PAD_UNKNOWN;
        return;
    }

#if (PADMUX_SET == PADMUX_SET_BY_FUNC)

    if (0 == mdrv_padmux_active())
    {
        MDrv_GPIO_PadVal_Set(nPadNo, PINMUX_FOR_GPIO_MODE);
    }

#else

    switch (nPadNo)
    {
        case PAD_FUART_RTS:

            //make sure it's reg_fuart_mode != 1
            if ((CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x03)) & (BIT02_T | BIT01_T | BIT00_T)) == BIT00_T)
            {
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x03), (BIT02_T | BIT01_T | BIT00_T));
            }

            break;

        case PAD_PM_GPIO9:

            //
            break;

        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            return;
            break;
    }

#endif

    // Save Power PadNum
    _gu16PowerPadNumForEachIp[(U16_T)eIpSel] = nPadNo;

    // Default power off
    Hal_CARD_PowerOff(eIP, 0);
}

void Hal_CARD_PullPADPin(IpOrder eIP, PadOrder ePAD, PinPullEmType ePinPull)
{
    IpSelect eIpSel = (IpSelect)eIP;
    PadSelect ePadSel = (PadSelect)ePAD;

    // Whatever if using padmux dts, we need to switch the pad mode.

    // IP_SD
    if (eIpSel == IP_SD)
    {
        if (ePadSel == PAD_SD)
        {
            if (ePinPull == EV_PULLDOWN)
            {
                // Pull Disable - D3, D2, D1, D0, CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x32), BIT12_T|BIT11_T|BIT10_T|BIT09_T|BIT08_T);

                // PAD -> GPIO mode
#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
                MDrv_GPIO_PadVal_Set(PAD_SD_CLK, PINMUX_FOR_GPIO_MODE);
                MDrv_GPIO_PadVal_Set(PAD_SD_CMD, PINMUX_FOR_GPIO_MODE);
                MDrv_GPIO_PadVal_Set(PAD_SD_D0, PINMUX_FOR_GPIO_MODE);
                MDrv_GPIO_PadVal_Set(PAD_SD_D1, PINMUX_FOR_GPIO_MODE);
                MDrv_GPIO_PadVal_Set(PAD_SD_D2, PINMUX_FOR_GPIO_MODE);
                MDrv_GPIO_PadVal_Set(PAD_SD_D3, PINMUX_FOR_GPIO_MODE);
#else
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08), BIT02_T | BIT03_T);    // SDIO mode = 0
#endif

                // Output Low
#if (GPIO_SET == GPIO_SET_BY_FUNC)
                MDrv_GPIO_Set_Low(PAD_SD_CLK);
                MDrv_GPIO_Set_Low(PAD_SD_CMD);
                MDrv_GPIO_Set_Low(PAD_SD_D0);
                MDrv_GPIO_Set_Low(PAD_SD_D1);
                MDrv_GPIO_Set_Low(PAD_SD_D2);
                MDrv_GPIO_Set_Low(PAD_SD_D3);
#else
                //SD_ClK
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x50), BIT05_T);   // output mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x50), BIT04_T);   // output:0
                //SD_CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x51), BIT05_T);   // output mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x51), BIT04_T);   // output:0
                //SD_D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x52), BIT05_T);   // output mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x52), BIT04_T);   // output:0
                //SD_D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x53), BIT05_T);   // output mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x53), BIT04_T);   // output:0
                //SD_D2
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x54), BIT05_T);   // output mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x54), BIT04_T);   // output:0
                //SD_D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x55), BIT05_T);   // output mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x55), BIT04_T);   // output:0
#endif
            }
            else if (ePinPull == EV_PULLUP)
            {
                // Pull Enable - D3, D2, D1, D0, CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x32), BIT12_T|BIT11_T|BIT10_T|BIT09_T|BIT08_T);

                // Input
#if (GPIO_SET == GPIO_SET_BY_FUNC)
                MDrv_GPIO_Pad_Odn(PAD_SD_CLK);
                MDrv_GPIO_Pad_Odn(PAD_SD_CMD);
                MDrv_GPIO_Pad_Odn(PAD_SD_D0);
                MDrv_GPIO_Pad_Odn(PAD_SD_D1);
                MDrv_GPIO_Pad_Odn(PAD_SD_D2);
                MDrv_GPIO_Pad_Odn(PAD_SD_D3);
#else
                //SD_CLK
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x50), BIT05_T);   // input mode
                //SD_CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x51), BIT05_T);   // input mode
                //SD_D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x52), BIT05_T);   // input mode
                //SD_D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x53), BIT05_T);   // input mode
                //SD_D2
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x54), BIT05_T);   // input mode
                //SD_D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x55), BIT05_T);   // input mode
#endif
                // SD Mode
#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SD_MODE);
#else
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08), BIT02_T | BIT03_T);    // SDIO mode = 0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08), BIT02_T );             // SDIO mode = 1
#endif
            }
        }
    }

    // IP_SDIO
    if (eIpSel == IP_SDIO)
    {
        if (ePadSel == PAD_SD1)
        {
            if (ePinPull == EV_PULLDOWN)
            {
                // Pull Disable - D3, D2, D1, D0, CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x31), BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T);

                // PAD -> GPIO mode
#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
                MDrv_GPIO_PadVal_Set(PAD_SD1_IO5, PINMUX_FOR_GPIO_MODE); // CLK
                MDrv_GPIO_PadVal_Set(PAD_SD1_IO4, PINMUX_FOR_GPIO_MODE); // CMD
                MDrv_GPIO_PadVal_Set(PAD_SD1_IO0, PINMUX_FOR_GPIO_MODE); // D0
                MDrv_GPIO_PadVal_Set(PAD_SD1_IO1, PINMUX_FOR_GPIO_MODE); // D1
                MDrv_GPIO_PadVal_Set(PAD_SD1_IO2, PINMUX_FOR_GPIO_MODE); // D2
                MDrv_GPIO_PadVal_Set(PAD_SD1_IO3, PINMUX_FOR_GPIO_MODE); // D3
#else
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08), BIT08_T);  // SDIO mode = 0
#endif

                // Output Low
#if (GPIO_SET == GPIO_SET_BY_FUNC)
                MDrv_GPIO_Set_Low(PAD_SD1_IO5);
                MDrv_GPIO_Set_Low(PAD_SD1_IO4);
                MDrv_GPIO_Set_Low(PAD_SD1_IO0);
                MDrv_GPIO_Set_Low(PAD_SD1_IO1);
                MDrv_GPIO_Set_Low(PAD_SD1_IO2);
                MDrv_GPIO_Set_Low(PAD_SD1_IO3);
#else
                //SD_ClK
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x45), BIT05_T);   // output mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x45), BIT04_T);   // output:0
                //SD_CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x44), BIT05_T);   // output mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x44), BIT04_T);   // output:0
                //SD_D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x40), BIT05_T);   // output mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x40), BIT04_T);   // output:0
                //SD_D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x41), BIT05_T);   // output mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x41), BIT04_T);   // output:0
                //SD_D2
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x42), BIT05_T);   // output mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x42), BIT04_T);   // output:0
                //SD_D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x43), BIT05_T);   // output mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x43), BIT04_T);   // output:0
#endif
            }
            else if (ePinPull == EV_PULLUP)
            {
                // Pull Enable - D3, D2, D1, D0, CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x31), BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T);

                // Input
#if (GPIO_SET == GPIO_SET_BY_FUNC)
                MDrv_GPIO_Pad_Odn(PAD_SD1_IO5);
                MDrv_GPIO_Pad_Odn(PAD_SD1_IO4);
                MDrv_GPIO_Pad_Odn(PAD_SD1_IO0);
                MDrv_GPIO_Pad_Odn(PAD_SD1_IO1);
                MDrv_GPIO_Pad_Odn(PAD_SD1_IO2);
                MDrv_GPIO_Pad_Odn(PAD_SD1_IO3);
#else
                //SD_CLK
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x45), BIT05_T);   // input mode
                //SD_CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x44), BIT05_T);   // input mode
                //SD_D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x40), BIT05_T);   // input mode
                //SD_D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x41), BIT05_T);   // input mode
                //SD_D2
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x42), BIT05_T);   // input mode
                //SD_D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x43), BIT05_T);   // input mode
#endif
                // SD Mode
#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SDIO_MODE);
#else
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08), BIT08_T);  // SDIO mode = 1
#endif
            }
        }
    }
}

//***********************************************************************************************************
// Clock Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_SetClock(IpOrder eIP, U32_T u32ClkFromIPSet)
{
    IpSelect eIpSel = (IpSelect)eIP;

    if (eIpSel == IP_SD)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SD), BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T);

        switch(u32ClkFromIPSet)
        {
            case CLK1_F:      //48000KHz
                break;
            case CLK1_E:      //43200KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SD), BIT02_T); //1
                break;
            case CLK1_D:      //40000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SD), BIT03_T); //2
                break;
            case CLK1_C:      //36000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SD), BIT03_T|BIT02_T); //3
                break;
            case CLK1_B:      //32000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SD), BIT04_T); //4
                break;
            case CLK1_A:      //20000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SD), BIT04_T|BIT02_T); //5
                break;
            case CLK1_9:      //12000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SD), BIT04_T|BIT03_T); //6
                break;
            case CLK1_8:      //300KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SD), BIT04_T|BIT03_T|BIT02_T); //7
                break;
        }
    }
    else if (eIpSel == IP_SDIO)
    {
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SDIO), BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T);

        switch(u32ClkFromIPSet)
        {
            case CLK2_F:      //48000KHz
                break;
            case CLK2_E:      //43200KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SDIO), BIT02_T); //1
                break;
            case CLK2_D:      //40000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SDIO), BIT03_T); //2
                break;
            case CLK2_C:      //36000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SDIO), BIT03_T|BIT02_T); //3
                break;
            case CLK2_B:      //32000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SDIO), BIT04_T); //4
                break;
            case CLK2_A:      //20000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SDIO), BIT04_T|BIT02_T); //5
                break;
            case CLK2_9:      //12000KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SDIO), BIT04_T|BIT03_T); //6
                break;
            case CLK2_8:      //300KHz
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SDIO), BIT04_T|BIT03_T|BIT02_T); //7
                break;
        }
    }
}

U32_T Hal_CARD_FindClockSetting(IpOrder eIP, U32_T u32ReffClk)
{
    U8_T  u8LV = 0;
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

    return u32RealClk;
}

//***********************************************************************************************************
// Power and Voltage Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_PowerOn(IpOrder eIP, U16_T u16DelayMs)
{
    IpSelect eIpSel = (IpSelect)eIP;
    U16_T nPadNo = 0;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }

    nPadNo = _gu16PowerPadNumForEachIp[(U16_T)eIpSel];

    if (nPadNo == PAD_UNKNOWN)
    {
        // Maybe we don't use power pin.
        return;
    }

#if (GPIO_SET == GPIO_SET_BY_FUNC)

    // Whatever mdrv_padmux_active is ON or OFF, just do GPIO_set.
    MDrv_GPIO_Set_Low(nPadNo);

#else

    switch (nPadNo)
    {
        case PAD_FUART_RTS:
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x17), BIT05_T);   // output mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x17), BIT04_T);   // output:0
            break;

        case PAD_PM_GPIO9:
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_GPIO_BANK, 0x09), BIT00_T);  // output mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_GPIO_BANK, 0x09), BIT01_T);  // output:0
            break;

        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            break;
    }

#endif

    Hal_Timer_mSleep(u16DelayMs);
    //Hal_Timer_mDelay(u16DelayMs);
}

void Hal_CARD_PowerOff(IpOrder eIP, U16_T u16DelayMs)
{
    IpSelect eIpSel = (IpSelect)eIP;
    U16_T nPadNo = 0;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }

    nPadNo = _gu16PowerPadNumForEachIp[(U16_T)eIpSel];

    if (nPadNo == PAD_UNKNOWN)
    {
        // Maybe we don't use power pin.
        return;
    }

#if (GPIO_SET == GPIO_SET_BY_FUNC)

    // Whatever mdrv_padmux_active is ON or OFF, just do GPIO_set.
    MDrv_GPIO_Set_High(nPadNo);

#else

    switch (nPadNo)
    {
        case PAD_FUART_RTS:
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x17), BIT05_T);   // output mode
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x17), BIT04_T);   // output:1
            break;

        case PAD_PM_GPIO9:
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PM_GPIO_BANK, 0x09), BIT00_T);  // output mode
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_GPIO_BANK, 0x09), BIT01_T);  // output:1
            break;

        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            break;
    }

#endif

    Hal_Timer_mSleep(u16DelayMs);
    //Hal_Timer_mDelay(u16DelayMs);
}


//***********************************************************************************************************
// Card Detect and GPIO Setting for Card Platform
//***********************************************************************************************************
void Hal_CARD_ConfigCdzPad(IpOrder eIP, U16_T nPadNum) // Hal_CARD_InitGPIO
{
    IpSelect eIpSel = (IpSelect)eIP;
    U16_T nPadNo = nPadNum;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        return;
    }

    if (nPadNo == PAD_UNKNOWN)
    {
        // Save CDZ PadNum
        _gu16CdzPadNumForEachIp[(U16_T)eIpSel] = PAD_UNKNOWN;
        return;
    }

// PADMUX
#if (PADMUX_SET == PADMUX_SET_BY_FUNC)

    if (0 == mdrv_padmux_active())
    {
        MDrv_GPIO_PadVal_Set(nPadNo, PINMUX_FOR_GPIO_MODE);
    }

#else

    switch (nPadNo)
    {
        case PAD_PM_SD_CDZ:
            //
            break;

        case PAD_SD1_IO6:
            //
            break;

        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            return;
            break;
    }

#endif

// GPIO
#if (GPIO_SET == GPIO_SET_BY_FUNC)
    // Whatever mdrv_padmux_active is ON or OFF, just set it to input mode.
    MDrv_GPIO_Pad_Odn(nPadNo);
#else

    switch (nPadNo)
    {
        case PAD_PM_SD_CDZ:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_GPIO_BANK, 0x47), BIT00_T);  //input mode
            break;

        case PAD_SD1_IO6:

            //
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x46), BIT05_T);   // input mode

#if 0 // reg_gpi_glhrm
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_GPI_INT_BANK, 0x42), BIT10_T); // For PAD_SD1_IO6
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_GPI_INT_BANK, 0x45), 8);
#endif
            break;

        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            return;
            break;
    }

#endif

    // This is a special case, SD_CDZ mode ON/OFF status will use different GIC path.
    // We must switch ON - SD_CDZ mode
    if (nPadNo == PAD_PM_SD_CDZ)
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PM_SLEEP_BANK, 0x28), BIT14_T); //SD_CDZ mode en
    }

    // Save CDZ PadNum
    _gu16CdzPadNumForEachIp[(U16_T)eIpSel] = nPadNo;
}

BOOL_T Hal_CARD_GetCdzState(IpOrder eIP) // Hal_CARD_GetGPIOState
{
    IpSelect eIpSel = (IpSelect)eIP;
    U16_T nPadNo = 0;
    U8_T nLv = 0;

    if (eIpSel >= IP_TOTAL)
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
        goto fail;
    }

    nPadNo = _gu16CdzPadNumForEachIp[(U16_T)eIpSel];

    if (nPadNo == PAD_UNKNOWN)
    {
        // Maybe we don't use CDZ pin.
        goto fail;
    }

#if (GPIO_SET == GPIO_SET_BY_FUNC)
    nLv = MDrv_GPIO_Pad_Read(nPadNo);
#else

    switch (nPadNo)
    {
        case PAD_PM_SD_CDZ:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PM_GPIO_BANK, 0x47)) & BIT02_T;
            break;

        case PAD_SD1_IO6:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x46)) & BIT00_T;
            break;

        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            goto fail;
            break;
    }

#endif

    if (!nLv) // Low Active
    {
        return TRUE;
    }

fail:

    return FALSE;
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
U32_T Hal_CARD_TransMIUAddr(U32_T u32Addr, U8_T* pu8MIUSel)
{
    return u32Addr-0x20000000;
}
