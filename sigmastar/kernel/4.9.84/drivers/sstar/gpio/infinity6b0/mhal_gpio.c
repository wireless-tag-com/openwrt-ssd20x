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
#include <linux/kernel.h>
#include <linux/irqdomain.h>
#include <linux/irq.h>
#include "mhal_gpio.h"
#include "ms_platform.h"
#include "gpio.h"
#include "irqs.h"
#include "padmux.h"
#include "mhal_pinmux.h"
#include <dt-bindings/interrupt-controller/arm-gic.h>

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

#define _CONCAT( a, b )     a##b
#define CONCAT( a, b )      _CONCAT( a, b )

// PADTOP
#define GPIO0_PAD PAD_GPIO0
#define GPIO0_OEN 0x103C00, BIT5
#define GPIO0_OUT 0x103C00, BIT4
#define GPIO0_IN  0x103C00, BIT0

#define GPIO1_PAD PAD_GPIO1
#define GPIO1_OEN 0x103C02, BIT5
#define GPIO1_OUT 0x103C02, BIT4
#define GPIO1_IN  0x103C02, BIT0

#define GPIO2_PAD PAD_GPIO2
#define GPIO2_OEN 0x103C04, BIT5
#define GPIO2_OUT 0x103C04, BIT4
#define GPIO2_IN  0x103C04, BIT0

#define GPIO3_PAD PAD_GPIO3
#define GPIO3_OEN 0x103C06, BIT5
#define GPIO3_OUT 0x103C06, BIT4
#define GPIO3_IN  0x103C06, BIT0

#define GPIO4_PAD PAD_GPIO4
#define GPIO4_OEN 0x103C08, BIT5
#define GPIO4_OUT 0x103C08, BIT4
#define GPIO4_IN  0x103C08, BIT0

#define GPIO5_PAD PAD_GPIO5
#define GPIO5_OEN 0x103C0A, BIT5
#define GPIO5_OUT 0x103C0A, BIT4
#define GPIO5_IN  0x103C0A, BIT0

#define GPIO6_PAD PAD_GPIO6
#define GPIO6_OEN 0x103C0C, BIT5
#define GPIO6_OUT 0x103C0C, BIT4
#define GPIO6_IN  0x103C0C, BIT0

#define GPIO7_PAD PAD_GPIO7
#define GPIO7_OEN 0x103C0E, BIT5
#define GPIO7_OUT 0x103C0E, BIT4
#define GPIO7_IN  0x103C0E, BIT0

#define GPIO8_PAD PAD_GPIO8
#define GPIO8_OEN 0x103C10, BIT5
#define GPIO8_OUT 0x103C10, BIT4
#define GPIO8_IN  0x103C10, BIT0

#define GPIO9_PAD PAD_GPIO9
#define GPIO9_OEN 0x103C12, BIT5
#define GPIO9_OUT 0x103C12, BIT4
#define GPIO9_IN  0x103C12, BIT0

#define GPIO10_PAD PAD_GPIO12
#define GPIO10_OEN 0x103C18, BIT5
#define GPIO10_OUT 0x103C18, BIT4
#define GPIO10_IN  0x103C18, BIT0

#define GPIO11_PAD PAD_GPIO13
#define GPIO11_OEN 0x103C1A, BIT5
#define GPIO11_OUT 0x103C1A, BIT4
#define GPIO11_IN  0x103C1A, BIT0

#define GPIO12_PAD PAD_GPIO14
#define GPIO12_OEN 0x103C1C, BIT5
#define GPIO12_OUT 0x103C1C, BIT4
#define GPIO12_IN  0x103C1C, BIT0

#define GPIO13_PAD PAD_GPIO15
#define GPIO13_OEN 0x103C1E, BIT5
#define GPIO13_OUT 0x103C1E, BIT4
#define GPIO13_IN  0x103C1E, BIT0

#define GPIO14_PAD PAD_FUART_RX
#define GPIO14_OEN 0x103C28, BIT5
#define GPIO14_OUT 0x103C28, BIT4
#define GPIO14_IN  0x103C28, BIT0

#define GPIO15_PAD PAD_FUART_TX
#define GPIO15_OEN 0x103C2A, BIT5
#define GPIO15_OUT 0x103C2A, BIT4
#define GPIO15_IN  0x103C2A, BIT0

#define GPIO16_PAD PAD_FUART_CTS
#define GPIO16_OEN 0x103C2C, BIT5
#define GPIO16_OUT 0x103C2C, BIT4
#define GPIO16_IN  0x103C2C, BIT0

#define GPIO17_PAD PAD_FUART_RTS
#define GPIO17_OEN 0x103C2E, BIT5
#define GPIO17_OUT 0x103C2E, BIT4
#define GPIO17_IN  0x103C2E, BIT0

