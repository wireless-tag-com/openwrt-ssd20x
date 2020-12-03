/*
* protected_mem_db.c- Sigmastar
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
 * protected_mem_db.c
 *
 * Simple miscellaneous shared memory driver
 * The driver implements a shared memory for Ceva DB structure
 *
 *	Linux kernel module
 *	simple single misc device file (miscdevice, misc_register)
 *	file_operations:
 *		read/write
 *		ioctl: lock and unlock
 * Created on: Sep 12, 2013
 * Author: Tal Halpern <Tal.Halperin@tandemg.com>
 * Modified: Ido Reis <ido.reis@tandemg.com>
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

#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>

#include "ceva_linkdrv-generic.h"
#include "ceva_linkdrv_shared_process_protected_db_def_copy.h"
#include "protected_mem_db.h"

#include "drv_debug.h"

/*!
 * struct protected_mem_db_data - the driver data
 * @dev_mutex: a mutex that protects the shared memory access
 * @lock_pid: an atomic variable to hold the last pid of the
 * thread which called the PROTECTED_MEM_DB_LOCK ioctl.
 *
 */
struct protected_mem_db_data {
	struct mutex dev_mutex;
	atomic_t lock_pid;
};

static int protected_mem_db_open(struct inode *inode, struct file *file);
static int protected_mem_db_release(struct inode *inode, struct file *file);
static ssize_t protected_mem_db_read(struct file *file, char __user *buf,
		size_t count, loff_t *ppos);
static ssize_t protected_mem_db_write(struct file *file,
		const char __user *buf, size_t count, loff_t *ppos);
static long protected_mem_db_ioctl(struct file *file, unsigned int cmd,
		unsigned long arg);
static struct protected_mem_db_data *protected_mem_db_data_init(void);

/* driver data object */
static struct protected_mem_db_data *drvdata;

/* the shared memory array statically allocated*/
static char shmp[sizeof(ceva_linkdrv_shared_process_protected_db)];

static const struct file_operations protected_mem_db_fops = {
	.owner	= THIS_MODULE,
	.open	= protected_mem_db_open,
	.release = protected_mem_db_release,
	.read	= protected_mem_db_read,
	.write	= protected_mem_db_write,
	.unlocked_ioctl = protected_mem_db_ioctl,
};

static struct miscdevice protected_mem_db_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= CEVA_PROT_MEM_DRV_NAME,
	.fops	= &protected_mem_db_fops,
};

/*!
 * open function
 * @param [in] inode not used by this module
 * @param [in] file not used by this module
 * @return always zero
 */
static int protected_mem_db_open(struct inode *inode, struct file *file)
{
	/* client related data can be allocated here and
	   stored in file->private_data */
	file->private_data = drvdata;

	XM6_MSG(XM6_MSG_DBG, "%s :: device opened by process %d\n",
			__FUNCTION__, task_tgid_vnr(current));

	return 0;
}

/*!
 * release function
 *
 * released the shared memory in case closing task is still locking it
 * could be due to software bug, abnormal application exit (SEG signal), etc
 * @param [in] inode not used by this module
 * @param [in] file not used by this module
 * @return always zero
 */
static int protected_mem_db_release(struct inode *inode, struct file *file)
{
	/* client related data can be retrieved from file->private_data
	   and released here */

	if (atomic_read(&(drvdata->lock_pid)) == task_tgid_vnr(current)) {
		atomic_set(&(drvdata->lock_pid), -1);
		mutex_unlock(&drvdata->dev_mutex);
	}

	/*
	 * place additional code here for closing a device
	 */
	XM6_MSG(XM6_MSG_DBG, "%s :: device closed by process %d\n",
			__FUNCTION__, task_tgid_vnr(current));
	return 0;
}

/*!
 * read handler
 *
 * reads data from the shared memory with a relative offset.
 * as the user interface does not include an offset argument, the user
 * has to write the offset value into the buf. the driver first read from
 * buf the offset value and user actual buffer address, and then reads
 * the data into the forwarded user supplied address.
 * @param [in] file not used by this module
 * @param [in/out] buf output buffer to read data to
 * @param [in] count number of bytes to read
 * @param [in] ppos not used by this module
 * @return number of bytes read from shared memory, negative values for errors
 * @see struct RWBuffer, protected_mem_db_write()
 */
static ssize_t protected_mem_db_read(struct file *file, char __user *buf,
		size_t count, loff_t *ppos) {
	struct RWBuffer to;
	struct protected_mem_db_data * dev;
	pid_t pid;

	if (!count) {
		return 0;
	}
	if (!access_ok(VERIFY_WRITE, buf, count)) {
		return -EFAULT;
	}

	dev = (struct protected_mem_db_data *) file->private_data;
	pid = task_tgid_vnr(current);

	if (atomic_read(&(drvdata->lock_pid)) != pid) {
		if (mutex_lock_interruptible(&dev->dev_mutex)) {
			return -EINTR;
		}
	}
	if (copy_from_user(&to, buf, sizeof(to)) != 0) {
		return -EFAULT;
	}
	if (to.offset + count > sizeof(ceva_linkdrv_shared_process_protected_db)) {
		return -EINVAL;
	}
	if (copy_to_user(to.buf, shmp + to.offset, count) != 0) {
		return -EFAULT;
	}
	*ppos += count;
	if (atomic_read(&(drvdata->lock_pid)) != pid) {
		mutex_unlock(&dev->dev_mutex);
	}

	return count;
}

