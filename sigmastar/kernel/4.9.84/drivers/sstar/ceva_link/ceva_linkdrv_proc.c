/*
* ceva_linkdrv_proc.c- Sigmastar
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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h> /* Necessary because we use proc fs */
#include <linux/seq_file.h> /* for seq_file */
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/ioport.h>

#include "ceva_linkdrv_xm6.h"
#include "ceva_linkdrv_proc.h"
#include "ms_platform.h"

#define REGR(base,idx)      ms_readw(((uint)base+(idx)*4))

static struct proc_dir_entry *proc_dir;

static u32 host_log_index = 0;
static u32 host_log_size = 0;
static u32 host_log_head = 0;
static u32 host_log_tail = 0;
static u8 *p_host_log_buf = NULL;

static u32 dsp_log_index = 0;
static u32 dsp_log_size = 0;
static u32 dsp_log_head = 0;
static u32 dsp_log_tail = 0;
static dsp_info_buf *p_dsp_info_buf = NULL;

static bool host_log_inited = false;
static bool dsp_log_inited = false;
static spinlock_t host_log_lock;
static struct xm6_dev_data *p_dev_data = NULL;

static void* host_log_seq_start(struct seq_file *s, loff_t *pos)
{
    if (host_log_size == 0)
    {
        return NULL;
    }
    
    if ((*pos) == 0)
    {
        //seq_printf(s, "head = %d, tail=%d, size=%d, index=%d\n", host_log_head, host_log_tail, host_log_size, host_log_index);
        *pos = host_log_index;
        return (void *)(p_host_log_buf + host_log_index * HOST_LOG_LINE_SIZE);
    }

    return NULL; 
}

static void* host_log_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    int n = *pos;

    if (n < host_log_index)
    {
        n += HOST_LOG_MAX_LINE;
    }

    if (n - host_log_index + 1 >= host_log_size)
    {
        return NULL;			
    }

    (*pos)++;

    if ((*pos) >= HOST_LOG_MAX_LINE)
    {
        *pos = 0;
    }

    return (void *)(p_host_log_buf + (*pos) * HOST_LOG_LINE_SIZE);
}

static void host_log_seq_stop(struct seq_file *s, void *v)
{

}

static int host_log_seq_show(struct seq_file *s, void *v)
{
    seq_printf(s, "%s", (char *)v); 
    return 0;
}

static struct seq_operations host_log_seq_ops = {
    .start = host_log_seq_start,
    .next  = host_log_seq_next,
    .stop  = host_log_seq_stop,
    .show  = host_log_seq_show
};

static void* dsp_log_seq_start(struct seq_file *s, loff_t *pos)
{
    if (p_dsp_info_buf == NULL)
    {
        return NULL;
    }

    dsp_log_index = p_dsp_info_buf->dsp_log_index;
    dsp_log_size = p_dsp_info_buf->dsp_log_size;
    dsp_log_head = p_dsp_info_buf->dsp_log_head;
    dsp_log_tail = p_dsp_info_buf->dsp_log_tail;
		
    if (dsp_log_size == 0)
    {
        return NULL;
    }
			
    if ((*pos) == 0)
    {
        /* seq_printf(s, "head = %d, tail=%d, size=%d, index=%d\n",
            dsp_log_head,
            dsp_log_tail,
            dsp_log_size,
            dsp_log_index); */
        *pos = dsp_log_index;
        return (void *)(&(p_dsp_info_buf->dsp_log_buf[0]) + dsp_log_index * DSP_LOG_LINE_SIZE);
    }

    return NULL; 
}

static void* dsp_log_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    int n = *pos;

    if (n < dsp_log_index)
    {
        n += DSP_LOG_MAX_LINE;
    }

    if (n - dsp_log_index + 1 >= dsp_log_size)
    {
        return NULL;			
    }

    (*pos)++;

    if ((*pos) >= DSP_LOG_MAX_LINE)
    {
        *pos = 0;
    }

    return (void *)(&(p_dsp_info_buf->dsp_log_buf[0]) + (*pos) * DSP_LOG_LINE_SIZE);
}

static void dsp_log_seq_stop(struct seq_file *s, void *v)
{

}

static int dsp_log_seq_show(struct seq_file *s, void *v)
{
    seq_printf(s, "%s", (char *)v); 
    return 0;
}

static struct seq_operations dsp_log_seq_ops = {
    .start = dsp_log_seq_start,
    .next  = dsp_log_seq_next,
    .stop  = dsp_log_seq_stop,
    .show  = dsp_log_seq_show
};

static int proc_host_log_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &host_log_seq_ops);
}

