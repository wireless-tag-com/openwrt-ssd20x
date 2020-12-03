/*
* linux-rtos-adaptor.c- Sigmastar
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
#include <linux/module.h>
#include <linux/device.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/arm-smccc.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/semaphore.h>
#include <linux/atomic.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>
#include "drv_dualos.h"

#define  E_MI_MODULE_ID_MAX 31

#define __MI_DEVICE_PROC 0xfffffffful
#define __MI_DEVICE_PROC_IO 0xfffffffeul
#define __MI_DEVICE_PROC_READLOG 0xfffffffdul
#define __MI_DEVICE_CONNECT 0
#define __MI_DEVICE_DISCONNECT 1
#define __MI_DEVICE_QUERY 2
#define __MI_DEVICE_POLL_CREATE 3
#define __MI_DEVICE_POLL_RELEASE 4
#define __MI_DEVICE_POLL_STATE 5
const unsigned int ALKAID_CALL_START =  0xff000000;
const unsigned int ALKAID_CALL_END   =  0xff000040;
const int ALKAID_RTKTRACE = 0;
//#define MMA_BASE (0x20000000ul+E_MMAP_ID_RTK_mma_heap_ADR)
//#define MMA_SIZE (0x0ul+E_MMAP_ID_RTK_mma_heap_LEN)
#define CTX_NUM 2
#define CTX_BASE 8

#define FIVE_SEC (HZ*5)

typedef struct {
    struct {
        union {
            void *curr_base;
            int pid;
            unsigned long cmd;
            unsigned long poll_handle;
        };
        long idx;
        long devid;
        union {
            unsigned long mma_base;
            unsigned long log_base;
        };
        unsigned int arg_size;
    };
    char __pad[64];
} linux_ctx __attribute__((aligned(64)));
struct proc_dev {
    int modid;
    int devid;
    int next;
    char cmd_list[];
} __attribute__((aligned(64)));
struct proc_buffer {
    int cost;
    char buf[];
};
#define POLL_FILE_MAX 64
typedef enum {
    E_MI_COMMON_POLL_NOT_READY         = (0x0)     ,
    E_MI_COMMON_FRAME_READY_FOR_READ   = (0x1 << 0),
    E_MI_COMMON_BUFFER_READY_FOR_WRITE = (0x1 << 1),
} MI_COMMON_PollFlag_e;

static int disable_os_adaptor = 0;
module_param(disable_os_adaptor, int, 0644);
MODULE_PARM_DESC(disable_os_adaptor, "Disable Linux-RTOS Adaptor");

/*
 * from ARM Architecture Reference Manual
 *                    ARMv7-A and ARMv7-R edition
 * B3.18.6 Cache maintenance operations, functional group, VMSA
 * Table B3-49 Cache and branch predictor maintenance operations, VMSA
 */
static void flush_cache_area(void *ptr, int len){
    const unsigned long cache_line_size = 64;
    unsigned long iter, end;
    iter = (unsigned long)ptr, end = (unsigned long)ptr + len;
    iter = iter/cache_line_size*cache_line_size;
    end = end/cache_line_size*cache_line_size;
    asm __volatile__("dsb st":::"memory"); /* data sync barrier for store */
    while(iter <= end){
        //asm __volatile__("mcr p15, 0, %0, c7, c11, 1"::"r"(iter):"memory"); /* DCCMVAC: flush to PoU (aka last level cache) */
        asm __volatile__("mcr p15, 0, %0, c7, c10, 1"::"r"(iter):"memory"); /* DCCMVAU: flush to PoC (aka main memory) */
        iter += cache_line_size;
    }
}
static void invalid_cache_area(void *ptr, int len){
    const unsigned long cache_line_size = 64;
    unsigned long iter, end;
    iter = (unsigned long)ptr, end = (unsigned long)ptr + len;
    iter = iter/cache_line_size*cache_line_size;
    end = end/cache_line_size*cache_line_size;
    while(iter <= end){
        asm __volatile__("mcr p15, 0, %0, c7, c6, 1"::"r"(iter):"memory"); /* DCIMVAC: invalidate to PoC */
        iter += cache_line_size;
    }
}
static void flush_and_invalid_cache_area(void *ptr, int len){
    const unsigned long cache_line_size = 64;
    unsigned long iter, end;
    iter = (unsigned long)ptr, end = (unsigned long)ptr + len;
    iter = iter/cache_line_size*cache_line_size;
    end = end/cache_line_size*cache_line_size;
    asm __volatile__("dsb st":::"memory"); /* data sync barrier for store */
    while(iter <= end){
        asm __volatile__("mcr p15, 0, %0, c7, c14, 1"::"r"(iter):"memory"); /* DCCIMVAC: flush & invalid to PoC (aka main memory) */
        iter += cache_line_size;
    }
}

static struct class *device_class;
static struct device *device_list[E_MI_MODULE_ID_MAX + 1];
static struct proc_dev *proc_list[E_MI_MODULE_ID_MAX];
static int device_major, poll_major;
static struct semaphore device_sem[CTX_NUM];
static struct semaphore ctx_sem;
static spinlock_t ctx_lock;
static DECLARE_BITMAP(ctx_bitmap, 32);
static atomic_t device_ref;
static struct resource *rtk_res;


