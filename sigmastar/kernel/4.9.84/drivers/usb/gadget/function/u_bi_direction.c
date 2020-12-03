#include <linux/device.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <linux/printk.h>
#include <linux/usb/composite.h>

#include "u_bi_direction.h"
#include "u_f.h"

#if (USB_PROC_DEBUG_ENABLE==1)
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#endif

extern unsigned int log_level;

/*
  1.  buffer_queue_deinit
  2.  buffer_queue_init
  3.  buffer_queue_find_buf
  4.  buffer_queue_find_free_buf
  5.  buffer_queue_finish_buf
  6.  buffer_queue_queue_data
  7.  buffer_queue_dequeue_data
  8.  buffer_queue_is_empty
  9.  buffer_queue_used_list_is_empty
  10. buffer_queue_reset
 */
static int buffer_queue_deinit(bi_buffer_queue_t *queue)
{
	int i;

	BDBG(" Enter\n");
	if(!queue->inited)
		return 0;

	for(i = 0; i < queue->qlen; i++)
	{
		if(queue->buf[i].buf.vaddr)
			vfree(queue->buf[i].buf.vaddr);
	}
	kfree(queue->buf);
	queue->buf = NULL;
	queue->qlen = 0;
	queue->inited = false;
	INIT_LIST_HEAD(&queue->used_list);
	return 0;
}

static int buffer_queue_init(bi_buffer_queue_t *queue,
		unsigned int qlen, unsigned int len, unsigned int base_off)
{
	int i;

	BDBG(" Enter\n");
	if(queue->inited)
		return 0;

	queue->qlen = qlen;
	queue->buf = kzalloc(qlen * sizeof(bi_data_t), GFP_KERNEL);
	if(queue->buf == NULL)
	{
		BERR("kzalloc Failed\n");
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&queue->used_list);
	spin_lock_init(&queue->lock);

	for(i = 0; i < qlen; i++)
	{
		queue->buf[i].used = false;
		queue->buf[i].buf.index = i;
		queue->buf[i].buf.length = len;
		queue->buf[i].buf.bytesused = 0;
		queue->buf[i].buf.mem_offset = PAGE_ALIGN(i * len + base_off);
		queue->buf[i].buf.vaddr = vmalloc_user(len);
		if(queue->buf[i].buf.vaddr == NULL) {
			BERR("vmalloc_user failed, ret %d\n", (int)queue->buf[i].buf.vaddr);
			goto error;
        	}
		BINFO("alloc a dev, offset 0x%x, addr 0x%x, len %u, index %d, memoffset 0x%x\n",
			queue->buf[i].buf.mem_offset, (unsigned int)queue->buf[i].buf.vaddr,
			queue->buf[i].buf.length, queue->buf[i].buf.index, queue->buf[i].buf.mem_offset);
	}
	queue->inited = true;
	return 0;

error:
	buffer_queue_deinit(queue);
	return -EINVAL;
}

/* Get ready Data from list */
static buffer_t * buffer_queue_dequeue_data(bi_buffer_queue_t *queue)
{
	unsigned long flags;
	bi_data_t * data;
	BDBG("Enter\n");
	if(!queue->inited)
	{
		BERR("queue 0x%x, queue->buf 0x%x\n",
				(unsigned int)queue, (unsigned int)queue->buf);
		return NULL;
	}

	spin_lock_irqsave(&queue->lock, flags);
	if (list_empty(&queue->used_list)) {
		spin_unlock_irqrestore(&queue->lock, flags);
		BWRN("No Ready Buf to be dequeue\n");
		return NULL;
	}
	data = list_first_entry(&queue->used_list, bi_data_t,list);
	list_del(&data->list);
	spin_unlock_irqrestore(&queue->lock, flags);
	return &data->buf;
}

/* Put A data to list_used */
static int buffer_queue_queue_data(bi_buffer_queue_t *queue, buffer_t *data, void *buf, unsigned int len)
{
	int i;
	unsigned long flags;

	BDBG("Enter \n");
	if(!queue->inited)
	{
		BERR("queue 0x%x, queue->buf 0x%x, queue->qlen %u\n",
				(unsigned int)queue, (unsigned int)queue->buf, queue->qlen);
		return -EINVAL;
	}

	if(data && data->bytesused)
	{
		for(i = 0; i < queue->qlen ; i++)
		{
       		if(queue->buf[i].buf.index != data->index ||
				queue->buf[i].used == true)
				continue;

			queue->buf[i].buf.bytesused = data->bytesused;
			BINFO("queue index %d bytesused %u\n",
			queue->buf[i].buf.index,
			queue->buf[i].buf.bytesused);
			spin_lock_irqsave(&queue->lock, flags);
			queue->buf[i].used = true;
			list_add_tail(&queue->buf[i].list, &queue->used_list);
			spin_unlock_irqrestore(&queue->lock, flags);
			return 0;
		}
		return -EINVAL;
	}

	if(!buf || !len)
		return -EINVAL;

	for(i = 0; i < queue->qlen ; i++)
	{
		if(queue->buf[i].used == true)
			continue;

		if(queue->buf[i].buf.length < len)
		{
			BWRN("buflen %u, putdata len %u \n",
				queue->buf[i].buf.length, len);
			return -EINVAL;
		}
		spin_lock_irqsave(&queue->lock, flags);
		queue->buf[i].used = true;
		memcpy(queue->buf[i].buf.vaddr, buf, len);
		queue->buf[i].buf.bytesused = len;

		list_add_tail(&queue->buf[i].list, &queue->used_list);
		spin_unlock_irqrestore(&queue->lock, flags);
		return 0;
	}
	BWRN("No Free Buf\n");
	return -ENOMEM;
}

