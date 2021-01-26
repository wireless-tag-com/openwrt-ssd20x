/*
* hal_sdmmc.c- Sigmastar
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
 * FileName hal_sdmmc.c
 *     @author jeremy.wang (2011/07/21)
 * Desc:
 * 	   HAL SD Driver will support basic SD function but not flow process.
 *         The goal is that we don't need to change HAL Level code (But its h file code)
 *
 *	   The limitation were listed as below:
 * 	   (1) This c file belongs to HAL level.
 *	   (2) Its h file is included by driver API level, not driver flow process.
 * 	   (3) FCIE/SDIO IP register and buffer opertion function belong to here.
 * 	   (4) Only IP flow concept, but not SD command concept.
 * 	   (5) This c file could not use project/os/cpu/icver/specific define option here, but its h file could.
 *
 * 	   P.S. EN_XX for ON/OFF Define, , V_XX  for Value Define,
 * 			RT_XX for Retry Times Define, WT_XX for Wait Time Define, M_XX for Mask Value Define
 *
 ***************************************************************************************************************/

#include "../inc/hal_sdmmc.h"
#include "../inc/hal_card_timer.h"

#if(EN_BIND_CARD_INT)
#include "../inc/hal_card_intr.h"  //inlcue but may not use
#endif

//***********************************************************************************************************
// Config Setting (Internel)
//***********************************************************************************************************

// Enable Setting
//-----------------------------------------------------------------------------------------------------------
#define EN_TRFUNC         (FALSE)
#define EN_DUMPREG        (TRUE)
#define EN_BYTEMODE       (TRUE)

// Retry Times
//-----------------------------------------------------------------------------------------------------------
#define RT_CLEAN_SDSTS				3
#define RT_CLEAN_MIEEVENT			3

// Wait Time
//-----------------------------------------------------------------------------------------------------------
#define WT_DAT0HI_END				3000    //(ms)
#define WT_FIFO_CLK_RDY				100     //(ms)

//***********************************************************************************************************
//***********************************************************************************************************

// Reg Static Init Setting
//-----------------------------------------------------------------------------------------------------------
#define V_MIE_PATH_INIT     R_SD_EN
#define V_MMA_PRI_INIT      (R_MMA_R_PRIORITY|R_MMA_W_PRIORITY|V_MIUCLK_CTRL_INIT|V_MIUBUS_CTRL_INIT)
#define V_MIE_INT_EN_INIT   (R_SD_CMD_END_EN)
#define V_RSP_SIZE_INIT     0x0
#define V_CMD_SIZE_INIT     0x05
#define V_SD_CTL_INIT       (R_SDIO_DET_INTSRC)
#define V_SD_MODE_INIT      (R_SD_CLK_EN|R_SD_CS_EN)
#define V_SD_BOOTCFG_INIT   (R_SDYPASSMODE)


// Mask Range
//-----------------------------------------------------------------------------------------------------------
#define M_SD_ERRSTS			(R_SD_DAT_CERR|R_SD_DAT_STSERR|R_SD_DAT_STSNEG|R_SD_CMD_NORSP|R_SD_CMDRSP_CERR) //0x1F
#define M_SD_MIEEVENT		(R_MMA_DATA_END|R_SD_CMD_END|R_SD_DATA_END|R_CARD_DMA_END|R_MMA_LSDONE_END)


// Mask Reg Value
//-----------------------------------------------------------------------------------------------------------
#define M_REG_STSERR(IP)            (CARD_REG(A_SD_STS_REG(IP)) & M_SD_ERRSTS)  //0x1F
#define M_REG_SDMIEEvent(IP)        (CARD_REG(A_MIE_EVENT_REG(IP)) & M_SD_MIEEVENT)
#define M_REG_FIFORDYCHK(IP)		(CARD_REG(A_MMA_PRI_REG_REG(IP)) & R_JOB_RW_DIR & R_FIFO_CLK_RDY)


//IP_FCIE or IP_SDIO Register Basic Address
//-----------------------------------------------------------------------------------------------------------
#define A_SD_REG_POS(IP)                GET_CARD_BANK(IP, 0)
#define A_SD_CFIFO_POS(IP)              GET_CARD_BANK(IP, 1)
#define A_SD_CIFD_BANK(IP)              GET_CARD_BANK(IP, 2)

#define A_MIE_EVENT_REG(IP)             GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x00)
#define A_MMA_PRI_REG_REG(IP)           GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x02)
#define A_MIU_DMA1_REG(IP)              GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x03)
#define A_MIU_DMA0_REG(IP)              GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x04)
#define A_CARD_DET_REG(IP)              GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x07)
#define A_CARD_PWR_REG(IP)              GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x08)
#define A_INTR_TEST_REG(IP)             GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x09)
#define A_MIE_PATH_CTL_REG(IP)          GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0A)
#define A_JOB_BL_CNT_REG(IP)            GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0B)
#define A_TR_BK_CNT_REG(IP)             GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0C)
#define A_RSP_SIZE_REG(IP)              GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0D)
#define A_CMD_SIZE_REG(IP)              GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0E)
#define A_CARD_WD_CNT_REG(IP)           GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x0F)
#define A_SD_MODE_REG(IP)               GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x10)
#define A_SD_CTL_REG(IP)                GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x11)
#define A_SD_STS_REG(IP)                GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x12)
#define A_EMMC_PATH_CTL_REG(IP)         GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x16)
#define A_SDIO_CTL_REG(IP)              GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x1B)
#define A_SDIO_ADDR0_REG(IP)            GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x1C)
#define A_SDIO_ADDR1_REG(IP)            GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x1D)
#define A_SDIO_STS_REG(IP)              GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x1E)
#define A_NC_REORDER_REG(IP)            GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x2D)
#define A_DAM_OFFSET_REG(IP)            GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x2E)
#define A_EMMC_BOOTCFG_REG(IP)          GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x2F)
#define A_TEST_MODE_REG(IP)             GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x30)
#define A_TEST_DEBUG1_REG(IP)           GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x31)
#define A_TEST_DEBUG2_REG(IP)           GET_CARD_REG_ADDR(A_SD_REG_POS(IP), 0x32)

#define A_CFIFO_OFFSET(IP, OFFSET)      GET_CARD_REG_ADDR(A_SD_CFIFO_POS(IP), OFFSET)
#define A_CIFD_OFFSET(IP, OFFSET)       GET_CARD_REG_ADDR(A_SD_CIFD_BANK(IP), OFFSET)


