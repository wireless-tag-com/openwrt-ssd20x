/*
* _ms_private.h- Sigmastar
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
#ifndef __MS_PRIVATE__
#define __MS_PRIVATE__


struct ms_chip
{
	void (*chip_flush_miu_pipe)(void);
	void (*chip_flush_memory)(void);
	void (*chip_read_memory)(void);
	int  (*cache_outer_is_enabled)(void);
	void (*cache_flush_all)(void);
	void (*cache_clean_range_va_pa)(unsigned long, unsigned long,unsigned long);
	void (*cache_flush_range_va_pa)(unsigned long, unsigned long,unsigned long);
	void (*cache_clean_range)(unsigned long, unsigned long);
	void (*cache_flush_range)(unsigned long, unsigned long);
	void (*cache_invalidate_range)(unsigned long, unsigned long);
	u64 (*phys_to_miu)(u64);
	u64 (*miu_to_phys)(u64);

	int   (*chip_get_device_id)(void);
	char* (*chip_get_platform_name)(void);
	int   (*chip_get_revision)(void);

	const char* (*chip_get_API_version)(void);

	int  (*chip_get_boot_dev_type)(void);
	unsigned long long  (*chip_get_riu_phys)(void);
	int  (*chip_get_riu_size)(void);
    int  (*chip_get_storage_type)(void);
    int  (*chip_get_package_type)(void);

	int (*chip_function_set)(int functionId, int param);

	u64 (*chip_get_us_ticks)(void);
};


#endif
