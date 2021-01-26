/*
* platform.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
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

// need review!!
/* used to identify FPGA and QC boards */
#define QC_BOARD            0
#define UBOOT_VERSION_FPGA      0 //KY: replace the COLUMBUS_FPGA
//#define ANDRIOD_LINUX       0

#define COLUMBUS    0
#define PIONEER     1
#define COLUMBUS2   2
#define CEDRIC      3
#define CHICAGO     4
#define PLATFORM    CHICAGO


//------------------------------------------------------------------------------
//
//  Define:  MSBASE_REG_RIU_PA
//
//  Locates the RIU register base.
//
#define MS_BASE_REG_RIU_PA					(0x1F000000)
#define MS_BASE_REG_UART0_PA              	GET_REG_ADDR(MS_BASE_REG_RIU_PA, 0x001D80)
#define MS_BASE_REG_TIMER2_PA              	GET_REG_ADDR(MS_BASE_REG_RIU_PA, 0x001DF0)
#define MS_BASE_REG_PMU_PA              	GET_REG_ADDR(MS_BASE_REG_RIU_PA, 0x001F80)
#define MS_BASE_REG_RTC_PA              	GET_REG_ADDR(MS_BASE_REG_RIU_PA, 0x001200)
#define MS_BASE_REG_EFUSE_PA              	GET_REG_ADDR(MS_BASE_REG_RIU_PA, 0x000C00)
#define MS_BASE_REG_SARADC_PA              	GET_REG_ADDR(MS_BASE_REG_RIU_PA, 0x001D00)
#define MS_BASE_REG_AUX_PA              	GET_REG_ADDR(MS_BASE_REG_RIU_PA, 0x000D80)
#define MS_BASE_REG_CHIPTOP_PA              GET_REG_ADDR(MS_BASE_REG_RIU_PA, 0x000F00)
#define MS_BASE_REG_PADTOP_PA               GET_REG_ADDR(MS_BASE_REG_RIU_PA, 0x000580)


#ifdef CONFIG_MS_ISP_FLASH
#define MS_SPI_ADDR       					(0x14000000)
#define MS_SPI_BOOT_ROM_SIZE                (0x00010000)
#define MS_SPI_UBOOT_SIZE                	(0x00080000)
#endif

typedef struct _DEVINFO{
    U8  header[8];
    U16 board_name;
    U8  package_name;
    U8  miu0_type;
    U8  miu0_size;
    U8  miu1_type;
    U8  miu1_size;
    U8  panel_type;
    U8  rtk_flag;
    U8  boot_device;
	U8  uartpad_select;
    U8  cpu_speed;
}DEVINFO_st;

typedef enum
{
	DEVINFO_BOOT_TYPE_SPI=0x01,
	DEVINFO_BOOT_TYPE_EMMC=0x02,
}DEVINFO_BOOT_TYPE;

typedef enum 
{
    E_SPI = 0x0,
    E_ROM = 0x1,
} EN_BOOT_SOURCE;


typedef enum
{
    DEVINFO_BD_MST154A_D01A_S    = 0x0801,
    DEVINFO_BD_MST786_SZDEMO     = 0x0802,
    DEVINFO_BD_MST786_CUSTOM10    = 0x0810,
    DEVINFO_BD_MST786_CUSTOM20    = 0x0820,
    DEVINFO_BD_MST786_CUSTOM30    = 0x0830,
    DEVINFO_BD_MST786_CUSTOM40    = 0x0840,    
    E_BD_UNKNOWN           = 0xFFFF
}DEVINFO_BOARD_TYPE;

typedef enum
{
    DEVINFO_PANEL_070_DEFAULT    = 0x0,
    DEVINFO_PANEL_HSD_070I_DW1   = 0x01,
    DEVINFO_PANEL_HSD_070P_FW3   = 0x02,
    DEVINFO_PANEL_AT_102_03TT91  = 0x03,
    DEVINFO_PANEL_HSD_062I_DW1   = 0x04,
    DEVINFO_PANEL_HSD_070I_DW2   = 0x05,
    DEVINFO_PANEL_HSD_LVDS_800480= 0x06,
    DEVINFO_PANEL_N070ICG_LD1     =0x08,
    DEVINFO_PANEL_HSD_062I_DW2   = 0x09,    
    E_PANEL_UNKNOWN           = 0xFF
}DEVINFO_PANEL_TYPE;

typedef enum
{
    DEVINFO_RTK_FLAG_0     = 0x0,
    DEVINFO_RTK_FLAG_1     = 0x01,
    E_RTK_UNKNOWN           = 0xFFFF
}DEVINFO_RTK_FLAG;

#endif // __PLATFORM_H__

/* 	END */
