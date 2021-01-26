/* SigmaStar trade secret */
/*
* msb250x_udc.c - Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: raul.wang <raul.wang@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include <common.h>
#include <linux/list.h>
#include <msb250x_udc.h>
#include <ms_types.h>

static int msb250x_ep_enable(struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc);
static int msb250x_ep_disable(struct usb_ep *_ep);
static struct usb_request* msb250x_ep_alloc_request(struct usb_ep *ep, gfp_t gfp_flags);
static void msb250x_ep_free_request(struct usb_ep *ep, struct usb_request *_req);
static int msb250x_ep_queue(struct usb_ep* _ep, struct usb_request* _req, gfp_t gfp_flags);
static int msb250x_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req);
static int msb250x_ep_set_halt(struct usb_ep* _ep, int value);
static int msb250x_ep_fifo_status(struct usb_ep *_ep);
static void msb250x_ep_fifo_flush(struct usb_ep *_ep);

static int msb250x_usb_pullup(struct usb_gadget *_gadget, int is_active);

static const char driver_name[] = "msb250x_udc";
static const char ep0name[] = "ep0";


static struct usb_ep_ops msb250x_ep_ops = {
	.enable = msb250x_ep_enable,
	.disable = msb250x_ep_disable,

	.alloc_request = msb250x_ep_alloc_request,
	.free_request = msb250x_ep_free_request,

	.queue = msb250x_ep_queue,
	.dequeue = msb250x_ep_dequeue,

	.set_halt = msb250x_ep_set_halt,
	.fifo_status = msb250x_ep_fifo_status,
	.fifo_flush = msb250x_ep_fifo_flush,
};

static struct usb_gadget_ops msb250x_gadget_ops = {
    .pullup = msb250x_usb_pullup,
};

static struct msb250x_udc memory = {
	.gadget = {
		.ops = &msb250x_gadget_ops,
		.ep0 = &memory.ep[0].ep,
		.name = driver_name,
	},

	/* control endpoint */
	.ep[0] = {
		.ep = {
			.name = ep0name,
			.ops = &msb250x_ep_ops,
			.maxpacket = EP0_FIFO_SIZE,
		},
		.dev = &memory,
		.ep_num = 1,
	},

	/* first group of endpoints */
	.ep[1] = {
		.ep = {
			.name = "ep1",
			.ops = &msb250x_ep_ops,
			.maxpacket = EP_FIFO_SIZE,
		},
		.dev = &memory,
		.ep_num = 1,
		.fifo_size = 8192,
	},

	.ep[2] = {
		.ep = {
			.name = "ep2",
			.ops = &msb250x_ep_ops,
			.maxpacket = EP_FIFO_SIZE,
		},
		.dev = &memory,
		.ep_num = 2,
		.fifo_size = 1024,
	},

	.ep[3] = {
		.ep = {
			.name = "ep3in-int",
			.ops = &msb250x_ep_ops,
			.maxpacket = EP_FIFO_SIZE,
		},
		.dev = &memory,
		.ep_num = 3,
		.fifo_size = 64,
	},
};

static void msb250x_reset_otg(void)
{
    ms_writeb(ms_readb(MSB250X_OTG0_USB_CFG0_L) & ~MSB250X_OTG0_CFG0_SRST_N, MSB250X_OTG0_USB_CFG0_L);  //low active
    ms_writeb(ms_readb(MSB250X_OTG0_USB_CFG0_L) | MSB250X_OTG0_CFG0_SRST_N, MSB250X_OTG0_USB_CFG0_L);   //default set as 1
}

static void msb250x_request_done(struct msb250x_ep *ep, struct msb250x_request *req, int status)
{
    //unsigned halted;

	list_del_init(&req->queue);

	if (likely(req->req.status == -EINPROGRESS))
    {
		req->req.status = status;
    }
	else
    {
		status = req->req.status;
    }

	if (status && status != -ESHUTDOWN)
    {
		debug("complete %s req %p stat %d len %u/%u\n",
		      ep->ep.name, &req->req, status,
		      req->req.actual, req->req.length);
	}

	/* don't modify queue heads during completion callback */
	//ep->halted = 1;

	req->req.complete(&ep->ep, &req->req);

	//printf("<USB>[%d] callback completed\n", ep->ep_num);

	//ep->halted = halted;
}


static void msb250x_request_nuke(struct msb250x_ep *ep, int status)
{
    struct msb250x_request *req;

    if (&ep->queue == NULL)
        return;

    while (!list_empty (&ep->queue))
    {
        req = list_entry (ep->queue.next, struct msb250x_request, queue);
        msb250x_request_done(ep, req, status);
    }
}

int msb250x_udc_get_autoNAK_cfg(u8 ep_num)
{
    int cfg = 0;
    u8  ep_bulkout = 0;

    for (cfg = 0; cfg < AUTONAK_COUNT; cfg++)
    {
        switch(cfg)
        {
            case 0:
                ep_bulkout = (ms_readb(MSB250X_OTG0_AUTONAK0_EP_BULKOUT) & 0x0f);
                break;
            case 1:
                ep_bulkout = (ms_readb(MSB250X_OTG0_AUTONAK1_EP_BULKOUT) & 0x0f);
                break;
            case 2:
                ep_bulkout = (ms_readb(MSB250X_OTG0_AUTONAK2_EP_BULKOUT) & 0xf0);
                ep_bulkout = ep_bulkout >> 4;
                break;
        }

        if (0 == ep_bulkout)
        {
            return (cfg + 1);
        }
    }

    return 0;
}

int msb250x_udc_disable_autoNAK_cfg(u8 cfg)
{
    cfg--;

    switch(cfg)
    {
        case 0:
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK0_EP_BULKOUT) & 0xf0), MSB250X_OTG0_AUTONAK0_EP_BULKOUT);
            ms_writew(0, MSB250X_OTG0_AUTONAK0_CTRL);
            break;
        case 1:
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK1_EP_BULKOUT) & 0xf0), MSB250X_OTG0_AUTONAK1_EP_BULKOUT);
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK1_CTRL) & ~(MSB250X_OTG0_AUTONAK1_EN | MSB250X_OTG0_AUTONAK1_OK2Rcv | MSB250X_OTG0_AUTONAK1_AllowAck)),
                      MSB250X_OTG0_AUTONAK1_CTRL);
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK1_RX_PKT_CNT) & 0xE000), MSB250X_OTG0_AUTONAK1_RX_PKT_CNT);
            break;
        case 2:
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK2_EP_BULKOUT) & 0x0f), MSB250X_OTG0_AUTONAK2_EP_BULKOUT);
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK2_CTRL) & ~(MSB250X_OTG0_AUTONAK2_EN | MSB250X_OTG0_AUTONAK2_OK2Rcv | MSB250X_OTG0_AUTONAK2_AllowAck)), MSB250X_OTG0_AUTONAK2_CTRL);
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK2_RX_PKT_CNT) & 0xE000), MSB250X_OTG0_AUTONAK2_RX_PKT_CNT);
            break;
    }

    return 0;
}

int msb250x_udc_init_autoNAK_cfg(void)
{
    int cfg = 0;

    for (cfg = 0; cfg < AUTONAK_COUNT; cfg++)
    {
        msb250x_udc_disable_autoNAK_cfg((cfg + 1));
    }

    //ms_writew(0, MSB250X_OTG0_USB_CFG5);
    //ms_writeb((ms_readb(MSB250X_OTG0_USB_CFG6_H) | 0x20), MSB250X_OTG0_USB_CFG6_H);//short_mode
    return 0;
}