#define GPIO18_PAD PAD_I2C0_SCL
#define GPIO18_OEN 0x103CC0, BIT5
#define GPIO18_OUT 0x103CC0, BIT4
#define GPIO18_IN  0x103CC0, BIT0

#define GPIO19_PAD PAD_I2C0_SDA
#define GPIO19_OEN 0x103CC2, BIT5
#define GPIO19_OUT 0x103CC2, BIT4
#define GPIO19_IN  0x103CC2, BIT0

#define GPIO20_PAD PAD_I2C1_SCL
#define GPIO20_OEN 0x103CC4, BIT5
#define GPIO20_OUT 0x103CC4, BIT4
#define GPIO20_IN  0x103CC4, BIT0

#define GPIO21_PAD PAD_I2C1_SDA
#define GPIO21_OEN 0x103CC6, BIT5
#define GPIO21_OUT 0x103CC6, BIT4
#define GPIO21_IN  0x103CC6, BIT0

#define GPIO22_PAD PAD_SR_IO00
#define GPIO22_OEN 0x103C40, BIT5
#define GPIO22_OUT 0x103C40, BIT4
#define GPIO22_IN  0x103C40, BIT0

#define GPIO23_PAD PAD_SR_IO01
#define GPIO23_OEN 0x103C42, BIT5
#define GPIO23_OUT 0x103C42, BIT4
#define GPIO23_IN  0x103C42, BIT0

#define GPIO24_PAD PAD_SR_IO02
#define GPIO24_OEN 0x103C44, BIT5
#define GPIO24_OUT 0x103C44, BIT4
#define GPIO24_IN  0x103C44, BIT0

#define GPIO25_PAD PAD_SR_IO03
#define GPIO25_OEN 0x103C46, BIT5
#define GPIO25_OUT 0x103C46, BIT4
#define GPIO25_IN  0x103C46, BIT0

#define GPIO26_PAD PAD_SR_IO04
#define GPIO26_OEN 0x103C48, BIT5
#define GPIO26_OUT 0x103C48, BIT4
#define GPIO26_IN  0x103C48, BIT0

#define GPIO27_PAD PAD_SR_IO05
#define GPIO27_OEN 0x103C4A, BIT5
#define GPIO27_OUT 0x103C4A, BIT4
#define GPIO27_IN  0x103C4A, BIT0

#define GPIO28_PAD PAD_SR_IO06
#define GPIO28_OEN 0x103C4C, BIT5
#define GPIO28_OUT 0x103C4C, BIT4
#define GPIO28_IN  0x103C4C, BIT0

#define GPIO29_PAD PAD_SR_IO07
#define GPIO29_OEN 0x103C4E, BIT5
#define GPIO29_OUT 0x103C4E, BIT4
#define GPIO29_IN  0x103C4E, BIT0

#define GPIO30_PAD PAD_SR_IO08
#define GPIO30_OEN 0x103C50, BIT5
#define GPIO30_OUT 0x103C50, BIT4
#define GPIO30_IN  0x103C50, BIT0

#define GPIO31_PAD PAD_SR_IO09
#define GPIO31_OEN 0x103C52, BIT5
#define GPIO31_OUT 0x103C52, BIT4
#define GPIO31_IN  0x103C52, BIT0

#define GPIO32_PAD PAD_SR_IO10
#define GPIO32_OEN 0x103C54, BIT5
#define GPIO32_OUT 0x103C54, BIT4
#define GPIO32_IN  0x103C54, BIT0

#define GPIO33_PAD PAD_SR_IO11
#define GPIO33_OEN 0x103C56, BIT5
#define GPIO33_OUT 0x103C56, BIT4
#define GPIO33_IN  0x103C56, BIT0

#define GPIO34_PAD PAD_SR_IO12
#define GPIO34_OEN 0x103C58, BIT5
#define GPIO34_OUT 0x103C58, BIT4
#define GPIO34_IN  0x103C58, BIT0

#define GPIO35_PAD PAD_SR_IO13
#define GPIO35_OEN 0x103C5A, BIT5
#define GPIO35_OUT 0x103C5A, BIT4
#define GPIO35_IN  0x103C5A, BIT0

#define GPIO36_PAD PAD_SR_IO14
#define GPIO36_OEN 0x103C5C, BIT5
#define GPIO36_OUT 0x103C5C, BIT4
#define GPIO36_IN  0x103C5C, BIT0

#define GPIO37_PAD PAD_SR_IO15
#define GPIO37_OEN 0x103C5E, BIT5
#define GPIO37_OUT 0x103C5E, BIT4
#define GPIO37_IN  0x103C5E, BIT0

#define GPIO38_PAD PAD_SR_IO16
#define GPIO38_OEN 0x103C60, BIT5
#define GPIO38_OUT 0x103C60, BIT4
#define GPIO38_IN  0x103C60, BIT0

