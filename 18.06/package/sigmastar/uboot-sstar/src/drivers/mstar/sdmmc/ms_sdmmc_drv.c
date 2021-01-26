/*
* ms_sdmmc_drv.c- Sigmastar
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
 * FileName ms_sdmmc_drv.c
 *     @author jeremy.wang (2013/07/26)
 * Desc:
 * 	   This layer is a simple sdmmc driver for special purpose (IP Verify etc...)
 * 	   (1) The goal is we don't need to change any HAL Driver code, but we can handle here.
 *
 ***************************************************************************************************************/

#include "inc/ms_sdmmc_drv.h"
#include "inc/hal_card_platform.h"
#include "inc/hal_card_timer.h"

//###########################################################################################################
#if (D_FCIE_M_VER == D_FCIE_M_VER__04)
//###########################################################################################################
#include "inc/hal_sdmmc.h"
//###########################################################################################################
#elif (D_FCIE_M_VER == D_FCIE_M_VER__05)
//###########################################################################################################
#include "inc/hal_sdmmc_v5.h"
//###########################################################################################################
#endif

//***********************************************************************************************************
// Config Setting (Internel)
//***********************************************************************************************************
#define EN_SDMMC_TRFUNC          (FALSE)
#define EN_SDMMC_TRSDINFO        (FALSE)

#define D_SDMMC_DRVTYPE          (SD_DRIVER_TYPE_B)
#define D_SDMMC_CURRLMT          (SD_CURR_LMT_200)
#define D_SDMMC_BUSMODE          (SD_MODE_HIGH_SPEED|SD_MODE_UHS_SDR25/*|SD_MODE_UHS_DDR50*/)


// Multiple SD Cards Setting for Each Slot
//-----------------------------------------------------------------------------------------------------------
static IPEmType ge_IPSlot[3]     = {D_SDMMC1_IP, D_SDMMC2_IP, D_SDMMC3_IP};
static PortEmType ge_PORTSlot[3] = {D_SDMMC1_PORT, D_SDMMC2_PORT, D_SDMMC3_PORT};
static PADEmType  ge_PADSlot[3]  = {D_SDMMC1_PAD, D_SDMMC2_PAD, D_SDMMC3_PAD};
static U32_T  gu32_MaxClkSlot[3] = {V_SDMMC1_MAX_CLK, V_SDMMC2_MAX_CLK, V_SDMMC3_MAX_CLK};

//static U8_T gu8_BusModeSupport[3] = {D_SDMMC1_BUSMODE, D_SDMMC2_BUSMODE, D_SDMMC3_BUSMODE};
//static U8_T gu8_DrvTypeSupport[3] = {D_SDMMC1_DRVTYPE, D_SDMMC2_DRVTYPE, D_SDMMC3_DRVTYPE};
//static U8_T gu8_CurrLimitSupport[3] = {D_SDMMC1_CURRLMT, D_SDMMC2_CURRLMT, D_SDMMC3_CURRLMT};


// Global Variable
//-----------------------------------------------------------------------------------------------------------
static SDMMCInfoStruct _stSDMMCInfo[3];
static RspStruct * _pstRsp[3];
static volatile U8_T gu8RspBuf[3][512];	  //512 bits

// Trace Funcion
//-----------------------------------------------------------------------------------------------------------
#if (EN_SDMMC_TRFUNC)
	#define pr_sd_err(fmt, arg...)   //
	#define pr_sd_main(fmt, arg...)  printf(fmt, ##arg)
	#define pr_sd_dbg(fmt, arg...)   //printf(fmt, ##arg)
#else
	#define pr_sd_err(fmt, arg...)   printf(fmt, ##arg)
	#define pr_sd_main(fmt, arg...)  //
	#define pr_sd_dbg(fmt, arg...)   //
#endif

#if (EN_SDMMC_TRSDINFO)
	#define pr_sd_info(fmt, arg...)  printf(fmt, ##arg)
#else
	#define pr_sd_info(fmt, arg...)  //
#endif

//============================================
// Mask Value
//============================================
#define M_SDMMC_CURRSTATE(u32Val)	( (u32Val & (BIT12_T | BIT11_T | BIT10_T | BIT09_T))>>9 )



static U32_T _RetArrToU32(U8_T* u8Arr)
{
    return ((*(u8Arr)<<24)|(*(u8Arr+1)<<16)|(*(u8Arr+2)<<8)|(*(u8Arr+3)));
}

static void _SDMMC_InfoInit(U8_T u8Slot)
{

	//Before _SDMMC_SEND_IF_COND
    _stSDMMCInfo[u8Slot].eHCS = 0;   // Init HCS=0

	//Before _SDMMC_SEND_OP_COND
	_stSDMMCInfo[u8Slot].eCardType     = EV_NOCARD;
    _stSDMMCInfo[u8Slot].u32RCAArg     = 0;
	_stSDMMCInfo[u8Slot].u32OCR        = 0;

	//Before _SDMMC_SEND_SCR
	_stSDMMCInfo[u8Slot].u8BusWidth = 1;
	_stSDMMCInfo[u8Slot].u8SpecVer = 0;
	_stSDMMCInfo[u8Slot].u8SpecVer1 = 0;

	//Before SDMMC_SwitchHighBus
	_stSDMMCInfo[u8Slot].u8AccessMode = 0; //DEF_SPEED_BUS_SPEED
	_stSDMMCInfo[u8Slot].u8DrvStrength = 0;
	_stSDMMCInfo[u8Slot].u8CurrMax = 0;
	_stSDMMCInfo[u8Slot].u8SD3BusMode = 0;
	_stSDMMCInfo[u8Slot].u8SD3DrvType = 0;
	_stSDMMCInfo[u8Slot].u8SD3CurrLimit = 0;
	_stSDMMCInfo[u8Slot].u32MaxClk = CLK_DEF_SPEED;

    //_stSDMMCInfo[u8Slot].stCID;
    //_stSDMMCInfo[u8Slot].stCSD;
	//_stSDMMCInfo[u8Slot].u32CardStatus = 0;
}

