/*
* regSERFLASH.h- Sigmastar
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

#ifndef _REG_SERFLASH_H_
#define _REG_SERFLASH_H_
#include "asm/arch/mach/platform.h"
//#include "mhal_chiptop_reg.h"

//-------------------------------------------------------------------------------------------------
//  Hardware Capability
//-------------------------------------------------------------------------------------------------

// !!! Uranus Serial Flash Notes: !!!
//  - The clock of DMA & Read via XIU operations must be < 3*CPU clock
//  - The clock of DMA & Read via XIU operations are determined by only REG_ISP_CLK_SRC; other operations by REG_ISP_CLK_SRC only
//  - DMA program can't run on DRAM, but in flash ONLY
//  - DMA from SPI to DRAM => size/DRAM start/DRAM end must be 8-B aligned


//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

// BASEADDR & BK

#define IO_ADDRESS(x) (x)

#define BASE_REG_ISP_ADDR			GET_REG_ADDR(IO_ADDRESS(MS_BASE_REG_RIU_PA), 0x000400)
//#define BASE_REG_PM_MISC_ADDR		GET_REG_ADDR(IO_ADDRESS(MS_BASE_REG_RIU_PA), 0x001700)
#define BASE_REG_PIU_ADDR			GET_REG_ADDR(IO_ADDRESS(MS_BASE_REG_RIU_PA), 0x001E00)
//#define BASE_REG_CLKGEN0_ADDR		GET_REG_ADDR(IO_ADDRESS(MS_BASE_REG_RIU_PA), 0x080580)
#define BASE_REG_GDMA_ADDR			GET_REG_ADDR(IO_ADDRESS(MS_BASE_REG_RIU_PA), 0x000380)

//Bit operation
#ifndef BIT
#define BIT(_bit_)                  (1 << (_bit_))
#endif

#define BITS(_bits_, _val_)         ((BIT(((1)?_bits_)+1)-BIT(((0)?_bits_))) & (_val_<<((0)?_bits_)))
#define BMASK(_bits_)               (BIT(((1)?_bits_)+1)-BIT(((0)?_bits_)))

// ISP_CMD

#define REG_ISP_PASSWORD            0x00 // ISP / XIU read / DMA mutual exclusive
#define REG_ISP_SPI_COMMAND         0x01
    // please refer to the serial flash datasheet
    #define ISP_SPI_CMD_READ                BITS(7:0, 0x03)
    #define ISP_SPI_CMD_FASTREAD            BITS(7:0, 0x0B)
    #define ISP_SPI_CMD_RDID                BITS(7:0, 0x9F)
    #define ISP_SPI_CMD_WREN                BITS(7:0, 0x06)
    #define ISP_SPI_CMD_WRDI                BITS(7:0, 0x04)
    #define ISP_SPI_CMD_SE                  BITS(7:0, 0x20)
    #define ISP_SPI_CMD_32BE                BITS(7:0, 0x52)
    #define ISP_SPI_CMD_64BE                BITS(7:0, 0xD8)
    #define ISP_SPI_CMD_CE                  BITS(7:0, 0xC7)
    #define ISP_SPI_CMD_PP                  BITS(7:0, 0x02)
    #define ISP_SPI_CMD_RDSR                BITS(7:0, 0x05)
    #define ISP_SPI_CMD_RDSR2               BITS(7:0, 0x35) // support for new WinBond Flash
    #define ISP_SPI_CMD_WRSR                BITS(7:0, 0x01)
    #define ISP_SPI_CMD_DP                  BITS(7:0, 0xB9)
    #define ISP_SPI_CMD_RDP                 BITS(7:0, 0xAB)
    #define ISP_SPI_CMD_RES                 BITS(7:0, 0xAB)
    #define ISP_SPI_CMD_REMS                BITS(7:0, 0x90)
    #define ISP_SPI_CMD_REMS4               BITS(7:0, 0xCF) // support for new MXIC Flash
    #define ISP_SPI_CMD_PARALLEL            BITS(7:0, 0x55)
    #define ISP_SPI_CMD_EN4K                BITS(7:0, 0xA5)
    #define ISP_SPI_CMD_EX4K                BITS(7:0, 0xB5)
	/* MXIC Individual Block Protection Mode */
    #define ISP_SPI_CMD_WPSEL               BITS(7:0, 0x68)
    #define ISP_SPI_CMD_SBLK                BITS(7:0, 0x36)
    #define ISP_SPI_CMD_SBULK               BITS(7:0, 0x39)
    #define ISP_SPI_CMD_RDSCUR              BITS(7:0, 0x2B)
    #define ISP_SPI_CMD_RDBLOCK             BITS(7:0, 0x3C)
    #define ISP_SPI_CMD_GBLK                BITS(7:0, 0x7E)
    #define ISP_SPI_CMD_GBULK               BITS(7:0, 0x98)
