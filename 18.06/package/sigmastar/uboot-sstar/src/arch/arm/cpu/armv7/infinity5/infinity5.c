/*
* infinity5.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: alterman.lin <alterman.lin@sigmastar.com.tw>
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

typedef struct
{
	unsigned char                       tagPrefix[3];
	unsigned char                       headerVersion[1];
	unsigned char                       libType[2];
	unsigned char                       chip[4];
	unsigned char                       changelist[8];
	unsigned char                       component[10];
	unsigned char                       reserved[1];
	unsigned char                       tagSuffix[3];


} MS_VERSION;

MS_VERSION UBT_VERSION =
{
	{'M','V','X'},
	MVXV_HEAD_VER,
	MVXV_LIB_TYPE, //R = general release version
	MVXV_CHIP_ID,
	MVXV_CHANGELIST,
	MVXV_COMP_ID,
	{'#'},
	{'X','V','M'},
};
DECLARE_GLOBAL_DATA_PTR;

//#define DEV_BUF_ADDR (PHYS_SDRAM_1+0x3F800)


int arch_cpu_init(void)
{

    gd->xtal_clk=CONFIG_PIUTIMER_CLOCK;



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
    gd->ram_size=CONFIG_UBOOT_RAM_SIZE;

    return 0;
}

DEVINFO_CHIP_TYPE ms_check_chip(void)
{
/*
	U16 chipType;
	chipType = INREG16(GET_REG_ADDR(REG_ADDR_BASE_MIU, 0x69)) & 0xF000;

	if (chipType == 0x6000)
		return DEVINFO_313E;
	else if (chipType == 0x8000)
		return DEVINFO_318;
	else if (chipType == 0x9000)
		return DEVINFO_318;
	else
		return DEVINFO_NON;
*/
	return DEVINFO_NON;
}

int checkboard(void)
{
/*
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
*/
        int i=0;

        printf("Version: ");
        for(i=0;i<2;i++)
        {
            printf("%c",UBT_VERSION.chip[i]);
        }
        for(i=0;i<8;i++)
        {
            printf("%c",UBT_VERSION.changelist[i]);
        }
        printf("\n");
        /*
		switch (ms_check_chip())
		{
			case DEVINFO_313E:
				printf("DEVINFO: 313E\n");
				break;
			case DEVINFO_318:
				printf("DEVINFO: 318\n");
				break;
			case DEVINFO_NON:
				printf("DEVINFO: NON\n");
				break;
			default:
				printf("DEVINFO:ERROR\n");
		}
        */

#ifdef CONFIG_CMD_BDI
        printf("***********************************************************\r\n");
        printf("* MEMORY LAYOUT                                            \r\n");
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
	return 0;
}



DEVINFO_BOOT_TYPE ms_devinfo_boot_type(void)
{
    U16 hwstrap,b_extEcc;
    hwstrap = INREG16( GET_REG_ADDR(MS_BASE_REG_DID_KEY_PA, 0x70));
	b_extEcc = INREG16( GET_REG_ADDR(REG_ADDR_BASE_PADTOP, 0x72)) & BIT0;

    if(hwstrap&BIT5)
    {
        return DEVINFO_BOOT_TYPE_SPI;
    }
    else if(hwstrap&BIT4)
    {
        return DEVINFO_BOOT_TYPE_NAND;
    }
    else if(hwstrap&BIT3)
    {
        return DEVINFO_BOOT_TYPE_EMMC;
    }
    else if((hwstrap&BIT2))
    {
		if(b_extEcc)
			return DEVINFO_BOOT_TYPE_SPINAND_INT_ECC;
		else
		return DEVINFO_BOOT_TYPE_SPINAND_EXT_ECC;
    }
    else
    {
        return DEVINFO_BOOT_TYPE_NONE;
    }
}


/*

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
*/

#ifdef CONFIG_BOARD_LATE_INIT
extern void *malloc(unsigned int);
#define BUF_SIZE 256

int board_late_init(void)
{
    /*
    U16 u16Status;
    char *chainmode="CBC";
    char *keytype="EFUSE";
    char *buffer;

    printf("*******************************************\nboard_late_init for upgrade image\n");

    buffer =(char *)malloc(BUF_SIZE);
    if((buffer==NULL))
    {
        printf("no memory for command string!!\n");
        return -1;
    }

    u16Status=INREG16(REG_ADDR_STATUS);
    if(u16Status)
    {
        printf("Status register=0x%08X\n", u16Status);
        if(u16Status & KEY_CUST_BIT)
        {
            memset(buffer, 0 , BUF_SIZE);
            sprintf(buffer, "aes enc %x %s %s KEY_CUST", KEY_CUST_LOAD_ADDRESS, chainmode, keytype);
            printf("%s\n", buffer);
            run_command(buffer, 0);
        }
        if(u16Status & ZBOOT_BIT)
        {
            memset(buffer, 0 , BUF_SIZE);
            sprintf(buffer, "aes enc %x %s %s ZBOOT", ZBOOT_LOAD_ADDRESS, chainmode, keytype);
            printf("%s\n", buffer);
            run_command(buffer, 0);
        }
        if(u16Status & UBOOT_BIT)
        {
            memset(buffer, 0 , BUF_SIZE);
            sprintf(buffer, "aes enc %x %s %s UBOOT", UBOOT_LOAD_ADDRESS, chainmode, keytype);
            printf("%s\n", buffer);
            run_command(buffer, 0);
        }
        if(u16Status & KERNEL_BIT)
        {
            memset(buffer, 0 , BUF_SIZE);
            sprintf(buffer, "aes enc %x %s %s KERNEL", KERNEL_LOAD_ADDRESS, chainmode, keytype);
            printf("%s\n", buffer);
            run_command(buffer, 0);
        }
        // *(volatile U16*)0x1F001CB8 = 0xFF;
        // *(volatile U16*)0x1F001CB8 = 0x79;
        //run_command("reset", 0);
        reset_cpu(0);
    }
    printf("*******************************************\n");
    */
    return 0;
}
#endif

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
	else if(DEVINFO_BOOT_TYPE_NAND==ms_devinfo_boot_type())
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
	else if(DEVINFO_BOOT_TYPE_NAND==ms_devinfo_boot_type())
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
	else if(DEVINFO_BOOT_TYPE_NAND==ms_devinfo_boot_type())
	{
		return nand_saveenv();
	}
}
#endif
#endif

#define  reg_flush_op_on_fire           (0x1F000000 + 0x102200*2 + 0x05*4)
#define  reg_Flush_miu_pipe_done_flag   (0x1F000000 + 0x102200*2 + 0x10*4)

inline void _chip_flush_miu_pipe(void)
{

	unsigned short dwReadData = 0;

	//toggle the flush miu pipe fire bit
	*(volatile unsigned short *)(reg_flush_op_on_fire) = 0x10;
	*(volatile unsigned short *)(reg_flush_op_on_fire) = 0x11;

	do
	{
		dwReadData = *(volatile unsigned short *)(reg_Flush_miu_pipe_done_flag);
		dwReadData &= (1<<12);  //Check Status of Flush Pipe Finish

	} while(dwReadData == 0);
}

extern void Chip_Flush_Memory(void);
extern void Chip_Read_Memory(void);

inline void Chip_Flush_Memory(void)
{
	_chip_flush_miu_pipe();
}

inline void Chip_Read_Memory(void)
{
	_chip_flush_miu_pipe();
}
