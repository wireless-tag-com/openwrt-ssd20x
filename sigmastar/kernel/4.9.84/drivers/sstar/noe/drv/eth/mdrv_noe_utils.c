/*
* mdrv_noe_utils.c- Sigmastar
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
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (“MStar Confidential Information”) by the recipient.
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   MDRV NOE UTILS .c
/// @brief  NOE Driver
///
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------
#include "mdrv_noe.h"



static int _MDrv_NOE_UTIL_Getnext(const char *src, int separator, char *dest)
{
    char *c;
    int len;

    if (!src || !dest)
        return -1;

    c = strchr(src, separator);
    if (!c) {
        strcpy(dest, src);
        return -1;
    }
    len = c - src;
    strncpy(dest, src, len);
    dest[len] = '\0';
    return len + 1;
}

int MDrv_NOE_UTIL_Str_To_Ip(unsigned int *ip, const char *str)
{
    int len;
    const char *ptr = str;
    char buf[128];
    unsigned char c[4];
    int i;
    int ret;

    for (i = 0; i < 3; ++i) {
        len = _MDrv_NOE_UTIL_Getnext(ptr, '.', buf);
        if (len == -1)
            return 1;   /* parse error */

        ret = kstrtoul(buf, 10, (unsigned long *)&c[i]);
        if (ret)
            return ret;

        ptr += len;
    }
    ret = kstrtoul(ptr, 0, (unsigned long *)&c[3]);
    if (ret)
        return ret;

    *ip = (c[0] << 24) + (c[1] << 16) + (c[2] << 8) + c[3];

    return 0;
}