/* Set the used flags to free */
static int buffer_queue_finish_buf(bi_buffer_queue_t *queue, buffer_t *buf)
{
	unsigned long flags;
	bi_data_t *qbuf = NULL;
	bi_data_t *data = NULL;

	if(!queue->inited)
	{
		BERR("queue 0x%x, queue->buf 0x%x, buf 0x%x\n",
				(unsigned int)queue, (unsigned int)queue->buf, (unsigned int)buf);
		return -EINVAL;
	}

	data = container_of(buf, bi_data_t ,buf);

	spin_lock_irqsave(&queue->lock, flags);
	list_for_each_entry(qbuf, &queue->used_list, list)
	{
		if(qbuf->buf.vaddr == buf->vaddr ||
		   qbuf->buf.index == buf->index ||
		   qbuf->buf.mem_offset == buf->mem_offset)
		{
			BWRN("BUF need to be handled !!!\n");
			return -EBUSY;
		}
	}
	spin_unlock_irqrestore(&queue->lock, flags);
	buf->bytesused = 0;
	data->used = false;
	return 0;
}

static buffer_t * buffer_queue_find_free_buf(bi_buffer_queue_t *queue)
{
	int i = 0;

	if(!queue->inited)
	{
		BERR("queue 0x%x, queue->buf 0x%x, queue->qlen %u\n",
				(unsigned int)queue, (unsigned int)queue->buf, queue->qlen);
		return NULL;
	}

	for(i = 0; i <queue->qlen; i++)
	{
		if(queue->buf[i].used == false)
			return &queue->buf[i].buf;
	}
	return NULL;
}

static buffer_t * buffer_queue_find_buf(bi_buffer_queue_t *queue, void *vaddr, unsigned int index, unsigned int mem_offset)
{
	int i = 0;
	buffer_t * buf = NULL;

	if(!queue->inited)
	{
		BERR("queue 0x%x, queue->buf 0x%x, queue->qlen %u\n",
				(unsigned int)queue, (unsigned int)queue->buf, queue->qlen);
		return NULL;
	}

	for(i = 0; i <queue->qlen; i++)
	{
		buf = &queue->buf[i].buf;
		BDBG("tmp index %d mem 0x%x offset 0x%x\n", buf->index, buf->mem_offset, mem_offset);
		if(buf->index == index)
		{
			BINFO("Find A buf By index %d\n", buf->index)
			return buf;
		}
		if(buf->vaddr == vaddr)
		{
			BINFO("Find A buf By vadd 0x%x\n", (unsigned int)buf->vaddr)
			return buf;
		}
		if(buf->mem_offset == mem_offset)
		{
			BINFO("Find A buf By offset 0x%x\n",buf->mem_offset)
			return buf;
		}
	}
	return NULL;
}

/* check if queue is empty (all is used) */
static bool buffer_queue_is_empty(bi_buffer_queue_t *queue)
{
	int i;

	if(!queue->inited)
	{
		BERR("queue 0x%x, queue->buf 0x%x, queue->qlen %u\n",
				(unsigned int)queue, (unsigned int)queue->buf, queue->qlen);
		return -EINVAL;
	}

	for(i = 0; i < queue->qlen; i++)
	{
		if(queue->buf[i].used == false)
			return false;
	}
	return true;
}

static bool buffer_queue_used_list_is_empty(bi_buffer_queue_t *queue)
{
	if(!queue->inited)
		return -EINVAL;

	return list_empty(&queue->used_list);
}

static int buffer_queue_reset(bi_buffer_queue_t *queue)
{
	buffer_t *buf;

	if(!queue->inited)
		return -EINVAL;

	while(1) {
		buf = buffer_queue_dequeue_data(queue);
		if(buf == NULL)
			break;
		buffer_queue_finish_buf(queue, buf);
	}
	return 0;
}

