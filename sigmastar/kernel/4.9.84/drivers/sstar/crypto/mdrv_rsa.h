/*
* mdrv_rsa.h- Sigmastar
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


#ifndef _IOCTL_TEST_H
#define _IOCTL_TEST_H

#include <linux/ioctl.h>

struct rsa_config {
unsigned int *pu32RSA_Sig;
unsigned int *pu32RSA_KeyN;
unsigned int *pu32RSA_KeyE;
unsigned int *pu32RSA_Output;
unsigned int u32RSA_KeyNLen;
unsigned int u32RSA_KeyELen;
unsigned int u32RSA_SigLen;
};




/* ocumentation/ioctl/ioctl-number.txt */
#define IOC_MAGIC '\x66'

#define MDrv_RSA_Reset                _IO(IOC_MAGIC, 0x92)
#define MDrv_RSA_Setmode              _IO(IOC_MAGIC, 0x93)
#define MDrv_RSA_Calculate            _IO(IOC_MAGIC, 0x94)

int rsa_crypto(struct rsa_config *op);

#endif