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

#ifndef _HAL_SPINAND_COMMON_H_
#define _HAL_SPINAND_COMMON_H_

//-------------------------------------------------------------------------------------------------
//  structure definition
//-------------------------------------------------------------------------------------------------

typedef struct
{
    U32  u32FspBaseAddr;     // REG_ISP_BASE
    U32  u32QspiBaseAddr;    // REG_QSPI_BASE
    U32  u32PMBaseAddr;      // REG_PM_BASE
    U32  u32CLK0BaseAddr;    // REG_PM_BASE
    U32  u32RiuBaseAddr;     // REG_PM_BASE
    U32  u32BDMABaseAddr;    // REG_BDMA_BASE
    U32  u32CHIPBaseAddr;    // REG_CHIPTOP_BASE
    U32  u32ISPBaseAddr;     // REG_ISP_BASE
    U32  u32ClkGenBaseAddr;     // REG_CLKGEN_BASE
} hal_fsp_t;

#endif // _HAL_SPINAND_COMMON_H_