/*
* eMMC_reg_v5.h- Sigmastar
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

#ifndef __eMMC_FCIE_V5_REG_H__
#define __eMMC_FCIE_V5_REG_H__

#define IP_FCIE_VERSION_5

#ifndef NULL
#define NULL    ((void*)0)
#endif
#ifndef BIT0
#define BIT0    (1<<0)
#endif
#ifndef BIT1
#define BIT1    (1<<1)
#endif
#ifndef BIT2
#define BIT2    (1<<2)
#endif
#ifndef BIT3
#define BIT3    (1<<3)
#endif
#ifndef BIT4
#define BIT4    (1<<4)
#endif
#ifndef BIT5
#define BIT5    (1<<5)
#endif
#ifndef BIT6
#define BIT6    (1<<6)
#endif
#ifndef BIT7
#define BIT7    (1<<7)
#endif
#ifndef BIT8
#define BIT8    (1<<8)
#endif
#ifndef BIT9
#define BIT9    (1<<9)
#endif
#ifndef BIT10
#define BIT10   (1<<10)
#endif
#ifndef BIT11
#define BIT11   (1<<11)
#endif
#ifndef BIT12
#define BIT12   (1<<12)
#endif
#ifndef BIT13
#define BIT13   (1<<13)
#endif
#ifndef BIT14
#define BIT14   (1<<14)
#endif
#ifndef BIT15
#define BIT15   (1<<15)
#endif
#ifndef BIT16
#define BIT16   (1<<16)
#endif
#ifndef BIT17
#define BIT17   (1<<17)
#endif
#ifndef BIT18
#define BIT18   (1<<18)
#endif
#ifndef BIT19
#define BIT19   (1<<19)
#endif
#ifndef BIT20
#define BIT20   (1<<20)
#endif
#ifndef BIT21
#define BIT21   (1<<21)
#endif
#ifndef BIT22
#define BIT22   (1<<22)
#endif
#ifndef BIT23
#define BIT23   (1<<23)
#endif
#ifndef BIT24
#define BIT24   (1<<24)
#endif
#ifndef BIT25
#define BIT25   (1<<25)
#endif
#ifndef BIT26
#define BIT26   (1<<26)
#endif
#ifndef BIT27
#define BIT27   (1<<27)
#endif
#ifndef BIT28
#define BIT28   (1<<28)
#endif
#ifndef BIT29
#define BIT29   (1<<29)
#endif
#ifndef BIT30
#define BIT30   (1<<30)
#endif
#ifndef BIT31
#define BIT31   (1<<31)
#endif

//------------------------------------------------------------------
#define FCIE_MIE_EVENT                  GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x00)
#define FCIE_MIE_INT_EN                 GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x01)
#define FCIE_MMA_PRI_REG                GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x02)
#define FCIE_MIU_DMA_ADDR_15_0          GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x03)
#define FCIE_MIU_DMA_ADDR_31_16         GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x04)
#define FCIE_MIU_DMA_LEN_15_0           GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x05)
#define FCIE_MIU_DMA_LEN_31_16          GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x06)
#define FCIE_MIE_FUNC_CTL               GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x07)
#define FCIE_JOB_BL_CNT                 GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x08)
#define FCIE_BLK_SIZE                   GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x09)
#define FCIE_CMD_RSP_SIZE               GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x0A)
#define FCIE_SD_MODE                    GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x0B)
#define FCIE_SD_CTRL                    GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x0C)
#define FCIE_SD_STATUS                  GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x0D)
#define FCIE_BOOT_CONFIG                GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x0E)
#define FCIE_DDR_MODE                   GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x0F)
//#define FCIE_DDR_TOGGLE_CNT             GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x10)
#define FCIE_RESERVED_FOR_SW            GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x10)
#define FCIE_SDIO_MOD                   GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x11)
//#define FCIE_SBIT_TIMER               GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x11)
#define FCIE_RSP_SHIFT_CNT              GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x12)
#define FCIE_RX_SHIFT_CNT               GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x13)
#define FCIE_ZDEC_CTL0                  GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x14)
#define FCIE_TEST_MODE                  GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x15)
#define FCIE_MMA_BANK_SIZE              GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x16)
#define FCIE_WR_SBIT_TIMER              GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x17)
//#define FCIE_SDIO_MODE                GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x17)
#define FCIE_RD_SBIT_TIMER              GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x18)
//#define FCIE_DEBUG_BUS0               GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x1E)
//#define FCIE_DEBUG_BUS1               GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x1F)
#define NC_CIFD_EVENT                   GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x30)
#define NC_CIFD_INT_EN                  GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x31)
#define FCIE_PWR_RD_MASK                GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x34)
#define FCIE_PWR_SAVE_CTL               GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x35)
#define FCIE_BIST                       GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x36)
#define FCIE_BOOT                       GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x37)
#define FCIE_EMMC_DEBUG_BUS0            GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x38)
#define FCIE_EMMC_DEBUG_BUS1            GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x39)
#define FCIE_BIST_PATTERN0              GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x3A)
#define FCIE_RST                        GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x3F)
//#define NC_WIDTH                      GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x41)
#define FCIE_NC_FUN_CTL                 GET_REG_ADDR(FCIE_REG_BASE_ADDR, 0x63)

#define FCIE_CMDFIFO_ADDR(u16_pos)      GET_REG_ADDR(FCIE_CMDFIFO_BASE_ADDR, u16_pos)
#define FCIE_CMDFIFO_BYTE_CNT           0x12// 9 x 16 bits

#define FCIE_CIFD_ADDR(u16_pos)         GET_REG_ADDR(FCIE_CIFD_BASE_ADDR, u16_pos)

#define NC_WBUF_CIFD_ADDR(u16_pos)      GET_REG_ADDR(FCIE_NC_WBUF_CIFD_BASE, u16_pos) // 32 x 16 bits SW Read only
#define NC_RBUF_CIFD_ADDR(u16_pos)      GET_REG_ADDR(FCIE_NC_RBUF_CIFD_BASE, u16_pos) // 32 x 16 bits   SW write/read

#define NC_CIFD_ADDR(u16_pos)           NC_RBUF_CIFD_ADDR(u16_pos)

#define NC_CIFD_WBUF_BYTE_CNT           0x40 // 32 x 16 bits
#define NC_CIFD_RBUF_BYTE_CNT           0x40 // 32 x 16 bits

#define FCIE_CIFD_BYTE_CNT              0x40 // 256 x 16 bits

//------------------------------------------------------------------
/* FCIE_MIE_EVENT  0x00 */
/* FCIE_MIE_INT_EN 0x01 */
#define BIT_DMA_END                     BIT0
#define BIT_SD_CMD_END                  BIT1
#define BIT_ERR_STS                     BIT2
//#define BIT_SD_DATA_END               BIT2
#define BIT_SDIO_INT                    BIT3
#define BIT_BUSY_END_INT                BIT4
#define BIT_R2N_RDY_INT                 BIT5
#define BIT_CARD_CHANGE                 BIT6
#define BIT_CARD2_CHANGE                BIT7
//#define BIT_PWR_SAVE_INT              BIT14
//#define BIT_BOOT_DONE_INT             BIT15
//#define BIT_ALL_CARD_INT_EVENTS         (BIT_DMA_END|BIT_SD_CMD_END|BIT_BUSY_END_INT)
#define BIT_ALL_CARD_INT_EVENTS         (BIT_DMA_END|BIT_SD_CMD_END|BIT_ERR_STS|BIT_BUSY_END_INT)

