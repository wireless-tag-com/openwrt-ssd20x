/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * SPDX-License-Identifier:	GPL-2.0+
 * Note: Part of this code has been derived from linux
 *
 */
#ifndef _USB_H_
#define _USB_H_

#define MP_USB_MSTAR 1

#include <usb_defs.h>
#include <linux/usb/ch9.h>
#include <asm/cache.h>
#include <part.h>
#include <linux/list.h>		/* for struct list_head */

/*
 * The EHCI spec says that we must align to at least 32 bytes.  However,
 * some platforms require larger alignment.
 */
#if ARCH_DMA_MINALIGN > 32
#define USB_DMA_MINALIGN	ARCH_DMA_MINALIGN
#else
#define USB_DMA_MINALIGN	32
#endif

/* Everything is aribtrary */
#define USB_ALTSETTINGALLOC		4
#define USB_MAXALTSETTING		128	/* Hard limit */

#define USB_MAX_DEVICE			32
#define USB_MAXCONFIG			8
#define USB_MAXINTERFACES		8
#define USB_MAXENDPOINTS		16
#if 1 //MBOOT_XHCI
#define USB_ENDPOINTALLOC		8
#endif
#define USB_MAXCHILDREN			8	/* This is arbitrary */
#define USB_MAX_HUB			16

#define USB_CNTL_TIMEOUT 100 /* 100ms timeout */

/*
 * This is the timeout to allow for submitting an urb in ms. We allow more
 * time for a BULK device to react - some are slow.
 */
#define USB_TIMEOUT_MS(pipe) (usb_pipebulk(pipe) ? 5000 : 100)

/* device request (setup) */
struct devrequest {
	unsigned char	requesttype;
	unsigned char	request;
	unsigned short	value;
	unsigned short	index;
	unsigned short	length;
} __attribute__ ((packed));

enum usb_interface_condition {
	USB_INTERFACE_UNBOUND = 0,
	USB_INTERFACE_BINDING,
	USB_INTERFACE_BOUND,
	USB_INTERFACE_UNBINDING,
};

/* Interface */
struct usb_interface {
	struct usb_interface_descriptor desc;
	struct usb_endpoint_descriptor ep_desc[USB_MAXENDPOINTS];

	unsigned char	no_of_ep;
	unsigned char	num_altsetting;
	unsigned char	act_altsetting;
	struct usb_host_interface *altsetting;

	enum usb_interface_condition condition;		/* state of binding */
	unsigned sysfs_files_created:1;	/* the sysfs attributes exist */
	unsigned ep_devs_created:1;	/* endpoint "devices" exist */
	unsigned unregistering:1;	/* unregistration is in progress */
	unsigned needs_remote_wakeup:1;	/* driver requires remote wakeup */
	unsigned needs_altsetting0:1;	/* switch to altsetting 0 is pending */
	unsigned needs_binding:1;	/* needs delayed unbind/rebind */
	unsigned reset_running:1;
	unsigned resetting_device:1;	/* true: bandwidth alloc after reset */
	/*
	 * Super Speed Device will have Super Speed Endpoint
	 * Companion Descriptor  (section 9.6.7 of usb 3.0 spec)
	 * Revision 1.0 June 6th 2011
	 */
	struct usb_ss_ep_comp_descriptor ss_ep_comp_desc[USB_MAXENDPOINTS];
} __attribute__ ((packed));

/* Configuration information.. */
struct usb_config {
	struct usb_config_descriptor desc;
	struct usb_interface if_desc[USB_MAXINTERFACES];

	unsigned char	no_of_if;	/* number of interfaces */
} __attribute__ ((packed));

enum {
	/* Maximum packet size; encoded as 0,1,2,3 = 8,16,32,64 */
	PACKET_SIZE_8   = 0,
	PACKET_SIZE_16  = 1,
	PACKET_SIZE_32  = 2,
	PACKET_SIZE_64  = 3,
};


/* USB_DT_SS_ENDPOINT_COMP: SuperSpeed Endpoint Companion descriptor */

struct usb_host_endpoint {
	struct usb_endpoint_descriptor		desc;
	struct usb_ss_ep_comp_descriptor	ss_ep_comp;
	struct list_head		urb_list;
	void				*hcpriv;
	struct ep_device		*ep_dev;	/* For sysfs info */

	unsigned char *extra;   /* Extra descriptors */
	int extralen;
	int enabled;
};

