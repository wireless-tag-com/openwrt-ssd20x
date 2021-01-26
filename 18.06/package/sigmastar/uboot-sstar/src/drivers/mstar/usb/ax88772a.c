/*
* ax88772a.c- Sigmastar
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

/* NAMING CONSTANT DECLARATIONS */
#define AX88772A_DRV_NAME		"AX88772A"
#define AX88796C_DRV_VERSION		"1.0.100"

/* Endpoint */
#define AX_CTRL_EP			0
#define AX_INTR_EP			1
#define AX_BULKIN_EP			2
#define AX_BULKOUT_EP			3

/* ASIX AX8817X based USB 2.0 Ethernet Devices */
#define AX_CMD_SET_SW_MII		0x06
#define AX_CMD_READ_MII_REG		0x07
#define AX_CMD_WRITE_MII_REG		0x08
#define AX_CMD_SET_HW_MII		0x0a
#define AX_CMD_READ_EEPROM		0x0b
#define AX_CMD_WRITE_EEPROM		0x0c
#define AX_CMD_WRITE_ENABLE		0x0d
#define AX_CMD_WRITE_DISABLE		0x0e
#define AX_CMD_READ_RX_CTL		0x0f
#define AX_CMD_WRITE_RX_CTL		0x10
#define AX_CMD_READ_IPG012		0x11
#define AX_CMD_WRITE_IPG0		0x12
#define AX_CMD_WRITE_IPG1		0x13
#define AX_CMD_READ_NODE_ID		0x13
#define AX_CMD_WRITE_NODE_ID		0x14
#define AX_CMD_WRITE_IPG2		0x14
#define AX_CMD_WRITE_MULTI_FILTER	0x16
#define AX88172_CMD_READ_NODE_ID	0x17
#define AX_CMD_READ_PHY_ID		0x19
#define AX_CMD_READ_MEDIUM_STATUS	0x1a
#define AX_CMD_WRITE_MEDIUM_MODE	0x1b
#define AX_CMD_READ_MONITOR_MODE	0x1c
#define AX_CMD_WRITE_MONITOR_MODE	0x1d
#define AX_CMD_READ_GPIOS		0x1e
#define AX_CMD_WRITE_GPIOS		0x1f
#define AX_CMD_SW_RESET			0x20
#define AX_CMD_SW_PHY_STATUS		0x21
#define AX_CMD_SW_PHY_SELECT		0x22

#define AX_SWRESET_CLEAR		0x00
#define AX_SWRESET_RR			0x01
#define AX_SWRESET_RT			0x02
#define AX_SWRESET_BZAUTO		0x04
#define AX_SWRESET_PRL			0x08
#define AX_SWRESET_BZ			0x10
#define AX_SWRESET_IPRL			0x20
#define AX_SWRESET_IPPD			0x40

#define SW_PHYSEL_PSEL			0x01
#define SW_PHYSEL_ASEL			0x02
#define SW_PHYSEL_SS_MII		0x04
#define SW_PHYSEL_SS_RMII		0x08
#define SW_PHYSEL_SS_RRMII		0x0C
#define SW_PHYSEL_SSEN			0x10

/* AX88772 Medium Mode Register */
#define AX_MEDIUM_PF		0x0080
#define AX_MEDIUM_JFE		0x0040
#define AX_MEDIUM_TFC		0x0020
#define AX_MEDIUM_RFC		0x0010
#define AX_MEDIUM_ENCK		0x0008
#define AX_MEDIUM_AC		0x0004
#define AX_MEDIUM_FD		0x0002
#define AX_MEDIUM_GM		0x0001
#define AX_MEDIUM_SM		0x1000
#define AX_MEDIUM_SBP		0x0800
#define AX_MEDIUM_PS		0x0200
#define AX_MEDIUM_RE		0x0100

#define AX88772_MEDIUM_DEFAULT	(AX_MEDIUM_FD | AX_MEDIUM_RFC | AX_MEDIUM_TFC | AX_MEDIUM_PS | AX_MEDIUM_AC | AX_MEDIUM_RE )

