/*
* sata_test.h- Sigmastar
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
#ifndef SATA_TEST_H
#define SATA_TEST_H

/// data type unsigned char, data length 1 byte
typedef unsigned char               MS_U8;                              // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short              MS_U16;                             // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned int                MS_U32;                             // 4 bytes
/// data type unsigned int, data length 8 byte
typedef unsigned long long          MS_U64;                             // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char                 MS_S8;                              // 1 byte
/// data type signed short, data length 2 byte
typedef signed short                MS_S16;                             // 2 bytes
/// data type signed int, data length 4 byte
typedef signed int                  MS_S32;                             // 4 bytes
/// data type signed int, data length 8 byte
typedef signed long long            MS_S64;                             // 8 bytes
/// data type float, data length 4 byte
typedef float                       MS_FLOAT;                           // 4 bytes
/// data type pointer content
typedef size_t                      MS_VIRT;                            // 8 bytes
/// data type hardware physical address
typedef size_t                      MS_PHYADDR;                         // 8 bytes
/// data type 64bit physical address
typedef MS_U64                      MS_PHY;                             // 8 bytes
/// data type size_t
typedef size_t                      MS_SIZE;                            // 8 bytes
/// definition for MS_BOOL
typedef unsigned char               MS_BOOL;                            // 1 byte
/// print type  MPRI_PHY
#define                             MPRI_PHY                            "%x"
/// print type  MPRI_PHY
#define                             MPRI_VIRT                           "%tx"

typedef unsigned char         bool;

#ifdef NULL
    #undef NULL
#endif
#define NULL                        0

#define TRUE                        1
/// definition for FALSE
#define FALSE                       0
#define ENABLE                      1
#define DISABLE                     0

// Debug Message
///ASCII color code
#define ASCII_COLOR_RED     "\033[1;31m"
#define ASCII_COLOR_WHITE   "\033[1;37m"
#define ASCII_COLOR_YELLOW  "\033[1;33m"
#define ASCII_COLOR_BLUE    "\033[1;36m"
#define ASCII_COLOR_GREEN   "\033[1;32m"
#define ASCII_COLOR_END     "\033[0m"

#if 1
    //#define FUNC_MSG(fmt, args...)            ({do{printf(ASCII_COLOR_GREEN"%s[%d] ",__FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}while(0);})
    #define FUNC_MSG(fmt, args...)            ({do{printf(fmt, ##args);}while(0);})
    #define FUNC_ERR(fmt, args...)            ({do{printf(ASCII_COLOR_RED"%s[%d] ",__FUNCTION__,__LINE__);printf(fmt, ##args);printf(ASCII_COLOR_END);}while(0);})
#else
    #define FUNC_MSG(fmt, args...)
    #define FUNC_ERR(fmt, args...)
#endif
// Buffer Management
#define SATA_MIU                 0

typedef struct
{
    MS_U16 u16PortNo;
    MS_U16 u16GenNo;
    MS_U16 u16LoopbackEnable;
    MS_U16 u16TxTestMode;
    MS_U16 u16TxTestSSCEnable;
} ST_SATA_Test_Property;

#endif // SATA_TEST_H
