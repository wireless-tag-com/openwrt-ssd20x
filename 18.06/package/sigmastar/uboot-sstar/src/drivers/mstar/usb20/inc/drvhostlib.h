/*
* drvhostlib.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: jiang.ann <jiang.ann@sigmastar.com.tw>
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

#ifndef __DRVHOSTLIB_H__
#define __DRVHOSTLIB_H__

#include "../hal/platform.h"
#include "ubootglue.h"

#define USBDELAY(x)	wait_ms(x)

/* =================== 1.Condition Definition  ============================================================ *
 * ======================================================================================================== */
//#define Interrupt_No_FramList_Temp_Solution NUSED
//#define FUSBH200_HOST_ONLY // NUSED

#define Host20_Debug_Info		0x01
#define IRQ_USB_Host20			40
#define Host20_Set_Address		0x03

/* Host Configuration */
#define Host20_QHD_Nat_Counter		0x00	/* Temp Solution from 15 to 0  //Bit28~31 */
// #define Host20_EOF1Time		0x00	/* For Full Speed Device */
#define Host20_EOF1Time			0x03	/* For High Speed Device */
#define Host20_Ctrl_Timeout		5		/* 5s Control qTD Timeout */
#define Host20_Timeout			5		/* 5s Bulk qTD Timeout */

/* =================== 2.Variable Definition  ============================================================= *
 * ======================================================================================================== */
#define HOST20_OK					0x00
#define HOST20_FAIL					0x01
#define HOST20_FATAL				0x02
#define HOST20_DEVICE_STALL		0x03
#define HOST20_TRANSACTION_ERROR   0x04

/******************************
 * Data Structure Allocation  *
 ******************************
 * 0x3000000~0x3001000  =>qHD *
 * 0x3001000~0x3002000  =>qTD *
 * 0x3002000~0x3003000  =>iTD *
 ******************************/
#define USB_BUFFER_START_ADR_4K_ALIGN		0
#define Host20_STRUCTURE_BASE_ADDRESS		0xf000	/* SDRAM start at 60K */

// #define Host20_STRUCTURE_qHD_BASE_ADDRESS	( Host20_DATA_PAGE_BASE_ADDRESS+Host20_Page_SIZE)	/* (DRAM=48M) */
#define Host20_qHD_SIZE		0x40	/* (48bytes), for alignment */
#define Host20_qHD_MAX		10	/* (10 ) */

// #define Host20_STRUCTURE_qTD_BASE_ADDRESS	(Host20_STRUCTURE_qHD_BASE_ADDRESS+Host20_qHD_SIZE*Host20_qHD_MAX)	/* (DRAM=48M) */
#define Host20_qTD_SIZE		0x20	/* (32bytes) */
#define Host20_qTD_MAX		0x10	/* (50 ) */

// #define Host20_STRUCTURE_Preiodic_Frame_List_BASE_ADDRESS	(Host20_STRUCTURE_BASE_ADDRESS+0x3000)	/* (DRAM=48M) */
// #define Host20_Preiodic_Frame_SIZE		0x04	/* (4bytes) */
// #define Host20_Preiodic_Frame_List_MAX	1024	/* (1024\) */

// #define Host20_STRUCTURE_iTD_BASE_ADDRESS	(Host20_STRUCTURE_BASE_ADDRESS+0x10000)	/* (DRAM=48M) */
// #define Host20_iTD_SIZE		0x40	/* (64bytes) */
// #define Host20_iTD_MAX		1024	/* (10 ) */

#define  Host20_TEMP_DATA		(Host20_STRUCTURE_qTD_BASE_ADDRESS+Host20_qTD_SIZE*Host20_qTD_MAX)
#define  Host20_TEMP_DATA_SIZE		0x80

#define Host20_Attach_Device_DATA	(Host20_TEMP_DATA+Host20_TEMP_DATA_SIZE)
#define Host20_Attach_Device_SIZE	sizeof(Host20_Attach_Device_Structure)

#define  OTGH_PT_BLK_DATA		(Host20_Attach_Device_DATA+Host20_Attach_Device_SIZE)
#define  OTGH_PT_BLK_SIZE 		sizeof(OTGH_PT_BLK_Struct)
// #define Host20_ATTACH_DEVICE_HERE	(Host20_TEMP_DATA+Host20_TEMP_DATA_SIZE)

#define Host20_MEM_TYPE_qTD			0x00
#define Host20_MEM_TYPE_iTD			0x01
#define Host20_MEM_TYPE_4K_BUFFER		0x02
#define Host20_MEM_TYPE_siTD			0x03

#define Host20_MEM_FREE			0x01
#define Host20_MEM_USED			0x02

#define MS_BIT0			0x01
#define MS_BIT1			0x02
#define MS_BIT2			0x04
#define MS_BIT3			0x08
#define MS_BIT4			0x10
#define MS_BIT5			0x20
#define MS_BIT6			0x40
#define MS_BIT7			0x80

#define MS_BIT8			0x00000100
#define MS_BIT9			0x00000200
#define MS_BIT10		0x00000400
#define MS_BIT11		0x00000800
#define MS_BIT12		0x00001000
#define MS_BIT13		0x00002000
#define MS_BIT14		0x00004000
#define MS_BIT15		0x00008000

#define MS_BIT16		0x00010000
#define MS_BIT17		0x00020000
#define MS_BIT18		0x00040000
#define MS_BIT19		0x00080000
#define MS_BIT20		0x00100000
#define MS_BIT21		0x00200000
#define MS_BIT22		0x00400000
#define MS_BIT23		0x00800000

#define MS_BIT24		0x01000000
#define MS_BIT25		0x02000000
#define MS_BIT26		0x04000000
#define MS_BIT27		0x08000000
#define MS_BIT28		0x10000000
#define MS_BIT29		0x20000000
#define MS_BIT30		0x40000000
#define MS_BIT31		0x80000000