/* FCIE_MMA_PRI_REG 0x02 */
#define BIT_MIU_R_PRI                   BIT0
#define BIT_MIU_W_PRI                   BIT1
#define BIT_MIU_SELECT_MASK             (BIT3|BIT2)
#define BIT_MIU1_SELECT                 BIT2
#define BIT_MIU2_SELECT                 BIT3
#define BIT_MIU3_SELECT                 (BIT3|BIT2)
//#define BIT_DATA_SCRAMBLE             BIT3
#define BIT_MIU_BUS_TYPE_MASK           (BIT4|BIT5)
#define BIT_MIU_BURST1                  (~BIT_MIU_BUS_TYPE_MASK)
#define BIT_MIU_BURST2                  (BIT4)
#define BIT_MIU_BURST4                  (BIT5)
#define BIT_MIU_BURST8                  (BIT4|BIT5)

/* FCIE_MIE_FUNC_CTL 0x07 */
#define BIT_EMMC_EN                     BIT0
#define BIT_SD_EN                       BIT1
#define BIT_SDIO_MOD                    BIT2

/* FCIE_BLK_CNT 0x08 */
#define BIT_SD_JOB_BLK_CNT_MASK         (BIT13-1)

/* FCIE_CMD_RSP_SIZE 0x0A */
#define BIT_RSP_SIZE_MASK               (BIT6-1)
#define BIT_CMD_SIZE_MASK               (BIT13|BIT12|BIT11|BIT10|BIT9|BIT8)
#define BIT_CMD_SIZE_SHIFT              8

