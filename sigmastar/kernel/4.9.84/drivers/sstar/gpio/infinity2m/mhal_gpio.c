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

#include "mhal_gpio.h"
#include "ms_platform.h"
#include "gpio.h"
#include "irqs.h"
#include "padmux.h"

#include "gpio_table.h"
#include "mhal_pinmux.h"

//-------------------------------------------------------------------------------------------------
//  Local Defines
//-------------------------------------------------------------------------------------------------
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
        INT_PMSLEEP_SD_CDZ,  // [58] PAD_PM_SD_CDZ
        INT_PMSLEEP_IRIN,    // [59] PAD_PM_IRIN
        -1,                  // [60] PADA_IDAC_OUT_B
        -1,                  // [61] PADA_IDAC_OUT_G
        -1,                  // [62] PADA_IDAC_OUT_R
        INT_PMSLEEP_SPI_CZ,  // [63] PAD_PM_SPI_CZ
        INT_PMSLEEP_SPI_CK,  // [64] PAD_PM_SPI_CK
        INT_PMSLEEP_SPI_DI,  // [65] PAD_PM_SPI_DI
        INT_PMSLEEP_SPI_DO,  // [66] PAD_PM_SPI_DO
        INT_PMSLEEP_SPI_WPZ, // [67] PAD_PM_SPI_WPZ
        INT_PMSLEEP_SPI_HLD, // [68] PAD_PM_SPI_HLD
        INT_PMSLEEP_LED0,    // [69] PAD_PM_LED0
        INT_PMSLEEP_LED1,    // [70] PAD_PM_LED1
};