static void _SDMMC_GetCIDInfo(U8_T u8Slot, U8_T * u8Arr)
{
    U8_T u8Pos=0, u8Offset=0;


	pr_sd_info("====================== [ CID Info for Slot: %u] =======================\n", u8Slot);

	//-----------------------------------------------------------------------------------------------------------------

	_stSDMMCInfo[u8Slot].stCID.MID = *u8Arr;
	pr_sd_info("[MID]         => (0x%02X)\n", _stSDMMCInfo[u8Slot].stCID.MID);
	//-----------------------------------------------------------------------------------------------------------------
    if(_stSDMMCInfo[u8Slot].eCardType == EV_SD)
	{
		_stSDMMCInfo[u8Slot].stCID.OID[0] = *(u8Arr+1);
		_stSDMMCInfo[u8Slot].stCID.OID[1] = *(u8Arr+2);
	}
    else if(_stSDMMCInfo[u8Slot].eCardType == EV_MMC)
		_stSDMMCInfo[u8Slot].stCID.OID[0] = *(u8Arr+2);

	pr_sd_info("[OID]         => [%c][%c]\n", _stSDMMCInfo[u8Slot].stCID.OID[0], _stSDMMCInfo[u8Slot].stCID.OID[1]);
	//-----------------------------------------------------------------------------------------------------------------

    for(u8Pos=0; u8Pos<5; u8Pos++)
		_stSDMMCInfo[u8Slot].stCID.PNM[u8Pos] = *(u8Arr+3+u8Pos);

    if(_stSDMMCInfo[u8Slot].eCardType == EV_SD)
    {
		u8Offset = 7;
		_stSDMMCInfo[u8Slot].stCID.PNM[5] = '_';
    }
    else if(_stSDMMCInfo[u8Slot].eCardType == EV_MMC)
    {
		u8Offset = 8;
        _stSDMMCInfo[u8Slot].stCID.PNM[5] = *(u8Arr+u8Offset);
    }

	pr_sd_info("[PNM]         => [%c][%c][%c][%c][%c][%c]\n",
			   _stSDMMCInfo[u8Slot].stCID.PNM[0], _stSDMMCInfo[u8Slot].stCID.PNM[1],
			   _stSDMMCInfo[u8Slot].stCID.PNM[2], _stSDMMCInfo[u8Slot].stCID.PNM[3],
			   _stSDMMCInfo[u8Slot].stCID.PNM[4], _stSDMMCInfo[u8Slot].stCID.PNM[5]);
	//-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCID.PRV = *(u8Arr+u8Offset+1);
	pr_sd_info("[PRV]         => (0x%02X)\n", _stSDMMCInfo[u8Slot].stCID.PRV);
	//-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCID.PSN = _RetArrToU32(u8Arr+u8Offset+2);
	pr_sd_info("[PSN]         => (0x%08X)\n", _stSDMMCInfo[u8Slot].stCID.PSN);

	pr_sd_info("======================================================================\n");


}

static void _SDMMC_GetCSDInfo(U8_T u8Slot, U8_T* u8Arr)
{
    U16_T ReffArr[0x10] = {0, 10, 12, 13, 14, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80 };
    U32_T u32CSIZE;
    U8_T  u8CSIZEMULTI, uMulti;

	pr_sd_info("====================== [ CSD Info for Slot: %u] =======================\n", u8Slot);
	//-----------------------------------------------------------------------------------------------------------------
    _stSDMMCInfo[u8Slot].stCSD.CSDSTR = (*u8Arr)>>6;
	pr_sd_info("[CSD_STR]      => (0x%02X)\n", _stSDMMCInfo[u8Slot].stCSD.CSDSTR);
	//-----------------------------------------------------------------------------------------------------------------
    if(_stSDMMCInfo[u8Slot].eCardType == EV_MMC)
		_stSDMMCInfo[u8Slot].stCSD.SPECVERS = (*u8Arr>>2) & 0xF;
	//-----------------------------------------------------------------------------------------------------------------

	_stSDMMCInfo[u8Slot].stCSD.TAAC_NS = 1;
    for(uMulti=1; uMulti<=(*(u8Arr+1) & 0x07); uMulti++ )
		_stSDMMCInfo[u8Slot].stCSD.TAAC_NS *=10;

	pr_sd_info("[TACC]         => (0x%02X)", *(u8Arr+1));
	pr_sd_info(" ==>CAL TU(%u) x TV(%u/10) = ", _stSDMMCInfo[u8Slot].stCSD.TAAC_NS, ReffArr[(*(u8Arr+1)>>3) & 0xF]);
    _stSDMMCInfo[u8Slot].stCSD.TAAC_NS *= ReffArr[(*(u8Arr+1)>>3) & 0xF];
	pr_sd_info(" %u (ns)\n", _stSDMMCInfo[u8Slot].stCSD.TAAC_NS/10);

	//-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.NSAC = *(u8Arr+2);
	pr_sd_info("[NSAC]         => (0x%02X) ==>CAL %u (clks)\n", _stSDMMCInfo[u8Slot].stCSD.NSAC, _stSDMMCInfo[u8Slot].stCSD.NSAC);
	//-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.TRAN_KB = 100;
    for(uMulti=1; uMulti<=(*(u8Arr+3) & 0x07); uMulti++ )
		_stSDMMCInfo[u8Slot].stCSD.TRAN_KB *=10;

	pr_sd_info("[TRAN_SPD]     => (0x%02X)", *(u8Arr+3));
	pr_sd_info("==>CAL TU(%u) x TV(%u/10) = ", _stSDMMCInfo[u8Slot].stCSD.TRAN_KB, ReffArr[(*(u8Arr+3)>>3) & 0xF]);
    _stSDMMCInfo[u8Slot].stCSD.TRAN_KB *= ReffArr[(*(u8Arr+3)>>3) & 0xF];
	pr_sd_info(" %u (Kbit)\n", _stSDMMCInfo[u8Slot].stCSD.TRAN_KB/10);
	//-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.CCC = (*(u8Arr+4)<<4) | (*(u8Arr+5)>>4);
	pr_sd_info("[CCC]          => (0x%04X)\n",  _stSDMMCInfo[u8Slot].stCSD.CCC);
	//-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.R_BLK_SIZE = *(u8Arr+5) & 0XF;
	pr_sd_info("[R_BLK_LEN]    => (2E_%u)\n",  _stSDMMCInfo[u8Slot].stCSD.R_BLK_SIZE);
	//-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.W_BLK_MISALIGN = (*(u8Arr+6)>>6) & 0x1;
	pr_sd_info("[W_BLK_MIS_ALG]=> (%u)\n",  _stSDMMCInfo[u8Slot].stCSD.W_BLK_MISALIGN);
	//-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.R_BLK_MISALIGN = (*(u8Arr+6)>>5) & 0x1;
	pr_sd_info("[R_BLK_MIS_ALG]=> (%u)\n",  _stSDMMCInfo[u8Slot].stCSD.R_BLK_MISALIGN);
	//-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.R2W_FACTOR = (*(u8Arr+12) & 0x1C)>> 2;
	pr_sd_info("[R2W_FACTOR]   => (%u)\n",  _stSDMMCInfo[u8Slot].stCSD.R2W_FACTOR);
	//-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.W_BLK_SIZE = ((*(u8Arr+12) & 0x03)<<2) | (*(u8Arr+13)>>6);
	pr_sd_info("[W_BLK_LEN]    => (2E_%u)\n",  _stSDMMCInfo[u8Slot].stCSD.W_BLK_SIZE);

	//-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.PERM_W_PROTECT = (*(u8Arr+14)>>5) & 0x1;
	pr_sd_info("[PERM_W_PRO]   => (%u)\n",  _stSDMMCInfo[u8Slot].stCSD.PERM_W_PROTECT);

	//-----------------------------------------------------------------------------------------------------------------

    _stSDMMCInfo[u8Slot].stCSD.TEMP_W_PROTECT = (*(u8Arr+14)>>4) & 0x1;
	pr_sd_info("[TMP_W_PRO]    => (%u)\n",  _stSDMMCInfo[u8Slot].stCSD.TEMP_W_PROTECT);
	//-----------------------------------------------------------------------------------------------------------------

	if((_stSDMMCInfo[u8Slot].stCSD.CSDSTR==1) && (_stSDMMCInfo[u8Slot].eCardType == EV_SD))
    {
		u32CSIZE = ((*(u8Arr+7) & 0x3F)<<16) | (*(u8Arr+8)<<8) | *(u8Arr+9);
		_stSDMMCInfo[u8Slot].stCSD.CAPCITY = (u32CSIZE+1) * 524288;

    }else //SD1.X and //MMC
    {
		u32CSIZE = ((*(u8Arr+6) & 0x3)<<10) | (*(u8Arr+7) << 2) | (*(u8Arr+8)>>6) ;
		u8CSIZEMULTI = ((*(u8Arr+9) & 0x3)<<1) | (*(u8Arr+10)>>7) ;
		_stSDMMCInfo[u8Slot].stCSD.CAPCITY = (u32CSIZE+1) * (1<<(u8CSIZEMULTI+2)) * (1<<_stSDMMCInfo[u8Slot].stCSD.R_BLK_SIZE);

    }

	pr_sd_info("[CAPACITY]     => %u (Bytes)\n",  _stSDMMCInfo[u8Slot].stCSD.CAPCITY);

	pr_sd_info("======================================================================\n");


}


