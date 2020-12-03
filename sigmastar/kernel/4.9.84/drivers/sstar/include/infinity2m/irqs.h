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
#define INT_IRQ_DUMMY_04          (GIC_SPI_MS_IRQ_START +  4)
#define INT_IRQ_FSP               (GIC_SPI_MS_IRQ_START +  5)
#define INT_IRQ_DUMMY_06          (GIC_SPI_MS_IRQ_START +  6)
#define INT_IRQ_POWER_0_NG        (GIC_SPI_MS_IRQ_START +  7)
#define INT_IRQ_POWER_1_NG        (GIC_SPI_MS_IRQ_START +  8)
#define INT_IRQ_DUMMY_09          (GIC_SPI_MS_IRQ_START +  9)
#define INT_IRQ_DUMMY_10          (GIC_SPI_MS_IRQ_START + 10)
#define INT_IRQ_DUMMY_11          (GIC_SPI_MS_IRQ_START + 11)
#define INT_IRQ_PM_ERROR_RESP     (GIC_SPI_MS_IRQ_START + 12)
#define INT_IRQ_WAKE_ON_LAN       (GIC_SPI_MS_IRQ_START + 13)
#define INT_IRQ_PWM_HOLD          (GIC_SPI_MS_IRQ_START + 14) //I2m modified
#define INT_IRQ_PWM_ROUND         (GIC_SPI_MS_IRQ_START + 15) //I2m modified
#define INT_IRQ_IRQ_FROM_PM       (GIC_SPI_MS_IRQ_START + 16)
#define INT_IRQ_CMDQ              (GIC_SPI_MS_IRQ_START + 17)
#define INT_IRQ_SATA              (GIC_SPI_MS_IRQ_START + 18) //I2m modified
#define INT_IRQ_SDIO              (GIC_SPI_MS_IRQ_START + 19)
#define INT_IRQ_DISP0             (GIC_SPI_MS_IRQ_START + 20) //I2m modified
#define INT_IRQ_DEC               (GIC_SPI_MS_IRQ_START + 21) //I2m modified
#define INT_IRQ_PS                (GIC_SPI_MS_IRQ_START + 22)
#define INT_IRQ_WADR_ERROR        (GIC_SPI_MS_IRQ_START + 23)
#define INT_IRQ_PM                (GIC_SPI_MS_IRQ_START + 24)
#define INT_IRQ_GE                (GIC_SPI_MS_IRQ_START + 25) //I2m modified
#define INT_IRQ_EMAC              (GIC_SPI_MS_IRQ_START + 26)
#define INT_IRQ_HEMCU             (GIC_SPI_MS_IRQ_START + 27)
#define INT_IRQ_DUMMY_12          (GIC_SPI_MS_IRQ_START + 28) //I2m modified
#define INT_IRQ_JPE               (GIC_SPI_MS_IRQ_START + 29)
#define INT_IRQ_USB_P3            (GIC_SPI_MS_IRQ_START + 30) //I2m modified
#define INT_IRQ_UHC_P3            (GIC_SPI_MS_IRQ_START + 31) //I2m modified
#define INT_IRQ_USB_P2            (GIC_SPI_MS_IRQ_START + 32) //I2m modified
#define INT_IRQ_UHC_P2            (GIC_SPI_MS_IRQ_START + 33) //I2m modified
#define INT_IRQ_UART_0            (GIC_SPI_MS_IRQ_START + 34)
#define INT_IRQ_UART_1            (GIC_SPI_MS_IRQ_START + 35)
#define INT_IRQ_MIIC_0            (GIC_SPI_MS_IRQ_START + 36)
#define INT_IRQ_MIIC_1            (GIC_SPI_MS_IRQ_START + 37)
#define INT_IRQ_MSPI_0            (GIC_SPI_MS_IRQ_START + 38)
#define INT_IRQ_UART_2            (GIC_SPI_MS_IRQ_START + 39) //I2m modified
#define INT_IRQ_BDMA_0            (GIC_SPI_MS_IRQ_START + 40)
#define INT_IRQ_BDMA_1            (GIC_SPI_MS_IRQ_START + 41)
#define INT_IRQ_BACH              (GIC_SPI_MS_IRQ_START + 42)
#define INT_IRQ_KEYPAD            (GIC_SPI_MS_IRQ_START + 43)
#define INT_IRQ_RTC               (GIC_SPI_MS_IRQ_START + 44)
#define INT_IRQ_SAR               (GIC_SPI_MS_IRQ_START + 45)
#define INT_IRQ_IMI               (GIC_SPI_MS_IRQ_START + 46)
#define INT_IRQ_FUART             (GIC_SPI_MS_IRQ_START + 47)
#define INT_IRQ_URDMA             (GIC_SPI_MS_IRQ_START + 48)
#define INT_IRQ_MIU               (GIC_SPI_MS_IRQ_START + 49)
#define INT_IRQ_DISP_1            (GIC_SPI_MS_IRQ_START + 50) //I2m modified
#define INT_IRQ_RIU_ERROR_RESP    (GIC_SPI_MS_IRQ_START + 51)
#define INT_IRQ_EMAC_1            (GIC_SPI_MS_IRQ_START + 52) //I2m modified
#define INT_IRQ_MMU               (GIC_SPI_MS_IRQ_START + 53) //I2m new
#define INT_IRQ_USB_INT_P1        (GIC_SPI_MS_IRQ_START + 54)
#define INT_IRQ_UHC_INT_P1        (GIC_SPI_MS_IRQ_START + 55)
#define INT_IRQ_GPI               (GIC_SPI_MS_IRQ_START + 56)
#define INT_IRQ_MIPI_TX_DSI       (GIC_SPI_MS_IRQ_START + 57) //I2m modified
#define INT_IRQ_DISP_2            (GIC_SPI_MS_IRQ_START + 58) //I2m modified
#define INT_IRQ_DISP_3            (GIC_SPI_MS_IRQ_START + 59) //I2m modified
#define INT_IRQ_MOVEDMA           (GIC_SPI_MS_IRQ_START + 60)  //I6 new
#define INT_IRQ_BDMA_2            (GIC_SPI_MS_IRQ_START + 61)  //I6 new
#define INT_IRQ_BDMA_3            (GIC_SPI_MS_IRQ_START + 62)  //I6 new
#define INT_IRQ_DIP0              (GIC_SPI_MS_IRQ_START + 63)  //I6 new
#define GIC_SPI_MS_IRQ_END        (GIC_SPI_MS_IRQ_START + 64)
#define GIC_SPI_MS_IRQ_NR         (GIC_SPI_MS_IRQ_END - GIC_SPI_MS_IRQ_START)