/* AX88772 RX_CTL values */
#define AX_RX_CTL_STOP		0x0000	/* Stop MAC */
#define AX_RX_CTL_SO		0x0080	/* Ethernet MAC start */
#define AX_RX_CTL_AP		0x0020	/* Accept physcial address from Multicast array */
#define AX_RX_CTL_AM		0x0010
#define AX_RX_CTL_AB		0x0008	/* Accetp Brocadcast frames*/
#define AX_RX_CTL_SEP		0x0004	/* Save error packets */
#define AX_RX_CTL_AMALL		0x0002	/* Accetp all multicast frames */
#define AX_RX_CTL_PRO		0x0001	/* Promiscuous Mode */
#define AX_RX_CTL_MFB_2048	0x0000
#define AX_RX_CTL_MFB_4096	0x0100
#define AX_RX_CTL_MFB_8192	0x0200
#define AX_RX_CTL_MFB_16384	0x0300


#define AX_DEFAULT_RX_CTL	(AX_RX_CTL_SO | AX_RX_CTL_AB )

/* GPIO 0 .. 2 toggles */
#define AX_GPIO_GPO0EN		0x01	/* GPIO0 Output enable */
#define AX_GPIO_GPO_0		0x02	/* GPIO0 Output value */
#define AX_GPIO_GPO1EN		0x04	/* GPIO1 Output enable */
#define AX_GPIO_GPO_1		0x08	/* GPIO1 Output value */
#define AX_GPIO_GPO2EN		0x10	/* GPIO2 Output enable */
#define AX_GPIO_GPO_2		0x20	/* GPIO2 Output value */
#define AX_GPIO_RESERVED	0x40	/* Reserved */
#define AX_GPIO_RSE		0x80	/* Reload serial EEPROM */

#define MII_BMCR		0x00	/* Basic mode control register */
#define MII_BMSR		0x01	/* Basic mode status register  */
#define MII_PHYSID1		0x02	/* PHYS ID 1                   */
#define MII_PHYSID2		0x03	/* PHYS ID 2                   */
#define MII_ADVERTISE		0x04	/* Advertisement control reg   */
#define MII_LPA			0x05	/* Link partner ability reg    */

/* Basic mode control register. */
#define BMCR_RESV               0x003f  /* Unused...                   */
#define BMCR_SPEED1000		0x0040  /* MSB of Speed (1000)         */
#define BMCR_CTST               0x0080  /* Collision test              */
#define BMCR_FULLDPLX           0x0100  /* Full duplex                 */
#define BMCR_ANRESTART          0x0200  /* Auto negotiation restart    */
#define BMCR_ISOLATE            0x0400  /* Disconnect DP83840 from MII */
#define BMCR_PDOWN              0x0800  /* Powerdown the DP83840       */
#define BMCR_ANENABLE           0x1000  /* Enable auto negotiation     */
#define BMCR_SPEED100           0x2000  /* Select 100Mbps              */
#define BMCR_LOOPBACK           0x4000  /* TXD loopback bits           */
#define BMCR_RESET              0x8000  /* Reset the DP83840           */

/* Basic mode status register. */
#define BMSR_ERCAP              0x0001  /* Ext-reg capability          */
#define BMSR_JCD                0x0002  /* Jabber detected             */
#define BMSR_LSTATUS            0x0004  /* Link status                 */
#define BMSR_ANEGCAPABLE        0x0008  /* Able to do auto-negotiation */
#define BMSR_RFAULT             0x0010  /* Remote fault detected       */
#define BMSR_ANEGCOMPLETE       0x0020  /* Auto-negotiation complete   */
#define BMSR_RESV               0x00c0  /* Unused...                   */
#define BMSR_ESTATEN		0x0100	/* Extended Status in R15 */
#define BMSR_100FULL2		0x0200	/* Can do 100BASE-T2 HDX */
#define BMSR_100HALF2		0x0400	/* Can do 100BASE-T2 FDX */
#define BMSR_10HALF             0x0800  /* Can do 10mbps, half-duplex  */
#define BMSR_10FULL             0x1000  /* Can do 10mbps, full-duplex  */
#define BMSR_100HALF            0x2000  /* Can do 100mbps, half-duplex */
#define BMSR_100FULL            0x4000  /* Can do 100mbps, full-duplex */
#define BMSR_100BASE4           0x8000  /* Can do 100mbps, 4k packets  */

