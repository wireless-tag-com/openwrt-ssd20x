/*
* ceva_linkdrv.c- Sigmastar
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
 * ceva_linkdrv.c
 *
 * Created on: Nov 11, 2013
 * Author: Ido Reis <ido.reis@tandemg.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <linux/fs.h>
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/stat.h>
#include <linux/mm.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/processor.h>

#include "ceva_linkdrv-generic.h"
#include "ceva_linkdrv_xm6.h"
#include "ceva_linkdrv.h"

#include "drv_debug.h"

MODULE_AUTHOR("Ido Reis <ido.reis@tandemg.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.1");
MODULE_DESCRIPTION("cevalink logical driver");

#define MODULE_NAME                    "ceva_linkdrv"
#define LINKDRV_PFD_PROC_NAME_MAX_SIZE (16)
#define CEVA_LINKDRV_PROC_EVENTS_NAME  "events"
#define CANCEL_READ_MAX_RELAX_TRIES    (100)
#define UNBLOCKED_BY_USER              ((CEVA_LINKDRV_FIFO_MAX_EVENTS+1)*(-1))

/*!
 * container class for ceva_linkdrv's operations for a specific process
 */
struct ceva_linkdrv_pfd {
	struct ceva_linkdrv *owner; /*!< ceva_linkdrv object that owns this obj */
	struct list_head list_node; /*!< list_node for maintenance */
	pid_t pid; /*!< process id that has opened the device */
	struct proc_dir_entry *dir; /*!< root dir for procfs entry */
	struct proc_dir_entry *events; /*!< procfs entry dedicated to process */
	DECLARE_KFIFO_PTR(fifo, ceva_event_t); /*!< kfifo to receive events */
	int *blocked; /*!< used to wakeup by workqueue */
	int events_ref_count; /*!< ref count for open */
};

static int _ceva_linkdrv_proc_open(struct inode *, struct file *);
static int _ceva_linkdrv_proc_release(struct inode *, struct file *);
static ssize_t _ceva_linkdrv_proc_read(struct file *, char __user *,
		size_t, loff_t *);
static int _ceva_linkdrv_proc_read_break(struct ceva_linkdrv *link);
static struct ceva_linkdrv_pfd *_get_linkdrv_pfd(struct ceva_linkdrv *link,
		pid_t pid);
static struct ceva_linkdrv_pfd *_create_linkdrv_pfd(struct ceva_linkdrv *link,
		pid_t pid);
static void _destroy_linkdrv_pfd(struct ceva_linkdrv_pfd *pfd);

static int _ceva_link_debug_proc_init(struct ceva_linkdrv *link);
static void _ceva_link_debug_proc_deinit(void);

/*!
 * procfs fops
 */
static struct file_operations ceva_linkdrv_proc_events_fops = {
	.owner = THIS_MODULE,
	.read = _ceva_linkdrv_proc_read,
	.open = _ceva_linkdrv_proc_open,
	.release = _ceva_linkdrv_proc_release,
};

/*!
 * handler for procfs directory (general)
 * @see ceva_linkdrv_init()
 */
static struct proc_dir_entry* ceva_linkdrv_proc_dir = NULL;
extern void Dump_ARMDSP_HndShakeInfo(void);
#define DEFAULT_TIMESLOT 200
unsigned int DSPTimeOutSlot=DEFAULT_TIMESLOT;

void SetUpDSPTimeSlot(unsigned int SetValue)
{
	DSPTimeOutSlot=(SetValue==0)?DEFAULT_TIMESLOT:SetValue;
	printk("SetUpDSPTimeSlot  DSPTimeOutSlot:%d \n",DSPTimeOutSlot);
	return;
}

/*!
 * search for existing pfd within the link's database (helper function)
 *
 * the key for each pfd is it's process id.
 * this functions searches the list for a pfd that matches the pid
 * @param [in] link ceva_linkdrv object
 * @param [in] pid process id to be used as a key to search for a pfd
 * @return valid struct ceva_linkdrv_pfd address if found, NULL if not
 */
