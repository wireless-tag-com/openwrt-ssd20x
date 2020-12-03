/*
 * bi_direction.c
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/usb/composite.h>

#include "u_bi_direction.h"
/*-------------------------------------------------------------------------*/
USB_GADGET_COMPOSITE_OPTIONS();

#define DRIVER_VERSION		"Sep 9 2018"

static const char longname[] = "MDD Bi";

static struct usb_bi_direction_options gbi_direction_options = {
	.bulk_in_buflen = GBI_DIRECTION_BULK_IN_BUFLEN,
	.bulk_out_buflen = GBI_DIRECTION_BULK_OUT_BUFLEN,
	.bulk_in_qlen = GBI_DIRECTION_BULK_IN_QLEN,
	.bulk_out_qlen = GBI_DIRECTION_BULK_OUT_QLEN,
};

#ifndef	CONFIG_USB_BI_DIRECTION_HNPTEST
#define DRIVER_VENDOR_NUM	0x0525
#define DRIVER_PRODUCT_NUM	0xa4a0
#define DEFAULT_AUTORESUME	0
#else
#define DRIVER_VENDOR_NUM	0x1a0a		/* OTG test device IDs */
#define DRIVER_PRODUCT_NUM	0xbadd
#define DEFAULT_AUTORESUME	5
#endif

/* If the optional "autoresume" mode is enabled, it provides good
 * functional coverage for the "USBCV" test harness from USB-IF.
 * It's always set if OTG mode is enabled.
 */
static unsigned autoresume = DEFAULT_AUTORESUME;
module_param(autoresume, uint, S_IRUGO);
MODULE_PARM_DESC(autoresume, "bi_direction, or seconds before remote wakeup");

/* Maximum Autoresume time */
static unsigned max_autoresume;
module_param(max_autoresume, uint, S_IRUGO);
MODULE_PARM_DESC(max_autoresume, "maximum seconds before remote wakeup");

/* Interval between two remote wakeups */
static unsigned autoresume_interval_ms;
module_param(autoresume_interval_ms, uint, S_IRUGO);
MODULE_PARM_DESC(autoresume_interval_ms,
		"milliseconds to increase successive wakeup delays");

static unsigned autoresume_step_ms;
/*-------------------------------------------------------------------------*/

static struct usb_device_descriptor device_desc = {
	.bLength =		sizeof device_desc,
	.bDescriptorType =	USB_DT_DEVICE,

	.bcdUSB =		cpu_to_le16(0x0200),
	.bDeviceClass =		USB_CLASS_PER_INTERFACE,

	.idVendor =		cpu_to_le16(DRIVER_VENDOR_NUM),
	.idProduct =		cpu_to_le16(DRIVER_PRODUCT_NUM),
	.bNumConfigurations =	1,
};

#ifdef CONFIG_USB_OTG
static struct usb_otg_descriptor otg_descriptor = {
	.bLength =		sizeof otg_descriptor,
	.bDescriptorType =	USB_DT_OTG,

	/* REVISIT SRP-only hardware is possible, although
	 * it would not be called "OTG" ...
	 */
	.bmAttributes =		USB_OTG_SRP | USB_OTG_HNP,
};

static const struct usb_descriptor_header *otg_desc[] = {
	(struct usb_descriptor_header *) &otg_descriptor,
	NULL,
};
#else
#define otg_desc	NULL
#endif

/* string IDs are assigned dynamically */
/* default serial number takes at least two packets */
static char serial[] = "03e72160";

#define USB_GBI_DIRECTION_NN_DESC	(USB_GADGET_FIRST_AVAIL_IDX + 0)

