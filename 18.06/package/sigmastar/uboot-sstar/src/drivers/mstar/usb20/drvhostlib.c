/*
* drvhostlib.c- Sigmastar
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

#include "hal/platform.h"
#include "inc/common.h"
#include "inc/drvhostlib.h"

#ifdef USB_LIB_DEBUG
#define USB_LIB_PRINTF MS_MSG
#else
#define USB_LIB_PRINTF(fmt, ...)
#endif
#ifdef USB_LIB_BULK_DEBUG
#define USB_LIB_BULK_PRINTF MS_MSG
#else
#define USB_LIB_BULK_PRINTF(fmt, ...)
#endif

#define Scsi_Max_Transfer_Len	16*1024
#define wTimeWaitForConnect_ms	9500
#ifdef USB_EMBEDDED_STANDARD_COMMAND
MS_UINT8 OTGH_GETDESCRIPTOR_DEVICE_8[]	= {0x80,0x06,0x00,0x01,0x00,0x00,0x08,0x00};
MS_UINT8 OTGH_GETDESCRIPTOR_DEVICE[]	= {0x80,0x06,0x00,0x01,0x00,0x00,0x12,0x00};
MS_UINT8 OTGH_GETDESCRIPTOR_CONFIG[]	= {0x80,0x06,0x00,0x02,0x00,0x00,0x08,0x00};
MS_UINT8 OTGH_SETADDRESS[]		= {0x00,0x05,0x03,0x00,0x00,0x00,0x00,0x00};
MS_UINT8 OTGH_SETCONFIGURATION[]	= {0x00,0x09,0x01,0x00,0x00,0x00,0x00,0x00};
MS_UINT8 OTGH_SET_FEATURE_OTG[]		= {0x00,0x03,0x03,0x00,0x00,0x00,0x00,0x00};

 /* Set Device Descriptor */
MS_UINT8 OTGH_SETDESCRIPTOR_DEVICE[]	= {0x00,0x07,0x00,0x01,0x00,0x00,0x12,0x00};
#endif

MS_UINT8 QtdBuf[ Host20_qTD_SIZE * Host20_qTD_MAX + 0x20*4] __attribute__ ((aligned (128)));

qHD_Structure  Host20_qHD_List_Control0 __attribute__ ((aligned (128)));
qHD_Structure  Host20_qHD_List_Control1 __attribute__ ((aligned (128)));
qHD_Structure  Host20_qHD_List_Bulk0 __attribute__ ((aligned (128)));
qHD_Structure  Host20_qHD_List_Bulk1 __attribute__ ((aligned (128)));
qHD_Structure  Host20_qHD_List_Intr __attribute__ ((aligned (128)));

Periodic_Frame_List_Structure  Host20_FramList __attribute__ ((aligned (4096)));

static MS_UINT16 waIntervalMap[11]={1,2,4,8,16,32,64,128,256,512,1024};

MS_UINT8 Host20_qTD_Manage[Host20_qTD_MAX];	// 1=>Free 2=>used

MS_UINT8 UsbCtrlBuf[0x100] __attribute__ ((aligned (128)));
MS_UINT8 UsbDataBuf[0x100] __attribute__ ((aligned (128)));

/* debug functions for dump QTD & QH data */
void Dump_QTD(MS_UINT32 addr);
void Dump_Data(MS_UINT32 addr, MS_UINT16 sz);

// TODO: FIXME or REMOVE IT
#if defined(USB_BUF_ALIGNED_PATCH)
#define ALIGNED_BUF_SIZE	24 * 1024
__declspec( align(4096) ) MS_UINT8 usb_temp_buf[ALIGNED_BUF_SIZE];
#endif

#if defined(CONFIG_USB_BOUNCE_BUF_PATCH)
#define BOUNCE_BUF_SIZE	24 * 1024
MS_UINT8 usb_bounce_buf[BOUNCE_BUF_SIZE] __attribute__ ((aligned (4096)));
#endif

static struct ehci_hcd ms_ehci;

MS_U32 MsOS_USB_PA2KSEG1(MS_U32 addr)
{
	//return ((MS_U32)(((MS_U32)addr) | 0x80000000));
	return addr;
}

MS_U32 MsOS_USB_PA2KSEG0(MS_U32 addr)
{
	//return ((MS_U32)(((MS_U32)addr) | 0x40000000));
	return addr;
}

MS_U32 MsOS_USB_VA2PA(MS_U32 addr)
{
	//return ((MS_U32)(((MS_U32)addr) & 0x1fffffff));
	return addr;
}

/* ====================================================================
 * Function Name: flib_Host20_Bulk_Init
 * Description:
 * Input: none
 * OutPut: none
 * ==================================================================== */
void flib_Host20_Bulk_Init(struct usb_device *dev, struct usb_hcd *hcd)
{
	struct usb_endpoint_descriptor *ep,*ep2;
	struct ehci_hcd *ehci = hcd->ehci;
	MS_UINT8 ii;

	ep = &dev->config.if_desc[0].ep_desc[0];
	ep2 = &dev->config.if_desc[0].ep_desc[1];

	USB_LIB_PRINTF("[USB] no_of_ep: %d\n", dev->config.if_desc[0].no_of_ep);

	for (ii=0; ii<dev->config.if_desc[0].no_of_ep; ii++)
	{
		if (dev->config.if_desc[0].ep_desc[ii].bmAttributes == OTGH_ED_BULK)
		{
			USB_LIB_PRINTF("[USB] find bulk ep: %d\n", ii);
			ep=&dev->config.if_desc[0].ep_desc[ii];
			ii++;
			break;
		}
	}

	for (;ii<dev->config.if_desc[0].no_of_ep; ii++)
	{
		if (dev->config.if_desc[0].ep_desc[ii].bmAttributes == OTGH_ED_BULK)
		{
			USB_LIB_PRINTF("[USB] find bulk ep2: %d\n", ii);
			ep2=&dev->config.if_desc[0].ep_desc[ii];
			break;
		}
	}

	USB_LIB_PRINTF("\n[USB] bulk max packet size: ep(%s) 0x%x, ep2(%s) 0x%x\n",
		(ep->bEndpointAddress & 0x80) ? "in" : "out", ep->wMaxPacketSize,
		(ep2->bEndpointAddress & 0x80) ? "in" : "out", ep2->wMaxPacketSize);

	/* <5>.Hook the qHD */
	if ((ep->bmAttributes)==OTGH_ED_BULK)
	{
		/* <5.1>.stop Asynchronous Schedule */
		flib_Host20_Asynchronous_Setting(hcd, HOST20_Disable);

		/* <5.2>.Hook the qHD for ED0~ED3 */
		/* Circle the QH: CtrQH -> Bulk0QH -> Bulk1QH -> CtrQH */
		ehci->qh_control1->bNextQHDPointer = (VA2PA((MS_UINT32)ehci->qh_bulk0) >> 5);
		ehci->qh_bulk0->bNextQHDPointer = (VA2PA((MS_UINT32)ehci->qh_bulk1) >> 5);
		ehci->qh_bulk1->bNextQHDPointer = (VA2PA((MS_UINT32)ehci->qh_control0) >> 5);

#if (_USB_FLUSH_BUFFER == 1)
		/* make sure QH chain modification has flushed to mem */
		Chip_Flush_Memory();
#endif
		/* <5.2>.Enable Asynchronous Schedule */
		flib_Host20_Asynchronous_Setting(hcd, HOST20_Enable);
	}

#if 0 // moved to ubootglue.c
	FirstBulkIn = 1;
	FirstBulkOut = 1;
#endif
	ehci->spBulkInqTD = NULL;

	/* Setting Max Packet Size */
	ehci->qh_bulk0->bMaxPacketSize = ep->wMaxPacketSize;	//wMaxPacketSize;
	ehci->qh_bulk1->bMaxPacketSize = ep2->wMaxPacketSize;	//wMaxPacketSize;

	/* set device address */
	ehci->qh_bulk0->bDeviceAddress = dev->devnum;
	ehci->qh_bulk1->bDeviceAddress = dev->devnum;

	/* set endpoint# address */
	ehci->qh_bulk0->bEdNumber = (ep->bEndpointAddress) & 0x0f;
	ehci->qh_bulk1->bEdNumber = (ep2->bEndpointAddress) & 0x0f;	//set endpoint address

	ehci->ep_bulk0_dir = (((ep->bEndpointAddress) & 0x80) > 0) ? OTGH_Dir_IN : OTGH_Dir_Out;
	USB_LIB_PRINTF("[USB] bulk0 is %s\n", ehci->ep_bulk0_dir ? "in" : "out");
}

/* ====================================================================
 * Function Name: flib_Host20_Interrupt_Init
 * Description:
 * //Reserve=> <1>.If Periodic Enable => Disable Periodic
 * <2>.Disable all the Frame List (Terminal=1)
 * <3>.Hang the Interrupt-qHD-1 to Frame List
 *
 * Input: wInterval=1~16 => Full Speed => 1ms ~ 32 sec
 *                          High Speed => 125us ~ 40.96 sec
 * OutPut:
 * ==================================================================== */
void  flib_Host20_Interrupt_Init(struct usb_device *dev, struct usb_hcd *hcd)
{
	MS_UINT32 i,j;
	MS_UINT32 wSize;
	MS_UINT8  x;
	MS_UINT8 *pData;
	MS_UINT16 wForceInterval = 0;
	struct usb_interface *iface;
	struct ehci_hcd *ehci = hcd->ehci;
	MS_UINT32 ehci_base = hcd->ehc_base;

	iface = &dev->config.if_desc[0];

	/* <1>.Disable the Periodic */
	flib_Host20_Periodic_Setting(hcd, HOST20_Disable);

	/* <2>.Init qHD for Interrupt(7~9) Scan the ED */
	x = mwHost20Port(ehci_base,0x10);
	x = (x&0xF3)|((MS_UINT8)HOST20_USBCMD_FrameListSize_256<<2);
	mwHost20Port_wr(ehci_base,0x10, x);

	for (i=0; i < (iface->desc.bNumEndpoints); i++)
	{
		/* that is an interrupt endpoint */
		if (iface->ep_desc[i].bmAttributes==OTGH_ED_INT)
		{
			wSize = iface->ep_desc[i].wMaxPacketSize;
			wForceInterval = iface->ep_desc[i].bInterval;
			ehci->qh_intr =(qHD_Structure*)KSEG02KSEG1(&Host20_qHD_List_Intr);
			flush_cache((ulong)&Host20_qHD_List_Intr, sizeof(qHD_Structure));

			pData=(MS_UINT8*)ehci->qh_intr;

			for (j = 0; (j < sizeof(qHD_Structure)) && (j<Host20_qHD_SIZE); j++)
				*(pData + j) = 0;

			/* Address=0,Head=1,EndPt=0,Size */
			flib_Host20_Allocate_QHD(hcd, ehci->qh_intr, HOST20_HD_Type_QH, dev->devnum, 0, (i+1), wSize);

			ehci->qh_intr->bHighBandwidth = 1;
			ehci->qh_intr->bInterruptScheduleMask = 1;
			ehci->qh_intr->bEdSpeed = hcd->bSpeed;
			ehci->qh_intr->bDataToggleControl = 0;
			/* interrupt schedule */
			ehci->qh_intr->bInterruptScheduleMask = 1;
			/* 1~3, MULTI =1 */
			ehci->qh_intr->bHighBandwidth = 1;

			/* support only one interrupt pipe */
			break;
		}
	}

	/* point to itself */
	ehci->qh_intr->bNextQHDPointer = (VA2PA((MS_UINT32)ehci->qh_intr) >> 5);
	/* terminated */
	ehci->qh_intr->bTerminate = 1;

	ehci->spIntInqTD = NULL;

	ehci->framelist = (Periodic_Frame_List_Structure*) KSEG02KSEG1(&Host20_FramList);
	flush_cache((ulong)&Host20_FramList, sizeof(Periodic_Frame_List_Structure));

	pData=(MS_UINT8 *)ehci->framelist;
	for (j=0;j<sizeof(Periodic_Frame_List_Structure);j++)
		*(pData+j)=0;

	/* <3>.Link qHD to the FameListCell by wInterval */
	for (i = 0; i < Host20_Preiodic_Frame_List_MAX; i++)
		ehci->framelist->sCell[i].bTerminal = 1;

	/* Find the Interval-X */
	x = 0;
	while(waIntervalMap[x]<wForceInterval)
	{
		x++;
		if(x > 10)
		{
			MS_MSG("[USB] Interval Input Error...\n");
			return;
		}
	};

	for (i = 0; i < Host20_Preiodic_Frame_List_MAX;  i = i + waIntervalMap[x])
	{
		ehci->framelist->sCell[i].bLinkPointer = (VA2PA((MS_UINT32)(ehci->qh_intr))>>5);
		ehci->framelist->sCell[i].bTerminal = 0;
		ehci->framelist->sCell[i].bType = HOST20_HD_Type_QH;
	}

	/* <4>.Set Periodic Base Address */
	mwHost20_PeriodicBaseAddr_Set(hcd->ehc_base, VA2PA((MS_UINT32)ehci->framelist));

	/* <5>.Enable the periodic */
	flib_Host20_Periodic_Setting(hcd, HOST20_Enable);
}

