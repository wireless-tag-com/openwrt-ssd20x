/*
* msb250x_udc.h- Sigmastar
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
* msb250x_udc.h- Sigmastar
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
/*------------------------------------------------------------------------------
    PROJECT: MSB250x Linux BSP
    DESCRIPTION:
          MSB250x dual role USB device controllers


    HISTORY:
         6/11/2010     Calvin Hung    First Revision

-------------------------------------------------------------------------------*/
#ifndef __MSB250X_UDC_H
#define __MSB250X_UDC_H

/******************************************************************************
 * Include Files
 ******************************************************************************/
#include <linux/kernel.h>
#include <asm/io.h>
#include <linux/usb/gadget.h>
//#include "msb250x_udc_common.h"
/******************************************************************************
 * Constants
 ******************************************************************************/
#define USB_HIGH_SPEED 1
#define QC_BOARD 1
#define EP0_FIFO_SIZE   64
#define EP_FIFO_SIZE    512

#define ms_readb(a)             readb((void*)(a))
#define ms_writeb(v, a)         writeb((v), (void*)(a))

#define ms_readw(a)             readw((void*)(a))
#define ms_writew(v, a)         writew((v), (void*)(a))

#define ms_writesb(v, a, l)     ({ \
                                    prefetch((void*)(v)); \
                                    writesb((volatile void *)(a), (void *)(v), (l)); \
                                })

#define ms_readsb(v, a, l)      readsb((const volatile void*)(a), (void*)(v), (l))
#if 1
#define ms_writelw(v, a)        ms_writew((v & 0xffff), (a)); \
                                ms_writew(((v >> 16) & 0xffff), ((volatile u32*)(a) + 1));
#define ms_readlw(a)            (volatile u32)((ms_readw((a)) & 0xffff) | (ms_readw(((volatile u32*)(a) + 1)) << 16));
#endif
/******************************************************************************
 * Variables
 ******************************************************************************/
enum ep0_state
{
    EP0_IDLE,
    EP0_IN_DATA_PHASE,
    EP0_OUT_DATA_PHASE,
};

struct msb250x_ep
{
    struct list_head queue;
    struct usb_gadget *gadget;
    struct msb250x_udc *dev;
    struct usb_ep ep;

    unsigned short fifo_size;
    u8  autoNAK_cfg;
    u8 halted : 1;
    u8 already_seen : 1;
    u8 setup_stage : 1;
    u8 shortPkt : 1;
    u8 zero : 1;
};

struct msb250x_request
{
    struct list_head        queue;      /* ep's requests */
    struct usb_request      req;
};

struct msb250x_udc
{
    spinlock_t lock;

    struct msb250x_ep ep[MSB250X_MAX_ENDPOINTS];
    struct usb_gadget           gadget;
    struct usb_gadget_driver    *driver;
    struct msb250x_request      fifo_req;
    struct platform_device      *pdev;
    u16 devstatus;
    int address;
    enum ep0_state ep0state;
    unsigned got_irq : 1;
    unsigned req_std : 1;
    unsigned delay_status : 1;
    unsigned req_pending : 1;
    unsigned soft_conn : 1;
    unsigned using_dma : 1;
};

/* --------------------- container_of ops ----------------------------------*/
static inline struct msb250x_ep *to_msb250x_ep(struct usb_ep *ep)
{
    return container_of(ep, struct msb250x_ep, ep);
}

static inline struct msb250x_udc *to_msb250x_udc(struct usb_gadget *gadget)
{
    return container_of(gadget, struct msb250x_udc, gadget);
}

static inline struct msb250x_request *to_msb250x_req(struct usb_request *req)
{
    return container_of(req, struct msb250x_request, req);
}

static inline void msb250x_set_ep_halt(struct msb250x_ep* ep, int value)
{
    ep->halted = value;
}

/* -----------------------------------------------------------*/

int msb250x_udc_get_autoNAK_cfg(u8 ep_num);

int msb250x_udc_release_autoNAK_cfg(u8 cfg);

void msb250x_udc_enable_autoNAK(u8 ep_num, u8 cfg);
void msb250x_udc_ok2rcv_for_packets(u8 cfg, u16 pkt_num);
void msb250x_udc_allowAck(u8 cfg);

void msb250x_request_nuke(struct msb250x_udc *udc,
                          struct msb250x_ep *ep, int status);

void msb250x_request_continue(struct msb250x_ep *ep);

void msb250x_request_done(struct msb250x_ep *ep,
                          struct msb250x_request *req,
                          int status);

struct msb250x_request*
msb250x_request_handler(struct msb250x_ep* ep,
                        struct msb250x_request* req);

int msb250x_udc_get_status(struct msb250x_udc *dev,
                           struct usb_ctrlrequest *crq);

void msb250x_udc_init_usb_ctrl(void);
void msb250x_udc_reset_otg(void);
void msb250x_udc_init_otg(struct msb250x_udc *udc);


#endif /* __MSB250X_UDC_H */