static long bi_reqbufs(struct bi_device *bdev, requeset_buf_t *user_data)
{
	struct	bi_device_handle *h = bdev->devh;

	requeset_buf_t rb;
	long ret = 0;
	BDBG(" Enter\n")

	ret = copy_from_user(&rb, user_data, sizeof(buffer_t));
	if(ret) {
		BERR("copy_from_user ERR %ld \n", ret);
		return ret;
	}

	BINFO(" count %d type %s length %u\n", rb.count, rb.type == TRANSFER_IN?"in":"out", rb.length);
	if(rb.type == TRANSFER_IN)
	{
		if(rb.count)
		{
			if(rb.length > 0 && rb.length <= GBI_DIRECTION_BULK_IN_BUFLEN)
				return buffer_queue_init(&h->bulk_in_queue, rb.count,
						rb.length, BULK_IN_QUEUE_BASE_OFFSET);
			else
				BERR("request buf size to large \n");
		}
		else
			return buffer_queue_deinit(&h->bulk_in_queue);
	}
	else if(rb.type == TRANSFER_OUT)
	{
		if(rb.count)
		{
			if(rb.length > 0 && rb.length <= GBI_DIRECTION_BULK_OUT_BUFLEN)
				return buffer_queue_init(&h->bulk_out_queue, rb.count,
						rb.length, BULK_OUT_QUEUE_BASE_OFFSET);
			else
				BERR("request buf size to large \n");
		}
		else
			return buffer_queue_deinit(&h->bulk_out_queue);
	}
	return -EINVAL;
}

/* QUEUE A BUF To IN QUEUE*/
static long bi_queue_buf(struct bi_device *bdev, buffer_t *user_data)
{
	struct	bi_device_handle *h = bdev->devh;
	buffer_t fdata;
	long ret = 0;
	BDBG("Enter\n");

	ret = copy_from_user(&fdata, user_data, sizeof(buffer_t));
	if(ret)
		return ret;

	return buffer_queue_queue_data(&h->bulk_in_queue, &fdata, NULL, 0);
}

/* Get A Free BUF From IN QUEUE*/
static long bi_dequeue_buf(struct bi_device *bdev, buffer_t *user_data)
{
	struct	bi_device_handle *h = bdev->devh;
	buffer_t *buf = NULL;
	BDBG("Enter\n");

	buf = buffer_queue_find_free_buf(&h->bulk_in_queue);
	if(buf == NULL)
		return -EBUSY;
	BINFO("get a free buf, index %d\n", buf->index);
	return copy_to_user(user_data, buf, sizeof(buffer_t));
}

static long bi_query_buf(struct bi_device *bdev, buffer_t *user_data)
{
	struct	bi_device_handle *h = bdev->devh;
	buffer_t fdata, *buf = NULL;
	unsigned int ret = 0;
	BDBG("Enter\n");

	ret = copy_from_user(&fdata, user_data, sizeof(buffer_t));
	if(ret)
		return ret;

	BINFO("find %s buf , index %d \n", fdata.type == TRANSFER_IN? "IN":"OUT", fdata.index);
	if(fdata.type == TRANSFER_IN)
		buf = buffer_queue_find_buf(&h->bulk_in_queue, (void*)0xFFFFFFFF, fdata.index, 0xFFFFFFFF);
	else if(fdata.type == TRANSFER_OUT)
		buf = buffer_queue_find_buf(&h->bulk_out_queue, (void*)0xFFFFFFFF, fdata.index, 0xFFFFFFFF);
	else
		return -EINVAL;

	if(buf == NULL)
		return -EINVAL;

	return copy_to_user(user_data, buf, sizeof(buffer_t));
}

static long bi_finish_buf(struct bi_device *bdev, buffer_t *user_data)
{
	struct	bi_device_handle *h = bdev->devh;
	buffer_t fdata, *buf = NULL;
	unsigned int ret = 0;

	BDBG("Enter\n");

	ret = copy_from_user(&fdata, user_data, sizeof(buffer_t));
	if(ret)
		return ret;

	buf = buffer_queue_find_buf(&h->bulk_out_queue, fdata.vaddr, fdata.index, fdata.mem_offset);
	if(buf == NULL)
		return -EINVAL;

	return buffer_queue_finish_buf(&h->bulk_out_queue, buf);
}

static long bi_dequeue_event(struct bi_device *bdev, buffer_t *buf)
{
	struct	bi_device_handle *h = bdev->devh;
	buffer_t *data = NULL;

	BDBG("Enter\n");

	if((data = buffer_queue_dequeue_data(&h->bulk_out_queue)) == NULL)
		return -ENOMEM;

	return copy_to_user(buf, data, sizeof(buffer_t));
}

static int bi_queue_in_packet(struct bi_device *bdev, void *data, unsigned int len)
{
	struct usb_request *req = NULL;
	unsigned long flags, ret = 0;

	BDBG("Enter\n");
	if (!len || !data)
		return -EINVAL;

	spin_lock_irqsave(&bdev->in_req_lock, flags);
	if (list_empty(&bdev->in_req_free)) {
		spin_unlock_irqrestore(&bdev->in_req_lock, flags);
		return 0;
	}
	req = list_first_entry(&bdev->in_req_free, struct usb_request, list);
	list_del(&req->list);
	spin_unlock_irqrestore(&bdev->in_req_lock, flags);

	req->length = min_t(unsigned int, bdev->bulk_in_buflen, len);
	req->zero = len < bdev->bulk_in_buflen;
	memcpy(req->buf, data, req->length);
	BINFO("queue a in request , size %d\n", req->length);
	if((ret = usb_ep_queue(bdev->in_ep, req, GFP_KERNEL)) < 0)
	{
		BERR("Queue A buf Failed \n");
		return -1;
	}
	return 0;
}

