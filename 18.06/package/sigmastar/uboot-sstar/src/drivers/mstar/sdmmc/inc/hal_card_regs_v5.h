/*
* hal_card_regs_v5.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: truman.yang <truman.yang@sigmastar.com.tw>
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

/***************************************************************************************************************
 *
 * FileName hal_card_regs_v5.h
 *     @author jeremy.wang (2015/06/03)
 * Desc:
 * 	   This file is the header file for hal_card_regs.h
 *
 * 	   We add a new header file to describe the meaning positions of fcie5 registers
 *
 ***************************************************************************************************************/

#ifndef __HAL_CARD_REGS_V5_H
#define __HAL_CARD_REGS_V5_H

//============================================
//MIE_EVENT: offset 0x00
//============================================
#define R_DATA_END              BIT00_T
#define R_CMD_END               BIT01_T
#define R_ERR_STS               BIT02_T
#define R_SDIO_INT              BIT03_T
#define R_BUSY_END_INT          BIT04_T
#define R_R2N_RDY_INT           BIT05_T
#define R_CARD_CHANGE           BIT06_T
#define R_CARD2_CHANGE          BIT07_T


//============================================
//MIE_INT_EN: offset 0x01
//============================================
#define R_DATA_END_IEN          BIT00_T
#define R_CMD_END_IEN           BIT01_T
#define R_ERR_STS_IEN           BIT02_T
#define R_SDIO_INT_IEN          BIT03_T
#define R_BUSY_END_IEN          BIT04_T
#define R_R2N_RDY_INT_IEN       BIT05_T
#define R_CARD_CHANGE_IEN       BIT06_T
#define R_CARD2_CHANGE_IEN      BIT07_T

//============================================
//MMA_PRI_REG: offset 0x02
//============================================
#define R_MIU_R_PRIORITY        BIT00_T
#define R_MIU_W_PRIORITY        BIT01_T

#define R_MIU1_SELECT           BIT02_T
#define R_MIU2_SELECT           BIT03_T
#define R_MIU3_SELECT           (BIT03_T|BIT02_T)

#define R_MIU_BUS_BURST2        BIT04_T
#define R_MIU_BUS_BURST4        BIT05_T
#define R_MIU_BUS_BURST8        (BIT05_T|BIT04_T)

//============================================
//MIE_FUNC_CTL: offset 0x07
//============================================
#define R_EMMC_EN               BIT00_T
#define R_SD_EN                 BIT01_T
#define R_SDIO_MODE             BIT02_T


//============================================
//SD_MODE: offset 0x0B
//============================================
#define R_CLK_EN                BIT00_T
#define R_BUS_WIDTH_4           BIT01_T
#define R_BUS_WIDTH_8           BIT02_T
#define R_DEST_R2N              BIT04_T
#define	R_DATASYNC              BIT05_T
#define	R_DMA_RD_CLK_STOP       BIT07_T
#define R_DIS_WR_BUSY_CHK       BIT08_T


//============================================
//SD_CTL: offset 0x0C
//============================================
#define R_RSPR2_EN              BIT00_T
#define R_RSP_EN                BIT01_T
#define R_CMD_EN                BIT02_T
#define R_DTRX_EN               BIT03_T
#define R_JOB_DIR               BIT04_T
#define R_ADMA_EN               BIT05_T
#define R_JOB_START             BIT06_T
#define R_CHK_CMD               BIT07_T
#define R_BUSY_DET_ON           BIT08_T
#define R_ERR_DET_ON            BIT09_T


//============================================
//SD_STS: offset 0x0D
//============================================
#define R_DAT_RD_CERR           BIT00_T
#define R_DAT_WR_CERR           BIT01_T
#define R_DAT_WR_TOUT           BIT02_T
#define R_CMD_NORSP             BIT03_T
#define R_CMDRSP_CERR           BIT04_T
#define R_DAT_RD_TOUT           BIT05_T
#define R_CARD_BUSY             BIT06_T
#define R_DAT0                  BIT08_T
#define R_DAT1                  BIT09_T
#define R_DAT2                  BIT10_T
#define R_DAT3                  BIT11_T
#define R_DAT4                  BIT12_T
#define R_DAT5                  BIT13_T
#define R_DAT6                  BIT14_T
#define R_DAT7                  BIT15_T


