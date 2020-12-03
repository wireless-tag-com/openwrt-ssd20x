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

#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
#include "mdrv_puse.h"
#endif

//***********************************************************************************************************
// Config Setting (Internal)
//***********************************************************************************************************
// Platform Register Basic Address
//------------------------------------------------------------------------------------
#define A_CLKGEN_BANK       GET_CARD_REG_ADDR(A_RIU_BASE, 0x81C00)//1038h
#define A_PADTOP_BANK       GET_CARD_REG_ADDR(A_RIU_BASE, 0x81E00)//103Ch
//#define A_GPI_INT_BANK      GET_CARD_REG_ADDR(A_RIU_BASE, 0x81E80)//103Dh
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
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK,0x25), BIT07_T);      //BK:x1133(sc_gp_ctrl)[B7] [0:1]/[boot_clk(12M):sd_clk]
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SD), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T);
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SD), BIT05_T); //BK:x1038(reg_ckg_sd)[B5] [0:1]/[boot_clk(12M):sd_clk]
    }
    else if (eIpSel == IP_SDIO)
    {
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK,0x25), BIT03_T);
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SDIO), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T);
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SDIO), BIT05_T);
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
    else if (ePadSel == PAD_SD1)
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
    else if (ePadSel == PAD_SD1_MD2)
    {
        //reg_sd1_pe:D3, D2, D1, D0, CMD=> pull en
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x18), BIT04_T); //D1
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x19), BIT04_T); //D0
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x1A), BIT04_T); //CMD
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x1C), BIT04_T); //D3
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x1d), BIT04_T); //D2

        //reg_sd1_drv: CLK, D3, D2, D1, D0, CMD => drv: 0
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x18), BIT07_T); //D1
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x19), BIT07_T); //D0
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x1A), BIT07_T); //CLK
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x1B), BIT07_T); //CMD
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x1C), BIT07_T); //D3
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x1D), BIT07_T); //D2
    }

    // Pull Down
    Hal_CARD_PullPADPin(eIP, ePAD, EV_PULLDOWN);
}

BOOL_T Hal_CARD_GetPadInfoCdzPad(IpOrder eIP, U32_T *nPadNum)
{
#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
    int puse;

    if (eIP == IP_ORDER_0)
    {
        puse = MDRV_PUSE_SDIO0_CDZ;
    }
    else if (eIP == IP_ORDER_1)
    {
        puse = MDRV_PUSE_SDIO1_CDZ;
    }
    else
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);

        *nPadNum = PAD_UNKNOWN;
        return FALSE;
    }

    *nPadNum = mdrv_padmux_getpad(puse);
    return TRUE;
#else
    return TRUE;
#endif
}

BOOL_T Hal_CARD_GetPadInfoPowerPad(IpOrder eIP, U32_T *nPadNum)
{
#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
    int puse;

    if (eIP == IP_ORDER_0)
    {
        puse = MDRV_PUSE_SDIO0_PWR;
    }
    else if (eIP == IP_ORDER_1)
    {
        puse = MDRV_PUSE_SDIO1_PWR;
    }
    else
    {
        pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);

        *nPadNum = PAD_UNKNOWN;
        return FALSE;
    }

    *nPadNum = mdrv_padmux_getpad(puse);
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
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SD0_MODE);
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SD0_CDZ_MODE);
            }
        }
        else if (eIpSel == IP_SDIO)
        {
            if (ePadSel == PAD_SD1)
            {
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SD1_MODE_1);
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SD1_CDZ_MODE_1);
            }
            else if (ePadSel == PAD_SD1_MD2)
            {
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SD1_MODE_2);
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SD1_CDZ_MODE_2);
            }
        }
    }