void msb250x_udc_enable_autoNAK(u8 ep_num, u8 cfg)
{
    cfg--;

    switch(cfg)
    {
        case 0:
            ep_num &= 0x0f;
            ms_writeb(ep_num, MSB250X_OTG0_AUTONAK0_EP_BULKOUT);
            {
                u16 ctrl = 0;
                ctrl |= MSB250X_OTG0_AUTONAK0_EN;
                ms_writew(ctrl, MSB250X_OTG0_AUTONAK0_CTRL);
            }
            break;
        case 1:
            ep_num &= 0x0f;
            ms_writeb(MSB250X_OTG0_AUTONAK1_EN | ep_num, MSB250X_OTG0_AUTONAK1_CTRL);
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK1_CTRL) & ~(MSB250X_OTG0_AUTONAK1_OK2Rcv | MSB250X_OTG0_AUTONAK1_AllowAck)), MSB250X_OTG0_AUTONAK1_CTRL);
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK1_RX_PKT_CNT) & 0xE000), MSB250X_OTG0_AUTONAK1_RX_PKT_CNT);
            break;
        case 2:
            ep_num = ep_num << 4;
            ep_num &= 0xf0;
            ms_writew(MSB250X_OTG0_AUTONAK2_EN | (u16) ep_num, MSB250X_OTG0_AUTONAK2_CTRL);
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK2_CTRL) & ~(MSB250X_OTG0_AUTONAK2_OK2Rcv | (u16) MSB250X_OTG0_AUTONAK2_AllowAck)), MSB250X_OTG0_AUTONAK2_CTRL);
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK2_RX_PKT_CNT) & 0xE000), MSB250X_OTG0_AUTONAK2_RX_PKT_CNT);
            break;

    }

    debug("<USB>[NAK][%d] NAK enabled.\n", cfg);
}

void msb250x_udc_ok2rcv_for_packets(u8 cfg, u16 pkt_num)
{
    u16 ctrl = 0;
    cfg--;

#if defined(GPIO_LED0_ENABLED)
    if (0 < pkt_num)
    {
        ms_writeb(ms_readb(GET_REG8_ADDR(REG_ADDR_BASE_PM_GPIO, 0x94)) | BIT1, GET_REG8_ADDR(REG_ADDR_BASE_PM_GPIO, 0x94));
    }
    else
    {
        ms_writeb(ms_readb(GET_REG8_ADDR(REG_ADDR_BASE_PM_GPIO, 0x94)) & ~BIT1, GET_REG8_ADDR(REG_ADDR_BASE_PM_GPIO, 0x94));
    }
#endif

    switch(cfg)
    {
        case 0:
            ctrl |= MSB250X_OTG0_AUTONAK0_EN;
            ctrl |= (pkt_num & ~0xE000);

            if (0 < pkt_num)
            {
                ctrl |= MSB250X_OTG0_AUTONAK0_OK2Rcv;
            }

            ms_writew(ctrl, MSB250X_OTG0_AUTONAK0_CTRL);
            break;
        case 1:
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK1_RX_PKT_CNT) & ~0xE000) | pkt_num, MSB250X_OTG0_AUTONAK1_RX_PKT_CNT);
            ctrl = ms_readb(MSB250X_OTG0_AUTONAK1_CTRL);

            if (0 < pkt_num)
            {
                ctrl |= MSB250X_OTG0_AUTONAK1_OK2Rcv;
            }

            ms_writeb(ctrl & 0xFF, MSB250X_OTG0_AUTONAK1_CTRL);
            break;
        case 2:
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK2_RX_PKT_CNT) & ~0xE000) | pkt_num, MSB250X_OTG0_AUTONAK2_RX_PKT_CNT);
            ctrl = ms_readw(MSB250X_OTG0_AUTONAK2_CTRL);

            if (0 < pkt_num)
            {
                ctrl |= MSB250X_OTG0_AUTONAK2_OK2Rcv;
            }

            ms_writew(ctrl, MSB250X_OTG0_AUTONAK2_CTRL);
            break;

    }



    debug("<USB>[NAK][%d] ok2rcv %d packets\n", cfg, pkt_num);
}

void msb250x_udc_allowAck(u8 cfg)
{
    cfg--;

    switch(cfg)
    {
        case 0:
            ms_writew(ms_readw(MSB250X_OTG0_AUTONAK0_CTRL), MSB250X_OTG0_AUTONAK0_CTRL);
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK0_CTRL) | MSB250X_OTG0_AUTONAK0_AllowAck), MSB250X_OTG0_AUTONAK0_CTRL);
            break;
        case 1:
            ms_writeb(ms_readb(MSB250X_OTG0_AUTONAK1_CTRL), MSB250X_OTG0_AUTONAK1_CTRL);
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK1_CTRL) | MSB250X_OTG0_AUTONAK1_AllowAck), MSB250X_OTG0_AUTONAK1_CTRL);
            break;
        case 2:
            ms_writeb(ms_readw(MSB250X_OTG0_AUTONAK2_CTRL), MSB250X_OTG0_AUTONAK2_CTRL);
            ms_writeb((ms_readw(MSB250X_OTG0_AUTONAK2_CTRL) | MSB250X_OTG0_AUTONAK2_AllowAck), MSB250X_OTG0_AUTONAK2_CTRL);
            break;
    }
    debug("<USB>[NAK][%d] allowAck\n", cfg);
}

u16 msb250x_udc_get_pkt_cnt(u8 cfg)
{
    u16 count = 0;
    cfg--;

    switch(cfg)
    {
        case 0:
            count = ms_readw(MSB250X_OTG0_AUTONAK0_CTRL);
            break;
        case 1:
            count = ms_readw(MSB250X_OTG0_AUTONAK1_RX_PKT_CNT);
            break;
        case 2:
            count = ms_readw(MSB250X_OTG0_AUTONAK2_RX_PKT_CNT);
            break;
        default:
            count = 0;
    }

    count &= ~0xE000;

    debug("<USB>[NAK][%d] %d packtes\n", cfg, count);

    return count;
}

static int msb250x_ep_read_riu(struct msb250x_ep* ep, struct msb250x_request* req)
{
    u8 *buf = NULL;
    int is_last = 0;
    u16 count = 0;
    u16 bytedone = 0;
    u8 ep_num = 0;

    ep_num = ep->ep_num;

    if (0 == req->req.length)
    {
        return 1;
    }

    buf = req->req.buf + req->req.actual;

    count = ms_readw(MSB250X_OTG0_EP_RXCOUNT_REG(ep_num));

    if ((req->req.length - req->req.actual) < count)
    {
        printf("<USB_WARN>[RX][%d] Bytes in fifo(0x%04x) is more than buffer size. buf/actual/length(0x%p/0x%04x/0x%04x)!\n",
               ep_num,
               count,
               req->req.buf,
               req->req.actual,
               req->req.length);
    }

    count = min((u16) count, (u16)(req->req.length - req->req.actual));

    if (0 < count)
    {
        while (count > bytedone)
        {
            *buf++ = ms_readb(OTG0_EP_FIFO_ACCESS_L(ep_num));
            bytedone++;
        }

        req->req.actual += count;

        if (bytedone != ep->ep.maxpacket || req->req.zero)
        {
            is_last = 1;
        }
    }

    is_last = (req->req.length == req->req.actual || 0 == count)? 1 : is_last;

    if (0 != ep_num && !usb_endpoint_xfer_int(ep->desc))
    {
        debug("<USB>[RX][%d] count/maxpacket(0x%04x/0x%04x) buff/last/actual/length(%p/%d/0x%04x/0x%04x) \n",
               ep_num,
               count,
               ep->ep.maxpacket,
               req->req.buf,
               is_last,
               req->req.actual,
               req->req.length);
    }

    return is_last;
}

