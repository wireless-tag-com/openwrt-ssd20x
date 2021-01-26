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

#ifndef _DISP_DEBUG_H_
#define _DISP_DEBUG_H_

//-----------------------------------------------------------------------------------------------------
// Variable Prototype
//-----------------------------------------------------------------------------------------------------
#ifndef _DRV_DISP_IF_C_
extern u32 _gu32DispDbgLevel;
#endif

//-----------------------------------------------------------------------------------------------------
// Debug Level
//-----------------------------------------------------------------------------------------------------

#define DISP_DBG_LEVEL_NONE              0x00000000
#define DISP_DBG_LEVEL_DRV               0x00000001
#define DISP_DBG_LEVEL_HAL               0x00000002
#define DISP_DBG_LEVEL_IO                0x00000004
#define DISP_DBG_LEVEL_CTX               0x00000008
#define DISP_DBG_LEVEL_COLOR             0x00000010
#define DISP_DBG_LEVEL_IRQ               0x00000020
#define DISP_DBG_LEVEL_IRQ_INTERNAL      0x00000040
#define DISP_DBG_LEVEL_IRQ_VGA_HPD       0x00000080
#define DISP_DBG_LEVEL_CLK               0x00000100
//-----------------------------------------------------------------------------------------------------
// Debug Macro
//-----------------------------------------------------------------------------------------------------
#define PRINT_NONE    "\33[m"
#define PRINT_RED     "\33[1;31m"
#define PRINT_YELLOW  "\33[1;33m"
#define PRINT_GREEN   "\33[1;32m"

#define DISP_DBG_EN 1


#if DISP_DBG_EN

