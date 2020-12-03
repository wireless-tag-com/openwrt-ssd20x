/*
* hal_ceva.h- Sigmastar
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
#ifndef HAL_CEVA_H
#define HAL_CEVA_H

#include "hal_ceva_reg.h"
#include <linux/kernel.h>

typedef enum
{
    CEVA_HAL_XM6_PWR_DISABLE    = 0x0,
    CEVA_HAL_XM6_PWR_ENABLE     = 0x1
} CEVA_HAL_XM6_PWR;

typedef enum
{
  CEVA_HAL_RESET_WARP_SYS           = 0x01,
  CEVA_HAL_RESET_WARP_MIU           = 0x02,
  CEVA_HAL_RESET_WARP_MCU           = 0x04,
  CEVA_HAL_RESET_WARP_ALL           = 0x07,
} CEVA_HAL_RESET_WARP;

typedef enum
{
    CEVA_HAL_IRQ_GVI                = 0x00000001, // General Violation Indication
    CEVA_HAL_IRQ_UOP                = 0x00000002, // Undefined Opcode Interrupt
    CEVA_HAL_IRQ_PI                 = 0x00000004, // Permission Interrupt
    CEVA_HAL_IRQ_MAPV               = 0x00000008, // Access Protection Violation
    CEVA_HAL_IRQ_EDP_WDOG           = 0x00000010, // EDP Watchdog Timeout
    CEVA_HAL_IRQ_IOP_WDOG           = 0x00000020, // I/O Port Watchdog Timeout
    CEVA_HAL_IRQ_EPP_WDOG           = 0x00000040, // EPP Watchdog timeout
    CEVA_HAL_IRQ_SNOOP0             = 0x00000080, // Snooping Interrupt 0
    CEVA_HAL_IRQ_SNOOP1             = 0x00000100, // Snooping Interrupt 1
    CEVA_HAL_IRQ_MCCI_MES           = 0x00000200, // Multicore Messaging Interface Interrupt
    CEVA_HAL_IRQ_DDMA_DBG_MATCH     = 0x00000400, // External Acknowledge for DDMA debug match
    CEVA_HAL_IRQ_QMAN               = 0x00008000, // QMAN Violation Indications Interrupt
    CEVA_HAL_IRQ_WARP               = 0x00001000, // WARP Accelerator Output Interrupt
    CEVA_HAL_IRQ_MCCI_RD            = 0x00002000, // Multicore Messaging Interface read indication
    CEVA_HAL_IRQ_ISP2CEVA_INT       = 0x00004000, // Interrupt Signal from MStar side = 0x0000, // such as timer interrpt
    CEVA_HAL_IRQ_INT0               = 0x00010000, // XM6 Maskable interrupt 0
    CEVA_HAL_IRQ_INT1               = 0x00020000, // XM6 Maskable interrupt 1
    CEVA_HAL_IRQ_INT2               = 0x00040000, // XM6 Maskable interrupt 2
    CEVA_HAL_IRQ_INT3               = 0x00080000, // XM6 Maskable interrupt 3
    CEVA_HAL_IRQ_INT4               = 0x00100000, // XM6 Maskable interrupt 4
    CEVA_HAL_IRQ_BP                 = 0x00200000, // XM6 Emulation software interrupt / Breakpoint interrupt
    CEVA_HAL_IRQ_NMI                = 0x00400000, // XM6 Non-maskable interrupt
    CEVA_HAL_IRQ_VINT               = 0x00800000, // XM6 Vectored interrupt
    CEVA_HAL_IRQ_TRP_SRV            = 0x01000000, // XM6 Software interrupt
    CEVA_HAL_IRQ_GPOUT_31           = 0x02000000, // XM6 General-purpose output pin 31
    CEVA_HAL_IRQ_OCM_GPOUT_3        = 0x04000000, // XM6 OCM General-purpose output pin 3
} CEVA_HAL_IRQ;

typedef enum
{
  CEVA_HAL_IRQ_TARGET_ARM       = 0,
  CEVA_HAL_IRQ_TARGET_XM6_INT0  = 1,
  CEVA_HAL_IRQ_TARGET_XM6_INT1  = 2,
  CEVA_HAL_IRQ_TARGET_XM6_INT2  = 3,
  CEVA_HAL_IRQ_TARGET_XM6_NMI   = 4,
  CEVA_HAL_IRQ_TARGET_XM6_VINT  = 5,
} CEVA_HAL_IRQ_TARGET;

typedef struct
{
    phys_addr_t base_sys;
    ceva_hal_reg_sys reg_sys;

    phys_addr_t base_axi2miu0;
    ceva_hal_reg_bus reg_axi2miu0;

    phys_addr_t base_axi2miu1;
    ceva_hal_reg_bus reg_axi2miu1;

    phys_addr_t base_axi2miu2;
    ceva_hal_reg_bus reg_axi2miu2;

    phys_addr_t base_axi2miu3;
    ceva_hal_reg_bus reg_axi2miu3;
} ceva_hal_handle;

void ceva_hal_init(ceva_hal_handle *handle, phys_addr_t base_sys, phys_addr_t base_axi2miu0, phys_addr_t base_axi2miu1, phys_addr_t base_axi2miu2, phys_addr_t base_axi2miu3);
void ceva_hal_enable_irq(ceva_hal_handle *handle, CEVA_HAL_IRQ_TARGET target, CEVA_HAL_IRQ irq);
CEVA_HAL_IRQ ceva_hal_get_irq_status(ceva_hal_handle *handle);
void ceva_hal_reset_warp(ceva_hal_handle *handle);
void ceva_hal_enable_warp(ceva_hal_handle *handle);
void ceva_hal_set_axi2miu(ceva_hal_handle *handle);

#endif // HAL_CEVA_H