struct usb_device {
	int	devnum;			/* Device number on USB bus */
	int	speed;			/* full/low/high */
	char	mf[32];			/* manufacturer */
	char	prod[32];		/* product */
	char	serial[32];		/* serial number */

	/* Maximum packet size; one of: PACKET_SIZE_* */
	int maxpacketsize;
	/* one bit for each endpoint ([0] = IN, [1] = OUT) */
	unsigned int toggle[2];
	/* endpoint halts; one bit per endpoint # & direction;
	 * [0] = IN, [1] = OUT
	 */
	unsigned int halted[2];
	int epmaxpacketin[16];		/* INput endpoint specific maximums */
	int epmaxpacketout[16];		/* OUTput endpoint specific maximums */

	int configno;			/* selected config number */
	/* Device Descriptor */
	struct usb_device_descriptor descriptor
		__attribute__((aligned(ARCH_DMA_MINALIGN)));
	struct usb_config config; /* config descriptor */
	struct usb_host_config *actconfig;

	int have_langid;		/* whether string_langid is valid yet */
	int string_langid;		/* language ID for strings */
	int (*irq_handle)(struct usb_device *dev);
	unsigned long irq_status;
	int irq_act_len;		/* transfered bytes */
	void *privptr;
	/*
	 * Child devices -  if this is a hub device
	 * Each instance needs its own set of data structures.
	 */
	unsigned long status;
	int act_len;			/* transfered bytes */
	int maxchild;			/* Number of ports if hub */
	int portnr;
	struct usb_device *parent;
	struct usb_device *children[USB_MAXCHILDREN];
	int slot_id;
	enum usb_device_state  	state;
	u32		route;			//hub routing
	struct usb_host_endpoint ep0;
	struct usb_host_endpoint bulk1;
	struct usb_host_endpoint bulk2;
	int portnum;			//real port number
#if 1 //MBOOT_XHCI
	u8 level;
#endif
	unsigned char *product;
	unsigned char *manufacturer;
	unsigned char *serial2;
	struct usb_host_endpoint *ep_in[16];
	struct usb_host_endpoint *ep_out[16];
	unsigned char **rawdescriptors;
	struct usb_host_config *pconfig;

	void *controller;		/* hardware controller private data */
	/* slot_id - for xHCI enabled devices */
};

struct urb {
	/* private: usb core and host controller only fields in the urb */
	int kref;		/* reference count of the URB */
	void *hcpriv;			/* private data for host controller */
	int use_count;		/* concurrent submissions counter */
	int reject;		/* submissions will fail */
	int unlinked;			/* unlink error code */

	/* public: documented fields in the urb that can be used by drivers */
	struct list_head urb_list;	/* list head for use by the urb's
					 * current owner */
	struct list_head anchor_list;	/* the URB may be anchored */
	struct usb_anchor *anchor;
	struct usb_device *dev;		/* (in) pointer to associated device */
	struct usb_host_endpoint *ep;	/* (internal) pointer to endpoint */
	unsigned int pipe;		/* (in) pipe information */
	unsigned int stream_id;		/* (in) stream ID */
	int status;			/* (return) non-ISO status */
	unsigned int transfer_flags;	/* (in) URB_SHORT_NOT_OK | ...*/
	void *transfer_buffer;		/* (in) associated data buffer */
	dma_addr_t transfer_dma;	/* (in) dma addr for transfer_buffer */
//	struct scatterlist *sg;		/* (in) scatter gather buffer list */
	int num_mapped_sgs;		/* (internal) mapped sg entries */
	int num_sgs;			/* (in) number of entries in the sg list */
	u32 transfer_buffer_length;	/* (in) data buffer length */
	u32 actual_length;		/* (return) actual transfer length */
	unsigned char *setup_packet;	/* (in) setup packet (control only) */
	dma_addr_t setup_dma;		/* (in) dma addr for setup_packet */
	int start_frame;		/* (modify) start frame (ISO) */
	int number_of_packets;		/* (in) number of ISO packets */
	int interval;			/* (modify) transfer interval
					 * (INT/ISO) */
	int error_count;		/* (return) number of ISO errors */
	void *context;			/* (in) context for completion */
	int complete;	/* (in) completion routine */
#ifdef CONFIG_MSTAR_CHIP
	 void *SetDMABuf;	//Note, for DMA
	 void *tmpSetDMABuf;
	 s32 SetDMALen;
	 void *Orignal_SetDMABuf;

