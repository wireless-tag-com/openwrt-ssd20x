/*
* drv_dualos.c- Sigmastar
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
 * drv_dualos.c
 *   ipc rsq with LH/RTOS
 */
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <linux/module.h>
#include <asm/compiler.h>
#include <asm/io.h>
#include "linux/arm-smccc.h"
#include "ms_platform.h"
#include "registers.h"
#include "drv_dualos.h"
#include "interos_call.h"
#include "rsq.h"
#include "sw_sem.h"
#include "lock.h"

/* proc */
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <linux/list.h>

static int disable_rtos = 0;
module_param(disable_rtos, int, 0644);
MODULE_PARM_DESC(disable_rtos, "Disable RTOS IPC");

static struct file *file_open(const char *path, int flags, int rights);
static void file_close(struct file *file);
static int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size);
static int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size);
static int file_sync(struct file *file);
/* */
struct sstream {
	reservoir_t		*rvr;
	struct resource	*res;
	unsigned int	id;
	void			*private;
};

#define	SHARE_SIZE		(0x1000)
static rtkinfo_t 		*_rtklin;
static struct sstream	_sstr[8] = { { 0 } };

static struct resource	*_rtkres;
unsigned int			_ipi[8] = { 0 };
#ifdef CONFIG_SS_AMP
struct semaphore		_interos_call_mbox;
struct semaphore		_interos_call_resp;
struct semaphore		_interos_call_req;
static int				_interos_call_pid;
static int              _signal_1st_using_mbox = 1;
#endif

struct rsqcb {
	struct list_head	list;
	int	(*rsqproc)(void *param, void *buf, int size, slot_t* slot);
	void			*private;
};

LIST_HEAD(_rsqcblst);
struct rsqcb	rec_rsq;
struct rsqrec {
	struct rsqcb	cb;
	char			file[32];
	reservoir_t		*rvr;
	unsigned int	frms;
	struct file		*fp;
	long long		off;
};

#ifdef CONFIG_SMP
static reroute_smc_info_t _reroute_smc_info;
static reroute_smc_busy_wait_info_t _reroute_smc_busy_wait_info;
#endif

static int c_logshow(struct seq_file *m, void *v)
{
	reservoir_t		*rv;
	unsigned int	rs;
	char			msg[256];
	slot_t			slot;

	rv = m->private;
	if (!rv) {
		seq_printf(m, "not available\n");
		return 0;
	}
/* for debug
	seq_printf(m, "Reservoir - %s(%p) size %dK\n", rv->name, rv, rv->size >> 10);
	seq_printf(m, "\tCurrent available frame NO.\t: %d~%d\n", rv->ns.rdno, rv->slotno);
	seq_printf(m, "\treset id                   \t: %d~%d\n", rv->ss.reset_id, rv->ns.reset_id);
	seq_printf(m, "\tDrop Count/Frms\t: %u/%u\n", rv->dropcnt, rv->dropfrms);
	seq_printf(m, "\tReservoir Reset\t: %u\n", rv->resetcnt);
	seq_printf(m, "\tHot sync\t: %u\n", rv->synccnt);
*/
	do {
        DUALOS_LOCK_INIT;
        DUALOS_LOCK;
		rs = rcvr_de_rsqslot(rv, msg, sizeof(msg), (slot_t*)&slot);
        DUALOS_UNLOCK;
		if (!rs) break;
		seq_printf(m, msg);
	} while (m->size >= (m->count + sizeof(msg)));

	seq_printf(m, "\n");

	return 0;
}

static void *c_start(struct seq_file *m, loff_t *pos)
{
	return *pos < 1 ? (void *)1 : NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

static void c_stop(struct seq_file *m, void *v)
{
}

const struct seq_operations rsqlog_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= c_logshow
};

