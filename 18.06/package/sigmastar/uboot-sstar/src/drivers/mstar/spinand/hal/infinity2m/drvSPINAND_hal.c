/*
* drvSPINAND_hal.c- Sigmastar
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

#include <linux/string.h>
#include <common.h>

// Common Definition
#include "MsCommon.h"
#include "MsIRQ.h"
#include "MsTypes.h"
#include "halSPINAND_common.h"
#include "../../inc/common/drvSPINAND.h"

extern hal_fsp_t _hal_fsp;
extern U32 _HAL_FSP_SendOneCmd(U8 u8Cmd);
extern U32 _HAL_FSP_CHECK_SPINAND_DONE(U8 *pu8Status);
//-------------------------------------------------------------------------------------------------
//  Macro definition
//-------------------------------------------------------------------------------------------------
#define REG_16(bank,addr) (*(volatile unsigned short*)(0x1F000000 | (bank << 9) | (addr << 2)))
#define REG_GPIO(addr) REG_16(0xF, addr)
#define QSPI_READ(addr)                     READ_WORD(_hal_fsp.u32QspiBaseAddr + (addr<<2))
#define QSPI_WRITE(addr, val)               WRITE_WORD(_hal_fsp.u32QspiBaseAddr + (addr<<2),(val))
#define CHIP_WRITE(addr, val)               WRITE_WORD(_hal_fsp.u32CHIPBaseAddr + (addr<<2),(val))
#define CHIP_READ(addr)                     READ_WORD(_hal_fsp.u32CHIPBaseAddr + (addr<<2))
#define PM_READ(addr)                       READ_WORD(_hal_fsp.u32PMBaseAddr + (addr<<2))
#define PM_WRITE(addr, val)                 WRITE_WORD(_hal_fsp.u32PMBaseAddr + (addr<<2),(val))
#define PM_WRITE_MASK(addr, val, mask)      WRITE_WORD_MASK(_hal_fsp.u32PMBaseAddr+ ((addr)<<2), (val), (mask))


U32 HAL_SPINAND_Init(void)
{
    U32 u32Ret;
    U8 u8Status;

    //set pad mux for spinand
	CHIP_WRITE(0x50, 0x000);//disable all pad in
    QSPI_WRITE(0x7A, 0x00); //CS use SPI#1
    PM_WRITE(0x35, 0x08);   //SPI_CZ

    // reset spinand
    //FSP init config
    u32Ret = _HAL_FSP_SendOneCmd(SPI_NAND_CMD_RESET);
    if(u32Ret != ERR_SPINAND_SUCCESS)
    {
        return u32Ret;
    }
    return _HAL_FSP_CHECK_SPINAND_DONE(&u8Status);
}

BOOL PalSpinand_SetDriving(U32 u32Pins, u16 u16Driving)
{
    if(u16Driving == 1)
    {
        if(u32Pins & E_SPI_PIN_CZ)    REG_GPIO(0x1) |= BIT(12);
        if(u32Pins & E_SPI_PIN_CK)    REG_GPIO(0x2) |= BIT(12);
        if(u32Pins & E_SPI_PIN_DI)    REG_GPIO(0x3) |= BIT(12);
        if(u32Pins & E_SPI_PIN_DO)    REG_GPIO(0x4) |= BIT(12);
        if(u32Pins & E_SPI_PIN_WPZ)   REG_GPIO(0x5) |= BIT(12);
        if(u32Pins & E_SPI_PIN_HOLDZ) REG_GPIO(0x5) |= BIT(12);
    }
    else
    {
        if(u32Pins & E_SPI_PIN_CZ)    REG_GPIO(0x1) &= ~(BIT(12));
        if(u32Pins & E_SPI_PIN_CK)    REG_GPIO(0x2) &= ~(BIT(12));
        if(u32Pins & E_SPI_PIN_DI)    REG_GPIO(0x3) &= ~(BIT(12));
        if(u32Pins & E_SPI_PIN_DO)    REG_GPIO(0x4) &= ~(BIT(12));
        if(u32Pins & E_SPI_PIN_WPZ)   REG_GPIO(0x5) &= ~(BIT(12));
        if(u32Pins & E_SPI_PIN_HOLDZ) REG_GPIO(0x5) &= ~(BIT(12));
    }
    printf("i2m set driving\n");
    return TRUE;
}


typedef struct
{
    U8  u8Clk;
    U16 eClkCkg;
} hal_clk_ckg_t;

static hal_clk_ckg_t _hal_ckg_spi_pm[] = {
     {12 , PM_SPI_CLK_XTALI }
    ,{27 , PM_SPI_CLK_27MHZ }
    ,{36 , PM_SPI_CLK_36MHZ }
    ,{43 , PM_SPI_CLK_43MHZ }
    ,{54 , PM_SPI_CLK_54MHZ }
    ,{72 , PM_SPI_CLK_72MHZ }
    ,{86 , PM_SPI_CLK_86MHZ }
    ,{108, PM_SPI_CLK_108MHZ}
};

BOOL HAL_SPINAND_DumpCkg(void)
{
    BOOL bRet = TRUE;
    U16 u16Reg;
    U8 u8Size, u8Idx, u8CkgSpi;
    hal_clk_ckg_t *pTable;

    u16Reg = PM_READ(REG_PM_CKG_SPI) & PM_SPI_CLK_SEL_MASK;
    u8Size = sizeof(_hal_ckg_spi_pm)/ sizeof(hal_clk_ckg_t);
    pTable = _hal_ckg_spi_pm;
    for(u8Idx = 0; u8Idx < u8Size; u8Idx++)
    {
        if(u16Reg == pTable[u8Idx].eClkCkg)
        {
            u8CkgSpi = pTable[u8Idx].u8Clk;
            break;
        }
    }
    if(u8Idx == u8Size)
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("Unknown PM CLK %X\r\n", u16Reg));
        bRet = FALSE;
    }
    else
    {
        DEBUG_SPINAND(E_SPINAND_DBGLV_ERR, printf("found i2m PM %d MHz\r\n", u8CkgSpi));
    }
    return bRet;
}