// Reg Dynamic Variable
//-----------------------------------------------------------------------------------------------------------
static RspStruct gst_RspStruct[3];
static volatile BOOL_T  gb_StopWProc[3] = {0};
static volatile BOOL_T  gb_SDIODevice[3] = {0};
static volatile U16_T   gu16_SD_MODE_DatLine[3]  = {0};
static volatile U16_T	gu16_SD_MODE_DatSync[3]  = {0};
static volatile U16_T   gu16_SD_CTL_SDIO_Det[3]  = {0};
static volatile U16_T   gu16_SD_CTL_SDIO_IntMode[3] = {0};
static volatile U16_T   gu16_SDIO_DetOn_RunOnce[3]  = {0};
static volatile U16_T   gu16_WT_NRC[3] = {0};             //Waiting Time for Nrc (us)

static volatile U8_T*   gpu8Buf[3];


// Trace Funcion
//-----------------------------------------------------------------------------------------------------------
#if (EN_TRFUNC)
	#define TR_H_SDMMC(p)	    p
#else
	#define TR_H_SDMMC(p)
#endif


// Register Operation Define ==> For Get Current Port Setting
//-----------------------------------------------------------------------------------------------------------
static U16_T _REG_GetPortSetting(IPEmType eIP)
{
	PortEmType ePort = GET_CARD_PORT(eIP);

	if( ePort == EV_PORT_SDIO1)
		return R_SDIO_SD_BUS_SW;
	else if ( ePort == EV_PORT_SDIO2 )
		return (R_SDIO_SD_BUS_SW|R_SDIO_PORT_SEL);
	else
		return 0;
}


// Register Operation Define ==> For Get Current DAT0 Value
//-----------------------------------------------------------------------------------------------------------
static U16_T _REG_GetDAT0(IPEmType eIP)
{
	PortEmType ePort = GET_CARD_PORT(eIP);

	if( ePort == EV_PORT_SDIO1)
		return (CARD_REG(A_SDIO_STS_REG(eIP)) & R_SDIO_DAT0);
	else if ( ePort == EV_PORT_SDIO2 )
		return (CARD_REG(A_SDIO_STS_REG(eIP)) & R_SDIO2_DAT0);
	else
		return (CARD_REG(A_SD_STS_REG(eIP)) & R_SD_DAT0);
}


// Register Operation Define ==> For Get Current CDET Value
//-----------------------------------------------------------------------------------------------------------
static U16_T _REG_GetCDET(IPEmType eIP)
{
	PortEmType ePort = GET_CARD_PORT(eIP);

	if( ePort == EV_PORT_SDIO1)
		return (CARD_REG(A_CARD_DET_REG(eIP)) & R_SDIO_DET_N);
	else if ( ePort == EV_PORT_SDIO2 )
		return (CARD_REG(A_CARD_DET_REG(eIP)) & R_SDIO2_DET_N);
	else
		return (CARD_REG(A_CARD_DET_REG(eIP)) & R_SD_DET_N);
}


// Register Operation Define ==> For Clean Reg and Special Case
//-----------------------------------------------------------------------------------------------------------
static RetEmType _REG_ClearSDSTS(IPEmType eIP, U8_T u8Retry)
{
	do
	{
		CARD_REG_SETBIT(A_SD_STS_REG(eIP), M_SD_ERRSTS);

		if ( gb_StopWProc[eIP] )
			return EV_FAIL;

		if ( !M_REG_STSERR(eIP) )
			return EV_OK;
		else if(!u8Retry)
			return EV_FAIL;

	} while(u8Retry--);
	return EV_FAIL;
}


// Register Operation Define ==> For Clear MIE Event
//-----------------------------------------------------------------------------------------------------------
static RetEmType _REG_ClearMIEEvent(IPEmType eIP, U8_T u8Retry)
{
	/****** Clean global MIEEvent for Interrupt ******/
#if(EN_BIND_CARD_INT)
	Hal_CARD_INT_ClearMIEEvent(eIP);
#endif

	/****** Clean MIEEvent Reg *******/
	do
	{
		CARD_REG(A_MIE_EVENT_REG(eIP)) = M_SD_MIEEVENT;

		if ( gb_StopWProc[eIP] )
			return EV_FAIL;

		if ( !M_REG_SDMIEEvent(eIP) )
			return EV_OK;
		else if(!u8Retry)
			return EV_FAIL;

	}while(u8Retry--);
	return EV_FAIL;
}


// Register Operation Define ==> For Wait DAT0 High
//-----------------------------------------------------------------------------------------------------------
static RetEmType _REG_WaitDat0HI(IPEmType eIP, U32_T u32WaitMs)
{
    U32_T u32DiffTime = 0;

	do
	{
		if ( gb_StopWProc[eIP] )
			return EV_FAIL;

		if ( _REG_GetDAT0(eIP) )
			return EV_OK;

		Hal_Timer_uDelay(1);
		u32DiffTime++;
	}while(u32DiffTime <= (u32WaitMs*1000));

	return EV_FAIL;
}


// Register Operation Define ==> For Wait FIFO Clk Ready
//-----------------------------------------------------------------------------------------------------------
static RetEmType _REG_WaitFIFOClkRdy(IPEmType eIP, CmdEmType eCmdType, U32_T u32WaitMs)
{
    U32_T u32DiffTime = 0;

	do
	{
		if ( gb_StopWProc[eIP] )
			return EV_FAIL;

		if( M_REG_FIFORDYCHK(eIP) ==  ((eCmdType>>6) & R_FIFO_CLK_RDY) )
			return EV_OK;

		Hal_Timer_uDelay(1);
		u32DiffTime++;
	}while(u32DiffTime <= (u32WaitMs*1000));
	return EV_FAIL;
}


// Register Operation Define ==> For Wait MIE Event
//-----------------------------------------------------------------------------------------------------------
static RetEmType _REG_WaitMIEEvent(IPEmType eIP, U16_T u16ReqEvent, U32_T u32WaitMs)
{
	U32_T u32DiffTime = 0;

#if(EN_BIND_CARD_INT)
	if ( Hal_CARD_INT_MIEModeRunning(eIP, EV_INT_SD) )	// Interrupt Mode
	{
		if ( !Hal_CARD_INT_WaitMIEEvent(eIP, u16ReqEvent, u32WaitMs) )
			return EV_FAIL;
		else
			return EV_OK;
	} else // Polling Mode
#endif
	{
		do
		{
			if ( gb_StopWProc[eIP] )
				return EV_FAIL;

			if ( (CARD_REG(A_MIE_EVENT_REG(eIP))&u16ReqEvent) == u16ReqEvent )
				return EV_OK;

			Hal_Timer_uDelay(1);
			u32DiffTime++;
		}while(u32DiffTime <= (u32WaitMs*1000));
		return EV_FAIL;
	}

}

