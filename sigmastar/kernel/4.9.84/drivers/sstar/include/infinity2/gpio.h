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

#define PAD_PM_IRIN                     0   //intr
#define PAD_PM_SPI_CZ                   1   //intr
#define PAD_PM_SPI_CK                   2   //intr
#define PAD_PM_SPI_DI                   3   //intr
#define PAD_PM_SPI_DO                   4   //intr
#define PAD_PM_SPI_WPZ                  5
#define PAD_PM_SPI_HOLDZ                6
#define PAD_PM_SPI_RSTZ                 7
#define PAD_PM_GPIO0                    8   //intr
#define PAD_PM_GPIO1                    9   //intr
#define PAD_PM_GPIO2                    10  //intr
#define PAD_PM_GPIO3                    11  //intr
#define PAD_PM_GPIO4                    12  //intr
#define PAD_PM_GPIO5                    13  //intr
#define PAD_PM_GPIO6                    14  //intr
#define PAD_PM_GPIO7                    15  //intr
#define PAD_PM_GPIO8                    16  //intr
#define PAD_PM_GPIO9                    17  //intr
#define PAD_PM_GPIO10                   18  //intr
#define PAD_PM_GPIO11                   19  //intr
#define PAD_PM_GPIO12                   20  //intr
#define PAD_PM_GPIO13                   21  //intr
#define PAD_PM_GPIO14                   22  //intr
#define PAD_PM_GPIO15                   23  //intr
#define PAD_PM_CEC                      24  //intr
#define PAD_HDMITX_HPD                  25
#define PAD_HDMIRX_HPD                  26
#define PAD_PM_SD30_CDZ                 27	//intr
#define PAD_PM_SD20_CDZ                 28	//intr
#define PAD_VID0                        29	//intr
#define PAD_VID1                        30	//intr
#define PAD_PM_LED0                     31	//intr
#define PAD_PM_LED1                     32	//intr
#define PAD_SAR_GPIO0                   33	//??
#define PAD_SAR_GPIO1                   34	//??
#define PAD_SAR_GPIO2                   35	//??
#define PAD_SAR_GPIO3                   36	//??
#define PAD_SAR_GPIO4                   37	//??
#define PAD_VPLUG_IN                    38	//??
#define PAD_PM_GPIO16                   39	//intr
#define PAD_PM_GPIO17                   40	//intr
#define PAD_PM_GPIO18                   41	//intr
#define PAD_PM_GPIO19                   42	//intr
#define PAD_PM_SPI_CZ1                  43	//intr
#define PAD_PM_SPI_CZ2                  44	//intr
#define PAD_SPDIF_OUT                   45
#define PAD_HSYNC_OUT                   46
#define PAD_VSYNC_OUT                   47
#define PAD_BT_I2S_RX_BCK               48
#define PAD_BT_I2S_RX_WCK               49
#define PAD_BT_I2S_RX_SDI               50
#define PAD_BT_I2S_TX_SDO               51
#define PAD_GPIO0                       52
#define PAD_GPIO1                       53
#define PAD_GPIO2                       54
#define PAD_GPIO3                       55
#define PAD_GPIO4                       56
#define PAD_GPIO5                       57
#define PAD_GPIO6                       58
#define PAD_GPIO7                       59
#define PAD_GPIO8                       60
#define PAD_GPIO9                       61
#define PAD_GPIO10                      62
#define PAD_GPIO11                      63
#define PAD_GPIO12                      64
#define PAD_GPIO13                      65
#define PAD_GPIO14                      66
#define PAD_GPIO15                      67
#define PAD_CODEC_I2S_TX_BCK            68
#define PAD_CODEC_I2S_TX_WCK            69
#define PAD_CODEC_I2S_TX_SDO            70
#define PAD_CODEC_I2S_RX_MCK            71
#define PAD_CODEC_I2S_RX_BCK            72
#define PAD_CODEC_I2S_RX_WCK            73
#define PAD_CODEC_I2S_RX_SDI0           74
#define PAD_CODEC_I2S_RX_SDI1           75
#define PAD_CODEC_I2S_RX_SDI2           76
#define PAD_CODEC_I2S_RX_SDI3           77
#define PAD_DMIC_BCK                    78
#define PAD_DMIC_CH0                    79
#define PAD_DMIC_CH1                    80
#define PAD_DMIC_CH2                    81
#define PAD_DMIC_CH3                    82
#define PAD_FUART_TX                    83
#define PAD_FUART_RX                    84
#define PAD_FUART_RTS                   85
#define PAD_FUART_CTS                   86
#define PAD_I2C0_SDA                    87
#define PAD_I2C0_SCL                    88
#define PAD_I2C2_SDA                    89
#define PAD_I2C2_SCL                    90
#define PAD_I2C3_SCL                    91
#define PAD_I2C3_SDA                    92
#define PAD_JTAG_TCK                    93
#define PAD_JTAG_TDO                    94
#define PAD_JTAG_TDI                    95
#define PAD_JTAG_TMS                    96
#define PAD_MIPI_TX_IO0         97 //OUTP_CH0
#define PAD_MIPI_TX_IO1         98 //OUTN_CH0
#define PAD_MIPI_TX_IO2         99 //OUTP_CH1
#define PAD_MIPI_TX_IO3         100 //OUTN_CH1
#define PAD_MIPI_TX_IO4         101 //OUTP_CH2
#define PAD_MIPI_TX_IO5         102 //OUTN_CH2
#define PAD_MIPI_TX_IO6         103 //OUTP_CH3
#define PAD_MIPI_TX_IO7         104 //OUTN_CH3
#define PAD_MIPI_TX_IO8         105 //OUTP_CH4
#define PAD_MIPI_TX_IO9         106 //OUTN_CH4
#define PAD_MISC_I2S_TX_BCK             107
#define PAD_MISC_I2S_TX_WCK             108
#define PAD_MISC_I2S_TX_SDO             109
#define PAD_MISC_I2S_RX_MCK             110
#define PAD_MISC_I2S_RX_BCK             111
#define PAD_MISC_I2S_RX_WCK             112
#define PAD_MISC_I2S_RX_SDI0            113
#define PAD_NAND_CEZ0                   114
#define PAD_NAND_CEZ1                   115
#define PAD_NAND_ALE                    116
#define PAD_NAND_CLE                    117
#define PAD_NAND_WEZ                    118
#define PAD_NAND_WPZ                    119
#define PAD_NAND_REZ                    120
#define PAD_NAND_RBZ                    121
#define PAD_NAND_DA0                    122
#define PAD_NAND_DA1                    123
#define PAD_NAND_DA2                    124
#define PAD_NAND_DA3                    125
#define PAD_NAND_DA4                    126
#define PAD_NAND_DA5                    127
#define PAD_NAND_DA6                    128
#define PAD_NAND_DA7                    129
#define PAD_NAND_DQS                    130
#define PAD_RGMII_0_MDIO                131
#define PAD_RGMII_0_TX_CTL              132
#define PAD_RGMII_0_MDC                 133
#define PAD_RGMII_0_TXD3                134
#define PAD_RGMII_0_RX_CLK              135
#define PAD_RGMII_0_TXD2                136
#define PAD_RGMII_0_RXD3                137
#define PAD_RGMII_0_TXD1                138
#define PAD_RGMII_0_RXD2                139
#define PAD_RGMII_0_TXD0                140
#define PAD_RGMII_0_RXD1                141
#define PAD_RGMII_0_TX_CLK              142
#define PAD_RGMII_0_RXD0                143
#define PAD_RGMII_0_RX_CTL              144
#define PAD_RGMII_1_TX_CTL              145
#define PAD_RGMII_1_RX_CLK              146
#define PAD_RGMII_1_TXD3                147
#define PAD_RGMII_1_RXD3                148
#define PAD_RGMII_1_TXD2                149
#define PAD_RGMII_1_RXD2                150
#define PAD_RGMII_1_TXD1                151
#define PAD_RGMII_1_RXD1                152
#define PAD_RGMII_1_TXD0                153
#define PAD_RGMII_1_RXD0                154
#define PAD_RGMII_1_TX_CLK              155
#define PAD_RGMII_1_RX_CTL              156
#define PAD_PWM0                        157
#define PAD_PWM1                        158
#define PAD_SD_CLK                      159
#define PAD_SD_CMD                      160
#define PAD_SD_D0                       161
#define PAD_SD_D1                       162
#define PAD_SD_D2                       163
#define PAD_SD_D3                       164
#define PAD_SD30_IO0                    165
#define PAD_SD30_IO1                    166
#define PAD_SD30_IO2                    167
#define PAD_SD30_IO3                    168
#define PAD_SD30_IO4                    169
#define PAD_SD30_IO5                    170
#define PAD_SNR0_D0                     171
#define PAD_SNR0_D1                     172
#define PAD_SNR0_D2                     173
#define PAD_SNR0_D3                     174
#define PAD_SNR0_D4                     175
#define PAD_SNR0_D5                     176
#define PAD_SNR0_D6                     177
#define PAD_SNR0_D7                     178
#define PAD_SNR0_D8                     179
#define PAD_SNR0_D9                     180
#define PAD_SNR0_GPIO0                  181
#define PAD_SNR0_GPIO1                  182
#define PAD_SNR0_GPIO2                  183
#define PAD_SNR0_GPIO3                  184
#define PAD_SNR0_GPIO4                  185
#define PAD_SNR0_GPIO5                  186
#define PAD_SNR0_GPIO6                  187
#define PAD_SNR0_GPIO7                  188
#define PAD_SNR1_D0                     189
#define PAD_SNR1_D1                     190
#define PAD_SNR1_D2                     191
#define PAD_SNR1_D3                     192
#define PAD_SNR1_D4                     193
#define PAD_SNR1_D5                     194
#define PAD_SNR1_D6                     195
#define PAD_SNR1_D7                     196
#define PAD_SNR1_D8                     197
#define PAD_SNR1_D9                     198
#define PAD_SNR1_GPIO0                  199
#define PAD_SNR1_GPIO1                  200
#define PAD_SNR1_GPIO2                  201
#define PAD_SNR1_GPIO3                  202
#define PAD_SNR1_GPIO4                  203
#define PAD_SNR1_GPIO5                  204
#define PAD_SNR1_GPIO6                  205
#define PAD_SNR1_GPIO7                  206
#define PAD_SNR2_D0                     207
#define PAD_SNR2_D1                     208
#define PAD_SNR2_D2                     209
#define PAD_SNR2_D3                     210
#define PAD_SNR2_D4                     211
#define PAD_SNR2_D5                     212
#define PAD_SNR2_D6                     213
#define PAD_SNR2_D7                     214
#define PAD_SNR2_D8                     215
#define PAD_SNR2_D9                     216
#define PAD_SNR2_GPIO0                  217
#define PAD_SNR2_GPIO1                  218
#define PAD_SNR2_GPIO2                  219
#define PAD_SNR2_GPIO3                  220
#define PAD_SNR2_GPIO4                  221
#define PAD_SNR2_GPIO5                  222
#define PAD_SNR2_GPIO6                  223
#define PAD_SNR2_GPIO7                  224
#define PAD_SNR3_D0                     225
#define PAD_SNR3_D1                     226
#define PAD_SNR3_D2                     227
#define PAD_SNR3_D3                     228
#define PAD_SNR3_D4                     229
#define PAD_SNR3_D5                     230
#define PAD_SNR3_D6                     231
#define PAD_SNR3_D7                     232
#define PAD_SNR3_D8                     233
#define PAD_SNR3_D9                     234
#define PAD_SNR3_GPIO0                  235
#define PAD_SNR3_GPIO1                  236
#define PAD_SNR3_GPIO2                  237
#define PAD_SNR3_GPIO3                  238
#define PAD_SNR3_GPIO4                  239
#define PAD_SNR3_GPIO5                  240
#define PAD_SNR3_GPIO6                  241
#define PAD_SNR3_GPIO7                  242
#define PAD_SPI0_CK                     243
#define PAD_SPI0_CZ0                    244
#define PAD_SPI0_DO                     245
#define PAD_SPI0_DI                     246
#define PAD_SPI1_CK                     247
#define PAD_SPI1_CZ0                    248
#define PAD_SPI1_DO                     249
#define PAD_SPI1_DI                     250
#define PAD_SPI2_CZ0                    251
#define PAD_SPI2_CK                     252
#define PAD_SPI2_DI                     253
#define PAD_SPI2_DO                     254
#define PAD_TTL_HSYNC                   255
#define PAD_TTL_VSYNC                   256
#define PAD_TTL_CLK                     257
#define PAD_TTL_DE                      258
#define PAD_TTL_D0                      259
#define PAD_TTL_D1                      260
#define PAD_TTL_D2                      261
#define PAD_TTL_D3                      262
#define PAD_TTL_D4                      263
#define PAD_TTL_D5                      264
#define PAD_TTL_D6                      265
#define PAD_TTL_D7                      266
#define PAD_TTL_D8                      267
#define PAD_TTL_D9                      268
#define PAD_TTL_D10                     269
#define PAD_TTL_D11                     270
#define PAD_TTL_D12                     271
#define PAD_TTL_D13                     272
#define PAD_TTL_D14                     273
#define PAD_TTL_D15                     274
#define PAD_TTL_D16                     275
#define PAD_TTL_D17                     276
#define PAD_TTL_D18                     277
#define PAD_TTL_D19                     278
#define PAD_TTL_D20                     279
#define PAD_TTL_D21                     280
#define PAD_TTL_D22                     281
#define PAD_TTL_D23                     282
#define PAD_TTL_GPIO0                   283
#define PAD_TTL_GPIO1                   284
#define PAD_TTL_GPIO2                   285
#define PAD_UART0_RX                    286
#define PAD_UART0_TX                    287
#define PAD_UART1_RX                    288
#define PAD_UART1_TX                    289
#define PAD_UART2_RX                    290
#define PAD_UART2_TX                    291
#define PAD_HDMITX_SCL                  292
#define PAD_HDMITX_SDA                  293
#define PAD_HDMITX_ARC                  294

#define GPIO_NR                         295
#define PAD_UNKNOWN     0xFFFF

#endif // #ifndef ___GPIO_H

