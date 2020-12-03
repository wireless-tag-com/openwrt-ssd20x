/*
* mhal_gpio.c- Sigmastar
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
//#include "MsCommon.h"
//#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <asm/io.h>

#include "mhal_gpio.h"
#include "mhal_gpio_reg.h"
#include "ms_platform.h"
#include "gpio.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
#if 1
#define _CONCAT( a, b )     a##b
#define CONCAT( a, b )      _CONCAT( a, b )
/*
#define BIT0    BIT(0)
#define BIT1    BIT(1)
#define BIT2    BIT(2)
#define BIT3    BIT(3)
#define BIT4    BIT(4)
#define BIT5    BIT(5)
#define BIT6    BIT(6)
#define BIT7    BIT(7)
*/
    // Dummy
#define GPIO999_OEN     0, 0
#define GPIO999_OUT     0, 0
#define GPIO999_IN      0, 0

#define GPIO0_PAD PAD_PM_GPIO0
#define GPIO0_OEN 0x0f08, BIT0
#define GPIO0_IN  0x0f10, BIT0
#define GPIO0_OUT 0x0f0c, BIT0

#define GPIO1_PAD PAD_PM_GPIO1
#define GPIO1_OEN 0x0f08, BIT1
#define GPIO1_IN  0x0f10, BIT1
#define GPIO1_OUT 0x0f0c, BIT1

#define GPIO2_PAD PAD_PM_IIC_SDA
#define GPIO2_OEN 0x0f08, BIT2
#define GPIO2_IN  0x0f10, BIT2
#define GPIO2_OUT 0x0f0c, BIT2

#define GPIO3_PAD PAD_PM_IIC_SCL
#define GPIO3_OEN 0x0f08, BIT3
#define GPIO3_IN  0x0f10, BIT3
#define GPIO3_OUT 0x0f0c, BIT3

#define GPIO4_PAD PAD_PM_MIIC_SDA
#define GPIO4_OEN 0x0f08, BIT4
#define GPIO4_IN  0x0f10, BIT4
#define GPIO4_OUT 0x0f0c, BIT4

#define GPIO5_PAD PAD_PM_MIIC_SCL
#define GPIO5_OEN 0x0f08, BIT5
#define GPIO5_IN  0x0f10, BIT5
#define GPIO5_OUT 0x0f0c, BIT5

#define GPIO6_PAD PAD_PM_SPI_CK
#define GPIO6_OEN 0x0f08, BIT6
#define GPIO6_IN  0x0f10, BIT6
#define GPIO6_OUT 0x0f0c, BIT6

#define GPIO7_PAD PAD_PM_SPI_CZ0
#define GPIO7_OEN 0x0f08, BIT7
#define GPIO7_IN  0x0f10, BIT7
#define GPIO7_OUT 0x0f0c, BIT7

#define GPIO8_PAD PAD_PM_SPI_GPIO
#define GPIO8_OEN 0x0f09, BIT0
#define GPIO8_IN  0x0f11, BIT0
#define GPIO8_OUT 0x0f0d, BIT0

#define GPIO9_PAD PAD_PM_SPI_DI
#define GPIO9_OEN 0x0f09, BIT1
#define GPIO9_IN  0x0f11, BIT1
#define GPIO9_OUT 0x0f0d, BIT1

#define GPIO10_PAD PAD_PM_SPI_DO
#define GPIO10_OEN 0x0f09, BIT2
#define GPIO10_IN  0x0f11, BIT2
#define GPIO10_OUT 0x0f0d, BIT2

#define GPIO11_PAD PAD_ONOFF
#define GPIO11_OEN 0x0f09, BIT3
#define GPIO11_IN  0x0f11, BIT3
#define GPIO11_OUT 0x0f0d, BIT3

#define GPIO12_PAD PAD_CHRGDET
#define GPIO12_OEN 0x0f09, BIT4
#define GPIO12_IN  0x0f11, BIT4
#define GPIO12_OUT 0x0f0d, BIT4

#define GPIO13_PAD PAD_32K_OUT
#define GPIO13_OEN 0x0f09, BIT5
#define GPIO13_IN  0x0f11, BIT5
#define GPIO13_OUT 0x0f0d, BIT5

#define GPIO14_PAD PAD_IRIN
#define GPIO14_OEN 0x0f09, BIT6
#define GPIO14_IN  0x0f11, BIT6
#define GPIO14_OUT 0x0f0d, BIT6

#define GPIO15_PAD PAD_CEC
#define GPIO15_OEN 0x0f09, BIT7
#define GPIO15_IN  0x0f11, BIT7
#define GPIO15_OUT 0x0f0d, BIT7

#define GPIO16_PAD PAD_PM_SD0_CDZ
#define GPIO16_OEN 0x0f0a, BIT0
#define GPIO16_IN  0x0f12, BIT0
#define GPIO16_OUT 0x0f0e, BIT0

#define GPIO17_PAD PAD_PM_SD1_CDZ
#define GPIO17_OEN 0x0f0a, BIT1
#define GPIO17_IN  0x0f12, BIT1
#define GPIO17_OUT 0x0f0e, BIT1

#define GPIO18_PAD PAD_PM_SD2_CDZ
#define GPIO18_OEN 0x0f0a, BIT2
#define GPIO18_IN  0x0f12, BIT2
#define GPIO18_OUT 0x0f0e, BIT2

#define GPIO19_PAD PAD_PM_SD_GPIO
#define GPIO19_OEN 0x0f0a, BIT3
#define GPIO19_IN  0x0f12, BIT3
#define GPIO19_OUT 0x0f0e, BIT3

#define GPIO20_PAD PAD_HOTPLUG
#define GPIO20_OEN 0x0f0a, BIT4
#define GPIO20_IN  0x0f12, BIT4
#define GPIO20_OUT 0x0f0e, BIT4

#define GPIO21_PAD PAD_BATOK
#define GPIO21_OEN 0x0f0a, BIT5
#define GPIO21_IN  0x0f12, BIT5
#define GPIO21_OUT 0x0f0e, BIT5

#define GPIO22_PAD PAD_PMIC_CHIPEN
#define GPIO22_OEN 0x0f0a, BIT6
#define GPIO22_IN  0x0f12, BIT6
#define GPIO22_OUT 0x0f0e, BIT6

#define GPIO23_PAD PAD_PMIC_STDBYN
#define GPIO23_OEN 0x0f0a, BIT7
#define GPIO23_IN  0x0f12, BIT7
#define GPIO23_OUT 0x0f0e, BIT7

#define GPIO24_PAD PAD_PMIC_INT
#define GPIO24_OEN 0x0f0b, BIT0
#define GPIO24_IN  0x0f13, BIT0
#define GPIO24_OUT 0x0f0f, BIT0

#define GPIO25_PAD PAD_SD0_D1
#define GPIO25_OEN 0x102b7A, BIT1
#define GPIO25_IN  0x102b7A, BIT2
#define GPIO25_OUT 0x102b7A, BIT0

#define GPIO26_PAD PAD_SD0_D0
#define GPIO26_OEN 0x102b79, BIT1
#define GPIO26_IN  0x102b79, BIT2
#define GPIO26_OUT 0x102b79, BIT0

#define GPIO27_PAD PAD_SD0_CLK
#define GPIO27_OEN 0x102b7C, BIT1
#define GPIO27_IN  0x102b7C, BIT2
#define GPIO27_OUT 0x102b7C, BIT0

#define GPIO28_PAD PAD_SD0_CMD
#define GPIO28_OEN 0x102b7B, BIT1
#define GPIO28_IN  0x102b7B, BIT2
#define GPIO28_OUT 0x102b7B, BIT0

#define GPIO29_PAD PAD_SD0_D3
#define GPIO29_OEN 0x102b7E, BIT1
#define GPIO29_IN  0x102b7E, BIT2
#define GPIO29_OUT 0x102b7E, BIT0

#define GPIO30_PAD PAD_SD0_D2
#define GPIO30_OEN 0x102b7D, BIT1
#define GPIO30_IN  0x102b7D, BIT2
#define GPIO30_OUT 0x102b7D, BIT0

#define GPIO31_PAD PAD_SD1_D1
#define GPIO31_OEN 0x102b80, BIT1
#define GPIO31_IN  0x102b80, BIT2
#define GPIO31_OUT 0x102b80, BIT0

#define GPIO32_PAD PAD_SD1_D0
#define GPIO32_OEN 0x102b7F, BIT1
#define GPIO32_IN  0x102b7F, BIT2
#define GPIO32_OUT 0x102b7F, BIT0

#define GPIO33_PAD PAD_SD1_CLK
#define GPIO33_OEN 0x102b82, BIT1
#define GPIO33_IN  0x102b82, BIT2
#define GPIO33_OUT 0x102b82, BIT0

