/*
* mhal_pinmux.c- Sigmastar
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
#include "ms_platform.h"
#include "mdrv_types.h"
#include "mhal_gpio.h"
#include "padmux.h"
#include "gpio.h"

//==============================================================================
//
//                              MACRO DEFINE
//
//==============================================================================

#define BASE_RIU_PA                         0xFD000000
#define PMSLEEP_BANK                        0x000E00
#define PM_SAR_BANK                         0x001400
#define ALBANY1_BANK                        0x151500
#define ALBANY2_BANK                        0x151600
#define CHIPTOP_BANK                        0x101E00
#define PADTOP_BANK                         0x103C00
#define PM_PADTOP_BANK                      0x003F00
#define UTMI0_BANK                          0x142100

#define _GPIO_W_WORD(addr,val)              {(*(volatile u16*)(addr)) = (u16)(val);}
#define _GPIO_W_WORD_MASK(addr,val,mask)    {(*(volatile u16*)(addr)) = ((*(volatile u16*)(addr)) & ~(mask)) | ((u16)(val) & (mask));}
#define _GPIO_R_BYTE(addr)                  (*(volatile u8*)(addr))
#define _GPIO_R_WORD_MASK(addr,mask)        ((*(volatile u16*)(addr)) & (mask))

#define GET_BASE_ADDR_BY_BANK(x, y)         ((x) + ((y) << 1))
#define _RIUA_8BIT(bank , offset)           GET_BASE_ADDR_BY_BANK(BASE_RIU_PA, bank) + (((offset) & ~1)<<1) + ((offset) & 1)
#define _RIUA_16BIT(bank , offset)          GET_BASE_ADDR_BY_BANK(BASE_RIU_PA, bank) + ((offset)<<2)

/* Non PM Pad : CHIPTOP_BANK */
#define REG_TEST_IN_MODE        0x12
    #define REG_TEST_IN_MODE_MASK   BIT1|BIT0
#define REG_TEST_OUT_MODE       0x12
    #define REG_TEST_OUT_MODE_MASK  BIT5|BIT4

/* Non PM Pad : PADTOP_BANK */
#define REG_DMIC_MODE                   0x60
    #define REG_DMIC_MODE_MASK              BIT0|BIT1|BIT2|BIT3
#define REG_BT656_OUT_MODE              0x60
    #define REG_BT656_OUT_MODE_MASK         BIT4|BIT5
#define REG_EJ_MODE                     0x60
    #define REG_EJ_MODE_MASK                BIT7|BIT8
#define REG_EMMC0_4B_MODE               0x61
    #define REG_EMMC0_4B_MODE_MASK          BIT0
#define REG_EMMC0_8B_MODE               0x61
    #define REG_EMMC0_8B_MODE_MASK          BIT1|BIT2
#define REG_EMMC0_RST_MODE              0x61
    #define REG_EMMC0_RST_MODE_MASK         BIT3
#define REG_EMMC1_4B_MODE               0x61
    #define REG_EMMC1_4B_MODE_MASK          BIT4|BIT5
#define REG_EMMC1_RST_MODE              0x61
    #define REG_EMMC1_RST_MODE_MASK         BIT6|BIT7
#define REG_I2S_MCK_MODE                0x62
    #define REG_I2S_MCK_MODE_MASK           BIT0|BIT1|BIT2
#define REG_I2S_RX_MODE                 0x62
    #define REG_I2S_RX_MODE_MASK            BIT4|BIT5|BIT6
#define REG_I2S_TX_MODE                 0x62
    #define REG_I2S_TX_MODE_MASK            BIT8|BIT9|BIT10
#define REG_I2S_RXTX_MODE               0x62
    #define REG_I2S_RXTX_MODE_MASK          BIT11|BIT12|BIT13
#define REG_LED0_MODE                   0x63
    #define REG_LED0_MODE_MASK              BIT0|BIT1|BIT2
#define REG_LED1_MODE                   0x63
    #define REG_LED1_MODE_MASK              BIT4|BIT5|BIT6
#define REG_MIPI_TX_MODE                0x64
    #define REG_MIPI_TX_MODE_MASK           BIT0|BIT1|BIT2
#define REG_OTP_TEST                    0x64
    #define REG_OTP_TEST_MASK               BIT8
#define REG_PWM0_MODE                   0x65
    #define REG_PWM0_MODE_MASK              BIT0|BIT1|BIT2
#define REG_PWM1_MODE                   0x65
    #define REG_PWM1_MODE_MASK              BIT4|BIT5|BIT6
#define REG_PWM2_MODE                   0x65
    #define REG_PWM2_MODE_MASK              BIT8|BIT9|BIT10
#define REG_PWM3_MODE                   0x65
    #define REG_PWM3_MODE_MASK              BIT12|BIT13|BIT14
#define REG_PWM4_MODE                   0x66
    #define REG_PWM4_MODE_MASK              BIT0|BIT1|BIT2
#define REG_PWM5_MODE                   0x66
    #define REG_PWM5_MODE_MASK              BIT4|BIT5|BIT6
#define REG_PWM6_MODE                   0x66
    #define REG_PWM6_MODE_MASK              BIT8|BIT9|BIT10
#define REG_PWM7_MODE                   0x66
    #define REG_PWM7_MODE_MASK              BIT12|BIT13|BIT14
#define REG_PWM8_MODE                   0x67
    #define REG_PWM8_MODE_MASK              BIT0|BIT1|BIT2|BIT3
#define REG_PWM9_MODE                   0x67
    #define REG_PWM9_MODE_MASK              BIT4|BIT5|BIT6|BIT7
#define REG_SD0_MODE                    0x67
    #define REG_SD0_MODE_MASK               BIT8
#define REG_SD0_CDZ_MODE                0x67
    #define REG_SD0_CDZ_MODE_MASK           BIT9
#define REG_SD1_MODE                    0x67
    #define REG_SD1_MODE_MASK               BIT12|BIT13
#define REG_SPI0_MODE                   0x68
    #define REG_SPI0_MODE_MASK              BIT0|BIT1|BIT2
#define REG_SPI1_MODE                   0x68
    #define REG_SPI1_MODE_MASK              BIT4|BIT5|BIT6
#define REG_SD1_CDZ_MODE                0x68
    #define REG_SD1_CDZ_MODE_MASK           BIT8|BIT9
#define REG_SR00_MIPI_MODE              0x69
    #define REG_SR00_MIPI_MODE_MASK         BIT0|BIT1|BIT2
#define REG_SR00_CTRL_MODE              0x69
    #define REG_SR00_CTRL_MODE_MASK         BIT4
#define REG_SR00_MCLK_MODE              0x69
    #define REG_SR00_MCLK_MODE_MASK         BIT5
#define REG_SR00_RST_MODE               0x69
    #define REG_SR00_RST_MODE_MASK          BIT6
#define REG_SR00_PDN_MODE               0x69
    #define REG_SR00_PDN_MODE_MASK          BIT8|BIT9
#define REG_SR01_MIPI_MODE              0x69
    #define REG_SR01_MIPI_MODE_MASK         BIT10
#define REG_SR01_CTRL_MODE              0x69
    #define REG_SR01_CTRL_MODE_MASK         BIT11
#define REG_SR01_MCLK_MODE              0x69
    #define REG_SR01_MCLK_MODE_MASK         BIT12
#define REG_SR01_RST_MODE               0x69
    #define REG_SR01_RST_MODE_MASK          BIT13|BIT14
#define REG_SR0_BT656_MODE              0x6a
    #define REG_SR0_BT656_MODE_MASK         BIT0|BIT1
#define REG_SR0_BT601_MODE              0x6a
    #define REG_SR0_BT601_MODE_MASK         BIT2|BIT3
#define REG_SR1_BT656_MODE              0x6b
    #define REG_SR1_BT656_MODE_MASK         BIT0
#define REG_SR1_BT601_MODE              0x6b
    #define REG_SR1_BT601_MODE_MASK         BIT1
#define REG_SR1_MIPI_MODE               0x6b
    #define REG_SR1_MIPI_MODE_MASK          BIT4|BIT5|BIT6
#define REG_SR1_CTRL_MODE               0x6b
    #define REG_SR1_CTRL_MODE_MASK          BIT8
#define REG_SR1_MCLK_MODE               0x6b
    #define REG_SR1_MCLK_MODE_MASK          BIT9|BIT10
#define REG_SR1_RST_MODE                0x6b
    #define REG_SR1_RST_MODE_MASK           BIT11|BIT12
#define REG_TTL16_MODE                  0x6c
    #define REG_TTL16_MODE_MASK             BIT0|BIT1|BIT2
#define REG_TTL24_MODE                  0x6c
    #define REG_TTL24_MODE_MASK             BIT4
#define REG_UART0_MODE                  0x6d
    #define REG_UART0_MODE_MASK             BIT0|BIT1|BIT2
#define REG_UART1_MODE                  0x6d
    #define REG_UART1_MODE_MASK             BIT4|BIT5|BIT6
#define REG_UART2_MODE                  0x6d
    #define REG_UART2_MODE_MASK             BIT8|BIT9|BIT10
#define REG_ETH_MODE                    0x6e
    #define REG_ETH_MODE_MASK               BIT0
#define REG_FUART_MODE                  0x6e
    #define REG_FUART_MODE_MASK             BIT4|BIT5|BIT6
#define REG_I2C0_MODE                   0x6f
    #define REG_I2C0_MODE_MASK              BIT0|BIT1|BIT2
#define REG_I2C1_MODE                   0x6f
    #define REG_I2C1_MODE_MASK              BIT4|BIT5
#define REG_I2C2_MODE                   0x6f
    #define REG_I2C2_MODE_MASK              BIT8|BIT9|BIT10
#define REG_SPI_GPIO                    0x70
    #define REG_SPI_GPIO_MASK               BIT0
#define REG_SPICSZ1_GPIO                0x70
    #define REG_SPICSZ1_GPIO_MASK           BIT1
#define REG_SPICSZ2_GPIO                0x70
    #define REG_SPICSZ2_GPIO_MASK           BIT2
#define REG_SPIHOLDN_GPIO               0x70
    #define REG_SPIHOLDN_GPIO_MASK          BIT3
#define REG_SPIWPN_GPIO                 0x70
    #define REG_SPIWPN_GPIO_MASK            BIT4
#define REG_DLA_EJ_MODE                 0x71
    #define REG_DLA_EJ_MODE_MASK            BIT0|BIT1

/* PM Sleep : PMSLEEP_BANK */
#define REG_PM_GPIO_PM_LOCK        0x12
    #define REG_PM_GPIO_PM_LOCK_MASK   0xFFFF
#define REG_PM_GPIO_PM4_INV     0x1c
    #define REG_PM_GPIO_PM4_INV_MASK BIT1
#define REG_PM_LINK_ISOEN2GPIO4 0x1c
    #define REG_PM_LINK_ISOEN2GPIO4_MASK BIT2
#define REG_PM_LINK_WKINT2GPIO4 0x1c
    #define REG_PM_LINK_WKINT2GPIO4_MASK BIT3
#define REG_PM_IR_IS_GPIO       0x1c
    #define REG_PM_IR_IS_GPIO_MASK  BIT4

#define REG_PM_SD_CDZ_MODE      0x28
    #define REG_PM_SD_CDZ_MODE_MASK BIT14
#define REG_PM_SPI_IS_GPIO      0x35
    #define REG_PM_SPI_IS_GPIO_MASK BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0
    #define REG_PM_SPI_GPIO_MASK            BIT0
    #define REG_PM_SPICSZ1_GPIO_MASK        BIT2
    #define REG_PM_SPICSZ2_GPIO_MASK        BIT3
    #define REG_PM_SPIWPN_GPIO_MASK         BIT4
    
#define REG_PM_SPICSZ1_GPIO     REG_PM_SPI_IS_GPIO
#define REG_PM_SPICSZ2_GPIO     REG_PM_SPI_IS_GPIO
#define REG_PM_SPI_GPIO         REG_PM_SPI_IS_GPIO
#define REG_PM_SPIWPN_GPIO      REG_PM_SPI_IS_GPIO

//#define REG_PM_SPIHOLDN_MODE    REG_PM_SPI_IS_GPIO


// PM PADTOP
#define REG_PM_I2CM_MODE          0x50
    #define REG_PM_I2CM_MODE_MASK   BIT1|BIT0

#define REG_PM_PWM0_MODE          0x51
    #define REG_PM_PWM0_MODE_MASK   BIT1|BIT0

#define REG_PM_SPIHOLDN_MODE      0x52
    #define REG_PM_SPIHOLDN_MODE_MASK       BIT0

#define REG_PM_UART1_MODE         0x53
    #define REG_PM_UART1_MODE_MASK          BIT1|BIT0
#define REG_PM_VID_MODE           0x53
    #define REG_PM_VID_MODE_MASK            BIT3|BIT2
	
#define REG_PM_PIR_DIR_LINK_MODE  0x54
    #define REG_PM_PIR_DIR_LINK_MODE_MASK   BIT1|BIT0
#define REG_PM_PIR_SERIN_MODE     0x54
    #define REG_PM_PIR_SERIN_MODE_MASK      BIT3|BIT2

#define REG_PM_PM_PAD_EXT_MODE_0  0x55
    #define REG_PM_PM_PAD_EXT_MODE_0_MASK   BIT0
#define REG_PM_PM_PAD_EXT_MODE_1  0x55
    #define REG_PM_PM_PAD_EXT_MODE_1_MASK   BIT1
#define REG_PM_PM_PAD_EXT_MODE_2  0x55
    #define REG_PM_PM_PAD_EXT_MODE_2_MASK   BIT2
#define REG_PM_PM_PAD_EXT_MODE_3  0x55
    #define REG_PM_PM_PAD_EXT_MODE_3_MASK   BIT3
#define REG_PM_PM_PAD_EXT_MODE_4  0x55
    #define REG_PM_PM_PAD_EXT_MODE_4_MASK   BIT4
#define REG_PM_PM_PAD_EXT_MODE_5  0x55
    #define REG_PM_PM_PAD_EXT_MODE_5_MASK   BIT5
#define REG_PM_PM_PAD_EXT_MODE_6  0x55
    #define REG_PM_PM_PAD_EXT_MODE_6_MASK   BIT6
#define REG_PM_PM_PAD_EXT_MODE_7  0x55
    #define REG_PM_PM_PAD_EXT_MODE_7_MASK   BIT7
#define REG_PM_PM_PAD_EXT_MODE_8  0x55
    #define REG_PM_PM_PAD_EXT_MODE_8_MASK   BIT8
#define REG_PM_PM_PAD_EXT_MODE_9  0x55
    #define REG_PM_PM_PAD_EXT_MODE_9_MASK   BIT9
#define REG_PM_PM_PAD_EXT_MODE_10  0x55
    #define REG_PM_PM_PAD_EXT_MODE_10_MASK  BIT10
#define REG_PM_PM_PAD_EXT_MODE_11  0x55
    #define REG_PM_PM_PAD_EXT_MODE_11_MASK  BIT11
#define REG_PM_PM_PAD_EXT_MODE_12  0x55
    #define REG_PM_PM_PAD_EXT_MODE_12_MASK  BIT12
#define REG_PM_PM_PAD_EXT_MODE_13  0x55
    #define REG_PM_PM_PAD_EXT_MODE_13_MASK  BIT13
#define REG_PM_PM_PAD_EXT_MODE_14  0x55
    #define REG_PM_PM_PAD_EXT_MODE_14_MASK  BIT14
#define REG_PM_PM_PAD_EXT_MODE_15  0x55
    #define REG_PM_PM_PAD_EXT_MODE_15_MASK  BIT15
#define REG_PM_PM_PAD_EXT_MODE_16  0x56
    #define REG_PM_PM_PAD_EXT_MODE_16_MASK  BIT0
#define REG_PM_PM_PAD_EXT_MODE_17  0x56
    #define REG_PM_PM_PAD_EXT_MODE_17_MASK  BIT1
#define REG_PM_PM_PAD_EXT_MODE_18  0x56
    #define REG_PM_PM_PAD_EXT_MODE_18_MASK  BIT2
#define REG_PM_PM_PAD_EXT_MODE_19  0x56
    #define REG_PM_PM_PAD_EXT_MODE_19_MASK  BIT3
#define REG_PM_PM_PAD_EXT_MODE_20  0x56
    #define REG_PM_PM_PAD_EXT_MODE_20_MASK  BIT4
#define REG_PM_PM_PAD_EXT_MODE_21  0x56
    #define REG_PM_PM_PAD_EXT_MODE_21_MASK  BIT5
#define REG_PM_PM_PAD_EXT_MODE_22  0x56
    #define REG_PM_PM_PAD_EXT_MODE_22_MASK  BIT6

#define REG_PM_PM51_UART_MODE     0x65
    #define REG_PM_PM51_UART_MODE_MASK      BIT1|BIT0
#define REG_PM_UART_IS_GPIO       0x65
    #define REG_PM_UART_IS_GPIO_MASK        BIT4

// GPIO ENABLE
#define REG_PM_UART_RX1_GPIO_MODE       0x19
    #define REG_PM_UART_RX1_GPIO_MODE_MASK  BIT3
#define REG_PM_UART_TX1_GPIO_MODE       0x20
    #define REG_PM_UART_TX1_GPIO_MODE_MASK  BIT3
#define REG_PM_UART_RX_GPIO_MODE        0x21
    #define REG_PM_UART_RX_GPIO_MODE_MASK   BIT3
#define REG_PM_UART_TX_GPIO_MODE        0x22
    #define REG_PM_UART_TX_GPIO_MODE_MASK   BIT3
#define REG_PM_I2CM_SCL_GPIO_MODE       0x11
    #define REG_PM_I2CM_SCL_GPIO_MODE_MASK  BIT3
#define REG_PM_I2CM_SDA_GPIO_MODE       0x12
    #define REG_PM_I2CM_SDA_GPIO_MODE_MASK  BIT3
#define REG_PM_GPIO0_GPIO_MODE          0x00
    #define REG_PM_GPIO0_GPIO_MODE_MASK     BIT3
#define REG_PM_GPIO1_GPIO_MODE          0x01
    #define REG_PM_GPIO1_GPIO_MODE_MASK     BIT3
#define REG_PM_GPIO2_GPIO_MODE          0x02
    #define REG_PM_GPIO2_GPIO_MODE_MASK     BIT3
#define REG_PM_GPIO3_GPIO_MODE          0x03
    #define REG_PM_GPIO3_GPIO_MODE_MASK     BIT3
#define REG_PM_GPIO4_GPIO_MODE          0x04
    #define REG_PM_GPIO4_GPIO_MODE_MASK     BIT3
#define REG_PM_GPIO5_GPIO_MODE          0x05
    #define REG_PM_GPIO5_GPIO_MODE_MASK     BIT3
#define REG_PM_GPIO6_GPIO_MODE          0x06
    #define REG_PM_GPIO6_GPIO_MODE_MASK     BIT3
#define REG_PM_GPIO7_GPIO_MODE          0x07
    #define REG_PM_GPIO7_GPIO_MODE_MASK     BIT3
#define REG_PM_GPIO8_GPIO_MODE          0x08
    #define REG_PM_GPIO8_GPIO_MODE_MASK     BIT3
#define REG_PM_GPIO9_GPIO_MODE          0x09
    #define REG_PM_GPIO9_GPIO_MODE_MASK     BIT3
#define REG_PM_GPIO10_GPIO_MODE         0x10
    #define REG_PM_GPIO10_GPIO_MODE_MASK    BIT3
#define REG_PM_SPI_CZ_GPIO_MODE         0x13
    #define REG_PM_SPI_CZ_GPIO_MODE_MASK    BIT3
#define REG_PM_SPI_CK_GPIO_MODE         0x14
    #define REG_PM_SPI_CK_GPIO_MODE_MASK    BIT3
#define REG_PM_SPI_DI_GPIO_MODE         0x15
    #define REG_PM_SPI_DI_GPIO_MODE_MASK    BIT3
#define REG_PM_SPI_DO_GPIO_MODE         0x16
    #define REG_PM_SPI_DO_GPIO_MODE_MASK    BIT3
#define REG_PM_SPI_WPZ_GPIO_MODE        0x17
    #define REG_PM_SPI_WPZ_GPIO_MODE_MASK   BIT3
#define REG_PM_SPI_HLD_GPIO_MODE        0x18
    #define REG_PM_SPI_HLD_GPIO_MODE_MASK   BIT3
#define REG_SAR_GPIO0_GPIO_MODE         0x11
    #define REG_SAR_GPIO0_GPIO_MODE_MASK    BIT0
#define REG_SAR_GPIO1_GPIO_MODE         0x11
    #define REG_SAR_GPIO1_GPIO_MODE_MASK    BIT1
#define REG_SAR_GPIO2_GPIO_MODE         0x11
    #define REG_SAR_GPIO2_GPIO_MODE_MASK    BIT2
#define REG_SAR_GPIO3_GPIO_MODE         0x11
    #define REG_SAR_GPIO3_GPIO_MODE_MASK    BIT3
#define REG_SAR_GPIO4_GPIO_MODE         0x11
    #define REG_SAR_GPIO4_GPIO_MODE_MASK    BIT4
#define REG_SAR_GPIO5_GPIO_MODE         0x11
    #define REG_SAR_GPIO5_GPIO_MODE_MASK    BIT5
    #define REG_SAR_GPIO_MODE_MASK         (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5)
#define REG_SD0_GPIO0_GPIO_MODE         0x10
    #define REG_SD0_GPIO0_GPIO_MODE_MASK    BIT3
#define REG_SD0_CDZ_GPIO_MODE           0x11
    #define REG_SD0_CDZ_GPIO_MODE_MASK      BIT3
#define REG_SD0_D1_GPIO_MODE            0x12
    #define REG_SD0_D1_GPIO_MODE_MASK       BIT3
#define REG_SD0_D0_GPIO_MODE            0x13
    #define REG_SD0_D0_GPIO_MODE_MASK       BIT3
#define REG_SD0_CLK_GPIO_MODE           0x14
    #define REG_SD0_CLK_GPIO_MODE_MASK      BIT3
#define REG_SD0_CMD_GPIO_MODE           0x15
    #define REG_SD0_CMD_GPIO_MODE_MASK      BIT3
#define REG_SD0_D3_GPIO_MODE            0x16
    #define REG_SD0_D3_GPIO_MODE_MASK       BIT3
#define REG_SD0_D2_GPIO_MODE            0x17
    #define REG_SD0_D2_GPIO_MODE_MASK       BIT3
#define REG_I2S0_MCLK_GPIO_MODE         0x18
    #define REG_I2S0_MCLK_GPIO_MODE_MASK    BIT3
#define REG_I2S0_BCK_GPIO_MODE          0x19
    #define REG_I2S0_BCK_GPIO_MODE_MASK     BIT3
#define REG_I2S0_WCK_GPIO_MODE          0x1a
    #define REG_I2S0_WCK_GPIO_MODE_MASK     BIT3
#define REG_I2S0_DI_GPIO_MODE           0x1b
    #define REG_I2S0_DI_GPIO_MODE_MASK      BIT3
#define REG_I2S0_DO_GPIO_MODE           0x1c
    #define REG_I2S0_DO_GPIO_MODE_MASK      BIT3
#define REG_I2C0_SCL_GPIO_MODE          0x1d
    #define REG_I2C0_SCL_GPIO_MODE_MASK     BIT3
#define REG_I2C0_SDA_GPIO_MODE          0x1e
    #define REG_I2C0_SDA_GPIO_MODE_MASK     BIT3
#define REG_ETH_LED0_GPIO_MODE          0x20
    #define REG_ETH_LED0_GPIO_MODE_MASK     BIT3
#define REG_ETH_LED1_GPIO_MODE          0x21
    #define REG_ETH_LED1_GPIO_MODE_MASK     BIT3
#define REG_FUART_RX_GPIO_MODE          0x22
    #define REG_FUART_RX_GPIO_MODE_MASK     BIT3
#define REG_FUART_TX_GPIO_MODE          0x23
    #define REG_FUART_TX_GPIO_MODE_MASK     BIT3
#define REG_FUART_CTS_GPIO_MODE         0x24
    #define REG_FUART_CTS_GPIO_MODE_MASK    BIT3
#define REG_FUART_RTS_GPIO_MODE         0x25
    #define REG_FUART_RTS_GPIO_MODE_MASK    BIT3
