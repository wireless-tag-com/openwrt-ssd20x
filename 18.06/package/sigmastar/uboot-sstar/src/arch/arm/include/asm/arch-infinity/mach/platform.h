/*
* platform.h- Sigmastar
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

#ifndef	__PLATFORM_H__
#define	__PLATFORM_H__

//------------------------------------------------------------------------------
//  Include Files
//------------------------------------------------------------------------------
#include "asm/arch/mach/ms_types.h"

//------------------------------------------------------------------------------
//  Macros
//------------------------------------------------------------------------------

// Register macros
#define REG(Reg_Addr)       (*(volatile U16*)(Reg_Addr))
#define GET_REG_ADDR(x, y)  ((x) + ((y) << 2))
#define GET_BASE_ADDR_BY_BANK(x, y)  ((x) + ((y) << 1))

//------------------------------------------------------------------------------
//
//  Macros:  INREGx/OUTREGx/SETREGx/CLRREGx
//
//  This macros encapsulates basic I/O operations.
//  Memory address space operation is used on all platforms.
//
#define INREG8(x)           ms_readb(x)
#define OUTREG8(x, y)       ms_writeb((u8)(y), x)
#define SETREG8(x, y)       OUTREG8(x, INREG8(x)|(y))
#define CLRREG8(x, y)       OUTREG8(x, INREG8(x)&~(y))
#define INREGMSK8(x, y)     (INREG8(x) & (y))
#define OUTREGMSK8(x, y, z) OUTREG8(x, ((INREG8(x)&~(z))|((y)&(z))))

#define INREG16(x)              ms_readw(x)
#define OUTREG16(x, y)          ms_writew((u16)(y), x)
#define SETREG16(x, y)          OUTREG16(x, INREG16(x)|(y))
#define CLRREG16(x, y)          OUTREG16(x, INREG16(x)&~(y))
#define INREGMSK16(x, y)        (INREG16(x) & (y))
#define OUTREGMSK16(x, y, z)    OUTREG16(x, ((INREG16(x)&~(z))|((y)&(z))))

#define INREG32(x)              ms_readl(x)
#define OUTREG32(x, y)          ms_writel((u32)(y), x)
#define SETREG32(x, y)          OUTREG32(x, INREG32(x)|(y))
#define CLRREG32(x, y)          OUTREG32(x, INREG32(x)&~(y))
#define INREGMSK32(x, y)        (INREG32(x) & (y))
#define OUTREGMSK32(x, y, z)    OUTREG32(x, ((INREG32(x)&~(z))|((y)&(z))))


#define XTAL_26000K        26000000
#define XTAL_24000K        24000000
#define XTAL_16369K        16369000
#define XTAL_16367K        16367000


//-----------------------------------------------------------------------------

// Chip revisions
#define MS_REVISION_U01    (0x0)
#define MS_REVISION_U02    (0x1)
#define MS_REVISION_U03    (0x2)


//------------------------------------------------------------------------------
//
//  Macros:  TYPE_CAST
//
//  This macros interprets the logic of type casting and shows the old type and
//  the new type of the casted variable.
//
#define TYPE_CAST(OldType, NewType, Var)    ((NewType)(Var))

//------------------------------------------------------------------------------
//
//  Macros:  MSTAR_ASSERT
//
//  This macro implements the assertion no matter in Debug or Release build.
//
#define MSTAR_ASSERT_PRINT(exp,file,line) printk(("\r\n*** ASSERTION FAILED in ") (file) ("(") (#line) ("):\r\n") (#exp) ("\r\n"))
#define MSTAR_ASSERT_AT(exp,file,line) (void)( (exp) || (MSTAR_ASSERT_PRINT(exp,file,line), 0 ) )
#define MSTAR_ASSERT(exp) MSTAR_ASSERT_AT(exp,__FILE__,__LINE__)


//------------------------------------------------------------------------------
//
//  Define:  MSBASE_REG_RIU_PA
//
//  Locates the RIU register base.
//
#define MS_BASE_REG_RIU_PA					(0x1F000000)
#define MS_BASE_REG_IMI_PA                  (0xA0000000)
#define MS_BASE_REG_UART0_PA              	GET_BASE_ADDR_BY_BANK(MS_BASE_REG_RIU_PA, 0x110800)
#define MS_BASE_REG_TIMER0_PA              	GET_BASE_ADDR_BY_BANK(MS_BASE_REG_RIU_PA, 0x003020)
#define MS_BASE_REG_TIMER2_PA              	GET_BASE_ADDR_BY_BANK(MS_BASE_REG_RIU_PA, 0x003000)
#define MS_BASE_REG_CHIPTOP_PA              GET_BASE_ADDR_BY_BANK(MS_BASE_REG_RIU_PA, 0x101E00)
#define MS_BASE_REG_DID_KEY_PA              GET_BASE_ADDR_BY_BANK(MS_BASE_REG_RIU_PA, 0x003800)
#define MS_BASE_REG_PM_SLEEP_PA             GET_BASE_ADDR_BY_BANK(MS_BASE_REG_RIU_PA, 0x000E00)
#define MS_BASE_REG_PM_RTC_PA               GET_BASE_ADDR_BY_BANK(MS_BASE_REG_RIU_PA, 0x001200)
#define REG_ADDR_BASE_WDT                   GET_BASE_ADDR_BY_BANK(MS_BASE_REG_RIU_PA, 0x003000)
		#define WDT_WDT_CLR             (0x00)
		#define WDT_DUMMY_REG_1         (0x01)
		#define WDT_RST_RSTLEN          (0x02)
		#define WDT_INTR_PERIOD         (0x03)
		#define WDT_MAX_PRD_L           (0x04)
		#define WDT_MAX_PRD_H           (0x05)



#define MS_SPI_ADDR       					(0x14000000)
#define MS_SPI_BOOT_ROM_SIZE                (0x00010000)
#define MS_SPI_UBOOT_SIZE                	(0x00080000)

typedef enum
{
    DEVINFO_BOOT_TYPE_NONE=0x00,
	DEVINFO_BOOT_TYPE_NAND,
	DEVINFO_BOOT_TYPE_SPI,
	DEVINFO_BOOT_TYPE_EMMC,
	DEVINFO_BOOT_TYPE_SPINAND_EXT_ECC,
	DEVINFO_BOOT_TYPE_SPINAND_INT_ECC,
	DEVINFO_BOOT_TYPE_SDCARD,
}DEVINFO_BOOT_TYPE;




#define MIU0_START_ADDR         PHYS_SDRAM_1
//#define REG_ADDR_STATUS         (MS_BASE_REG_CHIPTOP_PA + (0x21 << 2)) //sync with IPL
#define REG_ADDR_STATUS         (MS_BASE_REG_PM_RTC_PA+ (0x04 << 2)) //sync with IPL

/*==========================================================================
    MXPT 2016/03/03 updated
===========================================================================*/
#define IPL_CUST_OFFSET         0x10000 //ROM(16K) + IPL(48K) = 64K
#define IPL_CUST_MAX_SIZE       0xF000  //60K
#define IPL_CUST_LOAD_ADDRESS  (MIU0_START_ADDR + 0x03C00000)
#define IPL_CUST_BIT            BIT0
#define BOARD_INFO_SPI_ADDRESS (MS_SPI_ADDR + IPL_CUST_OFFSET + 56*1024) // board info is located at IPL_CUST offset 56K

