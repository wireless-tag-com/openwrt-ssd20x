/*
* hal_card_regs_v4.h- Sigmastar
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
 * FileName hal_card_regs_v4.h
 *     @author jeremy.wang (2015/08/13)
 * Desc:
 * 	   This file is the header file for hal_card_regs.h
 *
 * 	   We add a new header file to describe the meaning positions of fcie5 registers
 *
 ***************************************************************************************************************/

#ifndef __HAL_CARD_REGS_V4_H
#define __HAL_CARD_REGS_V4_H

//============================================
//MIE_EVENT:offset 0x00
//============================================
#define R_MMA_DATA_END          BIT00_T
#define R_SD_CMD_END            BIT01_T
#define R_SD_DATA_END           BIT02_T
#define R_MS_DATA_END           BIT03_T
#define R_CF_CMD_END            BIT04_T
#define R_CF_DATA_END           BIT05_T
#define R_SM_JOB_END            BIT06_T
#define R_SDIO_INT              BIT07_T
#define R_MIU_WP_ERR            BIT08_T
#define R_NC_JOB_END            BIT09_T
#define R_NC_RIU2NAND_END       BIT10_T
#define R_CARD_DMA_END          BIT11_T
#define R_NC_R2N_ECC_ERR        BIT12_T
#define R_PWR_SAVE_END          BIT13_T
#define R_MMA_LSDONE_END        BIT14_T
#define R_BOOT_DONE_END         BIT15_T  //NEW

//============================================
//MIE_INT_EN:offset 0x01
//============================================
#define R_MMA_DATA_END_EN       BIT00_T
#define R_SD_CMD_END_EN         BIT01_T
#define R_SD_DATA_END_EN        BIT02_T
#define R_MS_DATA_END_EN        BIT03_T
#define R_CF_CMD_END_EN         BIT04_T
#define R_CF_DATA_END_EN        BIT05_T
#define R_SM_JOB_END_EN         BIT06_T
#define R_SDIO_INT_EN           BIT07_T
#define R_MIU_WP_ERR_EN         BIT08_T
#define R_NC_JOB_END_EN         BIT09_T
#define R_NC_RIU2NAND_EN    	BIT10_T
#define R_CARD_DMA_END_EN   	BIT11_T
#define R_PWR_SAVE_END_EN   	BIT13_T
#define R_MMA_LSDONE_END_EN     BIT14_T
#define R_BOOT_DONE_END_EN      BIT15_T  //NEW

//============================================
//MMA_PRIORITY:offset 0x02
//============================================
#define R_MMA_R_PRIORITY        BIT00_T
#define R_MMA_W_PRIORITY        BIT01_T
#define R_JOB_RW_DIR            BIT02_T
#define R_DATA_SCRAM_EN         BIT03_T
#define R_MIU_REQ_RST           BIT04_T
#define R_FIFO_CLK_RDY          BIT05_T
#define R_MIU_BUS_BURST16       BIT08_T
#define R_MIU_BUS_BURST32       BIT09_T
#define R_MIU_BUS_CTRL_EN       BIT10_T
#define R_MIU_CLK_EN_SW         BIT12_T  //NEW
#define R_MIU_CLK_EN_HW         BIT13_T  //NEW
#define R_MIU_CLK_CTL_SW        BIT14_T  //NEW
#define R_MIU_CLK_FREE          BIT15_T  //NEW

//============================================
//CARD_EVENT:offset 0x05
//============================================
#define R_SD_STS_CHG            BIT00_T
#define R_MS_STS_CHG            BIT01_T
#define R_CF_STS_CHG            BIT02_T
#define	R_SM_STS_CHG            BIT03_T
#define	R_XD_STS_CHG            BIT04_T
#define R_SD_PWR_OC_CHG         BIT05_T
#define R_CF_PWR_OC_CHG         BIT06_T
#define R_SDIO_STS_CHG          BIT07_T
#define R_SDIO2_STS_CHG         BIT08_T

