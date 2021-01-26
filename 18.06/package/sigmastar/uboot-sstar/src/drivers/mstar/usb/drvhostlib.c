/*
* drvhostlib.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
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

#include <common.h>
#include "usb.h"
#include "drvhostlib.h"
#include "drvusbmain.h"
#include <malloc.h>
#include "usb_defs.h"
#include "ehci_usb.h"

#define Scsi_Max_Transfer_Len   16*1024 //65536;;yuwen

 UINT8 OTGH_GETDESCRIPTOR_DEVICE_8[]  = {0x80,0x06,0x00,0x01,0x00,0x00,0x08,0x00};
 UINT8 OTGH_GETDESCRIPTOR_DEVICE[]    = {0x80,0x06,0x00,0x01,0x00,0x00,0x12,0x00};
 UINT8 OTGH_GETDESCRIPTOR_CONFIG[]    = {0x80,0x06,0x00,0x02,0x00,0x00,0x08,0x00};

 UINT8 OTGH_SETADDRESS[]              = {0x00,0x05,03,0x00,0x00,0x00,0x00,0x00};

 UINT8 OTGH_SETCONFIGURATION[]        = {0x00,0x09,0x01,0x00,0x00,0x00,0x00,0x00};
 UINT8 OTGH_SET_FEATURE_OTG[]         = {0x00,0x03,0x03,0x00,0x00,0x00,0x00,0x00};
 UINT8 OTGH_SETDESCRIPTOR_DEVICE[]    = {0x00,0x07,0x00,0x01,0x00,0x00,0x12,0x00}; //Set Device Descriptor

 UINT8 QtdBuf[Host20_qTD_SIZE*Host20_qTD_MAX+0x20*4] __attribute__ ((aligned (128)));

 qHD_Structure  Host20_qHD_List_Control0 __attribute__ ((aligned (128)));
 qHD_Structure  Host20_qHD_List_Control1 __attribute__ ((aligned (128)))  ;
 qHD_Structure  Host20_qHD_List_Bulk0 __attribute__ ((aligned (128)));
 qHD_Structure  Host20_qHD_List_Bulk1 __attribute__ ((aligned (128))) ;
 qHD_Structure  Host20_qHD_List_Intr     __attribute__ ((aligned (128)));

 Periodic_Frame_List_Structure  Host20_FramList;
 qHD_Structure *pHost20_qHD_List_Control0,*pHost20_qHD_List_Control1;
 qHD_Structure *pHost20_qHD_List_Bulk0,*pHost20_qHD_List_Bulk1;
 qHD_Structure *pHost20_qHD_List_Intr;
 Periodic_Frame_List_Structure  *psHost20_FramList;

static UINT16 waIntervalMap[11]={1,2,4,8,16,32,64,128,256,512,1024};//

 //qHD_Structure  Host20_qHD_List_Work  _at_  (Host20_STRUCTURE_qHD_BASE_ADDRESS+4*Host20_qHD_SIZE);

 //qHD_Structure  *psHost20_qHD_List_Control[2];
// qHD_Structure  *psHost20_qHD_List_Bulk[2];

UINT8      Host20_qTD_Manage[Host20_qTD_MAX];        //1=>Free 2=>used
//Host20_Attach_Device_Structure   sAttachDevice  ;

//Host20_Attach_Device_Structure   *psDevice_AP;
UINT8 *pUsbCtrlBuf;
UINT8 UsbCtrlBuf[0x100] __attribute__ ((aligned (128)));
UINT8  pUsbData[128]  __attribute__ ((aligned (128))) ;
//volatile UINT32 gwLastqTDSendOK;
UINT8 *Host20_STRUCTURE_qTD_BASE_ADDRESS,*Host20_STRUCTURE_qHD_BASE_ADDRESS;
UINT8 *qTD_Base_Buf;
UINT8  bSpeed,bSendStatusError;
extern int _s32UsbEventId;
int  gTotalBytes;

int gInQHDArrayNum;
int gOutQHDArrayNum;
int FirstBulkIn,FirstBulkOut;

extern UINT8 XBYTE_READ(UINT32 Addr, UINT8 offset);
extern void XBYTE_SET(UINT32 Addr, UINT8 offset,UINT8 val);

#define BIT(x)	(1<<(x))
//extern void Chip_Flush_Memory(void);
#if defined(CONFIG_ARCH_CEDRIC)
#define  reg_flush_op_on_fire           0x1F203114
#define  reg_Flush_miu_pipe_done_flag   0x1F203140
#endif
#if defined(CONFIG_ARCH_CHICAGO)
#define  reg_flush_op_on_fire           (0x1F000000 + 0x1000*2 + 0x45*4)
#define  reg_Flush_miu_pipe_done_flag   (0x1F000000 + 0x1000*2 + 0x50*4)
#endif

void Chip_Flush_Memory(void)
{
//	unsigned long   dwLockFlag = 0;
	unsigned short dwReadData = 0;

	//toggle the flush miu pipe fire bit
	*(volatile unsigned short *)(reg_flush_op_on_fire) = 0x0;
	*(volatile unsigned short *)(reg_flush_op_on_fire) = 0x1;

	do
	{
		dwReadData = *(volatile unsigned short *)(reg_Flush_miu_pipe_done_flag);
		dwReadData &= BIT(12);	//Check Status of Flush Pipe Finish
			
	} while(dwReadData == 0);
}

//extern void Chip_Read_Memory(void);
void Chip_Read_Memory(void)
{
//	unsigned long   dwLockFlag = 0;
	unsigned short dwReadData = 0;

	//toggle the flush miu pipe fire bit
	*(volatile unsigned short *)(reg_flush_op_on_fire) = 0x0;
	*(volatile unsigned short *)(reg_flush_op_on_fire) = 0x1;

	do
	{
		dwReadData = *(volatile unsigned short *)(reg_Flush_miu_pipe_done_flag);
		dwReadData &= BIT(12);	//Check Status of Flush Pipe Finish
			
	} while(dwReadData == 0);
}

U32 MsOS_PA2KSEG1(U32 addr)
{
	return ((U32)(((U32)addr) | 0x80000000));
}

U32 MsOS_PA2KSEG0(U32 addr)
{
	return ((U32)(((U32)addr) | 0x40000000));
}

U32 MsOS_VA2PA(U32 addr)
{
	return ((U32)(((U32)addr) & 0x1fffffff));
}

//====================================================================
// * Function Name: OTGH_PT_Bulk_Init
// * Description:
// * Input: none
// * OutPut: none
//====================================================================
void USB_Bulk_Init(struct usb_device *dev)
{
    struct usb_endpoint_descriptor *ep,*ep2;

    ep=&dev->config.if_desc[0].ep_desc[0];
    ep2=&dev->config.if_desc[0].ep_desc[1];

    printf("bulk max packet size: ep %x ep2 %x\n", ep->wMaxPacketSize, ep2->wMaxPacketSize);

   //<5>.Hook the qHD
       if ((ep->bmAttributes)==OTGH_ED_BULK)
          {//<5.1>.stop Asynchronous Schedule
             flib_Host20_Asynchronous_Setting(HOST20_Disable);

                //<5.2>.Hook the qHD for ED0~ED3
           //       SetPointer(&(pHost20_qHD_List_Control1->bNextQHDPointer3),VirtoPhyAddr((UINT16)pHost20_qHD_List_Bulk0));
           //       SetPointer(&(pHost20_qHD_List_Bulk0->bNextQHDPointer3),VirtoPhyAddr((UINT16)pHost20_qHD_List_Bulk1));
            //      SetPointer(&(pHost20_qHD_List_Bulk1->bNextQHDPointer3),VirtoPhyAddr((UINT16)&Host20_qHD_List_Control0));
                  pHost20_qHD_List_Control1->bNextQHDPointer= (VA2PA((UINT32)pHost20_qHD_List_Bulk0)>>5);
                  pHost20_qHD_List_Bulk0->bNextQHDPointer=(VA2PA((UINT32)pHost20_qHD_List_Bulk1)>>5);
                  pHost20_qHD_List_Bulk1->bNextQHDPointer=(VA2PA((UINT32)pHost20_qHD_List_Control0)>>5);




           //<5.2>.Enable Asynchronous Schedule
           flib_Host20_Asynchronous_Setting(HOST20_Enable);

           }
        FirstBulkIn=1;
        FirstBulkOut=1;




       //Setting Max Packet Size and print message
         pHost20_qHD_List_Bulk0->bMaxPacketSize=ep->wMaxPacketSize;
         pHost20_qHD_List_Bulk1->bMaxPacketSize=ep2->wMaxPacketSize;
         pHost20_qHD_List_Bulk0->bDeviceAddress=dev->devnum;
         pHost20_qHD_List_Bulk1->bDeviceAddress=dev->devnum;
         pHost20_qHD_List_Bulk0->bEdNumber=(ep->bEndpointAddress) & 0x0f; //set endpoint address
         pHost20_qHD_List_Bulk1->bEdNumber=(ep2->bEndpointAddress) & 0x0f; //set endpoint address


         //For Bulk-Out = 2K bytes
  #if 0

#endif
        // wTemp=sOTGH_PT_BLK->pbDataPage_In[0];
         //for (i=0;i<(sOTGH_PT_BLK->wDataRange);i++)
             // *(wTemp+i)=0x88;

    if (((ep->bEndpointAddress)&0x80)>0)
       {
         gInQHDArrayNum=0;//Array 0           //for in 0, for out,1
         gOutQHDArrayNum=1;//Array 0
       }
    else
      {
         gInQHDArrayNum=1;//Array 0           //for in 0, for out,1
         gOutQHDArrayNum=0;//Array 0
       }


}
//====================================================================
// * Function Name: flib_Host20_Interrupt_Init
// * Description:
//   //Reserve=> <1>.If Periodic Enable => Disable Periodic
//   <2>.Disable all the Frame List (Terminal=1)
//   <3>.Hang the Interrupt-qHD-1 to Frame List
//
// * Input: wInterval=1~16 => Full Speed => 1ms ~ 32 sec
//                            High Speed => 125us ~ 40.96 sec
// * OutPut:
//====================================================================
void  flib_Host20_Interrupt_Init(struct usb_device *dev)
{
 UINT32 i,j;
 UINT32 wSize;
 UINT8  /*bInterruptNum,*/x;
 UINT8 *pData;
 UINT16 wForceInterval=0;
  struct usb_interface_descriptor *ip;

       ip=&dev->config.if_desc[0].desc;

 //<1>.Disable the Periodic
       flib_Host20_Periodic_Setting(HOST20_Disable);

 //<2>.Init qHD for Interrupt(7~9) Scan the ED

       //bInterruptNum=0;

       for (i=0; i< (ip->bNumEndpoints) ; i++)
           {
            if (ip->ep_desc[i].bmAttributes==OTGH_ED_INT)       //that is an interrupt endpoint
               {
                wSize=ip->ep_desc[i].wMaxPacketSize;
                     wForceInterval=ip->ep_desc[i].bInterval;
                   pHost20_qHD_List_Intr=&Host20_qHD_List_Intr;

                    pData=(UINT8*)pHost20_qHD_List_Intr;
                   //for (j=0;j<Host20_qHD_SIZE;j++)
                     for (j=0;(j<sizeof(qHD_Structure))&&(j<Host20_qHD_SIZE);j++)
                       *(pData+j)=0;

                    flib_Host20_Allocate_QHD(pHost20_qHD_List_Intr,HOST20_HD_Type_QH,dev->devnum,0,(i+1),wSize);//Address=0,Head=1,EndPt=0,Size

                 pHost20_qHD_List_Intr->bHighBandwidth=1;
                 pHost20_qHD_List_Intr->bInterruptScheduleMask=1;
                   pHost20_qHD_List_Intr->bEdSpeed= bSpeed;
                   pHost20_qHD_List_Intr->bDataToggleControl=0;
                   pHost20_qHD_List_Intr->bInterruptScheduleMask=1;     //interrupt schedule
                   pHost20_qHD_List_Intr->bHighBandwidth=1;  //1~3, MULTI =1
#if 0
             if (psDevice_AP->bDeviceOnHub==1)
                if (psDevice_AP->bSpeed!=HOST20_Attach_Device_Speed_High)
                    {//Init For Device on Hub
                     psHost20_qHD_List_Interrupt[bInterruptNum]->bHubAddr=sAttachDevice.bAdd;
                     psHost20_qHD_List_Interrupt[bInterruptNum]->bPortNumber=psDevice_AP->bOnHubPortNumber+1;
                     psHost20_qHD_List_Interrupt[bInterruptNum]->bSplitTransactionMask=0xFC;//??
                     psHost20_qHD_List_Interrupt[bInterruptNum]->bInterruptScheduleMask=0x01;//??
                     psHost20_qHD_List_Interrupt[bInterruptNum]->bControlEdFlag=0;//For Bulk
                    }// if (psDevice_AP->bDeviceOnHub==1)
#endif
                    break;          //support only one interrupt pipe
                //bInterruptNum++;
             }//    if (psDevice_AP->saCD[0].sInterface[0].sED[i].bED_bmAttributes==OTGH_ED_INT)




            }//   for (i=0;i<psDevice_AP->saCD[0].sInterface[0].bEP_NUMBER;i++)
         //Supose 1 endpoint for Interrupt
      //   i=0;
        // while(i<(bInterruptNum-1))

              pHost20_qHD_List_Intr->bNextQHDPointer=(VA2PA((UINT32)pHost20_qHD_List_Intr)>>5);      //point to itself
              pHost20_qHD_List_Intr->bTerminate=1;          //terminated


        psHost20_FramList=&Host20_FramList;
  //<3>.Link qHD to the FameListCell by wInterval
        for (i=0;i<Host20_Preiodic_Frame_List_MAX;i++)
            psHost20_FramList->sCell[i].bTerminal=1;
       //Find the Interval-X
        x=0;
        while(waIntervalMap[x]<wForceInterval)
        {
          x++;
          if (x>10)
             {
                 printf("Interval Input Error...\n");
                 return;
             }
          };

        for (i=0;i<Host20_Preiodic_Frame_List_MAX;i=i+waIntervalMap[x])
             { psHost20_FramList->sCell[i].bLinkPointer=((UINT32)(pHost20_qHD_List_Intr))>>5;
               psHost20_FramList->sCell[i].bTerminal=0;
               psHost20_FramList->sCell[i].bType=HOST20_HD_Type_QH;
             }


 //<4>.Set Periodic Base Address
     mwHost20_PeriodicBaseAddr_Set((UINT32)psHost20_FramList);


 //<5>.Enable the periodic
       flib_Host20_Periodic_Setting(HOST20_Enable);

}
//====================================================================
// * Function Name: flib_Host20_Periodic_Setting
// * Description:
// * Input:
// * OutPut:
//====================================================================
void flib_Host20_Periodic_Setting(UINT8 bOption)
{
  if (bOption==HOST20_Enable)
     {
     //<1>.If Already enable => return
           if (mwHost20_USBSTS_PeriodicStatus_Rd()>0)
               return ;

     //<2>.Disable Periodic
           mbHost20_USBCMD_PeriodicEnable_Set();

     //<3>.Polling Status
          // while(mwHost20_USBSTS_PeriodicStatus_Rd()==0);

     }else
     if (bOption==HOST20_Disable)
                {
                 //<1>.If Already Disable => return
                       if (mwHost20_USBSTS_PeriodicStatus_Rd()==0)
                           return ;

                 //<2>.Enable Periodic
                       mbHost20_USBCMD_PeriodicEnable_Clr();

                 //<3>.Polling Status
                 //      while(mwHost20_USBSTS_PeriodicStatus_Rd()>0);

                }
                else
                {
                    printf("??? Input Error 'flib_Host20_Periodic_Setting'...");
                }


}
//====================================================================
// * Function Name: flib_Host20_Issue_Interrupt
// * Description:
// * Input:
// * OutPut:
//====================================================================
void  flib_Host20_Issue_Interrupt(UINT8 bArrayListNum,UINT8 *buf,UINT16 hwSize)
{

 qTD_Structure *spTempqTD;

  //<1>.Fill TD
        spTempqTD =flib_Host20_GetStructure(Host20_MEM_TYPE_qTD); //The qTD will be release in the function "Send"
        if(spTempqTD==NULL) return;

        spTempqTD->bTotalBytes=hwSize;
        spTempqTD->ArrayBufferPointer_Word[0]=VA2PA((UINT32)(buf));        //cache issue, if cache enable
        spTempqTD->ArrayBufferPointer_Word[1]=0;
        spTempqTD->ArrayBufferPointer_Word[2]=0;
        spTempqTD->ArrayBufferPointer_Word[3]=0;
        spTempqTD->ArrayBufferPointer_Word[4]=0;

  //<2>.Analysis the Direction
         spTempqTD->bPID=HOST20_qTD_PID_IN;
 //<3>.Send TD
       flib_Host20_Send_qTD(spTempqTD ,pHost20_qHD_List_Intr,1000);


}
//************************************************************************************************************
//************************************************************************************************************
//                          *** Group-1:Main Function ***
//*************************************************************************************************************
//************************************************************************************************************
unsigned char mwHost20Port(int bOffset)
{
    if (bOffset & 1)
        return(*((UINT8 volatile   *) ( Host20_BASE_ADDRESS+bOffset*2-1)));
    else                                                                                                                //even
       return(*((UINT8 volatile   *) ( Host20_BASE_ADDRESS+bOffset*2)));
}
void mwHost20Port_wr(int bOffset,int value)
{
    if (bOffset & 1)
        (*((UINT8 volatile   *) ( Host20_BASE_ADDRESS+bOffset*2-1)))=value;
    else                                                                                                                //even
        (*((UINT8 volatile   *) ( Host20_BASE_ADDRESS+bOffset*2)))=value;
}

