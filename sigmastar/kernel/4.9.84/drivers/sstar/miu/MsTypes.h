/*
* MsTypes.h- Sigmastar
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
#ifndef _MS_TYPES_H_
#define _MS_TYPES_H_

#include <linux/types.h>

//-------------------------------------------------------------------------------------------------
//  System Data Type
//-------------------------------------------------------------------------------------------------
#ifdef CONFIG_64BIT
//=============================================================================
// Type and Structure Declaration
//=============================================================================
    /// data type unsigned char, data length 1 byte
    typedef unsigned char               MS_U8;                              // 1 byte
    /// data type unsigned short, data length 2 byte
    typedef unsigned short              MS_U16;                             // 2 bytes
    /// data type unsigned int, data length 4 byte
    typedef unsigned int                MS_U32;                             // 4 bytes
    /// data type unsigned int, data length 8 byte
    typedef unsigned long               MS_U64;                             // 8 bytes
    /// data type signed char, data length 1 byte
    typedef signed char                 MS_S8;                              // 1 byte
    /// data type signed short, data length 2 byte
    typedef signed short                MS_S16;                             // 2 bytes
    /// data type signed int, data length 4 byte
    typedef signed int                  MS_S32;                             // 4 bytes
    /// data type signed int, data length 8 byte
    typedef signed long                 MS_S64;                             // 8 bytes
    /// data type float, data length 4 byte
    typedef float                       MS_FLOAT;                           // 4 bytes
    /// data type pointer content
    typedef size_t                      MS_VIRT;                            // 8 bytes
    /// data type hardware physical address
    typedef size_t                      MS_PHYADDR;                         // 8 bytes
    /// data type size_t
    typedef size_t                      MS_SIZE;                            // 8 bytes
    /// data type 64bit physical address
    typedef MS_U64                      MS_PHY;                             // 8 bytes
	
	    /// print type  MPRI_PHY
    #define                             MPRI_PHY                            "%x"
    /// print type  MPRI_VIRT
    #define                             MPRI_VIRT                           "%tx"

#else
//=============================================================================
// Type and Structure Declaration
//=============================================================================
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

	
    /// print type  MPRI_PHY
    #define                             MPRI_PHY                            "%x"
    /// print type  MPRI_PHY
    #define                             MPRI_VIRT                           "%tx"
#endif
/// data type null pointer
#ifdef NULL
#undef NULL
#endif
#define NULL                        0

#ifdef CONFIG_MP_PURE_SN_32BIT
typedef unsigned int                MS_PHY64;                         // 32bit physical address
#else
typedef	unsigned long long          MS_PHY64;                         // 64bit physical address
#endif



//-------------------------------------------------------------------------------------------------
//  Software Data Type
//-------------------------------------------------------------------------------------------------

/// definition for MS_BOOL
typedef unsigned char               MS_BOOL;
/// definition for VOID
typedef void                        VOID;
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


///Define MS FB Format, to share with GE,GOP
/// FIXME THE NAME NEED TO BE REFINED, AND MUST REMOVE UNNESSARY FMT
typedef enum
{
    /// color format I1
    E_MS_FMT_I1                     = 0x0,
    /// color format I2
    E_MS_FMT_I2                     = 0x1,
    /// color format I4
    E_MS_FMT_I4                     = 0x2,
    /// color format palette 256(I8)
    E_MS_FMT_I8                     = 0x4,
    /// color format blinking display
    E_MS_FMT_FaBaFgBg2266  = 0x6,
    /// color format for blinking display format
    E_MS_FMT_1ABFgBg12355           = 0x7,
    /// color format RGB565
    E_MS_FMT_RGB565                 = 0x8,
    /// color format ARGB1555
    /// @note <b>[URANUS] <em>ARGB1555 is only RGB555</em></b>
    E_MS_FMT_ARGB1555               = 0x9,
    /// color format ARGB4444
    E_MS_FMT_ARGB4444               = 0xa,
    /// color format ARGB1555 DST
    E_MS_FMT_ARGB1555_DST           = 0xc,
    /// color format YUV422
    E_MS_FMT_YUV422                 = 0xe,
    /// color format ARGB8888
    E_MS_FMT_ARGB8888               = 0xf,

    E_MS_FMT_GENERIC                = 0xFFFF,

} MS_ColorFormat;


typedef union _MSIF_Version
{
    struct _DDI
    {
        MS_U8                       tag[4];
        MS_U8                       type[2];
        MS_U16                      customer;
        MS_U16                      model;
        MS_U16                      chip;
        MS_U8                       cpu;
        MS_U8                       name[4];
        MS_U8                       version[2];
        MS_U8                       build[2];
        MS_U8                       change[8];
        MS_U8                       os;
    } DDI;
    struct _MW
    {
        MS_U8                                     tag[4];
        MS_U8                                     type[2];
        MS_U16                                    customer;
        MS_U16                                    mod;
        MS_U16                                    chip;
        MS_U8                                     cpu;
        MS_U8                                     name[4];
        MS_U8                                     version[2];
        MS_U8                                     build[2];
        MS_U8                                     changelist[8];
        MS_U8                                     os;
    } MW;
    struct _APP
    {
        MS_U8                                     tag[4];
        MS_U8                                     type[2];
        MS_U8                                     id[4];
        MS_U8                                     quality;
        MS_U8                                     version[4];
        MS_U8                                     time[6];
        MS_U8                                     changelist[8];
        MS_U8                                     reserve[3];
    } APP;
} MSIF_Version;

typedef MS_U8      BOOLEAN;   ///< BOOLEAN

#define COUNTOF( array )    (sizeof(array) / sizeof((array)[0]))

#define _OFF                                    0
#define _ON                                     1
#define _HIGH                                   1
#define _LOW                                    0
#define _OUTPUT                                 0
#define _INPUT                                  1

#define bit  MS_U8

#if !defined(BIT0) && !defined(BIT1)
#define BIT0  0x0001
#define BIT1  0x0002
#define BIT2  0x0004
#define BIT3  0x0008
#define BIT4  0x0010
#define BIT5  0x0020
#define BIT6  0x0040
#define BIT7  0x0080
#define BIT8  0x0100
#define BIT9  0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000
#endif

typedef unsigned char BYTE;

#endif // _MS_TYPES_H_