/* ====================================================================
 * Function Name: flib_Host20_Periodic_Setting
 * Description:
 * Input:
 * OutPut:
 * ==================================================================== */
void flib_Host20_Periodic_Setting(struct usb_hcd *hcd, MS_UINT8 bOption)
{
	MS_UINT32 ehci_base = hcd->ehc_base;
	MS_UINT32 polling_count = 0;

	if (bOption == HOST20_Enable)
	{
		/* <1>.If Already enable => return */
		if (mwHost20_USBSTS_PeriodicStatus_Rd(ehci_base) > 0)
			return ;

		/* <2>.Disable Periodic */
		mbHost20_USBCMD_PeriodicEnable_Set(ehci_base);

		/* <3>.Polling Status */
		while (polling_count++ < 16 * 125) {
			if (mwHost20_USBSTS_PeriodicStatus_Rd(ehci_base) > 0) {
				return; /* succeed */
			}
			udelay(1); /* delay 1us */
		}

		/* timeout: 16 microframes */
		MS_MSG("[USB] timeout when starting periodic schedule\n");
	}
	else if (bOption == HOST20_Disable)
	{
		/* <1>.If Already Disable => return */
		if (mwHost20_USBSTS_PeriodicStatus_Rd(ehci_base) == 0)
			return ;

		/* <2>.Enable Periodic */
		mbHost20_USBCMD_PeriodicEnable_Clr(ehci_base);

		/* <3>.Polling Status */
		while (polling_count++ < 16 * 125) {
			if (mwHost20_USBSTS_PeriodicStatus_Rd(ehci_base) == 0) {
				return; /* succeed */
			}
			udelay(1); /* delay 1us */
		}

		/* timeout: 16 microframes */
		MS_MSG("[USB] timeout when stopping periodic schedule\n");
	}
	else
	{
		MS_MSG("[USB] ??? Input Error 'flib_Host20_Periodic_Setting'...\n");
	}
}

/* ====================================================================
 * Function Name: flib_Host20_Issue_Interrupt
 * Description:
 * Input:
 * OutPut:
 * ==================================================================== */
MS_UINT8  flib_Host20_Issue_Interrupt(struct usb_hcd *hcd, MS_UINT32 pwBuffer, MS_UINT16 hwSize)
{
	qTD_Structure *spTempqTD;
	struct ehci_hcd *ehci = hcd->ehci;
	MS_UINT32 workbuf = 0;
	MS_UINT8 bResult;

	if ( pwBuffer != (MS_UINT32) KSEG02KSEG1(pwBuffer) )
	{
		flush_cache((ulong)pwBuffer,hwSize);     //flush buffer to uncached buffer
#if (_USB_FLUSH_BUFFER == 1)
		Chip_Flush_Memory();
#endif
		pwBuffer=(MS_UINT32)KSEG02KSEG1(pwBuffer);
	}

	/* <1>.Fill TD */
	/* The qTD will be release in the function "Send" */
	spTempqTD = flib_Host20_GetStructure(hcd, Host20_MEM_TYPE_qTD);

	if(spTempqTD == NULL)
		return HOST20_FATAL;

	workbuf = pwBuffer;

	spTempqTD->bTotalBytes=hwSize;
	/* cache issue, if cache enable */
	spTempqTD->ArrayBufferPointer_Word[0] = VA2PA(workbuf);
	spTempqTD->ArrayBufferPointer_Word[1] = 0;
	spTempqTD->ArrayBufferPointer_Word[2] = 0;
	spTempqTD->ArrayBufferPointer_Word[3] = 0;
	spTempqTD->ArrayBufferPointer_Word[4] = 0;

	/* <2>.Analysis the Direction */
	spTempqTD->bPID=HOST20_qTD_PID_IN;

	/* <3>.Send TD */
	bResult=flib_Host20_Send_qTD2(hcd, spTempqTD, ehci->qh_intr, 10);

	return bResult;
}

/* ====================================================================
 * Function Name: flib_Host20_Issue_Interrupt_NonBlock
 * Description:
 * Input:
 * OutPut:
 * ==================================================================== */
MS_UINT8  flib_Host20_Issue_Interrupt_NonBlock(struct usb_hcd *hcd, MS_UINT32 pwBuffer, MS_UINT16 hwSize, MS_U32 *actual_len)
{
	qTD_Structure *spTempqTD;
	struct ehci_hcd *ehci = hcd->ehci;
	MS_UINT32 workbuf = 0;
	MS_UINT8 bResult=HOST20_OK;

	if (mwHost20_PORTSC_ConnectStatus_Rd(hcd->ehc_base) == 0)
	{
		MS_MSG("[USB] Port%d device disconn\n", hcd->port_index);
		bResult = HOST20_FATAL;
		hcd->urb_status |= USB_ST_DISCONNECT;
		flib_Host20_Periodic_Setting(hcd, HOST20_Disable);
		ehci->qh_intr->bOverlay_Status=0;      //clear halt status
		flib_Host20_ReleaseStructure(hcd, Host20_MEM_TYPE_qTD, (MS_UINT32)ehci->spIntInqTD);
		return bResult;
	}

	if (hcd->FirstIntIn == 1)
	{
		// If periodic still running then return, else release first qtd point
		if (ehci->spIntInqTD->bStatus_Active==1)
			hcd->IntrIn_Complete = 0;
		else
		{
			hcd->IntrIn_Complete = 1;
			hcd->FirstIntIn = 0;
			*actual_len = (MS_UINT32)(hwSize - (MS_UINT16)ehci->spIntInqTD->bTotalBytes);
			flib_Host20_ReleaseStructure(hcd, Host20_MEM_TYPE_qTD, (MS_UINT32)ehci->spIntInqTD);
		}
		return bResult;
	}

	hcd->IntrIn_Complete = 0;
	hcd->FirstIntIn = 1;

	ehci->spIntInqTD = (qTD_Structure*)PA2VA((( MS_UINT32)(ehci->qh_intr->bOverlay_NextqTD))<<5);

	if ( pwBuffer != (MS_UINT32) KSEG02KSEG1(pwBuffer) )
	{
		flush_cache((ulong)pwBuffer,hwSize);     //flush buffer to uncached buffer
#if (_USB_FLUSH_BUFFER == 1)
		Chip_Flush_Memory();
#endif
		pwBuffer=(MS_UINT32)KSEG02KSEG1(pwBuffer);
	}

	workbuf = pwBuffer;

	/* <1>.Fill TD */
	/* The qTD will be release in the function "Send" */
	spTempqTD = flib_Host20_GetStructure(hcd, Host20_MEM_TYPE_qTD);
	spTempqTD->bTotalBytes=hwSize;
	/* cache issue, if cache enable */
	spTempqTD->ArrayBufferPointer_Word[0] = VA2PA(workbuf);
	spTempqTD->ArrayBufferPointer_Word[1] = 0;
	spTempqTD->ArrayBufferPointer_Word[2] = 0;
	spTempqTD->ArrayBufferPointer_Word[3] = 0;
	spTempqTD->ArrayBufferPointer_Word[4] = 0;

	/* <2>.Analysis the Direction */
	spTempqTD->bPID=HOST20_qTD_PID_IN;

	/* <3>.Send TD */
	bResult=flib_Host20_Send_qTD_Intr(hcd, spTempqTD, ehci->qh_intr);

	return bResult;
}

/* *********************************************************************************
 * *********************************************************************************
 *                     *** Group-1:Main Function ***
 * *********************************************************************************
 * ********************************************************************************* */

unsigned char mwHost20Port(int addr, int bOffset)
{
	if (bOffset & 1)
		return(*((MS_UINT8 volatile *) ( addr + bOffset*2 - 1)));
	else	/* even */
		return(*((MS_UINT8 volatile *) ( addr + bOffset*2)));
}

void mwHost20Port_wr(int addr, int bOffset, int value)
{
	if (bOffset & 1)
		(*((MS_UINT8 volatile *) ( addr + bOffset*2 -1))) = value;
	else	/* even */
		(*((MS_UINT8 volatile *) ( addr + bOffset*2))) = value;
}

/* bOffset should be 32 bits alignment */
void mwHost20Portw(int addr, int bOffset, int value)
{
	*((MS_UINT32 volatile *)(addr + bOffset*2)) = value & 0xffff;
	*((MS_UINT32 volatile *)(addr + bOffset*2 + 4)) = (value >> 16) & 0xffff;
}

int mwHost20Bit_Rd(int addr, int bByte, int wBitNum)
{
	return (mwHost20Port(addr, bByte) & wBitNum);
}

void mwHost20Bit_Set(int addr, int bByte, int wBitNum)
{
	MS_UINT8 temp;
	temp = mwHost20Port(addr, bByte);
	temp |= wBitNum;
	mwHost20Port_wr(addr, bByte, temp);
}

void  mwHost20Bit_Clr(int addr, int bByte, int wBitNum)
{
	MS_UINT8 temp;
	temp = mwHost20Port(addr, bByte);
	temp &= ~wBitNum;
	mwHost20Port_wr(addr, bByte,temp);
}

/* ====================================================================
 * Function Name: flib_Host20_Init
 * Description: Init the Host HW and prepare the ED/TD
 *   <1>.Init All the Data Structure
 *   <1.1>.Build control list
 *       <1.2>.Build Bulk list
 *       <1.3>.Build Interrupt list
 *       <1.4>.Build ISO list (Reserved)
 *   <2>.Reset the chip
 *   <3>.Set HW register
 *       <3.1>.Enable FIFO-Enable(0x100->Bit5) & FPGA-Mode-Half-Speed (0x100->Bit1)
 *       <3.2>.Enable interrupts
 *       <3.3>.Periodic List Base Address register
 *       <3.4>.USBCMD (Interrupt/Threshod/Frame List/Run-Stop)
 *
 * Input: wTimeWaitForConnect_ms:The time of waitting for connecting
 * OutPut: 0:Device do not connect
 *           1:Host init ok
 *           2:Bus Rest Fail
 * ==================================================================== */
MS_UINT8 flib_Host20_Init(struct usb_hcd *hcd, MS_UINT8 wForDevice_B, MS_UINT16 wDelay)
{
	MS_UINT8 wValue = 0;
	MS_UINT16 wTimer_ms = 0;
	MS_UINT32 ehci_base = hcd->ehc_base;
	MS_UINT32 utmi_base = hcd->utmi_base;

	USB_DELAY(wDelay);

	if (mwHost20_PORTSC_ConnectStatus_Rd(ehci_base) == 0)
	{
		MS_MSG("[USB] No USB is connecting\n");
		return (0);
	}

	/* <1>.Waiting for the Device connect */
	if (wForDevice_B == 0) {
		/* host controller reset */
		flib_Host20_Reset_HC(hcd);
	}

	wValue = 0;
	wTimer_ms = 0;
	do {
		wValue = mwHost20_PORTSC_ConnectStatus_Rd(ehci_base);

		if (wValue==0) {
			/* 10, wait 1 ms */
			USB_DELAY(1);
			wTimer_ms++;
		}

		/* Case1:Waiting for 10 sec=10000 */
		/* Case2:Waiting for 100 ms =100 */
		if (wTimer_ms > wTimeWaitForConnect_ms)
		{
			MS_MSG("[USB] ??? Waiting for Peripheral Connecting Fail...\n");
			return (0);
		}
	}while(wValue == 0);

	mwHost20_Misc_EOF1Time_Set(ehci_base,Host20_EOF1Time);

	/* <2>.Init All the Data Structure & write Base Address register */
	flib_Host20_InitStructure(hcd);

	/* Write Base Address to Register */
	USB_LIB_PRINTF("[USB] Async base addr: 0x%x \n", hcd->ehci->async);
	mwHost20_CurrentAsynchronousAddr_Set(ehci_base, hcd->ehci->async);
	USB_LIB_PRINTF("[USB] Reg 0x28: 0x%lx 0x%lx\n", *((MS_UINT32 volatile *)(ehci_base + 0x28*2)),
				*((MS_UINT32 volatile *)(ehci_base + 0x28*2 + 4)));

	if (wForDevice_B == 0) {
		if (flib_Host20_PortBusReset(hcd) > 0)
			return(2);
	}

	if (hcd->bSpeed == 0) /* full speed */
	{
		mwHost20Bit_Clr(utmi_base, 0x9, MS_BIT7);
		mwHost20_Misc_EOF1Time_Set(ehci_base, 2);
	}
	else if (hcd->bSpeed == 2)	/* high speed */
	{
		/* HS rx robust enable */
		mwHost20Bit_Set(utmi_base, 0x9, MS_BIT7);
		mwHost20_Misc_EOF1Time_Set(ehci_base, 3);
	}

	flib_Host20_QHD_Control_Init(hcd);

	return (1);
}

/* ====================================================================
 * Function Name: flib_Host20_Close
 * Description:
 *   <1>.Suspend Host
 *   <2>.Disable the interrupt
 *   <3>.Clear Interrupt Status
 *   <4>.Issue HW Reset
 *   <5>.Free the Allocated Memory
 * Input:
 * OutPut:
 * ==================================================================== */
