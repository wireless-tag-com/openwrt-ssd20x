/*
* reboot_state.c- Sigmastar
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
#include <linux/irq.h>
#include <linux/gfp.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/time.h>
#include "mdrv_types.h"

#define BUFFER_LEN  0x40
static char reboot_type[BUFFER_LEN];
static char reboot_time[BUFFER_LEN];
static struct proc_dir_entry *proc_reboot_state_dir;

static U16 mstar_read_reg16( u32 bank, u32 reg)
{
	u16 val = 0; 
	
    u32 address = 0xfd000000 + bank*0x100*2 + (reg << 2);
    val = *( ( volatile u16* ) address );
	
	return val;
}

static int reboot_type_open(struct inode *inode, struct file *fp)
{
	memset(reboot_type, 0, BUFFER_LEN);
    return 0;
}

// 1: watchdog reboot
// 2: reboot
// 3: ac on
static ssize_t reboot_type_read(struct file *fp, char * buf, size_t size, loff_t *ppos)
{
	U16 RegVal = 0;
	
	if (!size || *ppos != 0)
        return 0; 

	RegVal = mstar_read_reg16(0x30,0x02);
	if(RegVal&0x01)
	{
		memcpy(reboot_type,"1",1);
	}
	else
	{
		RegVal = mstar_read_reg16(0x06,0x0);
		if(RegVal&0x01)
		    memcpy(reboot_type,"2",1);
		else
			memcpy(reboot_type,"3",1);
	}
	
	if (copy_to_user(buf, reboot_type, 1))
        return -EFAULT;

    *ppos += 1;

    return 1;
}

static int reboot_type_release(struct inode *inode, struct file *fp)
{
    printk("\n");
    return 0;
}

static int reboot_time_open(struct inode *inode, struct file *fp)
{
	memset(reboot_time, 0, BUFFER_LEN);
    return 0;
}

static ssize_t reboot_time_read(struct file *fp, char * buf, size_t size, loff_t *ppos)
{
	int len = 0;
	U16 RegVal = 0;
	struct tm tlocal;
	
	if (!size || *ppos != 0)
        return 0; 

    RegVal = mstar_read_reg16(0x06,0x1);
    tlocal.tm_year = RegVal;
	
	RegVal = mstar_read_reg16(0x06,0x2);
    tlocal.tm_mon = (RegVal&0xff00) >> 8;
    tlocal.tm_mday = RegVal&0xff;

    RegVal = mstar_read_reg16(0x06,0x3);
    tlocal.tm_hour = RegVal;

	RegVal = mstar_read_reg16(0x06,0x4);
    tlocal.tm_min = (RegVal&0xff00) >> 8;
    tlocal.tm_sec = RegVal&0xff;

    len = sprintf(reboot_time+len, "%ld-%d-%d %d:%d:%d", 
		tlocal.tm_year, tlocal.tm_mon, tlocal.tm_mday, tlocal.tm_hour, tlocal.tm_min, tlocal.tm_sec);
	
    *ppos += len;
	
	if (copy_to_user(buf, reboot_time, len))
        return -EFAULT;

    return len;
}

static int reboot_time_release(struct inode *inode, struct file *fp)
{
    printk("\n");
    return 0;
}

static const struct file_operations proc_reboot_type_operations = {
    .open    = reboot_type_open,
    .read    = reboot_type_read,
    .release = reboot_type_release,
};

static const struct file_operations proc_reboot_time_operations = {
    .open    = reboot_time_open,
    .read    = reboot_time_read,
    .release = reboot_time_release,
};

static int __init proc_reboot_state_init(void)
{
	proc_reboot_state_dir = proc_mkdir("reboot_state", NULL);
	proc_create("type", S_IRUSR | S_IWUSR, proc_reboot_state_dir,&proc_reboot_type_operations);
	proc_create("time", S_IRUSR | S_IWUSR, proc_reboot_state_dir,&proc_reboot_time_operations);
    return 0;
}

module_init(proc_reboot_state_init);