#define KEY_CUST_OFFSET         0x1F000 //ROM(16K) + IPL(48K) + IPL_CUST(60K) = 124K
#define KEY_CUST_MAX_SIZE       0x1000  //4K
#define KEY_CUST_LOAD_ADDRESS  (MIU0_START_ADDR + 0x03FFF000)
#define KEY_CUST_BIT            BIT1

/* DO NOT MODIFY MXPT_OFFSET */
#define MXPT_OFFSET             0x20000 //ROM(16K) + IPL(48K) + IPL_CUST(60K) + KEY_CUST(4K) = 128K
#define MXPT_MAX_SIZE           0x1000  //4K

#define DTB_OFFSET              0x21000 //ROM(16K) + IPL(48K) + IPL_CUST(60K) + KEY_CUST(4K) + MXPT(4K) = 132K
#define DTB_MAX_SIZE            0xF000  //60K
#define DTB_LOAD_ADDRESS       (MIU0_START_ADDR + 0x03FD0000)

#define UBOOT_OFFSET            0x30000 //ROM(16K) + IPL(48K) + IPL_CUST(60K) + KEY_CUST(4K) + MXPT(4K) + DTB(60K) = 192K
#define UBOOT_MAX_SIZE          0x1F000 //124K
#define UBOOT_MAX_DECOMP_SIZE   0x100000//1M
#define UBOOT_LOAD_ADDRESS     (MIU0_START_ADDR + 0x03F00000)
#define UBOOT_RUN_ADDRESS      (MIU0_START_ADDR + 0x03E00000)
#define UBOOT_BIT               BIT2

#define UBOOT_ENV_OFFSET        0x4F000 //
#define UBOOT_ENV_MAX_SIZE      0x1000  //4K

#define ZBOOT_OFFSET            0x50000 //
#define ZBOOT_MAX_SIZE          0x10000 //64K
#define ZBOOT_MAX_DECOMP_SIZE   0x100000//1M
#define ZBOOT_LOAD_ADDRESS     (MIU0_START_ADDR + 0x03D00000)
#define ZBOOT_RUN_ADDRESS      (MIU0_START_ADDR + 0x03E00000)
#define ZBOOT_BIT               BIT3

#define KERNEL_OFFSET           0x60000  //
#define KERNEL_MAX_SIZE         0x200000 //2M
#define KERNEL_MAX_DECOMP_SIZE  0x500000 //5M
#define KERNEL_LOAD_ADDRESS    (MIU0_START_ADDR + 0x03800000)
#define KERNEL_RUN_ADDRESS     (MIU0_START_ADDR + 0x00008000)
#define KERNEL_BIT              BIT4

#define FORCE_UBOOT_BIT         BIT15

#endif // __PLATFORM_H__

/* 	END */