MS_UINT8 flib_Host20_Close(struct usb_hcd *hcd, MS_UINT8 bForDeviceB)
{
	MS_UINT32 wTemp;
	MS_UINT32 ehci_base = hcd->ehc_base;

	if (mwHost20_USBINTR_Rd(ehci_base) > 0) {
		/* <1>.Suspend Host */
		if (bForDeviceB == 0)
		{
			flib_Host20_Suspend(hcd);
		}
		else
			flib_Host20_StopRun_Setting(hcd, HOST20_Disable);

		/* <2>.Disable the interrupt */
		mwHost20_USBINTR_Set(ehci_base, 0);

		/* <3>.Clear Interrupt Status */
		wTemp = mwHost20_USBSTS_Rd(ehci_base);
		wTemp = wTemp & 0x0000003F;
		mwHost20_USBSTS_Set(ehci_base, wTemp);
	}
	return (1);
}

inline void flib_Host20_Fix_DataLength_ByteAlign(struct usb_hcd *hcd)
{
	mwHost20Bit_Set(hcd->ehc_base, 0x81, MS_BIT6);
}

/* ====================================================================
 * Function Name: flib_Host20_StopRun_Setting
 * Description:
 * Input:
 * OutPut:
 * ==================================================================== */
void flib_Host20_StopRun_Setting(struct usb_hcd *hcd, MS_UINT8 bOption)
{
	MS_UINT32 ehci_base = hcd->ehc_base;
	MS_UINT32 polling_count = 0;

	if (bOption == HOST20_Enable) {
		if (mbHost20_USBCMD_RunStop_Rd(ehci_base) > 0)
			return;

		mbHost20_USBCMD_RunStop_Set(ehci_base);

		while (polling_count++ < 16 * 125) {
			if (mwHost20_USBSTS_HCHalted_Rd(ehci_base) == 0) {
				return; /* succeed */
			}
			udelay(1); /* delay 1us */
		}

		/* timeout: 16 microframes */
		MS_MSG("[USB] timeout when starting HC\n");
	}
	else if (bOption == HOST20_Disable) {
		if (mbHost20_USBCMD_RunStop_Rd(ehci_base) == 0)
			return;

		mbHost20_USBCMD_RunStop_Clr(ehci_base);

		while (polling_count++ < 16 * 125) {
			if (mwHost20_USBSTS_HCHalted_Rd(ehci_base) > 0) {
				return; /* succeed */
			}
			udelay(1); /* delay 1us */
		}

		/* timeout: 16 microframes */
		MS_MSG("[USB] timeout when stopping HC\n");
	}
	else {
		MS_MSG("[USB] ??? Input Error 'flib_Host20_StopRun_Setting'...\n");
		MS_MSG("[USB] USBCMD's Run/Stop bit remains unchanged\n");
	}
}

/* ====================================================================
 * Function Name: flib_Host20_Reset_HC
 * Description:
 * Input:
 * OutPut:
 * ==================================================================== */
void flib_Host20_Reset_HC(struct usb_hcd *hcd)
{
	MS_UINT32 ehci_base = hcd->ehc_base;
	MS_UINT32 tmp;

	/* host controller reset */
	mbHost20_USBCMD_HCReset_Set(ehci_base);

	tmp = 0;
	while(mbHost20_USBCMD_HCReset_Rd(ehci_base)>0)
	{
		tmp++;
		USB_DELAY(1);
		/* 100ms timeout*/
		if(tmp > 100)
		{
			MS_MSG("[USB] can not reset UHC\n");
			break;
		}
	}
}

/* ====================================================================
 * Function Name: flib_Host20_Asynchronous_Setting
 * Description:
 * Input:
 * OutPut:
 * ==================================================================== */
void flib_Host20_Asynchronous_Setting(struct usb_hcd *hcd, MS_UINT8 bOption)
{
	MS_UINT32 ehci_base = hcd->ehc_base;
	MS_UINT32 polling_count = 0;

	if (bOption == HOST20_Enable) {
		if (mwHost20_USBSTS_AsynchronousStatus_Rd(ehci_base) > 0)
			return;

		mbHost20_USBCMD_AsynchronousEnable_Set(ehci_base);

		while (polling_count++ < 16 * 125) {
			if (mwHost20_USBSTS_AsynchronousStatus_Rd(ehci_base) > 0) {
				return; /* succeed */
			}
			udelay(1); /* delay 1us */
		}

		/* timeout: 16 microframes */
		MS_MSG("[USB] timeout when starting asynchronous schedule\n");
	}
	else if (bOption==HOST20_Disable) {
		if (mwHost20_USBSTS_AsynchronousStatus_Rd(ehci_base) == 0)
			return;

		mbHost20_USBCMD_AsynchronousEnable_Clr(ehci_base);

		while (polling_count++ < 16 * 125) {
			if (mwHost20_USBSTS_AsynchronousStatus_Rd(ehci_base) == 0) {
				return; /* succeed */
			}
			udelay(1); /* delay 1us */
		}

		/* timeout: 16 microframes */
		MS_MSG("[USB] timeout when stopping asynchronous schedule\n");
	}
	else {
		MS_MSG("[USB] ??? Input Error 'flib_Host20_Asynchronous_Setting'...\n");
		// while(1);
	}
}

#if defined( USB_BAD_DEVICE_RETRY_PATCH )
extern MS_BOOL force_FS;

void flib_Host20_ForceSpeed(struct usb_hcd *hcd, MS_U32 port)
{
	MS_UINT32 ehci_base = hcd->ehc_base;

	MS_MSG("[USB] flib_Host20_ForceSpeed ++\n");

	writeb(readb(ehci_base+0x40*2) | 0x80, ehci_base+0x40*2);	/* force full speed */
	writeb(readb(ehci_base+0x34*2) | 0x40, ehci_base+0x34*2);

	MS_MSG("[USB] Force HS Reg: %x\n", readb(ehci_base+0x40*2));
}
#endif

/* ====================================================================
 * Function Name: flib_Host20_PortBusReset
 * Description:
 *   <1>.Waiting for HCHalted=0
 *   <2>.Write PortEnable=0(Reserved for Faraday-IP)
 *   <3>.Write PortReset=0
 *   <4>.Wait time
 *   <5>.Write PortReset=0
 *   <6>.Waiting for IRS->PortChangeDetect
 * Input:
 * OutPut:
 * ==================================================================== */
MS_UINT8 flib_Host20_PortBusReset(struct usb_hcd *hcd)
{
	MS_UINT32 wTmp;
	MS_UINT32 ehci_base = hcd->ehc_base;
	MS_UINT32 utmi_base = hcd->utmi_base;

#if defined( USB_BAD_DEVICE_RETRY_PATCH )
	if(force_FS == TRUE)
		flib_Host20_ForceSpeed(hcd, hcd->port_index);
#endif

	/* <1>.Disable RunStop */
	USB_LIB_PRINTF("[USB] disable run\n");
	if (mbHost20_USBCMD_RunStop_Rd(ehci_base) > 0)
		flib_Host20_StopRun_Setting(hcd, HOST20_Disable);

	/* <2>.Write PortReset=0 */

	mwHost20Portw(utmi_base, 0x2C, 0x00000010);
	mwHost20Port_wr(utmi_base, 0x2A, 0x80);

	mwHost20_PORTSC_PortReset_Set(ehci_base);

	/* <3>.Total Wait time=>70ms */
	USB_DELAY(50);
	mwHost20Port_wr(utmi_base, 0x2A, UTMI_DISCON_LEVEL_2A);
	USB_DELAY(20);

	// flib_Debug_LED_Off_All();; /* GPIO-High */

	/* <4>.Write PortReset=0 */
	mwHost20_PORTSC_PortReset_Clr(ehci_base);

	mwHost20Portw(utmi_base, 0x2C, UTMI_ALL_EYE_SETTING);

	/* <5>.Waiting for IRS->PortChangeDetect */
	wTmp = 0;

	while (1)
	{
		if (mwHost20_PORTSC_PortReset_Rd(ehci_base) == 0)
			break;

		wTmp++;

		if (wTmp > 20000)
		{
			MS_MSG("[USB] ??? Error waiting for Bus Reset Fail...==> Reset HW Control\n");
			flib_Host20_Reset_HC(hcd);
				return (1);
		}
	}

#if 0
	if (mwHost20_PORTSC_ForceSuspend_Rd())
	{
		USB_LIB_PRINTF("[USB] port suspend\n");
		mwHost20_PORTSC_ForceResume_Set();	/* force resume */
		USBDELAY(14);
		mwHost20_PORTSC_ForceResume_Clr();
	}
#endif

	/* UTMI TX/RX Reset */
	mwHost20Bit_Set(utmi_base, 0x6, MS_BIT0 | MS_BIT1);
	mwHost20Bit_Clr(utmi_base, 0x6, MS_BIT0 | MS_BIT1);

	/* <6>.Enable RunStop Bit */
	flib_Host20_StopRun_Setting(hcd, HOST20_Enable);

	/* wait some slow device to be ready */
	USB_DELAY(5);

	/* <7>.Detect Speed */
	hcd->bSpeed= mwOTG20_Control_HOST_SPD_TYP_Rd(ehci_base);
	MS_MSG ("[USB] Host Speed:%x \n",  hcd->bSpeed);

	/* <8>.Delay 20 ms ?? */
	return (0);
}

/* ====================================================================
 * Function Name: flib_Host20_Suspend
 * Description:
 *   <1>.Make sure PortEnable=1
 *   <2>.Write PORTSC->Suspend=1
 *   <3>.Waiting for the ISR->PORTSC->Suspend=1
 * Input:
 * OutPut: 0:OK
 *         1:Fail
 * ==================================================================== */
MS_UINT8 flib_Host20_Suspend(struct usb_hcd *hcd)
{
	MS_UINT32 ehci_base = hcd->ehc_base;

	if (mbHost20_USBCMD_RunStop_Rd(ehci_base) == 0)
		return(1);

	/* <1>.Make sure PortEnable=1 */
	if (mwHost20_PORTSC_EnableDisable_Rd(ehci_base) == 0)
		return(1);

	/* <2>.Write PORTSC->Suspend=1 */
	/* For Faraday HW request */
	flib_Host20_StopRun_Setting(hcd, HOST20_Disable);

	/* <3>.Write PORTSC->Suspend=1 */
	mwHost20_PORTSC_ForceSuspend_Set(ehci_base);

	/* <4>.Waiting for the ISR->PORTSC->Suspend=1 */
#if 0
	flib_Host20_TimerEnable_UnLock(1);	/* 1sec */
	bExitFlag = 0;

	do {
		if (mwHost20_PORTSC_ForceSuspend_Rd() > 0)
		bExitFlag = 1;

		if (gwOTG_Timer_Counter>5) {
			bExitFlag = 1;
			MS_MSG("[USB] >>> Fail => Time Out for Waiting ForceSuspend...\n");
		}
	}
	while(bExitFlag==0);
#else
	while(mwHost20_PORTSC_ForceSuspend_Rd(ehci_base)==0);
#endif
	return (0);
}

/* ====================================================================
 * Function Name: flib_Host20_Issue_Control
 * Description:
 *   <1>.Analysis the Controller Command => 3 type
 *   <2>.Case-1:"Setup/In/Out' Format..."
 *       (get status/get descriptor/get configuration/get interface)
 *   <3>.Case-2:'Setup/In' Format...      => Faraday Driver will not need
 *       (clear feature/set feature/set address/set Configuration/set interface  )
 *   <4>.Case-3:'Setup/Out/In'
 *       (set descriptor)
 * Input:
 * OutPut: 0: OK
 *         X: >0 => Fail
 * ==================================================================== */