static struct ceva_linkdrv_pfd *_get_linkdrv_pfd(struct ceva_linkdrv *link,
		pid_t pid) {
	struct list_head *pos;
	struct ceva_linkdrv_pfd * i;
	list_for_each(pos, &link->pfd_list) {
		i = (struct ceva_linkdrv_pfd *) list_entry(pos,
				struct ceva_linkdrv_pfd, list_node);
		if (!i) {
			XM6_MSG(XM6_MSG_WRN, "%s -> iterator is null\n", __FUNCTION__);
			continue;
		}
		if (i->pid == pid) {
			return i;
		}
	}
	return NULL;
}

/*!
 * create a ceva_linkdrv_pfd object (helper function)
 *
 * this function allocates and creates a ceva_linkdrv_pfd object to be used
 * by a specific process. it creates a procfs entry based on the caller
 * process id.
 * the created object should be added to the link's pfd list for maintenance.
 * @param [in/out] link owner of this object
 * @param [in] pid id of the caller/owner process
 * @return valid ceva_linkdrv_pfd object for success, NULL for failure
 * @see ceva_linkdrv_open_cb(), _destroy_linkdrv_pfd()
 */
static struct ceva_linkdrv_pfd *_create_linkdrv_pfd(struct ceva_linkdrv *link,
		pid_t pid) {
	struct ceva_linkdrv_pfd* pfd;
	char proc_name[LINKDRV_PFD_PROC_NAME_MAX_SIZE];

	if (!link) {
		return NULL;
	}
	pfd = kzalloc(sizeof(struct ceva_linkdrv_pfd), GFP_KERNEL);
	if (!pfd) {
		goto err_alloc;
	}
	if (kfifo_alloc(&pfd->fifo, CEVA_LINKDRV_FIFO_MAX_EVENTS, GFP_KERNEL)) {
		goto err_fifo;
	}

	sprintf(proc_name, "%d", pid);
	/* create directory */
	pfd->dir = proc_mkdir(proc_name, ceva_linkdrv_proc_dir);
	if (pfd->dir == NULL) {
		XM6_MSG(XM6_MSG_ERR, "could not create procfs dir (%s)\n", proc_name);
		goto err_procfs_dir;
	}
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29))
	pfd->dir->owner = THIS_MODULE;
#endif /* KERNEL_VERSION(2,6,29) */
	XM6_MSG(XM6_MSG_DBG, "procfs dir for process %s created\n", proc_name);

	/* create procfs entry for events */
	pfd->events = proc_create_data(
					CEVA_LINKDRV_PROC_EVENTS_NAME,         /* name */
					S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH, /* mode */
					pfd->dir,                              /* parent */
					&ceva_linkdrv_proc_events_fops,        /* fops */
					pfd);                                  /* data */
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29))
	pfd->events->owner = THIS_MODULE;
#endif /* KERNEL_VERSION(2,6,29) */
	if (!pfd->events) {
		XM6_MSG(XM6_MSG_ERR, "could not create procfs entry\n");
		goto err_procfs_entry;
	}
	XM6_MSG(XM6_MSG_DBG, "created new procfs entry in /proc/%s/%s/%s\n",
			MODULE_NAME, proc_name,
			CEVA_LINKDRV_PROC_EVENTS_NAME);
	pfd->owner = link;
	pfd->pid = pid;
	pfd->blocked = NULL;
	pfd->events_ref_count = 0;
	INIT_LIST_HEAD(&pfd->list_node);
	return pfd;

err_procfs_entry:
	remove_proc_entry(proc_name, ceva_linkdrv_proc_dir);
err_procfs_dir:
	kfifo_free(&pfd->fifo);
err_fifo:
	kfree(pfd);
err_alloc:
	return NULL;
}

/*!
 * destroys a ceva_linkdrv_pfd object (helper function)
 *
 * this function deletes and frees a ceva_linkdrv_pfd object.
 * it deletes the procfs entry created for this object.
 * the created object should be removed from the link's pfd list
 * before executing this function.
 * @param [in] pfd object to delete
 * @see ceva_linkdrv_release_cb(), _create_linkdrv_pfd()
 */
static void _destroy_linkdrv_pfd(struct ceva_linkdrv_pfd *pfd) {
	char proc_name[LINKDRV_PFD_PROC_NAME_MAX_SIZE];

	if (!pfd) {
		return;
	}

	sprintf(proc_name, "%d", pfd->pid);

	XM6_MSG(XM6_MSG_DBG, "destorying pfd %p, for process %s\n", pfd, proc_name);
	remove_proc_entry(CEVA_LINKDRV_PROC_EVENTS_NAME, pfd->dir);
	remove_proc_entry(proc_name, ceva_linkdrv_proc_dir);
	kfifo_free(&pfd->fifo);
	pfd->owner = NULL;
	pfd->pid = -1;
	pfd->blocked = NULL;
	kfree(pfd);
	XM6_MSG(XM6_MSG_DBG, "destoryed");
}