void mwHost20Portw(int bOffset,int value)             //bOffset should be 32 bits alignment
{
        *((UINT32 volatile   *)(Host20_BASE_ADDRESS+bOffset*2))=value & 0xffff;
        *((UINT32 volatile   *)(Host20_BASE_ADDRESS+bOffset*2+4))=(value>>16) & 0xffff;

}
int  mwHost20Bit_Rd(int bByte,int wBitNum)
{
   return (mwHost20Port(bByte)&wBitNum);
}
void  mwHost20Bit_Set(int bByte,int wBitNum)
{
    UINT8 temp;
    temp=mwHost20Port(bByte);
    temp|=wBitNum;
    mwHost20Port_wr(bByte,temp);

}
void  mwHost20Bit_Clr(int bByte,int wBitNum)
{
    UINT8 temp;
    temp=mwHost20Port(bByte);
    temp&=~wBitNum;

    mwHost20Port_wr(bByte,temp);
}

//====================================================================
// * Function Name: flib_OTGH_Init
// * Description: Init the Host HW and prepare the ED/TD
//   <1>.Init All the Data Structure
//       <1.1>.Build control list
//       <1.2>.Build Bulk list
//       <1.3>.Build Interrupt list
//       <1.4>.Build ISO list (Reserved)
//   <2>.Reset the chip
//   <3>.Set HW register
//       <3.1>.Enable FIFO-Enable(0x100->Bit5) & FPGA-Mode-Half-Speed (0x100->Bit1)
//       <3.2>.Enable interrupts
//       <3.3>.Periodic List Base Address register
//       <3.4>.USBCMD (Interrupt/Threshod/Frame List/Run-Stop)
//
// * Input: wTimeWaitForConnect_ms:The time of waitting for connecting
// * OutPut: 0:Device do not connect
//           1:Host init ok
//           2:Bus Rest Fail
//====================================================================
#define   wTimeWaitForConnect_ms 9500
extern void InitUSBIntr( void );

