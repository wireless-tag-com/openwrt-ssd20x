/*
* msb250x_dma.c- Sigmastar
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
* ms_dma.c- Sigmastar
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
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/ioport.h>
#include <linux/usb.h>
#include <linux/usb/gadget.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <linux/jiffies.h>
#include "msb250x_udc_reg.h"
#include "msb250x_udc.h"
#include "msb250x_gadget.h"

#define ENABLED_DMA_BYTES   512
#if 0
static inline void ms_writelw(uintptr_t val, u32 volatile* Reg)
{
    ms_writew((u32)(val & 0xffff), (uintptr_t) Reg);
    ms_writew((u32)(val>>16), (uintptr_t) (Reg+1));
    //printk( "write val:%x ",(val >> 16));
    //printk( "  *(Reg+4):%x\n",*(Reg+4));
}

static inline u32 ms_readlw(u32 volatile* Reg)
{
     return (volatile u32) ((ms_readw((uintptr_t) Reg)&0xffff)|(ms_readw((uintptr_t) (Reg+1))<<16));
}
#endif

static inline s8 msb250x_dma_get_channel(void)
{
    s8 ch;
    u16 ctrl = 0;

    struct otg0_dma_ctrlrequest* dma_ctrl = (struct otg0_dma_ctrlrequest*) &ctrl;

    for (ch = 1; ch <= MSB250X_USB_DMA_CHANNEL; ch++)
    {
        ctrl = ms_readw(MSB250X_OTG0_DMA_CNTL(ch));

        if (0 == dma_ctrl->bEnableDMA && 0 == dma_ctrl->bEndpointNumber)
        {
            return ch;
        }

    }

    printk("<USB>[DMA] No available channel!\n");
    /* cyg_semaphore_post(&ChannelDmaSem); */
    return -1;
}

u8 msb250x_dma_find_channel_by_ep(u8 ep_num)
{
    s8 ch = 0;
    u16 ctrl = 0;

    struct otg0_dma_ctrlrequest* dma_ctrl = (struct otg0_dma_ctrlrequest*) &ctrl;

    for (ch = 1; ch <= MSB250X_USB_DMA_CHANNEL; ch++)
    {
        ctrl = ms_readw(MSB250X_OTG0_DMA_CNTL(ch));

        if (ep_num == dma_ctrl->bEndpointNumber)
        {
            return ch;
        }

    }

    return 0;
}

EXPORT_SYMBOL(msb250x_dma_find_channel_by_ep);

void msb250x_dma_release_channel(s8 ch)
{
    u8 ep_num = 0;
    u16 ctrl = 0;
    struct otg0_dma_ctrlrequest* dma_ctrl = (struct otg0_dma_ctrlrequest*) &ctrl;

    //ctrl = ms_readw((uintptr_t) (DMA_CNTL_REGISTER(ch)));
    //ep_num = dma_ctrl->bEndpointNumber;

    ctrl = 0;
    ms_writew(ctrl, MSB250X_OTG0_DMA_CNTL(ch));
    ctrl = ms_readw(MSB250X_OTG0_DMA_CNTL(ch));

    if (0 != dma_ctrl->bEndpointNumber)
    {
        printk("<USB_ERR>[%d] DMA[%d] doesn't release!\n", ep_num, ch);
    }
}

EXPORT_SYMBOL(msb250x_dma_release_channel);

