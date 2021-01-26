/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef _API_HDMITX_H_
#define _API_HDMITX_H_

#include "mhal_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------
#ifndef DLL_PACKET

#ifndef HDMITX_OS_TYPE_UBOOT
#define DLL_PACKED __attribute__((__packed__))
#else
#define DLL_PACKED
#endif

#endif


//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------
#define MSIF_HDMITX_LIB_CODE               {'H','D','M','I'}
#define MSIF_HDMITX_LIBVER                 {'0','0'}
#define MSIF_HDMITX_BUILDNUM               {'2','0'}
#define MSIF_HDMITX_CHANGELIST             {'0','0','6','7','7','7','7','2'}
#define HDMITX_API_VERSION              /* Character String for DRV/API version             */  \
    MSIF_TAG,                           /* 'MSIF'                                           */  \
    MSIF_CLASS,                         /* '00'                                             */  \
    MSIF_CUS,                           /* 0x0000                                           */  \
    MSIF_MOD,                           /* 0x0000                                           */  \
    MSIF_CHIP,                                                                                  \
    MSIF_CPU,                                                                                   \
    MSIF_HDMITX_LIB_CODE,                  /* IP__                                             */  \
    MSIF_HDMITX_LIBVER,                    /* 0.0 ~ Z.Z                                        */  \
    MSIF_HDMITX_BUILDNUM,                  /* 00 ~ 99                                          */  \
    MSIF_HDMITX_CHANGELIST,                /* CL#                                              */  \
    MSIF_OS

typedef enum
{
    HDMITX_DVI            = 0,  // DVI without HDCP
    HDMITX_DVI_HDCP       = 1,  // DVI with HDCP
    HDMITX_HDMI           = 2,  // HDMI without HDCP
    HDMITX_HDMI_HDCP      = 3,  // HDMI with HDCP
} HDMITX_OUTPUT_MODE;

typedef enum
{
    HDMITX_SEND_PACKET        = 0x00,   // send packet
    HDMITX_CYCLIC_PACKET      = 0x04,   // cyclic packet by frame count
    HDMITX_STOP_PACKET        = 0x80,   // stop packet
} HDMITX_PACKET_PROCESS;

typedef enum
{
    HDMITX_NULL_PACKET        = 0x00,
    HDMITX_ACR_PACKET         = 0x01,
    HDMITX_AS_PACKET          = 0x02,
    HDMITX_GC_PACKET          = 0x03,
    HDMITX_ACP_PACKET         = 0x04,
    HDMITX_ISRC1_PACKET       = 0x05,
    HDMITX_ISRC2_PACKET       = 0x06,
    HDMITX_DSD_PACKET         = 0x07,
    HDMITX_HBR_PACKET         = 0x09,
    HDMITX_GM_PACKET          = 0x0A,

    HDMITX_VS_INFOFRAME       = 0x81,
    HDMITX_AVI_INFOFRAME      = 0x82,
    HDMITX_SPD_INFOFRAME      = 0x83,
    HDMITX_AUDIO_INFOFRAME    = 0x84,
    HDMITX_MPEG_INFOFRAME     = 0x85,
    HDMITX_HDR_INFOFRMAE      = 0x87, //0x86,
} HDMITX_PACKET_TYPE;

typedef enum
{
    HDMITX_VIDEO_CD_NoID     = 0, // DVI mode
    HDMITX_VIDEO_CD_24Bits     = 4, // HDMI 8 bits
    HDMITX_VIDEO_CD_30Bits     = 5, // HDMI 10 bits
    HDMITX_VIDEO_CD_36Bits     = 6, // HDMI 12 bits
    HDMITX_VIDEO_CD_48Bits     = 7, // HDMI 16 bits
} HDMITX_VIDEO_COLORDEPTH_VAL;

typedef enum
{
    HDMITX_VIDEO_COLOR_RGB444     = 0,
    HDMITX_VIDEO_COLOR_YUV422     = 1,
    HDMITX_VIDEO_COLOR_YUV444     = 2,
    HDMITX_VIDEO_COLOR_YUV420     = 3,
} HDMITX_VIDEO_COLOR_FORMAT;

typedef enum
{
    HDMITX_RES_640x480p        =0,
    HDMITX_RES_720x480i         = 1,
    HDMITX_RES_720x576i         = 2,
    HDMITX_RES_720x480p         = 3,
    HDMITX_RES_720x576p         = 4,
    HDMITX_RES_1280x720p_50Hz   = 5,
    HDMITX_RES_1280x720p_60Hz   = 6,
    HDMITX_RES_1920x1080i_50Hz  = 7,
    HDMITX_RES_1920x1080i_60Hz  = 8,
    HDMITX_RES_1920x1080p_24Hz  = 9,
    HDMITX_RES_1920x1080p_25Hz  = 10,
    HDMITX_RES_1920x1080p_30Hz  = 11,
    HDMITX_RES_1920x1080p_50Hz  = 12,
    HDMITX_RES_1920x1080p_60Hz  = 13,
    HDMITX_RES_1920x2205p_24Hz	= 14,
    HDMITX_RES_1280X1470p_50Hz	= 15,
    HDMITX_RES_1280X1470p_60Hz	= 16,
    HDMITX_RES_3840x2160p_24Hz  = 17,
    HDMITX_RES_3840x2160p_25Hz  = 18,
    HDMITX_RES_3840x2160p_30Hz  = 19,
    HDMITX_RES_3840x2160p_50Hz  = 20,
    HDMITX_RES_3840x2160p_60Hz  = 21,
    HDMITX_RES_4096x2160p_24Hz  = 22,
    HDMITX_RES_4096x2160p_25Hz  = 23,
    HDMITX_RES_4096x2160p_30Hz  = 24,
    HDMITX_RES_4096x2160p_50Hz  = 25,
    HDMITX_RES_4096x2160p_60Hz  = 26,
    HDMITX_RES_1600x1200p_60Hz  = 27,
    HDMITX_RES_1440x900p_60Hz   = 28,
    HDMITX_RES_1280x1024p_60Hz  = 29,
    HDMITX_RES_1024x768p_60Hz   = 30,
    HDMITX_RES_1280x720p_24Hz   = 31,
    HDMITX_RES_1280x720p_25Hz   = 32,
    HDMITX_RES_1280x720p_30Hz   = 33,
    HDMITX_RES_1366x768p_60Hz   = 34,
    HDMITX_RES_1280x800p_60Hz   = 35,
    HDMITX_RES_1680x1050p_60Hz  = 36,
    HDMITX_RES_MAX,
} HDMITX_VIDEO_TIMING;