#define GPIO34_PAD PAD_SD1_CMD
#define GPIO34_OEN 0x102b81, BIT1
#define GPIO34_IN  0x102b81, BIT2
#define GPIO34_OUT 0x102b81, BIT0

#define GPIO35_PAD PAD_SD1_D3
#define GPIO35_OEN 0x102b84, BIT1
#define GPIO35_IN  0x102b84, BIT2
#define GPIO35_OUT 0x102b84, BIT0

#define GPIO36_PAD PAD_SD1_D2
#define GPIO36_OEN 0x102b83, BIT1
#define GPIO36_IN  0x102b83, BIT2
#define GPIO36_OUT 0x102b83, BIT0

#define GPIO37_PAD PAD_A_GPS_EVENT
#define GPIO37_OEN 0x102b31, BIT1
#define GPIO37_IN  0x102b31, BIT2
#define GPIO37_OUT 0x102b31, BIT0

#define GPIO38_PAD PAD_AFE1_SGN
#define GPIO38_OEN 0x102b32, BIT1
#define GPIO38_IN  0x102b32, BIT2
#define GPIO38_OUT 0x102b32, BIT0

#define GPIO39_PAD PAD_AFE1_MAG
#define GPIO39_OEN 0x102b33, BIT1
#define GPIO39_IN  0x102b33, BIT2
#define GPIO39_OUT 0x102b33, BIT0

#define GPIO40_PAD PAD_RFSPI_CZ1
#define GPIO40_OEN 0x102b39, BIT1
#define GPIO40_IN  0x102b39, BIT2
#define GPIO40_OUT 0x102b39, BIT0

#define GPIO41_PAD PAD_AFE0_SGN
#define GPIO41_OEN 0x102b34, BIT1
#define GPIO41_IN  0x102b34, BIT2
#define GPIO41_OUT 0x102b34, BIT0

#define GPIO42_PAD PAD_AFE0_MAG
#define GPIO42_OEN 0x102b35, BIT1
#define GPIO42_IN  0x102b35, BIT2
#define GPIO42_OUT 0x102b35, BIT0

#define GPIO43_PAD PAD_RFSPI_CLK
#define GPIO43_OEN 0x102b38, BIT1
#define GPIO43_IN  0x102b38, BIT2
#define GPIO43_OUT 0x102b38, BIT0

#define GPIO44_PAD PAD_RFSPI_DATA
#define GPIO44_OEN 0x102b37, BIT1
#define GPIO44_IN  0x102b37, BIT2
#define GPIO44_OUT 0x102b37, BIT0

#define GPIO45_PAD PAD_RFSPI_CZ0
#define GPIO45_OEN 0x102b36, BIT1
#define GPIO45_IN  0x102b36, BIT2
#define GPIO45_OUT 0x102b36, BIT0

#define GPIO46_PAD PAD_BTI_RX_TX
#define GPIO46_OEN 0x102b06, BIT1
#define GPIO46_IN  0x102b06, BIT2
#define GPIO46_OUT 0x102b06, BIT0

#define GPIO47_PAD PAD_BTI_DATA1
#define GPIO47_OEN 0x102b05, BIT1
#define GPIO47_IN  0x102b05, BIT2
#define GPIO47_OUT 0x102b05, BIT0

#define GPIO48_PAD PAD_BTI_DATA0
#define GPIO48_OEN 0x102b04, BIT1
#define GPIO48_IN  0x102b04, BIT2
#define GPIO48_OUT 0x102b04, BIT0

#define GPIO49_PAD PAD_BT_CLK_24M
#define GPIO49_OEN 0x102b03, BIT1
#define GPIO49_IN  0x102b03, BIT2
#define GPIO49_OUT 0x102b03, BIT0

#define GPIO50_PAD PAD_BT_SPI_SCLK
#define GPIO50_OEN 0x102b02, BIT1
#define GPIO50_IN  0x102b02, BIT2
#define GPIO50_OUT 0x102b02, BIT0

#define GPIO51_PAD PAD_BT_SPI_SDATA
#define GPIO51_OEN 0x102b01, BIT1
#define GPIO51_IN  0x102b01, BIT2
#define GPIO51_OUT 0x102b01, BIT0

#define GPIO52_PAD PAD_BT_SPI_SENB
#define GPIO52_OEN 0x102b00, BIT1
#define GPIO52_IN  0x102b00, BIT2
#define GPIO52_OUT 0x102b00, BIT0

#define GPIO53_PAD PAD_BT_CHIP_EN
#define GPIO53_OEN 0x102b07, BIT1
#define GPIO53_IN  0x102b07, BIT2
#define GPIO53_OUT 0x102b07, BIT0

#define GPIO54_PAD PAD_SR_D0
#define GPIO54_OEN 0x102b97, BIT1
#define GPIO54_IN  0x102b97, BIT2
#define GPIO54_OUT 0x102b97, BIT0

#define GPIO55_PAD PAD_SR_D1
#define GPIO55_OEN 0x102b96, BIT1
#define GPIO55_IN  0x102b96, BIT2
#define GPIO55_OUT 0x102b96, BIT0

#define GPIO56_PAD PAD_SR_D2
#define GPIO56_OEN 0x102b99, BIT1
#define GPIO56_IN  0x102b99, BIT2
#define GPIO56_OUT 0x102b99, BIT0

#define GPIO57_PAD PAD_SR_D3
#define GPIO57_OEN 0x102b98, BIT1
#define GPIO57_IN  0x102b98, BIT2
#define GPIO57_OUT 0x102b98, BIT0

#define GPIO58_PAD PAD_SR_D4
#define GPIO58_OEN 0x102b9B, BIT1
#define GPIO58_IN  0x102b9B, BIT2
#define GPIO58_OUT 0x102b9B, BIT0

#define GPIO59_PAD PAD_SR_D5
#define GPIO59_OEN 0x102b9A, BIT1
#define GPIO59_IN  0x102b9A, BIT2
#define GPIO59_OUT 0x102b9A, BIT0

#define GPIO60_PAD PAD_SR_D6
#define GPIO60_OEN 0x102b9D, BIT1
#define GPIO60_IN  0x102b9D, BIT2
#define GPIO60_OUT 0x102b9D, BIT0

#define GPIO61_PAD PAD_SR_D7
#define GPIO61_OEN 0x102b9C, BIT1
#define GPIO61_IN  0x102b9C, BIT2
#define GPIO61_OUT 0x102b9C, BIT0

#define GPIO62_PAD PAD_SR_PCLK
#define GPIO62_OEN 0x102bA3, BIT1
#define GPIO62_IN  0x102bA3, BIT2
#define GPIO62_OUT 0x102bA3, BIT0

#define GPIO63_PAD PAD_SR_HSYNC
#define GPIO63_OEN 0x102b9F, BIT1
#define GPIO63_IN  0x102b9F, BIT2
#define GPIO63_OUT 0x102b9F, BIT0

#define GPIO64_PAD PAD_SR_STROBE
#define GPIO64_OEN 0x102b9E, BIT1
#define GPIO64_IN  0x102b9E, BIT2
#define GPIO64_OUT 0x102b9E, BIT0

#define GPIO65_PAD PAD_SR_VSYNC
#define GPIO65_OEN 0x102bA0, BIT1
#define GPIO65_IN  0x102bA0, BIT2
#define GPIO65_OUT 0x102bA0, BIT0

#define GPIO66_PAD PAD_SR_RST
#define GPIO66_OEN 0x102bA1, BIT1
#define GPIO66_IN  0x102bA1, BIT2
#define GPIO66_OUT 0x102bA1, BIT0

#define GPIO67_PAD PAD_SR_PWRDN
#define GPIO67_OEN 0x102bA2, BIT1
#define GPIO67_IN  0x102bA2, BIT2
#define GPIO67_OUT 0x102bA2, BIT0

#define GPIO68_PAD PAD_GPIO33
#define GPIO68_OEN 0x102b2A, BIT1
#define GPIO68_IN  0x102b2A, BIT2
#define GPIO68_OUT 0x102b2A, BIT0

#define GPIO69_PAD PAD_GPIO32
#define GPIO69_OEN 0x102b29, BIT1
#define GPIO69_IN  0x102b29, BIT2
#define GPIO69_OUT 0x102b29, BIT0

#define GPIO70_PAD PAD_GPIO31
#define GPIO70_OEN 0x102b28, BIT1
#define GPIO70_IN  0x102b28, BIT2
#define GPIO70_OUT 0x102b28, BIT0

#define GPIO71_PAD PAD_GPIO30
#define GPIO71_OEN 0x102b27, BIT1
#define GPIO71_IN  0x102b27, BIT2
#define GPIO71_OUT 0x102b27, BIT0

#define GPIO72_PAD PAD_GPIO29
#define GPIO72_OEN 0x102b26, BIT1
#define GPIO72_IN  0x102b26, BIT2
#define GPIO72_OUT 0x102b26, BIT0

