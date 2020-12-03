/*
* hal_movedma.h- Sigmastar
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

#ifndef __HAL_MOVE_DMA_H__
#define __HAL_MOVE_DMA_H__

/*=============================================================*/
// Include files
/*=============================================================*/

/*=============================================================*/
// Enumeration definition
/*=============================================================*/

typedef enum
{
    HAL_MOVEDMA_0 = 0,
    HAL_MOVEDMA_MAX
} HalMoveDmaId_e;

typedef enum
{
    HAL_MOVEDMA_NO_ERR           = 0,
    HAL_MOVEDMA_ERR_PARAM        = -1,
    HAL_MOVEDMA_POLLING_TIMEOUT  = -2
} HalMoveDmaErr_e;

/*=============================================================*/
// Structure definition
/*=============================================================*/

typedef void HalMoveDmaCBFunc(u32 argu);

typedef struct {
    u32                 u32SrcWidth;     ///< Width of source
    u32                 u32SrcOffset;    ///< Line-to-line offset of source
    u32                 u32DstWidth;     ///< Width of destination
    u32                 u32DstOffset;    ///< Line-to-line offset of destination
} HalMoveDmaLineOfst_t;

typedef struct {
    u32                 u32SrcAddr;
    u32                 u32SrcMiuSel;
    u32                 u32DstAddr;
    u32                 u32DstMiuSel;
    u32                 u32Count;
    HalMoveDmaCBFunc    *CallBackFunc;
    u32                 CallBackArg;
    u32                 bEnLineOfst;
    HalMoveDmaLineOfst_t *pstLineOfst;
} HalMoveDmaParam_t;

/*=============================================================*/
// Global function definition
/*=============================================================*/

HalMoveDmaErr_e HalMoveDma_Initialize(void);
HalMoveDmaErr_e HalMoveDma_MoveData(HalMoveDmaParam_t *ptMoveDmaParam);

#endif // __HAL_MOVE_DMA_H__