MS_UINT8 flib_Host20_Issue_Control (struct usb_hcd *hcd, MS_UINT8 bEdNum, MS_UINT8* pbCmd, MS_UINT16 hwDataSize, MS_UINT8* pbData)
{
	qTD_Structure  *spTempqTD;
	MS_UINT8 bReturnValue, bOneMore=1, bIsInput;
	qHD_Structure  *qh_ptr;
	struct ehci_hcd *ehci = hcd->ehci;

	if (bEdNum == 0)
		qh_ptr = ehci->qh_control0;
	else
	{
		ehci->qh_control1->bDeviceAddress = bEdNum;
		qh_ptr = ehci->qh_control1;
	}

	/* <0>.Allocate qTD & Data Buffer */
	/* 0=>qTD */
	spTempqTD = flib_Host20_GetStructure(hcd, Host20_MEM_TYPE_qTD);

	if(spTempqTD == NULL){
		return HOST20_FATAL;
	}

	/* <2.1>.Setup packet */
	/* <A>.Fill qTD */
	spTempqTD->bPID = HOST20_qTD_PID_SETUP;	/* Bit8~9 */
	spTempqTD->bTotalBytes = 8;	/* Bit16~30 */

	spTempqTD->bDataToggle = 0;	/* Bit31 */
	memcpy(ehci->ep0_buffer, pbCmd, 8);

	spTempqTD->ArrayBufferPointer_Word[0] = VA2PA((MS_UINT32)ehci->ep0_buffer);

	/* debug dump QTD data */
	// Dump_QTD(spTempqTD);
	// Dump_Data(XXX, XXX);

	/* <B>.Send qTD */
	bReturnValue= flib_Host20_Send_qTD(hcd, spTempqTD, qh_ptr, Host20_Ctrl_Timeout);
	if (bReturnValue > 0)
		goto exit_issue_control;

	/* <1>.Analysis the Controller Command */
#if 1
	bIsInput = usb_pipein(*pbCmd);

	if (hwDataSize > 0)
	/* Data Stage */
	 {
		/* <A>.Fill qTD */
			/* 0=>qTD */
			spTempqTD = flib_Host20_GetStructure(hcd, Host20_MEM_TYPE_qTD);

			if(spTempqTD == NULL){
				return HOST20_FATAL;
			}

			if (bIsInput)
				spTempqTD->bPID = HOST20_qTD_PID_IN;	/* Bit8~9 */
			else
			{
				spTempqTD->bPID = HOST20_qTD_PID_OUT;		/* Bit8~9 */
				memcpy(ehci->ep0_buffer, pbData, hwDataSize);
			}
			spTempqTD->bTotalBytes = hwDataSize;	/* Bit16~30 */
			spTempqTD->bDataToggle = 1;		/* Bit31 */

			spTempqTD->ArrayBufferPointer_Word[0] = VA2PA((MS_UINT32)ehci->ep0_buffer);

			/* <B>.Send qTD */
			bReturnValue = flib_Host20_Send_qTD(hcd, spTempqTD, qh_ptr,Host20_Ctrl_Timeout);
			if (bReturnValue>0)
				goto exit_issue_control;

			/* <C>.Waiting for result */
			if (bIsInput)
				memcpy((MS_UINT8 *)pbData, ehci->ep0_buffer, hwDataSize);

			// Dump_Data((MS_UINT16)pbData, hwDataSize);
	}

	// Send short packet
	if (bOneMore)
	{
		spTempqTD=flib_Host20_GetStructure(hcd, Host20_MEM_TYPE_qTD);
		if(spTempqTD == NULL){
			return HOST20_FATAL;
		}

		if (bIsInput)
			spTempqTD->bPID = HOST20_qTD_PID_OUT;	/* Bit8~9 */
		else
			spTempqTD->bPID = HOST20_qTD_PID_IN;	/* Bit8~9 */

		spTempqTD->bTotalBytes = 0;	/* Bit16~30 */

		spTempqTD->bDataToggle = 1;	/* Bit31 */

		bReturnValue = flib_Host20_Send_qTD(hcd, spTempqTD, qh_ptr, Host20_Ctrl_Timeout);
		if (bReturnValue > 0)
			goto exit_issue_control;
	}

#else
	USB_LIB_PRINTF("usb_pipein:%d\n",usb_pipein(*pbCmd));
	USB_LIB_PRINTF("hwDataSize:%d\n",hwDataSize);

	/* by Standard Request codes */
	switch (*(pbCmd + 1)) {
		/* <2>.Case-1:"Setup/In/Out' Format..." */
		case 0:		/* get status */
		case 6:		/* get descriptor */
		case 8: 	/* get configuration */
		case 10:	/* get interface */
		case 0xfe:	/* get Max Lun */
		/* <2.2>.In packet */
		SetupRead:
		/* <A>.Fill qTD */
			/* 0=>qTD */
			spTempqTD = flib_Host20_GetStructure(hcd, Host20_MEM_TYPE_qTD);

			if(spTempqTD == NULL){
				return HOST20_FATAL;
			}

			spTempqTD->bPID = HOST20_qTD_PID_IN;	/* Bit8~9 */
			spTempqTD->bTotalBytes = hwDataSize;	/* Bit16~30 */
			spTempqTD->bDataToggle = 1;		/* Bit31 */

			spTempqTD->ArrayBufferPointer_Word[0] = VA2PA((MS_UINT32)ehci->ep0_buffer);

			/* <B>.Send qTD */
			bReturnValue = flib_Host20_Send_qTD(hcd, spTempqTD, qh_ptr,Host20_Ctrl_Timeout);
			if (bReturnValue>0)
				goto exit_issue_control;

			/* <C>.Waiting for result */
			memcpy((MS_UINT8 *)pbData, ehci->ep0_buffer, hwDataSize);

			// Dump_Data((MS_UINT16)pbData, hwDataSize);

			/* <2.3>.Out packet */
			/* <A>.Fill qTD */
			/* 0=>qTD */
			spTempqTD=flib_Host20_GetStructure(hcd, Host20_MEM_TYPE_qTD);
			if(spTempqTD == NULL){
				return HOST20_FATAL;
			}

			spTempqTD->bPID = HOST20_qTD_PID_OUT;	/* Bit8~9 */
			spTempqTD->bTotalBytes = 0;	/* Bit16~30 */

			spTempqTD->bDataToggle = 1;	/* Bit31 */

			/* <B>.Send qTD */
			bReturnValue = flib_Host20_Send_qTD(hcd, spTempqTD, qh_ptr, Host20_Ctrl_Timeout);
			if (bReturnValue > 0)
				goto exit_issue_control;
			break;

		/* <3>.Case-2:'Setup/In' Format...  => Faraday Driver will not need */
		case 0xf0:	/* read */
		case 0xf1:	/* write */
			if (*(pbCmd) == 0xc0)
				goto SetupRead;
			else if (*(pbCmd) == 0x40)
				goto SetupWrite;
			break;

		case 1:		/* clear feature */
		case 3:		/* set feature */
		case 5:		/* set address */
		case 9:		/* set Configuration */
		case 11:	/* set interface */
		case 0xff:	/* device reset */
			/* <3.2>.In packet */

			/* <A>.Fill qTD */
			/* 0=>qTD */
			spTempqTD = flib_Host20_GetStructure(hcd, Host20_MEM_TYPE_qTD);

			if(spTempqTD == NULL){
				return HOST20_FATAL;
			}
			spTempqTD->bPID = HOST20_qTD_PID_IN;	/* Bit8~9 */
			spTempqTD->bTotalBytes = hwDataSize;	/* Bit16~30 */
			spTempqTD->bDataToggle = 1;		/* Bit31 */
			spTempqTD->ArrayBufferPointer_Word[0] = VA2PA((MS_UINT32)ehci->ep0_buffer);

			/* <B>.Send qTD */
			bReturnValue = flib_Host20_Send_qTD(hcd, spTempqTD, qh_ptr, Host20_Ctrl_Timeout);
			if (bReturnValue > 0)
				goto exit_issue_control;

			/* <C>.Copy Result */
			// memcpy(pbData,pUsbCtrlBuf, hwDataSize);
			break;

		/* <4>.Case-3:'Setup/Out/In' */
		case 7:		/* set descriptor */
			/* <4.2>.Out packet */
			/* <A>.Fill qTD */
		SetupWrite:
			/* 0=>qTD */
			spTempqTD = flib_Host20_GetStructure(hcd, Host20_MEM_TYPE_qTD);

			if(spTempqTD == NULL){
				return HOST20_FATAL;
			}
			spTempqTD->bPID = HOST20_qTD_PID_OUT;		/* Bit8~9 */
			spTempqTD->bTotalBytes = hwDataSize;		/* Bit16~30 */
			spTempqTD->bDataToggle = 1;			/* Bit31 */
			spTempqTD->ArrayBufferPointer_Word[0] = VA2PA((MS_UINT32)ehci->ep0_buffer);

			memcpy(ehci->ep0_buffer, pbData, hwDataSize);

			/* <B>.Send qTD */
			bReturnValue = flib_Host20_Send_qTD(hcd, spTempqTD, qh_ptr, Host20_Ctrl_Timeout);
			if (bReturnValue > 0)
				goto exit_issue_control;

			/* <4.3>.In packet */
			/* <A>.Fill qTD */
			/* 0=>qTD */
			spTempqTD=flib_Host20_GetStructure(hcd, Host20_MEM_TYPE_qTD);
			if(spTempqTD == NULL){
				return HOST20_FATAL;
			}
			spTempqTD->bPID = HOST20_qTD_PID_IN;	/* Bit8~9 */
			spTempqTD->bTotalBytes = 0;		/* Bit16~30 */
			spTempqTD->bDataToggle = 1;		/* Bit31 */

			/* <B>.Send qTD */
			bReturnValue = flib_Host20_Send_qTD(hcd, spTempqTD, qh_ptr, Host20_Ctrl_Timeout);
			if (bReturnValue > 0)
				goto exit_issue_control;
			break;

		default:
			if (*(pbCmd) & 0x80)
				goto SetupRead;
			else if (!(*(pbCmd) & 0x80))
				goto SetupWrite;
			break;
	}
#endif
	return (0);

exit_issue_control:
	return (bReturnValue);
}

void Dump_QTD(MS_UINT32 addr)
{
	MS_UINT8 i;

	USB_LIB_PRINTF("[USB] QH/QTD:%lx -> \n", addr);
	for (i = 0; i < 0x20 ; i = i+4)
	{
		USB_LIB_PRINTF("%x ", *(unsigned char volatile *)(addr + i+ 3));
		USB_LIB_PRINTF("%x ", *(unsigned char volatile *)(addr + i+ 2));
		USB_LIB_PRINTF("%x ", *(unsigned char volatile *)(addr + i+ 1));
		USB_LIB_PRINTF("%x ", *(unsigned char volatile *)(addr + i));
		USB_LIB_PRINTF("\n");
	}
}

void Dump_Data(MS_UINT32 addr, MS_UINT16 sz)
{
	MS_UINT16 i, xxx=0;

	USB_LIB_PRINTF("[USB] addr:%lx -> \n", addr);
	while (sz >= 0x10)
	{
		USB_LIB_PRINTF("\nADDR %x -> ", xxx);
		for (i = 0; i < 0x10; i++)
			USB_LIB_PRINTF("%x ", *(unsigned char volatile *)(addr + i));
		sz -= 0x10;
		addr += 0x10;
		xxx += 0x10;
	}

	USB_LIB_PRINTF("\nADDR %x -> ", xxx);
	for (i = 0; i < sz ; i++)
		USB_LIB_PRINTF("%02x ", *(unsigned char volatile *)(addr + i));
}

MS_UINT8 flib_Host20_Send_Receive_Bulk_Data(struct usb_hcd *hcd, void *buffer, int len, int dir_out)
{
	MS_UINT32 wTotalLengthRemain = 0;
	MS_UINT32 buf = 0;
	MS_UINT8 result = 0;
	MS_UINT32 TransferLen = 0;
	MS_UINT32 TransBuf;

	/* <1>.To fill the data buffer */
	wTotalLengthRemain = len;
	buf = (MS_UINT32)buffer;

	/* <2>.Issue Transfer */
	while (wTotalLengthRemain)
	{
		if(wTotalLengthRemain > Scsi_Max_Transfer_Len)
			TransferLen = Scsi_Max_Transfer_Len;
		else
			TransferLen = wTotalLengthRemain;

		if (dir_out)
		{
		#if defined(CONFIG_USB_BOUNCE_BUF_PATCH)
			TransBuf = (MS_UINT32) usb_bounce_buf;
			memcpy((void*)TransBuf, (void*)buf, TransferLen);
		#else
			TransBuf = buf;
		#endif

			result = flib_Host20_Issue_Bulk (hcd, TransferLen
				    , TransBuf, OTGH_Dir_Out);

			if (result != HOST20_OK)
				return result;
		}
		else
		{
		#if defined(CONFIG_USB_BOUNCE_BUF_PATCH)
			TransBuf = (MS_UINT32) usb_bounce_buf;
		#else
			TransBuf = buf;
		#endif

			result = flib_Host20_Issue_Bulk (hcd, TransferLen
				    , TransBuf, OTGH_Dir_IN);

			if (result != HOST20_OK)
				return result;

		#if defined(CONFIG_USB_BOUNCE_BUF_PATCH)
			memcpy((void*)buf, (void*)TransBuf, TransferLen);
		#endif
		}

		buf += TransferLen;
		wTotalLengthRemain -= TransferLen;
	}
	return(result);
}

/* ====================================================================
 * Function Name: flib_Host20_Issue_Bulk
 * Description: Input data must be 4K-Alignment
 *   <1>.MaxSize=20 K
 *   <2>.Support Only 1-TD
 * Input:
 * OutPut:
 * ==================================================================== */
