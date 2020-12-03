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

#define PINMUX_FOR_GPIO_MODE            0x00

#define PINMUX_FOR_EJ_MODE_1            0x01
#define PINMUX_FOR_EJ_MODE_2            0x02
#define PINMUX_FOR_ALLPAD_IN            0x03
#define PINMUX_FOR_MIPI_PAD_IN_1        0x04
#define PINMUX_FOR_MIPI_PAD_IN_2        0x05
#define PINMUX_FOR_MIPI_PAD_IN_3        0x06
#define PINMUX_FOR_TEST_IN_MODE_1       0x07
#define PINMUX_FOR_TEST_IN_MODE_2       0x08
#define PINMUX_FOR_TEST_IN_MODE_3       0x09
#define PINMUX_FOR_TEST_OUT_MODE_1      0x0a
#define PINMUX_FOR_TEST_OUT_MODE_2      0x0b
#define PINMUX_FOR_TEST_OUT_MODE_3      0x0c
#define PINMUX_FOR_I2C0_MODE_1          0x0d
#define PINMUX_FOR_I2C0_MODE_2          0x0e
#define PINMUX_FOR_I2C0_MODE_3          0x0f
#define PINMUX_FOR_I2C0_MODE_4          0x10
#define PINMUX_FOR_I2C1_MODE_1          0x11
#define PINMUX_FOR_I2C1_MODE_2          0x12
#define PINMUX_FOR_I2C1_MODE_3          0x13
#define PINMUX_FOR_SPI0_MODE_1          0x14
#define PINMUX_FOR_SPI0_MODE_2          0x15
#define PINMUX_FOR_SPI0_MODE_3          0x16
#define PINMUX_FOR_SPI0_MODE_4          0x17
#define PINMUX_FOR_SPI1_MODE_1          0x18
#define PINMUX_FOR_SPI1_MODE_2          0x19
#define PINMUX_FOR_SPI1_MODE_3          0x1a
#define PINMUX_FOR_SPI1_MODE_4          0x1b
#define PINMUX_FOR_FUART_MODE_1         0x1c
#define PINMUX_FOR_FUART_MODE_2         0x1d
#define PINMUX_FOR_FUART_MODE_3         0x1e
#define PINMUX_FOR_FUART_MODE_4         0x1f
#define PINMUX_FOR_FUART_MODE_5         0x20
#define PINMUX_FOR_FUART_MODE_6         0x21
#define PINMUX_FOR_UART0_MODE_1         0x22
#define PINMUX_FOR_UART0_MODE_2         0x23
#define PINMUX_FOR_UART0_MODE_3         0x24
#define PINMUX_FOR_UART0_MODE_4         0x25
#define PINMUX_FOR_UART1_MODE_1         0x26
#define PINMUX_FOR_UART1_MODE_2         0x27
#define PINMUX_FOR_UART1_MODE_3         0x28
#define PINMUX_FOR_UART1_MODE_4         0x29
#define PINMUX_FOR_SD_MODE              0x2a
#define PINMUX_FOR_SDIO_MODE            0x2b
#define PINMUX_FOR_PWM0_MODE_1          0x2c
#define PINMUX_FOR_PWM0_MODE_2          0x2d
#define PINMUX_FOR_PWM0_MODE_3          0x2e
#define PINMUX_FOR_PWM0_MODE_4          0x2f
#define PINMUX_FOR_PWM0_MODE_5          0x30
#define PINMUX_FOR_PWM1_MODE_1          0x31
#define PINMUX_FOR_PWM1_MODE_2          0x32
#define PINMUX_FOR_PWM1_MODE_3          0x33
#define PINMUX_FOR_PWM1_MODE_4          0x34
#define PINMUX_FOR_PWM1_MODE_5          0x35
#define PINMUX_FOR_PWM2_MODE_1          0x36
#define PINMUX_FOR_PWM2_MODE_2          0x37
#define PINMUX_FOR_PWM2_MODE_3          0x38
#define PINMUX_FOR_PWM2_MODE_4          0x39
#define PINMUX_FOR_PWM2_MODE_5          0x3a
#define PINMUX_FOR_PWM3_MODE_1          0x3b
#define PINMUX_FOR_PWM3_MODE_2          0x3c
#define PINMUX_FOR_PWM3_MODE_3          0x3d
#define PINMUX_FOR_PWM3_MODE_4          0x3e
#define PINMUX_FOR_PWM3_MODE_5          0x3f
#define PINMUX_FOR_PWM4_MODE_1          0x40
#define PINMUX_FOR_PWM4_MODE_2          0x41
#define PINMUX_FOR_PWM4_MODE_3          0x42
#define PINMUX_FOR_PWM4_MODE_4          0x43
#define PINMUX_FOR_PWM5_MODE_1          0x44
#define PINMUX_FOR_PWM5_MODE_2          0x45
#define PINMUX_FOR_PWM5_MODE_3          0x46
#define PINMUX_FOR_PWM5_MODE_4          0x47
#define PINMUX_FOR_PWM6_MODE_1          0x48
#define PINMUX_FOR_PWM6_MODE_2          0x49
#define PINMUX_FOR_PWM6_MODE_3          0x4a
#define PINMUX_FOR_PWM6_MODE_4          0x4b
#define PINMUX_FOR_PWM7_MODE_1          0x4c
#define PINMUX_FOR_PWM7_MODE_2          0x4d
#define PINMUX_FOR_PWM7_MODE_3          0x4e
#define PINMUX_FOR_PWM7_MODE_4          0x4f
#define PINMUX_FOR_PWM8_MODE_1          0x50
#define PINMUX_FOR_PWM8_MODE_2          0x51
#define PINMUX_FOR_PWM8_MODE_3          0x52
#define PINMUX_FOR_PWM8_MODE_4          0x53
#define PINMUX_FOR_PWM9_MODE_1          0x54
#define PINMUX_FOR_PWM9_MODE_2          0x55
#define PINMUX_FOR_PWM9_MODE_3          0x56
#define PINMUX_FOR_PWM9_MODE_4          0x57
#define PINMUX_FOR_PWM10_MODE_1         0x58
#define PINMUX_FOR_PWM10_MODE_2         0x59
#define PINMUX_FOR_PWM10_MODE_3         0x5a
#define PINMUX_FOR_PWM10_MODE_4         0x5b
#define PINMUX_FOR_SR_MODE_1            0x5c
#define PINMUX_FOR_SR_MODE_2            0x5d
#define PINMUX_FOR_SR_MODE_3            0x5e
#define PINMUX_FOR_SR_MODE_4            0x5f
#define PINMUX_FOR_SR_MCLK_MODE         0x60
#define PINMUX_FOR_SR_PDN_MODE_1        0x61
#define PINMUX_FOR_SR_PDN_MODE_2        0x62
#define PINMUX_FOR_SR_RST_MODE_1        0x63
#define PINMUX_FOR_SR_RST_MODE_2        0x64
#define PINMUX_FOR_SR_HVSYNC_MODE       0x65
#define PINMUX_FOR_SR_PCK_MODE          0x66
#define PINMUX_FOR_ETH_MODE             0x67
#define PINMUX_FOR_I2S_MODE_1           0x68
#define PINMUX_FOR_I2S_MODE_2           0x69
#define PINMUX_FOR_I2S_MODE_3           0x6a
#define PINMUX_FOR_DMIC_MODE_1          0x6b
#define PINMUX_FOR_DMIC_MODE_2          0x6c
#define PINMUX_FOR_DMIC_MODE_3          0x6d
#define PINMUX_FOR_TTL_MODE_1           0x6e
#define PINMUX_FOR_TTL_MODE_2           0x6f
#define PINMUX_FOR_CCIR_MODE_1          0x70
#define PINMUX_FOR_CCIR_MODE_2          0x71
#define PINMUX_FOR_CCIR_MODE_3          0x72