static int _gpi_to_irq_table[] = {
        INT_GPI_FIQ_GPIO0,
        INT_GPI_FIQ_GPIO1,
        INT_GPI_FIQ_GPIO2,
        INT_GPI_FIQ_GPIO3,
        INT_GPI_FIQ_GPIO4,
        INT_GPI_FIQ_GPIO5,
        INT_GPI_FIQ_GPIO6,
        INT_GPI_FIQ_GPIO7,
        INT_GPI_FIQ_GPIO8,
        INT_GPI_FIQ_GPIO9,
        INT_GPI_FIQ_GPIO10,
        INT_GPI_FIQ_GPIO11,
        INT_GPI_FIQ_GPIO12,
        INT_GPI_FIQ_GPIO13,
        INT_GPI_FIQ_GPIO14,
        INT_GPI_FIQ_FUART_RX,
        INT_GPI_FIQ_FUART_TX,
        INT_GPI_FIQ_FUART_CTS,
        INT_GPI_FIQ_FUART_RTS,
        INT_GPI_FIQ_TTL0,
        INT_GPI_FIQ_TTL1,
        INT_GPI_FIQ_TTL2,
        INT_GPI_FIQ_TTL3,
        INT_GPI_FIQ_TTL4,
        INT_GPI_FIQ_TTL5,
        INT_GPI_FIQ_TTL6,
        INT_GPI_FIQ_TTL7,
        INT_GPI_FIQ_TTL8,
        INT_GPI_FIQ_TTL9,
        INT_GPI_FIQ_TTL10,
        INT_GPI_FIQ_TTL11,
        INT_GPI_FIQ_TTL12,
        INT_GPI_FIQ_TTL13,
        INT_GPI_FIQ_TTL14,
        INT_GPI_FIQ_TTL15,
        INT_GPI_FIQ_TTL16,
        INT_GPI_FIQ_TTL17,
        INT_GPI_FIQ_TTL18,
        INT_GPI_FIQ_TTL19,
        INT_GPI_FIQ_TTL20,
        INT_GPI_FIQ_TTL21,
        INT_GPI_FIQ_TTL22,
        INT_GPI_FIQ_TTL23,
        INT_GPI_FIQ_TTL24,
        INT_GPI_FIQ_TTL25,
        INT_GPI_FIQ_TTL26,
        INT_GPI_FIQ_TTL27,
        INT_GPI_FIQ_UART0_RX,
        INT_GPI_FIQ_UART0_TX,
        INT_GPI_FIQ_UART1_RX,
        INT_GPI_FIQ_UART1_TX,
        INT_GPI_FIQ_SD_CLK,
        INT_GPI_FIQ_SD_CMD,
        INT_GPI_FIQ_SD_D0,
        INT_GPI_FIQ_SD_D1,
        INT_GPI_FIQ_SD_D2,
        INT_GPI_FIQ_SD_D3,
        INT_GPI_FIQ_SD_GPIO,
        -1, // [58] PAD_PM_SD_CDZ
        -1, // [59] PAD_PM_IRIN
        -1, // [60] PADA_IDAC_OUT_B
        -1, // [61] PADA_IDAC_OUT_G
        -1, // [62] PADA_IDAC_OUT_R
        -1, // [63] PAD_PM_SPI_CZ
        -1, // [64] PAD_PM_SPI_CK
        -1, // [65] PAD_PM_SPI_DI
        -1, // [66] PAD_PM_SPI_DO
        -1, // [67] PAD_PM_SPI_WPZ
        -1, // [68] PAD_PM_SPI_HLD
        -1, // [69] PAD_PM_LED0
        -1, // [70] PAD_PM_LED1
        -1, // [71] PAD_SAR_GPIO0
        -1, // [72] PAD_SAR_GPIO1
        -1, // [73] PAD_SAR_GPIO2
        -1, // [74] PAD_SAR_GPIO3
        -1, // [75] PAD_ETH_RN
        -1, // [76] PAD_ETH_RP
        -1, // [77] PAD_ETH_TN
        -1, // [78] PAD_ETH_TP
        -1, // [79] PAD_DM_P1
        -1, // [80] PAD_DP_P1
        -1, // [81] PAD_DM_P2
        -1, // [82] PAD_DP_P2
        -1, // [83] PAD_DM_P3
        -1, // [84] PAD_DP_P3
        INT_GPI_FIQ_HSYNC_OUT,
        INT_GPI_FIQ_VSYNC_OUT,
        INT_GPI_FIQ_HDMITX_SCL,
        INT_GPI_FIQ_HDMITX_SDA,
        INT_GPI_FIQ_HDMITX_HPD,
        INT_GPI_FIQ_SATA_GPIO
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

void MHal_GPIO_Set_POLARITY(U8 u8IndexGPIO, U8 reverse)
{
    // TBD
}

void MHal_GPIO_Set_Driving(U8 u8IndexGPIO, U8 setHigh)
{
    U32 r_driving[] = {0x101E7E, 0x101E7E, 0x101E7E, 0x101E7E};
    U8  m_driving[] = {BIT0,     BIT1,     BIT2,     BIT3};
    int index = u8IndexGPIO - PAD_TTL16;

    if (u8IndexGPIO < PAD_TTL16 || u8IndexGPIO > PAD_TTL19 )
    {
        printk("not support\n");
        return;
    }

    if (setHigh)
    {
        MHal_RIU_REG(r_driving[index]) |= m_driving[index];
    }
    else
    {
        MHal_RIU_REG(r_driving[index]) &= (~m_driving[index]);
    }
}

static int PMSLEEP_GPIO_To_Irq(U8 u8IndexGPIO)
{
    if ((u8IndexGPIO < PAD_PM_SD_CDZ) || (u8IndexGPIO > PAD_PM_LED1))
        return -1;
    else
        return _pmsleep_to_irq_table[u8IndexGPIO-PAD_PM_SD_CDZ];
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


    hwirq = PMSLEEP_GPIO_To_Irq(u8IndexGPIO);
    if( hwirq >= 0)
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

    hwirq = GPI_GPIO_To_Irq(u8IndexGPIO);
    if( hwirq >= 0)
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

    return virq;
}
