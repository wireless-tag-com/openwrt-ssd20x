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


/**
 * \defgroup HAL_HDMITX_group  HAL_HDMITX driver
 * @{
 */
#ifndef __MHAL_HDMITX_DATATYPE_H__
#define __MHAL_HDMITX_DATATYPE_H__

//-------------------------------------------------------------------------------------------------
//  enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_MHAL_HDMITX_RET_SUCCESS    = 0,
    E_MHAL_HDMITX_RET_CFGERR     = 1,
    E_MHAL_HDMITX_RET_NOTSUPPORT = 2,
    E_MHAL_HDMITX_RET_NONEED     = 3,
    E_MHAL_HDMITX_RET_HPDERR     = 4,
    E_MHAL_HDMITX_RET_ERR        = 5,
}MhalHdmitxRet_e;


typedef enum
{
    E_MHAL_HDMITX_COLOR_RGB444 = 0x01,
    E_MHAL_HDMITX_COLOR_YUV444 = 0x02,
    E_MHAL_HDMITX_COLOR_YUV422 = 0x04,
    E_MHAL_HDMITX_COLOR_YUV420 = 0x08,
    E_MHAL_HDMITX_COLOR_AUTO  =  0x10,
}MhalHdmitxColorType_e;


typedef enum
{
    E_MHAL_HDMITX_OUTPUT_MODE_DVI       = 0,
    E_MHAL_HDMITX_OUTPUT_MODE_DVI_HDCP  = 1,
    E_MHAL_HDMITX_OUTPUT_MODE_HDMI      = 2,
    E_MHAL_HDMITX_OUTPUT_MODE_HDMI_HDCP = 3,
}MhalHdmitxOutpuModeType_e;


typedef enum
{
    E_MHAL_HDMITX_CD_NO_ID   = 0,
    E_MHAL_HDMITX_CD_24_BITS = 1,
    E_MHAL_HDMITX_CD_30_BITS = 2,
    E_MHAL_HDMITX_CD_36_BITS = 3,
    E_MHAL_HDMITX_CD_48_BITS = 4,
}MhalHdmitxColorDepthType_e;


typedef enum
{
    E_MHAL_HDMITX_RES_720X480P_60HZ   = 0,
    E_MHAL_HDMITX_RES_720X576P_50HZ   = 1,
    E_MHAL_HDMITX_RES_1280X720P_60HZ  = 2,
    E_MHAL_HDMITX_RES_1280X720P_50HZ  = 3,
    E_MHAL_HDMITX_RES_1920X1080P_24HZ = 4,
    E_MHAL_HDMITX_RES_1920X1080P_25HZ = 5,
    E_MHAL_HDMITX_RES_1920X1080P_30HZ = 6,
    E_MHAL_HDMITX_RES_1920X1080P_50HZ = 7,
    E_MHAL_HDMITX_RES_1920X1080P_60HZ = 8,
    E_MHAL_HDMITX_RES_3840X2160P_24HZ = 9,
    E_MHAL_HDMITX_RES_3840X2160P_25HZ = 10,
    E_MHAL_HDMITX_RES_3840X2160P_30HZ = 11,
    E_MHAL_HDMITX_RES_3840X2160P_50HZ = 12,
    E_MHAL_HDMITX_RES_3840X2160P_60HZ = 13,
    E_MHAL_HDMITX_RES_1024X768P_60HZ  = 14,
    E_MHAL_HDMITX_RES_1366X768P_60HZ  = 15,
    E_MHAL_HDMITX_RES_1440X900P_60HZ  = 16,
    E_MHAL_HDMITX_RES_1280X800P_60HZ  = 17,
    E_MHAL_HDMITX_RES_1280X1024P_60HZ = 18,
    E_MHAL_HDMITX_RES_1680X1050P_60HZ = 19,
    E_MHAL_HDMITX_RES_1600X1200P_60HZ = 20,
    E_MHAL_HDMITX_RES_MAX             = 21,
}MhaHdmitxTimingResType_e;


