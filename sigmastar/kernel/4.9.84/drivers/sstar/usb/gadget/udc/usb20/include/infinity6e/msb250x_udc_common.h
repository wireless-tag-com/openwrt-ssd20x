/*
* msb250x_udc_common.h- Sigmastar
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
#ifndef MSB250X_MEMORY_H
#define MSB250X_MEMORY_H

#define GET_REG_ADDR(x, y)                  ((x) + ((y) << 2))
#define GET_BASE_ADDR_BY_BANK(x, y)         ((x) + ((y) << 1))

#define RIU_BASE                            0xFD200000
#define UTMI_BASE_ADDR                      GET_BASE_ADDR_BY_BANK(RIU_BASE, 0x42100)
#define USBC_BASE_ADDR                      GET_BASE_ADDR_BY_BANK(RIU_BASE, 0x42300)
#define OTG0_BASE_ADDR                      GET_BASE_ADDR_BY_BANK(RIU_BASE, 0x42500)

#define MIU0_BASE_ADDR                      0x20000000

#define MIU0_SIZE                           ((unsigned long) 0x10000000)

#define MIU0_BUS_BASE_ADDR                  ((unsigned long) 0x00000000)
#define MIU1_BUS_BASE_ADDR                  ((unsigned long) 0x80000000)

#define USB_MIU_SEL0                        ((u8) 0x70U)
#define USB_MIU_SEL1                        ((u8) 0xefU)
#define USB_MIU_SEL2                        ((u8) 0xefU)
#define USB_MIU_SEL3                        ((u8) 0xefU)

#define BIT0    0x0001
#define BIT1    0x0002
#define BIT2    0x0004
#define BIT3    0x0008
#define BIT4    0x0010
#define BIT5    0x0020
#define BIT6    0x0040
#define BIT7    0x0080
#define BIT8    0x0100

#define MSB250X_MAX_ENDPOINTS                   4
#define MSB250X_USB_DMA_CHANNEL                 3

#define ENABLE_OTG_USB_NEW_MIU_SLE              1

#define MSB250X_EPS_CAP(_dev, _ep_op) \
    .ep[0] = {                                                                      \
        .ep = {                                                                     \
            .name = ep0name,                                                        \
            .ops = _ep_op,                                                          \
            .caps = USB_EP_CAPS(USB_EP_CAPS_TYPE_CONTROL, USB_EP_CAPS_DIR_ALL),     \
        },                                                                          \
        .dev = _dev,                                                                \
    },                                                                              \
    .ep[1] = {                                                                      \
        .ep = {                                                                     \
            .name = "ep1",                                                          \
            .ops = _ep_op,                                                          \
            .caps = USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL),         \
        },                                                                          \
        .dev = _dev,                                                                \
    },                                                                              \
    .ep[2] = {                                                                      \
        .ep = {                                                                     \
            .name = "ep2",                                                          \
            .ops = _ep_op,                                                          \
            .caps = USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL),         \
        },                                                                          \
        .dev = _dev,                                                                \
    },                                                                              \
    .ep[3] = {                                                                      \
        .ep = {                                                                     \
            .name = "ep3",                                                          \
            .ops = _ep_op,                                                          \
            .caps = USB_EP_CAPS(USB_EP_CAPS_TYPE_ALL, USB_EP_CAPS_DIR_ALL),         \
        },                                                                          \
        .dev = _dev,                                                                \
    }

#define MSB250X_HIGH_BANDWIDTH_EP(_dev)     &((_dev)->ep[1].ep)

#define MSB250X_PA2BUS(a)   (MIU0_BUS_BASE_ADDR | (a - MIU0_BASE_ADDR))
#define MSB250X_BUS2PA(a)   ((a - MIU0_BUS_BASE_ADDR) + MIU0_BASE_ADDR)

#endif