#define DISP_ASSERT(_con)   \
    do {\
        if (!(_con)) {\
            CamOsPrintf("BUG at %s:%d assert(%s)\n",\
                    __FILE__, __LINE__, #_con);\
            BUG();\
        }\
    } while (0)

#define DISP_DBG(dbglv, _fmt, _args...)          \
    do                                          \
    if(_gu32DispDbgLevel & dbglv)                   \
    {                                           \
        CamOsPrintf(_fmt, ## _args);       \
    }while(0)

#define DISP_ERR(_fmt, _args...)                 \
    do{                                         \
        CamOsPrintf(PRINT_RED _fmt PRINT_NONE, ## _args);       \
    }while(0)




#else

#define     DISP_ASSERT(arg)
#define     DISP_DBG(dbglv, _fmt, _args...)
#define     DISP_ERR( _fmt, _args...)

#endif

//-----------------------------------------------------------------------------------------------------
// Parsing String
//-----------------------------------------------------------------------------------------------------
#define PARSING_CTX_TYPE(x)             ( x == E_DISP_CTX_TYPE_DEVICE    ? "DEVICE" : \
                                          x == E_DISP_CTX_TYPE_VIDLAYER  ? "VIDLAYER" : \
                                          x == E_DISP_CTX_TYPE_INPUTPORT ? "INPUTPORT" : \
                                                                           "UNKNOWN")


#define PARSING_HAL_QUERY_TYPE(x)       ( x == E_HAL_DISP_QUERY_DEVICE_INIT             ? "DEVICE_INIT" : \
                                          x == E_HAL_DISP_QUERY_DEVICE_ENABLE           ? "DEVICE_ENABLE" : \
                                          x == E_HAL_DISP_QUERY_DEVICE_ATTACH           ? "DEVICE_ATTACH" : \
                                          x == E_HAL_DISP_QUERY_DEVICE_DETACH           ? "DEVICE_DETACH" : \
                                          x == E_HAL_DISP_QUERY_DEVICE_BACKGROUND_COLOR ? "DEVICE_BACKGROUND_COLOR" : \
                                          x == E_HAL_DISP_QUERY_DEVICE_INTERFACE        ? "DEVICE_INTERFACE" : \
                                          x == E_HAL_DISP_QUERY_DEVICE_OUTPUTTIMING     ? "DEVICE_OUTPUTTIMING" : \
                                          x == E_HAL_DISP_QUERY_DEVICE_VGA_PARAM        ? "DEVICE_VGA_PARAM" : \
                                          x == E_HAL_DISP_QUERY_DEVICE_CVBS_PARAM       ? "DEVICE_CVBS_PARAM" : \
                                          x == E_HAL_DISP_QUERY_DEVICE_COLORTEMP        ? "DEVICE_COLORTEMP" : \
                                          x == E_HAL_DISP_QUERY_DEVICE_HDMI_PARAM       ? "DEVICE_HDMI_PARAM" : \
                                          x == E_HAL_DISP_QUERY_DEVICE_LCD_PARAM        ? "DEVICE_LCD_PARAM" : \
                                          x == E_HAL_DISP_QUERY_DEVICE_GAMMA_PARAM      ? "DEVICE_GAMMA_PARAM" : \
                                          x == E_HAL_DISP_QUERY_DEVICE_TIME_ZONE        ? "DEVICE_TIME_ZONE" : \
                                          x == E_HAL_DISP_QUERY_DEVICE_DISPLAY_INFO     ? "DEVICE_DISPLAY_INFO": \
                                          x == E_HAL_DISP_QUERY_VIDEOLAYER_INIT         ? "VIDEOLAYER_INIT" : \
                                          x == E_HAL_DISP_QUERY_VIDEOLAYER_ENABLE       ? "VIDEOLAYER_ENABLE" : \
                                          x == E_HAL_DISP_QUERY_VIDEOLAYER_BIND         ? "VIDEOLAYER_BIND" : \
                                          x == E_HAL_DISP_QUERY_VIDEOLAYER_UNBIND       ? "VIDEOLAYER_UNBIND" : \
                                          x == E_HAL_DISP_QUERY_VIDEOLAYER_ATTR         ? "VIDEOLAYER_ATTR" : \
                                          x == E_HAL_DISP_QUERY_VIDEOLAYER_COMPRESS     ? "VIDEOLAYER_COMPRESS" : \
                                          x == E_HAL_DISP_QUERY_VIDEOLAYER_PRIORITY     ? "VIDEOLAYER_PRIORITY" : \
                                          x == E_HAL_DISP_QUERY_VIDEOLAYER_BUFFER_FIRE  ? "VIDEOLAYER_BUFFER_FIRE" : \
                                          x == E_HAL_DISP_QUERY_VIDEOLAYER_CHECK_FIRE   ? "VIDEOLAYER_CHECK_FIRE" : \
                                          x == E_HAL_DISP_QUERY_INPUTPORT_INIT          ? "INPUTPORT_INIT" : \
                                          x == E_HAL_DISP_QUERY_INPUTPORT_ENABLE        ? "INPUTPORT_ENABLE" : \
                                          x == E_HAL_DISP_QUERY_INPUTPORT_ATTR          ? "INPUTPORT_ATTR" : \
                                          x == E_HAL_DISP_QUERY_INPUTPORT_SHOW          ? "INPUTPORT_SHOW" : \
                                          x == E_HAL_DISP_QUERY_INPUTPORT_HIDE          ? "INPUTPORT_HIDEN" : \
                                          x == E_HAL_DISP_QUERY_INPUTPORT_BEGIN         ? "INPUTPORT_BEGIN" : \
                                          x == E_HAL_DISP_QUERY_INPUTPORT_END           ? "INPUTPORT_END" : \
                                          x == E_HAL_DISP_QUERY_INPUTPORT_FLIP          ? "INPUTPORT_FLIP" : \
                                          x == E_HAL_DISP_QUERY_CLK_SET                 ? "CLK_SET" :\
                                          x == E_HAL_DISP_QUERY_CLK_GET                 ? "CLK_GET" :\
                                                                                          "UNKNOWN")


#define PARSING_HAL_QUERY_RET(x)        ( x == E_HAL_DISP_QUERY_RET_OK         ? "RET_OK" : \
                                          x == E_HAL_DISP_QUERY_RET_CFGERR     ? "RET_CFGERR" : \
                                          x == E_HAL_DISP_QUERY_RET_NONEED     ? "RET_NO_NEED" : \
                                          x == E_HAL_DISP_QUERY_RET_NOTSUPPORT ? "RET_NOT_SUPPORT" : \
                                                                                 "UNKNOWN")

