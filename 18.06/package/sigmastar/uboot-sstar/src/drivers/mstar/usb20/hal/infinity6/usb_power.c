/*
* usb_power.c- Sigmastar
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

#include "gpio.h"
#include "mhal_gpio.h"

void USB_Power_On(void)
{
  MHal_GPIO_Pad_Set(PAD_PM_GPIO2);
  MHal_GPIO_Set_High(PAD_PM_GPIO2);
  return;
}
//--------------------

