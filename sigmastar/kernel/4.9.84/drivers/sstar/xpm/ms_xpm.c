/*
* ms_xpm.c- Sigmastar
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
#include <asm/bug.h>
#include <asm/page.h>
#include <asm/string.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/ioctl.h>
#include <asm-generic/poll.h>
#include <linux/compiler.h>
#include <linux/delay.h>
#include <linux/export.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/init.h>
#include <linux/kern_levels.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/stddef.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include <mdrv_xpm_io.h>
#include <mdrv_xpm_io_st.h>

#define USE_WAKEUP_SOURCE			0

#define XPM_DEBUG        			1
#define MINOR_XPM_NUM               129

#if XPM_DEBUG
#define PRINT(fmt, args...)     printk("[XPM] " fmt, ## args)
#else
#define PRINT(fmt, args...)
#endif


#define XPM_NAME_SIZE				16
#define XPM_MIN_SUSPEND_WAIT_MS		20

static int xpm_open(struct inode *inode, struct file *filp);
static int xpm_release(struct inode *inode, struct file *filp);
static long xpm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
static ssize_t xpm_read(struct file *filp, char __user *buf, size_t sz, loff_t *off);
static ssize_t xpm_write(struct file *filp, const char __user *buf, size_t sz, loff_t *off);
static unsigned int xpm_poll(struct file *filp, struct poll_table_struct *wait);
static int xpm_deregister_source(struct file *filp);

static struct file_operations xpm_fops = {
    .owner = THIS_MODULE,
    .open = xpm_open,
    .release = xpm_release,
	.write = xpm_write,
	.read = xpm_read,
	.poll = xpm_poll,
    .unlocked_ioctl=xpm_ioctl,
};


static struct miscdevice xpm_dev = {
    .minor      = MINOR_XPM_NUM,
    .name       = "xpm",
    .fops       = &xpm_fops,

};

typedef struct
{
	char name[XPM_NAME_SIZE];
    unsigned int option; //reserved
    EN_XPM_SOURCE_STATUS status;
    struct file* fp;
    EN_XPM_STATE poll_state;
#if USE_WAKEUP_SOURCE
    struct wakeup_source *wake;
#endif
} XPM_SOURCE;

struct XPM_SOURCE_LIST
{

	XPM_SOURCE source;
	struct list_head list;

};

typedef struct
{
	EN_XPM_STATE state;
	struct mutex mutex;
	wait_queue_head_t wq;
} XPM_MANAGER;

static XPM_MANAGER xpmm;
static struct list_head kept_source_list_head;

static int suspend_wait_time=500; //ms


//static void *mm_mem_virt = NULL; /* virtual address of frame buffer 1 */



static int source_registered(XPM_SOURCE *src)
{
	return (strlen(src->name)>0);
}

EN_XPM_STATE xpm_get_state(void)
{

	return xpmm.state;
}


static int xpm_open(struct inode *inode, struct file *filp)
{

	int err=0;
	struct XPM_SOURCE_LIST *new=(struct XPM_SOURCE_LIST *)kmalloc(sizeof(struct XPM_SOURCE_LIST),GFP_KERNEL);
	if(!new){
		err=-ENOMEM;
		goto BEACH;
	}
	new->source.fp=NULL;

	mutex_lock(&xpmm.mutex);

	if(!(xpm_get_state()&XPM_STATE_WAKEUPED))
	{
		err=-EBUSY;
		goto BEACH;
	}

	memset(new->source.name,0,sizeof(new->source.name));

	list_add(&new->list, &kept_source_list_head);

	new->source.fp=filp;
	new->source.poll_state=XPM_STATE_WAKEUPED;
	filp->private_data=new;


BEACH:
	mutex_unlock(&xpmm.mutex);

	if(err==-ENOMEM)
	{
		printk(KERN_ERR "ERROR!! Failed to allocate memory for xpm source\n" ) ;
	}

	if(0==err)
	{
		PRINT("new xpm source: 0x%08X\n",(unsigned int)new->source.fp);
	}

    return err;
}

static int xpm_release(struct inode *inode, struct file *filp)
{
    struct XPM_SOURCE_LIST *entry;
    int err=0;

	mutex_lock(&xpmm.mutex);


	entry=(struct XPM_SOURCE_LIST *)filp->private_data;
	if(entry!=NULL)
	{
		list_del_init(&entry->list);
	}

	err=xpm_deregister_source(filp);
	if(err) goto BEACH;

	entry->source.fp=NULL;
	filp->private_data=NULL;

	kfree(entry);


BEACH:
	mutex_unlock(&xpmm.mutex);
	if(0==err)
	{
		PRINT("release XPM source: 0x%08X\n",(unsigned int)filp);
	}


    return err;
}