/*!
 * unblock user read operation on a procfs entry (not irq safe)
 *
 * used to cancel the read during release_cb, or deinit of the module
 * @param [in] pfd ceva_linkdrv_pfd object
 * @return zero for success, nonzero for failure
 * @note blocking function, as this function puts the caller thread/process
 * to sleep to allow the read process to be unlocked
 */
static int _cancel_linkdrv_pfd_read(struct ceva_linkdrv_pfd *pfd) {

	int i;

	if (pfd->blocked) {
		*pfd->blocked = UNBLOCKED_BY_USER;
		wake_up_interruptible_all(&pfd->owner->wq);
		/*
		 * give up the cpu to allow the read execution to receive the
		 * event once the event will wake up the correct pfd it will reset
		 * the blocked variable to NULL and exits with EPIPE.
		 * other read executions might wake up, but it will continue to
		 * wait as the blocked variable is only reset to 'this' pfd
		 * blocked variable
		 */
		for (i = 0; pfd->blocked && i < CANCEL_READ_MAX_RELAX_TRIES; i++) {
			msleep(0);
		}
		if (i == CANCEL_READ_MAX_RELAX_TRIES) {
			XM6_MSG(XM6_MSG_ERR, "%s:: unable to break read execution of process %d\n",
					__FUNCTION__, pfd->pid);
			return -EBUSY;
		}
	}
	return 0;
}

/*!
 * unblock user read operation on a procfs entry (irq safe)
 *
 * used to cancel the read during release_cb, or deinit of the module
 * @param [in] pfd ceva_linkdrv_pfd object
 * @return zero for success, nonzero for failure
 * @note blocking function, as this function puts the caller thread/process
 * to sleep to allow the read process to be unlocked
 */
static int _cancel_linkdrv_pfd_read_irqsafe(struct ceva_linkdrv_pfd *pfd) {

	struct ceva_linkdrv *link = pfd->owner;
	unsigned long flags;
	int i;

	if(!link || !pfd){
		XM6_MSG(XM6_MSG_ERR, "%s  pfd:0x%p or link:0x%p is NULL\n",__FUNCTION__,pfd,link);
		return -EINVAL;
	}
	spin_lock_irqsave(&link->waiting_lock, flags);
	if (pfd->blocked) {
		*pfd->blocked = UNBLOCKED_BY_USER;
		spin_unlock_irqrestore(&link->waiting_lock, flags);
		wake_up_interruptible_all(&link->wq);
		/*
		 * see above comment in _cancel_linkdrv_pfd_read
		 */
		for (i = 0; pfd->blocked && i < CANCEL_READ_MAX_RELAX_TRIES; i++) {
			mdelay(1);
		}
		if(pfd->blocked == NULL){
			XM6_MSG(XM6_MSG_ERR, "%s pfd->blocked is NULL\n",__FUNCTION__);
			return 0;
		}
		else{
			XM6_MSG(XM6_MSG_ERR, "%s:: unable to break read execution of process %d pfd->blocked:0x%p \n",__FUNCTION__, pfd->pid,pfd->blocked);
			return -EBUSY;
		}
		if (i == CANCEL_READ_MAX_RELAX_TRIES) {
			XM6_MSG(XM6_MSG_ERR, "%s:: unable to break read execution of process %d\n",__FUNCTION__, pfd->pid);
			return -EBUSY;
		}
	} else {
		spin_unlock_irqrestore(&link->waiting_lock, flags);
	}
	return 0;
}

/*!
 * callback function for opening a ceva device
 *
 * called by the owner of this module (usually the PCIe device driver)
 * creates a ceva_linkdrv_pfd object (if the first time 'open' was called by
 * this process) and adds it to the maintained list.
 * @param [in] link 'this' module object
 * @return zero for success, nonzero for failure
 */
