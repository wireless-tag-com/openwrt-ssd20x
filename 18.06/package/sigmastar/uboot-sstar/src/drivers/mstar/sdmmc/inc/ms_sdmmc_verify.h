/*
* ms_sdmmc_verify.h- Sigmastar
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
 * FileName ms_sdmmc_verify.h
 *     @author jeremy.wang (2013/07/26)
 * Desc:
 * 	   This file is the header file of ms_sdmmc_verify.c.
 *
 ***************************************************************************************************************/


#ifndef __MS_SDMMC_VERIFY_H
#define __MS_SDMMC_VERIFY_H

#include "hal_card_base.h"


//###########################################################################################################
#if (D_PROJECT == D_PROJECT__C3)    //For Cedric
//###########################################################################################################

#define A_DMA_W_BASE            0x41000000
#define A_DMA_R_BASE            0x41000000   //Not Support
#define A_ADMA_BASE             0x0          //Not Support

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__C4)    //For Chicago
//###########################################################################################################

#define A_DMA_W_BASE            0x24000000
#define A_DMA_R_BASE            0x24000000
#define A_ADMA_BASE             0x0          //Not Support


//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF)   //For iNfinity
//###########################################################################################################

#define A_DMA_W_BASE            0x20006000
#define A_DMA_R_BASE            0x20008000
#define A_ADMA_BASE             0x2000A000   //Not Support


//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__iNF3)||(D_PROJECT == D_PROJECT__iNF5)||(D_PROJECT == D_PROJECT__iNF6)||(D_PROJECT == D_PROJECT__iNF2m)||(D_PROJECT == D_PROJECT__iNF6E)||(D_PROJECT == D_PROJECT__iNF6B0)
//###########################################################################################################

#define A_DMA_W_BASE            0x20006000
#define A_DMA_R_BASE            0x20008000
#define A_ADMA_BASE             0x2000A000   //Not Support

//###########################################################################################################
#else    //Templete Description
//###########################################################################################################

#define A_DMA_W_BASE            0x0
#define A_DMA_R_BASE            0x0
#define A_ADMA_BASE             0x0

//###########################################################################################################
#endif
//###########################################################################################################
//


//***********************************************************************************************************

void IPV_SDMMC_TimerTest(U8_T u8Sec);
void IPV_SDMMC_CardDetect(U8_T u8Slot);
void IPV_SDMMC_Init(U8_T u8Slot);
void IPV_SDMMC_SetWideBus(U8_T u8Slot);
void IPV_SDMMC_SetHighBus(U8_T u8Slot);
void IPV_SDMMC_SetClock(U8_T u8Slot, U32_T u32ReffClk, U8_T u8DownLevel);
void IPV_SDMMC_SetBusTiming(U8_T u8Slot, U8_T u8BusTiming);
void IPV_SDMMC_CIFD_RW(U8_T u8Slot, U32_T u32SecAddr, BOOL_T bHidden);
void IPV_SDMMC_DMA_RW(U8_T u8Slot, U32_T u32SecAddr, U16_T u16SecCount, BOOL_T bHidden);

//###########################################################################################################
#if (D_FCIE_M_VER == D_FCIE_M_VER__05)
//###########################################################################################################
void IPV_SDMMC_ADMA_RW(U8_T u8Slot, U32_T u32SecAddr, BOOL_T bHidden);
//###########################################################################################################
#endif

void IPV_SDMMC_BurnRW(U8_T u8Slot, U8_T u8TransType, U32_T u32StartSec, U32_T u32EndSec);
void IPV_SDMMC_TestPattern(U8_T u8Slot, U8_T u8Pattern);
void IPV_SDMMC_SetPAD(U8_T u8Slot, U8_T u8IP, U8_T u8Port, U8_T u8PAD);
void IPV_SDMMC_PowerSavingModeVerify(U8_T u8Slot);


#endif // End of __MS_SDMMC_VERIFY_H