static long bi_device_ioctl(struct file *file,
	unsigned int cmd, unsigned long arg)
{
	struct bi_device *bdev = file->private_data;
	struct	bi_device_handle *h = bdev->devh;

	BDBG("Enter\n");
	if(!h || h->status < DEVICE_INITIALIZED)
	{
		BERR(" device status %d \n",!h ? 0 : h->status);
		return -EINVAL;
	}

	switch (cmd) {
	case USB_NERVE_DEQUEUE_EVENT:
		return bi_dequeue_event(bdev, (void*)arg);

	case USB_NERVE_FINISH_EVENT:
		return bi_finish_buf(bdev, (void*)arg);

	case USB_NERVE_DEQUEUE_BUF:
		return bi_dequeue_buf(bdev, (void*)arg);

	case USB_NERVE_QUEUE_BUF:
		return bi_queue_buf(bdev, (void*)arg);

	case USB_NERVE_REQBUFS:
		return bi_reqbufs(bdev, (void*)arg);

	case USB_NERVE_QUERYBUF:
		return bi_query_buf(bdev, (void*)arg);

	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

static void bi_device_queue_reset(struct bi_device *bdev)
{
	struct	bi_device_handle *h = bdev->devh;

	buffer_queue_reset(&h->bulk_in_queue);
	buffer_queue_reset(&h->bulk_out_queue);
}

static int bi_device_open(struct inode *inode, struct file *file)
{
	struct bi_device_handle *h =
		container_of(inode->i_cdev, struct bi_device_handle, cdev);

	BDBG("Enter\n");
	if(!h || h->status < DEVICE_INITIALIZED || h->opened)
		return -EINVAL;

	init_waitqueue_head(&h->waitq);
	file->private_data = h->bdev;

	h->opened = true;
	return 0;
}

static int bi_device_release(struct inode *inode, struct file *file)
{
	struct bi_device *bdev = file->private_data;
	struct bi_device_handle *h = bdev->devh;

	BDBG("Enter\n");
	if(!h || h->status < DEVICE_INITIALIZED || !h->opened)
		return -EINVAL;

	file->private_data = NULL;

	h->opened = false;
	return 0;
}

static unsigned int bi_device_poll(struct file *file, poll_table *wait)
{
	struct bi_device *bdev = file->private_data;
	struct bi_device_handle *h = bdev->devh;
	unsigned int mask = 0;

	BDBG("Enter\n");
	poll_wait(file, &h->waitq, wait);

	/*
	 *   use wake_up to toggle
	 */
	if(buffer_queue_used_list_is_empty(&h->bulk_out_queue) == false)
		mask |= POLLIN | POLLRDNORM;
	else
		BINFO("status 0x%x \n", h->status);

	return mask;
}

static int bi_device_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct bi_device *bdev = file->private_data;
	struct bi_device_handle *h = bdev->devh;
	unsigned long off = vma->vm_pgoff << PAGE_SHIFT;
	buffer_t * buf;
	int ret;

	BINFO("pageoffset %lx offset %lx \n", vma->vm_pgoff, off);

	if(BULK_IN_BUFFER(off))
	{
		if (!(vma->vm_flags & VM_SHARED)) {
		BWRN("Invalid vma flags, VM_SHARED needed\n");
		return -EINVAL;
		}
		if (!(vma->vm_flags & VM_READ)) {
			BWRN("Invalid vma flags, VM_READ needed\n");
			return -EINVAL;
		}
		buf = buffer_queue_find_buf(&h->bulk_in_queue, (void*)0xFFFFFFFF, 0xFFFFFFFF, off);
		if(buf == NULL) {
			BWRN("no such buf\n");
			return -ENOMEM;
		}
	}
	else if(BULK_OUT_BUFFER(off))
	{
		if (!(vma->vm_flags & VM_SHARED)) {
			BWRN("Invalid vma flags, VM_SHARED needed\n");
			return -EINVAL;
		}
		if (!(vma->vm_flags & VM_WRITE)) {
			BWRN("Invalid vma flags, VM_WRITE needed\n");
			return -EINVAL;
		}
		buf = buffer_queue_find_buf(&h->bulk_out_queue, (void*)0xFFFFFFFF, 0xFFFFFFFF, off);
		if(buf == NULL) {
			BWRN("no such buf\n");
			return -ENOMEM;
		}
	} else {
		BERR("wrong mem offset\n");
		return -EINVAL;
	}

	ret = remap_vmalloc_range(vma, buf->vaddr, 0);
	if (ret) {
		BERR("Remapping vmalloc memory, error: %d\n", ret);
		return ret;
	}
	return 0;
}

static const struct file_operations bi_fops = {
	.owner		= THIS_MODULE,
	.open		= bi_device_open,
	.release	= bi_device_release,
	.unlocked_ioctl = bi_device_ioctl,
	.poll		= bi_device_poll,
	.mmap		= bi_device_mmap,
};

#if (USB_PROC_DEBUG_ENABLE==1)
static inline char* getLogLevel(int log_level)
{
	switch(log_level)
	{
		case LOG_DBG:
			return "LOG_DBG";
		case LOG_INFO:
			return "LOG_INFO";
		case LOG_WRN:
			return "LOG_WRN";
		case LOG_ERR:
			return "LOG_ERR";
		default:
			return "Unknown Level";
	}
	return NULL;
}

static inline int setLogLevel(char *level)
{
	if (!strncmp(level, "LOG_DBG", 7))
		log_level = LOG_DBG;
	else if (!strncmp(level, "LOG_INFO", 8))
		log_level = LOG_INFO;
	else if (!strncmp(level, "LOG_WRN", 7))
		log_level = LOG_WRN;
	else if (!strncmp(level, "LOG_ERR", 7))
		log_level = LOG_ERR;
	else
	{
		printk("Invalid Args [%s]", level);
		printk("echo param >/proc/usb_bi/debug_level\n"
			"  param: LOG_DBG\n"
			"         LOG_INFO\n"
			"         LOG_WRN\n"
			"         LOG_ERR\n");
	};
	return 0;
}

static int bd_proc_show(struct seq_file *m, void *v)
{
	const struct file *file = m->file;

	if(!strcmp(file->f_path.dentry->d_iname, "debug_level"))
	{
		seq_printf(m, "%s\n", getLogLevel(log_level));
	}
	else
	if(!strcmp(file->f_path.dentry->d_iname, PROG_ENTRY))
	{
		seq_printf(m, "%s entity[%s] todo\n", __func__, PROG_ENTRY);
	}
	return 0;
}

static int bd_proc_write(
	struct file *file,
	const char __user *buffer,
	size_t count, loff_t *f_pos)
{
	char *args = NULL;

	args = kzalloc((count+1), GFP_KERNEL);
	if(!args)
		return -ENOMEM;

	if(copy_from_user(args, buffer, count))
	{
		kfree(args);
		return EFAULT;
	}

	if(!strcmp(file->f_path.dentry->d_iname, "debug_level"))
	{
	    setLogLevel(args);
	} else
	if(!strcmp(file->f_path.dentry->d_iname, PROG_ENTRY))
	{
		printk("%s entity[%s] todo\n", __func__,  PROG_ENTRY);
	} else
		count = -EINVAL;

	kfree(args);
	return count;
}

static int bd_proc_open(struct inode *inode, struct file *file)
{
	struct bi_device *bdev = file->private_data;

	return single_open(file, bd_proc_show, bdev);
}

static const struct file_operations bd_proc_fops = {
	.open  = bd_proc_open,
	.read  = seq_read,
	.llseek  = seq_lseek,
	.release = single_release,
	.write	= bd_proc_write,
};

static int bd_proc_create(struct bi_device_handle *h)
{
	h->proc_entry = proc_mkdir(PROG_ENTRY, 0);
	if(!h->proc_entry)
		BINFO("Dir esist\n");

	proc_create_data("debug_level", 0, h->proc_entry, &bd_proc_fops, h->bdev);
	proc_create_data(PROG_ENTRY, 0, h->proc_entry, &bd_proc_fops, h->bdev);

	return 0;
}

static void bd_proc_destroy(struct bi_device_handle *h)
{
	proc_remove(h->proc_entry);
}
#endif

static int f_bi_out_ep_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct bi_device	*bi = ep->driver_data;
	unsigned long flags;
	int status;

	BDBG("Enter\n");
	bi_out_packet_handle(ep, req);

	BINFO("queue a out request , size %d\n", req->length);
	status = usb_ep_queue(ep, req, GFP_ATOMIC);
	if (status == 0) {
		return 0;
	} else {
		BWRN("Unable to queue buffer to %s: %d\n",ep->name, status);
		goto free_req;
	}

free_req:
	spin_lock_irqsave(&bi->out_req_lock,flags);
	req->length = 0;
	list_add_tail(&req->list, &bi->out_req_free);
	spin_unlock_irqrestore(&bi->out_req_lock,flags);
	return status;
}