static int xpm_register_source(char* name,struct file *filp)
{

	struct list_head *ptr;
	struct XPM_SOURCE_LIST *entry,*match_entry=NULL;
	int err=0;

//	mutex_lock(&xpm.mutex);
	if(!(xpm_get_state()&XPM_STATE_WAKEUPED))
	{
		err=-EBUSY;
		goto BEACH_ERROR;
	}

	list_for_each(ptr, &kept_source_list_head)
	{
		entry = list_entry(ptr, struct XPM_SOURCE_LIST, list);
		if (0==strcmp(name,entry->source.name))
		{
			err=-1;
			PRINT("ERROR!! XPM source name:[%s] already registered\n",name);
			goto BEACH_ERROR;
		}
	}


	list_for_each(ptr, &kept_source_list_head)
	{
		entry = list_entry(ptr, struct XPM_SOURCE_LIST, list);
		if (entry->source.fp==filp)
		{
			match_entry=entry;
			break;;
		}
	}

	if(match_entry==NULL)
	{
		err=-1;
		PRINT("WARNING!! no XPM source:0x%08X found\n",(unsigned int)filp);
		goto BEACH_ERROR;
	}

	if(strlen(match_entry->source.name)>0)
	{
		err=-2;
		PRINT("WARNING!! XPM source:0x%08X already registered with name [%s]\n",(unsigned int)filp,match_entry->source.name);
		goto BEACH_ERROR;
	}

	strncpy(match_entry->source.name,name,(XPM_NAME_SIZE-1));
	match_entry->source.status=XPM_SOURCE_ACTIVE;

	PRINT("successfully register XPM source: 0x%08X[%s]\n",(unsigned int)match_entry->source.fp,match_entry->source.name);

BEACH_ERROR:
//	mutex_unlock(&xpm.mutex);
	return err;

}

static int xpm_deregister_source(struct file *filp)
{

	struct list_head *ptr;
	struct XPM_SOURCE_LIST *entry,*match_entry=NULL;
	int err=0;

//	mutex_lock(&xpm.mutex);
	list_for_each(ptr, &kept_source_list_head)
	{
		entry = list_entry(ptr, struct XPM_SOURCE_LIST, list);
		if (entry->source.fp==filp)
		{
			match_entry=entry;
			break;;
		}
	}

	if(match_entry==NULL)
	{
		err=-1;
		PRINT("WARNING!! no XPM source:0x%08X found\n",(unsigned int)filp);
		goto BEACH_ERROR;
	}

	memset(match_entry->source.name,0,sizeof(match_entry->source.name));

	PRINT("successfully deregister XPM source: 0x%08X\n",(unsigned int)match_entry->source.fp);

BEACH_ERROR:
//	mutex_unlock(&xpm.mutex);
	return err;

}
static long xpm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int err= 0;

	// extract the type and number bitfields, and don��t decode
	// wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	if (_IOC_TYPE(cmd) != XPM_IOCTL_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > IOCTL_XPM_MAXNR) return -ENOTTY;
    switch(cmd)
    {

    	case IOCTL_XPM_REGISTER_SOURCE:
    	{
			char _name[XPM_NAME_SIZE]={'\0'};
			if( copy_from_user((void*)_name, (void __user *)arg, (XPM_NAME_SIZE>strlen((char *)arg))?  (strlen((char *)arg)+1): XPM_NAME_SIZE-1 ) )
            {
				BUG();

			}
			if(strlen(_name)>0)
			{
				mutex_lock(&xpmm.mutex);
				err=xpm_register_source(_name,filp);
				mutex_unlock(&xpmm.mutex);
			}

    	}
    	break;

    	case IOCTL_XPM_DEREGISTER_SOURCE:
		{
			mutex_lock(&xpmm.mutex);
			err=xpm_deregister_source(filp);
			mutex_unlock(&xpmm.mutex);

		}
		break;

		default:
			PRINT("ERROR!! unknown IOCTL command 0x%08X\n", cmd);
			return -ENOTTY;
    }


	return err;
}

//static int mpm_supend_count=0;
//static int mpm_supend_task_delay=0;
//static int suspend_retry_count=0;
//
//
//static void _mpm_suspend_to_ram_task()
//{
//
//}