typedef enum
{
    HDMITX_VIC_NOT_AVAILABLE        = 0,
    HDMITX_VIC_640x480p_60_4_3	    = 1,
    HDMITX_VIC_720x480p_60_4_3	    = 2,
    HDMITX_VIC_720x480p_60_16_9	    = 3,
    HDMITX_VIC_1280x720p_60_16_9	= 4,
    HDMITX_VIC_1920x1080i_60_16_9	= 5,
    HDMITX_VIC_720x480i_60_4_3	    = 6,
    HDMITX_VIC_720x480i_60_16_9     = 7,
    HDMITX_VIC_720x240p_60_4_3	    = 8,
    HDMITX_VIC_720x240p_60_16_9	    = 9,
    HDMITX_VIC_2880x480i_60_4_3	    = 10,
    HDMITX_VIC_2880x480i_60_16_9	= 11,
    HDMITX_VIC_2880x240p_60_4_3	    = 12,
    HDMITX_VIC_2880x240p_60_16_9	= 13,
    HDMITX_VIC_1440x480p_60_4_3     = 14,
    HDMITX_VIC_1440x480p_60_16_9	= 15,
    HDMITX_VIC_1920x1080p_60_16_9	= 16,
    HDMITX_VIC_720x576p_50_4_3	    = 17,
    HDMITX_VIC_720x576p_50_16_9	    = 18,
    HDMITX_VIC_1280x720p_50_16_9	= 19,
    HDMITX_VIC_1920x1080i_50_16_9	= 20,
    HDMITX_VIC_720x576i_50_4_3      = 21,
    HDMITX_VIC_720x576i_50_16_9	    = 22,
    HDMITX_VIC_720x288p_50_4_3	    = 23,
    HDMITX_VIC_720x288p_50_16_9	    = 24,
    HDMITX_VIC_2880x576i_50_4_3	    = 25,
    HDMITX_VIC_2880x576i_50_16_9	= 26,
    HDMITX_VIC_2880x288p_50_4_3	    = 27,
    HDMITX_VIC_2880x288p_50_16_9    = 28,
    HDMITX_VIC_1440x576p_50_4_3	    = 29,
    HDMITX_VIC_1440x576p_50_16_9    = 30,
    HDMITX_VIC_1920x1080p_50_16_9   = 31,
    HDMITX_VIC_1920x1080p_24_16_9	= 32,
    HDMITX_VIC_1920x1080p_25_16_9	= 33,
    HDMITX_VIC_1920x1080p_30_16_9	= 34,
    HDMITX_VIC_2880x480p_60_4_3     = 35,
    HDMITX_VIC_2880x480p_60_16_9	= 36,
    HDMITX_VIC_2880x576p_50_4_3	    = 37,
    HDMITX_VIC_2880x576p_50_16_9	= 38,
    HDMITX_VIC_1920x1080i_50_16_9_1250_total	= 39,
    HDMITX_VIC_1920x1080i_100_16_9  = 40,
    HDMITX_VIC_1280x720p_100_16_9	= 41,
    HDMITX_VIC_720x576p_100_4_3     = 42,
    HDMITX_VIC_720x576p_100_16_9	= 43,
    HDMITX_VIC_720x576i_100_4_3	    = 44,
    HDMITX_VIC_720x576i_100_16_9	= 45,
    HDMITX_VIC_1920x1080i_120_16_9  = 46,
    HDMITX_VIC_1280x720p_120_16_9	= 47,
    HDMITX_VIC_720x480p_120_4_3	    = 48,
    HDMITX_VIC_720x480p_120_16_9    = 49,
    HDMITX_VIC_720x480i_120_4_3	    = 50,
    HDMITX_VIC_720x480i_120_16_9	= 51,
    HDMITX_VIC_720x576p_200_4_3	    = 52,
    HDMITX_VIC_720x576p_200_16_9    = 53,
    HDMITX_VIC_720x576i_200_4_3	    = 54,
    HDMITX_VIC_720x576i_200_16_9	= 55,
    HDMITX_VIC_720x480p_240_4_3     = 56,
    HDMITX_VIC_720x480p_240_16_9	= 57,
    HDMITX_VIC_720x480i_240_4_3	    = 58,
    HDMITX_VIC_720x480i_240_16_9	= 59,
    HDMITX_VIC_1280x720p_24_16_9	= 60,
    HDMITX_VIC_1280x720p_25_16_9	= 61,
    HDMITX_VIC_1280x720p_30_16_9	= 62,
    HDMITX_VIC_1920x1080p_120_16_9  = 63,
    HDMITX_VIC_1920x1080p_100_16_9  = 64,
    //vvv------------------------------------------- HDMI 2.0 :: 21:9 aspect ratio
    HDMITX_VIC_1280x720p_24_21_9   = 65,
    HDMITX_VIC_1280x720p_25_21_9   = 66,
    HDMITX_VIC_1280x720p_30_21_9   = 67,
    HDMITX_VIC_1280x720p_50_21_9   = 68,
    HDMITX_VIC_1280x720p_60_21_9   = 69,
    HDMITX_VIC_1280x720p_100_21_9  = 70,
    HDMITX_VIC_1280x720p_120_21_9  = 71,
    HDMITX_VIC_1920x1080p_24_21_9  = 72,
    HDMITX_VIC_1920x1080p_25_21_9  = 73,
    HDMITX_VIC_1920x1080p_30_21_9  = 74,
    HDMITX_VIC_1920x1080p_50_21_9  = 75,
    HDMITX_VIC_1920x1080p_60_21_9  = 76,
    HDMITX_VIC_1920x1080p_100_21_9 = 77,
    HDMITX_VIC_1920x1080p_120_21_9 = 78,
    HDMITX_VIC_1680x720p_24_21_9   = 79,
    HDMITX_VIC_1680x720p_25_21_9   = 80,
    HDMITX_VIC_1680x720p_30_21_9   = 81,
    HDMITX_VIC_1680x720p_50_21_9   = 82,
    HDMITX_VIC_1680x720p_60_21_9   = 83,
    HDMITX_VIC_1680x720p_100_21_9  = 84,
    HDMITX_VIC_1680x720p_120_21_9  = 85,
    HDMITX_VIC_2560x1080p_24_21_9  = 86,
    HDMITX_VIC_2560x1080p_25_21_9  = 87,
    HDMITX_VIC_2560x1080p_30_21_9  = 88,
    HDMITX_VIC_2560x1080p_50_21_9  = 89,
    HDMITX_VIC_2560x1080p_60_21_9  = 90,
    HDMITX_VIC_2560x1080p_100_21_9 = 91,
    HDMITX_VIC_2560x1080p_120_21_9 = 92,
//^^^------------------------------------------- HDMI 2.0 :: 21:9 aspect ratio
    HDMITX_VIC_3840x2160p_24_16_9   = 93,
    HDMITX_VIC_3840x2160p_25_16_9   = 94,
    HDMITX_VIC_3840x2160p_30_16_9   = 95,
    HDMITX_VIC_3840x2160p_50_16_9   = 96,
    HDMITX_VIC_3840x2160p_60_16_9   = 97,
    HDMITX_VIC_4096x2160p_24_256_135   = 98,
    HDMITX_VIC_4096x2160p_25_256_135   = 99,
    HDMITX_VIC_4096x2160p_30_256_135   = 100,
    HDMITX_VIC_4096x2160p_50_256_135   = 101,
    HDMITX_VIC_4096x2160p_60_256_135   = 102,
//vvv------------------------------------------- HDMI 2.0 :: 21:9 aspect ratio
    HDMITX_VIC_3840x2160p_24_64_27  = 103,
    HDMITX_VIC_3840x2160p_25_64_27  = 104,
    HDMITX_VIC_3840x2160p_30_64_27  = 105,
    HDMITX_VIC_3840x2160p_50_64_27  = 106,
    HDMITX_VIC_3840x2160p_60_64_27  = 107,
//^^^------------------------------------------- HDMI 2.0 :: 21:9 aspect ratio
} HDMITX_AVI_VIC;

