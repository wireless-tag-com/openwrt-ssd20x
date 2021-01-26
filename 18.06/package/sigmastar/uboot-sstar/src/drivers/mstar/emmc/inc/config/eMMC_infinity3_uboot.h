/*
* eMMC_infinity3_uboot.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: joe.su <joe.su@sigmastar.com.tw>
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

#ifndef __eMMC_INFINITY3_UBOOT__
#define __eMMC_INFINITY3_UBOOT__

#include <common.h>
#include <malloc.h>
#include <linux/string.h>

#include <config.h>
#include <command.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <asm/errno.h>

#ifndef U32
#define U32  unsigned int
#endif
#ifndef U16
#define U16  unsigned short
#endif
#ifndef U8
#define U8   unsigned char
#endif
#ifndef S32
#define S32  signed long
#endif
#ifndef S16
#define S16  signed short
#endif
#ifndef S8
#define S8   signed char
#endif


//=====================================================
// HW registers
//=====================================================
#define REG_OFFSET_SHIFT_BITS           2

#define REG_FCIE_U16(Reg_Addr)          (*(volatile U16*)(Reg_Addr))
#define GET_REG_ADDR(x, y)              ((x)+((y) << REG_OFFSET_SHIFT_BITS))

#define REG_FCIE(reg_addr)              REG_FCIE_U16(reg_addr)
#define REG_FCIE_W(reg_addr, val)       REG_FCIE(reg_addr) = (val)
#define REG_FCIE_R(reg_addr, val)       val = REG_FCIE(reg_addr)
#define REG_FCIE_SETBIT(reg_addr, val)  REG_FCIE(reg_addr) |= (val)
#define REG_FCIE_CLRBIT(reg_addr, val)  REG_FCIE(reg_addr) &= ~(val)
#define REG_FCIE_W1C(reg_addr, val)     REG_FCIE_W(reg_addr, REG_FCIE(reg_addr)&(val))
 
//------------------------------
#define RIU_BASE                0x1F200000

#define REG_BANK_FCIE0          0x20100U
#define REG_BANK_FCIE1          0x20180U
#define REG_BANK_FCIE2          0x20200U

#define FCIE0_BASE                      GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE0)
#define FCIE1_BASE                      GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE1)
#define FCIE2_BASE                      GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE2)

#define CHIPTOP_BASE            GET_REG_ADDR(RIU_BASE, REG_BANK_CHIPTOP)
#define FCIE_REG_BASE_ADDR      GET_REG_ADDR(FCIE0_BASE, 0x00)
#define FCIE_CMDFIFO_BASE_ADDR	GET_REG_ADDR(FCIE0_BASE, 0x20)
#define FCIE_CIFD_BASE_ADDR     GET_REG_ADDR(FCIE1_BASE, 0x00)

#define FCIE_NC_WBUF_CIFD_BASE	GET_REG_ADDR(FCIE1_BASE, 0x00)
#define FCIE_NC_RBUF_CIFD_BASE	GET_REG_ADDR(FCIE1_BASE, 0x20)

#define FCIE_POWEER_SAVE_MODE_BASE      GET_REG_ADDR(FCIE2_BASE, 0x00)

#include "eMMC_reg_v5.h"

#define BIT_MIE_FUNC_ENABLE             BIT_SD_EN //I6E:BIT_SDIO_MOD OTHERS:BIT_SD_EN

//--------------------------------clock gen------------------------------------
#define REG_BANK_CLKGEN0                0x1C00U	// (0x100B - 0x1000) x 80h
#define CLKGEN0_BASE                    GET_REG_ADDR(RIU_BASE, REG_BANK_CLKGEN0)

#define reg_ckg_fcie                    GET_REG_ADDR(CLKGEN0_BASE, 0x43)
#define BIT_FCIE_CLK_GATING             BIT0
#define BIT_FCIE_CLK_INVERSE            BIT1
#define BIT_CLKGEN_FCIE_MASK            (BIT5|BIT4|BIT3|BIT2)
#define BIT_FCIE_CLK_SRC_SEL            BIT6 // 0: clk_xtal 12M, 1: clk_nfie_p1

#define REG_BANK_SC_GP_CTRL				(0x1133 - 0x1000) * 0x80
#define SC_GP_CTRL_BASE                 GET_REG_ADDR(RIU_BASE, REG_BANK_SC_GP_CTRL)
#define reg_sc_gp_ctrl                  GET_REG_ADDR(SC_GP_CTRL_BASE, 0x25) //TBD:rename reg_sc_gp_ctrl to reg_ckg_sd

//--------------------------------chiptop--------------------------------------
#define REG_BANK_CHIPTOP                    0x0F00	// (0x101E - 0x1000) x 80h
#define PAD_CHIPTOP_BASE                    GET_REG_ADDR(RIU_BASE, REG_BANK_CHIPTOP)

#define reg_emmc_rstz_en                    GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x13)
#define BIT_EMMC_RST                        (BIT1)

#define reg_all_pad_in                      GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x50)
#define BIT_ALL_PAD_IN                      (BIT15)

#define reg_sd_config                       GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x08)
#define BIT_SD_MODE_MASK                    (BIT3|BIT2)

#define reg_emmc_config                     GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x13)
#define BIT_EMMC_MODE_MASK                  (BIT0)
#define BIT_EMMC_MODE_1                     BIT0

#define reg_nand_config                     GET_REG_ADDR(PAD_CHIPTOP_BASE, 0x08)
#define BIT_NAND_MODE_MASK                  (BIT0)

//--------------------------------clock gen------------------------------------
#define BIT_CLK_XTAL_12M                0x3
#define BIT_FCIE_CLK_20M                0xA
#define BIT_FCIE_CLK_32M                0x9
#define BIT_FCIE_CLK_36M                0x8
#define BIT_FCIE_CLK_40M                0x7
#define BIT_FCIE_CLK_43_2M              0x6
#define BIT_FCIE_CLK_54M                0x2
#define BIT_FCIE_CLK_62M                0x1
#define BIT_FCIE_CLK_72M                0x0
#define BIT_FCIE_CLK_86M                0x4
//										0xA
#define BIT_FCIE_CLK_300K               0xC
#define BIT_FCIE_CLK_48M                0x5
#define BIT_FCIE_CLK_5_4M               0xB

#define eMMC_FCIE_VALID_CLK_CNT         3 // FIXME

#define PLL_SKEW4_CNT                   9
#define MIN_OK_SKEW_CNT                 5

struct _eMMC_FCIE_ATOP_SET {

    U32 u32_ScanResult;
	U8  u8_Clk;
	U8  u8_Reg2Ch, u8_Skew4;
    U8  u8_Cell;
};
typedef struct _eMMC_FCIE_ATOP_SET eMMC_FCIE_ATOP_SET_t;


#define eMMC_RST_L()                    {REG_FCIE_SETBIT(reg_emmc_rstz_en, BIT_EMMC_RST);\
                                         REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_EMMC_RSTZ_EN);\
                                         REG_FCIE_CLRBIT(FCIE_BOOT_CONFIG, BIT_EMMC_RSTZ);}
#define eMMC_RST_H()                    {REG_FCIE_SETBIT(reg_emmc_rstz_en, BIT_EMMC_RST);\
                                         REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_EMMC_RSTZ_EN);\
                                         REG_FCIE_SETBIT(FCIE_BOOT_CONFIG, BIT_EMMC_RSTZ);}


#define REG_BANK_TIMER1                 0x1800
#define TIMER1_BASE                     GET_REG_ADDR(RIU_PM_BASE, REG_BANK_TIMER1)

#define TIMER1_ENABLE                   GET_REG_ADDR(TIMER1_BASE, 0x20)
#define TIMER1_HIT                      GET_REG_ADDR(TIMER1_BASE, 0x21)
#define TIMER1_MAX_LOW                  GET_REG_ADDR(TIMER1_BASE, 0x22)
#define TIMER1_MAX_HIGH                 GET_REG_ADDR(TIMER1_BASE, 0x23)
#define TIMER1_CAP_LOW                  GET_REG_ADDR(TIMER1_BASE, 0x24)
#define TIMER1_CAP_HIGH                 GET_REG_ADDR(TIMER1_BASE, 0x25)

#if 0
//--------------------------------sar5----------------------------
#define reg_sel_hvdetect                GET_REG_ADDR(RIU_PM_BASE+0x1C00, 0x28)
#define BIT_SEL_HVDETECT                BIT4
#define reg_vplug_in_pwrgd              GET_REG_ADDR(RIU_PM_BASE+0x1C00, 0x62)
#endif

//--------------------------------power saving mode----------------------------
#define REG_BANK_PM_SLEEP               (0x700)
#define PM_SLEEP_REG_BASE_ADDR          GET_REG_ADDR(RIU_PM_BASE, REG_BANK_PM_SLEEP)
#define reg_pwrgd_int_glirm             GET_REG_ADDR(PM_SLEEP_REG_BASE_ADDR, 0x61)
#define BIT_PWRGD_INT_GLIRM_EN          BIT9
#define BIT_PWEGD_INT_GLIRM_MASK        (BIT15|BIT14|BIT13|BIT12|BIT11|BIT10)


//=====================================================
// API declarations
//=====================================================

extern  U32 eMMC_hw_timer_delay(U32 u32us);
extern  U32 eMMC_hw_timer_sleep(U32 u32ms);
#define eMMC_HW_TIMER_MHZ                12000000//12MHz  [FIXME]
// define what latch method (mode) fcie has
// implement switch pad function with below cases

#define FCIE_MODE_GPIO_PAD_DEFO_SPEED       0
#define FCIE_eMMC_BYPASS                    FCIE_MODE_GPIO_PAD_DEFO_SPEED

#define FCIE_MODE_8BITS_MACRO_HIGH_SPEED    2
#define FCIE_eMMC_SDR                       FCIE_MODE_8BITS_MACRO_HIGH_SPEED

#define FCIE_MODE_8BITS_MACRO_DDR52         3
#define FCIE_MODE_32BITS_MACRO_DDR52        4
#define FCIE_eMMC_DDR                       FCIE_MODE_8BITS_MACRO_DDR52
#define FCIE_eMMC_DDR_8BIT_MACRO            FCIE_MODE_8BITS_MACRO_DDR52

#define FCIE_MODE_32BITS_MACRO_HS200        5
#define FCIE_eMMC_HS200                     FCIE_MODE_32BITS_MACRO_HS200

#define FCIE_MODE_32BITS_MACRO_HS400_DS     6 // data strobe
#define FCIE_MODE_32BITS_MACRO_HS400_SKEW4  7
#define FCIE_eMMC_5_1_AFIFO                 8
#define FCIE_eMMC_HS400                     FCIE_MODE_32BITS_MACRO_HS400_DS
#define FCIE_eMMC_HS400_DS                  FCIE_eMMC_HS400
#define FCIE_eMMC_HS400_SKEW4               FCIE_MODE_32BITS_MACRO_HS400_SKEW4


// define what latch method (mode) use for latch eMMC data
// switch FCIE mode when driver (kernel) change eMMC speed

#define EMMC_DEFO_SPEED_MODE            FCIE_MODE_GPIO_PAD_DEFO_SPEED
#define EMMC_HIGH_SPEED_MODE            FCIE_MODE_GPIO_PAD_DEFO_SPEED
#define EMMC_DDR52_MODE                 FCIE_MODE_32BITS_MACRO_DDR52 // FCIE_MODE_8BITS_MACRO_DDR52
#define EMMC_HS200_MODE                 FCIE_MODE_32BITS_MACRO_HS200
#define EMMC_HS400_MODE                 FCIE_MODE_32BITS_MACRO_HS400_DS // FCIE_MODE_32BITS_MACRO_HS400_SKEW4
#define ENABLE_AFIFO                    1


// define what speed we want this chip/project run
//------------------------------
// DDR48, DDR52, HS200, HS400
#define IF_DETECT_eMMC_DDR_TIMING       0 // DDR48 (digital macro)
#define ENABLE_eMMC_ATOP                0
#define ENABLE_eMMC_DDR52               0
#define ENABLE_eMMC_HS200               0
#define ENABLE_eMMC_HS400               0
#define eMMC_IF_TUNING_TTABLE()         (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_TUNING_TTABLE)
// mboot use this config

#define WRITE_TO_eMMC	0
#define READ_FROM_eMMC 1

extern  U32 eMMC_pads_switch(U32 u32_FCIE_IF_Type);
extern  U32 eMMC_clock_setting(U16 u16_ClkParam);
extern  U32 eMMC_config_clock(U16 u16_SeqAccessTime);
extern void eMMC_set_WatchDog(U8 u8_IfEnable);
extern void eMMC_reset_WatchDog(void);
extern  U32 eMMC_translate_DMA_address_Ex(U32 u32_DMAAddr, U32 u32_ByteCnt, int mode);
extern void eMMC_Invalidate_data_cache_buffer(U32 u32_addr, S32 s32_size);
extern void eMMC_flush_miu_pipe(void);
extern  U32 eMMC_PlatformResetPre(void);
extern  U32 eMMC_PlatformResetPost(void);
extern  U32 eMMC_PlatformInit(void);
extern  U32 eMMC_CheckIfMemCorrupt(void);
extern void eMMC_DumpPadClk(void);
#define eMMC_BOOT_PART_W   BIT0
#define eMMC_BOOT_PART_R   BIT1
extern  U32 eMMC_BootPartitionHandler_WR(U8 *pDataBuf, U16 u16_PartType, U32 u32_StartSector, U32 u32_SectorCnt, U8 u8_OP);
extern  U32 eMMC_BootPartitionHandler_E(U16 u16_PartType);
extern  U32 eMMC_hw_timer_start(void);
extern  U32 eMMC_hw_timer_tick(void);

//=====================================================
// partitions config
//=====================================================
// every blk is 512 bytes (reserve 2MB for internal use)
// reserve 0x1200 x 0x200, more than 2MB
#define eMMC_DRV_RESERVED_BLK_CNT       (0x200000/0x200)

#define eMMC_CIS_NNI_BLK_CNT            2
#define eMMC_CIS_PNI_BLK_CNT            2
#define eMMC_TEST_BLK_CNT               (0x100000/0x200)

#define eMMC_CIS_BLK_0                  (64*1024/512) // from 64KB
#define eMMC_NNI_BLK_0                  (eMMC_CIS_BLK_0+0)
#define eMMC_NNI_BLK_1                  (eMMC_CIS_BLK_0+1)
#define eMMC_PNI_BLK_0                  (eMMC_CIS_BLK_0+2)
#define eMMC_PNI_BLK_1                  (eMMC_CIS_BLK_0+3)
#define eMMC_DDRTABLE_BLK_0             (eMMC_CIS_BLK_0+4)
#define eMMC_DDRTABLE_BLK_1             (eMMC_CIS_BLK_0+5)
#define eMMC_HS200TABLE_BLK_0           (eMMC_CIS_BLK_0+6)
#define eMMC_HS200TABLE_BLK_1           (eMMC_CIS_BLK_0+7)
#define eMMC_HS400TABLE_BLK_0           (eMMC_CIS_BLK_0+8)
#define eMMC_HS400TABLE_BLK_1           (eMMC_CIS_BLK_0+9)
#define eMMC_DrvContext_BLK_0           (eMMC_CIS_BLK_0+10)
#define eMMC_DrvContext_BLK_1           (eMMC_CIS_BLK_0+11)
#define eMMC_ALLRSP_BLK_0               (eMMC_CIS_BLK_0+12)
#define eMMC_ALLRSP_BLK_1               (eMMC_CIS_BLK_0+13)
#define eMMC_BURST_LEN_BLK_0            (eMMC_CIS_BLK_0+14)
#define eMMC_LIFE_TEST_BYTE_CNT_BLK     (eMMC_CIS_BLK_0+15)
#define eMMC_CIS_BLK_END                eMMC_LIFE_TEST_BYTE_CNT_BLK
// last 1MB in reserved area, use for eMMC test
#define eMMC_TEST_BLK_0                 (eMMC_CIS_BLK_END+1)
#define eMMC_TOTAL_RESERVED_BLK_CNT     (eMMC_DRV_RESERVED_BLK_CNT+eMMC_CIS_BLK_0)


#define eMMC_LOGI_PART		    0x8000 // bit-or if the partition needs Wear-Leveling
#define eMMC_HIDDEN_PART	    0x4000 // bit-or if this partition is hidden, normally it is set for the LOGI PARTs.

#define eMMC_PART_HWCONFIG		(1|eMMC_LOGI_PART)
#define eMMC_PART_BOOTLOGO		(2|eMMC_LOGI_PART)
#define eMMC_PART_BL			(3|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_OS			(4|eMMC_LOGI_PART)
#define eMMC_PART_CUS			(5|eMMC_LOGI_PART)
#define eMMC_PART_UBOOT			(6|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_SECINFO		(7|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_OTP			(8|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_RECOVERY		(9|eMMC_LOGI_PART)
#define eMMC_PART_E2PBAK		(10|eMMC_LOGI_PART)
#define eMMC_PART_NVRAMBAK		(11|eMMC_LOGI_PART)
#define eMMC_PART_APANIC		(12|eMMC_LOGI_PART)
#define eMMC_PART_MISC			(14|eMMC_LOGI_PART)


#define eMMC_PART_FDD			(17|eMMC_LOGI_PART)
#define eMMC_PART_TDD			(18|eMMC_LOGI_PART)

#define eMMC_PART_E2P0          (19|eMMC_LOGI_PART)
#define eMMC_PART_E2P1          (20|eMMC_LOGI_PART)
#define eMMC_PART_NVRAM0		(21|eMMC_LOGI_PART)
#define eMMC_PART_NVRAM1		(22|eMMC_LOGI_PART)
#define eMMC_PART_SYSTEM		(23|eMMC_LOGI_PART)
#define eMMC_PART_CACHE			(24|eMMC_LOGI_PART)
#define eMMC_PART_DATA			(25|eMMC_LOGI_PART)
#define eMMC_PART_FAT 			(26|eMMC_LOGI_PART)

#define eMMC_PART_ENV			(0x0D|eMMC_LOGI_PART|eMMC_HIDDEN_PART) // uboot env
#define eMMC_PART_DEV_NODE		(0x0F|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_MIU	(0x20|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_EBOOT (0x21|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_RTK (0x22|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_PARAMS (0x23|eMMC_LOGI_PART|eMMC_HIDDEN_PART)
#define eMMC_PART_NVRAM 	(0x88|eMMC_LOGI_PART) //default start from 31 count

//extern char *gpas8_eMMCPartName[];

//=====================================================
// Driver configs
//=====================================================
#define eMMC_UPDATE_FIRMWARE            0

#define eMMC_ST_PLAT                    0x80000000
// [CAUTION]: to verify IP and HAL code, defaut 0
#define IF_IP_VERIFY						1 // [FIXME] -->

// need to eMMC_pads_switch
// need to eMMC_clock_setting
#define IF_FCIE_SHARE_IP                    1
//------------------------------
#define ENABLE_eMMC_INTERRUPT_MODE			0
#define ENABLE_eMMC_RIU_MODE				0 // for debug cache issue

#if ENABLE_eMMC_RIU_MODE
#undef IF_DETECT_eMMC_DDR_TIMING
#define IF_DETECT_eMMC_DDR_TIMING			0 // RIU mode can NOT use DDR
#endif
// <-- [FIXME]

//------------------------------
#define eMMC_FEATURE_RELIABLE_WRITE         0
#if eMMC_UPDATE_FIRMWARE
#undef  eMMC_FEATURE_RELIABLE_WRITE
#define eMMC_FEATURE_RELIABLE_WRITE         0
#endif

//------------------------------
#define eMMC_RSP_FROM_RAM                   0
#define eMMC_RSP_FROM_RAM_LOAD              0

#define eMMC_FCIE_LINUX_DRIVER	1

//------------------------------
//-------------------------------------------------------
// Devices has to be in 512B block length mode by default 
// after power-on, or software reset.
//-------------------------------------------------------

#define eMMC_SECTOR_512BYTEa       0x200
#define eMMC_SECTOR_512BYTE_BITSa  9
#define eMMC_SECTOR_512BYTE_MASKa  (eMMC_SECTOR_512BYTEa-1)

#define eMMC_SECTOR_BUF_16KBa      (eMMC_SECTOR_512BYTEa * 0x20)

#define eMMC_SECTOR_BYTECNTa       eMMC_SECTOR_512BYTEa
#define eMMC_SECTOR_BYTECNT_BITSa  eMMC_SECTOR_512BYTE_BITSa

#define eMMC_SECTOR_BUF_BYTECTN   eMMC_SECTOR_BUF_16KBa
extern U8 gau8_eMMC_SectorBuf[]; // 512 bytes
extern U8 gau8_eMMC_PartInfoBuf[]; // 512 bytes
//------------------------------
#define FCIE_REGRESSION_TEST               0
#define FCIE_GPIO_PAD_VERIFY               1
#define FCIE_SDR_FLASH_MACRO8_VERIFY       0
#define FCIE_DDR_FLASH_MACRO8_VERIFY       0
#define FCIE_HS200_FLASH_MACRO32_VERIFY    0
#define FCIE_HS400_FLASH_MACRO32_VERIFY    0
#define FCIE_eMMC5_1_FLASH_MACRO32_VERIFY    0
#define ENABLE_IMI_TEST                 0

#define BOOT_SRAM_DMA_TEST              0
#define WRITE_TEST_BOOT_CODE            0
#define BOOT_MODE_TEST                  0
#define FCIE_BIST_TEST                  0
#define FCIE_WRITE_TIMEOUT_INT_TEST     0
#define FCIE_READ_TIMEOUT_INT_TEST      0
#define FCIE_READ_CRC_ERROR_INT_TEST    0
#define FCIE_WRITE_CRC_ERROR_INT_TEST   0
#define FCIE_CMD_NO_RESPONSE_INT_TEST   0
#define FCIE_CMD_RSP_CRC_ERROR_INT_TEST 0
#define FCIE_RSP_SHIFT_TUNING_TEST      0 
#define FCIE_WCRC_SHIFT_TUNING_TEST     0 
#define FCIE_RSTOP_SHIFT_TUNING_TEST    0
#define FCIE_POWER_SAVING_TEST          0

//------------------------------
// Boot Partition:
//   [FIXME]: if platform has ROM code like G2P
//------------------------------
//	No Need in A3
#define BL_BLK_OFFSET          0
#define BL_BLK_CNT            (0xF200/0x200)
#define OTP_BLK_OFFSET         BL_BLK_CNT
#define OTP_BLK_CNT           (0x8000/0x200)
#define SecInfo_BLK_OFFSET    (BL_BLK_CNT+OTP_BLK_CNT)
#define SecInfo_BLK_CNT       (0x1000/0x200)
#define BOOT_PART_TOTAL_CNT   (BL_BLK_CNT+OTP_BLK_CNT+SecInfo_BLK_CNT)
// <-- [FIXME]

#define eMMC_CACHE_LINE		   0x80 // [FIXME]

//=====================================================
// tool-chain attributes
//===================================================== [FIXME] -->
#define eMMC_PACK0
#define eMMC_PACK1					__attribute__((__packed__))
#define eMMC_ALIGN0
#define eMMC_ALIGN1					__attribute__((aligned(eMMC_CACHE_LINE)))
// <-- [FIXME]

//=====================================================
// debug option
//=====================================================
#define eMMC_TEST_IN_DESIGN					0 // [FIXME]: set 1 to verify HW timer

#ifndef eMMC_DEBUG_MSG
#define eMMC_DEBUG_MSG						1
#endif

/* Define trace levels. */
#define eMMC_DEBUG_LEVEL_ERROR				(1)    /* Error condition debug messages. */
#define eMMC_DEBUG_LEVEL_WARNING			(2)    /* Warning condition debug messages. */
#define eMMC_DEBUG_LEVEL_HIGH				(3)    /* Debug messages (high debugging). */
#define eMMC_DEBUG_LEVEL_MEDIUM				(4)    /* Debug messages. */
#define eMMC_DEBUG_LEVEL_LOW				(5)    /* Debug messages (low debugging). */

