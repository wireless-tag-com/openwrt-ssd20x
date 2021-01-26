/*
* mhal_common.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: ryan.hsiao <ryan.hsiao@sigmastar.com.tw>
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


#ifndef _MHAL_COMMON_H_
#define _MHAL_COMMON_H_


//-------------------------------------------------------------------------------------------------
//  System Data Type
//-------------------------------------------------------------------------------------------------

/// data type unsigned char, data length 1 byte
typedef unsigned char               MS_U8;                              // 1 byte
/// data type unsigned short, data length 2 byte
typedef unsigned short              MS_U16;                             // 2 bytes
/// data type unsigned int, data length 4 byte
typedef unsigned int               MS_U32;                             // 4 bytes
/// data type unsigned int, data length 8 byte
typedef unsigned long long          MS_U64;                             // 8 bytes
/// data type signed char, data length 1 byte
typedef signed char                 MS_S8;                              // 1 byte
/// data type signed short, data length 2 byte
typedef signed short                MS_S16;                             // 2 bytes
/// data type signed int, data length 4 byte
typedef signed int                 MS_S32;                             // 4 bytes
/// data type signed int, data length 8 byte
typedef signed long long            MS_S64;                             // 8 bytes
/// data type float, data length 4 byte
typedef float                       MS_FLOAT;                           // 4 bytes
/// data type pointer content
typedef unsigned long long          MS_VIRT;                            // 8 bytes
/// data type hardware physical address
typedef MS_U64                      MS_PHYADDR;                         // 8 bytes
/// data type 64bit physical address
typedef MS_U64                      MS_PHY;                             // 8 bytes
/// data type size_t
typedef unsigned long long          MS_SIZE;                            // 8 bytes
/// definition for MS_BOOL
typedef unsigned char               MS_BOOL;
/// print type  MPRI_PHY
#define                             MPRI_PHY                            "%x"
/// print type  MPRI_PHY
#define                             MPRI_VIRT                           "%tx"
#ifdef NULL
#undef NULL
#endif
#define NULL                        0


//-------------------------------------------------------------------------------------------------
//  Software Data Type
//-------------------------------------------------------------------------------------------------


/// definition for VOID
#ifndef VOID
typedef void                        VOID;
#endif
/// definition for FILEID
typedef MS_S32                      FILEID;



/*
#ifndef true
/// definition for true
#define true                        1
/// definition for false
#define false                       0
#endif
*/

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
//-------------------------------------------------------------------------------------------------
//  Software Data Type
//-------------------------------------------------------------------------------------------------

typedef MS_U16                      MHAL_AUDIO_DEV;


//-------------------------------------------------------------------------------------------------
// MHAL Interface Return  Value Define
//-------------------------------------------------------------------------------------------------


#define MHAL_SUCCESS    (0)
#define MHAL_FAILURE    (-1)
#define MHAL_ERR_ID  (0x80000000L + 0x20000000L)


/******************************************************************************
|----------------------------------------------------------------|
| 1 |   APP_ID   |   MOD_ID    | ERR_LEVEL |   ERR_ID            |
|----------------------------------------------------------------|
|<--><--7bits----><----8bits---><--3bits---><------13bits------->|
******************************************************************************/

#define MHAL_DEF_ERR( module, level, errid) \
    ((MS_S32)( (MHAL_ERR_ID) | ((module) << 16 ) | ((level)<<13) | (errid) ))

typedef enum
{
    E_MHAL_ERR_LEVEL_INFO,       /* informational                                */
    E_MHAL_ERR_LEVEL_WARNING,    /* warning conditions                           */
    E_MHAL_ERR_LEVEL_ERROR,      /* error conditions                             */
    E_MHAL_ERR_LEVEL_BUTT
} MHAL_ErrLevel_e;