MS_UINT8  flib_Host20_Issue_Bulk (struct usb_hcd *hcd, MS_UINT32 hwSize, MS_UINT32 pwBuffer, MS_UINT8 bDirection)
{
	MS_UINT8 bTemp = 0, i = 0;
	MS_UINT32 count = 0;
	MS_UINT32 addr = 0, workbuf = 0;
#if defined(USB_BUF_ALIGNED_PATCH)
	MS_UINT32 mybuf = 0;
#endif
#if 0
	int TranSize = 0;
#endif
	qTD_Structure *spTempqTD = NULL;
	qHD_Structure *spTempqH = NULL;
	struct ehci_hcd *ehci = hcd->ehci;

	if (pwBuffer !=(MS_UINT32) KSEG02KSEG1(pwBuffer) )
	{
#if defined(USB_BUF_ALIGNED_PATCH)
		/* ?? any buf issue template */
		/* flush should be 8 bytes aligned */
		if (pwBuffer & 0x7)
		{
			USB_LIB_PRINTF("[USB] Use 8byte aligned buf\n");

			mybuf = (MS_UINT32) KSEG02KSEG1(usb_temp_buf);
			if (mybuf)
			{
				MS_MSG("[USB] usb memory get uncache buf failed!\n");
				return HOST20_FATAL;
			}
		}
		else
#endif
		{
			USB_LIB_BULK_PRINTF("[USB] Flush Data & MIU PIPE\n");

			/* flush/invalidate data to DRAM */
			flush_cache((ulong)pwBuffer,hwSize);
#if (_USB_FLUSH_BUFFER == 1)
			Chip_Flush_Memory();
#endif
			/* uncached buffer */
			pwBuffer=(MS_UINT32)KSEG02KSEG1(pwBuffer);
		}
	}
#if defined(USB_BUF_ALIGNED_PATCH)
	/* ?? any buf issue template */
	else
	{
		/* flush should be 8 bytes aligned */
		if (pwBuffer & 0x7)
		{
			USB_LIB_PRINTF("[USB] Use 8byte aligned buf\n");

			mybuf = (MS_UINT32) KSEG02KSEG1(usb_temp_buf);
			if(mybuf)
			{
				MS_MSG("[USB] usb memory get uncache buf failed!\n");
				return HOST20_FATAL;
			}
		}
	}
#endif
	/* The qTD will be release in the function "Send" */
	spTempqTD = flib_Host20_GetStructure(hcd, Host20_MEM_TYPE_qTD);

	if(spTempqTD == NULL) {
		return HOST20_FATAL;
	}

	spTempqTD->bTotalBytes = hwSize ;

	/* not allow buffer over 16K for my usage */
#if defined(USB_BUF_ALIGNED_PATCH)
	/* ?? any buf issue template */
	/* use original buf */
	if (!mybuf)
		workbuf = mybuf;
	else
#endif
		workbuf = pwBuffer;

	spTempqTD->ArrayBufferPointer_Word[0] = VA2PA(workbuf);

	/* rest of that page */
	count = 0x1000 - (workbuf & 0x0fff);

	/* ... iff needed */
	if ( hwSize < count)
		count = hwSize;
	else
	{
		workbuf +=  0x1000;
		workbuf &= ~0x0fff;

		/* per-qtd limit: from 16K to 20K (best alignment) */
		for (i = 1; (count < hwSize) && (i < 5); i++)
		{
			addr = workbuf;
			spTempqTD->ArrayBufferPointer_Word[i] = VA2PA(addr);
			workbuf += 0x1000;
			if ((count + 0x1000) < hwSize)
				count += 0x1000;
			else
				count = hwSize;
		}
	}

	spTempqH = (bDirection == ehci->ep_bulk0_dir) ? ehci->qh_bulk0 : ehci->qh_bulk1;
	USB_LIB_BULK_PRINTF("[USB] direction: %s, using qh_bulk%d\n",
		bDirection ? "in" : "out",
		(bDirection == ehci->ep_bulk0_dir) ? 0 : 1);

	/* <2>.Analysis the Direction */
	if (bDirection==OTGH_Dir_IN)
	{
		spTempqTD->bPID = HOST20_qTD_PID_IN;
		if (hcd->FirstBulkIn)
		{
			spTempqTD->bDataToggle = 0;
			spTempqH->bDataToggleControl = 1;
		}
	}
	else
	{
		spTempqTD->bPID=HOST20_qTD_PID_OUT;
		if (hcd->FirstBulkOut)
		{
			spTempqTD->bDataToggle = 0;
			spTempqH->bDataToggleControl = 1;
		}
#if defined(USB_BUF_ALIGNED_PATCH)
		/* ?? any buf issue template */
		if (mybuf)
			memcpy((void*)mybuf, (void*)pwBuffer, hwSize);	/* copy to 8byte aligned buffer */
#endif
	}

	/* <3>.Send TD */
	bTemp = flib_Host20_Send_qTD(hcd, spTempqTD ,spTempqH,Host20_Timeout);

	if ((hcd->FirstBulkIn) && (bDirection==OTGH_Dir_IN))
	{
		spTempqH->bDataToggleControl = 0;
		hcd->FirstBulkIn = 0;
	}

	if ((hcd->FirstBulkOut) && (bDirection==OTGH_Dir_Out))
	{
		spTempqH->bDataToggleControl = 0;
		hcd->FirstBulkOut = 0;
	}

	if (bDirection == OTGH_Dir_IN)
	{
#if defined(USB_BUF_ALIGNED_PATCH)
		/* ?? any buf issue template */
		if (mybuf)
			memcpy((void*)pwBuffer, (void*)mybuf, hwSize);	/* copy from 8byte aligned buffer */
#endif
#if 0
		TranSize = hwSize - hcd->total_bytes;
		if ((TranSize % 8) == 7)
		{
			USB_LIB_PRINTF("[USB] patch wallace bug\n");
			*((MS_UINT8*)workbuf+TranSize-1)=*((MS_UINT8*)workbuf+TranSize+3);	//fix hardware bug
		}
		else if ((TranSize % 8)==3)
		{
			USB_LIB_PRINTF("[USB] patch wallace bug\n");
			*((MS_UINT8*)workbuf+TranSize-1)=*((MS_UINT8*)workbuf+TranSize+7);	//fix hardware bug
		}
#endif
	}
	return (bTemp);
}

/* **********************************************************************************
 * **********************************************************************************
                     *** Group-4:Structure Function ***
 * **********************************************************************************
 * ********************************************************************************** */

/* ====================================================================
 * Function Name: flib_Host20_InitStructure
 * Description:
 *    1.Init qHD for Control
 *       qHD_C-->qHD_C-->qHD_C
 *    2.Init qHD for Bulk
 *       |-------------------------|
 *      qHD_C-->qHD_C-->qHD_B-->qHD_B
 *
 *    3.Init qHD for Interrupt
 *    4.Init iTD for ISO (Reserved for feature)
 * Input:Type =0 =>iTD
 *            =1 =>qTD
 *            =2
 * OutPut: 0:Fail
 *         1:ok
 *==================================================================== */
void flib_Host20_InitStructure(struct usb_hcd *hcd)
{
	MS_UINT16   i;
	MS_UINT8 *pData;
	struct ehci_hcd *ehci;

	hcd->ehci = &ms_ehci;
	ehci = hcd->ehci;

	ehci->ep0_buffer = (MS_UINT8 *)KSEG02KSEG1(UsbCtrlBuf);
	ehci->qtd_pool = (MS_UINT8 *)KSEG02KSEG1(QtdBuf);

	flush_cache((ulong)UsbDataBuf, sizeof(UsbDataBuf));
	hcd->ehci->pUsbDataBuf = (MS_UINT8 *)KSEG02KSEG1(UsbDataBuf);

	/* <1>.Clear memory */
	pData = ehci->qtd_pool;
	for (i=0 ; i < (Host20_qTD_SIZE*Host20_qTD_MAX+0x20) ; i++)
		pData[i]=0;

	//if ((MS_UINT32)Host20_STRUCTURE_qTD_BASE_ADDRESS & 0x10)
	//	Host20_STRUCTURE_qTD_BASE_ADDRESS += 0x10;	/* make it aligned with 32 */

	ehci->qh_control0 = (qHD_Structure*)KSEG02KSEG1(&Host20_qHD_List_Control0);
	ehci->qh_control1 = (qHD_Structure*)KSEG02KSEG1(&Host20_qHD_List_Control1);
	ehci->qh_bulk0    = (qHD_Structure*)KSEG02KSEG1(&Host20_qHD_List_Bulk0);
	ehci->qh_bulk1    = (qHD_Structure*)KSEG02KSEG1(&Host20_qHD_List_Bulk1);

	pData = (MS_UINT8*)ehci->qh_control0;
	for (i = 0 ; i < sizeof(qHD_Structure); i++)
		pData[i]=0;

	pData = (MS_UINT8*)ehci->qh_control1;
	for (i = 0 ; i < sizeof(qHD_Structure); i++)
		pData[i]=0;

	pData = (MS_UINT8*)ehci->qh_bulk0;
	for (i = 0 ; i < sizeof(qHD_Structure); i++)
		pData[i]=0;

	pData = (MS_UINT8*)ehci->qh_bulk1;
	for ( i=0 ; i < sizeof(qHD_Structure); i++)
		pData[i]=0;

	ehci->async = VA2PA((MS_UINT32)ehci->qh_control0);

	/* <2>.For qTD & iTD & 4K-Buffer Manage init */
	ehci->qtd_manage = &Host20_qTD_Manage[0];
	for (i = 0;i < Host20_qTD_MAX; i++)
		ehci->qtd_manage[i] = Host20_MEM_FREE;

	// psHost20_qHD_List_Control[0]->bType = HOST20_HD_Type_QH;
	/* Address=0,Head=1,EndPt=0,Size */
	flib_Host20_Allocate_QHD(hcd, ehci->qh_control0, HOST20_HD_Type_QH, 0, 1, 0, 64);
	/* Address=1,Head=0,EndPt=0,Size */
	flib_Host20_Allocate_QHD(hcd, ehci->qh_control1, HOST20_HD_Type_QH, 1, 0, 0, 64);

	/* Address=1,Head=0,EndPt=1,Size */
	flib_Host20_Allocate_QHD(hcd, ehci->qh_bulk0, HOST20_HD_Type_QH, 1, 0, 1, 64);
	/* Address=1,Head=0,EndPt=2,Size */
	flib_Host20_Allocate_QHD(hcd, ehci->qh_bulk1, HOST20_HD_Type_QH, 1, 0, 2, 64);

	/* <3.3>.Link the qHD (contol) */
	ehci->qh_control0->bNextQHDPointer = (VA2PA((MS_UINT32)ehci->qh_control1) >>5);
	ehci->qh_control1->bNextQHDPointer = (VA2PA((MS_UINT32)ehci->qh_control0) >>5);

	/* for QHs address checking*/
#if 0
	MS_MSG("ctrl0 %x size %x\n", &Host20_qHD_List_Control0, sizeof(qHD_Structure));
	MS_MSG("ctrl1 %x size %x\n", &Host20_qHD_List_Control1, sizeof(qHD_Structure));
	MS_MSG("bulk0 %x size %x\n", &Host20_qHD_List_Bulk0, sizeof(qHD_Structure));
	MS_MSG("bulk1 %x size %x\n", &Host20_qHD_List_Bulk1, sizeof(qHD_Structure));
#endif
}

/* ====================================================================
 * Function Name: flib_Host20_GetStructure
 * Description:
 *
 * Input:Type =0 =>qTD
 *            =1 =>iTD
 *            =2 =>4K Buffer
 * OutPut: 0:Fail
 *        ~0:Addrress
 * ==================================================================== */
qTD_Structure *flib_Host20_GetStructure(struct usb_hcd *hcd, MS_UINT8 Type)
{
	MS_UINT32 i;
	MS_UINT8 bFound = 0;
	qTD_Structure *spTempqTD;
	struct ehci_hcd *ehci = hcd->ehci;

	switch(Type)
	{
		// For qTD
		case Host20_MEM_TYPE_qTD:
			for (i = 0;i < Host20_qTD_MAX; i++)
				if (ehci->qtd_manage[i] == Host20_MEM_FREE)
 				{
					bFound = 1;
					ehci->qtd_manage[i] = Host20_MEM_USED;
					break;
				}

			if (bFound == 1)
			{
				spTempqTD = (qTD_Structure *)((MS_UINT32)ehci->qtd_pool + i*Host20_qTD_SIZE);
				memset((unsigned char *)spTempqTD, 0, Host20_qTD_SIZE);
				spTempqTD->bTerminate = 1;		/* Bit0 */
				spTempqTD->bStatus_Active = 0;		/* Bit7 */
				spTempqTD->bInterruptOnComplete = 1;	/* Bit15 */
				spTempqTD->bAlternateTerminate = 1;
				spTempqTD->bErrorCounter = 3;

				// USB_LIB_PRINTF("[USB] get QTD:%x\n", (MS_U32) spTempqTD);
				return (spTempqTD);
			}
			else
				MS_MSG("[USB] QTD underrun!\n");

			break;

		default:
			return 0;
			break;
	}

	/* Not Found... */
	return (0);
}

/* ====================================================================
 * Function Name: flib_Host20_ReleaseStructure
 * Description:
 *
 * Input:Type =0 =>qTD
 *            =1 =>iTD
 *            =2
 * OutPut: 0:Fail
 *        ~0:Addrress
  *==================================================================== */
void flib_Host20_ReleaseStructure(struct usb_hcd *hcd, MS_UINT8 Type, MS_UINT32 pwAddress)
{
	MS_UINT8 i;
	MS_UINT32 wReleaseNum;
	MS_UINT8  *pData;
	struct ehci_hcd *ehci = hcd->ehci;

	// USB_LIB_PRINTF("[USB] release QTD:%x\n",pwAddress);
	pData = (MS_UINT8*)pwAddress;

	switch(Type)
	{
		case Host20_MEM_TYPE_qTD:
			if (pwAddress<(MS_UINT32)ehci->qtd_pool)
			{
				MS_MSG("[USB] ??? Memory release area fail...\n");
				return;
			}

			if ((pwAddress-(MS_UINT32)ehci->qtd_pool) == 0)
				wReleaseNum = 0;
			else
				wReleaseNum=(pwAddress-(MS_UINT32)ehci->qtd_pool)/Host20_qTD_SIZE;

			/* ERROR FIX Prevent Tool 070522 */
			if (wReleaseNum >= Host20_qTD_MAX)
			{
				MS_MSG("[USB] ??? Memory release area fail...\n");
				return;
			}

			ehci->qtd_manage[wReleaseNum]=Host20_MEM_FREE;

			/* qTD size=32 bytes */
			for (i = 0;i < Host20_qTD_SIZE; i++)
				*(pData+i) = 0;
			break;
	}
}

