/*
* hal_sdmmc.h- Sigmastar
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
 * FileName hal_sdmmc.h
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 * 	   This file is the header file of hal_sdmmc.c.
 *
 ***************************************************************************************************************/

#ifndef __HAL_SDMMC_H
#define __HAL_SDMMC_H

#include "hal_card_regs.h"

//***********************************************************************************************************
// Config Setting (Externel)
//***********************************************************************************************************

//###########################################################################################################
#if (D_PROJECT == D_PROJECT__C3)    //For Cedric
//###########################################################################################################

#define EN_MIU_WDEN_PATCH_DIS       (FALSE)

#define V_MIUCLK_CTRL_INIT          (R_MIU_CLK_EN_HW)  //HW
#define V_MIUBUS_CTRL_INIT          0 				   //Disable MIU Bus Burst Ctrl

#define WT_EVENT_RSP                10		//(ms)
#define WT_EVENT_READ               2000	//(ms)
#define WT_EVENT_WRITE              3000	//(ms)

//###########################################################################################################
#elif (D_PROJECT == D_PROJECT__C4)   //For Chicago
//###########################################################################################################

#define EN_MIU_WDEN_PATCH_DIS       (FALSE)

#define V_MIUCLK_CTRL_INIT          (R_MIU_CLK_EN_HW)  //HW
#define V_MIUBUS_CTRL_INIT          0                  //Disable MIU Bus Burst Ctrl

#define WT_EVENT_RSP                10		//(ms)
#define WT_EVENT_READ               2000	//(ms)
#define WT_EVENT_WRITE              3000	//(ms)

//###########################################################################################################
#else  //Templete Description
//###########################################################################################################
/*

#define EN_MIU_WDEN_PATCH_DIS       (FALSE)

#define V_MIUCLK_CTRL_INIT          (R_MIU_CLK_EN_SW|R_MIU_CLK_CTL_SW)  //SW
#define V_MIUBUS_CTRL_INIT          (R_MIU_BUS_CTRL_EN)                 //Enable and Burst8 (32Bytes)

#define WT_EVENT_RSP                10		//(ms)
#define WT_EVENT_READ               2000	//(ms)
#define WT_EVENT_WRITE              3000	//(ms)

*/
#endif

//###########################################################################################################
#if (D_OS == D_OS__LINUX)   //For LInux
//###########################################################################################################
#define EN_BIND_CARD_INT            (TRUE)
//###########################################################################################################
#elif (D_OS == D_OS__UBOOT) //For UBoot
//###########################################################################################################
#define EN_BIND_CARD_INT            (FALSE)
//###########################################################################################################
#endif



//***********************************************************************************************************

typedef enum
{
	//SD_STS Reg Error
	EV_STS_OK               = 0x0000,
	EV_STS_DAT_CERR         = BIT00_T,
	EV_STS_DAT_STSERR       = BIT01_T,
	EV_STS_DAT_STSNEG       = BIT02_T,
	EV_STS_NORSP            = BIT03_T,
	EV_STS_RSP_CERR         = BIT04_T,

	//SD IP Error
	EV_STS_RIU_ERR          = BIT05_T,
	EV_STS_MIE_TOUT         = BIT06_T,
	EV_STS_FIFO_NRDY        = BIT07_T,
	EV_STS_DAT0_BUSY        = BIT08_T,

	//Stop Wait Process Error
	EV_SWPROC_ERR           = BIT09_T,

	//SD Driver Check Error
	EV_CMD8_PERR            = BIT10_T,  //Not Use
	EV_OCR_BERR             = BIT11_T,  //Not Use
	EV_OUT_VOL_RANGE        = BIT12_T,  //Not Use
	EV_STATE_ERR            = BIT13_T,  //Not Use

	//Other Error
	EV_OTHER_ERR            = BIT15_T,


} RspErrEmType;

//(5bits: Position Mapping to SD_CTL) (4bits: Identity)
typedef enum
{
	EV_CMDRSP	=0x000,
	EV_CMDREAD	=0x001,
	EV_CMDWRITE	=0x101,

} CmdEmType;