typedef enum
{
    HDMITX_VIDEO_AR_Reserved    = 0,
    HDMITX_VIDEO_AR_4_3         = 1,
    HDMITX_VIDEO_AR_16_9        = 2,
    HDMITX_VIDEO_AR_21_9        = 3,
} HDMITX_VIDEO_ASPECT_RATIO;

typedef enum
{
    HDMITX_VIDEO_SI_NoData    = 0,
    HDMITX_VIDEO_SI_Overscanned         = 1,
    HDMITX_VIDEO_SI_Underscanned        = 2,
    HDMITX_VIDEO_SI_Reserved    = 3,
} HDMITX_VIDEO_SCAN_INFO;

typedef enum
{
    HDMITX_VIDEO_AFD_SameAsPictureAR    = 8, // 1000
    HDMITX_VIDEO_AFD_4_3_Center         = 9, // 1001
    HDMITX_VIDEO_AFD_16_9_Center        = 10, // 1010
    HDMITX_VIDEO_AFD_14_9_Center        = 11, // 1011
    HDMITx_VIDEO_AFD_Others = 15, // 0000~ 0111, 1100 ~ 1111
} HDMITX_VIDEO_AFD_RATIO;


typedef enum
{
    HDMITX_VIDEO_VS_No_Addition    = 0, // 000
    HDMITX_VIDEO_VS_4k_2k          = 1, // 001
    HDMITX_VIDEO_VS_3D             = 2, // 010
    HDMITx_VIDEO_VS_Reserved       = 7, // 011~ 111
} HDMITX_VIDEO_VS_FORMAT;


typedef enum
{
    HDMITX_VIDEO_3D_FramePacking     = 0, // 0000
    HDMITX_VIDEO_3D_FieldAlternative = 1, // 0001
    HDMITX_VIDEO_3D_LineAlternative  = 2, // 0010
    HDMITX_VIDEO_3D_SidebySide_FULL  = 3, // 0011
    HDMITX_VIDEO_3D_L_Dep            = 4, // 0100
    HDMITX_VIDEO_3D_L_Dep_Graphic_Dep= 5, // 0101
    HDMITX_VIDEO_3D_TopandBottom     = 6, // 0110
    HDMITX_VIDEO_3D_SidebySide_Half  = 8, // 1000
    HDMITx_VIDEO_3D_Not_in_Use       = 15, // 1111
} HDMITX_VIDEO_3D_STRUCTURE;

typedef enum
{
    HDMITX_EDID_3D_FramePacking               = 1, // 3D_STRUCTURE_ALL_0
    HDMITX_EDID_3D_FieldAlternative           = 2, // 3D_STRUCTURE_ALL_1
    HDMITX_EDID_3D_LineAlternative            = 4, // 3D_STRUCTURE_ALL_2
    HDMITX_EDID_3D_SidebySide_FULL            = 8, // 3D_STRUCTURE_ALL_3
    HDMITX_EDID_3D_L_Dep                      = 16, // 3D_STRUCTURE_ALL_4
    HDMITX_EDID_3D_L_Dep_Graphic_Dep          = 32, // 3D_STRUCTURE_ALL_5
    HDMITX_EDID_3D_TopandBottom               = 64, // 3D_STRUCTURE_ALL_6
    HDMITX_EDID_3D_SidebySide_Half_horizontal = 256, // 3D_STRUCTURE_ALL_8
    HDMITX_EDID_3D_SidebySide_Half_quincunx   = 32768, // 3D_STRUCTURE_ALL_15
} HDMITX_EDID_3D_STRUCTURE_ALL;

typedef enum
{
    HDMITX_EDID_Color_RGB_444              = 1, // RGB 4:4:4
    HDMITX_EDID_Color_YCbCr_444            = 2, // YCbCr 4:4:4
    HDMITX_EDID_Color_YCbCr_422            = 4, // YCbCr 4:2:2
    HDMITX_EDID_Color_YCbCr_420            = 8, // YCbCr 4:2:0
} HDMITX_EDID_COLOR_FORMAT;

typedef enum
{
    HDMITx_VIDEO_4k2k_Reserved    = 0, // 0x00
    HDMITX_VIDEO_4k2k_30Hz        = 1, // 0x01
    HDMITX_VIDEO_4k2k_25Hz        = 2, // 0x02
    HDMITX_VIDEO_4k2k_24Hz        = 3, // 0x03
    HDMITx_VIDEO_4k2k_24Hz_SMPTE  = 4, // 0x04
} HDMITX_VIDEO_4k2k_VIC;


typedef enum
{
    HDMITX_AUDIO_FREQ_NO_SIG  = 0,
    HDMITX_AUDIO_32K          = 1,
    HDMITX_AUDIO_44K          = 2,
    HDMITX_AUDIO_48K          = 3,
    HDMITX_AUDIO_88K          = 4,
    HDMITX_AUDIO_96K          = 5,
    HDMITX_AUDIO_176K         = 6,
    HDMITX_AUDIO_192K         = 7,
    HDMITX_AUDIO_FREQ_MAX_NUM = 8,
} HDMITX_AUDIO_FREQUENCY;

typedef enum
{
    HDMITX_AUDIO_FORMAT_PCM   = 0,
    HDMITX_AUDIO_FORMAT_DSD   = 1,
    HDMITX_AUDIO_FORMAT_HBR   = 2,
    HDMITX_AUDIO_FORMAT_NA    = 3,
} HDMITX_AUDIO_SOURCE_FORMAT;

typedef enum
{
    HDMITX_AUDIO_CH_2  = 2, // 2 channels
    HDMITX_AUDIO_CH_8  = 8, // 8 channels
} HDMITX_AUDIO_CHANNEL_COUNT;

typedef enum
{
    HDMITX_AUDIO_PCM        = 0, // PCM
    HDMITX_AUDIO_NONPCM     = 1, // non-PCM
} HDMITX_AUDIO_CODING_TYPE;

typedef enum //C0, C1 field of AVIInfoFrame packet
{
    HDMITX_COLORIMETRY_NO_DATA      = 0,
    HDMITX_COLORIMETRY_SMPTE170M,
    HDMITX_COLORIMETRY_ITUR709,
    HDMITX_COLORIMETRY_EXTEND,
    HDMITX_COLORIMETRY_MAX
} HDMITX_AVI_COLORIMETRY;

typedef enum //EC0~EC2 filed of AVIInfoFrame packet
{
    HDMITX_EXT_COLORIMETRY_XVYCC601           = 0,
    HDMITX_EXT_COLORIMETRY_XVYCC709           = 1,
    HDMITX_EXT_COLORIMETRY_SYCC601            = 2,
    HDMITX_EXT_COLORIMETRY_ADOBEYCC601        = 3,
    HDMITX_EXT_COLORIMETRY_ADOBERGB           = 4,
    HDMITX_EXT_COLORIMETRY_BT2020CYCC         = 5, //mapping to ext. colorimetry format BT2020Y'cC'bcC'rc
    HDMITX_EXT_COLORIMETRY_BT2020YCC          = 6, //mapping to ext. colorimetry format BT2020 RGB or YCbCr
    HDMITX_EXT_COLORIMETRY_BT2020RGB          = 7  //mapping to ext. colorimetry format BT2020 RGB or YCbCr
} HDMITX_AVI_EXTENDED_COLORIMETRY;