static struct usb_string strings_dev[] = {
	[USB_GADGET_MANUFACTURER_IDX].s = "Sigmastar Ltd",
	[USB_GADGET_PRODUCT_IDX].s = longname,
	[USB_GADGET_SERIAL_IDX].s = serial,
	[USB_GBI_DIRECTION_NN_DESC].s	= "Bi Net Dev",
	{  }			/* end of list */
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

/*-------------------------------------------------------------------------*/

static struct timer_list	autoresume_timer;

static void bi_direction_autoresume(unsigned long _c)
{
	struct usb_composite_dev	*cdev = (void *)_c;
	struct usb_gadget		*g = cdev->gadget;

	if (!cdev->config)
		return;

	if (g->speed != USB_SPEED_UNKNOWN) {
		int status = usb_gadget_wakeup(g);
		INFO(cdev, "%s --> %d\n", __func__, status);
	}
}

static void bi_direction_suspend(struct usb_composite_dev *cdev)
{
	if (cdev->gadget->speed == USB_SPEED_UNKNOWN)
		return;

	if (autoresume) {
		if (max_autoresume &&
			(autoresume_step_ms > max_autoresume * 1000))
				autoresume_step_ms = autoresume * 1000;

		mod_timer(&autoresume_timer, jiffies +
			msecs_to_jiffies(autoresume_step_ms));
		DBG(cdev, "suspend, wakeup in %d milliseconds\n",
			autoresume_step_ms);

		autoresume_step_ms += autoresume_interval_ms;
	} else
		DBG(cdev, "%s\n", __func__);
}

static void bi_direction_resume(struct usb_composite_dev *cdev)
{
	DBG(cdev, "%s\n", __func__);
	del_timer(&autoresume_timer);
}

/*-------------------------------------------------------------------------*/
module_param_named(bulk_in_buflen, gbi_direction_options.bulk_in_buflen, uint, 0);
module_param_named(bulk_out_buflen, gbi_direction_options.bulk_out_buflen, uint, 0);

static struct usb_function *func_bd;
static struct usb_function_instance *func_inst_bd;

module_param_named(bulk_in_qlen, gbi_direction_options.bulk_in_qlen, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(bulk_in_qlen, "depth of bi bulk in queue");

module_param_named(bulk_out_qlen, gbi_direction_options.bulk_out_qlen, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(bulk_out_qlen, "depth of bi bulk out queue");

static int bd_config_setup(struct usb_configuration *c,
		const struct usb_ctrlrequest *ctrl)
{
	switch (ctrl->bRequest) {
	case 0x5b:
	case 0x5c:
	    return func_bd->setup(func_bd, ctrl);
	default:
		return -EOPNOTSUPP;
	}
}

static struct usb_configuration bi_driver = {
	.label          = "bi",
	.setup                  = bd_config_setup,//todo
	.bConfigurationValue = 1,
	.bmAttributes   = USB_CONFIG_ATT_SELFPOWER,
	/* .iConfiguration = DYNAMIC */
};

static int __init bi_direction_bind(struct usb_composite_dev *cdev)
{
	struct f_bd_opts	*bd_opts;
	int			status;

	status = usb_string_ids_tab(cdev, strings_dev);
	if (status < 0)
		return status;

	device_desc.iManufacturer = strings_dev[USB_GADGET_MANUFACTURER_IDX].id;
	device_desc.iProduct = strings_dev[USB_GADGET_PRODUCT_IDX].id;
	device_desc.iSerialNumber = strings_dev[USB_GADGET_SERIAL_IDX].id;

	setup_timer(&autoresume_timer, bi_direction_autoresume, (unsigned long) cdev);

	func_inst_bd = usb_get_function_instance("Bi");
	if (IS_ERR(func_inst_bd)) {
		status = PTR_ERR(func_inst_bd);
		return status;
	}

	bd_opts = container_of(func_inst_bd, struct f_bd_opts, func_inst);
	bd_opts->bulk_in_buflen = gbi_direction_options.bulk_in_buflen;
	bd_opts->bulk_in_qlen = gbi_direction_options.bulk_in_qlen;
	bd_opts->bulk_out_buflen = gbi_direction_options.bulk_out_buflen;
	bd_opts->bulk_out_qlen = gbi_direction_options.bulk_out_qlen;

	func_bd = usb_get_function(func_inst_bd);
	if (IS_ERR(func_bd)) {
		status = PTR_ERR(func_bd);
		goto err_put_func_inst_bd;
	}

	bi_driver.iConfiguration = strings_dev[USB_GBI_DIRECTION_NN_DESC].id;

	/* support autoresume for remote wakeup testing */
	bi_driver.bmAttributes &= ~USB_CONFIG_ATT_WAKEUP;
	bi_driver.descriptors = NULL;
	if (autoresume) {
		bi_driver.bmAttributes |= USB_CONFIG_ATT_WAKEUP;
		autoresume_step_ms = autoresume * 1000;
	}

	/* support OTG systems */
	if (gadget_is_otg(cdev->gadget)) {
		bi_driver.descriptors = otg_desc;
		bi_driver.bmAttributes |= USB_CONFIG_ATT_WAKEUP;
	}

	usb_add_config_only(cdev, &bi_driver);

	status = usb_add_function(&bi_driver, func_bd);
	if (status)
		goto err_conf_fbd;

	usb_ep_autoconfig_reset(cdev->gadget);
	usb_composite_overwrite_options(cdev, &coverwrite);

	INFO(cdev, "%s, version: " DRIVER_VERSION "\n", longname);

	return 0;

err_conf_fbd:
	usb_put_function(func_bd);
	func_bd = NULL;
err_put_func_inst_bd:
	usb_put_function_instance(func_inst_bd);
	func_inst_bd = NULL;
	return status;
}

static int bi_direction_unbind(struct usb_composite_dev *cdev)
{
	del_timer_sync(&autoresume_timer);
	if (!IS_ERR_OR_NULL(func_bd))
		usb_put_function(func_bd);
	usb_put_function_instance(func_inst_bd);
	return 0;
}

static __refdata struct usb_composite_driver bi_direction_driver = {
	.name		= "bi_direction",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.max_speed	= USB_SPEED_SUPER,
	.bind		= bi_direction_bind,
	.unbind		= bi_direction_unbind,
	.suspend	= bi_direction_suspend,
	.resume		= bi_direction_resume,
};

module_usb_composite_driver(bi_direction_driver);

MODULE_AUTHOR("Claude Rao");
MODULE_LICENSE("GPL");