// Register Operation Define ==> For Software Reset
//-----------------------------------------------------------------------------------------------------------
static void _REG_ResetIP(IPEmType eIP)
{

	CARD_REG_CLRBIT(A_MIE_PATH_CTL_REG(eIP), R_MMA_ENABLE);       //Avoid MMA_Enable lock (Auto Clear Fail)
	CARD_REG_CLRBIT(A_SD_CTL_REG(eIP), R_SD_CMD_EN|R_SD_DTRX_EN); //Avoid SD_CMD_EN/DTRX_EN lock (Auto Clear Fail)
	CARD_REG_CLRBIT(A_MMA_PRI_REG_REG(eIP), R_MIU_CLK_EN_HW);     //Avoid MIU_CLK_EN_HW lock (Auto Clear Fail)

	CARD_REG_SETBIT(A_MMA_PRI_REG_REG(eIP), R_MIU_REQ_RST);        //Avoid MIU Request lock (MIU hold)

	Hal_Timer_mDelay(1);

	CARD_REG_CLRBIT(A_TEST_MODE_REG(eIP), R_FCIE_SOFT_RST);

	Hal_Timer_mDelay(1);                                         //50x time => 2T (Max 20us)

	CARD_REG_SETBIT(A_TEST_MODE_REG(eIP), R_FCIE_SOFT_RST);

	Hal_Timer_mDelay(1);                                         //50x time => 2T (Max 20us)

	CARD_REG_CLRBIT(A_MMA_PRI_REG_REG(eIP), R_MIU_REQ_RST);
}

// Register Operation Define ==> Fix CIFD (SRAM) First Time R/W Bug
//-----------------------------------------------------------------------------------------------------------
static void _REG_CIFDEmptyRW(IPEmType eIP)
{
	U16_T u16FixSRAMBug = 0x00;

	CARD_REG(A_CIFD_OFFSET(eIP, 0)) = u16FixSRAMBug;
	CARD_REG(A_CIFD_OFFSET(eIP, 1)) = CARD_REG(A_CIFD_OFFSET(eIP, 0));
	u16FixSRAMBug = CARD_REG(A_CIFD_OFFSET(eIP, 1));

}


// IP Buffer Operation => Get Byte Value form Register
//-----------------------------------------------------------------------------------------------------------
static U8_T _BUF_GetByteFromRegAddr(volatile void *pBuf, U16_T u16Pos)
{
    if(u16Pos & 0x1)
        return CARD_REG_H8(GET_CARD_REG_ADDR(pBuf, u16Pos>>1));
    else
        return CARD_REG_L8(GET_CARD_REG_ADDR(pBuf, u16Pos>>1));
}


// IP Buffer Operation => CIFD FIFO Buffer Operation Define
//-----------------------------------------------------------------------------------------------------------
void _BUF_CIFD_DATA_IO(IPEmType eIP, CmdEmType eCmdType, volatile U16_T *pu16Buf, U8_T u8WordCnt)
{
	U8_T u8Pos = 0;

	for ( u8Pos = 0; u8Pos < u8WordCnt; u8Pos++ )
	{
		if ( eCmdType==EV_CMDREAD )
			pu16Buf[u8Pos] = CARD_REG(A_CIFD_OFFSET(eIP, u8Pos));
		else
			CARD_REG(A_CIFD_OFFSET(eIP, u8Pos)) = pu16Buf[u8Pos];
	}

}