int ceva_linkdrv_open_cb(struct ceva_linkdrv *link) {
	struct ceva_linkdrv_pfd *pfd;
	pid_t pid = task_tgid_vnr(current);
	unsigned long flags;
	int ret = 0;

	if (mutex_lock_interruptible(&link->mux)) {
		ret = -EINTR;
		goto out;
	}
	if (_get_linkdrv_pfd(link, pid) != NULL) {
		XM6_MSG(XM6_MSG_WRN, "this process already opened the device\n");
		ret = -EBUSY;
		goto out;
	}
	pfd = _create_linkdrv_pfd(link, pid);
	if (!pfd) {
		ret = -ENOMEM;
		goto out;
	}
	spin_lock_irqsave(&link->waiting_lock, flags);
	list_add_tail(&pfd->list_node, &link->pfd_list);
	spin_unlock_irqrestore(&link->waiting_lock, flags);

out:
	mutex_unlock(&link->mux);
	return 0;
}

/*!
 * callback function for closing a ceva device
 *
 * called by the owner of this module (usualy the PCIe device driver)
 * destroys the ceva_linkdrv_pfd object (if the ref count reaches zero)
 * and removes it from the maintained list.
 * @param [in] link 'this' module object
 * @return zero for success, nonzero for failure
 */
int ceva_linkdrv_release_cb(struct ceva_linkdrv *link) {
	struct ceva_linkdrv_pfd *pfd;
	pid_t pid;
	unsigned long flags;
	int ret = 0;

	if (mutex_lock_interruptible(&link->mux)) {
		ret = -EINTR;
		goto out;
	}
	pid = task_tgid_vnr(current);
	pfd = _get_linkdrv_pfd(link, pid);
	if (pfd == NULL) {
		pid_t tid = task_pid_nr(current);
		XM6_MSG(XM6_MSG_ERR, "this process did not opened the device (p%d/t%d)\n", pid, tid);
		ret = -ESRCH;
		goto out;
	}

	if (pfd->events_ref_count) {
		XM6_MSG(XM6_MSG_DBG, "%s executed before the entry has been released, ref = %d\n",
				__FUNCTION__, pfd->events_ref_count);
	}

	// release read executions before removing from list
	if (_cancel_linkdrv_pfd_read(pfd)) {
		XM6_MSG(XM6_MSG_ERR, "unable to release %d/%s since read is still in progress\n",
				pid, CEVA_LINKDRV_PROC_EVENTS_NAME);
		return -EBUSY;
	}
	// remove from list
	spin_lock_irqsave(&link->waiting_lock, flags);
	list_del_init(&pfd->list_node);
	spin_unlock_irqrestore(&link->waiting_lock, flags);
	// destroy object
	_destroy_linkdrv_pfd(pfd);

out:
	mutex_unlock(&link->mux);
	return 0;
}

/*!
 * special commands executer
 *
 * @param [in] link 'this' module object
 * @param [in] cmd command to execute
 * @param [in] arg additional argument (differ for each command)
 * @return zero for success, nonzero for failure
 * @see IOC_CEVADRV_BYPASS_REQ, IOC_CEVADRV_READ_DEBUG
 */
long ceva_linkdrv_ioctl(struct ceva_linkdrv *link,
						unsigned int cmd,
						unsigned long arg) {
	long ret;
	void *buf;
	ceva_event_t event;
	unsigned int __user *ubuf = (unsigned int __user *) arg;
#if defined(CEVA_LINKDRV_DEBUG)
	char __user *cbuf = (char __user *) arg;
#endif // defined(CEVA_LINKDRV_DEBUG)

	switch (cmd) {
	case IOC_CEVADRV_BYPASS_REQ:
		buf = link->owner->dma_buf.cpu_addr;
		XM6_MSG(XM6_MSG_DBG, "BYPASS command received,"
			  " coping from offset %u to offset %u"
			  " and generate event %02x\n",
			  CEVADEV_BYPASS__OUT_BUF_OFFSET,
			  CEVADEV_BYPASS__IN_BUF_OFFSET,
			  CEVADEV_BYPASS__EVENT);
		memcpy(buf + CEVADEV_BYPASS__OUT_BUF_OFFSET,
			   buf + CEVADEV_BYPASS__IN_BUF_OFFSET,
			   CEVADEV_BYPASS__BUF_LEN);
		event = CEVADEV_BYPASS__EVENT;
		ret = ceva_linkdrv_broadcast_events(link, &event, 1);
		if (!ret) {
			DEBUG_INFO_INC(link, bypass_interrupts_success);
		} else {
			DEBUG_INFO_INC(link, bypass_interrupts_failed);
		}
		break;
	case IOC_CEVADRV_READ_DEBUG:
#if defined(CEVA_LINKDRV_DEBUG)
		ret = copy_to_user(cbuf, &link->debug.data,
				sizeof(struct ceva_linkdrv_debug_info_s));
#else
		ret = -EINVAL;
#endif
		break;
	case IOC_CEVADRV_GET_PID_PROCID: {
		pid_t pid = task_tgid_vnr(current);
		struct ceva_linkdrv_pfd *i = _get_linkdrv_pfd(link, pid);
		if (!i) {
			ret = -ESRCH;//ENODEV;
			break;
		}
		XM6_MSG(XM6_MSG_DBG, "found, copying id #%02d to user\n", i->pid);
		ret = copy_to_user(ubuf, &i->pid, sizeof(int));
		break;
	}
	default:
		ret = -EINVAL;
	}
	return ret;
}