static RspStruct *_SDMMC_CMDReq(U8_T u8Slot, U8_T u8Cmd, U32_T u32Arg, SDMMCRspEmType eRspType)
{
	IPEmType eIP = ge_IPSlot[u8Slot];
	//RspErrEmType eErr = EV_STS_OK;
	RspStruct * eRspSt;

	pr_sd_main("_[sdmmc_%u] CMD_%u (0x%08X)", u8Slot, u8Cmd, u32Arg);

	Hal_SDMMC_SetCmdToken(eIP, u8Cmd, u32Arg);
	Hal_SDMMC_SendCmdAndWaitProcess(eIP, EV_EMP, EV_CMDRSP, eRspType, TRUE);
	eRspSt = Hal_SDMMC_GetRspToken(eIP);

	pr_sd_main("=> (Err: 0x%04X)\n", (U16_T)eRspSt->eErrCode);

	return eRspSt;
}


static RspStruct *_SDMMC_DATAReq(U8_T u8Slot, U8_T u8Cmd, U32_T u32Arg, U16_T u16BlkCnt, U16_T u16BlkSize, TransEmType eTransType, volatile U8_T *pu8Buf)
{
	IPEmType eIP = ge_IPSlot[u8Slot];
	//RspErrEmType eErr  = EV_STS_OK;
	CmdEmType eCmdType = EV_CMDREAD;
	RspStruct * eRspSt;

	BOOL_T bCloseClock = FALSE;

	pr_sd_main("_[sdmmc_%u] CMD_%u (0x%08X)__(TB: %u)(BSz: %u)", u8Slot, u8Cmd, u32Arg, u16BlkCnt, u16BlkSize);

	if( (u8Cmd == 24) || (u8Cmd==25))
		eCmdType = EV_CMDWRITE;

	if(u16BlkCnt>1)
		bCloseClock = FALSE;
	else
		bCloseClock = TRUE;

	Hal_SDMMC_SetCmdToken(eIP, u8Cmd, u32Arg);
	Hal_SDMMC_TransCmdSetting(eIP, eTransType, u16BlkCnt, u16BlkSize, Hal_CARD_TransMIUAddr((U32_T)pu8Buf), pu8Buf);
	Hal_SDMMC_SendCmdAndWaitProcess(eIP, eTransType, eCmdType, EV_R1, bCloseClock);
	eRspSt = Hal_SDMMC_GetRspToken(eIP);

	pr_sd_main("=> (Err: 0x%04X)\n", (U16_T)eRspSt->eErrCode);
	return eRspSt;


}


static RspStruct* _SDMMC_SEND_IF_COND(U8_T u8Slot)
{
	IPEmType eIP = ge_IPSlot[u8Slot];

    _pstRsp[eIP] = _SDMMC_CMDReq(u8Slot,  8, (BIT00_T<<8)|0xAA, EV_R7); //CMD8 ==> Alwasy Support STD Voltage

	if(_pstRsp[eIP]->eErrCode) {
		return _pstRsp[eIP];
    }
	if(_pstRsp[eIP]->u8ArrRspToken[4] != 0xAA)
	{
		_pstRsp[eIP]->eErrCode = EV_CMD8_PERR;
		 return _pstRsp[eIP];
	}

	_stSDMMCInfo[u8Slot].eHCS = EV_HCS; //HCS=1

	return _pstRsp[eIP];
}



static RspStruct* _SDMMC_SEND_OP_COND(U8_T u8Slot, CARDTypeEmType eCardType)
{
	IPEmType eIP = ge_IPSlot[u8Slot];
    U32_T u32ReqOCR = SD_OCR_RANGE | _stSDMMCInfo[u8Slot].eHCS;
    U16_T u16Count = 0;

    do
    {

		if(eCardType==EV_SD)
		{
			//------------------------------------------------------------------------------------------------------------
			_pstRsp[eIP] = _SDMMC_CMDReq(u8Slot, 55, _stSDMMCInfo[u8Slot].u32RCAArg, EV_R1); //CMD55;
			//------------------------------------------------------------------------------------------------------------
			if(_pstRsp[eIP]->eErrCode)
				return _pstRsp[eIP];

			//------------------------------------------------------------------------------------------------------------
			_pstRsp[eIP] = _SDMMC_CMDReq(u8Slot, 41, u32ReqOCR, EV_R3); //ACMD41;
			//------------------------------------------------------------------------------------------------------------
			if(_pstRsp[eIP]->eErrCode)
				return _pstRsp[eIP];

			_stSDMMCInfo[u8Slot].u32OCR = _RetArrToU32(_pstRsp[eIP]->u8ArrRspToken+1);

		}

		Hal_Timer_mDelay(1);
		u16Count++;

    }while( !(_stSDMMCInfo[u8Slot].u32OCR & R_OCR_READY) && (u16Count<1000));  // Card powered ready (0:busy,1:ready)

	_stSDMMCInfo[u8Slot].eHCS = (SDCAPEmType)(_stSDMMCInfo[u8Slot].u32OCR & R_OCR_CCS);

	if(u16Count >= 1000)
		_pstRsp[eIP]->eErrCode = EV_OCR_BERR; //Card is still busy
	else if(!(_stSDMMCInfo[u8Slot].u32OCR & SD_OCR_RANGE & 0x3FFFFFFF))
		_pstRsp[eIP]->eErrCode = EV_OUT_VOL_RANGE; //Double Confirm Voltage Range

	if(!_pstRsp[eIP]->eErrCode)
		_stSDMMCInfo[u8Slot].eCardType = eCardType;

	return _pstRsp[eIP];

}