static int rsqinfo_open(struct inode *inode, struct file *file)
{
	int					res = -ENOMEM;
	int					i;
	struct seq_file		*seq;

    res = seq_open(file, NULL);
	if (res)
		return res;
	seq = file->private_data;
	for (i = 0; i < sizeof(_sstr) / sizeof(struct sstream); i++) {
		if (!_sstr[i].rvr) continue;
		if (!strncmp(file->f_path.dentry->d_iname,
					 _sstr[i].rvr->name, sizeof(_sstr[i].rvr->name))) {
			seq->private = _sstr[i].rvr;
			break;
		}
	}
	if (i == (sizeof(_sstr) / sizeof(struct sstream)))
		return -ENOMEM;
	seq->op = _sstr[i].private;
	return res;
}

static ssize_t rsqinfo_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	char	code[32];
	reservoir_t	*rv;

	rv = (reservoir_t*)((struct seq_file *)file->private_data)->private;
	if (!rv) return len;
	if (copy_from_user(code, buf, 32))
		return -EFAULT;
	if (strncmp(rv->name, "log", 3) == 0) {
		signal_rtos(INTEROS_SC_L2R_RTK_LOG, 0, 0, 0);
	}

	return len;
}

#define	TTM(s) (((s) + 3000) / 6000)
static u64	_spent = 0, _lifet = 0;
static u64 _spent_hyp = 0, _spent_sc = 0;
static int c_show_rtk(struct seq_file *m, void *v)
{
	rtkinfo_t		*rtk;
	int		i;
	int		s;
	u64		cs, cl;
    int     sh, sc;
	u64     ch, cc;

	rtk = m->private;
	if (!rtk) {
		seq_printf(m, "not avaliable\n");
		return 0;
	}
	/* reset cpu usage after present */
	cs = rtk->spent;
	cl = rtk->lifet;
    ch = rtk->spent_hyp;
    cc = rtk->spent_sc;

	s = (int)div64_u64((cs - _spent) * 1000, (cl - _lifet));
	if (ch && cc)   // It means __ADV_HYP_PROF__ enabled in rtos
	{
	    sh = (int)div64_u64((ch - _spent_hyp) * 1000, (cl - _lifet));
	    sc = (int)div64_u64((cc - _spent_sc) * 1000, (cl - _lifet));
	}

	seq_printf(m, "RTOS: %s\n", rtk->version);

	if (ch && cc)   // It means __ADV_HYP_PROF__ enabled in rtos
	{
	    seq_printf(m, "\tcpu usage(hyp/sc/rtos): %u.%u%% (%llu/%llu)\n", s / 10, s % 10, cs - _spent, cl - _lifet);
	    seq_printf(m, "\tcpu usage(hyp): %u.%u%% (%llu/%llu)\n", sh / 10, sh % 10, ch - _spent_hyp, cl - _lifet);
	    seq_printf(m, "\tcpu usage(sc): %u.%u%% (%llu/%llu)\n", sc / 10, sc % 10, cc - _spent_sc, cl - _lifet);
	}
	else
	{
	    seq_printf(m, "\tcpu usage: %u.%u%% (%llu/%llu)\n", s / 10, s % 10, cs - _spent, cl - _lifet);
	}
	seq_printf(m, "\tttff(isp): %u ms\n", TTM(rtk->ttff_isp));
	seq_printf(m, "\tttff(scl): %u ms\n", TTM(rtk->ttff_scl));
	seq_printf(m, "\tttff(mfe): %u ms\n", TTM(rtk->ttff_mfe));
	seq_printf(m, "\tload ns  : %u ms\n", TTM(rtk->ldns_ts));
	seq_printf(m, "\tfiq count: %u\n", rtk->fiq_cnt);
	seq_printf(m, "\tsyscall  : %llu\n", rtk->syscall_cnt);
	for (i = 0; i < 8; i++)
		if (_ipi[i]) seq_printf(m, "\tipi%02d count: %u\n", i + 8, _ipi[i]);
	_spent = cs; _lifet = cl;
	if (ch && cc)   // It means __ADV_HYP_PROF__ enabled in rtk
	{
	    _spent_hyp = ch;
	    _spent_sc = cc;
	}

	return 0;
}

