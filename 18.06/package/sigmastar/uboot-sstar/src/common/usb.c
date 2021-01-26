/*
 * Most of this source has been derived from the Linux USB
 * project:
 * (C) Copyright Linus Torvalds 1999
 * (C) Copyright Johannes Erdfelt 1999-2001
 * (C) Copyright Andreas Gal 1999
 * (C) Copyright Gregory P. Smith 1999
 * (C) Copyright Deti Fliegl 1999 (new USB architecture)
 * (C) Copyright Randy Dunlap 2000
 * (C) Copyright David Brownell 2000 (kernel hotplug, usb_device_id)
 * (C) Copyright Yggdrasil Computing, Inc. 2000
 *     (usb_device_id matching changes by Adam J. Richter)
 *
 * Adapted for U-Boot:
 * (C) Copyright 2001 Denis Peter, MPL AG Switzerland
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * How it works:
 *
 * Since this is a bootloader, the devices will not be automatic
 * (re)configured on hotplug, but after a restart of the USB the
 * device should work.
 *
 * For each transfer (except "Interrupt") we wait for completion.
 */
#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <linux/compiler.h>
#include <linux/ctype.h>
#include <asm/byteorder.h>
//#include <asm/unaligned.h>
#include <errno.h>
#include <usb.h>
#ifdef CONFIG_4xx
#include <asm/4xx_pci.h>
#endif

#define USB_BUFSIZ	512

static struct usb_device usb_dev[USB_MAX_DEVICE];
static int dev_index;
static int asynch_allowed;
//static struct devrequest setup_packet;

#if (MP_USB_MSTAR==1)
extern void SetControl1MaxPacketEx(unsigned char max, int port);
extern void USB_Bulk_InitEx(struct usb_device *dev, int port);
extern void USB_Interrupt_InitEx(struct usb_device *dev, int port);
extern void Usb_host_SetEverInited(int val);
#ifdef ENABLE_USB_LAN_MODULE
extern int usb_lan_initialize(struct usb_device *udev);
#endif
#endif /* MP_USB_MSTAR==1 */

#define mdelay(n) ({unsigned long msec=(n); while (msec--) udelay(1000);})
#if defined(CONFIG_USB_XHCI)  && defined(ENABLE_XHC)
extern struct xhci_hcd     ms_xhci;
extern int gXHCI_rh_port_sel;
extern int USB3_init(void);
extern void ms_usb_reset_ept(struct usb_device *pDev, unsigned int ept_addr);
extern int USB3_enumerate(struct usb_device *pdev);
extern int ms_xhci_dev_enum(struct xhci_hcd *xhci, struct usb_device *dev, int rh_port);
extern void MDrv_UsbEnableXhciPower(u8 enable);
#endif

char usb_started; /* flag for the started/stopped USB status */

extern void SetControl1MaxPacket(unsigned char max);
extern void SetControl1MaxPacket2(unsigned char max);
extern void SetControl1MaxPacket3(unsigned char max);
extern void SetControl1MaxPacket4(unsigned char max);


extern int hub_port_reset(struct usb_device *dev, int port,
            unsigned short *portstat);

extern void USB_Bulk_Init(struct usb_device *dev);
extern void USB_Bulk_Init2(struct usb_device *dev);
extern void USB_Bulk_Init3(struct usb_device *dev);

extern void msAPI_Timer_Delayms(unsigned long u32DelayTime); //unit = ms


#ifndef CONFIG_USB_MAX_CONTROLLER_COUNT
#define CONFIG_USB_MAX_CONTROLLER_COUNT 1
#endif

int usb_scan_devices(void);

int usb_hub_probe(struct usb_device *dev, int ifnum);
void usb_hub_reset(void);

/***********************************************************************
 * wait_ms
 */

void wait_ms(unsigned long ms)
{
#if 0
    msAPI_Timer_Delayms(ms);
#else
    while(ms-->0)
        udelay(1000);
#endif
}

/***************************************************************************
 * Init USB Device
 */ 
int UsbPortSelect;
int usb_preinit(int port)
{
    int result=0;

#if 0
    //#if defined (CONFIG_TITANIA8) || defined(CONFIG_JANUS2)
    // Force USB to use MIU0.
    *((unsigned char volatile *)(0xBF200000+0x600*2+0xF0*2+1)) =  (*((unsigned char volatile *)(0xBF200000+0x600*2+0xF0*2+1))
              & 0xF1) ;
    *((unsigned char volatile *)(0xBF200000+0x1200*2+0xF0*2+1)) =  (*((unsigned char volatile *)(0xBF200000+0x1200*2+0xF0*2+1))
              & 0xF1) ;
#endif

	printf("Check USB port[%d]:\n", port);
        UsbPortSelect=port;

    dev_index=0;
    asynch_allowed=1;
    usb_hub_reset();
    /* init low_level USB */
    //retry:
    result = usb_lowlevel_preinit();
    /* if lowlevel init is OK, scan the bus for devices i.e. search HUBs and configure them */
    if(result==0) {
        printf("scanning bus for devices... ");
        //running=1;
        result=usb_scan_devices();
        if(result) {
            printf("Error, bad or unsupported device...\n");
            usb_started = 0;
            return -1;
        }
        usb_started = 1;
        return 0;
    }
    else {
        //printf("Error, couldn't init Lowlevel part\n");
        usb_started = 0;
        return -1;
    }
}

int _usb_init(int port)
{
    int result=0;

#if (MP_USB_MSTAR==1)
#if   defined(ENABLE_FIFTH_EHC)
	const char u8UsbPortCount = 5;
#elif defined(ENABLE_FOURTH_EHC)
	const char u8UsbPortCount = 4;
#elif defined(ENABLE_THIRD_EHC)
	const char u8UsbPortCount = 3;
#elif defined(ENABLE_SECOND_EHC)
	const char u8UsbPortCount = 2;
#else
	const char u8UsbPortCount = 1;
#endif
	char idx;

  USB_Power_On();

#if 0
    //#if defined (CONFIG_TITANIA8) || defined(CONFIG_JANUS2)
    // Force USB to use MIU0.
    *((unsigned char volatile *)(0xBF200000+0x600*2+0xF0*2+1)) =  (*((unsigned char volatile *)(0xBF200000+0x600*2+0xF0*2+1))
              & 0xF1) ;
    *((unsigned char volatile *)(0xBF200000+0x1200*2+0xF0*2+1)) =  (*((unsigned char volatile *)(0xBF200000+0x1200*2+0xF0*2+1))
              & 0xF1) ;
#endif

	//Stop USB controller to prevent multi HCDs accessing the same data content
	for(idx=0; idx<u8UsbPortCount; idx++)
	{
		usb_stop(idx);
	}

	printf("Check USB port[%d]:\n", port);
        UsbPortSelect=port;
#endif /* MP_USB_MSTAR==1 */
    dev_index=0;
    asynch_allowed=1;
    usb_hub_reset();
    /* init low_level USB */
    //retry:
    result = usb_lowlevel_init();
    /* if lowlevel init is OK, scan the bus for devices i.e. search HUBs and configure them */
    if(result==0) {
        printf("scanning bus for devices... ");
        //running=1;
        result=usb_scan_devices();
        if(result) {
            printf("Error, bad or unsupported device...\n");
            usb_started = 0;
            return -1;
        }
        usb_started = 1;
#if (CONFIG_COMMANDS & CFG_CMD_FAT)
        {
            char cmd_buf[32];
            extern int snprintf(char *str, size_t size, const char *fmt, ...);
            memset(cmd_buf, 0 , sizeof(cmd_buf));
            snprintf(cmd_buf, sizeof(cmd_buf), "setenv %s", ENV_PARTNO);
            run_command(cmd_buf, 0);
        }
#endif
        return 0;
    }
    else {
        printf("Error, couldn't init Lowlevel part\n");
        usb_started = 0;
        return -1;
    }
}

