/*
* msb250x_ep.c- Sigmastar
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
#include <linux/delay.h>
#include "msb250x_udc_reg.h"
#include "msb250x_udc.h"
#include "msb250x_gadget.h"
#include "msb250x_dma.h"

extern const char ep0name[];
#if 0
static const char *ep0states[]= {
        "EP0_IDLE",
        "EP0_IN_DATA_PHASE",
        "EP0_OUT_DATA_PHASE",
        "EP0_END_XFER",
        "EP0_STALL",
};
#endif
/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep0_clear_opr
+------------------------------------------------------------------------------
| DESCRIPTION : to clear the RxPktRdy bit
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
|                    |   |   |
+--------------------+---+---+-------------------------------------------------
*/

void msb250x_ep0_clear_opr(void)
{
    ms_writeb(MSB250X_OTG0_CSR0_SRXPKTRDY, MSB250X_OTG0_EP0_CSR0_REG);
}
EXPORT_SYMBOL(msb250x_ep0_clear_opr);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep0_clear_sst
+------------------------------------------------------------------------------
| DESCRIPTION : to clear SENT_STALL
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
|                    |   |   |
+--------------------+---+---+-------------------------------------------------
*/
void msb250x_ep0_clear_sst(void)
{
    ms_writeb(0x00, MSB250X_OTG0_EP0_CSR0_REG);
}
EXPORT_SYMBOL(msb250x_ep0_clear_sst);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep0_clear_se
+------------------------------------------------------------------------------
| DESCRIPTION : to clear the SetupEnd bit
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
|                    |   |   |
+--------------------+---+---+-------------------------------------------------
*/
void msb250x_ep0_clear_se(void)
{
    ms_writeb(MSB250X_OTG0_CSR0_SSETUPEND, MSB250X_OTG0_EP0_CSR0_REG);
}
EXPORT_SYMBOL(msb250x_ep0_clear_se);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep0_set_ipr
+------------------------------------------------------------------------------
| DESCRIPTION : to set the TxPktRdy bit affer loading a data packet into the FIFO
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
|                    |   |   |
+--------------------+---+---+-------------------------------------------------
*/
void msb250x_ep0_set_ipr(void)
{
    ms_writeb(MSB250X_OTG0_CSR0_TXPKTRDY, MSB250X_OTG0_EP0_CSR0_REG);
}
EXPORT_SYMBOL(msb250x_ep0_set_ipr);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep0_set_de
+------------------------------------------------------------------------------
| DESCRIPTION : to set the DataEnd bit
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
|                    |   |   |
+--------------------+---+---+-------------------------------------------------
*/
void msb250x_ep0_set_de(void)
{
    ms_writeb(MSB250X_OTG0_CSR0_DATAEND, MSB250X_OTG0_EP0_CSR0_REG);
}
EXPORT_SYMBOL(msb250x_ep0_set_de);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep0_set_ss
+------------------------------------------------------------------------------
| DESCRIPTION : to set the SendStall bit to terminate the current transaction
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
|                    |   |   |
+--------------------+---+---+-------------------------------------------------
*/
void msb250x_ep0_set_ss(void)
{
    ms_writeb(MSB250X_OTG0_CSR0_SENDSTALL, MSB250X_OTG0_EP0_CSR0_REG);
}
EXPORT_SYMBOL(msb250x_ep0_set_ss);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep0_set_de_out
+------------------------------------------------------------------------------
| DESCRIPTION : to clear the ServiceRxPktRdy bit and set the DataEnd bit
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
|                    |   |   |
+--------------------+---+---+-------------------------------------------------
*/
void msb250x_ep0_set_de_out(void)
{
    ms_writeb((MSB250X_OTG0_CSR0_SRXPKTRDY | MSB250X_OTG0_CSR0_DATAEND), MSB250X_OTG0_EP0_CSR0_REG);
}
EXPORT_SYMBOL(msb250x_ep0_set_de_out);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep0_set_sse_out
+------------------------------------------------------------------------------
| DESCRIPTION : to clear the ServiceRxPktRdy bit and clear the ServiceSetupEnd bit
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
|                    |   |   |
+--------------------+---+---+-------------------------------------------------
*/
void msb250x_ep0_set_sse_out(void)
{
    ms_writeb((MSB250X_OTG0_CSR0_SRXPKTRDY | MSB250X_OTG0_CSR0_SSETUPEND), MSB250X_OTG0_EP0_CSR0_REG);
}
EXPORT_SYMBOL(msb250x_ep0_set_sse_out);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep0_set_de_in
+------------------------------------------------------------------------------
| DESCRIPTION : to set the TxPktRdy bit and set the DataEnd bit
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
|                    |   |   |
+--------------------+---+---+-------------------------------------------------
*/
void msb250x_ep0_set_de_in(void)
{
    ms_writeb((MSB250X_OTG0_CSR0_TXPKTRDY | MSB250X_OTG0_CSR0_DATAEND), MSB250X_OTG0_EP0_CSR0_REG);
}
EXPORT_SYMBOL(msb250x_ep0_set_de_in);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep_enable
+------------------------------------------------------------------------------
| DESCRIPTION : configure endpoint, making it usable
|
| RETURN      : zero, or a negative error code.
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| _ep                 |x  |       | usb_ep struct point
|--------------------+---+---+-------------------------------------------------
| desc                 |x  |       | usb_endpoint_descriptor struct point
+--------------------+---+---+-------------------------------------------------
*/
int msb250x_ep_enable(struct usb_ep *_ep,
                      const struct usb_endpoint_descriptor *desc)
{
    struct msb250x_udc *dev = NULL;
    struct msb250x_ep *ep = NULL;
    u8 ep_num = 0;
    u8 csr1 = 0, csr2 = 0;

    unsigned long flags;
#if 1
    u8 power = 0;
    struct otg0_usb_power* pst_power = (struct otg0_usb_power*) &power;
#endif
    ep = to_msb250x_ep(_ep);

    if (!_ep || !desc || _ep->name == ep0name || desc->bDescriptorType != USB_DT_ENDPOINT)
    {
        printk(KERN_ERR "<USB> %s EINVAL\n", __func__);
        return -EINVAL;
    }

    dev = ep->dev;
    if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN)
    {
        printk(KERN_ERR "<USB> %s ESHUTDOWN\n", __func__);
        return -ESHUTDOWN;
    }

    spin_lock_irqsave(&ep->dev->lock, flags);//local_irq_save (flags);
    _ep->maxpacket = usb_endpoint_maxp(desc) & 0x7ff;
    _ep->mult = USB_EP_MAXP_MULT(usb_endpoint_maxp(desc));
    ep_num = usb_endpoint_num(desc);

    ep->autoNAK_cfg = 0;

    /* set type, direction, address; reset fifo counters */
    if (usb_endpoint_dir_in(desc))
    {
        csr1 = MSB250X_OTG0_TXCSR1_FLUSHFIFO | MSB250X_OTG0_TXCSR1_CLRDATAOTG;
        csr2 = MSB250X_OTG0_TXCSR2_MODE;

        if (usb_endpoint_xfer_isoc(desc))
        {
            csr2 |= MSB250X_OTG0_TXCSR2_ISOC;
        #if 0
            if (usb_endpoint_maxp(desc) > _ep->maxpacket)
            {
                power = ms_readb(MSB250X_OTG0_PWR_REG);
                pst_power->bISOUpdate = 1;
                ms_writeb(power, MSB250X_OTG0_PWR_REG);
            }
        #endif
        }

        ms_writew(usb_endpoint_maxp(desc), MSB250X_OTG0_EP_TXMAP_L_REG(ep_num));
        ms_writeb(csr1, MSB250X_OTG0_EP_TXCSR1_REG(ep_num));
        ms_writeb(csr2, MSB250X_OTG0_EP_TXCSR2_REG(ep_num));

        ep->fifo_size = 1 << (ms_readb(MSB250X_OTG0_EP_FIFOSIZE_REG(ep_num)) & 0xf0 >> 4);
    #ifndef CONFIG_USB_FPGA_VERIFICATION
        /* enable irqs */
        ms_writeb((ms_readb(MSB250X_OTG0_INTRTXE_REG) | MSB250X_OTG0_INTR_EP(ep_num)), MSB250X_OTG0_INTRTXE_REG);
    #endif
    }
    else
    {
        /* enable the enpoint direction as Rx */
        csr1 = MSB250X_OTG0_RXCSR1_FLUSHFIFO | MSB250X_OTG0_RXCSR1_CLRDATATOG;
        csr2 = 0;

        if(usb_endpoint_xfer_isoc(desc))
        {
            csr2 |= MSB250X_OTG0_RXCSR2_ISOC;
        }

        if (usb_endpoint_xfer_bulk(desc))
        {
            ep->autoNAK_cfg = msb250x_udc_get_autoNAK_cfg(ep_num);
            msb250x_udc_enable_autoNAK(ep_num, ep->autoNAK_cfg);
        }

        ms_writew(usb_endpoint_maxp(desc), MSB250X_OTG0_EP_RXMAP_L_REG(ep_num));
        ms_writeb(csr1, MSB250X_OTG0_EP_RXCSR1_REG(ep_num));
        ms_writeb(csr2, MSB250X_OTG0_EP_RXCSR2_REG(ep_num));

        ep->fifo_size = 1 << (ms_readb(MSB250X_OTG0_EP_FIFOSIZE_REG(ep_num)) & 0x0f);
        //ep->fifo_size = ms_readb(MSB250X_OTG0_FIFOSIZE_REG);
        /* enable irqs */
        ms_writeb((ms_readb(MSB250X_OTG0_INTRRXE_REG) | MSB250X_OTG0_INTR_EP(ep_num)), MSB250X_OTG0_INTRRXE_REG);
    }