/* ====================================================================
 * Function Name: flib_Host20_QHD_Control_Init
 * Description:
 *
 * Input:Type =0 =>qTD
 *            =1 =>iTD
 *            =2
 * OutPut: 0:Fail
 *         ~0:Addrress
 * ==================================================================== */
void flib_Host20_QHD_Control_Init(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd->ehci;

	/* <1>.Init Control-0/1 */
	ehci->qh_control0->bEdSpeed = hcd->bSpeed;
	ehci->qh_control0->bInactiveOnNextTransaction = 0;
	ehci->qh_control0->bDataToggleControl = 1;

	ehci->qh_control1->bEdSpeed = hcd->bSpeed;
	ehci->qh_control1->bInactiveOnNextTransaction = 0;
	ehci->qh_control1->bDataToggleControl = 1;

	 /* <2>.Init Bulk-0/1 */
	ehci->qh_bulk0->bEdSpeed = hcd->bSpeed;
	ehci->qh_bulk0->bInactiveOnNextTransaction = 0;
	ehci->qh_bulk0->bDataToggleControl = 0;

	ehci->qh_bulk1->bEdSpeed = hcd->bSpeed;
	ehci->qh_bulk1->bInactiveOnNextTransaction = 0;
	ehci->qh_bulk1->bDataToggleControl = 0;

	/* <12>.Enable Asynchronous */
	USB_LIB_PRINTF("[USB] enable aynch \n");
	mbHost20_USBCMD_AsynchronousEnable_Set(hcd->ehc_base);	/* Temp;;Bruce */
}

void flib_Host20_SetControl1MaxPacket(struct usb_hcd *hcd, MS_UINT8 max)
{
	struct ehci_hcd *ehci = hcd->ehci;

	ehci->qh_control1->bMaxPacketSize = max;
	USB_LIB_PRINTF("[USB] control1 max:%x\n", ehci->qh_control1->bMaxPacketSize);
}

/* ====================================================================
 * Function Name: flib_Host20_Allocate_QHD
 * Description:
 *
 * Input:Type =0 =>qTD
 *            =1 =>iTD
 *            =2
 * OutPut: 0:Fail
 *        ~0:Addrress
 * ==================================================================== */
void flib_Host20_Allocate_QHD(struct usb_hcd *hcd, qHD_Structure *psQHTemp, MS_UINT8 bNextType, MS_UINT8 bAddress, MS_UINT8 bHead, MS_UINT8 bEndPt, MS_UINT32 wMaxPacketSize)
{
	qTD_Structure *spTempqTD;

	psQHTemp->bTerminate = 0;		/* Bit0 */
	psQHTemp->bType = bNextType;		/* Bit2~1 */

	psQHTemp->bDeviceAddress = bAddress;		/* Bit0~6 */
	psQHTemp->bEdNumber = bEndPt;			/* Bit11~8 */
	psQHTemp->bHeadOfReclamationListFlag = bHead;	/* Bit15 */
	psQHTemp->bMaxPacketSize = wMaxPacketSize;	/* Bit16~26 */
	psQHTemp->bNakCounter = Host20_QHD_Nat_Counter;

	psQHTemp->bOverlay_NextTerminate = 1;
	psQHTemp->bOverlay_AlternateNextTerminate = 1;

	/* <2>.allocate dumy qTD */

	/* <2.1>.Allocate qTD */
	spTempqTD = flib_Host20_GetStructure(hcd, Host20_MEM_TYPE_qTD);	/* 0=>qTD */
	psQHTemp->bOverlay_NextqTD = (VA2PA((MS_UINT32)spTempqTD) >> 5);

	/* <2.3>.Active the qTD */
	psQHTemp->bOverlay_NextTerminate = 0;
}


/* ====================================================================
 * Function Name: flib_Host20_CheckingForResult_QHD
 * Description:
 * Input:Type
 * OutPut:
 * ==================================================================== */
MS_UINT8 flib_Host20_CheckingForResult_QHD(struct usb_hcd *hcd, qHD_Structure *spTempqHD)
{
	MS_UINT32 bQHStatus;
	MS_UINT16 wIntStatus;
	struct ehci_hcd *ehci = hcd->ehci;

	wIntStatus = mwHost20_USBSTS_Rd(hcd->ehc_base);

	hcd->urb_status = 0;

	/* <2>.Checking for the Error type interrupt => Halt the system */
	if (wIntStatus & HOST20_USBINTR_SystemError)
	{
		MS_MSG("[USB] USBSTS System Error... Halt the system...\n");
		ehci->bSendStatusError = 1;
	}

	if (wIntStatus & HOST20_USBINTR_USBError) {
		MS_MSG("[USB] USBSTS USB Error...\n");
		mwHost20_USBSTS_USBError_Set(hcd->ehc_base);
		ehci->bSendStatusError = 1;
	}

	if (ehci->bSendStatusError == 0)
		return HOST20_OK;

	USB_LIB_PRINTF("[USB] Send Error USBSTS Status:%x\n",wIntStatus);

	/* <1>.Analysis the qHD Status */
	ehci->bSendStatusError = 0;
	MS_MSG("[USB] USB Error Interrupt Event...\n");

	bQHStatus = spTempqHD->bOverlay_Status;
	USB_LIB_PRINTF("[USB] bQHStatus:%lx\n",bQHStatus);

	/* stop to check ehci status */
	// while (1) {};

	if (bQHStatus & HOST20_qTD_STATUS_Halted)
	{
		/* clear halt status */
		spTempqHD->bOverlay_Status = 0;
		spTempqHD->bOverlay_TotalBytes = 0;
		spTempqHD->bOverlay_Direction = 0;

		/* no other error status */
		if (bQHStatus == HOST20_qTD_STATUS_Halted)
		{
			MS_MSG("[USB] qHD Status STALL\n");
			hcd->urb_status |= USB_ST_STALLED;
			return HOST20_DEVICE_STALL;
		}
		else
			MS_MSG("[USB] qHD Status => Halted (Stall/Babble/Error Counter=0)...0x%lx\n", bQHStatus);
	}

	if (bQHStatus & HOST20_qTD_STATUS_BufferError)
	{
		hcd->urb_status |= USB_ST_BUF_ERR;
		MS_MSG("[USB] qHD Status => HOST20_qTD_STATUS_BufferError...\n");
	}

	if (bQHStatus & HOST20_qTD_STATUS_Babble)
	{
		MS_MSG("[USB] qHD Status => HOST20_qTD_STATUS_Babble !!! \n");
		/* don't fix babble error for Bert */
		spTempqHD->bOverlay_Status = 0;
		hcd->urb_status |= USB_ST_BABBLE_DET;

		return HOST20_OK;
	}

	if (bQHStatus & HOST20_qTD_STATUS_TransactionError)
	{
		MS_MSG("[USB] qHD Status => HOST20_qTD_STATUS_TransactionError...\n");
		hcd->urb_status |= USB_ST_CRC_ERR;
	}

	if (bQHStatus & HOST20_qTD_STATUS_MissMicroFrame)
	{
		MS_MSG("[USB] qHD Status => HOST20_qTD_STATUS_MissMicroFrame...\n");
		hcd->urb_status |= USB_ST_BIT_ERR;
	}

	/* <2>.Clear the status */
	spTempqHD->bOverlay_Status = 0;

	return HOST20_FAIL;
}

MS_UINT8 flib_Host20_Send_qTD_Intr(struct usb_hcd *hcd, qTD_Structure  *spHeadqTD ,qHD_Structure  *spTempqHD)
{
  MS_UINT8 bReturnValue;
  qTD_Structure   *spNewDumyqTD;
  qTD_Structure   *spOldDumyqTD;
  qTD_Structure   *spLastqTD;
  MS_UINT32 ehci_base = hcd->ehc_base;

  spOldDumyqTD=(qTD_Structure*)PA2VA((( MS_UINT32)(spTempqHD->bOverlay_NextqTD))<<5);

  memcpy(spOldDumyqTD,spHeadqTD,Host20_qTD_SIZE);

  //<2>.Prepare new dumy qTD
  spNewDumyqTD=spHeadqTD;
  memset((void *)spNewDumyqTD ,0, Host20_qTD_SIZE);
  spNewDumyqTD->bTerminate=1;

  //<3>.Find spLastqTD & link spLastqTD to NewDumyqTD & Set NewDumyqTD->T=1
  spLastqTD=spOldDumyqTD;
  while(spLastqTD->bTerminate==0) {
         spLastqTD=(qTD_Structure*)PA2VA((( MS_UINT32)(spLastqTD->bNextQTDPointer))<<5);

  };

  spLastqTD->bNextQTDPointer=VA2PA((MS_UINT32)spNewDumyqTD)>>5;

  spLastqTD->bTerminate=0;

  //Link Alternate qTD pointer
  spLastqTD->bAlternateQTDPointer=(VA2PA(( MS_UINT32)spNewDumyqTD)>>5);

  spLastqTD->bAlternateTerminate=0;

  //<4>.Set OldDumyqTD->Active=1
  //Dump_QTD(spOldDumyqTD);
  //Dump_QTD(spNewDumyqTD);

  while (mwHost20_USBSTS_Rd(ehci_base) & 0x3b)
  {
    mwHost20_USBSTS_Set(ehci_base,0x3b);      //clear interrupt, don't clear port change int
  }

  spOldDumyqTD->bStatus_Active=1;

  flush_cache((ulong)spOldDumyqTD,0x10);       //wried, seems like cache has bug,
                                                                    //CPU always keep its own copy,
                                                                    //even we use non-cache memory
  bReturnValue=HOST20_OK;

  return (bReturnValue);
}

