#ifndef __CONFIG_H
#define __CONFIG_H

/*------------------------------------------------------------------------------
    Constant
-------------------------------------------------------------------------------*/
/* This is columbus2 hardware */
#define CONFIG_ARCH_INFINITY2M	1
#define CONFIG_ARMCORTEXA7		/* This is an ARM V7 CPU core */
#define CONFIG_SYS_L2CACHE_OFF		/* No L2 cache */
/*#define CONFIG_SYS_ARCH_TIMER   1*/
#define CONFIG_MS_PIU_TIMER   1
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_DELAY_STR "\x0d" /* press ENTER to interrupt BOOT */

#define CONFIG_DISPLAY_BOARDINFO    1

#define CONFIG_BOARD_LATE_INIT

#if CONFIG_VERSION_FPGA
#define CONFIG_SYS_HZ_CLOCK 12000000
#define CONFIG_UART_CLOCK   12000000
#define CONFIG_BAUDRATE	    38400
#define CONFIG_PIUTIMER_CLOCK 12000000
#else
#define CONFIG_SYS_HZ_CLOCK 400000000
#define CONFIG_UART_CLOCK   172800000
#define CONFIG_BAUDRATE	    115200
#define CONFIG_PIUTIMER_CLOCK 12000000
#endif

#define CONFIG_WDT_CLOCK        CONFIG_PIUTIMER_CLOCK


/* define baud rate */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*------------------------------------------------------------------------------
    Macro
-------------------------------------------------------------------------------*/

/* boot delay time */
#define CONFIG_BOOTDELAY	0
#define CONFIG_ZERO_BOOTDELAY_CHECK

/*
#define CONFIG_MS_ANDROID_RECOVERY  1
#define CONFIG_MS_DISPLAY   1
#define CONFIG_MS_SHOW_LOGO 1
#define CONFIG_MS_ISP       1
#define CONFIG_MS_PIUTIMER 1
*/

#define CONFIG_SKIP_LOWLEVEL_INIT
/*#define CONFIG_DISPLAY_BOARDINFO *//*for checkboard*/
/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	    (CONFIG_ENV_SIZE + 4*1024*1024)/* (CONFIG_ENV_SIZE + 512*1024) */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_LONGHELP                     /* undef to save memory     */
#define CONFIG_SYS_PROMPT       "Wireless-tag# "  /* Monitor Command Prompt   */
#define CONFIG_SYS_CBSIZE       1024             /* Console I/O Buffer Size  */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	        (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	        64		    /* max number of command args   */
#define CONFIG_SYS_BARGSIZE	    CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size    */

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
#define PHYS_SDRAM_1		0x20000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x08000000	/* 128 MB */

/*Enable watchdog*/
/*#define CONFIG_HW_WATCHDOG 1*/
#ifdef CONFIG_HW_WATCHDOG
#define CONFIG_HW_WATCHDOG_TIMEOUT_S	60
#endif



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


#define CONFIG_SYS_MEMTEST_START	0x20000000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x24000000	/* 0 ... 64 MB in DRAM	*/

#define CONFIG_SYS_TEXT_BASE	0x23E00000
#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE  - GENERATED_GBL_DATA_SIZE)

#define CONFIG_UBOOT_RAM_SIZE   0x04000000 // let us to use only 64MB for uboot



/* RAM base address */
#define RAM_START_ADDR          0x20000000

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


#define CONFIG_CMDLINE_EDITING 1
#define CONFIG_AUTO_COMPLETE

/* boot time analysis*/
#define CONFIG_BOOT_TIME_ANALYSIS			0
#define CONFIG_BOOT_TIME_ANALYSIS_USE_RTC	0

#define CONFIG_SYS_NO_FLASH 			   1

#ifdef CONFIG_MS_ISP_FLASH
#define CONFIG_CMD_SF
#ifdef CONFIG_MS_SAVE_ENV_IN_ISP_FLASH
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SECT_SIZE    0x1000
#define CONFIG_ENV_SIZE         0x1000	/* Total Size of Environment Sector */
/* bottom 4KB of available space in Uboot */
/* 0x40000 reserved for UBoot, 0x40000 maximum storage size of uboot */
#define CONFIG_ENV_OFFSET       0x4F000

/* SPI NOR bootcmd */
#define CONFIG_BOOTCOMMAND " bootlogo 0 0 0 0 0; mw 1f001cc0 1; gpio out 71 0; sf probe 0; sf read 0x22000000 0x60000 0x200000; gpio out 71 1; bootm 0x22000000"

#endif

#endif


/*
 * File system, NAND flash, SD/MMC
 */



/*
 * ENV setting
 */
