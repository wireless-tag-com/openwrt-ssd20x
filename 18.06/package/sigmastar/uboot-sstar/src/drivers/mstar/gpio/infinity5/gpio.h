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

#define PAD_GPIO0           0
#define PAD_GPIO1           1
#define PAD_GPIO2           2
#define PAD_GPIO3           3
#define PAD_GPIO4           4
#define PAD_GPIO5           5
#define PAD_GPIO6           6
#define PAD_GPIO7           7
#define PAD_GPIO8           8
#define PAD_GPIO9           9
#define PAD_GPIO10          10
#define PAD_GPIO11          11
#define PAD_GPIO12          12
#define PAD_GPIO13          13
#define PAD_GPIO14          14
#define PAD_GPIO15          15
#define PAD_FUART_RX        16
#define PAD_FUART_TX        17
#define PAD_FUART_CTS       18
#define PAD_FUART_RTS       19
#define PAD_I2C0_SCL        20
#define PAD_I2C0_SDA        21
#define PAD_I2C1_SCL        22
#define PAD_I2C1_SDA        23
#define PAD_SNR0_D0         24
#define PAD_SNR0_D1         25
#define PAD_SNR0_D2         26
#define PAD_SNR0_D3         27
#define PAD_SNR0_D4         28
#define PAD_SNR0_D5         29
#define PAD_SNR0_D6         30
#define PAD_SNR0_D7         31
#define PAD_SNR0_D8         32
#define PAD_SNR0_D9         33
#define PAD_SNR0_D10        34
#define PAD_SNR0_D11        35
#define PAD_SNR0_GPIO0      36
#define PAD_SNR0_GPIO1      37
#define PAD_SNR0_GPIO2      38
#define PAD_SNR0_GPIO3      39
#define PAD_SNR0_GPIO4      40
#define PAD_SNR0_GPIO5      41
#define PAD_SNR0_GPIO6      42
#define PAD_SNR1_DA0P       43
#define PAD_SNR1_DA0N       44
#define PAD_SNR1_CKP        45
#define PAD_SNR1_CKN        46
#define PAD_SNR1_DA1P       47
#define PAD_SNR1_DA1N       48
#define PAD_SNR1_GPIO0      49
#define PAD_SNR1_GPIO1      50
#define PAD_SNR1_GPIO2      51
#define PAD_SNR1_GPIO3      52
#define PAD_SNR1_GPIO4      53
#define PAD_SNR1_GPIO5      54
#define PAD_SNR1_GPIO6      55
#define PAD_NAND_ALE        56
#define PAD_NAND_CLE        57
#define PAD_NAND_CEZ        58
#define PAD_NAND_WEZ        59
#define PAD_NAND_WPZ        60
#define PAD_NAND_REZ        61
#define PAD_NAND_RBZ        62
#define PAD_NAND_DA0        63
#define PAD_NAND_DA1        64
#define PAD_NAND_DA2        65
#define PAD_NAND_DA3        66
#define PAD_NAND_DA4        67
#define PAD_NAND_DA5        68
#define PAD_NAND_DA6        69
#define PAD_NAND_DA7        70
#define PAD_LCD_D0          71
#define PAD_LCD_D1          72
#define PAD_LCD_D2          73
#define PAD_LCD_D3          74
#define PAD_LCD_D4          75
#define PAD_LCD_D5          76
#define PAD_LCD_D6          77
#define PAD_LCD_D7          78
#define PAD_LCD_D8          79
#define PAD_LCD_D9          80
#define PAD_LCD_D10         81
#define PAD_LCD_D11         82
#define PAD_LCD_D12         83
#define PAD_LCD_D13         84
#define PAD_LCD_D14         85
#define PAD_LCD_D15         86
#define PAD_LCD_D16         87
#define PAD_LCD_D17         88
#define PAD_LCD_D18         89
#define PAD_LCD_D19         90
#define PAD_LCD_D20         91
#define PAD_LCD_D21         92
#define PAD_LCD_D22         93
#define PAD_LCD_D23         94
#define PAD_LCD_VSYNC       95
#define PAD_LCD_HSYNC       96
#define PAD_LCD_PCLK        97
#define PAD_LCD_DE          98
#define PAD_UART0_RX        99
#define PAD_UART0_TX        100
#define PAD_UART1_RX        101
#define PAD_UART1_TX        102
#define PAD_SPI0_CZ         103
#define PAD_SPI0_CK         104
#define PAD_SPI0_DI         105
#define PAD_SPI0_DO         106
#define PAD_SPI1_CZ         107
#define PAD_SPI1_CK         108
#define PAD_SPI1_DI         109
#define PAD_SPI1_DO         110
#define PAD_PWM0            111
#define PAD_PWM1            112
#define PAD_SD_CLK          113
#define PAD_SD_CMD          114
#define PAD_SD_D0           115
#define PAD_SD_D1           116
#define PAD_SD_D2           117
#define PAD_SD_D3           118
#define PAD_PM_SD_CDZ       119
#define PAD_PM_IRIN         120
#define PAD_PM_GPIO0        121
#define PAD_PM_GPIO1        122
#define PAD_PM_GPIO2        123
#define PAD_PM_GPIO3        124
#define PAD_PM_GPIO4        125
#define PAD_PM_GPIO5        126
#define PAD_PM_GPIO6        127
#define PAD_PM_GPIO7        128
#define PAD_PM_GPIO8        129
#define PAD_PM_GPIO9        130
#define PAD_PM_GPIO10       131
#define PAD_PM_GPIO11       132
#define PAD_PM_GPIO12       133
#define PAD_PM_GPIO13       134
#define PAD_PM_GPIO14       135
#define PAD_PM_GPIO15       136
#define PAD_PM_SPI_CZ       137
#define PAD_PM_SPI_CK       138
#define PAD_PM_SPI_DI       139
#define PAD_PM_SPI_DO       140
#define PAD_PM_SPI_WPZ      141
#define PAD_PM_SPI_HLD      142
#define PAD_PM_LED0         143
#define PAD_PM_LED1         144
#define PAD_SAR_GPIO0       145
#define PAD_SAR_GPIO1       146
#define PAD_SAR_GPIO2       147
#define PAD_SAR_GPIO3       148
#define PAD_ETH_RN          149
#define PAD_ETH_RP          150
#define PAD_ETH_TN          151
#define PAD_ETH_TP          152
#define PAD_USB_DM          153
#define PAD_USB_DP          154
#define PAD_DM_P1           155
#define PAD_DP_P1           156

#define GPIO_NR             157

#endif  // __GPIO_H__