#if 1
    if (usb_endpoint_xfer_isoc(desc))
    {
        power = ms_readb(MSB250X_OTG0_PWR_REG);

        if (0 == pst_power->bISOUpdate)
        {
            pst_power->bISOUpdate = 1;
            ms_writeb(power, MSB250X_OTG0_PWR_REG);
        }
    }
#endif
    msb250x_set_ep_halt(ep, 0);
    ep->shortPkt = 0;
    printk(KERN_INFO "<USB>[EP][%d] Enable for %s %s with maxpacket/fifo(%d/%d)\r\n",
           ep_num,
           usb_endpoint_xfer_bulk(desc)? "BULK" : (usb_endpoint_xfer_isoc(desc)? "ISOC" : "INT"), usb_endpoint_dir_in(desc)? "IN" : "OUT",
           _ep->maxpacket,
           ep->fifo_size);

    spin_unlock_irqrestore(&ep->dev->lock, flags);//local_irq_restore (flags);

    return 0;
}
EXPORT_SYMBOL(msb250x_ep_enable);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep_disable
+------------------------------------------------------------------------------
| DESCRIPTION : endpoint is no longer usable
|
| RETURN      : zero, or a negative error code.
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| _ep                 |x  |       | usb_ep struct point
+--------------------+---+---+-------------------------------------------------
*/
int msb250x_ep_disable(struct usb_ep *_ep)
{
    struct msb250x_ep *ep = to_msb250x_ep(_ep);
    u8 ch = 0;
    u8 ep_num = 0;
    unsigned long flags;
    struct msb250x_udc	*dev;


    if (!_ep || !_ep->desc)
    {
        printk("<USB>[%s] not enabled\n",_ep ? ep->ep.name : NULL);
        return -EINVAL;
    }

    dev = ep->dev;

    spin_lock_irqsave(&dev->lock,flags);//local_irq_save(flags);

    msb250x_set_ep_halt(ep, 1);
    ep_num = usb_endpoint_num(_ep->desc);

    if (0 < (ch = msb250x_dma_find_channel_by_ep(ep_num)))
    {
        msb250x_dma_release_channel(ch);
    }

    /* disable irqs */
    if (usb_endpoint_dir_in(_ep->desc))
    {
        ms_writeb((ms_readb(MSB250X_OTG0_INTRTXE_REG) & ~(MSB250X_OTG0_INTR_EP(ep_num))), MSB250X_OTG0_INTRTXE_REG);
    }
    else
    {
        if (usb_endpoint_xfer_bulk(_ep->desc))
        {
            msb250x_udc_release_autoNAK_cfg(ep->autoNAK_cfg);
            ep->autoNAK_cfg = 0;
        }

        ms_writeb((ms_readb(MSB250X_OTG0_INTRRXE_REG) & ~(MSB250X_OTG0_INTR_EP(ep_num))), MSB250X_OTG0_INTRRXE_REG);
    }

    msb250x_request_nuke(ep->dev, ep, -ESHUTDOWN);

    spin_unlock_irqrestore(&dev->lock,flags);//local_irq_restore(flags);
#if 0
    DMA_DUMP();
#endif

    printk("<USB>[EP][%d] disabled\n", ep_num);
//printb();
    return 0;
}
EXPORT_SYMBOL(msb250x_ep_disable);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep_alloc_request
+------------------------------------------------------------------------------
| DESCRIPTION : allocate a request object to use with this endpoint
|
| RETURN      : the request, or null if one could not be allocated.
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| _ep                |x  |       | usb_ep struct point
|--------------------+---+---+-------------------------------------------------
|gfp_flags           |x  |       | GFP_* flags to use
+--------------------+---+---+-------------------------------------------------
*/
struct usb_request*
msb250x_ep_alloc_request(struct usb_ep *_ep,
                         gfp_t gfp_flags)
{
    struct msb250x_request *req = NULL;

    if (!_ep)
        return NULL;

    req = kzalloc (sizeof(struct msb250x_request), gfp_flags);
    if (!req)
        return NULL;

    INIT_LIST_HEAD (&req->queue);
    return &req->req;
}
EXPORT_SYMBOL(msb250x_ep_alloc_request);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep_free_request
+------------------------------------------------------------------------------
| DESCRIPTION : frees a request object
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| _ep                 |x  |       | usb_ep struct point
|--------------------+---+---+-------------------------------------------------
| _req                 |x  |       |usb_request struct point
+--------------------+---+---+-------------------------------------------------
*/
void msb250x_ep_free_request(struct usb_ep *_ep,
                             struct usb_request *_req)
{
    struct msb250x_ep *ep = to_msb250x_ep(_ep);
    struct msb250x_request    *req = to_msb250x_req(_req);