/* Advertisement control register. */
#define ADVERTISE_SLCT          0x001f  /* Selector bits               */
#define ADVERTISE_CSMA          0x0001  /* Only selector supported     */
#define ADVERTISE_10HALF        0x0020  /* Try for 10mbps half-duplex  */
#define ADVERTISE_1000XFULL     0x0020  /* Try for 1000BASE-X full-duplex */
#define ADVERTISE_10FULL        0x0040  /* Try for 10mbps full-duplex  */
#define ADVERTISE_1000XHALF     0x0040  /* Try for 1000BASE-X half-duplex */
#define ADVERTISE_100HALF       0x0080  /* Try for 100mbps half-duplex */
#define ADVERTISE_1000XPAUSE    0x0080  /* Try for 1000BASE-X pause    */
#define ADVERTISE_100FULL       0x0100  /* Try for 100mbps full-duplex */
#define ADVERTISE_1000XPSE_ASYM 0x0100  /* Try for 1000BASE-X asym pause */
#define ADVERTISE_100BASE4      0x0200  /* Try for 100mbps 4k packets  */
#define ADVERTISE_PAUSE_CAP     0x0400  /* Try for pause               */
#define ADVERTISE_PAUSE_ASYM    0x0800  /* Try for asymetric pause     */
#define ADVERTISE_RESV          0x1000  /* Unused...                   */
#define ADVERTISE_RFAULT        0x2000  /* Say we can detect faults    */
#define ADVERTISE_LPACK         0x4000  /* Ack link partners response  */
#define ADVERTISE_NPAGE         0x8000  /* Next page bit               */

#define ADVERTISE_FULL (ADVERTISE_100FULL | ADVERTISE_10FULL | ADVERTISE_CSMA)
#define ADVERTISE_ALL (ADVERTISE_10HALF | ADVERTISE_10FULL | ADVERTISE_100HALF | ADVERTISE_100FULL)

#if 0
#define swap_16(x) \
	((unsigned short)( \
		(((unsigned short)(x) & (unsigned short)0x00ffU) << 8) | \
		(((unsigned short)(x) & (unsigned short)0xff00U) >> 8) ))
#define swap_32(x) \
	((unsigned long)( \
		(((unsigned long)(x) & (unsigned long)0x000000ffUL) << 24) | \
		(((unsigned long)(x) & (unsigned long)0x0000ff00UL) <<  8) | \
		(((unsigned long)(x) & (unsigned long)0x00ff0000UL) >>  8) | \
		(((unsigned long)(x) & (unsigned long)0xff000000UL) >> 24) ))
#endif

/* NIC specific static variables go here */
//static char packet[2096];
static unsigned char tx_packet[2048] __attribute__ ((aligned (128)));
static unsigned char rx_packet[2048] __attribute__ ((aligned (128)));

extern int usb_bulk_transfer_in(struct usb_device *dev,
			void *data, int len,int *transdata);
extern int usb_bulk_transfer_out(struct usb_device *dev,
			void *data, int len);
extern void  wait_ms(unsigned long ms);

/**************************************************************************
Operations
***************************************************************************/

/*
 * submits a control message and waits for comletion (at least timeout * 1ms)
 * If timeout is 0, we don't wait for completion (used as example to set and
 * clear keyboards LEDs). For data transfers, (storage transfers) we don't
 * allow control messages with 0 timeout, by previousely resetting the flag
 * asynch_allowed (usb_disable_asynch(1)).
 * returns the transfered length if OK or -1 if error. The transfered length
 * and the current status are stored in the dev->act_len and dev->status.
 *
 *	int usb_control_msg(struct usb_device *dev, unsigned int pipe,
 *			unsigned char request, unsigned char requesttype,
 *			unsigned short value, unsigned short index,
 *			void *data, unsigned short size, int timeout)
 *
 */
int
get_registers (struct usb_device *udev, u8 cmd, u16 value, u16 index, u16 size, void *data)
{

	return usb_control_msg (udev,
				usb_rcvctrlpipe(udev, AX_CTRL_EP),
				cmd,
				(USB_TYPE_VENDOR | USB_DIR_IN | USB_RECIP_DEVICE),
				value,
				index,
				data,
				size,
				USB_CNTL_TIMEOUT);
}

static int
set_registers (struct usb_device *udev, u8 cmd, u16 value, u16 index, u16 size, void *data)
{
	return usb_control_msg (udev,
				usb_sndctrlpipe (udev, AX_CTRL_EP),
				cmd,
				(USB_TYPE_VENDOR | USB_DIR_OUT | USB_RECIP_DEVICE),
				value,
				index,
				data,
				size,
				USB_CNTL_TIMEOUT);
}

