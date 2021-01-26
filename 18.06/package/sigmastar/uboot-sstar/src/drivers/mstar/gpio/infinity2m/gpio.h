/*
* gpio.h- Sigmastar
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
#ifndef ___GPIO_H
#define ___GPIO_H

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
#define PAD_GPIO10                      10
#define PAD_GPIO11                      11
#define PAD_GPIO12                      12
#define PAD_GPIO13                      13
#define PAD_GPIO14                      14
#define PAD_FUART_RX                    15
#define PAD_FUART_TX                    16
#define PAD_FUART_CTS                   17
#define PAD_FUART_RTS                   18
#define PAD_TTL0                        19
#define PAD_TTL1                        20
#define PAD_TTL2                        21
#define PAD_TTL3                        22
#define PAD_TTL4                        23
#define PAD_TTL5                        24
#define PAD_TTL6                        25
#define PAD_TTL7                        26
#define PAD_TTL8                        27
#define PAD_TTL9                        28
#define PAD_TTL10                       29
#define PAD_TTL11                       30
#define PAD_TTL12                       31
#define PAD_TTL13                       32
#define PAD_TTL14                       33
#define PAD_TTL15                       34
#define PAD_TTL16                       35
#define PAD_TTL17                       36
#define PAD_TTL18                       37
#define PAD_TTL19                       38
#define PAD_TTL20                       39
#define PAD_TTL21                       40
#define PAD_TTL22                       41
#define PAD_TTL23                       42
#define PAD_TTL24                       43
#define PAD_TTL25                       44
#define PAD_TTL26                       45
#define PAD_TTL27                       46
#define PAD_UART0_RX                    47
#define PAD_UART0_TX                    48
#define PAD_UART1_RX                    49
#define PAD_UART1_TX                    50
#define PAD_SD_CLK                      51
#define PAD_SD_CMD                      52
#define PAD_SD_D0                       53
#define PAD_SD_D1                       54
#define PAD_SD_D2                       55
#define PAD_SD_D3                       56
#define PAD_SD_GPIO                     57
#define PAD_PM_SD_CDZ                   58
#define PAD_PM_IRIN                     59
#define PADA_IDAC_OUT_B                 60
#define PADA_IDAC_OUT_G                 61
#define PADA_IDAC_OUT_R                 62
#define PAD_PM_SPI_CZ                   63
#define PAD_PM_SPI_CK                   64
#define PAD_PM_SPI_DI                   65
#define PAD_PM_SPI_DO                   66
#define PAD_PM_SPI_WPZ                  67
#define PAD_PM_SPI_HLD                  68
#define PAD_PM_LED0                     69
#define PAD_PM_LED1                     70
#define PAD_SAR_GPIO0                   71
#define PAD_SAR_GPIO1                   72
#define PAD_SAR_GPIO2                   73
#define PAD_SAR_GPIO3                   74
#define PAD_ETH_RN                      75
#define PAD_ETH_RP                      76
#define PAD_ETH_TN                      77
#define PAD_ETH_TP                      78
#define PAD_DM_P1                       79
#define PAD_DP_P1                       80
#define PAD_DM_P2                       81
#define PAD_DP_P2                       82
#define PAD_DM_P3                       83
#define PAD_DP_P3                       84
#define PAD_HSYNC_OUT                   85
#define PAD_VSYNC_OUT                   86
#define PAD_HDMITX_SCL                  87
#define PAD_HDMITX_SDA                  88
#define PAD_HDMITX_HPD                  89
#define PAD_SATA_GPIO                   90

#define GPIO_NR                         91
#define PAD_UNKNOWN     0xFFFF

#endif // ___GPIO_H
