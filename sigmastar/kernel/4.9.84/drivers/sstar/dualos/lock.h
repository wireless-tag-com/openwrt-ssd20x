/*
* lock.h- Sigmastar
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

#ifdef _LINUX_
    #ifdef CONFIG_SS_AMP
    #define DUALOS_LOCK_INIT    AMP_LOCK_INIT()
    #define DUALOS_LOCK         AMP_LOCK()
    #define DUALOS_UNLOCK       AMP_UNLOCK()
    #endif
    #ifdef CONFIG_LH_RTOS
    #include <linux/irqflags.h>
    #define DUALOS_LOCK_INIT
    #define DUALOS_LOCK         local_fiq_disable()
    #define DUALOS_UNLOCK       local_fiq_enable()
    #endif
#else //RTOS
    #ifdef __ENABLE_AMP__
    #define DUALOS_LOCK_INIT    AMP_LOCK_INIT()
    #define DUALOS_LOCK         AMP_LOCK()
    #define DUALOS_UNLOCK       AMP_UNLOCK()
    #else
    #define DUALOS_LOCK_INIT
    #define DUALOS_LOCK
    #define DUALOS_UNLOCK
    #endif
#endif
