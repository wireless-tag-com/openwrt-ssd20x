/*
* msb250x_udc.c- Sigmastar
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
* msb250x_udc.c- Sigmastar
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


/******************************************************************************
 * Include Files
 ******************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
//#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/clk.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>

#include <linux/usb.h>
#include <linux/usb/gadget.h>
#include <linux/usb/otg.h> // Mstar OTG operation
#include <linux/cdev.h>
#include <asm/uaccess.h>        /* copy_*_user */

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
//#include <asm/system.h>
#include <asm/unaligned.h>
//#include <mach/irqs.h>

//#include <mach/hardware.h>
#if defined(CONFIG_ARM64)
#else
#include <asm/mach-types.h>
#endif

#if 0
#include "padmux.h"
#include "mdrv_padmux.h"
#include "mdrv_gpio.h"
#include "mdrv_gpio_io.h"
#endif

#include <linux/mm.h>
#include <linux/dma-mapping.h>

#include "msb250x_udc_reg.h"
#include "msb250x_udc.h"
#include "msb250x_gadget.h"
#include "msb250x_ep.h"
#include "msb250x_dma.h"

//#define REG_ADDR_BASE_PM_GPIO         GET_REG8_ADDR(RIU_BASE_ADDR, 0x0F00)

/* the module parameter */
#define DRIVER_DESC "MSB250x USB Device Controller Gadget"
#define DRIVER_VERSION "5 June 2019"
#define DRIVER_AUTHOR "sigmastar.com"

#define AUTONAK_COUNT   2
#define MAX_ETHERNET_PACKET_SIZE    1518

/******************************************************************************
 * Variables
 ******************************************************************************/
const char ep0name[] = "ep0";

static const char sg_gadget_name[] = "msb250x_udc";

static const struct usb_ep_ops msb250x_ep_ops =
{
    .enable = msb250x_ep_enable,
    .disable = msb250x_ep_disable,

    .alloc_request = msb250x_ep_alloc_request,
    .free_request = msb250x_ep_free_request,

    .queue = msb250x_ep_queue,
    .dequeue = msb250x_ep_dequeue,

    .set_halt = msb250x_ep_set_halt,
};

static const struct usb_gadget_ops msb250x_gadget_ops =
{
    .get_frame = msb250x_gadget_get_frame,
    .wakeup = msb250x_gadget_wakeup,
    .set_selfpowered = msb250x_gadget_set_selfpowered,
    .pullup = msb250x_gadget_pullup,
    //.vbus_session = msb250x_gadget_vbus_session,
    //.vbus_draw    = msb250x_gadget_vbus_draw,
    .udc_start = msb250x_gadget_udc_start,
    .match_ep	= msb250x_gadget_match_ep,
    .udc_stop = msb250x_gadget_udc_stop,
};

static struct msb250x_udc memory = {
    .gadget = {
        .ep0 = &memory.ep[0].ep,
        .ops = &msb250x_gadget_ops,
        .max_speed = USB_SPEED_HIGH,
        .name = sg_gadget_name,
        .dev = {
            //.bus_id = "gadget",
            .init_name = "gadget",
            //.release	= nop_release,
        },
    },
    MSB250X_EPS_CAP(&memory, &msb250x_ep_ops),
};

