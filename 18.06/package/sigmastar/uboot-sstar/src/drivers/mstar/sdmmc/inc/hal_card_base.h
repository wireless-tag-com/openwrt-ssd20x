/*
* hal_card_base.h- Sigmastar
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
 * FileName hal_card_base.h
 *     @author jeremy.wang (2015/05/29)
 * Desc:
 *	   This header file is independent.
 * 	   We could put different hal_card_base.h in different build code folder but the same name.
 *	   We want to distinguish between this and others settings but the same project.
 * 	   Specific define is freedom for each project, but we need to put it at inner code of project define.
 *
 *	   For Base Setting:
 *     (1) BASE Project/OS/CPU/FCIE/Ver.. Option Type Define
 * 	   (2) BASE TYPE Define
 *     (3) BASE OS/CPU define and Include Files for Different OS/CPU
 *     (4) BASE Debug System
 *     (5) BASE Project/FCIE/Ver..  Define
 *
 * 	   P.S.	D_XX for define and D_XX__ two under line("__") to distinguish define and its define option.
 *
 ***************************************************************************************************************/

#ifndef __HAL_CARD_BASE_H
#define __HAL_CARD_BASE_H

//***********************************************************************************************************
// (1) BASE Project/OS/CPU/FCIE/ICVer.. Option Type Define
//***********************************************************************************************************

// Project Option Type Define
//-----------------------------------------------------------------------------------------------------------
#define D_PROJECT__C3	  1    //Cedric Project
#define D_PROJECT__C4	  2    //Chicago Project
#define D_PROJECT__iNF    3    //iNfinity Project
#define D_PROJECT__iNF3   4    //iNfinity3 Project
#define D_PROJECT__iNF2   5    //iNfinity2 Project
#define D_PROJECT__iNF5   6    //iNfinity6 Project
#define D_PROJECT__iNF6   7    //iNfinity6 Project
#define D_PROJECT__iNF2M  8    //iNfinity2m Project
#define D_PROJECT__iNF6E  9    //iNfinity6e Project
#define D_PROJECT__iNF6B0 10  //iNfinity6b0 Project
// OS Type Option
//-----------------------------------------------------------------------------------------------------------
#define D_OS__LINUX       1
#define D_OS__UBOOT       2
#define D_OS__WINCE       3
#define D_OS__EBOOT       4
#define D_OS__ROM         5
#define D_OS__YBOX        6

// CPU Type Option
//-----------------------------------------------------------------------------------------------------------
#define D_CPU__ARM        1
#define D_CPU__MIPS       2

// FCIE Type Major Version
//-----------------------------------------------------------------------------------------------------------
#define D_FCIE_M_VER__04        1
#define D_FCIE_M_VER__05        2

// FCIE Type Revision Version
//-----------------------------------------------------------------------------------------------------------
#define D_FCIE_R_VER__00        0
#define D_FCIE_R_VER__01        1
#define D_FCIE_R_VER__02        2
#define D_FCIE_R_VER__03        3
#define D_FCIE_R_VER__04        4
#define D_FCIE_R_VER__05        5

// BOARD Type Version
//-----------------------------------------------------------------------------------------------------------
#define D_BOARD_VER__00         0
#define D_BOARD_VER__01         1
#define D_BOARD_VER__02         2
#define D_BOARD_VER__03         3
#define D_BOARD_VER__04         4
#define D_BOARD_VER__05         5

// IC Version Option
//-----------------------------------------------------------------------------------------------------------
#define D_ICVER__00      0
#define D_ICVER__01      1
#define D_ICVER__02      2
#define D_ICVER__03      3
#define D_ICVER__04      4
#define D_ICVER__05      5
#define D_ICVER__06      6
#define D_ICVER__07      7
#define D_ICVER__08      8
#define D_ICVER__09      9


//***********************************************************************************************************
// (2) BASE Type Define
//***********************************************************************************************************
typedef unsigned char      U8_T;
typedef unsigned short     U16_T;
typedef unsigned int       U32_T;
typedef unsigned long long U64_T;
typedef signed char        S8_T;
typedef signed short       S16_T;
typedef signed int         S32_T;
typedef signed long long   S64_T;
typedef unsigned char      BOOL_T;