/*!
 * write handler
 *
 * writes data from user to a relative offset within the shared memory.
 * as the user interface does not include an offset argument, the user
 * has to write the offset value into the buf. the driver first read from
 * buf the offset value and user actual buffer address, and then reads
 * the data into the forwarded user supplied address.
 * @param [in] file not used by this module
 * @param [in] buf output buffer to read data to
 * @param [in] count number of bytes to write
 * @param [in] ppos not used by this module
 * @return number of bytes written to shared memory, negative values for errors
 * @see struct RWBuffer, protected_mem_db_read()
 */
static ssize_t protected_mem_db_write(struct file *file,
		const char __user *buf,	size_t count, loff_t *ppos)
{
	struct RWBuffer from;
	struct protected_mem_db_data * dev;
	pid_t pid;

	if (!count) {
		return 0;
	}
	if (!access_ok(VERIFY_READ, buf, count)) {
		return -EFAULT;
	}

	dev = (struct protected_mem_db_data *) file->private_data;
	pid = task_tgid_vnr(current);

	if (atomic_read(&(drvdata->lock_pid)) != pid) {
		if (mutex_lock_interruptible(&dev->dev_mutex)) {
			return -EINTR;
		}
	}
	/* copy the offset to local */
	if (copy_from_user(&from, buf, sizeof(from)) != 0) {
		return -EFAULT;
	}
	if (from.offset + count > sizeof(ceva_linkdrv_shared_process_protected_db)) {
		return -EINVAL;
	}
	if (copy_from_user(shmp + from.offset, from.buf, count) != 0) {
		return -EFAULT;
	}
	*ppos += count;
	if (atomic_read(&(drvdata->lock_pid)) != pid) {
		mutex_unlock(&dev->dev_mutex);
	}
	return count;
}

/*!
 * ioctl handler
 *
 * special commands executer.
 * this module allows single lock/unlock of the shared memory region
 * @param [in] filp device handler
 * @param [in] cmd command to execute (lock/unlock)
 * @param [in] arg currently not used by this module
 * @return zero for success, nonzero for failures
 */
long protected_mem_db_ioctl(struct file *file, unsigned int cmd,
		unsigned long arg)
{
	struct protected_mem_db_data *dev;
	pid_t pid;
	int ret = 0;

	/*
	 * don't even decode wrong cmds: better returning EINVAL than EFAULT
	 */
	if (_IOC_TYPE(cmd) != IOC_CEVADRV_MAGIC ||
		_IOC_NR(cmd) > IOC_CEVADRV_MAXNR) {
		ret = -EINVAL;
		goto exit;
	}

	dev = (struct protected_mem_db_data *) file->private_data;
	pid = task_tgid_vnr(current);

	switch (cmd) {
		case IOC_CEVADRV_PROTMEM_LOCK:
				XM6_MSG(XM6_MSG_DBG, "IOC_CEVADRV_PROTMEM_LOCK ioctl\n");
				if (mutex_lock_interruptible(&dev->dev_mutex)) {
					XM6_MSG(XM6_MSG_ERR, "%s :: unable to lock mutex\n", __FUNCTION__);
					ret = -EINTR;
					goto exit;
				}
				atomic_set(&(drvdata->lock_pid), pid);
				XM6_MSG(XM6_MSG_DBG, "device locked by process %d\n", pid);
			break;

		case IOC_CEVADRV_PROTMEM_UNLOCK:
				XM6_MSG(XM6_MSG_DBG, "IOC_CEVADRV_PROTMEM_UNLOCK ioctl\n");
				if (atomic_read(&(drvdata->lock_pid)) != pid) {
					XM6_MSG(XM6_MSG_ERR, "process %d did not locked the device\n", pid);
					ret = -EPERM;
					goto exit;
				}
				atomic_set(&(drvdata->lock_pid), -1);
				mutex_unlock(&dev->dev_mutex);
				XM6_MSG(XM6_MSG_DBG, "device unlocked\n");
			break;

		default:
			ret = -EINVAL;
	}
exit:
	return ret;
}

/*
 * Initialization and cleanup section
 */

/*!
 * deinit function
 *
 * releases any allocated memory done by this module
 */
void protected_mem_db_deinit(void)
{
	if (protected_mem_db_dev.this_device) {
		misc_deregister(&protected_mem_db_dev);
	}
	mutex_destroy(&drvdata->dev_mutex);
	kfree(drvdata);
}

/*!
 * helper function for init
 * @return valid object address for success, NULL for failures
 */
static struct protected_mem_db_data *protected_mem_db_data_init(void)
{
	struct protected_mem_db_data *drv;

	drv = kzalloc(sizeof(struct protected_mem_db_data), GFP_KERNEL);
	if (!drv) {
		return NULL;
	}

	mutex_init(&drv->dev_mutex);
	atomic_set(&drv->lock_pid, -1);
	/* initialize the shared memory*/
	memset(shmp, 0, sizeof(ceva_linkdrv_shared_process_protected_db));

	return drv;
}

/*!
 * init function
 *
 * initializes the module
 * @return zero for success, nonzero for failures
 */
int protected_mem_db_init(void)
{
	int ret = 0;

	drvdata = protected_mem_db_data_init();
	if (!drvdata) {
		XM6_MSG(XM6_MSG_ERR, "protected_mem_db_data_init failed\n");
		ret = -ENOMEM;
		goto exit;
	}

	ret = misc_register(&protected_mem_db_dev);
	if (ret < 0) {
		XM6_MSG(XM6_MSG_ERR, "misc_register failed\n");
		goto exit;
	}

	return 0;

exit:
	protected_mem_db_deinit();
	return ret;
}

MODULE_DESCRIPTION("protected_mem_db Simple misc device driver");
MODULE_AUTHOR("Tal Halpern");
MODULE_LICENSE("GPL");