#define PARSING_HAL_PIXEL_FMT(x)        ( x == E_HAL_DISP_PIXEL_FRAME_YUV422_YUYV        ? "YUV422_YUYV" : \
                                          x == E_HAL_DISP_PIXEL_FRAME_ARGB8888           ? "ARGB8888" : \
                                          x == E_HAL_DISP_PIXEL_FRAME_ABGR8888           ? "ABGR8888" : \
                                          x == E_HAL_DISP_PIXEL_FRAME_BGRA8888           ? "BGRA8888" : \
                                          x == E_HAL_DISP_PIXEL_FRAME_RGB565             ? "RGB565" : \
                                          x == E_HAL_DISP_PIXEL_FRAME_ARGB1555           ? "ARGB1555" : \
                                          x == E_HAL_DISP_PIXEL_FRAME_ARGB4444           ? "ARGB4444" : \
                                          x == E_HAL_DISP_PIXEL_FRAME_YUV_SEMIPLANAR_422 ? "YUV_SEMIPLANAR_422" : \
                                          x == E_HAL_DISP_PIXEL_FRAME_YUV_SEMIPLANAR_420 ? "YUV_SEMIPLANAR_420" : \
                                          x == E_HAL_DISP_PIXEL_FRAME_YUV_MST_420        ? "YUV_MST_420" : \
                                                                                           "UNKNOWN")

#define PARSING_HAL_COMPRESS_MD(x)      ( x == E_HAL_DISP_COMPRESS_MODE_NONE     ? "NONE" :\
                                          x == E_HAL_DISP_COMPRESS_MODE_SEG      ? "SEG" :\
                                          x == E_HAL_DISP_COMPRESS_MODE_LINE     ? "LINE" :\
                                          x == E_HAL_DISP_COMPRESS_MODE_FRAME    ? "FRAME" :\
                                                                                   "UNKNOWN")


#define PARSING_HAL_TILE_MD(x)          ( x == E_HAL_DISP_TILE_MODE_NONE   ? "NONE" :\
                                          x == E_HAL_DISP_TILE_MODE_16x16  ? "16x16" :\
                                          x == E_HAL_DISP_TILE_MODE_16x32  ? "16x32" :\
                                          x == E_HAL_DISP_TILE_MODE_32x16  ? "32x16" :\
                                          x == E_HAL_DISP_TILE_MODE_32x32  ? "32x31" :\
                                                                             "UNKNOWN")

#define PARSING_HAL_CSC_MATRIX(x)       ( x == E_HAL_DISP_CSC_MATRIX_BYPASS          ? "BYPASS" : \
                                          x == E_HAL_DISP_CSC_MATRIX_BT601_TO_BT709  ? "BT601_TO_BT709 " : \
                                          x == E_HAL_DISP_CSC_MATRIX_BT709_TO_BT601  ? "BT709_TO_BT601 " : \
                                          x == E_HAL_DISP_CSC_MATRIX_BT601_TO_RGB_PC ? "BT601_TO_RGB_PC" : \
                                          x == E_HAL_DISP_CSC_MATRIX_BT709_TO_RGB_PC ? "BT709_TO_RGB_PC" : \
                                          x == E_HAL_DISP_CSC_MATRIX_RGB_TO_BT601_PC ? "RGB_TO_BT601_PC" : \
                                          x == E_HAL_DISP_CSC_MATRIX_RGB_TO_BT709_PC ? "RGB_TO_BT709_PC" : \
                                                                                      "UNKNOWN")