void xpm_change_state(int state)
{


	xpmm.state=state;
	if(XPM_STATE_SUSPENDED != state)
	{
		wake_up_interruptible(&xpmm.wq);
	}

}

static ssize_t xpm_read(struct file *filp, char __user *buf, size_t sz, loff_t *off)
{

	if(sz<sizeof(xpmm.state))
	{
		return -EINVAL;
	}

	if( copy_to_user(buf,&xpmm.state,sizeof(xpmm.state)) )
	{
		BUG();
	}

	return sizeof(xpmm.state);
}

static ssize_t xpm_write(struct file *filp, const char __user *buf, size_t sz, loff_t *off)
{

	int cmd;
	int err=0;//;(cmd);

	if(sz<sizeof(cmd))
	{
		return -EINVAL;
	}


	memcpy(&cmd,buf,sizeof(cmd));

	switch((EN_XPM_SOURCE_STATUS)cmd)
	{
		case XPM_SOURCE_STANDBY:
		{


			XPM_SOURCE *source=&((struct XPM_SOURCE_LIST*)filp->private_data)->source;

		    if(!source_registered(source))
		    {
		    	PRINT("WANRING!! no effect on unregistered XPM source: 0x%08X\n",(unsigned int)filp);
		    	break;
		    }

		    source->status=XPM_SOURCE_STANDBY;
			PRINT("XPM source: 0x%08X[%s] STANDBY done\n",(unsigned int)source->fp,source->name);
		}
		break;



		default:
			PRINT("unknown XPM source command: 0x%08X\n",cmd);
	}

//BEACH:
	return err;
}



static unsigned int xpm_poll(struct file *filp, struct poll_table_struct *wait)
{

	EN_XPM_STATE st=XPM_STATE_END;
	unsigned int mask = 0;
	XPM_SOURCE *source=&((struct XPM_SOURCE_LIST*)filp->private_data)->source;

	if(!source_registered(source))
	{
		PRINT("ERROR!! can not poll unregistered XPM source: 0x%08X\n",(unsigned int)filp);
		return POLLNVAL;
	}


	poll_wait(filp, &xpmm.wq,  wait);
	st=xpm_get_state();
	if(( st == XPM_STATE_SUSPENDING) && (source->poll_state!=XPM_STATE_SUSPENDING))
	{
		PRINT("POLLIN XPM source: 0x%08X[%s][XPM_STATE_SUSPENDING]\n",(unsigned int)source->fp,source->name);
		source->poll_state=XPM_STATE_SUSPENDING;
		mask = POLLIN | POLLWRNORM;
	}
	else if(( st == XPM_STATE_WAKEUPED) && (source->poll_state!=XPM_STATE_WAKEUPED))
	{
		PRINT("POLLIN XPM source: 0x%08X[%s][XPM_STATE_WAKEUPED]\n",(unsigned int)source->fp,source->name);
		source->poll_state=XPM_STATE_WAKEUPED;
		mask = POLLIN | POLLWRNORM;
	}
	else if(( st == (XPM_STATE_WAKEUPED|XPM_STATE_SUSPEND_ABORT)) && (source->poll_state!=(XPM_STATE_WAKEUPED|XPM_STATE_SUSPEND_ABORT)))
	{
		PRINT("POLLIN XPM source: 0x%08X[%s][XPM_STATE_WAKEUPED]\n",(unsigned int)source->fp,source->name);
		source->poll_state=(XPM_STATE_WAKEUPED|XPM_STATE_SUSPEND_ABORT);
		mask = POLLIN | POLLWRNORM;
	}

	return mask;
}

int xpm_prepare_suspend(void)
{
	int t=0;
	int err=0;
//	int xpm_source_count=xpm_get_registered_source_count();


	mutex_lock(&xpmm.mutex);
	xpm_change_state(XPM_STATE_SUSPENDING);
	while(true)
	{
		struct list_head *ptr;
		struct XPM_SOURCE_LIST *entry;
		int active_source_count=0;

		msleep(XPM_MIN_SUSPEND_WAIT_MS);
		t+=XPM_MIN_SUSPEND_WAIT_MS;

		list_for_each(ptr, &kept_source_list_head)
		{
			entry = list_entry(ptr, struct XPM_SOURCE_LIST, list);
			if(source_registered(&entry->source) && entry->source.status != XPM_SOURCE_STANDBY)
			{
				active_source_count++;
				if(t>suspend_wait_time)
				{
					xpm_change_state(XPM_STATE_SUSPEND_ABORT|XPM_STATE_WAKEUPED);
					PRINT("XPM suspend aborted!! XPM source: 0x%08X[%s]\n",(unsigned int)entry->source.fp,entry->source.name);
					err=-EBUSY;
					goto BEACH;
				}

				break;
			}
		}

		if(0==active_source_count)
		{
			break;
		}
	}


BEACH:

	if(0==err)
	{
		PRINT("XPM suspend\n");
		xpm_change_state(XPM_STATE_SUSPENDED);
	}

	mutex_unlock(&xpmm.mutex);
	return err;
}
EXPORT_SYMBOL(xpm_prepare_suspend);