#define GPIO39_PAD PAD_SR_IO17
#define GPIO39_OEN 0x103C62, BIT5
#define GPIO39_OUT 0x103C62, BIT4
#define GPIO39_IN  0x103C62, BIT0

#define GPIO40_PAD PAD_UART0_RX
#define GPIO40_OEN 0x103C30, BIT5
#define GPIO40_OUT 0x103C30, BIT4
#define GPIO40_IN  0x103C30, BIT0

#define GPIO41_PAD PAD_UART0_TX
#define GPIO41_OEN 0x103C32, BIT5
#define GPIO41_OUT 0x103C32, BIT4
#define GPIO41_IN  0x103C32, BIT0

#define GPIO42_PAD PAD_UART1_RX
#define GPIO42_OEN 0x103C34, BIT5
#define GPIO42_OUT 0x103C34, BIT4
#define GPIO42_IN  0x103C34, BIT0

#define GPIO43_PAD PAD_UART1_TX
#define GPIO43_OEN 0x103C36, BIT5
#define GPIO43_OUT 0x103C36, BIT4
#define GPIO43_IN  0x103C36, BIT0

#define GPIO44_PAD PAD_SPI0_CZ
#define GPIO44_OEN 0x103CE0, BIT5
#define GPIO44_OUT 0x103CE0, BIT4
#define GPIO44_IN  0x103CE0, BIT0

#define GPIO45_PAD PAD_SPI0_CK
#define GPIO45_OEN 0x103CE2, BIT5
#define GPIO45_OUT 0x103CE2, BIT4
#define GPIO45_IN  0x103CE2, BIT0

#define GPIO46_PAD PAD_SPI0_DI
#define GPIO46_OEN 0x103CE4, BIT5
#define GPIO46_OUT 0x103CE4, BIT4
#define GPIO46_IN  0x103CE4, BIT0

#define GPIO47_PAD PAD_SPI0_DO
#define GPIO47_OEN 0x103CE6, BIT5
#define GPIO47_OUT 0x103CE6, BIT4
#define GPIO47_IN  0x103CE6, BIT0

#define GPIO48_PAD PAD_SPI1_CZ
#define GPIO48_OEN 0x103CE8, BIT5
#define GPIO48_OUT 0x103CE8, BIT4
#define GPIO48_IN  0x103CE8, BIT0

#define GPIO49_PAD PAD_SPI1_CK
#define GPIO49_OEN 0x103CEA, BIT5
#define GPIO49_OUT 0x103CEA, BIT4
#define GPIO49_IN  0x103CEA, BIT0

#define GPIO50_PAD PAD_SPI1_DI
#define GPIO50_OEN 0x103CEC, BIT5
#define GPIO50_OUT 0x103CEC, BIT4
#define GPIO50_IN  0x103CEC, BIT0

#define GPIO51_PAD PAD_SPI1_DO
#define GPIO51_OEN 0x103CEE, BIT5
#define GPIO51_OUT 0x103CEE, BIT4
#define GPIO51_IN  0x103CEE, BIT0

#define GPIO52_PAD PAD_PWM0
#define GPIO52_OEN 0x103C20, BIT5
#define GPIO52_OUT 0x103C20, BIT4
#define GPIO52_IN  0x103C20, BIT0

#define GPIO53_PAD PAD_PWM1
#define GPIO53_OEN 0x103C22, BIT5
#define GPIO53_OUT 0x103C22, BIT4
#define GPIO53_IN  0x103C22, BIT0

#define GPIO54_PAD PAD_SD_CLK
#define GPIO54_OEN 0x103CA0, BIT5
#define GPIO54_OUT 0x103CA0, BIT4
#define GPIO54_IN  0x103CA0, BIT0

#define GPIO55_PAD PAD_SD_CMD
#define GPIO55_OEN 0x103CA2, BIT5
#define GPIO55_OUT 0x103CA2, BIT4
#define GPIO55_IN  0x103CA2, BIT0

#define GPIO56_PAD PAD_SD_D0
#define GPIO56_OEN 0x103CA4, BIT5
#define GPIO56_OUT 0x103CA4, BIT4
#define GPIO56_IN  0x103CA4, BIT0

#define GPIO57_PAD PAD_SD_D1
#define GPIO57_OEN 0x103CA6, BIT5
#define GPIO57_OUT 0x103CA6, BIT4
#define GPIO57_IN  0x103CA6, BIT0

#define GPIO58_PAD PAD_SD_D2
#define GPIO58_OEN 0x103CA8, BIT5
#define GPIO58_OUT 0x103CA8, BIT4
#define GPIO58_IN  0x103CA8, BIT0