int msb250x_dma_setup_control(struct usb_ep *_ep,
                              struct msb250x_request *req,
                              u32 bytes)
{
    s8 ch = 0;
    u8 csr1 = 0;
    u8 csr2 = 0;

    u16 control = 0;
    u8 ep_num = 0;

    struct otg0_dma_ctrlrequest* dma_ctrl = (struct otg0_dma_ctrlrequest*) &control;
    struct otg0_ep_rxcsr_h* rxcsr_h = NULL;
    struct otg0_ep_rxcsr_l* rxcsr_l = NULL;
    struct otg0_ep_txcsr_h* txcsr_h = NULL;

    //struct msb250x_ep *ep = to_msb250x_ep(_ep);

    ep_num = usb_endpoint_num(_ep->desc);

    dma_ctrl->bEndpointNumber = ep_num;
    dma_ctrl->bEnableDMA = 1;
    dma_ctrl->bInterruptEnable = 1;
    dma_ctrl->bRurstMode = 3;
    dma_ctrl->bDirection = usb_endpoint_dir_out(_ep->desc)? 0 : 1;

    if (usb_endpoint_xfer_bulk(_ep->desc) && _ep->maxpacket < bytes)
    {
        dma_ctrl->bDMAMode = 1;
    }

    if (usb_endpoint_dir_out(_ep->desc)) // dma write
    {
        csr1 = ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(ep_num));
        csr2 = ms_readb(MSB250X_OTG0_EP_RXCSR2_REG(ep_num));

        rxcsr_l = (struct otg0_ep_rxcsr_l*) &csr1;
        rxcsr_h = (struct otg0_ep_rxcsr_h*) &csr2;

        csr2 &= ~RXCSR2_MODE1;

        if (1 == dma_ctrl->bDMAMode)
        {
            csr2 |= RXCSR2_MODE1;
        #if 0
            if (0 < (count % _ep->maxpacket))
            {
                rxcsr_h->bDMAReqMode = 0;
            }
        #endif
            rxcsr_h->bDisNyet = 1;
        }

    }
    else // dma read
    {
        txcsr_h = (struct otg0_ep_txcsr_h*) &csr2;
        csr2 = ms_readb(MSB250X_OTG0_EP_TXCSR2_REG(ep_num));

        csr2 &= ~TXCSR2_MODE1;

        if (1 == dma_ctrl->bDMAMode)
        {
            csr2 |= TXCSR2_MODE1;
        }
    }

    if (ENABLED_DMA_BYTES > bytes)
    {
    #if 0
        if (usb_endpoint_dir_out(_ep->desc))
        {
            if (1 == rxcsr_l->bRxPktRdy)
            {
                printk(KERN_DEBUG "<USB_WARN>[%d][DMA] bRxPktRdy/rxcount(%d/0x%04x) buf/actual/length(0x%p/0x%04x/0x%04x)\n",
                       ep_num, rxcsr_l->bRxPktRdy, ms_readw(MSB250X_OTG0_EP_RXCOUNT_L_REG(ep_num)), req->req.buf, req->req.actual, req->req.length);
            }
        }
    #endif
        return -1;
    }

    if (0 >= (ch = msb250x_dma_get_channel()))       /* no free channel */
    {
        printk("<USB>[%d][DMA] Get DMA channel fail!\n", ep_num);
        return -1;
    }

    ms_writeb(csr2, ((usb_endpoint_dir_out(_ep->desc))? MSB250X_OTG0_EP_RXCSR2_REG(ep_num) : MSB250X_OTG0_EP_TXCSR2_REG(ep_num)));
#if 0
    ms_writelw((uintptr_t) (req->req.dma + req->req.actual), (u32 volatile*) MSB250X_OTG0_DMA_ADDR(ch));
    ms_writelw((uintptr_t) count, (u32 volatile*) MSB250X_OTG0_DMA_COUNT(ch));
#else
    ms_writelw((req->req.dma + req->req.actual), MSB250X_OTG0_DMA_ADDR(ch));
    ms_writelw(bytes, MSB250X_OTG0_DMA_COUNT(ch));
#endif
    ms_writew(control, MSB250X_OTG0_DMA_CNTL(ch));

#if 0
    //if (usb_endpoint_dir_out(ep->ep.desc))
    printk(KERN_DEBUG "<USB>[%s][DMA][%d] %s mode/ctrl(0x%x/0x%04x) buff/bytes/actual/length(0x%p/0x%04x/0x%04x/0x%04x)\n",
                      _ep->name,
                      ch,
                      (usb_endpoint_dir_out(_ep->desc))? "RX" : "TX",
                      dma_ctrl->bDMAMode,
                      control,
                      req->req.buf,
                      bytes,
                      req->req.actual,
                      req->req.length);
#endif
    return 0;
}

EXPORT_SYMBOL(msb250x_dma_setup_control);

void msb250x_dma_isr_handler(u8 ch,
                             struct msb250x_udc *dev)
{
    int is_last = 0;

    u32 bytesleft, bytesdone, control;
    u8  csr2 = 0,csr1 = 0;
    dma_addr_t dma_handle;
    u8 ep_num;

	u16 ep_maxpacket = 0;

//    struct platform_device* pdev = dev->pdev;
    struct msb250x_ep* ep;
    struct usb_ep* _ep;
    struct msb250x_request *req = NULL;

    struct otg0_dma_ctrlrequest* dma_ctrl = (struct otg0_dma_ctrlrequest*) &control;
    struct otg0_ep_rxcsr_h* rxcsr_h = NULL;
    struct otg0_ep_rxcsr_l* rxcsr_l = NULL;
//    struct ep_txcsr_h* txcsr_h = NULL;
    struct otg0_ep_txcsr_l* txcsr_l = NULL;

    control = ms_readw(MSB250X_OTG0_DMA_CNTL(ch));
    dma_handle = (dma_addr_t) ms_readlw((u32 volatile*) MSB250X_OTG0_DMA_ADDR(ch));

    bytesleft = ms_readlw((u32 volatile*) MSB250X_OTG0_DMA_COUNT(ch));

    ep_num = dma_ctrl->bEndpointNumber;

    ep = &dev->ep[ep_num];
    _ep = &ep->ep;
    msb250x_dma_release_channel(ch);

    if (!_ep->desc)
    {
        //printk(KERN_ERR "<USB>[%s][DMA][%d] DMA had been disabled.\n", _ep->name, ch);
        return;
    }

    /* release DMA channel */