#define GPIO73_PAD PAD_GPIO28
#define GPIO73_OEN 0x102b25, BIT1
#define GPIO73_IN  0x102b25, BIT2
#define GPIO73_OUT 0x102b25, BIT0

#define GPIO74_PAD PAD_GPIO27
#define GPIO74_OEN 0x102b24, BIT1
#define GPIO74_IN  0x102b24, BIT2
#define GPIO74_OUT 0x102b24, BIT0

#define GPIO75_PAD PAD_GPIO26
#define GPIO75_OEN 0x102b23, BIT1
#define GPIO75_IN  0x102b23, BIT2
#define GPIO75_OUT 0x102b23, BIT0

#define GPIO76_PAD PAD_GPIO25
#define GPIO76_OEN 0x102b22, BIT1
#define GPIO76_IN  0x102b22, BIT2
#define GPIO76_OUT 0x102b22, BIT0

#define GPIO77_PAD PAD_GPIO24
#define GPIO77_OEN 0x102b21, BIT1
#define GPIO77_IN  0x102b21, BIT2
#define GPIO77_OUT 0x102b21, BIT0

#define GPIO78_PAD PAD_UART_RX1
#define GPIO78_OEN 0x102bB0, BIT1
#define GPIO78_IN  0x102bB0, BIT2
#define GPIO78_OUT 0x102bB0, BIT0

#define GPIO79_PAD PAD_UART_TX1
#define GPIO79_OEN 0x102bB1, BIT1
#define GPIO79_IN  0x102bB1, BIT2
#define GPIO79_OUT 0x102bB1, BIT0

#define GPIO80_PAD PAD_UART_RX2
#define GPIO80_OEN 0x102bB3, BIT1
#define GPIO80_IN  0x102bB3, BIT2
#define GPIO80_OUT 0x102bB3, BIT0

#define GPIO81_PAD PAD_UART_TX2
#define GPIO81_OEN 0x102bB2, BIT1
#define GPIO81_IN  0x102bB2, BIT2
#define GPIO81_OUT 0x102bB2, BIT0

#define GPIO82_PAD PAD_UART_RX3
#define GPIO82_OEN 0x102bB5, BIT1
#define GPIO82_IN  0x102bB5, BIT2
#define GPIO82_OUT 0x102bB5, BIT0

#define GPIO83_PAD PAD_UART_TX3
#define GPIO83_OEN 0x102bB4, BIT1
#define GPIO83_IN  0x102bB4, BIT2
#define GPIO83_OUT 0x102bB4, BIT0

#define GPIO84_PAD PAD_UART_RX4
#define GPIO84_OEN 0x102bB7, BIT1
#define GPIO84_IN  0x102bB7, BIT2
#define GPIO84_OUT 0x102bB7, BIT0

#define GPIO85_PAD PAD_UART_TX4
#define GPIO85_OEN 0x102bB6, BIT1
#define GPIO85_IN  0x102bB6, BIT2
#define GPIO85_OUT 0x102bB6, BIT0

#define GPIO86_PAD PAD_UART_CTS2
#define GPIO86_OEN 0x102bBD, BIT1
#define GPIO86_IN  0x102bBD, BIT2
#define GPIO86_OUT 0x102bBD, BIT0

#define GPIO87_PAD PAD_UART_RTS2
#define GPIO87_OEN 0x102bBC, BIT1
#define GPIO87_IN  0x102bBC, BIT2
#define GPIO87_OUT 0x102bBC, BIT0

#define GPIO88_PAD PAD_GPIO34
#define GPIO88_OEN 0x102b2B, BIT1
#define GPIO88_IN  0x102b2B, BIT2
#define GPIO88_OUT 0x102b2B, BIT0

#define GPIO89_PAD PAD_GPIO35
#define GPIO89_OEN 0x102b2C, BIT1
#define GPIO89_IN  0x102b2C, BIT2
#define GPIO89_OUT 0x102b2C, BIT0

#define GPIO90_PAD PAD_GPIO36
#define GPIO90_OEN 0x102b2D, BIT1
#define GPIO90_IN  0x102b2D, BIT2
#define GPIO90_OUT 0x102b2D, BIT0

#define GPIO91_PAD PAD_GPIO37
#define GPIO91_OEN 0x102b2E, BIT1
#define GPIO91_IN  0x102b2E, BIT2
#define GPIO91_OUT 0x102b2E, BIT0

#define GPIO92_PAD PAD_GPIO38
#define GPIO92_OEN 0x102b2F, BIT1
#define GPIO92_IN  0x102b2F, BIT2
#define GPIO92_OUT 0x102b2F, BIT0

#define GPIO93_PAD PAD_MPIF_CS1Z
#define GPIO93_OEN 0x102b5D, BIT1
#define GPIO93_IN  0x102b5D, BIT2
#define GPIO93_OUT 0x102b5D, BIT0

#define GPIO94_PAD PAD_MPIF_CS0Z
#define GPIO94_OEN 0x102b5E, BIT1
#define GPIO94_IN  0x102b5E, BIT2
#define GPIO94_OUT 0x102b5E, BIT0

#define GPIO95_PAD PAD_MPIF_D0
#define GPIO95_OEN 0x102b58, BIT1
#define GPIO95_IN  0x102b58, BIT2
#define GPIO95_OUT 0x102b58, BIT0

#define GPIO96_PAD PAD_MPIF_D1
#define GPIO96_OEN 0x102b59, BIT1
#define GPIO96_IN  0x102b59, BIT2
#define GPIO96_OUT 0x102b59, BIT0

#define GPIO97_PAD PAD_MPIF_D2
#define GPIO97_OEN 0x102b5A, BIT1
#define GPIO97_IN  0x102b5A, BIT2
#define GPIO97_OUT 0x102b5A, BIT0

#define GPIO98_PAD PAD_MPIF_D3
#define GPIO98_OEN 0x102b5B, BIT1
#define GPIO98_IN  0x102b5B, BIT2
#define GPIO98_OUT 0x102b5B, BIT0

#define GPIO99_PAD PAD_MPIF_CK
#define GPIO99_OEN 0x102b5F, BIT1
#define GPIO99_IN  0x102b5F, BIT2
#define GPIO99_OUT 0x102b5F, BIT0

#define GPIO100_PAD PAD_MPIF_BUSY
#define GPIO100_OEN 0x102b5C, BIT1
#define GPIO100_IN  0x102b5C, BIT2
#define GPIO100_OUT 0x102b5C, BIT0

#define GPIO101_PAD PAD_MIIC0_SDA
#define GPIO101_OEN 0x102b3B, BIT1
#define GPIO101_IN  0x102b3B, BIT2
#define GPIO101_OUT 0x102b3B, BIT0

#define GPIO102_PAD PAD_MIIC0_SCL
#define GPIO102_OEN 0x102b3C, BIT1
#define GPIO102_IN  0x102b3C, BIT2
#define GPIO102_OUT 0x102b3C, BIT0

#define GPIO103_PAD PAD_GPIO23
#define GPIO103_OEN 0x102b20, BIT1
#define GPIO103_IN  0x102b20, BIT2
#define GPIO103_OUT 0x102b20, BIT0

#define GPIO104_PAD PAD_GPIO22
#define GPIO104_OEN 0x102b1F, BIT1
#define GPIO104_IN  0x102b1F, BIT2
#define GPIO104_OUT 0x102b1F, BIT0

#define GPIO105_PAD PAD_GPIO21
#define GPIO105_OEN 0x102b1E, BIT1
#define GPIO105_IN  0x102b1E, BIT2
#define GPIO105_OUT 0x102b1E, BIT0

#define GPIO106_PAD PAD_GPIO20
#define GPIO106_OEN 0x102b1D, BIT1
#define GPIO106_IN  0x102b1D, BIT2
#define GPIO106_OUT 0x102b1D, BIT0

#define GPIO107_PAD PAD_GPIO19
#define GPIO107_OEN 0x102b1C, BIT1
#define GPIO107_IN  0x102b1C, BIT2
#define GPIO107_OUT 0x102b1C, BIT0

#define GPIO108_PAD PAD_GPIO18
#define GPIO108_OEN 0x102b1B, BIT1
#define GPIO108_IN  0x102b1B, BIT2
#define GPIO108_OUT 0x102b1B, BIT0

#define GPIO109_PAD PAD_GPIO17
#define GPIO109_OEN 0x102b1A, BIT1
#define GPIO109_IN  0x102b1A, BIT2
#define GPIO109_OUT 0x102b1A, BIT0

#define GPIO110_PAD PAD_GPIO16
#define GPIO110_OEN 0x102b19, BIT1
#define GPIO110_IN  0x102b19, BIT2
#define GPIO110_OUT 0x102b19, BIT0

