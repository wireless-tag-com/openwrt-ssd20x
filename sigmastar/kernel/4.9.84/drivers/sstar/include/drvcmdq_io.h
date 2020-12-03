/*
* drvcmdq_io.h- Sigmastar
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
#ifndef __DRVCMDQ_IO_HH__
#define __DRVCMDQ_IO_HH__

#define IOCTL_CMDQ_TEST1_CONFIG                                (1)
#define IOCTL_VIP_MAGIC                                        ('3')
/**
* Used to set CMDQ cmd, use ST_IOCTL_VIP_CMDQ_CONFIG.
*/
#define IOCTL_CMDQ_TEST_1                            _IO(IOCTL_VIP_MAGIC,  IOCTL_CMDQ_TEST1_CONFIG)


#endif
