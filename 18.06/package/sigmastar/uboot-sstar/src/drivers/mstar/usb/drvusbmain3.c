/*
* drvusbmain3.c- Sigmastar
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
#include <usb.h>
#include "drvhostlib3.h"
//#include "drvUSB.h"
#include "drvusbmain3.h"

//extern Host20_Attach_Device_Structure   sAttachDevice  ;
//static USBCallback _DrvUSB_CBFun = NULL;
extern UINT8 bSpeedP2;
// int _s32UsbEventId;
int Usb_host_Init3(void);
void MDrv_UsbClose3(void);
#if defined(ENABLE_THIRD_EHC)
//extern UINT8 bForceSpeed;//0=>All Clear 1=>Full Speed 2=>High Speed
#if 0
//U8 buf1[512];
#define  TEST_SIZE      64*1024
MS_BOOL MDrv_UsbDiskWrite( MS_U32 u32BlockAddr, MS_U32 u32BlockNum,MS_U32 u8Buffer);
MS_BOOL MDrv_UsbDiskRead(MS_U32 u32BlockAddr, MS_U32 u32BlockNum,MS_U32 u8Buffer);

void UsbTest(void)
{
MS_U32 i,j,k=0;
MS_U32 idx=0,xxx;
//U8 maxlun,lunbyte;
U8  *buf1,*buf2;
    //
 //   maxlun=	MDrv_GET_MASS_MAX_LUN();
 //   lunbyte=MDrv_GET_MASS_VALID_LUN();
	printf("usb test\n");
//	printf("maxlun :%02bx lunbyte:%02bx\n",maxlun,lunbyte);

    buf1=(U8*)MsOS_AllocateMemory(TEST_SIZE, gs32NonCachedPoolID );
    buf2=(U8*)MsOS_AllocateMemory(TEST_SIZE, gs32NonCachedPoolID );
    //printf("buf1:%x, buf2:%x\n",(MS_U32)buf1,(MS_U32)buf2);
    for (i=0 ; i < TEST_SIZE ; i++)
        {
            buf1[i]= i & 0xff;
      }

while (1)
	{
	    idx++;
          printf("Loop:%x \n",idx);
		//	MDrv_MIU_Copy
	     xxx=idx % 10000;

        // for (k=0; k < 8 ; k++)
            {
            if (MDrv_UsbDiskWrite(50+xxx+k*128,TEST_SIZE/512,(MS_U32)buf1)==FALSE)
                {
                   printf("write failed\n");
                   return;
                }
            }
         //for (k=0; k < 8 ; k++)
            {
            if (MDrv_UsbDiskRead(50+xxx+k*128,TEST_SIZE/512,(MS_U32)buf2)==FALSE)
                {
                    printf("read failed\n");
                    return;
                }
            }

	 	//MsOS_Delay_Task(10);
            for (j=0 ; j < TEST_SIZE ; j++)
                 {
                  if (buf2[j]!= buf1[j]) printf("data err\n");
                 }
          //  	MsOS_Delay_Task(500);
        }
	//time1=MDrv_Timer_TimeDifference( MDrv_Timer_GetTime0(), time1 ) ;
	//printf("time1:%lx",time1);
	//printf("Performace:%lx kBytes/sec",  (512*400)/time1);
}
#endif

int ever_inited3 = 0;

//====================================================================
// * Function Name: main
// * Description:
// * Input:
// * OutPut:
//====================================================================
int Usb_host_Init3(void)
{
    ever_inited3 = 1;

    extern void mwHost20Bit_Set3(int bByte,int wBitNum);
    mwHost20Bit_Set3(0x81,BIT6); // Data length byte alignment issue
    if (flib_OTGH_Init3(0, USB_WAIT_TIME)==0)
    {
        printf("usb init failed\n");
        return -1;
    }
    return bSpeedP2;
}

int Usb_host_PreInit3(void)
{
    extern void mwHost20Bit_Set3(int bByte,int wBitNum);
    mwHost20Bit_Set3(0x81,BIT6); // Data length byte alignment issue
    return -1;
}

int Usb_host_PostInit3(void)
{
    ever_inited3 = 1;

    extern void mwHost20Bit_Set3(int bByte,int wBitNum);
    mwHost20Bit_Set3(0x81,BIT6); // Data length byte alignment issue
    if (flib_OTGH_Init3(0, 0)==0)
    {
        printf("usb init failed\n");
        return -1;
    }
    return bSpeedP2;
}

#define mdelay(n) ({unsigned long msec=(n); while (msec--) udelay(1000);})
void MDrv_UsbClose3(void)
{
    if(ever_inited3 == 0)
        return;

    flib_Host20_StopRun_Setting3(HOST20_Disable);
    //flib_Host20_Close3(0);

    if (ever_inited3 == 2)
    {
        printf("[USB] Long time stable device. Waiting .5 second...\n");
        mdelay(500);
    }
}
#endif //#if defined(ENABLE_THIRD_EHC)