#define GPIO111_PAD PAD_GPIO15
#define GPIO111_OEN 0x102b18, BIT1
#define GPIO111_IN  0x102b18, BIT2
#define GPIO111_OUT 0x102b18, BIT0

#define GPIO112_PAD PAD_GPIO14
#define GPIO112_OEN 0x102b17, BIT1
#define GPIO112_IN  0x102b17, BIT2
#define GPIO112_OUT 0x102b17, BIT0

#define GPIO113_PAD PAD_GPIO13
#define GPIO113_OEN 0x102b16, BIT1
#define GPIO113_IN  0x102b16, BIT2
#define GPIO113_OUT 0x102b16, BIT0

#define GPIO114_PAD PAD_GPIO12
#define GPIO114_OEN 0x102b15, BIT1
#define GPIO114_IN  0x102b15, BIT2
#define GPIO114_OUT 0x102b15, BIT0

#define GPIO115_PAD PAD_GPIO11
#define GPIO115_OEN 0x102b14, BIT1
#define GPIO115_IN  0x102b14, BIT2
#define GPIO115_OUT 0x102b14, BIT0

#define GPIO116_PAD PAD_GPIO10
#define GPIO116_OEN 0x102b13, BIT1
#define GPIO116_IN  0x102b13, BIT2
#define GPIO116_OUT 0x102b13, BIT0

#define GPIO117_PAD PAD_GPIO9
#define GPIO117_OEN 0x102b12, BIT1
#define GPIO117_IN  0x102b12, BIT2
#define GPIO117_OUT 0x102b12, BIT0

#define GPIO118_PAD PAD_GPIO8
#define GPIO118_OEN 0x102b11, BIT1
#define GPIO118_IN  0x102b11, BIT2
#define GPIO118_OUT 0x102b11, BIT0

#define GPIO119_PAD PAD_GPIO7
#define GPIO119_OEN 0x102b10, BIT1
#define GPIO119_IN  0x102b10, BIT2
#define GPIO119_OUT 0x102b10, BIT0

#define GPIO120_PAD PAD_GPIO6
#define GPIO120_OEN 0x102b0F, BIT1
#define GPIO120_IN  0x102b0F, BIT2
#define GPIO120_OUT 0x102b0F, BIT0

#define GPIO121_PAD PAD_GPIO5
#define GPIO121_OEN 0x102b0E, BIT1
#define GPIO121_IN  0x102b0E, BIT2
#define GPIO121_OUT 0x102b0E, BIT0

#define GPIO122_PAD PAD_GPIO4
#define GPIO122_OEN 0x102b0D, BIT1
#define GPIO122_IN  0x102b0D, BIT2
#define GPIO122_OUT 0x102b0D, BIT0

#define GPIO123_PAD PAD_GPIO3
#define GPIO123_OEN 0x102b0C, BIT1
#define GPIO123_IN  0x102b0C, BIT2
#define GPIO123_OUT 0x102b0C, BIT0

#define GPIO124_PAD PAD_GPIO2
#define GPIO124_OEN 0x102b0B, BIT1
#define GPIO124_IN  0x102b0B, BIT2
#define GPIO124_OUT 0x102b0B, BIT0

#define GPIO125_PAD PAD_GPIO1
#define GPIO125_OEN 0x102b0A, BIT1
#define GPIO125_IN  0x102b0A, BIT2
#define GPIO125_OUT 0x102b0A, BIT0

#define GPIO126_PAD PAD_GPIO0
#define GPIO126_OEN 0x102b09, BIT1
#define GPIO126_IN  0x102b09, BIT2
#define GPIO126_OUT 0x102b09, BIT0

#define GPIO127_PAD PAD_TESTPIN
#define GPIO127_OEN 0x0, BIT0
#define GPIO127_IN  0x0, BIT0
#define GPIO127_OUT 0x0, BIT0

#define GPIO128_PAD PAD_SPDIF_OUT
#define GPIO128_OEN 0x102b94, BIT1
#define GPIO128_IN  0x102b94, BIT2
#define GPIO128_OUT 0x102b94, BIT0

#define GPIO129_PAD PAD_IIS_TRX_BCK
#define GPIO129_OEN 0x102b51, BIT1
#define GPIO129_IN  0x102b51, BIT2
#define GPIO129_OUT 0x102b51, BIT0

#define GPIO130_PAD PAD_IIS_TRX_WS
#define GPIO130_OEN 0x102b50, BIT1
#define GPIO130_IN  0x102b50, BIT2
#define GPIO130_OUT 0x102b50, BIT0

#define GPIO131_PAD PAD_IIS_TRX_OUT
#define GPIO131_OEN 0x102b53, BIT1
#define GPIO131_IN  0x102b53, BIT2
#define GPIO131_OUT 0x102b53, BIT0

#define GPIO132_PAD PAD_IIS_TRX_IN
#define GPIO132_OEN 0x102b52, BIT1
#define GPIO132_IN  0x102b52, BIT2
#define GPIO132_OUT 0x102b52, BIT0

#define GPIO133_PAD PAD_UART_RX5
#define GPIO133_OEN 0x102bB9, BIT1
#define GPIO133_IN  0x102bB9, BIT2
#define GPIO133_OUT 0x102bB9, BIT0

#define GPIO134_PAD PAD_UART_TX5
#define GPIO134_OEN 0x102bB8, BIT1
#define GPIO134_IN  0x102bB8, BIT2
#define GPIO134_OUT 0x102bB8, BIT0

#define GPIO135_PAD PAD_UART_CTS1
#define GPIO135_OEN 0x102bBB, BIT1
#define GPIO135_IN  0x102bBB, BIT2
#define GPIO135_OUT 0x102bBB, BIT0

#define GPIO136_PAD PAD_UART_RTS1
#define GPIO136_OEN 0x102bBA, BIT1
#define GPIO136_IN  0x102bBA, BIT2
#define GPIO136_OUT 0x102bBA, BIT0

#define GPIO137_PAD PAD_TCON_GPIO8
#define GPIO137_OEN 0x102bAE, BIT1
#define GPIO137_IN  0x102bAE, BIT2
#define GPIO137_OUT 0x102bAE, BIT0

#define GPIO138_PAD PAD_TCON_GPIO7
#define GPIO138_OEN 0x102bAD, BIT1
#define GPIO138_IN  0x102bAD, BIT2
#define GPIO138_OUT 0x102bAD, BIT0

#define GPIO139_PAD PAD_TCON_GPIO6
#define GPIO139_OEN 0x102bAC, BIT1
#define GPIO139_IN  0x102bAC, BIT2
#define GPIO139_OUT 0x102bAC, BIT0

#define GPIO140_PAD PAD_TCON_GPIO5
#define GPIO140_OEN 0x102bAB, BIT1
#define GPIO140_IN  0x102bAB, BIT2
#define GPIO140_OUT 0x102bAB, BIT0

#define GPIO141_PAD PAD_TCON_GPIO4
#define GPIO141_OEN 0x102bAA, BIT1
#define GPIO141_IN  0x102bAA, BIT2
#define GPIO141_OUT 0x102bAA, BIT0

#define GPIO142_PAD PAD_TCON_GPIO3
#define GPIO142_OEN 0x102bA9, BIT1
#define GPIO142_IN  0x102bA9, BIT2
#define GPIO142_OUT 0x102bA9, BIT0

#define GPIO143_PAD PAD_TCON_GPIO2
#define GPIO143_OEN 0x102bA8, BIT1
#define GPIO143_IN  0x102bA8, BIT2
#define GPIO143_OUT 0x102bA8, BIT0

#define GPIO144_PAD PAD_TCON_GPIO1
#define GPIO144_OEN 0x102bA7, BIT1
#define GPIO144_IN  0x102bA7, BIT2
#define GPIO144_OUT 0x102bA7, BIT0

#define GPIO145_PAD PAD_TCON_GPIO0
#define GPIO145_OEN 0x102bA6, BIT1
#define GPIO145_IN  0x102bA6, BIT2
#define GPIO145_OUT 0x102bA6, BIT0

#define GPIO146_PAD PAD_TTL_GPIO11
#define GPIO146_OEN 0x102b49, BIT1
#define GPIO146_IN  0x102b49, BIT2
#define GPIO146_OUT 0x102b49, BIT0

#define GPIO147_PAD PAD_TTL_GPIO10
#define GPIO147_OEN 0x102b48, BIT1
#define GPIO147_IN  0x102b48, BIT2
#define GPIO147_OUT 0x102b48, BIT0

#define GPIO148_PAD PAD_TTL_GPIO9
#define GPIO148_OEN 0x102b47, BIT1
#define GPIO148_IN  0x102b47, BIT2
#define GPIO148_OUT 0x102b47, BIT0

