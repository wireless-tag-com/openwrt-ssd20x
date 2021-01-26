#ifndef __CONFIG_H
#define __CONFIG_H

/*------------------------------------------------------------------------------
    Constant
-------------------------------------------------------------------------------*/
/* This is columbus2 hardware */
#define CONFIG_ARCH_CEDRIC	1
#define CONFIG_ARMCORTEXA9		/* This is an ARM V7 CPU core */
#define CONFIG_SYS_L2CACHE_OFF		/* No L2 cache */


#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_DELAY_STR "\x0d" /* press ENTER to interrupt BOOT */

/* define baud rate */
#define CONFIG_BAUDRATE	   115200
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*------------------------------------------------------------------------------
    Macro
-------------------------------------------------------------------------------*/

#define CONFIG_ARCH_CPU_INIT

/* boot delay time */
#define CONFIG_BOOTDELAY	0
#define CONFIG_ZERO_BOOTDELAY_CHECK

/*
#define CONFIG_MS_ANDROID_RECOVERY  1
#define CONFIG_MS_DISPLAY   1
#define CONFIG_MS_SHOW_LOGO 1
#define CONFIG_MS_ISP       1
*/

#define CONFIG_SYS_HZ		1000
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_DISPLAY_BOARDINFO /*for checkboard*/
/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	    (CONFIG_ENV_SIZE + 8*1024*1024)/* (CONFIG_ENV_SIZE + 512*1024) */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP	            /* undef to save memory     */
#define CONFIG_SYS_PROMPT	"Cedric # "	/* Monitor Command Prompt   */
#define CONFIG_SYS_CBSIZE	512		        /* Console I/O Buffer Size  */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	        (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	        32		    /* max number of command args   */
#define CONFIG_SYS_BARGSIZE	    	CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size    */

/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	    (4*1024*1024)  /* regular stack */

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x40000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x20000000	/* 512 MB */

/* following is a multiple DRAM bank sample*/
/*
 * Physical Memory Map
 */
/* #define CONFIG_NR_DRAM_BANKS	4			// we have 2 banks of DRAM
 * #define PHYS_SDRAM_1			0xa0000000	// SDRAM Bank #1
 * #define PHYS_SDRAM_1_SIZE		0x04000000	// 64 MB
 * #define PHYS_SDRAM_2			0xa4000000	// SDRAM Bank #2
 * #define PHYS_SDRAM_2_SIZE		0x00000000	// 0 MB
 * #define PHYS_SDRAM_3			0xa8000000	// SDRAM Bank #3
 * #define PHYS_SDRAM_3_SIZE		0x00000000	// 0 MB
 * #define PHYS_SDRAM_4			0xac000000	// SDRAM Bank #4
 * #define PHYS_SDRAM_4_SIZE		0x00000000	// 0 MB
 */
#define CONFIG_SYS_MEMTEST_START	0x40000000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x44000000	/* 0 ... 64 MB in DRAM	*/

#define CONFIG_SYS_TEXT_BASE	0x43E00000
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE  - GENERATED_GBL_DATA_SIZE)

#define CONFIG_UBOOT_RAM_SIZE   0x04000000 // let us to use only 64MB for uboot


#define CONFIG_ENV_SIZE         0x00020000	/* Total Size of Environment Sector */
#define CONFIG_ENV_RANGE        0x00060000

/* RAM base address */
#define RAM_START_ADDR          0x40000000

/* RAM size */
#define RAM_SIZE		        PHYS_SDRAM_1_SIZE
/* The address used to save tag list used when kernel is booting */
#define BOOT_PARAMS 	        (RAM_START_ADDR)
#define BOOT_PARAMS_LEN         0x2000

/* CFG load address */
#define CONFIG_SYS_LOAD_ADDR	        (BOOT_PARAMS+BOOT_PARAMS_LEN+0x4000)

#define CONFIG_CMDLINE_TAG       1    /* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG        1

/* kernel starting address */
#define KERNEL_RAM_BASE	        CFG_LOAD_ADDR

/* Which block used to save IPL, u-boot and kernel images */
#define IPL_NAND_BLOCK      0
#define UBOOT_NAND_BLOCK    1
#define KERNEL_NAND_BLOCK   2

/* define console_printf */
#define console_printf printf

#define CONFIG_CMDLINE_EDITING 1