extern unsigned int irq_of_parse_and_map(struct device_node *node, int index);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_udc_write_riu
+------------------------------------------------------------------------------
| DESCRIPTION : write the usb request info
|
| RETURN      : length
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| ep                      |x  |      | msb250x_ep struct point
|--------------------+---+---+-------------------------------------------------
| req                 |x  |      | msb250x_request struct point
+--------------------+---+---+-------------------------------------------------
*/
static int msb250x_udc_write_riu(struct msb250x_ep *ep,
                                 struct msb250x_request *req,
                                 u32 bytes)
{
    u8 ep_num = 0;

    int is_last = 0;

    struct usb_ep* _ep = &ep->ep;

    ep_num = IS_ERR_OR_NULL(_ep->desc)? 0 : usb_endpoint_num(_ep->desc);

    if (0 < bytes)
    {
        ms_writesb((req->req.buf + req->req.actual), OTG0_EP_FIFO_ACCESS_L(ep_num), bytes);
    }

    req->req.actual += bytes;

    /* last packet is often short (sometimes a zlp) */
    if (bytes != ep->ep.maxpacket || req->req.length == req->req.actual)
    {
        is_last = 1;
    }

#if 0
    if (!IS_ERR_OR_NULL(_ep->desc))
    printk(KERN_DEBUG "<USB>[UDC][%d][TX] maxpacket/bytesDone(0x%04x/0x%04x) buff/last/actual/length(%p/%d/0x%04x/0x%04x) \n",
                      ep_num,
                      ep->ep.maxpacket,
                      count,
                      req->req.buf,
                      is_last,
                      req->req.actual,
                      req->req.length);
#endif
#if 1
    if (is_last)
    {
        msb250x_request_done(ep, req, 0);
    }
#endif
    return is_last;
}

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_udc_read_riu
+------------------------------------------------------------------------------
| DESCRIPTION : read the usb request info
|
| RETURN      : length
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| ep                      |x  |      | msb250x_ep struct point
|--------------------+---+---+-------------------------------------------------
| req                 |x  |      | msb250x_request struct point
+--------------------+---+---+-------------------------------------------------
*/
static int msb250x_udc_read_riu(struct msb250x_ep *ep,
                                 struct msb250x_request *req,
                                 u32 bytes)
{
    u8 ep_num = 0;
    int is_last = 0;

    struct usb_ep* _ep = &ep->ep;
    //struct otg0_ep_rxcsr_l* rxcsr_l = NULL;

    if (0 == req->req.length)
    {
        return 1;
    }

    ep_num = IS_ERR_OR_NULL(_ep->desc)? 0 : usb_endpoint_num(_ep->desc);

    if (0 < bytes)
    {
        ms_readsb((req->req.buf + req->req.actual), OTG0_EP_FIFO_ACCESS_L(ep_num), bytes);

        req->req.actual += bytes;

        if (bytes != ep->ep.maxpacket || req->req.zero)
        {
            is_last = 1;
        }
    }

    if (req->req.length == req->req.actual)
    {
        is_last = 1;
    }

#if 0
    if (!IS_ERR_OR_NULL(_ep->desc))
    {
        printk(KERN_DEBUG "<USB>[UDC][%d] RX[RIU] count/maxpacket(0x%04x/0x%04x) buff/last/actual/length(%p/%d/0x%04x/0x%04x) \n",
                          ep_num,
                          count,
                          ep->ep.maxpacket,
                          req->req.buf,
                          is_last,
                          req->req.actual,
                          req->req.length);
    }
#endif

    if (is_last)
    {
        msb250x_request_done(ep, req, 0);
    }

    return is_last;
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
EXPORT_SYMBOL(msb250x_udc_get_autoNAK_cfg);

int msb250x_udc_release_autoNAK_cfg(u8 cfg)
{
    cfg--;

    switch(cfg)
    {
        case 0:
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK0_EP_BULKOUT) & 0xf0), MSB250X_OTG0_AUTONAK0_EP_BULKOUT);
            ms_writew(0x0000, MSB250X_OTG0_USB_CFG5_L);
        #if 0
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK0_CTRL) & ~(MSB250X_OTG0_AUTONAK0_EN | MSB250X_OTG0_AUTONAK0_OK2Rcv | MSB250X_OTG0_AUTONAK0_AllowAck)),
                      MSB250X_OTG0_AUTONAK0_CTRL);
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK0_RX_PKT_CNT) & 0xE000), MSB250X_OTG0_AUTONAK0_RX_PKT_CNT);
        #endif
            break;
        case 1:
            ms_writeb(ms_readb(MSB250X_OTG0_USB_CFG0_H) & 0x80, MSB250X_OTG0_USB_CFG0_H);
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK1_RX_PKT_CNT) & 0xE000), MSB250X_OTG0_AUTONAK1_RX_PKT_CNT);
        #if 0
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK1_EP_BULKOUT) & 0xf0), MSB250X_OTG0_AUTONAK1_EP_BULKOUT);
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK1_CTRL) & ~(MSB250X_OTG0_AUTONAK1_EN | MSB250X_OTG0_AUTONAK1_OK2Rcv | MSB250X_OTG0_AUTONAK1_AllowAck)),
                      MSB250X_OTG0_AUTONAK1_CTRL);
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK1_RX_PKT_CNT) & 0xE000), MSB250X_OTG0_AUTONAK1_RX_PKT_CNT);
        #endif
            break;
        case 2:
            ms_writeb((ms_readb(MSB250X_OTG0_AUTONAK2_EP_BULKOUT) & 0x0f), MSB250X_OTG0_AUTONAK2_EP_BULKOUT);
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK2_CTRL) & ~(MSB250X_OTG0_AUTONAK2_EN | MSB250X_OTG0_AUTONAK2_OK2Rcv | MSB250X_OTG0_AUTONAK2_AllowAck)), MSB250X_OTG0_AUTONAK2_CTRL);
            ms_writew((ms_readw(MSB250X_OTG0_AUTONAK2_RX_PKT_CNT) & 0xE000), MSB250X_OTG0_AUTONAK2_RX_PKT_CNT);
            break;
    }

    return 0;
}
EXPORT_SYMBOL(msb250x_udc_release_autoNAK_cfg);

int msb250x_udc_init_autoNAK_cfg(void)
{
    int cfg = 0;

    for (cfg = 0; cfg < AUTONAK_COUNT; cfg++)
    {
        msb250x_udc_release_autoNAK_cfg((cfg + 1));
    }

    return 0;
}
EXPORT_SYMBOL(msb250x_udc_init_autoNAK_cfg);

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

    //printk(KERN_DEBUG "<USB>[UDC][NAK][%d] enable autoNAK!\n", cfg);
}
EXPORT_SYMBOL(msb250x_udc_enable_autoNAK);

void msb250x_udc_ok2rcv_for_packets(u8 cfg, u16 pkt_num)
{
    u16 ctrl = 0;
    cfg--;

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

    //printk(KERN_DEBUG "<USB>[UDC][NAK][%d] ok2rcv %d packets\n", cfg, pkt_num);
}
EXPORT_SYMBOL(msb250x_udc_ok2rcv_for_packets);

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
    //printk(KERN_DEBUG "<USB>[UDC][NAK][%d] allowAck\n", cfg);
}
EXPORT_SYMBOL(msb250x_udc_allowAck);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_request_handler
+------------------------------------------------------------------------------
| DESCRIPTION : dispatch request to use DMA or FIFO
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| _ep                | x |   | usb_ep struct point
|--------------------+---+---+-------------------------------------------------
| _req               | x |   | usb_request struct point
+--------------------+---+---+-------------------------------------------------
*/

struct msb250x_request *
msb250x_request_handler(struct msb250x_ep* ep,
                        struct msb250x_request* req)
{
    u32 bytes;
    u16 pkt_num = 0;

    u8 ep_num = 0;
    u8 csr = 0;
    u8 intrusb = 0;
    u8 is_last = 0;

    struct msb250x_udc *dev = ep->dev;
    struct usb_ep* _ep = &ep->ep;
    struct otg0_ep0_csr_l* csr_l = NULL;
    struct otg0_ep_rxcsr_l* rxcsr_l = NULL;
    struct otg0_ep_txcsr_l* txcsr_l = NULL;
    struct otg0_usb_intr* st_intrusb = (struct otg0_usb_intr*) &intrusb;

    if (0 == ep->halted)
    {
        msb250x_set_ep_halt(ep, 1);

        intrusb = ms_readb(MSB250X_OTG0_INTRUSB_REG);

        if (1 == st_intrusb->bReset)
        {
            return req;
        }

        bytes = req->req.length - req->req.actual;

        if (IS_ERR_OR_NULL(_ep->desc))
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
                        bytes = min((u16)_ep->maxpacket, (u16)bytes);

                        is_last = msb250x_udc_write_riu(ep, req, bytes);

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
                        u16 counts = ms_readw(MSB250X_OTG0_EP_RXCOUNT_L_REG(ep_num));

                        if (bytes < counts)
                        {
                            printk(KERN_WARNING "<USB_WARN>[UDC][%d] Bytes in fifo(0x%04x) is more than buffer size. buf/actual/length(0x%p/0x%04x/0x%04x)!\n",
                                                ep_num,
                                                counts,
                                                req->req.buf,
                                                req->req.actual,
                                                req->req.length);
                        }

                        bytes = min((u16)counts, (u16)bytes);

                        is_last = msb250x_udc_read_riu(ep, req, bytes);

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
                    printk(KERN_ERR "<USB_ERR> EP0 Request Error !!\n");
            }

