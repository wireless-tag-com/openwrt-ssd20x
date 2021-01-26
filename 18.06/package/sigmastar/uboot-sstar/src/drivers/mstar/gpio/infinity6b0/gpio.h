/*
* gpio.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: giggshuang <giggshuang@sigmastar.com.tw>
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

#ifndef __GPIO_H__
#define __GPIO_H__

#define PAD_GPIO0                       0
#define PAD_GPIO1                       1
#define PAD_GPIO2                       2
#define PAD_GPIO3                       3
#define PAD_GPIO4                       4
#define PAD_GPIO5                       5
#define PAD_GPIO6                       6
#define PAD_GPIO7                       7
#define PAD_GPIO8                       8
#define PAD_GPIO9                       9
#define PAD_GPIO12                      10
#define PAD_GPIO13                      11
#define PAD_GPIO14                      12
#define PAD_GPIO15                      13
#define PAD_FUART_RX                    14
#define PAD_FUART_TX                    15
#define PAD_FUART_CTS                   16
#define PAD_FUART_RTS                   17
#define PAD_I2C0_SCL                    18
#define PAD_I2C0_SDA                    19
#define PAD_I2C1_SCL                    20
#define PAD_I2C1_SDA                    21
#define PAD_SR_IO00                     22
#define PAD_SR_IO01                     23
#define PAD_SR_IO02                     24
#define PAD_SR_IO03                     25
#define PAD_SR_IO04                     26
#define PAD_SR_IO05                     27
#define PAD_SR_IO06                     28
#define PAD_SR_IO07                     29
#define PAD_SR_IO08                     30
#define PAD_SR_IO09                     31
#define PAD_SR_IO10                     32
#define PAD_SR_IO11                     33
#define PAD_SR_IO12                     34
#define PAD_SR_IO13                     35
#define PAD_SR_IO14                     36
#define PAD_SR_IO15                     37
#define PAD_SR_IO16                     38
#define PAD_SR_IO17                     39
#define PAD_UART0_RX                    40
#define PAD_UART0_TX                    41
#define PAD_UART1_RX                    42
#define PAD_UART1_TX                    43
#define PAD_SPI0_CZ                     44
#define PAD_SPI0_CK                     45
#define PAD_SPI0_DI                     46
#define PAD_SPI0_DO                     47
#define PAD_SPI1_CZ                     48
#define PAD_SPI1_CK                     49
#define PAD_SPI1_DI                     50
#define PAD_SPI1_DO                     51
#define PAD_PWM0                        52
#define PAD_PWM1                        53
#define PAD_SD_CLK                      54
#define PAD_SD_CMD                      55
#define PAD_SD_D0                       56
#define PAD_SD_D1                       57
#define PAD_SD_D2                       58
#define PAD_SD_D3                       59
#define PAD_PM_SD_CDZ                   60
#define PAD_PM_IRIN                     61
#define PAD_PM_GPIO0                    62
#define PAD_PM_GPIO1                    63
#define PAD_PM_GPIO2                    64
#define PAD_PM_GPIO3                    65
#define PAD_PM_GPIO4                    66
#define PAD_PM_GPIO7                    67
#define PAD_PM_GPIO8                    68
#define PAD_PM_GPIO9                    69
#define PAD_PM_SPI_CZ                   70
#define PAD_PM_SPI_CK                   71
#define PAD_PM_SPI_DI                   72
#define PAD_PM_SPI_DO                   73
#define PAD_PM_SPI_WPZ                  74
#define PAD_PM_SPI_HLD                  75
#define PAD_PM_LED0                     76
#define PAD_PM_LED1                     77
#define PAD_SAR_GPIO0                   78
#define PAD_SAR_GPIO1                   79
#define PAD_SAR_GPIO2                   80
#define PAD_SAR_GPIO3                   81
#define PAD_ETH_RN                      82
#define PAD_ETH_RP                      83
#define PAD_ETH_TN                      84
#define PAD_ETH_TP                      85
#define PAD_USB_DM                      86
#define PAD_USB_DP                      87
#define PAD_SD1_IO0                     88
#define PAD_SD1_IO1                     89
#define PAD_SD1_IO2                     90
#define PAD_SD1_IO3                     91
#define PAD_SD1_IO4                     92
#define PAD_SD1_IO5                     93
#define PAD_SD1_IO6                     94
#define PAD_SD1_IO7                     95
#define PAD_SD1_IO8                     96

#define GPIO_NR                         97
#define PAD_UNKNOWN     0xFFFF

#endif  // __GPIO_H__
