/*
* mdrv_puse.h- Sigmastar
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
#ifndef __MDRV_PUSE_H__
#define __MDRV_PUSE_H__

/*
 * Naming Rule:       MDRV_PUSE_<IP>_<PAD_USE>
 *
 * define-value rule: Bit[16~31] for IP Group (e.g 0x00060000 for I2C
 *                    Bit[8~15]  for each IP in the group(e.g 0x00060000 for i2c0, ox00060100 for i2c1)
 *                    Bit[0~7]   for each PAD of a IP(e.g. 0x00060000 for i2c0_scl, 0x00060001 for i2c0_sda)
 *
 */

// Don't need to specify pad usage by which driver
#define MDRV_PUSE_NA                            0x00000000

// EMAC0 pad usage
#define MDRV_PUSE_EMAC0_LED                     0x00010000
// #define MDRV_PUSE_EMAC0_LED_GPIO_GREEN          0x00010001
// #define MDRV_PUSE_EMAC0_LED_GPIO_ORANGE         0x00010002
#define MDRV_PUSE_EMAC0_PHY_RESET               0x00010003

// EMAC1 pad usage
#define MDRV_PUSE_EMAC1_LED                     0x00010100
// #define MDRV_PUSE_EMAC1_LED_GPIO_GREEN          0x00010101
// #define MDRV_PUSE_EMAC1_LED_GPIO_ORANGE         0x00010102
#define MDRV_PUSE_EMAC1_PHY_RESET               0x00010103

// SDIO0
#define MDRV_PUSE_SDIO0_PWR                     0x00020000
#define MDRV_PUSE_SDIO0_CDZ                     0x00020001
#define MDRV_PUSE_SDIO0_CLK                     0x00020002
#define MDRV_PUSE_SDIO0_CMD                     0x00020003
#define MDRV_PUSE_SDIO0_D0                      0x00020004
#define MDRV_PUSE_SDIO0_D1                      0x00020005
#define MDRV_PUSE_SDIO0_D2                      0x00020006
#define MDRV_PUSE_SDIO0_D3                      0x00020007

// SDIO1
#define MDRV_PUSE_SDIO1_PWR                     0x00020100
#define MDRV_PUSE_SDIO1_CDZ                     0x00020101
#define MDRV_PUSE_SDIO1_CLK                     0x00020102
#define MDRV_PUSE_SDIO1_CMD                     0x00020103
#define MDRV_PUSE_SDIO1_D0                      0x00020104
#define MDRV_PUSE_SDIO1_D1                      0x00020105
#define MDRV_PUSE_SDIO1_D2                      0x00020106
#define MDRV_PUSE_SDIO1_D3                      0x00020107

// CPUFREQ
#define MDRV_PUSE_CPUFREQ_VID0                  0x00030000
#define MDRV_PUSE_CPUFREQ_VID1                  0x00030001

//IR
#define MDRV_PUSE_IR                            0x00040000

// EJ-Tag
#define MDRV_PUSE_EJ_TCK                        0x00050000
#define MDRV_PUSE_EJ_TMS                        0x00050001
#define MDRV_PUSE_EJ_TDO                        0x00050002
#define MDRV_PUSE_EJ_TDI                        0x00050003

// I2C
#define MDRV_PUSE_I2C0_SCL                      0x00060000
#define MDRV_PUSE_I2C0_SDA                      0x00060001
#define MDRV_PUSE_I2C1_SCL                      0x00060100
#define MDRV_PUSE_I2C1_SDA                      0x00060101
#define MDRV_PUSE_I2C1_DEV_RESET                0x00060102
#define MDRV_PUSE_I2C1_DEV_IRQ                  0x00060103
#define MDRV_PUSE_I2CSW_SCL                     0x00061000
#define MDRV_PUSE_I2CSW_SDA                     0x00061001

//I2
// UART0
#define MDRV_PUSE_UART0_RX                      0x00070000
#define MDRV_PUSE_UART0_TX                      0x00070001
//UART1
#define MDRV_PUSE_UART1_RX                      0x00070010
#define MDRV_PUSE_UART1_TX                      0x00070011
//UART2
#define MDRV_PUSE_UART2_RX                      0x00070020
#define MDRV_PUSE_UART2_TX                      0x00070021
//FUART
#define MDRV_PUSE_FUART_RX                      0x00070030
#define MDRV_PUSE_FUART_TX                      0x00070031
#define MDRV_PUSE_FUART_CTS                     0x00070032
#define MDRV_PUSE_FUART_RTS                     0x00070033

// PWM0
#define MDRV_PUSE_PWM0                          0x00080000
#define MDRV_PUSE_PWM1                          0x00080100
#define MDRV_PUSE_PWM2                          0x00080200
#define MDRV_PUSE_PWM3                          0x00080300
#define MDRV_PUSE_PWM4                          0x00080400
#define MDRV_PUSE_PWM5                          0x00080500
#define MDRV_PUSE_PWM6                          0x00080600
#define MDRV_PUSE_PWM7                          0x00080700
#define MDRV_PUSE_PWM8                          0x00080800
#define MDRV_PUSE_PWM9                          0x00080900