/* FCIE_SD_MODE 0x0B */
#define BIT_CLK_EN                      BIT0
#define BIT_SD_CLK_EN                   BIT_CLK_EN
#define BIT_SD_DATA_WIDTH_MASK          (BIT2|BIT1)
#define BIT_SD_DATA_WIDTH_1             0
#define BIT_SD_DATA_WIDTH_4             BIT1
#define BIT_SD_DATA_WIDTH_8             BIT2
#define BIT_DATA_DEST                   BIT4 // 0: DMA mode, 1: R2N mode
#define BIT_SD_DATA_CIFD                BIT_DATA_DEST
#define BIT_DATA_SYNC                   BIT5
//#define BIT_2SD_1CLK_SRC              BIT6
#define BIT_SD_DMA_R_CLK_STOP           BIT7
#define BIT_DIS_WR_BUSY_CHK             BIT8

#define BIT_SD_DEFAULT_MODE_REG         (BIT_CLK_EN)

/* FCIE_SD_CTRL 0x0C */
#define BIT_SD_RSPR2_EN                 BIT0
#define BIT_SD_RSP_EN                   BIT1
#define BIT_SD_CMD_EN                   BIT2
#define BIT_SD_DTRX_EN                  BIT3
#define BIT_SD_DAT_EN                   BIT_SD_DTRX_EN
#define BIT_SD_DAT_DIR_W                BIT4
#define BIT_ADMA_EN                     BIT5
#define BIT_JOB_START                   BIT6
#define BIT_CHK_CMD                     BIT7
#define BIT_BUSY_DET_ON                 BIT8
#define BIT_ERR_DET_ON                  BIT9

/* FCIE_SD_STATUS 0x0D */
#define BIT_DAT_RD_CERR                 BIT0
#define BIT_SD_R_CRC_ERR                BIT_DAT_RD_CERR
#define BIT_DAT_WR_CERR                 BIT1
#define BIT_SD_W_FAIL                   BIT_DAT_WR_CERR
#define BIT_DAT_WR_TOUT                 BIT2
#define BIT_SD_W_CRC_ERR                BIT_DAT_WR_TOUT
#define BIT_CMD_NO_RSP                  BIT3
#define BIT_SD_RSP_TIMEOUT              BIT_CMD_NO_RSP
#define BIT_CMD_RSP_CERR                BIT4
#define BIT_SD_RSP_CRC_ERR              BIT_CMD_RSP_CERR
#define BIT_DAT_RD_TOUT                 BIT5
//#define BIT_SD_CARD_WP                BIT5
#define BIT_SD_CARD_BUSY                BIT6

#define BITS_ERROR                      (BIT_SD_R_CRC_ERR|BIT_DAT_WR_CERR|BIT_DAT_WR_TOUT|BIT_CMD_NO_RSP|BIT_CMD_RSP_CERR|BIT_DAT_RD_TOUT)

#define BIT_SD_D0                       BIT8 // not correct D0 in 32 bits macron
//#define BIT_SD_DBUS_MASK              (BIT8|BIT9|BIT10|BIT11|BIT12|BIT13|BIT14|BIT15)
//#define BIT_SD_DBUS_SHIFT             8
#define BIT_SD_FCIE_ERR_FLAGS           (BIT5-1)
#define BIT_SD_CARD_D0_ST               BIT8
#define BIT_SD_CARD_D1_ST               BIT9
#define BIT_SD_CARD_D2_ST               BIT10
#define BIT_SD_CARD_D3_ST               BIT11
#define BIT_SD_CARD_D4_ST               BIT12
#define BIT_SD_CARD_D5_ST               BIT13
#define BIT_SD_CARD_D6_ST               BIT14
#define BIT_SD_CARD_D7_ST               BIT15

