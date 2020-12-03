/*
* mdrv_warp_io_st.h- Sigmastar
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
#ifndef _MDRV_WARP_IO_ST_H_
#define _MDRV_WARP_IO_ST_H_

#include <asm/types.h>
#include <linux/kernel.h>

//---------------------------------------------------------------------------
// Structure and enum.
//---------------------------------------------------------------------------

/// @brief MHAL WARP operation modes enumeration
typedef enum
{
    MHAL_WARP_OP_MODE_PERSPECTIVE           = 0,
    MHAL_WARP_OP_MODE_MAP                   = 1
} MHAL_WARP_OP_MODE_E;

/// @brief MHAL WARP image formats enumeration
typedef enum
{
    MHAL_WARP_IMAGE_FORMAT_RGBA             = 0,
    MHAL_WARP_IMAGE_FORMAT_YUV422           = 1,
    MHAL_WARP_IMAGE_FORMAT_YUV420           = 2
} MHAL_WARP_IMAGE_FORMAT_E;

/// @brief MHAL WARP perspective coefficients enumeration
typedef enum
{
    MHAL_WARP_PERSECTIVE_COEFFS_C00         = 0,
    MHAL_WARP_PERSECTIVE_COEFFS_C01         = 1,
    MHAL_WARP_PERSECTIVE_COEFFS_C02         = 2,
    MHAL_WARP_PERSECTIVE_COEFFS_C10         = 3,
    MHAL_WARP_PERSECTIVE_COEFFS_C11         = 4,
    MHAL_WARP_PERSECTIVE_COEFFS_C12         = 5,
    MHAL_WARP_PERSECTIVE_COEFFS_C20         = 6,
    MHAL_WARP_PERSECTIVE_COEFFS_C21         = 7,
    MHAL_WARP_PERSECTIVE_COEFFS_C22         = 8,
    MHAL_WARP_PERSECTIVE_COEFFS_NUM_COEFFS  = 9
} MHAL_WARP_PERSECTIVE_COEFFS_E;

/// @brief MHAL WARP displacement map resolution enumeration
typedef enum
{
    MHAL_WARP_MAP_RESLOUTION_8X8            = 0,
    MHAL_WARP_MAP_RESLOUTION_16X16          = 1
} MHAL_WARP_MAP_RESLOUTION_E;

/// @brief MHAL WARP displacement map format enumeration
typedef enum
{
    MHAL_WARP_MAP_FORMAT_ABSOLUTE           = 0,
    MHAL_WARP_MAP_FORMAT_RELATIVE           = 1
} MHAL_WARP_MAP_FORMAT_E;

/// @brief WARP image plane index enumeration
typedef enum
{
    MHAL_WARP_IMAGE_PLANE_RGBA              = 0,
    MHAL_WARP_IMAGE_PLANE_Y                 = 0,
    MHAL_WARP_IMAGE_PLANE_UV                = 1
} MHAL_WARP_IMAGE_PLANE_E;

typedef enum
{
    MHAL_WARP_INSTANCE_STATE_READY          = 0,
    MHAL_WARP_INSTANCE_STATE_PROCESSING     = 1,
    MHAL_WARP_INSTANCE_STATE_DONE           = 2,
    MHAL_WARP_INSTANCE_STATE_AXI_ERROR      = 3,
    MHAL_WARP_INSTANCE_STATE_PROC_ERROR     = 4
} MHAL_WARP_INSTANCE_STATE_E;

typedef enum
{
    MHAL_WARP_ISR_STATE_DONE                = 0,
    MHAL_WARP_ISR_STATE_PROCESSING          = 1
} MHAL_WARP_ISR_STATE_E;

/// @brief MHAL WARP displacement map entry for absolute coordinates format
typedef struct
{
    __s32 y;
    __s32 x;
} MHAL_WARP_DISPPLAY_ABSOLUTE_ENTRY_T;

/// @brief MHAL WARP displacement map entry for coordinates' relative offset format
typedef struct
{
    __s16 y;
    __s16 x;
} MHAL_WARP_DISPPLAY_RELATIVE_ENTRY_T;

/// @brief MHAL WARP displacement table descriptor
typedef struct
{
    MHAL_WARP_MAP_RESLOUTION_E resolution;              // map resolution
    MHAL_WARP_MAP_FORMAT_E     format;                  // map format (absolute, relative)
    __s32 size;                                         // size of Disp.
    __u64 table;
} MHAL_WARP_DISPLAY_TABLE_T;

typedef struct
{
    __s16 height;                                       // input tile height
    __s16 width;                                        // input tile width
    __s16 y;                                            // input tile top left y coordinate
    __s16 x;                                            // input tile top left x coordinate
}  MHAL_WARP_BOUND_BOX_ENTRY_T;

typedef struct
{
    __s32 size;                                         // bytes of bound boxes
    __u64 table;           // list of all bound boxes
}MHAL_WARP_BOUND_BOX_TABLE_T;

/// @brief MHAL WARP image descriptor
typedef struct
{
    MHAL_WARP_IMAGE_FORMAT_E format;                    // image format
    __u32 width;                                        // image width  (for YUV - Y plane width)
    __u32 height;                                       // image height (for YUV - Y plane width)
} MHAL_WARP_IMAGE_DESC_T;

/// @brief MHAL WARP image data structure
typedef struct
{
    __u32 num_planes;                                   // number of image planes
    __u64 data[2];                                      // pointers to the image planes' data
} MHAL_WARP_IMAGE_DATA_T;

/// @brief MHAL WARP hardware configuration structure
typedef struct
{
    MHAL_WARP_OP_MODE_E op_mode;                        // Operation mode
    MHAL_WARP_DISPLAY_TABLE_T disp_table;               // Displacement table descriptor
    MHAL_WARP_BOUND_BOX_TABLE_T bb_table;               // Bounding box table descriptor

    int coeff[MHAL_WARP_PERSECTIVE_COEFFS_NUM_COEFFS];  // Perspective transform coefficients

    MHAL_WARP_IMAGE_DESC_T input_image;                 // Input image
    MHAL_WARP_IMAGE_DATA_T input_data;

    MHAL_WARP_IMAGE_DESC_T output_image;                // Output image
    MHAL_WARP_IMAGE_DATA_T output_data;

    __u8 fill_value[2];                                 // Fill value for out of range pixels

} MHAL_WARP_CONFIG;

#endif // _MDRV_WARP_IO_ST_H_