static int f_bi_in_ep_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct bi_device	*bi = ep->driver_data;
	unsigned long flags;
	BDBG("Enter\n");

	spin_lock_irqsave(&bi->in_req_lock,flags);
	req->length = 0;
	list_add_tail(&req->list, &bi->in_req_free);
	spin_unlock_irqrestore(&bi->in_req_lock,flags);

	return 0;
}

static void bi_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct bi_device	*bi = ep->driver_data;
	int			status = req->status;
	unsigned long flags;

	BDBG("Enter\n");

	switch (status) {

	case 0:		/* normal completion? */
		if (ep == bi->out_ep) {
			f_bi_out_ep_complete(ep, req);
		} else {
			f_bi_in_ep_complete(ep, req);
		}
		break;
	default:
		BERR("%s bi complete --> %d, %d/%d\n", ep->name,
				status, req->actual, req->length);
	/* FALLTHROUGH */

	/* NOTE:  since this driver doesn't maintain an explicit record
	 * of requests it submitted (just maintains qlen count), we
	 * rely on the hardware driver to clean up on discobdect or
	 * endpoint disable.
	 */
	case -ECONNABORTED:		/* hardware forced ep reset */
	case -ECONNRESET:		/* request dequeued */
	case -ESHUTDOWN:		/* discobdect from host */
		if(ep == bi->out_ep) {
			spin_lock_irqsave(&bi->out_req_lock,flags);
			req->length = 0;
			list_add_tail(&req->list, &bi->out_req_free);
			spin_unlock_irqrestore(&bi->out_req_lock,flags);
		}
		else {
			spin_lock_irqsave(&bi->in_req_lock,flags);
			req->length = 0;
			list_add_tail(&req->list, &bi->in_req_free);
			spin_unlock_irqrestore(&bi->in_req_lock,flags);
		}
		return;
	}
}