#define OTGH_Dir_IN			0x01
#define OTGH_Dir_Out			0x00
#define OTGH_NULL			0x00
#define OTGH_ED_ISO			0x01
#define OTGH_ED_BULK			0x02
#define OTGH_ED_INT			0x03
#define OTGH_ED_Control			0x00
#define OTGH_FARADAY_TEST_AP		0x10237856
#define OTGH_SRP_HNP_Enable		0x03
#define OTGH_Remote_Wake_UP		0x00000400
#define OTGH_Remote_Wake_UP_INT		0x00000008

unsigned char mwHost20Port(int addr,int bOffset);
void mwHost20Portw(int addr,int bOffset,int value);
void mwHost20Port_wr(int addr,int bOffset, int value);
int  mwHost20Bit_Rd(int addr,int bByte,int wBitNum);
void  mwHost20Bit_Set(int addr,int bByte,int wBitNum);
void  mwHost20Bit_Clr(int addr,int bByte,int wBitNum);

/* =================== 2.Define Register Macro ================================================================ *
 * ============================================================================================================ */
/* <1>.Macro volatile */
#define mwHost20_NumPorts_Rd(a)		((mwHost20Port(a,0x04)&0x0000000F)

/* <4>.0x008(HCCPARAMS - Capability Parameters) */
#define mbHost20_ProgrammableFrameListFlag_Rd(a)		(mwHost20Bit_Rd(a,0x08,MS_BIT1))	// Bit 1

#define mbHost20_USBCMD_ParkMode_CNT_Rd(a)		((mwHost20Port(a,0x10)>>8)&0x00000003)
#define mbHost20_USBCMD_ParkMode_CNT_Set(a,bValue)	(mwHost20Port(a,0x011)=(mwHost20Port(a,0x011)&0xFC)|(( (MS_UINT8) bValue )<<8)  )	// Bit 8~9

#define mbHost20_USBCMD_InterruptOnAsync_Rd(a)		(mwHost20Bit_Rd(a,0x10,MS_BIT6)) 	// Bit 6
#define mbHost20_USBCMD_InterruptOnAsync_Set(a)		(mwHost20Bit_Set(a,0x10,MS_BIT6))	// Bit 6
#define mbHost20_USBCMD_InterruptOnAsync_Clr(a)		(mwHost20Bit_Clr(a,0x10,MS_BIT6))	// Bit 6

#define mbHost20_USBCMD_AsynchronousEnable_Rd(a)		(mwHost20Bit_Rd(a,0x10,MS_BIT5))	// Bit 5
#define mbHost20_USBCMD_AsynchronousEnable_Set(a)	(mwHost20Bit_Set(a,0x10,MS_BIT5))	// Bit 5
#define mbHost20_USBCMD_AsynchronousEnable_Clr(a)	(mwHost20Bit_Clr(a,0x10,MS_BIT5))	// Bit 5

#define mbHost20_USBCMD_PeriodicEnable_Rd(a)		(mwHost20Bit_Rd(a,0x10,MS_BIT4))	// Bit 4
#define mbHost20_USBCMD_PeriodicEnable_Set(a)		(mwHost20Bit_Set(a,0x10,MS_BIT4))	// Bit 4
#define mbHost20_USBCMD_PeriodicEnable_Clr(a)		(mwHost20Bit_Clr(a,0x10,MS_BIT4))	// Bit 4

#define mbHost20_USBCMD_FrameListSize_Rd(a)		((mwHost20Port(a,0x10)>>2)&0x00000003)	// Bit 2~3
#define mbHost20_USBCMD_FrameListSize_Set(a,bValue)	((mwHost20Port(a,0x10)=((mwHost20Port(a,0x10)&0xFFFFFFF3)|(((MS_UINT32)(bValue))<<2)))	// Bit 2~3

#define HOST20_USBCMD_FrameListSize_1024		0x00
#define HOST20_USBCMD_FrameListSize_512			0x01
#define HOST20_USBCMD_FrameListSize_256			0x02

#define mbHost20_USBCMD_HCReset_Rd(a)		(mwHost20Bit_Rd(a,0x10,MS_BIT1))	// Bit 1
#define mbHost20_USBCMD_HCReset_Set(a)		(mwHost20Bit_Set(a,0x10,MS_BIT1))	// Bit 1

#define mbHost20_USBCMD_RunStop_Rd(a)		(mwHost20Bit_Rd(a,0x10,MS_BIT0))	// Bit 0
#define mbHost20_USBCMD_RunStop_Set(a)		(mwHost20Bit_Set(a,0x10,MS_BIT0))	// Bit 0
#define mbHost20_USBCMD_RunStop_Clr(a)		(mwHost20Bit_Clr(a,0x10,MS_BIT0))	// Bit 0

#define HOST20_Enable		0x01
#define HOST20_Disable		0x00

/* <5>.0x014(USBSTS - USB Status Register) */
#define mwHost20_USBSTS_Rd(a)				(mwHost20Port(a,0x14))
#define mwHost20_USBSTS_Set(a,wValue)			mwHost20Port_wr(a,0x14,wValue)

#define mwHost20_USBSTS_AsynchronousStatus_Rd(a)		(mwHost20Bit_Rd(a,0x15,MS_BIT7))	// 14->Bit 15
#define mwHost20_USBSTS_PeriodicStatus_Rd(a)		(mwHost20Bit_Rd(a,0x15,MS_BIT6))

// #define mwHost20_USBSTS_Reclamation_Rd()		(mwHost20Bit_Rd(a,0x15,MS_BIT5))
#define mwHost20_USBSTS_HCHalted_Rd(a)                  (mwHost20Bit_Rd(a,0x15,MS_BIT4))

#define mwHost20_USBSTS_IntOnAsyncAdvance_Rd(a)		(mwHost20Bit_Rd(a,0x14,MS_BIT5))	// Bit 5
#define mwHost20_USBSTS_IntOnAsyncAdvance_Set(a)		(mwHost20Bit_Set(a,0x14,MS_BIT5))	// Bit 5

#define mwHost20_USBSTS_SystemError_Rd(a)		(mwHost20Bit_Rd(a,0x14,MS_BIT4))	// Bit 4
#define mwHost20_USBSTS_SystemError_Set(a)		(mwHost20Bit_Set(a,0x14,MS_BIT4))	// Bit 4