static int msb250x_ep_write_riu(struct msb250x_ep *ep, struct msb250x_request *req)
{
    u16 count = 0;
    u32 bytedone = 0;
    u8 *buf = NULL;
    int is_last = 0;
    u8 ep_num = 0;
    //printf("msb250x_ep_write_riu\n");

    ep_num = ep->ep_num;

    count = min((u32) ep->ep.maxpacket, (u32) (req->req.length - req->req.actual));
    buf = req->req.buf + req->req.actual;

    if (0 < count)
    {

        while (count > bytedone)
        {
            ms_writeb(*buf++, OTG0_EP_FIFO_ACCESS_L(ep_num));
            bytedone++;
        }

        req->req.actual += count;
    }

    /* last packet is often short (sometimes a zlp) */
    if (count != ep->ep.maxpacket || req->req.length == req->req.actual)
    {
        is_last = 1;
    }

    if (0 != ep_num && !usb_endpoint_xfer_int(ep->desc))
    {
        debug("<USB>[TX][%d] maxpacket/bytesDone(0x%04x/0x%04x) buff/last/actual/length(%p/%d/0x%04x/0x%04x) \n",
               ep_num,
               ep->ep.maxpacket,
               count,
               req->req.buf,
               is_last,
               req->req.actual,
               req->req.length);
    }

    return is_last;
}

static void msb250x_udc_pullup_i(int is_on)
{
    u8 power = 0;
    struct otg0_usb_power* pst_power = (struct otg0_usb_power*) &power;

    power = ms_readb(MSB250X_OTG0_PWR_REG);
    pst_power->bSoftConn = is_on;

    ms_writeb(power, MSB250X_OTG0_PWR_REG);

    printf("<USB>[UDC] PULL %s D+.\n", (is_on)? "UP" : "DOWN");
}

struct msb250x_request* msb250x_ep_handle_request(struct msb250x_ep* ep, struct msb250x_request* req)
{
    struct msb250x_udc* dev = ep->dev;

    u16 pkt_num = 0;

    u8  csr = 0;
    u8 intrusb = 0;
    u8 is_last = 0;
    u8 ep_num = 0;

    struct otg0_ep0_csr_l* csr_l = NULL;
    struct otg0_ep_rxcsr_l* rxcsr_l = NULL;
    struct otg0_ep_txcsr_l* txcsr_l = NULL;
    struct otg0_usb_intr* st_intrusb = (struct otg0_usb_intr*) &intrusb;

    ep_num = ep->ep_num;

    if (1 == st_intrusb->bReset)
    {
        return req;
    }

    if (0 == ep->halted)
    {
        //set_ep_halt(ep, 1);

        intrusb = ms_readb(MSB250X_OTG0_INTRUSB_REG);

        if (1 == st_intrusb->bReset)
        {
            return req;
        }

        if (0 == ep_num)
        {
            csr = ms_readb(MSB250X_OTG0_EP0_CSR0_REG);
            csr_l = (struct otg0_ep0_csr_l*) &csr;

            switch (ep->dev->ep0state)
            {
                case EP0_IDLE:
                    if (0 < dev->delay_status)
                    {
                        msb250x_ep0_clear_opr();
                        dev->delay_status--;
                        is_last = 1;
                    }
                    break;
                case EP0_IN_DATA_PHASE:
                    if (0 == csr_l->bTxPktRdy)
                    {
                        is_last = msb250x_ep_write_riu(ep, req);

                        if (is_last)
                        {
                            msb250x_ep0_set_de_in();
                            ep->dev->ep0state = EP0_IDLE;
                        }
                        else
                        {
                            msb250x_ep0_set_ipr();
                        }
                    }

                    break;

                case EP0_OUT_DATA_PHASE:
                    if (1 == csr_l->bRxPktRdy)
                    {
                        is_last = msb250x_ep_read_riu(ep, req);

                        if (is_last)
                        {
                            msb250x_ep0_set_de_out();
                            ep->dev->ep0state = EP0_IDLE;
                        }
                        else
                        {
                            msb250x_ep0_clear_opr();
                        }
                    }
                    break;
                default:
                    printf("<USB>[0] Request Error !!\n");
            }
            set_ep_halt(ep, 0);
        }
        else
        {
            if (usb_endpoint_dir_in(ep->desc))
            {
                txcsr_l = (struct otg0_ep_txcsr_l*) &csr;
                csr = ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(ep_num));

                //if (0 == (txcsr_l->bTxPktRdy | txcsr_l->bFIFONotEmpty))
                if (0 == txcsr_l->bTxPktRdy)
                {
                    is_last = msb250x_ep_write_riu(ep, req);
                    msb250x_ep_set_ipr(ep_num);
                }
            }
            else if (usb_endpoint_dir_out(ep->desc))
            {
                csr = ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(ep_num));
                rxcsr_l = (struct otg0_ep_rxcsr_l*) &csr;

                /* DMA RX: */
                if (!usb_endpoint_xfer_bulk(ep->desc))
                {
                    if (1 == rxcsr_l->bRxPktRdy)
                    {
                        is_last = msb250x_ep_read_riu(ep, req);
                        msb250x_ep_set_ipr(ep_num);
                        set_ep_halt(ep, 0);
                    }
                }
                else
                {
                    if (0 == rxcsr_l->bRxPktRdy)
                    {
                        if (1 == ep->shortPkt)
                        {
                            udelay(125);
                        }

                        //msb250x_udc_allowAck(ep->autoNAK_cfg);
                    #if 1
                        if (ep->ep.maxpacket >= req->req.length - req->req.actual)
                        {
                            msb250x_udc_allowAck(ep->autoNAK_cfg);
                            ep->shortPkt = 0;
                        }
                        else
                        {
                            pkt_num = ((req->req.length - req->req.actual) + (ep->ep.maxpacket - 1)) / ep->ep.maxpacket;
                            msb250x_udc_ok2rcv_for_packets(ep->autoNAK_cfg, pkt_num);
                            ep->shortPkt = 1;
                        }
                    #endif
                    }
                    else
                    {
                        is_last = msb250x_ep_read_riu(ep, req);
                        msb250x_ep_set_opr(ep_num);
                    }

                }
            }

            //
        }

    }

    if (1 == is_last)
    {
        msb250x_request_done(ep, req, 0);
        req = NULL;
    }

    return req;
}

static void msb250x_udc_init_MIU_sel(void)
{
    printk("<USB> config miu select [%x] [%x] [%x] [%x].\n", USB_MIU_SEL0, USB_MIU_SEL1, USB_MIU_SEL2, USB_MIU_SEL3);
    ms_writeb(USB_MIU_SEL0, GET_REG16_ADDR(USBC_BASE_ADDR, 0x0A));                                              //Setting MIU0 segment
    ms_writeb(USB_MIU_SEL1, GET_REG16_ADDR(USBC_BASE_ADDR, 0x0B));                                              //Setting MIU1 segment
    ms_writeb(USB_MIU_SEL2, GET_REG16_ADDR(USBC_BASE_ADDR, 0x0B) + 1);                                          //Setting MIU2 segment
    ms_writeb(USB_MIU_SEL3, GET_REG16_ADDR(USBC_BASE_ADDR, 0x0C));                                              //Setting MIU3 segment
    ms_writeb((ms_readb(GET_REG16_ADDR(USBC_BASE_ADDR, 0x0C) + 1) | 0x1), GET_REG16_ADDR(USBC_BASE_ADDR, 0x0C) + 1);  //Enable miu partition mechanism
}

