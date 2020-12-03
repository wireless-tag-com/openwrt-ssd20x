/*
* mdrv_spi.h- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/
//-----------------------------------------------------------------------------
//
// Copyright (c) 2008 MStar Semiconductor, Inc.  All rights reserved.
//
//-----------------------------------------------------------------------------
// FILE
//      mdrv_spi.h
//
// DESCRIPTION
//
//
// HISTORY
//
//-----------------------------------------------------------------------------

#ifndef __MDRV_SPI_H
#define __MDRV_SPI_H

//#include "ms_types.h"

#define DRV_NAME	"spi"
#define MSTAR_SPI_TIMEOUT_MS	       30000
#define MSPI_CFG_TIME_MAX              0xFF

#define MS_IO_OFFSET 0xDE000000
/* macro to get at MMIO space when running virtually */
#define IO_ADDRESS(x)           ( (u32)(x) + MS_IO_OFFSET )
//bank spi
#define REG_MSPI_WRITE_BUF             0x40
    #define MSPI_MAX_RW_BUF_SIZE       0x8

#define REG_MSPI_READ_BUF              0x44

#define REG_MSPI_RW_BUF_SIZE           0x48
    #define MSPI_RW_BUF_MASK           0xFF
    #define MSPI_READ_BUF_SHIFT        8

#define REG_MSPI_CTRL                  0x49
    #define MSPI_ENABLE_BIT            0x1
	#define MSPI_RESET_BIT             0x2
	#define MSPI_INT_EN_BIT            0x4
    #define MSPI_MODE_SHIFT            6
	#define MSPI_MODE_MASK             (3 << 6)
	#define MSPI_CLK_DIV_SHIFT         8
	#define MSPI_CLK_DIV_MASK          0xFF

#define REG_MSPI_TR_TIME               0x4A
#define REG_MSPI_TB_TIME               0x4B
    #define MSPI_TIME_MASK             0xFF
	#define MSPI_TIME_SHIFT            8

#define REG_MSPI_WRITE_FRAME_SIZE      0x4C
#define REG_MSPI_READ_FRAME_SIZE       0x4E
    #define MSPI_FRAME_SIZE_MASK       0x07
	#define MSPI_MAX_FRAME_BUF_NUM     8
	#define MSPI_FRAME_FIELD_BITS      3

#define REG_MSPI_LSB_FIRST             0x50

#define REG_MSPI_TRIGGER               0x5A
    #define MSPI_TRIGGER               0x01

#define REG_MSPI_DONE_FLAG             0x5B

#define REG_MSPI_CLEAR_DONE_FLAG       0x5C
    #define MSPI_CLEAR_DONE            0x01

#define REG_MSPI_CHIP_SELECT           0x5F


//bank clock gen0
#define REG_MSPI_CKGEN                 0x16
    #define MSPI_CKG_MSPI0_SHIFT       10
	#define MSPI_CKG_MSPI0_MASK        (0x0F << 8)
    #define MSPI_CKG_MSPI1_SHIFT       14
	#define MSPI_CKG_MSPI1_MASK        (0x0F << 12)

//bank chiptop
#define REG_MSPI0_MODE                 0x08
    #define MSPI_MSPI0_MODE_SHIFT      0
	#define MSPI_MSPI0_MODE_MASK       3

#define REG_MSPI1_MODE                 0x08
    #define MSPI_MSPI1_MODE_SHIFT      6
    #define MSPI_MSPI1_MODE_MASK	   (3 << 6)

typedef enum {
     E_MSPI_OK = 0
	,E_MSPI_PARAM_ERR = 1
    ,E_MSPI_INIT_ERR  = 2
    ,E_MSPI_READ_ERR  = 4
    ,E_MSPI_WRITE_ERR = 8
    ,E_MSPI_NULL
} MSPI_ERR_NO;

typedef enum
{
    E_MSPI0,
    E_MSPI1,
    E_MSPI_MAX,
}MSPI_CH;

typedef enum
{
    E_MSPI_READ,
    E_MSPI_WRITE
}MSPI_RW_DIR;

typedef enum
{
    E_MSPI_POL,
    E_MSPI_PHA,
    E_MSPI_CLK
}MSPI_CFG;

typedef enum {
    E_MSPI_MODE0, //CPOL = 0,CPHA =0
    E_MSPI_MODE1, //CPOL = 0,CPHA =1
    E_MSPI_MODE2, //CPOL = 1,CPHA =0
    E_MSPI_MODE3, //CPOL = 1,CPHA =1
    E_MSPI_MODE_MAX,
} MSPI_MODE;

typedef enum
{
    E_MSPI_TR_START_TIME,
    E_MSPI_TR_END_TIME,
    E_MSPI_TB_INTERVAL_TIME,
    E_MSPI_RW_TURN_AROUND_TIME
}MSPI_TIME_CFG;

typedef struct
{
    u8  spi_clk;
    u8  spi_div;
    u32 spi_speed;
}mspi_clk_cfg;

#define MSTAR_SPI_MODE_BITS	(SPI_CPOL | SPI_CPHA)

#define READ_WORD(baseaddr, bank)         (*(volatile u16*)(baseaddr + ((bank)<<2)))
#define WRITE_WORD(baseaddr, bank, val)   (*((volatile u16*)(baseaddr + ((bank)<<2)))) = (u16)(val)

#endif //__MDRV_SPI_H