    if (likely(!list_empty(&ep->queue)))
    {
        req = list_entry(ep->queue.next, struct msb250x_request, queue);
    }

    if (req)
    {
        bytesdone = dma_handle - req->req.dma - req->req.actual;

        if (1 == dma_ctrl->bBusError)
        {
            printk(KERN_ERR "<USB_ERR>[DMA] Bus ERR!\n");

            //ep->halted = 1; /* Winder */

            return;
        }

        ep_maxpacket = ep->ep.maxpacket;

        if (usb_endpoint_dir_in(ep->ep.desc))
        {
            csr1 = ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(ep_num));
            txcsr_l = (struct otg0_ep_txcsr_l*) &csr1;

            csr2 = ms_readb(MSB250X_OTG0_EP_TXCSR2_REG(ep_num));
            ms_writeb((csr2 & ~TXCSR2_MODE1), MSB250X_OTG0_EP_TXCSR2_REG(ep_num));

            if (0 == dma_ctrl->bDMAMode || 0 < (bytesdone % ep_maxpacket)) /* short packet || TX DMA mode0 */
            {
                txcsr_l->bTxPktRdy = 1;

                ms_writeb(csr1, MSB250X_OTG0_EP_TXCSR1_REG(ep_num));
            #if 0
                if (1 == dma_ctrl->bDMAMode)
                {
                    DBG_MSG("DMA_TX mode1 short packet\n");
                }
            #endif
            }
        }
        else
        {
            csr1 = ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(ep_num));
            csr2 = ms_readb(MSB250X_OTG0_EP_RXCSR2_REG(ep_num));

            rxcsr_l = (struct otg0_ep_rxcsr_l*) &csr1;
            rxcsr_h = (struct otg0_ep_rxcsr_h*) &csr2;

            ms_writeb((csr2 & ~RXCSR2_MODE1), MSB250X_OTG0_EP_RXCSR2_REG(ep_num));

        #if 1
            if (0 == bytesleft)
            {
                if (usb_endpoint_xfer_bulk(_ep->desc))
                {
                    if (1 == dma_ctrl->bDMAMode)
                    {
                        msb250x_udc_ok2rcv_for_packets(ep->autoNAK_cfg, 0);
                    }

                    while (0 == (ms_readb(MSB250X_OTG0_USB_CFG7_H) & 0x80)) //last done bit
                    {
                        //printk(KERN_DEBUG "<USB>[DMA][%d] Last done bit.\n", ep_num);
                    }

                }

                if (0 == dma_ctrl->bDMAMode || 0 == rxcsr_h->bDMAReqMode)
                {
                    rxcsr_l->bRxPktRdy = 0;
                    ms_writeb(csr1, MSB250X_OTG0_EP_RXCSR1_REG(ep_num));
                }
            }
        #endif

            msb250x_gadget_sync_request(ep->gadget, &req->req, req->req.actual, bytesdone);
        }

        req->req.actual += bytesdone;

        if (req->req.actual == req->req.length || (1 != dma_ctrl->bDMAMode && 0 < (bytesdone % ep->ep.maxpacket)))
        {
            is_last = 1;
        }
#if 0
        //if (!usb_endpoint_xfer_int(ep->ep.desc) && !usb_endpoint_dir_in(ep->ep.desc))
        //if (usb_endpoint_dir_out(ep->ep.desc))
        //if (1 == dma_ctrl->bDMAMode && 0 < (bytesdone % ep->ep.maxpacket))
        printk(KERN_DEBUG "<USB>[%s][DMA][%d] %s mode/%s/ctrl/bytesdone/bytesleft(0x%x/0x%x/0x%04x/0x%04x/0x%04x) buff/%s/actual/length(0x%p/0x%04x/0x%04x/0x%04x)\n",
                          _ep->name,
                          ch,
                          (usb_endpoint_dir_out(_ep->desc))? "RX" : "TX",
                          (usb_endpoint_dir_out(_ep->desc))? "bRxPktRdy" : "bTxPktRdy",
                          dma_ctrl->bDMAMode,
                          (usb_endpoint_dir_out(_ep->desc))? rxcsr_l->bRxPktRdy : txcsr_l->bTxPktRdy,
                          control,
                          bytesdone,
                          bytesleft,
                          (usb_endpoint_dir_out(_ep->desc))? "count" : "last",
                          req->req.buf,
                          (usb_endpoint_dir_out(_ep->desc))? ms_readw(MSB250X_OTG0_EP_RXCOUNT_L_REG(ep_num)) : is_last,
                          req->req.actual,
                          req->req.length);
#endif
        if (1 == is_last)
        {
            msb250x_request_done(ep, req, 0);
        }
    }

    if (usb_endpoint_dir_out(ep->ep.desc) || !usb_endpoint_xfer_isoc(_ep->desc))
    {
        msb250x_set_ep_halt(ep, 0);
        msb250x_request_continue(ep);
    }

    return;

}

EXPORT_SYMBOL(msb250x_dma_isr_handler);