void msb250x_udc_init_utmi(void)
{
    u8 ctrl_l = 0;

    struct usbc0_rst_ctrl_l* pst_usbc0_rst_ctrl_l = (struct usbc0_rst_ctrl_l*) &ctrl_l;
    // Disable UHC and OTG controllers
    //USBC_REG_WRITE8(0x4, USBC_REG_READ8(0x4)& (~0x3));
#if !defined(CONFIG_USB_MS_OTG)
    // Enable OTG controller
    ms_writeb(((ms_readb(GET_REG8_ADDR(USBC_BASE_ADDR, 0x01 * 2)) & ~(BIT0 | BIT1)) | BIT1), GET_REG8_ADDR(USBC_BASE_ADDR, 0x01 * 2));

#endif

#ifndef USB_ENABLE_UPLL
    //UTMI_REG_WRITE16(0, 0x4000);
    ms_writew(0x6BC3, GET_REG8_ADDR(UTMI_BASE_ADDR, 0x00)); // Turn on UPLL, reg_pdn: bit<9> reg_pdn: bit<15>, bit <2> ref_pdn
    mdelay(1);
    ms_writeb(0x69, GET_REG8_ADDR(UTMI_BASE_ADDR, 0x01));   // Turn on UPLL, reg_pdn: bit<9>
    mdelay(2);
    ms_writew(0x0001, GET_REG8_ADDR(UTMI_BASE_ADDR, 0x00)); //Turn all (including hs_current) use override mode
    // Turn on UPLL, reg_pdn: bit<9>
    mdelay(3);
#else
    // Turn on UTMI if it was powered down
    if (0x0001 != ms_readw(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x00)))
    {
        ms_writew(0x0001, GET_REG16_ADDR(UTMI_BASE_ADDR, 0x00)); //Turn all (including hs_current) use override mode
        mdelay(3);
    }
#endif
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x3c)) | 0x01), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x3c)); // set CA_START as 1
    mdelay(10);
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x3c)) & ~0x01), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x3c)); // release CA_START
    while (0 == (ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x3c)) & 0x02));        // polling bit <1> (CA_END)

    // Reset OTG controllers
    //USBC_REG_WRITE8(0, 0xC);
    //USBC_REG_WRITE8(0, USBC_REG_READ8(0)|(OTG_BIT3|OTG_BIT2));
    ctrl_l = ms_readb(GET_REG8_ADDR(USBC_BASE_ADDR, 0x00));
    pst_usbc0_rst_ctrl_l->bOTG_RST = 1;
    pst_usbc0_rst_ctrl_l->bREG_SUSPEND = 1;
    ms_writeb(ctrl_l, GET_REG8_ADDR(USBC_BASE_ADDR, 0x00));

    // Unlock Register R/W functions  (RST_CTRL[6] = 1)
    // Enter suspend  (RST_CTRL[3] = 1)
    //USBC_REG_WRITE8(0, 0x48);
    //USBC_REG_WRITE8(0, (USBC_REG_READ8(0)&~(OTG_BIT2))|OTG_BIT6);
    ctrl_l = ms_readb(GET_REG8_ADDR(USBC_BASE_ADDR, 0x00));
    pst_usbc0_rst_ctrl_l->bOTG_RST = 0;
    pst_usbc0_rst_ctrl_l->bOTG_XIU_ENABLE = 1;
    ms_writeb(ctrl_l, GET_REG8_ADDR(USBC_BASE_ADDR, 0x00));

    //printf("<USB> +UTMI\n");

    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x06)) & 0x9F) | 0x40, GET_REG8_ADDR(UTMI_BASE_ADDR, 0x06));          //reg_tx_force_hs_current_enable
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x03)) | 0x28), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x03));                 //Disconnect window select
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x03)) & 0xef), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x03));                 //Disconnect window select
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x07)) & 0xfd), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x07));                 //Disable improved CDR
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x09)) | 0x81), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x09));                 // UTMI RX anti-dead-loc, ISI effect improvement
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x15)) | 0x20), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x15));                 // Chirp signal source select
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x0b)) | 0x80), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x0b));                 // set reg_ck_inv_reserved[6] to solve timing problem

#if 0//defined(CONFIG_MSTAR_CEDRIC)
    //UTMI_REG_WRITE8(0x2c*2,   0x10);
    //UTMI_REG_WRITE8(0x2d*2-1, 0x02);
    //UTMI_REG_WRITE8(0x2f*2-1, 0x81);
    ms_writeb(0x10, GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16));
    ms_writeb(0x02, GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16) + 1);
    ms_writeb(0x81, GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17) + 1);
#else
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16)) | 0x10), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16));
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16) + 1) | 0x02), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16) + 1);
    //ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17)) | 0x10), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17));
    //ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17) + 1) | 0x01), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17) + 1);
#if 0
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16)) | 0x10), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16));
    //ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x2c)) | 0x98), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x2c));
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x2d)) | 0x02), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x2d));
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x2e)) | 0x10), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x2e));
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x2f)) | 0x01), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x2f));
#endif
#endif

    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x02)) | 0x80), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x02));           //avoid glitch

    //printf("<USB> -UTMI\n");
}

/*
 *	udc_enable - enable USB device controller
 */

void msb250x_udc_init_otg(void)
{
    //u8 ep_num = 0;
    u8 power = 0;
    u8 usb_cfg_h = 0;
    u8 otg0_usb_intr = 0;

    struct otg0_usb_cfg0_h* pst_otg0_usb_cfg0_h = (struct otg0_usb_cfg0_h*) &usb_cfg_h;
    struct otg0_usb_cfg6_h* pst_otg0_usb_cfg6_h = NULL;
    struct otg0_usb_power* pst_power = (struct otg0_usb_power*) &power;
    struct otg0_usb_intr* pst_intr_usb = (struct otg0_usb_intr*) &otg0_usb_intr;

    debug("<USB> %s[+]\n", __func__);
    ms_writeb((ms_readb(MSB250X_OTG0_USB_CFG6_H) | (CFG6_H_SHORT_MODE | CFG6_H_BUS_OP_FIX | CFG6_H_REG_MI_WDFIFO_CTRL)), MSB250X_OTG0_USB_CFG6_H);
    usb_cfg_h = ms_readb(MSB250X_OTG0_USB_CFG0_H);
    //printf("<USB> Reset OTG\n");

    pst_otg0_usb_cfg0_h->bDMPullDown = 1;
    ms_writeb(usb_cfg_h, MSB250X_OTG0_USB_CFG0_H);      //Enable DM pull down
    //printf("<USB> Enable DM pull down\n");

    // Set FAddr to 0
    ms_writeb(0, MSB250X_OTG0_FADDR_REG);

    pst_otg0_usb_cfg6_h = (struct otg0_usb_cfg6_h*) &usb_cfg_h;
    usb_cfg_h = ms_readb(MSB250X_OTG0_USB_CFG6_H);

    pst_otg0_usb_cfg6_h->bINT_WR_CLR_EN = 1;
    pst_otg0_usb_cfg6_h->bBusOPFix = 1;
    ms_writeb(usb_cfg_h, MSB250X_OTG0_USB_CFG6_H);

    /*
    USB_REG_WRITE8(M_REG_CFG6_H, USB_REG_READ8(M_REG_CFG6_H) | 0x08);
    USB_REG_WRITE8(M_REG_CFG6_H, USB_REG_READ8(M_REG_CFG6_H) | 0x40);
    */

    //while(0x18 != (USB_REG_READ8(M_REG_DEVCTL) & 0x18));

    power = ms_readb(MSB250X_OTG0_PWR_REG);

    pst_power->bSuspendMode = 0;
    pst_power->bSoftConn = 0;
    pst_power->bHSEnab = 1;

    ms_writeb(power, MSB250X_OTG0_PWR_REG);

    ms_writeb(0x00, MSB250X_OTG0_DEVCTL_REG);

    otg0_usb_intr = 0xff;
    pst_intr_usb->bSOF = 0;
    ms_writeb(otg0_usb_intr, MSB250X_OTG0_INTRUSB_REG);

    ms_readb(MSB250X_OTG0_INTRUSB_REG);

    // Flush the next packet to be transmitted/ read from the endpoint 0 FIFO
    ms_writeb(0x00, MSB250X_OTG0_INDEX_REG);
    ms_writeb(0x01 , MSB250X_OTG0_CSR0_FLSH_REG);

    ms_writew(0x0001, MSB250X_OTG0_INTRTXE_REG);
    ms_writew(0x0001, MSB250X_OTG0_INTRRXE_REG);

    ms_readw(MSB250X_OTG0_INTRTXE_REG);
    ms_readw(MSB250X_OTG0_INTRRXE_REG);

    debug("<USB> %s[-]\n", __func__);
}