typedef enum
{
    E_MHAL_HDMITX_AUDIO_FREQ_NO_SIG = 0,
    E_MHAL_HDMITX_AUDIO_FREQ_32K    = 1,
    E_MHAL_HDMITX_AUDIO_FREQ_44K    = 2,
    E_MHAL_HDMITX_AUDIO_FREQ_48K    = 3,
    E_MHAL_HDMITX_AUDIO_FREQ_88K    = 4,
    E_MHAL_HDMITX_AUDIO_FREQ_96K    = 5,
    E_MHAL_HDMITX_AUDIO_FREQ_176K   = 6,
    E_MHAL_HDMITX_AUDIO_FREQ_192K   = 7,
    E_MHAL_HDMITX_AUDIO_FREQ_NUM    = 8,
}MhalHdmitxAudioFreqType_e;


typedef enum
{
    E_MHAL_HDMITX_AUDIO_CH_2,
    E_MHAL_HDMITX_AUDIO_CH_8,
}MhalHdmitxAudioChannelType_e;

typedef enum
{
    E_MHAL_HDMITX_AUDIO_CODING_PCM,
    E_MHAL_HDMITX_AUDIO_CODING_NONPCM,
}MhalHdmitxAudioCodingType_e;

typedef enum
{
    E_MHAL_HDMITX_AUDIO_FORMAT_PCM,
    E_MHAL_HDMITX_AUDIO_FORMAT_DSD,
    E_MHAL_HDMITX_AUDIO_FORMAT_HBR,
    E_MHAL_HDMITX_AUDIO_FORMAT_NA,
}MhalHdmitxAudioSourceFormat_e;

typedef enum
{
    E_MHAL_HDMITX_MUTE_NONE   = 0x00,
    E_MHAL_HDMITX_MUTE_VIDEO  = 0x01,
    E_MHAL_HDMITX_MUTE_AUDIO  = 0x02,
    E_MHAL_HDMITX_MUTE_AVMUTE = 0x04,
}MhalHdmitxMuteType_e;

typedef enum
{
    E_MHAL_HDMITX_SINK_INFO_EDID_DATA    = 0,
    E_MHAL_HDMITX_SINK_INFO_HDMI_SUPPORT,
    E_MHAL_HDMITX_SINK_INFO_COLOR_FORMAT,
    E_MHAL_HDMITX_SINK_INFO_HPD_STATUS,
    E_MHAL_HDMITX_SINK_INFO_NUM,
}MhalHdmitxSinkInfoType_e;

typedef enum
{
    E_MHAL_HDMITX_INFOFRAM_TYPE_AVI   = 0,
    E_MHAL_HDMITX_INFOFRAM_TYPE_SPD   = 1,
    E_MHAL_HDMITX_INFOFRAM_TYPE_AUDIO = 2,
    E_MHAL_HDMITX_INFOFRAM_TYPE_MAX   = 3,
}MhalHdmitxInfoFrameType_e;

typedef enum
{
    E_MHAL_HDMITX_COLORIMETRY_NO_DATA = 0,
    E_MHAL_HDMITX_COLORIMETRY_SMPTE170M,
    E_MHAL_HDMITX_COLORIMETRY_ITUR709,
    E_MHAL_HDMITX_COLORIMETRY_EXTEND,
    E_MHAL_HDMITX_COLORIMETRY_MAX,
}MhalHdmitxColorimetry_e;

typedef enum
{
    E_MHAL_HDMITX_EXT_COLORIMETRY_XVYCC601 = 0,
    E_MHAL_HDMITX_EXT_COLORIMETRY_XVYCC709,
    E_MHAL_HDMITX_EXT_COLORIMETRY_SYCC601,
    E_MHAL_HDMITX_EXT_COLORIMETRY_ADOBEYCC601,
    E_MHAL_HDMITX_EXT_COLORIMETRY_ADOBERGB,
    E_MHAL_HDMITX_EXT_COLORIMETRY_BT2020CYCC, //mapping to ext. colorimetry format BT2020Y'cC'bcC'rc
    E_MHAL_HDMITX_EXT_COLORIMETRY_BT2020YCC, //mapping to ext. colorimetry format BT2020 RGB or YCbCr
    E_MHAL_HDMITX_EXT_COLORIMETRY_BT2020RGB, //mapping to ext. colorimetry format BT2020 RGB or YCbCr
    E_MHAL_HDMITX_EXT_COLORIMETRY_MAX,
}MhalHdmitxExtColorimetry_e;

