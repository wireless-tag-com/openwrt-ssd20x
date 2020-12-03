/*
* ms_platform.h- Sigmastar
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
#ifndef __MS_PLATFORM_H__
#define __MS_PLATFORM_H__

#include "mdrv_device_id.h"
#include "linux/proc_fs.h"

#define MS_IO_OFFSET 0xDE000000
/* macro to get at MMIO space when running virtually */
#define IO_ADDRESS(x)           ( (u32)(x) + MS_IO_OFFSET )
//#define __io_address(n)       ((void __iomem __force *)IO_ADDRESS(n))

// Register macros
#define GET_REG_ADDR(x, y)           ((x) + ((y) << 2))
#define GET_BASE_ADDR_BY_BANK(x, y)  ((x) + ((y) << 1))

/* read register by byte */
#define ms_readb(a) (*(volatile unsigned char *)IO_ADDRESS(a))

/* read register by word */
#define ms_readw(a) (*(volatile unsigned short *)IO_ADDRESS(a))

/* read register by long */
#define ms_readl(a) (*(volatile unsigned int *)IO_ADDRESS(a))

/* write register by byte */
#define ms_writeb(v,a) (*(volatile unsigned char *)IO_ADDRESS(a) = (v))

/* write register by word */
#define ms_writew(v,a) (*(volatile unsigned short *)IO_ADDRESS(a) = (v))

/* write register by long */
#define ms_writel(v,a) (*(volatile unsigned int *)IO_ADDRESS(a) = (v))


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


#define PM_READ32(x)        ((((unsigned int)INREG16(x+4)<<16)&0xFFFF0000) | (INREG16(x) & 0x0000FFFF))
#define PM_WRITE32(x,y)      do{OUTREG16(x,(y&0x0000FFFF));OUTREG16((x+4),((y>>16)&0x0000FFFF));}while(0)

#define XTAL_26000K        26000000
#define XTAL_24000K        24000000
#define XTAL_16369K        16369000
#define XTAL_16367K        16367000

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

struct MS_BIN_OPTION
{
    u8 name[8];
    u8 args[8];
};

typedef enum
{
    MS_BOOT_DEV_NONE=0x00,
    MS_BOOT_DEV_SPI=0x01,
    MS_BOOT_DEV_EMMC=0x02,
    MS_BOOT_DEV_8051=0x03,
} MS_BOOT_DEV_TYPE;

typedef enum
{
    MS_STORAGE_UNKNOWN =0x00,
    MS_STORAGE_NOR,
    MS_STORAGE_NAND,
    MS_STORAGE_EMMC,
    MS_STORAGE_SPINAND_ECC,
    MS_STORAGE_SPINAND_NOECC,
} MS_STORAGE_TYPE;

extern void     Chip_Flush_MIU_Pipe(void);
extern  void    Chip_Flush_Memory(void);
extern  void    Chip_Read_Memory(void);

extern  int     Chip_Cache_Outer_Is_Enabled(void);
extern  void    Chip_Flush_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
extern  void    Chip_Clean_Cache_Range_VA_PA(unsigned long u32VAddr,unsigned long u32PAddr,unsigned long u32Size);
extern  void    Chip_Flush_Cache_Range(unsigned long u32Addr, unsigned long u32Size);
extern  void    Chip_Clean_Cache_Range(unsigned long u32Addr, unsigned long u32Size);
extern  void    Chip_Inv_Cache_Range(unsigned long u32Addr, unsigned long u32Size);
extern  void    Chip_Flush_CacheAll(void);

extern  u64     Chip_Phys_to_MIU(u64 phys);
extern  u64     Chip_MIU_to_Phys(u64 miu);

extern  char*   Chip_Get_Platform_Name(void);
extern  int     Chip_Get_Device_ID(void);
extern  int     Chip_Get_Revision(void);

extern  int     Chip_Boot_Get_Dev_Type(void);
extern unsigned long long Chip_Get_RIU_Phys(void);
extern int Chip_Get_RIU_Size(void);
extern int Chip_Function_Set(int function_id, int param);

extern int Chip_Get_Storage_Type(void);
extern int Chip_Get_Package_Type(void);
extern u64 Chip_Get_US_Ticks(void);

typedef enum
{
    CHIP_FUNC_UART_ENABLE_LINE=0x0001,
    CHIP_FUNC_UART_DISABLE_LINE,
    CHIP_FUNC_IR_ENABLE,
    CHIP_FUNC_USB_VBUS_CONTROL,
    CHIP_FUNC_MCM_DISABLE_ID,
    CHIP_FUNC_MCM_ENABLE_ID,
    CHIP_FUNC_END,
    CHIP_FUNC_DUMMY=0xFFFF
} EN_CHIP_FUNC_ID;

#endif
