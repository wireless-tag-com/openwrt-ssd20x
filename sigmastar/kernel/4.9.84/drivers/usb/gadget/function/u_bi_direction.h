#ifndef __U_BI_DIRECTION_H
#define __U_BI_DIRECTION_H

#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/cdev.h>

#define GBI_DIRECTION_BULK_IN_BUFLEN	20 * 1024
#define GBI_DIRECTION_BULK_OUT_BUFLEN	40 * 1024
#define GBI_DIRECTION_BULK_IN_QLEN		3
#define GBI_DIRECTION_BULK_OUT_QLEN	    1

#define NERVE_DEVICE_NAME_SIZE 36
#define NERVE_NAME "usb_bi"
#define MODULE_VERION "1.0.0"
#define USB_PROC_DEBUG_ENABLE 1
#define PROG_ENTRY "usb_bi"

#define CONTROL_REQUEST_SIZE 64
#define MAX_NAME_LENGTH 52

#define MASK 0xFF000000
#define BULK_IN_QUEUE_BASE_OFFSET  0x0F000000
#define BULK_OUT_QUEUE_BASE_OFFSET 0xF0000000
#define BULK_IN_BUFFER(off) (((off & MASK) == BULK_IN_QUEUE_BASE_OFFSET))
#define BULK_OUT_BUFFER(off) (((off & MASK) == BULK_OUT_QUEUE_BASE_OFFSET))

typedef enum {
	LOG_DBG,
	LOG_INFO,
	LOG_WRN,
	LOG_ERR,
} log_level_e;

#define BDBG(fmt, args...) \
	{do{if(log_level<=LOG_DBG){printk("[DBG]:%s[%d]: " fmt , __FUNCTION__,__LINE__,##args);}}while(0);}
#define BINFO(fmt, args...) \
	{do{if(log_level<=LOG_INFO){printk("[INFO]:%s[%d]: " fmt , __FUNCTION__,__LINE__,##args);}}while(0);}
#define BWRN(fmt, args...) \
	{do{if(log_level<=LOG_WRN){printk("[WRN]:%s[%d]: " fmt , __FUNCTION__,__LINE__,##args);}}while(0);}
#define BERR(fmt, args...) \
	{do{if(log_level<=LOG_ERR){printk("[ERR]:%s[%d]: " fmt , __FUNCTION__,__LINE__,##args);}}while(0);}

typedef enum {
	TRANSFER_IN  = 0xABBA,
	TRANSFER_OUT = 0xBAAB
} buf_type_e;

typedef struct
{
	__u32 count;
	__u32 type; /* for usr to set */
	__u32 length;
} requeset_buf_t;

typedef struct
{
	__u32 type;
	__u32 index;
	__u32 length;
	__u32 bytesused;
	__u32 mem_offset;
	__u8 *vaddr;
} buffer_t;

typedef struct {
	bool used;
	buffer_t buf;
    struct list_head list;
} bi_data_t;

typedef struct {
	bool inited;
	__u32 qlen;
	bi_data_t *buf;
	spinlock_t lock;
	struct list_head used_list;
} bi_buffer_queue_t;

typedef struct {
	__u32 magic;
	__u32 length;
} bi_operation_permit_t;

#define USB_CAMMAND_MAGIC 0xFDCB

typedef struct {
	__u8 cmd;
	__u8 hostready;
	__u16 magic;
	__u32 dataLength;
	__u32 offset;
	char name[MAX_NAME_LENGTH];
} bi_control_header_t;

typedef enum {
	USB_GET_DEVICE_STATUS = 0,
	USB_RESET_REQUEST,
	USB_HOST_SET_DATA,
	USB_HOST_GET_DATA
} Usbcommands_t;

#define USB_NERVE_DEQUEUE_EVENT		_IOR('N', 1, buffer_t)
#define USB_NERVE_FINISH_EVENT		_IOW('N', 2, buffer_t)
#define USB_NERVE_DEQUEUE_BUF		_IOWR('N', 3, buffer_t)
#define USB_NERVE_QUEUE_BUF		_IOWR('N', 4, buffer_t)
#define USB_NERVE_REQBUFS		_IOWR('N', 5, requeset_buf_t)
#define USB_NERVE_QUERYBUF		_IOWR('N', 6, buffer_t)

struct usb_bi_direction_options {
	unsigned bulk_in_buflen;
	unsigned bulk_out_buflen;
	unsigned bulk_in_qlen;
	unsigned bulk_out_qlen;
};

struct f_bd_opts {
	struct usb_function_instance func_inst;
	unsigned bulk_in_buflen;
	unsigned bulk_in_qlen;
	unsigned bulk_out_buflen;
	unsigned bulk_out_qlen;

	/*
	 * Read/write access to configfs attributes is handled by configfs.
	 *
	 * This is to protect the data from concurrent access by read/write
	 * and create symlink/remove symlink.
	 */
	struct mutex	lock;
	int		refcnt;
};

/*
 * DEVICE_NOT_INIT    ：
 * 		Unreqister, the dev is unbound
 * DEVICE_INITIALIZED :
 * 		Register, the dev is bound
 * DEVICE_WAITING     :
 *		Host is cobdect to dev, and enable bi, waiting for control req
 * DEVICE_RUNNING     :
 *		A control request is dealing
 * DEVICE_FINISHED    :
 * 		A data required is finished, and ready for tx
 * DEVICE_PENDING     :
 *		A OUT DATA Packet is ready to Recv
 *
 * Attention: user is only for user space
 */

typedef enum {
	DEVICE_NOT_INIT = 0,
	DEVICE_INITIALIZED = 0x10,
	DEVICE_WAITING = 0x11,
	DEVICE_RUNNING = 0x12,
	DEVICE_FINISHED = 0x13,
	DEVICE_PENDING = 0x14,
} DeviceStatus_t;

struct bi_device_handle
{
	bool opened;
	dev_t major;

	DeviceStatus_t status;

	bi_buffer_queue_t bulk_in_queue;
	bi_buffer_queue_t bulk_out_queue;

	struct bi_device *bdev; //gadget device
	struct device *dev; //char device associated
#if (USB_PROC_DEBUG_ENABLE==1)
	struct proc_dir_entry* proc_entry;
#endif
	struct cdev cdev;
	struct class *nclass;
	wait_queue_head_t waitq;
};

struct bi_device {
	char	name[NERVE_DEVICE_NAME_SIZE];
	struct usb_function	function;
	unsigned	bulk_in_qlen;
	unsigned	bulk_out_qlen;
	unsigned	bulk_in_buflen;
	unsigned	bulk_out_buflen;
	struct usb_ep		*in_ep;
	struct usb_ep		*out_ep;
	struct list_head	in_req_free;
	struct list_head	out_req_free;
	struct usb_request	*in_req[GBI_DIRECTION_BULK_IN_QLEN];
	struct usb_request	*out_req[GBI_DIRECTION_BULK_OUT_QLEN];

	spinlock_t	in_req_lock;
	spinlock_t	out_req_lock;

	struct	device *dev; // gadget struct releated device
	struct	bi_device_handle *devh; //module handle
};

/* func specific */
static inline struct bi_device *func_to_bi(struct usb_function *f)
{
	return container_of(f, struct bi_device, function);
}

int bi_device_register(struct device *dev, struct bi_device *ndev);
void bi_device_unregister(struct bi_device *ndev);
int enable_bi_device(struct bi_device *ndev);
void disable_bi_device(struct bi_device *ndev);
int bi_out_packet_handle(struct usb_ep *ep, struct usb_request *req);
#endif /* __BI_DIRECTION_H */
