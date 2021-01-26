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

#ifndef _API_HDMITX_PRIV_H_
#define _API_HDMITX_PRIV_H_


////////////////////////////////////////////////////////////////////////////////
// Header Files
////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
{
#endif


#ifndef HDMITX_OS_TYPE_UBOOT
typedef struct __attribute__ ((packed))
#else
typedef struct
#endif
{
    MDrvHDMITX_PARAMETER_LIST       stHDMITxInfo;
    MDrvHdcpTx_PARAMETER_LIST       stHdcpTxInfo;
    MS_BOOL bEnableHDMITxTask;
    MS_BOOL bSetHPD;
    MS_BOOL bInit;
    MS_U32 u32AlignDummy;
} HDMITX_RESOURCE_PRIVATE;


#if 1
#define _HDMITX_SEMAPHORE_ENTRY(pInst)                                                              \
        if(MDrv_HDMITx_Get_Semaphore(pInst) != 1)                               \
        {      }

#define _HDMITX_GET_INST_PRIVATE()  \
    HDMITX_INSTANCE_PRIVATE *psHDMITXInstPri = NULL; \
    psHDMITXInstPri = (HDMITX_INSTANCE_PRIVATE *)pInstance;

#define _HDMITX_DECLARE_VARIABE() \
    HDMITX_INSTANCE_PRIVATE *psHDMITXInstPri = NULL; \
    HDMITX_RESOURCE_PRIVATE *psHDMITXResPri = NULL; \

#define _HDMITX_GET_VARIABLE_WITHOUT_DECLARE()  \
    psHDMITXInstPri = (HDMITX_INSTANCE_PRIVATE *)pInstance; \
    psHDMITXResPri = (HDMITX_RESOURCE_PRIVATE *)psHDMITXInstPri->psResPri;


#define _HDMITX_GET_VARIABLE()  \
    HDMITX_INSTANCE_PRIVATE *psHDMITXInstPri = NULL; \
    HDMITX_RESOURCE_PRIVATE *psHDMITXResPri = NULL; \
    psHDMITXInstPri = (HDMITX_INSTANCE_PRIVATE *)pInstance; \
    psHDMITXResPri = (HDMITX_RESOURCE_PRIVATE *)psHDMITXInstPri->psResPri;

#define _HDMITX_SEMAPHORE_RETURN(pInst)                                                             \
            MDrv_HDMITx_Release_Semaphore(pInst);
#endif


typedef MS_BOOL      (*IOCTL_HDMITX_INIT)                    (void *pInstance);
typedef MS_BOOL      (*IOCTL_HDMITX_EXIT)                    (void *pInstance);
typedef void         (*IOCTL_HDMITX_TUNRONOFF)               (void *pInstance, MS_BOOL state);
typedef void         (*IOCTL_HDMITX_ENABLEPACKETGEN)        (void *pInstance, MS_BOOL bflag);
typedef void         (*IOCTL_HDMITX_SETHDMITXMODE)          (void *pInstance, HDMITX_OUTPUT_MODE mode);
typedef void         (*IOCTL_HDMITX_SETHDMITXMODE_CD)       (void *pInstance, HDMITX_OUTPUT_MODE mode, HDMITX_VIDEO_COLORDEPTH_VAL val);
typedef void         (*IOCTL_HDMITX_SETTMDSONOFF)           (void *pInstance, MS_BOOL state);
typedef void         (*IOCTL_HDMITX_DISABLETMDSCTRL)        (void *pInstance, MS_BOOL bFlag);
typedef void         (*IOCTL_HDMITX_SETRBCHANNELSWAP)       (void *pInstance, MS_BOOL state);
typedef void         (*IOCTL_HDMITX_EXHIBIT)                 (void *pInstance);
typedef MS_BOOL      (*IOCTL_HDMITX_GETRXSTATUS)             (void);
typedef MS_BOOL      (*IOCTL_HDMITX_GETRXDCINFOFROMEDID)    (void *pInstance, HDMITX_VIDEO_COLORDEPTH_VAL *val);
typedef MS_BOOL      (*IOCTL_HDMITX_GETRXVIDEOFORMATFROMEDID)    (void *pInstance, MS_U8 *pu8Buffer, MS_U8 u8BufSize);
typedef MS_BOOL      (*IOCTL_HDMITX_GETVICLISTFROMEDID)             (void* pInstance, MS_U8 *pu8Buffer, MS_U8 u8BufSize);
typedef MS_BOOL      (*IOCTL_HDMITX_GETDATABLOCKLENGTHFROMEDID)    (void *pInstance, MS_U8 *pu8Length, MS_U8 u8TagCode);
typedef MS_BOOL      (*IOCTL_HDMITX_GETCOLORFORMATFROMEDID)            (void *pInstance, HDMITX_VIDEO_TIMING timing, HDMITX_EDID_COLOR_FORMAT *pColorFmt);
typedef MS_BOOL      (*IOCTL_HDMITX_GETRXAUDIOFORMATFROMEDID)    (void *pInstance, MS_U8 *pu8Buffer, MS_U8 u8BufSize);
typedef MS_BOOL      (*IOCTL_HDMITX_EDID_HDMISUPPORT)       (void *pInstance, MS_BOOL *HDMI_Support);
typedef MS_BOOL      (*IOCTL_HDMITX_GETRXIDMANUFACTURERNAME)     (void *pInstance, MS_U8 *pu8Buffer);
typedef MS_BOOL      (*IOCTL_HDMITX_GETEDIDDATA)     (void *pInstance, MS_U8 *pu8Buffer, MS_BOOL BlockIdx);
typedef MS_BOOL      (*IOCTL_HDMITX_GETRX3DSTRUCTUREFROMEDID)     (void *pInstance, HDMITX_VIDEO_TIMING timing, HDMITX_EDID_3D_STRUCTURE_ALL *p3DStructure);
typedef void         (*IOCTL_HDMITX_PKT_USER_DEFINE_CLEAR)           (void *pInstance);
typedef void         (*IOCTL_HDMITX_PKT_USER_DEFINE)        (void *pInstance, HDMITX_PACKET_TYPE packet_type, MS_BOOL def_flag, HDMITX_PACKET_PROCESS def_process, MS_U8 def_fcnt);
typedef MS_BOOL      (*IOCTL_HDMITX_PKT_CONTENT_DEFINE)     (void *pInstance, HDMITX_PACKET_TYPE packet_type, MS_U8 *data, MS_U8 length);
typedef void         (*IOCTL_HDMITX_SETVIDEOONOFF)          (void *pInstance, MS_BOOL state);
typedef void         (*IOCTL_HDMITX_SETCOLORFORMAT)         (void *pInstance, HDMITX_VIDEO_COLOR_FORMAT in_color, HDMITX_VIDEO_COLOR_FORMAT out_color);
typedef void         (*IOCTL_HDMITX_SET_VS_INFOFRAME)       (void *pInstance, HDMITX_VIDEO_VS_FORMAT vs_format, HDMITX_VIDEO_3D_STRUCTURE vs_3d, HDMITX_VIDEO_4k2k_VIC vs_vic);
typedef void         (*IOCTL_HDMITX_SETVIDEOOUTPUTTIMING)  (void *pInstance, HDMITX_VIDEO_TIMING mode);
typedef void         (*IOCTL_HDMITX_SETVIDEOOUTPUTASEPECTRATIO)  (void *pInstance, HDMITX_VIDEO_ASPECT_RATIO out_ar);
typedef void         (*IOCTL_HDMITX_SETVIDEOOUTPUTOVERSCAN_AFD)  (void *pInstance, MS_BOOL bflag, HDMITX_VIDEO_SCAN_INFO out_scaninfo, MS_U8 out_afd);
typedef void         (*IOCTL_HDMITX_SETAUDIOONOFF)          (void *pInstance, MS_BOOL state);
typedef void         (*IOCTL_HDMITX_SETAUDIOFREQUENCY)     (void *pInstance, HDMITX_AUDIO_FREQUENCY freq);
typedef void         (*IOCTL_HDMITX_SETVIDEOOUTPUTOVERSCAN_AFD_II)  (void *pInstance, MS_BOOL bflag, HDMITX_VIDEO_SCAN_INFO out_scaninfo, MS_U8 out_afd, MS_U8 A0);
typedef void         (*IOCTL_HDMITX_SETAUDIOCONFIGURATION) (void *pInstance, HDMITX_AUDIO_FREQUENCY freq, HDMITX_AUDIO_CHANNEL_COUNT ch, HDMITX_AUDIO_CODING_TYPE type);
typedef void         (*IOCTL_HDMITX_SETAUDIOSOURCEFORMAT)       (void *pInstance, HDMITX_AUDIO_SOURCE_FORMAT fmt);
typedef MS_BOOL      (*IOCTL_HDMITX_SETAVIINFOEXTCOLORIMETRY)       (void *pInstance, HDMITX_AVI_EXTENDED_COLORIMETRY ExtColorimetry, HDMITX_AVI_YCC_QUANT_RANGE YccQuantRange);
typedef MS_U32       (*IOCTL_HDMITX_GETAUDIOCTS)       (void *pInstance);
typedef void         (*IOCTL_HDMITX_MUTEAUDIOFIFO)       (void *pInstance, MS_BOOL bflag);
typedef void         (*IOCTL_HDMITX_GETHDCPKEY)       (void *pInstance, MS_BOOL useinternalkey, MS_U8 *data);
typedef MS_BOOL      (*IOCTL_HDMITX_GETBKSV)          (void *pInstance, MS_U8 *pdata);
typedef MS_BOOL      (*IOCTL_HDMITX_GETAKSV)          (void *pInstance, MS_U8 *pdata);
typedef void         (*IOCTL_HDMITX_SETHDCPONOFF)            (void *pInstance, MS_BOOL state);
typedef void         (*IOCTL_HDMITX_SETAVMUTE)               (void *pInstance, MS_BOOL bflag);
typedef MS_BOOL      (*IOCTL_HDMITX_GETAVMUTESTATUS)        (void *pInstance);
typedef void         (*IOCTL_HDMITX_HDCP_REVOCATIONKEY_LIST)        (void *pInstance, MS_U8 *data, MS_U16 size);
typedef HDMITX_REVOCATION_STATE (*IOCTL_HDMITX_HDCP_REVOCATIONKEY_CHECK)      (void *pInstance);
typedef MS_BOOL      (*IOCTL_HDMITX_HDCP_ISSRMSIGNATUREVALID)        (void *pInstance, MS_U8 *data, MS_U32 size);
typedef HDMITX_HDCP_STATUS      (*IOCTL_HDMITX_GETHDCPSTATUS)        (void *pInstance);
typedef void      (*IOCTL_HDMITX_HDCP_STARTAUTH)        (void *pInstance, MS_BOOL bFlag);
typedef HDMITX_INT_HDCP_STATUS      (*IOCTL_HDMITX_GETINTHDCPSTATUS)        (void *pInstance);
typedef HDMITX_INT_HDCP_STATUS      (*IOCTL_HDMITX_GETHDCP_PRESTATUS)       (void *pInstance);
//typedef MS_BOOL      (*IOCTL_HDMITX_GETLIBVER)        (void *pInstance, const MSIF_Version **ppVersion);
typedef MS_BOOL      (*IOCTL_HDMITX_GETINFO)        (void *pInstance, HDMI_TX_INFO *pInfo);
typedef MS_BOOL      (*IOCTL_HDMITX_GETSTATUS)        (void *pInstance, HDMI_TX_Status *pStatus);
typedef MS_BOOL      (*IOCTL_HDMITX_SETDBGLEVEL)        (MS_U16 u16DbgSwitch);
typedef void         (*IOCTL_HDMITX_UNHDCPRXCONTROL)      (void *pInstance, HDMITX_UNHDCPRX_CONTROL state);
typedef void         (*IOCTL_HDMITX_HDCPRXFAILCONTROL)    (void *pInstance, HDMITX_HDCPRXFail_CONTROL state);
typedef void         (*IOCTL_HDMITX_SETHPDGPIOPIN)         (void *pInstance, MS_U8 u8pin);
typedef void         (*IOCTL_HDMITX_ANALOGTUNING)         (void *pInstance, HDMITX_ANALOG_TUNING *pInfo);
typedef void         (*IOCTL_HDMITX_FORCEHDMIOUTPUTMODE)         (void *pInstance, MS_BOOL bflag, HDMITX_OUTPUT_MODE output_mode);
typedef MS_BOOL         (*IOCTL_HDMITX_FORCEHDMIOUTPUTCOLORFORMAT)         (void *pInstance, MS_BOOL bflag, HDMITX_VIDEO_COLOR_FORMAT output_color);
typedef void         (*IOCTL_HDMITX_DISABLEREGWRITE)           (void *pInstance, MS_BOOL bFlag);
typedef void         (*IOCTL_HDMITX_GETEDIDPHYADR)           (void *pInstance, MS_U8 *pdata);
typedef void         (*IOCTL_HDMITX_SETCECONOFF)           (void *pInstance, MS_BOOL bflag);
typedef MS_BOOL         (*IOCTL_HDMITX_GETCECSTATUS)         (void *pInstance);
typedef MS_BOOL      (*IOCTL_HDMITX_EDIDCHECKING)          (void *pInstance);
typedef MS_BOOL      (*IOCTL_HDMITX_RXBYPASS_MODE)         (void *pInstance, HDMITX_INPUT_FREQ freq, MS_BOOL bflag);
typedef MS_BOOL      (*IOCTL_HDMITX_DISABLE_RXBYPASS)      (void *pInstance);
typedef MS_BOOL      (*IOCTL_HDMITX_SETAKSV2R0INTERVAL)      (void *pInstance, MS_U32 u32Interval);
typedef MS_BOOL      (*IOCTL_HDMITX_ISRXVALID)                  (void *pInstance);
typedef MS_BOOL      (*IOCTL_HDMITX_GETCHIPCAPS)                (void *pInstance, EN_HDMITX_CAPS eCapType, MS_U32* pRet, MS_U32 ret_size);
typedef MS_U32       (*IOCTL_HDMITX_SETPOWERSTATE)              (void* pInstance, HDMITX_POWER_MODE u16PowerState);
typedef MS_BOOL      (*IOCTL_HDMITX_GETEDIDDATABLOCK)           (void* pInstance, HDMITX_CEA_DB_TAG_CODE enTagCode, HDMITX_CEA_EXT_TAG_CODE enExtTagCode, MS_U8* pu8Data, MS_U32 u32DataLen, MS_U32* pu32RealLen);
typedef MS_BOOL      (*IOCTL_HDMITX_GETKSVLIST)                     (void* pInstance, MS_U8 *pu8Bstatus, MS_U8* pu8KSVList, MS_U16 u16BufLen, MS_U16 *pu16KSVLength);
typedef MS_BOOL      (*IOCTL_HDMITX_HDCP2ACCESSX74OFFSET)      (void* pInstance, MS_U8 u8PortIdx, MS_U8 u8OffsetAddr, MS_U8 u8OpCode, MS_U8 *pu8RdBuf, MS_U16 u16RdLen, MS_U8 *pu8WRBuff, MS_U16 u16WrLen);
typedef void         (*IOCTL_HDMITX_HDCP2TxInit)                   (void* pInstance, MS_U8 u8PortIdx, MS_BOOL bEnable);
typedef void         (*IOCTL_HDMITX_HDCP2TxEnableEncrypt)        (void* pInstance, MS_U8 u8PortIdx, MS_BOOL bEnable);
typedef void         (*IOCTL_HDMITX_HDCP2TxFillCipherKey)        (void* pInstance, MS_U8 u8PortIdx, MS_U8 *pu8Riv, MS_U8 *pu8KsXORLC128);
typedef MS_BOOL      (*IOCTL_HDMITX_GENERALCTRL)                 (void* pInstance, MS_U32 u32Cmd, MS_U8* pu8Buf, MS_U32 u32BufSize);
typedef MS_BOOL      (*IOCTL_HDMITX_CMD_COLOR_AND_RANGE_TRANSFORM)  (void* pInstance, HDMITX_VIDEO_COLOR_FORMAT incolor, HDMITX_VIDEO_COLOR_FORMAT outcolor, HDMITX_QUANT_RANGE inange, HDMITX_QUANT_RANGE outrange);
typedef MS_BOOL      (*IOCTL_HDMITX_CMD_SSC_ENABLE)  (void* pInstance, MS_U8 ubSSCEn);
typedef MS_U8          (*IOCTL_HDMITX_CMD_SET_COLORIMETRY)  (void* pInstance, HDMITX_AVI_COLORIMETRY enColorimetry);
typedef MS_U32        (*IOCTL_HDMITX_CMD_GET_FULL_RX_STATUS)  (void* pInstance);
typedef HDMITX_TIMING_ERROR        (*IOCTL_HDMITX_CMD_TIMING_CAPABILITY_CHECK)  (void* pInstance, HDMITX_OUTPUT_MODE eOutputMode, HDMITX_VIDEO_TIMING idx, HDMITX_VIDEO_COLOR_FORMAT incolor_fmt, HDMITX_VIDEO_COLOR_FORMAT outcolor_fmt, HDMITX_VIDEO_COLORDEPTH_VAL color_depth);
typedef MS_BOOL      (*IOCTL_HDMITX_CMD_HDCP1X_COMPARE_RI)(void* pInstance);
typedef MS_U32       (*IOCTL_HDMITX_CMD_GET_TMDS_STATUS)  (void* pInstance);
typedef HDMITX_VIDEO_COLOR_FORMAT       (*IOCTL_HDMITX_CMD_GET_COLOR_FORMAT)  (void* pInstance);
typedef HDMITX_VIDEO_COLORDEPTH_VAL       (*IOCTL_HDMITX_CMD_GET_COLOR_DEPTH)  (void* pInstance);
typedef HDMITX_EDID_QUANT_RANGE        (*IOCTL_HDMITX_CMD_CHECK_EDID_QUANANTIZATION_RANGE)  (void* pInstance, HDMITX_VIDEO_TIMING idx, HDMITX_VIDEO_COLOR_FORMAT outcolor_fmt);

#if defined(VANCLEEF_MCP) || defined(RAPTORS_MCP)
typedef MS_BOOL      (*IOCTL_HDMITX_VANCLEEF_LOADHDCPKEY)      (void *pInstance, MS_U8* pu8HdcpKey, MS_U32 u32Size);
#endif

// OBSOLETE FUNCTION POINTER END

typedef struct
{
    MS_U32 u32DeviceID;
    MS_BOOL bUsed;
    void *psResPri; // HDMITX_RESOURCE_PRIVATE
    // OBSOLETE FUNCTION POINTER START
    // avoid using these functions pointers!!!!
}HDMITX_INSTANCE_PRIVATE;



MS_BOOL MApi_HDMITx_Init_U2(void *pInstance);
MS_BOOL MApi_HDMITx_Exit_U2(void *pInstance);
void MApi_HDMITx_TurnOnOff_U2(void *pInstance, MS_BOOL state);
void MApi_HDMITx_EnablePacketGen_U2(void *pInstance, MS_BOOL bflag);
void MApi_HDMITx_SetHDMITxMode_U2(void *pInstance, HDMITX_OUTPUT_MODE mode);
void MApi_HDMITx_SetHDMITxMode_CD_U2(void *pInstance, HDMITX_OUTPUT_MODE mode, HDMITX_VIDEO_COLORDEPTH_VAL val);
void MApi_HDMITx_SetTMDSOnOff_U2(void *pInstance, MS_BOOL state);
void MApi_HDMITx_DisableTMDSCtrl_U2(void *pInstance, MS_BOOL bFlag);
void MApi_HDMITx_SetRBChannelSwap_U2(void *pInstance, MS_BOOL state);
void MApi_HDMITx_Exhibit_U2(void *pInstance);
MS_BOOL MApi_HDMITx_GetRxStatus_U2(void);
MS_BOOL MApi_HDMITx_GetRxDCInfoFromEDID_U2(void *pInstance, HDMITX_VIDEO_COLORDEPTH_VAL *val);
MS_BOOL MApi_HDMITx_GetRxVideoFormatFromEDID_U2(void *pInstance, MS_U8 *pu8Buffer, MS_U8 u8BufSize);
MS_BOOL MApi_HDMITx_GetVICListFromEDID_U2(void* pInstance, MS_U8 *pu8Buffer, MS_U8 u8BufSize);
MS_BOOL MApi_HDMITx_GetDataBlockLengthFromEDID_U2(void *pInstance, MS_U8 *pu8Length, MS_U8 u8TagCode);
MS_BOOL MApi_HDMITx_GetRxAudioFormatFromEDID_U2(void *pInstance, MS_U8 *pu8Buffer, MS_U8 u8BufSize);
MS_BOOL MApi_HDMITx_EDID_HDMISupport_U2(void *pInstance, MS_BOOL *HDMI_Support);
MS_BOOL MApi_HDMITx_GetRxIDManufacturerName_U2(void *pInstance, MS_U8 *pu8Buffer);
MS_BOOL MApi_HDMITx_GetEDIDData_U2(void *pInstance, MS_U8 *pu8Buffer, MS_BOOL BlockIdx);
MS_BOOL MApi_HDMITx_GetRx3DStructureFromEDID_U2(void *pInstance, HDMITX_VIDEO_TIMING timing, HDMITX_EDID_3D_STRUCTURE_ALL *p3DStructure);
MS_BOOL MApi_HDMITx_GetColorFormatFromEDID_U2(void* pInstance, HDMITX_VIDEO_TIMING timing, HDMITX_EDID_COLOR_FORMAT *pColorFmt);
void MApi_HDMITx_PKT_User_Define_Clear_U2(void *pInstance);
void MApi_HDMITx_PKT_User_Define_U2(void *pInstance, HDMITX_PACKET_TYPE packet_type, MS_BOOL def_flag, HDMITX_PACKET_PROCESS def_process, MS_U8 def_fcnt);
MS_BOOL MApi_HDMITx_PKT_Content_Define_U2(void *pInstance, HDMITX_PACKET_TYPE packet_type, MS_U8 *data, MS_U8 length);
void MApi_HDMITx_SetVideoOnOff_U2(void *pInstance, MS_BOOL state);
void MApi_HDMITx_SetColorFormat_U2(void *pInstance, HDMITX_VIDEO_COLOR_FORMAT in_color, HDMITX_VIDEO_COLOR_FORMAT out_color);
void MApi_HDMITx_Set_VS_InfoFrame_U2(void *pInstance, HDMITX_VIDEO_VS_FORMAT vs_format, HDMITX_VIDEO_3D_STRUCTURE vs_3d, HDMITX_VIDEO_4k2k_VIC vs_vic);
void MApi_HDMITx_SetVideoOutputTiming_U2(void *pInstance, HDMITX_VIDEO_TIMING mode);
void MApi_HDMITx_SetVideoOutputAsepctRatio_U2(void *pInstance, HDMITX_VIDEO_ASPECT_RATIO out_ar);
void MApi_HDMITx_SetVideoOutputOverscan_AFD_U2(void *pInstance, MS_BOOL bflag, HDMITX_VIDEO_SCAN_INFO out_scaninfo, MS_U8 out_afd);
void MApi_HDMITx_SetAudioOnOff_U2(void *pInstance, MS_BOOL state);
void MApi_HDMITx_SetAudioFrequency_U2(void *pInstance, HDMITX_AUDIO_FREQUENCY freq);
void MApi_HDMITx_SetVideoOutputOverscan_AFD_II_U02(void *pInstance, MS_BOOL bflag, HDMITX_VIDEO_SCAN_INFO out_scaninfo, MS_U8 out_afd, MS_U8 A0);
void MApi_HDMITx_SetAudioConfiguration_U2(void *pInstance, HDMITX_AUDIO_FREQUENCY freq, HDMITX_AUDIO_CHANNEL_COUNT ch, HDMITX_AUDIO_CODING_TYPE type);
void MApi_HDMITx_SetAudioSourceFormat_U2(void *pInstance, HDMITX_AUDIO_SOURCE_FORMAT fmt);
MS_BOOL MApi_HDMITx_SetAVIInfoExtColorimetry_U2(void* pInstance, HDMITX_AVI_EXTENDED_COLORIMETRY enExtColorimetry, HDMITX_AVI_YCC_QUANT_RANGE enYccQuantRange);
MS_U32 MApi_HDMITx_GetAudioCTS_U2(void *pInstance);
void MApi_HDMITx_MuteAudioFIFO_U2(void *pInstance, MS_BOOL bflag);
void MApi_HDMITx_GetHdcpKey_U2(void *pInstance, MS_BOOL useinternalkey, MS_U8 *data);
MS_BOOL MApi_HDMITx_GetBksv_U2(void *pInstance, MS_U8 *pdata);
MS_BOOL MApi_HDMITx_GetAksv_U2(void *pInstance, MS_U8 *pdata);
void MApi_HDMITx_SetHDCPOnOff_U2(void *pInstance, MS_BOOL state);
void MApi_HDMITx_SetAVMUTE_U2(void *pInstance, MS_BOOL bflag);
MS_BOOL MApi_HDMITx_GetAVMUTEStatus_U2(void *pInstance);
void MApi_HDMITx_HDCP_RevocationKey_List_U2(void *pInstance, MS_U8 *data, MS_U16 size);
HDMITX_REVOCATION_STATE MApi_HDMITx_HDCP_RevocationKey_Check_U2(void *pInstance);
MS_BOOL MApi_HDMITx_HDCP_IsSRMSignatureValid_U2(void *pInstance, MS_U8 *data, MS_U32 size);
HDMITX_HDCP_STATUS MApi_HDMITx_GetHDCPStatus_U2(void *pInstance);
void MApi_HDMITx_HDCP_StartAuth_U2(void *pInstance, MS_BOOL bFlag);
HDMITX_INT_HDCP_STATUS MApi_HDMITx_GetINTHDCPStatus_U2(void *pInstance);
HDMITX_INT_HDCP_STATUS MApi_HDMITx_GetHDCP_PreStatus_U2(void *pInstance);
void MApi_HDMITx_UnHDCPRxControl_U2(void *pInstance, HDMITX_UNHDCPRX_CONTROL state);
void MApi_HDMITx_HDCPRxFailControl_U2(void *pInstance, HDMITX_HDCPRXFail_CONTROL state);
//MS_BOOL MApi_HDMITx_GetLibVer_U2(void *pInstance, const MSIF_Version **ppVersion);
MS_BOOL MApi_HDMITx_GetInfo_U2(void *pInstance, HDMI_TX_INFO *pInfo);
MS_BOOL MApi_HDMITx_GetStatus_U2(void *pInstance, HDMI_TX_Status *pStatus);
MS_BOOL MApi_HDMITx_SetDbgLevel_U2(MS_U16 u16DbgSwitch);
void MApi_HDMITx_SetHPDGpioPin_U2(void *pInstance, MS_U8 u8pin);
void MApi_HDMITx_AnalogTuning_U2(void *pInstance, HDMITX_ANALOG_TUNING *pInfo);
void MApi_HDMITx_ForceHDMIOutputMode_U2(void *pInstance, MS_BOOL bflag, HDMITX_OUTPUT_MODE output_mode);
MS_BOOL MApi_HDMITx_ForceHDMIOutputColorFormat_U2(void *pInstance, MS_BOOL bflag, HDMITX_VIDEO_COLOR_FORMAT output_color);
void MApi_HDMITx_DisableRegWrite_U2(void *pInstance, MS_BOOL bFlag);
void MApi_HDMITx_GetEDIDPhyAdr_U2(void *pInstance, MS_U8 *pdata);
void MApi_HDMITx_SetCECOnOff_U2(void *pInstance, MS_BOOL bflag);
MS_BOOL MApi_HDMITx_GetCECStatus_U2(void *pInstance);
MS_BOOL MApi_HDMITx_EdidChecking_U2(void *pInstance);
MS_BOOL MApi_HDMITx_RxBypass_Mode_U2(void *pInstance, HDMITX_INPUT_FREQ freq, MS_BOOL bflag);
MS_BOOL MApi_HDMITx_Disable_RxBypass_U2(void *pInstance);
MS_BOOL MApi_HDMITx_SetAksv2R0Interval_U2(void *pInstance, MS_U32 u32Interval);
MS_BOOL MApi_HDMITx_IsHDCPRxValid_U2(void *pInstance);
MS_BOOL MApi_HDMITx_GetChipCaps_U2(void *pInstance, EN_HDMITX_CAPS eCapType, MS_U32* pRet, MS_U32 ret_size);
MS_U32 MApi_HDMITx_SetPowerState_U2(void *pInstance, HDMITX_POWER_MODE u16PowerState);
MS_BOOL MApi_HDMITx_GetEdidDataBlocks_U2(void* pInstance, HDMITX_CEA_DB_TAG_CODE enTagCode, HDMITX_CEA_EXT_TAG_CODE enExtTagCode, MS_U8* pu8Data, MS_U32 u32DataLen, MS_U32* pu32RealLen);
MS_BOOL MApi_HDMITx_GetKSVList_U2(void* pInstance, MS_U8 *pu8Bstatus, MS_U8* pu8KSVList, MS_U16 u16BufLen, MS_U16 *pu16KSVLength);
MS_BOOL MApi_HDMITx_HDCP2AccessX74_U2(void* pInstance, MS_U8 u8PortIdx, MS_U8 u8OffsetAddr, MS_U8 u8OpCode, MS_U8 *pu8RdBuf, MS_U16 u16RdLen, MS_U8 *pu8WRBuff, MS_U16 u16WrLen);
void MApi_HDMITx_HDCP2TxInit_U2(void *pInstance, MS_U8 u8PortIdx, MS_BOOL bEnable);
void MApi_HDMITx_HDCP2TxEnableEncrypt_U2(void *pInstance, MS_U8 u8PortIdx, MS_BOOL bEnable);
void MApi_HDMITx_HDCP2TxFillCipherKey_U2(void *pInstance, MS_U8 u8PortIdx, MS_U8 *pu8Riv, MS_U8 *pu8KsXORLC128);
MS_BOOL MApi_HDMITx_GeneralCtrl_U2(void* pInstance, MS_U32 u32Cmd, MS_U8* pu8Buf, MS_U32 u32BufSize);
MS_BOOL MApi_HDMITx_ColorandRange_Transform_U2(void* pInstance, HDMITX_VIDEO_COLOR_FORMAT incolor, HDMITX_VIDEO_COLOR_FORMAT outcolor, HDMITX_QUANT_RANGE inange, HDMITX_QUANT_RANGE outrange);
MS_BOOL MApi_HDMITx_SSC_Enable_U2(void* pInstance, MS_U8 ubSSCEn);
MS_U8 MApi_HDMITx_SetAVIInfoColorimetry_U2(void* pInstance, HDMITX_AVI_COLORIMETRY enColorimetry);
MS_U32 MApi_HDMITx_GetFullRxStatus_U2(void* pInstance);
HDMITX_TIMING_ERROR MApi_HDMITx_CheckLegalTiming_U2(void* pInstance, HDMITX_OUTPUT_MODE eOutputMode, HDMITX_VIDEO_TIMING idx, HDMITX_VIDEO_COLOR_FORMAT incolor_fmt, HDMITX_VIDEO_COLOR_FORMAT outcolor_fmt, HDMITX_VIDEO_COLORDEPTH_VAL color_depth);
MS_BOOL MApi_HDMITx_Hdcp1xCompareRi_U2(void* pInstance);
MS_U32 MApi_HDMITx_GetTMDSStatus_U2(void* pInstance);
HDMITX_VIDEO_COLOR_FORMAT Mpi_HDMITx_GetAVIInfo_ColorFmt_U2(void* pInstance);
HDMITX_VIDEO_COLORDEPTH_VAL MApi_HDMITx_GetGC_ColorDepth_U2(void* pInstance);
HDMITX_EDID_QUANT_RANGE MApi_HDMITx_CheckEDIDQuantizationRange_U2(void* pInstance, HDMITX_VIDEO_TIMING idx, HDMITX_VIDEO_COLOR_FORMAT outcolor_fmt);

#if defined(VANCLEEF_MCP) || defined(RAPTORS_MCP)
MS_BOOL MApi_HDMITx_Vancleef_LoadHDCPKey_U2(void *pInstance, MS_U8* pu8HdcpKey, MS_U32 u32Size);
#endif



//internal use only
MS_U32 _HDMITX_GET_DEVICE_NUM(void);

MS_U32 HDMITXOpen(void** ppInstance, const void* const pAttribute);
MS_U32 HDMITXClose(void* pInstance);
MS_U32 HDMITXIoctl(void* pInstance, MS_U32 u32Cmd, void* pArgs);

#ifdef CONFIG_UTOPIA_PROC_DBG_SUPPORT
MS_U32 HDMITXMdbIoctl(MS_U32 cmd, const void* const pArgs);
#endif

#ifdef __cplusplus
}
#endif

#undef _API_HDMITX_PRIV_H_
#endif // _API_ACE_PRIV_H_