            msb250x_set_ep_halt(ep, 0);
        }
        else
        {
            ep_num = usb_endpoint_num(_ep->desc);

            if (!usb_endpoint_xfer_bulk(_ep->desc))
            {
                bytes = min((u16)(usb_endpoint_maxp_mult(_ep->desc) * _ep->maxpacket), (u16)bytes);
            }

            if (usb_endpoint_dir_in(_ep->desc))
            {
                txcsr_l = (struct otg0_ep_txcsr_l*) &csr;
                csr = ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(ep_num));

                //if (0 == (txcsr_l->bTxPktRdy | txcsr_l->bFIFONotEmpty))
                if (0 == txcsr_l->bTxPktRdy)
                {
                    if (0 == dev->using_dma || 0 != msb250x_dma_setup_control(_ep, req, bytes))
                    {
                        is_last = msb250x_udc_write_riu(ep, req, bytes);
                        ep_set_ipr(ep_num);
                    }
                }
            }
            else if (usb_endpoint_dir_out(_ep->desc))
            {
                csr = ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(ep_num));
                rxcsr_l = (struct otg0_ep_rxcsr_l*) &csr;

                if (usb_endpoint_xfer_bulk(_ep->desc))
                {
                    if (0 == rxcsr_l->bRxPktRdy)
                    {
                        pkt_num = (bytes + (_ep->maxpacket - 1)) / _ep->maxpacket;

                        if (1 == ep->shortPkt)
                        {
                            udelay(125);
                        }

                    #if defined(CONFIG_USB_AVOID_SHORT_PACKET_IN_BULK_OUT_WITH_DMA_FOR_ETHERNET)
                        if (MAX_ETHERNET_PACKET_SIZE < bytes)
                        {
                            pkt_num = (MAX_ETHERNET_PACKET_SIZE + (_ep->maxpacket - 1)) / _ep->maxpacket;
                        }
                        else
                        {
                            pkt_num = 1;
                        }
                    #endif

                        //if (ep->ep.maxpacket >= req->req.length)
                        if (1 == pkt_num)
                        {
                            msb250x_udc_allowAck(ep->autoNAK_cfg);
                            ep->shortPkt = 0;
                        }
                        else
                        {
                            bytes = min((u16)(pkt_num * _ep->maxpacket), (u16)bytes);

                            msb250x_dma_setup_control(&ep->ep, req, bytes);
                            msb250x_udc_ok2rcv_for_packets(ep->autoNAK_cfg, pkt_num);
                            ep->shortPkt = 1;
                        }
                    }
                }

                if (1 == rxcsr_l->bRxPktRdy)
                {
                    bytes = min((u16)bytes, (u16)ms_readw(MSB250X_OTG0_EP_RXCOUNT_L_REG(ep_num)));

                    if (0 == dev->using_dma || 0 != msb250x_dma_setup_control(&ep->ep, req, bytes))
                    {
                        is_last = msb250x_udc_read_riu(ep, req, bytes);
                        ep_set_opr(ep_num);
                    }
                }
            }
        }
    }

    if (1 == is_last)
    {
        req = NULL;
    }

    return req;
}
EXPORT_SYMBOL(msb250x_request_handler);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_request_nuke
+------------------------------------------------------------------------------
| DESCRIPTION : dequeue ALL requests
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| ep                        | x  |      | msb250x_ep struct point
|--------------------+---+---+-------------------------------------------------
| req                       | x  |      | msb250x_request struct point
|--------------------+---+---+-------------------------------------------------
| status                   | x  |      | reports completion code, zero or a negative errno
+--------------------+---+---+-------------------------------------------------
*/
void msb250x_request_nuke(struct msb250x_udc *udc,
                          struct msb250x_ep *ep, int status)
{
    /* Sanity check */
    if (&ep->queue == NULL)
        return;

    while (!list_empty (&ep->queue))
    {
        struct msb250x_request *req;
        req = list_entry (ep->queue.next, struct msb250x_request, queue);
        msb250x_request_done(ep, req, status);
    }
}
EXPORT_SYMBOL(msb250x_request_nuke);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_request_continue
+------------------------------------------------------------------------------
| DESCRIPTION : Schedule the next packet for this EP
|
| RETURN      :
+------------------------------------------------------------------------------
| Variable Name  |IN |OUT|                   Usage
|----------------+---+---+-----------------------------------------------------
| ep             | x |   | msb250x_ep struct point
+-------------- -+---+---+-----------------------------------------------------
*/
void msb250x_request_continue(struct msb250x_ep *ep)
{

    //printk("msb250x_request_continue\n");

    struct msb250x_request *req = NULL;

    if (likely(!list_empty(&ep->queue)))
    {
        req = list_entry(ep->queue.next, struct msb250x_request, queue);
        msb250x_request_handler(ep, req);
    }
}

EXPORT_SYMBOL(msb250x_request_continue);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_request_done
+------------------------------------------------------------------------------
| DESCRIPTION : complete the usb request
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| ep                        | x  |      | msb250x_ep struct point
|--------------------+---+---+-------------------------------------------------
| req                       | x  |      | msb250x_request struct point
|--------------------+---+---+-------------------------------------------------
| status                   | x  |      | reports completion code, zero or a negative errno
+--------------------+---+---+-------------------------------------------------
*/

void msb250x_request_done(struct msb250x_ep *ep,
                      struct msb250x_request *req, int status)
{
    struct msb250x_udc* dev = NULL;

    unsigned halted = ep->halted;

    dev = ep->dev;

    if(req==NULL)
    {
        printk("<USB> EP[%d] REQ NULL\n", IS_ERR_OR_NULL(ep->ep.desc)? 0 : usb_endpoint_num(ep->ep.desc));
        return;
    }

    if (ep0name != ep->ep.name && dev->using_dma)
    {
        msb250x_gadget_unmap_request(ep->gadget, &req->req, usb_endpoint_dir_in(ep->ep.desc));
    }

    list_del_init(&req->queue);