//============================================
//CARD_INT_EN:offset 0x06
//============================================
#define R_SD_STS_EN             BIT00_T
#define R_MS_STS_EN             BIT01_T
#define R_CF_STS_EN             BIT02_T
#define	R_SM_STS_EN             BIT03_T
#define	R_XD_STS_EN             BIT04_T
#define R_SD_PWR_OC_DET_EN      BIT05_T
#define R_CF_PWR_OC_DET_EN      BIT06_T
#define R_SD_CD_SRC             BIT07_T // 0: SD_CDZ, 1: SD_DAT3
#define R_SDIO_STS_EN           BIT08_T
#define R_SDIO_CD_SRC           BIT09_T
#define R_SDIO2_STS_EN          BIT10_T

//============================================
//CARD_DET:offset 0x07
//============================================
#define R_SD_DET_N              BIT00_T
#define R_MS_DET_N              BIT01_T
#define R_CF_DET_N              BIT02_T
#define	R_SM_DET_N              BIT03_T
#define	R_XD_DET_N              BIT04_T
#define R_SD_OCDET_STS          BIT05_T
#define R_CF_OCDET_STS          BIT06_T
#define R_SDIO_DET_N            BIT07_T
#define R_NF_RDBZ_STS           BIT08_T // C3
#define R_SDIO2_DET_N           BIT09_T

//============================================
//CARD_PWR:offset 0x08
//============================================
#define R_SD_PWR_ON             BIT00_T
#define R_CF_PWR_ON             BIT01_T
#define R_SD_PWR_PAD_OEN        BIT02_T
#define	R_CF_PWR_PAD_OEN        BIT03_T
#define R_RIU2NAND_JOB_END_ACT  BIT06_T // C3 for test
#define R_NC_JOB_END_ACT        BIT07_T // C3 for test

//============================================
//INTR_TEST:offset 0x09 all for test
//============================================
#define R_FORCE_MMA_DATA_END    BIT00_T
#define R_FORCE_SD_CMD_END      BIT01_T
#define R_FORCE_SD_DATA_END     BIT02_T
#define R_FORCE_MS_DATA_END     BIT03_T
#define R_FORCE_CF_REQ_END      BIT04_T
#define R_FORCE_CF_DATA_END     BIT05_T
#define R_FORCE_SM_JOB_END      BIT06_T
#define R_FORCE_NC_JOB_END      BIT06_T // for backward compatible
#define R_FORCE_SDIO_INT        BIT07_T
#define R_FORCE_SD_STS_CHG      BIT08_T
#define R_FORCE_MS_STS_CHG      BIT09_T
#define R_FORCE_CF_STS_CHG      BIT10_T
#define R_FORCE_SM_STS_CHG      BIT11_T
#define R_FORCE_XD_STS_CHG      BIT12_T
#define R_FORCE_SD_OC_STS_CHG   BIT13_T
#define R_FORCE_CF_OC_STS_CHG   BIT14_T
#define R_FORCE_SDIO_STS_CHG    BIT15_T

//============================================
//MIE_PATH_CTL:offset 0x0A
//============================================
#define R_MMA_ENABLE            BIT00_T
#define R_SD_EN                 BIT01_T
#define R_MS_EN                 BIT02_T
#define R_CF_EN                 BIT03_T
#define R_SM_EN                 BIT04_T
#define R_NC_EN                 BIT05_T

//============================================
//JOB_BL_CNT:offset 0xB
//============================================
#define R_JOB_BL_CNT0   BIT00_T
#define R_JOB_BL_CNT1   BIT01_T
#define R_JOB_BL_CNT2   BIT02_T
#define R_JOB_BL_CNT3   BIT03_T
#define R_JOB_BL_CNT4   BIT04_T
#define R_JOB_BL_CNT5   BIT05_T
#define R_JOB_BL_CNT6   BIT06_T
#define R_JOB_BL_CNT7   BIT07_T

//============================================
//TR_BK_CNT:offset 0xC
//============================================
#define R_TR_BK_CNT0   	BIT00_T
#define R_TR_BK_CNT1   	BIT01_T
#define R_TR_BK_CNT2    BIT02_T
#define R_TR_BK_CNT3    BIT03_T
#define R_TR_BK_CNT4    BIT04_T
#define R_TR_BK_CNT5    BIT05_T
#define R_TR_BK_CNT6    BIT06_T
#define R_TR_BK_CNT7    BIT07_T