static struct file_operations proc_host_log_ops = {
    .owner   = THIS_MODULE, // system
    .open    = proc_host_log_open,
    .read    = seq_read,    // system
    .llseek  = seq_lseek,   // system
    .release = seq_release  // system
};

static int proc_dsp_log_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &dsp_log_seq_ops);
}

static struct file_operations proc_dsp_log_ops = {
    .owner   = THIS_MODULE, // system
    .open    = proc_dsp_log_open,
    .read    = seq_read,    // system
    .llseek  = seq_lseek,   // system
    .release = seq_release  // system
};

static int proc_reg_show(struct seq_file *m, void *v)
{
    seq_printf(m, "ceva_sys0 regs:\n");
    seq_printf(m, "0x00 ~ 0x07: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n",
            REGR(p_dev_data->ceva_hal.base_sys, 0x00),
            REGR(p_dev_data->ceva_hal.base_sys, 0x01),
            REGR(p_dev_data->ceva_hal.base_sys, 0x02),
            REGR(p_dev_data->ceva_hal.base_sys, 0x03),
            REGR(p_dev_data->ceva_hal.base_sys, 0x04),
            REGR(p_dev_data->ceva_hal.base_sys, 0x05),
            REGR(p_dev_data->ceva_hal.base_sys, 0x06),
            REGR(p_dev_data->ceva_hal.base_sys, 0x07)
    );

    seq_printf(m, "0x08 ~ 0x0F: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n",
            REGR(p_dev_data->ceva_hal.base_sys, 0x08),
            REGR(p_dev_data->ceva_hal.base_sys, 0x09),
            REGR(p_dev_data->ceva_hal.base_sys, 0x0A),
            REGR(p_dev_data->ceva_hal.base_sys, 0x0B),
            REGR(p_dev_data->ceva_hal.base_sys, 0x0C),
            REGR(p_dev_data->ceva_hal.base_sys, 0x0D),
            REGR(p_dev_data->ceva_hal.base_sys, 0x0E),
            REGR(p_dev_data->ceva_hal.base_sys, 0x0F)
    );

    seq_printf(m, "0x10 ~ 0x17: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n",
            REGR(p_dev_data->ceva_hal.base_sys, 0x10),
            REGR(p_dev_data->ceva_hal.base_sys, 0x11),
            REGR(p_dev_data->ceva_hal.base_sys, 0x12),
            REGR(p_dev_data->ceva_hal.base_sys, 0x13),
            REGR(p_dev_data->ceva_hal.base_sys, 0x14),
            REGR(p_dev_data->ceva_hal.base_sys, 0x15),
            REGR(p_dev_data->ceva_hal.base_sys, 0x16),
            REGR(p_dev_data->ceva_hal.base_sys, 0x17)
    );

    seq_printf(m, "0x18 ~ 0x1F: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n",
            REGR(p_dev_data->ceva_hal.base_sys, 0x18),
            REGR(p_dev_data->ceva_hal.base_sys, 0x19),
            REGR(p_dev_data->ceva_hal.base_sys, 0x1A),
            REGR(p_dev_data->ceva_hal.base_sys, 0x1B),
            REGR(p_dev_data->ceva_hal.base_sys, 0x1C),
            REGR(p_dev_data->ceva_hal.base_sys, 0x1D),
            REGR(p_dev_data->ceva_hal.base_sys, 0x1E),
            REGR(p_dev_data->ceva_hal.base_sys, 0x1F)
    );

    seq_printf(m, "0x20 ~ 0x22: 0x%04x 0x%04x 0x%04x\n",
            REGR(p_dev_data->ceva_hal.base_sys, 0x20),
            REGR(p_dev_data->ceva_hal.base_sys, 0x21),
            REGR(p_dev_data->ceva_hal.base_sys, 0x22)
    );

    seq_printf(m, "0x40 ~ 0x44: 0x%04x 0x%04x 0x%04x 0x%04x 0x%04x\n",
            REGR(p_dev_data->ceva_hal.base_sys, 0x40),
            REGR(p_dev_data->ceva_hal.base_sys, 0x41),
            REGR(p_dev_data->ceva_hal.base_sys, 0x42),
            REGR(p_dev_data->ceva_hal.base_sys, 0x43),
            REGR(p_dev_data->ceva_hal.base_sys, 0x44)
    );

    seq_printf(m, "0x60 ~ 0x63: 0x%04x 0x%04x 0x%04x 0x%04x\n",
            REGR(p_dev_data->ceva_hal.base_sys, 0x60),
            REGR(p_dev_data->ceva_hal.base_sys, 0x61),
            REGR(p_dev_data->ceva_hal.base_sys, 0x62),
            REGR(p_dev_data->ceva_hal.base_sys, 0x63)
    );
    return 0;
}

