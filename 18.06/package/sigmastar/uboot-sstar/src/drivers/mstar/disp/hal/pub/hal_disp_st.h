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

#ifndef _HAL_DISP_ST_H_
#define _HAL_DISP_ST_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------
#define HAL_DISP_INTF_HDMI     (0x01L<<0)
#define HAL_DISP_INTF_CVBS     (0x01L<<1)
#define HAL_DISP_INTF_VGA      (0x01L<<2)
#define HAL_DISP_INTF_YPBPR    (0x01L<<3)
#define HAL_DISP_INTF_LCD      (0x01L<<4)

//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_HAL_DISP_QUERY_RET_OK = 0,
    E_HAL_DISP_QUERY_RET_CFGERR,
    E_HAL_DISP_QUERY_RET_NOTSUPPORT,
    E_HAL_DISP_QUERY_RET_NONEED,
}HalDispQueryRet_e;


typedef enum
{
    E_HAL_DISP_QUERY_DEVICE_INIT,
    E_HAL_DISP_QUERY_DEVICE_ENABLE,
    E_HAL_DISP_QUERY_DEVICE_ATTACH,
    E_HAL_DISP_QUERY_DEVICE_DETACH,
    E_HAL_DISP_QUERY_DEVICE_BACKGROUND_COLOR,
    E_HAL_DISP_QUERY_DEVICE_INTERFACE,
    E_HAL_DISP_QUERY_DEVICE_OUTPUTTIMING,
    E_HAL_DISP_QUERY_DEVICE_VGA_PARAM,
    E_HAL_DISP_QUERY_DEVICE_COLORTEMP,
    E_HAL_DISP_QUERY_DEVICE_CVBS_PARAM,
    E_HAL_DISP_QUERY_DEVICE_HDMI_PARAM,
    E_HAL_DISP_QUERY_DEVICE_LCD_PARAM,
    E_HAL_DISP_QUERY_DEVICE_GAMMA_PARAM,
    E_HAL_DISP_QUERY_DEVICE_TIME_ZONE,
    E_HAL_DISP_QUERY_DEVICE_DISPLAY_INFO,
    E_HAL_DISP_QUERY_VIDEOLAYER_INIT,
    E_HAL_DISP_QUERY_VIDEOLAYER_ENABLE,
    E_HAL_DISP_QUERY_VIDEOLAYER_BIND,
    E_HAL_DISP_QUERY_VIDEOLAYER_UNBIND,
    E_HAL_DISP_QUERY_VIDEOLAYER_ATTR,
    E_HAL_DISP_QUERY_VIDEOLAYER_COMPRESS,
    E_HAL_DISP_QUERY_VIDEOLAYER_PRIORITY,
    E_HAL_DISP_QUERY_VIDEOLAYER_BUFFER_FIRE,
    E_HAL_DISP_QUERY_VIDEOLAYER_CHECK_FIRE,
    E_HAL_DISP_QUERY_INPUTPORT_INIT,
    E_HAL_DISP_QUERY_INPUTPORT_ENABLE,
    E_HAL_DISP_QUERY_INPUTPORT_ATTR,
    E_HAL_DISP_QUERY_INPUTPORT_SHOW,
    E_HAL_DISP_QUERY_INPUTPORT_HIDE,
    E_HAL_DISP_QUERY_INPUTPORT_BEGIN,
    E_HAL_DISP_QUERY_INPUTPORT_END,
    E_HAL_DISP_QUERY_INPUTPORT_FLIP,
    E_HAL_DISP_QUERY_INPUTPORT_ROTATE,
    E_HAL_DISP_QUERY_INPUTPORT_CROP,
    E_HAL_DISP_QUERY_CLK_SET,
    E_HAL_DISP_QUERY_CLK_GET,
    E_HAL_DISP_QUERY_MAX,
}HalDispQueryType_e;