// IPC_SHARE_ADDR + 0x0000 ~ IPC_SHARE_ADDR + 0x1000  ==> RSQ for basic IPC
// IPC_SHARE_ADDR + 0x1000 ~ IPC_SHARE_ADDR + 0x5000  ==> RSQ for log
// IPC_SHARE_ADDR + 0x5000 ~ IPC_SHARE_ADDR + 0x6000  ==> RSQ for customize settings

static unsigned long mma_base = 0x25500000;
static unsigned long mma_size = 0x02700000;

/*
example:
os_adaptor=mma_base=0x25500000,mma_size=0x02700000
os_adaptor=mma_base=0x21F00000,mma_size=0x01D00000
*/
static bool parse_os_adaptor_config(char *cmdline, unsigned long *mma_base, unsigned long *mma_size)
{
    char *option;

    if(cmdline == NULL)
        goto INVALID_OS_ADAPTOR_CONFIG;

    option = strstr(cmdline, "mma_size=");
    if(option == NULL)
        goto INVALID_OS_ADAPTOR_CONFIG;
    option = strstr(cmdline, "mma_base=");
    if(option == NULL)
        goto INVALID_OS_ADAPTOR_CONFIG;
    sscanf(option, "mma_base=%lx,mma_size=%lx", mma_base, mma_size);

    return true;

INVALID_OS_ADAPTOR_CONFIG:

    return false;
}

int __init setup_os_adaptor(char *cmdline)
{
    if(!parse_os_adaptor_config(cmdline, &mma_base, &mma_size))
        printk(KERN_ERR "error: os_adaptor args invalid\n");

    return 0;
}
early_param("os_adaptor", setup_os_adaptor);

static atomic_t ctx_cost[CTX_NUM][sizeof(unsigned long)*8] = {};
static atomic_t ctx_freq[CTX_NUM] = {};


static struct proc_dir_entry *debug_tools;
static struct proc_dir_entry *proc_root;
struct debug_tool {
    struct proc_dir_entry *entry;
    void *obj;
    ssize_t (*write)(void *obj, const char **args, int count);
    ssize_t (*read)(void *obj);
};
struct debug_tool_freq {
    struct debug_tool dt;
    int interval;
};
struct debug_tool_info {
    struct debug_tool dt;
    const char *version;
};
static ssize_t ctx_cost_erase(void *obj, const char **args, int count){
    atomic_t (*cost)[sizeof(unsigned long)*8] = ctx_cost;
    int i, j;
    for(i = 0; i < CTX_NUM; ++i){
        for(j = 0; j < sizeof(unsigned long)*8; ++j){
            atomic_set(&cost[i][j], 0);
        }
    }
    return 0;
}
static ssize_t ctx_cost_hist(void *obj){
    atomic_t (*cost)[sizeof(unsigned long)*8] = ctx_cost;
    int i, j;
    for(i = 0; i < CTX_NUM; ++i){
        for(j = 0; j < sizeof(unsigned long)*8; ++j){
            printk("CTX_%d|%02d:%d\n", i, j, atomic_read(&cost[i][j]));
        }
        printk("CTX--------------------\n");
    }
    return 0;
}
static ssize_t ctx_cost_freq_setup(void *obj, const char **args, int count){
    struct debug_tool_freq *dtf = obj;
    if(count == 1){
	 	if (kstrtoint((const char *)args[0], 0, &dtf->interval))
			return -EFAULT;
        printk("freq watch interval=%dms\n", dtf->interval);
        return count;
    }
    return -EINVAL;
}
static ssize_t ctx_cost_freq(void *obj){
    atomic_t *freq = ctx_freq;
    int i;
    for(i = 0; i < CTX_NUM; ++i){
        atomic_xchg(freq+i, 0);
    }
    while(schedule_timeout_interruptible(msecs_to_jiffies(200)) == 0){
        char buf[8*(CTX_NUM+1)] = {0};
        unsigned long rval;
        unsigned long tmp;
        tmp = atomic_xchg(freq, 0);
        rval = sprintf(buf, "%8lu", tmp);
        for(i = 1; i < CTX_NUM; ++i){
            tmp = atomic_xchg(freq+i, 0);
            rval+=sprintf(buf+rval, "|%8lu", tmp);
        }
        printk("CTX_FREQ:%s\n", buf);
    }
    return 0;
}

static ssize_t compile_version_info(void *obj){
    struct debug_tool_info *dti = obj;
    printk("version string:%s\n", dti->version);
    return 0;
}

static struct debug_tool syscall_cost_column = {
    .write = ctx_cost_erase,
    .read = ctx_cost_hist,
};
static struct debug_tool_freq syscall_freq_linear = {
    {
        .obj = &syscall_freq_linear,
        .write = ctx_cost_freq_setup,
        .read = ctx_cost_freq,
    }
};
static struct debug_tool_info info_tool = {
    {
        .obj = &info_tool,
        .read = compile_version_info,
    },
    .version = "version",
};

static unsigned int time_log2(ktime_t start, ktime_t end){
    unsigned int idx = 0;
    unsigned long us = ktime_to_us(ktime_sub(end, start));
    while(us){
        idx = idx + 1;
        us = us >> 1;
    }
    return idx;
}