#define REG_SD1_CDZ_GPIO_MODE           0x26
    #define REG_SD1_CDZ_GPIO_MODE_MASK      BIT3
#define REG_SD1_D1_GPIO_MODE            0x27
    #define REG_SD1_D1_GPIO_MODE_MASK       BIT3
#define REG_SD1_D0_GPIO_MODE            0x28
    #define REG_SD1_D0_GPIO_MODE_MASK       BIT3
#define REG_SD1_CLK_GPIO_MODE           0x29
    #define REG_SD1_CLK_GPIO_MODE_MASK      BIT3
#define REG_SD1_CMD_GPIO_MODE           0x2a
    #define REG_SD1_CMD_GPIO_MODE_MASK      BIT3
#define REG_SD1_D3_GPIO_MODE            0x2b
    #define REG_SD1_D3_GPIO_MODE_MASK       BIT3
#define REG_SD1_D2_GPIO_MODE            0x2c
    #define REG_SD1_D2_GPIO_MODE_MASK       BIT3
#define REG_SD1_GPIO0_GPIO_MODE         0x2d
    #define REG_SD1_GPIO0_GPIO_MODE_MASK    BIT3
#define REG_SD1_GPIO1_GPIO_MODE         0x2e
    #define REG_SD1_GPIO1_GPIO_MODE_MASK    BIT3
#define REG_GPIO0_GPIO_MODE             0x00
    #define REG_GPIO0_GPIO_MODE_MASK        BIT3
#define REG_GPIO1_GPIO_MODE             0x01
    #define REG_GPIO1_GPIO_MODE_MASK        BIT3
#define REG_GPIO2_GPIO_MODE             0x02
    #define REG_GPIO2_GPIO_MODE_MASK        BIT3
#define REG_GPIO3_GPIO_MODE             0x03
    #define REG_GPIO3_GPIO_MODE_MASK        BIT3
#define REG_GPIO4_GPIO_MODE             0x04
    #define REG_GPIO4_GPIO_MODE_MASK        BIT3
#define REG_GPIO5_GPIO_MODE             0x05
    #define REG_GPIO5_GPIO_MODE_MASK        BIT3
#define REG_GPIO6_GPIO_MODE             0x06
    #define REG_GPIO6_GPIO_MODE_MASK        BIT3
#define REG_GPIO7_GPIO_MODE             0x07
    #define REG_GPIO7_GPIO_MODE_MASK        BIT3
#define REG_SR0_IO00_GPIO_MODE          0x36
    #define REG_SR0_IO00_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO01_GPIO_MODE          0x37
    #define REG_SR0_IO01_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO02_GPIO_MODE          0x38
    #define REG_SR0_IO02_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO03_GPIO_MODE          0x39
    #define REG_SR0_IO03_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO04_GPIO_MODE          0x3a
    #define REG_SR0_IO04_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO05_GPIO_MODE          0x3b
    #define REG_SR0_IO05_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO06_GPIO_MODE          0x3c
    #define REG_SR0_IO06_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO07_GPIO_MODE          0x3d
    #define REG_SR0_IO07_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO08_GPIO_MODE          0x3e
    #define REG_SR0_IO08_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO09_GPIO_MODE          0x3f
    #define REG_SR0_IO09_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO10_GPIO_MODE          0x40
    #define REG_SR0_IO10_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO11_GPIO_MODE          0x41
    #define REG_SR0_IO11_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO12_GPIO_MODE          0x42
    #define REG_SR0_IO12_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO13_GPIO_MODE          0x43
    #define REG_SR0_IO13_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO14_GPIO_MODE          0x44
    #define REG_SR0_IO14_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO15_GPIO_MODE          0x45
    #define REG_SR0_IO15_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO16_GPIO_MODE          0x46
    #define REG_SR0_IO16_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO17_GPIO_MODE          0x47
    #define REG_SR0_IO17_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO18_GPIO_MODE          0x48
    #define REG_SR0_IO18_GPIO_MODE_MASK     BIT3
#define REG_SR0_IO19_GPIO_MODE          0x49
    #define REG_SR0_IO19_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO00_GPIO_MODE          0x4a
    #define REG_SR1_IO00_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO01_GPIO_MODE          0x4b
    #define REG_SR1_IO01_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO02_GPIO_MODE          0x4c
    #define REG_SR1_IO02_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO03_GPIO_MODE          0x4d
    #define REG_SR1_IO03_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO04_GPIO_MODE          0x4e
    #define REG_SR1_IO04_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO05_GPIO_MODE          0x4f
    #define REG_SR1_IO05_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO06_GPIO_MODE          0x50
    #define REG_SR1_IO06_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO07_GPIO_MODE          0x51
    #define REG_SR1_IO07_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO08_GPIO_MODE          0x52
    #define REG_SR1_IO08_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO09_GPIO_MODE          0x53
    #define REG_SR1_IO09_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO10_GPIO_MODE          0x54
    #define REG_SR1_IO10_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO11_GPIO_MODE          0x55
    #define REG_SR1_IO11_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO12_GPIO_MODE          0x56
    #define REG_SR1_IO12_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO13_GPIO_MODE          0x57
    #define REG_SR1_IO13_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO14_GPIO_MODE          0x58
    #define REG_SR1_IO14_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO15_GPIO_MODE          0x59
    #define REG_SR1_IO15_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO16_GPIO_MODE          0x5a
    #define REG_SR1_IO16_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO17_GPIO_MODE          0x5b
    #define REG_SR1_IO17_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO18_GPIO_MODE          0x5c
    #define REG_SR1_IO18_GPIO_MODE_MASK     BIT3
#define REG_SR1_IO19_GPIO_MODE          0x5d
    #define REG_SR1_IO19_GPIO_MODE_MASK     BIT3
#define REG_GPIO8_GPIO_MODE             0x08
    #define REG_GPIO8_GPIO_MODE_MASK        BIT3
#define REG_GPIO9_GPIO_MODE             0x09
    #define REG_GPIO9_GPIO_MODE_MASK        BIT3
#define REG_GPIO10_GPIO_MODE            0x0a
    #define REG_GPIO10_GPIO_MODE_MASK       BIT3
#define REG_GPIO11_GPIO_MODE            0x0b
    #define REG_GPIO11_GPIO_MODE_MASK       BIT3
#define REG_GPIO12_GPIO_MODE            0x0c
    #define REG_GPIO12_GPIO_MODE_MASK       BIT3
#define REG_GPIO13_GPIO_MODE            0x0d
    #define REG_GPIO13_GPIO_MODE_MASK       BIT3
#define REG_GPIO14_GPIO_MODE            0x0e
    #define REG_GPIO14_GPIO_MODE_MASK       BIT3
#define REG_GPIO15_GPIO_MODE            0x0f
    #define REG_GPIO15_GPIO_MODE_MASK       BIT3
#define REG_SPI_CZ_GPIO_MODE            0x30
    #define REG_SPI_CZ_GPIO_MODE_MASK       BIT3
#define REG_SPI_CK_GPIO_MODE            0x31
    #define REG_SPI_CK_GPIO_MODE_MASK       BIT3
#define REG_SPI_DI_GPIO_MODE            0x32
    #define REG_SPI_DI_GPIO_MODE_MASK       BIT3
#define REG_SPI_DO_GPIO_MODE            0x33
    #define REG_SPI_DO_GPIO_MODE_MASK       BIT3
#define REG_SPI_WPZ_GPIO_MODE           0x34
    #define REG_SPI_WPZ_GPIO_MODE_MASK      BIT3
#define REG_SPI_HLD_GPIO_MODE           0x35
    #define REG_SPI_HLD_GPIO_MODE_MASK      BIT3


/* EMAC : ALBANY1_BANK */
#define REG_ATOP_RX_INOFF               0x69
    #define REG_ATOP_RX_INOFF_MASK          BIT14

/* EMAC : ALBANY2_BANK */
#define REG_ETH_GPIO_EN                 0x71
    #define REG_ETH_GPIO_EN_MASK            BIT3|BIT2|BIT1|BIT0


/* UTMI0 : UTMI0_BANK */
#define REG_UTMI0_FL_XVR_PDN            0x0
    #define REG_UTMI0_FL_XVR_PDN_MASK       BIT12
#define REG_UTMI0_REG_PDN               0x0
    #define REG_UTMI0_REG_PDN_MASK          BIT15    // 1: power doen 0: enable
#define REG_UTMI0_CLK_EXTRA0_EN         0x4
    #define REG_UTMI0_CLK_EXTRA0_EN_MASK    BIT7     // 1: power down 0: enable
#define REG_UTMI0_GPIO_EN               0x1f
    #define REG_UTMI0_GPIO_EN_MASK          BIT14


//-------------------- configuration -----------------
#define ENABLE_CHECK_ALL_PAD_CONFLICT       0

//==============================================================================
//
//                              STRUCTURE
//
//==============================================================================

typedef struct stPadMux
{
    U16 padID;
    U32 base;
    U16 offset;
    U16 mask;
    U16 val;
    U16 mode;
} ST_PadMuxInfo;

typedef struct stPadMode
{
    U8  u8PadName[32];
    U32 u32ModeRIU;
    U32 u32ModeMask;
} ST_PadModeInfo;

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