static int bi_free_request(struct bi_device *bdev)
{
	unsigned int		i;
	struct f_bd_opts	*bd_opts = NULL;
	struct usb_ep		*ep = NULL;

	bd_opts = container_of(bdev->function.fi, struct f_bd_opts, func_inst);

	/* in ep */
	ep = bdev->in_ep;
	for (i = 0; i < bd_opts->bulk_in_qlen; ++i) {
		if (bdev->in_req[i]) {
			usb_ep_dequeue(ep, bdev->in_req[i]);
			free_ep_req(ep, bdev->in_req[i]);
        	}
		bdev->in_req[i] = NULL;
	}
	INIT_LIST_HEAD(&bdev->in_req_free);

	/* out ep */
	ep = bdev->out_ep;
	for (i = 0; i < bd_opts->bulk_out_qlen; ++i) {
		if (bdev->out_req[i]) {
			usb_ep_dequeue(ep, bdev->out_req[i]);
			free_ep_req(ep, bdev->out_req[i]);
		}
		bdev->out_req[i] = NULL;
	}
	INIT_LIST_HEAD(&bdev->out_req_free);
	return 0;
}

/*
 *  Alloc OUT Endpoint Request and Queue For RX Control
 * 	Alloc IN Endpoint and add it to Free Queue
 */
static int bi_alloc_request(struct bi_device *bdev)
{
	unsigned		i;
	int			result;
	struct usb_ep		*ep = NULL;
	struct f_bd_opts	*bd_opts = NULL;
	struct usb_composite_dev	*cdev = NULL;

	result = 0;
	cdev = bdev->function.config->cdev;
	bd_opts = container_of(bdev->function.fi, struct f_bd_opts, func_inst);

	/* in ep */
	ep = bdev->in_ep;
	for (i = 0; i < bdev->bulk_in_qlen; i++) {
		bdev->in_req[i] = alloc_ep_req(ep, bd_opts->bulk_in_buflen);
		if (!bdev->in_req[i])
			break;

		bdev->in_req[i]->actual = 0;
		bdev->in_req[i]->length = 0;
		bdev->in_req[i]->complete = bi_complete;
		bdev->in_req[i]->context = bdev;

		BINFO("%s %s alloc req buf len %u addr 0x%x \n",
		__func__,ep->name,bd_opts->bulk_in_buflen, (unsigned int)bdev->in_req[i]->buf);
		list_add_tail(&bdev->in_req[i]->list, &bdev->in_req_free);
	}

	/* out ep */
	ep = bdev->out_ep;
	for (i = 0; i < bdev->bulk_out_qlen && result == 0; i++) {
		bdev->out_req[i] = alloc_ep_req(ep, bd_opts->bulk_out_buflen);
		if (!bdev->out_req[i])
			goto fail;

		bdev->out_req[i]->actual = 0;
		bdev->out_req[i]->length = CONTROL_REQUEST_SIZE;
		bdev->out_req[i]->complete = bi_complete;
		bdev->out_req[i]->context = bdev;

		BINFO("%s %s alloc req buf len %u addr 0x%x \n",
		__func__,ep->name, bd_opts->bulk_out_buflen, (unsigned int)bdev->out_req[i]->buf);
		result = usb_ep_queue(ep, bdev->out_req[i], GFP_ATOMIC);
		if (result) {
			ERROR(cdev, "%s queue req --> %d\n",ep->name, result);
			goto fail;
		}
	}
	return 0;

fail:
	return result;
}