void alkaid_registe_notify(int cpu, void *notify);
void *alkaid_unregiste_notify(int cpu);
typedef struct {
    wait_queue_head_t stPollHead;
    unsigned long poll_handle;
    struct list_head list;
} MI_COMMON_PollFileWrapper_t;
static LIST_HEAD(poll_task);
static bool alkaid_notify(int idx){
    int mid = idx-CTX_BASE;
    if(mid < CTX_NUM){
        if(down_trylock(device_sem+mid) == 0){
            printk("bug found at %s %d, %d\n", __func__, __LINE__, idx);
            *(int*)0 = 0;
        }
        up(device_sem+mid);
    }else{
        MI_COMMON_PollFileWrapper_t *f;
        rcu_read_lock();
        list_for_each_entry_rcu(f, &poll_task, list){
            wake_up(&f->stPollHead);
        }
        rcu_read_unlock();
    }
    return true;
}
static unsigned long alkaid_poll_wapper(MI_COMMON_PollFileWrapper_t *f, int cmd){
    unsigned long res = 0;
    linux_ctx ctx = {.poll_handle = 0, .idx = -1,};

    ctx.poll_handle = f ? f->poll_handle : 0;
    while(1) {
        if(down_timeout(&ctx_sem, FIVE_SEC) == 0){
            if(CTX_NUM > 1){
                spin_lock(&ctx_lock);
                ctx.idx = find_first_zero_bit(ctx_bitmap, 32);
                if(ctx.idx < CTX_NUM) {
                    set_bit(ctx.idx, ctx_bitmap);
                }
                spin_unlock(&ctx_lock);
            } else {
                ctx.idx = 0;
            }
            if(ctx.idx < CTX_NUM) {
                extern void Chip_Flush_MIU_Pipe(void);
                flush_cache_area(&ctx, sizeof(ctx));
                Chip_Flush_MIU_Pipe();
                break;
            }
        } else {
            printk(KERN_WARNING "dead lock check at %s %d, poll handl:%ld\n", __func__,
            __LINE__, ctx.poll_handle);
        }
    }
    res = signal_rtos(ALKAID_CALL_START+E_MI_MODULE_ID_MAX, __pa((long)&ctx), -1, cmd);
    while(1) {
        if (down_timeout(device_sem+ctx.idx, FIVE_SEC) == 0) {
            res = signal_rtos(ALKAID_CALL_START+E_MI_MODULE_ID_MAX, ctx.idx, -1, __MI_DEVICE_QUERY);
            if(res != -2) {
                break;
            } else {
                printk(KERN_ERR "bug found at %s %d\n", __func__, __LINE__);
                *(int*)0 = 0;
            }
        } else {
            printk(KERN_WARNING "dead lock check at %s %d!poll handl:%ld\n", __func__,
            __LINE__, ctx.poll_handle);
        }
    }
    clear_bit(ctx.idx, ctx_bitmap);
    up(&ctx_sem);

    return res;
}
static unsigned long alkaid_poll_init(void){
    return alkaid_poll_wapper(NULL, __MI_DEVICE_POLL_CREATE);
}
static int MI_PollAdp_Open(struct inode *inode, struct file *filp) {
    MI_COMMON_PollFileWrapper_t *fw = kmalloc(sizeof(MI_COMMON_PollFileWrapper_t), GFP_KERNEL);
    if(fw){
        init_waitqueue_head(&fw->stPollHead);
        fw->poll_handle = 0;
        INIT_LIST_HEAD(&fw->list);
        filp->private_data = fw;
        return 0;
    }
    return -ENOMEM;
}

static int MI_PollAdp_Release(struct inode *inode, struct file *filp) {
    MI_COMMON_PollFileWrapper_t *f = filp->private_data;

    list_del_rcu(&f->list);
    alkaid_poll_wapper(f, __MI_DEVICE_POLL_RELEASE);
    kfree(f);

    return 0;
}

static long MI_PollAdp_Ioctl(struct file *filp, unsigned int cmd, unsigned long ptr) {
    MI_COMMON_PollFileWrapper_t *f = filp->private_data;
    f->poll_handle = ptr;
    list_add_rcu(&f->list, &poll_task);
    return 0;
}

static unsigned int MI_PollAdp_Poll(struct file *filp, poll_table *wait){
    MI_COMMON_PollFileWrapper_t *f = filp->private_data;
    unsigned int req_events = poll_requested_events(wait);
    unsigned int mask = 0;
    unsigned long ret = 0;

    poll_wait(filp, &f->stPollHead, wait);
    ret = alkaid_poll_wapper(f, __MI_DEVICE_POLL_STATE);
    if(ret & E_MI_COMMON_FRAME_READY_FOR_READ)
        mask |= POLLIN;
    if(ret & E_MI_COMMON_BUFFER_READY_FOR_WRITE)
        mask |= POLLOUT;
    return req_events & mask;
}