#else //

    if (eIpSel == IP_SD)
    {
        if (ePadSel == PAD_SD)
        {
            // SD mode
            //OFF:x67 [B8]reg_sd0_mode [B9]reg_sd0_cdz_mode [B13:B12]reg_sd1_mode
            //OFF:x68 [B9:B8]reg_sd1_cdz_mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT13_T | BIT12_T | BIT09_T | BIT08_T);
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT08_T | BIT09_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T | BIT09_T);

            //Make sure reg_spi0_mode != 3
            if((GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68) & (BIT02_T | BIT01_T | BIT00_T)) == (BIT01_T | BIT00_T))
            {
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT02_T | BIT01_T | BIT00_T);
            }
        }
    }
    else if (eIpSel == IP_SDIO)
    {
        if (ePadSel == PAD_SD1)
        {
            //SD mode
            //OFF:x67 [B8]reg_sd0_mode [B9]reg_sd0_cdz_mode [B13:B12]reg_sd1_mode
            //OFF:x68 [B9:B8]reg_sd1_cdz_mode
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT12_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT08_T);

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

        case PAD_SD0_GPIO0:
            break;

        case PAD_SD1_GPIO0:
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
                //reg_sd0_pe: D3, D2, D1, D0, CMD=> pull dis
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x12), BIT04_T); //D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x13), BIT04_T); //D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x15), BIT04_T); //CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x16), BIT04_T); //D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x17), BIT04_T); //D2

                // PAD -> GPIO mode
#if 0//(PADMUX_SET == PADMUX_SET_BY_FUNC)
                MDrv_GPIO_PadVal_Set(PAD_SD0_CLK, PINMUX_FOR_GPIO_MODE);
                MDrv_GPIO_PadVal_Set(PAD_SD0_CMD, PINMUX_FOR_GPIO_MODE);
                MDrv_GPIO_PadVal_Set(PAD_SD0_D0, PINMUX_FOR_GPIO_MODE);
                MDrv_GPIO_PadVal_Set(PAD_SD0_D1, PINMUX_FOR_GPIO_MODE);
                MDrv_GPIO_PadVal_Set(PAD_SD0_D2, PINMUX_FOR_GPIO_MODE);
                MDrv_GPIO_PadVal_Set(PAD_SD0_D3, PINMUX_FOR_GPIO_MODE);
#else
                //OFF:x67 [B8]reg_sd0_mode [B9]reg_sd0_cdz_mode [B13:B12]reg_sd1_mode
                //OFF:x68 [B9:B8]reg_sd1_cdz_mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T);
#endif

                // Output Low
#if (GPIO_SET == GPIO_SET_BY_FUNC)
                MDrv_GPIO_Set_Low(PAD_SD0_CLK);
                MDrv_GPIO_Set_Low(PAD_SD0_CMD);
                MDrv_GPIO_Set_Low(PAD_SD0_D0);
                MDrv_GPIO_Set_Low(PAD_SD0_D1);
                MDrv_GPIO_Set_Low(PAD_SD0_D2);
                MDrv_GPIO_Set_Low(PAD_SD0_D3);
#else
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
#endif
            }
            else if(ePinPull == EV_PULLUP)
            {
                //reg_sd0_pe:D3, D2, D1, D0, CMD=> pull en
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x12), BIT04_T); //D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x13), BIT04_T); //D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x15), BIT04_T); //CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x16), BIT04_T); //D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x17), BIT04_T); //D2

                // Input
#if (GPIO_SET == GPIO_SET_BY_FUNC)
                MDrv_GPIO_Pad_Odn(PAD_SD0_CLK);
                MDrv_GPIO_Pad_Odn(PAD_SD0_CMD);
                MDrv_GPIO_Pad_Odn(PAD_SD0_D0);
                MDrv_GPIO_Pad_Odn(PAD_SD0_D1);
                MDrv_GPIO_Pad_Odn(PAD_SD0_D2);
                MDrv_GPIO_Pad_Odn(PAD_SD0_D3);
#else
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
#endif
                // SD Mode
#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SD0_MODE);
#else
                //OFF:x67 [B8]reg_sd0_mode [B9]reg_sd0_cdz_mode [B13:B12]reg_sd1_mode
                //OFF:x68 [B9:B8]reg_sd1_cdz_mode
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT08_T);
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
                //D3, D2, D1, D0, CMD=> pull dis
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x27), BIT04_T); //D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x28), BIT04_T); //D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2A), BIT04_T); //CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2B), BIT04_T); //D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2C), BIT04_T); //D2

                // PAD -> GPIO mode