/* TONY
 * target_ext_hub_port:
 * -1:		skip nothing
 *  0:		skip all port on exthub, but not roothub
 *  1~15:	skip all port on roothub, but not exthub
 */
int target_ext_hub_port = -1;

int usb_init(int port)
{
    target_ext_hub_port = -1;
    return _usb_init(port);
}

int usb_init_exhub_port(int port, int ext_hub_port)
{
    printf("[USB] usb_init_exhub_port++ %d\n", ext_hub_port);
    if (ext_hub_port >= 0 && ext_hub_port < 16)
    {
        target_ext_hub_port = ext_hub_port;
    }
    else
        return 0;

    target_ext_hub_port = ext_hub_port;
    printf("[USB] target exthub port: %d\n", target_ext_hub_port);

    return _usb_init(port);
}

int usb_post_init(int port)
{
    int result=0;

#if 0
    //#if defined (CONFIG_TITANIA8) || defined(CONFIG_JANUS2)
    // Force USB to use MIU0.
    *((unsigned char volatile *)(0xBF200000+0x600*2+0xF0*2+1)) =  (*((unsigned char volatile *)(0xBF200000+0x600*2+0xF0*2+1))
              & 0xF1) ;
    *((unsigned char volatile *)(0xBF200000+0x1200*2+0xF0*2+1)) =  (*((unsigned char volatile *)(0xBF200000+0x1200*2+0xF0*2+1))
              & 0xF1) ;
#endif

	printf("Check USB port[%d]:\n", port);
        UsbPortSelect=port;
    dev_index=0;
    asynch_allowed=1;
    printf("Check USB port[%d]:\n", port);
    //usb_hub_reset();
    /* init low_level USB */
    //retry:
    result = usb_lowlevel_postinit();
    /* if lowlevel init is OK, scan the bus for devices i.e. search HUBs and configure them */
    if(result==0) {
        printf("scanning bus for devices... ");
        //running=1;
        result=usb_scan_devices();
        if(result) {
            printf("Error, bad or unsupported device...\n");
            usb_started = 0;
            return -1;
        }
        usb_started = 1;
#if (CONFIG_COMMANDS & CFG_CMD_FAT)
        {
            char cmd_buf[32];
            extern int snprintf(char *str, size_t size, const char *fmt, ...);
            memset(cmd_buf, 0 , sizeof(cmd_buf));
            snprintf(cmd_buf, sizeof(cmd_buf), "setenv %s", ENV_PARTNO);
            run_command(cmd_buf, 0);
        }
#endif
        return 0;
    }
    else {
        printf("Error, couldn't init Lowlevel part\n");
        usb_started = 0;
        return -1;
    }
}

/******************************************************************************
 * Stop USB this stops the LowLevel Part and deregisters USB devices.
 */
int usb_stop(int p)
{
	int res = 0;

	asynch_allowed = 1;
	usb_started = 0;
	usb_hub_reset();
	res = usb_lowlevel_stop(p);

	return res;
}

/*
 * disables the asynch behaviour of the control message. This is used for data
 * transfers that uses the exclusiv access to the control and bulk messages.
 * Returns the old value so it can be restored later.
 */
int usb_disable_asynch(int disable)
{
	int old_value = asynch_allowed;

	asynch_allowed = !disable;
	return old_value;
}


/*-------------------------------------------------------------------
 * Message wrappers.
 *
 */

/*
 * submits an Interrupt Message
 */
#if (MP_USB_MSTAR==1)
int usb_submit_int_msg(struct usb_device *dev, unsigned long pipe,
			void *buffer, int transfer_len, int *actual_length, int interval)
#else
int usb_submit_int_msg(struct usb_device *dev, unsigned long pipe,
			void *buffer, int transfer_len, int interval)
#endif /* MP_USB_MSTAR==1 */
{
#if (MP_USB_MSTAR==1)
	 submit_int_msg(dev, pipe, buffer, transfer_len, interval);
	 if (USB_ST_DISCONNECT==dev->status)
		return -1;
	 *actual_length = dev->act_len;
	 debug("usb_submit_int_msg:act_len:%d\n", dev->act_len);
	 return 0;
#else
	return submit_int_msg(dev, pipe, buffer, transfer_len, interval);
#endif /* MP_USB_MSTAR==1 */
}

/*
 * submits a control message and waits for comletion (at least timeout * 1ms)
 * If timeout is 0, we don't wait for completion (used as example to set and
 * clear keyboards LEDs). For data transfers, (storage transfers) we don't
 * allow control messages with 0 timeout, by previousely resetting the flag
 * asynch_allowed (usb_disable_asynch(1)).
 * returns the transfered length if OK or -1 if error. The transfered length
 * and the current status are stored in the dev->act_len and dev->status.
 */
int usb_control_msg(struct usb_device *dev, unsigned int pipe,
			unsigned char request, unsigned char requesttype,
			unsigned short value, unsigned short index,
			void *data, unsigned short size, int timeout)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct devrequest, setup_packet, 1);

	if ((timeout == 0) && (!asynch_allowed)) {
		/* request for a asynch control pipe is not allowed */
		return -1;
	}

	/* set setup command */
	setup_packet->requesttype = requesttype;
	setup_packet->request = request;
	setup_packet->value = cpu_to_le16(value);
	setup_packet->index = cpu_to_le16(index);
	setup_packet->length = cpu_to_le16(size);
	debug("usb_control_msg: request: 0x%X, requesttype: 0x%X, " \
	      "value 0x%X index 0x%X length 0x%X\n",
	      request, requesttype, value, index, size);
	dev->status = USB_ST_NOT_PROC; /*not yet processed */

	if (submit_control_msg(dev, pipe, data, size, setup_packet) < 0)
		return -1;
	if (USB_ST_DISCONNECT==dev->status)
		return -3; //USB_STOR_TRANSPORT_NODEV
	if(timeout==0) {
		return (int)size;
	}


	/*
	 * Wait for status to update until timeout expires, USB driver
	 * interrupt handler may set the status when the USB operation has
	 * been completed.
	 */
	while(timeout>0) {
		if(!(dev->status & USB_ST_NOT_PROC))
			break;
		wait_ms(1);
		timeout--;

	}
	if(dev->status==0)
		return dev->act_len;
	else {
		return -1;
	}


}

/*-------------------------------------------------------------------
 * submits bulk message, and waits for completion. returns 0 if Ok or
 * -1 if Error.
 * synchronous behavior
 */
int usb_bulk_msg(struct usb_device *dev, unsigned int pipe,
			void *data, int len, int *actual_length, int timeout)
{
	if (len < 0)
		return -1;
	dev->status = USB_ST_NOT_PROC; /*not yet processed */
	if (submit_bulk_msg(dev, pipe, data, len) < 0)
		return -1;
	if (USB_ST_DISCONNECT==dev->status)
		return -3; //USB_STOR_TRANSPORT_NODEV	
	while(timeout>0) {
		if(!(dev->status & USB_ST_NOT_PROC))
			break;
		wait_ms(1);
		timeout--;

	}
	*actual_length = dev->act_len;
	if (dev->status == 0)
		return 0;
	else
		return -1;
}


/*-------------------------------------------------------------------
 * Max Packet stuff
 */

/*
 * returns the max packet size, depending on the pipe direction and
 * the configurations values
 */
int usb_maxpacket(struct usb_device *dev, unsigned long pipe)
{
	/* direction is out -> use emaxpacket out */
	if ((pipe & USB_DIR_IN) == 0)
		return dev->epmaxpacketout[((pipe>>15) & 0xf)];
	else
		return dev->epmaxpacketin[((pipe>>15) & 0xf)];
}

/*
 * The routine usb_set_maxpacket_ep() is extracted from the loop of routine
 * usb_set_maxpacket(), because the optimizer of GCC 4.x chokes on this routine
 * when it is inlined in 1 single routine. What happens is that the register r3
 * is used as loop-count 'i', but gets overwritten later on.
 * This is clearly a compiler bug, but it is easier to workaround it here than
 * to update the compiler (Occurs with at least several GCC 4.{1,2},x
 * CodeSourcery compilers like e.g. 2007q3, 2008q1, 2008q3 lite editions on ARM)
 *
 * NOTE: Similar behaviour was observed with GCC4.6 on ARMv5.
 */
