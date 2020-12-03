/*
* chiptop_reg.h- Sigmastar
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
#ifndef __CHIPTOP_REG_H__
#define __CHIPTOP_REG_H__

#define CHIPTOP_BANK                        0x101E00

#define REG_FUART_MODE                  0x03
    #define REG_FUART_MODE_MASK             BIT0|BIT1|BIT2
#define REG_UART0_MODE                  0x03
    #define REG_UART0_MODE_MASK             BIT4|BIT5|BIT6
#define REG_UART1_MODE                  0x03
    #define REG_UART1_MODE_MASK             BIT8|BIT9|BIT10
#define REG_UART2_MODE                  0x03
    #define REG_UART2_MODE_MASK             BIT12|BIT13|BIT14
#define REG_PWM0_MODE                   0x07
    #define REG_PWM0_MODE_MASK              BIT0|BIT1|BIT2
#define REG_PWM1_MODE                   0x07
    #define REG_PWM1_MODE_MASK              BIT3|BIT4|BIT5
#define REG_PWM2_MODE                   0x07
    #define REG_PWM2_MODE_MASK              BIT6|BIT7|BIT8
#define REG_PWM3_MODE                   0x07
    #define REG_PWM3_MODE_MASK              BIT9|BIT10|BIT11
#define REG_SDIO_MODE                   0x08
    #define REG_SDIO_MODE_MASK              BIT8|BIT9
#define REG_I2C0_MODE                   0x09
    #define REG_I2C0_MODE_MASK              BIT0|BIT1|BIT2
#define REG_I2C1_MODE                   0x09
    #define REG_I2C1_MODE_MASK              BIT4|BIT5|BIT6
#define REG_IDAC_MODE                   0x0a
    #define REG_IDAC_MODE_MASK              BIT0
#define REG_PM_SPICZ2_MODE              0x0a
    #define REG_PM_SPICZ2_MODE_MASK         BIT4|BIT5
#define REG_SATA_LED_MODE               0x0b
    #define REG_SATA_LED_MODE_MASK          BIT0
#define REG_SPI0_MODE                   0x0c
    #define REG_SPI0_MODE_MASK              BIT0|BIT1|BIT2
#define REG_BT1120_MODE                 0x0d
    #define REG_BT1120_MODE_MASK            BIT0|BIT1
#define REG_TTL_MODE                    0x0d
    #define REG_TTL_MODE_MASK               BIT8|BIT9|BIT10|BIT11
#define REG_TX_MIPI_MODE                0x0d
    #define REG_TX_MIPI_MODE_MASK           BIT12|BIT13
#define REG_ETH0_MODE                   0x0e
    #define REG_ETH0_MODE_MASK              BIT0
#define REG_ETH1_MODE                   0x0e
    #define REG_ETH1_MODE_MASK              BIT8|BIT9|BIT10|BIT11
#define REG_EJ_MODE                     0x0f
    #define REG_EJ_MODE_MASK                BIT0|BIT1
#define REG_DMIC_MODE                   0x0f
    #define REG_DMIC_MODE_MASK              BIT8|BIT9|BIT10
#define REG_I2S_MODE                    0x0f
    #define REG_I2S_MODE_MASK               BIT12|BIT13
#define REG_TEST_IN_MODE                0x12
    #define REG_TEST_IN_MODE_MASK           BIT0|BIT1
#define REG_TEST_OUT_MODE               0x12
    #define REG_TEST_OUT_MODE_MASK          BIT4|BIT5

#endif // __CHIPTOP_REG_H__