/*!
 * consumer open callback
 *
 * keeps ref count for number of times the entry was opened
 * @param [in] inode
 * @param [in] filp ceva_linkdrv_pfd handler associated with the procfs
 * @return zero for success, nonzero for failure
 */
static int _ceva_linkdrv_proc_open(struct inode *inode, struct file *filp) {

	struct ceva_linkdrv_pfd *pfd = PDE_DATA(filp->f_path.dentry->d_inode);
	pid_t pid, tid;

	if (!pfd) {
		XM6_MSG(XM6_MSG_ERR, "%s :: null pfd descriptor\n", __FUNCTION__);
		return -EINVAL;
	}

	pid = task_tgid_vnr(current);
	tid = task_pid_nr(current);

	pfd->events_ref_count++;
    XM6_MSG(XM6_MSG_ERR, "%d/%s was opened (by p%d/t%d), open descriptor value: %d\n", pid, CEVA_LINKDRV_PROC_EVENTS_NAME, pid, tid, pfd->events_ref_count);

	return 0;
}

/*!
 * consumer close callback
 *
 * in case close was called while user was still blocked in read this
 * function cancels the read, and releases the user context.
 * @param [in] inode
 * @param [in] filp ceva_linkdrv_pfd handler associated with the procfs
 * @return zero for success, nonzero for failure
 * @see _cancel_linkdrv_pfd_read_irqsafe()
 */
static int _ceva_linkdrv_proc_release(struct inode *inode, struct file *filp){

	struct ceva_linkdrv_pfd *pfd = PDE_DATA(filp->f_path.dentry->d_inode);
	pid_t pid, tid;

	if (!pfd) {
		Dump_ARMDSP_HndShakeInfo();
		XM6_MSG(XM6_MSG_ERR, "%s :: null pfd descriptor\n", __FUNCTION__);
		return -EINVAL;
	}

	pid = task_tgid_vnr(current);
	tid = task_pid_nr(current);

	if (pfd->events_ref_count == 1) {
		int ret = _cancel_linkdrv_pfd_read_irqsafe(pfd);
		if (ret) {
			Dump_ARMDSP_HndShakeInfo();
			XM6_MSG(XM6_MSG_ERR, "unable to cancel the read operation on %d/%s (p%d/t%d)\n",
					pid, CEVA_LINKDRV_PROC_EVENTS_NAME, pid, tid);
			return ret;
		}
		/*
		 * keep the pdf descriptor in the list, as we'll need it
		 * later on for deletion of the object
		 */
	}

	pfd->events_ref_count--;
	Dump_ARMDSP_HndShakeInfo();
	XM6_MSG(XM6_MSG_ERR, "%d/%s was closed (by p%d/t%d), open descriptor value: %d\n", pid, CEVA_LINKDRV_PROC_EVENTS_NAME, pid, tid, pfd->events_ref_count);

	return 0;
}