    if (likely (req->req.status == -EINPROGRESS))
        req->req.status = status;
    else
        status = req->req.status;

    msb250x_set_ep_halt(ep, 1);
    spin_unlock(&dev->lock);
    usb_gadget_giveback_request(&ep->ep, &req->req);
    spin_lock(&dev->lock);
    msb250x_set_ep_halt(ep, halted);
}

EXPORT_SYMBOL(msb250x_request_done);

static irqreturn_t msb250x_udc_link_isr(struct msb250x_udc *dev)
{
    u8 usb_status = 0;
    u8 power = 0;

    struct otg0_usb_power* pst_power = (struct otg0_usb_power*) &power;
    struct otg0_usb_intr* usb_st = (struct otg0_usb_intr*) &usb_status;

    usb_status = ms_readb(MSB250X_OTG0_INTRUSB_REG);
    ms_writeb(usb_status, MSB250X_OTG0_INTRUSB_REG);

    power = ms_readb(MSB250X_OTG0_PWR_REG);

    /* RESET */
    //if (usb_status & MSB250X_OTG0_INTRUSB_RESET)

    if (1 == usb_st->bReset)
    {
         /* two kind of reset :
         * - reset start -> pwr reg = 8
         * - reset end   -> pwr reg = 0
         **/
        printk(KERN_INFO "<USB>[LINK] Bus reset.\r\n");
    #ifndef CONFIG_USB_FPGA_VERIFICATION
        ms_writeb(0x10, GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16));        //TX-current adjust to 105%=> bit <4> set 1
        ms_writeb(0x02, (GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16) + 1));  // Pre-emphasis enable=> bit <1> set 1
        //ms_writeb(0x01, (GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17) + 1));  //HS_TX common mode current enable (100mV)=> bit <7> set 1
    #endif

        if (dev->driver && dev->driver->disconnect && USB_STATE_DEFAULT <= dev->gadget.state)
        {
            spin_unlock(&dev->lock);
            dev->driver->disconnect(&dev->gadget);
            spin_lock(&dev->lock);
        }

        //msb250x_udc_init_otg(dev);
        ms_writeb(0, MSB250X_OTG0_FADDR_REG);

        dev->address = 0;
        dev->ep0state = EP0_IDLE;

        //if (ms_readb(MSB250X_OTG0_PWR_REG)&MSB250X_OTG0_PWR_HS_MODE)
        if (1 == pst_power->bHSMode)
        {
            dev->gadget.speed = USB_SPEED_HIGH;
            printk(KERN_INFO "<USB>[LINK] High speed device\r\n");
        #ifndef CONFIG_USB_FPGA_VERIFICATION
            ms_writew(0x0230, GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16));  //B2 analog parameter
        #endif
        }
        else
        {
            dev->gadget.speed = USB_SPEED_FULL;
            printk(KERN_INFO "<USB>[LINK] Full speed device\r\n");
        #ifndef CONFIG_USB_FPGA_VERIFICATION
            ms_writew(0x0030, GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16));  //B2 analog parameter
        #endif
        }

        msb250x_udc_init_autoNAK_cfg();

        usb_gadget_set_state(&dev->gadget, USB_STATE_DEFAULT);
    }

    /* RESUME */
    //if (usb_status & MSB250X_OTG0_INTRUSB_RESUME)
    if (1 == usb_st->bResume)
    {
        printk(KERN_INFO "<USB>[LINK] Resume\r\n");

        if (dev->gadget.speed != USB_SPEED_UNKNOWN &&
            dev->driver &&
            dev->driver->resume)
         {
             spin_unlock(&dev->lock);
             dev->driver->resume(&dev->gadget);
             spin_lock(&dev->lock);
         }
    }

    /* SUSPEND */
    //if (usb_status & MSB250X_OTG0_INTRUSB_SUSPEND)
    if (1 == usb_st->bSuspend)
    {
        printk(KERN_INFO "<USB>[LINK] Suspend\r\n");

        //if (1 == pst_power->bSuspendMode)
        {
            if (dev->gadget.speed != USB_SPEED_UNKNOWN &&
                dev->driver &&
                dev->driver->suspend)
            {
                printk("call gadget->suspend\n");
                spin_unlock(&dev->lock);
                dev->driver->suspend(&dev->gadget);
                spin_lock(&dev->lock);

                dev->ep0state = EP0_IDLE;

                usb_gadget_set_state(&dev->gadget, USB_STATE_SUSPENDED);
            }


        }
    }

    return IRQ_HANDLED;
}

static irqreturn_t msb250x_udc_ep_isr(struct msb250x_udc *dev)
{
    u8 ep_num = 0;

    u8 usb_intr_rx = 0, usb_intr_tx = 0;

    usb_intr_rx = ms_readb(MSB250X_OTG0_INTRRX_REG);
    usb_intr_tx = ms_readb(MSB250X_OTG0_INTRTX_REG);
    ms_writeb(usb_intr_rx, MSB250X_OTG0_INTRRX_REG);
    ms_writeb(usb_intr_tx, MSB250X_OTG0_INTRTX_REG);

    if (usb_intr_tx & MSB250X_OTG0_INTR_EP(0))
    {
        msb250x_ep0_isr_handler(dev);
    }

    for (ep_num = 1; MSB250X_MAX_ENDPOINTS > ep_num; ep_num++)
    {
        if ((usb_intr_rx | usb_intr_tx) & MSB250X_OTG0_INTR_EP(ep_num))
        {
            msb250x_ep_isr_handler(dev, &dev->ep[ep_num]);
        }
    }

    return IRQ_HANDLED;
}

static irqreturn_t msb250x_udc_dma_isr(struct msb250x_udc *dev)
{
    u8 dma_ch = 0;
    u8 dma_intr;

    dma_intr = ms_readb(MSB250X_OTG0_DMA_INTR);
    ms_writeb(dma_intr, MSB250X_OTG0_DMA_INTR);

    if (dma_intr)
    {
        for (dma_ch = 0; dma_ch < MSB250X_USB_DMA_CHANNEL; dma_ch++)
        {
            if (dma_intr & (1 << dma_ch))
            {
                msb250x_dma_isr_handler((dma_ch + 1), dev);
            }
        }
    }

    return IRQ_HANDLED;
}