    if (!ep || !_req || (!_ep->desc && _ep->name != ep0name))
        return;

    WARN_ON (!list_empty (&req->queue));
    kfree(req);
}
EXPORT_SYMBOL(msb250x_ep_free_request);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep_queue
+------------------------------------------------------------------------------
| DESCRIPTION : queues (submits) an I/O request to an endpoint
|
| RETURN      : zero, or a negative error code
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| _ep                 |x  |       | usb_ep struct point
|--------------------+---+---+-------------------------------------------------
| _req                 |x  |       |usb_request struct point
|--------------------+---+---+-------------------------------------------------
| gfp_flags          |x  |       | GFP_* flags to use
+--------------------+---+---+-------------------------------------------------
*/

int msb250x_ep_queue(struct usb_ep *_ep,
                     struct usb_request *_req,
                     gfp_t gfp_flags)
{
    struct msb250x_request* req = to_msb250x_req(_req);
    struct msb250x_ep* ep = to_msb250x_ep(_ep);
    struct msb250x_udc* dev = NULL;
    unsigned long flags;

    if (unlikely (!_ep || (!_ep->desc && ep->ep.name != ep0name)))
    {
        printk("%s: invalid args\n", __FUNCTION__);
        return -EINVAL;
    }

    dev = ep->dev;

    if (unlikely (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN))
    {
        return -ESHUTDOWN;
    }

    spin_lock_irqsave(&dev->lock, flags);//local_irq_save (flags);

    if (unlikely(!_req || !_req->complete || !_req->buf || !list_empty(&req->queue)))
    {
        if (!_req)
            printk("%s: 1 X X X\n", __FUNCTION__);
        else
        {
            printk("%s: 0 %01d %01d %01d\n",
                __FUNCTION__, !_req->complete,!_req->buf,
                !list_empty(&req->queue));
        }

        spin_unlock_irqrestore(&dev->lock,flags);//local_irq_restore(flags);
        return -EINVAL;
    }

    _req->status = -EINPROGRESS;
    _req->actual = 0;

    if (ep0name != ep->ep.name && dev->using_dma)
    {
        msb250x_gadget_map_request(ep->gadget, _req, usb_endpoint_dir_in(_ep->desc));
    }

    if(list_empty(&ep->queue))
    {
        req = msb250x_request_handler(ep, req);
    }

    if (req != NULL)
    {
        list_add_tail(&req->queue, &ep->queue);
    }

    spin_unlock_irqrestore(&dev->lock,flags);//local_irq_restore(flags);

    return 0;

}
EXPORT_SYMBOL(msb250x_ep_queue);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep_dequeue
+------------------------------------------------------------------------------
| DESCRIPTION : dequeues (cancels, unlinks) an I/O request from an endpoint
|
| RETURN      : zero, or a negative error code
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| _ep                |x  |   | usb_ep struct point
|--------------------+---+---+-------------------------------------------------
| _req               |x  |   | usb_request struct point
+--------------------+---+---+-------------------------------------------------
*/
int msb250x_ep_dequeue(struct usb_ep *_ep,
                       struct usb_request *_req)
{
    struct msb250x_ep *ep = to_msb250x_ep(_ep);
    //struct msb250x_udc  *udc = NULL;
    struct msb250x_udc	*dev;
    struct msb250x_request    *req = NULL;
    int retval = -EINVAL;
    unsigned long flags;

