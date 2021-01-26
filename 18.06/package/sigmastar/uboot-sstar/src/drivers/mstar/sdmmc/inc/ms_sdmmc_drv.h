/*
* ms_sdmmc_drv.h- Sigmastar
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
 * FileName ms_sdmmc_drv.h
 *     @author jeremy.wang (2013/07/26)
 * Desc:
 *     This file is the header file of ms_sdmmc_drv.c.
 *
 ***************************************************************************************************************/
#ifndef __MS_SDMMC_DRV_H
#define __MS_SDMMC_DRV_H

#include "hal_card_base.h"

//***********************************************************************************************************
// Config Setting (Externel)
//***********************************************************************************************************

//###########################################################################################################
#if (D_PROJECT == D_PROJECT__C3)    //For Cedric
//###########################################################################################################
    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PORT_SDIO1                  //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define V_SDMMC1_MAX_CLK           32000000

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //SDIO0
    #define D_SDMMC2_PORT              EV_PORT_SDIO1                  //Port Setting
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define V_SDMMC2_MAX_CLK           32000000

    #define D_SDMMC3_IP                EV_IP_FCIE3                    //SDIO0
    #define D_SDMMC3_PORT              EV_PORT_SDIO1                  //Port Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //PAD_SD1

    #define V_SDMMC3_MAX_CLK           32000000

    #define EN_SDMMC_CDZREV            (FALSE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                25 //(ms)


//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__C4)    //For Chicago
//###########################################################################################################
    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PORT_SDIO1                  //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1

    #define V_SDMMC1_MAX_CLK           32000000


    #define D_SDMMC2_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC2_PORT              EV_PORT_SDIO1                  //Port Setting
    #define D_SDMMC2_PAD               EV_PAD1

    #define V_SDMMC2_MAX_CLK           32000000

    #define D_SDMMC3_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC3_PORT              EV_PORT_SDIO1                  //Port Setting
    #define D_SDMMC3_PAD               EV_PAD1

    #define V_SDMMC3_MAX_CLK           32000000

    #define EN_SDMMC_CDZREV            (FALSE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                25 //(ms)


//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF)
//###########################################################################################################

    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //FCIE
    #define D_SDMMC2_PORT              EV_PFCIE5_FCIE                 //Port Setting
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE3                    //Dummy Setting
    #define D_SDMMC3_PORT              EV_PFCIE5_FCIE                 //Dummy Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //Dummy Setting

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           48000000
    #define V_SDMMC2_MAX_CLK           48000000
    #define V_SDMMC3_MAX_CLK           48000000                       //Dummy Setting

    #define EN_SDMMC_CDZREV            (FALSE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                25 //(ms)


//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF3)
//###########################################################################################################

    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //FCIE
    #define D_SDMMC2_PORT              EV_PFCIE5_FCIE                 //Port Setting
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE3                    //Dummy Setting
    #define D_SDMMC3_PORT              EV_PFCIE5_FCIE                 //Dummy Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //Dummy Setting

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           48000000
    #define V_SDMMC2_MAX_CLK           48000000
    #define V_SDMMC3_MAX_CLK           48000000                       //Dummy Setting

    #define EN_SDMMC_CDZREV            (FALSE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                25 //(ms)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF5)
//###########################################################################################################

    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //FCIE
    #define D_SDMMC2_PORT              EV_PFCIE5_FCIE                 //Port Setting
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE3                    //Dummy Setting
    #define D_SDMMC3_PORT              EV_PFCIE5_FCIE                 //Dummy Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //Dummy Setting

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           48000000
    #define V_SDMMC2_MAX_CLK           48000000
    #define V_SDMMC3_MAX_CLK           48000000                       //Dummy Setting

    #define EN_SDMMC_CDZREV            (FALSE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                25 //(ms)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF6)
//###########################################################################################################

    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //FCIE
    #define D_SDMMC2_PORT              EV_PFCIE5_SDIO                 //Port Setting
//    #define D_SDMMC2_PORT              EV_PFCIE5_FCIE                 //Port Setting
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE3                    //Dummy Setting
    #define D_SDMMC3_PORT              EV_PFCIE5_FCIE                 //Dummy Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //Dummy Setting

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           48000000
    #define V_SDMMC2_MAX_CLK           48000000
    #define V_SDMMC3_MAX_CLK           48000000                       //Dummy Setting

    #define EN_SDMMC_CDZREV            (FALSE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                25 //(ms)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF2m)
//###########################################################################################################

    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //FCIE
    #define D_SDMMC2_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE3                    //Dummy Setting
    #define D_SDMMC3_PORT              EV_PFCIE5_FCIE                 //Dummy Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //Dummy Setting

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           48000000
    #define V_SDMMC2_MAX_CLK           48000000
    #define V_SDMMC3_MAX_CLK           48000000                       //Dummy Setting

    #define EN_SDMMC_CDZREV            (FALSE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                25 //(ms)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF6E)
    //###########################################################################################################

    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //FCIE
    #define D_SDMMC2_PORT              EV_PFCIE5_SDIO                 //Port Setting
    //    #define D_SDMMC2_PORT              EV_PFCIE5_FCIE                 //Port Setting
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE3                    //Dummy Setting
    #define D_SDMMC3_PORT              EV_PFCIE5_FCIE                 //Dummy Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //Dummy Setting

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           48000000
    #define V_SDMMC2_MAX_CLK           48000000
    #define V_SDMMC3_MAX_CLK           48000000                       //Dummy Setting

    #define EN_SDMMC_CDZREV            (TRUE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                25 //(ms)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF6B0)