#define GPIO149_PAD PAD_TTL_GPIO8
#define GPIO149_OEN 0x102b46, BIT1
#define GPIO149_IN  0x102b46, BIT2
#define GPIO149_OUT 0x102b46, BIT0

#define GPIO150_PAD PAD_TTL_GPIO7
#define GPIO150_OEN 0x102b45, BIT1
#define GPIO150_IN  0x102b45, BIT2
#define GPIO150_OUT 0x102b45, BIT0

#define GPIO151_PAD PAD_TTL_GPIO6
#define GPIO151_OEN 0x102b44, BIT1
#define GPIO151_IN  0x102b44, BIT2
#define GPIO151_OUT 0x102b44, BIT0

#define GPIO152_PAD PAD_TTL_GPIO5
#define GPIO152_OEN 0x102b43, BIT1
#define GPIO152_IN  0x102b43, BIT2
#define GPIO152_OUT 0x102b43, BIT0

#define GPIO153_PAD PAD_TTL_GPIO4
#define GPIO153_OEN 0x102b42, BIT1
#define GPIO153_IN  0x102b42, BIT2
#define GPIO153_OUT 0x102b42, BIT0

#define GPIO154_PAD PAD_TTL_GPIO3
#define GPIO154_OEN 0x102b41, BIT1
#define GPIO154_IN  0x102b41, BIT2
#define GPIO154_OUT 0x102b41, BIT0

#define GPIO155_PAD PAD_TTL_GPIO2
#define GPIO155_OEN 0x102b40, BIT1
#define GPIO155_IN  0x102b40, BIT2
#define GPIO155_OUT 0x102b40, BIT0

#define GPIO156_PAD PAD_TTL_GPIO1
#define GPIO156_OEN 0x102b3F, BIT1
#define GPIO156_IN  0x102b3F, BIT2
#define GPIO156_OUT 0x102b3F, BIT0

#define GPIO157_PAD PAD_TTL_GPIO0
#define GPIO157_OEN 0x102b3E, BIT1
#define GPIO157_IN  0x102b3E, BIT2
#define GPIO157_OUT 0x102b3E, BIT0

#define GPIO158_PAD PAD_NAND_WPZ
#define GPIO158_OEN 0x102b6A, BIT1
#define GPIO158_IN  0x102b6A, BIT2
#define GPIO158_OUT 0x102b6A, BIT0

#define GPIO159_PAD PAD_NAND_WEZ
#define GPIO159_OEN 0x102b72, BIT1
#define GPIO159_IN  0x102b72, BIT2
#define GPIO159_OUT 0x102b72, BIT0

#define GPIO160_PAD PAD_NAND_ALE
#define GPIO160_OEN 0x102b70, BIT1
#define GPIO160_IN  0x102b70, BIT2
#define GPIO160_OUT 0x102b70, BIT0

#define GPIO161_PAD PAD_NAND_CLE
#define GPIO161_OEN 0x102b71, BIT1
#define GPIO161_IN  0x102b71, BIT2
#define GPIO161_OUT 0x102b71, BIT0

#define GPIO162_PAD PAD_NAND_CE3Z
#define GPIO162_OEN 0x102b6F, BIT1
#define GPIO162_IN  0x102b6F, BIT2
#define GPIO162_OUT 0x102b6F, BIT0

#define GPIO163_PAD PAD_NAND_CE2Z
#define GPIO163_OEN 0x102b6E, BIT1
#define GPIO163_IN  0x102b6E, BIT2
#define GPIO163_OUT 0x102b6E, BIT0

#define GPIO164_PAD PAD_NAND_CE1Z
#define GPIO164_OEN 0x102b6D, BIT1
#define GPIO164_IN  0x102b6D, BIT2
#define GPIO164_OUT 0x102b6D, BIT0

#define GPIO165_PAD PAD_NAND_CE0Z
#define GPIO165_OEN 0x102b6C, BIT1
#define GPIO165_IN  0x102b6C, BIT2
#define GPIO165_OUT 0x102b6C, BIT0

#define GPIO166_PAD PAD_NAND_REZ
#define GPIO166_OEN 0x102b73, BIT1
#define GPIO166_IN  0x102b73, BIT2
#define GPIO166_OUT 0x102b73, BIT0

#define GPIO167_PAD PAD_NAND_RBZ
#define GPIO167_OEN 0x102b6B, BIT1
#define GPIO167_IN  0x102b6B, BIT2
#define GPIO167_OUT 0x102b6B, BIT0

#define GPIO168_PAD PAD_NAND_DA0
#define GPIO168_OEN 0x102b62, BIT1
#define GPIO168_IN  0x102b62, BIT2
#define GPIO168_OUT 0x102b62, BIT0

#define GPIO169_PAD PAD_NAND_DA1
#define GPIO169_OEN 0x102b63, BIT1
#define GPIO169_IN  0x102b63, BIT2
#define GPIO169_OUT 0x102b63, BIT0

#define GPIO170_PAD PAD_NAND_DA2
#define GPIO170_OEN 0x102b64, BIT1
#define GPIO170_IN  0x102b64, BIT2
#define GPIO170_OUT 0x102b64, BIT0

#define GPIO171_PAD PAD_NAND_DA3
#define GPIO171_OEN 0x102b65, BIT1
#define GPIO171_IN  0x102b65, BIT2
#define GPIO171_OUT 0x102b65, BIT0

#define GPIO172_PAD PAD_NAND_DQS
#define GPIO172_OEN 0x102b74, BIT1
#define GPIO172_IN  0x102b74, BIT2
#define GPIO172_OUT 0x102b74, BIT0

#define GPIO173_PAD PAD_NAND_DA4
#define GPIO173_OEN 0x102b66, BIT1
#define GPIO173_IN  0x102b66, BIT2
#define GPIO173_OUT 0x102b66, BIT0

#define GPIO174_PAD PAD_NAND_DA5
#define GPIO174_OEN 0x102b67, BIT1
#define GPIO174_IN  0x102b67, BIT2
#define GPIO174_OUT 0x102b67, BIT0

#define GPIO175_PAD PAD_NAND_DA6
#define GPIO175_OEN 0x102b68, BIT1
#define GPIO175_IN  0x102b68, BIT2
#define GPIO175_OUT 0x102b68, BIT0

#define GPIO176_PAD PAD_NAND_DA7
#define GPIO176_OEN 0x102b69, BIT1
#define GPIO176_IN  0x102b69, BIT2
#define GPIO176_OUT 0x102b69, BIT0

#define GPIO177_PAD PAD_SD2_D1
#define GPIO177_OEN 0x102b86, BIT1
#define GPIO177_IN  0x102b86, BIT2
#define GPIO177_OUT 0x102b86, BIT0

#define GPIO178_PAD PAD_SD2_D0
#define GPIO178_OEN 0x102b85, BIT1
#define GPIO178_IN  0x102b85, BIT2
#define GPIO178_OUT 0x102b85, BIT0

#define GPIO179_PAD PAD_SD2_CLK
#define GPIO179_OEN 0x102b88, BIT1
#define GPIO179_IN  0x102b88, BIT2
#define GPIO179_OUT 0x102b88, BIT0

#define GPIO180_PAD PAD_SD2_CMD
#define GPIO180_OEN 0x102b87, BIT1
#define GPIO180_IN  0x102b87, BIT2
#define GPIO180_OUT 0x102b87, BIT0

#define GPIO181_PAD PAD_SD2_D3
#define GPIO181_OEN 0x102b8A, BIT1
#define GPIO181_IN  0x102b8A, BIT2
#define GPIO181_OUT 0x102b8A, BIT0

#define GPIO182_PAD PAD_SD2_D2
#define GPIO182_OEN 0x102b89, BIT1
#define GPIO182_IN  0x102b89, BIT2
#define GPIO182_OUT 0x102b89, BIT0







//-------------------------------------------------------------------------------------------------
//  Local Structures
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Variables
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
static const struct gpio_setting
{
    U32 r_oen;
    U8  m_oen;
    U32 r_out;
    U8  m_out;
    U32 r_in;
    U8  m_in;
} gpio_table[] =
{
#define __GPIO__(_x_)   { CONCAT(CONCAT(GPIO, _x_), _OEN),   \
                          CONCAT(CONCAT(GPIO, _x_), _OUT),   \
                          CONCAT(CONCAT(GPIO, _x_), _IN) }
#define __GPIO(_x_)     __GPIO__(_x_)