static const struct file_operations pfops = {
    .owner      = THIS_MODULE,
    .open       = MI_PollAdp_Open,
    .release    = MI_PollAdp_Release,
    .unlocked_ioctl = MI_PollAdp_Ioctl,
    .poll       = MI_PollAdp_Poll,
    .llseek     = noop_llseek,
};
static int MI_DEVICE_Open(struct inode *inode, struct file *filp) {
    int id = iminor(inode);
    unsigned long res = 0;
    linux_ctx ctx = {.pid = current->pid, .idx = -1,};

    while(1) {
        if(down_timeout(&ctx_sem, FIVE_SEC) == 0){
            if(CTX_NUM > 1){
                spin_lock(&ctx_lock);
                ctx.idx = find_first_zero_bit(ctx_bitmap, 32);
                if(ctx.idx < CTX_NUM) {
                    set_bit(ctx.idx, ctx_bitmap);
                }
                spin_unlock(&ctx_lock);
            } else {
                ctx.idx = 0;
            }
            if(ctx.idx < CTX_NUM) {
                extern void Chip_Flush_MIU_Pipe(void);
                flush_cache_area(&ctx, sizeof(ctx));
                Chip_Flush_MIU_Pipe();
                break;
            }
        } else {
            printk(KERN_WARNING "dead lock check at %s %d!\n", __func__, __LINE__);
        }
    }
    res = signal_rtos(ALKAID_CALL_START+id, __pa((long)&ctx), -1, __MI_DEVICE_CONNECT);
    while(1) {
        if (down_timeout(device_sem+ctx.idx, FIVE_SEC) == 0) {
            res = signal_rtos(ALKAID_CALL_START+id, ctx.idx, -1, __MI_DEVICE_QUERY);
            if(res != -2) {
                break;
            } else {
                printk(KERN_ERR "bug found at %s %d\n", __func__, __LINE__);
                *(int*)0 = 0;
            }
        } else {
            printk(KERN_WARNING "dead lock check at %s %d!\n", __func__, __LINE__);
        }
    }
    clear_bit(ctx.idx, ctx_bitmap);
    up(&ctx_sem);
    /* map mma range */
    filp->private_data = (void*)vm_mmap(filp, 0, mma_size, PROT_READ|PROT_WRITE, MAP_SHARED, rtk_res->start);
    if((unsigned long)filp->private_data < PAGE_OFFSET){
        return res;
    }

    return (long)filp->private_data;
}

static int MI_DEVICE_Release(struct inode *inode, struct file *filp) {
    int id = iminor(inode);
    unsigned long res = 0;
    linux_ctx ctx = {.pid = current->pid, .idx = -1,};
    while(1) {
        if(down_timeout(&ctx_sem, FIVE_SEC) == 0){
            if(CTX_NUM > 1){
                spin_lock(&ctx_lock);
                ctx.idx = find_first_zero_bit(ctx_bitmap, 32);
                if(ctx.idx < CTX_NUM) {
                    set_bit(ctx.idx, ctx_bitmap);
                }
                spin_unlock(&ctx_lock);
            } else {
                ctx.idx = 0;
            }
            if(ctx.idx < CTX_NUM) {
                extern void Chip_Flush_MIU_Pipe(void);
                flush_cache_area(&ctx, sizeof(ctx));
                Chip_Flush_MIU_Pipe();
                break;
            }
        } else {
            printk(KERN_WARNING "dead lock check at %s %d!\n", __func__, __LINE__);
        }
    }
    res = signal_rtos(ALKAID_CALL_START+id, __pa((long)&ctx), -1, __MI_DEVICE_DISCONNECT);
    while(1) {
        if (down_timeout(device_sem+ctx.idx, FIVE_SEC) == 0) {
            res = signal_rtos(ALKAID_CALL_START+id, ctx.idx, -1, __MI_DEVICE_QUERY);
            if(res != -2) {
                break;
            } else {
                printk(KERN_ERR "bug found at %s %d\n", __func__, __LINE__);
                *(int*)0 = 0;
            }
        } else {
            printk(KERN_WARNING "dead lock check at %s %d,id=%d\n", __func__, __LINE__,id);
        }
    }
    clear_bit(ctx.idx, ctx_bitmap);
    up(&ctx_sem);
    if(atomic_dec_return(&device_ref) == 0){
        alkaid_unregiste_notify(0);
        printk("unregister notify\n");
    }
    return res;
}

static unsigned long vir2phy(struct task_struct *curr, void *ptr){
    unsigned long addr = (unsigned long)ptr;
    pgd_t *pgd = pgd_offset(curr->mm,addr);
    pud_t *pud = pud_offset(pgd,addr);
    pmd_t *pmd = pmd_offset(pud,addr);
    pte_t *pte = pmd_page_vaddr(*pmd)+pte_index(addr);
    return __pfn_to_phys(pte_pfn(*pte))+(addr&~PAGE_MASK);
}