    dev = ep->dev;

    if (!dev->driver)
        return -ESHUTDOWN;

    if(!_ep || !_req)
        return retval;

    // udc = to_msb250x_udc(ep->gadget);

    spin_lock_irqsave(&dev->lock,flags);//local_irq_save (flags);

    list_for_each_entry (req, &ep->queue, queue)
    {
        if (&req->req == _req)
        {
            list_del_init (&req->queue);
            _req->status = -ECONNRESET;
            retval = 0;
            break;
        }
    }

    if (retval == 0)
    {
        printk("dequeued req %p from %s, len %d buf %p\n",
            req, _ep->name, _req->length, _req->buf);

        msb250x_request_done(ep, req, -ECONNRESET);
    }

    spin_unlock_irqrestore(&dev->lock,flags);//local_irq_restore (flags);
    return retval;
}
EXPORT_SYMBOL(msb250x_ep_dequeue);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep_set_halt
+------------------------------------------------------------------------------
| DESCRIPTION : sets the endpoint halt feature.
|
| RETURN      : zero, or a negative error code
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| _ep                 |x  |       | usb_ep struct point
|--------------------+---+---+-------------------------------------------------
| value                 |x  |       |set halt or not
+--------------------+---+---+-------------------------------------------------
*/
int msb250x_ep_set_halt(struct usb_ep *_ep,
                        int value)
{
    u8 csr = 0;
    u8 ep_num = 0;

    struct msb250x_ep *ep = to_msb250x_ep(_ep);
    struct otg0_ep_rxcsr_l* rxcsr_l = NULL;
    struct otg0_ep_txcsr_l* txcsr_l = NULL;

    if (unlikely (!_ep || (!_ep->desc && ep->ep.name != ep0name)))
    {
        printk("%s: inval 2\n", __FUNCTION__);
        return -EINVAL;
    }

    //spin_lock_irqsave(&ep->dev->lock,flags);//local_irq_save (flags);

    msb250x_set_ep_halt(ep, value ? 1 : 0);

    if (IS_ERR_OR_NULL(_ep->desc))
    {
        msb250x_ep0_set_ss();
        msb250x_ep0_set_de_out();
    }
    else
    {
        ep_num = usb_endpoint_num(_ep->desc);

        if (usb_endpoint_dir_out(_ep->desc))
        {
            csr = ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(ep_num));
            rxcsr_l = (struct otg0_ep_rxcsr_l*) &csr;

            if (value)
            {
                rxcsr_l->bSendStall = 1;
                ms_writeb(csr, MSB250X_OTG0_EP_RXCSR1_REG(ep_num));
            }
            else
            {
                rxcsr_l->bSendStall = 0;
                rxcsr_l->bClrDataTog = 1;
                ms_writeb(csr, MSB250X_OTG0_EP_RXCSR1_REG(ep_num));

                if (1 == rxcsr_l->bRxPktRdy)
                {
                    msb250x_request_continue(ep);
                }
            }

        }
        else
        {
            csr = ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(ep_num));
            txcsr_l = (struct otg0_ep_txcsr_l*) &csr;

            if (value)
            {
                if (1 == txcsr_l->bFIFONotEmpty)
                {
                    printk("<USB>[%s] fifo busy, cannot halt\n", _ep->name);
                    msb250x_set_ep_halt(ep, 1);
                    //spin_unlock_irqrestore(&ep->dev->lock,flags);//local_irq_restore (flags);
                    return -EAGAIN;
                }

                txcsr_l->bSendStall = 1;
                ms_writeb(csr, MSB250X_OTG0_EP_TXCSR1_REG(ep_num));
            }
            else
            {
                txcsr_l->bClrDataTog = 1;
                ms_writeb(csr, MSB250X_OTG0_EP_TXCSR1_REG(ep_num));
                ms_writeb(0, MSB250X_OTG0_EP_TXCSR1_REG(ep_num));

                if (0 == txcsr_l->bTxPktRdy)
                {
                    msb250x_request_continue(ep);
                }
            }


        }
    }

    //spin_unlock_irqrestore(&ep->dev->lock,flags);//local_irq_restore (flags);

    return 0;
}
EXPORT_SYMBOL(msb250x_ep_set_halt);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep0_handle_idle
+------------------------------------------------------------------------------
| DESCRIPTION :handle the endpoint 0 when endpoint 0 is idle
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| dev                 |x  |      | msb250x_udc struct point
|--------------------+---+---+-------------------------------------------------
| ep                      |x  |      | msb250x_ep struct point
|--------------------+---+---+-------------------------------------------------
| crq                 |x  |      | usb_ctrlrequest struct point
|--------------------+---+---+-------------------------------------------------
| ep0csr                 |x  |      | the csr0 register value
+--------------------+---+---+-------------------------------------------------
*/
static void msb250x_ep0_handle_idle(struct msb250x_udc *dev,
                             struct msb250x_ep *ep)
{
    u8 address = 0;
    int len = 0, ret = 0;