#define REG_ISP_SPI_ADDR_L          0x02 // A[15:0]
#define REG_ISP_SPI_ADDR_H          0x03 // A[23:16]
#define REG_ISP_SPI_WDATA           0x04
    #define ISP_SPI_WDATA_DUMMY     BITS(7:0, 0xFF)
#define REG_ISP_SPI_RDATA           0x05
#define REG_ISP_SPI_CLKDIV          0x06 // clock = CPU clock / this div
	#define ISP_SPI_CLKDIV2			BIT(0)
	#define	ISP_SPI_CLKDIV4			BIT(2)
	#define	ISP_SPI_CLKDIV8			BIT(6)
	#define	ISP_SPI_CLKDIV16    	BIT(7)
	#define	ISP_SPI_CLKDIV32		BIT(8)
	#define	ISP_SPI_CLKDIV64		BIT(9)
	#define	ISP_SPI_CLKDIV128		BIT(10)
#define REG_ISP_DEV_SEL             0x07
#define REG_ISP_SPI_CECLR           0x08
    #define ISP_SPI_CECLR                   BITS(0:0, 1)
#define REG_ISP_SPI_RDREQ           0x0C
    #define ISP_SPI_RDREQ                   BITS(0:0, 1)
#define REG_ISP_SPI_ENDIAN          0x0F
#define REG_ISP_SPI_RD_DATARDY      0x15
    #define ISP_SPI_RD_DATARDY_MASK     BMASK(0:0)
    #define ISP_SPI_RD_DATARDY              BITS(0:0, 1)
#define REG_ISP_SPI_WR_DATARDY      0x16
    #define ISP_SPI_WR_DATARDY_MASK     BMASK(0:0)
    #define ISP_SPI_WR_DATARDY              BITS(0:0, 1)
#define REG_ISP_SPI_WR_CMDRDY       0x17
    #define ISP_SPI_WR_CMDRDY_MASK      BMASK(0:0)
    #define ISP_SPI_WR_CMDRDY               BITS(0:0, 1)
#define REG_ISP_TRIGGER_MODE        0x2a
#define REG_ISP_CHIP_SEL            0x36
    #define SFSH_CHIP_SEL_MASK          BMASK(6:0)
    #define SFSH_CHIP_SEL_FLASH1            BIT(0)
    #define SFSH_CHIP_SEL_FLASH2            BIT(1)
    #define SFSH_CHIP_SEL_SPI_DEV1          BIT(2)
    #define SFSH_CHIP_SEL_SPI_DEV2          BIT(3)
    #define SFSH_CHIP_SEL_SPI_DEV3          BIT(4)
    #define SFSH_CHIP_SEL_SPI_DEV4          BIT(5)
    #define SFSH_CHIP_SEL_SPI_DEV5          BIT(6)
//    #define SFSH_CHIP_SEC_MASK          BMASK(7:0)          // 0x00FF // TODO: review this define
    #define SFSH_CHIP_SEL_MODE_SEL_MASK BMASK(7:7)
    #define SFSH_CHIP_SEL_RIU               BITS(7:7, 1)    // 0x0080
    #define SFSH_CHIP_SEL_XIU               BITS(7:7, 0)    // 0x0000
#define REG_ISP_CHIP_RST            0x3F // SPI clock source  [0]:gate  [1]:inv  [4:2]:clk_sel  000:Xtal clock 001:27MHz 010:36MHz 011:43.2MHz 100:54MHz 101:72MHz 110:86MHz 111:108MHz  [5]:0:xtal 1:clk_Sel
    #define SFSH_CHIP_RESET_MASK          BMASK(2:2)
    #define SFSH_CHIP_RESET                 BITS(2:2, 0)
    #define SFSH_CHIP_NOTRESET              BITS(2:2, 1)
