/*
* eMMC_chicago_uboot.h- Sigmastar
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

#ifndef __eMMC_CHICAGO_UBOOT__
#define __eMMC_CHICAGO_UBOOT__

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
#define RIU_BASE                0x1F000000

#define REG_BANK_PADTOP			0x580
#define REG_BANK_CHIPTOP		0xF00
#define REG_BANK_FCIE0          0x1000
#define REG_BANK_FCIE1          0x1080
#define REG_BANK_FCIE2          0x1100
#define REG_BANK_FCIE3          0x1180

#define PADTOP_BASE				GET_REG_ADDR(RIU_BASE, REG_BANK_PADTOP)
#define CHIPTOP_BASE            GET_REG_ADDR(RIU_BASE, REG_BANK_CHIPTOP)
#define FCIE_REG_BASE_ADDR      GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE0) 
#define FCIE_CIFC_BASE_ADDR     GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE1)
#define FCIE_CIFD_BASE_ADDR     GET_REG_ADDR(RIU_BASE, REG_BANK_FCIE2)

#define REG_PADTOP_00 		GET_REG_ADDR(PADTOP_BASE, 0x00)
#define REG_PADTOP_01		GET_REG_ADDR(PADTOP_BASE, 0x01)
#define REG_PADTOP_40		GET_REG_ADDR(PADTOP_BASE, 0x40)
#define REG_PADTOP_43		GET_REG_ADDR(PADTOP_BASE, 0x43)

#include "eMMC_reg.h"

//------------------------------
/*
#define BIT_FCIE_CLK_Gate               BIT0 
#define BIT_FCIE_CLK_Inverse            BIT1
#define BIT_FCIE_CLK_MASK               (BIT2|BIT3|BIT4|BIT5)
#define BIT_FCIE_CLK_SHIFT              2
#define BIT_FCIE_CLK_SEL                BIT6 // 1: NFIE, 0: 12MHz
#define BIT_FCIE_CLK4X_Gate             BIT8
#define BIT_FCIE_CLK4X_Inverse          BIT9
#define BIT_FCIE_CLK4X_MASK             (BIT10|BIT11|BIT12)
#define BIT_FCIE_CLK4X_SHIFT            10

#define reg_ckg_MCU                     GET_REG_ADDR(CLKGEN_BASE, 0x10)
#define reg_ckg_MIU                     GET_REG_ADDR(CLKGEN_BASE, 0x1D)
*/
#define NFIE_REG_CLK_MASK          ((BIT6-1))
#define NFIE_REG_4XCLK_1_3M        ((0<<2))
#define NFIE_REG_4XCLK_26M         ((1<<2))
#define NFIE_REG_4XCLK_48M         ((2<<2))
#define NFIE_REG_4XCLK_80M         ((3<<2))
#define NFIE_REG_4XCLK_96M         ((4<<2))
#define NFIE_REG_4XCLK_120M        ((5<<2))
#define NFIE_REG_4XCLK_147M        ((6<<2))
#define NFIE_REG_4XCLK_160M        ((7<<2))
#define NFIE_REG_4XCLK_176_8M      ((8<<2))
#define NFIE_REG_4XCLK_192M        ((9<<2))
#define NFIE_REG_4XCLK_221M        ((10<<2))
#define NFIE_REG_4XCLK_240M        ((11<<2))
#define NFIE_REG_4XCLK_294_6M      ((13<<2))
#define NFIE_REG_4XCLK_353_6M      ((14<<2))
#define NFIE_REG_4XCLK_60M         ((15<<2))

#define eMMC_FCIE_VALID_CLK_CNT         5
extern  U16 gau16_FCIEClkSel[];

//--------------------------sd/eMMC/nand Mode

#define REG_CLK_EMMC            GET_REG_ADDR(CHIPTOP_BASE, 0x25)
#define REG_CLK_4X_DIV_EN		GET_REG_ADDR(CHIPTOP_BASE, 0x3F)
#define REG_RESET_PIN			GET_REG_ADDR(CHIPTOP_BASE, 0x31)

#define RIU_BASE_CLKGEN0        (RIU_BASE+(0x0580U<<REG_OFFSET_SHIFT_BITS))
#define RIU_BASE_CHIPGPIO1		(RIU_BASE+(0x0D00U<<REG_OFFSET_SHIFT_BITS))
#define RIU_BASE_CHIPTOP        (RIU_BASE+(0x0F00U<<REG_OFFSET_SHIFT_BITS))

#define eMMC_DBUS_WIDTH         8

#define eMMC_RST_L()	REG_FCIE_CLRBIT(REG_RESET_PIN, BIT0)
#define eMMC_RST_H()	REG_FCIE_SETBIT(REG_RESET_PIN, BIT0)

//=====================================================
// API declarations
//=====================================================

extern  U32 eMMC_hw_timer_delay(U32 u32us);

//#define eMMC_HW_TIMER_MHZ               (12*0x100000)//(384*100*1000)  // [FIXME]
#define FCIE_eMMC_DISABLE               0
#define FCIE_eMMC_DDR                   1
#define FCIE_eMMC_SDR                   2
#define FCIE_eMMC_BYPASS                3
#define FCIE_eMMC_TMUX                  4

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

// last 1MB in reserved area, use for eMMC test
#define eMMC_TEST_BLK_0                 (eMMC_DRV_RESERVED_BLK_CNT-eMMC_TEST_BLK_CNT)


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
// [CAUTION]: to detect DDR timiing parameters, only for DL
#define IF_DETECT_eMMC_DDR_TIMING           0
#define eMMC_IF_DDRT_TUNING()               (g_eMMCDrv.u32_DrvFlag&DRV_FLAG_DDR_TUNING)

// need to eMMC_pads_switch
// need to eMMC_clock_setting
#define IF_FCIE_SHARE_IP                    0
//------------------------------
#define FICE_BYTE_MODE_ENABLE               1 // always 1
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
extern U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KBa]; // 512 bytes
extern U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTEa]; // 512 bytes
//U8 gau8_eMMC_SectorBuf[eMMC_SECTOR_BUF_16KBa]; // 512 bytes
//U8 gau8_eMMC_PartInfoBuf[eMMC_SECTOR_512BYTEa]; // 512 bytes
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

#define eMMC_CACHE_LINE		   0x20 // [FIXME]

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
		else if(eMMC_IF_DDRT_TUNING())               \
			break;                                   \
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
#define FCIE_SLOWEST_CLK                NFIE_REG_4XCLK_1_3M
#define FCIE_SLOW_CLK                   NFIE_REG_4XCLK_80M
#define FCIE_DEFAULT_CLK                NFIE_REG_4XCLK_192M

//=====================================================
// transfer DMA Address
//=====================================================
#define MIU_BUS_WIDTH_BITS					3 // 8 bytes width [FIXME]
/*
 * Important:
 * The following buffers should be large enough for a whole eMMC block
 */
// FIXME, this is only for verifing IP
#define DMA_W_ADDR                      0x40C00000
#define DMA_R_ADDR                      0x40D00000
#define DMA_W_SPARE_ADDR                0x40E00000
#define DMA_R_SPARE_ADDR                0x40E80000
#define DMA_BAD_BLK_BUF                 0x40F00000


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


#endif /* __eMMC_CHICAGO_UBOOT__ */
