/*
* interos_call.h- Sigmastar
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
 * syscall.h
 */
#ifndef	__DUALOS_SYSCALL_H__
#define	__DUALOS_SYSCALL_H__

#ifdef __linux__
#include <linux/semaphore.h>
#endif
#include "drv_dualos.h"

#define TARGET_BITS_CORE0    (1 << 16)
#define TARGET_BITS_CORE1    (1 << 17)
#define NSATT_BITS_GROUP0    (0 << 15)
#define NSATT_BITS_GROUP1    (1 << 15)
#define SGIINTID_BITS_08     (8)
#define SGIINTID_BITS_09     (9)
#define SGIINTID_BITS_10     (10)
#define SGIINTID_BITS_11     (11)

#define INTEROS_CALL_SHMEM_PARAM_SIZE   0x80

#define INTEROS_CALL_SHMEM_OFFSET_RX   0x800
#define INTEROS_CALL_SHMEM_OFFSET_TX   (INTEROS_CALL_SHMEM_OFFSET_RX + INTEROS_CALL_SHMEM_PARAM_SIZE)

#define CORE_RTK    0
#define CORE_LINUX  1

#define RSQ_VERSION_ID  (0x100)

typedef struct {
    unsigned int        arg0_l;
    unsigned int        arg0_h;
    unsigned int        arg1_l;
    unsigned int        arg1_h;
    unsigned int        arg2_l;
    unsigned int        arg2_h;
    unsigned int        arg3_l;
    unsigned int        arg3_h;
    unsigned int        ret_l;
    unsigned int        ret_h;
} interos_call_mbox_args_t;

typedef struct {
    unsigned int        arg0;
    unsigned int        arg1;
    unsigned int        arg2;
    unsigned int        arg3;
    unsigned int        ret;
} interos_call_args_t;

#ifdef __linux__
typedef struct {
    u32                 type;
    u32                 arg1;
    u32                 arg2;
    u32                 arg3;
    u32                 ret;
    struct semaphore    slock;
    struct semaphore    rlock;
} reroute_smc_info_t;

typedef struct {
    u32                 type;
    u32                 arg1;
    u32                 arg2;
    u32                 arg3;
    u32                 ret;
    spinlock_t          slock;
    atomic_t            rlock;
} reroute_smc_busy_wait_info_t;
#endif

u64 _getsysts(void);
void init_interos_call(void *share, int size);
void handle_reroute_smc(void);
void handle_reroute_smc_busy_wait(void);
#endif	// __DUALOS_SYSCALL_H__