UINT8 flib_OTGH_Init(UINT8 wForDevice_B, UINT16 wDelay)
{
   UINT8 wValue=0;
   UINT16 wTimer_ms=0;

   USB_DELAY(wDelay);
   if (mwHost20_PORTSC_ConnectStatus_Rd() == 0)
   {
    printf("No USB is connecting\n");
    return (0);
   }

   //<1>.Waiting for the Device connect
#if 1
   if (wForDevice_B==0) {
      mbHost20_USBCMD_HCReset_Set();            //host controller reset
      while(mbHost20_USBCMD_HCReset_Rd()>0);
   }
#endif
   wValue=0;
   wTimer_ms=0;
   do {
      wValue=mwHost20_PORTSC_ConnectStatus_Rd();
          //  printf("wValue:%02bx\n",wValue);

      if (wValue==0) {
         USB_DELAY(1);//10, wait 1 ms
         wTimer_ms++;
      }
      //if (mwHost20_PORTSC_ConnectStatus_Rd()==0) return 0;
      if (wTimer_ms>wTimeWaitForConnect_ms)          // Case1:Waiting for 10 sec=10000
      {                                           // Case2:Waiting for 100 ms =100
         printf("??? Waiting for Peripheral Connecting Fail...\n");
         return (0);
      }
   }while(wValue==0);


   mwHost20_Misc_EOF1Time_Set(Host20_EOF1Time);
   //printf("34:%02bx\n",XBYTE[0x2434]);
   //<2>.Init All the Data Structure & write Base Address register
   flib_Host20_InitStructure();

   //Write Base Address to Register
  // Host20_STRUCTURE_BASE_ADDRESS
   //printf("Async base addr: 0x%x \n", VA2PA((UINT32)Host20_STRUCTURE_qHD_BASE_ADDRESS));

   mwHost20_CurrentAsynchronousAddr_Set(VA2PA((UINT32)Host20_STRUCTURE_qHD_BASE_ADDRESS));
    //printf("Reg 0x28: 0x%x 0x%x\n", *((UINT32 volatile   *)(Host20_BASE_ADDRESS+0x28*2)),
    //                 *((UINT32 volatile   *)(Host20_BASE_ADDRESS+0x28*2+4)));

   if (wForDevice_B==0) {
      if (flib_Host20_PortBusReset()>0)
         return(2);
   }
 if (bSpeed==0)                 //full speed
  {

    //XBYTE[UTMIBaseAddr+0x09]&=0x7F;
       UTMI_WRREG_ODD8(0x9,UTMI_RDREG_ODD8(0x9)&(0x7f));

    mwHost20_Misc_EOF1Time_Set(2);

  }
  else if (bSpeed==2)                //high speed
  {
    //XBYTE[UTMIBaseAddr+0x09]|=0x80;              //HS rx robust enable
     UTMI_WRREG_ODD8(0x9,UTMI_RDREG_ODD8(0x9)|(0x80));

    mwHost20_Misc_EOF1Time_Set(3);

  }

   flib_Host20_QHD_Control_Init();

   //printf("Async base addr: 0x%x \n", (UINT32)Host20_STRUCTURE_qHD_BASE_ADDRESS);


 // printf("return 1");
  //  InitUSBIntr();

   return (1);
}


//====================================================================
// * Function Name: flib_Host20_Close
// * Description:
//   <1>.Suspend Host
//   <2>.Disable the interrupt
//   <3>.Clear Interrupt Status
//   <4>.Issue HW Reset
//   <5>.Free the Allocated Memory
// * Input:
// * OutPut:
//====================================================================
UINT8 flib_Host20_Close(UINT8 bForDeviceB)
{
   UINT32 wTemp;

   if (mwHost20_USBINTR_Rd()>0) {
      //<1>.Suspend Host
      if (bForDeviceB==0)
        {
         flib_Host20_Suspend();
        }
      else
         flib_Host20_StopRun_Setting(HOST20_Disable);

      //<2>.Disable the interrupt
      mwHost20_USBINTR_Set(0);

      //<3>.Clear Interrupt Status
      wTemp=mwHost20_USBSTS_Rd();
      wTemp=wTemp&0x0000003F;
      mwHost20_USBSTS_Set(wTemp);
   }

   return (1);
}

//====================================================================
// * Function Name: flib_Host20_StopRun_Setting
// * Description:
// * Input:
// * OutPut:
//====================================================================
void flib_Host20_StopRun_Setting(UINT8 bOption)
{
   if (bOption==HOST20_Enable) {
      if (mbHost20_USBCMD_RunStop_Rd()>0)
         return;

      mbHost20_USBCMD_RunStop_Set();

      while(mbHost20_USBCMD_RunStop_Rd()==0);
   }
   else if (bOption==HOST20_Disable) {
      if (mbHost20_USBCMD_RunStop_Rd()==0)
         return;

      mbHost20_USBCMD_RunStop_Clr();

      while(mbHost20_USBCMD_RunStop_Rd()>0);
   }
   else {
      printf("??? Input Error 'flib_Host20_StopRun_Setting'...");
      while(1);
   }
}

//====================================================================
// * Function Name: flib_Host20_Asynchronous_Setting
// * Description:
// * Input:
// * OutPut:
//====================================================================
void flib_Host20_Asynchronous_Setting(UINT8 bOption)
{
   if (bOption==HOST20_Enable) {
      if (mwHost20_USBSTS_AsynchronousStatus_Rd()>0)
         return;

      mbHost20_USBCMD_AsynchronousEnable_Set();

      while(mwHost20_USBSTS_AsynchronousStatus_Rd()==0);
   }
   else if (bOption==HOST20_Disable) {
      if (mwHost20_USBSTS_AsynchronousStatus_Rd()==0)
         return;

      mbHost20_USBCMD_AsynchronousEnable_Clr();

      while(mwHost20_USBSTS_AsynchronousStatus_Rd()>0);
   }
   else {
      printf("??? Input Error 'flib_Host20_Asynchronous_Setting'...\n");
      // while(1);
   }
}

//void _noop_()
//{
   //  int i=0;
