/*
* mhal_sata_host_ahci.h- Sigmastar
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

#ifndef _MHAL_SATA_HOST_AHCI_H
#define _MHAL_SATA_HOST_AHCI_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pm.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>


struct ahci_platform_data {
#if defined(CONFIG_ARCH_INFINITY2)
        int (*init)(struct device *dev, void __iomem *addr , int id);
#elif defined(CONFIG_ARCH_INFINITY2M)
        int (*init)(struct device *dev, void __iomem *addr);
#endif
        void (*exit)(struct device *dev);
        int (*suspend)(struct device *dev);
        int (*resume)(struct device *dev);
        const struct ata_port_info *ata_port_info;
        unsigned int force_port_map;
        unsigned int mask_port_map;
};


void ss_sata_misc_init(void *mmio, int n_ports);
void ss_sata_phy_init(void *mmio,int phy_mode, int n_ports);
#endif