static void disable_ep(struct usb_composite_dev *cdev, struct usb_ep *ep)
{
	int	value;

	if (ep->driver_data) {
		value = usb_ep_disable(ep);
		if (value < 0)
			DBG(cdev, "disable %s --> %d\n",ep->name, value);
		ep->driver_data = NULL;
	}
}

void disable_endpoints(struct usb_composite_dev *cdev,
		struct usb_ep *in, struct usb_ep *out,
		struct usb_ep *int_in, struct usb_ep *int_out)
{
	BDBG("Enter\n");
	disable_ep(cdev, in);
	disable_ep(cdev, out);
	if (int_in)
		disable_ep(cdev, int_in);
	if (int_out)
		disable_ep(cdev, int_out);
}

void disable_bi_device(struct bi_device *bdev)
{
	struct usb_composite_dev *cdev = NULL;
	struct	bi_device_handle *h = bdev->devh;

	BDBG("Enter\n");
	if(!bdev || h->status < DEVICE_WAITING)
		return;

	cdev = bdev->function.config->cdev;
	if(!cdev)
		return;

	BINFO("disable eps \n");
	disable_endpoints(cdev, bdev->in_ep, bdev->out_ep, NULL, NULL);

	bi_free_request(bdev);

	h->status = DEVICE_INITIALIZED;
	BINFO("%s disabled\n", bdev->function.name);
}

static int enable_endpoint(struct usb_composite_dev *cdev, struct bi_device *bi,
		struct usb_ep *ep)
{
	int	result;

	result = config_ep_by_speed(cdev->gadget, &(bi->function), ep);
	if (result)
		goto fail;
	result = usb_ep_enable(ep);
	if (result < 0)
		goto fail;
	ep->driver_data = bi;
	return 0;

fail:
	return result;
}

int enable_bi_device(struct bi_device *bdev)
{
	int	result = 0;
	struct usb_composite_dev * cdev = NULL;
	struct	bi_device_handle *h = bdev->devh;

	cdev = bdev->function.config->cdev;

	BDBG("Enter\n");
	if(!bdev || !cdev || h->status != DEVICE_INITIALIZED)
		return -EINVAL;

	BINFO("enable ep in\n");
	result = enable_endpoint(cdev, bdev, bdev->in_ep);
	if (result)
		return result;

	BINFO("enable ep out\n");
	result = enable_endpoint(cdev, bdev, bdev->out_ep);
	if (result)
		return result;

	result = bi_alloc_request(bdev);
	if (result)
		return result;

	h->status = DEVICE_WAITING;
	BINFO("%s enabled\n", bdev->function.name);
	return result;
}

/*
 * bind with gadget dev and create a char dev
 */
int bi_device_register(struct device *dev, struct bi_device *bdev)
{
	dev_t devid;
	int major  = 0, ret = 0;
	struct device *char_dev = NULL;
	struct class *nclass = NULL;
    struct bi_device_handle *h = NULL;

	BDBG("Enter\n");
	printk("%s version: %s\n", NERVE_NAME, MODULE_VERION);

	if(!bdev)
        return -EINVAL;

	/* alloc a bi dev handle to process dev*/
	h = kzalloc(sizeof(*h), GFP_KERNEL);
	if(!h)
		return -ENOMEM;

	strcpy(bdev->name, NERVE_NAME);
	/* associate with gadget device */
	get_device(dev);
	bdev->dev = dev;
	if (dev == NULL) {
		/* If dev == NULL, then name must be filled in by the caller */
		if (WARN_ON(!bdev->name[0])) {
			ret = -EINVAL;
		}
		return 0;
	}
	/* Set name to driver name + device name if it is empty. */
	if (!bdev->name[0])
		snprintf(bdev->name, sizeof(bdev->name), "%s %s",
	dev->driver->name, dev_name(dev));
	if (!dev_get_drvdata(dev))
		dev_set_drvdata(dev, bdev);

	/* Alloc a char device */
	devid = MKDEV(major, 0);
	ret = alloc_chrdev_region(&devid, 0, 1, bdev->name);
	if(ret < 0)
		return -EINVAL;
	major = MAJOR(devid);

	cdev_init(&h->cdev, &bi_fops);
	h->cdev.owner= bi_fops.owner;

	ret = cdev_add(&h->cdev, MKDEV(major, 0), 1);
	if(ret < 0) {
		printk(KERN_INFO "Error %d adding char_mem device", ret);
		goto cdev_detroy;
	}

	/* register sys class and create a device */
	nclass = class_create(THIS_MODULE, bdev->name);
	if(IS_ERR(nclass)) {
		ret = PTR_ERR(nclass);
		goto cdev_detroy;
	}

	char_dev = device_create(nclass, NULL, devid, bdev, bdev->name);
	if(IS_ERR(char_dev)) {
		ret = PTR_ERR(char_dev);
		goto class_destroy;
	}

	h->major = major;
	h->nclass = nclass;
	h->dev = char_dev;
	h->status = DEVICE_INITIALIZED;
	h->opened = 0;

	h->bdev = bdev;
	bdev->devh = h;

	/* alloc endpoint requeset*/
	BINFO("Init req Spin lock and list\n");
	INIT_LIST_HEAD(&bdev->in_req_free);
	spin_lock_init(&bdev->in_req_lock);
	INIT_LIST_HEAD(&bdev->out_req_free);
	spin_lock_init(&bdev->out_req_lock);

#if (USB_PROC_DEBUG_ENABLE==1)//bi_device_handle
	bd_proc_create(h);
#endif

	return 0;

class_destroy:
	class_destroy(nclass);
cdev_detroy:
	cdev_del(&h->cdev);
	unregister_chrdev_region(devid, 1);
	return ret;
}