//}
//====================================================================
// * Function Name: flib_Host20_PortBusReset
// * Description:
//   <1>.Waiting for HCHalted=0
//   <2>.Write PortEnable=0(Reserved for Faraday-IP)
//   <3>.Write PortReset=0
//   <4>.Wait time
//   <5>.Write PortReset=0
//   <6>.Waiting for IRS->PortChangeDetect
// * Input:
// * OutPut:
//====================================================================
UINT8 flib_Host20_PortBusReset(void)
{
//   UINT8 ttt;
  UINT32 wTmp;
//   flib_Host20_ForceSpeed(0);

   //<1>.Disable RunStop

// printf("disable run\n");
   if (mbHost20_USBCMD_RunStop_Rd()>0)
      flib_Host20_StopRun_Setting(HOST20_Disable);
//   printf("0x10 ->%02bx \n",XBYTE[0x2410]);

   //<2>.Write PortReset=0

    UTMI_WRREG8(0x2C, 0x10);
    UTMI_WRREG_ODD8(0x2D, 0);
    UTMI_WRREG8(0x2E, 0x00);
    UTMI_WRREG_ODD8(0x2F, 0);
    UTMI_WRREG8(0x2A, 0x80);//Chirp K detection level: 0x80 => 400mv, 0x20 => 575mv

   mwHost20_PORTSC_PortReset_Set();

   //<3>.Wait time=>55ms
 //  flib_Host20_TimerEnable(55);
    USB_DELAY(50);
    UTMI_WRREG8(0x2A, 0x00);
    USB_DELAY(20);
   // flib_Debug_LED_Off_All();; //GPIO-High

   //<4>.Write PortReset=0
  // sAttachDevice.bPortReset=1;
 //temporarily marked  ,yuwen
   mwHost20_PORTSC_PortReset_Clr();

#if (ENABLE_AMBER3) || (ENABLE_EAGLE) || (ENABLE_KAISER)
    /* for early 40nm setting */
    UTMI_WRREG8(0x2C, 0x98);
    UTMI_WRREG_ODD8(0x2D, 0x02);
    UTMI_WRREG8(0x2E, 0x10);
    UTMI_WRREG_ODD8(0x2F, 0x01);
#else    /* for 55nm and later 40nm with 55nm setting */
    UTMI_WRREG8(0x2C, 0x10);
    UTMI_WRREG_ODD8(0x2D, 0x02);
    UTMI_WRREG_ODD8(0x2F, 0x81);
#endif

  // printf("Host Speed:%02bx\n",mwOTG20_Control_HOST_SPD_TYP_Rd());

   //<5>.Waiting for IRS->PortChangeDetect
  // printf("wait reset\n");
  #if 1
   wTmp=0;

   while (1)
   {
      if (mwHost20_PORTSC_PortReset_Rd()==0)  break;

      //MsOS_DelayTask(4);
      //if (mwHost20_PORTSC_ConnectStatus_Rd()==0) return 1;
      wTmp++;
      if (wTmp>20000)
    {
         printf("??? Error waiting for Bus Reset Fail...==> Reset HW Control\n");
         mbHost20_USBCMD_HCReset_Set();
         while(mbHost20_USBCMD_HCReset_Rd()==1);
         return (1);
       }
   }
   #endif
   //<6>.Enable RunStop Bit
#if 0
   if (mwHost20_PORTSC_ForceSuspend_Rd())
    {
        printf("port suspend\n");
        mwHost20_PORTSC_ForceResume_Set();          //force resume
        USBDELAY(14);
        mwHost20_PORTSC_ForceResume_Clr();
    }
#endif

 // XBYTE[UTMIBaseAddr+0x06]|=0x03;        //reset UTMI
 // XBYTE[UTMIBaseAddr+0x06]&=0xfc;
    UTMI_WRREG8(6,UTMI_RDREG8(6)|0x03);
    UTMI_WRREG8(6,UTMI_RDREG8(6)&0xfc);

          flib_Host20_StopRun_Setting(HOST20_Enable);

      USB_DELAY(5);     //wait some slow device to be ready
   //<7>.Detect Speed
    bSpeed= mwOTG20_Control_HOST_SPD_TYP_Rd();
    printf("Host type:%x \n",  bSpeed);
   //<8>.Delay 20 ms
   return (0);
}

//====================================================================
// * Function Name: flib_Host20_Suspend
// * Description:
//   <1>.Make sure PortEnable=1
//   <2>.Write PORTSC->Suspend=1
//   <3>.Waiting for the ISR->PORTSC->Suspend=1
// * Input:
// * OutPut: 0:OK
//           1:Fail
//====================================================================
UINT8 flib_Host20_Suspend(void)
{
   if (mbHost20_USBCMD_RunStop_Rd()==0)
      return(1);

   //<1>.Make sure PortEnable=1
   if (mwHost20_PORTSC_EnableDisable_Rd()==0)
      return(1);

   //<2>.Write PORTSC->Suspend=1
   flib_Host20_StopRun_Setting(HOST20_Disable);//For Faraday HW request

   //<3>.Write PORTSC->Suspend=1
   mwHost20_PORTSC_ForceSuspend_Set();

   //<4>.Waiting for the ISR->PORTSC->Suspend=1
#if 0
   flib_Host20_TimerEnable_UnLock(1);//1sec
   bExitFlag=0;

   do {
      if (mwHost20_PORTSC_ForceSuspend_Rd()>0)
         bExitFlag=1;

      if (gwOTG_Timer_Counter>5) {
         bExitFlag=1;
         printf(">>> Fail => Time Out for Waiting ForceSuspend...\n");
      }
   }
   while(bExitFlag==0);
#else
  while(mwHost20_PORTSC_ForceSuspend_Rd()==0);
#endif

   return (0);
}





void Dump_QTD(UINT32 addr);
void Dump_Data(UINT32 addr,UINT16 sz);
//====================================================================
 //For Control-Single qTD// * Function Name: flib_Host20_Issue_Control
