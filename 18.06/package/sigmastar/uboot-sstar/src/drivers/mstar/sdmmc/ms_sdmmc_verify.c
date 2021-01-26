/*
* ms_sdmmc_verify.c- Sigmastar
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
 * FileName ms_sdmmc_verify.c
 *     @author jeremy.wang (2015/10/01)
 * Desc:
 * 	   This layer is between UBOOT Common API layer and SDMMC Driver layer.
 * 	   (1) The goal is we could modify any verification flow but don't modify any sdmmc driver code.
 * 	   (2) Timer Test, PAD Test, Init Test, CIFD/DMA Test, Burning Test
 *
 ***************************************************************************************************************/

#include "inc/ms_sdmmc_verify.h"
#include "inc/ms_sdmmc_drv.h"
#include "inc/hal_card_timer.h"


//***********************************************************************************************************
// Config Setting (Internel)
//***********************************************************************************************************
#define D_DUMPCOMP					(TRUE)

#define BURN_START_SECTOR			30000
#define BURN_END_SECTOR				70000

#define BURN_CLK_CHG_CNT            120
#define BURN_CLK_CHG_LVL            3

#define BURN_PAT_CHG_CNT            20
#define BURN_PAT_CHG_LVL            5

static U8_T  gu8Pattern=0;
static U16_T   gu16Ret1=0, gu16Ret2=0, gu16Ret3=0;

#define D_TIMERTEST         "(SDMMC TimerTest)   "
#define D_SETPAD	        "(SDMMC SetPAD)      "
#define D_DAT1DET	        "(SDMMC DAT1Det)     "
#define D_CARDDET	        "(SDMMC CardDet)     "
#define D_SDMMCINIT		    "(SDMMC Init)        "
#define D_CIFDWRITE		    "(SDMMC CIFD W)      "
#define D_CIFDREAD		    "(SDMMC CIFD R)      "
#define D_DMAWRITE		    "(SDMMC DMA W)       "
#define D_DMAREAD		    "(SDMMC DMA R)       "
#define D_ADMAWRITE		    "(SDMMC ADMA W)      "
#define D_ADMAREAD		    "(SDMMC ADMA R)      "
#define D_COMPARE		    "(COMPARE CONTENT)   "
#define D_WIDEBUS		    "(SDMMC SetWideBus)  "
#define D_HIGHBUS           "(SDMMC SetHighBus)  "
#define D_SETBUSCLK         "(SDMMC SetBusClock) "
#define D_SETBUSTIMING      "(SDMMC SetBusTiming)"
#define D_TESTPATT          "(SDMMC TestPattern) "
#define D_PSINIT		    "(SDMMC PS Init)     "
#define D_PSTEST            "(SDMMC PS Test)     "


//----------------------------------------------------------------------------------------------------------
void _PRT_LINE(BOOL_T bHidden)
{
	if(!bHidden)
		prtstring("\r\n========================================================================================\r\n");

}

//----------------------------------------------------------------------------------------------------------
void _PRT_ENTER(BOOL_T bHidden)
{
	if(!bHidden)
		prtstring("\r\n");

}

//----------------------------------------------------------------------------------------------------------
void _PRT_MSG(U8_T u8Slot, char arrTestName[], U16_T u16Err, BOOL_T bHidden)
{
	if(u16Err)
	{
		prtstring("[sdmmc_"); prtUInt(u8Slot);	prtstring("]  ");
		prtstring(arrTestName); prtstring(" ........ ");
		prtstring("(FAIL)= "); prtU16Hex(u16Err); prtstring("\n");
	}
	else if(!bHidden)
	{
		prtstring("[sdmmc_"); prtUInt(u8Slot);	prtstring("]  ");
		prtstring(arrTestName); prtstring(" ........ ");
		prtstring("(PASS) "); prtstring("\n");
	}

}

void _PRT_VALUE(U8_T u8Slot, char arrTestName[], U32_T u32Value, BOOL_T bHidden)
{
	if(!bHidden)
	{
		prtstring("[sdmmc_"); prtUInt(u8Slot);	prtstring("]  ");
		prtstring(arrTestName);  prtstring(" ........ (");
		prtUInt(u32Value);  prtstring(")\n");
	}

}