#define mwHost20_USBSTS_FrameRollover_Rd(a)		(mwHost20Bit_Rd(a,0x14,MS_BIT3))	// Bit 3
#define mwHost20_USBSTS_FrameRollover_Set(a)		(mwHost20Bit_Set(a,0x14,MS_BIT3))	// Bit 3

#define mwHost20_USBSTS_PortChangeDetect_Rd(a)		(mwHost20Bit_Rd(a,0x14,MS_BIT2))	// Bit 2
#define mwHost20_USBSTS_PortChangeDetect_Set(a)		(mwHost20Bit_Set(a,0x14,MS_BIT2))	// Bit 2

#define mwHost20_USBSTS_USBError_Rd(a)			(mwHost20Bit_Rd(a,0x14,MS_BIT1))	// Bit 1
#define mwHost20_USBSTS_USBError_Set(a)			(mwHost20Bit_Set(a,0x14,MS_BIT1))	// Bit 1

#define mwHost20_USBSTS_CompletionOfTransaction_Rd(a)	(mwHost20Bit_Rd(a,0x14,MS_BIT0))	// Bit 0
#define mwHost20_USBSTS_CompletionOfTransaction_Set(a)	(mwHost20Bit_Set(a,0x14,MS_BIT0))	// Bit 0

/* <6>.0x018(USBINTR - USB Interrupt Enable Register) */
#define mwHost20_USBINTR_Rd(a)				(mwHost20Port(a,0x18))
#define mwHost20_USBINTR_Set(a,bValue)			mwHost20Port_wr(a,0x18,bValue)

#define mwHost20_USBINTR_IntOnAsyncAdvance_Rd(a)		(mwHost20Bit_Rd(a,0x18,MS_BIT5))	// Bit 5
#define mwHost20_USBINTR_IntOnAsyncAdvance_Set(a)	(mwHost20Bit_Set(a,0x18,MS_BIT5)) 	// Bit 5
#define mwHost20_USBINTR_IntOnAsyncAdvance_Clr(a)	(mwHost20Bit_Clr(a,0x18,MS_BIT5))	// Bit 5

#define mwHost20_USBINTR_SystemError_Rd(a)		(mwHost20Bit_Rd(a,0x18,MS_BIT4))	// Bit 4
#define mwHost20_USBINTR_SystemError_Set(a)		(mwHost20Bit_Set(a,0x18,MS_BIT4))	// Bit 4
#define mwHost20_USBINTR_SystemError_Clr(a)		(mwHost20Bit_Clr(a,0x18,MS_BIT4))	// Bit 4

#define mwHost20_USBINTR_FrameRollover_Rd(a)		(mwHost20Bit_Rd(a,0x18,MS_BIT3))	// Bit 3
#define mwHost20_USBINTR_FrameRollover_Set(a)		(mwHost20Bit_Set(a,0x18,MS_BIT3))	// Bit 3
#define mwHost20_USBINTR_FrameRollover_Clr(a)		(mwHost20Bit_Clr(a,0x18,MS_BIT3))	// Bit 3

#define mwHost20_USBINTR_PortChangeDetect_Rd(a)		(mwHost20Bit_Rd(a,0x18,MS_BIT2))	// Bit 2
#define mwHost20_USBINTR_PortChangeDetect_Set(a)		(mwHost20Bit_Set(a,0x18,MS_BIT2)) 	// Bit 2
#define mwHost20_USBINTR_PortChangeDetect_Clr(a)		(mwHost20Bit_Clr(a,0x18,MS_BIT2)) 	// Bit 2

#define mwHost20_USBINTR_USBError_Rd(a)			(mwHost20Bit_Rd(a,0x18,MS_BIT1))	// Bit 1
#define mwHost20_USBINTR_USBError_Set(a)			(mwHost20Bit_Set(a,0x18,MS_BIT1))	// Bit 1
#define mwHost20_USBINTR_USBError_Clr(a)			(mwHost20Bit_Clr(a,0x18,MS_BIT1))	// Bit 1

#define mwHost20_USBINTR_CompletionOfTransaction_Rd(a)	(mwHost20Bit_Rd(a,0x18,MS_BIT0))	// Bit 0
#define mwHost20_USBINTR_CompletionOfTransaction_Set(a)	(mwHost20Bit_Set(a,0x18,MS_BIT0))	// Bit 0
#define mwHost20_USBINTR_CompletionOfTransaction_Clr(a)	(mwHost20Bit_Clr(a,0x18,MS_BIT0))	// Bit 0

#define HOST20_USBINTR_IntOnAsyncAdvance		0x20
#define HOST20_USBINTR_SystemError			0x10
#define HOST20_USBINTR_FrameRollover			0x08
#define HOST20_USBINTR_PortChangeDetect			0x04
#define HOST20_USBINTR_USBError				0x02
#define HOST20_USBINTR_CompletionOfTransaction		0x01
#define USBWAITEVENTS	(HOST20_USBINTR_CompletionOfTransaction|HOST20_USBINTR_USBError|HOST20_USBINTR_PortChangeDetect|HOST20_USBINTR_SystemError)

/* <7>.0x01C(FRINDEX - Frame Index Register (Address = 01Ch)) */
// #define mwHost20_FrameIndex_Rd()		(mwHost20Port(0x1C)&0x00001FFF)	/* Only Read Bit0~Bit12(Skip Bit 13) */
// #define mwHost20_FrameIndex14Bit_Rd()	(mwHost20Port(0x1C)&0x00003FFF)	/* Only Read Bit0~Bit12(Skip Bit 13) */
// #define mwHost20_FrameIndex_Set(wValue)	(mwHost20Port(0x1C)=wValue)

/* <8>.0x024(PERIODICLISTBASE - Periodic Frame List Base Address Register (Address = 024h)) */
// #define mwHost20_PeriodicBaseAddr_Rd()	(mwHost20Port(0x24))
#define mwHost20_PeriodicBaseAddr_Set(a,wValue)	mwHost20Portw(a,0x24,wValue)

