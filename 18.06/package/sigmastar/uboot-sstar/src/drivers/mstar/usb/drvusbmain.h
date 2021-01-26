/*
* drvusbmain.h- Sigmastar
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
#ifndef Main_Host_Only__H
#define Main_Host_Only__H

#if defined(__ARM__)
#if defined(CONFIG_ARCH_CHICAGO)
#define HW_BASE         0x1f000000 
#else
#define HW_BASE         0x1f200000
#endif
#else
#define HW_BASE         0xbf200000
#endif

//=================== 1.Condition Definition  ============================================================
//========================================================================================================

	#define mbFUSBH200_HALFSPEEDEnable_Set()		  (mwHost20Bit_Set(0x40,BIT2))
	#define mbFUSBH200_Interrupt_OutPut_High_Set()	  (mwHost20Bit_Set(0x40,BIT3))
       #define mbFUSBH200_Interrupt_OutPut_Low_Set()     (mwHost20Bit_Clr(0x40,BIT3))
	#define mbFUSBH200_VBUS_OFF_Set()		          (mwHost20Bit_Set(0x40,BIT4))
	#define mbFUSBH200_VBUS_ON_Set()   		          (mwHost20Bit_Clr(0x40,BIT4))
	#define mbFUSBH200_VBUS_VLD_Rd()   		          (mwHost20Port(0x40)&BIT8)

#define USBWrite_REG32(reg_adr, data)      ( (*( (volatile MS_U32 *) (reg_adr) ) ) = ((MS_U32)data) )
#define USBRead_REG32(reg_adr)             ( *( (volatile MS_U32 *) (reg_adr) ) )
#define USBWrite_REG16(reg_adr, data)      ( (*( (volatile MS_U16 *) (reg_adr) ) ) = ((MS_U16)data) )
#define USBRead_REG16(reg_adr)             ( *( (volatile MS_U16 *) (reg_adr) ) )
#define USBWrite_REG8(reg_adr, data)      ( (*( (volatile UINT8 *) (reg_adr) ) ) = ((UINT8)data) )
#define USBRead_REG8(reg_adr)             ( *( (volatile UINT8 *) (reg_adr) ) )
#define  XBYTE   ((unsigned char volatile   *) 0)

#define UTMIBaseAddr    0xbf807500
//#define UTMI_RDREG8(reg_adr)             ( *( (volatile U8 *) (0xbf806280+reg_adr*2) ) )

#if 1//def CONFIG_USB_HOST0
   #define UTMI_WRREG8(reg_adr, data)      ( (*( (volatile UINT8 *)  (HW_BASE+0x7500+reg_adr*2) ) ) = ((UINT8)data) )
   #define UTMI_WRREG_ODD8(reg_adr, data)      ( (*( (volatile UINT8 *)  (HW_BASE+0x7500+reg_adr*2-1) ) ) = ((UINT8)data) )
   #define UTMI_RDREG8(reg_adr)        (*( (volatile UINT8 *)  (HW_BASE+0x7500+reg_adr*2) ) )
   #define UTMI_RDREG_ODD8(reg_adr)      (*( (volatile UINT8 *)  (HW_BASE+0x7500+reg_adr*2-1) ) )

//   #define UTMI_WRREG16(reg_adr, data)     ( (*( (volatile UINT16 *) (0xbf807500+reg_adr*4) ) ) = ((UINT16)data) )
#else
   #define UTMI_WRREG8(reg_adr, data)      ( (*( (volatile UINT8 *)  (HW_BASE+0x01a00+reg_adr*2) ) ) = ((UINT8)data) )
  // #define UTMI_WRREG16(reg_adr, data)     ( (*( (volatile UINT16 *) (0xbf801a00+reg_adr*4) ) ) = ((UINT16)data) )
#endif


  extern void FUSBH200_Driver_VBUS(void);
  extern void FUSBH200_Drop_VBUS(void);


#endif