// SDMMC Internel Logic Function
//-----------------------------------------------------------------------------------------------------------
static void _SDMMC_REG_Dump(IPEmType eIP)
{

#if (EN_DUMPREG)

	U8_T u8Pos, u8DGMode;

	prtstring("\n----------------------------------------------------\r\n");
	prtstring("  CMD_");
	prtUInt(gst_RspStruct[eIP].u8Cmd);
	prtstring(" (Arg: ");
	prtU32Hex(gst_RspStruct[eIP].u32Arg);
	prtstring(") [Line: ");
	prtUInt(gst_RspStruct[eIP].u32ErrLine);
	prtstring("]\r\n");

	prtstring("----------------------------------------------------\r\n");

	for(u8Pos = 0; u8Pos < gst_RspStruct[eIP].u8RspSize; u8Pos++)
	{
		if( (u8Pos == 0) || (u8Pos == 8) )
			prtstring("[");

		prtU8Hex(_BUF_GetByteFromRegAddr((volatile void *)A_SD_CFIFO_POS(eIP), u8Pos));
		prtstring(",");

		if( (u8Pos == 7) || (u8Pos == (gst_RspStruct[eIP].u8RspSize-1)) )
			prtstring("]\n");
	}

	prtstring("---------------DumpReg------------------------------\r\n");

	prtstring("[0x0A][MIE_PATH_CTL_REG]=    ");
	prtU16Hex(CARD_REG(A_MIE_PATH_CTL_REG(eIP)));
	prtstring("\r\n");

	prtstring("[0x10][SD_MODE_REG]=         ");
	prtU16Hex(CARD_REG(A_SD_MODE_REG(eIP)));
	prtstring("\r\n");

	prtstring("[0x11][SD_CTL_REG]=          ");
	prtU16Hex(CARD_REG(A_SD_CTL_REG(eIP)));
	prtstring("\r\n");

	prtstring("[0x12][SD_STS_REG]=          ");
	prtU16Hex(CARD_REG(A_SD_STS_REG(eIP)));
	prtstring("\r\n");

	if(GET_CARD_PORT(eIP) != EV_PORT_SD)
	{
		prtstring("[0x1E][SDIO_STS_REG]=        ");
		prtU16Hex(CARD_REG(A_SDIO_STS_REG(eIP)));
		prtstring("\r\n");
	}

#if(EN_BIND_CARD_INT)
	if ( Hal_CARD_INT_MIEModeRunning(eIP, EV_INT_SD) )	// Interrupt Mode
	{
		prtstring("[gu16_MIEEvent_ForInt]=      ");
		prtU16Hex(Hal_CARD_INT_GetMIEEvent(eIP));
		prtstring("\r\n");
	}
#endif

	prtstring("[0x01][MIE_EVENT_REG]=       ");
	prtU16Hex(CARD_REG(A_MIE_EVENT_REG(eIP)));
	prtstring("\r\n");


	prtstring("[0x02][MMA_PRI_REG_REG]=     ");
	prtU16Hex(CARD_REG(A_MMA_PRI_REG_REG(eIP)));
	prtstring("\r\n");

#if (EN_BYTEMODE)
	prtstring("[0x1D][SDIO_ADDR1_REG]=      ");
	prtU16Hex(CARD_REG(A_SDIO_ADDR1_REG(eIP)));
	prtstring("\r\n");

	prtstring("[0x1C][SDIO_ADDR0_REG]=      ");
	prtU16Hex(CARD_REG(A_SDIO_ADDR0_REG(eIP)));
	prtstring("\r\n");

	prtstring("[0x1B][SDIO_CTL_REG]=        ");
	prtU16Hex(CARD_REG(A_SDIO_CTL_REG(eIP)));
	prtstring("\r\n");
#else
	prtstring("[0x03][MIU_DMA1_REG]=        ");
	prtU16Hex(CARD_REG(A_MIU_DMA1_REG(eIP)));
	prtstring("\r\n");

	prtstring("[0x04][MIU_DMA0_REG]=        ");
	prtU16Hex(CARD_REG(A_MIU_DMA0_REG(eIP)));
	prtstring("\r\n");

	prtstring("[0x2E][DAM_OFFSET_REG]=	    ");
	prtU16Hex(CARD_REG(A_DAM_OFFSET_REG(eIP)));
	prtstring("\r\n");
#endif

	prtstring("[0x0B][JOB_BL_CNT_REG]=      ");
	prtU16Hex(CARD_REG(A_JOB_BL_CNT_REG(eIP)));
	prtstring("\r\n");

	prtstring("[0x0C][TR_BK_CNT_REG]=       ");
	prtU16Hex(CARD_REG(A_TR_BK_CNT_REG(eIP)));
	prtstring("\r\n");

	prtstring("[0x0F][CARD_WD_CNT_REG]=     ");
	prtU16Hex(CARD_REG(A_CARD_WD_CNT_REG(eIP)));
	prtstring("\r\n");

	prtstring("[0x31][SDMMC_DEBUG1]=        ");
	for(u8DGMode = 1; u8DGMode <=7; u8DGMode++)
	{
		CARD_REG_CLRBIT(A_TEST_MODE_REG(eIP), R_DEBUG_MODE0 | R_DEBUG_MODE1 | R_DEBUG_MODE2);
		CARD_REG_SETBIT(A_TEST_MODE_REG(eIP), (u8DGMode<<8));
		prtU16Hex(CARD_REG(A_TEST_DEBUG1_REG(eIP)));
		prtstring(", ");
	}
	prtstring("\r\n");
	prtstring("[0x32][SDMMC_DEBUG2]=        ");

	for(u8DGMode = 1; u8DGMode <=7; u8DGMode++)
	{
		CARD_REG_CLRBIT(A_TEST_MODE_REG(eIP), R_DEBUG_MODE0 | R_DEBUG_MODE1 | R_DEBUG_MODE2);
		CARD_REG_SETBIT(A_TEST_MODE_REG(eIP), (u8DGMode<<8));
		prtU16Hex(CARD_REG(A_TEST_DEBUG2_REG(eIP)));
		prtstring(", ");

	}
	prtstring("\r\n");

	prtstring("----------------------------------------------------\r\n");

#endif  //End #if(EN_DUMPREG)

}


static RspErrEmType _SDMMC_EndProcess(IPEmType eIP, CmdEmType eCmdType, RspErrEmType eRspErr, BOOL_T bCloseClk, int Line)
{
	U16_T u16RspErr = (U16_T)eRspErr;
	U16_T u16IPErr = EV_STS_RIU_ERR | EV_STS_MIE_TOUT | EV_STS_FIFO_NRDY | EV_STS_DAT0_BUSY;

	/****** (1) Record Information *******/
	gst_RspStruct[eIP].u32ErrLine = (U32_T)Line;
	gst_RspStruct[eIP].u8RspSize = (U8_T)CARD_REG(A_RSP_SIZE_REG(eIP));
	gst_RspStruct[eIP].eErrCode = eRspErr;

	/****** (2) Dump and the Reg Info + Reset IP *******/

	if ( u16RspErr && gb_StopWProc[eIP] )
	{
		eRspErr = EV_SWPROC_ERR;
		_REG_ResetIP(eIP);
	}
	else if( u16RspErr & u16IPErr ) //SD IP Error
	{
		_SDMMC_REG_Dump(eIP);
		_REG_ResetIP(eIP);
	}
	else if( u16RspErr & M_SD_ERRSTS ) //SD_STS Reg Error
	{
		//Do Nothing
	}

	/****** (3) Close clock and DMA Stop function ******/
	if(bCloseClk && !gb_SDIODevice[eIP])
		CARD_REG_CLRBIT(A_SD_MODE_REG(eIP), R_SD_CLK_EN|R_SD_DMA_RD_CLK_STOP);

	/****** (4) Clock MIU Clock (SW) when SW open or not *******/
	CARD_REG_CLRBIT(A_MMA_PRI_REG_REG(eIP), R_MIU_CLK_EN_SW);

	return eRspErr;
}

