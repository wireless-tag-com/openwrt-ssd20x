/*
* padmux.h- Sigmastar
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
#ifndef ___PADMUX_H
#define ___PADMUX_H

#define PINMUX_FOR_GPIO_MODE        0x00
#define PINMUX_FOR_FUART_MODE       0x01
#define PINMUX_FOR_UART0_MODE       0x02
#define PINMUX_FOR_UART1_MODE       0x03
#define PINMUX_FOR_PWM0_MODE        0x04
#define PINMUX_FOR_PWM1_MODE        0x05
#define PINMUX_FOR_PWM2_MODE        0x06
#define PINMUX_FOR_PWM3_MODE        0x07
#define PINMUX_FOR_PWM4_MODE        0x08
#define PINMUX_FOR_PWM5_MODE        0x09
#define PINMUX_FOR_PWM6_MODE        0x0a
#define PINMUX_FOR_PWM7_MODE        0x0b
#define PINMUX_FOR_SR_MODE          0x0c
#define PINMUX_FOR_NAND_MODE        0x0d
#define PINMUX_FOR_SD_MODE          0x0e
#define PINMUX_FOR_SDIO_MODE        0x0f
#define PINMUX_FOR_I2C0_MODE        0x10
#define PINMUX_FOR_I2C1_MODE        0x11
#define PINMUX_FOR_SPI0_MODE        0x12
#define PINMUX_FOR_SPI1_MODE        0x13
#define PINMUX_FOR_EJ_MODE          0x14
#define PINMUX_FOR_ETH_MODE         0x15
#define PINMUX_FOR_TTL_MODE         0x16
#define PINMUX_FOR_DMIC_MODE        0x17
#define PINMUX_FOR_I2S_MODE         0x18
#define PINMUX_FOR_I2S_RX_MODE      0x19
#define PINMUX_FOR_I2S_TX_MODE      0x1a
#define PINMUX_FOR_I2S_MCK_MODE     0x1b
#define PINMUX_FOR_TESTIN_MODE      0x1c
#define PINMUX_FOR_TESTOUT_MODE     0x1d
#define PINMUX_FOR_EMMC_MODE        0x1e
#define PINMUX_FOR_EMMC_RSTN_EN     0x1f
#define PINMUX_FOR_SR0_BT656_MODE   0x20
#define PINMUX_FOR_SR0_MIPI_MODE    0x21
#define PINMUX_FOR_SR0_PAR_MODE     0x22
#define PINMUX_FOR_I2C2_MODE        0x23
#define PINMUX_FOR_I2C3_MODE        0x24
#define PINMUX_FOR_SR1_BT656_MODE   0x25
#define PINMUX_FOR_SR1_MIPI_MODE    0x26
#define PINMUX_FOR_SR1_PAR_MODE     0x27

#define PINMUX_FOR_PM_PM_IR_IS_GPIO 0x28
#define PINMUX_FOR_PM_PWM0_MODE     0x29
#define PINMUX_FOR_PM_PWM1_MODE     0x2a
#define PINMUX_FOR_PM_LED_MODE      0x2b
#define PINMUX_FOR_PM_PWM2_MODE     0x2c
#define PINMUX_FOR_PM_PWM3_MODE     0x2d
#define PINMUX_FOR_PM_VID_MODE      0x2e
#define PINMUX_FOR_PM_SD_CDZ_MODE   0x2f
#define PINMUX_FOR_PM_SPI_IS_GPIO   0x30
#define PINMUX_FOR_PM_UART_IS_GPIO  0x31

#define PINMUX_FOR_UNKNOWN_MODE     0xFF

//==============================================================================
//
//                              FUNCTION PROTOTYPE
//
//==============================================================================

S32 HalPadSetVal(U32 u32PadID, U32 u32Mode);

#endif // ___PADMUX_H