#if 0//(PADMUX_SET == PADMUX_SET_BY_FUNC)
                MDrv_GPIO_PadVal_Set(PAD_SD1_CLK, PINMUX_FOR_GPIO_MODE); // CLK
                MDrv_GPIO_PadVal_Set(PAD_SD1_CMD, PINMUX_FOR_GPIO_MODE); // CMD
                MDrv_GPIO_PadVal_Set(PAD_SD1_D0, PINMUX_FOR_GPIO_MODE);  // D0
                MDrv_GPIO_PadVal_Set(PAD_SD1_D1, PINMUX_FOR_GPIO_MODE);  // D1
                MDrv_GPIO_PadVal_Set(PAD_SD1_D2, PINMUX_FOR_GPIO_MODE);  // D2
                MDrv_GPIO_PadVal_Set(PAD_SD1_D3, PINMUX_FOR_GPIO_MODE);  // D3
#else
                //OFF:x67 [B8]reg_sd0_mode [B9]reg_sd0_cdz_mode [B13:B12]reg_sd1_mode
                //OFF:x68 [B9:B8]reg_sd1_cdz_mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT12_T | BIT13_T);
#endif

                // Output Low
#if (GPIO_SET == GPIO_SET_BY_FUNC)
                MDrv_GPIO_Set_Low(PAD_SD1_CLK);
                MDrv_GPIO_Set_Low(PAD_SD1_CMD);
                MDrv_GPIO_Set_Low(PAD_SD1_D0);
                MDrv_GPIO_Set_Low(PAD_SD1_D1);
                MDrv_GPIO_Set_Low(PAD_SD1_D2);
                MDrv_GPIO_Set_Low(PAD_SD1_D3);
#else
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
#endif
            }
            else if(ePinPull == EV_PULLUP)
            {
                //D3, D2, D1, D0, CMD=> pull en
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x27), BIT04_T); //D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x28), BIT04_T); //D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2A), BIT04_T); //CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2B), BIT04_T); //D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2C), BIT04_T); //D2

                // Input
#if (GPIO_SET == GPIO_SET_BY_FUNC)
                MDrv_GPIO_Pad_Odn(PAD_SD1_CLK);
                MDrv_GPIO_Pad_Odn(PAD_SD1_CMD);
                MDrv_GPIO_Pad_Odn(PAD_SD1_D0);
                MDrv_GPIO_Pad_Odn(PAD_SD1_D1);
                MDrv_GPIO_Pad_Odn(PAD_SD1_D2);
                MDrv_GPIO_Pad_Odn(PAD_SD1_D3);
#else
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
#endif
                // SD Mode
#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SD1_MODE_1);
#else
                //OFF:x67 [B8]reg_sd0_mode [B9]reg_sd0_cdz_mode [B13:B12]reg_sd1_mode
                //OFF:x68 [B9:B8]reg_sd1_cdz_mode
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT12_T);
#endif
            }
        }
        else if (ePadSel == PAD_SD1_MD2)
        {
            if (ePinPull == EV_PULLDOWN)
            {
                //D3, D2, D1, D0, CMD=> pull dis
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x18), BIT04_T); //D1
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x19), BIT04_T); //D0
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x1A), BIT04_T); //CMD
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x1C), BIT04_T); //D3
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x1D), BIT04_T); //D2

                // PAD -> GPIO mode
#if 0//(PADMUX_SET == PADMUX_SET_BY_FUNC)
                MDrv_GPIO_PadVal_Set(PAD_I2S0_WCK, PINMUX_FOR_GPIO_MODE);  // CLK
                MDrv_GPIO_PadVal_Set(PAD_I2S0_DI, PINMUX_FOR_GPIO_MODE);   // CMD
                MDrv_GPIO_PadVal_Set(PAD_I2S0_BCK, PINMUX_FOR_GPIO_MODE);  // D0
                MDrv_GPIO_PadVal_Set(PAD_I2S0_MCLK, PINMUX_FOR_GPIO_MODE); // D1
                MDrv_GPIO_PadVal_Set(PAD_I2C0_SDA, PINMUX_FOR_GPIO_MODE);  // D2
                MDrv_GPIO_PadVal_Set(PAD_I2C0_SCL, PINMUX_FOR_GPIO_MODE);  // D3