#define REG_ISP_SPI_MODE            0x72
    #define SFSH_CHIP_FAST_MASK          BMASK(0:0) // SPI CMD [0x0B]
    #define SFSH_CHIP_FAST_ENABLE           BITS(0:0, 1)
    #define SFSH_CHIP_FAST_DISABLE          BITS(0:0, 0)
    #define SFSH_CHIP_2XREAD_MASK        BMASK(2:2) // SPI CMD [0xBB]
    #define SFSH_CHIP_2XREAD_ENABLE         BITS(2:2, 1)
    #define SFSH_CHIP_2XREAD_DISABLE        BITS(2:2, 0)
#define REG_ISP_SPI_CHIP_SELE       0x7A
    #define SFSH_CHIP_SELE_MASK          BMASK(1:0) // only for secure booting = 0;
    #define SFSH_CHIP_SELE_EXT1             BITS(1:0, 0)
    #define SFSH_CHIP_SELE_EXT2             BITS(1:0, 1)
    #define SFSH_CHIP_SELE_EXT3             BITS(1:0, 2)
#define REG_ISP_SPI_CHIP_SELE_BUSY  0x7B
    #define SFSH_CHIP_SELE_BUSY_MASK     BMASK(0:0)
    #define SFSH_CHIP_SELE_SWITCH           BITS(0:0, 1)
    #define SFSH_CHIP_SELE_DONE             BITS(0:0, 0)

//// PIU_DMA
//
//#define REG_PIU_DMA_STATUS          0x10 // [1]done [2]busy [8:15]state
//    #define PIU_DMA_DONE_MASK           BMASK(0:0)
//    #define PIU_DMA_DONE                    BITS(0:0, 1)
//    #define PIU_DMA_BUSY_MASK           BMASK(1:1)
//    #define PIU_DMA_BUSY                    BITS(1:1, 1)
//    #define PIU_DMA_STATE_MASK          BMASK(15:8)
//#define REG_PIU_SPI_CLK_SRC         0x26 // SPI clock source  [0]:gate  [1]:inv  [4:2]:clk_sel  000:Xtal clock 001:27MHz 010:36MHz 011:43.2MHz 100:54MHz 101:72MHz 110:86MHz 111:108MHz  [5]:0:xtal 1:clk_Sel
//    #define PIU_SPI_RESET_MASK          BMASK(8:8)
//    #define PIU_SPI_RESET                   BITS(8:8, 1)
//    #define PIU_SPI_NOTRESET                BITS(8:8, 0)
//    #define PSCS_DISABLE_MASK           BMASK(0:0)
//    #define PSCS_INVERT_MASK            BMASK(1:1)
//    #define PSCS_CLK_SEL_MASK           BMASK(4:2)
//    #define PSCS_CLK_SEL_XTAL               BITS(4:2, 0)
//    #define PSCS_CLK_SEL_27MHZ              BITS(4:2, 1)
//    #define PSCS_CLK_SEL_36MHZ              BITS(4:2, 2)
//    #define PSCS_CLK_SEL_43MHZ              BITS(4:2, 3)
//    #define PSCS_CLK_SEL_54MHZ              BITS(4:2, 4)
//    #define PSCS_CLK_SEL_72MHZ              BITS(4:2, 5)
//    #define PSCS_CLK_SEL_86MHZ              BITS(4:2, 6)
//    #define PSCS_CLK_SEL_108MHZ             BITS(4:2, 7)
//    #define PSCS_CLK_SRC_SEL_MASK       BMASK(5:5)
//    #define PSCS_CLK_SRC_SEL_XTAL           BITS(5:5, 0)
//    #define PSCS_CLK_SRC_SEL_CLK            BITS(5:5, 1)
//#define REG_PIU_DMA_SPISTART_L      0x70 // [15:0]
//#define REG_PIU_DMA_SPISTART_H      0x71 // [23:16]
//#define REG_PIU_DMA_DRAMSTART_L     0x72 // [15:0]  in unit of B; must be 8B aligned
//#define REG_PIU_DMA_DRAMSTART_H     0x73 // [23:16]
//#define REG_PIU_DMA_SIZE_L          0x74 // [15:0]  in unit of B; must be 8B aligned
//#define REG_PIU_DMA_SIZE_H          0x75 // [23:16]
//#define REG_PIU_DMA_CMD             0x76
//    #define PIU_DMA_CMD_FIRE            0x0001
//    #define PIU_DMA_CMD_LE              0x0000
//    #define PIU_DMA_CMD_BE              0x0020