// * Description:
//   <1>.Analysis the Controller Command => 3 type
//   <2>.Case-1:"Setup/In/Out' Format..."
//       (get status/get descriptor/get configuration/get interface)
//   <3>.Case-2:'Setup/In' Format...      => Faraday Driver will not need
//       (clear feature/set feature/set address/set Configuration/set interface  )
//   <4>.Case-3:'Setup/Out/In'
//       (set descriptor)
// * Input:
// * OutPut: 0: OK
//           X:>0 => Fail
//====================================================================
UINT8 flib_Host20_Issue_Control (UINT8 bEdNum,UINT8* pbCmd,UINT16 hwDataSize,UINT8* pbData)
{
   qTD_Structure  *spTempqTD;
//   UINT32       bpDataPage;
   UINT8        bReturnValue;
 // UINT8 i;
 qHD_Structure  *qh_ptr;
//  UINT8 i;

if (bEdNum==0)
     qh_ptr=pHost20_qHD_List_Control0;
else
{
    pHost20_qHD_List_Control1->bDeviceAddress = bEdNum;
    qh_ptr=pHost20_qHD_List_Control1;
}



   //<0>.Allocate qTD & Data Buffer
   spTempqTD=flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD
   if(spTempqTD==NULL){
      return HOST20_FATAL;
   }

  // bpDataPage=sAttachDevice.bDataBuffer;           //2//2k buffer

   //<2.1>.Setup packet
   //<A>.Fill qTD
   spTempqTD->bPID=HOST20_qTD_PID_SETUP;                   //Bit8~9
   spTempqTD->bTotalBytes=8;           //Bit16~30

   spTempqTD->bDataToggle=0;            //Bit31
  //  FillBufferArray(spTempqTD, bpDataPage);
   memcpy(pUsbCtrlBuf,pbCmd,8);


   spTempqTD->ArrayBufferPointer_Word[0]=VA2PA((UINT32)pUsbCtrlBuf);

  // printf("bpDataPage:%x\n",bpDataPage);
//   memcpy((UINT8  *)bpDataPage,pbCmd,8);

#if 1
   //<B>.Send qTD
  //  Dump_QTD(spTempqTD);

  //  Dump_Data(0x2400,0x50);
 //  bReturnValue=flib_Host20_Send_qTD(spTempqTD ,psHost20_qHD_List_Control[bEdNum],5);
     bReturnValue=flib_Host20_Send_qTD(spTempqTD ,qh_ptr,Host20_Timeout);
   if (bReturnValue>0)
               goto exit_issue_control;

 //  printf("c3");

   //<1>.Analysis the Controller Command
   switch (*(pbCmd+1)) { // by Standard Request codes
      // <2>.Case-1:"Setup/In/Out' Format..."
      case 0:       // get status
      case 6:       // get descriptor
      case 8:       // get configuration
      case 10:  // get interface
      case 0xfe:        //get Max Lun
         //<2.2>.In packet
    SetupRead:
         //<A>.Fill qTD
         spTempqTD=flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD
         if(spTempqTD==NULL){
            return HOST20_FATAL;
         }
         spTempqTD->bPID=HOST20_qTD_PID_IN;                   //Bit8~9
         spTempqTD->bTotalBytes=hwDataSize;           //Bit16~30

         spTempqTD->bDataToggle=1;            //Bit31

       //  spTempqTD->ArrayBufferPointer_Word[0]=VirtoPhyAddr(bpDataPage);
         //FillBufferArray(spTempqTD, bpDataPage);
         spTempqTD->ArrayBufferPointer_Word[0]=VA2PA((UINT32)pUsbCtrlBuf);


         //<B>.Send qTD
         //bReturnValue=flib_Host20_Send_qTD(spTempqTD ,psHost20_qHD_List_Control[bEdNum],5);
              bReturnValue=flib_Host20_Send_qTD(spTempqTD ,qh_ptr,Host20_Timeout);
         if (bReturnValue>0)
                   goto exit_issue_control;
//   printf("c4");


         //<C>.Waiting for result
         memcpy((UINT8  *)pbData,pUsbCtrlBuf,hwDataSize);

     //    Dump_Data((UINT16)pbData,hwDataSize);

         //<2.3>.Out packet

         //<A>.Fill qTD
         spTempqTD=flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD
         if(spTempqTD==NULL){
            return HOST20_FATAL;
         }
         spTempqTD->bPID=HOST20_qTD_PID_OUT;                   //Bit8~9
         spTempqTD->bTotalBytes=0;           //Bit16~30

         spTempqTD->bDataToggle=1;            //Bit31

         //<B>.Send qTD
       //  bReturnValue=flib_Host20_Send_qTD(spTempqTD ,psHost20_qHD_List_Control[bEdNum],5);
            bReturnValue=flib_Host20_Send_qTD(spTempqTD ,qh_ptr,Host20_Timeout);

         if (bReturnValue>0)
                   goto exit_issue_control;


         break;

      //<3>.Case-2:'Setup/In' Format...      => Faraday Driver will not need
      case 0xf0:   //read
      case 0xf1:    //write

          if (*(pbCmd)==0xc0)  goto SetupRead;
          else if (*(pbCmd)==0x40)  goto SetupWrite;

          break;

      case 1:       // clear feature
      case 3:       // set feature
      case 5:       // set address
      case 9:       // set Configuration
      case 11:  // set interface
      case 0xff:             //device reset

         //<3.2>.In packet

         //<A>.Fill qTD
         spTempqTD=flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD
         if(spTempqTD==NULL){
            return HOST20_FATAL;
         }
         spTempqTD->bPID=HOST20_qTD_PID_IN;                   //Bit8~9
         spTempqTD->bTotalBytes=hwDataSize;           //Bit16~30
         spTempqTD->bDataToggle=1;            //Bit31
         spTempqTD->ArrayBufferPointer_Word[0]=VA2PA((UINT32)pUsbCtrlBuf);
         //FillBufferArray(spTempqTD, bpDataPage);

         //<B>.Send qTD
        // bReturnValue=flib_Host20_Send_qTD(spTempqTD ,psHost20_qHD_List_Control[bEdNum],5);
             bReturnValue=flib_Host20_Send_qTD(spTempqTD ,qh_ptr,Host20_Timeout);

         if (bReturnValue>0)
           goto exit_issue_control;
         //<C>.Copy Result
        // memcpy(pbData,pUsbCtrlBuf,hwDataSize);
         break;

      //<4>.Case-3:'Setup/Out/In'
      case 7:       // set descriptor
         //<4.2>.Out packet
         //<A>.Fill qTD
       SetupWrite:
         spTempqTD=flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD
         if(spTempqTD==NULL){
            return HOST20_FATAL;
         }
         spTempqTD->bPID=HOST20_qTD_PID_OUT;                   //Bit8~9
         spTempqTD->bTotalBytes=hwDataSize;           //Bit16~30
         spTempqTD->bDataToggle=1;            //Bit31
         spTempqTD->ArrayBufferPointer_Word[0]=VA2PA((UINT32)pUsbCtrlBuf);
        // FillBufferArray(spTempqTD, bpDataPage);

         memcpy(pUsbCtrlBuf,pbData,hwDataSize);

         //<B>.Send qTD
      //   bReturnValue=flib_Host20_Send_qTD(spTempqTD ,psHost20_qHD_List_Control[bEdNum],5);
           bReturnValue=flib_Host20_Send_qTD(spTempqTD ,qh_ptr,Host20_Timeout);

         if (bReturnValue>0)
                     goto exit_issue_control;


         //<4.3>.In packet
         //<A>.Fill qTD
         spTempqTD=flib_Host20_GetStructure(Host20_MEM_TYPE_qTD);//0=>qTD
         if(spTempqTD==NULL){
            return HOST20_FATAL;
         }
         spTempqTD->bPID=HOST20_qTD_PID_IN;                   //Bit8~9
         spTempqTD->bTotalBytes=0;           //Bit16~30
         spTempqTD->bDataToggle=1;            //Bit31

         //<B>.Send qTD
       //  bReturnValue=flib_Host20_Send_qTD(spTempqTD ,psHost20_qHD_List_Control[bEdNum],5);
            bReturnValue=flib_Host20_Send_qTD(spTempqTD ,qh_ptr,Host20_Timeout);

         if (bReturnValue>0)
                       goto exit_issue_control;

      break;

      default:
          if ( *(pbCmd) & 0x80 )  goto SetupRead;
          else if ( !(*(pbCmd) & 0x80) )  goto SetupWrite;
         break;
   }
   #endif
   return (0);
exit_issue_control:

        return (bReturnValue);

}
void Dump_QTD(UINT32 addr)
{
        UINT8 i;
        printf("QH/QTD:%x -> \n",addr);
        for (i=0; i < 0x20 ; i=i+4)
        {
            printf("%02x ",*(unsigned char volatile  *)(addr+i+3));
            printf("%02x ",*(unsigned char volatile  *)(addr+i+2));
            printf("%02x ",*(unsigned char volatile  *)(addr+i+1));
            printf("%02x ",*(unsigned char volatile  *)(addr+i));
            printf("\n");
        }

}
void Dump_Data(UINT32 addr,UINT16 sz)
{
        UINT16 i,xxx=0;
        printf("addr:%x -> \n",addr);

        while (sz >= 0x10)
            {
               printf("\nADDR %x -> ",xxx);
                for (i=0; i < 0x10 ; i++)
                    printf("%02x ",*(unsigned char volatile  *)(addr+i));
                sz-=0x10;
                addr+=0x10;
                xxx+=0x10;
            }
            printf("\nADDR %x -> ",xxx);

             for (i=0; i < sz ; i++)
                    printf("%02x ",*(unsigned char volatile  *)(addr+i));

}
UINT8 Send_Receive_Bulk_Data(void *buffer,int len,int dir_out)
{

 // UINT8 *pbDataPage[5];
  UINT32 wTotalLengthRemain=0;
  UINT32 buf=0;
  UINT8 result=0;
  UINT16  TransferLen=0;
//printf("data phase\n");

  //<1>.To fill the data buffer

      wTotalLengthRemain=len;

      buf=(UINT32)buffer;


  //<2>.Issue Transfer

  while (wTotalLengthRemain)
    {
        if(wTotalLengthRemain > Scsi_Max_Transfer_Len)
                    TransferLen=Scsi_Max_Transfer_Len;
        else
                    TransferLen=wTotalLengthRemain;

           if (dir_out)
           {

            result=flib_Host20_Issue_Bulk (gOutQHDArrayNum,TransferLen
                             ,buf,OTGH_Dir_Out);

               if (result!=HOST20_OK) return result;

           }
            else
            {

           result=flib_Host20_Issue_Bulk (gInQHDArrayNum ,TransferLen
                             ,buf,OTGH_Dir_IN);

               if (result!=HOST20_OK) return result;
             }
           buf+=TransferLen;
           wTotalLengthRemain-=TransferLen;

    }
 return(result);

}


//====================================================================
// * Function Name: flib_Host20_Issue_Bulk
// * Description: Input data must be 4K-Alignment
//               <1>.MaxSize=20 K
//               <2>.Support Only 1-TD
// * Input:
// * OutPut:
//====================================================================
//#ifdef MS_NOSAPI
//UINT8 usb_temp_buf[Scsi_Max_Transfer_Len] __attribute__ ((aligned (32)));
//#endif
UINT8  flib_Host20_Issue_Bulk (UINT8 bArrayListNum,UINT16 hwSize,UINT32 pwBuffer,UINT8 bDirection)
{
    qTD_Structure  *spTempqTD=NULL;
    UINT8 bTemp=0,i=0;
    UINT16 count=0;
    UINT32 addr=0;
    UINT32 mybuf,workbuf=0/*,tempbuf*/;
    int    TranSize=0;
    qHD_Structure   *spTempqH=NULL;

     mybuf=0;
     mybuf=mybuf;
     //tempbuf=0;
     if ( pwBuffer !=(UINT32) KSEG02KSEG1(pwBuffer) )
    {
#if 0
         if (pwBuffer & 0x7)            //flush should be 8 bytes aligned
         {
            printf("flush cache buf ");
           //  #ifndef MS_NOSAPI
             tempbuf=(UINT32)malloc(hwSize);  //temporarily buffer for USB control
             mybuf=(UINT32) KSEG02KSEG1(tempbuf);         //uncached buffer
             //#else
                   //   mybuf=(UINT32) KSEG02KSEG1(usb_temp_buf);
                 //  #endif
              if ( tempbuf == 0 )  printf("usb memory allocate failed!\n");

        }
         else
#endif
        {

        //    printf(" fuh ");
          //  dcache_flush_range((void*)pwBuffer,hwSize);       //flush buffer to uncached buffer
          //marked , dcache not ready yet
           flush_cache((ulong)pwBuffer,hwSize);     //flush buffer to uncached buffer
                #if (_USB_FLUSH_BUFFER == 1)
                Chip_Flush_Memory();
                #endif
            pwBuffer=(UINT32)KSEG02KSEG1(pwBuffer);
        }
    }
     #if 0
     else
     {

          if (pwBuffer & 0x7)
             {
             //   printf(" buf ");

             tempbuf=(UINT32)malloc(hwSize );  //temporarily buffer for USB control
                mybuf=(UINT32) KSEG02KSEG1(tempbuf);         //uncached buffer
              if ( tempbuf == 0 )  printf("usb memory allocate failed!\n");

            }
         }
     #endif


        spTempqTD =flib_Host20_GetStructure(Host20_MEM_TYPE_qTD); //The qTD will be release in the function "Send"
        if(spTempqTD==NULL){
            return HOST20_FATAL;
        }
        spTempqTD->bTotalBytes=hwSize ;

      //    FillBufferArray2(spTempqTD,pwBufferArray,hwSize);         //use global buffer , because it is 4k alignment
#if 0
       spTempqTD->ArrayBufferPointer_Word[0]=pwBufferArray;
    if (((pwBufferArray&0xfff)+hwSize) > 0xfff)     //goto page 2
          spTempqTD->ArrayBufferPointer_Word[1]=pwBufferArray+0x1000;       //+4K
    if (hwSize > (0x1000+0x1000-(pwBufferArray&0xfff))
#endif
//not allow buffer over 16K for my usage
   //if (mybuf==0)            //use original buf
     workbuf=pwBuffer;
   /*else
     workbuf=mybuf;*/

    spTempqTD->ArrayBufferPointer_Word[0] = VA2PA(workbuf);

     count = 0x1000 - (workbuf & 0x0fff);  /* rest of that page */
     if ( hwSize < count)    /* ... iff needed */
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
//              lastnums++;
            workbuf += 0x1000;
            if ((count + 0x1000) < hwSize)
               count += 0x1000;
            else
                count = hwSize;
           }

  }
    if (bArrayListNum==0)
        {
        spTempqH=pHost20_qHD_List_Bulk0;
       // printf("use bulk 0\n");
        }
  else  // if (bArrayListNum==1)
    {
        //printf("use bulk 1\n");
        spTempqH=pHost20_qHD_List_Bulk1;
    }

  //<2>.Analysis the Direction
          if (bDirection==OTGH_Dir_IN)
            {
                 spTempqTD->bPID=HOST20_qTD_PID_IN;
                 if (FirstBulkIn)
            {
            spTempqTD->bDataToggle=0;
            spTempqH->bDataToggleControl=1;
            }
            }
          else
          {
                  spTempqTD->bPID=HOST20_qTD_PID_OUT;
                  if (FirstBulkOut)
            {
            spTempqTD->bDataToggle=0;
            spTempqH->bDataToggleControl=1;

            }
    //      if (mybuf)  memcpy((void*)mybuf,(void*)pwBuffer,hwSize);    //copy buffer
          }
  //<3>.Send TD
