/*
* irqs.h- Sigmastar
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
/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/

#ifndef __IRQS_H
#define __IRQS_H

#include "gpi-irqs.h"
#include "pmsleep-irqs.h"

/* [GIC irqchip]
        ID   0 -  15 : SGI
        ID  16 -  31 : PPI
        ID  32 -  63 : SPI:ARM_INTERNAL
        ID  64 - 127 : SPI:MS_IRQ (GIC_HWIRQ_MS_START)
        ID 128 - 159 : SPI:MS_FIQ
   [PMSLEEP irqchip]
        ID   0 -  31 : MS_PM_IRQ    */

#define GIC_SGI_NR                 16
#define GIC_PPI_NR                 16
#define GIC_SPI_ARM_INTERNAL_NR    32
#define GIC_HWIRQ_MS_START        (GIC_SGI_NR + GIC_PPI_NR + GIC_SPI_ARM_INTERNAL_NR)


/*  The folloing list are used in dtsi and get number by of_irq,
if need to get the interrupt number for request_irq(), manual calculate the number is
GIC_SGI_NR+GIC_PPI_NR+X=32+X        */

//NOTE(Spade): We count from GIC_SPI_ARM_INTERNAL because interrupt delcaration in dts is from SPI 0
/* MS_NON_PM_IRQ 32-95 */
#define GIC_SPI_MS_IRQ_START       GIC_SPI_ARM_INTERNAL_NR
#define INT_IRQ_NONPM_TO_MCU51    (GIC_SPI_MS_IRQ_START +  0)
#define INT_IRQ_FIQ_FROM_PM       (GIC_SPI_MS_IRQ_START +  1)
#define INT_IRQ_PM_SLEEP          (GIC_SPI_MS_IRQ_START +  2)
#define INT_IRQ_SAR_GPIO_WK       (GIC_SPI_MS_IRQ_START +  3)
#define INT_IRQ_PM_UART           (GIC_SPI_MS_IRQ_START +  4)
#define INT_IRQ_FSP               (GIC_SPI_MS_IRQ_START +  5)
#define INT_IRQ_SAR1              (GIC_SPI_MS_IRQ_START +  6)
#define INT_IRQ_PIR_RX            (GIC_SPI_MS_IRQ_START +  7)
#define INT_IRQ_RTC               (GIC_SPI_MS_IRQ_START +  8)
#define INT_IRQ_DIG_TOP           (GIC_SPI_MS_IRQ_START +  9)
#define INT_IRQ_PIR_TX            (GIC_SPI_MS_IRQ_START + 10)
#define INT_IRQ_SAR_KP            (GIC_SPI_MS_IRQ_START + 11)
#define INT_IRQ_MIIC_2            (GIC_SPI_MS_IRQ_START + 12)
#define INT_IRQ_MASTER_IIC        (GIC_SPI_MS_IRQ_START + 13)
#define INT_IRQ_MMU               (GIC_SPI_MS_IRQ_START + 14)
#define INT_IRQ_OTP               (GIC_SPI_MS_IRQ_START + 15)
#define INT_IRQ_IRQ_FROM_PM       (GIC_SPI_MS_IRQ_START + 16)
#define INT_IRQ_CMDQ              (GIC_SPI_MS_IRQ_START + 17)
#define INT_IRQ_SD                (GIC_SPI_MS_IRQ_START + 18)
#define INT_IRQ_SDIO              (GIC_SPI_MS_IRQ_START + 19)
#define INT_IRQ_SC_TOP            (GIC_SPI_MS_IRQ_START + 20)
#define INT_IRQ_MHE               (GIC_SPI_MS_IRQ_START + 21)
#define INT_IRQ_PS                (GIC_SPI_MS_IRQ_START + 22)
#define INT_IRQ_WADR_ERROR        (GIC_SPI_MS_IRQ_START + 23)
#define INT_IRQ_USB3_GP2TOP       (GIC_SPI_MS_IRQ_START + 24)
#define INT_IRQ_ISP               (GIC_SPI_MS_IRQ_START + 25)
#define INT_IRQ_EMAC              (GIC_SPI_MS_IRQ_START + 26)
#define INT_IRQ_HEMCU             (GIC_SPI_MS_IRQ_START + 27)
#define INT_IRQ_MFE               (GIC_SPI_MS_IRQ_START + 28)
#define INT_IRQ_JPE               (GIC_SPI_MS_IRQ_START + 29)
#define INT_IRQ_USB               (GIC_SPI_MS_IRQ_START + 30)
#define INT_IRQ_UHC               (GIC_SPI_MS_IRQ_START + 31)
#define INT_IRQ_OTG               (GIC_SPI_MS_IRQ_START + 32)
#define INT_IRQ_MIPI_CSI2         (GIC_SPI_MS_IRQ_START + 33)
#define INT_IRQ_UART_0            (GIC_SPI_MS_IRQ_START + 34)
#define INT_IRQ_UART_1            (GIC_SPI_MS_IRQ_START + 35)
#define INT_IRQ_MIIC_0            (GIC_SPI_MS_IRQ_START + 36)
#define INT_IRQ_MIIC_1            (GIC_SPI_MS_IRQ_START + 37)
#define INT_IRQ_MSPI_0            (GIC_SPI_MS_IRQ_START + 38)
#define INT_IRQ_MSPI_1            (GIC_SPI_MS_IRQ_START + 39)
#define INT_IRQ_BDMA_0            (GIC_SPI_MS_IRQ_START + 40)
#define INT_IRQ_BDMA_1            (GIC_SPI_MS_IRQ_START + 41)
#define INT_IRQ_BACH              (GIC_SPI_MS_IRQ_START + 42)
#define INT_IRQ_LDC_FEYE          (GIC_SPI_MS_IRQ_START + 43)
#define INT_IRQ_PM_DIP            (GIC_SPI_MS_IRQ_START + 44)
#define INT_IRQ_IVE               (GIC_SPI_MS_IRQ_START + 45)
#define INT_IRQ_IMI               (GIC_SPI_MS_IRQ_START + 46)
#define INT_IRQ_FUART             (GIC_SPI_MS_IRQ_START + 47)
#define INT_IRQ_URDMA             (GIC_SPI_MS_IRQ_START + 48)
#define INT_IRQ_MIU               (GIC_SPI_MS_IRQ_START + 49)
#define INT_IRQ_GOP               (GIC_SPI_MS_IRQ_START + 50)
#define INT_IRQ_RIU_ERROR_RESP    (GIC_SPI_MS_IRQ_START + 51)
#define INT_IRQ_BOOT_TOP          (GIC_SPI_MS_IRQ_START + 52)
#define INT_IRQ_DLA_TOP           (GIC_SPI_MS_IRQ_START + 53)
#define INT_IRQ_TX_CSI            (GIC_SPI_MS_IRQ_START + 54)
#define INT_IRQ_USB3_PHY          (GIC_SPI_MS_IRQ_START + 55)
#define INT_IRQ_GPI_OUT           (GIC_SPI_MS_IRQ_START + 56)
#define INT_IRQ_VIF               (GIC_SPI_MS_IRQ_START + 57)
#define INT_IRQ_SC1_TOP_INT       (GIC_SPI_MS_IRQ_START + 58)
#define INT_IRQ_SC2_TOP_INT       (GIC_SPI_MS_IRQ_START + 59)
#define INT_IRQ_MOVEDMA           (GIC_SPI_MS_IRQ_START + 60)
#define INT_IRQ_BDMA_2            (GIC_SPI_MS_IRQ_START + 61)
#define INT_IRQ_BDMA_3            (GIC_SPI_MS_IRQ_START + 62)
#define INT_IRQ_DIP0              (GIC_SPI_MS_IRQ_START + 63)
#define GIC_SPI_MS_IRQ_END        (GIC_SPI_MS_IRQ_START + 64)
#define GIC_SPI_MS_IRQ_NR         (GIC_SPI_MS_IRQ_END - GIC_SPI_MS_IRQ_START)

