/*
* sw_sem.c- Sigmastar
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

#include "sw_sem.h"

static intercore_sem_t *intercore_sem;

void intercore_sem_init(unsigned int addr)
{
    intercore_sem = (intercore_sem_t *)(addr);
}

void intercore_sem_lock(void)
{
	volatile char *p, *q;
	intercore_sem->nesting++;
	if (intercore_sem->nesting != 1) {
		//printk("Nesting %d\n", intercore_sem->nesting);
	}
	intercore_sem->flag[CORE1] = 1;
	intercore_sem->turn = CORE0;
	q = &intercore_sem->flag[CORE0];
	p = &intercore_sem->turn;
	while (*q == 0 && *p == 1) ;
}

void intercore_sem_unlock(void)
{
	intercore_sem->nesting--;
	intercore_sem->flag[CORE1] = 0;
}
