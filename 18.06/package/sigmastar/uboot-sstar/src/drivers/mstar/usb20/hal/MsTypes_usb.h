/*
* MsTypes_usb.h- Sigmastar
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

#ifndef _MS_TYPES_H_
#define _MS_TYPES_H_

//-------------------------------------------------------------------------------------------------
//  System Data Type
//-------------------------------------------------------------------------------------------------

/// data type unsigned char, data length 1 byte
typedef unsigned char               MS_U8;                              // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short              MS_U16;                             // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned long               MS_U32;                             // 4 bytes
/// data type unsigned int, data length 8 byte
typedef unsigned long long          MS_U64;                             // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char                 MS_S8;                              // 1 byte
/// data type signed short, data length 2 byte
typedef signed short                MS_S16;                             // 2 bytes
/// data type signed int, data length 4 byte
typedef signed long                 MS_S32;                             // 4 bytes
/// data type signed int, data length 8 byte
typedef signed long long            MS_S64;                             // 8 bytes
/// data type float, data length 4 byte
typedef float                       MS_FLOAT;                           // 4 bytes
/// data type null pointer
#ifdef NULL
#undef NULL
#endif
#define NULL                        0

typedef MS_U8      BOOLEAN;   ///< BOOLEAN
/// data type hardware physical address
typedef unsigned long               MS_PHYADDR;                         // 32bit physical address

typedef MS_U8   MS_UINT8;     // 1 byte
typedef MS_U16  MS_UINT16;    // 2 bytes
typedef MS_U32  MS_UINT32;    // 4 bytes
typedef unsigned char uchar;


//-------------------------------------------------------------------------------------------------
//  Software Data Type
//-------------------------------------------------------------------------------------------------

/// definition for MS_BOOL
typedef unsigned char               MS_BOOL;
/// definition for VOID
#ifndef VOID
typedef void                        VOID;
#endif
/// definition for FILEID
typedef MS_S32                      FILEID;

//[TODO] use MS_U8, ... instead
// data type for 8051 code
//typedef MS_U16                      WORD;
//typedef MS_U8                       BYTE;


#ifndef true
/// definition for true
#define true                        1
/// definition for false
#define false                       0
#endif


#if !defined(TRUE) && !defined(FALSE)
/// definition for TRUE
#define TRUE                        1
/// definition for FALSE
#define FALSE                       0
#endif


#if defined(ENABLE) && (ENABLE!=1)
#warning ENALBE is not 1
#else
#define ENABLE                      1
#endif

#if defined(DISABLE) && (DISABLE!=0)
#warning DISABLE is not 0
#else
#define DISABLE                     0
#endif

#endif	/* _MS_TYPES_H_ */
