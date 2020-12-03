/*
* mdrv_fb_st.h- Sigmastar
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

 /**
 * \ingroup fb_group
 * @{
 */
#ifndef _MDRV_FB_ST_H
#define _MDRV_FB_ST_H

//=============================================================================
// enum
//=============================================================================
/**
* Used to show current support color format of gop in FB_GOP_SUPINF_CONFIG
*/
typedef enum
{
    COLORFMT_ARGB8888 = 0,
    COLORFMT_RGB565   = 1,
    COLORFMT_YUV422   = 2,
    COLORFMT_I8PALETTE= 3,
    COLORFMT_MAX      = 4,
}FB_GOP_COLORFORMAT_TYPE;

/**
* Used to setup the mode of alpha blending for gop in FB_GOP_ALPHA_CONFIG
*/
typedef enum
{
    PIXEL_ALPHA = 0,
    CONST_ALPHA = 1,
}FB_GOP_ALPHA_TYPE;

/**
* Used to setup the buffer number for gop in FB_GOP_RESOLUTION_STRETCH_H_CONFIG
*/
typedef enum
{
    FB_SINGLE_BUFFER = 1,
    FB_DOUBLE_BUFFER = 2,
}FB_GOP_BUFFER_NUM;

/**
* Used to setup the stretch H ratio for gop in FB_GOP_RESOLUTION_STRETCH_H_CONFIG
*/
typedef enum
{
    FB_STRETCH_H_RATIO_1 = 1,
    FB_STRETCH_H_RATIO_2 = 2,
    FB_STRETCH_H_RATIO_4 = 4,
    FB_STRETCH_H_RATIO_8 = 8,
}FB_GOP_STRETCH_H_RATIO;

/**
* Used to setup the stretch H ratio for gop in FB_GOP_RESOLUTION_STRETCH_H_CONFIG
*/
typedef enum
{
    FB_INV_COLOR_ROTATE_CLOCKWISE_0   = 0,
    FB_INV_COLOR_ROTATE_CLOCKWISE_90  = 1,
    FB_INV_COLOR_ROTATE_CLOCKWISE_180 = 2,
    FB_INV_COLOR_ROTATE_CLOCKWISE_270 = 3,
}FB_GOP_INV_COLOR_ROTATE;

//=============================================================================
// struct
//=============================================================================



//=============================================================================
// struct for IOCTL_FB_GETFBSUPPORTINF
/**
* Used to get framebuffer support information
*/
typedef struct
{

    unsigned char bKeyAlpha;             ///< whether support colorkey
    unsigned char bConstAlpha;           ///< whether support constant alpha
    unsigned char bPixelAlpha;           ///< whether support pixel alpha
    unsigned char bColFmt[COLORFMT_MAX]; ///< support which color format
    unsigned long u32MaxWidth;           ///< the max pixels per line
    unsigned long u32MaxHeight;          ///< the max lines
}FB_GOP_SUPINF_CONFIG;

// struct for FB_GOP_GWIN_CONFIG
/**
* Used to set or get gwin parameters
*/
typedef struct
{
    unsigned short u18HStart; ///< gwin horizontal starting coordinate
    unsigned short u18HEnd;   ///< gwin horizontal ending coordinate
    unsigned short u18VStart; ///< gwin vertical starting coordinate
    unsigned short u18VEnd;   ///< gwin vertical ending coordinate
}FB_GOP_GWIN_CONFIG;

// struct for FB_GOP_ALPHA_CONFIG
/**
* Used to set or get gop alpha blending settings
*/
typedef struct
{
    unsigned char bEn;             ///< alpha blending enable or disable
    FB_GOP_ALPHA_TYPE enAlphaType; ///< set alpha type: pixel alpha or constant alpha
    unsigned char u8Alpha;         ///< constant alpha value, availble for setting constant alpha
}FB_GOP_ALPHA_CONFIG;