//***********************************************************************************************************
// SDMMC HAL Function
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_SetSDIODevice
 *     @author jeremy.wang (2014/2/13)
 * Desc:
 *
 * @param eIP :
 * @param bEnable :
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetSDIODevice(IPEmType eIP, BOOL_T bEnable)
{
	if(bEnable)
		gb_SDIODevice[eIP] = TRUE;
	else
		gb_SDIODevice[eIP] = FALSE;

}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_SetSDIOIntDet
 *     @author jeremy.wang (2012/2/20)
 * Desc:
 *
 * @param eIP :
 * @param bEnable :
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetSDIOIntDet(IPEmType eIP, BOOL_T bEnable)
{

	if(gb_SDIODevice[eIP])
	{
		if(bEnable)
		{
			if(!gu16_SDIO_DetOn_RunOnce[eIP])
			{
				gu16_SDIO_DetOn_RunOnce[eIP] = TRUE;  // For IP Bug, We just Run Once to avoid two thread use gu16_SD_CTL_SDIO_Det
				gu16_SD_CTL_SDIO_Det[eIP] = R_SDIO_DET_ON ; //Always On !! for IP Design Limitation
			}
		}

		#if(EN_BIND_CARD_INT)
				Hal_CARD_INT_SetMIEIntEn_ForSDIO(eIP, EV_INT_SD, bEnable);
		#endif
	}

}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_SetDataWidth
 *     @author jeremy.wang (2011/11/28)
 * Desc: According as Data Bus Width to Set IP DataWidth
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param eBusWidth : 1BIT/4BITs/8BITs
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetDataWidth(IPEmType eIP, SDMMCBusWidthEmType eBusWidth)
{
	gu16_SD_MODE_DatLine[eIP] = (U16_T)eBusWidth;

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_SetBusTiming
*     @author jeremy.wang (2015/7/29)
* Desc:
*
* @param eIP : FCIE1/FCIE2/...
* @param eBusTiming : LOW/DEF/HS/SDR12/DDR...
----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetBusTiming(IPEmType eIP, BusTimingEmType eBusTiming)
{

	switch ( eBusTiming )
	{
		case EV_BUS_LOW:
			gu16_SD_MODE_DatSync[eIP] = R_SD_DATSYNC;
			break;
		case EV_BUS_DEF:
			gu16_SD_MODE_DatSync[eIP] = 0;
			break;
		case EV_BUS_HS:
			gu16_SD_MODE_DatSync[eIP] = R_SD_DATSYNC;
			break;
		case EV_BUS_SDR12:
			break;
		case EV_BUS_SDR25:
			break;
		case EV_BUS_SDR50:
			break;
		case EV_BUS_SDR104:
			break;
		case EV_BUS_DDR50:
			break;
		case EV_BUS_HS200:
			break;

	}


}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_SetNrcDelay
*     @author jeremy.wang (2015/8/18)
* Desc: According as Current Clock to Set Nrc Delay
*
* @param eIP : FCIE1/FCIE2/...
* @param u32RealClk : Real Clock
----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetNrcDelay(IPEmType eIP, U32_T u32RealClk)
{

	if( u32RealClk >= 8000000 )			//>=8MHz
		gu16_WT_NRC[eIP] = 1;
	else if( u32RealClk >= 4000000 )	//>=4MHz
		gu16_WT_NRC[eIP] = 2;
	else if( u32RealClk >= 2000000 )	//>=2MHz
		gu16_WT_NRC[eIP] = 4;
	else if( u32RealClk >= 1000000 )	//>=1MHz
		gu16_WT_NRC[eIP] = 8;
	else if( u32RealClk >= 400000 )     //>=400KHz
		gu16_WT_NRC[eIP] = 20;
	else if( u32RealClk >= 300000 )	    //>=300KHz
		gu16_WT_NRC[eIP] = 27;
	else if( u32RealClk >= 100000 )     //>=100KHz
		gu16_WT_NRC[eIP] = 81;
	else if(u32RealClk==0)
		gu16_WT_NRC[eIP] = 100;         //Add Default Value (100us)
}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_SetCmdToken
*     @author jeremy.wang (2015/10/12)
* Desc: Set Cmd Token
*
* @param eIP : FCIE1/FCIE2/...
* @param u8Cmd : SD Command
* @param u32Arg : SD Argument
----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetCmdToken(IPEmType eIP, U8_T u8Cmd, U32_T u32Arg)
{
	gst_RspStruct[eIP].u8Cmd  = u8Cmd;
	gst_RspStruct[eIP].u32Arg = u32Arg;

	CARD_REG(A_CFIFO_OFFSET(eIP, 0)) = (((U8_T)(u32Arg>>24))<<8) | (0x40 + u8Cmd);
	CARD_REG(A_CFIFO_OFFSET(eIP, 1)) = (((U8_T)(u32Arg>>8))<<8) | ((U8_T)(u32Arg>>16));
	CARD_REG(A_CFIFO_OFFSET(eIP, 2)) = (U8_T)u32Arg;

	TR_H_SDMMC(prtstring("[S_")); TR_H_SDMMC(prtUInt(eIP));
	TR_H_SDMMC(prtstring("] CMD_")); TR_H_SDMMC(prtUInt(u8Cmd));

	TR_H_SDMMC(prtstring(" ("));
	TR_H_SDMMC(prtU32Hex(u32Arg));
	TR_H_SDMMC(prtstring(")"));


}


/*----------------------------------------------------------------------------------------------------------
*
* Function: Hal_SDMMC_GetRspToken
*     @author jeremy.wang (2015/7/9)
* Desc: Get Command Response Info
*
* @param eIP : FCIE1/FCIE2/...
*
* @return RspStruct*  : Response Struct
----------------------------------------------------------------------------------------------------------*/
RspStruct* Hal_SDMMC_GetRspToken(IPEmType eIP)
{
	U8_T u8Pos;

	TR_H_SDMMC(prtstring(" =>Rsp: ("));
	TR_H_SDMMC(prtU32Hex((U16_T)gst_RspStruct[eIP].eErrCode));
	TR_H_SDMMC(prtstring(")\r\n"));

	for(u8Pos=0; u8Pos<0x10; u8Pos++ )
		gst_RspStruct[eIP].u8ArrRspToken[u8Pos] = 0;

	TR_H_SDMMC(prtstring("["));

	for(u8Pos=0; u8Pos < gst_RspStruct[eIP].u8RspSize; u8Pos++)
	{
		gst_RspStruct[eIP].u8ArrRspToken[u8Pos] =  _BUF_GetByteFromRegAddr((volatile void *)A_SD_CFIFO_POS(eIP), u8Pos);

		TR_H_SDMMC(prtU8Hex(gst_RspStruct[eIP].u8ArrRspToken[u8Pos]));
		TR_H_SDMMC(prtstring(", "));

	}
	TR_H_SDMMC(prtstring("]"));

	TR_H_SDMMC(prtstring("\r\n\r\n"));

	return &gst_RspStruct[eIP];
}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_TransCmdSetting
 *     @author jeremy.wang (2011/12/1)
 * Desc: For Data Transfer Setting
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param eTransType : CIFD/DMA/NONE
 * @param u16BlkCnt : Block Cnt
 * @param u16BlkSize : Block Size
 * @param u32BufAddr : Memory Address (32bits)
 * @param pu8Buf : If run CIFD, it neet the buf pointer to do io between CIFD and Buf
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_TransCmdSetting(IPEmType eIP, TransEmType eTransType, U16_T u16BlkCnt, U16_T u16BlkSize, volatile U32_T u32BufAddr,  volatile U8_T *pu8Buf)
{

#if(!EN_BYTEMODE)
	#if (D_MIU_WIDTH==4)
	U8_T u8BitMask[8]={0x0F,0x0E,0x0C,0x08};
	#else
	U8_T u8BitMask[8]={0xFF,0xFE,0xFC,0xF8,0xF0,0xE0,0xC0,0x80};
	#endif
#endif

	if( eTransType == EV_DMA )
	{
		/***** Block Count ******/
		CARD_REG(A_JOB_BL_CNT_REG(eIP)) =  u16BlkCnt;

		/****** Block Size and DMA Addr******/
#if(EN_BYTEMODE)
		CARD_REG(A_SDIO_ADDR1_REG(eIP)) = (U16_T)(u32BufAddr>>16);
		CARD_REG(A_SDIO_ADDR0_REG(eIP)) = (U16_T)(u32BufAddr);
		CARD_REG(A_SDIO_CTL_REG(eIP)) = u16BlkSize | R_SDIO_BLK_MOD;
#else
		CARD_REG(A_MIU_DMA1_REG(eIP)) = (U16_T)(((u32BufAddr&0xFFFFFFF)/D_MIU_WIDTH)>>16);
		CARD_REG(A_MIU_DMA0_REG(eIP)) = (U16_T)((u32BufAddr)/D_MIU_WIDTH);
		CARD_REG(A_DAM_OFFSET_REG9(eIP)) = u8BitMask[u32BufAddr&(D_MIU_WIDTH-1)];
		CARD_REG(A_SDIO_CTL_REG(eIP)) = 0x00;
#endif

		CARD_REG(A_CARD_WD_CNT_REG(eIP)) = 0x00;

	}
	else //CIFD
	{
		if( (u16BlkSize>=512) || (u16BlkSize==0) )
			CARD_REG(A_CARD_WD_CNT_REG(eIP)) = 0;
		else
		{
			CARD_REG(A_CARD_WD_CNT_REG(eIP)) = (u16BlkSize/2);	//Word Count (Max: 256Words==>512Bytes)
		}

		CARD_REG(A_SDIO_CTL_REG(eIP)) = 0x00;

		gpu8Buf[eIP] = pu8Buf;

	}


}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_SendCmdAndWaitProcess
 *     @author jeremy.wang (2011/11/28)
 * Desc: Send CMD and Wait Process
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param eTransType : CIFD/DMA/NONE
 * @param eCmdType : CMDRSP/READ/WRITE
 * @param eRspType : R1/R2/R3/...
 * @param bCloseClk : Close Clock or not
 *
 * @return RspErrEmType  : Response Error Code
 ----------------------------------------------------------------------------------------------------------*/
RspErrEmType Hal_SDMMC_SendCmdAndWaitProcess(IPEmType eIP, TransEmType eTransType, CmdEmType eCmdType, SDMMCRspEmType eRspType, BOOL_T bCloseClk)
{
	U32_T u32WaitMS	= WT_EVENT_RSP;
	U16_T u16WaitMIEEvent = R_SD_CMD_END;
	U16_T u16MIE_PATH_MMA_EN = (U8_T)eTransType;
	U16_T u16MIE_TRANS_END = (u16MIE_PATH_MMA_EN<<14) | (u16MIE_PATH_MMA_EN<<11) | ((!u16MIE_PATH_MMA_EN)<<2);
	U16_T u16VAR_SD_MODE = _REG_GetPortSetting(eIP) | gu16_SD_MODE_DatLine[eIP] | gu16_SD_MODE_DatSync[eIP] ;

#if (EN_MIU_WDEN_PATCH_DIS)
	CARD_REG_SETBIT(A_NC_REORDER_REG(eIP), R_MIU_WDEN_PATCH_DIS);
#endif

	CARD_REG(A_RSP_SIZE_REG(eIP)) = V_RSP_SIZE_INIT | ((U8_T)eRspType);
	CARD_REG(A_CMD_SIZE_REG(eIP)) = V_CMD_SIZE_INIT;
	CARD_REG(A_MIE_PATH_CTL_REG(eIP)) = V_MIE_PATH_INIT;
	CARD_REG(A_EMMC_PATH_CTL_REG(eIP)) = 0;
	CARD_REG(A_SD_MODE_REG(eIP)) = V_SD_MODE_INIT | u16VAR_SD_MODE | (eTransType>>8) | (gb_SDIODevice[eIP] ? 0 : (u16MIE_PATH_MMA_EN<<11));
	CARD_REG(A_SD_CTL_REG(eIP))  = V_SD_CTL_INIT | gu16_SD_CTL_SDIO_Det[eIP] | gu16_SD_CTL_SDIO_IntMode[eIP] | (eRspType>>12) | (eCmdType>>4);

	CARD_REG(A_MMA_PRI_REG_REG(eIP)) = V_MMA_PRI_INIT | (eCmdType>>6);
	CARD_REG_SETBIT(A_EMMC_BOOTCFG_REG(eIP), V_SD_BOOTCFG_INIT);

	Hal_Timer_uDelay(gu16_WT_NRC[eIP]);

	if ( _REG_ClearSDSTS(eIP, RT_CLEAN_SDSTS) || _REG_ClearMIEEvent(eIP, RT_CLEAN_MIEEVENT) )
		return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_RIU_ERR, bCloseClk, __LINE__);

	if ( (eTransType== EV_DMA) && _REG_WaitFIFOClkRdy(eIP, eCmdType, WT_FIFO_CLK_RDY) )
		return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_FIFO_NRDY, bCloseClk, __LINE__);