#define GPIO59_PAD PAD_SD_D3
#define GPIO59_OEN 0x103CAA, BIT5
#define GPIO59_OUT 0x103CAA, BIT4
#define GPIO59_IN  0x103CAA, BIT0

// PM
#define GPIO60_PAD PAD_PM_SD_CDZ
#define GPIO60_OEN 0xF8E, BIT0
#define GPIO60_OUT 0xF8E, BIT1
#define GPIO60_IN  0xF8E, BIT2

#define GPIO61_PAD PAD_PM_IRIN
#define GPIO61_OEN 0xF28, BIT0
#define GPIO61_OUT 0xF28, BIT1
#define GPIO61_IN  0xF28, BIT2

#define GPIO62_PAD PAD_PM_GPIO0
#define GPIO62_OEN 0xF00, BIT0
#define GPIO62_OUT 0xF00, BIT1
#define GPIO62_IN  0xF00, BIT2

#define GPIO63_PAD PAD_PM_GPIO1
#define GPIO63_OEN 0xF02, BIT0
#define GPIO63_OUT 0xF02, BIT1
#define GPIO63_IN  0xF02, BIT2

#define GPIO64_PAD PAD_PM_GPIO2
#define GPIO64_OEN 0xF04, BIT0
#define GPIO64_OUT 0xF04, BIT1
#define GPIO64_IN  0xF04, BIT2

#define GPIO65_PAD PAD_PM_GPIO3
#define GPIO65_OEN 0xF06, BIT0
#define GPIO65_OUT 0xF06, BIT1
#define GPIO65_IN  0xF06, BIT2

#define GPIO66_PAD PAD_PM_GPIO4
#define GPIO66_OEN 0xF08, BIT0
#define GPIO66_OUT 0xF08, BIT1
#define GPIO66_IN  0xF08, BIT2

#define GPIO67_PAD PAD_PM_GPIO7
#define GPIO67_OEN 0xF0E, BIT0
#define GPIO67_OUT 0xF0E, BIT1
#define GPIO67_IN  0xF0E, BIT2

#define GPIO68_PAD PAD_PM_GPIO8
#define GPIO68_OEN 0xF10, BIT0
#define GPIO68_OUT 0xF10, BIT1
#define GPIO68_IN  0xF10, BIT2

#define GPIO69_PAD PAD_PM_GPIO9
#define GPIO69_OEN 0xF12, BIT0
#define GPIO69_OUT 0xF12, BIT1
#define GPIO69_IN  0xF12, BIT2

#define GPIO70_PAD PAD_PM_SPI_CZ
#define GPIO70_OEN 0xF30, BIT0
#define GPIO70_OUT 0xF30, BIT1
#define GPIO70_IN  0xF30, BIT2

#define GPIO71_PAD PAD_PM_SPI_CK
#define GPIO71_OEN 0xF32, BIT0
#define GPIO71_OUT 0xF32, BIT1
#define GPIO71_IN  0xF32, BIT2

#define GPIO72_PAD PAD_PM_SPI_DI
#define GPIO72_OEN 0xF34, BIT0
#define GPIO72_OUT 0xF34, BIT1
#define GPIO72_IN  0xF34, BIT2

#define GPIO73_PAD PAD_PM_SPI_DO
#define GPIO73_OEN 0xF36, BIT0
#define GPIO73_OUT 0xF36, BIT1
#define GPIO73_IN  0xF36, BIT2

#define GPIO74_PAD PAD_PM_SPI_WPZ
#define GPIO74_OEN 0xF88, BIT0
#define GPIO74_OUT 0xF88, BIT1
#define GPIO74_IN  0xF88, BIT2

#define GPIO75_PAD PAD_PM_SPI_HLD
#define GPIO75_OEN 0xF8A, BIT0
#define GPIO75_OUT 0xF8A, BIT1
#define GPIO75_IN  0xF8A, BIT2

#define GPIO76_PAD PAD_PM_LED0
#define GPIO76_OEN 0xF94, BIT0
#define GPIO76_OUT 0xF94, BIT1
#define GPIO76_IN  0xF94, BIT2

#define GPIO77_PAD PAD_PM_LED1
#define GPIO77_OEN 0xF96, BIT0
#define GPIO77_OUT 0xF96, BIT1
#define GPIO77_IN  0xF96, BIT2

// SAR
#define GPIO78_PAD PAD_SAR_GPIO0
#define GPIO78_OEN 0x1423, BIT0
#define GPIO78_OUT 0x1424, BIT0
#define GPIO78_IN  0x1425, BIT0

#define GPIO79_PAD PAD_SAR_GPIO1
#define GPIO79_OEN 0x1423, BIT1
#define GPIO79_OUT 0x1424, BIT1
#define GPIO79_IN  0x1425, BIT1