typedef enum
{
    E_MHAL_HDMITX_ASPECT_RATIO_INVALID = 0,       /**< unknown aspect ratio */
    E_MHAL_HDMITX_ASPECT_RATIO_4TO3,              /**< 4:3 */
    E_MHAL_HDMITX_ASPECT_RATIO_16TO9,             /**< 16:9 */
    E_MHAL_HDMITX_ASPECT_RATIO_21TO9,             /**< 21:9 */
    E_MHAL_HDMITX_ASPECT_RATIO_MAX
}MhalHdmitxAspectRatio_e;

typedef enum
{
    E_MHAL_HDMITX_YCC_QUANTIZATION_LIMITED_RANGE = 0,    /**< Limited quantization range of 220 levels when receiving a CE video format*/
    E_MHAL_HDMITX_YCC_QUANTIZATION_FULL_RANGE,           /**< Full quantization range of 256 levels when receiving an IT video format*/
    E_MHAL_HDMITX_YCC_QUANTIZATION_MAX
}MhalHdmitxYccQuantRange_e;

typedef enum
{
    E_MHAL_HDMITX_VIDEO_AFD_SameAsPictureAR = 8, // 1000
    E_MHAL_HDMITX_VIDEO_AFD_4_3_Center = 9,      // 1001
    E_MHAL_HDMITX_VIDEO_AFD_16_9_Center = 10,    // 1010
    E_MHAL_HDMITX_VIDEO_AFD_14_9_Center = 11,    // 1011
    E_MHAL_HDMITX_VIDEO_AFD_Others = 15,         // 0000~ 0111, 1100 ~ 1111
}MhalHdmitxVideoAfdRatio_e;

typedef enum
{
    E_MHAL_HDMITX_SCAN_INFO_NO_DATA = 0,             /**< No Scan information*/
    E_MHAL_HDMITX_SCAN_INFO_OVERSCANNED,             /**< Scan information, Overscanned (for television) */
    E_MHAL_HDMITX_SCAN_INFO_UNDERSCANNED,            /**< Scan information, Underscanned (for computer) */
    E_MHAL_HDMITX_SCAN_INFO_FUTURE,
    E_MHAL_HDMITX_SCAN_INFO_MAX
}MhalHdmitxScanInfo_e;

typedef enum
{
    E_MHAL_HDMITX_AUDIO_CODE_PCM = 0,
    E_MHAL_HDMITX_AUDIO_CODE_NON_PCM,
    E_MHAL_HDMITX_AUDIO_CODE_MAX
}MhalHdmitxAudioCodeType_e;

typedef enum
{
    E_MHAL_HDMITX_AUDIO_SAMPLERATE_UNKNOWN      = 0,
    E_MHAL_HDMITX_AUDIO_SAMPLERATE_32K          = 1,
    E_MHAL_HDMITX_AUDIO_SAMPLERATE_44K          = 2,
    E_MHAL_HDMITX_AUDIO_SAMPLERATE_48K          = 3,
    E_MHAL_HDMITX_AUDIO_SAMPLERATE_88K          = 4,
    E_MHAL_HDMITX_AUDIO_SAMPLERATE_96K          = 5,
    E_MHAL_HDMITX_AUDIO_SAMPLERATE_176K         = 6,
    E_MHAL_HDMITX_AUDIO_SAMPLERATE_192K         = 7,
    E_MHAL_HDMITX_AUDIO_SAMPLERATE_MAX,
}MhalHdmitxSampleRate_e;


//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------