static irqreturn_t msb250x_udc_isr(int irq, void *_dev)
{
    struct msb250x_udc *dev = _dev;

    unsigned long flags;

    spin_lock_irqsave(&dev->lock, flags);
/*
    ms_writeb(ms_readb(GET_REG8_ADDR(REG_ADDR_BASE_PM_GPIO, 0X94)) & ~BIT1, GET_REG8_ADDR(REG_ADDR_BASE_PM_GPIO, 0X94));
    ms_writeb(ms_readb(GET_REG8_ADDR(REG_ADDR_BASE_PM_GPIO, 0X94)) | BIT1, GET_REG8_ADDR(REG_ADDR_BASE_PM_GPIO, 0X94));
*/
    msb250x_udc_dma_isr(dev);
    msb250x_udc_link_isr(dev);
    msb250x_udc_ep_isr(dev);
    //ms_writeb((ms_readb(GET_REG8_ADDR(REG_ADDR_BASE_PM_GPIO, 0X94)) & ~BIT1), GET_REG8_ADDR(REG_ADDR_BASE_PM_GPIO, 0X94));
    spin_unlock_irqrestore(&dev->lock, flags);

    return IRQ_HANDLED;
}

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_udc_get_status
+------------------------------------------------------------------------------
| DESCRIPTION :get the USB device status
|
| RETURN      :0 when success, error code in other case.
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| dev                      |x  |      | msb250x_udc struct point
|--------------------+---+---+-------------------------------------------------
| crq                        |x  |      | usb_ctrlrequest struct point
+--------------------+---+---+-------------------------------------------------
*/
int msb250x_udc_get_status(struct msb250x_udc *dev,
                           struct usb_ctrlrequest *crq)
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
                status = ms_readb(MSB250X_OTG0_EP0_CSR0_REG);
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
EXPORT_SYMBOL(msb250x_udc_get_status);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_udc_disable
+------------------------------------------------------------------------------
| DESCRIPTION : disable udc
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| dev                 |x  |       | msb250x_udc struct point
|--------------------+---+---+-------------------------------------------------
*/
void msb250x_udc_disable(struct msb250x_udc *dev)
{
    /* Disable all interrupts */
    ms_writeb(0x00, MSB250X_OTG0_INTRUSBE_REG);
    ms_writeb(0x00, MSB250X_OTG0_INTRTXE_REG);
    ms_writeb(0x00, MSB250X_OTG0_INTRRXE_REG);

    /* Clear the interrupt registers */
    /* All active interrupts will be cleared when this register is read */
    ms_writeb(ms_readb(MSB250X_OTG0_INTRUSB_REG), MSB250X_OTG0_INTRUSB_REG);
    ms_writeb(ms_readb(MSB250X_OTG0_INTRTX_REG), MSB250X_OTG0_INTRTX_REG);
    ms_writeb(ms_readb(MSB250X_OTG0_INTRRX_REG), MSB250X_OTG0_INTRRX_REG);

    /* Good bye, cruel world */
    msb250x_gadget_pullup_i(0);

    /* Set speed to unknown */
    dev->gadget.speed = USB_SPEED_UNKNOWN;

}

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_udc_reinit
+------------------------------------------------------------------------------
| DESCRIPTION : reinit the ep list
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| dev                 |x  |       | msb250x_udc struct point
|--------------------+---+---+-------------------------------------------------
*/
static void msb250x_udc_reinit(struct msb250x_udc *dev)
{
    u8 ep_num = 0;

    /* device/ep0 records init */
    INIT_LIST_HEAD (&dev->gadget.ep_list);
    INIT_LIST_HEAD (&dev->gadget.ep0->ep_list);

    dev->gadget.speed = USB_SPEED_UNKNOWN;

    for (ep_num = 0; ep_num < MSB250X_MAX_ENDPOINTS; ep_num++)
    {
        struct msb250x_ep *ep = &dev->ep[ep_num];

        list_add_tail (&ep->ep.ep_list, &dev->gadget.ep_list);
        //ep->dev = dev;
        INIT_LIST_HEAD (&ep->queue);

        usb_ep_set_maxpacket_limit(&ep->ep, 1024);
        ep->gadget = &dev->gadget;
    }

    usb_ep_set_maxpacket_limit(&dev->ep[0].ep, 64);
    dev->gadget.ep0 = &dev->ep[0].ep;
    list_del_init(&dev->ep[0].ep.ep_list);
}

void msb250x_udc_init_usb_ctrl(void)
{
    u8 ctrl_l = 0;
    struct usbc0_rst_ctrl_l* pst_usbc0_rst_ctrl_l = (struct usbc0_rst_ctrl_l*) &ctrl_l;

    // Reset OTG controllers
    //USBC_REG_WRITE8(0, 0xC);
    //USBC_REG_WRITE8(0, USBC_REG_READ8(0)|(OTG_BIT3|OTG_BIT2));
    ctrl_l = ms_readb(GET_REG16_ADDR(USBC_BASE_ADDR, 0));
    pst_usbc0_rst_ctrl_l->bOTG_RST = 1;
    pst_usbc0_rst_ctrl_l->bREG_SUSPEND = 1;
    ms_writeb(ctrl_l, GET_REG16_ADDR(USBC_BASE_ADDR, 0));

    // Unlock Register R/W functions  (RST_CTRL[6] = 1)
    // Enter suspend  (RST_CTRL[3] = 1)
    //USBC_REG_WRITE8(0, 0x48);
    //USBC_REG_WRITE8(0, (USBC_REG_READ8(0)&~(OTG_BIT2))|OTG_BIT6);
    ctrl_l = ms_readb(GET_REG16_ADDR(USBC_BASE_ADDR, 0));
    pst_usbc0_rst_ctrl_l->bOTG_RST = 0;
    pst_usbc0_rst_ctrl_l->bOTG_XIU_ENABLE = 1;
    ms_writeb(ctrl_l, GET_REG16_ADDR(USBC_BASE_ADDR, 0));

    printk(KERN_DEBUG "<USB>[GADGET] Init USB controller\n");
}

EXPORT_SYMBOL(msb250x_udc_init_usb_ctrl);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_udc_init_otg
+------------------------------------------------------------------------------
| DESCRIPTION : enable udc
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| dev                 |x  |       | msb250x_udc struct point
|--------------------+---+---+-------------------------------------------------
*/