typedef enum //YQ1, YQ2 field of AVIInfoFrame packet
{
    HDMITX_YCC_QUANT_LIMIT          = 0x00,
    HDMITX_YCC_QUANT_FULL           = 0x01,
    HDMITX_YCC_QUANT_RESERVED       = 0x10
} HDMITX_AVI_YCC_QUANT_RANGE;

//HDMITx Capability
typedef enum
{
    E_HDMITX_CAP_SUPPORT_DVI  = 0, ///< return true if H/W support scaler device1
    E_HDMITX_CAP_SUPPORT_HDCP = 1
}EN_HDMITX_CAPS;

typedef struct DLL_PACKED
{
    MS_U8 Reserved;
}HDMI_TX_INFO;

typedef struct DLL_PACKED
{
    MS_BOOL bIsInitialized;
    MS_BOOL bIsRunning;
}HDMI_TX_Status;

typedef struct DLL_PACKED
{
    // HDMI Tx Current, Pre-emphasis and Double termination
    MS_U8    tm_txcurrent; // TX current control(U4: 0x11302B[13:12], K1: 0x11302B[13:11])
    MS_U8    tm_pren2; // pre-emphasis mode control, 0x11302D[5]
    MS_U8    tm_precon; // TM_PRECON, 0x11302E[7:4]
    MS_U8    tm_pren; // pre-emphasis enable, 0x11302E[11:8]
    MS_U8    tm_tenpre; // Double termination pre-emphasis enable, 0x11302F[3:0]
    MS_U8    tm_ten; // Double termination enable, 0x11302F[7:4]
} HDMITX_ANALOG_TUNING;

typedef struct DLL_PACKED
{
    // HDMI Tx Current
    MS_U8    u8DrvCurTap1Ch0;
    MS_U8    u8DrvCurTap1Ch1;
    MS_U8    u8DrvCurTap1Ch2;
    MS_U8    u8DrvCurTap1Ch3;
    MS_U8    u8DrvCurTap2Ch0;
    MS_U8    u8DrvCurTap2Ch1;
    MS_U8    u8DrvCurTap2Ch2;
    MS_U8    u8DrvCurTap2Ch3;
} HDMITX_ANALOG_DRV_CUR_CONFIG;

typedef enum
{
    E_HDCP_DISABLE      = 0, // HDCP disable
    E_HDCP_FAIL = 1, // HDCP fail
    E_HDCP_PASS = 2, // HDCP pass
} HDMITX_HDCP_STATUS;

typedef enum
{
    E_HDMITX_FSM_STATUS_PENDING       = 0,
    E_HDMITX_FSM_STATUS_VALIDATE_EDID = 1,
    E_HDMITX_FSM_STATUS_TRANSMIT      = 2,
    E_HDMITX_FSM_STATUS_DONE          = 3,
}HDMITX_FSM_STATUS;

typedef enum
{
    CHECK_NOT_READY = 0,
    CHECK_REVOKED = 1,
    CHECK_NOT_REVOKED = 2,
}HDMITX_REVOCATION_STATE;

typedef enum
{
    HDMITX_INT_HDCP_DISABLE      = 0, // HDCP disable
    HDMITX_INT_HDCP_FAIL         = 1, // HDCP fail
    HDMITX_INT_HDCP_PASS         = 2, // HDCP pass
    HDMITX_INT_HDCP_PROCESS      = 3, // HDCP processing
} HDMITX_INT_HDCP_STATUS;

typedef enum
{
    E_UNHDCPRX_NORMAL_OUTPUT      = 0, // still display normally
    E_UNHDCPRX_HDCP_ENCRYPTION = 1, // HDCP encryption to show snow screen
    E_UNHDCPRX_BLUE_SCREEN = 2, // blue screen
} HDMITX_UNHDCPRX_CONTROL;

typedef enum
{
    E_HDCPRXFail_NORMAL_OUTPUT      = 0, // still display normally
    E_HDCPRXFail_HDCP_ENCRYPTION = 1, // HDCP encryption to show snow screen
    E_HDCPRXFail_BLUE_SCREEN = 2, // blue screen
} HDMITX_HDCPRXFail_CONTROL;

typedef enum
{
    HDMITX_INPUT_LESS_60MHZ  =0,
    HDMITX_INPUT_60_to_160MHZ  =1,
    HDMITX_INPUT_OVER_160MHZ  =2,
} HDMITX_INPUT_FREQ;

typedef enum
{
    HDMITX_HDCP_RESET                                = 0x01,
    HDMITX_HDCP_WAITING_ACTIVE_RX 	     = 0x02,
    HDMITX_HDCP_CHECK_REPEATER_READY = 0x03,
    HDMITX_HDCP_CHECK_R0 			     = 0x04,
    HDMITX_HDCP_AUTH_DONE 			     = 0x05,
    HDMITX_HDCP_AUTH_FAIL 			     = 0x06,

    // bit[7:6]=00 for checking valid rx
    HDMITX_HDCP_RX_IS_NOT_VALID		= 0x00, // 00 00
    HDMITX_HDCP_RX_IS_VALID         = 0x10, // 00 01
    HDMITX_HDCP_RX_KEY_FAIL			= 0x20, // 00 10
    HDMITX_HDCP_TX_KEY_FAIL         = 0x30, // 00 11
    HDMITX_HDCP_RX_KEY_REVOKED        = 0x0F, // 00 00 11 11

    // bit[7:6]=01 for repeater
    HDMITX_HDCP_REPEATER_TIMEOUT 	= 0x40, // 01 00
    HDMITX_HDCP_REPEATER_READY 		= 0x50, // 01 01
    HDMITX_HDCP_REPEATER_NOT_READY 	= 0x60, // 01 10
    HDMITX_HDCP_REPEATER_VALID 		= 0x70, // 01 11

    // bit[7:6]=10 for SHA1
    HDMITX_HDCP_REPEATER_SHA1_FAIL 	= 0x80, // 10 00
    HDMITX_HDCP_REPEATER_SHA1_PASS 	= 0x90, // 10 01

    // bit[7:6]=11 for Ri
    HDMITX_HDCP_SYNC_RI_FAIL 		    = 0xC0, // 11 00
    HDMITX_HDCP_SYNC_RI_PASS 		    = 0xD0  // 11 01
}HDMITX_HDCP_AUTH_STATUS;

typedef enum
{
    HDMITX_CMD_NONE = 0,
    E_HDMITX_COLOR_AND_RANGE_TRANSFORM,
    HDMITX_CMD_COLOR_AND_RANGE_TRANSFORM,
    HDMITX_CMD_SSC_ENABLE,
    E_HDMITX_NDS_SET_ENC_EN,
    HDMITX_CMD_NDS_SET_ENC_EN,
    E_HDMITX_NDS_GET_HDCP_STATUS,
    HDMITX_CMD_NDS_GET_HDCP_STATUS,
    HDMITX_CMD_SET_COLORIMETRY,
    HDMITX_CMD_GET_FULL_RX_STATUS,
    HDMITX_CMD_TIMING_CAPABILITY_CHECK,
    HDMITX_CMD_HDCP1XTX_CHK_RI,
    HDMITX_CMD_GET_PANELSIZE_FROM_EDID,
    HDMITX_CMD_GET_TMDS_STATUS,
    HDMITX_CMD_GET_COLOR_FORMAT,
    HDMITX_CMD_GET_COLOR_DEPTH,
    HDMITX_CMD_CHECK_EDID_QUANTIZATION_RANGE,
    HDMITX_CMD_NUMBER,
} HDMITX_CTRL_ID;

