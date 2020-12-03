/**
 * io.h - DesignWare USB3 DRD IO Header
 *
 * Copyright (C) 2010-2011 Texas Instruments Incorporated - http://www.ti.com
 *
 * Authors: Felipe Balbi <balbi@ti.com>,
 *	    Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __DRIVERS_USB_DWC3_IO_H
#define __DRIVERS_USB_DWC3_IO_H

#include <linux/io.h>
#include "trace.h"
#include "debug.h"
#include "core.h"
#include "../drivers/sstar/include/ms_platform.h"


static inline u32 dwc3_to_sigma_rebase(u32 offset, u32 value)
{
    u32 axi_offset = offset >> 8;
    u32 bank = 0, bank_offset = 0;
    switch (axi_offset) {
        case 0xC1:
        case 0xC2:
            bank = 0x1A21;
            axi_offset = 0xC1;
	    break;
	case 0xC3:
        case 0xC4:
            axi_offset = 0xC3;
            bank = 0x1A22;
            break;
        case 0xC6:
        case 0xC7:
            axi_offset = 0xC6;
            bank = 0x1A23;
            break;
        case 0xC8:
        case 0xC9:
            axi_offset = 0xC8;
            bank = 0x1A24;
            break;
        case 0xCA:
        case 0xCB:
            axi_offset = 0xCA;
            bank = 0x1A25;
            break;
        case 0xD0:
        case 0xD1:
            axi_offset = 0xD0;
            bank = 0x1A26;
            break;
        case 0xD8:
        case 0xD9:
            axi_offset = 0xD8;
            bank = 0x1A27;
            break;
        case 0xDA:
        case 0xDB:
            axi_offset = 0xDA;
            bank = 0x1A28;
            break;
    };
    bank_offset = (offset - (axi_offset << 8));

    return (bank << 9) + 0x1F000000 + bank_offset;
}

static inline u32 dwc3_readl(void __iomem *base, u32 offset)
{
	u32 value;

	/*
	 * We requested the mem region starting from the Globals address
	 * space, see dwc3_probe in core.c.
	 * However, the offsets are given starting from xHCI address space.
	 */

	offset += ((u32)base -0xFD344200);
	value = INREG32(dwc3_to_sigma_rebase(offset, 0));
	/*
	 * When tracing we want to make it easy to find the correct address on
	 * documentation, so we revert it back to the proper addresses, the
	 * same way they are described on SNPS documentation
	 */
	dwc3_trace(trace_dwc3_readl, "addr %p value %08x",
			base - DWC3_GLOBALS_REGS_START + offset, value);

	return value;
}

static inline void dwc3_writel(void __iomem *base, u32 offset, u32 value)
{
	/*
	 * We requested the mem region starting from the Globals address
	 * space, see dwc3_probe in core.c.
	 * However, the offsets are given starting from xHCI address space.
	 */

	offset += ((u32)base -0xFD344200);
    OUTREG32(dwc3_to_sigma_rebase(offset, value), value);

	/*
	 * When tracing we want to make it easy to find the correct address on
	 * documentation, so we revert it back to the proper addresses, the
	 * same way they are described on SNPS documentation
	 */
	dwc3_trace(trace_dwc3_writel, "addr %p value %08x",
			base - DWC3_GLOBALS_REGS_START + offset, value);
}

#endif /* __DRIVERS_USB_DWC3_IO_H */
