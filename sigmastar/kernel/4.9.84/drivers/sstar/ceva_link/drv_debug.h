/*
* drv_debug.h- Sigmastar
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
#ifndef _CEVA_LINKDRV_MSG_H_
#define _CEVA_LINKDRV_MSG_H_

#include <linux/kernel.h>
#include <linux/module.h>

// Defines reference kern levels of printfk
#define XM6_MSG_INIT    3
#define XM6_MSG_ERR     3
#define XM6_MSG_WRN     4
#define XM6_MSG_DBG     5

#define XM6_MSG_LEVL    XM6_MSG_WRN

#define XM6_MSG_ENABLE

#if defined(XM6_MSG_ENABLE)
#define XM6_MSG_FUNC_ENABLE

#define XM6_STRINGIFY(x) #x
#define XM6_TOSTRING(x) XM6_STRINGIFY(x)

#if defined(XM6_MSG_FUNC_ENABLE)
#define XM6_MSG_TITLE   "[XM6, %s] "
#define XM6_MSG_FUNC    __func__
#else   // NOT defined(XM6_MSG_FUNC_ENABLE)
#define XM6_MSG_TITLE   "[XM6] %s"
#define XM6_MSG_FUNC    ""
#endif  // NOT defined(XM6_MSG_FUNC_ENABLE)

#define XM6_ASSERT(_con)   \
    do {\
        if (!(_con)) {\
            printk(KERN_CRIT "BUG at %s:%d assert(%s)\n",\
                    __FILE__, __LINE__, #_con);\
            BUG();\
        }\
    } while (0)

#define XM6_MSG(dbglv, _fmt, _args...)                          \
    do if(dbglv <= XM6_MSG_LEVL) {                              \
        printk(KERN_SOH XM6_TOSTRING(dbglv) XM6_MSG_TITLE  _fmt, XM6_MSG_FUNC, ## _args);   \
    } while(0)

#else   // NOT defined(XM6_MSG_ENABLE)
#define     XM6_ASSERT(arg)
#define     XM6_MSG(dbglv, _fmt, _args...)
#endif  // NOT defined(XM6_MSG_ENABLE)

#endif //_CEVA_LINKDRV_MSG_H_