#define EN_HDMITX_CTRL_ID HDMITX_CTRL_ID

typedef enum //color range
{
    HDMITX_QUANT_LIMIT          = 0x00,
    HDMITX_QUANT_FULL           = 0x01,
    HDMITX_QUANT_RESERVED   = 0x10,
} HDMITX_QUANT_RANGE;

#define EN_HDMITX_QUANT_RANGE HDMITX_QUANT_RANGE

typedef struct DLL_PACKED
{
    HDMITX_VIDEO_COLOR_FORMAT         input_color;
    HDMITX_VIDEO_COLOR_FORMAT         output_color;
    HDMITX_QUANT_RANGE                      input_range;
    HDMITX_QUANT_RANGE                      output_range;
    MS_BOOL                                           result;
} HDMITX_COLOR_AND_RANGE_TRANSFORM_PARAMETERS;

typedef struct DLL_PACKED
{
    MS_U32                                            u32StructVersion;//StructVersion Control
    MS_U8                                             u8SSCEn;
    MS_BOOL                                           result;
} HDMITX_SSCENABLE;

typedef enum
{
    HDMITX_SSCENABLE_STRUCTVER_NONE= 0,
    HDMITX_SSCENABLE_STRUCTVER_1= 1,
    HDMITX_SSCENABLE_STRUCTVER_NUM
}HDMITX_SSCENABLE_STRUCTVER;

typedef struct DLL_PACKED
{
    MS_U32                      u32StructVersion;//StructVersion Control
    HDMITX_AVI_COLORIMETRY      colorimetry;
    MS_U8                       u8Return;
} HDMITX_SET_COLORIMETRY;

typedef enum
{
    HDMITX_SET_COLORIMETRY_STRUCTVER_NONE= 0,
    HDMITX_SET_COLORIMETRY_STRUCTVER_1= 1,
    HDMITX_SET_COLORIMETRY_STRUCTVER_NUM
}HDMITX_SET_COLORIMETRY_STRUCTVER;

typedef enum
{
    HDMITX_DVIClock_L_HPD_L     = 0,
    HDMITX_DVIClock_L_HPD_H     = 1,
    HDMITX_DVIClock_H_HPD_L     = 2,
    HDMITX_DVIClock_H_HPD_H     = 3,
} HDMITX_RX_STATUS;

typedef struct DLL_PACKED
{
    MS_U32  u32StructVersion;//StructVersion Control
    MS_U32  u32RxStatus;
} HDMITX_GET_FULL_RX_STATUS;

typedef enum
{
    HDMITX_GET_FULL_RX_STATUS_STRUCTVER_NONE= 0,
    HDMITX_GET_FULL_RX_STATUS_STRUCTVER_1= 1,
    HDMITX_GET_FULL_RX_STATUS_STRUCTVER_NUM
}HDMITX_GET_FULL_RX_STATUS_STRUCTVER;

typedef struct DLL_PACKED
{
    MS_BOOL bReturn;
} HDMITx_HDCP1X_COMPARE_RI;

typedef enum
{
    HDMITX_TIMING_ERR_NONE           = 0x00000000,
    HDMITX_TIMING_ERR_CFG_ERR        = 0x00000001,
    HDMITX_TIMING_ERR_EDID_ERR       = 0x00000002,
    HDMITX_TIMING_ERR_COLOR_FMT      = 0x00000004,
    HDMITX_TIMING_ERR_COLOR_DEPTH    = 0x00000008,
    HDMITX_TIMING_ERR_TIMING         = 0x00000010,
    HDMITX_TIMING_ERR_HW_LIMIT       = 0x00000020,
    HDMITX_TIMING_ERR_SW_LIMIT       = 0x00000040,
    HDMITX_TIMING_ERR_SINK_LIMIT     = 0x00000080,
    HDMITX_TIMING_ERR_MAX            = 0xFFFFFFFF
}HDMITX_TIMING_ERROR;

typedef struct DLL_PACKED
{
    MS_U32  u32StructVersion;//StructVersion Control
    HDMITX_OUTPUT_MODE eOutputMode;
    HDMITX_VIDEO_TIMING eTiming;
    HDMITX_VIDEO_COLOR_FORMAT eInColor;
    HDMITX_VIDEO_COLOR_FORMAT eOutColor;
    HDMITX_VIDEO_COLORDEPTH_VAL eColorDepth;
    HDMITX_TIMING_ERROR  ubRet;
} HDMITX_CHECK_LEGAL_TIMING;

typedef enum
{
    HDMITX_CHECK_LEGAL_TIMING_STRUCTVER_NONE= 0,
    HDMITX_CHECK_LEGAL_TIMING_STRUCTVER_1= 1,
    HDMITX_CHECK_LEGAL_TIMING_STRUCTVER_NUM
}HDMITX_CHECK_LEGAL_TIMING_STRUCTVER;

typedef struct DLL_PACKED
{
    MS_U32  u32StructVersion;//StructVersion Control
    MS_U32  u32PanelWidth;
    MS_U32  u32PanelHeight;
    MS_U32  u32Ret;
} HDMITX_GET_PANELSIZE_FROM_EDID;

typedef enum
{
    HDMITX_GET_PANELSIZE_FROM_EDID_STRUCTVER_NONE= 0,
    HDMITX_GET_PANELSIZE_FROM_EDID_STRUCTVER_1= 1,
    HDMITX_GET_PANELSIZE_FROM_EDID_STRUCTVER_NUM
}HDMITX_GET_PANELSIZE_FROM_EDID_STRUCTVER;

typedef struct DLL_PACKED
{
    MS_U32  u32StructVersion;//StructVersion Control
    MS_U32  u32TMDSStatus;
    MS_U32  u32Ret;
} HDMITX_GET_TMDS_STATUS;

typedef enum
{
    HDMITX_GET_TMDS_STATUS_STRUCTVER_NONE= 0,
    HDMITX_GET_TMDS_STATUS_STRUCTVER_1= 1,
    HDMITX_GET_TMDS_STATUS_STRUCTVER_NUM
}HDMITX_GET_TMDS_STATUS_STRUCTVER;

//If user wants to get color format and depth in period of OS initializing
//Please get color format and depth with following condition
//1. Between MApi_HDMITx_DisableRegWrite(TRUE) and MApi_HDMITx_DisableRegWrite(FALSE)
//2. Before Exhibit
typedef struct DLL_PACKED
{
    MS_U32  u32StructVersion;//StructVersion Control
    HDMITX_VIDEO_COLOR_FORMAT  enColorFmt;
    MS_U32  u32Ret;
} HDMITX_GET_COLOR_FMT;

