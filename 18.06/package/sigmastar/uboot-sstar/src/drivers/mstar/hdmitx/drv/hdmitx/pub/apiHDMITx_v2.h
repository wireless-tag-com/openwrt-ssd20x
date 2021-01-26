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

#ifndef _API_HDMITX_V2_H_
#define _API_HDMITX_V2_H_

#include "mhal_common.h"


#ifdef __cplusplus
extern "C"
{
#endif

#ifndef DLL_PACKET

#ifndef HDMITX_OS_TYPE_UBOOT
#define DLL_PACKED __attribute__((__packed__))
#else
#define DLL_PACKED
#endif

#endif


typedef enum
{
    E_HDMITX_CMD_INIT,
    E_HDMITX_CMD_EXIT,
    E_HDMITX_CMD_TURNONOFF,
    E_HDMITX_CMD_ENABLEPACKETGEN,
    E_HDMITX_CMD_SETHDMITXMODE,
    E_HDMITX_CMD_SETHDMITXMODE_CD,
    E_HDMITX_CMD_SETTMDSONOFF,
    E_HDMITX_CMD_DISABLETMDSCTRL,
    E_HDMITX_CMD_SETRBCHANNELSWAP,
    E_HDMITX_CMD_EXHIBIT,

    E_HDMITX_CMD_GETRXSTATUS,
    E_HDMITX_CMD_GETRXDCINFOFROMEDID,
    E_HDMITX_CMD_GETRXVIDEODORMATFROMEDID,
    E_HDMITX_CMD_GETVICLISTFROMEDID,
    E_HDMITX_CMD_GETDATABLOCKLENGTHFROMEDID,
    E_HDMITX_CMD_GETRXAUDIOFORMATFROMEDID,
    E_HDMITX_CMD_EDID_HDMISUPPORT,
    E_HDMITX_CMD_GETRXIDMANUFACTURERNAME,
    E_HDMITX_CMD_GETEDIDDATA,
    E_HDMITX_CMD_GETRX3DSTRUCTUREFROMEDID,
    E_HDMITX_CMD_GETCOLORFORMATFROMEDID,
    E_HDMITX_CMD_PKT_USER_DEFINE_CLEAR,

    E_HDMITX_CMD_PKT_USER_DEFINE,
    E_HDMITX_CMD_PKT_CONTENT_DEFINE,
    E_HDMITX_CMD_SETVIDEOONOFF,
    E_HDMITX_CMD_SETCOLORFORMAT,
    E_HDMITX_CMD_SET_VS_INFOFRAME,
    E_HDMITX_CMD_SETVIDEOOUTPUTTIMING,
    E_HDMITX_CMD_SETVIDEOOUTPUTASEPCTRATIO, //wrong
    E_HDMITX_CMD_SETVIDEOOUTPUTOVERSCAN_AFD,
    E_HDMITX_CMD_SETAUDIOONOFF,
    E_HDMITX_CMD_SETAUDIOFREQUENCY,

    E_HDMITX_CMD_SETVIDEOOUTPUTOVERSCAN_AFD_II,
	E_HDMITX_CMD_SETAUDIOCONFIGURATION,
    E_HDMITX_CMD_SETAUDIOSOURCEFORMAT,
    E_HDMITX_CMD_SETAVIINFOEXTCOLORIMETRY,
    E_HDMITX_CMD_GETAUDIOCTS,
    E_HDMITX_CMD_MUTEAUDIOFIFO,
    E_HDMITX_CMD_GETHDCPKEY,
    E_HDMITX_CMD_GETBKSV,
    E_HDMITX_CMD_GETAKSV,
    E_HDMITX_CMD_SETHDCPONOFF,
    E_HDMITX_CMD_SETAVMUTE,

    E_HDMITX_CMD_GETAVMUTESTATUS,
    E_HDMITX_CMD_HDCP_REVOCATIONKEY_LIST,
    E_HDMITX_CMD_HDCP_REVOCATIONKEY_CHECK,
    E_HDMITX_CMD_HDCP_ISSRMSIGNATUREVALID,
    E_HDMITX_CMD_GETHDCPSTATUS,
    E_HDMITX_CMD_HDCP_STARTAUTH,
    E_HDMITX_CMD_GETINTHDCPSTATUS,
    E_HDMITX_CMD_GETHDCP_PRESTATUS,
    E_HDMITX_CMD_UNHDCPRXCONTROL,
    E_HDMITX_CMD_HDCPRXFAILCONTROL,

    E_HDMITX_CMD_GETLIBVER,
	E_HDMITX_CMD_GETINFO,
    E_HDMITX_CMD_GETSTATUS,
    E_HDMITX_CMD_SETDBGLEVEL,
    E_HDMITX_CMD_SETHPDGPIOPIN,
    E_HDMITX_CMD_ANALOGTUNING,
    E_HDMITX_CMD_FORCEHDMIOUTPUTMODE,
    E_HDMITX_CMD_FORCEHDMIOUTPUTCOLORFORMAT,
    E_HDMITX_CMD_DISABLEREGWRITE,
    E_HDMITX_CMD_GETEDIDPHYADR,

    E_HDMITX_CMD_SETCECONOFF,
	E_HDMITX_CMD_GETCECSTATUS,
    E_HDMITX_CMD_EDIDCHECKING,
    E_HDMITX_CMD_RXBYPASS_MODE,
    E_HDMITX_CMD_DISABLE_RXBYPASS,
    E_HDMITX_CMD_SETAKSV2R0INTERVAL,
    E_HDMITX_CMD_ISRXVALID,
    E_HDMITX_CMD_GETCHIPCAPS,
    E_HDMITX_CMD_SETPOWERSTATE,
    E_HDMITX_CMD_GETEDIDDATABLOCK,
    E_HDMITX_CMD_GETKSVLIST,
    E_HDMITX_CMD_HDCP2ACCESSX74,
    E_HDMITX_CMD_HDCP2TxInit,
    E_HDMITX_CMD_HDCP2TxEnableEncrypt,
    E_HDMITX_CMD_HDCP2TxFillCipherKey,
    E_HDMITX_CMD_GENERALCTRL,
    E_HDMITX_CMD_COLOR_AND_RANGE_TRANSFORM,
    E_HDMITX_CMD_SSC_ENABLE,
    E_HDMITX_CMD_SET_COLORIMETRY,
    E_HDMITX_CMD_GET_FULL_RX_STATUS,

    E_HDMITX_CMD_TIMING_CAPABILITY_CHECK,
    E_HDMITX_CMD_GET_PANELSIZE_FROM_EDID,
    E_HDMITX_CMD_GET_TMDS_STATUS,
    E_HDMITX_CMD_HDCP1XCOMPARERI,
    E_HDMITX_CMD_GET_COLOR_FORMAT,
    E_HDMITX_CMD_GET_COLOR_DEPTH,
    E_HDMITX_CMD_CHECK_EDID_QUANT_RANGE,
    #if (defined(VANCLEEF_MCP) || defined(RAPTORS_MCP))
    E_HDMITX_CMD_VANCLEEF_LOADHDCPKEY,
    #endif
} E_HDMITX_IOCTL_CMDS;

typedef struct DLL_PACKED
{
    MS_BOOL bReturn;
} stHDMITx_Init,*pstHDMITx_Init;

typedef struct DLL_PACKED
{
    MS_BOOL bReturn;
} stHDMITx_Exit,*pstHDMITx_Exit;


typedef struct DLL_PACKED
{
    MS_BOOL state;
} stHDMITx_TurnOnOff,*pstHDMITx_TurnOnOff;

typedef struct DLL_PACKED
{
    MS_BOOL bflag;
} stHDMITx_EnablePacketGen,*pstHDMITx_EnablePacketGen;


typedef struct DLL_PACKED
{
    HDMITX_OUTPUT_MODE mode;
} stHDMITx_SetHDMITxMode,*pstHDMITx_SetHDMITxMode;


typedef struct DLL_PACKED
{
    HDMITX_OUTPUT_MODE mode;
    HDMITX_VIDEO_COLORDEPTH_VAL val;
} stHDMITx_SetHDMITxMode_CD,*pstHDMITx_SetHDMITxMode_CD;

typedef struct DLL_PACKED
{
    MS_BOOL state;
} stHDMITx_SetTMDSOnOff,*pstHDMITx_SetTMDSOnOff;


typedef struct DLL_PACKED
{
    MS_BOOL bFlag;
} stHDMITx_DisableTMDSCtrl,*pstHDMITx_DisableTMDSCtrl;

typedef struct DLL_PACKED
{
    MS_BOOL state;
} stHDMITx_SetRBChannelSwap,*pstHDMITx_SetRBChannelSwap;

typedef struct DLL_PACKED
{
    MS_BOOL bReturn;
} stHDMITx_GetRxStatus,*pstHDMITx_GetRxStatus;

typedef struct DLL_PACKED
{
    HDMITX_VIDEO_COLORDEPTH_VAL *val;
    MS_BOOL bReturn;
} stHDMITx_GetRxDCInfoFromEDID,*pstHDMITx_GetRxDCInfoFromEDID;

typedef struct DLL_PACKED
{
    MS_U8 *pu8Buffer;
    MS_U8 u8BufSize;
    MS_BOOL bReturn;
} stHDMITx_GetRxVideoFormatFromEDID,*pstHDMITx_GetRxVideoFormatFromEDID;

typedef struct DLL_PACKED
{
    MS_U8 *pu8Buffer;
    MS_U8 u8BufSize;
    MS_BOOL bReturn;
} stHDMITx_GetVICListFromEDID,*pstHDMITx_GetVICListFromEDID;

typedef struct DLL_PACKED
{
    HDMITX_VIDEO_TIMING timing;
    HDMITX_EDID_COLOR_FORMAT *pColorFmt;
    MS_BOOL bReturn;
} stHDMITx_GetColorFormatFromEDID,*pstHDMITx_GetColorFormatFromEDID;

typedef struct DLL_PACKED
{
    MS_U8 *pu8Length;
    MS_U8 u8TagCode;
    MS_BOOL bReturn;
} stHDMITx_GetDataBlockLengthFromEDID,*pstHDMITx_GetDataBlockLengthFromEDID;

typedef struct DLL_PACKED
{
    MS_U8 *pu8Buffer;
    MS_U8 u8BufSize;
    MS_BOOL bReturn;
} stHDMITx_GetRxAudioFormatFromEDID,*pstHDMITx_GetRxAudioFormatFromEDID;

typedef struct DLL_PACKED
{
    MS_BOOL *HDMI_Support;
    MS_BOOL bReturn;
} stHDMITx_EDID_HDMISupport,*pstHDMITx_EDID_HDMISupport;

typedef struct DLL_PACKED
{
    MS_U8 *pu8Buffer;
    MS_BOOL bReturn;
} stHDMITx_GetRxIDManufacturerName,*pstHDMITx_GetRxIDManufacturerName;

typedef struct DLL_PACKED
{
    MS_U8 *pu8Buffer;
    MS_BOOL BlockIdx;
    MS_BOOL bReturn;
} stHDMITx_GetEDIDData,*pstHDMITx_GetEDIDData;

typedef struct DLL_PACKED
{
    HDMITX_VIDEO_TIMING timing;
    HDMITX_EDID_3D_STRUCTURE_ALL *p3DStructure;
    MS_BOOL bReturn;
} stHDMITx_GetRx3DStructureFromEDID,*pstHDMITx_GetRx3DStructureFromEDID;

typedef struct DLL_PACKED
{
    HDMITX_PACKET_TYPE packet_type;
    MS_BOOL def_flag;
	HDMITX_PACKET_PROCESS def_process;
    MS_U8 def_fcnt;
} stHDMITx_PKT_User_Define,*pstHDMITx_PKT_User_Define;

typedef struct DLL_PACKED
{
    HDMITX_PACKET_TYPE packet_type;
    MS_U8 *data;
    MS_U8 length;
    MS_BOOL bReturn;
} stHDMITx_PKT_Content_Define,*pstHDMITx_PKT_Content_Define;

typedef struct DLL_PACKED
{
    MS_BOOL state;
} stHDMITx_SetVideoOnOff,*pstHDMITx_SetVideoOnOff;


typedef struct DLL_PACKED
{
    HDMITX_VIDEO_COLOR_FORMAT in_color;
    HDMITX_VIDEO_COLOR_FORMAT out_color;
} stHDMITx_SetColorFormat,*pstHDMITx_SetColorFormat;

typedef struct DLL_PACKED
{
    HDMITX_VIDEO_VS_FORMAT vs_format;
    HDMITX_VIDEO_3D_STRUCTURE vs_3d;
    HDMITX_VIDEO_4k2k_VIC vs_vic;
} stHDMITx_Set_VS_InfoFrame,*pstHDMITx_Set_VS_InfoFrame;

typedef struct DLL_PACKED
{
    HDMITX_VIDEO_TIMING mode;
} stHDMITx_SetVideoOutputTiming,*pstHDMITx_SetVideoOutputTiming;

typedef struct DLL_PACKED
{
    HDMITX_VIDEO_ASPECT_RATIO out_ar;
} stHDMITx_SetVideoOutputAsepctRatio,*pstHDMITx_SetVideoOutputAsepctRatio;

typedef struct DLL_PACKED
{
    MS_BOOL bflag;
    HDMITX_VIDEO_SCAN_INFO out_scaninfo;
    MS_U8 out_afd;
} stHDMITx_SetVideoOutputOverscan_AFD,*pstHDMITx_SetVideoOutputOverscan_AFD;

typedef struct DLL_PACKED
{
    MS_BOOL state;
} stHDMITx_SetAudioOnOff,*pstHDMITx_SetAudioOnOff;

typedef struct DLL_PACKED
{
    HDMITX_AUDIO_FREQUENCY freq;
} stHDMITx_SetAudioFrequency,*pstHDMITx_SetAudioFrequency;

typedef struct DLL_PACKED
{
    MS_BOOL bflag;
    HDMITX_VIDEO_SCAN_INFO out_scaninfo;
    MS_U8 out_afd;
    MS_U8 A0;
} stHDMITx_SetVideoOutputOverscan_AFD_II,*pstHDMITx_SetVideoOutputOverscan_AFD_II;

typedef struct DLL_PACKED
{
    HDMITX_AUDIO_FREQUENCY freq;
    HDMITX_AUDIO_CHANNEL_COUNT ch;
    HDMITX_AUDIO_CODING_TYPE type;
} stHDMITx_SetAudioConfiguration,*pstHDMITx_SetAudioConfiguration;

typedef struct DLL_PACKED
{
    HDMITX_AUDIO_SOURCE_FORMAT fmt;
} stHDMITx_SetAudioSourceFormat,*pstHDMITx_SetAudioSourceFormat;

typedef struct DLL_PACKED
{
    HDMITX_AVI_EXTENDED_COLORIMETRY enExtColorimetry;
    HDMITX_AVI_YCC_QUANT_RANGE enYccQuantRange;
    MS_BOOL bReturn;
} stHDMITx_SetAVIInfoExtColorimetry, *pstHDMITx_SetAVIInfoExtColorimetry;

typedef struct DLL_PACKED
{
    MS_U32 u32Return;
} stHDMITx_GetAudioCTS,*pstHDMITx_GetAudioCTS;

typedef struct DLL_PACKED
{
    MS_BOOL bflag;
} stHDMITx_MuteAudioFIFO,*pstHDMITx_MuteAudioFIFO;

typedef struct DLL_PACKED
{
    MS_BOOL useinternalkey;
    MS_U8 *data;
} stHDMITx_GetHdcpKey,*pstHDMITx_GetHdcpKey;

typedef struct DLL_PACKED
{
    MS_U8 *pdata;
    MS_BOOL bReturn;
} stHDMITx_GetBksv,*pstHDMITx_GetBksv;

typedef struct DLL_PACKED
{
    MS_U8 *pdata;
    MS_BOOL bReturn;
} stHDMITx_GetAksv,*pstHDMITx_GetAksv;

typedef struct DLL_PACKED
{
    MS_BOOL state;
} stHDMITx_SetHDCPOnOff,*pstHDMITx_SetHDCPOnOff;

typedef struct DLL_PACKED
{
    MS_BOOL bflag;
} stHDMITx_SetAVMUTE,*pstHDMITx_SetAVMUTE;

typedef struct DLL_PACKED
{
    MS_BOOL bReturn;
} stHDMITx_GetAVMUTEStatus,*pstHDMITx_GetAVMUTEStatus;

typedef struct DLL_PACKED
{
    MS_U8 *data;
    MS_U16 size;
} stHDMITx_HDCP_RevocationKey_List,*pstHDMITx_HDCP_RevocationKey_List;

typedef struct DLL_PACKED
{
    HDMITX_REVOCATION_STATE stReturn;
} stHDMITx_HDCP_RevocationKey_Check,*pstHDMITx_HDCP_RevocationKey_Check;

typedef struct DLL_PACKED
{
    MS_U8 *data;
    MS_U32 size;
    MS_BOOL bReturn;
} stHDMITx_HDCP_IsSRMSignatureValid,*pstHDMITx_HDCP_IsSRMSignatureValid;

typedef struct DLL_PACKED
{
    HDMITX_HDCP_STATUS stReturn;
} stHDMITx_GetHDCPStatus,*pstHDMITx_GetHDCPStatus;

typedef struct DLL_PACKED
{
    MS_BOOL bFlag;
} stHDMITx_HDCP_StartAuth,*pstHDMITx_HDCP_StartAuth;

typedef struct DLL_PACKED
{
    HDMITX_INT_HDCP_STATUS stReturn;
} stHDMITx_GetINTHDCPStatus,*pstHDMITx_GetINTHDCPStatus;

typedef struct DLL_PACKED
{
    HDMITX_INT_HDCP_STATUS stReturn;
} stHDMITx_GetHDCP_PreStatus,*pstHDMITx_GetHDCP_PreStatus;

typedef struct DLL_PACKED
{
    HDMITX_UNHDCPRX_CONTROL state;
} stHDMITx_UnHDCPRxControl,*pstHDMITx_UnHDCPRxControl;

typedef struct DLL_PACKED
{
    HDMITX_HDCPRXFail_CONTROL state;
} stHDMITx_HDCPRxFailControl,*pstHDMITx_HDCPRxFailControl;

/*
typedef struct DLL_PACKED
{
    const MSIF_Version **ppVersion;
    MS_BOOL bReturn;
} stHDMITx_GetLibVer,*pstHDMITx_GetLibVer;
*/

typedef struct DLL_PACKED
{
    HDMI_TX_INFO *pInfo;
    MS_BOOL bReturn;
} stHDMITx_GetInfo,*pstHDMITx_GetInfo;

typedef struct DLL_PACKED
{
    HDMI_TX_Status *pStatus;
    MS_BOOL bReturn;
} stHDMITx_GetStatus,*pstHDMITx_GetStatus;

typedef struct DLL_PACKED
{
    MS_U16 u16DbgSwitch;
    MS_BOOL bReturn;
} stHDMITx_SetDbgLevels,*pstHDMITx_SetDbgLevel;

typedef struct DLL_PACKED
{
    MS_U8 u8pin;
} stHDMITx_SetHPDGpioPin,*pstHDMITx_SetHPDGpioPin;

typedef struct DLL_PACKED
{
    HDMITX_ANALOG_TUNING *pInfo;
} stHDMITx_AnalogTuning,*pstHDMITx_AnalogTuning;

typedef struct DLL_PACKED
{
    MS_BOOL bflag;
    HDMITX_OUTPUT_MODE output_mode;
} stHDMITx_ForceHDMIOutputMode,*pstHDMITx_ForceHDMIOutputMode;

typedef struct DLL_PACKED
{
    MS_BOOL bflag;
    HDMITX_VIDEO_COLOR_FORMAT output_color;
    MS_BOOL bReturn;
} stHDMITx_ForceHDMIOutputColorFormat,*pstHDMITx_ForceHDMIOutputColorFormat;

typedef struct DLL_PACKED
{
    MS_BOOL bFlag;
} stHDMITx_DisableRegWrite,*pstHDMITx_DisableRegWrite;

typedef struct DLL_PACKED
{
    MS_U8 *pdata;
} stHDMITx_GetEDIDPhyAdr,*pstHDMITx_GetEDIDPhyAdr;

typedef struct DLL_PACKED
{
    MS_BOOL bflag;
} stHDMITx_SetCECOnOff,*pstHDMITx_SetCECOnOff;

typedef struct DLL_PACKED
{
    MS_BOOL bReturn;
} stHDMITx_GetCECStatus,*pstHDMITx_GetCECStatus;

typedef struct DLL_PACKED
{
    MS_BOOL bReturn;
} stHDMITx_EdidChecking,*pstHDMITx_EdidChecking;

typedef struct DLL_PACKED
{
    HDMITX_INPUT_FREQ freq;
    MS_BOOL bflag;
    MS_BOOL bReturn;
} stHDMITx_RxBypass_Mode,*pstHDMITx_RxBypass_Mode;

typedef struct DLL_PACKED
{
    MS_BOOL bReturn;
} stHDMITx_Disable_RxBypass,*pstHDMITx_Disable_RxBypass;

typedef struct DLL_PACKED
{
    MS_U32 u32Interval;
    MS_BOOL bReturn;
} stHDMITx_SetAksv2R0Interval,*pstHDMITx_SetAksv2R0Interval;

typedef struct DLL_PACKED
{
    MS_BOOL bReturn;
} stHDMITx_IsHDCPRxValid,*pstHDMITx_IsHDCPRxValid;

typedef struct DLL_PACKED
{
    EN_HDMITX_CAPS eCapType;
    MS_U32* pRet;
    MS_U32 ret_size;
    MS_BOOL bReturn;
} stHDMITx_GetChipCaps,*pstHDMITx_GetChipCaps;

typedef struct DLL_PACKED
{
    HDMITX_POWER_MODE u16PowerState;
    MS_U32 u32Return;
} stHDMITx_SetPowerState,*pstHDMITx_SetPowerState;

typedef struct DLL_PACKED
{
    HDMITX_CEA_DB_TAG_CODE enTagCode;
    HDMITX_CEA_EXT_TAG_CODE enExtTagCode;
    MS_U8* pu8Data;
    MS_U32 u32DataLen;
    MS_U32* pu32RealLen;
    MS_BOOL bReturn;
} stHDMITx_GetEdidDataBlocks, *pstHDMITx_GetEdidDataBlocks;

typedef struct DLL_PACKED
{
    MS_U8* pu8Bstatus;
    MS_U8* pu8KSVList;
    MS_U16 u16BufLen;
    MS_U16* pu16KSVLength;
    MS_BOOL bReturn;
} stHDMITx_GetKSVList, *pstHDMITx_GetKSVList;

typedef struct DLL_PACKED
{
    MS_U8 u8PortIdx;
    MS_U8 u8OffsetAddr;
    MS_U8 u8OpCode;
    MS_U8 *pu8RdBuf;
    MS_U16 u16RdLen;
    MS_U8 *pu8WRBuff;
    MS_U16 u16WrLen;
    MS_BOOL bReturn;
} stHDMITx_HDCP2AccessX74, *pstHDMITx_HDCP2AccessX74;

typedef struct DLL_PACKED
{
    MS_U8 u8PortIdx;
    MS_BOOL bEnable;
} stHDMITx_HDCP2TxInit, *pstHDMITx_HDCP2TxInit;

typedef struct DLL_PACKED
{
    MS_U8 u8PortIdx;
    MS_BOOL bEnable;
} stHDMITx_HDCP2TxEnableEncrypt, *pstHDMITx_HDCP2TxEnableEncrypt;

typedef struct DLL_PACKED
{
    MS_U8 u8PortIdx;
    MS_U8 *pu8Riv;
    MS_U8 *pu8KsXORLC128;
} stHDMITx_HDCP2TxFillCipherKey, *pstHDMITx_HDCP2TxFillCipherKey;

typedef struct DLL_PACKED
{
    MS_U32 u32Cmd;
    void *pu8Buf;
    MS_U32 u32BufSize;
    MS_BOOL bReturn;
} stHDMITx_GeneralCtrl, *pstHDMITx_GeneralCtrl;

typedef struct DLL_PACKED
{
    HDMITX_VIDEO_COLOR_FORMAT       input_color;
    HDMITX_VIDEO_COLOR_FORMAT       output_color;
    HDMITX_QUANT_RANGE               input_range;
    HDMITX_QUANT_RANGE               output_range;
    MS_BOOL                                         result;
} stHDMITX_COLOR_AND_RANGE_TRANSFORM_PARAMETERS, *pstHDMITX_COLOR_AND_RANGE_TRANSFORM_PARAMETERS;

typedef struct DLL_PACKED
{
    MS_U8                                             ubSSCEn;
    MS_BOOL                                         result;
} stHDMITX_SSCENABLE, *pstHDMITX_SSCENABLE;

typedef struct DLL_PACKED
{
    MS_U32                      u32StructVersion;//StructVersion Control
    HDMITX_AVI_COLORIMETRY      colorimetry;
    MS_U8                       u8Return;
} stHDMITX_SET_COLORIMETRY, *pstHDMITX_SET_COLORIMETRY;

typedef struct DLL_PACKED
{
    MS_U32  u32StructVersion;//StructVersion Control
    MS_U32  u32RxStatus;
} stHDMITX_GET_FULL_RX_STATUS, *pstHDMITX_GET_FULL_RX_STATUS;

typedef struct DLL_PACKED
{
    MS_U32  u32StructVersion;//StructVersion Control
    HDMITX_OUTPUT_MODE eOutputMode;
    HDMITX_VIDEO_TIMING eTiming;
    HDMITX_VIDEO_COLOR_FORMAT eInColor;
    HDMITX_VIDEO_COLOR_FORMAT eOutColor;
    HDMITX_VIDEO_COLORDEPTH_VAL eColorDepth;
    HDMITX_TIMING_ERROR  ubRet;
} stHDMITX_CHECK_LEGAL_TIMING, *pstHDMITX_CHECK_LEGAL_TIMING;

typedef struct DLL_PACKED
{
    MS_BOOL bReturn;
} stHDMITX_HDCP1X_COMPARE_RI, *pstHDMITX_HDCP1X_COMPARE_RI;

typedef struct DLL_PACKED
{
    MS_U32  u32StructVersion;//StructVersion Control
    MS_U32  u32TMDSStatus;
    MS_U32  u32Ret;
} stHDMITX_GET_TMDS_STATUS, *pstHDMITX_GET_TMDS_STATUS;

typedef struct DLL_PACKED
{
    MS_U32  u32StructVersion;//StructVersion Control
    HDMITX_VIDEO_COLOR_FORMAT  enColorFmt;
    MS_U32  u32Ret;
} stHDMITX_GET_COLOR_FMT, *pstHDMITX_GET_COLOR_FMT;

typedef struct DLL_PACKED
{
    MS_U32  u32StructVersion;//StructVersion Control
    HDMITX_VIDEO_COLORDEPTH_VAL  enColorDepth;
    MS_U32  u32Ret;
} stHDMITX_GET_COLOR_DEPTH, *pstHDMITX_GET_COLOR_DEPTH;

typedef struct DLL_PACKED
{
    MS_U32                      u32StructVersion;//StructVersion Control
    HDMITX_VIDEO_TIMING         eTiming;
    HDMITX_VIDEO_COLOR_FORMAT   eOutColor;
    HDMITX_EDID_QUANT_RANGE     u8Return;
} stHDMITX_CHECK_EDID_QUAN_RANGE, *pstHDMITX_CHECK_EDID_QUAN_RANG;

#if defined(VANCLEEF_MCP) || defined(RAPTORS_MCP)
typedef struct DLL_PACKED
{
    MS_U8 *pHdcpKey;
    MS_U32 u32Size;
    MS_BOOL bReturn;
} stHDMITx_Vancleef_LoadHDCPKey,*pstHDMITx_Vancleef_LoadHDCPKey;
#endif

#ifdef __cplusplus
}
#endif


#endif // _API_HDMITX_H_