#if defined(ENABLE_OTG_USB_NEW_MIU_SLE)
static void msb250x_udc_init_MIU_sel(void)
{
    printk("<USB>[miu] [%x] [%x] [%x] [%x].\n", USB_MIU_SEL0, USB_MIU_SEL1, USB_MIU_SEL2, USB_MIU_SEL3);
    ms_writeb(USB_MIU_SEL0, GET_REG16_ADDR(USBC_BASE_ADDR, 0x0A));	                                            //Setting MIU0 segment
    ms_writeb(USB_MIU_SEL1, GET_REG16_ADDR(USBC_BASE_ADDR, 0x0B));	                                            //Setting MIU1 segment
    ms_writeb(USB_MIU_SEL2, GET_REG16_ADDR(USBC_BASE_ADDR, 0x0B) + 1);	                                        //Setting MIU2 segment
    ms_writeb(USB_MIU_SEL3, GET_REG16_ADDR(USBC_BASE_ADDR, 0x0C));	                                            //Setting MIU3 segment
    ms_writeb((ms_readb(GET_REG16_ADDR(USBC_BASE_ADDR, 0x0C) + 1) | 0x1), GET_REG16_ADDR(USBC_BASE_ADDR, 0x0C) + 1);  //Enable miu partition mechanism

#if  0//defined(DISABLE_MIU_LOW_BOUND_ADDR_SUBTRACT_ECO)
    printk("[USB] enable miu lower bound address subtraction\n");
    writeb(readb((void*)(USBC_base+0x0F*2-1)) | 0x1, (void*)(USBC_base+0x0F*2-1));
#endif
}
#endif

void msb250x_udc_init_utmi(void)
{
#if defined(ENABLE_OTG_USB_NEW_MIU_SLE)
    msb250x_udc_init_MIU_sel();
#endif

    // Disable UHC and OTG controllers
    //USBC_REG_WRITE8(0x4, USBC_REG_READ8(0x4)& (~0x3));
#if !defined(CONFIG_USB_MS_OTG)
    // Enable OTG controller
    ms_writeb(((ms_readb(GET_REG16_ADDR(USBC_BASE_ADDR, 0x01)) & ~(BIT0 | BIT1)) | BIT1), GET_REG16_ADDR(USBC_BASE_ADDR, 0x01));
#endif

#ifndef CONFIG_USB_FPGA_VERIFICATION
#ifdef CONFIG_USB_ENABLE_UPLL
    //UTMI_REG_WRITE16(0, 0x4000);
    ms_writew(0x6BC3, GET_REG16_ADDR(UTMI_BASE_ADDR, 0)); // Turn on UPLL, reg_pdn: bit<9> reg_pdn: bit<15>, bit <2> ref_pdn
    mdelay(1);
    ms_writeb(0x69, GET_REG16_ADDR(UTMI_BASE_ADDR, 0));   // Turn on UPLL, reg_pdn: bit<9>
    mdelay(2);
    ms_writew(0x0001, GET_REG16_ADDR(UTMI_BASE_ADDR, 0)); //Turn all (including hs_current) use override mode
    // Turn on UPLL, reg_pdn: bit<9>
    mdelay(3);
#else
    // Turn on UTMI if it was powered down
    if (0x0001 != ms_readw(GET_REG16_ADDR(UTMI_BASE_ADDR, 0)))
    {
        ms_writew(0x0001, GET_REG16_ADDR(UTMI_BASE_ADDR, 0)); //Turn all (including hs_current) use override mode
        mdelay(3);
    }
#endif

    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x3c)) | 0x01), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x3c)); // set CA_START as 1
    mdelay(10);
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x3c)) & ~0x01), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x3c)); // release CA_START
    while (0 == (ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x3c)) & 0x02));        // polling bit <1> (CA_END)

    //msb250x_udc_init_usb_ctrl();

    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x06)) & 0x9F) | 0x40, GET_REG8_ADDR(UTMI_BASE_ADDR, 0x06));      //reg_tx_force_hs_current_enable
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x03)) | 0x28), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x03));             //Disconnect window select
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x03)) & 0xef), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x03));             //Disconnect window select
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x07)) & 0xfd), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x07));             //Disable improved CDR

    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x09)) | 0x81), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x09));             // UTMI RX anti-dead-loc, ISI effect improvement
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x15)) | 0x20), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x15));             // Chirp signal source select
    ms_writeb((ms_readb(GET_REG8_ADDR(UTMI_BASE_ADDR, 0x0b)) | 0x80), GET_REG8_ADDR(UTMI_BASE_ADDR, 0x0b));             // set reg_ck_inv_reserved[6] to solve timing problem


#if 0//defined(CONFIG_MSTAR_CEDRIC)
    ms_writeb(0x10, GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16));
    ms_writeb(0x02, GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16) + 1);
    ms_writeb(0x81, GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17) + 1);
    //UTMI_REG_WRITE8(0x2c*2,   0x10);
    //UTMI_REG_WRITE8(0x2d*2-1, 0x02);
    //UTMI_REG_WRITE8(0x2f*2-1, 0x81);
#else
    //ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16)) | 0x98), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16));
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16)) | 0x10), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16));
    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16) + 1) | 0x02), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x16) + 1);
    //ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17)) | 0x10), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17));
    //ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17) + 1) | 0x01), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x17) + 1);
    //UTMI_REG_WRITE8(0x2c*2,   UTMI_REG_READ8(0x2c*2) |0x98);
    //UTMI_REG_WRITE8(0x2d*2-1, UTMI_REG_READ8(0x2d*2-1) |0x02);
    //UTMI_REG_WRITE8(0x2e*2,   UTMI_REG_READ8(0x2e*2) |0x10);
    //UTMI_REG_WRITE8(0x2f*2-1, UTMI_REG_READ8(0x2f*2-1) |0x01)
#endif


    ms_writeb((ms_readb(GET_REG16_ADDR(UTMI_BASE_ADDR, 0x02)) | 0x80), GET_REG16_ADDR(UTMI_BASE_ADDR, 0x02)); //avoid glitch
#endif
}