#if 1
#define ENV_SAVE_DEFAULT /* Save default ENV, if ENV is incorrect after it is readed from flash */
#endif


/*
 * FLASH driver setup
 */


#ifdef CONFIG_MS_SDMMC
#define CONFIG_CMD_FAT
#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_DOS_PARTITION
#define CONFIG_MS_SDMMC_MAX_READ_BLOCKS 1024
#endif

#ifdef CONFIG_MS_SPINAND
#if defined(CONFIG_MS_SAVE_ENV_IN_NAND_FLASH)
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET       CONFIG_MSTAR_ENV_NAND_OFFSET
#define CONFIG_MSTAR_ENV_NAND_OFFSET ms_nand_env_offset
/*#define CONFIG_MSTAR_ENV_NAND_OFFSET 0x440000*/
#define CONFIG_ENV_SIZE         0x1000 // Using 4K length for env is enough, this length must be the same as IPL's env when using fastboot. // 0x00020000
#endif

#define CONFIG_CMD_SPINAND_CIS
#define CONFIG_CMD_UBI
/* #define CONFIG_CMD_UBIFS */
#define CONFIG_UBI_MWRITE
#define MTDIDS_DEFAULT			"nand0=nand0"    /* "nor0=physmap-flash.0,nand0=nand" */
/*	must be different from real partition to test NAND partition function */
/* #define MTDPARTS_DEFAULT		"mtdparts=nand0:0xC0000@0x140000(NPT),-(UBI)" */
#define MTDPARTS_DEFAULT    "mtdparts=nand0:384k@1280k(IPL0),384k(IPL1),384k(IPL_CUST0),384k(IPL_CUST1),768k(UBOOT0),768k(UBOOT1),384k(ENV0),0x20000(KEY_CUST),0x60000(LOGO),0x60000(wtinfo),0x3000000(ubi),0x3000000(ubi2),-(opt)"
#define MTDPARTS2_DEFAULT    "mtdparts=nand0:384k@1280k(IPL0),384k(IPL1),384k(IPL_CUST0),384k(IPL_CUST1),768k(UBOOT0),768k(UBOOT1),384k(ENV0),0x20000(KEY_CUST),0x60000(LOGO),0x60000(wtinfo),0x3000000(ubi2),0x3000000(ubi),-(opt)"

#define CONFIG_EXTRA_ENV_SETTINGS                              \
       "mtdids=" MTDIDS_DEFAULT "\0"                           \
       "mtdparts=" MTDPARTS_DEFAULT "\0"                       \
       "mtdparts2=" MTDPARTS2_DEFAULT "\0"                       \
       "partition=nand0,0\0"                                   \
       ""

#define CONFIG_CMD_NAND

#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_NAND_MAX_CHIPS	1
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0 /* not actually used */
#endif /*CONFIG_CMD_NAND*/

/* SPI NAND bootcmd */
#if defined(WT2022) || defined(WT2011)
#define CONFIG_BOOTCOMMAND "setenv dispout CC0702I50R; setenv bootargs ${mtdparts}; gpio out 4 1; gpio out 18 1; bootlogo 0 0 0 0 0; mw 1f001cc0 1; gpio out 71 0; ubi part ubi; ubi read 0x22000000 kernel; gpio out 71 1; bootm 0x22000000; ubi part ubi2; ubi read 0x22000000 kernel; setenv bootargs ${mtdparts2}; bootm 0x22000000"
#elif defined(WT2020)
#define CONFIG_BOOTCOMMAND "setenv dispout FRD720X720BK; setenv bootargs ${mtdparts}; gpio out 4 1; gpio out 18 1; bootlogo 0 0 0 0 0; mw 1f001cc0 1; gpio out 71 0; ubi part ubi; ubi read 0x22000000 kernel; gpio out 71 1; bootm 0x22000000; ubi part ubi2; ubi read 0x22000000 kernel; setenv bootargs ${mtdparts2}; bootm 0x22000000"
#else
#if defined(WT_UBOOT)
#define CONFIG_BOOTCOMMAND "setenv bootargs ${mtdparts}; bootlogo 0 0 0 0 0; mw 1f001cc0 1; ubi part ubi; ubi read 0x22000000 kernel; bootm 0x22000000; ubi part ubi2; ubi read 0x22000000 kernel; setenv bootargs ${mtdparts2}; bootm 0x22000000"
#else
#define CONFIG_BOOTCOMMAND "bootlogo 0 0 0 0 0; mw 1f001cc0 1; gpio out 71 0; nand read.e 0x22000000 KERNEL 0x200000; gpio out 71 1; bootm 0x22000000; nand read.e 0x22000000 RECOVERY 0x500000; bootm 0x22000000; nand read.e 0x22000000 RECOVERY 0x500000; bootm 0x22000000"
#endif
#endif