typedef enum
{
    E_HAL_DISP_OUTPUT_PAL = 0,
    E_HAL_DISP_OUTPUT_NTSC,
    E_HAL_DISP_OUTPUT_960H_PAL,              /* ITU-R BT.1302 960 x 576 at 50 Hz (interlaced)*/
    E_HAL_DISP_OUTPUT_960H_NTSC,             /* ITU-R BT.1302 960 x 480 at 60 Hz (interlaced)*/

    E_HAL_DISP_OUTPUT_1080P24,
    E_HAL_DISP_OUTPUT_1080P25,
    E_HAL_DISP_OUTPUT_1080P30,

    E_HAL_DISP_OUTPUT_720P50,
    E_HAL_DISP_OUTPUT_720P60,
    E_HAL_DISP_OUTPUT_1080I50,
    E_HAL_DISP_OUTPUT_1080I60,
    E_HAL_DISP_OUTPUT_1080P50,
    E_HAL_DISP_OUTPUT_1080P60,

    E_HAL_DISP_OUTPUT_576P50,
    E_HAL_DISP_OUTPUT_480P60,

    E_HAL_DISP_OUTPUT_640x480_60,            /* VESA 640 x 480 at 60 Hz (non-interlaced) CVT */
    E_HAL_DISP_OUTPUT_800x600_60,            /* VESA 800 x 600 at 60 Hz (non-interlaced) */
    E_HAL_DISP_OUTPUT_1024x768_60,           /* VESA 1024 x 768 at 60 Hz (non-interlaced) */
    E_HAL_DISP_OUTPUT_1280x1024_60,          /* VESA 1280 x 1024 at 60 Hz (non-interlaced) */
    E_HAL_DISP_OUTPUT_1366x768_60,           /* VESA 1366 x 768 at 60 Hz (non-interlaced) */
    E_HAL_DISP_OUTPUT_1440x900_60,           /* VESA 1440 x 900 at 60 Hz (non-interlaced) CVT Compliant */
    E_HAL_DISP_OUTPUT_1280x800_60,           /* 1280*800@60Hz VGA@60Hz*/
    E_HAL_DISP_OUTPUT_1680x1050_60,          /* VESA 1680 x 1050 at 60 Hz (non-interlaced) */
    E_HAL_DISP_OUTPUT_1920x2160_30,          /* 1920x2160_30 */
    E_HAL_DISP_OUTPUT_1600x1200_60,          /* VESA 1600 x 1200 at 60 Hz (non-interlaced) */
    E_HAL_DISP_OUTPUT_1920x1200_60,          /* VESA 1920 x 1600 at 60 Hz (non-interlaced) CVT (Reduced Blanking)*/
    E_HAL_DISP_OUTPUT_2560x1440_30,          /* 2560x1440_30 */
    E_HAL_DISP_OUTPUT_2560x1600_60,          /* 2560x1600_60 */
    E_HAL_DISP_OUTPUT_3840x2160_30,          /* 3840x2160_30 */
    E_HAL_DISP_OUTPUT_3840x2160_60,          /* 3840x2160_60 */
    E_HAL_DISP_OUTPUT_USER,
    E_HAL_DISP_OUTPUT_MAX,
} HalDispDeviceTiming_e;


typedef enum
{
    E_HAL_DISP_CSC_MATRIX_BYPASS = 0,         /* do not change color space */

    E_HAL_DISP_CSC_MATRIX_BT601_TO_BT709,       /* change color space from BT.601 to BT.709 */
    E_HAL_DISP_CSC_MATRIX_BT709_TO_BT601,       /* change color space from BT.709 to BT.601 */

    E_HAL_DISP_CSC_MATRIX_BT601_TO_RGB_PC,      /* change color space from BT.601 to RGB */
    E_HAL_DISP_CSC_MATRIX_BT709_TO_RGB_PC,      /* change color space from BT.709 to RGB */

    E_HAL_DISP_CSC_MATRIX_RGB_TO_BT601_PC,      /* change color space from RGB to BT.601 */
    E_HAL_DISP_CSC_MATRIX_RGB_TO_BT709_PC,      /* change color space from RGB to BT.709 */

    E_HAL_DISP_CSC_MATRIX_MAX
} HalDispCscmatrix_e;

typedef enum
{
    E_HAL_DISP_PIXEL_FRAME_YUV422_YUYV = 0,
    E_HAL_DISP_PIXEL_FRAME_ARGB8888,
    E_HAL_DISP_PIXEL_FRAME_ABGR8888,
    E_HAL_DISP_PIXEL_FRAME_BGRA8888,

    E_HAL_DISP_PIXEL_FRAME_RGB565,
    E_HAL_DISP_PIXEL_FRAME_ARGB1555,
    E_HAL_DISP_PIXEL_FRAME_ARGB4444,

    E_HAL_DISP_PIXEL_FRAME_YUV_SEMIPLANAR_422,
    E_HAL_DISP_PIXEL_FRAME_YUV_SEMIPLANAR_420,
    // mdwin/mgwin
    E_HAL_DISP_PIXEL_FRAME_YUV_MST_420,

    E_HAL_DISP_PIXEL_FRAME_FORMAT_MAX,
} HalDispPixelFormat_e;


typedef enum
{
    E_HAL_DISP_COMPRESS_MODE_NONE,//no compress
    E_HAL_DISP_COMPRESS_MODE_SEG,//compress unit is 256 bytes as a segment
    E_HAL_DISP_COMPRESS_MODE_LINE,//compress unit is the whole line
    E_HAL_DISP_COMPRESS_MODE_FRAME,//compress unit is the whole frame
    E_HAL_DISP_COMPRESS_MODE_MAX, //number
} HalDispPixelCompressMode_e;


