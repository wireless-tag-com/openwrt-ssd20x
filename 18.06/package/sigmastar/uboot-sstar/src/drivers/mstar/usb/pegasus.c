/*
* pegasus.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
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

#include <net.h>
#include "usb.h"
#include "usb_lan.h"

enum pegasus_registers {
	EthCtrl0 = 0,
	EthCtrl1 = 1,
	EthCtrl2 = 2,
	EthID = 0x10,
	Reg1d = 0x1d,
	EpromOffset = 0x20,
	EpromData = 0x21,	/* 0x21 low, 0x22 high byte */
	EpromCtrl = 0x23,
	PhyAddr = 0x25,
	PhyData = 0x26, 	/* 0x26 low, 0x27 high byte */
	PhyCtrl = 0x28,
	UsbStst = 0x2a,
	EthTxStat0 = 0x2b,
	EthTxStat1 = 0x2c,
	EthRxStat = 0x2d,
	WakeupControl = 0x78,
	Reg7b = 0x7b,
	Gpio0 = 0x7e,
	Gpio1 = 0x7f,
	Reg81 = 0x81,
};

#define	PEGASUS_REQT_READ	    (0xc0)
#define	PEGASUS_REQT_WRITE	    (0x40)
#define	PEGASUS_REQ_GET_REGS	(0xf0)
#define	PEGASUS_REQ_SET_REGS	(0xf1)

#define	DEFAULT_GPIO_RESET	    (0x24)
#define	DEFAULT_GPIO_SET	    (0x26)

#define	PEGASUS_MTU             (1536)
#define	REG_TIMEOUT		        (100)

unsigned char netbuf[PEGASUS_MTU]  __attribute__ ((aligned (128)));

/* NIC specific static variables go here */
static char packet[2096];

extern void __inline__ wait_ms(unsigned long ms);
extern int usb_bulk_transfer_in(struct usb_device *dev,
			void *data, int len,int *transdata);
extern int usb_bulk_transfer_out(struct usb_device *dev,
			void *data, int len);

/**************************************************************************
Pegasus Operations
***************************************************************************/
static int get_registers(struct eth_device *nic, u16 reg, u16 size, void *data)
{
	return usb_control_msg(nic->udev, usb_rcvctrlpipe(nic->udev, 0),
                               PEGASUS_REQ_GET_REGS, PEGASUS_REQT_READ,
                               0, reg, data, size, 500);
}

static int set_registers(const struct eth_device *nic, u16 reg, u16 size, void *data)
{
	return usb_control_msg(nic->udev, usb_sndctrlpipe(nic->udev, 0),
                               PEGASUS_REQ_SET_REGS, PEGASUS_REQT_WRITE,
                               0, reg, (void *)data, size, 500);
}

static int set_register(const struct eth_device *nic, u16 reg, u8 data)
{
	return usb_control_msg(nic->udev, usb_sndctrlpipe(nic->udev, 0),
                               PEGASUS_REQ_SET_REGS, PEGASUS_REQT_WRITE,
                               data, reg, &data, 1, 500);
}

static void disable_net_traffic(const struct eth_device *nic)
{
    u8 tmp = 0;

    DEBUGFUNC();
    set_registers(nic, EthCtrl0, 2, &tmp);
}

static void pegasus_disable(struct eth_device *nic)
{
    DEBUGFUNC();

    netif_stop_queue(nic);
    disable_net_traffic(nic);
}

static inline int
reset_mac(struct eth_device *nic)
{
	u8 data = 0x8;
	int i;

    DEBUGFUNC();
	set_register(nic, EthCtrl1, 0x08);
	for (i = 0; i < REG_TIMEOUT; i++)
	{
		get_registers(nic, EthCtrl1, 1, &data);
		if (~data & 0x08)
		{
			set_register(nic, Gpio1, 0x26);
			set_register(nic, Gpio0, 0x24);
			set_register(nic, Gpio0, DEFAULT_GPIO_SET);
			break;
		}
	}
	if (i == REG_TIMEOUT)
		return 0;

	return 1;
}

