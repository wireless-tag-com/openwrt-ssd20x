/*
* mdrv_sata_host_ahci.h- Sigmastar
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

#ifndef _MDRV_SATA_HOST_AHCI_H_
#define _MDRV_SATA_HOST_AHCI_H_

struct sstar_platform_data {
    int (*init)(struct device *dev, void __iomem *addr , int id);
    void (*exit)(struct device *dev);
    int (*suspend)(struct device *dev);
    int (*resume)(struct device *dev);
    const struct ata_port_info *ata_port_info;
    unsigned int force_port_map;
    unsigned int mask_port_map;
};

#endif