/*#if(EN_BIND_CARD_INT)
	Hal_CARD_INT_SetMIEIntEn(eIP, EV_INT_SD, V_MIE_INT_EN_INIT | u16MIE_TRANS_END);
#endif*/

	if( eCmdType==EV_CMDREAD )
	{
		u16WaitMIEEvent |= u16MIE_TRANS_END;
		u32WaitMS |= WT_EVENT_READ;
		CARD_REG_SETBIT(A_MIE_PATH_CTL_REG(eIP), u16MIE_PATH_MMA_EN);
		CARD_REG_SETBIT(A_SD_CTL_REG(eIP), (R_SD_CMD_EN | R_SD_DTRX_EN) );
	}
	else
		CARD_REG_SETBIT(A_SD_CTL_REG(eIP), R_SD_CMD_EN);

	if ( _REG_WaitMIEEvent(eIP, u16WaitMIEEvent, u32WaitMS) )
		return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_MIE_TOUT, bCloseClk, __LINE__);

    //====== Trans Data between CIFD and Buf ======
	if( eTransType==EV_CIF )
		_BUF_CIFD_DATA_IO(eIP, eCmdType, (volatile U16_T *)gpu8Buf[eIP],  (U8_T)CARD_REG(A_CARD_WD_CNT_REG(eIP)) );

	if ( (eRspType == EV_R1B) && _REG_WaitDat0HI(eIP, WT_DAT0HI_END) )
		return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_DAT0_BUSY, bCloseClk, __LINE__);
	else if( (eRspType == EV_R3) || (eRspType == EV_R4) )  // For IP CRC bug
		CARD_REG(A_SD_STS_REG(eIP)) = R_SD_CMDRSP_CERR; //Clear CMD CRC Error


	if( (eCmdType==EV_CMDWRITE) && (!M_REG_STSERR(eIP)) )
	{
		if ( _REG_ClearSDSTS(eIP, RT_CLEAN_SDSTS) || _REG_ClearMIEEvent(eIP, RT_CLEAN_MIEEVENT) )
			return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_RIU_ERR, bCloseClk, __LINE__);

		CARD_REG_SETBIT(A_MIE_PATH_CTL_REG(eIP), u16MIE_PATH_MMA_EN);
		CARD_REG_SETBIT(A_SD_CTL_REG(eIP), R_SD_DTRX_EN);

		if ( _REG_WaitMIEEvent(eIP, u16MIE_TRANS_END, WT_EVENT_WRITE) )
			return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_MIE_TOUT, bCloseClk, __LINE__);

		if ( (eTransType== EV_CIF) && _REG_WaitDat0HI(eIP, WT_DAT0HI_END) )
			return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_DAT0_BUSY, bCloseClk, __LINE__);

	}

	return _SDMMC_EndProcess(eIP, eCmdType, (RspErrEmType)M_REG_STSERR(eIP), bCloseClk, __LINE__);
}


