/*
* hal_sdmmc_v5.h- Sigmastar
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
#ifndef __HAL_SDMMC_V5_H
#define __HAL_SDMMC_V5_H

#include "hal_card_regs.h"
//***********************************************************************************************************
// Config Setting (Externel)
//***********************************************************************************************************
#include "hal_card_platform_config.h"

//###########################################################################################################
#if (D_OS == D_OS__LINUX)   //For LInux
//###########################################################################################################
#define EN_BIND_CARD_INT            (TRUE)
//###########################################################################################################
#else
//###########################################################################################################
#define EN_BIND_CARD_INT            (FALSE)
//###########################################################################################################
#endif



typedef enum
{
    //SD_STS Reg Error
    EV_STS_OK               = 0x0000,
    EV_STS_RD_CERR          = BIT00_T,
    EV_STS_WD_CERR          = BIT01_T,
    EV_STS_WR_TOUT          = BIT02_T,
    EV_STS_NORSP            = BIT03_T,
    EV_STS_RSP_CERR         = BIT04_T,
    EV_STS_RD_TOUT          = BIT05_T,

    //SD IP Error
    EV_STS_RIU_ERR          = BIT06_T,
    EV_STS_DAT0_BUSY        = BIT07_T,
    EV_STS_MIE_TOUT         = BIT08_T,

    //Stop Wait Process Error
    EV_SWPROC_ERR           = BIT09_T,

    //SD Check Error
    EV_CMD8_PERR            = BIT10_T,
    EV_OCR_BERR             = BIT11_T,
    EV_OUT_VOL_RANGE        = BIT12_T,
    EV_STATE_ERR            = BIT13_T,

    //Other Error
    EV_OTHER_ERR            = BIT15_T,


} RspErrEmType;

typedef enum
{
    EV_CMDRSP   =0x000,
    EV_CMDREAD  =0x001,
    EV_CMDWRITE =0x101,

} CmdEmType;

typedef enum
{
    EV_EMP  = 0x0000,
    EV_ADMA = 0x0020,  //Add at FCIE5
    EV_DMA  = 0x0080,  //Change at FCIE5
    EV_CIF  = 0x1000,  //Change at FCIE5

} TransEmType;

//(2bits: Rsp Mapping to SD_CTL) (4bits: Identity) (8bits: RspSize)
typedef enum
{
    EV_NO   = 0x0000,   //No response type
    EV_R1   = 0x2105,
    EV_R1B  = 0x2205,
    EV_R2   = 0x3310,
    EV_R3   = 0x2405,
    EV_R4   = 0x2505,
    EV_R5   = 0x2605,
    EV_R6   = 0x2705,
    EV_R7   = 0x2805,

} SDMMCRspEmType;

typedef enum
{
    EV_BUS_1BIT  = 0x00,
    EV_BUS_4BITS = 0x02,
    EV_BUS_8BITS = 0x04,

} SDMMCBusWidthEmType;

typedef enum
{
    EV_MIE  = 0x0,
    EV_CIFD = 0x1,

} IPEventEmType;


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
    U32_T u32Arg;              //Mark for ROM
    U32_T u32ErrLine;          //Mark for ROM
    RspErrEmType eErrCode;
    U8_T u8RspSize;            //Mark for ROM
    U8_T u8ArrRspToken[0x10];    //U8_T u8ArrRspToken[0x10];  //Mark for ROM

} RspStruct;


typedef struct
{
    U32_T u32_End     : 1;
    U32_T u32_MiuSel  : 2;
    U32_T             : 13;
    U32_T u32_JobCnt  : 16;
    U32_T u32_Address;
    U32_T u32_DmaLen;
    U32_T u32_Dummy;

} AdmaDescStruct;


//  SDMMC Operation Function
//----------------------------------------------------------------------------------------------------------
void Hal_SDMMC_SetDataWidth(IpOrder eIP, SDMMCBusWidthEmType eBusWidth);
void Hal_SDMMC_SetBusTiming(IpOrder eIP, BusTimingEmType eBusTiming);
void Hal_SDMMC_SetNrcDelay(IpOrder eIP, U32_T u32RealClk);

void Hal_SDMMC_SetCmdToken(IpOrder eIP, U8_T u8Cmd, U32_T u32Arg);
RspStruct* Hal_SDMMC_GetRspToken(IpOrder eIP);
void Hal_SDMMC_TransCmdSetting(IpOrder eIP, TransEmType eTransType, U16_T u16BlkCnt, U16_T u16BlkSize, volatile U32_T u32BufAddr, volatile U8_T *pu8Buf);
RspErrEmType Hal_SDMMC_SendCmdAndWaitProcess(IpOrder eIP, TransEmType eTransType, CmdEmType eCmdType, SDMMCRspEmType eRspType, BOOL_T bCloseClk);
RspErrEmType Hal_SDMMC_RunBrokenDmaAndWaitProcess(IpOrder eIP, CmdEmType eCmdType);
void Hal_SDMMC_ADMASetting(volatile void *pDMATable, U8_T u8Item, U32_T u32SubLen, U16_T u16SubBCnt, U32_T u32SubAddr, U8_T u8MIUSel, BOOL_T bEnd);

//  SDMMC Special Operation Function
//----------------------------------------------------------------------------------------------------------
void Hal_SDMMC_ClkCtrl(IpOrder eIP, BOOL_T bEnable, U16_T u16DelayMs);
void Hal_SDMMC_Reset(IpOrder eIP);
void Hal_SDMMC_StopProcessCtrl(IpOrder eIP, BOOL_T bEnable);
BOOL_T Hal_SDMMC_OtherPreUse(IpOrder eIP);
ErrGrpEmType Hal_SDMMC_ErrGroup(RspErrEmType eErrType);

// SDMMC Information
//----------------------------------------------------------------------------------------------------------
void Hal_SDMMC_DumpMemTool(U8_T u8ListNum, volatile U8_T *pu8Buf);
U8_T Hal_SDMMC_GetDATBusLevel(IpOrder eIP);
U16_T Hal_SDMMC_GetMIEEvent(IpOrder eIP);

// SDMMC SDIO Setting
//----------------------------------------------------------------------------------------------------------
void Hal_SDMMC_SDIODeviceCtrl(IpOrder eIP, BOOL_T bEnable);
void Hal_SDMMC_SDIOIntDetCtrl(IpOrder eIP, BOOL_T bEnable);
void Hal_SDMMC_SetSDIOIntBeginSetting(IpOrder eIP, U8_T u8Cmd, U32_T u32Arg, CmdEmType eCmdType, U16_T u16BlkCnt);
void Hal_SDMMC_SetSDIOIntEndSetting(IpOrder eIP, RspErrEmType eRspErr, U16_T u16BlkCnt);

// SDMMC Interrupt Setting
//----------------------------------------------------------------------------------------------------------
#if(EN_BIND_CARD_INT)

void Hal_SDMMC_MIEIntCtrl(IpOrder eIP, BOOL_T bEnable);


#endif //End of EN_BIND_CARD_INT



#endif //End of __HAL_SDMMC_V5_H
