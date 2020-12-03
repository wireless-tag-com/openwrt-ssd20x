/*
* msb250x_ep.h- Sigmastar
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
#ifndef __MSB250X_EP_H
#define __MSB250X_EP_H
/******************************************************************************
 * Function definition
 ******************************************************************************/

static inline void ep_set_ipr(u8 ep_num)
{
    ms_writeb((ms_readb(MSB250X_OTG0_EP_TXCSR1_REG(ep_num)) | MSB250X_OTG0_TXCSR1_TXPKTRDY), MSB250X_OTG0_EP_TXCSR1_REG(ep_num));
}

static inline void ep_set_opr(u8 ep_num)
{
    ms_writeb((ms_readb(MSB250X_OTG0_EP_RXCSR1_REG(ep_num)) & ~MSB250X_OTG0_RXCSR1_RXPKTRDY), MSB250X_OTG0_EP_RXCSR1_REG(ep_num));
}

void msb250x_ep0_clear_opr(void);

void msb250x_ep0_clear_sst(void);

void msb250x_ep0_clear_se(void);

void msb250x_ep0_set_ipr(void);

void msb250x_ep0_set_de(void);

void msb250x_ep0_set_ss(void);

void msb250x_ep0_set_de_out(void);

void msb250x_ep0_set_sse_out(void);

void msb250x_ep0_set_de_in(void);

int msb250x_ep_enable(struct usb_ep *_ep, const
                      struct usb_endpoint_descriptor *desc);

int msb250x_ep_disable(struct usb_ep *_ep);

struct usb_request*
msb250x_ep_alloc_request(struct usb_ep *_ep,
                         gfp_t gfp_flags);

void msb250x_ep_free_request(struct usb_ep *_ep,
                             struct usb_request *_req);

int msb250x_ep_queue(struct usb_ep *_ep,
                     struct usb_request *_req,
                     gfp_t gfp_flags);

int msb250x_ep_dequeue(struct usb_ep *_ep,
                       struct usb_request *_req);

int msb250x_ep_set_halt(struct usb_ep *_ep,
                        int value);

void msb250x_ep0_isr_handler(struct msb250x_udc *dev);

void msb250x_ep_isr_handler(struct msb250x_udc* dev,
                            struct msb250x_ep *ep);

#endif