typedef enum
{
    E_MHAL_ERR_INVALID_DEVID = 1, /* invlalid device ID                           */
    E_MHAL_ERR_INVALID_CHNID = 2, /* invlalid channel ID                          */
    E_MHAL_ERR_ILLEGAL_PARAM = 3, /* at lease one parameter is illagal
                               * eg, an illegal enumeration value             */
    E_MHAL_ERR_EXIST         = 4, /* resource exists                              */
    E_MHAL_ERR_UNEXIST       = 5, /* resource unexists                            */
    E_MHAL_ERR_NULL_PTR      = 6, /* using a NULL point                           */
    E_MHAL_ERR_NOT_CONFIG    = 7, /* try to enable or initialize system, device
                              ** or channel, before configing attribute       */
    E_MHAL_ERR_NOT_SUPPORT   = 8, /* operation or type is not supported by NOW    */
    E_MHAL_ERR_NOT_PERM      = 9, /* operation is not permitted
                              ** eg, try to change static attribute           */
    E_MHAL_ERR_NOMEM         = 12,/* failure caused by malloc memory              */
    E_MHAL_ERR_NOBUF         = 13,/* failure caused by malloc buffer              */
    E_MHAL_ERR_BUF_EMPTY     = 14,/* no data in buffer                            */
    E_MHAL_ERR_BUF_FULL      = 15,/* no buffer for new data                       */
    E_MHAL_ERR_SYS_NOTREADY  = 16,/* System is not ready,maybe not initialed or
                              ** loaded. Returning the error code when opening
                              ** a device file failed.                        */
    E_MHAL_ERR_BADADDR       = 17,/* bad address,
                              ** eg. used for copy_from_user & copy_to_user   */
    E_MHAL_ERR_BUSY          = 18,/* resource is busy,
                              ** eg. destroy a venc chn without unregister it */
    E_MHAL_ERR_BUTT          = 63,/* maxium code, private error code of all modules
                              ** must be greater than it                      */
} MHAL_ErrCode_e;

/// data type 64bit physical address
typedef unsigned long long                       PHY;        // 8 bytes
typedef enum
{
    E_MHAL_PIXEL_FRAME_YUV422_YUYV = 0,
    E_MHAL_PIXEL_FRAME_ARGB8888,
    E_MHAL_PIXEL_FRAME_ABGR8888,
    E_MHAL_PIXEL_FRAME_BGRA8888,

    E_MHAL_PIXEL_FRAME_RGB565,
    E_MHAL_PIXEL_FRAME_ARGB1555,
    E_MHAL_PIXEL_FRAME_ARGB4444,
    E_MHAL_PIXEL_FRAME_I2,
    E_MHAL_PIXEL_FRAME_I4,
    E_MHAL_PIXEL_FRAME_I8,

    E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_422,
    E_MHAL_PIXEL_FRAME_YUV_SEMIPLANAR_420,
    // mstar mdwin/mgwin
    E_MHAL_PIXEL_FRAME_YUV_MST_420,

    //vdec mstar private video format
    E_MHAL_PIXEL_FRAME_YC420_MSTTILE1_H264,
    E_MHAL_PIXEL_FRAME_YC420_MSTTILE2_H265,
    E_MHAL_PIXEL_FRAME_YC420_MSTTILE3_H265,

    E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_RG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_GR,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_BG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_8BPP_GB,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_RG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_GR,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_BG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_10BPP_GB,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_RG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_GR,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_BG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_12BPP_GB,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_14BPP_RG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_14BPP_GR,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_14BPP_BG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_14BPP_GB,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_RG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_GR,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_BG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_16BPP_GB,
//////
    E_MHAL_PIXEL_FRAME_RGB_BAYER_RG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_GR,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_BG,
    E_MHAL_PIXEL_FRAME_RGB_BAYER_GB,

    E_MHAL_PIXEL_FRAME_RGBIR_R0,
    E_MHAL_PIXEL_FRAME_RGBIR_G0,
    E_MHAL_PIXEL_FRAME_RGBIR_B0,
    E_MHAL_PIXEL_FRAME_RGBIR_G1,
    E_MHAL_PIXEL_FRAME_RGBIR_G2,
    E_MHAL_PIXEL_FRAME_RGBIR_I0,
    E_MHAL_PIXEL_FRAME_RGBIR_G3,
    E_MHAL_PIXEL_FRAME_RGBIR_I1,

    E_MHAL_PIXEL_FRAME_FORMAT_MAX,
} MHalPixelFormat_e;

typedef enum
{
    E_MHAL_DATA_PRECISION_8BPP,
    E_MHAL_DATA_PRECISION_10BPP,
    E_MHAL_DATA_PRECISION_12BPP,
    E_MHAL_DATA_PRECISION_14BPP,
    E_MHAL_DATA_PRECISION_16BPP,
    E_MHAL_DATA_PRECISION_MAX,
} MHalDataPrecision_e;