//  if (bArrayListNum==0)
   //     bTemp=flib_Host20_Send_qTD(spTempqTD ,pHost20_qHD_List_Bulk0,Host20_Timeout);
 // else if (bArrayListNum==1)
        bTemp=flib_Host20_Send_qTD(spTempqTD ,spTempqH,Host20_Timeout);

        if ((FirstBulkIn)&&(bDirection==OTGH_Dir_IN))
        {
            spTempqH->bDataToggleControl=0;
            FirstBulkIn=0;
        }
        if ((FirstBulkOut)&&(bDirection==OTGH_Dir_Out))
        {
            spTempqH->bDataToggleControl=0;
            FirstBulkOut=0;
        }

       if (bDirection==OTGH_Dir_IN)
       {
       #if 1
                 TranSize=hwSize-gTotalBytes;
                 if ((TranSize % 8)==7)
                 {
                        //printf("patch wallace bug\n");
                        *((UINT8*)workbuf+TranSize-1)=*((UINT8*)workbuf+TranSize+3); //fix hardware bug
                 }
                 else if ((TranSize % 8)==3)
                 {
                        //printf("patch wallace bug\n");
                        *((UINT8*)workbuf+TranSize-1)=*((UINT8*)workbuf+TranSize+7); //fix hardware bug
                 }
      #endif
       }

   //    if (tempbuf)
     //  free((void*)tempbuf);
//
     //  printf("Z");
        return (bTemp);

}









//************************************************************************************************************
//************************************************************************************************************
//                          *** Group-4:Structure Function ***
//*************************************************************************************************************
//************************************************************************************************************
//====================================================================
// * Function Name: flib_Host20_InitStructure
// * Description:
//              1.Init qHD for Control
//                qHD_C-->qHD_C-->qHD_C
//              2.Init qHD for Bulk
//                |-------------------------|
//                qHD_C-->qHD_C-->qHD_B-->qHD_B
//
//              3.Init qHD for Interrupt
//              4.Init iTD for ISO (Reserved for feature)
// * Input:Type =0 =>iTD
//              =1 =>qTD
//              =2
// * OutPut: 0:Fail
//           1:ok
//====================================================================
void flib_Host20_InitStructure(void)
{
   UINT16   i;
   UINT8 *pData;

   //<1>.Clear memory

      // XBYTE[Host20_STRUCTURE_qHD_BASE_ADDRESS+i]=0x00;

    //for ( i=0 ; i < Host20_qTD_SIZE*Host20_qTD_MAX ; i++)
     //  XBYTE[Host20_STRUCTURE_qTD_BASE_ADDRESS+i]=0x00;

    pUsbCtrlBuf= (UINT8 *)KSEG02KSEG1(UsbCtrlBuf);

   // ASSERT(pUsbCtrlBuf != NULL)

    qTD_Base_Buf= (UINT8 *)KSEG02KSEG1(QtdBuf);
    //ASSERT(qTD_Base_Buf != NULL)
    Host20_STRUCTURE_qTD_BASE_ADDRESS=qTD_Base_Buf;

    pData=qTD_Base_Buf;
    for (i=0 ; i < (Host20_qTD_SIZE*Host20_qTD_MAX+0x20) ; i++)
        pData[i]=0;

    if ((UINT32)Host20_STRUCTURE_qTD_BASE_ADDRESS& 0x10)
        Host20_STRUCTURE_qTD_BASE_ADDRESS+=0x10;            //make it aligned with 32
    pHost20_qHD_List_Control0=(qHD_Structure*)KSEG02KSEG1(&Host20_qHD_List_Control0);
    pHost20_qHD_List_Control1=(qHD_Structure*)KSEG02KSEG1(&Host20_qHD_List_Control1);
    pHost20_qHD_List_Bulk0=(qHD_Structure*)KSEG02KSEG1(&Host20_qHD_List_Bulk0);
    pHost20_qHD_List_Bulk1=(qHD_Structure*)KSEG02KSEG1(&Host20_qHD_List_Bulk1);

    pData=(UINT8*)pHost20_qHD_List_Control0;
    for ( i=0 ; i < sizeof(qHD_Structure); i++)
        pData[i]=0;
    pData=(UINT8*)pHost20_qHD_List_Control1;
    for ( i=0 ; i < sizeof(qHD_Structure); i++)
        pData[i]=0;
    pData=(UINT8*)pHost20_qHD_List_Bulk0;
    for ( i=0 ; i < sizeof(qHD_Structure); i++)
        pData[i]=0;
    pData=(UINT8*)pHost20_qHD_List_Bulk1;
    for ( i=0 ; i < sizeof(qHD_Structure); i++)
        pData[i]=0;

    Host20_STRUCTURE_qHD_BASE_ADDRESS=(UINT8*)pHost20_qHD_List_Control0;

  //  printf("qhd:%x\n",(UINT32)pHost20_qHD_List_Control0);
  //  printf("qhd1:%x\n",(UINT32)pHost20_qHD_List_Control1);
   // printf("qtd:%x\n",(UINT32)qTD_Base_Buf);
   //<2>.For qTD & iTD & 4K-Buffer Manage init
   for (i=0;i<Host20_qTD_MAX;i++)
      Host20_qTD_Manage[i]=Host20_MEM_FREE;


//   sAttachDevice.bDataBuffer=flib_Host20_GetStructure(Host20_MEM_TYPE_4K_BUFFER);//For Control


  //
 // printf("List_control 0:%x\n",(UINT16) pHost20_qHD_List_Control0);
#if 1
//psHost20_qHD_List_Control[0]->bType=HOST20_HD_Type_QH;
   flib_Host20_Allocate_QHD(pHost20_qHD_List_Control0,HOST20_HD_Type_QH,0,1,0,64);//Address=0,Head=1,EndPt=0,Size

   flib_Host20_Allocate_QHD(pHost20_qHD_List_Control1,HOST20_HD_Type_QH,1,0,0,64);//Address=1,Head=0,EndPt=0,Size

#endif



#if 1
   flib_Host20_Allocate_QHD(pHost20_qHD_List_Bulk0,HOST20_HD_Type_QH,1,0,1,64);//Address=1,Head=0,EndPt=1,Size
   flib_Host20_Allocate_QHD(pHost20_qHD_List_Bulk1,HOST20_HD_Type_QH,1,0,2,64);//Address=1,Head=0,EndPt=2,Size
#endif


   //<3.3>.Link the qHD (contol)
   #if 1

 pHost20_qHD_List_Control0->bNextQHDPointer=(VA2PA((UINT32)pHost20_qHD_List_Control1)>>5);
 pHost20_qHD_List_Control1->bNextQHDPointer=(VA2PA((UINT32)pHost20_qHD_List_Control0)>>5);

  // SetPointer(&(pHost20_qHD_List_Control0->bNextQHDPointer3),VirtoPhyAddr((UINT16)pHost20_qHD_List_Control1));
    //SetPointer(&(pHost20_qHD_List_Control1->bNextQHDPointer3),VirtoPhyAddr((UINT16)pHost20_qHD_List_Control0));
   // SetPointer(&(Host20_qHD_List_Work.bNextQHDPointer3),VirtoPhyAddr((UINT16)&Host20_qHD_List_Work));

   #endif

}