	 void *TxDMABuf;
	 void *tmpTxDMABuf;
	 s32 TxDMALen;
	 void *Orignal_TxDMABuf;
#endif
//	struct usb_iso_packet_descriptor iso_frame_desc[0];
					/* (in) ISO ONLY */
};

/*
 * urb->transfer_flags:
 *
 * Note: URB_DIR_IN/OUT is automatically set in usb_submit_urb().
 */
#define URB_SHORT_NOT_OK	0x0001	/* report short reads as errors */
#define URB_ISO_ASAP		0x0002	/* iso-only, urb->start_frame
					 * ignored */
#define URB_NO_TRANSFER_DMA_MAP	0x0004	/* urb->transfer_dma valid on submit */
#define URB_NO_FSBR		0x0020	/* UHCI-specific */
#define URB_ZERO_PACKET		0x0040	/* Finish bulk OUT with short packet */
#define URB_NO_INTERRUPT	0x0080	/* HINT: no non-error interrupt
					 * needed */
#define URB_FREE_BUFFER		0x0100	/* Free transfer buffer with the URB */

/* The following flags are used internally by usbcore and HCDs */
#define URB_DIR_IN		0x0200	/* Transfer from device to host */
#define URB_DIR_OUT		0
#define URB_DIR_MASK		URB_DIR_IN

#define URB_DMA_MAP_SINGLE	0x00010000	/* Non-scatter-gather mapping */
#define URB_DMA_MAP_PAGE	0x00020000	/* HCD-unsupported S-G */
#define URB_DMA_MAP_SG		0x00040000	/* HCD-supported S-G */
#define URB_MAP_LOCAL		0x00080000	/* HCD-local-memory mapping */
#define URB_SETUP_MAP_SINGLE	0x00100000	/* Setup packet DMA mapped */
#define URB_SETUP_MAP_LOCAL	0x00200000	/* HCD-local setup packet */
#define URB_DMA_SG_COMBINED	0x00400000	/* S-G entries were combined */
#define URB_ALIGNED_TEMP_BUFFER	0x00800000	/* Temp buffer was alloc'd */

struct int_queue;

/*
 * You can initialize platform's USB host or device
 * ports by passing this enum as an argument to
 * board_usb_init().
 */
enum usb_init_type {
	USB_INIT_HOST,
	USB_INIT_DEVICE
};

/**********************************************************************
 * this is how the lowlevel part communicate with the outer world
 */
#define CONFIG_USB_EHCI
#if defined(CONFIG_USB_UHCI) || defined(CONFIG_USB_OHCI) || \
	defined(CONFIG_USB_EHCI) || defined(CONFIG_USB_OHCI_NEW) || \
	defined(CONFIG_USB_SL811HS) || defined(CONFIG_USB_ISP116X_HCD) || \
	defined(CONFIG_USB_R8A66597_HCD) || defined(CONFIG_USB_DAVINCI) || \
	defined(CONFIG_USB_OMAP3) || defined(CONFIG_USB_DA8XX) || \
	defined(CONFIG_USB_BLACKFIN) || defined(CONFIG_USB_AM35X) || \
	defined(CONFIG_USB_MUSB_DSPS) || defined(CONFIG_USB_MUSB_AM35X) || \
	defined(CONFIG_USB_MUSB_OMAP2PLUS) || defined(CONFIG_USB_XHCI) || \
	defined(CONFIG_USB_DWC2)

int usb_lowlevel_init(void);
int usb_lowlevel_preinit(void);
int usb_lowlevel_postinit(void);
int usb_lowlevel_stop(int p);
int submit_bulk_msg(struct usb_device *dev, unsigned long pipe,
			void *buffer, int transfer_len);
int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
			int transfer_len, struct devrequest *setup);
int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
			int transfer_len, int interval);
void usb_event_poll(void);
void usb_set_u3_chk_conn_time(u32 uTimesec);

#ifdef CONFIG_USB_EHCI /* Only the ehci code has pollable int support */
struct int_queue *create_int_queue(struct usb_device *dev, unsigned long pipe,
	int queuesize, int elementsize, void *buffer);
int destroy_int_queue(struct usb_device *dev, struct int_queue *queue);
void *poll_int_queue(struct usb_device *dev, struct int_queue *queue);
#endif

/* Defines */
#define USB_UHCI_VEND_ID	0x8086
#define USB_UHCI_DEV_ID		0x7112

/*
 * PXA25x can only act as USB device. There are drivers
 * which works with USB CDC gadgets implementations.
 * Some of them have common routines which can be used
 * in boards init functions e.g. udc_disconnect() used for
 * forced device disconnection from host.
 */
