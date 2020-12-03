/*
* mstar-lib.c- Sigmastar
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
/*
 * Mstar USB host lib
 * Copyright (C) 2014 MStar Inc.
 * Date: AUG 2014
 */

#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <asm/io.h>
#include "ehci-mstar.h"
#include "mstar-lib.h"


static inline void mstar_efuse_set_addr(uintptr_t addr, u16 u16value)
{
	//writew(XHC_EFUSE_OFFSET,  (void*)(_MSTAR_EFUSE_BASE+0x4E*2));
	writew(u16value,  (void*)addr);
}

static inline void mstar_efuse_trig_read(uintptr_t addr, u16 u16value)
{
	writew(readw((void*)addr) | u16value,  (void*)addr);
}

void mstar_efuse_wait_complete(uintptr_t addr, u16 u16value)
{
	int i;

	for (i=0;  i<10000; i++) {
		if ((readw((void*)addr) & u16value) == 0)
			break;
		udelay(1);
	}
	if (10000==i) {
		// timeout: 10ms
		printk(" !!! WARNING: read eFuse timeout !!!\n");
		return;
	}
}

static inline u16 mstar_efuse_read_data(uintptr_t addr)
{
	return readw((void*)addr);
}

u32 mstar_efuse_read(struct mstar_efuse *efuse)
{
	unsigned long	flags;
	u16 val;
	u32 ret;
	spinlock_t	efuse_lock=__SPIN_LOCK_UNLOCKED(efuse_lock);

	spin_lock_irqsave (&efuse_lock, flags);

	//set address
	mstar_efuse_set_addr(efuse->efuse_base_addr+efuse->reg_set_addr, efuse->bank_addr);

	//trig read command
	mstar_efuse_trig_read(efuse->efuse_base_addr+efuse->reg_read, efuse->issue_read);

	//wait read complete
	mstar_efuse_wait_complete(efuse->efuse_base_addr+efuse->reg_read, efuse->issue_read);

	//read data
	val = mstar_efuse_read_data(efuse->efuse_base_addr+efuse->reg_data);
	ret = val;
	val = mstar_efuse_read_data(efuse->efuse_base_addr+efuse->reg_data+0x2*2);
	ret += ((u32)val<<16);

	spin_unlock_irqrestore (&efuse_lock, flags);

	return ret;

}

#if defined(MSTAR_EFUSE_RTERM)
u16 mstar_efuse_rterm(void)
{
	struct mstar_efuse efuse;

	efuse.efuse_base_addr = MSTAR_EFUSE_BASE;
	efuse.reg_set_addr = EFUSE_REG_ADDR;
	efuse.reg_read = EFUSE_REG_READ;
	efuse.reg_data = EFUSE_REG_DATA;
	efuse.bank_addr = RTERM_BANK;
	efuse.issue_read = EFUSE_READ_TRIG;

	return (u16)mstar_efuse_read(&efuse);
}

EXPORT_SYMBOL_GPL(mstar_efuse_read);
EXPORT_SYMBOL_GPL(mstar_efuse_rterm);
#endif