#if 0
static int
ax88772a_mdio_read (struct usb_device *udev, int phy_id, int loc)
{
	unsigned short data;

	set_registers (udev, AX_CMD_SET_SW_MII, 0, 0, 0, NULL);

	get_registers (udev, AX_CMD_READ_MII_REG, phy_id, loc, 2, &data);

	set_registers (udev, AX_CMD_SET_HW_MII, 0, 0, 0, NULL);

	return data;
}

static void
ax88772a_mdio_write (struct usb_device *udev, int phy_id, int loc, int data)
{
	set_registers (udev, AX_CMD_SET_SW_MII, 0, 0, 0, NULL);

	set_registers (udev, AX_CMD_WRITE_MII_REG, phy_id, loc, 2, &data);

	set_registers (udev, AX_CMD_SET_HW_MII, 0, 0, 0, NULL);
}
#endif
static int ax88772a_set_mac_addr (struct usb_device *udev, u8 *mac)
{
	return set_registers (udev,
			AX_CMD_WRITE_NODE_ID,
			0, 0, 6, mac);
}

static int ax88772a_bind (struct usb_device *udev, struct eth_device *nic)
{
	int ret;

	do {
		/* select the embedded 10/100 Ethernet PHY */
		ret = set_registers (udev, AX_CMD_SW_PHY_SELECT,
				(SW_PHYSEL_PSEL | SW_PHYSEL_SSEN), 0, 0, NULL);
		if (ret < 0) break;

		/*
		 * set the embedded Ethernet PHY in power-up
		 * mode and operating state.
		 */
		ret = set_registers (udev, AX_CMD_SW_RESET,
				AX_SWRESET_IPRL, 0, 0, NULL);
		if (ret < 0) break;

		/*
		 * set the embedded Ethernet PHY in power-down
		 * mode and operating state.
		 */
		ret = set_registers (udev, AX_CMD_SW_RESET,
				(AX_SWRESET_IPPD | AX_SWRESET_IPRL), 0, 0, NULL);
		if (ret < 0) break;

		wait_ms(80);		//angus1225

		/*
		 * set the embedded Ethernet PHY in power-up mode
		 * and operating state.
		 */
		ret = set_registers (udev, AX_CMD_SW_RESET,
				AX_SWRESET_IPRL, 0, 0, NULL);
		if (ret < 0) break;

		wait_ms(500);		//angus1225

		/*
		 * set the embedded Ethernet PHY in power-up mode
		 * and reset state.
		 */
		ret = set_registers (udev, AX_CMD_SW_RESET,
				AX_SWRESET_CLEAR, 0, 0, NULL);
		if (ret < 0) break;

		/*
		 * set the embedded Ethernet PHY in power-up mode
		 * and operating state.
		 */
		ret = set_registers (udev, AX_CMD_SW_RESET,
			(AX_SWRESET_IPRL | AX_SWRESET_BZAUTO | AX_SWRESET_BZ),
			0, 0, NULL);
		if (ret < 0) break;

		/* stop MAC operation */
		ret = set_registers (udev, AX_CMD_WRITE_RX_CTL,
				AX_RX_CTL_STOP, 0, 0, NULL);
		if (ret < 0) break;

#if 1
		/* Get the MAC address */
		ret = get_registers (udev, AX_CMD_READ_NODE_ID,
				0, 0, 6, nic->enetaddr);
		if (ret < 0) break;
#else
		/* Sample code to show how to overwrite the MAC address */
		/* Assume the MAC address is 00-11-22-33-44-55 */
		nic->enetaddr[0] = 0x00;
		nic->enetaddr[1] = 0x11;
		nic->enetaddr[2] = 0x22;
		nic->enetaddr[3] = 0x33;
		nic->enetaddr[4] = 0x44;
		nic->enetaddr[5] = 0x55;
		ret = ax88772a_set_mac_addr (udev, nic->enetaddr);
		if (ret < 0) break;
#endif

		printf("Found AX88772A : %02x-%02x-%02x-%02x-%02x-%02x\n",
			nic->enetaddr[0], nic->enetaddr[1], nic->enetaddr[2],
			nic->enetaddr[3], nic->enetaddr[4], nic->enetaddr[5]);

		ax88772a_set_mac_addr (udev, nic->enetaddr);
		ret = set_registers (udev, AX_CMD_WRITE_MEDIUM_MODE,
				AX88772_MEDIUM_DEFAULT, 0, 0, NULL);

	} while (0);

#if 0
	/* Enable PHY pause capability */
	ax88772a_mdio_write (udev, 0x10, MII_ADVERTISE,
		(ax88772a_mdio_read (udev, 0x10, MII_ADVERTISE)
		| ADVERTISE_PAUSE_CAP));

	ax88772a_mdio_write (udev, 0x10, MII_BMCR,
		(BMCR_ANRESTART | BMCR_ANENABLE));
#endif
	return ret;
}