// Serial Flash Register // please refer to the serial flash datasheet
#define SF_SR_WIP_MASK                  BMASK(0:0)
#define SF_SR_WEL_MASK                  BMASK(1:1)
#define SF_SR_BP_MASK                   BMASK(5:2) // BMASK(4:2) is normal case but SERFLASH_TYPE_MX25L6405 use BMASK(5:2)
#define SF_SR_PROG_ERASE_ERR_MASK       BMASK(6:6)
#define SF_SR_SRWD_MASK                 BMASK(7:7)
    #define SF_SR_SRWD                      BITS(7:7, 1)
#if 0
// PM_SLEEP CMD.
#define REG_PM_CKG_SPI              0x20 // Ref spec. before using these setting.
    #define PM_SPI_CLK_SEL_MASK         BMASK(11:10)
//    #define PM_SPI_CLK_XTALI                BITS(13:10, 0)
//    #define PM_SPI_CLK_54MHZ                BITS(13:10, 1)
//    #define PM_SPI_CLK_86MHZ                BITS(13:10, 2)
//    #define PM_SPI_CLK_108MHZ               BITS(13:10, 3)
    #define PM_SPI_CLK_XTALI                BITS(11:10, 3)
    #define PM_SPI_CLK_54MHZ                BITS(11:10, 0)
//    #define PM_SPI_CLK_86MHZ                BITS(13:10, 2)
//    #define PM_SPI_CLK_108MHZ               BITS(13:10, 3)
    #define PM_SPI_CLK_SWITCH_MASK      BMASK(14:14)
    #define PM_SPI_CLK_SWITCH_OFF           BITS(14:14, 0)
    #define PM_SPI_CLK_SWITCH_ON            BITS(14:14, 1)
// For Power Consumption
#define REG_PM_GPIO_SPICZ_OEN		0x17
#define REG_PM_GPIO_SPICK_OEN		0x18
#define REG_PM_GPIO_SPIDI_OEN		0x19
#define REG_PM_GPIO_SPIDO_OEN		0x1A
#define REG_PM_SPI_IS_GPIO			0x35
	#define PM_SPI_GPIO_MASK			BMASK(3:0)
	#define PM_SPI_IS_GPIO					BITS(3:0, 0xF)
	#define PM_SPI_NOT_GPIO					BITS(3:0, 0x0)

// CLK_GEN0
#define REG_CLK0_CKG_SPI            0x16
    #define CLK0_CKG_SPI_MASK           BMASK(5:2)
    #define CLK0_CKG_SPI_XTALI              BITS(5:2, 0)
    #define CLK0_CKG_SPI_54MHZ              BITS(5:2, 1)
    #define CLK0_CKG_SPI_86MHZ              BITS(5:2, 2)
    #define CLK0_CKG_SPI_108MHZ             BITS(5:2, 3)
    #define CLK0_CLK_SWITCH_MASK        BMASK(6:6)
    #define CLK0_CLK_SWITCH_OFF           BITS(6:6, 0)
    #define CLK0_CLK_SWITCH_ON            BITS(6:6, 1)
#endif

//FSP REG
#define FSP_OFFSET      	0x60
#define REG_FSP_WD0   		(FSP_OFFSET+0x00)
#define REG_FSP_WD0_MASK	BMASK(7:0)
	#define FSP_WD0(byte)	BITS(7:0,(byte))
#define REG_FSP_WD1   		(FSP_OFFSET+0x00)
#define REG_FSP_WD1_MASK	BMASK(15:8)
	#define FSP_WD1(byte)	BITS(15:8,(byte))
