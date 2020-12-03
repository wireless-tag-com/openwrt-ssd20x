/*
* hal_card_base.h- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/
/***************************************************************************************************************
 *
 * FileName hal_card_base.h
 *     @author jeremy.wang (2015/05/29)
 * Desc:
 *     This header file is independent.
 *     We could put different hal_card_base.h in different build code folder but the same name.
 *     We want to distinguish between this and others settings but the same project.
 *     Specific define is freedom for each project, but we need to put it at inner code of project define.
 *
 *     For Base Setting:
 *     (1) BASE Project/OS/CPU/FCIE/Ver.. Option Type Define
 *     (2) BASE TYPE Define
 *     (3) BASE OS/CPU define and Include Files for Different OS/CPU
 *     (4) BASE Debug System
 *     (5) BASE Project/FCIE/Ver..  Define
 *
 *     P.S. D_XX for define and D_XX__ two under line("__") to distinguish define and its define option.
 *
 ***************************************************************************************************************/

#ifndef __HAL_CARD_BASE_H
#define __HAL_CARD_BASE_H

//***********************************************************************************************************
// (1) BASE Project/OS/CPU/FCIE/ICVer.. Option Type Define
//***********************************************************************************************************

// OS Type Option
//-----------------------------------------------------------------------------------------------------------
#define D_OS__LINUX       1
#define D_OS__UBOOT       2
#define D_OS__WINCE       3
#define D_OS__EBOOT       4
#define D_OS__YBOX        5
#define D_OS__NONE        6

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

#define FALSE   0
#define TRUE    1

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

#define SD_RED "\e[1;31m"
#define SD_DEF "\e[0m"



typedef enum
{
    IP_ORDER_0      = 0,
    IP_ORDER_1,
    IP_ORDER_2,

} IpOrder;

typedef enum
{
    IP_TYPE_FCIE    = 0,
    IP_TYPE_SDIO,
    IP_TYPE_NONE,

} IpType;

typedef enum
{
    PAD_ORDER_0     = 0,
    PAD_ORDER_1,
    PAD_ORDER_2,
    PAD_ORDER_3,
    PAD_ORDER_4,
    PAD_ORDER_5,

} PadOrder;




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
    EV_OK   = 0,
    EV_FAIL = 1,

} RetEmType;


//***********************************************************************************************************
// (3) BASE OS/CPU define and Include Files for Different OS/CPU
//***********************************************************************************************************
#define D_OS                D_OS__LINUX
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
#elif (D_OS == D_OS__NONE)
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
#define sdmmc_print     printk

//###########################################################################################################
#elif (D_OS == D_OS__UBOOT)
//###########################################################################################################
#define sdmmc_print     printf

//###########################################################################################################
#elif (D_OS == D_OS__NONE)
//###########################################################################################################
#define sdmmc_print(s)  uart_write_string((unsigned char*)s)

//###########################################################################################################
#elif (D_OS == D_OS__YBOX)
//###########################################################################################################
#define sdmmc_print(s)  printf

//###########################################################################################################
#else
//###########################################################################################################
#define sdmmc_print(s)  printf

//###########################################################################################################
#endif
//###########################################################################################################


//***********************************************************************************************************
// (5) BASE Project/FCIE/Ver.. Define
//***********************************************************************************************************

#define D_ICVER             D_ICVER_00
#define D_FCIE_R_VER        D_FCIE_R_VER__00
//#define D_BDVER           0

#endif //End of __HAL_CARD_BASE_H