//============================================
//SD_MODE:offset 0x10
//============================================
#define R_SD_CLK_EN             BIT00_T
#define R_SD_4BITS              BIT01_T
#define R_SD_8BITS              BIT02_T
#define R_SDDRL                 BIT03_T
#define R_SD_CS_EN              BIT04_T
#define R_SD_DEST               BIT05_T // 0: Data FIFO, 1: CIF FIFO
#define R_SD_DATSYNC            BIT06_T
#define R_MMC_BUS_TEST          BIT07_T
#define R_SDIO_RDWAIT           BIT08_T
#define R_SDIO_SD_BUS_SW        BIT09_T
#define R_SD_DMA_RD_CLK_STOP	BIT11_T
#define R_SDIO_PORT_SEL			BIT12_T

//============================================
//SD_CTL:offset 0x11
//============================================
#define R_SD_RSPR2_EN           BIT00_T // Response R2 type
#define R_SD_RSP_EN             BIT01_T
#define R_SD_CMD_EN             BIT02_T
#define R_SD_DTRX_EN            BIT03_T
#define R_SD_DTRX_DIR           BIT04_T // 0: R, 1:W
#define R_SDIO_INT_MOD0         BIT08_T
#define R_SDIO_INT_MOD1         BIT09_T
#define R_SDIO_DET_ON           BIT10_T
#define R_SDIO_DET_INTSRC       BIT11_T

//============================================
//SD_STS:offset 0x12
//============================================
#define R_SD_DAT_CERR           BIT00_T
#define R_SD_DAT_STSERR         BIT01_T
#define R_SD_DAT_STSNEG         BIT02_T
#define R_SD_CMD_NORSP          BIT03_T
#define R_SD_CMDRSP_CERR        BIT04_T
#define R_SD_WR_PRO_N           BIT05_T
#define R_SD_DAT0               BIT08_T
#define R_SD_DAT1               BIT09_T
#define R_SD_DAT2               BIT10_T
#define R_SD_DAT3               BIT11_T
#define R_SD_DAT4               BIT12_T
#define R_SD_DAT5               BIT13_T
#define R_SD_DAT6               BIT14_T
#define R_SD_DAT7               BIT15_T

//============================================
//MS_CTL:offset 0x14
//============================================
#define R_MS_REGTRX_EN          BIT00_T
#define R_MS_DTRX_EN            BIT01_T
#define R_MS_BUS_DIR            BIT02_T // 0: read form card, 1: Write to card
#define R_MS_BURST              BIT03_T
#define R_MS_DEST               BIT04_T

//============================================
//EMMC_PATH_CTL:offset 0x16
//============================================
#define R_EMMC_EN               BIT00_T

//============================================
//CF_CTL:offset 0x1A
//============================================
#define R_CF_REG_EN             BIT00_T
#define R_CF_DTRX_EN            BIT01_T
#define R_CF_BUS_DIR            BIT02_T
#define R_CF_DEST               BIT03_T // 0: Data FIFO, 1: CIF FIFO
#define R_CF_BURST              BIT04_T
#define R_CF_IORDY_DIS          BIT05_T
#define R_CF_BURST_ERR          BIT06_T

//============================================
//SDIO_CTL:offset 0x1B
//============================================
#define R_SDIO_BLK_MOD          BIT15_T

//============================================
//SDIO_STS:offset 0x1E
//============================================
#define R_SDIO_DAT0             BIT00_T
#define R_SDIO_DAT1             BIT01_T
#define R_SDIO_DAT2             BIT02_T
#define R_SDIO_DAT3             BIT03_T

#define R_SDIO2_DAT0            BIT08_T
#define R_SDIO2_DAT1            BIT09_T
#define R_SDIO2_DAT2            BIT10_T
#define R_SDIO2_DAT3            BIT11_T

