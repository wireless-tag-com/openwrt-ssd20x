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

#ifndef _HAL_DISP_COLOR_H_
#define _HAL_DISP_COLOR_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------
typedef enum
{
    E_DISP_COLOR_YUV_2_RGB_MATRIX_BYPASS,
    E_DISP_COLOR_YUV_2_RGB_MATRIX_HDTV,
    E_DISP_COLOR_YUV_2_RGB_MATRIX_SDTV,
    E_DISP_COLOR_YUV_2_RGB_MATRIX_USER,
    E_DISP_COLOR_YUV_2_RGB_MATRIX_MAX,
}HalDispColorYuvToRgbMatrixType_e;
//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------

#ifdef _HAL_DISP_COLOR_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif

INTERFACE void HalDispColorInitVar(void);
INTERFACE void HalDispColorSeletYuvToRgbMatrix(u8 u8Id, HalDispColorYuvToRgbMatrixType_e enType, s16 *psYuv2RgbMatrix);
INTERFACE void HalDispColorSetColorCorrectMatrix(u8 u8Id, s16 *psColorCorrectMatrix);
INTERFACE void HalDispColorAdjustBrightness( u8 u8Id, u8 u8Brightness);
INTERFACE void HalDispColorAdjustHCS( u8 u8Id, u8 u8Hue, u8  u8Saturation, u8  u8Contrast );
INTERFACE void HalDispColorAdjustVideoContrast( u8 u8Id, u8 u8Contrast );
INTERFACE void HalDispColorAdjustVideoSaturation( u8 u8Id, u8 u8Saturation );
INTERFACE void HalDispColorAdjustVideoCbCr( u8 u8Id, u8 u8Cb, u8 u8Cr );
INTERFACE void HalDispColorAdjustVideoHue( u8 u8Id, u8 u8Hue );
INTERFACE void HalDispColorAdjustVideoRGB( u8 u8Id, u8 u8RCon, u8 u8GCon, u8 u8BCon);
INTERFACE void HalDispColorAdjustPcContrastRGB( u8 u8Id, u8 u8Contrast, u8 u8Red, u8 u8Green, u8 u8Blue );
INTERFACE void HalDispColorAdjustPcContrast( u8 u8Id, u8 u8Contrast );
INTERFACE void HalDispColorAdjustPcRGB( u8 u8Id, u8 u8Red, u8 u8Green, u8 u8Blue );
INTERFACE void HalDispColorSetYUV2RGBCtrl( u8 u8Id, bool bEnable );
INTERFACE void HalDispColorSetPCsRGBCtrl( u8 u8Id, bool bEnable );
INTERFACE void HalDispColorSetHue256StepCtrl( u8 u8Id, bool bEnable );


#undef INTERFACE
#endif
