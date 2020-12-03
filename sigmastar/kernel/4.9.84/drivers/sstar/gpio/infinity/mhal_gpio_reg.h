/*
* mhal_gpio_reg.h- Sigmastar
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
#ifndef _REG_GPIO_H_
#define _REG_GPIO_H_
//-------------------------------------------------------------------------------------------------
//  Macro and Define
//-------------------------------------------------------------------------------------------------

////8 bit define
//CHIPTOP
#define REG_FUART_MODE     0x06
#define REG_UART0_MODE     0x06
#define REG_UART1_MODE     0x07
#define REG_SR_MODE        0x0C
#define REG_SR_I2C_MODE    0x0C
#define REG_PWM0_MODE      0x0E
#define REG_PWM1_MODE      0x0E
#define REG_PWM2_MODE      0x0E
#define REG_PWM3_MODE      0x0E
#define REG_NAND_MODE      0x10
#define REG_SD_MODE        0x10
#define REG_SDIO_MODE      0x11
#define REG_I2C0_MODE      0x12
#define REG_I2C1_MODE      0x12
#define REG_SPI0_MODE      0x18
#define REG_SPI1_MODE      0x18
#define REG_EJ_MODE        0x1E
#define REG_ETH_MODE       0x1E
#define REG_CCIR_MODE      0x1E
#define REG_TTL_MODE       0x1E
#define REG_I2S_MODE       0x1F
#define REG_DMIC_MODE      0x1F
#define REG_TEST_IN_MODE   0x24
#define REG_TEST_OUT_MODE  0x24
#define REG_ALL_PAD_IN     0xA1

//PMSLEEP
#define REG_PMLOCK_L_MODE  0x24
#define REG_PMLOCK_H_MODE  0x25
#define REG_IRIN_MODE      0x38

//SAR
#define REG_SAR_MODE       0x22

//-------------------------------------------------------------------------------------------------
//  Type and Structure
//-------------------------------------------------------------------------------------------------

#endif // _REG_GPIO_H_