//============================================
//NC_CIF_FIFO_CTL:offset 0x25
//============================================
// only need in montage serious
#define R_CIFC_RD_REQ           BIT00_T
#define R_CIFD_RD_REQ           BIT01_T

//============================================
//SM_CTL:offset 0x2B
//============================================
#define R_SM_REG_EN             BIT00_T
#define R_SM_DTRX_EN            BIT01_T
#define R_SM_BUS_DIR            BIT02_T
#define R_SM_DEST               BIT03_T
#define R_SM_BURST              BIT04_T


//============================================
//NC_REORDER:offset 0x2D
//============================================
#define R_MIU_WDEN_PATCH_DIS	BIT07_T
#define R_CSREG_PAD_IN_SEL      BIT15_T


//============================================
//eMMC_BOOT_CONFIG:offset 0x2F
//============================================
#define R_BOOT_ST2              BIT00_T
#define R_BOOT_END_EN           BIT01_T
#define R_BOOT_FROM_EN          BIT02_T
#define R_MACRO_EN              BIT08_T
#define R_SDDDRMODE             BIT09_T
#define R_SDYPASSMODE           BIT10_T
#define R_SDRINBYPASS           BIT11_T
#define R_FROM_TMUX             BIT12_T
#define R_CLKOE_DEL_EN          BIT13_T
#define R_SBIT_DDR_SEL          BIT14_T


//============================================
//TEST_MODE:offset 0x30
//============================================
#define R_DBFA_BISTFAIL         BIT00_T
#define R_DBFB_BISTFAIL         BIT01_T
#define R_CIFC_BISTFAIL         BIT02_T
#define R_CIFD_BISTFAIL         BIT03_T
#define R_PAD_SWAP              BIT07_T
#define R_DEBUG_MODE0           BIT08_T
#define R_DEBUG_MODE1           BIT09_T
#define R_DEBUG_MODE2           BIT10_T
#define R_SD_MS_COBUS           BIT11_T
#define R_FCIE_SOFT_RST         BIT12_T
#define R_NFIE_SOFT_RESETZ      BIT12_T // For backward compatible
#define R_ENDIAN_SEL            BIT13_T

//============================================
//NC_MISC offser 0x31
//============================================
//============================================
//PWRSAVE_MASK    0x34
//============================================
//============================================
//RWRSAVE_CTL     0x35
//============================================
#define R_PWRSAVE_EN            BIT00_T
#define R_PWRSAVE_TEST          BIT01_T
#define R_PWRSAVE_INT_EN        BIT02_T
#define R_PWRSAVE_RST_N         BIT03_T // low active
#define R_PWRSAVE_RIU_TEST      BIT05_T
#define R_PWRSAVE_HW_REST       BIT06_T
#define R_PWRSAVE_LOSS_PWR      BIT07_T

//============================================
//MIU_WP_MIN_AH_S :offset 0x38
//============================================
#define R_MIU_WP_EN             BIT08_T
#define R_FORCE_MIU_WP_ERR      BIT09_T
//============================================
//MIU_WP_MIN_AL   :offset 0x39
//============================================
//============================================
//MIU_WP_MAX_AH   :offset 0x3A
//============================================
//============================================
//MIU_WP_MAX_AL   :offset 0x3B
//============================================
//============================================
//MIU_WP_ERR_AH   :offset 0x3C
//============================================
//============================================
//MIU_WP_ERR_AL   :offset 0x3D
//============================================
//============================================
//FCIE_INFO       :offset 0x3F
//============================================
#define R_SDMMC_HW              BIT00_T
#define R_MS_HW                 BIT01_T
#define R_CF_HW                 BIT02_T
#define R_SM_HW                 BIT03_T
#define R_SDIO_HW               BIT04_T
#define R_NAND_HW               BIT05_T
#define R_MIU_BUS_WIDTH         (BIT06_T|WBIT07)
#define R_MIU_BUS_ALIGN_BYTE    BIT08_T
#define R_PROGRAM_PAD           BIT09_T
#define R_PROGRAM_CLK           BIT10_T




#endif //End of __HAL_CARD_REGS_V4_H