/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_RunBrokenDmaAndWaitProcess
 *     @author jeremy.wang (2011/12/1)
 * Desc: For Broken DMA Data Transfer
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param eCmdType : READ/WRITE
 *
 * @return RspErrEmType  : Response Error Code
 ----------------------------------------------------------------------------------------------------------*/
RspErrEmType Hal_SDMMC_RunBrokenDmaAndWaitProcess(IPEmType eIP, CmdEmType eCmdType)
{
	U32_T u32WaitMS	= 0;

	if(eCmdType==EV_CMDREAD)
		u32WaitMS = WT_EVENT_READ;
	else if(eCmdType==EV_CMDWRITE)
		u32WaitMS = WT_EVENT_WRITE;

	if ( _REG_ClearMIEEvent(eIP, RT_CLEAN_MIEEVENT) )
		return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_RIU_ERR, FALSE, __LINE__);

	if (_REG_WaitFIFOClkRdy(eIP, eCmdType, WT_FIFO_CLK_RDY))
		return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_FIFO_NRDY, FALSE, __LINE__);

	CARD_REG_SETBIT(A_MIE_PATH_CTL_REG(eIP), R_MMA_ENABLE);
	CARD_REG_SETBIT(A_SD_CTL_REG(eIP), R_SD_DTRX_EN);

	if ( _REG_WaitMIEEvent(eIP, R_CARD_DMA_END|R_MMA_LSDONE_END, u32WaitMS) )
		return _SDMMC_EndProcess(eIP, eCmdType, EV_STS_MIE_TOUT, FALSE, __LINE__);

	return _SDMMC_EndProcess(eIP, eCmdType, (RspErrEmType)M_REG_STSERR(eIP), FALSE, __LINE__);

}


ErrGrpEmType Hal_SDMMC_ErrGroup(RspErrEmType eErrType)
{

	switch((U16_T)eErrType)
	{
		case EV_STS_OK:
			return EV_EGRP_OK;

		case EV_STS_NORSP:
		case EV_STS_RIU_ERR:
		case EV_STS_MIE_TOUT:
		case EV_STS_FIFO_NRDY:
		case EV_STS_DAT0_BUSY:
			return EV_EGRP_TOUT;

		case EV_STS_DAT_CERR:
		case EV_STS_DAT_STSERR:
		case EV_STS_DAT_STSNEG:
		case EV_STS_RSP_CERR:
			return EV_EGRP_COMM;

		default:
			return EV_EGRP_OTHER;
	}


}



/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_SetMIEIntEn
 *     @author jeremy.wang (2014/5/28)
 * Desc:
 *
 * @param eIP :
 * @param eTransType :
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetMIEIntEn(IPEmType eIP, TransEmType eTransType)
{

#if(EN_BIND_CARD_INT)
	U16_T u16MIE_PATH_MMA_EN = (U8_T)eTransType;
	U16_T u16MIE_TRANS_END = (u16MIE_PATH_MMA_EN<<14) | (u16MIE_PATH_MMA_EN<<11) | ((!u16MIE_PATH_MMA_EN)<<2);
	Hal_CARD_INT_SetMIEIntEn(eIP, EV_INT_SD, V_MIE_INT_EN_INIT | u16MIE_TRANS_END);
#endif

}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_SetSDIOIntSetting
 *     @author jeremy.wang (2014/5/29)
 * Desc:
 *
 * @param eIP :
 * @param u8Cmd :
 * @param u32Arg :
 * @param eCmdType :
 * @param u16BlkCnt :
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetSDIOIntBeginSetting(IPEmType eIP, U8_T u8Cmd, U32_T u32Arg, CmdEmType eCmdType, U16_T u16BlkCnt)
{
	BOOL_T bSDIOAbortMode = (u8Cmd == 52) && ( (u32Arg & 0x83FFFE00) == 0x80000C00);

	gu16_SD_CTL_SDIO_IntMode[eIP] = 0;

	if(gb_SDIODevice[eIP])
	{
		if(eCmdType != EV_CMDRSP)
		{
			if(u16BlkCnt == 1)
				gu16_SD_CTL_SDIO_IntMode[eIP] = R_SDIO_INT_MOD1;
			else
			{
				//gu16_SD_CTL_SDIO_IntMode[eIP] = R_SDIO_INT_MOD0 | R_SDIO_INT_MOD1;
				/****** IP Bug, We could not use Multiple block Mode *******/
				gu16_SD_CTL_SDIO_Det[eIP] = 0;
			}
		}
		else if ( (u8Cmd == 12) || bSDIOAbortMode)
		{
			gu16_SD_CTL_SDIO_IntMode[eIP] = R_SDIO_INT_MOD0;
		}
	}

}