typedef enum
{
    E_MHAL_RGB_BAYER_RG = 0,
    E_MHAL_RGB_BAYER_GR = 1,
    E_MHAL_RGB_BAYER_GB = 2,
    E_MHAL_RGB_BAYER_BG = 3
} MHalRGBBayerID_e;

typedef enum
{
    E_MHAL_COMPRESS_MODE_NONE,//no compress
    E_MHAL_COMPRESS_MODE_SEG,//compress unit is 256 bytes as a segment
    E_MHAL_COMPRESS_MODE_LINE,//compress unit is the whole line
    E_MHAL_COMPRESS_MODE_FRAME,//compress unit is the whole frame
    E_MHAL_COMPRESS_MODE_BUTT, //number
} MHalPixelCompressMode_e;

typedef enum
{
    E_MHAL_FRAME_SCAN_MODE_PROGRESSIVE = 0x0,  // progessive.
    E_MHAL_FRAME_SCAN_MODE_INTERLACE   = 0x1,  // interlace.
    E_MHAL_FRAME_SCAN_MODE_MAX,
} MHalFrameScanMode_e;

typedef enum
{
    E_MHAL_FIELDTYPE_NONE,        //< no field.
    E_MHAL_FIELDTYPE_TOP,           //< Top field only.
    E_MHAL_FIELDTYPE_BOTTOM,    //< Bottom field only.
    E_MHAL_FIELDTYPE_BOTH,        //< Both fields.
    E_MHAL_FIELDTYPE_NUM
} MHalFieldType_e;

typedef struct MHalWindowRect_s
{
    MS_U16 u16X;
    MS_U16 u16Y;
    MS_U16 u16Width;
    MS_U16 u16Height;
}MHalWindowRect_t;

typedef enum
{
    E_MHAL_HDR_TYPE_OFF,
    E_MHAL_HDR_TYPE_VC,                 //virtual channel mode HDR, vc0->long, vc1->short
    E_MHAL_HDR_TYPE_DOL,
    E_MHAL_HDR_TYPE_EMBEDDED,  //compressed HDR mode
    E_MHAL_HDR_TYPE_LI,                //Line interlace HDR
    E_MHAL_HDR_TYPE_MAX
} MHalHDRType_e;

typedef enum
{
    E_MHAL_HDR_SOURCE_VC0,
    E_MHAL_HDR_SOURCE_VC1,
    E_MHAL_HDR_SOURCE_VC2,
    E_MHAL_HDR_SOURCE_VC3,
    E_MHAL_HDR_SOURCE_MAX
} MHal_HDRSource_e;

typedef enum
{
    E_MHAL_SNR_PAD_ID_0 = 0,
    E_MHAL_SNR_PAD_ID_1 = 1,
    E_MHAL_SNR_PAD_ID_2 = 2,
    E_MHAL_SNR_PAD_ID_3 = 3,
    E_MHAL_SNR_PAD_ID_MAX,
    E_MHAL_SNR_PAD_ID_NA = 0xFF,
} MHal_SNR_PAD_ID_e;

typedef enum
{
    E_MHAL_VIF_MODE_BT656,
    E_MHAL_VIF_MODE_DIGITAL_CAMERA, /* parallel interface */
    E_MHAL_VIF_MODE_BT1120_STANDARD,
    E_MHAL_VIF_MODE_BT1120_INTERLEAVED,
    E_MHAL_VIF_MODE_MIPI,
    E_MHAL_VIF_MODE_MAX
} MHal_VIF_IntfMode_e;

typedef enum
{
    E_MHAL_VIF_CLK_EDGE_SINGLE_UP,
    E_MHAL_VIF_CLK_EDGE_SINGLE_DOWN,
    E_MHAL_VIF_CLK_EDGE_DOUBLE,
    E_MHAL_VIF_CLK_EDGE_MAX
} MHal_VIF_ClkEdge_e;

typedef enum
{
    E_MHAL_VIF_PIN_POLAR_POS,
    E_MHAL_VIF_PIN_POLAR_NEG
} MHal_VIF_Polar_e;

typedef struct Mhal_VIF_SignalPolarity_s
{
    MHal_VIF_Polar_e   eVsyncPolarity;
    MHal_VIF_Polar_e   eHsyncPolarity;
    MHal_VIF_Polar_e   ePclkPolarity;
    MS_U32   VsyncDelay;
    MS_U32   HsyncDelay;
    MS_U32   PclkDelay;
} MHal_VIF_SyncAttr_t;

#endif // _MS_TYPES_H_