#else
                //OFF:x67 [B8]reg_sd0_mode [B9]reg_sd0_cdz_mode [B13:B12]reg_sd1_mode
                //OFF:x68 [B9:B8]reg_sd1_cdz_mode
                CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT12_T | BIT13_T);
#endif

                // Output Low
#if (GPIO_SET == GPIO_SET_BY_FUNC)
                MDrv_GPIO_Set_Low(PAD_I2S0_WCK);
                MDrv_GPIO_Set_Low(PAD_I2S0_DI);
                MDrv_GPIO_Set_Low(PAD_I2S0_BCK);
                MDrv_GPIO_Set_Low(PAD_I2S0_MCLK);
                MDrv_GPIO_Set_Low(PAD_I2C0_SDA);
                MDrv_GPIO_Set_Low(PAD_I2C0_SCL);
#else
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
#endif
            }
            else if(ePinPull == EV_PULLUP)
            {
                //D3, D2, D1, D0, CMD=> pull en
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x18), BIT04_T); //D1
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x19), BIT04_T); //D0
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x1A), BIT04_T); //CMD
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x1C), BIT04_T); //D3
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x1D), BIT04_T); //D2

                // Input
#if (GPIO_SET == GPIO_SET_BY_FUNC)
                MDrv_GPIO_Pad_Odn(PAD_I2S0_WCK);
                MDrv_GPIO_Pad_Odn(PAD_I2S0_DI);
                MDrv_GPIO_Pad_Odn(PAD_I2S0_BCK);
                MDrv_GPIO_Pad_Odn(PAD_I2S0_MCLK);
                MDrv_GPIO_Pad_Odn(PAD_I2C0_SDA);
                MDrv_GPIO_Pad_Odn(PAD_I2C0_SCL);
#else
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
#endif
                // SD Mode
#if (PADMUX_SET == PADMUX_SET_BY_FUNC)
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SD1_MODE_2);
#else
                //OFF:x67 [B8]reg_sd0_mode [B9]reg_sd0_cdz_mode [B13:B12]reg_sd1_mode
                //OFF:x68 [B9:B8]reg_sd1_cdz_mode
                CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT13_T);
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
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SD), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T); //[5]Boot_Sel [4:2]: Clk_Sel [1]: Clk_i [0]: Clk_g
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SD), BIT05_T); //boot sel
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK,0x25), BIT07_T); //[B3/B7]:[SDIO/SD] select BOOT clock source (glitch free) - 0: select BOOT clock 12MHz (xtali), 1: select FCIE/SPI clock source
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
        CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SDIO), BIT05_T|BIT04_T|BIT03_T|BIT02_T|BIT01_T|BIT00_T); //[5]Boot_Sel [4:2]: Clk_Sel [1]: Clk_i [0]: Clk_g
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_CLKGEN_BANK,REG_CLK_IP_SDIO), BIT05_T); //boot sel
        CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_SC_GP_CTRL_BANK,0x25), BIT07_T | BIT03_T);  //[B3/B7]:[SDIO/SD] select BOOT clock source (glitch free) - 0: select BOOT clock 12MHz (xtali), 1: select FCIE/SPI clock source

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
            //Before I6E: sd card power pin is PAD_FUART_RTS
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x25), BIT02_T);   // reg_fuart_gpio_oen_3
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x25), BIT01_T);   // reg_fuart_gpio_out_3
            break;

        case PAD_SD0_GPIO0:
            //I6E_ECO: sd card power enable. (Change fuart_gpio to sd0_gpio0)
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x10), BIT02_T);   // reg_sd0_gpio_oen_0
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x10), BIT01_T);   // reg_sd0_gpio_out_0
            break;

        case PAD_SD1_GPIO0:
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2D), BIT02_T);   // reg_sd1_gpio_oen_7
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2D), BIT01_T);   // reg_sd1_gpio_out_7
            break;

        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            break;
    }

#endif

    Hal_Timer_mSleep(u16DelayMs);
    //Hal_Timer_mDelay(u16DelayMs);
}