// struct for FB_GOP_COLORKEY_CONFIG
/**
* Used to set or get gop colorkey settings
*/
typedef struct
{
    unsigned char bEn; ///< colorkey enable or disable
    unsigned char u8R; ///< parameter for red color in colorkey
    unsigned char u8G; ///< parameter for green color in colorkey
    unsigned char u8B; ///< parameter for blue color in colorkey
}FB_GOP_COLORKEY_CONFIG;

// struct for FB_GOP_PaletteEntry
/**
* Used to set palette parameters
*/
typedef union
{
    /// RGBA8888
    struct
    {
        unsigned char u8B; ///< parameter for blue color in palette
        unsigned char u8G; ///< parameter for green color in palette
        unsigned char u8R; ///< parameter for red color in palette
        unsigned char u8A; ///< parameter for alpha in palette
    } BGRA;
} FB_GOP_PaletteEntry;

// struct for FB_GOP_RESOLUTION_STRETCH_H_CONFIG
/**
* Used to set or get gop colorkey settings
*/
typedef struct
{
    unsigned long u32Width;                  ///< the width for gop resolution
    unsigned long u32Height;                 ///< the height for gop resolution
    unsigned long u32DisplayWidth;           ///< the width for display resolution, =u32Width*enStretchH_Ratio
    unsigned long u32DisplayHeight;          ///< the height for display resolution,=u32Height
    FB_GOP_BUFFER_NUM enBufferNum;           ///< buffer number
    FB_GOP_STRETCH_H_RATIO enStretchH_Ratio; ///< stretch H ratio
}FB_GOP_RESOLUTION_STRETCH_H_CONFIG;

// struct for FB_GOP_INVCOLOR_AE_CONFIG
/**
* Used to set AE configurations into the engine of gop inverse color
*/
typedef struct
{
    unsigned long u32AEBlkWidth;       ///< the width for AE block
    unsigned long u32AEBlkHeight;      ///< the height for AE block
    unsigned long u32AEDisplayWidth;   ///< the width for AE display resolutionfor
    unsigned long u32AEDisplayHeight;  ///< the height for AE display resolutionfor
}FB_GOP_INVCOLOR_AE_CONFIG;

// struct for FB_GOP_INVCOLOR_SCALER_CONFIG
/**
* Used to set Scaler configurations into the engine of gop inverse color
*/
typedef struct
{
    unsigned char bCropEn;             ///< scaler do crop or not
    unsigned char bScalingEn;          ///< scaler do scaling or not
    unsigned char bRotateEn;           ///< scaler do rotate or not

    unsigned long u32ScalerWidth;      ///< the width for Scaler resolution
    unsigned long u32ScalerHeight;     ///< the height for Scaler resolution

    unsigned long u32CropXstart;       ///< the start x coordinate for Scaler crop to calculate the crop width, available for bCropEn=1
    unsigned long u32CropXend;         ///< the end x coordinate for Scaler crop to calculate the crop width, available for bCropEn=1
    unsigned long u32CropYstart;       ///< the start y coordinate for Scaler crop to calculate the crop height, available for bCropEn=1
    unsigned long u32CropYend;         ///< the end y coordinate for Scaler crop to calculate the crop height, available for bCropEn=1

    FB_GOP_INV_COLOR_ROTATE enRotateA; ///< the rotate angle for Scaler rotate, available for bRotateEn=1
}FB_GOP_INVCOLOR_SCALER_CONFIG;

// struct for FB_GOP_SW_INV_TABLE
/**
* Used to set sw inverse table
*/
typedef struct
{
    unsigned char *invTable;           ///< sw inverse table
    int size;                          ///<size of inverse table
}FB_GOP_SW_INV_TABLE;


//=============================================================================

#define GOP_PALETTE_ENTRY_NUM 256

//=============================================================================s
#endif //MDRV_FB_ST_H


/** @} */ // end of fb_group
