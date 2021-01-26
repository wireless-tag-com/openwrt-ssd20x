/*
* cedric.c- Sigmastar
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
	unsigned char                       tagPrefix[3];
	unsigned char                       headerVersion[1];
	unsigned char                       libType[1];
	unsigned char                       chip[2];
	unsigned char                       changelist[8];
	unsigned char                       component[12];
	unsigned char                       reserved[2];
	unsigned char                       tagSuffix[3];


} MS_VERSION;

MS_VERSION version =
{
	{'M','V','X'},
	MVXV_HEAD_VER,
	MVXV_LIB_TYPE, // could be 'S' for small version
	{'C','3'},
	MVXV_CHANGELIST,
	MVXV_COMP_ID,
	{'#','#'},
	{'X','V','M'},
};




DECLARE_GLOBAL_DATA_PTR;

#define DEV_BUF_ADDR (PHYS_SDRAM_1+0x3F800)


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

int dram_init(void)
{



    /* following code is not worked since bd has not allocated yet!!*/
    //    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    //    gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

    return 0;
}

int checkboard(void)
{

    DEVINFO_st* devinfo;
    int i;


	devinfo = (DEVINFO_st*)(DEV_BUF_ADDR);    // start address: 30K, size:  2K

//	for(i=0;i<sizeof(version.bn);i++)
//	{
//		putc(version.bn[i]);
//	}
//	for(i=0;i<sizeof(version.lib_type);i++)
//	{
//		putc(version.lib_type[i]);
//	}
//
//	putc('@');
	for(i=0;i<sizeof(version.changelist);i++)
	{
		putc(version.changelist[i]);
	}	
	putc('\n');
     
	if (devinfo->header[0]!='D' || devinfo->header[1]!='E' || devinfo->header[2]!='V' || devinfo->header[3]!='I' ||
		devinfo->header[4]!='N' || devinfo->header[5]!='F' || devinfo->header[6]!='O' || devinfo->header[7]!='.')
	{
		printf("***********************************************************\r\n");
		printf("* DEVINFO. header check failed !!!                        *\r\n");
		printf("* Please program DEVINFO section.                         *\r\n");
		printf("***********************************************************\r\n");
		while(1);
	}


    gd->boot_type=(int)devinfo->boot_device;
    gd->board_type=(int)devinfo->board_name;
    gd->ram_size=CONFIG_UBOOT_RAM_SIZE;
    gd->panel_type=(U8)devinfo->panel_type;
    gd->rtk_flag = (int)devinfo->rtk_flag;
	return 0;
}


DEVINFO_BOOT_TYPE ms_devinfo_boot_type(void)
{
	return (DEVINFO_BOOT_TYPE)gd->boot_type;
	//return (DEVINFO_BOOT_TYPE)DEVINFO_BOOT_TYPE_EMMC;
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

inline void Chip_Flush_Memory(void)
{
	_chip_flush_miu_pipe();
}

inline void Chip_Read_Memory(void)
{
	_chip_flush_miu_pipe();
}


U32 MsOS_PA2KSEG1(U32 addr)
{
	return ((U32)(((U32)addr) | 0x80000000));
}

U32 MsOS_PA2KSEG0(U32 addr)
{
	return ((U32)(((U32)addr) | 0x40000000));
}

U32 MsOS_VA2PA(U32 addr)
{
	return ((U32)(((U32)addr) & 0x1fffffff));
}



//void enable_caches(void)
//{
//	dcache_enable();
//	icache_enable();
//	puts("Caches enabled!!\n");
//}
//