static long MI_DEVICE_Ioctl(struct file *filp, unsigned int cmd, unsigned long ptr) {
    int id = iminor(file_inode(filp));
    long rval = -EIO;
    if(_IOC_TYPE(cmd) == 'i') {
        unsigned long res = 0;
        atomic_t (*cost)[sizeof(unsigned long)*8];
        atomic_t *freq;
        ktime_t t1, t2;
        t1 = ktime_get();
        if(ptr) {
            extern void Chip_Flush_MIU_Pipe(void);
            linux_ctx ctx = {.curr_base = filp->private_data, .idx = -1, .mma_base = mma_base};
            struct {
                int len;
                unsigned long long ptr;
            } tr;
            void *arg = NULL;
		 	if (copy_from_user((char *)&tr, (void*)ptr, sizeof(tr)))
				return -EFAULT;
            if(tr.len > _IOC_SIZE(cmd)) {
                printk(KERN_ERR "write cmd(0x%08x) overflow!", cmd);
                return -EINVAL;
            }

            if(tr.len > 4096) {
                printk(KERN_WARNING "write cmd(0x%08x) Send Big Data size(%d)!", cmd, tr.len);
            }

            if(_IOC_DIR(cmd) & _IOC_WRITE) {
                if(tr.len == 0) {
                    printk(KERN_ERR "write cmd(0x%08x) send null data!", cmd);
                    return -EINVAL;
                }
                arg = memdup_user((void*)(long)tr.ptr, tr.len);
                if(!arg)
                    return -ENOMEM;
                if(_IOC_DIR(cmd) & _IOC_READ) {
                    flush_and_invalid_cache_area(arg, tr.len);
                }else{
                    flush_cache_area(arg, tr.len);
                }
            } else if(_IOC_DIR(cmd) & _IOC_READ) {
                arg = kmalloc(tr.len+sizeof(long), GFP_KERNEL);
                if(!arg)
                    return -ENOMEM;
                invalid_cache_area(arg, tr.len);
            } else {
                printk(KERN_ERR "send a buffer to cmd(0x%08x) with_IOC_TYPE_NONE!\n", cmd);
                return -EINVAL;
            }
            ctx.arg_size = _IOC_SIZE(cmd);
            while(1) {
                if(down_timeout(&ctx_sem, FIVE_SEC) == 0){
                    if(CTX_NUM > 1){
                        spin_lock(&ctx_lock);
                        ctx.idx = find_first_zero_bit(ctx_bitmap, 32);
                        if(ctx.idx < CTX_NUM) {
                            set_bit(ctx.idx, ctx_bitmap);
                        }
                        spin_unlock(&ctx_lock);
                    } else {
                        ctx.idx = 0;
                    }
                    if(ctx.idx < CTX_NUM) {
                        flush_cache_area(&ctx, sizeof(ctx));
                        break;
                    }
                } else {
                    printk(KERN_WARNING "dead lock check at %s %d!dev:%d, cmd:%d\n", __func__,
                        __LINE__, id, _IOC_NR(cmd));
                }
            }
            Chip_Flush_MIU_Pipe();
            while(1) {
                res = signal_rtos(ALKAID_CALL_START+id, __pa((long)&ctx), cmd, __pa((unsigned long)arg));
                if (down_timeout(device_sem+ctx.idx, FIVE_SEC) == 0) {
                    res = signal_rtos(ALKAID_CALL_START+id, __pa((long)&ctx), cmd, __pa((unsigned long)arg));
                    if(res != -2) {
                        break;
                    } else {
                        printk(KERN_ERR "bug found at %s %d\n", __func__, __LINE__);
                        *(int*)0 = 0;
                    }
                } else {
                    printk(KERN_WARNING "dead lock check at %s %d!dev:%d, cmd:%d\n", __func__,
                        __LINE__, id, _IOC_NR(cmd));
                }
            }
            clear_bit(ctx.idx, ctx_bitmap);
            up(&ctx_sem);
            cost = ctx_cost+ctx.idx;
            freq = ctx_freq+ctx.idx;
            rval = res;
            if(_IOC_DIR(cmd) & _IOC_READ) {
                invalid_cache_area(arg, tr.len);
		 	    if (copy_to_user((char*)(long)tr.ptr, arg, tr.len))
				    return -EFAULT;
            }
            kfree(arg);
        } else {
            int ctxid;
            while(1) {
                if(down_timeout(&ctx_sem, FIVE_SEC) == 0){
                    if(CTX_NUM > 1){
                        spin_lock(&ctx_lock);
                        ctxid = find_first_zero_bit(ctx_bitmap, 32);
                        if(ctxid < CTX_NUM) {
                            set_bit(ctxid, ctx_bitmap);
                        }
                        spin_unlock(&ctx_lock);
                        if(ctxid < CTX_NUM)
                            break;
                    } else {
                        ctxid = 0;
                        break;
                    }
                } else {
                    printk(KERN_WARNING "dead lock check at %s %d!dev:%d, cmd:%d\n", __func__,
                        __LINE__, id, _IOC_NR(cmd));
                }
            }
            while(1) {
                res = signal_rtos(ALKAID_CALL_START+id, ctxid, cmd, 0);
                if (down_timeout(device_sem+ctxid, FIVE_SEC) == 0) {
                    res = signal_rtos(ALKAID_CALL_START+id, ctxid, cmd, 0);
                    if(res != -2) {
                        break;
                    } else {
                        printk(KERN_ERR "bug found at %s %d\n", __func__, __LINE__);
                        *(int*)0 = 0;
                    }
                } else {
                    printk(KERN_WARNING "dead lock check at %s %d!dev:%d, cmd:%d\n", __func__,
                        __LINE__, id, _IOC_NR(cmd));
                }
            }
            clear_bit(ctxid, ctx_bitmap);
            up(&ctx_sem);
            cost = ctx_cost+ctxid;
            freq = ctx_freq+ctxid;
            rval = res;
        }
        t2 = ktime_get();
        atomic_inc(freq);
        atomic_inc(*cost+time_log2(t1, t2));
    }else{
        unsigned long *vir = filp->private_data+cmd;
        unsigned long uval;
        get_user(uval, vir);
        printk("uva:%p,phy:%lx,off=%x,uval=%lx\n", vir, vir2phy(current, vir), cmd, uval);
        rval = 0;
    }
    return rval;
}
static int MI_DEVICE_Mmap(struct file *file, struct vm_area_struct *vma) {
    static const struct vm_operations_struct vma_ops = {};
    size_t size = vma->vm_end - vma->vm_start;

    vma->vm_page_prot = phys_mem_access_prot(file, vma->vm_pgoff,
                        size,
                        vma->vm_page_prot);

    vma->vm_ops = &vma_ops;

    /* Remap-pfn-range will mark the range VM_IO */
    if (remap_pfn_range(vma,
                        vma->vm_start,
                        vma->vm_pgoff,
                        size,
                        vma->vm_page_prot)) {
        return -EAGAIN;
    }
    return 0;
}
static const struct file_operations fops = {
    .owner      = THIS_MODULE,
    .open       = MI_DEVICE_Open,
    .release    = MI_DEVICE_Release,
    .unlocked_ioctl = MI_DEVICE_Ioctl,
    .mmap       = MI_DEVICE_Mmap,
    .llseek     = noop_llseek,
};
module_param(mma_base,ulong,0644);
module_param(mma_size,ulong,0644);
static unsigned long rtk_base(void){
    return signal_rtos(INTEROS_SC_L2R_HANDSHAKE, 0, 0, 0);
}