/* Higer debug level means more verbose */
#ifndef eMMC_DEBUG_LEVEL
#define eMMC_DEBUG_LEVEL					eMMC_DEBUG_LEVEL_HIGH//
#endif

#if defined(eMMC_DEBUG_MSG) && eMMC_DEBUG_MSG
#define eMMC_printf    printf                       // <-- [FIXME]
#define eMMC_debug(dbg_lv, tag, str, ...)	         \
	do {	                                         \
		if (dbg_lv > eMMC_DEBUG_LEVEL)				 \
			break;									 \
		else {										 \
			if (tag)								 \
				eMMC_printf("[ %s() ] ", __func__);\
													 \
			eMMC_printf(str, ##__VA_ARGS__);		 \
		} \
	} while(0)
#else /* eMMC_DEBUG_MSG */
#define eMMC_printf(...)
#define eMMC_debug(enable, tag, str, ...)	do{}while(0)
#endif /* eMMC_DEBUG_MSG */

#define eMMC_die(msg) while(1);//SYS_FAIL(""msg);

#define eMMC_stop() \
	while(1)  eMMC_reset_WatchDog();

//=====================================================
// unit for HW Timer delay (unit of us)
//=====================================================
#define HW_TIMER_DELAY_1us	    1
#define HW_TIMER_DELAY_5us	    5
#define HW_TIMER_DELAY_10us     10
#define HW_TIMER_DELAY_100us	100
#define HW_TIMER_DELAY_1ms	    (1000 * HW_TIMER_DELAY_1us)
#define HW_TIMER_DELAY_5ms	    (5    * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_10ms	    (10   * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_100ms	(100  * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_500ms	(500  * HW_TIMER_DELAY_1ms)
#define HW_TIMER_DELAY_1s	    (1000 * HW_TIMER_DELAY_1ms)

//=====================================================
// set FCIE clock
//=====================================================
#define FCIE_SLOWEST_CLK                BIT_FCIE_CLK_300K
#define FCIE_DEFO_SPEED_CLK             BIT_CLK_XTAL_12M
#define FCIE_HIGH_SPEED_CLK             BIT_FCIE_CLK_48M

// for backward compatible
#define FCIE_SLOW_CLK                   FCIE_DEFO_SPEED_CLK
#define FCIE_DEFAULT_CLK                FCIE_HIGH_SPEED_CLK
#define eMMC_PLL_CLK_SLOW               FCIE_DEFO_SPEED_CLK

//=====================================================
// transfer DMA Address
//=====================================================
#define MIU_BUS_WIDTH_BITS					3 // 8 bytes width [FIXME]
/*
 * Important:
 * The following buffers should be large enough for a whole eMMC block
 */
// FIXME, this is only for verifing IP
#define DMA_W_ADDR                      0x21600000
#define DMA_R_ADDR                      0x21700000
#define DMA_W_DDR_ADDR                  0x21800000
#define DMA_R_DDR_ADDR                  0x21900000

#define DMA_W_IMI_ADDR                          0xA0000000
#define DMA_R_IMI_ADDR                          0xA0001000
#define DMA_W_IMI_SPARE_ADDR                    0xA0002000
#define DMA_R_IMI_SPARE_ADDR                    0xA0003000

extern void *eMMC_memcpy (void *destaddr, void const *srcaddr, unsigned int len);

//=====================================================
// misc
//=====================================================
//#define BIG_ENDIAN
#define LITTLE_ENDIAN

#if (defined(BIT_DQS_MODE_MASK) && (BIT_DQS_MODE_MASK != (BIT12|BIT13|BIT14)))

#undef BIT_DQS_MODE_MASK
#undef BIT_DQS_MODE_2T
#undef BIT_DQS_MODE_1_5T
#undef BIT_DQS_MODE_2_5T
#undef BIT_DQS_MODE_1T

#define BIT_DQS_MODE_MASK               (BIT12|BIT13|BIT14)
#define BIT_DQS_MODE_0T                 (0 << BIT_DQS_MDOE_SHIFT)
#define BIT_DQS_MODE_0_5T               (1 << BIT_DQS_MDOE_SHIFT)
#define BIT_DQS_MODE_1T                 (2 << BIT_DQS_MDOE_SHIFT)
#define BIT_DQS_MODE_1_5T               (3 << BIT_DQS_MDOE_SHIFT)
#define BIT_DQS_MODE_2T                 (4 << BIT_DQS_MDOE_SHIFT)
#define BIT_DQS_MODE_2_5T               (5 << BIT_DQS_MDOE_SHIFT)
#define BIT_DQS_MODE_3T                 (6 << BIT_DQS_MDOE_SHIFT)
#define BIT_DQS_MODE_3_5T               (7 << BIT_DQS_MDOE_SHIFT)

#endif

#endif /* __eMMC_INFINITY3_UBOOT__ */