static void noinline
usb_set_maxpacket_ep(struct usb_device *dev, struct usb_endpoint_descriptor *ep)
{
	int b;
//	struct usb_endpoint_descriptor *ep;
	u16 ep_wMaxPacketSize=0;
	ep_wMaxPacketSize=ep_wMaxPacketSize;
	//ep = &dev->config.if_desc[if_idx].ep_desc[ep_idx];

	b = ep->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
	ep_wMaxPacketSize = get_unaligned(&ep->wMaxPacketSize);

	if ((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
						USB_ENDPOINT_XFER_CONTROL) {
		/* Control => bidirectional */
		dev->epmaxpacketout[b] = ep->wMaxPacketSize;
		dev->epmaxpacketin[b] = ep->wMaxPacketSize;
		debug("##Control EP epmaxpacketout/in[%d] = %d\n",
		      b, dev->epmaxpacketin[b]);
	} else {
		if ((ep->bEndpointAddress & 0x80) == 0) {
			/* OUT Endpoint */
			if (ep->wMaxPacketSize > dev->epmaxpacketout[b]) {
				dev->epmaxpacketout[b] = ep->wMaxPacketSize;
				debug("##EP epmaxpacketout[%d] = %d\n",
				      b, dev->epmaxpacketout[b]);
			}
		} else {
			/* IN Endpoint */
			if (ep->wMaxPacketSize > dev->epmaxpacketin[b]) {
				dev->epmaxpacketin[b] = ep->wMaxPacketSize;
				debug("##EP epmaxpacketin[%d] = %d\n",
				      b, dev->epmaxpacketin[b]);
			}
		} /* if out */
	} /* if control */
}

/*
 * set the max packed value of all endpoints in the given configuration
 */
int usb_set_maxpacket(struct usb_device *dev)
{
	int i, ii;

	for (i = 0; i < dev->config.desc.bNumInterfaces; i++)
		for (ii = 0; ii < dev->config.if_desc[i].desc.bNumEndpoints; ii++)
			usb_set_maxpacket_ep(dev,
					  &dev->config.if_desc[i].ep_desc[ii]);

	return 0;
}

/*******************************************************************************
 * Parse the config, located in buffer, and fills the dev->config structure.
 * Note that all little/big endian swapping are done automatically.
 * (wTotalLength has already been swapped and sanitized when it was read.)
 */
int usb_parse_config(struct usb_device *dev, unsigned char *buffer, int cfgno)
{
	struct usb_descriptor_header *head;
	int index, ifno, epno, curr_if_num;
	u16 ep_wMaxPacketSize=0;
	struct usb_interface *if_desc = NULL;
	ep_wMaxPacketSize=ep_wMaxPacketSize;
	ifno = -1;
	epno = -1;
	curr_if_num = -1;

	dev->configno = cfgno;
	head = (struct usb_descriptor_header *) &buffer[0];
	if (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
			head->bDescriptorType);
		return -1;
	}
	if (head->bLength != USB_DT_CONFIG_SIZE) {
		printf("ERROR: Invalid USB CFG length (%d)\n", head->bLength);
		return -1;
	}
	memcpy(&dev->config, head, USB_DT_CONFIG_SIZE);
	dev->config.no_of_if = 0;

	index = dev->config.desc.bLength;
	/* Ok the first entry must be a configuration entry,
	 * now process the others */
	head = (struct usb_descriptor_header *) &buffer[index];
	while (index + 1 < dev->config.desc.wTotalLength && head->bLength) {
		switch (head->bDescriptorType) {
		case USB_DT_INTERFACE:
			if (head->bLength != USB_DT_INTERFACE_SIZE) {
				printf("ERROR: Invalid USB IF length (%d)\n",
					head->bLength);
				break;
			}
			if (index + USB_DT_INTERFACE_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB IF descriptor overflowed buffer!\n");
				break;
			}
			if (((struct usb_interface_descriptor *) \
			     head)->bInterfaceNumber != curr_if_num) {
				/* this is a new interface, copy new desc */
				ifno = dev->config.no_of_if;
				if (ifno >= USB_MAXINTERFACES) {
					puts("Too many USB interfaces!\n");
					/* try to go on with what we have */
					return 1;
				}
				if_desc = &dev->config.if_desc[ifno];
				dev->config.no_of_if++;
				memcpy(if_desc, head,
					USB_DT_INTERFACE_SIZE);
				if_desc->no_of_ep = 0;
				if_desc->num_altsetting = 1;
				curr_if_num =
				     if_desc->desc.bInterfaceNumber;
			} else {
				/* found alternate setting for the interface */
				if (ifno >= 0) {
					if_desc = &dev->config.if_desc[ifno];
					if_desc->num_altsetting++;
				}
			}
			break;
		case USB_DT_ENDPOINT:
			if (head->bLength != USB_DT_ENDPOINT_SIZE) {
				printf("ERROR: Invalid USB EP length (%d)\n",
					head->bLength);
				break;
			}
			if (index + USB_DT_ENDPOINT_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EP descriptor overflowed buffer!\n");
				break;
			}
			if (ifno < 0) {
				puts("Endpoint descriptor out of order!\n");
				break;
			}
			epno = dev->config.if_desc[ifno].no_of_ep;
			if_desc = &dev->config.if_desc[ifno];
			if (epno > USB_MAXENDPOINTS) {
				printf("Interface %d has too many endpoints!\n",
					if_desc->desc.bInterfaceNumber);
				return 1;
			}
			/* found an endpoint */
			if_desc->no_of_ep++;
			memcpy(&if_desc->ep_desc[epno], head,
				USB_DT_ENDPOINT_SIZE);
			le16_to_cpus(&(dev->config.if_desc[ifno].ep_desc[epno].\
							       wMaxPacketSize));
			debug("if %d, ep %d\n", ifno, epno);
			break;
		case USB_DT_SS_ENDPOINT_COMP:
			if (head->bLength != USB_DT_SS_EP_COMP_SIZE) {
				printf("ERROR: Invalid USB EPC length (%d)\n",
					head->bLength);
				break;
			}
			if (index + USB_DT_SS_EP_COMP_SIZE >
			    dev->config.desc.wTotalLength) {
				puts("USB EPC descriptor overflowed buffer!\n");
				break;
			}
			if (ifno < 0 || epno < 0) {
				puts("EPC descriptor out of order!\n");
				break;
			}
			if_desc = &dev->config.if_desc[ifno];
			memcpy(&if_desc->ss_ep_comp_desc[epno], head,
				USB_DT_SS_EP_COMP_SIZE);
			break;
		default:
			if (head->bLength == 0)
				return 1;

			debug("unknown Description Type : %x\n",
			      head->bDescriptorType);

#ifdef DEBUG
			{
				unsigned char *ch = (unsigned char *)head;
				int i;

				for (i = 0; i < head->bLength; i++)
					debug("%02X ", *ch++);
				debug("\n\n\n");
			}
#endif
			break;
		}
		index += head->bLength;
		head = (struct usb_descriptor_header *)&buffer[index];
	}
	return 1;
}

/***********************************************************************
 * Clears an endpoint
 * endp: endpoint number in bits 0-3;
 * direction flag in bit 7 (1 = IN, 0 = OUT)
 */
int usb_clear_halt(struct usb_device *dev, int pipe)
{
	int result;
	int endp = usb_pipeendpoint(pipe)|(usb_pipein(pipe)<<7);

	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				 USB_REQ_CLEAR_FEATURE, USB_RECIP_ENDPOINT, 0,
				 endp, NULL, 0, USB_CNTL_TIMEOUT * 3);

	/* don't clear if failed */
	if (result < 0)
		return result;

	/*
	 * NOTE: we do not get status and verify reset was successful
	 * as some devices are reported to lock up upon this check..
	 */

	usb_endpoint_running(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/* toggle is reset on clear */
	usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe), 0);
	return 0;
}