typedef enum
{
    E_HAL_DISP_TILE_MODE_NONE     = 0x00,
    E_HAL_DISP_TILE_MODE_16x16    = 0x01,
    E_HAL_DISP_TILE_MODE_16x32    = 0x02,
    E_HAL_DISP_TILE_MODE_32x16    = 0x03,
    E_HAL_DISP_TILE_MODE_32x32    = 0x04,
} HalDispTileMode_e;

typedef enum
{
    E_HAL_DISP_MMAP_XC_MAIN = 0,         /* XC Main buffer */
    E_HAL_DISP_MMAP_XC_MENULOAD,       /* XC Menuload buffer */
    E_HAL_DISP_MMAP_MAX,
} HalDispMmapType_e;

typedef enum
{
    E_HAL_DISP_ROTATE_NONE,
    E_HAL_DISP_ROTATE_90,
    E_HAL_DISP_ROTATE_180,
    E_HAL_DISP_ROTATE_270,
    E_HAL_DISP_ROTATE_NUM,
} HalDispRotateMode_e;

typedef enum
{
    E_HAL_DISP_TIMEZONE_NONE       = 0x00,
    E_HAL_DISP_TIMEZONE_SYNC       = 0x01,
    E_HAL_DISP_TIMEZONE_BACKPORCH  = 0x02,
    E_HAL_DISP_TIMEZONE_ACTIVE     = 0x03,
    E_HAL_DISP_TIMEZONE_FRONTPORCH = 0x04,
} HalDispTimeZoneType_e;


typedef enum
{
    E_HAL_DISP_PICTURE_CONTRAST = 0,
    E_HAL_DISP_PICTURE_BRIGHTNESS,
    E_HAL_DISP_PICTURE_SATURATION,
    E_HAL_DISP_PICTURE_SHARPNESS,
    E_HAL_DISP_PICTURE_HUE,
    E_HAL_DISP_PICTURE_GAIN,
    E_HAL_DISP_PICTURE_NUM
}HalDispPictureType_e;

//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------
typedef struct
{
    HalDispQueryType_e enQueryType;
    void  *pInCfg;
    u32   u32CfgSize;
}HalDispQueryInConfig_t;


typedef struct
{
    HalDispQueryRet_e enQueryRet;
    void (*pSetFunc)(void *, void *);
}HalDispQueryOutConfig_t;

typedef struct
{
    HalDispQueryInConfig_t stInCfg;
    HalDispQueryOutConfig_t stOutCfg;
}HalDispQueryConfig_t;


typedef struct
{
    u16 u16HsyncWidth;
    u16 u16HsyncBackPorch;
    u16 u16Hstart;
    u16 u16Hactive;
    u16 u16Htotal;

    u16 u16VsyncWidth;
    u16 u16VsyncBackPorch;
    u16 u16Vstart;
    u16 u16Vactive;
    u16 u16Vtotal;
    u16 u16Fps;

    u16 u16SscStep;
    u16 u16SscSpan;
}HalDispDeviceTimingConfig_t;

typedef struct
{
    bool  bSynm;     /* sync mode(0:timing,as BT.656; 1:signal,as LCD) */
    bool  bIop;      /* interlaced or progressive display(0:i; 1:p) */
    u8    u8Intfb;   /* interlace bit width while output */

    u16  u16Vact ;  /* vertical active area */
    u16  u16Vbb;    /* vertical back blank porch */
    u16  u16Vfb;    /* vertical front blank porch */

    u16  u16Hact;   /* herizontal active area */
    u16  u16Hbb;    /* herizontal back blank porch */
    u16  u16Hfb;    /* herizontal front blank porch */
    u16  u16Hmid;   /* bottom herizontal active area */

    u16  u16Bvact;  /* bottom vertical active area */
    u16  u16Bvbb;   /* bottom vertical back blank porch */
    u16  u16Bvfb;   /* bottom vertical front blank porch */

    u16  u16Hpw;    /* horizontal pulse width */
    u16  u16Vpw;    /* vertical pulse width */

    bool  bIdv;      /* inverse data valid of output */
    bool  bIhs;      /* inverse horizontal synch signal */
    bool  bIvs;      /* inverse vertical synch signal */
    u32   u32FrameRate;
} HalDispSyncInfo_t;

typedef struct
{
    HalDispDeviceTiming_e eTimeType;
    HalDispDeviceTimingConfig_t stDeviceTimingCfg;
}HalDispDeviceTimingInfo_t;

