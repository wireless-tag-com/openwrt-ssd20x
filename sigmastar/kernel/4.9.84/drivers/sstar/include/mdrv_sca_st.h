/*
* mdrv_sca_st.h- Sigmastar
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
#ifndef _MDRV_SCA_ST_H
#define _MDRV_SCA_ST_H

#if !defined(MS_U8) && !defined(MS_BOOL)
#define MS_U8     unsigned char      
#define MS_U16    unsigned short     
#define MS_U32    unsigned long      
#define MS_U64    unsigned long long 
#define MS_BOOL   unsigned char      
#define MS_S16    signed short       
#endif

//=============================================================================
// structure & Enum
//=============================================================================
typedef enum
{
    SCA_DEST_MAIN   = 0,
    SCA_DEST_SUB    = 1,
    SCA_DEST_MAIN_1 = 2,
    SCA_DEST_MAIN_2 = 3,
    SCA_DEST_SUB_2  = 4,
    SCA_DEST_CVBSO  = 5,
    SCA_DEST_HDMITx = 6,
    SCA_DEST_NUM,
}SCA_DEST_TYPE;


typedef enum
{
    SCA_SRC_VGA,
    SCA_SRC_YPBPR,
    SCA_SRC_CVBS,
    SCA_SRC_CVBS2,
    SCA_SRC_CVBS3,
    SCA_SRC_SVIDEO,	// Add S-video input source //
    SCA_SRC_DTV,
    SCA_SRC_SC0_VOP,
    SCA_SRC_SC1_VOP,
    SCA_SRC_SC2_VOP,
    SCA_SRC_BT656,
    SCA_SRC_BT656_1,
    SCA_SRC_CAMERA,
    SCA_SRC_NUM,
}SCA_SRC_TYPE;

typedef enum
{
    SCA_MVOP_INPUT_DRAM,
    SCA_MVOP_INPUT_H264,
    SCA_MVOP_INPUT_MVD,
    SCA_MVOP_INPUT_RVD,
    SCA_MVOP_INPUT_CLIP,
    SCA_MVOP_INPUT_JPD,
    SCA_MVOP_INPUT_HVD_3DLR,
    SCA_MVOP_INPUT_MVD_3DLR,
    SCA_MVOP_INPUT_UNKNOWN,
} SCA_MVOP_INPUT_SEL;


typedef enum
{
    SCA_MODE_PARSING_SUPPORT,
    SCA_MODE_PARSING_UNSUPPORT,
    SCA_MODE_PARSING_FAIL,
}SCA_PARSING_MODE_STATUS;

typedef enum
{
    SCA_VE_OUTPUT_NTSC,
    SCA_VE_OUTPUT_PAL,
}SCA_VE_OUTPUT_STD;


typedef enum
{
    SCA_MVOP_TILE_8x32,
    SCA_MVOP_TILE_16x32,
    SCA_MVOP_TILE_NONE,
}SCA_MVOP_TILE_TYPE;

typedef enum
{
    SCA_MODE_MONITOR_NOSYNC = 0,                ///< Input timing stable, no input sync detected
    SCA_MODE_MONITOR_STABLE_SUPPORT_MODE,       ///< Input timing stable, has stable input sync and support this timing
    SCA_MODE_MONITOR_STABLE_UN_SUPPORT_MODE,    ///< Input timing stable, has stable input sync but this timing is not supported
    SCA_MODE_MONITOR_UNSTABLE,                  ///< Timing change, has to wait InfoFrame if HDMI input
}SCA_MODE_MOINITOR_STATUS;


typedef enum
{
    SCA_PICTURE_BRIGHTNESS        = 0x01,
    SCA_PICTURE_CONTRAST          = 0x02,
    SCA_PICTURE_HUE               = 0x04,
    SCA_PICTURE_SATURATION        = 0x08,
    SCA_PICTURE_SHARPNESS         = 0x10,
    SCA_PICTURE_COLOR_TEMPERATURE = 0x20,
}SCA_PICTURE_TYPE;

typedef enum
{
    SCA_COLOR_TEMP_COOL,
    SCA_COLOR_TEMP_NORMAL,
    SCA_COLOR_TEMP_WARM,
}SCA_COLOR_TEMPERATURE_TYPE;

typedef enum
{
    SCA_OUTPUT_480_I_60,
    SCA_OUTPUT_480_P_60,
    SCA_OUTPUT_576_I_50,
    SCA_OUTPUT_576_P_50,
    SCA_OUTPUT_720_P_50,
    SCA_OUTPUT_720_P_60,
    SCA_OUTPUT_1080_P_50,
    SCA_OUTPUT_1080_P_60,
    SCA_OUTPUT_480_P_30,
    SCA_OUTPUT_720_P_30,
}SCA_OUTPUT_TIMING_TYPE;

typedef enum
{
    SCA_REG_R_BYTE,
    SCA_REG_W_BYTE,
    SCA_REG_XC_R2BYTEMSK,
    SCA_REG_XC_W2BYTEMSK,
}SCA_RW_REG_TYPE;


typedef enum
{
    SCA_DISP_PRI_NONE     = 0,
    SCA_DISP_PRI_NORMAL   = 1,
    SCA_DISP_PRI_CAR_BACK = 2,
}SCA_DISP_PRIORITY_TYPE;

typedef enum
{
    SCA_PQ_BIN_ID_STD_MAIN     = 0x0,
    SCA_PQ_BIN_ID_STD_SUB      = 0x1,
    SCA_PQ_BIN_ID_STD_SC1_MAIN = 0x2,
    SCA_PQ_BIN_ID_STD_SC2_MAIN = 0x3,
    SCA_PQ_BIN_ID_STD_SC2_SUB  = 0x4,
    SCA_PQ_BIN_ID_EXT_MAIN     = 0x5,
    SCA_PQ_BIN_ID_EXT_SUB      = 0x6,
    SCA_PQ_BIN_ID_EXT_SC1_MAIN = 0x7,
    SCA_PQ_BIN_ID_EXT_SC2_MAIN = 0x8,
    SCA_PQ_BIN_ID_EXT_SC2_SUB  = 0x9,
    SCA_PQ_BIN_ID_NUM,
    SCA_PQ_BIN_ID_NONE         = 0xFF,
}SCA_PQ_BIN_ID_TYPE;

typedef enum
{
    SCA_DIP_DEST_FMT_YC422,
    SCA_DIP_DEST_FMT_RGB565,
    SCA_DIP_DEST_FMT_ARGB8888,
    SCA_DIP_DEST_FMT_YC420_MVOP,
    SCA_DIP_DEST_FMT_YC420_MFE,
}SCA_DIP_DEST_FMT_TYPE;

typedef enum
{
    SCA_DIP_TRIGGER_LOOP = 0,
    SCA_DIP_TRIGGER_ONCE,
}SCA_DIP_TRIGGER_TYPE;

typedef enum
{
    SCA_HDMITX_OUT_HDMI,
    SCA_HDMITX_OUT_HDMI_HDCP,
}SCA_HDMITX_MODE_TYPE;

typedef enum
{
    SCA_HDMITX_480_I_60,
    SCA_HDMITX_480_P_60,
    SCA_HDMITX_576_I_50,
    SCA_HDMITX_576_P_50,
    SCA_HDMITX_720_P_50,
    SCA_HDMITX_720_P_60,
    SCA_HDMITX_1080_I_50,
    SCA_HDMITX_1080_I_60,
    SCA_HDMITX_1080_P_50,
    SCA_HDMITX_1080_P_60,
    SCA_HDMITX_1080_P_30,
    SCA_HDMITX_1080_P_25,
    SCA_HDMITX_1080_P_24,
}SCA_HDMITX_OUTPUT_TIMINE_TYPE;

typedef enum
{
    SCA_HDMITX_OUTPUT_COLOR_RGB,
    SCA_HDMITX_OUTPUT_COLOR_YUV,
}SCA_HDMITX_OUTPUT_COLOR_TYPE;

typedef  enum
{
    SCA_CLONE_SCREEN_NONE,
    SCA_CLONE_SCREEN_GOP,
}SCA_CLONE_SCREEN_TYPE;


typedef enum
{
    SCA_DISPLAY_MUTE_BLACK,
    SCA_DISPLAY_MUTE_WHITE,
    SCA_DISPLAY_MUTE_BLUE,
    SCA_DISPLAY_MUTE_RED,
    SCA_DISPLAY_MUTE_GREEN,
}SCA_DISPLAY_MUTE_COLOR_TYPE;


//------------------------------------------------------------------------------
typedef struct
{
    SCA_SRC_TYPE enSrcType;
    SCA_DEST_TYPE enDestType;
    MS_BOOL bAutoDetect;
    SCA_DISP_PRIORITY_TYPE enPriType;
    MS_U32  dwUID;
}SCA_CONNECT_CONFIG, *PSCA_CONNECT_CONFIG;

typedef struct
{
    MS_U16     u16HSize;
    MS_U16     u16VSize;
    MS_U32     u32YOffset;
    MS_U32     u32UVOffset;
    MS_BOOL bSD;
    MS_BOOL bYUV422;
    MS_BOOL bProgressive;
    MS_BOOL bUV7bit;
    MS_BOOL bDramRdContd;
    MS_BOOL bField;
    MS_BOOL b422pack;
    MS_U16     u16StripSize;
    SCA_MVOP_INPUT_SEL enInputSel;
}SCA_MVOP_IN_CONFIG, *PSCA_MVOP_IN_CONFIG;

typedef struct
{
    MS_U16 u16HorSize;
    MS_U16 u16VerSize;
    MS_U16 u16FrameRate;
    MS_U8  u8AspectRate;
    MS_U8  u8Interlace;
}SCA_MVOP_OUT_CONFIG, *PSCA_MVOP_OUT_CONFIG;


typedef struct
{
    SCA_MVOP_TILE_TYPE  enTileType;
    SCA_MVOP_IN_CONFIG  stInConfig;
    SCA_MVOP_OUT_CONFIG stOutConfig;
}SCA_MVOP_CONFIG, *PSCA_MVOP_CONFIG;

typedef struct
{
    MS_BOOL bUpdate;
    MS_U16 u16InputVTotal;
    MS_BOOL bInterlace;
    MS_U16 u16InputVFreq;
}SCA_TIMING_CONFIG, *PSCA_TIMING_CONFIG;

typedef struct
{
    MS_U16 x;
    MS_U16 y;    ///<start y of the window
    MS_U16 width;
    MS_U16 height;
} SCA_WINDOW_TYPE, *PSCA_WINDOW_TYPE;

typedef struct
{
    MS_BOOL bSrcWin;
    MS_BOOL bCropWin;
    MS_BOOL bDispWin;
    SCA_WINDOW_TYPE stSrcWin;
    SCA_WINDOW_TYPE stCropWin;
    SCA_WINDOW_TYPE stDispWin;
}SCA_WINDOW_CONFIG, *PSCA_WINDOW_CONFIG;

typedef struct
{
    SCA_SRC_TYPE enSrcType;
    MS_U16 u16H_CapStart;
    MS_U16 u16H_CapSize;
    MS_U16 u16V_CapStart;
    MS_U16 u16V_CapSize;
    MS_U16 u16InputVFreq;
    MS_BOOL bHDuplicate;
    MS_BOOL bSrcInterlace;
    SCA_VE_OUTPUT_STD enVideoStd;
}SCA_VE_CONFIG, *PSCA_VE_CONFIG;

typedef struct
{
    SCA_SRC_TYPE      enSrcType;
    SCA_DEST_TYPE     enDestType;
    SCA_TIMING_CONFIG stTimingConfig;
    SCA_WINDOW_CONFIG stWinConfig;
    MS_U32  dwUID;
}SCA_SET_TIMING_WINDOW_CONFIG, *PSCA_SET_TIMING_WINDOW_CONFIG;


typedef struct
{
    MS_BOOL bEn;
    SCA_DEST_TYPE enDestType;
    MS_U32  dwUID;
}SCA_DISPLAY_MUTE_CONFIG, *PSCA_DISPLAY_MUTE_CONFIG;


typedef struct
{
    MS_BOOL bEn;
    SCA_SRC_TYPE enSrcType;
}SCA_CVBSO_MUTE_CONFIG, *PSCA_CVBSO_MUTE_CONFIG;


typedef struct
{
    SCA_SRC_TYPE      enSrcType;
    SCA_DEST_TYPE     enDestType;
}SCA_MODE_MONITOR_PARSE_IN_CONFIG, *PSCA_MODE_MONITOR_PARSE_IN_CONFIG;


typedef struct
{
    SCA_MODE_MOINITOR_STATUS eModeMonitorStatus;
    MS_U8 u8ModeIdx;
    MS_U16 u16HorizontalStart;
    MS_U16 u16VerticalStart;
    MS_U16 u16HorizontalTotal;
}SCA_MODE_MONITOR_PARSE_OUT_CONFIG, *PSCA_MODE_MONITOR_PARSE_OUT_CONFIG;

typedef struct
{
    SCA_MODE_MONITOR_PARSE_IN_CONFIG stMode_MP_InCfg;
    SCA_MODE_MONITOR_PARSE_OUT_CONFIG stMode_MP_OutCfg;
}SCA_MODE_MONITOR_PARSE_CONFIG;

typedef struct
{
    SCA_SRC_TYPE      enSrcType;
    SCA_DEST_TYPE     enDestType;
}SCA_GET_MODE_IN_CONFIG, *PSCA_GET_MODE_IN_CONFIG;


typedef struct
{
    SCA_WINDOW_TYPE stSrcWin;
    MS_U16 u16InputVFreq;
    MS_U16 u16InputVtotal;
    MS_BOOL bInterlace;
}SCA_GET_MODE_OUT_CONFIG, *PSCA_GET_MODE_OUT_CONFIG;

typedef struct
{
    SCA_GET_MODE_IN_CONFIG  stGetModeInCfg;
    SCA_GET_MODE_OUT_CONFIG stGetModeOutCfg;
}SCA_GET_MODE_CONFIG;
typedef struct
{
    MS_BOOL bMainEn;
    MS_BOOL bSubEn;
}SCA_DISPLAY_WINDOW_ON_OFF_CONFIG, *PSCA_DISPLAY_WINDOW_ON_OFF_CONFIG;

typedef struct
{
    MS_U32 u32YOffset;
    MS_U32 u32UVOffset;
    MS_BOOL bProgressive;
    MS_BOOL b422pack;
}SCA_MVOP_BASE_ADDR_CONFIG, *PSCA_MVOP_BASE_ADDR_CONFIG;


typedef struct
{
    int View;
    MS_U8 u8R;
    MS_U8 u8G;
    MS_U8 u8B;
    MS_BOOL Benable;
}SCA_COLOR_KEY, *PSCA_COLOR_KEY;


typedef struct
{
    SCA_SRC_TYPE enSrcType;
    SCA_DEST_TYPE enDestType;
    SCA_WINDOW_CONFIG stWinConfig;
    MS_BOOL bImmediateUpdate;
    MS_U32  dwUID;
}SCA_CHANGE_WINDOW_CONFIG, *PSCA_CHANGE_WINDOW_CONFIG;

typedef struct
{
    SCA_SRC_TYPE enSrcType;
    SCA_DEST_TYPE enDestType;
    SCA_PICTURE_TYPE enType;

    MS_U8 u8Brightness;
    MS_U8 u8Contrast;
    MS_U8 u8Hue;
    MS_U8 u8Saturation;
    MS_U8 u8Sharpness;
    SCA_COLOR_TEMPERATURE_TYPE enColorTemp;
}SCA_SET_PICTURE_CONFIG, *PSCA_SET_PICTURE_CONFIG;

typedef struct
{
    SCA_SRC_TYPE enSrcType;
    SCA_DEST_TYPE enDestType;

    MS_U8 u8Brightness;
    MS_U8 u8Contrast;
    MS_U8 u8Hue;
    MS_U8 u8Saturation;
    MS_U8 u8Sharpness;
    SCA_COLOR_TEMPERATURE_TYPE enColorTemp;
}SCA_PICTURE_DATA, *PSCA_PICTURE_DATA;

/// Struct for store ADC Gain and Offset
typedef struct
{
    MS_U16 u16RedGain;      ///< ADC red gain
    MS_U16 u16GreenGain;    ///< ADC green gain
    MS_U16 u16BlueGain;     ///< ADC blue gain
    MS_U16 u16RedOffset;    ///< ADC red offset
    MS_U16 u16GreenOffset;  ///< ADC green offset
    MS_U16 u16BlueOffset;   ///< ADC blue offset
} SCA_ADC_CALIB_GAINOFFSET, *PSCA_ADC_CALIB_GAINOFFSET;


typedef struct
{
	SCA_ADC_CALIB_GAINOFFSET stAdcGainOffset;
	MS_U8                       u8AdcCalOK;
}SCA_ADC_CALIB_INFO, *PSCA_ADC_CALIB_INFO;

typedef struct
{
    SCA_SRC_TYPE       enSrcType;
    SCA_ADC_CALIB_INFO stCalibInfo;
}SCA_CALIB_CONFIG;


typedef struct
{
	SCA_SRC_TYPE			eSourceInput;
	MS_BOOL                    bIsHDMode;
	SCA_ADC_CALIB_INFO		stAdcCalibInfo;
}SCA_ADC_CALIB_LOAD, *PSCA_ADC_CALIB_LOAD;


typedef struct
{
    SCA_DEST_TYPE enDestType;
    SCA_OUTPUT_TIMING_TYPE enTimingType;
}SCA_OUTPUT_TIMING_CONFIG, *PSCA_OUTPUT_TIMING_CONFIG;

//------------------------------------------------------------------------------
// AVD
//------------------------------------------------------------------------------
typedef enum
{
    AVD_SRC_CVBS,
	AVD_SRC_SVIDEO,	// Add s-video input source //
    AVD_SRC_NUM,
}AVD_SRC_TYPE;


typedef enum
{
    AVD_VIDEOSTANDARD_PAL_BGHI        = 0x00,        // Video standard PAL BGHI
    AVD_VIDEOSTANDARD_NTSC_M          = 0x01,        // Video standard NTSC M
    AVD_VIDEOSTANDARD_SECAM           = 0x02,        // Video standard SECAM
    AVD_VIDEOSTANDARD_NTSC_44         = 0x03,        // Video standard  NTSC 44
    AVD_VIDEOSTANDARD_PAL_M           = 0x04,        // Video standard  PAL M
    AVD_VIDEOSTANDARD_PAL_N           = 0x05,        // Video standard  PAL N
    AVD_VIDEOSTANDARD_PAL_60          = 0x06,        // Video standard PAL 60
    AVD_VIDEOSTANDARD_NOTSTANDARD     = 0x07,        // NOT Video standard
    AVD_VIDEOSTANDARD_AUTO            = 0x08,        // Video standard AUTO
    AVD_VIDEOSTANDARD_MAX                            // Max Number
} AVD_VIDEO_STANDARD;

typedef enum
{
    AVD_VIDEO_DETECT_UNSTABLE = 0,
    AVD_VIDEO_DETECT_STABLE = 1,
}AVD_VIDEO_DETECT_STATE;

//------------------------------------------------------------------------------
typedef struct
{
    AVD_SRC_TYPE enSrcType;
}AVD_CONNECT_CONFIG, *PAVD_CONNECT_CONFIG;

typedef struct
{
    AVD_VIDEO_STANDARD enVideoStandard;
    AVD_VIDEO_DETECT_STATE enDetectState;

}AVD_CHECK_VIDEOSTD_CONFIG, *PAVD_CHECK_VIDEOSTD_CONFIG;


typedef struct
{
	MS_U8  u8View;
	MS_U8  u8AlphaValue;
}SCA_CONSTANT_ALPHA_SETTING, *PSCA_CONSTANT_ALPHA_SETTING;

typedef struct
{
    MS_U8   u8View;
    MS_BOOL Benable;
}SCA_CONSTANTALPHA_STATE, *PSCA_CONSTANTALPHA_STATE;



typedef struct
{
        MS_U8	name[4];
        MS_U8   build[4];
        MS_U32	version;
        MS_U32	changeList;
}SCA_LIB_VER, *PSCA_LIB_VER;

typedef struct
{
    // 0x00, first 4 bits are unused
	MS_U8 INT_NA			:4;
    MS_U8 INT_TUNE_FAIL_P	:1;               ///< scaler dosen't have this interrupt now
    MS_U8 INT_VSINT			:1;                       ///< output Vsync interrupt
    MS_U8 INT_F2_VTT_CHG	:1;                  ///< main window, HDMI mute or Vsync polarity changed, Vtt change exceed BK1_1D[11:8]
    MS_U8 INT_F1_VTT_CHG	:1;
    MS_U8 INT_F2_VS_LOSE	:1;                  ///< didn't received Vsync for a while or Vtt count BK1_1F[10:0] exceed max value
    MS_U8 INT_F1_VS_LOSE	:1;
    MS_U8 INT_F2_JITTER		:1;                   ///< H/V start/end didn't be the same with privous value, usually used in HDMI/DVI input
    MS_U8 INT_F1_JITTER		:1;
    MS_U8 INT_F2_IPVS_SB    :1;                 ///< input V sync interruptB
    MS_U8 INT_F1_IPVS_SB	:1;
    MS_U8 INT_F2_IPHCS_DET	:1;                ///< input H sync interrupt
    MS_U8 INT_F1_IPHCS_DET	:1;

    // 0x10
    MS_U8 INT_F2_IPHCS1_DET	:1;                ///< pwm rising edge of left frame, please use MDrv_XC_InterruptAvaliable() to check if avalible or not
    MS_U8 INT_F1_IPHCS1_DET	:1;                ///< pwm falling edge of left frame, please use MDrv_XC_InterruptAvaliable() to check if avalible or not
    MS_U8 INT_F2_HTT_CHG	:1;                  ///< Hsync polarity changed or Hperiod change exceed BK1_1D[5:0]
    MS_U8 INT_F1_HTT_CHG	:1;
    MS_U8 INT_F2_HS_LOSE	:1;                  ///< didn't received H sync for a while or Hperiod count BK1_20[13:0] exceed max value
    MS_U8 INT_F1_HS_LOSE	:1;
    MS_U8 INT_DVI_CK_LOSE_F2:1;                ///< pwm rising edge of right frame, please use MDrv_XC_InterruptAvaliable() to check if avalible or not
    MS_U8 INT_DVI_CK_LOSE_F1:1;                ///< pwm falling edge of right frame, please use MDrv_XC_InterruptAvaliable() to check if avalible or not
    MS_U8 INT_F2_CSOG		:1;                     ///< composite sync or SoG input signal type changed (for example, SoG signal from none -> valid, valid -> none)
    MS_U8 INT_F1_CSOG		:1;
    MS_U8 INT_F2_ATS_READY	:1;               ///< scaler dosen't have this interrupt now
    MS_U8 INT_F1_ATS_READY	:1;
    MS_U8 INT_F2_ATP_READY	:1;                ///< auto phase ready interrupt
    MS_U8 INT_F1_ATP_READY	:1;
    MS_U8 INT_F2_ATG_READY	:1;               ///< scaler dosen't have this interrupt now
    MS_U8 INT_F1_ATG_READY	:1;
}SCA_DISP_INTR_CONFIG;

typedef struct
{
    #define SCA_NUM 3
	SCA_DISP_INTR_CONFIG stIntrConfig[SCA_NUM];
	MS_BOOL 			 bIntrConfig_DIPW;
	MS_U32               u32DIPW_Signal_PID;
}SCA_DISP_INTR_SETTING;

typedef struct
{
    #define SCA_NUM 3
    SCA_DISP_INTR_CONFIG  stDisp_Intr_Status[SCA_NUM];
    MS_U8 u8DIPW_Status;
}SCA_DISP_INTR_STATUS;

typedef struct
{
    SCA_RW_REG_TYPE enType;
    MS_U32 u32addr;
    MS_U16 u16value;
    MS_U16 u16mask;
}SCA_RW_REGISTER_CONFIG, *PSCA_RW_REGISTER_CONFIG;

typedef struct
{
    SCA_DEST_TYPE enDestType;
    MS_U16 u16version;
    MS_U16 u16contrast;
    MS_U16 u16r_gain;
    MS_U16 u16g_gain;
    MS_U16 u16b_gain;
    MS_U16 u16saturation;
    MS_U16 u16hue;
    MS_U16 u16color_correct_xy_r;
    MS_U16 u16color_correct_xy_g;
    MS_U16 u16color_correct_xy_b;
    MS_U16 u16color_correct_offset_r;
    MS_U16 u16color_correct_offset_g;
    MS_U16 u16color_correct_offset_b;
}SCA_ACE_INFO_CONFIG, *PSCA_ACE_INFO_CONFIG;

typedef struct
{
    MS_U32 u32CmdBufAddr;
    MS_U16 u16CmdBufLen;
    MS_U16 u16DataLen;
    MS_U16 u16PNL_Width;
    MS_U16 u16PNL_Height;
}SCA_DLC_INFO_CONFIG, *PSCA_DLC_INFO_CONFIG;

typedef struct
{
    MS_BOOL bChanged;
    MS_BOOL bDisConnect;
    SCA_DISP_PRIORITY_TYPE enPriType;
    SCA_DEST_TYPE enDestType;
    SCA_SRC_TYPE enSrcType;
    MS_U32       dwUID;
}SCA_DISP_PATH_CONFIG, *PSCA_DISP_PATH_CONFIG;

typedef struct
{
    MS_BOOL bEn;
    SCA_DEST_TYPE enDestType;
}SCA_MIRROR_CONFIG, *PSCA_MIRROR_CONFIG;


typedef struct
{
    SCA_PQ_BIN_ID_TYPE enPQBinIDType;
    MS_U32 u32PQBin_Addr;
    MS_U32 u32PQBin_Size;
}SCA_PQ_BIN_INFO;

typedef struct
{
    SCA_PQ_BIN_INFO stPQBinInfo[SCA_PQ_BIN_ID_NUM];
}SCA_PQ_BIN_CONFIG, *PSCA_PQ_BIN_CONFIG;
typedef struct {
    MS_U16 u16value;
    MS_BOOL bnegative;//0=add ,1=minus
}SCA_PQ_adj;

typedef struct {
    MS_BOOL icc_en;
    SCA_PQ_adj icc_gain_R;
	SCA_PQ_adj icc_gain_G;
	SCA_PQ_adj icc_gain_B;
	SCA_PQ_adj icc_gain_Y;
	SCA_PQ_adj icc_gain_M;
	SCA_PQ_adj icc_gain_C;
	SCA_PQ_adj icc_gain_F;
	SCA_PQ_adj icc_gain_NC;
}SCA_PQ_ICC_CFG,*PSCA_PQ_ICC_CFG;

typedef struct {
    MS_BOOL ibc_en;
    SCA_PQ_adj ibc_gain_R;//RGB...no use positive
    SCA_PQ_adj ibc_gain_G;
	SCA_PQ_adj ibc_gain_B;
	SCA_PQ_adj ibc_gain_Y;
	SCA_PQ_adj ibc_gain_M;
	SCA_PQ_adj ibc_gain_C;
	SCA_PQ_adj ibc_gain_F;	
}SCA_PQ_IBC_CFG,*PSCA_PQ_IBC_CFG;

typedef struct {
    MS_BOOL ihc_en;
    SCA_PQ_adj ihc_gain_R;//RGB...=max3F+P/N
    SCA_PQ_adj ihc_gain_G;
	SCA_PQ_adj ihc_gain_B;
	SCA_PQ_adj ihc_gain_Y;
	SCA_PQ_adj ihc_gain_M;
	SCA_PQ_adj ihc_gain_C;
	SCA_PQ_adj ihc_gain_F;
}SCA_PQ_IHC_CFG,*PSCA_PQ_IHC_CFG;





typedef struct
{
    SCA_DIP_DEST_FMT_TYPE enDestFmtType;
    MS_BOOL bClipEn;
    SCA_WINDOW_TYPE stClipWin;
    MS_U8 u8FrameNum;
    MS_U32 u32BuffAddress;
    MS_U32 u32BuffSize;
    MS_U32 u32C_BuffAddress;
    MS_U32 u32C_BuffSize;
    MS_U16 u16Width;
    MS_U16 u16Height;
    MS_BOOL bTriggle;
    SCA_DIP_TRIGGER_TYPE enTrigMode;
}SCA_DIP_CONFIG, *PSCA_DIP_CONFIG;


typedef struct
{
    MS_U32 u32BuffAddress;
    MS_U32 u32C_BuffAddress;
    MS_BOOL bTrig;
}SCA_DIP_WONCE_BASE_CONFIG, *PSCA_DIP_WONCE_BASE_CONFIG;

typedef struct
{
    SCA_HDMITX_OUTPUT_TIMINE_TYPE enHDMITx_OutputTiming;
    SCA_HDMITX_MODE_TYPE enHDMITx_Mode;
    SCA_HDMITX_OUTPUT_COLOR_TYPE enHDMITx_ColorType;
    MS_BOOL bEn;
}SCA_HDMITX_CONFIG, *PSCA_HDMITX_CONFIG;

typedef struct
{
    SCA_DEST_TYPE enDestType;
    MS_BOOL bEn;
}SCA_FREEZE_CONFIG, *PSCA_FREEZE_CONFIG;

typedef struct
{
    MS_BOOL bFroceInit;
    MS_U32 u32MST701Bin_Addr;
    MS_U32 u32MST701Bin_Bytes;
}SCA_MST701_CONFIG, *PSCA_MST701_CONFIG;


typedef struct
{
    SCA_SRC_TYPE enSrcType;
    SCA_DEST_TYPE enDestType;
    MS_BOOL bSuccess;
    MS_U8   u8ModeIndex;
    MS_U16  u16Hstart;
    MS_U16 u16Vstart;
    MS_U16 u16Phase;
    MS_U16 u16Htotal;
}SCA_GEOMETRY_CALI_CONFIG, *PSCA_GEOMETRY_CALI_CONFIG;

typedef struct
{
    MS_U16 u16SableCnt;
    MS_U16 u16NoSyncCnt;
    MS_U32 u32PollingPeriod; //ms
}SCA_ANALOG_POLLING_CONFIG, *PSCA_ANALOG_POLLING_CONFIG;

typedef struct
{
    SCA_CLONE_SCREEN_TYPE enCloneScreenType;
}SCA_CLONE_SCREEN_CONFIG, *PSCA_CLONE_SCREEN_CONFIG;

typedef struct
{
    MS_U16 u16Modulation;
    MS_U16 u16Deviation;
    MS_BOOL bEn;
}SCA_PNL_SSC_CONFIG, *PSCA_PNL_SSC_CONFIG;

typedef struct
{
    MS_U16 u16Htt;
    MS_U16 u16H_Active;
    MS_U16 u16HSync_Width;
    MS_U16 u16HSync_Fporch;
    MS_U16 u16HSync_Bporch;
    MS_U16 u16Vtt;
    MS_U16 u16V_Active;
    MS_U16 u16VSync_Width;
    MS_U16 u16VSync_Fporch;
    MS_U16 u16VSync_Bporch;
    MS_U16 u16DCLK_MHz;
}SCA_PNL_TIMING_CONFIG, *PSCA_PNL_TIMING_CONFIG;


typedef struct
{
    SCA_DEST_TYPE enDestType;
    SCA_SRC_TYPE enSrcType;
    MS_BOOL bSetModeDone;
}SCA_MONITOR_STATUS_CONFIG, *PSCA_MONITOR_STATUS_CONFIG;

typedef struct
{
    SCA_SRC_TYPE enSrcType;
    MS_BOOL bEn;
    MS_U16 u16H_Overscan;
    MS_U16 u16V_Overscan;
    MS_S16 s16H_Offset;
    MS_S16 s16V_Offset;
    MS_BOOL bCVBS_NTSC;
}SCA_USER_DISPLAY_CONFIG, *PSCA_USER_DISPLAY_CONFIG;

typedef struct
{
    SCA_DEST_TYPE enDestType;
    SCA_SRC_TYPE enSrcType;
    MS_BOOL bEn;
    MS_U16 u16H_Overscan;
    MS_U16 u16V_Overscan;
    MS_S16 s16H_Offset;
    MS_S16 s16V_Offset;
    MS_BOOL bCVBS_NTSC;
}SCA_USER_DISPLAY_CONFIG_EX, *PSCA_USER_DISPLAY_CONFIG_EX;

typedef struct
{
    SCA_DEST_TYPE enDestType;
    SCA_DISPLAY_MUTE_COLOR_TYPE enColorType;
}SCA_DISPLAY_MUTE_COLOR_CONFIG, *PSCA_DISPLAY_MUTE_COLOR_CONFIG;

typedef struct
{
    MS_BOOL bEn;
    MS_U16 u16H_ratio;
    MS_U16 u16V_ratio;
}SCA_CLONE_SCREEN_DISPLAY_RATIO_CONFIG, *PSCA_CLONE_SCREEN_DISPLAY_RATIO_CONFIG;

typedef struct
{


    MS_U8 ucLumaCurve[16];
    MS_U8 ucLumaCurve2_a[16];
    MS_U8 ucLumaCurve2_b[16];
    MS_U8 ucDlcHistogramLimitCurve[17];

    MS_U8 u8_L_L_U;
    MS_U8 u8_L_L_D;
    MS_U8 u8_L_H_U;
    MS_U8 u8_L_H_D;
    MS_U8 u8_S_L_U;
    MS_U8 u8_S_L_D;
    MS_U8 u8_S_H_U;
    MS_U8 u8_S_H_D;

    MS_U8 ucDlcPureImageMode;
    MS_U8 ucDlcLevelLimit;
    MS_U8 ucDlcAvgDelta;
    MS_U8 ucDlcAvgDeltaStill;
    MS_U8 ucDlcFastAlphaBlending;
    MS_U8 ucDlcYAvgThresholdL;
    MS_U8 ucDlcYAvgThresholdH;
    MS_U8 ucDlcBLEPoint;
    MS_U8 ucDlcWLEPoint;
    MS_U8 bEnableBLE;
    MS_U8 bEnableWLE;

    MS_U8 ucDlcYAvgThresholdM;
    MS_U8 ucDlcCurveMode;
    MS_U8 ucDlcCurveModeMixAlpha;

    MS_U8 ucDlcAlgorithmMode;

    MS_U8 ucDlcSepPointH;
    MS_U8 ucDlcSepPointL;
    MS_U16 uwDlcBleStartPointTH;
    MS_U16 uwDlcBleEndPointTH;
    MS_U8 ucDlcCurveDiff_L_TH;
    MS_U8 ucDlcCurveDiff_H_TH;
    MS_U16 uwDlcBLESlopPoint_1;
    MS_U16 uwDlcBLESlopPoint_2;
    MS_U16 uwDlcBLESlopPoint_3;
    MS_U16 uwDlcBLESlopPoint_4;
    MS_U16 uwDlcBLESlopPoint_5;
    MS_U16 uwDlcDark_BLE_Slop_Min;
    MS_U8 ucDlcCurveDiffCoringTH;
    MS_U8 ucDlcAlphaBlendingMin;
    MS_U8 ucDlcAlphaBlendingMax;
    MS_U8 ucDlcFlicker_alpha;
    MS_U8 ucDlcYAVG_L_TH;
    MS_U8 ucDlcYAVG_H_TH;
    MS_U8 ucDlcDiffBase_L;
    MS_U8 ucDlcDiffBase_M;
    MS_U8 ucDlcDiffBase_H;

    MS_U8 bCGCCGainCtrl;
    MS_U8 ucCGCCGain_offset;
    MS_U8 ucCGCChroma_GainLimitH;
    MS_U8 ucCGCChroma_GainLimitL;
    MS_U8 ucCGCYCslope;
    MS_U8 ucCGCYth;
}SCA_DLC_INIT_CONFIG, *PSCA_DLC_INIT_CONFIG;


typedef struct
{
    SCA_DEST_TYPE enDestType;
    MS_BOOL bDLCOnOff;
    MS_BOOL bUserControl;
}SCA_DLC_ONOFF_CONFIG, *PSCA_DLC_ONOFF_CONFIG;

typedef struct
{
    MS_BOOL bEn;
    MS_U16  u16Size;
    MS_U8   u8Msg[64];
}SCA_UEVENT_CONFIG;

typedef struct
{
    MS_BOOL bEn;
}SCA_CVBSOUT_DAC_CONFIG, *PSCA_CVBSOUT_DAC_CONFIG;


typedef struct
{
    MS_U16  u16x;
    MS_U16  u16y;
    MS_U16  u16Width;
    MS_U16  u16Height;
    MS_U16  u16Vtotal;
    MS_U16  u16VFreq;
    MS_BOOL bInterlace;
}SCA_CAMERA_INPUTTIMING_CONFIG, *PSCA_CAMERA_INPUTTIMING_CONFIG;
#endif //_MDRV_GFLIP_IO_H