/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_SetSDIOIntEndSetting
 *     @author jeremy.wang (2014/5/29)
 * Desc:
 *
 * @param eIP :
 * @param eErr :
 * @param u16BlkCnt :
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_SetSDIOIntEndSetting(IPEmType eIP, RspErrEmType eRspErr, U16_T u16BlkCnt)
{
	U16_T u16RspErr = (U16_T)eRspErr;

	if(gb_SDIODevice[eIP])
	{
		if(u16BlkCnt>1)
		{
			/****** IP Bug, We could not use Multiple block Mode *******/
			gu16_SD_CTL_SDIO_Det[eIP] = R_SDIO_DET_ON;
			CARD_REG_SETBIT(A_SD_CTL_REG(eIP), R_SDIO_DET_ON);
		}

		if(u16RspErr>0)
		{
			CARD_REG_CLRBIT(A_SD_CTL_REG(eIP), R_SDIO_INT_MOD0 | R_SDIO_INT_MOD1);
		}
	}
}



/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_WriteProtect
 *     @author jeremy.wang (2011/12/1)
 * Desc: Check Card Write Protect
 *
 * @param eIP : FCIE1/FCIE2/...
 *
 * @return BOOL_T  : Write Protect or not
 ----------------------------------------------------------------------------------------------------------*/
BOOL_T Hal_SDMMC_WriteProtect(IPEmType eIP)
{
	CARD_REG(A_MIE_PATH_CTL_REG(eIP)) = V_MIE_PATH_INIT;
	CARD_REG(A_SD_MODE_REG(eIP)) = _REG_GetPortSetting(eIP);

	if ( CARD_REG(A_SD_STS_REG(eIP)) & R_SD_WR_PRO_N )
		return (FALSE);  //No Write Protect
	return (TRUE);

}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_CardDetect
 *     @author jeremy.wang (2011/10/11)
 * Desc: Check Card Detection
 *
 * @param eIP : FCIE1/FCIE2/...
 *
 * @return BOOL_T  : Card Detection or not
 ----------------------------------------------------------------------------------------------------------*/
BOOL_T Hal_SDMMC_CardDetect(IPEmType eIP)
{
	CARD_REG_SETBIT(A_MIE_PATH_CTL_REG(eIP), V_MIE_PATH_INIT);
	CARD_REG_SETBIT(A_SD_MODE_REG(eIP), _REG_GetPortSetting(eIP));

	if ( _REG_GetCDET(eIP) )
		return (FALSE);  //No Card Detect
	return (TRUE);
}

BOOL_T Hal_SDMMC_OtherPreUse(IPEmType eIP)
{
	if(CARD_REG(A_EMMC_PATH_CTL_REG(eIP)) & R_EMMC_EN) //Emmc Path
		return (TRUE);

	if(!(CARD_REG(A_MIE_PATH_CTL_REG(eIP)) & R_SD_EN)) //Not SD Path
		return (TRUE);

	return (FALSE);
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_ClkCtrl
 *     @author jeremy.wang (2011/12/14)
 * Desc: OpenCard Clk for Special Request (We always Open/Close Clk by Every Cmd)
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param bOpen : Open Clk or not
 * @param u16DelayMs :  Delay ms to Specail Purpose
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_ClkCtrl(IPEmType eIP, BOOL_T bOpen, U16_T u16DelayMs)
{
	CARD_REG(A_MIE_PATH_CTL_REG(eIP)) = V_MIE_PATH_INIT;
	CARD_REG(A_SD_MODE_REG(eIP)) = _REG_GetPortSetting(eIP);

	if( bOpen )
		CARD_REG_SETBIT(A_SD_MODE_REG(eIP), R_SD_CLK_EN );
	else
		CARD_REG_CLRBIT(A_SD_MODE_REG(eIP), R_SD_CLK_EN );

    Hal_Timer_mDelay(u16DelayMs);
}


//***********************************************************************************************************
// SD Other Setting
//***********************************************************************************************************

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_Reset
 *     @author jeremy.wang (2011/11/30)
 * Desc: Reset IP to avoid IP Dead + Touch CIFD first time to avoid latch issue
 *
 * @param eIP :  FCIE1/FCIE2/...
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_Reset(IPEmType eIP)
{
	_REG_ResetIP(eIP);
	_REG_CIFDEmptyRW(eIP);
}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_WaitProcessCtrl
 *     @author jeremy.wang (2011/12/1)
 * Desc: Stop Process to Avoid Long Time Waiting
 *
 * @param eIP : FCIE1/FCIE2/...
 * @param bStop : Stop Process or not
 ----------------------------------------------------------------------------------------------------------*/
void Hal_SDMMC_WaitProcessCtrl(IPEmType eIP, BOOL_T bStop)
{
	gb_StopWProc[eIP] = bStop;

#if(EN_BIND_CARD_INT)
	if ( gb_StopWProc[eIP] )
		Hal_CARD_INT_WaitMIEEventCtrl(eIP, TRUE);
	else
		Hal_CARD_INT_WaitMIEEventCtrl(eIP, FALSE);
#endif

}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_GetDATBusLevel
 *     @author jeremy.wang (2014/5/22)
 * Desc:
 *
 * @param eIP :
 *
 * @return U8_T  :
 ----------------------------------------------------------------------------------------------------------*/
U8_T Hal_SDMMC_GetDATBusLevel(IPEmType eIP)
{
	U16_T u16Temp = 0;
	if(GET_CARD_PORT(eIP) != EV_PORT_SD)
	{
		u16Temp = CARD_REG(A_SDIO_STS_REG(eIP)) & 0x000F;
		return (U8_T)(u16Temp);
	}
	else
	{
		u16Temp = CARD_REG(A_SD_STS_REG(eIP)) & 0x0F00;
		return (U8_T)(u16Temp>>8);
	}

}

/*----------------------------------------------------------------------------------------------------------
 *
 * Function: Hal_SDMMC_GetMIEEvent
 *     @author jeremy.wang (2014/6/10)
 * Desc:
 *
 * @param eIP :
 *
 * @return U16_T  :
 ----------------------------------------------------------------------------------------------------------*/
U16_T Hal_SDMMC_GetMIEEvent(IPEmType eIP)
{
	return CARD_REG(A_MIE_EVENT_REG(eIP));
}