typedef struct
{
    HalDispCscmatrix_e eCscMatrix;
    u32 u32Luma;                     /* luminance:   0 ~ 100 default: 50 */
    u32 u32Contrast;                 /* contrast :   0 ~ 100 default: 50 */
    u32 u32Hue;                      /* hue      :   0 ~ 100 default: 50 */
    u32 u32Saturation;               /* saturation:  0 ~ 100 default: 50 */
} HalDispCsc_t;

typedef struct
{
    HalDispCsc_t stCsc;                     /* color space */
    u32 u32Gain;                          /* current gain of VGA signals. [0, 64). default:0x30 */
    u32 u32Sharpness;
} HalDispVgaParam_t;

typedef struct
{
    HalDispCsc_t stCsc;                     /* color space */
    u32 u32Sharpness;
} HalDispHdmiParam_t;

typedef struct
{
    bool bEnable;                     /* color space */
} HalDispCvbsParam_t;

typedef struct
{
    HalDispCsc_t stCsc;                     /* color space */
    u32 u32Sharpness;
} HalDispLcdParam_t;

typedef struct
{
    bool bEn;
    u16 u16EntryNum;
    u8 *pu8ColorR;
    u8 *pu8ColorG;
    u8 *pu8ColorB;
} HalDispGammaParam_t;

typedef struct
{
    u16 u16RedOffset;
    u16 u16GreenOffset;
    u16 u16BlueOffset;

    u16 u16RedColor;  // 00~FF, 0x80 is no change
    u16 u16GreenColor;// 00~FF, 0x80 is no change
    u16 u16BlueColor; // 00~FF, 0x80 is no change
}HalDispColorTemp_t;

typedef struct
{
    u16 u16X;
    u16 u16Y;
    u16 u16Width;
    u16 u16Height;
} HalDispVidWinRect_t;

typedef struct
{
    u32 u32Width;
    u32 u32Height;
} HalDispVideoLyaerSize_t;

typedef struct
{
    HalDispVidWinRect_t  stVidLayerDispWin;   /* Display resolution */
    HalDispVideoLyaerSize_t  stVidLayerSize;      /* Canvas size of the video layer */
    HalDispPixelFormat_e ePixFormat;         /* Pixel format of the video layer */
} HalDispVideoLayerAttr_t;

typedef struct
{
    bool bEnbale;                     /* Enable/Disable Compress */
} HalDispVideoLayerCompressAttr_t;


typedef struct
{
    HalDispVidWinRect_t stDispWin;                     /* rect of video out chn */
    u16 u16SrcWidth;
    u16 u16SrcHeight;
} HalDispInputPortAttr_t;

typedef  struct
{
    HalDispPixelFormat_e  ePixelFormat;
    HalDispPixelCompressMode_e eCompressMode;
    HalDispTileMode_e eTileMode;

    u64 au64PhyAddr[3];
    u32 au32Stride[3];
} HalDispVideoFrameData_t;


typedef struct
{
    HalDispPixelFormat_e enPixelFmt;
    u16 u16Width;
    u16 u16Height;
    bool bEn;
    u64 au64PhyAddr[3];
    u32 au32Stride[3];
} HalDispHwDmaConfig_t;

typedef struct
{
    HalDispRotateMode_e enRotate;
} HalDispInputPortRotate_t;

typedef struct
{
    HalDispTimeZoneType_e enType;
} HalDispTimeZone_t;

typedef struct
{
    u16 u16Htotal;
    u16 u16Vtotal;
    u16 u16HdeStart;
    u16 u16VdeStart;
    u16 u16Width;
    u16 u16Height;
    bool bInterlace;
    bool bYuvOutput;
} HalDispDisplayInfo_t;


typedef struct
{
    u16 u16OSD_0;
    u16 u16OSD_25;
    u16 u16OSD_50;
    u16 u16OSD_75;
    u16 u16OSD_100;
}HalDispPictureNonLinearCurveType_t;


typedef struct
{
    HalDispPictureNonLinearCurveType_t stHdmiPictureCurve[E_HAL_DISP_PICTURE_NUM];
    HalDispPictureNonLinearCurveType_t stVgaPictureCurve[E_HAL_DISP_PICTURE_NUM];
    HalDispPictureNonLinearCurveType_t stLcdPictureCurve[E_HAL_DISP_PICTURE_NUM];
}HalDispPictureNonLinearCurveConfig_t;

typedef struct
{
    bool bEn[HAL_DISP_CLK_NUM];
    u32  u32Rate[HAL_DISP_CLK_NUM];
    u32  u32Num;
} HalDispClkConfig_t;
#endif

