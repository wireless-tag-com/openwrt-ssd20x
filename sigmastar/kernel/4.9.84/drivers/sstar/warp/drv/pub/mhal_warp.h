/*
* mhal_warp.h- Sigmastar
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
#ifndef __MHAL_WARP_H__
#define __MHAL_WARP_H__

///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Sigmastar Semiconductor, Inc. All rights reserved.
//
// Wrapper device interface in kernel space
///////////////////////////////////////////////////////////////////////////////
#include "mhal_common.h"

typedef void* MHAL_WARP_DEV_HANDLE;
typedef void* MHAL_WARP_INST_HANDLE;

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
    MS_S32 y;
    MS_S32 x;
} MHAL_WARP_DISPPLAY_ABSOLUTE_ENTRY_T;

/// @brief MHAL WARP displacement map entry for coordinates' relative offset format
typedef struct
{
    MS_S16 y;
    MS_S16 x;
} MHAL_WARP_DISPPLAY_RELATIVE_ENTRY_T;

/// @brief MHAL WARP displacement table descriptor
typedef struct
{
    MHAL_WARP_MAP_RESLOUTION_E resolution;          // map resolution
    MHAL_WARP_MAP_FORMAT_E     format;                  // map format (absolute, relative)
    MS_S32 size;                                                        // size of Disp.
    MS_PHY table;
} MHAL_WARP_DISPLAY_TABLE_T;

typedef struct
{
    MS_S16 height;                                      // input tile height
    MS_S16 width;                                       // input tile width
    MS_S16 y;                                           // input tile top left y coordinate
    MS_S16 x;                                           // input tile top left x coordinate
}  MHAL_WARP_BOUND_BOX_ENTRY_T;

typedef struct
{
    MS_S32 size;                                                               // bytes of bound boxes
    MS_PHY table;           // list of all bound boxes
}MHAL_WARP_BOUND_BOX_TABLE_T;

/// @brief MHAL WARP image descriptor
typedef struct
{
    MHAL_WARP_IMAGE_FORMAT_E format;                    // image format
    MS_U32 width;                                       // image width  (for YUV - Y plane width)
    MS_U32 height;                                      // image height (for YUV - Y plane width)
} MHAL_WARP_IMAGE_DESC_T;

/// @brief MHAL WARP image data structure
typedef struct
{
    MS_U32 num_planes;                                  // number of image planes
    MS_PHY data[2];                                         // pointers to the image planes' data
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

    MS_U8 fill_value[2];                                // Fill value for out of range pixels

} MHAL_WARP_CONFIG;

//------------------------------------------------------------------------------
/// @brief Get warp IRQ id
/// @return IrqId
//------------------------------------------------------------------------------
MS_U32 MHAL_WARP_GetIrqNum(void);

/// @brief MHAL WARP call back for process done
/// @param[in]  instance: Instance handle.
/// @param[in]  user_data: user data passed by MHAL_WARP_Trigger()
/// @return 0: Instance is not ready for next request, MHAL_WARP_ReadyForNext()
///            has to be called to unlock instance
///         1: Instance is ready for next request
typedef  MS_BOOL (*MHAL_WARP_CALLBACK)(MHAL_WARP_INST_HANDLE instance, void *user_data);

//------------------------------------------------------------------------------
/// @brief Get Device Handle
/// @param[in]  id: Device Core ID.
/// @param[out] device: Pointer to device handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_WARP_CreateDevice(MS_U32 id, MHAL_WARP_DEV_HANDLE *device);

//------------------------------------------------------------------------------
/// @brief Destroy Device Handle
/// @param[in]  device: Device handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_WARP_DestroyDevice(MHAL_WARP_DEV_HANDLE device);

//------------------------------------------------------------------------------
/// @brief Get Instance Handle
/// @param[in]  device: Device handle.
/// @param[out] instance: Pointer to instance handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_WARP_CreateInstance(MHAL_WARP_DEV_HANDLE device, MHAL_WARP_INST_HANDLE *instance);

//------------------------------------------------------------------------------
/// @brief Destroy Instance Handle
/// @param[in]  instance: Instance handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_WARP_DestroyInstance(MHAL_WARP_INST_HANDLE instance);

//------------------------------------------------------------------------------
/// @brief Trigger Wrap Process
/// @param[in]  instance: Instance handle.
/// @param[in]  config: configuration.
/// @param[in]  data:
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MS_S32 MHAL_WARP_Trigger(MHAL_WARP_INST_HANDLE instance, MHAL_WARP_CONFIG* config, MHAL_WARP_CALLBACK callback, void *user_data);

//------------------------------------------------------------------------------
/// @brief Check Instance status
/// @param[in]  instance: Instance handle.
/// @return MHAL_ErrCode_e
//------------------------------------------------------------------------------
MHAL_WARP_INSTANCE_STATE_E MHAL_WARP_CheckState(MHAL_WARP_INST_HANDLE instance);

//------------------------------------------------------------------------------
/// @brief Set state of Warp instance to be ready if process done
/// @param[in]  instance: Instance handle.
/// @return  MHAL_WARP_INSTANCE_STATE_E
//------------------------------------------------------------------------------
MHAL_WARP_INSTANCE_STATE_E MHAL_WARP_ReadyForNext(MHAL_WARP_INST_HANDLE instance);

//------------------------------------------------------------------------------
/// @brief Device Interrupt Processing
/// @param[in]  device: Device handle.
/// @return MHAL_WARP_ISR_STATE_E
//------------------------------------------------------------------------------
MHAL_WARP_ISR_STATE_E MHAL_WARP_IsrProc(MHAL_WARP_DEV_HANDLE device);


#endif /* __MHAL_WARP_H__ */