void msb250x_udc_reset_otg(void)
{
    ms_writeb(ms_readb(MSB250X_OTG0_USB_CFG0_L) & ~MSB250X_OTG0_CFG0_SRST_N, MSB250X_OTG0_USB_CFG0_L);  //low active
    ms_writeb(ms_readb(MSB250X_OTG0_USB_CFG0_L) | MSB250X_OTG0_CFG0_SRST_N, MSB250X_OTG0_USB_CFG0_L);   //default set as 1
}
EXPORT_SYMBOL(msb250x_udc_reset_otg);

void msb250x_udc_init_otg(struct msb250x_udc *udc)
{
    //u8 ep_num = 0;
    u8 power = 0;
    u8 usb_cfg_l = 0;
    u8 usb_cfg_h = 0;
    u8 usb_intr = 0;

    struct otg0_usb_cfg0_h* pst_usb_cfg0_h = (struct otg0_usb_cfg0_h*) &usb_cfg_h;
    struct otg0_usb_cfg6_h* pst_usb_cfg6_h = NULL;
    struct otg0_usb_power* pst_power = (struct otg0_usb_power*) &power;
    struct otg0_usb_intr* pst_intr_usb = (struct otg0_usb_intr*) &usb_intr;

    //ms_writeb((ms_readb(MSB250X_OTG0_USB_CFG6_H) | (MSB250X_OTG0_CFG6_H_SHORT_MODE | MSB250X_OTG0_CFG6_H_BUS_OP_FIX | MSB250X_OTG0_CFG6_H_REG_MI_WDFIFO_CTRL)), MSB250X_OTG0_USB_CFG6_H);

    //ms_writeb(ms_readb(MSB250X_OTG0_USB_CFG1_H) | MSB250X_OTG0_CFG1_H_SHORT_ECO, MSB250X_OTG0_USB_CFG1_H);
    usb_cfg_l = ms_readb(MSB250X_OTG0_USB_CFG0_L);
    usb_cfg_h = ms_readb(MSB250X_OTG0_USB_CFG0_H);

    pst_usb_cfg0_h->bDMPullDown = 1;
    ms_writeb(usb_cfg_h, MSB250X_OTG0_USB_CFG0_H); //Enable DM pull down
    //printk("<USB>[UDC] Enable DM pull down\n");

    //USB_REG_WRITE16(0x100, USB_REG_READ16(0x100)|0x8000); /* Disable DM pull-down */

    // Set FAddr to 0
    ms_writeb(0, MSB250X_OTG0_FADDR_REG);

    pst_usb_cfg6_h = (struct otg0_usb_cfg6_h*) &usb_cfg_h;
    usb_cfg_h = ms_readb(MSB250X_OTG0_USB_CFG6_H);

    pst_usb_cfg6_h->bINT_WR_CLR_EN = 1;
    pst_usb_cfg6_h->bBusOPFix = 1;
    pst_usb_cfg6_h->bShortMode = 1;
    pst_usb_cfg6_h->bREG_MI_WDFIFO_CTRL = 1;
    ms_writeb(usb_cfg_h, MSB250X_OTG0_USB_CFG6_H);

    /*
    USB_REG_WRITE8(M_REG_CFG6_H, USB_REG_READ8(M_REG_CFG6_H) | 0x08);
    USB_REG_WRITE8(M_REG_CFG6_H, USB_REG_READ8(M_REG_CFG6_H) | 0x40);
    */

    //while(0x18 != (USB_REG_READ8(M_REG_DEVCTL) & 0x18));

    power = ms_readb(MSB250X_OTG0_PWR_REG);

    pst_power->bSuspendMode = 0;
    pst_power->bSoftConn = 0;

    if (USB_SPEED_HIGH == udc->gadget.max_speed)
    {
        pst_power->bHSEnab = 1;
    }
    else
    {
        pst_power->bHSEnab = 0;
    }
    ms_writeb(power, MSB250X_OTG0_PWR_REG);

    ms_writeb(0, MSB250X_OTG0_DEVCTL_REG);

    usb_intr = 0xff;
    pst_intr_usb->bConn = 0;
    pst_intr_usb->bSOF = 0;
    ms_writeb(usb_intr, MSB250X_OTG0_INTRUSB_REG);
    //printk("<USB>[UDC] Enable usb interrupt\n");

    ms_readb(MSB250X_OTG0_INTRUSB_REG);

    // Flush the next packet to be transmitted/ read from the endpoint 0 FIFO
    ms_writeb(0, MSB250X_OTG0_INDEX_REG);
    ms_writeb(0x1 , MSB250X_OTG0_CSR0_FLSH_REG);

    ms_writeb(0x01, MSB250X_OTG0_INTRTXE_REG);
    ms_writeb(0x01, MSB250X_OTG0_INTRRXE_REG);

    ms_readb(MSB250X_OTG0_INTRTXE_REG);
    //ms_readb(MSB250X_OTG0_INTRRXE_REG);
}
EXPORT_SYMBOL(msb250x_udc_init_otg);

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_utmi_init
+------------------------------------------------------------------------------
| DESCRIPTION : initial the UTMI interface
|
| RETURN      : NULL
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
*/
#if 0
static void msb250x_utmi_init(void)
{
    u32 temp;

    temp=ms_readw(UTMI_BASE_ADDR + 0x0C) | 0x0040;
    temp &= 0xDBFD;
    ms_writew(temp, UTMI_BASE_ADDR + 0x0C);

    temp = ms_readb(UTMI_BASE_ADDR + 0x11) | 0x62;
    ms_writeb(temp, UTMI_BASE_ADDR + 0x11);

    temp = ms_readb(UTMI_BASE_ADDR + 0x4D) | 0x08;
    temp &= 0xFB;
    ms_writeb(temp, UTMI_BASE_ADDR + 0x4D);

    temp = ms_readb(UTMI_BASE_ADDR + 0x51) | 0x08;
    ms_writeb(temp, UTMI_BASE_ADDR + 0x51);

    temp = ms_readb(UTMI_BASE_ADDR + 0x54) & 0xF0;
    ms_writeb(temp, UTMI_BASE_ADDR + 0x54);

    temp=ms_readw(UTMI_BASE_ADDR + 0x58) | 0x0791;
    temp &= 0xFFCD;
    ms_writew(temp, UTMI_BASE_ADDR + 0x58);

    temp = ms_readb(UTMI_BASE_ADDR + 0x5D) | 0x0E;
    ms_writeb(temp, UTMI_BASE_ADDR + 0x5D);

    while((ms_readb(UTMI_BASE_ADDR + 0x60) & 0x0001) == 0);

    temp = ms_readb(USBC_BASE_ADDR + 0x04) | 0x02;
    ms_writeb(temp, USBC_BASE_ADDR + 0x04);

    temp = ms_readb(OTG0_BASE_ADDR + 0x100) & 0xFE; /* Reset OTG */
    ms_writeb(temp, OTG0_BASE_ADDR + 0x100);

    temp = ms_readb(OTG0_BASE_ADDR + 0x100) | 0x01;
    ms_writeb(temp, OTG0_BASE_ADDR + 0x100);

    temp = ms_readb(OTG0_BASE_ADDR + 0x118) | 0x01;
    ms_writeb(temp, OTG0_BASE_ADDR + 0x118);
}
#endif


