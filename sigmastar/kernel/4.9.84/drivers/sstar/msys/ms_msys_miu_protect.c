/*
* ms_msys_miu_protect.c- Sigmastar
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
#include <linux/kernel.h>
#include <asm/uaccess.h>  /* for get_fs*/
#include <linux/miscdevice.h>
#include <linux/dma-mapping.h>      /* for dma_alloc_coherent */
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/compaction.h> /*  for sysctl_compaction_handler*/
#include <asm/cacheflush.h>

#include "registers.h"
#include "mdrv_miu.h"
extern struct miscdevice  sys_dev;


static ssize_t miu_protect_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *str = buf;
    int n;

    n = sprintf(str, "[%s][%d]: OK!\n", __FUNCTION__, __LINE__);
    return n;
}

static ssize_t miu_protect_entry(struct device *dev, struct device_attribute *attr, const char *buf, size_t n)
{
    unsigned char Result = FALSE;
    unsigned char u8Blockx;
    unsigned short u8ProtectId[16] = {0};
    unsigned long long u64BusStart;
    unsigned long long u64BusEnd;
    unsigned char  bSetFlag;

    unsigned char token[16];

    sscanf(buf, "%s", token);
    if (0 == strcasecmp(token, "set"))
    {
        sscanf(buf, "%s %hhu %hu %llx %llx %hhu", token, &u8Blockx, &u8ProtectId[0], &u64BusStart, &u64BusEnd, &bSetFlag);

        printk("%s(%d) INPUT: (u8Blockx,u8ProtectId,u64BusStart,u64BusEnd,bSetFlag)=(%hhu,%hu,0x%08llx,0x%08llx,%hhu)\n", __FUNCTION__, __LINE__, u8Blockx, u8ProtectId[0], u64BusStart, u64BusEnd, bSetFlag);

        Result = MDrv_MIU_Protect(u8Blockx, &u8ProtectId[0], u64BusStart, u64BusEnd, bSetFlag);

        if(Result == FALSE ) {
            printk("ERR: Result = %d", Result);
        }
    }
    else if (0 == strcasecmp(token, "test"))
    {
        unsigned int* _va;
        sscanf(buf, "%s %llx", token, &u64BusStart);

        _va = ioremap((unsigned int)u64BusStart, 0x1000);
        printk("%s(%d) Write: MIU @ 0x%08llx, VA @ 0x%08x\n", __FUNCTION__, __LINE__, u64BusStart, (unsigned int)_va);

        *(_va) = 0xDEADBEEF;
    }
    else if (0 == strcasecmp(token, "test1"))
    {
        sscanf(buf, "%s %llx", token, &u64BusStart);

        printk("%s(%d) Write: MIU @ 0x%08llx\n", __FUNCTION__, __LINE__, u64BusStart);

        *(unsigned int*)((void*)(unsigned int)u64BusStart) = 0xDEADBEEF;
    }
    else
    {
         printk("%s(%d) Wrong parameter:\n", __FUNCTION__, __LINE__);
         printk("%s(%d) Usage: echo [CMD] [miublk] [client id] [start addr] [end addr] [enable] > miu_protect\n", __FUNCTION__, __LINE__);
         printk("%s(%d) Ex: echo set 0 1 0x20000000 0x20100000 1 > miu_protect\n", __FUNCTION__, __LINE__);
    }

    return n;
}
DEVICE_ATTR(miu_protect, 0644, miu_protect_show, miu_protect_entry);

int msys_miu_protect_init(void)
{
    device_create_file(sys_dev.this_device, &dev_attr_miu_protect);
    return 0;
}
device_initcall(msys_miu_protect_init);