static ssize_t rtkinfo_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	char		code[128];
	rtkinfo_t	*rtk;

	rtk = (rtkinfo_t*)((struct seq_file *)file->private_data)->private;
	if (!rtk) return len;
	if (copy_from_user(code, buf, sizeof(code)))
		return -EFAULT;
	// rtkinfo simple command parser
	//		cli -
	if (!strncmp(code, "cli", 3)) {
		char	*ptr;
		// strip return and space in tail and head.
		ptr = strrchr(code, 0x0a);
		if (ptr) *ptr = '\0';
		for (ptr = code + 4; *ptr == ' ' || *ptr == '\t'; ptr++);
		// copy command to share buffer and send to S
		strncpy(rtk->sbox, ptr, 127);
		signal_rtos(INTEROS_SC_L2R_RTK_CLI, (u32)rtk, (u32)rtk->diff, 0);
	} else if (!strncmp(code, "reset", 4)) {
	}
	return len;
}

const struct seq_operations rtkinfo_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= c_show_rtk
};

static int rtkinfo_open(struct inode *inode, struct file *file)
{
	int					res = -ENOMEM;
	struct seq_file		*seq;

    res = seq_open(file, &rtkinfo_op);
	if (res)
		return res;
	seq = file->private_data;
	seq->private = (void*)_rtklin;
    return res;
}

static const struct file_operations proc_rsqinfo_operations = {
    .open       = rsqinfo_open,
    .read       = seq_read,
	.write		= rsqinfo_write,
    .llseek     = seq_lseek,
    .release    = seq_release,
};

static const struct file_operations proc_rtkinfo_operations = {
    .open       = rtkinfo_open,
    .read       = seq_read,
	.write		= rtkinfo_write,
    .llseek     = seq_lseek,
    .release    = seq_release,
};

static bool alkaid_notify_nop(int no){
    return false;
}
static bool (*alkaid_notify[1])(int no) = {alkaid_notify_nop};
void alkaid_registe_notify(int cpu, void *notify){
    alkaid_notify[cpu] = notify;
}
EXPORT_SYMBOL(alkaid_registe_notify);
void *alkaid_unregiste_notify(int cpu){
    void *notify = alkaid_notify[cpu];
    alkaid_notify[cpu] = alkaid_notify_nop;
    return notify;
}
EXPORT_SYMBOL(alkaid_unregiste_notify);

/* call from gic_handle_irq (gic-irq.c) */
void handle_rsq(unsigned int irqnr)
{
    if (alkaid_notify[0](irqnr))
    {
        return;
    }
	_ipi[irqnr - 8]++;
}

#ifdef CONFIG_SS_AMP
unsigned long do_interos_call(u32 type, u32 arg1, u32 arg2, u32 arg3)
{
    u32 ret = 0;

    switch (type) {
        case INTEROS_SC_R2L_MI_NOTIFY:
            if (alkaid_notify[0](arg1))
            {
                ret = 0;
            }
            break;
        default:
            break;
    }

    return ret;
}

void handle_interos_call_req(void)
{
    up(&_interos_call_req);
}

void handle_interos_call_resp(void)
{
    up(&_interos_call_resp);
}