int xpm_notify_wakeup(void)
{
	struct list_head *ptr;
	struct XPM_SOURCE_LIST *entry;

	mutex_lock(&xpmm.mutex);
	xpm_change_state(XPM_STATE_WAKEUPED);

	list_for_each(ptr, &kept_source_list_head)
	{
		entry = list_entry(ptr, struct XPM_SOURCE_LIST, list);
		if(source_registered(&entry->source))
		{
			entry->source.status = XPM_SOURCE_ACTIVE;
		}
	}

	mutex_unlock(&xpmm.mutex);
	PRINT("XPM wakeuped\n");
	return 0;
}

EXPORT_SYMBOL(xpm_notify_wakeup);

ssize_t xpm_show(struct kobject *kobj,struct kobj_attribute *attr, char *buf)
{

	char *str = buf;
	char *end = buf + PAGE_SIZE;
	struct list_head *ptr;
	struct XPM_SOURCE_LIST *entry;
	int i=0;

	str += scnprintf(str, end - str, "XPM state: 0x%02X\n", xpm_get_state());

	list_for_each(ptr, &kept_source_list_head)
	{
		XPM_SOURCE *source;
		entry = list_entry(ptr, struct XPM_SOURCE_LIST, list);
		source=(XPM_SOURCE *)(&entry->source);
		str += scnprintf(str, end - str, "%04d: 0x%08X[%s], 0x%02X \n",i,(unsigned int)source->fp,source->name,source->status);
		i++;
	}

	if (str > buf)
		str--;

	str += scnprintf(str, end - str, "\n");

	return (str - buf);
}

EXPORT_SYMBOL(xpm_show);

ssize_t xpm_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t n)
{
	int err=0;
	long st;


	if(kstrtoul(buf,10,&st))
	{
		return -EINVAL;
	}


	if((st!=(long)XPM_STATE_SUSPENDING) && (st!=(long)XPM_STATE_WAKEUPED) && (st!=(long)(XPM_STATE_WAKEUPED|XPM_STATE_SUSPEND_ABORT)) )
	{
		printk(KERN_ERR"Unsupport XPM state:0x%02X\n",(unsigned int)st);
		err=-EINVAL;
	}
	else
	{
		if(st==(long)XPM_STATE_SUSPENDING)
		{
			xpm_prepare_suspend();
		}
		else
		{
			xpm_notify_wakeup();
		}
	}

	return err ? err : n;
}
EXPORT_SYMBOL(xpm_store);


ssize_t xpm_suspend_wait_ms_show(struct kobject *kobj,struct kobj_attribute *attr, char *buf)
{

	char *str = buf;
	char *end = buf + PAGE_SIZE;

	str += scnprintf(str, end - str, "%d\n", suspend_wait_time);

	return (str - buf);
}

EXPORT_SYMBOL(xpm_suspend_wait_ms_show);

ssize_t xpm_suspend_wait_ms_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t n)
{
	long st;

	if(kstrtoul(buf,10,&st))
	{
		return 0;
    }
	suspend_wait_time=(st<XPM_MIN_SUSPEND_WAIT_MS)?XPM_MIN_SUSPEND_WAIT_MS:st;

	return n;
}
EXPORT_SYMBOL(xpm_suspend_wait_ms_store);



static int __init xpm_init(void)
{
    int ret;

    ret = misc_register(&xpm_dev);
    if (ret != 0) {
        printk("cannot register XPM miscdev (err=%d)\n",ret);
    }

    INIT_LIST_HEAD(&kept_source_list_head);
    init_waitqueue_head(&xpmm.wq);
    mutex_init(&xpmm.mutex);
    xpmm.state=XPM_STATE_WAKEUPED;

    return 0;
}


subsys_initcall(xpm_init);

MODULE_AUTHOR("SSTAR");
MODULE_DESCRIPTION("XPM driver");
MODULE_LICENSE("MSTAR");