#elif defined(CONFIG_USB_GADGET_PXA2XX)

extern void udc_disconnect(void);

#endif

/*
 * board-specific hardware initialization, called by
 * usb drivers and u-boot commands
 *
 * @param index USB controller number
 * @param init initializes controller as USB host or device
 */
int board_usb_init(int index, enum usb_init_type init);

/*
 * can be used to clean up after failed USB initialization attempt
 * vide: board_usb_init()
 *
 * @param index USB controller number for selective cleanup
 * @param init usb_init_type passed to board_usb_init()
 */
int board_usb_cleanup(int index, enum usb_init_type init);

#define	USB_PORT0	0
#define	USB_PORT1	1
#define	USB_PORT2	2
#define	USB_PORT3	3
#define	USB_PORT4	4 //XHCI port

#if defined (CONFIG_USB_STORAGE)


#define USB_MAX_STOR_DEV 1
block_dev_desc_t *usb_stor_get_dev(int index);
int usb_stor_scan(int mode);
int usb_stor_info(void);
// MSTAR start
#if defined (CONFIG_USB_STORAGE)
#define USB_WAIT_TIME 600 //ms
#endif
// MSTAR end

#endif

#ifdef CONFIG_USB_HOST_ETHER

#define USB_MAX_ETH_DEV 5
int usb_host_eth_scan(int mode);

#endif

#ifdef CONFIG_USB_KEYBOARD

int drv_usb_kbd_init(void);
int usb_kbd_deregister(int force);

#endif
/* routines */
int usb_init(int port); /* initialize the USB Controller */
int usb_preinit(int port);
int usb_post_init(int port);
int usb_stop(int p); /* stop the USB Controller */
int usb_stop_xhci(int p, int bDisablePower);



int usb_set_protocol(struct usb_device *dev, int ifnum, int protocol);
int usb_set_idle(struct usb_device *dev, int ifnum, int duration,
			int report_id);
struct usb_device *usb_get_dev_index(int index);
int usb_control_msg(struct usb_device *dev, unsigned int pipe,
			unsigned char request, unsigned char requesttype,
			unsigned short value, unsigned short index,
			void *data, unsigned short size, int timeout);
int usb_bulk_msg(struct usb_device *dev, unsigned int pipe,
			void *data, int len, int *actual_length, int timeout);
#if (MP_USB_MSTAR==1)
int usb_submit_int_msg(struct usb_device *dev, unsigned long pipe,
			void *buffer, int transfer_len, int *actual_length, int interval);
#else
int usb_submit_int_msg(struct usb_device *dev, unsigned long pipe,
			void *buffer, int transfer_len, int interval);
#endif
int usb_disable_asynch(int disable);
int usb_maxpacket(struct usb_device *dev, unsigned long pipe);
void wait_ms(unsigned long ms);
int usb_get_configuration_no(struct usb_device *dev, unsigned char *buffer,
				int cfgno);
int usb_get_report(struct usb_device *dev, int ifnum, unsigned char type,
			unsigned char id, void *buf, int size);
int usb_get_class_descriptor(struct usb_device *dev, int ifnum,
			unsigned char type, unsigned char id, void *buf,
			int size);
int usb_clear_halt(struct usb_device *dev, int pipe);
int usb_string(struct usb_device *dev, int index, char *buf, size_t size);
int usb_set_interface(struct usb_device *dev, int interface, int alternate);

/* big endian -> little endian conversion */
/* some CPUs are already little endian e.g. the ARM920T */
#define __swap_16(x) \
	({ unsigned short x_ = (unsigned short)x; \
	 (unsigned short)( \
		((x_ & 0x00FFU) << 8) | ((x_ & 0xFF00U) >> 8)); \
	})
#define __swap_32(x) \
	({ unsigned long x_ = (unsigned long)x; \
	 (unsigned long)( \
		((x_ & 0x000000FFUL) << 24) | \
		((x_ & 0x0000FF00UL) <<	 8) | \
		((x_ & 0x00FF0000UL) >>	 8) | \
		((x_ & 0xFF000000UL) >> 24)); \
	})

#ifdef __LITTLE_ENDIAN
# define swap_16(x) (x)
# define swap_32(x) (x)
#else
# define swap_16(x) __swap_16(x)
# define swap_32(x) __swap_32(x)
#endif