/*!
 * consumer read callback
 *
 * user is reading data from its relative procfs entry (created during open).
 * data is delivered using a kfifo object, and copied to the user's buffer.
 * this function blocks the user call until events are received from the CEVA
 * device.
 * @param [in] filp ceva_linkdrv_pfd handler associated with the procfs
 * @param [out] buf user buffer
 * @param [in] count number of bytes to read from kfifo
 * @param [in] ppos not used
 * @return number of bytes read, negative value for errors
 * @see ceva_linkdrv_broadcast_events(), _cancel_linkdrv_pfd_read(),
 * _cancel_linkdrv_pfd_read_irqsafe()
 */
static ssize_t _ceva_linkdrv_proc_read(struct file *filp, char __user *buf,
			size_t count, loff_t *ppos) {

	struct ceva_linkdrv_pfd *pfd = PDE_DATA(filp->f_path.dentry->d_inode);
	struct ceva_linkdrv *link = pfd->owner;
	unsigned long flags;
	int items, events_to_read, waiting, blocked, ret;
    pid_t pid;

	if  (!pfd || !link ) {
        XM6_MSG(XM6_MSG_ERR, "pfd or link is NULL\n");
		return -EINVAL;
	}

	if (!link->online) {
        XM6_MSG(XM6_MSG_ERR, "link is not online\n");
		return -EPIPE;
	}

	/*
	 * check if insufficient memory was provided to store the event's data,
	 * the buffer size should be for at least one event
	 */
	events_to_read = (count/sizeof(ceva_event_t));
	if (events_to_read <= 0) {
        XM6_MSG(XM6_MSG_ERR, "count is not correct\n");
		return -EINVAL;
	}
	
	spin_lock_irqsave(&link->waiting_lock, flags);
	// block caller thread for incoming events (if fifo is empty)
	if (kfifo_is_empty(&pfd->fifo)) {
		/*
		 * we use a local variable here to allow this operation to be break'ed
		 * in case the driver is unloaded, and the pfd object is no longer
		 * valid we still want the user to get the EPIPE, this could be done
		 * only if the driver will signal using a valid object (while the
		 * actual pfd is deleted)
		 */
		blocked = 0;
		// TODO: should we spinlock here?
		//spin_lock_irqsave(&link->waiting_lock, flags);
		pfd->blocked = &blocked;
		spin_unlock_irqrestore(&link->waiting_lock, flags);
		ret = wait_event_interruptible_timeout(link->wq, blocked != 0,DSPTimeOutSlot);
		/*
		 * we must reset the blocked pointer, as its a local variable
		 * no need to protect this part from irq as we are already not
		 * blocked, next read will check the kfifo anyhow
		 */
		pfd->blocked = NULL;
		if(ret== -ERESTARTSYS){
			pid = task_tgid_vnr(current);
			XM6_MSG(XM6_MSG_ERR, "read %d/%s was breake'd by signal (kill)\n", pid, CEVA_LINKDRV_PROC_EVENTS_NAME);
			Dump_ARMDSP_HndShakeInfo();
			return -ERESTARTSYS;
		}
		else if(ret == 0){
			pid = task_tgid_vnr(current);
			XM6_MSG(XM6_MSG_ERR, "read %d/%s was breake'd by TimeOut:%d \n", pid, CEVA_LINKDRV_PROC_EVENTS_NAME,DSPTimeOutSlot);
			Dump_ARMDSP_HndShakeInfo();
			return DSPTimeOutSlot;
		}
		else{
			pid = task_tgid_vnr(current);
			XM6_MSG(XM6_MSG_DBG, "read %d/%s Success remain jfifs :%d \n", pid, CEVA_LINKDRV_PROC_EVENTS_NAME,ret);
		}
		if (blocked < 0) {
			pid = task_tgid_vnr(current);
			XM6_MSG(XM6_MSG_ERR, "read %d/%s was breake'd by user (broken pipe)\n", pid, CEVA_LINKDRV_PROC_EVENTS_NAME);
			Dump_ARMDSP_HndShakeInfo();
			if (!link->online) {
				goto breaked_by_deinit;
			}
			return -EPIPE;
		}
	}
	else{
		spin_unlock_irqrestore(&link->waiting_lock, flags);
	}
	
	waiting = kfifo_len(&pfd->fifo);
	items = min(events_to_read, waiting);
	if (!items) {
		Dump_ARMDSP_HndShakeInfo();
		XM6_MSG(XM6_MSG_WRN, "events read was unblocked, but kfifo is empty\n");
		return 0;
	}
	XM6_MSG(XM6_MSG_DBG, "%d events waiting from device, coping to user %d events\n", waiting, items);
	ret = kfifo_to_user(&pfd->fifo, buf, items*sizeof(ceva_event_t), &items);
	return ret ? ret : items;

breaked_by_deinit:
	// remove current entry from the list
	list_del_init(&pfd->list_node);
	// delete pfd object
	_destroy_linkdrv_pfd(pfd);
	return -EPIPE;
}