/* <9>.0x028(ASYNCLISTADDR - Current Asynchronous List Address Register (Address = 028h)) */
//#define mwHost20_CurrentAsynchronousAddr_Rd()	(mwHost20Port(0x28) )
#define mwHost20_CurrentAsynchronousAddr_Set(a,wValue)	mwHost20Portw(a,0x28,wValue)

/* <10>.0x030(PORTSC - Port Status and Control Register(Address = 030h)) */
#define mwHost20_PORTSC_Rd(a)				mwHost20Port(a,0x30)
// #define mwHost20_PORTSC_LineStatus_Rd()		((mwHost20Port(0x30)>>10)&0x00000003)

#define mwHost20_PORTSC_PortReset_Rd(a)			mwHost20Bit_Rd(a,0x31,MS_BIT0)	// Bit 8
#define mwHost20_PORTSC_PortReset_Set(a)			mwHost20Bit_Set(a,0x31,MS_BIT0)	// Bit 8
#define mwHost20_PORTSC_PortReset_Clr(a)			mwHost20Bit_Clr(a,0x31,MS_BIT0)	// Bit 8

#define mwHost20_PORTSC_ForceSuspend_Rd(a)		mwHost20Bit_Rd(a,0x30,MS_BIT7)	// Bit 7
#define mwHost20_PORTSC_ForceSuspend_Set(a)		mwHost20Bit_Set(a,0x30,MS_BIT7)	// Bit 7

#define mwHost20_PORTSC_ForceResume_Rd(a)		mwHost20Bit_Rd(a,0x30,MS_BIT6)	// Bit 6
#define mwHost20_PORTSC_ForceResume_Set(a)		mwHost20Bit_Set(a,0x30,MS_BIT6)	// Bit 6
#define mwHost20_PORTSC_ForceResume_Clr(a)		mwHost20Bit_Clr(a,0x30,MS_BIT6)	// Bit 6

#define mwHost20_PORTSC_EnableDisableChange_Rd(a)	mwHost20Bit_Rd(a,0x30,MS_BIT3)	// Bit 3
#define mwHost20_PORTSC_EnableDisableChange_Set(a)	mwHost20Bit_Set(a,0x30,MS_BIT3)	// Bit 3

#define mwHost20_PORTSC_EnableDisable_Rd(a)		mwHost20Bit_Rd(a,0x30,MS_BIT2)	// Bit 2
#define mwHost20_PORTSC_EnableDisable_Set(a)		mwHost20Bit_Set(a,0x30,MS_BIT2)	// Bit 2
#define mwHost20_PORTSC_EnableDisable_Clr(a)		mwHost20Bit_Clr(a,0x30,MS_BIT2)	// Bit 2

#define mwHost20_PORTSC_ConnectChange_Rd(a)	          mwHost20Bit_Rd(a,0x30,MS_BIT1)	// Bit 1
#define mwHost20_PORTSC_ConnectChange_Set(a)	          mwHost20Bit_Set(a,0x30,MS_BIT1)	// Bit 1

#define mwHost20_PORTSC_ConnectStatus_Rd(a)	          mwHost20Bit_Rd(a,0x30,MS_BIT0)	// Bit 0

/* <10>.0x034(Misc. Register(Address = 034h)) */
#define mwHost20_Misc_EOF1Time_Set(a,bValue)	mwHost20Port_wr(a,0x34, ((mwHost20Port(a,0x34)&0xF3)|(((MS_UINT8)(bValue))<<2)) )
// (mwHost20Port(0x34)=((mwHost20Port(0x34)&0xF3)|(((MS_UINT8)(bValue))<<2)))	// Bit 2~3

/* <10>.0x034(Misc. Register(Address = 40h)) */
#define mwHost20_Control_LineStatus_Rd(a)		(mwHost20Port(a,0x41)& MS_BIT3)
#define mwHost20_Control_LineStatus_Set(a)		(mwHost20Bit_Set(a,0x41,MS_BIT3))
#define mwHost20_Control_LineStatus_Clr(a)		(mwHost20Bit_Clr(a,0x41,MS_BIT3))

#define mwOTG20_Control_HOST_SPD_TYP_Rd(a)		((mwHost20Port(a,0x41)>>1)&0x03)

#define mwHost20_Control_ForceFullSpeed_Rd(a)		(mwHost20Port(a,0x40)& MS_BIT7)
#define mwHost20_Control_ForceFullSpeed_Set(a)		(mwHost20Bit_Set(a,0x40,MS_BIT7))
#define mwHost20_Control_ForceFullSpeed_Clr(a)		(mwHost20Bit_Clr(a,0x40,MS_BIT7))

#define mwHost20_Control_ForceHighSpeed_Rd(a)		(mwHost20Port(a,0x40)& MS_BIT6)
#define mwHost20_Control_ForceHighSpeed_Set(a)		(mwHost20Bit_Set(a,0x40,MS_BIT6))
#define mwHost20_Control_ForceHighSpeed_Clr(a)		(mwHost20Bit_Clr(a,0x40,MS_BIT6))

#define mwOTG20_Control_Phy_Reset_Set(a)			(mwHost20Bit_Set(a,0x40,MS_BIT5))
#define mwOTG20_Control_Phy_Reset_Clr(a)			(mwHost20Bit_Clr(a,0x40,MS_BIT5))

#define mwOTG20_Control_Half_Speed(a)			(mwHost20Bit_Set(a,0x40,MS_BIT2))
#define mwOTG20_Control_Int_Polarity_Hi(a)		(mwHost20Bit_Set(a,0x40,MS_BIT3))

// #define mwHost20_Control_711MA_FS_Issue_Solve()	(mwHost20Bit_Set(a,0x41,MS_BIT4))

#define flib_Host20_Allocate_QHD_Macro(x,y,z,w,u,v)		\
	{	x->bType=y;					\
		x->bDeviceAddress=z;				\
		x->bHeadOfReclamationListFlag=w;		\
		x->bEdNumber=u;					\
		x->bMaxPacketSize=v;}

/* =================== 3.Structure Definition ============================================================= *
 * ======================================================================================================== */

/* <3.1>iTD Structure Definition*****************************************/