//====================================================================
// * Function Name: flib_Host20_GetStructure
// * Description:
//
// * Input:Type =0 =>qTD
//              =1 =>iTD
//              =2 =>4K Buffer
// * OutPut: 0:Fail
//           ~0:Addrress
//====================================================================
qTD_Structure *flib_Host20_GetStructure(UINT8 Type)
{
 UINT32 i;
 UINT8 bFound;
// UINT16 spTempqTD;
 qTD_Structure   *spTempqTD;
// iTD_Structure  *spTempiTD;
//  siTD_Structure  *spTempsiTD;
 bFound=0;

 switch(Type)
       {
        case Host20_MEM_TYPE_qTD:

             //For qTD

               for (i=0;i<Host20_qTD_MAX;i++)
                   if (Host20_qTD_Manage[i]==Host20_MEM_FREE)
                        {bFound=1;
                         Host20_qTD_Manage[i]=Host20_MEM_USED;
                         break;
                        }


               if (bFound==1)
                  {
                     //printf("USB base:%lx  \n",USB_BUFFER_START_ADR);

                   spTempqTD=(qTD_Structure    *)((UINT32)Host20_STRUCTURE_qTD_BASE_ADDRESS+i*Host20_qTD_SIZE);
                   memset((unsigned char   *)spTempqTD ,0, Host20_qTD_SIZE);
                   spTempqTD->bTerminate=1;         //Bit0
                   spTempqTD->bStatus_Active=0;             //Bit7
                   spTempqTD->bInterruptOnComplete=1;   //Bit15

                   spTempqTD->bAlternateTerminate=1;
                   spTempqTD->bErrorCounter=3;
                    return (spTempqTD);
                   }
               else printf("QTD underrun!\n");



        break;

        default:
               return 0;
        break;

        }

 //Not Found...


 return (0);

}
//====================================================================
// * Function Name: flib_Host20_ReleaseStructure
// * Description:
//
// * Input:Type =0 =>qTD
//              =1 =>iTD
//              =2
// * OutPut: 0:Fail
//           ~0:Addrress
//====================================================================
void flib_Host20_ReleaseStructure(UINT8 Type,UINT32 pwAddress)
{
 UINT8 i;
 UINT16 wReleaseNum;
 UINT8  *pData;

//printf("release QTD:%x\n",pwAddress);
 pData=(UINT8*)pwAddress;

  switch(Type)
        {
        case Host20_MEM_TYPE_qTD:

             if (pwAddress<(UINT32)Host20_STRUCTURE_qTD_BASE_ADDRESS)
                 {
                 printf("??? Memory release area fail...\n");
                 return;
                 }

             if ((pwAddress-(UINT32)Host20_STRUCTURE_qTD_BASE_ADDRESS)==0)
                wReleaseNum=0;
             else
                wReleaseNum=(pwAddress-(UINT32)Host20_STRUCTURE_qTD_BASE_ADDRESS)/Host20_qTD_SIZE;

            if (wReleaseNum>=Host20_qTD_MAX)//ERROR FIX Prevent Tool 070522
                 {
                 printf("??? Memory release area fail...\n");
                 return;
                 }


             Host20_qTD_Manage[wReleaseNum]=Host20_MEM_FREE;

             for (i=0;i<Host20_qTD_SIZE ;i++) //qTD size=32 bytes
                  *(pData+i)=0;


        break;



        }


}
//====================================================================
// * Function Name: flib_Host20_QHD_Control_Init
// * Description:
//
// * Input:Type =0 =>qTD
//              =1 =>iTD
//              =2
// * OutPut: 0:Fail
//           ~0:Addrress
//====================================================================
void flib_Host20_QHD_Control_Init(void)
{


  //<1>.Init Control-0/1
   pHost20_qHD_List_Control0->bEdSpeed= bSpeed;
 // printf("bEdSpeed:%bx\n",sAttachDevice.bSpeed);
     pHost20_qHD_List_Control0->bInactiveOnNextTransaction=0;
     pHost20_qHD_List_Control0->bDataToggleControl=1;

     pHost20_qHD_List_Control1->bEdSpeed= bSpeed;
     pHost20_qHD_List_Control1->bInactiveOnNextTransaction=0;
     pHost20_qHD_List_Control1->bDataToggleControl=1;

 //<2>.Init Bulk-0/1
    pHost20_qHD_List_Bulk0->bEdSpeed= bSpeed;
     pHost20_qHD_List_Bulk0->bInactiveOnNextTransaction=0;
     pHost20_qHD_List_Bulk0->bDataToggleControl=0;

     pHost20_qHD_List_Bulk1->bEdSpeed= bSpeed;
     pHost20_qHD_List_Bulk1->bInactiveOnNextTransaction=0;
     pHost20_qHD_List_Bulk1->bDataToggleControl=0;


//printf("enable aynch \n");
   //<12>.Enable Asynchronous

         mbHost20_USBCMD_AsynchronousEnable_Set();    //Temp;;Bruce
  //      printf("0x10:%02x",XBYTE[0x2410]);


}
void SetControl1MaxPacket(UINT8 max)
{
       pHost20_qHD_List_Control1->bMaxPacketSize=max;
       //printf("control1 max:%x\n",pHost20_qHD_List_Control1->bMaxPacketSize);

}
//====================================================================
// * Function Name: flib_Host20_Allocate_QHD
// * Description:
//
// * Input:Type =0 =>qTD
//              =1 =>iTD
//              =2
// * OutPut: 0:Fail
//           ~0:Addrress
//====================================================================
void flib_Host20_Allocate_QHD(qHD_Structure  *psQHTemp,UINT8 bNextType,UINT8 bAddress,UINT8 bHead,UINT8 bEndPt, UINT32 wMaxPacketSize)
{
   qTD_Structure  *spTempqTD;
  // UINT32 sp;
   psQHTemp->bTerminate=0;             //Bit0
   psQHTemp->bType=bNextType;          //Bit2~1

   psQHTemp->bDeviceAddress=bAddress;             //Bit0~6
   psQHTemp->bEdNumber=bEndPt;                  //Bit11~8
   psQHTemp->bHeadOfReclamationListFlag=bHead; //Bit15
   psQHTemp->bMaxPacketSize=wMaxPacketSize;            //Bit16~26
   psQHTemp->bNakCounter=Host20_QHD_Nat_Counter;

   psQHTemp->bOverlay_NextTerminate=1;
   psQHTemp->bOverlay_AlternateNextTerminate=1;


   //<2>.allocate dumy qTD
#if 1
   //<2.1>.Allocate qTD
   spTempqTD=flib_Host20_GetStructure(0);//0=>qTD
     psQHTemp->bOverlay_NextqTD=(VA2PA((UINT32)spTempqTD)>>5);


   #endif
   //<2.3>.Active the qTD
   psQHTemp->bOverlay_NextTerminate=0;

}
unsigned long  gUsbStatus;

//====================================================================
// * Function Name: flib_Host20_CheckingForResult_QHD
// * Description:
// * Input:Type
// * OutPut:
//====================================================================
UINT8 flib_Host20_CheckingForResult_QHD(qHD_Structure *spTempqHD)
{
    UINT8 bQHStatus;
    UINT16 wIntStatus;

    wIntStatus=mwHost20_USBSTS_Rd();
    //  printf("USB int:%x\n",wIntStatus);
    gUsbStatus=0;
    //<2>.Checking for the Error type interrupt => Halt the system
    if (wIntStatus&HOST20_USBINTR_SystemError)
    {
      // printf("???System Error... Halt the system...\n ");
      bSendStatusError=1;
    }
    if (wIntStatus&HOST20_USBINTR_USBError) {
      //printf("usb intr err\n");
      mwHost20_USBSTS_USBError_Set();
      bSendStatusError=1;
    }

    if (bSendStatusError==0)
        return HOST20_OK;
    //<1>.Analysis the qHD Status
    bSendStatusError=0;
    //  printf("??? USB Error Interrupt Event...\n");
    bQHStatus=spTempqHD->bOverlay_Status;
    // printf("bQHStatus:%x\n",bQHStatus);
    //while (1) {}              //stop to check ehci status
    if (bQHStatus&HOST20_qTD_STATUS_Halted)
    {
        spTempqHD->bOverlay_Status=0;      //clear halt status
        spTempqHD->bOverlay_TotalBytes=0;
        spTempqHD->bOverlay_Direction=0;
        if (bQHStatus==HOST20_qTD_STATUS_Halted)            //no other error status
        {
            //printf("STALL\n");
            gUsbStatus|=USB_ST_STALLED;
            return HOST20_DEVICE_STALL;
        }
        else
            printf("qHD Status => Halted (Stall/Babble/Error Counter=0)...0x%x\n", bQHStatus);
    }
    if (bQHStatus&HOST20_qTD_STATUS_BufferError)
    {
        gUsbStatus|=USB_ST_BUF_ERR;
        //printf("??? qHD Status => HOST20_qTD_STATUS_BufferError...\n");
    }
    if (bQHStatus&HOST20_qTD_STATUS_Babble)
    {
        printf("!!! qHD Status => HOST20_qTD_STATUS_Babble !!! \n");
        //sAttachDevice.bSendStatusError=0;         //don't fix babble error for Bert
        spTempqHD->bOverlay_Status=0;
        gUsbStatus|=USB_ST_BABBLE_DET;

        return HOST20_OK;

    }
    if (bQHStatus&HOST20_qTD_STATUS_TransactionError)
    {
        //printf("??? qHD Status => HOST20_qTD_STATUS_TransactionError...\n");
        gUsbStatus|=USB_ST_CRC_ERR;
    }
    if (bQHStatus&HOST20_qTD_STATUS_MissMicroFrame)
    {
        //printf("??? qHD Status => HOST20_qTD_STATUS_MissMicroFrame...\n");
        gUsbStatus|=USB_ST_BIT_ERR;
    }

    //<2>.Clear the status
    spTempqHD->bOverlay_Status=0;

    return HOST20_FAIL;
}


