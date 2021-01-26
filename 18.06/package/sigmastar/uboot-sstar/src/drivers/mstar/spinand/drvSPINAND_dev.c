/*
* drvSPINAND_dev.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: edie.chen <edie.chen@sigmastar.com.tw>
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

/*
 *  Collect Device Dependent functions
 */

#include <linux/string.h>
#include <common.h>

// Common Definition
#include "MsCommon.h"
#include "MsIRQ.h"
#include "MsTypes.h"
#include "halSPINAND_common.h"
//#include "MsOS.h"
//#include "drvMMIO.h"
#include "../../inc/common/drvSPINAND.h"

//#ifndef _DRV_SPICMD_H_
//#include "../../inc/common/drvSPICMD.h"
//#endif
#include "../../inc/common/spinand.h"

//-------------------------------------------------------------------------------------------------
//  Macro definition
//-------------------------------------------------------------------------------------------------
#define WRITE_WORD_MASK(_reg, _val, _mask)  { (*((volatile U16*)(_reg))) = ((*((volatile U16*)(_reg))) & ~(_mask)) | ((U16)(_val) & (_mask)); }
#define BDMA_READ(addr)                     READ_WORD(_hal_fsp.u32BDMABaseAddr + (addr<<2))
#define BDMA_WRITE(addr, val)               WRITE_WORD(_hal_fsp.u32BDMABaseAddr + (addr<<2),(val))
#define QSPI_READ(addr)                     READ_WORD(_hal_fsp.u32QspiBaseAddr + (addr<<2))
#define QSPI_WRITE(addr, val)               WRITE_WORD(_hal_fsp.u32QspiBaseAddr + (addr<<2),(val))
#define CLK_WRITE(addr, val)                WRITE_WORD(_hal_fsp.u32CLK0BaseAddr + (addr<<2),(val))
#define CLK_READ(addr)                      READ_WORD(_hal_fsp.u32CLK0BaseAddr + (addr<<2))
#define CLK_WRITE_MASK(addr, val, mask)     WRITE_WORD_MASK(_hal_fsp.u32CLK0BaseAddr + ((addr)<<2), (val), (mask))
#define CHIP_WRITE(addr, val)               WRITE_WORD(_hal_fsp.u32CHIPBaseAddr + (addr<<2),(val))
#define CHIP_READ(addr)                     READ_WORD(_hal_fsp.u32CHIPBaseAddr + (addr<<2))
#define PM_READ(addr)                       READ_WORD(_hal_fsp.u32PMBaseAddr + (addr<<2))
#define PM_WRITE(addr, val)                 WRITE_WORD(_hal_fsp.u32PMBaseAddr + (addr<<2),(val))
#define PM_WRITE_MASK(addr, val, mask)      WRITE_WORD_MASK(_hal_fsp.u32PMBaseAddr+ ((addr)<<2), (val), (mask))
#define ISP_READ(addr)                      READ_WORD(_hal_fsp.u32ISPBaseAddr + (addr<<2))
#define CLKGEN_READ(addr)                   READ_WORD(_hal_fsp.u32ClkGenBaseAddr + (addr<<2))
#define CLKGEN_WRITE(addr, val)             WRITE_WORD(_hal_fsp.u32ClkGenBaseAddr + (addr<<2),(val))
#define CLKGEN_WRITE_MASK(addr, val, mask)  WRITE_WORD_MASK(_hal_fsp.u32ClkGenBaseAddr + ((addr)<<2), (val), (mask))

//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

U32 GD_SPINAND_SetDriving(U16 u16Driving)
{
    U8 status = 0;
    U32 u32Ret;
    U8 val, u8NewV;
    BOOL bSet = FALSE;
    u32Ret = MDrv_SPINAND_ReadStatusRegister((U8*)&status, SPI_NAND_REG_FUT);
    //S1/S0: BIT5-BiT6
    // 0 0 : 50
    // 0 1 :25
    // 1 0 :75
    // 1 1 : 100
    if(u32Ret == ERR_SPINAND_SUCCESS)
    {
        val = status & (BIT(5) | BIT(6));
        u8NewV = status & (~(BIT(5) | BIT(6)));
        printf("Set driving :%d Reg.Value from %X\n", u16Driving, status);
        if(u16Driving == 0 || u16Driving == 50)
        {
            if(val != 0)
            {
                bSet = TRUE;
            }
        }
        else if(u16Driving == 1 || u16Driving == 25)
        {
            if(val != BIT(5))
            {
                u8NewV |= BIT(5);
                bSet = TRUE;
            }
        }
        else if(u16Driving == 2 || u16Driving == 75)
        {
            if(val != BIT(6))
            {
                u8NewV |= BIT(6);
                bSet = TRUE;
            }
        }
        else if(u16Driving == 3 || u16Driving == 100)
        {
            if(val != (BIT(5) | BIT(6)))
            {
                u8NewV |= BIT(5) | BIT(6);
                bSet = TRUE;
            }
        }
        else {
            printk("Invalid value 0x%X", u16Driving);
            return ERR_SPINAND_INVALID;
        }
        if(bSet)
        {
            printf("to %X\n", u8NewV);
            u32Ret = MDrv_SPINAND_WriteStatusRegister((U8*)&u8NewV, SPI_NAND_REG_FUT);
        }
    }
    return u32Ret;
}