/*
 *	udc_disable - disable USB device controller
 */
static void msb250x_udc_disable(struct msb250x_udc *dev)
{
	/* block all irqs */
    ms_writeb(0x00, MSB250X_OTG0_INTRUSBE_REG);
    ms_writeb(0x00, MSB250X_OTG0_INTRTX1E_REG);
    ms_writeb(0x00, MSB250X_OTG0_INTRRX1E_REG);

	/* if hardware supports it, disconnect from usb */
	msb250x_udc_pullup_i(0);

	ep0_idle(dev);
	dev->gadget.speed = USB_SPEED_UNKNOWN;
}

void usb_ep_set_maxpacket_limit(struct usb_ep *ep,
					      unsigned maxpacket_limit)
{
	ep->maxpacket = maxpacket_limit;
}

static void msb250x_udc_reinit(struct msb250x_udc *dev)
{
    u8 ep_num = 0;

    /* device/ep0 records init */
    INIT_LIST_HEAD (&dev->gadget.ep_list);
    INIT_LIST_HEAD (&dev->gadget.ep0->ep_list);

    dev->gadget.ep0 = &dev->ep[0].ep;
    dev->gadget.speed = USB_SPEED_UNKNOWN;

    for (ep_num = 0; ep_num < MSB250X_MAX_ENDPOINTS; ep_num++)
    {
        struct msb250x_ep *ep = &dev->ep[ep_num];

        ep->ep_num = ep_num;
        ep->autoNAK_cfg = 0;
        list_add_tail (&ep->ep.ep_list, &dev->gadget.ep_list);
        INIT_LIST_HEAD (&ep->queue);
    }

    usb_ep_set_maxpacket_limit(&dev->ep[0].ep, 64);
    list_del_init (&dev->ep[0].ep.ep_list);

    dev->delay_status = 0;

}

static int msb250x_udc_link_isr(struct msb250x_udc *dev)
{
    u8 usb_status = 0;
    u8 power = 0;

    struct otg0_usb_intr* usb_st = (struct otg0_usb_intr*) &usb_status;
    struct otg0_usb_power* pst_power = (struct otg0_usb_power*) &power;

    power = ms_readb(MSB250X_OTG0_PWR_REG);

    usb_status = ms_readb(MSB250X_OTG0_INTRUSB_REG);
    ms_writeb(usb_status, MSB250X_OTG0_INTRUSB_REG);

    if (1 == usb_st->bReset)
    {
        /* two kind of reset :
         * - reset start -> pwr reg = 8
         * - reset end   -> pwr reg = 0
         **/

        ms_writeb(0x10, GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16));        //TX-current adjust to 105%=> bit <4> set 1
        ms_writeb(0x02, (GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16) + 1));  // Pre-emphasis enable=> bit <1> set 1
        //ms_writeb(0x01, (GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17) + 1));  //HS_TX common mode current enable (100mV)=> bit <7> set 1

        //ms_writeb(0x10, (UTMI_BASE_ADDR + 0x58));           //TX-current adjust to 105%=> bit <4> set 1
        //ms_writeb(0x02, (UTMI_BASE_ADDR + 0x5A));           // Pre-emphasis enable=> bit <1> set 1
        //ms_writeb(0x01, (UTMI_BASE_ADDR + 0x5E));           //HS_TX common mode current enable (100mV)=> bit <7> set 1

        if (dev->driver && dev->driver->disconnect)
        {
            dev->driver->disconnect(&dev->gadget);
        }

        ms_writeb(0, MSB250X_OTG0_FADDR_REG);

        dev->address = 0;
        dev->delay_status = 0;
        dev->ep0state = EP0_IDLE;

        //if (udc_read8(MSB250X_UDC_PWR_REG)&MSB250X_UDC_PWR_HS_MODE)

        if (1 != pst_power->bHSMode)
        {
            //printf("<USB>[LINK] Not high speed device.\r\n");
            usb_gadget_disconnect(&dev->gadget);
            msb250x_reset_otg();
            mdelay(10);
            usb_gadget_connect(&dev->gadget);
            return IRQ_HANDLED;
        }
        dev->gadget.speed = USB_SPEED_HIGH;
        printf("<USB>[LINK] High speed device.\r\n");
        ms_writew(0x0230, GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16));  //B2 analog parameter

        msb250x_udc_init_autoNAK_cfg();

    #if 0
        if (1 == pst_power->bHSMode)
        {
            dev->gadget.speed = USB_SPEED_HIGH;
            printf("<USB>[LINK] High speed device.\r\n");
            ms_writew(0x0230, (UTMI_BASE_ADDR + 0x58));  //B2 analog parameter
        }
        else
        {
            dev->gadget.speed = USB_SPEED_FULL;
            printf("<USB>[LINK] Full speed device.\r\n");
            ms_writew(0x0030, (UTMI_BASE_ADDR + 0x58));  //B2 analog parameter
        }
    #endif

    }

    /* RESUME */
    if (1 == usb_st->bResume)
    {
        printf("<USB>[LINK] Resume power<0x%x>.\r\n", power);

        if (dev->gadget.speed != USB_SPEED_UNKNOWN
                && dev->driver
                && dev->driver->resume)
            dev->driver->resume(&dev->gadget);
    }

    /* SUSPEND */
    if (1 == usb_st->bSuspend)
    {
        printf("<USB>[LINK] Suspend.\r\n");

        //if (1 == pst_power->bSuspendMode)
        {
            if (dev->gadget.speed != USB_SPEED_UNKNOWN &&
                dev->driver &&
                dev->driver->suspend)
            {
                dev->driver->suspend(&dev->gadget);
                dev->ep0state = EP0_IDLE;
            }

        }

    }

    return IRQ_HANDLED;
}

static int msb250x_ep_set_halt(struct usb_ep* _ep, int value)
{
    struct msb250x_ep* ep = container_of(_ep, struct msb250x_ep, ep);

    u8 csr = 0;
    u8 ep_num = 0;

    struct otg0_ep_rxcsr_l* rxcsr_l = NULL;
    struct otg0_ep_txcsr_l* txcsr_l = NULL;

    if (unlikely (!_ep || (!ep->desc && ep->ep.name != ep0name)))
    {
        printf("%s: inval 2\n", __FUNCTION__);
        return -EINVAL;
    }

    ep_num = ep->ep_num;

    ep->halted = value ? 1 : 0;

    if (ep_num == 0)
    {
        msb250x_ep0_set_ss();
    }
    else
    {
        if (NULL != ep->desc)
        {
            if (usb_endpoint_dir_out(ep->desc))
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
                        //msb250x_ep_request_continue(ep);
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
                        printf("%s fifo busy, cannot halt\n", _ep->name);
                        ep->halted = 0;
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
                        //msb250x_ep_request_continue(ep);
                    }
                }


            }
        }

    }

    return 0;
}

static int msb250x_udc_get_status(struct msb250x_udc *dev, struct usb_ctrlrequest *crq)
{
    u8 status = 0;
    u8 ep_num = crq->wIndex & 0x7F;
    u8 is_in = crq->wIndex & USB_DIR_IN;

    switch (crq->bRequestType & USB_RECIP_MASK)
    {
        case USB_RECIP_INTERFACE:
            break;

        case USB_RECIP_DEVICE:
            status = dev->devstatus;
            break;

        case USB_RECIP_ENDPOINT:
            if (MSB250X_MAX_ENDPOINTS < ep_num || crq->wLength > 2)
                return 1;

            if (ep_num == 0)
            {
                status = ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(0));
                status &= MSB250X_OTG0_CSR0_SENDSTALL;
            }
            else
            {
                if (is_in)
                {
                    status = ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(ep_num));
                    status &= MSB250X_OTG0_TXCSR1_SENDSTALL;
                }
                else
                {
                    status = ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(ep_num));
                    status &= MSB250X_OTG0_RXCSR1_SENDSTALL;
                }
            }

            status = status ? 1 : 0;
            break;

        default:
            return 1;
    }

    /* Seems to be needed to get it working. ouch :( */
    udelay(5);
    ms_writeb(status & 0xFF, OTG0_EP_FIFO_ACCESS_L(0));
    ms_writeb(0, OTG0_EP_FIFO_ACCESS_L(0));

    return 0;
}