const ST_PadMuxInfo m_stPadMuxTbl[] =
{

    {PAD_PM_UART_RX1, PM_PADTOP_BANK, REG_PM_PM51_UART_MODE,             REG_PM_PM51_UART_MODE_MASK,            BIT0,               PINMUX_FOR_PM_PM51_UART_MODE_1},
    {PAD_PM_UART_RX1, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_0,          REG_PM_PM_PAD_EXT_MODE_0_MASK,         BIT0,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_0},
    {PAD_PM_UART_RX1, PM_PADTOP_BANK, REG_PM_UART_RX1_GPIO_MODE,      REG_PM_UART_RX1_GPIO_MODE_MASK,     BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_UART_RX1, PADTOP_BANK, REG_PWM4_MODE,                  REG_PWM4_MODE_MASK,                 BIT1|BIT0,          PINMUX_FOR_PWM4_MODE_3},
    {PAD_PM_UART_RX1, PM_PADTOP_BANK, REG_PM_UART1_MODE,                 REG_PM_UART1_MODE_MASK,                BIT0,               PINMUX_FOR_PM_UART1_MODE_1},

    {PAD_PM_UART_TX1, PM_PADTOP_BANK, REG_PM_PM51_UART_MODE,             REG_PM_PM51_UART_MODE_MASK,            BIT0,               PINMUX_FOR_PM_PM51_UART_MODE_1},
    {PAD_PM_UART_TX1, PADTOP_BANK, REG_PWM5_MODE,                  REG_PWM5_MODE_MASK,                 BIT5|BIT4,          PINMUX_FOR_PWM5_MODE_3},
    {PAD_PM_UART_TX1, PM_PADTOP_BANK, REG_PM_UART_TX1_GPIO_MODE,      REG_PM_UART_TX1_GPIO_MODE_MASK,     BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_UART_TX1, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_1,          REG_PM_PM_PAD_EXT_MODE_1_MASK,         BIT1,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_1},
    {PAD_PM_UART_TX1, PM_PADTOP_BANK, REG_PM_UART1_MODE,                 REG_PM_UART1_MODE_MASK,                BIT0,               PINMUX_FOR_PM_UART1_MODE_1},

    {PAD_PM_UART_RX, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_PM_UART_RX, PADTOP_BANK, REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT0,               PINMUX_FOR_UART0_MODE_1},
    {PAD_PM_UART_RX, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_2,          REG_PM_PM_PAD_EXT_MODE_2_MASK,         BIT2,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_2},
    {PAD_PM_UART_RX, PM_PADTOP_BANK, REG_PM_UART_RX_GPIO_MODE,       REG_PM_UART_RX_GPIO_MODE_MASK,      BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_UART_RX, PM_PADTOP_BANK, REG_PM_UART_IS_GPIO,            REG_PM_UART_IS_GPIO_MASK,           0,                  PINMUX_FOR_PM_PM_UART_IS_MODE},

    {PAD_PM_UART_TX, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_PM_UART_TX, PADTOP_BANK, REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT0,               PINMUX_FOR_UART0_MODE_1},
    {PAD_PM_UART_TX, PM_PADTOP_BANK, REG_PM_UART_IS_GPIO,            REG_PM_UART_IS_GPIO_MASK,           0,                  PINMUX_FOR_PM_PM_UART_IS_MODE},
    {PAD_PM_UART_TX, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_3,          REG_PM_PM_PAD_EXT_MODE_3_MASK,         BIT3,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_3},
    {PAD_PM_UART_TX, PM_PADTOP_BANK, REG_PM_UART_TX_GPIO_MODE,       REG_PM_UART_TX_GPIO_MODE_MASK,      BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_PM_I2CM_SCL, PM_PADTOP_BANK, REG_PM_I2CM_MODE,                  REG_PM_I2CM_MODE_MASK,                 BIT0,               PINMUX_FOR_PM_I2CM_MODE_1},
    {PAD_PM_I2CM_SCL, PADTOP_BANK, REG_I2C2_MODE,                  REG_I2C2_MODE_MASK,                 BIT10,              PINMUX_FOR_I2C2_MODE_4},
    {PAD_PM_I2CM_SCL, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_21,         REG_PM_PM_PAD_EXT_MODE_21_MASK,        BIT5,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_21},
    {PAD_PM_I2CM_SCL, PADTOP_BANK, REG_PWM6_MODE,                  REG_PWM6_MODE_MASK,                 BIT9|BIT8,          PINMUX_FOR_PWM6_MODE_3},
    {PAD_PM_I2CM_SCL, PM_PADTOP_BANK, REG_PM_I2CM_SCL_GPIO_MODE,      REG_PM_I2CM_SCL_GPIO_MODE_MASK,     BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_PM_I2CM_SDA, PADTOP_BANK, REG_PWM7_MODE,                  REG_PWM7_MODE_MASK,                 BIT13|BIT12,        PINMUX_FOR_PWM7_MODE_3},
    {PAD_PM_I2CM_SDA, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_22,         REG_PM_PM_PAD_EXT_MODE_22_MASK,        BIT6,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_22},
    {PAD_PM_I2CM_SDA, PADTOP_BANK, REG_I2C2_MODE,                  REG_I2C2_MODE_MASK,                 BIT10,              PINMUX_FOR_I2C2_MODE_4},
    {PAD_PM_I2CM_SDA, PM_PADTOP_BANK, REG_PM_I2CM_SDA_GPIO_MODE,      REG_PM_I2CM_SDA_GPIO_MODE_MASK,     BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_I2CM_SDA, PM_PADTOP_BANK, REG_PM_I2CM_MODE,                  REG_PM_I2CM_MODE_MASK,                 BIT0,               PINMUX_FOR_PM_I2CM_MODE_1},

    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_PM_GPIO0_GPIO_MODE,         REG_PM_GPIO0_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_4,          REG_PM_PM_PAD_EXT_MODE_4_MASK,         BIT4,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_4},
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_PM_PM51_UART_MODE,             REG_PM_PM51_UART_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_PM_PM51_UART_MODE_3},
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_PM_UART1_MODE,                 REG_PM_UART1_MODE_MASK,                BIT1,               PINMUX_FOR_PM_UART1_MODE_2},
    {PAD_PM_GPIO0, PADTOP_BANK, REG_LED1_MODE,                  REG_LED1_MODE_MASK,                 BIT5|BIT4,          PINMUX_FOR_LED1_MODE_3},
    {PAD_PM_GPIO0, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_PM_GPIO0, PADTOP_BANK, REG_I2C2_MODE,                  REG_I2C2_MODE_MASK,                 BIT10|BIT8,         PINMUX_FOR_I2C2_MODE_5},
    {PAD_PM_GPIO0, PADTOP_BANK, REG_LED0_MODE,                  REG_LED0_MODE_MASK,                 BIT1|BIT0,          PINMUX_FOR_LED0_MODE_3},
    {PAD_PM_GPIO0, PADTOP_BANK, REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT1|BIT0,          PINMUX_FOR_PWM0_MODE_3},
    {PAD_PM_GPIO0, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT5|BIT4,          PINMUX_FOR_FUART_MODE_3},
    {PAD_PM_GPIO0, PADTOP_BANK, REG_PWM8_MODE,                  REG_PWM8_MODE_MASK,                 BIT3,               PINMUX_FOR_PWM8_MODE_8},
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_PM_VID_MODE,                   REG_PM_VID_MODE_MASK,                  BIT3,               PINMUX_FOR_PM_VID_MODE_2},
    {PAD_PM_GPIO0, PM_PADTOP_BANK, REG_PM_PIR_SERIN_MODE,             REG_PM_PIR_SERIN_MODE_MASK,            BIT2,               PINMUX_FOR_PM_PIR_SERIN_MODE_1},
    {PAD_PM_GPIO0, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13|BIT11,        PINMUX_FOR_I2S_RXTX_MODE_5},

    {PAD_PM_GPIO1, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_5,          REG_PM_PM_PAD_EXT_MODE_5_MASK,         BIT5,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_5},
    {PAD_PM_GPIO1, PM_PADTOP_BANK, REG_PM_PWM0_MODE,                  REG_PM_PWM0_MODE_MASK,                 BIT0,               PINMUX_FOR_PM_PWM0_MODE_1},
    {PAD_PM_GPIO1, PM_PADTOP_BANK, REG_PM_UART1_MODE,                 REG_PM_UART1_MODE_MASK,                BIT1,               PINMUX_FOR_PM_UART1_MODE_2},
    {PAD_PM_GPIO1, PADTOP_BANK, REG_LED1_MODE,                  REG_LED1_MODE_MASK,                 BIT6,               PINMUX_FOR_LED1_MODE_4},
    {PAD_PM_GPIO1, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_PM_GPIO1, PADTOP_BANK, REG_I2C2_MODE,                  REG_I2C2_MODE_MASK,                 BIT10|BIT8,         PINMUX_FOR_I2C2_MODE_5},
    {PAD_PM_GPIO1, PADTOP_BANK, REG_PWM9_MODE,                  REG_PWM9_MODE_MASK,                 BIT7,               PINMUX_FOR_PWM9_MODE_8},
    {PAD_PM_GPIO1, PADTOP_BANK, REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT5|BIT4,          PINMUX_FOR_PWM1_MODE_3},
    {PAD_PM_GPIO1, PADTOP_BANK, REG_LED0_MODE,                  REG_LED0_MODE_MASK,                 BIT2,               PINMUX_FOR_LED0_MODE_4},
    {PAD_PM_GPIO1, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT5|BIT4,          PINMUX_FOR_FUART_MODE_3},
    {PAD_PM_GPIO1, PM_PADTOP_BANK, REG_PM_GPIO1_GPIO_MODE,         REG_PM_GPIO1_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_GPIO1, PM_PADTOP_BANK, REG_PM_PM51_UART_MODE,             REG_PM_PM51_UART_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_PM_PM51_UART_MODE_3},
    {PAD_PM_GPIO1, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13|BIT11,        PINMUX_FOR_I2S_RXTX_MODE_5},

    {PAD_PM_GPIO2, PADTOP_BANK, REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT9|BIT8,          PINMUX_FOR_PWM2_MODE_3},
    {PAD_PM_GPIO2, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_6,          REG_PM_PM_PAD_EXT_MODE_6_MASK,         BIT6,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_6},
    {PAD_PM_GPIO2, PM_PADTOP_BANK, REG_PM_GPIO2_GPIO_MODE,         REG_PM_GPIO2_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_GPIO2, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT5|BIT4,          PINMUX_FOR_FUART_MODE_3},
    {PAD_PM_GPIO2, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT6|BIT5,          PINMUX_FOR_FUART_MODE_6},
    {PAD_PM_GPIO2, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13|BIT11,        PINMUX_FOR_I2S_RXTX_MODE_5},

    {PAD_PM_GPIO3, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13|BIT11,        PINMUX_FOR_I2S_RXTX_MODE_5},
    {PAD_PM_GPIO3, PADTOP_BANK, REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT13|BIT12,        PINMUX_FOR_PWM3_MODE_3},
    {PAD_PM_GPIO3, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT5|BIT4,          PINMUX_FOR_FUART_MODE_3},
    {PAD_PM_GPIO3, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT6|BIT5,          PINMUX_FOR_FUART_MODE_6},
    {PAD_PM_GPIO3, PM_PADTOP_BANK, REG_PM_VID_MODE,                   REG_PM_VID_MODE_MASK,                  BIT3,               PINMUX_FOR_PM_VID_MODE_2},
    {PAD_PM_GPIO3, PM_PADTOP_BANK, REG_PM_GPIO3_GPIO_MODE,         REG_PM_GPIO3_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_GPIO3, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_7,          REG_PM_PM_PAD_EXT_MODE_7_MASK,         BIT7,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_7},

    {PAD_PM_GPIO4, PM_PADTOP_BANK, REG_PM_PWM0_MODE,                  REG_PM_PWM0_MODE_MASK,                 BIT1,               PINMUX_FOR_PM_PWM0_MODE_2},
    {PAD_PM_GPIO4, PADTOP_BANK, REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT8|BIT7,          PINMUX_FOR_EJ_MODE_3},
    {PAD_PM_GPIO4, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_8,          REG_PM_PM_PAD_EXT_MODE_8_MASK,         BIT8,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_8},
    {PAD_PM_GPIO4, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2|BIT1,          PINMUX_FOR_DMIC_MODE_6},
    {PAD_PM_GPIO4, PADTOP_BANK, REG_PWM4_MODE,                  REG_PWM4_MODE_MASK,                 BIT2,               PINMUX_FOR_PWM4_MODE_4},
    {PAD_PM_GPIO4, PM_PADTOP_BANK, REG_PM_I2CM_MODE,                  REG_PM_I2CM_MODE_MASK,                 BIT1,               PINMUX_FOR_PM_I2CM_MODE_2},
    {PAD_PM_GPIO4, PM_PADTOP_BANK, REG_PM_GPIO4_GPIO_MODE,         REG_PM_GPIO4_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_PM_GPIO5, PADTOP_BANK, REG_PWM5_MODE,                  REG_PWM5_MODE_MASK,                 BIT6,               PINMUX_FOR_PWM5_MODE_4},
    {PAD_PM_GPIO5, PADTOP_BANK, REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT8|BIT7,          PINMUX_FOR_EJ_MODE_3},
    {PAD_PM_GPIO5, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2|BIT1,          PINMUX_FOR_DMIC_MODE_6},
    {PAD_PM_GPIO5, PM_PADTOP_BANK, REG_PM_PIR_DIR_LINK_MODE,          REG_PM_PIR_DIR_LINK_MODE_MASK,         BIT0,               PINMUX_FOR_PM_PIR_DIR_LINK_MODE_1},
    {PAD_PM_GPIO5, PM_PADTOP_BANK, REG_PM_I2CM_MODE,                  REG_PM_I2CM_MODE_MASK,                 BIT1,               PINMUX_FOR_PM_I2CM_MODE_2},
    {PAD_PM_GPIO5, PM_PADTOP_BANK, REG_PM_PIR_SERIN_MODE,             REG_PM_PIR_SERIN_MODE_MASK,            BIT3,               PINMUX_FOR_PM_PIR_SERIN_MODE_2},
    {PAD_PM_GPIO5, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_9,          REG_PM_PM_PAD_EXT_MODE_9_MASK,         BIT9,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_9},
    {PAD_PM_GPIO5, PM_PADTOP_BANK, REG_PM_GPIO5_GPIO_MODE,         REG_PM_GPIO5_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_PM_GPIO6, PM_PADTOP_BANK, REG_PM_PIR_DIR_LINK_MODE,          REG_PM_PIR_DIR_LINK_MODE_MASK,         BIT1,               PINMUX_FOR_PM_PIR_DIR_LINK_MODE_2},
    {PAD_PM_GPIO6, PADTOP_BANK, REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT8|BIT7,          PINMUX_FOR_EJ_MODE_3},
    {PAD_PM_GPIO6, PM_PADTOP_BANK, REG_PM_GPIO6_GPIO_MODE,         REG_PM_GPIO6_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_GPIO6, PADTOP_BANK, REG_PWM6_MODE,                  REG_PWM6_MODE_MASK,                 BIT10,              PINMUX_FOR_PWM6_MODE_4},
    {PAD_PM_GPIO6, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_10,         REG_PM_PM_PAD_EXT_MODE_10_MASK,        BIT10,              PINMUX_FOR_PM_PM_PAD_EXT_MODE_10},

    {PAD_PM_GPIO7, PM_PADTOP_BANK, REG_PM_VID_MODE,                   REG_PM_VID_MODE_MASK,                  BIT2,               PINMUX_FOR_PM_VID_MODE_1},
    {PAD_PM_GPIO7, PADTOP_BANK, REG_PWM7_MODE,                  REG_PWM7_MODE_MASK,                 BIT14,              PINMUX_FOR_PWM7_MODE_4},
    {PAD_PM_GPIO7, PM_PADTOP_BANK, REG_PM_GPIO7_GPIO_MODE,         REG_PM_GPIO7_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_GPIO7, PADTOP_BANK, REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT8|BIT7,          PINMUX_FOR_EJ_MODE_3},
    {PAD_PM_GPIO7, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_11,         REG_PM_PM_PAD_EXT_MODE_11_MASK,        BIT11,              PINMUX_FOR_PM_PM_PAD_EXT_MODE_11},

    {PAD_PM_GPIO8, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_PM_GPIO8, PM_PADTOP_BANK, REG_PM_VID_MODE,                   REG_PM_VID_MODE_MASK,                  BIT2,               PINMUX_FOR_PM_VID_MODE_1},
    {PAD_PM_GPIO8, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_12,         REG_PM_PM_PAD_EXT_MODE_12_MASK,        BIT12,              PINMUX_FOR_PM_PM_PAD_EXT_MODE_12},
    {PAD_PM_GPIO8, PM_PADTOP_BANK, REG_PM_GPIO8_GPIO_MODE,         REG_PM_GPIO8_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_PM_GPIO9, PM_PADTOP_BANK, REG_PM_PM51_UART_MODE,             REG_PM_PM51_UART_MODE_MASK,            BIT1,               PINMUX_FOR_PM_PM51_UART_MODE_2},
    {PAD_PM_GPIO9, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_13,         REG_PM_PM_PAD_EXT_MODE_13_MASK,        BIT13,              PINMUX_FOR_PM_PM_PAD_EXT_MODE_13},
    {PAD_PM_GPIO9, PM_PADTOP_BANK, REG_PM_GPIO9_GPIO_MODE,         REG_PM_GPIO9_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_GPIO9, PADTOP_BANK, REG_PWM9_MODE,                  REG_PWM9_MODE_MASK,                 BIT4,               PINMUX_FOR_PWM9_MODE_1},
    {PAD_PM_GPIO9, PMSLEEP_BANK, REG_PM_SPICSZ2_GPIO,               REG_PM_SPICSZ2_GPIO_MASK,              0,                  PINMUX_FOR_PM_SPICSZ2_MODE},

    {PAD_PM_GPIO10, PM_PADTOP_BANK, REG_PM_PM51_UART_MODE,             REG_PM_PM51_UART_MODE_MASK,            BIT1,               PINMUX_FOR_PM_PM51_UART_MODE_2},
    {PAD_PM_GPIO10, PM_PADTOP_BANK, REG_PM_GPIO10_GPIO_MODE,        REG_PM_GPIO10_GPIO_MODE_MASK,       BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_GPIO10, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_14,         REG_PM_PM_PAD_EXT_MODE_14_MASK,        BIT14,              PINMUX_FOR_PM_PM_PAD_EXT_MODE_14},
    {PAD_PM_GPIO10, PADTOP_BANK, REG_PWM8_MODE,                  REG_PWM8_MODE_MASK,                 BIT0,               PINMUX_FOR_PWM8_MODE_1},
    {PAD_PM_GPIO10, PMSLEEP_BANK, REG_PM_SPICSZ2_GPIO,               REG_PM_SPICSZ2_GPIO_MASK,              0,                  PINMUX_FOR_PM_SPICSZ2_MODE},

    {PAD_PM_SPI_CZ, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_PM_SPI_CZ, PMSLEEP_BANK, REG_PM_SPICSZ1_GPIO,               REG_PM_SPICSZ1_GPIO_MASK,              0,                  PINMUX_FOR_PM_SPICSZ1_MODE},
    {PAD_PM_SPI_CZ, PM_PADTOP_BANK, REG_PM_SPI_CZ_GPIO_MODE,        REG_PM_SPI_CZ_GPIO_MODE_MASK,       BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_CZ, PADTOP_BANK, REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT5|BIT4,          PINMUX_FOR_I2S_RX_MODE_3},
    {PAD_PM_SPI_CZ, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT6|BIT4,          PINMUX_FOR_SPI1_MODE_5},
    {PAD_PM_SPI_CZ, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT6|BIT5|BIT4,     PINMUX_FOR_FUART_MODE_7},
    {PAD_PM_SPI_CZ, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_15,         REG_PM_PM_PAD_EXT_MODE_15_MASK,        BIT15,              PINMUX_FOR_PM_PM_PAD_EXT_MODE_15},
    {PAD_PM_SPI_CZ, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT12|BIT11,        PINMUX_FOR_I2S_RXTX_MODE_3},
    {PAD_PM_SPI_CZ, PADTOP_BANK, REG_PWM4_MODE,                  REG_PWM4_MODE_MASK,                 BIT2|BIT1,          PINMUX_FOR_PWM4_MODE_6},

    {PAD_PM_SPI_CK, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_19,         REG_PM_PM_PAD_EXT_MODE_19_MASK,        BIT3,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_19},
    {PAD_PM_SPI_CK, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_PM_SPI_CK, PM_PADTOP_BANK, REG_PM_SPI_CK_GPIO_MODE,        REG_PM_SPI_CK_GPIO_MODE_MASK,       BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_CK, PADTOP_BANK, REG_PWM5_MODE,                  REG_PWM5_MODE_MASK,                 BIT6|BIT5,          PINMUX_FOR_PWM5_MODE_6},
    {PAD_PM_SPI_CK, PMSLEEP_BANK, REG_PM_SPI_GPIO,                   REG_PM_SPI_GPIO_MASK,                  0,                  PINMUX_FOR_PM_SPI_MODE},
    {PAD_PM_SPI_CK, PADTOP_BANK, REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT5|BIT4,          PINMUX_FOR_I2S_RX_MODE_3},
    {PAD_PM_SPI_CK, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT6|BIT4,          PINMUX_FOR_SPI1_MODE_5},
    {PAD_PM_SPI_CK, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT6|BIT5|BIT4,     PINMUX_FOR_FUART_MODE_7},
    {PAD_PM_SPI_CK, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT12|BIT11,        PINMUX_FOR_I2S_RXTX_MODE_3},

    {PAD_PM_SPI_DI, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_16,         REG_PM_PM_PAD_EXT_MODE_16_MASK,        BIT0,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_16},
    {PAD_PM_SPI_DI, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT6|BIT4,          PINMUX_FOR_SPI1_MODE_5},
    {PAD_PM_SPI_DI, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_PM_SPI_DI, PMSLEEP_BANK, REG_PM_SPI_GPIO,                   REG_PM_SPI_GPIO_MASK,                  0,                  PINMUX_FOR_PM_SPI_MODE},
    {PAD_PM_SPI_DI, PADTOP_BANK, REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT5|BIT4,          PINMUX_FOR_I2S_RX_MODE_3},
    {PAD_PM_SPI_DI, PM_PADTOP_BANK, REG_PM_SPI_DI_GPIO_MODE,        REG_PM_SPI_DI_GPIO_MODE_MASK,       BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_DI, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT12|BIT11,        PINMUX_FOR_I2S_RXTX_MODE_3},
    {PAD_PM_SPI_DI, PADTOP_BANK, REG_PWM6_MODE,                  REG_PWM6_MODE_MASK,                 BIT10|BIT9,         PINMUX_FOR_PWM6_MODE_6},
    {PAD_PM_SPI_DI, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT6|BIT5|BIT4,     PINMUX_FOR_FUART_MODE_7},

    {PAD_PM_SPI_DO, PADTOP_BANK, REG_PWM7_MODE,                  REG_PWM7_MODE_MASK,                 BIT14|BIT13,        PINMUX_FOR_PWM7_MODE_6},
    {PAD_PM_SPI_DO, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_18,         REG_PM_PM_PAD_EXT_MODE_18_MASK,        BIT2,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_18},
    {PAD_PM_SPI_DO, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_PM_SPI_DO, PMSLEEP_BANK, REG_PM_SPI_GPIO,                   REG_PM_SPI_GPIO_MASK,                  0,                  PINMUX_FOR_PM_SPI_MODE},
    {PAD_PM_SPI_DO, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT6|BIT4,          PINMUX_FOR_SPI1_MODE_5},
    {PAD_PM_SPI_DO, PM_PADTOP_BANK, REG_PM_SPI_DO_GPIO_MODE,        REG_PM_SPI_DO_GPIO_MODE_MASK,       BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_DO, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT6|BIT5|BIT4,     PINMUX_FOR_FUART_MODE_7},
    {PAD_PM_SPI_DO, PADTOP_BANK, REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10,              PINMUX_FOR_I2S_TX_MODE_4},
    {PAD_PM_SPI_DO, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT12|BIT11,        PINMUX_FOR_I2S_RXTX_MODE_3},

    {PAD_PM_SPI_WPZ, PADTOP_BANK, REG_I2S_MCK_MODE,               REG_I2S_MCK_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_I2S_MCK_MODE_3},
    {PAD_PM_SPI_WPZ, PMSLEEP_BANK, REG_PM_SPIWPN_GPIO,                REG_PM_SPIWPN_GPIO_MASK,               0,                  PINMUX_FOR_PM_SPIWPN_MODE},
    {PAD_PM_SPI_WPZ, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_PM_SPI_WPZ, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT1|BIT0,          PINMUX_FOR_DMIC_MODE_3},
    {PAD_PM_SPI_WPZ, PM_PADTOP_BANK, REG_PM_SPI_WPZ_GPIO_MODE,       REG_PM_SPI_WPZ_GPIO_MODE_MASK,      BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_WPZ, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_17,         REG_PM_PM_PAD_EXT_MODE_17_MASK,        BIT1,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_17},

    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_PM_SPI_HLD_GPIO_MODE,       REG_PM_SPI_HLD_GPIO_MODE_MASK,      BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_PM_SPI_HLD, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_PM_SPI_HLD, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT1|BIT0,          PINMUX_FOR_DMIC_MODE_3},
    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_PM_SPIHOLDN_MODE,              REG_PM_SPIHOLDN_MODE_MASK,             0,                  PINMUX_FOR_PM_SPIHOLDN_MODE},
    {PAD_PM_SPI_HLD, PM_PADTOP_BANK, REG_PM_PM_PAD_EXT_MODE_20,         REG_PM_PM_PAD_EXT_MODE_20_MASK,        BIT4,               PINMUX_FOR_PM_PM_PAD_EXT_MODE_20},

    {PAD_SAR_GPIO0, PM_SAR_BANK, REG_SAR_GPIO0_GPIO_MODE,        REG_SAR_GPIO0_GPIO_MODE_MASK,       BIT0,               PINMUX_FOR_GPIO_MODE},
    {PAD_SAR_GPIO0, PADTOP_BANK, REG_SPIHOLDN_GPIO,              REG_SPIHOLDN_GPIO_MASK,             0,                  PINMUX_FOR_SPIHOLDN_MODE},
    {PAD_SAR_GPIO0, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},

    {PAD_SAR_GPIO1, PADTOP_BANK, REG_SPIHOLDN_GPIO,              REG_SPIHOLDN_GPIO_MASK,             0,                  PINMUX_FOR_SPIHOLDN_MODE},
    {PAD_SAR_GPIO1, PM_SAR_BANK, REG_SAR_GPIO1_GPIO_MODE,        REG_SAR_GPIO1_GPIO_MODE_MASK,       BIT1,               PINMUX_FOR_GPIO_MODE},
    {PAD_SAR_GPIO1, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},

    {PAD_SAR_GPIO2, PADTOP_BANK, REG_SPIHOLDN_GPIO,              REG_SPIHOLDN_GPIO_MASK,             0,                  PINMUX_FOR_SPIHOLDN_MODE},
    {PAD_SAR_GPIO2, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},
    {PAD_SAR_GPIO2, PM_SAR_BANK, REG_SAR_GPIO2_GPIO_MODE,        REG_SAR_GPIO2_GPIO_MODE_MASK,       BIT2,               PINMUX_FOR_GPIO_MODE},

    {PAD_SAR_GPIO3, PM_SAR_BANK, REG_SAR_GPIO3_GPIO_MODE,        REG_SAR_GPIO3_GPIO_MODE_MASK,       BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SAR_GPIO3, PADTOP_BANK, REG_SPIHOLDN_GPIO,              REG_SPIHOLDN_GPIO_MASK,             0,                  PINMUX_FOR_SPIHOLDN_MODE},
    {PAD_SAR_GPIO3, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},

    {PAD_SAR_GPIO4, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},
    {PAD_SAR_GPIO4, PADTOP_BANK, REG_SPIHOLDN_GPIO,              REG_SPIHOLDN_GPIO_MASK,             0,                  PINMUX_FOR_SPIHOLDN_MODE},
    {PAD_SAR_GPIO4, PM_SAR_BANK, REG_SAR_GPIO4_GPIO_MODE,        REG_SAR_GPIO4_GPIO_MODE_MASK,       BIT4,               PINMUX_FOR_GPIO_MODE},

    {PAD_SAR_GPIO5, PADTOP_BANK, REG_SPIHOLDN_GPIO,              REG_SPIHOLDN_GPIO_MASK,             0,                  PINMUX_FOR_SPIHOLDN_MODE},
    {PAD_SAR_GPIO5, PM_SAR_BANK, REG_SAR_GPIO5_GPIO_MODE,        REG_SAR_GPIO5_GPIO_MODE_MASK,       BIT5,               PINMUX_FOR_GPIO_MODE},
    {PAD_SAR_GPIO5, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},

    {PAD_SD0_GPIO0, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SD0_GPIO0, PADTOP_BANK, REG_SPICSZ2_GPIO,               REG_SPICSZ2_GPIO_MASK,              0,                  PINMUX_FOR_SPICSZ2_MODE},
    {PAD_SD0_GPIO0, PADTOP_BANK, REG_SD0_GPIO0_GPIO_MODE,        REG_SD0_GPIO0_GPIO_MODE_MASK,       BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_GPIO0, PADTOP_BANK, REG_PWM8_MODE,                  REG_PWM8_MODE_MASK,                 BIT1,               PINMUX_FOR_PWM8_MODE_2},
    {PAD_SD0_GPIO0, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_SD0_GPIO0, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},

    {PAD_SD0_CDZ, PADTOP_BANK, REG_EMMC0_RST_MODE,             REG_EMMC0_RST_MODE_MASK,            BIT3,               PINMUX_FOR_EMMC0_RST_MODE},
    {PAD_SD0_CDZ, PADTOP_BANK, REG_SD0_CDZ_GPIO_MODE,          REG_SD0_CDZ_GPIO_MODE_MASK,         BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_CDZ, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SD0_CDZ, PADTOP_BANK, REG_SD0_CDZ_MODE,               REG_SD0_CDZ_MODE_MASK,              BIT9,               PINMUX_FOR_SD0_CDZ_MODE},
    {PAD_SD0_CDZ, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},

    {PAD_SD0_D1, PADTOP_BANK, REG_EMMC0_4B_MODE,              REG_EMMC0_4B_MODE_MASK,             BIT0,               PINMUX_FOR_EMMC0_4B_MODE},
    {PAD_SD0_D1, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SD0_D1, PADTOP_BANK, REG_SD0_MODE,                   REG_SD0_MODE_MASK,                  BIT8,               PINMUX_FOR_SD0_MODE},
    {PAD_SD0_D1, PADTOP_BANK, REG_SD0_D1_GPIO_MODE,           REG_SD0_D1_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_D1, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},
    {PAD_SD0_D1, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT2,               PINMUX_FOR_EMMC0_8B_MODE_2},
    {PAD_SD0_D1, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},

    {PAD_SD0_D1, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},

    {PAD_SD0_D0, PADTOP_BANK, REG_EMMC0_4B_MODE,              REG_EMMC0_4B_MODE_MASK,             BIT0,               PINMUX_FOR_EMMC0_4B_MODE},
    {PAD_SD0_D0, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SD0_D0, PADTOP_BANK, REG_SD0_D0_GPIO_MODE,           REG_SD0_D0_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_D0, PADTOP_BANK, REG_SD0_MODE,                   REG_SD0_MODE_MASK,                  BIT8,               PINMUX_FOR_SD0_MODE},
    {PAD_SD0_D0, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},
    {PAD_SD0_D0, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT2,               PINMUX_FOR_EMMC0_8B_MODE_2},
    {PAD_SD0_D0, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},

    {PAD_SD0_D0, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},

    {PAD_SD0_CLK, PADTOP_BANK, REG_EMMC0_4B_MODE,              REG_EMMC0_4B_MODE_MASK,             BIT0,               PINMUX_FOR_EMMC0_4B_MODE},
    {PAD_SD0_CLK, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SD0_CLK, PADTOP_BANK, REG_SD0_CLK_GPIO_MODE,          REG_SD0_CLK_GPIO_MODE_MASK,         BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_CLK, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1|BIT0,          PINMUX_FOR_SPI0_MODE_3},
    {PAD_SD0_CLK, PADTOP_BANK, REG_SD0_MODE,                   REG_SD0_MODE_MASK,                  BIT8,               PINMUX_FOR_SD0_MODE},
    {PAD_SD0_CLK, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},
    {PAD_SD0_CLK, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT2,               PINMUX_FOR_EMMC0_8B_MODE_2},
    {PAD_SD0_CLK, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},

    {PAD_SD0_CLK, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},

    {PAD_SD0_CMD, PADTOP_BANK, REG_EMMC0_4B_MODE,              REG_EMMC0_4B_MODE_MASK,             BIT0,               PINMUX_FOR_EMMC0_4B_MODE},
    {PAD_SD0_CMD, PADTOP_BANK, REG_SD0_CMD_GPIO_MODE,          REG_SD0_CMD_GPIO_MODE_MASK,         BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_CMD, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SD0_CMD, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1|BIT0,          PINMUX_FOR_SPI0_MODE_3},
    {PAD_SD0_CMD, PADTOP_BANK, REG_SD0_MODE,                   REG_SD0_MODE_MASK,                  BIT8,               PINMUX_FOR_SD0_MODE},
    {PAD_SD0_CMD, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},
    {PAD_SD0_CMD, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT2,               PINMUX_FOR_EMMC0_8B_MODE_2},
    {PAD_SD0_CMD, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_SD0_CMD, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},

    {PAD_SD0_D3, PADTOP_BANK, REG_EMMC0_4B_MODE,              REG_EMMC0_4B_MODE_MASK,             BIT0,               PINMUX_FOR_EMMC0_4B_MODE},
    {PAD_SD0_D3, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SD0_D3, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1|BIT0,          PINMUX_FOR_SPI0_MODE_3},
    {PAD_SD0_D3, PADTOP_BANK, REG_SD0_MODE,                   REG_SD0_MODE_MASK,                  BIT8,               PINMUX_FOR_SD0_MODE},
    {PAD_SD0_D3, PADTOP_BANK, REG_SD0_D3_GPIO_MODE,           REG_SD0_D3_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_D3, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},
    {PAD_SD0_D3, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT2,               PINMUX_FOR_EMMC0_8B_MODE_2},
    {PAD_SD0_D3, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_SD0_D3, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},

    {PAD_SD0_D2, PADTOP_BANK, REG_EMMC0_4B_MODE,              REG_EMMC0_4B_MODE_MASK,             BIT0,               PINMUX_FOR_EMMC0_4B_MODE},
    {PAD_SD0_D2, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SD0_D2, PADTOP_BANK, REG_SD0_D2_GPIO_MODE,           REG_SD0_D2_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SD0_D2, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1|BIT0,          PINMUX_FOR_SPI0_MODE_3},
    {PAD_SD0_D2, PADTOP_BANK, REG_SD0_MODE,                   REG_SD0_MODE_MASK,                  BIT8,               PINMUX_FOR_SD0_MODE},
    {PAD_SD0_D2, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},
    {PAD_SD0_D2, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT2,               PINMUX_FOR_EMMC0_8B_MODE_2},
    {PAD_SD0_D2, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_SD0_D2, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},

    {PAD_I2S0_MCLK, PADTOP_BANK, REG_I2S_MCK_MODE,               REG_I2S_MCK_MODE_MASK,              BIT0,               PINMUX_FOR_I2S_MCK_MODE_1},
    {PAD_I2S0_MCLK, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_I2S0_MCLK, PADTOP_BANK, REG_I2S0_MCLK_GPIO_MODE,        REG_I2S0_MCLK_GPIO_MODE_MASK,       BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_I2S0_MCLK, PADTOP_BANK, REG_SD1_MODE,                   REG_SD1_MODE_MASK,                  BIT13,              PINMUX_FOR_SD1_MODE_2},
    {PAD_I2S0_MCLK, PADTOP_BANK, REG_PWM9_MODE,                  REG_PWM9_MODE_MASK,                 BIT6|BIT5|BIT4,     PINMUX_FOR_PWM9_MODE_7},
    {PAD_I2S0_MCLK, PADTOP_BANK, REG_EMMC1_4B_MODE,              REG_EMMC1_4B_MODE_MASK,             BIT4,               PINMUX_FOR_EMMC1_4B_MODE_1},
    {PAD_I2S0_MCLK, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,               PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_I2S0_MCLK, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},

    {PAD_I2S0_BCK, PADTOP_BANK, REG_EMMC1_4B_MODE,              REG_EMMC1_4B_MODE_MASK,             BIT4,               PINMUX_FOR_EMMC1_4B_MODE_1},
    {PAD_I2S0_BCK, PADTOP_BANK, REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_UART0_MODE_3},
    {PAD_I2S0_BCK, PADTOP_BANK, REG_SD1_MODE,                   REG_SD1_MODE_MASK,                  BIT13,              PINMUX_FOR_SD1_MODE_2},
    {PAD_I2S0_BCK, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT0,               PINMUX_FOR_DMIC_MODE_1},
    {PAD_I2S0_BCK, PADTOP_BANK, REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT8,               PINMUX_FOR_EJ_MODE_2},
    {PAD_I2S0_BCK, PADTOP_BANK, REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT4,               PINMUX_FOR_I2S_RX_MODE_1},
    {PAD_I2S0_BCK, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT0,               PINMUX_FOR_SPI0_MODE_1},
    {PAD_I2S0_BCK, PADTOP_BANK, REG_I2S0_BCK_GPIO_MODE,         REG_I2S0_BCK_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_I2S0_BCK, PADTOP_BANK, REG_PWM4_MODE,                  REG_PWM4_MODE_MASK,                 BIT2|BIT0,          PINMUX_FOR_PWM4_MODE_5},
    {PAD_I2S0_BCK, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_I2S0_BCK, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,               PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_I2S0_BCK, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_I2S0_BCK, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT11,              PINMUX_FOR_I2S_RXTX_MODE_1},

    {PAD_I2S0_WCK, PADTOP_BANK, REG_EMMC1_4B_MODE,              REG_EMMC1_4B_MODE_MASK,             BIT4,               PINMUX_FOR_EMMC1_4B_MODE_1},
    {PAD_I2S0_WCK, PADTOP_BANK, REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_UART0_MODE_3},
    {PAD_I2S0_WCK, PADTOP_BANK, REG_SD1_MODE,                   REG_SD1_MODE_MASK,                  BIT13,              PINMUX_FOR_SD1_MODE_2},
    {PAD_I2S0_WCK, PADTOP_BANK, REG_PWM5_MODE,                  REG_PWM5_MODE_MASK,                 BIT6|BIT4,          PINMUX_FOR_PWM5_MODE_5},
    {PAD_I2S0_WCK, PADTOP_BANK, REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT8,               PINMUX_FOR_EJ_MODE_2},
    {PAD_I2S0_WCK, PADTOP_BANK, REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT4,               PINMUX_FOR_I2S_RX_MODE_1},
    {PAD_I2S0_WCK, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT0,               PINMUX_FOR_SPI0_MODE_1},
    {PAD_I2S0_WCK, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT0,               PINMUX_FOR_DMIC_MODE_1},
    {PAD_I2S0_WCK, PADTOP_BANK, REG_I2S0_WCK_GPIO_MODE,         REG_I2S0_WCK_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_I2S0_WCK, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_I2S0_WCK, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,               PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_I2S0_WCK, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_I2S0_WCK, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT11,              PINMUX_FOR_I2S_RXTX_MODE_1},

    {PAD_I2S0_DI, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT11,              PINMUX_FOR_I2S_RXTX_MODE_1},
    {PAD_I2S0_DI, PADTOP_BANK, REG_EMMC1_4B_MODE,              REG_EMMC1_4B_MODE_MASK,             BIT4,               PINMUX_FOR_EMMC1_4B_MODE_1},
    {PAD_I2S0_DI, PADTOP_BANK, REG_SD1_MODE,                   REG_SD1_MODE_MASK,                  BIT13,              PINMUX_FOR_SD1_MODE_2},
    {PAD_I2S0_DI, PADTOP_BANK, REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT8,               PINMUX_FOR_EJ_MODE_2},
    {PAD_I2S0_DI, PADTOP_BANK, REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT4,               PINMUX_FOR_I2S_RX_MODE_1},
    {PAD_I2S0_DI, PADTOP_BANK, REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT5,               PINMUX_FOR_UART1_MODE_2},
    {PAD_I2S0_DI, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT0,               PINMUX_FOR_SPI0_MODE_1},
    {PAD_I2S0_DI, PADTOP_BANK, REG_PWM6_MODE,                  REG_PWM6_MODE_MASK,                 BIT10|BIT8,         PINMUX_FOR_PWM6_MODE_5},
    {PAD_I2S0_DI, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_I2S0_DI, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,               PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_I2S0_DI, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_I2S0_DI, PADTOP_BANK, REG_I2S0_DI_GPIO_MODE,          REG_I2S0_DI_GPIO_MODE_MASK,         BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_I2S0_DO, PADTOP_BANK, REG_I2S0_DO_GPIO_MODE,          REG_I2S0_DO_GPIO_MODE_MASK,         BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_I2S0_DO, PADTOP_BANK, REG_PWM7_MODE,                  REG_PWM7_MODE_MASK,                 BIT14|BIT12,        PINMUX_FOR_PWM7_MODE_5},
    {PAD_I2S0_DO, PADTOP_BANK, REG_EMMC1_4B_MODE,              REG_EMMC1_4B_MODE_MASK,             BIT4,               PINMUX_FOR_EMMC1_4B_MODE_1},
    {PAD_I2S0_DO, PADTOP_BANK, REG_SD1_MODE,                   REG_SD1_MODE_MASK,                  BIT13,              PINMUX_FOR_SD1_MODE_2},
    {PAD_I2S0_DO, PADTOP_BANK, REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT8,               PINMUX_FOR_EJ_MODE_2},
    {PAD_I2S0_DO, PADTOP_BANK, REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT5,               PINMUX_FOR_UART1_MODE_2},
    {PAD_I2S0_DO, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT0,               PINMUX_FOR_SPI0_MODE_1},
    {PAD_I2S0_DO, PADTOP_BANK, REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT8,               PINMUX_FOR_I2S_TX_MODE_1},
    {PAD_I2S0_DO, PADTOP_BANK, REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT9|BIT8,          PINMUX_FOR_I2S_TX_MODE_3},
    {PAD_I2S0_DO, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_I2S0_DO, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,               PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_I2S0_DO, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_I2S0_DO, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT11,              PINMUX_FOR_I2S_RXTX_MODE_1},

    {PAD_I2C0_SCL, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_I2C0_SCL, PADTOP_BANK, REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT0,               PINMUX_FOR_I2C0_MODE_1},
    {PAD_I2C0_SCL, PADTOP_BANK, REG_SD1_MODE,                   REG_SD1_MODE_MASK,                  BIT13,              PINMUX_FOR_SD1_MODE_2},
    {PAD_I2C0_SCL, PADTOP_BANK, REG_EMMC1_4B_MODE,              REG_EMMC1_4B_MODE_MASK,             BIT4,               PINMUX_FOR_EMMC1_4B_MODE_1},
    {PAD_I2C0_SCL, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2,               PINMUX_FOR_DMIC_MODE_4},
    {PAD_I2C0_SCL, PADTOP_BANK, REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT2|BIT0,          PINMUX_FOR_PWM0_MODE_5},
    {PAD_I2C0_SCL, PADTOP_BANK, REG_PWM8_MODE,                  REG_PWM8_MODE_MASK,                 BIT2|BIT1,          PINMUX_FOR_PWM8_MODE_6},
    {PAD_I2C0_SCL, PADTOP_BANK, REG_I2C0_SCL_GPIO_MODE,         REG_I2C0_SCL_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_I2C0_SCL, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,               PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_I2C0_SCL, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},

    {PAD_I2C0_SDA, PADTOP_BANK, REG_I2C0_SDA_GPIO_MODE,         REG_I2C0_SDA_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_I2C0_SDA, PADTOP_BANK, REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT0,               PINMUX_FOR_I2C0_MODE_1},
    {PAD_I2C0_SDA, PADTOP_BANK, REG_PWM9_MODE,                  REG_PWM9_MODE_MASK,                 BIT6|BIT5,          PINMUX_FOR_PWM9_MODE_6},
    {PAD_I2C0_SDA, PADTOP_BANK, REG_EMMC1_RST_MODE,             REG_EMMC1_RST_MODE_MASK,            BIT6,               PINMUX_FOR_EMMC1_RST_MODE_1},
    {PAD_I2C0_SDA, PADTOP_BANK, REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT6|BIT4,          PINMUX_FOR_PWM1_MODE_5},
    {PAD_I2C0_SDA, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_I2C0_SDA, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2,               PINMUX_FOR_DMIC_MODE_4},
    {PAD_I2C0_SDA, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,               PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_I2C0_SDA, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},
    {PAD_I2C0_SDA, PADTOP_BANK, REG_SD1_CDZ_MODE,               REG_SD1_CDZ_MODE_MASK,              BIT9,               PINMUX_FOR_SD1_CDZ_MODE_2},

    {PAD_ETH_LED0, PADTOP_BANK, REG_LED1_MODE,                  REG_LED1_MODE_MASK,                 BIT4,               PINMUX_FOR_LED1_MODE_1},
    {PAD_ETH_LED0, PADTOP_BANK, REG_LED0_MODE,                  REG_LED0_MODE_MASK,                 BIT0,               PINMUX_FOR_LED0_MODE_1},
    {PAD_ETH_LED0, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_ETH_LED0, PADTOP_BANK, REG_I2C2_MODE,                  REG_I2C2_MODE_MASK,                 BIT9|BIT8,          PINMUX_FOR_I2C2_MODE_3},
    {PAD_ETH_LED0, PADTOP_BANK, REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT1,               PINMUX_FOR_I2C0_MODE_2},
    {PAD_ETH_LED0, PADTOP_BANK, REG_PWM9_MODE,                  REG_PWM9_MODE_MASK,                 BIT5|BIT4,          PINMUX_FOR_PWM9_MODE_3},
    {PAD_ETH_LED0, PADTOP_BANK, REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT5|BIT4,          PINMUX_FOR_UART1_MODE_3},
    {PAD_ETH_LED0, PADTOP_BANK, REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT10|BIT8,         PINMUX_FOR_PWM2_MODE_5},
    {PAD_ETH_LED0, PADTOP_BANK, REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT1,               PINMUX_FOR_UART0_MODE_2},
    {PAD_ETH_LED0, PADTOP_BANK, REG_ETH_LED0_GPIO_MODE,         REG_ETH_LED0_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_ETH_LED0, PADTOP_BANK, REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT8,               PINMUX_FOR_I2S_TX_MODE_1},
    {PAD_ETH_LED0, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_ETH_LED0, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},


    {PAD_ETH_LED1, PADTOP_BANK, REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT14|BIT12,        PINMUX_FOR_PWM3_MODE_5},
    {PAD_ETH_LED1, PADTOP_BANK, REG_LED1_MODE,                  REG_LED1_MODE_MASK,                 BIT5,               PINMUX_FOR_LED1_MODE_2},
    {PAD_ETH_LED1, PADTOP_BANK, REG_LED0_MODE,                  REG_LED0_MODE_MASK,                 BIT1,               PINMUX_FOR_LED0_MODE_2},
    {PAD_ETH_LED1, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_ETH_LED1, PADTOP_BANK, REG_I2C2_MODE,                  REG_I2C2_MODE_MASK,                 BIT9|BIT8,          PINMUX_FOR_I2C2_MODE_3},
    {PAD_ETH_LED1, PADTOP_BANK, REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT1,               PINMUX_FOR_I2C0_MODE_2},
    {PAD_ETH_LED1, PADTOP_BANK, REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT5|BIT4,          PINMUX_FOR_UART1_MODE_3},
    {PAD_ETH_LED1, PADTOP_BANK, REG_ETH_LED1_GPIO_MODE,         REG_ETH_LED1_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_ETH_LED1, PADTOP_BANK, REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT1,               PINMUX_FOR_UART0_MODE_2},
    {PAD_ETH_LED1, PADTOP_BANK, REG_PWM8_MODE,                  REG_PWM8_MODE_MASK,                 BIT1|BIT0,          PINMUX_FOR_PWM8_MODE_3},
    {PAD_ETH_LED1, PADTOP_BANK, REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT8,               PINMUX_FOR_I2S_TX_MODE_1},
    {PAD_ETH_LED1, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_ETH_LED1, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},


    {PAD_FUART_RX, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5|BIT4,          PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_FUART_RX, PADTOP_BANK, REG_DLA_EJ_MODE,                REG_DLA_EJ_MODE_MASK,               BIT0,               PINMUX_FOR_DLA_EJ_MODE_1},
    {PAD_FUART_RX, PADTOP_BANK, REG_FUART_RX_GPIO_MODE,         REG_FUART_RX_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_FUART_RX, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_FUART_RX, PADTOP_BANK, REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT7,               PINMUX_FOR_EJ_MODE_1},
    {PAD_FUART_RX, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT4,               PINMUX_FOR_SPI1_MODE_1},
    {PAD_FUART_RX, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1,               PINMUX_FOR_SPI0_MODE_2},
    {PAD_FUART_RX, PADTOP_BANK, REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT2|BIT1,          PINMUX_FOR_PWM0_MODE_6},
    {PAD_FUART_RX, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT4,               PINMUX_FOR_FUART_MODE_1},
    {PAD_FUART_RX, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT6|BIT4,          PINMUX_FOR_FUART_MODE_5},
    {PAD_FUART_RX, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_FUART_RX, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_FUART_RX, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},


    {PAD_FUART_TX, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5|BIT4,          PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_FUART_TX, PADTOP_BANK, REG_DLA_EJ_MODE,                REG_DLA_EJ_MODE_MASK,               BIT0,               PINMUX_FOR_DLA_EJ_MODE_1},
    {PAD_FUART_TX, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1,               PINMUX_FOR_SPI0_MODE_2},
    {PAD_FUART_TX, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_FUART_TX, PADTOP_BANK, REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT7,               PINMUX_FOR_EJ_MODE_1},
    {PAD_FUART_TX, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT4,               PINMUX_FOR_SPI1_MODE_1},
    {PAD_FUART_TX, PADTOP_BANK, REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT6|BIT5,          PINMUX_FOR_PWM1_MODE_6},
    {PAD_FUART_TX, PADTOP_BANK, REG_FUART_TX_GPIO_MODE,         REG_FUART_TX_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_FUART_TX, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT4,               PINMUX_FOR_FUART_MODE_1},
    {PAD_FUART_TX, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT6|BIT4,          PINMUX_FOR_FUART_MODE_5},
    {PAD_FUART_TX, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_FUART_TX, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_FUART_TX, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},


    {PAD_FUART_CTS, PADTOP_BANK, REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT6|BIT4,          PINMUX_FOR_UART1_MODE_5},
    {PAD_FUART_CTS, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5|BIT4,          PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_FUART_CTS, PADTOP_BANK, REG_DLA_EJ_MODE,                REG_DLA_EJ_MODE_MASK,               BIT0,               PINMUX_FOR_DLA_EJ_MODE_1},
    {PAD_FUART_CTS, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_FUART_CTS, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_FUART_CTS, PADTOP_BANK, REG_FUART_CTS_GPIO_MODE,        REG_FUART_CTS_GPIO_MODE_MASK,       BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_FUART_CTS, PADTOP_BANK, REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT7,               PINMUX_FOR_EJ_MODE_1},
    {PAD_FUART_CTS, PADTOP_BANK, REG_PWM9_MODE,                  REG_PWM9_MODE_MASK,                 BIT5,               PINMUX_FOR_PWM9_MODE_2},
    {PAD_FUART_CTS, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT4,               PINMUX_FOR_SPI1_MODE_1},
    {PAD_FUART_CTS, PADTOP_BANK, REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT10|BIT9,         PINMUX_FOR_PWM2_MODE_6},
    {PAD_FUART_CTS, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1,               PINMUX_FOR_SPI0_MODE_2},
    {PAD_FUART_CTS, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT4,               PINMUX_FOR_FUART_MODE_1},
    {PAD_FUART_CTS, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_FUART_CTS, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},

    {PAD_FUART_CTS, PADTOP_BANK, REG_ETH_MODE,                   REG_ETH_MODE_MASK,                  BIT0,               PINMUX_FOR_ETH_MODE},

    {PAD_FUART_RTS, PADTOP_BANK, REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT6|BIT4,          PINMUX_FOR_UART1_MODE_5},
    {PAD_FUART_RTS, PADTOP_BANK, REG_FUART_RTS_GPIO_MODE,        REG_FUART_RTS_GPIO_MODE_MASK,       BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_FUART_RTS, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5|BIT4,          PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_FUART_RTS, PADTOP_BANK, REG_DLA_EJ_MODE,                REG_DLA_EJ_MODE_MASK,               BIT0,               PINMUX_FOR_DLA_EJ_MODE_1},
    {PAD_FUART_RTS, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_FUART_RTS, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_FUART_RTS, PADTOP_BANK, REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT14|BIT13,        PINMUX_FOR_PWM3_MODE_6},
    {PAD_FUART_RTS, PADTOP_BANK, REG_EJ_MODE,                    REG_EJ_MODE_MASK,                   BIT7,               PINMUX_FOR_EJ_MODE_1},
    {PAD_FUART_RTS, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT4,               PINMUX_FOR_SPI1_MODE_1},
    {PAD_FUART_RTS, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT1,               PINMUX_FOR_SPI0_MODE_2},
    {PAD_FUART_RTS, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT4,               PINMUX_FOR_FUART_MODE_1},
    {PAD_FUART_RTS, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},
    {PAD_FUART_RTS, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},

    {PAD_FUART_RTS, PADTOP_BANK, REG_ETH_MODE,                   REG_ETH_MODE_MASK,                  BIT0,               PINMUX_FOR_ETH_MODE},

    {PAD_SD1_CDZ, PADTOP_BANK, REG_EMMC1_RST_MODE,             REG_EMMC1_RST_MODE_MASK,            BIT7,               PINMUX_FOR_EMMC1_RST_MODE_2},
    {PAD_SD1_CDZ, PADTOP_BANK, REG_SD1_CDZ_MODE,               REG_SD1_CDZ_MODE_MASK,              BIT8,               PINMUX_FOR_SD1_CDZ_MODE_1},
    {PAD_SD1_CDZ, PADTOP_BANK, REG_SD1_CDZ_GPIO_MODE,          REG_SD1_CDZ_GPIO_MODE_MASK,         BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_SD1_D1, PADTOP_BANK, REG_SD1_D1_GPIO_MODE,           REG_SD1_D1_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SD1_D1, PADTOP_BANK, REG_EMMC1_4B_MODE,              REG_EMMC1_4B_MODE_MASK,             BIT5,               PINMUX_FOR_EMMC1_4B_MODE_2},
    {PAD_SD1_D1, PADTOP_BANK, REG_SD1_MODE,                   REG_SD1_MODE_MASK,                  BIT12,              PINMUX_FOR_SD1_MODE_1},
    {PAD_SD1_D1, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT6,               PINMUX_FOR_FUART_MODE_4},

    {PAD_SD1_D0, PADTOP_BANK, REG_EMMC1_4B_MODE,              REG_EMMC1_4B_MODE_MASK,             BIT5,               PINMUX_FOR_EMMC1_4B_MODE_2},
    {PAD_SD1_D0, PADTOP_BANK, REG_SD1_D0_GPIO_MODE,           REG_SD1_D0_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SD1_D0, PADTOP_BANK, REG_SD1_MODE,                   REG_SD1_MODE_MASK,                  BIT12,              PINMUX_FOR_SD1_MODE_1},
    {PAD_SD1_D0, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT6,               PINMUX_FOR_FUART_MODE_4},

    {PAD_SD1_CLK, PADTOP_BANK, REG_SD1_CLK_GPIO_MODE,          REG_SD1_CLK_GPIO_MODE_MASK,         BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SD1_CLK, PADTOP_BANK, REG_EMMC1_4B_MODE,              REG_EMMC1_4B_MODE_MASK,             BIT5,               PINMUX_FOR_EMMC1_4B_MODE_2},
    {PAD_SD1_CLK, PADTOP_BANK, REG_SD1_MODE,                   REG_SD1_MODE_MASK,                  BIT12,              PINMUX_FOR_SD1_MODE_1},
    {PAD_SD1_CLK, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT6,               PINMUX_FOR_FUART_MODE_4},

    {PAD_SD1_CMD, PADTOP_BANK, REG_EMMC1_4B_MODE,              REG_EMMC1_4B_MODE_MASK,             BIT5,               PINMUX_FOR_EMMC1_4B_MODE_2},
    {PAD_SD1_CMD, PADTOP_BANK, REG_SD1_MODE,                   REG_SD1_MODE_MASK,                  BIT12,              PINMUX_FOR_SD1_MODE_1},
    {PAD_SD1_CMD, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT6,               PINMUX_FOR_FUART_MODE_4},
    {PAD_SD1_CMD, PADTOP_BANK, REG_SD1_CMD_GPIO_MODE,          REG_SD1_CMD_GPIO_MODE_MASK,         BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_SD1_D3, PADTOP_BANK, REG_SD1_MODE,                   REG_SD1_MODE_MASK,                  BIT12,              PINMUX_FOR_SD1_MODE_1},
    {PAD_SD1_D3, PADTOP_BANK, REG_EMMC1_4B_MODE,              REG_EMMC1_4B_MODE_MASK,             BIT5,               PINMUX_FOR_EMMC1_4B_MODE_2},
    {PAD_SD1_D3, PADTOP_BANK, REG_SD1_D3_GPIO_MODE,           REG_SD1_D3_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_SD1_D2, PADTOP_BANK, REG_EMMC1_4B_MODE,              REG_EMMC1_4B_MODE_MASK,             BIT5,               PINMUX_FOR_EMMC1_4B_MODE_2},
    {PAD_SD1_D2, PADTOP_BANK, REG_SD1_D2_GPIO_MODE,           REG_SD1_D2_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SD1_D2, PADTOP_BANK, REG_SD1_MODE,                   REG_SD1_MODE_MASK,                  BIT12,              PINMUX_FOR_SD1_MODE_1},

    {PAD_SD1_GPIO0, PADTOP_BANK, REG_SD1_GPIO0_GPIO_MODE,        REG_SD1_GPIO0_GPIO_MODE_MASK,       BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_SD1_GPIO1, PADTOP_BANK, REG_SD1_GPIO1_GPIO_MODE,        REG_SD1_GPIO1_GPIO_MODE_MASK,       BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_GPIO0, PADTOP_BANK, REG_I2S_MCK_MODE,               REG_I2S_MCK_MODE_MASK,              BIT1,               PINMUX_FOR_I2S_MCK_MODE_2},
    {PAD_GPIO0, PADTOP_BANK, REG_DLA_EJ_MODE,                REG_DLA_EJ_MODE_MASK,               BIT1,               PINMUX_FOR_DLA_EJ_MODE_2},
    {PAD_GPIO0, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_GPIO0, PADTOP_BANK, REG_I2C2_MODE,                  REG_I2C2_MODE_MASK,                 BIT9,               PINMUX_FOR_I2C2_MODE_2},
    {PAD_GPIO0, PADTOP_BANK, REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT4,               PINMUX_FOR_UART1_MODE_1},
    {PAD_GPIO0, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1,               PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_GPIO0, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT2,               PINMUX_FOR_SPI0_MODE_4},
    {PAD_GPIO0, PADTOP_BANK, REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT1,               PINMUX_FOR_PWM0_MODE_2},
    {PAD_GPIO0, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT5,               PINMUX_FOR_FUART_MODE_2},
    {PAD_GPIO0, PADTOP_BANK, REG_GPIO0_GPIO_MODE,            REG_GPIO0_GPIO_MODE_MASK,           BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO0, PADTOP_BANK, REG_ETH_MODE,                   REG_ETH_MODE_MASK,                  BIT0,               PINMUX_FOR_ETH_MODE},

    {PAD_GPIO1, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT12,              PINMUX_FOR_I2S_RXTX_MODE_2},
    {PAD_GPIO1, PADTOP_BANK, REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT4,               PINMUX_FOR_UART1_MODE_1},
    {PAD_GPIO1, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1,               PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_GPIO1, PADTOP_BANK, REG_DLA_EJ_MODE,                REG_DLA_EJ_MODE_MASK,               BIT1,               PINMUX_FOR_DLA_EJ_MODE_2},
    {PAD_GPIO1, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_GPIO1, PADTOP_BANK, REG_I2C2_MODE,                  REG_I2C2_MODE_MASK,                 BIT9,               PINMUX_FOR_I2C2_MODE_2},
    {PAD_GPIO1, PADTOP_BANK, REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT5,               PINMUX_FOR_I2S_RX_MODE_2},
    {PAD_GPIO1, PADTOP_BANK, REG_GPIO1_GPIO_MODE,            REG_GPIO1_GPIO_MODE_MASK,           BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO1, PADTOP_BANK, REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT5,               PINMUX_FOR_PWM1_MODE_2},
    {PAD_GPIO1, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT2,               PINMUX_FOR_SPI0_MODE_4},
    {PAD_GPIO1, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT3,               PINMUX_FOR_DMIC_MODE_8},
    {PAD_GPIO1, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT5,               PINMUX_FOR_FUART_MODE_2},
    {PAD_GPIO1, PADTOP_BANK, REG_ETH_MODE,                   REG_ETH_MODE_MASK,                  BIT0,               PINMUX_FOR_ETH_MODE},

    {PAD_GPIO2, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT12,              PINMUX_FOR_I2S_RXTX_MODE_2},
    {PAD_GPIO2, PADTOP_BANK, REG_GPIO2_GPIO_MODE,            REG_GPIO2_GPIO_MODE_MASK,           BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO2, PADTOP_BANK, REG_DLA_EJ_MODE,                REG_DLA_EJ_MODE_MASK,               BIT1,               PINMUX_FOR_DLA_EJ_MODE_2},
    {PAD_GPIO2, PADTOP_BANK, REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT5,               PINMUX_FOR_I2S_RX_MODE_2},
    {PAD_GPIO2, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1,               PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_GPIO2, PADTOP_BANK, REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT9,               PINMUX_FOR_PWM2_MODE_2},
    {PAD_GPIO2, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT2,               PINMUX_FOR_SPI0_MODE_4},
    {PAD_GPIO2, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT3,               PINMUX_FOR_DMIC_MODE_8},
    {PAD_GPIO2, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT5,               PINMUX_FOR_FUART_MODE_2},
    {PAD_GPIO2, PADTOP_BANK, REG_ETH_MODE,                   REG_ETH_MODE_MASK,                  BIT0,               PINMUX_FOR_ETH_MODE},

    {PAD_GPIO3, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT12,              PINMUX_FOR_I2S_RXTX_MODE_2},
    {PAD_GPIO3, PADTOP_BANK, REG_DLA_EJ_MODE,                REG_DLA_EJ_MODE_MASK,               BIT1,               PINMUX_FOR_DLA_EJ_MODE_2},
    {PAD_GPIO3, PADTOP_BANK, REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT13,              PINMUX_FOR_PWM3_MODE_2},
    {PAD_GPIO3, PADTOP_BANK, REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT5,               PINMUX_FOR_I2S_RX_MODE_2},
    {PAD_GPIO3, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1,               PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_GPIO3, PADTOP_BANK, REG_SPI0_MODE,                  REG_SPI0_MODE_MASK,                 BIT2,               PINMUX_FOR_SPI0_MODE_4},
    {PAD_GPIO3, PADTOP_BANK, REG_FUART_MODE,                 REG_FUART_MODE_MASK,                BIT5,               PINMUX_FOR_FUART_MODE_2},
    {PAD_GPIO3, PADTOP_BANK, REG_GPIO3_GPIO_MODE,            REG_GPIO3_GPIO_MODE_MASK,           BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO3, PADTOP_BANK, REG_ETH_MODE,                   REG_ETH_MODE_MASK,                  BIT0,               PINMUX_FOR_ETH_MODE},

    {PAD_GPIO4, PADTOP_BANK, REG_GPIO4_GPIO_MODE,            REG_GPIO4_GPIO_MODE_MASK,           BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO4, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT12,              PINMUX_FOR_I2S_RXTX_MODE_2},
    {PAD_GPIO4, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1,               PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_GPIO4, PADTOP_BANK, REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT6,               PINMUX_FOR_UART1_MODE_4},
    {PAD_GPIO4, PADTOP_BANK, REG_PWM4_MODE,                  REG_PWM4_MODE_MASK,                 BIT1,               PINMUX_FOR_PWM4_MODE_2},
    {PAD_GPIO4, PADTOP_BANK, REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT9,               PINMUX_FOR_I2S_TX_MODE_2},
    {PAD_GPIO4, PADTOP_BANK, REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10|BIT8,         PINMUX_FOR_I2S_TX_MODE_5},
    {PAD_GPIO4, PADTOP_BANK, REG_ETH_MODE,                   REG_ETH_MODE_MASK,                  BIT0,               PINMUX_FOR_ETH_MODE},

    {PAD_GPIO5, PADTOP_BANK, REG_PWM5_MODE,                  REG_PWM5_MODE_MASK,                 BIT5,               PINMUX_FOR_PWM5_MODE_2},
    {PAD_GPIO5, PADTOP_BANK, REG_GPIO5_GPIO_MODE,            REG_GPIO5_GPIO_MODE_MASK,           BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO5, PADTOP_BANK, REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT6,               PINMUX_FOR_UART1_MODE_4},
    {PAD_GPIO5, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1,               PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_GPIO5, PADTOP_BANK, REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT9,               PINMUX_FOR_I2S_TX_MODE_2},
    {PAD_GPIO5, PADTOP_BANK, REG_ETH_MODE,                   REG_ETH_MODE_MASK,                  BIT0,               PINMUX_FOR_ETH_MODE},

    {PAD_GPIO6, PADTOP_BANK, REG_GPIO6_GPIO_MODE,            REG_GPIO6_GPIO_MODE_MASK,           BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO6, PADTOP_BANK, REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT1|BIT0,          PINMUX_FOR_I2C0_MODE_3},
    {PAD_GPIO6, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1,               PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_GPIO6, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT1,               PINMUX_FOR_DMIC_MODE_2},
    {PAD_GPIO6, PADTOP_BANK, REG_PWM6_MODE,                  REG_PWM6_MODE_MASK,                 BIT9,               PINMUX_FOR_PWM6_MODE_2},
    {PAD_GPIO6, PADTOP_BANK, REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT9,               PINMUX_FOR_I2S_TX_MODE_2},
    {PAD_GPIO6, PADTOP_BANK, REG_ETH_MODE,                   REG_ETH_MODE_MASK,                  BIT0,               PINMUX_FOR_ETH_MODE},

    {PAD_GPIO7, PADTOP_BANK, REG_PWM7_MODE,                  REG_PWM7_MODE_MASK,                 BIT13,              PINMUX_FOR_PWM7_MODE_2},
    {PAD_GPIO7, PADTOP_BANK, REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT1|BIT0,          PINMUX_FOR_I2C0_MODE_3},
    {PAD_GPIO7, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1,               PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_GPIO7, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT1,               PINMUX_FOR_DMIC_MODE_2},
    {PAD_GPIO7, PADTOP_BANK, REG_GPIO7_GPIO_MODE,            REG_GPIO7_GPIO_MODE_MASK,           BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_SR0_IO00, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1,               PINMUX_FOR_SR0_BT656_MODE_2},
    {PAD_SR0_IO00, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR0_IO00, PADTOP_BANK, REG_SR00_PDN_MODE,              REG_SR00_PDN_MODE_MASK,             BIT8,               PINMUX_FOR_SR00_PDN_MODE_1},
    {PAD_SR0_IO00, PADTOP_BANK, REG_SR01_MIPI_MODE,             REG_SR01_MIPI_MODE_MASK,            BIT10,              PINMUX_FOR_SR01_MIPI_MODE},
    {PAD_SR0_IO00, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_SR0_IO00, PADTOP_BANK, REG_SR0_IO00_GPIO_MODE,         REG_SR0_IO00_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO00, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SR0_IO00, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR0_IO00, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT3,               PINMUX_FOR_SR0_BT601_MODE_2},

    {PAD_SR0_IO01, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR0_IO01, PADTOP_BANK, REG_SR01_MIPI_MODE,             REG_SR01_MIPI_MODE_MASK,            BIT10,              PINMUX_FOR_SR01_MIPI_MODE},
    {PAD_SR0_IO01, PADTOP_BANK, REG_SR0_IO01_GPIO_MODE,         REG_SR0_IO01_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO01, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_SR0_IO01, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SR0_IO01, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR0_IO01, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT3,               PINMUX_FOR_SR0_BT601_MODE_2},

    {PAD_SR0_IO02, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT0,               PINMUX_FOR_SR00_MIPI_MODE_1},
    {PAD_SR0_IO02, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT1,               PINMUX_FOR_SR00_MIPI_MODE_2},
    {PAD_SR0_IO02, PADTOP_BANK, REG_SR0_IO02_GPIO_MODE,         REG_SR0_IO02_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO02, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR0_IO02, PADTOP_BANK, REG_SR01_MIPI_MODE,             REG_SR01_MIPI_MODE_MASK,            BIT10,              PINMUX_FOR_SR01_MIPI_MODE},
    {PAD_SR0_IO02, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,               PINMUX_FOR_TEST_IN_MODE_2},

    {PAD_SR0_IO02, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},

    {PAD_SR0_IO02, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR0_IO02, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT3,               PINMUX_FOR_SR0_BT601_MODE_2},

    {PAD_SR0_IO03, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT0,               PINMUX_FOR_SR00_MIPI_MODE_1},
    {PAD_SR0_IO03, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT1,               PINMUX_FOR_SR00_MIPI_MODE_2},
    {PAD_SR0_IO03, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR0_IO03, PADTOP_BANK, REG_SR0_IO03_GPIO_MODE,         REG_SR0_IO03_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO03, PADTOP_BANK, REG_SR01_MIPI_MODE,             REG_SR01_MIPI_MODE_MASK,            BIT10,              PINMUX_FOR_SR01_MIPI_MODE},
    {PAD_SR0_IO03, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,               PINMUX_FOR_TEST_IN_MODE_2},

    {PAD_SR0_IO03, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5,               PINMUX_FOR_TEST_OUT_MODE_2},

    {PAD_SR0_IO03, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR0_IO03, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT3,               PINMUX_FOR_SR0_BT601_MODE_2},

    {PAD_SR0_IO04, PADTOP_BANK, REG_SR0_IO04_GPIO_MODE,         REG_SR0_IO04_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO04, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SR0_IO04, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT0,               PINMUX_FOR_SR00_MIPI_MODE_1},
    {PAD_SR0_IO04, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT1,               PINMUX_FOR_SR00_MIPI_MODE_2},
    {PAD_SR0_IO04, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT2,               PINMUX_FOR_SR00_MIPI_MODE_4},
    {PAD_SR0_IO04, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR0_IO04, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR0_IO04, PADTOP_BANK, REG_SR01_MIPI_MODE,             REG_SR01_MIPI_MODE_MASK,            BIT10,              PINMUX_FOR_SR01_MIPI_MODE},
    {PAD_SR0_IO04, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},


    {PAD_SR0_IO04, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},


    {PAD_SR0_IO04, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR0_IO04, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT3,               PINMUX_FOR_SR0_BT601_MODE_2},

    {PAD_SR0_IO05, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SR0_IO05, PADTOP_BANK, REG_SR0_IO05_GPIO_MODE,         REG_SR0_IO05_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO05, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT0,               PINMUX_FOR_SR00_MIPI_MODE_1},
    {PAD_SR0_IO05, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT1,               PINMUX_FOR_SR00_MIPI_MODE_2},
    {PAD_SR0_IO05, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT2,               PINMUX_FOR_SR00_MIPI_MODE_4},
    {PAD_SR0_IO05, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR0_IO05, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR0_IO05, PADTOP_BANK, REG_SR01_MIPI_MODE,             REG_SR01_MIPI_MODE_MASK,            BIT10,              PINMUX_FOR_SR01_MIPI_MODE},
    {PAD_SR0_IO05, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},


    {PAD_SR0_IO05, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},


    {PAD_SR0_IO05, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR0_IO05, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT3,               PINMUX_FOR_SR0_BT601_MODE_2},

    {PAD_SR0_IO06, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SR0_IO06, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT0,               PINMUX_FOR_SR00_MIPI_MODE_1},
    {PAD_SR0_IO06, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT1,               PINMUX_FOR_SR00_MIPI_MODE_2},
    {PAD_SR0_IO06, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR00_MIPI_MODE_3},
    {PAD_SR0_IO06, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT2,               PINMUX_FOR_SR00_MIPI_MODE_4},
    {PAD_SR0_IO06, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR0_IO06, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR0_IO06, PADTOP_BANK, REG_SR0_IO06_GPIO_MODE,         REG_SR0_IO06_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO06, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},


    {PAD_SR0_IO06, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},


    {PAD_SR0_IO06, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR0_IO06, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT3,               PINMUX_FOR_SR0_BT601_MODE_2},

    {PAD_SR0_IO07, PADTOP_BANK, REG_SR0_IO07_GPIO_MODE,         REG_SR0_IO07_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO07, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SR0_IO07, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT0,               PINMUX_FOR_SR00_MIPI_MODE_1},
    {PAD_SR0_IO07, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT1,               PINMUX_FOR_SR00_MIPI_MODE_2},
    {PAD_SR0_IO07, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR00_MIPI_MODE_3},
    {PAD_SR0_IO07, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT2,               PINMUX_FOR_SR00_MIPI_MODE_4},
    {PAD_SR0_IO07, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR0_IO07, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR0_IO07, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},


    {PAD_SR0_IO07, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},

    {PAD_SR0_IO07, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR0_IO07, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT3,               PINMUX_FOR_SR0_BT601_MODE_2},

    {PAD_SR0_IO08, PADTOP_BANK, REG_SR0_IO08_GPIO_MODE,         REG_SR0_IO08_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO08, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SR0_IO08, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT0,               PINMUX_FOR_SR00_MIPI_MODE_1},
    {PAD_SR0_IO08, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR00_MIPI_MODE_3},
    {PAD_SR0_IO08, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT2,               PINMUX_FOR_SR00_MIPI_MODE_4},
    {PAD_SR0_IO08, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR0_IO08, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR0_IO08, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},


    {PAD_SR0_IO08, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},

    {PAD_SR0_IO08, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR0_IO08, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT3,               PINMUX_FOR_SR0_BT601_MODE_2},

    {PAD_SR0_IO09, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SR0_IO09, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT0,               PINMUX_FOR_SR00_MIPI_MODE_1},
    {PAD_SR0_IO09, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR00_MIPI_MODE_3},
    {PAD_SR0_IO09, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT2,               PINMUX_FOR_SR00_MIPI_MODE_4},
    {PAD_SR0_IO09, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR0_IO09, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR0_IO09, PADTOP_BANK, REG_SR0_IO09_GPIO_MODE,         REG_SR0_IO09_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO09, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},


    {PAD_SR0_IO09, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},

    {PAD_SR0_IO09, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR0_IO09, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT3,               PINMUX_FOR_SR0_BT601_MODE_2},

    {PAD_SR0_IO10, PADTOP_BANK, REG_SR0_IO10_GPIO_MODE,         REG_SR0_IO10_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO10, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SR0_IO10, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT0,               PINMUX_FOR_SR00_MIPI_MODE_1},
    {PAD_SR0_IO10, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR00_MIPI_MODE_3},
    {PAD_SR0_IO10, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR0_IO10, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},


    {PAD_SR0_IO10, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},

    {PAD_SR0_IO10, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR0_IO10, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT3,               PINMUX_FOR_SR0_BT601_MODE_2},

    {PAD_SR0_IO11, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SR0_IO11, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT0,               PINMUX_FOR_SR00_MIPI_MODE_1},
    {PAD_SR0_IO11, PADTOP_BANK, REG_SR00_MIPI_MODE,             REG_SR00_MIPI_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR00_MIPI_MODE_3},
    {PAD_SR0_IO11, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR0_IO11, PADTOP_BANK, REG_SR0_IO11_GPIO_MODE,         REG_SR0_IO11_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO11, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},


    {PAD_SR0_IO11, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},

    {PAD_SR0_IO11, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR0_IO11, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT3,               PINMUX_FOR_SR0_BT601_MODE_2},

    {PAD_SR0_IO12, PADTOP_BANK, REG_SR0_IO12_GPIO_MODE,         REG_SR0_IO12_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO12, PADTOP_BANK, REG_SR01_CTRL_MODE,             REG_SR01_CTRL_MODE_MASK,            BIT11,              PINMUX_FOR_SR01_CTRL_MODE},
    {PAD_SR0_IO12, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_SR0_IO12, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SR0_IO12, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},

    {PAD_SR0_IO13, PADTOP_BANK, REG_SR01_RST_MODE,              REG_SR01_RST_MODE_MASK,             BIT13,              PINMUX_FOR_SR01_RST_MODE_1},
    {PAD_SR0_IO13, PADTOP_BANK, REG_SR0_IO13_GPIO_MODE,         REG_SR0_IO13_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO13, PADTOP_BANK, REG_PWM8_MODE,                  REG_PWM8_MODE_MASK,                 BIT2|BIT1|BIT0,     PINMUX_FOR_PWM8_MODE_7},
    {PAD_SR0_IO13, PADTOP_BANK, REG_SR01_CTRL_MODE,             REG_SR01_CTRL_MODE_MASK,            BIT11,              PINMUX_FOR_SR01_CTRL_MODE},
    {PAD_SR0_IO13, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_SR0_IO13, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_TEST_OUT_MODE_3},
    {PAD_SR0_IO13, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},

    {PAD_SR0_IO14, PADTOP_BANK, REG_SR01_MCLK_MODE,             REG_SR01_MCLK_MODE_MASK,            BIT12,              PINMUX_FOR_SR01_MCLK_MODE},
    {PAD_SR0_IO14, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SR0_IO14, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR0_BT656_MODE_1},
    {PAD_SR0_IO14, PADTOP_BANK, REG_SR0_BT656_MODE,             REG_SR0_BT656_MODE_MASK,            BIT1|BIT0,          PINMUX_FOR_SR0_BT656_MODE_3},
    {PAD_SR0_IO14, PADTOP_BANK, REG_SR00_PDN_MODE,              REG_SR00_PDN_MODE_MASK,             BIT9,               PINMUX_FOR_SR00_PDN_MODE_2},
    {PAD_SR0_IO14, PADTOP_BANK, REG_SR01_CTRL_MODE,             REG_SR01_CTRL_MODE_MASK,            BIT11,              PINMUX_FOR_SR01_CTRL_MODE},
    {PAD_SR0_IO14, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_SR0_IO14, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},

    {PAD_SR0_IO14, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT2,               PINMUX_FOR_SR0_BT601_MODE_1},
    {PAD_SR0_IO14, PADTOP_BANK, REG_SR0_BT601_MODE,             REG_SR0_BT601_MODE_MASK,            BIT3,               PINMUX_FOR_SR0_BT601_MODE_2},
    {PAD_SR0_IO14, PADTOP_BANK, REG_SR0_IO14_GPIO_MODE,         REG_SR0_IO14_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_SR0_IO15, PADTOP_BANK, REG_SR00_CTRL_MODE,             REG_SR00_CTRL_MODE_MASK,            BIT4,               PINMUX_FOR_SR00_CTRL_MODE},
    {PAD_SR0_IO15, PADTOP_BANK, REG_SR01_RST_MODE,              REG_SR01_RST_MODE_MASK,             BIT14,              PINMUX_FOR_SR01_RST_MODE_2},
    {PAD_SR0_IO15, PADTOP_BANK, REG_SR0_IO15_GPIO_MODE,         REG_SR0_IO15_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO15, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_SR0_IO15, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_TEST_OUT_MODE_3},

    {PAD_SR0_IO16, PADTOP_BANK, REG_SR0_IO16_GPIO_MODE,         REG_SR0_IO16_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO16, PADTOP_BANK, REG_SR00_CTRL_MODE,             REG_SR00_CTRL_MODE_MASK,            BIT4,               PINMUX_FOR_SR00_CTRL_MODE},
    {PAD_SR0_IO16, PADTOP_BANK, REG_SR00_RST_MODE,              REG_SR00_RST_MODE_MASK,             BIT6,               PINMUX_FOR_SR00_RST_MODE},
    {PAD_SR0_IO16, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SR0_IO16, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},

    {PAD_SR0_IO16, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},


    {PAD_SR0_IO17, PADTOP_BANK, REG_SR0_IO17_GPIO_MODE,         REG_SR0_IO17_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO17, PADTOP_BANK, REG_SR00_CTRL_MODE,             REG_SR00_CTRL_MODE_MASK,            BIT4,               PINMUX_FOR_SR00_CTRL_MODE},
    {PAD_SR0_IO17, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SR0_IO17, PADTOP_BANK, REG_SR00_MCLK_MODE,             REG_SR00_MCLK_MODE_MASK,            BIT5,               PINMUX_FOR_SR00_MCLK_MODE},
    {PAD_SR0_IO17, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},

    {PAD_SR0_IO17, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},


    {PAD_SR0_IO18, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SR0_IO18, PADTOP_BANK, REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT4,               PINMUX_FOR_I2C1_MODE_1},
    {PAD_SR0_IO18, PADTOP_BANK, REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT5|BIT4,          PINMUX_FOR_I2C1_MODE_3},
    {PAD_SR0_IO18, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},

    {PAD_SR0_IO18, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},

    {PAD_SR0_IO18, PADTOP_BANK, REG_SR0_IO18_GPIO_MODE,         REG_SR0_IO18_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_SR0_IO19, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_SR0_IO19, PADTOP_BANK, REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT4,               PINMUX_FOR_I2C1_MODE_1},
    {PAD_SR0_IO19, PADTOP_BANK, REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT5|BIT4,          PINMUX_FOR_I2C1_MODE_3},
    {PAD_SR0_IO19, PADTOP_BANK, REG_SR0_IO19_GPIO_MODE,         REG_SR0_IO19_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR0_IO19, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT0,               PINMUX_FOR_TEST_IN_MODE_1},

    {PAD_SR0_IO19, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},


    {PAD_SR1_IO00, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO00, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO00, PADTOP_BANK, REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT2,               PINMUX_FOR_I2C0_MODE_4},
    {PAD_SR1_IO00, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT6,               PINMUX_FOR_SPI1_MODE_4},
    {PAD_SR1_IO00, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO00, PADTOP_BANK, REG_SR1_MCLK_MODE,              REG_SR1_MCLK_MODE_MASK,             BIT10,              PINMUX_FOR_SR1_MCLK_MODE_2},
    {PAD_SR1_IO00, PADTOP_BANK, REG_SR1_IO00_GPIO_MODE,         REG_SR1_IO00_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO00, PADTOP_BANK, REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT2,               PINMUX_FOR_PWM0_MODE_4},
    {PAD_SR1_IO00, PADTOP_BANK, REG_PWM8_MODE,                  REG_PWM8_MODE_MASK,                 BIT2,               PINMUX_FOR_PWM8_MODE_4},
    {PAD_SR1_IO00, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_SR1_IO00, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_TEST_OUT_MODE_3},

    {PAD_SR1_IO01, PADTOP_BANK, REG_SR1_IO01_GPIO_MODE,         REG_SR1_IO01_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO01, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO01, PADTOP_BANK, REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT2,               PINMUX_FOR_I2C0_MODE_4},
    {PAD_SR1_IO01, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT6,               PINMUX_FOR_SPI1_MODE_4},
    {PAD_SR1_IO01, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO01, PADTOP_BANK, REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT6,               PINMUX_FOR_PWM1_MODE_4},
    {PAD_SR1_IO01, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO01, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_SR1_IO01, PADTOP_BANK, REG_SR1_RST_MODE,               REG_SR1_RST_MODE_MASK,              BIT12,              PINMUX_FOR_SR1_RST_MODE_2},
    {PAD_SR1_IO01, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_TEST_OUT_MODE_3},

    {PAD_SR1_IO02, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO02, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO02, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO02, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT6,               PINMUX_FOR_SPI1_MODE_4},
    {PAD_SR1_IO02, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO02, PADTOP_BANK, REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT10,              PINMUX_FOR_PWM2_MODE_4},
    {PAD_SR1_IO02, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT0,               PINMUX_FOR_MIPI_TX_MODE_1},
    {PAD_SR1_IO02, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT1,               PINMUX_FOR_MIPI_TX_MODE_2},
    {PAD_SR1_IO02, PADTOP_BANK, REG_SR1_IO02_GPIO_MODE,         REG_SR1_IO02_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO02, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT4,               PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_SR1_IO02, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5,               PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_SR1_IO02, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_SR1_IO02, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_TEST_OUT_MODE_3},

    {PAD_SR1_IO03, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO03, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO03, PADTOP_BANK, REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT14,              PINMUX_FOR_PWM3_MODE_4},
    {PAD_SR1_IO03, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO03, PADTOP_BANK, REG_SR1_IO03_GPIO_MODE,         REG_SR1_IO03_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO03, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT6,               PINMUX_FOR_SPI1_MODE_4},
    {PAD_SR1_IO03, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO03, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT0,               PINMUX_FOR_MIPI_TX_MODE_1},
    {PAD_SR1_IO03, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT1,               PINMUX_FOR_MIPI_TX_MODE_2},
    {PAD_SR1_IO03, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT4,               PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_SR1_IO03, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5,               PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_SR1_IO03, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_TEST_IN_MODE_3},
    {PAD_SR1_IO03, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_TEST_OUT_MODE_3},

    {PAD_SR1_IO04, PADTOP_BANK, REG_SR1_IO04_GPIO_MODE,         REG_SR1_IO04_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO04, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT4,               PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_SR1_IO04, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO04, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO04, PADTOP_BANK, REG_SR1_BT656_MODE,             REG_SR1_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR1_BT656_MODE},
    {PAD_SR1_IO04, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO04, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO04, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT0,               PINMUX_FOR_MIPI_TX_MODE_1},
    {PAD_SR1_IO04, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT1,               PINMUX_FOR_MIPI_TX_MODE_2},
    {PAD_SR1_IO04, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT2,               PINMUX_FOR_MIPI_TX_MODE_4},
    {PAD_SR1_IO04, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT4,               PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_SR1_IO04, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5,               PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_SR1_IO04, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT6,               PINMUX_FOR_SR1_MIPI_MODE_4},
    {PAD_SR1_IO04, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT6|BIT4,          PINMUX_FOR_SR1_MIPI_MODE_5},

    {PAD_SR1_IO05, PADTOP_BANK, REG_SR1_IO05_GPIO_MODE,         REG_SR1_IO05_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO05, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT4,               PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_SR1_IO05, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO05, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO05, PADTOP_BANK, REG_SR1_BT656_MODE,             REG_SR1_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR1_BT656_MODE},
    {PAD_SR1_IO05, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO05, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO05, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT0,               PINMUX_FOR_MIPI_TX_MODE_1},
    {PAD_SR1_IO05, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT1,               PINMUX_FOR_MIPI_TX_MODE_2},
    {PAD_SR1_IO05, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT2,               PINMUX_FOR_MIPI_TX_MODE_4},
    {PAD_SR1_IO05, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT4,               PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_SR1_IO05, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5,               PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_SR1_IO05, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT6,               PINMUX_FOR_SR1_MIPI_MODE_4},
    {PAD_SR1_IO05, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT6|BIT4,          PINMUX_FOR_SR1_MIPI_MODE_5},

    {PAD_SR1_IO06, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT4,               PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_SR1_IO06, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO06, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO06, PADTOP_BANK, REG_SR1_BT656_MODE,             REG_SR1_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR1_BT656_MODE},
    {PAD_SR1_IO06, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO06, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO06, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT0,               PINMUX_FOR_MIPI_TX_MODE_1},
    {PAD_SR1_IO06, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT1,               PINMUX_FOR_MIPI_TX_MODE_2},
    {PAD_SR1_IO06, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_MIPI_TX_MODE_3},
    {PAD_SR1_IO06, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT2,               PINMUX_FOR_MIPI_TX_MODE_4},
    {PAD_SR1_IO06, PADTOP_BANK, REG_SR1_IO06_GPIO_MODE,         REG_SR1_IO06_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO06, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT4,               PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_SR1_IO06, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5,               PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_SR1_IO06, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_SR1_MIPI_MODE_3},
    {PAD_SR1_IO06, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT6,               PINMUX_FOR_SR1_MIPI_MODE_4},
    {PAD_SR1_IO06, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT6|BIT4,          PINMUX_FOR_SR1_MIPI_MODE_5},

    {PAD_SR1_IO07, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT4,               PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_SR1_IO07, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO07, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO07, PADTOP_BANK, REG_SR1_BT656_MODE,             REG_SR1_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR1_BT656_MODE},
    {PAD_SR1_IO07, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO07, PADTOP_BANK, REG_SR1_IO07_GPIO_MODE,         REG_SR1_IO07_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO07, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO07, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT0,               PINMUX_FOR_MIPI_TX_MODE_1},
    {PAD_SR1_IO07, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT1,               PINMUX_FOR_MIPI_TX_MODE_2},
    {PAD_SR1_IO07, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_MIPI_TX_MODE_3},
    {PAD_SR1_IO07, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT2,               PINMUX_FOR_MIPI_TX_MODE_4},
    {PAD_SR1_IO07, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT4,               PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_SR1_IO07, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5,               PINMUX_FOR_SR1_MIPI_MODE_2},
    {PAD_SR1_IO07, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_SR1_MIPI_MODE_3},
    {PAD_SR1_IO07, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT6,               PINMUX_FOR_SR1_MIPI_MODE_4},
    {PAD_SR1_IO07, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT6|BIT4,          PINMUX_FOR_SR1_MIPI_MODE_5},

    {PAD_SR1_IO08, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT4,               PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_SR1_IO08, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO08, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO08, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_SR1_IO08, PADTOP_BANK, REG_SR1_BT656_MODE,             REG_SR1_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR1_BT656_MODE},
    {PAD_SR1_IO08, PADTOP_BANK, REG_SR1_IO08_GPIO_MODE,         REG_SR1_IO08_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO08, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO08, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO08, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT0,               PINMUX_FOR_MIPI_TX_MODE_1},
    {PAD_SR1_IO08, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_MIPI_TX_MODE_3},
    {PAD_SR1_IO08, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT2,               PINMUX_FOR_MIPI_TX_MODE_4},
    {PAD_SR1_IO08, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT4,               PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_SR1_IO08, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_SR1_MIPI_MODE_3},
    {PAD_SR1_IO08, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT6,               PINMUX_FOR_SR1_MIPI_MODE_4},

    {PAD_SR1_IO09, PADTOP_BANK, REG_SR1_IO09_GPIO_MODE,         REG_SR1_IO09_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO09, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT4,               PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_SR1_IO09, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO09, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO09, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_SR1_IO09, PADTOP_BANK, REG_SR1_BT656_MODE,             REG_SR1_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR1_BT656_MODE},
    {PAD_SR1_IO09, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO09, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO09, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT0,               PINMUX_FOR_MIPI_TX_MODE_1},
    {PAD_SR1_IO09, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_MIPI_TX_MODE_3},
    {PAD_SR1_IO09, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT2,               PINMUX_FOR_MIPI_TX_MODE_4},
    {PAD_SR1_IO09, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT4,               PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_SR1_IO09, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_SR1_MIPI_MODE_3},
    {PAD_SR1_IO09, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT6,               PINMUX_FOR_SR1_MIPI_MODE_4},

    {PAD_SR1_IO10, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT4,               PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_SR1_IO10, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO10, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO10, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_SR1_IO10, PADTOP_BANK, REG_SR1_BT656_MODE,             REG_SR1_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR1_BT656_MODE},
    {PAD_SR1_IO10, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO10, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO10, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT0,               PINMUX_FOR_MIPI_TX_MODE_1},
    {PAD_SR1_IO10, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_MIPI_TX_MODE_3},
    {PAD_SR1_IO10, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT4,               PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_SR1_IO10, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_SR1_MIPI_MODE_3},
    {PAD_SR1_IO10, PADTOP_BANK, REG_SR1_IO10_GPIO_MODE,         REG_SR1_IO10_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_SR1_IO11, PADTOP_BANK, REG_SR1_IO11_GPIO_MODE,         REG_SR1_IO11_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO11, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT4,               PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_SR1_IO11, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO11, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO11, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_SR1_IO11, PADTOP_BANK, REG_SR1_BT656_MODE,             REG_SR1_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR1_BT656_MODE},
    {PAD_SR1_IO11, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO11, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO11, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT0,               PINMUX_FOR_MIPI_TX_MODE_1},
    {PAD_SR1_IO11, PADTOP_BANK, REG_MIPI_TX_MODE,               REG_MIPI_TX_MODE_MASK,              BIT1|BIT0,          PINMUX_FOR_MIPI_TX_MODE_3},
    {PAD_SR1_IO11, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT4,               PINMUX_FOR_SR1_MIPI_MODE_1},
    {PAD_SR1_IO11, PADTOP_BANK, REG_SR1_MIPI_MODE,              REG_SR1_MIPI_MODE_MASK,             BIT5|BIT4,          PINMUX_FOR_SR1_MIPI_MODE_3},

    {PAD_SR1_IO12, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO12, PADTOP_BANK, REG_PWM9_MODE,                  REG_PWM9_MODE_MASK,                 BIT6,               PINMUX_FOR_PWM9_MODE_4},
    {PAD_SR1_IO12, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO12, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO12, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_SR1_IO12, PADTOP_BANK, REG_SR1_IO12_GPIO_MODE,         REG_SR1_IO12_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_SR1_IO13, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5,               PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_SR1_IO13, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO13, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO13, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO13, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_SR1_IO13, PADTOP_BANK, REG_SR1_IO13_GPIO_MODE,         REG_SR1_IO13_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_SR1_IO14, PADTOP_BANK, REG_SR1_IO14_GPIO_MODE,         REG_SR1_IO14_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO14, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT4,               PINMUX_FOR_BT656_OUT_MODE_1},
    {PAD_SR1_IO14, PADTOP_BANK, REG_SR1_BT656_MODE,             REG_SR1_BT656_MODE_MASK,            BIT0,               PINMUX_FOR_SR1_BT656_MODE},
    {PAD_SR1_IO14, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO14, PADTOP_BANK, REG_SR1_BT601_MODE,             REG_SR1_BT601_MODE_MASK,            BIT1,               PINMUX_FOR_SR1_BT601_MODE},
    {PAD_SR1_IO14, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO14, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO14, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},

    {PAD_SR1_IO15, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO15, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO15, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_SR1_IO15, PADTOP_BANK, REG_SR1_IO15_GPIO_MODE,         REG_SR1_IO15_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO15, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO15, PADTOP_BANK, REG_SR1_CTRL_MODE,              REG_SR1_CTRL_MODE_MASK,             BIT8,               PINMUX_FOR_SR1_CTRL_MODE},

    {PAD_SR1_IO16, PADTOP_BANK, REG_SR1_CTRL_MODE,              REG_SR1_CTRL_MODE_MASK,             BIT8,               PINMUX_FOR_SR1_CTRL_MODE},
    {PAD_SR1_IO16, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO16, PADTOP_BANK, REG_SR1_IO16_GPIO_MODE,         REG_SR1_IO16_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO16, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO16, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO16, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_SR1_IO16, PADTOP_BANK, REG_SR1_RST_MODE,               REG_SR1_RST_MODE_MASK,              BIT11,              PINMUX_FOR_SR1_RST_MODE_1},

    {PAD_SR1_IO17, PADTOP_BANK, REG_SR1_IO17_GPIO_MODE,         REG_SR1_IO17_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO17, PADTOP_BANK, REG_SR1_CTRL_MODE,              REG_SR1_CTRL_MODE_MASK,             BIT8,               PINMUX_FOR_SR1_CTRL_MODE},
    {PAD_SR1_IO17, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO17, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO17, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_SR1_IO17, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO17, PADTOP_BANK, REG_SR1_MCLK_MODE,              REG_SR1_MCLK_MODE_MASK,             BIT9,               PINMUX_FOR_SR1_MCLK_MODE_1},

    {PAD_SR1_IO18, PADTOP_BANK, REG_I2C2_MODE,                  REG_I2C2_MODE_MASK,                 BIT8,               PINMUX_FOR_I2C2_MODE_1},
    {PAD_SR1_IO18, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO18, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO18, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO18, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_SR1_IO18, PADTOP_BANK, REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT5,               PINMUX_FOR_I2C1_MODE_2},
    {PAD_SR1_IO18, PADTOP_BANK, REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT5|BIT4,          PINMUX_FOR_I2C1_MODE_3},
    {PAD_SR1_IO18, PADTOP_BANK, REG_SR1_IO18_GPIO_MODE,         REG_SR1_IO18_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_SR1_IO19, PADTOP_BANK, REG_I2C2_MODE,                  REG_I2C2_MODE_MASK,                 BIT8,               PINMUX_FOR_I2C2_MODE_1},
    {PAD_SR1_IO19, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_SR1_IO19, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_SR1_IO19, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1,               PINMUX_FOR_TTL16_MODE_2},
    {PAD_SR1_IO19, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_SR1_IO19, PADTOP_BANK, REG_SR1_IO19_GPIO_MODE,         REG_SR1_IO19_GPIO_MODE_MASK,        BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SR1_IO19, PADTOP_BANK, REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT5,               PINMUX_FOR_I2C1_MODE_2},
    {PAD_SR1_IO19, PADTOP_BANK, REG_I2C1_MODE,                  REG_I2C1_MODE_MASK,                 BIT5|BIT4,          PINMUX_FOR_I2C1_MODE_3},

    {PAD_GPIO8, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5,               PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_GPIO8, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5|BIT4,          PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_GPIO8, PADTOP_BANK, REG_GPIO8_GPIO_MODE,            REG_GPIO8_GPIO_MODE_MASK,           BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO8, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_GPIO8, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_GPIO8, PADTOP_BANK, REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT6,               PINMUX_FOR_I2S_RX_MODE_4},
    {PAD_GPIO8, PADTOP_BANK, REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT6|BIT5,          PINMUX_FOR_UART1_MODE_6},
    {PAD_GPIO8, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_GPIO8, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_GPIO8, PADTOP_BANK, REG_PWM0_MODE,                  REG_PWM0_MODE_MASK,                 BIT0,               PINMUX_FOR_PWM0_MODE_1},
    {PAD_GPIO8, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13,              PINMUX_FOR_I2S_RXTX_MODE_4},

    {PAD_GPIO9, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5,               PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_GPIO9, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_GPIO9, PADTOP_BANK, REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT6,               PINMUX_FOR_I2S_RX_MODE_4},
    {PAD_GPIO9, PADTOP_BANK, REG_UART1_MODE,                 REG_UART1_MODE_MASK,                BIT6|BIT5,          PINMUX_FOR_UART1_MODE_6},
    {PAD_GPIO9, PADTOP_BANK, REG_PWM1_MODE,                  REG_PWM1_MODE_MASK,                 BIT4,               PINMUX_FOR_PWM1_MODE_1},
    {PAD_GPIO9, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_GPIO9, PADTOP_BANK, REG_GPIO9_GPIO_MODE,            REG_GPIO9_GPIO_MODE_MASK,           BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO9, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13,              PINMUX_FOR_I2S_RXTX_MODE_4},

    {PAD_GPIO10, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5,               PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_GPIO10, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_GPIO10, PADTOP_BANK, REG_I2S_RX_MODE,                REG_I2S_RX_MODE_MASK,               BIT6,               PINMUX_FOR_I2S_RX_MODE_4},
    {PAD_GPIO10, PADTOP_BANK, REG_PWM2_MODE,                  REG_PWM2_MODE_MASK,                 BIT8,               PINMUX_FOR_PWM2_MODE_1},
    {PAD_GPIO10, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_GPIO10, PADTOP_BANK, REG_GPIO10_GPIO_MODE,           REG_GPIO10_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO10, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13,              PINMUX_FOR_I2S_RXTX_MODE_4},

    {PAD_GPIO11, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5,               PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_GPIO11, PADTOP_BANK, REG_PWM3_MODE,                  REG_PWM3_MODE_MASK,                 BIT12,              PINMUX_FOR_PWM3_MODE_1},
    {PAD_GPIO11, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_GPIO11, PADTOP_BANK, REG_GPIO11_GPIO_MODE,           REG_GPIO11_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO11, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_GPIO11, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13,              PINMUX_FOR_I2S_RXTX_MODE_4},

    {PAD_GPIO12, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_GPIO12, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5,               PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_GPIO12, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5|BIT4,          PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_GPIO12, PADTOP_BANK, REG_DLA_EJ_MODE,                REG_DLA_EJ_MODE_MASK,               BIT1|BIT0,          PINMUX_FOR_DLA_EJ_MODE_3},
    {PAD_GPIO12, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_GPIO12, PADTOP_BANK, REG_PWM4_MODE,                  REG_PWM4_MODE_MASK,                 BIT0,               PINMUX_FOR_PWM4_MODE_1},
    {PAD_GPIO12, PADTOP_BANK, REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT2|BIT0,          PINMUX_FOR_I2C0_MODE_5},
    {PAD_GPIO12, PADTOP_BANK, REG_GPIO12_GPIO_MODE,           REG_GPIO12_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO12, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_GPIO12, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_GPIO12, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_GPIO12, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2|BIT0,          PINMUX_FOR_DMIC_MODE_5},
    {PAD_GPIO12, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT2,               PINMUX_FOR_EMMC0_8B_MODE_2},
    {PAD_GPIO12, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT5,               PINMUX_FOR_SPI1_MODE_2},
    {PAD_GPIO12, PADTOP_BANK, REG_PWM8_MODE,                  REG_PWM8_MODE_MASK,                 BIT2|BIT0,          PINMUX_FOR_PWM8_MODE_5},
    {PAD_GPIO12, PADTOP_BANK, REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10|BIT9,         PINMUX_FOR_I2S_TX_MODE_6},
    {PAD_GPIO12, PADTOP_BANK, REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10|BIT9|BIT8,    PINMUX_FOR_I2S_TX_MODE_7},
    {PAD_GPIO12, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,               PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_GPIO12, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_GPIO12, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13|BIT12,        PINMUX_FOR_I2S_RXTX_MODE_6},

    {PAD_GPIO13, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5,               PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_GPIO13, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5|BIT4,          PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_GPIO13, PADTOP_BANK, REG_DLA_EJ_MODE,                REG_DLA_EJ_MODE_MASK,               BIT1|BIT0,          PINMUX_FOR_DLA_EJ_MODE_3},
    {PAD_GPIO13, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_GPIO13, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_GPIO13, PADTOP_BANK, REG_PWM5_MODE,                  REG_PWM5_MODE_MASK,                 BIT4,               PINMUX_FOR_PWM5_MODE_1},
    {PAD_GPIO13, PADTOP_BANK, REG_I2C0_MODE,                  REG_I2C0_MODE_MASK,                 BIT2|BIT0,          PINMUX_FOR_I2C0_MODE_5},
    {PAD_GPIO13, PADTOP_BANK, REG_PWM9_MODE,                  REG_PWM9_MODE_MASK,                 BIT6|BIT4,          PINMUX_FOR_PWM9_MODE_5},
    {PAD_GPIO13, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT5,               PINMUX_FOR_SPI1_MODE_2},
    {PAD_GPIO13, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_GPIO13, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_GPIO13, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_GPIO13, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2|BIT0,          PINMUX_FOR_DMIC_MODE_5},
    {PAD_GPIO13, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT2,               PINMUX_FOR_EMMC0_8B_MODE_2},
    {PAD_GPIO13, PADTOP_BANK, REG_GPIO13_GPIO_MODE,           REG_GPIO13_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO13, PADTOP_BANK, REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10|BIT9,         PINMUX_FOR_I2S_TX_MODE_6},
    {PAD_GPIO13, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,               PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_GPIO13, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_GPIO13, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13|BIT12,        PINMUX_FOR_I2S_RXTX_MODE_6},

    {PAD_GPIO14, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5,               PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_GPIO14, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5|BIT4,          PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_GPIO14, PADTOP_BANK, REG_DLA_EJ_MODE,                REG_DLA_EJ_MODE_MASK,               BIT1|BIT0,          PINMUX_FOR_DLA_EJ_MODE_3},
    {PAD_GPIO14, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_GPIO14, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_GPIO14, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_GPIO14, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_GPIO14, PADTOP_BANK, REG_GPIO14_GPIO_MODE,           REG_GPIO14_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO14, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_GPIO14, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT5,               PINMUX_FOR_SPI1_MODE_2},
    {PAD_GPIO14, PADTOP_BANK, REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT2,               PINMUX_FOR_UART0_MODE_4},
    {PAD_GPIO14, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT2,               PINMUX_FOR_EMMC0_8B_MODE_2},
    {PAD_GPIO14, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13|BIT12,        PINMUX_FOR_I2S_RXTX_MODE_6},
    {PAD_GPIO14, PADTOP_BANK, REG_I2S_TX_MODE,                REG_I2S_TX_MODE_MASK,               BIT10|BIT9,         PINMUX_FOR_I2S_TX_MODE_6},
    {PAD_GPIO14, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,               PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_GPIO14, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_GPIO14, PADTOP_BANK, REG_PWM6_MODE,                  REG_PWM6_MODE_MASK,                 BIT8,               PINMUX_FOR_PWM6_MODE_1},

    {PAD_GPIO15, PADTOP_BANK, REG_PWM7_MODE,                  REG_PWM7_MODE_MASK,                 BIT12,              PINMUX_FOR_PWM7_MODE_1},
    {PAD_GPIO15, PADTOP_BANK, REG_I2S_MCK_MODE,               REG_I2S_MCK_MODE_MASK,              BIT2,               PINMUX_FOR_I2S_MCK_MODE_4},
    {PAD_GPIO15, PADTOP_BANK, REG_DLA_EJ_MODE,                REG_DLA_EJ_MODE_MASK,               BIT1|BIT0,          PINMUX_FOR_DLA_EJ_MODE_3},
    {PAD_GPIO15, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT0,               PINMUX_FOR_TTL16_MODE_1},
    {PAD_GPIO15, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT1|BIT0,          PINMUX_FOR_TTL16_MODE_3},
    {PAD_GPIO15, PADTOP_BANK, REG_TTL16_MODE,                 REG_TTL16_MODE_MASK,                BIT2,               PINMUX_FOR_TTL16_MODE_4},
    {PAD_GPIO15, PADTOP_BANK, REG_OTP_TEST,                   REG_OTP_TEST_MASK,                  BIT8,               PINMUX_FOR_OTP_TEST},
    {PAD_GPIO15, PADTOP_BANK, REG_GPIO15_GPIO_MODE,           REG_GPIO15_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_GPIO15, PADTOP_BANK, REG_TTL24_MODE,                 REG_TTL24_MODE_MASK,                BIT4,               PINMUX_FOR_TTL24_MODE},
    {PAD_GPIO15, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT5,               PINMUX_FOR_SPI1_MODE_2},
    {PAD_GPIO15, PADTOP_BANK, REG_UART0_MODE,                 REG_UART0_MODE_MASK,                BIT2,               PINMUX_FOR_UART0_MODE_4},
    {PAD_GPIO15, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT2,               PINMUX_FOR_EMMC0_8B_MODE_2},
    {PAD_GPIO15, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5,               PINMUX_FOR_BT656_OUT_MODE_2},
    {PAD_GPIO15, PADTOP_BANK, REG_BT656_OUT_MODE,             REG_BT656_OUT_MODE_MASK,            BIT5|BIT4,          PINMUX_FOR_BT656_OUT_MODE_3},
    {PAD_GPIO15, CHIPTOP_BANK, REG_TEST_IN_MODE,               REG_TEST_IN_MODE_MASK,              BIT1,               PINMUX_FOR_TEST_IN_MODE_2},
    {PAD_GPIO15, CHIPTOP_BANK, REG_TEST_OUT_MODE,              REG_TEST_OUT_MODE_MASK,             BIT4,               PINMUX_FOR_TEST_OUT_MODE_1},
    {PAD_GPIO15, PADTOP_BANK, REG_I2S_RXTX_MODE,              REG_I2S_RXTX_MODE_MASK,             BIT13|BIT12,        PINMUX_FOR_I2S_RXTX_MODE_6},

    {PAD_SPI_CZ, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2|BIT1|BIT0,     PINMUX_FOR_DMIC_MODE_7},
    {PAD_SPI_CZ, PADTOP_BANK, REG_SPICSZ1_GPIO,               REG_SPICSZ1_GPIO_MASK,              0,                  PINMUX_FOR_SPICSZ1_MODE},
    {PAD_SPI_CZ, PADTOP_BANK, REG_SPI_CZ_GPIO_MODE,           REG_SPI_CZ_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SPI_CZ, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT5|BIT4,          PINMUX_FOR_SPI1_MODE_3},

    {PAD_SPI_CK, PADTOP_BANK, REG_DMIC_MODE,                  REG_DMIC_MODE_MASK,                 BIT2|BIT1|BIT0,     PINMUX_FOR_DMIC_MODE_7},
    {PAD_SPI_CK, PADTOP_BANK, REG_SPI_CK_GPIO_MODE,           REG_SPI_CK_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SPI_CK, PADTOP_BANK, REG_SPI_GPIO,                   REG_SPI_GPIO_MASK,                  0,                  PINMUX_FOR_SPI_MODE},
    {PAD_SPI_CK, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT5|BIT4,          PINMUX_FOR_SPI1_MODE_3},

    {PAD_SPI_DI, PADTOP_BANK, REG_SPI_DI_GPIO_MODE,           REG_SPI_DI_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SPI_DI, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},
    {PAD_SPI_DI, PADTOP_BANK, REG_SPI_GPIO,                   REG_SPI_GPIO_MASK,                  0,                  PINMUX_FOR_SPI_MODE},
    {PAD_SPI_DI, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT5|BIT4,          PINMUX_FOR_SPI1_MODE_3},

    {PAD_SPI_DO, PADTOP_BANK, REG_SPI_DO_GPIO_MODE,           REG_SPI_DO_GPIO_MODE_MASK,          BIT3,               PINMUX_FOR_GPIO_MODE},
    {PAD_SPI_DO, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},
    {PAD_SPI_DO, PADTOP_BANK, REG_SPI_GPIO,                   REG_SPI_GPIO_MASK,                  0,                  PINMUX_FOR_SPI_MODE},
    {PAD_SPI_DO, PADTOP_BANK, REG_SPI1_MODE,                  REG_SPI1_MODE_MASK,                 BIT5|BIT4,          PINMUX_FOR_SPI1_MODE_3},

    {PAD_SPI_WPZ, PADTOP_BANK, REG_SPIWPN_GPIO,                REG_SPIWPN_GPIO_MASK,               0,                  PINMUX_FOR_SPIWPN_MODE},
    {PAD_SPI_WPZ, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},
    {PAD_SPI_WPZ, PADTOP_BANK, REG_SPI_WPZ_GPIO_MODE,          REG_SPI_WPZ_GPIO_MODE_MASK,         BIT3,               PINMUX_FOR_GPIO_MODE},

    {PAD_SPI_HLD, PADTOP_BANK, REG_SPIHOLDN_GPIO,              REG_SPIHOLDN_GPIO_MASK,             0,                  PINMUX_FOR_SPIHOLDN_MODE},
    {PAD_SPI_HLD, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},
    {PAD_SPI_HLD, PADTOP_BANK, REG_SPI_HLD_GPIO_MODE,          REG_SPI_HLD_GPIO_MODE_MASK,         BIT3,               PINMUX_FOR_GPIO_MODE},