#define PINMUX_FOR_PM_SPI_MODE          0x73
#define PINMUX_FOR_PM_SPIWPN_MODE       0x74
#define PINMUX_FOR_PM_SPIHOLDN_MODE     0x75
#define PINMUX_FOR_PM_SPICSZ1_MODE      0x76
#define PINMUX_FOR_PM_SPICSZ2_MODE      0x77
#define PINMUX_FOR_PM_PWM0_MODE_1       0x78
#define PINMUX_FOR_PM_PWM0_MODE_2       0x79
#define PINMUX_FOR_PM_PWM1_MODE_1       0x7a
#define PINMUX_FOR_PM_PWM1_MODE_2       0x7b
#define PINMUX_FOR_PM_PWM2_MODE_1       0x7c
#define PINMUX_FOR_PM_PWM2_MODE_2       0x7d
#define PINMUX_FOR_PM_PWM3_MODE_1       0x7e
#define PINMUX_FOR_PM_PWM3_MODE_2       0x7f
#define PINMUX_FOR_PM_PWM4_MODE         0x80
#define PINMUX_FOR_PM_PWM5_MODE         0x81
#define PINMUX_FOR_PM_PWM8_MODE         0x82
#define PINMUX_FOR_PM_PWM9_MODE         0x83
#define PINMUX_FOR_PM_PWM10_MODE        0x84
#define PINMUX_FOR_PM_UART1_MODE        0x85
#define PINMUX_FOR_PM_VID_MODE_1        0x86
#define PINMUX_FOR_PM_VID_MODE_2        0x87
#define PINMUX_FOR_PM_VID_MODE_3        0x88
#define PINMUX_FOR_PM_SD_CDZ_MODE       0x89
#define PINMUX_FOR_PM_LED_MODE          0x8a
#define PINMUX_FOR_PM_TTL_MODE_1        0x8b
#define PINMUX_FOR_PM_TTL_MODE_2        0x8c
#define PINMUX_FOR_PM_IRIN_MODE         0x8d

#define PINMUX_FOR_SAR_MODE             0x8e
#define PINMUX_FOR_USB_MODE             0x8f

#define PINMUX_FOR_UNKNOWN_MODE         0xFF

#endif // ___PADMUX_H