    struct usb_ctrlrequest ctrl_req;

    len =  ms_readb(MSB250X_OTG0_EP0_COUNT0_REG);

    if (sizeof(struct usb_ctrlrequest) != len)
    {
        printk(KERN_ERR "setup begin: fifo READ ERROR"
               " wanted %d bytes got %d. Stalling out...\n",
               sizeof(struct usb_ctrlrequest), len);

        msb250x_ep0_set_ss();
        return;
    }

    ms_readsb(&ctrl_req,(void *) OTG0_EP_FIFO_ACCESS_L(0), len);
#if 0
    printk(KERN_DEBUG "<USB>[EP][0][SETUP][%s] bRequestType/bRequest/wValue/wIndex/wlength(0x%02x/0x%02x/0x%04x/0x%04x/0x%04x)\r\n",
           (ctrl_req.bRequestType & USB_DIR_IN)? "IN" : "OUT",
           ctrl_req.bRequestType,
           ctrl_req.bRequest,
           le16_to_cpu(ctrl_req.wValue),
           le16_to_cpu(ctrl_req.wIndex),
           le16_to_cpu(ctrl_req.wLength));
#endif
    /* cope with automagic for some standard requests. */
    dev->req_std = (ctrl_req.bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD;
    dev->delay_status = 0;
    dev->req_pending = 1;

    if (dev->req_std)
    {
        switch (ctrl_req.bRequest)
        {
            case USB_REQ_SET_CONFIGURATION:
                //printk(KERN_DEBUG "USB_REQ_SET_CONFIGURATION ... \n");
                if (ctrl_req.bRequestType == USB_RECIP_DEVICE)
                {
                    dev->delay_status++;
                }
                break;

            case USB_REQ_SET_INTERFACE:
                //printk(KERN_DEBUG "USB_REQ_SET_INTERFACE ... \n");
                if (ctrl_req.bRequestType == USB_RECIP_INTERFACE)
                {
                    dev->delay_status++;
                }
                break;

            case USB_REQ_SET_ADDRESS:
                //printk(KERN_DEBUG "USB_REQ_SET_ADDRESS ... \n");
                if (ctrl_req.bRequestType == USB_RECIP_DEVICE)
                {
                    address = ctrl_req.wValue & 0x7F;
                    dev->address = address;
                    ms_writeb(address, MSB250X_OTG0_FADDR_REG);
                    usb_gadget_set_state(&dev->gadget, USB_STATE_ADDRESS);
                    msb250x_ep0_clear_opr();
                    return;
                }
                break;

            case USB_REQ_GET_STATUS:
                //printk(KERN_DEBUG "USB_REQ_GET_STATUS ... \n");
                msb250x_ep0_clear_opr();
                if (dev->req_std)
                {
                    if (0 == msb250x_udc_get_status(dev, &ctrl_req))
                    {
                        msb250x_ep0_set_de_in();
                        return;
                    }
                }
                break;

            case USB_REQ_CLEAR_FEATURE:
                //printk(KERN_DEBUG "USB_REQ_CLEAR_FEATURE ... \n");
                if (ctrl_req.bRequestType != USB_RECIP_ENDPOINT)
                    break;

                if (ctrl_req.wValue != USB_ENDPOINT_HALT || ctrl_req.wLength != 0)
                    break;

                msb250x_ep_set_halt(&dev->ep[ctrl_req.wIndex & 0x7f].ep, 0);
                msb250x_ep0_clear_opr();
                return;

            case USB_REQ_SET_FEATURE:
                //printk(KERN_DEBUG "USB_REQ_SET_FEATURE ... \n");

                if(ctrl_req.bRequestType == USB_RECIP_DEVICE)
                {
                    if(ctrl_req.wValue==0x02)//USB20_TEST_MODE
                    {
                        //nUsb20TestMode=crq->wIndex;
                    }
                }
                if (ctrl_req.bRequestType != USB_RECIP_ENDPOINT)
                    break;

                if (ctrl_req.wValue != USB_ENDPOINT_HALT || ctrl_req.wLength != 0)
                    break;

                msb250x_ep_set_halt(&dev->ep[ctrl_req.wIndex & 0x7f].ep, 1);
                msb250x_ep0_clear_opr();
                return;

            default:
                msb250x_ep0_clear_opr();
                break;
        }
    }
    else
    {
        msb250x_ep0_clear_opr();
    }

    if (ctrl_req.bRequestType & USB_DIR_IN)
    {
        dev->ep0state = EP0_IN_DATA_PHASE;
    }
    else
    {
        dev->ep0state = EP0_OUT_DATA_PHASE;
    }

    if (0 == ctrl_req.wLength)
    {
        dev->ep0state = EP0_IDLE;
    }

    if (dev->driver && dev->driver->setup)
    {
        spin_unlock (&dev->lock);
        ret = dev->driver->setup(&dev->gadget, &ctrl_req);
        spin_lock (&dev->lock);
    }
    else
        ret = -EINVAL;

    if (ret < 0)
    {
        if (0 < dev->delay_status)
        {
            printk("<USB> config change %02x fail %d?\n",
                ctrl_req.bRequest, ret);
            return;
        }

        if (ret == -EOPNOTSUPP)
            printk("<USB> Operation not supported\n");
        else
            printk("<USB> dev->driver->setup failed. (%d)\n", ret);

        udelay(5);
        msb250x_ep0_set_ss();
        msb250x_ep0_set_de_out();

        dev->ep0state = EP0_IDLE;

        /* deferred i/o == no response yet */
    }
    else if (dev->req_pending)
    {
        //printk(KERN_DEBUG "dev->req_pending... what now?\n");
        dev->req_pending = 0;
    }

    //printk(KERN_DEBUG "<USB>[0][EP] state(%s)\n", ep0states[dev->ep0state]);
}

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep0_isr_handler
+------------------------------------------------------------------------------
| DESCRIPTION :handle the endpoint 0
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| dev                 |x  |      | msb250x_udc struct point
+--------------------+---+---+-------------------------------------------------
*/
void msb250x_ep0_isr_handler(struct msb250x_udc *dev)
{
    u8 ep0csr = 0;

    struct msb250x_ep* ep = &dev->ep[0];
    struct msb250x_request* req = NULL;

    ep0csr = ms_readb(MSB250X_OTG0_EP0_CSR0_REG);

    /* clear stall status */
    if (ep0csr & MSB250X_OTG0_CSR0_SENTSTALL)
    {
        //printk("<USB>[0][EP] ... clear SENT_STALL ...\n");
        msb250x_request_nuke(dev, ep, -EPIPE);
        msb250x_ep0_clear_sst();
        dev->ep0state = EP0_IDLE;
        return;
    }

    /* clear setup end */
    if (ep0csr & MSB250X_OTG0_CSR0_SETUPEND)
    {
        //printk("... serviced SETUP_END ...\n");
        msb250x_request_nuke(dev, ep, 0);
        msb250x_ep0_clear_se();

        dev->ep0state = EP0_IDLE;
    }

    switch (dev->ep0state)
    {
        case EP0_IDLE:
            if (ep0csr & MSB250X_OTG0_CSR0_RXPKTRDY)
            {
                msb250x_ep0_handle_idle(dev, ep);
            }
            break;

        case EP0_IN_DATA_PHASE:     /* GET_DESCRIPTOR etc */
        case EP0_OUT_DATA_PHASE:    /* SET_DESCRIPTOR etc */
            if (!list_empty(&ep->queue))
            {
                req = list_entry(ep->queue.next, struct msb250x_request, queue);
                msb250x_set_ep_halt(ep, 0);
                msb250x_request_handler(ep, req);
            }
            break;

        default:
            dev->ep0state = EP0_IDLE;
            printk("EP0 status ... what now?\n");
            break;
    }
}
EXPORT_SYMBOL(msb250x_ep0_isr_handler);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_ep_isr_handler
+------------------------------------------------------------------------------
| DESCRIPTION :handle the endpoint except endpoint 0
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| ep                        |x  |      | msb250x_ep struct point
+--------------------+---+---+-------------------------------------------------
*/
void msb250x_ep_isr_handler(struct msb250x_udc* dev,
                            struct msb250x_ep *ep)
{
    struct usb_ep* _ep = &ep->ep;
    struct msb250x_request* req = NULL;
    struct otg0_ep_rxcsr_h* pst_rxcsr_h = NULL;
    struct otg0_ep_rxcsr_l* pst_rxcsr_l = NULL;
    struct otg0_ep_txcsr_l* pst_txcsr_l = NULL;