MS_UINT8 flib_Host20_Send_qTD2(struct usb_hcd *hcd, qTD_Structure  *spHeadqTD ,qHD_Structure  *spTempqHD, MS_UINT32 wTimeOutSec)
{
  MS_UINT8 bReturnValue;
  qTD_Structure   *spNewDumyqTD;
  qTD_Structure   *spOldDumyqTD;
  qTD_Structure   *spReleaseqTD;
  qTD_Structure   *spReleaseqTDNext;
  qTD_Structure   *spLastqTD;
  MS_UINT32 ehci_base = hcd->ehc_base;
  unsigned long wTimes;

  //if (wTimeOutSec==0)
  //{
  //  DEBUG_USB(printf("error, timeout sec is zero\n"));
  //}
    spOldDumyqTD=(qTD_Structure*)PA2VA((( MS_UINT32)(spTempqHD->bOverlay_NextqTD))<<5);

  //spTempqHD->bOverlay_Status|=HOST20_qTD_STATUS_Halted;
  while (mbHost20_USBCMD_PeriodicEnable_Rd(ehci_base))
  {
    mbHost20_USBCMD_PeriodicEnable_Clr(ehci_base);
  }

  //spHeadqTD->bTerminate=1;			//set to terminate
  memcpy(spOldDumyqTD,spHeadqTD,Host20_qTD_SIZE);
  //spOldDumyqTD->bStatus_Halted=1;

  //spOldDumyqTD->bStatus_Active=0;

  //<2>.Prepare new dumy qTD
  spNewDumyqTD=spHeadqTD;
  memset((void *)spNewDumyqTD ,0, Host20_qTD_SIZE);
  spNewDumyqTD->bTerminate=1;
  //spNewDumyqTD->bAlternateTerminate=1;
  //spNewDumyqTD->bStatus_Halted=1;
  //<3>.Find spLastqTD & link spLastqTD to NewDumyqTD & Set NewDumyqTD->T=1
  spLastqTD=spOldDumyqTD;
  while(spLastqTD->bTerminate==0) {
         spLastqTD=(qTD_Structure*)PA2VA((( MS_UINT32)(spLastqTD->bNextQTDPointer))<<5);

  };

  spLastqTD->bNextQTDPointer=VA2PA((MS_UINT32)spNewDumyqTD)>>5;

  spLastqTD->bTerminate=0;

  //Link Alternate qTD pointer
    spLastqTD->bAlternateQTDPointer=(VA2PA(( MS_UINT32)spNewDumyqTD)>>5);

  spLastqTD->bAlternateTerminate=0;



  //<4>.Set OldDumyqTD->Active=1
  //gwLastqTDSendOK=0;
  //sAttachDevice.psSendLastqTD=spLastqTD;
  //sAttachDevice.bSendStatusError=0;

  //Dump_QTD(spOldDumyqTD);
  //Dump_QTD(spNewDumyqTD);

  while (mwHost20_USBSTS_Rd(ehci_base) & 0x3b)
  {
    mwHost20_USBSTS_Set(ehci_base,0x3b);      //clear interrupt, don't clear port change int
  }
  //bExitLoop=0;
  //spOldDumyqTD->bStatus_Halted=0;
  //spOldDumyqTD->bStatus_Halted=0;

#if 0
    printf("spOldDumyqTD: %X\n", spOldDumyqTD);
    printf("bTerminate: %X\n", spOldDumyqTD->bTerminate);
    printf("bNextQTDPointer: %X\n", spOldDumyqTD->bNextQTDPointer);
    printf("bAlternateTerminate: %X\n", spOldDumyqTD->bAlternateTerminate);
    printf("bAlternateQTDPointer: %X\n", spOldDumyqTD->bAlternateQTDPointer);
    printf("bStatus_PingState: %X\n", spOldDumyqTD->bStatus_PingState);
    printf("bStatus_SplitState: %X\n", spOldDumyqTD->bStatus_SplitState);
    printf("bStatus_MissMicroFrame: %X\n", spOldDumyqTD->bStatus_MissMicroFrame);
    printf("bStatus_Transaction_Err: %X\n", spOldDumyqTD->bStatus_Transaction_Err);
    printf("bStatus_Babble: %X\n", spOldDumyqTD->bStatus_Babble);
    printf("bStatus_Buffer_Err: %X\n", spOldDumyqTD->bStatus_Buffer_Err);
    printf("bStatus_Halted: %X\n", spOldDumyqTD->bStatus_Halted);
    printf("bStatus_Active: %X\n", spOldDumyqTD->bStatus_Active);
    printf("bPID: %X\n", spOldDumyqTD->bPID);
    printf("bErrorCounter: %X\n", spOldDumyqTD->bErrorCounter);
    printf("CurrentPage: %X\n", spOldDumyqTD->CurrentPage);
    printf("bInterruptOnComplete: %X\n", spOldDumyqTD->bInterruptOnComplete);
    printf("bTotalBytes: %X\n", spOldDumyqTD->bTotalBytes);
    printf("bDataToggle: %X\n", spOldDumyqTD->bDataToggle);
    printf("ArrayBufferPointer_Word: %X\n", spOldDumyqTD->ArrayBufferPointer_Word[0]);
#endif
    //flib_Host20_StopRun_Setting(HOST20_Enable);
    //printf("RunStop: %X\n", mbHost20_USBCMD_RunStop_Rd());
  spOldDumyqTD->bStatus_Active=1;

  while (mbHost20_USBCMD_PeriodicEnable_Rd(ehci_base)==0)
  {
	mbHost20_USBCMD_PeriodicEnable_Set(ehci_base);
  }
  //spTempqHD->bOverlay_Status&=~HOST20_qTD_STATUS_Halted;

  //wait until asynchronous scheduler is idle

  //mbHost20_USBCMD_RunStop_Set();

  //spTempqHD->bOverlay_Status&=~0x40;			//clr HALT bit, start this queue head
  wTimes=0;
  //<5>.Waiting for result
  //EAL=0;
  while (1)
  {
    flush_cache((ulong)spOldDumyqTD,0x10);       //wried, seems like cache has bug,
                                                                                 //CPU always keep its own copy,
                                                                                 //even we use non-cache memory
    wTimes++;

    if ((spOldDumyqTD->bStatus_Active==0) &&
	    ( mwHost20_USBSTS_Rd(ehci_base)& HOST20_USBINTR_CompletionOfTransaction))              //wait until Status_Active become 0
    {
      bReturnValue=HOST20_OK;

      break;
      //bExitLoop=1;
      //printf("bReturn:%02bx\n",    bReturnValue);
    }
    //if (bExitLoop) break;
    #if 1
    if (mwHost20_USBSTS_Rd(ehci_base)&(HOST20_USBINTR_SystemError+HOST20_USBINTR_USBError))
    {
      bReturnValue=HOST20_OK;
      //if (spOldDumyqTD->bStatus_Active==1)
	  //{
	  //  printf("something wrong..USBINTR:%02bx\n",mwHost20_USBSTS_Rd());
	  //  printf("QH status:%02bx\n",spTempqHD->bOverlay_Status);
	  //}
      break;            //USB interrupt happened
    }
    #endif

	wTimes++;

	if (mwHost20_PORTSC_ConnectStatus_Rd(ehci_base) == 0)
	{
		MS_MSG("[USB] Port%d device disconn\n", hcd->port_index);
		bReturnValue = HOST20_FATAL;
		hcd->urb_status |= USB_ST_DISCONNECT;
		break;
	}

	udelay(1);

	/* Timeout: Xs*/
	if (wTimes > wTimeOutSec * 1000 * 1000)
	{
		bReturnValue = HOST20_FATAL;
		hcd->urb_status |= USB_ST_TIMEOUT;

		/* host controller reset */
		// mbHost20_USBCMD_HCReset_Set();
		// while(mbHost20_USBCMD_HCReset_Rd() > 0);

		MS_MSG("[USB] >>> Fail => Time Out for Send qTD...\n");

		USB_LIB_PRINTF("[USB] Reg 0x28: 0x%lx 0x%lx\n", *((MS_UINT32 volatile *)(ehci_base+0x28*2)),
			*((MS_UINT32 volatile *)(ehci_base+0x28*2+4)));

		USB_LIB_PRINTF("[USB] QH: \n");
		Dump_QTD((MS_UINT32)spTempqHD);

		USB_LIB_PRINTF("[USB] TD: \n");
		Dump_QTD((MS_UINT32)spOldDumyqTD);

		// while(1);
		break;
	}

  }

  while (mbHost20_USBCMD_PeriodicEnable_Rd(ehci_base))
  {
	mbHost20_USBCMD_PeriodicEnable_Clr(ehci_base);
  }

  //<6>.Checking the Result
  if (bReturnValue!=HOST20_FATAL)
    bReturnValue=flib_Host20_CheckingForResult_QHD(hcd, spTempqHD);
  if (bReturnValue==HOST20_TRANSACTION_ERROR)
  {
    //printf("Transaction Error\n");

    spTempqHD->bOverlay_Status=0;      //clear halt status
    spTempqHD->bOverlay_CurrentqTD=VA2PA((MS_UINT32)spNewDumyqTD)>>5;
  }

  //<5>.Release the all the qTD (Not include spNewDumyqTD)
  #if 1
  spReleaseqTD=spOldDumyqTD;
  do {
    //spReleaseqTDNext=((UINT32)(spReleaseqTD->bNextQTDPointerL))<<5 + ((UINT32)(spReleaseqTD->bNextQTDPointerH))<<16  ;
	spReleaseqTDNext=(qTD_Structure*)PA2VA(((MS_UINT32)(spReleaseqTD->bNextQTDPointer))<<5);

	flib_Host20_ReleaseStructure(hcd, Host20_MEM_TYPE_qTD,(MS_UINT32)spReleaseqTD);

    spReleaseqTD=spReleaseqTDNext;
  } while(((MS_UINT32)spReleaseqTD)!=((MS_UINT32)spNewDumyqTD));
  #endif

 return (bReturnValue);
}

/* ====================================================================
 * Function Name: flib_Host20_Send_qTD
 * Description:
 *    Case-1:1qTD
 *    Case-2:2qTD
 *    Case-3:3qTD above
 * Input:Type
 * OutPut: 0 => OK
 *         1 => TimeOut
 * ==================================================================== */
MS_UINT8 flib_Host20_Send_qTD(struct usb_hcd *hcd, qTD_Structure *spHeadqTD, qHD_Structure *spTempqHD, MS_UINT32 wTimeOutSec)
{
	MS_UINT8 bReturnValue;
	MS_UINT32 wTimes;
	MS_UINT32 ehci_base = hcd->ehc_base;
	struct ehci_hcd *ehci = hcd->ehci;
	qTD_Structure *spNewDumyqTD;
	qTD_Structure *spOldDumyqTD;
	qTD_Structure *spReleaseqTD;
	qTD_Structure *spReleaseqTDNext;
	qTD_Structure *spLastqTD;

	if (wTimeOutSec==0)
		MS_MSG("[USB] error, timeout sec is zero\n");

	spOldDumyqTD=(qTD_Structure*)PA2VA(((MS_UINT32)(spTempqHD->bOverlay_NextqTD)) << 5);

	// spTempqHD->bOverlay_Status |= HOST20_qTD_STATUS_Halted;
#if 0
	while (mbHost20_USBCMD_AsynchronousEnable_Rd())
	{
		/* pause asynchronous scheduler */
		mbHost20_USBCMD_AsynchronousEnable_Clr();
	}
#endif
	/* set to terminate */
	memcpy(spOldDumyqTD, spHeadqTD, Host20_qTD_SIZE);

	/* <2>.Prepare new dumy qTD */
	spNewDumyqTD = spHeadqTD;
	memset((unsigned char  *)spNewDumyqTD, 0, Host20_qTD_SIZE);
	spNewDumyqTD->bTerminate = 1;
	// spNewDumyqTD->bAlternateTerminate = 1;
	// spNewDumyqTD->bStatus_Halted = 1;

	/* <3>.Find spLastqTD & link spLastqTD to NewDumyqTD & Set NewDumyqTD->T=1 */
	spLastqTD = spOldDumyqTD;

	while(spLastqTD->bTerminate == 0) {
		spLastqTD = (qTD_Structure*)PA2VA((((MS_UINT32)(spLastqTD->bNextQTDPointer)) << 5));
	};

	spLastqTD->bNextQTDPointer = VA2PA((MS_UINT32)spNewDumyqTD) >> 5;
	spLastqTD->bTerminate = 0;

	/* Link Alternate qTD pointer */
	spLastqTD->bAlternateQTDPointer = VA2PA((MS_UINT32)spNewDumyqTD) >> 5;
	spLastqTD->bAlternateTerminate = 0;

	ehci->bSendStatusError = 0;

#if 0
	USB_LIB_PRINTF("[USB] Before ... \n");
	USB_LIB_PRINTF("[USB] QH: \n");
	Dump_QTD((MS_UINT32)spTempqHD);
	USB_LIB_PRINTF("[USB] TD: \n");
	Dump_QTD((MS_UINT32)spOldDumyqTD);
	USB_LIB_PRINTF("[USB] After ... \n");
#endif

	// while (mwHost20_USBSTS_Rd() & 0x3f)
	// {
		mwHost20_USBSTS_Set(ehci_base, 0x3f);	/* clear interrupt */
	// }

	// spOldDumyqTD->bStatus_Halted = 0;
	// spOldDumyqTD->bStatus_Halted = 0;
	spOldDumyqTD->bStatus_Active = 1;

#if (_USB_FLUSH_BUFFER == 1)
	Chip_Flush_Memory();
#endif

	// while (1){};
#if 0
	while (mbHost20_USBCMD_AsynchronousEnable_Rd() == 0)
	{
		/* re start asynchronous scheduler */
		mbHost20_USBCMD_AsynchronousEnable_Set();
	}
#endif
	// spTempqHD->bOverlay_Status &= ~HOST20_qTD_STATUS_Halted;

	/* wait until asynchronous scheduler is idle */
	// mbHost20_USBCMD_RunStop_Set();

	/* clr HALT bit, start this queue head */
	// spTempqHD->bOverlay_Status &= ~0x40;

	wTimes=0;

	while (1)
	{
		/* ******************************** *
		 * wried, seems like cache has bug, *
		 * CPU always keep its own copy,    *
	 	 * even we use non-cache memory     *
		 *********************************** */
		flush_cache((ulong)spOldDumyqTD,0x10);

		// mb();
		if ( mwHost20_USBSTS_Rd(ehci_base) & HOST20_USBINTR_CompletionOfTransaction )
		{
#if (_USB_FLUSH_BUFFER == 1)
			Chip_Read_Memory();
#endif
			if (spOldDumyqTD->bStatus_Active == 0)
			{
				bReturnValue = HOST20_OK;
				break;
			}
		}

		if (mwHost20_USBSTS_Rd(ehci_base) & (HOST20_USBINTR_SystemError+HOST20_USBINTR_USBError))
		{
			bReturnValue = HOST20_OK;
			break;	/* USB interrupt happened */
		}

		// if (gwOTG_Timer_Counter>wTimeOutSec)
			wTimes++;

		if (mwHost20_PORTSC_ConnectStatus_Rd(ehci_base) == 0)
		{
			MS_MSG("[USB] Port%d device disconn\n", hcd->port_index);
			bReturnValue = HOST20_FATAL;
			hcd->urb_status |= USB_ST_DISCONNECT;
			break;
		}

		udelay(1);

		/* Timeout: Xs*/
		if (wTimes > wTimeOutSec * 1000 * 1000)
		{
			bReturnValue = HOST20_FATAL;
			hcd->urb_status |= USB_ST_TIMEOUT;

			/* host controller reset */
			// mbHost20_USBCMD_HCReset_Set();
			// while(mbHost20_USBCMD_HCReset_Rd() > 0);

			MS_MSG("[USB] >>> Fail => Time Out for Send qTD...\n");

			USB_LIB_PRINTF("[USB] Reg 0x28: 0x%lx 0x%lx\n", *((MS_UINT32 volatile *)(ehci_base+0x28*2)),
				*((MS_UINT32 volatile *)(ehci_base+0x28*2+4)));

			USB_LIB_PRINTF("[USB] QH: \n");
			Dump_QTD((MS_UINT32)spTempqHD);

			USB_LIB_PRINTF("[USB] TD: \n");
			Dump_QTD((MS_UINT32)spOldDumyqTD);

			// while(1);
			break;
		}
	}

	hcd->total_bytes = spOldDumyqTD->bTotalBytes;

	/* <6>.Checking the Result */
	if (bReturnValue != HOST20_FATAL)
		bReturnValue = flib_Host20_CheckingForResult_QHD(hcd, spTempqHD);

	/* <5>.Release the all the qTD (Not include spNewDumyqTD) */
	spReleaseqTD=spOldDumyqTD;

	do {
		spReleaseqTDNext=(qTD_Structure*)PA2VA(((MS_UINT32)(spReleaseqTD->bNextQTDPointer)) << 5);

		flib_Host20_ReleaseStructure(hcd, Host20_MEM_TYPE_qTD,(MS_UINT32)spReleaseqTD);
		spReleaseqTD = spReleaseqTDNext;
	} while(((MS_UINT32)spReleaseqTD) != ((MS_UINT32)spNewDumyqTD));

	//<6>.Double Check the QH overlay status. Adjust it if need.
	if( spTempqHD->bOverlay_NextqTD != (VA2PA(( MS_UINT32)spNewDumyqTD)>>5)
		|| spTempqHD->bOverlay_Status & MS_BIT7 )
	{
		// Bug of WD 3.0 disk. Disconnection happens while data face of SETUP transaction.
		// If chain qTDs after disconnection/timeout, QH overlay will not be advanced by HW.
		// It muss up qTD chain layout. QH doesn't stop at old dumy and stare at new dumy.
		// SW advance QH overlay manually no matter HW advancing or not.
		// Run bit is cleared by HQ when disconnection, so it is safe to modify the QH.

		MS_MSG("[Warning] Adjust bad qTD chain..\r\n");

		spTempqHD->bOverlay_Status = spTempqHD->bOverlay_Status & ~MS_BIT7;
		spTempqHD->bOverlay_CurrentqTD = VA2PA(( MS_UINT32)spOldDumyqTD) >> 5;
		spTempqHD->bOverlay_NextqTD = VA2PA(( MS_UINT32)spNewDumyqTD) >> 5;
		spTempqHD->bOverlay_AlternateqTD = VA2PA(( MS_UINT32)spNewDumyqTD) >> 5;
	}
	return (bReturnValue);
}