//###########################################################################################################

    #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PFCIE5_SDIO                 //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE2                    //FCIE
    #define D_SDMMC2_PORT              EV_PFCIE5_SDIO                 //Port Setting
//    #define D_SDMMC2_PORT              EV_PFCIE5_FCIE                 //Port Setting
    #define D_SDMMC2_PAD               EV_PAD2                        //PAD_SD1

    #define D_SDMMC3_IP                EV_IP_FCIE3                    //Dummy Setting
    #define D_SDMMC3_PORT              EV_PFCIE5_FCIE                 //Dummy Setting
    #define D_SDMMC3_PAD               EV_PAD3                        //Dummy Setting

    #define V_SDMMC_CARDNUMS           1
    #define V_SDMMC1_MAX_CLK           48000000
    #define V_SDMMC2_MAX_CLK           48000000
    #define V_SDMMC3_MAX_CLK           48000000                       //Dummy Setting

    #define EN_SDMMC_CDZREV            (FALSE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                25 //(ms)

//###########################################################################################################
#else    //Template Description
//###########################################################################################################
/*  #define D_SDMMC1_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC1_PORT              EV_PORT_SDIO1                  //Port Setting
    #define D_SDMMC1_PAD               EV_PAD1                        //PAD_SD0

    #define D_SDMMC2_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC2_PORT              EV_PORT_SDIO1                  //Port Setting
    #define D_SDMMC2_PAD               EV_PAD1

    #define D_SDMMC3_IP                EV_IP_FCIE1                    //SDIO0
    #define D_SDMMC3_PORT              EV_PORT_SDIO1                  //Port Setting
    #define D_SDMMC3_PAD               EV_PAD1

    #define EN_SDMMC_CDZREV            (FALSE)

    #define WT_POWERUP                 20 //(ms)
    #define WT_POWERON                 60 //(ms)
    #define WT_POWEROFF                25 //(ms)

*/
//###########################################################################################################
#endif
//###########################################################################################################

//============================================
// OCR Register
//============================================
#define R_OCR_LOW    BIT07_T
#define R_OCR_27_28  BIT15_T
#define R_OCR_28_29  BIT16_T
#define R_OCR_29_30  BIT17_T
#define R_OCR_30_31  BIT18_T
#define R_OCR_31_32  BIT19_T
#define R_OCR_32_33  BIT20_T
#define R_OCR_33_34  BIT21_T
#define R_OCR_34_35  BIT22_T
#define R_OCR_35_36  BIT23_T
#define R_OCR_S18    BIT24_T
#define R_OCR_CCS    BIT30_T
#define R_OCR_READY  BIT31_T

#define SD_OCR_RANGE    (R_OCR_27_28| R_OCR_28_29| R_OCR_29_30 | R_OCR_30_31 | R_OCR_31_32 | R_OCR_32_33)

#define CLK_DEF_SPEED       25000000
#define CLK_HIGH_SPEED      50000000
#define CLK_SDR104_SPEED    208000000
#define CLK_SDR50_SPEED     100000000
#define CLK_DDR50_SPEED     CLK_HIGH_SPEED
#define CLK_SDR25_SPEED     CLK_HIGH_SPEED
#define CLK_SDR12_SPEED     CLK_DEF_SPEED

#define DEF_SPEED_BUS_SPEED         0
#define UHS_SDR12_BUS_SPEED         0
#define HIGH_SPEED_BUS_SPEED        1
#define UHS_SDR25_BUS_SPEED         1
#define UHS_SDR50_BUS_SPEED         2
#define UHS_SDR104_BUS_SPEED        3
#define UHS_DDR50_BUS_SPEED         4

#define SD_MODE_HIGH_SPEED  (1 << HIGH_SPEED_BUS_SPEED)
#define SD_MODE_UHS_SDR12   (1 << UHS_SDR12_BUS_SPEED)
#define SD_MODE_UHS_SDR25   (1 << UHS_SDR25_BUS_SPEED)
#define SD_MODE_UHS_SDR50   (1 << UHS_SDR50_BUS_SPEED)
#define SD_MODE_UHS_SDR104  (1 << UHS_SDR104_BUS_SPEED)
#define SD_MODE_UHS_DDR50   (1 << UHS_DDR50_BUS_SPEED)

#define UHS_B_DRV_TYPE          0
#define UHS_A_DRV_TYPE          1
#define UHS_C_DRV_TYPE          2
#define UHS_D_DRV_TYPE          3