static int inter_os_call_receiver(void *unused)
{
	struct task_struct	*tsk = current;
	u32 arg0, arg1, arg2, arg3, ret;
    int signal_1st_using_mbox;

    interos_call_mbox_args_t *ptr_mbox_args;
    interos_call_args_t *ptr_args;
    struct arm_smccc_res res;

	/* Setup a clean context for our children to inherit. */
	set_task_comm(tsk, "inter_os_call_receiver");
	sema_init(&_interos_call_req, 0);
	while (1) {
		down(&_interos_call_req);
        signal_1st_using_mbox = _signal_1st_using_mbox;

        if (signal_1st_using_mbox)
        {
            ptr_mbox_args = (interos_call_mbox_args_t *)(BASE_REG_MAILBOX_PA+BK_REG(0x50)+IO_OFFSET);
            arg0 = (ptr_mbox_args->arg0_h << 16) + ptr_mbox_args->arg0_l;
            arg1 = (ptr_mbox_args->arg1_h << 16) + ptr_mbox_args->arg1_l;
            arg2 = (ptr_mbox_args->arg2_h << 16) + ptr_mbox_args->arg2_l;
            arg3 = (ptr_mbox_args->arg3_h << 16) + ptr_mbox_args->arg3_l;
        }
        else
        {
            ptr_args = (interos_call_args_t *)((u32)_rtklin + INTEROS_CALL_SHMEM_OFFSET_RX);
            arg0 = ptr_args->arg0;
            arg1 = ptr_args->arg1;
            arg2 = ptr_args->arg2;
            arg3 = ptr_args->arg3;
        }

        /* process here */
        ret = do_interos_call(arg0, arg1, arg2, arg3);
        if (signal_1st_using_mbox)
        {
            ptr_mbox_args->ret_l = ret & 0xFFFF;
            ptr_mbox_args->ret_h = ret >> 16;
        }
        else
        {
            ptr_args->ret = ret;
        }
        arm_smccc_smc(INTEROS_SC_L2R_CALL, TARGET_BITS_CORE0, NSATT_BITS_GROUP0, SGIINTID_BITS_09, 0, 0, 0, 0, &res);
	}
	return 0;
}
#endif

static int __init epoch_init(void)
{
	char				proc[32];
	long				share;
	struct rlink_head	*rl;
#ifdef CONFIG_SMP
	int cpu = smp_processor_id();
#endif

	if (disable_rtos)
    {
		return 0;
    }

#ifdef CONFIG_SMP
    if (cpu != 0)
        return 0;

    sema_init(&(_reroute_smc_info.slock), 1);
    sema_init(&(_reroute_smc_info.rlock), 0);
    spin_lock_init(&(_reroute_smc_busy_wait_info.slock));
    atomic_set(&(_reroute_smc_busy_wait_info.rlock), 0);
#endif

    proc_mkdir("dualos", NULL);

#ifdef CONFIG_SS_AMP
	sema_init(&_interos_call_mbox, 1);
	sema_init(&_interos_call_resp, 0);
#endif

	/* get RTOS info */
	share  = (long)signal_rtos(INTEROS_SC_L2R_HANDSHAKE, (u32)0, (u32)0, (u32)0);
#ifdef CONFIG_SS_AMP
    _signal_1st_using_mbox = 0;
#endif
	_rtkres = request_mem_region(share, SHARE_SIZE, "dualos");
	_rtklin = (rtkinfo_t*)ioremap(_rtkres->start, resource_size(_rtkres));

#ifdef CONFIG_SS_AMP
    intercore_sem_init((u32)_rtklin + SHARE_SIZE - sizeof(intercore_sem_t));
#endif

        if (INTEROS_CALL_SHMEM_PARAM_SIZE < sizeof(interos_call_args_t)) {
            printk(KERN_ERR "Error!! interos shmem param address\n");
            *(int *)0 = 0;
        }

	if (!_rtklin							||
		_rtklin->size  != sizeof(*_rtklin)	||
		_rtklin->verid != RSQ_VERSION_ID) {
		proc_create("dualos/version_not_match", 0, NULL, &proc_rtkinfo_operations);
		printk(KERN_ERR "Error!! RTOS version not match\n");
		_rtklin = NULL;
		return 0;
	}
	snprintf(proc, sizeof(proc), "dualos/%s", _rtklin->name);
    proc_create(proc, 0, NULL, &proc_rtkinfo_operations);
	/* */
	for (rl = &(_rtklin->root); rl->nphys && rl->nsize;) {
		struct sstream		sst;

		sst.res = request_mem_region((long)rl->nphys, rl->nsize, "rsq");
		sst.rvr = (reservoir_t*)ioremap(sst.res->start, resource_size(sst.res));
		init_rsq_rcvr(sst.rvr);
		snprintf(proc, sizeof(proc), "dualos/%s", sst.rvr->name);
		proc_create(proc, 0, NULL, &proc_rsqinfo_operations);
		sst.id = sst.rvr->iid;
		if (strncmp(sst.rvr->name, "log", 3) == 0)
			sst.private = (void*)&rsqlog_op;  // set log read op
		_sstr[sst.id] = sst;
		rl = &(sst.rvr->link);
	}
	_rtklin->diff = (unsigned int)share - (unsigned int)_rtklin;	// offset for address transfer

#ifdef CONFIG_SS_AMP
	/* create a thread to get interos call */
    _interos_call_pid = kernel_thread(inter_os_call_receiver, NULL, CLONE_FS | CLONE_FILES);
#endif
	/* for debug */
	signal_rtos(INTEROS_SC_L2R_RSQ_INIT, (u32)_rtklin, _rtklin->diff, 0);
	return 0;
}