#define GPIO80_PAD PAD_SAR_GPIO2
#define GPIO80_OEN 0x1423, BIT2
#define GPIO80_OUT 0x1424, BIT2
#define GPIO80_IN  0x1425, BIT2

#define GPIO81_PAD PAD_SAR_GPIO3
#define GPIO81_OEN 0x1423, BIT3
#define GPIO81_OUT 0x1424, BIT3
#define GPIO81_IN  0x1425, BIT3

// ALBANY
#define GPIO82_PAD PAD_ETH_RN
#define GPIO82_OEN 0x33E2, BIT4
#define GPIO82_OUT 0x33E4, BIT0
#define GPIO82_IN  0x33E4, BIT4

#define GPIO83_PAD PAD_ETH_RP
#define GPIO83_OEN 0x33E2, BIT5
#define GPIO83_OUT 0x33E4, BIT1
#define GPIO83_IN  0x33E4, BIT5

#define GPIO84_PAD PAD_ETH_TN
#define GPIO84_OEN 0x33E2, BIT6
#define GPIO84_OUT 0x33E4, BIT2
#define GPIO84_IN  0x33E4, BIT6

#define GPIO85_PAD PAD_ETH_TP
#define GPIO85_OEN 0x33E2, BIT7
#define GPIO85_OUT 0x33E4, BIT3
#define GPIO85_IN  0x33E4, BIT7

// UTMI
#define GPIO86_PAD PAD_USB_DM
#define GPIO86_OEN 0x14210a, BIT4
#define GPIO86_OUT 0x14210a, BIT2
#define GPIO86_IN  0x142131, BIT5

#define GPIO87_PAD PAD_USB_DP
#define GPIO87_OEN 0x14210a, BIT5
#define GPIO87_OUT 0x14210a, BIT3
#define GPIO87_IN  0x142131, BIT4

//SDIO
#define GPIO88_PAD PAD_SD1_IO0
#define GPIO88_OEN 0x103C80, BIT5
#define GPIO88_OUT 0x103C80, BIT4
#define GPIO88_IN  0x103C80, BIT0

#define GPIO89_PAD PAD_SD1_IO1
#define GPIO89_OEN 0x103C82, BIT5
#define GPIO89_OUT 0x103C82, BIT4
#define GPIO89_IN  0x103C82, BIT0

#define GPIO90_PAD PAD_SD1_IO2
#define GPIO90_OEN 0x103C84, BIT5
#define GPIO90_OUT 0x103C84, BIT4
#define GPIO90_IN  0x103C84, BIT0

#define GPIO91_PAD PAD_SD1_IO3
#define GPIO91_OEN 0x103C86, BIT5
#define GPIO91_OUT 0x103C86, BIT4
#define GPIO91_IN  0x103C86, BIT0

#define GPIO92_PAD PAD_SD1_IO4
#define GPIO92_OEN 0x103C88, BIT5
#define GPIO92_OUT 0x103C88, BIT4
#define GPIO92_IN  0x103C88, BIT0

#define GPIO93_PAD PAD_SD1_IO5
#define GPIO93_OEN 0x103C8A, BIT5
#define GPIO93_OUT 0x103C8A, BIT4
#define GPIO93_IN  0x103C8A, BIT0

#define GPIO94_PAD PAD_SD1_IO6
#define GPIO94_OEN 0x103C8C, BIT5
#define GPIO94_OUT 0x103C8C, BIT4
#define GPIO94_IN  0x103C8C, BIT0

#define GPIO95_PAD PAD_SD1_IO7
#define GPIO95_OEN 0x103C8E, BIT5
#define GPIO95_OUT 0x103C8E, BIT4
#define GPIO95_IN  0x103C8E, BIT0

#define GPIO96_PAD PAD_SD1_IO8
#define GPIO96_OEN 0x103C90, BIT5
#define GPIO96_OUT 0x103C90, BIT4
#define GPIO96_IN  0x103C90, BIT0

U32 gChipBaseAddr    = 0xFD203C00;
U32 gPmSleepBaseAddr = 0xFD001C00;
U32 gSarBaseAddr     = 0xFD002800;
U32 gRIUBaseAddr     = 0xFD000000;

#define MHal_CHIPTOP_REG(addr)  (*(volatile U8*)(gChipBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_PM_SLEEP_REG(addr) (*(volatile U8*)(gPmSleepBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_SAR_GPIO_REG(addr) (*(volatile U8*)(gSarBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_RIU_REG(addr)      (*(volatile U8*)(gRIUBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))

#define REG_ALL_PAD_IN     0xA1

//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------