void bi_device_unregister(struct bi_device *bdev)
{
	struct bi_device_handle *h = bdev->devh;
	int major = h->major;
	struct device *char_dev = h->dev;
	struct class *nclass = h->nclass;
	dev_t devid = MKDEV(major, 0);

	if(!bdev || !h || h->status == DEVICE_NOT_INIT)
		return;

	if(nclass && char_dev)
		device_destroy(nclass, devid);

	if(nclass)
		class_destroy(nclass);

	cdev_del(&h->cdev);

	unregister_chrdev_region(devid, 1);
	bd_proc_destroy(h);
	kfree(h);
	bdev->devh = NULL;

	return;
}

int bi_ep0_complete(struct usb_ep *ep, struct usb_request *req)
{
	return 0;
}

int bi_ep0_setup(struct usb_ep *ep, struct usb_request *req)
{
	return 0;
}

int bi_out_packet_handle(struct usb_ep *ep, struct usb_request *req)
{
	bi_control_header_t	*crq = req->buf;
	struct bi_device	*bdev = ep->driver_data;
	struct	bi_device_handle *h = bdev->devh;
	bi_operation_permit_t	permit = {0xFFFF, 0};
	buffer_t		*buf = NULL;

	BINFO("Enter crq->magic 0x%x, reqlen %u, actual %u\n", crq->magic, req->length, req->actual);
	if(req->length != sizeof(bi_control_header_t) ||
		crq->magic != USB_CAMMAND_MAGIC){
		BDBG("Dealing specify out data reqlen %d \n",req->actual);
        if(req->actual)
		buffer_queue_queue_data(&h->bulk_out_queue, NULL, req->buf, req->actual);

		if(h->opened)
			wake_up_interruptible(&h->waitq);
		goto requeue_std_crq;
	}

	if(req->length != sizeof(bi_control_header_t))
		goto requeue_std_crq;

	/* only support a request now!!! */
	switch(crq->cmd)
	{
		case USB_GET_DEVICE_STATUS:
			BINFO("USB_GET_DEVICE_STATUS\n");
			bi_queue_in_packet(bdev, &h->status, sizeof(sizeof(h->status)));
			goto requeue_std_crq;
		case USB_RESET_REQUEST:
			BINFO("USB_RESET_REQUEST\n");
			bi_device_queue_reset(bdev);
			if (bdev->in_ep->driver_data)
				disable_bi_device(bdev);
			return enable_bi_device(bdev);
			//need to info user to reset
		case USB_HOST_GET_DATA:
			BINFO("USB_HOST_GET_DATA\n");
			buf = buffer_queue_dequeue_data(&h->bulk_in_queue);
			if(buf == NULL)
			{
				bi_queue_in_packet(bdev, &permit, sizeof(permit));
				goto requeue_std_crq;
			}
			permit.magic = 0xABCD;
			permit.length = min_t(unsigned int, buf->bytesused, crq->dataLength);
			if(crq->dataLength != buf->bytesused)
				BWRN("Get Length %u, Buf len %u\n",crq->dataLength, buf->bytesused);
			bi_queue_in_packet(bdev, &permit, sizeof(permit));
			bi_queue_in_packet(bdev, buf->vaddr, permit.length);
			buffer_queue_finish_buf(&h->bulk_in_queue, buf);
			break;
		case USB_HOST_SET_DATA:
			BINFO("USB_HOST_SET_DATA\n");
			if(buffer_queue_is_empty(&h->bulk_out_queue))
			{
				bi_queue_in_packet(bdev, &permit, sizeof(permit));
				goto requeue_std_crq;
			}
			permit.magic = 0xABCD;
			permit.length = min_t(unsigned int, bdev->bulk_out_buflen, crq->dataLength);
			bi_queue_in_packet(bdev, &permit, sizeof(permit));
			req->length = permit.length;
			return 0;
		default:
			goto requeue_std_crq;
	}

requeue_std_crq:
	req->length = CONTROL_REQUEST_SIZE;
	BINFO("Queue A Standard OUT Request\n");
	return 0;
}