#define FALSE	0
#define TRUE	1

#define BIT00_T 0x0001
#define BIT01_T 0x0002
#define BIT02_T 0x0004
#define BIT03_T 0x0008
#define BIT04_T 0x0010
#define BIT05_T 0x0020
#define BIT06_T 0x0040
#define BIT07_T 0x0080
#define BIT08_T 0x0100
#define BIT09_T 0x0200
#define BIT10_T 0x0400
#define BIT11_T 0x0800
#define BIT12_T 0x1000
#define BIT13_T 0x2000
#define BIT14_T 0x4000
#define BIT15_T 0x8000
#define BIT16_T 0x00010000
#define BIT17_T 0x00020000
#define BIT18_T 0x00040000
#define BIT19_T 0x00080000
#define BIT20_T 0x00100000
#define BIT21_T 0x00200000
#define BIT22_T 0x00400000
#define BIT23_T 0x00800000
#define BIT24_T 0x01000000
#define BIT25_T 0x02000000
#define BIT26_T 0x04000000
#define BIT27_T 0x08000000
#define BIT28_T 0x10000000
#define BIT29_T 0x20000000
#define BIT30_T 0x40000000
#define BIT31_T 0x80000000

typedef enum
{
	EV_IP_FCIE1     = 0,
	EV_IP_FCIE2     = 1,
	EV_IP_FCIE3     = 2,

} IPEmType;

typedef enum
{
	EV_SD_BANK      = 0,
	EV_CIFC_BANK    = 1,
	EV_CIFD_BANK    = 2,

} IPBankEmType;

typedef enum
{
	EV_PORT_SD      = 0,
	EV_PORT_SDIO1   = 1,
	EV_PORT_SDIO2   = 2,
	EV_PFCIE5_FCIE  = 3,   //Use Port Setting for FCIE5 Function Ctrl Setting
	EV_PFCIE5_SDIO  = 4,   //Use Port Setting for FCIE5 Function Ctrl Setting

} PortEmType;

typedef enum
{
	EV_PAD1         = 0,
	EV_PAD2         = 1,
	EV_PAD3         = 2,
    EV_PAD4         = 3,
    EV_PAD5         = 4,

} PADEmType;



typedef enum
{
	EV_BUS_LOW     = 0,
	EV_BUS_DEF     = 1,
	EV_BUS_HS      = 2,
	EV_BUS_SDR12   = 3,
	EV_BUS_SDR25   = 4,
	EV_BUS_SDR50   = 5,
	EV_BUS_SDR104  = 6,
	EV_BUS_DDR50   = 7,
	EV_BUS_HS200   = 8,

} BusTimingEmType;


typedef enum
{
	EV_OK	= 0,
	EV_FAIL	= 1,

} RetEmType;


//***********************************************************************************************************
// (3) BASE OS/CPU define and Include Files for Different OS/CPU
//***********************************************************************************************************

#define D_OS                D_OS__UBOOT
#define D_CPU               D_CPU__ARM

//###########################################################################################################
#if (D_OS == D_OS__LINUX)
//###########################################################################################################
#include <linux/kernel.h>
//###########################################################################################################
#elif (D_OS == D_OS__UBOOT)
//###########################################################################################################
#include <common.h>
//###########################################################################################################
#elif (D_OS == D_OS__ROM)
//###########################################################################################################
#include <drvUART.h>
//###########################################################################################################
#endif
//###########################################################################################################


//***********************************************************************************************************
// (4) BASE Debug System
//***********************************************************************************************************