static RspStruct* _SDMMC_SEND_SCR(U8_T u8Slot, volatile U8_T *pu8DataBuf)
{
	IPEmType eIP = ge_IPSlot[u8Slot];

	//------------------------------------------------------------------------------------------------------------
	_pstRsp[eIP] = _SDMMC_CMDReq(u8Slot, 55, _stSDMMCInfo[u8Slot].u32RCAArg, EV_R1); //CMD55;
	//------------------------------------------------------------------------------------------------------------
	if(_pstRsp[eIP]->eErrCode)
		return _pstRsp[eIP];

	//------------------------------------------------------------------------------------------------------------
	_pstRsp[eIP] = _SDMMC_DATAReq(u8Slot, 51, 0x00000000, 1, 8, EV_DMA, pu8DataBuf);  //ACMD51;
	//------------------------------------------------------------------------------------------------------------
	if(_pstRsp[eIP]->eErrCode)
		return _pstRsp[eIP];

	pr_sd_info("====================== [ SCR Info for Slot: %u] =======================\n", u8Slot);

	_stSDMMCInfo[u8Slot].u8SpecVer = (*pu8DataBuf) & 0x7;
	_stSDMMCInfo[u8Slot].u8SpecVer1 = ((*(pu8DataBuf+2)) & 0x80) >> 7;

	if((*(pu8DataBuf+1)) & 0x4)
		_stSDMMCInfo[u8Slot].u8BusWidth = 4;

	pr_sd_info("[SPECVER]      => (0x%02X)\n",  _stSDMMCInfo[u8Slot].u8SpecVer);
	pr_sd_info("[SPECVER1]     => (0x%02X)\n",  _stSDMMCInfo[u8Slot].u8SpecVer1);
	pr_sd_info("[BUSWIDTH]     => (0x%02X)\n",  _stSDMMCInfo[u8Slot].u8BusWidth);

	pr_sd_info("======================================================================\n");

	return _pstRsp[eIP];

}


static RspStruct* _SDMMC_SEND_STAUS(U8_T u8Slot)
{
	IPEmType eIP = ge_IPSlot[u8Slot];
	//--------------------------------------------------------------------------------------------------------
	_pstRsp[eIP] = _SDMMC_CMDReq(u8Slot, 13, _stSDMMCInfo[u8Slot].u32RCAArg, EV_R1);  //CMD13
	//--------------------------------------------------------------------------------------------------------
	if(!_pstRsp[eIP]->eErrCode)
	{
		_stSDMMCInfo[u8Slot].u32CardStatus = _RetArrToU32(_pstRsp[eIP]->u8ArrRspToken+1);
		if(!(M_SDMMC_CURRSTATE(_stSDMMCInfo[u8Slot].u32CardStatus) & 0x03)) //stand by state
			_pstRsp[eIP]->eErrCode = EV_STATE_ERR;
	}

	return _pstRsp[eIP];

}

static RspStruct* _SDMMC_SET_BUS_WIDTH(U8_T u8Slot)
{
	IPEmType eIP = ge_IPSlot[u8Slot];


	//------------------------------------------------------------------------------------------------------------
	_pstRsp[eIP] = _SDMMC_CMDReq(u8Slot, 55, _stSDMMCInfo[u8Slot].u32RCAArg, EV_R1); //CMD55;
	//------------------------------------------------------------------------------------------------------------
	if(_pstRsp[eIP]->eErrCode)
		return _pstRsp[eIP];

	//------------------------------------------------------------------------------------------------------------
	_pstRsp[eIP] = _SDMMC_CMDReq(u8Slot, 6, 0x00000002, EV_R1); //ACMD6;
	//------------------------------------------------------------------------------------------------------------

	return _pstRsp[eIP];

}

static RspStruct* _SDMMC_SD_SWITCH_FUNC(U8_T u8Slot, BOOL_T bSetMode, U8_T u8Group, U8_T u8Value, volatile U8_T *pu8DataBuf)
{
	IPEmType eIP = ge_IPSlot[u8Slot];
	U32_T u32Arg = (bSetMode << 31) | 0x00FFFFFF;
	u32Arg &= ~(0xF << (u8Group * 4));
	u32Arg |= u8Value << (u8Group * 4);

	//------------------------------------------------------------------------------------------------------------
	_pstRsp[eIP] = _SDMMC_DATAReq(u8Slot, 6, u32Arg, 1, 64, EV_DMA, pu8DataBuf);  //CMD6; Query
	//------------------------------------------------------------------------------------------------------------

	return _pstRsp[eIP];

}



static RspStruct* _SDMMC_Identification(U8_T u8Slot)
{
	IPEmType eIP = ge_IPSlot[u8Slot];

    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CMDReq(u8Slot,  0, 0x00000000, EV_NO); //CMD0;
    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_SEND_IF_COND(u8Slot); //CMD8
    //------------------------------------------------------------------------------------------------------------
	if(_pstRsp[eIP]->eErrCode && !(_pstRsp[eIP]->eErrCode & EV_STS_NORSP)) {
		return _pstRsp[eIP];
    }

    //------------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_SEND_OP_COND(u8Slot, EV_SD); //ACMD41
    //------------------------------------------------------------------------------------------------------------
	if(_pstRsp[eIP]->eErrCode)
		return _pstRsp[eIP];

    //--------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CMDReq(u8Slot, 2, 0x00000000, EV_R2); //CMD2;
    //--------------------------------------------------------------------------------------------------------
    if(_pstRsp[eIP]->eErrCode)
        return _pstRsp[eIP];
	else
		_SDMMC_GetCIDInfo(u8Slot, _pstRsp[eIP]->u8ArrRspToken+1);
    //--------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CMDReq(u8Slot, 3, _stSDMMCInfo[u8Slot].u32RCAArg, EV_R6); //CMD3;
    //--------------------------------------------------------------------------------------------------------
    if(_pstRsp[eIP]->eErrCode)
        return _pstRsp[eIP];

    if(_stSDMMCInfo[u8Slot].eCardType == EV_SD)
		_stSDMMCInfo[u8Slot].u32RCAArg = _RetArrToU32(_pstRsp[eIP]->u8ArrRspToken+1) & 0xFFFF0000;

    return _pstRsp[eIP];

}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_SwitchPAD
 *     @author jeremy.wang (2013/7/30)
 * Desc:
 *
 * @param u8Slot :
 ----------------------------------------------------------------------------------------------------------*/