#define PARSING_HAL_TMING_ID(x)         ( x == E_HAL_DISP_OUTPUT_PAL          ? "PAL" : \
                                          x == E_HAL_DISP_OUTPUT_NTSC         ? "NTSC" : \
                                          x == E_HAL_DISP_OUTPUT_960H_PAL     ? "960H_PAL" : \
                                          x == E_HAL_DISP_OUTPUT_960H_NTSC    ? "960H_NTSC" : \
                                          x == E_HAL_DISP_OUTPUT_1080P24      ? "1080P24" : \
                                          x == E_HAL_DISP_OUTPUT_1080P25      ? "1080P25" : \
                                          x == E_HAL_DISP_OUTPUT_1080P30      ? "1080P30" : \
                                          x == E_HAL_DISP_OUTPUT_720P50       ? "720P50" : \
                                          x == E_HAL_DISP_OUTPUT_720P60       ? "720P60" : \
                                          x == E_HAL_DISP_OUTPUT_1080I50      ? "1080I50" : \
                                          x == E_HAL_DISP_OUTPUT_1080I60      ? "1080I60" : \
                                          x == E_HAL_DISP_OUTPUT_1080P50      ? "1080P50" : \
                                          x == E_HAL_DISP_OUTPUT_1080P60      ? "1080P60" : \
                                          x == E_HAL_DISP_OUTPUT_576P50       ? "576P50" : \
                                          x == E_HAL_DISP_OUTPUT_480P60       ? "480P60" : \
                                          x == E_HAL_DISP_OUTPUT_640x480_60   ? "640x480_60" : \
                                          x == E_HAL_DISP_OUTPUT_800x600_60   ? "800x600_60" : \
                                          x == E_HAL_DISP_OUTPUT_1024x768_60  ? "1024x768_60" : \
                                          x == E_HAL_DISP_OUTPUT_1280x1024_60 ? "1280x1024_60" : \
                                          x == E_HAL_DISP_OUTPUT_1366x768_60  ? "1366x768_60" : \
                                          x == E_HAL_DISP_OUTPUT_1440x900_60  ? "1440x900_60" : \
                                          x == E_HAL_DISP_OUTPUT_1280x800_60  ? "1280x800_60" : \
                                          x == E_HAL_DISP_OUTPUT_1680x1050_60 ? "1680x1050_60" : \
                                          x == E_HAL_DISP_OUTPUT_1920x2160_30 ? "1920x2160_30" : \
                                          x == E_HAL_DISP_OUTPUT_1600x1200_60 ? "1600x1200_60" : \
                                          x == E_HAL_DISP_OUTPUT_1920x1200_60 ? "1920x1200_60" : \
                                          x == E_HAL_DISP_OUTPUT_2560x1440_30 ? "2560x1440_30" : \
                                          x == E_HAL_DISP_OUTPUT_2560x1600_60 ? "2560x1600_60" : \
                                          x == E_HAL_DISP_OUTPUT_3840x2160_30 ? "3840x2160_30" : \
                                          x == E_HAL_DISP_OUTPUT_3840x2160_60 ? "3840x2160_60" : \
                                          x == E_HAL_DISP_OUTPUT_USER         ? "USER" : \
                                                                                "UNKNOWN" )

#define PARSING_HAL_INTERFACE(x)       (x == HAL_DISP_INTF_HDMI ? "HDMI" : \
                                        x == HAL_DISP_INTF_CVBS ? "CVBS" : \
                                        x == HAL_DISP_INTF_VGA  ? "VGA" :  \
                                        x == HAL_DISP_INTF_LCD  ? "LCD" :  \
                                                                  "UNKNOWN")