//
// !! WARNING !! DO NOT MODIFIY !!!!
//
// These defines order must match following
// 1. the PAD name in GPIO excel
// 2. the perl script to generate the package header file
//
    //__GPIO(999), // 0 is not used

    __GPIO(0), __GPIO(1), __GPIO(2), __GPIO(3), __GPIO(4),
    __GPIO(5), __GPIO(6), __GPIO(7), __GPIO(8), __GPIO(9),
    __GPIO(10), __GPIO(11), __GPIO(12), __GPIO(13), __GPIO(14),
    __GPIO(15), __GPIO(16), __GPIO(17), __GPIO(18), __GPIO(19),
    __GPIO(20), __GPIO(21), __GPIO(22), __GPIO(23), __GPIO(24),
    __GPIO(25), __GPIO(26), __GPIO(27), __GPIO(28), __GPIO(29),
    __GPIO(30), __GPIO(31), __GPIO(32), __GPIO(33), __GPIO(34),
    __GPIO(35), __GPIO(36), __GPIO(37), __GPIO(38), __GPIO(39),
    __GPIO(40), __GPIO(41), __GPIO(42), __GPIO(43), __GPIO(44),
    __GPIO(45), __GPIO(46), __GPIO(47), __GPIO(48), __GPIO(49),
    __GPIO(50), __GPIO(51), __GPIO(52), __GPIO(53), __GPIO(54),
    __GPIO(55), __GPIO(56), __GPIO(57), __GPIO(58), __GPIO(59),
    __GPIO(60), __GPIO(61), __GPIO(62), __GPIO(63), __GPIO(64),
    __GPIO(65), __GPIO(66), __GPIO(67), __GPIO(68), __GPIO(69),
    __GPIO(70), __GPIO(71), __GPIO(72), __GPIO(73), __GPIO(74),
    __GPIO(75), __GPIO(76), __GPIO(77), __GPIO(78), __GPIO(79),
    __GPIO(80), __GPIO(81), __GPIO(82), __GPIO(83), __GPIO(84),
    __GPIO(85), __GPIO(86), __GPIO(87), __GPIO(88), __GPIO(89),
    __GPIO(90), __GPIO(91), __GPIO(92), __GPIO(93), __GPIO(94),
    __GPIO(95), __GPIO(96), __GPIO(97), __GPIO(98), __GPIO(99),
    __GPIO(100), __GPIO(101), __GPIO(102), __GPIO(103), __GPIO(104),
    __GPIO(105), __GPIO(106), __GPIO(107), __GPIO(108), __GPIO(109),
    __GPIO(110), __GPIO(111), __GPIO(112), __GPIO(113), __GPIO(114),
    __GPIO(115), __GPIO(116), __GPIO(117), __GPIO(118), __GPIO(119),
    __GPIO(120), __GPIO(121), __GPIO(122), __GPIO(123), __GPIO(124),
    __GPIO(125), __GPIO(126), __GPIO(127), __GPIO(128), __GPIO(129),
    __GPIO(130), __GPIO(131), __GPIO(132), __GPIO(133), __GPIO(134),
    __GPIO(135), __GPIO(136), __GPIO(137), __GPIO(138), __GPIO(139),
    __GPIO(140), __GPIO(141), __GPIO(142), __GPIO(143), __GPIO(144),
    __GPIO(145), __GPIO(146), __GPIO(147), __GPIO(148), __GPIO(149),
    __GPIO(150), __GPIO(151), __GPIO(152), __GPIO(153), __GPIO(154),
    __GPIO(155), __GPIO(156), __GPIO(157), __GPIO(158), __GPIO(159),
    __GPIO(160), __GPIO(161), __GPIO(162), __GPIO(163), __GPIO(164),
    __GPIO(165), __GPIO(166), __GPIO(167), __GPIO(168), __GPIO(169),
    __GPIO(170), __GPIO(171), __GPIO(172), __GPIO(173), __GPIO(174),
    __GPIO(175), __GPIO(176), __GPIO(177), __GPIO(178), __GPIO(179),
    __GPIO(180), __GPIO(181), __GPIO(182),

};
#endif

#define MS_INT_BASE GET_REG_ADDR(MS_BASE_REG_RIU_PA,0x80C80)
#define MS_BACH1_BASE GET_REG_ADDR(MS_BASE_REG_RIU_PA,0x89580)


//-------------------------------------------------------------------------------------------------
//  Debug Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Local Functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

//the functions of this section set to initialize
void MHal_GPIO_Init(void)
{
    MHal_GPIO_REG(REG_ALL_PAD_IN) &= ~BIT0;
}

void MHal_GPIO_WriteRegBit(U32 u32Reg, U8 u8Enable, U8 u8BitMsk)
{
    if(u8Enable)
        MHal_GPIO_REG(u32Reg) |= u8BitMsk;
    else
        MHal_GPIO_REG(u32Reg) &= (~u8BitMsk);
}

U8 MHal_GPIO_ReadRegBit(U32 u32Reg, U8 u8BitMsk)
{
    return ((MHal_GPIO_REG(u32Reg)&u8BitMsk)? 1 : 0);
}