/*
 * Calling this entity a "pipe" is glorifying it. A USB pipe
 * is something embarrassingly simple: it basically consists
 * of the following information:
 *  - device number (7 bits)
 *  - endpoint number (4 bits)
 *  - current Data0/1 state (1 bit)
 *  - direction (1 bit)
 *  - speed (2 bits)
 *  - max packet size (2 bits: 8, 16, 32 or 64)
 *  - pipe type (2 bits: control, interrupt, bulk, isochronous)
 *
 * That's 18 bits. Really. Nothing more. And the USB people have
 * documented these eighteen bits as some kind of glorious
 * virtual data structure.
 *
 * Let's not fall in that trap. We'll just encode it as a simple
 * unsigned int. The encoding is:
 *
 *  - max size:		bits 0-1	(00 = 8, 01 = 16, 10 = 32, 11 = 64)
 *  - direction:	bit 7		(0 = Host-to-Device [Out],
 *					(1 = Device-to-Host [In])
 *  - device:		bits 8-14
 *  - endpoint:		bits 15-18
 *  - Data0/1:		bit 19
 *  - pipe type:	bits 30-31	(00 = isochronous, 01 = interrupt,
 *					 10 = control, 11 = bulk)
 *
 * Why? Because it's arbitrary, and whatever encoding we select is really
 * up to us. This one happens to share a lot of bit positions with the UHCI
 * specification, so that much of the uhci driver can just mask the bits
 * appropriately.
 */
/* Create various pipes... */
#define create_pipe(dev,endpoint) \
		(((dev)->devnum << 8) | ((endpoint) << 15) | \
		(dev)->maxpacketsize)
#define default_pipe(dev) ((dev)->speed << 26)

#define usb_sndctrlpipe(dev, endpoint)	((PIPE_CONTROL << 30) | \
					 create_pipe(dev, endpoint))
#define usb_rcvctrlpipe(dev, endpoint)	((PIPE_CONTROL << 30) | \
					 create_pipe(dev, endpoint) | \
					 USB_DIR_IN)
#define usb_sndisocpipe(dev, endpoint)	((PIPE_ISOCHRONOUS << 30) | \
					 create_pipe(dev, endpoint))
#define usb_rcvisocpipe(dev, endpoint)	((PIPE_ISOCHRONOUS << 30) | \
					 create_pipe(dev, endpoint) | \
					 USB_DIR_IN)
#define usb_sndbulkpipe(dev, endpoint)	((PIPE_BULK << 30) | \
					 create_pipe(dev, endpoint))
#define usb_rcvbulkpipe(dev, endpoint)	((PIPE_BULK << 30) | \
					 create_pipe(dev, endpoint) | \
					 USB_DIR_IN)
#define usb_sndintpipe(dev, endpoint)	((PIPE_INTERRUPT << 30) | \
					 create_pipe(dev, endpoint))
#define usb_rcvintpipe(dev, endpoint)	((PIPE_INTERRUPT << 30) | \
					 create_pipe(dev, endpoint) | \
					 USB_DIR_IN)
#define usb_snddefctrl(dev)		((PIPE_CONTROL << 30) | \
					 default_pipe(dev))
#define usb_rcvdefctrl(dev)		((PIPE_CONTROL << 30) | \
					 default_pipe(dev) | \
					 USB_DIR_IN)

/* The D0/D1 toggle bits */
#define usb_gettoggle(dev, ep, out) (((dev)->toggle[out] >> ep) & 1)
#define usb_dotoggle(dev, ep, out)  ((dev)->toggle[out] ^= (1 << ep))
#define usb_settoggle(dev, ep, out, bit) ((dev)->toggle[out] = \
						((dev)->toggle[out] & \
						 ~(1 << ep)) | ((bit) << ep))

/* Endpoint halt control/status */
#define usb_endpoint_out(ep_dir)	(((ep_dir >> 7) & 1) ^ 1)
#define usb_endpoint_halt(dev, ep, out) ((dev)->halted[out] |= (1 << (ep)))
#define usb_endpoint_running(dev, ep, out) ((dev)->halted[out] &= ~(1 << (ep)))
#define usb_endpoint_halted(dev, ep, out) ((dev)->halted[out] & (1 << (ep)))

#define usb_packetid(pipe)	(((pipe) & USB_DIR_IN) ? USB_PID_IN : \
				 USB_PID_OUT)