/*
 * kerenl file I/O
 */
struct file *file_open(const char *path, int flags, int rights)
{
    struct file *filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

void file_close(struct file *file)
{
    filp_close(file, NULL);
}

int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

int file_sync(struct file *file)
{
    vfs_fsync(file, 0);
    return 0;
}

rtkinfo_t* get_rtkinfo(void)
{
    return _rtklin;
}

#define GICD_SGIR			0x0F00
#define GICD_BASE           0xF4001000
#define GICD_WRITEL(a,v)    (*(volatile unsigned int *)(u32)(GICD_BASE + a) = (v))

static void hal_send_SGI(int cpu, int no)
{
	GICD_WRITEL(GICD_SGIR, (1 << (cpu + 16)) | (1 << 15) | no);
}

/* call to RTOS */
#ifdef CONFIG_SS_AMP
unsigned long signal_rtos(u32 type, u32 arg1, u32 arg2, u32 arg3)
{
    interos_call_mbox_args_t *ptr_mbox_args;
    interos_call_args_t *ptr_args;
    struct arm_smccc_res res;
    u32 ret;
    int signal_1st_using_mbox;

    if (disable_rtos)
    {
        return -1;
    }

	down(&_interos_call_mbox);

    signal_1st_using_mbox = _signal_1st_using_mbox;

    if (signal_1st_using_mbox)
    {
        ptr_mbox_args = (interos_call_mbox_args_t *)(BASE_REG_MAILBOX_PA+BK_REG(0x60)+IO_OFFSET);
        ptr_mbox_args->arg0_l = type & 0xFFFF;
        ptr_mbox_args->arg0_h = type >> 16;
        ptr_mbox_args->arg1_l = arg1 & 0xFFFF;
        ptr_mbox_args->arg1_h = arg1 >> 16;
        ptr_mbox_args->arg2_l = arg2 & 0xFFFF;
        ptr_mbox_args->arg2_h = arg2 >> 16;
        ptr_mbox_args->arg3_l = arg3 & 0xFFFF;
        ptr_mbox_args->arg3_h = arg3 >> 16;
        ptr_mbox_args->ret_l = 0;
        ptr_mbox_args->ret_h = 0;
    }
    else
    {
        ptr_args = (interos_call_args_t *)((u32)_rtklin + INTEROS_CALL_SHMEM_OFFSET_TX);
        ptr_args->arg0 = type;
        ptr_args->arg1 = arg1;
        ptr_args->arg2 = arg2;
        ptr_args->arg3 = arg3;
        ptr_args->ret = 0;
    }

  	arm_smccc_smc(INTEROS_SC_L2R_CALL, TARGET_BITS_CORE0, NSATT_BITS_GROUP0, SGIINTID_BITS_10, 0, 0, 0, 0, &res);

	down(&_interos_call_resp);

    if (signal_1st_using_mbox)
    {
        ret = (ptr_mbox_args->ret_h << 16) + ptr_mbox_args->ret_l;
    }
    else
    {
        ret = ptr_args->ret;
    }

	up(&_interos_call_mbox);

    return ret;
}
#endif
#ifdef CONFIG_LH_RTOS
#ifndef CONFIG_SMP
unsigned long signal_rtos(u32 type, u32 arg1, u32 arg2, u32 arg3)
{
	struct arm_smccc_res res;

    if (disable_rtos)
    {
        return -1;
    }

	arm_smccc_smc(type, arg1, arg2, arg3,
		      0, 0, 0, 0, &res);

	return res.a0;
}
#else
unsigned long signal_rtos(u32 type, u32 arg1, u32 arg2, u32 arg3)
{
	struct arm_smccc_res res;
	int cpu = smp_processor_id();

    if (disable_rtos)
    {
        return -1;
    }

    if (cpu != 0)
    {
        down(&(_reroute_smc_info.slock));
        _reroute_smc_info.type = type;
        _reroute_smc_info.arg1 = arg1;
        _reroute_smc_info.arg2 = arg2;
        _reroute_smc_info.arg3 = arg3;
        hal_send_SGI(0, IPI_NR_REROUTE_SMC);
        down(&(_reroute_smc_info.rlock));
        res.a0 = _reroute_smc_info.ret;
        up(&(_reroute_smc_info.slock));
    }
    else
    {
    	arm_smccc_smc(type, arg1, arg2, arg3,
		      0, 0, 0, 0, &res);
    }

	return res.a0;
}

unsigned long signal_rtos_busy_wait(u32 type, u32 arg1, u32 arg2, u32 arg3)
{
	struct arm_smccc_res res;
	int cpu = smp_processor_id();

    if (cpu != 0)
    {
        unsigned long flags;
        spin_lock_irqsave(&(_reroute_smc_busy_wait_info.slock), flags);
        _reroute_smc_busy_wait_info.type = type;
        _reroute_smc_busy_wait_info.arg1 = arg1;
        _reroute_smc_busy_wait_info.arg2 = arg2;
        _reroute_smc_busy_wait_info.arg3 = arg3;
        hal_send_SGI(0, IPI_NR_REROUTE_SMC_BUSY_WAIT);
        atomic_inc(&(_reroute_smc_busy_wait_info.rlock));
        isb();
        dsb();
        while (atomic_read(&(_reroute_smc_busy_wait_info.rlock)));
        res.a0 = _reroute_smc_busy_wait_info.ret;
        spin_unlock_irqrestore(&(_reroute_smc_busy_wait_info.slock), flags);
    }
    else
    {
    	arm_smccc_smc(type, arg1, arg2, arg3,
		      0, 0, 0, 0, &res);
    }

	return res.a0;
}

void handle_reroute_smc(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(_reroute_smc_info.type,
                _reroute_smc_info.arg1,
                _reroute_smc_info.arg2,
                _reroute_smc_info.arg3,
		        0, 0, 0, 0, &res);
    _reroute_smc_info.ret = res.a0;
    up(&(_reroute_smc_info.rlock));
}

void handle_reroute_smc_busy_wait(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(_reroute_smc_busy_wait_info.type,
                _reroute_smc_busy_wait_info.arg1,
                _reroute_smc_busy_wait_info.arg2,
                _reroute_smc_busy_wait_info.arg3,
		        0, 0, 0, 0, &res);
    _reroute_smc_busy_wait_info.ret = res.a0;
    isb();
    dsb();
    atomic_dec(&(_reroute_smc_busy_wait_info.rlock));
}
#endif
#endif

fs_initcall(epoch_init);