/* MS_NON_PM_FIQ 96-127 */
#define GIC_SPI_MS_FIQ_START       GIC_SPI_MS_IRQ_END
#define INT_FIQ_TIMER_0           (GIC_SPI_MS_FIQ_START +  0)
#define INT_FIQ_TIMER_1           (GIC_SPI_MS_FIQ_START +  1)
#define INT_FIQ_WDT               (GIC_SPI_MS_FIQ_START +  2)
#define INT_FIQ_TIMER_2           (GIC_SPI_MS_FIQ_START +  3)
#define INT_FIQ_PIR_TOP           (GIC_SPI_MS_FIQ_START +  4)
#define INT_FIQ_PWM_TOP           (GIC_SPI_MS_FIQ_START +  5)
#define INT_FIQ_PM_XIU_TIMEOUT    (GIC_SPI_MS_FIQ_START +  6)
#define INT_FIQ_SAR_GPIO_3        (GIC_SPI_MS_FIQ_START +  7)
#define INT_FIQ_DUMMY_8           (GIC_SPI_MS_FIQ_START +  8)
#define INT_FIQ_DUMMY_9           (GIC_SPI_MS_FIQ_START +  9)
#define INT_FIQ_DUMMY_10          (GIC_SPI_MS_FIQ_START + 10)
#define INT_FIQ_DUMMY_11          (GIC_SPI_MS_FIQ_START + 11)
#define INT_FIQ_DUMMY_12          (GIC_SPI_MS_FIQ_START + 12)
#define INT_FIQ_DUMMY_13          (GIC_SPI_MS_FIQ_START + 13)
#define INT_FIQ_DUMMY_14          (GIC_SPI_MS_FIQ_START + 14)
#define INT_FIQ_DUMMY_15          (GIC_SPI_MS_FIQ_START + 15)
#define INT_FIQ_FIQ_FROM_PM       (GIC_SPI_MS_FIQ_START + 16)
#define INT_FIQ_MCU51_TO_ARM      (GIC_SPI_MS_FIQ_START + 17)
#define INT_FIQ_ARM_TO_MCU51      (GIC_SPI_MS_FIQ_START + 18)
#define INT_FIQ_DUMMY_19          (GIC_SPI_MS_FIQ_START + 19)
#define INT_FIQ_DUMMY_20          (GIC_SPI_MS_FIQ_START + 20)
#define INT_FIQ_LAN_ESD           (GIC_SPI_MS_FIQ_START + 21)
#define INT_FIQ_XIU_TIMEOUT       (GIC_SPI_MS_FIQ_START + 22)
#define INT_FIQ_SD_CDZ_0          (GIC_SPI_MS_FIQ_START + 23)
#define INT_FIQ_SD_CDZ_1          (GIC_SPI_MS_FIQ_START + 24)
#define INT_FIQ_DUMMY_25          (GIC_SPI_MS_FIQ_START + 25)
#define INT_FIQ_DUMMY_26          (GIC_SPI_MS_FIQ_START + 26)
#define INT_FIQ_DUMMY_27          (GIC_SPI_MS_FIQ_START + 27)
#define INT_FIQ_DUMMY_28          (GIC_SPI_MS_FIQ_START + 28)
#define INT_FIQ_DUMMY_29          (GIC_SPI_MS_FIQ_START + 29)
#define INT_FIQ_DUMMY_30          (GIC_SPI_MS_FIQ_START + 30)
#define INT_FIQ_DUMMY_31          (GIC_SPI_MS_FIQ_START + 31)
#define GIC_SPI_MS_FIQ_END        (GIC_SPI_MS_FIQ_START + 32)
#define GIC_SPI_MS_FIQ_NR         (GIC_SPI_MS_FIQ_END - GIC_SPI_MS_FIQ_START)

#endif // __ARCH_ARM_ASM_IRQS_H