typedef struct
{
    bool bVideoEn;
    MhalHdmitxColorType_e enInColor;
    MhalHdmitxColorType_e enOutColor;
    MhalHdmitxColorDepthType_e enColorDepth;
    MhalHdmitxOutpuModeType_e enOutputMode;
    MhaHdmitxTimingResType_e enTiming;

    bool bAudioEn;
    MhalHdmitxAudioFreqType_e enAudioFreq;
    MhalHdmitxAudioChannelType_e enAudioCh;
    MhalHdmitxAudioCodingType_e enAudioCode;
    MhalHdmitxAudioSourceFormat_e enAudioFmt;
}MhalHdmitxAttrConfig_t;

typedef struct
{
    bool bMute;
    MhalHdmitxMuteType_e enType;
}MhalHdmitxMuteConfig_t;

typedef struct
{
    bool bEn;
}MhalHdmitxSignalConfig_t;

typedef struct
{
    u8 u8BlockId;
    u8 au8EdidData[128];
}MhalHdmitxSinkEdidDataConfig_t;

typedef struct
{
    bool bSupported;
}MhalHdmitxSinkSupportHdmiConfig_t;

typedef struct
{
    MhaHdmitxTimingResType_e enTiming;
    MhalHdmitxColorType_e enColor;
}MhalHdmiSinkColorForamtConfig_t;

typedef struct
{
    bool bHpd;
}MhalHdmitxSinkHpdStatusConfig_t;

typedef union
{
    MhalHdmitxSinkEdidDataConfig_t stEdidData;
    MhalHdmitxSinkSupportHdmiConfig_t stSupportedHdmi;
    MhalHdmiSinkColorForamtConfig_t stColoFmt;
    MhalHdmitxSinkHpdStatusConfig_t stHpdStatus;
}MhalHdmitxSinkInfoUnit_t;

typedef struct
{
    MhalHdmitxSinkInfoType_e enType;
    MhalHdmitxSinkInfoUnit_t stInfoUnit;
}MhalHdmitxSinkInfoConfig_t;

typedef struct
{
    u8    u8DrvCurTap1Ch0;
    u8    u8DrvCurTap1Ch1;
    u8    u8DrvCurTap1Ch2;
    u8    u8DrvCurTap1Ch3;
    u8    u8DrvCurTap2Ch0;
    u8    u8DrvCurTap2Ch1;
    u8    u8DrvCurTap2Ch2;
    u8    u8DrvCurTap2Ch3;
}MhalHdmitxAnaloDrvCurConfig_t;


typedef struct
{
    MhalHdmitxScanInfo_e enScanInfo;
    u8 A0Value;
    MhalHdmitxColorType_e enColorType;

    bool bEnableAfdOverWrite;
    MhalHdmitxVideoAfdRatio_e enAfdRatio;
    MhalHdmitxAspectRatio_e enAspectRatio;

    MhalHdmitxColorimetry_e enColorimetry;
    MhalHdmitxExtColorimetry_e enExtColorimetry;
    MhaHdmitxTimingResType_e enTimingType;
    MhalHdmitxYccQuantRange_e enYccQuantRange;
}MhalHdmitxAviInfoFrameConfig_t;


typedef struct
{
    u32 u32ChannelCount; //2 4 6 8 channels
    MhalHdmitxAudioCodeType_e enAudioCodeType;//PCM NON-PCM
    MhalHdmitxSampleRate_e enSampleRate;
}MhalHdmitxAudInfoFrameConfig_t;

typedef struct
{
    u8 au8VendorName[8];
    u8 au8ProductDescription[16];
}MhalHdmitxSpdInfoFrameConfig_t;

typedef union
{
    MhalHdmitxAviInfoFrameConfig_t stAviInfoFrame;
    MhalHdmitxAudInfoFrameConfig_t stAudInfoFrame;
    MhalHdmitxSpdInfoFrameConfig_t stSpdInfoFrame;
}MhalHdmitxInfoFrameUint_u;

typedef struct
{
    bool bEn;
    MhalHdmitxInfoFrameType_e enType;
    MhalHdmitxInfoFrameUint_u stInfoFrame;
}MhalHdmitxInfoFrameConfig_t;


typedef struct
{
    u8 u8GpioNum;
}MhalHdmitxHpdConfig_t;
#endif
