/*
* mdrv_spinand_command.h- Sigmastar
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
#ifndef _DRV_SPINAND_COMMAND_H_
#define _DRV_SPINAND_COMMAND_H_

#undef PAGE_SIZE

typedef enum
{
    E_SPINAND_DBGLV_NONE,    //disable all the debug message
    E_SPINAND_DBGLV_INFO,    //information
    E_SPINAND_DBGLV_NOTICE,  //normal but significant condition
    E_SPINAND_DBGLV_WARNING, //warning conditions
    E_SPINAND_DBGLV_ERR,     //error conditions
    E_SPINAND_DBGLV_CRIT,    //critical conditions
    E_SPINAND_ALERT,   //action must be taken immediately
    E_SPINAND_DBGLV_EMERG,   //system is unusable
    E_SPINAND_DBGLV_DEBUG,   //debug-level messages
} SPINAND_DbgLv;

#define ID1              _gtSpinandInfo.au8_ID[0]
#define ID2              _gtSpinandInfo.au8_ID[1]
#define ID3              _gtSpinandInfo.au8_ID[2]
#define SPARE_SIZE       _gtSpinandInfo.u16_SpareByteCnt
#define BLOCK_PAGE_SIZE  _gtSpinandInfo.u16_BlkPageCnt
#define SECTOR_SIZE      _gtSpinandInfo.u16_SectorByteCnt
#define PAGE_SIZE        _gtSpinandInfo.u16_PageByteCnt
#define PLANE            _gtSpinandInfo.u8PlaneCnt
#define CLKCFG           _gtSpinandInfo.u8CLKConfig
#define READ_MODE        _gtSpinandInfo.u8ReadMode
#define WRITE_MODE       _gtSpinandInfo.u8WriteMode
#define BLOCKCNT         _gtSpinandInfo.u16_BlkCnt
#define DENSITY_2G   2048
// SPI NAND COMMAND                            //opcode
#define SPI_NAND_CMD_PGRD                        0x13  //page read
#define SPI_NAND_CMD_RDID                        0x9F  //read ID
#define SPI_NAND_CMD_WREN                        0x06  //write enable
#define SPI_NAND_CMD_WRDIS                       0x04  //write disable
#define SPI_NAND_CMD_RFC                         0x03  //read from cache
#define SPI_NAND_CMD_PP                          0x02  //program load x2 (load program data-2kb MAX)
#define SPI_NAND_CMD_QPP                         0x32  //program load x4 (load program data-2kb MAX)
#define SPI_NAND_CMD_RPL                         0x84  //program load random data (enter cache address / data)
#define SPI_NAND_CMD_PE                          0x10  //program execute(enter block/page address, no data, execute)
#define SPI_NAND_CMD_GF                          0x0F  //get feature
#define SPI_NAND_CMD_RESET                       0xFF  //reset the device
#define SPI_NAND_CMD_SF                          0x1F  //set features
#define SPI_NAND_CMD_BE                          0xD8  //block erase
#define SPI_NAND_CMD_CE                          0xC7  //??
#define SPI_NAND_CMD_DIESELECT                   0xC2
#define SPI_NAND_REG_PROT                0xA0   //Block lock
#define SPI_NAND_REG_FEAT                0xB0   //OTP
#define QUAD_ENABLE              1
#define SPI_NAND_REG_STAT                0xC0   //Status
                #define E_FAIL                   (0x01 << 2)
                #define P_FAIL                   (0x01 << 3)
                #define ECC_STATUS_PASS                     (0x00 << 4)
                #define ECC_1_3_CORRECTED                (0x01 << 4)
                #define ECC_NOT_CORRECTED               (0x02 << 4)
                #define ECC_4_6_CORRECTED                (0x03 << 4)
                #define ECC_7_8_CORRECTED                (0x05 << 4)
                #define SPI_NAND_STAT_OIP              (0x1)
#define SPI_NAND_REG_FUT                 0xD0

#define SPI_NAND_ADDR_LEN                        3
#define SPI_NAND_PAGE_ADDR_LEN                   2
#define SPI_NAND_PLANE_OFFSET                    6
#define SPI_PLANE_ADDR_MASK                      0x40

#endif
