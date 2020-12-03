/*
* drv_dualos.h- Sigmastar
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

#ifndef __DRV_DUALOS_H__
#define __DRV_DUALOS_H__

#include "cam_os_wrapper.h"

#define INTEROS_SC_L2R_HANDSHAKE        (0x800)
#define INTEROS_SC_L2R_RTK_CLI          (0x801)
#define INTEROS_SC_L2R_RTK_LOG          (0x802)
#define INTEROS_SC_L2R_SCR_FW_ENABLE    (0x803)
#define INTEROS_SC_L2R_SCR_FW_DISABLE   (0x804)
#define INTEROS_SC_L2R_RSQ_INIT         (0x100)
#define INTEROS_SC_L2R_CALL             (0x808)
#define INTEROS_SC_R2L_MI_NOTIFY        (0x809)


#define IPI_NR_RTOS_2_LINUX_CALL_REQ    8
#define IPI_NR_RTOS_2_LINUX_CALL_RESP   9
#define IPI_NR_LINUX_2_RTOS_CALL_REQ    10
#define IPI_NR_LINUX_2_RTOS_CALL_RESP   11
#define IPI_NR_REROUTE_SMC              14
#define IPI_NR_REROUTE_SMC_BUSY_WAIT    15

#define	RTKINFO_FIQCNT(c)	if (_rtk) _rtk->fiq_cnt++
#define	RTKINFO_FFIQTS()	_rtk->ffiq_ts = (unsigned int)(_getsysts() - epiod);
#define	RTKINFO_LOADNSTS()	_rtk->ldns_ts = (unsigned int)(_getsysts() - epiod);
#define	RTKINFO_TTFF_ISP()	if (!_rtk->ttff_isp) _rtk->ttff_isp = (unsigned int)(_getsysts() - epiod);
#define	RTKINFO_TTFF_SCL()	if (!_rtk->ttff_scl) _rtk->ttff_scl = (unsigned int)(_getsysts() - epiod);
#define	RTKINFO_TTFF_MFE()	if (!_rtk->ttff_mfe) _rtk->ttff_mfe = (unsigned int)(_getsysts() - epiod);
#define RTK_TIME_TO_US(x)   (x / 6)

struct rlink_head {
	struct rlink_head *next, *prev;
	void*           nphys;  // next object physical address
	unsigned int	nsize;	// next object size
	unsigned int	reserved;
};

typedef struct {
    struct rlink_head   root;
    char                name[8];
    char                version[64];
    unsigned int        verid;
    unsigned int        size;
    unsigned int        fiq_cnt;
    unsigned int        ffiq_ts;
    unsigned int        ttff_isp;
    unsigned int        ttff_scl;
    unsigned int        ttff_mfe;
    unsigned int        ldns_ts;
    u64                 start_ts;
    u64	                lifet;
    u64	                spent;
    u64                 spent_hyp;
    u64                 spent_sc;
    u64                 linux_idle_in_rtos_time;
    unsigned int        diff;
    unsigned int        linux_idle;
    u64                 syscall_cnt;

    // sbox must be 16-byte aligned
    unsigned char       sbox[1024];
} rtkinfo_t;

extern rtkinfo_t	*_rtk;
extern u64	epiod;

rtkinfo_t* get_rtkinfo(void);
unsigned long signal_rtos(u32 type, u32 arg1, u32 arg2, u32 arg3);
unsigned long signal_rtos_busy_wait(u32 type, u32 arg1, u32 arg2, u32 arg3);

#endif //__DRV_DUALOS_H__
