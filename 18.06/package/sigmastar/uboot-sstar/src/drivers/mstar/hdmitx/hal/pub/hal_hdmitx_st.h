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

#ifndef _HAL_HDMITX_ST_H_
#define _HAL_HDMITX_ST_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_HDMITX_QUERY_RET_OK = 0,
    E_HAL_HDMITX_QUERY_RET_CFGERR,
    E_HAL_HDMITX_QUERY_RET_NOTSUPPORT,
    E_HAL_HDMITX_QUERY_RET_NONEED,
    E_HAL_HDMITX_QUERY_RET_ERR,
}HalHdmitxQueryRet_e;

typedef enum
{
    E_HAL_HDMITX_QUERY_INIT,
    E_HAL_HDMITX_QUERY_DEINIT,
    E_HAL_HDMITX_QUERY_ATTR_BEGIN,
    E_HAL_HDMITX_QUERY_ATTR,
    E_HAL_HDMITX_QUERY_ATTR_END,
    E_HAL_HDMITX_QUERY_SIGNAL,
    E_HAL_HDMITX_QUERY_MUTE,
    E_HAL_HDMITX_QUERY_ANALOG_DRV_CUR,
    E_HAL_HDMITX_QUERY_SINK_INFO,
    E_HAL_HDMITX_QUERY_INFO_FRAME,
    E_HAL_HDMITX_QUERY_DEBUG_LEVEL,
    E_HAL_HDMITX_QUERY_HPD,
    E_HAL_HDMITX_QUERY_CLK_SET,
    E_HAL_HDMITX_QUERY_CLK_GET,
    E_HAL_HDMITX_QUERY_MAX,
}HalHdmitxQueryType_e;


//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_HDMITX_COLOR_RGB444 = 0x01,
    E_HAL_HDMITX_COLOR_YUV444 = 0x02,
    E_HAL_HDMITX_COLOR_YUV422 = 0x04,
    E_HAL_HDMITX_COLOR_YUV420 = 0x08,
    E_HAL_HDMITX_COLOR_AUTO  =  0x10,
}HalHdmitxColorType_e;


typedef enum
{
    E_HAL_HDMITX_OUTPUT_MODE_DVI       = 0,
    E_HAL_HDMITX_OUTPUT_MODE_DVI_HDCP  = 1,
    E_HAL_HDMITX_OUTPUT_MODE_HDMI      = 2,
    E_HAL_HDMITX_OUTPUT_MODE_HDMI_HDCP = 3,
}HalHdmitxOutpuModeType_e;


typedef enum
{
    E_HAL_HDMITX_CD_NO_ID   = 0,
    E_HAL_HDMITX_CD_24_BITS = 1,
    E_HAL_HDMITX_CD_30_BITS = 2,
    E_HAL_HDMITX_CD_36_BITS = 3,
    E_HAL_HDMITX_CD_48_BITS = 4,
}HalHdmitxColorDepthType_e;


typedef enum
{
    E_HAL_HDMITX_RES_720X480P_60HZ   = 0,
    E_HAL_HDMITX_RES_720X576P_50HZ   = 1,
    E_HAL_HDMITX_RES_1280X720P_60HZ  = 2,
    E_HAL_HDMITX_RES_1280X720P_50HZ  = 3,
    E_HAL_HDMITX_RES_1920X1080P_24HZ = 4,
    E_HAL_HDMITX_RES_1920X1080P_25HZ = 5,
    E_HAL_HDMITX_RES_1920X1080P_30HZ = 6,
    E_HAL_HDMITX_RES_1920X1080P_50HZ = 7,
    E_HAL_HDMITX_RES_1920X1080P_60HZ = 8,
    E_HAL_HDMITX_RES_3840X2160P_24HZ = 9,
    E_HAL_HDMITX_RES_3840X2160P_25HZ = 10,
    E_HAL_HDMITX_RES_3840X2160P_30HZ = 11,
    E_HAL_HDMITX_RES_3840X2160P_50HZ = 12,
    E_HAL_HDMITX_RES_3840X2160P_60HZ = 13,
    E_HAL_HDMITX_RES_1024X768P_60HZ  = 14,
    E_HAL_HDMITX_RES_1366X768P_60HZ  = 15,
    E_HAL_HDMITX_RES_1440X900P_60HZ  = 16,
    E_HAL_HDMITX_RES_1280X800P_60HZ  = 17,
    E_HAL_HDMITX_RES_1280X1024P_60HZ = 18,
    E_HAL_HDMITX_RES_1680X1050P_60HZ = 19,
    E_HAL_HDMITX_RES_1600X1200P_60HZ = 20,
    E_HAL_HDMITX_RES_MAX             = 21,
}HalHdmitxTimingResType_e;