#if 0
    {PAD_ETH_RN, PADTOP_BANK, REG_SPIHOLDN_GPIO,              REG_SPIHOLDN_GPIO_MASK,             0,                  PINMUX_FOR_SPIHOLDN_MODE},
    {PAD_ETH_RN, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},

    {PAD_ETH_RP, PADTOP_BANK, REG_SPIHOLDN_GPIO,              REG_SPIHOLDN_GPIO_MASK,             0,                  PINMUX_FOR_SPIHOLDN_MODE},
    {PAD_ETH_RP, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},

    {PAD_ETH_TN, PADTOP_BANK, REG_SPIHOLDN_GPIO,              REG_SPIHOLDN_GPIO_MASK,             0,                  PINMUX_FOR_SPIHOLDN_MODE},
    {PAD_ETH_TN, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},

    {PAD_ETH_TP, PADTOP_BANK, REG_SPIHOLDN_GPIO,              REG_SPIHOLDN_GPIO_MASK,             0,                  PINMUX_FOR_SPIHOLDN_MODE},
    {PAD_ETH_TP, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},

    {PAD_USB2_DM, PADTOP_BANK, REG_SPIHOLDN_GPIO,              REG_SPIHOLDN_GPIO_MASK,             0,                  PINMUX_FOR_SPIHOLDN_MODE},
    {PAD_USB2_DM, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},

    {PAD_USB2_DP, PADTOP_BANK, REG_SPIHOLDN_GPIO,              REG_SPIHOLDN_GPIO_MASK,             0,                  PINMUX_FOR_SPIHOLDN_MODE},
    {PAD_USB2_DP, PADTOP_BANK, REG_EMMC0_8B_MODE,              REG_EMMC0_8B_MODE_MASK,             BIT1,               PINMUX_FOR_EMMC0_8B_MODE_1},