MS_UINT8 flib_Host20_Start_BulkIn(struct usb_hcd *hcd, MS_UINT16 hwSize, MS_UINT32 pwBuffer)
{
	qTD_Structure  *spTempqTD = NULL;
	MS_UINT32 workbuf=0;
	qHD_Structure   *spTempqH = NULL;
	qTD_Structure  *spNewDumyqTD;
	qTD_Structure  *spOldDumyqTD;
	qTD_Structure  *spLastqTD;

	if ( pwBuffer != (MS_UINT32) KSEG02KSEG1(pwBuffer) )
	{
		flush_cache((ulong)pwBuffer,hwSize);     //flush buffer to uncached buffer
#if (_USB_FLUSH_BUFFER == 1)
		Chip_Flush_Memory();
#endif
		pwBuffer=(MS_UINT32)KSEG02KSEG1(pwBuffer);
	}

	spTempqTD = flib_Host20_GetStructure(hcd, Host20_MEM_TYPE_qTD); //The qTD will be release in the function "Send"

	if(spTempqTD == NULL)
		return HOST20_FATAL;

	spTempqTD->bTotalBytes = hwSize ;

	workbuf = pwBuffer;

	spTempqTD->ArrayBufferPointer_Word[0] = VA2PA(workbuf);

	spTempqH = (hcd->ehci->ep_bulk0_dir == OTGH_Dir_IN) ? hcd->ehci->qh_bulk0 : hcd->ehci->qh_bulk1;

	spTempqTD->bPID=HOST20_qTD_PID_IN;

	if (hcd->FirstBulkIn)
	{
		spTempqTD->bDataToggle=0;
		spTempqH->bDataToggleControl=1;
	}

	//<3>.Send TD
	spOldDumyqTD=(qTD_Structure*)PA2VA(((MS_UINT32)(spTempqH->bOverlay_NextqTD))<<5);

	hcd->ehci->spBulkInqTD = spOldDumyqTD;
	USB_LIB_BULK_PRINTF("[USB] spBulkInqTD: %x\n", (MS_U32)hcd->ehci->spBulkInqTD);

	memcpy(spOldDumyqTD, spTempqTD, Host20_qTD_SIZE);

	//<2>.Prepare new dumy qTD
	spNewDumyqTD = spTempqTD;
	memset((unsigned char *)spNewDumyqTD , 0, Host20_qTD_SIZE);
	spNewDumyqTD->bTerminate = 1;

	spLastqTD = spOldDumyqTD;
	while(spLastqTD->bTerminate==0) {
		spLastqTD=(qTD_Structure*)PA2VA((((MS_UINT32)(spLastqTD->bNextQTDPointer))<<5));
	};

	spLastqTD->bNextQTDPointer = VA2PA((MS_UINT32)spNewDumyqTD)>>5;
	spLastqTD->bTerminate = 0;
	spLastqTD->bAlternateQTDPointer = VA2PA((MS_UINT32)spNewDumyqTD)>>5;
	spLastqTD->bAlternateTerminate = 0;

	hcd->ehci->bSendStatusError = 0;

	mwHost20_USBSTS_Set(hcd->ehc_base, 0x3f);	/* clear interrupt */

	spOldDumyqTD->bStatus_Active = 1;

#if (_USB_FLUSH_BUFFER == 1)
	Chip_Flush_Memory();
#endif

	if (hcd->FirstBulkIn)
	{
		spTempqH->bDataToggleControl=0;
		hcd->FirstBulkIn=0;
	}

	return HOST20_OK;
}

MS_UINT8 flib_Host20_Get_BulkIn_Data(struct usb_hcd *hcd, struct usb_device *dev, unsigned int pipe,
	MS_U32 buf, MS_U32 length, MS_U32 *actual_len, int timeout)
{
	MS_U32 uTotalSize = 0, uRemainSize;
	MS_U16 wMaxPacketSize;
	MS_UINT8  bReturnValue = HOST20_OK;
	qHD_Structure  *spTempqHD;

	USB_LIB_BULK_PRINTF("[USB] flib_Host20_Get_BulkIn_Data enter...\n");

	timeout *= 1000; /* convert from ms to us */
	dev->status = hcd->urb_status = 0;
	wMaxPacketSize = dev->epmaxpacketin[((pipe>>15) & 0xf)];

	spTempqHD = (hcd->ehci->ep_bulk0_dir == OTGH_Dir_IN) ? hcd->ehci->qh_bulk0 : hcd->ehci->qh_bulk1;

	if (hcd->ehci->spBulkInqTD== NULL)
	{
		//Start a BulkIn with length MaxPktSize
		flib_Host20_Start_BulkIn(hcd, wMaxPacketSize, (MS_U32)hcd->ehci->pUsbDataBuf);
	}

	do
	{
		if ( mwHost20_USBSTS_Rd(hcd->ehc_base) & HOST20_USBINTR_CompletionOfTransaction )
		{
#if (_USB_FLUSH_BUFFER == 1)
			Chip_Read_Memory();
#endif
			if (hcd->ehci->spBulkInqTD->bStatus_Active == 0)
			{
				bReturnValue = HOST20_OK;
				// Add total return size
				uRemainSize = (MS_U16)hcd->ehci->spBulkInqTD->bTotalBytes;
				uTotalSize += wMaxPacketSize - uRemainSize;

				if (uRemainSize > 0)
				{
					//printf("short data\n");
					goto Trans_Done;
				}

				if (uTotalSize >= length)
					goto Trans_Done;;

				//Start a BulkIn with length MaxPktSize
				flib_Host20_Start_BulkIn(hcd, wMaxPacketSize, (MS_U32)(hcd->ehci->pUsbDataBuf+uTotalSize));
			}
		}

		if (mwHost20_USBSTS_Rd(hcd->ehc_base) & (HOST20_USBINTR_SystemError+HOST20_USBINTR_USBError))
		{
			bReturnValue=HOST20_OK;
			goto Trans_Done;	//USB interrupt happened
		}

		if (mwHost20_PORTSC_ConnectStatus_Rd(hcd->ehc_base) == 0)
		{
			MS_MSG("[USB] Port%d device disconn\n", hcd->port_index);
			bReturnValue = HOST20_FATAL;
			hcd->urb_status |= USB_ST_DISCONNECT;
			goto Trans_Done;
		}

		udelay(1);
	} while(timeout-->0);

	*actual_len = 0;

	return (bReturnValue); //Timeout return

Trans_Done:
	USB_LIB_BULK_PRINTF("[USB] Trans %d bytes\n", uTotalSize);

	memcpy((void*)buf, hcd->ehci->pUsbDataBuf, uTotalSize);
	*actual_len = uTotalSize;
	//<6>.Checking the Result
	if (bReturnValue!=HOST20_FATAL)
		bReturnValue=flib_Host20_CheckingForResult_QHD(hcd, spTempqHD);

	//<5>.Release the all the qTD (Not include spNewDumyqTD)
	flib_Host20_ReleaseStructure(hcd, Host20_MEM_TYPE_qTD, (MS_UINT32)(hcd->ehci->spBulkInqTD));

	hcd->ehci->spBulkInqTD = NULL;

	dev->status = hcd->urb_status;
	return (bReturnValue);
}

//------- xHCI --------
void U3phy_MS28_init(struct xhc_comp *xhci)
{
#ifdef XHCI_SINGLE_PORT_ENABLE_MAC
        writeb(readb((void*)(xhci->u3phy_D_base+0x84*2))|0x40, (void*)(xhci->u3phy_D_base+0x84*2)); // open XHCI MAC clock
#endif

	//-- 28 hpm mstar only---
	writew(0x0104, (void*) (xhci->u3phy_A_base+0x6*2));  // for Enable 1G clock pass to UTMI //[2] reg_pd_usb3_purt [7:6] reg_gcr_hpd_vsel

	//U3phy initial sequence
	writew(0x0,    (void*) (xhci->u3phy_A_base)); 		 // power on rx atop
	writew(0x0,    (void*) (xhci->u3phy_A_base+0x2*2));	 // power on tx atop
	//writew(0x0910, (void*) (U3PHY_D_base+0x4*2));        // the same as default
	writew(0x0,    (void*) (xhci->u3phy_A_base+0x3A*2));  // overwrite power on rx/tx atop
	writew(0x0160, (void*) (xhci->u3phy_D_base+0x18*2));
	writew(0x0,    (void*) (xhci->u3phy_D_base+0x20*2));	 // power on u3_phy clockgen
	writew(0x0,    (void*) (xhci->u3phy_D_base+0x22*2));	 // power on u3_phy clockgen

#ifdef XHCI_ENABLE_PD_OVERRIDE
        writew(0x308,    (void*) (xhci->u3phy_A_base+0x3A*2)); // [9,8,3] PD_TXCLK_USB3TXPLL, PD_USB3_IBIAS, PD_USB3TXPLL override enable
        writeb(readb((void*)(xhci->u3phy_A_base+0x3*2-1)) & 0xbb,     (void*)(xhci->u3phy_A_base+0x3*2-1)); // override value 0
#endif

	writeb(0xF4,   (void*) (xhci->u3phy_D_base+0x12*2));	  //TX lock threshold

	if(xhci->xhci_base != 0) {
		//disable compliance mode
		writeb(readb((void*)(xhci->xhci_base+0x6817)) | MS_BIT7, (void*)(xhci->xhci_base+0x6817));
	}
}

void xhci_enable_clock(void)
{
	static int clock_enable = 0;
#ifdef XHCI_PORT0_ADDR
	struct xhc_comp xc = XHC_COMP_PORT0;
#endif
#ifdef XHCI_PORT1_ADDR
	struct xhc_comp xc1 = XHC_COMP_PORT1;
#endif

	if (clock_enable)
		return;

#ifdef XHCI_PORT0_ADDR
	printf("xhci_enable_clock\n");
	U3phy_MS28_init(&xc);
	clock_enable = 1;
#endif
#ifdef XHCI_PORT1_ADDR
	U3phy_MS28_init(&xc1);
#endif
}

MS_UINT32 xhci_port_state_to_neutral(MS_UINT32 state)
{
	/* Save read-only status and port state */
	return (state & XHCI_PORT_RO) | (state & XHCI_PORT_RWS);
}

void xhci_ssport_set_state(struct xhc_comp *xhci, int bOn)
{
	MS_UINT32 temp;

	temp = readl((void*)(xhci->xhci_port_addr));
	printf("port status 0x%x: 0x%lx\n", xhci->xhci_port_addr, temp);

	if (bOn) {
		if ((temp & PORT_PLS_MASK) == USB_SS_PORT_LS_SS_DISABLED) {

			temp = xhci_port_state_to_neutral(temp);
			temp &= ~PORT_PLS_MASK;
			temp |= PORT_LINK_STROBE | USB_SS_PORT_LS_RX_DETECT;

			writel(temp, (void*)(xhci->xhci_port_addr));

			wait_ms(10);
			temp = readl((void*)(xhci->xhci_port_addr));
			printf("port status: 0x%lx\n", temp);
		}
	} else {
		if ((temp & PORT_PLS_MASK) != USB_SS_PORT_LS_SS_DISABLED) {

			temp = xhci_port_state_to_neutral(temp);
			writel(temp | PORT_PE, (void*)(xhci->xhci_port_addr));

			wait_ms(10);
			temp = readl((void*)(xhci->xhci_port_addr));
			printf("port status: 0x%lx\n", temp);
		}
	}

}
//--------------------