    u8 ep_num = 0;
    u8 csr1 = 0, csr2 = 0;
    u8 using_dma = 0;
    u16 counts = 0;
    u16 pkt_num = 0;
    //u16 ok2rcv_packets = 0;

    ep_num = usb_endpoint_num(_ep->desc);
    using_dma = msb250x_dma_find_channel_by_ep(ep_num);

    if (likely(!list_empty(&ep->queue)))
    {
        req = list_entry(ep->queue.next, struct msb250x_request, queue);
    }

    if (usb_endpoint_dir_in(_ep->desc))
    {
        csr1 = ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(ep_num));
        pst_txcsr_l = (struct otg0_ep_txcsr_l*) &csr1;

        if (1 == pst_txcsr_l->bIncompTx)
        {
            printk(KERN_DEBUG "<USB>[ep][%d] Incomplete transfer.\n", ep_num);
            //pst_txcsr_l->bIncompTx = 0;
            //ms_writeb(csr1, MSB250X_OTG0_EP_TXCSR1_REG(ep_num));
            //return;
        }

        if (1 == pst_txcsr_l->bSentStall)
        {
            pst_txcsr_l->bSentStall = 0;
            ms_writeb(csr1, MSB250X_OTG0_EP_TXCSR1_REG(ep_num));
            return;
        }