static unsigned msb250x_ep0_read_ctl_req(struct usb_ctrlrequest* crq)
{
    u8* buf = (unsigned char*) crq;
    u8 bytes_read = 0;
    u16 count = 0;

    count = ms_readw(MSB250X_OTG0_EP_RXCOUNT_REG(0));

    if (count != sizeof(struct usb_ctrlrequest))
    {
        printf("<USB>[0] wanted %d bytes got %d. Stalling out...\n", sizeof(struct usb_ctrlrequest), count);
        msb250x_ep0_set_ss();
        return 0;
    }

    while (count > bytes_read)
    {
        *buf++ = ms_readb(OTG0_EP_FIFO_ACCESS_L(0));
        bytes_read++;
    }

    return count;
}

static void msb250x_ep0_handle_idle(struct msb250x_udc *dev)
{
    struct usb_ctrlrequest ctrl_req;

    int ret = 0;

    //msb250x_ep_request_nuke(dev, ep, -EPROTO);

    if (!msb250x_ep0_read_ctl_req(&ctrl_req))
    {
        return;
    }

    debug("<USB>[0][SETUP][%s][%d] bRequestType/bRequest/wValue/wIndex/wlength(0x%02x/0x%02x/0x%04x/0x%04x/0x%04x)\r\n",
           (ctrl_req.bRequestType & USB_DIR_IN)? "IN" : "OUT",
           (ctrl_req.bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD,
           ctrl_req.bRequestType,
           ctrl_req.bRequest,
           le16_to_cpu(ctrl_req.wValue),
           le16_to_cpu(ctrl_req.wIndex),
           le16_to_cpu(ctrl_req.wLength));

    /* cope with automagic for some standard requests. */
    dev->req_std = (ctrl_req.bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD;
    dev->req_config = 0;
    dev->req_pending = 1;

    if (dev->req_std)
    {
        switch (ctrl_req.bRequest)
        {
            case USB_REQ_SET_CONFIGURATION:
                //printf("USB_REQ_SET_CONFIGURATION ... \n");
                if (ctrl_req.bRequestType == USB_RECIP_DEVICE)
                {
                    dev->delay_status++;
                }
                break;

            case USB_REQ_SET_INTERFACE:
                //printf("USB_REQ_SET_INTERFACE ... \n");
                if (ctrl_req.bRequestType == USB_RECIP_INTERFACE)
                {
                    dev->delay_status++;
                }
                break;

            case USB_REQ_SET_ADDRESS:
                //printf("USB_REQ_SET_ADDRESS ... \n");
                if (ctrl_req.bRequestType == USB_RECIP_DEVICE)
                {
                    u8 address = 0;

                    address = ctrl_req.wValue & 0x7F;
                    dev->address = address;
                    ms_writeb(address, MSB250X_OTG0_FADDR_REG);

                    msb250x_ep0_clear_opr();
                    return;
                }
                break;

            case USB_REQ_GET_STATUS:
                //printf("USB_REQ_GET_STATUS ... \n");
                msb250x_ep0_clear_opr();

                if (0 == msb250x_udc_get_status(dev, &ctrl_req))
                {
                    msb250x_ep0_set_de_in();
                    return;
                }
                break;

            case USB_REQ_CLEAR_FEATURE:
                printf("USB_REQ_CLEAR_FEATURE ... \n");
                if (ctrl_req.bRequestType != USB_RECIP_ENDPOINT)
                    break;

                if (ctrl_req.wValue != USB_ENDPOINT_HALT || ctrl_req.wLength != 0)
                    break;

                msb250x_ep_set_halt(&dev->ep[ctrl_req.wIndex & 0x7f].ep, 0);
                msb250x_ep0_clear_opr();
                return;

            case USB_REQ_SET_FEATURE:
                //printf("USB_REQ_SET_FEATURE ... \n");
                msb250x_ep0_clear_opr();

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
        ret = dev->driver->setup(&dev->gadget, &ctrl_req);
    }
    else
        ret = -EOPNOTSUPP;

    if (ret < 0)
    {
        if (0 < dev->delay_status)
        {
            printf("<USB> config change %02x fail %d?\n",
                ctrl_req.bRequest, ret);
            return;
        }

        if (ret == -EOPNOTSUPP)
            printf("Operation not supported\n");
        else
            printf("dev->driver->setup failed. (%d)\n", ret);

        udelay(5);
        msb250x_ep0_set_ss();
        msb250x_ep0_set_de_out();

        dev->ep0state = EP0_IDLE;

        /* deferred i/o == no response yet */
    }
    else if (dev->req_pending)
    {
        dev->req_pending = 0;
    }

    //printf("<USB>[0] ep0state %s\n", ep0states[dev->ep0state]);
}

static void msb250x_ep0_isr_handler(struct msb250x_udc *dev)
{
    u8 ep0csr = 0;

    struct msb250x_ep* ep = &dev->ep[0];
    struct msb250x_request* req = NULL;

    ep0csr = ms_readb(MSB250X_OTG0_EP0_CSR0_REG);

    /* clear stall status */
    if (ep0csr & MSB250X_OTG0_CSR0_SENTSTALL)
    {
        printk("<USB>[0][EP] ... clear SENT_STALL ...\n");
        msb250x_request_nuke(ep, -EPIPE);
        msb250x_ep0_clear_sst();
        dev->ep0state = EP0_IDLE;
        return;
    }

    /* clear setup end */
    if (ep0csr & MSB250X_OTG0_CSR0_SETUPEND)
    {
        //printk("... serviced SETUP_END ...\n");
        msb250x_request_nuke(ep, 0);
        msb250x_ep0_clear_se();

        dev->ep0state = EP0_IDLE;
    }

    switch (dev->ep0state)
    {
        case EP0_IDLE:
            if (ep0csr & MSB250X_OTG0_CSR0_RXPKTRDY)
            {
                msb250x_ep0_handle_idle(dev);
            }
            break;

        case EP0_IN_DATA_PHASE:     /* GET_DESCRIPTOR etc */
        case EP0_OUT_DATA_PHASE:    /* SET_DESCRIPTOR etc */
            set_ep_halt(ep, 0);

            if (!list_empty(&ep->queue))
            {
                req = list_entry(ep->queue.next, struct msb250x_request, queue);
                msb250x_ep_handle_request(ep, req);
            }
            break;

        default:
            dev->ep0state = EP0_IDLE;
            printk("EP0 status ... what now?\n");
            break;
    }
}

static void msb250x_ep_isr_handler(struct msb250x_ep *ep)
{
    struct msb250x_request* req = NULL;

    struct otg0_ep_rxcsr_l* rxcsr_l = NULL;
    struct otg0_ep_txcsr_l* txcsr_l = NULL;

    u8 csr1 = 0;
    u8 ep_num = 0;

    ep_num = usb_endpoint_num(ep->desc);

    if (!list_empty(&ep->queue))
    {
        req = list_entry(ep->queue.next, struct msb250x_request, queue);
    }

    if (NULL != req)
    {
        if (usb_endpoint_dir_in(ep->desc))
        {
            csr1 = ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(ep_num));
            txcsr_l = (struct otg0_ep_txcsr_l*) &csr1;

            if (1 == txcsr_l->bSentStall)
            {
                txcsr_l->bSentStall = 0;
                ms_writeb(csr1, MSB250X_OTG0_EP_TXCSR1_REG(ep_num));
                return;
            }

        }
        else
        {
            u16 counts = 0;

            csr1 = ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(ep_num));
            rxcsr_l = (struct otg0_ep_rxcsr_l*) &csr1;

            debug("<USB>[INT_RX][%d] bRxPktRdy/bSentStall/fifo(%d/%d/0x%04x).\n",
                   ep_num,
                   rxcsr_l->bRxPktRdy,
                   rxcsr_l->bSentStall,
                   ms_readw(MSB250X_OTG0_EP_RXCOUNT_REG(ep_num)));

            if (1 == rxcsr_l->bSentStall)
            {
                rxcsr_l->bSentStall = 0;
                ms_writeb(csr1, MSB250X_OTG0_EP_RXCSR1_REG(ep_num));
                return;
            }
        #if 1
            if (usb_endpoint_xfer_bulk(ep->desc))
            {
                if (1 == rxcsr_l->bRxPktRdy)
                {
                    if (ep->ep.maxpacket < req->req.length)
                    {
                        counts = ms_readw(MSB250X_OTG0_EP_RXCOUNT_REG(ep_num));

                        if (ep->ep.maxpacket != counts || (ep->ep.maxpacket * msb250x_udc_get_pkt_cnt(ep->autoNAK_cfg)) == req->req.length)
                        {
                            msb250x_udc_ok2rcv_for_packets(ep->autoNAK_cfg, 0);

                            if (ep->ep.maxpacket != counts)
                            {
                                ep->shortPkt &= 1;
                            }
                        }
                    }
                }
            }
        #endif
        }

        msb250x_ep_handle_request(ep, req);
    }
}

static int msb250x_udc_ep_isr(struct msb250x_udc *dev)
{
    u8 ep_num = 0;

    u8 usb_intr_rx = 0;
    u8 usb_intr_tx = 0;

    usb_intr_rx = ms_readb(MSB250X_OTG0_INTRRX_REG);
    usb_intr_tx = ms_readb(MSB250X_OTG0_INTRTX_REG);
    ms_writeb(usb_intr_rx, MSB250X_OTG0_INTRRX_REG);
    ms_writeb(usb_intr_tx, MSB250X_OTG0_INTRTX_REG);

    if (0 < (usb_intr_rx | usb_intr_tx))
    {
        //printf("<USB>[INTR] intr_rx<0x%x> intr_tx<0x%x>\r\n", usb_intr_rx, usb_intr_tx);

        if (usb_intr_tx & MSB250X_OTG0_INTR_EP(0))
        {
            msb250x_ep0_isr_handler(dev);
        }

        for (ep_num = 1; MSB250X_MAX_ENDPOINTS > ep_num; ep_num++)
        {
            if ((usb_intr_rx | usb_intr_tx) & MSB250X_OTG0_INTR_EP(ep_num))
            {
                msb250x_ep_isr_handler(&dev->ep[ep_num]);
            }
        }
    }

    return IRQ_HANDLED;
}

static int msb250x_udc_isr(void)
{
    struct msb250x_udc *dev = &memory;

    msb250x_udc_link_isr(dev);
    msb250x_udc_ep_isr(dev);

    return IRQ_HANDLED;
}

static int msb250x_ep_enable(struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc)
{
    struct msb250x_udc* dev = NULL;
    struct msb250x_ep* ep = container_of(_ep, struct msb250x_ep, ep);

    u32 maxp = 0;
    u8 csr1 = 0,csr2 = 0;
    u8 power = 0;

    struct otg0_usb_power* pst_power = (struct otg0_usb_power*) &power;

    if (!_ep ||
        !desc ||
        ep->desc ||
        _ep->name == ep0name ||
        desc->bDescriptorType != USB_DT_ENDPOINT)
    {
        return -EINVAL;
    }

    dev = ep->dev;

    if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN)
    {
        return -ESHUTDOWN;
    }

    maxp = usb_endpoint_maxp(desc);

    _ep->maxpacket = maxp & 0x7ff;

    ep->desc = desc;
    set_ep_halt(ep, 0);

    /* set type, direction, address; reset fifo counters */
    if (usb_endpoint_dir_in(desc))
    {
        csr1 = MSB250X_OTG0_TXCSR1_FLUSHFIFO | MSB250X_OTG0_TXCSR1_CLRDATAOTG;
        csr2 = MSB250X_OTG0_TXCSR2_MODE;

        power = ms_readb(MSB250X_OTG0_PWR_REG);
        pst_power->bISOUpdate = 0;

        if (usb_endpoint_xfer_isoc(desc))
        {
            csr2 |= MSB250X_OTG0_TXCSR2_ISOC;
            pst_power->bISOUpdate = 1;
        }

        ms_writeb(power, MSB250X_OTG0_PWR_REG);

        ms_writew(maxp, MSB250X_OTG0_EP_TXMAP_REG(ep->ep_num));
        ms_writeb(csr1, MSB250X_OTG0_EP_TXCSR1_REG(ep->ep_num));
        ms_writeb(csr2, MSB250X_OTG0_EP_TXCSR2_REG(ep->ep_num));

        ep->fifo_size = 1 << (ms_readb(MSB250X_OTG0_EP_FIFOSIZE_REG(ep->ep_num)) & 0xf0 >> 4);
        //ep->fifo_size = ms_readb(MSB250X_UDC_FIFOSIZE_REG);
        /* enable irqs */
        ms_writeb((ms_readb(MSB250X_OTG0_INTRTX1E_REG) | MSB250X_OTG0_INTR_EP(ep->ep_num)), MSB250X_OTG0_INTRTX1E_REG);
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
            ep->autoNAK_cfg = msb250x_udc_get_autoNAK_cfg(ep->ep_num);
            msb250x_udc_enable_autoNAK(ep->ep_num, ep->autoNAK_cfg);
        }

        ms_writew(maxp, MSB250X_OTG0_EP_RXMAP_REG(ep->ep_num));
        ms_writeb(csr1, MSB250X_OTG0_EP_RXCSR1_REG(ep->ep_num));
        ms_writeb(csr2, MSB250X_OTG0_EP_RXCSR2_REG(ep->ep_num));

        ep->fifo_size = 1 << (ms_readb(MSB250X_OTG0_EP_FIFOSIZE_REG(ep->ep_num)) & 0x0f);

        /* enable irqs */
        ms_writeb((ms_readb(MSB250X_OTG0_INTRRX1E_REG) | MSB250X_OTG0_INTR_EP(ep->ep_num)), MSB250X_OTG0_INTRRX1E_REG);
    }

    printf("<USB>[%d][EN] %s %s with maxpacket/fifo(%d/%d)\r\n",
           usb_endpoint_num(desc),
           usb_endpoint_xfer_bulk(desc)? "bulk" : (usb_endpoint_xfer_isoc(desc)? "isoc" : "int"), usb_endpoint_dir_in(desc)? "in" : "out",
           _ep->maxpacket,
           ep->fifo_size);

    return 0;
}