static int _pmsleep_to_irq_table[] = {
        INT_PMSLEEP_SD_CDZ,
        INT_PMSLEEP_IRIN,
        INT_PMSLEEP_GPIO_0,
        INT_PMSLEEP_GPIO_1,
        INT_PMSLEEP_GPIO_2,
        INT_PMSLEEP_GPIO_3,
        INT_PMSLEEP_GPIO_4,
        INT_PMSLEEP_GPIO_7,
        INT_PMSLEEP_GPIO_8,
        INT_PMSLEEP_GPIO_9,
        INT_PMSLEEP_SPI_CZ,
        INT_PMSLEEP_SPI_CK,
        INT_PMSLEEP_SPI_DI,
        INT_PMSLEEP_SPI_DO,
        INT_PMSLEEP_SPI_WPZ,
        INT_PMSLEEP_SPI_HLD,
        INT_PMSLEEP_LED0,
        INT_PMSLEEP_LED1,
};

static int _gpi_to_irq_table[] = {
        INT_GPI_FIQ_PAD_GPIO0,
        INT_GPI_FIQ_PAD_GPIO1,
        INT_GPI_FIQ_PAD_GPIO2,
        INT_GPI_FIQ_PAD_GPIO3,
        INT_GPI_FIQ_PAD_GPIO4,
        INT_GPI_FIQ_PAD_GPIO5,
        INT_GPI_FIQ_PAD_GPIO6,
        INT_GPI_FIQ_PAD_GPIO7,
        INT_GPI_FIQ_PAD_GPIO8,
        INT_GPI_FIQ_PAD_GPIO9,
        INT_GPI_FIQ_PAD_GPIO12,
        INT_GPI_FIQ_PAD_GPIO13,
        INT_GPI_FIQ_PAD_GPIO14,
        INT_GPI_FIQ_PAD_GPIO15,
        INT_GPI_FIQ_PAD_FUART_RX,
        INT_GPI_FIQ_PAD_FUART_TX,
        INT_GPI_FIQ_PAD_FUART_CTS,
        INT_GPI_FIQ_PAD_FUART_RTS,
        INT_GPI_FIQ_PAD_I2C0_SCL,
        INT_GPI_FIQ_PAD_I2C0_SDA,
        INT_GPI_FIQ_PAD_I2C1_SCL,
        INT_GPI_FIQ_PAD_I2C1_SDA,
        INT_GPI_FIQ_PAD_SR_IO00,
        INT_GPI_FIQ_PAD_SR_IO01,
        INT_GPI_FIQ_PAD_SR_IO02,
        INT_GPI_FIQ_PAD_SR_IO03,
        INT_GPI_FIQ_PAD_SR_IO04,
        INT_GPI_FIQ_PAD_SR_IO05,
        INT_GPI_FIQ_PAD_SR_IO06,
        INT_GPI_FIQ_PAD_SR_IO07,
        INT_GPI_FIQ_PAD_SR_IO08,
        INT_GPI_FIQ_PAD_SR_IO09,
        INT_GPI_FIQ_PAD_SR_IO10,
        INT_GPI_FIQ_PAD_SR_IO11,
        INT_GPI_FIQ_PAD_SR_IO12,
        INT_GPI_FIQ_PAD_SR_IO13,
        INT_GPI_FIQ_PAD_SR_IO14,
        INT_GPI_FIQ_PAD_SR_IO15,
        INT_GPI_FIQ_PAD_SR_IO16,
        INT_GPI_FIQ_PAD_SR_IO17,
        INT_GPI_FIQ_PAD_UART0_RX,
        INT_GPI_FIQ_PAD_UART0_TX,
        INT_GPI_FIQ_PAD_UART1_RX,
        INT_GPI_FIQ_PAD_UART1_TX,
        INT_GPI_FIQ_PAD_SPI0_CZ,
        INT_GPI_FIQ_PAD_SPI0_CK,
        INT_GPI_FIQ_PAD_SPI0_DI,
        INT_GPI_FIQ_PAD_SPI0_DO,
        INT_GPI_FIQ_PAD_SPI1_CZ,
        INT_GPI_FIQ_PAD_SPI1_CK,
        INT_GPI_FIQ_PAD_SPI1_DI,
        INT_GPI_FIQ_PAD_SPI1_DO,
        INT_GPI_FIQ_PAD_PWM0,
        INT_GPI_FIQ_PAD_PWM1,
        INT_GPI_FIQ_PAD_SD_CLK,
        INT_GPI_FIQ_PAD_SD_CMD,
        INT_GPI_FIQ_PAD_SD_D0,
        INT_GPI_FIQ_PAD_SD_D1,
        INT_GPI_FIQ_PAD_SD_D2,
        INT_GPI_FIQ_PAD_SD_D3,
        -1, // [60] PAD_PM_SD_CDZ
        -1, // [61] PAD_PM_IRIN
        -1, // [62] PAD_PM_GPIO0
        -1, // [63] PAD_PM_GPIO1
        -1, // [64] PAD_PM_GPIO2
        -1, // [65] PAD_PM_GPIO3
        -1, // [66] PAD_PM_GPIO4
        -1, // [67] PAD_PM_GPIO7
        -1, // [68] PAD_PM_GPIO8
        -1, // [69] PAD_PM_GPIO9
        -1, // [70] PAD_PM_SPI_CZ
        -1, // [71] PAD_PM_SPI_CK
        -1, // [72] PAD_PM_SPI_DI
        -1, // [73] PAD_PM_SPI_DO
        -1, // [74] PAD_PM_SPI_WPZ
        -1, // [75] PAD_PM_SPI_HLD
        -1, // [76] PAD_PM_LED0
        -1, // [77] PAD_PM_LED1
        -1, // [78] INT_GPI_FIQ_PAD_SAR_GPIO0,
        -1, // [79] INT_GPI_FIQ_PAD_SAR_GPIO1,
        -1, // [80] INT_GPI_FIQ_PAD_SAR_GPIO2,
        -1, // [81] INT_GPI_FIQ_PAD_SAR_GPIO3,
        -1, // [82] INT_GPI_FIQ_PAD_ETH_RN,
        -1, // [83] INT_GPI_FIQ_PAD_ETH_RP,
        -1, // [84] INT_GPI_FIQ_PAD_ETH_TN,
        -1, // [85] INT_GPI_FIQ_PAD_ETH_TP,
        -1, // [86] INT_GPI_FIQ_PAD_USB_DM,
        -1, // [87] INT_GPI_FIQ_PAD_USB_DP,
        INT_GPI_FIQ_PAD_SD1_IO0,
        INT_GPI_FIQ_PAD_SD1_IO1,
        INT_GPI_FIQ_PAD_SD1_IO2,
        INT_GPI_FIQ_PAD_SD1_IO3,
        INT_GPI_FIQ_PAD_SD1_IO4,
        INT_GPI_FIQ_PAD_SD1_IO5,
        INT_GPI_FIQ_PAD_SD1_IO6,
        INT_GPI_FIQ_PAD_SD1_IO7,
        INT_GPI_FIQ_PAD_SD1_IO8,
};

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
    __GPIO(0), __GPIO(1), __GPIO(2), __GPIO(3), __GPIO(4), __GPIO(5), __GPIO(6), __GPIO(7),
    __GPIO(8), __GPIO(9), __GPIO(10), __GPIO(11), __GPIO(12), __GPIO(13), __GPIO(14), __GPIO(15),
    __GPIO(16), __GPIO(17), __GPIO(18), __GPIO(19), __GPIO(20), __GPIO(21), __GPIO(22), __GPIO(23),
    __GPIO(24), __GPIO(25), __GPIO(26), __GPIO(27), __GPIO(28), __GPIO(29), __GPIO(30), __GPIO(31),
    __GPIO(32), __GPIO(33), __GPIO(34), __GPIO(35), __GPIO(36), __GPIO(37), __GPIO(38), __GPIO(39),
    __GPIO(40), __GPIO(41), __GPIO(42), __GPIO(43), __GPIO(44), __GPIO(45), __GPIO(46), __GPIO(47),
    __GPIO(48), __GPIO(49), __GPIO(50), __GPIO(51), __GPIO(52), __GPIO(53), __GPIO(54), __GPIO(55),
    __GPIO(56), __GPIO(57), __GPIO(58), __GPIO(59), __GPIO(60), __GPIO(61), __GPIO(62), __GPIO(63),
    __GPIO(64), __GPIO(65), __GPIO(66), __GPIO(67), __GPIO(68), __GPIO(69), __GPIO(70), __GPIO(71),
    __GPIO(72), __GPIO(73), __GPIO(74), __GPIO(75), __GPIO(76), __GPIO(77), __GPIO(78), __GPIO(79),
    __GPIO(80), __GPIO(81), __GPIO(82), __GPIO(83), __GPIO(84), __GPIO(85), __GPIO(86), __GPIO(87),
    __GPIO(88), __GPIO(89), __GPIO(90), __GPIO(91), __GPIO(92), __GPIO(93), __GPIO(94), __GPIO(95),
    __GPIO(96),
};

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