#define PARSING_HAL_ROTATE(x)           (x == E_HAL_DISP_ROTATE_NONE ? "NONE" :\
                                         x == E_HAL_DISP_ROTATE_90   ? "90" :\
                                         x == E_HAL_DISP_ROTATE_180  ? "180" :\
                                         x == E_HAL_DISP_ROTATE_270  ? "270" :\
                                                                       "UNKNOWN")

#define PARSING_HAL_TIMEZONE(x)         (x == E_HAL_DISP_TIMEZONE_SYNC       ? "SYNC" :\
                                         x == E_HAL_DISP_TIMEZONE_BACKPORCH  ? "BACKPORCH" :\
                                         x == E_HAL_DISP_TIMEZONE_ACTIVE     ? "ACTIVE" :\
                                         x == E_HAL_DISP_TIMEZONE_FRONTPORCH ? "FRONTPORCH" :\
                                                                               "UNKNOWN")

#define PARSING_HAL_IRQ_IOCTL(x)        (x == E_HAL_DISP_IRQ_IOCTL_ENABLE               ? "ENABLE"              : \
                                         x == E_HAL_DISP_IRQ_IOCTL_GET_FLAG             ? "GET_FLAG"            : \
                                         x == E_HAL_DISP_IRQ_IOCTL_CLEAR                ? "CLEAR"               : \
                                         x == E_HAL_DISP_IRQ_IOCTL_INTERNAL_SUPPORTED   ? "INTERNAL_SUPPORTED"  : \
                                         x == E_HAL_DISP_IRQ_IOCTL_INTERNAL_GET_ID      ? "INTERNAL_GET_ID"     : \
                                         x == E_HAL_DISP_IRQ_IOCTL_INTERNAL_ENABLE      ? "INTERNAL_ENABLE"     : \
                                         x == E_HAL_DISP_IRQ_IOCTL_INTERNAL_GET_FLAG    ? "INTERNAL_GET_FLAG"   : \
                                         x == E_HAL_DISP_IRQ_IOCTL_INTERNAL_CLEAR       ? "INTERNAL_CLEAR"      : \
                                         x == E_HAL_DISP_IRQ_IOCTL_INTERNAL_GET_STATUS  ? "INTERNAL_GET_STATUS" : \
                                                                                          "UNKNOWN")

#define PARSING_HAL_IRQ_TYPE(x)        (x == E_HAL_DISP_IRQ_TYPE_NONE                    ? "NONE"                     : \
                                        x == E_HAL_DISP_IRQ_TYPE_VSYNC                   ? "VSYNC"                    : \
                                        x == E_HAL_DISP_IRQ_TYPE_VDE                     ? "VDE"                      : \
                                        x == E_HAL_DISP_IRQ_TYPE_INTERNAL_VDE_NEGATIVE   ? "INTERNAL_VDE_NEGATIVE"    : \
                                        x == E_HAL_DISP_IRQ_TYPE_INTERNAL_VDE_POSITIVE   ? "INTERNAL_VDE_POSITIVE"    : \
                                        x == E_HAL_DISP_IRQ_TYPE_INTERNAL_VSYNC_NEGATIVE ? "INTERNAL_VSYNC_NEGATIVE"  : \
                                        x == E_HAL_DISP_IRQ_TYPE_INTERNAL_VSYNC_POSITIVE ? "INTERNAL_VSYNC_POSITIVE"  : \
                                        x == E_HAL_DISP_IRQ_TYPE_INTERNAL_TIMEZONE       ? "INTERNAL_TIME_ZONE"       : \
                                        x == E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON              ? "INTERNAL_VGP_HPD_ON"      : \
                                        x == E_HAL_DISP_IRQ_TYPE_VGA_HPD_OFF             ? "INTERNAL_VGP_HPD_OFF"     : \
                                        x == E_HAL_DISP_IRQ_TYPE_VGA_HPD_ON_OFF          ? "INTERNAL_VGA_HPD_ON_OFF"  : \
                                                                                           "UNKNOWN")
#endif // #ifndef



