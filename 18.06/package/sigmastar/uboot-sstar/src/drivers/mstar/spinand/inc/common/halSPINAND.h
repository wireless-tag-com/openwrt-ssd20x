/*
* halSPINAND.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: XXXX <XXXX@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef _HAL_SPINAND_H_
#define _HAL_SPINAND_H_

//-------------------------------------------------------------------------------------------------
//  Macro definition
//-------------------------------------------------------------------------------------------------
#define DEBUG_SPINAND(debug_level, x)     do { if (_u8SPINANDDbgLevel >= (debug_level)) (x); } while(0)

//-------------------------------------------------------------------------------------------------
//  Function and Variable
//-------------------------------------------------------------------------------------------------

MS_U32 HAL_SPINAND_Read (MS_U32 u32Addr, MS_U32 u32DataSize, MS_U8 *pu8Data);
MS_U32 HAL_SPINAND_Write( MS_U32 u32_PageIdx, MS_U8 *u8Data, MS_U8 *pu8_SpareBuf);
MS_U8  HAL_SPINAND_ReadID(MS_U32 u32DataSize, MS_U8 *pu8Data);
MS_U32 HAL_SPINAND_RFC(MS_U32 u32Addr, MS_U8 *pu8Data);
void HAL_SPINAND_PreHandle(SPINAND_MODE eMode);
void HAL_SPINAND_Config(MS_U32 u32PMRegBaseAddr, MS_U32 u32NonPMRegBaseAddr);
MS_U32 HAL_SPINAND_BLOCKERASE(MS_U32 u32_PageIdx);
U32 HAL_SPINAND_Init(void);
MS_U32 HAL_SPINAND_WriteProtect(MS_BOOL bEnable);
MS_BOOL HAL_SPINAND_PLANE_HANDLER(MS_U32 u32Addr);
MS_U32 HAL_SPINAND_SetMode(SPINAND_MODE eMode);
void HAL_SPINAND_CSCONFIG(void);
U8 HAL_SPINAND_WB_BBM(U32 u32LBA, U32 u32PBA);

extern MS_U8 _u8SPINANDDbgLevel;


#endif // _HAL_SPINAND_H_