static ssize_t proc_read(struct seq_file* q, void* v)
{
    struct proc_dev *pd = q->private;
    unsigned long res = 0;
    unsigned long logpage = get_zeroed_page(GFP_KERNEL);
    linux_ctx ctx = {.log_base = __pa(logpage), .idx = -1, .devid = pd->devid};
    while(1) {
        if(down_timeout(&ctx_sem, FIVE_SEC) == 0){
            if(CTX_NUM > 1){
                spin_lock(&ctx_lock);
                ctx.idx = find_first_zero_bit(ctx_bitmap, 32);
                if(ctx.idx < CTX_NUM) {
                    set_bit(ctx.idx, ctx_bitmap);
                }
                spin_unlock(&ctx_lock);
            } else {
                ctx.idx = 0;
            }
            if(ctx.idx < CTX_NUM) {
                extern void Chip_Flush_MIU_Pipe(void);
                flush_cache_area(&ctx, sizeof(ctx));
                Chip_Flush_MIU_Pipe();
                break;
            }
        } else {
            printk(KERN_WARNING "dead lock check at %s %d!\n", __func__, __LINE__);
        }
    }

    invalid_cache_area((void*)logpage, PAGE_SIZE);

    res = signal_rtos(ALKAID_CALL_START+pd->modid, __pa((long)&ctx), -1, __MI_DEVICE_PROC_IO);

    do {
        struct proc_buffer *pb = (struct proc_buffer*)logpage;
        down(device_sem+ctx.idx);

        if(pb->cost > 0){
            seq_write(q, pb->buf, pb->cost);
        }else{
            printk(KERN_WARNING "wake up with nothing!\n");
        }
        invalid_cache_area((void*)logpage, PAGE_SIZE);
        res = signal_rtos(ALKAID_CALL_START+pd->modid, ctx.idx, -1, __MI_DEVICE_PROC_READLOG);
    }while(res);

    clear_bit(ctx.idx, ctx_bitmap);
    up(&ctx_sem);
    free_page(logpage);
    return 0;
}

static ssize_t proc_write(struct file* file, const char __user* user_buf, size_t count, loff_t* ppos)
{
    unsigned long args[32] = {0ul};
    const char *lcmd;
    int c = 0;
    char *p, tc, *kbuf;

    struct seq_file *q = file->private_data;
    struct proc_dev *pd = q->private;

    kbuf = memdup_user(user_buf, count);
    if(!kbuf)
        return -ENOMEM;

    for(p = kbuf, tc = '\0'; tc != '\n' && (c < 32); ++p){
        p += strspn(p, " \t\r\f\v");
        if(*p == '\n')
            break;
        args[c++] = __pa(p);
        p += strcspn(p, " \t\n\r\f\v");
        tc = *p;
        *p = '\0';
    }
    lcmd = __va(args[0]);

    if(c < 32) {
        /* search cmd and exec */
        char *iter = pd->cmd_list, *cmd, *end = pd->cmd_list+pd->next;
        while(iter < end){
            cmd = iter;
            while(*iter++)
                ;
            if(strcmp(cmd, lcmd) == 0){
                unsigned long res = 0;
                unsigned long logpage = get_zeroed_page(GFP_KERNEL);
                linux_ctx ctx = {.cmd = __pa(args), .log_base = __pa(logpage), .arg_size = c, .idx = -1, .devid = pd->devid};
                while(1) {
                    if(down_timeout(&ctx_sem, FIVE_SEC) == 0){
                        if(CTX_NUM > 1){
                            spin_lock(&ctx_lock);
                            ctx.idx = find_first_zero_bit(ctx_bitmap, 32);
                            if(ctx.idx < CTX_NUM) {
                                set_bit(ctx.idx, ctx_bitmap);
                            }
                            spin_unlock(&ctx_lock);
                        } else {
                            ctx.idx = 0;
                        }
                        if(ctx.idx < CTX_NUM) {
                            extern void Chip_Flush_MIU_Pipe(void);
                            flush_cache_area(args, sizeof(long)*c);
                            flush_cache_area(kbuf, count);
                            flush_cache_area(&ctx, sizeof(ctx));
                            Chip_Flush_MIU_Pipe();
                            break;
                        }
                    } else {
                        printk(KERN_WARNING "dead lock check at %s %d!\n", __func__, __LINE__);
                    }
                }
                invalid_cache_area((void*)logpage, PAGE_SIZE);
                res = signal_rtos(ALKAID_CALL_START+pd->modid, __pa((long)&ctx), -1, __MI_DEVICE_PROC_IO);
                do {
                    struct proc_buffer *pb = (struct proc_buffer*)logpage;
                    down(device_sem+ctx.idx);
                    if(pb->cost > 0){
                        printk("%s", pb->buf);
                        memset(pb->buf, 0, pb->cost);
                    }else{
                        printk(KERN_WARNING "wake up with nothing!\n");
                    }
                    invalid_cache_area((void*)logpage, PAGE_SIZE);
                    res = signal_rtos(ALKAID_CALL_START+pd->modid, ctx.idx, -1, __MI_DEVICE_PROC_READLOG);
                } while(res);

                clear_bit(ctx.idx, ctx_bitmap);
                up(&ctx_sem);
                free_page(logpage);
                kfree(kbuf);
                return count;
            }
        }
    }
    kfree(kbuf);
    return -EINVAL;
}