/* <3.2>qTD Structure Definition*****************************************/
typedef struct  _BufferPointer
{
	MS_UINT8	Byte0;
	MS_UINT8	Byte1;
	MS_UINT8	Byte2;
	MS_UINT8	Byte3;
}BufferPointer_Struct;

#define Host20_Preiodic_Frame_List_MAX	256//1024	//(1024\)

typedef struct {
	/* <1>.Next_Link_Pointer Word */
	MS_UINT32	bTerminal:1;		// Bit11~0
	MS_UINT32	bType:2;		// Bit11~0
	MS_UINT32	bReserved:2;		// Bit14~12
	MS_UINT32	bLinkPointer:27;	// Bit15

} Periodic_Frame_List_Cell_Structure;

typedef struct {
	Periodic_Frame_List_Cell_Structure sCell[Host20_Preiodic_Frame_List_MAX];
} Periodic_Frame_List_Structure;

typedef struct _qTD {
	MS_UINT32	bTerminate:1;
	MS_UINT32	bReserve_1:4;
	MS_UINT32	bNextQTDPointer:27;

	/* <2>.Alternate Next qTD Word */
	MS_UINT32	bAlternateTerminate:1;
	MS_UINT32	bReserve_2:4;
	MS_UINT32	bAlternateQTDPointer:27;

	/* <3>.Status Word */
	MS_UINT32	bStatus_PingState:1;
	MS_UINT32	bStatus_SplitState:1;
	MS_UINT32	bStatus_MissMicroFrame:1;
	MS_UINT32	bStatus_Transaction_Err:1;
	MS_UINT32	bStatus_Babble:1;
	MS_UINT32	bStatus_Buffer_Err:1;
	MS_UINT32	bStatus_Halted:1;
	MS_UINT32	bStatus_Active:1;

	MS_UINT32	bPID:2;
	MS_UINT32	bErrorCounter:2;
	MS_UINT32	CurrentPage:3;
	MS_UINT32	bInterruptOnComplete:1;
	MS_UINT32	bTotalBytes:15;
	MS_UINT32	bDataToggle:1;

	/* <4>.Buffer Pointer Word Array */
	MS_UINT32	ArrayBufferPointer_Word[5];
} qTD_Structure;

#define HOST20_qTD_PID_OUT			0x00
#define HOST20_qTD_PID_IN			0x01
#define HOST20_qTD_PID_SETUP			0x02

#define HOST20_qTD_STATUS_Active		0x80
#define HOST20_qTD_STATUS_Halted		0x40
#define HOST20_qTD_STATUS_BufferError		0x20
#define HOST20_qTD_STATUS_Babble		0x10
#define HOST20_qTD_STATUS_TransactionError	0x08
#define HOST20_qTD_STATUS_MissMicroFrame	0x04
#define HOST20_qTD_STATUS_Split			0x02
#define HOST20_qTD_STATUS_Ping			0x01

typedef struct _qHD {
	/* <1>.Next_qHD_Pointer Word */
	MS_UINT32	bTerminate:1;
	MS_UINT32	bType:2;
	MS_UINT32	bReserve_1:2;
	MS_UINT32	bNextQHDPointer:27;

	/* <2>.qHD_2 Word */
	MS_UINT32	bDeviceAddress:7;
	MS_UINT32	bInactiveOnNextTransaction:1;
	MS_UINT32	bEdNumber:4;
	MS_UINT32	bEdSpeed:2;
	MS_UINT32	bDataToggleControl:1;
	MS_UINT32	bHeadOfReclamationListFlag:1;
	MS_UINT32	bMaxPacketSize:11;
	MS_UINT32	bControlEdFlag:1;
	MS_UINT32	bNakCounter:4;

	/* <3>.qHD_3 Word */
	MS_UINT32	bInterruptScheduleMask:8;
	MS_UINT32	bSplitTransactionMask:8;
	MS_UINT32	bHubAddr:7;
	MS_UINT32	bPortNumber:7;
	MS_UINT32	bHighBandwidth:2;

	/* <4>.Overlay_CurrentqTD */
	MS_UINT32	bOverlay_CurrentqTD;

	/* <5>.Overlay_NextqTD */
	MS_UINT32	bOverlay_NextTerminate:1;
	MS_UINT32	bOverlay_Reserve2:4;
	MS_UINT32	bOverlay_NextqTD:27;

	/* <6>.Overlay_AlternateNextqTD */
	MS_UINT32	bOverlay_AlternateNextTerminate:1;
	MS_UINT32	bOverlay_NanCnt:4;
	MS_UINT32	bOverlay_AlternateqTD:27;

	/* <7>.Overlay_TotalBytes */
	MS_UINT32	bOverlay_Status:8;
	MS_UINT32	bOverlay_PID:2;
	MS_UINT32	bOverlay_ErrorCounter:2;
	MS_UINT32	bOverlay_C_Page:3;
	MS_UINT32	bOverlay_InterruptOnComplete:1;
	MS_UINT32	bOverlay_TotalBytes:15;
	MS_UINT32	bOverlay_Direction:1;

	/* <8>.Overlay_BufferPointer0 */
	MS_UINT32	bOverlay_CurrentOffset:12;
	MS_UINT32	bOverlay_BufferPointer_0:20;

	/* <9>.Overlay_BufferPointer1 */
	MS_UINT32	bOverlay_C_Prog_Mask:8;
	MS_UINT32	bOverlay_Reserve3:4;
	MS_UINT32	bOverlay_BufferPointer_1:20;

	/* <10>.Overlay_BufferPointer2 */
	MS_UINT32	bOverlay_FrameTag:5;
	MS_UINT32	bOverlay_S_Bytes:7;
	MS_UINT32	bOverlay_BufferPointer_2:20;

	/* <11>.Overlay_BufferPointer3 */
	MS_UINT32	bOverlay_Reserve4:12;
	MS_UINT32	bOverlay_BufferPointer_3:20;

	/* <12>.Overlay_BufferPointer4 */
	MS_UINT32	bOverlay_Reserve5:12;
	MS_UINT32	bOverlay_BufferPointer_4:20;
} qHD_Structure;