/**********************************************************************
 * get_descriptor type
 */
int usb_get_descriptor(struct usb_device *dev, unsigned char type,
			unsigned char index, void *buf, int size)
{
	int res;
	res = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(type << 8) + index, 0,
			buf, size, USB_CNTL_TIMEOUT);
	return res;
}

/**********************************************************************
 * gets configuration cfgno and store it in the buffer
 */
int usb_get_configuration_no(struct usb_device *dev,
			     unsigned char *buffer, int cfgno)
{
	int result;
	unsigned int length;
	struct usb_config_descriptor *config;

	config = (struct usb_config_descriptor *)&buffer[0];
	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, 9);
	if (result < 9) {
		if (result < 0)
			printf("unable to get descriptor, error %lX\n",
				dev->status);
		else
			printf("config descriptor too short " \
				"(expected %i, got %i)\n", 9, result);
		return -1;
	}
	length = le16_to_cpu(config->wTotalLength);

	if (length > USB_BUFSIZ) {
		printf("%s: failed to get descriptor - too long: %d\n",
			__func__, length);
		return -1;
	}

	result = usb_get_descriptor(dev, USB_DT_CONFIG, cfgno, buffer, length);
	debug("get_conf_no %d Result %d, wLength %d\n", cfgno, result, length);
	config->wTotalLength = length; /* validated, with CPU byte order */

	return result;
}

/********************************************************************
 * set address of a device to the value in dev->devnum.
 * This can only be done by addressing the device via the default address (0)
 */
int usb_set_address(struct usb_device *dev,int addr)
{
	int res;

	debug("set address %d\n", dev->devnum);
	res = usb_control_msg(dev, usb_snddefctrl(dev),
				USB_REQ_SET_ADDRESS, 0,
				addr, 0,
				NULL, 0, USB_CNTL_TIMEOUT);
	return res;
}

/********************************************************************
 * set interface number to interface
 */
int usb_set_interface(struct usb_device *dev, int interface, int alternate)
{
	struct usb_interface *if_face = NULL;
	int ret, i;

	for (i = 0; i < dev->config.desc.bNumInterfaces; i++) {
		if (dev->config.if_desc[i].desc.bInterfaceNumber == interface) {
			if_face = &dev->config.if_desc[i];
			break;
		}
	}
	if (!if_face) {
		printf("selecting invalid interface %d", interface);
		return -1;
	}
	/*
	 * We should return now for devices with only one alternate setting.
	 * According to 9.4.10 of the Universal Serial Bus Specification
	 * Revision 2.0 such devices can return with a STALL. This results in
	 * some USB sticks timeouting during initialization and then being
	 * unusable in U-Boot.
	 */
	if (if_face->num_altsetting == 1)
		return 0;

	ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_INTERFACE, USB_RECIP_INTERFACE,
				alternate, interface, NULL, 0,
				USB_CNTL_TIMEOUT * 5);
	if (ret < 0)
		return ret;

	return 0;
}

/********************************************************************
 * set configuration number to configuration
 */
int usb_set_configuration(struct usb_device *dev, int configuration)
{
	int res;
	debug("set configuration %d\n", configuration);
	/* set setup command */
	res = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_CONFIGURATION, 0,
				configuration, 0,
				NULL, 0, USB_CNTL_TIMEOUT);
	if (res == 0) {
		dev->toggle[0] = 0;
		dev->toggle[1] = 0;
		return 0;
	} else
		return -1;
}

/********************************************************************
 * set protocol to protocol
 */
int usb_set_protocol(struct usb_device *dev, int ifnum, int protocol)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_PROTOCOL, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		protocol, ifnum, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * set idle
 */
int usb_set_idle(struct usb_device *dev, int ifnum, int duration, int report_id)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_IDLE, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		(duration << 8) | report_id, ifnum, NULL, 0, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get report
 */
int usb_get_report(struct usb_device *dev, int ifnum, unsigned char type,
		   unsigned char id, void *buf, int size)
{
	return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_REPORT,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get class descriptor
 */
int usb_get_class_descriptor(struct usb_device *dev, int ifnum,
		unsigned char type, unsigned char id, void *buf, int size)
{
	return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_RECIP_INTERFACE | USB_DIR_IN,
		(type << 8) + id, ifnum, buf, size, USB_CNTL_TIMEOUT);
}

/********************************************************************
 * get string index in buffer
 */
int usb_get_string(struct usb_device *dev, unsigned short langid,
		   unsigned char index, void *buf, int size)
{
	//int i;
	int result;

	//for (i = 0; i < 3; ++i) 
	{
		/* some devices are flaky */
		result = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
			(USB_DT_STRING << 8) + index, langid, buf, size,
			USB_CNTL_TIMEOUT);

	//	if (result > 0)
	//		break;
	}

	return result;
}


static void usb_try_string_workarounds(unsigned char *buf, int *length)
{
	int newlength, oldlength = *length;

	for (newlength = 2; newlength + 1 < oldlength; newlength += 2)
		if (!isprint(buf[newlength]) || buf[newlength + 1])
			break;

	if (newlength > 2) {
		buf[0] = newlength;
		*length = newlength;
	}
}


static int usb_string_sub(struct usb_device *dev, unsigned int langid,
		unsigned int index, unsigned char *buf)
{
	int rc;

	/* Try to read the string descriptor by asking for the maximum
	 * possible number of bytes */
	rc = usb_get_string(dev, langid, index, buf, 255);

	/* If that failed try to read the descriptor length, then
	 * ask for just that many bytes */
	if (rc < 2) {
		rc = usb_get_string(dev, langid, index, buf, 2);
		if (rc == 2)
			rc = usb_get_string(dev, langid, index, buf, buf[0]);
	}

	if (rc >= 2) {
		if (!buf[0] && !buf[1])
			usb_try_string_workarounds(buf, &rc);

		/* There might be extra junk at the end of the descriptor */
		if (buf[0] < rc)
			rc = buf[0];

		rc = rc - (rc & 1); /* force a multiple of two */
	}

	if (rc < 2)
		rc = -1;

	return rc;
}


/********************************************************************
 * usb_string:
 * Get string index and translate it to ascii.
 * returns string length (> 0) or error (< 0)
 */
int usb_string(struct usb_device *dev, int index, char *buf, size_t size)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, mybuf, USB_BUFSIZ);
	unsigned char *tbuf;
	int err;
	unsigned int u, idx;

	if (size <= 0 || !buf || !index)
		return -1;
	buf[0] = 0;
	tbuf = &mybuf[0];

	/* get langid for strings if it's not yet known */
	if (!dev->have_langid) {
		err = usb_string_sub(dev, 0, 0, tbuf);
		if (err < 0) {
			debug("error getting string descriptor 0 " \
			      "(error=%lx)\n", dev->status);
			return -1;
		} else if (tbuf[0] < 4) {
			debug("string descriptor 0 too short\n");
			return -1;
		} else {
			dev->have_langid = -1;
			dev->string_langid = tbuf[2] | (tbuf[3] << 8);
				/* always use the first langid listed */
			debug("USB device number %d default " \
			      "language ID 0x%x\n",
			      dev->devnum, dev->string_langid);
		}
	}

	err = usb_string_sub(dev, dev->string_langid, index, tbuf);
	if (err < 0)
		return err;

	size--;		/* leave room for trailing NULL char in output buffer */
	for (idx = 0, u = 2; u < err; u += 2) {
		if (idx >= size)
			break;
		if (tbuf[u+1])			/* high byte */
			buf[idx++] = '?';  /* non-ASCII character */
		else
			buf[idx++] = tbuf[u];
	}
	buf[idx] = 0;
	err = idx;
	return err;
}


/********************************************************************
 * USB device handling:
 * the USB device are static allocated [USB_MAX_DEVICE].
 */


/* returns a pointer to the device with the index [index].
 * if the device is not assigned (dev->devnum==-1) returns NULL
 */
