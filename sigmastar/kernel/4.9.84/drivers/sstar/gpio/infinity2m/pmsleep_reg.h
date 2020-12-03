/*
* pmsleep_reg.h- Sigmastar
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
#ifndef __PMSLEEP_REG_H__
#define __PMSLEEP_REG_H__

#define PMSLEEP_BANK                        0x000E00

/* PM PAD : PMSLEEP_BANK */
#define REG_GPIO_PM_LOCK        0x12
    #define REG_GPIO_PM_LOCK_MASK   0xFFFF
#define REG_PM_GPIO_PM4_INV     0x1c
    #define REG_PM_GPIO_PM4_INV_MASK BIT1
#define REG_PM_LINK_WKINT2GPIO4 0x1c
    #define REG_PM_LINK_WKINT2GPIO4_MASK BIT3

#define REG_PM_IR_IS_GPIO       0x1c
    #define REG_PM_IR_IS_GPIO_MASK  BIT4
#define REG_PM_IRIN_MODE        REG_PM_IR_IS_GPIO

#define REG_PM_PWM0_MODE        0x28
    #define REG_PM_PWM0_MODE_MASK   BIT1|BIT0
#define REG_PM_PWM1_MODE        0x28
    #define REG_PM_PWM1_MODE_MASK   BIT3|BIT2
#define REG_PM_PWM2_MODE        0x28
    #define REG_PM_PWM2_MODE_MASK   BIT7|BIT6
#define REG_PM_PWM3_MODE        0x28
    #define REG_PM_PWM3_MODE_MASK   BIT9|BIT8
#define REG_PM_PWM4_MODE        0x27
    #define REG_PM_PWM4_MODE_MASK   BIT0
#define REG_PM_PWM5_MODE        0x27
    #define REG_PM_PWM5_MODE_MASK   BIT1
#define REG_PM_PWM8_MODE        0x27
    #define REG_PM_PWM8_MODE_MASK   BIT2
#define REG_PM_PWM9_MODE        0x27
    #define REG_PM_PWM9_MODE_MASK   BIT3
#define REG_PM_PWM10_MODE        0x27
    #define REG_PM_PWM10_MODE_MASK   BIT4
#define REG_PM_UART1_MODE        0x27
    #define REG_PM_UART1_MODE_MASK   BIT8
#define REG_PM_LED_MODE         0x28
    #define REG_PM_LED_MODE_MASK    BIT5|BIT4

#define REG_PM_VID_MODE         0x28
    #define REG_PM_VID_MODE_MASK    BIT13|BIT12
#define REG_PM_SD_CDZ_MODE      0x28
    #define REG_PM_SD_CDZ_MODE_MASK BIT14

#define REG_PM_SPI_IS_GPIO      0x35
    #define REG_PM_SPI_IS_GPIO_MASK BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
    #define REG_PM_SPI_GPIO_MASK            BIT0
    #define REG_PM_SPICSZ1_GPIO_MASK        BIT2
    #define REG_PM_SPICSZ2_GPIO_MASK        BIT3
    #define REG_PM_SPIWPN_GPIO_MASK         BIT4
    #define REG_PM_SPIHOLDN_MODE_MASK       BIT6 | BIT7
#define REG_PM_SPICSZ1_GPIO     REG_PM_SPI_IS_GPIO
#define REG_PM_SPICSZ2_GPIO     REG_PM_SPI_IS_GPIO
#define REG_PM_SPI_GPIO         REG_PM_SPI_IS_GPIO
#define REG_PM_SPIWPN_GPIO      REG_PM_SPI_IS_GPIO
#define REG_PM_SPIHOLDN_MODE    REG_PM_SPI_IS_GPIO

#define REG_PM_UART_IS_GPIO     0x35
    #define REG_PM_UART_IS_GPIO_MASK        BIT11|BIT10|BIT9|BIT8

#define REG_PM_SPICSZ2_MODE     0x36
    #define REG_PM_SPICSZ2_MODE_MASK        BIT12

#endif // __PMSLEEP_REG_H__