#define HOST20_HD_Type_iTD		0x00
#define HOST20_HD_Type_QH		0x01
#define HOST20_HD_Type_siTD		0x02
#define HOST20_HD_Type_FSTN		0x03

/* <3.4>.Test Condition Definition****************************************/
typedef struct {
	MS_UINT8	bStructureEnable;		/* Enable = 0x66  Disable=>Others */
	MS_UINT8	bInterruptThreshod;		/* 01,02,04,08,10,20,40 */
	MS_UINT8	bAsynchronousParkMode;		/* 00=>Disable,01=>Enable */
	MS_UINT8	bAsynchronousParkModeCounter;	/* 01,02,03 */
	MS_UINT8	bFrameSize;			/* 00,01,02 */
} Host20_Init_Condition_Structure;

#define HOST20_FrameSize_1024		0x00
#define HOST20_FrameSize_512		0x01
#define HOST20_FrameSize_256		0x02

/* <3.5>.Host20's Attach Device Info Structure****************************************/

/* OTGHost Device Structure */
typedef struct
{
	MS_UINT8	bDEVICE_LENGTH;			// bLength
	MS_UINT8	bDT_DEVICE;			// bDescriptorType
	MS_UINT8	bVerLowByte;			// bcdUSB
	MS_UINT8	bVerHighByte;

	MS_UINT8	bDeviceClass;			// bDeviceClass
	MS_UINT8	bDeviceSubClass;		// bDeviceSubClas;
	MS_UINT8	bDeviceProtocol;		// bDeviceProtocol
	MS_UINT8	bEP0MAXPACKETSIZE;		// bMaxPacketSize0

	MS_UINT8	bVIDLowByte;			// idVendor
	MS_UINT8	bVIDHighByte;
	MS_UINT8	bPIDLowByte;			// idProduct
	MS_UINT8	bPIDHighByte;
	MS_UINT8	bRNumLowByte;			// bcdDeviceReleaseNumber
	MS_UINT8	bRNumHighByte;

	MS_UINT8	bManufacturer;			// iManufacturer
	MS_UINT8	bProduct;			// iProduct
	MS_UINT8	bSerialNumber;			// iSerialNumber
	MS_UINT8	bCONFIGURATION_NUMBER;		// bNumConfigurations
}OTGH_Descriptor_Device_Struct;

/* <3.6>.OTGHost Configuration Structure =>
	 Only Support 2 Configuration / 5 Interface / 1 Class / 5 Endpoint /1 OTG */

#define  HOST20_CONFIGURATION_NUM_MAX		0X02
#define  HOST20_INTERFACE_NUM_MAX		0X05
#define  HOST20_ENDPOINT_NUM_MAX		0X05
#define  HOST20_CLASS_NUM_MAX			0x01

#define  HOST20_CONFIGURATION_LENGTH		0X09
#define  HOST20_INTERFACE_LENGTH		0X09
#define  HOST20_ENDPOINT_LENGTHX		0X07
#define  HOST20_CLASS_LENGTHX			0X09

typedef struct
{
	/* <3>.Define for ED-OTG */
	MS_UINT8	bED_OTG_Length;
	MS_UINT8	bED_OTG_bDescriptorType;
	MS_UINT8	bED_OTG_bAttributes;
} OTGH_Descriptor_OTG_Struct;

typedef struct
{
	/* <3>.Define for ED-1 */
	MS_UINT8	bED_Length;
	MS_UINT8	bED_bDescriptorType;
	MS_UINT8	bED_EndpointAddress;
	MS_UINT8	bED_bmAttributes;
	MS_UINT8	bED_wMaxPacketSizeLowByte;
	MS_UINT8	bED_wMaxPacketSizeHighByte;
	MS_UINT8	bED_Interval;

} OTGH_Descriptor_EndPoint_Struct;

typedef struct
{
	MS_UINT8	bClass_LENGTH;
	MS_UINT8	bClaNumberss;
	MS_UINT8	bClassVerLowByte;
	MS_UINT8	bClassVerHighByte;
	MS_UINT8	bCityNumber;
	MS_UINT8	bFollowDescriptorNum;
	MS_UINT8	bReport;
	MS_UINT8	bLengthLowByte;
	MS_UINT8	bLengthHighByte;
} OTGH_Descriptor_Class_Struct;

typedef struct
{
	/* <2>.Define for Interface-1 */
	MS_UINT8	bINTERFACE_LENGTH;	// bLength
	MS_UINT8	bDT_INTERFACE;		// bDescriptorType INTERFACE
	MS_UINT8	bInterfaceNumber;	// bInterfaceNumber
	MS_UINT8	bAlternateSetting;	// bAlternateSetting
	MS_UINT8	bEP_NUMBER;		// bNumEndpoints(excluding endpoint zero)
	MS_UINT8	bInterfaceClass;	// bInterfaceClass
	MS_UINT8	bInterfaceSubClass;	// bInterfaceSubClass
	MS_UINT8	bInterfaceProtocol;	// bInterfaceProtocol
	MS_UINT8	bInterface;		// iInterface

	OTGH_Descriptor_Class_Struct	sClass[HOST20_CLASS_NUM_MAX];
	OTGH_Descriptor_EndPoint_Struct	sED[HOST20_ENDPOINT_NUM_MAX];
} OTGH_Descriptor_Interface_Struct;

typedef struct
{
	MS_UINT8	bCONFIG_LENGTH;		// bLength
	MS_UINT8	bDT_CONFIGURATION;	// bDescriptorType CONFIGURATION
	MS_UINT8	bTotalLengthLowByte;	// wTotalLength, include all descriptors
	MS_UINT8	bTotalLengthHighByte;
	MS_UINT8	bINTERFACE_NUMBER;	// bNumInterface
	MS_UINT8	bConfigurationValue;	// bConfigurationValue
	MS_UINT8	bConfiguration;		// iConfiguration
	MS_UINT8	bAttribute;		// bmAttribute
	MS_UINT8	bMaxPower;		// iMaxPower (2mA units)

	OTGH_Descriptor_Interface_Struct	sInterface[HOST20_INTERFACE_NUM_MAX];
} OTGH_Descriptor_Configuration_Only_Struct;

