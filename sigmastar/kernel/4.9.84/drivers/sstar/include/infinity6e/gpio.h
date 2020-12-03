/*
* gpio.h- Sigmastar
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
#ifndef ___GPIO_H
#define ___GPIO_H

#define PAD_PM_UART_RX1                 0
#define PAD_PM_UART_TX1                 1
#define PAD_PM_UART_RX                  2
#define PAD_PM_UART_TX                  3
#define PAD_PM_I2CM_SCL                 4
#define PAD_PM_I2CM_SDA                 5
#define PAD_PM_GPIO0                    6
#define PAD_PM_GPIO1                    7
#define PAD_PM_GPIO2                    8
#define PAD_PM_GPIO3                    9
#define PAD_PM_GPIO4                    10
#define PAD_PM_GPIO5                    11
#define PAD_PM_GPIO6                    12
#define PAD_PM_GPIO7                    13
#define PAD_PM_GPIO8                    14
#define PAD_PM_GPIO9                    15
#define PAD_PM_GPIO10                   16
#define PAD_PM_SPI_CZ                   17
#define PAD_PM_SPI_CK                   18
#define PAD_PM_SPI_DI                   19
#define PAD_PM_SPI_DO                   20
#define PAD_PM_SPI_WPZ                  21
#define PAD_PM_SPI_HLD                  22
#define PAD_SAR_GPIO0                   23
#define PAD_SAR_GPIO1                   24
#define PAD_SAR_GPIO2                   25
#define PAD_SAR_GPIO3                   26
#define PAD_SAR_GPIO4                   27
#define PAD_SAR_GPIO5                   28
#define PAD_SD0_GPIO0                   29
#define PAD_SD0_CDZ                     30
#define PAD_SD0_D1                      31
#define PAD_SD0_D0                      32
#define PAD_SD0_CLK                     33
#define PAD_SD0_CMD                     34
#define PAD_SD0_D3                      35
#define PAD_SD0_D2                      36
#define PAD_I2S0_MCLK                   37
#define PAD_I2S0_BCK                    38
#define PAD_I2S0_WCK                    39
#define PAD_I2S0_DI                     40
#define PAD_I2S0_DO                     41
#define PAD_I2C0_SCL                    42
#define PAD_I2C0_SDA                    43
#define PAD_ETH_LED0                    44
#define PAD_ETH_LED1                    45
#define PAD_FUART_RX                    46
#define PAD_FUART_TX                    47
#define PAD_FUART_CTS                   48
#define PAD_FUART_RTS                   49
#define PAD_SD1_CDZ                     50
#define PAD_SD1_D1                      51
#define PAD_SD1_D0                      52
#define PAD_SD1_CLK                     53
#define PAD_SD1_CMD                     54
#define PAD_SD1_D3                      55
#define PAD_SD1_D2                      56
#define PAD_SD1_GPIO0                   57
#define PAD_SD1_GPIO1                   58
#define PAD_GPIO0                       59
#define PAD_GPIO1                       60
#define PAD_GPIO2                       61
#define PAD_GPIO3                       62
#define PAD_GPIO4                       63
#define PAD_GPIO5                       64
#define PAD_GPIO6                       65
#define PAD_GPIO7                       66
#define PAD_SR0_IO00                    67
#define PAD_SR0_IO01                    68
#define PAD_SR0_IO02                    69
#define PAD_SR0_IO03                    70
#define PAD_SR0_IO04                    71
#define PAD_SR0_IO05                    72
#define PAD_SR0_IO06                    73
#define PAD_SR0_IO07                    74
#define PAD_SR0_IO08                    75
#define PAD_SR0_IO09                    76
#define PAD_SR0_IO10                    77
#define PAD_SR0_IO11                    78
#define PAD_SR0_IO12                    79
#define PAD_SR0_IO13                    80
#define PAD_SR0_IO14                    81
#define PAD_SR0_IO15                    82
#define PAD_SR0_IO16                    83
#define PAD_SR0_IO17                    84
#define PAD_SR0_IO18                    85
#define PAD_SR0_IO19                    86
#define PAD_SR1_IO00                    87
#define PAD_SR1_IO01                    88
#define PAD_SR1_IO02                    89
#define PAD_SR1_IO03                    90
#define PAD_SR1_IO04                    91
#define PAD_SR1_IO05                    92
#define PAD_SR1_IO06                    93
#define PAD_SR1_IO07                    94
#define PAD_SR1_IO08                    95
#define PAD_SR1_IO09                    96
#define PAD_SR1_IO10                    97
#define PAD_SR1_IO11                    98
#define PAD_SR1_IO12                    99
#define PAD_SR1_IO13                    100
#define PAD_SR1_IO14                    101
#define PAD_SR1_IO15                    102
#define PAD_SR1_IO16                    103
#define PAD_SR1_IO17                    104
#define PAD_SR1_IO18                    105
#define PAD_SR1_IO19                    106
#define PAD_GPIO8                       107
#define PAD_GPIO9                       108
#define PAD_GPIO10                      109
#define PAD_GPIO11                      110
#define PAD_GPIO12                      111
#define PAD_GPIO13                      112
#define PAD_GPIO14                      113
#define PAD_GPIO15                      114
#define PAD_SPI_CZ                      115
#define PAD_SPI_CK                      116
#define PAD_SPI_DI                      117
#define PAD_SPI_DO                      118
#define PAD_SPI_WPZ                     119
#define PAD_SPI_HLD                     120
#define PAD_ETH_RN                      121
#define PAD_ETH_RP                      122
#define PAD_ETH_TN                      123
#define PAD_ETH_TP                      124
#define PAD_USB2_DM                     125
#define PAD_USB2_DP                     126

#define GPIO_NR                         127
#define PAD_UNKNOWN     0xFFFF

#endif // #ifndef ___GPIO_H