void MHal_GPIO_Init(void)
{
    MHal_CHIPTOP_REG(REG_ALL_PAD_IN) &= ~BIT7;
}

void MHal_GPIO_Pad_Set(U8 u8IndexGPIO)
{
    HalPadSetVal(u8IndexGPIO, PINMUX_FOR_GPIO_MODE);
}

int MHal_GPIO_PadGroupMode_Set(U32 u32PadMode)
{
    return HalPadSetMode(u32PadMode);
}

int MHal_GPIO_PadVal_Set(U8 u8IndexGPIO, U32 u32PadMode)
{
    return HalPadSetVal((U32)u8IndexGPIO, u32PadMode);
}

void MHal_GPIO_Pad_Oen(U8 u8IndexGPIO)
{
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_oen) &= (~gpio_table[u8IndexGPIO].m_oen);
}

void MHal_GPIO_Pad_Odn(U8 u8IndexGPIO)
{
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_oen) |= gpio_table[u8IndexGPIO].m_oen;
}

U8 MHal_GPIO_Pad_Level(U8 u8IndexGPIO)
{
    return ((MHal_RIU_REG(gpio_table[u8IndexGPIO].r_in)&gpio_table[u8IndexGPIO].m_in)? 1 : 0);
}

U8 MHal_GPIO_Pad_InOut(U8 u8IndexGPIO)
{
    return ((MHal_RIU_REG(gpio_table[u8IndexGPIO].r_oen)&gpio_table[u8IndexGPIO].m_oen)? 1 : 0);
}