#define REG_FSP_WD2   		(FSP_OFFSET+0x01)
#define REG_FSP_WD2_MASK	BMASK(7:0)
	#define FSP_WD2(byte)	BITS(7:0,(byte))
#define REG_FSP_WD3   		(FSP_OFFSET+0x01)
#define REG_FSP_WD3_MASK	BMASK(15:8)
	#define FSP_WD3(byte)	BITS(15:8,(byte))
#define REG_FSP_WD4   		(FSP_OFFSET+0x02)
#define REG_FSP_WD4_MASK	BMASK(7:0)
	#define FSP_WD4(byte)	BITS(7:0,(byte))
#define REG_FSP_WD5   		(FSP_OFFSET+0x02)
#define REG_FSP_WD5_MASK	BMASK(15:8)
	#define FSP_WD5(byte)		BITS(15:8,(byte))
#define REG_FSP_WD6   		(FSP_OFFSET+0x03)
#define REG_FSP_WD6_MASK	BMASK(7:0)
	#define FSP_WD6(byte)	BITS(7:0,(byte))
#define REG_FSP_WD7   		(FSP_OFFSET+0x03)
#define REG_FSP_WD7_MASK	BMASK(15:8)
	#define FSP_WD7(byte)	BITS(15:8,(byte))
#define REG_FSP_WD8   		(FSP_OFFSET+0x04)
#define REG_FSP_WD8_MASK	BMASK(7:0)
	#define FSP_WD8(byte)	BITS(7:0,(byte))
#define REG_FSP_WD9   		(FSP_OFFSET+0x04)
#define REG_FSP_WD9_MASK	BMASK(15:8)
	#define FSP_WD9(byte)	BITS(15:8,(byte))

#define REG_FSP_RD0   		(FSP_OFFSET+0x05)
#define REG_FSP_RD0_MASK	BMASK(7:0)
	#define FSP_RD0(byte)	BITS(7:0,(byte))
#define REG_FSP_RD1   		(FSP_OFFSET+0x05)
#define REG_FSP_RD1_MASK	BMASK(15:8)
	#define FSP_RD1(byte)	BITS(15:8,(byte))
#define REG_FSP_RD2   		(FSP_OFFSET+0x06)
#define REG_FSP_RD2_MASK	BMASK(7:0)
	#define FSP_RD2(byte)	BITS(7:0,(byte))
#define REG_FSP_RD3   		(FSP_OFFSET+0x06)
#define REG_FSP_RD3_MASK	BMASK(15:8)
	#define FSP_RD3(byte)	BITS(15:8,(byte))
#define REG_FSP_RD4   		(FSP_OFFSET+0x07)
#define REG_FSP_RD4_MASK	BMASK(7:0)
	#define FSP_RD4(byte)	BITS(7:0,(byte))
#define REG_FSP_RD5   		(FSP_OFFSET+0x07)
#define REG_FSP_RD5_MASK	BMASK(15:8)
	#define FSP_RD5(byte)	BITS(15:8,(byte))
#define REG_FSP_RD6   		(FSP_OFFSET+0x08)
#define REG_FSP_RD6_MASK	BMASK(7:0)
	#define FSP_RD6(byte)	BITS(7:0,(byte))
#define REG_FSP_RD7   		(FSP_OFFSET+0x08)
#define REG_FSP_RD7_MASK	BMASK(15:8)
	#define FSP_RD7(byte)	BITS(15:8,(byte))
#define REG_FSP_RD8   		(FSP_OFFSET+0x09)
#define REG_FSP_RD8_MASK	BMASK(7:0)
	#define FSP_RD8(byte)	BITS(7:0,(byte))
#define REG_FSP_RD9   		(FSP_OFFSET+0x09)
#define REG_FSP_RD9_MASK	BMASK(15:8)
	#define FSP_RD9(byte)	BITS(15:8,(byte))

#define REG_FSP_WBF_SIZE		(FSP_OFFSET+0x0A)
#define REG_FSP_WBF_SIZE0_MASK	BMASK(3:0)
	#define FSP_WBF_SIZE0(bits)	BITS(3:0,(bits))