/* MS_NON_PM_FIQ 96-127 */
#define GIC_SPI_MS_FIQ_START       GIC_SPI_MS_IRQ_END
#define INT_FIQ_TIMER_0           (GIC_SPI_MS_FIQ_START +  0)
#define INT_FIQ_TIMER_1           (GIC_SPI_MS_FIQ_START +  1)
#define INT_FIQ_WDT               (GIC_SPI_MS_FIQ_START +  2)
#define INT_FIQ_IR                (GIC_SPI_MS_FIQ_START +  3)
#define INT_FIQ_IR_RC             (GIC_SPI_MS_FIQ_START +  4)
#define INT_FIQ_POWER_0_NG        (GIC_SPI_MS_FIQ_START +  5)
#define INT_FIQ_POWER_1_NG        (GIC_SPI_MS_FIQ_START +  6)
#define INT_FIQ_POWER_2_NG        (GIC_SPI_MS_FIQ_START +  7)
#define INT_FIQ_PM_XIU_TIMEOUT    (GIC_SPI_MS_FIQ_START +  8)
#define INT_FIQ_DUMMY_09          (GIC_SPI_MS_FIQ_START +  9)
#define INT_FIQ_DUMMY_10          (GIC_SPI_MS_FIQ_START + 10)
#define INT_FIQ_DUMMY_11          (GIC_SPI_MS_FIQ_START + 11)
#define INT_FIQ_TIMER_2           (GIC_SPI_MS_FIQ_START + 12)
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
#define INT_FIQ_SD_CDZ            (GIC_SPI_MS_FIQ_START + 23)
#define INT_FIQ_SAR_GPIO_0        (GIC_SPI_MS_FIQ_START + 24)
#define INT_FIQ_SAR_GPIO_1        (GIC_SPI_MS_FIQ_START + 25)
#define INT_FIQ_SAR_GPIO_2        (GIC_SPI_MS_FIQ_START + 26)
#define INT_FIQ_SAR_GPIO_3        (GIC_SPI_MS_FIQ_START + 27)
#define INT_FIQ_SPI0_GPIO_0       (GIC_SPI_MS_FIQ_START + 28)
#define INT_FIQ_SPI0_GPIO_1       (GIC_SPI_MS_FIQ_START + 29)
#define INT_FIQ_SPI0_GPIO_2       (GIC_SPI_MS_FIQ_START + 30)
#define INT_FIQ_SPI0_GPIO_3       (GIC_SPI_MS_FIQ_START + 31)
#define GIC_SPI_MS_FIQ_END        (GIC_SPI_MS_FIQ_START + 32)
#define GIC_SPI_MS_FIQ_NR         (GIC_SPI_MS_FIQ_END - GIC_SPI_MS_FIQ_START)