static int msb250x_ep_disable(struct usb_ep *_ep)
{
    struct msb250x_ep* ep = NULL;

    u16 int_en_reg = 0;

    if (!_ep)
    {
        printf("%s not enabled\n",_ep ? ep->ep.name : NULL);
        return -EINVAL;
    }

    ep = container_of(_ep, struct msb250x_ep, ep);

    if (!ep->desc)
    {
        return -EINVAL;
    }

    set_ep_halt(ep, 1);

    /* disable irqs */
    if (usb_endpoint_dir_in(ep->desc))
    {
        int_en_reg = ms_readw(MSB250X_OTG0_INTRTXE_REG);
        ms_writew(int_en_reg & ~(MSB250X_OTG0_INTR_EP(ep->ep_num)), MSB250X_OTG0_INTRTXE_REG);
    }
    else
    {
        if (usb_endpoint_xfer_bulk(ep->desc))
        {
            msb250x_udc_disable_autoNAK_cfg(ep->autoNAK_cfg);
            ep->autoNAK_cfg = 0;
        }

        int_en_reg = ms_readw(MSB250X_OTG0_INTRRXE_REG);
        ms_writew(int_en_reg & ~(MSB250X_OTG0_INTR_EP(ep->ep_num)), MSB250X_OTG0_INTRRXE_REG);
    }

    msb250x_request_nuke(ep, -ESHUTDOWN);
    ep->desc = NULL;
    return 0;
}