//(6bits: Position Mapping to SD_Mode) (8bits: Position Mapping to MIE_Path)
typedef enum
{
	EV_EMP	= 0x0000,
	EV_DMA	= 0x0001,
	EV_CIF	= 0x2000,

} TransEmType;

//(2bits: Rsp Mapping to SD_CTL) (4bits: Identity) (8bits: RspSize)
typedef enum
{
	EV_NO	= 0x0000,	//No response type
	EV_R1	= 0x2105,
	EV_R1B 	= 0x2205,
	EV_R2	= 0x3310,
	EV_R3	= 0x2405,
	EV_R4	= 0x2505,
	EV_R5	= 0x2605,
	EV_R6	= 0x2705,
	EV_R7	= 0x2805,

} SDMMCRspEmType;


typedef enum
{
	EV_BUS_1BIT  = 0x00,
	EV_BUS_4BITS = 0x02,
	EV_BUS_8BITS = 0x04,

} SDMMCBusWidthEmType;


typedef enum
{
	EV_EGRP_OK       = 0x0,
	EV_EGRP_TOUT     = 0x1,
	EV_EGRP_COMM     = 0x2,
	EV_EGRP_OTHER    = 0x3,

} ErrGrpEmType;


typedef struct
{
	U8_T u8Cmd;
	U32_T u32Arg;
	U32_T u32ErrLine;
	RspErrEmType eErrCode;
	U8_T u8RspSize;
	U8_T u8ArrRspToken[0x10];

} RspStruct;

// SDMMC Function
//----------------------------------------------------------------------------------------------------------
void Hal_SDMMC_SetSDIODevice(IPEmType eIP, BOOL_T bEnable);
void Hal_SDMMC_SetSDIOIntDet(IPEmType eIP, BOOL_T bEnable);

void Hal_SDMMC_SetDataWidth(IPEmType eIP, SDMMCBusWidthEmType eBusWidth);
void Hal_SDMMC_SetBusTiming(IPEmType eIP, BusTimingEmType eBusTiming);
void Hal_SDMMC_SetNrcDelay(IPEmType eIP, U32_T u32RealClk);

void Hal_SDMMC_SetCmdToken(IPEmType eIP, U8_T u8Cmd, U32_T u32Arg);
RspStruct* Hal_SDMMC_GetRspToken(IPEmType eIP);
void Hal_SDMMC_TransCmdSetting(IPEmType eIP, TransEmType eTransType, U16_T u16BlkCnt, U16_T u16BlkSize, volatile U32_T u32BufAddr,  volatile U8_T *pu8Buf);
RspErrEmType Hal_SDMMC_SendCmdAndWaitProcess(IPEmType eIP, TransEmType eTransType, CmdEmType eCmdType, SDMMCRspEmType eRspType, BOOL_T bCloseClk);
RspErrEmType Hal_SDMMC_RunBrokenDmaAndWaitProcess(IPEmType eIP, CmdEmType eCmdType);

ErrGrpEmType Hal_SDMMC_ErrGroup(RspErrEmType eErrType);

void Hal_SDMMC_SetMIEIntEn(IPEmType eIP, TransEmType eTransType);
void Hal_SDMMC_SetSDIOIntBeginSetting(IPEmType eIP, U8_T u8Cmd, U32_T u32Arg, CmdEmType eCmdType, U16_T u16BlkCnt);
void Hal_SDMMC_SetSDIOIntEndSetting(IPEmType eIP, RspErrEmType eRspErr, U16_T u16BlkCnt);

BOOL_T Hal_SDMMC_WriteProtect(IPEmType eIP);
BOOL_T Hal_SDMMC_CardDetect(IPEmType eIP);
BOOL_T Hal_SDMMC_OtherPreUse(IPEmType eIP);

void Hal_SDMMC_ClkCtrl(IPEmType eIP, BOOL_T bOpen, U16_T u16DelayMs);
void Hal_SDMMC_Reset(IPEmType eIP);
void Hal_SDMMC_WaitProcessCtrl(IPEmType eIP, BOOL_T bStop);

U8_T Hal_SDMMC_GetDATBusLevel(IPEmType eIP);
U16_T Hal_SDMMC_GetMIEEvent(IPEmType eIP);



#endif //End of __HAL_SDMMC_H