struct usb_device *usb_get_dev_index(int index)
{
	if (usb_dev[index].devnum == -1)
		return NULL;
	else
		return &usb_dev[index];
}

/* returns a pointer of a new device structure or NULL, if
 * no device struct is available
 */
struct usb_device *usb_alloc_new_device()
{
	int i;
	debug("New Device %d\n", dev_index);
	if (dev_index == USB_MAX_DEVICE) {
		printf("ERROR, too many USB Devices, max=%d\n", USB_MAX_DEVICE);
		return NULL;
	}
	/* default Address is 0, real addresses start with 1 */
	usb_dev[dev_index].devnum = dev_index + 1;
	usb_dev[dev_index].maxchild = 0;
	for (i = 0; i < USB_MAXCHILDREN; i++)
		usb_dev[dev_index].children[i] = NULL;
	usb_dev[dev_index].parent = NULL;
	//usb_dev[dev_index].controller = controller;
	dev_index++;
	return &usb_dev[dev_index - 1];
}

/*
 * Free the newly created device node.
 * Called in error cases where configuring a newly attached
 * device fails for some reason.
 */
void usb_free_device(void)
{
	dev_index--;
	debug("Freeing device node: %d\n", dev_index);
	memset(&usb_dev[dev_index], 0, sizeof(struct usb_device));
	usb_dev[dev_index].devnum = -1;
}

/*
 * XHCI issues Enable Slot command and thereafter
 * allocates device contexts. Provide a weak alias
 * function for the purpose, so that XHCI overrides it
 * and EHCI/OHCI just work out of the box.
 */
__weak int usb_alloc_device(struct usb_device *udev)
{
	return 0;
}
/*
 * By the time we get here, the device has gotten a new device ID
 * and is in the default state. We need to identify the thing and
 * get the ball rolling..
 *
 * Returns 0 for success, != 0 for error.
 */
int usb_new_device(struct usb_device *dev)
{
	int addr, err;
	int tmp;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, tmpbuf, USB_BUFSIZ);

	/*
	 * Allocate usb 3.0 device context.
	 * USB 3.0 (xHCI) protocol tries to allocate device slot
	 * and related data structures first. This call does that.
	 * Refer to sec 4.3.2 in xHCI spec rev1.0
	 */
	if (usb_alloc_device(dev)) {
		printf("Cannot allocate device context to get SLOT_ID\n");
		return -1;
	}

	/* We still haven't set the Address yet */
	addr = dev->devnum;
	dev->devnum = 0;

//#ifdef CONFIG_LEGACY_USB_INIT_SEQ
#if 0 //MSTAR
	/* this is the old and known way of initializing devices, it is
	 * different than what Windows and Linux are doing. Windows and Linux
	 * both retrieve 64 bytes while reading the device descriptor
	 * Several USB stick devices report ERR: CTL_TIMEOUT, caused by an
	 * invalid header while reading 8 bytes as device descriptor. */
	dev->descriptor.bMaxPacketSize0 = 8;	    /* Start off at 8 bytes  */
	dev->maxpacketsize = PACKET_SIZE_8;
	dev->epmaxpacketin[0] = 8;
	dev->epmaxpacketout[0] = 8;

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, tmpbuf, 8);
	if (err < 8) {
		printf("\n      USB device not responding, " \
		       "giving up (status=%lX)\n", dev->status);
		return 1;
	}
	memcpy(&dev->descriptor, tmpbuf, 8);
#else
	/* This is a Windows scheme of initialization sequence, with double
	 * reset of the device (Linux uses the same sequence)
	 * Some equipment is said to work only with such init sequence; this
	 * patch is based on the work by Alan Stern:
	 * http://sourceforge.net/mailarchive/forum.php?
	 * thread_id=5729457&forum_id=5398
	 */
	__maybe_unused struct usb_device_descriptor *desc;
	struct usb_device *parent = dev->parent;
	unsigned short portstatus;

	/* send 64-byte GET-DEVICE-DESCRIPTOR request.  Since the descriptor is
	 * only 18 bytes long, this will terminate with a short packet.  But if
	 * the maxpacket size is 8 or 16 the device may be waiting to transmit
	 * some more, or keeps on retransmitting the 8 byte header. */

	desc = (struct usb_device_descriptor *)tmpbuf;
	dev->descriptor.bMaxPacketSize0 = 64;	    /* Start off at 64 bytes  */
	/* Default to 64 byte max packet size */
	dev->maxpacketsize = PACKET_SIZE_64;
	dev->epmaxpacketin[0] = 64;
	dev->epmaxpacketout[0] = 64;

	/*
	 * XHCI needs to issue a Address device command to setup
	 * proper device context structures, before it can interact
	 * with the device. So a get_descriptor will fail before any
	 * of that is done for XHCI unlike EHCI.
	 */
#ifndef CONFIG_USB_XHCI
	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0, desc, 64);
	if (err < 0) {
		debug("usb_new_device: usb_get_descriptor() failed\n");
		return 1;
	}

	dev->descriptor.bMaxPacketSize0 = desc->bMaxPacketSize0;
	/*
	 * Fetch the device class, driver can use this info
	 * to differentiate between HUB and DEVICE.
	 */
	dev->descriptor.bDeviceClass = desc->bDeviceClass;
#endif

	if (parent) {
		/* reset the port for the second time */
		err = hub_port_reset(dev->parent, dev->portnr - 1, &portstatus);
		if (err < 0) {
			printf("\n     Couldn't reset port %i\n", dev->portnr);
			return 1;
		}
	}
#endif

	dev->epmaxpacketin[0] = dev->descriptor.bMaxPacketSize0;
	dev->epmaxpacketout[0] = dev->descriptor.bMaxPacketSize0;
	switch (dev->descriptor.bMaxPacketSize0) {
	case 8:
		dev->maxpacketsize  = PACKET_SIZE_8;
		break;
	case 16:
		dev->maxpacketsize = PACKET_SIZE_16;
		break;
	case 32:
		dev->maxpacketsize = PACKET_SIZE_32;
		break;
	case 64:
		dev->maxpacketsize = PACKET_SIZE_64;
		break;
		default: printf("Unreasonable MaxPacketSize %d \n",dev->descriptor.bMaxPacketSize0); break;
	}

	err = usb_set_address(dev, addr); /* set address */	
	dev->devnum = addr;

	if (err < 0) {
		printf("\n      USB device not accepting new address " \
			"(error=%lX)\n", dev->status);
		return 1;
	}

	wait_ms(10);	/* Let the SET_ADDRESS settle */

	tmp = sizeof(dev->descriptor);

	err = usb_get_descriptor(dev, USB_DT_DEVICE, 0,
				 tmpbuf, sizeof(dev->descriptor));
	if (err < tmp) {
		if (err < 0)
			printf("unable to get device descriptor (error=%d)\n",
			       err);
		else
			printf("USB device descriptor short read " \
				"(expected %i, got %i)\n", tmp, err);
		return 1;
	}
	memcpy(&dev->descriptor, tmpbuf, sizeof(dev->descriptor));
	/* correct le values */
	le16_to_cpus(&dev->descriptor.bcdUSB);
	le16_to_cpus(&dev->descriptor.idVendor);
	le16_to_cpus(&dev->descriptor.idProduct);
	le16_to_cpus(&dev->descriptor.bcdDevice);
	/* only support for one config for now */
	err = usb_get_configuration_no(dev, tmpbuf, 0);
	if (err < 0) {
		printf("usb_new_device: Cannot read configuration, " \
		       "skipping device %04x:%04x\n",
		       dev->descriptor.idVendor, dev->descriptor.idProduct);
		return -1;
	}
	usb_parse_config(dev, tmpbuf, 0);
