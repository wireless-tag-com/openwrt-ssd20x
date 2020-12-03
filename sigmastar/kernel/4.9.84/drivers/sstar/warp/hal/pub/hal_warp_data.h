/*
* hal_warp_data.h- Sigmastar
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
#ifndef _HAL_WARP_DATA_H_
#define _HAL_WARP_DATA_H_

#include <linux/kernel.h>

/// @brief MHAL WARP operation modes enumeration
typedef enum
{
    HAL_WARP_OP_MODE_PERSPECTIVE        = 0,
    HAL_WARP_OP_MODE_MAP                      = 1
} HAL_WARP_OP_MODE_E;

/// @brief MHAL WARP image formats enumeration
typedef enum
{
    HAL_WARP_IMAGE_FORMAT_RGBA             = 0,
    HAL_WARP_IMAGE_FORMAT_YUV422           = 1,
    HAL_WARP_IMAGE_FORMAT_YUV420           = 2
} HAL_WARP_IMAGE_FORMAT_E;

/// @brief MHAL WARP perspective coefficients enumeration
typedef enum
{
    HAL_WARP_PERSECTIVE_COEFFS_C00         = 0,
    HAL_WARP_PERSECTIVE_COEFFS_C01         = 1,
    HAL_WARP_PERSECTIVE_COEFFS_C02         = 2,
    HAL_WARP_PERSECTIVE_COEFFS_C10         = 3,
    HAL_WARP_PERSECTIVE_COEFFS_C11         = 4,
    HAL_WARP_PERSECTIVE_COEFFS_C12         = 5,
    HAL_WARP_PERSECTIVE_COEFFS_C20         = 6,
    HAL_WARP_PERSECTIVE_COEFFS_C21         = 7,
    HAL_WARP_PERSECTIVE_COEFFS_C22         = 8,
    HAL_WARP_PERSECTIVE_COEFFS_NUM_COEFFS  = 9
} HAL_WARP_PERSECTIVE_COEFFS_E;

/// @brief MHAL WARP displacement map resolution enumeration
typedef enum
{
    HAL_WARP_MAP_RESLOUTION_8X8            = 0,
    HAL_WARP_MAP_RESLOUTION_16X16          = 1
} HAL_WARP_MAP_RESLOUTION_E;

/// @brief MHAL WARP displacement map format enumeration
typedef enum
{
    HAL_WARP_MAP_FORMAT_ABSOLUTE           = 0,
    HAL_WARP_MAP_FORMAT_RELATIVE           = 1
} HAL_WARP_MAP_FORMAT_E;

/// @brief WARP image plane index enumeration
typedef enum
{
    HAL_WARP_IMAGE_PLANE_RGBA              = 0,
    HAL_WARP_IMAGE_PLANE_Y                 = 0,
    HAL_WARP_IMAGE_PLANE_UV                = 1
} HAL_WARP_IMAGE_PLANE_E;

typedef enum
{
    HAL_WARP_INSTANCE_STATE_READY          = 0,
    HAL_WARP_INSTANCE_STATE_PROCESSING     = 1,
    HAL_WARP_INSTANCE_STATE_DONE           = 2,
    HAL_WARP_INSTANCE_STATE_AXI_ERROR      = 3,
    HAL_WARP_INSTANCE_STATE_PROC_ERROR     = 4
} HAL_WARP_INSTANCE_STATE_E;

typedef enum
{
    HAL_WARP_ISR_STATE_DONE                = 0,
    HAL_WARP_ISR_STATE_PROCESSING          = 1
} HAL_WARP_ISR_STATE_E;

/// @brief MHAL WARP displacement map entry for absolute coordinates format
typedef struct
{
    s32 y;
    s32 x;
} HAL_WARP_DISPPLAY_ABSOLUTE_ENTRY_T;

/// @brief MHAL WARP displacement map entry for coordinates' relative offset format
typedef struct
{
    s16 y;
    s16 x;
} HAL_WARP_DISPPLAY_RELATIVE_ENTRY_T;

/// @brief MHAL WARP displacement table descriptor
typedef struct
{
    HAL_WARP_MAP_RESLOUTION_E resolution;              // map resolution
    HAL_WARP_MAP_FORMAT_E     format;                  // map format (absolute, relative)
    s32 size;                                        // size of Disp.
    u64 table;
} HAL_WARP_DISPLAY_TABLE_T;

typedef struct
{
    s16 height;                                      // input tile height
    s16 width;                                       // input tile width
    s16 y;                                           // input tile top left y coordinate
    s16 x;                                           // input tile top left x coordinate
}  HAL_WARP_BOUND_BOX_ENTRY_T;

typedef struct
{
    s32 size;                                        // number of bound boxes
    u64 table;           // list of all bound boxes
}HAL_WARP_BOUND_BOX_TABLE_T;

/// @brief MHAL WARP image descriptor
typedef struct
{
    HAL_WARP_IMAGE_FORMAT_E format;                    // image format
    u32 width;                                       // image width  (for YUV - Y plane width)
    u32 height;                                      // image height (for YUV - Y plane width)
} HAL_WARP_IMAGE_DESC_T;

/// @brief MHAL WARP image data structure
typedef struct
{
    u32 num_planes;                                  // number of image planes
    u64 data[2];                                     // pointers to the image planes' data
} HAL_WARP_IMAGE_DATA_T;

/// @brief MHAL WARP hardware configuration structure
typedef struct
{
    HAL_WARP_OP_MODE_E op_mode;                        // Operation mode
    HAL_WARP_DISPLAY_TABLE_T disp_table;               // Displacement table descriptor
    HAL_WARP_BOUND_BOX_TABLE_T bb_table;               // Bounding box table descriptor

    int coeff[HAL_WARP_PERSECTIVE_COEFFS_NUM_COEFFS];  // Perspective transform coefficients

    HAL_WARP_IMAGE_DESC_T input_image;                 // Input image
    HAL_WARP_IMAGE_DATA_T input_data;

    HAL_WARP_IMAGE_DESC_T output_image;                // Output image
    HAL_WARP_IMAGE_DATA_T output_data;

    u8 fill_value[2];                                // Fill value for out of range pixels

} HAL_WARP_CONFIG;

#endif //_HAL_WARP_DATA_H_
