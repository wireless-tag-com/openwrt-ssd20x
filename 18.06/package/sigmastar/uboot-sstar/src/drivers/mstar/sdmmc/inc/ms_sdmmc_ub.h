/*
* ms_sdmmc_ub.h- Sigmastar
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
 * FileName ms_sdmmc_ub.h
 *     @author jeremy.wang (2013/07/11)
 * Desc:
 *     This file is the header file of ms_sdmmc_ub.c.
 *
 ***************************************************************************************************************/


#ifndef __MS_SDMMC_UB_H
#define __MS_SDMMC_UB_H


//***********************************************************************************************************
// Config Setting (Externel)
//***********************************************************************************************************

//###########################################################################################################
#if (D_PROJECT == D_PROJECT__C3)    //For Cedric
//###########################################################################################################
    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PORT_SDIO1                  //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //SDIO1
    #define D_SDMMC2_PORT              EV_PORT_SDIO1
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE3                    //SDIO0
    #define D_SDMMC3_PORT              EV_PORT_SDIO1                  //Port Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //PAD_SD0

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           32000000
    #define V_SDMMC2_MAX_CLK           32000000
    #define V_SDMMC3_MAX_CLK           32000000

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                25 //(ms)


//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__C4)    //For Chicago
//###########################################################################################################
    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PORT_SDIO1                  //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE1                    //Dummy Setting
    #define D_SDMMC2_PORT              EV_PORT_SDIO1                  //Dummy Setting
    #define D_SDMMC2_PAD               EV_PAD1                        //Dummy Setting

    #define D_SDMMC3_IP                EV_IP_FCIE1                    //Dummy Setting
    #define D_SDMMC3_PORT              EV_PORT_SDIO1                  //Dummy Setting
    #define D_SDMMC3_PAD               EV_PAD1                        //Dummy Setting

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           32000000
    #define V_SDMMC2_MAX_CLK           32000000                       //Dummy Setting
    #define V_SDMMC3_MAX_CLK           32000000                       //Dummy Setting

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                25 //(ms)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF)   //For iNfinity
//###########################################################################################################

    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //FCIE
    #define D_SDMMC2_PORT              EV_PFCIE5_FCIE                 //Port Setting
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE2                    //Dummy Setting
    #define D_SDMMC3_PORT              EV_PFCIE5_FCIE                 //Dummy Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //Dummy Setting

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           32000000
    #define V_SDMMC2_MAX_CLK           32000000
    #define V_SDMMC3_MAX_CLK           32000000                       //Dummy Setting

    #define EN_SDMMC_CDZREV            (FALSE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                80 //(ms)


//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF3)   //For iNfinity
//###########################################################################################################

    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //FCIE
    #define D_SDMMC2_PORT              EV_PFCIE5_FCIE                 //Port Setting
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE2                    //Dummy Setting
    #define D_SDMMC3_PORT              EV_PFCIE5_FCIE                 //Dummy Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //Dummy Setting

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           32000000
    #define V_SDMMC2_MAX_CLK           32000000
    #define V_SDMMC3_MAX_CLK           32000000                       //Dummy Setting

    #define EN_SDMMC_CDZREV            (FALSE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                80 //(ms)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF5)   //For iNfinity5
//###########################################################################################################

    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //FCIE
    #define D_SDMMC2_PORT              EV_PFCIE5_FCIE                 //Port Setting
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE2                    //Dummy Setting
    #define D_SDMMC3_PORT              EV_PFCIE5_FCIE                 //Dummy Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //Dummy Setting

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           32000000
    #define V_SDMMC2_MAX_CLK           32000000
    #define V_SDMMC3_MAX_CLK           32000000                       //Dummy Setting

    #define EN_SDMMC_CDZREV            (FALSE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                80 //(ms)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF6)   //For iNfinity6
//###########################################################################################################

    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //FCIE
    #define D_SDMMC2_PORT              EV_PFCIE5_FCIE                 //Port Setting
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE2                    //Dummy Setting
    #define D_SDMMC3_PORT              EV_PFCIE5_FCIE                 //Dummy Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //Dummy Setting

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           32000000
    #define V_SDMMC2_MAX_CLK           32000000
    #define V_SDMMC3_MAX_CLK           32000000                       //Dummy Setting

    #define EN_SDMMC_CDZREV            (FALSE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                80 //(ms)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF6E)   //For iNfinity6e
//###########################################################################################################

    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //FCIE
    #define D_SDMMC2_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE2                    //Dummy Setting
    #define D_SDMMC3_PORT              EV_PFCIE5_SDIO                 //Dummy Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //Dummy Setting

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           32000000
    #define V_SDMMC2_MAX_CLK           32000000
    #define V_SDMMC3_MAX_CLK           32000000                       //Dummy Setting

    #define EN_SDMMC_CDZREV            (TRUE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                80 //(ms)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF6B0)   //For iNfinity6b0
//###########################################################################################################

    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //FCIE
    #define D_SDMMC2_PORT              EV_PFCIE5_FCIE                 //Port Setting
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE2                    //Dummy Setting
    #define D_SDMMC3_PORT              EV_PFCIE5_FCIE                 //Dummy Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //Dummy Setting

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           32000000
    #define V_SDMMC2_MAX_CLK           32000000
    #define V_SDMMC3_MAX_CLK           32000000                       //Dummy Setting

    #define EN_SDMMC_CDZREV            (FALSE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                80 //(ms)

//###########################################################################################################
#else    //Templete Description
//###########################################################################################################
/*
    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PORT_SDIO1                  //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //SDIO1
    #define D_SDMMC2_PORT              EV_PORT_SDIO1
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE3                    //SDIO0
    #define D_SDMMC3_PORT              EV_PORT_SDIO1                  //Port Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //PAD_SD0

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           32000000
    #define V_SDMMC2_MAX_CLK           32000000
    #define V_SDMMC3_MAX_CLK           32000000

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                25 //(ms)

*/

//###########################################################################################################
#endif
//###########################################################################################################

//***********************************************************************************************************
typedef enum
{
    EV_SDMMC1 = 0,
    EV_SDMMC2 = 1,
    EV_SDMMC3 = 2,

} SlotEmType;

typedef enum
{
    EV_POWER_OFF = 0,
    EV_POWER_ON  = 1,
    EV_POWER_UP  = 2,

} PowerEmType;



#endif // End of __MS_SDMMC_UB_H