void _DUMP_MEM_ARR(volatile U8_T* u8Arr, U16_T u16Size)
{
   unsigned short u16Pos;

   prtstring("\r\n---------------------------------------------------------------------------------------------\r\n");

   for(u16Pos=0; u16Pos<u16Size; u16Pos++)
   {
	   if(u16Pos%16==0 && u16Pos!=0)
		   prtstring("\r\n");

	   prtstring("(");
	   prtU8(u8Arr[u16Pos]);
	   prtstring(")");
   }
   prtstring("\r\n----------------------------------------------------------------------------------------------\r\n");

}


U16_T _MEM_COMPARE(volatile U8_T * u8Arr1, volatile U8_T * u8Arr2, U16_T u16Size)
{
	U16_T u16Pos;

    for(u16Pos=0; u16Pos<u16Size; u16Pos++)
    {
        if(u8Arr1[u16Pos]!=u8Arr2[u16Pos])
			return 1;
    }

	return 0;
}


void _SET_MEM_PATTERN(volatile U8_T * u8Arr, U16_T u16Size, U8_T u8Pattern)
{
    U16_T u16Pos = 0;
    U8_T ctmp = 0;
	U8_T change = 0;


	if(u8Pattern==0)
	{
	   ctmp = 0;
	   for(u16Pos=0; u16Pos<u16Size; u16Pos++)
	   {
		   if(u16Pos%16==0 && u16Pos!=0)
				ctmp++;

		   u8Arr[u16Pos] = (unsigned char)ctmp;
	   }
	}
	else if(u8Pattern==1)
	{
	   ctmp = 0;
	   for(u16Pos=0; u16Pos<u16Size; u16Pos++)
	   {
		   u8Arr[u16Pos] = (unsigned char)ctmp;
		   ctmp++;
	   }
	}
	else if(u8Pattern==2)
	{
	   ctmp = 0xFF;
	   for(u16Pos=0; u16Pos<u16Size; u16Pos++)
	   {
		   u8Arr[u16Pos] = (unsigned char)ctmp;
		   ctmp--;
	   }
	}
	else if(u8Pattern==3)
	{

	   for(u16Pos=0; u16Pos<u16Size; u16Pos++)
	   {
		   if((u16Pos%0x100)==0)
		   {
			   if(change)
			   {
				   ctmp = 0xFF;
				   change = 0;
			   }
			   else
			   {
				   ctmp = 0x00;
				   change = 1;
			   }
		   }

		   u8Arr[u16Pos] = (unsigned char)ctmp;

		   if(change)
			   ctmp++;
		   else
			   ctmp--;


	   }
	}
	else if(u8Pattern==4)  //0xFF, 0x00
	{
	   ctmp = 0xFF;
	   for(u16Pos=0; u16Pos<=u16Size; u16Pos++)
	   {
		   u8Arr[u16Pos] = (unsigned char)ctmp;

		   if(ctmp==0)
			   ctmp = 0xFF;
		   else
			   ctmp = 0x0;
	   }
	}
	else if(u8Pattern==5)  //0x55, 0xAA
	{
	   ctmp = 0x55;
	   for(u16Pos=0; u16Pos<=u16Size; u16Pos++)
	   {
		   u8Arr[u16Pos] = (unsigned char)ctmp;

		   if(ctmp==0xAA)
			   ctmp = 0x55;
		   else
			   ctmp = 0xAA;
	   }
	}


}