void Hal_CARD_DumpPadMux(PadOrder ePAD)
{
    PadSelect ePadSel = (PadSelect)ePAD;

    if (ePadSel == PAD_SD1)
    {
        printk("reg_allpad_in; reg[101EA1]#7=0b\n");
        printk("%8X\n", CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x50)));
        printk("reg_test_in_mode; reg[101E24]#1 ~ #0=0b\n");
        printk("reg_test_out_mode; reg[101E24]#5 ~ #4=0b\n");
        printk("%8X\n", CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x12)));
        printk("reg_spi0_mode; reg[101E18]#2 ~ #0=0b\n");
        printk("%8X\n", CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0C)));
        printk("reg_fuart_mode; reg[101E06]#2 ~ #0=0b !=4,!=6\n");
        printk("%8X\n", CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x03)));
        printk("reg_sdio_mode; reg[101E11]#0=0b ==0x100\n");
        printk("%8X\n", CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x08)));
        printk("reg_pwm0_mode; reg[101E0E]#2 ~ #0=0b\n");
        printk("reg_pwm2_mode; reg[101E0F]#0 ~ #-2=0b\n");
        printk("%8X\n", CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x07)));
        printk("reg_i2s_mode; reg[101E1F]#3 ~ #2=0b\n");
        printk("reg_ttl_mode; reg[101E1E]#7 ~ #6=0b\n");
        printk("%8X\n", CARD_REG(GET_CARD_REG_ADDR(A_CHIPTOP_BANK, 0x0F)));
    }
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
            //Before I6E: sd card power pin is PAD_FUART_RTS
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x25), BIT02_T);   // reg_fuart_gpio_oen_3
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x25), BIT01_T);   // reg_fuart_gpio_out_3
            break;

        case PAD_SD0_GPIO0:
            //I6E_ECO: sd card power enable. (Change fuart_gpio to sd0_gpio0)
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x10), BIT02_T);   // reg_sd0_gpio_oen_0
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x10), BIT01_T);   // reg_sd0_gpio_out_0
            break;

        case PAD_SD1_GPIO0:
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2D), BIT02_T);   // reg_sd1_gpio_oen_7
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x2D), BIT01_T);   // reg_sd1_gpio_out_7
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
        switch (nPadNo)
        {
            case PAD_SD0_CDZ:
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SD0_CDZ_MODE);
                break;

            case PAD_SD1_CDZ:
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SD1_CDZ_MODE_1);
                break;

            case PAD_I2C0_SDA:
                MDrv_GPIO_PadGroupMode_Set(PINMUX_FOR_SD1_CDZ_MODE_2);
                break;

            default:
                pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
                return;
                break;
        }
    }

#else

    switch (nPadNo)
    {
        case PAD_SD0_CDZ:
            //OFF:x67 [B8]reg_sd0_mode [B9]reg_sd0_cdz_mode [B13:B12]reg_sd1_mode
            //OFF:x68 [B9:B8]reg_sd1_cdz_mode
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x67), BIT09_T);
            break;

        case PAD_SD1_CDZ:
            //OFF:x67 [B8]reg_sd0_mode [B9]reg_sd0_cdz_mode [B13:B12]reg_sd1_mode
            //OFF:x68 [B9:B8]reg_sd1_cdz_mode
            CARD_REG_CLRBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT08_T | BIT09_T);
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x68), BIT08_T);
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
        case PAD_SD0_CDZ:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x11), BIT02_T);   // PAD_SD0_CDZ:reg_sd0_gpio_oen_1
            break;

        case PAD_SD1_CDZ:
            CARD_REG_SETBIT(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x26), BIT02_T);   // PAD_SD1_CDZ:reg_sd1_gpio_oen_0
            break;

        default:
            pr_sd_err("sdmmc error ! [%s][%d]\n", __FUNCTION__, __LINE__);
            return;
            break;
    }

#endif

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
        case PAD_SD0_CDZ:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x11)) & BIT00_T;
            break;

        case PAD_SD1_CDZ:
            nLv = CARD_REG(GET_CARD_REG_ADDR(A_PADTOP_BANK, 0x26)) & BIT00_T;
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