// DMIC
#define MDRV_PUSE_DMIC_D1                       0x00090000
#define MDRV_PUSE_DMIC_D0                       0x00090001
#define MDRV_PUSE_DMIC_CLK                      0x00090002

// I2S
#define MDRV_PUSE_I2S_WCK                       0x000A0000
#define MDRV_PUSE_I2S_BCK                       0x000A0001
#define MDRV_PUSE_I2S_SDI                       0x000A0002
#define MDRV_PUSE_I2S_SDO                       0x000A0003


// TTL / Tx Mipi
#define MDRV_PUSE_TTL_DOUT00                    0x000B0000
#define MDRV_PUSE_TTL_DOUT01                    0x000B0001
#define MDRV_PUSE_TTL_DOUT02                    0x000B0002
#define MDRV_PUSE_TTL_DOUT03                    0x000B0003
#define MDRV_PUSE_TTL_DOUT04                    0x000B0004
#define MDRV_PUSE_TTL_DOUT05                    0x000B0005
#define MDRV_PUSE_TTL_DOUT06                    0x000B0006
#define MDRV_PUSE_TTL_DOUT07                    0x000B0007
#define MDRV_PUSE_TTL_DOUT08                    0x000B0008
#define MDRV_PUSE_TTL_DOUT09                    0x000B0009
#define MDRV_PUSE_TTL_DOUT10                    0x000B000A
#define MDRV_PUSE_TTL_DOUT11                    0x000B000B
#define MDRV_PUSE_TTL_DOUT12                    0x000B000C
#define MDRV_PUSE_TTL_DOUT13                    0x000B000D
#define MDRV_PUSE_TTL_DOUT14                    0x000B000E
#define MDRV_PUSE_TTL_DOUT15                    0x000B000F
#define MDRV_PUSE_TTL_DOUT16                    0x000B0010
#define MDRV_PUSE_TTL_DOUT17                    0x000B0011
#define MDRV_PUSE_TTL_DOUT18                    0x000B0012
#define MDRV_PUSE_TTL_DOUT19                    0x000B0013
#define MDRV_PUSE_TTL_DOUT20                    0x000B0014
#define MDRV_PUSE_TTL_DOUT21                    0x000B0015
#define MDRV_PUSE_TTL_DOUT22                    0x000B0016
#define MDRV_PUSE_TTL_DOUT23                    0x000B0017
#define MDRV_PUSE_TTL_DOUT24                    0x000B0018
#define MDRV_PUSE_TTL_CLK                       0x000B0019
#define MDRV_PUSE_TTL_HSYNC                     0x000B001A
#define MDRV_PUSE_TTL_VSYNC                     0x000B001B
#define MDRV_PUSE_TTL_DE                        0x000B001C

#define MDRV_PUSE_TX_MIPI_P_CH0                 0x000B0080
#define MDRV_PUSE_TX_MIPI_N_CH0                 0x000B0081
#define MDRV_PUSE_TX_MIPI_P_CH1                 0x000B0082
#define MDRV_PUSE_TX_MIPI_N_CH1                 0x000B0083
#define MDRV_PUSE_TX_MIPI_P_CH2                 0x000B0084
#define MDRV_PUSE_TX_MIPI_N_CH2                 0x000B0085
#define MDRV_PUSE_TX_MIPI_P_CH3                 0x000B0086
#define MDRV_PUSE_TX_MIPI_N_CH3                 0x000B0087
#define MDRV_PUSE_TX_MIPI_P_CH4                 0x000B0088
#define MDRV_PUSE_TX_MIPI_N_CH4                 0x000B0089

#define MDRV_PUSE_IDAC_HSYNC                    0x000B0100
#define MDRV_PUSE_IDAC_VSYNC                    0x000B0101

// SPI0
#define MDRV_PUSE_SPI0_CZ                       0x000C0000
#define MDRV_PUSE_SPI0_CK                       0x000C0001
#define MDRV_PUSE_SPI0_DI                       0x000C0002
#define MDRV_PUSE_SPI0_DO                       0x000C0003

// SAR
#define MDRV_PUSE_SAR_GPIO0                     0x000D0000
#define MDRV_PUSE_SAR_GPIO1                     0x000D0100

//usb host
#define MDRV_PUSE_UTMI1_DM                      0x000E0000
#define MDRV_PUSE_UTMI1_DP                      0x000E0000
#define MDRV_PUSE_UTMI2_DM                      0x000E0100
#define MDRV_PUSE_UTMI2_DP                      0x000E0100
#define MDRV_PUSE_UTMI3_DM                      0x000E0200
#define MDRV_PUSE_UTMI3_DP                      0x000E0200
#define MDRV_PUSE_UTMI_POWER                    0x000E1000

// Audio
#define MDRV_PUSE_AIO_AMP_PWR                   0x000F0000
#define MDRV_PUSE_AIO_MCK                       0x000F0001

#endif // #define __MDRV_PUSE_H__