#if (MP_USB_MSTAR==1)
    /* do not accept device if ext hub target is set: only suport interface 0*/
    printf("\n[USB] interface[0] conf:%x value %x: \n", dev->config.desc.bConfigurationValue, dev->config.if_desc[0].desc.bInterfaceClass);
    if(dev->parent == NULL && target_ext_hub_port > 0
        && dev->config.if_desc[0].desc.bInterfaceClass == 8)
    {
        printf("[USB] Don't plug Flash Disk into TV directly!!\n");
        return -1;
    }	

	if (dev->config.if_desc[0].desc.bInterfaceClass == 3) //Don't scan for HID class device
	{
	        printf("[USB] Stop to parse HID calss devices!!\n");
	        return -1;
	}
#endif /* MP_USB_MSTAR==1 */

	usb_set_maxpacket(dev);
	/* we set the default configuration here */
	if (usb_set_configuration(dev, dev->config.desc.bConfigurationValue)) {
		printf("failed to set default configuration " \
			"len %d, status %lX\n", dev->act_len, dev->status);
		return -1;
	}
	debug("new device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
	      dev->descriptor.iManufacturer, dev->descriptor.iProduct,
	      dev->descriptor.iSerialNumber);
	memset(dev->mf, 0, sizeof(dev->mf));
	memset(dev->prod, 0, sizeof(dev->prod));
	memset(dev->serial, 0, sizeof(dev->serial));
	wait_ms(200);
	if (dev->descriptor.iManufacturer)
		usb_string(dev, dev->descriptor.iManufacturer,
			   dev->mf, sizeof(dev->mf));
	if (dev->descriptor.iProduct)
		usb_string(dev, dev->descriptor.iProduct,
			   dev->prod, sizeof(dev->prod));
	if (dev->descriptor.iSerialNumber)
		usb_string(dev, dev->descriptor.iSerialNumber,
			   dev->serial, sizeof(dev->serial));
	debug("Manufacturer %s\n", dev->mf);
	debug("Product      %s\n", dev->prod);
	debug("SerialNumber %s\n", dev->serial);
    /* do not scan device on ext hub */
    if(dev->parent == NULL && target_ext_hub_port == 0)
    {
        printf("[USB] Please plug Flash Disk into TV directly!!\n");
        return 0;
    }
	
#if (MP_USB_MSTAR==1)
	//printf("[USB] idVendor = %x, idProduct = %x\n", dev->descriptor.idVendor, dev->descriptor.idProduct);
	if (((dev->descriptor.idVendor == 0x0411) && (dev->descriptor.idProduct==0x01e7))
			|| ((dev->descriptor.idVendor == 0x0411) && (dev->descriptor.idProduct==0x01de)))
	{
		//printf("[USB] Buffalo U3 disk found!\n");
		Usb_host_SetEverInited(2);
	}

#ifdef ENABLE_USB_LAN_MODULE
	if (dev->config.if_desc[0].desc.bInterfaceClass == USB_CLASS_HUB)
	{
		/* Do nothing. Let usb_hub_probe handle it */
	}
	else if (dev->config.if_desc[0].desc.bInterfaceClass == USB_CLASS_MASS_STORAGE)
	{
		/* Do nothing. Let usb_stor_scan handle it */
	}
	else
	{
		USB_Bulk_InitEx(dev, UsbPortSelect);
		usb_lan_initialize(dev);
	}
#endif
#endif /* MP_USB_MSTAR==1 */

	/* now prode if the device is a hub */
	usb_hub_probe(dev, 0);
	return 0;
}

extern int usb_stor_scan(int mode);
#ifdef ENABLE_USB_LAN_MODULE
extern int
usb_lan_initialize(struct usb_device *udev);
#endif
extern int ever_inited;
#if defined(ENABLE_SECOND_EHC)
extern int ever_inited2;
#endif
#if defined(ENABLE_THIRD_EHC)
extern int ever_inited3;
#endif
#if defined(ENABLE_FOURTH_EHC)
extern int ever_inited4;
#endif

int usb_scan_devices(void)
{
	int i;
	int err=0;
	struct usb_device *dev;

	/* first make all devices unknown */
	for (i = 0; i < USB_MAX_DEVICE; i++) {
		memset(&usb_dev[i], 0, sizeof(struct usb_device));
		usb_dev[i].devnum = -1;
	}
	dev_index = 0;
	/* device 0 is always present (root hub, so let it analyze) */
	dev = usb_alloc_new_device();
	if(dev == NULL)
	{
		printf("%s: usb_alloc_new_device() return NULL, at %d\n", __func__, __LINE__);
		return -1;
	}
#if 0//defined(CONFIG_USB_XHCI) && defined(ENABLE_XHC)
	dev->level = 1;
	if (UsbPortSelect == 4)
	{
            err = USB3_enumerate(dev);
            if (err)
                return -1;
	}
	else
#endif
	{
		err=usb_new_device(dev);
		if(err)
			return -1;
	}
	printf("%d USB Device(s) found\n", dev_index);

    return 0;
}

/****************************************************************************
 * HUB "Driver"
 * Probes device for being a hub and configurate it
 */

#undef  USB_HUB_DEBUG

#ifdef	USB_HUB_DEBUG
#define	USB_HUB_PRINTF(fmt, args...)	printf(fmt , ##args)
#else
#define USB_HUB_PRINTF(fmt, args...)
#endif


static struct usb_hub_device hub_dev[USB_MAX_HUB];
static int usb_hub_index;


int usb_get_hub_descriptor(struct usb_device *dev, void *data, int size)
{
	unsigned wValue;

	if (dev->speed == USB_SPEED_SUPER)
		wValue = USB_DT_SS_HUB;
	else
		wValue = USB_DT_HUB;

	return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
		USB_REQ_GET_DESCRIPTOR, USB_DIR_IN | USB_RT_HUB,
		wValue << 8, 0, data, size, USB_CNTL_TIMEOUT);
}

int usb_clear_hub_feature(struct usb_device *dev, int feature)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_HUB, feature,
				0, NULL, 0, USB_CNTL_TIMEOUT);
}

int usb_clear_port_feature(struct usb_device *dev, int port, int feature)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature,
				port, NULL, 0, USB_CNTL_TIMEOUT);
}

int usb_set_port_feature(struct usb_device *dev, int port, int feature)
{
	return usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				USB_REQ_SET_FEATURE, USB_RT_PORT, feature,
				port, NULL, 0, USB_CNTL_TIMEOUT);
}

int usb_get_hub_status(struct usb_device *dev, void *data)
{
	return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_HUB, 0, 0,
			data, sizeof(struct usb_hub_status), USB_CNTL_TIMEOUT);
}

int usb_get_port_status(struct usb_device *dev, int port, void *data)
{
	return usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port,
			data, sizeof(struct usb_hub_status), USB_CNTL_TIMEOUT);
}


static void usb_hub_power_on(struct usb_hub_device *hub)
{
	int i;
	struct usb_device *dev;

	dev = hub->pusb_dev;
	/* Enable power to the ports */
	USB_HUB_PRINTF("enabling power on all ports\n");
	for (i = 0; i < dev->maxchild; i++) {
		usb_set_port_feature(dev, i + 1, USB_PORT_FEAT_POWER);
		USB_HUB_PRINTF("port %d returns %lX\n", i + 1, dev->status);		
	}
	wait_ms(hub->desc.bPwrOn2PwrGood * 2);
}

void usb_hub_reset(void)
{
	usb_hub_index = 0;
}

struct usb_hub_device *usb_hub_allocate(void)
{
	if (usb_hub_index < USB_MAX_HUB)
		return &hub_dev[usb_hub_index++];

	printf("ERROR: USB_MAX_HUB (%d) reached\n", USB_MAX_HUB);
	return NULL;
}

#define MAX_TRIES 5

static inline char *portspeed(int portstatus)
{
	if (portstatus & (1 << USB_PORT_FEAT_HIGHSPEED))
		return "480 Mb/s";
	else if (portstatus & (1 << USB_PORT_FEAT_LOWSPEED))
		return "1.5 Mb/s";
	else
		return "12 Mb/s";
}