static int proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read, PDE_DATA(inode));
}

static const struct file_operations proc_ops = {
    .owner      = THIS_MODULE,
    .open       = proc_open,
    .read       = seq_read,
    .write      = proc_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static void fetch_proc(int id, const char *name){
    unsigned long res = 0;
    unsigned long zpage = get_zeroed_page(GFP_KERNEL);
    struct proc_dev *pd = (struct proc_dev*)zpage;
    invalid_cache_area(pd, PAGE_SIZE);
    res = signal_rtos(ALKAID_CALL_START+id, __pa(pd), -1, __MI_DEVICE_PROC);
    if(res != __pa(pd)){
        char path[32] = "mi_";
        struct proc_dir_entry *dir;
        int len = res-__pa(pd);
        proc_list[id] = pd = kmemdup(pd, len, GFP_KERNEL);
        strcat(path, name);
        dir = proc_mkdir(path, proc_root);
        do{
            //char *iter = pd->cmd_list, *cmd, *end = pd->cmd_list+pd->next;
            sprintf(path, "mi_%s%d", name, pd->devid);
            proc_create_data(path, 0640, dir, &proc_ops, pd);
            /*
            while(iter < end){
                cmd = iter;
                while(*iter++)
                    ;
            }*/
            if((long)(pd->cmd_list+pd->next) != (long)proc_list[id]+len){
                pd = (struct proc_dev*)(pd->cmd_list+pd->next);
            }else{
                break;
            }
        }while(1);
    }else{
        proc_list[id] = NULL;
    }
    free_page(zpage);
}

static ssize_t debug_tool_read(struct seq_file* q, void* v)
{
    struct debug_tool *dt = q->private;
    if(dt->read)
        return dt->read(dt->obj);
    return -EIO;
}

static ssize_t debug_tool_write(struct file* file, const char __user* user_buf, size_t count, loff_t* ppos)
{
    const char *args[32] = {NULL};
    int c = 0;
    char *p, tc, *kbuf;

    struct seq_file *q = file->private_data;
    struct debug_tool *dt = q->private;

    if(!dt->write)
        return -EIO;

    kbuf = memdup_user(user_buf, count);
    if(!kbuf)
        return -ENOMEM;

    for(p = kbuf, tc = '\0'; tc != '\n' && (c < 32); ++p){
        p += strspn(p, " \t\r\f\v");
        if(*p == '\n')
            break;
        args[c++] = p;
        p += strcspn(p, " \t\n\r\f\v");
        tc = *p;
        *p = '\0';
    }

    if(c < 32) {
        dt->write(dt->obj,args,c);
        kfree(kbuf);
        return count;
    }
    kfree(kbuf);
    return -EINVAL;
}

static int debug_tool_open(struct inode *inode, struct file *file)
{
    return single_open(file, debug_tool_read, PDE_DATA(inode));
}

static const struct file_operations debug_tool_ops = {
    .owner      = THIS_MODULE,
    .open       = debug_tool_open,
    .read       = seq_read,
    .write      = debug_tool_write,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static bool debug_tool_create(const char *name, struct debug_tool *dt){
    dt->entry = proc_create_data(name, 0640, debug_tools, &debug_tool_ops, dt);
    if (!dt->entry)
    {
        printk(KERN_ERR "failed  to  create  procfs  file  %s.\n",name);
        return false;
    }
    return true;
}

static void debug_tool_delete(struct debug_tool *dt){
    proc_remove(dt->entry);
}

static void linux_adp_SyncPreloadStatus(void)
{
    /*
     * Fixme, always create "dualos/audio_init" and "dualos/video_init" here,
     * need design a mechanism to sync preload status inside mi_sys.
     */
    proc_mkdir("dualos/audio_init", NULL);
    proc_mkdir("dualos/video_init", NULL);
}

static int __init linux_adaptor_init(void) {
    const char *dev_list[] = {
        "ive",    /* 0 */
        "vdf",    /* 1 */
        "venc",   /* 2 */
        "rgn",    /* 3 */
        "ai",     /* 4 */
        "ao",     /* 5 */
        "vif",    /* 6 */
        "vpe",    /* 7 */
        "vdec",   /* 8 */
        "sys",    /* 9 */
        "fb",     /* 10 */
        "hdmi",   /* 11 */
        "divp",   /* 12 */
        "gfx",    /* 13 */
        "vdisp",  /* 14 */
        "disp",   /* 15 */
        "os",     /* 16 */
        "iae",    /* 17 */
        "md",     /* 18 */
        "od",     /* 19 */
        "shadow", /* 20 */
        "warp",   /* 21 */
        "uac",    /* 22 */
        "ldc",    /* 23 */
        "sd",     /* 24 */
        "panel",  /* 25 */
        "cipher", /* 26 */
        "sensor",    /* 27 */
        "wlan",    /* 28 */
        "ipu",    /* 29 */
        "mipitx",    /* 30 */
    };
    int err = 0, i;

    if (disable_os_adaptor)
        return 0;

    if(E_MI_MODULE_ID_MAX < sizeof(dev_list)/sizeof(*dev_list)){
        return -EINVAL;
    }
    err = -EIO;
    device_major = register_chrdev(0, "rtos-adaptor", &fops);
    if(device_major <= 0)
        goto fail_register_chrdev;

    device_class = class_create(THIS_MODULE, "rtos-adaptor");
    err = PTR_ERR(device_class);
    if (IS_ERR(device_class))
        goto fail_class_create;

    proc_root = proc_mkdir("mi_modules", NULL);
    for(i = 0; i < E_MI_MODULE_ID_MAX; ++i){
        device_list[i] = device_create(device_class,
                                    NULL,
                                    MKDEV(device_major, i),
                                    device_list+i,
                                    "mi_%s",
                                    dev_list[i]);
        fetch_proc(i, dev_list[i]);
    }
    poll_major = register_chrdev(0, "poll-dev", &pfops);
    if(poll_major <= 0)
        goto fail_register_chrdev2;

    device_list[E_MI_MODULE_ID_MAX] = device_create(device_class,
                                    NULL,
                                    MKDEV(poll_major, 0),
                                    device_list+E_MI_MODULE_ID_MAX,
                                    "mi_poll");

    if(device_list[E_MI_MODULE_ID_MAX] == NULL)
        goto fail_create_poll_dev;

    for(i = 0; i < CTX_NUM; ++i)
        sema_init(device_sem+i, 0);
    sema_init(&ctx_sem, CTX_NUM);
    atomic_set(&device_ref, 0);
    spin_lock_init(&ctx_lock);
    for(i = 0; i < 32; ++i)
        clear_bit(i, ctx_bitmap);
    rtk_res = request_mem_region(mma_base, mma_size, "mma");
    if(!rtk_res){
        goto fail_mem_req;
    }
    printk("#map req:(0x%x,0x%x)|0x%lx\n", rtk_res->start, resource_size(rtk_res), rtk_base());
    debug_tools = proc_mkdir("adaptor-debug-tools", NULL);
    if(!debug_tools)
        goto fail_create_debug_tools;

    if(!debug_tool_create("syscall_cost", &syscall_cost_column))
        goto fail_create_syscall_cost;

    if(!debug_tool_create("syscall_freq", &syscall_freq_linear.dt))
        goto fail_create_syscall_freq;

    if(!debug_tool_create("info_tool", &info_tool.dt))
        goto fail_create_info_tool;

    linux_adp_SyncPreloadStatus();

    alkaid_registe_notify(0, alkaid_notify);
    printk("register notify\n");

    alkaid_poll_init();
    printk("poll init dev\n");

    printk("linux-adaptor init success!(%s)\n", __TIME__);
    return 0;

fail_create_info_tool:
    printk(KERN_ERR "create info tool failed!\n");
    debug_tool_delete(&syscall_freq_linear.dt);
fail_create_syscall_freq:
    printk(KERN_ERR "create syscall freq analyzer failed!\n");
    debug_tool_delete(&syscall_cost_column);
fail_create_syscall_cost:
    printk(KERN_ERR "create syscall cost analyzer failed!\n");
    proc_remove(debug_tools);
fail_create_debug_tools:
    printk(KERN_ERR "proc mkdir failed!\n");
    release_mem_region(rtk_res->start, mma_size);
fail_mem_req:
    printk(KERN_ERR "request mem failed\n");
    device_destroy(device_class, MKDEV(poll_major, 0));
fail_create_poll_dev:
    printk(KERN_ERR "create poll dev failed\n");
    unregister_chrdev(poll_major, "poll-dev");
fail_register_chrdev2:
    printk(KERN_ERR "unable to get mi device\n");
    for(i = 0; i < E_MI_MODULE_ID_MAX; ++i){
        if(device_list[i])
            device_destroy(device_class, MKDEV(device_major, i));
    }
    unregister_chrdev(device_major, "rtos-adaptor");
fail_class_create:
    printk(KERN_ERR "fail create class\n");
fail_register_chrdev:
    printk(KERN_ERR "unable to get mi device\n");
    class_destroy(device_class);
    return err;
}
module_init(linux_adaptor_init)

static void __exit linux_adaptor_exit(void) {
    int i;
    debug_tool_delete(&info_tool.dt);
    debug_tool_delete(&syscall_freq_linear.dt);
    debug_tool_delete(&syscall_cost_column);
    proc_remove(debug_tools);
    release_mem_region(rtk_res->start, mma_size);
    device_destroy(device_class, MKDEV(poll_major, 0));
    unregister_chrdev(poll_major, "poll-dev");
    for(i = 0; i < E_MI_MODULE_ID_MAX; ++i){
        if(device_list[i])
            device_destroy(device_class, MKDEV(device_major, i));
    }
    unregister_chrdev(device_major, "rtos-adaptor");
    class_destroy(device_class);
}
module_exit(linux_adaptor_exit)

MODULE_LICENSE("GPL v2");