#define usb_pipeout(pipe)	((((pipe) >> 7) & 1) ^ 1)
#define usb_pipein(pipe)	(((pipe) >> 7) & 1)
#define usb_pipedevice(pipe)	(((pipe) >> 8) & 0x7f)
#define usb_pipe_endpdev(pipe)	(((pipe) >> 8) & 0x7ff)
#define usb_pipeendpoint(pipe)	(((pipe) >> 15) & 0xf)
#define usb_pipedata(pipe)	(((pipe) >> 19) & 1)
#define usb_pipetype(pipe)	(((pipe) >> 30) & 3)
#define usb_pipeisoc(pipe)	(usb_pipetype((pipe)) == PIPE_ISOCHRONOUS)
#define usb_pipeint(pipe)	(usb_pipetype((pipe)) == PIPE_INTERRUPT)
#define usb_pipecontrol(pipe)	(usb_pipetype((pipe)) == PIPE_CONTROL)
#define usb_pipebulk(pipe)	(usb_pipetype((pipe)) == PIPE_BULK)

#define usb_pipe_ep_index(pipe)	\
		usb_pipecontrol(pipe) ? (usb_pipeendpoint(pipe) * 2) : \
				((usb_pipeendpoint(pipe) * 2) - \
				 (usb_pipein(pipe) ? 0 : 1))

/*************************************************************************
 * Hub Stuff
 */
struct usb_port_status {
	unsigned short wPortStatus;
	unsigned short wPortChange;
} __attribute__ ((packed));

struct usb_hub_status {
	unsigned short wHubStatus;
	unsigned short wHubChange;
} __attribute__ ((packed));


/* Hub descriptor */
struct usb_hub_descriptor {
	unsigned char  bLength;
	unsigned char  bDescriptorType;
	unsigned char  bNbrPorts;
	unsigned short wHubCharacteristics;
	unsigned char  bPwrOn2PwrGood;
	unsigned char  bHubContrCurrent;
	unsigned char  DeviceRemovable[(USB_MAXCHILDREN+1+7)/8];
	unsigned char  PortPowerCtrlMask[(USB_MAXCHILDREN+1+7)/8];
	/* DeviceRemovable and PortPwrCtrlMask want to be variable-length
	   bitmaps that hold max 255 entries. (bit0 is ignored) */
} __attribute__ ((packed));

#define USB_MAXIADS		(USB_MAXINTERFACES/2)
/*
 * Descriptor types ... USB 2.0 spec table 9.5
 */
#define USB_DT_DEVICE			0x01
#define USB_DT_CONFIG			0x02
#define USB_DT_STRING			0x03
#define USB_DT_INTERFACE		0x04
#define USB_DT_ENDPOINT			0x05
#define USB_DT_DEVICE_QUALIFIER		0x06
#define USB_DT_OTHER_SPEED_CONFIG	0x07
#define USB_DT_INTERFACE_POWER		0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG			0x09
#define USB_DT_DEBUG			0x0a
#define USB_DT_INTERFACE_ASSOCIATION	0x0b
/* these are from the Wireless USB spec */
#define USB_DT_SECURITY			0x0c
#define USB_DT_KEY			0x0d
#define USB_DT_ENCRYPTION_TYPE		0x0e
#define USB_DT_BOS			0x0f
#define USB_DT_DEVICE_CAPABILITY	0x10
#define USB_DT_WIRELESS_ENDPOINT_COMP	0x11
#define USB_DT_WIRE_ADAPTER		0x21
#define USB_DT_RPIPE			0x22
#define USB_DT_CS_RADIO_CONTROL		0x23
/* From the T10 UAS specification */
#define USB_DT_PIPE_USAGE		0x24
/* From the USB 3.0 spec */
#define	USB_DT_SS_ENDPOINT_COMP		0x30
#define USB_DT_SS_EP_COMP_SIZE		6

struct usb_hub_device {
	struct usb_device *pusb_dev;
	struct usb_hub_descriptor desc;
};

int usb_hub_probe(struct usb_device *dev, int ifnum);
void usb_hub_reset(void);
int hub_port_reset(struct usb_device *dev, int port,
			  unsigned short *portstat);

struct usb_device *usb_alloc_new_device(void);

int usb_new_device(struct usb_device *dev);
void usb_free_device(void);
int usb_alloc_device(struct usb_device *dev);

#ifdef CONFIG_ENABLE_FIRST_EHC
#define ENABLE_FIRST_EHC
#endif

#ifdef CONFIG_ENABLE_SECOND_EHC
#define ENABLE_SECOND_EHC
#endif

#ifdef CONFIG_ENABLE_THIRD_EHC
#define ENABLE_THIRD_EHC
#endif

#endif /*_USB_H_ */