/*  Not used in dtsi,
if need to get the interrupt number for request_irq(), use gpio_to_irq() to obtain irq number.
Or manual calculate the number is
GIC_SGI_NR+GIC_PPI_NR+GIC_SPI_ARM_INTERNAL_NR+GIC_SPI_MS_IRQ_NR+GIC_SPI_MS_FIQ_NR+X=160+X   */
/* MS_PM_SLEEP_FIQ 0-31 */
#define PMSLEEP_FIQ_START         0
#define INT_PMSLEEP_DUMMY_0       (PMSLEEP_FIQ_START +  0)
#define INT_PMSLEEP_DUMMY_1       (PMSLEEP_FIQ_START +  1)
#define INT_PMSLEEP_DUMMY_2       (PMSLEEP_FIQ_START +  2)
#define INT_PMSLEEP_DUMMY_3       (PMSLEEP_FIQ_START +  3)
#define INT_PMSLEEP_DUMMY_4       (PMSLEEP_FIQ_START +  4)
#define INT_PMSLEEP_DUMMY_5       (PMSLEEP_FIQ_START +  5)
#define INT_PMSLEEP_DUMMY_6       (PMSLEEP_FIQ_START +  6)
#define INT_PMSLEEP_DUMMY_7       (PMSLEEP_FIQ_START +  7)
#define INT_PMSLEEP_DUMMY_8       (PMSLEEP_FIQ_START +  8)
#define INT_PMSLEEP_DUMMY_9       (PMSLEEP_FIQ_START +  9)
#define INT_PMSLEEP_DUMMY_10      (PMSLEEP_FIQ_START + 10)
#define INT_PMSLEEP_DUMMY_11      (PMSLEEP_FIQ_START + 11)
#define INT_PMSLEEP_DUMMY_12      (PMSLEEP_FIQ_START + 12)
#define INT_PMSLEEP_DUMMY_13      (PMSLEEP_FIQ_START + 13)
#define INT_PMSLEEP_DUMMY_14      (PMSLEEP_FIQ_START + 14)
#define INT_PMSLEEP_DUMMY_15      (PMSLEEP_FIQ_START + 15)
#define INT_PMSLEEP_DUMMY_16      (PMSLEEP_FIQ_START + 16)
#define INT_PMSLEEP_DUMMY_17      (PMSLEEP_FIQ_START + 17)
#define INT_PMSLEEP_DUMMY_18      (PMSLEEP_FIQ_START + 18)
#define INT_PMSLEEP_DUMMY_19      (PMSLEEP_FIQ_START + 19)
#define INT_PMSLEEP_IRIN          (PMSLEEP_FIQ_START + 20)
#define INT_PMSLEEP_UART_RX       (PMSLEEP_FIQ_START + 21)
#define INT_PMSLEEP_DUMMY_22      (PMSLEEP_FIQ_START + 22)
#define INT_PMSLEEP_DUMMY_23      (PMSLEEP_FIQ_START + 23)
#define INT_PMSLEEP_SPI_CZ        (PMSLEEP_FIQ_START + 24)
#define INT_PMSLEEP_SPI_CK        (PMSLEEP_FIQ_START + 25)
#define INT_PMSLEEP_SPI_DI        (PMSLEEP_FIQ_START + 26)
#define INT_PMSLEEP_SPI_DO        (PMSLEEP_FIQ_START + 27)
#define INT_PMSLEEP_DUMMY_28      (PMSLEEP_FIQ_START + 28)
#define INT_PMSLEEP_DUMMY_29      (PMSLEEP_FIQ_START + 29)
#define INT_PMSLEEP_DUMMY_30      (PMSLEEP_FIQ_START + 30)
#define INT_PMSLEEP_DUMMY_31      (PMSLEEP_FIQ_START + 31)
#define INT_PMSLEEP_DUMMY_32      (PMSLEEP_FIQ_START + 32)
#define INT_PMSLEEP_DUMMY_33      (PMSLEEP_FIQ_START + 33)
#define INT_PMSLEEP_DUMMY_34      (PMSLEEP_FIQ_START + 34)
#define INT_PMSLEEP_DUMMY_35      (PMSLEEP_FIQ_START + 35)
#define INT_PMSLEEP_DUMMY_36      (PMSLEEP_FIQ_START + 36)
#define INT_PMSLEEP_DUMMY_37      (PMSLEEP_FIQ_START + 37)
#define INT_PMSLEEP_DUMMY_38      (PMSLEEP_FIQ_START + 38)
#define INT_PMSLEEP_DUMMY_39      (PMSLEEP_FIQ_START + 39)
#define INT_PMSLEEP_DUMMY_40      (PMSLEEP_FIQ_START + 40)
#define INT_PMSLEEP_DUMMY_41      (PMSLEEP_FIQ_START + 41)
#define INT_PMSLEEP_DUMMY_42      (PMSLEEP_FIQ_START + 42)
#define INT_PMSLEEP_DUMMY_43      (PMSLEEP_FIQ_START + 43)
#define INT_PMSLEEP_DUMMY_44      (PMSLEEP_FIQ_START + 44)
#define INT_PMSLEEP_DUMMY_45      (PMSLEEP_FIQ_START + 45)
#define INT_PMSLEEP_DUMMY_46      (PMSLEEP_FIQ_START + 46)
#define INT_PMSLEEP_DUMMY_47      (PMSLEEP_FIQ_START + 47)
#define INT_PMSLEEP_DUMMY_48      (PMSLEEP_FIQ_START + 48)
#define INT_PMSLEEP_DUMMY_49      (PMSLEEP_FIQ_START + 49)
#define INT_PMSLEEP_DUMMY_50      (PMSLEEP_FIQ_START + 50)
#define INT_PMSLEEP_DUMMY_51      (PMSLEEP_FIQ_START + 51)
#define INT_PMSLEEP_DUMMY_52      (PMSLEEP_FIQ_START + 52)
#define INT_PMSLEEP_DUMMY_53      (PMSLEEP_FIQ_START + 53)
#define INT_PMSLEEP_DUMMY_54      (PMSLEEP_FIQ_START + 54)
#define INT_PMSLEEP_DUMMY_55      (PMSLEEP_FIQ_START + 55)
#define INT_PMSLEEP_DUMMY_56      (PMSLEEP_FIQ_START + 56)
#define INT_PMSLEEP_DUMMY_57      (PMSLEEP_FIQ_START + 57)
#define INT_PMSLEEP_DUMMY_58      (PMSLEEP_FIQ_START + 58)
#define INT_PMSLEEP_DUMMY_59      (PMSLEEP_FIQ_START + 59)
#define INT_PMSLEEP_DUMMY_60      (PMSLEEP_FIQ_START + 60)
#define INT_PMSLEEP_DUMMY_61      (PMSLEEP_FIQ_START + 61)
#define INT_PMSLEEP_DUMMY_62      (PMSLEEP_FIQ_START + 62)
#define INT_PMSLEEP_DUMMY_63      (PMSLEEP_FIQ_START + 63)
#define INT_PMSLEEP_DUMMY_64      (PMSLEEP_FIQ_START + 64)
#define INT_PMSLEEP_DUMMY_65      (PMSLEEP_FIQ_START + 65)
#define INT_PMSLEEP_DUMMY_66      (PMSLEEP_FIQ_START + 66)
#define INT_PMSLEEP_DUMMY_67      (PMSLEEP_FIQ_START + 67)
#define INT_PMSLEEP_SPI_WPZ       (PMSLEEP_FIQ_START + 68)
#define INT_PMSLEEP_SPI_HLD       (PMSLEEP_FIQ_START + 69)
#define INT_PMSLEEP_DUMMY_70      (PMSLEEP_FIQ_START + 70)
#define INT_PMSLEEP_SD_CDZ        (PMSLEEP_FIQ_START + 71)
#define INT_PMSLEEP_DUMMY_72      (PMSLEEP_FIQ_START + 72)
#define INT_PMSLEEP_DUMMY_73      (PMSLEEP_FIQ_START + 73)
#define INT_PMSLEEP_LED0          (PMSLEEP_FIQ_START + 74)
#define INT_PMSLEEP_LED1          (PMSLEEP_FIQ_START + 75)
#define PMSLEEP_FIQ_END           (PMSLEEP_FIQ_START + 76)
#define PMSLEEP_FIQ_NR            (PMSLEEP_FIQ_END - PMSLEEP_FIQ_START)

#define PMSLEEP_IRQ_START           PMSLEEP_FIQ_END
#define INT_PMSLEEP_IRQ_DUMMY_00    (PMSLEEP_IRQ_START +  0)
#define INT_PMSLEEP_IRQ_SAR         (PMSLEEP_IRQ_START +  1)
#define INT_PMSLEEP_IRQ_WOL         (PMSLEEP_IRQ_START +  2)
#define INT_PMSLEEP_IRQ_DUMMY_03    (PMSLEEP_IRQ_START +  3)
#define INT_PMSLEEP_IRQ_RTC         (PMSLEEP_IRQ_START +  4)
#define INT_PMSLEEP_IRQ_DUMMY_05    (PMSLEEP_IRQ_START +  5)
#define INT_PMSLEEP_IRQ_SAR_GPIO    (PMSLEEP_IRQ_START +  6)
#define INT_PMSLEEP_IRQ_DUMMY_07    (PMSLEEP_IRQ_START +  7)
#define PMSLEEP_IRQ_END             (PMSLEEP_IRQ_START +  8)
#define PMSLEEP_IRQ_NR              (PMSLEEP_IRQ_END - PMSLEEP_IRQ_START)
#endif // __ARCH_ARM_ASM_IRQS_H
