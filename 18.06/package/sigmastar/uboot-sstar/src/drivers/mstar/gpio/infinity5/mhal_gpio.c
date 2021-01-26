/*
* mhal_gpio.c- Sigmastar
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

#ifndef _MHAL_GPIO_C_
#define _MHAL_GPIO_C_


#include <common.h>
#include <command.h>
#include <MsDebug.h>

#include "mhal_gpio.h"
#include "mhal_gpio_reg.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------

#define GPIO_HAL_ERR(x, args...)        //{printf(x, ##args);}
#define GPIO_HAL_NOTSUPPORT()           {printf("%s not support!\n", __FUNCTION__);}

#ifndef MS_ASSERT
#define MS_ASSERT(expr)                 do {                                                        \
                                            if(!(expr))                                             \
                                                printf("MVD assert fail %s %d!\n", __FILE__, __LINE__); \
                                        } while(0)
#endif


#define _CONCAT( a, b )     a##b
#define CONCAT( a, b )      _CONCAT( a, b )

//PADTOP
#define GPIO0_PAD PAD_GPIO0
#define GPIO0_OEN 0x103c00, BIT5
#define GPIO0_IN  0x103c00, BIT0
#define GPIO0_OUT 0x103c00, BIT4

#define GPIO1_PAD PAD_GPIO1
#define GPIO1_OEN 0x103c02, BIT5
#define GPIO1_IN  0x103c02, BIT0
#define GPIO1_OUT 0x103c02, BIT4

#define GPIO2_PAD PAD_GPIO2
#define GPIO2_OEN 0x103c04, BIT5
#define GPIO2_IN  0x103c04, BIT0
#define GPIO2_OUT 0x103c04, BIT4

#define GPIO3_PAD PAD_GPIO3
#define GPIO3_OEN 0x103c06, BIT5
#define GPIO3_IN  0x103c06, BIT0
#define GPIO3_OUT 0x103c06, BIT4

#define GPIO4_PAD PAD_GPIO4
#define GPIO4_OEN 0x103c08, BIT5
#define GPIO4_IN  0x103c08, BIT0
#define GPIO4_OUT 0x103c08, BIT4

#define GPIO5_PAD PAD_GPIO5
#define GPIO5_OEN 0x103c0a, BIT5
#define GPIO5_IN  0x103c0a, BIT0
#define GPIO5_OUT 0x103c0a, BIT4

#define GPIO6_PAD PAD_GPIO6
#define GPIO6_OEN 0x103c0c, BIT5
#define GPIO6_IN  0x103c0c, BIT0
#define GPIO6_OUT 0x103c0c, BIT4

#define GPIO7_PAD PAD_GPIO7
#define GPIO7_OEN 0x103c0e, BIT5
#define GPIO7_IN  0x103c0e, BIT0
#define GPIO7_OUT 0x103c0e, BIT4

#define GPIO8_PAD PAD_GPIO8
#define GPIO8_OEN 0x103c10, BIT5
#define GPIO8_IN  0x103c10, BIT0
#define GPIO8_OUT 0x103c10, BIT4

#define GPIO9_PAD PAD_GPIO9
#define GPIO9_OEN 0x103c12, BIT5
#define GPIO9_IN  0x103c12, BIT0
#define GPIO9_OUT 0x103c12, BIT4

#define GPIO10_PAD PAD_GPIO10
#define GPIO10_OEN 0x103c14, BIT5
#define GPIO10_IN  0x103c14, BIT0
#define GPIO10_OUT 0x103c14, BIT4

#define GPIO11_PAD PAD_GPIO11
#define GPIO11_OEN 0x103c16, BIT5
#define GPIO11_IN  0x103c16, BIT0
#define GPIO11_OUT 0x103c16, BIT4

#define GPIO12_PAD PAD_GPIO12
#define GPIO12_OEN 0x103c18, BIT5
#define GPIO12_IN  0x103c18, BIT0
#define GPIO12_OUT 0x103c18, BIT4

#define GPIO13_PAD PAD_GPIO13
#define GPIO13_OEN 0x103c1a, BIT5
#define GPIO13_IN  0x103c1a, BIT0
#define GPIO13_OUT 0x103c1a, BIT4

#define GPIO14_PAD PAD_GPIO14
#define GPIO14_OEN 0x103c1c, BIT5
#define GPIO14_IN  0x103c1c, BIT0
#define GPIO14_OUT 0x103c1c, BIT4

#define GPIO15_PAD PAD_GPIO15
#define GPIO15_OEN 0x103c1e, BIT5
#define GPIO15_IN  0x103c1e, BIT0
#define GPIO15_OUT 0x103c1e, BIT4

#define GPIO16_PAD PAD_FUART_RX
#define GPIO16_OEN 0x103c28, BIT5
#define GPIO16_IN  0x103c28, BIT0
#define GPIO16_OUT 0x103c28, BIT4

#define GPIO17_PAD PAD_FUART_TX
#define GPIO17_OEN 0x103c2a, BIT5
#define GPIO17_IN  0x103c2a, BIT0
#define GPIO17_OUT 0x103c2a, BIT4

#define GPIO18_PAD PAD_FUART_CTS
#define GPIO18_OEN 0x103c2c, BIT5
#define GPIO18_IN  0x103c2c, BIT0
#define GPIO18_OUT 0x103c2c, BIT4

#define GPIO19_PAD PAD_FUART_RTS
#define GPIO19_OEN 0x103c2e, BIT5
#define GPIO19_IN  0x103c2e, BIT0
#define GPIO19_OUT 0x103c2e, BIT4

#define GPIO20_PAD PAD_I2C0_SCL
#define GPIO20_OEN 0x103c38, BIT5
#define GPIO20_IN  0x103c38, BIT0
#define GPIO20_OUT 0x103c38, BIT4

#define GPIO21_PAD PAD_I2C0_SDA
#define GPIO21_OEN 0x103c3a, BIT5
#define GPIO21_IN  0x103c3a, BIT0
#define GPIO21_OUT 0x103c3a, BIT4

#define GPIO22_PAD PAD_I2C1_SCL
#define GPIO22_OEN 0x103c3c, BIT5
#define GPIO22_IN  0x103c3c, BIT0
#define GPIO22_OUT 0x103c3c, BIT4

#define GPIO23_PAD PAD_I2C1_SDA
#define GPIO23_OEN 0x103c3e, BIT5
#define GPIO23_IN  0x103c3e, BIT0
#define GPIO23_OUT 0x103c3e, BIT4

#define GPIO24_PAD PAD_SNR0_D0
#define GPIO24_OEN 0x103c40, BIT5
#define GPIO24_IN  0x103c40, BIT0
#define GPIO24_OUT 0x103c40, BIT4

#define GPIO25_PAD PAD_SNR0_D1
#define GPIO25_OEN 0x103c42, BIT5
#define GPIO25_IN  0x103c42, BIT0
#define GPIO25_OUT 0x103c42, BIT4

#define GPIO26_PAD PAD_SNR0_D2
#define GPIO26_OEN 0x103c44, BIT5
#define GPIO26_IN  0x103c44, BIT0
#define GPIO26_OUT 0x103c44, BIT4

#define GPIO27_PAD PAD_SNR0_D3
#define GPIO27_OEN 0x103c46, BIT5
#define GPIO27_IN  0x103c46, BIT0
#define GPIO27_OUT 0x103c46, BIT4

#define GPIO28_PAD PAD_SNR0_D4
#define GPIO28_OEN 0x103c48, BIT5
#define GPIO28_IN  0x103c48, BIT0
#define GPIO28_OUT 0x103c48, BIT4

#define GPIO29_PAD PAD_SNR0_D5
#define GPIO29_OEN 0x103c4a, BIT5
#define GPIO29_IN  0x103c4a, BIT0
#define GPIO29_OUT 0x103c4a, BIT4

#define GPIO30_PAD PAD_SNR0_D6
#define GPIO30_OEN 0x103c4c, BIT5
#define GPIO30_IN  0x103c4c, BIT0
#define GPIO30_OUT 0x103c4c, BIT4

#define GPIO31_PAD PAD_SNR0_D7
#define GPIO31_OEN 0x103c4e, BIT5
#define GPIO31_IN  0x103c4e, BIT0
#define GPIO31_OUT 0x103c4e, BIT4

#define GPIO32_PAD PAD_SNR0_D8
#define GPIO32_OEN 0x103c50, BIT5
#define GPIO32_IN  0x103c50, BIT0
#define GPIO32_OUT 0x103c50, BIT4

#define GPIO33_PAD PAD_SNR0_D9
#define GPIO33_OEN 0x103c52, BIT5
#define GPIO33_IN  0x103c52, BIT0
#define GPIO33_OUT 0x103c52, BIT4

#define GPIO34_PAD PAD_SNR0_D10
#define GPIO34_OEN 0x103c54, BIT5
#define GPIO34_IN  0x103c54, BIT0
#define GPIO34_OUT 0x103c54, BIT4

#define GPIO35_PAD PAD_SNR0_D11
#define GPIO35_OEN 0x103c56, BIT5
#define GPIO35_IN  0x103c56, BIT0
#define GPIO35_OUT 0x103c56, BIT4

#define GPIO36_PAD PAD_SNR0_GPIO0
#define GPIO36_OEN 0x103c58, BIT5
#define GPIO36_IN  0x103c58, BIT0
#define GPIO36_OUT 0x103c58, BIT4

#define GPIO37_PAD PAD_SNR0_GPIO1
#define GPIO37_OEN 0x103c5a, BIT5
#define GPIO37_IN  0x103c5a, BIT0
#define GPIO37_OUT 0x103c5a, BIT4

#define GPIO38_PAD PAD_SR_IO14
#define GPIO38_OEN 0x103c5c, BIT5
#define GPIO38_IN  0x103c5c, BIT0
#define GPIO38_OUT 0x103c5c, BIT4

#define GPIO39_PAD PAD_SNR0_GPIO3
#define GPIO39_OEN 0x103c5e, BIT5
#define GPIO39_IN  0x103c5e, BIT0
#define GPIO39_OUT 0x103c5e, BIT4

#define GPIO40_PAD PAD_SNR0_GPIO4
#define GPIO40_OEN 0x103c60, BIT5
#define GPIO40_IN  0x103c60, BIT0
#define GPIO40_OUT 0x103c60, BIT4

#define GPIO41_PAD PAD_SNR0_GPIO5
#define GPIO41_OEN 0x103c62, BIT5
#define GPIO41_IN  0x103c62, BIT0
#define GPIO41_OUT 0x103c62, BIT4

#define GPIO42_PAD PAD_SNR0_GPIO6
#define GPIO42_OEN 0x103c64, BIT5
#define GPIO42_IN  0x103c64, BIT0
#define GPIO42_OUT 0x103c64, BIT4

#define GPIO43_PAD PAD_SNR1_DA0P
#define GPIO43_OEN 0x103c74, BIT5
#define GPIO43_IN  0x103c74, BIT0
#define GPIO43_OUT 0x103c74, BIT4

#define GPIO44_PAD PAD_SNR1_DA0N
#define GPIO44_OEN 0x103c76, BIT5
#define GPIO44_IN  0x103c76, BIT0
#define GPIO44_OUT 0x103c76, BIT4

#define GPIO45_PAD PAD_SNR1_CKP
#define GPIO45_OEN 0x103c78, BIT5
#define GPIO45_IN  0x103c78, BIT0
#define GPIO45_OUT 0x103c78, BIT4

#define GPIO46_PAD PAD_SNR1_CKN
#define GPIO46_OEN 0x103c7a, BIT5
#define GPIO46_IN  0x103c7a, BIT0
#define GPIO46_OUT 0x103c7a, BIT4

#define GPIO47_PAD PAD_SNR1_DA1P
#define GPIO47_OEN 0x103c7c, BIT5
#define GPIO47_IN  0x103c7c, BIT0
#define GPIO47_OUT 0x103c7c, BIT4

#define GPIO48_PAD PAD_SNR1_DA1N
#define GPIO48_OEN 0x103c7e, BIT5
#define GPIO48_IN  0x103c7e, BIT0
#define GPIO48_OUT 0x103c7e, BIT4

#define GPIO49_PAD PAD_SNR1_GPIO0
#define GPIO49_OEN 0x103c66, BIT5
#define GPIO49_IN  0x103c66, BIT0
#define GPIO49_OUT 0x103c66, BIT4

#define GPIO50_PAD PAD_SNR1_GPIO1
#define GPIO50_OEN 0x103c68, BIT5
#define GPIO50_IN  0x103c68, BIT0
#define GPIO50_OUT 0x103c68, BIT4

#define GPIO51_PAD PAD_SNR1_GPIO2
#define GPIO51_OEN 0x103c6a, BIT5
#define GPIO51_IN  0x103c6a, BIT0
#define GPIO51_OUT 0x103c6a, BIT4

#define GPIO52_PAD PAD_SNR1_GPIO3
#define GPIO52_OEN 0x103c6c, BIT5
#define GPIO52_IN  0x103c6c, BIT0
#define GPIO52_OUT 0x103c6c, BIT4

#define GPIO53_PAD PAD_SNR1_GPIO4
#define GPIO53_OEN 0x103c6e, BIT5
#define GPIO53_IN  0x103c6e, BIT0
#define GPIO53_OUT 0x103c6e, BIT4

#define GPIO54_PAD PAD_SNR1_GPIO5
#define GPIO54_OEN 0x103c70, BIT5
#define GPIO54_IN  0x103c70, BIT0
#define GPIO54_OUT 0x103c70, BIT4

#define GPIO55_PAD PAD_SNR1_GPIO6
#define GPIO55_OEN 0x103c72, BIT5
#define GPIO55_IN  0x103c72, BIT0
#define GPIO55_OUT 0x103c72, BIT4

#define GPIO56_PAD PAD_NAND_ALE
#define GPIO56_OEN 0x103c80, BIT5
#define GPIO56_IN  0x103c80, BIT0
#define GPIO56_OUT 0x103c80, BIT4

#define GPIO57_PAD PAD_NAND_CLE
#define GPIO57_OEN 0x103c82, BIT5
#define GPIO57_IN  0x103c82, BIT0
#define GPIO57_OUT 0x103c82, BIT4

#define GPIO58_PAD PAD_NAND_CEZ
#define GPIO58_OEN 0x103c84, BIT5
#define GPIO58_IN  0x103c84, BIT0
#define GPIO58_OUT 0x103c84, BIT4

#define GPIO59_PAD PAD_NAND_WEZ
#define GPIO59_OEN 0x103c86, BIT5
#define GPIO59_IN  0x103c86, BIT0
#define GPIO59_OUT 0x103c86, BIT4

#define GPIO60_PAD PAD_NAND_WPZ
#define GPIO60_OEN 0x103c88, BIT5
#define GPIO60_IN  0x103c88, BIT0
#define GPIO60_OUT 0x103c88, BIT4

#define GPIO61_PAD PAD_NAND_REZ
#define GPIO61_OEN 0x103c8a, BIT5
#define GPIO61_IN  0x103c8a, BIT0
#define GPIO61_OUT 0x103c8a, BIT4

#define GPIO62_PAD PAD_NAND_RBZ
#define GPIO62_OEN 0x103c8c, BIT5
#define GPIO62_IN  0x103c8c, BIT0
#define GPIO62_OUT 0x103c8c, BIT4

#define GPIO63_PAD PAD_NAND_DA0
#define GPIO63_OEN 0x103c8e, BIT5
#define GPIO63_IN  0x103c8e, BIT0
#define GPIO63_OUT 0x103c8e, BIT4

#define GPIO64_PAD PAD_NAND_DA1
#define GPIO64_OEN 0x103c90, BIT5
#define GPIO64_IN  0x103c90, BIT0
#define GPIO64_OUT 0x103c90, BIT4

#define GPIO65_PAD PAD_NAND_DA2
#define GPIO65_OEN 0x103c92, BIT5
#define GPIO65_IN  0x103c92, BIT0
#define GPIO65_OUT 0x103c92, BIT4

#define GPIO66_PAD PAD_NAND_DA3
#define GPIO66_OEN 0x103c94, BIT5
#define GPIO66_IN  0x103c94, BIT0
#define GPIO66_OUT 0x103c94, BIT4

#define GPIO67_PAD PAD_NAND_DA4
#define GPIO67_OEN 0x103c96, BIT5
#define GPIO67_IN  0x103c96, BIT0
#define GPIO67_OUT 0x103c96, BIT4

#define GPIO68_PAD PAD_NAND_DA5
#define GPIO68_OEN 0x103c98, BIT5
#define GPIO68_IN  0x103c98, BIT0
#define GPIO68_OUT 0x103c98, BIT4

#define GPIO69_PAD PAD_NAND_DA6
#define GPIO69_OEN 0x103c9a, BIT5
#define GPIO69_IN  0x103c9a, BIT0
#define GPIO69_OUT 0x103c9a, BIT4

#define GPIO70_PAD PAD_NAND_DA7
#define GPIO70_OEN 0x103c9c, BIT5
#define GPIO70_IN  0x103c9c, BIT0
#define GPIO70_OUT 0x103c9c, BIT4

#define GPIO71_PAD PAD_LCD_D0
#define GPIO71_OEN 0x103ca0, BIT5
#define GPIO71_IN  0x103ca0, BIT0
#define GPIO71_OUT 0x103ca0, BIT4

#define GPIO72_PAD PAD_LCD_D1
#define GPIO72_OEN 0x103ca2, BIT5
#define GPIO72_IN  0x103ca2, BIT0
#define GPIO72_OUT 0x103ca2, BIT4

#define GPIO73_PAD PAD_LCD_D2
#define GPIO73_OEN 0x103ca4, BIT5
#define GPIO73_IN  0x103ca4, BIT0
#define GPIO73_OUT 0x103ca4, BIT4

#define GPIO74_PAD PAD_LCD_D3
#define GPIO74_OEN 0x103ca6, BIT5
#define GPIO74_IN  0x103ca6, BIT0
#define GPIO74_OUT 0x103ca6, BIT4

#define GPIO75_PAD PAD_LCD_D4
#define GPIO75_OEN 0x103ca8, BIT5
#define GPIO75_IN  0x103ca8, BIT0
#define GPIO75_OUT 0x103ca8, BIT4

#define GPIO76_PAD PAD_LCD_D5
#define GPIO76_OEN 0x103caa, BIT5
#define GPIO76_IN  0x103caa, BIT0
#define GPIO76_OUT 0x103caa, BIT4

#define GPIO77_PAD PAD_LCD_D6
#define GPIO77_OEN 0x103cac, BIT5
#define GPIO77_IN  0x103cac, BIT0
#define GPIO77_OUT 0x103cac, BIT4

#define GPIO78_PAD PAD_LCD_D7
#define GPIO78_OEN 0x103cae, BIT5
#define GPIO78_IN  0x103cae, BIT0
#define GPIO78_OUT 0x103cae, BIT4

#define GPIO79_PAD PAD_LCD_D8
#define GPIO79_OEN 0x103cb0, BIT5
#define GPIO79_IN  0x103cb0, BIT0
#define GPIO79_OUT 0x103cb0, BIT4

#define GPIO80_PAD PAD_LCD_D9
#define GPIO80_OEN 0x103cb2, BIT5
#define GPIO80_IN  0x103cb2, BIT0
#define GPIO80_OUT 0x103cb2, BIT4

#define GPIO81_PAD PAD_LCD_D10
#define GPIO81_OEN 0x103cb4, BIT5
#define GPIO81_IN  0x103cb4, BIT0
#define GPIO81_OUT 0x103cb4, BIT4

#define GPIO82_PAD PAD_LCD_D11
#define GPIO82_OEN 0x103cb6, BIT5
#define GPIO82_IN  0x103cb6, BIT0
#define GPIO82_OUT 0x103cb6, BIT4

#define GPIO83_PAD PAD_LCD_D12
#define GPIO83_OEN 0x103cb8, BIT5
#define GPIO83_IN  0x103cb8, BIT0
#define GPIO83_OUT 0x103cb8, BIT4

#define GPIO84_PAD PAD_LCD_D13
#define GPIO84_OEN 0x103cba, BIT5
#define GPIO84_IN  0x103cba, BIT0
#define GPIO84_OUT 0x103cba, BIT4

#define GPIO85_PAD PAD_LCD_D14
#define GPIO85_OEN 0x103cbc, BIT5
#define GPIO85_IN  0x103cbc, BIT0
#define GPIO85_OUT 0x103cbc, BIT4

#define GPIO86_PAD PAD_LCD_D15
#define GPIO86_OEN 0x103cbe, BIT5
#define GPIO86_IN  0x103cbe, BIT0
#define GPIO86_OUT 0x103cbe, BIT4

#define GPIO87_PAD PAD_LCD_D16
#define GPIO87_OEN 0x103cc0, BIT5
#define GPIO87_IN  0x103cc0, BIT0
#define GPIO87_OUT 0x103cc0, BIT4

#define GPIO88_PAD PAD_LCD_D17
#define GPIO88_OEN 0x103cc2, BIT5
#define GPIO88_IN  0x103cc2, BIT0
#define GPIO88_OUT 0x103cc2, BIT4

#define GPIO89_PAD PAD_LCD_D18
#define GPIO89_OEN 0x103cc4, BIT5
#define GPIO89_IN  0x103cc4, BIT0
#define GPIO89_OUT 0x103cc4, BIT4

#define GPIO90_PAD PAD_LCD_D19
#define GPIO90_OEN 0x103cc6, BIT5
#define GPIO90_IN  0x103cc6, BIT0
#define GPIO90_OUT 0x103cc6, BIT4

#define GPIO91_PAD PAD_LCD_D20
#define GPIO91_OEN 0x103cc8, BIT5
#define GPIO91_IN  0x103cc8, BIT0
#define GPIO91_OUT 0x103cc8, BIT4

#define GPIO92_PAD PAD_LCD_D21
#define GPIO92_OEN 0x103cca, BIT5
#define GPIO92_IN  0x103cca, BIT0
#define GPIO92_OUT 0x103cca, BIT4

#define GPIO93_PAD PAD_LCD_D22
#define GPIO93_OEN 0x103ccc, BIT5
#define GPIO93_IN  0x103ccc, BIT0
#define GPIO93_OUT 0x103ccc, BIT4

#define GPIO94_PAD PAD_LCD_D23
#define GPIO94_OEN 0x103cce, BIT5
#define GPIO94_IN  0x103cce, BIT0
#define GPIO94_OUT 0x103cce, BIT4

#define GPIO95_PAD PAD_LCD_VSYNC
#define GPIO95_OEN 0x103cd0, BIT5
#define GPIO95_IN  0x103cd0, BIT0
#define GPIO95_OUT 0x103cd0, BIT4

#define GPIO96_PAD PAD_LCD_HSYNC
#define GPIO96_OEN 0x103cd2, BIT5
#define GPIO96_IN  0x103cd2, BIT0
#define GPIO96_OUT 0x103cd2, BIT4

#define GPIO97_PAD PAD_LCD_PCLK
#define GPIO97_OEN 0x103cd4, BIT5
#define GPIO97_IN  0x103cd4, BIT0
#define GPIO97_OUT 0x103cd4, BIT4

#define GPIO98_PAD PAD_LCD_DE
#define GPIO98_OEN 0x103cd6, BIT5
#define GPIO98_IN  0x103cd6, BIT0
#define GPIO98_OUT 0x103cd6, BIT4

#define GPIO99_PAD PAD_UART0_RX
#define GPIO99_OEN 0x103c30, BIT5
#define GPIO99_IN  0x103c30, BIT0
#define GPIO99_OUT 0x103c30, BIT4

#define GPIO100_PAD PAD_UART0_TX
#define GPIO100_OEN 0x103c32, BIT5
#define GPIO100_IN  0x103c32, BIT0
#define GPIO100_OUT 0x103c32, BIT4

#define GPIO101_PAD PAD_UART1_RX
#define GPIO101_OEN 0x103c34, BIT5
#define GPIO101_IN  0x103c34, BIT0
#define GPIO101_OUT 0x103c34, BIT4

#define GPIO102_PAD PAD_UART1_TX
#define GPIO102_OEN 0x103c36, BIT5
#define GPIO102_IN  0x103c36, BIT0
#define GPIO102_OUT 0x103c36, BIT4

#define GPIO103_PAD PAD_SPI0_CZ
#define GPIO103_OEN 0x103ce0, BIT5
#define GPIO103_IN  0x103ce0, BIT0
#define GPIO103_OUT 0x103ce0, BIT4

#define GPIO104_PAD PAD_SPI0_CK
#define GPIO104_OEN 0x103ce2, BIT5
#define GPIO104_IN  0x103ce2, BIT0
#define GPIO104_OUT 0x103ce2, BIT4

#define GPIO105_PAD PAD_SPI0_DI
#define GPIO105_OEN 0x103ce4, BIT5
#define GPIO105_IN  0x103ce4, BIT0
#define GPIO105_OUT 0x103ce4, BIT4

#define GPIO106_PAD PAD_SPI0_DO
#define GPIO106_OEN 0x103ce6, BIT5
#define GPIO106_IN  0x103ce6, BIT0
#define GPIO106_OUT 0x103ce6, BIT4

#define GPIO107_PAD PAD_SPI1_CZ
#define GPIO107_OEN 0x103ce8, BIT5
#define GPIO107_IN  0x103ce8, BIT0
#define GPIO107_OUT 0x103ce8, BIT4

#define GPIO108_PAD PAD_SPI1_CK
#define GPIO108_OEN 0x103cea, BIT5
#define GPIO108_IN  0x103cea, BIT0
#define GPIO108_OUT 0x103cea, BIT4

#define GPIO109_PAD PAD_SPI1_DI
#define GPIO109_OEN 0x103cec, BIT5
#define GPIO109_IN  0x103cec, BIT0
#define GPIO109_OUT 0x103cec, BIT4

#define GPIO110_PAD PAD_SPI1_DO
#define GPIO110_OEN 0x103cee, BIT5
#define GPIO110_IN  0x103cee, BIT0
#define GPIO110_OUT 0x103cee, BIT4

#define GPIO111_PAD PAD_PWM0
#define GPIO111_OEN 0x103c20, BIT5
#define GPIO111_IN  0x103c20, BIT0
#define GPIO111_OUT 0x103c20, BIT4

#define GPIO112_PAD PAD_PWM1
#define GPIO112_OEN 0x103c22, BIT5
#define GPIO112_IN  0x103c22, BIT0
#define GPIO112_OUT 0x103c22, BIT4

#define GPIO113_PAD PAD_SD_CLK
#define GPIO113_OEN 0x103cf0, BIT5
#define GPIO113_IN  0x103cf0, BIT0
#define GPIO113_OUT 0x103cf0, BIT4

#define GPIO114_PAD PAD_SD_CMD
#define GPIO114_OEN 0x103cf2, BIT5
#define GPIO114_IN  0x103cf2, BIT0
#define GPIO114_OUT 0x103cf2, BIT4

#define GPIO115_PAD PAD_SD_D0
#define GPIO115_OEN 0x103cf4, BIT5
#define GPIO115_IN  0x103cf4, BIT0
#define GPIO115_OUT 0x103cf4, BIT4

#define GPIO116_PAD PAD_SD_D1
#define GPIO116_OEN 0x103cf6, BIT5
#define GPIO116_IN  0x103cf6, BIT0
#define GPIO116_OUT 0x103cf6, BIT4

#define GPIO117_PAD PAD_SD_D2
#define GPIO117_OEN 0x103cf8, BIT5
#define GPIO117_IN  0x103cf8, BIT0
#define GPIO117_OUT 0x103cf8, BIT4

#define GPIO118_PAD PAD_SD_D3
#define GPIO118_OEN 0x103cfa, BIT5
#define GPIO118_IN  0x103cfa, BIT0
#define GPIO118_OUT 0x103cfa, BIT4

//PM
#define GPIO119_PAD PAD_PM_SD_CDZ
#define GPIO119_OEN 0x0f8e, BIT0
#define GPIO119_IN  0x0f8e, BIT2
#define GPIO119_OUT 0x0f8e, BIT1

#define GPIO120_PAD PAD_PM_IRIN
#define GPIO120_OEN 0x0f28, BIT0
#define GPIO120_IN  0x0f28, BIT2
#define GPIO120_OUT 0x0f28, BIT1

#define GPIO121_PAD PAD_PM_GPIO0
#define GPIO121_OEN 0x0f00, BIT0
#define GPIO121_IN  0x0f00, BIT2
#define GPIO121_OUT 0x0f00, BIT1

#define GPIO122_PAD PAD_PM_GPIO1
#define GPIO122_OEN 0x0f02, BIT0
#define GPIO122_IN  0x0f02, BIT2
#define GPIO122_OUT 0x0f02, BIT1

#define GPIO123_PAD PAD_PM_GPIO2
#define GPIO123_OEN 0x0f04, BIT0
#define GPIO123_IN  0x0f04, BIT2
#define GPIO123_OUT 0x0f04, BIT1

#define GPIO124_PAD PAD_PM_GPIO3
#define GPIO124_OEN 0x0f06, BIT0
#define GPIO124_IN  0x0f06, BIT2
#define GPIO124_OUT 0x0f06, BIT1

#define GPIO125_PAD PAD_PM_GPIO4
#define GPIO125_OEN 0x0f08, BIT0
#define GPIO125_IN  0x0f08, BIT2
#define GPIO125_OUT 0x0f08, BIT1

#define GPIO126_PAD PAD_PM_GPIO5
#define GPIO126_OEN 0x0f0a, BIT0
#define GPIO126_IN  0x0f0a, BIT2
#define GPIO126_OUT 0x0f0a, BIT1

#define GPIO127_PAD PAD_PM_GPIO6
#define GPIO127_OEN 0x0f0c, BIT0
#define GPIO127_IN  0x0f0c, BIT2
#define GPIO127_OUT 0x0f0c, BIT1

#define GPIO128_PAD PAD_PM_GPIO7
#define GPIO128_OEN 0x0f0e, BIT0
#define GPIO128_IN  0x0f0e, BIT2
#define GPIO128_OUT 0x0f0e, BIT1

#define GPIO129_PAD PAD_PM_GPIO8
#define GPIO129_OEN 0x0f10, BIT0
#define GPIO129_IN  0x0f10, BIT2
#define GPIO129_OUT 0x0f10, BIT1

#define GPIO130_PAD PAD_PM_GPIO9
#define GPIO130_OEN 0x0f12, BIT0
#define GPIO130_IN  0x0f12, BIT2
#define GPIO130_OUT 0x0f12, BIT1

#define GPIO131_PAD PAD_PM_GPIO10
#define GPIO131_OEN 0x0f14, BIT0
#define GPIO131_IN  0x0f14, BIT2
#define GPIO131_OUT 0x0f14, BIT1

#define GPIO132_PAD PAD_PM_GPIO11
#define GPIO132_OEN 0x0f16, BIT0
#define GPIO132_IN  0x0f16, BIT2
#define GPIO132_OUT 0x0f16, BIT1

#define GPIO133_PAD PAD_PM_GPIO12
#define GPIO133_OEN 0x0f18, BIT0
#define GPIO133_IN  0x0f18, BIT2
#define GPIO133_OUT 0x0f18, BIT1

#define GPIO134_PAD PAD_PM_GPIO13
#define GPIO134_OEN 0x0f1a, BIT0
#define GPIO134_IN  0x0f1a, BIT2
#define GPIO134_OUT 0x0f1a, BIT1

#define GPIO135_PAD PAD_PM_GPIO14
#define GPIO135_OEN 0x0f1c, BIT0
#define GPIO135_IN  0x0f1c, BIT2
#define GPIO135_OUT 0x0f1c, BIT1

#define GPIO136_PAD PAD_PM_GPIO15
#define GPIO136_OEN 0x0f1e, BIT0
#define GPIO136_IN  0x0f1e, BIT2
#define GPIO136_OUT 0x0f1e, BIT1

#define GPIO137_PAD PAD_PM_SPI_CZ
#define GPIO137_OEN 0x0f30, BIT0
#define GPIO137_IN  0x0f30, BIT2
#define GPIO137_OUT 0x0f30, BIT1

#define GPIO138_PAD PAD_PM_SPI_CK
#define GPIO138_OEN 0x0f32, BIT0
#define GPIO138_IN  0x0f32, BIT2
#define GPIO138_OUT 0x0f32, BIT1

#define GPIO139_PAD PAD_PM_SPI_DI
#define GPIO139_OEN 0x0f34, BIT0
#define GPIO139_IN  0x0f34, BIT2
#define GPIO139_OUT 0x0f34, BIT1

#define GPIO140_PAD PAD_PM_SPI_DO
#define GPIO140_OEN 0x0f36, BIT0
#define GPIO140_IN  0x0f36, BIT2
#define GPIO140_OUT 0x0f36, BIT1

#define GPIO141_PAD PAD_PM_SPI_WPZ
#define GPIO141_OEN 0x0f88, BIT0
#define GPIO141_IN  0x0f88, BIT2
#define GPIO141_OUT 0x0f88, BIT1

#define GPIO142_PAD PAD_PM_SPI_HLD
#define GPIO142_OEN 0x0f8a, BIT0
#define GPIO142_IN  0x0f8a, BIT2
#define GPIO142_OUT 0x0f8a, BIT1

#define GPIO143_PAD PAD_PM_LED0
#define GPIO143_OEN 0x0f94, BIT0
#define GPIO143_IN  0x0f94, BIT2
#define GPIO143_OUT 0x0f94, BIT1

#define GPIO144_PAD PAD_PM_LED1
#define GPIO144_OEN 0x0f96, BIT0
#define GPIO144_IN  0x0f96, BIT2
#define GPIO144_OUT 0x0f96, BIT1

//SAR
#define GPIO145_PAD PAD_SAR_GPIO0
#define GPIO145_OEN 0x1423, BIT0
#define GPIO145_IN  0x1425, BIT0
#define GPIO145_OUT 0x1424, BIT0

#define GPIO146_PAD PAD_SAR_GPIO1
#define GPIO146_OEN 0x1423, BIT1
#define GPIO146_IN  0x1425, BIT1
#define GPIO146_OUT 0x1424, BIT1

#define GPIO147_PAD PAD_SAR_GPIO2
#define GPIO147_OEN 0x1423, BIT2
#define GPIO147_IN  0x1425, BIT2
#define GPIO147_OUT 0x1424, BIT2

#define GPIO148_PAD PAD_SAR_GPIO3
#define GPIO148_OEN 0x1423, BIT3
#define GPIO148_IN  0x1425, BIT3
#define GPIO148_OUT 0x1424, BIT3

//ALBANY
#define GPIO149_PAD PAD_ETH_RN
#define GPIO149_OEN 0x33e2, BIT4
#define GPIO149_IN  0x33e4, BIT4
#define GPIO149_OUT 0x33e4, BIT0

#define GPIO150_PAD PAD_ETH_RP
#define GPIO150_OEN 0x33e2, BIT5
#define GPIO150_IN  0x33e4, BIT5
#define GPIO150_OUT 0x33e4, BIT1

#define GPIO151_PAD PAD_ETH_TN
#define GPIO151_OEN 0x33e2, BIT6
#define GPIO151_IN  0x33e4, BIT6
#define GPIO151_OUT 0x33e4, BIT2

#define GPIO152_PAD PAD_ETH_TP
#define GPIO152_OEN 0x33e2, BIT7
#define GPIO152_IN  0x33e4, BIT7
#define GPIO152_OUT 0x33e4, BIT3

//UTMI
#define GPIO153_PAD PAD_USB_DM
#define GPIO153_OEN 0x14210a, BIT4
#define GPIO153_IN  0x142131, BIT5
#define GPIO153_OUT 0x14210a, BIT2

#define GPIO154_PAD PAD_USB_DP
#define GPIO154_OEN 0x14210a, BIT5
#define GPIO154_IN  0x142131, BIT4
#define GPIO154_OUT 0x14210a, BIT3

#define GPIO155_PAD PAD_DM_P1
#define GPIO155_OEN 0x14290a, BIT4
#define GPIO155_IN  0x142931, BIT5
#define GPIO155_OUT 0x14290a, BIT2

#define GPIO156_PAD PAD_DP_P1
#define GPIO156_OEN 0x14290a, BIT5
#define GPIO156_IN  0x142931, BIT4
#define GPIO156_OUT 0x142924, BIT3


U32 gChipBaseAddr    = 0x1F203C00;
U32 gPmSleepBaseAddr = 0x1F001C00;
U32 gSarBaseAddr     = 0x1F002800;
U32 gRIUBaseAddr     = 0x1F000000;

#define MHal_CHIPTOP_REG(addr)  (*(volatile U8*)(gChipBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_PM_SLEEP_REG(addr) (*(volatile U8*)(gPmSleepBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_SAR_GPIO_REG(addr) (*(volatile U8*)(gSarBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))
#define MHal_RIU_REG(addr)      (*(volatile U8*)(gRIUBaseAddr + (((addr) & ~1)<<1) + (addr & 1)))

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
    __GPIO(155), __GPIO(156)
};

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
    MHal_CHIPTOP_REG(REG_ALL_PAD_IN) &= ~BIT7;
}

void MHal_CHIPTOP_WriteRegBit(U32 u32Reg, U8 u8Enable, U8 u8BitMsk)
{
    if(u8Enable)
        MHal_CHIPTOP_REG(u32Reg) |= u8BitMsk;
    else
        MHal_CHIPTOP_REG(u32Reg) &= (~u8BitMsk);
}

U8 MHal_CHIPTOP_ReadRegBit(U32 u32Reg, U8 u8BitMsk)
{
    return ((MHal_CHIPTOP_REG(u32Reg)&u8BitMsk)? 1 : 0);
}

U8 MHal_CHIPTOP_ReadRegMsk(U32 u32Reg, U8 u8BitMsk)
{
    return (MHal_CHIPTOP_REG(u32Reg)&u8BitMsk);
}

void MHal_PM_SLEEP_WriteRegBit(U32 u32Reg, U8 u8Enable, U8 u8BitMsk)
{
    if(u8Enable)
        MHal_PM_SLEEP_REG(u32Reg) |= u8BitMsk;
    else
        MHal_PM_SLEEP_REG(u32Reg) &= (~u8BitMsk);
}

U8 MHal_PM_SLEEP_ReadRegBit(U32 u32Reg, U8 u8BitMsk)
{
    return ((MHal_PM_SLEEP_REG(u32Reg)&u8BitMsk)? 1 : 0);
}

U8 MHal_PM_SLEEP_ReadRegMsk(U32 u32Reg, U8 u8BitMsk)
{
    return (MHal_PM_SLEEP_REG(u32Reg)&u8BitMsk);
}

void MHal_SAR_GPIO_WriteRegBit(U32 u32Reg, U8 u8Enable, U8 u8BitMsk)
{
    if(u8Enable)
        MHal_SAR_GPIO_REG(u32Reg) |= u8BitMsk;
    else
        MHal_SAR_GPIO_REG(u32Reg) &= (~u8BitMsk);
}

void MHal_FuartPAD_DisableFunction(MS_GPIO_NUM u32IndexGPIO)
{
    //reg_fuart_mode
    if( MHal_CHIPTOP_ReadRegMsk(REG_FUART_MODE, BIT0|BIT1|BIT2) == BIT0 ){
        printf("[gpio] Disable FUART function\n");
        MHal_CHIPTOP_WriteRegBit(REG_FUART_MODE, DISABLE, BIT0|BIT1|BIT2);
    }
    else if (PAD_FUART_RTS != u32IndexGPIO) {
        if( MHal_CHIPTOP_ReadRegMsk(REG_FUART_MODE, BIT0|BIT1|BIT2) == (BIT0|BIT1) ){
            printf("[gpio] Disable FUART function\n");
            MHal_CHIPTOP_WriteRegBit(REG_FUART_MODE, DISABLE, BIT0|BIT1|BIT2);
        }
    }
    //reg_spi0_mode
    if( MHal_CHIPTOP_ReadRegMsk(REG_SPI0_MODE, BIT0|BIT1) == (BIT0|BIT1) ){
        printf("[gpio] Disable SPI0 function\n");
        MHal_CHIPTOP_WriteRegBit(REG_SPI0_MODE, DISABLE, BIT0|BIT1);
    }
    //reg_EJ_mode
    if( MHal_CHIPTOP_ReadRegMsk(REG_EJ_MODE, BIT0|BIT1) == BIT0 ){
        printf("[gpio] Disable EJ_MODE function\n");
        MHal_CHIPTOP_WriteRegBit(REG_EJ_MODE,DISABLE, BIT0|BIT1);
    }
    //REG_TEST_IN_MODE
    MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
}

void MHal_SPI0PAD_DisableFunction(void)
{
    //REG_TEST_IN_MODE
    MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
    MHal_CHIPTOP_WriteRegBit(REG_TEST_OUT_MODE,DISABLE,BIT5|BIT4);

    //spi0_mode
    if( MHal_CHIPTOP_ReadRegMsk(REG_SPI0_MODE, BIT1|BIT0) == BIT0 ){
        printf("[gpio] Disable SPI0 function\n");
        MHal_CHIPTOP_WriteRegBit(REG_SPI0_MODE, DISABLE, BIT0|BIT1);
    }

    //reg_EJ_mode
    if( MHal_CHIPTOP_ReadRegMsk(REG_EJ_MODE,BIT1|BIT0) == BIT1 ){
        printf("[gpio] Disable EJ_MODE function\n");
        MHal_CHIPTOP_WriteRegBit(REG_EJ_MODE,DISABLE,BIT1|BIT0);
    }
}

void MHal_GPIO_Pad_Set(MS_GPIO_NUM u32IndexGPIO)
{
    switch(u32IndexGPIO)
    {
    case PAD_GPIO0:
    case PAD_GPIO1:
    case PAD_GPIO2:
    case PAD_GPIO3:
        //MHal_CHIPTOP_WriteRegBit(REG_FUART_MODE,DISABLE,BIT0|BIT1);
        //MHal_CHIPTOP_WriteRegBit(REG_I2S_MODE,DISABLE,BIT2);
        break;
    case PAD_GPIO4:
    case PAD_GPIO5:
        //MHal_CHIPTOP_WriteRegBit(REG_UART0_MODE,DISABLE,BIT5|BIT4);
        //MHal_CHIPTOP_WriteRegBit(REG_DMIC_MODE,DISABLE,BIT0);
        break;
    case PAD_GPIO6:
        //MHal_CHIPTOP_WriteRegBit(REG_UART1_MODE,DISABLE,BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_DMIC_MODE,DISABLE,BIT0);
        break;
    case PAD_GPIO7:
        //MHal_CHIPTOP_WriteRegBit(REG_UART1_MODE,DISABLE,BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_ETH_MODE,DISABLE,BIT2);
        break;
    case PAD_GPIO8:
    case PAD_GPIO9:
    case PAD_GPIO10:
    case PAD_GPIO11:
        //MHal_CHIPTOP_WriteRegBit(REG_SPI0_MODE,DISABLE,BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_ETH_MODE,DISABLE,BIT2);
        break;
    case PAD_GPIO12:
        //MHal_CHIPTOP_WriteRegBit(REG_SPI1_MODE,DISABLE,BIT5|BIT4);
        //MHal_CHIPTOP_WriteRegBit(REG_PWM0_MODE,DISABLE,BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_ETH_MODE,DISABLE,BIT2);
        break;
    case PAD_GPIO13:
        //MHal_CHIPTOP_WriteRegBit(REG_SPI1_MODE,DISABLE,BIT5|BIT4);
        //MHal_CHIPTOP_WriteRegBit(REG_PWM1_MODE,DISABLE,BIT3|BIT2);
        //MHal_CHIPTOP_WriteRegBit(REG_ETH_MODE,DISABLE,BIT2);
        break;
    case PAD_GPIO14:
        //MHal_CHIPTOP_WriteRegBit(REG_SPI1_MODE,DISABLE,BIT5|BIT4);
        //MHal_CHIPTOP_WriteRegBit(REG_PWM2_MODE,DISABLE,BIT4);
        //MHal_CHIPTOP_WriteRegBit(REG_ETH_MODE,DISABLE,BIT2);
        break;
    case PAD_GPIO15:
        //MHal_CHIPTOP_WriteRegBit(REG_SPI1_MODE,DISABLE,BIT5|BIT4);
        //MHal_CHIPTOP_WriteRegBit(REG_PWM3_MODE,DISABLE,BIT6);
        //MHal_CHIPTOP_WriteRegBit(REG_ETH_MODE,DISABLE,BIT2);
        break;
    case PAD_PWM0:
        //MHal_CHIPTOP_WriteRegBit(REG_I2C0_MODE,DISABLE,BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_I2C1_MODE,DISABLE,BIT5|BIT4);
        //MHal_CHIPTOP_WriteRegBit(REG_PWM0_MODE,DISABLE,BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_TTL_MODE,DISABLE,BIT6);
        break;

    case PAD_PWM1:
        //MHal_CHIPTOP_WriteRegBit(REG_I2C0_MODE,DISABLE,BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_I2C1_MODE,DISABLE,BIT5|BIT4);
        //MHal_CHIPTOP_WriteRegBit(REG_PWM1_MODE,DISABLE,BIT3|BIT2);
        break;
    case PAD_FUART_RX:
        //PWM0
        if( MHal_CHIPTOP_ReadRegMsk(REG_PWM0_MODE, BIT2|BIT1|BIT0) == (BIT1|BIT0) ){
            printf("[gpio] Disable PWM0 function\n");
            MHal_CHIPTOP_WriteRegBit(REG_PWM0_MODE,DISABLE, BIT2|BIT1|BIT0);
        }
        //reg_uart0_mode
        if( MHal_CHIPTOP_ReadRegMsk(REG_UART0_MODE, BIT6|BIT5|BIT4) == BIT5 ){
            printf("[gpio] Disable UART0 function\n");
            MHal_CHIPTOP_WriteRegBit(REG_UART0_MODE, DISABLE, BIT6|BIT5|BIT4);
        }
        MHal_FuartPAD_DisableFunction(u32IndexGPIO);
        break;

    case PAD_FUART_TX:
        //PWM1
        if( MHal_CHIPTOP_ReadRegMsk(REG_PWM1_MODE, BIT5|BIT4|BIT3) == BIT4 ){
            printf("[gpio] Disable PWM1 function\n");
            MHal_CHIPTOP_WriteRegBit(REG_PWM1_MODE,DISABLE, BIT5|BIT4|BIT3);
        }

        //reg_uart0_mode
        if( MHal_CHIPTOP_ReadRegMsk(REG_UART0_MODE, BIT6|BIT5|BIT4) == BIT5 ){
            printf("[gpio] Disable UART0 function\n");
            MHal_CHIPTOP_WriteRegBit(REG_UART0_MODE, DISABLE, BIT6|BIT5|BIT4);
        }
        MHal_FuartPAD_DisableFunction(u32IndexGPIO);
        break;

    case PAD_FUART_CTS:
        //PWM2
        if( MHal_CHIPTOP_ReadRegMsk(REG_PWM2_MODE, BIT2|BIT1|BIT0) == BIT1 ){
            printf("[gpio] Disable PWM2 function\n");
            MHal_CHIPTOP_WriteRegBit(REG_PWM2_MODE,DISABLE, BIT2|BIT1|BIT0);
        }

        //reg_uart1_mode
        if( MHal_CHIPTOP_ReadRegMsk(REG_UART1_MODE, BIT1|BIT0) == BIT1 ){
            printf("[gpio] Disable UART1 function\n");
            MHal_CHIPTOP_WriteRegBit(REG_UART1_MODE, DISABLE, BIT1|BIT0);
        }
        MHal_FuartPAD_DisableFunction(u32IndexGPIO);
        break;

    case PAD_FUART_RTS:
        //PWM3
        if( MHal_CHIPTOP_ReadRegMsk(REG_PWM3_MODE, BIT6|BIT5|BIT4) == BIT5 ){
            printf("[gpio] Disable PWM3 function\n");
            MHal_CHIPTOP_WriteRegBit(REG_PWM3_MODE,DISABLE, BIT6|BIT5|BIT4);
        }

        //reg_uart1_mode
        if( MHal_CHIPTOP_ReadRegMsk(REG_UART1_MODE, BIT0|BIT1) == BIT1 ){
            printf("[gpio] Disable UART1 function\n");
            MHal_CHIPTOP_WriteRegBit(REG_UART1_MODE, DISABLE, BIT0|BIT1);
        }
        MHal_FuartPAD_DisableFunction(u32IndexGPIO);
        break;

    case PAD_UART0_RX:
    case PAD_UART0_TX:
        //MHal_CHIPTOP_WriteRegBit(REG_UART0_MODE,DISABLE,BIT5|BIT4);
        break;
    case PAD_UART1_RX:
    case PAD_UART1_TX:
        //MHal_CHIPTOP_WriteRegBit(REG_UART1_MODE,DISABLE,BIT1|BIT0);
        break;
    case PAD_SNR0_D0:
    case PAD_SNR0_D1:
        //MHal_CHIPTOP_WriteRegBit(REG_I2C0_MODE,DISABLE,BIT1|BIT0);
        break;
    case PAD_SNR0_D2:
        //MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_TEST_OUT_MODE,DISABLE,BIT5|BIT4);
        //MHal_CHIPTOP_WriteRegBit(REG_SR_MODE,DISABLE,BIT2|BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_CCIR_MODE,DISABLE,BIT4);
        break;
    case PAD_SNR0_D3:
    case PAD_SNR0_D4:
    case PAD_SNR0_D5:
    case PAD_SNR0_D6:
    case PAD_SNR0_D7:
    case PAD_SNR0_D8:
    case PAD_SNR0_D9:
        //MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_TEST_OUT_MODE,DISABLE,BIT5|BIT4);
        //MHal_CHIPTOP_WriteRegBit(REG_SR_MODE,DISABLE,BIT2|BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_CCIR_MODE,DISABLE,BIT4);
        break;
    case PAD_SNR0_D10:
    case PAD_SNR0_D11:
        //MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_TEST_OUT_MODE,DISABLE,BIT5|BIT4);
        break;
    case PAD_SNR0_GPIO0:
    case PAD_SNR0_GPIO1:
    case PAD_SNR0_GPIO2:
    case PAD_SNR0_GPIO3:
    case PAD_SNR0_GPIO4:
    case PAD_SNR0_GPIO5:
    case PAD_SNR0_GPIO6:
        //MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_TEST_OUT_MODE,DISABLE,BIT5|BIT4);
        //MHal_CHIPTOP_WriteRegBit(REG_SR_MODE,DISABLE,BIT2|BIT1|BIT0);
        break;
    case PAD_NAND_ALE:
        break;
    case PAD_NAND_CLE:
    case PAD_NAND_CEZ:
    case PAD_NAND_WEZ:
    case PAD_NAND_WPZ:
    case PAD_NAND_REZ:
    case PAD_NAND_RBZ:
        //MHal_CHIPTOP_WriteRegBit(REG_NAND_MODE,DISABLE,BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_SD_MODE,DISABLE,BIT2);
        //MHal_CHIPTOP_WriteRegBit(REG_TTL_MODE,DISABLE,BIT6);
        break;
    case PAD_NAND_DA0:
    case PAD_NAND_DA1:
    case PAD_NAND_DA2:
    case PAD_NAND_DA3:
    case PAD_NAND_DA4:
    case PAD_NAND_DA5:
    case PAD_NAND_DA6:
    case PAD_NAND_DA7:
        //MHal_CHIPTOP_WriteRegBit(REG_NAND_MODE,DISABLE,BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_TTL_MODE,DISABLE,BIT6);
        break;
    case PAD_SD_CLK:
    case PAD_SD_CMD:
        //MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
        break;
    case PAD_SD_D0:
    case PAD_SD_D1:
    case PAD_SD_D2:
        //MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_TEST_OUT_MODE,DISABLE,BIT5|BIT4);
        break;
    case PAD_SD_D3:
        //MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
        //MHal_CHIPTOP_WriteRegBit(REG_TEST_OUT_MODE,DISABLE,BIT5|BIT4);
        //MHal_CHIPTOP_WriteRegBit(REG_SPI1_MODE,DISABLE,BIT5|BIT4);
        //MHal_CHIPTOP_WriteRegBit(REG_SDIO_MODE,DISABLE,BIT0);
        break;
    case PAD_I2C0_SCL:
    case PAD_I2C0_SDA:
        //MHal_CHIPTOP_WriteRegBit(REG_TEST_IN_MODE,DISABLE,BIT1|BIT0);
        break;
    case PAD_I2C1_SCL:
    case PAD_I2C1_SDA:
        //MHal_CHIPTOP_WriteRegBit(REG_I2C1_MODE,DISABLE,BIT5|BIT4);
        break;

    case PAD_SPI0_CZ:
        //PWM4
        if( MHal_CHIPTOP_ReadRegMsk(REG_PWM4_MODE, BIT1|BIT0) == BIT1 ){
            printf("[gpio] Disable PWM4 function\n");
            MHal_CHIPTOP_WriteRegBit(REG_PWM4_MODE,DISABLE, BIT1|BIT0);
        }
        MHal_SPI0PAD_DisableFunction();
        break;

    case PAD_SPI0_CK:
        //PWM5
        if( MHal_CHIPTOP_ReadRegMsk(REG_PWM5_MODE, BIT4|BIT3) == BIT4 ){
            printf("[gpio] Disable PWM5 function\n");
            MHal_CHIPTOP_WriteRegBit(REG_PWM5_MODE,DISABLE, BIT4|BIT3);
        }
        MHal_SPI0PAD_DisableFunction();
        break;

    case PAD_SPI0_DI:
        //PWM6
        if( MHal_CHIPTOP_ReadRegMsk(REG_PWM6_MODE, BIT1|BIT0) == BIT1 ){
            printf("[gpio] Disable PWM6 function\n");
            MHal_CHIPTOP_WriteRegBit(REG_PWM6_MODE,DISABLE, BIT1|BIT0);
        }
        MHal_SPI0PAD_DisableFunction();
        break;

    case PAD_SPI0_DO:
        //PWM7
        if( MHal_CHIPTOP_ReadRegMsk(REG_PWM7_MODE, BIT5|BIT4) == BIT5 ){
            printf("[gpio] Disable PWM7 function\n");
            MHal_CHIPTOP_WriteRegBit(REG_PWM7_MODE,DISABLE, BIT5|BIT4);
        }
        MHal_SPI0PAD_DisableFunction();
        break;

    case PAD_SPI1_CZ:
    case PAD_SPI1_CK:
    case PAD_SPI1_DI:
    case PAD_SPI1_DO:
        //MHal_CHIPTOP_WriteRegBit(REG_SPI1_MODE,DISABLE,BIT5|BIT4);
        //MHal_CHIPTOP_WriteRegBit(REG_TTL_MODE,DISABLE,BIT6);
        break;
    case PAD_PM_IRIN:
        MHal_PM_SLEEP_WriteRegBit(REG_IRIN_MODE,ENABLE,BIT4);
        break;
    case PAD_PM_GPIO4:
        MHal_PM_SLEEP_WriteRegBit(REG_PMLOCK_L_MODE,ENABLE,0xBE);
        MHal_PM_SLEEP_WriteRegBit(REG_PMLOCK_H_MODE,ENABLE,0xBA);
        break;
    case PAD_PM_LED0:
    case PAD_PM_LED1:
        if( MHal_PM_SLEEP_ReadRegMsk(REG_LED_MODE,BIT4|BIT5) == BIT4 ){
            printf("[gpio] Disable ethernet ACK/LINK led\n");
            MHal_PM_SLEEP_WriteRegBit(REG_LED_MODE,DISABLE,BIT4);
        }
        break;
    case PAD_SAR_GPIO0:
        MHal_SAR_GPIO_WriteRegBit(REG_SAR_MODE,DISABLE,BIT0);
        break;
    case PAD_SAR_GPIO1:
        MHal_SAR_GPIO_WriteRegBit(REG_SAR_MODE,DISABLE,BIT1);
        break;
    case PAD_SAR_GPIO2:
        MHal_SAR_GPIO_WriteRegBit(REG_SAR_MODE,DISABLE,BIT2);
        break;
    case PAD_SAR_GPIO3:
        MHal_SAR_GPIO_WriteRegBit(REG_SAR_MODE,DISABLE,BIT3);
        break;
    default:
        break;

    }
}

int MHal_GPIO_PadGroupMode_Set(U32 u32PadMode)
{
    (void)u32PadMode;
    return -1; // not support
}

int MHal_GPIO_PadVal_Set(MS_GPIO_NUM u32IndexGPIO, U32 u32PadMode)
{
    (void)u32IndexGPIO;
    (void)u32PadMode;
    return -1; // not support
}

void MHal_GPIO_Pad_Oen(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen) &= (~gpio_table[u32IndexGPIO].m_oen);
    }
    else
    {
        MS_ASSERT(0);
    }
}

void MHal_GPIO_Pad_Odn(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen) |= gpio_table[u32IndexGPIO].m_oen;
    }
    else
    {
        MS_ASSERT(0);
    }
}

int MHal_GPIO_Pad_Level(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        return ((MHal_RIU_REG(gpio_table[u32IndexGPIO].r_in)&gpio_table[u32IndexGPIO].m_in)? 1 : 0);
    }
    else
    {
        MS_ASSERT(0);
        return 0;
    }
}

U8 MHal_GPIO_Pad_InOut(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        return ((MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen)&gpio_table[u32IndexGPIO].m_oen)? 1 : 0);
    }
    else
    {
        MS_ASSERT(0);
        return 0;
    }
}

void MHal_GPIO_Pull_High(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_out) |= gpio_table[u32IndexGPIO].m_out;
    }
    else
    {
        MS_ASSERT(0);
    }
}

void MHal_GPIO_Pull_Low(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_out) &= (~gpio_table[u32IndexGPIO].m_out);
    }
    else
    {
        MS_ASSERT(0);
    }
}

void MHal_GPIO_Set_High(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen) &= (~gpio_table[u32IndexGPIO].m_oen);
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_out) |= gpio_table[u32IndexGPIO].m_out;
    }
    else
    {
        MS_ASSERT(0);
    }
}

void MHal_GPIO_Set_Low(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen) &= (~gpio_table[u32IndexGPIO].m_oen);
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_out) &= (~gpio_table[u32IndexGPIO].m_out);
    }
    else
    {
        MS_ASSERT(0);
    }
}

void MHal_GPIO_Set_Input(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen) |= (gpio_table[u32IndexGPIO].m_oen);
    }
    else
    {
        MS_ASSERT(0);
    }
}

void MHal_GPIO_Set_Output(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen) &= (gpio_table[u32IndexGPIO].m_oen);
    }
    else
    {
        MS_ASSERT(0);
    }
}

int MHal_GPIO_Get_InOut(MS_GPIO_NUM u32IndexGPIO)
{
    if (u32IndexGPIO >= 0 && u32IndexGPIO < END_GPIO_NUM)
    {
        return ((MHal_RIU_REG(gpio_table[u32IndexGPIO].r_oen)&gpio_table[u32IndexGPIO].m_oen)? 1 : 0);
    }
    else
    {
        MS_ASSERT(0);
        return 0;
    }
}

#endif //_MHAL_GPIO_C_