#define SD_DRIVER_TYPE_B    (1 << UHS_B_DRV_TYPE)
#define SD_DRIVER_TYPE_A    (1 << UHS_A_DRV_TYPE)
#define SD_DRIVER_TYPE_C    (1 << UHS_C_DRV_TYPE)
#define SD_DRIVER_TYPE_D    (1 << UHS_D_DRV_TYPE)

#define UHS_200_CURR_LMT        0
#define UHS_400_CURR_LMT        1
#define UHS_600_CURR_LMT        2
#define UHS_800_CURR_LMT        3

#define SD_CURR_LMT_200     (1 << UHS_200_CURR_LMT)
#define SD_CURR_LMT_400     (1 << UHS_400_CURR_LMT)
#define SD_CURR_LMT_600     (1 << UHS_600_CURR_LMT)
#define SD_CURR_LMT_800     (1 << UHS_800_CURR_LMT)


typedef enum
{
    EV_NOCARD   =0,     // Unknow memory card
    EV_MMC      =1,     // MMC card
    EV_SDIO     =5,     // SDIO
    EV_SD       =41,    // SD card

} CARDTypeEmType;


typedef enum
{
    EV_SCS  =0,
    EV_HCS  =R_OCR_CCS,
} SDCAPEmType;


typedef struct
{
    U8_T MID;
    U8_T OID[2];
    U8_T PNM[6];
    U8_T PRV;
    U32_T PSN;

} CARDCIDStruct;


typedef struct
{
    U8_T CSDSTR;
    U8_T SPECVERS;
    U32_T TAAC_NS;
    U8_T NSAC;
    U8_T R2W_FACTOR;
    U32_T TRAN_KB;
    U16_T CCC;
    U8_T R_BLK_SIZE;
    U8_T W_BLK_SIZE;
    U8_T W_BLK_MISALIGN;
    U8_T R_BLK_MISALIGN;
    U8_T PERM_W_PROTECT;
    U8_T TEMP_W_PROTECT;
    U32_T CAPCITY;

} CARDCSDStruct;


typedef struct
{
    U32_T   u32RCAArg;          // Relative Card Address
    U32_T   u32OCR;
    U32_T   u32CardStatus;
    U32_T   u32MaxClk;
    U8_T    u8SpecVer;
    U8_T    u8SpecVer1;
    U8_T    u8BusWidth;
    U8_T    u8AccessMode;
    U8_T    u8DrvStrength;
    U8_T    u8CurrMax;
    U8_T    u8SD3BusMode;
    U8_T    u8SD3DrvType;
    U8_T    u8SD3CurrLimit;
    SDCAPEmType eHCS;
    CARDCIDStruct stCID;
    CARDCSDStruct stCSD;
    CARDTypeEmType eCardType;

} SDMMCInfoStruct;


typedef enum
{
    EV_POWER_OFF = 0,
    EV_POWER_ON  = 1,
    EV_POWER_UP  = 2,

} PowerEmType;


void SDMMC_SwitchPAD(U8_T u8Slot);
void SDMMC_SetPAD(U8_T u8Slot, IPEmType eIP, PortEmType ePort, PADEmType ePAD);

void SDMMC_SetPower(U8_T u8Slot, PowerEmType ePower);
U32_T SDMMC_SetClock(U8_T u8Slot, U32_T u32ReffClk, U8_T u8DownLevel);
void SDMMC_SetBusTiming(U8_T u8Slot, BusTimingEmType eBusTiming);

BOOL_T SDMMC_CardDetect(U8_T u8Slot);

U16_T SDMMC_SetWideBus(U8_T u8Slot);
U16_T SDMMC_SwitchHighBus(U8_T u8Slot);
U16_T SDMMC_Init(U8_T u8Slot);

U16_T SDMMC_CIF_BLK_R(U8_T u8Slot, U32_T u32CardAddr, volatile U8_T *pu8DataBuf);
U16_T SDMMC_CIF_BLK_W(U8_T u8Slot, U32_T u32CardAddr, volatile U8_T *pu8DataBuf);
U16_T SDMMC_DMA_BLK_R(U8_T u8Slot, U32_T u32CardAddr, U16_T u16BlkCnt, volatile U8_T *pu8DataBuf);
U16_T SDMMC_DMA_BLK_W(U8_T u8Slot, U32_T u32CardAddr, U16_T u16BlkCnt, volatile U8_T *pu8DataBuf);


//###########################################################################################################
#if (D_FCIE_M_VER == D_FCIE_M_VER__05)
//###########################################################################################################
U16_T SDMMC_ADMA_BLK_R(U8_T u8Slot, U32_T u32CardAddr, U32_T *pu32ArrDAddr, U16_T *pu16ArrBlkCnt, U16_T u16ItemCnt, volatile void *pDMATable);
U16_T SDMMC_ADMA_BLK_W(U8_T u8Slot, U32_T u32CardAddr, U32_T *pu32ArrDAddr, U16_T *pu16ArrBlkCnt, U16_T u16ItemCnt, volatile void *pDMATable);
//###########################################################################################################
#endif



#endif // End of __MS_SDMMC_DRV_H