/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_TimerTest
*     @author jeremy.wang (2015/10/13)
* Desc: Timer Test for IP Verification
*
* @param u8Sec : Seconds
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_TimerTest(U8_T u8Sec)
{
	U8_T u8CurSec;

	_PRT_LINE(FALSE);

	_PRT_MSG(0, D_TIMERTEST, 0, FALSE);
	prtstring("-->");

	for(u8CurSec=0; (u8CurSec<u8Sec)&&(u8CurSec<=10); u8CurSec++ )
	{
		prtstring("(@)");
		Hal_Timer_mDelay(1000);
	}

	_PRT_LINE(FALSE);

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_CardDetect
*     @author jeremy.wang (2015/10/1)
* Desc: Card Detection for IP Verification
*
* @param u8Slot : Slot ID
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_CardDetect(U8_T u8Slot)
{
	_PRT_LINE(FALSE);

	gu16Ret1 = !SDMMC_CardDetect(u8Slot);
	_PRT_MSG(u8Slot, D_CARDDET, gu16Ret1, FALSE);

	_PRT_LINE(FALSE);

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_Init
*     @author jeremy.wang (2015/10/1)
* Desc: SDMMC Init for IP Verification
*
* @param u8Slot : Slot ID
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_Init(U8_T u8Slot)
{
	_PRT_LINE(FALSE);

	gu16Ret1 = SDMMC_Init(u8Slot);
	_PRT_MSG(u8Slot, D_SDMMCINIT, gu16Ret1, FALSE);

	_PRT_LINE(FALSE);

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_SetWideBus
*     @author jeremy.wang (2015/10/6)
* Desc: Set Wide Bus (4Bits) for IP Verification
*
* @param u8Slot : Slot ID
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_SetWideBus(U8_T u8Slot)
{
	_PRT_LINE(FALSE);

	gu16Ret1 = SDMMC_SetWideBus(u8Slot);
	_PRT_MSG(u8Slot, D_WIDEBUS, gu16Ret1, FALSE);

	_PRT_LINE(FALSE);

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_SetHighBus
*     @author jeremy.wang (2015/10/13)
* Desc: Set HighSpeed Bus for IP Verification
*
* @param u8Slot : Slot ID
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_SetHighBus(U8_T u8Slot)
{
	_PRT_LINE(FALSE);

	gu16Ret1 = SDMMC_SwitchHighBus(u8Slot);
	_PRT_MSG(u8Slot, D_HIGHBUS, gu16Ret1, FALSE);

	_PRT_LINE(FALSE);

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_SetClock
*     @author jeremy.wang (2015/10/13)
* Desc: Set Clock for IP Verification
*
* @param u8Slot : Slot ID
* @param u32ReffClk :  Clock Hz
* @param u8DownLevel : Downgrade Level
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_SetClock(U8_T u8Slot, U32_T u32ReffClk, U8_T u8DownLevel)
{
	U32_T u32RealClk = 0;

	_PRT_LINE(FALSE);

	u32RealClk = SDMMC_SetClock(u8Slot, u32ReffClk, u8DownLevel);
	_PRT_VALUE(u8Slot, D_SETBUSCLK, u32RealClk, FALSE);

	_PRT_LINE(FALSE);
}


/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_SetBusTiming
*     @author jeremy.wang (2015/10/13)
* Desc: Set Bus Timing for IP Verification
*
* @param u8Slot : Slot ID
* @param u8BusTiming : Bus Timing (Enum Type number)
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_SetBusTiming(U8_T u8Slot, U8_T u8BusTiming)
{
	_PRT_LINE(FALSE);

	SDMMC_SetBusTiming(u8Slot, (BusTimingEmType)u8BusTiming);
	_PRT_VALUE(u8Slot, D_SETBUSTIMING, u8BusTiming, FALSE);

	_PRT_LINE(FALSE);
}


/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_CIFD_RW
*     @author jeremy.wang (2015/10/13)
* Desc: CIFD R/W for IP Verification
*
* @param u8Slot : Slot ID
* @param u32SecAddr : Sector Address
* @param bHidden : Hidden Print
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_CIFD_RW(U8_T u8Slot, U32_T u32SecAddr, BOOL_T bHidden)
{
	 volatile U8_T * _pu8_DMA_W_Buf1 = (volatile U8_T *)(A_DMA_W_BASE);
     volatile U8_T * _pu8_DMA_R_Buf1 = (volatile U8_T *)(A_DMA_R_BASE);

	_PRT_LINE(bHidden);

	_SET_MEM_PATTERN(_pu8_DMA_W_Buf1, 512, gu8Pattern);

	gu16Ret1 = SDMMC_CIF_BLK_W(u8Slot, u32SecAddr<<9, _pu8_DMA_W_Buf1);
	_PRT_MSG(u8Slot, D_CIFDWRITE, gu16Ret1, bHidden);
	_PRT_ENTER(bHidden);

	memset((void*)_pu8_DMA_R_Buf1, 0,  512);

	gu16Ret2 = SDMMC_CIF_BLK_R(u8Slot, u32SecAddr<<9, _pu8_DMA_R_Buf1);
	_PRT_MSG(u8Slot, D_CIFDREAD, gu16Ret2, bHidden);
	_PRT_ENTER(bHidden);

	gu16Ret3= _MEM_COMPARE(_pu8_DMA_W_Buf1, _pu8_DMA_R_Buf1, 512);
	_PRT_MSG(u8Slot, D_COMPARE, gu16Ret3, bHidden);

	_PRT_LINE(bHidden);

#if (D_DUMPCOMP)
	if(gu16Ret3)
	{
		_DUMP_MEM_ARR(_pu8_DMA_W_Buf1, 512);
		_DUMP_MEM_ARR(_pu8_DMA_R_Buf1, 512);
	}
#endif

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_DMA_RW
*     @author jeremy.wang (2015/10/13)
* Desc: DMA R/W for IP Verification
*
* @param u8Slot : Slot ID
* @param u32SecAddr : Sector Address
* @param u16SecCount : Sector Count
* @param bHidden : Hidden Print
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_DMA_RW(U8_T u8Slot, U32_T u32SecAddr, U16_T u16SecCount, BOOL_T bHidden)
{
	 volatile U8_T * _pu8_DMA_W_Buf1 = (volatile U8_T *)(A_DMA_W_BASE);
     volatile U8_T * _pu8_DMA_R_Buf1 = (volatile U8_T *)(A_DMA_R_BASE);

	if(u16SecCount>8)
		return;

	_PRT_LINE(bHidden);

	_SET_MEM_PATTERN(_pu8_DMA_W_Buf1, u16SecCount*512, gu8Pattern);

	gu16Ret1 = SDMMC_DMA_BLK_W(u8Slot, u32SecAddr<<9, u16SecCount, _pu8_DMA_W_Buf1);
	_PRT_MSG(u8Slot, D_DMAWRITE, gu16Ret1, bHidden);
	_PRT_ENTER(bHidden);

	memset((void*)_pu8_DMA_R_Buf1, 0,  u16SecCount*512);

	gu16Ret2 = SDMMC_DMA_BLK_R(u8Slot, u32SecAddr<<9, u16SecCount, _pu8_DMA_R_Buf1);
	_PRT_MSG(u8Slot, D_DMAREAD, gu16Ret2, bHidden);
	_PRT_ENTER(bHidden);


	gu16Ret3= _MEM_COMPARE(_pu8_DMA_W_Buf1, _pu8_DMA_R_Buf1, u16SecCount*512);
	_PRT_MSG(u8Slot, D_COMPARE, gu16Ret3, bHidden);

	_PRT_LINE(bHidden);

#if (D_DUMPCOMP)
	if(gu16Ret3)
	{
		_DUMP_MEM_ARR(_pu8_DMA_W_Buf1, u16SecCount*512);
		_DUMP_MEM_ARR(_pu8_DMA_R_Buf1, u16SecCount*512);
	}
#endif

}


//###########################################################################################################
#if (D_FCIE_M_VER == D_FCIE_M_VER__05)
//###########################################################################################################

/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_ADMA_RW
*     @author jeremy.wang (2015/10/13)
* Desc: ADMA R/W for IP Verification
*
* @param u8Slot : Slot ID
* @param u32SecAddr : Sector Address
* @param bHidden : Hidden Print
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_ADMA_RW(U8_T u8Slot, U32_T u32SecAddr, BOOL_T bHidden)
{
	U32_T au32DMAAddr[10];
	U16_T au16BlkCnt[10];

	volatile U8_T * _pu8_DMA_W_Buf1 = (volatile U8_T *)(A_DMA_W_BASE);
	volatile U8_T * _pu8_DMA_W_Buf2 = (volatile U8_T *)(A_DMA_W_BASE+0x200);
    volatile U8_T *_pu8_DMA_R_Buf1 = (volatile U8_T *)(A_DMA_R_BASE);
    volatile U8_T *_pu8_DMA_R_Buf2 = (volatile U8_T *)(A_DMA_R_BASE+0x200);

	_PRT_LINE(bHidden);

	_SET_MEM_PATTERN(_pu8_DMA_W_Buf1, 2*512, gu8Pattern);

	au32DMAAddr[0] = (U32_T)_pu8_DMA_W_Buf1;  au16BlkCnt[0] = 1;
	au32DMAAddr[1] = (U32_T)_pu8_DMA_W_Buf2;  au16BlkCnt[1] = 1;

	gu16Ret1 = SDMMC_ADMA_BLK_W(u8Slot, u32SecAddr<<9, au32DMAAddr, au16BlkCnt, 2, (volatile void*)A_ADMA_BASE);
	_PRT_MSG(u8Slot, D_ADMAWRITE, gu16Ret1, bHidden);
	_PRT_ENTER(bHidden);

	memset((void*)_pu8_DMA_R_Buf1, 0,  2*512);

	au32DMAAddr[0] = (U32_T)_pu8_DMA_R_Buf1;  au16BlkCnt[0] = 1;
	au32DMAAddr[1] = (U32_T)_pu8_DMA_R_Buf2;  au16BlkCnt[1] = 1;

	gu16Ret2 = SDMMC_ADMA_BLK_R(u8Slot,  u32SecAddr<<9, au32DMAAddr, au16BlkCnt, 2, (volatile void*)A_ADMA_BASE);
	_PRT_MSG(u8Slot, D_ADMAREAD, gu16Ret2, bHidden);
	_PRT_ENTER(bHidden);

	gu16Ret3= _MEM_COMPARE(_pu8_DMA_W_Buf1, _pu8_DMA_R_Buf1, 2*512);
	_PRT_MSG(u8Slot, D_COMPARE, gu16Ret3, bHidden);

	_PRT_LINE(bHidden);


#if (D_DUMPCOMP)
	if(gu16Ret3)
	{
		_DUMP_MEM_ARR(_pu8_DMA_W_Buf1, 2*512);
		_DUMP_MEM_ARR(_pu8_DMA_R_Buf1, 2*512);
	}
#endif

}

//###########################################################################################################
#endif  // End (D_FCIE_M_VER == D_FCIE_M_VER__05)


/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_BurnRW
*     @author jeremy.wang (2015/10/13)
* Desc: Burning R/W for IP Verification
*
* @param u8Slot : Slot ID
* @param u8TransType : DMA/ADMA/CIFD ...
* @param u32StartSec : Start Sector
* @param u32EndSec : End Sector
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_BurnRW(U8_T u8Slot, U8_T u8TransType, U32_T u32StartSec, U32_T u32EndSec)
{
	U32_T u32Sec, u32ClkCnt = 0, u32PatCnt =0, u8DownLvl = 0, u8PatLvl = 0;

	prtstring("\r\n******************************* [Begin Burning ] ***************************************\r\n");

	IPV_SDMMC_SetClock(u8Slot, 0, u8DownLvl);      //Set to Maximum Clock
	IPV_SDMMC_TestPattern(u8Slot, u8PatLvl);

	if( (u32StartSec==0) && (u32EndSec==0))
	{
		u32StartSec = BURN_START_SECTOR;
		u32EndSec = BURN_END_SECTOR;
	}

    for(u32Sec=u32StartSec; u32Sec<u32EndSec; u32Sec++)
    {

		u32ClkCnt++;
		u32PatCnt++;

		if(u8TransType==1) //DMA
		{
			prtstring("Verify_DMA_RW (SEC#)=");  prtUInt(u32Sec); prtstring(".........\r\n");
			IPV_SDMMC_DMA_RW(u8Slot, u32Sec, 2, TRUE);

			if( (gu16Ret1>0) ||  (gu16Ret2>0) || (gu16Ret3>0)  )
				return;
		}

//###########################################################################################################
#if (D_FCIE_M_VER == D_FCIE_M_VER__05)
//###########################################################################################################
		else if(u8TransType==2)
		{
			prtstring("Verify_ADMA_RW (SEC#)=");  prtUInt(u32Sec); prtstring(".........\r\n");
			IPV_SDMMC_ADMA_RW(u8Slot, u32Sec, TRUE);

			if( (gu16Ret1>0) ||  (gu16Ret2>0) || (gu16Ret3>0)  )
				return;
		}
//###########################################################################################################
#endif  // End (D_FCIE_M_VER == D_FCIE_M_VER__05)

		else
		{
			prtstring("Verify_CIFD_RW (SEC#)=");  prtUInt(u32Sec); prtstring(".........\r\n");
			IPV_SDMMC_CIFD_RW(u8Slot, u32Sec, TRUE);

			if( (gu16Ret1>0) ||  (gu16Ret2>0) || (gu16Ret3>0)  )
				return;

		}

		if(u32ClkCnt>=BURN_CLK_CHG_CNT)
		{
			u32ClkCnt = 0;
			u8DownLvl++;

			if(u8DownLvl>BURN_CLK_CHG_LVL)
				u8DownLvl = 0;

			IPV_SDMMC_SetClock(u8Slot, 0, u8DownLvl);
		}

		if(u32PatCnt>=BURN_PAT_CHG_CNT)
		{
			u32PatCnt = 0;
			u8PatLvl++;

			if(u8PatLvl>BURN_PAT_CHG_LVL)
				u8PatLvl = 0;

			IPV_SDMMC_TestPattern(u8Slot, u8PatLvl);

		}


		//Hal_Timer_mDelay(200);

    }

	prtstring("\r\n******************************* [Begin Endg ] *****************************************\r\n");

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_TestPattern
*     @author jeremy.wang (2015/10/1)
* Desc: Test Pattern for IP Verification
*
* @param u8Slot : Slot ID
* @param u8Pattern : Pattern Number
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_TestPattern(U8_T u8Slot, U8_T u8Pattern)
{
	_PRT_LINE(FALSE);

	gu8Pattern = u8Pattern;
	_PRT_VALUE(u8Slot, D_TESTPATT, (U32_T)gu8Pattern, FALSE);

	_PRT_LINE(FALSE);

}


/*----------------------------------------------------------------------------------------------------------
*
* Function: IPV_SDMMC_SetPAD
*     @author jeremy.wang (2015/10/1)
* Desc: Set PAD for IP Verification
*
* @param u8Slot : Slot ID
* @param u8IP : IP (Enum IP number)
* @param u8Port : Port (Enum Port number)
* @param u8PAD : PAD (Enum PAD number)
----------------------------------------------------------------------------------------------------------*/
void IPV_SDMMC_SetPAD(U8_T u8Slot, U8_T u8IP, U8_T u8Port, U8_T u8PAD)
{

	_PRT_LINE(FALSE);

	SDMMC_SetPAD(u8Slot, (IPEmType)u8IP, (PortEmType)u8Port, (PADEmType)u8PAD);
	SDMMC_SwitchPAD(u8Slot);
	_PRT_MSG(u8Slot, D_SETPAD, 0, FALSE);

	_PRT_LINE(FALSE);



}

/* FCIE_PWR_SAVE_CTL 0x35 */
#define BIT_POWER_SAVE_MODE             (1<<0)
#define BIT_SD_POWER_SAVE_RIU           (1<<1)
#define BIT_POWER_SAVE_MODE_INT_EN      (1<<2)
#define BIT_SD_POWER_SAVE_RST           (1<<3)
#define BIT_POWER_SAVE_INT_FORCE        (1<<4)
#define BIT_RIU_SAVE_EVENT              (1<<5)
#define BIT_RST_SAVE_EVENT              (1<<6)
#define BIT_BAT_SAVE_EVENT              (1<<7)
#define BIT_BAT_SD_POWER_SAVE_MASK      (1<<8)
#define BIT_RST_SD_POWER_SAVE_MASK      (1<<9)
#define BIT_POWER_SAVE_MODE_INT         (1<<15)

U32_T _SDMMC_hw_timer_delay(U32_T u32us)
{
	#if 0
   volatile int i = 0;

	for (i = 0; i < (u32us>>0); i++)
	{
		#if 1
		volatile int j = 0, tmp;
		for (j = 0; j < 0x2; j++)
		{
			tmp = j;
		}
		#endif
	}
	#else

	extern void udelay(unsigned long usec);
	udelay(u32us);

	#endif
	return u32us;
}

#define A_PWR_SAVE_MASK_REG(IP)			GET_CARD_REG_ADDR(GET_CARD_BANK(IP, 0), 0x34)
#define A_PWR_SAVE_CTRL_REG(IP)			GET_CARD_REG_ADDR(GET_CARD_BANK(IP, 0), 0x35)
#define A_SD_PWR_SAVE_FIFO_POS(IP)		GET_CARD_BANK(IP, 2)

#define A_PWR_SAVE_FIFO_OFFSET(IP, OFFSET)     GET_CARD_REG_ADDR(A_SD_PWR_SAVE_FIFO_POS(IP), OFFSET)

//------------------------------------------------------------------
/*
 * Power Save FIFO Cmd*
 */
#define PWR_BAT_CLASS    (0x1 << 13)  /* Battery lost class */
#define PWR_RST_CLASS    (0x1 << 12)  /* Reset Class */

/* Command Type */
#define PWR_CMD_WREG     (0x0 << 9)   /* Write data */
#define PWR_CMD_RDCP     (0x1 << 9)   /* Read and cmp data. If mismatch, HW retry */
#define PWR_CMD_WAIT     (0x2 << 9)   /* Wait idle, max. 128T */
#define PWR_CMD_WINT     (0x3 << 9)   /* Wait interrupt */
#define PWR_CMD_STOP     (0x7 << 9)   /* Stop */

/* RIU Bank */
#define PWR_CMD_BK0      (0x0 << 7)
#define PWR_CMD_BK1      (0x1 << 7)
#define PWR_CMD_BK2      (0x2 << 7)
#define PWR_CMD_BK3      (0x3 << 7)

#define PWR_RIU_ADDR     (0x0 << 0)


void IPV_SDMMC_PowerSavingModeVerify(U8_T u8Slot)
{
    U32_T u32_Count;
    U8_T eIP = u8Slot;

    CARD_REG(A_PWR_SAVE_CTRL_REG(eIP)) = CARD_REG(A_PWR_SAVE_CTRL_REG(eIP)) & ~(BIT_BAT_SD_POWER_SAVE_MASK);

    /* (1) Clear HW Enable */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x00)) = 0x0000;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x01)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x07;

    /* (2) Clear All Interrupt */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x02)) = 0xffff;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x03)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x00;

    /* (3) Clear SD MODE Enable */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x04)) = 0x0000;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x05)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0B;

    /* (4) Clear SD CTL Enable */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x06)) = 0x0000;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x07)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0C;

    /* (5) Reset Start */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x08)) = 0x0000;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x09)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x3F;

    /* (6) Reset End */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x0A)) = 0x0001;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x0B)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x3F;

    /* (7) Set "SD_MOD" */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x0C)) = 0x0021;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x0D)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0B;

    #if (D_PROJECT == D_PROJECT__iNF6E)
    /* (8) Enable "reg_sdio_en" */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x0E)) = 0x0004;
    #else
    /* (8) Enable "reg_sd_en" */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x0E)) = 0x0001;
    #endif
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x0F)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x07;

    /* (9) Command Content, IDLE */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x10)) = 0x0040;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x11)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x20;

    /* (10) Command Content, STOP */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x12)) = 0x0000;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x13)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x21;

    /* (11) Command Content, STOP */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x14)) = 0x0000;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x15)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x22;

    /* (12) Command & Response Size */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x16)) = 0x0500;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x17)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0A;

    /* (13) Enable Interrupt, SD_CMD_END */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x18)) = 0x0002;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x19)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x01;

    /* (14) Command Enable + job Start */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x1A)) = 0x0044;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x1B)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WREG | PWR_CMD_BK0 | 0x0C;

    /* (15) Wait Interrupt */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x1C)) = 0x0000;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x1D)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_WINT;

    /* (16) STOP */
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x1E)) = 0x0000;
	CARD_REG(A_PWR_SAVE_FIFO_OFFSET(eIP, 0x1F)) = PWR_BAT_CLASS | PWR_RST_CLASS | PWR_CMD_STOP;

    CARD_REG(A_PWR_SAVE_CTRL_REG(eIP)) = CARD_REG(A_PWR_SAVE_CTRL_REG(eIP)) & ~(BIT_SD_POWER_SAVE_RST);
    CARD_REG(A_PWR_SAVE_CTRL_REG(eIP)) = CARD_REG(A_PWR_SAVE_CTRL_REG(eIP)) | (BIT_SD_POWER_SAVE_RST);

    CARD_REG(A_PWR_SAVE_CTRL_REG(eIP)) = CARD_REG(A_PWR_SAVE_CTRL_REG(eIP)) | (BIT_POWER_SAVE_MODE);

    /* Step 4: Before Nand IP DMA end, use RIU interface to test power save function */
    CARD_REG(A_PWR_SAVE_CTRL_REG(eIP)) = BIT_POWER_SAVE_MODE
                     | BIT_SD_POWER_SAVE_RIU
                     | BIT_POWER_SAVE_MODE_INT_EN
                     | BIT_SD_POWER_SAVE_RST;


    for (u32_Count=0; u32_Count < 1000*1000*1000; u32_Count++)
    {
        if ((CARD_REG(A_PWR_SAVE_CTRL_REG(eIP)) & BIT_POWER_SAVE_MODE_INT) == BIT_POWER_SAVE_MODE_INT)
        {
            prtstring("eMMC_PWRSAVE_CTL: ");  prtU16Hex(CARD_REG(A_PWR_SAVE_CTRL_REG(eIP)));prtstring("\n");
            break;
        }
        _SDMMC_hw_timer_delay(1000);
    }
    prtstring("eMMC_PWRSAVE_CTL: ");  prtU16Hex(CARD_REG(A_PWR_SAVE_CTRL_REG(eIP)));prtstring("\n");
}