typedef enum
{
    HDMITX_GET_COLOR_FMT_STRUCTVER_NONE= 0,
    HDMITX_GET_COLOR_FMT_STRUCTVER_1= 1,
    HDMITX_GET_COLOR_FMT_STRUCTVER_NUM
}HDMITX_GET_COLOR_FMT_STRUCTVER;

//If user wants to get color format and depth in period of OS initializing
//Please get color format and depth with following condition
//1. Between MApi_HDMITx_DisableRegWrite(TRUE) and MApi_HDMITx_DisableRegWrite(FALSE)
//2. Before Exhibit
typedef struct DLL_PACKED
{
    MS_U32  u32StructVersion;//StructVersion Control
    HDMITX_VIDEO_COLORDEPTH_VAL  enColorDepth;
    MS_U32  u32Ret;
} HDMITX_GET_COLOR_DEPTH;

typedef enum
{
    HDMITX_GET_COLOR_DEPTH_STRUCTVER_NONE= 0,
    HDMITX_GET_COLOR_DEPTH_STRUCTVER_1= 1,
    HDMITX_GET_COLOR_DEPTH_STRUCTVER_NUM
}HDMITX_GET_COLOR_DEPTH_STRUCTVER;

typedef enum //check EDID support color range
{
    HDMITX_EDID_QUANT_LIMIT  = 0x00,
    HDMITX_EDID_QUANT_FULL   = 0x01,
    HDMITX_EDID_QUANT_BOTH   = 0x10,
} HDMITX_EDID_QUANT_RANGE;

typedef struct DLL_PACKED
{
    MS_U32                      u32StructVersion;//StructVersion Control
    HDMITX_VIDEO_TIMING         eTiming;
    HDMITX_VIDEO_COLOR_FORMAT   eOutColor;
    HDMITX_EDID_QUANT_RANGE     u8Return;
} HDMITX_CHECK_EDID_QUAN_RANGE;

typedef enum
{
    HDMITX_CHECK_EDID_QUAN_RANGE_STRUCTVER_NONE= 0,
    HDMITX_CHECK_EDID_QUAN_RANGE_STRUCTVER_1= 1,
    HDMITX_CHECK_EDID_QUAN_RANGE_STRUCTVER_NUM
}HDMITX_CHECK_EDID_QUAN_RANGE_STRUCTVER;

typedef enum
{
    HDMITX_TMDS_OFF = 0,
    HDMITX_TMDS_FULL_ON = 1,
    HDMITX_TMDS_NUM
} HDMITx_TMDS_STATUS;

typedef enum
{
    E_CEA_EXT_TAG_VCDB = 0,         //video capability data block
    E_CEA_EXT_TAG_VSVDB = 1,        //vendor-specific video data block
    E_CEA_EXT_TAG_VDDDB = 2,        //VESA display device data block
    E_CEA_EXT_TAG_VVTBE = 3,        //VESA video timing block extension
    E_CEA_EXT_TAG_CDB = 5,          //colorimetry data block
    E_CEA_EXT_TAG_HSMDB = 6,             //HDR static meta data block
    E_CEA_EXT_TAG_420VDB = 14,      //YCbCr420 video data block
    E_CEA_EXT_TAG_420CMDB = 15,     //YCbCr420 cpability map data block
    E_CEA_EXT_TAG_VSADB = 17,       //vendor-specific audio data block
    E_CEA_EXT_TAG_IFDB = 32         //infoframe data block
} HDMITX_CEA_EXT_TAG_CODE;

typedef enum
{
    E_CEA_TAG_CODE_AUDIO = 1,
    E_CEA_TAG_CODE_VIDEO = 2,
    E_CEA_TAG_CODE_VSDB = 3,
    E_CEA_TAG_CODE_SPEAKER_ALLOCAT = 4,
    E_CEA_TAG_CODE_VDTC = 5, //VESA display transfer characteristic data block
    E_CEA_TAG_CODE_EXT_TAG = 7,
    E_CEA_TAG_CODE_HFVSDB = 8 //new block in hdmi 20
} HDMITX_CEA_DB_TAG_CODE;

enum HDMITX_HDCP1_X74OffSET_ADDR//typedef enum
{
    E_HDCP1_OFFSETADDR_Bksv = (MS_U8)0x00,
    E_HDCP1_OFFSETADDR_RiPrime = (MS_U8)0x08,
    E_HDCP1_OFFSETADDR_PjPrime = (MS_U8)0x0A,
    E_HDCP1_OFFSETADDR_Aksv = (MS_U8)0x10,
    E_HDCP1_OFFSETADDR_Ainfo = (MS_U8)0x15,
    E_HDCP1_OFFSETADDR_An = (MS_U8)0x18,
    E_HDCP1_OFFSETADDR_VPrime = (MS_U8)0x20,
    E_HDCP1_OFFSETADDR_VPrimeH1 = (MS_U8)0x24,
    E_HDCP1_OFFSETADDR_VPrimeH2 = (MS_U8)0x28,
    E_HDCP1_OFFSETADDR_VPrimeH3 = (MS_U8)0x2C,
    E_HDCP1_OFFSETADDR_VPrimeH4 = (MS_U8)0x30,
    E_HDCP1_OFFSETADDR_BCaps = (MS_U8)0x40,
    E_HDCP1_OFFSETADDR_BStatus = (MS_U8)0x41,
    E_HDCP1_OFFSETADDR_KsvFifo = (MS_U8)0x43,
    E_HDCP1_OFFSETADDR_DBG = (MS_U8)0xC0
}; //HDMITX_HDCP1x_X74OffSET_ADDR;

/*********************************************************************/
/*                                                                                                                     */
/*                                         HDCP22 Relative                                                    */
/*                                                                                                                     */
/*********************************************************************/
enum HDMITX_HDCP2_X74OffSET_ADDR//typedef enum
{
    E_HDCP2_OFFSETADDR_HDCP2Version = (MS_U8)0x50,
    E_HDCP2_OFFSETADDR_WriteMessage = (MS_U8)0x60,
    E_HDCP2_OFFSETADDR_RxStatus = (MS_U8)0x70,
    E_HDCP2_OFFSETADDR_ReadMessage = (MS_U8)0x80,
    E_HDCP2_OFFSETADDR_DBG = (MS_U8)0xC0,
}; //HDMITX_HDCP2_X74OffSET_ADDR;

enum HDMITX_HDCP2_OPCODE//typedef enum
{
    E_HDCP2_OPCODE_WRITE = (MS_U8)0x00,
    E_HDCP2_OPCODE_READ = (MS_U8)0x01,
}; //HDMITX_HDCP2_OPCODE;



typedef enum
{
    E_HDMITX_POWER_SUSPEND     = 1,    // State is backup in DRAM, components power off.
    E_HDMITX_POWER_RESUME      = 2,    // Resume from Suspend or Hibernate mode
    E_HDMITX_POWER_MECHANICAL  = 3,    // Full power off mode. System return to working state only after full reboot
    E_HDMITX_POWER_SOFT_OFF    = 4,    // The system appears to be off, but some components remain powered for trigging boot-up.
} HDMITX_POWER_MODE;


//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------


//*********************//
//        DVI / HDMI           //
//*********************//

MS_BOOL  MApi_HDMITx_Init(void);

MS_BOOL  MApi_HDMITx_Exit(void);