//============================================
//BOOT_MOD:offset 0x0E
//============================================
#define R_BOOT_MODE             BIT02_T


//============================================
//DDR_MOD: offset 0x0F
//============================================
#define R_PAD_IN_BYPASS         BIT00_T
#define R_PAD_IN_RDY_SEL        BIT01_T
#define R_PRE_FULL_SEL0         BIT02_T
#define R_PRE_FULL_SEL1         BIT03_T
#define R_DDR_MACRO_EN          BIT07_T
#define R_DDR_EN                BIT08_T
#define R_PAD_CLK_SEL           BIT10_T
#define R_PAD_IN_SEL_IP         BIT11_T
#define R_DDR_MACRO32_EN        BIT12_T
#define R_PAD_IN_SEL            BIT13_T
#define R_FALL_LATCH            BIT14_T
#define R_PAD_IN_MASK           BIT15_T


//============================================
//SDIO_MOD: offset 0x11
//============================================
#define R_SDIO_INT_MOD0         BIT00_T
#define R_SDIO_INT_MOD1         BIT01_T
#define R_SDIO_INT_MOD_SW_EN    BIT02_T
#define R_SDIO_DET_INT_SRC      BIT03_T
#define R_SDIO_INT_TUNE0        BIT04_T
#define R_SDIO_INT_TUNE1        BIT05_T
#define R_SDIO_INT_TUNE2        BIT06_T
#define R_SDIO_INT_TUNE_CLR0    BIT07_T
#define R_SDIO_INT_TUNE_CLR1    BIT08_T
#define R_SDIO_INT_TUNE_CLR2    BIT09_T
#define R_SDIO_RDWAIT_EN        BIT11_T
#define R_SDIO_BLK_GAP_DIS      BIT12_T
#define R_SDIO_INT_STOP_DMA     BIT13_T
#define R_SDIO_INT_TUNE_SW      BIT14_T
#define R_SDIO_INT_ASYN_EN      BIT15_T


//============================================
//TEST_MOD: offset 0x15
//============================================
#define R_SDDR1                 BIT00_T
#define R_SD_DEBUG_MOD0         BIT01_T
#define R_SD_DEBUG_MOD1         BIT02_T
#define R_SD_DEBUG_MOD2         BIT03_T
#define R_BIST_MODE             BIT04_T


//============================================
//WR_SBIT_TIMER: offset 0x17
//============================================
#define R_WR_SBIT_TIMER_EN      BIT15_T


//============================================
//RD_SBIT_TIMER: offset 0x18
//============================================
#define R_RD_SBIT_TIMER_EN      BIT15_T


//============================================
//SDIO_DET_ON: offset 0x2F
//============================================
#define R_SDIO_DET_ON           BIT00_T


//============================================
//CIFD_EVENT: offset 0x30
//============================================
#define R_WBUF_FULL             BIT00_T
#define R_WBUF_EMPTY_TRIG       BIT01_T
#define R_RBUF_FULL_TRIG        BIT02_T
#define R_RBUF_EMPTY            BIT03_T


//============================================
//CIFD_INT_EN: offset 0x31
//============================================
#define R_WBUF_FULL_IEN         BIT00_T
#define R_RBUF_EMPTY_IEN        BIT01_T
#define R_F_WBUF_FULL_INT       BIT08_T
#define R_F_RBUF_EMPTY_INT      BIT09_T


//============================================
//BOOT_MODE:offset 0x37
//============================================
#define R_NAND_BOOT_EN          BIT00_T
#define R_BOOTSRAM_ACCESS_SEL   BIT01_T
#define R_IMI_SEL               BIT02_T


//============================================
//CIFD_INT_EN: offset 0x39
//============================================
#define R_DEBUG_MOD0            BIT08_T
#define R_DEBUG_MOD1            BIT09_T
#define R_DEBUG_MOD2            BIT10_T
#define R_DEBUG_MOD3            BIT11_T


//============================================
//FCIE_RST:offset 0x3F
//============================================
#define R_FCIE_SOFT_RST         BIT00_T
#define R_RST_MIU_STS           BIT01_T
#define R_RST_MIE_STS           BIT02_T
#define	R_RST_MCU_STS           BIT03_T
#define	R_RST_ECC_STS           BIT04_T



#endif //End of __HAL_CARD_FCIE5_H