#if 1
//====================================================================
// * Function Name: flib_Host20_Send_qTD
// * Description:
//   Case-1:1qTD
//   Case-2:2qTD
//   Case-3:3qTD above
// * Input:Type
// * OutPut: 0 => OK
//           1 => TimeOut
//====================================================================
UINT8 flib_Host20_Send_qTD(qTD_Structure  *spHeadqTD ,qHD_Structure  *spTempqHD,int wTimeOutSec)
{
   UINT8  bReturnValue;
   qTD_Structure  *spNewDumyqTD;
   qTD_Structure  *spOldDumyqTD;
   qTD_Structure  *spReleaseqTD;
   qTD_Structure  *spReleaseqTDNext;
   qTD_Structure  *spLastqTD;
  // UINT32 wDummyTemp;
//   UINT32  *pwData;
   unsigned long wTimes;
 //  UINT32 StartTime;
//   MS_BOOL   wait;
//   UINT32                 UINT32Events = 0;

  if (wTimeOutSec==0)
        printf("error, timeout sec is zero\n");


   //spOldDumyqTD=GetPointer(&(spTempqHD->bOverlay_NextqTD3));
   spOldDumyqTD=(qTD_Structure*)PA2VA(((UINT32)(spTempqHD->bOverlay_NextqTD))<<5);

  // spTempqHD->bOverlay_Status|=HOST20_qTD_STATUS_Halted;
  #if 0
  while (mbHost20_USBCMD_AsynchronousEnable_Rd())
    {
    mbHost20_USBCMD_AsynchronousEnable_Clr();//pause asynchronous scheduler
    }
  #endif
//printf("U");
 //  spHeadqTD->bTerminate=1;           //set to terminate
   memcpy(spOldDumyqTD,spHeadqTD,Host20_qTD_SIZE);

   //<2>.Prepare new dumy qTD
   spNewDumyqTD=spHeadqTD;
   memset((unsigned char  *)spNewDumyqTD ,0, Host20_qTD_SIZE);
   spNewDumyqTD->bTerminate=1;
  // spNewDumyqTD->bAlternateTerminate=1;
   //spNewDumyqTD->bStatus_Halted=1;
   //<3>.Find spLastqTD & link spLastqTD to NewDumyqTD & Set NewDumyqTD->T=1
   spLastqTD=spOldDumyqTD;
   while(spLastqTD->bTerminate==0) {
      spLastqTD=(qTD_Structure*)PA2VA((((UINT32)(spLastqTD->bNextQTDPointer))<<5));
   };
   spLastqTD->bNextQTDPointer= VA2PA((UINT32)spNewDumyqTD)>>5;
   spLastqTD->bTerminate=0;

   //Link Alternate qTD pointer
  // SetPointer(&(spLastqTD->bAlternateQTDPointer3),VirtoPhyAddr((UINT32)spNewDumyqTD));
   spLastqTD->bAlternateQTDPointer= VA2PA((UINT32)spNewDumyqTD)>>5;

   spLastqTD->bAlternateTerminate=0;

   //sAttachDevice.psSendLastqTD=spLastqTD;
   bSendStatusError=0;

    #if 0
    printf("Before ... \n");
    printf("QH: \n");
    Dump_QTD((UINT32)spTempqHD);
    printf("TD: \n");
    Dump_QTD((UINT32)spOldDumyqTD);
    printf("After ... \n");
    #endif

// while (mwHost20_USBSTS_Rd() & 0x3f)
    //{
        mwHost20_USBSTS_Set(0x3f);      //clear interrupt
    //}
   //bExitLoop=0;
   //spOldDumyqTD->bStatus_Halted=0;
   //spOldDumyqTD->bStatus_Halted=0;
   spOldDumyqTD->bStatus_Active=1;

#if (_USB_FLUSH_BUFFER == 1)
    Chip_Flush_Memory();
#endif

 // while (1){}
#if 0
   while (mbHost20_USBCMD_AsynchronousEnable_Rd()==0)
    {
         mbHost20_USBCMD_AsynchronousEnable_Set();//re start asynchronous scheduler
    }
 #endif
   //   spTempqHD->bOverlay_Status&=~HOST20_qTD_STATUS_Halted;
  //wait until asynchronous scheduler is idle

   //mbHost20_USBCMD_RunStop_Set();

  // spTempqHD->bOverlay_Status&=~0x40;         //clr HALT bit, start this queue head
//XBYTE[0x2410]|=1;//start run
  // wDummyTemp=0;
   wTimes=0;
#if  0
   StartTime=MsOS_GetSystemTime();
   //<5>.Waiting for result
   //printf("wait flag");
   wait= MsOS_WaitEvent(_s32UsbEventId, USBWAITEVENTS, &UINT32Events, E_OR_CLEAR, wTimeOutSec*1000);
  // printf("ok");
//    if (UINT32Events & HOST20_USBINTR_CompletionOfTransaction)
    bReturnValue=HOST20_OK;
    if (wait==FALSE) bReturnValue=HOST20_FATAL;     //timeout
    if (UINT32Events & HOST20_USBINTR_PortChangeDetect)
        {
            printf("port change int\n");
           bReturnValue=HOST20_FATAL;
        }
 #else              //non interrupt
    wTimes=0;
   while (1)
    {
  flush_cache((ulong)spOldDumyqTD,0x10);       //wried, seems like cache has bug,
                                                                                 //CPU always keep its own copy,
                                                                                 //even we use non-cache memory
//mb();
        if ( mwHost20_USBSTS_Rd() & HOST20_USBINTR_CompletionOfTransaction )
        {
        #if (_USB_FLUSH_BUFFER == 1)
            Chip_Read_Memory();
        #endif
            if (spOldDumyqTD->bStatus_Active==0)
            {
                bReturnValue=HOST20_OK;
                break;
            }
        }
    //     if (bExitLoop) break;
          if (mwHost20_USBSTS_Rd()&(HOST20_USBINTR_SystemError+HOST20_USBINTR_USBError))
            {
                  bReturnValue=HOST20_OK;
                  break;            //USB interrupt happened
            }
        // if (gwOTG_Timer_Counter>wTimeOutSec)
      wTimes++;

      //
      if (mwHost20_PORTSC_ConnectStatus_Rd()==0)
      {
          printf("P0 device disconn\n");
          bReturnValue = HOST20_FATAL;
          gUsbStatus|=USB_ST_DISCONNECT;
          break;
      }

      wait_ms(1);

     // if ((MsOS_GetSystemTime()-StartTime) > wTimeOutSec *1000)

       if (wTimes > wTimeOutSec*500)
        {
         //   bExitLoop=2;
            bReturnValue=HOST20_FATAL;
            gUsbStatus|=USB_ST_TIMEOUT;

            //mbHost20_USBCMD_HCReset_Set();            //host controller reset
               //  while(mbHost20_USBCMD_HCReset_Rd()>0);

            printf(">>> Fail => Time Out for Send qTD...\n");

            #if 0

            printf("Reg 0x28: 0x%x 0x%x\n", *((UINT32 volatile   *)(Host20_BASE_ADDRESS+0x28*2)),
                             *((UINT32 volatile   *)(Host20_BASE_ADDRESS+0x28*2+4)));


            printf("QH: \n");
            Dump_QTD((UINT32)spTempqHD);
            printf("TD: \n");
            Dump_QTD((UINT32)spOldDumyqTD);


            //while(1);

            #endif

            break;
         }

    }
#endif
    gTotalBytes=spOldDumyqTD->bTotalBytes;
   //<6>.Checking the Result
   if (bReturnValue!=HOST20_FATAL)
      bReturnValue=flib_Host20_CheckingForResult_QHD(spTempqHD);

   //<5>.Release the all the qTD (Not include spNewDumyqTD)
   #if 1
   spReleaseqTD=spOldDumyqTD;
   do {
      spReleaseqTDNext=(qTD_Structure*)PA2VA(((UINT32)(spReleaseqTD->bNextQTDPointer))<<5);

      flib_Host20_ReleaseStructure(Host20_MEM_TYPE_qTD,(UINT32)spReleaseqTD);
      spReleaseqTD=spReleaseqTDNext;
   } while(((UINT32)spReleaseqTD)!=((UINT32)spNewDumyqTD));
#endif

    //<6>.Double Check the QH overlay status. Adjust it if need.
    if( spTempqHD->bOverlay_NextqTD != (VA2PA(( UINT32)spNewDumyqTD)>>5)
        || spTempqHD->bOverlay_Status & BIT7 )
    {
        // Bug of WD 3.0 disk. Disconnection happens while data face of SETUP transaction.
        // If chain qTDs after disconnection/timeout, QH overlay will not be advanced by HW.
        // It muss up qTD chain layout. QH doesn't stop at old dumy and stare at new dumy.
        // SW advance QH overlay manually no matter HW advancing or not.
        // Run bit is cleared by HQ when disconnection, so it is safe to modify the QH.

        printf("[Warning] Adjust bad qTD chain..\r\n");

        spTempqHD->bOverlay_Status = spTempqHD->bOverlay_Status & ~BIT7;
        spTempqHD->bOverlay_CurrentqTD = VA2PA(( UINT32)spOldDumyqTD) >> 5;
        spTempqHD->bOverlay_NextqTD = VA2PA(( UINT32)spNewDumyqTD) >> 5;
        spTempqHD->bOverlay_AlternateqTD = VA2PA(( UINT32)spNewDumyqTD) >> 5; 
    }
//printf("F");
   return (bReturnValue);
}

#endif

