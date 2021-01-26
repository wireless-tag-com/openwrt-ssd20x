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

#ifndef _DRV_DISP_IRQ_H_
#define _DRV_DISP_IRQ_H_

//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  Enum
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  structure
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------


#ifdef _DRV_DISP_IRQ_C_
#define INTERFACE
#else
#define INTERFACE extern
#endif
INTERFACE void DrvDispIrqSetIsrNum(u32 u32DevId, u32 u32IsrNum);
INTERFACE bool DrvDispIrqGetIsrNum(void *pDevCtx, u32 *pu32IsrNum);
INTERFACE bool DrvDispIrqEnable(void *pDevCtx, u32 u32DevIrq, bool bEnable);
INTERFACE bool DrvDispIrqGetFlag(void *pDevCtx, MHAL_DISP_IRQFlag_t *pstIrqFlag);
INTERFACE bool DrvDispIrqClear(void *pDevCtx, void* pData);

INTERFACE bool DrvDispIrqCreateInternalIsr(void);
INTERFACE bool DrvDispIrqDestroyInternalIsr(void);

#undef INTERFACE
#endif