typedef enum
{
    E_HAL_HDMITX_AUDIO_FREQ_NO_SIG = 0,
    E_HAL_HDMITX_AUDIO_FREQ_32K    = 1,
    E_HAL_HDMITX_AUDIO_FREQ_44K    = 2,
    E_HAL_HDMITX_AUDIO_FREQ_48K    = 3,
    E_HAL_HDMITX_AUDIO_FREQ_88K    = 4,
    E_HAL_HDMITX_AUDIO_FREQ_96K    = 5,
    E_HAL_HDMITX_AUDIO_FREQ_176K   = 6,
    E_HAL_HDMITX_AUDIO_FREQ_192K   = 7,
    E_HAL_HDMITX_AUDIO_FREQ_NUM    = 8,
}HalHdmitxAudioFreqType_e;


typedef enum
{
    E_HAL_HDMITX_AUDIO_CH_2,
    E_HAL_HDMITX_AUDIO_CH_8,
}HalHdmitxAudioChannelType_e;

typedef enum
{
    E_HAL_HDMITX_AUDIO_CODING_PCM,
    E_HAL_HDMITX_AUDIO_CODING_NONPCM,
}HalHdmitxAudioCodingType_e;

typedef enum
{
    E_HAL_HDMITX_AUDIO_FORMAT_PCM,
    E_HAL_HDMITX_AUDIO_FORMAT_DSD,
    E_HAL_HDMITX_AUDIO_FORMAT_HBR,
    E_HAL_HDMITX_AUDIO_FORMAT_NA,
}HalHdmitxAudioSourceFormat_e;

typedef enum
{
    E_HAL_HDMITX_MUTE_NONE   = 0x00,
    E_HAL_HDMITX_MUTE_VIDEO  = 0x01,
    E_HAL_HDMITX_MUTE_AUDIO  = 0x02,
    E_HAL_HDMITX_MUTE_AVMUTE = 0x04,
}HalHdmitxMuteType_e;

typedef enum
{
    E_HAL_HDMITX_HDCP_14,
    E_HAL_HDMITX_HDCP_22,
}HalHdmitxHdcpType_e;

typedef enum
{
    E_HAL_HDMITX_SINK_INFO_EDID_DATA    = 0,
    E_HAL_HDMITX_SINK_INFO_HDMI_SUPPORT,
    E_HAL_HDMITX_SINK_INFO_COLOR_FORMAT,
    E_HAL_HDMITX_SINK_INFO_HPD_STATUS,
    E_HAL_HDMITX_SINK_INFO_NUM,
}HalHdmitxSinkInfoType_e;

typedef enum
{
    E_HAL_HDMITX_INFOFRAM_TYPE_AVI   = 0,
    E_HAL_HDMITX_INFOFRAM_TYPE_SPD   = 1,
    E_HAL_HDMITX_INFOFRAM_TYPE_AUDIO = 2,
    E_HAL_HDMITX_INFOFRAM_TYPE_MAX   = 3,
}HalHdmitxInfoFrameType_e;