int hub_port_reset(struct usb_device *dev, int port,
			unsigned short *portstat)
{
	int tries;
	struct usb_port_status portsts;
	unsigned short portstatus, portchange;

	USB_HUB_PRINTF("hub_port_reset: resetting port %d...\n", port);
	for (tries = 0; tries < MAX_TRIES; tries++) {

		usb_set_port_feature(dev, port + 1, USB_PORT_FEAT_RESET);
		wait_ms(200);

		if (usb_get_port_status(dev, port + 1, &portsts) < 0) {
			USB_HUB_PRINTF("get_port_status failed status %lX\n",
					dev->status);
			return -1;
		}
		portstatus = le16_to_cpu(portsts.wPortStatus);
		portchange = le16_to_cpu(portsts.wPortChange);

		USB_HUB_PRINTF("portstatus %x, change %x, %s\n",
				portstatus, portchange,
				portspeed(portstatus));

		USB_HUB_PRINTF("STAT_C_CONNECTION = %d STAT_CONNECTION = %d" \
			       "  USB_PORT_STAT_ENABLE %d\n",
			(portchange & USB_PORT_STAT_C_CONNECTION) ? 1 : 0,
			(portstatus & USB_PORT_STAT_CONNECTION) ? 1 : 0,
			(portstatus & USB_PORT_STAT_ENABLE) ? 1 : 0);

		if ((portchange & USB_PORT_STAT_C_CONNECTION) ||
		    !(portstatus & USB_PORT_STAT_CONNECTION))
			return -1;

		if (portstatus & USB_PORT_STAT_ENABLE)
			break;

		wait_ms(200);
	}

	if (tries == MAX_TRIES) {
		USB_HUB_PRINTF("Cannot enable port %i after %i retries, " \
				"disabling port.\n", port + 1, MAX_TRIES);
		USB_HUB_PRINTF("Maybe the USB cable is bad?\n");
		return -1;
	}

	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_RESET);
	*portstat = portstatus;
	return 0;
}


void usb_hub_port_connect_change(struct usb_device *dev, int port)
{
	struct usb_device *usb;
	struct usb_port_status portsts;
	unsigned short portstatus=0;
	#ifdef  USB_HUB_DEBUG
	unsigned short portchange=0;
	#endif
	/* Check status */
	if (usb_get_port_status(dev, port + 1, &portsts) < 0) {
		USB_HUB_PRINTF("get_port_status failed\n");
		return;
	}

	portstatus = le16_to_cpu(portsts.wPortStatus);
	#ifdef  USB_HUB_DEBUG
	portchange = le16_to_cpu(portsts.wPortChange);
	USB_HUB_PRINTF("portstatus %x, change %x, %s\n",
			portstatus, portchange, portspeed(portstatus));
	#endif

	/* Clear the connection change status */
	usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_C_CONNECTION);

	/* Disconnect any existing devices under this port */
	if (((!(portstatus & USB_PORT_STAT_CONNECTION)) &&
	     (!(portstatus & USB_PORT_STAT_ENABLE))) || (dev->children[port])) {
		USB_HUB_PRINTF("usb_disconnect(&hub->children[port]);\n");
		/* Return now if nothing is connected */
		if (!(portstatus & USB_PORT_STAT_CONNECTION))
			return;
	}
	wait_ms(200);

	/* Reset the port */
	if (hub_port_reset(dev, port, &portstatus) < 0) {
		printf("cannot reset port %i!?\n", port + 1);
		return;
	}

	wait_ms(200);

	/* Allocate a new device struct for it */
	usb = usb_alloc_new_device();
	if(usb == NULL)
	{
		printf("%s: usb_alloc_new_device() return NULL, at %d\n", __func__, __LINE__);
		return;
	}

	if (dev->descriptor.bDeviceProtocol == 3)
		usb->speed = USB_SPEED_SUPER;
	else if (portstatus & USB_PORT_STAT_HIGH_SPEED)
		usb->speed = USB_SPEED_HIGH;
	else if (portstatus & USB_PORT_STAT_LOW_SPEED)
		usb->speed = USB_SPEED_LOW;
	else
		usb->speed = USB_SPEED_FULL;

	dev->children[port] = usb;
	usb->parent = dev;
#if 1 //MBOOT_XHCI
	usb->level = dev->level + 1;
#endif
	/* Run it through the hoops (find a driver, etc) */
#if defined(CONFIG_USB_XHCI) && defined(ENABLE_XHC)//MBOOT_XHCI
	if (UsbPortSelect == 4)
	{
		int portnum;

		if (!usb->parent)
			portnum = gXHCI_rh_port_sel;
		else
		{
			portnum = port + 1;
			usb->portnum = dev->portnum;
			printf("usb->portnum: %d\n", usb->portnum);
		}

		if (ms_xhci_dev_enum(&ms_xhci, usb, portnum) < 0 )
			return;
	}
	else
#endif
	{
		if (usb_new_device(usb)) {
			/* Woops, disable the port */
			USB_HUB_PRINTF("hub: disabling port %d\n", port + 1);
			usb_clear_port_feature(dev, port + 1, USB_PORT_FEAT_ENABLE);
		}
	}
}


u32 g_u3_chk_conn_time = 5;  //default 5 seconds
void usb_set_u3_chk_conn_time(u32 uTimesec)
{
	printf("set usb 30 check connection time to %d sec\n", uTimesec);
	g_u3_chk_conn_time = uTimesec;
}

int usb_hub_configure(struct usb_device *dev)
{
	unsigned char buffer[USB_BUFSIZ], *bitmap;
	struct usb_hub_descriptor *descriptor;
	#ifdef  USB_HUB_DEBUG
	struct usb_hub_status *hubsts=NULL;
	#endif
	int i, j, chk_conn_tries;
	u8 AnyPortConn;
	struct usb_hub_device *hub;

	buffer[0]=0; //for DZ check.

	/* "allocate" Hub device */
	hub = usb_hub_allocate();
	if (hub == NULL)
		return -1;
	hub->pusb_dev = dev;
	/* Get the the hub descriptor */
	if (usb_get_hub_descriptor(dev, buffer, 4) < 0) {
		USB_HUB_PRINTF("usb_hub_configure: failed to get hub " \
				   "descriptor, giving up %lX\n", dev->status);
		return -1;
	}
	descriptor = (struct usb_hub_descriptor *)buffer;

	/* silence compiler warning if USB_BUFSIZ is > 256 [= sizeof(char)] */
	i = descriptor->bLength;
	if (i > USB_BUFSIZ) {
		USB_HUB_PRINTF("usb_hub_configure: failed to get hub " \
				"descriptor - too long: %d\n",
				descriptor->bLength);
		return -1;
	}

	if (usb_get_hub_descriptor(dev, buffer, descriptor->bLength) < 0) {
		USB_HUB_PRINTF("usb_hub_configure: failed to get hub " \
				"descriptor 2nd giving up %lX\n", dev->status);
		return -1;
	}
	memcpy((unsigned char *)&hub->desc, buffer, descriptor->bLength);
	/* adjust 16bit values */
	hub->desc.wHubCharacteristics =
				le16_to_cpu(descriptor->wHubCharacteristics);
	/* set the bitmap */
	bitmap = (unsigned char *)&hub->desc.DeviceRemovable[0];
	/* devices not removable by default */
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8);
	bitmap = (unsigned char *)&hub->desc.PortPowerCtrlMask[0];
	memset(bitmap, 0xff, (USB_MAXCHILDREN+1+7)/8); /* PowerMask = 1B */

	for (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.DeviceRemovable[i] = descriptor->DeviceRemovable[i];

	for (i = 0; i < ((hub->desc.bNbrPorts + 1 + 7)/8); i++)
		hub->desc.PortPowerCtrlMask[i] = descriptor->PortPowerCtrlMask[i];

	dev->maxchild = descriptor->bNbrPorts;
	USB_HUB_PRINTF("%d ports detected\n", dev->maxchild);

	switch (hub->desc.wHubCharacteristics & HUB_CHAR_LPSM) {
	case 0x00:
		USB_HUB_PRINTF("ganged power switching\n");
		break;
	case 0x01:
		USB_HUB_PRINTF("individual port power switching\n");
		break;
	case 0x02:
	case 0x03:
		USB_HUB_PRINTF("unknown reserved power switching mode\n");
		break;
        default: break;
	}

	if (hub->desc.wHubCharacteristics & HUB_CHAR_COMPOUND)
		USB_HUB_PRINTF("part of a compound device\n");
	else
		USB_HUB_PRINTF("standalone hub\n");

	switch (hub->desc.wHubCharacteristics & HUB_CHAR_OCPM) {
	case 0x00:
		USB_HUB_PRINTF("global over-current protection\n");
		break;
	case 0x08:
		USB_HUB_PRINTF("individual port over-current protection\n");
		break;
	case 0x10:
	case 0x18:
		USB_HUB_PRINTF("no over-current protection\n");
		break;
        default: break;
	}

	USB_HUB_PRINTF("power on to power good time: %dms\n",
			descriptor->bPwrOn2PwrGood * 2);
	USB_HUB_PRINTF("hub controller current requirement: %dmA\n",
			descriptor->bHubContrCurrent);

	for (i = 0; i < dev->maxchild; i++)
		USB_HUB_PRINTF("port %d is%s removable\n", i + 1,
			hub->desc.DeviceRemovable[(i + 1) / 8] & \
					   (1 << ((i + 1) % 8)) ? " not" : "");

	if (sizeof(struct usb_hub_status) > USB_BUFSIZ) {
		USB_HUB_PRINTF("usb_hub_configure: failed to get Status - " \
				"too long: %d\n", descriptor->bLength);
		return -1;
	}

	if (usb_get_hub_status(dev, buffer) < 0) {
		USB_HUB_PRINTF("usb_hub_configure: failed to get Status %lX\n",
				dev->status);
		return -1;
	}

#if defined(CONFIG_USB_XHCI) && defined(ENABLE_XHC)//MBOOT_XHCI
	if (dev->descriptor.bDeviceProtocol == 3) //for SS hub
	{
		printf("set hub depth %d\n", dev->level - 1);
		if (usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
				12 /*hub_set_depth*/, USB_RT_HUB,
				dev->level - 1, 0, NULL, 0,
				5000) < 0)
		{
			USB_HUB_PRINTF("Set hub depth error\n");
			return -1;
		}
	}
