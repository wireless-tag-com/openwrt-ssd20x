/*
* drvusbmain4.h- Sigmastar
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
#ifndef DRVUSBMAIN4__H
#define DRVUSBMAIN4__H

#if defined(__ARM__)
#define HW_BASE         0x1f200000
#else
#define HW_BASE         0xbf200000
#endif

//=================== 1.Condition Definition  ============================================================
//========================================================================================================

	#define mbFUSBH200_HALFSPEEDEnable_Set()		  (mwHost20Bit_Set4(0x40,BIT2))
	#define mbFUSBH200_Interrupt_OutPut_High_Set()	  (mwHost20Bit_Set4(0x40,BIT3))
       #define mbFUSBH200_Interrupt_OutPut_Low_Set()     (mwHost20Bit_Clr4(0x40,BIT3))
	#define mbFUSBH200_VBUS_OFF_Set()		          (mwHost20Bit_Set4(0x40,BIT4))
	#define mbFUSBH200_VBUS_ON_Set()   		          (mwHost20Bit_Clr4(0x40,BIT4))
	#define mbFUSBH200_VBUS_VLD_Rd()   		          (mwHost20Port4(0x40)&BIT8)

#define USBWrite_REG32(reg_adr, data)      ( (*( (volatile MS_U32 *) (reg_adr) ) ) = ((MS_U32)data) )
#define USBRead_REG32(reg_adr)             ( *( (volatile MS_U32 *) (reg_adr) ) )
#define USBWrite_REG16(reg_adr, data)      ( (*( (volatile MS_U16 *) (reg_adr) ) ) = ((MS_U16)data) )
#define USBRead_REG16(reg_adr)             ( *( (volatile MS_U16 *) (reg_adr) ) )
#define USBWrite_REG8(reg_adr, data)      ( (*( (volatile UINT8 *) (reg_adr) ) ) = ((UINT8)data) )
#define USBRead_REG8(reg_adr)             ( *( (volatile UINT8 *) (reg_adr) ) )
#define  XBYTE   ((unsigned char volatile   *) 0)

#if defined(CONFIG_KAISERIN)
#define UTMI3_BASE    0x41400
#else
#define UTMI3_BASE    0x44100
#endif

#define UTMI_WRREG8(reg_adr, data)      ( (*( (volatile UINT8 *)  (HW_BASE+UTMI3_BASE+reg_adr*2) ) ) = ((UINT8)data) )
#define UTMI_WRREG_ODD8(reg_adr, data)      ( (*( (volatile UINT8 *)  (HW_BASE+UTMI3_BASE+reg_adr*2-1) ) ) = ((UINT8)data) )
#define UTMI_RDREG8(reg_adr)        (*( (volatile UINT8 *)  (HW_BASE+UTMI3_BASE+reg_adr*2) ) )
#define UTMI_RDREG_ODD8(reg_adr)      (*( (volatile UINT8 *)  (HW_BASE+UTMI3_BASE+reg_adr*2-1) ) )


#endif
