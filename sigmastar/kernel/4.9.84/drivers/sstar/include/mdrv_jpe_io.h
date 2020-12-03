/*
* mdrv_jpe_io.h- Sigmastar
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
#ifndef _MDRV_JPE_IO_H_
#define _MDRV_JPE_IO_H_

#define IOCPARM_MASK    0x7f            /* parameters must be < 128 bytes */
#define IOC_JPE_VOID    0x20000000      /* no parameters */
#define IOC_JPE_OUT     0x40000000      /* copy out parameters */
#define IOC_JPE_IN      0x80000000      /* copy in parameters */
#define IOC_JPE_INOUT   (IOC_JPE_IN|IOC_JPE_OUT)

#define _IO_JPE(x,y)    (IOC_JPE_VOID|((x)<<8)|(y))
#define _IOR_JPE(x,y,t) (IOC_JPE_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))
#define _IOW_JPE(x,y,t) (IOC_JPE_IN|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

//JPE IOC COMMANDS
#define JPE_IOC_MAGIC 'J'
// Initialize *pJpeInfo
#define JPE_IOC_INIT                _IO_JPE(JPE_IOC_MAGIC, 0)
// Set up JPE RIU and fire JPE
#define JPE_IOC_ENCODE_FRAME        _IO_JPE(JPE_IOC_MAGIC, 1)
// Get output buffer status
#define JPE_IOC_GETBITS             _IOR_JPE(JPE_IOC_MAGIC, 2, __u32)
//
#define JPE_IOC_GET_CAPS            _IOR_JPE(JPE_IOC_MAGIC, 3, __u32)
//
#define JPE_IOC_SET_OUTBUF          _IOR_JPE(JPE_IOC_MAGIC, 4, __u32)

#endif // _MDRV_JPE_IO_H_