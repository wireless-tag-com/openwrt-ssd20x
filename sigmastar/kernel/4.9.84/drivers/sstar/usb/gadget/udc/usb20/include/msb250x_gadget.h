/*
* msb250x_gadget.h- Sigmastar
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
#ifndef __MSB250X_GAGET_H
#define __MSB250X_GAGET_H

void msb250x_gadget_sync_request(struct usb_gadget* gadget, struct usb_request* req, int offset, int size);

int msb250x_gadget_map_request(struct usb_gadget* gadget, struct usb_request* req, int is_in);

void msb250x_gadget_unmap_request(struct usb_gadget* gadget, struct usb_request* req, int is_in);

void msb250x_gadget_pullup_i(int is_on);

int msb250x_gadget_get_frame(struct usb_gadget *g);

struct usb_ep*
msb250x_gadget_match_ep(struct usb_gadget *g,
		                struct usb_endpoint_descriptor *desc,
		                struct usb_ss_ep_comp_descriptor *ep_comp);

int msb250x_gadget_wakeup(struct usb_gadget *_gadget);

int msb250x_gadget_set_selfpowered(struct usb_gadget *g,
                                   int value);

int msb250x_gadget_pullup(struct usb_gadget *g,
                          int is_on);

int msb250x_gadget_vbus_session(struct usb_gadget *g,
                                int is_active);

int msb250x_gadget_vbus_draw(struct usb_gadget *g,
                             unsigned ma);

int msb250x_gadget_udc_start(struct usb_gadget *g,
                             struct usb_gadget_driver *driver);

int msb250x_gadget_udc_stop(struct usb_gadget *g);

#endif