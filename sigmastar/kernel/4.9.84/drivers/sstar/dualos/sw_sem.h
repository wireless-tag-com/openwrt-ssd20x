/*
* sw_sem.h- Sigmastar
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

/* for RTK2_DISABLE_K()/RTK2_ENABLE_K() */
//#include "rtkincl.h"

#define AMP_OS_CS_INIT()        unsigned long cpu_sr = 0
#define AMP_OS_CS_ENTER()       local_irq_save(cpu_sr)
#define AMP_OS_CS_EXIT()        local_irq_restore(cpu_sr)

#define AMP_CORE_LOCK()         intercore_sem_lock()
#define AMP_CORE_UNLOCK()       intercore_sem_unlock()

#define AMP_LOCK_INIT()         AMP_OS_CS_INIT()
	
#define AMP_LOCK()              AMP_OS_CS_ENTER();  \
					            AMP_CORE_LOCK()
					
#define AMP_UNLOCK()            AMP_CORE_UNLOCK();  \
		 			            AMP_OS_CS_EXIT()

#define CORE0       (0)
#define CORE1       (1)

typedef struct
{
	char flag[2];
	char turn;
	char nesting;
} intercore_sem_t;

void intercore_sem_init(unsigned int addr);
void intercore_sem_lock(void);
void intercore_sem_unlock(void);