static int proc_reg_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_reg_show, NULL);
}

static struct file_operations proc_reg_ops = {
    .owner   = THIS_MODULE, // system
    .open    = proc_reg_open,
    .read    = seq_read,    // system
    .llseek  = seq_lseek,   // system
    .release = seq_release  // system
};

int host_log_init(void)
{
    if (host_log_inited == true)
    {
        return 0;
    }

    p_host_log_buf = kmalloc(HOST_LOG_BUF_MEM_SIZE, GFP_KERNEL);
    if (p_host_log_buf == NULL)
    {
        return -ENOMEM;
    }

    memset(p_host_log_buf, 0, HOST_LOG_BUF_MEM_SIZE);
    spin_lock_init(&host_log_lock);
    host_log_inited = true;

    return 0;
}

void host_log_add(const char *fmt, ...)
{
    char buf[HOST_LOG_LINE_SIZE];
    va_list args;
    unsigned long flags;

    if (p_host_log_buf == NULL)
    {
        return;
    }

    spin_lock_irqsave(&host_log_lock, flags);

    va_start(args, fmt);
    vsnprintf(buf, HOST_LOG_LINE_SIZE, fmt, args);
    va_end(args);
    
    buf[HOST_LOG_LINE_SIZE - 2] = '\n';
    buf[HOST_LOG_LINE_SIZE - 1] = '\0';
    memcpy((void *)(p_host_log_buf + host_log_head * HOST_LOG_LINE_SIZE), (void *)buf, HOST_LOG_LINE_SIZE);

    host_log_head++;
    if (host_log_head >= HOST_LOG_MAX_LINE)
    {
        host_log_head = 0;
    }

    host_log_tail = host_log_head + HOST_LOG_MAX_LINE - 1;
    if (host_log_tail >= HOST_LOG_MAX_LINE)
    {
        host_log_tail = host_log_tail - HOST_LOG_MAX_LINE;
    }

    host_log_size++;
    if (host_log_size >= HOST_LOG_MAX_LINE)
    {
        host_log_size = HOST_LOG_MAX_LINE;
    }

    if (host_log_tail >= host_log_size - 1)
    {
        host_log_index = host_log_tail - host_log_size + 1;
    }
    else
    {
        host_log_index = host_log_tail + HOST_LOG_MAX_LINE - host_log_size + 1;
    }

    spin_unlock_irqrestore(&host_log_lock, flags);
}

int dsp_log_init(phys_addr_t phy_addr, u32 size)
{
    void *virt_addr;

    if (dsp_log_inited == true)
    {
        return 0;
    }
	
    if (request_mem_region(phy_addr, size, "dsp_log_buf") == NULL)
    {
        return -ENOMEM;
    }

    virt_addr = ioremap_nocache(phy_addr, size);
    if (virt_addr == NULL)
    {
        return -ENOMEM;
    }

    p_dsp_info_buf = (dsp_info_buf *)(virt_addr);
    memset(p_dsp_info_buf, 0, size);
    Chip_Flush_MIU_Pipe();
    dsp_log_inited = true;

    return 0;
}

int ceva_linkdrv_proc_init(struct xm6_dev_data *p_data)
{
    struct proc_dir_entry *pde;
    
    proc_dir = proc_mkdir("ceva_link", NULL);
    if (!proc_dir)
    {
        return -1;
    }

    pde = proc_create("host_log", S_IRUGO, proc_dir, &proc_host_log_ops);
    if (!pde)
    {
        goto out_ceva_link;
    }

    pde = proc_create("dsp_log", S_IRUGO, proc_dir, &proc_dsp_log_ops);
    if (!pde)
    {
        goto out_host_log;
    }

    pde = proc_create("reg", S_IRUGO, proc_dir, &proc_reg_ops);
    if (!pde)
    {
        goto out_dsp_log;
    }

    p_dev_data = p_data;

    return 0;
    
out_dsp_log:
    remove_proc_entry("dsp_log", proc_dir);
out_host_log:
    remove_proc_entry("host_log", proc_dir);
out_ceva_link:
	remove_proc_entry("ceva_link", NULL);

    return -1;    
}

void ceva_linkdrv_proc_exit(void)
{
    remove_proc_entry("reg", proc_dir);
    remove_proc_entry("dsp_log", proc_dir);
    remove_proc_entry("host_log", proc_dir);
	remove_proc_entry("ceva_link", NULL);
}