/*****************************
 * Support Configuration x2  *
 *         Interface     x5  *
 *         EndPoint      x5  *
 *         OTG           X1  *
 *****************************/

typedef struct
{
	/* <2>.Descriptor Information */
	OTGH_Descriptor_Device_Struct			sDD;
	OTGH_Descriptor_Configuration_Only_Struct	saCD[HOST20_CONFIGURATION_NUM_MAX];
	OTGH_Descriptor_OTG_Struct			sOTG;

	MS_UINT8		bAdd;
	/* 0=>Low Speed / 1=>Full Speed / 2 => High Speed */
	MS_UINT8		bSpeed;
	MS_UINT8		bSendStatusError;

	// MS_UINT8		bReportDescriptor[0x74];
	// MS_UINT8		bStringLanguage[10];
	// MS_UINT8		bStringManufacture[0xFF];
	// MS_UINT8		bStringProduct[0xFF];
	// MS_UINT8		bStringSerialN[0xFF];

	/* <3>.For ISO Information */
	// MS_UINT8		bISOTransferEnable;
	// MS_UINT32		wISOiTDAddress[1024];

} Host20_Attach_Device_Structure;

#define HOST20_Attach_Device_Speed_Full		0x00
#define HOST20_Attach_Device_Speed_Low		0x01
#define HOST20_Attach_Device_Speed_High		0x02

/* <3.7>.Control Command Structure */
typedef struct {
	/* (In/Out),(Standard...),(Device/Interface...) */
	MS_UINT8	bmRequestType;
	MS_UINT8	bRequest;	// GetStatus .....
	MS_UINT8	wValueLow;	// Byte2
	MS_UINT8	wValueHigh;	// Byte3
	MS_UINT8	wIndexLow;	// Byte4
	MS_UINT8	wIndexHigh;	// Byte5
	MS_UINT8	wLengthLow;	// Byte6
	MS_UINT8	wLengthHigh;	// Byte7
} Host20_Control_Command_Structure;

#define HOST20_CONTROL_GetStatus		0x00
#define HOST20_CONTROL_ClearFeature		0x01
#define HOST20_CONTROL_SetFeature		0x03
#define HOST20_CONTROL_SetAddress		0x05
#define HOST20_CONTROL_GetDescriptor		0x06
#define HOST20_CONTROL_SetDescriptor		0x07
#define HOST20_CONTROL_GetConfiguration		0x08
#define HOST20_CONTROL_GetInterface		0x0A
#define HOST20_CONTROL_SetInterface		0x0B
#define HOST20_CONTROL_SyncFrame		0x0C

#define HOST20_HID_GetReport			0x01
#define HOST20_HID_GetIdle			0x02
#define HOST20_HID_GetProtocol			0x03
#define HOST20_HID_SetReport			0x09
#define HOST20_HID_SetIdle			0x0A
#define HOST20_HID_SetProtocol			0x0B

/* <3.8>.BufferPointerArray */
typedef struct {
	MS_UINT32	BufferPointerArray[8];
} Host20_BufferPointerArray_Structure;

/* =================== 4.Extern Function Definition ======================================================= *
 * ======================================================================================================== */

extern MS_UINT8		flib_Host20_Init(struct usb_hcd *hcd, MS_UINT8 wForDevice_B, MS_UINT16 wDelay);
extern MS_UINT8		flib_Host20_Close(struct usb_hcd *hcd, MS_UINT8 bForDeviceB);

// extern MS_UINT8	flib_Host20_ISR(void);
extern MS_UINT8		flib_Host20_PortBusReset(struct usb_hcd *hcd);
extern MS_UINT8		flib_Host20_Suspend(struct usb_hcd *hcd);
extern MS_UINT8		flib_Host20_Resume(void);
extern void 		flib_Host20_RemoteWakeUp(void);
extern MS_UINT8		flib_Host20_Waiting_Result(void);
extern MS_UINT8		flib_Host20_Issue_Control(struct usb_hcd *hcd, MS_UINT8 bEdNum, MS_UINT8* pbCmd, MS_UINT16 hwDataSize, MS_UINT8* pbData);
extern MS_UINT8		flib_Host20_Issue_Bulk(struct usb_hcd *hcd, MS_UINT32 hwSize, MS_UINT32 pwBufferArray, MS_UINT8 bDirection);
extern void		flib_Host20_Issue_ISO (MS_UINT8 bCheckResult, MS_UINT32 wEndPt, MS_UINT32 wMaxPacketSize, MS_UINT32 wSize, MS_UINT32 *pwBufferArray, MS_UINT32 wOffset, MS_UINT8 bDirection, MS_UINT8 bMult);

extern MS_UINT8		flib_Host20_Issue_Interrupt(struct usb_hcd *hcd, MS_UINT32 pwBuffer, MS_UINT16 hwSize);
extern MS_UINT8  flib_Host20_Issue_Interrupt_NonBlock(struct usb_hcd *hcd, MS_UINT32 pwBuffer, MS_UINT16 hwSize, MS_U32 *actual_len);
extern MS_UINT8		flib_Host20_Enumerate(MS_UINT8 bNormalEnumerate, MS_UINT8 bAddress);

extern void 		flib_DumpDeviceDescriptor(OTGH_Descriptor_Device_Struct *sDevice);
//extern void		flib_PrintDeviceInfo(void);
//extern void		flib_PrintDeviceInfo_ByInput(Host20_Attach_Device_Structure *psAttachDevice);
//extern void		flib_Host20_TimerISR(void);
// extern void		flib_Host20_TimerEnable(MS_UINT32 wTime_ms);
extern void		flib_Host20_InitStructure(struct usb_hcd *hcd);
extern qTD_Structure*	flib_Host20_GetStructure(struct usb_hcd *hcd, MS_UINT8 Type);
extern void		flib_Host20_ReleaseStructure(struct usb_hcd *hcd, MS_UINT8 Type, MS_UINT32 pwAddress);
extern void		flib_Host20_QHD_Control_Init(struct usb_hcd *hcd);
//extern void		flib_Host20_Allocate_QHD1(qHD_Structure  *psQHTemp);
extern MS_UINT8 flib_Host20_Send_qTD_Intr(struct usb_hcd *hcd, qTD_Structure  *spHeadqTD ,qHD_Structure  *spTempqHD);
extern MS_UINT8 	flib_Host20_Send_qTD2(struct usb_hcd *hcd, qTD_Structure  *spHeadqTD ,qHD_Structure  *spTempqHD, MS_UINT32 wTimeOutSec);
extern MS_UINT8		flib_Host20_Send_qTD(struct usb_hcd *hcd, qTD_Structure *spHeadqTD ,qHD_Structure *spTempqHD, MS_UINT32 wTimeOutSec);
extern void		flib_Host20_Allocate_QHD(struct usb_hcd *hcd, qHD_Structure *psQHTemp, MS_UINT8 bNextType, MS_UINT8 bAddress, MS_UINT8 bHead, MS_UINT8 bEndPt, MS_UINT32 wMaxPacketSize);