#endif
};

static const ST_PadModeInfo m_stPadModeInfoTbl[] =
{
    {"GPIO",    0,                                              0},
    // Non PM
    {"EJ_MODE_1",               _RIUA_16BIT(PADTOP_BANK,REG_EJ_MODE),                   REG_EJ_MODE_MASK},
    {"EJ_MODE_2",               _RIUA_16BIT(PADTOP_BANK,REG_EJ_MODE),                   REG_EJ_MODE_MASK},
    {"EJ_MODE_3",               _RIUA_16BIT(PADTOP_BANK,REG_EJ_MODE),                   REG_EJ_MODE_MASK},
    {"DLA_EJ_MODE_1",           _RIUA_16BIT(PADTOP_BANK,REG_DLA_EJ_MODE),               REG_DLA_EJ_MODE_MASK},
    {"DLA_EJ_MODE_2",           _RIUA_16BIT(PADTOP_BANK,REG_DLA_EJ_MODE),               REG_DLA_EJ_MODE_MASK},
    {"DLA_EJ_MODE_3",           _RIUA_16BIT(PADTOP_BANK,REG_DLA_EJ_MODE),               REG_DLA_EJ_MODE_MASK},
    {"TEST_IN_MODE_1",          _RIUA_16BIT(CHIPTOP_BANK,REG_TEST_IN_MODE),             REG_TEST_IN_MODE_MASK},
    {"TEST_IN_MODE_2",          _RIUA_16BIT(CHIPTOP_BANK,REG_TEST_IN_MODE),             REG_TEST_IN_MODE_MASK},
    {"TEST_IN_MODE_3",          _RIUA_16BIT(CHIPTOP_BANK,REG_TEST_IN_MODE),             REG_TEST_IN_MODE_MASK},
    {"TEST_OUT_MODE_1",         _RIUA_16BIT(CHIPTOP_BANK,REG_TEST_OUT_MODE),            REG_TEST_OUT_MODE_MASK},
    {"TEST_OUT_MODE_2",         _RIUA_16BIT(CHIPTOP_BANK,REG_TEST_OUT_MODE),            REG_TEST_OUT_MODE_MASK},
    {"TEST_OUT_MODE_3",         _RIUA_16BIT(CHIPTOP_BANK,REG_TEST_OUT_MODE),            REG_TEST_OUT_MODE_MASK},
    {"SPI_MODE",                _RIUA_16BIT(PADTOP_BANK,REG_SPI_GPIO),                  REG_SPI_GPIO_MASK},
    {"SPIWPN_MODE",             _RIUA_16BIT(PADTOP_BANK,REG_SPIWPN_GPIO),               REG_SPIWPN_GPIO_MASK},
    {"SPIHOLDN_MODE",           _RIUA_16BIT(PADTOP_BANK,REG_SPIHOLDN_GPIO),             REG_SPIHOLDN_GPIO_MASK},
    {"SPICSZ1_MODE",            _RIUA_16BIT(PADTOP_BANK,REG_SPICSZ1_GPIO),              REG_SPICSZ1_GPIO_MASK},
    {"SPICSZ2_MODE",            _RIUA_16BIT(PADTOP_BANK,REG_SPICSZ2_GPIO),              REG_SPICSZ2_GPIO_MASK},
    {"I2C0_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                 REG_I2C0_MODE_MASK},
    {"I2C0_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                 REG_I2C0_MODE_MASK},
    {"I2C0_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                 REG_I2C0_MODE_MASK},
    {"I2C0_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                 REG_I2C0_MODE_MASK},
    {"I2C0_MODE_5",             _RIUA_16BIT(PADTOP_BANK,REG_I2C0_MODE),                 REG_I2C0_MODE_MASK},
    {"I2C1_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_I2C1_MODE),                 REG_I2C1_MODE_MASK},
    {"I2C1_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_I2C1_MODE),                 REG_I2C1_MODE_MASK},
    {"I2C1_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_I2C1_MODE),                 REG_I2C1_MODE_MASK},
    {"I2C2_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_I2C2_MODE),                 REG_I2C2_MODE_MASK},
    {"I2C2_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_I2C2_MODE),                 REG_I2C2_MODE_MASK},
    {"I2C2_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_I2C2_MODE),                 REG_I2C2_MODE_MASK},
    {"I2C2_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_I2C2_MODE),                 REG_I2C2_MODE_MASK},
    {"I2C2_MODE_5",             _RIUA_16BIT(PADTOP_BANK,REG_I2C2_MODE),                 REG_I2C2_MODE_MASK},
    {"SPI0_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_SPI0_MODE),                 REG_SPI0_MODE_MASK},
    {"SPI0_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_SPI0_MODE),                 REG_SPI0_MODE_MASK},
    {"SPI0_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_SPI0_MODE),                 REG_SPI0_MODE_MASK},
    {"SPI0_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_SPI0_MODE),                 REG_SPI0_MODE_MASK},
    {"SPI1_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_SPI1_MODE),                 REG_SPI1_MODE_MASK},
    {"SPI1_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_SPI1_MODE),                 REG_SPI1_MODE_MASK},
    {"SPI1_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_SPI1_MODE),                 REG_SPI1_MODE_MASK},
    {"SPI1_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_SPI1_MODE),                 REG_SPI1_MODE_MASK},
    {"SPI1_MODE_5",             _RIUA_16BIT(PADTOP_BANK,REG_SPI1_MODE),                 REG_SPI1_MODE_MASK},
    {"FUART_MODE_1",            _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                REG_FUART_MODE_MASK},
    {"FUART_MODE_2",            _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                REG_FUART_MODE_MASK},
    {"FUART_MODE_3",            _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                REG_FUART_MODE_MASK},
    {"FUART_MODE_4",            _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                REG_FUART_MODE_MASK},
    {"FUART_MODE_5",            _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                REG_FUART_MODE_MASK},
    {"FUART_MODE_6",            _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                REG_FUART_MODE_MASK},
    {"FUART_MODE_7",            _RIUA_16BIT(PADTOP_BANK,REG_FUART_MODE),                REG_FUART_MODE_MASK},
    {"UART0_MODE_1",            _RIUA_16BIT(PADTOP_BANK,REG_UART0_MODE),                REG_UART0_MODE_MASK},
    {"UART0_MODE_2",            _RIUA_16BIT(PADTOP_BANK,REG_UART0_MODE),                REG_UART0_MODE_MASK},
    {"UART0_MODE_3",            _RIUA_16BIT(PADTOP_BANK,REG_UART0_MODE),                REG_UART0_MODE_MASK},
    {"UART0_MODE_4",            _RIUA_16BIT(PADTOP_BANK,REG_UART0_MODE),                REG_UART0_MODE_MASK},
    {"UART1_MODE_1",            _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                REG_UART1_MODE_MASK},
    {"UART1_MODE_2",            _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                REG_UART1_MODE_MASK},
    {"UART1_MODE_3",            _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                REG_UART1_MODE_MASK},
    {"UART1_MODE_4",            _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                REG_UART1_MODE_MASK},
    {"UART1_MODE_5",            _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                REG_UART1_MODE_MASK},
    {"UART1_MODE_6",            _RIUA_16BIT(PADTOP_BANK,REG_UART1_MODE),                REG_UART1_MODE_MASK},
    {"SD0_MODE",                _RIUA_16BIT(PADTOP_BANK,REG_SD0_MODE),                  REG_SD0_MODE_MASK},
    {"SD0_CDZ_MODE",            _RIUA_16BIT(PADTOP_BANK,REG_SD0_CDZ_MODE),              REG_SD0_CDZ_MODE_MASK},
    {"SD1_MODE_1",              _RIUA_16BIT(PADTOP_BANK,REG_SD1_MODE),                  REG_SD1_MODE_MASK},
    {"SD1_MODE_2",              _RIUA_16BIT(PADTOP_BANK,REG_SD1_MODE),                  REG_SD1_MODE_MASK},
    {"SD1_CDZ_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_SD1_CDZ_MODE),              REG_SD1_CDZ_MODE_MASK},
    {"SD1_CDZ_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_SD1_CDZ_MODE),              REG_SD1_CDZ_MODE_MASK},
    {"EMMC0_8B_MODE_1",         _RIUA_16BIT(PADTOP_BANK,REG_EMMC0_8B_MODE),             REG_EMMC0_8B_MODE_MASK},
    {"EMMC0_8B_MODE_2",         _RIUA_16BIT(PADTOP_BANK,REG_EMMC0_8B_MODE),             REG_EMMC0_8B_MODE_MASK},
    {"EMMC0_4B_MODE",           _RIUA_16BIT(PADTOP_BANK,REG_EMMC0_4B_MODE),             REG_EMMC0_4B_MODE_MASK},
    {"EMMC1_4B_MODE_1",         _RIUA_16BIT(PADTOP_BANK,REG_EMMC1_4B_MODE),             REG_EMMC1_4B_MODE_MASK},
    {"EMMC1_4B_MODE_2",         _RIUA_16BIT(PADTOP_BANK,REG_EMMC1_4B_MODE),             REG_EMMC1_4B_MODE_MASK},
    {"EMMC0_RST_MODE",          _RIUA_16BIT(PADTOP_BANK,REG_EMMC0_RST_MODE),            REG_EMMC0_RST_MODE_MASK},
    {"EMMC1_RST_MODE_1",        _RIUA_16BIT(PADTOP_BANK,REG_EMMC1_RST_MODE),            REG_EMMC1_RST_MODE_MASK},
    {"EMMC1_RST_MODE_2",        _RIUA_16BIT(PADTOP_BANK,REG_EMMC1_RST_MODE),            REG_EMMC1_RST_MODE_MASK},
    {"PWM0_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                 REG_PWM0_MODE_MASK},
    {"PWM0_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                 REG_PWM0_MODE_MASK},
    {"PWM0_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                 REG_PWM0_MODE_MASK},
    {"PWM0_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                 REG_PWM0_MODE_MASK},
    {"PWM0_MODE_5",             _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                 REG_PWM0_MODE_MASK},
    {"PWM0_MODE_6",             _RIUA_16BIT(PADTOP_BANK,REG_PWM0_MODE),                 REG_PWM0_MODE_MASK},
    {"PWM1_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                 REG_PWM1_MODE_MASK},
    {"PWM1_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                 REG_PWM1_MODE_MASK},
    {"PWM1_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                 REG_PWM1_MODE_MASK},
    {"PWM1_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                 REG_PWM1_MODE_MASK},
    {"PWM1_MODE_5",             _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                 REG_PWM1_MODE_MASK},
    {"PWM1_MODE_6",             _RIUA_16BIT(PADTOP_BANK,REG_PWM1_MODE),                 REG_PWM1_MODE_MASK},
    {"PWM2_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                 REG_PWM2_MODE_MASK},
    {"PWM2_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                 REG_PWM2_MODE_MASK},
    {"PWM2_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                 REG_PWM2_MODE_MASK},
    {"PWM2_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                 REG_PWM2_MODE_MASK},
    {"PWM2_MODE_5",             _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                 REG_PWM2_MODE_MASK},
    {"PWM2_MODE_6",             _RIUA_16BIT(PADTOP_BANK,REG_PWM2_MODE),                 REG_PWM2_MODE_MASK},
    {"PWM3_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                 REG_PWM3_MODE_MASK},
    {"PWM3_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                 REG_PWM3_MODE_MASK},
    {"PWM3_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                 REG_PWM3_MODE_MASK},
    {"PWM3_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                 REG_PWM3_MODE_MASK},
    {"PWM3_MODE_5",             _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                 REG_PWM3_MODE_MASK},
    {"PWM3_MODE_6",             _RIUA_16BIT(PADTOP_BANK,REG_PWM3_MODE),                 REG_PWM3_MODE_MASK},
    {"PWM4_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_PWM4_MODE),                 REG_PWM4_MODE_MASK},
    {"PWM4_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_PWM4_MODE),                 REG_PWM4_MODE_MASK},
    {"PWM4_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_PWM4_MODE),                 REG_PWM4_MODE_MASK},
    {"PWM4_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_PWM4_MODE),                 REG_PWM4_MODE_MASK},
    {"PWM4_MODE_5",             _RIUA_16BIT(PADTOP_BANK,REG_PWM4_MODE),                 REG_PWM4_MODE_MASK},
    {"PWM4_MODE_6",             _RIUA_16BIT(PADTOP_BANK,REG_PWM4_MODE),                 REG_PWM4_MODE_MASK},
    {"PWM5_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_PWM5_MODE),                 REG_PWM5_MODE_MASK},
    {"PWM5_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_PWM5_MODE),                 REG_PWM5_MODE_MASK},
    {"PWM5_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_PWM5_MODE),                 REG_PWM5_MODE_MASK},
    {"PWM5_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_PWM5_MODE),                 REG_PWM5_MODE_MASK},
    {"PWM5_MODE_5",             _RIUA_16BIT(PADTOP_BANK,REG_PWM5_MODE),                 REG_PWM5_MODE_MASK},
    {"PWM5_MODE_6",             _RIUA_16BIT(PADTOP_BANK,REG_PWM5_MODE),                 REG_PWM5_MODE_MASK},
    {"PWM6_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_PWM6_MODE),                 REG_PWM6_MODE_MASK},
    {"PWM6_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_PWM6_MODE),                 REG_PWM6_MODE_MASK},
    {"PWM6_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_PWM6_MODE),                 REG_PWM6_MODE_MASK},
    {"PWM6_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_PWM6_MODE),                 REG_PWM6_MODE_MASK},
    {"PWM6_MODE_5",             _RIUA_16BIT(PADTOP_BANK,REG_PWM6_MODE),                 REG_PWM6_MODE_MASK},
    {"PWM6_MODE_6",             _RIUA_16BIT(PADTOP_BANK,REG_PWM6_MODE),                 REG_PWM6_MODE_MASK},
    {"PWM7_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_PWM7_MODE),                 REG_PWM7_MODE_MASK},
    {"PWM7_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_PWM7_MODE),                 REG_PWM7_MODE_MASK},
    {"PWM7_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_PWM7_MODE),                 REG_PWM7_MODE_MASK},
    {"PWM7_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_PWM7_MODE),                 REG_PWM7_MODE_MASK},
    {"PWM7_MODE_5",             _RIUA_16BIT(PADTOP_BANK,REG_PWM7_MODE),                 REG_PWM7_MODE_MASK},
    {"PWM7_MODE_6",             _RIUA_16BIT(PADTOP_BANK,REG_PWM7_MODE),                 REG_PWM7_MODE_MASK},
    {"PWM8_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_PWM8_MODE),                 REG_PWM8_MODE_MASK},
    {"PWM8_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_PWM8_MODE),                 REG_PWM8_MODE_MASK},
    {"PWM8_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_PWM8_MODE),                 REG_PWM8_MODE_MASK},
    {"PWM8_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_PWM8_MODE),                 REG_PWM8_MODE_MASK},
    {"PWM8_MODE_5",             _RIUA_16BIT(PADTOP_BANK,REG_PWM8_MODE),                 REG_PWM8_MODE_MASK},
    {"PWM8_MODE_6",             _RIUA_16BIT(PADTOP_BANK,REG_PWM8_MODE),                 REG_PWM8_MODE_MASK},
    {"PWM8_MODE_7",             _RIUA_16BIT(PADTOP_BANK,REG_PWM8_MODE),                 REG_PWM8_MODE_MASK},
    {"PWM8_MODE_8",             _RIUA_16BIT(PADTOP_BANK,REG_PWM8_MODE),                 REG_PWM8_MODE_MASK},
    {"PWM9_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_PWM9_MODE),                 REG_PWM9_MODE_MASK},
    {"PWM9_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_PWM9_MODE),                 REG_PWM9_MODE_MASK},
    {"PWM9_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_PWM9_MODE),                 REG_PWM9_MODE_MASK},
    {"PWM9_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_PWM9_MODE),                 REG_PWM9_MODE_MASK},
    {"PWM9_MODE_5",             _RIUA_16BIT(PADTOP_BANK,REG_PWM9_MODE),                 REG_PWM9_MODE_MASK},
    {"PWM9_MODE_6",             _RIUA_16BIT(PADTOP_BANK,REG_PWM9_MODE),                 REG_PWM9_MODE_MASK},
    {"PWM9_MODE_7",             _RIUA_16BIT(PADTOP_BANK,REG_PWM9_MODE),                 REG_PWM9_MODE_MASK},
    {"PWM9_MODE_8",             _RIUA_16BIT(PADTOP_BANK,REG_PWM9_MODE),                 REG_PWM9_MODE_MASK},
    {"ETH_MODE",                _RIUA_16BIT(PADTOP_BANK,REG_ETH_MODE),                  REG_ETH_MODE_MASK},
    {"LED0_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_LED0_MODE),                 REG_LED0_MODE_MASK},
    {"LED0_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_LED0_MODE),                 REG_LED0_MODE_MASK},
    {"LED0_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_LED0_MODE),                 REG_LED0_MODE_MASK},
    {"LED0_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_LED0_MODE),                 REG_LED0_MODE_MASK},
    {"LED1_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_LED1_MODE),                 REG_LED1_MODE_MASK},
    {"LED1_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_LED1_MODE),                 REG_LED1_MODE_MASK},
    {"LED1_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_LED1_MODE),                 REG_LED1_MODE_MASK},
    {"LED1_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_LED1_MODE),                 REG_LED1_MODE_MASK},
    {"I2S_MCK_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_I2S_MCK_MODE),              REG_I2S_MCK_MODE_MASK},
    {"I2S_MCK_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_I2S_MCK_MODE),              REG_I2S_MCK_MODE_MASK},
    {"I2S_MCK_MODE_3",          _RIUA_16BIT(PADTOP_BANK,REG_I2S_MCK_MODE),              REG_I2S_MCK_MODE_MASK},
    {"I2S_MCK_MODE_4",          _RIUA_16BIT(PADTOP_BANK,REG_I2S_MCK_MODE),              REG_I2S_MCK_MODE_MASK},
    {"I2S_RX_MODE_1",           _RIUA_16BIT(PADTOP_BANK,REG_I2S_RX_MODE),               REG_I2S_RX_MODE_MASK},
    {"I2S_RX_MODE_2",           _RIUA_16BIT(PADTOP_BANK,REG_I2S_RX_MODE),               REG_I2S_RX_MODE_MASK},
    {"I2S_RX_MODE_3",           _RIUA_16BIT(PADTOP_BANK,REG_I2S_RX_MODE),               REG_I2S_RX_MODE_MASK},
    {"I2S_RX_MODE_4",           _RIUA_16BIT(PADTOP_BANK,REG_I2S_RX_MODE),               REG_I2S_RX_MODE_MASK},
    {"I2S_TX_MODE_1",           _RIUA_16BIT(PADTOP_BANK,REG_I2S_TX_MODE),               REG_I2S_TX_MODE_MASK},
    {"I2S_TX_MODE_2",           _RIUA_16BIT(PADTOP_BANK,REG_I2S_TX_MODE),               REG_I2S_TX_MODE_MASK},
    {"I2S_TX_MODE_3",           _RIUA_16BIT(PADTOP_BANK,REG_I2S_TX_MODE),               REG_I2S_TX_MODE_MASK},
    {"I2S_TX_MODE_4",           _RIUA_16BIT(PADTOP_BANK,REG_I2S_TX_MODE),               REG_I2S_TX_MODE_MASK},
    {"I2S_TX_MODE_5",           _RIUA_16BIT(PADTOP_BANK,REG_I2S_TX_MODE),               REG_I2S_TX_MODE_MASK},
    {"I2S_TX_MODE_6",           _RIUA_16BIT(PADTOP_BANK,REG_I2S_TX_MODE),               REG_I2S_TX_MODE_MASK},
    {"I2S_TX_MODE_7",           _RIUA_16BIT(PADTOP_BANK,REG_I2S_TX_MODE),               REG_I2S_TX_MODE_MASK},
    {"I2S_RXTX_MODE_1",         _RIUA_16BIT(PADTOP_BANK,REG_I2S_RXTX_MODE),             REG_I2S_RXTX_MODE_MASK},
    {"I2S_RXTX_MODE_2",         _RIUA_16BIT(PADTOP_BANK,REG_I2S_RXTX_MODE),             REG_I2S_RXTX_MODE_MASK},
    {"I2S_RXTX_MODE_3",         _RIUA_16BIT(PADTOP_BANK,REG_I2S_RXTX_MODE),             REG_I2S_RXTX_MODE_MASK},
    {"I2S_RXTX_MODE_4",         _RIUA_16BIT(PADTOP_BANK,REG_I2S_RXTX_MODE),             REG_I2S_RXTX_MODE_MASK},
    {"I2S_RXTX_MODE_5",         _RIUA_16BIT(PADTOP_BANK,REG_I2S_RXTX_MODE),             REG_I2S_RXTX_MODE_MASK},
    {"I2S_RXTX_MODE_6",         _RIUA_16BIT(PADTOP_BANK,REG_I2S_RXTX_MODE),             REG_I2S_RXTX_MODE_MASK},
    {"DMIC_MODE_1",             _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                 REG_DMIC_MODE_MASK},
    {"DMIC_MODE_2",             _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                 REG_DMIC_MODE_MASK},
    {"DMIC_MODE_3",             _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                 REG_DMIC_MODE_MASK},
    {"DMIC_MODE_4",             _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                 REG_DMIC_MODE_MASK},
    {"DMIC_MODE_5",             _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                 REG_DMIC_MODE_MASK},
    {"DMIC_MODE_6",             _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                 REG_DMIC_MODE_MASK},
    {"DMIC_MODE_7",             _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                 REG_DMIC_MODE_MASK},
    {"DMIC_MODE_8",             _RIUA_16BIT(PADTOP_BANK,REG_DMIC_MODE),                 REG_DMIC_MODE_MASK},
    {"SR00_MIPI_MODE_1",        _RIUA_16BIT(PADTOP_BANK,REG_SR00_MIPI_MODE),            REG_SR00_MIPI_MODE_MASK},
    {"SR00_MIPI_MODE_2",        _RIUA_16BIT(PADTOP_BANK,REG_SR00_MIPI_MODE),            REG_SR00_MIPI_MODE_MASK},
    {"SR00_MIPI_MODE_3",        _RIUA_16BIT(PADTOP_BANK,REG_SR00_MIPI_MODE),            REG_SR00_MIPI_MODE_MASK},
    {"SR00_MIPI_MODE_4",        _RIUA_16BIT(PADTOP_BANK,REG_SR00_MIPI_MODE),            REG_SR00_MIPI_MODE_MASK},
    {"SR00_CTRL_MODE",          _RIUA_16BIT(PADTOP_BANK,REG_SR00_CTRL_MODE),            REG_SR00_CTRL_MODE_MASK},
    {"SR00_MCLK_MODE",          _RIUA_16BIT(PADTOP_BANK,REG_SR00_MCLK_MODE),            REG_SR00_MCLK_MODE_MASK},
    {"SR00_RST_MODE",           _RIUA_16BIT(PADTOP_BANK,REG_SR00_RST_MODE),             REG_SR00_RST_MODE_MASK},
    {"SR00_PDN_MODE_1",         _RIUA_16BIT(PADTOP_BANK,REG_SR00_PDN_MODE),             REG_SR00_PDN_MODE_MASK},
    {"SR00_PDN_MODE_2",         _RIUA_16BIT(PADTOP_BANK,REG_SR00_PDN_MODE),             REG_SR00_PDN_MODE_MASK},
    {"SR01_MIPI_MODE",          _RIUA_16BIT(PADTOP_BANK,REG_SR01_MIPI_MODE),            REG_SR01_MIPI_MODE_MASK},
    {"SR01_CTRL_MODE",          _RIUA_16BIT(PADTOP_BANK,REG_SR01_CTRL_MODE),            REG_SR01_CTRL_MODE_MASK},
    {"SR01_MCLK_MODE",          _RIUA_16BIT(PADTOP_BANK,REG_SR01_MCLK_MODE),            REG_SR01_MCLK_MODE_MASK},
    {"SR01_RST_MODE_1",         _RIUA_16BIT(PADTOP_BANK,REG_SR01_RST_MODE),             REG_SR01_RST_MODE_MASK},
    {"SR01_RST_MODE_2",         _RIUA_16BIT(PADTOP_BANK,REG_SR01_RST_MODE),             REG_SR01_RST_MODE_MASK},
    {"SR0_BT601_MODE_1",        _RIUA_16BIT(PADTOP_BANK,REG_SR0_BT601_MODE),            REG_SR0_BT601_MODE_MASK},
    {"SR0_BT601_MODE_2",        _RIUA_16BIT(PADTOP_BANK,REG_SR0_BT601_MODE),            REG_SR0_BT601_MODE_MASK},
    {"SR0_BT656_MODE_1",        _RIUA_16BIT(PADTOP_BANK,REG_SR0_BT656_MODE),            REG_SR0_BT656_MODE_MASK},
    {"SR0_BT656_MODE_2",        _RIUA_16BIT(PADTOP_BANK,REG_SR0_BT656_MODE),            REG_SR0_BT656_MODE_MASK},
    {"SR0_BT656_MODE_3",        _RIUA_16BIT(PADTOP_BANK,REG_SR0_BT656_MODE),            REG_SR0_BT656_MODE_MASK},
    {"SR1_MIPI_MODE_1",         _RIUA_16BIT(PADTOP_BANK,REG_SR1_MIPI_MODE),             REG_SR1_MIPI_MODE_MASK},
    {"SR1_MIPI_MODE_2",         _RIUA_16BIT(PADTOP_BANK,REG_SR1_MIPI_MODE),             REG_SR1_MIPI_MODE_MASK},
    {"SR1_MIPI_MODE_3",         _RIUA_16BIT(PADTOP_BANK,REG_SR1_MIPI_MODE),             REG_SR1_MIPI_MODE_MASK},
    {"SR1_MIPI_MODE_4",         _RIUA_16BIT(PADTOP_BANK,REG_SR1_MIPI_MODE),             REG_SR1_MIPI_MODE_MASK},
    {"SR1_MIPI_MODE_5",         _RIUA_16BIT(PADTOP_BANK,REG_SR1_MIPI_MODE),             REG_SR1_MIPI_MODE_MASK},
    {"SR1_CTRL_MODE",           _RIUA_16BIT(PADTOP_BANK,REG_SR1_CTRL_MODE),             REG_SR1_CTRL_MODE_MASK},
    {"SR1_MCLK_MODE_1",         _RIUA_16BIT(PADTOP_BANK,REG_SR1_MCLK_MODE),             REG_SR1_MCLK_MODE_MASK},
    {"SR1_MCLK_MODE_2",         _RIUA_16BIT(PADTOP_BANK,REG_SR1_MCLK_MODE),             REG_SR1_MCLK_MODE_MASK},
    {"SR1_RST_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_SR1_RST_MODE),              REG_SR1_RST_MODE_MASK},
    {"SR1_RST_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_SR1_RST_MODE),              REG_SR1_RST_MODE_MASK},
    {"SR1_BT601_MODE",          _RIUA_16BIT(PADTOP_BANK,REG_SR1_BT601_MODE),            REG_SR1_BT601_MODE_MASK},
    {"SR1_BT656_MODE",          _RIUA_16BIT(PADTOP_BANK,REG_SR1_BT656_MODE),            REG_SR1_BT656_MODE_MASK},
    {"MIPI_TX_MODE_1",          _RIUA_16BIT(PADTOP_BANK,REG_MIPI_TX_MODE),              REG_MIPI_TX_MODE_MASK},
    {"MIPI_TX_MODE_2",          _RIUA_16BIT(PADTOP_BANK,REG_MIPI_TX_MODE),              REG_MIPI_TX_MODE_MASK},
    {"MIPI_TX_MODE_3",          _RIUA_16BIT(PADTOP_BANK,REG_MIPI_TX_MODE),              REG_MIPI_TX_MODE_MASK},
    {"MIPI_TX_MODE_4",          _RIUA_16BIT(PADTOP_BANK,REG_MIPI_TX_MODE),              REG_MIPI_TX_MODE_MASK},
    {"TTL24_MODE",              _RIUA_16BIT(PADTOP_BANK,REG_TTL24_MODE),                REG_TTL24_MODE_MASK},
    {"TTL16_MODE_1",            _RIUA_16BIT(PADTOP_BANK,REG_TTL16_MODE),                REG_TTL16_MODE_MASK},
    {"TTL16_MODE_2",            _RIUA_16BIT(PADTOP_BANK,REG_TTL16_MODE),                REG_TTL16_MODE_MASK},
    {"TTL16_MODE_3",            _RIUA_16BIT(PADTOP_BANK,REG_TTL16_MODE),                REG_TTL16_MODE_MASK},
    {"TTL16_MODE_4",            _RIUA_16BIT(PADTOP_BANK,REG_TTL16_MODE),                REG_TTL16_MODE_MASK},
    {"BT656_OUT_MODE_1",        _RIUA_16BIT(PADTOP_BANK,REG_BT656_OUT_MODE),            REG_BT656_OUT_MODE_MASK},
    {"BT656_OUT_MODE_2",        _RIUA_16BIT(PADTOP_BANK,REG_BT656_OUT_MODE),            REG_BT656_OUT_MODE_MASK},
    {"BT656_OUT_MODE_3",        _RIUA_16BIT(PADTOP_BANK,REG_BT656_OUT_MODE),            REG_BT656_OUT_MODE_MASK},
    {"OTP_TEST",                _RIUA_16BIT(PADTOP_BANK,REG_OTP_TEST),                  REG_OTP_TEST_MASK},
    // PM Sleep and PM PADTOP
    {"PM_SPI_MODE",             _RIUA_16BIT(PMSLEEP_BANK,REG_PM_SPI_GPIO),               REG_PM_SPI_GPIO_MASK},
    {"PM_SPIWPN_MODE",          _RIUA_16BIT(PMSLEEP_BANK,REG_PM_SPIWPN_GPIO),            REG_PM_SPIWPN_GPIO_MASK},
    {"PM_SPIHOLDN_MODE",        _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_SPIHOLDN_MODE),          REG_PM_SPIHOLDN_MODE_MASK},
    {"PM_SPICSZ1_MODE",         _RIUA_16BIT(PMSLEEP_BANK,REG_PM_SPICSZ1_GPIO),           REG_PM_SPICSZ1_GPIO_MASK},
    {"PM_SPICSZ2_MODE",         _RIUA_16BIT(PMSLEEP_BANK,REG_PM_SPICSZ2_GPIO),           REG_PM_SPICSZ2_GPIO_MASK},
    {"PM_VID_MODE_1",           _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_VID_MODE),               REG_PM_VID_MODE_MASK},
    {"PM_VID_MODE_2",           _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_VID_MODE),               REG_PM_VID_MODE_MASK},
    {"PM_PIR_SERIN_MODE_1",     _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PIR_SERIN_MODE),         REG_PM_PIR_SERIN_MODE_MASK},
    {"PM_PIR_SERIN_MODE_2",     _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PIR_SERIN_MODE),         REG_PM_PIR_SERIN_MODE_MASK},
    {"PM_PIR_DIR_LINK_MODE_1",  _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PIR_DIR_LINK_MODE),      REG_PM_PIR_DIR_LINK_MODE_MASK},
    {"PM_PIR_DIR_LINK_MODE_2",  _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PIR_DIR_LINK_MODE),      REG_PM_PIR_DIR_LINK_MODE_MASK},
    {"PM_I2CM_MODE_1",          _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_I2CM_MODE),              REG_PM_I2CM_MODE_MASK},
    {"PM_I2CM_MODE_2",          _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_I2CM_MODE),              REG_PM_I2CM_MODE_MASK},
    {"PM_PWM0_MODE_1",          _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PWM0_MODE),              REG_PM_PWM0_MODE_MASK},
    {"PM_PWM0_MODE_2",          _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PWM0_MODE),              REG_PM_PWM0_MODE_MASK},
    {"PM_UART1_MODE_1",         _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_UART1_MODE),             REG_PM_UART1_MODE_MASK},
    {"PM_UART1_MODE_2",         _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_UART1_MODE),             REG_PM_UART1_MODE_MASK},
    {"PM_PM51_UART_MODE_1",     _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM51_UART_MODE),         REG_PM_PM51_UART_MODE_MASK},
    {"PM_PM51_UART_MODE_2",     _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM51_UART_MODE),         REG_PM_PM51_UART_MODE_MASK},
    {"PM_PM51_UART_MODE_3",     _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM51_UART_MODE),         REG_PM_PM51_UART_MODE_MASK},
    {"PM_PM_PAD_EXT_MODE_0",    _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_0),      REG_PM_PM_PAD_EXT_MODE_0_MASK},
    {"PM_PM_PAD_EXT_MODE_1",    _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_1),      REG_PM_PM_PAD_EXT_MODE_1_MASK},
    {"PM_PM_PAD_EXT_MODE_2",    _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_2),      REG_PM_PM_PAD_EXT_MODE_2_MASK},
    {"PM_PM_PAD_EXT_MODE_3",    _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_3),      REG_PM_PM_PAD_EXT_MODE_3_MASK},
    {"PM_PM_PAD_EXT_MODE_4",    _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_4),      REG_PM_PM_PAD_EXT_MODE_4_MASK},
    {"PM_PM_PAD_EXT_MODE_5",    _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_5),      REG_PM_PM_PAD_EXT_MODE_5_MASK},
    {"PM_PM_PAD_EXT_MODE_6",    _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_6),      REG_PM_PM_PAD_EXT_MODE_6_MASK},
    {"PM_PM_PAD_EXT_MODE_7",    _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_7),      REG_PM_PM_PAD_EXT_MODE_7_MASK},
    {"PM_PM_PAD_EXT_MODE_8",    _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_8),      REG_PM_PM_PAD_EXT_MODE_8_MASK},
    {"PM_PM_PAD_EXT_MODE_9",    _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_9),      REG_PM_PM_PAD_EXT_MODE_9_MASK},
    {"PM_PM_PAD_EXT_MODE_10",   _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_10),     REG_PM_PM_PAD_EXT_MODE_10_MASK},
    {"PM_PM_PAD_EXT_MODE_11",   _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_11),     REG_PM_PM_PAD_EXT_MODE_11_MASK},
    {"PM_PM_PAD_EXT_MODE_12",   _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_12),     REG_PM_PM_PAD_EXT_MODE_12_MASK},
    {"PM_PM_PAD_EXT_MODE_13",   _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_13),     REG_PM_PM_PAD_EXT_MODE_13_MASK},
    {"PM_PM_PAD_EXT_MODE_14",   _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_14),     REG_PM_PM_PAD_EXT_MODE_14_MASK},
    {"PM_PM_PAD_EXT_MODE_15",   _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_15),     REG_PM_PM_PAD_EXT_MODE_15_MASK},
    {"PM_PM_PAD_EXT_MODE_16",   _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_16),     REG_PM_PM_PAD_EXT_MODE_16_MASK},
    {"PM_PM_PAD_EXT_MODE_17",   _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_17),     REG_PM_PM_PAD_EXT_MODE_17_MASK},
    {"PM_PM_PAD_EXT_MODE_18",   _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_18),     REG_PM_PM_PAD_EXT_MODE_18_MASK},
    {"PM_PM_PAD_EXT_MODE_19",   _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_19),     REG_PM_PM_PAD_EXT_MODE_19_MASK},
    {"PM_PM_PAD_EXT_MODE_20",   _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_20),     REG_PM_PM_PAD_EXT_MODE_20_MASK},
    {"PM_PM_PAD_EXT_MODE_21",   _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_21),     REG_PM_PM_PAD_EXT_MODE_21_MASK},
    {"PM_PM_PAD_EXT_MODE_22",   _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_PM_PAD_EXT_MODE_22),     REG_PM_PM_PAD_EXT_MODE_22_MASK},
    {"PM_PM_UART_IS_MODE",      _RIUA_16BIT(PM_PADTOP_BANK,REG_PM_UART_IS_GPIO),           REG_PM_UART_IS_GPIO_MASK},
    {"PM_SAR_MODE",             _RIUA_16BIT(PM_SAR_BANK,REG_SAR_GPIO0_GPIO_MODE),          REG_SAR_GPIO_MODE_MASK},
};

//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================

//------------------------------------------------------------------------------
//  Function    : _HalCheckPin
//  Description :
//------------------------------------------------------------------------------
static S32 _HalCheckPin(U32 padID)
{
    if (GPIO_NR <= padID) {
        return FALSE;
    }
    return TRUE;
}

#if 0
static void _HalSARGPIOWriteRegBit(u32 u32RegOffset, bool bEnable, U8 u8BitMsk)
{
    if (bEnable)
        _GPIO_R_BYTE(_RIUA_8BIT(PM_SAR_BANK, u32RegOffset)) |= u8BitMsk;
    else
        _GPIO_R_BYTE(_RIUA_8BIT(PM_SAR_BANK, u32RegOffset)) &= (~u8BitMsk);
}
#endif

static void _HalPadDisablePadMux(U32 u32PadModeID)
{
    if (_GPIO_R_WORD_MASK(m_stPadModeInfoTbl[u32PadModeID].u32ModeRIU, m_stPadModeInfoTbl[u32PadModeID].u32ModeMask)) {
        _GPIO_W_WORD_MASK(m_stPadModeInfoTbl[u32PadModeID].u32ModeRIU, 0, m_stPadModeInfoTbl[u32PadModeID].u32ModeMask);
    }
}

static S32 HalPadSetMode_General(U32 u32PadID, U32 u32Mode)
{
    U32 u32RegAddr = 0;
    U16 u16RegVal  = 0;
    U8  u8ModeIsFind = 0;
    U16 i = 0;

    for (i = 0; i < sizeof(m_stPadMuxTbl)/sizeof(struct stPadMux); i++)
    {
        if (u32PadID == m_stPadMuxTbl[i].padID)
        {
            u32RegAddr = _RIUA_16BIT(m_stPadMuxTbl[i].base, m_stPadMuxTbl[i].offset);

            if (u32Mode == m_stPadMuxTbl[i].mode)
            {
                u16RegVal = _GPIO_R_WORD_MASK(u32RegAddr, 0xFFFF);
                u16RegVal &= ~(m_stPadMuxTbl[i].mask);
                u16RegVal |= m_stPadMuxTbl[i].val; // CHECK Multi-Pad Mode

                _GPIO_W_WORD_MASK(u32RegAddr, u16RegVal, 0xFFFF);

                u8ModeIsFind = 1;
#if (ENABLE_CHECK_ALL_PAD_CONFLICT == 0)
                break;
#endif
            }
            else
            {
                u16RegVal = _GPIO_R_WORD_MASK(u32RegAddr, m_stPadMuxTbl[i].mask);

                if (u16RegVal == m_stPadMuxTbl[i].val)
                {
                    printk(KERN_INFO"[Padmux]reset PAD%d(reg 0x%x:%x; mask0x%x) t0 %s (org: %s)\n",
                           u32PadID,
                           m_stPadMuxTbl[i].base,
                           m_stPadMuxTbl[i].offset,
                           m_stPadMuxTbl[i].mask,
                           m_stPadModeInfoTbl[u32Mode].u8PadName,
                           m_stPadModeInfoTbl[m_stPadMuxTbl[i].mode].u8PadName);
                    if (m_stPadMuxTbl[i].val != 0)
                    {
                        _GPIO_W_WORD_MASK(u32RegAddr, 0, m_stPadMuxTbl[i].mask);
                    }
                    else
                    {
                        _GPIO_W_WORD_MASK(u32RegAddr, m_stPadMuxTbl[i].mask, m_stPadMuxTbl[i].mask);
                    }
                }
            }
        }
    }

    return (u8ModeIsFind) ? 0 : -1;
}

static S32 HalPadSetMode_MISC(U32 u32PadID, U32 u32Mode)
{
    switch(u32PadID)
    {
    /* FUART */
    case PAD_FUART_RX: /* reg_ej_mode; reg[103C60]#7 ~ #8 */
    case PAD_FUART_TX:
    case PAD_FUART_CTS:
    case PAD_FUART_RTS:
        if (u32Mode == PINMUX_FOR_GPIO_MODE)
        {
            _HalPadDisablePadMux(PINMUX_FOR_EJ_MODE_1);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(PADTOP_BANK,REG_FUART_RX_GPIO_MODE+(u32PadID-PAD_FUART_RX)), BIT3, REG_FUART_RX_GPIO_MODE_MASK);

        }
        else if (u32Mode == PINMUX_FOR_FUART_MODE_1)
        {
            _HalPadDisablePadMux(PINMUX_FOR_EJ_MODE_1);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(PADTOP_BANK,REG_FUART_RX_GPIO_MODE+(u32PadID-PAD_FUART_RX)), 0, REG_FUART_RX_GPIO_MODE_MASK);
        }
        else
        {
            return -1;
        }
        break;
    /* SAR */
    case PAD_SAR_GPIO0: /* reg_sar_aisel; reg[1422]#5 ~ #0=0b */
    case PAD_SAR_GPIO1:
    case PAD_SAR_GPIO2:
    case PAD_SAR_GPIO3:
    case PAD_SAR_GPIO4:
    case PAD_SAR_GPIO5:
        if (u32Mode == PINMUX_FOR_GPIO_MODE)
        {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(PM_SAR_BANK,REG_SAR_GPIO0_GPIO_MODE), 0, 1<<(u32PadID-PAD_SAR_GPIO0));
        }
        else if (u32Mode == PINMUX_FOR_PM_SAR_MODE)
        {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(PM_SAR_BANK,REG_SAR_GPIO0_GPIO_MODE), 1<<(u32PadID-PAD_SAR_GPIO0), 1<<(u32PadID-PAD_SAR_GPIO0));
        }
        else
        {
            return -1;
        }
        break;
    /* lan-top */
    case PAD_ETH_RN:
    case PAD_ETH_RP:
    case PAD_ETH_TN:
    case PAD_ETH_TP:
        if (u32Mode == PINMUX_FOR_GPIO_MODE)
        {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY1_BANK,REG_ATOP_RX_INOFF), BIT14, BIT14);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY2_BANK,REG_ETH_GPIO_EN), 1<<(u32PadID-PAD_ETH_RN), 1<<(u32PadID-PAD_ETH_RN));
        }
        else if (u32Mode == PINMUX_FOR_ETH_MODE)
        {
            _HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE_2);
            _HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE_2);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY1_BANK,REG_ATOP_RX_INOFF), 0, BIT14);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(ALBANY2_BANK,REG_ETH_GPIO_EN), 0, 1<<(u32PadID-PAD_ETH_RN));
        }
        else
        {
            return -1;
        }
        break;
    /* UTMI */
    case PAD_USB2_DM:
    case PAD_USB2_DP:
        if (u32Mode == PINMUX_FOR_GPIO_MODE)
        {
            //_HalPadDisablePadMux(PINMUX_FOR_TEST_IN_MODE);
            //_HalPadDisablePadMux(PINMUX_FOR_TEST_OUT_MODE);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(UTMI0_BANK,REG_UTMI0_GPIO_EN), REG_UTMI0_GPIO_EN_MASK, REG_UTMI0_GPIO_EN_MASK);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(UTMI0_BANK,REG_UTMI0_CLK_EXTRA0_EN), ~REG_UTMI0_CLK_EXTRA0_EN_MASK, REG_UTMI0_CLK_EXTRA0_EN_MASK);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(UTMI0_BANK,REG_UTMI0_REG_PDN), ~REG_UTMI0_REG_PDN_MASK, REG_UTMI0_REG_PDN_MASK);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(UTMI0_BANK,REG_UTMI0_FL_XVR_PDN), ~REG_UTMI0_FL_XVR_PDN_MASK, REG_UTMI0_FL_XVR_PDN_MASK);
        }
        else if (u32Mode == PINMUX_FOR_USB_MODE)
        {
            _GPIO_W_WORD_MASK(_RIUA_16BIT(UTMI0_BANK,REG_UTMI0_GPIO_EN), ~REG_UTMI0_GPIO_EN_MASK, REG_UTMI0_GPIO_EN_MASK);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(UTMI0_BANK,REG_UTMI0_CLK_EXTRA0_EN), REG_UTMI0_CLK_EXTRA0_EN_MASK, REG_UTMI0_CLK_EXTRA0_EN_MASK);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(UTMI0_BANK,REG_UTMI0_REG_PDN), REG_UTMI0_REG_PDN_MASK, REG_UTMI0_REG_PDN_MASK);
            _GPIO_W_WORD_MASK(_RIUA_16BIT(UTMI0_BANK,REG_UTMI0_FL_XVR_PDN), REG_UTMI0_FL_XVR_PDN_MASK, REG_UTMI0_FL_XVR_PDN_MASK);
        }
        else
        {
            return -1;
        }
        break;
    default:
        break;
    }

    return 0;
}

//------------------------------------------------------------------------------
//  Function    : HalPadSetVal
//  Description :
//------------------------------------------------------------------------------
S32 HalPadSetVal(U32 u32PadID, U32 u32Mode)
{
    if (FALSE == _HalCheckPin(u32PadID)) {
        return FALSE;
    }

    if ((u32PadID >= PAD_FUART_RX && u32PadID <= PAD_FUART_RTS) ||
        (u32PadID >= PAD_SAR_GPIO0 && u32PadID <= PAD_SAR_GPIO5) ||
        (u32PadID >= PAD_ETH_RN && u32PadID <= PAD_USB2_DP)) 
    {
        return HalPadSetMode_MISC(u32PadID, u32Mode);
    }
    else
    {
        return HalPadSetMode_General(u32PadID, u32Mode);
    }
}
//------------------------------------------------------------------------------
//  Function    : HalPadSet
//  Description :
//------------------------------------------------------------------------------
S32 HalPadSetMode(U32 u32Mode)
{
    U32 u32PadID;
    U16 k = 0;

    for (k = 0; k < sizeof(m_stPadMuxTbl)/sizeof(struct stPadMux); k++)
    {
        if (u32Mode == m_stPadMuxTbl[k].mode)
        {
            u32PadID = m_stPadMuxTbl[k].padID;
            if (HalPadSetMode_General( u32PadID, u32Mode) < 0)
            {
                return -1;
            }
        }
    }

    return 0;
}