// HDMI Tx module On/Off
/*
    Before turn on HDMI TX module, video and audio source should be prepared ready and set the following APIs first.
        {
            ...
            MApi_HDMITx_TurnOnOff(TRUE);
            MApi_HDMITx_SetRBChannelSwap(TRUE);
            MApi_HDMITx_SetColorFormat(HDMITX_VIDEO_COLOR_YUV444, HDMITX_VIDEO_COLOR_RGB444);
            MApi_HDMITx_SetVideoOnOff(TRUE);
            MApi_HDMITx_SetHDMITxMode_CD(HDMITX_HDMI, HDMITX_VIDEO_CD_24Bits);
            MApi_HDMITx_SetVideoOutputTiming(HDMITX_RES_1920x1080p_60Hz);
            MApi_HDMITx_Exhibit();
            ...
        }

*/
void  MApi_HDMITx_TurnOnOff(MS_BOOL state);

// HDMI packet enable or not
void  MApi_HDMITx_EnablePacketGen(MS_BOOL bflag);

// HDMI Tx output is DVI / HDMI mode
void  MApi_HDMITx_SetHDMITxMode(HDMITX_OUTPUT_MODE mode);

// HDMI Tx output is DVI / HDMI mode and color depth
void  MApi_HDMITx_SetHDMITxMode_CD(HDMITX_OUTPUT_MODE mode, HDMITX_VIDEO_COLORDEPTH_VAL val);

// HDMI Tx TMDS signal On/Off
void  MApi_HDMITx_SetTMDSOnOff(MS_BOOL state);

// HDMI Tx TMDS control disable/enable
void  MApi_HDMITx_DisableTMDSCtrl(MS_BOOL bFlag);

// HDMI Tx AvMute control disable/enable
void MApi_HDMITx_DisableAvMuteCtrl(MS_BOOL bFlag);

// HDMI Tx R/B channel swap
void  MApi_HDMITx_SetRBChannelSwap(MS_BOOL state);

// HDMI Tx Exhibit funtcion
void  MApi_HDMITx_Exhibit(void);

// HDMI Tx force output mode
void  MApi_HDMITx_ForceHDMIOutputMode(MS_BOOL bflag, HDMITX_OUTPUT_MODE output_mode);

// HDMI Tx force output color format
MS_BOOL  MApi_HDMITx_ForceHDMIOutputColorFormat(MS_BOOL bflag, HDMITX_VIDEO_COLOR_FORMAT output_color);

// Get the connected HDMI Rx status
MS_BOOL  MApi_HDMITx_GetRxStatus(void);

// Get Rx's deep color definition from EDID
MS_BOOL  MApi_HDMITx_GetRxDCInfoFromEDID(HDMITX_VIDEO_COLORDEPTH_VAL *val);

// Get Rx's support video format from EDID
MS_BOOL  MApi_HDMITx_GetRxVideoFormatFromEDID(MS_U8 *pu8Buffer, MS_U8 u8BufSize);

// Get vic list from EDID
MS_BOOL  MApi_HDMITx_GetVICListFromEDID(MS_U8 *pu8Buffer, MS_U8 u8BufSize);

// Get Rx's data block length
MS_BOOL  MApi_HDMITx_GetDataBlockLengthFromEDID(MS_U8 *pu8Length, MS_U8 u8TagCode);

// Get Rx's support audio format from EDID
MS_BOOL  MApi_HDMITx_GetRxAudioFormatFromEDID(MS_U8 *pu8Buffer, MS_U8 u8BufSize);

// Get Rx's support mode from EDID
MS_BOOL  MApi_HDMITx_EDID_HDMISupport(MS_BOOL *HDMI_Support);

// Get Rx's ID Manufacturer Name from EDID
MS_BOOL  MApi_HDMITx_GetRxIDManufacturerName(MS_U8 *pu8Buffer);


MS_BOOL  MApi_HDMITx_GetBksv(MS_U8 *pdata);

MS_BOOL  MApi_HDMITx_GetAksv(MS_U8 *pdata);


// Get Rx's EDID data
MS_BOOL  MApi_HDMITx_GetEDIDData(MS_U8 *pu8Buffer, MS_BOOL BlockIdx);

// Get  Rx's supported 3D structures of specific timing from EDID
MS_BOOL  MApi_HDMITx_GetRx3DStructureFromEDID(HDMITX_VIDEO_TIMING timing, HDMITX_EDID_3D_STRUCTURE_ALL *p3DStructure);

// Get color format from EDID
MS_BOOL  MApi_HDMITx_GetColorFormatFromEDID(HDMITX_VIDEO_TIMING timing, HDMITX_EDID_COLOR_FORMAT *pColorFmt);

// This function clear settings of user defined packet
void  MApi_HDMITx_PKT_User_Define_Clear(void);

// This function set user defined hdmi packet
void  MApi_HDMITx_PKT_User_Define(HDMITX_PACKET_TYPE packet_type, MS_BOOL def_flag,
HDMITX_PACKET_PROCESS def_process, MS_U8 def_fcnt);

// This function let user define hdmi packet content
MS_BOOL  MApi_HDMITx_PKT_Content_Define(HDMITX_PACKET_TYPE packet_type, MS_U8 *data, MS_U8 length);


//*********************//
//             Video                //
//*********************//

// HDMI Tx video output On/Off
void  MApi_HDMITx_SetVideoOnOff(MS_BOOL state);
// HDMI Tx video color format
void  MApi_HDMITx_SetColorFormat(HDMITX_VIDEO_COLOR_FORMAT in_color, HDMITX_VIDEO_COLOR_FORMAT out_color);
// HDMI Tx video output timing
void  MApi_HDMITx_SetVideoOutputTiming(HDMITX_VIDEO_TIMING mode);
// HDMI Tx video output aspect ratio
void  MApi_HDMITx_SetVideoOutputAsepctRatio(HDMITX_VIDEO_ASPECT_RATIO out_ar);
// HDMI Tx video output Overscan and AFD ratio
void  MApi_HDMITx_SetVideoOutputOverscan_AFD(MS_BOOL bflag, HDMITX_VIDEO_SCAN_INFO out_scaninfo, MS_U8 out_afd);
void  MApi_HDMITx_SetVideoOutputOverscan_AFD_II(MS_BOOL bflag, HDMITX_VIDEO_SCAN_INFO out_scaninfo, MS_U8 out_afd, MS_U8 A0 );
void  MApi_HDMITx_Set_VS_InfoFrame(HDMITX_VIDEO_VS_FORMAT vs_format, HDMITX_VIDEO_3D_STRUCTURE vs_3d, HDMITX_VIDEO_4k2k_VIC vs_vic);

MS_BOOL MApi_HDMITx_SSC_Enable(MS_U8 ubSSCEn);
MS_BOOL  MApi_HDMITx_SetAVIInfoExtColorimetry(HDMITX_AVI_EXTENDED_COLORIMETRY enExtColorimetry, HDMITX_AVI_YCC_QUANT_RANGE enYccQuantRange);


//*********************//
//             Audio                //
//*********************//

