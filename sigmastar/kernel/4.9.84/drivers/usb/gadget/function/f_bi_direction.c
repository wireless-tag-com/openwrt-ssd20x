/*
 * f_bi_direction.c
 *
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/usb/composite.h>

#include "u_bi_direction.h"

/*
 * NEURAL NETWORK FUNCTION ... a testing vehicle for USB peripherals,
 *
 */
/*-------------------------------------------------------------------------*/
unsigned int log_level = LOG_ERR;
module_param(log_level, uint, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(log_level, "Log level For Bi Module");

static struct usb_interface_descriptor bi_intf = {
	.bLength =		sizeof bi_intf,
	.bDescriptorType =	USB_DT_INTERFACE,

	.bNumEndpoints =	2,
	.bInterfaceClass =	USB_CLASS_VENDOR_SPEC,
	/* .iInterface = DYNAMIC */
};

/* full speed support: */

static struct usb_endpoint_descriptor fs_bi_in_ep_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_IN,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor fs_bi_out_ep_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
};

static struct usb_descriptor_header *fs_bi_descs[] = {
	(struct usb_descriptor_header *) &bi_intf,
	(struct usb_descriptor_header *) &fs_bi_out_ep_desc,
	(struct usb_descriptor_header *) &fs_bi_in_ep_desc,
	NULL,
};

/* high speed support: */

static struct usb_endpoint_descriptor hs_bi_in_ep_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_endpoint_descriptor hs_bi_out_ep_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(512),
};

static struct usb_descriptor_header *hs_bi_descs[] = {
	(struct usb_descriptor_header *) &bi_intf,
	(struct usb_descriptor_header *) &hs_bi_in_ep_desc,
	(struct usb_descriptor_header *) &hs_bi_out_ep_desc,
	NULL,
};

/* super speed support: */

static struct usb_endpoint_descriptor ss_bi_in_ep_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor ss_bi_in_ep_comp_desc = {
	.bLength =		USB_DT_SS_EP_COMP_SIZE,
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,
	.bMaxBurst =		0,
	.bmAttributes =		0,
	.wBytesPerInterval =	0,
};

static struct usb_endpoint_descriptor ss_bi_out_ep_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	cpu_to_le16(1024),
};

static struct usb_ss_ep_comp_descriptor ss_bi_out_ep_comp_desc = {
	.bLength =		USB_DT_SS_EP_COMP_SIZE,
	.bDescriptorType =	USB_DT_SS_ENDPOINT_COMP,
	.bMaxBurst =		0,
	.bmAttributes =		0,
	.wBytesPerInterval =	0,
};

static struct usb_descriptor_header *ss_bi_descs[] = {
	(struct usb_descriptor_header *) &bi_intf,
	(struct usb_descriptor_header *) &ss_bi_in_ep_desc,
	(struct usb_descriptor_header *) &ss_bi_in_ep_comp_desc,
	(struct usb_descriptor_header *) &ss_bi_out_ep_desc,
	(struct usb_descriptor_header *) &ss_bi_out_ep_comp_desc,
	NULL,
};

/* function-specific strings: */

static struct usb_string strings_bi[] = {
	[0].s = "bi input and output",
	{  }			/* end of list */
};

static struct usb_gadget_strings stringtab_bi = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_bi,
};

static struct usb_gadget_strings *bi_function_strings[] = {
	&stringtab_bi,
	NULL,
};