/* FCIE_BOOT_CONFIG 0x0E */
#define BIT_EMMC_RSTZ                   BIT0
#define BIT_EMMC_RSTZ_EN                BIT1
#define BIT_BOOT_MODE_EN                BIT2
//#define BIT_BOOT_END                  BIT3

/* FCIE_DDR_MODE 0x0F */
//#define BIT_DQS_DELAY_CELL_MASK       (BIT0|BIT1|BIT2|BIT3)
//#define BIT_DQS_DELAY_CELL_SHIFT      0
#define BIT_MACRO_MODE_MASK             (BIT1|BIT7|BIT8|BIT10|BIT12|BIT13|BIT14)
#define BIT_PAD_IN_BYPASS               BIT0
#define BIT_PAD_IN_RDY_SEL              BIT1
#define BIT_PRE_FULL_SEL0               BIT2
#define BIT_PRE_FULL_SEL1               BIT3
#define BIT_8BIT_MACRO_EN               BIT7
#define BIT_DDR_EN                      BIT8
//#define BIT_SDR200_EN                 BIT9
//#define BIT_BYPASS_EN                 BIT10
#define BIT_PAD_CLK_SEL                 BIT10
//#define BIT_SDRIN_BYPASS_EN           BIT11
#define BIT_32BIT_MACRO_EN              BIT12
#define BIT_PAD_IN_SEL_SD               BIT13
#define BIT_FALL_LATCH                  BIT14
#define BIT_PAD_IN_MASK                 BIT15

/* FCIE_TOGGLE_CNT 0x10 */
#define BITS_8_MACRO32_SDR_TOGGLE_CNT   0x210
#define BITS_4_MACRO32_SDR_TOGGLE_CNT   0x410

#define BITS_8_MACRO32_DDR_TOGGLE_CNT   0x110
#define BITS_4_MACRO32_DDR_TOGGLE_CNT   0x210

/* FCIE_SDIO_MOD 0x11 */
#define BIT_REG_SDIO_MOD_MASK           (BIT1|BIT0)
#define BIT_SDIO_DET_ON                 BIT2
#define BIT_SDIO_DET_INT_SRC            BIT3

/* FCIE_RSP_SHIFT_CNT 0x12 */
#define BIT_RSP_SHIFT_TUNE_MASK         (BIT4 - 1)
#define BIT_RSP_SHIFT_SEL               BIT4            /*SW or HW by default  0*/

/* FCIE_RX_SHIFT_CNT 0x13 */
#define BIT_RSTOP_SHIFT_TUNE_MASK       (BIT4 - 1)
#define BIT_RSTOP_SHIFT_SEL             BIT4
#define BIT_WRSTS_SHIFT_TUNE_MASK       (BIT8|BIT9|BIT10|BIT11)
#define BIT_WRSTS_SHIFT_SEL             BIT12

/* FCIE_ZDEC_CTL0 0x14 */
#define BIT_ZDEC_EN                     BIT0
#define BIT_SD2ZDEC_PTR_CLR             BIT1

/* FCIE_TEST_MODE 0x15 */
#define BIT_SDDR1                       BIT0
#define BIT_DEBUG_MODE_MASK             (BIT3|BIT2|BIT1)
#define BIT_DEBUG_MODE_SHIFT            1
#define BIT_BIST_MODE                   BIT4
//#define BIT_DS_TESTEN                 BIT1
//#define BIT_TEST_MODE                 BIT2
//#define BIT_DEBUG_MODE_MASK           BIT3|BIT4|BIT5
//#define BIT_DEBUG_MODE_SHIFT          3
//#define BIT_TEST_MIU                  BIT6
//#define BIT_TEST_MIE                  BIT7
//#define BIT_TEST_MIU_STS              BIT8
//#define BIT_TEST_MIE_STS              BIT9
//#define BIT_BIST_MODE                 BIT10