#define REG_FSP_WBF_SIZE1_MASK	BMASK(7:4)
	#define FSP_WBF_SIZE1(bits)	BITS(7:4,(bits))
#define REG_FSP_WBF_SIZE2_MASK	BMASK(11:8)
	#define FSP_WBF_SIZE2(bits)	BITS(11:8,(bits))

#define REG_FSP_RBF_SIZE		(FSP_OFFSET+0x0B)
#define REG_FSP_RBF_SIZE0_MASK	BMASK(3:0)
	#define FSP_RBF_SIZE0(bits)	BITS(3:0,(bits))
#define REG_FSP_RBF_SIZE1_MASK	BMASK(7:4)
	#define FSP_RBF_SIZE1(bits)	BITS(7:4,(bits))
#define REG_FSP_RBF_SIZE2_MASK	BMASK(11:8)
	#define FSP_RBF_SIZE2(bits)	BITS(11:8,(bits))

#define REG_FSP_CTRL			(FSP_OFFSET+0x0C)
#define REG_FSP_CTRL1_2CMD_MASK	BMASK(15:15)
	#define FSP_CTRL1_2CMD(bit)	BITS(15:15,(bit))
#define REG_FSP_CTRL1_3CMD_MASK	BMASK(14:14)
	#define	FSP_CTRL1_3CMD(bit) BITS(14:14,(bit))
#define REG_FSP_CTRL1_ACS_MASK	BMASK(13:13)
	#define FSP_CTRL1_ACS(bit)	BITS(13:13,(bit))
#define REG_FSP_CTRL1_RDSR_MASK	BMASK(12:11)
	#define FSP_CTRL1_RDSR_1CMD	BITS(12:11, 0)
	#define FSP_CTRL1_RDSR_2CMD	BITS(12:11, 1)
	#define FSP_CTRL1_RDSR_3CMD	BITS(12:11, 2)
#define REG_FSP_CTRL0_ACE_MASK	BMASK(3:3)
	#define	FSP_CTRL0_ACE(bit)	BITS(3:3,(bit))
#define REG_FSP_CTRL0_INT_MASK	BMASK(2:2)
	#define FSP_CTRL0_INT(bit)	BITS(2:2,(bit))
#define REG_FSP_CTRL0_RST_MASK	BMASK(1:1)
	#define FSP_CTRL0_RST(bit)	BITS(1:1,(bit))
#define REG_FSP_CTRL0_FSP_MASK	BMASK(0:0)
	#define	FSP_CTRL0_FSP(bit)	BITS(0:0,(bit))
#define REG_FSP_TRIGGER			(FSP_OFFSET+0x0D)
	#define REG_FSP_FIRE_MASK	BMASK(0:0)
	#define REG_FSP_FIRE		BITS(0:0,(1))
#define REG_FSP_DONE_FLAG		(FSP_OFFSET+0x0E)
	#define	REG_FSP_DONE_MASK   BMASK(0:0) 
	#define FSP_DONE_FLAG       BITS(0:0,(1))
	#define FSP_AUTO_CHK_ERR    BITS(0:0,(0))
#define REG_FSP_CLR_FLAG		(FSP_OFFSET+0x0F)
	#define FSP_CLR_FLAG_MASK	BMASK(0:0)
	#define FSP_CLR_FLAG		BITS(0:0,(1))

//FSP_CTRL0
#define FSP_ENABLE          BIT(0)
#define FSP_NOT_RESET       BIT(1)
#define FSP_INT_ENABLE      BIT(2)
#define FSP_AUTOCHK_ENABLE  BIT(3)
//FSP_CTRL1
#define FSP_RDSR_FIRST                 0x00
#define FSP_RDSR_SECOND                0x01
#define FSP_RDSR_THIRD                 0x10
#define FSP_FLASH_STATUS_AUTO_CHK      BIT(5)
#define FSP_THIRD_CMD_ENABLE           BIT(6)
#define FSP_SECOND_CMD_ENABLE          BIT(7)
//FSP_TRIGGER
#define FSP_TRIGGER_ENABLE          BIT(0)
//FSP_CLEAR
#define FSP_CLEAR_DONE_FLAG         BIT(0)
#endif // _REG_SERFLASH_H_