/*!
 * producer data generation callback
 *
 * this function is called from the physical driver module to notify user
 * of CEVA device events. it pushed events data to each of the consumenrs
 * kfifo object, and releases blocked read operations.
 * @param [in] link 'this' module object
 * @param [in] events array of events to push
 * @param [in] sz number of events waiting in 'events'
 * @return zero for success, nonzero for failure
 */
int ceva_linkdrv_broadcast_events(struct ceva_linkdrv* link,
		ceva_event_t* events, int sz) {
	struct list_head *pos;
	struct ceva_linkdrv_pfd *i;
	int avail, ret;
	unsigned long flags;

    XM6_MSG(XM6_MSG_DBG, "broadcast events\n");

	if  (!link->online) {
        XM6_MSG(XM6_MSG_ERR, "link is not online\n");
		return -EPIPE;
	}

	DEBUG_INFO_ADD(link, events_recieved, sz);
	list_for_each(pos, &link->pfd_list) {
		i = list_entry(pos, struct ceva_linkdrv_pfd, list_node);
		if (!i) {
			XM6_MSG(XM6_MSG_WRN, "list entry is NULL\n");
			continue;
		}
		link=i->owner;
		// copy event data to the link's consumers
		avail = kfifo_avail(&i->fifo);
		if (!avail) {
			DEBUG_INFO_INC(link, fifo_full);
			XM6_MSG(XM6_MSG_WRN, "no available space for %s kfifo, %d missed\n", CEVA_LINKDRV_PROC_EVENTS_NAME,sz);
			continue;
		}
		spin_lock_irqsave(&link->waiting_lock, flags);
		ret = kfifo_in(&i->fifo, events, min(sz, avail));
		if (!ret) {
			/*
			 * should we continue here?
			 * fifo is full, but perhaps user is blocked from some reason
			 * we should increment blocked anyway for the benefit of the doubt
			 */
			DEBUG_INFO_INC(link, fifo_in_err);
			XM6_MSG(XM6_MSG_WRN, "unable to insert more data to descriptor %s's kfifo, "
				  "kfifo_avail returned with %d\n", CEVA_LINKDRV_PROC_EVENTS_NAME, avail);
		}
		/*
		 * check if user us waiting on events
		 */
		if (i->blocked) {
			(*i->blocked)++;
		}
		spin_unlock_irqrestore(&link->waiting_lock, flags);
	}
	// broadcast 'stop wait' for all waiting threads
	wake_up_interruptible_all(&link->wq);
	
	return 0;
}
EXPORT_SYMBOL_GPL(ceva_linkdrv_broadcast_events);

/*!
 * cancel any read operations blocked on the link's device (all entries)
 *
 * called by the module deinit function
 * @param [in] link 'this' module object
 * @return zero for success, nonzero for failure
 * @see ceva_linkdrv_deinit
 */
static int _ceva_linkdrv_proc_read_break(struct ceva_linkdrv *link) {

	struct list_head *pos;
	struct ceva_linkdrv_pfd* i;
	int j;
	unsigned long flags;

	XM6_MSG(XM6_MSG_DBG, "canceling read on all events entries\n");
	spin_lock_irqsave(&link->waiting_lock, flags);
	// drop all waiting receivers from link
	list_for_each(pos, &link->pfd_list) {
		// get iterator
		i = list_entry(pos, struct ceva_linkdrv_pfd, list_node);
		/*
		 * ensure blocked get negative value (irq might increment it
		 * after function exits)
		 */
		if (i->blocked) {
			*(i->blocked) = UNBLOCKED_BY_USER;
		} else {
			// remove current entry from the list
			list_del_init(&i->list_node);
			// delete pfd object
			_destroy_linkdrv_pfd(i);
		}
	}
	spin_unlock_irqrestore(&link->waiting_lock, flags);
	// broadcast 'stop wait' for all waiting threads
	wake_up_interruptible_all(&link->wq);

	/*
	 * now we have to wait for all remaining pfds that are blocked,
	 * as we can't release the waitqueue until they stopped waiting on it
	 */
	for (j = 0; (j < CANCEL_READ_MAX_RELAX_TRIES &&
				 !list_empty(&link->pfd_list)); j++) {
		msleep(0);
	}
	if (j == CANCEL_READ_MAX_RELAX_TRIES) {
		XM6_MSG(XM6_MSG_ERR, "unable to break all waiting read executions\n");
		return -EAGAIN;
	}
	return 0;
}