void SDMMC_SwitchPAD(U8_T u8Slot)
{
	IPEmType eIP      = ge_IPSlot[u8Slot];
	PortEmType ePort  = ge_PORTSlot[u8Slot];
	PADEmType ePAD    = ge_PADSlot[u8Slot];

	Hal_CARD_IPOnceSetting(eIP);
	Hal_CARD_SetPADToPortPath(eIP, ePort, ePAD, FALSE);
	Hal_CARD_InitPADPin(ePAD, FALSE);
	Hal_CARD_IPBeginSetting(eIP);
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_SetPAD
 *     @author jeremy.wang (2013/7/30)
 * Desc:
 *
 * @param u8Slot :
 * @param eIP :
 * @param ePort :
 * @param ePAD :
 ----------------------------------------------------------------------------------------------------------*/
void SDMMC_SetPAD(U8_T u8Slot, IPEmType eIP, PortEmType ePort, PADEmType ePAD)
{
	ge_IPSlot[u8Slot] = eIP;
	ge_PORTSlot[u8Slot] = ePort;
	ge_PADSlot[u8Slot] = ePAD;

}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_SetPower
 *     @author jeremy.wang (2013/7/30)
 * Desc:
 *
 * @param u8Slot :
 * @param ePower :
 ----------------------------------------------------------------------------------------------------------*/
void SDMMC_SetPower(U8_T u8Slot, PowerEmType ePower)
{
	IPEmType eIP      = ge_IPSlot[u8Slot];
	PADEmType ePAD    = ge_PADSlot[u8Slot];

	if(ePower == EV_POWER_OFF) // Power Off
	{
		Hal_SDMMC_ClkCtrl(eIP, FALSE, 0);
		Hal_CARD_PullPADPin(ePAD, EV_PULLDOWN, FALSE);
		Hal_CARD_PowerOff(ePAD, WT_POWEROFF); //For SD PAD

	}
    else if(ePower == EV_POWER_ON)  // Power Up
    {
		Hal_CARD_PullPADPin(ePAD, EV_PULLUP, FALSE);
		Hal_CARD_PowerOn(ePAD, WT_POWERUP);
    }
	else if(ePower == EV_POWER_UP) // Power On
	{
		Hal_SDMMC_ClkCtrl(eIP, TRUE, WT_POWERON);
		Hal_SDMMC_Reset(eIP); // For SRAM Issue
	}
}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_SetClock
 *     @author jeremy.wang (2013/8/15)
 * Desc:
 *
 * @param u8Slot :
 * @param u32ReffClk :
 * @param u8PassLevel :
 *
 * @return U32_T  :
 ----------------------------------------------------------------------------------------------------------*/
U32_T SDMMC_SetClock(U8_T u8Slot, U32_T u32ReffClk, U8_T u8DownLevel)
{
	IPEmType eIP = ge_IPSlot[u8Slot];
	U32_T u32RealClk = 0;

	if(u32ReffClk==0) //Not Set
		u32ReffClk = _stSDMMCInfo[u8Slot].u32MaxClk;

	if(u32ReffClk > gu32_MaxClkSlot[u8Slot])
		u32ReffClk = gu32_MaxClkSlot[u8Slot];


	//************ Set Bus Clock ******************
	u32RealClk = Hal_CARD_FindClockSetting(eIP, u32ReffClk, 0, u8DownLevel);
	Hal_CARD_SetClock(eIP, u32RealClk);
	Hal_SDMMC_SetNrcDelay(eIP, u32RealClk);

	pr_sd_info("====================== [ Clk Info for Slot: %u ==>(%u Hz)] \n", u8Slot, u32RealClk);

	return u32RealClk;

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: SDMMC_SetBusTiming
*     @author jeremy.wang (2015/10/2)
* Desc:
*
* @param u8Slot :
* @param u8BusSpdMode :
----------------------------------------------------------------------------------------------------------*/
void SDMMC_SetBusTiming(U8_T u8Slot, BusTimingEmType eBusTiming)
{
	IPEmType eIP = ge_IPSlot[u8Slot];

	Hal_SDMMC_SetBusTiming(eIP, eBusTiming);
	Hal_CARD_SetBusTiming(eIP, eBusTiming);

}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_CardDetect
 *     @author jeremy.wang (2013/7/29)
 * Desc:
 *
 * @return BOOL_T  :
 ----------------------------------------------------------------------------------------------------------*/
BOOL_T SDMMC_CardDetect(U8_T u8Slot)
{
	//IPEmType eIP = ge_IPSlot[u8Slot];

	SDMMC_SwitchPAD(u8Slot);

	return Hal_CARD_GetGPIOState((GPIOEmType)u8Slot);
}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_SetWideBus
 *     @author jeremy.wang (2013/8/15)
 * Desc:
 *
 * @param u8Slot :
 *
 * @return U16_T  :
 ----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_SetWideBus(U8_T u8Slot)
{
	IPEmType eIP      = ge_IPSlot[u8Slot];

    if(_stSDMMCInfo[u8Slot].eCardType==EV_SD)
	{
		if(_stSDMMCInfo[u8Slot].u8BusWidth == 4)
		{
			_pstRsp[eIP] = _SDMMC_SET_BUS_WIDTH(u8Slot);

			if(_pstRsp[eIP]->eErrCode)
				return (U16_T)_pstRsp[eIP]->eErrCode;

			Hal_SDMMC_SetDataWidth(eIP, EV_BUS_4BITS);
		}
	}

	return 0;
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_SwitchHighBus
 *     @author jeremy.wang (2013/8/15)
 * Desc:
 *
 * @param u8Slot :
 *
 * @return U16_T  :
 ----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_SwitchHighBus(U8_T u8Slot)
{
	IPEmType eIP      = ge_IPSlot[u8Slot];
	U8_T  u8BusMode   = 0, u8DrvType = 0, u8CurrLimit = 0;
	U32_T u32MaxClk = CLK_DEF_SPEED;

    if(_stSDMMCInfo[u8Slot].eCardType==EV_SD)
	{
		if(_stSDMMCInfo[u8Slot].u8SpecVer==0) //SD 1.0=> Not Support SD CMD6 for HS
			return 0;

		//************************************** Check Function ***********************************************

		//------------------------------------------------------------------------------------------------------------
		_pstRsp[eIP] = _SDMMC_SD_SWITCH_FUNC(u8Slot, 0, 0, 1, gu8RspBuf[u8Slot]); //Query Group 1
		//------------------------------------------------------------------------------------------------------------
		if(_pstRsp[eIP]->eErrCode)
			return (U16_T)_pstRsp[eIP]->eErrCode;

		u8BusMode = gu8RspBuf[u8Slot][13];

		// SD 3.0
		if(_stSDMMCInfo[u8Slot].u8SpecVer1)
		{
			 _stSDMMCInfo[u8Slot].u8SD3BusMode = u8BusMode;

			//------------------------------------------------------------------------------------------------------------
			_pstRsp[eIP] = _SDMMC_SD_SWITCH_FUNC(u8Slot, 0, 2, 1, gu8RspBuf[u8Slot]); //Query Group 3
			//------------------------------------------------------------------------------------------------------------
			if(_pstRsp[eIP]->eErrCode)
				return (U16_T)_pstRsp[eIP]->eErrCode;

			_stSDMMCInfo[u8Slot].u8SD3DrvType = gu8RspBuf[u8Slot][9];
			//------------------------------------------------------------------------------------------------------------
			_pstRsp[eIP] = _SDMMC_SD_SWITCH_FUNC(u8Slot, 0, 3, 1, gu8RspBuf[u8Slot]); //Query Group 4
			//------------------------------------------------------------------------------------------------------------
			if(_pstRsp[eIP]->eErrCode)
				return (U16_T)_pstRsp[eIP]->eErrCode;

			_stSDMMCInfo[u8Slot].u8SD3CurrLimit = gu8RspBuf[u8Slot][7];

		}

		//printf("===> gu8RspBuf[13,9,7] = (0x%02X)(0x%02X)(0x%02X)\n", u8BusMode, _stSDMMCInfo[u8Slot].u8SD3DrvType, _stSDMMCInfo[u8Slot].u8SD3CurrLimit);

		//************************************** Set Funciton ***********************************************

		if( !(_stSDMMCInfo[u8Slot].u32OCR & R_OCR_S18)) // SD 2.0, SD3.0 => SD2.0
		{
			u8BusMode = u8BusMode & D_SDMMC_BUSMODE;

			if(u8BusMode & SD_MODE_HIGH_SPEED) // Support High Speed
			{
				_stSDMMCInfo[u8Slot].u8AccessMode = HIGH_SPEED_BUS_SPEED;

				//------------------------------------------------------------------------------------------------------------
				_pstRsp[eIP] = _SDMMC_SD_SWITCH_FUNC(u8Slot, 1, 0, _stSDMMCInfo[u8Slot].u8AccessMode, gu8RspBuf[u8Slot]); //Set Group 1
				//------------------------------------------------------------------------------------------------------------
				if(_pstRsp[eIP]->eErrCode)
					return (U16_T)_pstRsp[eIP]->eErrCode;

				if( (gu8RspBuf[u8Slot][16] & 0xF) != _stSDMMCInfo[u8Slot].u8AccessMode )
					printf("_[sdmmc_%u] Warning: Problem switching high bus speed mode!\n", u8Slot);
				else
					_stSDMMCInfo[u8Slot].u32MaxClk = CLK_HIGH_SPEED;

			}

		}
		else //SD3.0
		{

			u8DrvType = _stSDMMCInfo[u8Slot].u8SD3DrvType & D_SDMMC_DRVTYPE;

			if(u8DrvType & SD_DRIVER_TYPE_B)
				_stSDMMCInfo[u8Slot].u8DrvStrength = UHS_B_DRV_TYPE;
			else if(u8DrvType & SD_DRIVER_TYPE_A)
				_stSDMMCInfo[u8Slot].u8DrvStrength = UHS_A_DRV_TYPE;
			else if(u8DrvType & SD_DRIVER_TYPE_C)
				_stSDMMCInfo[u8Slot].u8DrvStrength = UHS_C_DRV_TYPE;
			else if(u8DrvType & SD_DRIVER_TYPE_D)
				_stSDMMCInfo[u8Slot].u8DrvStrength = UHS_D_DRV_TYPE;

			//------------------------------------------------------------------------------------------------------------
			_pstRsp[eIP] = _SDMMC_SD_SWITCH_FUNC(u8Slot, 1, 2, _stSDMMCInfo[u8Slot].u8DrvStrength, gu8RspBuf[u8Slot]); //Set Group 3
			//------------------------------------------------------------------------------------------------------------
			if(_pstRsp[eIP]->eErrCode)
				return (U16_T)_pstRsp[eIP]->eErrCode;

			if( (gu8RspBuf[u8Slot][15] & 0xF) != _stSDMMCInfo[u8Slot].u8DrvStrength )
				printf("_[sdmmc_%u] Warning: Problem switching drive strength!\n", u8Slot);


			u8CurrLimit = _stSDMMCInfo[u8Slot].u8SD3CurrLimit & D_SDMMC_CURRLMT;

			if(u8CurrLimit & SD_CURR_LMT_800)
				_stSDMMCInfo[u8Slot].u8CurrMax = UHS_800_CURR_LMT;
			else if(u8CurrLimit & SD_CURR_LMT_600)
				_stSDMMCInfo[u8Slot].u8CurrMax = UHS_600_CURR_LMT;
			else if(u8CurrLimit & SD_CURR_LMT_400)
				_stSDMMCInfo[u8Slot].u8CurrMax = UHS_400_CURR_LMT;
			else if(u8CurrLimit & SD_CURR_LMT_200)
				_stSDMMCInfo[u8Slot].u8CurrMax = UHS_200_CURR_LMT;

			//------------------------------------------------------------------------------------------------------------
			_pstRsp[eIP] = _SDMMC_SD_SWITCH_FUNC(u8Slot, 1, 3, _stSDMMCInfo[u8Slot].u8CurrMax, gu8RspBuf[u8Slot]); //Set Group 4
			//------------------------------------------------------------------------------------------------------------
			if(_pstRsp[eIP]->eErrCode)
				return (U16_T)_pstRsp[eIP]->eErrCode;
			if( ( (gu8RspBuf[u8Slot][15]>>4) & 0xF) != _stSDMMCInfo[u8Slot].u8DrvStrength )
				printf("_[sdmmc_%u] Warning: Problem switching current limit!\n", u8Slot);


			u8BusMode = _stSDMMCInfo[u8Slot].u8SD3BusMode & D_SDMMC_BUSMODE;

			if(u8BusMode & SD_MODE_UHS_SDR104) // Support SDR 104
			{
				_stSDMMCInfo[u8Slot].u8AccessMode = UHS_SDR104_BUS_SPEED;
				u32MaxClk = CLK_SDR104_SPEED;
			}
			else if(u8BusMode & SD_MODE_UHS_DDR50) // Support DDR 50
			{
				_stSDMMCInfo[u8Slot].u8AccessMode = UHS_DDR50_BUS_SPEED;
				u32MaxClk = CLK_DDR50_SPEED;
			}
			else if(u8BusMode & SD_MODE_UHS_SDR50) // Support SDR 50
			{
				_stSDMMCInfo[u8Slot].u8AccessMode = UHS_SDR50_BUS_SPEED;
				u32MaxClk = CLK_SDR50_SPEED;
			}
			else if(u8BusMode & SD_MODE_UHS_SDR25) // Support SDR 25
			{
				_stSDMMCInfo[u8Slot].u8AccessMode = UHS_SDR25_BUS_SPEED;
				u32MaxClk = CLK_SDR25_SPEED;
			}
			else if(u8BusMode & SD_MODE_UHS_SDR12) // Support SDR 12
			{
				_stSDMMCInfo[u8Slot].u8AccessMode = UHS_SDR12_BUS_SPEED;
				u32MaxClk = CLK_SDR12_SPEED;
			}

			//------------------------------------------------------------------------------------------------------------
			_pstRsp[eIP] = _SDMMC_SD_SWITCH_FUNC(u8Slot, 1, 0, _stSDMMCInfo[u8Slot].u8AccessMode, gu8RspBuf[u8Slot]); //Set Group 1
			//------------------------------------------------------------------------------------------------------------
			if(_pstRsp[eIP]->eErrCode)
				return (U16_T)_pstRsp[eIP]->eErrCode;

			if( (gu8RspBuf[u8Slot][16] & 0xF)!= _stSDMMCInfo[u8Slot].u8AccessMode )
				printf("_[sdmmc_%u] Warning: Problem switching bus speed mode!\n", u8Slot);
			else
				_stSDMMCInfo[u8Slot].u32MaxClk = u32MaxClk;
		}

	}

	return 0;

}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: SDMMC_Init
 *     @author jeremy.wang (2013/7/30)
 * Desc: SDMMC driver init
 *
 * @param u8Slot : Slot ID
 *
 * @return U16_T  : U16 Error Code
 ----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_Init(U8_T u8Slot)
{
	IPEmType eIP  = ge_IPSlot[u8Slot];

	_SDMMC_InfoInit(u8Slot);

	SDMMC_SwitchPAD(u8Slot);
	SDMMC_SetPower(u8Slot, EV_POWER_OFF);
	SDMMC_SetPower(u8Slot, EV_POWER_ON);
	SDMMC_SetPower(u8Slot, EV_POWER_UP);

	SDMMC_SetClock(u8Slot, 400000, 0);
	SDMMC_SetBusTiming(u8Slot, EV_BUS_LOW);

	Hal_SDMMC_SetDataWidth(eIP, EV_BUS_1BIT);
	//Hal_SDMMC_SetSDIOClk(eIP, TRUE); //For Measure Clock, Don't Stop Clock

    //--------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_Identification(u8Slot);
    //--------------------------------------------------------------------------------------------------------
    if(_pstRsp[eIP]->eErrCode)
        pr_sd_info(" errCmd:%d:x%x\n", _pstRsp[eIP]->u8Cmd, _pstRsp[eIP]->eErrCode);
    if(_pstRsp[eIP]->eErrCode)
		return (U16_T)_pstRsp[eIP]->eErrCode;

    //--------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CMDReq(u8Slot, 9, _stSDMMCInfo[u8Slot].u32RCAArg, EV_R2);  //CMD9
    //--------------------------------------------------------------------------------------------------------
    if(_pstRsp[eIP]->eErrCode)
		return (U16_T)_pstRsp[eIP]->eErrCode;
	else
		_SDMMC_GetCSDInfo(u8Slot, _pstRsp[eIP]->u8ArrRspToken+1);

    //--------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_SEND_STAUS(u8Slot); //CMD13
    //--------------------------------------------------------------------------------------------------------
    if(_pstRsp[eIP]->eErrCode)
		return (U16_T)_pstRsp[eIP]->eErrCode;

    //--------------------------------------------------------------------------------------------------------
    _pstRsp[eIP] = _SDMMC_CMDReq(u8Slot,  7, _stSDMMCInfo[u8Slot].u32RCAArg, EV_R1B); //CMD7;
    //--------------------------------------------------------------------------------------------------------
    if(_pstRsp[eIP]->eErrCode)
		return (U16_T)_pstRsp[eIP]->eErrCode;

	_pstRsp[eIP] = _SDMMC_SEND_SCR(u8Slot, gu8RspBuf[u8Slot]);


    return (U16_T)_pstRsp[eIP]->eErrCode;

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: SDMMC_CIF_BLK_R
*     @author jeremy.wang (2015/7/24)
* Desc: SDMMC use CIFD to Read Blocks
*
* @param u8Slot : Slot ID
* @param u32CardAddr : Card Address
* @param u16BlkCnt : Block Count
* @param pu8DataBuf :  Data Buffer Pointer
*
* @return U16_T  : U16 Error Code
----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_CIF_BLK_R(U8_T u8Slot, U32_T u32CardAddr, volatile U8_T *pu8DataBuf)
{
	IPEmType eIP = ge_IPSlot[u8Slot];

    if(_stSDMMCInfo[u8Slot].eHCS == EV_HCS)
		u32CardAddr>>=9;

	//------------------------------------------------------------------------------------------------------------
	_pstRsp[eIP] = _SDMMC_DATAReq(u8Slot, 17, u32CardAddr, 1, 512, EV_CIF, pu8DataBuf);  //CMD17
	//------------------------------------------------------------------------------------------------------------
	return _pstRsp[eIP]->eErrCode;

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: SDMMC_CIF_BLK_W
*     @author jeremy.wang (2015/7/24)
* Desc: SDMMC use CIFD to Write Blocks
*
* @param u8Slot : Slot ID
* @param u32CardAddr : Card Address
* @param pu8DataBuf : Data Buffer Pointer
*
* @return U16_T  : U16 Error Code
----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_CIF_BLK_W(U8_T u8Slot, U32_T u32CardAddr, volatile U8_T *pu8DataBuf)
{
	IPEmType eIP = ge_IPSlot[u8Slot];

    if(_stSDMMCInfo[u8Slot].eHCS == EV_HCS)
		u32CardAddr>>=9;

	//------------------------------------------------------------------------------------------------------------
	_pstRsp[eIP] = _SDMMC_DATAReq(u8Slot, 24, u32CardAddr, 1, 512, EV_CIF, pu8DataBuf);  //CMD24
	//------------------------------------------------------------------------------------------------------------
	return _pstRsp[eIP]->eErrCode;

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: SDMMC_DMA_BLK_R
*     @author jeremy.wang (2015/7/23)
* Desc: SDMMC use DMA to Read Blocks
*
* @param u8Slot : Slot ID
* @param u32CardAddr : Card Address
* @param u16BlkCnt : Block Count
* @param pu8DataBuf : Data Buffer Pointer
*
* @return U16_T  : U16 Error Code
----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_DMA_BLK_R(U8_T u8Slot, U32_T u32CardAddr, U16_T u16BlkCnt, volatile U8_T *pu8DataBuf)
{
	IPEmType eIP = ge_IPSlot[u8Slot];
	U8_T u8CMD = 17;
	U16_T u16Ret = 0;

    if(_stSDMMCInfo[u8Slot].eHCS == EV_HCS)
		u32CardAddr>>=9;

	if(u16BlkCnt>1)
		u8CMD = 18;

	//------------------------------------------------------------------------------------------------------------
	_pstRsp[eIP] = _SDMMC_DATAReq(u8Slot, u8CMD, u32CardAddr, u16BlkCnt, 512, EV_DMA, pu8DataBuf);  //CMD17, CMD18
	//------------------------------------------------------------------------------------------------------------
	u16Ret = _pstRsp[eIP]->eErrCode;

	if(u16BlkCnt>1)
		//--------------------------------------------------------------------------------------------------------
		_pstRsp[eIP] = _SDMMC_CMDReq(u8Slot,  12, 0x00000000, EV_R1B); //CMD12;
		//--------------------------------------------------------------------------------------------------------


	return u16Ret;

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: SDMMC_DMA_BLK_W
*     @author jeremy.wang (2015/7/23)
* Desc: SDMMC use DMA to Write Blocks
*
* @param u8Slot : Slot ID
* @param u32CardAddr : Card Address
* @param u16BlkCnt : Block Count
* @param pu8DataBuf : Data Buffer Pointer
*
* @return U16_T  : U16 Error Code
----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_DMA_BLK_W(U8_T u8Slot, U32_T u32CardAddr, U16_T u16BlkCnt, volatile U8_T *pu8DataBuf)
{
	IPEmType eIP = ge_IPSlot[u8Slot];
	U8_T u8CMD = 24;
	U16_T u16Ret = 0;

    if(_stSDMMCInfo[u8Slot].eHCS == EV_HCS)
		u32CardAddr>>=9;

	if(u16BlkCnt>1)
		u8CMD = 25;

	//------------------------------------------------------------------------------------------------------------
	_pstRsp[eIP] = _SDMMC_DATAReq(u8Slot, u8CMD, u32CardAddr, u16BlkCnt, 512, EV_DMA, pu8DataBuf);  //CMD24, CMD25
	//------------------------------------------------------------------------------------------------------------
	u16Ret = _pstRsp[eIP]->eErrCode;

	if(u16BlkCnt>1)
		//--------------------------------------------------------------------------------------------------------
		_pstRsp[eIP] = _SDMMC_CMDReq(u8Slot,  12, 0x00000000, EV_R1B); //CMD12;
		//--------------------------------------------------------------------------------------------------------


	return u16Ret;

}


//###########################################################################################################
#if (D_FCIE_M_VER == D_FCIE_M_VER__05)
//###########################################################################################################

/*----------------------------------------------------------------------------------------------------------
*
* Function: SDMMC_ADMA_BLK_R
*     @author jeremy.wang (2015/7/24)
* Desc:
*
* @param u8Slot : Slot ID
* @param u32CardAddr : Card Address
* @param pu32ArrDAddr : DMA Address Array
* @param pu16ArrBCnt : DMA Block Count Array
* @param u16ItemCnt : DMA Item
* @param pDMATable : Memory Pointer to Find DMA Table
*
* @return U16_T  : U16 Error Code
----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_ADMA_BLK_R(U8_T u8Slot, U32_T u32CardAddr, U32_T *pu32ArrDAddr, U16_T *pu16ArrBCnt, U16_T u16ItemCnt, volatile void *pDMATable)
{
	IPEmType eIP = ge_IPSlot[u8Slot];
	U8_T u8CMD = 17;
	U16_T u16Ret = 0, u16Item = 0, u16BlkCnt = 0;
	BOOL_T bEnd = (FALSE);


    if(_stSDMMCInfo[u8Slot].eHCS == EV_HCS)
		u32CardAddr>>=9;

	for(u16Item=0; u16Item<u16ItemCnt; u16Item++ )
	{
		if( u16Item==(u16ItemCnt-1) )
			bEnd = (TRUE);

		u16BlkCnt += pu16ArrBCnt[u16Item];
		Hal_SDMMC_ADMASetting(eIP, (volatile void *)pDMATable, u16Item, pu16ArrBCnt[u16Item]<<9, (U32_T)pu32ArrDAddr[u16Item], 0, bEnd);
	}

	if(u16BlkCnt>1)
		u8CMD = 18;

	//------------------------------------------------------------------------------------------------------------
	_pstRsp[eIP] = _SDMMC_DATAReq(u8Slot, u8CMD, u32CardAddr, u16BlkCnt, 512, EV_ADMA, (volatile U8_T*)pDMATable);  //CMD17, CMD18
	//------------------------------------------------------------------------------------------------------------
	u16Ret = _pstRsp[eIP]->eErrCode;

	if(u16BlkCnt>1)
		//--------------------------------------------------------------------------------------------------------
		_pstRsp[eIP] = _SDMMC_CMDReq(u8Slot,  12, 0x00000000, EV_R1B); //CMD12;
		//--------------------------------------------------------------------------------------------------------


	return u16Ret;

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: SDMMC_ADMA_BLK_W
*     @author jeremy.wang (2015/7/24)
* Desc:
*
* @param u8Slot : Slot ID
* @param u32CardAddr : Card Address
* @param pu32ArrDAddr : DMA Address Array
* @param pu16ArrBCnt : DMA Block Count Array
* @param u16ItemCnt : DMA Item
* @param pDMATable : Memory Pointer to Find DMA Table
*
* @return U16_T  : U16 Error Code
----------------------------------------------------------------------------------------------------------*/
U16_T SDMMC_ADMA_BLK_W(U8_T u8Slot, U32_T u32CardAddr, U32_T *pu32ArrDAddr, U16_T *pu16ArrBCnt, U16_T u16ItemCnt, volatile void *pDMATable)
{
	IPEmType eIP = ge_IPSlot[u8Slot];
	U8_T u8CMD = 24;
	U16_T u16Ret = 0, u16Item = 0, u16BlkCnt = 0;

	BOOL_T bEnd = (FALSE);


    if(_stSDMMCInfo[u8Slot].eHCS == EV_HCS)
		u32CardAddr>>=9;

	for(u16Item=0; u16Item<u16ItemCnt; u16Item++ )
	{
		if( u16Item==(u16ItemCnt-1) )
			bEnd = (TRUE);

		u16BlkCnt +=pu16ArrBCnt[u16Item];
		Hal_SDMMC_ADMASetting(eIP, (volatile void *)pDMATable, u16Item, pu16ArrBCnt[u16Item]<<9, pu32ArrDAddr[u16Item], 0, bEnd);
	}

	if(u16BlkCnt>1)
		u8CMD = 25;

	//------------------------------------------------------------------------------------------------------------
	_pstRsp[eIP] = _SDMMC_DATAReq(u8Slot, u8CMD, u32CardAddr, u16BlkCnt, 512, EV_ADMA, (volatile U8_T*)pDMATable);  //CMD24, CMD25
	//------------------------------------------------------------------------------------------------------------
	u16Ret = _pstRsp[eIP]->eErrCode;

	if(u16BlkCnt>1)
		//--------------------------------------------------------------------------------------------------------
		_pstRsp[eIP] = _SDMMC_CMDReq(u8Slot,  12, 0x00000000, EV_R1B); //CMD12;
		//--------------------------------------------------------------------------------------------------------

	return u16Ret;

}


//###########################################################################################################
#endif  // End (D_FCIE_M_VER == D_FCIE_M_VER__05)


/*void SDMMC_SDIO_DET_ON(U8_T u8Slot)
{
}*/














