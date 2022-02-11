/*
* mdrv_spinand_common.h- Sigmastar
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
#ifndef _DRV_SPINAND_COMMON_H_
#define _DRV_SPINAND_COMMON_H_


#define DEBUG_SPINAND(debug_level, x)     do { if (_u8SPINANDDbgLevel >= (debug_level)) (x); } while(0)

#ifndef U32
#define U32  unsigned long
#endif
#ifndef U16
#define U16  unsigned short
#endif
#ifndef U8
#define U8   unsigned char
#endif
#ifndef S32
#define S32  signed long
#endif
#ifndef S16
#define S16  signed short
#endif
#ifndef S8
#define S8   signed char
#endif
#ifndef NULL
#define NULL (void*)0
#endif
#ifndef BOOL
#define BOOL unsigned char
#endif

typedef enum
{
    E_WP_AREA_EXACTLY_AVAILABLE,
    E_WP_AREA_PARTIALLY_AVAILABLE,
    E_WP_AREA_NOT_AVAILABLE,
    E_WP_TABLE_NOT_SUPPORT,
} SPINAND_WP_AREA_e;

typedef enum _SPINAND_ERROR_NUM
{
    ERR_SPINAND_SUCCESS,
    ERR_SPINAND_RESET_FAIL,
    ERR_SPINAND_TIMEOUT,
    ERR_SPINAND_BDMA_TIMEOUT,
    ERR_SPINAND_BAD_BLK,
    ERR_SPINAND_E_FAIL,
    ERR_SPINAND_W_FAIL,
    ERR_SPINAND_INVALID,
    ERR_SPINAND_UNKNOWN_ID,
    ERR_SPINAND_ECC_1_3_CORRECTED,
    ERR_SPINAND_ECC_4_6_CORRECTED,
    ERR_SPINAND_ECC_7_8_CORRECTED,
    ERR_SPINAND_ECC_ERROR,
} SPINAND_FLASH_ERRNO_e;

typedef enum
{
    E_SPINAND_SINGLE_MODE,
    E_SPINAND_FAST_MODE,
    E_SPINAND_DUAL_MODE,
    E_SPINAND_DUAL_MODE_IO,
    E_SPINAND_QUAD_MODE,
    E_SPINAND_QUAD_MODE_IO,
} SPINAND_MODE;

typedef struct
{
    U8   u8_IDByteCnt;
    U8   au8_ID[15];
    U16  u16_SpareByteCnt;
    U16  u16_PageByteCnt;
    U16  u16_BlkPageCnt;
    U16  u16_BlkCnt;
    U16  u16_SectorByteCnt;
    U8   u8PlaneCnt;
    U8   u8WrapConfig;
    BOOL bRIURead;
    U8   u8CLKConfig;
    U8   u8_UBOOTPBA;
    U8   u8_BL0PBA;
    U8   u8_BL1PBA;
    U8   u8_HashPBA[3][2];
    U8   u8ReadMode;
    U8   u8WriteMode;
} SPINAND_FLASH_INFO_t;
#if !defined(TRUE) && !defined(FALSE)
/// definition for TRUE
#define TRUE                        1
/// definition for FALSE
#define FALSE                       0
#endif
#endif