//###########################################################################################################
#if (D_OS == D_OS__LINUX)
//###########################################################################################################
#define prtstring(s)	printk(s)
#define prtUInt(v) 		printk("%u", v)
#define prtU8(v)		printk("0x%02X", v)
#define prtU8Hex(v)		printk("0x%02X", v)
#define prtU16Hex(v)	printk("0x%04X", v)
#define prtU32Hex(v)	printk("0x%08X", v)
//###########################################################################################################
#elif (D_OS == D_OS__UBOOT)
//###########################################################################################################
#define prtstring(s)	printf(s)
#define prtUInt(v) 		printf("%u", v)
#define prtU8(v)		printf("0x%02X", v)
#define prtU8Hex(v)		printf("0x%02X", v)
#define prtU16Hex(v)	printf("0x%04X", v)
#define prtU32Hex(v)	printf("0x%08X", v)
//###########################################################################################################
#elif (D_OS == D_OS__ROM)
//###########################################################################################################
#define prtstring(s)	uart_write_string((unsigned char*)s)
#define prtUInt(v)      uart_write_U32_hex(v)
#define prtU8(v)        uart_write_U8_hex(v)
#define prtU8Hex(v)     uart_write_U8_hex(v)
#define prtU16Hex(v)	uart_write_U16_hex(v)
#define prtU32Hex(v)	uart_write_U32_hex(v)
//###########################################################################################################
#elif (D_OS == D_OS__YBOX)
//###########################################################################################################
#define prtstring(s)	printf(s)
#define prtUInt(v) 		printf("%u", v)
#define prtU8(v)		printf("0x%02X", v)
#define prtU8Hex(v)		printf("0x%02X", v)
#define prtU16Hex(v)	printf("0x%04X", v)
#define prtU32Hex(v)	printf("0x%08X", v)
//###########################################################################################################
#else
//###########################################################################################################
#define prtstring(s)    printf(s)
#define prtUInt(v)      printf("%u", v)
#define prtU8(v)        printf("0x%02X", v)
#define prtU8Hex(v)     printf("0x%02X", v)
#define prtU16Hex(v)    printf("0x%04X", v)
#define prtU32Hex(v)    printf("0x%08X", v)
//###########################################################################################################
#endif
//###########################################################################################################


//***********************************************************************************************************
// (5) BASE Project/FCIE/Ver.. Define
//***********************************************************************************************************

#define D_ICVER             D_ICVER_00
#define D_FCIE_R_VER        D_FCIE_R_VER__00
//#define D_BDVER           0


//###########################################################################################################
#if defined(CONFIG_ARCH_CEDRIC)
//###########################################################################################################
#define D_PROJECT           D_PROJECT__C3
#define D_FCIE_M_VER        D_FCIE_M_VER__04
//###########################################################################################################
#elif defined(CONFIG_ARCH_CHICAGO)
//###########################################################################################################
#define D_PROJECT           D_PROJECT__C4
#define D_FCIE_M_VER        D_FCIE_M_VER__04
//###########################################################################################################
#elif defined(CONFIG_ARCH_INFINITY)
//###########################################################################################################
#define D_PROJECT           D_PROJECT__iNF
#define D_FCIE_M_VER        D_FCIE_M_VER__05
//###########################################################################################################
#elif defined(CONFIG_ARCH_INFINITY3)
//###########################################################################################################
#define D_PROJECT           D_PROJECT__iNF3
#define D_FCIE_M_VER        D_FCIE_M_VER__05
//###########################################################################################################
#elif defined(CONFIG_ARCH_INFINITY5)
//###########################################################################################################
#define D_PROJECT           D_PROJECT__iNF5
#define D_FCIE_M_VER        D_FCIE_M_VER__05
//###########################################################################################################
#elif defined(CONFIG_ARCH_INFINITY6)
//###########################################################################################################
#define D_PROJECT           D_PROJECT__iNF6
#define D_FCIE_M_VER        D_FCIE_M_VER__05
//###########################################################################################################
#elif defined(CONFIG_ARCH_INFINITY2M)
//###########################################################################################################
#define D_PROJECT           D_PROJECT__iNF2m
#define D_FCIE_M_VER        D_FCIE_M_VER__05
//###########################################################################################################
#elif defined(CONFIG_ARCH_INFINITY6E)
//###########################################################################################################
#define D_PROJECT           D_PROJECT__iNF6E
#define D_FCIE_M_VER        D_FCIE_M_VER__05
//###########################################################################################################
#elif defined(CONFIG_ARCH_INFINITY6B0)
//###########################################################################################################
#define D_PROJECT           D_PROJECT__iNF6B0
#define D_FCIE_M_VER        D_FCIE_M_VER__05
//###########################################################################################################
#else
#error Undefined platform
#endif




#endif //End of __HAL_CARD_BASE_H