// HDMI Tx audio output On/Off
void  MApi_HDMITx_SetAudioOnOff(MS_BOOL state);
// HDMI Tx audio output sampling frequency
// For Uranus
void  MApi_HDMITx_SetAudioFrequency(HDMITX_AUDIO_FREQUENCY freq);
// HDMI Tx Module audio output: sampling frequency, channel count and coding type
// For Oberon
void  MApi_HDMITx_SetAudioConfiguration(HDMITX_AUDIO_FREQUENCY freq, HDMITX_AUDIO_CHANNEL_COUNT ch, HDMITX_AUDIO_CODING_TYPE type);
// HDMI Tx get audio CTS value.
MS_U32  MApi_HDMITx_GetAudioCTS(void);
// HDMI Tx mute/unmute audio FIFO.
void  MApi_HDMITx_MuteAudioFIFO(MS_BOOL bflag);
// Set HDMI audio source format
void  MApi_HDMITx_SetAudioSourceFormat(HDMITX_AUDIO_SOURCE_FORMAT fmt);

//void MApi_HDMITx_SetAudioFrequencyFromMad(void);
//*********************//
//             HDCP                //
//*********************//

// HDMI Tx Get HDCP key (set internal/external HDCP key)
// @param[in] useinternalkey: TRUE -> from internal, FALSE -> from external, like SPI flash
void  MApi_HDMITx_GetHdcpKey(MS_BOOL useinternalkey, MS_U8 *data);
// HDMI Tx HDCP encryption On/Off
void  MApi_HDMITx_SetHDCPOnOff(MS_BOOL state);
// This routine set HDMI Tx AVMUTE
void  MApi_HDMITx_SetAVMUTE(MS_BOOL bflag);
// This routine get HDMI Tx AVMUTE status
MS_BOOL  MApi_HDMITx_GetAVMUTEStatus(void);
// Get HDMITx FSM Status
HDMITX_FSM_STATUS MApi_HDMITx_GetFsmStatus(void);
// HDMI Tx HDCP status
HDMITX_HDCP_STATUS  MApi_HDMITx_GetHDCPStatus(void);
// HDCP start Authentication
void  MApi_HDMITx_HDCP_StartAuth(MS_BOOL bFlag);
// HDMI Tx Internal HDCP status
HDMITX_INT_HDCP_STATUS  MApi_HDMITx_GetINTHDCPStatus(void);
// HDMI Tx HDCP pre-status
HDMITX_INT_HDCP_STATUS  MApi_HDMITx_GetHDCP_PreStatus(void);
// HDMI video output or blank or encryption while connected with unsupport HDCP Rx
void  MApi_HDMITx_UnHDCPRxControl(HDMITX_UNHDCPRX_CONTROL state);
// HDMI video output or blank or encryption while HDCP authentication fail
void  MApi_HDMITx_HDCPRxFailControl(HDMITX_HDCPRXFail_CONTROL state);
// This routine to set the time interval from sent aksv to R0.
MS_BOOL  MApi_HDMITx_SetAksv2R0Interval(MS_U32 u32Interval);
// This API to get active Rx status.
MS_BOOL  MApi_HDMITx_IsHDCPRxValid(void);
// This API return revocation check state
HDMITX_REVOCATION_STATE  MApi_HDMITx_HDCP_RevocationKey_Check(void);
// This API will update revocation list (note : size 1 = 5 bytes !!!)
void  MApi_HDMITx_HDCP_RevocationKey_List(MS_U8 *data, MS_U16 size);


// Debug
//MS_BOOL  MApi_HDMITx_GetLibVer(const MSIF_Version **ppVersion);

MS_BOOL  MApi_HDMITx_GetInfo(HDMI_TX_INFO *pInfo);

MS_BOOL  MApi_HDMITx_GetStatus(HDMI_TX_Status *pStatus);

MS_BOOL  MApi_HDMITx_HDCP_IsSRMSignatureValid(MS_U8 *data, MS_U32 size);

/**
* @brief set debug mask
* @param[in] u16DbgSwitch DEBUG MASK,
*   0x01: Debug HDMITX, 0x02: Debug HDCP
*/
MS_BOOL  MApi_HDMITx_SetDbgLevel(MS_U16 u16DbgSwitch);

void  MApi_HDMITx_SetHPDGpioPin(MS_U8 u8pin);

// Adjust HDMITx analog setting for HDMI test or compliant issue
void  MApi_HDMITx_AnalogTuning(HDMITX_ANALOG_TUNING *pInfo);

void MApi_HDMITX_SetAnalogDrvCur(HDMITX_ANALOG_DRV_CUR_CONFIG *pDrcCurCfg);

void  MApi_HDMITx_DisableRegWrite(MS_BOOL bFlag);

//*********************//
//             CEC                 //
//*********************//

/// This routine get EDID physical address
void  MApi_HDMITx_GetEDIDPhyAdr(MS_U8 *pdata);
// This routine turn on/off HDMI Tx CEC
void  MApi_HDMITx_SetCECOnOff(MS_BOOL bflag);
// This routine get HDMI Tx CEC On/Off status
MS_BOOL  MApi_HDMITx_GetCECStatus(void);
// This routine force get EDID from reciver
MS_BOOL  MApi_HDMITx_EdidChecking(void);

//*********************//
//      RxBypassMode         //
//*********************//
MS_BOOL  MApi_HDMITx_RxBypass_Mode(HDMITX_INPUT_FREQ freq, MS_BOOL bflag);

MS_BOOL  MApi_HDMITx_Disable_RxBypass(void);


//*************************//
//      CHIP Capaibility   //
//*************************//
MS_BOOL  MApi_HDMITx_GetChipCaps(EN_HDMITX_CAPS eCapType, MS_U32* pRet, MS_U32 ret_size);

MS_U32  MApi_HDMITx_SetPowerState(HDMITX_POWER_MODE u16PowerState);



MS_BOOL  MApi_HDMITx_GetEdidDataBlocks(HDMITX_CEA_DB_TAG_CODE enTagCode, HDMITX_CEA_EXT_TAG_CODE enExtTagCode, MS_U8* pu8Data, MS_U32 u32DataLen, MS_U32* pu32RealLen);

MS_BOOL  MApi_HDMITx_GetKSVList(MS_U8 *pu8Bstatus, MS_U8* pu8KSVList, MS_U16 u16BufLen, MS_U16 *pu16KSVLength);

MS_BOOL  MApi_HDMITx_GeneralCtrl(MS_U32 u32Cmd, void* pu8Buf, MS_U32 u32BufSize);
/*********************************************************************/
/*                                                                                                                     */
/*                                         HDCP22 Relative                                                    */
/*                                                                                                                     */
/*********************************************************************/
MS_BOOL  MApi_HDMITx_HDCP2AccessX74(MS_U8 u8PortIdx, MS_U8 u8OffsetAddr, MS_U8 u8OpCode, MS_U8 *pu8RdBuf, MS_U16 u16RdLen, MS_U8 *pu8WRBuff, MS_U16 u16WrLen);
void  MApi_HDMITx_HDCP2TxInit(MS_U8 u8PortIdx, MS_BOOL bEnable);
void  MApi_HDMITx_HDCP2TxEnableEncrypt(MS_U8 u8PortIdx, MS_BOOL bEnable);
void  MApi_HDMITx_HDCP2TxFillCipherKey(MS_U8 u8PortIdx, MS_U8 *pu8Riv, MS_U8 *pu8KsXORLC128);

#ifdef __cplusplus
}
#endif


#endif // _API_HDMITX_H_

