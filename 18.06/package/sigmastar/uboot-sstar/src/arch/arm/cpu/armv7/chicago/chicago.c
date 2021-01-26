/*
* chicago.c- Sigmastar
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

#include <common.h>
#include "environment.h"
#include "asm/arch/mach/ms_types.h"
#include "asm/arch/mach/platform.h"
#include "asm/arch/mach/io.h"
#include "ms_version.h"

#ifndef MS_CHANGELIST
#define MS_CHANGELIST {'0', '0', '0', '0', '0', '0', '0', '0'}
#endif

typedef struct
{
    U8                       tag[4];
    U8                       flag[2];
    U8                       lib_type[2];
    U8                       bn[6];
    U8                       changelist[8];
    U8                       product[2];
    U8                       customer[1];
    U8                       reserved[6];
    U8                       checksum;

} MS_Version;

MS_Version version =
{
	{'M','S','V','C'},
	{'0','0'},
	CONFIG_MS_LIB_TYPE_2BYTE_STRING,
	{'U','B','O','O','T',' '},
	MS_CHANGELIST,
	{'C','4'},
	{'0'},
	{'0','0','0','0','0','0'},
    'T'
};


DECLARE_GLOBAL_DATA_PTR;

//#define DEV_BUF_ADDR (PHYS_SDRAM_1+0x3F800)


int arch_cpu_init(void)
{


	gd->xtal_clk=XTAL_26000K;


	/* following code is not worked since bd has not allocated yet!!*/
//
//	gd->bd->bi_arch_number = MACH_TYPE_COLUMBUS2;
//
//	/* adress of boot parameters */
//	gd->bd->bi_boot_params = BOOT_PARAMS;


    return 0;
}

// we borrow the DRAN init to do the devinfo setting...
int dram_init(void)
{

    int i=0;
    printf("\nUBOOT CL:");
    for(i=2;i<8;i++)
    {
        printf("%c",version.changelist[i]);
    }   
    printf("\n");

#ifdef CONFIG_CMD_BDI
    printf("***********************************************************\r\n");
    printf("* CHICAGO MEMORY LAYOUT                                    \r\n");
    printf("* PHYS_SDRAM_1:           0X%08x                           \r\n",PHYS_SDRAM_1);
    printf("* PHYS_SDRAM_1_SIZE:      0X%08x                           \r\n",PHYS_SDRAM_1_SIZE);
    printf("* CONFIG_SYS_TEXT_BASE:   0X%08x                           \r\n",CONFIG_SYS_TEXT_BASE);
    printf("* CONFIG_SYS_SDRAM_BASE:  0X%08x                           \r\n",CONFIG_SYS_SDRAM_BASE);
    printf("* CONFIG_SYS_INIT_SP_ADDR:0X%08x  (gd_t *)pointer          \r\n",CONFIG_SYS_INIT_SP_ADDR);
    printf("* SCFG_MEMP_START:        0X%08x                           \r\n",SCFG_MEMP_START);
    printf("* SCFG_PNLP_START:        0X%08x                           \r\n",SCFG_PNLP_START);
    printf("* BOOT_PARAMS:            0X%08x                           \r\n",BOOT_PARAMS);
    printf("* CONFIG_SYS_LOAD_ADDR:   0X%08x                           \r\n",CONFIG_SYS_LOAD_ADDR);
    //printf("* KERNEL_RAM_BASE:0X%08x                                   \r\n",KERNEL_RAM_BASE);
    printf("* CONFIG_UNLZO_DST_ADDR:  0X%08x                           \r\n",CONFIG_UNLZO_DST_ADDR);
    printf("\r\n");
    printf("* CONFIG_ENV_SIZE:        0X%08x                           \r\n",CONFIG_ENV_SIZE);
    printf("* CONFIG_SYS_MALLOC_LEN:  0X%08x                           \r\n",CONFIG_SYS_MALLOC_LEN);
    printf("* CONFIG_STACKSIZE:       0X%08x                           \r\n",CONFIG_STACKSIZE);
    printf("* KERNEL_IMAGE_SIZE:      0X%08x                           \r\n",KERNEL_IMAGE_SIZE);
    printf("***********************************************************\r\n");
#endif

#if CONFIG_ARCH_CHICAGO
    gd->boot_type=(int)DEVINFO_BOOT_TYPE_EMMC;
    gd->board_type=(int)E_BD_UNKNOWN;
    gd->ram_size=CONFIG_UBOOT_RAM_SIZE;
    gd->panel_type=(U8)E_PANEL_UNKNOWN;
    gd->rtk_flag = (int)0;
#endif
    return 0;
}

DEVINFO_BOOT_TYPE ms_devinfo_boot_type(void)
{
	//return (DEVINFO_BOOT_TYPE)gd->boot_type;
	return (DEVINFO_BOOT_TYPE)DEVINFO_BOOT_TYPE_EMMC;
}

DEVINFO_BOARD_TYPE ms_devinfo_board_type(void)
{
	return (DEVINFO_BOARD_TYPE)gd->board_type;
}

DEVINFO_BOARD_TYPE ms_devinfo_panel_type(void)
{
	return (DEVINFO_PANEL_TYPE)gd->panel_type;
}

DEVINFO_RTK_FLAG ms_devinfo_rtk_flag(void)
{
    return (DEVINFO_RTK_FLAG)gd->rtk_flag;
}


U16 OALReadBootSource(void)
{
	return ((INREG16(0x1F003C00+0x0065*4)&BIT2) == BIT2);
}

#ifndef CONFIG_MS_SAVE_ENV_IN_ISP_FLASH
#if (defined(CONFIG_MS_NAND) && defined(CONFIG_MS_EMMC))
extern void mmc_env_relocate_spec(void);
extern void nand_env_relocate_spec(void);
extern int mmc_env_init(void);
extern int nand_env_init(void);
extern int mmc_saveenv(void);
extern int nand_saveenv(void);

env_t *env_ptr;

void env_relocate_spec(void)
{
	if(DEVINFO_BOOT_TYPE_EMMC==ms_devinfo_boot_type())
	{
		return mmc_env_relocate_spec();
	}
	else
	{
		return nand_env_relocate_spec();
	}
}


int env_init(void)
{
	if(DEVINFO_BOOT_TYPE_EMMC==ms_devinfo_boot_type())
	{
		return mmc_env_init();
	}
	else
	{
		return nand_env_init();
	}

}

int saveenv(void)
{
	if(DEVINFO_BOOT_TYPE_EMMC==ms_devinfo_boot_type())
	{
		return mmc_saveenv();
	}
	else
	{
		return nand_saveenv();
	}
}
#endif
#endif


inline void _chip_flush_miu_pipe(void)
{
//	unsigned long   dwLockFlag = 0;
	unsigned short dwReadData = 0;

//	spin_lock_irqsave(&l2prefetch_lock, dwLockFlag);
	//toggle the flush miu pipe fire bit
	*(volatile unsigned short *)(0x1F203114) = 0x0;
	*(volatile unsigned short *)(0x1F203114) = 0x1;

	do
	{
		dwReadData = *(volatile unsigned short *)(0x1F203140);
		dwReadData &= (1<<12);  //Check Status of Flush Pipe Finish

	} while(dwReadData == 0);

//	spin_unlock_irqrestore(&l2prefetch_lock, dwLockFlag);

}

//void enable_caches(void)
//{
//	dcache_enable();
//	icache_enable();
//	puts("Caches enabled!!\n");
//}
//