void MHal_GPIO_PAD_32K_OUT(U8 u8Enable)
{
	if (u8Enable)
		MHal_GPIO_WriteRegBit(REG_PMGPIO_01,ENABLE,BIT3);
	else
		MHal_GPIO_WriteRegBit(REG_PMGPIO_01,DISABLE,BIT3);
}
void MHal_GPIO_Pad_Set(U8 u8IndexGPIO)
{
    switch(u8IndexGPIO)
    {
    case PAD_PM_GPIO0:
    case PAD_PM_GPIO1:
        MHal_GPIO_WriteRegBit(REG_PMGPIO_01,DISABLE,BIT0);
        break;

    case PAD_PM_IIC_SDA:
    case PAD_PM_IIC_SCL:
        MHal_GPIO_WriteRegBit(REG_PMGPIO_00,DISABLE,BIT2);
        break;

    case PAD_PM_MIIC_SDA:
    case PAD_PM_MIIC_SCL:
        MHal_GPIO_WriteRegBit(REG_PMGPIO_00,DISABLE,BIT4);
        break;

    case PAD_PM_SPI_CK:
    case PAD_PM_SPI_CZ0:
    case PAD_PM_SPI_GPIO:
    case PAD_PM_SPI_DI:
    case PAD_PM_SPI_DO:
        MHal_GPIO_WriteRegBit(REG_PMGPIO_00,DISABLE,BIT5|BIT6|BIT7);
        break;

    case PAD_ONOFF:
        MHal_GPIO_WriteRegBit(REG_PMGPIO_01,DISABLE,BIT1);
        break;

    case PAD_CHRGDET:
        MHal_GPIO_WriteRegBit(REG_PMGPIO_01,DISABLE,BIT2);
        break;

    case PAD_32K_OUT:
        MHal_GPIO_WriteRegBit(REG_PMGPIO_01,DISABLE,BIT3);
        break;

    case PAD_IRIN:
        MHal_GPIO_WriteRegBit(REG_PMGPIO_00,DISABLE,BIT3);
        break;

    case PAD_CEC:
        MHal_GPIO_WriteRegBit(REG_PMGPIO_00,DISABLE,BIT1);
        break;

    case PAD_PM_SD0_CDZ:
        MHal_GPIO_WriteRegBit(REG_PMGPIO_01,DISABLE,BIT4);
        break;
		
    case PAD_PM_SD1_CDZ:
        MHal_GPIO_WriteRegBit(REG_PMGPIO_01,DISABLE,BIT5);
        break;
		
    case PAD_PM_SD2_CDZ:
        MHal_GPIO_WriteRegBit(REG_PMGPIO_01,DISABLE,BIT6);
        break;

    case PAD_SD0_D1:
    case PAD_SD0_D0:
    case PAD_SD0_CLK:
    case PAD_SD0_CMD:
    case PAD_SD0_D3:
    case PAD_SD0_D2:
        MHal_GPIO_WriteRegBit(REG_GPIO1_04,DISABLE,BIT0);
        break;

    case PAD_SD1_D1:
    case PAD_SD1_D0:
    case PAD_SD1_CLK:
    case PAD_SD1_CMD:
    case PAD_SD1_D3:
    case PAD_SD1_D2:
        MHal_GPIO_WriteRegBit(REG_GPIO1_04,DISABLE,BIT2|BIT3);
        break;

    case PAD_A_GPS_EVENT:
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT6);
        break;

    case PAD_AFE1_SGN:
    case PAD_AFE1_MAG:
    case PAD_RFSPI_CZ1:
        MHal_GPIO_WriteRegBit(REG_GPIO1_2A,DISABLE,BIT6);
        break;

    case PAD_AFE0_SGN:
    case PAD_AFE0_MAG:
    case PAD_RFSPI_CZ0:
        MHal_GPIO_WriteRegBit(REG_GPIO1_2A,DISABLE,BIT7);
        break;

    case PAD_RFSPI_CLK:
    case PAD_RFSPI_DATA:
        MHal_GPIO_WriteRegBit(REG_GPIO1_2A,DISABLE,BIT6|BIT7);
        break;

    case PAD_BTI_RX_TX:
    case PAD_BTI_DATA1:
    case PAD_BTI_DATA0:
    case PAD_BT_CLK_24M:
    case PAD_BT_SPI_SCLK:
    case PAD_BT_SPI_SDATA:
    case PAD_BT_SPI_SENB:
    case PAD_BT_CHIP_EN:
        MHal_GPIO_WriteRegBit(REG_GPIO1_00,DISABLE,BIT1);
        break;

    case PAD_SR_D0:
    case PAD_SR_D1:
    case PAD_SR_D2:
    case PAD_SR_D3:
    case PAD_SR_D4:
    case PAD_SR_D5:
    case PAD_SR_D6:
    case PAD_SR_D7:
    case PAD_SR_PCLK:
        MHal_GPIO_WriteRegBit(REG_GPIO1_05,DISABLE,BIT0);
        MHal_GPIO_WriteRegBit(REG_GPIO1_00,DISABLE,BIT4);
        break;

    case PAD_SR_HSYNC:
    case PAD_SR_STROBE:
    case PAD_SR_VSYNC:
    case PAD_SR_RST:
    case PAD_SR_PWRDN:
        MHal_GPIO_WriteRegBit(REG_GPIO1_05,DISABLE,BIT0);
        break;

    case PAD_GPIO33:
    case PAD_GPIO32:
        MHal_GPIO_WriteRegBit(REG_GPIO1_05,DISABLE,BIT1);
        break;

    case PAD_GPIO31:
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT3);
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT4);
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,DISABLE,BIT7);
        break;

    case PAD_GPIO30:
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT3);
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT4);
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,DISABLE,BIT6);
        break;

    case PAD_GPIO29:
        MHal_GPIO_WriteRegBit(REG_GPIO1_21,DISABLE,BIT2);
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,DISABLE,BIT5);
        break;

    case PAD_GPIO28:
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,DISABLE,BIT4);
		OUTREGMSK16(GET_REG_ADDR(MS_BACH1_BASE, 0x16),0x01,0x01);
        break;

    case PAD_GPIO27:
        MHal_GPIO_WriteRegBit(REG_GPIO1_00,DISABLE,BIT2|BIT3|BIT5|BIT6);
        MHal_GPIO_WriteRegBit(REG_GPIO1_01,DISABLE,BIT3|BIT4);
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT2|BIT3);
        break;

    case PAD_GPIO26:
        MHal_GPIO_WriteRegBit(REG_GPIO1_00,DISABLE,BIT2|BIT3);
        MHal_GPIO_WriteRegBit(REG_GPIO1_01,DISABLE,BIT3|BIT4);
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT2|BIT3);
        break;

    case PAD_GPIO25:
        MHal_GPIO_WriteRegBit(REG_GPIO1_00,DISABLE,BIT2|BIT3|BIT5|BIT6);
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT2|BIT3);
        break;

    case PAD_GPIO24:
        MHal_GPIO_WriteRegBit(REG_GPIO1_00,DISABLE,BIT2|BIT3|BIT5|BIT6);
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT2|BIT3);
        MHal_GPIO_WriteRegBit(REG_GPIO1_01,DISABLE,BIT3|BIT4);
        break;

    case PAD_UART_RX1:
    case PAD_UART_TX1:
        MHal_GPIO_WriteRegBit(REG_GPIO1_06,DISABLE,BIT2);
        break;

    case PAD_UART_RX2:
    case PAD_UART_TX2:
        MHal_GPIO_WriteRegBit(REG_GPIO1_06,DISABLE,BIT3);
        break;

    case PAD_UART_RX3:
    case PAD_UART_TX3:
        MHal_GPIO_WriteRegBit(REG_GPIO1_06,DISABLE,BIT4);
        break;

    case PAD_UART_RX4:
    case PAD_UART_TX4:
        MHal_GPIO_WriteRegBit(REG_GPIO1_06,DISABLE,BIT5);
        break;

    case PAD_UART_CTS2:
    case PAD_UART_RTS2:
        MHal_GPIO_WriteRegBit(REG_GPIO1_01,DISABLE,BIT6);
        break;

    case PAD_MPIF_CS1Z:
        MHal_GPIO_WriteRegBit(REG_GPIO1_03,DISABLE,BIT0);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT6);
        break;

    case PAD_MPIF_CS0Z:
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT4|BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT4|BIT6);
        break;

    case PAD_MPIF_D0:
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT4|BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_05,DISABLE,BIT7);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT6);
        break;

    case PAD_MPIF_D1:
    case PAD_MPIF_D2:
    case PAD_MPIF_D3:
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT4|BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT6);
        break;

    case PAD_MPIF_CK:
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT4|BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_05,DISABLE,BIT7);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT6);
        break;

    case PAD_MPIF_BUSY:
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT4|BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_05,DISABLE,BIT7);
        break;

    case PAD_MIIC0_SDA:
    case PAD_MIIC0_SCL:
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT2);
        break;

    case PAD_GPIO23:
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT6);
        break;

    case PAD_GPIO13:
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,DISABLE,BIT3);
        break;

    case PAD_GPIO10:
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT0);
        break;

    case PAD_GPIO9:
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,DISABLE,BIT2);
        break;

    case PAD_GPIO8:
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,DISABLE,BIT1);
        break;

    case PAD_GPIO7:
        MHal_GPIO_WriteRegBit(REG_GPIO1_26,DISABLE,BIT7);
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,DISABLE,BIT0);
        break;

    case PAD_GPIO6:
        MHal_GPIO_WriteRegBit(REG_GPIO1_26,DISABLE,BIT6);
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT1);
        break;

    case PAD_GPIO5:
        MHal_GPIO_WriteRegBit(REG_GPIO1_26,DISABLE,BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2A,DISABLE,BIT5);
        break;

    case PAD_GPIO4:
        MHal_GPIO_WriteRegBit(REG_GPIO1_26,DISABLE,BIT4);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2A,DISABLE,BIT4);
        break;

    case PAD_GPIO3:
        MHal_GPIO_WriteRegBit(REG_GPIO1_26,DISABLE,BIT3);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2A,DISABLE,BIT3);
        break;

    case PAD_GPIO2:
        MHal_GPIO_WriteRegBit(REG_GPIO1_26,DISABLE,BIT2);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2A,DISABLE,BIT2);
        break;

    case PAD_GPIO1:
        MHal_GPIO_WriteRegBit(REG_GPIO1_26,DISABLE,BIT1);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2A,DISABLE,BIT1);
        break;

    case PAD_GPIO0:
        MHal_GPIO_WriteRegBit(REG_GPIO1_26,DISABLE,BIT0);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2A,DISABLE,BIT0);
        break;

    case PAD_SPDIF_OUT:
        MHal_GPIO_WriteRegBit(REG_GPIO1_05,DISABLE,BIT6);
        break;

    case PAD_IIS_TRX_BCK:
        MHal_GPIO_WriteRegBit(REG_GPIO1_00,DISABLE,BIT2|BIT3|BIT5|BIT6);
        MHal_GPIO_WriteRegBit(REG_GPIO1_01,DISABLE,BIT3|BIT4);
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT2|BIT3);
        break;

    case PAD_IIS_TRX_WS:
        MHal_GPIO_WriteRegBit(REG_GPIO1_00,DISABLE,BIT2|BIT3);
        MHal_GPIO_WriteRegBit(REG_GPIO1_01,DISABLE,BIT3|BIT4);
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT2|BIT3);
        break;

    case PAD_IIS_TRX_OUT:
        MHal_GPIO_WriteRegBit(REG_GPIO1_00,DISABLE,BIT2|BIT3|BIT5|BIT6);
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT2|BIT3);
        break;

    case PAD_IIS_TRX_IN:
        MHal_GPIO_WriteRegBit(REG_GPIO1_00,DISABLE,BIT2|BIT3|BIT5|BIT6);
        MHal_GPIO_WriteRegBit(REG_GPIO1_01,DISABLE,BIT3|BIT4);
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT2|BIT3);
        break;

    case PAD_UART_RX5:
    case PAD_UART_TX5:
        MHal_GPIO_WriteRegBit(REG_GPIO1_06,DISABLE,BIT6);
        break;

    case PAD_UART_CTS1:
    case PAD_UART_RTS1:
        MHal_GPIO_WriteRegBit(REG_GPIO1_01,DISABLE,BIT5);
        break;

    case PAD_TCON_GPIO7:
        MHal_GPIO_WriteRegBit(REG_GPIO1_22,DISABLE,BIT7);
        break;

    case PAD_TCON_GPIO5:
        MHal_GPIO_WriteRegBit(REG_GPIO1_22,DISABLE,BIT0);
        break;

    case PAD_TCON_GPIO3:
        MHal_GPIO_WriteRegBit(REG_GPIO1_22,DISABLE,BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT2);
        break;

    case PAD_TCON_GPIO2:
        MHal_GPIO_WriteRegBit(REG_GPIO1_22,DISABLE,BIT6);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT2);
        break;

    case PAD_TCON_GPIO1:
        MHal_GPIO_WriteRegBit(REG_GPIO1_22,DISABLE,BIT1);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT2);
        break;

    case PAD_TCON_GPIO0:
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT3);
        break;

    case PAD_TTL_GPIO11:
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT5);
        break;

    case PAD_TTL_GPIO10:
        MHal_GPIO_WriteRegBit(REG_GPIO1_22,DISABLE,BIT2);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT5);
        break;

    case PAD_TTL_GPIO9:
    case PAD_TTL_GPIO8:
    case PAD_TTL_GPIO7:
    case PAD_TTL_GPIO6:
    case PAD_TTL_GPIO5:
    case PAD_TTL_GPIO4:
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT5);
        break;

    case PAD_TTL_GPIO3:
        MHal_GPIO_WriteRegBit(REG_GPIO1_22,DISABLE,BIT4);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT5);
        break;

    case PAD_TTL_GPIO2:
        MHal_GPIO_WriteRegBit(REG_GPIO1_22,DISABLE,BIT3);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT5);
        break;

    case PAD_TTL_GPIO1:
    case PAD_TTL_GPIO0:
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT5);
        break;

    case PAD_NAND_WPZ:
    case PAD_NAND_WEZ:
    case PAD_NAND_ALE:
    case PAD_NAND_CLE:
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT6);
        break;

    case PAD_NAND_CE3Z:
        MHal_GPIO_WriteRegBit(REG_GPIO1_21,DISABLE,BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT1);
        break;

    case PAD_NAND_CE2Z:
        MHal_GPIO_WriteRegBit(REG_GPIO1_2C,DISABLE,BIT0);
        break;

    case PAD_NAND_CE1Z:
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT7);
        break;

    case PAD_NAND_CE0Z:
    case PAD_NAND_REZ:
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT6);
        break;

    case PAD_NAND_RBZ:
        MHal_GPIO_WriteRegBit(REG_GPIO1_21,DISABLE,BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT6);
        break;

    case PAD_NAND_DA0:
    case PAD_NAND_DA1:
    case PAD_NAND_DA2:
    case PAD_NAND_DA3:
        MHal_GPIO_WriteRegBit(REG_GPIO1_21,DISABLE,BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT6);
        break;

    case PAD_NAND_DQS:
        MHal_GPIO_WriteRegBit(REG_GPIO1_21,DISABLE,BIT5);
        break;

    case PAD_NAND_DA4:
    case PAD_NAND_DA5:
    case PAD_NAND_DA6:
    case PAD_NAND_DA7:
        MHal_GPIO_WriteRegBit(REG_GPIO1_21,DISABLE,BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT6);
        break;

    case PAD_SD2_D1:
    case PAD_SD2_D0:
    case PAD_SD2_CLK:
    case PAD_SD2_CMD:
    case PAD_SD2_D3:
    case PAD_SD2_D2:
        MHal_GPIO_WriteRegBit(REG_GPIO1_04,DISABLE,BIT4);
        break;

    case PAD_PM_SD_GPIO:
    case PAD_HOTPLUG:
    case PAD_BATOK:
    case PAD_PMIC_CHIPEN:
    case PAD_PMIC_STDBYN:
    case PAD_PMIC_INT:
    case PAD_GPIO34:
    case PAD_GPIO35:
    case PAD_GPIO36:
    case PAD_GPIO37:
    case PAD_GPIO38:
    case PAD_GPIO22:
    case PAD_GPIO21:
    case PAD_GPIO20:
    case PAD_GPIO19:
    case PAD_GPIO18:
    case PAD_GPIO17:
    case PAD_GPIO16:
    case PAD_GPIO15:
    case PAD_GPIO14:
    case PAD_GPIO12:
    case PAD_GPIO11:
    case PAD_TESTPIN:
    case PAD_TCON_GPIO8:
    case PAD_TCON_GPIO6:
    case PAD_TCON_GPIO4:
    default:
        break;
    }
}
void MHal_GPIO_Pad_Oen(U8 u8IndexGPIO)
{

    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_oen) &= (~gpio_table[u8IndexGPIO].m_oen);
}