/* boot time analysis*/
#define CONFIG_BOOT_TIME_ANALYSIS			0
#define CONFIG_BOOT_TIME_ANALYSIS_USE_RTC	0

#define CONFIG_SYS_NO_FLASH 			   1

#ifdef CONFIG_MS_ISP_FLASH
#define CONFIG_CMD_SF
#define CONFIG_CMD_MSTAR_SF
#ifdef CONFIG_MS_SAVE_ENV_IN_ISP_FLASH
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE    0x00010000
#undef  CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE         CONFIG_ENV_SECT_SIZE	/* Total Size of Environment Sector */
#endif
/*
#define CONFIG_ENV_SIZE			(32 << 10)
#define CONFIG_ENV_OFFSET		(544 << 10)
#define CONFIG_ENV_SECT_SIZE	(32 << 10)
*/
#endif


/*
 * File system, NAND flash, SD/MMC
 */
#define CONFIG_CMD_FAT



/*
 * FLASH driver setup
 */


#ifdef CONFIG_MS_SDMMC
#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_DOS_PARTITION
#define CONFIG_MS_SDMMC_MAX_READ_BLOCKS 1024
#endif


#ifdef CONFIG_MS_NAND

#ifndef CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET_OOB
#endif
/* #define ENABLE_NAND_RIU_MODE 1 */

#define CONFIG_CMD_UBI
/* #define CONFIG_CMD_UBIFS */
#define CONFIG_UBI_MWRITE

#define MTDIDS_DEFAULT			"nand0=nand0"    /* "nor0=physmap-flash.0,nand0=nand" */
#define MTDPARTS_DEFAULT		"mtdparts=nand0:0xC0000@0x140000(NPT),-(UBI)"  /* must be different from real partition to test NAND partition function */

#define CONFIG_EXTRA_ENV_SETTINGS                              \
       "mtdids=" MTDIDS_DEFAULT "\0"                           \
       "mtdparts=" MTDPARTS_DEFAULT "\0"                       \
       "partition=nand0,0\0"                                   \
       ""

#define CONFIG_CMD_NAND
#define CONFIG_CMD_CIS

#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_NAND_MAX_CHIPS	1
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0 /* not actually used */
#define CONFIG_CMD_BDI		/* bdinfo			*/
#endif

#endif

#ifdef CONFIG_MS_EMMC
#ifndef CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_IS_IN_MMC       1
#define CONFIG_SYS_MMC_ENV_DEV     1
#endif
#define CONFIG_MS_EMMC_DEV_INDEX   1
#define CONFIG_EMMC_PARTITION
#define CONFIG_UNLZO_DST_ADDR  0x44000000
#endif

#if defined(CONFIG_ENV_IS_IN_NAND) || defined(CONFIG_ENV_IS_IN_MMC) || defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#define CONFIG_MSTAR_ENV_OFFSET
#define CONFIG_CMD_SAVEENV	/* saveenv			*/
#else
#define CONFIG_ENV_IS_NOWHERE
#endif

#if defined(CONFIG_MS_NAND) || defined(CONFIG_MS_MTD_ISP_FLASH)
#define CONFIG_RBTREE
#define CONFIG_MTD_DEVICE               /* needed for mtdparts commands */
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#endif

#define CONFIG_LZO
#define CONFIG_MZ

/* CMD */

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_AUTOSCRIPT

/*
 * USB configuration
 */
#ifdef CONFIG_MS_USB

#define     CONFIG_USB
#define     CONFIG_CMD_USB
#define     CONFIG_USB_STORAGE

#ifdef CONFIG_MS_ENABLE_USB_LAN_MODULE
#define ENABLE_USB_LAN_MODULE
#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_BOOTP_GATEWAY
#endif

#endif


#endif	/* __CONFIG_H */


/*following config should be considered to be removed after confirmed*/

/* #define CONFIG_SKIP_RELOCATE_UBOOT dropped after uboot 201012 */
/*
#undef CONFIG_USE_IRQ	// we don't need IRQ/FIQ stuff
*/
/*#define KERNEL_IMAGE_SIZE       0x1000000	// 10M  kernel image size */

/*  move to cedric_defconfig
 *
 * 	#define CONFIG_MS_SDMMC     1
 *	#define CONFIG_MS_EMMC      1
 *  #define CONFIG_MS_NAND      1
 *
 */