typedef enum
{
    E_HAL_HDMITX_STATUS_FLAG_ATTR_BEGIN = 0x0001,
    E_HAL_HDMITX_STATUS_FLAG_ATTR       = 0x0002,
    E_HAL_HDMITX_STATUS_FLAG_ATTR_END   = 0x0004,
    E_HAL_HDMITX_STATUS_FLAG_SIGNAL     = 0x0008,
    E_HAL_HDMITX_STATUS_FLAG_AUDIO_MUTE = 0x0010,
    E_HAL_HDMITX_STATUS_FLAG_VIDEO_MUTE = 0x0020,
    E_HAL_HDMITX_STATUS_FLAG_AV_MUTE    = 0x0040,
    E_HAL_HDMITX_STATUS_FLAG_INFO_FRAME = 0x0080,
}HalHdmitxStatusFlag_e;

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    HalHdmitxQueryType_e enQueryType;
    void  *pInCfg;
    u32   u32CfgSize;
}HalHdmitxQueryInConfig_t;


typedef struct
{
    HalHdmitxQueryRet_e enQueryRet;
    void (*pSetFunc)(void *, void *);
}HalHdmitxQueryOutConfig_t;

typedef struct
{
    HalHdmitxQueryInConfig_t stInCfg;
    HalHdmitxQueryOutConfig_t stOutCfg;
}HalHdmitxQueryConfig_t;


typedef struct
{
    bool bVideoEn;
    HalHdmitxColorType_e enInColor;
    HalHdmitxColorType_e enOutColor;
    HalHdmitxColorDepthType_e enColorDepth;
    HalHdmitxOutpuModeType_e enOutputMode;
    HalHdmitxTimingResType_e enTiming;

    bool bAudioEn;
    HalHdmitxAudioFreqType_e enAudioFreq;
    HalHdmitxAudioChannelType_e enAudioCh;
    HalHdmitxAudioCodingType_e enAudioCode;
    HalHdmitxAudioSourceFormat_e enAudioFmt;
}HalHdmitxAttrConfig_t;

typedef struct
{
    bool bEn;
    HalHdmitxHdcpType_e enType;
}HalHdmitxHdcpConfig_t;

typedef struct
{
    bool bEn;
    HalHdmitxHdcpType_e enHdcpType;
}HalHdmiHdcpConfig_t;

typedef struct
{
    bool bMute;
    HalHdmitxMuteType_e enType;
}HalHdmitxMuteConfig_t;

typedef struct
{
    bool bEn;
}HalHdmitxSignalConfig_t;


typedef struct
{
    u8 u8BlockId;
    u8 au8EdidData[128];
}HalHdmitxSinkEdidDataConfig_t;

typedef struct
{
    bool bSupported;
}HalHdmitxSinkSupportHdmiConfig_t;

typedef struct
{
    HalHdmitxTimingResType_e enTiming;
    HalHdmitxColorType_e enColor;
}HalHdmitxSinkColorFormatConfig_t;

typedef struct
{
    bool bHpd;
}HalHdmitxSinkHpdStatusConfig_t;

typedef union
{
    HalHdmitxSinkEdidDataConfig_t stEdidData;
    HalHdmitxSinkSupportHdmiConfig_t stSupportedHdmi;
    HalHdmitxSinkColorFormatConfig_t stColoFmt;
    HalHdmitxSinkHpdStatusConfig_t stHpdStatus;
}HalHdmitxSinkInfoUnit_t;

typedef struct
{
    HalHdmitxSinkInfoType_e enType;
    HalHdmitxSinkInfoUnit_t stInfoUnit;
}HalHdmitxSinkInfoConfig_t;

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
}HalHdmitxAnaloDrvCurConfig_t;


typedef struct
{
    bool bEn;
    HalHdmitxInfoFrameType_e enType;
    u8 au8Data[128];
    u8 u8DataLen;
}HalHdmitxInfoFrameConfig_t;


typedef struct
{
    u8 u8GpioNum;
}HalHdmitxHpdConfig_t;

typedef struct
{
    bool bEn[HAL_HDMITX_CLK_NUM];
    u32  u32Rate[HAL_HDMITX_CLK_NUM];
    u32  u32Num;
} HalHdmitxClkConfig_t;

#endif