/* FCIE_WR_SBIT_TIMER 0x17 */
#define BIT_WR_SBIT_TIMER_MASK          (BIT15-1)
#define BIT_WR_SBIT_TIMER_EN            BIT15

/* FCIE_RD_SBIT_TIMER 0x18 */
#define BIT_RD_SBIT_TIMER_MASK          (BIT15-1)
#define BIT_RD_SBIT_TIMER_EN            BIT15


/* NC_CIFD_EVENT 0x30 */
#define BIT_WBUF_FULL                   BIT0
#define BIT_WBUF_EMPTY_TRI              BIT1
#define BIT_RBUF_FULL_TRI               BIT2
#define BIT_RBUF_EMPTY                  BIT3

/* NC_CIFD_INT_EN 0x31 */
#define BIT_WBUF_FULL_INT_EN            BIT0
#define BIT_RBUF_EMPTY_INT_EN           BIT1
#define BIT_F_WBUF_FULL_INT             BIT2
#define BIT_F_RBUF_EMPTY_INT            BIT3

/* FCIE_PWR_SAVE_CTL 0x35 */
#define BIT_POWER_SAVE_MODE             BIT0
#define BIT_SD_POWER_SAVE_RIU           BIT1
#define BIT_POWER_SAVE_MODE_INT_EN      BIT2
#define BIT_SD_POWER_SAVE_RST           BIT3
#define BIT_POWER_SAVE_INT_FORCE        BIT4
#define BIT_RIU_SAVE_EVENT              BIT5
#define BIT_RST_SAVE_EVENT              BIT6
#define BIT_BAT_SAVE_EVENT              BIT7
#define BIT_BAT_SD_POWER_SAVE_MASK      BIT8
#define BIT_RST_SD_POWER_SAVE_MASK      BIT9
#define BIT_POWER_SAVE_MODE_INT         BIT15

/* FCIE_BOOT 0x37 */
#define BIT_NAND_BOOT_EN                BIT0
#define BIT_BOOTSRAM_ACCESS_SEL         BIT1

/* FCIE_BOOT 0x39 */
#define BIT_DEBUG_MODE_MSK              (BIT11|BIT10|BIT9|BIT8)
#define BIT_DEBUG_MODE_SET              (BIT10|BIT8)

/* FCIE_RESET 0x3F */

#define BIT_FCIE_SOFT_RST_n             BIT0
#define BIT_RST_MIU_STS                 BIT1
#define BIT_RST_MIE_STS                 BIT2
#define BIT_RST_MCU_STS                 BIT3
#define BIT_RST_ECC_STS                 BIT4
//#define BIT_RST_STS_MASK                (BIT_RST_MIU_STS | BIT_RST_MIE_STS | BIT_RST_MCU_STS | BIT_RST_ECC_STS)
#define BIT_RST_STS_MASK                (BIT_RST_MIU_STS | BIT_RST_MIE_STS | BIT_RST_MCU_STS)
#define BIT_NC_DEB_SEL_SHIFT            12
#define BIT_NC_DEB_SEL_MASK             (BIT15|BIT14|BIT13|BIT12)

//------------------------------------------------------------------
/*
 * Power Save FIFO Cmd*
 */
#define PWR_BAT_CLASS    (0x1 << 13)  /* Battery lost class */
#define PWR_RST_CLASS    (0x1 << 12)  /* Reset Class */

/* Command Type */
#define PWR_CMD_WREG     (0x0 << 9)   /* Write data */
#define PWR_CMD_RDCP     (0x1 << 9)   /* Read and cmp data. If mismatch, HW retry */
#define PWR_CMD_WAIT     (0x2 << 9)   /* Wait idle, max. 128T */
#define PWR_CMD_WINT     (0x3 << 9)   /* Wait interrupt */
#define PWR_CMD_STOP     (0x7 << 9)   /* Stop */

/* RIU Bank */
#define PWR_CMD_BK0      (0x0 << 7)
#define PWR_CMD_BK1      (0x1 << 7)
#define PWR_CMD_BK2      (0x2 << 7)
#define PWR_CMD_BK3      (0x3 << 7)

#define PWR_RIU_ADDR     (0x0 << 0)

#endif /* __eMMC_FCIE_V5_REG_H__ */