void MHal_GPIO_Pad_Odn(U8 u8IndexGPIO)
{
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_oen) |= gpio_table[u8IndexGPIO].m_oen;
}

U8 MHal_GPIO_Pad_Level(U8 u8IndexGPIO)
{
    return ((MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_in)&gpio_table[u8IndexGPIO].m_in)? 1 : 0);
}

U8 MHal_GPIO_Pad_InOut(U8 u8IndexGPIO)
{
    return ((MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_oen)&gpio_table[u8IndexGPIO].m_oen)? 1 : 0);
}

void MHal_GPIO_Pull_High(U8 u8IndexGPIO)
{
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_out) |= gpio_table[u8IndexGPIO].m_out;
}

void MHal_GPIO_Pull_Low(U8 u8IndexGPIO)
{
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_out) &= (~gpio_table[u8IndexGPIO].m_out);
}

void MHal_GPIO_Set_High(U8 u8IndexGPIO)
{
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_oen) &= (~gpio_table[u8IndexGPIO].m_oen);
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_out) |= gpio_table[u8IndexGPIO].m_out;
}

void MHal_GPIO_Set_Low(U8 u8IndexGPIO)
{
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_oen) &= (~gpio_table[u8IndexGPIO].m_oen);
    MHal_GPIO_REG(gpio_table[u8IndexGPIO].r_out) &= (~gpio_table[u8IndexGPIO].m_out);
}

void MHal_Enable_GPIO_INT(U8 u8IndexGPIO)
{
    switch(u8IndexGPIO)
    {
    case PAD_GPIO7:
        MHal_GPIO_WriteRegBit(REG_GPIO1_26,DISABLE,BIT7);
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,ENABLE,BIT0);
        break;
    case PAD_GPIO8:
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,ENABLE,BIT1);
        break;
    case PAD_GPIO9:
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,ENABLE,BIT2);
        break;
    case PAD_GPIO13:
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,ENABLE,BIT3);
        break;
    case PAD_GPIO28:
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,ENABLE,BIT4);
        break;
    case PAD_GPIO29:
        MHal_GPIO_WriteRegBit(REG_GPIO1_21,DISABLE,BIT2);
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,ENABLE,BIT5);
        break;
    case PAD_GPIO30:
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT3);
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT4);
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,ENABLE,BIT6);
        break;
    case PAD_GPIO31:
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT5);
        MHal_GPIO_WriteRegBit(REG_GPIO1_02,DISABLE,BIT3);
        MHal_GPIO_WriteRegBit(REG_GPIO1_20,DISABLE,BIT4);
        MHal_GPIO_WriteRegBit(REG_GPIO1_25,ENABLE,BIT7);
        break;
    default:
        break;
    }
}

void MHal_GPIO_Set_POLARITY(U8 u8IndexGPIO,U8 reverse)
{
/*
IdxGPIO  GPIOx  IdxFIQ
70  --GPIO31 -- 63 -- ext_gpio_int[7]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
71  --GPIO30 -- 58 -- ext_gpio_int[6]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
72  --GPIO29 -- 57 -- ext_gpio_int[5]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
73  -- GPIO28 -- 56 -- ext_gpio_int[4]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
113 --GPIO13 -- 55 -- ext_gpio_int[3]  -- reg_hst0_fiq_polarity_63_48_  -- [h00b,h00b]
117 --GPIO9   -- 47 -- ext_gpio_int[2]  -- reg_hst0_fiq_polarity_47_32_  -- [h00a,h00a]
118 --GPIO8   -- 43 -- ext_gpio_int[1]  -- reg_hst0_fiq_polarity_47_32_  -- [h00a,h00a]
119 --GPIO7   -- 39 -- ext_gpio_int[0]  -- reg_hst0_fiq_polarity_47_32_  -- [h00a,h00a]
*/

    switch(u8IndexGPIO)
    {
    case 119:  //INT_FIQ_EXT_GPIO0
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 7); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 7); //Set To Raising edge trigger
		break;
    case 118:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 11); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 11); //Set To Raising edge trigger
		break;
    case 117:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 15); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0a), 1 << 15); //Set To Raising edge trigger
		break;
	case 113:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 7); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 7); //Set To Raising edge trigger
		break;		
    case 73:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 8); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 8); //Set To Raising edge trigger
		break;	
    case 72:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 9); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 9); //Set To Raising edge trigger
		break;	
    case 71:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 10); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 10); //Set To Raising edge trigger
		break;	
    case 70:
        if(reverse==0)
            SETREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 15); //Set To Falling edge trigger
        else
            CLRREG16(GET_REG_ADDR(MS_INT_BASE, 0x0b), 1 << 15); //Set To Raising edge trigger
		break;	
    default:
        break;
    }
}