#endif /*CONFIG_MS_SPINAND*/

#ifdef CONFIG_MS_NAND

/*
#ifndef CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET_OOB
#endif

*/

#if defined(CONFIG_MS_SAVE_ENV_IN_NAND_FLASH)
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET       CONFIG_MSTAR_ENV_NAND_OFFSET
#define CONFIG_MSTAR_ENV_NAND_OFFSET ms_nand_env_offset
#define CONFIG_ENV_SIZE         0x00020000
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
#endif

#endif

#ifdef CONFIG_MS_EMMC
#ifndef CONFIG_MS_SAVE_ENV_IN_ISP_FLASH
#define CONFIG_ENV_IS_IN_MMC       1
#define CONFIG_SYS_MMC_ENV_DEV     1
#define CONFIG_MS_EMMC_DEV_INDEX   1
#define CONFIG_EMMC_PARTITION
#define CONFIG_UNLZO_DST_ADDR  0x24000000
#define CONFIG_ENV_SIZE         0x00020000
#endif
#endif

#if defined(CONFIG_ENV_IS_IN_NAND) || defined(CONFIG_ENV_IS_IN_MMC) || defined(CONFIG_ENV_IS_IN_SPI_FLASH) || defined(CONFIG_MS_SPINAND)

#define CONFIG_CMD_SAVEENV	/* saveenv */
#else
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE         0x00020000	/* Total Size of Environment Sector */
#define CONFIG_ENV_RANGE        0x00060000

#endif

#if defined(CONFIG_MS_NAND) || defined(CONFIG_MS_MTD_ISP_FLASH) || defined(CONFIG_MS_SPINAND)
#define CONFIG_RBTREE
#define CONFIG_MTD_DEVICE               /* needed for mtdparts commands */
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#endif


/* CMD */

#define CONFIG_CMD_CACHE
#define CONFIG_CMD_MEMORY	/* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_RUN		/* run command in env variable	*/
#define CONFIG_AUTOSCRIPT

/*
 * USB configuration
 */
#ifdef CONFIG_MS_USB
#if !defined(CONFIG_USB)
    #define     CONFIG_USB
#endif
#define     CONFIG_CMD_USB
#if !defined(CONFIG_USB_STORAGE)
    #define     CONFIG_USB_STORAGE
#endif

#ifndef CONFIG_CMD_FAT
#define     CONFIG_CMD_FAT
#endif

#ifndef CONFIG_DOS_PARTITION
#define     CONFIG_DOS_PARTITION
#endif

#ifdef CONFIG_MS_ENABLE_USB_LAN_MODULE
#define ENABLE_USB_LAN_MODULE
#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_BOOTP_GATEWAY
#endif

#endif



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

 /* Ethernet configuration */
#ifdef CONFIG_MS_EMAC
#define CONFIG_MINIUBOOT
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_TFTP_PORT
#endif

#define CONFIG_XZ
#define CONFIG_MZ

#define XZ_DICT_ADDR            0x21000000  /*used for XZ decompress*/
#define XZ_DICT_LENGTH          0x01000000

/* I2C configuration */
/*#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_MSTAR
#define I2C_MS_DECLARATIONS2
#define CONFIG_SYS_I2C_MS_SPEED (300000)
#define CONFIG_SYS_I2C_MS_SLAVE (0x7F)
*/
/*
 * padmux: 1 -> PAD_HDMITX_SCL, PAD_HDMITX_SDA
 *         2 -> PAD_TTL1, PAD_TTL2
 *         3 -> PAD_TTL14, PAD_TTL15
 *         4 -> PAD_GPIO6, PAD_GPIO7
 */
/*#define CONFIG_SYS_I2C_0_PADMUX (0x1)*/
/*
 * padmux: 1 -> PAD_GPIO2, PAD_GPIO3
 *         2 -> PAD_HDMITX_SCL, PAD_HDMITX_SDA
 *         3 -> PAD_FUART_CTS, PAD_FUART_RTS
 *         4 -> PAD_TTL22, PAD_TTL23
 *         5 -> PAD_SD_CLK, PAD_SD_CMD
 */
/*#define CONFIG_SYS_I2C_1_PADMUX (0x1)*/

/* EEPROM */
/*#define CONFIG_CMD_EEPROM
#define CONFIG_HARD_I2C
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN (2)
#define CONFIG_SYS_I2C_EEPROM_ADDR (0x54)
*/

/*
#define ENABLE_DOUBLE_SYSTEM_CHECK  1
*/

/* SENSOR */
/*#define CONFIG_MS_SRCFG*/

#endif	/* __CONFIG_H */