/*-------------------------------------------------------------------------*/
static int bi_function_bind(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct bi_device	*bi = func_to_bi(f);
	int			id;
	int ret;

	/* allocate interface ID(s) */
	id = usb_interface_id(c, f);
	if (id < 0)
		return id;
	bi_intf.bInterfaceNumber = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_bi[0].id = id;
	bi_intf.iInterface = id;

	/* allocate endpoints */
	/* control ep */
//todo
	/* Bulk In Endpoint */
	bi->in_ep = usb_ep_autoconfig(cdev->gadget, &fs_bi_in_ep_desc);
	if (!bi->in_ep) {
autoconf_fail:
		ERROR(cdev, "%s: can't autoconfigure on %s\n",
			f->name, cdev->gadget->name);
		return -ENODEV;
	}
	bi->in_ep->driver_data = cdev;	/* claim */

	/* Bulk In Endpoint */
	bi->out_ep = usb_ep_autoconfig(cdev->gadget, &fs_bi_out_ep_desc);
	if (!bi->out_ep)
		goto autoconf_fail;
	bi->out_ep->driver_data = cdev;	/* claim */

	/* support high speed hardware */
	hs_bi_in_ep_desc.bEndpointAddress =
		fs_bi_in_ep_desc.bEndpointAddress;
	hs_bi_out_ep_desc.bEndpointAddress = fs_bi_out_ep_desc.bEndpointAddress;

	/* support super speed hardware */
	ss_bi_in_ep_desc.bEndpointAddress =
		fs_bi_in_ep_desc.bEndpointAddress;
	ss_bi_out_ep_desc.bEndpointAddress = fs_bi_out_ep_desc.bEndpointAddress;

	ret = usb_assign_descriptors(f, fs_bi_descs, hs_bi_descs,
                    ss_bi_descs,NULL);
	if (ret)
		return ret;

	ret = bi_device_register(&cdev->gadget->dev, bi);
	if (ret)
		return ret;

	DBG(cdev, "%s speed %s: IN/%s, OUT/%s\n",
		(gadget_is_superspeed(c->cdev->gadget) ? "super" :
		(gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full")),
			f->name, bi->in_ep->name, bi->out_ep->name);
	return 0;
}

static void bi_function_unbind(struct usb_configuration * c,
                        struct usb_function * f)
{
	struct bi_device	*bi = func_to_bi(f);

	bi_device_unregister(bi);
}

static void bi_function_free(struct usb_function *f)
{
	struct f_bd_opts *opts;

	opts = container_of(f->fi, struct f_bd_opts, func_inst);

	mutex_lock(&opts->lock);
	opts->refcnt--;
	mutex_unlock(&opts->lock);

	usb_free_all_descriptors(f);
	kfree(func_to_bi(f));
}

static int bi_function_set_alt(struct usb_function *f,
		unsigned intf, unsigned alt)
{
	struct bi_device	*ndev = func_to_bi(f);

	/* we know alt is bi_direction */

	if (ndev->in_ep->driver_data)
		disable_bi_device(ndev);
	return enable_bi_device(ndev);
}

static void bi_function_disable(struct usb_function *f)
{
	struct bi_device	*bi = func_to_bi(f);

	disable_bi_device(bi);
}

static int bi_function_setup(struct usb_function *f,
		const struct usb_ctrlrequest *ctrl)
{
	struct usb_configuration        *c = f->config;
	struct usb_request	*req = c->cdev->req;
	int			value = -EOPNOTSUPP;
	u16			w_index = le16_to_cpu(ctrl->wIndex);
	u16			w_value = le16_to_cpu(ctrl->wValue);
	u16			w_length = le16_to_cpu(ctrl->wLength);

	req->length = USB_COMP_EP0_BUFSIZ;
	/* composite driver infrastructure handles everything except
	 * the two control test requests.
	 */

    goto unknown;//todo
	switch (ctrl->bRequest) {

	default:
unknown:
		VDBG(c->cdev,
			"unknown control req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
	}

	/* respond with data transfer or status phase? */
	if (value >= 0) {
		//VDBG
		INFO(c->cdev, "in_ep/out_ep req%02x.%02x v%04x i%04x l%d\n",
			ctrl->bRequestType, ctrl->bRequest,
			w_value, w_index, w_length);
		req->zero = 0;
		req->length = value;
		value = usb_ep_queue(c->cdev->gadget->ep0, req, GFP_ATOMIC);
		if (value < 0)
			ERROR(c->cdev, "in_ep/out_ep response, err %d\n",
					value);
	}

	/* device either stalls (value < 0) or reports success */
	return value;
}

static void bi_function_suspend(struct usb_function * f)
{
    //todo
}

static void bi_function_resume(struct usb_function * f)
{
    //todo
}

static struct usb_function *bi_alloc(struct usb_function_instance *fi)
{
	struct bi_device	*bi;
	struct f_bd_opts	*bd_opts;

	bi = kzalloc(sizeof *bi, GFP_KERNEL);
	if (!bi)
		return ERR_PTR(-ENOMEM);

	bd_opts = container_of(fi, struct f_bd_opts, func_inst);

	mutex_lock(&bd_opts->lock);
	bd_opts->refcnt++;
	mutex_unlock(&bd_opts->lock);

	bi->bulk_in_buflen = bd_opts->bulk_in_buflen;
	bi->bulk_in_qlen = bd_opts->bulk_in_qlen;
	bi->bulk_out_buflen = bd_opts->bulk_out_buflen;
	bi->bulk_out_qlen = bd_opts->bulk_out_qlen;

	bi->function.name = "bi";
	bi->function.bind = bi_function_bind;
	bi->function.unbind = bi_function_unbind;
	bi->function.setup = bi_function_setup;
	bi->function.set_alt = bi_function_set_alt;
	bi->function.disable = bi_function_disable;
	bi->function.resume = bi_function_resume;
	bi->function.suspend = bi_function_suspend;
	bi->function.strings = bi_function_strings;

	bi->function.free_func = bi_function_free;

	return &bi->function;
}

static inline struct f_bd_opts *to_f_bd_opts(struct config_item *item)
{
	return container_of(to_config_group(item), struct f_bd_opts,
			    func_inst.group);
}

static void bd_attr_release(struct config_item *item)
{
	struct f_bd_opts *bd_opts = to_f_bd_opts(item);

	usb_put_function_instance(&bd_opts->func_inst);
}

static struct configfs_item_operations bd_item_ops = {
	.release		= bd_attr_release,
};

static ssize_t f_bd_opts_bulk_in_qlen_show(struct config_item *item, char *page)
{
	struct f_bd_opts *opts = to_f_bd_opts(item);
	int result;

	mutex_lock(&opts->lock);
	result = sprintf(page, "%d", opts->bulk_in_qlen);
	mutex_unlock(&opts->lock);

	return result;
}

static ssize_t f_bd_opts_bulk_out_qlen_show(struct config_item *item, char *page)
{
	struct f_bd_opts *opts = to_f_bd_opts(item);
	int result;

	mutex_lock(&opts->lock);
	result = sprintf(page, "%d", opts->bulk_out_qlen);
	mutex_unlock(&opts->lock);

	return result;
}

static ssize_t f_bd_opts_bulk_in_qlen_store(struct config_item *item,
				    const char *page, size_t len)
{
	struct f_bd_opts *opts = to_f_bd_opts(item);
	int ret;
	u32 num;

	mutex_lock(&opts->lock);
	if (opts->refcnt) {
		ret = -EBUSY;
		goto end;
	}

	ret = kstrtou32(page, 0, &num);
	if (ret)
		goto end;

	opts->bulk_in_qlen = num;
	ret = len;
end:
	mutex_unlock(&opts->lock);
	return ret;
}

static ssize_t f_bd_opts_bulk_out_qlen_store(struct config_item *item,
				    const char *page, size_t len)
{
	struct f_bd_opts *opts = to_f_bd_opts(item);
	int ret;
	u32 num;

	mutex_lock(&opts->lock);
	if (opts->refcnt) {
		ret = -EBUSY;
		goto end;
	}

	ret = kstrtou32(page, 0, &num);
	if (ret)
		goto end;

	opts->bulk_out_qlen = num;
	ret = len;
end:
	mutex_unlock(&opts->lock);
	return ret;
}

CONFIGFS_ATTR(f_bd_opts_, bulk_in_qlen);
CONFIGFS_ATTR(f_bd_opts_, bulk_out_qlen);

static ssize_t f_bd_opts_bulk_in_buflen_show(struct config_item *item, char *page)
{
	struct f_bd_opts *opts = to_f_bd_opts(item);
	int result;

	mutex_lock(&opts->lock);
	result = sprintf(page, "%d", opts->bulk_in_buflen);
	mutex_unlock(&opts->lock);

	return result;
}

static ssize_t f_bd_opts_bulk_out_buflen_show(struct config_item *item, char *page)
{
	struct f_bd_opts *opts = to_f_bd_opts(item);
	int result;

	mutex_lock(&opts->lock);
	result = sprintf(page, "%d", opts->bulk_out_buflen);
	mutex_unlock(&opts->lock);

	return result;
}

static ssize_t f_bd_opts_bulk_in_buflen_store(struct config_item *item,
				    const char *page, size_t len)
{
	struct f_bd_opts *opts = to_f_bd_opts(item);
	int ret;
	u32 num;

	mutex_lock(&opts->lock);
	if (opts->refcnt) {
		ret = -EBUSY;
		goto end;
	}

	ret = kstrtou32(page, 0, &num);
	if (ret)
		goto end;

	opts->bulk_in_buflen = num;
	ret = len;
end:
	mutex_unlock(&opts->lock);
	return ret;
}

static ssize_t f_bd_opts_bulk_out_buflen_store(struct config_item *item,
				    const char *page, size_t len)
{
	struct f_bd_opts *opts = to_f_bd_opts(item);
	int ret;
	u32 num;

	mutex_lock(&opts->lock);
	if (opts->refcnt) {
		ret = -EBUSY;
		goto end;
	}

	ret = kstrtou32(page, 0, &num);
	if (ret)
		goto end;

	opts->bulk_out_buflen = num;
	ret = len;
end:
	mutex_unlock(&opts->lock);
	return ret;
}

CONFIGFS_ATTR(f_bd_opts_, bulk_in_buflen);
CONFIGFS_ATTR(f_bd_opts_, bulk_out_buflen);

static struct configfs_attribute *bd_attrs[] = {
	&f_bd_opts_attr_bulk_in_qlen,
	&f_bd_opts_attr_bulk_out_qlen,
	&f_bd_opts_attr_bulk_in_buflen,
	&f_bd_opts_attr_bulk_out_buflen,
	NULL,
};

static struct config_item_type bd_func_type = {
	.ct_item_ops    = &bd_item_ops,
	.ct_attrs	= bd_attrs,
	.ct_owner       = THIS_MODULE,
};

static void bd_free_instance(struct usb_function_instance *fi)
{
	struct f_bd_opts *bd_opts;

	bd_opts = container_of(fi, struct f_bd_opts, func_inst);
	kfree(bd_opts);
}

static struct usb_function_instance *bi_alloc_instance(void)
{
	struct f_bd_opts *bd_opts;

	bd_opts = kzalloc(sizeof(*bd_opts), GFP_KERNEL);
	if (!bd_opts)
		return ERR_PTR(-ENOMEM);
	mutex_init(&bd_opts->lock);
	bd_opts->func_inst.free_func_inst = bd_free_instance;
	bd_opts->bulk_in_buflen = GBI_DIRECTION_BULK_IN_BUFLEN;
	bd_opts->bulk_in_qlen = GBI_DIRECTION_BULK_IN_QLEN;
	bd_opts->bulk_out_buflen = GBI_DIRECTION_BULK_OUT_BUFLEN;
	bd_opts->bulk_out_qlen = GBI_DIRECTION_BULK_OUT_QLEN;

	config_group_init_type_name(&bd_opts->func_inst.group, "",
				    &bd_func_type);

	return  &bd_opts->func_inst;
}
DECLARE_USB_FUNCTION(Bi, bi_alloc_instance, bi_alloc);

int __init bd_modinit(void)
{
	int ret;

	ret = usb_function_register(&Biusb_func);
	if (ret)
		return ret;
	return ret;
}
void __exit bd_modexit(void)
{
	usb_function_unregister(&Biusb_func);
}
module_init(bd_modinit);
module_exit(bd_modexit);

MODULE_AUTHOR("Claude Rao");
MODULE_LICENSE("GPL");