static int pegasus_reset(struct eth_device *nic)
{
    DEBUGFUNC();

    if (!reset_mac(nic))
    {
        USB_LAN_ERR("Reset mac error\n");
        return 0;
    }
    set_register(nic, Reg1d, 0);
    //reset phy
    set_register(nic, Reg7b, 1);
    wait_ms(100);
    //stop reset phy
    set_register(nic, Reg7b, 2);
    return 1;
}

static int enable_net_traffic(const struct eth_device *nic)
{
    u8 data[4] = {0xC9, 0 | 0x20 /* set full duplex */ | 0x10 /* set 100 Mbps */, 0x01 /* loopback = 0x09, otherwise = 0x01 */};

    DEBUGFUNC();

    return set_registers(nic, EthCtrl0, 3, data);
}

static void set_ethernet_addr(struct eth_device *nic)
{
    u8 node_id[6]={0};

    DEBUGFUNC();
    memset(node_id, 0, sizeof(node_id));
    get_registers(nic, EthID, sizeof(node_id), node_id);
    memcpy(nic->enetaddr, node_id, sizeof(node_id));

    USB_LAN_ERR("net addr:%x-%x-%x-%x-%x-%x\n",nic->enetaddr[0],nic->enetaddr[1],nic->enetaddr[2],nic->enetaddr[3],nic->enetaddr[4],nic->enetaddr[5]);
}

/**************************************************************************
POLL - Wait for a frame
***************************************************************************/
static int
pegasus_rx(struct eth_device *nic)
{
    void *usb_buffer;
    int datalen;

    DEBUGFUNC();

    usb_buffer=(void*)&netbuf[0];
    usb_bulk_transfer_in((nic->udev),usb_buffer, PEGASUS_MTU,&datalen);

	memcpy((uchar *)packet,(usb_buffer),datalen);
#ifdef USB_LAN_DEBUG
    dump_pkt((uchar *)packet, datalen);
#endif
	NetReceive((uchar *)packet, datalen);
	return 1;
}

/**************************************************************************
TRANSMIT - Transmit a frame
***************************************************************************/
static int
pegasus_tx(struct eth_device *nic, volatile void *packet, int length)
{
    void *usb_buffer;

    usb_buffer=(void*)&netbuf[0];
	memcpy(usb_buffer+2,(void *)packet,length);
	//@FIXME:add 2 dummy bytes to avoid USB LAN ignore it
    length +=2;
    while (length < 60)         //padding to 60 bytes
    {
        *((u8*)usb_buffer+length)=0;
		length++;
	}

#ifdef USB_LAN_DEBUG
    dump_pkt(usb_buffer, length);
#endif
    usb_bulk_transfer_out( nic->udev,usb_buffer, length);
	return 1;
}

static int
pegasus_open(struct eth_device *nic, bd_t * bis)
{
    u8 enet_addr[6];
    DEBUGFUNC();

    memcpy(enet_addr, nic->enetaddr, 6);
    set_registers(nic, EthID, 6, enet_addr);
	enable_net_traffic(nic);
	return 1;
}

/**************************************************************************
INIT - set up ethernet interface(s)
***************************************************************************/
int pegasus_init(struct eth_device *nic)
{
    DEBUGFUNC();

    nic->init = pegasus_open;
    nic->recv = pegasus_rx;
    nic->send = pegasus_tx;
    nic->halt = pegasus_disable;

    set_register(nic, EpromCtrl, 0);
    set_register(nic, EpromOffset, 0x04);
    set_register(nic, EpromCtrl, 2);
    set_register(nic, EpromCtrl, 0);
    set_register(nic, EpromData, 0);

    if (!pegasus_reset(nic))
    {
		USB_LAN_ERR("pegasus - device reset failed \n");
		return 0;
	}

    set_register(nic, 0x80, 0xc0);
    set_register(nic, 0x83, 0xff);
    set_register(nic, 0x84, 0x01);
    set_register(nic, Reg81, 0x02);

 	set_ethernet_addr(nic);
 	return 1;
}

