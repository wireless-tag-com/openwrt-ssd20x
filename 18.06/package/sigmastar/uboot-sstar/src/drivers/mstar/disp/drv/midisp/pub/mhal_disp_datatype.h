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
 * \defgroup HAL_DISP_group  HAL_DISP driver
 * @{
 */
#ifndef __MHAL_DISP_DATATYPE_H__
#define __MHAL_DISP_DATATYPE_H__

// Device: Set/Get output to Interface: HDMI/VGA
#define MHAL_DISP_INTF_HDMI     (0x01L<<0)
#define MHAL_DISP_INTF_CVBS     (0x01L<<1)
#define MHAL_DISP_INTF_VGA      (0x01L<<2)
#define MHAL_DISP_INTF_YPBPR    (0x01L<<3)
#define MHAL_DISP_INTF_LCD      (0x01L<<4)

typedef enum
{
    E_MHAL_DISP_OUTPUT_PAL = 0,
    E_MHAL_DISP_OUTPUT_NTSC,
    E_MHAL_DISP_OUTPUT_960H_PAL,              /* ITU-R BT.1302 960 x 576 at 50 Hz (interlaced)*/
    E_MHAL_DISP_OUTPUT_960H_NTSC,             /* ITU-R BT.1302 960 x 480 at 60 Hz (interlaced)*/

    E_MHAL_DISP_OUTPUT_1080P24,
    E_MHAL_DISP_OUTPUT_1080P25,
    E_MHAL_DISP_OUTPUT_1080P30,

    E_MHAL_DISP_OUTPUT_720P50,
    E_MHAL_DISP_OUTPUT_720P60,
    E_MHAL_DISP_OUTPUT_1080I50,
    E_MHAL_DISP_OUTPUT_1080I60,
    E_MHAL_DISP_OUTPUT_1080P50,
    E_MHAL_DISP_OUTPUT_1080P60,

    E_MHAL_DISP_OUTPUT_576P50,
    E_MHAL_DISP_OUTPUT_480P60,

    E_MHAL_DISP_OUTPUT_640x480_60,            /* VESA 640 x 480 at 60 Hz (non-interlaced) CVT */
    E_MHAL_DISP_OUTPUT_800x600_60,            /* VESA 800 x 600 at 60 Hz (non-interlaced) */
    E_MHAL_DISP_OUTPUT_1024x768_60,           /* VESA 1024 x 768 at 60 Hz (non-interlaced) */
    E_MHAL_DISP_OUTPUT_1280x1024_60,          /* VESA 1280 x 1024 at 60 Hz (non-interlaced) */
    E_MHAL_DISP_OUTPUT_1366x768_60,           /* VESA 1366 x 768 at 60 Hz (non-interlaced) */
    E_MHAL_DISP_OUTPUT_1440x900_60,           /* VESA 1440 x 900 at 60 Hz (non-interlaced) CVT Compliant */
    E_MHAL_DISP_OUTPUT_1280x800_60,           /* 1280*800@60Hz VGA@60Hz*/
    E_MHAL_DISP_OUTPUT_1680x1050_60,          /* VESA 1680 x 1050 at 60 Hz (non-interlaced) */
    E_MHAL_DISP_OUTPUT_1920x2160_30,          /* 1920x2160_30 */
    E_MHAL_DISP_OUTPUT_1600x1200_60,          /* VESA 1600 x 1200 at 60 Hz (non-interlaced) */
    E_MHAL_DISP_OUTPUT_1920x1200_60,          /* VESA 1920 x 1600 at 60 Hz (non-interlaced) CVT (Reduced Blanking)*/
    E_MHAL_DISP_OUTPUT_2560x1440_30,          /* 2560x1440_30 */
    E_MHAL_DISP_OUTPUT_2560x1600_60,          /* 2560x1600_60 */
    E_MHAL_DISP_OUTPUT_3840x2160_30,          /* 3840x2160_30 */
    E_MHAL_DISP_OUTPUT_3840x2160_60,          /* 3840x2160_60 */
    E_MHAL_DISP_OUTPUT_USER,
    E_MHAL_DISP_OUTPUT_MAX,
} MHAL_DISP_DeviceTiming_e;

typedef enum
{
    E_MHAL_DISP_CSC_MATRIX_BYPASS = 0,         /* do not change color space */

    E_MHAL_DISP_CSC_MATRIX_BT601_TO_BT709,       /* change color space from BT.601 to BT.709 */
    E_MHAL_DISP_CSC_MATRIX_BT709_TO_BT601,       /* change color space from BT.709 to BT.601 */

    E_MHAL_DISP_CSC_MATRIX_BT601_TO_RGB_PC,      /* change color space from BT.601 to RGB */
    E_MHAL_DISP_CSC_MATRIX_BT709_TO_RGB_PC,      /* change color space from BT.709 to RGB */

    E_MHAL_DISP_CSC_MATRIX_RGB_TO_BT601_PC,      /* change color space from RGB to BT.601 */
    E_MHAL_DISP_CSC_MATRIX_RGB_TO_BT709_PC,      /* change color space from RGB to BT.709 */

    E_MHAL_DISP_CSC_MATRIX_MAX
} MHAL_DISP_CscMattrix_e;

typedef enum
{
    E_MHAL_DISP_PIXEL_FRAME_YUV422_YUYV = 0,
    E_MHAL_DISP_PIXEL_FRAME_ARGB8888,
    E_MHAL_DISP_PIXEL_FRAME_ABGR8888,

    E_MHAL_DISP_PIXEL_FRAME_RGB565,
    E_MHAL_DISP_PIXEL_FRAME_ARGB1555,
    E_MHAL_DISP_PIXEL_FRAME_I2,
    E_MHAL_DISP_PIXEL_FRAME_I4,
    E_MHAL_DISP_PIXEL_FRAME_I8,

    E_MHAL_DISP_PIXEL_FRAME_YUV_SEMIPLANAR_422,
    E_MHAL_DISP_PIXEL_FRAME_YUV_SEMIPLANAR_420,
    // mdwin/mgwin
    E_MHAL_DISP_PIXEL_FRAME_YUV_MST_420,

    //vdec private video format
    E_MHAL_DISP_PIXEL_FRAME_YC420_MSTTILE1_H264,
    E_MHAL_DISP_PIXEL_FRAME_YC420_MSTTILE2_H265,
    E_MHAL_DISP_PIXEL_FRAME_YC420_MSTTILE3_H265,
    E_MHAL_DISP_PIXEL_FRAME_FORMAT_MAX,
} MHAL_DISP_PixelFormat_e;

typedef enum
{
    E_MHAL_DISP_COMPRESS_MODE_NONE,//no compress
    E_MHAL_DISP_COMPRESS_MODE_SEG,//compress unit is 256 bytes as a segment
    E_MHAL_DISP_COMPRESS_MODE_LINE,//compress unit is the whole line
    E_MHAL_DISP_COMPRESS_MODE_FRAME,//compress unit is the whole frame
    E_MHAL_DISP_COMPRESS_MODE_MAX, //number
} MHAL_DISP_PixelCompressMode_e;

typedef enum
{
    E_MHAL_DISP_TILE_MODE_NONE     = 0x00,
    E_MHAL_DISP_TILE_MODE_16x16    = 0x01,
    E_MHAL_DISP_TILE_MODE_16x32    = 0x02,
    E_MHAL_DISP_TILE_MODE_32x16    = 0x03,
    E_MHAL_DISP_TILE_MODE_32x32    = 0x04,
} MHAL_DISP_TileMode_e;

typedef enum
{
    E_MHAL_DISP_MMAP_XC_MAIN = 0,         /* XC Main buffer */
    E_MHAL_DISP_MMAP_XC_MENULOAD,       /* XC Menuload buffer */
    E_MHAL_DISP_MMAP_MAX,
} MHAL_DISP_MmapType_e;

typedef enum
{
    E_MHAL_DISP_ROTATE_NONE,
    E_MHAL_DISP_ROTATE_90,
    E_MHAL_DISP_ROTATE_180,
    E_MHAL_DISP_ROTATE_270,
    E_MHAL_DISP_ROTATE_NUM,
}MHAL_DISP_RorateMode_e;

typedef enum
{
    E_MHAL_DISP_TIMEZONE_UNKONWN    = 0x00,
    E_MHAL_DISP_TIMEZONE_SYNC       = 0x01,
    E_MHAL_DISP_TIMEZONE_BACKPORCH  = 0x02,
    E_MHAL_DISP_TIMEZONE_ACTIVE     = 0x03,
    E_MHAL_DISP_TIMEZONE_FRONTPORCH = 0x04,
}MHAL_DISP_TimeZoneType_e;

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct MHAL_DISP_MmapInfo_s
{
    MS_U8     u8Gid;                         // Mmap ID
    MS_U8     u8Layer;                       // Memory Layer
    MS_U8     u8MiuNo;                       // 0: MIU0 / 1: MIU1 / 2: MIU2
    MS_U8     u8CMAHid;                      // Memory CMAHID
    MS_U32    u32Addr;                       // Memory Address
    MS_U32    u32Size;                       // Memory Size
    MS_U32    u32Align;                      // Memory Align
    MS_U32    u32MemoryType;                 // Memory Type
}MHAL_DISP_MmapInfo_t;

/// Define PANEL Signaling Type
typedef enum
{
    E_MHAL_LINK_TTL,                              ///< TTL  type
    E_MHAL_LINK_LVDS,                             ///< LVDS type
    E_MHAL_LINK_RSDS,                             ///< RSDS type
    E_MHAL_LINK_MINILVDS,                         ///< TCON
    E_MHAL_LINK_ANALOG_MINILVDS,                  ///< Analog TCON
    E_MHAL_LINK_DIGITAL_MINILVDS,                 ///< Digital TCON
    E_MHAL_LINK_MFC,                              ///< Ursa (TTL output to Ursa)
    E_MHAL_LINK_DAC_I,                            ///< DAC output
    E_MHAL_LINK_DAC_P,                            ///< DAC output
    E_MHAL_LINK_PDPLVDS,                          ///< For PDP(Vsync use Manually MODE)
    E_MHAL_LINK_EXT,                              /// EXT LPLL TYPE
}MHAL_DISP_ApiPnlLinkType_e;

typedef enum
{
    E_MHAL_ASPECT_RATIO_4_3    = 0,         ///< set aspect ratio to 4 : 3
    E_MHAL_ASPECT_RATIO_WIDE,               ///< set aspect ratio to 16 : 9
    E_MHAL_ASPECT_RATIO_OTHER,              ///< resvered for other aspect ratio other than 4:3/ 16:9
}MHAL_DISP_PnlAspectRatio_e;

/// Define TI bit mode
typedef enum
{
    E_MHAL_TI_10BIT_MODE = 0,
    E_MHAL_TI_8BIT_MODE = 2,
    E_MHAL_TI_6BIT_MODE = 3,
} MHAL_DISP_ApiPnlTiBitMode_e;

/// Define which panel output timing change mode is used to change VFreq for same panel
typedef enum
{
    E_MHAL_CHG_DCLK   = 0,      ///<change output DClk to change Vfreq.
    E_MHAL_CHG_HTOTAL = 1,      ///<change H total to change Vfreq.
    E_MHAL_CHG_VTOTAL = 2,      ///<change V total to change Vfreq.
} MHAL_DISP_ApiPnlOutTimingMode_e;
/// Define panel output format bit mode
typedef enum
{
    E_MHAL_OUTPUT_10BIT_MODE = 0,//default is 10bit, becasue 8bit panel can use 10bit config and 8bit config.
    E_MHAL_OUTPUT_6BIT_MODE = 1, //but 10bit panel(like PDP panel) can only use 10bit config.
    E_MHAL_OUTPUT_8BIT_MODE = 2, //and some PDA panel is 6bit.
} MHAL_DISP_ApiPnlOutPutFormatBitMode_e;

typedef struct __attribute__((packed))
{
    const char *m_pPanelName;                ///<  PanelName
#if !(defined(UFO_PUBLIC_HEADER_212) || defined(UFO_PUBLIC_HEADER_300))
#if !defined (__aarch64__)
    MS_U32 u32AlignmentDummy0;
#endif
#endif
    //
    //  Panel output
    //
    MS_U8 m_bPanelDither :1;                 ///<  PANEL_DITHER, keep the setting
    MHAL_DISP_ApiPnlLinkType_e m_ePanelLinkType   :4;  ///<  PANEL_LINK

    ///////////////////////////////////////////////
    // Board related setting
    ///////////////////////////////////////////////
    MS_U8 m_bPanelDualPort  :1;              ///<  VOP_21[8], MOD_4A[1],    PANEL_DUAL_PORT, refer to m_bPanelDoubleClk
    MS_U8 m_bPanelSwapPort  :1;              ///<  MOD_4A[0],               PANEL_SWAP_PORT, refer to "LVDS output app note" A/B channel swap
    MS_U8 m_bPanelSwapOdd_ML    :1;          ///<  PANEL_SWAP_ODD_ML
    MS_U8 m_bPanelSwapEven_ML   :1;          ///<  PANEL_SWAP_EVEN_ML
    MS_U8 m_bPanelSwapOdd_RB    :1;          ///<  PANEL_SWAP_ODD_RB
    MS_U8 m_bPanelSwapEven_RB   :1;          ///<  PANEL_SWAP_EVEN_RB

    MS_U8 m_bPanelSwapLVDS_POL  :1;          ///<  MOD_40[5], PANEL_SWAP_LVDS_POL, for differential P/N swap
    MS_U8 m_bPanelSwapLVDS_CH   :1;          ///<  MOD_40[6], PANEL_SWAP_LVDS_CH, for pair swap
    MS_U8 m_bPanelPDP10BIT      :1;          ///<  MOD_40[3], PANEL_PDP_10BIT ,for pair swap
    MS_U8 m_bPanelLVDS_TI_MODE  :1;          ///<  MOD_40[2], PANEL_LVDS_TI_MODE, refer to "LVDS output app note"

    ///////////////////////////////////////////////
    // For TTL Only
    ///////////////////////////////////////////////
    MS_U8 m_ucPanelDCLKDelay;                ///<  PANEL_DCLK_DELAY
    MS_U8 m_bPanelInvDCLK   :1;              ///<  MOD_4A[4],                   PANEL_INV_DCLK
    MS_U8 m_bPanelInvDE     :1;              ///<  MOD_4A[2],                   PANEL_INV_DE
    MS_U8 m_bPanelInvHSync  :1;              ///<  MOD_4A[12],                  PANEL_INV_HSYNC
    MS_U8 m_bPanelInvVSync  :1;              ///<  MOD_4A[3],                   PANEL_INV_VSYNC

    ///////////////////////////////////////////////
    // Output driving current setting
    ///////////////////////////////////////////////
    // driving current setting (0x00=4mA, 0x01=6mA, 0x02=8mA, 0x03=12mA)
    MS_U8 m_ucPanelDCKLCurrent;              ///<  define PANEL_DCLK_CURRENT
    MS_U8 m_ucPanelDECurrent;                ///<  define PANEL_DE_CURRENT
    MS_U8 m_ucPanelODDDataCurrent;           ///<  define PANEL_ODD_DATA_CURRENT
    MS_U8 m_ucPanelEvenDataCurrent;          ///<  define PANEL_EVEN_DATA_CURRENT

    ///////////////////////////////////////////////
    // panel on/off timing
    ///////////////////////////////////////////////
    MS_U16 m_wPanelOnTiming1;                ///<  time between panel & data while turn on power
    MS_U16 m_wPanelOnTiming2;                ///<  time between data & back light while turn on power
    MS_U16 m_wPanelOffTiming1;               ///<  time between back light & data while turn off power
    MS_U16 m_wPanelOffTiming2;               ///<  time between data & panel while turn off power

    ///////////////////////////////////////////////
    // panel timing spec.
    ///////////////////////////////////////////////
    // sync related
    MS_U16 m_ucPanelHSyncWidth;               ///<  VOP_01[7:0], PANEL_HSYNC_WIDTH
    MS_U16 m_ucPanelHSyncBackPorch;           ///<  PANEL_HSYNC_BACK_PORCH, no register setting, provide value for query only

                                             ///<  not support Manuel VSync Start/End now
                                             ///<  VOP_02[10:0] VSync start = Vtt - VBackPorch - VSyncWidth
                                             ///<  VOP_03[10:0] VSync end = Vtt - VBackPorch
    MS_U16 m_ucPanelVSyncWidth;               ///<  define PANEL_VSYNC_WIDTH
    MS_U16 m_ucPanelVBackPorch;               ///<  define PANEL_VSYNC_BACK_PORCH

    // DE related
    MS_U16 m_wPanelHStart;                   ///<  VOP_04[11:0], PANEL_HSTART, DE H Start (PANEL_HSYNC_WIDTH + PANEL_HSYNC_BACK_PORCH)
    MS_U16 m_wPanelVStart;                   ///<  VOP_06[11:0], PANEL_VSTART, DE V Start
    MS_U16 m_wPanelWidth;                    ///< PANEL_WIDTH, DE width (VOP_05[11:0] = HEnd = HStart + Width - 1)
    MS_U16 m_wPanelHeight;                   ///< PANEL_HEIGHT, DE height (VOP_07[11:0], = Vend = VStart + Height - 1)

    // DClk related
    MS_U16 m_wPanelMaxHTotal;                ///<  PANEL_MAX_HTOTAL. Reserved for future using.
    MS_U16 m_wPanelHTotal;                   ///<  VOP_0C[11:0], PANEL_HTOTAL
    MS_U16 m_wPanelMinHTotal;                ///<  PANEL_MIN_HTOTAL. Reserved for future using.

    MS_U16 m_wPanelMaxVTotal;                ///<  PANEL_MAX_VTOTAL. Reserved for future using.
    MS_U16 m_wPanelVTotal;                   ///<  VOP_0D[11:0], PANEL_VTOTAL
    MS_U16 m_wPanelMinVTotal;                ///<  PANEL_MIN_VTOTAL. Reserved for future using.

    MS_U8 m_dwPanelMaxDCLK;                  ///<  PANEL_MAX_DCLK. Reserved for future using.
    MS_U8 m_dwPanelDCLK;                     ///<  LPLL_0F[23:0], PANEL_DCLK          ,{0x3100_10[7:0], 0x3100_0F[15:0]}
    MS_U8 m_dwPanelMinDCLK;                  ///<  PANEL_MIN_DCLK. Reserved for future using.
	MS_U16 m_wPanelVFreqx10;				 ///<  frame rate

                                             ///<  spread spectrum
    MS_U16 m_wSpreadSpectrumStep;            ///<  move to board define, no use now.
    MS_U16 m_wSpreadSpectrumSpan;            ///<  move to board define, no use now.

    MS_U8 m_ucDimmingCtl;                    ///<  Initial Dimming Value
    MS_U8 m_ucMaxPWMVal;                     ///<  Max Dimming Value
    MS_U8 m_ucMinPWMVal;                     ///<  Min Dimming Value

    MS_U8 m_bPanelDeinterMode   :1;          ///<  define PANEL_DEINTER_MODE,  no use now
    MHAL_DISP_PnlAspectRatio_e m_ucPanelAspectRatio; ///<  Panel Aspect Ratio, provide information to upper layer application for aspect ratio setting.
  /*
    *
    * Board related params
    *
    *  If a board ( like BD_MST064C_D01A_S ) swap LVDS TX polarity
    *    : This polarity swap value =
    *      (LVDS_PN_SWAP_H<<8) | LVDS_PN_SWAP_L from board define,
    *  Otherwise
    *    : The value shall set to 0.
    */
    MS_U16 m_u16LVDSTxSwapValue;
    MHAL_DISP_ApiPnlTiBitMode_e m_ucTiBitMode;                         ///< MOD_4B[1:0], refer to "LVDS output app note"
    MHAL_DISP_ApiPnlOutPutFormatBitMode_e m_ucOutputFormatBitMode;

    MS_U8 m_bPanelSwapOdd_RG    :1;          ///<  define PANEL_SWAP_ODD_RG
    MS_U8 m_bPanelSwapEven_RG   :1;          ///<  define PANEL_SWAP_EVEN_RG
    MS_U8 m_bPanelSwapOdd_GB    :1;          ///<  define PANEL_SWAP_ODD_GB
    MS_U8 m_bPanelSwapEven_GB   :1;          ///<  define PANEL_SWAP_EVEN_GB

    /**
    *  Others
    */
    MS_U8 m_bPanelDoubleClk     :1;             ///<  LPLL_03[7], define Double Clock ,LVDS dual mode
    MS_U32 m_dwPanelMaxSET;                     ///<  define PANEL_MAX_SET
    MS_U32 m_dwPanelMinSET;                     ///<  define PANEL_MIN_SET
    MHAL_DISP_ApiPnlOutTimingMode_e m_ucOutTimingMode;   ///<Define which panel output timing change mode is used to change VFreq for same panel
    MS_U8 m_bPanelNoiseDith     :1;             ///<  PAFRC mixed with noise dither disable
} MHAL_DISP_PanelType_t;

typedef struct MHAL_DISP_PanelConfig_s
{
    MHAL_DISP_PanelType_t stPanelAttr;
    MHAL_DISP_DeviceTiming_e eTiming;
    MS_U32 u32OutputDev;
    MS_BOOL bValid;
}MHAL_DISP_PanelConfig_t;

typedef struct MHAL_DISP_SyncInfo_s
{
    MS_BOOL  bSynm;     /* sync mode(0:timing,as BT.656; 1:signal,as LCD) */
    MS_BOOL  bIop;      /* interlaced or progressive display(0:i; 1:p) */
    MS_U8    u8Intfb;   /* interlace bit width while output */

    MS_U16  u16Vact ;  /* vertical active area */
    MS_U16  u16Vbb;    /* vertical back blank porch */
    MS_U16  u16Vfb;    /* vertical front blank porch */

    MS_U16  u16Hact;   /* herizontal active area */
    MS_U16  u16Hbb;    /* herizontal back blank porch */
    MS_U16  u16Hfb;    /* herizontal front blank porch */
    MS_U16  u16Hmid;   /* bottom herizontal active area */

    MS_U16  u16Bvact;  /* bottom vertical active area */
    MS_U16  u16Bvbb;   /* bottom vertical back blank porch */
    MS_U16  u16Bvfb;   /* bottom vertical front blank porch */

    MS_U16  u16Hpw;    /* horizontal pulse width */
    MS_U16  u16Vpw;    /* vertical pulse width */

    MS_BOOL  bIdv;      /* inverse data valid of output */
    MS_BOOL  bIhs;      /* inverse horizontal synch signal */
    MS_BOOL  bIvs;      /* inverse vertical synch signal */
    MS_U32   u32FrameRate;
} MHAL_DISP_SyncInfo_t;

typedef struct
{
    MHAL_DISP_DeviceTiming_e eTimeType;
    MHAL_DISP_SyncInfo_t     *pstSyncInfo; // Just for E_HAL_DISP_OUTPUT_USER
} MHAL_DISP_DeviceTimingInfo_t;

typedef struct
{
    MHAL_DISP_CscMattrix_e eCscMatrix;
    MS_U32 u32Luma;                     /* luminance:   0 ~ 100 default: 50 */
    MS_U32 u32Contrast;                 /* contrast :   0 ~ 100 default: 50 */
    MS_U32 u32Hue;                      /* hue      :   0 ~ 100 default: 50 */
    MS_U32 u32Saturation;               /* saturation:  0 ~ 100 default: 50 */
} MHAL_DISP_Csc_t;

typedef struct
{
    MHAL_DISP_Csc_t stCsc;                     /* color space */
    MS_U32 u32Gain;                          /* current gain of VGA signals. [0, 64). default:0x30 */
    MS_U32 u32Sharpness;
} MHAL_DISP_VgaParam_t;

typedef struct
{
    MHAL_DISP_Csc_t stCsc;                     /* color space */
    MS_U32 u32Sharpness;
} MHAL_DISP_HdmiParam_t;

typedef struct
{
    MS_BOOL bEnable;                     /* color space */
} MHAL_DISP_CvbsParam_t;


typedef struct
{
    MS_U16 u16X;
    MS_U16 u16Y;
    MS_U16 u16Width;
    MS_U16 u16Height;
} MHAL_DISP_VidWinRect_t;

typedef struct
{
    MS_U32 u32Width;
    MS_U32 u32Height;
} MHAL_DISP_VideoLayerSize_t;

typedef struct
{
    MHAL_DISP_VidWinRect_t  stVidLayerDispWin;   /* Display resolution */
    MHAL_DISP_VideoLayerSize_t  stVidLayerSize;      /* Canvas size of the video layer */
    MHalPixelFormat_e ePixFormat;         /* Pixel format of the video layer */
} MHAL_DISP_VideoLayerAttr_t;

typedef struct
{
    MHAL_DISP_VidWinRect_t stDispWin;                     /* rect of video out chn */
    MS_U16 u16SrcWidth;
    MS_U16 u16SrcHeight;
} MHAL_DISP_InputPortAttr_t;

typedef  struct  MHAL_DISP_VideoFrameData_s
{
    MHalPixelFormat_e  ePixelFormat;
    MHAL_DISP_PixelCompressMode_e eCompressMode;

    MHAL_DISP_TileMode_e eTileMode;

    MS_PHYADDR    aPhyAddr[3];
    MS_U32 au32Stride[3];
} MHAL_DISP_VideoFrameData_t;
//MI_S32 mi_sys_MMA_Alloc(MI_U8 *u8MMAHeapName, MI_U32 u32blkSize ,MI_PHY *phyAddr);
//MI_S32 mi_sys_MMA_Free(MI_PHY phyAddr);

typedef struct {
    // Success return 0
    MS_S32 (*alloc)(MS_U8 *pu8Name, MS_U32 size, unsigned long long *pu64PhyAddr);
    // Success return 0
    MS_S32 (*free)(unsigned long long u64PhyAddr);
} MHAL_DISP_AllocPhyMem_t;

typedef struct
{
    MHAL_DISP_RorateMode_e enRotate;
}MHAL_DISP_RotateConfig_t;

typedef struct
{
    MS_BOOL bEnbale;                     /* Enable/Disable Compress */
} MHAL_DISP_CompressAttr_t;

typedef struct
{
    MS_U32 u32IrqMask;
    MS_U32 u32IrqFlag;
}MHAL_DISP_IRQFlag_t;

typedef struct
{
    MS_U16 u16RedOffset;
    MS_U16 u16GreenOffset;
    MS_U16 u16BlueOffset;

    MS_U16 u16RedColor;  // 00~FF, 0x80 is no change
    MS_U16 u16GreenColor;// 00~FF, 0x80 is no change
    MS_U16 u16BlueColor; // 00~FF, 0x80 is no change
}MHAL_DISP_ColorTempeture_t;

typedef struct
{
    MHAL_DISP_Csc_t stCsc;                   /* color space */
    MS_U32 u32Sharpness;
}MHAL_DISP_LcdParam_t;

typedef struct
{
    MS_BOOL   bEn;
    MS_U16 u16EntryNum;
    MS_U8 *pu8ColorR;
    MS_U8 *pu8ColorG;
    MS_U8 *pu8ColorB;
}MHAL_DISP_GammaParam_t;


typedef struct
{
    MHAL_DISP_TimeZoneType_e enType;
}MHAL_DISP_TimeZone_t;


typedef struct
{
    MS_U16 u16Htotal;
    MS_U16 u16Vtotal;
    MS_U16 u16HdeStart;
    MS_U16 u16VdeStart;
    MS_U16 u16Width;
    MS_U16 u16Height;
    MS_BOOL bInterlaceMode;
    MS_BOOL bYuvOutput;
}MHAL_DISP_DisplayInfo_t;

#endif