static struct usb_request* msb250x_ep_alloc_request(struct usb_ep *ep, gfp_t gfp_flags)
{
	struct msb250x_request* req;

	req = kzalloc(sizeof(struct msb250x_request), gfp_flags);
	if (!req)
		return 0;

	memset(req, 0, sizeof *req);
	INIT_LIST_HEAD(&req->queue);

    debug("%s: %s %p req: %p\n", __func__, ep->name, ep, &req->req);
	return &req->req;
}

static void msb250x_ep_free_request(struct usb_ep *ep, struct usb_request *_req)
{
	struct msb250x_request *req;

	debug("%s: %p\n", __func__, ep);

	req = container_of(_req, struct msb250x_request, req);
	WARN_ON(!list_empty(&req->queue));
	kfree(req);
}


/** Queue one request
 *  Kickstart transfer if needed
 */
static int msb250x_ep_queue(struct usb_ep* _ep, struct usb_request* _req, gfp_t gfp_flags)
{
	struct msb250x_request *req = container_of(_req, struct msb250x_request, req);
	struct msb250x_ep *ep;
	struct msb250x_udc *dev;

	if (unlikely(!_req || !_req->complete || !_req->buf || !list_empty(&req->queue)))
    {

		debug("%s: bad params\n", __func__);
		return -EINVAL;
	}

	ep = container_of(_ep, struct msb250x_ep, ep);

	if (unlikely(!_ep || (!ep->desc && ep->ep.name != ep0name)))
    {

		debug("%s: bad ep: %s, %d, %p\n", __func__,
		      ep->ep.name, !ep->desc, _ep);
		return -EINVAL;
	}

	dev = ep->dev;

	if (unlikely(!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN))
    {

		debug("%s: bogus device state %p\n", __func__, dev->driver);
		return -ESHUTDOWN;
	}

	_req->status = -EINPROGRESS;
	_req->actual = 0;

	if (list_empty(&ep->queue))
    {
        req = msb250x_ep_handle_request(ep, req);
	}

	/* pio or dma irq handler advances the queue. */
	if (req != 0)
		list_add_tail(&req->queue, &ep->queue);

	return 0;
}

/* dequeue JUST ONE request */
static int msb250x_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct msb250x_ep *ep;
	struct msb250x_request *req;

	debug("%s: %p\n", __func__, _ep);

	ep = container_of(_ep, struct msb250x_ep, ep);

	if (!_ep || ep->ep.name == ep0name)
		return -EINVAL;

	/* make sure it's actually queued on this endpoint */
	list_for_each_entry(req, &ep->queue, queue)
	{
		if (&req->req == _req)
			break;
	}

	if (&req->req != _req)
    {
		return -EINVAL;
	}
	msb250x_request_done(ep, req, -ECONNRESET);

	return 0;
}

/*
 * Return bytes in EP FIFO
 */
static int msb250x_ep_fifo_status(struct usb_ep *_ep)
{
	int count = 0;
	struct msb250x_ep* ep = container_of(_ep, struct msb250x_ep, ep);

	if (!_ep)
    {
		debug("%s: bad ep\n", __func__);
		return -ENODEV;
	}

	/* LPD can't report unclaimed bytes from IN fifos */
	if (usb_endpoint_dir_in(ep->desc))
    {
		return -EOPNOTSUPP;
    }

	return count;
}

/*
 * Flush EP FIFO
 */
static void msb250x_ep_fifo_flush(struct usb_ep* _ep)
{
	struct msb250x_ep* ep = NULL;

	if (unlikely(!_ep || (!ep->desc && ep->ep.name != ep0name)))
    {
		debug("%s: bad ep\n", __func__);
		return;
	}

    ep = container_of(_ep, struct msb250x_ep, ep);

    if (0 == ep->ep_num)
    {
        ms_writeb(ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(0)) | MSB250X_OTG0_CSR0_FLUSHFIFO, MSB250X_OTG0_EP_TXCSR1_REG(0));
    }
    else
    {
        if (usb_endpoint_dir_in(ep->desc))
        {
            ms_writeb(ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(ep->ep_num)) | MSB250X_OTG0_TXCSR1_FLUSHFIFO, MSB250X_OTG0_EP_TXCSR1_REG(ep->ep_num));
        }
        else
        {
            ms_writeb(ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(ep->ep_num)) | MSB250X_OTG0_RXCSR1_FLUSHFIFO, MSB250X_OTG0_EP_TXCSR1_REG(ep->ep_num));
        }
    }
}


/* drivers may have software control over D+ pullup */
static int msb250x_usb_pullup(struct usb_gadget *_gadget, int is_active)
{
	msb250x_udc_pullup_i(is_active != 0);
	return 0;
}

int usb_gadget_handle_interrupts()
{
    return msb250x_udc_isr();
}

int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
    struct msb250x_udc *dev = &memory;

    int retval = 0;

    if (!driver ||
       (driver->speed != USB_SPEED_FULL && driver->speed != USB_SPEED_HIGH) ||
        !driver->bind ||
        !driver->disconnect ||
        !driver->setup)
    {
        return -EINVAL;
    }

	if (dev->driver)
    {
		return -EBUSY;
    }

    msb250x_udc_reinit(dev);

    msb250x_udc_init_MIU_sel();
    msb250x_udc_init_utmi();
    msb250x_reset_otg();
    msb250x_udc_init_otg();
    usb_gadget_disconnect(&dev->gadget);

    retval = driver->bind(&dev->gadget);

	if (retval)
    {
		dev->driver = 0;
		return retval;
	}

#if defined(GPIO_LED0_ENABLED)
    ms_writeb(ms_readb(GET_REG8_ADDR(REG_ADDR_BASE_PM_PWM, 0x4E)) & ~BIT3, GET_REG8_ADDR(REG_ADDR_BASE_PM_PWM, 0x4E));
    ms_writeb(ms_readb(GET_REG8_ADDR(REG_ADDR_BASE_PM_PWM, 0x51)) & ~(BIT4 | BIT5), GET_REG8_ADDR(REG_ADDR_BASE_PM_PWM, 0x51));
    ms_writeb(ms_readb(GET_REG8_ADDR(REG_ADDR_BASE_PM_PWM, 0x50)) & ~(BIT4 | BIT5), GET_REG8_ADDR(REG_ADDR_BASE_PM_PWM, 0x50));
    ms_writeb(ms_readb(GET_REG8_ADDR(REG_ADDR_BASE_PM_GPIO, 0x94)) & ~(BIT0 | BIT1), GET_REG8_ADDR(REG_ADDR_BASE_PM_GPIO, 0x94));
#endif
    dev->driver = driver;
    printf("registered gadget driver '%s'\n", driver_name);

    return 0;
}

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
    struct msb250x_udc *dev = &memory;

	if (!dev)
		return -ENODEV;
	if (!driver || driver != dev->driver)
		return -EINVAL;

	msb250x_udc_disable(dev);
    driver->disconnect(&dev->gadget);
	driver->unbind(&dev->gadget);
    dev->driver = 0;

    printf("unregistered gadget driver '%s'\n", driver_name);
	return 0;
}

