/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#define _GOP_SYSFS_C_

#include <linux/device.h>
#include "cam_os_wrapper.h"
#include "gop_ut.h"
#include "cam_sysfs.h"

#ifdef GOP_OS_TYPE_LINUX_KERNEL_TEST

ssize_t check_goptest_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    if(NULL!=buf)
    {
        GopUtStrConfig_t stStringCfg;

        GopUtParsingCommand((char *)buf, &stStringCfg);

        GopTestStore(&stStringCfg);
        return n;
    }
    return 0;
}

ssize_t check_goptest_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return GopTestShow(buf);
}
static DEVICE_ATTR(goptest,0644, check_goptest_show, check_goptest_store);
#endif

void DrvGopSysfsInit(struct device *device)
{
#ifdef GOP_OS_TYPE_LINUX_KERNEL_TEST
    CamDeviceCreateFile(device, &dev_attr_goptest);
#endif
}