//extern MS_UINT8	flib_Host20_Issue_Control_Turbo(MS_UINT8 bEdNum, MS_UINT8* pbCmd, MS_UINT32 wDataSize, MS_UINT32 *pwPageAddress, MS_UINT32 wCurrentOffset);
//extern void		flib_Host20_Control_Command_Request(Host20_Control_Command_Structure *pbCMD, MS_UINT8 bmRequestType_Temp, MS_UINT8 bRequest_Temp, MS_UINT16 wValue_Temp, MS_UINT16 wIndex_Temp, MS_UINT16 wLength_Temp);
//extern void		flib_DumpString(MS_UINT8 *pbTemp, MS_UINT8 bSize);
extern void		flib_Host20_Interrupt_Init(struct usb_device *dev, struct usb_hcd *hcd);

//extern void		flib_Host20_Asynchronous_Enable(void);
//extern void		flib_Host20_Asynchronous_Disable(void);

//extern MS_UINT8	flib_OTGH_Checking_RemoteWakeUp(void);
//extern MS_UINT8	flib_OTGH_RemoteWakeEnable(void);
extern void		flib_Host20_StopRun_Setting(struct usb_hcd *hcd, MS_UINT8 bOption);
extern void		flib_Host20_Reset_HC(struct usb_hcd *hcd);
extern void		flib_Host20_Asynchronous_Setting(struct usb_hcd *hcd, MS_UINT8 bOption);
extern void		flib_Host20_Periodic_Setting(struct usb_hcd *hcd, MS_UINT8 bOption);
extern void		flib_Host20_SetControl1MaxPacket(struct usb_hcd *hcd, MS_UINT8 max);
extern void		flib_Host20_Fix_DataLength_ByteAlign(struct usb_hcd *hcd);

/* =================== 5.Call Extern Function Definition ================================================== *
 * ======================================================================================================== */
extern MS_UINT8		flib_Host20_Send_Receive_Bulk_Data(struct usb_hcd *hcd, void *buffer, int len, int dir_out);
extern void		flib_Host20_Bulk_Init(struct usb_device *dev, struct usb_hcd *hcd);
extern MS_UINT8		flib_Host20_Get_BulkIn_Data(struct usb_hcd *hcd, struct usb_device *dev, unsigned int pipe,
							MS_U32 buf, MS_U32 length, MS_U32 *actual_len, int timeout);

/* =================== 6.Extern Variable Definition ======================================================= *
 * ======================================================================================================== */

extern MS_UINT8 OTGH_GETDESCRIPTOR_DEVICE[];
extern MS_UINT8 OTGH_GETDESCRIPTOR_CONFIG[];
extern MS_UINT8 OTGH_SETADDRESS[];
extern MS_UINT8 OTGH_SETCONFIGURATION[];
extern MS_UINT8 OTGH_GETDESCRIPTOR_OTG[];
extern MS_UINT8 OTGH_SET_FEATURE_OTG[];
extern MS_UINT8 OTGH_SETDESCRIPTOR_DEVICE[];
//extern qHD_Structure		Host20_qHD_List_Control0; // NUSED
//extern qHD_Structure		Host20_qHD_List_Bulk0; // NUSED
//extern qHD_Structure		Host20_qHD_List_Control1; // NUSED
//extern qHD_Structure		Host20_qHD_List_Bulk1; // NUSED

//extern MS_UINT8		Host20_qTD_Manage[Host20_qTD_MAX];		/* 1=>Free 2=>used */ // NUSED
extern Host20_Init_Condition_Structure sInitCondition;

extern Host20_Attach_Device_Structure		*psDevice_AP;

/* =================== EHCI structure refactoring ======================================================= *
 * ======================================================================================================== */
struct ehci_hcd {	/* one per controller */
	/* per-HC memory pools (could be per-bus, but ...) */
	qHD_Structure	*qh_control0;
	qHD_Structure	*qh_control1;
	qHD_Structure	*qh_bulk0;
	qHD_Structure	*qh_bulk1;
	qHD_Structure	*qh_intr;

	qTD_Structure	*spBulkInqTD;
	qTD_Structure	*spIntInqTD;
	u8		*pUsbDataBuf;

	Periodic_Frame_List_Structure	*framelist;

	uchar	*qtd_pool;  /* one or more per qh */
	uchar	*qtd_manage;

	uchar	*ep0_buffer;
	uchar	bSendStatusError;
	uchar	ep_bulk0_dir;

	/* async schedule support */
	int	async;

};


//------- xHCI --------
#define PORT_PE				(1 << 1)
#define PORT_PLS_MASK		(0xf << 5)
#define PORT_LINK_STROBE	(1 << 16)

#define	XHCI_PORT_RO	((1<<0) | (1<<3) | (0xf<<10) | (1<<30))
#define XHCI_PORT_RWS	((0xf<<5) | (1<<9) | (0x3<<14) | (0x7<<25))

#define USB_SS_PORT_LS_SS_DISABLED	0x0080
#define USB_SS_PORT_LS_RX_DETECT	0x00a0

extern void xhci_enable_clock(void);
extern void xhci_ssport_set_state(struct xhc_comp *xhci, int bOn);
//--------------------


#endif	/* __DRVHOSTLIB_H__ */