void MHal_GPIO_Pull_High(U8 u8IndexGPIO)
{
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_out) |= gpio_table[u8IndexGPIO].m_out;
}

void MHal_GPIO_Pull_Low(U8 u8IndexGPIO)
{
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_out) &= (~gpio_table[u8IndexGPIO].m_out);
}

void MHal_GPIO_Set_High(U8 u8IndexGPIO)
{
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_oen) &= (~gpio_table[u8IndexGPIO].m_oen);
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_out) |= gpio_table[u8IndexGPIO].m_out;
}

void MHal_GPIO_Set_Low(U8 u8IndexGPIO)
{
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_oen) &= (~gpio_table[u8IndexGPIO].m_oen);
    MHal_RIU_REG(gpio_table[u8IndexGPIO].r_out) &= (~gpio_table[u8IndexGPIO].m_out);
}

void MHal_Enable_GPIO_INT(U8 u8IndexGPIO)
{
    // TBD
}

static int PMSLEEP_GPIO_To_Irq(U8 u8IndexGPIO)
{
    if ((u8IndexGPIO < PAD_PM_SD_CDZ) || (u8IndexGPIO > PAD_PM_LED1))
        return -1;
    else
    {
        return _pmsleep_to_irq_table[u8IndexGPIO-PAD_PM_SD_CDZ];
    }
}

static int GPI_GPIO_To_Irq(U8 u8IndexGPIO)
{
    if (u8IndexGPIO >= GPIO_NR)
        return -1;
    else
        return _gpi_to_irq_table[u8IndexGPIO];
}

//MHal_GPIO_To_Irq return any virq
int MHal_GPIO_To_Irq(U8 u8IndexGPIO)
{
    struct device_node *intr_node;
    struct irq_domain *intr_domain;
    struct irq_fwspec fwspec;
    int hwirq, virq = -1;

    if ((hwirq = PMSLEEP_GPIO_To_Irq(u8IndexGPIO)) >= 0)
    {
        //get virtual irq number for request_irq
        intr_node = of_find_compatible_node(NULL, NULL, "sstar,pm-intc");
        intr_domain = irq_find_host(intr_node);
        if(!intr_domain)
            return -ENXIO;

        fwspec.param_count = 1;
        fwspec.param[0] = hwirq;
        fwspec.fwnode = of_node_to_fwnode(intr_node);
        virq = irq_create_fwspec_mapping(&fwspec);
    }
    else if ((hwirq = GPI_GPIO_To_Irq(u8IndexGPIO)) >= 0)
    {
        //get virtual irq number for request_irq
        intr_node = of_find_compatible_node(NULL, NULL, "sstar,gpi-intc");
        intr_domain = irq_find_host(intr_node);
        if(!intr_domain)
            return -ENXIO;

        fwspec.param_count = 1;
        fwspec.param[0] = hwirq;
        fwspec.fwnode = of_node_to_fwnode(intr_node);
        virq = irq_create_fwspec_mapping(&fwspec);
    }
    else if ((u8IndexGPIO >= PAD_SAR_GPIO0 && u8IndexGPIO <= PAD_SAR_GPIO3))
    {
        //get virtual irq number for request_irq
        intr_node = of_find_compatible_node(NULL, NULL, "sstar,main-intc");
        intr_domain = irq_find_host(intr_node);
        if(!intr_domain)
            return -ENXIO;

        fwspec.param_count = 3;
        fwspec.param[0] = GIC_SPI;
        fwspec.param[1] = u8IndexGPIO - PAD_SAR_GPIO0 + INT_FIQ_SAR_GPIO_0;
        fwspec.param[2] = IRQ_TYPE_NONE;
        fwspec.fwnode = of_node_to_fwnode(intr_node);
        virq = irq_create_fwspec_mapping(&fwspec);
    }

    return virq;
}

void MHal_GPIO_Set_POLARITY(U8 u8IndexGPIO, U8 reverse)
{
    // TBD
}
