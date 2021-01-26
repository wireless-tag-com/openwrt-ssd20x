/*
* rtl8150.c- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: jiang.ann <jiang.ann@sigmastar.com.tw>
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


#define	IDR					0x0120
#define	MAR					0x0126
#define	CR					0x012e
#define	TCR					0x012f
#define	RCR					0x0130
#define	TSR					0x0132
#define	RSR					0x0133
#define	CON0				0x0135
#define	CON1				0x0136
#define	MSR					0x0137
#define	PHYADD				0x0138
#define	PHYDAT				0x0139
#define	PHYCNT				0x013b
#define	GPPC				0x013d
#define	BMCR				0x0140
#define	BMSR				0x0142
#define	ANAR				0x0144
#define	ANLP				0x0146
#define	AER					0x0148
#define CSCR				0x014C  /* This one has the link status */
#define CSCR_LINK_STATUS	(1 << 3)

#define	USB_LAN_REQT_READ	    0xc0
#define	USB_LAN_REQT_WRITE	    0x40
#define	USB_LAN_REQ_GET_REGS	0x05
#define	USB_LAN_REQ_SET_REGS	0x05

/* Receive status register errors */
#define RSR_ROK             (1)
#define RSR_CRC				(1<<2)
#define RSR_FAE				(1<<1)
#define RSR_ERRORS			(RSR_CRC | RSR_FAE)

#define	RTL8150_MTU			1700

static unsigned char netbuf[RTL8150_MTU]  __attribute__ ((aligned (8)));

extern void __inline__ wait_ms(unsigned long ms);
extern int usb_bulk_transfer_in(struct usb_device *dev,
		void *data, int len,int *transdata);
extern int usb_bulk_transfer_out(struct usb_device *dev,
		void *data, int len);

/* NIC specific static variables go here */
static char packet[2096];

/**************************************************************************
  RTL 8150 Operations
 ***************************************************************************/
static int get_registers(struct eth_device *nic, u16 indx, u16 size, void *data)
{

	return usb_control_msg(nic->udev, usb_rcvctrlpipe(nic->udev, 0),
			USB_LAN_REQ_GET_REGS, USB_LAN_REQT_READ,
			indx, 0, data, size, 500);
}

static int set_registers(struct eth_device *nic, u16 indx, u16 size, void *data)
{
	return usb_control_msg(nic->udev, usb_sndctrlpipe(nic->udev, 0),
			USB_LAN_REQ_SET_REGS, USB_LAN_REQT_WRITE,
			indx, 0, data, size, 500);
}

static void disable_net_traffic(struct eth_device *nic)
{
	u8 cr;
	get_registers(nic, CR, 1, &cr);
	cr &= 0xf3;
	set_registers(nic, CR, 1, &cr);
}

static void rtl8150_disable(struct eth_device *nic)
{
	netif_stop_queue(nic);
	disable_net_traffic(nic);
}

static int rtl8150_reset(struct eth_device *nic)
{
	u8 data = 0x10;
	unsigned long i = HZ;
	set_registers(nic, CR, 1, &data);
	do {
		get_registers(nic, CR, 1, &data);
	} while ((data & 0x10) && --i);
	//wait_ms(3000);
	//printf("data:%x\n",data);
	return (i > 0) ? 1 : 0;
}

static int enable_net_traffic(struct eth_device *nic)
{
	u8 cr, tcr, rcr, msr;

	DEBUGFUNC();

	/* RCR bit7=1 attach Rx info at the end;  =0 HW CRC (which is broken) */
	rcr = 0x9e;
	tcr = 0xd8;         //windows RTL driver set to 0x18
	cr = 0x0c;
	//if (!(rcr & 0x80))
	//	set_bit(USB_LAN_HW_CRC, &nic->flags);

	set_registers(nic, RCR, 1, &rcr);
	set_registers(nic, TCR, 1, &tcr);
	set_registers(nic, CR, 1, &cr);
	get_registers(nic, MSR, 1, &msr);

	return 0;
}

static void set_ethernet_addr(struct eth_device *nic)
{
	u8 node_id[6];

	DEBUGFUNC();
	memset(node_id, 0, sizeof(node_id));

	get_registers(nic, IDR, sizeof(node_id), node_id);
	memcpy(nic->enetaddr, node_id, sizeof(node_id));

	USB_LAN_DBG("net addr:%x-%x-%x-%x-%x-%x\n",nic->enetaddr[0],nic->enetaddr[1],nic->enetaddr[2],nic->enetaddr[3],nic->enetaddr[4],nic->enetaddr[5]);
}

/**************************************************************************
  POLL - Wait for a frame
 ***************************************************************************/
static int
rtl8150_rx(struct eth_device *nic)
{
	void *usb_buffer; //20080602 Nick Add
	int datalen;

	usb_buffer=(void*)&netbuf[0];
	if (usb_bulk_transfer_in((nic->udev),usb_buffer, RTL8150_MTU,&datalen) < 0)
		return -1;

	memcpy((uchar *)packet,(usb_buffer),datalen);
#ifndef CONFIG_USE_UBOOT2017
	NetReceive((uchar *)packet, datalen);
#else
        net_process_received_packet((uchar *)packet, datalen);
#endif

	return 1;
}

/**************************************************************************
  TRANSMIT - Transmit a frame
 ***************************************************************************/
static int
rtl8150_tx(struct eth_device *nic, volatile void *packet, int length)
{
	void *usb_buffer; //20080602 Nick Add

	usb_buffer=(void*)&netbuf[0];
	memcpy(usb_buffer,(void *)packet,length);

	while (length < 60)         //padding to 60 bytes
	{
		*((u8*)packet+length)=0;
		length++;
	}

	if (usb_bulk_transfer_out( nic->udev,usb_buffer, length) < 0)
		return -1;

	return 1;
}

static int
rtl8150_open(struct eth_device *nic, bd_t * bis)
{
	DEBUGFUNC();

	set_registers(nic, IDR, 6, nic->enetaddr);
	rtl8150_reset(nic);
	enable_net_traffic(nic);
	return 1;
}

/**************************************************************************
  INIT - set up ethernet interface(s)
 ***************************************************************************/
int rtl8150_init(struct eth_device *nic)
{
	DEBUGFUNC();

	nic->init = rtl8150_open;
	nic->recv = rtl8150_rx;
	nic->send = rtl8150_tx;
	nic->halt = rtl8150_disable;

	if (!rtl8150_reset(nic)) {
		printf("rtl8150 - device reset failed \n");
	}

	set_ethernet_addr(nic);

	strcpy(nic->name, "rtl8150");

	return 1;
}