        if (!using_dma)
        {
            msb250x_set_ep_halt(ep, 0);
        }
    }
    else
    {
        csr1 = ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(ep_num));
        csr2 = ms_readb(MSB250X_OTG0_EP_RXCSR2_REG(ep_num));

        pst_rxcsr_l = (struct otg0_ep_rxcsr_l*) &csr1;
        pst_rxcsr_h = (struct otg0_ep_rxcsr_h*) &csr2;

        if (1 == pst_rxcsr_l->bSentStall)
        {
            pst_rxcsr_l->bSentStall = 0;
            ms_writeb(csr1, MSB250X_OTG0_RXCSR1_REG);
            return;
        }

        if (req)
        {
            if (1 == pst_rxcsr_l->bRxPktRdy)
            {
                if (usb_endpoint_xfer_bulk(_ep->desc))
                {
                    counts = ms_readw(MSB250X_OTG0_EP_RXCOUNT_L_REG(ep_num));
                    pkt_num = (req->req.length - req->req.actual + (_ep->maxpacket - 1)) / _ep->maxpacket;

                    if (req->req.length <= _ep->maxpacket || counts < _ep->maxpacket || 1 == pkt_num)
                    {
                        msb250x_udc_ok2rcv_for_packets(ep->autoNAK_cfg, 0);

                        if (counts < _ep->maxpacket)
                        {
                            ep->shortPkt |= 1;
                        }
                    #if 0
                        printk(KERN_DEBUG "<USB>[EP][%d] RX[INT] actual/length(0x%04x/0x%04x) dma/shortPkt/fifo/pkt_num(%d/%d/0x%04x/%d).\n",
                            ep_num,
                            req->req.actual,
                            req->req.length,
                            using_dma,
                            ep->shortPkt,
                            counts,
                            pkt_num);
                    #endif
                    }
                }

                if (using_dma)
                {
                    msb250x_dma_isr_handler(using_dma, dev); //meet short packet
                    msb250x_set_ep_halt(ep, 0);
                }
                else
                {
                    msb250x_set_ep_halt(ep, 0);
                    req = msb250x_request_handler(ep, req);
                }

                using_dma = msb250x_dma_find_channel_by_ep(ep_num); //need to refresh dma channel after handle request

                if (!using_dma)
                {
                /*
                    1. zero length packet need to continue request again.
                    2. when request is completed by riu mode, it is necessary to continue request.
                */
                    if (NULL == req || 0 == counts)
                    {
                        msb250x_set_ep_halt(ep, 0);
                    }
                }
            }

        }
    }

    msb250x_request_continue(ep);
}
EXPORT_SYMBOL(msb250x_ep_isr_handler);