static void ax88772a_disable(struct eth_device *nic)
{
	netif_stop_queue(nic);
	/* stop MAC operation */
	set_registers (nic->udev, AX_CMD_WRITE_RX_CTL,
			AX_RX_CTL_STOP, 0, 0, NULL);
}

/**************************************************************************
POLL - Wait for a frame
***************************************************************************/
static int ax88772a_rx(struct eth_device *nic)
{
	int ret, actual_length;
	unsigned long header;
	int pktlen;
	unsigned char *pkt;

	actual_length = 0;
#if 0	//angus1225
	ret = usb_bulk_msg (nic->udev,
			usb_rcvbulkpipe(nic->udev, AX_BULKIN_EP),
			rx_packet,
			2048,
			&actual_length,
			USB_CNTL_TIMEOUT);
#else
	ret = usb_bulk_transfer_in (nic->udev,
			rx_packet, 2048, &actual_length);
#endif
	if (ret < 0)
		return 1;

	pkt = rx_packet;

	while (actual_length) {

		memcpy (&header, pkt, 4);

		if ((short)(header & 0x0000ffff) !=
			~((short)((header & 0xffff0000) >> 16))) {
			break;
		}

		pktlen = (header & 0xffff);
		if (pktlen > 1514) {
			printf ("  Packet length error (%d)\n", pktlen);
			break;
		}

		/* Skip packet header */
		pkt += 4;

		NetReceive(pkt, pktlen);

		pkt += ((pktlen + 1) & 0xfffe);

		actual_length -= ((pktlen + 1) & 0xfffe) + 4;
	}

	return 1;
}

/**************************************************************************
TRANSMIT - Transmit a frame
***************************************************************************/
static int
ax88772a_tx(struct eth_device *nic, void *packet, int length)
{
	int padlen;
	unsigned long header, pktlen;
	unsigned long padbytes = 0xffff0000;

	//int actual_length;
	//int ret;

	pktlen = length;

	header = ((pktlen ^ 0x0000ffff) << 16) + pktlen;
	padlen = ((pktlen + 4) % 512) ? 0 : 4;

	memcpy (tx_packet, &header, 4);
	memcpy (&tx_packet[4], (unsigned char *)packet, length);
	if (padlen)
		memcpy (&tx_packet[pktlen + 4], (unsigned long *)padbytes, 4);
#if 0
	ret = usb_bulk_msg (nic->udev,
				usb_sndbulkpipe (nic->udev, AX_BULKOUT_EP),
				tx_packet,
				(pktlen + 4 + padlen),
				&actual_length,
				USB_CNTL_TIMEOUT);
#else
	usb_bulk_transfer_out (nic->udev,
			tx_packet, (pktlen + 4 + padlen));
#endif
	return 1;
}

static int ax88772a_open(struct eth_device *nic, bd_t * bis)
{

	memset (rx_packet, 0, 2048);

	/* Set RX_CTL to default values with 2k buffer, and enable cactus */
	set_registers (nic->udev, AX_CMD_WRITE_RX_CTL,
		AX_DEFAULT_RX_CTL, 0, 0, NULL);

	return 1;
}

/**************************************************************************
INIT - set up ethernet interface(s)
***************************************************************************/
int ax88772a_init (struct eth_device *nic)
{
    nic->init = ax88772a_open;
    nic->recv = ax88772a_rx;
    nic->send = ax88772a_tx;
    nic->halt = ax88772a_disable;

    if (ax88772a_bind (nic->udev, nic))
        return 0;

    printf ("%s :v%s %s %s\n      http://www.asix.com.tw\n",
        AX88772A_DRV_NAME, AX88796C_DRV_VERSION,
        __TIME__, __DATE__);

    return 1;
}