/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_udc_probe
+------------------------------------------------------------------------------
| DESCRIPTION : The generic driver interface function which called for initial udc
|
| RETURN      : zero on success, else negative error code
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| pdev                 |x  |       | platform_device struct point
|--------------------+---+---+-------------------------------------------------
*/

static int msb250x_udc_probe(struct platform_device *pdev)
{
    struct msb250x_udc *dev = &memory;
    int retval = 0;
    int irq = -1;

    spin_lock_init(&dev->lock);

    dev->gadget.dev.parent = &pdev->dev;
    dev->pdev = pdev;

    platform_set_drvdata(pdev, dev);

    msb250x_udc_disable(dev);

    msb250x_udc_reinit(dev);

    /* irq setup after old hardware state is cleaned up */
    irq = irq_of_parse_and_map(pdev->dev.of_node, 0);

    if (0 != (retval = request_irq(irq/*INT_MS_OTG*/, msb250x_udc_isr, 0, sg_gadget_name, dev)))
    {
        printk(KERN_ERR "<USB>[DRV] request_irq fail. irq/err(%d/%d)\n", irq, retval);
        return -EBUSY;
    }

    dev->got_irq = 1;
    printk(KERN_INFO "<USB>[DRV] %s irq --> %d\n", pdev->name, irq);

    if (0 != usb_add_gadget_udc(&pdev->dev, &dev->gadget))
    {
        printk(KERN_ERR "<USB>[DRV] Error in probe.\n");
        return -EBUSY;
    }

    dev->using_dma = (!dma_set_mask(&pdev->dev, DMA_BIT_MASK(64)))? 1 : 0;

    //msb250x_udc_init_otg(udc);
    msb250x_udc_init_utmi();
    usb_gadget_set_state(&dev->gadget, USB_STATE_POWERED);
#if 0
    MDrv_GPIO_PadVal_Set(76, PINMUX_FOR_GPIO_MODE);
    ms_writeb(ms_readb(GET_REG8_ADDR(REG_ADDR_BASE_PM_GPIO, 0X94)) & ~BIT0, GET_REG8_ADDR(REG_ADDR_BASE_PM_GPIO, 0X94));
#endif
    printk(KERN_INFO "<USB>[DRV] complete porbe\n");
    return retval;
}

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_udc_remove
+------------------------------------------------------------------------------
| DESCRIPTION : The generic driver interface function which called for disable udc
|
| RETURN      : zero on success, else negative error code
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
| pdev                 |x  |       | platform_device struct point
|--------------------+---+---+-------------------------------------------------
*/
static int msb250x_udc_remove(struct platform_device *pdev)
{
    struct msb250x_udc *dev = platform_get_drvdata(pdev);
    int irq;

    if (!dev->driver)
        return -EBUSY;

    irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
    if(dev->got_irq)
    {
        //printk("free irq: %d \n", udc->irq);
        free_irq(irq, dev);
        dev->got_irq = 0;
    }

    platform_set_drvdata(pdev, NULL);

    usb_del_gadget_udc(&dev->gadget);

    printk(KERN_INFO "<USB>[UDC] remove ok\n");
    return 0;
}

#ifdef CONFIG_PM
static int msb250x_udc_suspend(struct platform_device *pdev, pm_message_t message)
{
    struct msb250x_udc *udc = platform_get_drvdata(pdev);

    // disable udc
    msb250x_udc_disable(udc);

    // disable power
    ms_writeb(0, MSB250X_OTG0_PWR_REG);
    return 0;
}

int msb250x_udc_resume(struct platform_device *pdev)
{
    struct msb250x_udc *udc = platform_get_drvdata(pdev);

    {
        if (udc->driver)
        {
            // enable udc
            msb250x_udc_reset_otg();
            msb250x_udc_init_otg(udc);
            mdelay(1);
        }
    }
    return 0;
}
#else
#define msb250x_udc_suspend NULL
#define msb250x_udc_resume NULL
#endif

/*
+------------------------------------------------------------------------------
| FUNCTION    : msb250x_udc_init
+------------------------------------------------------------------------------
| DESCRIPTION : The generic driver interface function for register this driver
|               to Linux Kernel.
|
| RETURN      : 0 when success, error code in other case.
|
+------------------------------------------------------------------------------
| Variable Name      |IN |OUT|                   Usage
|--------------------+---+---+-------------------------------------------------
|                    |   |   |
+--------------------+---+---+-------------------------------------------------
*/


static struct of_device_id mstar_udc_of_device_ids[] =
{
    {.compatible = "sstar,infinity-udc"},
    {},
};

static struct platform_driver ss_udc_driver =
{
    .probe 		= msb250x_udc_probe,
    .remove 	= msb250x_udc_remove,
#ifdef CONFIG_PM
    .suspend	= msb250x_udc_suspend,
    .resume		= msb250x_udc_resume,
#endif
    .driver = {
        .name	= "soc:Sstar-udc",
        .of_match_table = mstar_udc_of_device_ids,
//		.bus	= &platform_bus_type,
    }
};

module_platform_driver(ss_udc_driver);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");


//arch_initcall(ms_udc_device_init);
//fs_initcall(msb250x_udc_init); //use fs_initcall due to this should be earlier than ADB module_init
//module_exit(msb250x_udc_exit);

//MODULE_ALIAS(DRIVER_NAME);
//MODULE_LICENSE("GPL");
//MODULE_DESCRIPTION(DRIVER_DESC);