#ifdef ENABLE_DEBUG

#define CEVA_LINKDRV_PROC_DEBUG "debug"
static struct proc_dir_entry *ceva_linkdrv_proc_entry_debug;

static int _ceva_link_debug_proc_write_cb(struct file *file,
		const char __user *buffer, unsigned long count, void *data) {
	struct ceva_linkdrv *link = (struct ceva_linkdrv *) data;
	char str[] = { "ABCD" };
	ceva_event_t event;
	memcpy(&event, str, sizeof(ceva_event_t));

	XM6_MSG(XM6_MSG_DBG, "sending event\n");

	ceva_linkdrv_broadcast_events(link, &event, 1);

	return count;
}

static int _ceva_link_debug_proc_init(struct ceva_linkdrv *link)
{

	ceva_linkdrv_proc_entry_debug = create_proc_read_entry(
			CEVA_LINKDRV_PROC_DEBUG,              /* name */
			S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH, /* mode */
			ceva_linkdrv_proc_dir,                 /* parent */
			NULL,                                  /* read */
			link);                                 /* data */

	if (!ceva_linkdrv_proc_entry_debug)
		return -ENOMEM;

	ceva_linkdrv_proc_entry_debug->write_proc =
			_ceva_link_debug_proc_write_cb;

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29))
	ceva_linkdrv_proc_entry_debug->owner = THIS_MODULE;
#endif /* KERNEL_VERSION(2,6,29) */

	return 0;
}

static void _ceva_link_debug_proc_deinit()
{
	remove_proc_entry(CEVA_LINKDRV_PROC_DEBUG, ceva_linkdrv_proc_dir);
}

#else

static int _ceva_link_debug_proc_init(struct ceva_linkdrv *link) { return 0; }
static void _ceva_link_debug_proc_deinit() {}

#endif

/*!
 * init module
 * @param [in/out] link 'this' module object
 * @return zero for success, nonzero for failure
 */
int ceva_linkdrv_init(struct ceva_linkdrv *link, struct xm6_dev_data *owner) {

	int ret = 0;

	link->owner = owner;
	mutex_init(&link->mux);
	spin_lock_init(&link->waiting_lock);
	INIT_LIST_HEAD(&link->pfd_list);
	init_waitqueue_head(&link->wq);

	/* create directory */
	ceva_linkdrv_proc_dir = proc_mkdir(MODULE_NAME, NULL);
	if(ceva_linkdrv_proc_dir == NULL) {
		ret = -ENOMEM;
		goto out;
	}
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,29))
	ceva_linkdrv_proc_dir->owner = THIS_MODULE;
#endif /* KERNEL_VERSION(2,6,29) */
	XM6_MSG(XM6_MSG_DBG, "procfs dir created: %s\n", MODULE_NAME);

	if (_ceva_link_debug_proc_init(link) != 0) {
		ret = -ENOMEM;
		goto no_debug;
	}
	link->online = 1;
	return 0;

no_debug:
	remove_proc_entry(MODULE_NAME, NULL);
out:
	return ret;
}
EXPORT_SYMBOL_GPL(ceva_linkdrv_init);

/*!
 * deinit module
 *
 * @param [in/out] link 'this' module object
 */
void ceva_linkdrv_deinit(struct ceva_linkdrv *link) {

	XM6_MSG(XM6_MSG_ERR, "ceva_linkdrv_deinit\n");
	Dump_ARMDSP_HndShakeInfo();
	link->online = 0;
	_ceva_linkdrv_proc_read_break(link);
	_ceva_link_debug_proc_deinit();
	remove_proc_entry(MODULE_NAME, NULL);
	mutex_destroy(&link->mux);
}
EXPORT_SYMBOL_GPL(ceva_linkdrv_deinit);
