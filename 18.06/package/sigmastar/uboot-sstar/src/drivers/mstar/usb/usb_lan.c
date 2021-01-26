/*
* usb_lan.c- Sigmastar
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

//extern int rtl8150_init(struct eth_device *nic);
//extern int pegasus_init(struct eth_device *nic);
extern int ax88772a_init(struct eth_device *nic);

struct eth_device *gnic=NULL;
typedef int  (*__entry) (struct eth_device*);

struct init_info {
	unsigned short idVendor;
	unsigned short idProduct;
    __entry init;
};

struct init_info ctrlTbl[] =
{
//	{VENDOR_ID_REALTEK, PRODUCT_ID_RTL8150, rtl8150_init}
//	,{VENDOR_ID_ADMTEK, PRODUCT_ID_ADM8515, pegasus_init}
	{VENDOR_ID_ASIX, PRODUCT_ID_AX88772A, ax88772a_init}
};

#define USBLAN_MAX_INFOTBL  (sizeof(ctrlTbl)/sizeof(struct init_info))

static int lookup_ctrlTbl(u16 idVendor, u16 idProduct)
{
    u8 idx = 0;

    for(; idx < USBLAN_MAX_INFOTBL; idx++)
    {
        if (idVendor == ctrlTbl[idx].idVendor
            && idProduct == ctrlTbl[idx].idProduct)
            break;
    }
    return idx;
}

void usb_lan_release(void)
{
    if (gnic)
        free(gnic);
}

void dump_pkt(u8 *pkt, u32 len)
{
    u32 i = 0;

    DEBUGFUNC();

    for (; i< len; i++)
    {
        if (i%0x10 == 0)
            printf("\n%x: ", (u32)pkt);

        if (*pkt < 0x10)
            printf("0%x ", *pkt);
        else
            printf("%x ", *pkt);
        pkt++;
    }

    printf("\n");
}

/**************************************************************************
PROBE - Look for an adapter, this routine's visible to the outside
You should omit the last argument struct pci_device * for a non-PCI NIC
***************************************************************************/
int
usb_lan_initialize(struct usb_device *udev)
{
	struct usb_lan_hw *hw = NULL;
	struct eth_device *nic = NULL;
	u8 ret = 1;

    DEBUGFUNC();

	nic = (struct eth_device *) malloc(sizeof (struct eth_device));
	if (!nic) {
	    return -ENOMEM;
	}
    hw = (struct usb_lan_hw *) malloc(sizeof (struct usb_lan_hw));
	if (!hw) {
        free(nic);
	    return -ENOMEM;
	}

    gnic=nic;
    nic->udev = udev;
	nic->priv = hw;

    printf("vendor:%x product:%x\n",nic->udev->descriptor.idVendor, nic->udev->descriptor.idProduct);
    hw->tblIdx = lookup_ctrlTbl(nic->udev->descriptor.idVendor, nic->udev->descriptor.idProduct);
    if (USBLAN_MAX_INFOTBL == hw->tblIdx)
    {
        USB_LAN_ERR("Can't find usb lan dev!!\n");
        free(nic);
        free(hw);
         return -ENXIO;
    }

    printf("Go to fxp:%x\n", (unsigned int)ctrlTbl[hw->tblIdx].init);
    //printf cause fxp crash
#if 0
    ret = ctrlTbl[hw->tblIdx].init(nic);
#else
//    if (hw->tblIdx==0)
//        ret = rtl8150_init(nic);
//    else if (hw->tblIdx==1)
//        ret = pegasus_init(nic);
//    else
        ret = ax88772a_init(nic);

#endif

    if (!ret)
    {
        printf("init failed device:%u\n", hw->tblIdx);
        free(nic);
        free(hw);        
        return 0;
    }

    memcpy(nic->name,"AX88772",sizeof("AX88772"));
	eth_register(nic);

//	uEMAC_start = 0;
//	uUSB_LAN_start = 1;
//    free(nic);
//    free(hw);
	return 1;
}