#endif
	#ifdef  USB_HUB_DEBUG
	hubsts = (struct usb_hub_status *)buffer;
	USB_HUB_PRINTF("get_hub_status returned status %X, change %X\n",
			le16_to_cpu(hubsts->wHubStatus),
			le16_to_cpu(hubsts->wHubChange));
	USB_HUB_PRINTF("local power source is %s\n",
		(le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_LOCAL_POWER) ? \
		"lost (inactive)" : "good");
	USB_HUB_PRINTF("%sover-current condition exists\n",
		(le16_to_cpu(hubsts->wHubStatus) & HUB_STATUS_OVERCURRENT) ? \
		"" : "no ");
	#endif
	usb_hub_power_on(hub);

	printf("Wait for hub reset....\n");

    if(dev->parent == NULL && target_ext_hub_port > 0)
    {
        printf("\n=======[ Filter Port for Exthub on Roothub ]=======\n");
    }

	if (dev->speed == USB_SPEED_SUPER)
    {   
		chk_conn_tries = g_u3_chk_conn_time;
    }
	else
    {   
		chk_conn_tries = 1;
    }

	for (j=0; j<=chk_conn_tries; j++)
	{
		AnyPortConn = 0;
		printf("checking hub ports...\n");
		for (i = 0; i < dev->maxchild; i++) {
			struct usb_port_status portsts;
			unsigned short portstatus, portchange;

			/* for exthub on roothub*/
			if(dev->parent == NULL)
			{
				if (target_ext_hub_port > 0)    /* for port 1~15*/
				{
					printf("Hello Port %d", i+1);
					if(i+1 != target_ext_hub_port)
					{
						printf(" Skip\n");
						continue;
					}
					else
						printf(" No Skip\n");
				}
			}

			if (usb_get_port_status(dev, i + 1, &portsts) < 0) {
				USB_HUB_PRINTF("get_port_status failed\n");
				continue;
			}

			portstatus = le16_to_cpu(portsts.wPortStatus);
			portchange = le16_to_cpu(portsts.wPortChange);
			USB_HUB_PRINTF("Port %d Status %X Change %X\n",
					i + 1, portstatus, portchange);

			if (portchange & USB_PORT_STAT_C_CONNECTION) {
				USB_HUB_PRINTF("port %d connection change\n", i + 1);
				usb_hub_port_connect_change(dev, i);
				AnyPortConn = 1;
			}
			if (portchange & USB_PORT_STAT_C_ENABLE) {
				USB_HUB_PRINTF("port %d enable change, status %x\n",
						i + 1, portstatus);
				usb_clear_port_feature(dev, i + 1,
							USB_PORT_FEAT_C_ENABLE);

				/* EM interference sometimes causes bad shielded USB
				 * devices to be shutdown by the hub, this hack enables
				 * them again. Works at least with mouse driver */
				if (!(portstatus & USB_PORT_STAT_ENABLE) &&
				     (portstatus & USB_PORT_STAT_CONNECTION) &&
				     ((dev->children[i]))) {
					USB_HUB_PRINTF("already running port %i "  \
							"disabled by hub (EMI?), " \
							"re-enabling...\n", i + 1);
						usb_hub_port_connect_change(dev, i);
						AnyPortConn = 1;
				}
			}
			if (portstatus & USB_PORT_STAT_SUSPEND) {
				USB_HUB_PRINTF("port %d suspend change\n", i + 1);
				usb_clear_port_feature(dev, i + 1,
							USB_PORT_FEAT_SUSPEND);
			}

			if (portchange & USB_PORT_STAT_C_OVERCURRENT) {
				USB_HUB_PRINTF("port %d over-current change\n", i + 1);
				usb_clear_port_feature(dev, i + 1,
							USB_PORT_FEAT_C_OVER_CURRENT);
				usb_hub_power_on(hub);
			}

			if (portchange & USB_PORT_STAT_C_RESET) {
				USB_HUB_PRINTF("port %d reset change\n", i + 1);
				usb_clear_port_feature(dev, i + 1,
							USB_PORT_FEAT_C_RESET);
			}
		} /* end for i all ports */

		if (j < chk_conn_tries)
		{
			if (AnyPortConn)
				break;
			else if (dev->speed == USB_SPEED_SUPER)
				mdelay(1000);
		}
	}
	return 0;
}

int usb_hub_probe(struct usb_device *dev, int ifnum)
{
	struct usb_interface *iface;
	struct usb_endpoint_descriptor *ep;
	int ret;

	iface = &dev->config.if_desc[ifnum];
	/* Is it a hub? */
	if (iface->desc.bInterfaceClass != USB_CLASS_HUB)
		return 0;
	/* Some hubs have a subclass of 1, which AFAICT according to the */
	/*  specs is not defined, but it works */
	if ((iface->desc.bInterfaceSubClass != 0) &&
	    (iface->desc.bInterfaceSubClass != 1))
		return 0;
	/* Multiple endpoints? What kind of mutant ninja-hub is this? */
	if (iface->desc.bNumEndpoints != 1)
		return 0;
	ep = &iface->ep_desc[0];
	/* Output endpoint? Curiousier and curiousier.. */
	if (!(ep->bEndpointAddress & USB_DIR_IN))
		return 0;
	/* If it's not an interrupt endpoint, we'd better punt! */
	if ((ep->bmAttributes & 3) != 3)
		return 0;
	/* We found a hub */
	USB_HUB_PRINTF("USB hub found\n");
	ret = usb_hub_configure(dev);
	return ret;
}

__weak
int board_usb_init(int index, enum usb_init_type init)
{
	return 0;
}
/* EOF */
